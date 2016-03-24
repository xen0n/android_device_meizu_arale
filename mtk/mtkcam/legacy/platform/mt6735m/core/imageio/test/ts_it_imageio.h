/**
* @file ts_it_imageio.h
*
* TS_IT thread Header File
*/

//! \file  ts_camio.cpp
//! \brief
#define _USE_IMAGIO_

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

////camio
//#include <mtkcam/campipe/_buffer.h>
//#include <mtkcam/campipe/pipe_mgr_drv.h>
//#include <mtkcam/campipe/IPipe.h>
//#include <mtkcam/campipe/ICamIOPipe.h>

//imageio
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/imageio/ispio_utility.h>
//#include "PipeImp.h"//../pipe/inc/
//#include "CamIOPipe.h"//../pipe/inc/
//#include "CampipeImgioPipeMapper.h" //core\campipe\inc

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

#define FAKE_SENSOR         (0)

/*******************************************************************************
*
********************************************************************************/
#if FAKE_SENSOR == 1
#include "ts_fake_sensor.h"
#endif

namespace NSImageio {
namespace NSIspio{
    class Ts_IT
    {

        public:     ////    Constructor/Destructor.
            Ts_IT();
            /**
            * @brief  Destructor
            */
            virtual ~Ts_IT();

        public:
            virtual int startPreview(int count);
            virtual int sensorInit();
            virtual int sensorUninit();
            virtual MVOID freeRawMem();

            virtual MVOID TS_Thread_Init(int count);
            virtual MBOOL TS_Thread_UnInit();

        private:

            /* Inport & OutPort settings */
            virtual MBOOL setConfigPortInfo();
            virtual MBOOL skipFrame(MUINT32 const u4SkipCount);

            virtual MBOOL start();
//            virtual MBOOL dequeHWBuf(MUINT32 const u4TimeoutMs = 0xFFFFFFFF);


            static MVOID* previewProc(MVOID *arg);
//            virtual MBOOL enqueBuf(NSCamPipe::PortID const ePortID, NSCamPipe::QBufInfo const& rQBufInfo);
//            virtual MBOOL dequeBuf(NSCamPipe::PortID const ePortID, NSCamPipe::QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

        public:
             IMemDrv *mpIMemDrv;
             int mode;//0: preview / else:capture
             char            *mSensorCallerName;

        private:
            MUINT32         mSensorDev;
            MUINT32         mSensorIndex;

            ICamIOPipe      *mpCamIOPipe;
            IPostProcPipe   *mpPostProcPipe;

            #if FAKE_SENSOR == 1
            TS_FakeSensor   *mpSensorHal;
            #else
            IHalSensor      *mpSensorHal;
            #endif
            SensorStaticInfo    sensorStaticInfo[3];
            SensorDynamicInfo   sensorDynamicInfo[3];

            //NSCamPipe::SensorPortInfo mrSensorPortInfo;
//            NSCamPipe::QTimeStampBufInfo mrRawQTBufInfo;
//            NSCamPipe::QTimeStampBufInfo mrYuvQTBufInfo;
//            NSCamPipe::QBufInfo rRawBuf;

            vector<IMEM_BUF_INFO>   vRawMem;
            IMEM_BUF_INFO           dispoBuf;
            IMEM_BUF_INFO           vidoBuf;

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

            pthread_t m_TestEnDequethread;
            Mutex mLock; //mutable
            volatile MINT32 mInitCount;
            sem_t m_semTestEnDequethread;
        };

}
}
