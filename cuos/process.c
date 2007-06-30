/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cucon/fwd.h>
#include <cu/str.h>
#include <cucon/list.h>
#include <cu/str.h>
#include <cuos/path.h>
#include <cuos/process.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

cu_str_t
cuos_search_path_cstr_str(char const *path, cu_str_t file)
{
    char const *e;
    do {
	struct stat fst;
	cu_str_t str;
	e = strchr(path, ':');
	if (e) {
	    str = cu_str_new_charr(path, e - path);
	    path = e + 1;
	}
	else
	    str = cu_str_new_cstr(path);
	str = cuos_path_join(str, file);
	assert(str);
	if (stat(cu_str_to_cstr(str), &fst) == 0)
	    return str;
    } while (e);
    return NULL;
}

pid_t
cuos_start_prog_fd(char const *prg, char const *const *argv,
		   char *const *envp, int *fd_arr)
{
    pid_t pid;
    int i;

    if ((pid = fork()) == 0) {
	if (fd_arr) {
	    for (i = 0; i < 3; ++i)
		if (fd_arr[i] != -1) {
		    int j;
		    dup2(fd_arr[i], i);
		    for (j = i + 1; j < 3; ++j)
			if (fd_arr[i] == fd_arr[j])
			    break;
		    if (j != 3)
			close(fd_arr[i]);
		}
	}
	for (i = 0; prg[i]; ++i)
	    if (prg[i] == '/')
		break;
	if (!prg[i]) {
	    cu_str_t str_prg;
	    char const *path = getenv("PATH");
	    if (!path)
		path = ":/bin:/usr/bin";
	    str_prg
		= cuos_search_path_cstr_str(path, cu_str_new_cstr(prg));
	    if (!str_prg) {
		cu_errf("Path lookup of ‘%s’ failed.", prg);
		exit(1);
	    }
	    prg = cu_str_to_cstr(str_prg);
	}
	if (envp)
	    execve(prg, (char *const *)argv, envp);
	else
	    execv(prg, (char *const *)argv);
	cu_errf("Could not exec %s in subprocess: %s",
		 prg, strerror(errno));
	exit(1);
    }
    else if (pid == -1)
	cu_errf("Failed to create subprocess for %s: %s",
		 prg, strerror(errno));
    if (fd_arr)
	for (i = 0; i < 3; ++i)
	    if (fd_arr[i] != -1)
		close(fd_arr[i]);
    return pid;
}

pid_t
cuos_start_prog_io(char const *prg, char const *const *argv,
		      char *const *envp,
		      FILE **in, FILE **out, FILE **err)
{
    int i;
    pid_t pid = -1;
    int fd_pairs[3][2];
    int fd_arr[3];
    FILE **files[] = { in, out, err };
    for (i = 0; i < 3; ++i) {
	if (files[i]) {
	    int j_this = (i == 0? 1 : 0);
	    if (pipe(fd_pairs[i]) != 0) {
		cu_errf("Could not create a pipe.\n");
		goto failed_fds;
	    }
	    *files[i] = fdopen(fd_pairs[i][j_this], i == 0? "w" : "r");
	    if (*files[i] == NULL) {
		cu_errf("Could not create FILE from file descriptor: %s",
			 strerror(errno));
		close(fd_pairs[i][0]);
		close(fd_pairs[i][1]);
		goto failed_fds;
	    }
	    fd_arr[i] = fd_pairs[i][1 - j_this];
	}
	else
	    fd_arr[i] = -1;
    }

    pid = cuos_start_prog_fd(prg, argv, envp, fd_arr);
    if (pid == -1) {
	for (i = 0; i < 3; ++i)
	    if (files[i]) {
		if (*files[i])
		    fclose(*files[i]);
	    }
    }
    return pid;

failed_fds:
    while (--i > 0) {
	if (files[i]) {
	    close(fd_pairs[i][i == 0]);
	    fclose(*files[i]);
	}
    }
    return -1;
}

int
cuos_wait_for_prog(char const *prg, pid_t pid)
{
    int status;
    if (waitpid(pid, &status, 0) == -1) {
	cu_errf("Wait-for-subprocess %s failed: %s",
		 prg, strerror(errno));
	return status;
    }
    if (WIFEXITED(status)) {
	if (WEXITSTATUS(status) != 0) {
	    cu_errf("%s failed with status=%d.",
		     prg, WEXITSTATUS(status));
	}
	return status;
    }
    else if (WIFSIGNALED(status)) {
	cu_errf("%s terminated with signal %d.",
		 prg, WTERMSIG(status));
	return status;
    }
    else
	return status;
}

int
cuos_call_prog(char const *prg, char const *const *argv, char *const *envp)
{
    pid_t pid = cuos_start_prog_fd(prg, argv, envp, NULL);
    if (pid == -1)
	return -1;
    return cuos_wait_for_prog(prg, pid);
}

int
cuos_call_prog_fd(char const *prg, char const *const *argv,
		     char *const *envp, int *fd_arr)
{
    pid_t pid = cuos_start_prog_fd(prg, argv, envp, fd_arr);
    if (pid == -1)
	return -1;
    return cuos_wait_for_prog(prg, pid);
}

void
cuos_argv_cct_strlist(char const **argv, cucon_list_t lst)
{
    cucon_listnode_t it;
    for (it = cucon_list_begin(lst);
	 it != cucon_list_end(lst);
	 it = cucon_listnode_next(it))
	*argv++ = cu_str_to_cstr(cucon_listnode_ptr(it));
    *argv = NULL;
}

static cu_bool_t
cuos_shell_char_is_safe(char ch)
{
    return isalnum(ch) || ch == '_' || ch == '-' || ch == '+'
	|| ch == '/';
}

cu_str_t
cuos_shell_escape_cstr(char const *cstr)
{
    cu_str_t dst = cu_str_new();
    char ch;
    while ((ch = *cstr)) {
	if (!cuos_shell_char_is_safe(ch))
	    cu_str_append_char(dst, '\\');
	cu_str_append_char(dst, ch);
	++cstr;
    }
    return dst;
}

void
cuos_argv_fprint(char const **argv, FILE *out)
{
    int i;
    fputs(argv[0], out);
    for (i = 1; argv[i]; ++i) {
	cu_str_t esc = cuos_shell_escape_cstr(argv[i]);
	fputc(' ', out);
	fputs(cu_str_to_cstr(esc), out);
    }
}
