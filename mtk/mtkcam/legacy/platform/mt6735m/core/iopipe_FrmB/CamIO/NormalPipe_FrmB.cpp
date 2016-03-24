/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
//carson added for testing
//#include "_MyUtils.h"
/*++++++++++++++++from _MyUtils.h++++++++++++++++ */
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
using namespace android;
//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>

using namespace NSCam;

using namespace NSCam::Utils;

//
//#include <mtkcam/Log.h>

#include <mtkcam/hal/IHalSensor.h>

#include <kd_imgsensor_define.h>

/*------------------from _MyUtils.h------------------ */

//#include "VDThread.h"
/*++++++++++++++++from VDThread.h++++++++++++++++ */
#include <utils/Thread.h>
#include <utils/Condition.h>
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
/*------------------from VDThread.h------------------ */

#include <cutils/properties.h>

#include "NormalPipe_FrmB.h"
#include <mtkcam/hal/IHalSensor.h>
#include "PortMap_FrmB.h"
#include "isp_datatypes_FrmB.h"

//thread priority
#include <system/thread_defs.h>
#include <sys/resource.h>
#include <utils/ThreadDefs.h>


//include is for following complie option. use following statement is because .so r not the same
#include "isp_function_FrmB.h"
#define PASS1_CQ_CONTINUOUS_MODE__  1

#ifdef _PASS1_CQ_CONTINUOUS_MODE_
  #if (PASS1_CQ_CONTINUOUS_MODE__ == 0)
  #error "cq_mode_not_sync_1"
  #endif
#else
  #if (PASS1_CQ_CONTINUOUS_MODE__ == 1)
  #error "cq_mode_not_sync_2"
  #endif
#endif

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NormalPipe_FrmB"
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio_FrmB;
using namespace NSIspio_FrmB;

#define SENSOR_METADATA_IS_READY 0
#define SENSOR_TYPE_IS_RAW       1

DECLARE_DBG_LOG_VARIABLE(iopipe);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (iopipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (iopipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (iopipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (iopipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (iopipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (iopipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define CONFIG_PIPE_TIME            0// default:0
DECLARE_DBG_LOG_VARIABLE(iopipetime);
#define PIPETIME_VRB(fmt, arg...)        do { if (iopipetime_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPETIME_DBG(fmt, arg...)        do { if (iopipetime_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPETIME_INF(fmt, arg...)        do { if (iopipetime_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPETIME_WRN(fmt, arg...)        do { if (iopipetime_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPETIME_ERR(fmt, arg...)        do { if (iopipetime_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPETIME_AST(cond, fmt, arg...)  do { if (iopipetime_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define CONFIG_PIPE_FOR_SYSTRACE     0   // default:0
#if CONFIG_PIPE_FOR_SYSTRACE
#define ATRACE_TAG                  ATRACE_TAG_ALWAYS//must before include Trace.h
#include <utils/Trace.h>
#define PIPE_TRACE_CALL()           ATRACE_CALL()
#define PIPE_TRACE_NAME(name)       ATRACE_NAME(name)
#define PIPE_TRACE_BEGIN(name)      ATRACE_BEGIN(name)
#define PIPE_TRACE_END()            ATRACE_END()
#else
#define PIPE_TRACE_CALL()
#define PIPE_TRACE_NAME(name)
#define PIPE_TRACE_BEGIN(name)
#define PIPE_TRACE_END()
#endif // CONFIG_PIPE_FOR_SYSTRACE

/******************************************************************************
 *
 ******************************************************************************/
namespace
{
    Mutex                           gNPipeCapMutex;
    //
    IMetadata                       gPipeCap;

    //  OUT PORT
    IMetadata                       gPortCap_IMGO;
    IMetadata                       gPortCap_IMG2O;
};

//map portId to deque container index
#define _PortMap(PortIdx)    ({\
    int _idx=0;\
    if(PortIdx == PORT_IMG2O.index)\
        _idx = 0;\
    else if(PortIdx == PORT_IMGO.index)\
        _idx = 1;\
    else\
        PIPE_ERR("error:portidx:0x%x\n",PortIdx);\
    _idx;\
})

// mapping element NormalPipe_FrmB::mTgOut_size
#define    _TGMapping(x)({\
    MUINT32 _tg=0;\
    if(x==CAM_TG_1)\
        _tg = 0;\
    else\
        PIPE_ERR("un supported TG:0x%x\n",x);\
    _tg;\
})

//switch to camiopipe index
#define _NORMALPIPE_GET_TGIFMT(sensorIdx,fmt) {\
    switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType) {\
        case SENSOR_TYPE_RAW:\
            switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit) {\
                case RAW_SENSOR_8BIT:    fmt = NSCam::eImgFmt_BAYER8; break;\
                case RAW_SENSOR_10BIT:    fmt = NSCam::eImgFmt_BAYER10; break;\
                case RAW_SENSOR_12BIT:    fmt = NSCam::eImgFmt_BAYER12; break;\
                case RAW_SENSOR_14BIT:  fmt = NSCam::eImgFmt_BAYER14; break;\
                default: PIPE_ERR("Err sen raw fmt(%d) err\n", pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit); break;\
            }\
            break;\
        case SENSOR_TYPE_YUV:\
            switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder) {\
                case SENSOR_FORMAT_ORDER_UYVY: fmt = NSCam::eImgFmt_UYVY; break;\
                case SENSOR_FORMAT_ORDER_VYUY: fmt = NSCam::eImgFmt_VYUY; break;\
                case SENSOR_FORMAT_ORDER_YVYU: fmt = NSCam::eImgFmt_YVYU; break;\
                case SENSOR_FORMAT_ORDER_YUYV: fmt = NSCam::eImgFmt_YUY2; break;\
                default:    PIPE_ERR("Err sen yuv fmt err\n"); break;\
            }\
            break;\
        case SENSOR_TYPE_JPEG:\
            fmt = NSCam::eImgFmt_JPEG; break;\
            break;\
        default:\
            PIPE_ERR("Err sen type(%d) err\n", pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType);\
            break;\
    }\
}

//switch to camiopipe idx
#define _NORMALPIPE_GET_TGI_PIX_ID(sensorIdx,pix_id) do{\
    switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder){\
        case SENSOR_FORMAT_ORDER_RAW_B:    pix_id = ERawPxlID_B; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gb: pix_id = ERawPxlID_Gb; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gr: pix_id = ERawPxlID_Gr; break;\
        case SENSOR_FORMAT_ORDER_RAW_R: pix_id = ERawPxlID_R; break;\
        case SENSOR_FORMAT_ORDER_UYVY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_VYUY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YUYV: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YVYU: pix_id = (ERawPxlID)0; break;\
        default:    PIPE_ERR("Error Pix_id: sensorIdx=%d, sensorFormatOrder=%d", sensorIdx, pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder); break;\
    }\
    PIPE_DBG("SensorIdx=%d, sensorFormatOrder=%d", sensorIdx, pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder);\
}while(0);

#define _NORMALPIPE_GET_SENSORTYPE(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType)
//switch to camiopipe index
#define _NOMRALPIPE_CVT_TGIDX(sensorIdx)({\
    MUINT32 tgidx;\
    switch(pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo) {\
        case CAM_TG_1: tgidx = EPortIndex_TG1I; break;\
        default:    PIPE_ERR("Error tg idx err"); break;\
    }\
    tgidx;\
})

#define _NORMALPIPE_GET_SENSORCFG_CROP_W(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.w)
#define _NORMALPIPE_GET_SENSORCFG_CROP_H(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.h)
#define _NORMALPIPE_GET_SENSOR_DEV_ID(sensorIdx)    (pAllSensorInfo->mSenInfo[sensorIdx].mDevId)

#define _NORMALPIPE_GET_TG_IDX(sensorIdx)     (pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo)
#define _NORMALPIPE_GET_PIX_MODE(sensorIdx)   (pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.pixelMode)

#define _SensorTypeToPortFormat(sensorType) ({\
    NSImageio_FrmB::NSIspio_FrmB::EScenarioFmt _fmt = NSImageio_FrmB::NSIspio_FrmB::eScenarioFmt_RAW;\
    switch(sensorType){\
        case NSCam::SENSOR_TYPE_RAW:  _fmt = NSImageio_FrmB::NSIspio_FrmB::eScenarioFmt_RAW;\
            break;\
        case NSCam::SENSOR_TYPE_YUV:  _fmt = NSImageio_FrmB::NSIspio_FrmB::eScenarioFmt_YUV;\
            break;\
        case NSCam::SENSOR_TYPE_RGB:  _fmt = NSImageio_FrmB::NSIspio_FrmB::eScenarioFmt_RGB;\
            break;\
        case NSCam::SENSOR_TYPE_JPEG: _fmt = NSImageio_FrmB::NSIspio_FrmB::eScenarioFmt_JPG;\
            break;\
        default:\
            break;\
   }\
   _fmt;\
})

typedef enum{

   NPIPE_PASS_PASS1 = 1,
   NPIPE_PASS_PASS1_D = 2,
   NPIPE_PASS_CAMSV = 4,
   NPIPE_PASS_CAMSV_D = 8,
}NPIPE_PASS_E;

typedef enum{

  NPIPE_Sensor_0 = 0,
  NPIPE_Sensor_1,
  NPIPE_Sensor_0And1,
  NPIPE_Sensor_RSVD,
}NPIPE_IDX_E;

static NormalPipe_FrmB* pNormalPipe_FrmB[NPIPE_Sensor_RSVD] = {NULL, NULL, NULL};
static platSensorsInfo *pAllSensorInfo = NULL;
static Mutex  NPipeGLock;//NormalPipe_FrmB Global Lock

#if 1
#define NormalPipeGLock() NPipeGLock.lock()
#define NormalPipeGUnLock() NPipeGLock.unlock()
#else
#define NormalPipeGLock()
#define NormalPipeGUnLock()
#endif

static MUINT32 NPIPE_ALLOC_MEM = 0;
static MUINT32 NPIPE_FREE_MEM = 0;

#define NPIPE_MEM_NEW(dstPtr,type,size)\
do{\
       NPIPE_ALLOC_MEM += size; \
       dstPtr = new type;\
}while(0)


#define NPIPE_MEM_DEL(dstPtr,size)\
do{\
       NPIPE_FREE_MEM += size; \
       delete dstPtr;\
       dstPtr = NULL;\
}while(0)

#define NPIPE_DUMP_MEM_INFO(string) \
do {\
   if (1) {\
       PIPE_DBG("%s::NPIPE_ALLOC_MEM=0x%x, NPIPE_FREE_MEM=0x%x",string,NPIPE_ALLOC_MEM, NPIPE_FREE_MEM);\
   }\
}while(0)


/******************************************************************************
 *
 ******************************************************************************/
INormalPipe_FrmB*
INormalPipe_FrmB::createInstance(MUINT32 *pSensorIndex, MUINT32 sensorCnt,char const* szCallerName, MINT32 burstQnum)
{
    //NPIPE_DUMP_MEM_INFO("createInstance");

    if ((sensorCnt > NPIPE_Sensor_0And1) || \
        (sensorCnt > NPIPE_MAX_ENABLE_SENSOR_CNT)) {
        PIPE_ERR("Invalid sCnt=%d", sensorCnt);
    }

    NormalPipeGLock();
    if (!pAllSensorInfo){
        IHalSensorList *mSList = NULL;
        _sensorInfo_t  *pSInfo = NULL;
        MUINT32 SCnt = 0;

        NPIPE_MEM_NEW(pAllSensorInfo, platSensorsInfo, sizeof(platSensorsInfo));
        PIPE_INF("[%d]createInstance, %s,Alloc pAllSensorInfo=0x%8x",pSensorIndex[0],szCallerName, pAllSensorInfo);

        mSList = IHalSensorList::get();
#ifdef USING_MTK_LDVT
        mSList->searchSensors();
#endif
        SCnt =  mSList->queryNumberOfSensors();

        pAllSensorInfo->mSList       = mSList;
        pAllSensorInfo->mExistedSCnt = SCnt;
        pAllSensorInfo->mUserCnt     = 0;
        if ((SCnt > NPIPE_MAX_SENSOR_CNT) || (SCnt==0)) {
            PIPE_ERR("Not support NPIPE_MAX_SENSOR_CNT sensors " );
            NPIPE_MEM_DEL(pAllSensorInfo, sizeof(platSensorsInfo));
            pAllSensorInfo = NULL;
            NormalPipeGUnLock();
            return MFALSE;
        }

        for (int i = 0; i < SCnt; i++) {
            pSInfo = &pAllSensorInfo->mSenInfo[i];

            pSInfo->mIdx       = i;
            pSInfo->mTypeforMw = mSList->queryType(i);
            pSInfo->mDevId     = mSList->querySensorDevIdx(i);
            mSList->querySensorStaticInfo(pSInfo->mDevId, &pSInfo->mSInfo);
            PIPE_DBG("[%d]SensorName=%s, Type=%d, DevId=%d", \
                     i, mSList->queryDriverName(i), \
                     pSInfo->mTypeforMw, pSInfo->mDevId);
        }
    }
    pAllSensorInfo->mUserCnt++;
    NormalPipeGUnLock();


    NormalPipe_FrmB* pNPipe = 0;

    if (1 == sensorCnt) {//NPIPE_Sensor_0 ||  NPIPE_Sensor_1

        if (pSensorIndex[0] > NPIPE_Sensor_1){
            PIPE_ERR("INormalPipe_FrmB::createInstance InvalidSensorIdx = %d", pSensorIndex[0]);
            return MFALSE;
        }

        NormalPipeGLock();
        pNPipe = pNormalPipe_FrmB[pSensorIndex[0]];
        if (NULL == pNPipe)  {

            //pNPipe = new NormalPipe(&pSensorIndex[0], 1, szCallerName);
            NPIPE_MEM_NEW(pNormalPipe_FrmB[pSensorIndex[0]], NormalPipe_FrmB(&pSensorIndex[0], 1, szCallerName, burstQnum), sizeof(NormalPipe_FrmB));
            pNPipe = pNormalPipe_FrmB[pSensorIndex[0]];
            pNPipe->mUserCnt= 0;
            pNPipe->mpSensorIdx[0] = pSensorIndex[0];
            pNPipe->mpEnabledSensorCnt = sensorCnt;
            PIPE_INF("[%d]createInstance, %s,pNormalPipe_FrmB[%d]=0x%8x create\n",pSensorIndex[0],szCallerName, pSensorIndex[0], pNPipe);

            if (pNPipe->mpFrmB_Thread == NULL){
                pNPipe->mpFrmB_Thread = NormalPipe_FrmB_Thread::CreateInstance(pNPipe);
                if(pNPipe->mpFrmB_Thread == NULL){
                    PIPE_ERR("error: FrmB_Thread:createinstance fail\n");
                    return NULL;
                }
            }
        }
        else {
            //PIPE_DBG("pNormalPipe_FrmB[%d]=0x%8x exist\n", pSensorIndex[0], pNPipe);
        }
        pNPipe->mUserCnt++;
        NormalPipeGUnLock();
    }
    else if (2 == sensorCnt){
        PIPE_ERR("Not Support Create 2 sensor at the same time\n");
   }

    //PIPE_DBG("mUserCnt=%d -",pNPipe->mUserCnt);
    //PIPE_DBG("createInstance-.(0x%8x)NPipe[%d]->mUserCnt(%d) ", pNPipe, pNPipe->mpSensorIdx[0], pNPipe->mUserCnt);

    return pNPipe;
}


INormalPipe_FrmB*
INormalPipe_FrmB::createInstance(MUINT32 SensorIndex,char const* szCallerName, MINT32 burstQnum)
{
   return createInstance(&SensorIndex, 1, szCallerName,burstQnum);
}


INormalPipe_FrmB*
INormalPipe_FrmB::createInstance( char const* szCallerName, MINT32 burstQnum)
{
   MUINT32 sidx = 0;
    return createInstance(&sidx, 1, szCallerName,burstQnum);
}


