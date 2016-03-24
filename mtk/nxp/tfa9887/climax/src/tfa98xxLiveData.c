/*
 * tfa98xxLiveData.c
 *
 *  Created on: Jun 7, 2012
 *      Author: wim
 */
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "../inc/tfa98xxLiveData.h"
#include "../inc/nxpTfa98xx.h"


// globals
int tfa98xxLiveData_trace = 1;
int tfa98xxLiveData_verbose = 0;

static int maxdev = 0;
static unsigned char tfa98xxI2cbase;    // global for i2c access
static FILE *pFile;

#define I2C(idx) ((tfa98xxI2cbase+idx)*2)
#define PrintAssert(e) if ((e)) \
	fprintf(stderr, "PrintAssert:%s:%d %s\n",__FUNCTION__,__LINE__, Tfa98xx_GetErrorString(e))

static void dump_state_info(Tfa98xx_StateInfo_t * pState);
/*maxdev
 * set the I2C slave base address and the nr of consecutive devices
 *  the devices will be opened TODO check for conflicts if already open
 */
nxpTfa98xx_Error_t nxpTfa98xxOpenLiveDataSlaves(Tfa98xx_handle_t *handlesIn, int i2cbase, int maxdevices)
{
        Tfa98xx_Error_t err;
        int i;
        maxdev = maxdevices;

        if (maxdev < 1 | maxdev > 4)    // only allow one 1 up to 4
                return nxpTfa98xx_Error_BadParam;

        tfa98xxI2cbase = i2cbase;       // global for i2c access

        for (i = 0; i < maxdev; i++)
        {
           if( handlesIn[i] == -1)
           {
              err = Tfa98xx_Open(I2C(i), &handlesIn[i] );
	           PrintAssert( err);
           }
        }

        return nxpTfa98xx_Error_Ok;
}

/*
 * return the structure with the required data record
 *  When Speaker Damage is detected  the error code will reflect this.
 */
nxpTfa98xx_Error_t nxpTfa98xxGetLiveData( Tfa98xx_handle_t *handlesIn,
                                          int idx,
                                          nxpTfa98xx_LiveData_t * record)
{
        Tfa98xx_Error_t err;
        Tfa98xx_StateInfo_t stateInfo;
        unsigned char inforegs[6];
        short icTemp_2Complement;

        err = Tfa98xx_DspGetStateInfo(handlesIn[idx], &stateInfo);
        assert(err == Tfa98xx_Error_Ok);
        if (err == Tfa98xx_Error_Ok) {
                if (tfa98xxLiveData_verbose)
                        dump_state_info(&stateInfo);
        }
        record->statusFlags = stateInfo.statusFlag;
        record->limitClip = stateInfo.sMax;     /* Current Clip/Lim threshold */
        record->agcGain = stateInfo.agcGain;    /* Current AGC Gain value */
        record->limitGain = stateInfo.limGain;
        record->speakerTemp = stateInfo.T;
        record->boostExcursion = stateInfo.X1;  /* Current estimated Excursion value caused by Speakerboost gain control */
        record->manualExcursion = stateInfo.X2;
        record->speakerResistance = stateInfo.Re;
        // get regs
        err = Tfa98xx_ReadData(handlesIn[idx], 0, 6, inforegs);
        assert(err == Tfa98xx_Error_Ok);
        //
        record->statusRegister = inforegs[0] << 8 | inforegs[1];
        record->batteryVoltage = inforegs[2] << 8 | inforegs[3];
        record->batteryVoltage = (record->batteryVoltage * 5.5)/1024;

        icTemp_2Complement = inforegs[4] << 8 | inforegs[5];
        record->icTemp = (icTemp_2Complement >= 256) ? (icTemp_2Complement - 512) : icTemp_2Complement;


        return 0;
}

/*
 * return the structure with the required data record
 *  When Speaker Damage is detected  the error code will reflect this.
 */
nxpTfa98xx_Error_t nxpTfa98xxDummyDataInit(void)
{
   char line[256];
   pFile = fopen("record.csv","r");

   if (pFile == NULL)
   {
      return nxpTfa98xx_Error_BadParam;
   }

   /*skip the 1st line*/
	fgets( line , sizeof(line), pFile );

   return nxpTfa98xx_Error_Ok;
}

/*
 * return the structure with the required data record
 *  When Speaker Damage is detected  the error code will reflect this.
 */
