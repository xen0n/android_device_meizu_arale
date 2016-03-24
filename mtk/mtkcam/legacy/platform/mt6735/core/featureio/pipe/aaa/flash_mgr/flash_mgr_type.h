#ifndef __FLASH_MGR_TYPE_H__
#define __FLASH_MGR_TYPE_H__

/** \brief When executing flash flow (currently, only doPfOneFrame), the input struct is FlashExePara, output struct is FlashExeRep.
*
*/
typedef struct
{
    int isBurst;
    void* staBuf;
    int staX;
    int staY;

    int flickerMode;
    float evComp;

    AWB_GAIN_T NonPreferencePrvAWBGain;
    AWB_GAIN_T PreferencePrvAWBGain;
    MINT32 i4AWBCCT;

}FlashExePara;

/** \brief When executing flash flow (currently, only doPfOneFrame), the input struct is FlashExePara, output struct is FlashExeRep.
*
*/
typedef struct
{
    int isCal;
    int nextIso;
    int nextIsFlash;
    int nextExpTime;
    int nextAfeGain;
    int nextIspGain;
    int nextDuty;
    int nextStep;
    int isEnd;
    int isCurFlashOn;
    int flashAwbWeight;
    int isFlashCalibrationMode;
}FlashExeRep;


#endif //#ifndef __FLASH_MGR_TYPE_H__
