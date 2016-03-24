#undef __cplusplus 

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include "Scribo.h"

//Link  with Scribo.lib. Make sure the Scribo.lib can be found by the linker
#ifdef _X64
#pragma comment (lib, "Scribo64.lib")	
#else
#pragma comment (lib, "Scribo.lib")	
#endif

/* implementation of the NXP_I2C API on Windows */
#include "NXP_I2C.h"

#include <stdio.h>
#include <assert.h>

#define I2C_MINIMUM_BUF_SZ (255)

typedef struct
{
  char msg[32];
} NXP_I2C_ErrorStr_t;

static int gI2cBufSz = 0;
static bool bInit = false;

static NXP_I2C_Error_t init_I2C(void)
{
#define bufSz 1024
	char buffer[bufSz];
	unsigned char ma, mi, bma, bmi;
	unsigned short sz = bufSz;
	unsigned long busFreq;

	NXP_I2C_Error_t error = NXP_I2C_NoInterfaceFound;

	//Show some info about Scribo client and server
	if (ClientVersion(&ma, &mi, &bma, &bmi)) {
		printf("Scribo client version: %d.%d.%d.%d\r\n", ma, mi, bma, bmi);
	} else {
		printf("Failed to get Scribo client version\r\n");
	}

	if (ServerVersion(&ma, &mi, &bma, &bmi)) {
		printf("Scribo server version: %d.%d.%d.%d\r\n", ma, mi, bma, bmi);
	} else {
		printf("Failed to get Scribo server version\r\n");
	}

	//If Scribo isn't connected yet, try to connect
	if (!ConnectionValid()) Connect();

	if (ConnectionValid()) 
	{
		//We are connected, so print some info
		if (VersionStr(buffer, &sz)) {
			//just to be safe
			if (sz < bufSz) buffer[sz] = '\0';
			printf_s("Remote target: %s\r\n", buffer);
		}
		else {
			printf("Remote target didn't return version string\r\n");
		}

		busFreq = GetSpeed();
		if (busFreq > 0) {
			printf("I2C bus frequency: %d\r\n", busFreq);
		}
		else {
			printf("I2C bus frequency unknown\r\n");
		}

		//How much data can we send in one transaction? (varies per target)
		gI2cBufSz = MaxI2CBufferSize();
		error = gI2cBufSz > 0 ? NXP_I2C_Ok : NXP_I2C_NoInit;
		//Some early firmware report less than 255 (incorrectly)
		if (gI2cBufSz < I2C_MINIMUM_BUF_SZ) gI2cBufSz = I2C_MINIMUM_BUF_SZ;	// I2C_MINIMUM_BUF_SZ (255) is minimum
		//Keep room for register offset
		gI2cBufSz -= 1;
		printf("I2C data buffer size:%d\r\n", gI2cBufSz);
	} 
	else {
		printf("Scribo is not connected to a remote target\r\n");
	}
	return error;
}

static const NXP_I2C_ErrorStr_t errorStr[NXP_I2C_ErrorMaxValue] =
{
  { "UnassignedErrorCode" },
  { "Ok" },
  { "NoAck" },
  { "SclStuckAtOne" },
  { "SdaStuckAtOne" },
  { "SclStuckAtZero" },
  { "SdaStuckAtZero" },
  { "TimeOut" },
  { "ArbLost" },
  { "NoInit" },
  { "Disabled" },
  { "UnsupportedValue" },
  { "UnsupportedType" },
  { "NoInterfaceFound" },
  { "NoPortnumber" }
};

static NXP_I2C_Error_t translate_error(uint32_t error)
{
  NXP_I2C_Error_t retval;

  switch (error)
  {
    case eNone: retval=NXP_I2C_Ok;
      break;
	case eI2C_DATA_NACK:
    case eI2C_SLA_NACK: retval=NXP_I2C_NoAck;
      break;
    case eI2C_UNSPECIFIED: retval=NXP_I2C_ArbLost;
      break;
	case eNoResponse: retval=NXP_I2C_TimeOut;
      break;
    case eNoService: retval=NXP_I2C_NoInit;
      break;
   case eInvalidPinMode: retval=NXP_I2C_UnsupportedValue;
      break;
    case eNotAnInteger: retval=NXP_I2C_UnsupportedType;
      break;
	case eBufferOverRun: retval=NXP_I2C_BufferOverRun;
		break;
    case eNoConnect: retval=NXP_I2C_NoInterfaceFound;
      break;
    case eInvalidPinNumber: retval=NXP_I2C_NoPortnumber;
      break;
    default:
      /* error without appropriate code */
    	 retval=NXP_I2C_UnassignedErrorCode;
  }

  if (error != eNone)
	{
    printf("I2C error %d (%s)\n", error, &errorStr[retval].msg);
	}
  return retval;
}

static NXP_I2C_Error_t init_if_firsttime(void)
{
  NXP_I2C_Error_t retval = NXP_I2C_Ok;

  if (!bInit)
  {
    retval = init_I2C();
	if (retval == NXP_I2C_Ok) bInit = true;
  }
  return retval;
}

NXP_I2C_Error_t NXP_I2C_Write(  unsigned char sla,
								int num_write_bytes,
                                const unsigned char data[] )
{
	NXP_I2C_Error_t retval;

	retval = init_if_firsttime();
	if (NXP_I2C_Ok==retval)	{
		if (num_write_bytes > gI2cBufSz ) {
			fprintf(stderr, "%s: too many bytes: %d (max = %d)\n", __FUNCTION__, num_write_bytes, gI2cBufSz );
			return NXP_I2C_UnsupportedValue;
		}
		if (!Write(sla >> 1, data, ((uint16_t)num_write_bytes & 0xFFFF))) {
			retval = translate_error( LastError() );
		}
	}
	return retval;
}

NXP_I2C_Error_t NXP_I2C_WriteRead(  unsigned char sla,
									int num_write_bytes,
                                    const unsigned char write_data[],
                                    int num_read_bytes,
                                    unsigned char read_data[] )
{
	unsigned short rCnt = ((unsigned short)num_read_bytes);
		
	NXP_I2C_Error_t retval;
	retval = init_if_firsttime();

	if (NXP_I2C_Ok==retval) {
		if (num_write_bytes > gI2cBufSz) {
			fprintf(stderr, "%s: too many bytes to write: %d (max = %d)\r\n", __FUNCTION__, num_write_bytes, gI2cBufSz);
			return NXP_I2C_UnsupportedValue;
		}
		if (num_read_bytes > gI2cBufSz) {
			fprintf(stderr, "%s: too many bytes to read: %d (max = %d)\r\n", __FUNCTION__, num_read_bytes, gI2cBufSz);
			return NXP_I2C_UnsupportedValue;
		}
		if (!WriteRead(sla >> 1, write_data, ((uint16_t)num_write_bytes), read_data, &rCnt)) {
			retval = translate_error( LastError() );
		}
	}
	return retval;
}

int NXP_I2C_BufferSize()
{
	NXP_I2C_Error_t error;
	error = init_if_firsttime();
	if (error == NXP_I2C_Ok) {
		return gI2cBufSz > I2C_MINIMUM_BUF_SZ ? gI2cBufSz : I2C_MINIMUM_BUF_SZ - 1;
	}
	return I2C_MINIMUM_BUF_SZ - 1; //255 is minimum
}