nxpTfa98xx_Error_t nxpTfa98xxGetDummyData(nxpTfa98xx_LiveData_t * record )
{
   nxpTfa98xx_Error_t err = nxpTfa98xx_Error_BadParam;
   char line[256];
   int n = 0;
	float agcGain = 0;
   float limitGain = 0;
   float limitClip = 0;
	float boostExcursion = 0,manualExcursion = 0,speakerResistance = 0;
	unsigned int  batteryVoltage = 0,icTemp = 0;
	unsigned int statusFlags = 0,speakerTemp = 0,deltatime = 0,statusRegister = 0;

   /*skip the 1st line if end of file*/
	if ( feof(pFile) ) {
		rewind(pFile);
		fgets( line , sizeof(line), pFile );
	}

	fgets( line , sizeof(line), pFile );
	n = sscanf(line, "%hx,0x%4hx,%f,%f,%f,%f,%d,%d,%f,%f,%f",
            &statusRegister,
            &statusFlags, &agcGain, &limitGain, &limitClip,
            &batteryVoltage, &speakerTemp, &icTemp, &boostExcursion,
            &manualExcursion, &speakerResistance );
   if (n == 11)
   {
      record->statusFlags = statusFlags;
      record->limitClip = limitClip;     /* Current Clip/Lim threshold */
      record->agcGain = agcGain;    /* Current AGC Gain value */
      record->limitGain = limitGain;
      record->speakerTemp = speakerTemp;
      /* Current estimated Excursion value caused by Speakerboost gain control */
      record->boostExcursion = boostExcursion;
      record->manualExcursion = manualExcursion;
      record->speakerResistance = speakerResistance;
      record->statusRegister = statusRegister;
      record->batteryVoltage = batteryVoltage;
      record->icTemp = icTemp;
   }
   else
   {
      return nxpTfa98xx_Error_Fail;
   }

   return nxpTfa98xx_Error_Ok;
}

/*
 * run for the amount of specified intervals (in milliseconds) and return the recorded data.
 *   (block until ready: count*msInterval msecs)
 */
nxpTfa98xx_Error_t nxpTfa98xxLogSpeakerStateInfo(int msInterval, int count,
                                                 nxpTfa98xx_LiveData_t **
                                                 record)
{
        return 1;
}

/*
 * call the recordSpeakerStateInfoCallback function every msInterval with the recorded data
 *  (this will start a thread and will stop when the callback returns 0)
 */
nxpTfa98xx_Error_t nxpTfa98xxSendSpeakerStateInfo(int msInterval,
                                                  int
                                                  (*recordSpeakerStateInfoCallback)
                                                  (nxpTfa98xx_LiveData_t *
                                                   record))
{
        return 1;
}

static char *stateFlagsStr(int stateFlags)
{
        static char flags[10];

        flags[0] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_Activity)) ? 'A' : 'a';
        flags[1] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_S_Ctrl)) ? 'S' : 's';
        flags[2] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_Muted)) ? 'M' : 'm';
        flags[3] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_X_Ctrl)) ? 'X' : 'x';
        flags[4] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_T_Ctrl)) ? 'T' : 't';
        flags[5] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_NewModel)) ? 'L' : 'l';
        flags[6] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_VolumeRdy)) ? 'V' : 'v';
        flags[7] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_Damaged)) ? 'D' : 'd';
        flags[8] =
            (stateFlags & (0x1 << Tfa98xx_SpeakerBoost_SignalClipping)) ? 'C' :
            'c';

        flags[9] = 0;
        return flags;
}

static void dump_state_info(Tfa98xx_StateInfo_t * pState)
{
        printf
            ("state: flags %s, agcGain %2.1f\tlimGain %2.1f\tsMax %2.1f\tT %d\tX1 %2.1f\tX2 %2.1f\tRe %2.2f\n",
             stateFlagsStr(pState->statusFlag), pState->agcGain,
             pState->limGain, pState->sMax, pState->T, pState->X1, pState->X2,
             pState->Re);
}

void tfa98xxPrintRecordHeader(FILE * csv, int deltatime)
{
        fprintf(csv, "recording interval time: %d ms\n", deltatime);
        fprintf(csv, "statusRegister,"
                "statusFlags,"
                "agcGain,"
                "limitGain,"
                "limitClip,"
                "batteryVoltage,"
                "speakerTemp,"
                "icTemp,"
                "boostExcursion," "manualExcursion," "speakerResistance" "\n");
}

/*
 *  data logger for life time testing
 */
typedef enum
{
	tfaLeft,
	tfaRight
} side_t;
typedef enum
{
	tfaZmodel,
	tfaXmodel
} model_t;
#define MODELBUFSIZE (423+5) //32bits linenr + 0x55 + modeldata
static void tfa98xxLogModel(Tfa98xx_handle_t *handlesIn, side_t idx,
			int currentLine, FILE *fp, model_t type)
{
	int actual;
   unsigned char buffer[MODELBUFSIZE];
	*(unsigned int*)buffer= currentLine;


	buffer[4] = 0x55; //marker

	tfa98xxGetRawSpeakerModel(handlesIn, &buffer[5], type, idx); //X=1 Z=0
	actual = fwrite( buffer, 1, sizeof(buffer), fp);
	assert(actual==MODELBUFSIZE);

}
static int tfa98xxLogStateLine(Tfa98xx_handle_t *handlesIn, side_t idx, int currentline, FILE *fp)
{
	nxpTfa98xx_LiveData_t record;
    nxpTfa98xxGetLiveData(handlesIn, idx, &record);
    fprintf(fp, "%d,0x%04x,0x%04x,%f,%f,%f,%d,%d,%d,%f,%f,%f\n", currentline,
    		record.statusRegister,       //x
            record.statusFlags,     //x Masked bit word, see Tfa98xxStatusFlags
            record.agcGain, //f Current AGC Gain value
            record.limitGain,       //f Current Limiter Gain value
            record.limitClip,       //f Current Clip/Lim threshold
            record.batteryVoltage,  //d
            record.speakerTemp,     //d Current Speaker Temperature value
            record.icTemp,  //d Current ic/die Temperature value
            record.boostExcursion,  //f Current estimated Excursion value caused by Speakerboost gain control
            record.manualExcursion, //f Current estimated Excursion value caused by manual gain setting
            record.speakerResistance        //f Current Loudspeaker blocked resistance
        );
    return 0;
}
/*
 * stateinfo: 01L_0000.CSV 01R_0000.CSV
 * xmodels:	  01L_XMDL.BIN 01R_XMDL.BIN
 * zmodels:	  01L_ZMDL.BIN 01R_ZMDL.BIN
 */
