#include <stdio.h>
#include "Tfa98xx.h"

/* support for error code translation into text */

static char latest_errorstr[64];

const char* Tfa98xx_GetErrorString(Tfa98xx_Error_t error)
{
  const char* pErrStr;

  switch (error)
  {
  case Tfa98xx_Error_Ok:
    pErrStr = "Ok";
    break;
  case Tfa98xx_Error_DSP_not_running:
    pErrStr = "DSP_not_running";
    break;
  case Tfa98xx_Error_Bad_Parameter:
    pErrStr = "Bad_Parameter";
    break;
  case Tfa98xx_Error_NotOpen:
    pErrStr = "NotOpen";
    break;
  case Tfa98xx_Error_OutOfHandles:
    pErrStr = "OutOfHandles";
    break;
  case Tfa98xx_Error_RpcBusy:
    pErrStr = "RpcBusy";
    break;
  case Tfa98xx_Error_RpcModId:
    pErrStr = "RpcModId";
    break;
  case Tfa98xx_Error_RpcParamId:
    pErrStr = "RpcParamId";
    break;
  case Tfa98xx_Error_RpcInfoId:
    pErrStr = "RpcInfoId";
    break;
  case Tfa98xx_Error_RpcNotAllowedSpeaker:
    pErrStr = "RpcNotAllowedSpeaker";
    break;
  case Tfa98xx_Error_Not_Supported:
    pErrStr = "Not_Supported";
    break;
  case Tfa98xx_Error_I2C_Fatal:
    pErrStr = "I2C_Fatal";
    break;
  case Tfa98xx_Error_I2C_NonFatal:
    pErrStr = "I2C_NonFatal";
    break;
  case Tfa98xx_Error_StateTimedOut:
	pErrStr = "WaitForState_TimedOut";
	break;
  default:
    sprintf(latest_errorstr, "Unspecified error (%d)", (int)error);
    pErrStr = latest_errorstr;
  }
  return pErrStr;
}
