/**   StatusReg Register ($00) **/
#define TFA98XX_STATUSREG           0x00
#define TFA9890_STATUSREG_POR       0x0a5c
/* POR */
/*     0=VddD is below 1.2V     */
/*     1=VddD is OK     */
#define TFA98XX_STATUSREG_VDDS        (0x1<<0)
#define TFA98XX_STATUSREG_VDDS_POS        0
#define TFA98XX_STATUSREG_VDDS_MAX        1
#define TFA98XX_STATUSREG_VDDS_MSK        0x1
/* PLL */
/*     0=PLL not in lock     */
/*     1=PLL in lock     */
#define TFA98XX_STATUSREG_PLLS        (0x1<<1)
#define TFA98XX_STATUSREG_PLLS_POS        1
#define TFA98XX_STATUSREG_PLLS_MAX        1
#define TFA98XX_STATUSREG_PLLS_MSK        0x2
/* OTP */
/*     0=Temperature To High     */
/*     1=Temperature OK     */
#define TFA98XX_STATUSREG_OTDS        (0x1<<2)
#define TFA98XX_STATUSREG_OTDS_POS        2
#define TFA98XX_STATUSREG_OTDS_MAX        1
#define TFA98XX_STATUSREG_OTDS_MSK        0x4
/* OVP */
/*     0 = VddP is To High     */
/*     1 = VddP is OK     */
#define TFA98XX_STATUSREG_OVDS        (0x1<<3)
#define TFA98XX_STATUSREG_OVDS_POS        3
#define TFA98XX_STATUSREG_OVDS_MAX        1
#define TFA98XX_STATUSREG_OVDS_MSK        0x8
/* UVP */
/*     0 = VddP is To Low     */
/*     1 = VddP is OK     */
#define TFA98XX_STATUSREG_UVDS        (0x1<<4)
#define TFA98XX_STATUSREG_UVDS_POS        4
#define TFA98XX_STATUSREG_UVDS_MAX        1
#define TFA98XX_STATUSREG_UVDS_MSK        0x10
/* OCP */
/*     0 = Current is OK     */
/*     1 = Current is to High     */
#define TFA98XX_STATUSREG_OCDS        (0x1<<5)
#define TFA98XX_STATUSREG_OCDS_POS        5
#define TFA98XX_STATUSREG_OCDS_MAX        1
#define TFA98XX_STATUSREG_OCDS_MSK        0x20
/* Clocks */
/*     0 = Clock is unstable     */
/*     1 = Clock is Stable      */
#define TFA98XX_STATUSREG_CLKS        (0x1<<6)
#define TFA98XX_STATUSREG_CLKS_POS        6
#define TFA98XX_STATUSREG_CLKS_MAX        1
#define TFA98XX_STATUSREG_CLKS_MSK        0x40
/* CLIP */
/*     0 = Not clipping     */
/*     1 = Clipping     */
#define TFA98XX_STATUSREG_CLIPS       (0x1<<7)
#define TFA98XX_STATUSREG_CLIPS_POS       7
#define TFA98XX_STATUSREG_CLIPS_MAX       1
#define TFA98XX_STATUSREG_CLIPS_MSK       0x80
/* MTP  */
/*     0 = MTP Vacant     */
/*     1 = MTP Busy      */
#define TFA98XX_STATUSREG_MTPB        (0x1<<8)
#define TFA98XX_STATUSREG_MTPB_POS        8
#define TFA98XX_STATUSREG_MTPB_MAX        1
#define TFA98XX_STATUSREG_MTPB_MSK        0x100
/* BOOST */
/*     0 = Vboost not in window     */
/*     1 = Vboost in window (OK)      */
#define TFA98XX_STATUSREG_DCCS        (0x1<<9)
#define TFA98XX_STATUSREG_DCCS_POS        9
#define TFA98XX_STATUSREG_DCCS_MAX        1
#define TFA98XX_STATUSREG_DCCS_MSK        0x200
/* Speaker */
/*     0 = Speaker is OK     */
/*     1 = Speaker error     */
#define TFA98XX_STATUSREG_SPKS        (0x1<<10)
#define TFA98XX_STATUSREG_SPKS_POS        10
#define TFA98XX_STATUSREG_SPKS_MAX        1
#define TFA98XX_STATUSREG_SPKS_MSK        0x400
/* cold start flag */
/*     0 = Not a cold start, already running     */
/*     1 = Cold start (via POR)     */
#define TFA98XX_STATUSREG_ACS         (0x1<<11)
#define TFA98XX_STATUSREG_ACS_POS         11
#define TFA98XX_STATUSREG_ACS_MAX         1
#define TFA98XX_STATUSREG_ACS_MSK         0x800
/* flag engage */
/*     0 = amplifier is not switching     */
/*     1 = amplifier is switching     */
#define TFA98XX_STATUSREG_SWS         (0x1<<12)
#define TFA98XX_STATUSREG_SWS_POS         12
#define TFA98XX_STATUSREG_SWS_MAX         1
#define TFA98XX_STATUSREG_SWS_MSK         0x1000
/* flag watchdog reset */
/*     0 = no reset due to watchdog     */
/*     1 = reset due to watchdog     */
#define TFA98XX_STATUSREG_WDS         (0x1<<13)
#define TFA98XX_STATUSREG_WDS_POS         13
#define TFA98XX_STATUSREG_WDS_MAX         1
#define TFA98XX_STATUSREG_WDS_MSK         0x2000
/* amplifier is enabled by manager */
/*     0 = amplifier is not enabled     */
/*     1 = amplifier is enabled     */
#define TFA98XX_STATUSREG_AMPS        (0x1<<14)
#define TFA98XX_STATUSREG_AMPS_POS        14
#define TFA98XX_STATUSREG_AMPS_MAX        1
#define TFA98XX_STATUSREG_AMPS_MSK        0x4000
/* references are enabled by manager */
/*     0 = references are not enabled     */
/*     1 = references are enabled     */
#define TFA98XX_STATUSREG_AREFS       (0x1<<15)
#define TFA98XX_STATUSREG_AREFS_POS       15
#define TFA98XX_STATUSREG_AREFS_MAX       1
#define TFA98XX_STATUSREG_AREFS_MSK       0x8000

/**   BatteryVoltage Register ($01) **/
#define TFA98XX_BATTERYVOLTAGE      0x01
#define TFA9890_BATTERYVOLTAGE_POR  0x0000
/* Battery voltage readout; 0[V]..5.5[V] */
#define TFA98XX_BATTERYVOLTAGE_BATS   (0x3ff<<0)
#define TFA98XX_BATTERYVOLTAGE_BATS_POS   0
#define TFA98XX_BATTERYVOLTAGE_BATS_MAX   1023
#define TFA98XX_BATTERYVOLTAGE_BATS_MSK   0x3ff
/* not used */
#define TFA98XX_BATTERYVOLTAGE_10     (0x3f<<10)
#define TFA98XX_BATTERYVOLTAGE_10_POS     10
#define TFA98XX_BATTERYVOLTAGE_10_MAX     63
#define TFA98XX_BATTERYVOLTAGE_10_MSK     0xfc00

/**   Temperature Register ($02) **/
#define TFA98XX_TEMPERATURE         0x02
#define TFA9890_TEMPERATURE_POR     0x0000
/* Temperature readout */
#define TFA98XX_TEMPERATURE_TEMPS     (0x1ff<<0)
#define TFA98XX_TEMPERATURE_TEMPS_POS     0
#define TFA98XX_TEMPERATURE_TEMPS_MAX     511
#define TFA98XX_TEMPERATURE_TEMPS_MSK     0x1ff
/* not used */
#define TFA98XX_TEMPERATURE_9         (0x7f<<9)
#define TFA98XX_TEMPERATURE_9_POS         9
#define TFA98XX_TEMPERATURE_9_MAX         127
#define TFA98XX_TEMPERATURE_9_MSK         0xfe00

/**   RevisionNumber Register ($03) **/
#define TFA98XX_REVISIONNUMBER      0x03
#define TFA9890_REVISIONNUMBER_POR  0x0080
/*  */
#define TFA98XX_REVISIONNUMBER_REV    (0xff<<0)
#define TFA98XX_REVISIONNUMBER_REV_POS    0
#define TFA98XX_REVISIONNUMBER_REV_MAX    255
#define TFA98XX_REVISIONNUMBER_REV_MSK    0xff
/* not used */
#define TFA98XX_REVISIONNUMBER_8      (0xff<<8)
#define TFA98XX_REVISIONNUMBER_8_POS      8
#define TFA98XX_REVISIONNUMBER_8_MAX      255
#define TFA98XX_REVISIONNUMBER_8_MSK      0xff00

