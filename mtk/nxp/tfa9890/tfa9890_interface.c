#include <Tfa98xx.h>
#include <Tfa98xx_Registers.h>
#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <NXP_I2C.h>
#ifndef WIN32
// need PIN access
#include <lxScribo.h>
#endif

#include <utils/Log.h>
#define LOG_TAG "TFA9890"
#include "tfa9890_cust.h"

//#define SPDIF_AUDIO
//#define USB_AUDIO

static int tfa_debug = 1;
#define TFA_LOGD(fmt, arg...) do {\
    if(tfa_debug) \
        ALOGD("[%s: %d]"fmt, __func__, __LINE__, ##arg); \
}while(0)
#define TFA_LOGE(fmt, arg...) ALOGE("[%s: %d]"fmt, __func__, __LINE__, ##arg)

#define assert(condition) do {\
    if(!(condition)) \
        ALOGE("[%s: %d]", __func__, __LINE__); \
}while(0)

#ifndef WIN32
#define Sleep(ms) usleep((ms)*1000)
#define _fileno fileno
#define _GNU_SOURCE   /* to avoid link issues with sscanf on NxDI? */
#endif

#define TRY_TIMES (10)
#define I2C_ADDRESS  0x68  //(TFA_I2CSLAVEBASE << 1)


int tfa98xxRun_trace=1;
#define TRACEIN  if(tfa98xxRun_trace) printf("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xxRun_trace) printf("Leave %s\n", __FUNCTION__);

int cli_verbose=0; /* verbose flag */
extern regdef_t regdefs[];
extern unsigned char  tfa98xxI2cSlave; // global for i2c access
unsigned char  tfa98xxI2cSlave=TFA_I2CSLAVEBASE; // global for i2c access
int NXP_I2C_verbose=0;

static Tfa98xx_handle_t g_handle;
static int bypass_dsp_incall = 0;
static int first = 0;
/* *INDENT-OFF* */
regdef_t regdefs[] = {
        { 0x00, 0x081d, 0xfeff, "statusreg"}, //ignore MTP busy bit
        { 0x01, 0x0, 0x0, "batteryvoltage"},
        { 0x02, 0x0, 0x0, "temperature"},
        { 0x03, 0x0012, 0xffff, "revisionnumber"},
        { 0x04, 0x888b, 0xffff, "i2sreg"},
        { 0x05, 0x13aa, 0xffff, "bat_prot"},
        { 0x06, 0x001f, 0xffff, "audio_ctr"},
        { 0x07, 0x0fe6, 0xffff, "dcdcboost"},
        { 0x08, 0x0800, 0x3fff, "spkr_calibration"}, //NOTE: this is a software fix to 0xcoo
        { 0x09, 0x041d, 0xffff, "sys_ctrl"},
        { 0x0a, 0x3ec3, 0x7fff, "i2s_sel_reg"},
        { 0x40, 0x0, 0x00ff, "hide_unhide_key"},
        { 0x41, 0x0, 0x0, "pwm_control"},
        { 0x46, 0x0, 0x0, "currentsense1"},
        { 0x47, 0x0, 0x0, "currentsense2"},
        { 0x48, 0x0, 0x0, "currentsense3"},
        { 0x49, 0x0, 0x0, "currentsense4"},
        { 0x4c, 0x0, 0xffff, "abisttest"},
        { 0x62, 0x0, 0, "mtp_copy"},
        { 0x70, 0x0, 0xffff, "cf_controls"},
        { 0x71, 0x0, 0, "cf_mad"},
        { 0x72, 0x0, 0, "cf_mem"},
        { 0x73, 0x00ff, 0xffff, "cf_status"},
        { 0x80, 0x0, 0, "mtp"},
        { 0x83, 0x0, 0, "mtp_re0"},
        { 0xff, 0,0, NULL}
};


/*
 *
 */
// status register errors to check for not 1
#define TFA98XX_STATUSREG_ERROR1_SET_MSK (  \
		TFA98XX_STATUSREG_OCDS  )
#define TFA98XX_STATUSREG_ERROR2_SET_MSK (  \
		TFA98XX_STATUSREG_ACS |   \
		TFA98XX_STATUSREG_WDS )

static int tfaRun_CheckEvents(unsigned short regval) {
	int severity=0;

	//see if following alarms are set
	if ( regval & TFA98XX_STATUSREG_ERROR1_SET_MSK ) //
		severity = 1;
	// next will overwrite if set
	if ( regval & TFA98XX_STATUSREG_ERROR2_SET_MSK )
		severity = 2;
	// check error conditions

	return severity;
}

