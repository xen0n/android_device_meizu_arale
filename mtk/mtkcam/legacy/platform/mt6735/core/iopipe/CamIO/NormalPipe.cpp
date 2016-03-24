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

#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;

//
#include <mtkcam/Log.h>
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

#include "NormalPipe.h"
#include <mtkcam/hal/IHalSensor.h>
#include "PortMap.h"
#include "isp_datatypes.h"

//include is for following complie option. use following statement is because .so r not the same
#include "isp_function.h"
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

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

#define SENSOR_METADATA_IS_READY 0
#define SENSOR_TYPE_IS_RAW       1

#ifndef USING_PIP_8M //bound 5M sub-cam
    #define RAW_D_SENSOR_RESOLUTION_LIMT  5054400//2600x1944
#else //bound 8M sub-cam
    #define RAW_D_SENSOR_RESOLUTION_LIMT  8294400//3840x2160,16:9
#endif

static MBOOL checkDumpNormalPipe(void);

static MBOOL  gUpdateBQNumDone=false;
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
    IMetadata                       gPortCap_RRZO;
    IMetadata                       gPortCap_UFO;
};

//switch to camiopipe index
#define _NORMALPIPE_GET_TGIFMT(sensorIdx,fmt) {\
    switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType) {\
        case SENSOR_TYPE_RAW:\
            switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit) {\
                case RAW_SENSOR_8BIT:   fmt = NSCam::eImgFmt_BAYER8; break;\
                case RAW_SENSOR_10BIT:  fmt = NSCam::eImgFmt_BAYER10; break;\
                case RAW_SENSOR_12BIT:  fmt = NSCam::eImgFmt_BAYER12; break;\
                case RAW_SENSOR_14BIT:  fmt = NSCam::eImgFmt_BAYER14; break;\
                default: CAM_LOGE("Err sen raw fmt(%d) err\n", pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit); break;\
            }\
            break;\
        case SENSOR_TYPE_YUV:\
            switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder) {\
                case SENSOR_FORMAT_ORDER_UYVY: fmt = NSCam::eImgFmt_UYVY; break;\
                case SENSOR_FORMAT_ORDER_VYUY: fmt = NSCam::eImgFmt_VYUY; break;\
                case SENSOR_FORMAT_ORDER_YVYU: fmt = NSCam::eImgFmt_YVYU; break;\
                case SENSOR_FORMAT_ORDER_YUYV: fmt = NSCam::eImgFmt_YUY2; break;\
                default:    CAM_LOGE("Err sen yuv fmt err\n"); break;\
            }\
            break;\
        case SENSOR_TYPE_JPEG:\
            fmt = NSCam::eImgFmt_JPEG; break;\
            break;\
        default:\
            CAM_LOGE("Err sen type(%d) err\n", pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType);\
            break;\
    }\
}

//switch to camiopipe idx
#define _NORMALPIPE_GET_TGI_PIX_ID(sensorIdx,pix_id) do{\
    switch(pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder){\
        case SENSOR_FORMAT_ORDER_RAW_B: pix_id = ERawPxlID_B; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gb: pix_id = ERawPxlID_Gb; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gr: pix_id = ERawPxlID_Gr; break;\
        case SENSOR_FORMAT_ORDER_RAW_R: pix_id = ERawPxlID_R; break;\
        case SENSOR_FORMAT_ORDER_UYVY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_VYUY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YUYV: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YVYU: pix_id = (ERawPxlID)0; break;\
        default:    CAM_LOGE("Error Pix_id: sensorIdx=%d, sensorFormatOrder=%d", sensorIdx, pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder); break;\
    }\
    CAM_LOGD("SensorIdx=%d, sensorFormatOrder=%d", sensorIdx, pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder);\
}while(0);

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

#define _NORMALPIPE_GET_SENSOR_WIDTH(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureWidth)
#define _NORMALPIPE_GET_SENSOR_HEIGHT(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureHeight)


#define _NORMALPIPE_GET_SENSORCFG_CROP_W(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.w)
#define _NORMALPIPE_GET_SENSORCFG_CROP_H(sensorIdx) (pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.h)
#define _NORMALPIPE_GET_SENSOR_DEV_ID(sensorIdx)    (pAllSensorInfo->mSenInfo[sensorIdx].mDevId)

#define _NORMALPIPE_GET_TG_IDX(sensorIdx)     (pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo)
#define _NORMALPIPE_GET_PIX_MODE(sensorIdx)   (pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.pixelMode)


MUINT32 p1ConvSensorHalTG2Num(MUINT32 sensorHalTG) {

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
   return ISP_DRV_BASIC_CQ_NUM;
}

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
static platSensorsInfo *pAllSensorInfo = NULL;
static Mutex  NPipeGLock;//NormalPipe Global Lock

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
       MUINTPTR                 base_vAddr;
       MUINTPTR                 base_pAddr;
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
        MUINTPTR                 _base_vAddr = 0,
        MUINTPTR                 _base_pAddr = 0,
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
       #if 0
        if ((pSensorIndex[0] > 2) || (pSensorIndex[1] > 2)){
            CAM_LOGE("INormalPipe::createInstance InvalidSensorIdx = %d, %d", pSensorIndex[0], pSensorIndex[1]);
            return MFALSE;
        }

        pNPipe = pNormalPipe[NPIPE_Sensor_0And1];
        if (NULL == pNPipe)  {
            pNPipe = new NormalPipe(pSensorIndex, sensorCnt, szCallerName);
            pNPipe->mUserCnt = 0; //initialize
            pNormalPipe[NPIPE_Sensor_0And1] = pNPipe;
        }
        else {
            CAM_LOGD("pNormalPipe[%d]= 0x%8x\n", pSensorIndex[0], pNPipe);
        }
        pNPipe->mpSensorIdx[0] = pSensorIndex[0];
        pNPipe->mpSensorIdx[1] = pSensorIndex[1];
      #else
       CAM_LOGE("Not Support Create 2 sensor at the same time\n");
      #endif
   }

    //CAM_LOGD("mUserCnt=%d -",pNPipe->mUserCnt);
    //CAM_LOGD("createInstance-.(0x%8x)NPipe[%d]->mUserCnt(%d) ", pNPipe, pNPipe->mpSensorIdx[0], pNPipe->mUserCnt);

    return pNPipe;
}


INormalPipe*
INormalPipe::createInstance(MUINT32 SensorIndex,char const* szCallerName, MINT32 burstQnum)
{
   return createInstance(&SensorIndex, 1, szCallerName,burstQnum);
}


