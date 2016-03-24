/**
 * @file tfa98xxLiveData.h
 *  @brief Life Data:
 *
 *    This code provides support and reference to read out the actual values from the Tfa98xx:
 *      -gain
 *      -output level
 *      -speaker temperature
 *      -speaker excursion
 *      -speaker resistance
 *      -various statuss flags.
 *
 *      It is similar to how the TFA9887 API is  used  to fill in the bulk of the traces and flags in the Pico GUI.
 *      This code will be implemented as a separate source file and added to climax.
 *
 *      These options will be added to record  the LiveData:
 *       - --record=msInterval
 *       - --count=count
 *       - --output=file.bin (TBS)  //TODO
 *
 *       If an output  file is specified the records will be stored with a header indicating interval, count and version.
 *       Else a line will be printed to the screen per record.
 *
 *  Created on: Jun 7, 2012
 *      Author: Wim Lemmers
 */

#ifndef TFA98XXLIFEDATA_H_
#define TFA98XXLIFEDATA_H_

#include "../inc/nxpTfa98xx.h"

#define NXPTFA_LIVE_REV_MAJOR    (0)         // major API rev
#define NXPTFA_LIVE_REV_MINOR    (1)         // minor

extern int tfa98xxLiveData_trace;
extern int tfa98xxLiveData_verbose;

/**
 * life data structure
 *  note the order is in line with the graphs on the pico GUI
 */
typedef struct nxpTfa98xx_LiveData {
        unsigned short statusRegister;/**  status flags (from i2c register ) */
        unsigned short statusFlags;   /**  Masked bit word, see Tfa98xx_SpeakerBoostStatusFlags */
        float agcGain;                /**  Current AGC Gain value */
        float limitGain;              /**  Current Limiter Gain value */
        float limitClip;              /**  Current Clip/Lim threshold */
        float batteryVoltage;         /**  battery level (from i2c register )*/
        int   speakerTemp;            /**  Current Speaker Temperature value */
        short icTemp;                 /**   Current ic/die Temperature value (from i2c register ) */
        float boostExcursion;         /**  Current estimated Excursion value caused by Speakerboost gain control */
        float manualExcursion;        /**  Current estimated Excursion value caused by manual gain setting */
        float speakerResistance;      /**  Current Loudspeaker blocked resistance */
} nxpTfa98xx_LiveData_t;

/** Speaker Model structure. */
/* All parameters related to the Speaker are collected into this structure*/
typedef struct SPKRBST_SpkrModel {
        double pFIR[128];       /* Pointer to Excurcussion  Impulse response or
                                   Admittance Impulse response (reversed order!!) */
        int Shift_FIR;          /* Exponent of HX data */
        float leakageFactor;    /* Excursion model integration leakage */
        float ReCorrection;     /* Correction factor for Re */
        float xInitMargin;      /*(1)Margin on excursion model during startup */
        float xDamageMargin;    /* Margin on excursion modelwhen damage has been detected */
        float xMargin;          /* Margin on excursion model activated when LookaHead is 0 */
        float Bl;               /* Loudspeaker force factor */
        int fRes;               /*(1)Estimated Speaker Resonance Compensation Filter cutoff frequency */
        int fResInit;           /* Initial Speaker Resonance Compensation Filter cutoff frequency */
        float Qt;               /* Speaker Resonance Compensation Filter Q-factor */
        float xMax;             /* Maximum excursion of the speaker membrane */
        float tMax;             /* Maximum Temperature of the speaker coil */
        float tCoefA;           /*(1)Temperature coefficient */
} SPKRBST_SpkrModel_t;          /* (1) this value may change dynamically */

/**
 * set the I2C slave base address and the nr of consecutive devices
 *  the devices will be opened //TODO check for conflicts if already open
 *  @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxOpenLiveDataSlaves(Tfa98xx_handle_t *handlesIn, int i2cbase, int maxdevices);

/**
 * print all information from the nxpTfa98xx_LiveData structure to a file
 *  @param open FILE pointer
 */
