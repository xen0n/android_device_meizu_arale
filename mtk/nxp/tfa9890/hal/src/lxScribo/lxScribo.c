//
// lxScribo main entry
//
// 	 This is the initiator that sets up the connection to either a serial/RS232 device or a socket.
//
// 	 input args:
// 	 	 none: 		connect to the default device, /dev/Scribo
// 	 	 string: 	assume this is a special device name
// 	 	 number:	use this to connect to a socket with this number
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdint.h>
#include <ctype.h>
#include "NXP_I2C.h"
#include "lxScribo.h"
#include <assert.h>
#include "Scribo.h"
#include <utils/Log.h>

#define LOG_TAG  "lxScribo"

//#define SCRIBOVERBOSE

#ifdef SCRIBOVERBOSE
int lxScribo_verbose=1;
#else
int lxScribo_verbose=0;
#endif

#define VERBOSE if (lxScribo_verbose)
extern unsigned char  tfa98xxI2cSlave; // global for access check
static int lxScriboFd=-1;
static int isDirect=0;        // global that tells if scribo is used for the target

// Command headers for UART comms, let op: commando's worden omgekeerd op de seriele bus gezet
// dus 'wr' wordt 'rw' in de buffer.
const uint16_t cmdVersion   = 'v' ;  //Version
const uint16_t cmdRead      = 'r' ;  //Read I2C
const uint16_t cmdWrite     = 'w' ;  //Write I2C
//const uint16_t cmdWriteRead = 'w'<<8|'r';  // 'wr' Write and read I2C
const uint16_t cmdWriteRead = ('w' << 8 | 'r');  // 'wr' Write and read I2C
const uint16_t cmdPinSet        = ('p' << 8 | 's') ;  //pin set
const uint16_t cmdPinRead     = ('p' << 8 | 'r') ;  //pin get

const uint8_t terminator    = 0x02;  //All commands and answers are terminated with 0x02

struct cmdHeader {
	uint16_t cmd;
	uint16_t length;
};
static void hexdump(char *str, const unsigned char * data, int num_write_bytes) //TODO cleanup/consolidate all hexdumps
{
	int i;

    ALOGD("%s", str);
	for(i=0;i<num_write_bytes;i++)
	{
        ALOGD("0x%02x ", data[i]);
	}
    ALOGD("\n");
	fflush(stdout);
}
void dump_buffer(int length, char *buf)
{
	uint8_t *ptr = (uint8_t*)buf;
	while (length--)
		if (isprint(*ptr))
			putchar(*ptr++);
		else {
            ALOGD("<0x%02x>", *ptr++);
			fflush(stdout);
		}
    ALOGD("\n");
	fflush(stdout);
}

static int lxScriboGetResponseHeader(int fd, const uint16_t cmd, int* prlength)
{
	uint8_t response[6];
	uint16_t rcmd, rstatus; //, rlength;
	int length;
#ifdef CYGWIN
	uint8_t *pRcv;
	int actual;
	
		pRcv=response;
	do {
		actual = read(fd, pRcv, sizeof(response)-length); //
		pRcv += actual;
		length += actual;
		if(actual<0)
				break;
	} while (length<sizeof(response) );
	
#else
	length = read(fd, response, sizeof(response)); //
#endif //CYGWIN
	VERBOSE hexdump("rsp:", response, sizeof(response));
	// response (lsb/msb)= echo cmd[0] cmd[1] , status [0] [1], length [0] [1] ...data[...]....terminator[0]
	if ( length==sizeof(response) )
	{
		rcmd    = response[0] | response[1]<<8;
		rstatus = response[2] | response[3]<<8;
		*prlength = response[4] | response[5]<<8;  /* extra bytes that need to be read */
		
		assert(cmd == rcmd); /* must get response to expected cmd */

		if (rstatus != 0) {
            ALOGD("Error: scribo error: 0x%02x\n", rstatus);
			//exit(1);
		}

		return (int)rstatus; /* iso length */
	}
	else {
        ALOGD("bad response length=%d\n", length);
		return -1; //exit(1);
	}


	return -1;
}
/*
 * set pin
 */
