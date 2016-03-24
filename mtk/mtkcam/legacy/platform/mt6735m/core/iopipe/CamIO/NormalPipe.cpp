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
#define LOG_TAG "NormalPipe"
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

/* Header files
********************************/
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/Thread.h>
#include <utils/Condition.h>
#include <cutils/properties.h>

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#if 0
#include <mtkcam/utils/common.h>
#include <mtkcam/common/hw/hwstddef.h>
#include <mtkcam/common/camutils/CamFormat.h>
#include <mtkcam/drv_common/imem_drv.h>
#include <mtkcam/v1/camutils/CamInfo.h>
#endif

#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/hal/sensor_hal.h>
#include <kd_imgsensor_define.h>

#include "isp_function.h"
#include "NormalPipe.h"
#include "PortMap.h"
#include "isp_datatypes.h"
#include "mtk_metadata_tag.h"

/************************************
* [floria]
************************************/
#if 0
#else //Wrapper
    using namespace std;
    #define FUNCTION_LOG_START      CAM_LOGD("+");
    #define FUNCTION_LOG_END        CAM_LOGD("-");
    #define _USE_SEMA_
#endif
/************************************
*
************************************/
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
//using namespace NSImageio;
//using namespace NSIspio;

#define PASS1_CQ_CONTINUOUS_MODE__  1
#define SENSOR_METADATA_IS_READY 0
#define SENSOR_TYPE_IS_RAW       1

//#include "mapper/iopipeImgioPipeMapper.h"

static MBOOL checkDumpNormalPipe(void);

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
};

//switch to camiopipe index
#define _NORMALPIPE_GET_TGIFMT(sensorType, sensorFormatOrder, rawSensorBit, rawPixelID,fmt) {\
    switch(sensorType) {\
        case SENSOR_HAL_TYPE_RAW:\
            switch(rawSensorBit) {\
                case 8:    fmt = NSCam::eImgFmt_BAYER8; break;\
                case 10: fmt = NSCam::eImgFmt_BAYER10; break;\
                case 12: fmt = NSCam::eImgFmt_BAYER12; break;\
                default: fmt = NSCam::eImgFmt_BAYER8; break;\
            }\
            switch(sensorFormatOrder)\
            {\
                case 0: rawPixelID = NSImageio::NSIspio::ERawPxlID_B; break;\
                case 1: rawPixelID = NSImageio::NSIspio::ERawPxlID_Gb; break;\
                case 2: rawPixelID = NSImageio::NSIspio::ERawPxlID_Gr; break;\
                case 3: rawPixelID = NSImageio::NSIspio::ERawPxlID_R; break;\
                default: rawPixelID = NSImageio::NSIspio::ERawPxlID_Gb; break;\
            }\
            break;\
        case SENSOR_HAL_TYPE_YUV:\
        case SENSOR_HAL_TYPE_YCBCR:\
            switch(sensorFormatOrder) {\
                case SENSOR_FORMAT_ORDER_UYVY: fmt = NSCam::eImgFmt_UYVY; break;\
                case SENSOR_FORMAT_ORDER_VYUY: fmt = NSCam::eImgFmt_VYUY; break;\
                case SENSOR_FORMAT_ORDER_YVYU: fmt = NSCam::eImgFmt_YVYU; break;\
                case SENSOR_FORMAT_ORDER_YUYV: fmt = NSCam::eImgFmt_YUY2; break;\
                default:    CAM_LOGE("Err sen yuv fmt err\n"); break;\
            }\
            break;\
        case SENSOR_HAL_TYPE_RGB565:\
            fmt = NSCam::eImgFmt_RGB565;\
            break;\
        case SENSOR_HAL_TYPE_RGB888:\
            fmt = NSCam::eImgFmt_RGB888;\
            break;\
        case SENSOR_HAL_TYPE_JPEG:\
            fmt = NSCam::eImgFmt_JPEG;\
            break;\
        default:\
            CAM_LOGE("Err sen type(%d) err\n", sensorType);\
            break;\
    }\
}

#if 0
#define _NORMALPIPE_GET_TGIFMT(sensorType, sensorFormatOrder, rawSensorBit, rawPixelID,fmt) {\
    switch(sensorType) {\
        case SENSOR_HAL_TYPE_RAW:\
            switch(rawSensorBit) {\
                case RAW_SENSOR_8BIT:    fmt = NSCam::eImgFmt_BAYER8; break;\
                case RAW_SENSOR_10BIT:    fmt = NSCam::eImgFmt_BAYER10; break;\
                case RAW_SENSOR_12BIT:    fmt = NSCam::eImgFmt_BAYER12; break;\
                case RAW_SENSOR_14BIT:  fmt = NSCam::eImgFmt_BAYER14; break;\
                default: CAM_LOGE("Err sen raw fmt(%d) err\n", rawSensorBit); break;\
            }\
            switch(sensorFormatOrder)\
            {\
                case 0: rawPixelID = NSImageio::NSIspio::ERawPxlID_B; break;\
                case 1: rawPixelID = NSImageio::NSIspio::ERawPxlID_Gb; break;\
                case 2: rawPixelID = NSImageio::NSIspio::ERawPxlID_Gr; break;\
                case 3: rawPixelID = NSImageio::NSIspio::ERawPxlID_R; break;\
                default: rawPixelID = NSImageio::NSIspio::ERawPxlID_Gb; break;\
            }\
            break;\
        case SENSOR_HAL_TYPE_YUV:\
        case SENSOR_HAL_TYPE_YCBCR:\
            switch(sensorFormatOrder) {\
                case SENSOR_FORMAT_ORDER_UYVY: fmt = NSCam::eImgFmt_UYVY; break;\
                case SENSOR_FORMAT_ORDER_VYUY: fmt = NSCam::eImgFmt_VYUY; break;\
                case SENSOR_FORMAT_ORDER_YVYU: fmt = NSCam::eImgFmt_YVYU; break;\
                case SENSOR_FORMAT_ORDER_YUYV: fmt = NSCam::eImgFmt_YUY2; break;\
                default:    CAM_LOGE("Err sen yuv fmt err\n"); break;\
            }\
            break;\
        case SENSOR_HAL_TYPE_RGB565:\
            fmt = NSCam::eImgFmt_RGB565;\
            break;\
        case SENSOR_HAL_TYPE_RGB888:\
            fmt = NSCam::eImgFmt_RGB888;\
            break;\
        case SENSOR_HAL_TYPE_JPEG:\
            fmt = NSCam::eImgFmt_JPEG;\
            break;\
        default:\
            CAM_LOGE("Err sen type(%d) err\n", sensorType);\
            break;\
    }\
}
#endif
//switch to camiopipe idx
#define _NORMALPIPE_GET_TGI_PIX_ID(rawPixelID,pix_id) {\
    switch(rawPixelID){\
        case SENSOR_FORMAT_ORDER_RAW_B:    pix_id = ERawPxlID_B; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gb: pix_id = ERawPxlID_Gb; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gr: pix_id = ERawPxlID_Gr; break;\
        case SENSOR_FORMAT_ORDER_RAW_R: pix_id = ERawPxlID_R; break;\
        case SENSOR_FORMAT_ORDER_UYVY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_VYUY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YUYV: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YVYU: pix_id = (ERawPxlID)0; break;\
        default:    CAM_LOGE("Error Pix_id=%d", rawPixelID); break;\
    }\
    CAM_LOGD("rawPixelID=%d", rawPixelID);\
}

#define _NORMALPIPE_GET_SENSORTYPE(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType)
//switch to camiopipe index
#define _NOMRALPIPE_CVT_TGIDX(sensorIdx,tgidx){\
    switch(pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo) {\
        case CAM_TG_1: tgidx = EPortIndex_TG1I; break;\
        case CAM_TG_2: tgidx = EPortIndex_TG2I; break;\
        case CAM_SV_1: tgidx = EPortIndex_CAMSV_TG1I; break;\
        case CAM_SV_2: tgidx = EPortIndex_CAMSV_TG2I; break;\
        default:    CAM_LOGE("Error tg idx err"); break;\
    }\
}

#if 0
#define _NORMALPIPE_GET_SENSORCFG_CROP_W(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.w)
#define _NORMALPIPE_GET_SENSORCFG_CROP_H(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.h)
#define _NORMALPIPE_GET_SENSOR_DEV_ID(sensorIdx)    (pAllSensorInfo->mSenInfo[sensorIdx].mDevId)

#define _NORMALPIPE_GET_TG_IDX(sensorIdx)     (pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo)
#define _NORMALPIPE_GET_PIX_MODE(sensorIdx)   (pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.pixelMode)
#endif

//map portId to deque container index
#define _PortMap(PortIdx)    ({\
    int _idx=0;\
    if( (PortIdx == PORT_IMGO.index))\
        _idx = 1;\
    _idx;\
})

