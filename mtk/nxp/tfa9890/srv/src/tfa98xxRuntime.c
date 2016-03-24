/*
 * tfa98xxRuntime.c
 *
 *  Created on: Feb 5, 2013
 *      Author: wim
 */

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "nxpTfa98xx.h"
#include "tfa98xxRuntime.h"
#include "Tfa98xx_Registers.h"

// retry values
#define AREFS_TRIES 100
#define CFSTABLE_TRIES 100

int tfa98xxRun_trace=1;
#define TRACEIN  if(tfa98xxRun_trace) printf("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xxRun_trace) printf("Leave %s\n", __FUNCTION__);

#ifndef WIN32
/************************
 * time measurement
 */
#include <sys/time.h>
#include <sys/resource.h>

typedef struct tag_time_measure
{
  struct timeval startTimeVal;
  struct timeval stopTimeVal;

  struct rusage startTimeUsage;
  struct rusage stopTimeUsage;
} time_measure;

time_measure * startTimeMeasuring()
{
  time_measure * tu = malloc(sizeof(time_measure));
  if(!tu)
    exit(1);

  getrusage(RUSAGE_SELF, &tu->startTimeUsage);
  gettimeofday(&tu->startTimeVal,0);

  return tu;
}

void stopTimeMeasuring(time_measure * tu)
{
  getrusage(RUSAGE_SELF, &tu->stopTimeUsage);
  gettimeofday(&tu->stopTimeVal,0);
}

void printMeasuredTime(time_measure * tu)
{
  struct timeval elapsedVal;
  struct timeval userVal;
  struct timeval systemVal;

  double elapsed_millis = 0.0f;
  double user_millis = 0.0f;
  double system_millis = 0.0f;

  timersub(&tu->stopTimeVal, &tu->startTimeVal, &elapsedVal);
  timersub(&tu->stopTimeUsage.ru_utime, &tu->startTimeUsage.ru_utime, &userVal);
  timersub(&tu->stopTimeUsage.ru_stime, &tu->startTimeUsage.ru_stime, &systemVal);

  elapsed_millis = elapsedVal.tv_sec * 1000 + (double) elapsedVal.tv_usec / 1000;
  user_millis = userVal.tv_sec * 1000 + (double) userVal.tv_usec / 1000;
  system_millis = systemVal.tv_sec * 1000 + (double) systemVal.tv_usec / 1000;

  printf("-execution times [ms]:");
  printf("Total: %f, User: %f,System: %f\n", elapsed_millis, user_millis, system_millis);
}
#endif
/*
 *
 */
// global param cache
extern nxpTfa98xxParameters_t tfaParams;
extern int nxpTfaCurrentProfile;
/*
 * verbose enable
 */
int gTfaRun_timingVerbose=0;
extern int cli_verbose;
/*
 * accounting globals
 */
int gTfaRun_useconds=0;
extern int gNXP_i2c_writes, gNXP_i2c_reads; // declared in NXP i2c hal
/*
 * sleep requested amount of micro seconds
 */
#ifdef tfaRun_Sleepus
void _tfaRun_Sleepus(int us)
#else
void tfaRun_Sleepus(int us)
#endif
{
#ifdef WIN32
	int rest;

	rest = us%100;
	if (rest)
		us += 100; // round up for windows TODO check usec in windows
	Sleep(us/100); // is usinterval
#else
    usleep(us); // is usinterval
#endif
    gTfaRun_useconds += us;
}
/*
 * trace wrapper for tfaRun_Sleepus
 */
void tfaRun_SleepusTrace(int us, char *file, int line){
	if (gTfaRun_timingVerbose)
		printf("sleep %d us @%s:%d\n", us, file, line);
	_tfaRun_Sleepus(us);
}
/*
 * start the maximus speakerboost algorithm
 *  this implies a full system startup when the system was not already started
 *
 */
