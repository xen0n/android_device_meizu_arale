#ifndef DATAMNGR_DEBUG_H
#define DATAMNGR_DEBUG_H 
#include "mdfx.h"

extern unsigned dm_debug_lv;

#define dm_print(tag, format, args ...)     do { \
	if ((dm_debug_lv & tag) && tag == ERR_LV) { \
		mdfx_print((tag), "[DMGR-ERR] "format, ## args); \
	} \
	else if ((dm_debug_lv & tag) && tag == WARN_LV) { \
		mdfx_print((tag), "[DMGR-WAR] "format, ## args); \
	} \
	else if ((dm_debug_lv & tag) && tag == INFO_LV) { \
		mdfx_print((tag), "[DMGR-INF] "format, ## args); \
	} \
	else if ((dm_debug_lv & tag) && tag == DEBUG_LV) { \
		mdfx_print((tag), "[DMGR-DBG] "format, ## args); \
	} \
} while (0)


#endif
