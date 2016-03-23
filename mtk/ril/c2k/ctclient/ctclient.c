#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include "cutils/log.h"
#include <cutils/sockets.h>
#include <cutils/properties.h>

#include <c2k_log.h>


#define LOG_TAG                 "CTCLIENT-AT"
#define BAUDRATE                B115200
#define SOCKET_CTCLIENT         "rild-ctclient"
#define SEND_BUFFER_SIZE        264
#define RECV_BUFFER_SIZE        1024
#define DEBUG_CTCLIENT
#ifdef DEBUG_CTCLIENT
#define DBG_CTCLIENT(...) LOGD(__VA_ARGS__)
#else
#define DBG_CTCLIENT(...)
#endif
static int fd_ttygs_modem = -1;
static int fd_ttygs_at = -1;
static int fd_socket = -1;
static int request_from_modem = 0;
static int preparing_manual_dial = 0;
static struct termios new;
static char send_buffer_at[SEND_BUFFER_SIZE];
static char send_buffer_modem[SEND_BUFFER_SIZE];
static char recv_buffer[RECV_BUFFER_SIZE];
static pthread_mutex_t s_write_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_signal_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_signalcond = PTHREAD_COND_INITIALIZER;
/*Initialize PC interface device*/
static int init_device(void)
{
    DBG_CTCLIENT("init_device");
    fd_ttygs_at = open("/dev/ttyRB2",O_RDWR);
    if(fd_ttygs_at == -1) {
        LOGE("can't open ttyRB2!\n");   
	LOGE("fail open ttyRB2 errno = %d",errno); 
	return -1;
    }
    fd_ttygs_modem = open("/dev/ttyRB0",O_RDWR);
    if(fd_ttygs_modem == -1) {
        LOGE("can't open ttyRB0!\n");   
	LOGE("fail open ttyRB0 errno = %d",errno); 
        return -1;
    }
    memset(&new, 0, sizeof(new));

    new.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
    new.c_cflag &= (~HUPCL); /*don't sent DTR=0 while close fd*/
    new.c_iflag = IGNPAR|ICRNL;
    new.c_oflag = 0;
    new.c_lflag = 0;
    new.c_cc[VTIME] = 0;
    new.c_cc[VMIN] = 1;

    tcflush(fd_ttygs_at, TCIFLUSH);
    tcsetattr(fd_ttygs_at, TCSANOW, &new);
    tcflush(fd_ttygs_modem, TCIFLUSH);
    tcsetattr(fd_ttygs_modem, TCSANOW, &new);
    return 0;
}
static void *send_to_pc(void *p) {
    int n = 0;
    int ret = 0;
    int ret_open = 0;
    int ret_at = 0;
    int ret_modem = 0;
    static fd_set writeatFds;
    static fd_set writemodemFds;
    struct timeval tv;


    while(1) {
        FD_ZERO(&writeatFds);
        FD_SET(fd_ttygs_at, &writeatFds);
        FD_ZERO(&writemodemFds);
        FD_SET(fd_ttygs_modem, &writemodemFds);
        memset(recv_buffer, 0, RECV_BUFFER_SIZE);
        //n = recv(fd_socket, recv_buffer, RECV_BUFFER_SIZE, 0);
        n = read(fd_socket, recv_buffer, RECV_BUFFER_SIZE);
        if(n <= 0) {
            LOGE("recv error, n = %d, errno = %d", n, errno);
            break;
        }
        if(!request_from_modem){
            DBG_CTCLIENT("AT Response from CP < %s\n", recv_buffer);
            /*Fix HANDROID#1989 : Avoid block; */
            tv.tv_sec = 3;
            tv.tv_usec = 0;
            ret = select(fd_ttygs_at+1,NULL,&writeatFds,NULL,&tv);
            if (ret == 0) {
                LOGE("ttyGS2 write select timeout");
                continue;
            } else if (ret < 0){
                LOGE("ttyGS2 write select error");
                continue;
            } 
            ret_at = write(fd_ttygs_at, recv_buffer, n);
            if(ret_at < 0) {
                LOGE("write error\n");
		DBG_CTCLIENT("%s %s %d",__FILE__,__FUNCTION__,__LINE__);
                //break;
            }
        } else {
            if(preparing_manual_dial ){
                sleep(5);
                preparing_manual_dial = 0;
        	   }
            DBG_CTCLIENT("Modem Response from CP < %s\n", recv_buffer);
            request_from_modem = 0;
            tv.tv_sec = 3;
            tv.tv_usec = 0;
            ret = select(fd_ttygs_modem+1,NULL,&writemodemFds,NULL,&tv);
            if (ret == 0) {
                LOGE("ttyGS0 write select timeout");
                continue;
            } else if (ret < 0){
                LOGE("ttyGS0 write select error");
                continue;
            } 
            ret_modem = write(fd_ttygs_modem, recv_buffer, n);
            if(ret_modem < 0) {
                LOGE("write modem port ret\n");
                //break;
            }
        }
    }

    DBG_CTCLIENT("send_to_pc return !");
    pthread_cond_signal(&s_signalcond);
    return NULL;
}