INormalPipe*
INormalPipe::createInstance( char const* szCallerName, MINT32 burstQnum)
{
   MUINT32 sidx = 0;
    return createInstance(&sidx, 1, szCallerName,burstQnum);
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

    //  RRZO
    {
        IMetadata& cap = gPortCap_RRZO;
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
            entry.push_back(PORT_RRZO, Type2Type<MINT32>());
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
            entry.push_back(RRZ_LIMITATION, Type2Type<MINT32>());
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
    //  UFO
    {

    }
    //
    //  Pipe
    {
        IMetadata& cap = gPipeCap;
        {   // out port
        IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_OUT_PORT_INFO);
        entry.push_back(gPortCap_IMGO, Type2Type<IMetadata>());
        entry.push_back(gPortCap_RRZO, Type2Type<IMetadata>());
        entry.push_back(gPortCap_UFO, Type2Type<IMetadata>());
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
unsigned long NormalPipe::mTwinEnable[CAM_SV_2+1][2] = {{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single},{0,ePxlMode_One_Single}};//record this & twin mode,0 for this, 1 for twinmode
MUINT32 NormalPipe::mEnablePath = 0;

MVOID
NormalPipe::destroyInstance(char const* szCallerName)
{
    int idx;
    char const* szNormalPipeName = "NormalPipe";

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

    if (NPIPE_Sensor_RSVD == idx) {
        CAM_LOGE("INormalPipe::destroyInstance. Can't find the entry");
        NormalPipeGUnLock();
        return;
    }

    if ((pNormalPipe[idx]->mUserCnt <= 0)){
        CAM_LOGD("INormalPipe::destroyInstance. No instance now");
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
        if  ( mpHalSensor == NULL )  {
            CAM_LOGE("NULL pHalSensor");
            NormalPipeGUnLock();
            return;
        }
        mpHalSensor->destroyInstance(szNormalPipeName);
        mpHalSensor= NULL;

        if(mpCamIOPipe)
        {
            mpCamIOPipe->destroyInstance();
            mpCamIOPipe = NULL;
        }
        CAM_LOGD("Destroy NPipe[%d]=0x%8x",idx, pNormalPipe[idx]);
        NPIPE_MEM_DEL(pNormalPipe[idx], sizeof(NormalPipe));
    }

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

    NormalPipeGUnLock();

    //CAM_LOGD("destroyInstance-.pNormalPipe =0x%8x/0x%8x/0x%8x", pNormalPipe[0], pNormalPipe[1], pNormalPipe[2]);

}


/******************************************************************************
 *
 ******************************************************************************/
NormalPipe::NormalPipe(char const* szCallerName)
    : IHalCamIO()
    , mpHalSensor(NULL)
    , mpCamIOPipe(NULL)
    , mpFrameMgr(NULL)
    , mpName(szCallerName)
    , mBurstQNum(1)
{
    mpHalSensor  = NULL;
    mpEnabledSensorCnt = 1;

    mbTwinEnable = MFALSE;
    mDumpNPipeFlag = MFALSE;
    mConfigDone = MFALSE;
    mRrzoOut_size.w = 0;
    mRrzoOut_size.h = 0;
    mFrmCntPerMnum = 0;
    mLastMnum = 0;
    m_bEISChkFlg = MFALSE;
    mImgoOut_size.w = 0;
    mImgoOut_size.h = 0;
    mPureRaw =MFALSE;
    mDynSwtRawType = MTRUE;
}


NormalPipe::NormalPipe(MUINT32* pSensorIdx, MUINT32 SensorIdxCnt, char const* szCallerName, MINT32 burstQnum)
    : IHalCamIO()
    , mpHalSensor(NULL)
    , mpCamIOPipe(NULL)
    , mpFrameMgr(NULL)
    , mpName(szCallerName)
    , mBurstQNum(1)
{
    mpHalSensor  = NULL;
    mpEnabledSensorCnt = SensorIdxCnt;

    mbTwinEnable = MFALSE;
    mDumpNPipeFlag = MFALSE;
    mConfigDone = MFALSE;
    mRrzoOut_size.w = 0;
    mRrzoOut_size.h = 0;
    mFrmCntPerMnum = 0;
    mLastMnum = 0;
    //update burstQnumber if mw member update it, otherwise use default value 1
    CAM_LOGD("income BQNum(%d)",burstQnum);
    mBurstQNum=burstQnum;
    mImgoOut_size.w = 0;
    mImgoOut_size.h = 0;
    mPureRaw =MFALSE;
    mDynSwtRawType = MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::start()
{
    if(this->mConfigDone == MFALSE)
    {
        CAM_LOGE("error: mConfigDone=0\n");
        return MFALSE;
    }
    bool ret = MTRUE;

    if (mpCamIOPipe)
    {
        ret = mpCamIOPipe->start();
    }

    //start sensor ?????

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::stop()
{

    bool ret = MTRUE;

    CAM_LOGD("++++ stop[%d] ++++",mpSensorIdx[0]);


    if (mpCamIOPipe)
    {
        ret = mpCamIOPipe->stop();
        //for twin mode chk under 2 sensor
        if(mTwinEnable[CAM_TG_1][0] == (unsigned long)this){
            mTwinEnable[CAM_TG_1][0] = mTwinEnable[CAM_TG_1][1] = ePxlMode_One_Single;
            mEnablePath -= NPIPE_PASS_PASS1;
        }
        else if(mTwinEnable[CAM_TG_2][0] == (unsigned long)this){
            mTwinEnable[CAM_TG_2][0] = mTwinEnable[CAM_TG_2][1] = ePxlMode_One_Single;
            mEnablePath -= NPIPE_PASS_PASS1_D;
        }
        else if(mTwinEnable[CAM_SV_1][0] == (unsigned long)this){
            mEnablePath -= NPIPE_PASS_CAMSV;
        }
        else if(mTwinEnable[CAM_SV_2][0] == (unsigned long)this){
            mEnablePath -= NPIPE_PASS_CAMSV_D;
        }

        if( !mEnablePath)
        {
            gUpdateBQNumDone=false;
        }
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
    mDumpNPipeFlag = checkDumpNormalPipe();

#if 0
    for (idx = 0; idx < NPIPE_Sensor_RSVD; idx++){
       if (this == pNormalPipe[idx]){
          break;
       }
    }

    if (NPIPE_Sensor_RSVD == idx) {
        CAM_LOGE("NormalPipe::init. Can't find the entry");
        return MFALSE;
    }

    if (pNormalPipe[idx]->mUserCnt > 0)  {
        CAM_LOGD("NormalPipe::init.User>0");
       return MTRUE;
    }
#endif

    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->init();
    }


#ifdef USE_IMAGEBUF_HEAP
    if (!mpFrameMgr) {
        NPIPE_MEM_NEW(mpFrameMgr,FrameMgr(),sizeof(FrameMgr));
    }
    mpFrameMgr->init();
#endif

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::uninit()
{
    MUINT32 idx;
    bool ret = MTRUE;
    MUINT32 SIdx;

    CAM_LOGD("uninit+,  SenIdx=%d", mpSensorIdx[0]);

    //Mutex::Autolock lock(mCfgLock);

    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->uninit();
        mConfigDone = MFALSE;
    }

    //CAM_LOGD("after mpCamIOPipe->uninit");

    //20140305: Middleware(Allan) will control the powerOn/Off life cycle.
    //if (mpHalSensor)  {
    //    mpHalSensor->powerOff("NormalPipe", mpEnabledSensorCnt, mpSensorIdx);
    //}

#if 0
    for (int i = 0; i < mpEnabledSensorCnt; i++)  {

        SIdx = mpSensorIdx[i];

        NPipeGLock();
        if (pAllSensorInfo) {
            memset (&pAllSensorInfo->mSenInfo[SIdx].mConfig, 0, sizeof(IHalSensor::ConfigParam));
            pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINT32)NULL;
            CAM_LOGD("Reset mSenInfo[%d], sz=%d ", SIdx, sizeof(IHalSensor::ConfigParam));
        }
        else {
            CAM_LOGE("NormalPipe::uninit: pAllSensorInfo NULL");
        }
        NPipeGUnLock();
    }
#else
    for (int i = 0; i < mpEnabledSensorCnt; i++)  {

        SIdx = mpSensorIdx[i];

        NormalPipeGLock();
        if (pAllSensorInfo) {
            pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINT32)NULL;
        }
        else {
            CAM_LOGE("NormalPipe::uninit: pAllSensorInfo NULL");
        }
        NormalPipeGUnLock();
    }

#endif

    {
        CamManager* pCamMgr = CamManager::getInstance();
        pCamMgr->setAvailableHint(MTRUE);
    }

#ifdef USE_IMAGEBUF_HEAP

    if (mpFrameMgr)  {
        mpFrameMgr->uninit();
    }
#endif

    CAM_LOGD("uninit-,  SenIdx=%d", mpSensorIdx[0]);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::enque(QBufInfo const& rQBuf)
{
    bool ret = MTRUE;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QBufInfo rQBufInfo;
    //
    //CAM_LOGD("NormalPipe::enque +");
    if (MTRUE == rQBuf.mvOut.empty())  {
        CAM_LOGD("enquer empty");
        return MFALSE;
    }
    if (!mConfigDone) {
        CAM_LOGE("[enque]Not Rdy, Call configPipe first, SenIdx=%d", mpSensorIdx[0]);
        return MFALSE;
    }

#ifdef USE_IMAGEBUF_HEAP
    if ((NULL == mpFrameMgr)) {
        CAM_LOGE("enque mpFrameMgr==NULL ");
        return MFALSE;
    }
#endif

    //CAM_LOGD("NormalPipe::enque  rQBuf.mvOut.size=%d ",  rQBuf.mvOut.size());
    Mutex::Autolock lock(mEnQLock);

    for (MUINT32 i = 0; i < rQBuf.mvOut.size(); i++)
    {
        //image buffer data structure pool for MW
        //CAM_LOGD("before mpFrameMgr->enque");
        //
        portID.index = rQBuf.mvOut.at(i).mPortID.index;
        //Raw-D path, convert IMGO to IMGO-D
        if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0]))){
             if (PORT_IMGO == rQBuf.mvOut.at(i).mPortID){
                 portID.index = PORT_IMGO_D.index;
             }
             if (PORT_RRZO == rQBuf.mvOut.at(i).mPortID){
                 portID.index = PORT_RRZO_D.index;
             }
        }

        //
#ifdef USE_IMAGEBUF_HEAP
        mpFrameMgr->enque(rQBuf.mvOut.at(i).mBuffer);
#endif

        rQBufInfo.vBufInfo.resize(1);
        //rQBufInfo.vBufInfo[0].memID =       (MINT32)(imgo_memId[i]); //
#ifdef USE_IMAGEBUF_HEAP
        rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)rQBuf.mvOut.at(i).mBuffer->getBufSizeInBytes(0); //bytes
        rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINTPTR)rQBuf.mvOut.at(i).mBuffer->getBufVA(0); //
        rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINTPTR)rQBuf.mvOut.at(i).mBuffer->getBufPA(0); //
#else
        rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)rQBuf.mvOut.at(i).mSize;
        rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINTPTR)rQBuf.mvOut.at(i).mVa;
        rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINTPTR)rQBuf.mvOut.at(i).mPa;
#endif
        rQBufInfo.vBufInfo[0].header  =     rQBuf.mvOut.at(i).mMetaData.mPrivateData;
        rQBufInfo.vBufInfo[0].mBufIdx = rQBuf.mvOut.at(i).mBufIdx; //used when replace buffer

        //
        if ((portID.index == PORT_RRZO.index) ||(portID.index == PORT_RRZO_D.index)) {
        CAM_LOGD("[MyEnQ:%d]PortId(%d),sz/va/pa/idx(%d,0x%x,0x%x,0x%x)",\
                    mpSensorIdx[0],portID.index, \
                    rQBufInfo.vBufInfo[0].u4BufSize[0],\
                    rQBufInfo.vBufInfo[0].u4BufVA[0],\
                    rQBufInfo.vBufInfo[0].u4BufPA[0],\
                    rQBufInfo.vBufInfo[0].mBufIdx);
        }
        //
        if (mpCamIOPipe)  {
            if ( false == mpCamIOPipe->enqueOutBuf(portID, rQBufInfo) ) {
                CAM_LOGE("enqueOutBuf");
                return false;
            }
        }
    }
    //
    //CAM_LOGD("NormalPipe::enque -");
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::
replace(BufInfo const& bufOld, BufInfo const& bufNew)
{
    bool ret = MTRUE;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QBufInfo rQBufInfo;

    portID.index = bufOld.mPortID.index;

    if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx[0])){
         if (PORT_IMGO.index == bufOld.mPortID.index){
             portID.index = PORT_IMGO_D.index;
         }
         if (PORT_RRZO.index == bufOld.mPortID.index){
             portID.index = PORT_RRZO_D.index;
         }
    }


    rQBufInfo.vBufInfo.resize(2);
#ifdef USE_IMAGEBUF_HEAP
    rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)bufOld.mBuffer->getBufSizeInBytes(0); //bytes
    rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINTPTR)bufOld.mBuffer->getBufVA(0); //
    rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINTPTR)bufOld.mBuffer->getBufPA(0); //
#else
    rQBufInfo.vBufInfo[0].u4BufSize[0] =   (MUINT32)bufOld.mSize;
    rQBufInfo.vBufInfo[0].u4BufVA[0] =     (MUINTPTR)bufOld.mVa;
    rQBufInfo.vBufInfo[0].u4BufPA[0] =     (MUINTPTR)bufOld.mPa;
#endif
    rQBufInfo.vBufInfo[0].header  =     bufOld.mMetaData.mPrivateData;
    rQBufInfo.vBufInfo[0].mBufIdx =     bufOld.mBufIdx; //used when replace buffer

#ifdef USE_IMAGEBUF_HEAP
    rQBufInfo.vBufInfo[1].u4BufSize[0] =   (MUINT32)bufNew.mBuffer->getBufSizeInBytes(0); //bytes
    rQBufInfo.vBufInfo[1].u4BufVA[0] =     (MUINTPTR)bufNew.mBuffer->getBufVA(0); //
    rQBufInfo.vBufInfo[1].u4BufPA[0] =     (MUINTPTR)bufNew.mBuffer->getBufPA(0); //