/**   I2SReg Register ($04) **/
#define TFA98XX_I2SREG              0x04
#define TFA9890_I2SREG_POR          0x888b
/* I2SFormat data 1 input: */
/*     0 = Philips standard I2S     */
/*     1 = Philips standard I2S     */
/*     2 = MSB justify     */
/*     3 = Philips standard I2S [default]     */
/*     4 = LSB Justify 16 bits     */
/*     5 = LSB Justify 18 bits     */
/*     6 = LSB Justify 20 bits     */
/*     7 = LSB Justify 24 bits     */
#define TFA98XX_I2SREG_I2SF           (0x7<<0)
#define TFA98XX_I2SREG_I2SF_POS           0
#define TFA98XX_I2SREG_I2SF_MAX           7
#define TFA98XX_I2SREG_I2SF_MSK           0x7
/* ChannelSelection data1 input  (In CoolFlux) */
/*     0 = Stereo     */
/*     1 = Left [default]     */
/*     2 = Right     */
/*     3 = Mono =(L+R)/2     */
#define TFA98XX_I2SREG_CHS12          (0x3<<3)
#define TFA98XX_I2SREG_CHS12_POS          3
#define TFA98XX_I2SREG_CHS12_MAX          3
#define TFA98XX_I2SREG_CHS12_MSK          0x18
/* ChannelSelection data 2 input (coolflux input, the DCDC converter gets the other signal) */
/*     0 = Left channel to CF DSP right channel to other vamp mux [default]     */
/*     1 = Right channel to CF DSP left channel to other vamp mux     */
#define TFA98XX_I2SREG_CHS3           (0x1<<5)
#define TFA98XX_I2SREG_CHS3_POS           5
#define TFA98XX_I2SREG_CHS3_MAX           1
#define TFA98XX_I2SREG_CHS3_MSK           0x20
/* Input selection for amplifier */
/*     0 = I2S input 1 left channel (CoolFlux bypassed)     */
/*     1 = I2S input 1 Right channel (CoolFlux bypassed)     */
/*     2 = Output Coolflux DSP [default]     */
/*     3 = Output Collflux DSP     */
#define TFA98XX_I2SREG_CHSA           (0x3<<6)
#define TFA98XX_I2SREG_CHSA_POS           6
#define TFA98XX_I2SREG_CHSA_MAX           3
#define TFA98XX_I2SREG_CHSA_MSK           0xc0
/* selection data out */
/*     0 = I2S-TX [default]     */
/*     1 = datai1     */
/*     2 = datai2     */
/*     3 = datai3     */
#define TFA98XX_I2SREG_I2SDOC         (0x3<<8)
#define TFA98XX_I2SREG_I2SDOC_POS         8
#define TFA98XX_I2SREG_I2SDOC_MAX         3
#define TFA98XX_I2SREG_I2SDOC_MSK         0x300
/* idp protection */
/*     0 = on     */
/*     1 = off     */
#define TFA98XX_I2SREG_DISP           (0x1<<10)
#define TFA98XX_I2SREG_DISP_POS           10
#define TFA98XX_I2SREG_DISP_MAX           1
#define TFA98XX_I2SREG_DISP_MSK           0x400
/* Enable data output */
/*     0 = data output in tristate     */
/*     1 = normal mode [default]     */
#define TFA98XX_I2SREG_I2SDOE         (0x1<<11)
#define TFA98XX_I2SREG_I2SDOE_POS         11
#define TFA98XX_I2SREG_I2SDOE_MAX         1
#define TFA98XX_I2SREG_I2SDOE_MSK         0x800
/* sample rate setting */
/*     0 = 8kHk     */
/*     1 =11.025kHz     */
/*     2 = 12kHz     */
/*     3 = 16kHz     */
/*     4 = 22.05kHz     */
/*     5 = 24kHz     */
/*     6 = 32kHz     */
/*     7 = 44.1kHz     */
/*     8 = 48kHz [default]     */
#define TFA98XX_I2SREG_I2SSR          (0xf<<12)
#define TFA98XX_I2SREG_I2SSR_POS          12
#define TFA98XX_I2SREG_I2SSR_MAX          15
#define TFA98XX_I2SREG_I2SSR_MSK          0xf000

/**   bat_prot Register ($05) **/
#define TFA98XX_BAT_PROT            0x05
#define TFA9890_BAT_PROT_POR        0x9392
/* ProtectionAttackTime */
/*     0 = 0.56 dB/Sample     */
/*     1 = 1.12 dB/sample     */
/*     2 = 2.32 dB/Sample [default]     */
/*     3 = infinite dB/Sample     */
#define TFA98XX_BAT_PROT_BSSCR        (0x3<<0)
#define TFA98XX_BAT_PROT_BSSCR_POS        0
#define TFA98XX_BAT_PROT_BSSCR_MAX        3
#define TFA98XX_BAT_PROT_BSSCR_MSK        0x3
/* ProtectionThreshold */
/*        normal   steep              */
/*     0 = 2.73V   2.99V     */
/*     1 = 2.83V   3.09V     */
/*     2 = 2.93V   3.19V     */
/*     3 = 3.03V   3.29V     */
/*     4 = 3.13V   3.39V (default)     */
/*     5 = 3.23V   3.49V     */
/*     6 = 3.33V   3.59V     */
/*     7 = 3.43V   3.69V     */
/*     8 = 3.53V   3.79V      */
/*     9 = 3.63V   3.89V     */
/*     10 = 3.73V   3.99V     */
/*     11 = 3.83V   4.09V     */
/*     12 = 3.93V   4.19V     */
/*     13 = 4.03V   4.29V     */
/*     14 = 4.13V   4.39V     */
/*     15 = 4.23V   4.49V     */
#define TFA98XX_BAT_PROT_BSST         (0xf<<2)
#define TFA98XX_BAT_PROT_BSST_POS         2
#define TFA98XX_BAT_PROT_BSST_MAX         15
#define TFA98XX_BAT_PROT_BSST_MSK         0x3c
/* ProtectionMaximumReduction */
/*     0 = 3V     */
/*     1 = 4V     */
/*     2 = 5V [default]     */
/*     3 = infinite     */
#define TFA98XX_BAT_PROT_BSSRL        (0x3<<6)
#define TFA98XX_BAT_PROT_BSSRL_POS        6
#define TFA98XX_BAT_PROT_BSSRL_MAX        3
#define TFA98XX_BAT_PROT_BSSRL_MSK        0xc0
/* Protection Release Timer */
/*     0 = 0.4 sec     */
/*     1 = 0.8 sec     */
/*     2 = 1.2 sec     */
/*     3 = 1.6 sec [default]     */
/*     4 = 2 sec     */
/*     5 = 2,4 sec     */
/*     6 = 2.8 sec     */
/*     7 = 3.2 sec     */
#define TFA98XX_BAT_PROT_BSSRR        (0x7<<8)
#define TFA98XX_BAT_PROT_BSSRR_POS        8
#define TFA98XX_BAT_PROT_BSSRR_MAX        7
#define TFA98XX_BAT_PROT_BSSRR_MSK        0x700
/* ProtectionHysterese */
/*     0 =no hysterese     */
/*     1 = 0.05V     */
/*     2 = 0.1V [default]     */
/*     3 = 0.2V     */
#define TFA98XX_BAT_PROT_BSSHY        (0x3<<11)
#define TFA98XX_BAT_PROT_BSSHY_POS        11
#define TFA98XX_BAT_PROT_BSSHY_MAX        3
#define TFA98XX_BAT_PROT_BSSHY_MSK        0x1800
/* reset clipper */
/*     0 = clipper is not reset if CF is bypassed [default]     */
/*     1 = reset the clipper via I2C in case the CF is bypassed     */
#define TFA98XX_BAT_PROT_BSSR         (0x1<<13)
#define TFA98XX_BAT_PROT_BSSR_POS         13
#define TFA98XX_BAT_PROT_BSSR_MAX         1
#define TFA98XX_BAT_PROT_BSSR_MSK         0x2000
/* battery voltage for I2C read out only */
/*     0 = minimum battery value [reset]     */
/*     1 = avarage battery value     */
#define TFA98XX_BAT_PROT_BSAV         (0x1<<14)
#define TFA98XX_BAT_PROT_BSAV_POS         14
#define TFA98XX_BAT_PROT_BSAV_MAX         1
#define TFA98XX_BAT_PROT_BSAV_MSK         0x4000
/* bypass clipper battery protection */
/*     0 = clipper active  [device default]     */
/*     1 = clipper bypassed  [new default]     */
#define TFA98XX_BAT_PROT_BSSBY        (0x1<<15)
#define TFA98XX_BAT_PROT_BSSBY_POS        15
#define TFA98XX_BAT_PROT_BSSBY_MAX        1
#define TFA98XX_BAT_PROT_BSSBY_MSK        0x8000