static int write_to_ril(int fd, char *buf, int num){
    int ret = 0;
    DBG_CTCLIENT("write_to_ril buf = %s",buf);
    pthread_mutex_lock(&s_write_mutex);
    ret = write(fd,buf,num);
    pthread_mutex_unlock(&s_write_mutex);
    return ret;
}

/*Read from ttyRB2 and transfer data to socket rild-ctclient*/
static void *at_to_ril(void *p) {
    int ret = 0;
    int n = 0;
    while(1) {
        char c;
        memset(send_buffer_at, 0, SEND_BUFFER_SIZE);
        for(n = 0; n < SEND_BUFFER_SIZE; n++) {
            ret = read(fd_ttygs_at, &c, 1);
            if(ret != 1) {
                LOGE("fail to get from PC");
                goto exit;
            }
            if(c==0){
                n=-1;
                memset(send_buffer_at, 0, SEND_BUFFER_SIZE);
                continue;
            }
            if(c >= 'a' && c <= 'z') {
                c += 'A' - 'a';
            }

            send_buffer_at[n] = c;
                DBG_CTCLIENT("send_buffer_at[%d] =  %c ",n,send_buffer_at[n]);
                if(c == '\n' || c == '\r')
                    break;
            }

            DBG_CTCLIENT("AT from PC > %s\n", send_buffer_at);
            char tempstr[PROPERTY_VALUE_MAX] = {0};
            property_get("gsm.sim.state", tempstr, "0");
            if(strcmp(tempstr,"READY") || !strncmp(send_buffer_at, "ATE0", 4)) {
                char *ready = "\r\nOK\r\n";
                ret = write(fd_ttygs_at, ready, 6);
                DBG_CTCLIENT("!!UIM NOT READY!!");
            } else {
                if(!strncmp(send_buffer_at, "AT^PREFMODE=", 12)) {
                    char *ready = "\r\nOK\r\n";
                    ret = write(fd_ttygs_at, ready, 6);
                }
                n = write_to_ril(fd_socket,send_buffer_at, n+1);
                if(n < 0) {
                    LOGE("fail send to ril (%d)\n", n);
                    break;
                }
            }
        }
exit:
    DBG_CTCLIENT("at_to_ril return !");
    pthread_cond_signal(&s_signalcond);
    return NULL;
}