#define MODELLOGINTERVAL (30)
int tfa98xxLogger( int interval, int loopcount)
{
	Tfa98xx_handle_t handlesIn[] ={-1, -1};
	int modelinterval = interval*MODELLOGINTERVAL;
	int currentline=1, modelcount=0, i, j;
	char logline[128];
	// logfiles
	FILE *fp[6], *out[2];
//	FILE  *gStateInfoLeft, *gStateInfoRight;
//	FILE *gZmodelLeft,*gZmodelRight, *gXmodelLeft, *gXmodelRight;
	char *fname[]={ "01L_0000.CSV" ,"01R_0000.CSV",
				   "01L_XMDL.BIN" ,"01R_XMDL.BIN",
				   "01L_ZMDL.BIN" ,"01R_ZMDL.BIN" };

	/*
	 * print info:
	 *   state info interval = interval secs
	 *   total lines = loopcount
	 *   total time = loopcount*interval secs /60=mins/3600=hours
	 *   model log time = every MODELLOGINTERVAL lines = modelinterval secs
	 *
	 */
	out[0] = stdout;
	out[1] = fopen("RUN.LOG", "a");
	for (i=0;i<2;i++) {
		fprintf(out[i], "data logger starting:\n");
		fprintf(out[i], " state info interval = %d seconds\n", interval);
		fprintf(out[i], " total lines = %d\n", loopcount);
		fprintf(out[i], " total time = %d seconds = %.2f minutes = %.2f hours\n",
				loopcount*interval, (float)(loopcount*interval)/60, (float)(loopcount*interval)/3600);
		fprintf(out[i], " model log time every %d lines = %d seconds\n",
				MODELLOGINTERVAL ,modelinterval);
		fprintf(out[i], " files:");
		for (j=0; j<6; j++) {
			fprintf(out[i]," %s", fname[j]);
		}
		fprintf(out[i], "\n");

		// open files
		for (i=0; i<6; i++) {
			fp[i] = fopen( fname[i], "a+b");
			if ( fp[i] == NULL) {
				fprintf(stderr, "can't open logfile:%s\n", fname[i]);
				return 0;
			}
		}
	}
	fflush(out[1]);
	fclose(out[1]);

	if (  nxpTfa98xx_Error_Ok == nxpTfa98xxOpenLiveDataSlaves(handlesIn, tfa98xxI2cSlave, 2))
        do {


        	tfa98xxLogStateLine(handlesIn, tfaLeft, currentline, fp[0]);// left
        	tfa98xxLogStateLine(handlesIn, tfaRight, currentline, fp[1]);// right
        	if (currentline > modelinterval*modelcount) {
        		//do model
        		modelcount++;
        		//
        		tfa98xxLogModel( handlesIn, tfaLeft,  currentline, fp[2], tfaZmodel);
        		tfa98xxLogModel( handlesIn, tfaRight, currentline, fp[3], tfaZmodel);
        		tfa98xxLogModel( handlesIn, tfaLeft,  currentline, fp[4], tfaXmodel);
        		tfa98xxLogModel( handlesIn, tfaRight, currentline, fp[5], tfaXmodel);
        		printf("line:%d, model:%d\n", currentline, modelcount);
        	} else printf("line:%d\n", currentline);
        	currentline++;
            loopcount = ( loopcount == 0) ? 1 : loopcount-1 ;
#ifdef WIN32
			   Sleep(interval*1000); // is seconds interval
#else
            usleep(interval*1000000); // is seconds interval
#endif
        } while (loopcount>0) ;

	// close files
	for (i=0; i<6; i++)
		fclose(fp[i]);

	return currentline;

}
void tfa98xxPrintRecord(Tfa98xx_handle_t *handlesIn, FILE * csv, int idx)
{
        nxpTfa98xx_Error_t error = nxpTfa98xx_Error_Ok;
        nxpTfa98xx_LiveData_t record;

        error = nxpTfa98xxGetLiveData(handlesIn, idx, &record);
        fprintf(csv, "0x%04x,0x%04x,%f,%f,%f,%f,%d,%d,%f,%f,%f\n", record.statusRegister,       //x
                record.statusFlags,     //x Masked bit word, see Tfa98xx_SpeakerBoostStatusFlags
                record.agcGain, //f Current AGC Gain value
                record.limitGain,       //f Current Limiter Gain value
                record.limitClip,       //f Current Clip/Lim threshold
                record.batteryVoltage,  //f
                record.speakerTemp,     //d Current Speaker Temperature value
                record.icTemp,  //d Current ic/die Temperature value
                record.boostExcursion,  //f Current estimated Excursion value caused by Speakerboost gain control
                record.manualExcursion, //f Current estimated Excursion value caused by manual gain setting
                record.speakerResistance        //f Current Loudspeaker blocked resistance
            );
}

