#define LOG_TAG "asec_helper"

#include <stdlib.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/mount.h>

#include <cutils/fs.h>
#include <cutils/log.h>

#include <private/android_filesystem_config.h>

#define EMULATED_SD_MOUNTPOINT "/data/media"
#define SEC_ASECDIR_EXT        "/mnt/secure/asec"


// This is Volume::mountAsecExternal(), modified to help mount "external" asec
// on emulated sd.
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    char legacy_path[PATH_MAX];
    char secure_path[PATH_MAX];

    snprintf(legacy_path, PATH_MAX, "%s/android_secure", EMULATED_SD_MOUNTPOINT);
    snprintf(secure_path, PATH_MAX, "%s/.android_secure", EMULATED_SD_MOUNTPOINT);

    // Recover legacy secure path
    if (!access(legacy_path, R_OK | X_OK) && access(secure_path, R_OK | X_OK)) {
        if (rename(legacy_path, secure_path)) {
            SLOGE("Failed to rename legacy asec dir (%s)", strerror(errno));
        }
    }

    if (fs_prepare_dir(secure_path, 0770, AID_MEDIA_RW, AID_MEDIA_RW) != 0) {
        SLOGW("fs_prepare_dir failed: %s", strerror(errno));
        return -1;
    }

    if (mount(secure_path, SEC_ASECDIR_EXT, "", MS_BIND, NULL)) {
        SLOGE("Failed to bind mount points %s -> %s (%s)", secure_path,
                SEC_ASECDIR_EXT, strerror(errno));
        return -1;
    }

    return 0;
}


/* vim:set ai et ts=4 sw=4 sts=4 fenc=utf-8: */
