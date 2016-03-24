/*
 * dummy i2c sandbox
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
//#include <linux/stat.h>
#include <inttypes.h>	//TODO fix/converge types
#include <fcntl.h>
#include "../../../climax/inc/i2c-dev.h"

// TFA9887 API
#include "../../../Tfa98xx/inc/Tfa9887.h"
#include "../../../Tfa98xx/inc/Tfa9887_Registers.h"
#include "../../../Tfa98xxhal/inc/lxScribo.h"
#include "../../../Tfa98xx/inc/Tfa98xx.h"	// for i2c slave address

#include "../../../climax/inc/tfa98xxDiagnostics.h" // for regdefs

static uint8_t dummySlave=0x36;
// globals
int lxDummy_trace = 0;
int lxDummy_verbose = 0;
int lxDummyFailTest;	// nr of the test to fail
char *lxDummyArg; //extra command line arg for test settings
extern int cli_verbose;
#define WIMVERBOSE if (cli_verbose || lxDummy_verbose)

// endian swap
#define BE2LEW( x )   (( ( (x) << 8 ) | ( (x) & 0xFF00 ) >> 8 )&0xFFFF)

#define BE2LEDW( x )  (\
               (   (x) << 24 ) \
             | ( ( (x) & 0x0000FF00 ) << 8 ) \
             | ( ( (x) & 0x00FF0000 ) >> 8 ) \
             | (   (x) >> 24 ) \
             )

unsigned char stateInfo[]={0x01, 0x03, 0xe4, 0x08, 0x00, 0x00, 0x04, 0x6e, 0xb1, 0x1c,
			0xc0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xa7, 0x00, 0x03, 0x9f, 0x00, 0x00, 0x00};
unsigned char lsmodel[], lsmodelw[];
// local buffers
Tfa9887_Config_t lastConfig;
Tfa9887_Preset_t lastPreset;
Tfa9887_SpeakerParameters_t lastSpeaker;

static void setStateInfo(Tfa9887_SpeakerBoost_StateInfo_t* pInfo, unsigned char *bytes);
static void makeStateInfo( float agcGain , float limGain, float sMax,
							int T, int statusFlag,
							float X1, float X2, float Re);
/*
 * in the local register cache the values are stored as little endian,
 *  all processing is done in natural little endianess
 * The i2c data is big endian
 */
static uint16_t *regptr,Reg[256];

uint8_t pmem[1024*16*3];
uint8_t xmem[8192*3]={0,0,0,'1','2','3','4','1','2','3','4','1','2','3','4','1','2','3','4'};
uint8_t ymem[2048*3];
uint8_t iomem[1024*64*3];
int memIdx; // set via cf mad

// actual default
//0x00: 0x091d (statusreg)
//0x03: 0x0012 (revisionnumber)
//0x04: 0x888b (i2sreg)
//0x05: 0x13aa (bat_prot)
//0x06: 0x001f (audio_ctr)
//0x07: 0x0fe6 (dcdcboost)
//0x08: 0x0800 (spkr_calibration)
//0x09: 0x041d (sys_ctrl)
//0x0a: 0x3ec3 (i2s_sel_reg)
//0x40: 0x0000 (hide_unhide_key)
//0x4c: 0x0000 (abisttest)
//0x70: 0x0000 (cf_controls)
//0x71: 0x0000 (cf_mad)
//0x72: 0x0000 (cf_mem)
//0x73: 0x00ff (cf_status)
static FILE *infile;
/*
 * get state info from file and wrap around
 */
static int getStateInfo(void)
{
	int n;
	float agcGain,limitGain,limitClip,
	boostExcursion,manualExcursion,speakerResistance;
	unsigned int  batteryVoltage,icTemp;
	unsigned int statusFlags,speakerTemp,deltatime,statusRegister;
	char line[256];

	if (infile==0)
		return -1;

	if ( feof(infile) ) {
		rewind(infile);
		fgets( line , sizeof(line), infile ); //skip 1st line
	}

	fgets( line , sizeof(line), infile );
	n = sscanf(line, "%hx,0x%4hx,%f,%f,%f,%d,%d,%d,%f,%f,%f",
            &statusRegister,
            &statusFlags, &agcGain, &limitGain, &limitClip,
            &batteryVoltage, &speakerTemp, &icTemp, &boostExcursion,
            &manualExcursion, &speakerResistance );
  //  printf("%x >%s\n",statusRegister,line);

	if ( 11==n ){

		makeStateInfo(agcGain, limitGain, limitClip, speakerTemp,
				statusFlags, boostExcursion, manualExcursion, speakerResistance);
		Reg[0] = statusRegister;
		Reg[1] = batteryVoltage;
		Reg[2] = icTemp;
		return 0;
	}
	return 1;

}
/*
 * set the input file for stat info input
 */
static int setInputFile(char *file)
{
    char line[256];

	infile = fopen(file, "r");

	if (infile==0)
		return 0;

	fgets( line , sizeof(line), infile ); //skip 1st line

	return 1;
}
/*
 *
 */
