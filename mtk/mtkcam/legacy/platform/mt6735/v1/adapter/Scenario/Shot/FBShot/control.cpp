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
#define LOG_TAG "MtkCam/FBShot"

#include "Facebeauty.h"
#include <mtkcam/camnode/ICamGraphNode.h>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] \n" fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] \n" fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] \n" fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] \n" fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] \n" fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] \n" fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] \n" fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

static sem_t semSTEP1Done;
static sem_t semSTEP3Done;
static sem_t semSTEP4Done;
static sem_t semSTEP5Done;

//#define Debug_Mode

#ifdef Debug_Mode
#include <stdio.h>

int count=0;

static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}
#endif

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
InitialAlgorithm(MUINT32 srcWidth, MUINT32 srcHeight, MINT32 gBlurLevel, MINT32 FBTargetColor)
{
    MINT32 err = 0;

    sem_init(&semSTEP1Done, 0, 0);
    sem_init(&semSTEP3Done, 0, 0);
    sem_init(&semSTEP4Done, 0, 0);
    sem_init(&semSTEP5Done, 0, 0);

    MY_LOGD("[InitialAlgorithm] In srcWidth %d srcHeight %d",srcWidth,srcHeight);
    MTKPipeFaceBeautyEnvInfo FaceBeautyEnvInfo;
    MTKPipeFaceBeautyTuningPara FaceBeautyTuningInfo;
    CPTLog(Event_FBShot_InitialAlgorithm, CPTFlagStart);
    //::memset(&FaceBeautyEnvInfo,0,sizeof(MTKPipeFaceBeautyEnvInfo));
    mStep1Width = (srcWidth >> 1) & 0xFFFFFFF0;
    mStep1Height = (srcHeight >> 1) & 0xFFFFFFF0;
    FaceBeautyEnvInfo.Step2SrcImgWidth = mDSWidth;
    FaceBeautyEnvInfo.Step2SrcImgHeight = mDSHeight;
    FaceBeautyEnvInfo.SrcImgWidth = srcWidth;
    FaceBeautyEnvInfo.SrcImgHeight = srcHeight;
    FaceBeautyEnvInfo.Step1SrcImgWidth = mStep1Width;
    FaceBeautyEnvInfo.Step1SrcImgHeight = mStep1Height;
    FaceBeautyEnvInfo.FDWidth = FBFDWidth;
    FaceBeautyEnvInfo.FDHeight = FBFDHeight;
    FaceBeautyEnvInfo.WorkingBufAddr = (MINTPTR) mpWorkingBuferr;
    FaceBeautyEnvInfo.WorkingBufSize = FBWorkingBufferSize;

    if(mSDKMode)
    {
        MY_LOGD("[InitialAlgorithm] SDK use YV12 format");
        FaceBeautyEnvInfo.SrcImgFormat = MTKPIPEFACEBEAUTY_IMAGE_YV12;
    } else {
        FaceBeautyEnvInfo.SrcImgFormat = MTKPIPEFACEBEAUTY_IMAGE_YUV422;
    }

    FaceBeautyEnvInfo.STEP1_ENABLE = true;

    FaceBeautyEnvInfo.pTuningPara = &FaceBeautyTuningInfo;
    FaceBeautyEnvInfo.pTuningPara->SmoothLevel = mSmoothLevel;
    FaceBeautyEnvInfo.pTuningPara->ContrastLevel = mContrastLevel;
    FaceBeautyEnvInfo.pTuningPara->BrightLevel = mBrightLevel;
    FaceBeautyEnvInfo.pTuningPara->RuddyLevel = mRuddyLevel;
    FaceBeautyEnvInfo.pTuningPara->EnlargeEyeLevel = mEnlargeEyeLevel;
    FaceBeautyEnvInfo.pTuningPara->SlimFaceLevel = mSlimFaceLevel;
    FaceBeautyEnvInfo.pTuningPara->WarpFaceNum = (mExtremeBeauty)? 1: 0;
    FaceBeautyEnvInfo.pTuningPara->MinFaceRatio = gMinFaceRatio;
    FaceBeautyEnvInfo.pTuningPara->AlignTH1 = -10000;
    FaceBeautyEnvInfo.pTuningPara->AlignTH2 = -5;
    MY_LOGD("[InitialAlgorithm] smooth-level=%d, skin-color=%d, enlarge-eye=%d, slim-face=%d, warp-face-num=%d", mSmoothLevel, mBrightLevel, mEnlargeEyeLevel, mSlimFaceLevel, FaceBeautyEnvInfo.pTuningPara->WarpFaceNum);

    err = mpFb->mHalFacebeautifyInit(&FaceBeautyEnvInfo);
    MY_LOGD("[InitialAlgorithm] algorithm initail done");
    CPTLog(Event_FBShot_InitialAlgorithm, CPTFlagEnd);
    MY_LOGD("[InitialAlgorithm] Out");
    if(err)
        return  MFALSE;
    else
    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/

MBOOL
Mhal_facebeauty::
ImgProcess(IImageBuffer const* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srctype, IImageBuffer const* Desbufinfo, MUINT32 desWidth, MUINT32 desHeight, EImageFormat destype, MUINT32 transform) const
{
    MY_LOGD("[Resize] srcAdr 0x%p srcWidth %d srcHeight %d desAdr 0x%p desWidth %d desHeight %d ",Srcbufinfo,Srcbufinfo->getImgSize().w,Srcbufinfo->getImgSize().h,Desbufinfo,Desbufinfo->getImgSize().w,Desbufinfo->getImgSize().h);

    ((IImageBuffer*)Srcbufinfo)->syncCache(eCACHECTRL_FLUSH);

    NSCam::NSIoPipe::NSSImager::ISImager *mpISImager = NSCam::NSIoPipe::NSSImager::ISImager::createInstance(Srcbufinfo);
    if (mpISImager == NULL)
    {
        MY_LOGE("Null ISImager Obj \n");
        return MFALSE;
    }

    //
    mpISImager->setTargetImgBuffer(Desbufinfo);
    //
    mpISImager->setTransform(transform);
    //
    //mpISImager->setFlip(0);
    //
    //mpISImager->setResize(desWidth, desHeight);
    //
    mpISImager->setEncodeParam(1, 90);
    //
    //mpISImager->setROI(Rect(0, 0, srcWidth, srcHeight));
    //
    mpISImager->execute();
    //Sava Test
    #ifdef Debug_Mode
    //if(count==0)
    {
       MY_LOGD("Save resize file");
       char szFileName[100];
       ::sprintf(szFileName, "/sdcard/imgprc_%d_%d_%d_%d_%d.yuv", (int)srctype, (int)destype,srcWidth,desWidth,count);
       Desbufinfo->saveToFile(szFileName);
       MY_LOGD("Save resize file done");
    }
    count++;
    #endif
    MY_LOGD("[Resize] Out");
    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
CancelAllSteps()
{
    mCancel = MTRUE;
    sem_post(&semSTEP1Done);
    sem_post(&semSTEP3Done);
    sem_post(&semSTEP4Done);
    sem_post(&semSTEP5Done);
    return MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
STEP2(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* DSbufinfo, MtkCameraFaceMetadata* FaceInfo, void* FaceBeautyResultInfo)
{
    CAM_TRACE_BEGIN("STEP2");
    MY_LOGD("[STEP2] srcAdr 0x%x DSbufinfo 0x%x size ds %d",(MUINT32)Srcbufinfo->getBufVA(0),(MUINT32)DSbufinfo->getBufVA(0),DSbufinfo->getBufSizeInBytes(0));
    CPTLog(Event_FBShot_STEP2, CPTFlagStart);
    MINT32 err = 0;
    if(mSDKMode)
    {
       MY_LOGD("[STEP2] SDK VGA use YV12 format");
       ImgProcess(Srcbufinfo, srcWidth, srcHeight, eImgFmt_YV12, mpDSImg, mpDSImg->getImgSize().w, mpDSImg->getImgSize().h, eImgFmt_I422);
    } else {
       ImgProcess(Srcbufinfo, srcWidth, srcHeight, eImgFmt_I422, mpDSImg, mpDSImg->getImgSize().w, mpDSImg->getImgSize().h, eImgFmt_I422);
    }
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "VGAImg", mpDSImg->getImgSize().w, mpDSImg->getImgSize().h);
    mpDSImg->saveToFile(szFileName);
    MY_LOGD("[STEP2] Save File done vga.adr 0x%x",mpDSImg->getBufVA(0));
    #endif

    /* Do face detection on captured image */
    if (!doFaceDetection(mpDSImg,
                            mpDSImg->getImgSize().w,
                            mpDSImg->getImgSize().h,
                            mSDKMode? mu4W_yuv: mShotParam.mi4PostviewWidth,
                            mSDKMode? mu4H_yuv: mShotParam.mi4PostviewHeight,
                            (mu4ShotMode == NSCam::eShotMode_FaceBeautyShotCc)))
    {
        MY_LOGD("[STEP2] FD fail");
        return  MFALSE;
    }

    MTKPipeFaceBeautyPos FacePos[15] = {0};
    int i=0;
    for (Vector<FACE_BEAUTY_POS>::const_iterator it = mBeautifiedFacePos.begin();
            it != mBeautifiedFacePos.end();
            it++)
    {
        if ((it->x < -1000) || (it->x > 1000) ||
            (it->y < -1000) || (it->y > 1000))
        {
            break;
        }

        FacePos[i].x = ((it->x + 1000) * (mDSWidth / 2)) / 2000;
        FacePos[i].y = ((it->y + 1000) * (mDSHeight / 2)) / 2000;
        MY_LOGD("[STEP2] Beautified face @[%d,%d]",FacePos[i].x,FacePos[i].y);
        i++;
    }

#ifndef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
    /* When vFB is disabled, compose beautified face positions from FD result */
    int faceNum = (mExtremeBeauty)? 1: FaceInfo->number_of_faces;
    for(int i=0;i<faceNum;i++)
    {
        FacePos[i].x = (FaceInfo->faces[i].rect[0] + FaceInfo->faces[i].rect[2]) / 2;
        FacePos[i].y = (FaceInfo->faces[i].rect[1] + FaceInfo->faces[i].rect[3]) / 2;
    }
#endif //MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT

    /* When vFB is disabled, compose beautified face positions from FD result */
    if(mSDKMode)
    {
        int sdkfaceNum = (mExtremeBeauty)? 1: FBmetadata.number_of_faces;
        for(int i=0;i<sdkfaceNum;i++)
        {
            FacePos[i].x = (FBmetadata.faces[i].rect[0] + FBmetadata.faces[i].rect[2]) / 2;
            FacePos[i].y = (FBmetadata.faces[i].rect[1] + FBmetadata.faces[i].rect[3]) / 2;
            MY_LOGD("SDKFB: face num %d x =%d y = %d",FBmetadata.number_of_faces,FacePos[i].x,FacePos[i].y);
        }
    }

    sem_wait(&semSTEP1Done);
    if(mCancel)
    {
         MY_LOGD("[STEP2] Canceled");
         goto lbExit;
    }

    CAM_TRACE_BEGIN("HALSTEP2");
    err = mpFb->mHalSTEP2((void*)mpDSImg->getBufVA(0),(void*)FaceInfo,FacePos,FaceBeautyResultInfo);
    CAM_TRACE_END();
    if(err)
    {
         MY_LOGD("[STEP2] Algo fail");
         goto lbExit;
    }
    CPTLog(Event_FBShot_STEP2, CPTFlagEnd);
    MY_LOGD("[STEP2] Out err %d",err);
    CAM_TRACE_END();
lbExit:
    if(err)
        return  MFALSE;
    else
        return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
STEP3(IImageBuffer* Srcbufinfo, void* FaceBeautyResultInfo) const
{
    CAM_TRACE_BEGIN("STEP3");
      MINT32 err = 0;
      #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "MapSrcImg", mDSWidth, mDSHeight);
    mpDSImg->saveToFile(szFileName);
    MY_LOGD("[STEP3] Save File done vga.adr 0x%x",mpDSImg->getBufVA(0));
    #endif
    MY_LOGD("[STEP3] srcAdr 0x%x size  %d",(MUINT32)mpDSImg->getBufVA(0),mpDSImg->getBufSizeInBytes(0));
    CPTLog(Event_FBShot_STEP3, CPTFlagStart);
    CAM_TRACE_BEGIN("HALSTEP3");
    err=mpFb->mHalSTEP3((void*)mpDSImg->getBufVA(0),FaceBeautyResultInfo);
    CAM_TRACE_END();
    CPTLog(Event_FBShot_STEP3, CPTFlagEnd);

    sem_post(&semSTEP3Done);

    //Sava test
    #ifdef Debug_Mode
    MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.raw", "TextureMap", mDSWidth, mDSHeight);
    saveBufToFile(szFileName, (MUINT8*)tmp->Step3ResultAddr_1, (mDSWidth * mDSHeight));
    MY_LOGD("[STEP3] Save File done desAdr 0x%x",tmp->Step3ResultAddr_1);
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.raw", "ColorMap", mDSWidth, mDSHeight);
    saveBufToFile(szFileName, (MUINT8*)tmp->Step3ResultAddr_2, (mDSWidth * mDSHeight));
    MY_LOGD("[STEP3] Save File done desAdr 0x%x",tmp->Step3ResultAddr_2);
    #endif

    CAM_TRACE_END();
    if(err)
    {
         MY_LOGE("[STEP3] mHalSTEP3 fail");
         return  MFALSE;
    }
    else
    {
        MY_LOGD("[STEP3] mHalSTEP3 out");
        return  MTRUE;
    }
}
/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
STEP1(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* Desbufinfo, IImageBuffer* Alphabufinfo, void* FaceBeautyResultInfo)
{
    CAM_TRACE_BEGIN("STEP1");
    MBOOL   ret = MFALSE;
    IImageBuffer* tmpSTEP1s = NULL;
    IImageBuffer* tmpSTEP1d = NULL;
    MY_LOGD("[STEP1] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x ",(MUINT32)Srcbufinfo->getBufVA(0),srcWidth,srcHeight,(MUINT32)Desbufinfo->getBufVA(0));
    CPTLog(Event_FBShot_STEP1, CPTFlagStart);
    tmpSTEP1s = allocMem(eImgFmt_I420, mStep1Width, mStep1Height);
    if(!tmpSTEP1s)
    {
        MY_LOGE("[STEP1] tmpSTEP1s alloc fail");
        ret = MFALSE;
        goto lbExit;
    }
    tmpSTEP1d = allocMem(eImgFmt_I420, mStep1Width, mStep1Height);
    if(!tmpSTEP1d)
    {
        MY_LOGE("[STEP1] tmpSTEP1s alloc fail");
        ret = MFALSE;
        goto lbExit;
    }
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    if(mSDKMode)
    {
        MY_LOGD("[STEP1] SDK Resize1 use YV12 format");
        ImgProcess(Srcbufinfo, srcWidth, srcHeight, eImgFmt_YV12, tmpSTEP1s, mStep1Width, mStep1Height, eImgFmt_I420);
        CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    } else {
        ImgProcess(Srcbufinfo, srcWidth, srcHeight, eImgFmt_I422, tmpSTEP1s, mStep1Width, mStep1Height, eImgFmt_I420);
        CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    }
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img0", mStep1Width, mStep1Height);
    tmpSTEP1s->saveToFile(szFileName);
    MY_LOGD("[STEP1] Save File done tmpSTEP1s->getBufVA(0) 0x%x",tmpSTEP1s->getBufVA(0));
    #endif

    MY_LOGI("[STEP1] resize 1 done ");
    CPTLog(Event_FBShot_STEP1Algo, CPTFlagStart);
    CAM_TRACE_BEGIN("STEP1");
    ret=mpFb->mHalSTEP1((void*)tmpSTEP1s->getBufVA(0),(void*)tmpSTEP1d->getBufVA(0),FaceBeautyResultInfo);
    CAM_TRACE_END();
    CPTLog(Event_FBShot_STEP1Algo, CPTFlagEnd);
    MY_LOGI("[STEP1] algorithm done ret %d",ret);
    if(ret)
    {
         MY_LOGD("[STEP1] STEP1 fail");
         goto lbExit;
    }

    #ifdef Debug_Mode
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img1", mStep1Width, mStep1Height);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step1ResultAddr, ((mStep1Width) * (mStep1Height) * 3 / 2));
    MY_LOGD("[STEP1] Save File done msFaceBeautyResultInfo.Step1ResultAddr 0x%x",msFaceBeautyResultInfo.Step1ResultAddr);
    #endif
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    //Do resize Alphamap
    #ifdef Debug_Mode
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img2", mStep1Width, mStep1Height);
    tmpSTEP1d->saveToFile(szFileName);
    MY_LOGD("[STEP1] Save File done tmpSTEP1->getBufVA(0) 0x%x",tmpSTEP1d->getBufVA(0));
    #endif
    //
    //memcpy((void*) tmpSTEP1d->getBufVA(0), (void*) msFaceBeautyResultInfo.Step1ResultAddr, ((mStep1Width) * (mStep1Height) * 3 / 2));
    sem_post(&semSTEP1Done);
    //
    if(mSDKMode)
    {
        MY_LOGD("[STEP1] SDK Resize2 use YV12 format");
        ImgProcess(tmpSTEP1d, (mStep1Width), (mStep1Height), eImgFmt_I420, Desbufinfo, srcWidth, srcHeight, eImgFmt_YV12);
        CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    } else {
        ImgProcess(tmpSTEP1d, (mStep1Width), (mStep1Height), eImgFmt_I420, Desbufinfo, srcWidth, srcHeight, eImgFmt_I422);
        CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    }
    MY_LOGI("[STEP1] resize 2 done ");

    //Sava Test
    #ifdef Debug_Mode
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img", srcWidth, srcHeight);
    Desbufinfo->saveToFile(szFileName);
    MY_LOGD("[STEP1] Save File done Srcbufinfo 0x%x",Srcbufinfo->getBufVA(0));
    #endif
    CPTLog(Event_FBShot_STEP1, CPTFlagEnd);
    CAM_TRACE_END();

 lbExit:
    deallocMem(tmpSTEP1s);
    deallocMem(tmpSTEP1d);
    if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

MBOOL
Mhal_facebeauty::
STEP4(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* Blurbufinfo, IImageBuffer* Alphabufinfo, void* FaceBeautyResultInfo)
{
    CAM_TRACE_BEGIN("STEP4");
      MINT32 ret = 0;
      MY_LOGD("[STEP4] in");
      MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    CPTLog(Event_FBShot_STEP4, CPTFlagStart);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    int const DSAMPX = mDSWidth;
    int const DSAMPY = mDSHeight;
    IImageBuffer* tmpStep4 = allocMem(eImgFmt_I422, DSAMPX, DSAMPY);
    if(!tmpStep4)
    {
        MY_LOGE("[STEP4] tmpStep4 alloc fail");
        goto lbExit;
    }
    //copy DS Alphamap to tmp buffer for HW excute
    //
    sem_wait(&semSTEP3Done);
    if(mCancel)
    {
         MY_LOGD("[STEP4] Canceled");
         goto lbExit;
    }

    memcpy((void*)tmpStep4->getBufVA(0),msFaceBeautyResultInfo.Step3ResultAddr_1,tmpStep4->getBufSizeInBytes(0));
    //Do resize Alphamap
    ImgProcess(tmpStep4, DSAMPX, DSAMPY, eImgFmt_I422, Alphabufinfo, srcWidth, srcHeight, eImgFmt_I422);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    MY_LOGD("[STEP4] resize done");
    CPTLog(Event_FBShot_STEP4Algo, CPTFlagStart);
    CAM_TRACE_BEGIN("HALSTEP4");
    ret=mpFb->mHalSTEP4((void*)Srcbufinfo->getBufVA(0),(void*)Blurbufinfo->getBufVA(0),(void*)Alphabufinfo->getBufVA(0),FaceBeautyResultInfo);
    CAM_TRACE_END();
    CPTLog(Event_FBShot_STEP4Algo, CPTFlagEnd);

    sem_post(&semSTEP4Done);

    MY_LOGD("[STEP4] algorithm done ret %d",ret);
    if(ret)
    {
         MY_LOGD("[STEP4] STEP4 fail");
         goto lbExit;
    }
    CPTLog(Event_FBShot_STEP4, CPTFlagEnd);
    //------------------ Sava test ----------------//
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "Step4Img", srcWidth, srcHeight);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step4ResultAddr, (srcWidth * srcHeight * 2));
    MY_LOGD("[STEP4] Save File done Step4ResultAddr 0x%x Result 0x%x",msFaceBeautyResultInfo.Step4ResultAddr,ret);
    #endif
    //------------------ Sava test ----------------//
lbExit:
    deallocMem(tmpStep4);
       MY_LOGD("[STEP4] out ret %d",ret);
    CAM_TRACE_END();
    if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

MBOOL
Mhal_facebeauty::
STEP5(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* Alphabufinfo, void* FaceBeautyResultInfo)
{
    CAM_TRACE_BEGIN("STEP5");
      MINT32 ret = 0;
      MY_LOGD("[STEP5] in");
      MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    CPTLog(Event_FBShot_STEP5, CPTFlagStart);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    int const DSAMPX = mDSWidth;
    int const DSAMPY = mDSHeight;
    IImageBuffer* tmpStep5 = allocMem(eImgFmt_I422, DSAMPX, DSAMPY);
    if(!tmpStep5)
    {
        MY_LOGE("[STEP5] tmpStep5 alloc fail");
        goto lbExit;
    }
    memcpy((void*)tmpStep5->getBufVA(0),msFaceBeautyResultInfo.Step3ResultAddr_2,tmpStep5->getBufSizeInBytes(0));
    ImgProcess(tmpStep5, DSAMPX, DSAMPY, eImgFmt_I422, Alphabufinfo, srcWidth, srcHeight, eImgFmt_I422);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);

    MY_LOGD("[STEP5] resize done");

    sem_wait(&semSTEP4Done);
    if(mCancel)
    {
         MY_LOGD("[STEP4] Canceled");
         goto lbExit;
    }

    CPTLog(Event_FBShot_STEP5Algo, CPTFlagStart);
    CAM_TRACE_BEGIN("HALSTEP5");
    ret=mpFb->mHalSTEP5((void*)Srcbufinfo->getBufVA(0),(void*)Alphabufinfo->getBufVA(0),FaceBeautyResultInfo);
    CAM_TRACE_END();
    CPTLog(Event_FBShot_STEP5Algo, CPTFlagEnd);

    sem_post(&semSTEP5Done);

    MY_LOGD("[STEP5] algorithm done ret %d",ret);
    if(ret)
    {
         MY_LOGD("[STEP5] STEP5 fail");
         goto lbExit;
    }
    CPTLog(Event_FBShot_STEP5, CPTFlagEnd);
    //------------------ Sava test ----------------//
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP5Img", srcWidth, srcHeight);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step5ResultAddr, (srcWidth * srcHeight * 2));
    MY_LOGD("[STEP5] Save File done Step5ResultAddr 0x%x Result 0x%x ",msFaceBeautyResultInfo.Step5ResultAddr,ret);
    #endif
    //------------------ Sava test ----------------//
lbExit:
    deallocMem(tmpStep5);
       MY_LOGD("[STEP5] out ret %d",ret);
    CAM_TRACE_END();
       if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

MBOOL
Mhal_facebeauty::
STEP6(IImageBuffer* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IImageBuffer* tmpbufinfo, void* FaceBeautyResultInfo) const
{
    CAM_TRACE_BEGIN("STEP6");
      MINT32 ret = 0;
      MY_LOGD("[STEP6] in");
      MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;

    sem_wait(&semSTEP5Done);
    if(mCancel)
    {
         MY_LOGD("[STEP5] Canceled");
         goto lbExit;
    }

    CPTLog(Event_FBShot_STEP6, CPTFlagStart);
    CAM_TRACE_BEGIN("HALSTEP6");
    ret=mpFb->mHalSTEP6((void*)Srcbufinfo->getBufVA(0),(void*)tmpbufinfo->getBufVA(0),FaceBeautyResultInfo);
    CAM_TRACE_END();
    MY_LOGD("[STEP6] algorithm done ret %d",ret);
    CPTLog(Event_FBShot_STEP6, CPTFlagEnd);
    //------------------ Sava test ----------------//
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "Step6Img", srcWidth, srcHeight);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step6ResultAddr, (srcWidth * srcHeight * 2));
    MY_LOGD("[STEP6] Save File done Step6ResultAddr 0x%x Result 0x%x ",msFaceBeautyResultInfo.Step6ResultAddr,ret);
    #endif
    //------------------ Sava test ----------------//
lbExit:
    CAM_TRACE_END();
       MY_LOGD("[STEP6] out ret %d",ret);
       if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

