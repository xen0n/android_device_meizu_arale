#define LOG_TAG "flashlight_drv.cpp"

#ifdef WIN32
    #include "stdio.h"
    #include "time.h"
    #include "strobe_drv_2led.h"
    #include "flashlight_drv_2led.h"
    #include "kd_flashlight.h"
    #include "windows.h"
    #include "win_test_cpp.h"
    #include "strobe_global_driver.h"
#else
    #include <utils/Errors.h>
    #include <cutils/log.h>
    #include <fcntl.h>
    #include "ae_feature.h"
    #include "../inc/strobe_drv.h"
    #include "flashlight_drv.h"
    #include "kd_flashlight.h"
    #include <mtkcam/Log.h>
    #include "camera_custom_nvram.h"
    #include "camera_custom_types.h"
    #include "camera_custom_AEPlinetable.h"
    #include "camera_custom_nvram.h"
    #include <cutils/log.h>
    #include "flash_feature.h"
    #include "flash_param.h"
    #include "flash_tuning_custom.h"
    #include <kd_camera_feature.h>
    #include "strobe_global_driver.h"
#endif
//======================================================
#ifdef WIN32
    #define logI(fmt, ...)    {printf(fmt, __VA_ARGS__); printf("\n");}
    #define logE(fmt, ...)    {printf("merror: line=%d, ", __LINE__); printf(fmt, __VA_ARGS__); printf("\n");}
#else
    #define logI(fmt, arg...)  CAM_LOGD( fmt, ##arg)
    #define logE(fmt, arg...)  CAM_LOGD("error: line=%d, " fmt, __LINE__, ##arg)
#endif

#define MTK_LOG_ENABLE 1
#include <time.h>
static int getMs()
{
#ifdef WIN32
    int t;
    t = clock();
    return t;
#else
    //    max:
    //    2147483648 digit
    //    2147483.648 second
    //    35791.39413 min
    //    596.5232356 hour
    //    24.85513481 day
    //int t;
    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //t = (tv.tv_sec*1000 + (tv.tv_usec+500)/1000);
    int t;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (ts.tv_sec*1000+ts.tv_nsec/1000000);
    logI("t=%d",t);
    return t;
#endif
}

 static void sleepMs(int ms)
{
#ifdef WIN32
     Sleep(ms);
#else
     usleep(ms*1000);
#endif
}

FlashlightDrv FlashlightDrv::singleton1;
StrobeDrv* FlashlightDrv::getInstance(int sensorDrv, int strobeId)
{
    logI("getInstance %d %d",sensorDrv, strobeId);

    logI("getInstance line=%d",__LINE__);
    FlashlightDrv* p=0;

    if(sensorDrv==e_CAMERA_MAIN_SENSOR)
    {
        if(strobeId==1)
        {
            p =  &singleton1;
        }
        else if(strobeId==2)
        {
            static FlashlightDrv singleton2;
            p =  &singleton2;
        }
    }
    else if(sensorDrv==e_CAMERA_SUB_SENSOR)
    {
        if(strobeId==1)
{
            static FlashlightDrv singleton3;
            p =  &singleton3;
}
        else if(strobeId==2)
        {
            static FlashlightDrv singleton4;
            p =  &singleton4;
        }
    }
    else if(sensorDrv==e_CAMERA_MAIN_2_SENSOR)
    {
        if(strobeId==1)
{
            static FlashlightDrv singleton5;
            p =  &singleton5;
        }
        else if(strobeId==2)
    {
            static FlashlightDrv singleton6;
            p =  &singleton6;
        }
    }
    if(p!=0)
    {
        p->mSensorDev = sensorDrv;
        p->mStrobeId = strobeId;
        return p;
    }
    else
    {
        logE("input is wrong %d", __LINE__);
        return 0;
    }
}


int FlashlightDrv::isLowPower(int* bLow)
{
    logI("isLowPower()");
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommandRet(FLASH_IOC_IS_LOW_POWER, mSensorDev, mStrobeId, bLow);
    logI("isLowPower(%d)", *bLow);
    return err;
}