Tfa98xx_Error_t tfaRun_PowerCycleCF(Tfa98xx_handle_t handle){
	Tfa98xx_Error_t err;

	TRACEIN

	err = Tfa98xx_Powerdown(handle, 1);
        if(err != Tfa98xx_Error_Ok) {
            ALOGE("FUNC: %s, LINE: %s", __func__, __LINE__);
            return err;
        }
	err = Tfa98xx_Powerdown(handle, 0);
        if(err != Tfa98xx_Error_Ok) {
            ALOGE("FUNC: %s, LINE: %s", __func__, __LINE__);
            return err;
        }

	return err;
}

static float tCoefFromSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes)
{
    int iCoef;

    /* tCoef(A) is the last parameter of the speaker */
    iCoef = (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-3]<<16)
                    + (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-2]<<8)
                    + speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-1];

     return (float)iCoef/(1<<23);
}

static void tCoefToSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes, float tCoef)
{
    int iCoef;

    iCoef =(int)(tCoef*(1<<23));

    speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-3] = (iCoef>>16)&0xFF;
    speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-2] = (iCoef>>8)&0xFF;
    speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-1] = (iCoef)&0xFF;
}

static void muteAmplifier(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;
    int tries = 0;

    /* signal the TFA98xx to mute plop free and turn off the amplifier */
    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
    if(err != Tfa98xx_Error_Ok) {
        ALOGE("FUNC: %s, LINE: %s", __func__, __LINE__);
        return err;
    }

    /* now wait for the amplifier to turn off */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    if(err != Tfa98xx_Error_Ok) {
        ALOGE("FUNC: %s, LINE: %s", __func__, __LINE__);
        return err;
    }

    do {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        if (err != Tfa98xx_Error_Ok) {
            ALOGE("FUNC: %s, LINE: %s", __func__, __LINE__);
            break;
        }
        if ( (status & TFA98XX_STATUSREG_SWS_MSK) == TFA98XX_STATUSREG_SWS_MSK) {
            tries++;
            usleep(1*1000);
        } else {
            break;
        }
    } while (tries < TRY_TIMES);
}

static void loadSpeakerFile(const char* fileName, Tfa98xx_SpeakerParameters_t speakerBytes)
{
    int ret;
    FILE* f;

    ALOGD("using speaker %s\n", fileName);

    f=fopen(fileName, "rb");
    if (NULL == f) {
        ALOGE("%s: %u: Not exist %s\n", __func__, __LINE__, fileName);
        return;
    }
    ret = fread(speakerBytes, 1, sizeof(Tfa98xx_SpeakerParameters_t), f);
    if (ret != sizeof(Tfa98xx_SpeakerParameters_t)) {
        ALOGE("fread for '%s' error(%d)", fileName, ret);
    }
    fclose(f);
}

static int dspSupporttCoef(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    int bSupporttCoef;

    err = Tfa98xx_DspSupporttCoef(handle, &bSupporttCoef);
    if(err != Tfa98xx_Error_Ok) {
        ALOGE("FUNC: %s, LINE: %s", __func__, __LINE__);
        return err;
    }

    return bSupporttCoef;
}


/* load a speaker model from a file, as generated by the GUI, or saved from a previous execution */
static void setSpeaker(Tfa98xx_handle_t handle, const char* fileName, Tfa98xx_SpeakerParameters_t speakerBytes)
{
    Tfa98xx_Error_t err;
    int ret;
    FILE* f;

    printf("using speaker %s\n", fileName);

    f=fopen(fileName, "rb");
    if (NULL == f) {
        printf("%s: %u: Not exist %s\n", __func__, __LINE__, fileName);
        return;
    }
    ret = fread(speakerBytes, 1, sizeof(Tfa98xx_SpeakerParameters_t), f);
    if (ret != sizeof(Tfa98xx_SpeakerParameters_t)) {
        ALOGE("fread for '%s' error(%d)", fileName, ret);
        goto close;
    }
    err = Tfa98xx_DspWriteSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), speakerBytes);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("");
        goto close;
    }
close:
    fclose(f);
}
/* save the current speaker model to a file, for future use */
static void saveSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes, const char* fileName)
{
    int ret;
    FILE* f;

    f=fopen(fileName, "wb");
    if (NULL == f) {
        TFA_LOGE("Not exist %s\n", fileName);
        return;
    }
    ret = fwrite(speakerBytes, 1, sizeof(Tfa98xx_SpeakerParameters_t), f);
    if(ret != sizeof(Tfa98xx_SpeakerParameters_t)) {
        TFA_LOGE("fwrite for '%s' error(%d)", fileName, ret);
    }
    fclose(f);
}

