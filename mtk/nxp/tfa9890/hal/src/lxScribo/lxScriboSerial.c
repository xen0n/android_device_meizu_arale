#include <stdio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include "lxScribo.h"

#ifndef B460800
#define B460800 460800
#endif
//#define BAUD B500000 // 16M xtal
#define BAUD B460800 // 14.7M xtal	//TODO autodetect baud

extern int lxScribo_verbose;

int lxScriboSerialInit(char *dev)
{
    struct termios tio;
    int tty_fd;
    
    if ( lxScribo_verbose )
    	printf("Opening serial Scribo connection\n");

    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL|CSTOPB;           // 8n2, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;

    tty_fd=open(dev, O_RDWR );      //| O_NONBLOCK);
    cfsetospeed(&tio,BAUD);         // 115200 baud
    cfsetispeed(&tio,BAUD);         // 115200 baud

    tcsetattr(tty_fd,TCSANOW,&tio);

    //    lxScriboSetPin(tty_fd, 4, 0x8000); // Weak pull-up on PA4. Is power-up UDA1355.

	return tty_fd;
}
