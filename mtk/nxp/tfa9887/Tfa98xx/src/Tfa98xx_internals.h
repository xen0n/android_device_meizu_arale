#ifndef TFA98XX_INTERNALS_H
#define TFA98XX_INTERNALS_H

#include "../inc/Tfa98xx.h"

/* the following type mappings are compiler specific */
typedef int int24;
typedef unsigned char subaddress_t;

/*
 * early defs aligned with (tbd) generated names
 */
/* power-on/reset values */
#define TFA98XX_SYSTEM_CONTROL_POR		(0x025D)/*For BCK, 425D for WS*/
#define TFA98XX_CALIBRATION_POR			(0x3800)

/* SystemControl bits */
#define TFA98XX_SYSTEM_CONTROL_PWDN     (1<<0)
#define TFA98XX_SYSTEM_CONTROL_I2CR		(1<<1)
#define TFA98XX_SYSTEM_CONTROL_CFE      (1<<2)
#define TFA98XX_SYSTEM_CONTROL_AMPE     (1<<3)
#define TFA98XX_SYSTEM_CONTROL_DCA      (1<<4) // DC-to-DC converter mode select
#define TFA98XX_SYSTEM_CONTROL_SBSL   	(1<<5)
#define TFA98XX_SYSTEM_CONTROL_AMPC 	(1<<6)


/* CurrentSense registers */
#define TFA98XX_CURRENTSENSE1			(0x46)
#define TFA98XX_CURRENTSENSE2			(0x47)
#define TFA98XX_CURRENTSENSE3			(0x48)
#define TFA98XX_CURRENTSENSE4			(0x49)

#define TFA98XX_REVISION       (subaddress_t)0x03
#define TFA98XX_I2S_CONTROL    (subaddress_t)0x04
#define TFA98XX_AUDIO_CONTROL  (subaddress_t)0x06
#define TFA98XX_CALIBRATION    (subaddress_t)0x08
#define TFA98XX_SYSTEM_CONTROL (subaddress_t)0x09
#define TFA98XX_I2S_SEL        (subaddress_t)0x0A
//#define TFA98XX_CF_CONTROLS    (subaddress_t)0x70 //TODO cleanup reg defs
//#define TFA98XX_CF_MAD         (subaddress_t)0x71
//#define TFA98XX_CF_MEM         (subaddress_t)0x72
//#define TFA98XX_CF_STATUS      (subaddress_t)0x73

/* I2S_CONTROL bits */
#define TFA98XX_I2SCTRL_RATE_SHIFT (12)
#define TFA98XX_I2SCTRL_RATE_08000 (0<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_11025 (1<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_12000 (2<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_16000 (3<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_22050 (4<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_24000 (5<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_32000 (6<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_44100 (7<<TFA98XX_I2SCTRL_RATE_SHIFT)
#define TFA98XX_I2SCTRL_RATE_48000 (8<<TFA98XX_I2SCTRL_RATE_SHIFT)

#define TFA98XX_I2SCTRL_CHANSEL_SHIFT      3
#define TFA98XX_I2SCTRL_INPUT_SEL_SHIFT    6

#define TFA98XX_I2SCTRL_DATAI2_SHIFT      5

#define TFA98XX_I2SSEL_I2SOUT_LEFT_SHIFT  0
#define TFA98XX_I2SSEL_I2SOUT_RIGHT_SHIFT 3


/* SYSTEM CONTROL bits */
#define TFA98XX_SYSCTRL_POWERDOWN    TFA98XX_SYSTEM_CONTROL_PWDN
#define TFA98XX_SYSCTRL_RESETI2C     TFA98XX_SYSTEM_CONTROL_I2CR
#define TFA98XX_SYSCTRL_ENBL_AMP     TFA98XX_SYSTEM_CONTROL_AMPE
#define TFA98XX_SYSCTRL_DCA          TFA98XX_SYSTEM_CONTROL_DCA
#define TFA98XX_SYSCTRL_CONFIGURED   TFA98XX_SYSTEM_CONTROL_SBSL
#define TFA98XX_SYSCTRL_SEL_ENBL_AMP TFA98XX_SYSTEM_CONTROL_AMPE

/* Audio control bits */
#define TFA98XX_AUDIOCTRL_MUTE       (1<<5)

/* module Ids */
#define MODULE_FRAMEWORK        0
#define MODULE_SPEAKERBOOST     1
#define MODULE_BIQUADFILTERBANK 2

/* RPC commands */
#define SB_PARAM_SET_LSMODEL      0x06  // Load a full model into SpeakerBoost.
#define SB_PARAM_SET_EQ			  0x0A  // 2 Equaliser Filters.
#define SB_PARAM_SET_PRESET       0x0D  // Load a preset
#define SB_PARAM_SET_CONFIG		  0x0E  // Load a config
#define SB_PARAM_SET_DBDRC        0x0F
#define SB_PARAM_SET_AGCINS       0x10

#define SB_PARAM_GET_RE0          0x85  /* gets the speaker calibration impedance (@25 degrees celsius) */
#define SB_PARAM_GET_LSMODEL      0x86  // Gets current LoudSpeaker Model.
#define SB_PARAM_GET_CONFIG_PRESET 0x80
#define SB_PARAM_GET_STATE        0xC0

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

/* maximum number of bytes in 1 I2C write transaction */
#define MAX_I2C_LENGTH			NXP_I2C_MAX_SIZE

#define TFA98XX_GENERIC_SLAVE_ADDRESS 0x6C

typedef struct {
	int in_use;
	unsigned char slave_address;
	int rev;
} Tfa98xx_handle_private_t;


#define TFA98XX_CF_RESET  1

#define BIQUAD_COEFF_SIZE       6

#define EQ_COEFF_SIZE           7

/* the number of elements in Tfa98xx_SpeakerBoost_StateInfo */
#define FW_STATE_SIZE             9
#define FW_STATEDBDRC_SIZE        14    /* extra elements for DBDRC */

#define SPKRBST_HEADROOM			7					  /* Headroom applied to the main input signal */
#define SPKRBST_AGCGAIN_EXP			SPKRBST_HEADROOM	  /* Exponent used for AGC Gain related variables */
#define SPKRBST_TEMPERATURE_EXP     9
#define SPKRBST_LIMGAIN_EXP			4			          /* Exponent used for Gain Corection related variables */
#define SPKRBST_TIMECTE_EXP         1
#define DSP_MAX_GAIN_EXP            7

#define FEATURE1_TCOEF              0x100            /* bit8 set means tCoefA expected */
#define FEATURE1_DBDRC              0x200            /* bit8 set means tCoefA expected */


Tfa98xx_Error_t Tfa98xx_WriteData(Tfa98xx_handle_t handle, unsigned char subaddress, int num_bytes, const unsigned char data[]);
Tfa98xx_Error_t Tfa98xx_ReadData(Tfa98xx_handle_t handle, unsigned char subaddress, int num_bytes, unsigned char data[]);
Tfa98xx_Error_t Tfa98xx_DspSetParam(Tfa98xx_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, const unsigned char data[]);
Tfa98xx_Error_t Tfa98xx_DspGetParam(Tfa98xx_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, unsigned char data[]);

void Tfa98xx_ConvertData2Bytes(int num_data, const int24 data[], unsigned char bytes[]);
void Tfa98xx_ConvertBytes2Data(int num_bytes, const unsigned char bytes[], int24 data[]);

#endif // TFA98XX_INTERNALS_H
