/*
 * climax.c
 *
 *  Created on: Apr 3, 2012
 *      Author: nlv02095
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <ctype.h>
#include <stdint.h>
#include "cmdline.h"
#include "climax.h"
#include "nxpTfa98xx.h"
#include "lxScribo.h"
#include "tfa98xxLiveData.h"
#include "tfa98xxRuntime.h"
/*
 * globals
 */
struct gengetopt_args_info gCmdLine; /* Globally visible command line args */


/*
 * module globals for output control
 */
int cli_verbose=0;	/* verbose flag */
int cli_trace=0 ;	/* message trace flag from bb_ctrl */
int cli_quiet=0;	/* don't print too much */
int NXP_I2C_verbose=0;

int socket_verbose = 0;
extern int lxScribo_verbose;
int i2cserver_verbose = 0; /* used in i2cserver */

extern char *lxDummyArg; //extra command line arg for test settings
extern int optind;   // processed option count
/*
 * file type names offset is type enum
 */
static const char *filetypeName[] = {
		"patch file" ,
		"speaker file" ,
		"preset file" ,
		"config file" ,
		"equalizer file" ,
		"drc file" ,
//		"mdrc file" ,
		"unknown file"
};
static const char *speakerSideName[] = {
		"Left" ,
		"Right" ,
		"Left+Right" ,
		"None"
};
cliSpeakerSide_t gSpeakerSide = cli_speaker_none;	 /* which to address */

int main(int argc, char *argv[])
{
    char *devicename, *xarg;
    int status, fd;
    Tfa98xx_handle_t handlesIn[] ={-1, 0}; //default one device

    devicename = cliInit(argc, argv);
	xarg = argv[optind]; // this is the remaining argv

    if ( gCmdLine.slave_given ) {
        tfa98xxI2cSlave = gCmdLine.slave_arg;
    }

    if ( gCmdLine.speaker_given) {
    	gSpeakerSide = gCmdLine.speaker_arg[0];//cli_speaker_none;
    	if ( gSpeakerSide== cli_speaker_both)
    		handlesIn[1]=-1; //address 2 devices
    }

    if ( cli_verbose )
        printf("devicename=%s, i2c=0x%02x\n" ,devicename, tfa98xxI2cSlave);

    fd = cliTargetDevice(devicename);
           
    tfa98xxLiveData_verbose = cli_verbose;
    status = cliCommands(fd, xarg, handlesIn);	// execute the commands

    if ( gCmdLine.record_given ) {  // TODO run in thread
        int loopcount=gCmdLine.count_arg;
        FILE *outfile;

        if ( gCmdLine.output_given) {
            outfile = fopen(gCmdLine.output_arg,"w");
        } else
            outfile = stdout;

        tfa98xxPrintRecordHeader(outfile, gCmdLine.record_arg);
        //
        if (  nxpTfa98xx_Error_Ok == nxpTfa98xxOpenLiveDataSlaves(handlesIn, tfa98xxI2cSlave, 1))
            do {
                tfa98xxPrintRecord(handlesIn, outfile, 0);
                loopcount = ( gCmdLine.count_arg == 0) ? 1 : loopcount-1 ;
                tfaRun_Sleepus(1000*gCmdLine.record_arg); // is msinterval
            } while (loopcount>0) ;

        if ( gCmdLine.output_given) {
            printf("written to %s\n", gCmdLine.output_arg);
            fclose(outfile);
        }
    }
    if (gCmdLine.recordStereo_given)
    {
       int loopcount=gCmdLine.count_arg;
        FILE *outfile;

        if ( gCmdLine.output_given) {
            outfile = fopen(gCmdLine.output_arg,"w");
        } else
            outfile = stdout;

        tfa98xxPrintRecordHeader(outfile, gCmdLine.recordStereo_arg);
        //
        if (  nxpTfa98xx_Error_Ok == nxpTfa98xxOpenLiveDataSlaves(handlesIn, tfa98xxI2cSlave, 2))
            do {
                tfa98xxPrintRecordStereo(handlesIn, outfile);
                loopcount = ( gCmdLine.count_arg == 0) ? 1 : loopcount-1 ;
                tfaRun_Sleepus(gCmdLine.recordStereo_arg*1000); // is msinterval
            } while (loopcount>0) ;

        if ( gCmdLine.output_given) {
            printf("written to %s\n", gCmdLine.output_arg);
            fclose(outfile);
        }
    }
    if (gCmdLine.logger_given) {
    	// call with interval and count
    	tfa98xxLogger( gCmdLine.logger_arg, gCmdLine.count_arg);
    }

#ifndef WIN32
    if ( gCmdLine.server_given ) {
        printf("statusreg:0x%02x\n", tfa98xxReadRegister(0,handlesIn)); // read to ensure device is opened
        cliSocketServer(gCmdLine.server_arg); // note socket is ascii string
    }
    if ( gCmdLine.client_given ) {
        printf("statusreg:0x%02x\n", tfa98xxReadRegister(0,handlesIn)); // read to ensure device is opened
        cliClientServer(gCmdLine.client_arg); // note socket is ascii string
    }
#endif
    exit (status);

}

