#ifndef __LIBMDFX_ENTITY_MDMNGR_H__
#define __LIBMDFX_ENTITY_MDMNGR_H__

/* ------------------------------------ */
/*  Modem Manager                       */
/* ------------------------------------ */

typedef struct _mdmngr_atcmd_req {
    void               *priv_data;
    unsigned int        timeout;
    char                resp_prefix[16];
    char                cmd[512];
} mdmngr_atcmd_req_t;

enum ATCMD_RESULT_e {
    ATCMD_RESULT_OK = 0,
    ATCMD_RESULT_CMD_ERROR,
    ATCMD_RESULT_TIMEOUT,
    ATCMD_RESULT_NOT_SUPPORT,
    ATCMD_RESULT_DATA_NULL,
    ATCMD_RESULT_CONNECTION_DISCONNECT,
    ATCMD_RESULT_RIL_ERROR,
};

typedef struct _mdmngr_atcmd_resp {
    void               *priv_data;
    unsigned int        result;
    char                data[4096];
} mdmngr_atcmd_resp_t;

typedef struct _mdmngr_aturc_register_req {
    char                urc_prefix[512];
} mdmngr_aturc_register_req_t;

enum ATURC_TYPE_e {
    ATURC_ESIMS,
    ATURC_EUSIM,
    ATURC_ESIMAPP,
    ATURC_ECSQ,
    ATURC_CREG,
    ATURC_CGREG,
    ATURC_CEREG,
    ATURC_CIREPI,
    ATURC_STKPCI,
};

typedef struct _mdmngr_aturc_notify {
    int                 type;
    char                data[512];
} mdmngr_aturc_notify_t;


enum MDMNGR_STATE_e {
    MDMNGR_STATE_READY = 0,
    MDMNGR_STATE_OOS,
};

typedef struct _mdmngr_state {
    int                 state;
} mdmngr_state_t;

typedef struct _rilproxy_atcmd_req {
    void               *priv_data;
    unsigned int        channel;
    char               *cmd;
} rilproxy_atcmd_req_t;

typedef mdmngr_atcmd_resp_t rilproxy_atcmd_resp_t;

typedef struct _rilproxy_ril_connect_req {
    char                cmd_socket_name[32];
    char                urc_socket_name[32];
} rilproxy_ril_connect_req_t;

typedef struct _rilproxy_ril_resp {
    int                 result;
} rilproxy_ril_resp_t;

#endif /* __LIBMDFX_ENTITY_MDMNGR_H__ */
