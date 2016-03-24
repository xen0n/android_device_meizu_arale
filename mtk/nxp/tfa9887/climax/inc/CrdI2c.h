#ifndef __CRDI2CDLL_H
#define __CRDI2CDLL_H

/* ****************************************************************************

This is the C(++) interface file for the CRDi2c32.dll library.

History:

12-Feb-08	KG 	First Version based in CrdI2c32.dll version 5.6.4.2425

**************************************************************************** */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BOOL_DEFINED
  typedef unsigned char bool;
  #define true ((bool)(1==1))
  #define false ((bool)(1==0))
#endif

typedef enum { SCL, SDA } i2cPins;

typedef enum { i2cNoInterface, i2cDSM, i2cSinglemaster, i2cHardwareless, i2cDSI, i2cSSI } i2cInterfaceTypes;
	
typedef enum { i2cBus1, i2cBus2, i2cBus2Burst } i2cBusTypes;

typedef enum { i2cNoError, i2cNoAck, i2cSclStuckAtOne, i2cSdaStuckAtOne, i2cSclStuckAtZero, i2cSdaStuckAtZero, 
	i2cTimeOut, i2cArbLost, i2cNoInit, i2cDisabled, i2cUnsupportedValue, i2cUnsupportedType, i2cNoInterfaceFound, 
	i2cNoPortnumber } i2cErrorTypes;

typedef unsigned char i2cData[257];

typedef unsigned char i2cDataLarge[32784];

struct i2cRecType
{
	unsigned char           hDevAddress;
	i2cInterfaceTypes       hInterface;
	int                     hPort;
	int                     hBitrate;
	int                     hTimeout;
	i2cBusTypes             hBusNr;
	bool                    hStopOnError;
	bool                    hInUse;
	int                     hDelay;
	bool                    hTrace;
} ;

typedef i2cInterfaceTypes i2cEnumList[7];

typedef enum { iCentronics, iRs232 } iPortTypes;

typedef enum { cPin1, cPin2, cPin3, cPin4, cPin5, cPin6, cPin7, cPin8, cPin9, cPin10, cPin11, cPin12, 
	cPin13, cPin14, cPin15, cPin16, cPin17 } cPins;

typedef enum { rPin1, rPin3, rPin4, rPin6, rPin7, rPin8, rPin9 } rPins;

typedef struct {
	unsigned char cPin1  : 1; /* Bit 0 of first byte */
	unsigned char cPin2  : 1; /* Bit 1 of first byte */
	unsigned char cPin3  : 1; /* Bit 2 of first byte */
	unsigned char cPin4  : 1;
	unsigned char cPin5  : 1;
	unsigned char cPin6  : 1;
	unsigned char cPin7  : 1;
	unsigned char cPin8  : 1;
	unsigned char cPin9  : 1; /* Bit 0 of second byte */
	unsigned char cPin10 : 1; /* Bit 1 of second byte */
	unsigned char cPin11 : 1;
	unsigned char cPin12 : 1;
	unsigned char cPin13 : 1;
	unsigned char cPin14 : 1;
	unsigned char cPin15 : 1;
	unsigned char cPin16 : 1; 
	unsigned char cPin17 : 1; /* Bit 0 of third byte */
	unsigned int  dummy0 : 15;
} cPinSet; /* This simulates the Delphi Set operator "Set of cPins" */

typedef struct {
	unsigned char rPin1  : 1; /* Bit 0 of first byte */
	unsigned char rPin3  : 1; /* Bit 1 of first byte */
	unsigned char rPin4  : 1;
	unsigned char rPin6  : 1;
	unsigned char rPin7  : 1;
	unsigned char rPin8  : 1;
	unsigned char rPin9  : 1;
	unsigned char dummy0 : 1;
} rPinSet; /* This simulates the Delphi Set operator "Set of rPins" */

typedef enum { dsiSCL, dsiSDA, dsiIN1, dsiIN2 } dsiPins;

typedef enum { hwlSCL, hwlSDA } hwlPins;

typedef enum { dsmSCL1, dsmSDA1, dsmSCL2, dsmSDA2, dsmPin2, dsmPin3, dsmPin4, dsmPin5, dsmPin6, dsmPin7, 
	dsmPin8 } dsmPins;