#ifndef WIN32
/*
 *
 */
int activeSocket; // global
void cliSocketServer(char *socket)
{
	 int length, i;
	 uint8_t cmd[2], buf[256], *ptr, *devname;

	activeSocket=lxScriboListenSocketInit(socket);

	if(activeSocket<0) {
		fprintf(stderr, "something wrong with socket %s\n", socket);
		exit(1);
	}

	while(1){
		length = read(activeSocket, buf, 256);
		if (socket_verbose & (length>0)) {
			printf("recv: ");
			for(i=0;i<length;i++)
			     printf("0x%02x ", buf[i]);
			printf("\n");
		}
		if (length>0)
		  CmdProcess(buf,  length);
		else {
			close(activeSocket);
			tfaRun_Sleepus(10000);
			activeSocket=lxScriboListenSocketInit(socket);
		}
	}

}

/*
 *
 */

void cliClientServer(char *server)
{
	 int length, i;
	 uint8_t cmd[2], buf[256], *ptr, *devname;

	activeSocket=lxScriboSocketInit(server);

	if(activeSocket<0) {
		fprintf(stderr, "something wrong with client %s\n", server);
		exit(1);
	}

	while(1){
		length = read(activeSocket, buf, 256);
		if (socket_verbose & (length>0)) {
			printf("recv: ");
			for(i=0;i<length;i++)
			     printf("0x%02x ", buf[i]);
			printf("\n");
		}
		if (length>0)
		  CmdProcess(buf,  length);
		else {
			close(activeSocket);
			tfaRun_Sleepus(10000);
			activeSocket=lxScriboSocketInit(server);
		}
	}
}
#endif
/*
 *
 */
cliSpeakerSide_t cliParseSpeakerSide(char *name)
{
	char firstLetter;
	cliSpeakerSide_t side;


	if (name==NULL)
		side=cli_speaker_none;
	else
	{
		firstLetter = tolower(name[0]);
		switch (firstLetter) {
			case '0':
			case 'l':
				side = cli_speaker_left;
				break;
			case '1':
			case 'r':
				side = cli_speaker_right;
				break;
			case 'b':
				side = cli_speaker_both;
				break;
			default:
				side = cli_speaker_none;
				break;
		}
	}
    if ( cli_verbose )
    	printf ("selected speaker: %s \n", speakerSideName[side]);
    else if (side==cli_speaker_none)
    	fprintf(stderr, "Warning: no speaker selected!\n");

    return side;
}
/*
 *
 */
nxpTfa98xxParamsType_t cliParseFiletype(char *filename)
{
	char *ext;
	nxpTfa98xxParamsType_t ftype;

	// get filename extension

	ext = strrchr(filename, '.'); // point to filename extension

	if ( ext == NULL ) {
		ftype = tfa_no_params;	// no '.'
	}

	// now look for supported type
	else if ( strcmp(ext, ".patch")==0 )
		ftype = tfa_patch_params;
	else if ( strcmp(ext, ".speaker")==0 )
		ftype = tfa_speaker_params;
	else if ( strcmp(ext, ".preset")==0 )
		ftype = tfa_preset_params;
	else if ( strcmp(ext, ".config")==0 )
		ftype = tfa_config_params;
	else if ( strcmp(ext, ".eq")==0 )
		ftype = tfa_equalizer_params;
	else if ( strcmp(ext, ".drc")==0 )
		ftype = tfa_drc_params;
	else ftype = tfa_no_params;

    if ( cli_verbose )
    	printf("file %s is a %s.\n" , filename, filetypeName[ftype]);

    return ftype;

}
/*
 * init the gengetopt stuff
 */
char *cliInit(int argc, char **argv)
{
	char *devicename;

    cmdline_parser (argc, argv, &gCmdLine);
    if(argc==1) // nothing on cmdline
    {
    		cmdline_parser_print_help();
    		exit(1);
    }
    // extra command line arg for test settings
    lxDummyArg = argv[optind]; // this is the remaining argv

    // generic flags
    if (gCmdLine.verbose_given) {
    	cli_verbose= 1;
    	lxScribo_verbose      =  (1 & gCmdLine.verbose_arg)!=0;
    	socket_verbose 		  =  (2 & gCmdLine.verbose_arg)!=0;
    	i2cserver_verbose     =  (4 & gCmdLine.verbose_arg)!=0;
    	gTfaRun_timingVerbose =  (8 & gCmdLine.verbose_arg)!=0;
    }
    //lxScribo_verbose=
    tfa98xx_verbose = cli_verbose;
    tfa98xx_quiet = gCmdLine.quiet_given;
    cli_trace=gCmdLine.trace_given;
//    tfa98xx_trace = cli_trace;
#ifndef WIN32
    i2c_trace = cli_verbose;
#endif
    NXP_I2C_verbose= cli_trace;
    cli_quiet=gCmdLine.quiet_given;

    if (gCmdLine.device_given)
    	devicename=gCmdLine.device_arg;
    else
#ifdef TFA_I2CDEVICE
    	devicename=TFA_I2CDEVICE;
#else
    	devicename=DEVNAME;
#endif
    return devicename;

}