int lxScriboSetPin(int fd, int pin, int value)
{
#ifdef WIN32
	return 0;
#else
	uint8_t cmd[6];
    int stat;
	int ret;
	uint8_t term;
	int rlength;
	
    ALOGD("SetPin\n");

    cmd[0] = cmdPinSet;
    cmd[1] = cmdPinSet>>8;
    cmd[2] = (uint8_t)pin;
    cmd[3] = (uint8_t)value;
    cmd[4] = (uint8_t)(value>>8);
    cmd[5]= terminator;

	// write header
    VERBOSE ALOGD("cmd: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
			cmd[0], cmd[1],cmd[2],cmd[3],cmd[4], cmd[5]);
    ret = write(fd, cmd, sizeof(cmd));
	assert(ret == sizeof(cmd));

    stat = lxScriboGetResponseHeader( fd, cmdPinSet, &rlength);
	assert(stat == 0);
	assert(rlength == 0); // expect no return data
	ret = read(fd, &term, 1);
	assert(ret == 1);
	
    VERBOSE ALOGD("term: 0x%02x\n", term);
	usleep(1000000);
	assert(term == terminator);

    ALOGD("SetPin done\n");
    return stat>=0;
#endif
}
/*
 * get pin state
 */
int lxScriboGetPin(int fd, int pin)
{
#ifdef WIN32
	return 0;
#else
	uint8_t cmd[4];
	int value, length;
	int ret;
	int rlength;

	cmd[0] = cmdPinRead;
	cmd[1] = cmdPinRead>>8;
	cmd[2] = pin;
	cmd[3] = terminator;

	// write header
    VERBOSE ALOGD("cmd: 0x%02x 0x%02x 0x%02x 0x%02x \n",
			cmd[0], cmd[1],cmd[2],cmd[3]);
	ret = write(fd, cmd, sizeof(cmd));
	assert(ret == sizeof(cmd));

	ret = lxScriboGetResponseHeader( fd, cmdPinRead, &rlength);
	assert(ret == 0);
	assert(rlength == 1); /*  1 byte result? */

	length = read(fd, &value, 1);
	assert(length == 1);
	
	// TODO: read terminator ???

	return value;
#endif
}
/*
 * retrieve the version string from the device
 */
int lxScriboVersion(int fd, char *buffer)
{
#ifdef WIN32
	return 0;
#else
	char cmd[3];
	int length;
	int ret;
	int rlength;

	cmd[0] = cmdVersion;
	cmd[1] = cmdVersion>>8;
	cmd[2] = terminator;
//	dump_buffer(sizeof(cmd), cmd);
	ret = write(fd, cmd, sizeof(cmd));
	assert(ret == sizeof(cmd));

	ret = lxScriboGetResponseHeader( fd, cmdVersion, &rlength);
	assert(ret == 0);
	assert(rlength > 0);

	length = read(fd, buffer, 256);
	strcat(buffer,"\n");
	length++;
	

	return length;
#endif
}
int lxScriboWrite(int fd, int size, uint8_t *buffer, uint32_t *pError)
{
	uint8_t cmd[5], slave,term;
	int status, total=0;
	int rlength;

	*pError = eNone;
	// slave is the 1st byte in wbuffer
	slave = buffer[0] >> 1;

#if 0
	if (slave != tfa98xxI2cSlave) {
        ALOGD("wrong slave 0x%02x iso 0x%02x\n", slave, tfa98xxI2cSlave);
		return 0;
	}
#endif

	size -= 1;
	buffer++;

#ifdef WIN32
	return NXP_I2C_Write(slave, size, buffer);
#else

	cmd[0] = cmdWrite;// lsb
	cmd[1] = cmdWrite>>8;// msb
	cmd[2] = slave; // 1st byte is the i2c slave address
	cmd[3] = size & 0xff; // lsb
	cmd[4] = (size >> 8) & 0xff; // msb

	// write header
    VERBOSE ALOGD("cmd: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ",
			cmd[0], cmd[1],cmd[2],cmd[3],cmd[4]);
	status = write(fd, cmd, sizeof(cmd));
	if(status>0)
		total+=status;
	else 
	{
		*pError = ePortWriteFail;
		return status;
	}
	// write payload
	VERBOSE hexdump("\t\twdata:", buffer, size);
	status = write(fd, buffer, size);
	if(status>0)
		total+=status;
	else 
	{
		*pError = ePortWriteFail;
		return status;
	}
	// write terminator
	cmd[0] = terminator;
    VERBOSE ALOGD("term: 0x%02x\n", cmd[0]);
	status = write(fd, cmd, 1);
	if(status>0)
		total+=status;
	else 
	{
		*pError = ePortWriteFail;
		return status;
	}

	status = lxScriboGetResponseHeader(fd, cmdWrite, &rlength);
	if (status != 0)
	{
		*pError = status;
	}			
	assert(rlength == 0); // expect no return data
	status = read(fd, &term, 1);
	if (status < 0)
	{
		if (*pError == eNone) *pError = ePortReadFail;
		return status;
	}
	assert(term == terminator);
    VERBOSE ALOGD("term: 0x%02x\n", term);
	return total;
#endif
}