// mapping element NormalPipe::mTgOut_size
#define    _TGMapping(x)({\
    MUINT32 _tg=0;\
    if(x==CAM_TG_1)\
        _tg = 0;\
    else if(x==CAM_TG_2)\
        _tg = 1;\
    else if(x = CAM_SV_1)\
        _tg = 2;\
    else if(x = CAM_SV_2)\
        _tg = 3;\
    _tg;\
})


MUINT32 p1ConvSensorHalTG2Num(MUINT32 sensorHalTG)
{
   switch (sensorHalTG)
   {
        case CAM_TG_1: return 1;
        case CAM_TG_2: return 2;
        case CAM_SV_1: return 3;
        case CAM_SV_2: return 4;
        default:
            CAM_LOGE("Err: Invalid sensorHalTG(%d)",sensorHalTG);
        return CAM_TG_1;
    }
    return ISP_DRV_CQ_NUM;
}

#if 1
NSImageio::NSIspio::EScenarioFmt _sensorTypeToPortFormat(MUINT32 sensorType)
{
    switch (sensorType)  {
        case SENSOR_HAL_TYPE_RAW:
            return NSImageio::NSIspio::eScenarioFmt_RAW;
            break;
        case SENSOR_HAL_TYPE_YUV:
        case SENSOR_HAL_TYPE_YCBCR:
            return NSImageio::NSIspio::eScenarioFmt_YUV;
            break;
        case SENSOR_HAL_TYPE_RGB565:
        case SENSOR_HAL_TYPE_RGB888:
            return NSImageio::NSIspio::eScenarioFmt_RGB;
            break;
        case SENSOR_HAL_TYPE_JPEG:
            return NSImageio::NSIspio::eScenarioFmt_JPG;
            break;
        default:
            break;
    }
   return NSImageio::NSIspio::eScenarioFmt_RAW;
}

#else //95
NSImageio::NSIspio::EScenarioFmt _sensorTypeToPortFormat(MUINT32 sensorType)
{

   switch (sensorType)  {
       case NSCam::SENSOR_TYPE_RAW:  return NSImageio::NSIspio::eScenarioFmt_RAW;
       case NSCam::SENSOR_TYPE_YUV:  return NSImageio::NSIspio::eScenarioFmt_YUV;
       case NSCam::SENSOR_TYPE_RGB:  return NSImageio::NSIspio::eScenarioFmt_RGB;
       case NSCam::SENSOR_TYPE_JPEG: return NSImageio::NSIspio::eScenarioFmt_JPG;
   }
   return NSImageio::NSIspio::eScenarioFmt_RAW;
}
#endif


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

static NormalPipe* pNormalPipe[NPIPE_Sensor_RSVD] = {NULL, NULL, NULL};
#if 0
static platSensorsInfo *pAllSensorInfo = NULL;
#endif
static Mutex  NPipeGLock;//NormalPipe Global Lock


#define NormalPipeGLock() NPipeGLock.lock()
#define NormalPipeGUnLock() NPipeGLock.unlock()

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
       CAM_LOGD("%s::NPIPE_ALLOC_MEM=0x%x, NPIPE_FREE_MEM=0x%x",string,NPIPE_ALLOC_MEM, NPIPE_FREE_MEM);\
   }\
}while(0)


   struct stNPipe_RRZ_INFO
   {
       MUINT32                 srcX;
       MUINT32                 srcY;
       MUINT32                 srcW;
       MUINT32                 srcH;
       MUINT32                 dstW;
       MUINT32                 dstH;
   };

   struct stNPipe_BUF_INFO
   {
       MUINT32                status;
       MUINT32                 base_vAddr;
       MUINT32                 base_pAddr;
       MUINT32                 size;
       MUINT32                 memID;
       MINT32                  bufSecu;
       MINT32                  bufCohe;
       MUINT64                 timeStampS;
       MUINT32                 timeStampUs;
       //
       MUINT32                 img_w;
       MUINT32                 img_h;
       MUINT32                 img_stride;
       MUINT32                 img_fmt;
       MUINT32                 img_pxl_id;
       MUINT32                 img_wbn;
       MUINT32                 img_ob;
       MUINT32                 img_lsc;
       MUINT32                 img_rpg;
       MUINT32                 m_num_0;
       MUINT32                 m_num_1;
       MUINT32                 frm_cnt;
       MUINT32                 dma;

       stNPipe_RRZ_INFO        rrz_info;
       //
       MVOID                   *header_info; //point to self
       MUINT32                 header_size;

       MUINT32                 jpg_size;
       MVOID                   *private_info;
       struct stNPipe_BUF_INFO *next;
    //
    stNPipe_BUF_INFO(
        MUINT32           _status = 0,
        MUINT32                 _base_vAddr = 0,
        MUINT32                 _base_pAddr = 0,
        MUINT32                 _size = 0,
        MUINT32                 _memID = -1,
        MINT32                  _bufSecu = 0,
        MINT32                  _bufCohe = 0,
        MUINT64                 _timeStampS = 0,
        MUINT32                 _timeStampUs = 0,
        MUINT32                 _img_w = 0,
        MUINT32                 _img_h = 0,
        MUINT32                 _img_fmt = 0,
        MUINT32                 _img_hbin1 = 0,
        MUINT32                 _img_ob = 0,
        MUINT32                 _img_lsc = 0,
        MUINT32                 _img_rpg = 0,
        MUINT32                 jpg_size = 0,
        MVOID                   *_private_info = NULL,
        struct stNPipe_BUF_INFO   *_next = 0)
    : status(_status)
    , base_vAddr(_base_vAddr)
    , base_pAddr(_base_pAddr)
    , size(_size)
    , memID(_memID)
    , bufSecu(_bufSecu)
    , bufCohe(_bufCohe)
    , timeStampS(_timeStampS)
    , timeStampUs(_timeStampUs)
    , private_info(_private_info)
    , next(_next)
    {}
   };

enum ENpipeUpdateTgtDev
{
    eNpipeUpdateTgtDev_Main  = 1,
    eNpipeUpdateTgtDev_Sub   = 2,
    eNpipeUpdateTgtDev_Main2 = 3,
};


/******************************************************************************
 *
 ******************************************************************************/
#if 0
void INormalPipe::Lock(void) {

    mLock.lock();

}

void INormalPipe::UnLock(void) {

   mLock.unlock();

}
#endif


INormalPipe*
INormalPipe::createInstance(MUINT32 *pSensorIndex, MUINT32 sensorCnt,char const* szCallerName, MINT32 burstQnum)
{
    //NPIPE_DUMP_MEM_INFO("createInstance");

    if ((sensorCnt > NPIPE_Sensor_0And1) || \
        (sensorCnt > NPIPE_MAX_ENABLE_SENSOR_CNT)) {
        CAM_LOGE("Invalid sCnt=%d", sensorCnt);
    }

#if 0
    NormalPipeGLock();
    if (!pAllSensorInfo){
        IHalSensorList *mSList = NULL;
        _sensorInfo_t  *pSInfo = NULL;
        MUINT32 SCnt = 0;

        NPIPE_MEM_NEW(pAllSensorInfo, platSensorsInfo, sizeof(platSensorsInfo));
        CAM_LOGD("[%d]createInstance, %s,Alloc pAllSensorInfo=0x%8x",pSensorIndex[0],szCallerName, pAllSensorInfo);

        mSList = IHalSensorList::get();
#ifdef USING_MTK_LDVT
        mSList->searchSensors();
#endif
        SCnt =  mSList->queryNumberOfSensors();

        pAllSensorInfo->mSList       = mSList;
        pAllSensorInfo->mExistedSCnt = SCnt;
        pAllSensorInfo->mUserCnt     = 0;
        if ((SCnt > NPIPE_MAX_SENSOR_CNT) || (SCnt==0)) {
            CAM_LOGE("Not support NPIPE_MAX_SENSOR_CNT sensors " );
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

            CAM_LOGD("[%d]SensorName=%s, Type=%d, DevId=%d", \
                     i, mSList->queryDriverName(i), \
                     pSInfo->mTypeforMw, pSInfo->mDevId );
        }
    }
    pAllSensorInfo->mUserCnt++;
    NormalPipeGUnLock();
#endif

    NormalPipe* pNPipe = 0;

    if (1 == sensorCnt) {//NPIPE_Sensor_0 ||  NPIPE_Sensor_1

        if (pSensorIndex[0] > NPIPE_Sensor_1){
            CAM_LOGE("INormalPipe::createInstance InvalidSensorIdx = %d", pSensorIndex[0]);
            return MFALSE;
        }

        NormalPipeGLock();
        pNPipe = pNormalPipe[pSensorIndex[0]];
        if (NULL == pNPipe)  {

            //pNPipe = new NormalPipe(&pSensorIndex[0], 1, szCallerName);
            NPIPE_MEM_NEW(pNormalPipe[pSensorIndex[0]], NormalPipe(&pSensorIndex[0], 1, szCallerName, burstQnum), sizeof(NormalPipe));

            pNPipe = pNormalPipe[pSensorIndex[0]];
            pNPipe->mUserCnt= 0;
            pNPipe->mpSensorIdx[0] = pSensorIndex[0];
            pNPipe->mpEnabledSensorCnt = sensorCnt;
            CAM_LOGD("[%d]createInstance, %s,pNormalPipe[%d]=0x%8x create\n",pSensorIndex[0],szCallerName, pSensorIndex[0], pNPipe);
        }
        else {
            //CAM_LOGD("pNormalPipe[%d]=0x%8x exist\n", pSensorIndex[0], pNPipe);
        }
        pNPipe->mUserCnt++;
        NormalPipeGUnLock();
    }
    else if (2 == sensorCnt){
       CAM_LOGE("Not Support Create 2 sensor at the same time\n");
   }

    //CAM_LOGD("mUserCnt=%d -",pNPipe->mUserCnt);
    //CAM_LOGD("createInstance-.(0x%8x)NPipe[%d]->mUserCnt(%d) ", pNPipe, pNPipe->mpSensorIdx[0], pNPipe->mUserCnt);

    return pNPipe;
}