void tfa98xxPrintRecordStereo(Tfa98xx_handle_t *handlesIn, FILE * csv)
{
   nxpTfa98xx_Error_t error = nxpTfa98xx_Error_Ok;
   nxpTfa98xx_LiveData_t record;
   int i = 0;

   for (i = 0; i < 2; i++)
   {
      printf("Device %d\n", i);
      error = nxpTfa98xxGetLiveData(handlesIn, i, &record);
      fprintf(csv, "0x%04x,0x%04x,%f,%f,%f,%f,%d,%d,%f,%f,%f\n", record.statusRegister,       //x
                record.statusFlags,     //x Masked bit word, see Tfa98xx_SpeakerBoostStatusFlags
                record.agcGain, //f Current AGC Gain value
                record.limitGain,       //f Current Limiter Gain value
                record.limitClip,       //f Current Clip/Lim threshold
                record.batteryVoltage,  //f
                record.speakerTemp,     //d Current Speaker Temperature value
                record.icTemp,  //d Current ic/die Temperature value
                record.boostExcursion,  //f Current estimated Excursion value caused by Speakerboost gain control
                record.manualExcursion, //f Current estimated Excursion value caused by manual gain setting
                record.speakerResistance        //f Current Loudspeaker blocked resistance
            );
   }
}


typedef int int24;
typedef float fix;
#define MIN(a,b) ((a)<(b)?(a):(b))
static void convertData2Bytes(int num_data, const int24 data[],
                              unsigned char bytes[])
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

/* convert DSP memory bytes to signed 24 bit integers
   data contains "num_bytes/3" elements
   bytes contains "num_bytes" elements */
static void convertBytes2Data(int num_bytes, const unsigned char bytes[],
                              int24 data[])
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

/////////////////////////////////////////////////////////
// Sorensen in-place split-radix FFT for real values
// data: array of doubles:
// re(0),re(1),re(2),...,re(size-1)
//
// output:
// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
// normalized by array length
//
// Source:
// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
void realfft_split(double *data, long n)
{
        long i, j, k, i5, i6, i7, i8, i0, id, i1, i2, i3, i4, n2, n4, n8;
        double t1, t2, t3, t4, t5, t6, a3, ss1, ss3, cc1, cc3, a, e, sqrt2;
        float pi = 3.1415926f;
        sqrt2 = sqrt(2.0);
        n4 = n - 1;
//data shuffling
        for (i = 0, j = 0, n2 = n / 2; i < n4; i++) {
                if (i < j) {
                        t1 = data[j];
                        data[j] = data[i];
                        data[i] = t1;
                }
                k = n2;
                while (k <= j) {
                        j -= k;
                        k >>= 1;
                }
                j += k;
        }
    /*----------------------*/
//length two butterflies
        i0 = 0;
        id = 4;
        do {
                for (; i0 < n4; i0 += id) {
                        i1 = i0 + 1;
                        t1 = data[i0];
                        data[i0] = t1 + data[i1];
                        data[i1] = t1 - data[i1];
                }
                id <<= 1;
                i0 = id - 2;
                id <<= 1;
        } while (i0 < n4);
    /*----------------------*/
//L shaped butterflies
        n2 = 2;
        for (k = n; k > 2; k >>= 1) {
                n2 <<= 1;
                n4 = n2 >> 2;
                n8 = n2 >> 3;
                e = 2 * pi / (n2);
                i1 = 0;
                id = n2 << 1;
                do {
                        for (; i1 < n; i1 += id) {
                                i2 = i1 + n4;
                                i3 = i2 + n4;
                                i4 = i3 + n4;
                                t1 = data[i4] + data[i3];
                                data[i4] -= data[i3];
                                data[i3] = data[i1] - t1;
                                data[i1] += t1;
                                if (n4 != 1) {
                                        i0 = i1 + n8;
                                        i2 += n8;
                                        i3 += n8;
                                        i4 += n8;
                                        t1 = (data[i3] + data[i4]) / sqrt2;
                                        t2 = (data[i3] - data[i4]) / sqrt2;
                                        data[i4] = data[i2] - t1;
                                        data[i3] = -data[i2] - t1;
                                        data[i2] = data[i0] - t2;
                                        data[i0] += t2;
                                }
                        }
                        id <<= 1;
                        i1 = id - n2;
                        id <<= 1;
                } while (i1 < n);
                a = e;
                for (j = 2; j <= n8; j++) {
                        a3 = 3 * a;
                        cc1 = cos(a);
                        ss1 = sin(a);
                        cc3 = cos(a3);
                        ss3 = sin(a3);
                        a = j * e;
                        i = 0;
                        id = n2 << 1;
                        do {
                                for (; i < n; i += id) {
                                        i1 = i + j - 1;
                                        i2 = i1 + n4;
                                        i3 = i2 + n4;
                                        i4 = i3 + n4;
                                        i5 = i + n4 - j + 1;
                                        i6 = i5 + n4;
                                        i7 = i6 + n4;
                                        i8 = i7 + n4;
                                        t1 = data[i3] * cc1 + data[i7] * ss1;
                                        t2 = data[i7] * cc1 - data[i3] * ss1;
                                        t3 = data[i4] * cc3 + data[i8] * ss3;
                                        t4 = data[i8] * cc3 - data[i4] * ss3;
                                        t5 = t1 + t3;
                                        t6 = t2 + t4;
                                        t3 = t1 - t3;
                                        t4 = t2 - t4;
                                        t2 = data[i6] + t6;
                                        data[i3] = t6 - data[i6];
                                        data[i8] = t2;
                                        t2 = data[i2] - t3;
                                        data[i7] = -data[i2] - t3;
                                        data[i4] = t2;
                                        t1 = data[i1] + t5;
                                        data[i6] = data[i1] - t5;
                                        data[i1] = t1;
                                        t1 = data[i5] + t4;
                                        data[i5] -= t4;
                                        data[i2] = t1;
                                }
                                id <<= 1;
                                i = id - n2;
                                id <<= 1;
                        } while (i < n);
                }
        }
// energy component is disabled
//division with array length
        // for (i = 0; i < n; i++)
        //     data[i] /= n;
}

