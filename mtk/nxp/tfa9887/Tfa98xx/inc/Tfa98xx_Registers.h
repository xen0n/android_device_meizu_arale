#ifndef TFA98XX_REGISTERS_H
#define TFA98XX_REGISTERS_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "Tfa98xx_genregs.h"
/*
 * early defs aligned with (tbd) generated names
 */

typedef unsigned char subaddress_t;
/* power-on/reset values */
#define TFA98XX_SYSTEM_CONTROL_POR		(0x025D)/*For BCK, 425D for WS*/
#define TFA98XX_CALIBRATION_POR			(0x3800)

/* SystemControl bits */
#define TFA98XX_SYSTEM_CONTROL_PWDN     (1<<0)
#define TFA98XX_SYSTEM_CONTROL_I2CR		(1<<1)
#define TFA98XX_SYSTEM_CONTROL_CFE_POS	(2)
#define TFA98XX_SYSTEM_CONTROL_CFE      (1<<TFA98XX_SYSTEM_CONTROL_CFE_POS)
#define TFA98XX_SYSTEM_CONTROL_AMPE_POS (3)
#define TFA98XX_SYSTEM_CONTROL_AMPE     (1<<TFA98XX_SYSTEM_CONTROL_AMPE_POS)
#define TFA98XX_SYSTEM_CONTROL_DCA      (1<<4) // DC-to-DC converter mode select
#define TFA98XX_SYSTEM_CONTROL_SBSL   	(1<<5)
#define TFA98XX_SYSTEM_CONTROL_AMPC 	(1<<6)
#define TFA98XX_SYSTEM_CONTROL_ISEL_POS (13)
#define TFA98XX_SYSTEM_CONTROL_IPLL_POS (14)
#define TFA98XX_SYSTEM_CONTROL_ISEL 	(1<<TFA98XX_SYSTEM_CONTROL_ISEL_POS)
#define TFA98XX_SYSTEM_CONTROL_IPLL 	(1<<TFA98XX_SYSTEM_CONTROL_IPLL_POS)

/* CurrentSense registers */
#define TFA98XX_CURRENTSENSE1			(0x46)
#define TFA98XX_CURRENTSENSE2			(0x47)
#define TFA98XX_CURRENTSENSE3			(0x48)
#define TFA98XX_CURRENTSENSE4			(0x49)

#define TFA98XX_REVISION       (subaddress_t)0x03
#define TFA98XX_I2S_CONTROL    (subaddress_t)0x04
#define TFA98XX_AUDIO_CONTROL  (subaddress_t)0x06
#define TFA98XX_CALIBRATION    (subaddress_t)0x08
#define TFA98XX_SYSTEM_CONTROL (subaddress_t)0x09
#define TFA98XX_I2S_SEL        (subaddress_t)0x0A

#define TFA98XX_STATUS         (unsigned short)0x00

#define TFA98XX_SPKR_REG       (unsigned short)0x08

#define TFA98XX_MTP            (unsigned short)0x80

/* STATUS bits */
#define TFA98XX_STATUS_VDDS       (1<<0) /*  */
#define TFA98XX_STATUS_PLLS       (1<<1) /* plls locked */
#define TFA98XX_STATUS_OTDS       (1<<2) /*  */
#define TFA98XX_STATUS_OVDS       (1<<3) /*  */
#define TFA98XX_STATUS_UVDS       (1<<4) /*  */
#define TFA98XX_STATUS_OCDS       (1<<5) /*  */
#define TFA98XX_STATUS_CLKS       (1<<6) /* clocks stable */
//
#define TFA98XX_STATUS_MTPB       (1<<8) /* MTP operation busy */
//
#define TFA98XX_STATUS_DCCS       (1<<9) /*  */

#define TFA98XX_STATUS_ACS        (1<<11) /* cold started */
#define TFA98XX_STATUS_SWS        (1<<12) /* amplifier switching */
#define TFA98XX_STATUS_WDS        (1<<13) /* DSP watchDog */
#define TFA98XX_STATUS_AMPS       (1<<14)
#define TFA98XX_STATUS_AREFS      (1<<15)

/* Speaker control register */
#define TFA98XX_SPKR_DCPVP        (1<<10) /* DCDC protection */

/* MTP bits */
#define TFA98XX_MTP_MTPOTC        (1<<0)  /* one time calibration */
#define TFA98XX_MTP_MTPEX         (1<<1)  /* one time calibration done */

/*
 * generated defines
 */
#define TFA98XX_STATUSREG (0x00)
#define TFA98XX_BATTERYVOLTAGE (0x01)
#define TFA98XX_TEMPERATURE (0x02)
#define TFA98XX_I2SREG (0x04)
#define TFA98XX_BAT_PROT (0x05)
#define TFA98XX_AUDIO_CTR (0x06)
#define TFA98XX_DCDCBOOST (0x07)
#define TFA98XX_SPKR_CALIBRATION (0x08)
#define TFA98XX_SYS_CTRL (0x09)
#define TFA98XX_I2S_SEL_REG (0x0a)
#define TFA98XX_REVISIONNUMBER (0x03)
#define TFA98XX_HIDE_UNHIDE_KEY (0x40)
#define TFA98XX_PWM_CONTROL (0x41)
#define TFA98XX_CURRENTSENSE1 (0x46)
#define TFA98XX_CURRENTSENSE2 (0x47)
#define TFA98XX_CURRENTSENSE3 (0x48)
#define TFA98XX_CURRENTSENSE4 (0x49)
#define TFA98XX_ABISTTEST (0x4c)
#define TFA98XX_RESERVE1 (0x0c)
#define TFA98XX_MTP_COPY (0x62)
#define TFA98XX_CF_CONTROLS (0x70)
#define TFA98XX_CF_MAD (0x71)
#define TFA98XX_CF_MEM (0x72)
#define TFA98XX_CF_STATUS (0x73)
#define TFA98XX_RESERVE2 (0x0d)

#ifdef __cplusplus
}
#endif

#endif // TFA98XX_REGISTERS_H