INormalPipe*
INormalPipe::createInstance(MUINT32 SensorIndex,char const* szCallerName, MINT32 burstQnum)
{
   return createInstance(&SensorIndex, 1, szCallerName, burstQnum);
}


INormalPipe*
INormalPipe::createInstance( char const* szCallerName, MINT32 burstQnum)
{
   MUINT32 sidx = 0;
   return createInstance(&sidx, 1, szCallerName, burstQnum);
}

void
INormalPipe::destroyInstance(char const* szCallerName)
{
    int idx;
    //CAM_LOGD("INormalPipe::destroyInstance %s", szCallerName);

    return;
}


/******************************************************************************
 *
 ******************************************************************************/

IMetadata const&
INormalPipe::queryCapability(MUINT32 iOpenId) //iOpenId is sensorID
{

    Mutex::Autolock _l(gNPipeCapMutex);

    if  ( ! gPipeCap.isEmpty() )  {
        return  gPipeCap;
    }

    //  Pipe
    {
        IMetadata& cap = gPipeCap;
        {   // out port
        IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_OUT_PORT_INFO);
        entry.push_back(gPortCap_IMGO, Type2Type<IMetadata>());
        cap.update(entry.tag(), entry);
        }
        cap.sort();
    }
    //
    CAM_LOGD("count:%d", gPipeCap.count());
    return  gPipeCap;

}



/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::mEnablePath = 0;

MVOID
NormalPipe::destroyInstance(char const* szCallerName)
{
    int idx;

    NormalPipeGLock();
    for (idx = 0; idx < NPIPE_Sensor_RSVD; idx++){
       if (this == pNormalPipe[idx]){
          break;
       }
    }
    if (MTRUE == mDumpNPipeFlag)
    {
       CAM_LOGD("[%d]destroyInstance, %s", idx, szCallerName);
    }

    if ((pNormalPipe[idx]->mUserCnt <= 0)){
        CAM_LOGD("INormalPipe::destroyInstance. No instance now");
        NormalPipeGUnLock();
        return;
    }

    if (NPIPE_Sensor_RSVD == idx) {
        CAM_LOGE("INormalPipe::destroyInstance. Can't find the entry");
        NormalPipeGUnLock();
        return;
    }

    if (pNormalPipe[idx]->mUserCnt > 0)  {
        pNormalPipe[idx]->mUserCnt--;
    }

    if (MTRUE == mDumpNPipeFlag)
    {
        CAM_LOGD("(0x%8x)NPipe[%d]->mUserCnt(%d) ", pNormalPipe[idx], idx, pNormalPipe[idx]->mUserCnt);
    }

    if ((pNormalPipe[idx]->mUserCnt == 0))  {
#if 0
        if  ( mpHalSensor == NULL )  {
            CAM_LOGE("NULL pHalSensor");
            NormalPipeGUnLock();
            return;
        }
        mpHalSensor->destroyInstance();
        mpHalSensor= NULL;
#endif

        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
        CAM_LOGD("Destroy NPipe[%d]=0x%8x",idx, pNormalPipe[idx]);
        NPIPE_MEM_DEL(pNormalPipe[idx], sizeof(NormalPipe));
    }

#if 0
    if (pAllSensorInfo) {
       if (pAllSensorInfo->mUserCnt > 0){
           pAllSensorInfo->mUserCnt--;
       }
       else{
          //return;
       }

       if ((pAllSensorInfo->mUserCnt == 0) && (pAllSensorInfo)){
           CAM_LOGD("Destroy pAllSensorInfo. idx=%d",idx);
           NPIPE_MEM_DEL(pAllSensorInfo, sizeof(platSensorsInfo));
           NPIPE_DUMP_MEM_INFO("Destroy");
           //NPIPE_ALLOC_MEM = 0;
           //NPIPE_FREE_MEM = 0;
       }
    }
#endif
    NormalPipeGUnLock();

    //CAM_LOGD("destroyInstance-.pNormalPipe =0x%8x/0x%8x/0x%8x", pNormalPipe[0], pNormalPipe[1], pNormalPipe[2]);

}


/******************************************************************************
 *
 ******************************************************************************/
NormalPipe::NormalPipe(char const* szCallerName)
    : IHalCamIO()
    , mpCamIOPipe(NULL)
    , mpFrameMgr(NULL)
    , mpName(szCallerName)
    , mBurstQNum(1)
    /* for Wrapper */
    , mfgIsYUVPortON(MFALSE)
{
    mpEnabledSensorCnt = 1;

    mbTwoPixelEnable = MFALSE;
    mDumpNPipeFlag = MFALSE;
    mConfigDone = MFALSE;
    mRrzoOut_size.w = 0;
    mRrzoOut_size.h = 0;
    mFrmCntPerMnum = 0;
    mLastMnum = 0;
    m_bEISChkFlg = MFALSE;

    m_bStarted = MFALSE;
    mRingBufDepth = 0;

    for(int i=0;i<MaxPortIdx;i++){
        mpDeQueue[i] = NULL;
        PortIndex[i] = 0;
    }
}

NormalPipe::NormalPipe(MUINT32* pSensorIdx, MUINT32 SensorIdxCnt, char const* szCallerName, MINT32 burstQnum)
    : IHalCamIO()
    , mpCamIOPipe(NULL)
    , mpFrameMgr(NULL)
    , mpName(szCallerName)
    , mBurstQNum(1)
    /* for Wrapper */
    , mfgIsYUVPortON(MFALSE)
{
    mpEnabledSensorCnt = SensorIdxCnt;

    mbTwoPixelEnable = MFALSE;
    mDumpNPipeFlag = MFALSE;
    mConfigDone = MFALSE;
    mRrzoOut_size.w = 0;
    mRrzoOut_size.h = 0;
    mFrmCntPerMnum = 0;
    mLastMnum = 0;
    //update burstQnumber if mw member update it, otherwise use default value 1
    CAM_LOGD("income BQNum(%d)",burstQnum);
    mBurstQNum=burstQnum;

    m_bStarted = MFALSE;
    mRingBufDepth = 0;

    for(int i=0;i<MaxPortIdx;i++){
        mpDeQueue[i] = NULL;
        PortIndex[i] = 0;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::start()
{
    MBOOL ret = MFALSE;

    /*---------------------------------------------------
    *   Frame based
    ---------------------------------------------------*/
    this->m_bStarted = MTRUE;
    /*---------------------------------------------------
    *   Frame based (end)
    ---------------------------------------------------*/

    ret = mpCamIOPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                (MINT32)NSImageio::NSIspio::EPortIndex_IMGO,
                                 0,
                                 0
                               );

    if (mfgIsYUVPortON)
    {
    ret = mpCamIOPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                   (MINT32)NSImageio::NSIspio::EPortIndex_IMG2O,
                                    0,
                                    0
                                   );
    }

    //
    // (2) start CQ
    ret = mpCamIOPipe->startCQ0();
    //
    // ! let commond queue trigger mode as continuous mode
    ret = mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,
                               (MINT32)NSImageio::NSIspio::EPIPE_CQ_NONE,
                               0,
                               0
                                ); //[?]
    //
    ret = mpCamIOPipe->start();
    // (3) sync vync, 82 has only TG1
    //ret = mpCamIOPipe->irq( NSImageio::NSIspio::EPipePass_PASS1_TG1,
    //                       NSImageio::NSIspio::EPIPEIRQ_VSYNC
    //                       );

