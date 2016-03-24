#ifndef TFA9887_REGISTERS_H
#define TFA9887_REGISTERS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <Tfa98xx_genregs.h>

#define TFA9887_STATUS         (unsigned short)0x00

#define TFA9887_MTP            (unsigned short)0x80

/* STATUS bits */
#define TFA9887_STATUS_VDDS       (1<<0) /*  */
#define TFA9887_STATUS_PLLS       (1<<1) /* plls locked */
#define TFA9887_STATUS_OTDS       (1<<2) /*  */
#define TFA9887_STATUS_OVDS       (1<<3) /*  */
#define TFA9887_STATUS_UVDS       (1<<4) /*  */
#define TFA9887_STATUS_OCDS       (1<<5) /*  */
#define TFA9887_STATUS_CLKS       (1<<6) /* clocks stable */
//
#define TFA9887_STATUS_MTPB       (1<<8) /* MTP operation busy */
//
#define TFA9887_STATUS_DCCS       (1<<9) /*  */

#define TFA9887_STATUS_ACS        (1<<11) /* cold started */
#define TFA9887_STATUS_SWS        (1<<12) /* amplifier switching */
#define TFA9887_STATUS_WDS        (1<<13) /* DSP watchDog */
#define TFA9887_STATUS_AMPS       (1<<14)

/* SystemControl bits */
#define TFA9887_SYSTEM_CONTROL_PWDN     (1<<0)
#define TFA9887_SYSTEM_CONTROL_I2CR		(1<<1)
#define TFA9887_SYSTEM_CONTROL_CFE_POS	(2)
#define TFA9887_SYSTEM_CONTROL_CFE      (1<<TFA9887_SYSTEM_CONTROL_CFE_POS)
#define TFA9887_SYSTEM_CONTROL_AMPE_POS (3)
#define TFA9887_SYSTEM_CONTROL_AMPE     (1<<TFA9887_SYSTEM_CONTROL_AMPE_POS)
#define TFA9887_SYSTEM_CONTROL_DCA      (1<<4) // DC-to-DC converter mode select
#define TFA9887_SYSTEM_CONTROL_SBSL   	(1<<5)
#define TFA9887_SYSTEM_CONTROL_AMPC 	(1<<6)
#define TFA9887_SYSTEM_CONTROL_ISEL_POS (13)
#define TFA9887_SYSTEM_CONTROL_IPLL_POS (14)
#define TFA9887_SYSTEM_CONTROL_ISEL 	(1<<TFA9887_SYSTEM_CONTROL_ISEL_POS)
#define TFA9887_SYSTEM_CONTROL_IPLL 	(1<<TFA9887_SYSTEM_CONTROL_IPLL_POS)

/* MTP bits */
#define TFA9887_MTP_MTPOTC        (1<<0)  /* one time calibration */
#define TFA9887_MTP_MTPEX         (1<<1)  /* one time calibration done */

/*
 * generated defines
 */
#define TFA9887_STATUSREG (0x00)
#define TFA9887_BATTERYVOLTAGE (0x01)
#define TFA9887_TEMPERATURE (0x02)
#define TFA9887_I2SREG (0x04)
#define TFA9887_BAT_PROT (0x05)
#define TFA9887_AUDIO_CTR (0x06)
#define TFA9887_DCDCBOOST (0x07)
#define TFA9887_SPKR_CALIBRATION (0x08)
#define TFA9887_SYS_CTRL (0x09)
#define TFA9887_I2S_SEL_REG (0x0a)
#define TFA9887_REVISIONNUMBER (0x03)
#define TFA9887_HIDE_UNHIDE_KEY (0x40)
#define TFA9887_PWM_CONTROL (0x41)
#define TFA9887_CURRENTSENSE1 (0x46)
#define TFA9887_CURRENTSENSE2 (0x47)
#define TFA9887_CURRENTSENSE3 (0x48)
#define TFA9887_CURRENTSENSE4 (0x49)
#define TFA9887_ABISTTEST (0x4c)
#define TFA9887_RESERVE1 (0x0c)
#define TFA9887_MTP_COPY (0x62)
#define TFA9887_CF_CONTROLS (0x70)
#define TFA9887_CF_MAD (0x71)
#define TFA9887_CF_MEM (0x72)
#define TFA9887_CF_STATUS (0x73)
#define TFA9887_RESERVE2 (0x0d)

#ifdef __cplusplus
}
#endif

#endif // TFA9887_REGISTERS_H

