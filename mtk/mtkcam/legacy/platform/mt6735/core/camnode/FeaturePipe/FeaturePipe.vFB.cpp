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
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
////#define LOG_TAG "MtkCam/PostProc.PipeWrapper"
/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   LOG_TAG    // Decide a Log TAG for current file.
#define  LOG_TAG    "MtkCam/FeaturePipeVFB"

#include <mtkcam/Log.h>

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)



#define FUNCTION_LOG_START      LOG_DBG("+");
#define FUNCTION_LOG_END        LOG_DBG("-");
#define ERROR_LOG               LOG_ERR("Error");
//

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <vector>
#include <queue>
#include <cutils/atomic.h>
#include <ui/GraphicBuffer.h>
#include "FeaturePipe.BufHandler.h"
#include "FeaturePipe.h"
#include "MTKWarp.h"
#include "MTKFeatureSet.h"
#include "DpBlitStream.h"
#include <mtkcam/utils/Format.h>
#include <mtkcam/common.h>
#include <mtkcam/featureio/IHal3A.h>
#include <mtkcam/common/faces.h>
#include "vfb_hal_base.h"
#include "eis_hal.h"    // For EisHal in FeaturePipeNode.h.
#include "vhdr_hal.h"
#include <fd_hal_base.h>
#include "FeaturePipeNode.h"
#include "FeaturePipe.vFB.h"
#include "camera/MtkCamera.h"
#include <cutils/properties.h>  // For property_get().

using namespace NSCam;
using namespace NSCamNode;
using namespace NSCam::Utils::Format;

#define GESTURE_ENABLE (1)
static unsigned char FaceNum =0;

#ifdef _DEBUG_DUMP
#include <fcntl.h>
#include <sys/stat.h>

