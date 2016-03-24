/*
 * @file tfa98xxDiagnostics.c
 *
 *  Created on: Jun 7, 2012
 *      Author: Wim Lemmers
 */

#include "../inc/tfa98xxDiagnostics.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// TFA98XX API
#include "../../Tfa98xx/inc/Tfa98xx.h"
#include "../../Tfa98xx/inc/Tfa98xx_Registers.h"

#ifdef noDIAGTRACE
#include <stdio.h>
#define TRACEIN  if(tfa98xxDiag_trace) printf("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xxDiag_trace) printf("Leave %s\n", __FUNCTION__);
#else
#define TRACEIN
#define TRACEOUT
#endif

/* *INDENT-OFF* */
regdef_t regdefs[] = {
        { 0x00, 0x081d, 0xfeff, "statusreg"}, //ignore MTP busy bit
        { 0x01, 0x0, 0x0, "batteryvoltage"},
        { 0x02, 0x0, 0x0, "temperature"},
        { 0x03, 0x0012, 0xffff, "revisionnumber"},
        { 0x04, 0x888b, 0xffff, "i2sreg"},
        { 0x05, 0x13aa, 0xffff, "bat_prot"},
        { 0x06, 0x001f, 0xffff, "audio_ctr"},
        { 0x07, 0x0fe6, 0xffff, "dcdcboost"},
        { 0x08, 0x0800, 0x3fff, "spkr_calibration"}, //NOTE: this is a software fix to 0xcoo
        { 0x09, 0x041d, 0xffff, "sys_ctrl"},
        { 0x0a, 0x3ec3, 0x7fff, "i2s_sel_reg"},
        { 0x40, 0x0, 0x00ff, "hide_unhide_key"},
        { 0x41, 0x0, 0x0, "pwm_control"},
        { 0x46, 0x0, 0x0, "currentsense1"},
        { 0x47, 0x0, 0x0, "currentsense2"},
        { 0x48, 0x0, 0x0, "currentsense3"},
        { 0x49, 0x0, 0x0, "currentsense4"},
        { 0x4c, 0x0, 0xffff, "abisttest"},
        { 0x62, 0x0, 0, "mtp_copy"},
        { 0x70, 0x0, 0xffff, "cf_controls"},
        { 0x71, 0x0, 0, "cf_mad"},
        { 0x72, 0x0, 0, "cf_mem"},
        { 0x73, 0x00ff, 0xffff, "cf_status"},
        { 0x80, 0x0, 0, "mtp"},
        { 0x83, 0x0, 0, "mtp_re0"},
        { 0xff, 0,0, NULL}
};
/* *INDENT-ON* */
#define MAXREGS ((sizeof(regdefs)/sizeof(regdef_t))-1)

// status register errors to check for not 1
#define TFA98XX_STATUS_ERRORS_SET_MSK (  \
		TFA98XX_STATUS_OCDS  )
// status register errors to check for not 0
#define TFA98XX_STATUS_ERRORS_CLR_MSK (  TFA98XX_STATUS_VDDS  |\
		TFA98XX_STATUS_UVDS  |  \
		TFA98XX_STATUS_OVDS  |  \
		TFA98XX_STATUS_OTDS    )
//      TFA98XX_STATUS_DCCS   ) TODO check bit

// register used in rw diag
#define RWTEST_REG TFA98XX_CF_MAD
/*
 * check status register flags and do not check coldstart
 */
int tfa98xxDiagStatus(Tfa98xx_handle_t handle, unsigned short mask,
                      unsigned short clearmask);

// globals
int tfa98xxDiag_trace = 1;
int tfa98xxDiag_verbose = 1;
static Tfa98xx_handle_t gHandle=-1; // for nesting
static int lastTest = -1;
static int lastError = -1;
static char lastErrorString[256] = "";

static Tfa98xx_Error_t lastApiError;

/*
 * TODO make this permanent?
 */
int tfa98xxDiagLoadPresetsDouble(int slave);
int tfa98xxDiagLoadPresetsMultiple(int slave);


/* *INDENT-OFF* */
/*
 * test functions array
 */