#if 0
    CAM_LOGD(" Wait for stable frame:%d", mu4SkipFrame);
    skipFrame(mu4SkipFrame);
#endif

    #ifdef _USE_SEMA_
    //must deque-> enque ...
    CAM_LOGD("Release Deque Resource");
    ::sem_post(&mSemDeQue);
    #endif

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::stop()
{

    bool ret = MTRUE;

    /*---------------------------------------------------
    *   Frame based
    ---------------------------------------------------*/
    this->m_bStarted = MFALSE;
    /*---------------------------------------------------
    *   Frame based (end)
    ---------------------------------------------------*/

    if (mpCamIOPipe)
    {
        ret = mpCamIOPipe->stop();
    }

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::init()
{
    MUINT32 idx;
    bool ret = MTRUE;
    //char const* szCallerName = "NormalPipe";
    CAM_LOGD("NormalPipe::init+ SenIdx=%d", mpSensorIdx[0]);

    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->init();
    }


#ifdef USE_IMAGEBUF_HEAP
    if (!mpFrameMgr) {
        NPIPE_MEM_NEW(mpFrameMgr,FrameMgr(),sizeof(FrameMgr));
    }
    mpFrameMgr->init();
#endif
    /*---------------------------------------------------
    *   Frame based
    ---------------------------------------------------*/
    for(int i=0;i<MaxPortIdx;i++){
        if(!mpDeQueue[i]){
            NPIPE_MEM_NEW(mpDeQueue[i],QueueMgr(),sizeof(QueueMgr));
            mpDeQueue[i]->init();
        }
    }
    #ifdef _USE_SEMA_
    ::sem_init(&mSemDeQue, 0, 0);
    ::sem_init(&mSemEnQue, 0, 0);
    #endif
    /*---------------------------------------------------
    *   Frame based (end)
    ---------------------------------------------------*/

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::uninit()
{
    MBOOL ret = MTRUE;
    //
    if (NULL != mpSensorHal)
    {
         mpSensorHal->uninit();
         mpSensorHal->destroyInstance();
         mpSensorHal = NULL;
    }

    //
    if (NULL != mpCamIOPipe)
    {
        if (MTRUE != mpCamIOPipe->uninit())
        {
            ret = MFALSE;
        }
    }

#ifdef USE_IMAGEBUF_HEAP
    if (mpFrameMgr)  {
        mpFrameMgr->uninit();
        NPIPE_MEM_DEL(mpFrameMgr,sizeof(FrameMgr));
    }
#endif

    /*---------------------------------------------------
    *   Frame based
    ---------------------------------------------------*/
    for(int i=0;i<MaxPortIdx;i++){
        if (mpDeQueue[i])  {
            mpDeQueue[i]->uninit();
            NPIPE_MEM_DEL(mpDeQueue[i],sizeof(QueueMgr));
        }
    }
    #ifdef _USE_SEMA_
    ::sem_post(&mSemDeQue);
    ::sem_post(&mSemEnQue);
    #endif
    /*---------------------------------------------------
    *   Frame based (end)
    ---------------------------------------------------*/
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::enque(QBufInfo const& rQBuf)
{
//95
    bool ret = MTRUE;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QBufInfo rQBufInfo;
    BufInfo _buf;
    //
    //CAM_LOGD("NormalPipe::enque +");
    Mutex::Autolock lock(mEnQLock);

    if(this->checkEnque(rQBuf) == MFALSE){
        CAM_LOGE("enque fail\n");
        return MFALSE;
    }

    if(this->m_bStarted == MFALSE){
        //isp not started yet, push buffer into FBC directly
        for (MUINT32 i = 0; i < rQBuf.mvOut.size(); i++)
        {
            this->mRingBufDepth++;
            //
            portID.index = rQBuf.mvOut.at(i).mPortID.index;
            //82 only surpport IMGO
            if(portID.index != PORT_IMGO.index)
            {
                CAM_LOGE("enque fail: (%d/%d), port.Idx(%d)!=PORT_IMGO.index(%d)\n", i, rQBuf.mvOut.size(), portID.index, PORT_IMGO.index);
                return MFALSE;
            }
            //
    #ifdef USE_IMAGEBUF_HEAP
            mpFrameMgr->enque(rQBuf.mvOut.at(i).mBuffer);
    #endif

            rQBufInfo.vBufInfo.resize(1);
            //rQBufInfo.vBufInfo[0].memID =       (MINT32)(imgo_memId[i]); //
    #ifdef USE_IMAGEBUF_HEAP
            rQBufInfo.vBufInfo[0].u4BufSize =   (MUINT32)rQBuf.mvOut.at(i).mBuffer->getBufSizeInBytes(0); //bytes
            rQBufInfo.vBufInfo[0].u4BufVA =     (MUINT32)rQBuf.mvOut.at(i).mBuffer->getBufVA(0); //
            rQBufInfo.vBufInfo[0].u4BufPA =     (MUINT32)rQBuf.mvOut.at(i).mBuffer->getBufPA(0); //
    #else
            rQBufInfo.vBufInfo[0].u4BufSize =   (MUINT32)rQBuf.mvOut.at(i).mSize;
            rQBufInfo.vBufInfo[0].u4BufVA =     (MUINT32)rQBuf.mvOut.at(i).mVa;
            rQBufInfo.vBufInfo[0].u4BufPA =     (MUINT32)rQBuf.mvOut.at(i).mPa;
    #endif

            //
            if (mpCamIOPipe)  {
                //bImdMode r no needed before isp start
                if ( false == mpCamIOPipe->enqueOutBuf(portID, rQBufInfo) ) {
                    CAM_LOGE("error:enqueOutBuf");
                    return MFALSE;
                }
            }
        }
    }
    else{
        #ifdef _USE_SEMA_
        //must deque-> enque -> ...
        CAM_LOGD("Block Enque Resource");
        ::sem_wait(&mSemEnQue);
        #endif

        /*---------------------------------------------------
        *   Frame based
        *   check if need to replace buffer
        ---------------------------------------------------*/
        for (MUINT32 i = 0; i < rQBuf.mvOut.size(); i++)
        {
            BufInfo pBuf = rQBuf.mvOut.at(i);
            portID.index = rQBuf.mvOut.at(i).mPortID.index;
            //82 only surpport IMGO
            if(portID.index != PORT_IMGO.index)
            {
                CAM_LOGE("enque fail: (%d/%d), port.Idx(%d)!=PORT_IMGO.index(%d)\n", i, rQBuf.mvOut.size(), portID.index, PORT_IMGO.index);
//                return MFALSE;
                ret = MFALSE;
                goto EQ_EXIT;
            }

            this->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::QueueMgr::eCmd_front,(MUINT32)&_buf,0);

    #ifdef USE_IMAGEBUF_HEAP
            //mark, because of imagebuffer will be unlock by MW, drv can't access imagebuffer here
//            if(_buf.mBuffer->getBufPA(0) == pBuf.mBuffer->getBufPA(0)){
            if(_buf.mPa == pBuf.mBuffer->getBufPA(0))
            {
    #else
            if(_buf.mPa == pBuf.mPa)
            {
    #endif
                //enque without exchange
                //
                rQBufInfo.vBufInfo.resize(1);
    #ifdef USE_IMAGEBUF_HEAP
                mpFrameMgr->enque(rQBuf.mvOut.at(i).mBuffer);
                //mark, because of imagebuffer will be unlock by MW, drv can't access imagebuffer here
                //rQBufInfo.vBufInfo[0].u4BufSize =   (MUINT32)_buf.mBuffer->getBufSizeInBytes(0); //bytes
                //rQBufInfo.vBufInfo[0].u4BufVA =       (MUINT32)_buf.mBuffer->getBufVA(0); //
                //rQBufInfo.vBufInfo[0].u4BufPA =       (MUINT32)_buf.mBuffer->getBufPA(0); //
                rQBufInfo.vBufInfo[0].u4BufSize = (MUINT32)_buf.mSize;
                rQBufInfo.vBufInfo[0].u4BufVA = (MUINT32)_buf.mVa;
                rQBufInfo.vBufInfo[0].u4BufPA = (MUINT32)_buf.mPa;
    #else
                rQBufInfo.vBufInfo[0].u4BufSize =   (MUINT32)_buf.mSize;
                rQBufInfo.vBufInfo[0].u4BufVA =       (MUINT32)_buf.mVa;
                rQBufInfo.vBufInfo[0].u4BufPA =       (MUINT32)_buf.mPa;
    #endif
                //
                if (mpCamIOPipe)  {
                    CAM_LOGD("enqueOutBuf");
                    //assign 1 for enqueCq is for enabe Immediate enque mode
                    if ( false == mpCamIOPipe->enqueOutBuf(portID, rQBufInfo) ) {
                        CAM_LOGE("enqueOutBuf");
//                        return MFALSE;
                        ret = MFALSE;
                        goto EQ_EXIT;
                    }
                }
                this->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::QueueMgr::eCmd_pop,0,0);
            }
            else{
                //exchange old dequed buffer
                ret = this->replace(_buf, pBuf);
            }
        }
    }

EQ_EXIT:
    //
    #ifdef _USE_SEMA_
    if(this->m_bStarted == MTRUE)
    {
        //must deque-> enque ...
        CAM_LOGD("Release Deque Resource");
        ::sem_post(&mSemDeQue);
    }
    #endif

    //CAM_LOGD("NormalPipe::enque -");
    return ret;
}


/*****************************************************************************
*
******************************************************************************/
MBOOL
NormalPipe::checkEnque(QBufInfo const& rQBuf)
{
    MBOOL ret = MTRUE;
    if (MTRUE == rQBuf.mvOut.empty())  {
        CAM_LOGE("error:queue empty");
        return MFALSE;
    }
    if (!this->mConfigDone) {
        CAM_LOGE("error:Not Rdy, Call configPipe first, SenIdx=%d", this->mpSensorIdx[0]);
        return MFALSE;
    }

#ifdef USE_IMAGEBUF_HEAP
    if ((NULL == mpFrameMgr)) {
        CAM_LOGE("enque mpFrameMgr==NULL ");
        return MFALSE;
    }
#endif
    //IO checking
    for(MUINT32 i = 0; i < rQBuf.mvOut.size(); i++){
        MSize _tgsize = this->mTgOut_size[CAM_TG_1];
        if( (rQBuf.mvOut.at(i).mPortID == PORT_IMGO)){

            if( (_tgsize.w != rQBuf.mvOut.at(i).FrameBased.mDstSize.w) || (_tgsize.h != rQBuf.mvOut.at(i).FrameBased.mDstSize.h) ){
                CAM_LOGI("Warning: imgo: out size(0x%x_0x%x) != tg size(0x%x_0x%x)\n",rQBuf.mvOut.at(i).FrameBased.mDstSize.w,rQBuf.mvOut.at(i).FrameBased.mDstSize.h,_tgsize.w,_tgsize.h);
            }
        }
        if( (rQBuf.mvOut.at(i).FrameBased.mCropRect.p.x + rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w) > _tgsize.w){
            CAM_LOGE("error: witdh: crop region out of grab window(0x%x_0x%x_0x%x) \n",\
            rQBuf.mvOut.at(i).FrameBased.mCropRect.p.x,rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w,_tgsize.w);
            return MFALSE;
        }
        if( (rQBuf.mvOut.at(i).FrameBased.mCropRect.p.y + rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h) > _tgsize.h){
            CAM_LOGE("error: height: crop region out of grab window(0x%x_0x%x_0x%x) \n",\
            rQBuf.mvOut.at(i).FrameBased.mCropRect.p.y,rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h,_tgsize.h);
            return MFALSE;
        }
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::replace(BufInfo const& bufOld, BufInfo const& bufNew)
{
//    CAM_LOGI("[replace] not supported");
    MBOOL ret = MTRUE;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QBufInfo rQBufInfo;
    portID.index = bufOld.mPortID.index;

    rQBufInfo.vBufInfo.resize(2);
                //rQBufInfo.vBufInfo[0].memID =       (MINT32)(imgo_memId[i]); //
#ifdef USE_IMAGEBUF_HEAP
    //mark, because of imagebuffer will be unlock by MW, drv can't access imagebuffer here
//    rQBufInfo.vBufInfo[0].u4BufSize =   (MUINT32)bufOld.mBuffer->getBufSizeInBytes(0); //bytes
//    rQBufInfo.vBufInfo[0].u4BufVA =     (MUINT32)bufOld.mBuffer->getBufVA(0); //
//    rQBufInfo.vBufInfo[0].u4BufPA =     (MUINT32)bufOld.mBuffer->getBufPA(0); //
    rQBufInfo.vBufInfo[0].u4BufSize =   (MUINT32)bufOld.mSize;
    rQBufInfo.vBufInfo[0].u4BufVA =     (MUINT32)bufOld.mVa;
    rQBufInfo.vBufInfo[0].u4BufPA =     (MUINT32)bufOld.mPa;
#else
    rQBufInfo.vBufInfo[0].u4BufSize =   (MUINT32)bufOld.mSize;
    rQBufInfo.vBufInfo[0].u4BufVA =     (MUINT32)bufOld.mVa;
    rQBufInfo.vBufInfo[0].u4BufPA =     (MUINT32)bufOld.mPa;
#endif

#ifdef USE_IMAGEBUF_HEAP
    rQBufInfo.vBufInfo[1].u4BufSize =   (MUINT32)bufNew.mBuffer->getBufSizeInBytes(0); //bytes
    rQBufInfo.vBufInfo[1].u4BufVA =     (MUINT32)bufNew.mBuffer->getBufVA(0); //
    rQBufInfo.vBufInfo[1].u4BufPA =     (MUINT32)bufNew.mBuffer->getBufPA(0); //
#else
    rQBufInfo.vBufInfo[1].u4BufSize =   (MUINT32)bufNew.mSize;
    rQBufInfo.vBufInfo[1].u4BufVA =     (MUINT32)bufNew.mVa;
    rQBufInfo.vBufInfo[1].u4BufPA =     (MUINT32)bufNew.mPa;
#endif

#ifdef USE_IMAGEBUF_HEAP
//mark, because of imagebuffer will be unlock by MW, drv can't access imagebuffer here
//    mpFrameMgr->deque(bufOld.mBuffer->getBufVA(0));
    mpFrameMgr->deque(bufOld.mVa);
    mpFrameMgr->enque(bufNew.mBuffer);
#endif

    CAM_LOGD("[MyEnQ:%d][replace] PortId(%d)+. PA(0x%x/0x%x)",\
         mpSensorIdx[0],portID.index, bufOld.mPa, bufNew.mPa);

    //
    if (mpCamIOPipe)  {
        //assign 1 for enqueCq is for enabe Immediate enque mode
        if ( false == mpCamIOPipe->enqueOutBuf(portID, rQBufInfo) ) {
            CAM_LOGE("replace");
            return MFALSE;
        }
    }
    this->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::QueueMgr::eCmd_pop,0,0);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::deque(QBufInfo& rQBuf, MUINT32 u4TimeoutMs)
{
    FUNCTION_LOG_START
    #ifdef _USE_SEMA_
    //must deque-> enque ...
    CAM_LOGD("Block Deque Resource");
    ::sem_wait(&mSemDeQue);
    #endif

    IImageBuffer *pframe= NULL;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    ///---------------------------------------------------------
    //Jackie, wait deque directly
    // (1). wait interrupt done
    //mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,NSImageio::NSIspio::EPIPEIRQ_PATH_DONE);

    // (2.1). dequeue buffer , Raw
//        CAM_LOGD("mpCamIOPipe->dequeOutBuf");

    NSImageio::NSIspio::PortID  portID;
    MUINT32 port_cnt =  rQBuf.mvOut.size();
    for (MUINT32 ii=0 ; ii < port_cnt; ii++)
    {
        portID.index = rQBuf.mvOut.at(ii).mPortID.index;
        //82 only surpport IMGO
        if(portID.index != PORT_IMGO.index)
        {
            CAM_LOGE("deque fail: (%d/%d), port.Idx(%d)!=PORT_IMGO.index(%d)\n", ii, rQBuf.mvOut.size(), portID.index, PORT_IMGO.index);
            return MFALSE;
        }
        //
        if(!mpCamIOPipe->dequeOutBuf(portID, rQTSBufInfo))
        {
            CAM_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMGO) fail ");
            return MFALSE;
        }

        if( rQTSBufInfo.vBufInfo.size() == 0 )
        {
            CAM_LOGE("deque failed");
            return MFALSE;
        }

        // (2.2). put buffer Raw in queue
//        CAM_LOGD("rQBuf.mvOut.push_back loop, rQTSBufInfo size(%d)", rQTSBufInfo.vBufInfo.size());
        for (MUINT32 i = 0; i < rQTSBufInfo.vBufInfo.size(); i++)
        {

            ResultMetadata result(MRect(MPoint(0, 0), MSize(0,0)), \
                                   0/*transform*/, \
                                   0 /*mMagicNum_hal*/, \
                                   0/*mMagicNum_tuning*/,\
                                   getTimeStamp_ns(rQTSBufInfo.vBufInfo.at(i).i4TimeStamp_sec,  rQTSBufInfo.vBufInfo.at(i).i4TimeStamp_us));

    #if 0//for debug
            CAM_LOGD("create buff: size(%d), VA(0x%x), PA(0x%x)",
                rQTSBufInfo.vBufInfo.at(i).u4BufSize,
                rQTSBufInfo.vBufInfo.at(i).u4BufVA,
                rQTSBufInfo.vBufInfo.at(i).u4BufPA);
    #endif


#ifdef USE_IMAGEBUF_HEAP
            pframe = mpFrameMgr->deque(rQTSBufInfo.vBufInfo.at(i).u4BufVA);
            if (NULL == pframe) {
                CAM_LOGE("[MyDeQ:%d]Fail: PortId(%d), 0==pframe, idx=%d, va=0x%x",\
                mpSensorIdx[0],portID.index,i, rQTSBufInfo.vBufInfo.at(i).u4BufVA);
                return MFALSE;
            }
            pframe->setTimestamp(rQTSBufInfo.vBufInfo.at(i).getTimeStamp_ns());
#endif

             BufInfo buff(rQBuf.mvOut.at(ii).mPortID,
                        pframe,
                        rQTSBufInfo.vBufInfo.at(i).u4BufSize,
                        rQTSBufInfo.vBufInfo.at(i).u4BufVA,
                        rQTSBufInfo.vBufInfo.at(i).u4BufPA,
                        result);

            rQBuf.mvOut.at(ii) = buff;

            /*---------------------------------------------------
            *   Frame based
            ---------------------------------------------------*/
            this->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::QueueMgr::eCmd_push,(MUINT32)&buff,0);
    #if 0// for debug
            int nDqSize;
            this->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::QueueMgr::eCmd_getsize,(MUINT32)&nDqSize,0);
            CAM_LOGD("after deque: portID(%d), mpDeQueue.size(%d)", _PortMap(portID.index), nDqSize);
    #endif
            /*---------------------------------------------------
            * Frame based (end)
            ---------------------------------------------------*/
        }

    #if 0//for debug
        CAM_LOGD("rQBuf.mvOut.size(%d)", rQBuf.mvOut.size());
        for (MUINT32 i = 0; i < rQBuf.mvOut.size(); i++)
        {
            CAM_LOGD("(VA,PA,Size,ID)(0x%x,0x%x,%d)",
                rQBuf.mvOut.at(i).mVa,
                rQBuf.mvOut.at(i).mPa, rQBuf.mvOut.at(i).mSize);
        }
    #endif
    }

    //must deque-> enque ...
    #ifdef _USE_SEMA_
    CAM_LOGD("Release Enque Resource");
    ::sem_post(&mSemEnQue);
    #endif

    FUNCTION_LOG_END
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/

MBOOL
NormalPipe::_setTgPixelAndDualMode(void)
{
    CAM_LOGI("[_setTgPixelAndDualMode] not supported");
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::configPipe(QInitParam const& vInPorts)

{
    //CAM_LOGD("+");
    MBOOL dump = 0;
    MBOOL ret = MTRUE;
    MUINT32 temp, SIdx;
    MUINT32 sensorDevIdx;
    char const* szCallerName = "NormalPipe";
//    IHalSensor::ConfigParam    sensorConfigParam[4];
    MUINT32                    mpTGToSensorIdx[4];
    EImageFormat               tgi_eImgFmt[4]; //Temp Used
    MBOOL     _b2pixEn = MFALSE;
    MBOOL     _rst = MTRUE;
    PortID     _twinPort;

    CAM_LOGD("configPipe+, SenIdx=%d", mpSensorIdx[0]);
    Mutex::Autolock lock(mCfgLock);

    /*********************************************************
    *   (0) Parameter Check
    *********************************************************/

    if (mConfigDone) {
        CAM_LOGE("NPipe::configPipe ReEntry, SenIdx=%d", mpSensorIdx[0]);
        return MFALSE;
    }

    ///1 Parameter Check
    if (mpEnabledSensorCnt != vInPorts.mSensorCfg.size()) {
        CAM_LOGE("NPipe::configPipe SensorCfgCnt is not match. (%d,%d)", mpEnabledSensorCnt, vInPorts.mSensorCfg.size());
        return MFALSE;
    }

    if (1 == mpEnabledSensorCnt) {
        if (mpSensorIdx[0] != vInPorts.mSensorCfg.at(0).index) {
            CAM_LOGE("NPipe::configPipe SensorIdx is not match. (%d,%d)", mpSensorIdx[0], vInPorts.mSensorCfg.at(0).index);
        }
    }
    else if (2 == mpEnabledSensorCnt) {
        CAM_LOGE("NPipe::configPipe NOT SUPPORT 2 Sensors");
    }
#if 0
    /*********************************************************
    *   (1) callbacks
    *********************************************************/
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    /*********************************************************
    *   (2) CQ config
    *********************************************************/
    ret = ret
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,
                                  (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                                   0,
                                   0
                                   )
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_TRIGGER_MODE,
                                  (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                                  (MINT32)NSImageio::NSIspio::EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                                  (MINT32)NSImageio::NSIspio::EPIPECQ_TRIG_BY_START
                                  )
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                  (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,
                                   0,
                                   0
                                  );
    if (!ret)
    {
        CAM_LOGE("Cammand queue config fail:%d", mpCamIOPipe->getLastErrorCode());
        return ret;
    }
#endif
    /*********************************************************
    * (3) Sensor ports
    *********************************************************/
    SensorPortInfo SensorPort;
    SensorPortInfo *pSensorPort = &SensorPort;
    pSensorPort->u4DeviceID = (vInPorts.mSensorCfg.at(0).index == 0)?(SENSOR_DEV_MAIN):(SENSOR_DEV_SUB);//halSensorDev_e
    pSensorPort->u4Scenario = vInPorts.mSensorCfg.at(0).scenarioId;//(ACDK_SCENARIO_ID_ENUM):0-3
    pSensorPort->u4Bitdepth = vInPorts.mBitdepth;
    pSensorPort->fgBypassDelay = MFALSE; //////TBD
    pSensorPort->fgBypassScenaio = vInPorts.mSensorCfg.at(0).isBypassScenario;
    pSensorPort->u4RawType = vInPorts.mPortInfo.at(0).mPureRaw;// The raw type, 0: pure raw, 1: pre-process raw
///
    ::memcpy(&mrSensorPortInfo, const_cast<SensorPortInfo*>(pSensorPort),sizeof(SensorPortInfo));
    MUINT32 u4SensorWidth = 0, u4SensorHeight = 0;
    MUINT32 u4RawPixelID = 0;
    EImageFormat eSensorFmt = eImgFmt_UNKNOWN;
    // (3.1) Sensor instance
    //if (NULL == mpSensorHal)
    //{
        mpSensorHal = SensorHal::createInstance();
        if (NULL == mpSensorHal)
        {
            CAM_LOGE("Null sensorHal object");
            return MFALSE;
        }
    //}
    //
    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(pSensorPort->u4DeviceID),
                                 SENSOR_CMD_SET_SENSOR_DEV,
                                 0,
                                 0,
                                 0
                                 );
    mpSensorHal->init();

    ret = querySensorInfo(  pSensorPort->u4DeviceID,
                            pSensorPort->u4Scenario,
                            pSensorPort->u4Bitdepth,
                            eSensorFmt, u4SensorWidth, u4SensorHeight, u4RawPixelID);

    CAM_LOGD("Sensor: (devID,scen)(%d,%d) (w,h,fmt,bits,pixID)(%d,%d,0x%x,%d,%d) (bpDelay,bpScen,rawType)(%d,%d,%d)",
                       pSensorPort->u4DeviceID, pSensorPort->u4Scenario,
                       u4SensorWidth, u4SensorHeight, eSensorFmt, pSensorPort->u4Bitdepth, u4RawPixelID,
                       pSensorPort->fgBypassDelay, pSensorPort->fgBypassScenaio, pSensorPort->u4RawType);
    //
    MUINT32 u4SensorStride = u4SensorWidth;
    if (eImgFmt_BAYER8 == eSensorFmt || eImgFmt_BAYER10 == eSensorFmt || eImgFmt_BAYER12 == eSensorFmt)
    {
        u4SensorStride = NSImageio::NSIspio::queryRawStride(eSensorFmt, u4SensorWidth);
    }

    /*********************************************************
    *   (4) create ICamIOPipe
    *********************************************************/
    if (mpCamIOPipe)
    {
        CAM_LOGE("CamioPipe is not NULL for 1st cfg.");
        return MFALSE;
    }
    mpCamIOPipe = ICamIOPipe::createInstance(mScenarioID, mScenarioFmt);
    if ( MFALSE == mpCamIOPipe->init() ) {
    CAM_LOGE("mpCamIOPipe->init Fail");
    return MFALSE;
    }

    /*********************************************************
    *   (1) callbacks
    *********************************************************/
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    /*********************************************************
    *   (2) CQ config
    *********************************************************/
    ret = ret
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,
                                  (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                                   0,
                                   0
                                   )
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_TRIGGER_MODE,
                                  (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                                  (MINT32)NSImageio::NSIspio::EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                                  (MINT32)NSImageio::NSIspio::EPIPECQ_TRIG_BY_START
                                  )
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                  (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,
                                   0,
                                   0
                                  );
    if (!ret)
    {
        CAM_LOGE("Cammand queue config fail:%d", mpCamIOPipe->getLastErrorCode());
        return ret;
    }

    /*********************************************************
    *   (5) In ports
    *********************************************************/
    vector<NSImageio::NSIspio::PortInfo const*> vCamIOInPorts;
    NSImageio::NSIspio::PortInfo tgi;
    tgi.eImgFmt = eSensorFmt;
    _NORMALPIPE_GET_TGI_PIX_ID(u4RawPixelID, tgi.eRawPxlID);
    tgi.u4ImgWidth = u4SensorWidth;
    tgi.u4ImgHeight = u4SensorHeight;
    tgi.u4Stride[0] = u4SensorStride;
    tgi.u4Stride[1] = 0;
    tgi.u4Stride[2] = 0;
    tgi.type = EPortType_Sensor;
    mu4DeviceID = pSensorPort->u4DeviceID;
    tgi.index = EPortIndex_TG1I;
    tgi.inout  = EPortDirection_In;
    tgi.u4BufSize  = (MUINT32)0;
    vCamIOInPorts.push_back(&tgi);
    this->mTgOut_size[CAM_TG_1].w = tgi.u4ImgWidth;
    this->mTgOut_size[CAM_TG_1].h = tgi.u4ImgHeight;

    // The raw type, 0: pure raw, 1: pre-process raw
    if(pSensorPort->u4RawType == 1)
    {
        ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_IMGO_RAW_TYPE,
                                      (MINT32)eRawImageType_PreProc,
                                       0,
                                       0
                                      );
    }
    /*********************************************************
    *   (5) Out ports
    *********************************************************/
    vector<NSImageio::NSIspio::PortInfo const*> vCamIOOutPorts;
    NSImageio::NSIspio::PortInfo imgo;
    NSImageio::NSIspio::PortInfo img2o;
    NSImageio::NSIspio::PortInfo *pOut = NULL;

    //CAM_LOGD("vInPorts.mPortInfo.size()=%d", vInPorts.mPortInfo.size());
    for (int i=0; i<vInPorts.mPortInfo.size();i++) {
        //Port info convert
        if ( PORT_IMGO == vInPorts.mPortInfo.at(i).mPortID ){
           pOut = &imgo;
           imgo.u4Offset = 0;
        }
        else if ( PORT_IMG2O == vInPorts.mPortInfo.at(i).mPortID ) {
           pOut = &img2o;
           mfgIsYUVPortON = MTRUE;
        }
        //
        pOut->index         = vInPorts.mPortInfo.at(i).mPortID.index;
        pOut->eImgFmt       = vInPorts.mPortInfo.at(i).mFmt;
        pOut->u4ImgWidth    = vInPorts.mPortInfo.at(i).mDstSize.w; //tg_grab_w;
        pOut->u4ImgHeight   = vInPorts.mPortInfo.at(i).mDstSize.h; //tg_grab_h;

        //no crop
        pOut->crop.y =      0;
        pOut->crop.h =      pOut->u4ImgHeight;
        pOut->u4Stride[ESTRIDE_1ST_PLANE] = u4SensorStride;
        pOut->u4Stride[ESTRIDE_2ND_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
        pOut->u4Stride[ESTRIDE_3RD_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
        pOut->type   = EPortType_Memory;
        pOut->inout  = EPortDirection_Out;

        if (0) {
             CAM_LOGD("DMAO[%d] sz=(%d,%d), Stride=%d",\
              pOut->index, pOut->u4ImgWidth, pOut->u4ImgHeight,pOut->u4Stride[0]);
        }

        vCamIOOutPorts.push_back(pOut);
    }

    /*********************************************************
    *   (6) configPipe
    *********************************************************/
   if ( MFALSE == mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts) ) {
        CAM_LOGE("mpCamIOPipe->configPipe Fail");
        return MFALSE;
    }
    //CAM_LOGD("================After mpCamIOPipe->configPipe ================");

#if 0//95
    /*********************************************************
    *   (7) set CQ trigger by EPIPECQ_TRIG_BY_PASS1_DONE
    *********************************************************/
    //CAM_LOGD("mpCamIOPipe->startCQ0 ");
#if PASS1_CQ_CONTINUOUS_MODE__
    mpCamIOPipe->startCQ0();

    //continuous mode
    mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_TRIGGER_MODE,
                            (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                            (MINT32)NSImageio::NSIspio::EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                            (MINT32)NSImageio::NSIspio::EPIPECQ_TRIG_BY_PASS1_DONE);
#else
    //cause CQ0 was configurred as SINGLE_IMMEDIATELY
    mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_TRIGGER_MODE,
                            (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                            (MINT32)NSImageio::NSIspio::EPIPECQ_TRIGGER_SINGLE_EVENT,
                            (MINT32)NSImageio::NSIspio::EPIPECQ_TRIG_BY_PASS1_DONE);
    mpCamIOPipe->startCQ0();//for rrz configframe before start
#endif


    //CAM_LOGD("[%d]irq(CAM_TG_1)(EPIPEIRQ_VSYNC) ", i);
    mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,
                    NSImageio::NSIspio::EPIPEIRQ_VSYNC);
#endif

    /*********************************************************
    *   (8) configSensor
    *********************************************************/
    ret = configSensor( pSensorPort->u4DeviceID,
                        pSensorPort->u4Scenario,
                        u4SensorWidth, u4SensorHeight,
                        pSensorPort->fgBypassDelay, pSensorPort->fgBypassScenaio, MTRUE);

    // The raw type, 0: pure raw, 1: pre-process raw  2: sensor test pattern
    if(pSensorPort->u4RawType == 2)
    {
        MINT32 u32Enable = 1;
        mpSensorHal->sendCommand(static_cast<halSensorDev_e>(pSensorPort->u4DeviceID),
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MINT32)&u32Enable,
                                           0,
                                           0);
        CAM_LOGD("Sensor Test Pattern");
    }

     /*********************************************************
     *  query skip frame to wait for stable
     *********************************************************/
    mu4SkipFrame = 0;
    if (pSensorPort->fgBypassDelay == MFALSE)
    {
        MUINT32 u4Mode = SENSOR_CAPTURE_DELAY;
        //
        mpSensorHal->sendCommand(static_cast<halSensorDev_e>(pSensorPort->u4DeviceID),
                                  static_cast<int>(SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT),
                                  reinterpret_cast<int>(&mu4SkipFrame),
                                  reinterpret_cast<int>(&u4Mode));
    }

    /*********************************************************
    *
    *********************************************************/
    mConfigDone = MTRUE;
    return MTRUE;
}

