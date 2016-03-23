#ifndef DATAMNGR_UTIL_H
#define DATAMNGR_UTIL_H
#include "datamngr_debug.h"

extern unsigned dm_debug_lv;

#define DM_MUTEX_LOCK(mutex) do { \
	dm_print(DEBUG_LV, "%s mutex lock\n", __FUNCTION__); \
	if (mutex == NULL) { \
		mutex = (thrd_lock_t *) malloc(sizeof(thrd_lock_t)); \
		mdfx_mutex_init(mutex); \
	} \
	mdfx_mutex_lock(mutex); \
} while (0)


#define DM_MUTEX_UNLOCK(mutex) do { \
	dm_print(DEBUG_LV, "%s mutex unlock\n", __FUNCTION__); \
	if (mutex == NULL) { \
		dm_print(ERR_LV, "%s mutex lock before lock\n", __FUNCTION__); \
	} \
	else { \
		mdfx_mutex_unlock(mutex); \
	} \
} while (0)

#endif


