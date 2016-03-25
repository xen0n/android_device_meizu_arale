#ifndef __WO_CFG_H__
#define __WO_CFG_H__

#include <stdbool.h>

typedef union cust_value_t {
	int  integer;
	char *str;
	bool boolean;
} cust_value_t;


typedef struct cust_setting_t {
	char* system_property_key;
	cust_value_t default_value;
} cust_setting_t;

typedef enum cust_setting_type_t {
	SETTING_START,
	CUST_PCSCF_IP4_VALUE,
	CUST_PCSCF_IP6_VALUE,
    FORCE_TSI_64,
    USE_CFG_VIP,
	SETTING_END
} cust_setting_type_t;

static cust_setting_t cust_settings[SETTING_END] = {
	[CUST_PCSCF_IP4_VALUE] = {"persist.net.wo.cust_pcscf_4",  {.integer = 20}},
	[CUST_PCSCF_IP6_VALUE] = {"persist.net.wo.cust_pcscf_6",  {.integer = 21}},
	[FORCE_TSI_64]         = {"persist.net.wo.force_tsi_64",  {.boolean = false}},
	[USE_CFG_VIP]          = {"persist.net.wo.use_cfg_vip",   {.boolean = false}}
};

int get_cust_setting(cust_setting_type_t type, char *value);
bool get_cust_setting_bool(cust_setting_type_t type);
int get_cust_setting_int(cust_setting_type_t type);

static inline const char* get_key(cust_setting_type_t type);
static inline cust_value_t get_default(cust_setting_type_t type);

#endif