/**   audio_ctr Register ($06) **/
#define TFA98XX_AUDIO_CTR           0x06
#define TFA9890_AUDIO_CTR_POR       0x000f
/* Enable dynamic powerstage activation */
/*     0 =  dpsa off     */
/*     1 = dpsa on [default]     */
#define TFA98XX_AUDIO_CTR_DPSA        (0x1<<0)
#define TFA98XX_AUDIO_CTR_DPSA_POS        0
#define TFA98XX_AUDIO_CTR_DPSA_MAX        1
#define TFA98XX_AUDIO_CTR_DPSA_MSK        0x1
/* control slope */
/*     0 =  slope 1     */
/*     1 =  slope 1     */
/*     2 = slope 1     */
/*     3 = slope 2     */
/*     4 = slope 1     */
/*     5 = slope 2     */
/*     6 = slope 2     */
/*     7 = slope 3  [default]     */
/*     8 = slope 1     */
/*     9 = slope 2     */
/*     10 = slope 2     */
/*     11 = slope 3     */
/*     12 = slope 2     */
/*     13 = slope 3     */
/*     14 = slope 3     */
/*     15 = maximal     */
#define TFA98XX_AUDIO_CTR_AMPSL       (0xf<<1)
#define TFA98XX_AUDIO_CTR_AMPSL_POS       1
#define TFA98XX_AUDIO_CTR_AMPSL_MAX       15
#define TFA98XX_AUDIO_CTR_AMPSL_MSK       0x1e
/* Soft mute in CoolFlux */
/*     0= no mute [default]     */
/*     1= muted     */
#define TFA98XX_AUDIO_CTR_CFSM        (0x1<<5)
#define TFA98XX_AUDIO_CTR_CFSM_POS        5
#define TFA98XX_AUDIO_CTR_CFSM_MAX        1
#define TFA98XX_AUDIO_CTR_CFSM_MSK        0x20
/* Input selection for the second channel of the DCDC inteligent mode detector */
/*     0=I2S input2 [default]     */
/*     1=CoolFlux second channel     */
#define TFA98XX_AUDIO_CTR_6           (0x1<<6)
#define TFA98XX_AUDIO_CTR_6_POS           6
#define TFA98XX_AUDIO_CTR_6_MAX           1
#define TFA98XX_AUDIO_CTR_6_MSK           0x40
/* batsensesteepness */
/*     0 = 5.4V/V if ctr_supplysense = 1     */
/*     1 = 10.8V/V if ctrl_supplysense = 1     */
/*     0 = 3.13V/V if ctrl_supplysense = 0     */
/*     1 = 6.25V/V if ctrl_supplysense = 0     */
#define TFA98XX_AUDIO_CTR_BSSS        (0x1<<7)
#define TFA98XX_AUDIO_CTR_BSSS_POS        7
#define TFA98XX_AUDIO_CTR_BSSS_MAX        1
#define TFA98XX_AUDIO_CTR_BSSS_MSK        0x80
/* volume control (in CoolFlux) */
#define TFA98XX_AUDIO_CTR_VOL         (0xff<<8)
#define TFA98XX_AUDIO_CTR_VOL_POS         8
#define TFA98XX_AUDIO_CTR_VOL_MAX         255
#define TFA98XX_AUDIO_CTR_VOL_MSK         0xff00

/**   DCDCboost Register ($07) **/
#define TFA98XX_DCDCBOOST           0x07
#define TFA9890_DCDCBOOST_POR       0x8fff
/* Boost voltage */
/*     0 = 6.0 V     */
/*     1 = 6.5 V     */
/*     2 = 7.0 V     */
/*     3 = 7.5 V     */
/*     4 = 8.0 V     */
/*     5 = 8.5 V     */
/*     6 = 9.0 V      */
/*     7 = 9.5 V [default]     */
#define TFA98XX_DCDCBOOST_DCVO        (0x7<<0)
#define TFA98XX_DCDCBOOST_DCVO_POS        0
#define TFA98XX_DCDCBOOST_DCVO_MAX        7
#define TFA98XX_DCDCBOOST_DCVO_MSK        0x7
/* Max boost coil current */
/*     0 = 0.48 A     */
/*     1 = 0.96 A     */
/*     2 = 1.44 A     */
/*     3 = 1.92 A     */
/*     4 = 2.4 A     */
/*     5 = 2.88 A     */
/*     6 = 3.56 A     */
/*     7 = 3.8 A [default]     */
#define TFA98XX_DCDCBOOST_DCMCC       (0x7<<3)
#define TFA98XX_DCDCBOOST_DCMCC_POS       3
#define TFA98XX_DCDCBOOST_DCMCC_MAX       7
#define TFA98XX_DCDCBOOST_DCMCC_MSK       0x38
/* Setting for the slope of the boost converter power stage */
/*     0 =slope is 25%     */
/*     1 =slope is 50%     */
/*     2= slope is 75%     */
/*     3= slope is 100% [default]     */
#define TFA98XX_DCDCBOOST_6           (0x3<<6)
#define TFA98XX_DCDCBOOST_6_POS           6
#define TFA98XX_DCDCBOOST_6_MAX           3
#define TFA98XX_DCDCBOOST_6_MSK           0xc0
/* Setting for the part of the power transistor voltage to be used in peak current mode control */
/*     0 = scaling is 40%     */
/*     1 = scaling is 60%     */
/*     2= scaling is 80%     */
/*     3= scaling is 100% [default]     */
#define TFA98XX_DCDCBOOST_8           (0x3<<8)
#define TFA98XX_DCDCBOOST_8_POS           8
#define TFA98XX_DCDCBOOST_8_MAX           3
#define TFA98XX_DCDCBOOST_8_MSK           0x300
/* Intelligent boost mode */
/*     0 = Off     */
/*     1 = On [default]     */
#define TFA98XX_DCDCBOOST_DCIE        (0x1<<10)
#define TFA98XX_DCDCBOOST_DCIE_POS        10
#define TFA98XX_DCDCBOOST_DCIE_MAX        1
#define TFA98XX_DCDCBOOST_DCIE_MSK        0x400
/* Soft RampUp/Down mode for DCDC controller */
/*     0 =  Immediate               : 0 Cycle      */
/*     1 =  Fast      (Default)    : 32 Cycles/step at 2MHz, 16 cycles/step at 1MHz and 0.5MHz     */
#define TFA98XX_DCDCBOOST_DCSR        (0x1<<11)
#define TFA98XX_DCDCBOOST_DCSR_POS        11
#define TFA98XX_DCDCBOOST_DCSR_MAX        1
#define TFA98XX_DCDCBOOST_DCSR_MSK        0x800
/* delay compensation in current patg compared to delay in the audio path (relative)  */
/*     0 = 0 clk_c clock delay [default]      */
/*     1 = -1 clk_c clock delay     */
/*     2 = +1 clk_c clock delay     */
/*     3 = +2 clk_c clock delay     */
#define TFA98XX_DCDCBOOST_12          (0x3<<12)
#define TFA98XX_DCDCBOOST_12_POS          12
#define TFA98XX_DCDCBOOST_12_MAX          3
#define TFA98XX_DCDCBOOST_12_MSK          0x3000
/* Selection intelligent boost detector input */
/*     0 = Class-D input [default]     */
/*     1 = Max(L,R)     */
#define TFA98XX_DCDCBOOST_14          (0x1<<14)
#define TFA98XX_DCDCBOOST_14_POS          14
#define TFA98XX_DCDCBOOST_14_MAX          1
#define TFA98XX_DCDCBOOST_14_MSK          0x4000
/* battery sensor selection */
/*     1 = Vbat with range 0 - 5.5V     */
/*     0 = Vbst with range 0 - 9.5V     */
#define TFA98XX_DCDCBOOST_15          (0x1<<15)
#define TFA98XX_DCDCBOOST_15_POS          15
#define TFA98XX_DCDCBOOST_15_MAX          1
#define TFA98XX_DCDCBOOST_15_MSK          0x8000

/**   spkr_calibration Register ($08) **/
#define TFA98XX_SPKR_CALIBRATION    0x08
#define TFA9890_SPKR_CALIBRATION_POR  0x3800
/* select external temperature also the ext_temp will be put on the temp read out  */
/*     0 = internal temperature     */
/*     1 = external temperature     */
#define TFA98XX_SPKR_CALIBRATION_TROS  (0x1<<0)
#define TFA98XX_SPKR_CALIBRATION_TROS_POS  0
#define TFA98XX_SPKR_CALIBRATION_TROS_MAX  1
#define TFA98XX_SPKR_CALIBRATION_TROS_MSK  0x1
/* external temperature setting to be given by host */
#define TFA98XX_SPKR_CALIBRATION_EXTTS  (0x1ff<<1)
#define TFA98XX_SPKR_CALIBRATION_EXTTS_POS  1
#define TFA98XX_SPKR_CALIBRATION_EXTTS_MAX  511
#define TFA98XX_SPKR_CALIBRATION_EXTTS_MSK  0x3fe
/* Peak voltage protection boost converter */
/*     0 = off [default]      */
/*     1 = 0n     */
#define TFA98XX_SPKR_CALIBRATION_10   (0x1<<10)
#define TFA98XX_SPKR_CALIBRATION_10_POS   10
#define TFA98XX_SPKR_CALIBRATION_10_MAX   1
#define TFA98XX_SPKR_CALIBRATION_10_MSK   0x400
/* DCDC synchronisation off + 7 positions */
/*     0 = off + 7 positions     */
/*     1 = off     */
/*     2 = on min     */
/*     3 = on, 3     */
/*     4 = on, 4     */
/*     5 = on, 5     */
/*     6 = on, 6     */
/*     7 = on, max     */
#define TFA98XX_SPKR_CALIBRATION_11   (0x7<<11)
#define TFA98XX_SPKR_CALIBRATION_11_POS   11
#define TFA98XX_SPKR_CALIBRATION_11_MAX   7
#define TFA98XX_SPKR_CALIBRATION_11_MSK   0x3800
/* sample valid moment for CS in single sample moment mode */
/*     0= used in case cs is continiously sampling     */
/*     1= used in case cs only takes one sample per PWM period     */
#define TFA98XX_SPKR_CALIBRATION_14   (0x1<<14)
#define TFA98XX_SPKR_CALIBRATION_14_POS   14
#define TFA98XX_SPKR_CALIBRATION_14_MAX   1
#define TFA98XX_SPKR_CALIBRATION_14_MSK   0x4000
/* not used */
#define TFA98XX_SPKR_CALIBRATION_15   (0x1<<15)
#define TFA98XX_SPKR_CALIBRATION_15_POS   15
#define TFA98XX_SPKR_CALIBRATION_15_MAX   1
#define TFA98XX_SPKR_CALIBRATION_15_MSK   0x8000

