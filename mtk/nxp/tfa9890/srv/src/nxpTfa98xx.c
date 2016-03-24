/*
 * tfa98xx.c
 *
 * this is the interface to the Tfa98xx API
 *
 *  Created on: Apr 5, 2012
 *      Author: wim
 */

#define LOG_TAG "tfa89xx"
#define LOG_NDEBUG 0

#include <stdio.h>
#include <assert.h>
#ifndef WIN32
#include <unistd.h>
#include <libgen.h>
#else
#include <Windows.h>
#endif
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "nxpTfa98xx.h"
#include "tfa98xxCalibration.h"
#include "tfa98xxRuntime.h"
#include <Tfa98xx_Registers.h>

/*
 * static profile definitions
 */
static int predefI2sRoute[]={TFA_DEFAULT_I2SINPUT, TFA_DEFAULT_I2SINPUT, TFA_DEFAULT_I2SINPUT};
static int predefSampleRate[]={TFA_PROFILE0_SAMPLERATE, TFA_PROFILE1_SAMPLERATE,TFA_PROFILE2_SAMPLERATE};

#define false 0
#define true  1
extern int cli_verbose; //TODO move to runtime
/*
 * module globals
 */

static   char *paramsName[]={  "patch",
        "speaker",
        "preset",
        "config",
        "equalizer",
		"drc"};
static int tfaHardwarePoweredDown = false; // default is self detect
nxpTfa98xxParameters_t tfaParams = {0};  // TODO do we allow tfaParams to be global (needed for speaker file update?
int nxpTfaCurrentProfile=0; // TODO deal with profiles
static int nxpTfaCurrentVolstep=0;

int tfa98xx_trace = 0;
int tfa98xx_verbose = 0;
int tfa98xx_quiet = 0;

#define TRACEIN  if(tfa98xx_trace) printf("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xx_trace) printf("Leave %s\n", __FUNCTION__);

#define MAXDEV ( (int) (sizeof(handles)/sizeof(Tfa98xx_handle_t)) )
static int idx = 0;         // TODO cleanup for single device

unsigned char  tfa98xxI2cSlave=TFA_I2CSLAVEBASE; // global for i2c access
#define I2C(idx) ((tfa98xxI2cSlave+idx)*2)
#define I2C_CHUNKSIZE (256) // max i2c xfer length
#define HAVE_ATOF 0 //1


#define BIQUAD_COEFF_SIZE       6
#define Tfa98xx_BIQUAD_LENGTH (1+(BIQUAD_COEFF_SIZE*3)) // TODO put this in API def?
static char latest_errorstr[64];

/*
 * clock control
 */
/*
 * I2S clock is present
 *
  *  * if power was off
 *  *   initialize
 *  *   configure
 *
 *  * check if we really have a stable clock
 *  * cancel amp_mute
 *  * clear powerdown state
 *
 *  return:
 *      nxpTfa98xx_Error_Ok
 *      nxpTfa98xx_Error_NoClock
 *      nxpTfa98xx_Error_LowAPI
 *
 */
nxpTfa98xx_Error_t nxpTfa98xxClockEnable(Tfa98xx_handle_t *handlesIn ) 
{
	unsigned short statusReg = 0;
	Tfa98xx_Error_t err87;
	nxpTfa98xx_Error_t err;
	int sws;

  unsigned int handleCount = Tfa98xx_MaxDevices();
  
  for (idx = 0; idx < handleCount; idx++)
  {
	  // todo: error handling. Error from first device will may get masked!!

	  TRACEIN

	  if( handlesIn[idx] == -1)
	  {
		  //err = nxpTfa98xxOpen(handlesIn[idx]);
		  err87 = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
		  if (err87) return err87;

		  // start the system algorithm
		  err87 = tfaRun_SpeakerBoost(handlesIn[idx], tfaHardwarePoweredDown);
		  if (err87) return err87;
		  // unmute
		  err87 = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Off);
		  if (err87) return err87;

		  tfaHardwarePoweredDown = false; // only once if told so
	  }
	  TRACEOUT
  }


	return Tfa98xx_Error_Ok;

}

/*
 * I2S clock will stop
 *
 * * set amp_mute (implicit soft mute)
 * * set powerdown
 *
  *  return:
 *      nxpTfa98xx_Error_Ok
 *      nxpTfa98xx_Error_LowAPI
 *
 */
nxpTfa98xx_Error_t nxpTfa98xxClockDisable(Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err87;
    nxpTfa98xx_Error_t err;
    unsigned short statusReg;
    unsigned int handleCount = sizeof(handlesIn)/sizeof(handlesIn[0]);

    for (idx = 0; idx < handleCount; idx++)
    {

    	// todo: error handling. Error from first device will may get masked!!

		TRACEIN
		if( handlesIn[idx] == -1)
		{
			err87 = Tfa98xx_Open(((tfa98xxI2cSlave+idx)*2), &handlesIn[idx] );
			if (err87 != Tfa98xx_Error_Ok)
			{
				return (nxpTfa98xx_Error_t) err87;
			}

		    err87 = Tfa98xx_ReadRegister16( handlesIn[idx] , 0, &statusReg);

			// check if amp is already off
			if ( TFA98XX_STATUSREG_SWS_MSK & statusReg == 0 ) {
		        TRACEOUT
				return nxpTfa98xx_Error_Ok;
			}
			err87 = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Amplifier);
			if (err87 != Tfa98xx_Error_Ok)
			{
			  return (nxpTfa98xx_Error_t) err87;
			}

			err87 = Tfa98xx_Powerdown(handlesIn[idx], true);
			if (err87 != Tfa98xx_Error_Ok)
			{
			  return (nxpTfa98xx_Error_t) err87;
			}
			// amp still on ?
			 err = tfaRun_StatusWait( handlesIn[idx], // wait for 0, 10 times
						  TFA98XX_STATUSREG_VDDS_MSK, 1, 10) ? nxpTfa98xx_Error_AmpOn : nxpTfa98xx_Error_Ok;

			// amp still on ?
			err = tfaRun_StatusWait( handlesIn[idx], // wait for 0, 10 times
						  TFA98XX_STATUSREG_SWS_MSK, 0, 10) ? nxpTfa98xx_Error_AmpOn : nxpTfa98xx_Error_Ok;

			TRACEOUT
		}
    }

    return err87 == Tfa98xx_Error_Ok ?
            err : nxpTfa98xx_Error_LowAPI;  //  lower level is worse
}
static int tfa98xxSelectStereo(Tfa98xx_handle_t *handlesIn )
{
   Tfa98xx_Error_t err87;
   err87 = Tfa98xx_SelectChannel(handlesIn[0], Tfa98xx_Channel_L);
	err87 = Tfa98xx_SelectChannel(handlesIn[1], Tfa98xx_Channel_R);

	/* ensure stereo routing is correct: in this example we use
	 * gain is on L channel from 1->2
	 * gain is on R channel from 2->1
	 * on the other channel of DATAO we put Isense
	 */

	err87 = Tfa98xx_SelectI2SOutputLeft(handlesIn[0], Tfa98xx_I2SOutputSel_DSP_Gain);
	err87 = Tfa98xx_SelectStereoGainChannel(handlesIn[1], Tfa98xx_StereoGainSel_Left);

	err87 = Tfa98xx_SelectI2SOutputRight(handlesIn[1], Tfa98xx_I2SOutputSel_DSP_Gain);
	err87 = Tfa98xx_SelectStereoGainChannel(handlesIn[0], Tfa98xx_StereoGainSel_Right);

	err87 = Tfa98xx_SelectI2SOutputRight(handlesIn[0], Tfa98xx_I2SOutputSel_CurrentSense);
	err87 = Tfa98xx_SelectI2SOutputLeft(handlesIn[1], Tfa98xx_I2SOutputSel_CurrentSense);

   return err87;
}


/*
 * do a cold start init on next clock enable
 *
 *
 */
nxpTfa98xx_Error_t nxpTfa98xxPowerdown(void) // TODO is nxpTfa98xxClockEnable with init param enough? of POR bit sufficient
{

    TRACEIN

    tfaHardwarePoweredDown = true;      // just set the flag

    TRACEOUT

    return nxpTfa98xx_Error_Ok;
}
const char* nxpTfa98xxGetErrorString(nxpTfa98xx_Error_t error) {
    const char* pErrStr;

    switch (error) {
    case nxpTfa98xx_Error_Ok:
        pErrStr = "Ok";
        break;
    case nxpTfa98xx_Error_Fail:
        pErrStr = "generic failure";
        break;
    case nxpTfa98xx_Error_NoClock:
        pErrStr = "No I2S Clock";
        break;
    case nxpTfa98xx_Error_LowAPI:
        pErrStr = "error returned from lower API level";
        break;
    case nxpTfa98xx_Error_BadParam:
        pErrStr = "wrong parameter";
        break;
    case nxpTfa98xx_Error_AmpOn:
        pErrStr = "amp is still running";
        break;
    case nxpTfa98xx_Error_DSP_not_running:
        pErrStr = "DSP is not running";
        break;

    default:
        sprintf(latest_errorstr, "Unspecified error (%d)", (int) error);
        pErrStr = latest_errorstr;
    }
    return pErrStr;
}


