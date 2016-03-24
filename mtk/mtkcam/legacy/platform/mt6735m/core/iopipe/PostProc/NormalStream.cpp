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
#define LOG_TAG "Iop/P2NStm"

#include "NormalStream.h"
#include "PortMap.h"
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <vector>
#include <cutils/atomic.h>

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(P2NormalStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2NormalStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)



#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;

ESoftwareScenario const gSenDevSwScenMap[ENormalStreamTag_total][SENSOR_DEV_MAIN_3D+1]=
{
    {eSoftwareScenario_Main_Normal_Stream,eSoftwareScenario_Main_Normal_Stream,eSoftwareScenario_Sub_Normal_Stream,\
        eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num},//ENormalStreamTag_Stream
    {eSoftwareScenario_Main_VSS_Capture,eSoftwareScenario_Main_VSS_Capture,eSoftwareScenario_Sub_VSS_Capture,\
        eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num},//ENormalStreamTag_Vss
    {eSoftwareScenario_Main_Pure_Raw_Stream,eSoftwareScenario_Main_Pure_Raw_Stream,eSoftwareScenario_Sub_Pure_Raw_Stream,\
        eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num}//ENormalStreamTag_PureRaw
};

extern SwHwScenarioPathMapping mSwHwPathMapping[eSoftwareScenario_total_num];
/******************************************************************************
 *
 ******************************************************************************/
 #if 0//CHRISTOPHER, do not need in current stage
namespace
{
    using namespace NSCam::NSIoPipe;
    //
    Mutex                           gPipeCapMutex;
    IMetadata                       gPipeCap;
    //
    //  IN PORT
    IMetadata                       gPortCap_IMGI;
    IMetadata                       gPortCap_VIPI;
    //
    //  OUT PORT
    IMetadata                       gPortCap_IMG2O;
    IMetadata                       gPortCap_IMG3O;
    IMetadata                       gPortCap_WROTO;
    IMetadata                       gPortCap_WDMAO;
    IMetadata                       gPortCap_JPEGO;
    //
};
/******************************************************************************
 *
 ******************************************************************************/
