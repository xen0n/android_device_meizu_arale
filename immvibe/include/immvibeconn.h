#ifndef _IMMVIBECONN_H_
#define _IMMVIBECONN_H_

int immvibe_conn_open(void);
int immvibe_conn_vibrate(int fd, int duration);
int immvibe_conn_stop(int fd);

#endif