/*
 * basic customer/application specific settings
 *  called during init
 *  TODO is this correct & complete :customer/application specific settings
 */
static Tfa98xx_Error_t tfaInitSettings(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;

    TRACEIN

    if (tfa98xx_quiet == 0)
        printf("writing initialisation settings to device\n");

    err = Tfa98xx_Init(handle);
    PrintAssert(err);
    if (err) // likely to occur here if any error
        return err;

    err = Tfa98xx_SetSampleRate(handle,
    		tfaParams.profile[nxpTfaCurrentProfile].sampleRate);
    PrintAssert(err);
    err = Tfa98xx_SelectChannel(handle, idx==0 ? Tfa98xx_Channel_L : Tfa98xx_Channel_R); // TODO fix for stereo
    PrintAssert(err);
    err = Tfa98xx_SelectAmplifierInput(handle, Tfa98xx_AmpInputSel_DSP);
    PrintAssert(err);

    /* start at -12 dB */
    err = Tfa98xx_SetVolume(handle, -1.0);
    PrintAssert(err);

    err = Tfa98xx_Powerdown(handle, 0);
    PrintAssert(err);

    TRACEOUT

    return err;
}
/*
 * minimal settings for sound
 */
void tfaQuickInit(Tfa98xx_handle_t *handlesIn   )
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;

   TRACEIN

   if( handlesIn[idx] == -1 ) 
   {
      err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
      PrintAssert( err) ;
   }

   if ( Tfa98xx_Error_Ok != err) 
   {
      err = Tfa98xx_Init(handlesIn[idx]);
   }

   // basic settings for quickset
   Tfa98xx_WriteRegister16( handlesIn[idx], 4,0x880B ); //48 kHz I2S with coolflux in Bypass
   Tfa98xx_WriteRegister16( handlesIn[idx], 9,0x0219 ); //1.0 uF coil and other system settings
   Tfa98xx_WriteRegister16( handlesIn[idx], 9,0x0618 ); //power on
   
   TRACEOUT

}

/*
 * initialize the API
 */
int tfa98xxInit( Tfa98xx_handle_t handle )
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Other;

    TRACEIN

    if( handle == -1 ) 
    {
       err = Tfa98xx_Open(I2C(idx), &handle );
       PrintAssert( err );
       if (err) return err;
       
    }

    //TODO  tfaRun_Sleepus(10000);  // wait for 10ms

    err = tfaInitSettings(handle); // already PrintAsserted

    TRACEOUT

    return err;
}
/*
 *  int registers and coldboot dsp
 */
int tfa98xxReset( Tfa98xx_handle_t *handlesIn )
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    TRACEIN

   if( handlesIn[idx] == -1 ) 
   {
      err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
      PrintAssert( err) ;
      
   }

    if( handlesIn[idx] != -1 ) 
    {

    	err = tfaRun_Startup(handlesIn[idx]);
    	PrintAssert(err);
    	if (err)
    		return err;

    	/* force cold boot */
    	err = tfaRun_Coldboot(handlesIn[idx], 1); // set ACS
    	PrintAssert(err);

    }

    TRACEOUT

    return err;
}

/*
 * close all devices
 */
int tfa98xxClose( Tfa98xx_handle_t *handlesIn )
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;

   TRACEIN

   if( handlesIn[idx] == -1 ) 
   {
      err = Tfa98xx_Close(handlesIn[idx] );
      PrintAssert( err );
      if (err) return err;
   }

   TRACEOUT

   return err;
}

/*
 * parse the equalizer ascii input line into int
 */
static int tfa98xxGetInt(const char* s, int* r)
{
    int i;

    i = 0;
    while (s[i] == ' ')
    {
        i++;
    }
    *r = atoi(s + i);
    while ((s[i] != 0) && (s[i] != ' '))
    {
        i++;
    }
    return i;
}
/*
 * parse the equalizer ascii input line into float
 */
static int tfa98xxGetFloat(const char* s, float* f)
{
    int i;
    int  minus;
    float res;
    float d;
    char *end;

    res = 0;
    minus = false;
    i = 0;
    while (s[i] == ' ')
    {
        i++;
    }
#if HAVE_ATOF
    *f = strtof(s + i, &end);
    while ((s[i] != 0) && (s[i] != ' '))
    {
        i++;
    }
#else
    if (s[i] == '-')
    {
        minus = true;
        i++;
    }
    else if (s[i] == '+')
    {
        i++;
    }
    while ((s[i] >= '0') * (s[i] <= '9'))
    {
        res = res * 10 + s[i] - '0';
        i++;
    }
    if (s[i] == '.')
    {
        i++;
        d = 10;
        while ((s[i] >= '0') * (s[i] <= '9'))
        {
            res = res + ((float)(s[i] - '0')) / d;
            i++;
            d *= 10;
        }
    }
    if (minus)
    {
        res = -res;
    }
    *f = res;
#endif
    return i;
}

static Tfa98xx_Error_t tfa98xxSetEqualizer(Tfa98xx_handle_t handle, const char* eqData) {
    int i;
    int ind;
    float b0, b1, b2, a1, a2;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    if( handle == -1) {
        err = Tfa98xx_Open(I2C(idx), &handle );
        PrintAssert( err);
        if (err) return err;
    }

    i = 0;
    ind = 0;
    while (eqData[i] != 0) {
        /* read biquad index + 5 parameters */
        i += tfa98xxGetInt(eqData + i, &ind);
        i += tfa98xxGetFloat(eqData + i, &b0);
        i += tfa98xxGetFloat(eqData + i, &b1);
        i += tfa98xxGetFloat(eqData + i, &b2);
        i += tfa98xxGetFloat(eqData + i, &a1);
        i += tfa98xxGetFloat(eqData + i, &a2);
        /* skip to the next line */
        while ((eqData[i] != 0) && (eqData[i] <= ' ')) {
            i++;
        }

        if ((b0 != 1) || (b1 != 0) || (b2 != 0) || (a1 != 0) || (a2 != 0)) {
            err = Tfa98xx_DspBiquad_SetCoeff(handle, ind, b0, b1, b2, a1, a2);
            PrintAssert( err);
            if (err) return err;
        } else {
            err = Tfa98xx_DspBiquad_Disable(handle, ind);
            PrintAssert( err);
            if (err) return err;
        }

    }
    for (i = ind + 1; i <= 10; i++) {
        err = Tfa98xx_DspBiquad_Disable(handle, ind);
        PrintAssert( err);
        if (err) return err;
    }

    return err;
}

static Tfa98xx_Error_t tfa98xxSetEqualizerStereo(Tfa98xx_handle_t *handlesIn, const char* eqData)
{
    int i;
    int ind;
    float b0, b1, b2, a1, a2;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    if( handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PrintAssert( err);
        if (err) return err;
    }

    i = 0;
    ind = 0;
    while (eqData[i] != 0) {
        /* read biquad index + 5 parameters */
        i += tfa98xxGetInt(eqData + i, &ind);
        i += tfa98xxGetFloat(eqData + i, &b0);
        i += tfa98xxGetFloat(eqData + i, &b1);
        i += tfa98xxGetFloat(eqData + i, &b2);
        i += tfa98xxGetFloat(eqData + i, &a1);
        i += tfa98xxGetFloat(eqData + i, &a2);
        /* skip to the next line */
        while ((eqData[i] != 0) && (eqData[i] <= ' ')) {
            i++;
        }

        if ((b0 != 1) || (b1 != 0) || (b2 != 0) || (a1 != 0) || (a2 != 0)) {
            err = Tfa98xx_DspBiquad_SetCoeffMultiple(2, handlesIn, ind, b0, b1, b2, a1, a2);
            PrintAssert( err);
            if (err) return err;
        } else {
            err = Tfa98xx_DspBiquad_DisableMultiple(2, handlesIn, ind);
            PrintAssert( err);
            if (err) return err;
        }

    }
    for (i = ind + 1; i <= 10; i++) {
        err = Tfa98xx_DspBiquad_DisableMultiple(2, handlesIn, ind);
        PrintAssert( err);
        if (err) return err;
    }

    return err;
}


/*
 * load a volume step
 */
