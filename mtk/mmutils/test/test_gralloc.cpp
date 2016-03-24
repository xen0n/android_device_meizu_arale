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

#define LOG_TAG "mmutils/test"
//
#include "../inc/Log.h"
#include <utils/String8.h>
#include <ui/GraphicBuffer.h>
#include <IGrallocHelper.h>
using namespace android;
using namespace mmutils;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        MM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        MM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        MM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        MM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        MM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        MM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        MM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
#define ARRAYOF(a) (sizeof(a)/(sizeof(a[0])))


/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
static int const
gTestFormat[] = {
    HAL_PIXEL_FORMAT_BLOB,
    HAL_PIXEL_FORMAT_YV12,
    HAL_PIXEL_FORMAT_YCrCb_420_SP,
    HAL_PIXEL_FORMAT_YCbCr_422_I,
#if 0
    HAL_PIXEL_FORMAT_YCbCr_422_SP,
    //
    HAL_PIXEL_FORMAT_Y8,
    HAL_PIXEL_FORMAT_Y16,
#endif
    //
    HAL_PIXEL_FORMAT_YCbCr_420_888,
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED,
    //
    HAL_PIXEL_FORMAT_RGBA_8888,
#if 0
    HAL_PIXEL_FORMAT_RGBX_8888,
    HAL_PIXEL_FORMAT_RGB_888,
    HAL_PIXEL_FORMAT_RGB_565,
    HAL_PIXEL_FORMAT_BGRA_8888,
#endif
    //
    HAL_PIXEL_FORMAT_RAW_OPAQUE,
    HAL_PIXEL_FORMAT_RAW16,
};


/**
 *
 */
static int const
gTestUsage[] = {
#define GRALLOC_USAGE_SW_RW (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN)

#if 0
    0,
    (GRALLOC_USAGE_SW_RW),
#endif

    (                      GRALLOC_USAGE_HW_VIDEO_ENCODER),
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_VIDEO_ENCODER),
    //
    (                      GRALLOC_USAGE_HW_TEXTURE),
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_TEXTURE),
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_CAMERA_ZSL),
    //
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_CAMERA_ZSL),

};


/**
 *
 */
static int const
gTestResolution[][2] = {
    { 176,  144},
    { 320,  240},
    { 640,  480},
    { 800,  600},
    { 960,  540},
    {1280,  720},
    {1920, 1080},
    {3224, 2448},
};


};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
static
void
test_dump()
{
    IGrallocHelper const* pGrallocHelper = IGrallocHelper::singleton();
    pGrallocHelper->dumpToLog();
}


/******************************************************************************
 *
 ******************************************************************************/
static
void
test_static()
{
    ::printf("+ test_static \n");
    //
    IGrallocHelper const* pGrallocHelper = IGrallocHelper::singleton();
    //
    for (size_t i = 0; i < ARRAYOF(gTestResolution); i++) {
        for (size_t j = 0; j < ARRAYOF(gTestFormat); j++) {
            for (size_t k = 0; k < ARRAYOF(gTestUsage); k++) {
                int const format = gTestFormat[j];
                int const usage  = gTestUsage[k];
                int w = gTestResolution[i][0];
                int h = gTestResolution[i][1];
                if  ( HAL_PIXEL_FORMAT_BLOB == format ) {
                    w = w * h;
                    h = 1;
                }
                GrallocStaticInfo staticInfo;
                GrallocRequest request;
                request.usage           = usage;
                request.format          = format;
                request.widthInPixels   = w;
                request.heightInPixels  = h;
                int status = pGrallocHelper->query(&request, &staticInfo);
                if  ( 0 != status ) {
                    MY_LOGE(
                        "[FAILURE] status:%d %dx%d format:%#x(%s) usage:%#x(%s)",
                        status, w, h,
                        format, pGrallocHelper->queryPixelFormatName(format).string(),
                        usage, pGrallocHelper->queryGrallocUsageName(usage).string()
                    );
                    continue;
                }
            }
        }
    }
    //
    ::printf("- test_static \n");
}


/******************************************************************************
 *
 ******************************************************************************/
