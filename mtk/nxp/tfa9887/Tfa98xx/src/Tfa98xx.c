#include <assert.h>
#include <string.h>
#include <math.h>
#include "Tfa98xx_internals.h"
#include "../inc/Tfa98xx_Registers.h"
#include "../../Tfa98xxhal/inc/NXP_I2C.h"
#include "../inc/initTfa9887.h"
#include <utils/Log.h>

#define LOG_TAG "Tfa98xx"

#define OPTIMIZED_RPC

/* 4 possible I2C addresses, but only 2 devices (for stereo mode)
 */
#define MAX_HANDLES 2
static Tfa98xx_handle_private_t handlesLocal[MAX_HANDLES];

int handle_is_open(Tfa98xx_handle_t h)
{
  int retval = 0;

  if ( (h>=0) && (h<MAX_HANDLES) )
  {
    retval = handlesLocal[h].in_use != 0;
  }
  return retval;
}

/* translate a I2C driver error into an error for Tfa9887 API */
static Tfa98xx_Error_t classify_i2c_error(NXP_I2C_Error_t i2c_error)
{
        switch (i2c_error) {
        case NXP_I2C_Ok:
                return Tfa98xx_Error_Ok;
        case NXP_I2C_NoAck:
        case NXP_I2C_ArbLost:
        case NXP_I2C_TimeOut:
                return Tfa98xx_Error_I2C_NonFatal;
        default:
                return Tfa98xx_Error_I2C_Fatal;
        }
}

/* the patch contains a header with the following
 * IC revision register: 1 byte, 0xFF means don't care
 * XMEM address to check: 2 bytes, big endian, 0xFFFF means don't care
 * XMEM value to expect: 3 bytes, big endian
 */
static Tfa98xx_Error_t
checkICROMversion(Tfa98xx_handle_t handle, const unsigned char patchheader[])
{
        ALOGD("checkICROMversion");
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short checkrev;
        unsigned short checkaddress;
        int checkvalue;
        int value = 0;
        unsigned short status;
        checkrev = patchheader[0];
        if ((checkrev != 0xFF) && (checkrev != handlesLocal[handle].rev)) {
                return Tfa98xx_Error_Not_Supported;
        }
        checkaddress = (patchheader[1] << 8) + patchheader[2];
        checkvalue =
            (patchheader[3] << 16) + (patchheader[4] << 8) + patchheader[5];
        if (checkaddress != 0xFFFF) {
                /* before reading XMEM, check if we can access the DSP */
                error = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
                if (error == Tfa98xx_Error_Ok) {
                        if ((status & 0x0043) != 0x0043)
                        {
                                /* one of Vddd, PLL and clocks not ok */
                                error = Tfa98xx_Error_DSP_not_running;
                        }
                }
                /* read some address register that allows checking the correct ROM version */
                if (error == Tfa98xx_Error_Ok) {
                        error =
                            Tfa98xx_DspReadMem(handle, checkaddress, 1, &value);
                }
                if (error == Tfa98xx_Error_Ok) {
                        if (value != checkvalue) {
                                error = Tfa98xx_Error_Not_Supported;
                        }
                }
        }
        return error;
}


Tfa98xx_Error_t
Tfa98xx_DspGetSwFeatureBits(Tfa98xx_handle_t handle, int features[2])
{
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
    unsigned char bytes[3 * 2];

    error =
        Tfa98xx_DspGetParam(handle, MODULE_FRAMEWORK, FW_PARAM_GET_FEATURE_BITS,
                            sizeof(bytes), bytes);
    if (error != Tfa98xx_Error_Ok) {
		/* new function, old ROM code may respond with Tfa98xx_Error_RpcParamId */
        return error;
    }
    Tfa98xx_ConvertBytes2Data(sizeof(bytes), bytes, features);

	return error;
}

Tfa98xx_Error_t
Tfa98xx_Open(unsigned char slave_address, Tfa98xx_handle_t * pHandle)
{
        ALOGD("Tfa98xx_Open");
        Tfa98xx_Error_t error = Tfa98xx_Error_OutOfHandles;
        unsigned short rev;
        unsigned short status = 0;
        int i;
        assert(pHandle != (Tfa98xx_handle_t *) 0);
        *pHandle = -1;
        /* find free handle */
        for (i = 0; i < MAX_HANDLES; ++i) {
                if (!handlesLocal[i].in_use)
                {
                        switch (slave_address)
                        {
                        //case TFA98XX_GENERIC_SLAVE_ADDRESS:
                        case 0x68:
                        case 0x6A:
                        case 0x6C:
                        case 0x6E:
                                handlesLocal[i].in_use = 1;
                                handlesLocal[i].slave_address = slave_address;
                                /*Read status*/
                                error = Tfa98xx_ReadRegister16(i, TFA98XX_STATUS,
                                                               &status);
                                /*Read revision number*/
                                error =
                                    Tfa98xx_ReadRegister16(i, TFA98XX_REVISION,
                                                           &rev);
                                ALOGD("handlesLocal[i].rev = %d",rev);
                                if (Tfa98xx_Error_Ok != error)
                                {
                                        handlesLocal[i].in_use = 0;
                                        ALOGD("handlesLocal[i].in_use = 0; error = %d",error);
                                        return error;
                                }
                                ALOGD("handlesLocal[i].rev = %d",rev);
                                handlesLocal[i].rev = rev;
                                *pHandle = i;
                                error = Tfa98xx_Error_Ok;
                                break;
                        default:
                                /* wrong slave address */
                                error = Tfa98xx_Error_Bad_Parameter;
                        }
                        break;  /* handle is assigned, exit the for-loop */
                }
        }
        ALOGD("Tfa98xx_Open error = %d",error);
        return error;
}

Tfa98xx_Error_t Tfa98xx_Close(Tfa98xx_handle_t handle)
{
        if (handle_is_open(handle)) {
                handlesLocal[handle].in_use = 0;
                return Tfa98xx_Error_Ok;
        } else {
                return Tfa98xx_Error_NotOpen;
        }
}

Tfa98xx_Error_t Tfa98xx_Init(Tfa98xx_handle_t handle) {
	Tfa98xx_Error_t error;
	ALOGD("Tfa98xx_Init handlesLocal[handle].rev = %d",handlesLocal[handle].rev);
	/* some other registers must be set for optimal amplifier behaviour
	 * This is implemented in a seperate .c file specific for the type number
	 */
	switch (handlesLocal[handle].rev) {
	case 0x12:
	         ALOGD("Tfa9887_specific");
		error = Tfa9887_specific(handle);
		break;
	default:
		/* unsupported case, possibly intermediate version */
	         ALOGD("Tfa98xx_Error_Not_Supported");
		error = Tfa98xx_Error_Not_Supported;
		assert(0);
	}
	ALOGD("Tfa98xx_Init error = %d",error);
  return error;
}


static Tfa98xx_Error_t
processPatchFile(Tfa98xx_handle_t handle, int length,
                 const unsigned char *bytes)
{
        unsigned short size;
        int index;
        unsigned char buffer[MAX_I2C_LENGTH];
        NXP_I2C_Error_t i2c_error = NXP_I2C_Ok;
#if 0 /* no check on length to allow also RAM image loading which has bigger 'patch' files */
        if (length > TFA98XX_MAXPATCH_LENGTH) {
                return Tfa98xx_Error_Bad_Parameter;
        }
#endif
        /* expect following format in patchBytes:
         * 2 bytes length of I2C transaction in little endian, then the bytes, excluding the slave address which is added from the handle
         * This repeats for the whole file
         */
        index = 0;
        while (index < length) {
                /* extract little endian length */
                size = bytes[index] + bytes[index + 1] * 256;
                index += 2;
                if ((index + size) > length) {
                        /* too big, outside the buffer, error in the input data */
                        return Tfa98xx_Error_Bad_Parameter;
                }
                buffer[0] = handlesLocal[handle].slave_address;
                if ((size + 1) > MAX_I2C_LENGTH) {
                        /* too big, must fit buffer */
                        return Tfa98xx_Error_Bad_Parameter;
                }
                memcpy(buffer + 1, bytes + index, size);
                i2c_error = NXP_I2C_Write(size + 1, buffer);
                if (i2c_error != NXP_I2C_Ok) {
                        break;
                }
                index += size;
        }
        return classify_i2c_error(i2c_error);
}

/*
 * write a 16 bit subaddress
 */