int FlashlightDrv::lowPowerDetectStart(int lowPowerDuty)
{
    logI("lowPowerDetectStart()");
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
     int err;
    err = pKDrv->sendCommand(FLASH_IOC_LOW_POWER_DETECT_START, mSensorDev, mStrobeId, lowPowerDuty);
    return err;
}
int FlashlightDrv::lowPowerDetectEnd()
{
    logI("lowPowerDetectEnd()");
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_LOW_POWER_DETECT_END, mSensorDev, mStrobeId, 0);
    return err;
}
int FlashlightDrv::getPartId(int* partId)
{
    logI("getPartId()");
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    pKDrv->openkd();
    int err;
    err = pKDrv->sendCommandRet(FLASH_IOC_GET_PART_ID, mSensorDev, mStrobeId, partId);
    pKDrv->closekd();
    logI("getPartId %d",*partId);
    return err;
}
//======================================================
int FlashlightDrv::getVBat(int* vbat)
{
    int err=0;
    int v;
    FILE* fp;
    fp = fopen("/sys/class/power_supply/battery/batt_vol", "rb");
    if(fp!=0)
    {
        int ret;
        ret = fscanf(fp,"%d",&v);
        if(ret == 1) //read 1
        {
            *vbat = v;
        }
        else
            err = STROBE_FILE_ERR2;
        fclose(fp);
    }
    else
    {
        err = STROBE_FILE_ERR;
    }
    return err;
}
//======================================================
int FlashlightDrv::isOn(int* a_isOn)
{
    logI("isOn()\n");
    *a_isOn = m_isOn;
    return 0;

}
int FlashlightDrv::checkValid()
{
    if( mStrobeId<=0 || mStrobeId>=3)
        return -1;
    if( mSensorDev != e_CAMERA_SUB_SENSOR &&  mSensorDev != e_CAMERA_MAIN_SENSOR &&  mSensorDev != e_CAMERA_MAIN_2_SENSOR)
        return -1;
    return 0;
}

int FlashlightDrv::setOnOff(int a_isOn)
{
    int hasHw;
    hasHw = hasFlashHw();
    if(hasHw!=1)
        return 0;

    logI("setOnOff() isOn = %d\n",a_isOn);
    if(checkValid()!=0)
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    int err = 0;


    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();

    if(a_isOn==1)
    {
        if(m_isOn==0)
        {
            mTurnOnTime = getMs();
            mOnDuty=m_duty;
        }
        if(m_duty>mOnDuty)
            mOnDuty=m_duty;

        int minPreOnTime;
        err = getPreOnTimeMsDuty(m_duty, &minPreOnTime);
        if(minPreOnTime>100)
            minPreOnTime=100;
        else if(minPreOnTime<0)
            minPreOnTime=-1;
        if(err<0)
        {

            logI("no preon");
        }
        else
        {
            if(minPreOnTime==-1)
            {
                logI("no preon(preontime=-1)");
            }
            else
            {
                logI("preon support %d",m_preOnTime);
                if(m_preOnTime==-1)
                {

                    setPreOn();
                    sleepMs(minPreOnTime);
                }
                else
                {

                    int curTime;
                    int sleepTimeMs;
                    curTime = getMs();
                    sleepTimeMs = (minPreOnTime-(curTime-m_preOnTime));
                    logI("preon sleep %d ms", sleepTimeMs);
                    if(sleepTimeMs>minPreOnTime)
                        sleepTimeMs = minPreOnTime;
                    if(sleepTimeMs>0)
                    {
                        sleepMs( sleepTimeMs);
                    }
                }
            }
        }

        err = pKDrv->sendCommand(FLASH_IOC_SET_ONOFF, mSensorDev, mStrobeId, 1);
    }
    else if(a_isOn==0)
    {
        if(m_isOn==1)
        {
            mTurnOffTime= getMs();
            if(mTurnOffTime-mTurnOnTime>mTimeOutTime && mTimeOutTime!=0)
                logE("TimeOut");
        }
        m_preOnTime=-1;

        err = pKDrv->sendCommand(FLASH_IOC_SET_ONOFF, mSensorDev, mStrobeId, 0);
    }
    else
    {

        err = STROBE_ERR_PARA_INVALID;
    }

    if (err < 0)
    {

        logE("setOnOff() err=%d\n", err);
    }

    if(err==0)
        m_isOn=a_isOn;
    return err;
}
//======================================================
int FlashlightDrv::setStep(int step)
{
    logI("setStep() step = %d\n",step);
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_SET_STEP, mSensorDev, mStrobeId, step);
    m_step = step;
    return err;
    }
