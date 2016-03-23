#ifndef SIMMNGR_IF_H
#define SIMMNGR_IF_H

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


typedef struct _sim_req_info_ {
    int sim_id;// for select sim, phone : from imsa, dongle : only 1?
    int sim_type;// 0 : usim, 1 : isim
    int request;
    int ch_id;// isim only
    int next_handle;// need next cmd handle
    int rec_num;// store rec num for linear fixed
    int rec_idx;// store rec idx for linear fixed
    int rec_size;// store rec size for linear fixed
} sim_req_info_t;

typedef struct _sim_aka_info_ {
    int sim_id;// for select sim, phone : from imsa, dongle : only 1?
    int sim_type;// 0 : usim, 1 : isim
    int ch_id;// isim only
    int rand_len;
    char rand[32];
    int autn_len;
    char autn[32];
} sim_aka_info_t;

typedef struct _sim_aka_result_ {
    int result;
    int response_len;
    char response[256];
} sim_aka_result_t;

typedef struct _sim_type_info_ {
    int result;
    int sim_type;// 0 : usim, 1 : isim
    int ch_id;// isim only
} sim_type_info_t;

typedef struct _sim_imsi_result_ {
    int result;
    int imsi_len;
    char imsi[16];
} sim_imsi_result_t;

typedef struct _sim_mnc_len_result_ {
    int result;
    int mnc_len;
} sim_mnc_len_result_t;

typedef struct _sim_psismsc_result_ {
    int result;
    int psismsc_len;
    unsigned char psismsc[256];
} sim_psismsc_result_t;

typedef struct _sim_impu_result_ {
    int result;
    int impu_num;
    int impu_len[5];
    unsigned char impu[5][256];
} sim_impu_result_t;

typedef struct _sim_smsp_result_ {
    int result;
    int smsp_len;
    unsigned char smsp[64];
} sim_smsp_result_t;

typedef struct _sim_impi_result_ {
    int result;
    int impi_len;
    unsigned char impi[256];
} sim_impi_result_t;

typedef struct _sim_domain_result_ {
    int result;
    int domain_len;
    unsigned char domain[256];
} sim_domain_result_t;

typedef struct _sim_service_table_result_ {
    int result;
    int service_table_len;
    unsigned char service_table[256];
} sim_service_table_result_t;

typedef struct _sim_pcscf_result_ {
    int result;
    int pcscf_num;
    int pcscf_len[20];// V4 + V6
    unsigned char   pcscf[20][128];// V4 + V6
} sim_pcscf_result_t;

typedef struct _sim_state_result_ {
    int result;
    int state;
} sim_state_result_t;

typedef struct _sim_data_notify {
    unsigned int sim_id;
    unsigned int data;
} sim_data_notify_t;

typedef struct {
	char name[32];
} sim_user_checkin_info_t;

typedef struct _sim_insert_check {
    unsigned int sim_id;
    unsigned int insert_flag;
} sim_insert_check_t;

typedef struct {
    int result;
    int error_code;
} sim_ims_func_result_t;

typedef struct _sim_imei_result_ {
    int result;
    int imei_len;
    char imei[16];
} sim_imei_result_t;

typedef struct _sim_stk_notify_ {
	unsigned int sim_id;
    int result;// 0: fail, 1: start, 2: end
    int cmd_type;
	int cmd_qualifier;
    unsigned char aid[32];// string
	unsigned int efid_num;// max 256
    unsigned int efid[256];// hex
} sim_stk_notify_t;


typedef int (*simmngr_sim_insert_state_notify_fn)(sim_data_notify_t *sim_state, void *priv);
typedef int (*simmngr_sim_usim_detect_notify_fn)(sim_data_notify_t *usim_detect, void *priv);
typedef int (*simmngr_sim_isim_ch_notify_fn)(sim_data_notify_t *isim_ch, void *priv);
typedef int (*simmngr_sim_insert_check_notify_fn)(sim_data_notify_t *insert_check, void *priv);
typedef int (*simmngr_sim_stk_refresh_notify_fn)(sim_stk_notify_t *stk_notify, void *priv);
typedef struct _mal_simmngr_notify_function_ {
    simmngr_sim_insert_state_notify_fn          sim_insert_state;
    simmngr_sim_usim_detect_notify_fn           sim_usim_detect;
    simmngr_sim_isim_ch_notify_fn               sim_isim_ch;
    simmngr_sim_insert_check_notify_fn          sim_insert_check;
    simmngr_sim_stk_refresh_notify_fn			sim_stk_refresh;
} mal_simmngr_notify_function_t;


extern mal_simmngr_notify_function_t mal_simmngr_notify;


int mal_query_csq(void *conn_ptr, int *rssi, int *ber);
int mal_sim_notify_checkin(void *conn_ptr, sim_user_checkin_info_t *user_checkin_info);
int mal_sim_notify_checkout(void *conn_ptr, char *name, int name_len);
int mal_check_sim_type(void *conn_ptr, int sim_id, int *sim_type, int *ch_id);
int mal_check_sim_state(void *conn_ptr, int sim_id, int *sim_state);
int mal_query_sim_info(void *conn_ptr, sim_req_info_t *req_info, char *out_data, int *out_len);
int mal_sim_aka_verify(void *conn_ptr, sim_aka_info_t *aka_info, char *response, int *response_len);
int mal_ims_func_enable(void *conn_ptr, int *error_code);
int mal_ims_func_disable(void *conn_ptr, int *error_code);
int mal_sim_insert_check(void *conn_ptr, int sim_id);
int mal_simmngr_register(mal_simmngr_notify_function_t *callback_fn);

#endif

