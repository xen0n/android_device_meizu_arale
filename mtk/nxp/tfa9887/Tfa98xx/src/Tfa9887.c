#include "../inc/Tfa9887.h"
#include "../inc/Tfa98xx.h"
#include <string.h>
#include "Tfa98xx_internals.h"
#include <assert.h>
#include <utils/Log.h>

#define LOG_TAG "Tfa9887"

Tfa9887_Error_t Tfa9887_Open(unsigned char slave_address,
							 Tfa9887_handle_t * pHandle)
{
	return Tfa98xx_Open(slave_address, pHandle);
}

Tfa9887_Error_t Tfa9887_Init(Tfa9887_handle_t handle)
{
	return Tfa98xx_Init(handle);
}

Tfa9887_Error_t Tfa9887_Close(Tfa9887_handle_t handle)
{
	return Tfa98xx_Close(handle);
}

Tfa9887_Error_t Tfa9887_Powerdown(Tfa9887_handle_t handle,
								  int powerdown)
{
	return Tfa98xx_Powerdown(handle, powerdown);
}

Tfa9887_Error_t Tfa9887_SetConfigured(Tfa9887_handle_t handle)
{
	return Tfa98xx_SetConfigured(handle);
}

Tfa9887_Error_t Tfa9887_SelectAmplifierInput(Tfa9887_handle_t handle,
											 Tfa9887_AmpInputSel_t
											 input_sel)
{
	return Tfa98xx_SelectAmplifierInput(handle, input_sel);
}

Tfa9887_Error_t Tfa9887_SelectI2SOutputLeft(Tfa9887_handle_t handle,
											Tfa9887_OutputSel_t
											output_sel)
{
	return Tfa98xx_SelectI2SOutputLeft(handle, output_sel);
}

Tfa9887_Error_t Tfa9887_SelectI2SOutputRight(Tfa9887_handle_t handle,
											 Tfa9887_OutputSel_t
											 output_sel)
{
	return Tfa98xx_SelectI2SOutputRight(handle, output_sel);
}

Tfa9887_Error_t Tfa9887_SelectStereoGainChannel(Tfa9887_handle_t handle,
												Tfa9887_StereoGainSel_t
												gain_sel)
{
	return Tfa98xx_SelectStereoGainChannel(handle, gain_sel);
}

Tfa9887_Error_t Tfa9887_SetVolume(Tfa9887_handle_t handle, float voldB)
{
	return Tfa98xx_SetVolume(handle, voldB);
}

Tfa9887_Error_t Tfa9887_GetVolume(Tfa9887_handle_t handle,
								  float *pVoldB)
{
	return Tfa98xx_GetVolume(handle, pVoldB);
}

Tfa9887_Error_t Tfa9887_SetSampleRate(Tfa9887_handle_t handle,
									  int rate)
{
	return Tfa98xx_SetSampleRate(handle, rate);
}

Tfa9887_Error_t Tfa9887_GetSampleRate(Tfa9887_handle_t handle,
									  int *pRate)
{
	return Tfa98xx_GetSampleRate(handle, pRate);
}

Tfa9887_Error_t Tfa9887_SelectChannel(Tfa9887_handle_t handle,
									  Tfa9887_Channel_t channel)
{
	return Tfa98xx_SelectChannel(handle, channel);
}

Tfa9887_Error_t Tfa9887_SetMute(Tfa9887_handle_t handle,
								Tfa9887_Mute_t mute)
{
	return Tfa98xx_SetMute(handle, mute);
}

Tfa9887_Error_t Tfa9887_GetMute(Tfa9887_handle_t handle,
								Tfa9887_Mute_t * pMute)
{
	return Tfa98xx_GetMute(handle, (Tfa98xx_Mute_t*)pMute);
}

Tfa9887_Error_t Tfa9887_DspPatch(Tfa9887_handle_t handle,
								 int patchLength,
								 const unsigned char *patchBytes)
{
	return Tfa98xx_DspPatch(handle, patchLength, patchBytes);
}

Tfa9887_Error_t Tfa9887_DspWriteConfig(Tfa9887_handle_t handle,
									   const Tfa9887_Config_t pParameters)
{
	return Tfa98xx_DspWriteConfig(handle, TFA9887_CONFIG_LENGTH, pParameters);
}

Tfa9887_Error_t Tfa9887_DspWriteSpeakerParameters(Tfa9887_handle_t
												  handle, const Tfa9887_SpeakerParameters_t pParameters)
{
	return Tfa98xx_DspWriteSpeakerParameters(handle, TFA9887_SPEAKERPARAMETER_LENGTH, pParameters);
}

