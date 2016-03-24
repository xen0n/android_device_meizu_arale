/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _IMGSENSOR_DRV_H
#define _IMGSENSOR_DRV_H

#include "sensor_drv.h"
#include "camera_custom_sensor.h"
#include "kd_camera_feature.h"
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/log.h>

using namespace android;

#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    ALOGD("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    ALOGD("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    ALOGE("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#include "uvvf.h"

#if 1
#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define LOG_MSG(fmt, arg...)
#define LOG_WRN(fmt, arg...)
#define LOG_ERR(fmt, arg...)
#endif
#endif


/*******************************************************************************
*
********************************************************************************/

//#define YUV_TUNING_SUPPORT


/*******************************************************************************
*
********************************************************************************/
class ImgSensorDrv : public SensorDrv {
public:
    static SensorDrv* getInstance();

private:
    ImgSensorDrv();
    virtual ~ImgSensorDrv();

public:
    virtual void destroyInstance();

public:

    virtual MINT32 init(MINT32 sensorIdx);
    virtual MINT32 uninit();

    virtual MINT32 open(MINT32 sensorIdx);
    virtual MINT32 close(MINT32 sensorIdx);

    virtual MINT32 setScenario(SENSOR_DRIVER_SCENARIO_T scenarioconf);

    virtual MINT32 start();
    virtual MINT32 stop();
    virtual MINT32 waitSensorEventDone( MUINT32 EventType, MUINT32 Timeout);//HDR

    virtual MINT32 getInfo(MUINT32 ScenarioId[2],ACDK_SENSOR_INFO_STRUCT *pSensorInfo[2],ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData[2]);
    virtual MINT32 getInfo2(SENSOR_DEV_ENUM sensorDevId, SENSORDRV_INFO_STRUCT *pSensorInfo);
    virtual MINT32 getResolution(ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution[2]);

    virtual MINT32 sendCommand( SENSOR_DEV_ENUM sensorDevId, MUINT32 cmd, MUINTPTR parg1 = 0, MUINTPTR parg2 = 0, MUINTPTR parg3 = 0);

    virtual MINT32 setFoundDrvsActive(MUINT32 socketIdxes);
    virtual MUINT32 getMainSensorID() const { return m_mainSensorId; }
    virtual MUINT32 getMain2SensorID() const { return m_main2SensorId; }
    virtual MUINT32 getSubSensorID() const { return m_subSensorId; }
    virtual IMAGE_SENSOR_TYPE getCurrentSensorType(SENSOR_DEV_ENUM sensorDevId);
    virtual NSFeature::SensorInfoBase*  getMainSensorInfo() const { return  m_pMainSensorInfo; }
    virtual NSFeature::SensorInfoBase*  getMain2SensorInfo() const { return  m_pMain2SensorInfo; }
    virtual NSFeature::SensorInfoBase*  getSubSensorInfo()  const { return  m_pSubSensorInfo;  }
    //
    virtual MINT32 releaseI2CTriggerLock();
    virtual MINT32 impSearchSensor(pfExIdChk pExIdChkCbf);//6593
private:
    //virtual MINT32 impSearchSensor(pfExIdChk pExIdChkCbf);//6593
    MINT32 featureControl(CAMERA_DUAL_CAMERA_SENSOR_ENUM InvokeCamera, ACDK_SENSOR_FEATURE_ENUM FeatureId,  UINT8 *pFeaturePara,MUINT32 *pFeatureParaLen);
    MINT32 getSensorDelayFrameCnt(SENSOR_DEV_ENUM sensorDevId, MUINT32 mode);
    IMGSENSOR_SOCKET_POSITION_ENUM getSocketPosition(CAMERA_DUAL_CAMERA_SENSOR_ENUM socket);

#if defined (YUV_TUNING_SUPPORT)
    void customerInit(void);
    char* getHexToken(char *inStr, MUINT32 *outVal);
#endif
public:
    static    ImgSensorDrv*            singleton();


private:
    int     m_fdSensor;
    //int     m_userCnt;

    MUINT32  m_LineTimeInus[2];

    MUINT32     m_mainSensorId;
    MUINT32     m_main2SensorId;
    MUINT32     m_subSensorId;

    enum { BAD_SENSOR_INDEX = 0xFF };
    UINT8   m_mainSensorIdx;
    UINT8   m_main2SensorIdx;
    UINT8   m_subSensorIdx;

    SENSOR_DRIVER_LIST_T m_mainSensorDrv;
    SENSOR_DRIVER_LIST_T m_main2SensorDrv;
    SENSOR_DRIVER_LIST_T m_subSensorDrv;

    NSFeature::SensorInfoBase*  m_pMainSensorInfo;
    NSFeature::SensorInfoBase*  m_pMain2SensorInfo;
    NSFeature::SensorInfoBase*  m_pSubSensorInfo;

    MSDK_SENSOR_INIT_FUNCTION_STRUCT*   m_pstSensorInitFunc;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT  m_SenosrResInfo[2];
    ACDK_SENSOR_INFO_STRUCT m_sensorInfo[2];
    ACDK_SENSOR_INFO_STRUCT* m_psensorInfo[2];
    ACDK_SENSOR_CONFIG_STRUCT m_sensorConfigData[2];
    ACDK_SENSOR_CONFIG_STRUCT* m_psensorConfigData[2];
    MUINT16 mImageTargetWidth;
    MUINT16 mImageTargetHeight;

    //pthread_mutex_t                     m_sensorMutex;
    volatile int mUsers;
    mutable Mutex mLock;

};

/*******************************************************************************
*
********************************************************************************/

#endif // _IMGSENSOR_DRV_H