Tfa98xx_Error_t
Tfa98xx_WriteRegister16(Tfa98xx_handle_t handle,
                        unsigned char subaddress, unsigned short value)
{
        NXP_I2C_Error_t i2c_error;
        int bytes2write = 4;    /* slave address, subaddress and 2 bytes of the value */
        unsigned char write_data[4];
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        write_data[0] = handlesLocal[handle].slave_address;
        write_data[1] = subaddress;
        write_data[2] = (value >> 8) & 0xFF;
        write_data[3] = value & 0xFF;
        i2c_error = NXP_I2C_Write(bytes2write, write_data);
        //ALOGD("Tfa98xx_WriteRegister16 subaddress = 0x%x value = 0x%x",subaddress,value);
        return classify_i2c_error(i2c_error);
}

/*
 * Write all the bytes specified by num_bytes and data
 */
Tfa98xx_Error_t
Tfa98xx_WriteData(Tfa98xx_handle_t handle,
                  unsigned char subaddress, int num_bytes,
                  const unsigned char data[])
{
        NXP_I2C_Error_t i2c_error;
        int bytes2write = num_bytes + 2;        /* slave address, subaddress followed by the data */
        //ALOGD("Tfa98xx_WriteData subaddress = 0x%x num_bytes = %d",subaddress,num_bytes);
        unsigned char write_data[MAX_I2C_LENGTH + 2];
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        if (num_bytes > MAX_I2C_LENGTH)
                return Tfa98xx_Error_Bad_Parameter;
        write_data[0] = handlesLocal[handle].slave_address;
        write_data[1] = subaddress;
        memcpy(write_data + 2, data, num_bytes);
        i2c_error = NXP_I2C_Write(bytes2write, write_data);
        return classify_i2c_error(i2c_error);
}

Tfa98xx_Error_t
Tfa98xx_ReadRegister16(Tfa98xx_handle_t handle,
                       unsigned char subaddress, unsigned short *pValue)
{
        NXP_I2C_Error_t i2c_error;
        const int bytes2write = 2;      /* slave address and subaddress */
        const int bytes2read = 3;       /* slave address and 2 bytes that will contain the data of the register */
        unsigned char write_data[2];
        unsigned char read_buffer[3];
        assert(pValue != (unsigned short *)0);
        if (!handle_is_open(handle))
        {
             ALOGD("Tfa98xx_ReadRegister16 Tfa98xx_Error_NotOpen");
                return Tfa98xx_Error_NotOpen;
        }
        write_data[0] = handlesLocal[handle].slave_address;
        read_buffer[0] = handlesLocal[handle].slave_address | 0x1;
        write_data[1] = subaddress;
        read_buffer[1] = read_buffer[2] = 0;
        i2c_error =
            NXP_I2C_WriteRead(bytes2write, write_data, bytes2read, read_buffer);
        if (classify_i2c_error(i2c_error) != Tfa98xx_Error_Ok)
        {
                return classify_i2c_error(i2c_error);
        } else
        {
                *pValue = (read_buffer[1] << 8) + read_buffer[2];
                return Tfa98xx_Error_Ok;
        }
}

Tfa98xx_Error_t
Tfa98xx_ReadData(Tfa98xx_handle_t handle,
                 unsigned char subaddress, int num_bytes, unsigned char data[])
{

        NXP_I2C_Error_t i2c_error;
        const int bytes2write = 2;      /* slave address and subaddress */
        const int bytes2read = 1 + num_bytes;   /* slave address and bytes that will be read */
        unsigned char write_data[2];
        unsigned char read_buffer[MAX_I2C_LENGTH];
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        if (bytes2read > MAX_I2C_LENGTH)
                return Tfa98xx_Error_Bad_Parameter;
        write_data[0] = handlesLocal[handle].slave_address;
        read_buffer[0] = handlesLocal[handle].slave_address | 0x1;
        write_data[1] = subaddress;
        i2c_error =
            NXP_I2C_WriteRead(bytes2write, write_data, bytes2read, read_buffer);
        if (classify_i2c_error(i2c_error) != Tfa98xx_Error_Ok) {
                return classify_i2c_error(i2c_error);
        } else {
                memcpy(data, read_buffer + 1, num_bytes);
                return Tfa98xx_Error_Ok;
        }
}

Tfa98xx_Error_t Tfa98xx_Powerdown(Tfa98xx_handle_t handle, int powerdown)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the SystemControl register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYSTEM_CONTROL, &value);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        switch (powerdown) {
        case 1:
                value |= TFA98XX_SYSCTRL_POWERDOWN;
                break;
        case 0:
                value &= ~(TFA98XX_SYSCTRL_POWERDOWN);
                break;
        default:
                return Tfa98xx_Error_Bad_Parameter;
        }
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYSTEM_CONTROL, value);
        return error;
}

Tfa98xx_Error_t Tfa98xx_SetConfigured(Tfa98xx_handle_t handle)
{
        ALOGD("+Tfa98xx_SetConfigured");

        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the SystemControl register, modify the bit and write again */
       value |=(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS);
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYSTEM_CONTROL, &value);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        value |= TFA98XX_SYSCTRL_CONFIGURED;
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYSTEM_CONTROL, value);
        ALOGD("-Tfa98xx_SetConfigured");
        return error;
}