typedef struct {
	unsigned char dsmSCL1  : 1; /* Bit 0 of first byte */
	unsigned char dsmSDA1  : 1; /* Bit 1 of first byte */
	unsigned char dsmSCL2  : 1;
	unsigned char dsmSDA2  : 1;
	unsigned char dsmPin2  : 1;
	unsigned char dsmPin3  : 1;
	unsigned char dsmPin4  : 1;
	unsigned char dsmPin5  : 1;
	unsigned char dsmPin6  : 1; /* Bit 0 of second byte */
	unsigned char dsmPin7  : 1;
	unsigned char dsmPin8  : 1;
	unsigned char dummy0 : 5;
} dsmPinSet; /* This simulates the Delphi Set operator "Set of rPins" */

typedef enum { usbSCL1, usbSDA1, usbSCL2, usbSDA2, usbPin1, usbPin2, usbPin3, usbPin4,
               usbPin5, usbPin6, usbPin7, usbPin8 } usbPins;

typedef struct {
	unsigned char usbSCL1  : 1; /* Bit 0 of first byte */
	unsigned char usbSDA1  : 1; /* Bit 1 of first byte */
	unsigned char usbSCL2  : 1;
	unsigned char usbSDA2  : 1;
	unsigned char usbPin1  : 1;
	unsigned char usbPin2  : 1;
	unsigned char usbPin3  : 1;
	unsigned char usbPin4  : 1;
	unsigned char usbPin5  : 1; /* Bit 0 of second byte */
	unsigned char usbPin6  : 1;
	unsigned char usbPin7  : 1;
	unsigned char usbPin8  : 1;
	unsigned char dummy0 : 4;
} usbPinSet; /* This simulates the Delphi Set operator "Set of rPins" */


typedef enum { smSCL, smSDA } smPins;

typedef enum { ssiSCL, ssiSDA } ssiPins;

typedef enum { pNil, pDSM, pSM, pHWL, pDSI, pSSI, pCentronics, pRS232 } TPlotMode;

typedef enum {fSCLin, fSCLout, fSDAin, fSDAout, fHigh, fLow, fNone} TPinFunc;

typedef struct {
  TPinFunc Func;
  bool     Inv;
} TcsiPin;

typedef TcsiPin Tcsi[17];

typedef enum {csiSCL, csiSDA} csiPins;

typedef void __stdcall TPortDCallbackProc(unsigned char DevNum, unsigned char Portstate);





typedef void __stdcall i2cAbout_type();
extern i2cAbout_type *i2cAbout;

typedef bool __stdcall i2cBlock_type(bool OnOff);
extern i2cBlock_type *i2cBlock;

typedef bool __stdcall i2cIsBlock_type();
extern i2cIsBlock_type *i2cIsBlock;

typedef void __stdcall i2cCalibrate_type();
extern i2cCalibrate_type *i2cCalibrate;

typedef void __stdcall i2cConfig_type();
extern i2cConfig_type *i2cConfig;

typedef void __stdcall i2cDebug_type(bool OnOff);
extern i2cDebug_type *i2cDebug;

typedef void __stdcall i2cDebugStr_type(char * GetString);
extern i2cDebugStr_type *i2cDebugStr;

typedef void __stdcall i2cGetVersion_type(char * GetString);
extern i2cGetVersion_type *i2cGetVersion;

typedef void __stdcall i2cEnumPorts_type(i2cInterfaceTypes * E, bool InitAll);
extern i2cEnumPorts_type *i2cEnumPorts;

typedef i2cErrorTypes __stdcall i2cError_type();
extern i2cError_type *i2cError;

typedef void __stdcall i2cExecute_type(i2cBusTypes Bus, int NrOfBytes, unsigned char * Data);
extern i2cExecute_type *i2cExecute;

typedef void __stdcall i2cExecuteLarge_type(i2cBusTypes Bus, int NrOfBytes, unsigned char * Data);
extern i2cExecuteLarge_type *i2cExecuteLarge;

typedef void __stdcall i2cExecuteRS_type(i2cBusTypes Bus, int NrOfWriteBytes, const unsigned char * WriteData, int NrOfReadBytes, unsigned char * ReadData);
extern i2cExecuteRS_type *i2cExecuteRS;