/**   sys_ctrl Register ($09) **/
#define TFA98XX_SYS_CTRL            0x09
#define TFA9890_SYS_CTRL_POR        0x824d
/* ON/OFF */
/*     0 = Operating     */
/*     1 = Powerdown [default]     */
#define TFA98XX_SYS_CTRL_PWDN         (0x1<<0)
#define TFA98XX_SYS_CTRL_PWDN_POS         0
#define TFA98XX_SYS_CTRL_PWDN_MAX         1
#define TFA98XX_SYS_CTRL_PWDN_MSK         0x1
/* I2CReset */
/*     0 = Normal operation [default]     */
/*     1 = Reset all register to default     */
#define TFA98XX_SYS_CTRL_I2CR         (0x1<<1)
#define TFA98XX_SYS_CTRL_I2CR_POS         1
#define TFA98XX_SYS_CTRL_I2CR_MAX         1
#define TFA98XX_SYS_CTRL_I2CR_MSK         0x2
/* EnableCoolFlux */
/*     0 = Coolflux OFF     */
/*     1 = Coolflux ON [default]     */
#define TFA98XX_SYS_CTRL_CFE          (0x1<<2)
#define TFA98XX_SYS_CTRL_CFE_POS          2
#define TFA98XX_SYS_CTRL_CFE_MAX          1
#define TFA98XX_SYS_CTRL_CFE_MSK          0x4
/* EnableAmplifier */
/*     0 = Amplifier OFF     */
/*     1 = Amplifier ON [default]     */
#define TFA98XX_SYS_CTRL_AMPE         (0x1<<3)
#define TFA98XX_SYS_CTRL_AMPE_POS         3
#define TFA98XX_SYS_CTRL_AMPE_MAX         1
#define TFA98XX_SYS_CTRL_AMPE_MSK         0x8
/* EnableBoost */
/*     0 = Boost OFF (Follower mode)     */
/*     1 = Boost ON [default]     */
#define TFA98XX_SYS_CTRL_DCA          (0x1<<4)
#define TFA98XX_SYS_CTRL_DCA_POS          4
#define TFA98XX_SYS_CTRL_DCA_MAX          1
#define TFA98XX_SYS_CTRL_DCA_MSK          0x10
/* Coolflux configured */
/*     0 = coolflux not configured [default]     */
/*     1 = coolflux configured     */
#define TFA98XX_SYS_CTRL_SBSL         (0x1<<5)
#define TFA98XX_SYS_CTRL_SBSL_POS         5
#define TFA98XX_SYS_CTRL_SBSL_MAX         1
#define TFA98XX_SYS_CTRL_SBSL_MSK         0x20
/* Selection on how AmplifierEnabling */
/*     0 = Enable amplifier independent of CoolFlux [default]     */
/*     1 = CoolFlux enables amplifier (SW_Bit: cf_enabl_amplifier)      */
#define TFA98XX_SYS_CTRL_AMPC         (0x1<<6)
#define TFA98XX_SYS_CTRL_AMPC_POS         6
#define TFA98XX_SYS_CTRL_AMPC_MAX         1
#define TFA98XX_SYS_CTRL_AMPC_MSK         0x40
/* DCDC switch off */
/*     0 = normal DCDC functionality [default]     */
/*     1 = DCDC switched off     */
#define TFA98XX_SYS_CTRL_DCDIS        (0x1<<7)
#define TFA98XX_SYS_CTRL_DCDIS_POS        7
#define TFA98XX_SYS_CTRL_DCDIS_MAX        1
#define TFA98XX_SYS_CTRL_DCDIS_MSK        0x80
/* Iddq test amplifier */
/*     0 = amplifier is normal mode [default]     */
/*     1 = amplifier is in the test mode [new default]     */
#define TFA98XX_SYS_CTRL_PSDR         (0x1<<8)
#define TFA98XX_SYS_CTRL_PSDR_POS         8
#define TFA98XX_SYS_CTRL_PSDR_MAX         1
#define TFA98XX_SYS_CTRL_PSDR_MSK         0x100
/* Coil Value */
/*     0 = 0.7 uH     */
/*     1 = 1.0 uH  [new default]     */
/*     2 = 1.5 uH  [device default]     */
/*     3 = 2.2 uH     */
#define TFA98XX_SYS_CTRL_DCCV         (0x3<<9)
#define TFA98XX_SYS_CTRL_DCCV_POS         9
#define TFA98XX_SYS_CTRL_DCCV_MAX         3
#define TFA98XX_SYS_CTRL_DCCV_MSK         0x600
/* Selection CoolFluxClock */
/*     0 = clk_d (100MHz at 48kHz input) [default]     */
/*     1 = clk_d/2 (50MHz at 48kHz input)     */
/*     2 = clk_d*3/4 (75MHz at 48kHz input)     */
/*     3 = clk_d (100MHz at 48kHz input)     */
#define TFA98XX_SYS_CTRL_CCFD         (0x3<<11)
#define TFA98XX_SYS_CTRL_CCFD_POS         11
#define TFA98XX_SYS_CTRL_CCFD_MAX         3
#define TFA98XX_SYS_CTRL_CCFD_MSK         0x1800
/* selection input 1 or 2 */
/*     0 = input 1 [default]     */
/*     1 = input 2     */
#define TFA98XX_SYS_CTRL_ISEL         (0x1<<13)
#define TFA98XX_SYS_CTRL_ISEL_POS         13
#define TFA98XX_SYS_CTRL_ISEL_MAX         1
#define TFA98XX_SYS_CTRL_ISEL_MSK         0x2000
/* selection input PLL for lock */
/*     0 = BCK [default]     */
/*     1 = WS     */
#define TFA98XX_SYS_CTRL_IPLL         (0x1<<14)
#define TFA98XX_SYS_CTRL_IPLL_POS         14
#define TFA98XX_SYS_CTRL_IPLL_MAX         1
#define TFA98XX_SYS_CTRL_IPLL_MSK         0x4000
/* not used */
#define TFA98XX_SYS_CTRL_15           (0x1<<15)
#define TFA98XX_SYS_CTRL_15_POS           15
#define TFA98XX_SYS_CTRL_15_MAX           1
#define TFA98XX_SYS_CTRL_15_MSK           0x8000

/**   I2S_sel_reg Register ($0a) **/
#define TFA98XX_I2S_SEL_REG         0x0a
#define TFA9890_I2S_SEL_REG_POR     0x3ec3
/* Output selection dataout left channel */
/*     0=CurrentSense signal     */
/*     1=Coolflux output 3 (e.g. gain)     */
/*     2=Coolflux output 2 (second channel)     */
/*     3=Coolflux output 1 (main channel) [default]     */
/*     4=datai3 left     */
/*     5=datai3 right     */
/*     6= dcdc feedforward audio current     */
#define TFA98XX_I2S_SEL_REG_DOLS      (0x7<<0)
#define TFA98XX_I2S_SEL_REG_DOLS_POS      0
#define TFA98XX_I2S_SEL_REG_DOLS_MAX      7
#define TFA98XX_I2S_SEL_REG_DOLS_MSK      0x7
/* Output selection dataout right channel */
/*     0=CurrentSense signal [default]     */
/*     1=Coolflux output 3 (e.g. gain)     */
/*     2=Coolflux output 2 (second channel)     */
/*     3=Coolflux output 1 (main channel)      */
/*     4=datai3 left     */
/*     5=datai3 right     */
/*     6= dcdc feedforward audio current     */
#define TFA98XX_I2S_SEL_REG_DORS      (0x7<<3)
#define TFA98XX_I2S_SEL_REG_DORS_POS      3
#define TFA98XX_I2S_SEL_REG_DORS_MAX      7
#define TFA98XX_I2S_SEL_REG_DORS_MSK      0x38
/* Selection speaker induction */
/*     0 = 22 uH     */
/*     1 = 27 uH     */
/*     2 = 33 uH     */
/*     3 = 39 uH [default]     */
/*     4 = 47 uH     */
/*     5 = 56 uH     */
/*     6 = 68 uH     */
/*     7 = 82 uH     */
#define TFA98XX_I2S_SEL_REG_SPKL      (0x7<<6)
#define TFA98XX_I2S_SEL_REG_SPKL_POS      6
#define TFA98XX_I2S_SEL_REG_SPKL_MAX      7
#define TFA98XX_I2S_SEL_REG_SPKL_MSK      0x1c0
/* Selection speaker impedance */
/*     0 = defined by DSP     */
/*     1 = 4 ohm     */
/*     2 = 6 ohm     */
/*     3 = 8 ohm [default]     */
#define TFA98XX_I2S_SEL_REG_SPKR      (0x3<<9)
#define TFA98XX_I2S_SEL_REG_SPKR_POS      9
#define TFA98XX_I2S_SEL_REG_SPKR_MAX      3
#define TFA98XX_I2S_SEL_REG_SPKR_MSK      0x600
/* DCDC speaker current compensation gain */
/*     0 = Off [new default]     */
/*     1 = 70%     */
/*     2 = 75%     */
/*     3 = 80%     */
/*     4 = 85%     */
/*     5 = 90%     */
/*     6 = 95%     */
/*     7 = 100% [device default]     */
/*     8 = 105%     */
/*     9 = 110%     */
/*     10 = 115%     */
/*     11 = 120%     */
/*     12 = 125%     */
/*     13 = 130%     */
/*     14 = 135%     */
/*     15 = 140%     */
#define TFA98XX_I2S_SEL_REG_DCFG      (0xf<<11)
#define TFA98XX_I2S_SEL_REG_DCFG_POS      11
#define TFA98XX_I2S_SEL_REG_DCFG_MAX      15
#define TFA98XX_I2S_SEL_REG_DCFG_MSK      0x7800
/* DCDC speaker current compensation sign */
/*     0 = positive [default]     */
/*     1 = negative     */
#define TFA98XX_I2S_SEL_REG_15        (0x1<<15)
#define TFA98XX_I2S_SEL_REG_15_POS        15
#define TFA98XX_I2S_SEL_REG_15_MAX        1
#define TFA98XX_I2S_SEL_REG_15_MSK        0x8000