#else
    rQBufInfo.vBufInfo[1].u4BufSize[0] =   (MUINT32)bufNew.mSize;
    rQBufInfo.vBufInfo[1].u4BufVA[0] =     (MUINTPTR)bufNew.mVa;
    rQBufInfo.vBufInfo[1].u4BufPA[0] =     (MUINTPTR)bufNew.mPa;
#endif
    rQBufInfo.vBufInfo[1].header  =     bufNew.mMetaData.mPrivateData;
    rQBufInfo.vBufInfo[1].mBufIdx =     bufNew.mBufIdx; //used when replace buffer

    //CAM_LOGD("[MyEnQ:%d][replace] PortId(%d)+. Idx(0x%x/0x%x)",\
    //       mpSensorIdx[0],portID.index, bufOld.mBufIdx, bufNew.mBufIdx);

    mpFrameMgr->deque(bufOld.mBuffer->getBufVA(0));
    mpFrameMgr->enque(bufNew.mBuffer);

    if (mpCamIOPipe)  {
        if ( false == mpCamIOPipe->enqueOutBuf(portID, rQBufInfo) ) {
            CAM_LOGE("replace");
            return false;
        }
    }

    return ret;
}



/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::deque(QBufInfo& rQBuf, MUINT32 u4TimeoutMs)
{
    //CAM_LOGD("[MyDeQ:%d]+++++++++++++++++++++++",mpSensorIdx[0]);
    bool ret = MTRUE;
    vector<NSImageio::NSIspio::PortID> QportID; //used for CamIOPipe
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    MUINT32 deqBufCnt = 0;
    IImageBuffer *pframe= NULL;

    MUINT32 port_cnt =  rQBuf.mvOut.size();


    if ((!mpCamIOPipe) || (0 == port_cnt)) {
       CAM_LOGE("[deque] Error: !mpCamIOPipe=%x || rQBuf.mvOut.size=%d", mpCamIOPipe, port_cnt);
       return MFALSE;
    }
    if (!mConfigDone) {
        CAM_LOGE("[deque]Not Rdy, Call configPipe first, SenIdx=%d", mpSensorIdx[0]);
        return MFALSE;
    }

    //CAM_LOGD("+, BufInfoCnt=%d", port_cnt);
    Mutex::Autolock lock(mDeQLock);

    NSImageio::NSIspio::PortID  portID;    //used for CamIOPipe

    for (MUINT32 ii=0 ; ii<port_cnt ; ii++ ) {
        //
        portID.index = rQBuf.mvOut.at(ii).mPortID.index;
        if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx[0])){
             if (PORT_IMGO == rQBuf.mvOut.at(ii).mPortID){
                 portID.index = PORT_IMGO_D.index;
             }
             if (PORT_RRZO == rQBuf.mvOut.at(ii).mPortID){
                 portID.index = PORT_RRZO_D.index;
             }
        }
        if (MFALSE == mpCamIOPipe->dequeOutBuf(portID, rQTSBufInfo) ) {
            if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx[0])){
                CAM_LOGE("[MyDeQ] deque[PorID=%d] Mnum=0x%x Fail", portID.index, mLastMnum);
            }
            if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx[0])){
                CAM_LOGE("[MyDeQ]D deque[PorID=%d] Mnum=0x%x Fail", portID.index, mLastMnum);
            }
            return MFALSE;
        } else {
            //CAM_LOGD("[MyDeQ]deque[PorID=%d] OK, cnt = %d", portID.index, rQTSBufInfo.vBufInfo.size());
        }
        //

        if ( rQTSBufInfo.vBufInfo.size() >= 1 )  {

            deqBufCnt += rQTSBufInfo.vBufInfo.size();

            for (MUINT32 idx = 0; idx < rQTSBufInfo.vBufInfo.size(); idx++)  {
                 BufInfo buff;
                 ResultMetadata result;

                stNPipe_BUF_INFO *pBufHeader = NULL;
                stNPipe_RRZ_INFO *pRRzInfo   = NULL;
                pBufHeader = (stNPipe_BUF_INFO*)rQTSBufInfo.vBufInfo.at(idx).header;
                pRRzInfo   = &pBufHeader->rrz_info;
                if ((portID.index == PORT_RRZO.index) ||(portID.index == PORT_RRZO_D.index) || (portID.index == PORT_IMGO.index))
                    CAM_LOGD("[MyDeQ:%d]PortId(%d),va=0x%x,BufHeader=0x%x,PixelId=%x, MNum=0x%x, rrzDstSz=%d,%d,rrzSrcSz=%d,%d",mpSensorIdx[0], \
                             portID.index,\
                             rQTSBufInfo.vBufInfo.at(idx).u4BufVA[0], pBufHeader,pBufHeader->img_pxl_id,\
                             pBufHeader->m_num_0,\
                             pRRzInfo->dstW, pRRzInfo->dstH,pRRzInfo->srcW, pRRzInfo->srcH);

                if ((portID.index == PORT_RRZO.index) ||(portID.index == PORT_RRZO_D.index)) {
                    mFrmCntPerMnum++;
                    if (mFrmCntPerMnum > 20) {
                        CAM_LOGI("[3ASync:%d]No new MNum from Mw for %d frames now", mpSensorIdx[0],mFrmCntPerMnum);
                    }
                }

#ifdef USE_IMAGEBUF_HEAP
                 pframe = mpFrameMgr->deque((MINTPTR)rQTSBufInfo.vBufInfo.at(idx).u4BufVA[0]);
                 if (NULL == pframe) {
                      CAM_LOGE("[MyDeQ:%d]Fail: PortId(%d), 0==pframe, idx=%d, va=0x%x",\
                        mpSensorIdx[0],portID.index,idx, rQTSBufInfo.vBufInfo.at(idx).u4BufVA[0]);
                      return MFALSE;
                 }
                 pframe->setTimestamp(rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_ns());
#endif

                 //result.mCropRect = rQTSBufInfo.vBufInfo.at(idx).crop_win;
                 result.mDstSize = rQTSBufInfo.vBufInfo.at(idx).DstSize;
                 if(rQBuf.mvOut.at(ii).mPortID == PORT_IMGO){
                    result.mCrop_s = MRect(MPoint(0,0),MSize(rQTSBufInfo.vBufInfo.at(idx).img_w,rQTSBufInfo.vBufInfo.at(idx).img_h));
                    result.mCrop_d = rQTSBufInfo.vBufInfo.at(idx).crop_win;
                }
                else if(rQBuf.mvOut.at(ii).mPortID == PORT_RRZO){
                    result.mCrop_s = rQTSBufInfo.vBufInfo.at(idx).crop_win;
                    result.mCrop_d = MRect(MPoint(0,0),MSize(result.mDstSize.w,result.mDstSize.h));
                }
                else
                    CAM_LOGE("unsupported dmao:0x%x\n",rQBuf.mvOut.at(ii).mPortID.index);
                 result.mTransform = 0;
                 result.mMagicNum_hal = rQTSBufInfo.vBufInfo.at(idx).m_num_0;
                 result.mMagicNum_tuning = rQTSBufInfo.vBufInfo.at(idx).m_num_1;
                 result.mRawType = rQTSBufInfo.vBufInfo.at(idx).raw_type;
                 result.mTimeStamp = getTimeStamp_ns(rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_sec,  rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_us);
                 result.mPrivateData = rQTSBufInfo.vBufInfo.at(idx).header;
                 result.mPrivateDataSize = rQTSBufInfo.vBufInfo.at(idx).headerSize;

                buff.mPortID = rQBuf.mvOut.at(ii).mPortID;
                buff.mBuffer = pframe;
                buff.mMetaData = result;
                buff.mSize = rQTSBufInfo.vBufInfo.at(idx).u4BufSize[0];
                buff.mVa = rQTSBufInfo.vBufInfo.at(idx).u4BufVA[0];
                buff.mPa = rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0];

                 rQBuf.mvOut.at(ii) = buff;
              }
        }
        else {
            CAM_LOGD("[MyDeQ:%d] ###Fail### dequeSz(%d)<1",mpSensorIdx[0], rQTSBufInfo.vBufInfo.size());
        }
        //

    }
    if (!deqBufCnt){
        //CAM_LOGE("[MyDeQ] deqBufCnt == 0!!!!!!!!!!!!!1");
        return MFALSE;
    }

    //CAM_LOGD("[MyDeQ]===========================================");

    return ret;
}


