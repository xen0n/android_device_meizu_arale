#ifndef TFA9887_H
#define TFA9887_H

#include "Tfa98xx.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Type containing all the possible errors that can occur
 *
 */
typedef enum Tfa9887_Error {
  Tfa9887_Error_Ok = 0,
  Tfa9887_Error_DSP_not_running,  /* communication with the DSP failed, presumably because DSP not running */
  Tfa9887_Error_Bad_Parameter,
	Tfa9887_Error_NotOpen,          /* the given handle is not open */
	Tfa9887_Error_OutOfHandles,     /* too many handles */

	Tfa9887_Error_RpcBase = 100,
	Tfa9887_Error_RpcBusy = 101,
	Tfa9887_Error_RpcModId = 102,
	Tfa9887_Error_RpcParamId = 103,
	Tfa9887_Error_RpcInfoId = 104,
	Tfa9887_Error_RpcNotAllowedSpeaker = 105,

  Tfa9887_Error_Not_Implemented,
  Tfa9887_Error_Not_Supported,
  Tfa9887_Error_I2C_Fatal,        /* Fatal I2C error occurred */
  Tfa9887_Error_I2C_NonFatal,     /* Nonfatal I2C error, and retry count reached */
  Tfa9887_Error_Other = 1000
} Tfa9887_Error_t;

typedef enum Tfa9887_AmpInputSel {
	Tfa9887_AmpInputSel_I2SLeft,
	Tfa9887_AmpInputSel_I2SRight,
	Tfa9887_AmpInputSel_DSP
} Tfa9887_AmpInputSel_t;

typedef enum Tfa9887_OutputSel {
	Tfa9887_I2SOutputSel_CurrentSense,
	Tfa9887_I2SOutputSel_DSP_Gain,
	Tfa9887_I2SOutputSel_DSP_AEC,
	Tfa9887_I2SOutputSel_Amp,
	Tfa9887_I2SOutputSel_DataI3R,
	Tfa9887_I2SOutputSel_DataI3L,
	Tfa9887_I2SOutputSel_DcdcFFwdCur,
} Tfa9887_OutputSel_t;

typedef enum Tfa9887_StereoGainSel {
	Tfa9887_StereoGainSel_Left,
	Tfa9887_StereoGainSel_Right
} Tfa9887_StereoGainSel_t;

#define TFA9887_SPEAKERPARAMETER_LENGTH 423
typedef unsigned char Tfa9887_SpeakerParameters_t[TFA9887_SPEAKERPARAMETER_LENGTH];

#define TFA9887_CONFIG_LENGTH 165
typedef unsigned char Tfa9887_Config_t[TFA9887_CONFIG_LENGTH];

#define TFA9887_PRESET_LENGTH    87
typedef unsigned char Tfa9887_Preset_t[TFA9887_PRESET_LENGTH];

#define TFA9887_MAXPATCH_LENGTH (3*1024)


/* the number of biquads supported */
#define TFA9887_BIQUAD_NUM              10

typedef enum Tfa9887_SpeakerType {
	Tfa9887_Speaker_FreeSpeaker=0,
	Tfa9887_Speaker_RA11x15,
	Tfa9887_Speaker_RA13x18,
	Tfa9887_Speaker_RA9x13,

	Tfa9887_Speaker_Max

} Tfa9887_SpeakerType_t;


typedef enum Tfa9887_Channel {
	Tfa9887_Channel_L,
	Tfa9887_Channel_R,
	Tfa9887_Channel_L_R,
	Tfa9887_Channel_Stereo
} Tfa9887_Channel_t;

typedef enum Tfa9887_Mute {
	Tfa9887_Mute_Off,
	Tfa9887_Mute_Digital,
	Tfa9887_Mute_Amplifier
} Tfa9887_Mute_t;


typedef enum Tfa9887_SpeakerBoostStatusFlags
{
	Tfa9887_SpeakerBoost_Activity=0		,		/* Input signal activity. */
	Tfa9887_SpeakerBoost_S_Ctrl				,		/* S Control triggers the limiter */
	Tfa9887_SpeakerBoost_Muted			  ,		/* 1 when signal is muted */
	Tfa9887_SpeakerBoost_X_Ctrl 			,		/* X Control triggers the limiter */
	Tfa9887_SpeakerBoost_T_Ctrl 			,		/* T Control triggers the limiter */
	Tfa9887_SpeakerBoost_NewModel			,		/* New model is available */
	Tfa9887_SpeakerBoost_VolumeRdy		,		/* 0 means stable volume, 1 means volume is still smoothing */
	Tfa9887_SpeakerBoost_Damaged			,		/* Speaker Damage detected  */
	Tfa9887_SpeakerBoost_SignalClipping		/* Input Signal clipping detected */
} Tfa9887_SpeakerBoostStatusFlags_t ;

