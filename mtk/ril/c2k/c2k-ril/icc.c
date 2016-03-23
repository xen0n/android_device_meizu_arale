#define LOG_TAG "C2K_RIL"
#include <utils/Log.h>
#include "icc.h"

/**
 *        (see VIA AT Command Spec for details)
 *
 *          0        MF            0x3F00
 *          1        DF_CDMA       0x7F25
 *          2        DF_TELECOM    0x7F10
 *          3        DF_GSM        0x7F20
 *          4        DF_PHS        0x7FD0
 *
 */

static DFidTable CONVERT_TABLE[] = {
        {"3F00", 0},
        //7F25->UIM, 7FFF->CSIM
        {"7F25", 1},
        /*ALPS02001503, MD3 will translate the path diff by DFID == 1, 2015/03/25{*/
        {"7FFF", 1},
        /*ALPS02001503, MD3 will translate the path diff by DFID == 1, 2015/03/25}*/
        {"7F10", 2},
        {"7F20", 3},
        {"7FD0", 4},
        {"5F3A", 10}};

void ParseUtkProcmdStr(const char* at, char** data)
{
    if ( at == NULL )
    {
        return;
    }

    char* sep   = ":,";
    char* token;

    char* tmp = (char*)malloc(strlen(at) + 1);
    memset(tmp, 0, strlen(at) + 1);
    memcpy(tmp, at, strlen(at));

    int index = 0;
    token = strtok(tmp, sep);
    while ( token != NULL )
    {
        token = strtok(NULL, sep);
        //proactive data
        if ( index == 1)
        {
            *data = (char*)malloc(strlen(token) + 1);
            memset(*data, 0 , strlen(token) + 1);
            memcpy(*data, token, strlen(token));
        }
        index++;
    }
  free(tmp);
}

void ParseAtcmdCRSM(const char* at, char** proactive_cmd)
{
    if ( at == NULL )
        return;

    char* sep   = ":,";
    char* token;

    char* tmp = (char*)malloc(strlen(at) + 1);
    memset(tmp, 0, strlen(at) + 1);
    memcpy(tmp, at, strlen(at));

    int index = 0;
    token = strtok(tmp, sep);
    while ( token != NULL )
    {

        token = strtok(NULL, sep);

        if ( index == 2 && token )
        {
            *proactive_cmd = (char*)malloc(strlen(token) + 1);
            memset(*proactive_cmd, 0, strlen(token) + 1);
            strcpy(*proactive_cmd, token);
        }

        index++;
    }
  free(tmp);
}

void ParseUtkRawData(const char* at, char** data)
{
    if ( at == NULL )
        return;

    char* sep   = ":";
    char* token;

    char* tmp = (char*)malloc(strlen(at) + 1);
    memset(tmp, 0, strlen(at) + 1);
    memcpy(tmp, at, strlen(at));

    token = strtok(tmp, sep);
    token = strtok(NULL, sep);
    if (token )
    {
        *data = (char*)malloc(strlen(token) + 1);
        memset(*data, 0, strlen(token) + 1);
        strcpy(*data, token);
    }

    free(tmp);
}

int convertDfId(char * DFid)
{
    int len = sizeof(CONVERT_TABLE) / sizeof(DFidTable);
    int i;
    for(i = 0; i < len; i++)
    {
        if (strcmp(DFid, CONVERT_TABLE[i].Name) == 0)
        {
            return CONVERT_TABLE[i].Value;
        }
    }

    return -1;
}