typedef void __stdcall i2cExecuteRSLarge_type(i2cBusTypes Bus, int NrOfWriteBytes, const unsigned char * WriteData, int NrOfReadBytes, unsigned char * ReadData);
extern i2cExecuteRSLarge_type *i2cExecuteRSLarge;

typedef void __stdcall i2cFindPort_type(i2cInterfaceTypes * IntType, int PortNumber, bool InitAll);
extern i2cFindPort_type *i2cFindPort;

typedef int __stdcall i2cGetBitrate_type();
extern i2cGetBitrate_type *i2cGetBitrate;

typedef i2cBusTypes __stdcall i2cGetBus_type();
extern i2cGetBus_type *i2cGetBus;

typedef int __stdcall i2cGetBusNumber_type();
extern i2cGetBusNumber_type *i2cGetBusNumber;

typedef int __stdcall i2cGetByteDelay_type();
extern i2cGetByteDelay_type *i2cGetByteDelay;

typedef bool __stdcall i2cGetCaracasMode_type();
extern i2cGetCaracasMode_type *i2cGetCaracasMode;

typedef unsigned char __stdcall i2cGetDeviceAddress_type();
extern i2cGetDeviceAddress_type *i2cGetDeviceAddress;

typedef i2cInterfaceTypes __stdcall i2cGetInterfaceType_type();
extern i2cGetInterfaceType_type *i2cGetInterfaceType;

typedef bool __stdcall i2cGetPin_type(i2cPins Pin);
extern i2cGetPin_type *i2cGetPin;

typedef int __stdcall i2cGetPort_type();
extern i2cGetPort_type *i2cGetPort;

typedef bool __stdcall i2cGetStopOnError_type();
extern i2cGetStopOnError_type *i2cGetStopOnError;

typedef int __stdcall i2cGetTimeOut_type();
extern i2cGetTimeOut_type *i2cGetTimeOut;

typedef bool __stdcall i2cInterfacePresent_type(bool InitAll);
extern i2cInterfacePresent_type *i2cInterfacePresent;

typedef bool __stdcall i2cIsDebug_type();
extern i2cIsDebug_type *i2cIsDebug;

typedef int __stdcall i2cLoadSettingsP_type(char * Fname);
extern i2cLoadSettingsP_type *i2cLoadSettingsP;

typedef void __stdcall i2cSaveSettingsP_type(char * Fname);
extern i2cSaveSettingsP_type *i2cSaveSettingsP;

typedef void __stdcall i2cSetBitrate_type(int NewValue);
extern i2cSetBitrate_type *i2cSetBitrate;

typedef void __stdcall i2cSetBus_type(i2cBusTypes Bus);
extern i2cSetBus_type *i2cSetBus;

typedef void __stdcall i2cSetBusNumber_type(int NewNumber);
extern i2cSetBusNumber_type *i2cSetBusNumber;

typedef void __stdcall i2cSetByteDelay_type(int DelayTime);
extern i2cSetByteDelay_type *i2cSetByteDelay;

typedef void __stdcall i2cSetCaracasMode_type(bool SetOn);
extern i2cSetCaracasMode_type *i2cSetCaracasMode;

typedef int __stdcall i2cCaracasReplyCount_type();
extern i2cCaracasReplyCount_type *i2cCaracasReplyCount;

typedef void __stdcall i2cSetDeviceAddress_type(unsigned char NewAddress);
extern i2cSetDeviceAddress_type *i2cSetDeviceAddress;

typedef void __stdcall i2cSetInterfaceType_type(i2cInterfaceTypes NewValue);
extern i2cSetInterfaceType_type *i2cSetInterfaceType;

typedef void __stdcall i2cSetPin_type(i2cPins Pin, bool Level);
extern i2cSetPin_type *i2cSetPin;

typedef void __stdcall i2cSetPort_type(int PortNumber, bool InitAll);
extern i2cSetPort_type *i2cSetPort;

typedef void __stdcall i2cSetPortAndInterfaceType_type(int NewPort, i2cInterfaceTypes NewValue);
extern i2cSetPortAndInterfaceType_type *i2cSetPortAndInterfaceType;

