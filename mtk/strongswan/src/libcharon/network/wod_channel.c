#include "wod_channel.h"

#include <daemon.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#ifdef ANDROID
#include <cutils/sockets.h>

#define	MAX_WOD_MSG_LEN		512
#define	WOD_TCP_TIMEOUT		10

int wod_tcp_tx(char* txbuf, int txlen)
{
	int sockfd = 0, ret;
	struct sockaddr_in servaddr,cliaddr;
	struct timeval timeout;      
	timeout.tv_sec = WOD_TCP_TIMEOUT;
	timeout.tv_usec = 0;
	char inrxbuf[MAX_WOD_MSG_LEN] = {0};

	DBG1(DBG_IKE, "create socket");

	sockfd = socket_local_client("wod_ipsec", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
	if (sockfd < 0) {
		DBG1(DBG_IKE, "create android socket failed : %d !!!", sockfd);
		return -2;
	}

	DBG1(DBG_IKE, "tx... len: %d", txlen);
	ret = send(sockfd, txbuf, txlen, 0);
	if (ret < 0) {
		DBG1(DBG_IKE, "TX failed ... !!!");
		return -1;
	}

	DBG1(DBG_IKE, "setsockopt rx timeout");
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		DBG1(DBG_IKE, "setsockopt failed");
	}

	DBG1(DBG_IKE, "rx...");
	ret = recv(sockfd, inrxbuf, MAX_WOD_MSG_LEN-1, 0);
	if (ret < 0) {
		DBG1(DBG_IKE, "RX failed ... !!!");
		return -1;
	}	

	DBG1(DBG_IKE, "tx... done");
	close(sockfd);
	return 0;
}

int wod_tcp_txrx(char* txbuf, int txlen, char *rxbuf, int *rxlen)
{
	int sockfd = 0, ret;
	struct sockaddr_in servaddr,cliaddr;
	struct timeval timeout;      
	timeout.tv_sec = WOD_TCP_TIMEOUT;
	timeout.tv_usec = 0;

	*rxlen = 0;
	DBG1(DBG_IKE, "create socket");
   	sockfd = socket_local_client("wod_ipsec", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
	if (sockfd < 0) {
		DBG1(DBG_IKE, "create android socket failed : %d !!!", sockfd);
		return -2;
	}

	DBG1(DBG_IKE, "tx... len: %d", txlen);
	ret = send(sockfd, txbuf, txlen, 0);
	if (ret < 0) {
		DBG1(DBG_IKE, "TX failed ... !!!");
		return -1;
	}

	DBG1(DBG_IKE, "setsockopt rx timeout");
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		DBG1(DBG_IKE, "setsockopt failed");
	}

	DBG1(DBG_IKE, "rx...");
	ret = recv(sockfd, rxbuf, MAX_WOD_MSG_LEN-1, 0);
	if (ret < 0) {
		DBG1(DBG_IKE, "RX failed ... !!!");
		return -1;
	}
	*rxlen = ret;
	
	DBG1(DBG_IKE, "rx... got len: %d rx: %s", *rxlen, rxbuf);
	close(sockfd);

	return 0;
}
#else
int wcp_tcp_tx(char* txbuf, int txlen)
{	return 0; }

int wcp_tcp_txrx(char* txbuf, int txlen)
{	return 0; }
#endif