static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_DBG("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_DBG("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        LOG_ERR("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_DBG("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            LOG_ERR("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    LOG_DBG("done writing %d bytes to file [%s]\n", size, fname);
    ::close(fd);
    return true;
}
#endif

static void postionFacesInImage(MUINT32 ImageWidth, MUINT32 ImageHeight, MINT32 numFace, MtkCameraFace* faces)
{
    for(int i = 0; i < numFace; i++)
    {
        faces[i].rect[0] = ((faces[i].rect[0] + 1000) * ImageWidth) / 2000;
        faces[i].rect[1] = ((faces[i].rect[1] + 1000) * ImageHeight) / 2000;
        faces[i].rect[2] = ((faces[i].rect[2] + 1000) * ImageWidth) / 2000;
        faces[i].rect[3] = ((faces[i].rect[3] + 1000) * ImageHeight) / 2000;

        int face_size = faces[i].rect[2] - faces[i].rect[0];
        if(face_size >= 30)
        {
            int zoom_size;
            zoom_size = face_size/15;

            if( (faces[i].rect[0] - zoom_size >= 0) &&
                    (faces[i].rect[1] - zoom_size >= 0) &&
                    (faces[i].rect[2] + zoom_size <= ImageWidth -1) &&
                    (faces[i].rect[3] + zoom_size <= ImageHeight -1))
            {
                zoom_size = face_size/12;
                if( (faces[i].rect[0] - zoom_size >= 0) &&
                        (faces[i].rect[1] - zoom_size >= 0) &&
                        (faces[i].rect[2] + zoom_size <= ImageWidth -1) &&
                        (faces[i].rect[3] + zoom_size <= ImageHeight -1))
                {
                    zoom_size = face_size/10;
                    if( (faces[i].rect[0] - zoom_size >= 0) &&
                            (faces[i].rect[1] - zoom_size >= 0) &&
                            (faces[i].rect[2] + zoom_size <= ImageWidth -1) &&
                            (faces[i].rect[3] + zoom_size <= ImageHeight -1))
                    {
                        zoom_size = face_size/8;
                        if( (faces[i].rect[0] - zoom_size >= 0) &&
                                (faces[i].rect[1] - zoom_size >= 0) &&
                                (faces[i].rect[2] + zoom_size <= ImageWidth -1) &&
                                (faces[i].rect[3] + zoom_size <= ImageHeight -1))
                        {
                            zoom_size = face_size/7;
                            if( (faces[i].rect[0] - zoom_size >= 0) &&
                                    (faces[i].rect[1] - zoom_size >= 0) &&
                                    (faces[i].rect[2] + zoom_size <= ImageWidth -1) &&
                                    (faces[i].rect[3] + zoom_size <= ImageHeight -1))
                            {
                                ;
                            }
                            else
                            {
                                zoom_size = face_size/8;
                            }
                        }
                        else
                        {
                            zoom_size = face_size/10;
                        }
                    }
                    else
                    {
                        zoom_size = face_size/12;
                    }
                }
                else
                {
                    zoom_size = face_size/15;
                }
                faces[i].rect[0] -= zoom_size;
                faces[i].rect[1] -= zoom_size;
                faces[i].rect[2] += zoom_size;
                faces[i].rect[3] += zoom_size;
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_DBG
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[FD:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[FD:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGI("[FD:%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[FD:%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[FD:%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[FD:%s] " fmt, __func__, ##arg); } while(0)
FeaturePipeFdNode*
FeaturePipeFdNode::createInstance(MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe* pFeaturePipeObj)
{
    LOG_AST(featureOnMask & FEATURE_ON_MASK_BIT_VFB, "Create FD node while vFB is off", 0);
    return new FeaturePipeFdNode("FeaturePipeFdNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY, featureOnMask, mOpenedSensorIndex, pFeaturePipeObj);
}
MVOID
FeaturePipeFdNode::destroyInstance()
{
    delete this;
}
FeaturePipeFdNode::FeaturePipeFdNode(
        const char* name,
        eThreadNodeType type,
        int policy,
        int priority,
        MUINT32 featureOnMask,
        MUINT32 mOpenedSensorIndex,
        FeaturePipe* pFeaturePipeObj)
        : FeaturePipeNode(name, type, policy, priority, featureOnMask, mOpenedSensorIndex, pFeaturePipeObj),
          mFDWidth(0),
          mFDHeight(0),
          mFDInited(MFALSE)
{
    addDataSupport(ENDPOINT_SRC, P2A_TO_FD_DSIMG);
    addDataSupport(ENDPOINT_DST, FD_TO_VFBSW);
    addNotifySupport(FEATURE_MASK_CHANGE);
}
MBOOL
FeaturePipeFdNode::onInit()
{
#define FD_WORKING_BUF_SIZE 5242880 //5M: 1024*1024*5

    FUNCTION_LOG_START;
    MRESULT mret;

    /* Create FD HAL instance */
    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_HW);
    if (mpFDHalObj == NULL)
    {
        LOG_ERR("halFDBase create instance failed");
        return 0;
    }

    mpFDWorkingBuffer = new unsigned char[FD_WORKING_BUF_SIZE];
    mFDVGABuffer = new unsigned char[640*480*2];
    mFDYBuffer = new unsigned char[VFB_DS_IMAGE_MAX_WIDTH*VFB_DS_IMAGE_MAX_HEIGHT];

    /* Create Gesture HAL instance */
    mpGDHalObj = halGSBase::createInstance(HAL_GS_OBJ_SW);
    if (mpGDHalObj == NULL)
    {
        LOG_ERR("halGDBase create instance failed");
        return 0;
    }
    mGSFaceMetadata.faces = (MtkCameraFace *) mGSFaceInfo;
    mGSFaceMetadata.posInfo = (MtkFaceInfo *) mGSPoseInfo;


    mp3AHal = NS3A::IHal3A::createInstance(NS3A::IHal3A::E_Camera_1, mOpenedSensorIndex, "FeaturePipe_Vfb");
    if (!mp3AHal)
    {
        LOG_ERR("IHal3A::createInstance failed!");
        goto _Exit;
    }

    /* Allocate buffer for next node */
    pBufHandler = getBufferHandler(FD_TO_VFBSW);
    if (pBufHandler == NULL)
    {
        LOG_ERR("ICamBufHandler failed!");
        goto _Exit;
    }
    {
        AllocInfo allocinfo(sizeof(VFB_NODE_FACE_METADATA),
                            1,
                            eImgFmt_BAYER8,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        for(MUINT32 i = 0; i < FD_NODE_FACE_INFO_NUM; i++)
        {
            if( !pBufHandler->requestBuffer(FD_TO_VFBSW, allocinfo) )
                goto _Exit;
        }
    }

    FUNCTION_LOG_END;
    return true;

_Exit:
    if (mpFDWorkingBuffer != NULL)
    {
        delete [] mpFDWorkingBuffer;
    }
    if (mFDVGABuffer != NULL)
    {
        delete [] mFDVGABuffer;
    }
    if (mFDYBuffer != NULL)
    {
        delete [] mFDYBuffer;
    }
    if (mp3AHal != NULL)
    {
        mp3AHal->destroyInstance("FeaturePipe_VfbP2B");
    }
    return false;
}

MBOOL
FeaturePipeFdNode::
onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;

    if (mpFDWorkingBuffer != NULL)
    {
        delete [] mpFDWorkingBuffer;
    }
    if (mFDVGABuffer != NULL)
    {
        delete [] mFDVGABuffer;
    }
    if (mFDYBuffer != NULL)
    {
        delete [] mFDYBuffer;
    }
    if (mp3AHal != NULL)
    {
        mp3AHal->destroyInstance("FeaturePipe_VfbP2B");
    }
    if (mpFDHalObj != NULL)
    {
        if (mFDInited)
        {
            mpFDHalObj->halFDUninit();
        }
        mpFDHalObj->destroyInstance();
    }

    if(mpGDHalObj != NULL)
    {
      LOG_DBG("GD_destroy(vFB)");
      //mpGDHalObj->halGSUninit();
      mpGDHalObj->destroyInstance();
      mpGDHalObj = NULL;
    }

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeFdNode::
onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeFdNode::
onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeFdNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNCTION_LOG_START;
    LOG_DBG("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    switch( datamsg )
    {
        case FEATURE_MASK_CHANGE:
            if (FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(mFeatureOnMask) != FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(ext1))
            {
                LOG_DBG("Gesture Shot is %s", (FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(ext1))? "enabled": "disabled");
            }
            mFeatureOnMask = ext1;
            break;
        default:
            break;
    }

    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeFdNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    Mutex::Autolock lock(mLock);

    FeaturePipeNodeImgReqJob_s job;
    job.imgReq = *(ImgRequest*) buf;
    job.mFrameNo = ext;
    switch (data)
    {
        case P2A_TO_FD_DSIMG:
            mqJobDsImg.push(job);
            triggerLoop();
            break;

        default:
            LOG_AST(0, "Wrong data (%d)!", data);
    }

    //FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeFdNode::
threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;
    MRESULT mret;

    //CPTLog(Event_FeaturePipe_FDThread, CPTFlagStart);

    /* Get DS image */
    FeaturePipeNodeImgReqJob_s jobDsImg;
    {
        Mutex::Autolock lock(mLock);

        jobDsImg = mqJobDsImg.front();
        mqJobDsImg.pop();
    }
    MUINT32 frameNo = jobDsImg.mFrameNo;

    CAM_TRACE_FMT_BEGIN("FDThread:%d", frameNo);

    /* Prepare buffer for next node */
    ImgRequest faceMetaDataReq;
    if(!pBufHandler->dequeBuffer(FD_TO_VFBSW, &faceMetaDataReq))
    {
        LOG_AST(false, "Out of FD_TO_VFBSW buffer", 0);
        return MFALSE;
    }
    VFB_NODE_FACE_METADATA* pFDResult = (VFB_NODE_FACE_METADATA*) const_cast<IImageBuffer*>(faceMetaDataReq.mBuffer)->getBufVA(0);
    pFDResult->mFaceMetadata.faces = (MtkCameraFace *) pFDResult->mFaceInfo;
    pFDResult->mFaceMetadata.posInfo = (MtkFaceInfo *) pFDResult->mPoseInfo;

    /* Face detection */
    CPTLog(Event_FeaturePipe_VFB_FD, CPTFlagStart);
    MINT32 FaceCount = doFaceDetection(const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer), &pFDResult->mFaceMetadata);
    CPTLog(Event_FeaturePipe_VFB_FD, CPTFlagEnd);
    /* Face AE/AF */
    if (FaceCount)
    {
        mp3AHal->setFDInfo(&pFDResult->mFaceMetadata);
    }

    /* Gesture Shot */
#if (GESTURE_ENABLE)
    if (FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(mFeatureOnMask) && (FaceCount))
    {
        MINT32 GestureDetected = doGestureShot(const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer));

        {
            /////////////////////////////////////////////////////////////////////
            // cpchen: filter GS results with FD results: no gesture inside face regions
            /////////////////////////////////////////////////////////////////////
            bool bEnableGSFilterWithFD = true;
            //bool bEnableGSFilterWithFD = false;
            float fIntersetAreaRatio = 0.25f;
            float fMaxRelativeRatio = 3.0f;
            if( bEnableGSFilterWithFD )
            {
               int newCount = 0;
               for (int gi = 0; gi < mGSFaceMetadata.number_of_faces; ++gi)
               {
                  // gesture rectangle
                  int gx0 = mGSFaceMetadata.faces[gi].rect[0];
                  int gy0 = mGSFaceMetadata.faces[gi].rect[1];
                  int gx1 = mGSFaceMetadata.faces[gi].rect[2];
                  int gy1 = mGSFaceMetadata.faces[gi].rect[3];
                  int garea = (gx1 - gx0) * (gy1 - gy0);

                  bool bIsOverlap = false;
                  for (int fi = 0; fi < mGSFaceMetadata.number_of_faces; ++fi)
                  {
                     // face rectangle
                     int fx0 = pFDResult->mFaceMetadata.faces[fi].rect[0];
                     int fy0 = pFDResult->mFaceMetadata.faces[fi].rect[1];
                     int fx1 = pFDResult->mFaceMetadata.faces[fi].rect[2];
                     int fy1 = pFDResult->mFaceMetadata.faces[fi].rect[3];
                     int farea = (fx1 - fx0) * (fy1 - fy0);

                     // interset rectangle
                     int ix0 = max(gx0, fx0);
                     int iy0 = max(gy0, fy0);
                     int ix1 = min(gx1, fx1);
                     int iy1 = min(gy1, fy1);
                     int iarea = 0;
                     if ((ix1 > ix0) && (iy1 > iy0))
                        iarea = (ix1 - ix0) * (iy1 - iy0);

                     // overlap determination
                     float minArea = min(garea, farea);
                     float overlapRatio = (float)iarea / minArea;
                     float relativeRatio = (float)farea / garea;
                     if (overlapRatio >= fIntersetAreaRatio)
                     {
                        //MY_LOGD("Gesture overlap with Face: (%d,%d,%d,%d) + (%d,%d,%d,%d) = (%d,%d,%d,%d)\n", gx0, gy0, gx1, gy1, fx0, fy0, fx1, fy1, ix0, iy0, ix1, iy1, iarea);
                        //MY_LOGD("Gesture overlap ratio = %.3f, area = %d, minArea = %.0f\n", overlapRatio, iarea, minArea);
                        bIsOverlap = true;
                        break;
                     }
                  } // end of for each face rectangle

                  // skip overlapped gesture rectangles, move non-overlapped gesture rectangles forward
                  if (!bIsOverlap)
                  {
                     mGSFaceMetadata.faces[newCount].rect[0] = mGSFaceMetadata.faces[gi].rect[0];
                     mGSFaceMetadata.faces[newCount].rect[1] = mGSFaceMetadata.faces[gi].rect[1];
                     mGSFaceMetadata.faces[newCount].rect[2] = mGSFaceMetadata.faces[gi].rect[2];
                     mGSFaceMetadata.faces[newCount].rect[3] = mGSFaceMetadata.faces[gi].rect[3];
                     mGSFaceMetadata.faces[newCount].score = mGSFaceMetadata.faces[gi].score;
                     mGSFaceMetadata.faces[newCount].id = mGSFaceMetadata.faces[gi].id;
                     mGSFaceMetadata.faces[newCount].left_eye[0] = mGSFaceMetadata.faces[gi].left_eye[0];
                     mGSFaceMetadata.faces[newCount].left_eye[1] = mGSFaceMetadata.faces[gi].left_eye[1];
                     mGSFaceMetadata.faces[newCount].right_eye[0] = mGSFaceMetadata.faces[gi].right_eye[0];
                     mGSFaceMetadata.faces[newCount].right_eye[1] = mGSFaceMetadata.faces[gi].right_eye[1];
                     mGSFaceMetadata.faces[newCount].mouth[0] = mGSFaceMetadata.faces[gi].mouth[0];
                     mGSFaceMetadata.faces[newCount].mouth[1] = mGSFaceMetadata.faces[gi].mouth[1];
                     mGSFaceMetadata.posInfo[newCount].rop_dir = mGSFaceMetadata.posInfo[gi].rop_dir;
                     mGSFaceMetadata.posInfo[newCount].rip_dir = mGSFaceMetadata.posInfo[gi].rip_dir;
                     ++newCount;
                  }
               }
               // number of gesture rectangles after filtering
               mGSFaceMetadata.number_of_faces = newCount;
               GestureDetected = newCount;

               // debug message
               if (GestureDetected == 0)
                  LOG_DBG("Scenario GD: Gesture detected but filtered out by face!!!");
            }
        }
        if (!GestureDetected)
        {
            char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("camera.featurepipe.fakegesture", EnableOption, "0");
            if (EnableOption[0] == '1')
            {
                GestureDetected = true;
                property_set("camera.featurepipe.fakegesture", "0");
            }
        }
        if (GestureDetected)
        {
            mpFeaturePipeObj->performCallback(CAMERA_MSG_PREVIEW_METADATA, MTK_CAMERA_MSG_EXT_NOTIFY_GESTURE_DETECT);
        }
    }
#endif

    /* Return buffer to previous node */
    LOG_DBG("Returning P2A node DS image buffer of frame %d", jobDsImg.mFrameNo);
    handleReturnBuffer(P2A_TO_FD_DSIMG, (MUINTPTR) &jobDsImg.imgReq, jobDsImg.mFrameNo);

    /* Post buffer to next node */
    LOG_DBG("Posting FD->P2B node of frame %d", frameNo);
    handlePostBuffer(FD_TO_VFBSW, (MUINTPTR) &faceMetaDataReq, frameNo);

    //CPTLog(Event_FeaturePipe_FDThread, CPTFlagEnd);
    CAM_TRACE_FMT_END();

    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeFdNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(0x%x), buf(0x%x), ext(0x%x)", data, buf, ext);
    pBufHandler->enqueBuffer(data, ((ImgRequest*)buf)->mBuffer);
    //FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeFdNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}


/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_DBG
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[VFB:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[VFB:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGI("[VFB:%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[VFB:%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[VFB:%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[VFB:%s] " fmt, __func__, ##arg); } while(0)
FeaturePipeVfbNode*
FeaturePipeVfbNode::createInstance(MUINT32 featureOnMask, FeaturePipe* pFeaturePipeObj)
{
    LOG_AST(featureOnMask & FEATURE_ON_MASK_BIT_VFB, "Create vFB node while vFB is off", 0);
    return new FeaturePipeVfbNode("FeaturePipeVfbNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY, featureOnMask, pFeaturePipeObj);
}
MVOID
FeaturePipeVfbNode::destroyInstance()
{
    delete this;
}
FeaturePipeVfbNode::FeaturePipeVfbNode(
        const char* name,
        eThreadNodeType type,
        int policy,
        int priority,
        MUINT32 featureOnMask,
        FeaturePipe* pFeaturePipeObj)
        : FeaturePipeNode(name, type, policy, priority, featureOnMask, 0, pFeaturePipeObj),
          mSmoothLevel(DEFAULT_SMOOTH_LEVEL),
          mSkinColor(DEFAULT_SKIN_COLOR),
          mEnlargeEyeLevel(DEFAULT_ENLARGE_EYE_LEVEL),
          mSlimFaceLevel(DEFAULT_SLIM_FACE_LEVEL),
          mExtremeBeauty(DEFAULT_EXTREME_BEAUTY),
          mTouchX(0),
          mTouchY(0),
          mRecordingHint(MFALSE)
{
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_SRC, EISSW_TO_VFBSW);
    }
    addDataSupport(ENDPOINT_SRC, SWITCHER_TO_VFBSW);
    addDataSupport(ENDPOINT_SRC, P2A_TO_VFBSW_DSIMG);
    addDataSupport(ENDPOINT_SRC, FD_TO_VFBSW);
    addDataSupport(ENDPOINT_DST, VFBSW_TO_GPU);
    addDataSupport(ENDPOINT_DST, VFBSW_TO_P2B_DSIMG);
    addDataSupport(ENDPOINT_DST, VFBSW_TO_P2B_ALPCL);
    addDataSupport(ENDPOINT_DST, VFBSW_TO_P2B_ALPNR);
    addDataSupport(ENDPOINT_DST, VFBSW_TO_P2B_PCA);
}
MBOOL
FeaturePipeVfbNode::onInit()
{
    FUNCTION_LOG_START;
    MRESULT mret;

    /* Create vFB HAL instance */
    mpVfbHal = halVFBBase::createInstance();
    if (mpVfbHal == NULL)
    {
        LOG_ERR("halVFBBase create instance failed!");
        goto _Exit;
    }

    /* Init vFB HAL */
    struct MTKPipeVfbEnvInfo vfbInfo;
    vfbInfo.Features = 0;
    ADD_FEATURE(vfbInfo.Features, MTK_VFB_BIT);
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        ADD_FEATURE(vfbInfo.Features, MTK_EIS2_BIT);
    }
    vfbInfo.ThreadPolicy = SCHED_POLICY;
    vfbInfo.ThreadPriority = SCHED_PRIORITY;
    vfbInfo.SrcImgWidth = FULL_IMAGE_MAX_WIDTH;
    vfbInfo.SrcImgHeight = FULL_IMAGE_MAX_HEIGHT;
    vfbInfo.DsImgWidth = VFB_DS_IMAGE_MAX_WIDTH;
    vfbInfo.DsImgHeight = VFB_DS_IMAGE_MAX_HEIGHT;
    vfbInfo.FDWidth = VFBFDWidth;
    vfbInfo.FDHeight = VFBFDHeight;
    vfbInfo.SrcImgFormat = eImgFmt_YUY2;
    vfbInfo.DsImgFormat = eImgFmt_YV16;
    vfbInfo.RecordingHint = mRecordingHint;
    LOG_DBG("mHalVFBInit %s full image %dx%d in %d format, down-scaled image %dx%d in %d format", (vfbInfo.RecordingHint)? "recording": "previewing", vfbInfo.SrcImgWidth, vfbInfo.SrcImgHeight, vfbInfo.SrcImgFormat, vfbInfo.DsImgWidth, vfbInfo.DsImgHeight, vfbInfo.DsImgFormat);
    if (mpVfbHal->mHalVFBInit(vfbInfo) == false)
    {
        LOG_ERR("mHalVFBInit init failed!");
        goto _Exit;
    }

    /* Allocate working memory */
    MINT32 size;
    size = mpVfbHal->mHalVFBGetWorkingBufSize();
    if (size == 0)
    {
        LOG_ERR("mHalVFBGetWorkingBufSize failed!");
        goto _Exit;
    }
    mpWorkingBuffer = (MUINT8*) malloc(size);
    if (mpWorkingBuffer == NULL)
    {
        LOG_ERR("vFB working buffer allocation failed!");
        goto _Exit;
    }
    if (mpVfbHal->mHalVFBSetWorkingBuf((void*) mpWorkingBuffer, size) == false)
    {
        LOG_ERR("mHalVFBSetWorkingBuf failed!");
        goto _Exit;
    }

    /* Allocate buffer for next node */
    pBufHandler = getBufferHandler(VFBSW_TO_P2B_DSIMG);
    if (pBufHandler == NULL)
    {
        LOG_ERR("ICamBufHandler failed!");
        goto _Exit;
    }
    {
        AllocInfo allocinfo(VFB_DS_IMAGE_MAX_WIDTH,
                            VFB_DS_IMAGE_MAX_HEIGHT,
                            eImgFmt_YUY2,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        for(MUINT32 i = 0; i < VFB_NODE_VFB_PARAMS_BUFFER_NUM; i++)
        {
            if( !pBufHandler->requestBuffer(VFBSW_TO_P2B_DSIMG, allocinfo) )
                goto _Exit;
        }
    }
    {
        AllocInfo allocinfo(VFB_DS_IMAGE_MAX_WIDTH,
                            VFB_DS_IMAGE_MAX_HEIGHT,
                            eImgFmt_BAYER8,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        for(MUINT32 i = 0; i < VFB_NODE_VFB_PARAMS_BUFFER_NUM; i++)
        {
            if( !pBufHandler->requestBuffer(VFBSW_TO_P2B_ALPCL, allocinfo) )
                goto _Exit;
            if( !pBufHandler->requestBuffer(VFBSW_TO_P2B_ALPNR, allocinfo) )
                goto _Exit;
        }
    }
    {
        AllocInfo allocinfo(160, //160x27=180x4x6
                            27,
                            eImgFmt_BAYER8,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        for(MUINT32 i = 0; i < VFB_NODE_VFB_PARAMS_BUFFER_NUM; i++)
        {
            if( !pBufHandler->requestBuffer(VFBSW_TO_P2B_PCA, allocinfo) )
                goto _Exit;
        }
    }
    {
        AllocInfo allocinfo(VFB_DS_IMAGE_MAX_WIDTH * 2 * 2, //Warp Map X + Y
                            VFB_DS_IMAGE_MAX_HEIGHT * 2,
                            eImgFmt_BAYER8,
                            eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN);
        for(MUINT32 i = 0; i < VFB_NODE_WARP_MAP_BUFFER_NUM; i++)
        {
            if( !pBufHandler->requestBuffer(VFBSW_TO_GPU, allocinfo) )
                goto _Exit;
        }
    }

    if (!FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        /* Allocate zero-ed Warp Map */
        mpClearWarpMapX = (MINT32*) malloc(VFB_DS_IMAGE_MAX_WIDTH * VFB_DS_IMAGE_MAX_HEIGHT * 4);
        mpClearWarpMapY = (MINT32*) malloc(VFB_DS_IMAGE_MAX_WIDTH * VFB_DS_IMAGE_MAX_HEIGHT * 4);
        memset(mpClearWarpMapX, 0, VFB_DS_IMAGE_MAX_WIDTH * VFB_DS_IMAGE_MAX_HEIGHT * 4);
        memset(mpClearWarpMapY, 0, VFB_DS_IMAGE_MAX_WIDTH * VFB_DS_IMAGE_MAX_HEIGHT * 4);
    }

    FUNCTION_LOG_END;
    return true;

_Exit:
    if (mpVfbHal != NULL)
    {
        mpVfbHal->destroyInstance();
    }
    if (mpWorkingBuffer != NULL)
    {
        free(mpWorkingBuffer);
    }
    return false;
}

MBOOL
FeaturePipeVfbNode::
onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;

    mpVfbHal->mHalVFBUninit();
    mpVfbHal->destroyInstance();
    if (mpWorkingBuffer != NULL)
    {
        free(mpWorkingBuffer);
    }
    if (!FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        free(mpClearWarpMapX);
        free(mpClearWarpMapY);
    }

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeVfbNode::
onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeVfbNode::
onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeVfbNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNCTION_LOG_START;
    LOG_DBG("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeVfbNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define MIN(a,b) (((a) < (b))? (a): (b))
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    Mutex::Autolock lock(mLock);

    MUINT32 numReadyJobs = MIN(mqJobFullImg.size(),mqJobFaceMetadata.size());
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
        numReadyJobs = MIN(numReadyJobs, mqJobWarpMap.size());

    FeaturePipeNodeImgReqJob_s job;
    FeaturePipeNodeTagImgReqJob_s tagJob;
    job.imgReq = *(ImgRequest*) buf;
    job.mFrameNo = ext;
    switch (data)
    {
        case SWITCHER_TO_VFBSW:
            tagJob.tagImgReq = *(TagImgRequest*) buf;
            tagJob.mFrameNo = ext;
            mqJobFullImg.push(tagJob);
            break;

        case P2A_TO_VFBSW_DSIMG:
            mqJobDsImg.push(job);
            break;

        case FD_TO_VFBSW:
            mqJobFaceMetadata.push(job);
            break;

        case EISSW_TO_VFBSW:
            mqJobWarpMap.push(job);
            break;

        default:
            LOG_AST(0, "Wrong data (%d)!", data);
    }

    MUINT32 newReadyJobs = MIN(mqJobFullImg.size(),mqJobFaceMetadata.size());
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
        newReadyJobs = MIN(newReadyJobs, mqJobWarpMap.size());
    if (newReadyJobs > numReadyJobs)
        triggerLoop();

    //FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeVfbNode::
threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;
    MRESULT mret;
    FeaturePipeNodeImgReqJob_s jobWarpMap;

    CPTLog(Event_FeaturePipe_VFBThread, CPTFlagStart);

    FeaturePipeNodeTagImgReqJob_s jobFullImg;
    FeaturePipeNodeImgReqJob_s jobDsImg;
    FeaturePipeNodeImgReqJob_s jobFaceMetadata;
    {
        /* All jobs have to be de-queued in one place!! */
        Mutex::Autolock lock(mLock);
        if(!(mqJobFullImg.size() && mqJobDsImg.size() && mqJobFaceMetadata.size()) ||
                (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) && !mqJobWarpMap.size())){
                LOG_ERR("One or more of job queues are empty\n");
                return false;
        }

        /* Get full image */
        jobFullImg = mqJobFullImg.front();
        mqJobFullImg.pop();

        /* Get DS image */
        jobDsImg = mqJobDsImg.front();
        mqJobDsImg.pop();

        /* Get face metadata */
        jobFaceMetadata = mqJobFaceMetadata.front();
        mqJobFaceMetadata.pop();

        if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
        {
            /* Get Warp Map */
            jobWarpMap = mqJobWarpMap.front();
            mqJobWarpMap.pop();
        }
    }
    MUINT32 frameNo = jobDsImg.mFrameNo;
    VFB_NODE_FACE_METADATA* pFDResult = (VFB_NODE_FACE_METADATA*) const_cast<IImageBuffer*>(jobFaceMetadata.imgReq.mBuffer)->getBufVA(0);

    CAM_TRACE_FMT_BEGIN("VFBThread:%d", frameNo);

    /* Set per frame process info */
    struct MTKPipeVfbProcInfo VfbProcInfo;
    struct MTKPipeVfbResultInfo VfbResultInfo;
    //Input
    VfbProcInfo.SmoothLevel = mSmoothLevel;
    VfbProcInfo.SkinColor = mSkinColor;
    VfbProcInfo.EnlargeEyeLevel = mEnlargeEyeLevel;
    VfbProcInfo.SlimFaceLevel = mSlimFaceLevel;
    VfbProcInfo.ExtremeBeauty = jobFullImg.tagImgReq.vfbExtreme;
    if ((mTouchX != 0) && (mTouchY != 0))
    {
        VfbProcInfo.TouchX = ((mTouchX + 1000) * jobDsImg.imgReq.mBuffer->getImgSize().w) / 2000;
        VfbProcInfo.TouchY = ((mTouchY + 1000) * jobDsImg.imgReq.mBuffer->getImgSize().h) / 2000;
        mTouchX = mTouchY = 0;
    }
    else
    {
        VfbProcInfo.TouchX = VfbProcInfo.TouchY = 0;
    }
    VfbProcInfo.DsImgAddr = (MUINT8*) jobDsImg.imgReq.mBuffer->getBufVA(0);
    /* Copy face position for callback */
    MINT32 FaceCount = pFDResult->mFaceMetadata.number_of_faces;
    MtkCameraFace cbFaceInfo[15];
    for (int i=0; i<15; i++)
    {
        cbFaceInfo[i] = pFDResult->mFaceInfo[i];
    }
    postionFacesInImage(jobDsImg.imgReq.mBuffer->getImgSize().w,
                        jobDsImg.imgReq.mBuffer->getImgSize().h,
                        FaceCount,
                        pFDResult->mFaceMetadata.faces);
    for (int i=0; i<FaceCount; i++)
    {
        LOG_DBG("Found face index %d left %d top %d right %d button %d pose %d (frame %d)",i+1,pFDResult->mFaceMetadata.faces[i].rect[0],pFDResult->mFaceMetadata.faces[i].rect[1],pFDResult->mFaceMetadata.faces[i].rect[2],pFDResult->mFaceMetadata.faces[i].rect[3],pFDResult->mFaceMetadata.posInfo[i].rip_dir, frameNo);
    }
    VfbProcInfo.FaceMetadata = &pFDResult->mFaceMetadata;
    //Output
    /* Prepare buffer for next node */
    MUINT32 DSClipWidth = jobDsImg.imgReq.mBuffer->getImgSize().w;
    MUINT32 DSClipHeight = jobDsImg.imgReq.mBuffer->getImgSize().h;
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        DSClipWidth /= (EIS_FACTOR/100.0);
        DSClipHeight /= (EIS_FACTOR/100.0);
    }
    ImgRequest dsImgReq;
    ImgRequest alphaCLImgReq;
    ImgRequest alphaNRImgReq;
    ImgRequest pcaImgReq;
    ImgRequest warpImgReq;
    if(!pBufHandler->dequeBuffer(VFBSW_TO_P2B_DSIMG, &dsImgReq))
    {
        LOG_AST(false, "Out of VFBSW_TO_P2B_DSIMG buffer", 0);
        return MFALSE;
    }
    if ((dsImgReq.mBuffer->getImgSize().w != DSClipWidth) ||
            (dsImgReq.mBuffer->getImgSize().h != DSClipHeight))
    {
        if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(VFBSW_TO_P2B_DSIMG, &dsImgReq.mBuffer, DSClipWidth, DSClipHeight) )
        {
            LOG_ERR("Resize DS image buffer failed");
            return MFALSE;
        }
    }
    VfbProcInfo.WarpedDsImage = (MUINT8*) const_cast<IImageBuffer*>(dsImgReq.mBuffer)->getBufVA(0);
    // Alpha CL
    if(!pBufHandler->dequeBuffer(VFBSW_TO_P2B_ALPCL, &alphaCLImgReq))
    {
        LOG_AST(false, "Out of VFBSW_TO_P2B_ALPCL buffer", 0);
        return MFALSE;
    }
    if ((alphaCLImgReq.mBuffer->getImgSize().w != DSClipWidth) ||
            (alphaCLImgReq.mBuffer->getImgSize().h != DSClipHeight))
    {
        if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(VFBSW_TO_P2B_ALPCL, &alphaCLImgReq.mBuffer, DSClipWidth, DSClipHeight) )
        {
            LOG_ERR("Resize AlphaCL buffer failed");
            return MFALSE;
        }
    }
    VfbProcInfo.AlphaCL = (MUINT8*) const_cast<IImageBuffer*>(alphaCLImgReq.mBuffer)->getBufVA(0);
    // Alpha NR
    if(!pBufHandler->dequeBuffer(VFBSW_TO_P2B_ALPNR, &alphaNRImgReq))
    {
        LOG_AST(false, "Out of VFBSW_TO_P2B_ALPNR buffer", 0);
        return MFALSE;
    }
    if ((alphaNRImgReq.mBuffer->getImgSize().w != DSClipWidth) ||
            (alphaNRImgReq.mBuffer->getImgSize().h != DSClipHeight))
    {
        if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(VFBSW_TO_P2B_ALPNR, &alphaNRImgReq.mBuffer, DSClipWidth, DSClipHeight) )
        {
            LOG_ERR("Resize AlphaNR buffer failed");
            return MFALSE;
        }
    }
    VfbProcInfo.AlphaNR = (MUINT8*) const_cast<IImageBuffer*>(alphaNRImgReq.mBuffer)->getBufVA(0);
    // PCA
    if(!pBufHandler->dequeBuffer(VFBSW_TO_P2B_PCA, &pcaImgReq))
    {
        LOG_AST(false, "Out of VFBSW_TO_P2B_PCA buffer", 0);
        return MFALSE;
    }
    VfbProcInfo.PCATable = (MINT32*) const_cast<IImageBuffer*>(pcaImgReq.mBuffer)->getBufVA(0);
    // Warp Map
    if(!pBufHandler->dequeBuffer(VFBSW_TO_GPU, &warpImgReq))
    {
        LOG_AST(false, "Out of VFBSW_TO_GPU buffer", 0);
        return MFALSE;
    }
    if ((warpImgReq.mBuffer->getImgSize().w != jobDsImg.imgReq.mBuffer->getImgSize().w * 2 * 2) ||
            (warpImgReq.mBuffer->getImgSize().h != jobDsImg.imgReq.mBuffer->getImgSize().h * 2))
    {
        if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(VFBSW_TO_GPU, &warpImgReq.mBuffer, jobDsImg.imgReq.mBuffer->getImgSize().w * 2 * 2, jobDsImg.imgReq.mBuffer->getImgSize().h * 2) )
        {
            LOG_ERR("Resize Warp Map failed");
            return MFALSE;
        }
    }
    VfbProcInfo.WarpMapX = (MINT32*) const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0);
    VfbProcInfo.WarpMapY = (MINT32*) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0) + const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufSizeInBytes(0) / 2);

    /* VFB Processing */
    VfbProcInfo.ClipWidth = jobFullImg.tagImgReq.imgReq.mBuffer->getImgSize().w;
    VfbProcInfo.ClipHeight = jobFullImg.tagImgReq.imgReq.mBuffer->getImgSize().h;
    VfbProcInfo.DsClipWidth = DSClipWidth;
    VfbProcInfo.DsClipHeight = DSClipHeight;
    VfbProcInfo.SrcImgWidth = jobFullImg.tagImgReq.imgReq.mBuffer->getImgSize().w;
    VfbProcInfo.SrcImgHeight = jobFullImg.tagImgReq.imgReq.mBuffer->getImgSize().h;
    VfbProcInfo.DsImgWidth = jobDsImg.imgReq.mBuffer->getImgSize().w;
    VfbProcInfo.DsImgHeight = jobDsImg.imgReq.mBuffer->getImgSize().h;
    VfbProcInfo.FDWidth = VfbProcInfo.DsImgWidth;
    VfbProcInfo.FDHeight = VfbProcInfo.DsImgHeight;
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        VfbProcInfo.PreWarpMapX = (MINT32*) const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getBufVA(0);
        VfbProcInfo.PreWarpMapY = (MINT32*) ((MINTPTR) VfbProcInfo.PreWarpMapX + (const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getBufSizeInBytes(0) / 2));
        VfbProcInfo.PreWarpMapSize[0] = VfbProcInfo.PreWarpMapSize[1] = const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getBufSizeInBytes(0) / 2;
    }
    else
    {
        VfbProcInfo.PreWarpMapX = mpClearWarpMapX;
        VfbProcInfo.PreWarpMapY = mpClearWarpMapY;
        VfbProcInfo.PreWarpMapSize[0] = VfbProcInfo.PreWarpMapSize[1] = VFB_DS_IMAGE_MAX_WIDTH * VFB_DS_IMAGE_MAX_HEIGHT * 4;
    }
    LOG_DBG("mHalVFBProc Processing QVGA 0x%x, Warped QVGA 0x%x, AlphaCL 0x%x, AlphaNR 0x%x, PCA Table 0x%x, Warp Map 0x%x, 0x%x", VfbProcInfo.DsImgAddr, VfbProcInfo.WarpedDsImage, VfbProcInfo.AlphaCL, VfbProcInfo.AlphaNR, VfbProcInfo.PCATable, VfbProcInfo.WarpMapX, VfbProcInfo.WarpMapY);
    CPTLog(Event_FeaturePipe_VFB_Proc, CPTFlagStart);
    if (mpVfbHal->mHalVFBProc(VfbProcInfo, &VfbResultInfo) != true)
    {
        LOG_ERR("mHalVFBProc failed!");
        /* TODO: what now? */
    }
    CPTLog(Event_FeaturePipe_VFB_Proc, CPTFlagEnd);

    /* Flush cache */
    const_cast<IImageBuffer*>(dsImgReq.mBuffer)->syncCache(eCACHECTRL_FLUSH);
    const_cast<IImageBuffer*>(alphaCLImgReq.mBuffer)->syncCache(eCACHECTRL_FLUSH);
    const_cast<IImageBuffer*>(alphaNRImgReq.mBuffer)->syncCache(eCACHECTRL_FLUSH);
    const_cast<IImageBuffer*>(pcaImgReq.mBuffer)->syncCache(eCACHECTRL_FLUSH);
    const_cast<IImageBuffer*>(warpImgReq.mBuffer)->syncCache(eCACHECTRL_FLUSH);

    /* Callback FD & FB result */
    for(int j = 0; j < FaceCount; j++)
        cbFaceInfo[j].score = 0;
    for(int i = 0; i < 15; i++)
    {
        if ((VfbResultInfo.FacePos[i][0] == 0) &&
                (VfbResultInfo.FacePos[i][1] == 0))
        {
            break;
        }

        for(int j = 0; j < FaceCount; j++)
        {
            if ((VfbResultInfo.FacePos[i][0] >= pFDResult->mFaceMetadata.faces[j].rect[0]) &&
                    (VfbResultInfo.FacePos[i][1] >= pFDResult->mFaceMetadata.faces[j].rect[1]) &&
                    (VfbResultInfo.FacePos[i][0] <= pFDResult->mFaceMetadata.faces[j].rect[2]) &&
                    (VfbResultInfo.FacePos[i][1] <= pFDResult->mFaceMetadata.faces[j].rect[3]))
            {
                LOG_DBG("Beautified face index %d @[%d,%d] (frame %d)", j+1, (cbFaceInfo[j].rect[0] + cbFaceInfo[j].rect[2]) / 2, (cbFaceInfo[j].rect[1] + cbFaceInfo[j].rect[3]) / 2, frameNo);
                cbFaceInfo[j].score = 100;
            }
        }
    }

    camera_frame_metadata_t retFaces;
    retFaces.number_of_faces = FaceCount;
    retFaces.faces = reinterpret_cast<camera_face_t*>(cbFaceInfo);
    mpFeaturePipeObj->performCallback(CAMERA_MSG_PREVIEW_METADATA, &retFaces);

#ifndef _DEBUG_DUMP
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.featurepipe.dumpvfb", EnableOption, "0");
    if (EnableOption[0] == '1')
#endif
    {
        char szFileName[100];
        ::sprintf(szFileName, "/sdcard/vfbi_%dx%d_%04d.yuv", jobDsImg.imgReq.mBuffer->getImgSize().w, jobDsImg.imgReq.mBuffer->getImgSize().h, frameNo);
        const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer)->saveToFile(szFileName);
        ::sprintf(szFileName, "/sdcard/vfbo_%dx%d_%04d.yuv", dsImgReq.mBuffer->getImgSize().w, dsImgReq.mBuffer->getImgSize().h, frameNo);
        const_cast<IImageBuffer*>(dsImgReq.mBuffer)->saveToFile(szFileName);
        ::sprintf(szFileName, "/sdcard/vfbo_alphacl_%04d.bin", frameNo);
        const_cast<IImageBuffer*>(alphaCLImgReq.mBuffer)->saveToFile(szFileName);
        ::sprintf(szFileName, "/sdcard/vfbo_alphanr_%04d.bin", frameNo);
        const_cast<IImageBuffer*>(alphaNRImgReq.mBuffer)->saveToFile(szFileName);
        ::sprintf(szFileName, "/sdcard/vfbo_pcatbl_%04d.bin", frameNo);
        const_cast<IImageBuffer*>(pcaImgReq.mBuffer)->saveToFile(szFileName);
        ::sprintf(szFileName, "/sdcard/vfbo_warpmap_%04d.bin", frameNo);
        const_cast<IImageBuffer*>(warpImgReq.mBuffer)->saveToFile(szFileName);
    }

    /* Return buffer to previous node */
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        LOG_DBG("Returning EIS node warp map buffer %d", jobWarpMap.mFrameNo);
        handleReturnBuffer(EISSW_TO_VFBSW, (MUINTPTR) &jobWarpMap.imgReq, jobWarpMap.mFrameNo);
    }
    LOG_DBG("Returning P2A node DS image buffer of frame %d", jobDsImg.mFrameNo);
    handleReturnBuffer(P2A_TO_VFBSW_DSIMG, (MUINTPTR) &jobDsImg.imgReq, jobDsImg.mFrameNo);
    LOG_DBG("Returning FD node face metadata buffer of frame %d", jobDsImg.mFrameNo);
    handleReturnBuffer(FD_TO_VFBSW, (MUINTPTR) &jobFaceMetadata.imgReq, jobDsImg.mFrameNo);

#if 0//def _UNITTEST
    for (FeaturePipeBufList::iterator it = mNodeWarpMapBuf[mNodeWarpMapBufIdx].bufList.begin(); it != mNodeWarpMapBuf[mNodeWarpMapBufIdx].bufList.end(); it++)
    {
        if (it->etype == FEATURE_STREAM_BUF_GPU_WARP_MAP)
        {
            loadFileToBuf("/data/testvfb_warpmapx.bin", (MUINT8*) it->u.warpMap.warpMapX, 320*240*4);
            loadFileToBuf("/data/testvfb_warpmapy.bin", (MUINT8*) it->u.warpMap.warpMapY, 320*240*4);
            break;
        }
    }
#endif

    /* Post buffer to next node */
    LOG_DBG("Posting VFB->P2B node of frame %d", frameNo);
    LOG_DBG("Posting VFB->P2B ds req(%x) mbuffer(%x)", &dsImgReq, dsImgReq.mBuffer);
    handlePostBuffer(VFBSW_TO_P2B_ALPCL, (MUINTPTR) &alphaCLImgReq, frameNo);
    handlePostBuffer(VFBSW_TO_P2B_ALPNR, (MUINTPTR) &alphaNRImgReq, frameNo);
    handlePostBuffer(VFBSW_TO_P2B_PCA, (MUINTPTR) &pcaImgReq, frameNo);
    handlePostBuffer(VFBSW_TO_P2B_DSIMG, (MUINTPTR) &dsImgReq, frameNo);
    if (jobFullImg.tagImgReq.vfbExtreme ||
        FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        LOG_DBG("Posting VFB->GPU node of frame %d", frameNo);
        handlePostBuffer(VFBSW_TO_GPU, (MUINTPTR) &warpImgReq, frameNo);
    }
    else
    {
        pBufHandler->enqueBuffer(VFBSW_TO_GPU, warpImgReq.mBuffer);
    }

    CPTLog(Event_FeaturePipe_VFBThread, CPTFlagEnd);
    CAM_TRACE_FMT_END();

    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeVfbNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(0x%x), buf(0x%x), ext(0x%x)", data, buf, ext);
    LOG_DBG("Recycling VFB node %s image of frame %d",
                    (data == VFBSW_TO_P2B_DSIMG)? "DS":
                    ((data == VFBSW_TO_P2B_ALPCL)? "AlphaCL":
                    ((data == VFBSW_TO_P2B_ALPNR)? "AlphaNR":
                    ((data == VFBSW_TO_P2B_PCA)? "PCA":
                    ((data == VFBSW_TO_GPU)? "warp map": "!!!")))),
                    ext);
    pBufHandler->enqueBuffer(data, ((ImgRequest*)buf)->mBuffer);
    //FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeVfbNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}

MBOOL
FeaturePipeVfbNode::
getDSImgSize(MSize& srcImgSize, MSize& dsImgSize)
{
    if((srcImgSize.w*3) == (srcImgSize.h*4))
    {
        dsImgSize.w = 320;
        dsImgSize.h = 240;
    }
    else if((srcImgSize.w*9) == (srcImgSize.h*16))
    {
        dsImgSize.w = 320;
        dsImgSize.h = 180;
    }
    else if((srcImgSize.w*3) == (srcImgSize.h*5))
    {
        dsImgSize.w = 320;
        dsImgSize.h = 192;
    }
    else
    {
        dsImgSize.w = 320;
        if(srcImgSize.w != 0)
          dsImgSize.h = 320 * srcImgSize.h/srcImgSize.w;
        else
          dsImgSize.h = 180;
    }
    return MTRUE;
}

MBOOL
FeaturePipeVfbNode::setTuning(FeaturePipeTuningInfo& tuningInfo)
{
    mSmoothLevel = tuningInfo.SmoothLevel;
    mSkinColor = tuningInfo.SkinColor;
    mEnlargeEyeLevel = tuningInfo.EnlargeEyeLevel;
    mSlimFaceLevel = tuningInfo.SlimFaceLevel;
    mExtremeBeauty = tuningInfo.ExtremeBeauty;
    return MTRUE;
}

MBOOL
FeaturePipeVfbNode::setTouch(MINT32 touchX, MINT32 touchY)
{
    if ((touchX >= -1000) && (touchX <= 1000) && (touchY >= -1000) && (touchY <= 1000))
    {
        mTouchX = touchX;
        mTouchY = touchY;
    }
    return MTRUE;
}

MBOOL
FeaturePipeVfbNode::set(FeaturePipeSetType setType, const MINTPTR setData)
{
    switch (setType)
    {
        case RECORDING_HINT:
            if (mRecordingHint != (MBOOL) setData)
            {
                mRecordingHint = (MBOOL) setData;
                LOG_DBG("Set recording hint to %d", mRecordingHint);
            }
            break;
    }
    return MTRUE;
}

MVOID
FeaturePipeVfbNode::setPCA(IImageBuffer* pImageBuffer)
{
    LOG_DBG("Set vFB PCA@0x%x", pImageBuffer->getBufVA(0));
    halVFBTuning::getInstance().mHalVFBTuningSetPCA((MINT32*)pImageBuffer->getBufVA(0));
}
MVOID
FeaturePipeVfbNode::setLCE(IImageBuffer* pImageBuffer)
{
    LOG_DBG("Set vFB LCE@0x%x", pImageBuffer->getBufPA(0));
    halVFBTuning::getInstance().mHalVFBTuningSetLCE((MINT32*)pImageBuffer->getBufPA(0));
}

MINT32
FeaturePipeFdNode::doFaceDetection(IImageBuffer* pImageBuffer, MtkCameraFaceMetadata *faceMetadata)
{
//#define _WORKAROUND_FD_NOT_SUPPORT_YUY2

    MUINT32 imgw = pImageBuffer->getImgSize().w;
    MUINT32 imgh = pImageBuffer->getImgSize().h;

    if (mFDInited &&
        (mFDWidth != 0) &&
        ((mFDWidth != imgw) ||
         (mFDHeight != imgh)))
    {
        mpFDHalObj->halFDUninit();
    }
    if ((mFDWidth == 0) ||
        (mFDWidth != imgw) ||
        (mFDHeight != imgh))
    {
        mpFDHalObj->halFDInit(imgw,
                              imgh,
                              mpFDWorkingBuffer,
                              FD_WORKING_BUF_SIZE,
                              1,
                              2); //2:enhanced for FB
        mFDInited = MTRUE;
    }
    mFDWidth = imgw;
    mFDHeight = imgh;

    MINT32 numFace;
    if(FaceNum>0)
        mpFDHalObj->halFTBufferCreate(mFDVGABuffer, (MUINT8*) pImageBuffer->getBufVA(0), 1, mFDWidth, mFDHeight);

#ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
    MUINT8* dstbufadr = (MUINT8*) malloc(imgw * imgh * 3 / 2);
    for (int i=0;i<imgw*imgh*2/4;i++)
    {
        dstbufadr[2*i]=((MUINT8*)pImageBuffer->getBufVA(0))[4*i];
        dstbufadr[2*i+1]=((MUINT8*)pImageBuffer->getBufVA(0))[4*i+2];
    }
    //saveBufToFile("/sdcard/fdi_YV12.yuv", dstbufadr, imgw*imgh * 3/2);
#else
    mpFDHalObj->halFDYUYV2ExtractY(mFDYBuffer, (MUINT8*) pImageBuffer->getBufVA(0), mFDWidth, mFDHeight);
#endif

    //pImageBuffer->saveToFile("/sdcard/fdi_YUY2.yuv");
    MUINT8 count = 0;
    do {
        //LOG_DBG("Face detection try %d", count + 1);
        mpFDHalObj->halFDDo(0,
                             mFDVGABuffer,
#ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
                             dstbufadr,
#else
                             mFDYBuffer,
#endif
                            false,
                            mRotation,
#ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
                            3);
#else
                            (MUINT8*)pImageBuffer->getBufPA(0));
#endif
        numFace = mpFDHalObj->halFDGetFaceResult(faceMetadata);
        FaceNum = numFace;
    } while ((numFace == 0) && (++count < 1));
    LOG_DBG("Face detection tried %d times and found %d faces", count, numFace);

#ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
    free(dstbufadr);
#endif

    return numFace;
}

MBOOL
FeaturePipeFdNode::setRotation(MINT32 rotation)
{
    LOG_DBG("Face detection: rotation %d", rotation);
    mRotation = rotation;
    return MTRUE;
}

MINT32
FeaturePipeFdNode::doGestureShot(IImageBuffer* pImageBuffer)
{
    MUINT32 SrcW = pImageBuffer->getImgSize().w;
    MUINT32 SrcH = pImageBuffer->getImgSize().h;
    MUINT32 GD_Result = 0;

    //Initial GD Start
    LOG_DBG("StartGD_Init(vFB): SrcW:%d, SrcH:%d", SrcW, SrcH);
    mpGDHalObj->halGSInit(SrcW, SrcH, (MUINT8 *) mpFDWorkingBuffer, FD_WORKING_BUF_SIZE);

    mpGDHalObj->halGSDo((MUINT8 *)mFDYBuffer, mRotation);

    GD_Result = mpGDHalObj->halGSGetGestureResult(&mGSFaceMetadata);
    LOG_DBG("Gesture Shot result %d", GD_Result);

    mpGDHalObj->halGSUninit();
    LOG_DBG("GD_Unit(vFB) finish!");

    return GD_Result;
}
#endif //MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