void
INormalPipe_FrmB::destroyInstance(char const* szCallerName)
{
    int idx;
    //PIPE_DBG("INormalPipe_FrmB::destroyInstance %s", szCallerName);

    return;
}


/******************************************************************************
 *
 ******************************************************************************/

IMetadata const&
INormalPipe_FrmB::queryCapability(MUINT32 iOpenId) //iOpenId is sensorID
{

    Mutex::Autolock _l(gNPipeCapMutex);

    if  ( ! gPipeCap.isEmpty() )  {
        return  gPipeCap;
    }

    //  IMG2O
    {
        IMetadata& cap = gPortCap_IMG2O;
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
            entry.push_back(PORT_IMG2O, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }

        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_TRANSFORM);
            entry.push_back(0, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }

        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
            entry.push_back(MTK_IOPIPE_INFO_CROP_SYMMETRIC, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }

        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_LINEBUFFER);
            entry.push_back(3072, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }

        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
            entry.push_back(eImgFmt_BAYER8, Type2Type<MINT32>());
            entry.push_back(eImgFmt_BAYER10, Type2Type<MINT32>());
            entry.push_back(eImgFmt_BAYER12, Type2Type<MINT32>());
            entry.push_back(eImgFmt_YUY2, Type2Type<MINT32>());
            entry.push_back(eImgFmt_YVYU, Type2Type<MINT32>());
            entry.push_back(eImgFmt_UYVY, Type2Type<MINT32>());
            entry.push_back(eImgFmt_VYUY, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
    }
    //  IMGO
    {

    }

    //
    //  Pipe
    {
        IMetadata& cap = gPipeCap;
        {   // out port
        IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_OUT_PORT_INFO);
        entry.push_back(gPortCap_IMGO, Type2Type<IMetadata>());
        entry.push_back(gPortCap_IMG2O, Type2Type<IMetadata>());
        cap.update(entry.tag(), entry);
        }
        cap.sort();
    }
    //
    PIPE_DBG("count:%d", gPipeCap.count());
    return  gPipeCap;

}


/******************************************************************************
 *
 ******************************************************************************/
unsigned long NormalPipe_FrmB::mTwinEnable[CAM_SV_2+1][2] = {{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single}};//record this & twin mode,0 for this, 1 for twinmode
MUINT32 NormalPipe_FrmB::mEnablePath = 0;

MVOID
NormalPipe_FrmB::destroyInstance(char const* szCallerName)
{
    int idx;
    char const* szNormalPipeName = "NormalPipe";

    NormalPipeGLock();
    for (idx = 0; idx < NPIPE_Sensor_RSVD; idx++){
       if (this == pNormalPipe_FrmB[idx]){
          break;
       }
    }

    if ((pNormalPipe_FrmB[idx]->mUserCnt <= 0)){
        PIPE_DBG("INormalPipe_FrmB::destroyInstance. No instance now");
        NormalPipeGUnLock();
        return;
    }

    if (NPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("INormalPipe_FrmB::destroyInstance. Can't find the entry");
        NormalPipeGUnLock();
        return;
    }

    if (pNormalPipe_FrmB[idx]->mUserCnt > 0)  {
        pNormalPipe_FrmB[idx]->mUserCnt--;
    }


    if ((pNormalPipe_FrmB[idx]->mUserCnt == 0))  {
        if  ( mpHalSensor == NULL )  {
            PIPE_ERR("NULL pHalSensor");
            NormalPipeGUnLock();
            return;
        }
        mpHalSensor->destroyInstance(szNormalPipeName);
        mpHalSensor= NULL;

        this->mpFrmB_Thread->DestroyInstance();
        this->mpFrmB_Thread = NULL;

        if(this->mpCamIOPipe != NULL)
            mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
        PIPE_INF("Destroy NPipe[%d]=0x%8x",idx, pNormalPipe_FrmB[idx]);
        NPIPE_MEM_DEL(pNormalPipe_FrmB[idx], sizeof(NormalPipe_FrmB));
    }

    if (pAllSensorInfo) {
       if (pAllSensorInfo->mUserCnt > 0){
           pAllSensorInfo->mUserCnt--;
       }
       else{
          //return;
       }

       if ((pAllSensorInfo->mUserCnt == 0) && (pAllSensorInfo)){
           PIPE_DBG("Destroy pAllSensorInfo. idx=%d",idx);
           NPIPE_MEM_DEL(pAllSensorInfo, sizeof(platSensorsInfo));
           NPIPE_DUMP_MEM_INFO("Destroy");
           //NPIPE_ALLOC_MEM = 0;
           //NPIPE_FREE_MEM = 0;
       }

    }

    NormalPipeGUnLock();

    //PIPE_DBG("destroyInstance-.pNormalPipe_FrmB =0x%8x/0x%8x/0x%8x", pNormalPipe_FrmB[0], pNormalPipe_FrmB[1], pNormalPipe_FrmB[2]);

}


/******************************************************************************
 *
 ******************************************************************************/
NormalPipe_FrmB::NormalPipe_FrmB(MUINT32* pSensorIdx, MUINT32 SensorIdxCnt, char const* szCallerName, MINT32 burstQnum)
    : IHalCamIO()
    , mpHalSensor(NULL)
    , mpCamIOPipe(NULL)
    , mpFrameMgr(NULL)
    , mpName(szCallerName)
    , mBurstQNum(1)
    , mUpdateBQNumDone(MFALSE)
    , mpFrmB_Thread(NULL)
{
    mpHalSensor  = NULL;
    mpEnabledSensorCnt = SensorIdxCnt;

    mbTwoPixelEnable = MFALSE;
    mConfigDone = MFALSE;
    mImg2oOut_size.w = 0;
    mImg2oOut_size.h = 0;
    //update burstQnumber if mw member update it, otherwise use default value 1
    PIPE_DBG("income BQNum(%d)",burstQnum);
    mBurstQNum=burstQnum;
    mOpenedPort = _DMAO_NONE;
    m_bStarted = MFALSE;
    mRingBufDepth = 0;

    for(int i=0;i<MaxPortIdx;i++){
        mpEnqueRequest[i] = NULL;
        mpDummyRequest[i] = NULL;
        mpEnqueReserved[i] = NULL;
        mpDeQueue[i] = NULL;
        PortIndex[i] = 0;
    }
    DBG_LOG_CONFIG(imageio, iopipe);
    DBG_LOG_CONFIG(imageio, iopipetime);
    m_DropCB = NULL;
    m_returnCookie = NULL;
    mPrvEnqSOFIdx = 0xFFFFFFFF;
    m_b1stEnqLoopDone = MFALSE;
    m_PerFrameCtl = MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::start()
{
    MBOOL ret = MTRUE;
       MUINT32 _size=0,_tmp;
    PIPE_INF("E:");
    if(this->mConfigDone == MFALSE)
    {
        PIPE_ERR("error: mConfigDone=0\n");
        return MFALSE;
    }


    this->m_bStarted = MTRUE;

    //chk dummyframe depth and ringbuffer depth,
    //dummy depth == ringbuf depth is for easy debug.
    for(int i=0;i<this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum);i++){
        this->mpDummyRequest[_PortMap(this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize,(MUINTPTR)&_tmp,0);
        _size += _tmp;
    }
    if(_size != this->mRingBufDepth){
        this->m_bStarted = MFALSE;
        PIPE_ERR("error:dummy buffer num != ringbuf num (0x%x != 0x%x), enabled port:0x%x\n",\
            _size,this->mRingBufDepth,this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Opened));
        return MFALSE;
    }
    if (this->mpFrmB_Thread){
        if(this->mpFrmB_Thread->Start() == MFALSE){
            PIPE_ERR("error: FrmB_thread start fail\n");
            return MFALSE;
        }
    }
    if (this->mpCamIOPipe)
    {
        ret = this->mpCamIOPipe->start();
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::stop()
{
    bool ret = MTRUE;

    PIPE_DBG("++++ stop[%d] ++++",this->mpSensorIdx[0]);

    if (mpCamIOPipe)
    {
        if(this->mpFrmB_Thread){
            if(this->mpFrmB_Thread->PreStop() == MFALSE){
                PIPE_ERR("error: FrmB_Thread prestop fail\n");
                return MFALSE;
            }
        }
        PIPE_TRACE_BEGIN("TAG_STOP");
        ret = mpCamIOPipe->stop();
        //for twin mode chk under 2 sensor
        if(mTwinEnable[CAM_TG_1][0] == (unsigned long)this){
            mTwinEnable[CAM_TG_1][0] = mTwinEnable[CAM_TG_1][1] = ePxlMode_One_Single;
            mEnablePath -= NPIPE_PASS_PASS1;
        }
        PIPE_TRACE_END();

        PIPE_TRACE_BEGIN("TAG_THREAD_STOP");
        if(this->mpFrmB_Thread){
            if(this->mpFrmB_Thread->Stop() == MFALSE){
                PIPE_ERR("error: FrmB_Thread stop fail\n");
                return MFALSE;
            }
        }
        PIPE_TRACE_END();
    }
    PIPE_DBG("---- stop[%d] ----",this->mpSensorIdx[0]);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::init()
{
    MUINT32 idx;
    bool ret = MTRUE;
    //char const* szCallerName = "NormalPipe_FrmB";
    PIPE_DBG("NormalPipe_FrmB::init+ SenIdx=%d", mpSensorIdx[0]);

    if(this->mpFrmB_Thread){
        if(this->mpFrmB_Thread->init() == MFALSE){
            PIPE_ERR("error: FrmB_Thread init fail\n");
            return MFALSE;
        }
    }

    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->init();
    }


#ifdef USE_IMAGEBUF_HEAP
    if (!mpFrameMgr) {
        NPIPE_MEM_NEW(mpFrameMgr,FrameMgr(),sizeof(FrameMgr));
    }
    mpFrameMgr->init();

    for(int i=0;i<MaxPortIdx;i++){
        if(!mpEnqueRequest[i]){
            NPIPE_MEM_NEW(mpEnqueRequest[i],QueueMgr(),sizeof(QueueMgr));
            mpEnqueRequest[i]->init();
        }
        if(!mpDummyRequest[i]){
            NPIPE_MEM_NEW(mpDummyRequest[i],QueueMgr(),sizeof(QueueMgr));
            mpDummyRequest[i]->init();
        }
        if(!mpEnqueReserved[i]){
            NPIPE_MEM_NEW(mpEnqueReserved[i],QueueMgr(),sizeof(QueueMgr));
            mpEnqueReserved[i]->init();
        }
        if(!mpDeQueue[i]){
            NPIPE_MEM_NEW(mpDeQueue[i],QueueMgr(),sizeof(QueueMgr));
            mpDeQueue[i]->init();
        }
    }
#endif

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::uninit()
{
    MUINT32 idx;
    bool ret = MTRUE;
    MUINT32 SIdx;

    PIPE_DBG("uninit+,  SenIdx=%d", mpSensorIdx[0]);

    //Mutex::Autolock lock(mCfgLock);
    if (this->mpFrmB_Thread){
        if(this->mpFrmB_Thread->uninit() == MFALSE){
            PIPE_ERR("error FrmB_Thread uninit fail\n");
            return MFALSE;
        }
    }
    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->uninit();
        mConfigDone = MFALSE;
    }

    //PIPE_DBG("after mpCamIOPipe->uninit");

    //20140305: Middleware(Allan) will control the powerOn/Off life cycle.
    //if (mpHalSensor)  {
    //    mpHalSensor->powerOff("NormalPipe_FrmB", mpEnabledSensorCnt, mpSensorIdx);
    //}

    for (int i = 0; i < mpEnabledSensorCnt; i++)  {
        SIdx = mpSensorIdx[i];

        NormalPipeGLock();
        if (pAllSensorInfo) {
            pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINT32)NULL;
        }
        else {
            PIPE_ERR("error:NormalPipe_FrmB::uninit: pAllSensorInfo NULL");
        }
        NormalPipeGUnLock();
    }


#ifdef USE_IMAGEBUF_HEAP

    if (mpFrameMgr)  {
        mpFrameMgr->uninit();
        NPIPE_MEM_DEL(mpFrameMgr,sizeof(FrameMgr));
    }
#endif
    for(int i=0;i<MaxPortIdx;i++){
        if (mpEnqueRequest[i])  {
            mpEnqueRequest[i]->uninit();
            NPIPE_MEM_DEL(mpEnqueRequest[i],sizeof(QueueMgr));
        }
        if (mpDummyRequest[i])  {
            mpDummyRequest[i]->uninit();
            NPIPE_MEM_DEL(mpDummyRequest[i],sizeof(QueueMgr));
        }
        if (mpEnqueReserved[i])  {
            mpEnqueReserved[i]->uninit();
            NPIPE_MEM_DEL(mpEnqueReserved[i],sizeof(QueueMgr));
        }
        if (mpDeQueue[i])  {
            mpDeQueue[i]->uninit();
            NPIPE_MEM_DEL(mpDeQueue[i],sizeof(QueueMgr));
        }
    }
    PIPE_DBG("uninit-,  SenIdx=%d", mpSensorIdx[0]);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::enque(QBufInfo const& rQBuf)
{
    bool ret = MTRUE;
    NSImageio_FrmB::NSIspio_FrmB::PortID portID;
    NSImageio_FrmB::NSIspio_FrmB::QBufInfo rQBufInfo;
    BufInfo _buf;
    //
    //PIPE_DBG("NormalPipe_FrmB::enque +");
    //

    Mutex::Autolock lock(this->mEnQLock);
    this->mEnqContainerLock.lock();

    if(this->checkEnque(rQBuf) == MFALSE){
        PIPE_ERR("enque fail\n");
        this->mEnqContainerLock.unlock();
        return MFALSE;
    }

    if(this->m_bStarted == MFALSE){
        //isp not started yet, push buffer into FBC directly
        for (MUINT32 i = 0; i < rQBuf.mvOut.size(); i++)
        {
            this->mRingBufDepth++;
            //
            portID.index = rQBuf.mvOut.at(i).mPortID.index;

            //
#ifdef USE_IMAGEBUF_HEAP
            mpFrameMgr->enque(rQBuf.mvOut.at(i).mBuffer);
#endif

            rQBufInfo.vBufInfo.resize(1);
            //rQBufInfo.vBufInfo[0].memID =       (MINT32)(imgo_memId[i]); //
#ifdef USE_IMAGEBUF_HEAP
            rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)rQBuf.mvOut.at(i).mBuffer->getBufSizeInBytes(0); //bytes
            rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINT32)rQBuf.mvOut.at(i).mBuffer->getBufVA(0); //
            rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINT32)rQBuf.mvOut.at(i).mBuffer->getBufPA(0); //
#else
            rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)rQBuf.mvOut.at(i).mSize;
            rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINT32)rQBuf.mvOut.at(i).mVa;
            rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINT32)rQBuf.mvOut.at(i).mPa;