MBOOL
NormalPipe::_setTgPixelAndDualMode(void)
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

    //CAM_LOGD("_setTgPixelAndDualMode+ Sidx=[%d, %d], tg=[%d, %d],PixelMode=[%d,%d]",\
    //         SIdx, S2Idx, tg, tg2, pixelCnt, pixelCnt2);



    if (mpEnabledSensorCnt > 1) {
        CAM_LOGE("Error: NotSupportCfg 2 Sensors");
    }

    /*if (mpEnabledSensorCnt == 1)*/
    {
        MBOOL   enable2PixelTwin = MFALSE;

        if ((tg == CAM_TG_1)) {
            // TG2 not support Twin Mode

            if ( (NSCam::SENSOR_TYPE_RAW  == _NORMALPIPE_GET_SENSORTYPE(SIdx)) ||
                 (NSCam::SENSOR_TYPE_JPEG == _NORMALPIPE_GET_SENSORTYPE(SIdx)) ){
                enable2PixelTwin = _NORMALPIPE_GET_PIX_MODE(SIdx) == TWO_PIXEL_MODE ? 1:0;

                //when open IMGO only & pure raw => set to single ISP
                if(enable2PixelTwin && this->mPureRaw == MTRUE && mDynSwtRawType == MFALSE &&
                   mImgoOut_size.w > 0 && mRrzoOut_size.w == 0)
                    enable2PixelTwin = MFALSE;
            }
            else if (NSCam::SENSOR_TYPE_YUV  == _NORMALPIPE_GET_SENSORTYPE(SIdx)) { // YUV uses one isp
                enable2PixelTwin = MFALSE;
            }
        }
        else if(tg == CAM_TG_2){//if tg2 with 2 pix mode , it will return error at configpipe()
            if ( (NSCam::SENSOR_TYPE_RAW  == _NORMALPIPE_GET_SENSORTYPE(SIdx)) ||
                 (NSCam::SENSOR_TYPE_JPEG == _NORMALPIPE_GET_SENSORTYPE(SIdx)) ){
                enable2PixelTwin = _NORMALPIPE_GET_PIX_MODE(SIdx) == TWO_PIXEL_MODE ? 1:0;

                //when open IMGO only & pure raw => set to single ISP
                if(enable2PixelTwin && this->mPureRaw == MTRUE && mDynSwtRawType == MFALSE &&
                   mImgoOut_size.w > 0 && mRrzoOut_size.w == 0)
                    enable2PixelTwin = MFALSE;
            }
            else if (NSCam::SENSOR_TYPE_YUV  == _NORMALPIPE_GET_SENSORTYPE(SIdx)) { // YUV uses one isp
                enable2PixelTwin = MFALSE;
            }
        }
        else if ((tg == CAM_SV_1) || (tg == CAM_SV_2))  {

            enable2PixelTwin = _NORMALPIPE_GET_PIX_MODE(SIdx) == TWO_PIXEL_MODE ? 1:0;
        }
        mTwinEnable[tg][0] = (unsigned long)this;
        if (enable2PixelTwin){
            if (0 == SIdx){
                if ((pNormalPipe[1]) && (_NORMALPIPE_GET_PIX_MODE(1) == TWO_PIXEL_MODE) &&
                     ((NSCam::SENSOR_TYPE_RAW  == _NORMALPIPE_GET_SENSORTYPE(1)) ||
                      (NSCam::SENSOR_TYPE_JPEG == _NORMALPIPE_GET_SENSORTYPE(1)))){
                    CAM_LOGE("Error: 2 sensors both enable Two Pixel Mode");
                    enable2PixelTwin = MFALSE;
                    return MFALSE;
                }
            }
            if (1 == SIdx){
                if ((pNormalPipe[0]) && (_NORMALPIPE_GET_PIX_MODE(0) == TWO_PIXEL_MODE) &&
                     ((NSCam::SENSOR_TYPE_RAW  == _NORMALPIPE_GET_SENSORTYPE(0)) ||
                      (NSCam::SENSOR_TYPE_JPEG == _NORMALPIPE_GET_SENSORTYPE(0)))){
                    CAM_LOGE("Error: 2 sensors both enable Two Pixel Mode");
                    enable2PixelTwin = MFALSE;
                    return MFALSE;
                }
            }
        }
        mbTwinEnable =  enable2PixelTwin;
        if(enable2PixelTwin){
            if(tg <= CAM_TG_2){
                mTwinEnable[tg][1] = ePxlMode_Two_Twin;
            }
        }
        else if (MFALSE == enable2PixelTwin && 1 == pixelCnt){//yuv, or always pure raw
            if (tg <= CAM_TG_2){
                mTwinEnable[tg][1] = ePxlMode_Two_Single;
            }
        }

        CAM_LOGD("1sensor tg[%d], PixelMode=[%d], ISPMode=[%d]", tg, pixelCnt, mTwinEnable[tg][1]);
    }

#if 0
    else if (mpEnabledSensorCnt == 2) {

        if ((pNormalPipe[0]) || (pNormalPipe[1])){
            CAM_LOGE("Enable 2sensors, but the prev config is not released...");
        }

        mpTGModeArray[tg]  = ePxlMode_One_Single;
        mpTGModeArray[tg2] = ePxlMode_One_Single;
        mbTwinEnable = MFALSE;

        //CAM_LOGD("2sensor: mpTGModeArray=[%d,%d]", tg, mpTGModeArray[tg], mpTGModeArray[tg2]);
    }
    else if (mpEnabledSensorCnt == 3) {
        mpTGModeArray[tg]  = ePxlMode_One_Single;
        mpTGModeArray[tg2] = ePxlMode_One_Single;
        mbTwinEnable = MFALSE;

    }
#endif

    return MTRUE;
}

#define DUMP_SENSOR_INFO()                                 \
do    {                                                    \
        MUINT32 SCnt = 2;                                  \
        _sensorInfo_t  *pSInfo = NULL;                     \
        for (int i = 0; i < mpEnabledSensorCnt; i++) {     \
            pSInfo = &pAllSensorInfo->mSenInfo[i];         \
            CAM_LOGD("[%d]Type=%d, DevId=%d, tg=%d",       \
                     i,                                    \
                     pSInfo->mTypeforMw, pSInfo->mDevId,_NORMALPIPE_GET_TG_IDX( mpSensorIdx[i] ));\
        }\
    } while(0);

/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalPipe::Reset()
{
    CAM_LOGI("NormalPipe: ESD flow start +");

    char const* szCallerName = "NormalPipe";
    if(this->mConfigDone){
        CAM_LOGE("error:Normalpipe Config already Done");
    }

    if (this->mpHalSensor == NULL){
         this->mpHalSensor = pAllSensorInfo->mSList->createSensor(szCallerName, this->mpEnabledSensorCnt, this->mpSensorIdx);
         CAM_LOGE("error: ESD flow error, HalSensor NULL obj");
    }

    CAM_LOGI("Reset+, SenIdx=%d", this->mpSensorIdx[0]);

    //we need to power off sensor and power on the sensor.
    //Power off
    this->mpHalSensor->powerOff(szCallerName, this->mpEnabledSensorCnt, this->mpSensorIdx);
    //Power On
    this->mpHalSensor->powerOn(szCallerName, this->mpEnabledSensorCnt, this->mpSensorIdx);
    //Configure the sensor again.
    this->mpHalSensor->configure(this->mpEnabledSensorCnt, &this->m_sensorConfigParam);

    for (int i = 0; i < this->mpEnabledSensorCnt; i++)
    {
        switch ( _NORMALPIPE_GET_TG_IDX((this->mpSensorIdx[i])) )
        {
           case CAM_TG_1:
               if(MFALSE == this->mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC,(int)ISP_DRV_IRQ_USER_ISPDRV))
               {
                    CAM_LOGE("wait VSYNC fail");
                    return MFALSE;
               }
               break;
           case CAM_TG_2:
               if(MFALSE == this->mpCamIOPipe->irq(EPipePass_PASS1_TG1_D, EPIPEIRQ_VSYNC,(int)ISP_DRV_IRQ_USER_ISPDRV))
               {
                    CAM_LOGE("wait VSYNC fail");
                    return MFALSE;
               }
               break;
           default:
               break;
        }

    }

    this->mConfigDone = MTRUE;

    CAM_LOGI("-");
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
    MUINT   sensorScenId;         //  sensor scenario ID.
    char const* szCallerName = "NormalPipe";
    MUINT32                    mpTGToSensorIdx[4];
    EImageFormat               tgi_eImgFmt[4]; //Temp Used
    ISP_QUERY_RST QueryRst;
    //rrzo width is 2304 whether it is twin mode or single raw
#if 0
    MBOOL   _b2pixEn = MFALSE;
    MBOOL   _rst = MTRUE;
    PortID  _twinPort;
#endif
    CAM_LOGD("configPipe+, SenIdx=%d", mpSensorIdx[0]);
    Mutex::Autolock lock(mCfgLock);

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
#if 0
        if ((mpSensorIdx[0] != vInPorts.mSensorCfg.at(0).index) && \
            (mpSensorIdx[0] != vInPorts.mSensorCfg.at(1).index)) {
            CAM_LOGE("NPipe::configPipe SensorIdx0 is not match. (%d,%d,%d)", mpSensorIdx[0], vInPorts.mSensorCfg.at(0).index, vInPorts.mSensorCfg.at(1).index);
        }
        if ((mpSensorIdx[1] != vInPorts.mSensorCfg.at(0).index) && \
            (mpSensorIdx[1] != vInPorts.mSensorCfg.at(1).index)) {
            CAM_LOGE("NPipe::configPipe SensorIdx1 is not match. (%d,%d,%d)", mpSensorIdx[1], vInPorts.mSensorCfg.at(0).index, vInPorts.mSensorCfg.at(1).index);
        }
#endif
        CAM_LOGE("NPipe::configPipe NOT SUPPORT 2 Sensors");
    }


    ///2 Get Info
    memset(&this->m_sensorConfigParam, 0, sizeof(IHalSensor::ConfigParam));
    memset(mpTGToSensorIdx,   0, sizeof(MUINT32) * 4);

    for (int i = 0; i < mpEnabledSensorCnt; i++)  {

        SIdx = mpSensorIdx[i];

        if (NULL != pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner) {
            CAM_LOGD("[Warning] Overwrite %d-SensorCfg...", SIdx);
        }
        pAllSensorInfo->mSenInfo[SIdx].mConfig = vInPorts.mSensorCfg.at(i);
        pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINTPTR)this;

        this->m_sensorConfigParam = vInPorts.mSensorCfg.at(i); //used for sensorCfg
        sensorScenId = this->m_sensorConfigParam.scenarioId;
        _NORMALPIPE_GET_TGIFMT(SIdx, tgi_eImgFmt[SIdx]);

        //enforce seninf/tg in 2-pixmode if rrzo over RRZ_LIMITATION is required
        for (int j=0; j<vInPorts.mPortInfo.size();j++) {
            if (( PORT_RRZO == vInPorts.mPortInfo.at(j).mPortID )|| ( PORT_RRZO_D == vInPorts.mPortInfo.at(j).mPortID )) {
                if(vInPorts.mPortInfo.at(j).mDstSize.w > RRZ_LIMITATION){
                    this->m_sensorConfigParam.twopixelOn = MTRUE;
                    break;
                }
            }
         }
    }


    if (!mpHalSensor){
         mpHalSensor = pAllSensorInfo->mSList->createSensor(szCallerName, mpEnabledSensorCnt, mpSensorIdx);
    } else {
        CAM_LOGE("[Error] ConfigPipe ReEntry...");
    }

    if (!mpHalSensor){
        CAM_LOGE("mpHalSensor Fail");
        return MFALSE;
    }


    //20140305: Middleware(Allan) will control the powerOn/Off life cycle.
    //mpHalSensor->powerOn(szCallerName, mpEnabledSensorCnt, mpSensorIdx);
    mpHalSensor->configure(mpEnabledSensorCnt, &this->m_sensorConfigParam);
    for (int i = 0; i < mpEnabledSensorCnt; i++) {

        SIdx = mpSensorIdx[i];
        //
        if(vInPorts.mRawType){
            CAM_LOGD("Enable sensor test pattern(DevId = %d)\n",pAllSensorInfo->mSenInfo[SIdx].mDevId);
            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId,SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,(MUINTPTR)&vInPorts.mRawType,0,0);
        }
        //
        mpHalSensor->querySensorDynamicInfo(pAllSensorInfo->mSenInfo[SIdx].mDevId, &pAllSensorInfo->mSenInfo[SIdx].mDInfo);
        if (_NORMALPIPE_GET_TG_IDX(SIdx) <= 0){
            CAM_LOGE("Error: mpDynamicInfo.TgInfo <= 0,SIdx(0x%x),mDevId(0x%x),tg_idx(%d)", \
                      SIdx, \
                      pAllSensorInfo->mSenInfo[SIdx].mDevId, \
                      _NORMALPIPE_GET_TG_IDX(SIdx));
            return MFALSE;
        }
        mpTGToSensorIdx[_NORMALPIPE_GET_TG_IDX(SIdx) - 1] = SIdx;
        //rrzo width is 2304 whether it is twin mode or single raw