typedef void __stdcall i2cSetStopOnError_type(bool SetOn);
extern i2cSetStopOnError_type *i2cSetStopOnError;

typedef void __stdcall i2cSetTimeOut_type(int NewValue);
extern i2cSetTimeOut_type *i2cSetTimeOut;

typedef void __stdcall i2cStart_type();
extern i2cStart_type *i2cStart;

typedef void __stdcall i2cReStart_type();
extern i2cReStart_type *i2cReStart;

typedef void __stdcall i2cStop_type();
extern i2cStop_type *i2cStop;

typedef void __stdcall i2cReadAcknowledge_type(bool * Error);
extern i2cReadAcknowledge_type *i2cReadAcknowledge;

typedef void __stdcall i2cSendAcknowledge_type();
extern i2cSendAcknowledge_type *i2cSendAcknowledge;

typedef void __stdcall i2cSendNoAcknowledge_type();
extern i2cSendNoAcknowledge_type *i2cSendNoAcknowledge;

typedef void __stdcall i2cReadByte_type(unsigned char * Value);
extern i2cReadByte_type *i2cReadByte;

typedef void __stdcall i2cSendByte_type(unsigned char Value);
extern i2cSendByte_type *i2cSendByte;

typedef void __stdcall cEnableRead_type();
extern cEnableRead_type *cEnableRead;

typedef void __stdcall cEnableWrite_type();
extern cEnableWrite_type *cEnableWrite;

typedef bool __stdcall cGetPin_type(cPins Pin);
extern cGetPin_type *cGetPin;

typedef cPinSet __stdcall cGetPort_type();
extern cGetPort_type *cGetPort;

typedef void __stdcall cSetPin_type(cPins Pin);
extern cSetPin_type *cSetPin;

typedef void __stdcall cSetPort_type(bool Level);
extern cSetPort_type *cSetPort;

typedef void __stdcall iChangeLPT_type(unsigned char LPTnum, int NewAddress);
extern iChangeLPT_type *iChangeLPT;

typedef void __stdcall iClearBuffer_type();
extern iClearBuffer_type *iClearBuffer;

typedef int __stdcall iGetBuffer_type();
extern iGetBuffer_type *iGetBuffer;

typedef int __stdcall iGetDelay_type();
extern iGetDelay_type *iGetDelay;

typedef int __stdcall iGetPCport_type(iPortTypes * PortType);
extern iGetPCport_type *iGetPCport;

typedef int __stdcall iGetPCportAddress_type();
extern iGetPCportAddress_type *iGetPCportAddress;

typedef int __stdcall iLPTaddress_type(unsigned char PortNumber);
extern iLPTaddress_type *iLPTaddress;

typedef void __stdcall iSetDelay_type(int DelayTime);
extern iSetDelay_type *iSetDelay;

typedef void __stdcall iSetPCport_type(iPortTypes PortType, int PortNumber, bool InitAll);
extern iSetPCport_type *iSetPCport;

typedef void __stdcall iWriteBuffer_type(int Address);
extern iWriteBuffer_type *iWriteBuffer;

typedef void __stdcall rChangeRS232_type(unsigned char RS232num, int NewAddress);
extern rChangeRS232_type *rChangeRS232;

typedef bool __stdcall rGetPin_type(rPins Pin);
extern rGetPin_type *rGetPin;

typedef int __stdcall rLockPort_type(int PortNr, bool TrueFalse);
extern rLockPort_type *rLockPort;

typedef void __stdcall rSetPin_type(rPins Pin, bool Level);
extern rSetPin_type *rSetPin;

typedef bool __stdcall InPort_type(unsigned short int Address);
extern InPort_type *InPort;

typedef void __stdcall OutPort_type(unsigned short int Address);
extern OutPort_type *OutPort;

typedef int __stdcall rRS232address_type(unsigned char PortNumber);
extern rRS232address_type *rRS232address;

typedef void __stdcall usbClearInputs_type();
extern usbClearInputs_type *usbClearInputs;

typedef bool __stdcall usbExecuteBurst_type();
extern usbExecuteBurst_type *usbExecuteBurst;