tfa98xxDiagTest_t DiagTest[] = {
       {tfa98xxDiagAll,"run all tests"},
       {tfa98xxDiagI2cScan, "scan of I2C for ID registers and check for expected device"},
       {tfa98xxDiagI2cRw, "write/read test of an I2C register"},
       {tfa98xxDiagStatusCold, "check status register flags and assume coldstart (or fail)"},
       {tfa98xxDiagRegisterDefaults, "verify default state of relevant registers"},
       {tfa98xxDiagClock, "enable clocks in bypass and verify status"},
       {tfa98xxDiagDsp, "start DSP and verify (by reading ROM tag and check status)"},
       {tfa98xxDiagLoadConfig, "load configuration settings and verify readback"},
       {tfa98xxDiagLoadPreset, "load preset values and verify readback"},
       {tfa98xxDiagLoadSpeaker, "load speaker parameters and verify readback"},
       {tfa98xxDiagBattery, "check battery level"},
       {tfa98xxDiagSpeakerPresence, "verify speaker presence by checking the resistance"},
       {tfa98xxDiagI2sInput, "assume I2S input and verify signal activity"},
       {tfa98xxDiagLoadPresetsDouble, "load presets L+R 2x single API"},
       {tfa98xxDiagLoadPresetsMultiple, "load presets L+R multiple API"}//,
//TBD?       {tfa98xxDiagCalibration, "calibrate speaker and verify speaker presence and DCDC"},
//       {tfa98xxDiagI2sOutput, "for testing the I2S output an external receiver should acknowlege data presence (TBD)"}
};
/* *INDENT-ON* */

/*
 *  the functions will return 0 if passed
 *  else fail and the return value may contain extra info
 */
 /*
  * run a testnr
  */
int tfa98xxDiag(int slave, int testnr)
{
        int maxtest = sizeof(DiagTest) / sizeof(tfa98xxDiagTest_t);
        int result;

        if (testnr > maxtest) {
                fprintf(stderr, "%s: test number:%d too high, max %d \n",
                        __FUNCTION__, testnr, maxtest);
                return -1;
        }

        if (tfa98xxDiag_verbose)
                printf("test %d : %s\n", testnr, DiagTest[testnr].description);

        result = DiagTest[testnr].function(slave);

        if (testnr != 0)        // don't if 0 that's for all tests
                lastTest = testnr;      // remember the last test for reporting

        return result;

}

// run all
int tfa98xxDiagAll(int slave)
{
        int i, result = 0, maxtest =
            sizeof(DiagTest) / sizeof(tfa98xxDiagTest_t) - 1;

        TRACEIN;
        for (i = 1; i <= maxtest; i++) {
                result = tfa98xxDiag(slave, i);
                if (result != 0)
                        break;
        }
        TRACEOUT;
        return result;
}

/*
 * return latest testnr
 */
int tfa98xxDiagGetLatest(void)
{
        return lastTest;
}

/*
 * return last error code
 */
int tfa98xxDiagGetLastError(void)
{
        return lastError;
}

/*
 * return last error string
 */
char *tfa98xxDiagGetLastErrorString(void)
{
        return lastErrorString;
}

/*
 * return testname string
 *  if testnr is too big an empty string is returned
 *
 */
const char *tfa98xxDiagGetTestNameString(int testnr)
{
        if (testnr < MAXREGS)
                return DiagTest[testnr].description;
        else
                return "";
}

/*
 * scan of I2C for ID registers and check for expected device
 *  open the expected device
 */
int tfa98xxDiagI2cScan(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);

        if (lastApiError == Tfa98xx_Error_Ok) {
				Tfa98xx_Close(handle);
        }
        else {
                result = 1;     // non-0 if fail
                sprintf(lastErrorString, "can't find i2c slave 0x%0x", slave);
        }

        lastError = result;
        TRACEOUT;

        return result;
}

/*
 * write/read test of a register that has no risk of causing damage
 *   return code:
 *   	2 :  write value wrong
 *
  */
