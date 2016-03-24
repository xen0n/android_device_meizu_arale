/*
 * tfa98xxCalibration.c
 *
 *  Created on: Feb 5, 2013
 *      Author: wim
 */

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#define Sleep(ms) usleep(ms*1000)
#else
#include <Windows.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../inc/nxpTfa98xx.h"
#include "../inc/tfa98xxRuntime.h"
#include "../inc/tfa98xxCalibration.h"
#include "../../Tfa98xx/inc/Tfa98xx_Registers.h"

/* global parameter cache */
extern nxpTfa98xxParameters_t tfaParams;

static int dspSupporttCoef(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	int bSupporttCoef;

	err = Tfa98xx_DspSupporttCoef(handle, &bSupporttCoef);
	assert(err == Tfa98xx_Error_Ok);

	return bSupporttCoef;
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
    tfa98xxCaltCoefToSpeaker(loadedSpeaker, 0.0f);
 //   load_all_settings(handle, loadedSpeaker, LOCATION_FILES "RefSettings_v2.config", LOCATION_FILES "RefPreset_v2.Donau.preset");
    tfa98xxLoadParams(handle);
      printf("tCoef = %f\n", tCoef);
    /* start calibration and wait for result */
    err = Tfa98xx_SetConfigured(handle);
    if (err != Tfa98xx_Error_Ok)
    {
       return err;
    }

    waitCalibration(handle, &calibrateDone);
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
    tfa98xxCaltCoefToSpeaker(loadedSpeaker, tCoefA);

    /* !!! the host needs to save this loadedSpeaker as it is needed after the next cold boot !!! */

    return err;
}
float tfa98xxCaltCoefFromSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes)
{
	int iCoef;

	/* tCoef(A) is the last parameter of the speaker */
	iCoef = (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-3]<<16) + (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-2]<<8) + speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-1];

	return (float)iCoef/(1<<23);
}

void tfa98xxCaltCoefToSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes, float tCoef)
{
	int iCoef;

	iCoef =(int)(tCoef*(1<<23));

	speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-3] = (iCoef>>16)&0xFF;
	speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-2] = (iCoef>>8)&0xFF;
	speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-1] = (iCoef)&0xFF;
}
Tfa98xx_Error_t tfa98xxCalSetCalibrateOnce(Tfa98xx_handle_t handle)
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
                  usleep(10000); // wait 1ms for mtp to complete
                  err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
                  if ( status & (1<<8))
                      ;//printf("0=%0x\n",status);
                  else break;
              }
       }
       return err;
}
Tfa98xx_Error_t tfa98xxCalSetCalibrationAlways(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short mtp, status;

    err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A); /* unlock key2 */
    if (err != Tfa98xx_Error_Ok)
    {
       return err;
    }

    /* Read MTP Register  */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
    if (err != Tfa98xx_Error_Ok)
    {
       return err;
    }

    // if mtp=0 then it already set to calibration always, so skip
    if ( mtp!=0)
    {
        err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, 0); /* MTPOTC=0, MTPEX=0 */
        if (err != Tfa98xx_Error_Ok)
         {
            return err;
         }
        err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP_COPY, 1<<11); /* CIMTP=1 */
        if (err != Tfa98xx_Error_Ok)
         {
            return err;
         }
        // note writing MPT to 0 takes long
        usleep(140000); // wait 140000ms for mtp to complete
        // polling status for MTP busy clear
        for(;;){
            usleep(10000); // wait 10ms for mtp to complete
            err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
            if ( status & (1<<8))
                ;//printf("0=%0x\n",status);
            else break;
        }
   }
   return err;

}
/*
 *
 *
 */
static void coldboot_patch(Tfa98xx_handle_t handle){
const char coldboot[] = { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x08, 0x00, 0x70, 0x00, 0x07, 0x81, 0x00, 0x00, 0x00, 0x01 };

	tfa98xxSetParams(tfa_patch_params, (void*) coldboot, sizeof(coldboot), handle);
}
/*
 *
 */