void leakageFilter(double *data, int cnt, double lf)
{
        int i;
        double phi;
        float pi = 3.1415926f;

        for (i = 0; i < cnt; i++) {
                phi = sin(0.5 * pi * i / (cnt));
                phi *= phi;
                data[i] = sqrt(pow(1 + lf, 2) - 4 * lf * phi);
        }
}

/**
* Untangle the real and imaginary parts and take the modulus
*/
void untangle(double *waveform, double *hAbs, int L)
{
        int i, cnt = L / 2;

        hAbs[0] =
            sqrt((waveform[0] * waveform[0]) +
                 (waveform[L / 2] * waveform[L / 2]));
        for (i = 1; i < cnt; i++)
                hAbs[i] =
                    sqrt((waveform[i] * waveform[i]) +
                         (waveform[L - i] * waveform[L - i]));

}

/**
* Untangle the real and imaginary parts and take the modulus
*  and divide by the leakage filter
*/
void untangle_leakage(double *waveform, double *hAbs, int L, double leakage)
{
        int i, cnt = L / 2;
        double m_Aw[64];

        // create the filter
        leakageFilter(m_Aw, cnt, leakage);
        //
        hAbs[0] =
            sqrt((waveform[0] * waveform[0]) +
                 (waveform[L / 2] * waveform[L / 2])) / m_Aw[0];
        for (i = 1; i < cnt; i++)
                hAbs[i] =
                    sqrt((waveform[i] * waveform[i]) +
                         (waveform[L - i] * waveform[L - i])) / m_Aw[i];
}