int tfa98xxDiagI2cRw(int slave)
{
        Tfa98xx_handle_t handle;
        int testregoffset, result = 0;      // 1 is failure
        unsigned short testreg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 1;

        // get the index of the testreg
        for ( testregoffset = 0;  testregoffset < MAXREGS;  testregoffset++) {
        	if (regdefs[ testregoffset].offset == RWTEST_REG)
        		break;
        }

        // powerdown to avoid side effects
        lastApiError = Tfa98xx_Powerdown(handle, 1);
        assert(lastApiError == Tfa98xx_Error_Ok);
        // check pwron default first
        lastApiError = Tfa98xx_ReadRegister16(handle, RWTEST_REG, &testreg);    //
        assert(lastApiError == Tfa98xx_Error_Ok);

        if (regdefs[ testregoffset].pwronDefault != (testreg & regdefs[ testregoffset].pwronTestmask)) {
                sprintf(lastErrorString,
                        "poweron default wrong: %s (0x%02x), exp:0x%04x rcv:0x%04x\n",
                        regdefs[ testregoffset].name, regdefs[ testregoffset].offset,
                        regdefs[ testregoffset].pwronDefault, testreg);
                result = 2;
                goto stop;
        }
        // write 0x1234
        lastApiError = Tfa98xx_WriteRegister16(handle, RWTEST_REG, 0x1234);     //
        assert(lastApiError == Tfa98xx_Error_Ok);
        lastApiError = Tfa98xx_ReadRegister16(handle, RWTEST_REG, &testreg);    //
        assert(lastApiError == Tfa98xx_Error_Ok);

        // restore default, else pwrdefault may fail
        lastApiError = Tfa98xx_WriteRegister16(handle, RWTEST_REG, regdefs[RWTEST_REG].pwronDefault);   //
        assert(lastApiError == Tfa98xx_Error_Ok);

        if (0x1234 != testreg) {
                sprintf(lastErrorString,
                        "read back value mismatch: (testreg=0x%02x), exp:0x%04x rcv:0x%04x\n",
                        RWTEST_REG, 0x1234, testreg);
                result = 1;
        }

 stop:
		Tfa98xx_Close(handle);

        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * check status register flags and assume coldstart (or fail)
 *   return code:
 *      1 : error bit
 *   	2 : not cold powered on
 *   	other internal errors
 */
int tfa98xxDiagStatusCold(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure
        unsigned short statusreg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok) {
                return 3;
        }

        lastApiError =
            Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &statusreg);
        assert(lastApiError == Tfa98xx_Error_Ok);

        if (!(statusreg & TFA98XX_STATUS_ACS)) {        /* ensure cold booted */
                sprintf(lastErrorString, "not a cold start");
                result = 2;
                goto stop;
        }

        if (tfa98xxDiagStatus
            (handle, TFA98XX_STATUS_ERRORS_SET_MSK,
             TFA98XX_STATUS_ERRORS_CLR_MSK)) {
                sprintf(lastErrorString, "status errorbit active");
                result = 1;
        }

 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * check status register flags for any of the error bits set
 *  return 0 if ok
 *  	   1 if not ok
 *  	   other internal errors
 */
int tfa98xxDiagStatus(Tfa98xx_handle_t handle, unsigned short setmask,
                      unsigned short clearmask)
{
        int result = 0;         // 1 is failure
        unsigned short statusreg;

        TRACEIN;

        lastApiError =
            Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &statusreg);
        assert(lastApiError == Tfa98xx_Error_Ok);

        if ((statusreg & setmask))      /* check for any of these bits set */
                return 1;
        if ((~statusreg & clearmask))   /* check for any of these bits clear */
                return 1;

        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * verify default state of relevant registers
 */
int tfa98xxDiagRegisterDefaults(int slave)
{
        Tfa98xx_handle_t handle;
        int i, result = 0;      // 1 is failure
        unsigned short regval;
        unsigned char reg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 2;

        for (i = 0; i < MAXREGS; i++) {
                if (regdefs[i].pwronTestmask == 0)
                        continue;
                lastApiError =
                    Tfa98xx_ReadRegister16(handle, regdefs[i].offset, &regval);
                assert(lastApiError == Tfa98xx_Error_Ok);

                if (regdefs[i].pwronDefault !=
                    (regval & regdefs[i].pwronTestmask)) {
                        sprintf(lastErrorString,
                                "poweron default wrong: %s (0x%02x), exp:0x%04x rcv:0x%04x\n",
                                regdefs[i].name, regdefs[i].offset,
                                regdefs[i].pwronDefault, regval);
                        result++;
                }
        }
        // set DC-DC peak protection bit
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SPKR_CALIBRATION, 0x0c00);       //
        assert(lastApiError == Tfa98xx_Error_Ok);

 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * dump all known registers
 *   returns:
 *     0 if slave can't be opened
 *     nr of registers displayed
 */
