/*
 * nxpTfa98xx.h
 *
 *  interface to the Tfa98xx API
 *
 *  Created on: Apr 6, 2012
 *      Author: wim
 */

#ifndef NXPTFA98XX_H_
#define NXPTFA98XX_H_

#ifdef __ANDROID__
#include <utils/Log.h>
#else
#define LOGV if (0/*tfa98xx_verbose*/) printf //TODO improve logging
#endif

#include "Tfa98xx.h"
#include "tfa9890_cust.h"

#define PrintAssert(e) if ((e)) fprintf(stderr, "PrintAssert:%s (%s:%d) %s\n",__FUNCTION__,__FILE__,__LINE__, Tfa98xx_GetErrorString(e))


#if defined(__cplusplus)
extern "C" {
#endif

#define NXPTFA_APP_REV_MAJOR    (1)         // major API rev
#define NXPTFA_APP_REV_MINOR    (2)         // minor
   
/**
 * Type containing all the possible errors that can be returned
 *
 */
typedef enum nxpTfa98xx_Error {
    nxpTfa98xx_Error_Ok = 0,
    nxpTfa98xx_Error_Fail,		// generic failure, avoid mislead message
    nxpTfa98xx_Error_NoClock,   // no clock detected
    nxpTfa98xx_Error_LowAPI,    // error returned from lower API level
    nxpTfa98xx_Error_BadParam,  // wrong parameter
    nxpTfa98xx_Error_AmpOn,     // amp is still running
    nxpTfa98xx_Error_DSP_not_running, // dsp is not running
    nxpTfa98xx_Error_Other
} nxpTfa98xx_Error_t;

/*
 * profiles & volumesteps
 *
 */
#define TFA_MAX_FILENAME		(64) // clip filename if longer
#define TFA_MAX_PROFILES        (3)
#define TFA_MAX_VSTEPS          (16)
#define TFA_FILE_MAX_VSTEPS     (64)
#define TFA_ISMUTE(v)           ((v)>=TFA_MAX_VSTEPS)   // mute if vstep is bigger than max step
//#define TFA_I2CDEVICE		"/dev/i2c-2"		// Panda = 4 device driver interface special file
//#define TFA_I2CSLAVEBASE		(0x34)              // tfa device slave address of 1st (=left) device
//#define TFA_CHECK_REV      	(0x12)                  // revision register contents

#define TFA_PROFILE0_SAMPLERATE	(44100)
#define TFA_PROFILE1_SAMPLERATE	(44100)
#define TFA_PROFILE2_SAMPLERATE	(44100)
// defaults
#define TFA_DEFAULT_SAMPLERATE 	TFA_PROFILE0_SAMPLERATE
#define TFA_DEFAULT_I2SINPUT	0
#define TFA_DEFAULT_PLLINPUT	1		// i2s data in1

#define TFA_I2SCTRL_BYPASS_DSP   (65407)
#define TFA_I2SCTRL_UNBYPASS_DSP (1<<7)

extern unsigned char  tfa98xxI2cSlave; // global for i2c access

#define TFA98XX_EQ_LENGTH				 (10*128) // TODO properly implement eq ;=max10 lines

#pragma pack (push, 1) //TODO double check alignments

typedef struct nxpTfa98xxVolumeStep {
    float attenuation;              // IEEE single float
    unsigned char CF[TFA98XX_PRESET_LENGTH];
} nxpTfa98xxVolumeStep_t;

#define NXPTFA_MAXLINE              (256)       // maximum string length

#define TFA_VSTEP_ID 		"VP1_00"
#define TFA_MAXPATCH_LENGTH (3*1024)    // TODO move this into low API

#define TFA98XX_MAXTAG              (138)        // TODO move to Tfa98xx api


typedef struct nxpTfa98xxVolumeStepFile {
    char id[6];                     // 6 bytes ID = "VP1_00"
    unsigned short size;            // 16 bit integer data size (not including header)
    char crc[4];              //  CRC value for data after header. Uses 0xEDB88320 as polynomial (same as zip). TODO assure 32bits int
    //
    nxpTfa98xxVolumeStep_t vstep[TFA_FILE_MAX_VSTEPS];
} nxpTfa98xxVolumeStepFile_t;
#pragma pack (pop)

typedef struct nxpTfa98xxProfile {
    int valid;
    int i2sRoute;   // TODO define routing
    int ipll;
    int sampleRate;
    int vsteps;     // total amount of vsteps in this profile
	char filename[TFA_MAX_FILENAME];
    //TODO more profile params?
    nxpTfa98xxVolumeStep_t vstep[TFA_MAX_VSTEPS];
    // TODO add eq/biquad floats
    char EqValid;
    char Eqfilename[TFA_MAX_FILENAME];
    const char EQ[TFA98XX_EQ_LENGTH];
#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices
	int DrcLength;
	char Drcfilename[TFA_MAX_FILENAME];
    unsigned char Drc[TFA98XX_DRC_LENGTH]; 
#endif    
} nxpTfa98xxProfile_t;


/*
 * the DSP parameters that are loaded after poweron
 */
typedef struct nxpTfa98xxParameters {
    int patchLength;
    char patchFile[TFA_MAX_FILENAME];
    const unsigned char patchBuffer[TFA_MAXPATCH_LENGTH];
    int speakerLength;
    char speakerFile[TFA_MAX_FILENAME];
    unsigned char speakerBuffer[TFA98XX_SPEAKERPARAMETER_LENGTH];
    int configLength;
    char configFile[TFA_MAX_FILENAME];
    unsigned char configBuffer[TFA98XX_CONFIG_LENGTH];
    // profiles
    nxpTfa98xxProfile_t profile[TFA_MAX_PROFILES];

} nxpTfa98xxParameters_t;

/*
 * the DSP parameters that are loaded after poweron
 */
typedef struct nxpTfa98xxGetParameters {
    int patchLength;
    unsigned char patchBuffer[TFA_MAXPATCH_LENGTH];
    int speakerLength;
    unsigned char speakerBuffer[TFA98XX_SPEAKERPARAMETER_LENGTH];
    int configLength;
    unsigned char configBuffer[TFA98XX_CONFIG_LENGTH];
    // profiles
    nxpTfa98xxProfile_t profile[TFA_MAX_PROFILES];

} nxpTfa98xxGetParameters_t;
/*
 * buffer types for setting parameters
 */
typedef enum nxpTfa98xxParamsType {
    tfa_patch_params,
    tfa_speaker_params,
    tfa_preset_params,
    tfa_config_params,
    tfa_equalizer_params,
    tfa_drc_params,
//    tfa_mdrc_params,
    tfa_no_params
} nxpTfa98xxParamsType_t;


/*
 * nxpTfa98xx API calls
 */
int nxpTfa98xxCalibration(Tfa98xx_handle_t *handlesIn, int once); /* calibration */
nxpTfa98xx_Error_t nxpTfa98xxStoreParameters(Tfa98xx_handle_t *handlesIn, nxpTfa98xxParamsType_t params, void *data, int length); // provides all loadable parameters
nxpTfa98xx_Error_t nxpTfa98xxStoreProfile(int profile, void *data, int length); // provide profile
nxpTfa98xx_Error_t nxpTfa98xxClockEnable(Tfa98xx_handle_t *handlesIn); // I2S clock is present
nxpTfa98xx_Error_t nxpTfa98xxClockDisable(Tfa98xx_handle_t *handlesIn);     // I2S clock will stop
nxpTfa98xx_Error_t nxpTfa98xxPowerdown(void);   // do a cold start init on next clock enable
nxpTfa98xx_Error_t nxpTfa98xxSetVolume(Tfa98xx_handle_t *handlesIn, int profile, int step); // apply volume step, or mute

int nxpTfa98xxGetVolume(Tfa98xx_handle_t *handlesIn, float *getVol); /* get volume for selected idx */
int nxpTfa98xxSetVolumeDB(Tfa98xx_handle_t *handlesIn, float vol); /* master volume control */
int tfa98xxSaveParamsFile(Tfa98xx_handle_t *handlesIn, char *filename );
int tfa98xxLoadParamsFile(Tfa98xx_handle_t *handlesIn, char *filename );
int nxpTfa98xxBypassDSP(Tfa98xx_handle_t *handlesIn); 
int nxpTfa98xxUnBypassDSP(Tfa98xx_handle_t *handlesIn);
/* select device for testing and individual addressing functions */
int nxpTfa98xxSetIdx( int idxIn ); /* default = left */
/* notify the Tfa98xx of the sample rate of the I2S bus that will be used.
 * @param rate in Hz.  must be 32000, 44100 or 48000
 */
nxpTfa98xx_Error_t nxpTfa98xxSetSampleRate(Tfa98xx_handle_t *handlesIn, int rate);

/* read the Tfa98xx of the sample rate of the I2S bus that will be used.
 * @param pRate pointer to rate in Hz. Will be one of 32000, 44100 or 48000 if successful
 */
nxpTfa98xx_Error_t nxpTfa98xxGetSampleRate(Tfa98xx_handle_t *handlesIn, int* pRate);

nxpTfa98xx_Error_t nxpTfa98xxVersions(Tfa98xx_handle_t *handlesIn, char *strings, int maxlength); // return all version
const char* nxpTfa98xxGetErrorString(nxpTfa98xx_Error_t error);
int tfa98xxSaveFile(Tfa98xx_handle_t *handlesIn, char *filename, nxpTfa98xxParamsType_t params);
int tfa98xxLoadFile(Tfa98xx_handle_t *handlesIn, char *filename, nxpTfa98xxParamsType_t params);
/*
 * return current tCoef , set new value if arg!=0
 */
float tfa98xxTcoef(float tCoef);
float tCoefFromSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes);
/*
 * reset MtpEx register to do re-calibration.
 */