int lxScriboWriteRead(int fd, int wsize, const uint8_t *wbuffer, int rsize,
		uint8_t *rbuffer, uint32_t *pError) {
	uint8_t cmd[5], rcnt[2], slave, *rptr, term;
	int length = 0;
	int status, total = 0;
	int rlength;

	*pError = eNone;
	// slave is the 1st byte in wbuffer
	slave = wbuffer[0] >> 1;

	// fail if not target
#if 0
	if (slave != tfa98xxI2cSlave) {
        ALOGD("wrong slave 0x%02x iso 0x%02x\n", slave, tfa98xxI2cSlave);
		*pError = eBadSlaveAddress; // ?
		return 0;
	}
#endif

	wsize -= 1;
	wbuffer++;

#ifdef WIN32
	return NXP_I2C_WriteRead(slave, wsize, wbuffer, rsize, rbuffer);
#else

	if ((slave<<1) + 1 == rbuffer[0]) // write & read to same target
			{
		//Format = 'wr'(16) + sla(8) + w_cnt(16) + data(8 * w_cnt) + r_cnt(16) + 0x02
		cmd[0] = cmdWriteRead & 0xFF ;
		cmd[1] = cmdWriteRead >> 8;
		cmd[2] = slave;
		cmd[3] = wsize & 0xff; // lsb
		cmd[4] = (wsize >> 8) & 0xff; // msb

		// write header
		VERBOSE hexdump("cmd:", cmd, sizeof(cmd));
		status = write(fd, cmd, sizeof(cmd));
		if (status > 0)
			total += status;
		else
		{
			*pError = ePortWriteFail;
			return status;
		}
		// write payload
		VERBOSE hexdump("\t\twdata:", wbuffer, wsize);
		status = write(fd, wbuffer, wsize);
		if (status > 0)
			total += status;
		else
		{
			*pError = ePortWriteFail;
			return status;
		}

		// write readcount
		rsize -= 1;
		rcnt[0] = rsize & 0xff; // lsb
		rcnt[1] = (rsize >> 8) & 0xff; // msb
		VERBOSE hexdump("rdcount:",rcnt, 2);
		status = write(fd, rcnt, 2);
		if (status > 0)
			total += status;
		else
		{
			*pError = ePortWriteFail;
			return status;
		}

		// write terminator
		cmd[0] = terminator;
        VERBOSE ALOGD("term: 0x%02x\n", cmd[0]);
		status = write(fd, cmd, 1);
		if (status > 0)
			total += status;
		else
		{
			*pError = ePortWriteFail;
			return status;
		}

		//if( rcnt[1] | rcnt[0] >100)    // TODO check timing
		if (rsize > 100) // ?????????????????????????????????????
			usleep(20000);
		// slave is the 1st byte in rbuffer, remove here
		//rsize -= 1;
		rptr = rbuffer+1;
		// read back, blocking
		status = lxScriboGetResponseHeader(fd, cmdWriteRead, &rlength);
		if (status != 0)
		{
			*pError = status;
		}			
		assert(rlength == rsize);
		
		/*** FOR CYGWING - remove or fix this! 
		//	VERBOSE printf("Reading %d bytes\n", rsize);
		#ifdef CYGWIN
			length=0;
			do {
				int actual;
				actual = read(fd, rptr, rsize-length);  //
				rptr += actual;
				length += actual;
				if(actual<0)
						break;
			} while (length<rsize );
		#else
				length = read(fd, rptr, rsize); //
		#endif //CYGWIN	
		****/
		
		
		//	VERBOSE printf("Reading %d bytes\n", rsize);
		length = read(fd, rptr, rsize); //
		if (length < 0)
		{
			if (*pError == eNone) *pError = ePortReadFail;
			return -1;
		}
		VERBOSE hexdump("\trdata:",rptr, rsize);
		// else something wrong, still read the terminator
		//	if(status>0) TODO handle error
		status = read(fd, &term, 1);
		if (length < 0)
		{
			if (*pError == eNone) *pError = ePortReadFail;
			return -1;
		}
		assert(term == terminator);
        VERBOSE ALOGD("rterm: 0x%02x\n", term);
	}
	else {
        ALOGD("!!!! write slave != read slave !!! %s:%d\n", __FILE__, __LINE__);
		*pError = eBadSlaveAddress;
		status = -1;
		return status;
	}
	return length>0 ? (length + 1) : 0; // we need 1 more for the length because of the slave address
#endif
}


