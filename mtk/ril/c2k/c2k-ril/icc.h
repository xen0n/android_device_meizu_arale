#ifndef VIA_TELECOM_ICC_H
#define VIA_TELECOM_ICC_H

#include <telephony/ril.h>

typedef enum
{
    COMMAND_READ_BINARY = 0xb0,
    COMMAND_UPDATE_BINARY = 0xd6,
    COMMAND_READ_RECORD = 0xb2,
    COMMAND_UPDATE_RECORD = 0xdc,
    COMMAND_SEEK = 0xa2,
    COMMAND_SELECT = 0xa4,
    COMMAND_GET_RESPONSE = 0xc0
} Icc_Command;

typedef struct
{
    char* Name;
    int Value;
} DFidTable;

void ParseUtkProcmdStr(const char* at, char** data);
void ParseUtkRawData(const char* at, char** data);

void ParseAtcmdCRSM(const char* at, char** proactive_cmd);

int convertDfId(char* DFid);

#endif // VIA_TELECOM_ICC_H