MBOOL
NormalPipe::configPipe(QInitParam const& vInPorts, NSImageio::NSIspio::EScenarioID const eSWScenarioID)
{
    mScenarioID = eSWScenarioID;
    return this->configPipe(vInPorts);
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::DummyFrame(QBufInfo const& rQBuf)
{
    CAM_LOGI("[DummyFrame] not supported");
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::configFrame(QFrameParam const& rQParam)
{
    CAM_LOGI("[configFrame] not supported");
    return MFALSE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalPipe::skipFrame(MUINT32 const u4SkipCount)
{
    CAM_LOGD(" + (u4SkipCount) = (%d)", u4SkipCount);
    MBOOL ret = MTRUE;
    for (MUINT32 i = 0; i < u4SkipCount ; i++)
    {
        //
        NSImageio::NSIspio::QTimeStampBufInfo rQTimeOutBufInfo;
        NSImageio::NSIspio::PortID rPortID(NSImageio::NSIspio::EPortType_Memory,
                                         NSImageio::NSIspio::EPortIndex_IMGO,
                                        1);
        //
        ret = mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);
        if (!ret)
        {
            CAM_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMGO) fail ");
            return ret;
        }

        //
        mpCamIOPipe->enqueOutBuf(rPortID, rQTimeOutBufInfo);

        if (mfgIsYUVPortON)
        {
            rPortID.index =  NSImageio::NSIspio::EPortIndex_IMG2O;
            ret = mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);
            //
            if (!ret)
            {
                CAM_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMG2O) fail ");
                return ret;
            }
            //
            mpCamIOPipe->enqueOutBuf(rPortID, rQTimeOutBufInfo);
        }
        //ret = mpCamIOPipe->irq( (NSImageio::NSIspio::EPipePass_PASS1_TG1) ,
        //                   NSImageio::NSIspio::EPIPEIRQ_VSYNC
        //                   );

    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
NormalPipe::attach(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType)
{
    CAM_LOGI("[attach] not supported");//mpCamIOPipe->registerIrq(irq)
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::wait(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout)
{
    //return immediately if have received signal alread; otherwise, wait right now.
    MBOOL ret = MTRUE;
    FUNCTION_LOG_START;
    switch(eType)
    {
        case EPipeSignal_SOF:
            ret = mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,NSImageio::NSIspio::EPIPEIRQ_VSYNC);
            break;
        case EPipeSignal_EOF:
            ret = mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1, NSImageio::NSIspio::EPIPEIRQ_PATH_DONE);
            break;
        default:
            break;
    }

    FUNCTION_LOG_END;
    return ret;
}



