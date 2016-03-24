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
#include "RecordClient.h"
#include "RecBufManager.h"
#include <media/hardware/MetadataBufferType.h>
//
using namespace NSCamClient;
using namespace NSRecordClient;
//


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)
#define FPS_CNT_TIME                (990*1000*1000) //ns
#define EMPTY_QUE_WAIT_TIME         (10*1000) //us
#define NO_ENQUE_WAIT_TIME          (10*1000) //us


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
//
#define RECORDING_FORMAT_HAL        (HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
#define RECORDING_GRALLOC_USAGE     (GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_HW_VIDEO_ENCODER)

/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
initBuffers()
{
    bool ret = false;
    //
    //  (1) Lock
    //Mutex::Autolock _l(mModuleMtx);
    //
    //  (2) Allocate buffers.
    if(mpImgBufMgr != 0)
    {
        MY_LOGD("RecBufManager::already alloc()");
        ret = true;
        goto lbExit;
    }
    //
    MY_LOGD("RecBufManager::first time alloc()");
    //
    if(mpParamsMgr->getStr(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES).isEmpty()) {
        //for usb camera
        //Video size should indentical with preview size when video-size-values is null.
        mpParamsMgr->getPreviewSize(&mi4RecWidth, &mi4RecHeight);
    } else {
    	mpParamsMgr->getVideoSize(&mi4RecWidth, &mi4RecHeight);
    }
    //
    if(mbMetaMode)
    {
        MY_LOGD("+ record: WxH=%dx%d, num(%d), FPS(%d), FMT(%d)",
                mi4RecWidth,
                mi4RecHeight,
                mRecBufNum,
                mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE),
                RECORDING_FORMAT_HAL);
    }
    else
    {
        MY_LOGD("+ record: WxH=%dx%d, num(%d), FPS(%d), FMT(%s)",
                mi4RecWidth,
                mi4RecHeight,
                mRecBufNum,
                mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE),
                getImgImgFormat());
    }
    //
    muImgBufIdx = 0;
    //
    if(mbMetaMode)
    {
        mpImgBufMgr = RecBufManager::alloc(
                        mbMetaMode,
                        "",
                        RECORDING_GRALLOC_USAGE,
                        RECORDING_FORMAT_HAL,
                        mi4RecWidth,
                        mi4RecHeight,
                        0,
                        0,
                        0,
                        mRecBufNum,
                        "RecordClientCb",
                        mpCamMsgCbInfo->mRequestMemory,
                        mi4BufSecu,
                        mi4BufCohe);
    }
    else
    {
        mpImgBufMgr = RecBufManager::alloc(
                        mbMetaMode,
                        getImgImgFormat(),
                        0,
                        0,
                        mi4RecWidth,
                        mi4RecHeight,
                        getImgStrideY(),
                        getImgStrideU(),
                        getImgStrideV(),
                        mRecBufNum,
                        "RecordClientCb",
                        mpCamMsgCbInfo->mRequestMemory,
                        mi4BufSecu,
                        mi4BufCohe);
    }

    //
    if  ( mpImgBufMgr == 0 )
    {
        MY_LOGE("RecBufManager::alloc() fail");
        goto lbExit;
    }
    //
    for (int i = 0; i < mRecBufNum; i++)
    {
        REC_BUF_INFO_STRUCT RecBufInfo;
        RecBufInfo.Sta = REC_BUF_STA_EMPTY;
        RecBufInfo.BufIonFd = mpImgBufMgr->getBuf(i)->getIonFd();
        RecBufInfo.CbVirAddr = (void*)(mpImgBufMgr->getBuf(i)->get_camera_memory_virAddr());
        mvRecBufInfo.push_back(RecBufInfo);
    }
    //
    //
    mpExtImgProc = ExtImgProc::createInstance();
    if(mpExtImgProc != NULL)
    {
        mpExtImgProc->init();
    }
    //
    //
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecordClient::
uninitBuffers()
{
    //  (1) Lock
    Mutex::Autolock _l(mBufferMtx);
    //
    //  (2) Free buffers.
    if(mpImgBufMgr != 0)
    {
        mvRecBufInfo.clear();
        muImgBufIdx = 0;
        mpImgBufMgr = 0;
    }
    //
    //
    if(mpExtImgProc != NULL)
    {
        mpExtImgProc->uninit();
        mpExtImgProc->destroyInstance();
        mpExtImgProc = NULL;
    }
    //
    mBufferCond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
prepareAllTodoBuffers(sp<IImgBufQueue>const& rpBufQueue, sp<RecBufManager>const& rpBufMgr)
{
    bool ret = false, findEmpty = false;
    uint32_t i, enqCount = 0, fillCount = 0, cbCount = 0;
    sp<ICameraImgBuf> pCameraImgBuf = NULL;
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    for(i = 0; i < mvRecBufInfo.size(); i++)
    {
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_EMPTY)
        {
            pCameraImgBuf = rpBufMgr->getBuf(i);
            findEmpty = true;
            mvRecBufInfo.editItemAt(i).Sta = REC_BUF_STA_ENQUE;
            //
            if(mbMetaMode)
            {
                void* vaddr = NULL;
                rpBufMgr->getBuf(i)->getGrallocBuffer()->lock(RECORDING_GRALLOC_USAGE, &vaddr);
                rpBufMgr->getBuf(i)->setVirAddr(vaddr);
            }
            MY_LOGD("EQ:Idx(%d),VA(%p),CbVA(%p)",
                    i,
                    (MUINTPTR)(pCameraImgBuf->getVirAddr()),
                    (MUINTPTR)(rpBufMgr->getBuf(i)->get_camera_memory_virAddr()));
            //
            ret = rpBufQueue->enqueProcessor(
                                ImgBufQueNode(pCameraImgBuf, ImgBufQueNode::eSTATUS_TODO));
            //
            if(!ret)
            {
                MY_LOGW("enqueProcessor() fails");
                goto lbExit;
            }
        }
        else
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_ENQUE)
        {
            enqCount++;
        }
        else
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_FILL)
        {
            fillCount++;
        }
        else
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_CB)
        {
            cbCount++;
        }
    }
    //
    if(!findEmpty)
    {
        MY_LOGW("No EMPTY buf:Enq(%d),Fill(%d),CB(%d)",
                enqCount,
                fillCount,
                cbCount);
    }
    //
    ret = true;
