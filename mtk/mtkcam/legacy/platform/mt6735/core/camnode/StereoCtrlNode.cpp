/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "MtkCam/SCNode"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::Utils::Format;

#include <semaphore.h>
#include <vector>
#include <list>
using namespace std;
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
using namespace android;
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include "./inc/IspSyncControlHw.h"
//
#include "mtkcam/drv/imem_drv.h"
//
#include <aee.h>
//
#include <mtkcam/featureio/stereo_hal_base.h>
//
#include <mtkcam/camnode/StereoCtrlNode.h>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)

#define MY_LOGV2(fmt, arg...)       CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD2(fmt, arg...)       CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI2(fmt, arg...)       CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW2(fmt, arg...)       CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE2(fmt, arg...)       CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA2(fmt, arg...)       CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF2(fmt, arg...)       CAM_LOGF("[%s] " fmt, __func__, ##arg)

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)
#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

#define FUNC_START          MY_LOGD("+")
#define FUNC_END            MY_LOGD("-")

#define ENABLE_CAMIOCONTROL_LOG (0)
#define ENABLE_BUFCONTROL_LOG   (1)
#define BUFFER_RETURN_CHECK     (1)

#define AEE_ASSERT(String)    \
    do {                      \
        aee_system_exception( \
            LOG_TAG,          \
            NULL,             \
            DB_OPT_DEFAULT,   \
            String);          \
    } while(0)

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_NAME        "SCtrl"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*   utilities
********************************************************************************/

/*******************************************************************************
 *
 ********************************************************************************/
class StereoCtrlNodeImpl : public StereoCtrlNode
{
    public: // ctor & dtor
        StereoCtrlNodeImpl(SCNodeInitCfg const initCfg);
        ~StereoCtrlNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations
        MBOOL           isReadyToAlgo() const;

        MBOOL           pushBuf(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext);

    protected:
        MINT32          getOpenId_Main() const      { return mSensorId_Main; }
        MINT32          getOpenId_Main2() const     { return mSensorId_Main2; }
        MUINT32         getStereoType() const       { return mInitCfg.mScenarioType; }

        struct PostBufInfo
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;

            public:     ////    ctor.
                PostBufInfo(
                        MUINT32         _data= 0,
                        IImageBuffer*   _buf = NULL,
                        MUINT32         _ext = 0
                    )
                    : data(_data)
                    , buf(_buf)
                    , ext(_ext) {}
        };

        struct FeoBufInfo
        {
            MUINT32          data;
            IMEM_BUF_INFO*   buf;
            MUINT32          ext;

            public:     ////    ctor.
                FeoBufInfo(
                        MUINT32         _data= 0,
                        IMEM_BUF_INFO*  _buf = NULL,
                        MUINT32         _ext = 0
                    )
                    : data(_data)
                    , buf(_buf)
                    , ext(_ext) {}
        };

    private:
        //     init config
        SCNodeInitCfg const     mInitCfg;
        //
        MINT32                  mSensorId_Main;
        MINT32                  mSensorId_Main2;
        //
        StereoHalBase*          mpStereoHal;
        //
        // ************ buffer control ************ //
        mutable Mutex           mLock;
        MUINT32                 muPostFrameCnt;
        list<PostBufInfo>       mlPostBufMain;
        list<PostBufInfo>       mlPostBufImg;
        list<PostBufInfo>       mlPostBufImg_Main2;
        list<FeoBufInfo>        mlPostBufFeo;
        list<FeoBufInfo>        mlPostBufFeo_Main2;
        list<PostBufInfo>       mlPostBufRgb;
        list<PostBufInfo>       mlPostBufRgb_Main2;
        //
        IImageBuffer*           mpAlgoDstImgBuf;
};


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNode*
StereoCtrlNode::
createInstance(SCNodeInitCfg const initCfg)
{
    return new StereoCtrlNodeImpl(initCfg);
}


/*******************************************************************************
 *
 ********************************************************************************/
void
StereoCtrlNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNode::
    StereoCtrlNode()