/*Read from ttyRB0 and transfer data to socket rild-ctclient*/
static void *modem_to_ril(void *p) {
    int ret = 0;
    int n = 0;
    while(1) {
        char c;

        memset(send_buffer_modem, 0, SEND_BUFFER_SIZE);

        for(n = 0; n < SEND_BUFFER_SIZE; n++) {
            ret = read(fd_ttygs_modem, &c, 1);
            request_from_modem = 1;
            if(ret != 1) {
                LOGE("fail to get from PC");
                goto exit;
            }
            if(c==0){
                n=-1;
                memset(send_buffer_modem, 0, SEND_BUFFER_SIZE);
                continue;
            }
            if(c >= 'a' && c <= 'z') {
                c += 'A' - 'a';
            }

            send_buffer_modem[n] = c;
            DBG_CTCLIENT("send_buffer_modem[%d] =  %c ",n,send_buffer_modem[n]);
            if ((('\n' == send_buffer_modem[2]) || ('\r' == send_buffer_modem[2]))
                && !strncmp(send_buffer_modem, "AT", 2)) {
                send_buffer_modem[2] = 0;
            }
            if(c == '\n' || c == '\r') {
                break;
            }
        }

            DBG_CTCLIENT("Modem from PC > %s\n", send_buffer_modem);
            if(strncmp(send_buffer_modem, "ATD",3) == 0){
                DBG_CTCLIENT("prepareing manual dial ");
                preparing_manual_dial = 1;
            }
            if ( !strncmp(send_buffer_modem, "ATE", 3) ||!strncmp(send_buffer_modem, "ATZ", 3) 
                ||!strncmp(send_buffer_modem, "ATQ", 3) || !strncmp(send_buffer_modem, "ATS", 3)
                ||!strncmp(send_buffer_modem, "ATH", 3) || !strcmp(send_buffer_modem, "AT")) {
                char *ready = "\r\nOK\r\n";
                ret = write(fd_ttygs_modem, ready, 6);
                if (ret < 0 ) {
                    LOGD("fail write OK to fd_ttygs_modem, ret = %d, errno = %d",
                        ret, errno);
                }
            } else {
                n = write_to_ril(fd_socket,send_buffer_modem, n+1);
                if(n < 0) {
                    LOGE("fail send to ril (%d)\n", n);
                    break;
                }
            }
        }
exit:
    DBG_CTCLIENT("modem_to_ril return !");
    pthread_cond_signal(&s_signalcond);
    return NULL;
}
int main(int argc, char *argv[]) {
    int n, ret;
    pthread_attr_t attr;
    pthread_t tid;
    pthread_attr_t attr_modem;
    pthread_attr_t attr_at;
    pthread_t tid_modem;
    LOGD("enter main: suffix Version: %s", VIA_SUFFIX_VERSION);
    //int pid;
    ret = init_device();
    if(ret < 0){
        goto exit;
    }
    fd_socket = socket_local_client(SOCKET_CTCLIENT, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
    if (fd_socket < 0) {
        LOGE("failed to get socket '" SOCKET_CTCLIENT "' (%d)", fd_socket);
        goto exit;
    }
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&tid, &attr, send_to_pc, &attr);
    if(ret < 0) {
        LOGE("fail to create pthread (%d)\n", ret);
        goto exit;
    }
    pthread_attr_init(&attr_modem);
    pthread_attr_setdetachstate(&attr_modem, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&tid_modem, &attr_modem, modem_to_ril, &attr_modem);
    if(ret < 0) {
        LOGE("fail to create pthread (%d)\n", ret);
        goto exit;
    }
    pthread_attr_init(&attr_at);
    pthread_attr_setdetachstate(&attr_at, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&tid_modem, &attr_at, at_to_ril, &attr_at);
    if(ret < 0) {
        LOGE("fail to create pthread (%d)\n", ret);
        goto exit;
    }
    LOGD("before cond wait");
    ret = pthread_cond_wait(&s_signalcond, &s_signal_mutex);
    LOGD("after cond wait");

exit:
    LOGD("ctclient main exit !");
    LOGD("leave main: suffix Version: %s", VIA_SUFFIX_VERSION);
    if(fd_socket >= 0)
    {
        close(fd_socket);
        fd_socket = -1;
    }
    if(fd_ttygs_at >= 0)
    {
        close(fd_ttygs_at);
        fd_ttygs_at = -1;
    }
    if(fd_ttygs_modem >= 0)
    {
        close(fd_ttygs_modem);
        fd_ttygs_modem = -1;
    }
    
    return 0;
}
