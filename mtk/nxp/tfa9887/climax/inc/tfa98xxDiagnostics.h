/**
 * @file tfa98xxDiagnostics.h
 *  @brief Production diagnostics:
 *      simple exercise of the Maximus registers to ensure proper wiring and
 *      accessibility of the functionality of Maximus.
 *
 *     @brief  The following tests are available:
 *
 *              - test 0 : run all tests
 *              - test 1 : scan of I2C for ID registers and check for expected device
 *              - test 2 : write/read - test of a register
 *              - test 3 : check status register flags and assume coldstart (or fail)
 *              - test 4 : verify default state of relevant registers
 *              - test 5 : enable clocks in bypass and verify status
 *              - test 6 : start DSP and verify (by reading ROM tag and check status)
 *              - test 7 : load configuration settings and verify readback
 *              - test 8 : load preset values and verify readback
 *              - test 9 : load speaker parameters and verify readback
 *              - test 10 : check battery level
 *              - test 11 : verify speaker presence by checking the resistance
 *              - test 12 : assume I2S input and verify signal activity
 *
 *  Created on: Jun 7, 2012
 *      Author: Wim Lemmers
 */

#ifndef TFA98XXDIAGNOSTICS_H_
#define TFA98XXDIAGNOSTICS_H_

#define NXPTFA_DIAG_REV_MAJOR    (1)         // major API rev
#define NXPTFA_DIAG_REV_MINOR    (0)         // minor

extern int tfa98xxDiag_trace;
extern int tfa98xxDiag_verbose;

#define DIAGTRACE

/**
 * Diagnostic test description structure.
 */
typedef struct tfa98xxDiagTest {
        int (*function) (int);          /**< The function pointer of the test */
        const char *description;/**< single line description */
} tfa98xxDiagTest_t;

/**
 * run a testnumber
 *
 *  All the diagnostic test functions will return 0 if passed
 *  on failure the return value may contain extra info.
 * @param slave I2C slave under test
 * @param testnumber diagnostic test number
 * @return if testnumber is too big an empty string is returned
 */
int tfa98xxDiag(int slave, int testnumber);

/******************************************************************************
 *
 *    the test functions
 *
 *****************************************************************************/

/**
 * scan of I2C for ID registers and check for expected device
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa98xxDiagI2cScan(int slave);

/**
 * write/read test of a register that has no risk of causing damage
 *  - check for default value
 *  - write pattern and read verify
 *  .
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 RW pattern mismatch
 * @return 2 pwron default wrong
 */
int tfa98xxDiagI2cRw(int slave);

/**
 * check status register flags and assume coldstart (or fail)
 *  status register errorbits checked for 1
 *  - OCDS
 *  status register errorbits checked for 0
 *  - VDDS UVDS OVDS OTDS
 *  .
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 error bit found active
 * @return 2 not cold power on
 * @return 3 I2C slave not found or other internal errors
 */
int tfa98xxDiagStatusCold(int slave);

/**
 * verify default state of relevant registers
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 error bit found active
 * @return 2 I2C slave not found or other internal errors
 *
 */
int tfa98xxDiagRegisterDefaults(int slave);

/**
 * enable clocks in bypass
 *  - verify clock running
 *  - check error status bits
 *  .
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 clock not running
 * @return 2 error bit found active
 * @return 3 I2C slave not found or other internal errors
 *
 */
int tfa98xxDiagClock(int slave);

/**
 * start dsp and verify (by reading ROM tag and check status)
 * @param slave I2C slave under test
 * @return 1 DSP failure
 * @return 2 wrong DSP revtag
 * @return 3 I2C slave not found or other internal errors
 *
 */
int tfa98xxDiagDsp(int slave);

/**
 * load configuration settings and verify by reading back
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 DSP parameters mismatch
 * @return 2 I2C slave not found or other internal errors
 */
int tfa98xxDiagLoadConfig(int slave);

/**
 * load preset values and verify by reading back
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 DSP parameters mismatch
 * @return 2 I2C slave not found or other internal errors
 */
int tfa98xxDiagLoadPreset(int slave);

/**
 * load speaker parameters and verify by reading back
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 DSP parameters mismatch
 * @return 2 I2C slave not found or other internal errors
 */
int tfa98xxDiagLoadSpeaker(int slave);

/**
 * check battery level to be above 2Volts
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 battery level too low
 * @return 2 clock not running
 * @return 3 error bit found active
 * @return 4 I2C slave not found or other internal errors
 */
int tfa98xxDiagBattery(int slave);

/**
 * verify the presence of the speaker by checking the resistance for non-0
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 speaker not detected
 * @return 2 speaker parameters could not be loaded
 * @return 3 I2C slave not found or other internal errors
 */
int tfa98xxDiagSpeakerPresence(int slave);

/**
 * calibrate speaker and verify speaker presence check R range ,  verifies power from Vbat via DCDC to amplifier
 * @param slave I2C slave under test
 */
int tfa98xxDiagCalibration(int slave);

/**
 * assume I2S input and verify signal activity
 *  - initialize DSP
 *  - load speaker file
 *  - select I2S input
 *  - get DspSpeakerBoost StateInfo
 *  - check for audio active bit
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 audio not detected
 * @return 2 speaker parameters could not be loaded
 * @return 3 I2C slave not found or other internal errors

 */
int tfa98xxDiagI2sInput(int slave);
/**
 * for testing the I2S output an external receiver should acknowlege data presence
 * @param slave I2C slave under test
 */
int tfa98xxDiagI2sOutput(int slave);

/******************************************************************************
 *
 *    generic support functions
 *
 *****************************************************************************/

/**
 * dump all known registers
 *   returns:
 *     0 if slave can't be opened
 *     nr of registers displayed
 * @param slave I2C slave under test
 */
int tfa98xxDiagRegisterDump(int slave);

/**
 * run all tests
 * @param slave I2C slave under test
 * @return last error code
 */
int tfa98xxDiagAll(int slave);

/**
 * return the number of the test that was executed last
 * @param slave I2C slave under test
 * @return latest testnumber
 */
int tfa98xxDiagGetLatest(void);

/**
 * return the errorcode of the test that was executed last
 * @return last error code
 */
int tfa98xxDiagGetLastError(void);

/**
 * return the errorstring of the test that was executed last
 * @return last error string
 */
char *tfa98xxDiagGetLastErrorString(void);

/**
 * Return the single line test description.
 * @param testnumber
 * @return testname string
 * @return if testnumber is too big an empty string is returned
 */
const char *tfa98xxDiagGetTestNameString(int testnumber);

/**
 *  register definition structure
 */
typedef struct regdef {
        unsigned char offset;                   /**< subaddress offset */
        unsigned short pwronDefault;    /**< register contents after poweron */
        unsigned short pwronTestmask; /**< mask of bits not test */
        char *name;                                             /**< short register name */
} regdef_t;

extern regdef_t regdefs[];      // defined in tfa98xxDiagnostics.c

#endif                          /* TFA98XXDIAGNOSTICS_H_ */