static void setConfig(Tfa98xx_handle_t handle, const char* fileName)
{
    Tfa98xx_Error_t err;
    Tfa98xx_Config_t config;
    int ret;
    FILE* f;

    TFA_LOGD("using config %s\n", fileName);

    /* read from file and check the size */
    f=fopen(fileName, "rb");
    if (NULL == f) {
        TFA_LOGE("Not exist %s\n", fileName);
        return;
    }
    ret = fread(config, 1, sizeof(config), f);
    if(ret != sizeof(config)) {
        TFA_LOGE("fread for '%s' error(%d)", fileName, ret);
    }
    /* TODO: solve in case TFA98XX_FULL */
    fclose(f);

    /* now send bytes to the DSP */
    err = Tfa98xx_DspWriteConfig(handle, sizeof(Tfa98xx_Config_t), config);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("");
    }
}

/* load a preset from a file, as generated by the GUI, can be done at runtime */
static void setPreset(Tfa98xx_handle_t handle, const char* fileName)
{
    int ret;
    int presetSize;
    unsigned char* buffer;
    FILE* f;
    struct stat st;
    Tfa98xx_Error_t err;

    TFA_LOGD("using preset %s\n", fileName);

    f=fopen(fileName, "rb");
    if (NULL == f) {
        TFA_LOGE("Not exist %s\n", fileName);
        return;
    }
    ret = fstat(_fileno(f), &st);
    if(ret < 0) {
        TFA_LOGE("fstat for '%s' error(%d)", fileName, ret);
        goto close;
    }
    presetSize = st.st_size;
    if(presetSize != TFA98XX_PRESET_LENGTH) {
        TFA_LOGE("presetSize(%d) != TFA98XX_PRESET_LENGTH", presetSize);
        goto close;
    }
    buffer = (unsigned char*)malloc(presetSize);
    if(buffer == NULL) {
        TFA_LOGE("malloc failed!");
        goto close;
    }
    ret = fread(buffer, 1, presetSize, f);
    if(ret != presetSize) {
        TFA_LOGE("fread for '%s' error(%d)", fileName, ret);
        goto free;
    }
    err = Tfa98xx_DspWritePreset(handle, TFA98XX_PRESET_LENGTH, buffer);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("");
    }
free:
    free(buffer);
close:
    fclose(f);
}


/* load a set of EQ settings from a file, as generated by the GUI, can be done at runtime */
static void setEQ(Tfa98xx_handle_t handle, const char* fileName)
{
    int ret;
    FILE* f;
    Tfa98xx_Error_t err;
    int ind; /* biquad index */
    float b0, b1, b2, a1, a2; /* the coefficients */
    int line = 1;
    char buffer[256];

    TFA_LOGD("using EQ %s\n", fileName);

    f=fopen(fileName, "rb");
    if (NULL == f) {
        TFA_LOGE("Not exist %s\n", fileName);
        return;
    }

    while (!feof(f)) {
        if (NULL == fgets(buffer, sizeof(buffer)-1, f) ) {
            break;
        }
        ret = sscanf(buffer, "%d %f %f %f %f %f", &ind, &b0, &b1, &b2, &a1, &a2);
        if (ret == 6) {
            if ((b0 != 1) || (b1 != 0) || (b2 != 0) || (a1 != 0) || (a2 != 0)) {
                err = Tfa98xx_DspBiquad_SetCoeff(handle, ind, b0, b1, b2, a1, a2);
                if(err != Tfa98xx_Error_Ok) {
                    TFA_LOGE("");
                    goto close;
                }
                TFA_LOGD("Loaded biquad %d\n", ind);
            } else {
                err = Tfa98xx_DspBiquad_Disable(handle, ind);
                if(err != Tfa98xx_Error_Ok) {
                    TFA_LOGE("");
                    goto close;
                }
                TFA_LOGD("Disabled biquad %d\n", ind);
            }
        }
        else {
            TFA_LOGD("error parsing file, line %d\n", line);
            //break;
        }
        line++;
    }
close:
    fclose(f);
}

/* load a DSP ROM code patch from file */
static void dspPatch(Tfa98xx_handle_t handle, const char* fileName)
{
    int ret;
    int fileSize;
    unsigned char* buffer;
    FILE* f;
    struct stat st;
    Tfa98xx_Error_t err;

    TFA_LOGD("Loading patch file %s\n", fileName);

    f=fopen(fileName, "rb");
    if (NULL == f) {
        TFA_LOGE("Not exist %s\n", fileName);
        return;
    }
    ret = fstat(_fileno(f), &st);
    if(ret < 0) {
        TFA_LOGE("fstat for '%s' error(%d)", fileName, ret);
        goto close;
    }
    fileSize = st.st_size;
    buffer = malloc(fileSize);
    if(buffer == NULL) {
        TFA_LOGE("malloc failed!");
        goto close;
    }
    ret = fread(buffer, 1, fileSize, f);
    if(ret != fileSize) {
        TFA_LOGE("fread for '%s' error(%d)", fileName, ret);
        goto free_;
    }
    err = Tfa98xx_DspPatch(handle, fileSize, buffer);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("");
    }