static void resetRegs(void)
{
    Reg[0x00] =  0x081d ;    /* statusreg */
    Reg[0x03] =  0x0012 ;    /* revisionnumber */
    Reg[0x04] =  0x888b ;    /* i2sreg */
    Reg[0x05] =  0x13aa ;    /* bat_prot */
    Reg[0x06] =  0x001f ;    /* audio_ctr */
    Reg[0x07] =  0x0fe6 ;    /* dcdcboost */
    Reg[0x08] =  0x0800 ;    /* spkr_calibration */
    Reg[0x09] =  0x041d ;    /* sys_ctrl */
    Reg[0x0a] =  0x3ec3 ;    /* i2s_sel_reg */
    Reg[0x40] =  0x0000 ;    /* hide_unhide_key */
    Reg[0x41] =  0x0000 ;    /* pwm_control */
    Reg[0x4c] =  0x0000 ;    /* abisttest */
    Reg[0x62] =  0x0000 ;
    Reg[0x70] =  0x0000 ;    /* cf_controls */
    Reg[0x71] =  0x0000 ;    /* cf_mad */
    Reg[0x72] =  0x0000 ;    /* cf_mem */
    Reg[0x73] =  0x00ff ;    /* cf_status */
    Reg[0x80] =  0x0000 ;
}
/*
 * for debugging
 */
int lxDummyInit(char *file)
{

        int fd;

        if (lxDummyArg) {
        	if ( !setInputFile(lxDummyArg) ) // if filename use it
        		lxDummyFailTest = atoi(lxDummyArg);
        }
        WIMVERBOSE printf("arg: %s\n", lxDummyArg);

    	printf("I2C slave=0x%02x\n", dummySlave);
    	resetRegs();
    	Reg[0x00] =  0x091d ;    /* statusreg mtp bit is only on after real pwron */
    	xmem[231*3] = 1; //calibration done
    	// lsmodel default
    	 memcpy(lastSpeaker, lsmodel, sizeof(Tfa9887_SpeakerParameters_t));
    	// fail tests
        switch (lxDummyFailTest) {
    	case 1:
    		dummySlave=0; // no devices
    		break;
    	case 2:
    		//
    		break;
    	case 3:
    		//set error bit
    		Reg[0] |= TFA9887_STATUS_OCDS;
    		break;
    	case 4:
    		//set wrong default
    		Reg[TFA9887_SYS_CTRL] = 0xdead;
    		break;
    	case 5:
    		//clocks
    		break;
    	case 10:
    		Reg[TFA9887_BATTERYVOLTAGE] = (unsigned short)1/(5.5/1024); //1V
    		break;
    	case 11:
    		//xmem[231*3] = 0; //calibration not done
    		break;
        }
//        Reg[0]=0x805f;
//        Reg[0x73]=0x1ff;
        makeStateInfo(1,2,3,4,5,6,7,8);
        xmem[0x2210*3]    = 0x73; /* N1D2 */
        xmem[0x2210*3+1]=  0x33;
        xmem[0x2210*3+2]=  0x33;

	return 9887;
}
int dummy_trace=0;

static void hexdump(int num_write_bytes, const unsigned char * data)
{
	int i;

	for(i=0;i<num_write_bytes;i++)
	{
		printf("0x%02x ", data[i]);
	}

}

/* modules */
#define MODULE_SPEAKERBOOST  1

/* RPC commands */
#define PARAM_SET_LSMODEL        0x06  // Load a full model into SpeakerBoost.
#define PARAM_SET_LSMODEL_SEL    0x07  // Select one of the default models present in Tfa9887 ROM.
#define PARAM_SET_EQ             0x0A  // 2 Equaliser Filters.
#define PARAM_SET_PRESET         0x0D  // Load a preset
#define PARAM_SET_CONFIG         0x0E  // Load a config

#define PARAM_GET_RE0            0x85  /* gets the speaker calibration impedance (@25 degrees celsius) */
#define PARAM_GET_LSMODEL        0x86  // Gets current LoudSpeaker Model.
#define PARAM_GET_LSMODELW        0xC1  // Gets current LoudSpeaker excursion Model.
#define PARAM_GET_ALL           0x80 // read current config and preset.
#define PARAM_GET_STATE                  0xC0
#define PARAM_GET_TAG           0xFF

/* RPC Status results */
#define STATUS_OK                  0
#define STATUS_INVALID_MODULE_ID   2
#define STATUS_INVALID_PARAM_ID    3
#define STATUS_INVALID_INFO_ID     4

//reg73
//cf_err[7:0]     8   [7 ..0] 0           cf error flags
//reg73 cf_ack[7:0]     8   [15..8] 0           acknowledge of requests (8 channels")"

