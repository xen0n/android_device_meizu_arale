#include <stdio.h>
#include <unistd.h>

#include <immvibeconn.h>


int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
	int fd;
	int ret;

	fd = immvibe_conn_open();
	if (fd < 0) {
		fprintf(stderr, "immvibe daemon connection failed: errno=%d\n", fd);
		return 1;
	}

	ret = immvibe_conn_vibrate(fd, 500);
	if (ret < 0) {
		fprintf(stderr, "vibrate command send failed: errno=%d\n", ret);
		ret = 1;
	}

	close(fd);

	return ret;
}
