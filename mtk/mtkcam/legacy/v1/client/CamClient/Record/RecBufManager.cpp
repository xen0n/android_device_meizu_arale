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

#define LOG_TAG "MtkCam/RecordClient"
//
#include <linux/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ion/ion.h>
#include <ion.h>
#include <binder/MemoryHeapBase.h>
#include <camera/CameraParameters.h>
#include <camera/MtkCameraParameters.h>
//
#include "../MyUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include "RecBufManager.h"
//
using namespace NSCamClient;
using namespace NSRecordClient;
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#if defined(MTK_ION_SUPPORT)
#define REC_BUF_ION     (1)
#else
#define REC_BUF_ION     (0)
#endif

/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf*
RecImgBuf::
alloc(
    camera_request_memory   requestMemory,
    sp<RecImgInfo>&         rpImgInfo
)
{
    bool ret = true;
    RecImgBuf* pRecImgBuf = NULL;
    camera_memory_t* camera_memory = NULL;
    int32_t IonDevFd = -1,IonBufFd = -1, IonBufFlag = 0,ImgBufSize = 0;
    ion_user_handle_t IonHandle = NULL;
    sp<GraphicBuffer> spGraphicBuffer = NULL;
    //
    if(rpImgInfo->mbMetaMode)
    {
        //(1) create gralloc buffer
        CAM_LOGD("[RecImgBuf::alloc][GraphicBuffer] Size(%d x %d), F(0x%X), BU(0x%X)",
                rpImgInfo->mu4ImgWidth,
                rpImgInfo->mu4ImgHeight,
                rpImgInfo->mu4ImgFormat,
                rpImgInfo->mu4BufUsage);
        spGraphicBuffer = new GraphicBuffer(rpImgInfo->mu4ImgWidth,
                                            rpImgInfo->mu4ImgHeight,
                                            rpImgInfo->mu4ImgFormat,
                                            rpImgInfo->mu4BufUsage);


        // (2) create camera_memory buffer
        //* --------------------------------------------------------------
        //* |  kMetadataBufferTypeGrallocSource | sizeof(buffer_handle_t) |
        //* --------------------------------------------------------------
        ImgBufSize = sizeof(int)+sizeof(buffer_handle_t);
    }
    else
    {
        ImgBufSize = rpImgInfo->mImgBufSize;
        IonBufFlag = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
    }
    //
    #if REC_BUF_ION
    //
    CAM_LOGD("[RecImgBuf::alloc][mt_ion_open]");
    IonDevFd = mt_ion_open(LOG_TAG);
    if(IonDevFd < 0)
    {
        CAM_LOGE("[RecImgBuf::alloc]ion_open fail");
        goto lbExit;
    }
    //
    CAM_LOGD("[RecImgBuf::alloc][ion_alloc_mm]IonDevFd(%d), ImgBufSize(%d)",
            IonDevFd,
            ImgBufSize);
    if(ion_alloc_mm(
        IonDevFd,
        ImgBufSize,
        32,
        IonBufFlag,
        &IonHandle))
    {
        CAM_LOGE("[RecImgBuf::alloc]ion_alloc_mm fail");
        goto lbExit;
    }
    //
    CAM_LOGD("[RecImgBuf::alloc][ion_share]");
    if(ion_share(
        IonDevFd,
        IonHandle,
        &IonBufFd))
    {
        CAM_LOGE("[RecImgBuf::alloc]ion_share fail");
        goto lbExit;
    }
    //
    CAM_LOGD("[RecImgBuf::alloc][requestMemory] fd(%d), S(%d)", IonBufFd, ImgBufSize);
    camera_memory = requestMemory(IonBufFd, ImgBufSize, 1, NULL);
    if  ( ! camera_memory )
    {
        ret = false;
        CAM_LOGE("[RecImgBuf::alloc][requestMemory] id:%d, size:%d", IonBufFd, ImgBufSize);
        goto lbExit;
    }
    //
    #else
    //
    CAM_LOGD("[RecImgBuf::alloc][MemoryHeapBase]");
    sp<MemoryHeapBase> pMemHeapBase = new MemoryHeapBase(ImgBufSize, 0, rpImgInfo->ms8ImgName);
    if  ( pMemHeapBase == 0 )
    {
        CAM_LOGE("[RecImgBuf::alloc][PrvCbImgBuf::alloc] cannot new MemoryHeapBase");
        goto lbExit;
    }
    //
    CAM_LOGD("[RecImgBuf::alloc][requestMemory]");
    camera_memory = requestMemory(pMemHeapBase->getHeapID(), ImgBufSize, 1, NULL);
    if  ( ! camera_memory )
    {
        ret = false;
        CAM_LOGE("[RecImgBuf::alloc][requestMemory] id:%d, size:%d", pMemHeapBase->getHeapID(), ImgBufSize);
        goto lbExit;
    }
    //
    pMemHeapBase = 0;
    //
    #endif
    //
    pRecImgBuf = new RecImgBuf(
                            *camera_memory,
                            rpImgInfo,
                            IonDevFd,
                            IonBufFd,
                            IonHandle,
                            spGraphicBuffer);
lbExit:
    return  ret == true ? pRecImgBuf : NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf::
RecImgBuf(
    camera_memory_t const&      rCamMem,
    sp<RecImgInfo>&             rpImgInfo,
    int32_t                     IonDevFd,
    int32_t                     IonBufFd,
    ion_user_handle_t           IonHandle,
    sp<GraphicBuffer>           spGraphicBuffer
)
    : ICameraImgBuf()
    , mpImgInfo(rpImgInfo)
    , mi8Timestamp(0)
    , mCamMem(rCamMem)
    , mIonDevFd(IonDevFd)
    , mIonBufFd(IonBufFd)
    , mIonHandle(IonHandle)
    , mpGraphicBuffer(spGraphicBuffer)
    , mpBufVA(NULL)
    , mpBufPA(NULL)
{
    #if REC_BUF_ION
    CAM_LOGD("[RecImgBuf::RecImgBuf]ION");
    #else
    CAM_LOGD("[RecImgBuf::RecImgBuf]M4U");
    #endif

    if(mpImgInfo->mbMetaMode)
    {
        CAM_LOGD("[RecImgBuf::RecImgBuf]gralloc on");
        //
        //(.1) query image real format
        gralloc_extra_query(mpGraphicBuffer->handle, GRALLOC_EXTRA_GET_FORMAT, &mpImgInfo->mu4ImgFormat);

        //(.2) query buffer stride
        if( mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_YV12 ||
            mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_I420)
        {
            int UStrideMask = 15;
            int VStrideMask = 15;  //because of no way to get U and V stride
            mpImgInfo->mu4ImgStrideY = mpGraphicBuffer->getStride();
            mpImgInfo->mu4ImgStrideU = ((~UStrideMask) & (UStrideMask + (rpImgInfo->mu4ImgWidth >> 1)));
            mpImgInfo->mu4ImgStrideV = ((~VStrideMask) & (VStrideMask + (rpImgInfo->mu4ImgWidth >> 1)));

            //(.3) calculate buffer size
            mpImgInfo->mImgBufSize = mpImgInfo->mu4ImgStrideY * (rpImgInfo->mu4ImgHeight);
            mpImgInfo->mImgBufSize += mpImgInfo->mu4ImgStrideU * (rpImgInfo->mu4ImgHeight >> 1);
            mpImgInfo->mImgBufSize += mpImgInfo->mu4ImgStrideV * (rpImgInfo->mu4ImgHeight >> 1);

            //(.4)
            mpImgInfo->mu4BitsPerPixel = 12;
        }
        else
        {
            //TODO
        }

        //(.5) query buffer ion fd
        mi4BufIonFd = -1;
        gralloc_extra_query(mpGraphicBuffer->handle, GRALLOC_EXTRA_GET_ION_FD, &mi4BufIonFd);
        if ( mi4BufIonFd < 0 )
        {
            CAM_LOGD("[RecImgBuf::RecImgBuf] fd:%d <= 0", mi4BufIonFd);
        }

        if(mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_YV12)
        {
            mpImgInfo->ms8ImgFormat = CameraParameters::PIXEL_FORMAT_YUV420P;
        }
        else
        if(mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_I420)
        {
            mpImgInfo->ms8ImgFormat = MtkCameraParameters::PIXEL_FORMAT_YUV420I;
        }
        else
        {
            //TODO
        }

        CAM_LOGD("[RecImgBuf::RecImgBuf][handle] mpGraphicBuffer->handle: 0x%8x", mpGraphicBuffer->handle);
        CAM_LOGD("[RecImgBuf::RecImgBuf][callbackMem] fd(%d), mem(0x%X), VA(0x%X), S(%d)",
                mIonBufFd,
                (uint32_t*)&mCamMem,
                (uint32_t*)mCamMem.data,
                mCamMem.size);
        CAM_LOGD("[GrallocMem] W(%d), H(%d), F(%s), fd(%d), stride{%d, %d, %d}, S(%d)",
                getImgWidth(),
                getImgHeight(),
                getImgFormat().string(),
                getIonFd(), getImgWidthStride(0),
                getImgWidthStride(1),
                getImgWidthStride(2),
                getBufSize());
    }
    else
    {
        CAM_LOGD("[RecImgBuf::RecImgBuf]gralloc off");
    }


}


/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf::
~RecImgBuf()
{
    struct ion_handle_data IonHandleData;
    //
    CAM_LOGD(
        "[RecImgBuf::~RecImgBuf]"
        "Name(%s),ION(%d),VA(0x%08X),Size(%d),Fmt(%s),Str(%d),W(%d),H(%d),BPP(%d),TS(%lld)",
        getBufName(),
        getIonFd(),
        getVirAddr(),
        getBufSize(),
        getImgFormat().string(),
        getImgWidthStride(),
        getImgWidth(),
        getImgHeight(),
        getBitsPerPixel(),
        getTimestamp()
    );

    //
    #if REC_BUF_ION
    //
    if(mIonDevFd >= 0)
    {
        if(mIonBufFd >= 0)
        {
            ion_share_close(
                mIonDevFd,
                mIonBufFd);
        }
        //
        ion_free(
            mIonDevFd,
            mIonHandle);
        //
        ion_close(mIonDevFd);
    }
    #endif
    if  ( mCamMem.release )
    {
        mCamMem.release(&mCamMem);
        mCamMem.release = NULL;
    }
    //
    if(mpImgInfo->mbMetaMode)
    {
        if (mpGraphicBuffer != 0)
        {
            mpGraphicBuffer = 0;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecImgBuf::
dump() const
{
    CAM_LOGD(
        "[RecImgBuf::dump]"
        "Name(%s),ION(%d),VA(0x%08X),Size(%d),Fmt(%s),Str(%d),W(%d),H(%d),BPP(%d),TS(%lld),S/C(%d/%d)",
        getBufName(),
        getIonFd(),
        getVirAddr(),
        getBufSize(),
        getImgFormat().string(),
        getImgWidthStride(),
        getImgWidth(),
        getImgHeight(),
        getBitsPerPixel(),
        getTimestamp(),
        getBufSecu(),
        getBufCohe()
    );
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager*
RecBufManager::
alloc(
    bool const              bMetaMode,
    char const*const        szImgFormat,
    uint32_t const          u4BufUsage,
    uint32_t const          u4ImgFormat,
    uint32_t const          u4ImgWidth,
    uint32_t const          u4ImgHeight,
    uint32_t const          u4ImgStrideY,
    uint32_t const          u4ImgStrideU,
    uint32_t const          u4ImgStrideV,
    uint32_t const          u4BufCount,
    char const*const        szName,
    camera_request_memory   requestMemory,
    int32_t const           i4BufSecu,
    int32_t const           i4BufCohe
)
{
    RecBufManager* pMgr = new RecBufManager(
                                bMetaMode,
                                szImgFormat,
                                u4BufUsage,
                                u4ImgFormat,
                                u4ImgWidth,
                                u4ImgHeight,
                                u4ImgStrideY,
                                u4ImgStrideU,
                                u4ImgStrideV,
                                u4BufCount,
                                szName,
                                requestMemory,
                                i4BufSecu,
                                i4BufCohe);
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
RecBufManager::
RecBufManager(
    bool const              bMetaMode,
    char const*const        szImgFormat,
    uint32_t const          u4BufUsage,
    uint32_t const          u4ImgFormat,
    uint32_t const          u4ImgWidth,
    uint32_t const          u4ImgHeight,
    uint32_t const          u4ImgStrideY,
    uint32_t const          u4ImgStrideU,
    uint32_t const          u4ImgStrideV,
    uint32_t const          u4BufCount,
    char const*const        szName,
    camera_request_memory   requestMemory,
    int32_t const           i4BufSecu,
    int32_t const           i4BufCohe
)
    : RefBase()
    //
    , ms8Name(szName)
    , mbMetaMode(bMetaMode)
    , ms8ImgFormat(szImgFormat)
    , mu4BufUsage(u4BufUsage)
    , mu4ImgFormat(u4ImgFormat)
    , mu4ImgWidth(u4ImgWidth)
    , mu4ImgHeight(u4ImgHeight)
    , mu4ImgStrideY(u4ImgStrideY)
    , mu4ImgStrideU(u4ImgStrideU)
    , mu4ImgStrideV(u4ImgStrideV)
    , mu4BufCount(u4BufCount)
    //
    , mvImgBuf()
    , mRequestMemory(requestMemory)
    //
    , mi4BufSecu(i4BufSecu)
    , mi4BufCohe(i4BufCohe)
    //
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager::
~RecBufManager()
{
    uninit();
    //
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecBufManager::
init()
{
    MY_LOGD("+ mu4BufCount(%d)", mu4BufCount);
    //
    mvImgBuf.clear();
    mvImgBufIonFd.clear();
    for (size_t i = 0; i < mu4BufCount; i++)
    {
        sp<RecImgInfo> spRecImgInfo = new RecImgInfo(
                                            mbMetaMode,
                                            mu4BufUsage,
                                            mu4ImgFormat,
                                            mu4ImgWidth,
                                            mu4ImgHeight,
                                            mu4ImgStrideY,
                                            mu4ImgStrideU,
                                            mu4ImgStrideV,
                                            ms8ImgFormat,
                                            ms8Name,
                                            mi4BufSecu,
                                            mi4BufCohe);
        RecImgBuf* pRecImgBuf = RecImgBuf::alloc(
            mRequestMemory,
            spRecImgInfo
        );
        if  ( pRecImgBuf == 0 )
        {
            MY_LOGE("cannot allocate pRecImgBuf [%d]", i);
            goto lbExit;
        }
        //
        pRecImgBuf->dump();
        mvImgBuf.push_back(pRecImgBuf);
        if  ( mvImgBuf[i] == 0 )
        {
            MY_LOGE("cannot allocate mvImgBuf[%d]", i);
            goto lbExit;
        }
        //
        mvImgBufIonFd.push_back(pRecImgBuf->getIonFd());
        if  ( mvImgBufIonFd[i] == 0 )
        {
            MY_LOGE("cannot allocate mvImgBufIonFd[%d]", i);
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
RecBufManager::
uninit()
{
    MY_LOGD("+ mvImgBuf.size(%d), mu4BufCount(%d)", mvImgBuf.size(), mu4BufCount);
    //
    for (size_t i = 0; i < mvImgBuf.size(); i++)
    {
        mvImgBuf.editItemAt(i) = NULL;
    }
    //
    mvImgBufIonFd.clear();
    //
    MY_LOGD("-");
}