Tfa98xx_Error_t
Tfa98xx_SelectAmplifierInput(Tfa98xx_handle_t handle,
                             Tfa98xx_AmpInputSel_t input_sel)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the SystemControl register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_CONTROL, &value);
        if (error == Tfa98xx_Error_Ok) {
                // clear the 2 bits first
                value &= ~(0x3 << TFA98XX_I2SCTRL_INPUT_SEL_SHIFT);
                switch (input_sel) {
                case Tfa98xx_AmpInputSel_I2SLeft:
                        value |= (0x0 << TFA98XX_I2SCTRL_INPUT_SEL_SHIFT);
                        break;
                case Tfa98xx_AmpInputSel_I2SRight:
                        value |= (0x1 << TFA98XX_I2SCTRL_INPUT_SEL_SHIFT);
                        break;
                case Tfa98xx_AmpInputSel_DSP:
                        value |= (0x2 << TFA98XX_I2SCTRL_INPUT_SEL_SHIFT);
                        break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_I2S_CONTROL, value);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_SelectI2SOutputLeft(Tfa98xx_handle_t handle,
                            Tfa98xx_OutputSel_t output_sel)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the I2S SEL register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_SEL, &value);
        if (error == Tfa98xx_Error_Ok) {
                // clear the 3 bits first
                value &= ~(0x7 << TFA98XX_I2SSEL_I2SOUT_LEFT_SHIFT);
                switch (output_sel) {
                case Tfa98xx_I2SOutputSel_CurrentSense:
                case Tfa98xx_I2SOutputSel_DSP_Gain:
                case Tfa98xx_I2SOutputSel_DSP_AEC:
                case Tfa98xx_I2SOutputSel_Amp:
                case Tfa98xx_I2SOutputSel_DataI3R:
                case Tfa98xx_I2SOutputSel_DataI3L:
                case Tfa98xx_I2SOutputSel_DcdcFFwdCur:
                        /* enum definition matches the HW registers */
                        value |=
                            (output_sel << TFA98XX_I2SSEL_I2SOUT_LEFT_SHIFT);
                        break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
        if (error == Tfa98xx_Error_Ok) {
                error = Tfa98xx_WriteRegister16(handle, TFA98XX_I2S_SEL, value);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_SelectI2SOutputRight(Tfa98xx_handle_t handle,
                             Tfa98xx_OutputSel_t output_sel)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the I2S SEL register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_SEL, &value);
        if (error == Tfa98xx_Error_Ok) {
                // clear the 3 bits first
                value &= ~(0x7 << TFA98XX_I2SSEL_I2SOUT_RIGHT_SHIFT);
                switch (output_sel) {
                case Tfa98xx_I2SOutputSel_CurrentSense:
                case Tfa98xx_I2SOutputSel_DSP_Gain:
                case Tfa98xx_I2SOutputSel_DSP_AEC:
                case Tfa98xx_I2SOutputSel_Amp:
                case Tfa98xx_I2SOutputSel_DataI3R:
                case Tfa98xx_I2SOutputSel_DataI3L:
                case Tfa98xx_I2SOutputSel_DcdcFFwdCur:
                        /* enum definition matches the HW registers */
                        value |=
                            (output_sel << TFA98XX_I2SSEL_I2SOUT_RIGHT_SHIFT);
                        break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
        if (error == Tfa98xx_Error_Ok) {
                error = Tfa98xx_WriteRegister16(handle, TFA98XX_I2S_SEL, value);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_SelectStereoGainChannel(Tfa98xx_handle_t handle,
                                Tfa98xx_StereoGainSel_t gain_sel)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the I2S Control register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_CONTROL, &value);
        if (error == Tfa98xx_Error_Ok) {
                // clear the 1 bits first
                value &= ~(0x1 << TFA98XX_I2SCTRL_DATAI2_SHIFT);
                switch (gain_sel) {
                case Tfa98xx_StereoGainSel_Left:
                case Tfa98xx_StereoGainSel_Right:
                        /* enum definition matches the HW registers */
                        value |= (gain_sel << TFA98XX_I2SCTRL_DATAI2_SHIFT);
                        break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_I2S_CONTROL, value);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_SelectChannel(Tfa98xx_handle_t handle, Tfa98xx_Channel_t channel)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the SystemControl register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_CONTROL, &value);
        if (error == Tfa98xx_Error_Ok) {
                // clear the 2 bits first
                value &= ~(0x3 << TFA98XX_I2SCTRL_CHANSEL_SHIFT);
                switch (channel) {
                case Tfa98xx_Channel_L:
                        value |= (0x1 << TFA98XX_I2SCTRL_CHANSEL_SHIFT);
                        break;
                case Tfa98xx_Channel_R:
                        value |= (0x2 << TFA98XX_I2SCTRL_CHANSEL_SHIFT);
                        break;
                case Tfa98xx_Channel_L_R:
                        value |= (0x3 << TFA98XX_I2SCTRL_CHANSEL_SHIFT);
                        break;
                case Tfa98xx_Channel_Stereo:
                        /* real stereo on 1 DSP not yet supported */
                        error = Tfa98xx_Error_Not_Supported;
                        break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_I2S_CONTROL, value);
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_SetSampleRate(Tfa98xx_handle_t handle, int rate)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* read the SystemControl register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_CONTROL, &value);
        if (error == Tfa98xx_Error_Ok) {
                // clear the 4 bits first
                value &= (~(0xF << TFA98XX_I2SCTRL_RATE_SHIFT));
                switch (rate) {
                case 48000:
                        value |= TFA98XX_I2SCTRL_RATE_48000;
                        break;
                case 44100:
                        value |= TFA98XX_I2SCTRL_RATE_44100;
                        break;
                case 32000:
                        value |= TFA98XX_I2SCTRL_RATE_32000;
                        break;
                case 24000:
                        value |= TFA98XX_I2SCTRL_RATE_24000;
                        break;
                case 22050:
                        value |= TFA98XX_I2SCTRL_RATE_22050;
                        break;
                case 16000:
                        value |= TFA98XX_I2SCTRL_RATE_16000;
                        break;
                case 12000:
                        value |= TFA98XX_I2SCTRL_RATE_12000;
                        break;
                case 11025:
                        value |= TFA98XX_I2SCTRL_RATE_11025;
                        break;
                case 8000:
                        value |= TFA98XX_I2SCTRL_RATE_08000;
                        break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_I2S_CONTROL, value);
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_GetSampleRate(Tfa98xx_handle_t handle, int *pRate)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        assert(pRate != 0);
        /* read the SystemControl register, modify the bit and write again */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_CONTROL, &value);
        if (error == Tfa98xx_Error_Ok) {
                // clear the 4 bits first
                value = value & (0xF << TFA98XX_I2SCTRL_RATE_SHIFT);
                switch (value) {
                case TFA98XX_I2SCTRL_RATE_48000:
                        *pRate = 48000;
                        break;
                case TFA98XX_I2SCTRL_RATE_44100:
                        *pRate = 44100;
                        break;
                case TFA98XX_I2SCTRL_RATE_32000:
                        *pRate = 32000;
                        break;
                case TFA98XX_I2SCTRL_RATE_24000:
                        *pRate = 24000;
                        break;
                case TFA98XX_I2SCTRL_RATE_22050:
                        *pRate = 22050;
                        break;
                case TFA98XX_I2SCTRL_RATE_16000:
                        *pRate = 16000;
                        break;
                case TFA98XX_I2SCTRL_RATE_12000:
                        *pRate = 12000;
                        break;
                case TFA98XX_I2SCTRL_RATE_11025:
                        *pRate = 11025;
                        break;
                case TFA98XX_I2SCTRL_RATE_08000:
                        *pRate = 8000;
                        break;
                default:
                        /* cannot happen, only 9 cases possible and handled above */
                        assert(0);
                }
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspWriteSpeakerParameters(Tfa98xx_handle_t handle,
                                  int length,
                                  const unsigned char *pSpeakerBytes)
{
        Tfa98xx_Error_t error;
        if (pSpeakerBytes != 0) {
                /* by design: keep the data opaque and no interpreting/calculation */
                error =
                    Tfa98xx_DspSetParam(handle, MODULE_SPEAKERBOOST,
                                        SB_PARAM_SET_LSMODEL, length,
                                        pSpeakerBytes);
        } else {
                error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspWriteSpeakerParametersMultiple(int handle_cnt,
                                          Tfa98xx_handle_t
                                          handles[],
                                          int length,
                                          const unsigned char *pSpeakerBytes)
{
        Tfa98xx_Error_t error;
        if (pSpeakerBytes != 0) {
                /* by design: keep the data opaque and no interpreting/calculation */
                error =
                    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                                                MODULE_SPEAKERBOOST,
                                                SB_PARAM_SET_LSMODEL, length,
                                                pSpeakerBytes);
        } else {
                error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspReadSpeakerParameters(Tfa98xx_handle_t handle,
                                 int length, unsigned char *pSpeakerBytes)
{
        Tfa98xx_Error_t error;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        if (pSpeakerBytes != 0) {
                error =
                    Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST,
                                        SB_PARAM_GET_LSMODEL, length,
                                        pSpeakerBytes);
        } else {
                error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_DspSupporttCoef(Tfa98xx_handle_t handle, int *pbSupporttCoef)
{
   Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

   /* older ROM code, result depends on REV */
	switch(handlesLocal[handle].rev)
	{
      case 0x12:
		   *pbSupporttCoef = 0;  /* old N1D ROM code expects tCoefA */
          error = Tfa98xx_Error_Ok;
			 break;
      case 0x80:
		   /* feature bits available in MTP directly */
			*pbSupporttCoef = 1; /* bit=0 means tCoef expected */
         error = Tfa98xx_Error_Ok;
			break;
		default:
		   /* unsupported case, possibly intermediate version */
			error = Tfa98xx_Error_Not_Supported;
			assert(0);
   }

   return error;
}
Tfa98xx_Error_t Tfa98xx_DspSupportDBDrc(Tfa98xx_handle_t handle, int *pbSupportDBDrc)
{
	    int featureBits[2];
	    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

	    assert(pbSupportDBDrc != 0);

	    error = Tfa98xx_DspGetSwFeatureBits(handle, featureBits);
	    if (error == Tfa98xx_Error_Ok) {
		   /* easy case: new API available */
		   *pbSupportDBDrc = (featureBits[0] & FEATURE1_DBDRC) == 0; /* bit=0 means DBDRC enabled */
		}
		else if (error == Tfa98xx_Error_RpcParamId) {
		    /* older ROM code, doesn't support it */
	        *pbSupportDBDrc = 0;  /* old ROM code and ICs don't support it */
			error = Tfa98xx_Error_Ok;
	    }
		// else some other error, return transparantly

		// pbSupportDBDrc only changed when error == Tfa98xx_Error_Ok
	    return error;
}

Tfa98xx_Error_t
Tfa98xx_DspWriteConfig(Tfa98xx_handle_t handle, int length,
                       const unsigned char *pConfigBytes)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        error =
            Tfa98xx_DspSetParam(handle, MODULE_SPEAKERBOOST, SB_PARAM_SET_CONFIG,
                                length, pConfigBytes);
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspWriteConfigMultiple(int handle_cnt,
                               Tfa98xx_handle_t handles[],
                               int length, const unsigned char *pConfigBytes)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        error =
            Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                                        MODULE_SPEAKERBOOST, SB_PARAM_SET_CONFIG,
                                        length, pConfigBytes);
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspWritePreset(Tfa98xx_handle_t handle, int length,
                       const unsigned char *pPresetBytes)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        if (pPresetBytes != 0) {
                /* by design: keep the data opaque and no interpreting/calculation */
                error =
                    Tfa98xx_DspSetParam(handle, MODULE_SPEAKERBOOST,
                                        SB_PARAM_SET_PRESET, length, pPresetBytes);
        } else {
                error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_DspReadConfig(Tfa98xx_handle_t handle, int length, Tfa98xx_Config_t pParameters)
{
   Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
   if (pParameters != 0)
   {
      error = Tfa98xx_DspGetParam(  handle, MODULE_SPEAKERBOOST,
                                    SB_PARAM_GET_CONFIG_PRESET, length, pParameters);
   }
   else
   {
      error = Tfa98xx_Error_Bad_Parameter;
   }
   return error;
}

Tfa98xx_Error_t Tfa98xx_DspReadPreset(Tfa98xx_handle_t handle, int length, Tfa98xx_Preset_t pParameters)
{
   Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

	if (pParameters != 0)
   {
      error = Tfa98xx_DspGetParam(  handle, MODULE_SPEAKERBOOST,
                                    SB_PARAM_GET_CONFIG_PRESET, length, pParameters);
   }
   else
   {
      error = Tfa98xx_Error_Bad_Parameter;
   }

   return error;
}
Tfa98xx_Error_t
Tfa98xx_DspWritePresetMultiple(int handle_cnt,
                               Tfa98xx_handle_t handles[],
                               int length, const unsigned char *pPresetBytes)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        if (pPresetBytes != 0) {
                /* by design: keep the data opaque and no interpreting/calculation */
                error =
                    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                                                MODULE_SPEAKERBOOST,
                                                SB_PARAM_SET_PRESET, length,
                                                pPresetBytes);
        } else {
                error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_SetVolume(Tfa98xx_handle_t handle, float voldB)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        if (voldB <= 0.0f) {
                /* 0x00 -> 0.0 dB
                 * 0x01 -> -0.5 dB
                 * ...
                 * 0xFE -> -127dB
                 * 0xFF -> muted
                 */
                error =
                    Tfa98xx_ReadRegister16(handle, TFA98XX_AUDIO_CONTROL,
                                           &value);
        } else {
                error = Tfa98xx_Error_Bad_Parameter;
        }
        if (error == Tfa98xx_Error_Ok) {
                int volume_value;
                volume_value = (int)(voldB / (-0.5f));
                if (volume_value > 255) {
                        volume_value = 255;
                }
                /* volume value is in the top 8 bits of the register */
                value = (value & 0x00FF) | (unsigned short)(volume_value << 8);
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_AUDIO_CONTROL,
                                            value);
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_GetVolume(Tfa98xx_handle_t handle, float *pVoldB)
{
        Tfa98xx_Error_t error;
        unsigned short value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        assert(pVoldB != 0);
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_AUDIO_CONTROL, &value);
        if (error == Tfa98xx_Error_Ok) {
                value >>= 8;
                *pVoldB = (-0.5f) * value;
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_SetMute(Tfa98xx_handle_t handle, Tfa98xx_Mute_t mute)
{
        ALOGD("+Tfa98xx_SetMute");
        Tfa98xx_Error_t error;
        unsigned short audioctrl_value;
        unsigned short sysctrl_value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        error =
            Tfa98xx_ReadRegister16(handle, TFA98XX_AUDIO_CONTROL,
                                   &audioctrl_value);
        if (error != Tfa98xx_Error_Ok)
                return error;
       sysctrl_value |=(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS);
        error =
            Tfa98xx_ReadRegister16(handle, TFA98XX_SYSTEM_CONTROL,
                                   &sysctrl_value);
        if (error != Tfa98xx_Error_Ok)
                return error;
        switch (mute) {
        case Tfa98xx_Mute_Off:
                /* previous state can be digital or amplifier mute,
                 * clear the cf_mute and set the enbl_amplifier bits
                 *
                 * To reduce PLOP at power on it is needed to switch the
                 * amplifier on with the DCDC in follower mode (enbl_boost =
                 * ï¿?ï¿?at $09[4] This workaround is also needed when toggling the powerdown bit!
                 */
                audioctrl_value &= ~(TFA98XX_AUDIOCTRL_MUTE);
                sysctrl_value |= (TFA98XX_SYSCTRL_ENBL_AMP |
                                  TFA98XX_SYSCTRL_DCA );
                break;
        case Tfa98xx_Mute_Digital:
                /* expect the amplifier to run */
                /* set the cf_mute bit */
                audioctrl_value |= TFA98XX_AUDIOCTRL_MUTE;
                /* set the enbl_amplifier bit */
                sysctrl_value |= (TFA98XX_SYSCTRL_ENBL_AMP);
                /* clear active mode */
                sysctrl_value &= ~(TFA98XX_SYSCTRL_DCA);
                break;
        case Tfa98xx_Mute_Amplifier:
                /* clear the cf_mute bit */
                audioctrl_value &= ~TFA98XX_AUDIOCTRL_MUTE;
                /* clear the enbl_amplifier bit and active mode */
                sysctrl_value &= ~(TFA98XX_SYSCTRL_ENBL_AMP | TFA98XX_SYSCTRL_DCA);
                break;
        default:
                error = Tfa98xx_Error_Bad_Parameter;
        }
        if (error != Tfa98xx_Error_Ok)
                return error;
        error =
            Tfa98xx_WriteRegister16(handle, TFA98XX_AUDIO_CONTROL,
                                    audioctrl_value);
        if (error != Tfa98xx_Error_Ok)
                return error;
       sysctrl_value |=(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS);
        error =
            Tfa98xx_WriteRegister16(handle, TFA98XX_SYSTEM_CONTROL,
                                    sysctrl_value);
        ALOGD("-Tfa98xx_SetMute");
        return error;
}

Tfa98xx_Error_t Tfa98xx_GetMute(Tfa98xx_handle_t handle, Tfa98xx_Mute_t * pMute)
{
        Tfa98xx_Error_t error;
        unsigned short audioctrl_value;
        unsigned short sysctrl_value;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        assert(pMute != 0);
        error =
            Tfa98xx_ReadRegister16(handle, TFA98XX_AUDIO_CONTROL,
                                   &audioctrl_value);
        if (error != Tfa98xx_Error_Ok)
                return error;
        error =
            Tfa98xx_ReadRegister16(handle, TFA98XX_SYSTEM_CONTROL,
                                   &sysctrl_value);
        if (error != Tfa98xx_Error_Ok)
                return error;
        if (sysctrl_value & TFA98XX_SYSCTRL_ENBL_AMP) {
                /* amplifier is enabled */
                if (audioctrl_value & (TFA98XX_AUDIOCTRL_MUTE)) {
                        *pMute = Tfa98xx_Mute_Digital;
                } else {
                        *pMute = Tfa98xx_Mute_Off;
                }
        } else {
                /* amplifier disabled. */
                *pMute = Tfa98xx_Mute_Amplifier;
        }
        return Tfa98xx_Error_Ok;
}

void Tfa98xx_ConvertData2Bytes(int num_data, const int24 data[], unsigned char bytes[])
{
        int i;                  /* index for data */
        int k;                  /* index for bytes */
        int d;
        /* note: cannot just take the lowest 3 bytes from the 32 bit integer, because also need to take care of clipping any value > 2&23 */
        for (i = 0, k = 0; i < num_data; ++i, k += 3) {
                if (data[i] >= 0) {
                        d = MIN(data[i], (1 << 23) - 1);
                } else {
                        d = (1 << 24) - MIN(-data[i], 1 << 23); /* 2's complement */
                }
                assert(d >= 0);
                assert(d < (1 << 24));  /* max 24 bits in use */
                bytes[k] = (d >> 16) & 0xFF;    /* MSB */
                bytes[k + 1] = (d >> 8) & 0xFF;
                bytes[k + 2] = (d) & 0xFF;      /* LSB */
        }
}

static Tfa98xx_Error_t
calcBiquadCoeff(float b0, float b1, float b2,
                float a1, float a2, unsigned char bytes[BIQUAD_COEFF_SIZE * 3])
{
        float max_coeff;
        int headroom;
        int24 coeff_buffer[BIQUAD_COEFF_SIZE];
        /* find max value in coeff to define a scaler */
        max_coeff = (float)fabs(b0);
        if (fabs(b1) > max_coeff)
                max_coeff = (float)fabs(b1);
        if (fabs(b2) > max_coeff)
                max_coeff = (float)fabs(b2);
        if (fabs(a1) > max_coeff)
                max_coeff = (float)fabs(a1);
        if (fabs(a2) > max_coeff)
                max_coeff = (float)fabs(a2);
        /* round up to power of 2 */
        headroom = (int)ceil(log(max_coeff + pow(2.0, -23)) / log(2.0));
        /* some sanity checks on headroom */
        if (headroom > 8)
                return Tfa98xx_Error_Bad_Parameter;
        if (headroom < 0)
                headroom = 0;
        /* set in correct order and format for the DSP */
        coeff_buffer[0] = headroom;
        coeff_buffer[1] = (int24) (-a2 * pow(2.0, 23 - headroom));
        coeff_buffer[2] = (int24) (-a1 * pow(2.0, 23 - headroom));
        coeff_buffer[3] = (int24) (b2 * pow(2.0, 23 - headroom));
        coeff_buffer[4] = (int24) (b1 * pow(2.0, 23 - headroom));
        coeff_buffer[5] = (int24) (b0 * pow(2.0, 23 - headroom));
        /* convert float to fixed point and then bytes suitable for transmission over I2C */
        Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
        return Tfa98xx_Error_Ok;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetCoeff(Tfa98xx_handle_t handle,
                           int biquad_index, float b0,
                           float b1, float b2, float a1, float a2)
{
        Tfa98xx_Error_t error;
        unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        if (biquad_index > TFA98XX_BIQUAD_NUM)
                return Tfa98xx_Error_Bad_Parameter;
        if (biquad_index < 1)
                return Tfa98xx_Error_Bad_Parameter;
        error = calcBiquadCoeff(b0, b1, b2, a1, a2, bytes);
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_DspSetParam(handle, MODULE_BIQUADFILTERBANK,
                                        (unsigned char)biquad_index,
                                        (BIQUAD_COEFF_SIZE * 3), bytes);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetCoeffMultiple(int handle_cnt,
                                   Tfa98xx_handle_t handles[],
                                   int biquad_index, float b0,
                                   float b1, float b2, float a1, float a2)
{
        Tfa98xx_Error_t error;
        unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
        if (biquad_index > TFA98XX_BIQUAD_NUM)
                return Tfa98xx_Error_Bad_Parameter;
        if (biquad_index < 1)
                return Tfa98xx_Error_Bad_Parameter;
        error = calcBiquadCoeff(b0, b1, b2, a1, a2, bytes);
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                                                MODULE_BIQUADFILTERBANK,
                                                (unsigned char)biquad_index,
                                                (BIQUAD_COEFF_SIZE * 3), bytes);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetAllCoeff(Tfa98xx_handle_t handle,
                              float b[TFA98XX_BIQUAD_NUM * 5])
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        int i;
        unsigned char bytes[BIQUAD_COEFF_SIZE * TFA98XX_BIQUAD_NUM * 3];
        for (i = 0; (i < TFA98XX_BIQUAD_NUM) && (error == Tfa98xx_Error_Ok);
             ++i) {
                error =
                    calcBiquadCoeff(b[i * 5], b[i * 5 + 1], b[i * 5 + 2],
                                    b[i * 5 + 3], b[i * 5 + 4],
                                    bytes + i * BIQUAD_COEFF_SIZE * 3);
        }
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_DspSetParam(handle, MODULE_BIQUADFILTERBANK,
                                        0 /* program all at once */ ,
                                        (unsigned char)(BIQUAD_COEFF_SIZE *
                                                        TFA98XX_BIQUAD_NUM * 3),
                                        bytes);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetAllCoeffMultiple(int handle_cnt,
                                      Tfa98xx_handle_t
                                      handles[],
                                      float b[TFA98XX_BIQUAD_NUM * 5])
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        int i;
        unsigned char bytes[BIQUAD_COEFF_SIZE * TFA98XX_BIQUAD_NUM * 3];
        for (i = 0; (i < TFA98XX_BIQUAD_NUM) && (error == Tfa98xx_Error_Ok);
             ++i) {
                error =
                    calcBiquadCoeff(b[i * 5], b[i * 5 + 1], b[i * 5 + 2],
                                    b[i * 5 + 3], b[i * 5 + 4],
                                    bytes + i * BIQUAD_COEFF_SIZE * 3);
        }
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                                                MODULE_BIQUADFILTERBANK,
                                                0 /* program all at once */ ,
                                                (unsigned
                                                 char)(BIQUAD_COEFF_SIZE *
                                                       TFA98XX_BIQUAD_NUM * 3),
                                                bytes);
        }
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_Disable(Tfa98xx_handle_t handle, int biquad_index)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        int24 coeff_buffer[BIQUAD_COEFF_SIZE];
        unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
        if (biquad_index > TFA98XX_BIQUAD_NUM) {
                return Tfa98xx_Error_Bad_Parameter;
        }
        if (biquad_index < 1) {
                return Tfa98xx_Error_Bad_Parameter;
        }
        /* set in correct order and format for the DSP */
        coeff_buffer[0] = (int24) - 8388608;    /* -1.0f */
        coeff_buffer[1] = 0;
        coeff_buffer[2] = 0;
        coeff_buffer[3] = 0;
        coeff_buffer[4] = 0;
        coeff_buffer[5] = 0;
        /* convert float to fixed point and then bytes suitable for transmaission over I2C */
        Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
        error =
            Tfa98xx_DspSetParam(handle, MODULE_BIQUADFILTERBANK,
                                (unsigned char)biquad_index,
                                (unsigned char)BIQUAD_COEFF_SIZE * 3, bytes);
        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_DisableMultiple(int handle_cnt,
                                  Tfa98xx_handle_t handles[], int biquad_index)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        int24 coeff_buffer[BIQUAD_COEFF_SIZE];
        unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
        if (biquad_index > TFA98XX_BIQUAD_NUM) {
                return Tfa98xx_Error_Bad_Parameter;
        }
        if (biquad_index < 1) {
                return Tfa98xx_Error_Bad_Parameter;
        }
        /* set in correct order and format for the DSP */
        coeff_buffer[0] = (int24) - 8388608;    /* -1.0f */
        coeff_buffer[1] = 0;
        coeff_buffer[2] = 0;
        coeff_buffer[3] = 0;
        coeff_buffer[4] = 0;
        coeff_buffer[5] = 0;
        /* convert float to fixed point and then bytes suitable for transmaission over I2C */
        Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
        error =
            Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                                        MODULE_BIQUADFILTERBANK,
                                        (unsigned char)biquad_index,
                                        (unsigned char)BIQUAD_COEFF_SIZE * 3,
                                        bytes);
        return error;
}

#define PATCH_HEADER_LENGTH 6
Tfa98xx_Error_t
Tfa98xx_DspPatch(Tfa98xx_handle_t handle, int patchLength,
                 const unsigned char *patchBytes)
{
        ALOGD("Tfa98xx_DspPatch handle = %d patchLength = %d",handle,patchLength);
        Tfa98xx_Error_t error;
        if (!handle_is_open(handle))
        {
                return Tfa98xx_Error_NotOpen;
        }
        if (patchLength < PATCH_HEADER_LENGTH)
        {
                return Tfa98xx_Error_Bad_Parameter;
        }
        error = checkICROMversion(handle, patchBytes);
        if (Tfa98xx_Error_Ok != error) {
                return error;
        }
        error =
            processPatchFile(handle, patchLength - PATCH_HEADER_LENGTH,
                             patchBytes + PATCH_HEADER_LENGTH);
        ALOGD("-Tfa98xx_DspPatch handle = %d patchLength = %d",handle,patchLength);
        return error;
}

/* read the return code for the RPC call */
static Tfa98xx_Error_t checkRpcStatus(Tfa98xx_handle_t handle, int *pRpcStatus)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short cf_ctrl = 0x0002;        /* the value to sent to the CF_CONTROLS register: cf_req=00000000, cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
        unsigned short cf_mad = 0x0000; /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters) */
        unsigned char mem[3];   /* will be filled with the status read from DSP memory */
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        if (pRpcStatus == 0)
                return Tfa98xx_Error_Bad_Parameter;
#ifdef OPTIMIZED_RPC
        {
                /* minimize the number of I2C transactions by making use of the autoincrement in I2C */
                unsigned char buffer[4];
                /* first the data for CF_CONTROLS */
                buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
                buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
                /* write the contents of CF_MAD which is the subaddress following CF_CONTROLS */
                buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
                buffer[3] = (unsigned char)(cf_mad & 0xFF);
                error =
                    Tfa98xx_WriteData(handle, TFA98XX_CF_CONTROLS,
                                      sizeof(buffer), buffer);
        }
#else                           /*
                                 */
        /* 1) write DMEM=XMEM to the DSP XMEM */
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
                                            cf_ctrl);
        }
        if (error == Tfa98xx_Error_Ok) {
                /* write the address in XMEM where to read */
                error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, cf_mad);
        }
#endif                          /*
                                 */
        if (error == Tfa98xx_Error_Ok) {
                /* read 1 word (24 bit) from XMEM */
                error =
                    Tfa98xx_ReadData(handle, TFA98XX_CF_MEM,
                                     3 /*sizeof(mem) */ , mem);
        }
        if (error == Tfa98xx_Error_Ok) {
                *pRpcStatus = (mem[0] << 16) | (mem[1] << 8) | mem[2];
        }
        return error;
}

/* check that num_byte matches the memory type selected */
static Tfa98xx_Error_t check_size(Tfa98xx_DMEM_e which_mem, int num_bytes)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        int modulo_size = 1;
        switch (which_mem) {
        case Tfa98xx_DMEM_PMEM:
                /* 32 bit PMEM */
                modulo_size = 4;
                break;
        case Tfa98xx_DMEM_XMEM:
        case Tfa98xx_DMEM_YMEM:
        case Tfa98xx_DMEM_IOMEM:
                /* 24 bit MEM */
                modulo_size = 3;
                break;
        default:
                error = Tfa98xx_Error_Bad_Parameter;
        }
        if (error == Tfa98xx_Error_Ok) {
                if ((num_bytes % modulo_size) != 0) {
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
        return error;
}

static Tfa98xx_Error_t
writeParameter(Tfa98xx_handle_t handle,
               unsigned char module_id,
               unsigned char param_id,
               int num_bytes, const unsigned char data[])
{
        Tfa98xx_Error_t error;
        unsigned short cf_ctrl = 0x0002;        /* the value to be sent to the CF_CONTROLS register: cf_req=00000000, cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
        unsigned short cf_mad = 0x0001; /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters) */
        error = check_size(Tfa98xx_DMEM_XMEM, num_bytes);
        if (error == Tfa98xx_Error_Ok) {
                if ((num_bytes <= 0) || (num_bytes > MAX_PARAM_SIZE)) {
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
#ifdef OPTIMIZED_RPC
        {
                /* minimize the number of I2C transactions by making use of the autoincrement in I2C */
                unsigned char buffer[7];
                /* first the data for CF_CONTROLS */
                buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
                buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
                /* write the contents of CF_MAD which is the subaddress following CF_CONTROLS */
                buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
                buffer[3] = (unsigned char)(cf_mad & 0xFF);
                /* write the module and RPC id into CF_MEM, which follows CF_MAD */
                buffer[4] = 0;
                buffer[5] = module_id + 128;
                buffer[6] = param_id;
                error =
                    Tfa98xx_WriteData(handle, TFA98XX_CF_CONTROLS,
                                      sizeof(buffer), buffer);
        }
#else                           /*
                                 */
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
                                            cf_ctrl);
        }
        if (error == Tfa98xx_Error_Ok) {
                error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, cf_mad);
        }
        if (error == Tfa98xx_Error_Ok) {
                unsigned char id[3];
                id[0] = 0;
                id[1] = module_id + 128;
                id[2] = param_id;
                error = Tfa98xx_WriteData(handle, TFA98XX_CF_MEM, 3, id);
        }
#endif                          /* OPTIMIZED_RPC */
        if (error == Tfa98xx_Error_Ok) {
                int offset = 0;
                int chunk_size =
                    ROUND_DOWN(NXP_I2C_MAX_SIZE, 3 /* XMEM word size */ );
                int remaining_bytes = num_bytes;
                /* due to autoincrement in cf_ctrl, next write will happen at the next address */
                while ((error == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
                        if (remaining_bytes < chunk_size) {
                                chunk_size = remaining_bytes;
                        }
                        // else chunk_size remains at initialize value above
                        error =
                            Tfa98xx_WriteData(handle, TFA98XX_CF_MEM,
                                              chunk_size, data + offset);
                        remaining_bytes -= chunk_size;
                        offset += chunk_size;
                }
        }
        return error;
}

static Tfa98xx_Error_t executeParam(Tfa98xx_handle_t handle)
{
        Tfa98xx_Error_t error;
        unsigned short cf_ctrl = 0x0002;        /* the value to be sent to the CF_CONTROLS register: cf_req=00000000, cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
        cf_ctrl |= (1 << 8) | (1 << 4); /* set the cf_req1 and cf_int bit */
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
        return error;
}

static Tfa98xx_Error_t waitResult(Tfa98xx_handle_t handle)
{
        Tfa98xx_Error_t error;
        unsigned short cf_status;       /* the contents of the CF_STATUS register */
        int tries = 0;
        do {
                error =
                    Tfa98xx_ReadRegister16(handle, TFA98XX_CF_STATUS,
                                           &cf_status);
                tries++;
        }
        while ((error == Tfa98xx_Error_Ok) && ((cf_status & 0x0100) == 0) && (tries < TFA98XX_API_WAITRESULT_NTRIES));     /* don't wait forever, DSP is pretty quick to respond (< 1ms) */
        if (tries >= TFA98XX_API_WAITRESULT_NTRIES) {
                /* something wrong with communication with DSP */
                error = Tfa98xx_Error_DSP_not_running;
        }
        return error;
}

/* Execute RPC protocol to write something to the DSP */
Tfa98xx_Error_t
Tfa98xx_DspSetParam(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id, int num_bytes,
                    const unsigned char data[])
{
        ALOGD("+Tfa98xx_DspSetParam");
        Tfa98xx_Error_t error;
        int rpcStatus = STATUS_OK;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        /* 1) write the id and data to the DSP XMEM */
        error = writeParameter(handle, module_id, param_id, num_bytes, data);
        /* 2) wake up the DSP and let it process the data */
        if (error == Tfa98xx_Error_Ok) {
                error = executeParam(handle);
        }
        /* check the result when addressed an IC uniquely */
        if (handlesLocal[handle].slave_address != TFA98XX_GENERIC_SLAVE_ADDRESS)
        {
                ALOGD("handlesLocal[handle].slave_address = 0x%x TFA98XX_GENERIC_SLAVE_ADDRESS = 0x%x",handlesLocal[handle].slave_address,TFA98XX_GENERIC_SLAVE_ADDRESS);
                /* 3) wait for the ack */
                if (error == Tfa98xx_Error_Ok)
                {
                        error = waitResult(handle);
                }
                /* 4) check the RPC return value */
                if (error == Tfa98xx_Error_Ok)
                {
                        error = checkRpcStatus(handle, &rpcStatus);
                }
                if (error == Tfa98xx_Error_Ok)
                {
                        if (rpcStatus != STATUS_OK) {
                                /* DSP RPC call returned an error */
                                error =
                                    (Tfa98xx_Error_t) (rpcStatus +
                                                       Tfa98xx_Error_RpcBase);
                        }
                }
        }
        ALOGD("-Tfa98xx_DspSetParam");
        return error;
}

/* Execute RPC protocol to write something to all the DSPs interleaved, stop at the first error
 * optimized to minimize the latency between the execution point on the various DSPs */
Tfa98xx_Error_t
Tfa98xx_DspSetParamMultiple(int handle_cnt,
                            Tfa98xx_handle_t handles[],
                            unsigned char module_id,
                            unsigned char param_id,
                            int num_bytes, const unsigned char data[])
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        int i;
        int rpcStatus = STATUS_OK;
        for (i = 0; i < handle_cnt; ++i) {
                if (!handle_is_open(handles[i]))
                        return Tfa98xx_Error_NotOpen;
        }
        /* from here onward, any error will fall through without executing the following for loops */
        /* 1) write the id and data to the DSP XMEM */
        for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
                error =
                    writeParameter(handles[i], module_id, param_id, num_bytes,
                                   data);
        }
        /* 2) wake up the DSP and let it process the data */
        for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
                error = executeParam(handles[i]);
        }
        /* 3) wait for the ack */
        for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
                error = waitResult(handles[i]);
        }
        /* 4) check the RPC return value */
        for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
                error = checkRpcStatus(handles[i], &rpcStatus);
                if (rpcStatus != STATUS_OK) {
                        /* DSP RPC call returned an error */
                        error =
                            (Tfa98xx_Error_t) (rpcStatus +
                                               Tfa98xx_Error_RpcBase);
                        /* stop at first error */
                        return error;
                }
        }
        return error;
}

