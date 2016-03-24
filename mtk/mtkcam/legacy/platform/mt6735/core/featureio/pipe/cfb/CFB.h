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
#ifndef _MAHL_FEATUREIO_CFB_H_
#define _MAHL_FEATUREIO_CFB_H_

#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <fcntl.h>
#include <linux/cache.h>
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;

#include <mtkcam/common/faces.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>

#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/utils/ImageBufferHeap.h>
#include <mtkcam/hal/IHalSensor.h>
//#include <mtkcam/hal/sensor_hal.h>

#include <Shot/IShot.h>

#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
#include <mtkcam/camshot/ISmartShot.h>
#include <mtkcam/iopipe/SImager/ISImager.h>
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>

#include "ImpShot.h"
#include <mtkcam/featureio/facebeautify_hal_base.h>
#include <mtkcam/sdk/hal/IFaceBeautySDKHal.h>
#include <mtkcam/exif/IDbgInfoContainer.h>

#include "FaceBeautyEffectHal.h"
#include "SDKFaceBeauty.h"

//-----vfb--------
#include <mtkcam/featureio/vfb_hal_base.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <mtkcam/featureio/IHal3A.h>
#include "MTKVideoFaceBeauty.h"
//----------------

using namespace android;
using namespace NSShot;
using namespace NSCamShot;

using namespace NSIoPipe;
using namespace NSPostProc;
using namespace NS3A;

//#define USING_MTK_PERFSERVICE
#define FD_INIT_NO_ERROR 0

// Interface for SDK HAL
#if 0
MBOOL CaptureFaceBeautySDK_apply(
        IImageBuffer const* SrcImgBuffer,
        IImageBuffer const* DstImgBuffer,
        FACE_BEAUTY_SDK_HAL_PARAMS const* pParam,
        EImageFormat const ImageFormat = eImgFmt_I422);
void CaptureFaceBeautySDK_init(void);
void CaptureFaceBeautySDK_uninit(void);

void SetCallBackSDK(FaceBeautyEffectHal* effecthalobject);
#endif

class my_encode_params
{
	public:
		// buffer
		IImageBuffer*			pSrc;
		IImageBuffer*			pDst;

		// settings
		MUINT32 				transform;
		MRect					crop;
		MUINT32 				isSOI;
		MUINT32 				quality;
		MUINT32 				codecType;
};


/*******************************************************************************
*
*******************************************************************************/
namespace NSCam {

class Mhal_facebeauty :public RefBase
{
public:
    MUINT32         mu4W_yuv;       //  YUV Width
    MUINT32         mu4H_yuv;       //  YUV Height
    MUINT32         mDSWidth;
    MUINT32         mDSHeight;

//-----VFB------
halVFBBase*      mpVfbHal;
MUINT8*          mpWorkingBuffer;
MINT32*		     mpClearWarpMapX;
MINT32*		     mpClearWarpMapY;
MUINT32          mSensorType;
MUINT32          FDWidth;
MUINT32          FDHeight;
MUINT32          mthmSize;
MBOOL            IsVFB;


MtkCameraFace mvfb_FDFaceInfo[15];
MtkFaceInfo   mvfb_FDPoseInfo[15];
MtkCameraFaceMetadata  mvfb_FBmetadata;

IImageBuffer* buffer_DS_Input;
IImageBuffer* buffer_DS_Output;

IImageBuffer* buffer_ALPCL;
IImageBuffer* buffer_ALPNR;
IImageBuffer* buffer_PCA;
IImageBuffer* buffer_warp_GPU;
//IImageBuffer* jpegBuf;
IImageBuffer* thumbBuf;



//---------------

//-------p2b-----
IFeatureStream*  mpIFeatureStream; 
NS3A::IHal3A*    mp3AHal;
//	output buffer.
IImageBuffer* mpOutputbuffer;
IImageBuffer* p2b_tempinput;
//--------------

public:  ////    Buffers.
    //
    void* mpWorkingBuferr;
    MUINT32 FBWorkingBufferSize;
    //  Source.
    IImageBuffer* mpSource;
    //  alpha map.
    IImageBuffer* mpAmap;
    IImageBuffer* mpAmapForStep5;
    //  Blurred image
    IImageBuffer* mpBlurImg;
    //  DS image
    IImageBuffer* mpDSImg;