#endif
            rQBufInfo.vBufInfo[0].header  =     NULL;//rQBuf.mvOut.at(i).mMetaData.mPrivateData;
            rQBufInfo.vBufInfo[0].mBufIdx = rQBuf.mvOut.at(i).mBufIdx; //used when replace buffer

            //
            if (mpCamIOPipe)  {
                //bImdMode r no needed before isp start
                if ( false == mpCamIOPipe->enqueOutBuf(portID, rQBufInfo) ) {
                    PIPE_ERR("error:enqueOutBuf");
                    this->mEnqContainerLock.unlock();
                    return MFALSE;
                }
            }
        }
    }
    else{
        if(this->checkDropEnque(rQBuf) == MFALSE)
            goto _enque_exit;
    }

    //all enque request r all push into enquereqest , but enque before start have special control flow at enquerequest()
    {
        for (MUINT32 i = 0; i < rQBuf.mvOut.size(); i++)
        {
            _buf = rQBuf.mvOut.at(i);

            switch(_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0]))){
                case CAM_TG_1:
                    break;
                default:
                    PIPE_ERR("unsupport TG:0x%x\n",_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0])));
                    return MFALSE;
                    break;
            }
            this->mpEnqueRequest[_PortMap(_buf.mPortID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_push,(MUINTPTR)&_buf,0);
            if (this->m_bStarted == MFALSE)
            {
                PIPE_INF("MyEnq: dma(0x%x) pa(0x%x) io(0x%x_0x%x_0x%x_0x%x_0x%x_0x%x) mag(0x%x)\n",_buf.mPortID.index,\
                    _buf.mBuffer->getBufPA(0),\
                    _buf.FrameBased.mCropRect.p.x,_buf.FrameBased.mCropRect.p.y,_buf.FrameBased.mCropRect.s.w,_buf.FrameBased.mCropRect.s.h,_buf.FrameBased.mDstSize.w,_buf.FrameBased.mDstSize.h,\
                    _buf.FrameBased.mMagicNum_tuning);
            }
            else
            {
                PIPE_DBG("MyEnq: dma(0x%x) pa(0x%x) io(0x%x_0x%x_0x%x_0x%x_0x%x_0x%x) mag(0x%x)\n",_buf.mPortID.index,\
                    _buf.mBuffer->getBufPA(0),\
                    _buf.FrameBased.mCropRect.p.x,_buf.FrameBased.mCropRect.p.y,_buf.FrameBased.mCropRect.s.w,_buf.FrameBased.mCropRect.s.h,_buf.FrameBased.mDstSize.w,_buf.FrameBased.mDstSize.h,\
                    _buf.FrameBased.mMagicNum_tuning);
            }
        }
    }

#if 0// check all element in enque container
        {
            MUINT32 _size;
            BufInfo _buf;
            for(int i=0;i<this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum);i++){
                this->mpEnqueRequest[_PortMap(this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINT32)&_size,0);
                for(int j=0;j<_size;j++){
                    if(MFALSE == this->mpEnqueRequest[_PortMap(this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_at  ,j,(MUINT32)&_buf))
                        PIPE_INF("F:enque port:0x%x at 0x%x fail\n",this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i),j);
                    PIPE_INF("F: enque Port:0x%x, PA:0x%x, mag:0x%x\n",\
                        _buf.mPortID.index,\
                        _buf.mPa,\
                        _buf.mMetaData.mMagicNum_hal);
                }
            }
        }
        PIPE_DBG("Dequeue:-\n");
#endif
    //flush to cmdQ at very first enque before start
    if((this->m_bStarted == MFALSE) && (this->GetRingBufferDepth() == 1) ){
        this->mpFrmB_Thread->ImmediateRequest(MTRUE);
    }
_enque_exit:
    //
    //PIPE_DBG("NormalPipe_FrmB::enque -");
    this->mEnqContainerLock.unlock();
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::DummyFrame(QBufInfo const& rQBuf){
    MBOOL ret = MTRUE;
    BufInfo _buf;

    if(this->checkEnque(rQBuf) == MFALSE){
        PIPE_ERR("Dummy frame fail\n");
        return MFALSE;
    }

    if(this->m_bStarted == MTRUE){
        PIPE_ERR("error:isp already started, can't push dummy frame\n");
        return MFALSE;
    }


    for(int i=0;i<rQBuf.mvOut.size();i++){
        _buf = rQBuf.mvOut.at(i);
        _buf.FrameBased.mMagicNum_tuning |= _DUMMY_MAGIC;
        switch(_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0]))){
            case CAM_TG_1:
                break;
            default:
                PIPE_ERR("unsupport TG:0x%x\n",_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0])));
                return MFALSE;
                break;
        }
        this->mpDummyRequest[_PortMap(_buf.mPortID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_push,(MUINTPTR)&_buf,0);
        PIPE_DBG("MyEnq_dummy: dma(0x%x) pa(0x%x) io(0x%x_0x%x_0x%x_0x%x_0x%x_0x%x) mag(0x%x)\n",_buf.mPortID.index,\
                _buf.mBuffer->getBufPA(0),\
                _buf.FrameBased.mCropRect.p.x,_buf.FrameBased.mCropRect.p.y,_buf.FrameBased.mCropRect.s.w,_buf.FrameBased.mCropRect.s.h,_buf.FrameBased.mDstSize.w,_buf.FrameBased.mDstSize.h,\
                _buf.FrameBased.mMagicNum_tuning);
    }

#if 0
{
    MUINT32 _size;
    BufInfo buf;

    if(this->mOpenedPort & NormalPipe_FrmB::_IMGO_ENABLE)
        this->mpDummyRequest[_PortMap(PORT_IMGO.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize,(MUINT32)&_size,0);
    else
        this->mpDummyRequest[_PortMap(PORT_IMG2O.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize,(MUINT32)&_size,0);

    for(int i=0;i<this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum);i++){
        this->mpDummyRequest[_PortMap(this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize,(MUINT32)&_size,0);
        for(int j=0;j<_size;j++){
            if(MFALSE == this->mpDummyRequest[_PortMap(this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_at,j,(MUINT32)&buf))
                PIPE_ERR("error: port:0x%x fail at 0x%x\n",this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i),j);
            PIPE_DBG("DummyFrame: Port:0x%x, PA:0x%x, mag:0x%x\n",\
                                buf.mPortID.index,\
                                buf.mBuffer->getBufPA(0),\
                                buf.FrameBased.mMagicNum_tuning);
        }
    }
}
#endif
    return ret;
}
/*****************************************************************************
*
******************************************************************************/
MBOOL
NormalPipe_FrmB::checkEnque(QBufInfo const& rQBuf)
{
    MBOOL ret = MTRUE;
    ISP_QUERY_RST queryRst;
    E_ISP_PIXMODE _PixMode;
    E_ISP_QUERY   _op;
    if (MTRUE == rQBuf.mvOut.empty())  {
        PIPE_ERR("error:queue empty");
        return MFALSE;
    }
    if (!this->mConfigDone) {
        PIPE_ERR("error:Not Rdy, Call configPipe first, SenIdx=%d", this->mpSensorIdx[0]);
        return MFALSE;
    }

    if(rQBuf.mvOut.size() != this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum)){
        PIPE_ERR("error:current enabled port number:0x%x,enqued port:0x%x, number r mismatch\n",this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum),rQBuf.mvOut.size());
        return MFALSE;
    }

    //TG + twinmode + dmao combinational check
    for(MUINT32 i = 0; i < rQBuf.mvOut.size(); i++){
        if( rQBuf.mvOut.at(i).mBufIdx != 0xFFFF){
            PIPE_ERR("error:replace buffer with the same address r not supported in v3, plz make sure mBufIdx=0xffff\n");
            return MFALSE;
        }
        //for HDRViewFinder: if current SOF = EnqueRequest SOF, reserve this request
        if(0xFFFF == rQBuf.mvOut.at(i).FrameBased.mSOFidx){
            PIPE_ERR("error:buffer brings no sof idx(%x)\n", rQBuf.mvOut.at(i).FrameBased.mSOFidx);
            return MFALSE;
        }

        if (this->mbTwoPixelEnable){
            //in case of enque p1_d damo under twin-mode
            PIPE_ERR("error:twin mode r not supported\n");
            return MFALSE;
        }
        else {

            if ( PORT_IMG2O == rQBuf.mvOut.at(i).mPortID || PORT_IMGO == rQBuf.mvOut.at(i).mPortID ){
                if (CAM_TG_1 != _NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0])) {
                    PIPE_ERR("error:PORT_IMG2O/PORT_IMGO belong to CAM_TG_1");
                    return MFALSE;
                }
            }
            else {
                PIPE_ERR("error: support on IMGO/IMG2O dma port \n");
                return MFALSE;
            }
        }
    }
    //IO checking
    for(MUINT32 i = 0; i < rQBuf.mvOut.size(); i++){
        MSize _tgsize = this->mTgOut_size[_TGMapping(_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0])))];
        if(rQBuf.mvOut.at(i).mPortID == PORT_IMGO){
            //
            _op = (_tgsize.w != rQBuf.mvOut.at(i).FrameBased.mDstSize.w)?(ISP_QUERY_CROP_X_PIX):(ISP_QUERY_X_PIX);
            //
            if( (_tgsize.w < rQBuf.mvOut.at(i).FrameBased.mDstSize.w) || (_tgsize.h < rQBuf.mvOut.at(i).FrameBased.mDstSize.h) ){
                PIPE_ERR("error: imgo: out size(0x%x_0x%x) > tg size(0x%x_0x%x)\n",rQBuf.mvOut.at(i).FrameBased.mDstSize.w,rQBuf.mvOut.at(i).FrameBased.mDstSize.h,_tgsize.w,_tgsize.h);
                return MFALSE;
            }
            if((rQBuf.mvOut.at(i).FrameBased.mDstSize.w > rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w) || (rQBuf.mvOut.at(i).FrameBased.mDstSize.h > rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h)){
                PIPE_ERR("error: imgo: out size(0x%x_0x%x) > crop size(0x%x_0x%x)\n",rQBuf.mvOut.at(i).FrameBased.mDstSize.w,rQBuf.mvOut.at(i).FrameBased.mDstSize.h,\
                rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w,rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h);
                return MFALSE;
            }
        }
        else if(rQBuf.mvOut.at(i).mPortID == PORT_IMG2O){
            //
            _op = ISP_QUERY_X_PIX;
            //
            if( (rQBuf.mvOut.at(i).FrameBased.mDstSize.w == 0) || (rQBuf.mvOut.at(i).FrameBased.mDstSize.h == 0) ){
                PIPE_ERR("error: hrz: out size is 0(0x%x_0x%x)\n",rQBuf.mvOut.at(i).FrameBased.mDstSize.w,rQBuf.mvOut.at(i).FrameBased.mDstSize.h);
                return MFALSE;
            }
            if( (_tgsize.w < rQBuf.mvOut.at(i).FrameBased.mDstSize.w) || (_tgsize.h < rQBuf.mvOut.at(i).FrameBased.mDstSize.h) ){
                PIPE_ERR("error: hrz: out size(0x%x_0x%x) > tg size(0x%x_0x%x)\n",rQBuf.mvOut.at(i).FrameBased.mDstSize.w,rQBuf.mvOut.at(i).FrameBased.mDstSize.h,_tgsize.w,_tgsize.h);
                return MFALSE;
            }
            //
            //because cropping coordinate can't be based on TG coordinate. so , not supported.
            if( (rQBuf.mvOut.at(i).FrameBased.mCropRect.p.x != 0) || (rQBuf.mvOut.at(i).FrameBased.mCropRect.p.y != 0) || \
                (rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w != _tgsize.w) || \
                (rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h != _tgsize.h) ){
                PIPE_ERR("error: dma cropping r not supported in IMG2O\n");
                return MFALSE;
            }
        }
        if( (rQBuf.mvOut.at(i).FrameBased.mCropRect.p.x + rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w) > _tgsize.w){
            PIPE_ERR("error: witdh: crop region out of grab window(0x%x_0x%x_0x%x) \n",\
            rQBuf.mvOut.at(i).FrameBased.mCropRect.p.x,rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w,_tgsize.w);
            return MFALSE;
        }
        if( (rQBuf.mvOut.at(i).FrameBased.mCropRect.p.y + rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h) > _tgsize.h){
            PIPE_ERR("error: height: crop region out of grab window(0x%x_0x%x_0x%x) \n",\
            rQBuf.mvOut.at(i).FrameBased.mCropRect.p.y,rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h,_tgsize.h);
            return MFALSE;
        }

        if(this->mbTwoPixelEnable){
            PIPE_ERR("error: 2 pix r not supported\n");
            return MFALSE;
        }
        _PixMode = (this->mbTwoPixelEnable)? ISP_QUERY_2_PIX_MODE:ISP_QUERY_1_PIX_MODE;

#define __FMT(portidx,fmt)({\
    MUINT32 _fmt = fmt;\
    if((portidx == EPortIndex_RRZO) || (portidx == EPortIndex_RRZO_D)){\
        switch(fmt){\
            case eImgFmt_BAYER8: _fmt = eImgFmt_FG_BAYER8; break;    \
            case eImgFmt_BAYER10: _fmt = eImgFmt_FG_BAYER10; break; \
            case eImgFmt_BAYER12: _fmt = eImgFmt_FG_BAYER12; break; \
            default: _fmt = eImgFmt_FG_BAYER10; break;                \
        }\
    }\
    _fmt;})

        if(ISP_QuerySize(rQBuf.mvOut.at(i).mPortID.index,_op,(EImageFormat)__FMT(rQBuf.mvOut.at(i).mPortID.index,rQBuf.mvOut.at(i).mBuffer->getImgFormat()),rQBuf.mvOut.at(i).FrameBased.mDstSize.w,queryRst,_PixMode) == 0){
            PIPE_ERR(" fmt(0x%x) | dma(0x%x) err\n",__FMT(rQBuf.mvOut.at(i).mPortID.index,rQBuf.mvOut.at(i).mBuffer->getImgFormat()),rQBuf.mvOut.at(i).mPortID.index);
            return MFALSE;
        }
        else{
            if(rQBuf.mvOut.at(i).FrameBased.mDstSize.w != queryRst.x_pix){
                PIPE_ERR("error:port:0x%x width r invalid under op:0x%x. cur:0x%x, valid:0x%x\n",rQBuf.mvOut.at(i).mPortID.index,_op,rQBuf.mvOut.at(i).FrameBased.mDstSize.w,queryRst.x_pix);
                return MFALSE;
            }
        }
    }
