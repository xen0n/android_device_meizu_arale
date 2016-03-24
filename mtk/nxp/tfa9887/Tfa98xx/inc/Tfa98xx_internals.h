#ifndef TFA98XX_INTERNALS_H
#define TFA98XX_INTERNALS_H

#include "Tfa98xx.h"

/* the following type mappings are compiler specific */
typedef int int24;
typedef unsigned char subaddress_t;

/* i2c defs are in inc/Tfa98xx_Registers.h */

/* module Ids */
#define MODULE_FRAMEWORK        0
#define MODULE_SPEAKERBOOST     1
#define MODULE_BIQUADFILTERBANK 2

/* RPC commands */
#define SB_PARAM_SET_LSMODEL      0x06  // Load a full model into SpeakerBoost.
#define SB_PARAM_SET_EQ			  0x0A  // 2 Equaliser Filters. 
#define SB_PARAM_SET_PRESET       0x0D  // Load a preset
#define SB_PARAM_SET_CONFIG		  0x0E  // Load a config 
#define SB_PARAM_SET_DRC          0x0F
#define SB_PARAM_SET_AGCINS       0x10 

#define SB_PARAM_GET_RE0          0x85  /* gets the speaker calibration impedance (@25 degrees celsius) */
#define SB_PARAM_GET_LSMODEL      0x86  // Gets current LoudSpeaker Model.
#define SB_PARAM_GET_CONFIG_PRESET 0x80
#define SB_PARAM_GET_STATE        0xC0
#define SB_PARAM_GET_XMODEL       0xC1  // Gets current Excursion Model.

#define FW_PARAM_GET_STATE        0x84
#define FW_PARAM_GET_FEATURE_BITS 0x85

/* RPC Status results */
#define STATUS_OK                  0
#define STATUS_INVALID_MODULE_ID   2
#define STATUS_INVALID_PARAM_ID    3
#define STATUS_INVALID_INFO_ID     4


/* the maximum message length in the communication with the DSP */
#define MAX_PARAM_SIZE (145*3)

#define MIN(a,b) ((a)<(b)?(a):(b))
#define ROUND_DOWN(a,n) (((a)/(n))*(n))

#define TFA98XX_GENERIC_SLAVE_ADDRESS 0x1C

typedef enum {
	supportNo,
	supportYes,
	supportNotSet
} featureSupport_t;

typedef struct {
	int in_use;
	unsigned char slave_address;
	unsigned short rev;
	featureSupport_t supportDrc;
	featureSupport_t supportFramework;
} Tfa98xx_handle_private_t;


#define TFA98XX_CF_RESET  1

#define BIQUAD_COEFF_SIZE       6

#define EQ_COEFF_SIZE           7

/* the number of elements in Tfa98xx_SpeakerBoost_StateInfo */
#define FW_STATE_SIZE             9 

  #define FW_STATE_MAX_SIZE       FW_STATE_SIZE

#define SPKRBST_HEADROOM			7					  /* Headroom applied to the main input signal */
#define SPKRBST_AGCGAIN_EXP			SPKRBST_HEADROOM	  /* Exponent used for AGC Gain related variables */
#define SPKRBST_TEMPERATURE_EXP     9
#define SPKRBST_LIMGAIN_EXP			4			          /* Exponent used for Gain Corection related variables */
#define SPKRBST_TIMECTE_EXP         1
#define DSP_MAX_GAIN_EXP            7

#define FEATURE1_TCOEF              0x100            /* bit8 set means tCoefA expected */
#define FEATURE1_DRC                0x200            /* bit9 NOT set means DRC expected */

Tfa98xx_Error_t Tfa98xx_DspSupportFramework(Tfa98xx_handle_t, int *pbSupportFramework);
//Tfa98xx_Error_t Tfa98xx_WriteData(Tfa98xx_handle_t handle, unsigned char subaddress, int num_bytes, const unsigned char data[]);
//Tfa98xx_Error_t Tfa98xx_ReadData(Tfa98xx_handle_t handle, unsigned char subaddress, int num_bytes, unsigned char data[]);
//Tfa98xx_Error_t Tfa98xx_DspSetParam(Tfa98xx_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, const unsigned char data[]);
//Tfa98xx_Error_t Tfa98xx_DspGetParam(Tfa98xx_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, unsigned char data[]);
//Tfa98xx_Error_t Tfa98xx_specific(Tfa98xx_handle_t handle);
Tfa98xx_Error_t Tfa98xx_DspReadSpkrParams(Tfa98xx_handle_t handle, unsigned char paramId, int length, unsigned char *pSpeakerBytes);
Tfa98xx_Error_t Tfa98xx_DspSetParamVarWait(Tfa98xx_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, const unsigned char data[], int waitRetryCount);
Tfa98xx_Error_t Tfa98xx_DspSetParamMultipleVarWait(int handle_cnt, Tfa98xx_handle_t handles[], unsigned char module_id, unsigned char param_id, int num_bytes, const unsigned char data[], int waitRetryCount);

void Tfa98xx_ConvertData2Bytes(int num_data, const int24 data[], unsigned char bytes[]);
void Tfa98xx_ConvertBytes2Data(int num_bytes, const unsigned char bytes[], int24 data[]);





#endif // TFA98XX_INTERNALS_H