static
IMetadata const&
constructPortCapability()
{
    //CHRISTOPHER, do not need in current stage
    Mutex::Autolock _l(gPipeCapMutex);
    //
    if  ( ! gPipeCap.isEmpty() )
    {
        return  gPipeCap;
    }
    //
    //  IMGI
    {
        IMetadata& cap = gPortCap_IMGI;
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
            entry.push_back(PORT_IMGI, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_TRANSFORM);
            entry.push_back(eTransform_FLIP_V, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
            entry.push_back(MTK_IOPIPE_INFO_CROP_NOT_SUPPORT, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
            entry.push_back(eImgFmt_BAYER8, Type2Type<MINT32>());
            entry.push_back(eImgFmt_BAYER10,Type2Type<MINT32>());
            entry.push_back(eImgFmt_BAYER12,Type2Type<MINT32>());
            entry.push_back(eImgFmt_YUY2,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_YVYU,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_UYVY,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_VYUY,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_RGB565, Type2Type<MINT32>());
            entry.push_back(eImgFmt_ARGB888,Type2Type<MINT32>());
            entry.push_back(eImgFmt_RGB888, Type2Type<MINT32>());
            entry.push_back(eImgFmt_FG_BAYER8, Type2Type<MINT32>());
            entry.push_back(eImgFmt_FG_BAYER10,Type2Type<MINT32>());
            entry.push_back(eImgFmt_FG_BAYER12,Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
    }
    //  VIPI
    {
        IMetadata& cap = gPortCap_VIPI;
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
            entry.push_back(PORT_VIPI, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_TRANSFORM);
            entry.push_back(eTransform_FLIP_V, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
            entry.push_back(MTK_IOPIPE_INFO_CROP_NOT_SUPPORT, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
            //yuv422
            entry.push_back(eImgFmt_YUY2,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_YVYU,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_UYVY,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_VYUY,   Type2Type<MINT32>());
            //420 2p(y plane, uv would be vip2i)
            entry.push_back(eImgFmt_NV12,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_NV21,   Type2Type<MINT32>());
            //420 3p(y plane, u would be vip2i and v would be vip3i)
            entry.push_back(eImgFmt_YV12,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_I420,   Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
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
            entry.push_back(eTransform_FLIP_V, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
            entry.push_back(MTK_IOPIPE_INFO_CROP_SYMMETRIC|MTK_IOPIPE_INFO_CROP_ASYMMETRIC, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
            entry.push_back(eImgFmt_YUY2,   Type2Type<MINT32>());   //need match with the setting of imgi, ex,imgi(yuyv)->img2o(yuyv)
            entry.push_back(eImgFmt_YVYU,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_UYVY,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_VYUY,   Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
    }
    //  IMG3O
    {
        IMetadata& cap = gPortCap_IMG3O;
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
            entry.push_back(PORT_IMG3O, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_TRANSFORM);
            entry.push_back(eTransform_FLIP_V, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
            entry.push_back(MTK_IOPIPE_INFO_CROP_SYMMETRIC|MTK_IOPIPE_INFO_CROP_ASYMMETRIC, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
            //yuv422
            entry.push_back(eImgFmt_YUY2,   Type2Type<MINT32>());   //need match with the setting of vipi, ex,vipi(yuyv)->img3o(yuyv)
            entry.push_back(eImgFmt_YVYU,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_UYVY,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_VYUY,   Type2Type<MINT32>());
            //420 2p(y plane, uv would be img3bo)
            entry.push_back(eImgFmt_NV12,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_NV21,   Type2Type<MINT32>());
            //420 3p(y plane, u would be img3bo and v would be imb3co)
            entry.push_back(eImgFmt_YV12,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_I420,   Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
    }
    //  WROTO
    {
        IMetadata& cap = gPortCap_WROTO;
        {
        IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
        entry.push_back(PORT_WROTO, Type2Type<MINT32>());
        cap.update(entry.tag(), entry);
        }
        {
        IMetadata::IEntry entry(MTK_IOPIPE_INFO_TRANSFORM);
        entry.push_back(eTransform_FLIP_H|eTransform_ROT_90|eTransform_ROT_180|eTransform_ROT_270, Type2Type<MINT32>());
        cap.update(entry.tag(), entry);
        }
        {
        IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
        entry.push_back(MTK_IOPIPE_INFO_CROP_SYMMETRIC|MTK_IOPIPE_INFO_CROP_ASYMMETRIC, Type2Type<MINT32>());
        cap.update(entry.tag(), entry);
        }
        {
        IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
        entry.push_back(eImgFmt_YUY2, Type2Type<MINT32>());
        entry.push_back(eImgFmt_UYVY, Type2Type<MINT32>());
        entry.push_back(eImgFmt_NV12,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_NV21,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_NV16,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_NV61,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_YV12,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_I420,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_YV16,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_I422,   Type2Type<MINT32>());
        entry.push_back(eImgFmt_RGB565, Type2Type<MINT32>());
        entry.push_back(eImgFmt_ARGB888,Type2Type<MINT32>());
        entry.push_back(eImgFmt_RGB888, Type2Type<MINT32>());
        //TODO
        cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
    }
    //  WDMAO
    {
        IMetadata& cap = gPortCap_WDMAO;
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
            entry.push_back(PORT_WDMAO, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_TRANSFORM);
            entry.push_back(0, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
            entry.push_back(MTK_IOPIPE_INFO_CROP_SYMMETRIC|MTK_IOPIPE_INFO_CROP_ASYMMETRIC, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
            entry.push_back(eImgFmt_YUY2, Type2Type<MINT32>());
            entry.push_back(eImgFmt_UYVY, Type2Type<MINT32>());
            entry.push_back(eImgFmt_RGB565, Type2Type<MINT32>());
            entry.push_back(eImgFmt_RGB888, Type2Type<MINT32>());
            entry.push_back(eImgFmt_ARGB888, Type2Type<MINT32>());
            entry.push_back(eImgFmt_NV21, Type2Type<MINT32>());
            entry.push_back(eImgFmt_NV12, Type2Type<MINT32>());
            entry.push_back(eImgFmt_YV12,   Type2Type<MINT32>());
            entry.push_back(eImgFmt_I420,   Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
    }
    //JPEG
    {
        IMetadata& cap = gPortCap_JPEGO;
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_PORT_ID);
            entry.push_back(PORT_JPEGO, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_TRANSFORM);
            entry.push_back(0, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_CROP);
            entry.push_back(0, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_FORMATS);
            entry.push_back(eImgFmt_JPEG, Type2Type<MINT32>());
            cap.update(entry.tag(), entry);
        }
        //
        cap.sort();
    }
    //
    //  Pipe
    {
        IMetadata& cap = gPipeCap;
        {   // in port
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_IN_PORT_INFO);
            entry.push_back(gPortCap_IMGI, Type2Type<IMetadata>());
            entry.push_back(gPortCap_VIPI, Type2Type<IMetadata>());       //only for nr3d in normal streaming
            cap.update(entry.tag(), entry);
        }
        {   // out port
            IMetadata::IEntry entry(MTK_IOPIPE_INFO_AVAILABLE_OUT_PORT_INFO);
            entry.push_back(gPortCap_IMG2O, Type2Type<IMetadata>());
            entry.push_back(gPortCap_IMG3O, Type2Type<IMetadata>());      //only for nr3d in normal streaming
            entry.push_back(gPortCap_WROTO, Type2Type<IMetadata>());
            entry.push_back(gPortCap_WDMAO, Type2Type<IMetadata>());
            entry.push_back(gPortCap_JPEGO, Type2Type<IMetadata>());
            cap.update(entry.tag(), entry);
        }
        cap.sort();
    }
    //
    LOG_DBG("count:%d", gPipeCap.count());
    return  gPipeCap;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
INormalStream*
INormalStream::
createInstance(
    char const* szCallerName,
    NSCam::NSIoPipe::NSPostProc::ENormalStreamTag streamTag,
    MUINT32 openedSensorIndex,
    NSCam::NSIoPipe::NSPostProc::EScenarioFormat scenFmt)
{
    FUNCTION_LOG_START;
    //[1] create HalPipeWrapper
    return new NormalStream(streamTag,openedSensorIndex,scenFmt);
    FUNCTION_LOG_END;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL INormalStream::queryCapability(IMetadata& rCapability)
{
    FUNCTION_LOG_START;
    #if 0
    rCapability = constructPortCapability();
    FUNCTION_LOG_END;
    return  !rCapability.isEmpty();
    #else
    LOG_ERR("do not support this function now ");
    return MTRUE;
    #endif
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
NormalStream::
destroyInstance(
    char const* szCallerName)
{
    FUNCTION_LOG_START;
    if(mpHalPipeWrapper==NULL)
    {
        LOG_INF("sTag(%d),NULL mpHalPipeWrapper.",mStreamTag);
    }
    else
    {
        mpHalPipeWrapper->destroyInstance();
        mpHalPipeWrapper=NULL;
    }
    //
    if(mpPostProcPipe==NULL)
    {
        LOG_INF("sTag(%d),NULL mpPostProcPipe.",mStreamTag);
    }
    else
    {
        mpPostProcPipe->destroyInstance();
        mpPostProcPipe=NULL;
    }
    FUNCTION_LOG_END;
    delete this;
}
/*******************************************************************************
*
********************************************************************************/
NormalStream::
NormalStream(
    NSCam::NSIoPipe::NSPostProc::ENormalStreamTag streamTag,
    MUINT32 openedSensorIndex,
    NSCam::NSIoPipe::NSPostProc::EScenarioFormat scenFmt)    //change to use sensor index
            : mpHalPipeWrapper(NULL)
            , mpPostProcPipe(NULL)
            , mOpenedSensor(SENSOR_DEV_NONE)
            , mHalSensorList(NULL)
            , mJpegWorkBufSize(0)
            , mStreamTag(ENormalStreamTag_Stream)
            , mSWScen(eSoftwareScenario_total_num)
            , mScenFmt(NSCam::NSIoPipe::NSPostProc::eScenarioFormat_RAW)
{
   //
   mStreamTag=streamTag;
   //
   mpHalPipeWrapper=HalPipeWrapper::createInstance();

   #if 0//CHRISTOPHER, do not need in current stage
   if(openedSensorIndex==0xFFFF)
   {    //pure pass2
        mOpenedSensor=SENSOR_DEV_NONE;
   }
   else
   {   //p1+p2, get sensor dev firstly

       #ifndef USING_MTK_LDVT
       mHalSensorList=IHalSensorList::get();
       if(mHalSensorList)
       {
           //[1] get sensor dev index from sensorID
           MINT32 sensorDEVIdx=mHalSensorList->querySensorDevIdx(openedSensorIndex);
           mOpenedSensor=static_cast<MUINT32>(sensorDEVIdx);
       }
       else
       {
            LOG_ERR("NULL mHalSensorList,tag/sidx/sdev(%d/0x%x/0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor);
       }
       #else
       //default use main sensor in ldvt load is user pass in sensor index
       mOpenedSensor=SENSOR_DEV_MAIN;
       LOG_INF("in LDVT load, sdev(0x%x)",mOpenedSensor);
       #endif


   }
   #else    //sensor index 0 and 1 means main/sub sensor repectively in v1 driver
   switch (openedSensorIndex)
   {
       case 0:
           mOpenedSensor=SENSOR_DEV_NONE;
           break;
       case 1:
           mOpenedSensor=SENSOR_DEV_MAIN;
           break;
       default:
           mOpenedSensor=SENSOR_DEV_SUB;
           break;
   }
   mScenFmt=scenFmt;
   #endif
   //default value for jpeg
   //mJpegCfg.soi_en=1; //CHRISTOPHER, do not need in current stage
   //mJpegCfg.fQuality=90;//CHRISTOPHER, do not need in current stage
   LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x),fmt(%d)",mStreamTag,openedSensorIndex,mOpenedSensor,(MUINT32)(&mDequeuedBufList),mScenFmt);
}


/*******************************************************************************
*
********************************************************************************/
NormalStream::~NormalStream()
{

}



/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
init()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    MUINT32 cropPathNum=0;
    //[1] init HalPipeWrapper, and caller postproc pipe object would be created at this stage according to different sw scenario
    mSWScen=gSenDevSwScenMap[mStreamTag][(MINT32)mOpenedSensor];
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x)!",mStreamTag,mOpenedSensor);
        ret=false;
        return ret;
    }
    else
    {
        LOG_INF("sTag/sensorDev/swScen/scenFmt (%d)/(0x%x)/(%d)/(%d)",mStreamTag,mOpenedSensor,mSWScen,mScenFmt);
        mpHalPipeWrapper->init(mSWScen,(MUINT32&)(this->mpPostProcPipe),mScenFmt);
    }


    //[2] get supported crop paths
    #if 0//CHRISTOPHER, do not need in current stage
    if(this->mpPostProcPipe)
    {
        //query crop information
        mpPostProcPipe->queryCropPathNum(mSwHwPathMapping[mSWScen].hwPath, cropPathNum);
        for(int i=0;i<cropPathNum;i++)
        {
            CropPathInfo crop;
            mCropPaths.push_back(crop);
        }
        mpPostProcPipe->queryScenarioInfo(mSwHwPathMapping[mSWScen].hwPath, mCropPaths);
    }
    else
    {
        LOG_ERR("sTag(%d),Init fail!",mStreamTag);
        ret=false;
    }
    //[3] set default value for jpg related params
    mpPostProcPipe->sendCommand(EPIPECmd_SET_JPEG_CFG, (MINT32)&mJpegCfg,0,0);
    #endif

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
uninit()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    //[1] uninit postproc
    if (NULL != mpPostProcPipe)
    {
        if (true != mpPostProcPipe->uninit())
        {
            LOG_ERR("sTag(%d),caller PostProc uninit fail",mStreamTag);
            ret = false;
            return ret;
        }
    }

    //[2] uninit HalPipeWrapper
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x)!",mStreamTag,mOpenedSensor);
        ret=false;
        return ret;
    }
    else
    {
        LOG_INF("sTag/sensorDev/swScen (%d)/(0x%x)/(%d)",mStreamTag,mOpenedSensor,mSWScen);
        mpHalPipeWrapper->uninit(mSWScen,this->mpPostProcPipe);
    }
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
enque(
    QParams const& rParams)
{
    //FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x)!",mStreamTag,mOpenedSensor);
        ret=false;
        return ret;
    }
    else
    {
        EBufferTag tag=EBufferTag_Blocking;
        switch(mSWScen)
        {
            case eSoftwareScenario_Main_VSS_Capture:
            case eSoftwareScenario_Sub_VSS_Capture:
                tag=EBufferTag_Vss;
                break;
            default:
                break;
        }
        QFeatureData dummyData;
        ret=mpHalPipeWrapper->enque(mSWScen,(rParams),this->mpPostProcPipe,this->mDequeuedBufList,tag,dummyData);
        if(!ret)
        {
            LOG_ERR("enque Fail");
        }
    }
    //FUNCTION_LOG_END;
    return ret;

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
deque(
    QParams& rParams,
    MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = false;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_DBG("sTag(%d),i8TimeoutNs(%lld)",mStreamTag,i8TimeoutNs);
    QFeatureData dummyData;
    ret=mpHalPipeWrapper->deque((rParams),this->mpPostProcPipe,(MUINT32)(&mDequeuedBufList),i8TimeoutNs,dummyData);
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
queryCropInfo(
    android::Vector<MCropPathInfo>& mvCropPathInfo)
{
    #if 0 //CHRISTOPHER, do not need in current stage
    for(int i=0;i<mCropPaths.size();i++)
    {

        MCropPathInfo crop;
        crop.mGroupIdx=mCropPaths[i].u4CropGroup;
        for(int j=0;j<mCropPaths[i].PortIdxVec.size();j++)
        {
            crop.mvPorts.push_back(mCropPaths[i].PortIdxVec[j]);
        }
        mvCropPathInfo.push_back(crop);
    }
    #else
    LOG_ERR("do not support this function now");
    #endif
    return true;
}
/******************************************************************************
*
******************************************************************************/

MBOOL
NormalStream::
setJpegParam(
    EJpgCmd jpgCmd,
    int arg1,
    int arg2)
{
    FUNCTION_LOG_START;
    bool ret = true;
    #if 0 //CHRISTOPHER, do not need in current stage

    if(mpPostProcPipe==NULL)
    {
        LOG_ERR("sTag(%d),NULL mpPostProcPipe",mStreamTag);
        return false;
    }
    switch(jpgCmd)
    {
        case EJpgCmd_SetWorkBufSize:    //setJpegParam(EJpgCmd_SetWorkBufSize,0,0)
            mpPostProcPipe->sendCommand(EPIPECmd_SET_JPEG_WORKBUF_SIZE, mJpegWorkBufSize,0,0);
            break;
        case EJpgCmd_SetQualityParam:
            mJpegCfg.soi_en=arg1;
            mJpegCfg.fQuality=arg2;     //setJpegParam(EJpgCmd_SetQualityParam,soi_en,fQuality)
            mpPostProcPipe->sendCommand(EPIPECmd_SET_JPEG_CFG, (MINT32)&mJpegCfg,0,0);
            break;
        default:
            break;
    }
    #else
    LOG_ERR("do not support this function now ");
    #endif
    FUNCTION_LOG_END;
    return ret;
}
/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
startVideoRecord(
    MINT32 wd,
    MINT32 ht,
    MINT32 fps)
{
    FUNCTION_LOG_START;
    LOG_INF("mSWScen(%d),mpHalPipeWrapper()0x%x",mSWScen,mpHalPipeWrapper);
    bool ret=true;
    //
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("plz do init first");
        ret=false;
    }
    else
    {
        if(mpHalPipeWrapper)
        {
            ret=mpHalPipeWrapper->startVideoRecord(wd,ht,fps,mSWScen);
        }
        else
        {
            LOG_ERR("Null pointer");
            ret=false;
        }
    }
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
stopVideoRecord()
{
    FUNCTION_LOG_START;
    bool ret=true;
    //
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("plz do init first");
        ret=false;
    }
    else
    {
        if(mpHalPipeWrapper)
        {
            ret=mpHalPipeWrapper->stopVideoRecord(mSWScen);
        }
        else
        {
            LOG_ERR("Null pointer");
            ret=false;
        }
    }
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
deTuningQue(
    unsigned int& size,
    void* &pTuningQueBuf)
{
    LOG_ERR("[Error]camera1.0 not support this function");
    //
    return false;
}
/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
enTuningQue(
    void* pTuningQueBuf)
{
    LOG_ERR("[Error]camera1.0 not support this function");
    //
    return false;
}