void tfa98xxPrintRecord(Tfa98xx_handle_t *handlesIn, FILE * csv, int idx);
void tfa98xxPrintRecordStereo(Tfa98xx_handle_t *handlesIn, FILE * csv);
/**
 * print a line with the  time value in ms and another line with
 * all fields from the nxpTfa98xx_LiveData structure.
 * @param open FILE pointer
 * @param time value to print a milliseconds
 */
void tfa98xxPrintRecordHeader(FILE * csv, int mstime);

/**
 * set the I2C slave base address and the nr of consecutive devices
 *  the devices will be opened //TODO check for conflicts if already open
 *  @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxLiveDataSlaves(int i2cbase, int maxdevices);

/**
 *  Get the live data information
 *  @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxGetLiveData( Tfa98xx_handle_t *handlesIn,
                                          int idx,
                                          nxpTfa98xx_LiveData_t * record);

/**
 *  Init dummy data
 *  @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxDummyDataInit(void);

/**
 *  Get the data information from dummy
 *  @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxGetDummyData( nxpTfa98xx_LiveData_t * record );

/**
 *  Get the speaker model
 *  @return last error code
 */
int nxpTfa98xxGetSpeakerModel(   Tfa98xx_handle_t *handlesIn,
                                 int xmodel,
                                 SPKRBST_SpkrModel_t *record,
                                 double waveformData[65],
                                 float frequency[65],
                                 int idx );
/**
 *  print the speaker model
 *  @return last error code
 */

/**
 *  retrieve the speaker model and retrun the buffer
 *  @param device handles
 *  @param the buffer pointer
 *  @param select 1 = xcursion model, 0 = impedance model
 *  @param device handle index
 *  @return last error code
 */
void tfa98xxGetRawSpeakerModel(Tfa98xx_handle_t *handlesIn,
							unsigned char *buffer, int xmodel, int idx);
void tfa98xxPrintSpeakerModel(Tfa98xx_handle_t *handlesIn,
                              FILE * csv, int xmodel, int idx);
/*
 *
 */
int tfa98xxLogger( int interval, int loopcount);
/**
 *  Set the speaker model
 *  @return last error code
 */
int nxpTfa98xxSetSpeakerModel(   Tfa98xx_handle_t *handlesIn,
                                 SPKRBST_SpkrModel_t *record,
                                 int idx );


int nxpTfa98xxSetVIsCalToConfig( Tfa98xx_handle_t *handlesIn,
                                    float VIsCal,
                                    int idx );

int nxpTfa98xxSetVsenseCalToConfig( Tfa98xx_handle_t *handlesIn,
                                    float Vsense,
                                    int idx );

int nxpTfa98xxSetIsenseCalToConfig( Tfa98xx_handle_t *handlesIn,
                                    float Isense,
                                    int idx );

int nxpTfa98xxGetVICalfromConfig(   Tfa98xx_handle_t *handlesIn,
                                    float *VIsCal,
                                    float *Vsense,
                                    float *Isense,
                                    int idx );

int nxpTfa98xxSetAgcGainMaxDBToPreset( Tfa98xx_handle_t *handlesIn,
                                       float agcGainMaxDB,
                                       int idx );

int nxpTfa98xxGetAgcGainMaxDBfromPreset(  Tfa98xx_handle_t *handlesIn,
                                          float *agcGainMaxDB,
                                          int idx );

// these functions are not yet implemented
#if 0
/**
 * return the structure with the required data record
 *  When Speaker Damage is detected  the error code will reflect this.
 *  @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxGetLiveData(nxpTfa98xx_LiveData_t * record);

/**
 * run for the amount of specified intervals (in milliseconds) and return the recorded data.
 *   (block until ready: count*msInterval msecs)
 *   @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxLogSpeakerStateInfo(int msInterval, int count,
                                                 nxpTfa98xx_LiveData_t **
                                                 record);

/**
 * call the recordSpeakerStateInfoCallback function every msInterval with the recorded data
 *  (this will start a thread and will stop when the callback returns 0)
 *  @return last error code
 */
nxpTfa98xx_Error_t nxpTfa98xxSendSpeakerStateInfo(int msInterval, int
                                                   (*recordSpeakerStateInfoCallback)
                                                   (nxpTfa98xx_LiveData_t *
                                                    record));
#endif                          //0

#endif                          /* TFA98XXLIFEDATA_H_ */