/* Execute RPC protocol to read something from the DSP */
Tfa98xx_Error_t
Tfa98xx_DspGetParam(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id, int num_bytes, unsigned char data[])
{
        Tfa98xx_Error_t error;
        unsigned short cf_ctrl = 0x0002;        /* the value to be sent to the CF_CONTROLS register: cf_req=00000000, cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
        unsigned short cf_mad = 0x0001; /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters) */
        unsigned short cf_status;       /* the contents of the CF_STATUS register */
        int rpcStatus = STATUS_OK;
        if (!handle_is_open(handle))
                return Tfa98xx_Error_NotOpen;
        if (handlesLocal[handle].slave_address == TFA98XX_GENERIC_SLAVE_ADDRESS) {
                /* cannot read */
                return Tfa98xx_Error_Bad_Parameter;
        }
        error = check_size(Tfa98xx_DMEM_XMEM, num_bytes);
        if (error == Tfa98xx_Error_Ok) {
                if ((num_bytes <= 0) || (num_bytes > MAX_PARAM_SIZE)) {
                        error = Tfa98xx_Error_Bad_Parameter;
                }
        }
#ifdef OPTIMIZED_RPC
        {
                /* minimize the number of I2C transactions by making use of the autoincrement in I2C */
                unsigned char buffer[7];
                /* first the data for CF_CONTROLS */
                buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
                buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
                /* write the contents of CF_MAD which is the subaddress following CF_CONTROLS */
                buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
                buffer[3] = (unsigned char)(cf_mad & 0xFF);
                /* write the module and RPC id into CF_MEM, which follows CF_MAD */
                buffer[4] = 0;
                buffer[5] = module_id + 128;
                buffer[6] = param_id;
                error =
                    Tfa98xx_WriteData(handle, TFA98XX_CF_CONTROLS,
                                      sizeof(buffer), buffer);
        }
#else                           /*
                                 */
        /* 1) write the id and data to the DSP XMEM */
        if (error == Tfa98xx_Error_Ok) {
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
                                            cf_ctrl);
        }
        if (error == Tfa98xx_Error_Ok) {
                error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, cf_mad);
        }
        if (error == Tfa98xx_Error_Ok) {
                unsigned char id[3];
                id[0] = 0;
                id[1] = module_id + 128;
                id[2] = param_id;
                /* only try MEM once, if error, need to resend mad as well */
                error = Tfa98xx_WriteData(handle, TFA98XX_CF_MEM, 3, id);
        }
