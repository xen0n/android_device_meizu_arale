#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <cutils/properties.h>

#include "immvibeclient.h"


#define FORCE_STORE_PATH "/data/.libimmvibeclient_force"
#define FORCE_STORE_PROP "persist.openimmvibe.vibe_force"
#define DEFAULT_FORCE    32  /* 25% of 127 */



static uint8_t get_force_userspace_legacy(void)
{
	FILE *fp = NULL;
	unsigned int result;
	int ret;

	fp = fopen(FORCE_STORE_PATH, "r");
	if (!fp) {
		/* TODO: log */
		/* return the default value */
		return DEFAULT_FORCE;
	}

	ret = fscanf(fp, "%u\n", &result);
	if (ret == EOF || ret < 1) {
		/* TODO: log */
		result = DEFAULT_FORCE;
	}

	fclose(fp);
	return result;
}


static void migrate_force_store(uint8_t force)
{
	int ret = immvibe_api_set_force_userspace(force);
	if (ret) {
		/*
		 * Whoops, property write failed!
		 * Shouldn't continue deleting the known-good legacy store as that
		 * would result in user preference loss.
		 */
		/* TODO: log */
		return;
	}

	/* Remove the legacy store */
	unlink(FORCE_STORE_PATH);
}


uint8_t immvibe_api_get_force_userspace(void)
{
	int ret;

	ret = access(FORCE_STORE_PATH, F_OK);
	if (ret) {
		/* Legacy store inaccessible, use new property-based store */
		return (uint8_t) property_get_int32(FORCE_STORE_PROP, DEFAULT_FORCE);
	}

	/* Legacy store detected, migrate before returning */
	uint8_t result = get_force_userspace_legacy();
	migrate_force_store(result);
	return result;
}


int immvibe_api_set_force_userspace(uint8_t force)
{
	char buf[16];
	snprintf(buf, sizeof(buf), "%u", force);
	return property_set(FORCE_STORE_PROP, buf);
}