#define 	CTL_CF_RST_DSP 	(0)
#define 	CTL_CF_DMEM 	(1)
#define 	CTL_CF_AIF 		(3)
#define    CTL_CF_INT 		(4)
#define    CTL_CF_REQ 		(5)
#define    STAT_CF_ERR		(0)
#define    STAT_CF_ACK		(8)
#define    CF_PMEM			(0)
#define    CF_XMEM			(1)
#define    CF_YMEM			(2)
#define    CF_IOMEM			(3)

char *cfmemName[]={"pmem", "xmem", "ymem", "iomem"};
int lxDummyMemw(int  type, const uint8_t *data)
{
	uint8_t *memptr;
	int idx = memIdx;
	WIMVERBOSE
			printf("W %s[%02d]: 0x%02x 0x%02x 0x%02x\n", cfmemName[type], idx/3, data[0], data[1], data[2]);
	switch(type) {
		case CF_PMEM:
			idx &= 64*1024-sizeof(pmem); // mask off high bits
			if (idx<sizeof(pmem))
				memptr=&pmem[idx];
			else {
				printf("!pmem address too big: max=%d, mad=%d\n", (int)sizeof(pmem)/3, idx/3);
			}
			break;
		case CF_XMEM:
			memptr=&xmem[idx];
			break;
		case CF_YMEM:
			memptr=&ymem[idx];
			break;
		case CF_IOMEM:
			memptr=&iomem[idx];
			break;
	}
    *memptr++ = *data++;
    *memptr++ = *data++;
    *memptr++ = *data++;
	 memIdx+=3;
	return 3; //TODO 3
}
int lxDummyMemr(int  type, uint8_t *data)
{
	uint8_t *memptr;
	int idx = memIdx;

	switch(type) {
		case CF_PMEM:
			memptr=&pmem[idx];
			break;
		case CF_XMEM:
			memptr=&xmem[idx];
			break;
		case CF_YMEM:
			memptr=&ymem[idx];
			break;
		case CF_IOMEM:
			memptr=&iomem[idx];
			break;
	}
	WIMVERBOSE
			printf("R %s[%02d]: 0x%02x 0x%02x 0x%02x\n", cfmemName[type], idx/3, memptr[0], memptr[1], memptr[2]);

	  *data++ = *memptr++;
	  *data++ = *memptr++;
	  *data++ = *memptr++;
//    *data++ =0;
//    *data++ =0;
//    *data++ =0;

	 memIdx+=3;

	 return 3; //TODO 3
}

//0x00 : 0x091d
//0x04 < 0x880b
//0x00 : 0x091d
//0x09 < 0x0219
//0x00 : 0x091d
//0x09 < 0x0618
//0x00 : 0xd85f

//int lxDummyEmulate(uint8_t reg, uint16_t data)
//{
//
//	WIMVERBOSE
//	switch(reg){
//	case 0x70: printf("<cf_ctl=0x%04x>",data); //cf_req[15:8]=0, cf_int[4]=0, cf_aif[3]=0, cf_dmem[2:1]=XMEM=01, cf_rst_dsp[0]=0
//		break;
//	case 0x71: printf("<cf_mad=0x%04x>",data); //memory address to be accessed (0 : Status, 1 : ID, 2 : parameters
//		break;
//	case 0x72: printf("<cf_mem>"); //data
//		break;
//	case 0x73: printf("<cf_status>"); // cfg_err[8:15] cfg_ack[7:0]
//		break;
//	}
//
//	return 0;
//}

static int i2cWrite( int length, const uint8_t *data);
static int i2cRead( int length, uint8_t *data);

uint8_t currentreg; // active register

static const char *getRegname(int reg)
{
	int i;

	for(i=0;i<256;i++) {
		if (regdefs[i].name==NULL)
			break;
		if ( reg == regdefs[i].offset )
			return regdefs[i].name;
	}
	return "unknown";
}
/*
 * emulation of tfa9887 register read
 */