/**   cgu_clock_sync_reg Register ($0c) **/
#define TFA98XX_CGU_CLOCK_SYNC_REG  0x0c
#define TFA9890_CGU_CLOCK_SYNC_REG_POR  0x8000
/* Delay count for clock synchronisation */
#define TFA98XX_CGU_CLOCK_SYNC_REG_0  (0x1fff<<0)
#define TFA98XX_CGU_CLOCK_SYNC_REG_0_POS  0
#define TFA98XX_CGU_CLOCK_SYNC_REG_0_MAX  8191
#define TFA98XX_CGU_CLOCK_SYNC_REG_0_MSK  0x1fff
/* not used */
#define TFA98XX_CGU_CLOCK_SYNC_REG_13  (0x3<<13)
#define TFA98XX_CGU_CLOCK_SYNC_REG_13_POS  13
#define TFA98XX_CGU_CLOCK_SYNC_REG_13_MAX  3
#define TFA98XX_CGU_CLOCK_SYNC_REG_13_MSK  0x6000
/* Enable CGU clock synchronisation */
#define TFA98XX_CGU_CLOCK_SYNC_REG_15  (0x1<<15)
#define TFA98XX_CGU_CLOCK_SYNC_REG_15_POS  15
#define TFA98XX_CGU_CLOCK_SYNC_REG_15_MAX  1
#define TFA98XX_CGU_CLOCK_SYNC_REG_15_MSK  0x8000

/**   adc_sync_reg Register ($0d) **/
#define TFA98XX_ADC_SYNC_REG        0x0d
#define TFA9890_ADC_SYNC_REG_POR    0x8000
/* Delay count for ADC synchronisation */
#define TFA98XX_ADC_SYNC_REG_0        (0x1fff<<0)
#define TFA98XX_ADC_SYNC_REG_0_POS        0
#define TFA98XX_ADC_SYNC_REG_0_MAX        8191
#define TFA98XX_ADC_SYNC_REG_0_MSK        0x1fff
/* not used */
#define TFA98XX_ADC_SYNC_REG_13       (0x3<<13)
#define TFA98XX_ADC_SYNC_REG_13_POS       13
#define TFA98XX_ADC_SYNC_REG_13_MAX       3
#define TFA98XX_ADC_SYNC_REG_13_MSK       0x6000
/* Enable ADC synchronisation */
#define TFA98XX_ADC_SYNC_REG_15       (0x1<<15)
#define TFA98XX_ADC_SYNC_REG_15_POS       15
#define TFA98XX_ADC_SYNC_REG_15_MAX       1
#define TFA98XX_ADC_SYNC_REG_15_MSK       0x8000

/**   reserved_1 Register ($0e) **/
#define TFA98XX_RESERVED_1          0x0e
#define TFA9890_RESERVED_1_POR      0x0f01
/* to switch off dcdc reduction with bat prot  */
#define TFA98XX_RESERVED_1_0          (0x1<<0)
#define TFA98XX_RESERVED_1_0_POS          0
#define TFA98XX_RESERVED_1_0_MAX          1
#define TFA98XX_RESERVED_1_0_MSK          0x1
/* test option for frinch caps */
#define TFA98XX_RESERVED_1_1          (0x1<<1)
#define TFA98XX_RESERVED_1_1_POS          1
#define TFA98XX_RESERVED_1_1_MAX          1
#define TFA98XX_RESERVED_1_1_MSK          0x2
/* for extra connections digital to analog  */
#define TFA98XX_RESERVED_1_2          (0x1f<<2)
#define TFA98XX_RESERVED_1_2_POS          2
#define TFA98XX_RESERVED_1_2_MAX          31
#define TFA98XX_RESERVED_1_2_MSK          0x7c
/* icomp dem switch */
#define TFA98XX_RESERVED_1_7          (0x1<<7)
#define TFA98XX_RESERVED_1_7_POS          7
#define TFA98XX_RESERVED_1_7_MAX          1
#define TFA98XX_RESERVED_1_7_MSK          0x80
/*  */
#define TFA98XX_RESERVED_1_8          (0xff<<8)
#define TFA98XX_RESERVED_1_8_POS          8
#define TFA98XX_RESERVED_1_8_MAX          255
#define TFA98XX_RESERVED_1_8_MSK          0xff00

/**   interrupt_reg Register ($0f) **/
#define TFA98XX_INTERRUPT_REG       0x0f
#define TFA9890_INTERRUPT_REG_POR   0x0040
/* mask flag_por for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_VDDD    (0x1<<0)
#define TFA98XX_INTERRUPT_REG_VDDD_POS    0
#define TFA98XX_INTERRUPT_REG_VDDD_MAX    1
#define TFA98XX_INTERRUPT_REG_VDDD_MSK    0x1
/* mask flag_otpok for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_OTDD    (0x1<<1)
#define TFA98XX_INTERRUPT_REG_OTDD_POS    1
#define TFA98XX_INTERRUPT_REG_OTDD_MAX    1
#define TFA98XX_INTERRUPT_REG_OTDD_MSK    0x2
/* mask flag_ovpok for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_OVDD    (0x1<<2)
#define TFA98XX_INTERRUPT_REG_OVDD_POS    2
#define TFA98XX_INTERRUPT_REG_OVDD_MAX    1
#define TFA98XX_INTERRUPT_REG_OVDD_MSK    0x4
/* mask flag_uvpok for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_UVDD    (0x1<<3)
#define TFA98XX_INTERRUPT_REG_UVDD_POS    3
#define TFA98XX_INTERRUPT_REG_UVDD_MAX    1
#define TFA98XX_INTERRUPT_REG_UVDD_MSK    0x8
/* mask flag_ocp_alarm for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_OCDD    (0x1<<4)
#define TFA98XX_INTERRUPT_REG_OCDD_POS    4
#define TFA98XX_INTERRUPT_REG_OCDD_MAX    1
#define TFA98XX_INTERRUPT_REG_OCDD_MSK    0x10
/* mask flag_clocks_stable for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_CLKD    (0x1<<5)
#define TFA98XX_INTERRUPT_REG_CLKD_POS    5
#define TFA98XX_INTERRUPT_REG_CLKD_MAX    1
#define TFA98XX_INTERRUPT_REG_CLKD_MSK    0x20
/* mask flag_pwrokbst for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_DCCD    (0x1<<6)
#define TFA98XX_INTERRUPT_REG_DCCD_POS    6
#define TFA98XX_INTERRUPT_REG_DCCD_MAX    1
#define TFA98XX_INTERRUPT_REG_DCCD_MSK    0x40
/* mask flag_cf_speakererror for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_SPKD    (0x1<<7)
#define TFA98XX_INTERRUPT_REG_SPKD_POS    7
#define TFA98XX_INTERRUPT_REG_SPKD_MAX    1
#define TFA98XX_INTERRUPT_REG_SPKD_MSK    0x80
/* mask flag_watchdog_reset for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_WDD     (0x1<<8)
#define TFA98XX_INTERRUPT_REG_WDD_POS     8
#define TFA98XX_INTERRUPT_REG_WDD_MAX     1
#define TFA98XX_INTERRUPT_REG_WDD_MSK     0x100
/* mask flag_lost_clk for interupt generation */
/*     0 = enable interrupt     */
/*     1 = mask interrupt     */
#define TFA98XX_INTERRUPT_REG_9       (0x1<<9)
#define TFA98XX_INTERRUPT_REG_9_POS       9
#define TFA98XX_INTERRUPT_REG_9_MAX       1
#define TFA98XX_INTERRUPT_REG_9_MSK       0x200
/* Reverse */
#define TFA98XX_INTERRUPT_REG_10      (0xf<<10)
#define TFA98XX_INTERRUPT_REG_10_POS      10
#define TFA98XX_INTERRUPT_REG_10_MAX      15
#define TFA98XX_INTERRUPT_REG_10_MSK      0x3c00
/* enabling interrupt */
/*     0 = interrupt disabled     */
/*     1 = interrupt enabled     */
#define TFA98XX_INTERRUPT_REG_INT     (0x1<<14)
#define TFA98XX_INTERRUPT_REG_INT_POS     14
#define TFA98XX_INTERRUPT_REG_INT_MAX     1
#define TFA98XX_INTERRUPT_REG_INT_MSK     0x4000
/* Setting polarity interupt */
/*     0 = interrupt active low     */
/*     1 = interrupt active high     */
#define TFA98XX_INTERRUPT_REG_INTP    (0x1<<15)
#define TFA98XX_INTERRUPT_REG_INTP_POS    15
#define TFA98XX_INTERRUPT_REG_INTP_MAX    1
#define TFA98XX_INTERRUPT_REG_INTP_MSK    0x8000