    //  Postview image
    IImageBuffer* mpPostviewImgBuf;

    MTKPipeFaceBeautyResultInfo msFaceBeautyResultInfo;
    MUINT8*         mpFDWorkingBuffer;
public:
    halFACEBEAUTIFYBase*     mpFb;
    IMemDrv*        mpIMemDrv;
    IImageBufferAllocator* mpIImageBufAllocator;

protected:  ////    Resolutions.
    MUINT32         mPostviewWidth;
    MUINT32         mPostviewHeight;
    MUINT32         mStep1Width;
    MUINT32         mStep1Height;

protected:  ////    Info.
    MtkCameraFace FBFaceInfo[15];
    MtkFaceInfo MTKPoseInfo[15];
    MtkCameraFaceMetadata  FBmetadata;

public:  ////    Tuning parameter.
    MINT32          mSmoothLevel;
    MINT32          mEnlargeEyeLevel;
    MINT32          mSlimFaceLevel;
    MINT32          mBrightLevel;
    MINT32          mRuddyLevel;
    MINT32          mContrastLevel;
    MINT32          mExtremeBeauty;
    MBOOL           mSDKMode;

    typedef struct
    {
        int32_t x;
        int32_t y;
    } FACE_BEAUTY_POS;
    Vector<FACE_BEAUTY_POS> mBeautifiedFacePos;

public:
    typedef struct
    {
        IImageBuffer* pImgBuf;
        IMEM_BUF_INFO memBuf;
    } ImageBufferMap;
    Vector<ImageBufferMap> mvImgBufMap;
    MUINT32         mTransform;
    MBOOL           mCancel;
    Mutex           mUninitMtx;

    //SDK
    //MBOOL                   mSDKMode;
    IImageBuffer const*     mSrcImgBuffer;
    IImageBuffer const*     mDstImgBuffer;
    IDbgInfoContainer *     mDebugInfo;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    Mhal_facebeauty(IImageBuffer const* SrcImgBuffer, IImageBuffer const* DstImgBuffer);
    virtual ~Mhal_facebeauty()  {}
    virtual bool    onCreate(MtkCameraFaceMetadata* FaceInfo, Vector<FACE_BEAUTY_POS> const& FacePos);    
    virtual void    onDestroy();    
    virtual bool    sendCommand(uint32_t const  cmd, MUINTPTR const  arg1, uint32_t const  arg2, uint32_t const  arg3 = 0);
    //virtual bool    setCallback(sp<IShotCallback>& rpShotCallback);
    virtual MBOOL   doCapture();
    virtual MBOOL   SaveJpg();
    virtual MBOOL   WaitSaveDone();
    virtual MBOOL   doStep2And3And5();
    //
    virtual MBOOL   apply();
    virtual MBOOL   SDKcreateFBJpegImg();

//------VFB---------------------
//    virtual MBOOL   init_VFB();
//    virtual MBOOL   uninit_VFB();
//    virtual MBOOL   do_VFB();
//    virtual MBOOL   querySensorInfo(NSCam::SensorStaticInfo &mSensorInfo);
    virtual MBOOL   postionFacesInImage(int u4PostviewWidth, int u4PostviewHeight);
    virtual MBOOL   do_thumbnail();
    virtual MBOOL   thumbnail_encode();
//------------------------------

//------p2b---------------------
//    virtual MBOOL   initP2b();
//    virtual MBOOL   UninitP2b();
//    virtual MBOOL   doP2b();
//    virtual MVOID   setPCA(IImageBuffer* pImageBuffer);
//    static  MVOID   p2bCbFunc(QParams& rParams);
//------------------------------

	
protected:  ////    Capture command.
    virtual MBOOL   onCmd_capture();
    virtual MBOOL   onCmd_reset();
    virtual MBOOL   onCmd_cancel();

protected:  ////                    callbacks
    static MBOOL fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg);
    static MBOOL fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg);