static int tfaRead(uint8_t *data) {
	int reglen = 2; //default
	uint8_t reg = currentreg;
	uint16_t regval;
	static short temperature=0;

	switch (reg) {
		case TFA9887_STATUSREG /*0x00*/:
		case TFA9887_REVISIONNUMBER /*0x03*/:
		case TFA9887_I2SREG /*0x04*/:
		case TFA9887_BAT_PROT /*0x05*/:
		case TFA9887_AUDIO_CTR /*0x06*/:
		case TFA9887_DCDCBOOST /*0x07*/:
		case TFA9887_SPKR_CALIBRATION /*0x08*/:
		case TFA9887_SYS_CTRL /*0x09*/:
		case TFA9887_I2S_SEL_REG /*0x0a*/:
		case TFA9887_RESERVE1 /*0x0c*/:
		case TFA9887_RESERVE2 /*0x0d*/:
		case TFA9887_HIDE_UNHIDE_KEY /*0x40*/:
		case TFA9887_PWM_CONTROL /*0x41*/:
		case TFA9887_CURRENTSENSE1 /*0x46*/:
		case TFA9887_CURRENTSENSE2 /*0x47*/:
		case TFA9887_CURRENTSENSE3 /*0x48*/:
		case TFA9887_CURRENTSENSE4 /*0x49*/:
		case TFA9887_ABISTTEST /*0x4c*/:
		case TFA9887_CF_CONTROLS /*0x70*/:
		case TFA9887_MTP_COPY /*(0x62)*/:
		case TFA9887_MTP /*(0x80)*/:
		case 0x52://TODO
		case 0xb: //TODO
			regval = Reg[reg]; // just return
            reglen = 2;
            currentreg++; // autoinc
            break;
        case TFA9887_TEMPERATURE /*0x02*/:
            if ( temperature++ > 170)
                temperature=-40;
            Reg[TFA9887_TEMPERATURE] = temperature;
            regval = Reg[reg]; // just return
            reglen = 2;
            currentreg++; // autoinc
            break;
		case TFA9887_BATTERYVOLTAGE /*0x01*/:
			if (lxDummyFailTest==10)
				Reg[TFA9887_BATTERYVOLTAGE] = (unsigned short)1/(5.5/1024); //1V
			regval = Reg[reg]; // just return
            reglen = 2;
            currentreg++; // autoinc
            break;
		case TFA9887_CF_STATUS /*0x73*/:
			regval = Reg[reg]; // just return
			reglen = 2;
			currentreg++; // autoinc
			break;
		case TFA9887_CF_MEM /*0x72*/:
			reglen = lxDummyMemr( (Reg[TFA9887_CF_CONTROLS]>>CTL_CF_DMEM)&0x03, data);
			break;
		case TFA9887_CF_MAD /*0x71*/:
			regval = Reg[reg]; // just return
			if (lxDummyFailTest==2)
				regval = 0xdead; //fail test
            reglen = 2;
            currentreg++; // autoinc
            break;
		default:
			printf("undefined rd register: 0x%02x\n", reg);
			regval = Reg[reg]; // just return anyway
			currentreg++; // autoinc
			break;
	}


	if (reg!=TFA9887_CF_MEM) {
	  WIMVERBOSE
		printf("0x%02x:0x%04x (%s)\n", reg, regval, getRegname(reg));

	    *(uint16_t*) (data) = BE2LEW (regval); // return in proper endian
	}

	return reglen;

}

#define STATE_SIZE             8
/* convert DSP memory bytes to signed 24 bit integers
   data contains "num_bytes/3" elements
   bytes contains "num_bytes" elements */
static void convert24Data2Bytes(int num_data, unsigned char bytes[], int data[])
{
	int i; /* index for data */
	int k; /* index for bytes */
	int d;
	int num_bytes = num_data*3;


	for (i=0, k=0; i<num_data; ++i, k+=3)
	{
		*bytes = 0xff & (data[i]>>16);
		if ( data[i]<0)
			*bytes++ |= 0x80; //sign
		else
			*bytes++;
		*bytes++ = 0xff & (data[i]>>8);
		*bytes++ = 0xff & (data[i]);
	}
}
//from Tfa9887_internals.h
#define SPKRBST_HEADROOM			 7										/* Headroom applied to the main input signal */
#define SPKRBST_AGCGAIN_EXP			SPKRBST_HEADROOM	  /* Exponent used for AGC Gain related variables */
#define SPKRBST_TEMPERATURE_EXP     9
#define SPKRBST_LIMGAIN_EXP			    4					     /* Exponent used for Gain Corection related variables */
#define SPKRBST_TIMECTE_EXP         1