free_:
    free(buffer);
close:
    fclose(f);
}

static char* stateFlagsStr(int stateFlags)
{
    static char flags[10];

    flags[0] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_Activity)) ? 'A':'a';
    flags[1] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_S_Ctrl)) ? 'S':'s';
    flags[2] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_Muted)) ? 'M':'m';
    flags[3] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_X_Ctrl)) ? 'X':'x';
    flags[4] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_T_Ctrl)) ? 'T':'t';
    flags[5] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_NewModel)) ? 'L':'l';
    flags[6] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_VolumeRdy)) ? 'V':'v';
    flags[7] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_Damaged)) ? 'D':'d';
    flags[8] = (stateFlags & (0x1<<Tfa98xx_SpeakerBoost_SignalClipping)) ? 'C':'c';

    flags[9] = 0;
    return flags;
}

static void dump_state_info(Tfa98xx_StateInfo_t* pState)
{
  TFA_LOGD("state: flags %s, agcGain %2.1f\tlimGain %2.1f\tsMax %2.1f\tT %d\tX1 %2.1f\tX2 %2.1f\tRe %2.2f\tshortOnMips %d\n",
                stateFlagsStr(pState->statusFlag),
                pState->agcGain,
                pState->limGain,
                pState->sMax,
                pState->T,
                pState->X1,
                pState->X2,
                pState->Re,
                pState->shortOnMips);
}

static void load_all_settings(Tfa98xx_handle_t handle,
                            Tfa98xx_SpeakerParameters_t speakerBytes, const char* configFile,
                            const char* presetFile, const char* eqFile)
{
    Tfa98xx_Error_t err;

    /* load fullmodel */
    err = Tfa98xx_DspWriteSpeakerParameters(handle,
                                sizeof(Tfa98xx_SpeakerParameters_t), speakerBytes);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("");
        return;
    }

    /* load the settings */
    setConfig(handle, configFile);
    /* load a preset */
    setPreset(handle, presetFile);
    /* load an EQ file */
    setEQ(handle, eqFile);
}

static void waitCalibration(Tfa98xx_handle_t handle, int *calibrateDone)
{
    Tfa98xx_Error_t err;
    int tries = 0;
    unsigned short mtp;
#define WAIT_TRIES 50

    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("");
    }

    /* in case of calibrate once wait for MTPEX */
    if ( mtp & TFA98XX_MTP_MTPOTC) {
        while (tries < WAIT_TRIES) {   // TODO optimise with wait estimation
            err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
            *calibrateDone = ( mtp & TFA98XX_MTP_MTPEX);    /* check MTP bit1 (MTPEX) */
            if(*calibrateDone != 0) {
                break;
            }
            TFA_LOGD("waiting for TFA98XX_MTP, tries: %d", tries);
            usleep(20*1000);
            tries++;
        }
     } else {
        while (tries<WAIT_TRIES) {   // TODO optimise with wait estimation
            err = Tfa98xx_DspReadMem(handle, 231, 1, calibrateDone);
            if(err != Tfa98xx_Error_Ok || (*calibrateDone == 0)) {
                usleep(20*1000);
                tries++;
             } else {
                break;
            }
            TFA_LOGD("polling xmem for calibration, tries: %d", tries);
        }
    }

    TFA_LOGE("tires: %d", tries);
}

#ifdef SPDIF_AUDIO
typedef int  (__stdcall *SetPin_type)(unsigned char pinNumber, unsigned short value);
static SetPin_type SetPin;

void InitSpdifAudio()
{
  HMODULE hDll;
    unsigned char i2c_UDAmode2[] = {0x34, 0x00, 0x28, 0x2E};
    unsigned char i2c_UDAI2SSpdif[] = {0x34, 0x50, 0x01, 0x51};

    /* copied the relevant code from C:\Program Files\NXP\I2C\C\CrdI2c.c
     */
    hDll = LoadLibrary(L"Scribo.dll");
    if (hDll == 0) {
        fprintf(stderr, "Could not open Scribo.dll\n");
        return ;
    }

    SetPin = (SetPin_type) GetProcAddress(hDll, "SetPin");
    if (SetPin == NULL) {
        FreeLibrary(hDll);
        return; // function not found in library
    }

    SetPin(4, 0x1); /* Weak pull up on PA4 powers up the UDA1355 */
    NXP_I2C_Write(sizeof(i2c_UDAmode2), i2c_UDAmode2);
    NXP_I2C_Write(sizeof(i2c_UDAI2SSpdif), i2c_UDAI2SSpdif);
}
#endif
#ifdef USB_AUDIO
#ifdef WIN32
typedef int  (__stdcall *SetPin_type)(unsigned char pinNumber, unsigned short value);
static SetPin_type SetPin;

