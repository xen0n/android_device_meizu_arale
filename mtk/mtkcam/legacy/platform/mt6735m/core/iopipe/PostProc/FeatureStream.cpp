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
#define LOG_TAG "Iop/P2FStm"

#include "FeatureStream.h"
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
DECLARE_DBG_LOG_VARIABLE(P2FeatureStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (P2FeatureStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2FeatureStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2FeatureStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2FeatureStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2FeatureStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2FeatureStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)



#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;

ESoftwareScenario const gSenDevSwScenMap[EFeatureStreamTag_total][SENSOR_DEV_MAIN_3D+1]=
{
    {eSoftwareScenario_Main_Normal_Stream,eSoftwareScenario_Main_Normal_Stream,eSoftwareScenario_Sub_Normal_Stream,\
        eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num},//EFeatureStreamTag_Stream
    {eSoftwareScenario_Main_Pure_Raw_Stream,eSoftwareScenario_Main_Pure_Raw_Stream,eSoftwareScenario_Sub_Pure_Raw_Stream,\
        eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num}//EFeatureStreamTag_PRaw_Stream
};

extern SwHwScenarioPathMapping mSwHwPathMapping[eSoftwareScenario_total_num];

/******************************************************************************
 *
 ******************************************************************************/
IFeatureStream*
IFeatureStream::
createInstance(
    char const* szCallerName,
    NSCam::NSIoPipe::NSPostProc::EFeatureStreamTag streamTag,
    MUINT32 openedSensorIndex,
    NSCam::NSIoPipe::NSPostProc::EScenarioFormat scenFmt)
{
    FUNCTION_LOG_START;
    //[1] create HalPipeWrapper
    return new FeatureStream(streamTag,openedSensorIndex,scenFmt);
    FUNCTION_LOG_END;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL IFeatureStream::queryCapability(IMetadata& rCapability)
{
    FUNCTION_LOG_START;
    LOG_INF("No support query capability in feature stream");
    FUNCTION_LOG_END;
    return  !rCapability.isEmpty();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FeatureStream::
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
FeatureStream::
FeatureStream(
    EFeatureStreamTag streamTag,
    MUINT32 openedSensorIndex,
    NSCam::NSIoPipe::NSPostProc::EScenarioFormat scenFmt)    //change to use sensor index, 0xFFFF means pure pass2
            : mpHalPipeWrapper(NULL)
            , mpPostProcPipe(NULL)
            , mOpenedSensor(SENSOR_DEV_NONE)
            , mHalSensorList(NULL)
            , mStreamTag(EFeatureStreamTag_Stream)
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
   {    //p1+p2, get sensor dev firstly
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
   LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x),fmt(%d)",mStreamTag,openedSensorIndex,mOpenedSensor,(MUINT32)(&mDequeuedBufList),mScenFmt);
}


/*******************************************************************************
*
********************************************************************************/
FeatureStream::~FeatureStream()
{

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureStream::
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
    #endif
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureStream::
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
FeatureStream::
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
        QFeatureData dummyData;
        ret=mpHalPipeWrapper->enque(mSWScen,(rParams),this->mpPostProcPipe,this->mDequeuedBufList,EBufferTag_Blocking,dummyData);
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
FeatureStream::
enque(
    QParams const& rParams,
    QFeatureData& rFeatureData)
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
        ret=mpHalPipeWrapper->enque(mSWScen,(rParams),this->mpPostProcPipe,this->mDequeuedBufList,EBufferTag_Blocking,rFeatureData);
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
FeatureStream::
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
FeatureStream::
deque(
    QParams& rParams,
    MINT64 i8TimeoutNs,
    QFeatureData& rFeatureData
    )
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = false;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_INF("sTag(%d),i8TimeoutNs(%lld)",mStreamTag,i8TimeoutNs);
    ret=mpHalPipeWrapper->deque((rParams),this->mpPostProcPipe,(MUINT32)(&mDequeuedBufList),i8TimeoutNs,rFeatureData);
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
FeatureStream::
queryCropInfo(
    android::Vector<MCropPathInfo>& mvCropPathInfo)
{
    #if 0//CHRISTOPHER, do not need in current stage
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
FeatureStream::
queryGroupMember(
    MINT32 portIdx,
    MBOOL& beforeCRZ,
    android::Vector<MINT32>& vGroupPortIdx)
{
    #if 0 //CHRISTOPHER, do not need in current stage
    FUNCTION_LOG_START;
    bool isGet=false;
    //default set the dma is before crz
    beforeCRZ=true;

    //parsing data
    for(int i=0;i<mCropPaths.size();i++)
    {
        //[1] check there is vipi in this scenario or not
        isGet=false;
        for(int j=0;j<mCropPaths[i].PortIdxVec.size();j++)
        {
            if(mCropPaths[i].PortIdxVec[j]== portIdx)
            {
                isGet=true;
            }
            else
            {}
            if(isGet)
            {
                break;
            }
        }

        //[2]add index of port in the same group
        if(isGet)
        {
            //update the dma position before/after crz
            if(mCropPaths[i].u4CropGroup != ECropGroupIndex_NONE)
            {
                beforeCRZ=false;
            }
            for(int j=0;j<mCropPaths[i].PortIdxVec.size();j++)
            {

                if((mCropPaths[i].PortIdxVec[j]== portIdx) || (portIdx == -1))
                {}
                else
                {
                    vGroupPortIdx.push_back(mCropPaths[i].PortIdxVec[j]);
                }
            }
            //suppose the dma should be only included in one group
            break;
        }
    }
    LOG_INF("return bcrz(%d)/size(%d)",beforeCRZ,vGroupPortIdx.size());
    FUNCTION_LOG_END;
    #else
    LOG_ERR("do not support this function now");
    #endif
    return true;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
FeatureStream::
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
FeatureStream::
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
FeatureStream::
sendCommand(
    MINT32 cmd,
    MINT32 arg1,
    MINT32 arg2)
{
    bool ret=true;

    #if 1
    ret=mpHalPipeWrapper->sendCommand(cmd, mSWScen, arg1, arg2);
    if(!ret)
    {
        LOG_ERR("[Error]sendCommand(0x%x,0x%x,0x%x) Fail",cmd,arg1,arg2);
    }
    #else
    mpPostProcPipe->sendCommand(cmd, mSWScen, arg1, arg2);
    #endif

    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
FeatureStream::
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
FeatureStream::
enTuningQue(
    void* pTuningQueBuf)
{
    LOG_ERR("[Error]camera1.0 not support this function");
    //
    return false;
}