#undef __FMT

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::checkDropEnque(QBufInfo const& rQBuf)
{
    NormalPipe_FRM_STATUS _drop_status;
    MUINT32 _sof_idx,_size;
    BufInfo _buf,_popbuf;

    this->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINTPTR)&_drop_status,0,0);
    this->sendCommand(EPIPECmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx,0,0);

    if(this->m_PerFrameCtl && _drop_status == _drop_frame_status){
        PIPE_INF("warning:cur frame r drop frame, drop this enque request(pa:0x%x,mag:0x%x)\n",(MUINT32)rQBuf.mvOut.at(0).mBuffer->getBufPA(0),rQBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
        if(this->m_DropCB)
            this->m_DropCB(rQBuf.mvOut.at(0).FrameBased.mMagicNum_tuning, this->m_returnCookie);

        this->mPrvEnqSOFIdx = _sof_idx;
        return MFALSE;
    }

    //to chk if enque twice within the the frame
    //drop previous enque request and push this request into enque container
    if(this->m_PerFrameCtl && _sof_idx == this->mPrvEnqSOFIdx){
        for (int i = 0; i < rQBuf.mvOut.size(); i++)
        {
            _buf = rQBuf.mvOut.at(i);

            this->mpEnqueRequest[_PortMap(_buf.mPortID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize,(MUINTPTR)&_size,0);
            if(_size == 0){
                //if enque just on sof , and enquethread awake after sof. this case is ok.
                this->mPrvEnqSOFIdx = _sof_idx;
                return MTRUE;
            }
            else{
                this->mpEnqueRequest[_PortMap(_buf.mPortID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_at,_size-1,(MUINTPTR)&_popbuf);
                this->mpEnqueRequest[_PortMap(_buf.mPortID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop_back,0,0);
            }
        }

        PIPE_INF("warning:can't push enque into drv more than once within 1 frame,drop request(pa:0x%x,mag:0x%x)sof(%d/%d)\n",
            (MUINT32)_popbuf.mBuffer->getBufPA(0),_popbuf.FrameBased.mMagicNum_tuning, this->mPrvEnqSOFIdx,_sof_idx);
        if(this->m_DropCB)
            this->m_DropCB(_popbuf.FrameBased.mMagicNum_tuning, this->m_returnCookie);
    }

    this->mPrvEnqSOFIdx = _sof_idx;
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::deque(QBufInfo& rQBuf, MUINT32 u4TimeoutMs)
{
    //PIPE_DBG("[MyDeQ:%d]+++++++++++++++++++++++",mpSensorIdx[0]);
    bool ret = MTRUE;
    vector<NSImageio_FrmB::NSIspio_FrmB::PortID> QportID; //used for CamIOPipe
    NSImageio_FrmB::NSIspio_FrmB::QTimeStampBufInfo rQTSBufInfo;
    IImageBuffer *pframe= NULL;
    NSImageio_FrmB::NSIspio_FrmB::PortID  portID;    //used for CamIOPipe
    BufInfo _buf;
    MUINT32 _size;


    if (0 == rQBuf.mvOut.size()) {
        PIPE_ERR("Error: dequeue container is empty");
        return MFALSE;
    }
    if (!this->mConfigDone) {
        PIPE_ERR("[deque]Not Rdy, Call configPipe first, SenIdx=%d", this->mpSensorIdx[0]);
        return MFALSE;
    }

    for (MUINT32 ii=0 ; ii<rQBuf.mvOut.size() ; ii++ ) {
        //
        portID.index = rQBuf.mvOut.at(ii).mPortID.index;
        if (MFALSE == this->mpCamIOPipe->dequeOutBuf(portID, rQTSBufInfo) ) {
            PIPE_ERR("error:[MyDeQ]tg:0x%x deque[PorID=%d]", _NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0]),portID.index);
            return MFALSE;
        } //else {
        //PIPE_DBG("[MyDeQ]deque[PorID=%d] OK, cnt = %d", portID.index, rQTSBufInfo.vBufInfo.size());
        //}
        //

        if ( rQTSBufInfo.vBufInfo.size() >= 1 )  {

            for (MUINT32 idx = 0; idx < rQTSBufInfo.vBufInfo.size(); idx++)  {
                BufInfo buff;
                ResultMetadata result;
                if(idx >= 1){
                    PIPE_ERR("[MyDeQ] dequed image from FBC r over 1 buffer(0x%x) at portID:0x%x\n",\
                    rQTSBufInfo.vBufInfo.size(),portID.index);
                    PIPE_ERR("[MyDeQ] current deuqe,normal-pipe r supported only 1 buffer for 1 port at 1 time\n");
                    PIPE_ERR("[MyDeQ] drop remainded dequed buffer, and deque/enque flow will be crashed after this op.\n");
                    break;
                }

#ifdef USE_IMAGEBUF_HEAP
                pframe = mpFrameMgr->deque(rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0]);//chage from va 2 pa is because of in camera3, drv may have no va infor.
                if (NULL == pframe) {
                    PIPE_ERR("[MyDeQ:%d]Fail: PortId(%d), 0==pframe, idx=%d, pa=0x%x",\
                    mpSensorIdx[0],portID.index,idx, rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0]);
                    return MFALSE;
                }
                pframe->setTimestamp(rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_ns());
#endif

                result.mDstSize = rQTSBufInfo.vBufInfo.at(idx).DstSize;
                result.mTransform = 0;
                result.mMagicNum_hal = rQTSBufInfo.vBufInfo.at(idx).m_num_0;
                result.mMagicNum_tuning = rQTSBufInfo.vBufInfo.at(idx).m_num_1;
                result.mTimeStamp = getTimeStamp_ns(rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_sec,  rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_us);
                if(rQBuf.mvOut.at(ii).mPortID == PORT_IMGO){
                    result.mCrop_s = MRect(MPoint(0,0),MSize(rQTSBufInfo.vBufInfo.at(idx).img_w,rQTSBufInfo.vBufInfo.at(idx).img_h));
                    result.mCrop_d = rQTSBufInfo.vBufInfo.at(idx).crop_win;
                }
                else if(rQBuf.mvOut.at(ii).mPortID == PORT_IMG2O){
                    result.mCrop_s = rQTSBufInfo.vBufInfo.at(idx).crop_win;
                    result.mCrop_d = MRect(MPoint(0,0),MSize(result.mDstSize.w,result.mDstSize.h));
                }
                else{
                }

                buff.mPortID = rQBuf.mvOut.at(ii).mPortID;
                buff.mBuffer = pframe;
                buff.mMetaData = result;
                buff.mSize = rQTSBufInfo.vBufInfo.at(idx).u4BufSize[0];
                buff.mVa = rQTSBufInfo.vBufInfo.at(idx).u4BufVA[0];
                buff.mPa = rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0];

                //check if this frame is dummy or not
                //add semephore to avoid Enque thread access "mpDummyRequest" at the same time
                this->mEnqContainerLock.lock();
                this->mpDummyRequest[_PortMap(portID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize,(MUINTPTR)&_size,0);
                for(MUINT32 i=0;i<_size;i++){
                    if(MFALSE == this->mpDummyRequest[_PortMap(portID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_at,i,(MUINTPTR)&_buf))
                        PIPE_ERR("error:dummy request fail: port:0x%x fail at 0x%x\n",portID.index,i);
                    else{
                        if(_buf.mBuffer->getBufPA(0) == buff.mPa){
                            buff.mMetaData.m_bDummyFrame = MTRUE;
                            break;
                        }
                    }
                }
                this->mEnqContainerLock.unlock();
                //
                rQBuf.mvOut.at(ii) = buff;
                this->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_push,(MUINTPTR)&buff,0);


                if(buff.mMetaData.m_bDummyFrame == MTRUE){
                    //avoid apply error m# due to bad system performance (ex. enque EnqueRequest over VYSNC)
                    if(!(_DUMMY_MAGIC & result.mMagicNum_hal))
                    {
                        result.mMagicNum_hal |= _DUMMY_MAGIC;
                        result.mMagicNum_tuning |= _DUMMY_MAGIC;
                        buff.mMetaData.mMagicNum_hal |= _DUMMY_MAGIC;
                        buff.mMetaData.mMagicNum_tuning |= _DUMMY_MAGIC;
                    }
                    //
                    PIPE_DBG("MyDeq: dummy frame: dma(0x%x) pa(0x%x) io(0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x) mag(0x%x_0x%x)\n",portID.index,\
                    buff.mPa,\
                    result.mCrop_s.p.x,result.mCrop_s.p.y,result.mCrop_s.s.w,result.mCrop_s.s.h, \
                    result.mCrop_d.p.x,result.mCrop_d.p.y,result.mCrop_d.s.w,result.mCrop_d.s.h, \
                    result.mDstSize.w,result.mDstSize.h,\
                    result.mMagicNum_hal,result.mMagicNum_tuning);
                    ret = MFALSE;
                }
                else{
                    //avoid apply error m# due to bad system performance (ex. enque EnqueRequest over VYSNC)
                    if(_DUMMY_MAGIC & result.mMagicNum_hal)
                    {
                        result.mMagicNum_hal &= (~_DUMMY_MAGIC);
                        result.mMagicNum_tuning &= (~_DUMMY_MAGIC);
                        buff.mMetaData.mMagicNum_hal &= (~_DUMMY_MAGIC);
                        buff.mMetaData.mMagicNum_tuning &= (~_DUMMY_MAGIC);
                    }
                    //
                    PIPE_DBG("MyDeq: dma(0x%x) pa(0x%x) io(0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x) mag(0x%x_0x%x)\n",portID.index,\
                    buff.mPa,\
                    result.mCrop_s.p.x,result.mCrop_s.p.y,result.mCrop_s.s.w,result.mCrop_s.s.h, \
                    result.mCrop_d.p.x,result.mCrop_d.p.y,result.mCrop_d.s.w,result.mCrop_d.s.h, \
                    result.mDstSize.w,result.mDstSize.h,\
                    result.mMagicNum_hal,result.mMagicNum_tuning);
                }
            }
        }
        else {
            PIPE_ERR("[MyDeQ]tg:0x%x dequeSz is 0,[PorID=%d]\n",_NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0]), rQTSBufInfo.vBufInfo.size(),portID.index);
            return MFALSE;
        }
        //
    }

#if 0// check all element in deque container
    {
        MUINT32 _size;
        BufInfo _buf;
        for(int i=0;i<this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum);i++){
            this->mpDeQueue[_PortMap(this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINT32)&_size,0);
            for(int j=0;j<_size;j++){
                if(MFALSE == this->mpDeQueue[_PortMap(this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_at  ,j,(MUINT32)&_buf))
                    PIPE_INF("deque port:0x%x at 0x%x fail\n",this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i),j);
                PIPE_DBG("Dequeue: Port:0x%x, PA:0x%x, mag:0x%x\n",\
                    _buf.mPortID.index,\
                    _buf.mPa,\
                    _buf.mMetaData.mMagicNum_hal);
            }
        }
    }
    PIPE_DBG("Dequeue:-\n");
#endif
    return ret;
}


MBOOL
NormalPipe_FrmB::_setTgPixelAndDualMode(void)
{
    MUINT32 tg, tg2;
    MUINT32 SIdx, S2Idx;
    MUINT32 pixelCnt, pixelCnt2;

    SIdx  = mpSensorIdx[0];
//    S2Idx = mpSensorIdx[1];
    tg  = _NORMALPIPE_GET_TG_IDX(SIdx);
//    tg2 = _NORMALPIPE_GET_TG_IDX(S2Idx);
    pixelCnt  = _NORMALPIPE_GET_PIX_MODE(SIdx);
//    pixelCnt2 = _NORMALPIPE_GET_PIX_MODE(S2Idx);

    //PIPE_DBG("_setTgPixelAndDualMode+ Sidx=[%d, %d], tg=[%d, %d],PixelMode=[%d,%d]",\
    //         SIdx, S2Idx, tg, tg2, pixelCnt, pixelCnt2);



    if (mpEnabledSensorCnt > 1) {
        PIPE_ERR("Error: NotSupportCfg 2 Sensors");
        return MFALSE;
    }

    /*if (mpEnabledSensorCnt == 1)*/
    {
        MBOOL   enable2PixelTwin = MFALSE;

        if ((tg == CAM_TG_1)) {
            // TG2 not support Twin Mode

            if ( (NSCam::SENSOR_TYPE_RAW  == _NORMALPIPE_GET_SENSORTYPE(SIdx)) ||
                 (NSCam::SENSOR_TYPE_JPEG == _NORMALPIPE_GET_SENSORTYPE(SIdx)) ){
                enable2PixelTwin = _NORMALPIPE_GET_PIX_MODE(SIdx) == TWO_PIXEL_MODE ? 1:0;
                if(enable2PixelTwin){
                    PIPE_ERR("error: 2 pix r not supported\n");
                    return MFALSE;
                }
            }
        }
        else{
            PIPE_ERR("error: unsupported tg :0x%x\n",tg);
            return MFALSE;
        }
        mTwinEnable[tg][0] = (unsigned long)this;
        if (enable2PixelTwin){
            if (0 == SIdx){
                if ((pNormalPipe_FrmB[1]) && (_NORMALPIPE_GET_PIX_MODE(1) == TWO_PIXEL_MODE)){
                    PIPE_ERR("Error: 2 sensors both enable Two Pixel Mode");
                    enable2PixelTwin = MFALSE;
                    return MFALSE;
                }
            }
            if (1 == SIdx){
                if ((pNormalPipe_FrmB[0]) && (_NORMALPIPE_GET_PIX_MODE(0) == TWO_PIXEL_MODE)){
                    PIPE_ERR("Error: 2 sensors both enable Two Pixel Mode");
                    enable2PixelTwin = MFALSE;
                    return MFALSE;
                }
            }
        }
        mbTwoPixelEnable =  enable2PixelTwin;
        if(enable2PixelTwin){
            if(tg <= CAM_TG_2){
                mTwinEnable[tg][1] = ePxlMode_Two_Twin;
               }
        }

        PIPE_DBG("1sensor tg[%d],PixelMode=[%d]", tg, pixelCnt);
    }

    return MTRUE;
}