#if 0
        //enforce seninf/tg in 2-pixmode if rrzo over RRZ_LIMITATION is required
        for (int j=0; j<vInPorts.mPortInfo.size();j++) {
            if (( PORT_RRZO == vInPorts.mPortInfo.at(j).mPortID )|| ( PORT_RRZO_D == vInPorts.mPortInfo.at(j).mPortID )) {
                if(vInPorts.mPortInfo.at(j).mDstSize.w > RRZ_LIMITATION){
                    CAM_LOGI("configPipe Enable TwinMode due to RRZOut.W > 2304!!!!!(%d)\n",_NORMALPIPE_GET_TG_IDX((mpSensorIdx[0])));

                    _b2pixEn = MTRUE;
                    if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0]))){
                        _twinPort = PORT_RRZO;
                    }
                    else {//CAM_TG_2 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0])))
                        _twinPort = PORT_RRZO_D;
                    }
                    break;
                }
            }
         }
         //double chk if 2pixEnOn & 2nd rrzo also enabled
         //MW must guarantee no rrzo > 2304 && 2nd raw sensor is needed.
         if(_b2pixEn){
              for (int j=0; j<vInPorts.mPortInfo.size();j++) {
                  if(_twinPort == PORT_RRZO){
                       if ( PORT_RRZO_D == vInPorts.mPortInfo.at(j).mPortID ) {
                            CAM_LOGE("isp support no rrzo.w > 2304 && rrzo_d enable\n");
                            _rst = MFALSE;
                            break;
                       }
                  }else{//_twinport is PORT_RRZO_D
                       if ( PORT_RRZO == vInPorts.mPortInfo.at(j).mPortID ) {
                            CAM_LOGE("isp support no rrzo_d.w > 2304 && rrzo enable\n");
                            _rst = MFALSE;
                            break;
                       }
                  }
              }
         }
         if(_rst == MFALSE)
             return _rst;
#endif
    }


    vector<PortInfo const*> vCamIOInPorts;
    vector<PortInfo const*> vCamIOOutPorts;
    PortInfo tgi[NPIPE_MAX_ENABLE_SENSOR_CNT];
    PortInfo imgo;
    PortInfo rrzo;
    PortInfo imgo_d;
    PortInfo rrzo_d;
    PortInfo camsv_imgo,camsv_imgo_d;
    PortInfo *pOut = NULL;
    MBOOL    enable2PixelTwin = MFALSE;
    MUINT32  tg1SIdx = 0, tg2SIdx = 0, tg = 0;

    tg1SIdx = mpTGToSensorIdx[0];
    tg2SIdx = mpTGToSensorIdx[1];

    bool fpsL120=false; //fps is larger than 120 or not

    if (!mpCamIOPipe)
    {
        //create ICamIOPipe
        MUINT32 sType = _NORMALPIPE_GET_SENSORTYPE(tg1SIdx);
        mpCamIOPipe = ICamIOPipe::createInstance(eDrvScenario_CC, _sensorTypeToPortFormat(sType), (MINT8 const*)mpName);
        if ( MFALSE == mpCamIOPipe->init() ) {
             CAM_LOGE("mpCamIOPipe->init Fail");
             return MFALSE;
        }

        //CAM_LOGD("%d-Sensors 's mpCamIOPipe: 0x%x", mpEnabledSensorCnt, mpCamIOPipe);

        //get IMGO RawType, check it is pure raw of not
        //mxxxxOut_size is for _setTgPixelAndDualMode(), check IMGO only
        for (int j=0; j<vInPorts.mPortInfo.size();j++) {
            if (( PORT_RRZO == vInPorts.mPortInfo.at(j).mPortID )|| ( PORT_RRZO_D == vInPorts.mPortInfo.at(j).mPortID )) {
                mRrzoOut_size = vInPorts.mPortInfo.at(j).mDstSize;
            }
            if (( PORT_IMGO == vInPorts.mPortInfo.at(j).mPortID )|| ( PORT_IMGO_D == vInPorts.mPortInfo.at(j).mPortID )){
                this->mPureRaw = vInPorts.mPortInfo.at(j).mPureRaw;
                mImgoOut_size = vInPorts.mPortInfo.at(j).mDstSize;
            }
         }
        this->mDynSwtRawType = vInPorts.m_DynamicRawType;
        CAM_LOGD("SenIdx=%d, DynamicRawType=%d, PureRaw=%d", mpSensorIdx[0], this->mDynSwtRawType, this->mPureRaw);

        //determine 2 pixel Mode or not for Raw
        NormalPipeGLock();
        if (!_setTgPixelAndDualMode()) { //call this before setting tg information
            CAM_LOGE("Error: determine 2PixelInfo fail");
            NormalPipeGUnLock();
            return MFALSE;
        }

        if((CAM_TG_2 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx[0])) && (MFALSE == mbTwinEnable)) //CAM_TG_2
        {
            if (0x1 == mpSensorIdx[0])
            {
                if (RAW_D_SENSOR_RESOLUTION_LIMT < _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx[0])*_NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx[0]))
                {
                    NormalPipeGUnLock();
                    CAM_LOGE("Error: Width: 0x%x, Height: 0x%x", _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx[0]), _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx[0]));
#ifndef USING_PIP_8M
                    CAM_LOGE("Error: ISP-D doesn't enable twin mode, and sensor resultion is bigger than 5M size ");
#else
                    CAM_LOGE("Error: ISP-D doesn't enable twin mode, and sensor resultion is bigger than 8M size ");
