/*
	Here is an interface of the thread lock to hide the platform-dependent libraries. 

	The mutex lock allows only one thread entering the critical section, and only the thread setting the lock can unlock the mutex. 
	No additional argument is needed for the thrd_lock_init, thrd_lock_on, and thrd_lock_off when using the mutex lock. 
	thrd_lock_init(thrd_lock_ptr, thrd_lock_type_mutex);

	The semphore allows only N threads entering the critical section. 
	The thrd_lock_init requires the capacity and the initial value of avail_rsc when using the semephore. 
	thrd_lock_init(thrd_lock_ptr, thrd_lock_type_sem, capacity, avail_rsc);

	The producer-consumer model controls a bounded buffer. 
	The thrd_lock_init requires the capacity when using the producer-consumer model. 
	The thrd_lock_on requires the is_prod when using the producer-consumer model. 
	The thrd_lock_off requires the is_prod when using the producer-consumer model, and the is_done to specify if it was completed. 
	thrd_lock_init(thrd_lock_ptr, thrd_lock_type_prod_cons, capacity);
	thrd_lock_prod_off(thrd_lock_ptr, is_done);
	thrd_lock_cons_off(thrd_lock_ptr, is_done);
*/

#ifndef __THRD_THRD_LOCK_H__
#define __THRD_THRD_LOCK_H__

#include "sys_info.h"
#include <stddef.h>
#include <stdarg.h>
#ifdef __LINUX_PLAT__
#include <pthread.h>
#include <sys/types.h>
#endif

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
#ifdef __LINUX_PLAT__
// => POSIX thread lock
// N/A
#endif

// Type definitions
typedef struct thrd_lock thrd_lock_t;
typedef struct thrd_lock* thrd_lock_ptr_t;
typedef enum thrd_lock_type
{
	thrd_lock_type_none, 
	thrd_lock_type_mutex, 
	thrd_lock_type_sem, 
	thrd_lock_type_prod_cons
}thrd_lock_type_t;

// Macros
// => Mutex
// N/A
// => Semphore
#define THRD_LOCK_SEM_MAX_CAPACITY	(((size_t)(~0) << 1) >> 1)
// => Producer-consumer model
#define THRD_LOCK_PROD_CONS_MAX_CAPACITY	((size_t)(~0))
#define THRD_LOCK_PROD	(true)
#define THRD_LOCK_CONS	(false)