typedef struct Tfa9887_SpeakerBoost_StateInfo
{
	float	agcGain;			/* Current AGC Gain value */
	float	limGain;			/* Current Limiter Gain value */
	float	sMax;				  /* Current Clip/Lim threshold */
	int		T;					  /* Current Speaker Temperature value */
	int	  statusFlag;		/* Masked bit word, see Tfa9887_SpeakerBoostStatusFlags */
	float	X1;					  /* Current estimated Excursion value caused by Speakerboost gain control */
	float	X2;					  /* Current estimated Excursion value caused by manual gain setting */
	float Re;           /* Current Loudspeaker blocked resistance */
} Tfa9887_SpeakerBoost_StateInfo_t;

typedef int Tfa9887_handle_t;

/**
 * The TFA9887 slave address can be 0x68, 6A, 6C or 6E
 */
Tfa9887_Error_t Tfa9887_Open(unsigned char slave_address, Tfa9887_handle_t* pHandle);

/**
 * Load the default HW settings in the device
 */
Tfa9887_Error_t Tfa9887_Init(Tfa9887_handle_t handle);

/**
 * Close the instance handle
 */
Tfa9887_Error_t Tfa9887_Close(Tfa9887_handle_t handle);

/**
 * Initialize the I2C hal
 */
#ifdef WIN32
Tfa9887_Error_t Tfa9887_I2C_Init( void );
#endif
/* control the powerdown bit of the TFA9887
 * @param powerdown must be 1 or 0
 */
Tfa9887_Error_t Tfa9887_Powerdown(Tfa9887_handle_t handle, int powerdown);

/* Notify the DSP that all parameters have been loaded.
 * @param configured must be 1 or 0
 */
Tfa9887_Error_t Tfa9887_SetConfigured(Tfa9887_handle_t handle);

/* control the input_sel bits of the TFA9887, to indicate what is sent to the amplfier and speaker
 * @param input_sel, see Tfa9887_AmpInputSel_t
 */
Tfa9887_Error_t Tfa9887_SelectAmplifierInput(Tfa9887_handle_t handle, Tfa9887_AmpInputSel_t input_sel);

/* control the I2S left output of the TFA9887
 * @param output_sel, see Tfa9887_OutputSel_t
 */
Tfa9887_Error_t Tfa9887_SelectI2SOutputLeft(Tfa9887_handle_t handle, Tfa9887_OutputSel_t output_sel);

/* control the I2S right output of the TFA9887
 * @param output_sel, see Tfa9887_OutputSel_t
 */
Tfa9887_Error_t Tfa9887_SelectI2SOutputRight(Tfa9887_handle_t handle, Tfa9887_OutputSel_t output_sel);

/* indicates on which channel of DATAI2 the gain from the other TFA9887 IC is set
 * @param gain_sel, see Tfa9887_StereoGainSel_t
 */
Tfa9887_Error_t Tfa9887_SelectStereoGainChannel(Tfa9887_handle_t handle, Tfa9887_StereoGainSel_t gain_sel);

/* control the volume of the DSP
 * @param voldB volume in dB.  must be between 0 and -inf
 */
Tfa9887_Error_t Tfa9887_SetVolume(Tfa9887_handle_t handle, float voldB);

/* read the currently set volume
 * @param voldB volume in dB.
 */
Tfa9887_Error_t Tfa9887_GetVolume(Tfa9887_handle_t handle, float* pVoldB);

/* notify the TFA9887 of the sample rate of the I2S bus that will be used.
 * @param rate in Hz.  must be 32000, 44100 or 48000
 */
Tfa9887_Error_t Tfa9887_SetSampleRate(Tfa9887_handle_t handle, int rate);

/* read the TFA9887 of the sample rate of the I2S bus that will be used.
 * @param pRate pointer to rate in Hz. Will be one of 32000, 44100 or 48000 if successful
 */
Tfa9887_Error_t Tfa9887_GetSampleRate(Tfa9887_handle_t handle, int* pRate);


/* set the input channel to use
 * @param channel see Tfa9887_Channel_t enumeration
 */
Tfa9887_Error_t Tfa9887_SelectChannel(Tfa9887_handle_t handle, Tfa9887_Channel_t channel);

/* mute/unmute the audio
 * @param mute see Tfa9887_Mute_t enumeration
 */
Tfa9887_Error_t Tfa9887_SetMute(Tfa9887_handle_t handle, Tfa9887_Mute_t mute);

Tfa9887_Error_t Tfa9887_GetMute(Tfa9887_handle_t handle, Tfa9887_Mute_t* pMute);

/* The following functions can only be called when the DSP is running (hence the 'Dsp' in the name)
 * - I2S clock must be active,
 * - IC must be in operating mode
 */

/* patch the ROM code of the DSP */
Tfa9887_Error_t Tfa9887_DspPatch(Tfa9887_handle_t handle, int patchLength, const unsigned char* patchBytes);

/* Get the system wide parameters */
Tfa9887_Error_t Tfa9887_DspReadConfig(Tfa9887_handle_t handle, Tfa9887_Config_t pParameters);

/* load the system wide parameters */
Tfa9887_Error_t Tfa9887_DspWriteConfig(Tfa9887_handle_t handle, const Tfa9887_Config_t pParameters);

