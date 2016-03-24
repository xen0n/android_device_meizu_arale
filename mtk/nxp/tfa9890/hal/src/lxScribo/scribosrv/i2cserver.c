/*
 * i2cserver.c
 *
 *  Created on: Apr 21, 2012
 *      Author: wim
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lxScribo.h"
#include "NXP_I2C.h"

extern int gTargetFd;	 //TODO make this cleaner: interface filedescr
extern int i2cserver_verbose;
#define VERBOSE if (i2cserver_verbose)

static i2c_Speed=0;
int i2c_GetSpeed(int bus) {
	return i2c_Speed;
}
void i2c_SetSpeed(int bus, int bitrate) {
	i2c_Speed=bitrate;
}

static void hexdump(int num_write_bytes, const unsigned char * data)
{
	int i;

	for(i=0;i<num_write_bytes;i++)
	{
		printf("0x%02x ", data[i]);
	}

}

int i2c_WriteRead(int bus, int addrWr, void* dataWr, int sizeWr, int* nWr, void* dataRd, int sizeRd, int* nRd)
{
	int NrOfWriteBytes, status=1;
	unsigned char * WriteData, wbuf[1024],rbuf[1024];
	int NrOfReadBytes;
	unsigned char * ReadData,reg;
	u_int16_t *words;


	wbuf[0]= addrWr<<1; //put address first
	memcpy(&wbuf[1], dataWr, sizeWr); // append rest of write bytes

	NrOfWriteBytes=sizeWr+1;
	WriteData=wbuf;

	NrOfReadBytes=sizeRd+1; //compensate for extra read slave byte
	ReadData=dataRd;
	if (NrOfReadBytes) {
		rbuf[0]=(addrWr<<1) + 1;	// read slave addr
	}
	reg = WriteData[1];
	words = (u_int16_t*)WriteData;

	//printf("addr:0x%02x reg=0x%02x\n", addrWr, reg );


	VERBOSE printf("%s W %d bytes:\t",__FUNCTION__, NrOfWriteBytes);
	VERBOSE hexdump(NrOfWriteBytes, WriteData);
	VERBOSE printf("\n");

	status = ( NXP_I2C_Ok == i2cExecuteRS( NrOfWriteBytes, WriteData ,
							   NrOfReadBytes, rbuf) );

	if (dataRd) {
		memcpy(dataRd, rbuf+1, sizeRd); // get rid of read slave
		*nRd=sizeRd; //actual

		VERBOSE printf(" R %d bytes ", NrOfReadBytes);
		VERBOSE hexdump(NrOfReadBytes, ReadData);
		VERBOSE printf("\n");
	}

	return status;
}

_Bool i2c_Write(int bus, int addrWr, void* dataWr, int sizeWr)
{
//	return i2c_WriteRead(bus, addr, data, size, NULL, NULL, 0, NULL);
	int NrOfWriteBytes;
	unsigned char * WriteData, wbuf[1024];
	unsigned char reg;
	u_int16_t *words;

	wbuf[0]= addrWr<<1; //put address first
	memcpy(&wbuf[1], dataWr, sizeWr); // append rest of write bytes

	NrOfWriteBytes=sizeWr+1;
	WriteData=wbuf;

	reg = WriteData[1];
	words = (u_int16_t*)WriteData;

	//printf("addr:0x%02x reg=0x%02x\n", addrWr, reg );


	VERBOSE printf("%s W %d bytes:\t",__FUNCTION__, NrOfWriteBytes);
	VERBOSE hexdump(NrOfWriteBytes, WriteData);
	VERBOSE printf("\n");

	if ( NXP_I2C_Ok != i2cExecute( NrOfWriteBytes, WriteData ) )
		return 0;

	return 1;

}

_Bool i2c_Read(int bus, int addr, void* data, int size, int* nr)
{
	return i2c_WriteRead(bus, addr, NULL, 0, NULL, data, size, nr);
}

_Bool gui_GetValue(menuElement_t item, int* val)
{
	printf("%s:%s\n", __FILE__, __FUNCTION__);
	return 0;
}
_Bool gui_SetValue(menuElement_t item, int* val)
{
	printf("%s:%s\n", __FILE__, __FUNCTION__);
	return 0;
}