#endif                          /*
                                 */
        /* 2) wake up the DSP and let it process the data */
        if (error == Tfa98xx_Error_Ok) {
                cf_ctrl |= (1 << 8) | (1 << 4); /* set the cf_req1 and cf_int bit */
                error =
                    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
                                            cf_ctrl);
        }
        /* 3) wait for the ack */
        if (error == Tfa98xx_Error_Ok) {
                int tries = 0;
                do {
                        error =
                            Tfa98xx_ReadRegister16(handle, TFA98XX_CF_STATUS,
                                                   &cf_status);
                        tries++;
                }
                while ((error == Tfa98xx_Error_Ok) && ((cf_status & 0x0100) == 0) && (tries < TFA98XX_API_WAITRESULT_NTRIES));     /* don't wait forever, DSP is pretty quick to respond (< 1ms) */
                if (tries >= TFA98XX_API_WAITRESULT_NTRIES) {
                        /* something wrong with communication with DSP */
                        return Tfa98xx_Error_DSP_not_running;
                }
        }
        /* 4) check the RPC return value */
        if (error == Tfa98xx_Error_Ok) {
                error = checkRpcStatus(handle, &rpcStatus);
        }
        if (error == Tfa98xx_Error_Ok) {
                if (rpcStatus != STATUS_OK) {
                        /* DSP RPC call returned an error */
                        error =
                            (Tfa98xx_Error_t) (rpcStatus +
                                               Tfa98xx_Error_RpcBase);
                }
        }
        /* 5) read the resulting data */
        if (error == Tfa98xx_Error_Ok) {
                cf_mad = 0x0002;        /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters) */
                error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, cf_mad);
        }
        if (error == Tfa98xx_Error_Ok) {
                int offset = 0;
                int chunk_size =
                    ROUND_DOWN(NXP_I2C_MAX_SIZE, 3 /* XMEM word size */ );
                int remaining_bytes = num_bytes;
                /* due to autoincrement in cf_ctrl, next write will happen at the next address */
                while ((error == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
                        if (remaining_bytes < 252) {
                                chunk_size = remaining_bytes;
                        }
                        // else chunk_size remains at initialize value above
                        error =
                            Tfa98xx_ReadData(handle, TFA98XX_CF_MEM, chunk_size,
                                             data + offset);
                        remaining_bytes -= chunk_size;
                        offset += chunk_size;
                }
        }
        return error;
}

