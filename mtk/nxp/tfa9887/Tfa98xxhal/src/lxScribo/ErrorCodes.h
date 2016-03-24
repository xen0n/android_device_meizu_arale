#ifndef _ERRORCODES_H_
#define _ERRORCODES_H_

#include <inttypes.h>
//#include "Version.h"   //Just for the debug define

/*
//If we ever run out of RAM, change these to defines.
const uint16_t       eCommErrBase = 0x100;
const uint16_t            eComErr = eCommErrBase << 0;  //Communication error.
const uint16_t            eBadCmd = eCommErrBase << 1;  //Unrecognized command
const uint16_t         eBadFormat = eCommErrBase << 2;  //Wrong command format
const uint16_t     eInvalidLength = eCommErrBase << 3;  //More data was expected
const uint16_t eMissingTerminator = eCommErrBase << 4;  //The terminator is missing
const uint16_t     eBadTerminator = eCommErrBase << 5;  //The terminator does not match cTerminator
const uint16_t  eMissingReadCount = eCommErrBase << 6;  //The read count was missing in the UART RX buffer
*/
#define eNone                 0x00

#define eCommErrBase          0x100
#define eComErr               (eCommErrBase << 0)  //Communication error.
#define eBadCmd               (eCommErrBase << 1)  //Unrecognized command
#define eBadFormat            (eCommErrBase << 2)  //Wrong command format
#define eInvalidLength        (eCommErrBase << 3)  //More data was expected
#define eMissingTerminator    (eCommErrBase << 4)  //The terminator is missing
#define eBadTerminator        (eCommErrBase << 5)  //Invalid terminating character.
#define eMissingReadCount     (eCommErrBase << 6)  //The read count was missing in the UART RX buffer

const uint16_t   eI2CspeedTooHigh = 1;  //The requested I2C speed is too high.
const uint16_t    eI2CspeedTooLow = 2;  //The requested I2C speed is too low.
const uint16_t   eBadSlaveAddress = 3;  //The slave address has more than 7 bits.
//const uint16_t  eMissingReadCount = 4;  //The read count was missing in the UART RX buffer
const uint16_t       eMissingData = 5;  //Host sent fewer bytes than specified
const uint16_t        eNoResponse = 6;  //slave didn't respond (unfortunately we can't see if the slave ACK-ed its address)
const uint16_t     eBufferOverRun = 7;  //Internal buffers are too small to execute the request.
const uint16_t      eI2C_SLA_NACK = 8;  //Slave address not acknowledged
const uint16_t     eI2C_DATA_NACK = 9;  //Data not acknowledged
const uint16_t   eI2C_UNSPECIFIED = 10; //An unspecified I2C error occured.
const uint16_t  eInvalidPinNumber = 11; //Pin number is invalid or has no function
const uint16_t    eInvalidPinMode = 12; //Pin cannot be set to the requested mode
const uint16_t      eNotAnInteger = 13; //An ASCII integer was expected, but none was given.
const uint16_t eInvalidSubAddress = 14; //An invalid register offset for I2C access was given.
const uint16_t    eMissingI2CData = 15; //slave sent fewer bytes than requested within timeout period
const uint16_t           MaxError = 16; //Don't forget to update when adding error codes!

//extern const char *errMsg[];
//The following errors relate to this host and deal with errors while processing the uC results.
#define         eBadPrefix (1 << 16)  //The prefix doesn't match the command.
#define      eNoTerminator (2 << 16)  //The result lacks, or has a wrong terminator.
#define        eIncomplete (3 << 16)  //More data was expected.
#define      ePortOpenFail (4 << 16)  //Failed to open serial port
#define     ePortWriteFail (8 << 16)  //Failed to write to serial port
#define      ePortReadFail (16 << 16)  //Failed to read from serial port

//void PrintErrorToUart(uint16_t err);

#endif //_ERRORCODES_H_
