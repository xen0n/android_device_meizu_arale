#ifndef TFA9890_CUST_H
#define TFA9890_CUST_H

#define TFA_I2CDEVICE "/dev/smartpa_i2c" //dev node provides the i2c read/write interface.
#define TFA_I2CSLAVEBASE (0x34)  // tfa device slave address
#define SAMPLE_RATE 44100 //init sample rate(unit Hz)
//#define BYPASS_DSP //use smart pa as a normal class D PA.
/* Before the echo cancellation argo in handsfree mode is ok, when in call mode, bypass the dsp. 
 * When MTK's new EC argo with smart pa is ok, remove this part. */
//#define BYPASS_DSP_INCALL 
#ifdef BYPASS_DSP_INCALL
#undef BYPASS_DSP
#endif

/* the base speaker file, containing tCoef */
/* Dumbo speaker
#define SPEAKER_FILENAME "KS_13X18_DUMBO.speaker"
#define PRESET_FILENAME "Example_0_0.KS_13X18_DUMBO.preset"
#define EQ_FILENAME "Example_0_0.KS_13X18_DUMBO.eq"*/
/* Sambo speaker*/
//#define SPEAKER_FILENAME "KS_90_13x18_Sambo_V2R2.speaker"
//#define PRESET_FILENAME "Seltech_HQ_V2R2_0_0_KS_90_13x18_Sambo_V2R2.preset"
//#define EQ_FILENAME "Seltech_HQ_V2R2_0_0_KS_90_13x18_Sambo_V2R2.eq"
//#define CONFIG_FILENAME "Setup.config"
//#define PATCH_FILENAME "TFA9890_N1C3_1_5_1.patch"

#define COLDBOOTPATCH_FILENAME "coldboot.patch"

#define PATCH_FILENAME "TFA9890_N1C3_1_5_1.patch"
/* the base speaker file, containing tCoef */
#define SPEAKER_FILENAME "ly.speaker"
#define CONFIG_FILENAME  "ly.config"
#define PRESET_FILENAME  "ly_0_0_ly.preset"
#define EQ_FILENAME      "ly_0_0_ly.eq"


#ifdef WIN32
// cwd = dir where vcxproj is
#define LOCATION_FILES "../../../../settings/"
#else
// cwd = linux dir
#define LOCATION_FILES "/etc/smartpa_params/"
#endif



#endif/*TFA9890_CUST_H*/
