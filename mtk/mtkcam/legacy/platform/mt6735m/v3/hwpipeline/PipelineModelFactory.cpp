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

#define LOG_TAG "MtkCam/HwPipeline"
//
#include "MyUtils.h"
#include <mtkcam/v3/hwpipeline/PipelineModelFactory.h>
#include <mtkcam/v3/hwpipeline/PipelineModel_Default.h>
#include <mtkcam/v3/hwpipeline/PipelineModel_Hdr.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
namespace {


typedef ConfigurePipeline::Params           ConfigParamsT;
typedef ConfigurePipeline::ImageStreamsT    ImageStreamsT;
typedef ConfigurePipeline::MetaStreamsT     MetaStreamsT;


/******************************************************************************
 *
 ******************************************************************************/
static
android::sp<IPipelineModel>
configurePipelineModel_Default(ConfigParamsT& rCfgParams)
{
    typedef PipelineModel_Default PipelineT;

    PipelineT::PipeConfigParams appStreams;

    rCfgParams.pPipelineModel = NULL;
    sp<PipelineT> pPipelineModel = PipelineT::create(rCfgParams.openId);;
    if  ( pPipelineModel == 0 ) {
        MY_LOGE("Fail to new a pipeline model");
        return NULL;
    }
    //
    MetaStreamsT const&  vMetaStreams = *rCfgParams.pvMetaStreams;
    ImageStreamsT const& vImageStreams = *rCfgParams.pvImageStreams;
    appStreams.pMeta_Control = vMetaStreams[0];
    for (size_t i = 0; i < vImageStreams.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = vImageStreams[i].get() )
        {
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
            case eImgFmt_BAYER14:
                appStreams.pImage_Raw = pStreamInfo;
                break;
                //
            case eImgFmt_BLOB:
                appStreams.pImage_Jpeg_Stall = pStreamInfo;
                break;
                //
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
                appStreams.vImage_Yuv_NonStall.push_back(pStreamInfo);
                break;
                //
            default:
                MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                break;
            }
        }
    }
    //
    MERROR err = pPipelineModel->configure(appStreams);
    if  ( OK != err ) {
        MY_LOGE("pPipelineModel->configure err:%d", err);
        pPipelineModel = NULL;
        return NULL;
    }
    //
    return pPipelineModel;
}

#ifdef MTK_CAM_HDR_SUPPORT
static
android::sp<IPipelineModel>
configurePipelineModel_Hdr(ConfigParamsT& rCfgParams)
{
    typedef PipelineModel_Hdr PipelineT;
    PipelineT::PipeConfigParams appStreams;
    rCfgParams.pPipelineModel = NULL;
    sp<PipelineT> pPipelineModel = PipelineT::create(rCfgParams.openId);
    if  ( pPipelineModel == 0 ) {
        MY_LOGE("Fail to new a HDR pipeline model");
        return NULL;
    }
    MetaStreamsT const&  vMetaStreams = *rCfgParams.pvMetaStreams;
    ImageStreamsT const& vImageStreams = *rCfgParams.pvImageStreams;
    appStreams.pMeta_Control = vMetaStreams[0];
    for (size_t i = 0; i < vImageStreams.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = vImageStreams[i].get() )
        {
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
            case eImgFmt_BAYER14:
                appStreams.pImage_Raw = pStreamInfo;
                break;
            case eImgFmt_BLOB:
                appStreams.pImage_Jpeg_Stall = pStreamInfo;
                break;
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
                appStreams.vImage_Yuv_NonStall.push_back(pStreamInfo);
                break;
            default:
                MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                break;
            }
        }
    }
    MERROR err = pPipelineModel->configure(appStreams);
    if  ( OK != err ) {
        MY_LOGE("pPipelineModel->configure err:%d", err);
        pPipelineModel = NULL;
        return NULL;
    }
    return pPipelineModel;
}
#endif
}   //namespace


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ConfigurePipeline::
operator()(Params& rParams)
{
    if(rParams.pipelineScene == DEFAULT) {
    rParams.pPipelineModel = configurePipelineModel_Default(rParams);
    }
    else if(rParams.pipelineScene == HDR) {
#ifdef MTK_CAM_HDR_SUPPORT
        rParams.pPipelineModel = configurePipelineModel_Hdr(rParams);
#else
        rParams.pPipelineModel = configurePipelineModel_Default(rParams);
#endif
    }
    //
    if  ( rParams.pPipelineModel == 0 ) {
        return -ENODEV;
    }
    return OK;
}