: ICamThreadNode( MODULE_NAME, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNode::
~StereoCtrlNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNodeImpl::
    StereoCtrlNodeImpl(SCNodeInitCfg const initCfg)
    : StereoCtrlNode()
    , mInitCfg(initCfg)
    , mSensorId_Main(-1)
    , mSensorId_Main2(-1)
    , mpStereoHal(NULL)
    , muPostFrameCnt(0)
    , mpAlgoDstImgBuf(NULL)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_FEO_0 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_FEO_1 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_IMG_0 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_IMG_1 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_RGB_0 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_RGB_1 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_MAIN_SRC );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_DST_M );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_DST_S );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_MAIN_DST );
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNodeImpl::
~StereoCtrlNodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    String8 const s8MainIdKey("MTK_SENSOR_DEV_MAIN");
    String8 const s8Main2IdKey("MTK_SENSOR_DEV_MAIN_2");
    Utils::Property::tryGet(s8MainIdKey, mSensorId_Main);
    Utils::Property::tryGet(s8Main2IdKey, mSensorId_Main2);
    INIT_DATA_STEREO_IN_T   sDataIn;
    INIT_DATA_STEREO_OUT_T  sDataOut;
    RRZ_DATA_STEREO_T       sRrzData;
    //
    mlPostBufMain.clear();
    mlPostBufImg.clear();
    mlPostBufImg_Main2.clear();
    mlPostBufFeo.clear();
    mlPostBufFeo_Main2.clear();
    mlPostBufRgb.clear();
    mlPostBufRgb_Main2.clear();
    //
    IspSyncControlHw* pISC_main = IspSyncControlHw::createInstance( getOpenId_Main() );
    IspSyncControlHw* pISC_main2= IspSyncControlHw::createInstance( getOpenId_Main2() );
    if( !pISC_main || !pISC_main2)
    {
        MY_LOGE("create IspSyncControlHw failed");
        goto lbExit;
    }
    //
    switch ( getStereoType() )
    {
        case STEREO_CTRL_PREVIEW:
            sDataIn.eScenario = STEREO_SCENARIO_PREVIEW;
            break;
        case STEREO_CTRL_RECORD:
            sDataIn.eScenario = STEREO_SCENARIO_RECORD;
            break;
        case STEREO_CTRL_CAPTURE:
            sDataIn.eScenario = STEREO_SCENARIO_CAPTURE;
            break;
        default:
            MY_LOGE("unsupport stereo type(%d)", getStereoType());
            break;
    }
    //
    sDataIn.main1_sensor_index      = getOpenId_Main();
    sDataIn.main2_sensor_index      = getOpenId_Main2();
    // TODO: FIXME
    sDataIn.main_image_size = MSize(mInitCfg.mMainImgWidth, mInitCfg.mMainImgHeight);
    sDataIn.algo_image_size = MSize(mInitCfg.mAlgoImgWidth, mInitCfg.mAlgoImgHeight);
    MY_LOGD("main_image_size(%dx%d) algo_image_size(%dx%d)",
        sDataIn.main_image_size.w, sDataIn.main_image_size.h, sDataIn.algo_image_size.w, sDataIn.algo_image_size.h);
    //
    mpStereoHal = StereoHalBase::createInstance();
    if( !mpStereoHal || !mpStereoHal->STEREOInit(sDataIn, sDataOut) )
    {
        MY_LOGE("STEREOInit fail");
        goto lbExit;
    }
    //
    // TODO: FIX RRZ SETTING
    mpStereoHal->STEREOGetRrzInfo(sRrzData);
    pISC_main->setVideoSize(0, 0);
    pISC_main->setRrzoMinSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);
    pISC_main->setPreviewSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);
    pISC_main->setPass1InitRrzoSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);
    pISC_main->setInitialRrzoSize(sRrzData.rrz_crop_main1, sRrzData.rrz_size_main1);
    pISC_main2->setVideoSize(0, 0);
    pISC_main2->setRrzoMinSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);
    pISC_main2->setPreviewSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);
    pISC_main2->setPass1InitRrzoSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);
    pISC_main2->setInitialRrzoSize(sRrzData.rrz_crop_main2, sRrzData.rrz_size_main2);
    //
    pISC_main->calRrzoMaxZoomRatio();
    pISC_main2->calRrzoMaxZoomRatio();
    //
    ret = MTRUE;