Tfa98xx_Error_t tfaRun_SpeakerBoost(Tfa98xx_handle_t handle, int force)
{
	Tfa98xx_Error_t err;

	if ( force ) {
		err= tfaRun_ColdStartup(handle);
		if ( err ) return err;
		// DSP is running now
	}

	if ( tfaRun_IsCold(handle)) {
		int calibrateDone;

		printf("coldstart%s\n", force? " (forced)":"");

		err = tfaRun_Startup(handle);
		PrintAssert(err);
		if ( err ) return err;

		err = tfaRun_StartDSP(handle);
		PrintAssert(err);
		if ( err ) return err;
		// DSP is running now

		// write all settings
		tfa98xxLoadParams(handle);    // TODO add to runtime
		// tell DSP it's loaded
		err = Tfa98xx_SetConfigured(handle);
		PrintAssert(err);
		if ( err ) return err;

		// await calibration, this should return ok
		tfa98xxCalWaitCalibration(handle, &calibrateDone);
		if (!calibrateDone) {
			printf("Calibration not done!\n");
			return Tfa98xx_Error_StateTimedOut;
		}
	} else { // warm
		int tries, status;
		/* power on the sub system */
		err = Tfa98xx_Powerdown(handle, 0);
		PrintAssert(err);
		// wait until everything is stable, in case clock has been off
		if (cli_verbose)
			printf("Waiting for DSP system stable...\n");
		for ( tries=CFSTABLE_TRIES; tries > 0; tries-- ) {
			err = Tfa98xx_DspSystemStable(handle, &status);
			assert(err == Tfa98xx_Error_Ok);
			if ( status )
				break;
		}
		if (tries==0) {// timedout
			printf("DSP subsystem start timed out\n");
			return Tfa98xx_Error_StateTimedOut;
		}
	}

	return err;
}
/*
 * validate the parameter cache
 *  if not loaded fix defaults
 */
void tfaRun_ValidateParams(nxpTfa98xxParameters_t *tfaParams,
		int defaultprofile) {
	int prof;

	if (!tfaParams->profile[defaultprofile].valid) {
		tfaParams->profile[defaultprofile].EqValid = 0;
		tfaParams->profile[defaultprofile].sampleRate = TFA_DEFAULT_SAMPLERATE;
		tfaParams->profile[defaultprofile].i2sRoute = TFA_DEFAULT_I2SINPUT;
		tfaParams->profile[defaultprofile].ipll = TFA_DEFAULT_PLLINPUT;
		tfaParams->profile[defaultprofile].valid = 1;
	}
	for (prof = 0; prof < TFA_MAX_PROFILES; prof++) {
		// for valid profiles fix sample rate
		if (tfaParams->profile[prof].valid) {
			if (!tfaParams->profile[prof].sampleRate) {
				tfaParams->profile[defaultprofile].sampleRate = TFA_DEFAULT_SAMPLERATE;
			}
			if (!tfaParams->profile[prof].i2sRoute) {
				tfaParams->profile[prof].i2sRoute = TFA_DEFAULT_I2SINPUT;
			}
		}
	}
}
#define TFA98XX_XMEM_CALIBRATION_DONE 231 //0xe7
#define TFA98XX_XMEM_COUNT_BOOT		  161 //0xa1
/*
 * sleep requested amount of milliseconds TODO support milliseconds?
 */
//void tfaRun_Sleepms(int ms)
//{
//	tfaRun_Sleepus(1000 * ms);
//}
int tfaRun_IsCaldone(Tfa98xx_handle_t handle) {
	int calibrateDone;

	Tfa98xx_DspReadMem(handle, TFA98XX_XMEM_CALIBRATION_DONE, 1, &calibrateDone);

	return calibrateDone;
}
/*
 *
 */
Tfa98xx_Error_t tfaRun_ResetCountClear(Tfa98xx_handle_t handle) {

  return Tfa98xx_DspWriteMem(handle,TFA98XX_XMEM_COUNT_BOOT , 0);

}
/*
 *
 */
int tfaRun_ResetCount(Tfa98xx_handle_t handle) {
	int count;

	Tfa98xx_DspReadMem(handle, TFA98XX_XMEM_COUNT_BOOT, 1, &count);

  return count;

}
/*
 *
 */
