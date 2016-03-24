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

#define LOG_TAG "ImgSensorDrv"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//
#include <camera_custom_imgsensor_cfg.h>
#include "imgsensor_drv.h"
#include "kd_imgsensor.h"


MUINT32 sensorDrvInit[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};


/******************************************************************************
 *
 ******************************************************************************/
SensorDrv*
SensorDrv::
get()
{

    return ImgSensorDrv::singleton();
}


/******************************************************************************
 *
 ******************************************************************************/
ImgSensorDrv*
ImgSensorDrv::
singleton()
{
    static ImgSensorDrv inst;
    return &inst;
}

/*******************************************************************************
*
********************************************************************************/
SensorDrv*
ImgSensorDrv::
getInstance()
{
    LOG_MSG("[ImgSensorDrv] getInstance \n");
    static ImgSensorDrv singleton;

    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
ImgSensorDrv::
destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
ImgSensorDrv()
    : SensorDrv()
    , m_fdSensor(-1)
    , m_mainSensorId(SENSOR_DOES_NOT_EXIST)
    , m_main2SensorId(SENSOR_DOES_NOT_EXIST)
    , m_subSensorId(SENSOR_DOES_NOT_EXIST)
    , m_mainSensorIdx(BAD_SENSOR_INDEX)
    , m_main2SensorIdx(BAD_SENSOR_INDEX)
    , m_subSensorIdx(BAD_SENSOR_INDEX)
    , m_pMainSensorInfo(NULL)
    , m_pSubSensorInfo(NULL)
    , m_pstSensorInitFunc(NULL)
    , mUsers(0)

{
    m_LineTimeInus[0] = 31;
    m_LineTimeInus[1] = 31;
    memset(&m_SenosrResInfo[0], 0, sizeof(ACDK_SENSOR_RESOLUTION_INFO_STRUCT));
    memset(&m_SenosrResInfo[1], 0, sizeof(ACDK_SENSOR_RESOLUTION_INFO_STRUCT));
    //
    memset((void*)&m_mainSensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    memset((void*)&m_main2SensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    memset((void*)&m_subSensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    m_mainSensorDrv.number = 0;
    m_main2SensorDrv.number = 0;
    m_subSensorDrv.number = 0;
    memset(&m_sensorInfo[0], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&m_sensorInfo[1], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&m_sensorConfigData[0], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    memset(&m_sensorConfigData[1], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    m_psensorInfo[0] = &m_sensorInfo[0];
    m_psensorInfo[1] = &m_sensorInfo[1];
    m_psensorConfigData[0] = &m_sensorConfigData[0];
    m_psensorConfigData[1] = &m_sensorConfigData[1];

}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
~ImgSensorDrv()
{
    m_mainSensorIdx = BAD_SENSOR_INDEX;
    m_main2SensorIdx = BAD_SENSOR_INDEX;
    m_subSensorIdx = BAD_SENSOR_INDEX;
    LOG_MSG ("[~ImgSensorDrv]\n");
}

/*******************************************************************************
*
********************************************************************************/

MINT32
ImgSensorDrv::impSearchSensor(pfExIdChk pExIdChkCbf)
{
    MUINT32 SensorEnum = (MUINT32) DUAL_CAMERA_MAIN_SENSOR;
    MUINT32 i,id[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};
    MBOOL SensorConnect=TRUE;
    UCHAR cBuf[64];
    MINT32 err = SENSOR_NO_ERROR;
    MINT32 err2 = SENSOR_NO_ERROR;
    ACDK_SENSOR_INFO_STRUCT SensorInfo;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT SensorResolution;
    MINT32 sensorDevs = SENSOR_NONE;
    IMAGE_SENSOR_TYPE sensorType = IMAGE_SENSOR_TYPE_UNKNOWN;
    IMGSENSOR_SOCKET_POSITION_ENUM socketPos = IMGSENSOR_SOCKET_POS_NONE;


    //! If imp sensor search process already done before,
    //! only need to return the sensorDevs, not need to
    //! search again.
    if (SENSOR_DOES_NOT_EXIST != m_mainSensorId) {
        //been processed.
        LOG_MSG("[impSearchSensor] Already processed \n");
        if (BAD_SENSOR_INDEX != m_mainSensorIdx) {
            sensorDevs |= SENSOR_MAIN;
        }
        if (BAD_SENSOR_INDEX != m_main2SensorIdx) {
            sensorDevs |= SENSOR_MAIN_2;
        }
        if (BAD_SENSOR_INDEX != m_subSensorIdx) {
            sensorDevs |= SENSOR_SUB;
        }

        #ifdef  ATVCHIP_MTK_ENABLE

            sensorDevs |= SENSOR_ATV;

        #endif


        return sensorDevs;
    }

    GetSensorInitFuncList(&m_pstSensorInitFunc);

    LOG_MSG("SENSOR search start \n");

    if (-1 != m_fdSensor) {
        ::close(m_fdSensor);
        m_fdSensor = -1;
    }
    sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
    m_fdSensor = ::open(cBuf, O_RDWR);
    if (m_fdSensor < 0) {
         LOG_ERR("[impSearchSensor]: error opening %s: %s \n", cBuf, strerror(errno));
        return sensorDevs;
    }

    // search main/main_2/sub 3 sockets
#ifdef MTK_MAIN2_IMGSENSOR
    for (SensorEnum = DUAL_CAMERA_MAIN_SENSOR; SensorEnum <= DUAL_CAMERA_MAIN_2_SENSOR; SensorEnum <<= 1)  {
        LOG_MSG("impSearchSensor search to main_2\n");
#else
   #ifdef MTK_SUB_IMGSENSOR
    for (SensorEnum = DUAL_CAMERA_MAIN_SENSOR; SensorEnum <= DUAL_CAMERA_SUB_SENSOR; SensorEnum <<= 1)  {
        LOG_MSG("impSearchSensor search to sub\n");
   #else
    for (SensorEnum = DUAL_CAMERA_MAIN_SENSOR; SensorEnum < DUAL_CAMERA_SUB_SENSOR; SensorEnum <<= 1)  {
        LOG_MSG("impSearchSensor search to main\n");
   #endif
#endif


        //
        for (i = 0; i < MAX_NUM_OF_SUPPORT_SENSOR; i++) {
            //end of driver list
            if (m_pstSensorInitFunc[i].getCameraDefault == NULL) {
                LOG_MSG("m_pstSensorInitFunc[i].getCameraDefault is NULL: %d \n", i);
                break;
            }
                //set sensor driver
            id[KDIMGSENSOR_INVOKE_DRIVER_0] = (SensorEnum << KDIMGSENSOR_DUAL_SHIFT) | i;
            LOG_MSG("set sensor driver id =%x\n", id[KDIMGSENSOR_INVOKE_DRIVER_0]);
            err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,&id[KDIMGSENSOR_INVOKE_DRIVER_0] );
                if (err < 0) {
                    LOG_ERR("ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
                }


                //err = open();
                err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_CHECK_IS_ALIVE);


                if (err < 0) {
                    LOG_MSG("[impSearchSensor] Err-ctrlCode (%s) \n", strerror(errno));
                }
            //
            sensorType = this->getCurrentSensorType((SENSOR_DEV_ENUM)SensorEnum);
            //
            socketPos = this->getSocketPosition((CAMERA_DUAL_CAMERA_SENSOR_ENUM)SensorEnum);
                //check extra ID , from EEPROM maybe
                //may need to keep power here
                if (NULL != pExIdChkCbf) {
                    err2 = pExIdChkCbf();
                    if (err2 < 0) {
                        LOG_ERR("Error:pExIdChkCbf() \n");
                    }
                }

                //power off sensor
                //close(SensorEnum);//ToDo: Check if necessary

                if (err < 0 || err2 < 0) {
                    LOG_MSG("sensor ID mismatch\n");
                }
                else {
                    if (SensorEnum == DUAL_CAMERA_MAIN_SENSOR) {
                //m_mainSensorIdx = i;
                //m_mainSensorId = m_pstSensorInitFunc[m_mainSensorIdx].SensorId;
                m_mainSensorDrv.index[m_mainSensorDrv.number] = i;
                m_mainSensorDrv.type[m_mainSensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_mainSensorDrv.firstRawIndex ) {
                    m_mainSensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_mainSensorDrv.firstYuvIndex ) {
                    m_mainSensorDrv.firstYuvIndex = i;
                }
                m_mainSensorDrv.position = socketPos;
                m_mainSensorDrv.sensorID = m_pstSensorInitFunc[m_mainSensorDrv.index[m_mainSensorDrv.number]].SensorId;
                // LOG_MSG("MAIN sensor m_mainSensorDrv.number=%d, m_mainSensorDrv.index=%d\n",m_mainSensorDrv.number,m_mainSensorDrv.index[m_mainSensorDrv.number]);
                m_mainSensorDrv.number++;
                //
                m_pMainSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                if  ( m_pMainSensorInfo )
                {
                    NSFeature::SensorInfoBase* pSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                    LOG_MSG("found <%#x/%s/%s>", pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
                }
                else
                {
                    LOG_WRN("m_pMainSensorInfo==NULL\n");
                }
                LOG_MSG("MAIN sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
                //break;
            }
            else if (SensorEnum == DUAL_CAMERA_MAIN_2_SENSOR) {
                //m_main2SensorIdx = i;
                //m_main2SensorId = m_pstSensorInitFunc[m_main2SensorIdx].SensorId;

                m_main2SensorDrv.index[m_main2SensorDrv.number] = i;
                m_main2SensorDrv.type[m_main2SensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_main2SensorDrv.firstRawIndex ) {
                    m_main2SensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_main2SensorDrv.firstYuvIndex ) {
                    m_main2SensorDrv.firstYuvIndex = i;
                }
                m_main2SensorDrv.position = socketPos;
                m_main2SensorDrv.sensorID = m_pstSensorInitFunc[m_main2SensorDrv.index[m_main2SensorDrv.number]].SensorId;
                //LOG_MSG("MAIN2 sensor m_main2SensorDrv.number=%d, m_main2SensorDrv.index=%d\n",m_main2SensorDrv.number,m_main2SensorDrv.index[m_main2SensorDrv.number]);
                m_main2SensorDrv.number++;
                //
                m_pMain2SensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                if  ( m_pMain2SensorInfo )
                {
                    NSFeature::SensorInfoBase* pSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                    LOG_MSG("found <%#x/%s/%s>", pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
                }
                else
                {
                    LOG_WRN("m_pMain2SensorInfo==NULL\n");
                }
                LOG_MSG("MAIN_2 sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
            }
            else if (SensorEnum == DUAL_CAMERA_SUB_SENSOR) {
                //m_subSensorIdx = i;
                //m_subSensorId = m_pstSensorInitFunc[m_subSensorIdx].SensorId;
                m_subSensorDrv.index[m_subSensorDrv.number] = i;
                m_subSensorDrv.type[m_subSensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_subSensorDrv.firstRawIndex ) {
                    m_subSensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_subSensorDrv.firstYuvIndex ) {
                    m_subSensorDrv.firstYuvIndex = i;
                }
                m_subSensorDrv.position = socketPos;
                m_subSensorDrv.sensorID = m_pstSensorInitFunc[m_subSensorDrv.index[m_subSensorDrv.number]].SensorId;
                //LOG_MSG("SUB sensor m_subSensorDrv.number=%d, m_subSensorDrv.index=%d\n",m_subSensorDrv.number,m_subSensorDrv.index[m_subSensorDrv.number]);
                m_subSensorDrv.number++;
                //
                m_pSubSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                if  ( m_pSubSensorInfo )
                {
                    NSFeature::SensorInfoBase* pSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                    LOG_MSG("found <%#x/%s/%s>", pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
                }
                else
                {
                    LOG_WRN("m_pSubSensorInfo==NULL\n");
                }
                LOG_MSG("SUB sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
                //break;
            }
        }//

        }
    }
    //close system call may be off sensor power. check first!!!
    if(m_fdSensor >= 0)
    {
        ::close(m_fdSensor);
        m_fdSensor = -1;
    }

    //
    if (BAD_SENSOR_INDEX != m_mainSensorDrv.index[0]) {
        m_mainSensorId = m_mainSensorDrv.sensorID;
        //init to choose first
        m_mainSensorIdx = m_mainSensorDrv.index[0];
        sensorDevs |= SENSOR_MAIN;
    }
    if (BAD_SENSOR_INDEX != m_main2SensorDrv.index[0]) {
        m_main2SensorId = m_main2SensorDrv.sensorID;
        //init to choose first
        m_main2SensorIdx = m_main2SensorDrv.index[0];
        sensorDevs |= SENSOR_MAIN_2;
    }
    if (BAD_SENSOR_INDEX != m_subSensorDrv.index[0]) {
        m_subSensorId = m_subSensorDrv.sensorID;
        //init to choose first
        m_subSensorIdx = m_subSensorDrv.index[0];
        sensorDevs |= SENSOR_SUB;
    }

    #ifdef  ATVCHIP_MTK_ENABLE

        sensorDevs |= SENSOR_ATV;

    #endif


    if (sensorDevs == SENSOR_NONE) {
        LOG_ERR( "Error No sensor found!! \n");
    }
    //
    LOG_MSG("SENSOR search end: 0x%x /[0x%x][%d]/[0x%x][%d]/[0x%x][%d]\n", sensorDevs,
    m_mainSensorId,m_mainSensorIdx,m_main2SensorId,m_main2SensorIdx,m_subSensorId,m_subSensorIdx);

    return sensorDevs;
}//


/*******************************************************************************
*
********************************************************************************/

#define N2D_PRIORITY_DRIVER Yuv
#define N3D_PRIORITY_DRIVER Yuv
#define _SELECT_PRIORITY_DRIVER_(a,b)    do { if ( m_##a##SensorDrv.number > 1 && BAD_SENSOR_INDEX != m_##a##SensorDrv.first##b##Index ) { \
                                        m_##a##SensorIdx = m_##a##SensorDrv.first##b##Index; }}while(0)
#define SELECT_PRIORITY_DRIVER(a,b) _SELECT_PRIORITY_DRIVER_(a,b)

//modify sub
#define IMGSNESOR_FILL_SET_DRIVER_INFO(a) do{ \
        if ( DUAL_CAMERA_MAIN_SENSOR & a ) {    \
            sensorDrvInit[KDIMGSENSOR_INVOKE_DRIVER_0] = \
                    (DUAL_CAMERA_MAIN_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_mainSensorIdx; \
        }   \
        if ( DUAL_CAMERA_MAIN_2_SENSOR & a ) {  \
            sensorDrvInit[KDIMGSENSOR_INVOKE_DRIVER_1] = \
                    (DUAL_CAMERA_MAIN_2_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_main2SensorIdx;   \
        }   \
        if ( DUAL_CAMERA_SUB_SENSOR & a ) { \
            sensorDrvInit[KDIMGSENSOR_INVOKE_DRIVER_1] = \
                    (DUAL_CAMERA_SUB_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_subSensorIdx;    \
        }\
    }while(0);

MINT32
ImgSensorDrv::init(
    MINT32 sensorIdx
)
{
    UCHAR cBuf[64];
    MINT32 ret = SENSOR_NO_ERROR;
    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;
    //MUINT32 sensorDrvInfo[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};
    IMAGE_SENSOR_TYPE sensorType_prioriy = IMAGE_SENSOR_TYPE_UNKNOWN;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResInfo[2];
    SENSOR_DEV_ENUM sensorDevId;


    if (  0 == sensorIdx ) {
        LOG_ERR("invalid sensorIdx[0x%08x] \n",sensorIdx);
        return SENSOR_INVALID_PARA;
    }
    //select driver
    if ( ( DUAL_CAMERA_MAIN_SENSOR|DUAL_CAMERA_MAIN_2_SENSOR ) == sensorIdx ) {
        //N3D mode
        SELECT_PRIORITY_DRIVER(main,N3D_PRIORITY_DRIVER);
    }
    else {
        //2D mode
        SELECT_PRIORITY_DRIVER(main,N2D_PRIORITY_DRIVER);
        //SELECT_PRIORITY_DRIVER(main2,N2D_PRIORITY_DRIVER);
        SELECT_PRIORITY_DRIVER(sub,N2D_PRIORITY_DRIVER);
    }
    //
    IMGSNESOR_FILL_SET_DRIVER_INFO(sensorIdx);
    //
    LOG_MSG("[init] mUsers = %d\n", mUsers);
    Mutex::Autolock lock(mLock);

    if (mUsers == 0) {
        if (m_fdSensor == -1) {
            sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
            m_fdSensor = ::open(cBuf, O_RDWR);
            if (m_fdSensor < 0) {
                LOG_ERR("[init]: error opening %s: %s \n", cBuf, strerror(errno));
                return SENSOR_INVALID_DRIVER;
            }
        }
    }


    //set sensor driver
    ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,sensorDrvInit);
    if (ret < 0) {
       LOG_ERR("[init]: ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
    }

    android_atomic_inc(&mUsers);

    //init. resolution
    pSensorResInfo[0] = &m_SenosrResInfo[0];
    pSensorResInfo[1] = &m_SenosrResInfo[1];

    ret = getResolution(pSensorResInfo);
    if (ret < 0) {
        LOG_ERR("[init]: Get Resolution error\n");
        return SENSOR_UNKNOWN_ERROR;
    }


    if(SENSOR_MAIN & sensorIdx ) {
        sensorDevId = SENSOR_MAIN;

        //calculater g_LineTimeInus for exposure time convert.
        FeatureParaLen = sizeof(MUINTPTR);
        LOG_MSG("[featureControl]: SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ\n");
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
           LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
           return SENSOR_UNKNOWN_ERROR;
        }

        FeatureParaLen = sizeof(pFeaturePara16);
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
            LOG_ERR("[init]: SENSOR_FEATURE_GET_PERIOD error\n");
            return SENSOR_UNKNOWN_ERROR;
        }

        if (FeaturePara32) {
            //in setting domain, use preview line time only
            //sensor drv will convert to capture line time when setting to capture mode.
            m_LineTimeInus[0] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000+(FeaturePara32>>1))/FeaturePara32);
        }
        LOG_MSG("[init]: m_LineTimeInus[0] = %d\n", m_LineTimeInus[0] );
    }

    if((SENSOR_MAIN_2 & sensorIdx)|| (SENSOR_SUB & sensorIdx)) {
        if(SENSOR_MAIN_2 & sensorIdx) {
            sensorDevId = SENSOR_MAIN_2;
        }
        else {
            sensorDevId = SENSOR_SUB;
        }

        //calculater g_LineTimeInus for exposure time convert.
        FeatureParaLen = sizeof(MUINTPTR);
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
           LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
           return SENSOR_UNKNOWN_ERROR;
        }

        FeatureParaLen = sizeof(pFeaturePara16);
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
            LOG_ERR("[init]: SENSOR_FEATURE_GET_PERIOD error\n");
            return SENSOR_UNKNOWN_ERROR;
        }

        if (FeaturePara32) {
            //in setting domain, use preview line time only
            //sensor drv will convert to capture line time when setting to capture mode.
            m_LineTimeInus[1] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000+(FeaturePara32>>1))/FeaturePara32);
        }
        LOG_MSG("[init]: m_LineTimeInus[1] = %d\n", m_LineTimeInus[1] );
    }


    return SENSOR_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::uninit(
)
{
    //MHAL_LOG("[halSensorUninit] \n");
    MINT32 ret = SENSOR_NO_ERROR;

#if 0
    ret = close();
    if (ret < 0)
    {
        LOG_ERR("[uninit] SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
        return MHAL_UNKNOWN_ERROR;
    }
#endif

    //pthread_mutex_lock(&m_sensorMutex);
    LOG_MSG("[uninit]imgsensor_drv mUsers = %d\n", mUsers);

    Mutex::Autolock lock(mLock);
    //
    if (mUsers <= 0) {
        // No more users
        return SENSOR_NO_ERROR;
    }

    if (mUsers == 1) {
        sensorDrvInit[KDIMGSENSOR_INVOKE_DRIVER_0] = 0;
        sensorDrvInit[KDIMGSENSOR_INVOKE_DRIVER_1] = 0;
        if (m_fdSensor > 0) {
            ::close(m_fdSensor);
        }
        m_fdSensor = -1;
    }
    android_atomic_dec(&mUsers);
    //m_userCnt --;
    //pthread_mutex_unlock(&m_sensorMutex);


    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::setScenario(SENSOR_DRIVER_SCENARIO_T scenarioconf)
{
    MINT32 ret = SENSOR_NO_ERROR;
    //ToDo: remove
    MUINT32 tempId[2];
    SENSOR_DEV_ENUM tempDevId[2];

 #if 1
    MUINT32 i = 0;
    ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT ImageWindow[2];
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData[2];

    ACDK_SENSOR_CONTROL_STRUCT sensorCtrl;

    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;

    //ToDo: remove
    for(i=0; i<2 ; i++) {
        tempId[i] = scenarioconf.sId;
        tempDevId[i] = scenarioconf.sensorDevId;
    }


    //for(i=0; i<KDIMGSENSOR_MAX_INVOKE_DRIVERS; i++) {
    for(i=0; i<1; i++) {
        if(SENSOR_SCENARIO_ID_UNNAMED_START != tempId[i]) {//ToDo: remove
            //if(ACDK_SCENARIO_ID_MAX != sId[i]) {
            //FPS
            FeaturePara32 = scenarioconf.InitFPS;
            FeatureParaLen = sizeof(MUINTPTR);
            ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)tempDevId[i], SENSOR_FEATURE_SET_FRAMERATE,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
            if (ret < 0) {
                 LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_FRAMERATE error\n");
                 return SENSOR_UNKNOWN_ERROR;
            }
            //HDR Enable
            //if(scenarioconf.HDRMode <= 1)
            {
                FeaturePara32 = scenarioconf.HDRMode;
                FeatureParaLen = sizeof(MUINTPTR);
                ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)tempDevId[i], SENSOR_FEATURE_SET_HDR,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
                if (ret < 0) {
                     LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_FRAMERATE error\n");
                     return SENSOR_UNKNOWN_ERROR;
                }
            }
            SensorConfigData[i].SensorImageMirror = ACDK_SENSOR_IMAGE_NORMAL;

            switch(tempId[i])//ToDo: remove
            //switch(sId[i])
            {
                case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CAMERA_PREVIEW;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorPreviewWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorPreviewHeight;
                    break;
                case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_STILL_CAPTURE;
                    SensorConfigData[i].EnableShutterTansfer = FALSE;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorFullWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorFullHeight;
                    break;
                case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_VIDEO;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorVideoWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorVideoHeight;
                    break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                    tempId[i] = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO;
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_HIGHSP_VIDEO1;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorHighSpeedVideoWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorHighSpeedVideoHeight;
                    break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                    tempId[i] = MSDK_SCENARIO_ID_SLIM_VIDEO;
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_HIGHSP_VIDEO2;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorSlimVideoWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorSlimVideoHeight;
                    break;
                case SENSOR_SCENARIO_ID_CUSTOM1:
                    tempId[i] = MSDK_SCENARIO_ID_CUSTOM1;
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CUSTOM1;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorCustom1Width;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorCustom1Height;
                    break;

                case SENSOR_SCENARIO_ID_CUSTOM2:
                    tempId[i] = MSDK_SCENARIO_ID_CUSTOM2;
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CUSTOM2;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorCustom2Width;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorCustom2Height;
                    break;
                case SENSOR_SCENARIO_ID_CUSTOM3:
                    tempId[i] = MSDK_SCENARIO_ID_CUSTOM3;
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CUSTOM3;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorCustom3Width;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorCustom3Height;
                    break;
                case SENSOR_SCENARIO_ID_CUSTOM4:
                    tempId[i] = MSDK_SCENARIO_ID_CUSTOM4;
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CUSTOM4;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorCustom4Width;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorCustom4Height;
                    break;
                case SENSOR_SCENARIO_ID_CUSTOM5:
                    tempId[i] = MSDK_SCENARIO_ID_CUSTOM5;
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CUSTOM5;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorCustom5Width;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorCustom5Height;
                    break;
                default:
                    LOG_ERR("[setScenario] error scenario id\n");
                    return SENSOR_UNKNOWN_ERROR;
            }

            //set sensor preview/capture mode
            sensorCtrl.InvokeCamera = (CAMERA_DUAL_CAMERA_SENSOR_ENUM)scenarioconf.sensorDevId;//6593

            sensorCtrl.ScenarioId = (MSDK_SCENARIO_ID_ENUM)tempId[i];//ToDo:remove
            //sensorCtrl.ScenarioId = (MSDK_SCENARIO_ID_ENUM)sId[i];
            sensorCtrl.pImageWindow = &ImageWindow[i];
            sensorCtrl.pSensorConfigData = &SensorConfigData[i];

            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_CONTROL , &sensorCtrl);
            if (ret < 0) {
                LOG_ERR("[setScenario]Err-ctrlCode (%s) \n", strerror(errno));
                return -errno;
            }
            //get exposure line time for each scenario
            FeatureParaLen = sizeof(MUINTPTR);
            ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)tempDevId[i],SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);//ToDo: remove
            //ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId[i],SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
            if (ret < 0) {
               LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
               return SENSOR_UNKNOWN_ERROR;
            }

            FeatureParaLen = sizeof(pFeaturePara16);
            ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)tempDevId[i], SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);//ToDo: remove
            //ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId[i], SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
            if (ret < 0) {
                LOG_ERR("[setScenario]: SENSOR_FEATURE_GET_PERIOD error\n");
                return SENSOR_UNKNOWN_ERROR;
            }

            if (FeaturePara32) {
                if(FeaturePara32 >= 1000) {
                    if(SENSOR_MAIN == scenarioconf.sensorDevId) {
                        m_LineTimeInus[0] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000 + ((FeaturePara32/1000)-1))/(FeaturePara32/1000));   // 1000 base , 33657 mean 33.657 us
                    }
                    else{
                        m_LineTimeInus[1] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000 + ((FeaturePara32/1000)-1))/(FeaturePara32/1000));   // 1000 base , 33657 mean 33.657 us
                    }
                    //m_LineTimeInus[i] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000 + ((FeaturePara32/1000)-1))/(FeaturePara32/1000));   // 1000 base , 33657 mean 33.657 us
                }
                else {
                    LOG_ERR("[setScenario]: Sensor clock too slow = %d %d\n", FeaturePara32, pFeaturePara16[0]);
                }
                LOG_MSG("[setScenario]DevID = %d, m_LineTimeInus[0] = %d, m_LineTimeInus[1] = %d Scenario id = %d, PixelClk = %d, PixelInLine = %d\n",
                          tempDevId[i], m_LineTimeInus[0],m_LineTimeInus[1], tempId[i], FeaturePara32, pFeaturePara16[0]);
            }
            //set target width/height
            mImageTargetWidth = pFeaturePara16[0];
            mImageTargetHeight = pFeaturePara16[1];


        }
    }
