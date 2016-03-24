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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
#include <cutils/properties.h>  // For property_get().

using namespace android;
using namespace MtkCamUtils;
//
#include <camera/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkEngCamAdapter.h"
using namespace NSMtkEngCamAdapter;
//
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/featureio/capturenr.h>
#include <camera_custom_capture_nr.h>


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
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
*******************************************************************************/
static MBOOL dumpCaptureNRThresholds(MUINT32 openId, char* const filename);
/******************************************************************************
*
*******************************************************************************/
MVOID
CamAdapter::
dumpTuningParams()
{
    mkdir("/sdcard/isp", S_IRWXU | S_IRWXG | S_IRWXO);
    {
        const char* NRTHRES_FILENAME = "/sdcard/isp/nr.thres";
        // dump capture multi-pass nr thresholds
        if( !dumpCaptureNRThresholds(getOpenId(), const_cast<char*>(NRTHRES_FILENAME)) )
        {
            MY_LOGE("dump nr threshold failed");
        }
    }

    {
      const  char* SWNR_FILENAME =  "/sdcard/isp/swnr.params";
        // dump swnr params
        if( !SwNRParam::getInstance(getOpenId())->dumpParamsToFile( const_cast<char*>(SWNR_FILENAME)) )
        {
            MY_LOGE("dump swnr params failed");
        }
    }
}

MBOOL
dumpCaptureNRThresholds(MUINT32 openId, char* const filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp ) {
        CAM_LOGE("fopen fail: %s", filename);
        return MFALSE;
    }
    else {
        NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
        if( pHalSensorList->queryType(openId) == NSCam::NSSensorType::eYUV ) {
            // yuv sensor not support multi-pass NR
            return MTRUE;
        }
        Capture_NR_Th_t NR_thres;
        MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(openId);
        //fwrite(#shotmode, 1, strlen(#shotmode), fp );
#define FWRITE_MODE( shotmode, usemfll )                                          \
        do {                                                                      \
            get_capture_nr_th(sensorDev, shotmode, usemfll, &NR_thres);           \
            fprintf(fp, "%d(%s),%d:%d/%d\n",                                      \
                    shotmode, #shotmode, usemfll, NR_thres.hwth, NR_thres.swth);  \
        } while(0)

        FWRITE_MODE(eShotMode_NormalShot       , MFALSE);
        FWRITE_MODE(eShotMode_ContinuousShot   , MFALSE);
        FWRITE_MODE(eShotMode_ContinuousShotCc , MFALSE);
        FWRITE_MODE(eShotMode_SmileShot        , MFALSE);
        FWRITE_MODE(eShotMode_HdrShot          , MFALSE);
        FWRITE_MODE(eShotMode_AsdShot          , MFALSE);
        FWRITE_MODE(eShotMode_ZsdShot          , MFALSE);
        FWRITE_MODE(eShotMode_FaceBeautyShot   , MFALSE);
        FWRITE_MODE(eShotMode_Mav              , MFALSE);
        FWRITE_MODE(eShotMode_Autorama         , MFALSE);
        FWRITE_MODE(eShotMode_MultiMotionShot  , MFALSE);
        FWRITE_MODE(eShotMode_Panorama3D       , MFALSE);
        FWRITE_MODE(eShotMode_Single3D         , MFALSE);
        FWRITE_MODE(eShotMode_VideoSnapShot    , MFALSE);

        FWRITE_MODE(eShotMode_NormalShot       , MTRUE);
        FWRITE_MODE(eShotMode_ContinuousShot   , MTRUE);
        FWRITE_MODE(eShotMode_ContinuousShotCc , MTRUE);
        FWRITE_MODE(eShotMode_SmileShot        , MTRUE);
        FWRITE_MODE(eShotMode_HdrShot          , MTRUE);
        FWRITE_MODE(eShotMode_AsdShot          , MTRUE);
        FWRITE_MODE(eShotMode_ZsdShot          , MTRUE);
        FWRITE_MODE(eShotMode_FaceBeautyShot   , MTRUE);
        FWRITE_MODE(eShotMode_Mav              , MTRUE);
        FWRITE_MODE(eShotMode_Autorama         , MTRUE);
        FWRITE_MODE(eShotMode_MultiMotionShot  , MTRUE);
        FWRITE_MODE(eShotMode_Panorama3D       , MTRUE);
        FWRITE_MODE(eShotMode_Single3D         , MTRUE);
        FWRITE_MODE(eShotMode_VideoSnapShot    , MTRUE);

        fclose(fp);
    }
    return MTRUE;
}