/* convert DSP memory bytes to signed 24 bit integers
   data contains "num_bytes/3" elements
   bytes contains "num_bytes" elements */
void Tfa98xx_ConvertBytes2Data(int num_bytes, const unsigned char bytes[], int24 data[])
{
        int i;                  /* index for data */
        int k;                  /* index for bytes */
        int d;
        int num_data = num_bytes / 3;
        assert((num_bytes % 3) == 0);
        for (i = 0, k = 0; i < num_data; ++i, k += 3) {
                d = (bytes[k] << 16) | (bytes[k + 1] << 8) | (bytes[k + 2]);
                assert(d >= 0);
                assert(d < (1 << 24));  /* max 24 bits in use */
                if (bytes[k] & 0x80) {  /* sign bit was set */
                        d = -((1 << 24) - d);
                }
                data[i] = d;
        }
}

Tfa98xx_Error_t
Tfa98xx_DspGetStateInfo(Tfa98xx_handle_t handle,
                                     Tfa98xx_StateInfo_t * pInfo)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        int24 data[FW_STATE_SIZE + FW_STATEDBDRC_SIZE]; /* allocate worst case */
        unsigned char bytes[3 * (FW_STATE_SIZE + FW_STATEDBDRC_SIZE)];
		int bSupportDBDrc;
		int stateSize = FW_STATE_SIZE; /* contains the actual amount of parameters transferred, depends on IC and ROM code version */
		int i;

        assert(pInfo != 0);

		/* init to default value to have sane values even when some features aren't supported */
		for (i=0; i<FW_STATE_SIZE + FW_STATEDBDRC_SIZE; i++) {
			data[i] = 0;
		}

		error = Tfa98xx_DspSupportDBDrc(handle, &bSupportDBDrc);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
		if (bSupportDBDrc) stateSize += FW_STATEDBDRC_SIZE;

        error =
            Tfa98xx_DspGetParam(handle, MODULE_FRAMEWORK, FW_PARAM_GET_STATE,
                                3 * stateSize, bytes);
        if (error != Tfa98xx_Error_Ok) {
			if (error == Tfa98xx_Error_RpcParamId) {
				/* old ROM code, ask SpeakerBoost and only do first portion */
				stateSize=8;
				error =
					Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST, SB_PARAM_GET_STATE,
										3 * stateSize, bytes);
			}
			else {
				return error;
			}
        }
        Tfa98xx_ConvertBytes2Data(3 * stateSize, bytes, data);
        pInfo->agcGain = (float)data[0] / (1 << (23 - SPKRBST_AGCGAIN_EXP));    /* /2^23*2^(SPKRBST_AGCGAIN_EXP) */
        pInfo->limGain = (float)data[1] / (1 << (23 - SPKRBST_LIMGAIN_EXP));    /* /2^23*2^(SPKRBST_LIMGAIN_EXP) */
        pInfo->sMax = (float)data[2] / (1 << (23 - SPKRBST_HEADROOM));  /* /2^23*2^(SPKRBST_HEADROOM) */
        pInfo->T = data[3] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));     /* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
        pInfo->statusFlag = data[4];
        pInfo->X1 = (float)data[5] / (1 << (23 - SPKRBST_HEADROOM));    /* /2^23*2^(SPKRBST_HEADROOM) */
        pInfo->X2 = (float)data[6] / (1 << (23 - SPKRBST_HEADROOM));    /* /2^23*2^(SPKRBST_HEADROOM) */
        pInfo->Re = (float)data[7] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));     /* /2^23*2^(SPKRBST_TEMPERATURE_EXP) */
		pInfo->shortOnMips = data[8];
		pInfo->dbdrcState.GRhighDrc1[0] = (float)data[9] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRhighDrc1[1] = (float)data[10] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRhighDrc2[0] = (float)data[11] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRhighDrc2[1] = (float)data[12] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRlowDrc1[0] = (float)data[13] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRlowDrc1[1] = (float)data[14] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRlowDrc2[0] = (float)data[15] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRlowDrc2[1] = (float)data[16] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRpostDrc1[0] = (float)data[17] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRpostDrc1[1] = (float)data[18] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRpostDrc2[0] = (float)data[19] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRpostDrc2[1] = (float)data[20] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRblDrc[0] = (float)data[21] / (1 << (23 - DSP_MAX_GAIN_EXP));
		pInfo->dbdrcState.GRblDrc[1] = (float)data[22] / (1 << (23 - DSP_MAX_GAIN_EXP));

        return error;
}

