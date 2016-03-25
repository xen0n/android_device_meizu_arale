#ifdef __ANDROID__
#include <cutils/properties.h>
#endif

#include <stdlib.h>
#include "cust_settings.h"
#include "settings.h"
#include "settings.h"
#include "debug.h"

int get_cust_setting(cust_setting_type_t type, char* value) {
	const char *key = get_key(type);
	const char *default_value = get_default(type).str;
	int len = 0;
#ifdef __ANDROID__
	if (key) {
		len = property_get(key, value, default_value);
	} else if (default_value) {
		len = strlen(default_value);
		strcpy(value, default_value);
	}
#else
	if (default_value) {
		len = strlen(default_value);
		strcpy(value, default_value);
	}
#endif
	return 0;
}

bool get_cust_setting_bool(cust_setting_type_t type) {
	const char *key = get_key(type);
	bool ret  = get_default(type).boolean;
#ifdef __ANDROID__
	if (key) {
		ret = property_get_bool(key, ret);
	}
#endif
	return ret;
}

int get_cust_setting_int(cust_setting_type_t type) {
	const char *key = get_key(type);
	int  ret  = get_default(type).integer;
#ifdef __ANDROID__
	if (key) {
		char tmp[PROP_VALUE_MAX];
		if(__system_property_get(key, tmp)) {
			ret = atoi(tmp);
		}
	}
#endif
	return ret;
}

static inline const char* get_key(cust_setting_type_t type) {
	return cust_settings[type].system_property_key;
}

static inline cust_value_t get_default(cust_setting_type_t type) {
	return cust_settings[type].default_value;
}