nxpTfa98xx_Error_t nxpTfa98xxSetVolume( Tfa98xx_handle_t *handlesIn, int profile, int step ) // apply volume step, or mute
{
    nxpTfa98xx_Error_t err = nxpTfa98xx_Error_Ok;
    Tfa98xx_Error_t err87;
    unsigned short statusReg, sysctrlReg;
    
    TRACEIN

    statusReg = tfa98xxReadRegister(0, handlesIn); // TODO what to check , will fail furtheron
    // check if this is mute
    if (TFA_ISMUTE(step)) {
        err87 = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Digital); // soft mute
        if (err87 != Tfa98xx_Error_Ok)
         {
            return err87;
         }
        if (tfa98xx_verbose)
            printf("SetMute\n");
    } else {
        if (tfa98xx_verbose)
            printf("Setting volume to %3.1f dB\n",tfaParams.profile[profile].vstep[step].attenuation );
        // load step
        if (step > tfaParams.profile[nxpTfaCurrentProfile].vsteps){
            return nxpTfa98xx_Error_BadParam;
        }
        //
        // only reload  if changed profile
        //
        if (nxpTfaCurrentProfile != profile) {
            err87 = Tfa98xx_SetSampleRate(handlesIn[idx],
                    tfaParams.profile[profile].sampleRate); //TODO  we need to mute here
            PrintAssert(err87);
           //  Amplifier mute = soft mute + disabling amp at zero crossing of the pilot.
            err87 = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Amplifier);
            PrintAssert(err87);
            // i2s input route
            sysctrlReg = tfa98xxReadRegister( 9, handlesIn); // bit13 = intf_sel 0=in1 1=in2
            switch (tfaParams.profile[profile].i2sRoute)
            {
            case 1:
                sysctrlReg &= ~(0x1<<13);
                break;
            case 2:
                sysctrlReg |= (0x1<<13);
                break;
            default:
                break;
            }
            tfa98xxWriteRegister( 9, sysctrlReg, handlesIn);
            // TODO check if need to Tfa98xx_SelectChannel depending on i2s signal
            //  wait 20ms (which is more than enough for all use cases), or poll engaged bit.
            tfaRun_Sleepus(20000);
            //
            err87 = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Off);
            PrintAssert(err87);
            
#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices
              // load the drc
              err = tfa98xxSetParams(tfa_drc_params,
                    tfaParams.profile[nxpTfaCurrentProfile].Drc,
                    tfaParams.profile[nxpTfaCurrentProfile].DrcLength, handlesIn[idx]);
#endif
         }
        // load the new preset
        err = tfa98xxSetParams(tfa_preset_params,
                tfaParams.profile[nxpTfaCurrentProfile].vstep[step].CF,
                TFA98XX_PRESET_LENGTH, handlesIn[idx]);
        if (tfa98xx_verbose)
            tfa98xxShowProfile(profile);
    }
    nxpTfa98xxSetVolumeDB(handlesIn, tfaParams.profile[profile].vstep[step].attenuation);
    err87 = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Off);

    PrintAssert(err87);

    nxpTfaCurrentProfile = profile;

    TRACEOUT

    return err;
}
/*
 * write all stored parameters to the DSP
 */
nxpTfa98xx_Error_t tfa98xxLoadParams(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err87;

    TRACEIN

    // patchloading moved to start

    if (tfaParams.configLength) {
        if (!tfa98xxSetParams(tfa_config_params, (void*) tfaParams.configBuffer,
                tfaParams.configLength, handle))
            return nxpTfa98xx_Error_BadParam;
    }
    if (tfaParams.speakerLength) {
        if (!tfa98xxSetParams(tfa_speaker_params, tfaParams.speakerBuffer,
                tfaParams.speakerLength, handle))
            return nxpTfa98xx_Error_BadParam;
    }
    if (tfaParams.profile[nxpTfaCurrentProfile].valid ) {
      if (!tfa98xxSetParams(tfa_preset_params, tfaParams.profile[nxpTfaCurrentProfile].vstep[0].CF ,
                TFA98XX_PRESET_LENGTH, handle))
      {   
            return nxpTfa98xx_Error_BadParam;
      } 
#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices
      // drc
      if ( tfaParams.profile[nxpTfaCurrentProfile].DrcLength )
       if (!tfa98xxSetParams(tfa_drc_params, tfaParams.profile[nxpTfaCurrentProfile].Drc ,
    		   tfaParams.profile[nxpTfaCurrentProfile].DrcLength, handle))
      {
            return nxpTfa98xx_Error_BadParam;
      }
#endif      
      // write eq if valid
      if (tfaParams.profile[nxpTfaCurrentProfile].EqValid) {
    	  if (!  tfa98xxSetParams(tfa_equalizer_params, (void*)tfaParams.profile[nxpTfaCurrentProfile].EQ ,
    	                  TFA98XX_EQ_LENGTH, handle) )
          {
                return nxpTfa98xx_Error_BadParam;
          }
      }
      err87 = Tfa98xx_SetVolume( handle, tfaParams.profile[nxpTfaCurrentProfile].vstep[0].attenuation );
    }

    TRACEOUT

    return nxpTfa98xx_Error_Ok;

}


/*
 *
 */
void tfa98xxShowProfile( int profile)
{
    printf("profile[%d]: vsteps=%d, i2sRoute=%d, sampleRate=%d\n",
        profile, tfaParams.profile[profile].vsteps,
        tfaParams.profile[profile].i2sRoute,
        tfaParams.profile[profile].sampleRate);
}

/*
 * check what kind of preset file version we have here
 *   if old/single
 *     load a single vstep @step0
 *       assume volume=0
 *       cache the preset data
 */
nxpTfa98xx_Error_t nxpTfa98xxStoreProfile(int profile, void *data, int length) {
    nxpTfa98xx_Error_t err = nxpTfa98xx_Error_Ok;
    char *ptr = data;
    int step,steps;
    nxpTfa98xxVolumeStepFile_t *stepfile;

    TRACEIN

    // cleanup first, not needed
    //bzero(&tfaParams.profile[profile], sizeof(nxpTfa98xxProfile_t)); //TODO if error current is lost

    if (0 == strncmp(TFA_VSTEP_ID, ptr, 6)) { // it's a new type
    // TODO crc checking ! corrupt file may crash now
        stepfile = (nxpTfa98xxVolumeStepFile_t *) data;
        steps = stepfile->size / sizeof(nxpTfa98xxVolumeStep_t);

        if (steps >= TFA_MAX_VSTEPS) {
            fprintf(stderr, "too many volumesteps:%d, clipped to %d\n", steps, TFA_MAX_VSTEPS-1);
            //  return nxpTfa98xx_Error_BadParam; //
            steps = TFA_MAX_VSTEPS-1;
        }
        tfaParams.profile[profile].vsteps = steps;
        // copy data
        for (step = 0; step < tfaParams.profile[profile].vsteps; step++) {
            // load it
            tfaParams.profile[profile].vstep[step].attenuation = stepfile->vstep[step].attenuation ;
//          ptr=&stepfile->vstep[step].attenuation;
//          printf(" att steps file=%02x %02x %02x %02x, %f\n" , ptr[3], ptr[2], ptr[1], ptr[0],
//                              tfaParams.profile[profile].vstep[step].attenuation );
            memcpy(tfaParams.profile[profile].vstep[step].CF, stepfile->vstep[step].CF, TFA98XX_PRESET_LENGTH);
        }

    } else { // old single file
        step = 0;   // only step 0
        tfaParams.profile[profile].vsteps = 1; // only one
        tfaParams.profile[profile].vstep[step].attenuation = 0; // default TODO default att. ok?
        // copy data
        if (TFA98XX_PRESET_LENGTH != length) // double check
            return nxpTfa98xx_Error_BadParam;
        memcpy(tfaParams.profile[profile].vstep[step].CF, data,
                TFA98XX_PRESET_LENGTH);
    }

    // set hardcoded values
//    tfaParams.profile[profile].i2sRoute = predefI2sRoute[profile]; // TODO define
//    tfaParams.profile[profile].sampleRate = predefSampleRate[profile]; // TODO define
    tfaParams.profile[profile].valid=1;

    if (tfa98xx_verbose)
        tfa98xxShowProfile(profile);

    TRACEOUT

    return err;
}

/*
 * cache the parameter data into local memory
 */
nxpTfa98xx_Error_t nxpTfa98xxStoreParameters(Tfa98xx_handle_t *handlesIn, nxpTfa98xxParamsType_t params, void *data, int length )
{
    nxpTfa98xx_Error_t err = nxpTfa98xx_Error_Ok;

    TRACEIN

    /*
     * call the specific setter functions
     *  rely on the API for error checking
    */
    switch ( params )
    {
      case tfa_patch_params:
         tfaParams.patchLength = length;
         if  ( length > (int)sizeof(tfaParams.patchBuffer) )
            return nxpTfa98xx_Error_BadParam;
         memcpy( (void*)  tfaParams.patchBuffer, data, length);
         break;
      case tfa_config_params:
         tfaParams.configLength = length;
         if  ( length > (int)sizeof(tfaParams.configBuffer) )
            return nxpTfa98xx_Error_BadParam;
         memcpy( (void*) tfaParams.configBuffer, data, length);
         break;
      case tfa_speaker_params:
         tfaParams.speakerLength = length;
         if  ( length > (int)sizeof(tfaParams.speakerBuffer) )
            return nxpTfa98xx_Error_BadParam;
         memcpy( tfaParams.speakerBuffer, data, length);
         break;
#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices
      case tfa_drc_params:
         if  ( length > TFA98XX_DRC_LENGTH )
            return nxpTfa98xx_Error_BadParam;
         tfaParams.profile[nxpTfaCurrentProfile].DrcLength = length;
         memcpy( tfaParams.profile[nxpTfaCurrentProfile].Drc , data, length);
         break;
#endif         
      case tfa_preset_params:
         err= nxpTfa98xxStoreProfile(nxpTfaCurrentProfile, data, length); // loads single or multi presets
         break;
      case tfa_equalizer_params:// store eq in profile
         if ( tfaParams.profile[nxpTfaCurrentProfile].valid ) {
        	 memcpy( (void*) tfaParams.profile[nxpTfaCurrentProfile].EQ, data, length);
        	 tfaParams.profile[nxpTfaCurrentProfile].EqValid = 1;
         }
         else {
        	 fprintf(stderr, "%s profile[%d] not yet valid\n",__FUNCTION__,nxpTfaCurrentProfile);
        	 return nxpTfa98xx_Error_BadParam;
         }
         break;
      default:
         fprintf(stderr, "%s Error: bad parameter:%d\n", __FUNCTION__, params) ;
         return nxpTfa98xx_Error_BadParam;
         break;
    }

    TRACEOUT

    return err;
}

