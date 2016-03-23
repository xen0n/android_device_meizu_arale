#ifndef __LIBMDFX_ENTITY_NWMNGR_H__
#define __LIBMDFX_ENTITY_NWMNGR_H__

/* ------------------------------------ */
/*  Network Manager                     */
/* ------------------------------------ */

typedef struct _nwmngr_ps_reg_state {
    unsigned int            cmd_result;
    unsigned int            n;
    unsigned int            state;
    char                    lac[8];
    char                    ci[12];
    unsigned int            Act;
    char                    rac[4];
    unsigned int            cause_type;
    unsigned int            reject_cause;
    
    unsigned int            wifi_state;
    
    char                    raw_data[128];
} nwmngr_ps_reg_state_t;


#endif /* __LIBMDFX_ENTITY_NWMNGR_H__ */
