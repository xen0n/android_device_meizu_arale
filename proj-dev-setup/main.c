#define LOG_TAG "proj_dev_setup"

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <cutils/properties.h>
#include <cutils/log.h>
#include <private/android_filesystem_config.h>


static void s_mknod(const char *pathname, mode_t type, unsigned int maj, unsigned int min)
{
	mknod(pathname, type, makedev(maj, min));
}


static void s_mkdirp(const char *pathname)
{
	char buf[PATH_MAX];
	char currpath[PATH_MAX], *pathpiece;
	struct stat st;
	int ret;

	// buffer for strtok
	// hopefully the strings are not too long
	strncpy(buf, pathname, PATH_MAX);

	// reset path
	strcpy(currpath, "");
	// create the pieces of the path along the way
	pathpiece = strtok(buf, "/");
	if(buf[0] == '/') {
		// prepend / if needed
		strcat(currpath, "/");
	}
	while(pathpiece != NULL) {
		if(strlen(currpath) + strlen(pathpiece) + 2/*NUL and slash*/ > PATH_MAX) {
			ALOGE("Invalid path specified: too long");
			return;  // return 1;
		}
		strcat(currpath, pathpiece);
		strcat(currpath, "/");
		if(stat(currpath, &st) != 0) {
			ret = mkdir(currpath, 0777);
			if(ret < 0) {
				ALOGE("mkdir failed for %s, %s\n", currpath, strerror(errno));
				return;  // return ret;
			}
		}
		pathpiece = strtok(NULL, "/");
	}
}


int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	ALOGI("starting");

#include "setup.c"

	ALOGI("exiting");

	return 0;
}