#endif
                    return MFALSE;
                }
            }
            else
            {
                if (RAW_D_SENSOR_RESOLUTION_LIMT < _NORMALPIPE_GET_SENSOR_WIDTH(mpSensorIdx[0])*_NORMALPIPE_GET_SENSOR_HEIGHT(mpSensorIdx[0]))
                {
                    NormalPipeGUnLock();
                    CAM_LOGE("Error: Width: 0x%x, Height: 0x%x", _NORMALPIPE_GET_SENSOR_WIDTH(mpSensorIdx[0]), _NORMALPIPE_GET_SENSOR_HEIGHT(mpSensorIdx[0]));
#ifndef USING_PIP_8M
                    CAM_LOGE("Error: ISP-D doesn't enable twin mode, and sensor resultion is bigger than 5M size-main cam");
#else
                    CAM_LOGE("Error: ISP-D doesn't enable twin mode, and sensor resultion is bigger than 8M size-main cam");
#endif
                    return MFALSE;
                }
            }
        }
        NormalPipeGUnLock();
        //
        for (int i = 0; i < mpEnabledSensorCnt;i++)
        {
            //in/out port param
            MUINT32 sensorIdx = mpSensorIdx[i];

            if (NSCam::SENSOR_TYPE_YUV == _NORMALPIPE_GET_SENSORTYPE(sensorIdx)) {
                if ((mTwinEnable[_NORMALPIPE_GET_TG_IDX(sensorIdx)][1] != ePxlMode_Two_Single) && (mTwinEnable[_NORMALPIPE_GET_TG_IDX(sensorIdx)][1] != ePxlMode_One_Single)){
                    CAM_LOGE("error: YUV sensor not use one/two-pixel mode and one isp, sensorIdx=%d, tg=%d, PixelMode=%d", sensorIdx, _NORMALPIPE_GET_TG_IDX(sensorIdx), mTwinEnable[_NORMALPIPE_GET_TG_IDX(sensorIdx)][1]);
                    mTwinEnable[_NORMALPIPE_GET_TG_IDX(sensorIdx)][1] = mTwinEnable[_NORMALPIPE_GET_TG_IDX(sensorIdx)][0] = ePxlMode_One_Single;//reset to avoid affect next time operation
                    return MFALSE;
                }
            }
            else {
                if(_NORMALPIPE_GET_TG_IDX(sensorIdx) == CAM_TG_1){
                    if(mTwinEnable[CAM_TG_2][1] == ePxlMode_Two_Twin){
                        CAM_LOGE("error:twin mode is enable in TG_2, can't open TG_1");
                        mTwinEnable[CAM_TG_2][1] = mTwinEnable[CAM_TG_2][0] = ePxlMode_One_Single;//reset to avoid affect next time operation
                        return MFALSE;
                    }else if(mTwinEnable[CAM_TG_1][1] == ePxlMode_Two_Twin){
                        if(mEnablePath & NPIPE_PASS_PASS1_D){
                            CAM_LOGE("error:TG_2 already opened, can't open TG_1 with Twin mode\n");
                            mTwinEnable[CAM_TG_1][1] = mTwinEnable[CAM_TG_1][0] = ePxlMode_One_Single;//reset to avoid affect next time operation
                            return MFALSE;
                        }
                    }
                }
                else if(_NORMALPIPE_GET_TG_IDX(sensorIdx) == CAM_TG_2){
                    if(mTwinEnable[CAM_TG_1][1] == ePxlMode_Two_Twin){
                        CAM_LOGE("error:twin mode is enable in TG_1, can't open TG_2");
                        mTwinEnable[CAM_TG_1][1] = mTwinEnable[CAM_TG_1][0] = ePxlMode_One_Single;//reset to avoid affect next time operation
                        return MFALSE;
                    }else if(mTwinEnable[CAM_TG_2][1] == ePxlMode_Two_Twin){
                        if(mEnablePath & NPIPE_PASS_PASS1){
                            CAM_LOGE("error:TG_1 already opened, can't open TG_2 with Twin mode\n");
                            mTwinEnable[CAM_TG_2][1] = mTwinEnable[CAM_TG_2][0] = ePxlMode_One_Single;//reset to avoid affect next time operation
                            return MFALSE;
                        }
                    }
                }
            }
            tg = _NORMALPIPE_GET_TG_IDX(sensorIdx);
            _NOMRALPIPE_CVT_TGIDX(sensorIdx, tgi[i].index);
            tgi[i].ePxlMode    = (NSImageio::NSIspio::EPxlMode)mTwinEnable[tg][1];
            tgi[i].eImgFmt     = tgi_eImgFmt[sensorIdx];
            _NORMALPIPE_GET_TGI_PIX_ID(sensorIdx, tgi[i].eRawPxlID);
            tgi[i].u4ImgWidth  = _NORMALPIPE_GET_SENSORCFG_CROP_W(sensorIdx);
            tgi[i].u4ImgHeight = _NORMALPIPE_GET_SENSORCFG_CROP_H(sensorIdx);
            tgi[i].type        = EPortType_Sensor;
            tgi[i].inout       = EPortDirection_In;
            tgi[i].tgFps       = vInPorts.mSensorCfg.at(i).framerate;
            vCamIOInPorts.push_back(&tgi[i]);
            mTgOut_size[tg].w = tgi[i].u4ImgWidth;
            mTgOut_size[tg].h = tgi[i].u4ImgHeight;
            //
            if (1) {

               CAM_LOGD("SensorTg=%d, tgIdx=%d, devID=%d, eRawPxlID=%d,PixelMode=%d W/H=[%d, %d],fps(%d)",\
                 tg, tgi[i].index, pAllSensorInfo->mSenInfo[mpSensorIdx[i]].mDevId, tgi[i].eRawPxlID, tgi[i].ePxlMode, tgi[i].u4ImgWidth, tgi[i].u4ImgHeight, tgi[i].tgFps);
            }

            if(tgi[i].tgFps >= 120)
            {
                fpsL120=true;
            }

        }

    //update cq table num if mw member update burst queue number
        //we do not realloc virtual buffer when config for second sensor while another one is running
        CAM_LOGD("mBurstQNum(%d),gUpdateBQNumDone(%d)",mBurstQNum,gUpdateBQNumDone);
         //return err if user try to enable slow motion(update burstQNum) while another sensor is running
        if(gUpdateBQNumDone && fpsL120)
        {
            CAM_LOGE("do not support slow motion in two sensor mode, mBQNum(%d),gUpdateBQNumDone(%d)",mBurstQNum,gUpdateBQNumDone);
            return MFALSE;
        }
        //
        if(!gUpdateBQNumDone)
        {
            mpCamIOPipe->sendCommand(EPIPECmd_UPDATE_BURST_QUEUE_NUM, mBurstQNum,0,0);
            gUpdateBQNumDone=true;
        }
        //
        //CAM_LOGD("vInPorts.mPortInfo.size()=%d", vInPorts.mPortInfo.size());
        for (int i=0; i<vInPorts.mPortInfo.size();i++) {

            //Port info convert
            if ( PORT_IMGO == vInPorts.mPortInfo.at(i).mPortID ) {
                //CAM_LOGD("[%d] PORT_IMGO", i);

                if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0]))){
                   pOut = &imgo;
                   mImgoOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                   CAM_LOGD("configPipe PORT_IMGO mImgOut_size:%d,%d", mImgoOut_size.w, mImgoOut_size.h);
                   pOut->index   = vInPorts.mPortInfo.at(i).mPortID.index;
                }

                //Raw-D path, convert IMGO to IMGO-D
                if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0]))){
                   //vInPorts.mPortInfo.at(i).mPortID = PORT_IMGO_D;
                   pOut = &imgo_d;
                   mImgodOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                   pOut->index    = PORT_IMGO_D.index;
                }
            }
            else if ( PORT_RRZO == vInPorts.mPortInfo.at(i).mPortID ) {

                if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0]))){
                    pOut = &rrzo;
                    mRrzoOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                    CAM_LOGD("configPipe PORT_RRZO, mRrzoOut_size:%d,%d, Crop:%d,%d,%d,%d", mRrzoOut_size.w, mRrzoOut_size.h);
                    pOut->index    = vInPorts.mPortInfo.at(i).mPortID.index;
                    m_bEISChkFlg = MTRUE;
                }

                //Raw-D path, convert IMGO to IMGO-D
                if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0]))){
                   pOut = &rrzo_d;
                   mRrzoOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                   pOut->index   = PORT_RRZO_D.index;
                }
            }
            else if ( PORT_CAMSV_IMGO == vInPorts.mPortInfo.at(i).mPortID ) {
                pOut = &camsv_imgo;
                mCamSvImgoOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                pOut->index    = vInPorts.mPortInfo.at(i).mPortID.index;
            }
            else if ( PORT_CAMSV2_IMGO == vInPorts.mPortInfo.at(i).mPortID ) {
                pOut = &camsv_imgo_d;
                mCamSv2ImgoOut_size = vInPorts.mPortInfo.at(i).mDstSize;
                pOut->index    = vInPorts.mPortInfo.at(i).mPortID.index;
            }

            //
            pOut->eImgFmt        = vInPorts.mPortInfo.at(i).mFmt;
            pOut->u4PureRaw      = vInPorts.mPortInfo.at(i).mPureRaw;
            pOut->u4PureRawPak    = vInPorts.mPortInfo.at(i).mPureRawPak;
            pOut->u4ImgWidth     = vInPorts.mPortInfo.at(i).mDstSize.w; //tg_grab_w;
            //
            //add for dmao x-size query in pipeimp
            if(NSCam::SENSOR_TYPE_YUV == _NORMALPIPE_GET_SENSORTYPE(mpSensorIdx[0]))
                pOut->ePxlMode = NSImageio::NSIspio::ePxlMode_Two_Single;
            else
                pOut->ePxlMode = (this->mbTwinEnable == 1) ? (NSImageio::NSIspio::ePxlMode_Two_Twin):(NSImageio::NSIspio::ePxlMode_One_Single);
            if ((pOut->index  == EPortIndex_RRZO)||(pOut->index  == EPortIndex_RRZO_D)) {
                if ( (0 == mbTwinEnable) && (/*CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL*/RRZ_LIMITATION < pOut->u4ImgWidth) ) {
                    CAM_LOGE("rrzo out width exceed (%d > 2304)",pOut->u4ImgWidth);
                    CAM_LOGE("ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!");
                    return MFALSE;
                }
                if(ISP_QuerySize(pOut->index,ISP_QUERY_X_PIX,pOut->eImgFmt,pOut->u4ImgWidth,QueryRst) == 0){
                    CAM_LOGE(" fmt(0x%x) | dma(0x%x) err\n",pOut->eImgFmt,pOut->index);
                    return MFALSE;
                }else {
                    if(QueryRst.x_pix != pOut->u4ImgWidth) {
                        CAM_LOGE(" rrzo x resolution r in-valid (cur:%d   valid:%d)\n",pOut->u4ImgWidth,QueryRst.x_pix);
                        return MFALSE;
                    }
                }
                //CAM_LOGD("NormalPipe: PORT_RRZO u4ImgWidth=%d", pOut->u4ImgWidth);
            }

            pOut->u4ImgHeight     = vInPorts.mPortInfo.at(i).mDstSize.h; //tg_grab_h;
            pOut->crop1.x         = vInPorts.mPortInfo.at(i).mCropRect.p.x; //0;
            pOut->crop1.y         = vInPorts.mPortInfo.at(i).mCropRect.p.y; //0;
            pOut->crop1.floatX    = 0;
            pOut->crop1.floatY    = 0;
            pOut->crop1.w         = vInPorts.mPortInfo.at(i).mCropRect.s.w; //tg_grab_w
            pOut->crop1.h         = vInPorts.mPortInfo.at(i).mCropRect.s.h;;//tg_grab_h
            //pOut->u4Stride[ESTRIDE_1ST_PLANE] = queryRawStride((MUINT32)pOut->eImgFmt, pOut->u4ImgWidth);
            pOut->u4Stride[ESTRIDE_1ST_PLANE] = vInPorts.mPortInfo.at(i).mStride[0];
            if(ISP_QuerySize(pOut->index,ISP_QUERY_STRIDE_BYTE,pOut->eImgFmt,pOut->u4ImgWidth,QueryRst) == 0){
                CAM_LOGE(" fmt(0x%x) | dma(0x%x) err\n",pOut->eImgFmt,pOut->index);
                return MFALSE;
            }else {
                if(QueryRst.stride_byte != pOut->u4Stride[ESTRIDE_1ST_PLANE]){
                    CAM_LOGD("cur dma(%d) stride:0x%x, recommanded:0x%x",pOut->index,pOut->u4Stride[ESTRIDE_1ST_PLANE],QueryRst.stride_byte);
                }
            }
            //if ((pOut->index  == EPortIndex_RRZO)||(pOut->index  == EPortIndex_RRZO_D)) {
                //pOut->u4Stride[ESTRIDE_1ST_PLANE] = pOut->u4ImgWidth;
            //}

            pOut->u4Stride[ESTRIDE_2ND_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
            pOut->u4Stride[ESTRIDE_3RD_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
            pOut->type   = EPortType_Memory;
            pOut->inout  = EPortDirection_Out;

            if (0) {
                 CAM_LOGD("DMAO[%d] Crop=[%d,%d,%d,%d], sz=(%d,%d), Stride=%d",\
                  pOut->index, pOut->crop1.x, pOut->crop1.y, pOut->crop1.w, pOut->crop1.h, \
                  pOut->u4ImgWidth, pOut->u4ImgHeight,pOut->u4Stride[0]);
            }

            //
            if ((NSCam::SENSOR_TYPE_YUV == _NORMALPIPE_GET_SENSORTYPE(i)) &&
                (!(pOut->index == EPortIndex_RRZO) || (pOut->index == EPortIndex_RRZO_D)))
            {
                //RRZO doesn't support YUV sensor
                vCamIOOutPorts.push_back(pOut);
            }
            else
            {
                vCamIOOutPorts.push_back(pOut);
            }
        }


        //DUMP_SENSOR_INFO();
        //CQ
        for (int i = 0; i < mpEnabledSensorCnt; i++)
        {
            switch ( _NORMALPIPE_GET_TG_IDX((mpSensorIdx[i])) )
            {
               case CAM_TG_1:
                   mEnablePath |= NPIPE_PASS_PASS1;
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL     ,(MINT32)EPIPE_PASS1_CQ0,0,0);
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                   CAM_LOGD("CAM_TG_1: SetCQ0_Channel&Mode");
                   break;
               case CAM_TG_2:
                   mEnablePath |= NPIPE_PASS_PASS1_D;
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_D_CQ_CHANNEL     ,(MINT32)EPIPE_PASS1_CQ0_D,0,0);
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_D_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                   CAM_LOGD("CAM_TG_2: SetCQ0D_Channel&Mode");
                   break;
                case CAM_SV_1:
                    mEnablePath |= NPIPE_PASS_CAMSV;
                    break;
                case CAM_SV_2:
                    mEnablePath |= NPIPE_PASS_CAMSV_D;
                    break;
               default:
                   break;
            }
        }

        //config pipe
        if ( MFALSE == mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts) ) {
            CAM_LOGE("mpCamIOPipe->configPipe Fail");
            return MFALSE;
        }
        //CAM_LOGD("================After mpCamIOPipe->configPipe ================");
        //add this for camera v1, bypass imgo magic number when tuning sync,unless v1 have imgo cropping request!
        this->mpCamIOPipe->sendCommand(EPIPECmd_SET_IMGOBYPASS,MTRUE,0,0);

        for (int i = 0; i < mpEnabledSensorCnt; i++)
        {
            switch ( _NORMALPIPE_GET_TG_IDX((mpSensorIdx[i])) )
            {
               case CAM_TG_1:
                   mpCamIOPipe->startCQ0();
                   //CAM_LOGD("mpCamIOPipe->startCQ0 ");
                   #if PASS1_CQ_CONTINUOUS_MODE__
                   //continuous mode
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                   #else
                   //cause CQ0 was configurred as SINGLE_IMMEDIATELY
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                   mpCamIOPipe->startCQ0();//for rrz configframe before start
                   #endif
                   //CAM_LOGD("[%d]irq(CAM_TG_1)(EPIPEIRQ_VSYNC) ", i);
                   //KK's wait irq has bug, and we fix it in L. we mark it in L,
                   // because of performance issue
                   //if(MFALSE == mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC,(int)ISP_DRV_IRQ_USER_ISPDRV))
                   //{
                   //   CAM_LOGE("after cfg, wait VSYNC fail");
                   //   if(this->Reset() == MFALSE)
                   //   {
                   //       CAM_LOGE("after cfg, Reset");
                   //       return MFALSE;
                   //   }
                   //}
                   break;
               case CAM_TG_2:
                   mpCamIOPipe->startCQ0_D();
                   //CAM_LOGD("mpCamIOPipe->startCQ0_D ");
                   #if PASS1_CQ_CONTINUOUS_MODE__
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                   #else
                   mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_SINGLE_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                   mpCamIOPipe->startCQ0_D();//for rrz configframe before start
                   #endif
                   //CAM_LOGD("[%d]irq(CAM_TG_2)(EPIPEIRQ_VSYNC) ", i);
                   //KK's wait irq has bug, and we fix it in L. we mark it in L,
                   // because of performance issue
                   //if(MFALSE == mpCamIOPipe->irq(EPipePass_PASS1_TG1_D, EPIPEIRQ_VSYNC,(int)ISP_DRV_IRQ_USER_ISPDRV))
                   //{
                   //     CAM_LOGE("after cfg, wait VSYNC fail");
                   //     return MFALSE;
                   //}
                   break;
               default:
                   break;
            }

        }

    }
    else {
        CAM_LOGE("CamioPipe is not NULL for 1st cfg.");
    }

    {
        CamManager* pCamMgr = CamManager::getInstance();
        pCamMgr->setAvailableHint((0 == mbTwinEnable)? MTRUE : MFALSE);
    }

    this->mConfigDone = MTRUE;
    //CAM_LOGD("-");
    return ret;
}



