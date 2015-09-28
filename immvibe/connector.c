#define LOG_TAG "ImmVibeConnector"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <cutils/log.h>

#include <immvibedaemon.h>
#include <immvibeconn.h>


int immvibe_conn_open(void)
{
	int fd;
	socklen_t len;
	struct sockaddr_un remote;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		int errsv = errno;
		ALOGE("socket creation failed: errno=%d", errsv);
		return -errsv;
	}

	ALOGV("connecting immvibe daemon");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, IMMVIBED_SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(fd, (struct sockaddr *)&remote, len) == -1) {
		int errsv = errno;
		ALOGE("connect failed: errno=%d", errsv);
		close(fd);
		return -errsv;
	}

	ALOGV("immvibe daemon connected");
	return fd;
}


static int send_internal(int fd, const char *buf, size_t len)
{
	size_t remaining = len;

	ALOGV("sending %u bytes to daemon", remaining);
	while (remaining) {
		ssize_t ret = send(fd, buf, remaining, 0);
		if (ret < 0) {
			int errsv = errno;
			ALOGE("send failed: errno=%d", errsv);
			return -errsv;
		}
		remaining -= ret;
	}

	return 0;
}


int immvibe_conn_vibrate(int fd, int duration)
{
	char buf[16];
	size_t len;
	memset(buf, 0, sizeof(buf));
	len = snprintf(buf, sizeof(buf), "+%d$", duration);
	return send_internal(fd, buf, len);
}


int immvibe_conn_stop(int fd)
{
	return send_internal(fd, "-0$", 3);
}