Tfa9887_Error_t Tfa9887_DspReadSpeakerParameters(Tfa9887_handle_t
												 handle, Tfa9887_SpeakerParameters_t pParameters)
{
	return Tfa98xx_DspReadSpeakerParameters(handle, TFA9887_SPEAKERPARAMETER_LENGTH, pParameters);
}

/* load all the parameters for a preset from a file */
Tfa9887_Error_t Tfa9887_DspWritePreset(Tfa9887_handle_t handle,
									   Tfa9887_Preset_t pPreset)
{
	return Tfa98xx_DspWritePreset(handle, TFA9887_PRESET_LENGTH, pPreset);
}

Tfa9887_Error_t Tfa9887_DspBiquad_SetCoeff(Tfa9887_handle_t handle,
										   int biquad_index, float b0,
										   float b1, float b2, float a1,
										   float a2)
{
	return Tfa98xx_DspBiquad_SetCoeff(handle, biquad_index, b0, b1, b2, a1, a2);
}

Tfa9887_Error_t Tfa9887_DspBiquad_SetAllCoeff(Tfa9887_handle_t handle,
											  float
											  coef[TFA9887_BIQUAD_NUM *
												   5])
{
	return Tfa98xx_DspBiquad_SetAllCoeff(handle, coef);
}

Tfa9887_Error_t Tfa9887_DspBiquad_Disable(Tfa9887_handle_t handle,
										  int biquad_index)
{
	return Tfa98xx_DspBiquad_Disable(handle, biquad_index);
}

Tfa9887_Error_t Tfa9887_DspGetCalibrationImpedance(Tfa9887_handle_t
												   handle,
												   float *pRe25)
{
	return Tfa98xx_DspGetCalibrationImpedance(handle, pRe25);
}

Tfa9887_Error_t Tfa9887_DspSpeakerBoost_GetStateInfo(Tfa9887_handle_t
													 handle,
													 Tfa9887_SpeakerBoost_StateInfo_t
													 * pInfo)
{
#define SB_STATE_SIZE 8
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

    unsigned char bytes[3 * SB_STATE_SIZE];
    int24 data[SB_STATE_SIZE];
    assert(pInfo != 0);
    error =
        Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST, SB_PARAM_GET_STATE,
                            3 * SB_STATE_SIZE, bytes);
    if (error != Tfa98xx_Error_Ok) {
            return error;
    }
    Tfa98xx_ConvertBytes2Data(3 * SB_STATE_SIZE, bytes, data);
    pInfo->agcGain = (float)data[0] / (1 << (23 - SPKRBST_AGCGAIN_EXP));    /* /2^23*2^(SPKRBST_AGCGAIN_EXP) */
    pInfo->limGain = (float)data[1] / (1 << (23 - SPKRBST_LIMGAIN_EXP));    /* /2^23*2^(SPKRBST_LIMGAIN_EXP) */
    pInfo->sMax = (float)data[2] / (1 << (23 - SPKRBST_HEADROOM));  /* /2^23*2^(SPKRBST_HEADROOM) */
    pInfo->T = data[3] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));     /* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
    pInfo->statusFlag = data[4];
    pInfo->X1 = (float)data[5] / (1 << (23 - SPKRBST_HEADROOM));    /* /2^23*2^(SPKRBST_HEADROOM) */
    pInfo->X2 = (float)data[6] / (1 << (23 - SPKRBST_HEADROOM));    /* /2^23*2^(SPKRBST_HEADROOM) */
    pInfo->Re = (float)data[7] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));     /* /2^23*2^(SPKRBST_TEMPERATURE_EXP) */
    return error;
}

Tfa9887_Error_t Tfa9887_DspWriteConfigMultiple(int handle_cnt,
											   Tfa9887_handle_t
											   handles[], const Tfa9887_Config_t pParameters)
{
	return Tfa98xx_DspWriteConfigMultiple(handle_cnt, handles, TFA9887_CONFIG_LENGTH, pParameters);
}

Tfa9887_Error_t Tfa9887_DspWriteSpeakerParametersMultiple(int
														  handle_cnt,
														  Tfa9887_handle_t
														  handles[],
														  const Tfa9887_SpeakerParameters_t pParameters)
{
	return Tfa98xx_DspWriteSpeakerParametersMultiple(handle_cnt, handles, TFA9887_SPEAKERPARAMETER_LENGTH, pParameters);
}

