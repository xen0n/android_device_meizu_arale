#ifndef DATAMNGR_OP_HANDLER_H
#define DATAMNGR_OP_HANDLER_H


typedef struct {
    char *prefix;
    int (*handler)(char *, event_t *);
    int hijack;
} dm_urc_handle_t;


#if 1
#define RILD_REQ_IF_ID(if_id) (if_id)
#define IF_ID_FROM_RILD_REQ(if_id) (if_id)
#else
// interface id of RILD setup data call request = actul interface id + 1
#define RILD_REQ_IF_ID(if_id) (if_id + 1)
#define IF_ID_FROM_RILD_REQ(if_id) (if_id - 1)
#endif

#define INVALID_IF_ID            (-1)
#define IMS_IF_ID                   (4) // IMS network id
#define EMERGENCY_IF_ID      (5) // Emergency network id

#ifdef VZW_REQ_SUPPORT

#define ADMIN_IF_ID              (0) // VZWADMIN class (1)  - 1 
#define INTERNET_IF_ID         (2) // VZWINTERNET class (3)  - 1 
#define APP_IF_ID                  (3) // VZWAPP class (4)  - 1 

#define OP_HANDLE_SUPPORT
#endif

typedef enum {
    OP_FAIL = -1,
    OP_SUCC = 0,
    OP_REJECT,    
    OP_SKIP,
} op_rval_t;


typedef struct dm_op_handle
{
    int (*init)();
    int (*before_radio_power_on)();
    int (*before_radio_power_off)();
    int (*init_apn_done)();
    int (*set_init_apn)();
    int (*req_apn_conn)(dm_req_setup_data_call_t *, event_t *);
    int (*resp_apn_conn)(dm_resp_setup_data_call_t *, event_t *);
    int (*req_apn_disconn)(dm_req_deactivate_data_call_t *, event_t *);
    int (*urc_apn_disconn)(int, event_t *event);
    int (*set_apn)(int);
    int (*set_apn_misc)(int);
    int (*get_urc_num)();
    int (*get_urc_prefix)(int, char *, int *);
    int (*handle_urc)(char *, event_t *);
    int (*ra_init_error)(int);
    int (*ra_refresh_error)(int);
    int (*no_pcscf)(int);
    //int (*conn_auto_apn)();
    void (*dump)();

} dm_op_handle_t;


#ifdef OP_HANDLE_SUPPORT
#define EXEC_OP_HANDLE(ret, func, args ...) do { \
    dm_op_handle_t *op_handle =  get_op_handle(); \
    if (op_handle && op_handle->func) { \
        ret = op_handle->func(## args); \
    } \
    else { \
        dm_print(ERR_LV, "Null op handle!!\n"); \
        ret = SYS_FAIL; \
    } \
} while (0)

#else

#define EXEC_OP_HANDLE(func, args ...) 
#endif

#ifdef VZW_REQ_SUPPORT
extern dm_op_handle_t * get_op_handle();
#else
static dm_op_handle_t * get_op_handle() {
    return NULL;
}
#endif

#endif