void InitUsbAudio()
{
    HMODULE hDll;
    int ret;

    /* copied the relevant code from C:\Program Files\NXP\I2C\C\CrdI2c.c
     */
    hDll = LoadLibrary(L"Scribo.dll");
    if (hDll == 0) {
        fprintf(stderr, "Could not open Scribo.dll\n");
        return ;
    }

    SetPin = (SetPin_type) GetProcAddress(hDll, "SetPin");
    if (SetPin == NULL) {
        FreeLibrary(hDll);
        return; // function not found in library
    }

    ret = SetPin(4, 0x8000); /* Active low on PA4 switches off UDA1355. */
}
#else
void InitUsbAudio()
{
    int fd;
    fd = lxScriboGetFd();
    lxScriboSetPin(fd, 4, 0x8000);
}
#endif
#endif /* USB_AUDIO */


static void resetMtpEx(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short mtp;
    unsigned short status;
    int tries = 0;

    /* reset MTPEX bit because calibration happened with wrong tCoefA */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
    assert(err == Tfa98xx_Error_Ok);
    /* all settings loaded, signal the DSP to start calibration, only needed once after cold boot */

    /* reset MTPEX bit if needed */
    if ( (mtp & TFA98XX_MTP_MTPOTC) && (mtp & TFA98XX_MTP_MTPEX))
    {
        err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A); /* unlock key2 */
        assert(err == Tfa98xx_Error_Ok);

        err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, 1); /* MTPOTC=1, MTPEX=0 */
        assert(err == Tfa98xx_Error_Ok);
        err = Tfa98xx_WriteRegister16(handle, 0x62, 1<<11); /* CIMTP=1 */
        assert(err == Tfa98xx_Error_Ok);
    }

    do
    {
        tries++;
        Sleep(10);
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        assert(err == Tfa98xx_Error_Ok);

    } while ( (status & TFA98XX_STATUSREG_MTPB_MSK) == TFA98XX_STATUSREG_MTPB_MSK
                    && tries < TRY_TIMES);
    if(tries == TRY_TIMES) {
        ALOGE("Waiting for TFA98XX_STATUSREG_MTPB timeouts!");
    }
}

#if defined(BYPASS_DSP) || defined(BYPASS_DSP_INCALL)
/*Switching to bypass mode:
1. Make sure device is in power down and muted.
2. Execute bypass function to set registers.
3. Warm start up with also checking SystemStable function.
4. Unmute and set DCDC again to follower mode. */
/*
 * Bypass DSP handling
 */
int nxptfa98xxbypassdsp(Tfa98xx_handle_t handlesIn, int bypass)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Other;
    unsigned short i2SRead = 0;
    unsigned short sysRead = 0;
    unsigned short sysCtrlRead = 0;

    TFA_LOGD("+");

    if( handlesIn == -1) {
        err = Tfa98xx_Open(I2C_ADDRESS, &handlesIn );
        if (err) return err;
    }

    err = Tfa98xx_ReadRegister16( handlesIn, TFA98XX_I2SREG, &i2SRead);
    err = Tfa98xx_ReadRegister16( handlesIn, TFA98XX_I2S_SEL_REG, &sysRead);
    err = Tfa98xx_ReadRegister16( handlesIn, TFA98XX_SYS_CTRL, &sysCtrlRead);

    if(bypass) {
        i2SRead &= ~(TFA98XX_I2SREG_CHSA_MSK); /* Set CHSA to bypass DSP */
        sysRead &= ~(TFA98XX_I2S_SEL_REG_DCFG_MSK);/* Set DCDC compensation to off */
        sysRead |= TFA98XX_I2S_SEL_REG_SPKR_MSK; /* Set impedance as 8ohm */
        sysCtrlRead &= ~(TFA98XX_SYS_CTRL_DCA_MSK);/* Set DCDC to follower mode */
        sysCtrlRead &= ~(TFA98XX_SYS_CTRL_CFE_MSK);/* Disable coolflux */
    } else {
        i2SRead |= TFA98XX_I2SREG_CHSA_MSK; /* Set CHSA to bypass DSP */
        sysRead |= TFA98XX_I2S_SEL_REG_DCFG_MSK;/* Set DCDC compensation to off */
        sysRead &= ~(TFA98XX_I2S_SEL_REG_SPKR_MSK); /* Set impedance as 8ohm */
        sysCtrlRead |= TFA98XX_SYS_CTRL_DCA_MSK;/* Set DCDC to follower mode */
        sysCtrlRead |= TFA98XX_SYS_CTRL_CFE_MSK;/* Disable coolflux */
    }

    /* Set CHSA to bypass DSP */
    err = Tfa98xx_WriteRegister16( handlesIn, TFA98XX_I2SREG, i2SRead);
    /* Set DCDC compensation to off and set impedance as 8ohm */
    err = Tfa98xx_WriteRegister16( handlesIn, TFA98XX_I2S_SEL_REG, sysRead);
    /* Set DCDC to follower mode and disable coolflux  */
    err = Tfa98xx_WriteRegister16( handlesIn, TFA98XX_SYS_CTRL, sysCtrlRead);

    return err;
}
#endif