#define PARAM_GET_LSMODEL           0x86        // Gets current LoudSpeaker impedance Model.
#define PARAM_GET_LSMODELW        0xC1  // Gets current LoudSpeaker xcursion Model.
int nxpTfa98xxGetSpeakerModel(   Tfa98xx_handle_t *handlesIn,
                                 int xmodel,
                                 SPKRBST_SpkrModel_t *record,
                                 double waveformData[65],
                                 float frequency[65],
                                 int idx )
{
   Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
   unsigned char bytes[3 * 141];
   int24 data[141];
   int i = 0;
   double fft[128], untangled[128];

   error = Tfa98xx_DspGetParam(handlesIn[idx], 1,
                               xmodel ? PARAM_GET_LSMODELW :
                               PARAM_GET_LSMODEL, 423, bytes);
   assert(error == Tfa98xx_Error_Ok);

   convertBytes2Data(sizeof(bytes), bytes, data);

   for (i = 0; i < 128; i++)
   {
      /*record->pFIR[i] = (double)data[i] / ((1 << 23) * 2);*/
      record->pFIR[i] = (double)data[i] / (1 << 22);
   }

   record->Shift_FIR = data[i++];   ///< Exponent of HX data
   record->leakageFactor = (float)data[i++] / (1 << (23));  ///< Excursion model integration leakage
   record->ReCorrection = (float)data[i++] / (1 << (23));   ///< Correction factor for Re
   record->xInitMargin = (float)data[i++] / (1 << (23 - 2));        ///< (can change) Margin on excursion model during startup
   record->xDamageMargin = (float)data[i++] / (1 << (23 - 2));      ///< Margin on excursion modelwhen damage has been detected
   record->xMargin = (float)data[i++] / (1 << (23 - 2));    ///< Margin on excursion model activated when LookaHead is 0
   record->Bl = (float)data[i++] / (1 << (23 - 2)); ///< Loudspeaker force factor
   record->fRes = data[i++];        ///< (can change) Estimated Speaker Resonance Compensation Filter cutoff frequency
   record->fResInit = data[i++];    ///< Initial Speaker Resonance Compensation Filter cutoff frequency
   record->Qt = (float)data[i++] / (1 << (23 - 6)); ///< Speaker Resonance Compensation Filter Q-factor
   record->xMax = (float)data[i++] / (1 << (23 - 7));       ///< Maximum excursion of the speaker membrane
   record->tMax = (float)data[i++] / (1 << (23 - 9));       ///< Maximum Temperature of the speaker coil
   record->tCoefA = (float)data[i++] / (1 << 23);   ///< (can change) Temperature coefficient

   if (xmodel)
   {
      //xmodel 0xc1
      for (i = 0; i < 128; i++)
      {
         fft[127 - i] = (double)data[i] / (1 << 22);
      }
      realfft_split(fft, 128);
      untangle_leakage(fft, untangled, 128, -record->leakageFactor);
      for (i = 0; i < 128 / 2; i++)
      {
         waveformData[i] = 2 * untangled[i];
      }
   }
   else
   {
      //zmodel 0x86
      for (i = 0; i < 128; i++)
      {
         fft[i] = (double)data[i] / (1 << 22);
      }
      realfft_split(fft, 128);
      untangle(fft, untangled, 128);
      for (i = 0; i < 128 / 2; i++)
      {
         waveformData[i] = 1 / untangled[i];
      }
   }

   frequency[0] = 0;
   for (i = 1; i < 65; i++)
   {
      frequency[i] = frequency[i-1] + 62.5;
   }

   return error;
}

int nxpTfa98xxSetSpeakerModel(   Tfa98xx_handle_t *handlesIn,
                                 SPKRBST_SpkrModel_t *record,
                                 int idx )
{
   Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
   unsigned char bytes[3 * 141];
   int24 data[141];
   int i = 0;

   assert(error == Tfa98xx_Error_Ok);

   for (i = 0; i < 128; i++)
   {
      data[i] = record->pFIR[i] * (1 << 22);
   }

   data[i++] = record->Shift_FIR; /*Exponent of HX data*/
   data[i++] = record->leakageFactor * (1 << (23)); /*Excursion model integration leakage*/
   data[i++] = record->ReCorrection * (1 << (23)); /*Correction factor for Re*/
   data[i++] = record->xInitMargin * (1 << (23 - 2)); /*(can change) Margin on excursion model during startup*/
   data[i++] = record->xDamageMargin * (1 << (23 - 2)); /*Margin on excursion modelwhen damage has been detected*/
   data[i++] = record->xMargin * (1 << (23 - 2)); /*Margin on excursion model activated when LookaHead is 0*/
   data[i++] = record->Bl * (1 << (23 - 2)); /*Loudspeaker force factor*/
   data[i++] = record->fRes; /*(can change) Estimated Speaker Resonance Compensation Filter cutoff frequency*/
   data[i++] = record->fResInit; /*Initial Speaker Resonance Compensation Filter cutoff frequency*/
   data[i++] = record->Qt * (1 << (23 - 6)); /*Speaker Resonance Compensation Filter Q-factor*/
   data[i++] = record->xMax * (1 << (23 - 7)); /*Maximum excursion of the speaker membrane*/
   data[i++] = record->tMax * (1 << (23 - 9)); /*Maximum Temperature of the speaker coil*/
   data[i++] = record->tCoefA * (1 << 23); /*(can change) Temperature coefficient*/

   convertData2Bytes(141, data, bytes);

   error = Tfa98xx_DspWriteSpeakerParameters(handlesIn[idx], TFA98XX_SPEAKERPARAMETER_LENGTH, (unsigned char*) bytes);

   return error;
}

