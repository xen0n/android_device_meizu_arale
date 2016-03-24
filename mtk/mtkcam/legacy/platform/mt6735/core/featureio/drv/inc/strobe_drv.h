#pragma once






class StrobeDrv
{
protected:

    /*******************************************************************************
       * Functionality : mhal strobe base descontrustor
       *
       ********************************************************************************/
    virtual ~StrobeDrv() = 0;

public:

    /*******************************************************************************
       * Strobe Error Code
       *
       ********************************************************************************/
    typedef enum
    {
        STROBE_NO_ERROR = 0,                  ///< The function work successfully
        STROBE_UNKNOWN_ERROR = -1000,    ///< Unknown error
        STROBE_FILE_ERR,
        STROBE_FILE_ERR2,


        STROBE_ERR_PARA_INVALID,
    } STROBE_ERROR_ENUM;

    /*******************************************************************************
       * Strobe Type
       *
       ********************************************************************************/
    typedef enum
    {
        FLASHLIGHT_NONE = 0,
        FLASHLIGHT_LED_ONOFF,       // LED always on/off
        FLASHLIGHT_LED_CONSTANT,    // CONSTANT type LED
        FLASHLIGHT_LED_PEAK,        // peak strobe type LED
        FLASHLIGHT_LED_TORCH,       // LED turn on when switch FLASH_ON
        FLASHLIGHT_XENON_SCR,       // SCR strobe type Xenon
        FLASHLIGHT_XENON_IGBT       // IGBT strobe type Xenon
    }   FLASHLIGHT_TYPE_ENUM;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    static StrobeDrv*   getInstance(int sensorDev, int strobeId=1);


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/



    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual FLASHLIGHT_TYPE_ENUM getFlashlightType() = 0;


    /*******************************************************************************
       * Functionality : initialization
       *
       ********************************************************************************/
    virtual int init()= 0;

    virtual int initTemp()= 0;


    /*******************************************************************************
       * Functionality : uninitialization
       *
       ********************************************************************************/
    virtual int uninit()= 0;




    /*******************************************************************************
       * Functionality : send IOCTL command to kernel driver to set duty of strobe
       *
       ********************************************************************************/






    /*******************************************************************************
       * Functionality : send IOCTL command to kernel driver to set state of strobe
       *
       ********************************************************************************/
    virtual int getPartId(int* partId)=0;





    virtual int isOn(int* a_isOn) =0;
    virtual int setOnOff(int a_isOn) =0;
    virtual int setStep(int step)=0;
    virtual int setDuty(int duty)=0;



    virtual int getVBat(int* vbat)=0;
    virtual int setTimeOutTime(int ms)=0;

    virtual int hasFlashHw()=0;


    virtual int setPreOn()=0;
    virtual int getPreOnTimeMs(int* ms)=0;

    virtual int setReg(int reg, int val)=0;
    virtual int getReg(int reg, int* val)=0;

    virtual int getDuty(int* duty)=0;
    virtual int getStep(int* step)=0;


    virtual int hasLowPowerDetect(int* hasLPD)=0;
    virtual int isLowPower(int* bLow)=0;
    virtual int lowPowerDetectStart(int lowPowerDuty)=0;
    virtual int lowPowerDetectEnd()=0;


    virtual int setStrobeInfo(int dutyNum, int tabNum, int* tabId, int* timeOutMs, float* coolingTM)=0;


    virtual int getTimeOutTime(int duty, int* tTimeOut)=0; //call before flash is on
    virtual int getCoolTM(int duty, float* coolTM)=0;








/*
int tabNum;
    int tabMode;
    int tabId[10]; //index or current
    float coolingTM[10]; //time multiply factor
    int timOutMs[10];
    */



protected:

private:

};


