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

#define LOG_TAG "MtkCam/CamClient/FDClient"
//
#include "FDClient.h"
//

using namespace NSCamClient;
using namespace NSFDClient;
//

/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)
#define GET_FD_RESULT (1)
#define GET_SD_RESULT (1)
#define GET_GD_RESULT (1)

static unsigned char *src_buf;
static unsigned char *dst_rgb;
static unsigned char *dst_y;
static unsigned char numFace =0;
static unsigned char plane =1;

unsigned char g_BufferGroup = 0;

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
doFD(ImgBufQueNode const& rQueNode, bool &rIsDetected_FD, bool &rIsDetected_SD, bool rDoSD, bool &rIsDetected_GD, bool rDoGD)
{
    bool ret = true;
    MINT32 SD_Result, GD_Result;

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //MY_LOGD("Rotation_Info1:%d", Rotation_Info);

//********************************************************************************************//
    int srcWidth=0,  srcHeight=0;
    mpParamsMgr->getPreviewSize(&srcWidth, &srcHeight);

//********************************************************************************************//

    dst_rgb = (unsigned char *)DDPBuffer;
    src_buf = (unsigned char *)rQueNode.getImgBuf()->getVirAddr();
    dst_y = (unsigned char *)ExtractYBuffer;

    g_BufferGroup = doBufferAnalysis(rQueNode);
    //g_BufferGroup = 1;

    //mpFDHalObj->halFDBufferCreate(dst_rgb, src_buf, g_BufferGroup);
    //if(numFace>0) //ASD buffer
        mpFDHalObj->halFTBufferCreate(dst_rgb, src_buf, plane, srcWidth, srcHeight);

    if(plane == 1)
    {
        mpFDHalObj->halFDYUYV2ExtractY(dst_y, src_buf, srcWidth, srcHeight);
        mpFDHalObj->halFDDo((unsigned char *)0, dst_rgb,  dst_y, rDoSD, Rotation_Info, (unsigned char *)rQueNode.getImgBuf()->getPhyAddr());
    }
    else
        mpFDHalObj->halFDDo((unsigned char *)0, dst_rgb,  (unsigned char *)rQueNode.getImgBuf()->getVirAddr(), rDoSD, Rotation_Info, (unsigned char *)rQueNode.getImgBuf()->getPhyAddr());

    if( NULL != mpDetectedFaces )
        numFace = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);
    else
        MY_LOGW("Get FD Result Fail!");

    if(rDoGD == 1)
    {
        if(plane == 1)
            mpGDHalObj->halGSDo(dst_y, Rotation_Info);
        else
            mpGDHalObj->halGSDo((MUINT8*)rQueNode.getImgBuf()->getVirAddr(), Rotation_Info);
    }

//********************************************************************************************//

#if (GET_FD_RESULT)

    MY_LOGD("Scenario FD Num: %d",numFace );
    if(numFace>0)
        rIsDetected_FD = 1;
   else
        rIsDetected_FD = 0;
#endif

#if (GET_SD_RESULT)
    SD_Result = mpFDHalObj->halSDGetSmileResult();
    MY_LOGD("Scenario SD Result1: %d",SD_Result );
    if(SD_Result>0)
        rIsDetected_SD = 1;
   else
        rIsDetected_SD = 0;
#endif