/*
 * invalidate the parameter data from local memory
 */
void tfa98xxDeleteParams( nxpTfa98xxParamsType_t param )
{
    TRACEIN

    tfaParams.patchLength = 0;
    tfaParams.configLength = 0;
    tfaParams.speakerLength = 0;

    TRACEOUT


}
/*
 * return the config file length in bytes
 */
int tfa98xxConfigLength(Tfa98xx_handle_t handle) {
	   Tfa98xx_Error_t err;
	   int actualLength ,longConfig;

	   TRACEIN

#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices

 	  err = Tfa98xx_DspSupportFramework(handle, &longConfig);
	  PrintAssert(err);

	  actualLength = longConfig ? TFA98XX_CONFIG_LENGTH : 165; //shorf for old devices
#else
	  actualLength = TFA98XX_CONFIG_LENGTH;
#endif

	  TRACEOUT

	  return actualLength;
}
/*
 *
 */
int tfa98xxSetParams(   nxpTfa98xxParamsType_t params, void *data, int length, 
                        Tfa98xx_handle_t handle)
{
   Tfa98xx_Error_t err;
   int actualLength = 0, len, longConfig;

   TRACEIN

   if( handle == -1 ) 
   {  
      return Tfa98xx_Error_NotOpen;
   } 
   /*
    * call the specific setter functions
    *  rely on the API for error checking
   */
   switch ( params )
   {
      case tfa_patch_params:
         err = Tfa98xx_DspPatch(handle, length, (const unsigned char *) data);
         PrintAssert(err);
         actualLength = err ? 0 : length;
         break;
      case tfa_speaker_params:
         err = Tfa98xx_DspWriteSpeakerParameters(handle, TFA98XX_SPEAKERPARAMETER_LENGTH, (unsigned char *) data );
         PrintAssert(err);
         actualLength = err ? 0 : TFA98XX_SPEAKERPARAMETER_LENGTH;
         break;
      case tfa_config_params:
         err = Tfa98xx_DspWriteConfig(handle, tfa98xxConfigLength(handle), (const unsigned char *) data );
         PrintAssert(err);
         actualLength = err ? 0 :  tfa98xxConfigLength(handle);
         break;
      case tfa_preset_params:
         err = Tfa98xx_DspWritePreset(handle, TFA98XX_PRESET_LENGTH, ( unsigned char *) data );
         PrintAssert(err);
         actualLength = err ? 0 : TFA98XX_PRESET_LENGTH;
         break;
#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices
      case tfa_drc_params:
         err = Tfa98xx_DspWriteDrc(handle, TFA98XX_DRC_LENGTH, ( unsigned char *) data );
         PrintAssert(err);
         actualLength = err ? 0 : TFA98XX_DRC_LENGTH;
         break;
#endif         
      case tfa_equalizer_params:
         err = tfa98xxSetEqualizer(handle, (const char *) data);
         PrintAssert(err);
         actualLength = length;// TODO err ? 0 : Tfa98xx_BIQUAD_LENGTH;
         break;
      default:
         fprintf(stderr, "%s Error: bad parameter:%d\n", __FUNCTION__, params) ;
         actualLength = 0;
         break;
    }
    if ( (actualLength!=0) && (tfa98xx_quiet==0) )
      printf("written %s data to device\n", paramsName[params]);
    
    TRACEOUT

    return actualLength;
}

/*
 * volume control
 *
 *  note that the get only operates on the selected device
 */
int nxpTfa98xxGetVolume( Tfa98xx_handle_t *handlesIn, float *getVol )
{
   Tfa98xx_Error_t err;
   float vol = 0;

   TRACEIN

   if (handlesIn[idx] == -1) 
   {
      err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
      PrintAssert( err);
      if (err) 
      {
         return err;
      }
      
   }
   // call API
   err = Tfa98xx_GetVolume(handlesIn[idx], &vol);
   PrintAssert( err);
   if (err) 
   {
      return err;
   }
   vol = vol * 10;

   *getVol = vol;

   TRACEOUT
   
    return err;
}

int nxpTfa98xxSetVolumeDB( Tfa98xx_handle_t *handlesIn, float volume )
{
   Tfa98xx_Error_t err;

   TRACEIN

   if (handlesIn[idx] == -1) 
   {
      err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
      PrintAssert( err);
      if ( err ) return err;
      
   }
      
   err = Tfa98xx_SetVolume(handlesIn[idx], volume);
//      err = Tfa98xx_SetVolume(handle[idx], (float) vol / (float) 10);
   PrintAssert( err);
   
   TRACEOUT
   
   return err;
}

/*
 *
 */
void tfa98xxPowerUp(Tfa98xx_handle_t *handlesIn)
{
    Tfa98xx_Error_t err;

    TRACEIN

    if (handlesIn[idx] == -1) 
    {
       err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
       PrintAssert( err);
       if (err) return ;
       
    }
    // call API
    err = Tfa98xx_Powerdown(handlesIn[idx], 0);
    PrintAssert( err);
    
    TRACEOUT
}

/*
 *
 */
void tfa98xxPowerdown(Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err;

    TRACEIN

    if (handlesIn[idx] == -1) 
    {
       err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
       PrintAssert( err);
       if (err) return ;
       
    }
    // call API
    err = Tfa98xx_Powerdown(handlesIn[idx], 1);
    PrintAssert( err);
    
    TRACEOUT

}
/*
 * note that this only operates on the selected device
 */
int tfa98xxGetCalibrationImpedance( float* re0, Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err;

    TRACEIN

    if (handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
        PrintAssert( err);
        if ( err ) return err;
        
    }
    // call API
    err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], re0);
    PrintAssert( err);
    return err;

    TRACEOUT

    return err;
}
/*
 * new calibration funtions
 */
float tCoefFromSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes)
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

/*
 * return current tCoef , set new value if arg!=0
 */
float tfa98xxTcoef(float tCoef)
{
    float current;

    current = tCoefFromSpeaker(tfaParams.speakerBuffer);

    if ( tCoef )
        tCoefToSpeaker(tfaParams.speakerBuffer, tCoef);

    return current;
}

/*
 *  calculate a new tCoefA and put the result into the loaded Speaker params
 */
static Tfa98xx_Error_t calculateSpeakertCoefA(  Tfa98xx_handle_t handle,
                                                Tfa98xx_SpeakerParameters_t loadedSpeaker,
                                                float tCoef )
{
    Tfa98xx_Error_t err;
    float re25, tCoefA;
    int Tcal; /* temperature at which the calibration happened */
    int T0;
    int calibrateDone = 0;

    err = Tfa98xx_DspGetCalibrationImpedance(handle, &re25);
    PrintAssert(err);
    printf("re25 = %f\n", re25);
    
    /* use dummy tCoefA, also eases the calculations, because tCoefB=re25 */
    tCoefToSpeaker(loadedSpeaker, 0.0f);
 //   load_all_settings(handle, loadedSpeaker, LOCATION_FILES "RefSettings_v2.config", LOCATION_FILES "RefPreset_v2.Donau.preset");
    tfa98xxLoadParams(handle);
      printf("tCoef = %f\n", tCoef);
    /* start calibration and wait for result */
    err = Tfa98xx_SetConfigured(handle);
    if (err != Tfa98xx_Error_Ok)
    {
       return err;
    }

    tfa98xxCalWaitCalibration(handle, &calibrateDone);
    if (calibrateDone)
    {
      err = Tfa98xx_DspGetCalibrationImpedance(handle, &re25);
    }
    else
    {
       re25 = 0;
    }
    err = Tfa98xx_DspReadMem(handle, 232, 1, &Tcal);
    if (err != Tfa98xx_Error_Ok)
    {
       return err;
    }
    printf("Calibration value is %2.2f ohm @ %d degress\n", re25, Tcal);

    /* calculate the tCoefA */
    T0 = 25; /* definition of temperature for Re0 */
    tCoefA = tCoef * re25 / (tCoef * (Tcal - T0)+1); /* TODO: need Rapp influence */
    printf("Final tCoefA %1.5f\n", tCoefA);

    /* update the speaker model */
    tCoefToSpeaker(loadedSpeaker, tCoefA);

    /* !!! the host needs to save this loadedSpeaker as it is needed after the next cold boot !!! */

    return err;
}