int lxScriboRegister(char *dev) {

	if ( lxScribo_verbose )
        ALOGD("%s:target device=%s\n", __FUNCTION__, dev);

	if (lxScriboFd>0)
		return lxScriboFd; // TODO avoid opening twice already opened before

	// default scribo
	lxWrite = lxScriboWrite;
	lxWriteRead = lxScriboWriteRead;

#ifndef WIN32
	if ( strchr( dev , ':' ) != 0)	// if : in name > it's a socket
		lxScriboInit=lxScriboSocketInit;
	else if ( strncmp (dev, "/dev/i2c",  8 ) == 0 ) { // if /dev/i2c... direct i2c device
		lxWrite = lxI2cWrite;
		lxWriteRead = lxI2cWriteRead;
		lxScriboInit= lxI2cInit;
		isDirect=1; // no scribo involved
		VERBOSE printf("%s: i2c\n", __FUNCTION__);
	}
	else if ( strncmp (dev, "/dev/smartpa_i2c",  sizeof("/dev/smartpa_i2c") -1) == 0 ) {
		lxWrite = lxI2cWrite;
		lxWriteRead = lxI2cWriteRead;
		lxScriboInit= lxI2cInit;
		isDirect=1; // no scribo involved
		VERBOSE ALOGD("%s: smartpa_i2c\n", __FUNCTION__);
	}
	else if ( strncmp (dev, "/dev/",  5 ) == 0 ) // if /dev/ it must be a serial device
		lxScriboInit=lxScriboSerialInit;
	else if ( strncmp (dev, "dummy",  5 ) == 0 ) {// if dummy act dummy
		lxWrite = lxDummyWrite;
		lxWriteRead = lxDummyWriteRead;
		lxScriboInit=lxDummyInit;
	    isDirect=1; // no scribo involved
	}
	else {
        ALOGD("%s: devicename %s is not a valid target\n", __FUNCTION__, dev); // anything else is a file
		_exit(1);
	}
	lxScriboFd = (*lxScriboInit)(dev);
#else
	lxScriboFd = (int)(dev);
#endif	
	return lxScriboFd;
}

int lxScriboGetFd(void)
{
	if ( lxScriboFd < 0 )
		fprintf(stderr, "Warning: the target is not open\n");

	return lxScriboFd;
}

int lxScriboPrintTargetRev(int fd)
{
	 int  length, stat;
	 char  buf[256];

	memset(buf, 0, sizeof(buf));

	stat = length = lxScriboVersion(fd, buf);
    ALOGD("%d\n", length);
	if (length) {
		dump_buffer(length, buf);
	}
	return stat;
}
/*
 * return version string if applicable
 */
int lxScriboGetRev(int fd, char *str, int max) {
    int length;

    if (isDirect) // no scribo involved
        length = sprintf(str, "(no scribo used)\n");
    else
        length = lxScriboVersion(fd, str);

    return length;
}