lbExit:
    if(pISC_main)
    {
        pISC_main->destroyInstance();
        pISC_main = NULL;
    }
    if(pISC_main2)
    {
        pISC_main2->destroyInstance();
        pISC_main2 = NULL;
    }
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;

    if ( mpStereoHal )
    {
        mpStereoHal->STEREODestroy();
        mpStereoHal->destroyInstance();
        mpStereoHal = NULL;
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    MSize const     imgSize = MSize(mInitCfg.mAlgoImgWidth, mInitCfg.mAlgoImgHeight);
    MINT32 const    usage   = eBUFFER_USAGE_HW_RENDER|eBUFFER_USAGE_HW_TEXTURE|eBUFFER_USAGE_SW_WRITE_RARELY;
    MUINT32 const   format  = eImgFmt_RGBA8888;
    MUINT32 const planeCount= queryPlaneCount(format);
    MUINT32 bufStridesInBytes[] = {0,0,0};
    MINT32 bufBoundaryInBytes[] = {0,0,0};
    for (MUINT32 i = 0; i < planeCount; i++)
    {
        bufStridesInBytes[i] =
            (queryPlaneWidthInPixels(format,i, imgSize.w)*queryPlaneBitsPerPixel(format,i))>>3;
    }
    IImageBufferAllocator::ImgParam imgParam(
            format,
            imgSize,
            bufStridesInBytes,
            bufBoundaryInBytes,
            planeCount);
    IImageBufferAllocator::ExtraParam extParam(usage);
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    mpAlgoDstImgBuf = allocator->alloc_gb(LOG_TAG, imgParam, extParam);
    CHECK_OBJECT(mpAlgoDstImgBuf);

    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onStop()
{
    Mutex::Autolock lock(mLock);
    FUNC_START;
    list<PostBufInfo>::iterator bufIter;
    list<FeoBufInfo>::iterator feoIter;
    //
#define RET_BUFFER( postbuf, iter )                                             \
    for(iter = postbuf.begin(); iter != postbuf.end(); iter++)                  \
    {                                                                           \
        MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)", (*iter).data, (*iter).buf); \
        handleReturnBuffer((*iter).data, (MUINTPTR)((*iter).buf));               \
    }

    RET_BUFFER(mlPostBufMain, bufIter)
    RET_BUFFER(mlPostBufImg, bufIter)
    RET_BUFFER(mlPostBufImg_Main2, bufIter)
    RET_BUFFER(mlPostBufRgb, bufIter)
    RET_BUFFER(mlPostBufRgb_Main2, bufIter)
    RET_BUFFER(mlPostBufFeo, feoIter)
    RET_BUFFER(mlPostBufFeo_Main2, feoIter)

#undef RET_BUFFER

    if ( mpAlgoDstImgBuf != NULL )
    {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        allocator->free(mpAlgoDstImgBuf);
        mpAlgoDstImgBuf = NULL;
    }

    FUNC_END;
    return syncWithThread(); //wait for jobs done
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return pushBuf(data, buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL ret = MTRUE;
    MY_LOGD("data %d, buf 0x%x ext 0x%08X", data, buf, ext);
    switch(data)
    {
        case STEREO_CTRL_MAIN_DST:
            handleReturnBuffer(STEREO_CTRL_MAIN_SRC, buf);
            break;
        case STEREO_CTRL_DST_M:
            handleReturnBuffer(STEREO_CTRL_IMG_0, buf);
            break;
        case STEREO_CTRL_DST_S:
            handleReturnBuffer(STEREO_CTRL_IMG_1, buf);
            break;
        case STEREO_CTRL_RGB_0:
        case STEREO_CTRL_RGB_1:
            handleReturnBuffer(data, buf);
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    //
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
StereoCtrlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
    char dumppath[256];
    sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt, cnt)                \
        do{                                                         \
            IImageBuffer* buffer = (IImageBuffer*)pbuf;             \
            char filename[256];                                     \
            sprintf(filename, "%s%s_%d_%dx%d_%d.%s",                \
                    dumppath,                                       \
                    #type,                                          \
                    getSensorIdx(),                                 \
                    buffer->getImgSize().w,buffer->getImgSize().h,  \
                    cnt,                                            \
                    fileExt                                         \
                   );                                               \
            buffer->saveToFile(filename);                           \
        }while(0)

    if(!makePath(dumppath,0660))
    {
        MY_LOGE("makePath [%s] fail",dumppath);
        return;
    }

    switch( data )
    {
        case STEREO_CTRL_IMG_0:
            DUMP_IImageBuffer( STEREO_CTRL_IMG_0, buf, "yuv", muPostFrameCnt );
            break;
        case STEREO_CTRL_IMG_1:
            DUMP_IImageBuffer( STEREO_CTRL_IMG_1, buf, "yuv", muPostFrameCnt );
            break;
        default:
            MY_LOGE("not handle this yet data(%d)", data);
            break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
pushBuf(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);
    MUINT32 postBufSize = 0;

    if ( data == STEREO_CTRL_IMG_1 )    // TODO: check scenario type
    {
        if ( getStereoType() != STEREO_CTRL_CAPTURE )
        {
            handleReturnBuffer(data, buf);
            return MTRUE;
        }
    }

    if ( data == STEREO_CTRL_FEO_0 || data == STEREO_CTRL_FEO_1 )
    {
        FeoBufInfo feoBufData(data, (IMEM_BUF_INFO*)buf, ext);
        switch (data)
        {
            case STEREO_CTRL_FEO_0:
                mlPostBufFeo.push_back(feoBufData);
                postBufSize = mlPostBufFeo.size();
                break;
            case STEREO_CTRL_FEO_1:
                mlPostBufFeo_Main2.push_back(feoBufData);
                postBufSize = mlPostBufFeo_Main2.size();
                break;
            default:
                MY_LOGW("unsupported data(%d)", data);
                break;
        }
    }
    else
    {
        PostBufInfo postBufData(data, (IImageBuffer*)buf, ext);
        switch (data)
        {
            case STEREO_CTRL_MAIN_SRC:
                mlPostBufMain.push_back(postBufData);
                postBufSize = mlPostBufMain.size();
                break;
            case STEREO_CTRL_IMG_0:
                mlPostBufImg.push_back(postBufData);
                postBufSize = mlPostBufImg.size();
                break;
            case STEREO_CTRL_IMG_1:
                mlPostBufImg_Main2.push_back(postBufData);
                postBufSize = mlPostBufImg_Main2.size();
                break;
            case STEREO_CTRL_RGB_0:
                mlPostBufRgb.push_back(postBufData);
                postBufSize = mlPostBufRgb.size();
                break;
            case STEREO_CTRL_RGB_1:
                mlPostBufRgb_Main2.push_back(postBufData);
                postBufSize = mlPostBufRgb_Main2.size();
                break;
            default:
                MY_LOGW("unsupported data(%d)", data);
                break;
        }
    }
    MY_LOGD("size(%d), data(%d), buf(0x%x)", postBufSize, data, buf);

    if( isReadyToAlgo() )
    {
        muPostFrameCnt++;
        triggerLoop();
    }

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
isReadyToAlgo() const
{
    MBOOL ret = MFALSE;
    switch ( getStereoType() )
    {
        case STEREO_CTRL_PREVIEW:
        case STEREO_CTRL_RECORD:
            ret = ( mlPostBufImg.size() != 0
                && mlPostBufFeo.size() != 0
                && mlPostBufFeo_Main2.size() != 0 );
            break;
        case STEREO_CTRL_CAPTURE:
            ret = ( mlPostBufMain.size() != 0 && mlPostBufImg.size() != 0
                && mlPostBufFeo.size() != 0 && mlPostBufFeo_Main2.size() != 0
                && mlPostBufRgb.size() != 0 && mlPostBufRgb_Main2.size() != 0 );
            break;
        default:
            MY_LOGE("unsupported stereo type(%d)", getStereoType());
            break;
    }
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
threadLoopUpdate()
{
    MBOOL ret = MTRUE;
    //
    MBOOL isZoom = MFALSE;
    SET_DATA_STEREO_T sDataIn;
    OUT_DATA_STEREO_T sDataOut;
    PostBufInfo postBufMain;
    PostBufInfo postBufImg, postBufImg_Main2;
    FeoBufInfo  postBufFeo, postBufFeo_Main2;
    PostBufInfo postBufRgb, postBufRgb_Main2;
    {
        Mutex::Autolock lock(mLock);
        //
        if ( mlPostBufImg.size() == 0
            || mlPostBufFeo.size() == 0
            || mlPostBufFeo_Main2.size() == 0 )
        {
            MY_LOGW("skip threadloop: img(%d) feo(%d) feo(%d)",
                mlPostBufImg.size(), mlPostBufFeo.size(), mlPostBufFeo_Main2.size());
            return ret;
        }
        if ( getStereoType() == STEREO_CTRL_CAPTURE )
        {
            if ( mlPostBufMain.size() == 0 || mlPostBufImg_Main2.size() == 0
                || mlPostBufRgb.size() == 0 || mlPostBufRgb_Main2.size() == 0 )
            {
                MY_LOGW("skip threadloop: main(%d) img2(%d) rgb(%d) rgb2(%d)",
                    mlPostBufMain.size(), mlPostBufImg_Main2.size(), mlPostBufRgb.size(), mlPostBufRgb_Main2.size());
                return ret;
            }
            postBufMain     = mlPostBufMain.front();
            postBufImg_Main2= mlPostBufImg_Main2.front();
            postBufRgb      = mlPostBufRgb.front();
            postBufRgb_Main2= mlPostBufRgb_Main2.front();
            mlPostBufMain.pop_front();
            mlPostBufImg_Main2.pop_front();
            mlPostBufRgb.pop_front();
            mlPostBufRgb_Main2.pop_front();
            IImageBufferHeap* pSrcHeap  = postBufImg_Main2.buf->getImageBufferHeap();
            IImageBufferHeap* pDstHeap  = mpAlgoDstImgBuf->getImageBufferHeap();
            sDataIn.mSrcGraphicBuffer   = pSrcHeap->getGraphicBuffer();
            sDataIn.mDstGraphicBuffer   = pDstHeap->getGraphicBuffer();
            sDataIn.u4RgbaAddr_main1    = (void*)postBufRgb.buf->getBufVA(0);
            sDataIn.u4RgbaAddr_main2    = (void*)postBufRgb_Main2.buf->getBufVA(0);
        }
        //
        postBufImg      = mlPostBufImg.front();
        postBufFeo      = mlPostBufFeo.front();
        postBufFeo_Main2= mlPostBufFeo_Main2.front();
        mlPostBufImg.pop_front();
        mlPostBufFeo.pop_front();
        mlPostBufFeo_Main2.pop_front();
        isZoom = (MBOOL)postBufImg.ext;
        //
        sDataIn.u4FEBufAddr_main1 = (void*)postBufFeo.buf->virtAddr;
        sDataIn.u4FEBufAddr_main2 = (void*)postBufFeo_Main2.buf->virtAddr;
        //
        MY_LOGD("postCnt:%d  main(0x%x) img(0x%x/0x%x) rgb(0x%x/0x%x) feo(%d:0x%x/%d:0x%x), isZoom(%d)",
            muPostFrameCnt, postBufFeo_Main2.buf,
            postBufImg.buf, postBufImg_Main2.buf,
            postBufRgb.buf, postBufRgb_Main2.buf,
            postBufFeo.buf->memID, postBufFeo.buf->virtAddr,
            postBufFeo_Main2.buf->memID, postBufFeo_Main2.buf->virtAddr,
            isZoom);
    }
    //
    if ( !isZoom )
    {
        if( !mpStereoHal || !mpStereoHal->STEREOSetParams(sDataIn) )
        {
            MY_LOGE("STEREOSetParams fail");
            goto lbExit;
        }
        if( !mpStereoHal || !mpStereoHal->STEREORun(sDataOut) )
        {
            MY_LOGE("STEREORun fail");
            goto lbExit;
        }
    }
    else
    {
        // TODO: fix me. should call stereo api
        sDataOut.main_crop.s = postBufImg.buf->getImgSize();
    }
    //
    // TODO: error handle
lbExit:
    //
    #if 0 //AARON FIXME
    handlePostBuffer( STEREO_CTRL_DST_M , (MUINTPTR)postBufImg.buf, (MUINT32)&sDataOut.algo_main1 );
    #else
    handlePostBuffer( STEREO_CTRL_DST_M , (MUINTPTR)postBufImg.buf, 0 );
    #endif
//    MY_LOGD("retBuffer: data(%d/%d)  buf(0x%x/0x%x)", postBufFeo.data, postBufFeo_Main2.data, postBufFeo.buf, postBufFeo_Main2.buf);
    handleReturnBuffer(postBufFeo.data, (MUINTPTR)postBufFeo.buf);
    handleReturnBuffer(postBufFeo_Main2.data, (MUINTPTR)postBufFeo_Main2.buf);
    //
    if ( getStereoType() == STEREO_CTRL_CAPTURE )
    {
        #if 0 //AARON FIXME
        handlePostBuffer( STEREO_CTRL_MAIN_DST , (MUINTPTR)postBufMain.buf, (MUINT32)&sDataOut.main_crop );
        handlePostBuffer( STEREO_CTRL_DST_S , (MUINTPTR)postBufImg_Main2.buf, (MUINT32)&sDataOut.algo_main2 );
        #else
        handlePostBuffer( STEREO_CTRL_MAIN_DST , (MUINTPTR)postBufMain.buf, 0 );
        handlePostBuffer( STEREO_CTRL_DST_S , (MUINTPTR)postBufImg_Main2.buf, 0 );
        #endif
//        MY_LOGD("retBuffer: data(%d/%d)  buf(0x%x/0x%x)", postBufRgb.data, postBufRgb_Main2.data, postBufRgb.buf, postBufRgb_Main2.buf);
        handleReturnBuffer(postBufRgb.data, (MUINTPTR)postBufRgb.buf);
        handleReturnBuffer(postBufRgb_Main2.data, (MUINTPTR)postBufRgb_Main2.buf);
    }
    //
//    FUNC_END;
    return ret;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