static void setStateInfo(Tfa9887_SpeakerBoost_StateInfo_t* pInfo, unsigned char *bytes)
{
	int data[STATE_SIZE];

	data[0] = (int)roundf(pInfo->agcGain*(1<<(23-SPKRBST_AGCGAIN_EXP)));
	data[1] = (int)roundf(pInfo->limGain*(1<<(23-SPKRBST_LIMGAIN_EXP)));
	data[2] = (int)roundf(pInfo->sMax*(1<<(23-SPKRBST_HEADROOM)));
	data[3] = pInfo->T*(1<<(23-SPKRBST_TEMPERATURE_EXP));
	data[4] = pInfo->statusFlag;
	data[5] = (int)roundf(pInfo->X1*(1<<(23-SPKRBST_HEADROOM)));
	data[6] = (int)roundf(pInfo->X2*(1<<(23-SPKRBST_HEADROOM)));
	data[7] = (int)roundf(pInfo->Re*(1<<(23-SPKRBST_TEMPERATURE_EXP)));

	convert24Data2Bytes(STATE_SIZE, bytes, data);

}
static void makeStateInfo( float agcGain , float limGain, float sMax,
							int T, int statusFlag,
							float X1, float X2, float Re)
{
	Tfa9887_SpeakerBoost_StateInfo_t Info;

    Info.agcGain = agcGain;
    Info.limGain = limGain;
    Info.sMax = sMax;
    Info.T = T;
    Info.statusFlag = statusFlag;
    Info.X1 = X1;
    Info.X2 = X2;
    Info.Re = Re;

    setStateInfo(&Info, stateInfo);

}
#define TFA9887_MAXTAG              (138)
#define DSP_revstring        "< Dec 21 2011 - 12:33:16 -  SpeakerBoostOnCF >"
// fill xmem return values
static int setDspParamsXmem( int param) {
    int i,j;
    uint8_t *ptr;
    char *tag=DSP_revstring;
    /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters) */
    switch ( param)
    {
    case 0xff: //tag
        ptr = &xmem[6+2];
        for(i=0,j=0;i<TFA9887_MAXTAG;i++,j+=3) {
            ptr[j] = tag[i];   //24 bits, byte[2]
        }
        if (lxDummyFailTest==6) ptr[0]='!'; //fail
        //   *pRpcStatus = (mem[0]<<16) | (mem[1]<<8) | mem[2];
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        break;
    case PARAM_SET_CONFIG:
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        memcpy( lastConfig, &xmem[6], sizeof(Tfa9887_Config_t));
        if (lxDummyFailTest==7) lastConfig[0] = ~lastConfig[0];
        break;
    case PARAM_SET_PRESET:
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        memcpy( lastPreset, &xmem[6], sizeof(Tfa9887_Preset_t));
        if (lxDummyFailTest==8)
        	lastPreset[1] = ~lastPreset[1];
        break;
    case PARAM_SET_LSMODEL:
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        memcpy( lastSpeaker, &xmem[6], sizeof(Tfa9887_SpeakerParameters_t));
        break;
    case PARAM_GET_LSMODEL:
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        if (lxDummyFailTest!=9)
            memcpy(&xmem[6], lastSpeaker,  sizeof(Tfa9887_SpeakerParameters_t));
        else
            bzero(&xmem[6], sizeof(Tfa9887_SpeakerParameters_t));
        break;
    case PARAM_GET_LSMODELW: // for now just return the speakermodel
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        memcpy(&xmem[6], lsmodelw,  sizeof(Tfa9887_SpeakerParameters_t));
        break;
    case PARAM_GET_ALL:
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        memcpy(&xmem[6], lastConfig,  sizeof(Tfa9887_Config_t));
        memcpy(&xmem[6+sizeof(Tfa9887_Config_t)], lastPreset,  sizeof(Tfa9887_Preset_t));
        break;
    case PARAM_GET_STATE:
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        getStateInfo();
        if (lxDummyFailTest!=12)
        	memcpy(&xmem[6], stateInfo,  sizeof(stateInfo));
        else
        	bzero(&xmem[6], sizeof(Tfa9887_SpeakerParameters_t));
        break;

    case PARAM_GET_RE0:
        xmem[0]=0;
        xmem[1]=0;
        xmem[2]=0;
        //        i2cExecuteRS W   2: 0x6c 0x72
        //        i2cExecuteRS R   4: 0x6d  0x01 0xc6 0x90
        xmem[6]= 0x01;
        xmem[7]= 0xc6;
        xmem[8]= 0x90;

        if (lxDummyFailTest==11) {
            xmem[6]= 0;
            xmem[7]= 0;
            xmem[8]= 0;
        }
        break;
    }

    return i-1;
}
typedef unsigned short unit16_t;
static int cfControlReg( unit16_t val)
{ unsigned char code1, code2;

    if ( val & (1<<CTL_CF_INT) ) // if irq
    {
        code1 = xmem[4];
        code2 = xmem[5];
        if ( code1 == 0x81 )
            setDspParamsXmem( code2);
        Reg[TFA9887_CF_STATUS] |= (1<<STAT_CF_ACK); // set ack
    }

    return val;
}
/*
 * emulation of tfa9887 register write
 *
 *  write current register , autoincrement  and return bytes consumed
 *
 */