/**   CurrentSense1 Register ($46) **/
#define TFA98XX_CURRENTSENSE1       0x46
#define TFA9890_CURRENTSENSE1_POR   0x4000
/* bypass_gc, bypasses the CS gain correction */
#define TFA98XX_CURRENTSENSE1_0       (0x1<<0)
#define TFA98XX_CURRENTSENSE1_0_POS       0
#define TFA98XX_CURRENTSENSE1_0_MAX       1
#define TFA98XX_CURRENTSENSE1_0_MSK       0x1
/* gain control by means of MTP or i2c; 0 = MTP */
#define TFA98XX_CURRENTSENSE1_1       (0x1<<1)
#define TFA98XX_CURRENTSENSE1_1_POS       1
#define TFA98XX_CURRENTSENSE1_1_MAX       1
#define TFA98XX_CURRENTSENSE1_1_MSK       0x2
/* + / - 128 steps in steps of 1/4 %  2's compliment */
#define TFA98XX_CURRENTSENSE1_2       (0xff<<2)
#define TFA98XX_CURRENTSENSE1_2_POS       2
#define TFA98XX_CURRENTSENSE1_2_MAX       255
#define TFA98XX_CURRENTSENSE1_2_MSK       0x3fc
/* bypass Low-Pass filter in temperature sensor */
#define TFA98XX_CURRENTSENSE1_10      (0x1<<10)
#define TFA98XX_CURRENTSENSE1_10_POS      10
#define TFA98XX_CURRENTSENSE1_10_MAX      1
#define TFA98XX_CURRENTSENSE1_10_MSK      0x400
/* bypass_pwmcounter */
#define TFA98XX_CURRENTSENSE1_11      (0x1<<11)
#define TFA98XX_CURRENTSENSE1_11_POS      11
#define TFA98XX_CURRENTSENSE1_11_MAX      1
#define TFA98XX_CURRENTSENSE1_11_MSK      0x800
/* does not switch to neg */
#define TFA98XX_CURRENTSENSE1_12      (0x1<<12)
#define TFA98XX_CURRENTSENSE1_12_POS      12
#define TFA98XX_CURRENTSENSE1_12_MAX      1
#define TFA98XX_CURRENTSENSE1_12_MSK      0x1000
/* switches to neg depending on level */
#define TFA98XX_CURRENTSENSE1_13      (0x7<<13)
#define TFA98XX_CURRENTSENSE1_13_POS      13
#define TFA98XX_CURRENTSENSE1_13_MAX      7
#define TFA98XX_CURRENTSENSE1_13_MSK      0xe000

/**   CurrentSense2 Register ($47) **/
#define TFA98XX_CURRENTSENSE2       0x47
#define TFA9890_CURRENTSENSE2_POR   0x5be1
/* switch_fb */
#define TFA98XX_CURRENTSENSE2_0       (0x1<<0)
#define TFA98XX_CURRENTSENSE2_0_POS       0
#define TFA98XX_CURRENTSENSE2_0_MAX       1
#define TFA98XX_CURRENTSENSE2_0_MSK       0x1
/* se_hyst */
#define TFA98XX_CURRENTSENSE2_1       (0xf<<1)
#define TFA98XX_CURRENTSENSE2_1_POS       1
#define TFA98XX_CURRENTSENSE2_1_MAX       15
#define TFA98XX_CURRENTSENSE2_1_MSK       0x1e
/* se_level */
#define TFA98XX_CURRENTSENSE2_5       (0x1f<<5)
#define TFA98XX_CURRENTSENSE2_5_POS       5
#define TFA98XX_CURRENTSENSE2_5_MAX       31
#define TFA98XX_CURRENTSENSE2_5_MSK       0x3e0
/* temperature compensation trimming */
#define TFA98XX_CURRENTSENSE2_10      (0x3f<<10)
#define TFA98XX_CURRENTSENSE2_10_POS      10
#define TFA98XX_CURRENTSENSE2_10_MAX      63
#define TFA98XX_CURRENTSENSE2_10_MSK      0xfc00

/**   CurrentSense3 Register ($48) **/
#define TFA98XX_CURRENTSENSE3       0x48
#define TFA9890_CURRENTSENSE3_POR   0x0340
/* negin */
#define TFA98XX_CURRENTSENSE3_0       (0x1<<0)
#define TFA98XX_CURRENTSENSE3_0_POS       0
#define TFA98XX_CURRENTSENSE3_0_MAX       1
#define TFA98XX_CURRENTSENSE3_0_MSK       0x1
/* cs_sein */
#define TFA98XX_CURRENTSENSE3_1       (0x1<<1)
#define TFA98XX_CURRENTSENSE3_1_POS       1
#define TFA98XX_CURRENTSENSE3_1_MAX       1
#define TFA98XX_CURRENTSENSE3_1_MSK       0x2
/* Coincidence current sense */
/*     0 = Allow switch of dcdc during clk_cs_clksh  [new default]     */
/*     1 = Prevent dcdc switching during clk_cs_clksh [device default]     */
#define TFA98XX_CURRENTSENSE3_2       (0x1<<2)
#define TFA98XX_CURRENTSENSE3_2_POS       2
#define TFA98XX_CURRENTSENSE3_2_MAX       1
#define TFA98XX_CURRENTSENSE3_2_MSK       0x4
/* for iddq testing in powerstage of boost convertor */
#define TFA98XX_CURRENTSENSE3_3       (0x1<<3)
#define TFA98XX_CURRENTSENSE3_3_POS       3
#define TFA98XX_CURRENTSENSE3_3_MAX       1
#define TFA98XX_CURRENTSENSE3_3_MSK       0x8
/* Switch protection on to prevent simultaniously switching power stages bst and amp */
#define TFA98XX_CURRENTSENSE3_4       (0x1<<4)
#define TFA98XX_CURRENTSENSE3_4_POS       4
#define TFA98XX_CURRENTSENSE3_4_MAX       1
#define TFA98XX_CURRENTSENSE3_4_MSK       0x10
/* 0 = clock_sh = 8 * Fs */
/*     1 = clock_sh = 512 Fs     */
/*     2 = clock_sh = 1024 Fs     */
/*     3 = clock_sh = 2048 Fs     */
#define TFA98XX_CURRENTSENSE3_5       (0x3<<5)
#define TFA98XX_CURRENTSENSE3_5_POS       5
#define TFA98XX_CURRENTSENSE3_5_MAX       3
#define TFA98XX_CURRENTSENSE3_5_MSK       0x60
/* delay of se and neg */
#define TFA98XX_CURRENTSENSE3_7       (0x7f<<7)
#define TFA98XX_CURRENTSENSE3_7_POS       7
#define TFA98XX_CURRENTSENSE3_7_MAX       127
#define TFA98XX_CURRENTSENSE3_7_MSK       0x3f80
/* sample & hold track time: */
/*     00 = 2 clock cycles; 20.3[ns] [default]     */
/*     01 = 4 clock cycles; 40.7[ns]     */
/*     10 = 8 clock cycles; 81.4[ns]     */
/*     11 = is no fixed time, but as N1B     */
#define TFA98XX_CURRENTSENSE3_14      (0x3<<14)
#define TFA98XX_CURRENTSENSE3_14_POS      14
#define TFA98XX_CURRENTSENSE3_14_MAX      3
#define TFA98XX_CURRENTSENSE3_14_MSK      0xc000

