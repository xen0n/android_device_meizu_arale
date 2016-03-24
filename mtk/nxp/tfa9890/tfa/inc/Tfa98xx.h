#ifndef TFA98XX_H
#define TFA98XX_H

#ifdef __cplusplus
extern "C" {
#endif

#define TFA98XX_API_REV_MAJOR			 (2)         // major API rev
#define TFA98XX_API_REV_MINOR			 (9)         // minor
#define TFA98XX_API_REV_STR			    "2.9"

/* Maximum number of retries for DSP result
 * Keep this value low!
 * If certain calls require longer wait conditions, the 
 * application should poll, not the API 
 * The total wait time depends on device settings. Those 
 * are application specific.
 */
#define TFA98XX_WAITRESULT_NTRIES          40 
#define TFA98XX_WAITRESULT_NTRIES_LONG   2000

/* following lengths are in bytes */
#define TFA98XX_SPEAKERPARAMETER_LENGTH   423
#define TFA98XX_PRESET_LENGTH              87

/* make full the default */

  #define TFA98XX_CONFIG_LENGTH           165

/*
MUST BE CONSISTANT: either one uses opaque arrays of bytes, or not!!!
*/

typedef unsigned char Tfa98xx_SpeakerParameters_t[TFA98XX_SPEAKERPARAMETER_LENGTH];
typedef unsigned char Tfa98xx_Config_t[TFA98XX_CONFIG_LENGTH];
typedef unsigned char Tfa98xx_Preset_t[TFA98XX_PRESET_LENGTH];

/* Type containing all the possible errors that can occur
 *
 */
typedef enum Tfa98xx_Error {
		Tfa98xx_Error_Ok = 0,
		Tfa98xx_Error_DSP_not_running,  /* communication with the DSP failed, presumably because DSP not running */
		Tfa98xx_Error_Bad_Parameter,
		Tfa98xx_Error_NotOpen,  /* the given handle is not open */
		Tfa98xx_Error_OutOfHandles,     /* too many handles */
		Tfa98xx_Error_StateTimedOut,	/* the expected response did not occur within the expected time */

		Tfa98xx_Error_RpcBase = 100,
		Tfa98xx_Error_RpcBusy = 101,
		Tfa98xx_Error_RpcModId = 102,
		Tfa98xx_Error_RpcParamId = 103,
		Tfa98xx_Error_RpcInfoId = 104,
		Tfa98xx_Error_RpcNotAllowedSpeaker = 105,

		Tfa98xx_Error_Not_Implemented,
		Tfa98xx_Error_Not_Supported,
		Tfa98xx_Error_I2C_Fatal,        /* Fatal I2C error occurred */
		Tfa98xx_Error_I2C_NonFatal,     /* Nonfatal I2C error, and retry count reached */
		Tfa98xx_Error_Other = 1000
} Tfa98xx_Error_t;

typedef enum Tfa98xx_AmpInputSel {
		Tfa98xx_AmpInputSel_I2SLeft,
		Tfa98xx_AmpInputSel_I2SRight,
		Tfa98xx_AmpInputSel_DSP
} Tfa98xx_AmpInputSel_t;

typedef enum Tfa98xx_OutputSel {
		Tfa98xx_I2SOutputSel_CurrentSense,
		Tfa98xx_I2SOutputSel_DSP_Gain,
		Tfa98xx_I2SOutputSel_DSP_AEC,
		Tfa98xx_I2SOutputSel_Amp,
		Tfa98xx_I2SOutputSel_DataI3R,
		Tfa98xx_I2SOutputSel_DataI3L,
		Tfa98xx_I2SOutputSel_DcdcFFwdCur,
} Tfa98xx_OutputSel_t;

typedef enum Tfa98xx_StereoGainSel {
		Tfa98xx_StereoGainSel_Left,
		Tfa98xx_StereoGainSel_Right
} Tfa98xx_StereoGainSel_t;

#define TFA98XX_MAXPATCH_LENGTH (3*1024)

/* the number of biquads supported */
#define TFA98XX_BIQUAD_NUM              10

typedef enum Tfa98xx_Channel {
		Tfa98xx_Channel_L,
		Tfa98xx_Channel_R,
		Tfa98xx_Channel_L_R,
		Tfa98xx_Channel_Stereo
} Tfa98xx_Channel_t;

typedef enum Tfa98xx_Mute {
		Tfa98xx_Mute_Off,
		Tfa98xx_Mute_Digital,
		Tfa98xx_Mute_Amplifier
} Tfa98xx_Mute_t;

typedef enum Tfa98xx_SpeakerBoostStatusFlags {
		Tfa98xx_SpeakerBoost_Activity = 0,      /* Input signal activity. */
		Tfa98xx_SpeakerBoost_S_Ctrl,    /* S Control triggers the limiter */
		Tfa98xx_SpeakerBoost_Muted,     /* 1 when signal is muted */
		Tfa98xx_SpeakerBoost_X_Ctrl,    /* X Control triggers the limiter */
		Tfa98xx_SpeakerBoost_T_Ctrl,    /* T Control triggers the limiter */
		Tfa98xx_SpeakerBoost_NewModel,  /* New model is available */
		Tfa98xx_SpeakerBoost_VolumeRdy, /* 0 means stable volume, 1 means volume is still smoothing */
		Tfa98xx_SpeakerBoost_Damaged,   /* Speaker Damage detected  */
		Tfa98xx_SpeakerBoost_SignalClipping     /* Input Signal clipping detected */
} Tfa98xx_SpeakerBoostStatusFlags_t;



typedef struct Tfa98xx_StateInfo {
	    /* SpeakerBoost State */
		float agcGain;  /* Current AGC Gain value */
		float limGain;  /* Current Limiter Gain value */
		float sMax;     /* Current Clip/Lim threshold */
		int T;          /* Current Speaker Temperature value */
		int statusFlag; /* Masked bit word, see Tfa98xx_SpeakerBoostStatusFlags */
		float X1;       /* Current estimated Excursion value caused by Speakerboost gain control */
		float X2;       /* Current estimated Excursion value caused by manual gain setting */
		float Re;       /* Current Loudspeaker blocked resistance */
		/* Framework state */
		int   shortOnMips; /* increments each time a MIPS problem is detected on the DSP, only presenton newer ICs */
} Tfa98xx_StateInfo_t;

/* possible memory values for DMEM in CF_CONTROLs */
typedef enum {
	Tfa98xx_DMEM_PMEM=0,
	Tfa98xx_DMEM_XMEM=1,
	Tfa98xx_DMEM_YMEM=2,
	Tfa98xx_DMEM_IOMEM=3,
} Tfa98xx_DMEM_e;

/**
 *  register definition structure
 */
typedef struct regdef {
        unsigned char offset;         /**< subaddress offset */
        unsigned short pwronDefault;  /**< register contents after poweron */
        unsigned short pwronTestmask; /**< mask of bits not test */
        char *name;                                             /**< short register name */
} regdef_t;

typedef int Tfa98xx_handle_t;

/**
 * The TFA9887 slave address can be 0x68, 6A, 6C or 6E
 */
Tfa98xx_Error_t Tfa98xx_Open(unsigned char slave_address,
							 Tfa98xx_handle_t * pHandle);
/**
 * Check if device is opened.
 */
int handle_is_open(Tfa98xx_handle_t h);

/**
 * Load the default HW settings in the device
 */
Tfa98xx_Error_t Tfa98xx_Init(Tfa98xx_handle_t handle);

/**
 * Return the maximum nr of devices
 */
int Tfa98xx_MaxDevices(void);

/**
 * Initialize the I2C hal
 */
 /*
#ifdef WIN32
Tfa98xx_Error_t Tfa98xx_I2C_Init( void );
#endif
*/
/**
 * Close the instance handle
 */
Tfa98xx_Error_t Tfa98xx_Close(Tfa98xx_handle_t handle);

/* control the powerdown bit of the TFA9887
 * @param powerdown must be 1 or 0
 */
Tfa98xx_Error_t Tfa98xx_Powerdown(Tfa98xx_handle_t handle,
								  int powerdown);

/* Notify the DSP that all parameters have been loaded.
 * @param configured must be 1 or 0
 */
Tfa98xx_Error_t Tfa98xx_SetConfigured(Tfa98xx_handle_t handle);

/* control the input_sel bits of the TFA9887, to indicate what is sent to the amplfier and speaker
 * @param input_sel, see Tfa98xx_AmpInputSel_t
 */
Tfa98xx_Error_t Tfa98xx_SelectAmplifierInput(Tfa98xx_handle_t handle,
											 Tfa98xx_AmpInputSel_t
											 input_sel);

/* control the I2S left output of the TFA9887
 * @param output_sel, see Tfa98xx_OutputSel_t
 */
Tfa98xx_Error_t Tfa98xx_SelectI2SOutputLeft(Tfa98xx_handle_t handle,
											Tfa98xx_OutputSel_t
											output_sel);

/* control the I2S right output of the TFA9887
 * @param output_sel, see Tfa98xx_OutputSel_t
 */
Tfa98xx_Error_t Tfa98xx_SelectI2SOutputRight(Tfa98xx_handle_t handle,
											 Tfa98xx_OutputSel_t
											 output_sel);

/* indicates on which channel of DATAI2 the gain from the other TFA9887 IC is set
 * @param gain_sel, see Tfa98xx_StereoGainSel_t
 */
Tfa98xx_Error_t Tfa98xx_SelectStereoGainChannel(Tfa98xx_handle_t handle,
												Tfa98xx_StereoGainSel_t
												gain_sel);

/* control the volume of the DSP
 * @param voldB volume in dB.  must be between 0 and -inf
 */
Tfa98xx_Error_t Tfa98xx_SetVolume(Tfa98xx_handle_t handle, float voldB);

/* read the currently set volume
 * @param voldB volume in dB.
 */
Tfa98xx_Error_t Tfa98xx_GetVolume(Tfa98xx_handle_t handle,
								  float *pVoldB);

/* notify the TFA9887 of the sample rate of the I2S bus that will be used.
 * @param rate in Hz.  must be 32000, 44100 or 48000
 */
Tfa98xx_Error_t Tfa98xx_SetSampleRate(Tfa98xx_handle_t handle,
									  int rate);

/* read the TFA9887 of the sample rate of the I2S bus that will be used.
 * @param pRate pointer to rate in Hz. Will be one of 32000, 44100 or 48000 if successful
 */
Tfa98xx_Error_t Tfa98xx_GetSampleRate(Tfa98xx_handle_t handle,
									  int *pRate);

/* set the input channel to use
 * @param channel see Tfa98xx_Channel_t enumeration
 */
Tfa98xx_Error_t Tfa98xx_SelectChannel(Tfa98xx_handle_t handle,
									  Tfa98xx_Channel_t channel);

/* mute/unmute the audio
 * @param mute see Tfa98xx_Mute_t enumeration
 */
Tfa98xx_Error_t Tfa98xx_SetMute(Tfa98xx_handle_t handle,
								Tfa98xx_Mute_t mute);

Tfa98xx_Error_t Tfa98xx_GetMute(Tfa98xx_handle_t handle,
								Tfa98xx_Mute_t * pMute);

/* Yields the number of parameters to be used in Tfa98xx_DspWriteConfig()
 * @pointer to parameter count. Only assigned if return value == Tfa98xx_Error_Ok
 * @return error code
 */
Tfa98xx_Error_t Tfa98xx_DspConfigParameterCount(Tfa98xx_handle_t handle, int *pParamCount);

/* set or clear DSP reset signal
 * @param new state
 * @return error code
 */
Tfa98xx_Error_t Tfa98xx_DspReset(Tfa98xx_handle_t handle, int state);

/* check the state of the DSP subsystem
 * return ready = 1 when clocks are stable to allow safe DSP subsystem access
 * @param pointer to state flag, non-zero if clocks are not stable
 * @return error code
 */
Tfa98xx_Error_t Tfa98xx_DspSystemStable(Tfa98xx_handle_t handle, int *ready);

/* The following functions can only be called when the DSP is running (hence the 'Dsp' in the name)
 * - I2S clock must be active, 
 * - IC must be in operating mode
 */

/* patch the ROM code of the DSP */
Tfa98xx_Error_t Tfa98xx_DspPatch(Tfa98xx_handle_t handle,
								 int patchLength,
								 const unsigned char *patchBytes);

/* Check whether the DSP expects tCoef or tCoefA as last parameter in the speaker parameters
 * *pbSupporttCoef=1 when DSP expects tCoef, 
 * *pbSupporttCoef=0 when it expects tCoefA (and the elaborate woraround to calculate tCoefA from tCoef on the host) 
 */
Tfa98xx_Error_t Tfa98xx_DspSupporttCoef(Tfa98xx_handle_t handle, int *pbSupporttCoef);

/* return the device revision id
 */
unsigned short Tfa98xx_GetDeviceRevision( Tfa98xx_handle_t handle);


/* load the system wide parameters */
Tfa98xx_Error_t Tfa98xx_DspWriteConfig(Tfa98xx_handle_t handle,
									   int length,
									   const unsigned char
									   *pConfigBytes);
/* Get the system wide parameters */
Tfa98xx_Error_t Tfa98xx_DspReadConfig( Tfa98xx_handle_t handle, 
                                       int length, 
									   unsigned char *pConfigBytes);

/* load explicitly the speaker parameters in case of free speaker, */
/* or when using a saved speaker model */
Tfa98xx_Error_t Tfa98xx_DspWriteSpeakerParameters(Tfa98xx_handle_t handle, 
												  int length,
												  const unsigned char *pSpeakerBytes);

/* read the current speaker parameters as used by the SpeakerBoost processing */
Tfa98xx_Error_t Tfa98xx_DspReadSpeakerParameters(Tfa98xx_handle_t handle, 
												 int length,
												 unsigned char *pSpeakerBytes);

/* read the current speaker excursion model as used by SpeakerBoost processing */
Tfa98xx_Error_t Tfa98xx_DspReadExcursionModel(Tfa98xx_handle_t handle, 
												 int length,
												 unsigned char *pSpeakerBytes);

/* load all the parameters for a preset from a file */
Tfa98xx_Error_t Tfa98xx_DspWritePreset(Tfa98xx_handle_t handle,
									   int length,
									   const unsigned char *pPresetBytes);
/* Get the system wide parameters */
Tfa98xx_Error_t Tfa98xx_DspReadPreset( Tfa98xx_handle_t handle, 
                                       int length, 
									   unsigned char *pPresetBytes);


/* set the biquad coeeficient for the indicated biquad filter (index) */
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetCoeff(Tfa98xx_handle_t handle,
										   int biquad_index, float b0,
										   float b1, float b2, float a1,
										   float a2);
/* load all biquads.  The coefficients are in the same order as in the */
/* Tfa98xx_DspBiquad_SetCoeff function b0, b1, ...                     */
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetAllCoeff(Tfa98xx_handle_t handle,
											  float coef[TFA98XX_BIQUAD_NUM * 5]);

/* disable a certain biquad */
Tfa98xx_Error_t Tfa98xx_DspBiquad_Disable(Tfa98xx_handle_t handle,
										  int biquad_index);

/* Get the calibration result */
Tfa98xx_Error_t Tfa98xx_DspGetCalibrationImpedance(Tfa98xx_handle_t
												   handle,
												   float *pRe25);

/* read the current status of the DSP, typically used for development, */
/* not essential to be used in a product                               */
Tfa98xx_Error_t Tfa98xx_DspGetStateInfo(Tfa98xx_handle_t
													 handle,
													 Tfa98xx_StateInfo_t
													 * pInfo);

/* optimized SetParam type of functions that allows writing the same    */
/* parameters to multiple device with minimal delay between the devices */
Tfa98xx_Error_t Tfa98xx_DspWriteConfigMultiple(int handle_cnt,
											   Tfa98xx_handle_t
											   handles[], int length,
											   const unsigned char
											   *pConfigBytes);
Tfa98xx_Error_t Tfa98xx_DspWriteSpeakerParametersMultiple(int
														  handle_cnt,
														  Tfa98xx_handle_t
														  handles[],
														  int length,
														  const unsigned
														  char
														  *pSpeakerBytes);
Tfa98xx_Error_t Tfa98xx_DspWritePresetMultiple(int handle_cnt,
											   Tfa98xx_handle_t
											   handles[], int length,
											   const unsigned char
											   *pPresetBytes);


Tfa98xx_Error_t Tfa98xx_DspBiquad_SetCoeffMultiple(int handle_cnt,
												   Tfa98xx_handle_t
												   handles[],
												   int biquad_index,
												   float b0, float b1,
												   float b2, float a1,
												   float a2);
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetAllCoeffMultiple(int handle_cnt,
													  Tfa98xx_handle_t
													  handles[],
													  float
													  coef
													  [TFA98XX_BIQUAD_NUM
													   * 5]);
Tfa98xx_Error_t Tfa98xx_DspBiquad_DisableMultiple(int handle_cnt,
												  Tfa98xx_handle_t
												  handles[],
												  int biquad_index);


/* low level routines, not part of official API and might be removed in the future */
Tfa98xx_Error_t Tfa98xx_ReadRegister16(Tfa98xx_handle_t handle,
									   unsigned char subaddress,
									   unsigned short *pValue);
Tfa98xx_Error_t Tfa98xx_WriteRegister16(Tfa98xx_handle_t handle,
										unsigned char subaddress,
										unsigned short value);
Tfa98xx_Error_t Tfa98xx_DspReadMem(Tfa98xx_handle_t handle,
								   unsigned short start_offset,
								   int num_words, int *pValues);
Tfa98xx_Error_t Tfa98xx_DspWriteMem(Tfa98xx_handle_t handle,
									unsigned short address, int value);
Tfa98xx_Error_t Tfa98xx_DspSetParam(Tfa98xx_handle_t handle,
									unsigned char module_id,
									unsigned char param_id,
									int num_bytes,
									const unsigned char data[]);
Tfa98xx_Error_t Tfa98xx_DspSetParamMultiple(int handle_cnt,
											Tfa98xx_handle_t handles[],
											unsigned char module_id,
											unsigned char param_id,
											int num_bytes,
											const unsigned char data[]);
Tfa98xx_Error_t Tfa98xx_DspGetParam(Tfa98xx_handle_t handle,
									unsigned char module_id,
									unsigned char param_id,
									int num_bytes,
									unsigned char data[]);
Tfa98xx_Error_t
Tfa98xx_DspExecuteRpc(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id, int num_inbytes, unsigned char indata[],
                    int num_outbytes, unsigned char outdata[]);
Tfa98xx_Error_t
Tfa98xx_ReadData(Tfa98xx_handle_t handle,
                 unsigned char subaddress, int num_bytes, unsigned char data[]);
Tfa98xx_Error_t
Tfa98xx_WriteData(Tfa98xx_handle_t handle,
                  unsigned char subaddress, int num_bytes,
                  const unsigned char data[]);

Tfa98xx_Error_t Tfa98xx_DspWriteMemory(Tfa98xx_handle_t handle, Tfa98xx_DMEM_e which_mem, 
                   unsigned short start_offset, int num_words, int *pValues);
Tfa98xx_Error_t Tfa98xx_DspReadMemory(Tfa98xx_handle_t handle, Tfa98xx_DMEM_e which_mem, 
                   unsigned short start_offset, int num_words, int *pValues);

/* specific setting for initialization of different type of device */
/*
Tfa98xx_Error_t Tfa98xx_specific(Tfa98xx_handle_t handle);
*/

/* support for converting error codes into text */
const char *Tfa98xx_GetErrorString(Tfa98xx_Error_t error);

#ifdef __cplusplus
}
#endif

#endif                          // TFA98XX_H