#endif
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::start(
)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::stop(
)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
/*unsed from now*/
MINT32
ImgSensorDrv::getSensorDelayFrameCnt(
    SENSOR_DEV_ENUM sensorDevId,
    MUINT32 mode
)
{

    MUINT32 scenarioId[2] = {SENSOR_SCENARIO_ID_NORMAL_PREVIEW,SENSOR_SCENARIO_ID_NORMAL_PREVIEW};



    LOG_MSG("[getSensorDelayFrameCnt] mode = %d\n", mode);
    if (SENSOR_NO_ERROR != getInfo(scenarioId, m_psensorInfo, m_psensorConfigData)) {
       LOG_ERR("[searchSensor] Error:getInfo() \n");
       return 0;
    }


    if(SENSOR_MAIN == sensorDevId) {
        if ( SENSOR_PREVIEW_DELAY == mode) {
            return m_psensorInfo[0]->PreviewDelayFrame;
        }
        else if (SENSOR_VIDEO_DELAY == mode) {
            return m_psensorInfo[0]->VideoDelayFrame;
        }
        else if (SENSOR_CAPTURE_DELAY == mode) {
            return m_psensorInfo[0]->CaptureDelayFrame;
        }
        else if (SENSOR_YUV_AWB_SETTING_DELAY == mode) {
            return m_psensorInfo[0]->YUVAwbDelayFrame;
        }
        else if (SENSOR_YUV_EFFECT_SETTING_DELAY == mode) {
            return m_psensorInfo[0]->YUVEffectDelayFrame;
        }
        else if (SENSOR_AE_SHUTTER_DELAY == mode) {
            return m_psensorInfo[0]->AEShutDelayFrame;
        }
        else if (SENSOR_AE_GAIN_DELAY == mode) {
            return m_psensorInfo[0]->AESensorGainDelayFrame;
        }
        else if (SENSOR_AE_ISP_DELAY == mode) {
            return m_psensorInfo[0]->AEISPGainDelayFrame;
        }
        else {
            return 0;
        }
    }
    else if((SENSOR_MAIN_2 == sensorDevId) ||(SENSOR_SUB == sensorDevId)) {
        if ( SENSOR_PREVIEW_DELAY == mode) {
            return m_psensorInfo[1]->PreviewDelayFrame;
        }
        else if (SENSOR_VIDEO_DELAY == mode) {
            return m_psensorInfo[1]->VideoDelayFrame;
        }
        else if (SENSOR_CAPTURE_DELAY == mode) {
            return m_psensorInfo[1]->CaptureDelayFrame;
        }
        else if (SENSOR_YUV_AWB_SETTING_DELAY == mode) {
            return m_psensorInfo[1]->YUVAwbDelayFrame;
        }
        else if (SENSOR_YUV_EFFECT_SETTING_DELAY == mode) {
            return m_psensorInfo[1]->YUVEffectDelayFrame;
        }
        else if (SENSOR_AE_SHUTTER_DELAY == mode) {
            return m_psensorInfo[1]->AEShutDelayFrame;
        }
        else if (SENSOR_AE_GAIN_DELAY == mode) {
            return m_psensorInfo[1]->AESensorGainDelayFrame;
        }
        else if (SENSOR_AE_ISP_DELAY == mode) {
            return m_psensorInfo[1]->AEISPGainDelayFrame;
        }
        else {
            return 0;
        }
    }
    else {
        LOG_ERR("[getSensorDelayFrameCnt] Error sensorDevId ! \n");
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/

//HDR
MINT32
ImgSensorDrv::waitSensorEventDone(
    MUINT32 EventType,
    MUINT32 Timeout
)
{
    MINT32 ret = 0;
    LOG_MSG("[ImgSensorDrv]waitSensorEventDone: EventType = %d, Timeout=%d\n",EventType,Timeout);
    switch (EventType) {
        case WAIT_SENSOR_SET_SHUTTER_GAIN_DONE:
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_SHUTTER_GAIN_WAIT_DONE, &Timeout);
            break;
        default :
            break;
    }
    if(ret < 0)
    {
        LOG_MSG("waitSensorEventDone err, Event = %d", EventType);
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/


MINT32
ImgSensorDrv::sendCommand(
        SENSOR_DEV_ENUM sensorDevId,
        MUINT32 cmd,
        MUINTPTR arg1,
        MUINTPTR arg2,
        MUINTPTR arg3
)
{
    // parg#: pointer to paras, regard pointed variable as MUINT32 type
    /// Notice: Regard the type of varibale used in middleware to be MUINT32
    /// Should discuss this part with middleware when you need to create a new sendCommand enum
    MUINT32* parg1 = (MUINT32*)arg1;
    MUINT32* parg2 = (MUINT32*)arg2;
    MUINT32* parg3 = (MUINT32*)arg3;

    // parg#Addr: pointer to an adress. Regard pointed variable as MUINTPTR type, cause it's a pointer address.
    MUINTPTR* parg1Addr = (MUINTPTR*)parg1;
    MUINTPTR* parg2Addr = (MUINTPTR*)parg2;
    MUINTPTR* parg3Addr = (MUINTPTR*)parg3;

    MINT32 err = SENSOR_NO_ERROR;

    ACDK_SENSOR_FEATURE_ENUM FeatureId = SENSOR_FEATURE_BEGIN;
    MUINT8 *pFeaturePara = NULL; // Pointer to feature data that communicate with kernel
    MUINT32 FeatureParaLen = 0; // The length of feature data

    MUINT64 FeaturePara[4]; // Convert input paras to match the format used in kernel

#if 0
    /// [TODO] Remove these varialbe, only use one FeaturePara[4];
    /// These variables are used to convert input paras to match the format used in kernel (Each command may use difference data format)
    MUINT16 u2FeaturePara=0;
    MUINT32 u4FeaturePara[4];

    MUINT16 *pu2FeaturePara = NULL;
    MUINT32 *pu4Feature = NULL;
    MUINT32 *pu4FeaturePara = NULL;
#endif

//#define SENDCMD_LOG // Open sendcommand log, for test only
    switch (cmd) {


    case CMD_SENSOR_SET_SENSOR_EXP_TIME:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        if(SENSOR_MAIN == sensorDevId) {
            FeaturePara[0] = ((1000 * (*parg1)) / m_LineTimeInus[0]);
            if(FeaturePara[0] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[0], m_LineTimeInus[0]);
                FeaturePara[0] = 1;
            }
            FeatureParaLen = sizeof(MUINT64);
            pFeaturePara = (MUINT8*)FeaturePara;
        }
        else if((SENSOR_MAIN_2 == sensorDevId)||(SENSOR_SUB == sensorDevId)) {
            FeaturePara[0] = ((1000 * (*parg1)) / m_LineTimeInus[1]);
            if(FeaturePara[0] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[0], m_LineTimeInus[1]);
                FeaturePara[0] = 1;
            }
            FeatureParaLen = sizeof(MUINT64);
            pFeaturePara = (MUINT8*)FeaturePara;
        }
        else{
            LOG_ERR("sensorDevId is incorrect!!\n");
        }

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_LINE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_GAIN:
        FeatureId = SENSOR_FEATURE_SET_GAIN;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif

        break;

    case CMD_SENSOR_SET_FLICKER_FRAME_RATE:
       FeatureId = SENSOR_FEATURE_SET_AUTO_FLICKER_MODE;
       FeaturePara[0] = *parg1;
       FeatureParaLen = sizeof(MUINT64);
       pFeaturePara =  (MUINT8*)FeaturePara;
       break;


    case CMD_SENSOR_SET_VIDEO_FRAME_RATE:
        FeatureId = SENSOR_FEATURE_SET_VIDEO_MODE;
        FeaturePara[0]= *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_SYNC;
        if(SENSOR_MAIN == sensorDevId) {
            FeaturePara[0] = *parg1; // RAW Gain R, Gr
            FeaturePara[1] = *(parg1+1);  // RAW Gain Gb, B
            FeaturePara[2] = *(parg1+2);  // Exposure time
            FeaturePara[2] = ((1000 * FeaturePara[2]) / m_LineTimeInus[0]);
            if(FeaturePara[2]  == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC] m_LineTime[0] = %llu %d\n", FeaturePara[2] , m_LineTimeInus[0]);
                FeaturePara[2]  = 1;
            }
            FeaturePara[2] = (FeaturePara[2] ) | (((*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
            FeaturePara[3] = *(parg1+4);  // Delay frame cnt
            FeatureParaLen = sizeof(MUINT64) * 4;
            pFeaturePara = (MUINT8*)FeaturePara;
        }
        else if((SENSOR_MAIN_2 == sensorDevId)||(SENSOR_SUB == sensorDevId)) {
            FeaturePara[0] = *parg1; // RAW Gain R, Gr
            FeaturePara[1] = *(parg1+1);  // RAW Gain Gb, B
            FeaturePara[2] = *(parg1+2);  // Exposure time
            FeaturePara[2] = ((1000 * FeaturePara[2]) / m_LineTimeInus[1]);
            if(FeaturePara[2]  == 0) {   // avoid the line number to zero
               LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC] m_LineTime[1] = %llu %d\n", FeaturePara[2] , m_LineTimeInus[1]);
               FeaturePara[2]  = 1;
            }
            FeaturePara[2] = (FeaturePara[2] ) | (((*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
            FeaturePara[3] = *(parg1+4);  // Delay frame cnt
            FeatureParaLen = sizeof(MUINT64) * 4;
            pFeaturePara = (MUINT8*)FeaturePara;

        }
        else{
            LOG_ERR("sensorDevId is incorrect!! \n");
        }

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;



    case CMD_SENSOR_SET_CCT_FEATURE_CONTROL:
        FeatureId = (ACDK_SENSOR_FEATURE_ENUM)*parg1;
        pFeaturePara = (MUINT8*)parg2;
        FeatureParaLen = (MUINT32)*parg3;
        break;

    case CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA:
        FeatureId = SENSOR_FEATURE_SET_CALIBRATION_DATA;
        pFeaturePara = (UINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_CALIBRATION_DATA_STRUCT);
        break;

    case CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        switch(*parg1)
        {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                FeaturePara[0] = MSDK_SCENARIO_ID_VIDEO_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                FeaturePara[0] = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                FeaturePara[0] = MSDK_SCENARIO_ID_SLIM_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM1;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM2;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM3;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM4;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM5;
                break;
            default:
                LOG_ERR("[setScenario] error scenario id\n");
                return SENSOR_UNKNOWN_ERROR;
        }
        break;

    case CMD_SENSOR_SET_TEST_PATTERN_OUTPUT:
        FeatureId = SENSOR_FEATURE_SET_TEST_PATTERN;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_TEST_PATTERN_OUTPUT] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_ESHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER_GAIN;
        FeaturePara[0] = *parg1; // exposure time (us)
        if(sensorDevId == SENSOR_MAIN) {
            FeaturePara[0] = ((1000 * FeaturePara[0] ) / m_LineTimeInus[0]);
        }
        else if ((sensorDevId == SENSOR_SUB) || (sensorDevId == SENSOR_MAIN) ) {
            FeaturePara[0] = ((1000 * FeaturePara[0] ) / m_LineTimeInus[1]);
        }
        else {
            LOG_ERR("Error sensorDevId !\n");
        }
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] m_LineTime = %llu %d\n", FeaturePara[0] , m_LineTimeInus[0]);
            FeaturePara[0]  = 1;
        }
        FeaturePara[2] = (FeaturePara[0] ) | (((*(parg1+1))>>4) << 16); // Sensor gain from 10b to 6b base
        FeaturePara[0] = 0; // RAW Gain R, Gr
        FeaturePara[1] = 0;  // RAW Gain Gb, B
        FeaturePara[3] = 0;  // Delay frame cnt
        LOG_MSG("CMD_SENSOR_SET_ESHUTTER_GAIN: Exp=%d, SensorGain=%d\n", (MUINT32)FeaturePara[2]&0x0000FFFF, (MUINT32)FeaturePara[2]>>16);
        FeatureParaLen = sizeof(MUINT64) * 4;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
         break;

    case CMD_SENSOR_SET_OB_LOCK:
        FeatureId = SENSOR_FEATURE_SET_OB_LOCK;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SNESOR_SET_SENSOR_OTP_AWB_CMD:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_OTP_AWB_CMD;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SNESOR_SET_SENSOR_OTP_LSC_CMD:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_OTP_LSC_CMD;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT:
        {
            *parg1 = getSensorDelayFrameCnt(sensorDevId,(MUINT32)*parg2);
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT] *parg1 = %d \n", (MUINT32)*parg1);
#endif
            return err;
        }
        break;

    case CMD_SENSOR_GET_INPUT_BIT_ORDER:
        // Bit 0~7 as data input
        switch  (sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor1);
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor2);

                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_INDATA_FORMAT> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_INPUT_BIT_ORDER] *parg1 = %d \n", (MUINT32)*parg1);
#endif
        return  err;
        break;

    case CMD_SENSOR_GET_PAD_PCLK_INV:
        switch(sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor1);
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor2);
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_PAD_PCLK_INV> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PAD_PCLK_INV] *parg1 = %d \n", (MUINT32)*parg1);
#endif
        return  err;
        break;

    case CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE:
        using namespace NSCamCustomSensor;
        NSCamCustomSensor::SensorOrientation_T orientation;
        orientation = NSCamCustomSensor::getSensorOrientation();
        switch(sensorDevId)
        {
            case SENSOR_MAIN:
                *parg1 = orientation.u4Degree_0;
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = orientation.u4Degree_1;
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1 = orientation.u4Degree_2;
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
        return err;
        break;

    case CMD_SENSOR_GET_SENSOR_FACING_DIRECTION:
        switch(sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor1);
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor2);
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_FACING_DIRECTION> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
        return  err;

        break;

    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ:
        FeatureId = SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PIXEL_CLOCK_FREQ] parg1 = %p \n", parg1);
