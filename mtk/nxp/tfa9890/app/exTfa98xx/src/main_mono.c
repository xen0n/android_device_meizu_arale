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

//#define SPDIF_AUDIO
#define USB_AUDIO


#ifndef WIN32

#define Sleep(ms) usleep((ms)*1000)
#define _fileno fileno
#define _GNU_SOURCE   /* to avoid link issues with sscanf on NxDI? */
#endif

#define I2C_ADDRESS   0x68  //cruson modify TFA9890 SLAVE ADDRESS  0x6C->68
#define SAMPLE_RATE 48000

#ifdef WIN32
// cwd = dir where vcxproj is
#define LOCATION_FILES "../../../../settings/"
#else
// cwd = linux dir
#define LOCATION_FILES "../../../settings/"
#endif


/* the base speaker file, containing tCoef */
/* Dumbo speaker
#define SPEAKER_FILENAME "KS_13X18_DUMBO.speaker"
#define PRESET_FILENAME "Example_0_0.KS_13X18_DUMBO.preset"
#define EQ_FILENAME "Example_0_0.KS_13X18_DUMBO.eq"*/
/* Sambo speaker*/
#define SPEAKER_FILENAME "KS_90_13x18_Sambo_V2R2.speaker"
#define PRESET_FILENAME "Seltech_HQ_V2R2_0_0_KS_90_13x18_Sambo_V2R2.preset"
#define EQ_FILENAME "Seltech_HQ_V2R2_0_0_KS_90_13x18_Sambo_V2R2.eq"
#define CONFIG_FILENAME "Setup.config"
#define PATCH_FILENAME "TFA9890_N1C2_1_5_1.patch"
// nor B12 #define PATCH_FILENAME "TFA9890_N1B12_3_2_3.patch"

int tfa98xxRun_trace=1;
#define TRACEIN  if(tfa98xxRun_trace) printf("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xxRun_trace) printf("Leave %s\n", __FUNCTION__);

int cli_verbose=0;	/* verbose flag */
#define TFA_I2CSLAVEBASE		(0x34)              // tfa device slave address of 1st (=left) device  cruson  0x36->34
extern regdef_t regdefs[];
extern unsigned char  tfa98xxI2cSlave; // global for i2c access
unsigned char  tfa98xxI2cSlave=TFA_I2CSLAVEBASE; // global for i2c access
int NXP_I2C_verbose=0;


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
	assert(err == Tfa98xx_Error_Ok);
	err = Tfa98xx_Powerdown(handle, 0);
	assert(err == Tfa98xx_Error_Ok);

	return err;
}

static float tCoefFromSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes)
{
    int iCoef;

    /* tCoef(A) is the last parameter of the speaker */
    iCoef = (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-3]<<16) + (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-2]<<8) + speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-1];

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

    /* signal the TFA98xx to mute plop free and turn off the amplifier */
    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
    assert(err == Tfa98xx_Error_Ok);

    /* now wait for the amplifier to turn off */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    assert(err == Tfa98xx_Error_Ok);
    while ( (status & TFA98XX_STATUSREG_SWS_MSK) == TFA98XX_STATUSREG_SWS_MSK)
    {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        assert(err == Tfa98xx_Error_Ok);
    }
}

static void loadSpeakerFile(const char* fileName, Tfa98xx_SpeakerParameters_t speakerBytes)
{
    int ret;
    FILE* f;

    printf("using speaker %s\n", fileName);

    f=fopen(fileName, "rb");
    assert(f != NULL);
    ret = fread(speakerBytes, 1, sizeof(Tfa98xx_SpeakerParameters_t), f);
    assert(ret == sizeof(Tfa98xx_SpeakerParameters_t));
    fclose(f);
}

static int dspSupporttCoef(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    int bSupporttCoef;

    err = Tfa98xx_DspSupporttCoef(handle, &bSupporttCoef);
    assert(err == Tfa98xx_Error_Ok);

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
    assert(f!=NULL);
    ret = fread(speakerBytes, 1, sizeof(Tfa98xx_SpeakerParameters_t), f);
    assert(ret == sizeof(Tfa98xx_SpeakerParameters_t));
    err = Tfa98xx_DspWriteSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), speakerBytes);
    assert(err == Tfa98xx_Error_Ok);
    fclose(f);
}
/* save the current speaker model to a file, for future use */
static void saveSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes, const char* fileName)
{
    int ret;
    FILE* f;

    f=fopen(fileName, "wb");
    assert(f!=NULL);
    ret = fwrite(speakerBytes, 1, sizeof(Tfa98xx_SpeakerParameters_t), f);
    assert(ret == sizeof(Tfa98xx_SpeakerParameters_t));
    fclose(f);
}

