/*
 * cliCommands.c
 *
 *  Created on: Apr 3, 2012
 *      Author: nlv02095
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#include <inttypes.h>	//TODO fix/converge types
#endif
#include "../../Tfa98xxhal/inc/lxScribo.h"

#include "cli/cmdline.h"
#include "../inc/climax.h"

#include "../inc/nxpTfa98xx.h"
#include "../inc/tfa98xxDiagnostics.h"
#include "../inc/tfa98xxLiveData.h"

cliSpeakerSide_t gSpeakerSide = cli_speaker_none;	 /* which to address */
extern struct gengetopt_args_info gCmdLine; /* Globally visible command line args */
extern int nxpTfaCurrentProfile;
extern nxpTfa98xxParameters_t tfaParams;
/*
 *
 */
static size_t cliFileToBuffer(char *filename, char *buffer, int max)
{
	FILE *f;
	size_t len;

	f = fopen(filename, "rb");;
	if (!f) {
		fprintf(stderr, "can't open %s\n", filename);
		return -1;
	}

	len = fread(buffer, 1, max, f);
	if(cli_verbose) printf("%s: %i bytes from %s\n", __FUNCTION__, (int)len, filename);

	fclose(f);

	return len;
}

static int cliWriteSpeakerFile(char *outputfile, unsigned char *speakerBuffer )
{
 FILE *f;

    f= fopen(outputfile, "w+b");
    if ( f <= 0) {
        fprintf( stderr, "Can't open %s for writing\n", outputfile);
        return 1;
    }
    fwrite( (const void*)speakerBuffer, TFA98XX_SPEAKERPARAMETER_LENGTH, 1 , f);
    fclose(f);
	printf("written %s\n", outputfile);

	return 0;
}
/*
 * execute the commands
 */
