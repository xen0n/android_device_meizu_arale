/*
 * lxScribo.h
 *
 *  Created on: Mar 14, 2012
 *      Author: nlv02095
 */

#ifndef LXSCRIBO_H_
#define LXSCRIBO_H_

#define LXSCRIBO_VERSION 	0.1
#define SERIALDEV 			"/dev/ttyUSB0" // "/dev/Scribo"
#define LXSOCKET 			"9887" 		// note: in ascii for api alignment with serial
#include <utils/Log.h>


int lxScriboRegister(char *dev);	// register target and return opened file desc.
int lxScriboGetFd(void);			// return active file desc.

int lxScriboVersion(int fd, char *buffer);
int lxScriboWrite(int fd, int size, uint8_t *buffer, uint32_t *pError);
int lxScriboWriteRead(int fd, int wsize, const uint8_t *wbuffer
										   , int rsize, unsigned char *rbuffer, uint32_t *pError);
int lxScriboPrintTargetRev(int fd);
int lxScriboSerialInit(char *dev);
int lxScriboSocketInit(char *dev);
void lxScriboSocketExit(void);
int lxScriboSetPin(int fd, int pin, int value);

int lxI2cInit(char *dev);
int lxI2cWrite(int fd, int size, uint8_t *buffer);
int lxI2cWriteRead(int fd, int wsize, const uint8_t *wbuffer
										   , int rsize, unsigned char *rbuffer);

int lxDummyInit(char *dev);
int lxDummyWrite(int fd, int size, uint8_t *buffer);
int lxDummyWriteRead(int fd, int wsize, const uint8_t *wbuffer
										   , int rsize, unsigned char *rbuffer);

int (*lxScriboInit)(char *dev);
int (*lxWrite)(int fd, int size, uint8_t *buffer, uint32_t *pError);
int (*lxWriteRead)(int fd, int wsize, const uint8_t *wbuffer
										   , int rsize, unsigned char *rbuffer, uint32_t *pError);

extern int lxScribo_verbose;
extern int i2c_trace;
extern int NXP_I2C_verbose;

// for dummy
#define rprintf printf

//from gpio.h
#define I2CBUS 0
//from gui.h
/* IDs of menu items */
typedef enum
{
  ITEM_ID_VOLUME = 100,
  ITEM_ID_PRESET,
  ITEM_ID_EQUALIZER,

  ITEM_ID_STANDALONE,

  ITEM_ID_ENABLESCREENDUMP,
} menuElement_t;

#endif /* LXSCRIBO_H_ */