int tfaRun_IsCold(Tfa98xx_handle_t handle)
{
	unsigned short status;

	/* check status ACS bit to set */
	status = tfa98xxReadRegister(TFA98XX_STATUSREG, &handle); // this will also call open
	return (status & TFA98XX_STATUSREG_ACS)!=0;
}
/*
 *
 */
int tfaRun_IsAmpRunning(Tfa98xx_handle_t handle)
{
	unsigned short status;
	Tfa98xx_Error_t err;

	/* check status SWS bit to set */
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	PrintAssert(err);

	return (status & TFA98XX_STATUSREG_SWS_MSK)!=0;
}
/*
 * wait for a status bit condition
 *  return 0 if set within maxTries
 *   	   else timedout
 */
int tfaRun_StatusWait(Tfa98xx_handle_t handle, unsigned short bitmask,
			int value, int maxTries)
{
	unsigned short status;
	int tries, set=0;
	Tfa98xx_Error_t err;

	for ( tries=1; tries < maxTries; tries++ ) {
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
		assert(err == Tfa98xx_Error_Ok);
		set = (status & bitmask) != 0; // bit set or clear?
		if ( set  == value ) {
			if (cli_verbose) printf("%s: %d tries\n",__FUNCTION__, tries);
			return 0;
		}
	}

	return -1;

}
Tfa98xx_Error_t tfaRun_Coldboot(Tfa98xx_handle_t handle, int state)
{
#define CF_CONTROL 0x8100
	Tfa98xx_Error_t err=Tfa98xx_Error_Ok;
	int tries = 10;
	unsigned short ratereg;

	/* repeat set ACS bit until set as requested */
	while ( state == !tfaRun_IsCold(handle) ) {
		/* set colstarted in CF_CONTROL to force ACS */
		err = tfa98xx_DspWrite ( (Tfa98xx_DMEM_IOMEM<<16)|CF_CONTROL , 1 , &handle);
		PrintAssert(err);

		if (tries-- == 0) {
			printf("coldboot (ACS) did not %s\n", state ? "set":"clear");
			return Tfa98xx_Error_Other;
		}
	}

	return err;
}
/*
 * select i2c and pll at the same time, this saves some i2c transactions
 */
Tfa98xx_Error_t tfaRun_SelectInputs( Tfa98xx_handle_t handle, int i2sIn, int ipll )
{
	Tfa98xx_Error_t error;
	unsigned short value;

	error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
	if (error) return error;

	// input selects 1 or 2, the registers takes 0 or 1
	//  so fix the actual value (illegal value always yied 0  TODO checking for bad param
	i2sIn = i2sIn == 2 ? 1 : 0;
	value &= ~TFA98XX_SYS_CTRL_ISEL;
	value |= (i2sIn<<TFA98XX_SYS_CTRL_ISEL_POS);
	value &= ~TFA98XX_SYS_CTRL_IPLL;
	value |= (ipll<<TFA98XX_SYS_CTRL_IPLL_POS);

	error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);

	return error;

}

/*
 * selection input 1 or 2
 *  1 = input 1 [default]
 *  2 = input 2
 */
Tfa98xx_Error_t tfaRun_SelectI2sInput( Tfa98xx_handle_t handle, int input )
{
	Tfa98xx_Error_t error;
	unsigned short value;

	error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
	if (error) return error;

	input-=1; // reg

	value &= ~TFA98XX_SYS_CTRL_ISEL;
	value |= (input<<TFA98XX_SYS_CTRL_ISEL_POS);
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);

	return error;

}
/*
 * selection input PLL for lock
 *  0 = BCK [default]
 *  1 = WS
 *  */
Tfa98xx_Error_t tfaRun_SelectPLLInput( Tfa98xx_handle_t handle, int input )
{
	Tfa98xx_Error_t error;
	unsigned short value;

	error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
	if (error) return error;

	input-=1; // reg

	value &= ~TFA98XX_SYS_CTRL_IPLL;
	value |= (input<<TFA98XX_SYS_CTRL_IPLL_POS);
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);

	return error;
}
/*
 * set CFE and AMPE
 */