Tfa98xx_Error_t resetMtpEx(Tfa98xx_handle_t *handlesIn);

int tfa98xxReset( Tfa98xx_handle_t *handlesIn ); /* init registers and coldboot dsp */
void tfa98xxShowProfile( int profile); // for verbose
/*
 *  lower levels generic API
 */
extern int tfa98xx_trace, tfa98xx_verbose, tfa98xx_quiet;
/*
 * write all cached parameters to the DSP
 */
nxpTfa98xx_Error_t tfa98xxLoadParams(Tfa98xx_handle_t handle);
/* load parameter buffer */
int  tfa98xxSetParams(  nxpTfa98xxParamsType_t params, void *data, int length,
                        Tfa98xx_handle_t handle);
/* initialize and enable */
int tfa98xxInit( Tfa98xx_handle_t handle);
int tfa98xxClose( Tfa98xx_handle_t *handlesIn ); /* close all devices */
void tfaQuickInit(Tfa98xx_handle_t *handlesIn); //TODO do we want this eventually?

/* Get parameter buffer */
nxpTfa98xx_Error_t tfa98xxDspGetParam( unsigned char module_id, unsigned char param_id, 
                                       int num_bytes, unsigned char data[],
                                       Tfa98xx_handle_t *handlesIn    );
/* calibration */
//in tfa98xxCal int tfa98xxCalibration(Tfa98xx_handle_t *handlesIn, int once );
int nxpTfa98xxCalibrationStereo(Tfa98xx_handle_t *handlesIn, int once);
/* wait until calibration impedance is ok */
int tfa98xxWaitCalibration(Tfa98xx_handle_t *handlesIn ); /* 0 return is ok, else timed-out */
/* wait until calibration impedance is ok */
int waitCalibration(Tfa98xx_handle_t handlesIn, int *calibrateDone); /* 0 return is ok, else timed-out */
/* get calibration impedance for selected idx */
int tfa98xxGetCalibrationImpedance( float* re0, 
                                    Tfa98xx_handle_t *handlesIn); /* if return==0  DC impedance in re0 */
/*
 * tfa98xxGetState of selected idx:
 *      Current Loudspeaker blocked resistance
 *      Current Speaker Temperature value
 *  TODO is this enough ?
 */
int             tfa98xxGetState(float* resistance, float* temperature, Tfa98xx_handle_t *handlesIn);
void            tfa98xxPowerUp(Tfa98xx_handle_t *handlesIn);
void            tfa98xxPowerdown(Tfa98xx_handle_t *handlesIn);
/* r/w for selected idx */
unsigned short  tfa98xxReadRegister(unsigned char offset, 
                                 Tfa98xx_handle_t *handlesIn);
void            tfa98xxWriteRegister(  unsigned char offset, unsigned short value, 
                                       Tfa98xx_handle_t *handlesIn);
#if defined(__cplusplus)
}  /* extern "C" */
#endif
#endif /* TFA98XX_H_ */
