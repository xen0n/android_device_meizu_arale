#pragma once

#ifdef WIN32
#define    DEF_CriticalSection_Win(cs) CRITICAL_SECTION cs;
#define    DEF_CriticalSection_Linux(cs)
#define    DEF_AutoLock(cs) AutoLock lock(cs)
#define    DEF_InitCs(cs) InitializeCriticalSection(&cs)
#else

#include <utils/threads.h>
using namespace android;
#define    DEF_CriticalSection_Win(cs)
#define    DEF_CriticalSection_Linux(cs) mutable Mutex cs;
#define    DEF_AutoLock(cs) Mutex::Autolock lock(cs)
#define    DEF_InitCs(cs)
#endif





//

class NvBufUtil
{
public:
    enum
    {
        e_SensorDevWrong=-1000,
        e_NvramIdWrong,
        e_NV_SensorDevWrong,
        e_NV_FileRWWrong,
        e_NV_SensorIdNull,
    };
public:

    static NvBufUtil& getInstance();
    void setAndroidMode(int isAndroid=1); //anroid mode: android:1, meta(cct, factory):0

    /*
    nvRamId:
    CAMERA_NVRAM_DATA_ISP,
    CAMERA_NVRAM_DATA_3A,
    CAMERA_NVRAM_DATA_SHADING,
    CAMERA_NVRAM_DATA_LENS,
    CAMERA_DATA_AE_PLINETABLE,
    CAMERA_NVRAM_DATA_STROBE,
    CAMERA_NVRAM_DATA_N3D3A,
    CAMERA_NVRAM_DATA_GEOMETRY,
    CAMERA_NVRAM_VERSION,
    */
    //int setSensorId(int sensorDev, int id);
    int getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p);
    int getBufAndRead(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead=0);
    int write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);

    // note: please provide memory to call the function.
    // For sync the buf data with NvRam data, the internal buf can't be used in the function.
    int readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);

    int writeAllOemToData();
    int isOemAvailable();

private:
    DEF_CriticalSection_Linux(m_cs);
    NvBufUtil();
};

int nvbufUtil_getSensorId(int sensorDev, int& sensorId);




/*
//##########################################
// mechanism explain
//##########################################
//++++++++++++++++++++++++++++++
// getBufAndRead
//++++++++++++++++++++++++++++++
1st time:
  ---------     ---------
  | nvbuf | <-- | nvram |
  ---------     ---------
         ---------
  *p <-- | nvbuf |
         ---------

others:
         ---------
  *p <-- | nvbuf |
         ---------

//++++++++++++++++++++++++++++++
// write
//++++++++++++++++++++++++++++++

  ---------     ---------
  | nvbuf | --> | nvram |
  ---------     ---------


//++++++++++++++++++++++++++++++
// read default
//++++++++++++++++++++++++++++++

  ----------------     -----------------
  | external buf | <-- | default nvram |
  ----------------     -----------------
* default nvram is the content in code


//##########################################
//   sample test code
//##########################################
 {
        int err;
        NVRAM_CAMERA_FEATURE_STRUCT* buf;
        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, 1, (void*&)buf);
        logI("qq ln=%d 0x%x err=%d temp=%d",__LINE__, (int)buf, err, buf->temp);
        buf->temp=10;
        err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_FEATURE, 1);
        logI("qq ln=%d 0x%x err=%d",__LINE__, (int)buf, err);
        buf->temp=12;

        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, 2, (void*&)buf);
        logI("qq ln=%d 0x%x err=%d",__LINE__, (int)buf, err);
        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, 4, (void*&)buf);
        logI("qq ln=%d 0x%x err=%d",__LINE__, (int)buf, err);

        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, 1, (void*&)buf, 1);
        logI("qq ln=%d 0x%x err=%d buf=%d",__LINE__, (int)buf, err, buf->temp);
    }

*/