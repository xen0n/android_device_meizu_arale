#ifndef _SCRIBO_H_
#define _SCRIBO_H_

#include "inttypes.h"  //replace with your own

#if !defined (bool)
	  typedef unsigned char bool;
	  #define true ((bool)(1==1))
	  #define false ((bool)(1==0))
	#define __BOOL_DEFINED
#endif

#ifdef WIN32
#ifdef SCRIBO_EXPORTS
  #define SCRIBO_API extern "C" __declspec(dllexport)
  //#define SCRIBO_API
#else
  #define SCRIBO_API __declspec(dllimport)
#endif

#define CCSPEC __stdcall
#else
#define SCRIBO_API
#define CCSPEC
#endif

#if !defined(SCRIBO_EXPORTS)
//Error codes
#define eNone 0x0000

//The following errors relate to the uC and deal with errors reading the UART or parsing the buffer
//const uint32_t       eCommErrBase = 100;  //Communication error.
#define       eCommErrBase 0x100
#define            eComErr (eCommErrBase << 0)  //Communication error.
#define            eBadCmd (eCommErrBase << 1)  //Unrecognized command
#define         eBadFormat (eCommErrBase << 2)  //Wrong command format
#define     eInvalidLength (eCommErrBase << 3)  //More data was expected
#define eMissingTerminator (eCommErrBase << 4)  //The terminator is missing
#define     eBadTerminator (eCommErrBase << 5)  //The terminator does not match cTerminator
#define  eMissingReadCount (eCommErrBase << 6)  //The read count was missing in the UART RX buffer

//The following errors relate to the uC and deal with errors while executing instructions
#define    eI2CspeedTooHigh (1)  //The requested I2C speed is too high.
#define     eI2CspeedTooLow (2)  //The requested I2C speed is too low.
#define    eBadSlaveAddress (3)  //The slave address has more than 7 bits.
//const uint32_t  eMissingReadCount = 4;  //The read count was missing in the UART RX buffer
#define        eMissingData (5)  //slave sent fewer bytes than requested or host sent fewer bytes than specified
#define         eNoResponse (6)  //slave didn't respond (unfortunately we can't see if the slave ACK-ed its address)
#define      eBufferOverRun (7)  //Internal buffers are too small to execute the request.
#define       eI2C_SLA_NACK (8)  //Slave address not acknowledged
#define      eI2C_DATA_NACK (9)  //Data not acknowledged
#define    eI2C_UNSPECIFIED (10) //An unspecified I2C error occured.
#define   eInvalidPinNumber (11) //Pin number is invalid or has no function
#define     eInvalidPinMode (12) //Pin cannot be set to the requested mode
#define       eNotAnInteger (13) //An ASCII integer was expected, but none was given.
#define  eInvalidSubAddress (14) //An invalid register offset for I2C access was given.
#define     eMissingI2CData (15) //slave sent fewer bytes than requested within timeout period
#define            MaxError (16) //Don't forget to update when adding error codes!

//The following errors relate to this host and deal with errors while processing the uC results.
#define         eBadPrefix (1 << 16)  //The prefix doesn't match the command.
#define      eNoTerminator (2 << 16)  //The result lacks, or has a wrong terminator.
#define        eIncomplete (3 << 16)  //More data was expected.
#define      ePortOpenFail (4 << 16)  //Failed to open serial port
#define     ePortWriteFail (8 << 16)  //Failed to write to serial port
#define      ePortReadFail (16 << 16)  //Failed to read from serial port
//const uint32_t      ePortReadFail = eHostErrBase << 5;  //Failed to read from serial port

//const uint32_t         eCSErrBase = 24;
#define eCSErrBase 24
#define        eSvrProcess (1 << eCSErrBase)  //Could not create a server process. (CreateProcess failed)
#define         eSvrLaunch (2 << eCSErrBase)  //Failed to launch the server. (WaitForInputIdle failed)
#define        ePipeCreate (3 << eCSErrBase)  //Could not create pipe connection
#define         ePipeWrite (4 << eCSErrBase)  //Could not write to pipe
#define          ePipeRead (5 << eCSErrBase)  //Could not read from pipe
#define          ePipeMode (6 << eCSErrBase)  //Could not set pipe mode
#define         ePipeComms (7 << eCSErrBase)  //Communication with pipe server was corrupted
#endif //SCRIBO_EXPORTS

SCRIBO_API bool CCSPEC Connect();
SCRIBO_API bool CCSPEC ConnectionValid();
SCRIBO_API bool CCSPEC Version(uint8_t *major, uint8_t *minor);
SCRIBO_API bool CCSPEC VersionStr(char *string, uint16_t *sz);
SCRIBO_API bool CCSPEC ClientVersion(uint8_t *major, uint8_t *minor, uint8_t *third, uint8_t *fourth);
SCRIBO_API bool CCSPEC ClientFileVersion(uint8_t *major, uint8_t *minor, uint8_t *third, uint8_t *fourth);
SCRIBO_API bool CCSPEC ServerVersion(uint8_t *major, uint8_t *minor, uint8_t *third, uint8_t *fourth);
SCRIBO_API bool CCSPEC ServerFileVersion(uint8_t *major, uint8_t *minor, uint8_t *third, uint8_t *fourth);
SCRIBO_API bool CCSPEC Read(uint8_t sla, uint8_t *data, uint16_t *cnt);
SCRIBO_API bool CCSPEC Write(uint8_t sla, const uint8_t *data, uint16_t cnt);
SCRIBO_API bool CCSPEC WriteRead(uint8_t sla, const uint8_t *data, uint16_t wcnt, uint8_t *dataOut, uint16_t *rcnt);
SCRIBO_API uint32_t CCSPEC SetSpeed(uint32_t Hz);
SCRIBO_API uint32_t CCSPEC GetSpeed();
SCRIBO_API bool CCSPEC SetPin(uint8_t pinNumber, uint16_t value);
SCRIBO_API bool CCSPEC GetPin(uint8_t pinNumber, uint16_t *value);
SCRIBO_API uint32_t CCSPEC LastError();
SCRIBO_API uint32_t CCSPEC MaxI2CBufferSize();

#endif //_SCRIBO_H_