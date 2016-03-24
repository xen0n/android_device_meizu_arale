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

#define LOG_TAG "MtkSdk/Gesture"

#include <common.h>
//
using namespace NSCam;
#include <binder/MemoryHeapBase.h>

#include "../inc/Utils.h"

using namespace android;
using namespace MtkCamUtils;
using namespace NSCam::Utils;

#include "ImgBufManager.h"
//
using namespace NSSdkClient;
using namespace NSGestureClient;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[ImgBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[ImgBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[ImgBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[ImgBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[ImgBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[ImgBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[ImgBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)



/******************************************************************************
 * New and Allocate Image buffer
 ******************************************************************************/
ClientImgBuf*
ClientImgBuf::
alloc(
    sp<ImgInfo const>const& rpImgInfo
)
{
    ClientImgBuf* pClientImgBuf = NULL;

    sp<MemoryHeapBase> pMemHeapBase = new MemoryHeapBase(rpImgInfo->mImgBufSize, 0, rpImgInfo->ms8ImgName);
    if  ( pMemHeapBase == 0 )
    {
        CAM_LOGE("[ClientImgBuf::alloc] cannot new MemoryHeapBase");
        goto lbExit;
    }
    pClientImgBuf = new ClientImgBuf(pMemHeapBase, rpImgInfo);
    //
lbExit:
    return  pClientImgBuf;
}


/******************************************************************************
 *
 ******************************************************************************/
ClientImgBuf::
ClientImgBuf(
    sp<MemoryHeapBase> const &pMemoryHeapBase,
    sp<ImgInfo const>const& rpImgInfo,
    int64_t const timestamp /*= 0*/
)
    : mpImgInfo(rpImgInfo)
    , mi8Timestamp(timestamp)
    , mpMemoryHeapBase(pMemoryHeapBase)
{
    CAM_LOGD("[ClientImgBuf::ClientImgBuf]");
}


/******************************************************************************
 * Free and deallocate image
 ******************************************************************************/
ClientImgBuf::
~ClientImgBuf()
{
    CAM_LOGD(
        "[ClientImgBuf::~ClientImgBuf] "
        "Buffer[%s@0x%08X@%d@%s@(%d)%dx%d@%d@Timestamp(%lld)] ",
        getBufName(), getVirAddr(), getBufSize(), getImgFormat().string(),
        getImgWidthStride(), getImgWidth(), getImgHeight(),
        getBitsPerPixel(), getTimestamp()
    );
    //
    mpMemoryHeapBase = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ClientImgBuf::
dump() const
{
    CAM_LOGD(
        "[ClientImgBuf::dump] "
        "Buffer[%s@0x%08X@%d@%s@(%d)%dx%d@%d@Timestamp(%lld)]",
        getBufName(), getVirAddr(), getBufSize(), getImgFormat().string(),
        getImgWidthStride(), getImgWidth(), getImgHeight(),
        getBitsPerPixel(), getTimestamp()
    );
}




/******************************************************************************
 *
 ******************************************************************************/
ImgBufManager*
ImgBufManager::
alloc(
    char const*const        szImgFormat,
    uint32_t const          u4ImgWidth,
    uint32_t const          u4ImgHeight,
    uint32_t const          u4BufCount,
    char const*const        szName
)
{
    ImgBufManager* pMgr = new ImgBufManager(
        szImgFormat, u4ImgWidth, u4ImgHeight,
        u4BufCount, szName
    );
    //
    if  ( pMgr && ! pMgr->init() )
    {
        // return NULL due to init failure.
        pMgr = NULL;
    }
    //
    return pMgr;
}


/******************************************************************************
 *
 ******************************************************************************/
ImgBufManager::
ImgBufManager(
    char const*const        szImgFormat,
    uint32_t const          u4ImgWidth,
    uint32_t const          u4ImgHeight,
    uint32_t const          u4BufCount,
    char const*const        szName
)
    : RefBase()
    //
    , ms8Name(szName)
    , ms8ImgFormat(szImgFormat)
    , mu4ImgWidth(u4ImgWidth)
    , mu4ImgHeight(u4ImgHeight)
    , mu4BufCount(u4BufCount)
    //
    , mvImgBuf()
    //
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
ImgBufManager::
~ImgBufManager()
{
    uninit();
    //
    MY_LOGD("");
}



/******************************************************************************
 *
 ******************************************************************************/
bool
ImgBufManager::
init()
{
    MY_LOGD("+ mu4BufCount(%d)", mu4BufCount);
    //
    mvImgBuf.clear();
    for (size_t i = 0; i < mu4BufCount; i++)
    {
        ClientImgBuf* pClientImgBuf = ClientImgBuf::alloc(
            new ImgInfo(
                mu4ImgWidth,
                mu4ImgHeight,
                ms8ImgFormat,
                ms8Name
            )
        );
        if  ( pClientImgBuf == 0 )
        {
            MY_LOGE("cannot allocate pClientImgBuf [%d]", i);
            goto lbExit;
        }
        //
        pClientImgBuf->dump();
        mvImgBuf.push_back(pClientImgBuf);
        if  ( mvImgBuf[i] == 0 )
        {
            MY_LOGE("cannot allocate mvImgBuf[%d]", i);
            goto lbExit;
        }
    }
    //
    //
    MY_LOGD("- ret(1)");
    return true;
lbExit:
    MY_LOGD("- ret(0)");
    uninit();
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ImgBufManager::
uninit()
{
    MY_LOGD("+ mu4BufCount(%d)", mu4BufCount);
    //
    for (size_t i = 0; i < mu4BufCount; i++)
    {
        mvImgBuf.editItemAt(i) = NULL;
    }
    //
    MY_LOGD("-");
}

