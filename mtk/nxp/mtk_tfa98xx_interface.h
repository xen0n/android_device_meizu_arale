#ifndef _MTK_TFA98XX_HEADFILE_
#define _MTK_TFA98XX_HEADFILE_


#ifdef __cplusplus
extern "C" {
#endif

int  MTK_Tfa98xx_Init(void);
int  MTK_Tfa98xx_Deinit(void);
void MTK_Tfa98xx_SpeakerOn(void);
void MTK_Tfa98xx_Reset(void);
void MTK_Tfa98xx_SpeakerOff(void);
void MTK_Tfa98xx_SetSampleRate(int samplerate);
void MTK_Tfa98xx_SetBypassDspIncall(int bypass);
void MTK_Tfa98xx_EchoReferenceConfigure(int config);
#ifdef __cplusplus
}
#endif

#endif //_MTK_TFA98XX_HEADFILE_