static
void
test_dynamic()
{
    ::printf("+ test_dynamic \n");
    //
    IGrallocHelper const* pGrallocHelper = IGrallocHelper::singleton();
    //
    for (size_t i = 0; i < ARRAYOF(gTestResolution); i++) {
        for (size_t j = 0; j < ARRAYOF(gTestFormat); j++) {
            for (size_t k = 0; k < ARRAYOF(gTestUsage); k++) {
                int const format = gTestFormat[j];
                int const usage  = gTestUsage[k];
                int w = gTestResolution[i][0];
                int h = gTestResolution[i][1];
                if  ( HAL_PIXEL_FORMAT_BLOB == format ) {
                    w = w * h;
                    h = 1;
                }

                GrallocStaticInfo staticInfo;
                GrallocDynamicInfo dynamicInfo;
                GrallocRequest request;
                sp<GraphicBuffer> pGB = new GraphicBuffer(w, h, format, usage);
                if  ( pGB == 0 || ! pGB->handle ) {
                    MY_LOGW(
                        "Bad GraphicBuffer:%p buffer_handle_t:%p %dx%d format:%#x(%s) usage:%#x(%s)",
                        pGB.get(), pGB->handle, w, h,
                        format, pGrallocHelper->queryPixelFormatName(format).string(),
                        usage, pGrallocHelper->queryGrallocUsageName(usage).string()
                    );
                    continue;
                }
                //
                int status = pGrallocHelper->query(pGB->handle, &staticInfo, &dynamicInfo);
                if  ( 0 != status ) {
                    MY_LOGE(
                        "[FAILURE] buffer_handle_t:%p status:%d %dx%d format:%#x(%s) usage:%#x(%s)",
                        pGB->handle, status, w, h,
                        format, pGrallocHelper->queryPixelFormatName(format).string(),
                        usage, pGrallocHelper->queryGrallocUsageName(usage).string()
                    );
                    continue;
                }
                //
                MY_LOGD("*****************************************************************************");
                MY_LOGD(
                    "%dx%d usage:0x%08x(%s) format:0x%08x(%s) -->",
                    w, h,
                    usage, pGrallocHelper->queryGrallocUsageName(usage).string(),
                    format, pGrallocHelper->queryPixelFormatName(format).string()
                );
                MY_LOGD(
                    "%dx%d usage:0x%08x(%s) format:0x%08x(%s)",
                    pGB->getWidth(), pGB->getHeight(),
                    usage, pGrallocHelper->queryGrallocUsageName(usage).string(),
                    staticInfo.format, pGrallocHelper->queryPixelFormatName(staticInfo.format).string()
                );
                for (size_t i_plane = 0; i_plane < staticInfo.planes.size(); i_plane++) {
                    MY_LOGD(
                        "  [%zu] sizeInBytes:%zu rowStrideInBytes:%zu",
                        i_plane,
                        staticInfo.planes[i_plane].sizeInBytes,
                        staticInfo.planes[i_plane].rowStrideInBytes
                    );
                }
                String8 s8IonFds;
                s8IonFds.appendFormat("buffer_handle_t:%p #ion_fd=%zu ==> ", pGB->handle, dynamicInfo.ionFds.size());
                for (size_t i_ion = 0; i_ion < dynamicInfo.ionFds.size(); i_ion++) {
                    s8IonFds.appendFormat("%d ", dynamicInfo.ionFds[i_ion]);
                }
                MY_LOGD("%s", s8IonFds.string());
            }
        }
    }
    //
    ::printf("- test_dynamic \n");
}


/******************************************************************************
 *  Main
 ******************************************************************************/
int
main(int argc , char *argv[])
{
    MY_LOGD("argc:%d", argc);
    for (int i = 0; i < argc; i++) {
        MY_LOGD("%s", argv[i]);
    }
    //
    ::printf("========================= \n");
    switch  (argc)
    {
    case 2:
        if  ( 0 == ::strcmp(argv[1], "s") ) {
            test_static();
            test_dump();
        }
        else if  ( 0 == ::strcmp(argv[1], "d") ) {
            test_dynamic();
            test_dump();
        }
        break;
        //
    default:
        test_static();
        test_dynamic();
        test_static();
        test_dump();
        break;
    }
    //
    return  0;
}

