#include <Tfa98xx.h>
#include "Tfa98xx_Registers.h"
#if 1//lipeng debug
#include <utils/Log.h>
#define LOG_TAG "TFA9890"
#include <cutils/log.h>
#endif

Tfa98xx_Error_t Tfa9890_specific(Tfa98xx_handle_t handle)
{
	    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short value;

        ALOGD("[%s][%s] 2", __FILE__, __func__);
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;

        /* reset all i2C registers to default */
        error = Tfa98xx_WriteRegister16(handle, 0x09, 0x0002);

        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_ReadRegister16(handle, 0x09, &value);
        }
        if (Tfa98xx_Error_Ok == error) {
                /* DSP must be in control of the amplifier to avoid plops */
                value |= TFA98XX_SYS_CTRL_AMPE_POS;
                error = Tfa98xx_WriteRegister16(handle, 0x09, value);
        }

        /* some other registers must be set for optimal amplifier behaviour */

        if (Tfa98xx_Error_Ok == error) {
				error = Tfa98xx_WriteRegister16(handle, 0x06, 0x000F); // not for n1c
		}
        if (Tfa98xx_Error_Ok == error) {
				error = Tfa98xx_WriteRegister16(handle, 0x07, 0x8FFF); // not for n1c
		}
		if (Tfa98xx_Error_Ok == error) {
				error = Tfa98xx_WriteRegister16(handle, 0x08, 0x7832); // change for n1c
        }
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x09, 0x824D);
        }
        if (Tfa98xx_Error_Ok == error) {
        		error = Tfa98xx_WriteRegister16(handle, 0x0A, 0x38E5); // not for n1c
		}
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x0E, 0x0F01);// not for n1c
		}

        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x46, 0x4000);// not for n1c
        }
        if (Tfa98xx_Error_Ok == error) {
				error = Tfa98xx_WriteRegister16(handle, 0x47, 0x5901);
		}
        if (Tfa98xx_Error_Ok == error) {
				error = Tfa98xx_WriteRegister16(handle, 0x48, 0x06C0);  
		}
		if (Tfa98xx_Error_Ok == error) {
				error = Tfa98xx_WriteRegister16(handle, 0x49, 0xAD93);
        }

        return error;
}


/*
 * Tfa9890_DspSystemStable will compensate for the wrong behavior of CLKS
 * to determine if the DSP subsystem is ready for patch and config loading.
 *
 * A MTP calibration register is checked for non-zero.
 *
 * Note that this only works after i2c reset because this will clear the MTP contents.
 * When we are configured then the DSP communication will synchronize access.
 *
 */
Tfa98xx_Error_t Tfa9890_DspSystemStable(Tfa98xx_handle_t handle, int *ready)
{
	Tfa98xx_Error_t error;
	unsigned short status, mtp0, mtp1;
	int tries;

	/* check the contents of the STATUS register */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	if (error) goto errorExit;

	/* if AMPS is set then we were already configured and running
	 *   no need to check further
	 */
	*ready = (status & TFA98XX_STATUSREG_AMPS_MSK)
		           == (TFA98XX_STATUSREG_AMPS_MSK);
	if (*ready) // if  ready go back
		return error; // will be Tfa98xx_Error_Ok

	/* check AREFS and CLKS: not ready if either is clear */
	*ready = (status & (TFA98XX_STATUSREG_AREFS_MSK | TFA98XX_STATUSREG_CLKS_MSK))
		            == (TFA98XX_STATUSREG_AREFS_MSK | TFA98XX_STATUSREG_CLKS_MSK);
	if (! *ready) // if not ready go back
		return error; // will be Tfa98xx_Error_Ok

	/* check MTPB
	 *   mtpbusy will be active when the subsys copies MTP to I2C
	 *   2 times retry avoids catching this short mtpbusy active period
	 */
	for ( tries=2; tries > 0; tries-- ) {
		error = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
		if (error) goto errorExit;
		/* check the contents of the STATUS register */
		*ready = (status & TFA98XX_STATUSREG_MTPB_MSK) == 0;
		if (*ready) // if ready go on
			break;
	}
	if ( tries==0)
		return Tfa98xx_Error_Ok; /* *ready will be 0 if retries exausted*/

	/* check the contents of  MTP register for non-zero,
	 *  this indicates that the subsys is ready  */
	error = Tfa98xx_ReadRegister16(handle, 0x84, &mtp0);
	if (error) goto errorExit;

	*ready = (mtp0 != 0) ; // The MTP register written?

	return error;

	errorExit:
	*ready = 0;
	assert(error);	// an error here can be considered to be fatal
	return error;
}

/*
 * Tfa9890_DspReset will deal with clock gating control in order
 * to reset the DSP for warm state restart
 */
// the CurrentSense4 register is not in the datasheet, define local
#define TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF (1<<2)
#define TFA98XX_CURRENTSENSE4 0x49
Tfa98xx_Error_t Tfa9890_DspReset(Tfa98xx_handle_t handle, int state)
{
	Tfa98xx_Error_t error;
	unsigned short value49, value;

	/* for TFA9890 temporarily disable clock gating when dsp reset is used */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_CURRENTSENSE4, &value49);
	if (error) return error;

	if (Tfa98xx_Error_Ok == error) {
		/* clock gating off */
		value = value49 | TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF; // set the bit
		error = Tfa98xx_WriteRegister16(handle, TFA98XX_CURRENTSENSE4, value);
		if (error) return error;
	}

	error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &value);
	if (error) return error;

	/* set requested the DSP reset signal state */
	value = state ? (value |  TFA98XX_CF_CONTROLS_RST_MSK) :
			(value & ~TFA98XX_CF_CONTROLS_RST_MSK);

	error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
			value);

	/* clock gating restore */
	error = Tfa98xx_WriteRegister16(handle, 0x49, value49);/* clock gating restore */
	return error;
}
