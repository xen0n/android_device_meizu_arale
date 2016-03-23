#ifndef SIMMNGR_STRUCT_H
#define SIMMNGR_STRUCT_H

#include "mdfx.h"

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

#endif