typedef i2cInterfaceTypes __stdcall usbFound_type();
extern usbFound_type *usbFound;

typedef unsigned char __stdcall usbGetBurstPort_type();
extern usbGetBurstPort_type *usbGetBurstPort;

typedef unsigned char __stdcall usbGetPort_type();
extern usbGetPort_type *usbGetPort;

typedef void __stdcall usbInternalBus_type(bool SetOpen);
extern usbInternalBus_type *usbInternalBus;

typedef void __stdcall usbSetADC_type(bool SetOpen);
extern usbSetADC_type *usbSetADC;

typedef void __stdcall usbSetBitrate_type(int NewValue);
extern usbSetBitrate_type *usbSetBitrate;

typedef void __stdcall usbSetBurstMode_type(bool SetOn);
extern usbSetBurstMode_type *usbSetBurstMode;

typedef void __stdcall usbSetBurstPort_type(unsigned char Port);
extern usbSetBurstPort_type *usbSetBurstPort;

typedef void __stdcall usbSetDevice_type(int DevNum);
extern usbSetDevice_type *usbSetDevice;

typedef void __stdcall usbSetStopOnError_type(bool SetOn);
extern usbSetStopOnError_type *usbSetStopOnError;

typedef void __stdcall usbSetPin_type(usbPins Pin, bool Level);
extern usbSetPin_type *usbSetPin;

typedef bool __stdcall usbGetPin_type(usbPins Pin);
extern usbGetPin_type *usbGetPin;

typedef void __stdcall usbRenumerate_type();
extern usbRenumerate_type *usbRenumerate;

typedef void __stdcall usbSetPort_type(unsigned char Pins);
extern usbSetPort_type *usbSetPort;

typedef void __stdcall usbSetTimeOut_type(int NewValue);
extern usbSetTimeOut_type *usbSetTimeOut;

typedef bool __stdcall usbStartInterrupt_type(usbPins Iopin, unsigned char Status, TPortDCallbackProc CallBackFunc);
extern usbStartInterrupt_type *usbStartInterrupt;

typedef void __stdcall usbStopInterrupt_type();
extern usbStopInterrupt_type *usbStopInterrupt;

typedef void __stdcall usbGetBurstData_type(unsigned char * Data, int * Len);
extern usbGetBurstData_type *usbGetBurstData;

typedef void __stdcall usbGetBurstDataLong_type(unsigned short int * Data, int * Len);
extern usbGetBurstDataLong_type *usbGetBurstDataLong;

typedef int __stdcall dsmGetLPTport_type();
extern dsmGetLPTport_type *dsmGetLPTport;

typedef bool __stdcall dsmGetPin_type(dsmPins Pin);
extern dsmGetPin_type *dsmGetPin;

typedef dsmPinSet __stdcall dsmGetPort_type();
extern dsmGetPort_type *dsmGetPort;

typedef bool __stdcall dsmPresent_type();
extern dsmPresent_type *dsmPresent;

typedef void __stdcall dsmSetLPTport_type(int LPTnumber, bool InitAll);
extern dsmSetLPTport_type *dsmSetLPTport;

typedef void __stdcall dsmSetPin_type(dsmPins Pin, bool Level);
extern dsmSetPin_type *dsmSetPin;

typedef void __stdcall dsmSetPort_type(dsmPinSet Pins);
extern dsmSetPort_type *dsmSetPort;

typedef int __stdcall smGetLPTport_type();
extern smGetLPTport_type *smGetLPTport;

typedef bool __stdcall smGetPin_type(smPins Pin);
extern smGetPin_type *smGetPin;

typedef bool __stdcall smPresent_type();
extern smPresent_type *smPresent;

typedef void __stdcall smSetLPTport_type(int LPTnumber, bool InitAll);
extern smSetLPTport_type *smSetLPTport;

typedef void __stdcall smSetPin_type(smPins Pin, bool Level);
extern smSetPin_type *smSetPin;

typedef void __stdcall csiGetConfig_type(iPortTypes * PortType, Tcsi * Def);
extern csiGetConfig_type *csiGetConfig;

typedef bool __stdcall csiGetPin_type(csiPins Pin);
extern csiGetPin_type *csiGetPin;