static void setConfig(Tfa98xx_handle_t handle, const char* fileName)
{
    Tfa98xx_Error_t err;
    Tfa98xx_Config_t config;
    int ret;
    FILE* f;

    printf("using config %s\n", fileName);

    /* read from file and check the size */
    f=fopen(fileName, "rb");
    assert(f!=NULL);
    ret = fread(config, 1, sizeof(config), f);

    /* TODO: solve in case TFA98XX_FULL */

    fclose(f);

    /* now send bytes to the DSP */
    err = Tfa98xx_DspWriteConfig(handle, sizeof(Tfa98xx_Config_t), config);
    assert(err == Tfa98xx_Error_Ok);
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
    
    printf("using preset %s\n", fileName);

    f=fopen(fileName, "rb");
    assert(f!=NULL);
    ret = fstat(_fileno(f), &st);
    assert(ret == 0);
    presetSize = st.st_size;
    assert(presetSize == TFA98XX_PRESET_LENGTH);
    buffer = (unsigned char*)malloc(presetSize);
    assert(buffer != NULL);
    ret = fread(buffer, 1, presetSize, f);
    assert(ret == presetSize);
    err = Tfa98xx_DspWritePreset(handle, TFA98XX_PRESET_LENGTH, buffer);
    assert(err == Tfa98xx_Error_Ok);
    fclose(f);
    free(buffer);
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

    printf("using EQ %s\n", fileName);

    f=fopen(fileName, "rb");
    assert(f!=NULL);

    while (!feof(f))
    {
        if (NULL == fgets(buffer, sizeof(buffer)-1, f) )
        {
            break;
        }
        ret = sscanf(buffer, "%d %f %f %f %f %f", &ind, &b0, &b1, &b2, &a1, &a2);
        if (ret == 6)
        {
            if ((b0 != 1) || (b1 != 0) || (b2 != 0) || (a1 != 0) || (a2 != 0)) {
                err = Tfa98xx_DspBiquad_SetCoeff(handle, ind, b0, b1, b2, a1, a2);
                assert(err == Tfa98xx_Error_Ok);
                printf("Loaded biquad %d\n", ind);
      } else {
        err = Tfa98xx_DspBiquad_Disable(handle, ind);
                assert(err == Tfa98xx_Error_Ok);
                printf("Disabled biquad %d\n", ind);
            }
        }
        else {
            printf("error parsing file, line %d\n", line);
            //break;
        }
        line++;
    }
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

    printf("Loading patch file %s\n", fileName);

    f=fopen(fileName, "rb");
    assert(f!=NULL);
    ret = fstat(_fileno(f), &st);
    assert(ret == 0);
    fileSize = st.st_size;
    buffer = malloc(fileSize);
    assert(buffer != NULL);
    ret = fread(buffer, 1, fileSize, f);
    assert(ret == fileSize);
    err = Tfa98xx_DspPatch(handle, fileSize, buffer);
    assert(err == Tfa98xx_Error_Ok);
    fclose(f);
    free(buffer);
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
  printf("state: flags %s, agcGain %2.1f\tlimGain %2.1f\tsMax %2.1f\tT %d\tX1 %2.1f\tX2 %2.1f\tRe %2.2f\tshortOnMips %d\n",
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

static void load_all_settings(Tfa98xx_handle_t handle, Tfa98xx_SpeakerParameters_t speakerBytes, const char* configFile, const char* presetFile, const char* eqFile)
{
    Tfa98xx_Error_t err;

    /* load fullmodel */
    err = Tfa98xx_DspWriteSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), speakerBytes);
    assert(err == Tfa98xx_Error_Ok);

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
#define WAIT_TRIES 1000

    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);

    /* in case of calibrate once wait for MTPEX */
    if ( mtp & TFA98XX_MTP_MTPOTC) {
        while ( (*calibrateDone == 0) && (tries < WAIT_TRIES))
        {   // TODO optimise with wait estimation
            err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
            *calibrateDone = ( mtp & TFA98XX_MTP_MTPEX);    /* check MTP bit1 (MTPEX) */
            tries++;
        }
    } else /* poll xmem for calibrate always */
    {
        while ((*calibrateDone == 0) && (tries<WAIT_TRIES) )
        {   // TODO optimise with wait estimation
            err = Tfa98xx_DspReadMem(handle, 231, 1, calibrateDone);
            tries++;
        }
        if(tries==WAIT_TRIES)
            printf("calibrateDone 231 timedout\n");
    }

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
        Sleep(10);
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        assert(err == Tfa98xx_Error_Ok);
        
    } while ( (status & TFA98XX_STATUSREG_MTPB_MSK) == TFA98XX_STATUSREG_MTPB_MSK);
    assert( (status & TFA98XX_STATUSREG_MTPB_MSK) == 0);
}