static void coldStartup(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;
    int ready = 0;
    int tries = 0;
    ALOGD("Tfa98xx: + coldStartup");
    /* load the optimal TFA98xx in HW settings */
    err = Tfa98xx_Init(handle);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_Init error(%d)", err);
    }

   /* Set sample rate to example 44100*/
    err = Tfa98xx_SetSampleRate(handle, SAMPLE_RATE);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_SetSampleRate error(%d)", err);
    }

#ifdef BYPASS_DSP
    err = nxptfa98xxbypassdsp(handle, 1);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("nxptfa98xxbypassdsp error(%d)", err);
        return;
    }
#endif

    /* Power On the device by setting bit 0 to 0 of register 9*/
   err = Tfa98xx_Powerdown(handle, 0);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_Powerdown to 0 error(%d)", err);
    }

    TFA_LOGD("Waiting for IC to start up\n");

   /* Check the PLL is powered up from status register 0*/
    while (tries < TRY_TIMES)
    {
        /* not ok yet */
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        if(err != Tfa98xx_Error_Ok || (status & TFA98XX_STATUSREG_AREFS_MSK) == 0) {
            tries++;
            usleep(1*1000);
         } else {
            break;
        }
    }

    TFA_LOGD("status: 0x%04x, tries: %d", status, tries);
#ifndef BYPASS_DSP
    /*  powered on
     *    - now it is allowed to access DSP specifics
     *    - stall DSP by setting reset
     * */
    err = Tfa98xx_DspReset(handle, 1);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGD("Tfa98xx_DspReset to 1 error(%d)", err);
    }

    /*  wait until the DSP subsystem hardware is ready
     *    note that the DSP CPU is not running yet (RST=1)
     * */
    tries = 0;
    while (tries < TRY_TIMES)
    {
        /* are we ready? */
    	err = Tfa98xx_DspSystemStable(handle, &ready);
        if(err != Tfa98xx_Error_Ok || ready == 0) {
            tries++;
         } else {
            break;
        }
    }
    TFA_LOGD("ready: 0x%04x, tries: %d, err: %d", ready, tries, err);

   /* Load cold-boot patch for the first time to force cold start-up.
    *  use the patchload only to write the internal register
    * */
    dspPatch(handle, LOCATION_FILES COLDBOOTPATCH_FILENAME);

    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_ReadRegister16 TFA98XX_STATUSREG error(%d)", err);
    }

    if(!(status & TFA98XX_STATUSREG_ACS_MSK)) {/* ensure cold booted */
        TFA_LOGE("not cold booted yet");
    }

    /* cold boot, need to load all parameters and patches */
#ifdef PATCH_FILENAME
    /* patch the ROM code */
    dspPatch(handle, LOCATION_FILES PATCH_FILENAME);
#else
	printf("No patchfile.\n");
	/* in this case start the DSP */
	err = Tfa98xx_DspReset(handle, 0);
        if(err != Tfa98xx_Error_Ok) {
            TFA_LOGE("Tfa98xx_DspReset to 0 error(%d)", err);
        }
#endif
#endif//BYPASS_DSP
    ALOGD("Tfa98xx: - coldStartup");
}

static void statusCheck(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;

   /* Check status from register 0*/
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
   if (status & TFA98XX_STATUSREG_WDS_MSK)
   {
      printf("DSP watchDog triggerd");
      return;
   }
}

static void setOtc(Tfa98xx_handle_t handle, int otcOn)
{
    Tfa98xx_Error_t err;
    unsigned short mtp;
    unsigned short status;
    int mtpChanged = 0;
    int tries = 0;
    ALOGD("Tfa98xx: + setOtc");
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
    assert(err == Tfa98xx_Error_Ok);

    assert((otcOn == 0) || (otcOn == 1) );

    /* set reset MTPEX bit if needed */
    if ( (mtp & TFA98XX_MTP_MTPOTC) != otcOn)
    {
        /* need to change the OTC bit, set MTPEX=0 in any case */
        err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A); /* unlock key2 */
        assert(err == Tfa98xx_Error_Ok);

        err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, (unsigned short)otcOn); /* MTPOTC=otcOn, MTPEX=0 */
        assert(err == Tfa98xx_Error_Ok);
        err = Tfa98xx_WriteRegister16(handle, 0x62, 1<<11); /* CIMTP=1 */
        assert(err == Tfa98xx_Error_Ok);

        mtpChanged =1;

    }
    //Sleep(13*16); /* need to wait until all parameters are copied into MTP */
    do
    {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        if(err != Tfa98xx_Error_Ok
            || (status & TFA98XX_STATUSREG_MTPB_MSK) == TFA98XX_STATUSREG_MTPB_MSK ) {
            tries++;
            Sleep(10);
        } else {
            break;
        }
    } while (tries < TRY_TIMES);
    TFA_LOGE("status: 0x%x, tries: %d", status, tries);
    ALOGD("Tfa98xx: - setOtc");
}

