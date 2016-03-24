/*Simple Echo Server, demonstrating basic socket system calls.
 * Binds to random port between 9000 and 9999,
 * echos back to client all received messages
 * 'quit' message from client kills server gracefully
 *
 * To test server operation, open a telnet connection to host
 * E.g. telnet YOUR_IP PORT
 * PORT will be specified by server when run
 * Anything sent by telnet client to server will be echoed back
 *
 *
 * Written by: Ajay Gopinathan, Jan 08
 * ajay.gopinathan@ucalgary.ca
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>

static	int listenSocket=-1;
static int activeSocket=-1;

typedef void (*sighandler_t)(int);

void lxScriboSocketExit(void)
{
	printf("%s closing sockets\n", __FUNCTION__);

	// still bind error after re-open when traffic was active
	if (listenSocket>0)
		shutdown(listenSocket, SHUT_RDWR);
		//close(listenSocket);

	if (activeSocket>0)
		shutdown(activeSocket, SHUT_RDWR);
		//close(activeSocket);

	_exit(0);
}

/*
 * ctl-c handler
 */
static void lxScriboCtlc(int sig)
{

		(void) signal(SIGINT, SIG_DFL);

		lxScriboSocketExit();

}
/*
 * exit handler
 */
static void lxScriboAtexit(void)
{
		lxScriboSocketExit();
}

int lxScriboSocketInit(char *server)//, char *hostname)
{
	   char *hostname, *portnr;
	   int activeSocket = socket(AF_INET, SOCK_STREAM, 0);  /* init socket descriptor */
	   struct sockaddr_in sin;
	   struct hostent *host;
	   int port;

	   portnr = strchr ( server , ':');
	   if ( portnr == NULL )
	   {
		   fprintf (stderr, "%s: %s is not a valid servername, use host:port\n",__FUNCTION__, server);
		   return -1;
	   }
	   hostname=server;
	   *portnr++ ='\0'; //terminate
	   port=atoi(portnr);

	   host = gethostbyname(hostname);
	   if ( !host ) {
		   fprintf(stderr, "Error: wrong hostname: %s\n", hostname);
		   exit(1);
	   }

		if(port==0) // illegal input
			return -1;

	   /*** PLACE DATA IN sockaddr_in struct ***/
	   memcpy(&sin.sin_addr.s_addr, host->h_addr, host->h_length);
	   sin.sin_family = AF_INET;
	   sin.sin_port = htons(port);

	   /*** CONNECT SOCKET TO THE SERVICE DESCRIBED BY sockaddr_in struct ***/
	   if (connect(activeSocket, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	     {
	     fprintf(stderr,"error connecting to %s:%d\n", hostname , port);
	     return -1;;
	     }

	   atexit(lxScriboAtexit);
	   (void) signal(SIGINT, lxScriboCtlc);

	   return activeSocket;

}

/*
 * the sockets are created first and then waits until a connection is done
 * the active socket is returned
 */
int lxScriboListenSocketInit(char *socketnr)
{

//int main(int args, char *argv[]){

	int port;
	int  rc;
	char hostname[50];

	port = atoi(socketnr);
	if(port==0) // illegal input
		return -1;

	rc = gethostname(hostname,sizeof(hostname));

	struct sockaddr_in serverAdd;
	struct sockaddr_in clientAdd;
	socklen_t clientAddLen;

	atexit(lxScriboAtexit);
	(void) signal(SIGINT, lxScriboCtlc);

	printf("Listening to %s:%d\n", hostname, port);

	memset(&serverAdd, 0, sizeof(serverAdd));
	serverAdd.sin_family = AF_INET;
	serverAdd.sin_port = htons(port);

	//Bind to any local server address using htonl (host to network long):
	serverAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	//Or specify address using inet_pton:
	//inet_pton(AF_INET, "127.0.0.1", &serverAdd.sin_addr.s_addr);

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(listenSocket == -1){
		printf("Error creating socket\n");
		return -1;
	}

	if(bind(listenSocket, (struct sockaddr*) &serverAdd, sizeof(serverAdd)) == -1){
		printf("Bind error\n");
		return -1;
	}

	if(listen(listenSocket, 5) == -1){
		printf("Listen Error\n");
		return -1;
	}

	clientAddLen = sizeof(clientAdd);
	activeSocket = accept(listenSocket, (struct sockaddr*) &clientAdd, &clientAddLen);


	char clientIP [100];
	inet_ntop(AF_INET, &clientAdd.sin_addr.s_addr, &clientIP, sizeof(clientAdd));
	printf("Received connection from %s\n", clientIP);

	close(listenSocket);

	return (activeSocket);

}