static void coldStartup(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;
    int ready = 0;

    /* load the optimal TFA98xx in HW settings */
    err = Tfa98xx_Init(handle);
    assert(err == Tfa98xx_Error_Ok);

   /* Set sample rate to example 44100*/
    err = Tfa98xx_SetSampleRate(handle, SAMPLE_RATE);
    assert(err == Tfa98xx_Error_Ok);

    /* Power On the device by setting bit 0 to 0 of register 9*/
   err = Tfa98xx_Powerdown(handle, 0);
    assert(err == Tfa98xx_Error_Ok);

    printf("Waiting for IC to start up\n");
 
   /* Check the PLL is powered up from status register 0*/
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    assert(err == Tfa98xx_Error_Ok);
    while ( (status & TFA98XX_STATUSREG_AREFS_MSK) == 0)
    {
        /* not ok yet */
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        assert(err == Tfa98xx_Error_Ok);
    }

	/*  powered on
	 *    - now it is allowed to access DSP specifics
	 *    - stall DSP by setting reset
	 * */
	err = Tfa98xx_DspReset(handle, 1);
	assert(err == Tfa98xx_Error_Ok);

	/*  wait until the DSP subsystem hardware is ready
	 *    note that the DSP CPU is not running yet (RST=1)
	 * */
    while ( ready == 0)
    {
        /* are we ready? */
    	err = Tfa98xx_DspSystemStable(handle, &ready);
        assert(err == Tfa98xx_Error_Ok);
    }

   /* Load cold-boot patch for the first time to force cold start-up.
    *  use the patchload only to write the internal register
    * */
    dspPatch(handle, LOCATION_FILES "coldboot.patch");

    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    assert(err == Tfa98xx_Error_Ok);

    assert(status & TFA98XX_STATUSREG_ACS_MSK);  /* ensure cold booted */

    /* cold boot, need to load all parameters and patches */
#ifdef 	PATCH_FILENAME
    /* patch the ROM code */
    dspPatch(handle, LOCATION_FILES PATCH_FILENAME);
#else
	printf("No patchfile.\n");
	/* in this case start the DSP */
	err = Tfa98xx_DspReset(handle, 0);
	assert(err == Tfa98xx_Error_Ok);
#endif
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
        Sleep(10);
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        assert(err == Tfa98xx_Error_Ok);
        
    } while ( (status & TFA98XX_STATUSREG_MTPB_MSK) == TFA98XX_STATUSREG_MTPB_MSK);
    assert( (status & TFA98XX_STATUSREG_MTPB_MSK) == 0);
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

int main(/* int argc, char* argv[] */)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle;
    int i;
    Tfa98xx_Mute_t mute;
    Tfa98xx_SpeakerParameters_t lsModel;
    Tfa98xx_StateInfo_t stateInfo;
    Tfa98xx_SpeakerParameters_t loadedSpeaker;
	unsigned short status;

    float re25;
     int calibrateDone = 0;

    /* use the generic slave address for optimizations */
	/* create handle */
	err = Tfa98xx_Open(I2C_ADDRESS, &handle);
	if ( err != Tfa98xx_Error_Ok)
	{
		printf("Unable to connect to the device. Check the I2C address and/or device is powered ON.\n");
	}
	assert(err == Tfa98xx_Error_Ok);
#if 0         // uncomment below code if scanning for I2C devices is required
    unsigned char slave_address;
   /* try all possible addresses, stop at the first found */
    for (slave_address = 0x68; slave_address <= 0x6E ; slave_address+=2)
    {
        printf("Trying slave address 0x%x\n", slave_address);
        err = Tfa98xx_Open(slave_address, &handle);
        if (err == Tfa98xx_Error_Ok)
        {
            break;
        }
    }
#endif
    /* should have found a device */
    assert(err == Tfa98xx_Error_Ok);

#ifdef SPDIF_AUDIO
    InitSpdifAudio();
#endif
#ifdef USB_AUDIO
    InitUsbAudio();