// Functions
#define thrd_lock_init(thrd_lock_ptr, type, args ...)	(unlikely((thrd_lock_ptr) == NULL || (thrd_lock_ptr)->init_fp == NULL) ? NULL : (thrd_lock_ptr)->init_fp((thrd_lock_ptr), (type), ## args))
#define thrd_lock_exit(thrd_lock_ptr)	(unlikely((thrd_lock_ptr) == NULL || (thrd_lock_ptr)->exit_fp == NULL) ? NULL : (thrd_lock_ptr)->exit_fp((thrd_lock_ptr)))
#define thrd_lock_on(thrd_lock_ptr, args ...)	(unlikely((thrd_lock_ptr) == NULL || (thrd_lock_ptr)->on_fp == NULL) ? SYS_FAIL : (thrd_lock_ptr)->on_fp((thrd_lock_ptr), ## args))
#define thrd_lock_off(thrd_lock_ptr, args ...)	(unlikely((thrd_lock_ptr) == NULL || (thrd_lock_ptr)->off_fp == NULL) ? SYS_FAIL : (thrd_lock_ptr)->off_fp((thrd_lock_ptr), ## args))
#define thrd_lock_get_type(thrd_lock_ptr)	(unlikely((thrd_lock_ptr) == NULL) ? thrd_lock_type_none : (const thrd_lock_type_t)((thrd_lock_ptr)->type))	// NEED_TO_BE_NOTICED, the type might be modified
// => Semphore
#define thrd_lock_sem_wait(thrd_lock_ptr)	thrd_lock_on((thrd_lock_ptr))
#define thrd_lock_sem_signal(thrd_lock_ptr)	thrd_lock_off((thrd_lock_ptr))
// => Producer-consumer model
#define thrd_lock_prod_on(thrd_lock_ptr)	thrd_lock_on((thrd_lock_ptr), THRD_LOCK_PROD)
#define thrd_lock_prod_off(thrd_lock_ptr, num_of_done)	thrd_lock_off((thrd_lock_ptr), THRD_LOCK_PROD, (num_of_done))
#define thrd_lock_cons_on(thrd_lock_ptr)	thrd_lock_on((thrd_lock_ptr), THRD_LOCK_CONS)
#define thrd_lock_cons_off(thrd_lock_ptr, num_of_done)	thrd_lock_off((thrd_lock_ptr), THRD_LOCK_CONS, (num_of_done))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the thrd_lock_ptr were given as a value, a compile-time error would be given
// => Default thread lock: POSIX thread lock
#define default_thrd_lock_employ(thrd_lock_ptr)	posix_thrd_lock_employ(thrd_lock_ptr)
#define default_thrd_lock_dismiss(thrd_lock_ptr)	posix_thrd_lock_dismiss(thrd_lock_ptr)
#ifdef __LINUX_PLAT__
// => POSIX thread lock
#define posix_thrd_lock_employ(thrd_lock_ptr)	(unlikely((thrd_lock_ptr) == NULL) ? NULL : \
                                             	(((thrd_lock_ptr)->init_fp = posix_thrd_lock_init), \
                                             	((thrd_lock_ptr)->exit_fp = posix_thrd_lock_exit), \
                                             	((thrd_lock_ptr)->on_fp = posix_thrd_lock_on), \
                                             	((thrd_lock_ptr)->off_fp = posix_thrd_lock_off), \
                                             	(thrd_lock_ptr)))
#define posix_thrd_lock_dismiss(thrd_lock_ptr)	(unlikely((thrd_lock_ptr) == NULL) ? NULL : \
                                              	(((thrd_lock_ptr)->init_fp = NULL), \
                                              	((thrd_lock_ptr)->exit_fp = NULL), \
                                              	((thrd_lock_ptr)->on_fp = NULL), \
                                              	((thrd_lock_ptr)->off_fp = NULL), \
                                              	(thrd_lock_ptr)))
#endif

// Implementation
// => Abstract data type
struct thrd_lock
{
	// General variables
	// => Public
	thrd_lock_ptr_t (*init_fp) (thrd_lock_ptr_t thrd_lock_ptr, thrd_lock_type_t type, ...);
	thrd_lock_ptr_t (*exit_fp) (thrd_lock_ptr_t thrd_lock_ptr);
	int (*on_fp) (thrd_lock_ptr_t thrd_lock_ptr, ...);
	int (*off_fp) (thrd_lock_ptr_t thrd_lock_ptr, ...);
	// => Private
	thrd_lock_type_t type;
	// Individual variables which must be private
	union 
	{
#ifdef __LINUX_PLAT__
		union
		{
			// Mutex lock
			struct
			{
				pthread_mutex_t mutex;
			}mutex;

			// Semphore
			struct
			{
				ssize_t avail_rsc;
				size_t capacity;
				pthread_mutex_t cond_mutex;
				pthread_cond_t cond_var;
			}sem;

			// Producer-consumer model
			struct
			{
				size_t avail_rsc;
				size_t capacity;
				pthread_mutex_t mutex;
				pthread_mutex_t cond_mutex;
				pthread_cond_t cond_var;
			}prod_cons;
		}posix;
#endif
	}idv;
};

#ifdef __LINUX_PLAT__
// => POSIX thread lock
extern thrd_lock_ptr_t posix_thrd_lock_init (thrd_lock_ptr_t thrd_lock_ptr, thrd_lock_type_t type, ...);
extern thrd_lock_ptr_t posix_thrd_lock_exit (thrd_lock_ptr_t thrd_lock_ptr);
extern int posix_thrd_lock_on (thrd_lock_ptr_t thrd_lock_ptr, ...);
extern int posix_thrd_lock_off (thrd_lock_ptr_t thrd_lock_ptr, ...);
#endif

#endif