//======================================================
int FlashlightDrv::setDuty(int duty)
{
    logI("setDuty() duty = %d\n",duty);
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_SET_DUTY, mSensorDev, mStrobeId, duty);
    m_duty = duty;
    if(m_isOn==1 && m_duty>mOnDuty)
        mOnDuty=m_duty;

    return err;
}
//======================================================
int FlashlightDrv::setTimeOutTime(int ms)
{

    logI("setTimeOutTime() ms = %d\n",ms);
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_SET_TIME_OUT_TIME_MS, mSensorDev, mStrobeId, ms);
    mTimeOutTime = ms;
    return err;
}
//======================================
int FlashlightDrv::setPreOn()
{
    logI("qq setPreOn()");
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_PRE_ON, mSensorDev, mStrobeId, 0);
    m_preOnTime=getMs();
    return err;
}
//======================================
int FlashlightDrv::getPreOnTimeMs(int* ms)
{
    logI("getPreOnTimeMs()");
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommandRet(FLASH_IOC_GET_PRE_ON_TIME_MS, mSensorDev, mStrobeId, ms);
    logI("getPreOnTimeMs=%d", *ms);
    return err;
}

int FlashlightDrv::getPreOnTimeMsDuty(int duty, int* ms)
{
    logI("getPreOnTimeMsDuty()");
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_GET_PRE_ON_TIME_MS_DUTY, mSensorDev, mStrobeId, (long)duty);
    err = pKDrv->sendCommandRet(FLASH_IOC_GET_PRE_ON_TIME_MS, mSensorDev, mStrobeId, ms);
    logI("getPreOnTimeMs=%d err=%d", *ms, err);
    return err;
}


int FlashlightDrv::setReg(int reg, int val)
{
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err1;
    int err2;
    int err3;
    err1 = pKDrv->sendCommand(FLASH_IOC_SET_REG_ADR, mSensorDev, mStrobeId, reg);
    err2 = pKDrv->sendCommand(FLASH_IOC_SET_REG_VAL, mSensorDev, mStrobeId, val);
    err3 = pKDrv->sendCommand(FLASH_IOC_SET_REG, mSensorDev, mStrobeId, 0);
    if(err1!=0)
        return err1;
    if(err2!=0)
        return err2;
    if(err3!=0)
        return err3;
    return 0;
}
int FlashlightDrv::getReg(int reg, int* val)
  {
    int ret;
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    ret = pKDrv->sendCommand(FLASH_IOC_GET_REG, mSensorDev, mStrobeId, reg);
    *val = ret;
      return 0;
  }
//======================================================
int FlashlightDrv::hasFlashHw()
{
  if(e_CAMERA_MAIN_SENSOR==mSensorDev)
  {
#ifdef DUMMY_FLASHLIGHT
    return 0;
#else
    return 1;
#endif
  }
  else if(e_CAMERA_SUB_SENSOR==mSensorDev)
  {
if(cust_isSubFlashSupport()==1)
    return 1;
else
    return 0;
  }
  else
    return 0;


}

StrobeDrv::FLASHLIGHT_TYPE_ENUM FlashlightDrv::getFlashlightType()
{
    int hasHw;
    hasHw = hasFlashHw();
    if(hasHw==1)
        return StrobeDrv::FLASHLIGHT_LED_CONSTANT;
    else
    return StrobeDrv::FLASHLIGHT_NONE;
  }

//======================================================
FlashlightDrv::FlashlightDrv()
      : StrobeDrv()
    , mUsers(0)
    , m_flashType((int)StrobeDrv::FLASHLIGHT_NONE)

{
    logI("FlashlightDrv()\n");
    m_isOn=0;
    m_duty=0;
    m_step=0;
    m_bTempInit=0;
    mDutyNum=0;
    mTurnOnTime=0;
    mOnDuty = -1;

}

//======================================================
FlashlightDrv::~FlashlightDrv()
{
    logI("FlashlightDrv line=%d",__LINE__);

}
//======================================================
int FlashlightDrv::initTemp()
{
    logI("[initTemp] mUsers = %d\n", mUsers);
    logI("initTemp line=%d",__LINE__);
    int err = 0;
    Mutex::Autolock lock(mLock);
    if (mUsers > 0)
    {

        }
    else
    {
        init_real();
        m_bTempInit=1;
        }
    return StrobeDrv::STROBE_NO_ERROR;
    }

int FlashlightDrv::init()
{
    logI("[init] mUsers = %d\n", mUsers);
    logI("init line=%d",__LINE__);
    int err = 0;
    Mutex::Autolock lock(mLock);

    if(m_bTempInit==1)
    {
        m_bTempInit=0;
        return StrobeDrv::STROBE_NO_ERROR;
    }
    if (mUsers == 0)
    {
        init_real();
    }
    else
        android_atomic_inc(&mUsers);
    return StrobeDrv::STROBE_NO_ERROR;
}

int FlashlightDrv::init_real()
{
    logI("[init_real] mUsers = %d\n", mUsers);
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->init(mSensorDev, mStrobeId);
    m_isOn=0;
            m_duty=-1;
            m_step=0;
            m_preOnTime=-1;
            android_atomic_inc(&mUsers);
    return err;
}

