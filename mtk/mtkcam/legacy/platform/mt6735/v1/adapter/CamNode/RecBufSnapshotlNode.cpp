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
#define LOG_TAG "MtkCam/RBSSNode"
//
#include <utils/RefBase.h>
#include <CamUtils.h>
using namespace android;
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <semaphore.h>
using namespace std;
//
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <mtkcam/iopipe/SImager/ISImager.h>
using namespace NSIoPipe::NSSImager;
//
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
#include <mtkcam/hwutils/HwMisc.h>
//
#include <Scenario/Shot/IShot.h>
using namespace NSShot;
//
#include <CamNode/RecBufSnapshotlNode.h>
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#else
#define MY_LOGV(fmt, arg...)       CAM_LOGV("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGD(fmt, arg...)       CAM_LOGD("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_LOGI("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_LOGW("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_LOGE("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGA(fmt, arg...)       CAM_LOGA("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGF(fmt, arg...)       CAM_LOGF("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#endif

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")
#define FUNC_NAME  MY_LOGD("")

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_NAME        "RecBufSnapshotlNode"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_CAPTURE)

/*******************************************************************************
*
********************************************************************************/


class RecBufSnapshotlNodeImpl : public RecBufSnapshotlNode
{
    public: // ctor & dtor
        RecBufSnapshotlNodeImpl(sp<IParamsManager> spParamsMgr);
        ~RecBufSnapshotlNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations
        virtual MBOOL encodeJpg(MUINT32 bufType);
        virtual MBOOL integrateJpg();
        virtual MBOOL callbackJpg();
        virtual MBOOL startRecording(
                        MUINT32 width,
                        MUINT32 height,
                        MUINT32 format);

        virtual MBOOL stopRecording();
        virtual MBOOL takePicture(
                        MUINT32             thumbWidth,
                        MUINT32             thumbHeight,
                        MUINT32             rotation,
                        MUINT32             jpegQuality,
                        MUINT32             jpegQualityThumb,
                        sp<IShotCallback>   pShotCallback);

    private:
        #define REC_IMG_STRIDE_Y    (16)
        #define REC_IMG_STRIDE_U    (16)
        #define REC_IMG_STRIDE_V    (16)

        #define ALIGN_UP_SIZE(in,align)     ((in+align-1) & ~(align-1))
        #define ALIGN_DOWN_SIZE(in,align)   (in & ~(align-1))

        typedef struct
        {
            MUINT32 planeCnt;
            MUINTPTR addr[3];
            MUINT32 size[3];
            MUINT32 stride[3];
        }REC_BUF_INFO;

        enum
        {
            BUF_YUV_MAIN,
            BUF_JPG_MAIN,
            BUF_JPG_THUMB,
            BUF_JPG_MAIN_R,
            BUF_JPG_ALL,
            BUF_AMOUNT
        };

        MBOOL allocBuffer(
                IImageBuffer*&      pImageBuffer,
                MUINT32             width,
                MUINT32             height,
                MUINT32             format);
        MBOOL freeBuffer(IImageBuffer*& pImageBuffer);

        mutable Mutex       mLock;
        Condition           mCond;
        MBOOL               mbTakePic;
        MBOOL               mbWaitImg;
        MBOOL               mbRecord;
        MBOOL               mIsThumb;
        MUINT32             mRotation;
        MUINT32             mJpegQuality;
        MUINT32             mJpegQualityThumb;
        MUINT32             mJpgSize;
        REC_BUF_INFO        mRecBufInfo;
        IImageBuffer*       mpImageBuffer[BUF_AMOUNT];
        sp<IShotCallback>   mpShotCallback;
        sp<IParamsManager>  mspParamsMgr;
};


/*******************************************************************************
*
********************************************************************************/
RecBufSnapshotlNode*
RecBufSnapshotlNode::
createInstance(sp<IParamsManager> spParamsMgr)
{
    return new RecBufSnapshotlNodeImpl(spParamsMgr);
}