/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::configFrame(QFrameParam const& rQParam)
{
    //CAM_LOGD("configFrame+: param size: (%d)", rQParam.mvOut.size());
    bool ret = MTRUE;
    MINT32 i = 0;
    RRZCfg rrz;
    NSImageio::NSIspio::EPortIndex tgIdx;
    NSCam::NSIoPipe::PortID port;

    //CAM_LOGD("size(%d),mbTwinEnable(%d),mpEnabledSensorCnt(%d),", \
    //        rQParam.mvOut.size(),\
    //        mbTwinEnable, \
    //        mpEnabledSensorCnt);

    //
    for ( i =0 ; i < rQParam.mvOut.size();i++ ) {

        port = rQParam.mvOut.at(i).mPortID ;

        if (( PORT_RRZO == port )  || ( PORT_RRZO_D == port) || (PORT_IMGO == port) )
        {
            if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX((mpSensorIdx[0]))){
                 port = PORT_RRZO_D;
            }

            //CAM_LOGD("configFrame: idx =%d, port=%d", i, port.index);
            rrz.crop.x      = rQParam.mvOut.at(i).mCropRect.p.x;
            rrz.crop.y      = rQParam.mvOut.at(i).mCropRect.p.y;
            rrz.crop.floatX = 0;
            rrz.crop.floatY = 0;
            rrz.crop.w      = rQParam.mvOut.at(i).mCropRect.s.w;
            rrz.crop.h      = rQParam.mvOut.at(i).mCropRect.s.h;
            //
            rrz.out.w       = rQParam.mvOut.at(i).mDstSize.w;
            rrz.out.h       = rQParam.mvOut.at(i).mDstSize.h;
            rrz.out.stride  = rrz.out.w;
            rrz.m_num       = rQParam.mMagicNum;
            //

      //when you want to enable PDAF func, please check-in with Middleware at the same time
#if 0 //js_test pdaf remove after p1node complete implemetation
            rrz.mImgSel = eRawFmt_Processed; /*!<default processed raw*/
#else

            if(this->mDynSwtRawType){
                switch( rQParam.mRawOutFmt ) {
                    case EPipe_PURE_RAW:
                        rrz.mImgSel = eRawFmt_Pure;
                        break;
                    case EPipe_PROCESSED_RAW:
                    default:
                        rrz.mImgSel = eRawFmt_Processed;
                        break;
                }
            }
            else
                rrz.mImgSel = (this->mPureRaw)? eRawFmt_Pure: eRawFmt_Processed;
#endif
            //
            if (mbTwinEnable){
                tgIdx = EPortIndex_TG1I;
                if ((1 == mpEnabledSensorCnt) && ( PORT_RRZO_D == port )){
                    CAM_LOGE("RawPath doesnt support RRZO_D config");
                    return MFALSE;
                }
            }
            else {

               if ( PORT_RRZO == port || PORT_IMGO == port ){
                   if ((1 == mpEnabledSensorCnt) &&
                       (CAM_TG_1 != _NORMALPIPE_GET_TG_IDX(mpSensorIdx[0]))) {
                       CAM_LOGE("[Err]PORT_RRZO/PORT_IMGO belong to CAM_TG_1");
                       return MFALSE;
                   }
                   tgIdx = EPortIndex_TG1I;
               }
               else {
                   if ((1 == mpEnabledSensorCnt) &&
                       (CAM_TG_2 != _NORMALPIPE_GET_TG_IDX(mpSensorIdx[0]))) {
                       CAM_LOGE("[Err]PORT_RRZO_D belong to CAM_TG_2");
                       return MFALSE;
                   }
                   tgIdx = EPortIndex_TG2I;
               }
           }
            /*CAM_LOGD("[3ASync]cfFrm:PortIdx[%d],TG(%d),mNum=%x,Crop:(x,y,fx,fy,w,h)=(%d,%d,%d,%d,%d,%d); out:(x,y,stride)=(%d,%d,%d)",\
                     port.index,tgIdx,rQParam.mMagicNum,\
                     rrz.crop.x, rrz.crop.y, 0, 0, rrz.crop.w, rrz.crop.h,\
                     rrz.out.w, rrz.out.h, rrz.out.stride);*/

            if (mpCamIOPipe) {

                //Arg1: RRZCfg
                //Arg2: EPortIndex_TG1I/EPortIndex_TG2I
                //Arg3: sensorDev

                MUINT32 sensorDev = eSoftwareScenario_Main_Normal_Stream;
                if (SENSOR_DEV_SUB == _NORMALPIPE_GET_SENSOR_DEV_ID((mpSensorIdx[0]))) {
                    sensorDev = eSoftwareScenario_Sub_Normal_Stream;
                }
                else if (SENSOR_DEV_MAIN_2 == _NORMALPIPE_GET_SENSOR_DEV_ID((mpSensorIdx[0]))) {
                    sensorDev = eSoftwareScenario_Main2_N3D_Stream;
                }
                //lock rrz in order to update rrzo size for EIS to get rrz size
                NormalPipeGLock();
                if ( MFALSE == mpCamIOPipe->sendCommand(EPIPECmd_SET_RRZ,(MINTPTR)&rrz, tgIdx, sensorDev) ){
                    CAM_LOGE("Error:EPIPECmd_SET_RRZ Fail");
                    ret = MFALSE;
                }
                if(tgIdx == EPortIndex_TG1I){
                    mRrzoOut_size.w = rrz.out.w;
                    mRrzoOut_size.h = rrz.out.h;
                }else if(tgIdx == EPortIndex_TG2I){
                    mRrzodOut_size.w = rrz.out.w;
                    mRrzodOut_size.h = rrz.out.h;
                }
                m_bEISChkFlg = MTRUE;
                NormalPipeGUnLock();
#if 0  //Workaround for exclude P1 ISP setting
                static MUINT32 ispMagic = 0;
                if ( MFALSE == mpCamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE, tgIdx,ispMagic++,0) ) {
                    CAM_LOGE("EPIPECmd_SET_P1_UPDATE");
                    ret = false;
                }
#endif
            }
        }
        else{
           //TODO
           CAM_LOGD("configFrame: Not IMGO/RRZO/RRZOD Port-");

        }
    }

    //CAM_LOGD("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