#endif

   /* cold boot, need to load all parameters and patches */
   
   coldStartup(handle);
   /*Set to calibration once*/
   /* Only needed for really first time calibration */
   setOtc(handle, 1);

   loadSpeakerFile(LOCATION_FILES SPEAKER_FILENAME, loadedSpeaker);
    
   /* Check if MTPEX bit is set for calibration once mode */
   if(checkMTPEX(handle) == 0)
   {
       printf("DSP not yet calibrated. Calibration will start.\n");
       
       /* ensure no audio during special calibration */
       err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
       assert(err == Tfa98xx_Error_Ok);
    }
    else
    {
        printf("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
    }

    /* Load all settings (for TFA9887: this is the 2nd time. Now speaker model contains tCoefA. */
    load_all_settings(handle, loadedSpeaker, LOCATION_FILES CONFIG_FILENAME, LOCATION_FILES PRESET_FILENAME, LOCATION_FILES EQ_FILENAME);

    /* do calibration (again), if needed */
    err = Tfa98xx_SetConfigured(handle);
    assert(err == Tfa98xx_Error_Ok);
    
   /* Wait until the calibration is done. 
   * The MTPEX bit would be set and remain as 1 if MTPOTC is set to 1 */
   waitCalibration(handle, &calibrateDone);
   if (calibrateDone)
   {
      Tfa98xx_DspGetCalibrationImpedance(handle,&re25);
   }
   else
   {
      re25 = 0;
   }
   printf("Calibration value is %2.2f ohm\n", re25);
   
   /*Checking the current status for DSP status and DCPVP */
   statusCheck(handle);

   /*Remark:*/
   /*Reset the MTPEX bit to do the calibration for new speaker*/ 
   /*resetMtpEx(handle);*/
   
   /*Set digital mute and unmute to test if mute state is correct*/
   err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
    assert(err == Tfa98xx_Error_Ok);
    err = Tfa98xx_GetMute(handle, &mute);
    assert(err == Tfa98xx_Error_Ok);

    assert(mute == Tfa98xx_Mute_Digital);

    /* Start playing music here */
    //DebugBreak();

    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);
    assert(err == Tfa98xx_Error_Ok);

   /*Starting to retrieve the live data info for 100 loop*/
    for (i=0; i<50; ++i)
    {
        err = Tfa98xx_DspGetStateInfo(handle, &stateInfo);
        assert(err == Tfa98xx_Error_Ok);
        dump_state_info(&stateInfo);
        Sleep(1000);
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
		switch(tfaRun_CheckEvents(status)) 
		{//idx=0
			case 1: //printf(">>>>>>>>>>repower CF\n");
				tfaRun_PowerCycleCF(handle);
			break;
			case 2:
				printf(">>>>>>>>>>full reset required!!\n");
			break;
			default:	//nop
			break;
		}

    }


    /* playing with the volume: first down, then up again */
    for (i=2; i<5; i++)
    {
        float vol;

        printf("Setting volume to %3.1f dB\n", -6.0f*i);
        err = Tfa98xx_SetVolume(handle, -6.0f*i);
        assert(err == Tfa98xx_Error_Ok);
        err = Tfa98xx_GetVolume(handle, &vol);
        assert(err == Tfa98xx_Error_Ok);
        assert( fabs(-6.0f*i - vol) < 0.001) ;
        err = Tfa98xx_DspGetStateInfo(handle, &stateInfo);
        assert(err == Tfa98xx_Error_Ok);
        dump_state_info(&stateInfo);
        Sleep(1000);
    }
    for (i=5; i>=0; i--)
    {
        printf("Setting volume to %3.1f dB\n", -6.0f*i);
        err = Tfa98xx_SetVolume(handle, -6.0f*i);
        assert(err == Tfa98xx_Error_Ok);
        dump_state_info(&stateInfo);
        Sleep(1000);
    }

    Sleep(5000);

    /* check LS model */
    err = Tfa98xx_DspReadSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), lsModel);
    assert(err == Tfa98xx_Error_Ok);
    if (0 != memcmp(loadedSpeaker, lsModel, TFA98XX_SPEAKERPARAMETER_LENGTH))
    {
        printf("Speaker model has been updated\n");
        saveSpeaker(lsModel, "saved.speaker");
    }

    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
    assert(err == Tfa98xx_Error_Ok);

	err = Tfa98xx_Powerdown(handle, 1);
	assert(err == Tfa98xx_Error_Ok);

	err = Tfa98xx_Close(handle);
	assert(err == Tfa98xx_Error_Ok);


	return 0;
}