int tfa98xxDiagRegisterDump(int slave)
{
        Tfa98xx_handle_t handle;
        int i;
        unsigned short regval;
        unsigned char reg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 0;

        for (i = 0; i < MAXREGS; i++) {
                lastApiError =
                    Tfa98xx_ReadRegister16(handle, regdefs[i].offset, &regval);
                assert(lastApiError == Tfa98xx_Error_Ok);
                printf("0x%02x: 0x%04x (%s)\n",
                       regdefs[i].offset, regval, regdefs[i].name);
        }

        Tfa98xx_Close(handle);

        TRACEOUT;
        return i;
}

/*
 * enable clocks in bypass and verify status
 */
int tfa98xxDiagClock(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 3;

        // enable the clock in bypass mode

        // 48 kHz left channel I2S with coolflux in Bypass
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, 0x880b); //
        assert(lastApiError == Tfa98xx_Error_Ok);
        //  PLL=BCK, input 1, power off
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0219);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
        // 1.0 uF coil, PLL=BCK, input 1, power on
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0618);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
		// give it some time
		usleep(14000); // 14ms good for all rates
        // check if clocks are stable and running
        // expect: 0x00 : 0xd85f
        if (tfa98xxDiagStatus
            (handle, 0, TFA98XX_STATUS_PLLS | TFA98XX_STATUS_CLKS)) {
                sprintf(lastErrorString, "clock not running");
                result = 1;     // fail if any of these is clear
                goto stop;
        }
        // any other errors
        if (tfa98xxDiagStatus
            (handle, TFA98XX_STATUS_ERRORS_SET_MSK,
             TFA98XX_STATUS_ERRORS_CLR_MSK)) {
                sprintf(lastErrorString, "status errorbit active");
                result = 2;
        }
 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/********************************************************************************
 * TODO check if the DSP should be powered down after every test
 */
static void coldStartup(Tfa98xx_handle_t handle)
{
        Tfa98xx_Error_t err;
        unsigned short status;

        /* load the optimal TFA9887 in HW settings */
        err = Tfa98xx_Init(handle);
        assert(err == Tfa98xx_Error_Ok);

        err = Tfa98xx_SetSampleRate(handle, 44100);
        assert(err == Tfa98xx_Error_Ok);

        err = Tfa98xx_Powerdown(handle, 0);
        assert(err == Tfa98xx_Error_Ok);

}

/*
 * start dsp and verify (by reading ROM tag and check status)
 */
#define TFA98XX_MAXTAG              (138)
#define DSP_revstring        "< Dec 21 2011 - 12:33:16 -  SpeakerBoostOnCF >"

