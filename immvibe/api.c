#include <stdio.h>
#include <errno.h>

#include "immvibeclient.h"


#define STRENGTH_STORE_PATH "/data/.libimmvibeclient_strength"
#define DEFAULT_STRENGTH    96  /* 75% of 127 */


uint8_t immvibe_api_get_strength_userspace(void)
{
	FILE *fp = NULL;
	unsigned int result;
	int ret;

	fp = fopen(STRENGTH_STORE_PATH, "r");
	if (!fp) {
		/* TODO: log */
		/* return the default value */
		return DEFAULT_STRENGTH;
	}

	ret = fscanf(fp, "%u\n", &result);
	if (ret == EOF || ret < 1) {
		/* TODO: log */
		result = DEFAULT_STRENGTH;
	}

	fclose(fp);
	return result;
}


int immvibe_api_set_strength_userspace(uint8_t strength)
{
	FILE *fp = NULL;
	int ret;

	fp = fopen(STRENGTH_STORE_PATH, "w");
	if (!fp) {
		int errsv = errno;

		/* TODO: log */
		return errsv;
	}

	ret = (int) fprintf(fp, "%u\n", strength);

	fclose(fp);
	return ret;
}
