#pragma once

#ifdef WIN32
#include "win_test.h"
#include "win_test_cpp.h"

#else
#include <utils/threads.h>
using namespace android;

#endif

/*******************************************************************************
*
********************************************************************************/
class FlashlightDrv : public StrobeDrv
{
private:
    FlashlightDrv();
    virtual ~FlashlightDrv();
    static FlashlightDrv singleton1;

public:
    static StrobeDrv* getInstance(int sensorDrv, int strobeId);
    virtual int getPartId(int* partId);
    virtual FLASHLIGHT_TYPE_ENUM getFlashlightType();
    virtual int init();

    virtual int initTemp();
    virtual int uninit();
    virtual int isOn(int* a_isOn);

    virtual int setOnOff(int a_isOn);
    virtual int setStep(int step);
    virtual int setDuty(int duty);





    virtual int getVBat(int* vbat);
    virtual int setTimeOutTime(int ms);


    virtual int hasFlashHw();
    virtual int setPreOn();
    virtual int getPreOnTimeMs(int* ms);
    virtual int getPreOnTimeMsDuty(int duty, int* ms);
    virtual int setReg(int reg, int val);
    virtual int getReg(int reg, int* val);
    virtual int getDuty(int* duty);
    virtual int getStep(int* step);
    virtual int hasLowPowerDetect(int* hasLPD);

    virtual int isLowPower(int* bLow);
    virtual int lowPowerDetectStart(int lowPowerDuty);
    virtual int lowPowerDetectEnd();



    virtual int setStrobeInfo(int dutyNum, int tabNum, int* tabId, int* timeOutMs, float* coolingTM);


    virtual int getTimeOutTime(int duty, int* tTimeOut);
    virtual int getCoolTM(int duty, float* coolTM);




protected:
private:
    int init_real();
    int checkValid();
private:
    int mSensorDev;
    int m_preOnTime;
    int mUsers;
    int m_isOn;
    int m_bTempInit;
    mutable Mutex mLock;
    int m_flashType;
    int mStrobeId;
    int m_duty;
    int m_step;

    int mDutyNum;
    int mTabNum;
    int mTabId[10];
    float mCoolTMTab[10];
    int mTimeoutTab[10];

    int mTurnOnTime;
    int mTurnOffTime;
    int mOnDuty;
    int mTimeOutTime;




};