Tfa98xx_Error_t tfa98xxCalibration(Tfa98xx_handle_t *handlesIn, int idx, int once )
{
   float tCoef, re25;
   Tfa98xx_Error_t err;
   int calibrateDone = 0;

   printf("calibrate %s\n", once ? "once" : "always" );

   tfa98xxCalColdStartup(handlesIn[idx]);
   if ( once )
   {
	   tfa98xxCalSetCalibrateOnce(handlesIn[idx]);
   }
   else
   {
	   tfa98xxCalSetCalibrationAlways(handlesIn[idx]);
   }
   tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);

   printf("tCoef = %1.5f\n", tCoef);
	if(tfa98xxCalCheckMTPEX(handlesIn[idx]) == 0)
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
		   tfaRunMuteAmplifier(handlesIn[idx]); /* clean shutdown to avoid plop */
		   tfa98xxCalColdStartup(handlesIn[idx]);
      }
	}
	else
	{
		printf("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
	}

   tfa98xxLoadParams(handlesIn[idx]);    // write all settings

   /* do calibration (again), if needed */
   err = Tfa98xx_SetConfigured(handlesIn[idx]);
   if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
   waitCalibration(handlesIn[idx], &calibrateDone);
   if (calibrateDone)
   {
      err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx],&re25);
   }
   else
   {
      re25 = 0;
   }
   printf("Calibration value is %2.2f ohm\n", re25);

   /* Unmute after calibration */
   err = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Off);
   //err = Tfa98xx_DspReadSpeakerParameters(handlesIn[idx], TFA98XX_SPEAKERPARAMETER_LENGTH, tfaParams.speakerBuffer);
   //tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);

   //printf("tCoefA = %1.5f\n", tCoef);

   return err;

}

void tfa98xxCalColdStartup(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	unsigned short status = 0;
   unsigned short value = 0;
   int tries = 0;
   int sampleRate = 0;

	/* load the optimal TFA98XX in HW settings */
	err = Tfa98xx_Init(handle);
	PrintAssert(err);

	sampleRate = TFA_PROFILE0_SAMPLERATE;

   err = Tfa98xx_SetSampleRate(handle, sampleRate);
	PrintAssert(err);

	err = Tfa98xx_Powerdown(handle, 0);
	PrintAssert(err);

   printf("Waiting for AMPS and CLKS to start up\n"); // TODO make a clock wait in runtime
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);

   while ( ( (status & TFA98XX_STATUS_CLKS) == 0) &&
            (tries < TFA98XX_API_WAITRESULT_NTRIES) )
	{
		/* not ok yet */
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
		assert(err == Tfa98xx_Error_Ok);
      tries++;
	}

   if (tries >= TFA98XX_API_WAITRESULT_NTRIES)
   {
      printf("CLKS start up timed-out\n");
   }
   tries = 0;

   while ( ( (status & TFA98XX_STATUS_AMPS) == 0) &&
            (tries < TFA98XX_API_WAITRESULT_NTRIES) )
	{
		/* not ok yet */
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
		assert(err == Tfa98xx_Error_Ok);
      tries++;
	}

   if (tries >= TFA98XX_API_WAITRESULT_NTRIES)
   {
      printf("AMPS start up timed-out\n");
   }
   tries = 0;

   usleep(10000);
   /* some other registers must be set for optimal amplifier behaviour */
   if (Tfa98xx_Error_Ok == err)
   {
      err = Tfa98xx_WriteRegister16(handle, 0x70, 0x01);
   }
   value |=(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS);
   err = Tfa98xx_ReadRegister16(handle, TFA98XX_SYSTEM_CONTROL, &value);
   assert(err == Tfa98xx_Error_Ok);

   value |= TFA98XX_SYSTEM_CONTROL_CFE;/*(1<<2)*/
   value |=(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS);

   err = Tfa98xx_WriteRegister16(handle, TFA98XX_SYSTEM_CONTROL, value);
   assert(err == Tfa98xx_Error_Ok);

   err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
   assert(err == Tfa98xx_Error_Ok);

   while ( ( (status & TFA98XX_STATUS_AMPS) == 1) &&
            (tries < TFA98XX_API_WAITRESULT_NTRIES) )
	{
		/* not ok yet */
	   err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
	   assert(err == Tfa98xx_Error_Ok);
      tries++;
	}
   tries = 0;

	usleep(10000);

   coldboot_patch(handle);

	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	PrintAssert(err);

	if ( !(status & TFA98XX_STATUS_ACS))
   {
      printf("Not Cold booted");
      return;  /* ensure cold booted */
   }

   value |= TFA98XX_SYSTEM_CONTROL_AMPE; /*(1<<3)*/
   value |=(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS);

   err = Tfa98xx_WriteRegister16(handle, TFA98XX_SYSTEM_CONTROL, value);

   /* Maybe need delay to check SWS status */

	/* cold boot, need to load all parameters and patches */
	/* patch the ROM code */
	if (tfaParams.patchLength) // only load if hwe have it
		tfa98xxSetParams(tfa_patch_params, (void*) tfaParams.patchBuffer,
            tfaParams.patchLength, handle);

}
/*
 *
 *
 */
int tfa98xxCalCheckMTPEX(Tfa98xx_handle_t handle)
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
