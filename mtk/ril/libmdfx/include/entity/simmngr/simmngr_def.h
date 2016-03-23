#ifndef SIMMNGR_DEF_H
#define SIMMNGR_DEF_H

#include "mdfx.h"

#define IGNORE_CHAR(s,c) {while ((*(s)) == (c) && *(s)) (s)++;}
#define FIND_CHAR(s,c) {while ((*(s)) != (c) && *(s)) (s)++;}

// request
typedef enum _sim_query_request_{
    USIM_QUERY_IMSI = 0,
    USIM_QUERY_MNC_LEN,
    USIM_QUERY_PSISMSC,
    USIM_QUERY_SMSP,
    ISIM_QUERY_IMPI,
    ISIM_QUERY_PSISMSC,
    ISIM_QUERY_IMPU,
    ISIM_QUERY_DOMAIN,
    ISIM_QUERY_SERVICE_TABLE,
    ISIM_QUERY_PCSCF,
    SIM_QUERY_IMEI,
} sim_query_request_e;


// sim type
#define TYPE_USIM       0
#define TYPE_ISIM       1

typedef enum _sim_state_{
    SIM_STATE_NOTIFY_NOT_READY = 0,
    SIM_STATE_NOTIFY_READY = 1,
    SIM_STATE_NOTIFY_REFRESH = 2,
} sim_notify_state_e;

typedef enum _sim_insert_state_{
    SIM_INSERT_PLUG_OUT = 0,
    SIM_INSERT_PLUG_IN,
    SIM_INSERT_ABNORMAL,
} sim_insert_state_e;

typedef enum _sim_check_state_{
    SIM_CHECK_OK = 0,
	SIM_CHECK_FAIL,
    SIM_CHECK_INIT,
    SIM_CHECK_RETRY,
    SIM_CHECK_NOT_INSERT,
} sim_check_state_e;

typedef enum _sim_stk_cmd_type_{
    CMD_TYPE_INVALID = 0,
	CMD_TYPE_REFRESH,
} sim_stk_cmd_type_e;

typedef enum _sim_refresh_cmd_{
	CMD_SIM_INIT_FULL_FILE_CHANGE  = 0,// SIM Initialization and Full File Change Notification
	CMD_SIM_FILE_UPDATE,// A file on SIM has been updated
	CMD_SIM_INIT_FILE_CHANGE,// SIM Initialization and File Change Notification
	CMD_SIM_INIT,// SIM initialized, All files should be re-read
	CMD_SIM_RESET,// SIM reset.  SIM power required, SIM may be locked and all files should be re-read 
} sim_refresh_cmd_e;

#endif