void tfa98xxGetRawSpeakerModel(Tfa98xx_handle_t *handlesIn,
							unsigned char *buffer, int xmodel, int idx)
{
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

    error = Tfa98xx_DspGetParam(handlesIn[idx], 1,
                                xmodel ? PARAM_GET_LSMODELW : PARAM_GET_LSMODEL,
                                423, buffer);
    assert(error == Tfa98xx_Error_Ok);

}
void tfa98xxPrintSpeakerModel(Tfa98xx_handle_t *handlesIn, FILE * csv, int xmodel, int idx)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        SPKRBST_SpkrModel_t record;
        unsigned char bytes[3 * 141];
        int24 data[141];
        int i, hz;
        double fft[128], waveform[128], untangled[128];

        error = Tfa98xx_DspGetParam(handlesIn[idx], 1,
                                    xmodel ? PARAM_GET_LSMODELW :
                                    PARAM_GET_LSMODEL, 423, bytes);
        assert(error == Tfa98xx_Error_Ok);

        convertBytes2Data(sizeof(bytes), bytes, data);

        for (i = 0; i < 128; i++) {
                record.pFIR[i] = (double)data[i] / (1 << 22);
        }

        record.Shift_FIR = data[i++];   ///< Exponent of HX data
        record.leakageFactor = (float)data[i++] / (1 << (23));  ///< Excursion model integration leakage
        record.ReCorrection = (float)data[i++] / (1 << (23));   ///< Correction factor for Re
        record.xInitMargin = (float)data[i++] / (1 << (23 - 2));        ///< (can change) Margin on excursion model during startup
        record.xDamageMargin = (float)data[i++] / (1 << (23 - 2));      ///< Margin on excursion modelwhen damage has been detected
        record.xMargin = (float)data[i++] / (1 << (23 - 2));    ///< Margin on excursion model activated when LookaHead is 0
        record.Bl = (float)data[i++] / (1 << (23 - 2)); ///< Loudspeaker force factor
        record.fRes = data[i++];        ///< (can change) Estimated Speaker Resonance Compensation Filter cutoff frequency
        record.fResInit = data[i++];    ///< Initial Speaker Resonance Compensation Filter cutoff frequency
        record.Qt = (float)data[i++] / (1 << (23 - 6)); ///< Speaker Resonance Compensation Filter Q-factor
        record.xMax = (float)data[i++] / (1 << (23 - 7));       ///< Maximum excursion of the speaker membrane
        record.tMax = (float)data[i++] / (1 << (23 - 9));       ///< Maximum Temperature of the speaker coil
        record.tCoefA = (float)data[i++] / (1 << 23);   ///< (can change) Temperature coefficient

        if (xmodel) {
                //xmodel 0xc1
                for (i = 0; i < 128; i++)
                {
                   fft[127 - i] =(double)data[i] / (1 << 22);
                }
                realfft_split(fft, 128);
                untangle_leakage(fft, untangled, 128, -record.leakageFactor);
                for (i = 0; i < 128 / 2; i++)
                        waveform[i] = 2 * untangled[i];
        } else {
                //zmodel 0x86
                for (i = 0; i < 128; i++)
                        fft[i] = (double)data[i] / (1 << 22);
                realfft_split(fft, 128);
                untangle(fft, untangled, 128);
                for (i = 0; i < 128 / 2; i++)
                        waveform[i] = 1 / untangled[i];
        }

        fprintf(stdout,
                "leakageFactor,"
                "ReCorrection,"
                "xInitMargin,"
                "xDamageMargin,"
                "xMargin,"
                "Bl," "fRes," "fResInit," "Qt," "xMax," "tMax," "tCoefA\n");

//        fprintf(csv, "%f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%.2f,%.4f\n",
        fprintf(stdout,
                "%f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%.2f,%.4f\n",
                //                record.Shift_FIR,        ///< Exponent of HX data: always 0
                record.leakageFactor,   ///< Excursion model integration leakage
                record.ReCorrection,    ///< Correction factor for Re
                record.xInitMargin,     ///< * Margin on excursion model during startup
                record.xDamageMargin,   ///< Margin on excursion modelwhen damage has been detected
                record.xMargin, ///< Margin on excursion model activated when LookaHead is 0
                record.Bl,      ///< Loudspeaker force factor
                record.fRes,    ///< * Estimated Speaker Resonance Compensation Filter cutoff frequency
                record.fResInit,        ///< Initial Speaker Resonance Compensation Filter cutoff frequency
                record.Qt,      ///< Speaker Resonance Compensation Filter Q-factor
                record.xMax,    ///< Maximum excursion of the speaker membrane
                record.tMax,    ///< Maximum Temperature of the speaker coil
                record.tCoefA   ///< * beetje Temperature coefficient
            );

//        fprintf(csv,"%s_rawdata, pFIR,fftin, fft, untangled, waveform\n", xmodel?"x":"z");
//        for(i=0;i<128;i++){
//            fprintf(csv, "0x%03x,%f,%f,%f,%f,%f\n", data[i], record.pFIR[i], fftin[i],  fft[i], untangled[i], waveform[i]);
//            //                fprintf(csv, "%f, %f,n", record.pFIR[i], waveform[i], (unsigned int)data[127-i] );
//        }
        fprintf(csv, "Hz, %s\n", xmodel ? "xcursion mm/V" : "impedance Ohm");
        hz = xmodel ? 62.5 : 40;
        i = xmodel ? 1 : 0;
        for (; i < 64; i++) {
                fprintf(csv, "%d,%f\n", hz, waveform[i]);
                hz += 62.5;
        }

        if (tfa98xxLiveData_verbose) {
                printf("FIR: 0x%03x\n", data[0]);
        }

}