static int checkMTPEX(Tfa98xx_handle_t handle)
{
	unsigned short mtp;
	Tfa98xx_Error_t err;	
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
   if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
  
   if ( mtp & (1<<1))	/* check MTP bit1 (MTPEX) */
		return 1;					/* MTPEX is 1, calibration is done */
	else 
		return 0;					/* MTPEX is 0, calibration is not done yet */
}

static Tfa98xx_Error_t muteAmplifier(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	unsigned short status;
   int tries = 0;

	/* signal the TFA98XX to mute plop free and turn off the amplifier */
	err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
	if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }

	/* now wait for the amplifier to turn off */
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
	while ( ((status & TFA98XX_STATUSREG_SWS) == TFA98XX_STATUSREG_SWS) && (tries < 1000))
	{
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
      tries++;
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
	}
   if (tries == 1000)
   {
      /*The amplifier is always switching*/
      return Tfa98xx_Error_Other;
   }

   return err;
}

static void ClearCalMtp(Tfa98xx_handle_t handle)
{
       Tfa98xx_Error_t err;
  unsigned short mtp, status;

       /* Read MTP Register  */
       err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
       if (err != Tfa98xx_Error_Ok)
      {
         return;
      }


       /* Clear if non-0.*/
       if ( (mtp & TFA98XX_MTP_MTPOTC) != 0)
       {
              err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A); /* unlock key2 */
              if (err != Tfa98xx_Error_Ok)
              {
                  return;
              }

              err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, 0); /* MTPOTC=0, MTPEX=0 */
              if (err != Tfa98xx_Error_Ok)
              {
                  return;
              }
              err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP_COPY, 1<<11); /* CIMTP=1 */
              if (err != Tfa98xx_Error_Ok)
              {
                  return;
              }
              // polling status for MTP busy clear
              for(;;){
                  tfaRun_Sleepus(10000); // wait 1ms for mtp to complete
                  err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
                  if ( status & (1<<8))
                      ;//printf("0=%0x\n",status);
                  else break;
              }
       }
}

Tfa98xx_Error_t resetMtpEx(Tfa98xx_handle_t *handlesIn)
{
	Tfa98xx_Error_t err;
   unsigned short mtp;
	unsigned short status;

   if( handlesIn[idx] == -1) 
   {
     err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
	  PrintAssert( err);
   }
   
	err = Tfa98xx_ReadRegister16(handlesIn[idx], TFA98XX_MTP, &mtp);
	if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
	/* all settings loaded, signal the DSP to start calibration, only needed once after cold boot */

	/* reset MTPEX bit if needed */
	if ( (mtp & TFA98XX_MTP_MTPOTC) && (mtp & TFA98XX_MTP_MTPEX)) 
	{
		err = Tfa98xx_WriteRegister16(handlesIn[idx], 0x0B, 0x5A); /* unlock key2 */
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }

		err = Tfa98xx_WriteRegister16(handlesIn[idx], TFA98XX_MTP, 1); /* MTPOTC=1, MTPEX=0 */
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
		err = Tfa98xx_WriteRegister16(handlesIn[idx], 0x62, 1<<11); /* CIMTP=1 */
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
	}
	do
	{
		tfaRun_Sleepus(10000);
		err = Tfa98xx_ReadRegister16(handlesIn[idx], TFA98XX_STATUSREG, &status);
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
		
	} while ( (status & TFA98XX_STATUSREG_MTPB) == TFA98XX_STATUSREG_MTPB);
	if (err != Tfa98xx_Error_Ok)
   {
         return err;
   }
   return err;
}

static Tfa98xx_Error_t resetMtpExLocal(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
   unsigned short mtp;
	unsigned short status;
  
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
	if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
	/* all settings loaded, signal the DSP to start calibration, only needed once after cold boot */

	/* reset MTPEX bit if needed */
	if ( (mtp & TFA98XX_MTP_MTPOTC) && (mtp & TFA98XX_MTP_MTPEX)) 
	{
		err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A); /* unlock key2 */
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }

		err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, 1); /* MTPOTC=1, MTPEX=0 */
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
		err = Tfa98xx_WriteRegister16(handle, 0x62, 1<<11); /* CIMTP=1 */
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
	}
	do
	{
		tfaRun_Sleepus(10000);
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
		if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
		
	} while ( (status & TFA98XX_STATUSREG_MTPB) == TFA98XX_STATUSREG_MTPB);
	if (err != Tfa98xx_Error_Ok)
   {
         return err;
   }
   return err;
}

static Tfa98xx_Error_t Set_CalibrateOnce(Tfa98xx_handle_t handle)
{
       Tfa98xx_Error_t err;
  unsigned short mtp, status;

       /* Read MTP Register  */
       err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
       if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }

       /* all settings loaded, signal the DSP to start calibration, only needed once after cold boot */

       /* Verify if device is already set to calibrate once.*/
       if ( (mtp & TFA98XX_MTP_MTPOTC) == 0)
       {
              err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A); /* unlock key2 */
              if (err != Tfa98xx_Error_Ok)
               {
                  return err;
               }

              err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, 1); /* MTPOTC=1, MTPEX=0 */
              if (err != Tfa98xx_Error_Ok)
               {
                  return err;
               }
              err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP_COPY, 1<<11); /* CIMTP=1 */
              if (err != Tfa98xx_Error_Ok)
               {
                  return err;
               }
              // polling status for MTP busy clear
              for(;;){
                  tfaRun_Sleepus(10000); // wait 1ms for mtp to complete
                  err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
                  if ( status & (1<<8))
                      ;//printf("0=%0x\n",status);
                  else break;
              }
       }
       return err;
}

static int dspSupporttCoef(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	int bSupporttCoef;

	err = Tfa98xx_DspSupporttCoef(handle, &bSupporttCoef);
	assert(err == Tfa98xx_Error_Ok);

	return bSupporttCoef;
}

static int firstTimeCalibration(Tfa98xx_handle_t *handlesIn, int once)
{
   float tCoef, re25;
   Tfa98xx_Error_t err;

   tfaRun_ColdStartup(handlesIn[idx]); 	 //TODO multiple calib?

 

   tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);

   printf("tCoef = %1.5f\n", tCoef);
	if(checkMTPEX(handlesIn[idx]) == 0)
   {

      printf("DSP not yet calibrated.\n");
      /* ensure no audio during special calibration */
      err = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Digital);
      assert(err == Tfa98xx_Error_Ok);
      if (!dspSupporttCoef(handlesIn[idx]))
      {
    	   printf(" 2 step calibration\n");
		   err = calculateSpeakertCoefA(handlesIn[idx], tfaParams.speakerBuffer, tCoef);
		   assert(err == Tfa98xx_Error_Ok);

		   /* if we were in one-time calibration (OTC) mode, clear the calibration results
		   from MTP so next time 2nd calibartion step can start. */
		   resetMtpEx(handlesIn);

		   /* force recalibration now with correct tCoefA */
		   muteAmplifier(handlesIn[idx]); /* clean shutdown to avoid plop */
		   tfaRun_ColdStartup(handlesIn[idx]);
      }
	}
	else
	{
		printf("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
	}

   if ( once )
   {
      Set_CalibrateOnce(handlesIn[idx]);
   } 
   else 
   {
	   tfa98xxCalSetCalibrationAlways(handlesIn[idx]);
   }

   tfa98xxLoadParams(handlesIn[idx]);    // write all settings

   /* do calibration (again), if needed */
   err = Tfa98xx_SetConfigured(handlesIn[idx]);
   if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
 //  waitCalibration(handlesIn[idx], &re25);
   err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx],&re25);
   printf("Calibration value is %2.2f ohm\n", re25);

   //err = Tfa98xx_DspReadSpeakerParameters(handlesIn[idx], TFA98XX_SPEAKERPARAMETER_LENGTH, tfaParams.speakerBuffer);
   //tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);

   //printf("tCoefA = %1.5f\n", tCoef);

   return err;
}

int nxpTfa98xxCalibration(Tfa98xx_handle_t *handlesIn, int once)
{
	Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

	if( handlesIn[0] == -1)
	{
		err = Tfa98xx_Open( tfa98xxI2cSlave*2, &handlesIn[0] );
		PrintAssert( err);
	}
	if (err == Tfa98xx_Error_Ok) {
		err = tfa98xxCalibration(handlesIn, 0, once);
		PrintAssert( err);
	}

	return err;
}

