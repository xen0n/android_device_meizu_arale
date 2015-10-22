#define LOG_TAG "OpenImmVibed"

#include <cutils/log.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/un.h>

#include <immvibeclient.h>
#include <immvibedaemon.h>

#include "threadqueue.h"

#define MAXEVENTS 64

#define MSG_VIBRATE 1
#define MSG_STOP    2


static struct immvibe_handle *fd;
static pthread_t worker_thread;
static struct threadqueue work_queue;


static void *worker_thread_proc(void *arg)
{
	struct threadqueue *queue = (struct threadqueue *)arg;
	struct threadmsg msg;
	int ret;

	ALOGI("worker thread started");

	while (1) {
		ret = thread_queue_get(queue, NULL, &msg);

		switch (msg.msgtype) {
			case MSG_VIBRATE: {
				int duration = (int)msg.data;
				uint8_t force = immvibe_api_get_force_userspace();
				ALOGV("worker: vibrate(duration=%d, force=%d)", duration, force);

				immvibe_play(fd, force);
				usleep(duration * 1000);  // XXX: too lazy to nanosleep and handle EINTR
				immvibe_play(fd, 0);

				break;
			}

			case MSG_STOP: {
				ALOGV("worker: stop()");
				immvibe_play(fd, 0);

				break;
			}

			default: {
				ALOGW("worker: unknown work, should never happen");
			}
		}
	}

	return NULL;
}


static void process_buf(const char *buf, int count __attribute__((unused)))
{
	int ret;

	switch (buf[0]) {
		case '+': {
			int duration;
			ret = sscanf(&buf[1], "%d$", &duration);
			if (!ret) {
				ALOGW("process_buf: malformed vibrate request");
				return;
			}

			ALOGV("process_buf: vibrate(duration=%d)", duration);
			thread_queue_add(&work_queue, (void *)duration, MSG_VIBRATE);
			break;
		}

		case '-': {
			ALOGV("process_buf: stop()");
			thread_queue_add(&work_queue, NULL, MSG_STOP);
			break;
		}

		default: {
			ALOGW("process_buf: unrecognized request '%s'", buf);
		}
	}
}


static int set_nonblock(int fd)
{
	int flags;
	int ret;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) {
		ret = errno;
		goto bail;
	}

	flags |= O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret < 0) {
		ret = errno;
		goto bail;
	}

	return 0;

bail:
	return ret;
}


static int init_socket(void)
{
	struct sockaddr_un local;
	socklen_t len;
	int fd = -1;
	int ret;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		ret = errno;
		goto bail;
	}

	local.sun_family = AF_UNIX;
	strncpy(local.sun_path, IMMVIBED_SOCK_PATH, sizeof(local.sun_path));
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	unlink(IMMVIBED_SOCK_PATH);

	if (bind(fd, (struct sockaddr *)&local, len)) {
		ret = errno;
		goto bail;
	}

	ret = set_nonblock(fd);
	if (ret) {
		goto bail;
	}

	return fd;

bail:
	if (fd >= 0) {
		close(fd);
	}

	return -ret;
}


static int process_client(int fd)
{
	int done = 0;
	size_t total_size = 0;
	char buf[512];

	while (1) {
		ssize_t count;

		count = read(fd, buf, sizeof(buf));
		if (count < 0) {
			if (errno != EAGAIN) {
				ALOGW("read errno=%d", errno);
				done = 1;
			}
			return done;
		} else if (count == 0) {
			done = 1;
			return done;
		}

		total_size += count;
		if (total_size >= 3) {
			process_buf(buf, total_size);
			done = 1;
			return done;
		}
	}
}


int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
	int ret;
	int sfd;
	int efd;

	struct epoll_event event;
	struct epoll_event *events;

	ALOGI("OpenImmVibed starting");

	ret = thread_queue_init(&work_queue);
	if (ret) {
		ALOGE("error initializing work queue: ret=%d", ret);
		return 1;
	}

	ret = pthread_create(&worker_thread, NULL, worker_thread_proc, &work_queue);
	if (ret) {
		ALOGE("error creating worker thread: ret=%d", ret);
		return 1;
	}

	fd = immvibe_open(&ret);
	if (!fd) {
		ALOGE("error opening immvibe driver: errno=%d", ret);
		return 1;
	}

	sfd = init_socket();
	if (sfd < 0) {
		ALOGE("error initializing socket: errno=%d", sfd);
		return 1;
	}

	ret = listen(sfd, SOMAXCONN);
	if (ret < 0) {
		ALOGE("error starting listening: errno=%d", errno);
		return 1;
	}

	efd = epoll_create1(0);
	if (efd < 0) {
		ALOGE("error during epoll_create: errno=%d", errno);
		return 1;
	}

	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	ret = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (ret < 0) {
		ALOGE("error during epoll_ctl: errno=%d", errno);
		return 1;
	}

	events = calloc(MAXEVENTS, sizeof(event));

	while (1) {
		int n, i;

		n = epoll_wait (efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if (events[i].events & EPOLLHUP) {
				ALOGV("fd=%d dropped connection", events[i].data.fd);
				close(events[i].data.fd);
				continue;
			}

			if ((events[i].events & EPOLLERR) || (!(events[i].events & EPOLLIN))) {
				ALOGW("epoll fd error, continuing");
				close(events[i].data.fd);
				continue;
			}

			if (sfd == events[i].data.fd) {
				while (1) {
					struct sockaddr_un in_addr;
					socklen_t in_len;
					int infd;

					in_len = sizeof(in_addr);
					infd = accept(sfd, (struct sockaddr *)&in_addr, &in_len);
					if (infd < 0) {
						if (!((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
							ALOGW("accept errno=%d, continuing", errno);
						}
						break;
					}

					ret = set_nonblock(infd);
					if (ret < 0) {
						ALOGW("set_nonblock errno=%d, dropping connection", ret);
						close(infd);
						break;
					}

					/*
					 * there may be very early data filled in, if not processed
					 * immediately the data is highly likely to be lost on next
					 * loop.
					 */
					process_client(infd);

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					ret = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
					if (ret < 0) {
						ALOGW("epoll_ctl errno=%d, dropping connection", errno);
						close(infd);
						break;
					}
				}
				continue;
			}

			if (process_client(events[i].data.fd)) {
				ALOGV("Closed connection on fd %d", events[i].data.fd);
				close(events[i].data.fd);
			}
		}
	}

	free(events);
	close(sfd);

	return 0;
}