Tfa98xx_Error_t
Tfa98xx_DspGetCalibrationImpedance(Tfa98xx_handle_t handle, float *pRe25)
{
	Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
	unsigned char bytes[3];
	int24 data[1];

   error = Tfa98xx_DspGetParam(  handle, MODULE_SPEAKERBOOST,
                                 SB_PARAM_GET_RE0, 3, bytes);
	if (error == Tfa98xx_Error_Ok)
	{
		Tfa98xx_ConvertBytes2Data(3, bytes, data);
		*pRe25 = (float)data[0] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));        /* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
	}
	return error;
}

Tfa98xx_Error_t
Tfa98xx_DspReadMem(Tfa98xx_handle_t handle,
                   unsigned short start_offset, int num_words, int *pValues)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short cf_ctrl; /* the value to sent to the CF_CONTROLS register */
        unsigned char bytes[MAX_I2C_LENGTH];
        int burst_size;         /* number of words per burst size */
        int bytes_per_word = 3;
        int num_bytes;
        int *p;
        /* first set DMEM and AIF, leaving other bits intact */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        cf_ctrl &= ~0x000E;     /* clear AIF & DMEM */
        cf_ctrl |= (Tfa98xx_DMEM_XMEM << 1);    /* set DMEM, leave AIF cleared for autoincrement */
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, start_offset);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        num_bytes = num_words * bytes_per_word;
        p = pValues;
        for (; num_bytes > 0;) {
                burst_size = ROUND_DOWN(MAX_I2C_LENGTH, bytes_per_word);
                if (num_bytes < burst_size) {
                        burst_size = num_bytes;
                }
                assert(burst_size <= sizeof(bytes));
                error =
                    Tfa98xx_ReadData(handle, TFA98XX_CF_MEM, burst_size, bytes);
                if (error != Tfa98xx_Error_Ok) {
                        return error;
                }
                Tfa98xx_ConvertBytes2Data(burst_size, bytes, p);
                num_bytes -= burst_size;
                p += burst_size / bytes_per_word;
        }
        return Tfa98xx_Error_Ok;
}