/* Select a speaker from the predefined list -> deprecated */
/* Tfa9887_Error_t Tfa9887_DspSelectSpeaker(Tfa9887_handle_t handle, Tfa9887_SpeakerType_t speaker); */
/* load explicitly the speaker parameters in case of free speaker, or when using a saved speaker model */
Tfa9887_Error_t Tfa9887_DspWriteSpeakerParameters(Tfa9887_handle_t handle, const Tfa9887_SpeakerParameters_t pParameters);
/* read the current speaker parameters as used by the SpeakerBoost processing */
Tfa9887_Error_t Tfa9887_DspReadSpeakerParameters(Tfa9887_handle_t handle, Tfa9887_SpeakerParameters_t pParameters);
/* load all the parameters for a preset from a file */
Tfa9887_Error_t Tfa9887_DspWritePreset(Tfa9887_handle_t handle, Tfa9887_Preset_t pPreset);
/* Get the parameters for a preset */
Tfa9887_Error_t Tfa9887_DspReadPreset(Tfa9887_handle_t handle, Tfa9887_Preset_t pParameters);
/* set the biquad coeeficient for the indicated biquad filter (index) */
Tfa9887_Error_t Tfa9887_DspBiquad_SetCoeff(Tfa9887_handle_t handle, int biquad_index, float b0, float b1, float b2, float a1, float a2);
/* load all biquads.  The coefficients are in the same order als the Tfa9887_DspBiquad_SetCoeff function b0, b1, ... */
Tfa9887_Error_t Tfa9887_DspBiquad_SetAllCoeff(Tfa9887_handle_t handle, float coef[TFA9887_BIQUAD_NUM*5]);
/* disable a certain biquad */
Tfa9887_Error_t Tfa9887_DspBiquad_Disable(Tfa9887_handle_t handle, int biquad_index);

/* Get the calibration result */
Tfa9887_Error_t Tfa9887_DspGetCalibrationImpedance(Tfa9887_handle_t handle, float *pRe25);

/* read the current status of the SpeakerBoost, typically used for development, not essential to be used in a product */
Tfa9887_Error_t Tfa9887_DspSpeakerBoost_GetStateInfo(Tfa9887_handle_t handle, Tfa9887_SpeakerBoost_StateInfo_t* pInfo);

/* optimized SetParam type of functions that allows writing the same parameters to multiple device with minimal delay between the devices */
Tfa9887_Error_t Tfa9887_DspWriteConfigMultiple(int handle_cnt, Tfa9887_handle_t handles[], const Tfa9887_Config_t pParameters);
Tfa9887_Error_t Tfa9887_DspWriteSpeakerParametersMultiple(int handle_cnt, Tfa9887_handle_t handles[], const Tfa9887_SpeakerParameters_t pParameters);
Tfa9887_Error_t Tfa9887_DspWritePresetMultiple(int handle_cnt, Tfa9887_handle_t handles[], Tfa9887_Preset_t pPreset);
Tfa9887_Error_t Tfa9887_DspBiquad_SetCoeffMultiple(int handle_cnt, Tfa9887_handle_t handles[], int biquad_index, float b0, float b1, float b2, float a1, float a2);
Tfa9887_Error_t Tfa9887_DspBiquad_SetAllCoeffMultiple(int handle_cnt, Tfa9887_handle_t handles[], float coef[TFA9887_BIQUAD_NUM*5]);
Tfa9887_Error_t Tfa9887_DspBiquad_DisableMultiple(int handle_cnt, Tfa9887_handle_t handles[], int biquad_index);

/* low level routines, not part of official API and might be removed in the future */
Tfa9887_Error_t Tfa9887_ReadRegister16(Tfa9887_handle_t handle, unsigned char subaddress, unsigned short *pValue);
Tfa9887_Error_t Tfa9887_WriteRegister16(Tfa9887_handle_t handle, unsigned char subaddress, unsigned short value);
Tfa9887_Error_t Tfa9887_DspReadMem(Tfa9887_handle_t handle, unsigned short start_offset, int num_words, int *pValues);
Tfa9887_Error_t Tfa9887_DspWriteMem(Tfa9887_handle_t handle, unsigned short address, int value);
Tfa9887_Error_t Tfa9887_DspSetParam(Tfa9887_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, const unsigned char data[]);
Tfa9887_Error_t Tfa9887_DspSetParamMultiple(int handle_cnt, Tfa9887_handle_t handles[], unsigned char module_id, unsigned char param_id, int num_bytes, const unsigned char data[]);
	Tfa9887_Error_t Tfa9887_DspGetParam(Tfa9887_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, unsigned char data[]);

/* Read data from the address
 * @param unsigned char subaddress, int num_bytes, unsigned char data[]
 */
Tfa9887_Error_t Tfa9887_ReadData(Tfa9887_handle_t handle,
                 unsigned char subaddress, int num_bytes, unsigned char data[]);

/* support for converting error codes into text */
const char* Tfa9887_GetErrorString(Tfa9887_Error_t error);

#ifdef __cplusplus
}
#endif

#endif // TFA9887_H
