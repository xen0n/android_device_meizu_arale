
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <utils/Log.h>
#if defined(NXP_TFA9890_SUPPORT)

#elif defined(NXP_TFA9887_SUPPORT)
#include "tfa9887/interface/tfa9887_interface.h"
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
int MTK_Tfa98xx_Check_TfaOpen(void);

int MTK_Tfa98xx_Check_TfaOpen(void)
{
#if defined(NXP_TFA9890_SUPPORT)
   return tfa9890_check_tfaopen();
#elif defined(NXP_TFA9887_SUPPORT)
   return tfa9887_check_tfaopen();
#endif

}

int MTK_Tfa98xx_Init(void)
{
    int res;
    ALOGD("Tfa98xx: +%s",__func__);
#if defined(NXP_TFA9890_SUPPORT)
    res = tfa9890_init();
#elif defined(NXP_TFA9887_SUPPORT)
    res = tfa9887_init();
#endif
    ALOGD("Tfa98xx: -%s, res= %d",__func__,res);
    return res;
}

int MTK_Tfa98xx_Deinit(void)
{
	  int res = 0;
    ALOGD("Tfa98xx: +%s",__func__);
    if(MTK_Tfa98xx_Check_TfaOpen())
    {
#if defined(NXP_TFA9890_SUPPORT)
        res = tfa9890_deinit();
#elif defined(NXP_TFA9887_SUPPORT)
        res = tfa9887_deinit();
#endif
    }
    ALOGD("Tfa98xx: -%s, res= %d",__func__, res);
    return res;
}

void MTK_Tfa98xx_SpeakerOn(void)
{
    ALOGD("Tfa98xx: +%s, nxp_init_flag= %d",__func__,MTK_Tfa98xx_Check_TfaOpen());
    if(!MTK_Tfa98xx_Check_TfaOpen())                   //already done in tfa9890_SpeakerOn()
    	MTK_Tfa98xx_Init();

#if defined(NXP_TFA9890_SUPPORT)
    tfa9890_SpeakerOn();
#elif defined(NXP_TFA9887_SUPPORT)
    tfa9887_SpeakerOn();
#endif
    ALOGD("Tfa98xx: -%s, nxp_init_flag= %d",__func__,MTK_Tfa98xx_Check_TfaOpen());
}

void MTK_Tfa98xx_SpeakerOff(void)
{
    ALOGD("Tfa98xx: +%s",__func__);

#if defined(NXP_TFA9890_SUPPORT)
    tfa9890_SpeakerOff();
    usleep(10*1000);
#elif defined(NXP_TFA9887_SUPPORT)
    tfa9887_SpeakerOff();
    usleep(10*1000);
#endif

    ALOGD("Tfa98xx: -%s",__func__);
}

void MTK_Tfa98xx_SetSampleRate(int samplerate)
{
    ALOGD("Tfa98xx: +%s, samplerate=%d",__func__,samplerate);

#if defined(NXP_TFA9890_SUPPORT)
    tfa9890_setSamplerate(samplerate);
#elif defined(NXP_TFA9887_SUPPORT)
    tfa9887_setSamplerate(samplerate);
#endif

    ALOGD("Tfa98xx: -%s",__func__);
}
void MTK_Tfa98xx_SetBypassDspIncall(int bypass)
{
    ALOGD("Tfa98xx: +%s, bypass= %d",__func__,bypass);
#if defined(NXP_TFA9890_SUPPORT)
    tfa9890_set_bypass_dsp_incall(bypass);
#elif defined(NXP_TFA9887_SUPPORT)
    tfa9887_set_bypass_dsp_incall(bypass);
#endif
    ALOGD("Tfa98xx: -%s",__func__);
}

void MTK_Tfa98xx_EchoReferenceConfigure(int config)
{
    ALOGD("Tfa98xx: +%s, nxp_init_flag= %d, config= %d",__func__,MTK_Tfa98xx_Check_TfaOpen(), config);
    if(MTK_Tfa98xx_Check_TfaOpen())
    {
#if defined(NXP_TFA9890_SUPPORT)
       tfa9890_EchoReferenceConfigure(config);
#elif defined(NXP_TFA9887_SUPPORT)
       tfa9887_EchoReferenceConfigure(config);
#endif
    }
    ALOGD("Tfa98xx: -%s,",__func__);
}

void MTK_Tfa98xx_Reset()
{
    ALOGD("MTK_Tfa98xx_Reset");
#if defined(NXP_TFA9890_SUPPORT)
    tfa9890_deinit();
    tfa9890_reset();
#elif defined(NXP_TFA9887_SUPPORT)
    tfa9887_deinit();
    tfa9887_reset();
#endif
    ALOGD("Tfa98xx: -%s,",__func__);
}

EXPORT_SYMBOL(MTK_Tfa98xx_Check_TfaOpen);
EXPORT_SYMBOL(MTK_Tfa98xx_Init);
EXPORT_SYMBOL(MTK_Tfa98xx_Reset);
EXPORT_SYMBOL(MTK_Tfa98xx_Deinit);
EXPORT_SYMBOL(MTK_Tfa98xx_SpeakerOn);
EXPORT_SYMBOL(MTK_Tfa98xx_SpeakerOff);
EXPORT_SYMBOL(MTK_Tfa98xx_SetSampleRate);
EXPORT_SYMBOL(MTK_Tfa98xx_SetBypassDspIncall);
EXPORT_SYMBOL(MTK_Tfa98xx_EchoReferenceConfigure);