protected:
    MBOOL handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size);
    MBOOL handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT32 const Mode);
    MBOOL handleYuvDataCallback(MUINT8* const puBuf, MUINT32 const u4Size);

public:  ////    Invoked by capture().
    virtual MBOOL   createJpegImg(IImageBuffer const * rSrcImgBufInfo, NSCamShot::JpegParam const & rJpgParm, MUINT32 const u4Transform, IImageBuffer const * rJpgImgBufInfo, MUINT32 & u4JpegSize);
    virtual MBOOL   createJpegImgWithThumbnail(IImageBuffer const *rYuvImgBufInfo, IImageBuffer const *rPostViewBufInfo, MUINT32 const Mode);
    virtual MBOOL   createFBJpegImg(IImageBuffer* Srcbufinfo, int u4SrcWidth, int u4SrcHeight, MUINT32 const Mode);
    virtual MBOOL   createFullFrame(IImageBuffer* Srcbufinfo);
    virtual MBOOL   doFaceDetection(IImageBuffer* Srcbufinfo, int u4SrcWidth, int u4SrcHeight, int u4PostviewWidth, int u4PostviewHeight, MBOOL bSkipDetect = false);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:  ////    Buffers.
    virtual MBOOL   requestBufs();
    virtual MBOOL   releaseBufs();
    virtual IImageBuffer* allocMem(MUINT32 fmt, MUINT32 w, MUINT32 h);
    virtual void    deallocMem(IImageBuffer* pBuf);
    virtual IImageBuffer* resizeMem(IImageBuffer* pBuf, MUINT32 w, MUINT32 h);
    virtual MBOOL   ImgProcess(IImageBuffer const* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srctype, IImageBuffer const* Desbufinfo, MUINT32 desWidth, MUINT32 desHeight, EImageFormat destype, MUINT32 transform = 0) const;

protected:  ////    Misc.
    virtual MBOOL   InitialAlgorithm(MUINT32 srcWidth, MUINT32 srcHeight, MINT32 gBlurLevel, MINT32 FBTargetColor);
    //virtual MBOOL   ImgProcess(IImageBuffer const* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srctype, IImageBuffer const* Desbufinfo, MUINT32 desWidth, MUINT32 desHeight, EImageFormat destype, MUINT32 transform = 0) const;
    virtual MBOOL   STEP2(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* DSbufinfo, MtkCameraFaceMetadata* FaceInfo, void* FaceBeautyResultInfo);
    virtual MBOOL   STEP3(IImageBuffer* Srcbufinfo, void* FaceBeautyResultInfo) const;
    virtual MBOOL   STEP1(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* Desbufinfo, IImageBuffer* NRbufinfo, void* FaceBeautyResultInfo);
    virtual MBOOL   STEP4(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* Blurbufinfo, IImageBuffer* Alphabufinfo, void* FaceBeautyResultInfo);
    virtual MBOOL   STEP5(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* Alphabufinfo, void* FaceBeautyResultInfo);
    virtual MBOOL   STEP6(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* tmpbufinfo, void* FaceBeautyResultInfo) const;
    virtual MBOOL   CancelAllSteps();

#ifdef USING_MTK_PERFSERVICE
    virtual MBOOL   initPerf();
    virtual MBOOL   uninitPerf();
    virtual MBOOL   enablePerfService(MUINT32 scen);
    virtual MBOOL   disablePerfService(MUINT32 scen);
#endif

};
};

#endif  //  _MAHL_FEATUREIO_CFB_H_