#define DUMP_SENSOR_INFO()                                 \
do    {                                                    \
        MUINT32 SCnt = 2;                                  \
        _sensorInfo_t  *pSInfo = NULL;                     \
        for (int i = 0; i < mpEnabledSensorCnt; i++) {     \
            pSInfo = &pAllSensorInfo->mSenInfo[i];         \
            PIPE_DBG("[%d]Type=%d, DevId=%d, tg=%d",       \
                     i,                                    \
                     pSInfo->mTypeforMw, pSInfo->mDevId,_NORMALPIPE_GET_TG_IDX( mpSensorIdx[i] ));\
        }\
    } while(0);


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::configPipe(QInitParam const& vInPorts)
{
    //PIPE_DBG("+");
    MBOOL dump = 0;
    MBOOL ret = MTRUE;
    MUINT32 temp, SIdx;
    MUINT32 sensorDevIdx;
    char const* szCallerName = "NormalPipe_FrmB";
    IHalSensor::ConfigParam    sensorConfigParam[4];
    MUINT32                    mpTGToSensorIdx[4];
    EImageFormat               tgi_eImgFmt[4]; //Temp Used
    ISP_QUERY_RST QueryRst;
    MBOOL     _rst = MTRUE;

    PIPE_INF("configPipe+, SenIdx=%d", mpSensorIdx[0]);
    Mutex::Autolock lock(mCfgLock);

    if (mConfigDone) {
        PIPE_ERR("NPipe::configPipe ReEntry, SenIdx=%d", mpSensorIdx[0]);
        return MFALSE;
    }

    ///1 Parameter Check
    if (mpEnabledSensorCnt != vInPorts.mSensorCfg.size()) {
        PIPE_ERR("NPipe::configPipe SensorCfgCnt is not match. (%d,%d)", mpEnabledSensorCnt, vInPorts.mSensorCfg.size());
        return MFALSE;
    }

    if (1 == mpEnabledSensorCnt) {
        if (mpSensorIdx[0] != vInPorts.mSensorCfg.at(0).index) {
            PIPE_ERR("NPipe::configPipe SensorIdx is not match. (%d,%d)", mpSensorIdx[0], vInPorts.mSensorCfg.at(0).index);
            return MFALSE;
        }
    }
    else if (2 == mpEnabledSensorCnt) {
        PIPE_ERR("NPipe::configPipe NOT SUPPORT 2 Sensors");
        return MFALSE;
    }


    ///2 Get Info
    PIPE_TRACE_BEGIN("TAG_SENSOR");
    memset(sensorConfigParam, 0, sizeof(IHalSensor::ConfigParam) * 4);
    memset(mpTGToSensorIdx,   0, sizeof(MUINT32) * 4);

    for (int i = 0; i < mpEnabledSensorCnt; i++)  {

        SIdx = mpSensorIdx[i];

        if (NULL != pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner) {
            PIPE_DBG("[Warning] Overwrite %d-SensorCfg...", SIdx);
        }
        pAllSensorInfo->mSenInfo[SIdx].mConfig = vInPorts.mSensorCfg.at(i);
        pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINTPTR)this;

        sensorConfigParam[i] = vInPorts.mSensorCfg.at(i); //used for sensorCfg
        _NORMALPIPE_GET_TGIFMT(SIdx, tgi_eImgFmt[SIdx]);

    }


    if (!mpHalSensor){
         mpHalSensor = pAllSensorInfo->mSList->createSensor(szCallerName, mpEnabledSensorCnt, mpSensorIdx);
    } else {
        PIPE_ERR("[Error] ConfigPipe ReEntry...");
    }

    if (!mpHalSensor){
        PIPE_ERR("mpHalSensor Fail");
        return MFALSE;
    }


    //20140305: Middleware(Allan) will control the powerOn/Off life cycle.
    //mpHalSensor->powerOn(szCallerName, mpEnabledSensorCnt, mpSensorIdx);
    mpHalSensor->configure(mpEnabledSensorCnt, sensorConfigParam);
    for (int i = 0; i < mpEnabledSensorCnt; i++) {

        SIdx = mpSensorIdx[i];
        //
        if(vInPorts.mRawType){
            PIPE_DBG("Enable sensor test pattern(DevId = %d)\n",pAllSensorInfo->mSenInfo[SIdx].mDevId);
            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId,SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,(MUINTPTR)&vInPorts.mRawType,0,0);
        }
        //
        mpHalSensor->querySensorDynamicInfo(pAllSensorInfo->mSenInfo[SIdx].mDevId, &pAllSensorInfo->mSenInfo[SIdx].mDInfo);
        if (_NORMALPIPE_GET_TG_IDX(SIdx) <= 0){
            PIPE_ERR("Error: mpDynamicInfo.TgInfo <= 0,SIdx(0x%x),mDevId(0x%x),tg_idx(%d)", \
                      SIdx, \
                      pAllSensorInfo->mSenInfo[SIdx].mDevId, \
                      _NORMALPIPE_GET_TG_IDX(SIdx));
            return MFALSE;
        }
        mpTGToSensorIdx[_NORMALPIPE_GET_TG_IDX(SIdx) - 1] = SIdx;
    }
    PIPE_TRACE_END();

    vector<PortInfo const*> vCamIOInPorts;
    vector<PortInfo const*> vCamIOOutPorts;
    PortInfo tgi[NPIPE_MAX_ENABLE_SENSOR_CNT];
    PortInfo imgo;
    PortInfo img2o;
    PortInfo *pOut = NULL;
    MBOOL    enable2PixelTwin = MFALSE;
    MUINT32  tg1SIdx = 0, tg2SIdx = 0, tg = 0;

    tg1SIdx = mpTGToSensorIdx[0];
    tg2SIdx = mpTGToSensorIdx[1];

    if (!mpCamIOPipe)
    {
        PIPE_TRACE_BEGIN("TAG_SETTING");
        //create ICamIOPipe
        MUINT32 sType = _NORMALPIPE_GET_SENSORTYPE(tg1SIdx);
        mpCamIOPipe = ICamIOPipe::createInstance(eDrvScenario_CC, _SensorTypeToPortFormat(sType), (MINT8 const*)mpName);
        if ( MFALSE == mpCamIOPipe->init() ) {
             PIPE_ERR("mpCamIOPipe->init Fail");
             return MFALSE;
        }

        //update cq table num if mw member update burst queue number
        PIPE_DBG("mBurstQNum(%d),mUpdateBQNumDone(%d)",mBurstQNum,mUpdateBQNumDone);
        if(!mUpdateBQNumDone)
        {
            mpCamIOPipe->sendCommand(EPIPECmd_UPDATE_BURST_QUEUE_NUM, mBurstQNum,0,0);
        }
        mUpdateBQNumDone=true;
        //
        //PIPE_DBG("%d-Sensors 's mpCamIOPipe: 0x%x", mpEnabledSensorCnt, mpCamIOPipe);

        //determine 2 pixel Mode or not for Raw
        if (!_setTgPixelAndDualMode()) { //call this before setting tg information
            PIPE_ERR("Error: determine 2PixelInfo fail");
            return MFALSE;
        }


        //
        for (int i = 0; i < mpEnabledSensorCnt;i++)
        {
            //in/out port param
            MUINT32 sensorIdx = mpSensorIdx[i];


            tg = _NORMALPIPE_GET_TG_IDX(sensorIdx);
            tgi[i].index = _NOMRALPIPE_CVT_TGIDX(sensorIdx);
            tgi[i].ePxlMode    = (NSImageio_FrmB::NSIspio_FrmB::EPxlMode)mTwinEnable[tg][1];
            tgi[i].eImgFmt     = tgi_eImgFmt[sensorIdx];
            _NORMALPIPE_GET_TGI_PIX_ID(sensorIdx, tgi[i].eRawPxlID);
            tgi[i].u4ImgWidth  = _NORMALPIPE_GET_SENSORCFG_CROP_W(sensorIdx);
            tgi[i].u4ImgHeight = _NORMALPIPE_GET_SENSORCFG_CROP_H(sensorIdx);
            tgi[i].type        = EPortType_Sensor;
            tgi[i].inout       = EPortDirection_In;
            tgi[i].tgFps       = vInPorts.mSensorCfg.at(i).framerate;
            vCamIOInPorts.push_back(&tgi[i]);
            this->mTgOut_size[_TGMapping(tg)].w = tgi[i].u4ImgWidth;
            this->mTgOut_size[_TGMapping(tg)].h = tgi[i].u4ImgHeight;
            //
            if (1) {

               PIPE_INF("SensorTg=%d, tgIdx=%d, devID=%d, eRawPxlID=%d,PixelMode=%d W/H=[%d, %d]",\
                 tg, tgi[i].index, pAllSensorInfo->mSenInfo[mpSensorIdx[i]].mDevId, tgi[i].eRawPxlID, tgi[i].ePxlMode, tgi[i].u4ImgWidth, tgi[i].u4ImgHeight);
            }

        }

        //PIPE_DBG("vInPorts.mPortInfo.size()=%d", vInPorts.mPortInfo.size());
        for (int i=0; i<vInPorts.mPortInfo.size();i++) {

            //Port info convert
            if ( PORT_IMGO == vInPorts.mPortInfo.at(i).mPortID ) {
                //PIPE_DBG("[%d] PORT_IMGO", i);

                if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0]))){
                   pOut = &imgo;
                   this->mImgoOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                   pOut->index   = vInPorts.mPortInfo.at(i).mPortID.index;
                }

                this->mOpenedPort |= NormalPipe_FrmB::_IMGO_ENABLE;
            }
            else if ( PORT_IMG2O == vInPorts.mPortInfo.at(i).mPortID ) {

                if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0]))){
                    pOut = &img2o;
                    this->mImg2oOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                    PIPE_DBG("configPipe PORT_IMG2O, mImg2oOut_size:%d,%d, Crop:%d,%d,%d,%d", \
                        this->mImg2oOut_size.w, this->mImg2oOut_size.h,\
                        vInPorts.mPortInfo.at(i).mCropRect.p.x,vInPorts.mPortInfo.at(i).mCropRect.p.y,\
                        vInPorts.mPortInfo.at(i).mCropRect.s.w,vInPorts.mPortInfo.at(i).mCropRect.s.h);
                    pOut->index    = vInPorts.mPortInfo.at(i).mPortID.index;
                }

                this->mOpenedPort |= NormalPipe_FrmB::_IMG2O_ENABLE;
            }

            if((this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum) > MaxPortIdx) || (this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum) == 0) ){
                PIPE_ERR("dmao config error,opend port num:0x%x(max:0x%x)\n",this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum),MaxPortIdx);
                return MFALSE;
            }

            //
            pOut->eImgFmt        = vInPorts.mPortInfo.at(i).mFmt;
            pOut->u4PureRaw      = vInPorts.mPortInfo.at(i).mPureRaw;
            pOut->u4PureRawPak   = vInPorts.mPortInfo.at(i).mPureRawPak;
            pOut->u4ImgWidth     = vInPorts.mPortInfo.at(i).mDstSize.w;
            //add for dmao x-size query in pipeimp
            pOut->ePxlMode    = (this->mbTwoPixelEnable == 1) ? (NSImageio_FrmB::NSIspio_FrmB::ePxlMode_Two_Twin):(NSImageio_FrmB::NSIspio_FrmB::ePxlMode_One_Single);

            if (pOut->index  == EPortIndex_IMG2O) {
                MSize _tgsize = this->mTgOut_size[_TGMapping(_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[0])))];
                if(ISP_QuerySize(pOut->index,ISP_QUERY_X_PIX,pOut->eImgFmt,pOut->u4ImgWidth,QueryRst,((this->mbTwoPixelEnable == 1)?(ISP_QUERY_2_PIX_MODE):(ISP_QUERY_1_PIX_MODE))) == 0){
                    PIPE_ERR(" fmt(0x%x) | dma(0x%x) err\n",pOut->eImgFmt,pOut->index);
                    return MFALSE;
                }else {
                    if(QueryRst.x_pix != pOut->u4ImgWidth) {
                        PIPE_ERR(" port:0x%x width r in-valid (cur:%d   valid:%d)\n",pOut->index,pOut->u4ImgWidth,QueryRst.x_pix);
                        return MFALSE;
                    }
                }
                //
                //because cropping coordinate can't be based on TG coordinate. so , not supported.
                if( (vInPorts.mPortInfo.at(i).mCropRect.p.x != 0) || (vInPorts.mPortInfo.at(i).mCropRect.p.y != 0) || \
                    (vInPorts.mPortInfo.at(i).mCropRect.s.w != _tgsize.w) || \
                    (vInPorts.mPortInfo.at(i).mCropRect.s.h != _tgsize.h) ){
                    PIPE_ERR("error: dma cropping r not supported in IMG2O\n");
                    return MFALSE;
                }
                //PIPE_DBG("NormalPipe_FrmB: PORT_IMG2O u4ImgWidth=%d", pOut->u4ImgWidth);
            }
            if(pOut->index  == EPortIndex_IMGO){
                E_ISP_QUERY _op;
                if(pOut->u4ImgWidth != this->mTgOut_size[_TGMapping(tg)].w)
                    _op = ISP_QUERY_CROP_X_PIX;
                else
                    _op = ISP_QUERY_X_PIX;

                if(ISP_QuerySize(pOut->index,_op,pOut->eImgFmt,pOut->u4ImgWidth,QueryRst,((this->mbTwoPixelEnable == 1)?(ISP_QUERY_2_PIX_MODE):(ISP_QUERY_1_PIX_MODE))) == 0){
                    PIPE_ERR(" fmt(0x%x) | dma(0x%x) err\n",pOut->eImgFmt,pOut->index);
                    return MFALSE;
                }else {
                    if(QueryRst.x_pix != pOut->u4ImgWidth) {
                        PIPE_ERR(" port:0x%x width r in-valid (cur:%d   valid:%d)\n",pOut->index,pOut->u4ImgWidth,QueryRst.x_pix);
                        return MFALSE;
                    }
                }
            }

            pOut->u4ImgHeight     = vInPorts.mPortInfo.at(i).mDstSize.h;
            pOut->crop1.x         = vInPorts.mPortInfo.at(i).mCropRect.p.x;
            pOut->crop1.y         = vInPorts.mPortInfo.at(i).mCropRect.p.y;
            pOut->crop1.floatX    = 0;
            pOut->crop1.floatY    = 0;
            pOut->crop1.w         = vInPorts.mPortInfo.at(i).mCropRect.s.w;
            pOut->crop1.h         = vInPorts.mPortInfo.at(i).mCropRect.s.h;
            //pOut->u4Stride[ESTRIDE_1ST_PLANE] = queryRawStride((MUINT32)pOut->eImgFmt, pOut->u4ImgWidth);
            pOut->u4Stride[ESTRIDE_1ST_PLANE] = vInPorts.mPortInfo.at(i).mStride[0];
            if(ISP_QuerySize(pOut->index,ISP_QUERY_STRIDE_BYTE,pOut->eImgFmt,pOut->u4ImgWidth,QueryRst) == 0){
                PIPE_ERR(" fmt(0x%x) | dma(0x%x) err\n",pOut->eImgFmt,pOut->index);
                return MFALSE;
            }else {
                if(QueryRst.stride_byte != pOut->u4Stride[ESTRIDE_1ST_PLANE]){
                    PIPE_DBG("cur dma(%d) stride:0x%x, recommanded:0x%x",pOut->index,pOut->u4Stride[ESTRIDE_1ST_PLANE],QueryRst.stride_byte);
                }
            }


            pOut->u4Stride[ESTRIDE_2ND_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
            pOut->u4Stride[ESTRIDE_3RD_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
            pOut->type   = EPortType_Memory;
            pOut->inout  = EPortDirection_Out;

            if (1) {
                 PIPE_DBG("DMAO[%d] Crop=[%d,%d,%d,%d], sz=(%d,%d), Stride=%d",\
                  pOut->index, pOut->crop1.x, pOut->crop1.y, pOut->crop1.w, pOut->crop1.h, \
                  pOut->u4ImgWidth, pOut->u4ImgHeight,pOut->u4Stride[0]);
            }

            //
            if ((NSCam::SENSOR_TYPE_YUV == _NORMALPIPE_GET_SENSORTYPE(this->mpSensorIdx[0])) && (pOut->index == EPortIndex_IMG2O))
            {
                //IMG2O doesn't support YUV sensor
                PIPE_ERR("error: img2o support no YUV fmt\n");
                return MFALSE;
            }
            else
            {
                vCamIOOutPorts.push_back(pOut);
            }
        }


        //DUMP_SENSOR_INFO();
        //CQ
        for (int i = 0; i < this->mpEnabledSensorCnt; i++)
        {
            switch ( _NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[i])) )
            {
                case CAM_TG_1:
                    mEnablePath |= NPIPE_PASS_PASS1;
                    this->mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL     ,(MINT32)EPIPE_PASS1_CQ0,0,0);
                    this->mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                    PIPE_DBG("CAM_TG_1: SetCQ0_Channel&Mode");
                break;
                case CAM_TG_2:
                case CAM_SV_1:
                case CAM_SV_2:
                default:
                    PIPE_ERR("error: unsupported TG:0x%x\n",_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[i])));
                    return MFALSE;
                break;
            }
        }

        //config pipe
        if ( MFALSE == this->mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts) ) {
            PIPE_ERR("mpCamIOPipe->configPipe Fail");
            return MFALSE;
        }
        //PIPE_DBG("================After mpCamIOPipe->configPipe ================");
        PIPE_TRACE_END();
        PIPE_TRACE_BEGIN("TAG_WAIT_V");
        for (int i = 0; i < mpEnabledSensorCnt; i++)
        {
            switch ( _NORMALPIPE_GET_TG_IDX((mpSensorIdx[i])) )
            {
                case CAM_TG_1:
                    this->mpCamIOPipe->startCQ0();
                    //PIPE_DBG("mpCamIOPipe->startCQ0 ");
#if PASS1_CQ_CONTINUOUS_MODE__
                    //continuous mode
                    this->mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
#else
                    //cause CQ0 was configurred as SINGLE_IMMEDIATELY
                    this->mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                    this->mpCamIOPipe->startCQ0();//for hrz configframe before start
#endif
                    PIPE_DBG("[%d]irq(CAM_TG_1)(EPIPEIRQ_VSYNC) ", i);
                    //if(MFALSE == this->mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC, (int)ISP_DRV_IRQ_USER_ISPDRV))
                    //performance issue: change to non clear wait
                    if(MFALSE == this->mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC, (int)ISP_DRV_IRQ_CLEAR_NONE, (int)ISP_DRV_IRQ_USER_ISPDRV))
                    {
                        PIPE_ERR("after cfg, wait VSYNC fail");
                        return MFALSE;
                    }

                    break;
                case CAM_TG_2:
                default:
                    PIPE_ERR("error: unsupported TG:0x%x\n",_NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[i])));
                    return MFALSE;
                break;
            }

        }
        PIPE_TRACE_END();

        if( (this->mOpenedPort & NormalPipe_FrmB::_IMGO_ENABLE) == 0 )
            this->mpCamIOPipe->sendCommand(EPIPECmd_SET_IMGOBYPASS,MTRUE,0,0);
        if( (this->mOpenedPort & NormalPipe_FrmB::_IMG2O_ENABLE) == 0 )
            this->mpCamIOPipe->sendCommand(EPIPECmd_SET_IMG2OBYPASS,MTRUE,0,0);
        if(this->mOpenedPort== 0 ){
            PIPE_ERR("error:no dmao enabled\n");
            return MFALSE;
        }
        this->m_DropCB = vInPorts.m_DropCB;
        this->m_returnCookie= vInPorts.m_returnCookie;
        this->m_PerFrameCtl = vInPorts.m_PerFrameCtl;
        PIPE_DBG("DropCB(0x%x)/Cookie(0x%x)/PerFrameCtl(%d)", this->m_DropCB,this->m_returnCookie, vInPorts.m_PerFrameCtl);
    }
    else {
        PIPE_ERR("CamioPipe is not NULL for 1st cfg.");
    }


    this->mConfigDone = MTRUE;
    PIPE_INF("-");
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
NormalPipe_FrmB::attach(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType)
{
#if 0
    //PIPE_DBG("NormalPipe_FrmB::attach +");
    //FIXMEE
    Irq_t irq(0,0,0,0,(MINT8*)pConsumer->getName(),0);

    irq.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
    //irq.Status = EPipeSignal_EOF?CAM_CTL_INT_P1_STATUS_PASS1_DON_ST:CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    irq.Timeout = 30000; //ms
    //irq.UserName
    //irq.irq_TStamp


    if (eType == EPipeSignal_SOF)
    {
        irq.Status = CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;
    }
    else if (eType == EPipeSignal_EOF)
    {
        irq.Status = CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    }

    if (mpCamIOPipe)
    {
        //
        if ( MFALSE == mpCamIOPipe->registerIrq(irq) )
        {
            PIPE_ERR("registerIrq(%d,%d,%d,%s,%d,)",irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
        }
    }
#else
    PIPE_ERR("error:unsupported api\n");
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe_FrmB::wait(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout)
{
    //return immediately if have received signal alread; otherwise, wait right now.

    PIPE_DBG("wait+");

    bool ret = MTRUE;
#if 0    //not supported
    Irq_t irq(0,0,0,0,(MINT8*)pConsumer->getName(),0);

    irq.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
    //irq.Status = EPipeSignal_EOF?CAM_CTL_INT_P1_STATUS_PASS1_DON_ST:CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    irq.Timeout = 30000; //ms
    //irq.UserName
    //irq.irq_TStamp


    if (eType == EPipeSignal_SOF)
    {
        irq.Status = CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;
    }
    else if (eType == EPipeSignal_EOF)
    {
        irq.Status = CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    }

    if ( ! mpCamIOPipe)
    {
        //
        if ( MFALSE == mpCamIOPipe->irq(irq) )
        {
            PIPE_ERR("irq(%d,%d,%d,%s,%d,)",irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
            ret = false;
        }
    }
    PIPE_DBG("wait-");
#else
    ret = MFALSE;
#endif
    return ret;

}



MBOOL
NormalPipe_FrmB::wait(MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout)
{
    PIPE_DBG("wait+(0x%x)\n",eType);
    EPipeIRQ _irq;
    bool ret = MTRUE;
    MUINT32 TG = _NORMALPIPE_GET_TG_IDX(sensorIdx);
    switch(TG)
    {
        case CAM_TG_1:                TG = EPipePass_PASS1_TG1;
            break;
        case CAM_TG_2:
        case CAM_SV_1:
        case CAM_SV_2:
        default:
            PIPE_ERR("TG idx error(0x%x,0x%x)\n",this->mpSensorIdx[0],sensorIdx);
            return MFALSE;
            break;
    }
    switch(eType)
    {
        case EPipeSignal_SOF:
            _irq = EPIPEIRQ_SOF;
            break;
        case EPipeSignal_EOF:
            _irq = EPIPEIRQ_PATH_DONE;
            break;
        default:
            PIPE_ERR("signal error(0x%x)\n",_irq);
            return MFALSE;
            break;
    }

    if (this->mpCamIOPipe)
    {
        if ( MFALSE == this->mpCamIOPipe->irq((NSImageio_FrmB::NSIspio_FrmB::EPipePass)TG, _irq,(int)ISP_DRV_IRQ_USER_ISPDRV))
        {
            PIPE_ERR("irq(0x%x,0x%x)\n",TG,_irq);
            ret = MFALSE;
        }
    }
    else{
        ret = MFALSE;
        PIPE_ERR("camiopipe obj isn't exist(0x%x)\n",this);
    }
    PIPE_DBG("wait-\n");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MUINT32
NormalPipe_FrmB::getIspReg(::ESoftwareScenario scen)
{
    PIPE_DBG("getIspReg+");
    if ( ! mpCamIOPipe )
    {
        return 0;
    }
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/

MBOOL
NormalPipe_FrmB::
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int  ret = MTRUE;
    MUINT32 tg;
    NormalPipe_HBIN_Info *hbin = NULL;
    NormalPipe_EIS_Info *pEis = NULL;
    MINT32 sensorDev = 1;
    //PIPE_DBG("sendCommand+, SenIdx=%d", mpSensorIdx[0]);
    if ( !this->mConfigDone ) {
        PIPE_ERR("error:sendCommand(0x%x) fail, send before configpipe(0x%x,0x%x)",cmd,this,this->mConfigDone);
        return MFALSE;
    }

    switch (cmd) {
    case EPIPECmd_SET_EIS_CBFP:
        ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, arg3);
        break;
    case EPIPECmd_GET_MODULE_HANDLE: //handle-based
        //arg1: [In]   enum EModule
        //arg2: [Out] (MUINT32)Module_Handle
        //arg3: user name string
    case EPIPECmd_RELEASE_MODULE_HANDLE: //handle-based
        //arg1: [In]   Handle
        //arg2: user name string
    case EPIPECmd_SET_MODULE_CFG_DONE:  //handle-based
        //arg1: [In]   enum EModule
    case EPIPECmd_SET_MODULE_EN:
        //arg1 : MODULE-Name;
        //arg2 : MBOOL Enable(TRUE)/Disable(FALSE)
    case EPIPECmd_SET_MODULE_SEL:
        //arg1 : MODULE-Name;
        //arg2 : MBOOL Select(TRUE)/DeSel(FALSE)
    case EPIPECmd_SET_MODULE_DBG_DUMP: //dbg only
        //arg1 : handle;
        if ( !mpCamIOPipe ) {
            PIPE_ERR("[Error]mpCamIOPipe=NULL");
            return 0;
        }
        ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, arg3);
        break;

    case EPIPECmd_GET_TG_OUT_SIZE:
        //arg1 : sensor-idx
        //arg2 : width
        //arg3 : height
        if (2 < arg1) {
            PIPE_ERR("EPIPECmd_GET_TG_OUT_SIZE Err: Invalid SensorIdx(%d)!",arg1);
            return MFALSE;
        }
        *((MINT32*)arg2) =_NORMALPIPE_GET_SENSORCFG_CROP_W(arg1);
        *((MINT32*)arg3) =_NORMALPIPE_GET_SENSORCFG_CROP_H(arg1);
        //PIPE_DBG("EPIPECmd_GET_TG_OUT_SIZE SIdx(%d), w=0x%x, h=0x%x!",arg1, *((MINT32*)arg2), *((MINT32*)arg3));
        break;

    case EPIPECmd_GET_HBIN_INFO:
        //arg1 : sensor-idx
        //arg2 : NormalPipe_HBIN_Info
        if ((2 < arg1) || (NULL == arg2)) {
            PIPE_ERR("EPIPECmd_GET_HBIN_INFO Err: Invalid Para arg1(%d), arg2=0x%x!",arg1, arg2);
            return MFALSE;
        }
        tg = _NORMALPIPE_GET_TG_IDX(arg1);
        PIPE_DBG("EPIPECmd_GET_HBIN_INFO SIdx(%d), BinInfoAddr=0x%x",arg1, arg2);

        hbin = (NormalPipe_HBIN_Info*) arg2;
        switch (tg) {
            case CAM_TG_1:
                if (mbTwoPixelEnable) {
                //HBin will be enable, size downsampe 1/2 for 3A
                hbin->mEnabled = MTRUE;
                //hbin->size.w = mImgoOut_size.w >> 1;
                //hbin->size.h = mImgoOut_size.h >> 1;
                hbin->size.w = mTgOut_size[_TGMapping(tg)].w>>1;
                hbin->size.h = mTgOut_size[_TGMapping(tg)].h;
                } else {
                hbin->mEnabled = MFALSE;
                hbin->size.w = mTgOut_size[_TGMapping(tg)].w;
                hbin->size.h = mTgOut_size[_TGMapping(tg)].h;
                }
                break;
            case CAM_TG_2:
                default:
                PIPE_ERR("EPIPECmd_GET_HBIN_INFO Err: Invalid SensorIdx(%d)!",arg1);
                return MFALSE;
        }
        PIPE_DBG("EPIPECmd_GET_HBIN_INFO SIdx(%d),twin=%d,hbin_en=%d w=0x%x, h=0x%x!",arg1,mbTwoPixelEnable, hbin->mEnabled,hbin->size.w, hbin->size.h);
        break;
    case EPIPECmd_GET_EIS_INFO:
        //arg1 : sensor-idx
        //arg2 : NormalPipe_EIS_Info*
        if (NULL == arg2) {
            PIPE_ERR("EPIPECmd_GET_EIS_INFO Err: Invalid Para arg1(%d)",arg1);
            return MFALSE;
        }
        pEis = (NormalPipe_EIS_Info*) arg2;
        tg = _NORMALPIPE_GET_TG_IDX(arg1);
        switch (tg) {
            case CAM_TG_1:
                pEis->mSupported = MTRUE;
                break;
            default:
                pEis->mSupported = MFALSE;
                break;
        }

        break;
    case EPIPECmd_GET_TWIN_INFO:
        *((MINT32*)arg1) = mbTwoPixelEnable;
    break;
    case EPIPECmd_SET_P1_UPDATE:
    case EPIPECmd_SET_HRZ:
    case EPIPECmd_SET_IMGO:
        PIPE_ERR("error:unsupported cmd(0x%x) in Frame-based opeartion\n",cmd);
        return MFALSE;
        break;
    case EPIPECmd_GET_RMX_OUT_SIZE:
    case EPIPECmd_SET_LCS_CBFP:
        PIPE_ERR("error: unsupported cmd(0x%x)\n",cmd);
        return MFALSE;
        break;
    break;
    case EPIPECmd_GET_CUR_FRM_STATUS:
        ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, arg3);
        //if 1st enq loop does NOT apply patch, MW cannot enque at this timing
        if(MFALSE == m_b1stEnqLoopDone)
            *(MUINT32*)arg1 = _1st_enqloop_status;
        break;
    default:
        if ( !mpCamIOPipe ) {
            PIPE_ERR("[Error]mpCamIOPipe=NULL");
            return 0;
        }
        ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, arg3);
        break;
    }
    if(ret != MTRUE){
        PIPE_ERR("error: sendCommand fail: (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", cmd, arg1, arg2, arg3);
    }
    //PIPE_DBG("sendCommand-");
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
//arg1 : only valid in cmd:_GetPort_Index
MUINT32
NormalPipe_FrmB::GetOpenedPortInfo(NormalPipe_FrmB::ePortInfoCmd cmd,MUINT32 arg1)
{
    MUINT32 _num=0;
    switch(cmd){
        case NormalPipe_FrmB::_GetPort_Opened:
            return (MUINT32)this->mOpenedPort;
            break;
        case NormalPipe_FrmB::_GetPort_OpendNum:
            if(this->mOpenedPort& NormalPipe_FrmB::_IMGO_ENABLE)
                _num++;
            if(this->mOpenedPort& NormalPipe_FrmB::_IMG2O_ENABLE)
                _num++;
            return _num;
            break;
        case NormalPipe_FrmB::_GetPort_Index:
            if((this->mOpenedPort & NormalPipe_FrmB::_IMGO_ENABLE) && (this->mOpenedPort & NormalPipe_FrmB::_IMG2O_ENABLE) ){
                switch(_NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0])){
                    case CAM_TG_1:
                        this->PortIndex[_PortMap(PORT_IMG2O.index)] = PORT_IMG2O.index;
                        this->PortIndex[_PortMap(PORT_IMGO.index)] = PORT_IMGO.index;
                        break;
                    default:
                        PIPE_ERR("error: tg err:0x%x",_NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0]));
                        break;
                }
            }
            else if(this->mOpenedPort& NormalPipe_FrmB::_IMG2O_ENABLE){
                switch(_NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0])){
                    case CAM_TG_1:
                        this->PortIndex[0] = PORT_IMG2O.index;
                        break;
                    default:
                        PIPE_ERR("error: tg err:0x%x",_NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0]));
                        break;
                }
            }
            else if(this->mOpenedPort& NormalPipe_FrmB::_IMGO_ENABLE){
                switch(_NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0])){
                    case CAM_TG_1:
                        this->PortIndex[0] = PORT_IMGO.index;
                        break;
                    default:
                        PIPE_ERR("error: tg err:0x%x",_NORMALPIPE_GET_TG_IDX(this->mpSensorIdx[0]));
                        break;
                }
            }
            else{
                PIPE_ERR("no dmao r opened\n");
            }
            return this->PortIndex[arg1];
            break;
        default:
            PIPE_ERR("un-supported cmd:0x%x\n",cmd);
            return 0;
            break;
    }
}
MUINT32 NormalPipe_FrmB::GetRingBufferDepth(void)
{
    return (this->mRingBufDepth / this->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum));
}

