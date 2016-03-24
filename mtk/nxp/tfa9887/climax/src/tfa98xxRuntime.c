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
#include "../inc/nxpTfa98xx.h"
#include "../inc/tfa98xxRuntime.h"
#include "../../Tfa98xx/inc/Tfa98xx_Registers.h"


Tfa98xx_Error_t tfaRunMuteAmplifier(Tfa98xx_handle_t handle)
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
	while ( ((status & TFA98XX_STATUS_SWS) == TFA98XX_STATUS_SWS) && (tries < 1000))
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