Tfa98xx_Error_t tfaRun_EnableCF( Tfa98xx_handle_t handle, int state )
{
	Tfa98xx_Error_t error;
	unsigned short value;

	error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
	if (error) return error;

	value &= ~TFA98XX_SYS_CTRL_AMPE_MSK; //enable amp
	value |= (state<<TFA98XX_SYS_CTRL_AMPE_POS);
	value &= ~TFA98XX_SYS_CTRL_CFE_MSK;
	value |= (state<<TFA98XX_SYS_CTRL_CFE_POS);
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);

	return error;

}

/*
 * load the patch if any
 *   else tell no loaded
 */
Tfa98xx_Error_t tfaRun_LoadPatch(Tfa98xx_handle_t handle) {

	if (tfaParams.patchLength) {
		if (!tfa98xxSetParams(tfa_patch_params, (void*) tfaParams.patchBuffer,
				tfaParams.patchLength, handle))
			return Tfa98xx_Error_Bad_Parameter;
	} else
		printf("No patch loaded\n");

	return Tfa98xx_Error_Ok;
}
/*
 *  this will load the patch witch will implicitly start the DSP
 *   if no patch is available the DPS is started immediately
 */
Tfa98xx_Error_t tfaRun_StartDSP(Tfa98xx_handle_t handle) {
	Tfa98xx_Error_t err;

	err = tfaRun_LoadPatch(handle);
	PrintAssert(err);
	if (err)
		return err;

	if (!tfaParams.patchLength) // patchload not done
		err = Tfa98xx_DspReset(handle, 0); // clear reset, otherwise done during patch load
	return err;
}
/*
 * load audio input property settings
 *  sample rate, input select, PLL input select
 */
Tfa98xx_Error_t tfaRun_SetAudio(Tfa98xx_handle_t handle, int profile)
{
	Tfa98xx_Error_t err;
	int rate = tfaParams.profile[profile].sampleRate ?
			tfaParams.profile[profile].sampleRate : TFA_PROFILE0_SAMPLERATE;  //TODO fix everywhere

	err = Tfa98xx_SetSampleRate(handle, rate);
	if (err) return err;

	err = tfaRun_SelectInputs( handle,
					tfaParams.profile[profile].i2sRoute,
					tfaParams.profile[profile].ipll);
	return err;

}
/*
 * start the clocks and wait until the AMP is switching
 *  on return the DSP sub system will be ready for loading
 */
Tfa98xx_Error_t tfaRun_Startup(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	int tries, status;
	unsigned short statusreg;

	/* load the optimal TFA98XX in HW settings */
	err = Tfa98xx_Init(handle);
	PrintAssert(err);

	/* I2S settings to define the audio input properties
	 *  these must be set before the subsys is up */
	err = tfaRun_SetAudio(handle ,nxpTfaCurrentProfile);
	PrintAssert(err);

	/* power on the sub system */
	err = Tfa98xx_Powerdown(handle, 0);
	PrintAssert(err);

	/* AREFS indicates that the PLL will be enabled */
	if (cli_verbose)
		printf("Waiting for AREFS...\n");
	for ( tries=1; tries < AREFS_TRIES; tries++ ) {
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &statusreg);
		assert(err == Tfa98xx_Error_Ok);
		if ( statusreg & TFA98XX_STATUSREG_AREFS )
			break;
	}
	if (tries == AREFS_TRIES) {
		if (cli_verbose) printf("Timed out\n");
		return Tfa98xx_Error_StateTimedOut;
	}  else
		if (cli_verbose) printf(" OK (tries=%d)\n", tries);

	/*  powered on
	 *    - now it is allowed to access DSP specifics
	 *    - stall DSP by setting reset
	 * */
	err = Tfa98xx_DspReset(handle, 1);
	PrintAssert( err) ;

	/*  wait until the DSP subsystem hardware is ready
	 *    note that the DSP CPU is not running (RST=1) */
	if (cli_verbose)
		printf("Waiting for DSP system stable...\n");
	for ( tries=1; tries < CFSTABLE_TRIES; tries++ ) {
		err = Tfa98xx_DspSystemStable(handle, &status);
		assert(err == Tfa98xx_Error_Ok);
		if ( status )
			break;
	}
	if (tries == CFSTABLE_TRIES) {
		if (cli_verbose) printf("Timed out\n");
		return Tfa98xx_Error_StateTimedOut;
	}  else
		if (cli_verbose) printf(" OK (tries=%d)\n", tries);

	/* the CF subsystem is enabled */

	if (cli_verbose) printf("reset count:0x%x\n", tfaRun_ResetCount(handle));

	return err;
}
/*
 * run the startup/init sequence and set ACS bit
 */