#ifdef USE_IMAGEBUF_HEAP
/******************************************************************************
 *
 ******************************************************************************/
void
NormalPipe_FrmB::
FrameMgr::
init()
{
    mvFrameQ.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
void
NormalPipe_FrmB::
FrameMgr::
uninit()
{
    mvFrameQ.clear();
}



/******************************************************************************
 *
 ******************************************************************************/
void
NormalPipe_FrmB::
FrameMgr::
enque(IImageBuffer *pframe)
{
    Mutex::Autolock lock(mFrameMgrLock);
    PIPE_DBG("FrameMgr::enque+, 0x%x", pframe->getBufPA(0));
    mvFrameQ.add(pframe->getBufPA(0), pframe);//change from VA 2 PA is because of in camera3, drv may have no VA
  //PIPE_DBG("FrameMgr::enque -");
}

/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
NormalPipe_FrmB::
FrameMgr::
deque(int key) //key == va
{
    Mutex::Autolock lock(mFrameMgrLock);
    PIPE_DBG("FrameMgr::deque+ key=0x%x",key);
    IImageBuffer *pframe = mvFrameQ.valueFor(key);
    // exist
    if (pframe != NULL)
    {
        mvFrameQ.removeItem(key);
        //PIPE_DBG("FrameMgr::deque 0x%x-", key);
        return pframe;
    }
    //PIPE_DBG("FrameMgr::deque- (NULL)");
    return NULL;
}


#else

void
NormalPipe_FrmB::
FrameMgr::
enque(const BufInfo& pframe)
{
}


NSCam::NSIoPipe::NSCamIOPipe::BufInfo*
NormalPipe_FrmB::
FrameMgr::
deque(int key) //key == va
{
    return NULL;
}
#endif
/****************************************************
*
*****************************************************/
void
NormalPipe_FrmB:: QueueMgr:: init(void)
{
    this->m_v_pQueue.clear();
}

void
NormalPipe_FrmB:: QueueMgr:: uninit(void)
{
    this->m_v_pQueue.clear();
}

MBOOL
NormalPipe_FrmB:: QueueMgr:: sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mQueueLock);
    switch(cmd){
        case eCmd_getsize:
            *(MUINTPTR*)arg1 = this->m_v_pQueue.size();
            break;
        case eCmd_push:
            this->m_v_pQueue.push_back(*(BufInfo*)(arg1));
            break;
        case eCmd_pop:
            this->m_v_pQueue.pop_front();
            break;
        case eCmd_front:
            *(BufInfo*)arg1 = this->m_v_pQueue.front();
            {
                BufInfo buf;
                buf = this->m_v_pQueue.front();
            }
            break;
        case eCmd_at:
            if(this->m_v_pQueue.size()>arg1)
                *(BufInfo*)arg2 = this->m_v_pQueue.at(arg1);
            else{
                PIPE_INF("size have been reduce to 0x%x(0x%x) by enque\n",this->m_v_pQueue.size(),arg1);
                ret = MFALSE;
            }
            break;
        case eCmd_pop_back:
            this->m_v_pQueue.pop_back();
            break;
        case eCmd_end:
        default:
            PIPE_ERR("error: unsopported cmd:0x%x\n",cmd);
            ret = MFALSE;
            break;
    }
    return ret;
}

/*****************************************************
*
*****************************************************/
NormalPipe_FrmB_Thread::NormalPipe_FrmB_Thread(NormalPipe_FrmB* obj)
{
    m_bStart = MFALSE;
    mCnt = 0;
    m_pNormalpipe_FrmB = (NormalPipe_FrmB* )obj;
    m_EnqueCnt = 0;
}

