#ifndef APN_TABLE_H
#define APN_TABLE_H

#include "datamngr_def.h"

#ifdef APN_TABLE_SUPPORT

#include "datamngr_debug.h"
#define APN_NI_SIZE	32
#define APN_BEARER_SIZE 32
#define MAX_APN_ENTRY	16

#ifdef VZW_REQ_SUPPORT

#define VZW_APN_VERSION 20150729
#define LTE_BEARER "LTE"
#define VZW_IMS_NI "VZWIMS"
#define VZW_ADMIN_NI "VZWADMIN"
#define VZW_INTERNET_NI "VZWINTERNET"
#define VZW_APP_NI "VZWAPP"

#define VZW_APN_FILE "/data/.vzw_apn_info.raw"

typedef enum {
    VZW_IMS_CLASS = 1, 
    VZW_ADMIN_CLASS = 2,
    VZW_INTERNET_CLASS =3,
    VZW_APP_CLASS = 4,
} vzw_apn_class_t; 

#endif

typedef enum {
	MODE_NORMAL,
	MODE_ROAMING,
	MODE_IMS_TEST,		
	MODE_NUM
} apn_mode_t;


typedef struct apn_table_entry {
	
	unsigned apn_class;
	char ni[APN_NI_SIZE];
	char bearer[APN_BEARER_SIZE];
	unsigned ip_ver;
	unsigned enable;
	unsigned time;
	unsigned max_conn;
	unsigned max_conn_t;
	unsigned wait_time;
	unsigned throttle_time;
	
} apn_table_entry_t;

typedef struct {
	unsigned apn_class;
	char ni[APN_NI_SIZE];
	char bearer[APN_BEARER_SIZE];
	unsigned ip_ver;
	unsigned enable;
	unsigned time;
} apn_info_t;


typedef struct {
	unsigned max_conn;
	unsigned max_conn_t;
	unsigned wait_time;
	unsigned throttle_time;
} apn_para_t;


typedef struct apn_table {
	unsigned version;
	unsigned num;
	apn_mode_t mode;
	unsigned char active[MAX_APN_ENTRY];
	apn_table_entry_t table[MAX_APN_ENTRY];

} apn_table_t;


typedef struct apn_ops
{
	int (*reset)();
	int (*init)();
	int (*store)();
	int (*restore)();
	int (*set_mode)(apn_mode_t);
	int (*set_apn)(apn_table_entry_t *);
	int (*edit_apn)(unsigned, apn_info_t *);
	int (*edit_apn_para)(unsigned, apn_para_t *);
	int (*get_apn_by_class)(apn_table_entry_t *, unsigned);
	int (*get_apn_by_index)(apn_table_entry_t *, unsigned);
	int (*get_apn_by_name)(apn_table_entry_t *, char *);
	int (*get_init_apn)(apn_table_entry_t *);
	int (*get_ims_apn)(apn_table_entry_t *);
	int (*get_internet_apn)(apn_table_entry_t *);
	void (*dump_entry)(unsigned);
	void (*dump)();
	unsigned (*apn_num)();

} apn_ops_t;


extern apn_ops_t * get_apn_handle();

#endif
#endif