int tfa98xxDiagDsp(int slave)
{
        Tfa98xx_handle_t handle;
        int i, result = 0;      // !0 is failure
        char tag[TFA98XX_MAXTAG], string[TFA98XX_MAXTAG + 1], *ptr;
        const char *exp = DSP_revstring;

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 3;

        coldStartup(handle);

        lastApiError =
            Tfa98xx_DspGetParam(handle, 1 /*MODULE_SPEAKERBOOST */ , 0xFF,
                                TFA98XX_MAXTAG, tag);
        if (lastApiError != Tfa98xx_Error_Ok) {
                sprintf(lastErrorString, "DSP failure");
                return 1;
        }

        ptr = string;
        // the characters are in every 3rd byte
        for (i = 2; i < TFA98XX_MAXTAG; i += 3) {
                if (isprint(tag[i])) {
                        *ptr++ = tag[i];        // only printable chars
                }
        }
        *ptr = '\0';

        if (strcmp(exp, string)) {
                sprintf(lastErrorString, "wrong DSP revtag: exp %s rcv:%s\n",
                        exp, string);
                return 2;
        }
 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * load configuration settings and verify readback
 */
unsigned char settings_Setup87_config[];
int tfa98xxDiagLoadConfig(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 1;         // 1 is failure
        Tfa98xx_Config_t cfg;

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 2;

        coldStartup(handle);
        lastApiError = Tfa98xx_DspWriteConfig(handle, TFA98XX_CONFIG_LENGTH,
        		settings_Setup87_config);
        assert(lastApiError == Tfa98xx_Error_Ok);

        lastApiError =
            Tfa98xx_DspGetParam(handle, 1 /*MODULE_SPEAKERBOOST */ , 0x80,
                                sizeof(cfg), cfg);
        assert(lastApiError == Tfa98xx_Error_Ok);

        result =
            0 != memcmp(settings_Setup87_config, cfg, sizeof(Tfa98xx_Config_t));
        if (result)
                sprintf(lastErrorString, "DSP parameters mismatch");

 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * load preset values and verify readback
 */
unsigned char settings_HQ_KS_13X18_DUMBO_preset[];

int tfa98xxDiagLoadPreset(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 1;         // 1 is failure
        Tfa98xx_Config_t cfg;
        unsigned char tstbuf[0x87 + sizeof(cfg)];

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 2;

        coldStartup(handle);
        lastApiError =
            Tfa98xx_DspWritePreset(handle, TFA98XX_PRESET_LENGTH,
            			settings_HQ_KS_13X18_DUMBO_preset);
        assert(lastApiError == Tfa98xx_Error_Ok);

        lastApiError =
            Tfa98xx_DspGetParam(handle, 1 /*MODULE_SPEAKERBOOST */ , 0x80,
                                sizeof(tstbuf), tstbuf);
        assert(lastApiError == Tfa98xx_Error_Ok);

        result =
            0 != memcmp(settings_HQ_KS_13X18_DUMBO_preset, &tstbuf[sizeof(cfg)],
                        sizeof(Tfa98xx_Preset_t));
        if (result)
                sprintf(lastErrorString, "DSP parameters mismatch");

 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

unsigned char settings_KS_13X18_DUMBO_speaker[];
/*
 * load speaker parameters and verify readback
 *
 *   note: this function can be called from other tests it uses the global handle
 */
int tfa98xxDiagLoadSpeaker(int slave)
{
        int result = 1;         // 1 is failure
        int gbl=0;
        Tfa98xx_SpeakerParameters_t spkr;

        TRACEIN;

        if ( gHandle<0 ) {
            lastApiError = Tfa98xx_Open(slave << 1, &gHandle);
            if (lastApiError != Tfa98xx_Error_Ok)
                return 2;
        } else
            gbl = 1;

        coldStartup(gHandle);
        lastApiError =
            Tfa98xx_DspWriteSpeakerParameters(gHandle, TFA98XX_SPEAKERPARAMETER_LENGTH,
                                              settings_KS_13X18_DUMBO_speaker);
        assert(lastApiError == Tfa98xx_Error_Ok);

        lastApiError =
            Tfa98xx_DspGetParam(gHandle, 1 /*MODULE_SPEAKERBOOST */ , 0x86,
                                sizeof(spkr), spkr);
        assert(lastApiError == Tfa98xx_Error_Ok);

        result =
            0 != memcmp(settings_KS_13X18_DUMBO_speaker, spkr,
                        sizeof(Tfa98xx_SpeakerParameters_t));
        if (result)
                sprintf(lastErrorString, "DSP parameters mismatch");

 stop:
         if ( !gbl ) {
             Tfa98xx_Close(gHandle);
             lastError = result;
             gHandle=-1;
         }
        TRACEOUT;
        return result;

}

/*
 * check battery level to be above 2Volts
 *
 *
 */
int tfa98xxDiagBattery(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure
        unsigned short reg;

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 4;

        // enable the clock in bypass mode

        // 48 kHz left channel I2S with coolflux in Bypass
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, 0x880b); //
        assert(lastApiError == Tfa98xx_Error_Ok);
        //  PLL=BCK, input 1, power off
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0219);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
        // 1.0 uF coil, PLL=BCK, input 1, power on
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0618);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
        // check if clocks are stable and running
		// give it some time
		usleep(14000); // 14ms good for all rates
        // expect: 0x00 : 0xd85f
        if (tfa98xxDiagStatus
            (handle, 0, TFA98XX_STATUS_PLLS | TFA98XX_STATUS_CLKS)) {
                sprintf(lastErrorString, "clock not running");
                result = 2;     // fail if any of these is clear
                goto stop;
        }
        // check battery level
        lastApiError =
            Tfa98xx_ReadRegister16(handle, TFA98XX_BATTERYVOLTAGE, &reg);
        assert(lastApiError == Tfa98xx_Error_Ok);
        // 2V: 2/(5.5/1024)=372
        if (reg < 372) {
                sprintf(lastErrorString,
                        "battery level too low: exp > 2.0V, rcv=%2.2f",
                        reg * (5.5 / 1024));
                result = 1;
                goto stop;
        }
        // any other errors
        if (tfa98xxDiagStatus
            (handle, TFA98XX_STATUS_ERRORS_SET_MSK,
             TFA98XX_STATUS_ERRORS_CLR_MSK)) {
                sprintf(lastErrorString, "status errorbit active");
                result = 3;
        }
 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * wait until calibration impedance is ok
 *  operate on all devices that have a handle open
 */
static void waitCalibration(Tfa98xx_handle_t handle, float *pRe25)
{
        Tfa98xx_Error_t err;
        int calibrateDone, count = 500;

        assert(pRe25 != NULL);

        err = Tfa98xx_DspReadMem(handle, 231, 1, &calibrateDone);
        assert(err == Tfa98xx_Error_Ok);
        while ((calibrateDone == 0) && (count-- > 0))   // TODO protect loop with timeout?
        {
                err = Tfa98xx_DspReadMem(handle, 231, 1, &calibrateDone);
                assert(err == Tfa98xx_Error_Ok);
        }
        err = Tfa98xx_DspGetCalibrationImpedance(handle, pRe25);
        assert(err == Tfa98xx_Error_Ok);

}

/*
 * verify speaker presence by checking the resistance
 */
int tfa98xxDiagSpeakerPresence(int slave)
{
        Tfa98xx_handle_t handle;
        int result;
        float re0;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 3;

        gHandle = handle;
        result = tfa98xxDiagLoadSpeaker(slave);
        if (result) {
                result = 2;
                goto stop;
        }
        lastApiError = Tfa98xx_SetConfigured(handle);
        assert(lastApiError == Tfa98xx_Error_Ok);
        waitCalibration(handle, &re0);

        // check R for non-0
        if (re0 == 0) {
                sprintf(lastErrorString, "Speaker not detected");
                result = 1;
        } else
                result = 0;

 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        gHandle = -1;
        TRACEOUT;
        return result;
}

/*
 * calibrate speaker and verify speaker presence check R range ,  verifies power from Vbat via DCDC to amplifier
 */
int tfa98xxDiagCalibration(int slave)
{
        TRACEIN;

        TRACEOUT;
        return 1;
}

/*
 * assume I2S input and verify signal activity
 */
int tfa98xxDiagI2sInput(int slave)
{
        {
                Tfa98xx_handle_t handle;
                int result;
                Tfa98xx_StateInfo_t stateInfo;
                unsigned short sysctrlReg;

                TRACEIN;

                lastApiError = Tfa98xx_Open(slave << 1, &handle);
                if (lastApiError != Tfa98xx_Error_Ok)
                        return 3;

                gHandle = handle; // global prevents reopen
                result = tfa98xxDiagLoadSpeaker(slave);
                if (result) {
                        result = 2;
                        goto stop;
                }
                lastApiError = Tfa98xx_SetConfigured(handle);
                assert(lastApiError == Tfa98xx_Error_Ok);

                //select channel

                lastApiError =
                    Tfa98xx_ReadRegister16(handle, 0x09, &sysctrlReg);
                assert(lastApiError == Tfa98xx_Error_Ok);

                lastApiError = Tfa98xx_WriteRegister16(handle, 0x09, sysctrlReg & ~(0x1 << 13));        // input 1
                //   lastApiError = Tfa98xx_WriteRegister16(handle, 0x09, sysctrlReg | (0x1<<13));  // input 2
                assert(lastApiError == Tfa98xx_Error_Ok);

                lastApiError =
                    Tfa98xx_DspGetStateInfo(handle, &stateInfo);
                assert(lastApiError == Tfa98xx_Error_Ok);

                // check for activity
                if ((stateInfo.statusFlag &
                     (1 << Tfa98xx_SpeakerBoost_Activity)) == 0) {
                        sprintf(lastErrorString, "no audio active on input");
                        result = 1;
                } else
                        result = 0;

 stop:
                Tfa98xx_Close(handle);
                lastError = result;
                gHandle = -1;
                TRACEOUT;
                return result;
        }
}

/*
 * for testing the I2S output an external receiver should acknowlege data presence
 */
int tfa98xxDiagI2sOutput(int slave)
{
        TRACEIN;

        TRACEOUT;
        return 1;
}
/*
 * ************************************ TODO keep these demos?
 */
int tfa98xxDiagLoadPresetsDouble(int slave) {
	Tfa98xx_handle_t hL, hR ;
	int result = 0; // 1 is failure
	Tfa98xx_Config_t cfg;
	unsigned char tstbuf[0x87 + sizeof(cfg)];

	TRACEIN;
	lastApiError = Tfa98xx_Open(slave << 1, &hL);
	if (lastApiError != Tfa98xx_Error_Ok)
		return 2;
	lastApiError = Tfa98xx_Open(slave+1 << 1, &hR);
	if (lastApiError != Tfa98xx_Error_Ok)
		return 3;


	lastApiError = Tfa98xx_DspWritePreset(hL, TFA98XX_PRESET_LENGTH,
			settings_HQ_KS_13X18_DUMBO_preset);
	assert(lastApiError == Tfa98xx_Error_Ok);

	lastApiError = Tfa98xx_DspWritePreset(hR, TFA98XX_PRESET_LENGTH,
			settings_HQ_KS_13X18_DUMBO_preset);
	assert(lastApiError == Tfa98xx_Error_Ok);


  stop:
	Tfa98xx_Close(hL);
	Tfa98xx_Close(hR);
	lastError = result;
	TRACEOUT;
	return result;
}

int tfa98xxDiagLoadPresetsMultiple(int slave)
{
	Tfa98xx_handle_t hL, hR, handles[2] ;
	int result = 0; // 1 is failure
	Tfa98xx_Config_t cfg;
	unsigned char tstbuf[0x87 + sizeof(cfg)];

	TRACEIN;
	lastApiError = Tfa98xx_Open(slave << 1, &hL);
	if (lastApiError != Tfa98xx_Error_Ok)
		return 2;
	handles[0]=hL;
	lastApiError = Tfa98xx_Open(slave+1 << 1, &hR);
	if (lastApiError != Tfa98xx_Error_Ok)
		return 3;
	handles[1]=hR;


	lastApiError = Tfa98xx_DspWritePresetMultiple(2, handles, TFA98XX_PRESET_LENGTH,
			settings_HQ_KS_13X18_DUMBO_preset);
	assert(lastApiError == Tfa98xx_Error_Ok);


  stop:
	Tfa98xx_Close(hL);
	Tfa98xx_Close(hR);
	lastError = result;
	TRACEOUT;
	return result;
}
/*
 * binary buffers
 */
// xxd -i settings/Setup87.config
unsigned char settings_Setup87_config[] = {
        0x09, 0xf3, 0x33, 0x01, 0x3e, 0x66, 0x00, 0x54, 0xcd, 0x00, 0x00, 0x14,
        0x00, 0x00, 0x02, 0x1a, 0xee, 0xb4, 0x1b, 0x49, 0x64, 0x1c, 0x62, 0xc3,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x4b, 0x00, 0x01, 0x4b, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xfa, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00, 0x01, 0x40, 0x00,
        0x00, 0x03, 0x47, 0x01, 0x47, 0xae, 0x00, 0x19, 0x9a, 0x00, 0x00, 0x00,
        0x00, 0x40, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00,
        0x00, 0x80, 0x00, 0x00, 0x0f, 0xff, 0x07, 0xc2, 0x8f, 0x00, 0x03, 0xe8,
        0x08, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x01, 0x01, 0x47, 0xae, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x19, 0x99, 0x9a, 0x00, 0x80, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x30,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x30, 0xec, 0x00, 0x00, 0x00, 0x03, 0xd7,
        0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00
};

unsigned int settings_Setup87_config_len = 165;

unsigned char settings_HQ_KS_13X18_DUMBO_preset[] = {
        0x00, 0x00, 0x07, 0x00, 0x01, 0x2c, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x1f, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 0x01, 0x47, 0xae,
        0x00, 0x2b, 0xb1, 0x00, 0x00, 0x9d, 0x00, 0x0d, 0x1b, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x0c, 0xcd, 0x00, 0x40, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x48,
        0x00, 0x01, 0x48, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0c, 0xcd,
        0x00, 0x00, 0x03
};

unsigned int settings_HQ_KS_13X18_DUMBO_preset_len = 87;

unsigned char settings_KS_13X18_DUMBO_speaker[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf4, 0x89, 0xff, 0xf2, 0xf4,
        0xff, 0xf9, 0xff, 0xff, 0xf7, 0xcb, 0xff, 0xfa, 0xc3, 0xff, 0xfa, 0xc8,
        0xff, 0xfb, 0xbc, 0xff, 0xfb, 0xa9, 0xff, 0xfc, 0x57, 0xff, 0xfb, 0x53,
        0xff, 0xfc, 0x38, 0xff, 0xfb, 0x4f, 0xff, 0xfb, 0xd5, 0xff, 0xfb, 0x6b,
        0xff, 0xfc, 0x45, 0xff, 0xfb, 0xb1, 0xff, 0xfc, 0x89, 0xff, 0xfc, 0x8d,
        0xff, 0xfc, 0x61, 0xff, 0xfd, 0x1a, 0xff, 0xfc, 0x30, 0xff, 0xfb, 0xbf,
        0xff, 0xfc, 0x5e, 0xff, 0xfa, 0xf6, 0xff, 0xfc, 0xcd, 0xff, 0xfb, 0x95,
        0xff, 0xfc, 0xa5, 0xff, 0xfb, 0xe5, 0xff, 0xfd, 0x09, 0xff, 0xfc, 0xf7,
        0xff, 0xfc, 0x6d, 0xff, 0xfc, 0xf4, 0xff, 0xfc, 0x4a, 0xff, 0xfc, 0x54,
        0xff, 0xfb, 0xea, 0xff, 0xfb, 0x3f, 0xff, 0xfc, 0xa2, 0xff, 0xfc, 0x50,
        0xff, 0xfb, 0xe9, 0xff, 0xfd, 0x66, 0xff, 0xfd, 0x0f, 0xff, 0xff, 0x42,
        0xff, 0xfe, 0xea, 0xff, 0xff, 0x61, 0xff, 0xff, 0x0e, 0xff, 0xfd, 0x33,
        0xff, 0xfc, 0x00, 0xff, 0xfb, 0x50, 0xff, 0xf8, 0xd9, 0xff, 0xf8, 0xd4,
        0xff, 0xf9, 0xf3, 0xff, 0xf9, 0xab, 0xff, 0xfe, 0xfe, 0x00, 0x00, 0x0c,
        0x00, 0x03, 0xcf, 0x00, 0x04, 0x4b, 0x00, 0x05, 0x07, 0x00, 0x01, 0x39,
        0xff, 0xfd, 0x80, 0xff, 0xf4, 0xff, 0xff, 0xf3, 0x34, 0xff, 0xeb, 0xa4,
        0xff, 0xf1, 0x06, 0xff, 0xf1, 0xaf, 0x00, 0x02, 0x1c, 0x00, 0x08, 0xe5,
        0x00, 0x1c, 0xe1, 0x00, 0x1a, 0x8e, 0x00, 0x22, 0x74, 0x00, 0x0c, 0xcf,
        0x00, 0x00, 0xfd, 0xff, 0xda, 0x59, 0xff, 0xcd, 0x2a, 0xff, 0xb0, 0x80,
        0xff, 0xc2, 0xf7, 0xff, 0xca, 0x40, 0x00, 0x08, 0x63, 0x00, 0x29, 0xd9,
        0x00, 0x7c, 0x85, 0x00, 0x7d, 0xb1, 0x00, 0xa8, 0x44, 0x00, 0x46, 0xe9,
        0x00, 0x2e, 0xcc, 0xff, 0x58, 0x94, 0xff, 0xc5, 0x98, 0x07, 0xa2, 0xd1,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x4a, 0x6b, 0x2a, 0x68, 0xf5, 0xc3, 0x26, 0x66, 0x66,
        0x26, 0x66, 0x66, 0x24, 0xcc, 0xcd, 0x19, 0x99, 0x9a, 0x00, 0x02, 0x8d,
        0x00, 0x02, 0x8d, 0x04, 0x00, 0x00, 0x00, 0x67, 0xae, 0x1c, 0xc0, 0x00,
        0x03, 0x7b, 0x4a
};

unsigned int settings_KS_13X18_DUMBO_speaker_len = 423;