static int tfaWrite( const uint8_t *data)
{
	int reglen=2; //default
	uint8_t reg=currentreg;
	uint16_t wordvalue;

	wordvalue = data[0]<<8 | data[1];
	switch (reg) {
		case TFA9887_STATUSREG /*0x00*/:
		case TFA9887_BATTERYVOLTAGE /*0x01*/:
		case TFA9887_TEMPERATURE /*0x02*/:
		case TFA9887_REVISIONNUMBER /*0x03*/:
		case TFA9887_I2SREG /*0x04*/:
		case TFA9887_BAT_PROT /*0x05*/:
		case TFA9887_AUDIO_CTR /*0x06*/:
		case TFA9887_DCDCBOOST /*0x07*/:
		case TFA9887_SPKR_CALIBRATION /*0x08*/:
		case TFA9887_I2S_SEL_REG /*0x0a*/:
		case TFA9887_RESERVE1 /*0x0c*/:
		case TFA9887_RESERVE2 /*0x0d*/:
		case TFA9887_HIDE_UNHIDE_KEY /*0x40*/:
		case TFA9887_PWM_CONTROL /*0x41*/:
		case TFA9887_CURRENTSENSE1 /*0x46*/:
		case TFA9887_CURRENTSENSE2 /*0x47*/:
		case TFA9887_CURRENTSENSE3 /*0x48*/:
		case TFA9887_CURRENTSENSE4 /*0x49*/:
		case TFA9887_ABISTTEST /*0x4c*/:
		case TFA9887_CF_STATUS /*0x73*/:
		case TFA9887_MTP_COPY /*(0x62)*/:
		case TFA9887_MTP /*(0x80)*/:
        case 0x52://TODO
        case 0xb: //TODO
            Reg[reg]=  wordvalue;
			break;
		case TFA9887_CF_MAD /*0x71*/:
			Reg[reg]=  wordvalue;
			memIdx = wordvalue*3; //set glbl mem idx
			break;
		case TFA9887_CF_CONTROLS /*0x70*/:	 // cf_req=00000000, cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0
			Reg[reg]=   cfControlReg (wordvalue);
			break;
		case TFA9887_CF_MEM /*0x72*/:
			reglen = lxDummyMemw( (Reg[TFA9887_CF_CONTROLS]>>CTL_CF_DMEM)&0x03, data);
			break;
		case TFA9887_SYS_CTRL /*0x09*/:
			if ( wordvalue & (1<<1)) //I2CR reset
				resetRegs();
			if (lxDummyFailTest!=5) {
				if ( (wordvalue & (1<<0)) && !(wordvalue & (1<<13)) ){//powerdown=1, i2s input 1
					Reg[TFA9887_STATUSREG] &=  ~(TFA9887_STATUS_PLLS | TFA9887_STATUS_CLKS);
					Reg[TFA9887_BATTERYVOLTAGE] =0;
				} else {
					Reg[TFA9887_STATUSREG] |=  (TFA9887_STATUS_PLLS | TFA9887_STATUS_CLKS);
					Reg[TFA9887_BATTERYVOLTAGE] =0x300;
				}
			}
			Reg[reg]=  wordvalue;
			break;
		default:
			printf("undefined wr register: 0x%02x\n", reg);
			Reg[reg]=  wordvalue;
			break;
	}

	/* all but cf_mem autoinc and 2 bytes */
	if (reg!=TFA9887_CF_MEM) {
		currentreg++; // autoinc
		reglen = 2;

		WIMVERBOSE
			printf("0x%02x<0x%04x (%s)\n", reg, wordvalue, getRegname(reg));
	}
	return reglen;
}

/*
 * read I2C
 */
static int i2cRead(int length, uint8_t *data) {
	uint8_t slave; //
	int idx, reglen;

	slave = data[0] / 2;
	if (slave != dummySlave) {
		fprintf(stderr, "dummy slave read NoAck\n");
		return 0;
	}

	//  currentreg  : start subaddress

//ln =length - 1;  // without slaveaddress
	idx = 1;
	while (idx < length) {
		idx += tfaRead(&data[idx]); //write current and return bytes consumed
	};

	return length;
}


/*
 * write i2c
 */
static int i2cWrite(int length, const uint8_t *data) {
	uint8_t slave; //
	int idx, reglen;

	slave = data[0] / 2;
	if (slave != dummySlave) {
		fprintf(stderr, "dummy slave NoAck\n");
		return 0;
	}

	currentreg = data[1]; // start subaddress

	 // without slaveaddress and regaddr
	idx = 2;
	while (idx < length) {
		idx += tfaWrite(&data[idx]); //write current and return bytes consumed
	};

	return length;

}

int lxDummyWriteRead(int fd, int NrOfWriteBytes, const uint8_t * WriteData,
		int NrOfReadBytes, uint8_t * ReadData) {
	uint8_t buf[256],reg;
	int length,i;

	// there's always a write
	length = i2cWrite(NrOfWriteBytes, WriteData);
	// and maybe a read
	if ((NrOfReadBytes!=0) && (length!=0) )
		length = i2cRead( NrOfReadBytes, ReadData);

	if ( length<0 ) {
		fprintf(stderr, "lxDummy slave error\n");
		return 0;
	}

	return length;

//	if (WriteData[0] !=  (tfa98xxI2cSlave<<1)) {
//		printf ("wrong slave 0x%02x iso 0x%02x\n", WriteData[0]>>1, tfa98xxI2cSlave);
//	//	return 0;
//	}
}

int lxDummyWrite(int fd, int size, uint8_t *buffer)
{
	uint8_t cmd[5], *ptr, slave,term;
	int length,status, total=0;

	return lxDummyWriteRead( fd, size, buffer, 0, NULL);

}