#endif
        break;

    case CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        FeatureId = SENSOR_FEATURE_GET_PERIOD;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_SENSOR_FEATURE_INFO:
        switch(sensorDevId)
        {
            using namespace NSFeature;
            case SENSOR_MAIN:
                *parg1Addr = (MUINTPTR)m_pMainSensorInfo;
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1Addr = (MUINTPTR)m_pSubSensorInfo;
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1Addr = (MUINTPTR)m_pMain2SensorInfo;
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_FEATURE_INFO> - bad sensor id(%x)", (int)sensorDevId);
                parg1Addr = NULL;
                err = -1;
                break;
        }
        return  err;

        break;
     case CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeatureParaLen = sizeof(MUINT64) * 2;

        switch(*parg1)
        {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                FeaturePara[0] = MSDK_SCENARIO_ID_VIDEO_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                FeaturePara[0] = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                FeaturePara[0] = MSDK_SCENARIO_ID_SLIM_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM1;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM2;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM3;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM4;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM5;
                break;
            default:
                LOG_ERR("[setScenario] error scenario id\n");
                return SENSOR_UNKNOWN_ERROR;
        }
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
        LOG_MSG("FeaturePara %d\n",(FeaturePara[1]));
        LOG_MSG("framerate = %d",(MUINT32)(*parg2));