static int checkMTPEX(Tfa98xx_handle_t handle)
{
    unsigned short mtp;
    Tfa98xx_Error_t err;
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
    assert(err == Tfa98xx_Error_Ok);

    if ( mtp & (1<<1))  /* check MTP bit1 (MTPEX) */
        return 1;                   /* MTPEX is 1, calibration is done */
    else
        return 0;                   /* MTPEX is 0, calibration is not done yet */
}


int tfa9890_deinit(void)
{
	 Tfa98xx_handle_t *handle = g_handle;
	 Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
   if(first)
   {
	 	  err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
	      assert(err == Tfa98xx_Error_Ok);

	      err = Tfa98xx_Powerdown(handle, 1);
	      assert(err == Tfa98xx_Error_Ok);

	      err = Tfa98xx_Close(handle);
	      assert(err == Tfa98xx_Error_Ok);
	      first = 0;
	 }

	 return 1;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int tfa9890_init()
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle;
    Tfa98xx_SpeakerParameters_t loadedSpeaker;
    Tfa98xx_StateInfo_t stateInfo;
    unsigned short status = 0;
    float re25 = 0.0;
    float tCoefA = 0.0;
    int calibrateDone = 0;

    ALOGD("%s +",__func__);

    /* create handle */
    err = Tfa98xx_Open(I2C_ADDRESS, &handle);
    if(err != Tfa98xx_Error_Ok) {
        ALOGE("[%s] Tfa98xx_Open failed!", __func__);
        return -1;
    }

    g_handle = handle;
    coldStartup(handle);

#ifndef BYPASS_DSP
    /* load current speaker file to structure */
    loadSpeakerFile(LOCATION_FILES SPEAKER_FILENAME, loadedSpeaker);

    /*Set to calibration once*/
    /* Only needed for really first time calibration */
    setOtc(handle, 1);

    /* Check if MTPEX bit is set for calibration once mode */
    if(checkMTPEX(handle) == 0)
    {
        TFA_LOGD("DSP not yet calibrated. Calibration will start.\n");

        /* ensure no audio during special calibration */
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        if (err != Tfa98xx_Error_Ok) {
            TFA_LOGE("Tfa98xx_SetMute to Tfa98xx_Mute_Digital failed(%d)", err);
        }
    }
    else
    {
        TFA_LOGD("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
    }
    err = Tfa98xx_DspWriteSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), loadedSpeaker);
    if (err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_DspWriteSpeakerParameters failed(%d)", err);
    }
    /* load the settings */
    setConfig(handle, LOCATION_FILES CONFIG_FILENAME);
    /* load a preset */
    setPreset(handle, LOCATION_FILES PRESET_FILENAME);
    /* set the equalizer to Rock mode */
    setEQ(handle, LOCATION_FILES EQ_FILENAME);

    /* all settings loaded, signal the DSP to start calibration */

    calibrateDone = 0;

    err = Tfa98xx_SetConfigured(handle);
    if (err != Tfa98xx_Error_Ok) {
        TFA_LOGE("");
    }

    waitCalibration(handle, &calibrateDone);
    if (calibrateDone)
    {
        Tfa98xx_DspGetCalibrationImpedance(handle,&re25);
        TFA_LOGD("Calibration value is %2.2f ohm\n", re25);
        /*Checking the current status for DSP status and DCPVP */
        statusCheck(handle);
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        assert(err == Tfa98xx_Error_Ok);
    }
    else
    {
        re25 = 0;
        TFA_LOGE("Calibration failed\n");
    }
#endif //BYPASS_DSP
    err = Tfa98xx_Powerdown(handle, 1);
    assert(err == Tfa98xx_Error_Ok);

    {
    // set battery safeguard control to be active, addr. 0x05, bit[15]=1->0,active
    // set battery supply safeguard threshold as 3.53V, bit[5:2] = 0x1000
        unsigned short iRead = 0;
        err = Tfa98xx_ReadRegister16( handle, TFA98XX_BAT_PROT, &iRead);
        iRead &= ~(1<<TFA98XX_BAT_PROT_BSSBY_POS);
        iRead &= ~(0xF<<TFA98XX_BAT_PROT_BSST_POS);
        iRead |= (8<<TFA98XX_BAT_PROT_BSST_POS);
        err = Tfa98xx_WriteRegister16( handle, TFA98XX_BAT_PROT, iRead);
    }
    first = 1;
    ALOGD("%s -",__func__);
    return 1;
}

