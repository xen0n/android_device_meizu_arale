/**
* @file ts_UT.h
*
* ts_UT thread Header File
*/

//! \file  ts_camio.cpp
//! \brief
#define _USE_IMAGIO_
#if 1
#define _USE_THREAD_QUE_
#endif

#define LOG_TAG "ImageioTest"

#include <vector>

using namespace std;

//#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/drv/res_mgr_drv.h>

//camio
#include <mtkcam/campipe/_buffer.h>
#include <mtkcam/campipe/pipe_mgr_drv.h>
#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/ICamIOPipe.h>

//imageio
#ifdef _USE_IMAGIO_

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/ispio_utility.h>
#include "PipeImp.h"//../pipe/inc/
#include "CamIOPipe.h"//../pipe/inc/
#include "CampipeImgioPipeMapper.h" //core\campipe\inc

//using namespace NSImageio;
//using namespace NSIspio;

#else

using namespace NSCamPipe;

#endif

//
//#include <mtkcam/hal/sensor_hal.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/drv_common/imem_drv.h>

//thread
#include <semaphore.h>
#include <pthread.h>
#include <utils/threads.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <mtkcam/v1/config/PriorityDefs.h>

using namespace android;

/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)

#define FAKE_SENSOR     (0)

#ifndef _MTK_CAMERA_CORE_CAMIOPIPE_TEST_UT_H_
#define _MTK_CAMERA_CORE_CAMIOPIPE_TEST_UT_H_

/*******************************************************************************
*
********************************************************************************/
#if FAKE_SENSOR == 1
#include "ts_fake_sensor.h"
#endif

namespace NSImageio {
namespace NSIspio{
    class Ts_UT
    {

        public:     ////    Constructor/Destructor.
            Ts_UT();
            /**
            * @brief  Destructor
            */
            virtual ~Ts_UT();

        public:
            virtual int main_ts_CamIO_ZSD(int count);
            virtual int main_SetSensor_init();
            virtual int main_SetSensor_uninit();
            virtual MVOID freeRawMem();

            #ifdef _USE_THREAD_QUE_
                virtual MVOID TS_Thread_Init(int count);
                virtual MBOOL TS_Thread_UnInit();
            #endif // _USE_THREAD_QUE_

        private:

            /* Inport & OutPort settings */
            //virtual MBOOL queryPipeProperty(NSCamPipe::ESWScenarioID const eSWScenarioID,vector<NSCamPipe::PortProperty> &vInPorts, vector<NSCamPipe::PortProperty> &vOutPorts);
            //virtual MBOOL querySensorInfo(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4BitDepth, EImageFormat &eFmt,  MUINT32 &u4Width, MUINT32 &u4Height, MUINT32 & u4RawPixelID);
            //virtual MBOOL configSensor(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4Width, MUINT32 const u4Height, MUINT32 const byPassDelay, MUINT32 const u4ByPassScenario, MBOOL const fgIsContinuous);
            virtual MBOOL setConfigPortInfo();
            virtual MBOOL skipFrame(MUINT32 const u4SkipCount);

            virtual MBOOL start();
            //virtual MBOOL dequeHWBuf(MUINT32 const u4TimeoutMs = 0xFFFFFFFF);


            #ifdef _USE_THREAD_QUE_
                static MVOID* endeque_Thread(MVOID *arg);
            #endif
            //virtual MBOOL enqueBuf(NSCamPipe::PortID const ePortID, NSCamPipe::QBufInfo const& rQBufInfo);
            //virtual MBOOL dequeBuf(NSCamPipe::PortID const ePortID, NSCamPipe::QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

        public:
             IMemDrv *mpIMemDrv;
             int mode;//0: preview / else:capture
             char            *mSensorCallerName;

        private:
            MUINT32         mSensorDev;
            MUINT32         mSensorIndex;
            ICamIOPipe      *mpCamIOPipe;
#if FAKE_SENSOR == 1
            TS_FakeSensor   *mpSensorHal;
#else
            IHalSensor      *mpSensorHal;
#endif
            SensorStaticInfo    sensorStaticInfo[3];
            SensorDynamicInfo   sensorDynamicInfo[3];

            //NSCamPipe::SensorPortInfo mrSensorPortInfo;
            NSCamPipe::QTimeStampBufInfo mrRawQTBufInfo;
            NSCamPipe::QTimeStampBufInfo mrYuvQTBufInfo;
            NSCamPipe::QBufInfo rRawBuf;

            vector<IMEM_BUF_INFO> vRawMem;

            MUINT32 mu4DeviceID;
            MUINT32 mu4SkipFrame;

            /************************
            * Sensor settings
            ************************/
            MBOOL mfgIsYUVPortON;
            MINT32 continuous;
            MINT32 is_yuv_sensor;//cfg_sensor_yuv;

            //halSensorDev_e sensorDevId;

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
#endif

}
}
