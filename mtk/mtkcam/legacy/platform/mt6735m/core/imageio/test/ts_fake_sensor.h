/**
* @file ts_UT.h
*
* ts_UT thread Header File
*/

//! \file  ts_camio.cpp
//! \brief

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
//
////imageio
//
//#include <mtkcam/imageio/IPipe.h>
//#include <mtkcam/imageio/ICamIOPipe.h>
//#include <mtkcam/imageio/ispio_utility.h>
//#include "PipeImp.h"//../pipe/inc/
//#include "CamIOPipe.h"//../pipe/inc/

#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/drv_common/imem_drv.h>
#include <mtkcam/drv/isp_drv.h>

//thread
#include <semaphore.h>
#include <pthread.h>
#include <utils/threads.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <mtkcam/v1/config/PriorityDefs.h>


/*******************************************************************************
*
********************************************************************************/
#define TS_LOGD(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define TS_LOGE(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)

/*******************************************************************************
*
********************************************************************************/

class TS_FakeSensor {
public:
    TS_FakeSensor():
        m_pIspDrv(NULL),
        m_pIspReg(NULL)
        {}

    IspDrv      *m_pIspDrv;
    isp_reg_t   *m_pIspReg;

    MVOID                   destroyInstance(
                                        char const* szCallerName = ""
                                    ) {}

    MBOOL                   powerOn(
                                           char const* szCallerName,
                                           MUINT const uCountOfIndex,
                                           MUINT const*pArrayOfIndex
                                    );
    MBOOL                   powerOff(
                                          char const* szCallerName,
                                          MUINT const uCountOfIndex,
                                          MUINT const*pArrayOfIndex
                                    );
    MBOOL                   configure(
                                            MUINT const         uCountOfParam,
                                            IHalSensor::ConfigParam const*  pArrayOfParam
                                        );
    MINT                    sendCommand(
                                            MUINT sensorDevIdx,
                                            MUINTPTR cmd,
                                            MUINTPTR arg1,
                                            MUINTPTR arg2,
                                            MUINTPTR arg3
                                    ) { return 1; }
    MBOOL                   querySensorDynamicInfo(
                                      MUINT32 sensorIdx,
                                      SensorDynamicInfo *pSensorDynamicInfo
                                   );

};

class TS_FakeSensorList {
public:
    static TS_FakeSensorList*   get();

    MUINT                   queryNumberOfSensors() { return 1; }
    char const*             queryDriverName(MUINT const index) { return "Fakegarnett"; }
    MUINT                   querySensorDevIdx(MUINT const index) { return 1; };
    MUINT                   searchSensors() { return 0; }
    TS_FakeSensor*          createSensor(
                                char const* szCallerName,
                                MUINT const uCountOfIndex,
                                MUINT const*pArrayOfIndex
                            );
    MVOID                   querySensorStaticInfo(
                                    MUINT sensorDevIdx,
                                    SensorStaticInfo *pSensorStaticInfo
                                );
};