MBOOL
NormalPipe::wait(MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout)
{
    MBOOL ret = MTRUE;
    FUNCTION_LOG_START;
    switch(eType)
    {
        case EPipeSignal_SOF:
            ret = mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,NSImageio::NSIspio::EPIPEIRQ_VSYNC);
            break;
        case EPipeSignal_EOF:
            ret = mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1, NSImageio::NSIspio::EPIPEIRQ_PATH_DONE);
            break;
        default:
            break;
    }

    FUNCTION_LOG_END;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MUINT32
NormalPipe::getIspReg(::ESoftwareScenario scen)
{
    CAM_LOGD("getIspReg+");
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
NormalPipe::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    int  ret = MTRUE;

   //FUNCTION_LOG_START;
    CAM_LOGD("+ tid(%d), (cmd, arg1, arg2, arg3) = (%d, %d, %d, %d)",
                     gettid(), cmd, arg1, arg2, arg3) ;
    ECamIOPipeCmd eCmd = static_cast<ECamIOPipeCmd>(cmd);

    switch (cmd) {
        case ECamIOPipeCmd_QUERY_BAYER_RAW_SRIDE:
        {
            EImageFormat eFmt = static_cast<EImageFormat>(arg1);
            *(MINT32 *)arg3 = NSImageio::NSIspio::queryRawStride(eFmt, (MUINT32)arg2);
        }
        break;
        default:
            CAM_LOGE("not support command");
        break;
    }
    //FUNCTION_LOG_END;

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
static
MBOOL
checkDumpNormalPipe(void)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("cam.dumpnpipelog.enable", value, "0");
    int32_t enable = atoi(value);
    //
    if (enable & 0x1)
    {
        return MTRUE;
    }
    return MFALSE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalPipe::
configSensor(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4Width, MUINT32 const u4Height, MUINT32 const byPassDelay, MUINT32 const u4ByPassScenario,  MBOOL const fgIsContinuous)
{
    //FUNCTION_LOG_START;
    //MY_LOGD("+ (id, scen, w, h) = (%d, %d, %d, %d)",
    //            u4DeviceID, u4Scenario, u4Width, u4Height);
    //MY_LOGD(" configSensor(byPassDelay, u4ByPassScenario, fgIsContinuous) = (%d, %d, %d)",
    //                   byPassDelay, u4ByPassScenario, fgIsContinuous);

    MBOOL ret = MFALSE;
    //
    halSensorIFParam_t halSensorIFParam[2];
    ::memset(halSensorIFParam, 0, sizeof(halSensorIFParam_t) * 2);

    MUINT32 index = ((SENSOR_DEV_MAIN == u4DeviceID)||(SENSOR_DEV_ATV == u4DeviceID)) ? 0 : 1;

    halSensorIFParam[index].u4SrcW = u4Width;
    halSensorIFParam[index].u4SrcH = u4Height;
    halSensorIFParam[index].u4CropW = u4Width;
    halSensorIFParam[index].u4CropH = u4Height;
    halSensorIFParam[index].u4IsContinous = fgIsContinuous;
    halSensorIFParam[index].u4IsBypassSensorScenario = u4ByPassScenario;
#warning always set bypass sensor delay to true, due to hw not support delay frame currently.
    halSensorIFParam[index].u4IsBypassSensorDelay = MTRUE;
    halSensorIFParam[index].scenarioId = static_cast<ACDK_SCENARIO_ID_ENUM>(u4Scenario);
    ret = mpSensorHal->setConf(halSensorIFParam);

    //FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalPipe::
querySensorInfo(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4BitDepth, EImageFormat &eFmt,  MUINT32 &u4Width, MUINT32 &u4Height, MUINT32 & u4RawPixelID)
{
    //MY_LOGD("+(id,scen,bits) = (%d,%d,%d)", u4DeviceID, u4Scenario, u4BitDepth);

    MINT32 cmd = 0;
    switch (u4Scenario)
    {
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE;
        break;
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE;
        break;
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_VIDEO_RANGE;
        break;
    }

    // Sensor type
    SensorHalType_e eSensorType;
    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                             SENSOR_CMD_GET_SENSOR_TYPE,
                             reinterpret_cast<int>(&eSensorType),
                             0,
                             0
                            );

     CAM_LOGD("eSensorType = (%d)", eSensorType);

    //get sensor format info
    halSensorRawImageInfo_t rRawImgInfo;
    memset(&rRawImgInfo, 0, sizeof(rRawImgInfo));

    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                                          SENSOR_CMD_GET_RAW_INFO,
                                          (MINT32)&rRawImgInfo,
                                          1,
                                          0
                                         );

    _NORMALPIPE_GET_TGIFMT(eSensorType, rRawImgInfo.u1Order, u4BitDepth, u4RawPixelID, eFmt);

    //scenario format
    mScenarioFmt = _sensorTypeToPortFormat(eSensorType);

    // resolution
    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                             cmd,
                             (int)&u4Width,
                             (int)&u4Height,
                             0
                            );

    //MY_LOGD("-(fmt,w,h,pixID) = (0x%x,%d,%d,%d)", eFmt, u4Width, u4Height, u4RawPixelID);

    return MTRUE;
}

