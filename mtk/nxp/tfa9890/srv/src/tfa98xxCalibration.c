/*
 * tfa98xxCalibration.c
 *
 *  Created on: Feb 5, 2013
 *      Author: wim
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "nxpTfa98xx.h"
#include "tfa98xxRuntime.h"
#include "tfa98xxCalibration.h"
#include "Tfa98xx_Registers.h"

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
 * return the tcoefA
 */
float tfa98xxGetTcoefA(Tfa98xx_handle_t *handlesIn)
{
	Tfa98xx_Error_t err;
	float re25, tCoefA, tCoef;
	int Tcal; /* temperature at which the calibration happened */
	int T0;
	int calibrateDone = 0;
	Tfa98xx_handle_t handle;


	if( handlesIn[0] == -1)
	{
		err = Tfa98xx_Open( tfa98xxI2cSlave*2, &handlesIn[0] );
		PrintAssert( err);
		if (err != Tfa98xx_Error_Ok)
		{
			return err;
		}
	}

	handle = handlesIn[0] ;

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
	printf("Calibration value is %2.2f ohm @ %d degrees\n", re25, Tcal);
	tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);
	/* calculate the tCoefA */
	T0 = 25; /* definition of temperature for Re0 */
	tCoefA = tCoef * re25 / (tCoef * (Tcal - T0)+1); /* TODO: need Rapp influence */
	printf(" Final tCoefA %1.5f\n", tCoefA);

	return tCoefA;
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

    /* make sure there is no valid calibration still present */
    err = Tfa98xx_DspGetCalibrationImpedance(handle, &re25);
    PrintAssert(err);
    assert(fabs(re25) < 0.1);
    printf(" re25 = %f\n", re25);

    /* use dummy tCoefA, also eases the calculations, because tCoefB=re25 */
    tfa98xxCaltCoefToSpeaker(loadedSpeaker, 0.0f);
    /* load all params from the local parameter buffer */
    tfa98xxLoadParams(handle);

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
    printf("Calibration value is %2.2f ohm @ %d degrees\n", re25, Tcal);

    /* calculate the tCoefA */
    T0 = 25; /* definition of temperature for Re0 */
    tCoefA = tCoef * re25 / (tCoef * (Tcal - T0)+1); /* TODO: need Rapp influence */
    printf(" Final tCoefA %1.5f\n", tCoefA);

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
/*
 * wait for calibrateDone
 */
#define TFA98XX_API_WAITRESULT_NTRIES TFA98XX_WAITRESULT_NTRIES_LONG // defined in API
Tfa98xx_Error_t tfa98xxCalWaitCalibration(Tfa98xx_handle_t handle, int *calibrateDone)
{
	Tfa98xx_Error_t err;
	int tries = 0;
	unsigned short mtp;

	err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);

	/* in case of calibrate once wait for MTPEX */
	if ( mtp & TFA98XX_MTP_MTPOTC) {
		while ( (*calibrateDone == 0) && (tries < TFA98XX_API_WAITRESULT_NTRIES))
		{	// TODO optimise with wait estimation
			err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
			*calibrateDone = ( mtp & TFA98XX_MTP_MTPEX);	/* check MTP bit1 (MTPEX) */
			tries++;
		}
	} else /* poll xmem for calibrate always */
	{
		while ((*calibrateDone == 0) && (tries<TFA98XX_API_WAITRESULT_NTRIES) )
		{	// TODO optimise with wait estimation
			err = Tfa98xx_DspReadMem(handle, 231, 1, calibrateDone);
			tries++;
		}
	}
	if(tries==TFA98XX_API_WAITRESULT_NTRIES) {
		printf("!!calibrateDone timedout!!\n");
		err = Tfa98xx_Error_StateTimedOut;
	}
	return err;
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
                  tfaRun_Sleepus(10000); // wait 1ms for mtp to complete
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
        tfaRun_Sleepus(140000); // wait 140000ms for mtp to complete
        // polling status for MTP busy clear
        for(;;){
            tfaRun_Sleepus(10000); // wait 10ms for mtp to complete
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
 */
Tfa98xx_Error_t tfa98xxCalibration(Tfa98xx_handle_t *handlesIn, int idx, int once )
{
   float tCoef, re25;
   Tfa98xx_Error_t err;
   int calibrateDone = 0;

   printf("calibrate %s\n", once ? "once" : "always" );

   err = tfaRun_ColdStartup(handlesIn[idx]);

   if (err)
	   return err;

   if ( once )
   {
	   tfa98xxCalSetCalibrateOnce(handlesIn[idx]);
   }
   else
   {
	   tfa98xxCalSetCalibrationAlways(handlesIn[idx]);
	   /* check Re,
	    *  in case the old value is still there reset it
	    */
		   err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], &re25);
		   PrintAssert(err);
		   if ( fabs(re25) > 0.1){
			   printf(" cleaning up old Re (=%.02f)\n", re25);
			   /* run startup again to clean up old calibration */
			   err = tfaRun_ColdStartup(handlesIn[idx]);
			   if (err)
				   return err;
		   }
   }
	if(tfa98xxCalCheckMTPEX(handlesIn[idx]) == 0)
   {
	  /* doublecheck by getting Re, it should return 0 if not calibrated*/
	  err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], &re25);
	  PrintAssert(err);
	  if ( fabs(re25) < 0.1)
			  printf("DSP not yet calibrated.\n");
	  else {
		  printf("Error: DSP already calibrated (R=%.02f Ohms).\n", re25);
		  return Tfa98xx_Error_Bad_Parameter;
	  }
      /* ensure no audio during special calibration */
      err = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Digital);
      assert(err == Tfa98xx_Error_Ok);
      if (!dspSupporttCoef(handlesIn[idx]))
      {
    	   printf(" 2 step calibration\n");
    	   tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);
    	   printf(" tCoef = %1.5f\n", tCoef);

		   err = calculateSpeakertCoefA(handlesIn[idx], tfaParams.speakerBuffer, tCoef);
		   assert(err == Tfa98xx_Error_Ok);

		   /* if we were in one-time calibration (OTC) mode, clear the calibration results
		   from MTP so next time 2nd calibartion step can start. */
		   resetMtpEx(handlesIn);

		   /* force recalibration now with correct tCoefA */
		   tfaRun_MuteAmplifier(handlesIn[idx]); /* clean shutdown to avoid plop */
		   tfaRun_ColdStartup(handlesIn[idx]);
      }
	}
	else
	{
		printf("DSP already calibrated.\n Calibration skipped, previous results loaded from MTP.\n");
	}

   tfa98xxLoadParams(handlesIn[idx]);    // write all settings

   /* do calibration (again), if needed */
   err = Tfa98xx_SetConfigured(handlesIn[idx]);
   if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
   err = tfa98xxCalWaitCalibration(handlesIn[idx], &calibrateDone);
   if(err) goto errorExit;

   if (calibrateDone)
   {
      err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx],&re25);
   }
   else
   {
      re25 = 0;
   }
   printf("Calibration value is \t\t\t\t\t%2.2f ohm\n", re25);

errorExit:
   /* Unmute after calibration */
   Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Off);

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