// life data models
// ls model (0x86)
unsigned char lsmodel[] = { 0xff, 0xfd, 0xda, 0xff, 0xfe, 0x4d, 0xff, 0xfe,
		0x5e, 0xff, 0xfe, 0x22, 0xff, 0xfe, 0xdd, 0xff, 0xfd, 0x64, 0xff, 0xff,
		0x03, 0xff, 0xfe, 0x17, 0xff, 0xfc, 0x9c, 0xff, 0xff, 0x05, 0xff, 0xfd,
		0x14, 0xff, 0xfe, 0x0e, 0xff, 0xfb, 0xa6, 0xff, 0xfe, 0x80, 0xff, 0xfe,
		0x05, 0x00, 0x00, 0x4b, 0xff, 0xfe, 0xa7, 0xff, 0xfe, 0x2d, 0xff, 0xfe,
		0x15, 0xff, 0xfd, 0xa1, 0xff, 0xfd, 0xff, 0xff, 0xfd, 0x23, 0xff, 0xfd,
		0x67, 0xff, 0xfc, 0x8c, 0xff, 0xff, 0x64, 0xff, 0xfd, 0x43, 0xff, 0xfd,
		0xfd, 0xff, 0xfd, 0xbd, 0xff, 0xfd, 0x33, 0xff, 0xfc, 0x49, 0xff, 0xff,
		0x2f, 0xff, 0xfb, 0x31, 0x00, 0x00, 0x1b, 0xff, 0xfc, 0xed, 0xff, 0xff,
		0xa9, 0xff, 0xfb, 0x1c, 0x00, 0x00, 0x7e, 0xff, 0xf9, 0x9c, 0xff, 0xfe,
		0xf3, 0xff, 0xfc, 0x16, 0xff, 0xfd, 0x5d, 0xff, 0xfd, 0xd3, 0xff, 0xfd,
		0xd3, 0xff, 0xfb, 0x75, 0xff, 0xff, 0x77, 0xff, 0xfb, 0xcd, 0xff, 0xfe,
		0x79, 0xff, 0xfb, 0x3a, 0xff, 0xfc, 0x28, 0xff, 0xfd, 0x43, 0xff, 0xfb,
		0xca, 0xff, 0xfd, 0xa4, 0xff, 0xfb, 0xdb, 0xff, 0xfd, 0xad, 0xff, 0xfc,
		0x48, 0xff, 0xfd, 0x39, 0xff, 0xfa, 0x51, 0xff, 0xfd, 0x00, 0xff, 0xfc,
		0x05, 0xff, 0xfc, 0x5a, 0xff, 0xfb, 0xda, 0xff, 0xfb, 0xf4, 0xff, 0xfb,
		0x82, 0xff, 0xfe, 0xb2, 0xff, 0xfb, 0x20, 0xff, 0xfd, 0x31, 0xff, 0xfc,
		0x41, 0xff, 0xfb, 0x5c, 0xff, 0xfb, 0x5a, 0xff, 0xfb, 0x65, 0xff, 0xfb,
		0x9c, 0xff, 0xfc, 0xc8, 0xff, 0xfc, 0xf4, 0xff, 0xfd, 0xfd, 0xff, 0xfa,
		0xdb, 0xff, 0xfd, 0x98, 0xff, 0xf8, 0xdd, 0xff, 0xfa, 0x87, 0xff, 0xfa,
		0xfa, 0xff, 0xfc, 0x3f, 0xff, 0xfc, 0xd5, 0xff, 0xfc, 0xea, 0xff, 0xff,
		0x22, 0xff, 0xfc, 0xa1, 0xff, 0xfb, 0x57, 0xff, 0xf6, 0xe0, 0xff, 0xf7,
		0x18, 0xff, 0xf5, 0xf4, 0xff, 0xfb, 0x10, 0xff, 0xfa, 0xf3, 0x00, 0x06,
		0xda, 0x00, 0x02, 0x98, 0x00, 0x08, 0x24, 0xff, 0xf9, 0x8f, 0xff, 0xf9,
		0x06, 0xff, 0xe1, 0x97, 0xff, 0xef, 0x74, 0xff, 0xe0, 0x02, 0x00, 0x05,
		0x51, 0x00, 0x01, 0x07, 0x00, 0x2f, 0x18, 0x00, 0x13, 0xce, 0x00, 0x2e,
		0x58, 0xff, 0xde, 0x37, 0xff, 0xea, 0x91, 0xff, 0x8f, 0xff, 0xff, 0xcc,
		0x0d, 0xff, 0x9e, 0xf0, 0x00, 0x40, 0x7c, 0x00, 0x24, 0x43, 0x00, 0xda,
		0x5a, 0x00, 0x4b, 0xa9, 0x00, 0xad, 0x10, 0xff, 0x3a, 0x6e, 0x00, 0x5c,
		0x45, 0x06, 0x81, 0x0d, 0xff, 0x9e, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x33, 0x33, 0x79, 0x99,
		0x9a, 0x20, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x00, 0x18, 0xf5,
		0xc3, 0x00, 0x03, 0x2a, 0x00, 0x03, 0x2a, 0x04, 0x00, 0x00, 0x00, 0x67,
		0xae, 0x1c, 0xc0, 0x00, 0x00, 0x01, 0x88 };