/**   CurrentSense4 Register ($49) **/
#define TFA98XX_CURRENTSENSE4       0x49
#define TFA9890_CURRENTSENSE4_POR   0xad93
/* Bypass clip control (function depending on digimux clip_x) */
/*     0 = clip control old      */
/*     1 = clip control bypassed [default]     */
#define TFA98XX_CURRENTSENSE4_0       (0x1<<0)
#define TFA98XX_CURRENTSENSE4_0_POS       0
#define TFA98XX_CURRENTSENSE4_0_MAX       1
#define TFA98XX_CURRENTSENSE4_0_MSK       0x1
/* Bypass clip control (function depending on digimux clip_x) */
/*     0 = clip control test 1     */
/*     1 = clip control test 2 [default]     */
#define TFA98XX_CURRENTSENSE4_1       (0x1<<1)
#define TFA98XX_CURRENTSENSE4_1_POS       1
#define TFA98XX_CURRENTSENSE4_1_MAX       1
#define TFA98XX_CURRENTSENSE4_1_MSK       0x2
/* to disable clock gating in the coolflux */
#define TFA98XX_CURRENTSENSE4_2       (0x1<<2)
#define TFA98XX_CURRENTSENSE4_2_POS       2
#define TFA98XX_CURRENTSENSE4_2_MAX       1
#define TFA98XX_CURRENTSENSE4_2_MSK       0x4
/* Only to be used for the TFA9889/N1B4 with analog boost control loop. Several control and enable signals are now available that were previously only available in direct mode (ticket SC23297): */
/*     ctrl_peakcur<3:0> can be used to set the peak current limit in 16 steps to 4.5A max.      */
/*     ctrl_peakcur<5:4> are not used yet      */
/*     enbl_hi_peak are not used yet     */
/*     enbl_hi_large are not used yet     */
/*     enbl_hi_small are not used yet     */
/*     enbl_lo_small are not used yet     */
/*     enbl_lo_large are not used yet     */
#define TFA98XX_CURRENTSENSE4_3       (0x1<<3)
#define TFA98XX_CURRENTSENSE4_3_POS       3
#define TFA98XX_CURRENTSENSE4_3_MAX       1
#define TFA98XX_CURRENTSENSE4_3_MSK       0x8
/* clock switch for battery protection clipper, it switches back to old frequency */
#define TFA98XX_CURRENTSENSE4_4       (0x1<<4)
#define TFA98XX_CURRENTSENSE4_4_POS       4
#define TFA98XX_CURRENTSENSE4_4_MAX       1
#define TFA98XX_CURRENTSENSE4_4_MSK       0x10
/* 8 ohm mode for current sense (gain mode) */
/*     0 = 4 ohm (default)     */
/*     1 = 8 ohm     */
#define TFA98XX_CURRENTSENSE4_5       (0x1<<5)
#define TFA98XX_CURRENTSENSE4_5_POS       5
#define TFA98XX_CURRENTSENSE4_5_MAX       1
#define TFA98XX_CURRENTSENSE4_5_MSK       0x20
/*  */
#define TFA98XX_CURRENTSENSE4_6       (0x1<<6)
#define TFA98XX_CURRENTSENSE4_6_POS       6
#define TFA98XX_CURRENTSENSE4_6_MAX       1
#define TFA98XX_CURRENTSENSE4_6_MSK       0x40
/* delay_sh, tunes S7H delay */
#define TFA98XX_CURRENTSENSE4_7       (0x1f<<7)
#define TFA98XX_CURRENTSENSE4_7_POS       7
#define TFA98XX_CURRENTSENSE4_7_MAX       31
#define TFA98XX_CURRENTSENSE4_7_MSK       0xf80
/* Invert the sample/hold clock for current sense ADC */
#define TFA98XX_CURRENTSENSE4_12      (0x1<<12)
#define TFA98XX_CURRENTSENSE4_12_POS      12
#define TFA98XX_CURRENTSENSE4_12_MAX      1
#define TFA98XX_CURRENTSENSE4_12_MSK      0x1000
/* Invert neg signal */
#define TFA98XX_CURRENTSENSE4_13      (0x1<<13)
#define TFA98XX_CURRENTSENSE4_13_POS      13
#define TFA98XX_CURRENTSENSE4_13_MAX      1
#define TFA98XX_CURRENTSENSE4_13_MSK      0x2000
/* Invert se signal */
#define TFA98XX_CURRENTSENSE4_14      (0x1<<14)
#define TFA98XX_CURRENTSENSE4_14_POS      14
#define TFA98XX_CURRENTSENSE4_14_MAX      1
#define TFA98XX_CURRENTSENSE4_14_MSK      0x4000
/* switches between Single Ende and differentail mode; 1 = single ended */
#define TFA98XX_CURRENTSENSE4_15      (0x1<<15)
#define TFA98XX_CURRENTSENSE4_15_POS      15
#define TFA98XX_CURRENTSENSE4_15_MAX      1
#define TFA98XX_CURRENTSENSE4_15_MSK      0x8000

/**   abisttest Register ($4c) **/
#define TFA98XX_ABISTTEST           0x4c
#define TFA9890_ABISTTEST_POR       0x0000

/**   mtp_ctrl_reg3 Register ($62) **/
#define TFA98XX_MTP_CTRL_REG3       0x62
#define TFA9890_MTP_CTRL_REG3_POR   0x8000
/* start copying all the data from i2cregs_mtp to mtp [Key 2 protected] */
#define TFA98XX_MTP_CTRL_REG3_CIMTP   (0x1<<11)
#define TFA98XX_MTP_CTRL_REG3_CIMTP_POS   11
#define TFA98XX_MTP_CTRL_REG3_CIMTP_MAX   1
#define TFA98XX_MTP_CTRL_REG3_CIMTP_MSK   0x800

/**   cf_controls Register ($70) **/
#define TFA98XX_CF_CONTROLS         0x70
#define TFA9890_CF_CONTROLS_POR     0x0000
/* Reset CoolFlux DSP */
/*     0 = Reset not active [default]     */
/*     1 = Reset active     */
#define TFA98XX_CF_CONTROLS_RST       (0x1<<0)
#define TFA98XX_CF_CONTROLS_RST_POS       0
#define TFA98XX_CF_CONTROLS_RST_MAX       1
#define TFA98XX_CF_CONTROLS_RST_MSK       0x1
/* Target memory for access */
/*     0 =  pmem [default]     */
/*     1 =  xmem     */
/*     2 =  ymem     */
/*     3 =  iomem     */
#define TFA98XX_CF_CONTROLS_DMEM      (0x3<<1)
#define TFA98XX_CF_CONTROLS_DMEM_POS      1
#define TFA98XX_CF_CONTROLS_DMEM_MAX      3
#define TFA98XX_CF_CONTROLS_DMEM_MSK      0x6
/* Autoincrement-flag for memory-address */
/*     0 = Autoincrement ON [default]     */
/*     1 = Autoincrement OFF     */
#define TFA98XX_CF_CONTROLS_AIF       (0x1<<3)
#define TFA98XX_CF_CONTROLS_AIF_POS       3
#define TFA98XX_CF_CONTROLS_AIF_MAX       1
#define TFA98XX_CF_CONTROLS_AIF_MSK       0x8
/* Interrupt CoolFlux DSP */
#define TFA98XX_CF_CONTROLS_CFINT     (0x1<<4)
#define TFA98XX_CF_CONTROLS_CFINT_POS     4
#define TFA98XX_CF_CONTROLS_CFINT_MAX     1
#define TFA98XX_CF_CONTROLS_CFINT_MSK     0x10
/* not used */
#define TFA98XX_CF_CONTROLS_5         (0x7<<5)
#define TFA98XX_CF_CONTROLS_5_POS         5
#define TFA98XX_CF_CONTROLS_5_MAX         7
#define TFA98XX_CF_CONTROLS_5_MSK         0xe0
/* request for access (8 channels) */
#define TFA98XX_CF_CONTROLS_REQ       (0xff<<8)
#define TFA98XX_CF_CONTROLS_REQ_POS       8
#define TFA98XX_CF_CONTROLS_REQ_MAX       255
#define TFA98XX_CF_CONTROLS_REQ_MSK       0xff00

/**   cf_mad Register ($71) **/
#define TFA98XX_CF_MAD              0x71
#define TFA9890_CF_MAD_POR          0x0000
/* memory-address to be accessed */
#define TFA98XX_CF_MAD_MADD           (0xffff<<0)
#define TFA98XX_CF_MAD_MADD_POS           0
#define TFA98XX_CF_MAD_MADD_MAX           65535
#define TFA98XX_CF_MAD_MADD_MSK           0xffff

/**   cf_mem Register ($72) **/
#define TFA98XX_CF_MEM              0x72
#define TFA9890_CF_MEM_POR          0x0000
/* activate memory access (24- or 32-bits data is written/read to/from memory */
#define TFA98XX_CF_MEM_MEMA           (0xffff<<0)
#define TFA98XX_CF_MEM_MEMA_POS           0
#define TFA98XX_CF_MEM_MEMA_MAX           65535
#define TFA98XX_CF_MEM_MEMA_MSK           0xffff

/**   cf_status Register ($73) **/
#define TFA98XX_CF_STATUS           0x73
#define TFA9890_CF_STATUS_POR       0x00ff
/* cf error Flags */
#define TFA98XX_CF_STATUS_ERR         (0xff<<0)
#define TFA98XX_CF_STATUS_ERR_POS         0
#define TFA98XX_CF_STATUS_ERR_MAX         255
#define TFA98XX_CF_STATUS_ERR_MSK         0xff
/* acknowledge of requests (8 channels")" */
#define TFA98XX_CF_STATUS_ACK         (0xff<<8)
#define TFA98XX_CF_STATUS_ACK_POS         8
#define TFA98XX_CF_STATUS_ACK_MAX         255
#define TFA98XX_CF_STATUS_ACK_MSK         0xff00

/**   Key2Protected_spkr_cal_mtp Register ($80) **/
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP   0x80
#define TFA9890_KEY2PROTECTED_SPKR_CAL_MTP_POR  0x0000
/* Calibration schedule (key2 protected) */
/*     0 = Calibrate after each POR [default]     */
/*     1 = One time calibration      */
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPOTC  (0x1<<0)
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPOTC_POS  0
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPOTC_MAX  1
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPOTC_MSK  0x1
/* (key2 protected) */
/*     calibration of Ron has been executed.     */
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPEX  (0x1<<1)
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPEX_POS  1
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPEX_MAX  1
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_MTPEX_MSK  0x2
/* not used */
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_2  (0x3fff<<2)
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_2_POS  2
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_2_MAX  16383
#define TFA98XX_KEY2PROTECTED_SPKR_CAL_MTP_2_MSK  0xfffc