#if (GET_GD_RESULT)
if(rDoGD == 1)
{
    if( NULL != mpDetectedGestures )
        GD_Result = mpGDHalObj->halGSGetGestureResult(mpDetectedGestures);

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
       for (int gi = 0; gi < mpDetectedGestures->number_of_faces; ++gi)
       {
          // gesture rectangle
          int gx0 = mpDetectedGestures->faces[gi].rect[0];
          int gy0 = mpDetectedGestures->faces[gi].rect[1];
          int gx1 = mpDetectedGestures->faces[gi].rect[2];
          int gy1 = mpDetectedGestures->faces[gi].rect[3];
          int garea = (gx1 - gx0) * (gy1 - gy0);

          bool bIsOverlap = false;
          for (int fi = 0; fi < mpDetectedFaces->number_of_faces; ++fi)
          {
             // face rectangle
             int fx0 = mpDetectedFaces->faces[fi].rect[0];
             int fy0 = mpDetectedFaces->faces[fi].rect[1];
             int fx1 = mpDetectedFaces->faces[fi].rect[2];
             int fy1 = mpDetectedFaces->faces[fi].rect[3];
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
//             if ( overlapRatio >= fIntersetAreaRatio &&
//                   relativeRatio <= fMaxRelativeRatio && relativeRatio >= (1.0 / fMaxRelativeRatio) )
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
             mpDetectedGestures->faces[newCount].rect[0] = mpDetectedGestures->faces[gi].rect[0];
             mpDetectedGestures->faces[newCount].rect[1] = mpDetectedGestures->faces[gi].rect[1];
             mpDetectedGestures->faces[newCount].rect[2] = mpDetectedGestures->faces[gi].rect[2];
             mpDetectedGestures->faces[newCount].rect[3] = mpDetectedGestures->faces[gi].rect[3];
             mpDetectedGestures->faces[newCount].score = mpDetectedGestures->faces[gi].score;
             mpDetectedGestures->faces[newCount].id = mpDetectedGestures->faces[gi].id;
             mpDetectedGestures->faces[newCount].left_eye[0] = mpDetectedGestures->faces[gi].left_eye[0];
             mpDetectedGestures->faces[newCount].left_eye[1] = mpDetectedGestures->faces[gi].left_eye[1];
             mpDetectedGestures->faces[newCount].right_eye[0] = mpDetectedGestures->faces[gi].right_eye[0];
             mpDetectedGestures->faces[newCount].right_eye[1] = mpDetectedGestures->faces[gi].right_eye[1];
             mpDetectedGestures->faces[newCount].mouth[0] = mpDetectedGestures->faces[gi].mouth[0];
             mpDetectedGestures->faces[newCount].mouth[1] = mpDetectedGestures->faces[gi].mouth[1];
             mpDetectedGestures->posInfo[newCount].rop_dir = mpDetectedGestures->posInfo[gi].rop_dir;
             mpDetectedGestures->posInfo[newCount].rip_dir = mpDetectedGestures->posInfo[gi].rip_dir;
             ++newCount;
          }
       }
       // number of gesture rectangles after filtering
       mpDetectedGestures->number_of_faces = newCount;
       GD_Result = newCount;

       // debug message
       if (GD_Result == 0)
          MY_LOGD("Scenario GD: Gesture detected but filtered out by face!!!");
    }
    /////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////
    // cpchen: face is a prerequiste of gesture shot, no face no gesture shot
    /////////////////////////////////////////////////////////////////////
    bool bEnableGSPrerequisteWithFD = true;
    if (bEnableGSPrerequisteWithFD && mpDetectedFaces->number_of_faces == 0)
    {
       mpDetectedGestures->number_of_faces = 0;
       GD_Result = 0;

       // debug message
       MY_LOGD("Scenario GD: Gesture detected but no face!");
    }
    /////////////////////////////////////////////////////////////////////

    MY_LOGD("Scenario GD Result: %d",GD_Result );
    if(GD_Result>0)
        rIsDetected_GD = 1;
   else
        rIsDetected_GD = 0;
#endif
}
    //mpFDHalObj->halFDASDBufferCreate(dst_rgb, src_buf, plane);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "-");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/

int
FDClient::
doBufferAnalysis(ImgBufQueNode const& a_rQueNode)
{
    int BufWidth = a_rQueNode.getImgBuf()->getImgWidth();
    int BufHeight  = a_rQueNode.getImgBuf()->getImgHeight();
    //plane = a_rQueNode.getPlaneCount();
    plane = FmtUtils::queryPlaneCount(a_rQueNode.getImgBuf()->getImgFormat().string());
    //MY_LOGD("getImgWidth:%d, getImgHeight:%d", a_rQueNode.getImgBuf()->getImgWidth(), a_rQueNode.getImgBuf()->getImgHeight());

    MY_LOGD("plane: %d",plane );

    //Need PlaneCount Information
    if( (BufWidth==640) && (BufHeight == 480) && (plane==2))
        return 0;
    else if ( (BufWidth*3 == BufHeight*4) && (plane==2) )
        return 1;
    else if ( (BufWidth*3 == BufHeight*4) && (plane==3) )
        return 2;
    else if ( ( BufWidth*9 == BufHeight*16) && (plane==2) )
        return 3;
    else if ( ( BufWidth*9 == BufHeight*16) && (plane==3) )
        return 4;
    else if ( ( BufWidth*3 == BufHeight*5) && (plane==2) )
        return 5;
    else if ( ( BufWidth*3 == BufHeight*5) && (plane==3) )
        return 6;
    else if ( (plane==2) )
        return 3;
    else if ( (plane==3) )
        return 4;
    else
        return 9;

    return plane;

}