// ls model (0xc1)
unsigned char lsmodelw[] = { 0x01, 0x01, 0x92, 0x03, 0x51, 0xff, 0xff, 0x0c,
		0x36, 0x02, 0x09, 0xa2, 0xfe, 0x37, 0x8d, 0xff, 0x38, 0xb9, 0xfd, 0xc1,
		0xe3, 0xff, 0xa0, 0xd9, 0xff, 0x57, 0x2c, 0x00, 0xfd, 0x7c, 0x00, 0x87,
		0x4c, 0x01, 0x22, 0xbe, 0x00, 0x37, 0x73, 0x00, 0x57, 0x0f, 0xff, 0x89,
		0x0d, 0xff, 0xcd, 0x66, 0xff, 0x88, 0x40, 0xff, 0xfd, 0x65, 0xff, 0xf2,
		0x9e, 0x00, 0x50, 0x0c, 0x00, 0x29, 0x25, 0x00, 0x4b, 0x29, 0x00, 0x11,
		0x26, 0x00, 0x0f, 0xb4, 0xff, 0xec, 0x4a, 0xff, 0xf9, 0xc0, 0xff, 0xef,
		0xa7, 0x00, 0x0b, 0xf0, 0x00, 0x0b, 0x94, 0x00, 0x17, 0x5b, 0x00, 0x14,
		0x83, 0x00, 0x14, 0xcb, 0x00, 0x0a, 0x89, 0x00, 0x07, 0x88, 0x00, 0x01,
		0xec, 0x00, 0x06, 0xbb, 0x00, 0x06, 0xda, 0x00, 0x08, 0x03, 0x00, 0x0a,
		0x94, 0x00, 0x0b, 0x6b, 0x00, 0x0e, 0xaa, 0x00, 0x04, 0xe3, 0x00, 0x0a,
		0x4c, 0x00, 0x03, 0xf7, 0x00, 0x05, 0xee, 0x00, 0x06, 0x2b, 0x00, 0x08,
		0x4b, 0x00, 0x08, 0x93, 0x00, 0x08, 0x7e, 0x00, 0x08, 0x55, 0x00, 0x06,
		0x9c, 0x00, 0x04, 0xe3, 0x00, 0x08, 0x4b, 0x00, 0x02, 0x34, 0x00, 0x07,
		0x5f, 0x00, 0x06, 0x88, 0x00, 0x06, 0x92, 0x00, 0x05, 0xa6, 0x00, 0x06,
		0x0d, 0x00, 0x04, 0x72, 0x00, 0x08, 0x41, 0x00, 0x03, 0xed, 0x00, 0x05,
		0x2b, 0x00, 0x03, 0x2a, 0x00, 0x05, 0x7d, 0x00, 0x03, 0x0b, 0x00, 0x05,
		0x4a, 0x00, 0x03, 0x5e, 0x00, 0x04, 0x91, 0x00, 0x05, 0x87, 0x00, 0x01,
		0xb9, 0x00, 0x04, 0x9b, 0x00, 0x00, 0x90, 0x00, 0x04, 0xb0, 0x00, 0x02,
		0x2a, 0x00, 0x02, 0x20, 0x00, 0x02, 0x7c, 0x00, 0x03, 0x91, 0x00, 0x00,
		0xec, 0x00, 0x05, 0x69, 0xff, 0xff, 0x99, 0x00, 0x03, 0xd9, 0x00, 0x00,
		0x3e, 0x00, 0x02, 0x3e, 0xff, 0xff, 0xeb, 0x00, 0x03, 0x35, 0x00, 0x00,
		0x85, 0x00, 0x02, 0x49, 0x00, 0x01, 0xa5, 0x00, 0x01, 0x48, 0x00, 0x01,
		0x15, 0x00, 0x01, 0x67, 0x00, 0x00, 0x48, 0x00, 0x01, 0x9a, 0x00, 0x01,
		0x1f, 0x00, 0x01, 0x29, 0x00, 0x00, 0xc3, 0x00, 0x00, 0xe2, 0x00, 0x00,
		0xa4, 0x00, 0x00, 0x9a, 0x00, 0x00, 0x67, 0xff, 0xff, 0xeb, 0x00, 0x00,
		0x85, 0x00, 0x00, 0x5c, 0x00, 0x00, 0xf6, 0x00, 0x00, 0x67, 0x00, 0x00,
		0x90, 0x00, 0x00, 0x29, 0x00, 0x00, 0x85, 0x00, 0x00, 0x48, 0x00, 0x00,
		0x1f, 0x00, 0x00, 0x48, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x29, 0x00, 0x00,
		0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x33, 0x33, 0x79, 0x99,
		0x9a, 0x20, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x00, 0x18, 0xf5,
		0xc3, 0x00, 0x03, 0x2a, 0x00, 0x03, 0x2a, 0x04, 0x00, 0x00, 0x00, 0x67,
		0xae, 0x1c, 0xc0, 0x00, 0x00, 0x01, 0x88 };