NormalPipe::attach(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType)
{
    //CAM_LOGD("NormalPipe::attach +");
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
            CAM_LOGE("registerIrq(%d,%d,%d,%s,%d,)",irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipe::wait(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout)
{
    //return immediately if have received signal alread; otherwise, wait right now.

    CAM_LOGD("wait+");

    bool ret = true;
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
            CAM_LOGE("irq(%d,%d,%d,%s,%d,)",irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
            ret = false;
        }
    }
    CAM_LOGD("wait-");
    return ret;
}



MBOOL
NormalPipe::wait(MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout)
{
    CAM_LOGD("wait+");

    bool ret = MTRUE;
    Irq_t irq(0,0,0,0, (signed char*)this->pipeName(),0);

    irq.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
    irq.Timeout = 30000; //ms
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
            CAM_LOGE("irq(%d,%d,%d,%s,%d,)",irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
            ret = MFALSE;
        }
    }
    CAM_LOGD("wait-");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MUINT32*
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
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int  ret = MTRUE;
    MUINT32 tg;
    NormalPipe_HBIN_Info *hbin = NULL;
    NormalPipe_EIS_Info *pEis = NULL;
    MINT32 sensorDev = 1;
    MUINT32 _flg = MFALSE;
    //CAM_LOGD("sendCommand+, SenIdx=%d", mpSensorIdx[0]);
    if ( !mConfigDone ) {
        CAM_LOGE("error:sendCommand(0x%x) fail, send before configpipe(0x%x,0x%x)",cmd,this,this->mConfigDone);
        return MFALSE;
    }


    if ((EPIPECmd_SET_P1_UPDATE == cmd) ||(EPIPECmd_SET_RRZ == cmd)) {
        mFrmCntPerMnum = 0;

        if (EPIPECmd_SET_RRZ == cmd) {
            RRZCfg* pRrzCfg = (RRZCfg*)arg1;
            if (! arg1) {
                CAM_LOGE("Error: sendCommand+, arg1 == NULL.");
                return MFALSE;
            }
            mLastMnum = pRrzCfg->m_num;
        } else {
            mLastMnum = arg2;
        }
    }

    switch (cmd) {
        case EPIPECmd_SET_EIS_CBFP:
        case EPIPECmd_SET_LCS_CBFP:
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
                CAM_LOGE("[Error]mpCamIOPipe=NULL");
                return 0;
            }
            ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, arg3);
            break;

        case EPIPECmd_SET_P1_UPDATE:
            if ( !mpCamIOPipe ) {
                CAM_LOGE("[Error]mpCamIOPipe=NULL");
                return 0;
            }
            //Arg1: TG path number
            //Arg2: magic number
            //Arg3: sensorDev
            sensorDev = eSoftwareScenario_Main_Normal_Stream;
            if (SENSOR_DEV_SUB == _NORMALPIPE_GET_SENSOR_DEV_ID((mpSensorIdx[0]))) {
                sensorDev = eSoftwareScenario_Sub_Normal_Stream;
            }
            else if (SENSOR_DEV_MAIN_2 == _NORMALPIPE_GET_SENSOR_DEV_ID((mpSensorIdx[0]))) {
                sensorDev = eSoftwareScenario_Main2_N3D_Stream;
            }
            ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, sensorDev);
            break;
       case EPIPECmd_GET_CUR_FRM_RAWFMT:

            if(_NORMALPIPE_GET_TG_IDX(mpSensorIdx[0]) == CAM_TG_1){
                ret = mpCamIOPipe->sendCommand(cmd, arg1, EPortIndex_TG1I, arg3);
            }
            else if(_NORMALPIPE_GET_TG_IDX(mpSensorIdx[0]) == CAM_TG_2)
            {
                ret = mpCamIOPipe->sendCommand(cmd, arg1, EPortIndex_TG2I, arg3);
            }
            else
            {
                CAM_LOGE("TG Num is error!");
                return 0;
            }
            break;
       case EPIPECmd_GET_TG_OUT_SIZE:
           //arg1 : sensor-idx
           //arg2 : width
           //arg3 : height
           #if 1
            if (2 < arg1) {
                 CAM_LOGE("EPIPECmd_GET_TG_OUT_SIZE Err: Invalid SensorIdx(%d)!",arg1);
                return MFALSE;
            }
            *((MINT32*)arg2) =_NORMALPIPE_GET_SENSORCFG_CROP_W(arg1);
            *((MINT32*)arg3) =_NORMALPIPE_GET_SENSORCFG_CROP_H(arg1);
           #else

            if (2 < arg1) {
                 CAM_LOGE("EPIPECmd_GET_TG_OUT_SIZE Err: Invalid SensorIdx(%d)!",arg1);
                return -1;
            }
            tg = _NORMALPIPE_GET_TG_IDX(arg1);
            switch (CAM_TG_1) {
                case CAM_TG_1:
                    *((MINT32*)arg2) = mImgoOut_size.w;
                    *((MINT32*)arg3) = mImgoOut_size.h;
                    break;
                case CAM_TG_2:
                    *((MINT32*)arg2) = mImgodOut_size.w;
                    *((MINT32*)arg3) = mImgodOut_size.h;
                    break;
                case CAM_SV_1:
                    *((MINT32*)arg2) = mCamSvImgoOut_size.w;
                    *((MINT32*)arg3) = mCamSvImgoOut_size.h;
                    break;
                case CAM_SV_2:
                    *((MINT32*)arg2) = mCamSv2ImgoOut_size.w;
                    *((MINT32*)arg3) = mCamSv2ImgoOut_size.h;
                    break;
            }
           #endif
           //CAM_LOGD("EPIPECmd_GET_TG_OUT_SIZE SIdx(%d), w=0x%x, h=0x%x!",arg1, *((MINT32*)arg2), *((MINT32*)arg3));
           break;

       case EPIPECmd_GET_HBIN_INFO:
           //arg1 : sensor-idx
           //arg2 : NormalPipe_HBIN_Info
           if ((2 < arg1) || (NULL == arg2)) {
                CAM_LOGE("EPIPECmd_GET_HBIN_INFO Err: Invalid Para arg1(%d), arg2=0x%x!",arg1, arg2);
               return MFALSE;
           }
           tg = _NORMALPIPE_GET_TG_IDX(arg1);
           CAM_LOGD("EPIPECmd_GET_HBIN_INFO SIdx(%d), BinInfoAddr=0x%x",arg1, arg2);

           hbin = (NormalPipe_HBIN_Info*) arg2;
           switch (tg) {
               case CAM_TG_1:
                   if (mbTwinEnable) {
                       //HBin will be enable, size downsampe 1/2 for 3A
                       hbin->mEnabled = MTRUE;
                       //hbin->size.w = mImgoOut_size.w >> 1;
                       //hbin->size.h = mImgoOut_size.h >> 1;
                       hbin->size.w = mTgOut_size[tg].w>>1;
                       hbin->size.h = mTgOut_size[tg].h;
                   } else {
                       hbin->mEnabled = MFALSE;
                       hbin->size.w = mTgOut_size[tg].w;
                       hbin->size.h = mTgOut_size[tg].h;
                   }
                   break;
               case CAM_TG_2:
                   if (mbTwinEnable) {
                       //HBin will be enable, size downsampe 1/2 for 3A
                       hbin->mEnabled = MFALSE;
                       hbin->size.w = 0;
                       hbin->size.h = 0;
                   } else {
                       hbin->mEnabled = MFALSE;
                       hbin->size.w = 0;
                       hbin->size.h = 0;
                   }
                   break;
               default:
                   CAM_LOGE("EPIPECmd_GET_HBIN_INFO Err: Invalid SensorIdx(%d)!",arg1);
                   return MFALSE;
           }
           CAM_LOGD("EPIPECmd_GET_HBIN_INFO SIdx(%d),twin=%d,hbin_en=%d w=0x%x, h=0x%x!",arg1,mbTwinEnable, hbin->mEnabled,hbin->size.w, hbin->size.h);
           break;

       case EPIPECmd_GET_RMX_OUT_SIZE:
            //arg1 : sensor-idx
            //arg2 : width
            //arg3 : height
            NormalPipeGLock();//for protection
            _flg = m_bEISChkFlg;
            NormalPipeGUnLock();
            if(_flg == MTRUE){
                m_bEISChkFlg = MFALSE;
                //hold query behavior until tuning && configframe magic number r all retrieved
                CAM_LOGD("rrz query hold");
                if ( !mpCamIOPipe ) {
                   CAM_LOGE("[Error]mpCamIOPipe=NULL");
                   return 0;
               }
            }
            else
                CAM_LOGD("rrz size have no new update\n");

            if (2 < arg1) {
                CAM_LOGE("EPIPECmd_GET_RMX_OUT_SIZE Err: Invalid SensorIdx(%d)!",arg1);
                return MFALSE;
            }
            tg = _NORMALPIPE_GET_TG_IDX(arg1);
            switch (tg) {
                case CAM_TG_1:
                    *((MINT32*)arg2) = mRrzoOut_size.w;
                    *((MINT32*)arg3) = mRrzoOut_size.h;
                    break;
                case CAM_TG_2:
                    *((MINT32*)arg2) = mRrzodOut_size.w;
                    *((MINT32*)arg3) = mRrzodOut_size.h;
                    break;
                default:
                    ret = MFALSE;
                    CAM_LOGE("EPIPECmd_GET_RMX_OUT_SIZE Fail! SIdx=%d, TG=%d",arg1,tg);
                    break;
            }
            //CAM_LOGD("EPIPECmd_GET_RMX_OUT_SIZE SIdx(%d),tg(%d), w=0x%x, h=0x%x!",arg1,tg, *((MINT32*)arg2), *((MINT32*)arg3));
            break;

       case EPIPECmd_GET_EIS_INFO:
           //arg1 : sensor-idx
           //arg2 : NormalPipe_EIS_Info*
           if (NULL == arg2) {
                CAM_LOGE("EPIPECmd_GET_EIS_INFO Err: Invalid Para arg1(%d)",arg1);
               return MFALSE;
           }
           pEis = (NormalPipe_EIS_Info*) arg2;
#if 0
           if (mEnablePath & NPIPE_PASS_PASS1) {
               pEis->mSupported = MTRUE;
           }
           else {
               pEis->mSupported = MFALSE;
           }
#else
               tg = _NORMALPIPE_GET_TG_IDX(arg1);
               switch (tg) {
                   case CAM_TG_1:
                       pEis->mSupported = MTRUE;
                       break;
                   case CAM_TG_2:
                       pEis->mSupported = MFALSE;
                       break;
                   default:
                       pEis->mSupported = MFALSE;
                       break;
               }
#endif
           break;

       case EPIPECmd_GET_TWIN_INFO:
           *((MINT32*)arg1) = mbTwinEnable;
           break;

       default:
           if ( !mpCamIOPipe ) {
               CAM_LOGE("[Error]mpCamIOPipe=NULL");
               return 0;
           }
           ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, arg3);
           break;
    }
    if(ret != MTRUE){
        CAM_LOGE("error: sendCommand fail: (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", cmd, arg1, arg2, arg3);
    }
    //CAM_LOGD("sendCommand-");
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
deque(MINTPTR key) //key == va
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