int nxpTfa98xxSetVIsCalToConfig( Tfa98xx_handle_t *handlesIn,
                                 float VIsCal,
                                 int idx )
{
	int iVIsCal = 0;
   Tfa98xx_Config_t pParameters = {0};
   Tfa98xx_Error_t err;

	iVIsCal =(int)(VIsCal*(1<<(23-7)));

   err = Tfa98xx_DspReadConfig(handlesIn[idx], TFA98XX_CONFIG_LENGTH, pParameters);

	pParameters[0] = (iVIsCal>>16)&0xFF;
	pParameters[1] = (iVIsCal>>8)&0xFF;
	pParameters[2] = (iVIsCal)&0xFF;

   err = Tfa98xx_DspWriteConfig(handlesIn[idx], TFA98XX_CONFIG_LENGTH, pParameters);

   return err;
}

int nxpTfa98xxSetVsenseCalToConfig( Tfa98xx_handle_t *handlesIn,
                                    float Vsense,
                                    int idx )
{
	int iVsense = 0;
   Tfa98xx_Config_t pParameters = {0};
   Tfa98xx_Error_t err;

	iVsense =(int)(Vsense*(1<<(23-10)));

   err = Tfa98xx_DspReadConfig(handlesIn[idx], TFA98XX_CONFIG_LENGTH, pParameters);

	pParameters[3] = (iVsense>>16)&0xFF;
	pParameters[4] = (iVsense>>8)&0xFF;
	pParameters[5] = (iVsense)&0xFF;

   err = Tfa98xx_DspWriteConfig(handlesIn[idx], TFA98XX_CONFIG_LENGTH, pParameters);

   return err;
}

int nxpTfa98xxSetIsenseCalToConfig( Tfa98xx_handle_t *handlesIn,
                                    float Isense,
                                    int idx )
{
	int iIsense = 0;
   Tfa98xx_Config_t pParameters = {0};
   Tfa98xx_Error_t err;

	iIsense =(int)(Isense*(1<<(23-10)));

   err = Tfa98xx_DspReadConfig(handlesIn[idx], TFA98XX_CONFIG_LENGTH, pParameters);

	pParameters[6] = (iIsense>>16)&0xFF;
	pParameters[7] = (iIsense>>8)&0xFF;
	pParameters[8] = (iIsense)&0xFF;

   err = Tfa98xx_DspWriteConfig(handlesIn[idx], TFA98XX_CONFIG_LENGTH, pParameters);

   return err;
}

int nxpTfa98xxGetVICalfromConfig(   Tfa98xx_handle_t *handlesIn,
                                    float *VIsCal,
                                    float *Vsense,
                                    float *Isense,
                                    int idx )
{
   unsigned char bytes[3 * 55];
   int24 data[55];
   Tfa98xx_Error_t err;

   err = Tfa98xx_DspReadConfig(handlesIn[idx], TFA98XX_CONFIG_LENGTH, bytes);

   convertBytes2Data(sizeof(bytes), bytes, data);

	*VIsCal = (float)data[0] / (1 << (23 - 7));
   *Vsense = (float)data[1] / (1 << (23 - 10));
   *Isense = (float)data[2] / (1 << (23 - 10));

   return err;
}

int nxpTfa98xxSetAgcGainMaxDBToPreset( Tfa98xx_handle_t *handlesIn,
                                       float agcGainMaxDB,
                                       int idx )
{
	int gainMax = 0;
   unsigned char bytes[252] = {0};
   Tfa98xx_Preset_t pParameters = {0};
   Tfa98xx_Error_t err;
   int i = 0;

	gainMax =(int)(agcGainMaxDB*(1<<(23-8)));

   err = Tfa98xx_DspReadPreset(handlesIn[idx], (TFA98XX_CONFIG_LENGTH+TFA98XX_PRESET_LENGTH), bytes);

   for (i=0; i<87; i++)
   {
      pParameters[i] = bytes[i+165];
   }
	pParameters[39] = (gainMax>>16)&0xFF;
	pParameters[40] = (gainMax>>8)&0xFF;
	pParameters[41] = (gainMax)&0xFF;

   err = Tfa98xx_DspWritePreset(handlesIn[idx], TFA98XX_PRESET_LENGTH, pParameters);

   return err;
}

int nxpTfa98xxGetAgcGainMaxDBfromPreset(  Tfa98xx_handle_t *handlesIn,
                                          float *agcGainMaxDB,
                                          int idx )
{
   unsigned char bytes[252];
   int24 data[84];
   Tfa98xx_Error_t err;

   err = Tfa98xx_DspReadPreset(handlesIn[idx], (TFA98XX_CONFIG_LENGTH+TFA98XX_PRESET_LENGTH), bytes);

   convertBytes2Data(sizeof(bytes), bytes, data);

	*agcGainMaxDB = (float)data[68] / (1 << (23 - 8));

   return err;
}