void tfa9890_set_bypass_dsp_incall(int bypass)
{
    TFA_LOGD("bypass: %d", bypass);
    bypass_dsp_incall = bypass == 0 ? 0 : 1;
}

static int get_bypass_dsp_incall()
{
    return bypass_dsp_incall;
}

void tfa9890_SpeakerOn(void)
{
    TFA_LOGD("+");
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;

    ALOGD("%s +, first_flag= %d",__func__,first);
    if (!first) {
         tfa9890_init(SAMPLE_RATE);
    }
#ifdef BYPASS_DSP_INCALL
    if(get_bypass_dsp_incall()) {
        err = nxptfa98xxbypassdsp(handle, 1);
        if(err != Tfa98xx_Error_Ok) {
            TFA_LOGD("nxptfa98xxbypassdsp error(%d)", err);
        }
    } else {
        err = nxptfa98xxbypassdsp(handle, 0);
        if(err != Tfa98xx_Error_Ok) {
            TFA_LOGD("nxptfa98xxbypassdsp error(%d)", err);
        }
    }
#endif

    err = Tfa98xx_Powerdown(handle, 0);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGD("Tfa98xx_Powerdown to 0 error(%d)", err);
    }
    // Sleep 15ms to wait for DSP stabled
    usleep(25*1000);

#ifdef BYPASS_DSP_INCALL
    if(get_bypass_dsp_incall()) {
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off + 10);
    } else {
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);
    }
#else
    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);
#endif
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGD("Tfa98xx_SetMute to Tfa98xx_Mute_Off error(%d)", err);
        return;
    }

    TFA_LOGD("-");
}

void tfa9890_SpeakerOff(void)
{
    TFA_LOGD("+");
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;

    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
    usleep(70*1000);

    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGD("Tfa98xx_SetMute to Tfa98xx_Mute_Digital error(%d)", err);
        return;
    }
    err = Tfa98xx_Powerdown(handle, 1);
    if(err != Tfa98xx_Error_Ok) {
        TFA_LOGD("Tfa98xx_Powerdown to 1 error(%d)", err);
        return;
    }

    TFA_LOGD("-");
}
//set samplerate at tfa9890 powerdown
void tfa9890_setSamplerate(int samplerate)
{
    ALOGD("%s +",__func__);
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;

    if ((8000 == samplerate) || (16000 == samplerate)
                    ||(48000 == samplerate) || (44100 == samplerate)) {
        err = Tfa98xx_Powerdown(handle, 1);
        assert(err == Tfa98xx_Error_Ok);
        err = Tfa98xx_SetSampleRate(handle, samplerate);
        assert(err == Tfa98xx_Error_Ok);
        err = Tfa98xx_Powerdown(handle, 0);
        assert(err == Tfa98xx_Error_Ok);
    } else {
        ALOGI("%s Samplerate = %d is not support",__func__, samplerate);
    }
    ALOGD("%s -",__func__);
}

void tfa9890_EchoReferenceConfigure(int config)
{
    ALOGD("%s +, flag= %d",__func__,config);
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;
    if(config)
    {
       Tfa98xx_SelectI2SOutputLeft(handle, Tfa98xx_I2SOutputSel_DSP_AEC);
       TFA_LOGD("tfa9890_EchoReferenceConfigure Left, error(%d)", err);
       assert(err == Tfa98xx_Error_Ok);

       Tfa98xx_SelectI2SOutputRight(handle, Tfa98xx_I2SOutputSel_DSP_AEC);
       TFA_LOGD("tfa9890_EchoReferenceConfigure Right, error(%d)", err);
       assert(err == Tfa98xx_Error_Ok);
    }
    ALOGD("%s -, flag= %d",__func__,config);
}
int tfa9890_check_tfaopen(void)
{
   return first;
}

void tfa9890_reset(void)
{
    first = 0;
}

EXPORT_SYMBOL(tfa9890_init);
EXPORT_SYMBOL(tfa9890_deinit);
EXPORT_SYMBOL(tfa9890_reset);
EXPORT_SYMBOL(tfa9890_eqset);
EXPORT_SYMBOL(tfa9890_SpeakerOff);
EXPORT_SYMBOL(tfa9890_SpeakerOn);
EXPORT_SYMBOL(tfa9890_setSamplerate);
EXPORT_SYMBOL(tfa9890_set_bypass_dsp_incall);
EXPORT_SYMBOL(tfa9890_EchoReferenceConfigure);
EXPORT_SYMBOL(tfa9890_check_tfaopen);

