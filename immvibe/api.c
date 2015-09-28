#include <stdio.h>
#include <errno.h>

#include "immvibeclient.h"


#define FORCE_STORE_PATH "/data/.libimmvibeclient_force"
#define DEFAULT_FORCE    96  /* 75% of 127 */


uint8_t immvibe_api_get_force_userspace(void)
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


int immvibe_api_set_force_userspace(uint8_t force)
{
	FILE *fp = NULL;
	int ret;

	fp = fopen(FORCE_STORE_PATH, "w");
	if (!fp) {
		int errsv = errno;

		/* TODO: log */
		return errsv;
	}

	ret = (int) fprintf(fp, "%u\n", force);

	fclose(fp);
	return ret;
}
