/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuflow/workers.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

double g_I;

cu_clop_def0(burner, void)
{
    int i;
    int const steps = 10000000;
    double I = 0.0;
    printf("burner enters.\n");
    for (i = 0; i < steps; ++i)
	I += sin(i*M_PI/steps);
    g_I += I;
    printf("burner returns.\n");
}

cu_clop_def(clicky, void, struct timespec *t_now)
{
    struct timespec t_next;
    memcpy(&t_next, t_now, sizeof(struct timespec));
    t_next.tv_sec += 1;
    printf("clicky at %ld.%09ld s.\n", t_now->tv_sec, t_now->tv_nsec);
    cuflow_workers_call_at(clicky, &t_next);
}

int main()
{
    struct timespec t_click;
    cuflow_init();
    g_I = 0.0;
    cuflow_workers_spawn(2);

    clock_gettime(CLOCK_REALTIME, &t_click);
    t_click.tv_sec += 1;
    cuflow_workers_call_at(clicky, &t_click);
    sleep(3);

    cuflow_workers_call(burner);
    cuflow_workers_call(burner);
    cuflow_workers_call(burner);
    cuflow_workers_spawn(5);
    cuflow_workers_call(burner);
    cuflow_workers_spawn(2);
    cuflow_workers_call(burner);
    cuflow_workers_spawn(0);
    return 0;
}