Tfa9887_Error_t Tfa9887_DspWritePresetMultiple(int handle_cnt,
											   Tfa9887_handle_t
											   handles[], Tfa9887_Preset_t pPreset)
{
	return Tfa98xx_DspWritePresetMultiple(handle_cnt, handles, TFA9887_PRESET_LENGTH, pPreset);
}

Tfa9887_Error_t Tfa9887_DspBiquad_SetCoeffMultiple(int handle_cnt,
												   Tfa9887_handle_t
												   handles[],
												   int biquad_index,
												   float b0, float b1,
												   float b2, float a1,
												   float a2)
{
	return Tfa98xx_DspBiquad_SetCoeffMultiple(handle_cnt, handles, biquad_index, b0, b1, b2, a1, a2);
}

Tfa9887_Error_t Tfa9887_DspBiquad_SetAllCoeffMultiple(int handle_cnt,
													  Tfa9887_handle_t
													  handles[],
													  float
													  coef
													  [TFA9887_BIQUAD_NUM
													   * 5])
{
	return Tfa98xx_DspBiquad_SetAllCoeffMultiple(handle_cnt, handles, coef);
}

Tfa9887_Error_t Tfa9887_DspBiquad_DisableMultiple(int handle_cnt,
												  Tfa9887_handle_t
												  handles[],
												  int biquad_index)
{
	return Tfa98xx_DspBiquad_DisableMultiple(handle_cnt, handles, biquad_index);
}

/* low level routines, not part of official API and might be removed in the future */
Tfa9887_Error_t Tfa9887_ReadRegister16(Tfa9887_handle_t handle,
									   unsigned char subaddress,
									   unsigned short *pValue)
{
	return Tfa98xx_ReadRegister16(handle, subaddress, pValue);
}

Tfa9887_Error_t Tfa9887_WriteRegister16(Tfa9887_handle_t handle,
										unsigned char subaddress,
										unsigned short value)
{
	return Tfa98xx_WriteRegister16(handle, subaddress, value);
}

Tfa9887_Error_t Tfa9887_DspReadMem(Tfa9887_handle_t handle,
								   unsigned short start_offset,
								   int num_words, int *pValues)
{
	return Tfa98xx_DspReadMem(handle, start_offset, num_words, pValues);
}

Tfa9887_Error_t Tfa9887_DspWriteMem(Tfa9887_handle_t handle,
									unsigned short address, int value)
{
	return Tfa98xx_DspWriteMem(handle, address, value);
}

Tfa9887_Error_t Tfa9887_DspSetParam(Tfa9887_handle_t handle,
									unsigned char module_id,
									unsigned char param_id,
									int num_bytes,
									const unsigned char data[])
{
	return Tfa98xx_DspSetParam(handle, module_id, param_id, num_bytes, data);
}

Tfa9887_Error_t Tfa9887_DspSetParamMultiple(int handle_cnt,
											Tfa9887_handle_t handles[],
											unsigned char module_id,
											unsigned char param_id,
											int num_bytes,
											const unsigned char data[])
{
	return Tfa98xx_DspSetParamMultiple(handle_cnt, handles, module_id, param_id, num_bytes, data);
}
Tfa9887_Error_t Tfa9887_DspGetParam(Tfa9887_handle_t handle,
									unsigned char module_id,
									unsigned char param_id,
									int num_bytes,
									unsigned char data[])
{
	return Tfa98xx_DspGetParam(handle, module_id, param_id, num_bytes, data);
}

Tfa9887_Error_t
Tfa9887_DspExecuteRpc(Tfa9887_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id, int num_inbytes, unsigned char indata[],
                    int num_outbytes, unsigned char outdata[])
{
	return Tfa98xx_DspExecuteRpc(handle, module_id, param_id, num_inbytes, indata, num_outbytes, outdata);
}


/* support for converting error codes into text */
const char *Tfa9887_GetErrorString(Tfa9887_Error_t error)
{
	return Tfa98xx_GetErrorString(error);
}

Tfa9887_Error_t
Tfa9887_ReadData(Tfa9887_handle_t handle,
                 unsigned char subaddress, int num_bytes, unsigned char data[])
{
	return Tfa98xx_ReadData(handle, subaddress, num_bytes, data);
}
Tfa9887_Error_t
Tfa9887_WriteData(Tfa9887_handle_t handle,
                 unsigned char subaddress, int num_bytes, unsigned char data[])
{
	return Tfa98xx_WriteData(handle, subaddress, num_bytes, data);
}
