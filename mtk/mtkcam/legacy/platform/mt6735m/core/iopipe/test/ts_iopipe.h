/**
* @file ts_UT.h
*
* ts_UT thread Header File
*/

//! \file  ts_camio.cpp
//! \brief
#if 0
#define _USE_THREAD_QUE_
#endif

#define LOG_TAG "iopipeTest"
//
#include <vector>
//#include <linux/cache.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>

/************************
* NormalPipe
************************/
#include <mtkcam/hal/sensor_hal.h>
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/iopipe/CamIO/INormalPipe_wrap.h>
#include <iopipe/CamIO/PortMap.h>
#include <mtkcam/drv_common/imem_drv.h>

#define USE_IMAGEBUF_HEAP
#ifdef USE_IMAGEBUF_HEAP
    #include <mtkcam/utils/ImageBufferHeap.h>
#endif

/************************************
*  namespace
************************************/
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

//thread
#ifdef _USE_THREAD_QUE_
    #include <pthread.h>
    #include <utils/threads.h>
    #include <semaphore.h>
#endif

/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

#define BUF_NUM          3
#define PREVIEW_WIDTH    1600
#define PREVIEW_HEIGHT   1200

#ifndef _MTK_CAMERA_CORE_CAMIOPIPE_TEST_UT_H_
#define _MTK_CAMERA_CORE_CAMIOPIPE_TEST_UT_H_

/*******************************************************************************
*
********************************************************************************/

namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {


    class Ts_UT
    {

        public:     ////    Constructor/Destructor.
            Ts_UT();
            /**
            * @brief  Destructor
            */
            virtual ~Ts_UT();

        public:
             virtual int main_ts_IOPipe_ZSD(int count);
            virtual int main_SetSensor_init();
            virtual int main_SetSensor_uninit();
            virtual MVOID freeRawMem();

            #ifdef _USE_THREAD_QUE_
                virtual MVOID TS_Thread_Init(int count);
                virtual MBOOL TS_Thread_UnInit();
            #endif // _USE_THREAD_QUE_

        private:
            virtual MBOOL skipFrame(MUINT32 const u4SkipCount);

            #ifdef _USE_THREAD_QUE_
                static MVOID* endeque_Thread(MVOID *arg);
            #endif

        public:
             IMemDrv *mpIMemDrv;
             int mode;//0: preview / else:capture

        private:
            /************************
            * NormalPipe
            ************************/
            INormalPipe* mpCamIO;


            /************************
            * mem
            ************************/
            QBufInfo rRawBuf;
            QBufInfo rDummyBuf;
            QBufInfo rDequeBuf;

            vector<IMEM_BUF_INFO> vRawMem;
            vector<IMEM_BUF_INFO> vDequeMem;
            vector<IMEM_BUF_INFO> vDummyMem;
            MUINT32 mu4SkipFrame;

            /************************
            * Sensor
            ************************/
            SensorHal* mpSensorHal;
            MBOOL mfgIsYUVPortON;
            MINT32 continuous;
            MINT32 is_yuv_sensor;//cfg_sensor_yuv;

//            MUINT32 mu4DeviceID;
            halSensorDev_e sensorDevId;
            MUINT32 u4Scenario;
            MUINT32 u4Bitdepth;

            /*sensor width/height*/
            MUINT32 u4SensorFullWidth;
            MUINT32 u4SensorFullHeight;
            MUINT32 u4SensorHalfWidth;
            MUINT32 u4SensorHalfHeight;

            MUINT32 u4SensorWidth;
            MUINT32 u4SensorHeight;

#ifdef _USE_THREAD_QUE_ //for enque/deque thread
            pthread_t m_TestEnDequethread;
            Mutex mLock; //mutable
            volatile MINT32 mInitCount;
            sem_t m_semTestEnDequethread;
#endif
        };
};
};
};
#endif