NormalPipe_FrmB_Thread* NormalPipe_FrmB_Thread::CreateInstance(NormalPipe_FrmB* obj)
{
    if(obj == 0)
    {
        PIPE_ERR("create instance fail(0x%x_0x%x)",obj);
        return NULL;
    }

    return new NormalPipe_FrmB_Thread(obj);
}

MBOOL NormalPipe_FrmB_Thread::DestroyInstance(void)
{
    delete this;
    return MTRUE;
}

MBOOL NormalPipe_FrmB_Thread::init(void)
{
    MBOOL ret = MTRUE;
    PIPE_INF("mInitCount(%d) ", this->mCnt);
    //
    if(this->mCnt > 0)
    {
        android_atomic_inc((volatile MINT32*)&this->mCnt);
        return ret;
    }

    // Init semphore
    ::sem_init(&this->m_semEnqueThread, 0, 0);

    // Create main thread for preview and capture
    pthread_create(&this->m_EnqueThread, NULL, this->IspEnqueThread, this);

    //
    android_atomic_inc((volatile MINT32*)&this->mCnt);
    return ret;
}

MBOOL NormalPipe_FrmB_Thread::uninit(void)
{
    PIPE_INF("mInitCount(%d) ", this->mCnt);
    //
    if(this->mCnt <= 0)
    {
        // No more users
        return MTRUE;
    }
    // More than one user
    android_atomic_dec((volatile MINT32*)&this->mCnt);
    //
    if(this->mCnt > 0)
    {
        return MTRUE;
    }

    return MTRUE;
}


MBOOL NormalPipe_FrmB_Thread::Start(void)
{
    PIPE_INF("E:");

    this->mLock.lock();
    this->m_bStart = MTRUE;
    this->mLock.unlock();

    ::sem_post(&this->m_semEnqueThread);
    return MTRUE;
}

MBOOL NormalPipe_FrmB_Thread::PreStop(void)
{
    this->mLock.lock();
    this->m_bStart = MFALSE;
    this->mLock.unlock();

    return MTRUE;
}

MBOOL NormalPipe_FrmB_Thread::Stop(void)
{
    PIPE_TRACE_CALL();
    PIPE_DBG("Thread E:");
//    this->m_bStart = MFALSE;
    //
    pthread_join(this->m_EnqueThread, NULL);
    ::sem_wait(&this->m_semEnqueThread); // wait until thread prepare to leave
    PIPE_DBG("Thread X:");
    return MTRUE;
}

/**
    this enque run in immediate mode,enque 1 port with 1 buffer at 1 time.
*/
MBOOL NormalPipe_FrmB_Thread::Enque(BufInfo* pBuf)
{
    bool ret = MTRUE;
    NSImageio_FrmB::NSIspio_FrmB::PortID portID;
    NSImageio_FrmB::NSIspio_FrmB::QBufInfo rQBufInfo;
    BufInfo    _buf;
    //
    //PIPE_DBG("NormalPipe_FrmB::enque +");


    //image buffer data structure pool for MW
    //PIPE_DBG("before mpFrameMgr->enque");
    //
    portID.index = pBuf->mPortID.index;

#ifdef USE_IMAGEBUF_HEAP
    this->m_pNormalpipe_FrmB->mpFrameMgr->enque(pBuf->mBuffer);
#endif

    if( pBuf->mBufIdx != 0xFFFF){
        PIPE_ERR("error:replace buffer with the same address r not supported in v3, plz make sure mBufIdx=0xffff\n");
        return MFALSE;
    }


    //search Dequeue by portID,pop up oldest element in deque container
    this->m_pNormalpipe_FrmB->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front,(MUINTPTR)&_buf,0);
    if(_buf.mPa == pBuf->mBuffer->getBufPA(0)){

        //enque without exchange
        //
        rQBufInfo.vBufInfo.resize(1);
        //rQBufInfo.vBufInfo[0].memID =       (MINT32)(imgo_memId[i]); //
#ifdef USE_IMAGEBUF_HEAP
        rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)pBuf->mBuffer->getBufSizeInBytes(0); //bytes
        rQBufInfo.vBufInfo[0].u4BufVA[0] =       (MUINT32)pBuf->mBuffer->getBufVA(0); //
        rQBufInfo.vBufInfo[0].u4BufPA[0] =       (MUINT32)pBuf->mBuffer->getBufPA(0); //
#else
        rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)pBuf->mSize;
        rQBufInfo.vBufInfo[0].u4BufVA[0] =       (MUINT32)pBuf->mVa;
        rQBufInfo.vBufInfo[0].u4BufPA[0] =       (MUINT32)pBuf->mPa;
#endif
        rQBufInfo.vBufInfo[0].header  =         NULL;//pBuf->mMetaData.mPrivateData;
        rQBufInfo.vBufInfo[0].mBufIdx =         pBuf->mBufIdx; //used when replace buffer
    }
    else{

        //exchange old dequed buffer
        //
        rQBufInfo.vBufInfo.resize(2);
        //rQBufInfo.vBufInfo[0].memID =       (MINT32)(imgo_memId[i]); //
#ifdef USE_IMAGEBUF_HEAP
        //mark, because of imagebuffer will be unlock by MW, drv can't access imagebuffer here
        //rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)_buf.mBuffer->getBufSizeInBytes(0); //bytes
        //rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINT32)_buf.mBuffer->getBufVA(0); //
        //rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINT32)_buf.mBuffer->getBufPA(0); //
        rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)_buf.mSize;
        rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINT32)_buf.mVa;
        rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINT32)_buf.mPa;
#else
        rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)_buf.mSize;
        rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINT32)_buf.mVa;
        rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINT32)_buf.mPa;
#endif
        rQBufInfo.vBufInfo[0].header  =         NULL;//_buf.mMetaData.mPrivateData;
        rQBufInfo.vBufInfo[0].mBufIdx =         _buf.mBufIdx; //used when replace buffer

#ifdef USE_IMAGEBUF_HEAP
        rQBufInfo.vBufInfo[1].u4BufSize[0] =   (MUINT32)pBuf->mBuffer->getBufSizeInBytes(0); //bytes
        rQBufInfo.vBufInfo[1].u4BufVA[0] =     (MUINT32)pBuf->mBuffer->getBufVA(0); //
        rQBufInfo.vBufInfo[1].u4BufPA[0] =     (MUINT32)pBuf->mBuffer->getBufPA(0); //
#else
        rQBufInfo.vBufInfo[1].u4BufSize[0] =   (MUINT32)pBuf->mSize;
        rQBufInfo.vBufInfo[1].u4BufVA[0] =     (MUINT32)pBuf->mVa;
        rQBufInfo.vBufInfo[1].u4BufPA[0] =     (MUINT32)pBuf->mPa;
#endif
        rQBufInfo.vBufInfo[1].header  =         NULL;//pBuf->mMetaData.mPrivateData;
        rQBufInfo.vBufInfo[1].mBufIdx =         pBuf->mBufIdx; //used when replace buffer
    }

    if(rQBufInfo.vBufInfo.size() == 1)
        PIPE_DBG("Enque port:0x%x PA:0x%x,mag:0x%x\n",portID.index,rQBufInfo.vBufInfo[0].u4BufPA[0],pBuf->FrameBased.mMagicNum_tuning);
    else
        PIPE_DBG("Enque port:0x%x PA:0x%x,exchange:0x%x, mag:0x%x\n",portID.index,rQBufInfo.vBufInfo[0].u4BufPA[0],rQBufInfo.vBufInfo[1].u4BufPA[0],pBuf->FrameBased.mMagicNum_tuning);

    //
    if (this->m_pNormalpipe_FrmB->mpCamIOPipe)  {
        //assign 1 for enqueCq is for enabe Immediate enque mode
        if ( MFALSE == this->m_pNormalpipe_FrmB->mpCamIOPipe->enqueOutBuf(portID, rQBufInfo ,1) ) {
            PIPE_ERR("error: enqueOutBuf fail");
            return MFALSE;
        }
    }
    //pop out deque container
    this->m_pNormalpipe_FrmB->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop,0,0);
    //
    //PIPE_DBG("NormalPipe_FrmB::enque -");
    return ret;
}


MBOOL NormalPipe_FrmB_Thread::DmaCfg(BufInfo* pParam)
{
    //PIPE_DBG("configFrame+: param size: (%d)", prQParam->mvOut.size());
    bool ret = MTRUE;
    MINT32 i = 0;
    DMACfg _dmao;
    NSImageio_FrmB::NSIspio_FrmB::EPortIndex tgIdx;
    NSCam::NSIoPipe::PortID port;
    MUINT32 _cmd = EPIPECmd_SET_HRZ;
    //PIPE_DBG("size(%d),mbTwoPixelEnable(%d),mpEnabledSensorCnt(%d),", \
    //          prQParam->mvOut.size(),\
    //          mbTwoPixelEnable, \
    //          mpEnabledSensorCnt);

    //

    port = pParam->mPortID ;

    if(( PORT_IMG2O == port ) || (PORT_IMGO == port))
    {}
    else{
        PIPE_ERR("error: this port(0x%x_0x%x) r not supported in DmaCfg()\n",_NORMALPIPE_GET_TG_IDX((this->m_pNormalpipe_FrmB->mpSensorIdx[0])),port.index);
        return MFALSE;
    }

    //PIPE_DBG("configFrame: idx =%d, port=%d", i, port.index);
    _dmao.crop.x        = pParam->FrameBased.mCropRect.p.x;
    _dmao.crop.y        = pParam->FrameBased.mCropRect.p.y;
    _dmao.crop.floatX = 0;
    _dmao.crop.floatY = 0;
    if( PORT_IMG2O == port ){
        _dmao.crop.w = pParam->FrameBased.mDstSize.w;
        _dmao.crop.h = pParam->FrameBased.mDstSize.h;
    }
    else{
        _dmao.crop.w = pParam->FrameBased.mCropRect.s.w;
        _dmao.crop.h = pParam->FrameBased.mCropRect.s.h;
    }
    //
    _dmao.out.w        = pParam->FrameBased.mDstSize.w;
    _dmao.out.h        = pParam->FrameBased.mDstSize.h;
    _dmao.out.stride    = _dmao.out.w;
    _dmao.m_num        = pParam->FrameBased.mMagicNum_tuning;
    //
#if 1 //js_test pdaf remove after p1node complete implemetation
    _dmao.mImgSel = eRawFmt_Processed; /*!<default processed raw*/
#else
    switch( prQParam->mRawOutFmt ) {
        case EPipe_PURE_RAW:
            _dmao.mImgSel = eRawFmt_Pure;
            break;
        case EPipe_PROCESSED_RAW:
        default:
            _dmao.mImgSel = eRawFmt_Processed;
            break;
    }
#endif

    if (this->m_pNormalpipe_FrmB->mpCamIOPipe) {

        //Arg1: HRZCfg
        //Arg2: EPortIndex_TG1I/EPortIndex_TG2I
        //Arg3: sensorDev
        //for de-tuningQueue input
        MUINT32 sensorDev = eSoftwareScenario_Main_Normal_Stream;
        if (SENSOR_DEV_SUB == _NORMALPIPE_GET_SENSOR_DEV_ID((this->m_pNormalpipe_FrmB->mpSensorIdx[0]))) {
            sensorDev = eSoftwareScenario_Sub_Normal_Stream;
        }
        //else if (SENSOR_DEV_MAIN_2 == _NORMALPIPE_GET_SENSOR_DEV_ID((this->m_pNormalpipe_FrmB->mpSensorIdx[0]))) {
        //    sensorDev = eSoftwareScenario_Main2_N3D_Stream;
        //}

        //lock hrz in order to update img2o size for EIS to get hrz size
        NormalPipeGLock();
        if( PORT_IMG2O == port || PORT_IMGO == port ){
            tgIdx = EPortIndex_TG1I;
            if(PORT_IMGO == port)
                _cmd = EPIPECmd_SET_IMGO;
        }
        else{
            PIPE_ERR("error:unsupported dma port(0x%x)\n",port.index);
            return MFALSE;
        }

        PIPE_DBG("Dmacfg_%d:cmd:0x%x, crop:(%d,%d,%d,%d),outsize:(%d,%d)\n",tgIdx,_cmd,_dmao.crop.x,_dmao.crop.y,_dmao.crop.w,_dmao.crop.h,_dmao.out.w,_dmao.out.h);

        if ( MFALSE == this->m_pNormalpipe_FrmB->mpCamIOPipe->sendCommand(_cmd,(MINTPTR)&_dmao, tgIdx, sensorDev) ){
            PIPE_ERR("Error:EPIPECmd_SET_HRZ/IMGO(0x%x) Fail\n",_cmd);
            return MFALSE;
        }
        if(PORT_IMG2O == port){
            this->m_pNormalpipe_FrmB->mImg2oOut_size.w = _dmao.out.w;
            this->m_pNormalpipe_FrmB->mImg2oOut_size.h = _dmao.out.h;
        }else if(PORT_IMGO == port){
            this->m_pNormalpipe_FrmB->mImgoOut_size.w = _dmao.out.w;
            this->m_pNormalpipe_FrmB->mImgoOut_size.h = _dmao.out.h;
        }
        NormalPipeGUnLock();
    }

    //PIPE_DBG("-");

    return ret;
}


MBOOL NormalPipe_FrmB_Thread::TuningUpdate(MUINT32 magNum)
{
    MBOOL ret = MTRUE;
    MUINT32 sensorDev = eSoftwareScenario_Main_Normal_Stream;

    if (SENSOR_DEV_SUB == _NORMALPIPE_GET_SENSOR_DEV_ID((this->m_pNormalpipe_FrmB->mpSensorIdx[0]))) {
        sensorDev = eSoftwareScenario_Sub_Normal_Stream;
    }
    //else if (SENSOR_DEV_MAIN_2 == _NORMALPIPE_GET_SENSOR_DEV_ID((this->m_pNormalpipe_FrmB->mpSensorIdx[0]))) {
    //    sensorDev = eSoftwareScenario_Main2_N3D_Stream;
    //}

    PIPE_DBG("TuningUpdate_%d: mag:0x%x\n",_NOMRALPIPE_CVT_TGIDX(this->m_pNormalpipe_FrmB->mpSensorIdx[0]),magNum);

    ret = this->m_pNormalpipe_FrmB->mpCamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE, \
        _NOMRALPIPE_CVT_TGIDX(this->m_pNormalpipe_FrmB->mpSensorIdx[0]), magNum, sensorDev);
    return ret;
}


MVOID NormalPipe_FrmB_Thread::DummyFrameToEnque(MUINT32 _OpenedPort)
{
    MUINT32 _size;
    BufInfo    _buf;
    PIPE_DBG("dummy frame to enque\n");
    for(int i=0;i<_OpenedPort;i++){
        this->m_pNormalpipe_FrmB->mpDummyRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINTPTR)&_size,0);
    }

    if(_size != 0){
        //enque multi-PortID
        for(int i=0;i<_OpenedPort;i++){
            this->m_pNormalpipe_FrmB->mpDummyRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front  ,(MUINTPTR)&_buf,0);
            //
            this->Enque(&_buf);
            //
            this->DmaCfg(&_buf);
            //
            this->m_pNormalpipe_FrmB->mpDummyRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
            //dummy frame can't just drop, need to push back
            this->m_pNormalpipe_FrmB->mpDummyRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_push  ,(MUINTPTR)&_buf,0);
        }

        //
        this->TuningUpdate(_buf.FrameBased.mMagicNum_tuning);
    }
    else{    //err case, dummyrequest shouldn't empty
        PIPE_ERR("ERROR: DummyRequest r remainded in 0, drop frame\n");
    }
}

MVOID NormalPipe_FrmB_Thread::FrameToEnque(MUINT32 _OpenedPort)
{
    PIPE_TRACE_CALL();
    BufInfo    _buf;
    PIPE_DBG("frame to enque\n");
    //enque multi-PortID
    for(int i=0;i<_OpenedPort;i++){
        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front  ,(MUINTPTR)&_buf,0);
        //
        this->Enque(&_buf);
        //
        this->DmaCfg(&_buf);
        //
        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
    }

    //
    this->TuningUpdate(_buf.FrameBased.mMagicNum_tuning);
}