lbExit:
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
cancelAllUnreturnBuffers()
{
    uint32_t i;
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    for(i = 0; i < mvRecBufInfo.size(); i++)
    {
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_ENQUE)
        {
            mvRecBufInfo.editItemAt(i).Sta = REC_BUF_STA_EMPTY;
            //
            if(mbMetaMode)
            {
                mpImgBufMgr->getBuf(i)->getGrallocBuffer()->unlock();
            }
        }
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "-");
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
waitAndHandleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue)
{
    bool ret = false;
    uint32_t i,size;
    Vector<ImgBufQueNode> vQueNode;
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //
    {
        Mutex::Autolock _l(mModuleMtx);
        //
        size = mvRecBufInfo.size();
        for(i=0; i<size; i++)
        {
            if(mvRecBufInfo[i].Sta == REC_BUF_STA_ENQUE)
            {
                break;
            }
        }
    }
    //
    if(i == size)
    {
        MY_LOGW("No ENQUE buf");
        usleep(NO_ENQUE_WAIT_TIME);
        return ret;
    }
    //
    //  (1) deque buffers from processor.
    rpBufQueue->dequeProcessor(vQueNode);
    if  ( vQueNode.empty() ) {
        MY_LOGW("vQueNode.empty()");
        usleep(EMPTY_QUE_WAIT_TIME);
        goto lbExit;
    }
    //
    //  (2) handle buffers dequed from processor.
    ret = handleReturnBuffers(vQueNode);

lbExit:
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
handleReturnBuffers(Vector<ImgBufQueNode>const& rvQueNode)
{
    uint32_t i,j;
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //
    //  (1) Lock
    //Mutex::Autolock _l(mModuleMtx);
    //
    //  (2) Remove from List and peform callback, one by one.
    int32_t const queSize = rvQueNode.size();
    for (i = 0; i < queSize; i++)
    {
        if(!(rvQueNode[i].isDONE()))
        {
            MY_LOGW("rvQueNode idx(%d) is not done",i);
            continue;
        }
        ImgBufQueNode const&    rQueNode    = rvQueNode[i];
        sp<IImgBuf>const&       rpQueImgBuf = rQueNode.getImgBuf();     //  ImgBuf in Queue.
        //
        if(rpQueImgBuf == 0)
        {
            MY_LOGW("i(%d):rpQueImgBuf is NULL",i);
            continue;
        }
        //
        {
            Mutex::Autolock _l(mModuleMtx);
            //
            for(j=0; j<mvRecBufInfo.size(); j++)
            {
                if(mvRecBufInfo[j].Sta == REC_BUF_STA_ENQUE)
                {
                    if(mvRecBufInfo[j].BufIonFd == rpQueImgBuf->getIonFd())
                    {
                        mvRecBufInfo.editItemAt(j).Sta = REC_BUF_STA_FILL;
                        //
                        if(mbMetaMode)
                        {
                            mpImgBufMgr->getBuf(j)->getGrallocBuffer()->unlock();
                        }
                        break;
                    }
                }
            }
        }
        //
        if(j == mvRecBufInfo.size())
        {
            MY_LOGE("Can't find FD(%d)/VA(%p)",
                    rpQueImgBuf->getIonFd(),
                    (MUINTPTR)(rpQueImgBuf->getVirAddr()));
            return  false;
        }
        //
        MY_LOGD_IF(
            0,
            "CB:i(%d/%d),Idx(%d),Sta(%d),Info(%p/%d.%06d)",
            i,
            queSize-1,
            j,
            rQueNode.getStatus(),
            (MUINTPTR)(rpQueImgBuf->getVirAddr()),
            (uint32_t)((rpQueImgBuf->getTimestamp()/1000)/1000000),
            (uint32_t)((rpQueImgBuf->getTimestamp()/1000)%1000000)
        );
        //
        if(mpExtImgProc != NULL)
        {
            if(mpExtImgProc->getImgMask() & ExtImgProc::BufType_Record)
            {
                IExtImgProc::ImgInfo img;
                //
                img.bufType     = ExtImgProc::BufType_Record;
                img.format      = rpQueImgBuf->getImgFormat();
                img.width       = rpQueImgBuf->getImgWidth();
                img.height      = rpQueImgBuf->getImgHeight();
                img.stride[0]   = rpQueImgBuf->getImgWidthStride(0);
                img.stride[1]   = rpQueImgBuf->getImgWidthStride(1);
                img.stride[2]   = rpQueImgBuf->getImgWidthStride(2);
                img.virtAddr    = (MUINTPTR)(rpQueImgBuf->getVirAddr());
                img.bufSize     = rpQueImgBuf->getBufSize();
                //
                mpExtImgProc->doImgProc(img);
            }
        }
        //
        //
        if(!performRecordCallback(j, mpImgBufMgr->getBuf(j), rQueNode.getCookieDE()))
        {
            Mutex::Autolock _l(mModuleMtx);
            mvRecBufInfo.editItemAt(j).Sta = REC_BUF_STA_EMPTY;
        }
    }
    //
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
performRecordCallback(int32_t bufIdx, sp<RecImgBuf>const& pCameraImgBuf, int32_t const msgType)
{
    nsecs_t timeDiff;
    //
    if  ( pCameraImgBuf != 0 )
    {
        //  [1] Dump image if wanted.
        if  ( 0 < mi4DumpImgBufCount )
        {
            MY_LOGD("<dump image> mi4DumpImgBufCount(%d) > 0", mi4DumpImgBufCount);
            Mutex::Autolock _lock(mDumpMtx);
            saveBufToFile(
                String8::format(
                    "%s_%s-(%d)%dx%d_%03d.yuv",
                    ms8DumpImgBufPath.string(),
                    pCameraImgBuf->getImgFormat().string(),
                    pCameraImgBuf->getImgWidthStride(),
                    pCameraImgBuf->getImgWidth(),
                    pCameraImgBuf->getImgHeight(),
                    mi4DumpImgBufIndex
                ),
                (uint8_t*)pCameraImgBuf->getVirAddr(),
                pCameraImgBuf->getBufSize()
            );
            ::android_atomic_dec(&mi4DumpImgBufCount);
            ::android_atomic_inc(&mi4DumpImgBufIndex);
        }

        if(!isMsgEnabled())
        {
            MY_LOGW_IF(ENABLE_LOG_PER_FRAME, "No REC CB: isEnabledState(%d), isMsgEnabled(%d)", isEnabledState(), isMsgEnabled());
            return false;
        }
        //
        if(mLastTimeStamp >= pCameraImgBuf->getTimestamp())
        {
            MY_LOGW("TimeStamp:Last(%d.%06d) >= Cur(%d.%06d)",
                    (uint32_t)((mLastTimeStamp/1000)/1000000),
                    (uint32_t)((mLastTimeStamp/1000)%1000000),
                    (uint32_t)((pCameraImgBuf->getTimestamp()/1000)/1000000),
                    (uint32_t)((pCameraImgBuf->getTimestamp()/1000)%1000000));
            return false;
        }
        mLastTimeStamp = pCameraImgBuf->getTimestamp();
        //  [2] Callback
        NSCam::Utils::CamProfile profile(__FUNCTION__, "RecordClient");
        ::android_atomic_inc(&mi4CallbackRefCount);
        mi8CallbackTimeInMs = getTimeInMs();
        //
        mFrameCount++;
        mTimeEnd = systemTime();
        timeDiff = mTimeEnd - mTimeStart;
        if(timeDiff > FPS_CNT_TIME)
        {
            MY_LOGD("FPS(%d)",mFrameCount);
            mFrameCount = 0;
            mTimeStart = mTimeEnd;
        }
        //
        {
            Mutex::Autolock _l(mModuleMtx);
            //
            mvRecBufInfo.editItemAt(bufIdx).Sta = REC_BUF_STA_CB;
        }
        //
        if(mbMetaMode)
        {
           // The following shows the layout of a metadata buffer,
           //* where buffer type is a 4-byte field of MetadataBufferType,
           //* and the payload is the metadata information.
           //*
           //* --------------------------------------------------------------
           //* |  buffer type  |          payload                           |
           //* --------------------------------------------------------------
           *((uint32_t*)pCameraImgBuf->get_camera_memory()->data) = kMetadataBufferTypeGrallocSource;
           *((buffer_handle_t*)(pCameraImgBuf->get_camera_memory()->data + sizeof(uint32_t))) = pCameraImgBuf->getGrallocBufferHandle();
        }
        //
        MY_LOGD("CB:FC(%02d),Idx(%d),VA(0x%08X),CbVA(0x%08X),GrallocHandle(0x%08X),TS(%d.%06d)",
                mFrameCount,
                bufIdx,
                pCameraImgBuf->getVirAddr(),
                pCameraImgBuf->get_camera_memory_virAddr(),
                pCameraImgBuf->getGrallocBufferHandle(),
                (uint32_t)((pCameraImgBuf->getTimestamp()/1000)/1000000),
                (uint32_t)((pCameraImgBuf->getTimestamp()/1000)%1000000));
        mpCamMsgCbInfo->mDataCbTimestamp(
            (nsecs_t)pCameraImgBuf->getTimestamp(),
            (int32_t)CAMERA_MSG_VIDEO_FRAME,
            pCameraImgBuf->get_camera_memory(),
            pCameraImgBuf->getBufIndex(),
            mpCamMsgCbInfo->mCbCookie
        );
        //MY_LOGD("CB done");
        //
        ::android_atomic_dec(&mi4CallbackRefCount);
        //profile.print_overtime(10, "mDataCb(%x) - index(%d)", msgType, pCameraImgBuf->getBufIndex());
        return true;
    }
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecordClient::
releaseRecordingFrame(const void *opaque)
{
    uint32_t i,size;
    //
    Mutex::Autolock _lock(mModuleMtx);
    //
    size = mvRecBufInfo.size();
    //
    for(i=0; i<size; i++)
    {
        if(mvRecBufInfo[i].Sta  == REC_BUF_STA_CB)
        {
            if(mvRecBufInfo[i].CbVirAddr == opaque)
            {
                if(mbMetaMode)
                {
                    MY_LOGD("Idx(%d),FD(%d),VA(%p),32byte(%d/0x%08X)",
                            i,
                            mvRecBufInfo[i].BufIonFd,
                            (MUINTPTR)opaque,
                            *(MUINTPTR*)opaque,
                            *(MUINTPTR*)(opaque + sizeof(uint32_t)));
                }
                else
                {
                    MY_LOGD("Idx(%d),FD(%d),VA(%p)",
                            i,
                            mvRecBufInfo[i].BufIonFd,
                            (MUINTPTR)opaque);
                }
                mvRecBufInfo.editItemAt(i).Sta = REC_BUF_STA_EMPTY;
                break;
            }
        }
    }
    //
    if(i == size)
    {
        MY_LOGE("Can't find VA(%p)",(MUINTPTR)opaque);
    }
}