typedef int __stdcall csiGetPort_type();
extern csiGetPort_type *csiGetPort;

typedef iPortTypes __stdcall csiGetType_type();
extern csiGetType_type *csiGetType;

typedef bool __stdcall csiPresent_type();
extern csiPresent_type *csiPresent;

typedef void __stdcall csiSetConfig_type(iPortTypes * PortType, Tcsi Def);
extern csiSetConfig_type *csiSetConfig;

typedef void __stdcall csiSetPin_type(csiPins Pin, bool Level);
extern csiSetPin_type *csiSetPin;

typedef void __stdcall csiSetPort_type(int PortNumber, bool InitAll);
extern csiSetPort_type *csiSetPort;

typedef void __stdcall csiSetType_type(iPortTypes PortType);
extern csiSetType_type *csiSetType;

typedef int __stdcall hwlGetLPTport_type();
extern hwlGetLPTport_type *hwlGetLPTport;

typedef bool __stdcall hwlGetPin_type(hwlPins Pin);
extern hwlGetPin_type *hwlGetPin;

typedef bool __stdcall hwlPresent_type();
extern hwlPresent_type *hwlPresent;

typedef void __stdcall hwlSetLPTport_type(int LPTnumber, bool InitAll);
extern hwlSetLPTport_type *hwlSetLPTport;

typedef void __stdcall hwlSetPin_type(hwlPins Pin, bool Level);
extern hwlSetPin_type *hwlSetPin;

typedef int __stdcall dsiGetBus_type();
extern dsiGetBus_type *dsiGetBus;

typedef bool __stdcall dsiGetPin_type(dsiPins Pin);
extern dsiGetPin_type *dsiGetPin;

typedef int __stdcall dsiGetRSport_type();
extern dsiGetRSport_type *dsiGetRSport;

typedef bool __stdcall dsiPresent_type();
extern dsiPresent_type *dsiPresent;

typedef void __stdcall dsiSelectBus_type(int BusNumber);
extern dsiSelectBus_type *dsiSelectBus;

typedef void __stdcall dsiSetPin_type(dsiPins Pin, bool Level);
extern dsiSetPin_type *dsiSetPin;

typedef void __stdcall dsiSetRSport_type(int RS232number, bool InitAll);
extern dsiSetRSport_type *dsiSetRSport;

typedef bool __stdcall ssiGetPin_type(ssiPins Pin);
extern ssiGetPin_type *ssiGetPin;

typedef int __stdcall ssiGetRSport_type();
extern ssiGetRSport_type *ssiGetRSport;

typedef bool __stdcall ssiPresent_type();
extern ssiPresent_type *ssiPresent;

typedef void __stdcall ssiSetPin_type(ssiPins Pin, bool Level);
extern ssiSetPin_type *ssiSetPin;

typedef void __stdcall ssiSetRSport_type(int RS232number, bool InitAll);
extern ssiSetRSport_type *ssiSetRSport;

typedef void __stdcall DelayMS_type(int T);
extern DelayMS_type *DelayMS;

typedef void __stdcall DelayUS_type(int T);
extern DelayUS_type *DelayUS;

typedef void __stdcall StartTimer_type();
extern StartTimer_type *StartTimer;

typedef int __stdcall TimerMS_type();
extern TimerMS_type *TimerMS;

typedef int __stdcall TimerUS_type();
extern TimerUS_type *TimerUS;

typedef void __stdcall crdInitADC_type(i2cBusTypes BusNr, unsigned char Address, unsigned char Mode);
extern crdInitADC_type *crdInitADC;

typedef void __stdcall crdReadADC_type(unsigned char * A0, unsigned char * A1, unsigned char * A2, unsigned char * A3);
extern crdReadADC_type *crdReadADC;

typedef void __stdcall crdSetDAC_type(unsigned char NewValue);
extern crdSetDAC_type *crdSetDAC;


/* Load the library (initialize) - needs to be called before using the dll functions */
/* If the return value is equal to 0 the Dll could be loaded successfully */
int LoadDll(void);

/* unload library ("deinitialize") */
void UnloadDll(void);
#ifdef __cplusplus
}
#endif

#endif