MVOID NormalPipe_FrmB_Thread::EnqueRequest(MUINT32 sof_idx)
{
    PIPE_TRACE_CALL();
    BufInfo    _buf;
    MUINT32 _size,_tmp;
    MUINT32 _ReservedReq = 0;
    MUINT32 _OpenedPort = this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum);
    MUINT32 *_ptr;
    _ptr = new MUINT32[_OpenedPort];
    //bypass these request until deque container is not empty
    _size=0;
    PIPE_DBG("EnqueRequest(%d_%d): \n",sof_idx,this->m_EnqueCnt);

    if(this->m_EnqueCnt++ < (this->m_pNormalpipe_FrmB->GetRingBufferDepth()-1)){ //-1 is because of first tuning is an immediate_request at enque()
        MUINT32 _patch_Num = 1;
        PIPE_DBG("update tuning in 1st buffer_loop (%d_%d) ",this->m_EnqueCnt,this->m_pNormalpipe_FrmB->GetRingBufferDepth());
        int nLost = min(sof_idx, this->m_pNormalpipe_FrmB->GetRingBufferDepth()-1);
        if(nLost > this->m_EnqueCnt){
            _patch_Num += (nLost - this->m_EnqueCnt);
            this->m_EnqueCnt += (_patch_Num-1);
            PIPE_INF("patch EnqueRequest at 1st buffer_loop(%d)\n",_patch_Num);
        }
        while(_patch_Num--){
            //enque multi-PortID
            for(int i=0;i<_OpenedPort;i++){
                this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front  ,(MUINTPTR)&_buf,0);
                //no need to enque into hw in 1st loop
                //this->Enque(&_buf);
                //
                this->DmaCfg(&_buf);
                //
                this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
            }
            this->TuningUpdate(_buf.FrameBased.mMagicNum_tuning);
        }
        //if 1st enq loop does NOT apply patch, MW cannot enque at this timing
        if(this->m_EnqueCnt == this->m_pNormalpipe_FrmB->GetRingBufferDepth()-1)
        {
            PIPE_DBG("1st buffer_loop done(%d)\n", this->m_EnqueCnt);
            this->m_pNormalpipe_FrmB->m_b1stEnqLoopDone = MTRUE;
        }
        delete [] _ptr;
        return;
    }

    for(int i=0;i<_OpenedPort;i++){
        this->m_pNormalpipe_FrmB->mpDeQueue[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINTPTR)&_ptr[i],0);
        _size += _ptr[i];
    }
    if(_size == 0){
        PIPE_INF("Warning: Dequeue container is empty, bypass this SOF(may drop frame)\n");
        delete [] _ptr;
        return;
    }else if( _OpenedPort > 1){
        for(int i=0;i<_OpenedPort;i++){
            //if deque container r not sync. between different dma port,one of the dma deque container r empty
            if(_ptr[i] == 0){
                PIPE_INF("warning:deque is interrupted, and the deuque in port:0x%x is empty, bypass this SOF(may drop frame)\n",this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i));
                delete [] _ptr;
                return;
            }
        }
        //if deque container under different dma port r not sync., but all dma deque container r not empty, only waring to notify
        //it's ok to this mechanism
        if( (_size % _OpenedPort) !=0 ){
            char _str[32];
            char* _pstr = _str;
            _str[0] = '\0';
            for(int i=0;i<_OpenedPort;i++){
                sprintf(_pstr,"0x%x",_ptr[i]);
                while(*_pstr++ == '\0');
            }
            PIPE_INF("Warning: deque is interrupted by enquethread(0x%x,%s)",_OpenedPort,_str);
        }
    }
    delete [] _ptr;

    //
    for(int i=0;i<_OpenedPort;i++){
        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINTPTR)&_size,0);
    }

    if(_size == 0){
        //
        //no request , user dummyrequest to prevent drop frame if current status r drop frame or next frame r drop frame
        NormalPipe_FRM_STATUS _drop_status;
        this->m_pNormalpipe_FrmB->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINTPTR)&_drop_status,0,0);
        PIPE_DBG("CUR FRAME STATUS:0x%x\n",_drop_status);

        if(_drop_status != _normal_status){
            this->DummyFrameToEnque(_OpenedPort);
        }
    }
    else{//available request from MW
        if(this->m_pNormalpipe_FrmB->m_PerFrameCtl){//Full mode means 1 request per frame, if requests r over 2, drop request
            this->m_DropEnQ.resize(0);
            for(int i=0;i<_OpenedPort;i++){
                MUINT32 _dropCnt = 2;
                this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINTPTR)&_size,0);
#if 1        //push oldest request into drv, and push other requests into reserve que for next frame.
                while(_size > 1){
                    PIPE_TRACE_BEGIN("TAG_Drop");
                    PIPE_INF("Warning: over 1 request in enqueRequest container(0x%x_0x%x)\n",\
                        _size,\
                        _OpenedPort);
                    //request over 2, must drop
                    MUINT32 __size;
                    this->m_pNormalpipe_FrmB->mpEnqueReserved[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINTPTR)&__size,0);
                    if(__size != 0)
                        _dropCnt = 1;
                    //if reserved container r empty, drop requestes which r over 2, otherwise, leave only 1 request.
                    while(_size>_dropCnt){
                        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front  ,(MUINTPTR)&_buf,0);
                        PIPE_INF("Warning: drop request: Port:0x%x, PA:0x%x, mag:0x%x\n",\
                            _buf.mPortID.index,\
                            _buf.mBuffer->getBufPA(0),\
                            _buf.FrameBased.mMagicNum_tuning);
                        //pop older request
                        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
                        _size--;
                        //only need to push 1 port into this vector
                        if(i==0)
                            this->m_DropEnQ.push_back(_buf.FrameBased.mMagicNum_tuning);
                    }
                    //
                    if(__size == 0){
                        //always 2 elements r remained in container,pop latest into reserved container
                        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_at  ,1,(MUINTPTR)&_buf);
                        PIPE_INF("Warning: reserved request: Port:0x%x, PA:0x%x, mag:0x%x\n",\
                                _buf.mPortID.index,\
                                _buf.mBuffer->getBufPA(0),\
                                _buf.FrameBased.mMagicNum_tuning);
                        this->m_pNormalpipe_FrmB->mpEnqueReserved[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_push  ,(MUINTPTR)&_buf,0);
                        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop_back  ,0,0);
                        _size--;
                    }
                    PIPE_TRACE_END();
                }
#else
                //over 1 request , use latest request and drop others
                while(_size > 1){
                    PIPE_INF("Warning: over 1 request in enqueRequest container(0x%x_0x%x)\n",\
                        _size,\
                        _OpenedPort);
                    //
                    this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_at  ,0,(MUINT32)&_buf);
                    PIPE_INF("Warning: drop request: Port:0x%x, PA:0x%x, mag:0x%x\n",\
                        _buf.mPortID.index,\
                        _buf.mBuffer->getBufPA(0),\
                        _buf.FrameBased.mMagicNum_tuning);
                    //pop older request
                    this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
                    //only need to push 1 port into this vector
                    if(i==0)
                        this->m_DropEnQ.push_back(_buf.FrameBased.mMagicNum_tuning);
#if 0
                    //reflush _size
                    this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINT32)&_size,0);
                    PIPE_DBG("new size after pop:0x%x\n",_size);
#else
                    _size--;
#endif
                }
#endif
            }

            //CB for notify MW which enque be dropped.
            for(int i=0;i<this->m_DropEnQ.size();i++){
                PIPE_INF("drop enque request:0x%x(0x%x)\n",this->m_DropEnQ.at(i),this->m_DropEnQ.size());
                if(this->m_pNormalpipe_FrmB->m_DropCB!= NULL){
                    this->m_pNormalpipe_FrmB->m_DropCB(this->m_DropEnQ.at(i), this->m_pNormalpipe_FrmB->m_returnCookie);
                }
            }

            //for HDRViewFinder: if current SOF = EnqueRequest SOF, reserve this request
            if(0 == this->m_DropEnQ.size()){
                this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,0))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front,(MUINTPTR)&_buf,0);
                MUINT32 _cur_sof_idx = sof_idx;
                MUINT32 _buf_sof_idx = _buf.FrameBased.mSOFidx;
                //sof = 0~255, _cur_sof_idx-_buf_sof must == 1
                if(_cur_sof_idx == 0 && _buf_sof_idx != 0)
                {
                    _cur_sof_idx += 256;
                }
                if(_cur_sof_idx - _buf_sof_idx <= 0)
                {
                    PIPE_INF("warning: SOF cur(%d)<=buf(%d), use dummy frame\n", sof_idx, _buf_sof_idx);
                    _ReservedReq = 1;
                }
                if(_cur_sof_idx - _buf_sof_idx > 1)
                {
                    PIPE_INF("warning: EnqueThread wakup too slow, cur(%d)-buf(%d)>2\n", sof_idx, _buf_sof_idx);
                }
                if(1 == _ReservedReq)
                {
                    this->DummyFrameToEnque(_OpenedPort);
                    for(int i=0;i<_OpenedPort;i++){
                        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front  ,(MUINTPTR)&_buf,0);
                        this->m_pNormalpipe_FrmB->mpEnqueReserved[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_push,(MUINTPTR)&_buf,0);
                        this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
                    }
                }
            }

        }

        if(0 == _ReservedReq){
            NormalPipe_FRM_STATUS _drop_status;
            this->m_pNormalpipe_FrmB->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINTPTR)&_drop_status,0,0);
            if(_drop_status == _drop_frame_status){
                PIPE_INF("warning: cur frame r drop frame, use dummy frame\n");
                this->DummyFrameToEnque(_OpenedPort);
            }
            else{
                //start enqueRequest
                this->FrameToEnque(_OpenedPort);
            }
        }

        //if reserved conatiner r not empty, push reserved container into enque container
        this->m_pNormalpipe_FrmB->mpEnqueReserved[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,0))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_getsize ,(MUINTPTR)&_size,0);
        if(_size != 0){
            for(int i=0;i<_OpenedPort;i++){
                this->m_pNormalpipe_FrmB->mpEnqueReserved[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front  ,(MUINTPTR)&_buf,0);
                this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_push,(MUINTPTR)&_buf,0);
                this->m_pNormalpipe_FrmB->mpEnqueReserved[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
            }
        }
    }
}

MVOID NormalPipe_FrmB_Thread::ImmediateRequest(MBOOL bByPassHwEnq){
    BufInfo    _buf;
    PIPE_DBG("ImmediateRequest\n");
    for(int i=0;i<this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_OpendNum);i++){
            this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_front  ,(MUINTPTR)&_buf,0);
            //
            if(bByPassHwEnq == MFALSE)
                this->Enque(&_buf);
            else
                PIPE_DBG("bypass hw enque\n");
            //
            this->DmaCfg(&_buf);
            //
            this->m_pNormalpipe_FrmB->mpEnqueRequest[_PortMap(this->m_pNormalpipe_FrmB->GetOpenedPortInfo(NormalPipe_FrmB::_GetPort_Index,i))]->sendCmd(NormalPipe_FrmB::QueueMgr::eCmd_pop  ,0,0);
        }

        //
        this->TuningUpdate(_buf.FrameBased.mMagicNum_tuning);
}

MVOID* NormalPipe_FrmB_Thread::IspEnqueThread(void *arg)
{
    NormalPipe_FrmB_Thread *_this = reinterpret_cast<NormalPipe_FrmB_Thread*>(arg);
    MUINT32 _sof_idx,_sof_idx2;
    struct timeval tv;
    MUINT32 _t_wait, _t_wake, _t_SOF, _t_Equnlock;

    /// set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_LOMO;
    //
    struct sched_param sched_p;

    ::sched_getparam(0, &sched_p);
    if(policy == SCHED_OTHER)
    {
        setThreadPriority(policy, priority);
#if 0
        int _policy    = SCHED_OTHER;
        int _priority  = NICE_CAMERA_LOMO;
        getThreadPriority(_policy, _priority);
        //
        PIPE_DBG(
            "[IspEnqueThread] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , policy, _policy, priority, _priority
            );
#endif
    }
    else
    {    //  Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }
    //  detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    ::sem_wait(&_this->m_semEnqueThread);

    PIPE_INF("Warning:temp borrow NICE_CAMERA_LOMO priority, need to create self priority\n");
    //set thread name
    switch(_NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe_FrmB->mpSensorIdx[0])){
        case CAM_TG_1:    ::prctl(PR_SET_NAME,"IspEnqueThread_TG1",0,0,0);
            break;
        default:
            PIPE_ERR("error: Err TG:0x%x(0x%x)\n",_NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe_FrmB->mpSensorIdx[0]),_this->m_pNormalpipe_FrmB->mpSensorIdx[0]);
            goto _exit;
            break;
    }
    //
    while (1)
    {
        _this->mLock.lock();
        if(_this->m_bStart == MFALSE){
            _this->mLock.unlock();
            PIPE_DBG("EqThread leave\n");
            break;
        }
        _this->mLock.unlock();
        //
#if CONFIG_PIPE_TIME
        PIPETIME_DBG("EqThread wait+\n");
        ::gettimeofday(&tv, NULL);
        _t_wait = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
        //
        _this->m_pNormalpipe_FrmB->wait(_this->m_pNormalpipe_FrmB->mpSensorIdx[0],EPipeSignal_SOF,300);
        //
#if CONFIG_PIPE_TIME
        PIPETIME_DBG("EqThread wait-\n");
        ::gettimeofday(&tv, NULL);
        _t_wake = tv.tv_sec * 1000000 + tv.tv_usec;
//        if(_t_wake-_t_wait >= 1000)// >1ms
        {
            PIPETIME_DBG("_T: EqThread wait (%d)\n", _t_wake-_t_wait);
        }
#endif

        //check again for stop isp, it will flush EnqueThread SOF IRQ
        _this->mLock.lock();
        if(_this->m_bStart == MFALSE){
            _this->mLock.unlock();
            PIPE_DBG("EqThread leave after wait SOF\n");
            break;
        }
        _this->mLock.unlock();
        //
        _this->m_pNormalpipe_FrmB->sendCommand(EPIPECmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx,0,0);
        PIPE_DBG("cur SOF = 0x%x\n",_sof_idx);
        //
#if CONFIG_PIPE_TIME
        ::gettimeofday(&tv, NULL);
        _t_SOF = tv.tv_sec * 1000000 + tv.tv_usec;
        if(_t_SOF-_t_wake >= 1000)// >1ms
        {
            PIPETIME_DBG("_T: EqThread sendComm:getSOF (%d)\n", _t_SOF-_t_wake);
        }
#endif
        //
        _this->m_pNormalpipe_FrmB->mEnqContainerLock.lock();
        _this->EnqueRequest(_sof_idx);
        _this->m_pNormalpipe_FrmB->mEnqContainerLock.unlock();
        //
#if CONFIG_PIPE_TIME
        PIPETIME_DBG("EqThread EqRequest done\n");
        ::gettimeofday(&tv, NULL);
        _t_Equnlock = tv.tv_sec * 1000000 + tv.tv_usec;
        if(_t_Equnlock-_t_SOF >= 1000)// >1ms
        {
            PIPETIME_DBG("_T: EqThread EqRequest (%d)\n", _t_Equnlock-_t_SOF);
        }
#endif
        //
        _this->m_pNormalpipe_FrmB->sendCommand(EPIPECmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx2,0,0);

        if(_sof_idx != _sof_idx2){
            PIPE_TRACE_BEGIN("TAG_OverVsync");
            PIPE_INF("Warning: TG(0x%x): EnqueRequest over vsync(0x%x_0x%x)\n",_NORMALPIPE_GET_TG_IDX((_this->m_pNormalpipe_FrmB->mpSensorIdx[0])),_sof_idx,_sof_idx2);
            PIPE_TRACE_END();
        }
    }
_exit:
    ::sem_post(&_this->m_semEnqueThread);
    //

    return NULL;
}