int nxpTfa98xxCalibrationStereo(Tfa98xx_handle_t *handlesIn, int once)
{
   float tCoef, re25;
   Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
   int i = 0;
   int calibrateDone = 0;

   printf("calibrate %s\n", once ? "once" : "always" );

   tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);

   for (i = 0; i < 2; i++)
   {
      if( handlesIn[i] == -1) 
      {
         err = Tfa98xx_Open(((tfa98xxI2cSlave+i)*2), &handlesIn[i] );
	      PrintAssert( err);
      }

      if ( err != Tfa98xx_Error_Ok )
      {
         return err;
      }
      
      //coldStartup(handlesIn[i]); 	 //TODO multiple calib?
      tfaRun_ColdStartup(handlesIn[i]);

      printf("tCoef = %1.5f\n", tCoef);
      if(checkMTPEX(handlesIn[i]) == 0)
      {

         printf("DSP not yet calibrated.\n");
         /* ensure no audio during special calibration */
         err = Tfa98xx_SetMute(handlesIn[i], Tfa98xx_Mute_Digital);
         assert(err == Tfa98xx_Error_Ok);
         if (!dspSupporttCoef(handlesIn[i]))
         {
            printf("still tCoef in speaker file, special calibration needed\n");

            err = calculateSpeakertCoefA(handlesIn[i], tfaParams.speakerBuffer, tCoef);
            if (err != Tfa98xx_Error_Ok)
            {
               return err;
            };

            resetMtpExLocal(handlesIn[i]);
            /* force recalibration now with correct tCoefA */
		      err = muteAmplifier(handlesIn[i]); /* clean shutdown to avoid plop */
            PrintAssert( err);
            tfaRun_ColdStartup(handlesIn[i]);
         }
      }

      if ( once )
      {
         Set_CalibrateOnce(handlesIn[i]);
      } 
      else 
      {
    	  tfa98xxCalSetCalibrationAlways(handlesIn[i]);
      }
   }
   tfa98xxSelectStereo(handlesIn);
   for (i = 0; i < 2; i++)
   {
      tfa98xxLoadParams(handlesIn[i]);    // write all settings

      /* do calibration (again), if needed */
      err = Tfa98xx_SetConfigured(handlesIn[i]);
      if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
      tfa98xxCalWaitCalibration(handlesIn[i], &calibrateDone);
      if (calibrateDone)
      {
         err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[i], &re25);
      }
      else
      {
         re25 = 0;
      }
      printf("Calibration value is %2.2f ohm\n", re25);

      err = Tfa98xx_DspReadSpeakerParameters(handlesIn[i], TFA98XX_SPEAKERPARAMETER_LENGTH, tfaParams.speakerBuffer);
      tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);

      printf("tCoefA = %1.5f\n", tCoef);
   }
   return err;
}

/*
 * select device for testing and individual addressing
 *
 * this is for read/write
 */
int nxpTfa98xxSetIdx( int idxIn )
{
    int err = 0;

    if ( idxIn > 2) {
        fprintf(stderr, "%s Error: index %i bigger then max devices %i\n", __FUNCTION__, idx, 2) ;
        err = 1;
        return err;
    }
    idx = idxIn;

    return err;
}

/*
 * Bypass DSP handling
 */
int nxpTfa98xxBypassDSP(Tfa98xx_handle_t *handlesIn)
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;
   unsigned short i2SRead = 0;

   TRACEIN

   // basic settings for quickset
   err = Tfa98xx_ReadRegister16( handlesIn[idx], 4, &i2SRead);

   i2SRead &= TFA_I2SCTRL_BYPASS_DSP; 

   err = Tfa98xx_WriteRegister16( handlesIn[idx], 4, i2SRead ); 

   TRACEOUT
   
   return err;
}

/*
 * Unbypassed the DSP
 */
int nxpTfa98xxUnBypassDSP(Tfa98xx_handle_t *handlesIn)
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;
   unsigned short i2SRead = 0;

   TRACEIN

   // basic settings for quickset
   err = Tfa98xx_ReadRegister16( handlesIn[idx], 4, &i2SRead);

   i2SRead |= TFA_I2SCTRL_UNBYPASS_DSP; 

   err = Tfa98xx_WriteRegister16( handlesIn[idx], 4, i2SRead ); 

   TRACEOUT
   
   return err;
}

/*
 * Set sample rate;
 * @param rate in Hz.  must be 32000, 44100 or 48000
 */
nxpTfa98xx_Error_t nxpTfa98xxSetSampleRate(Tfa98xx_handle_t *handlesIn, int rate)
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;

   TRACEIN
	if( handlesIn[0] == -1)
	{
		err = Tfa98xx_Open( tfa98xxI2cSlave*2, &handlesIn[0] );
		PrintAssert( err);
	}
   // basic settings for quickset
   err = Tfa98xx_SetSampleRate(handlesIn[idx], rate);

   TRACEOUT
   
   return (nxpTfa98xx_Error_t)err;
}

/*
 * Get sample rate;
 * @param rate in Hz.  must be 32000, 44100 or 48000
 */
nxpTfa98xx_Error_t nxpTfa98xxGetSampleRate(Tfa98xx_handle_t *handlesIn, int *pRate)
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;

   TRACEIN

   // basic settings for quickset
   err = Tfa98xx_GetSampleRate(handlesIn[idx], pRate);

   TRACEOUT
   
   return (nxpTfa98xx_Error_t)err;
}

/*
 * operates only on selected dev
 */
int tfa98xxGetState(float* resistance, float* temperature, Tfa98xx_handle_t *handlesIn )
{
    Tfa98xx_Error_t err;
    Tfa98xx_StateInfo_t info;

    TRACEIN

    if (handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
        PrintAssert( err);
        if (err) return err;
    }

    err = Tfa98xx_DspGetStateInfo(handlesIn[idx], &info);
    PrintAssert( err);

    if (err == Tfa98xx_Error_Ok) {
        *resistance = info.Re;
        *temperature = info.T;
    }
    TRACEOUT

    return err;
}

/*
 * read a register
 */
unsigned short tfa98xxReadRegister(unsigned char offset, Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err;
    unsigned short value;

    TRACEIN

    if( handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PrintAssert( err);
        if (err) return err;
        
    }

    err = Tfa98xx_ReadRegister16( handlesIn[idx] , offset, &value);
    PrintAssert( err);

    TRACEOUT

    return value;

}
/*
 * read a register
 */
unsigned int tfa98xxReadXmem(unsigned char offset, Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err;
    int value;

    TRACEIN

    if( handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PrintAssert( err);
        if (err) return err;

    }
    err = Tfa98xx_DspReadMem(handlesIn[idx] , offset, 1, &value);
    PrintAssert( err);

    TRACEOUT

    return value & 0xffffff;

}
/*
 * write a register
 */
void tfa98xxWriteRegister( unsigned char offset, unsigned short value, 
                           Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err;

    TRACEIN

    if( handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PrintAssert( err);
        if (err) return ;
        
    }

    err = Tfa98xx_WriteRegister16( handlesIn[idx] , offset, value);
    PrintAssert( err);

    TRACEOUT

}
/*
 * DSP mem access
 *
 *  the msb 0xM0000 , M is the DSP mem region
 */
unsigned int
tfa98xx_DspRead( unsigned int offset, Tfa98xx_handle_t *handlesIn)
{
	 Tfa98xx_handle_t handle; // TODO cleanup handle shuffle
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short cf_ctrl; /* the value to sent to the CF_CONTROLS register */
        unsigned char bytes[4];
        int burst_size;         /* number of words per burst size */
        int bytes_per_word = 3;
        int num_bytes;
        int idx=0; //TODO I2C
        int dmem = (offset>>16) & 0xf;
        unsigned int value;
//    	Tfa98xx_DMEM_PMEM=0,
//    	Tfa98xx_DMEM_XMEM=1,
//    	Tfa98xx_DMEM_YMEM=2,
//    	Tfa98xx_DMEM_IOMEM=3
        if( handlesIn[0] == -1) {
            error = Tfa98xx_Open(I2C(idx), &handlesIn[0] );
            assert( error==Tfa98xx_Error_Ok);
        }
        handle = handlesIn[0];
        /* first set DMEM and AIF, leaving other bits intact */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        cf_ctrl &= ~0x000E;     /* clear AIF & DMEM */
        cf_ctrl |= (dmem << 1);    /* set DMEM, leave AIF cleared for autoincrement */
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, offset & 0xffff);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        num_bytes =  bytes_per_word;

        error =
            Tfa98xx_ReadData(handle, TFA98XX_CF_MEM, 3, bytes);
        if (error != Tfa98xx_Error_Ok) {
        		printf("DSP mem read error\n");
                return -1;
        }
        Tfa98xx_ConvertBytes2Data(3, bytes, &value);

        return value;
}

