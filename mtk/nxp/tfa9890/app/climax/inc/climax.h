/*
 * climax.h
 *
 *  Created on: Apr 3, 2012
 *      Author: nlv02095
 */

#ifndef CLIMAX_H_
#define CLIMAX_H_

#include "nxpTfa98xx.h"

typedef enum cliSpeakerSide {
	cli_speaker_left='L',
	cli_speaker_right='R',
	cli_speaker_both='B',
	cli_speaker_none=0
} cliSpeakerSide_t;

char *cliInit(int argc, char **argv);
int cliCommands(int targetfd, char *xarg, Tfa98xx_handle_t *handlesIn);
nxpTfa98xxParamsType_t cliParseFiletype(char *filename);
int cliTargetDevice(char *devname);
cliSpeakerSide_t cliParseSpeakerSide(char *name);
#ifndef WIN32
void cliSocketServer(char *socket);
void cliClientServer(char *socket);
#endif
//int cliSaveParamsFile( char *filename );
//int cliLoadParamsFile( char *filename );


/*
 *  globals for output control
 */
extern int cli_verbose;	/* verbose flag */
extern int cli_trace;	/* message trace flag from bb_ctrl */
extern int cli_quiet;	/* don't print too much */

#define TRACEIN(F)  if(cli_trace) printf("Enter %s\n", F);
#define TRACEOUT(F) if(cli_trace) printf("Leave %s\n", F);

#endif /* CLIMAX_H_ */