//======================================================
int FlashlightDrv::uninit()
    {
    //logI("uninit line=%d",__LINE__);
    //MHAL_LOG("[halSTROBEUninit] \n");
    //logI("[uninit] mUsers = %d\n", mUsers);
    Mutex::Autolock lock(mLock);
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->uninit(mSensorDev, mStrobeId);
    android_atomic_dec(&mUsers);
    m_bTempInit=0;
        return StrobeDrv::STROBE_NO_ERROR;
    }
//======================================================
int FlashlightDrv::getDuty(int* duty)
{
    logI("getDuty()\n");

    *duty = m_duty;
    return 0;

}

int FlashlightDrv::getStep(int* step)
{
    logI("getStep()\n");

    *step = m_step;
    return 0;

}

int FlashlightDrv::hasLowPowerDetect(int* hasLPD)
{
    logI("hasLowPowerDetect()");


    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommandRet(FLASH_IOC_HAS_LOW_POWER_DETECT, mSensorDev, mStrobeId, hasLPD);
    logI("hasLowPowerDetect=%d", *hasLPD);
    return err;
}
/*
int FlashlightDrv::lowPowerDetectStart()
{
    logI("lowPowerDetectStart()");
    int hasLPD;
    hasLowPowerDetect(&hasLPD);
    if(hasLPD==0)
        return 0;
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_LOW_POWER_DETECT_START, mSensorDev, mStrobeId, 0);
    return err;
}
int FlashlightDrv::lowPowerDetectEnd(int* isLowPower)
    {
    logI("lowPowerDetectEnd()");
    int hasLPD;
    *isLowPower=0;
    hasLowPowerDetect(&hasLPD);
    if(hasLPD==0)
        {
        *isLowPower=0;
        return 0;
    }
    StrobeGlobalDriver* pKDrv;
    pKDrv = StrobeGlobalDriver::getInstance();
    int err;
    err = pKDrv->sendCommand(FLASH_IOC_LOW_POWER_DETECT_END, mSensorDev, mStrobeId, (long)isLowPower);
    logI("isLowPower=%d", *isLowPower);
    return err;

}*/

int FlashlightDrv::setStrobeInfo(int dutyNum, int tabNum, int* tabId, int* timeOutMs, float* coolingTM)
{
    logI("setStrobeInfo ln=%d", __LINE__);
    mDutyNum=dutyNum;
    mTabNum = tabNum;
    int i;
    logI("setStrobeInfo ln=%d tabNum=%d", __LINE__,tabNum);
    for(i=0;i<tabNum;i++)
    {
        logI("ln=%d i=%d %d %f %d", __LINE__,i,tabId[i],coolingTM[i],timeOutMs[i]);
        mTabId[i]=tabId[i];
        mCoolTMTab[i]=coolingTM[i];
        mTimeoutTab[i]=timeOutMs[i];
    }
    logI("setStrobeInfo ln=%d", __LINE__);
    return 0;
}


template <class T, class T2>
    static T2 flash_interp(T x1, T2 y1, T x2, T2 y2, T x)
    {
        return y1+ (y2-y1)*(x-x1)/(x2-x1);
    }

template <class Tx, class T2>
static T2 flash_calYFromXYTab(int n, Tx* xNode, T2* yNode, Tx x)
{
    T2 y=yNode[0];
    int i;
    Tx xst;
    T2 yst;
    Tx xed;
    T2 yed;
    xst=xNode[0];
    yst=yNode[0];
    if(x<xNode[0])
        x=xNode[0];
    else if(x>xNode[n-1])
        x=xNode[n-1];

    for(i=1;i<n;i++)
    {
        xed=xNode[i];
        yed=yNode[i];
        if(x<=xNode[i])
        {
            y=flash_interp(xst, yst, xed, yed, x);
            break;
        }
        xst=xed;
        yst=yed;
    }
    if(x<=xNode[0])
        y=yNode[0];
    else if(x>=xNode[n-1])
        y=yNode[n-1];
    return y;
}



int FlashlightDrv::getTimeOutTime(int duty, int* tTimeOut)
{
    if(duty==-1)
        *tTimeOut=0;
    *tTimeOut = flash_calYFromXYTab(mTabNum, mTabId, mTimeoutTab, duty);;
    return 0;
}
int FlashlightDrv::getCoolTM(int duty, float* coolTM)
{
    if(duty==-1)
        *coolTM=0;
    *coolTM = flash_calYFromXYTab(mTabNum, mTabId, mCoolTMTab, duty);;
    return 0;
}