int
tfa98xx_DspWrite( unsigned int address, int value,
		Tfa98xx_handle_t *handlesIn)
{
	 Tfa98xx_handle_t handle; // TODO cleanup handle shuffle
       Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short cf_ctrl; /* the value to sent to the CF_CONTROLS register */
        unsigned char bytes[3];
        int dmem = (address>>16) & 0xf;
        int idx=0; //TODO I2C

        if( handlesIn[0] == -1) {
            error = Tfa98xx_Open(I2C(idx), &handlesIn[0] );
            assert( error==Tfa98xx_Error_Ok);
        }
        handle = handlesIn[0];

        /* first set DMEM and AIF, leaving other bits intact */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        cf_ctrl &= ~0x000E;     /* clear AIF & DMEM */
        cf_ctrl |= (dmem << 1);    /* set DMEM, leave AIF cleared for autoincrement */
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, address & 0xffff);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        Tfa98xx_ConvertData2Bytes(1, &value, bytes);
        error = Tfa98xx_WriteData(handle, TFA98XX_CF_MEM, 3, bytes);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        return Tfa98xx_Error_Ok;
}
/*
 * get tag
 *
 */
static nxpTfa98xx_Error_t nxpTfa98xx_GetDspTag(Tfa98xx_handle_t *handlesIn, char *string, int *size)
{
    Tfa98xx_Error_t err87;
    nxpTfa98xx_Error_t err = nxpTfa98xx_Error_Ok;
    int i;
    char tag[TFA98XX_MAXTAG];

    *size = 0;

    // interface should already be opened
    err87 = Tfa98xx_DspGetParam(handlesIn[idx], 1 /*MODULE_SPEAKERBOOST*/ , 0xFF,
            TFA98XX_MAXTAG, tag);

    PrintAssert( err87);

    if (err87 == Tfa98xx_Error_Ok) {
        // the characters are in every 3rd byte
        for ( i=2 ; i<TFA98XX_MAXTAG ; i+=3) 
        {
                if ( isprint(tag[i]) ) {
                    *string++ =  tag[i];    // only printable chars
                    (*size)++;
                }
        }
         *string = '\0';
    }
    if (err87 == Tfa98xx_Error_DSP_not_running)
        return nxpTfa98xx_Error_DSP_not_running;
    else if (err87 == Tfa98xx_Error_Bad_Parameter)
        return nxpTfa98xx_Error_BadParam;

    return err;
}

/*
 * return version strings
 */
nxpTfa98xx_Error_t nxpTfa98xxVersions( Tfa98xx_handle_t *handlesIn, char *strings, int maxlength )
{
	nxpTfa98xx_Error_t err = nxpTfa98xx_Error_Ok;
    Tfa98xx_Error_t err87;
    char str[NXPTFA_MAXLINE],str1[NXPTFA_MAXLINE];
    int length=0, i;
    unsigned short reg;

    // API rev
    sprintf(str, "nxpTfa API rev: %d.%d\n", NXPTFA_APP_REV_MAJOR, NXPTFA_APP_REV_MINOR);
    length = strlen(str);

    if (  length > maxlength )
        return nxpTfa98xx_Error_BadParam; // max length too short

    strcpy(strings, str);
    strings += length;
    length  += length;

    // tfa9887 API rev
    sprintf(str, "Tfa98xx API rev: %d.%d\n",
    		TFA98XX_API_REV_MAJOR, TFA98XX_API_REV_MINOR);

    length = strlen(str);

    if (  length > maxlength )
        return nxpTfa98xx_Error_BadParam; // max length too short

    strcpy(strings, str);
    strings += length;
    length  += length;

	// chip rev
    reg = tfa98xxReadRegister(0x03, handlesIn); //TODO define a rev somewhere
    sprintf(str, "Tfa98xx HW  rev: 0x%04x\n", reg);

    length = strlen(str);

    if (  length > maxlength )
        return nxpTfa98xx_Error_BadParam; // max length too short

    strcpy(strings, str);
    strings += length;
    length  += length;

	//
    if (tfaHardwarePoweredDown) {
		err = tfaRun_Startup(handlesIn[idx]);
		PrintAssert(err);
		/* start */
		err = tfaRun_StartDSP(handlesIn[idx]);
		PrintAssert(err);
		// a wait is needed in this case becaus the DSP is starting
		tfaRun_Sleepus(50000);
    }

#if 1
    // coolflux ROM rev
    err = nxpTfa98xx_GetDspTag( handlesIn, str, &i);
    if ( err != nxpTfa98xx_Error_Ok)
        return err;
    length += i;

    if (  length > maxlength )
        return nxpTfa98xx_Error_BadParam; // max length too short
    sprintf(str1, "DSP revstring: \"%s\"\n", str);
    strcpy(strings, str1);
    strings += strlen(str1);
    length  += strlen(str1);
#endif
    *strings = '\0';

    return err;

}

/*
 * read DSP parameters
 */
nxpTfa98xx_Error_t tfa98xxDspGetParam( unsigned char module_id, unsigned char param_id, 
                                       int num_bytes, unsigned char data[],
                                       Tfa98xx_handle_t *handlesIn    )
{
    Tfa98xx_Error_t err87;
    nxpTfa98xx_Error_t err = nxpTfa98xx_Error_Ok;

    TRACEIN

    if( handlesIn[idx] == -1) {
        err87 = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PrintAssert( err87);
        if (err87) return err87;
        
    }

    err87 = Tfa98xx_DspGetParam(handlesIn[idx], module_id, param_id, num_bytes, data );
    PrintAssert( err87);

    if (err87 ==  Tfa98xx_Error_DSP_not_running)
    	err = nxpTfa98xx_Error_DSP_not_running;
    else if (err87 == Tfa98xx_Error_Bad_Parameter)
    	err = nxpTfa98xx_Error_BadParam;

    TRACEOUT

    return err;

}

/*
 * save params
 */
int tfa98xxSaveParamsFile(Tfa98xx_handle_t *handlesIn, char *filename)
{
      FILE *f;
      int c;
 
      f = fopen( filename, "wb");
      if (!f)
            {
                printf("Unable to open %s\n", filename);
                return 0;
            }
      c = fwrite( (void*)&tfaParams, sizeof(nxpTfa98xxParameters_t) , 1, f );
      fclose(f);

      return c;
}

/*
 * save dedicated params
 */