/**   Key1Protected_MTP1 Register ($81) **/
#define TFA98XX_KEY1PROTECTED_MTP1  0x81
#define TFA9890_KEY1PROTECTED_MTP1_POR  0x0000
/* calibr_vout_offset (DCDCoffset) 2's compliment (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTP1_0  (0x3f<<0)
#define TFA98XX_KEY1PROTECTED_MTP1_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTP1_0_MAX  63
#define TFA98XX_KEY1PROTECTED_MTP1_0_MSK  0x3f
/* delta gain for vamp (alpha) 2's compliment (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTP1_6  (0xf<<6)
#define TFA98XX_KEY1PROTECTED_MTP1_6_POS  6
#define TFA98XX_KEY1PROTECTED_MTP1_6_MAX  15
#define TFA98XX_KEY1PROTECTED_MTP1_6_MSK  0x3c0
/* offset for vamp (Ampoffset) 2's compliment (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTP1_10  (0x3f<<10)
#define TFA98XX_KEY1PROTECTED_MTP1_10_POS  10
#define TFA98XX_KEY1PROTECTED_MTP1_10_MAX  63
#define TFA98XX_KEY1PROTECTED_MTP1_10_MSK  0xfc00

/**   Key1Protected_MTP2 Register ($82) **/
#define TFA98XX_KEY1PROTECTED_MTP2  0x82
#define TFA9890_KEY1PROTECTED_MTP2_POR  0x0000
/* gain current sense (Imeasalpha) 2's compliment (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTP2_0  (0xff<<0)
#define TFA98XX_KEY1PROTECTED_MTP2_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTP2_0_MAX  255
#define TFA98XX_KEY1PROTECTED_MTP2_0_MSK  0xff
/* temperature offset 2's compliment (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTP2_8  (0x1f<<8)
#define TFA98XX_KEY1PROTECTED_MTP2_8_POS  8
#define TFA98XX_KEY1PROTECTED_MTP2_8_MAX  31
#define TFA98XX_KEY1PROTECTED_MTP2_8_MSK  0x1f00
/* temperature gain 2's compliment (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTP2_13  (0x7<<13)
#define TFA98XX_KEY1PROTECTED_MTP2_13_POS  13
#define TFA98XX_KEY1PROTECTED_MTP2_13_MAX  7
#define TFA98XX_KEY1PROTECTED_MTP2_13_MSK  0xe000

/**   Key1Protected_MTP3 Register ($83) **/
#define TFA98XX_KEY1PROTECTED_MTP3  0x83
#define TFA9890_KEY1PROTECTED_MTP3_POR  0x0000
/* Ron resistance of coil (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTP3_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTP3_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTP3_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTP3_0_MSK  0xffff

/**   Key1Protected_MTP4 Register ($84) **/
#define TFA98XX_KEY1PROTECTED_MTP4  0x84
#define TFA9890_KEY1PROTECTED_MTP4_POR  0x0000
/* Offset of amplifier level shifter */
#define TFA98XX_KEY1PROTECTED_MTP4_0  (0x7f<<0)
#define TFA98XX_KEY1PROTECTED_MTP4_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTP4_0_MAX  127
#define TFA98XX_KEY1PROTECTED_MTP4_0_MSK  0x7f
/* not used */
#define TFA98XX_KEY1PROTECTED_MTP4_7  (0x1<<7)
#define TFA98XX_KEY1PROTECTED_MTP4_7_POS  7
#define TFA98XX_KEY1PROTECTED_MTP4_7_MAX  1
#define TFA98XX_KEY1PROTECTED_MTP4_7_MSK  0x80
/* Offset of amplifier level shifter */
#define TFA98XX_KEY1PROTECTED_MTP4_8  (0x7f<<8)
#define TFA98XX_KEY1PROTECTED_MTP4_8_POS  8
#define TFA98XX_KEY1PROTECTED_MTP4_8_MAX  127
#define TFA98XX_KEY1PROTECTED_MTP4_8_MSK  0x7f00
/* not used */
#define TFA98XX_KEY1PROTECTED_MTP4_15  (0x1<<15)
#define TFA98XX_KEY1PROTECTED_MTP4_15_POS  15
#define TFA98XX_KEY1PROTECTED_MTP4_15_MAX  1
#define TFA98XX_KEY1PROTECTED_MTP4_15_MSK  0x8000

/**   Key1Protected_MTP5 Register ($85) **/
#define TFA98XX_KEY1PROTECTED_MTP5  0x85
#define TFA9890_KEY1PROTECTED_MTP5_POR  0x0000
/* bit 0 = disable function dcdcoff_mode ($09[7])  */
/*          0: Function dcdcoff_mode enabled via ($09[7])     */
/*          1: DCDC permanant switched off     */
/*     bit 1 = disable function enbl_coolflux ($9[2])     */
/*          0:  function enbl_coolflux enabled via ($9[2])     */
/*          1: CoolFlux permanant switched off     */
/*     bit 2 = Disable function bypass_clipper ($5[0])     */
/*          0:  function bypass_clipper enabled via ($5[15])     */
/*          1: Clipper permanant bypassed     */
/*     bit 3 = disable function intf_sel ($9[13])     */
/*          0:  function intf_sel enabled via ($5[15])     */
/*          1: Permanant input 1     */
/*     bit 4 = disable function datao_sel ($04[9,8])     */
/*          0:  function datao_sel enabled via ($04[9,8])     */
/*          1: datao_sel ($04[9,8]) is fixed to 00 resulting into I2S-TX     */
/*     bit 5 = Star     */
/*          0:  All customers     */
/*          1: Star only     */
#define TFA98XX_KEY1PROTECTED_MTP5_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTP5_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTP5_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTP5_0_MSK  0xffff

/**   Key1Protected_MTP6 Register ($86) **/
#define TFA98XX_KEY1PROTECTED_MTP6  0x86
#define TFA9890_KEY1PROTECTED_MTP6_POR  0x0000
/* MTP-control SW */
/*     bit 0 = 1 --> disable all 2 dual precision biquads     */
/*     bit 1 = 1  --> disable all 6 single precision biquads     */
/*     bit 8 = 1 --> use tcoefA     */
/*     bit 9  is reserved for TFA9887B/87B differentation     */
#define TFA98XX_KEY1PROTECTED_MTP6_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTP6_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTP6_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTP6_0_MSK  0xffff

/**   Key1Protected_MTP7 Register ($87) **/
#define TFA98XX_KEY1PROTECTED_MTP7  0x87
#define TFA9890_KEY1PROTECTED_MTP7_POR  0x0000
/* MTP-control SW2 */
/*     bit 0 = 1  --> disable all 2 dual precision biquads     */
/*     bit 1 = 1  --> disable all 6 single precision biquads     */
/*     bit 2 = 1  --> disable activity detector     */
/*     bit 3 = 1  --> disable silence detector     */
/*     bit 4 = 1  --> disable X protection     */
/*     bit 5 = 1  --> disable BW extension     */
/*     bit 6 = 1  --> disable speaker temperature protection     */
/*     bit 7 = 1  --> disable traces     */
/*     bit 8 = 1  --> disable damage detection     */
/*     bit 9 = 1  --> AGC = off     */
/*     bit 10 = 1  --> disable low latency     */
/*     bit 11 = 1  --> disable 2 EQ filters     */
/*     bit 12 = 1  --> disable low cut off filter     */
/*     bit 13 = 1  --> disable resonance compensation filter     */
/*     bit 14 = 1  --> no stereo synchronisation     */
/*     bit 15 = 1  --> no x margins     */
#define TFA98XX_KEY1PROTECTED_MTP7_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTP7_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTP7_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTP7_0_MSK  0xffff

/**   Key1Protected_MTPA Register ($8a) **/
#define TFA98XX_KEY1PROTECTED_MTPA  0x8a
#define TFA9890_KEY1PROTECTED_MTPA_POR  0x0000
/* (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTPA_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTPA_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTPA_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTPA_0_MSK  0xffff

/**   Key1Protected_MTPB Register ($8b) **/
#define TFA98XX_KEY1PROTECTED_MTPB  0x8b
#define TFA9890_KEY1PROTECTED_MTPB_POR  0x0000
/* (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTPB_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTPB_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTPB_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTPB_0_MSK  0xffff

/**   Key1Protected_MTPC Register ($8c) **/
#define TFA98XX_KEY1PROTECTED_MTPC  0x8c
#define TFA9890_KEY1PROTECTED_MTPC_POR  0x0000
/* (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTPC_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTPC_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTPC_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTPC_0_MSK  0xffff

/**   Key1Protected_MTPD Register ($8d) **/
#define TFA98XX_KEY1PROTECTED_MTPD  0x8d
#define TFA9890_KEY1PROTECTED_MTPD_POR  0x0000
/* (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTPD_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTPD_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTPD_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTPD_0_MSK  0xffff

/**   Key1Protected_MTPE Register ($8e) **/
#define TFA98XX_KEY1PROTECTED_MTPE  0x8e
#define TFA9890_KEY1PROTECTED_MTPE_POR  0x0000
/* (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTPE_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTPE_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTPE_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTPE_0_MSK  0xffff

/**   Key1Protected_MTPF Register ($8f) **/
#define TFA98XX_KEY1PROTECTED_MTPF  0x8f
#define TFA9890_KEY1PROTECTED_MTPF_POR  0x0000
/* (key1 protected) */
#define TFA98XX_KEY1PROTECTED_MTPF_0  (0xffff<<0)
#define TFA98XX_KEY1PROTECTED_MTPF_0_POS  0
#define TFA98XX_KEY1PROTECTED_MTPF_0_MAX  65535
#define TFA98XX_KEY1PROTECTED_MTPF_0_MSK  0xffff

