/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_THREADLOCAL_H
#define CU_THREADLOCAL_H

#include <cu/fwd.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_threadlocal_h cu/threadlocal.h: Thread-Local Storage Boilerplates
 *@{\ingroup cu_base_mod
 *
 * These boilerplates allows the client application or library to define
 * thread-local storage using an available mechanism.  Currently, the __thread
 * keyword is used if detected, otherwise the POSIX threads functons are used.
 * Initialisation is assured either by creating threads with \ref
 * cu_pthread_create or by calling \ref cu_thread_init before any local storage
 * is used.
 *
 * \def CU_THREADLOCAL_DECL(NAME, PRIVATE_NAME)
 * Emits declarations for the header file.  Assuming that the struct NAME_s and
 * an associated pointer NAME_t have been defined, this provides a function
 * <tt>NAME_t NAME(void)</tt> which returns the thread-local storage.  \a
 * PRIVATE_NAME is a prefix for for identifiers which are to be considered
 * private.
 *
 * \def CU_THREADLOCAL_DEF(NAME, PRIVATE_NAME, STATIC_NAME)
 * Emits definitions associated with \ref CU_THREADLOCAL_DECL(\a NAME, \a
 * PRIVATE_NAME).  \a STATIC_NAME is a prefix for file local definitions
 * including the following two functions which must be defined:
 * \li <tt>void STATIC_NAME_init(NAME_t tls);</tt>
 * \li <tt>void STATIC_NAME_destruct(NAME_t tls);</tt>
 *
 * \def CU_THREADLOCAL_INIT(NAME, PRIVATE_NAME, STATIC_NAME)
 * Emits initialisation code for definitions in \ref CU_THREADLOCAL_DEF(\a
 * NAME, \a PRIVATE_NAME, \a STATIC_NAME).  This code must be run in the main
 * thread before other threads are created, typically during a global
 * initialisation phase.
 */

#ifdef CUCONF_HAVE_THREAD_KEYWORD

#  define CU_THREADLOCAL_DECL(name, private_name)			\
    extern __thread struct name##_s private_name##_inst;		\
									\
    /*!Returns thread-local state. */					\
    CU_SINLINE name##_t name(void)					\
    { return &private_name##_inst; }					\
									\
    CU_END_BOILERPLATE

#  define CU_THREADLOCAL_DEF(name, private_name, static_name)		\
    __thread struct name##_s private_name##_inst;			\
									\
    cu_clop_def0(static_name##_on_thread_entry, void)			\
    {									\
	name##_t tls = name();						\
	GC_add_roots((char *)tls, (char *)(tls + 1));			\
	static_name##_init(tls);					\
    }									\
									\
    cu_clop_def0(static_name##_on_thread_exit, void)			\
    {									\
	name##_t tls = name();						\
	static_name##_destruct(tls);					\
	GC_remove_roots((char *)tls, (char *)(tls + 1));		\
    }									\
									\
    CU_END_BOILERPLATE

#  define CU_THREADLOCAL_INIT(name, private_name, static_name)		\
    cu_register_thread_init(cu_clop_ref(static_name##_on_thread_entry),	\
			    cu_clop_ref(static_name##_on_thread_exit))

#else /* !CUCONF_HAVE_THREAD_KEYWORD */

#  define CU_THREADLOCAL_DECL(name, private_name)			\
    extern pthread_key_t private_name##_key;				\
									\
    /*!Returns thread-local state. */					\
    CU_SINLINE name##_t name(void)					\
    { return (name##_t)pthread_getspecific(private_name##_key); }	\
									\
    CU_END_BOILERPLATE

#  define CU_THREADLOCAL_DEF(name, private_name, static_name)		\
    pthread_key_t private_name##_key;					\
									\
    cu_clop_def0(static_name##_on_thread_entry, void)			\
    {									\
	int err_code;							\
	name##_t tls = cu_gnew_u(struct name##_s);			\
	static_name##_init(tls);					\
	err_code = pthread_setspecific(private_name##_key, tls);	\
	if (err_code)							\
	    cu_handle_syserror(err_code, "pthread_setspecific");	\
    }									\
									\
    void static_name##_on_thread_exit(void *tls)			\
    {									\
	static_name##_destruct(tls);					\
	cu_gfree_u(tls);						\
    }									\
									\
    CU_END_BOILERPLATE

#  define CU_THREADLOCAL_INIT(name, private_name, static_name)		\
    do {								\
	cu_pthread_key_create(&private_name##_key, (void (*)(void *))	\
		CU_MARG(void (*)(name##_t), static_name##_destruct));	\
	cu_register_thread_init(cu_clop_ref(static_name##_on_thread_entry), \
				cu_clop_null);				\
    } while (0)

#endif /* !CUCONF_HAVE_THREAD_KEYWORD */

/*!@}*/
CU_END_DECLARATIONS

#endif