int tfa98xxSaveFile(Tfa98xx_handle_t *handlesIn, char *filename, nxpTfa98xxParamsType_t params)
{
   FILE *f;
 
   int c=0, i=0;
   unsigned char bytes[252] = {0};

   Tfa98xx_Error_t err;
 
   if( handlesIn[idx] == -1) 
   {
      err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
      PrintAssert( err);
      if (err) return err;
        
   }
      
   /*
    * call the specific setter functions
    *  rely on the API for error checking
   */
   switch ( params )
   {
      case tfa_patch_params:
         break;
      case tfa_speaker_params:
         err = Tfa98xx_DspReadSpeakerParameters(handlesIn[idx], TFA98XX_SPEAKERPARAMETER_LENGTH, tfaParams.speakerBuffer);
         if (err)
         {
            printf("DSP error\n");
            return err;
         }
         PrintAssert(err);
         f = fopen( filename, "wb");
         if (!f)
         {
             printf("Unable to open %s\n", filename);
             err = Tfa98xx_Error_Other;
             return err;
         }
         c = fwrite( (void*)&tfaParams.speakerBuffer, TFA98XX_SPEAKERPARAMETER_LENGTH , 1, f );
         fclose(f);
         break;
      case tfa_config_params:
         err = Tfa98xx_DspReadConfig(handlesIn[idx], tfa98xxConfigLength(handlesIn[idx]), tfaParams.configBuffer);
         if (err)
         {
            printf("DSP error\n");
            return err;
         }
         PrintAssert(err);
         f = fopen( filename, "wb");
         if (!f)
         {
            printf("Unable to open %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         c = fwrite( (void*)&tfaParams.configBuffer, tfa98xxConfigLength(handlesIn[idx]) , 1, f );
         fclose(f);
         break;
      case tfa_preset_params:
         err = Tfa98xx_DspReadPreset(  handlesIn[idx], 
        		 tfa98xxConfigLength(handlesIn[idx])+TFA98XX_PRESET_LENGTH,
                                       bytes );
         if (err)
         {
            printf("DSP error\n");
            return err;
         }
         for (i=0; i<TFA98XX_PRESET_LENGTH; i++)
         {
            tfaParams.profile[nxpTfaCurrentProfile].vstep[0].CF[i] = bytes[i+165];
         }
         PrintAssert(err);
         f = fopen( filename, "wb");
         if (!f)
         {
             printf("Unable to open %s\n", filename);
             err = Tfa98xx_Error_Other;
             return err;
         }
         c = fwrite( (void*)&tfaParams.profile[nxpTfaCurrentProfile].vstep[0].CF, TFA98XX_PRESET_LENGTH , 1, f );
         fclose(f);
         break;
      case tfa_equalizer_params:
         break;
      default:
         fprintf(stderr, "%s Error: bad parameter:%d\n", __FUNCTION__, params) ;
         break;
   }

   if (c != 1)
   {
      printf("Unable to handle the file %s\n", filename);
      err = Tfa98xx_Error_Other;
      return err;
   }

   return Tfa98xx_Error_Ok;
}

/*
 * Load dedicated params
 */
int tfa98xxLoadFile(Tfa98xx_handle_t *handlesIn, char *filename, nxpTfa98xxParamsType_t params)
{
   FILE *f;
   int c=0, i=0;
   unsigned char bytes[252] = {0};

   Tfa98xx_Error_t err;
 
   if( handlesIn[idx] == -1) 
   {
      err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
      PrintAssert( err);
      if (err) return err;
        
   }

   /*
    * call the specific setter functions
    *  rely on the API for error checking
   */
   switch ( params )
   {
      case tfa_patch_params:
         f = fopen( filename, "rb");
         if (!f)
         {
            printf("Unable to open %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         c = fread( (void*)&tfaParams.patchBuffer, 1, sizeof(tfaParams.patchBuffer), f );
         fclose(f);
         tfaParams.patchLength=c;
         if (tfaParams.patchLength > sizeof(tfaParams.patchBuffer))
         {
            printf("Patch file too big: %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         strncpy ( tfaParams.patchFile, filename, TFA_MAX_FILENAME);
         // TODO properly use tfa98xxSetParams() for this
         err = Tfa98xx_DspPatch(handlesIn[idx], tfaParams.patchLength,
        		 (const unsigned char *) tfaParams.patchBuffer);
         PrintAssert(err);
         if ( err==0 )
        	 printf("Loaded patch %s\n", filename);
         break;
      case tfa_speaker_params:
         f = fopen( filename, "rb");
         if (!f)
         {
            printf("Unable to open %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         c = fread( (void*)&tfaParams.speakerBuffer, 1, TFA98XX_SPEAKERPARAMETER_LENGTH, f );
         fclose(f);
         if (c != TFA98XX_SPEAKERPARAMETER_LENGTH)
         {
            printf("Wrong speaker size %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         err = Tfa98xx_DspWriteSpeakerParameters(  handlesIn[idx], 
                                                   TFA98XX_SPEAKERPARAMETER_LENGTH, 
                                                   tfaParams.speakerBuffer);
         if (err)
         {
            printf("DSP error\n");
            return err;
         }
         break;
      case tfa_config_params:
         f = fopen( filename, "rb");
         if (!f)
         {
            printf("Unable to open %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         c = fread( (void*)&tfaParams.configBuffer, 1, tfa98xxConfigLength(handlesIn[idx]), f );
         fclose(f);
         if (c > tfa98xxConfigLength(handlesIn[idx]))
         {
            printf("Wrong config size %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         err = Tfa98xx_DspWriteConfig( handlesIn[idx], 
        		 tfa98xxConfigLength(handlesIn[idx]),
                                       tfaParams.configBuffer);
         if (err)
         {
            printf("DSP error\n");
            return err;
         }
         break;
      case tfa_preset_params:
         f = fopen( filename, "rb");
         if (!f)
         {
            printf("Unable to open %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         c = fread(  (void*)&tfaParams.profile[nxpTfaCurrentProfile].vstep[0].CF, 
                     1, TFA98XX_PRESET_LENGTH, f );
         fclose(f);
         if (c != TFA98XX_PRESET_LENGTH)
         {
            printf("Wrong preset size %s\n", filename);
            err = Tfa98xx_Error_Other;
            return err;
         }
         err = Tfa98xx_DspWritePreset( handlesIn[idx], 
                                       TFA98XX_PRESET_LENGTH, 
                                       tfaParams.profile[nxpTfaCurrentProfile].vstep[0].CF);
         if (err)
         {
            printf("DSP error\n");
            return err;
         }
         break;
      case tfa_equalizer_params:
         break;
      default:
         fprintf(stderr, "%s Error: bad parameter:%d\n", __FUNCTION__, params) ;
         break;
   }

   return Tfa98xx_Error_Ok;
}
/*
 * store zero terminated filename to cached params
 */
int tfaRun_NameToParam( char *fullname, nxpTfa98xxParamsType_t type, nxpTfa98xxParameters_t *parms)
{
   

#ifndef WIN32
   char *name;

	name = basename(fullname);
#else
   char name[100];

	_splitpath(fullname, NULL, NULL, name, NULL);
#endif
	switch(type) {
	case tfa_patch_params:
		strncpy ( tfaParams.patchFile, name, TFA_MAX_FILENAME);
		break;
	case tfa_speaker_params:
		strncpy ( tfaParams.speakerFile, name, TFA_MAX_FILENAME);
		break;
	case tfa_preset_params:
		strncpy ( tfaParams.profile[nxpTfaCurrentProfile].filename , name, TFA_MAX_FILENAME);
		break;
	case tfa_config_params:
		strncpy ( tfaParams.configFile, name, TFA_MAX_FILENAME);
		break;
	case tfa_equalizer_params:
		strncpy ( tfaParams.profile[nxpTfaCurrentProfile].Eqfilename , name, TFA_MAX_FILENAME);
		break;
#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices
	case tfa_drc_params:
		strncpy ( tfaParams.profile[nxpTfaCurrentProfile].Drcfilename , name, TFA_MAX_FILENAME);
		break;
//	case tfa_mdrc_params:
#endif    

		break;
	default:
		return 1;
	}
	return 0;
}
/*
 * display parameters
 */
void tfaRun_ShowParameters( nxpTfa98xxParameters_t *parms)
{
	int  p;

	printf("parameters:\n");
	// show commons

	if( parms->patchLength ) {
		printf(" patch: %s (%d bytes)", parms->patchFile, parms->patchLength);
	} else
		printf(" no patch");
	printf("\n");
	if( parms->configLength ) {
		printf(" config: %s (%d bytes)", parms->configFile, parms->configLength);
	} else
		printf(" no config");
	printf("\n");
	if( parms->speakerLength ) {
		printf(" speaker: %s (%d bytes)", parms->speakerFile, parms->speakerLength);
	} else
		printf(" no speaker");
	printf("\n");

	// show profiles
	for (p=0;p<TFA_MAX_PROFILES;p++) {
		printf(" profile[%d]:", p);
		if ( parms->profile[p].valid ){
			printf(" %s", parms->profile[p].filename);
			printf(" vsteps:%d", parms->profile[p].vsteps);
			printf(", sampleRate:%d", parms->profile[p].sampleRate);
			printf(", i2sIn%d", parms->profile[p].i2sRoute);
			printf(", pllin: %s", parms->profile[p].ipll ? "WS" : "BCK");
			if ( parms->profile[p].EqValid )
				printf("\n             %s", parms->profile[p].Eqfilename);
#ifdef	 TFA98XX_DRC_LENGTH //this is known if the define is for all devices
			if ( parms->profile[p].DrcLength )
				printf("\n             %s", parms->profile[p].Drcfilename);
#endif
		} else
			printf(" not valid");
		printf("\n");
	}
}
/*
 * load params
 */
int tfa98xxLoadParamsFile( Tfa98xx_handle_t *handlesIn, char *filename )
{
   FILE *f;
   int c;

   f = fopen( filename, "rb");
   if (!f)
   {
      printf("Unable to open %s\n", filename);
      return 0;
   }
   c = fread( (void*)&tfaParams, sizeof(nxpTfa98xxParameters_t) , 1, f );
   fclose(f);

   return c;

}

#ifndef WIN32
#include <libgen.h>

/*
 * dump the params into files
 */
static int outFile(int length, void *buffer, char *basename, char *ext)
{char *base, *ptr, outfile[80];
FILE *out;


	strncpy(outfile, basename, sizeof(outfile));
	strcat(outfile, ext);
	out = fopen( outfile, "wb");
	if(out==0) {
		printf("%s: can't open:%s\n", __FUNCTION__, outfile);
		return 0;
	}
	if ( length ) {
			fwrite(buffer, length, 1, out);
			fclose(out);
			printf("created %s (%d bytes)\n", outfile,
					length);
	}	\
	return length;
}
void tfa98xxDumpParams(char *parmsFilename)
{
	char *filename[]={".patch", ".speaker", ".config", ".preset"};
	char *base, *ptr, outfile[80];
	FILE *out;
	int i, length;
	void *buf;

	//strncpy (base, parmsFilename, sizeof(base));
	base = basename(parmsFilename);

	ptr = strrchr(base, (int)'.');
	if (ptr)
		*ptr='\0';
	else
	{
		printf("%s: bad parm filename:%s\n", __FUNCTION__, parmsFilename);
		return;
	}

	i=0;
	strncpy(outfile, base, sizeof(outfile));
	outFile(tfaParams.patchLength, (void*) tfaParams.patchBuffer,
			base , filename[i++]);
	outFile(tfaParams.speakerLength, (void*) tfaParams.speakerBuffer,
			base , filename[i++]);
	outFile(tfaParams.configLength, (void*) tfaParams.configBuffer,
			base , filename[i++]);
	outFile(TFA98XX_PRESET_LENGTH, (void*) tfaParams.profile[nxpTfaCurrentProfile].vstep[0].CF,
			base , filename[i++]);


}
#endif