Tfa98xx_Error_t
Tfa98xx_DspWriteMem(Tfa98xx_handle_t handle, unsigned short address, int value)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short cf_ctrl; /* the value to sent to the CF_CONTROLS register */
        unsigned char bytes[3];
        /* first set DMEM and AIF, leaving other bits intact */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        cf_ctrl &= ~0x000E;     /* clear AIF & DMEM */
        cf_ctrl |= (Tfa98xx_DMEM_XMEM << 1);    /* set DMEM, leave AIF cleared for autoincrement */
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, address);
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


/* Execute generic RPC protocol that has both input and output parameters */
Tfa98xx_Error_t
Tfa98xx_DspExecuteRpc(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id, int num_inbytes, unsigned char indata[],
                    int num_outbytes, unsigned char outdata[])
{
    Tfa98xx_Error_t error;
    int rpcStatus = STATUS_OK;
    int i;


    if (!handle_is_open(handle))
            return Tfa98xx_Error_NotOpen;

    /* 1) write the id and data to the DSP XMEM */
    error = writeParameter(handle, module_id, param_id, num_inbytes, indata);

    /* 2) wake up the DSP and let it process the data */
    if (error == Tfa98xx_Error_Ok) {
            error = executeParam(handle);
    }

    /* 3) wait for the ack */
	  if (error == Tfa98xx_Error_Ok) {
			error = waitResult(handle);
	  }

	/* 4) check the RPC return value */
	if (error == Tfa98xx_Error_Ok) {
			error = checkRpcStatus(handle, &rpcStatus);
	}
	if (error == Tfa98xx_Error_Ok) {
			if (rpcStatus != STATUS_OK) {
					/* DSP RPC call returned an error */
					error =
						(Tfa98xx_Error_t) (rpcStatus +
										   Tfa98xx_Error_RpcBase);
			}
	}

    /* 5) read the resulting data */
    error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, 2 /*start_offset*/);
    if (error != Tfa98xx_Error_Ok) {
            return error;
    }
    /* read in chunks, limited by max I2C length */
    for (i=0 ; i < num_outbytes ; ) {
    	    int burst_size = ROUND_DOWN(MAX_I2C_LENGTH, 3 /*bytes_per_word*/);
            if ( (num_outbytes-i) < burst_size) {
                    burst_size = num_outbytes-i;
            }
            error =
                Tfa98xx_ReadData(handle, TFA98XX_CF_MEM, burst_size, outdata+i);
            if (error != Tfa98xx_Error_Ok) {
                    return error;
            }
            i += burst_size;
    }

    return error;
}

Tfa98xx_Error_t
Tfa98xx_DspReadMemory(Tfa98xx_handle_t handle, Tfa98xx_DMEM_e which_mem,
                   unsigned short start_offset, int num_words, int *pValues)
{
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

	int input[3];
	unsigned char input_bytes[3*3];
	unsigned char output_bytes[80*3];

	/* want to read all in 1 RPC call, so limit the max size */
	if (num_words > 80)
	{
		return Tfa98xx_Error_Bad_Parameter;
	}

	input[0] = which_mem;
	input[1] = start_offset;
	input[2] = num_words;
	Tfa98xx_ConvertData2Bytes(3, input, input_bytes);

	error = Tfa98xx_DspExecuteRpc(handle, 0 /* moduleId*/, 5 /* paramId */,
								sizeof(input_bytes), input_bytes,
								num_words*3, output_bytes);

	Tfa98xx_ConvertBytes2Data(num_words*3, output_bytes, pValues);

	return error;
}

Tfa98xx_Error_t Tfa98xx_DspWriteMemory(Tfa98xx_handle_t handle, Tfa98xx_DMEM_e which_mem,
                   unsigned short start_offset, int num_words, int *pValues)
{
	int output[3];
	unsigned char output_bytes[83*3];
	int num_bytes;

	/* want to read all in 1 RPC call, so limit the max size */
	if (num_words > 80)
	{
		return Tfa98xx_Error_Bad_Parameter;
	}

	output[0] = which_mem;
	output[1] = start_offset;
	output[2] = num_words;
	Tfa98xx_ConvertData2Bytes(3, output, output_bytes);
	Tfa98xx_ConvertData2Bytes(num_words, pValues, output_bytes+3*3);

	num_bytes = 3*(num_words+3);

	return Tfa98xx_DspSetParam(handle, 0 /* framework */, 4 /* param */, num_bytes, output_bytes);
}

/* load all the parameters for the DBDRC settings from a file */
Tfa98xx_Error_t Tfa98xx_DspWriteDBDrc(Tfa98xx_handle_t handle,
									   int length,
									   const unsigned char *pDBDrcBytes)
{
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
    if (pDBDrcBytes != 0) {
            /* by design: keep the data opaque and no interpreting/calculation */
            error =
                Tfa98xx_DspSetParam(handle, MODULE_SPEAKERBOOST,
                                    SB_PARAM_SET_DBDRC, length, pDBDrcBytes);
    } else {
            error = Tfa98xx_Error_Bad_Parameter;
    }
    return error;
}

Tfa98xx_Error_t Tfa98xx_DspWriteDBDrcMultiple(int handle_cnt,
                                          Tfa98xx_handle_t handles[],
                                          int length,
                                          const unsigned char *pDBDrcBytes)
{
        Tfa98xx_Error_t error;
        if (pDBDrcBytes != 0) {
                /* by design: keep the data opaque and no interpreting/calculation */
                error =
                    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                                                MODULE_SPEAKERBOOST,
                                                SB_PARAM_SET_DBDRC, length,
                                                pDBDrcBytes);
        } else {
                error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

Tfa98xx_Error_t Tfa98xx_DspSetAgcGainInsert(Tfa98xx_handle_t handle,
									   Tfa98xx_AgcGainInsert_t agcGainInsert)
{
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
    unsigned char bytes[3];
    Tfa98xx_ConvertData2Bytes(1, (int24*)&agcGainInsert, bytes);

    error =
        Tfa98xx_DspSetParam(handle, MODULE_SPEAKERBOOST,
                            SB_PARAM_SET_AGCINS, 3, bytes);
    return error;
}
Tfa98xx_Error_t Tfa98xx_DspSetAgcGainInsertMultiple(
											int handle_cnt,
											Tfa98xx_handle_t handles[],
											Tfa98xx_AgcGainInsert_t agcGainInsert)
{
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
    unsigned char bytes[3];

	Tfa98xx_ConvertData2Bytes(1, (int24*)&agcGainInsert, bytes);

    error =
        Tfa98xx_DspSetParamMultiple(handle_cnt, handles, MODULE_SPEAKERBOOST,
                            SB_PARAM_SET_AGCINS, 3, bytes);
    return error;
}

#ifdef WIN32
Tfa98xx_Error_t Tfa98xx_I2C_Init( void)
{
	NXP_I2C_Error_t retval = NXP_I2C_UnassignedErrorCode;

   retval = init_I2C();

   return (Tfa98xx_Error_t) retval;
}
#endif
