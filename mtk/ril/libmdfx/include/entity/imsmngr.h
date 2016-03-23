#ifndef __LIBMDFX_ENTITY_IMSMNGR_H__
#define __LIBMDFX_ENTITY_IMSMNGR_H__

typedef struct _imsmngr_req {
    unsigned int        request_id;
    unsigned int        request_len;
    char                request_data[0];
} imsmngr_req_t;

typedef struct _imsmngr_resp {
    unsigned int        result;
    unsigned int        response_id;
    unsigned int        response_len;
    char                response_data[0];
} imsmngr_resp_t;

typedef struct _imsmngr_notify {
    unsigned int        notify_id;
    unsigned int        notify_len;
    char                notify_data[0];
} imsmngr_notify_t;


#endif /* __LIBMDFX_ENTITY_IMSMNGR_H__ */