int cliCommands(int targetfd, char *xarg, Tfa98xx_handle_t *handlesIn)
{
unsigned char buffer[4*1024];	//,byte TODO check size or use malloc
int i,writes;
nxpTfa98xxParamsType_t params;
int length;
int loopcount=gCmdLine.loop_arg;
float imp, tCoef;
nxpTfa98xx_Error_t error=0;

  if ( gCmdLine.load_given) {
	  tfa98xxLoadParamsFile( handlesIn, gCmdLine.load_arg); // read params
#ifndef WIN32
	  if ( gCmdLine.splitparms_given)
		  tfa98xxDumpParams(gCmdLine.load_arg); // save all parameters in there
#endif
  }
  if ( gCmdLine.loadConfig_given)
     tfa98xxLoadFile( handlesIn, gCmdLine.loadConfig_arg, tfa_config_params);
  if ( gCmdLine.loadPreset_given)
     tfa98xxLoadFile( handlesIn, gCmdLine.loadPreset_arg, tfa_preset_params);
  if ( gCmdLine.loadSpeaker_given)
     tfa98xxLoadFile( handlesIn, gCmdLine.loadSpeaker_arg, tfa_speaker_params);

  do {
//	gSpeakerSide = gCmdLine.speaker_given ? cliParseSpeakerSide(gCmdLine.speaker_arg) : cli_speaker_left;
//	    tfa98xxSetIdx(gSpeakerSide) ; // only L & R, B gives error

    if ( gCmdLine.init_given )
    	error = nxpTfa98xxPowerdown();  // note: this is just a software flag

    if ( gCmdLine.reset_given )
        error = tfa98xxReset(handlesIn);

    if ( gCmdLine.pin_given ) {
    		if (xarg) {//set if extra arg
    			lxScriboSetPin(targetfd,gCmdLine.pin_arg, atoi(xarg));
    			printf("pin%d < %d\n", gCmdLine.pin_arg, atoi(xarg));
    		}
    		else
    			printf("pin%d : %d\n", gCmdLine.pin_arg,lxScriboGetPin(targetfd,gCmdLine.pin_arg));
    }
    if ( gCmdLine.profile_given ) {
    	if ( TFA_MAX_PROFILES > gCmdLine.profile_arg )
    		nxpTfaCurrentProfile = gCmdLine.profile_arg;
    	else {
    		printf("?illegal profilenr:%d ;max=%d\n", gCmdLine.profile_arg, TFA_MAX_PROFILES-1);
    		fprintf(stdout, "last status:%s\n", nxpTfa98xxGetErrorString(nxpTfa98xx_Error_BadParam));
    		return nxpTfa98xx_Error_BadParam;
    	}
    }
    // the param file is a multi arg
    for (i = 0; i < (int)gCmdLine.params_given; ++i)
    {
    	// refuse if there is no i2s clock
//    	error = nxpTfa98xxClockEnable();
//    	if ( error !=  nxpTfa98xx_Error_Ok ) {
//    		printf("Error: %s\n", nxpTfa98xxGetErrorString(error));
//    		return error;	// do not continue with BCK
//    	}
    	params = (nxpTfa98xxParamsType_t) cliParseFiletype (gCmdLine.params_arg[i]);// TODO nxpTfa98xxParamsType_t must replace cli type_t
    	if ( params != tfa_no_params) {
    		length = cliFileToBuffer( gCmdLine.params_arg[i], buffer, sizeof(buffer) );
    		if ( length >0)
    			error = nxpTfa98xxStoreParameters( handlesIn, params, buffer, length);
    		else
    		    _exit(1);
    	} else {
    	    printf("not a valid input file: %s\n", gCmdLine.params_arg[i]);
    	    _exit(1);
    	}
    }

    if ( gCmdLine.clock_given ) {
    	error = gCmdLine.clock_arg ?  nxpTfa98xxClockEnable(handlesIn) : nxpTfa98xxClockDisable(handlesIn);
    }
    if ( gCmdLine.clockStereo_given ) {
    	error = gCmdLine.clockStereo_arg ?  nxpTfa98xxClockEnableStereo(handlesIn) : nxpTfa98xxClockDisableStereo(handlesIn);
    }

    if ( gCmdLine.currentprof_given ) {
    	nxpTfaCurrentProfile = gCmdLine.currentprof_arg;
    }
    //
    // a write is only performed if there is a register AND a regwrite option
    // the nr of register args determine the total amount of transactions
    //
    writes = gCmdLine.regwrite_given;
    for (i = 0; i < (int)gCmdLine.register_given; ++i)
    {
    	if ( !writes ) // read if no write arg
    	  printf("0x%02x : 0x%04x\n",
    			gCmdLine.register_arg[i] , tfa98xxReadRegister(gCmdLine.register_arg[i], handlesIn) );
    	else {
      	  printf("0x%02x < 0x%04x\n", gCmdLine.register_arg[i], gCmdLine.regwrite_arg[i]);
      	  tfa98xxWriteRegister ( gCmdLine.register_arg[i], gCmdLine.regwrite_arg[i], handlesIn);
    	  writes--; // consumed write arg
    	}
    }
    // read xmem
    for (i = 0; i < (int)gCmdLine.xmem_given; ++i)
    {
    	printf("xmem[0x%02x] : 0x%06x\n", tfa98xxReadXmem(gCmdLine.xmem_arg[i], handlesIn) );
    }
    // tCoef
    if ( gCmdLine.tcoef_given ) {
        float oldtCoef;
        oldtCoef = tfa98xxTcoef( gCmdLine.tcoef_arg ); // no change if 0
        printf("current tCoef%c : %f\n", (oldtCoef<0.005f)?' ':'A', oldtCoef); //check which one
        if ( gCmdLine.tcoef_arg )
            printf("new     tCoef%c : %f\n", (gCmdLine.tcoef_arg<0.005f)?' ':'A', gCmdLine.tcoef_arg);
        if (gCmdLine.output_given) {
            cliWriteSpeakerFile(gCmdLine.output_arg, tfaParams.speakerBuffer );
        }
    }

   if ( gCmdLine.resetMtpEx_given ) {
		error = resetMtpEx( handlesIn);
		if ( error!=nxpTfa98xx_Error_Ok ) {
			fprintf(stderr, "reset MTPEX failed\n");
		}
	}
   // must come after loading param files, it will need the loaded speakerfile
	if ( gCmdLine.calibrate_given ) {
		error = nxpTfa98xxCalibration( handlesIn, gCmdLine.calibrate_arg[0]=='o'); //once if o , else always
		if ( error!=nxpTfa98xx_Error_Ok ) {
			fprintf(stderr, "1st-time calibration failed\n");
		} else {
			if (gCmdLine.output_given) {
				cliWriteSpeakerFile(gCmdLine.output_arg, tfaParams.speakerBuffer );
			}
		}
	}
   if ( gCmdLine.calibrateStereo_given ) {
		error = nxpTfa98xxCalibrationStereo( handlesIn, gCmdLine.calibrateStereo_arg[0]=='o'); //once if o , else always
		if ( error!=nxpTfa98xx_Error_Ok ) {
			fprintf(stderr, "1st-time calibration failed\n");
		} else {
			if (gCmdLine.output_given) {
				cliWriteSpeakerFile(gCmdLine.output_arg, tfaParams.speakerBuffer );
			}
		}
	}

	// shows the current  impedance
   if ( gCmdLine.calshow_given )
   {
		error = tfa98xxGetCalibrationImpedance(&imp, handlesIn);
      printf("current calibration impedance: %f\n", imp);
//    printf("current calibration impedance: %f, calculated calibration impedance: %f\n", imp, 0);
      error = Tfa98xx_DspReadSpeakerParameters(handlesIn[0],  TFA98XX_SPEAKERPARAMETER_LENGTH, tfaParams.speakerBuffer);
      tCoef = tCoefFromSpeaker(tfaParams.speakerBuffer);
      printf("current calibration tCoefA: %f\n", tCoef);
   }

    if ( gCmdLine.volume_given ) {
    	if ( gCmdLine.profile_given ) nxpTfa98xxSetVolume (handlesIn, gCmdLine.profile_arg, gCmdLine.volume_arg);
    	else 						  nxpTfa98xxSetVolume (handlesIn, nxpTfaCurrentProfile, gCmdLine.volume_arg);
    }

    if ( gCmdLine.versions_given ) {
        if (cli_verbose ) lxScriboPrintTargetRev(targetfd);
        error = nxpTfa98xxVersions(handlesIn, (char*)buffer, sizeof(buffer));
        length=strlen(buffer);
        // append scribo rev here becaus the nxp interface does not now about scribo
        length += lxScriboGetRev(targetfd, buffer+length, 256); // overwrite the terminator
        *(buffer+length) = '\0';        // terminate
        if ( nxpTfa98xx_Error_Ok == error)
                puts((char*)buffer);

    }

    if ( gCmdLine.dsp_given ) {
        int count;
    	/* the maximum message length in the communication with the DSP */
#       define MAX_PARAM_SIZE (144)
        count = gCmdLine.count_given? gCmdLine.count_arg :  MAX_PARAM_SIZE;
        if(count>sizeof(buffer)) count=sizeof(buffer);
    	// speakerboost=1
    	error = tfa98xxDspGetParam( 1, gCmdLine.dsp_arg,  count, buffer, handlesIn);
    	if (error == nxpTfa98xx_Error_Ok) {
    		for(i=0;i<count;i++)
    		{
    			printf("0x%02x ", buffer[i]);
    		}
    		printf("\n");
    	}

    }

    /* gCmdLine.diag_arg=0 means all tests
     *
     */
    if ( gCmdLine.diag_given) {
        int code,nr;

       // tfa98xxClose(handlesIn);

        nr = gCmdLine.diag_arg;
        code = tfa98xxDiag ( gCmdLine.slave_given? gCmdLine.slave_arg : tfa98xxI2cSlave,
                                gCmdLine.diag_arg) ;
        nr = tfa98xxDiagGetLatest();
        printf ("test %d %s (code=%d) %s\n", nr, code ? "Failed" : "Passed", code,
        												tfa98xxDiagGetLastErrorString());
        error = code? 1:0; // non-0 fail
    }
    if ( gCmdLine.dump_given)
    	tfa98xxDiagRegisterDump(tfa98xxI2cSlave);

    if ( gCmdLine.dumpStereo_given)
    {
       int index = 0;
       for (index = 0; index < 2; index++)
       {
         printf ("Device %d dump\n", index);
         tfa98xxDiagRegisterDump(tfa98xxI2cSlave+index);
       }
    }

    if ( gCmdLine.dumpmodel_given) {

        FILE *outfile;
        int model=gCmdLine.dumpmodel_arg[0]=='x';
		  nxpTfa98xxOpenLiveDataSlaves(handlesIn, tfa98xxI2cSlave, 1);

        printf("dumping %s model\n", model?"excursion":"impedance");

        if ( gCmdLine.output_given) {
            outfile = fopen(gCmdLine.output_arg,"w");
            tfa98xxPrintSpeakerModel(handlesIn, outfile, model, 0);
            printf("written to %s\n", gCmdLine.output_arg);
            fclose(outfile);
        } else
            tfa98xxPrintSpeakerModel(handlesIn, stdout, model, 0);
    }

    if ( gCmdLine.loop_given) {
    	loopcount = ( gCmdLine.loop_arg == 0) ? 1 : loopcount-1 ;
    }

    if ( (error!=nxpTfa98xx_Error_Ok)  | cli_verbose )
    	fprintf(stdout, "last status:%s\n", nxpTfa98xxGetErrorString(error));

  } while (loopcount>0) ;

  if ( gCmdLine.save_given) tfa98xxSaveParamsFile( handlesIn, gCmdLine.save_arg);
  if ( gCmdLine.saveConfig_given)
     tfa98xxSaveFile( handlesIn, gCmdLine.saveConfig_arg, tfa_config_params);
  if ( gCmdLine.savePreset_given)
     tfa98xxSaveFile( handlesIn, gCmdLine.savePreset_arg, tfa_preset_params);
  if ( gCmdLine.saveSpeaker_given)
     tfa98xxSaveFile( handlesIn, gCmdLine.saveSpeaker_arg, tfa_speaker_params);

  return error;
}

/*
 *
 */
int cliTargetDevice(char *devname)
{
	int fd;

	TRACEIN(__FUNCTION__);

	fd = lxScriboRegister(devname);

	if (fd < 0) {
		printf("Can't open %s\n", devname);
		exit(0);
	}

	return fd;

	TRACEOUT(__FUNCTION__);

	//return fd or die
}