#endif
        break;

    case CMD_SENSOR_GET_FAKE_ORIENTATION:
         switch(sensorDevId)
         {
             using namespace NSCamCustomSensor;
             case SENSOR_MAIN:
                 *parg1 = isRetFakeMainOrientation();
                 err = 0;
                 break;
             case SENSOR_SUB:
                 *parg1 = isRetFakeSubOrientation();
                 err = 0;
                 break;
             case SENSOR_MAIN_2:
                 *parg1 = isRetFakeMain2Orientation();
                 err = 0;
                 break;


             default:
                 LOG_ERR("[sendCommand]<CMD_SENSOR_GET_FAKE_ORIENTATION> - bad sensor id(%x)", (int)sensorDevId);
                 *parg1 = 0;
                 err = -1;
                 break;
         }
         return  err;
     break;
    case CMD_SENSOR_GET_SENSOR_VIEWANGLE:
        using namespace NSCamCustomSensor;
        NSCamCustomSensor::SensorViewAngle_T viewangle;
        viewangle = NSCamCustomSensor::getSensorViewAngle();
        switch(sensorDevId)
        {
            case SENSOR_MAIN:
                *parg1 = viewangle.MainSensorHorFOV;
                *parg2 = viewangle.MainSensorVerFOV;
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = viewangle.SubSensorHorFOV;
                *parg2 = viewangle.SubSensorVerFOV;
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1 = viewangle.Main2SensorHorFOV;
                *parg2 = viewangle.Main2SensorVerFOV;
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
        return err;
       break;

     case CMD_SENSOR_GET_MCLK_CONNECTION:
        switch(sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getSensorMclkConnection(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getSensorMclkConnection(eDevId_ImgSensor1);
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1 = getSensorMclkConnection(eDevId_ImgSensor2);
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_MCLK_CONNECTION> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MCLK_CONNECTION] *parg1 = %d \n", (MUINT32)*parg1);
#endif
        return  err;
        break;

     case CMD_SENSOR_GET_MIPI_SENSOR_PORT:
        switch(sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getMipiSensorPort(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getMipiSensorPort(eDevId_ImgSensor1);
                err = 0;
                break;
            case SENSOR_MAIN_2:
                *parg1 = getMipiSensorPort(eDevId_ImgSensor2);
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_MIPI_SENSOR_PORT> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MIPI_SENSOR_PORT] *parg1 = %d \n", (MUINT32)*parg1);
#endif
        return  err;
        break;

     case CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE:
        FeatureId = SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

     case CMD_SENSOR_GET_TEMPERATURE_VALUE:
       FeatureId = SENSOR_FEATURE_GET_TEMPERATURE_VALUE;
       FeatureParaLen = sizeof(MUINT64);
       pFeaturePara = (MUINT8*)FeaturePara;
       break;

     case CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO:

        switch(*parg1)
        {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                FeaturePara[0] = MSDK_SCENARIO_ID_VIDEO_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                FeaturePara[0] = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                FeaturePara[0] = MSDK_SCENARIO_ID_SLIM_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM1;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM2;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM3;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM4;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM5;
                break;
            default:
                LOG_ERR("[setScenario] error scenario id\n");
                return SENSOR_UNKNOWN_ERROR;
        }

        FeatureId = SENSOR_FEATURE_GET_CROP_INFO;
        //u4FeaturePara[0] = *parg1;
        FeaturePara[1] = (MUINTPTR)parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_VC_INFO:
        switch(*parg2)
        {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                FeaturePara[0] = MSDK_SCENARIO_ID_VIDEO_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                FeaturePara[0] = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                FeaturePara[0] = MSDK_SCENARIO_ID_SLIM_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM1;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM2;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM3;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM4;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                FeaturePara[0] = MSDK_SCENARIO_ID_CUSTOM5;
                break;
            default:
                LOG_ERR("[setScenario] error scenario id\n");
                return SENSOR_UNKNOWN_ERROR;
        }
        FeatureId = SENSOR_FEATURE_GET_VC_INFO;
        FeaturePara[1] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_VC_INFO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
      case CMD_SENSOR_GET_SENSOR_PDAF_INFO:

        switch(*parg1)
        {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                FeaturePara[0] = MSDK_SCENARIO_ID_VIDEO_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                FeaturePara[0] = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                FeaturePara[0] = MSDK_SCENARIO_ID_SLIM_VIDEO;
                break;

            default:
                LOG_ERR("[setScenario] error scenario id\n");
                return SENSOR_UNKNOWN_ERROR;
        }

        FeatureId = SENSOR_FEATURE_GET_PDAF_INFO;
        FeaturePara[1] = (MUINTPTR)parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
        LOG_ERR("[SENSOR_FEATURE_GET_PDAF_INFO]%llu %llu\n",FeaturePara[0],FeaturePara[1]);

        break;

     case CMD_SENSOR_GET_PDAF_DATA:
        FeatureId = SENSOR_FEATURE_GET_PDAF_DATA;
        FeaturePara[0] = *parg1;//offset
        FeaturePara[1] = (MUINTPTR)(*parg2);//the address of pointer pointed
        FeaturePara[2] = *parg3;//size of buff
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        LOG_MSG("[CMD_SENSOR_GET_PDAF_DATA] 22 0x%llu 0x%llu 0x%llu", FeaturePara[0], FeaturePara[1], FeaturePara[2]);
        break;

     case CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64) * 2;

        switch(*parg1)
        {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                FeaturePara[0] = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                FeaturePara[0] = MSDK_SCENARIO_ID_VIDEO_PREVIEW;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                FeaturePara[0] = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                FeaturePara[0] = MSDK_SCENARIO_ID_SLIM_VIDEO;
                break;
            default:
                LOG_ERR("[setScenario] error scenario id\n");
                return SENSOR_UNKNOWN_ERROR;
        }
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

     case CMD_SENSOR_SET_SENSOR_AWB_GAIN:
            FeatureId = SENSOR_FEATURE_SET_AWB_GAIN;
            pFeaturePara = (UINT8*)parg1;
            FeatureParaLen = sizeof(SET_SENSOR_AWB_GAIN);
            break;

     case CMD_SENSOR_SET_YUV_FEATURE_CMD:
         FeatureId = SENSOR_FEATURE_SET_YUV_CMD;
         FeaturePara[0] = *parg1;
         FeaturePara[1] = *parg2;
         FeatureParaLen = sizeof(MUINT64) * 2;
         pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
         break;

    case CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE:
        FeatureId = SENSOR_FEATURE_SINGLE_FOCUS_MODE;
        //LOG_MSG("CMD_SENSOR_SINGLE_FOCUS_MODE\n");
        break;


    case CMD_SENSOR_SET_YUV_CANCEL_AF:
        FeatureId = SENSOR_FEATURE_CANCEL_AF;
        //LOG_MSG("CMD_SENSOR_CANCEL_AF\n");
        break;

    case CMD_SENSOR_SET_YUV_CONSTANT_AF:
        FeatureId = SENSOR_FEATURE_CONSTANT_AF;
        break;

    case CMD_SENSOR_SET_YUV_INFINITY_AF:
        FeatureId = SENSOR_FEATURE_INFINITY_AF;
        break;

    case CMD_SENSOR_SET_YUV_AF_WINDOW:
        FeatureId = SENSOR_FEATURE_SET_AF_WINDOW;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("zone_addr=0x%llu\n", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_AE_WINDOW:
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("AEzone_addr=0x%llu\n", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_GAIN_AND_ESHUTTER;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_YUV_3A_CMD:
        FeatureId = SENSOR_FEATURE_SET_YUV_3A_CMD;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("YUV_3A_CMD=0x%x\n", (MUINT32)*parg1);
        break;

    case CMD_SENSOR_GET_YUV_AF_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AF_STATUS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_STATUS,parg1=0x%x,FeatureParaLen=0x%x,pFeaturePara=0x%x\n",
        //parg1, FeatureParaLen, pFeaturePara);
        break;

     case CMD_SENSOR_GET_YUV_AE_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AE_STATUS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_YUV_AWB_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AWB_STATUS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN:
        FeatureId = SENSOR_FEATURE_GET_EV_AWB_REF;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("p_ref=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        FeatureId = SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("p_cur=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_EXIF_INFO:
        FeatureId = SENSOR_FEATURE_GET_EXIF_INFO;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("EXIF_addr=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_DELAY_INFO:
        FeatureId = SENSOR_FEATURE_GET_DELAY_INFO;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO:
        FeatureId = SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("FLASHLIGHT_INFO=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("TRIGGER_FLASHLIGHT=0x%x\n", (MUINT32)parg1);
        break;
    case CMD_SENSOR_SET_YUV_AUTOTEST:
        FeatureId = SENSOR_FEATURE_AUTOTEST_CMD;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_SET_N3D_I2C_STREAM_REGDATA:
        FeatureId = SENSOR_FEATURE_SET_N3D_I2C_STREAM_REGDATA;
        //FeatureParaLen = sizeof(MUINT32)*4;   //writeI2CID, writeAddr, writePara, writeByte ==> Check this, why need * 4?
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_N3D_STOP_STREAMING:
        FeatureId = SENSOR_FEATURE_SET_N3D_STOP_STREAMING;
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_N3D_START_STREAMING:
        FeatureId = SENSOR_FEATURE_SET_N3D_START_STREAMING;
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_FRAMERATE:
        FeatureId = SENSOR_FEATURE_SET_FRAMERATE;
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_IHDR_SHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN;
        if(SENSOR_MAIN == sensorDevId) {
            FeaturePara[0] = (1000 * (*parg1)) / m_LineTimeInus[0];
            FeaturePara[1] = (1000 * (*parg2)) / m_LineTimeInus[0];
            if(FeaturePara[0] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[0], m_LineTimeInus[0]);
                FeaturePara[0] = 1;
            }
            if(FeaturePara[1] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[1], m_LineTimeInus[0]);
                FeaturePara[1] = 1;
            }
        }
        else if((SENSOR_MAIN_2 == sensorDevId)||(SENSOR_SUB == sensorDevId)) {
            FeaturePara[0] = (1000 * (*parg1)) / m_LineTimeInus[1];
            FeaturePara[1] = (1000 * (*parg2)) / m_LineTimeInus[1];
            if(FeaturePara[0] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[0], m_LineTimeInus[1]);
                FeaturePara[0] = 1;
            }
            if(FeaturePara[1] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[1], m_LineTimeInus[1]);
                FeaturePara[1] = 1;
            }

        }
        else{
              LOG_ERR("sensorDevId is incorrect!!\n");
        }

        FeaturePara[2]= *parg3;
        FeaturePara[2] >>= 4; //from 10b to 6b base

        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_HDR_SHUTTER:
        FeatureId = SENSOR_FEATURE_SET_HDR_SHUTTER;
        if(SENSOR_MAIN == sensorDevId) {
            FeaturePara[0] = (1000 * (*parg1)) / m_LineTimeInus[0];
            FeaturePara[1] = (1000 * (*parg2)) / m_LineTimeInus[0];
            if(FeaturePara[0] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[0], m_LineTimeInus[0]);
                FeaturePara[0] = 1;
            }
            if(FeaturePara[1] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[1], m_LineTimeInus[0]);
                FeaturePara[1] = 1;
            }
        }
        else if((SENSOR_MAIN_2 == sensorDevId)||(SENSOR_SUB == sensorDevId)) {
            FeaturePara[0] = (1000 * (*parg1)) / m_LineTimeInus[1];
            FeaturePara[1] = (1000 * (*parg2)) / m_LineTimeInus[1];
            if(FeaturePara[0] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[0], m_LineTimeInus[1]);
                FeaturePara[0] = 1;
            }
            if(FeaturePara[1] == 0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", FeaturePara[1], m_LineTimeInus[1]);
                FeaturePara[1] = 1;
            }

        }
        else{
              LOG_ERR("sensorDevId is incorrect!!\n");
        }

        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_MIN_MAX_FPS:
        FeatureId = SENSOR_FEATURE_SET_MIN_MAX_FPS;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER:
        if(SENSOR_MAIN == sensorDevId) {
            *parg1 = m_LineTimeInus[0];
            *parg2 = mImageTargetHeight;
        }
        else if((SENSOR_MAIN_2 == sensorDevId)||(SENSOR_SUB == sensorDevId)) {
            *parg1 = m_LineTimeInus[1];
            *parg2 = mImageTargetHeight;
        }
        else{
            LOG_ERR("sensorDevId is incorrect!!\n");
        }
       break;

    default:
        LOG_ERR("[sendCommand]Command ID = %d is undefined\n",cmd);
        return SENSOR_UNKNOWN_ERROR;
    }

    if (m_fdSensor == -1) {
        LOG_ERR("[sendCommand]m_fdSensor fail, sendCommand must be called after powerOn()!\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    err= featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, FeatureId,  (MUINT8*)pFeaturePara,(MUINT32*)&FeatureParaLen);
    if (err < 0) {
        LOG_ERR("[sendCommand] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }
     switch (cmd) {
        case CMD_SENSOR_GET_YUV_AE_STATUS:
        case CMD_SENSOR_GET_YUV_AWB_STATUS:
        case CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        case CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        case CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        case CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
        case CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE:
        case CMD_SENSOR_GET_TEMPERATURE_VALUE:
        case CMD_SENSOR_GET_YUV_AF_STATUS:

        case CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ:
            *((MUINT32*)parg1)=FeaturePara[0];
            break;


        case CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO:
        case CMD_SENSOR_SET_YUV_AUTOTEST:
            *((MUINT32*)parg1)=FeaturePara[0];
            *((MUINT32*)parg2)=FeaturePara[1];
            break;
        case CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY:
        case CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
            *((MUINT32*)parg2)=FeaturePara[1];
#ifdef SENDCMD_LOG
            LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
            //LOG_MSG("FeaturePara %llx\n",(MUINTPTR)(&FeaturePara[1]));
            LOG_MSG("FeaturePara %d\n",(FeaturePara[1]));
            LOG_MSG("value = %d",(MUINT32)(*parg2));
#endif
        break;
        default:
            break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
#if defined(YUV_TUNING_SUPPORT)
/////////////////////////////////////////////////////////////////////////
//
//  getHexToken () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
char*
ImgSensorDrv::getHexToken(char *inStr, MUINT32 *outVal)
{
    MUINT32 thisVal, tVal;
    char x;
    char *thisStr = inStr;

    thisVal = 0;

    // If first character is ';', we have a comment, so
    // get out of here.

    if (*thisStr == ';')
    {
        return (thisStr);
    }
        // Process hex characters.

    while (*thisStr)
    {
        // Do uppercase conversion if necessary.

        x = *thisStr;
        if ((x >= 'a') && (x <= 'f'))
        {
            x &= ~0x20;
        }
        // Check for valid digits.

        if ( !(((x >= '0') && (x <= '9')) || ((x >= 'A') && (x <= 'F'))))
        {
            break;
        }
        // Hex ASCII to binary conversion.

        tVal = (MUINT32)(x - '0');
        if (tVal > 9)
        {
            tVal -= 7;
        }

        thisVal = (thisVal * 16) + tVal;

        thisStr++;
    }

        // Return updated pointer and decoded value.

    *outVal = thisVal;
    return (thisStr);
}

/*******************************************************************************
*
********************************************************************************/
void
ImgSensorDrv::customerInit(void)
{
    FILE *fp = NULL;

    fp = fopen("/data/sensor_init.txt", "r");
    if (fp == NULL)
    {
        printf("No Customer Sensor Init File Exist \n");
        return;
    }

    ACDK_SENSOR_REG_INFO_STRUCT sensorReg;
    memset (&sensorReg, 0, sizeof(sensorReg));
    UINT32 featureLen = sizeof(ACDK_SENSOR_REG_INFO_STRUCT);

    char addrStr[20];
    char dataStr[20];
    LOG_MSG("[Write Customer Sensor Init Reg]:\n");
    fgets(dataStr, 20, fp);
    if (strncmp(dataStr, "mt65xx_yuv_tuning", 17) != 0)
    {
        LOG_ERR("Error Password \n");
        fclose(fp);
        return;
    }

    while (!feof(fp))
    {
        fscanf(fp, "%s %s\n", addrStr, dataStr);
        if (strlen(addrStr) != 0 && strlen(dataStr) != 0)
        {
            u32 addrVal = 0;
            u32 dataVal = 0;

            getHexToken(addrStr, &addrVal);
            getHexToken(dataStr, &dataVal);

            LOG_MSG("Addr:0x%x, data:0x%x\n", addrVal, dataVal);
            sensorReg.RegAddr = addrVal;
            sensorReg.RegData = dataVal;

            featureControl(DUAL_CAMERA_MAIN_SENSOR,SENSOR_FEATURE_SET_REGISTER, (MUINT8 *)&sensorReg, (MUINT32 *)&featureLen);
        }
    }

    fclose(fp);
}
#endif


/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::open(MINT32 sensorIdx)
{
    MINT32 err = SENSOR_NO_ERROR;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_CURRENT_SENSOR, &sensorIdx);

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_OPEN);
    if (err < 0) {
        LOG_MSG("[open] Err-ctrlCode (%s) \n", strerror(errno));

        return -errno;
    }

#if  defined(YUV_TUNING_SUPPORT )
    customerInit();
#endif
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::close(MINT32 sensorIdx)
{
    MINT32 err = SENSOR_NO_ERROR;
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_CURRENT_SENSOR, &sensorIdx);

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_CLOSE);
    if (err < 0) {
        LOG_ERR("[close] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }


    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::getInfo(
    MUINT32 ScenarioId[2],
    ACDK_SENSOR_INFO_STRUCT *pSensorInfo[2],
    ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData[2]
)
{
    ACDK_SENSOR_GETINFO_STRUCT getInfo;
    MINT32 err = SENSOR_NO_ERROR;
    MINT32 i=0;
    LOG_MSG("[getInfo],m_fdSensor = 0x%x \n",m_fdSensor);


    if (NULL == pSensorInfo|| NULL == pSensorConfigData) {
        LOG_ERR("[getInfo] NULL pointer\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    for(i=0; i<2; i++) {
        getInfo.ScenarioId[i] = (MSDK_SCENARIO_ID_ENUM)ScenarioId[i];
        getInfo.pInfo[i] = pSensorInfo[i];
        getInfo.pConfig[i] = pSensorConfigData[i];

    }

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETINFO , &getInfo);

    if (err < 0) {
        LOG_ERR("[getInfo]Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }
    return err;
}


/*******************************************************************************
*
********************************************************************************/

MINT32
ImgSensorDrv::getInfo2(
    SENSOR_DEV_ENUM sensorDevId,
    SENSORDRV_INFO_STRUCT *pSensorInfo
)
{
    IMAGESENSOR_GETINFO_STRUCT getInfo2;
    ACDK_SENSOR_INFO2_STRUCT    Info2;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT SensorResolution;

    MINT32 err                  = SENSOR_NO_ERROR;
    MINT32 i                    = 0;
    getInfo2.SensorId           = (MUINT32)sensorDevId;
    getInfo2.pInfo              = &Info2;
    getInfo2.pSensorResolution  = &SensorResolution;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETINFO2 , &getInfo2);

    if (err < 0) {
        LOG_ERR("[getInfo2]Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }
    // Basic information
    pSensorInfo->SensorPreviewResolutionX                = Info2.SensorPreviewResolutionX;
    pSensorInfo->SensorPreviewResolutionY                = Info2.SensorPreviewResolutionY;
    pSensorInfo->SensorFullResolutionX                   = Info2.SensorFullResolutionX ;
    pSensorInfo->SensorFullResolutionY                   = Info2.SensorFullResolutionY;
    pSensorInfo->SensorClockFreq                         = Info2.SensorClockFreq;
    pSensorInfo->SensorCameraPreviewFrameRate            = Info2.SensorCameraPreviewFrameRate;
    pSensorInfo->SensorVideoFrameRate                    = Info2.SensorVideoFrameRate;
    pSensorInfo->SensorStillCaptureFrameRate             = Info2.SensorStillCaptureFrameRate;
    pSensorInfo->SensorWebCamCaptureFrameRate            = Info2.SensorWebCamCaptureFrameRate;
    pSensorInfo->SensorClockPolarity                     = Info2.SensorClockPolarity;
    pSensorInfo->SensorClockFallingPolarity              = Info2.SensorClockFallingPolarity;
    pSensorInfo->SensorClockRisingCount                  = Info2.SensorClockRisingCount;
    pSensorInfo->SensorClockFallingCount                 = Info2.SensorClockFallingCount;
    pSensorInfo->SensorClockDividCount                   = Info2.SensorClockDividCount;
    pSensorInfo->SensorPixelClockCount                   = Info2.SensorPixelClockCount;
    pSensorInfo->SensorDataLatchCount                    = Info2.SensorDataLatchCount;
    pSensorInfo->SensorHsyncPolarity                     = Info2.SensorHsyncPolarity;
    pSensorInfo->SensorVsyncPolarity                     = Info2.SensorVsyncPolarity;
    pSensorInfo->SensorInterruptDelayLines               = Info2.SensorInterruptDelayLines;
    pSensorInfo->SensorResetActiveHigh                   = Info2.SensorResetActiveHigh;
    pSensorInfo->SensorResetDelayCount                   = Info2.SensorResetDelayCount;
    pSensorInfo->SensroInterfaceType                     = Info2.SensroInterfaceType;
    pSensorInfo->SensorOutputDataFormat                  = Info2.SensorOutputDataFormat;
    pSensorInfo->SensorMIPILaneNumber                    = Info2.SensorMIPILaneNumber;
    pSensorInfo->CaptureDelayFrame                       = Info2.CaptureDelayFrame;
    pSensorInfo->PreviewDelayFrame                       = Info2.PreviewDelayFrame;
    pSensorInfo->VideoDelayFrame                         = Info2.VideoDelayFrame;
    pSensorInfo->HighSpeedVideoDelayFrame                = Info2.HighSpeedVideoDelayFrame;
    pSensorInfo->SlimVideoDelayFrame                     = Info2.SlimVideoDelayFrame;
    pSensorInfo->Custom1DelayFrame                       = Info2.Custom1DelayFrame;
    pSensorInfo->Custom2DelayFrame                       = Info2.Custom2DelayFrame;
    pSensorInfo->Custom3DelayFrame                       = Info2.Custom3DelayFrame;
    pSensorInfo->Custom4DelayFrame                       = Info2.Custom4DelayFrame;
    pSensorInfo->Custom5DelayFrame                       = Info2.Custom5DelayFrame;
    pSensorInfo->YUVAwbDelayFrame                        = Info2.YUVAwbDelayFrame;
    pSensorInfo->YUVEffectDelayFrame                     = Info2.YUVEffectDelayFrame;
    pSensorInfo->SensorGrabStartX_PRV                    = Info2.SensorGrabStartX_PRV;
    pSensorInfo->SensorGrabStartY_PRV                    = Info2.SensorGrabStartY_PRV;
    pSensorInfo->SensorGrabStartX_CAP                    = Info2.SensorGrabStartX_CAP;
    pSensorInfo->SensorGrabStartY_CAP                    = Info2.SensorGrabStartY_CAP;
    pSensorInfo->SensorGrabStartX_VD                     = Info2.SensorGrabStartX_VD;
    pSensorInfo->SensorGrabStartY_VD                     = Info2.SensorGrabStartY_VD;
    pSensorInfo->SensorGrabStartX_VD1                    = Info2.SensorGrabStartX_VD1;
    pSensorInfo->SensorGrabStartY_VD1                    = Info2.SensorGrabStartY_VD1;
    pSensorInfo->SensorGrabStartX_VD2                    = Info2.SensorGrabStartX_VD2;
    pSensorInfo->SensorGrabStartY_VD2                    = Info2.SensorGrabStartY_VD2;
    pSensorInfo->SensorGrabStartX_CST1                   = Info2.SensorGrabStartX_CST1;
    pSensorInfo->SensorGrabStartY_CST1                   = Info2.SensorGrabStartY_CST1;
    pSensorInfo->SensorGrabStartX_CST2                   = Info2.SensorGrabStartX_CST2;
    pSensorInfo->SensorGrabStartY_CST2                   = Info2.SensorGrabStartY_CST2;
    pSensorInfo->SensorGrabStartX_CST3                   = Info2.SensorGrabStartX_CST3;
    pSensorInfo->SensorGrabStartY_CST3                   = Info2.SensorGrabStartY_CST3;
    pSensorInfo->SensorGrabStartX_CST4                   = Info2.SensorGrabStartX_CST4;
    pSensorInfo->SensorGrabStartY_CST4                   = Info2.SensorGrabStartY_CST4;
    pSensorInfo->SensorGrabStartX_CST5                   = Info2.SensorGrabStartX_CST5;
    pSensorInfo->SensorGrabStartY_CST5                   = Info2.SensorGrabStartY_CST5;
    pSensorInfo->SensorDrivingCurrent                    = Info2.SensorDrivingCurrent;
    pSensorInfo->SensorMasterClockSwitch                 = Info2.SensorMasterClockSwitch;
    pSensorInfo->AEShutDelayFrame                        = Info2.AEShutDelayFrame;
    pSensorInfo->AESensorGainDelayFrame                  = Info2.AESensorGainDelayFrame;
    pSensorInfo->AEISPGainDelayFrame                     = Info2.AEISPGainDelayFrame;
    pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount  = Info2.MIPIDataLowPwr2HighSpeedTermDelayCount;
    pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount= Info2.MIPIDataLowPwr2HighSpeedSettleDelayCount;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM0          = Info2.MIPIDataLowPwr2HSSettleDelayM0;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM1          = Info2.MIPIDataLowPwr2HSSettleDelayM1;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM2          = Info2.MIPIDataLowPwr2HSSettleDelayM2;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM3          = Info2.MIPIDataLowPwr2HSSettleDelayM3;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM4          = Info2.MIPIDataLowPwr2HSSettleDelayM4;
    pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount   = Info2.MIPICLKLowPwr2HighSpeedTermDelayCount;
    pSensorInfo->SensorWidthSampling                     = Info2.SensorWidthSampling;
    pSensorInfo->SensorHightSampling                     = Info2.SensorHightSampling;
    pSensorInfo->SensorPacketECCOrder                    = Info2.SensorPacketECCOrder;
    pSensorInfo->iHDR_First_IS_LE                        = Info2.IHDR_LE_FirstLine;
    pSensorInfo->SensorModeNum                           = Info2.SensorModeNum;
    if(Info2.IHDR_Support == 1)
    {
        pSensorInfo->iHDRSupport                         = MTRUE;
    }
    pSensorInfo->PDAF_Support                            = Info2.PDAF_Support;
    pSensorInfo->virtualChannelSupport = MFALSE;
    pSensorInfo->MIPIsensorType                          = Info2.MIPIsensorType;
    pSensorInfo->SettleDelayMode                         = Info2.SettleDelayMode;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_PRE                 = Info2.IMGSENSOR_DPCM_TYPE_PRE;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_CAP                 = Info2.IMGSENSOR_DPCM_TYPE_CAP;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_VD                  = Info2.IMGSENSOR_DPCM_TYPE_VD;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_VD1                 = Info2.IMGSENSOR_DPCM_TYPE_VD1;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_VD2                 = Info2.IMGSENSOR_DPCM_TYPE_VD2;
    /*SCAM*/
    pSensorInfo->SCAM_DataNumber                         = Info2.SCAM_DataNumber;
    pSensorInfo->SCAM_DDR_En                              = Info2.SCAM_DDR_En;
    pSensorInfo->SCAM_CLK_INV                              = Info2.SCAM_CLK_INV;
    //Support Resolution
    pSensorInfo->SensorPreviewWidth                      = SensorResolution.SensorPreviewWidth;
    pSensorInfo->SensorPreviewHeight                     = SensorResolution.SensorPreviewHeight;
    pSensorInfo->SensorCapWidth                         = SensorResolution.SensorFullWidth;
    pSensorInfo->SensorCapHeight                        = SensorResolution.SensorFullHeight;
    pSensorInfo->SensorVideoWidth                        = SensorResolution.SensorVideoWidth;
    pSensorInfo->SensorVideoHeight                       = SensorResolution.SensorVideoHeight;
    pSensorInfo->SensorVideo1Width                        = SensorResolution.SensorHighSpeedVideoWidth;
    pSensorInfo->SensorVideo1Height                       = SensorResolution.SensorHighSpeedVideoHeight;
    pSensorInfo->SensorVideo2Width                        = SensorResolution.SensorSlimVideoWidth;
    pSensorInfo->SensorVideo2Height                       = SensorResolution.SensorSlimVideoHeight;
    pSensorInfo->SensorCustom1Width                        = SensorResolution.SensorCustom1Width;
    pSensorInfo->SensorCustom1Height                       = SensorResolution.SensorCustom1Height;
    pSensorInfo->SensorCustom2Width                        = SensorResolution.SensorCustom2Width;
    pSensorInfo->SensorCustom2Height                       = SensorResolution.SensorCustom2Height;
    pSensorInfo->SensorCustom3Width                        = SensorResolution.SensorCustom3Width;
    pSensorInfo->SensorCustom3Height                       = SensorResolution.SensorCustom3Height;
    pSensorInfo->SensorCustom4Width                        = SensorResolution.SensorCustom4Width;
    pSensorInfo->SensorCustom4Height                       = SensorResolution.SensorCustom4Height;
    pSensorInfo->SensorCustom5Width                        = SensorResolution.SensorCustom5Width;
    pSensorInfo->SensorCustom5Height                       = SensorResolution.SensorCustom5Height;
    pSensorInfo->SensorEffectivePreviewWidth             = SensorResolution.SensorEffectivePreviewWidth;
    pSensorInfo->SensorEffectivePreviewHeight            = SensorResolution.SensorEffectivePreviewHeight;
    pSensorInfo->SensorEffectiveCapWidth                = SensorResolution.SensorEffectiveFullWidth;
    pSensorInfo->SensorEffectiveCapHeight               = SensorResolution.SensorEffectiveFullHeight;
    pSensorInfo->SensorEffectiveVideoWidth               = SensorResolution.SensorEffectiveVideoWidth;
    pSensorInfo->SensorEffectiveVideoHeight              = SensorResolution.SensorEffectiveVideoHeight;
    pSensorInfo->SensorPreviewWidthOffset                = SensorResolution.SensorPreviewWidthOffset;//from effective width to output width
    pSensorInfo->SensorPreviewHeightOffset               = SensorResolution.SensorPreviewHeightOffset;//from effective height to output height
    pSensorInfo->SensorCapWidthOffset                   = SensorResolution.SensorFullWidthOffset;//from effective width to output width
    pSensorInfo->SensorCapHeightOffset                  = SensorResolution.SensorFullHeightOffset;//from effective height to output height
    pSensorInfo->SensorVideoWidthOffset                  = SensorResolution.SensorVideoWidthOffset;//from effective width to output width
    pSensorInfo->SensorVideoHeightOffset                 = SensorResolution.SensorVideoHeightOffset;//from effective height to output height
    LOG_MSG("prv w=0x%x,h=0x%x\n",pSensorInfo->SensorPreviewWidth,pSensorInfo->SensorPreviewHeight);
    LOG_MSG("cap w=0x%x,h=0x%x\n",pSensorInfo->SensorCapWidth,pSensorInfo->SensorCapHeight);
    LOG_MSG("vd  w=0x%x,h=0x%x\n",pSensorInfo->SensorVideoWidth,pSensorInfo->SensorVideoHeight);
    LOG_MSG("pre GrapX=0x%x,GrapY=0x%x\n",pSensorInfo->SensorGrabStartX_PRV,pSensorInfo->SensorGrabStartY_PRV);
    //LOG_MSG("cap GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_CAP,pSensorInfo->SensorGrabStartY_CAP);
    //LOG_MSG("vd  GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_VD,pSensorInfo->SensorGrabStartY_VD);
    //LOG_MSG("vd1 GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_VD1,pSensorInfo->SensorGrabStartY_VD1);
    //LOG_MSG("vd2 GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_VD2,pSensorInfo->SensorGrabStartY_VD2);

   return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::getResolution(
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution[2]
)
{
    LOG_MSG("[getResolution] ACDK_SENSOR_RESOLUTION_INFO_STRUCT");
    MINT32 err = SENSOR_NO_ERROR;
    if (NULL == pSensorResolution) {
        LOG_ERR("[getResolution] NULL pointer\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    ACDK_SENSOR_PRESOLUTION_STRUCT resolution = {{pSensorResolution[0], pSensorResolution[1]}};
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETRESOLUTION2, &resolution);

    if (err < 0) {
        LOG_ERR("[getResolution] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}//halSensorGetResolution

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::featureControl(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM InvokeCamera,
    ACDK_SENSOR_FEATURE_ENUM FeatureId,
    MUINT8 *pFeaturePara,
    MUINT32 *pFeatureParaLen
)
{
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MINT32 err = SENSOR_NO_ERROR;

    if(SENSOR_FEATURE_SINGLE_FOCUS_MODE == FeatureId || SENSOR_FEATURE_CANCEL_AF == FeatureId
        || SENSOR_FEATURE_CONSTANT_AF == FeatureId){
    //AF INIT || AF constant has no parameters
    }
    else{

        if (NULL == pFeaturePara || NULL == pFeatureParaLen) {
            return SENSOR_INVALID_PARA;
        }
    }
    featureCtrl.InvokeCamera = InvokeCamera;
    featureCtrl.FeatureId = FeatureId;
    featureCtrl.pFeaturePara = pFeaturePara;
    featureCtrl.pFeatureParaLen = pFeatureParaLen;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
    if (err < 0) {
        LOG_ERR("[featureControl] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}//halSensorFeatureControl

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::setFoundDrvsActive(
MUINT32 socketIdxes
)
{
    MINT32 err = SENSOR_NO_ERROR;
    MUINT32 sensorDrvInfo[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};

    IMGSNESOR_FILL_SET_DRIVER_INFO(socketIdxes);

    LOG_MSG("[%s][0x%x] \n",__FUNCTION__,socketIdxes);
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,sensorDrvInfo);
    if (err < 0) {
        LOG_ERR("ERROR:setFoundDrvsActive\n");
    }
    return err;
}
/*******************************************************************************
*
********************************************************************************/

IMAGE_SENSOR_TYPE
ImgSensorDrv::getCurrentSensorType(
    SENSOR_DEV_ENUM sensorDevId
)
{

    MUINT32 scenarioId[2] = {SENSOR_SCENARIO_ID_NORMAL_PREVIEW,SENSOR_SCENARIO_ID_NORMAL_PREVIEW};

    LOG_MSG("[getCurrentSensorType] \n");


    if (SENSOR_NO_ERROR != getInfo(scenarioId, m_psensorInfo, m_psensorConfigData)) {
       LOG_ERR("[searchSensor] Error:getInfo() \n");
       return IMAGE_SENSOR_TYPE_UNKNOWN;
    }


    if(SENSOR_MAIN == sensorDevId ) {

        if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B &&
             m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
            return IMAGE_SENSOR_TYPE_RAW;
        }
        else if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW8_B &&
             m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
            return IMAGE_SENSOR_TYPE_RAW8;
        }
        else if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY &&
                    m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
            return IMAGE_SENSOR_TYPE_YUV;
        }
        else if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                    m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
            return IMAGE_SENSOR_TYPE_YCBCR;
        }
        else {
            return IMAGE_SENSOR_TYPE_UNKNOWN;
        }
    }
    else if((SENSOR_MAIN_2 == sensorDevId)||(SENSOR_SUB == sensorDevId)) {

        if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B &&
             m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
            return IMAGE_SENSOR_TYPE_RAW;
        }
        if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW8_B &&
             m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
            return IMAGE_SENSOR_TYPE_RAW8;
        }
        else if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY &&
                    m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
            return IMAGE_SENSOR_TYPE_YUV;
        }
        else if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                    m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
            return IMAGE_SENSOR_TYPE_YCBCR;
        }
        else {
            return IMAGE_SENSOR_TYPE_UNKNOWN;
        }
    }
    else {
        LOG_ERR("[getCurrentSensorType] Error sensorDevId ! \n");
    }



    return IMAGE_SENSOR_TYPE_UNKNOWN;
}



/*******************************************************************************
*
********************************************************************************/
IMGSENSOR_SOCKET_POSITION_ENUM
ImgSensorDrv::getSocketPosition(
CAMERA_DUAL_CAMERA_SENSOR_ENUM socket) {
MUINT32 socketPos = socket;
MINT32 err = SENSOR_NO_ERROR;
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_SOCKET_POS , &socketPos);
    if (err < 0) {
        LOG_ERR("[getInfo]Err-ctrlCode (%s) \n", strerror(errno));
        return IMGSENSOR_SOCKET_POS_NONE;
    }

    LOG_MSG("[%s]:[%d][%d] \n",__FUNCTION__,socket,socketPos);

    return (IMGSENSOR_SOCKET_POSITION_ENUM)socketPos;
}
//
MINT32 ImgSensorDrv::releaseI2CTriggerLock()
{
    MINT32 err = SENSOR_NO_ERROR;
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_RELEASE_I2C_TRIGGER_LOCK);
    if (err < 0)
    {
        LOG_ERR("err 0x%x\n",err);
        LOG_ERR("[setTriggerDatatoI2CBuffer]Err-ctrlCode (%s) \n", strerror(errno));
        return SENSOR_UNKNOWN_ERROR;
    }
    return err;
}