Tfa98xx_Error_t tfaRun_ColdStartup(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;

	err = tfaRun_Startup(handle);
	PrintAssert(err);
	if (err)
		return err;

	/* force cold boot */
	err = tfaRun_Coldboot(handle, 1); // set ACS
	PrintAssert(err);
	if (err)
		return err;

	/* start */
	err = tfaRun_StartDSP(handle);
	PrintAssert(err);

	return err;
}
/*
 *
 */
Tfa98xx_Error_t tfaRun_MuteAmplifier(Tfa98xx_handle_t handle)
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
/*
 *
 */
// status register errors to check for not 1
#define TFA98XX_STATUSREG_ERROR1_SET_MSK (  \
		TFA98XX_STATUSREG_OCDS  )
// status register errors to check for not 0
#define TFA98XX_STATUSREG_ERROR1_CLR_MSK ( \
		TFA98XX_STATUSREG_UVDS  |  \
		TFA98XX_STATUSREG_OVDS  |  \
		TFA98XX_STATUSREG_OTDS    )
#define TFA98XX_STATUSREG_ERROR2_SET_MSK (  \
		TFA98XX_STATUSREG_ACS |   \
		TFA98XX_STATUSREG_WDS )

/*
 * return of 0 means ok, else the system needs to be configured
 */
int tfaRun_CheckAlgo(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	static int ampTempCheck=0;// correct value after startup
	int ampTemp; // newly read value

	if ( ampTempCheck==0) { // first time
		if (!tfaRun_IsCold(handle)) {
			err = Tfa98xx_DspReadMem(handle, 666, 1, &ampTempCheck); //get initial value
			if (err)
				return 2; //other error
		}
		else
			return 1; // need to be (re-)configured
	}
	err = Tfa98xx_DspReadMem(handle, 666, 1, &ampTemp);
	if (err)
		return 2; //other error

	if ( ampTemp != ampTempCheck) {
		printf("ampTemp mismatch!!!=0x%04x, expected=0x%04x\n", ampTemp, ampTempCheck);
		return 1; // !! error need to be (re-)configured
	}
	return 0; //ok
}

int tfaRun_CheckEvents(unsigned short regval) {
	int severity=0;

//TODO see if all alarms are similar
//	if ( (regval & TFA98XX_STATUSREG_ERROR1_SET_MSK) || (~regval & TFA98XX_STATUSREG_ERROR1_CLR_MSK) )
	if ( regval & TFA98XX_STATUSREG_OCDS) //
		severity = 1;
	// next will overwrite if set
	if ( regval & TFA98XX_STATUSREG_ERROR2_SET_MSK )
		severity = 2;
	// check error conditions

	return severity;
}

Tfa98xx_Error_t tfaRun_PowerCycleCF(Tfa98xx_handle_t handle){
	Tfa98xx_Error_t err;
	int tries;
	int status;

	TRACEIN

	err = Tfa98xx_Powerdown(handle, 1);
	PrintAssert(err);
	err = Tfa98xx_Powerdown(handle, 0);
	PrintAssert(err);

	return err;
}