/*******************************************************************************
*
********************************************************************************/
void
RecBufSnapshotlNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
RecBufSnapshotlNode::
RecBufSnapshotlNode()
    : ICamThreadNode( MODULE_NAME, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
*
********************************************************************************/
RecBufSnapshotlNode::
~RecBufSnapshotlNode()
{
}


/*******************************************************************************
*
********************************************************************************/
RecBufSnapshotlNodeImpl::
RecBufSnapshotlNodeImpl(sp<IParamsManager> spParamsMgr)
    : mbTakePic(MFALSE)
    , mbWaitImg(MFALSE)
    , mbRecord(MFALSE)
    , mIsThumb(MFALSE)
    , mRotation(0)
    , mspParamsMgr(spParamsMgr)
{
    //addSupport
    addDataSupport( ENDPOINT_SRC , RECBUFSS_SRC);
}


/*******************************************************************************
*
********************************************************************************/
RecBufSnapshotlNodeImpl::
~RecBufSnapshotlNodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret = syncWithThread(); //wait for jobs done
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNC_START;
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNC_START;

    if(mbWaitImg)
    {
        MUINT32 i;
        {
            Mutex::Autolock lock(mLock);
            mbWaitImg = MFALSE;
            mbTakePic = MTRUE;
        }
        IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
        //
        mRecBufInfo.planeCnt = pImageBuffer->getPlaneCount();
        MY_LOGD("planeCnt(%d)",mRecBufInfo.planeCnt);
        for(i=0; i<mRecBufInfo.planeCnt; i++)
        {
            mRecBufInfo.addr[i] = pImageBuffer->getBufVA(i);
            mRecBufInfo.size[i] = pImageBuffer->getBufSizeInBytes(i);
            mRecBufInfo.stride[i] = pImageBuffer->getBufStridesInBytes(i);
            MY_LOGD("addr(0x%X),size(%d),stride(%d)",
                    mRecBufInfo.addr[i],
                    mRecBufInfo.size[i],
                    mRecBufInfo.stride[i]);
        }
        triggerLoop();
    }
    handlePostBuffer(data, buf, ext);
    //FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    handleReturnBuffer(data, buf, ext);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
RecBufSnapshotlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //TBD
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    MUINT32 i, size, idxStart, idxEnd;
    MUINTPTR addr;
    //
    Mutex::Autolock lock(mLock);
    //
    for(i=0; i<mRecBufInfo.planeCnt; i++)
    {
        MY_LOGD("Plane(%d),Size(%d -> %d)",
                i,
                mRecBufInfo.size[i],
                mpImageBuffer[BUF_YUV_MAIN]->getBufSizeInBytes(i));
        if(mpImageBuffer[BUF_YUV_MAIN]->getBufSizeInBytes(i) >= mRecBufInfo.size[i])
        {
            MY_LOGD("DST(0x%X/%d),SRC(0x%X/%d)",
                    mpImageBuffer[BUF_YUV_MAIN]->getBufVA(i),
                    mpImageBuffer[BUF_YUV_MAIN]->getBufSizeInBytes(i),
                    mRecBufInfo.addr[i],
                    mRecBufInfo.size[i]);
            memcpy(
                (MUINT8*)(mpImageBuffer[BUF_YUV_MAIN]->getBufVA(i)),
                (MUINT8*)(mRecBufInfo.addr[i]),
                mRecBufInfo.size[i]);
        }
        else
        {
            MY_LOGE("Size %d > %d",
                    mRecBufInfo.size[i],
                    mpImageBuffer[BUF_YUV_MAIN]->getBufSizeInBytes(i));
        }
    }
    //main & thumb: YUV to JPG
    if( mRotation == 0 ||
        mRotation == 180)
    {
        encodeJpg(BUF_JPG_MAIN);
    }
    else
    {
        encodeJpg(BUF_JPG_MAIN_R);

    }
    //
    if(mIsThumb)
    {
        encodeJpg(BUF_JPG_THUMB);
    }
    //
    integrateJpg();
    callbackJpg();
    mbTakePic = MFALSE;
    mCond.broadcast();
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
encodeJpg(MUINT32 bufType)
{
    FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    MY_LOGD("bufType(%d)",bufType);
    ISImager *pISImager = ISImager::createInstance(mpImageBuffer[BUF_YUV_MAIN]);
    if( !pISImager )
    {
        ret = MFALSE;
        return ret;
    }
    //
    pISImager->setTargetImgBuffer(mpImageBuffer[bufType]);
    //
    MUINT32 transform = 0;
    switch(mRotation)
    {
        case 0:
            transform = 0;
            break;
        case 90:
            transform = eTransform_ROT_90;
            break;
        case 180:
            transform = eTransform_ROT_180;
            break;
        case 270:
            transform = eTransform_ROT_270;
            break;
        default:
            break;
    }
    pISImager->setTransform(transform);
    //
    MBOOL isSOI;
    if(bufType == BUF_JPG_THUMB)
    {
        isSOI = MTRUE;
        pISImager->setEncodeParam(
                        isSOI,
                        mJpegQualityThumb);
    }
    else
    {
        isSOI = MFALSE;
        pISImager->setEncodeParam(
                        isSOI,
                        mJpegQuality);
    }
    //
    if(bufType == BUF_JPG_THUMB)
    {
        int imgW, imgH;
        switch(mRotation)
        {
            case 0:
            case 180:
                imgW = mpImageBuffer[BUF_JPG_THUMB]->getImgSize().w;
                imgH = mpImageBuffer[BUF_JPG_THUMB]->getImgSize().h;
                break;
            case 90:
            case 270:
                imgW = mpImageBuffer[BUF_JPG_THUMB]->getImgSize().h;
                imgH = mpImageBuffer[BUF_JPG_THUMB]->getImgSize().w;
                break;
            default:
                break;
        }
        NSCamHW::Rect SrcRect(0, 0, mpImageBuffer[BUF_YUV_MAIN]->getImgSize().w, mpImageBuffer[BUF_YUV_MAIN]->getImgSize().h);
        NSCamHW::Rect DstRect(0, 0, imgW, imgH);
        NSCamHW::Rect CropRect = MtkCamUtils::calCrop(SrcRect, DstRect, 100);
        MY_LOGD("SrcRect: %d, %d",
                SrcRect.w,
                SrcRect.h);
        MY_LOGD("DstRect: %d, %d",
                DstRect.w,
                DstRect.h);
        MY_LOGD("Crop for Thumb: %d, %d, %d, %d",
                CropRect.x,
                CropRect.y,
                CropRect.w,
                CropRect.h);
        pISImager->setCropROI(
                    MRect(MPoint(CropRect.x, CropRect.y),
                    MSize(  CropRect.w,
                            CropRect.h)));
    }
    else
    {
        pISImager->setCropROI(
                        MRect(MPoint(0, 0),
                        MSize(  mpImageBuffer[BUF_YUV_MAIN]->getImgSize().w,
                                mpImageBuffer[BUF_YUV_MAIN]->getImgSize().h)));
    }
    pISImager->execute();
    pISImager->destroyInstance();
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
integrateJpg()
{
    FUNC_START;
    //
    MBOOL ret = MTRUE;
    MUINT32 bufTypeMain, exifHeaderSize = 0, debugExifHeaderSize = 0;
    CamExif jpgExif;
    CamExifParam jpgExifParam;
    CamDbgParam jpgDbgParam;
    IHal3A* p3AHal = IHal3A::createInstance(
                                IHal3A::E_Camera_1,
                                getSensorIdx(),
                                LOG_TAG);
    //
    mJpgSize = 0;
    //
    if( !mspParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).isEmpty() &&
        !mspParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).isEmpty())
    {
        jpgExifParam.u4GpsIsOn = 1;
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSLatitude),           mspParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).string(),           mspParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).length());
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSLongitude),          mspParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).string(),          mspParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).length());
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSTimeStamp),          mspParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).string(),          mspParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).length());
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSProcessingMethod),   mspParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).string(),  mspParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).length());
        jpgExifParam.u4GPSAltitude = ::atoi(mspParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE).string());
    }
    else
    {
        MY_LOGD("No GPS data");
    }
    //
    jpgExifParam.u4Orientation = 0;
    jpgExifParam.u4ZoomRatio = 0;
    jpgExifParam.u4Facing = NSCam::IHalSensorList::get()->queryFacingDirection(getSensorIdx());
    //
    jpgDbgParam.u4CamMode = eAppMode_VideoMode;
    jpgDbgParam.u4ShotMode = 0;
    //
    jpgExif.init(
        jpgExifParam,
        jpgDbgParam);
    //
    p3AHal->set3AEXIFInfo(&jpgExif);
    //
    if( mRotation == 0 ||
        mRotation == 180)
    {
        bufTypeMain = BUF_JPG_MAIN;
    }
    else
    {
        bufTypeMain = BUF_JPG_MAIN_R;
    }
    //
    MY_LOGD("Size(%dx%d)",
            mpImageBuffer[bufTypeMain]->getImgSize().w,
            mpImageBuffer[bufTypeMain]->getImgSize().h);
    if(mIsThumb)
    {
        jpgExif.makeExifApp1(
            mpImageBuffer[bufTypeMain]->getImgSize().w,
            mpImageBuffer[bufTypeMain]->getImgSize().h,
            mpImageBuffer[BUF_JPG_THUMB]->getBitstreamSize(),
            (MUINT8*)(mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)),
            &exifHeaderSize);
    }
    else
    {
        jpgExif.makeExifApp1(
            mpImageBuffer[bufTypeMain]->getImgSize().w,
            mpImageBuffer[bufTypeMain]->getImgSize().h,
            0,
            (MUINT8*)(mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)),
            &exifHeaderSize);
    }
    //
    mJpgSize += exifHeaderSize;
    MY_LOGD("JPG bitStrSize:Add exif(%d/%d)",
            exifHeaderSize,
            mJpgSize);
    //
    if(mIsThumb)
    {
        MY_LOGD("memcpy:DST(0x%08X + %d = 0x%08X),SRC(0x%08X),Size(%d)",
                mpImageBuffer[BUF_JPG_ALL]->getBufVA(0),
                mJpgSize,
                mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)+mJpgSize,
                mpImageBuffer[BUF_JPG_THUMB]->getBufVA(0),
                mpImageBuffer[BUF_JPG_THUMB]->getBitstreamSize());
        memcpy(
            (MUINT8*)(mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)+mJpgSize),
            (MUINT8*)(mpImageBuffer[BUF_JPG_THUMB]->getBufVA(0)),
            mpImageBuffer[BUF_JPG_THUMB]->getBitstreamSize());
        mJpgSize += mpImageBuffer[BUF_JPG_THUMB]->getBitstreamSize();
        MY_LOGD("JPG bitStrSize:Add Thumbnail(%d/%d)",
                mpImageBuffer[BUF_JPG_THUMB]->getBitstreamSize(),
                mJpgSize);
    }
    //
    //p3AHal->setDebugInfo(&jpgExif);
    //mpSensorHal->setDebugInfo(&jpgExif);
    jpgExif.appendDebugExif(
        (MUINT8*)(mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)+mJpgSize),
        &debugExifHeaderSize);
    mJpgSize += debugExifHeaderSize;
    MY_LOGD("JPG bitStrSize:Add debug exif(%d/%d)",
            debugExifHeaderSize,
            mJpgSize);
    //
    MY_LOGD("memcpy:DST(0x%08X + %d = 0x%08X),SRC(0x%08X),Size(%d)",
            mpImageBuffer[BUF_JPG_ALL]->getBufVA(0),
            mJpgSize,
            mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)+mJpgSize,
            mpImageBuffer[bufTypeMain]->getBufVA(0),
            mpImageBuffer[bufTypeMain]->getBitstreamSize());
    memcpy(
        (MUINT8*)(mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)+mJpgSize),
        (MUINT8*)(mpImageBuffer[bufTypeMain]->getBufVA(0)),
        mpImageBuffer[bufTypeMain]->getBitstreamSize());
    //
    mJpgSize += mpImageBuffer[bufTypeMain]->getBitstreamSize();
    MY_LOGD("JPG bitStrSize:Add Main(%d/%d)",
            mpImageBuffer[bufTypeMain]->getBitstreamSize(),
            mJpgSize);
    //
    p3AHal->destroyInstance(LOG_TAG);
    jpgExif.uninit();
    //
    if(mIsThumb)
    {
        freeBuffer(mpImageBuffer[BUF_JPG_THUMB]);
    }
    //
    MY_LOGD("mJpgSize(%d)",mJpgSize);
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
callbackJpg()
{
    FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    if(mpShotCallback != NULL)
    {
        mpShotCallback->onCB_Shutter(true, 0);
        mpShotCallback->onCB_RawImage(0, 0, NULL);
        mpShotCallback->onCB_CompressedImage(
                            0,
                            mJpgSize,
                            (MUINT8*)(mpImageBuffer[BUF_JPG_ALL]->getBufVA(0)),
                            0,
                            NULL,
                            0,
                            MTRUE);
    }
    else
    {
        MY_LOGE("mpShotCallback is NULL");
    }
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
startRecording(
    MUINT32 width,
    MUINT32 height,
    MUINT32 format)
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    MY_LOGD("VDO(%dx%d)",
            width,
            height);
    //
    allocBuffer(
        mpImageBuffer[BUF_YUV_MAIN],
        width,
        height,
        format);
    allocBuffer(
        mpImageBuffer[BUF_JPG_MAIN],
        width,
        height,
        eImgFmt_JPEG);
    allocBuffer(
        mpImageBuffer[BUF_JPG_MAIN_R],
        height,
        width,
        eImgFmt_JPEG);
    allocBuffer(
        mpImageBuffer[BUF_JPG_ALL],
        width,
        height,
        eImgFmt_JPEG);
    //
    mbRecord = MTRUE;
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
stopRecording()
{
    Mutex::Autolock lock(mLock);
    //
    if( mbTakePic ||
        mbWaitImg)
    {
        MY_LOGD("Wait lock");
        mCond.wait(mLock);
    }
    //
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    MY_LOGD("Record(%d)",mbRecord);
    if(mbRecord)
    {
        freeBuffer(mpImageBuffer[BUF_YUV_MAIN]);
        freeBuffer(mpImageBuffer[BUF_JPG_MAIN]);
        freeBuffer(mpImageBuffer[BUF_JPG_MAIN_R]);
        freeBuffer(mpImageBuffer[BUF_JPG_ALL]);
    }
    mbRecord = MFALSE;
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
takePicture(
    MUINT32             thumbWidth,
    MUINT32             thumbHeight,
    MUINT32             rotation,
    MUINT32             jpegQuality,
    MUINT32             jpegQualityThumb,
    sp<IShotCallback>   pShotCallback)
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    MY_LOGD("Thumb(%dx%d),rotation(%d)",
            thumbWidth,
            thumbHeight,
            rotation);
    //
    mRotation = rotation;
    mJpegQuality = jpegQuality;
    mJpegQualityThumb = jpegQualityThumb;
    mpShotCallback = pShotCallback;
    //
    //
    if( thumbWidth == 0 ||
        thumbHeight == 0)
    {
        mIsThumb = MFALSE;
    }
    else
    {
        mIsThumb = MTRUE;
    }
    //
    if(mIsThumb)
    {
        allocBuffer(
            mpImageBuffer[BUF_JPG_THUMB],
            thumbWidth,
            thumbHeight,
            eImgFmt_JPEG);
    }
    //
    mbWaitImg = MTRUE;
    MY_LOGD("Wait lock");
    mCond.wait(mLock);
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
allocBuffer(
    IImageBuffer*&      pImageBuffer,
    MUINT32             width,
    MUINT32             height,
    MUINT32             format)
{
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    //allocate buffer
    MUINT32 bufStrideInByte[3] = {  ALIGN_UP_SIZE(width,    REC_IMG_STRIDE_Y),
                                    ALIGN_UP_SIZE(width/2,  REC_IMG_STRIDE_U),
                                    ALIGN_UP_SIZE(width/2,  REC_IMG_STRIDE_V)};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

    MY_LOGD("S(%dx%d), fmt(0x%x), stride(%d/%d/%d)",
            width,
            height,
            format,
            bufStrideInByte[0],
            bufStrideInByte[1],
            bufStrideInByte[2]);


    if(format == eImgFmt_JPEG )
    {
        MUINT32 bufsize = width*height + DBG_EXIF_SIZE;
        IImageBufferAllocator::ImgParam imgParam(
                MSize(width,height),
                bufsize,
                0);
        pImageBuffer = allocator->alloc_ion(LOG_TAG, imgParam);

    }
    else
    {
        IImageBufferAllocator::ImgParam imgParam(
                format,
                MSize(width,height),
                bufStrideInByte,
                bufBoundaryInBytes,
                queryPlaneCount(format));
        pImageBuffer = allocator->alloc_ion(LOG_TAG, imgParam);
    }

    if(!pImageBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_MASK))
    {
        MY_LOGE("lock Buffer failed\n");
        return  MFALSE;
    }

    if ( !pImageBuffer->syncCache( eCACHECTRL_INVALID ) )
    {
        MY_LOGE("syncCache failed\n");
        return  MFALSE;
    }

    MY_LOGD("Done");
    return MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
RecBufSnapshotlNodeImpl::
freeBuffer(IImageBuffer*& pImageBuffer)
{
    MBOOL ret = MTRUE;
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    if(!pImageBuffer->unlockBuf(LOG_TAG))
    {
        CAM_LOGE("unlock Buffer failed\n");
        ret = MFALSE;
    }
    allocator->free(pImageBuffer);
    return ret;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