#ifdef USE_IMAGEBUF_HEAP
/******************************************************************************
 *
 ******************************************************************************/
void
NormalPipe::
FrameMgr::
init()
{
    mvFrameQ.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
void
NormalPipe::
FrameMgr::
uninit()
{
    mvFrameQ.clear();
}



/******************************************************************************
 *
 ******************************************************************************/
void
NormalPipe::
FrameMgr::
enque(IImageBuffer *pframe)
{
    Mutex::Autolock lock(mFrameMgrLock);
    //CAM_LOGD("FrameMgr::enque+, 0x%x/0x%x", pframe->getBufVA(0), pframe);
    mvFrameQ.add(pframe->getBufVA(0), pframe);
  //CAM_LOGD("FrameMgr::enque -");
}

/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
NormalPipe::
FrameMgr::
deque(int key) //key == va
{
    Mutex::Autolock lock(mFrameMgrLock);
    //CAM_LOGD("FrameMgr::deque+");
    IImageBuffer *pframe = mvFrameQ.valueFor(key);
    // exist
    if (pframe != NULL)
    {
        mvFrameQ.removeItem(key);
        //CAM_LOGD("FrameMgr::deque 0x%x-", key);
        return pframe;
    }
    //CAM_LOGD("FrameMgr::deque- (NULL)");
    return NULL;
}


#else

void
NormalPipe::
FrameMgr::
enque(const BufInfo& pframe)
{
}


NSCam::NSIoPipe::NSCamIOPipe::BufInfo*
NormalPipe::
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
NormalPipe:: QueueMgr:: init(void)
{
    this->m_v_pQueue.clear();
}

void
NormalPipe:: QueueMgr:: uninit(void)
{
    this->m_v_pQueue.clear();
}

MBOOL
NormalPipe:: QueueMgr:: sendCmd(QueueMgr_cmd cmd,MUINT32 arg1,MUINT32 arg2)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mQueueLock);
    switch(cmd){
        case eCmd_getsize:
            *(MUINT32*)arg1 = this->m_v_pQueue.size();
            break;
        case eCmd_push:
            this->m_v_pQueue.push_back(*(BufInfo*)(arg1));
            break;
        case eCmd_pop:
            this->m_v_pQueue.pop_front();
            break;
        case eCmd_front:
            *(BufInfo*)arg1 = this->m_v_pQueue.front();
            break;
        case eCmd_at:
            *(BufInfo*)arg2 = this->m_v_pQueue.at(arg1);
            break;
        case eCmd_end:
        default:
            CAM_LOGE("error: unsopported cmd:0x%x\n",cmd);
            ret = MFALSE;
            break;
    }
    return ret;
}


