#include <utils/Log.h>
#include "../inc/Tfa98xx.h"
#include "../inc/Tfa98xx_Registers.h"

#define LOG_TAG "Tfa9887specific"

Tfa98xx_Error_t Tfa9887_specific(Tfa98xx_handle_t handle)
{
        ALOGD("Tfa9887_specific");
        Tfa98xx_Error_t error;
        unsigned short value;

        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* reset all i2C registers to default */
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYSTEM_CONTROL, TFA98XX_SYSTEM_CONTROL_I2CR );
        if ( Tfa98xx_Error_Ok != error)
        {
            ALOGD("Tfa98xx_Error_Ok != error error = %d ",error);
            return error;
        }

        /* adjust the default control values for detecting CF subsys ready by
         * polling AMPS
        */
        value = TFA98XX_SYSTEM_CONTROL_POR &
    				~(TFA98XX_SYSTEM_CONTROL_AMPE | // clear AMPE to prevent starting the amp too early
    				  TFA98XX_SYSTEM_CONTROL_CFE |
                 TFA98XX_SYSTEM_CONTROL_DCA ); // clear CFE to prevent AMPS from clearing
           value |=(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS);
    	  error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYSTEM_CONTROL, value);

        /* some other registers must be set for optimal amplifier behaviour */
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x05, 0x13AB);
        }
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x06, 0x001F);
        }
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x08, 0x3C4E);
        }
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x0A, 0x3EC3);
        }
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x41, 0x0308);
        }
        if (Tfa98xx_Error_Ok == error) {
                error = Tfa98xx_WriteRegister16(handle, 0x49, 0x0E82);
        }
        ALOGD("Tfa9887_specific error = %d",error);

        return error;
}

