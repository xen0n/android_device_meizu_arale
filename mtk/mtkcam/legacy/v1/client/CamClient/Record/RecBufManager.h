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

#ifndef _MTK_HAL_CAMCLIENT_RECORD_RECBUFMANAGER_H_
#define _MTK_HAL_CAMCLIENT_RECORD_RECBUFMANAGER_H_
//
#include <ion/ion.h>
#include <linux/ion.h>
//
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferExtra.h>
#include <ui/gralloc_extra.h>

namespace android {
namespace NSCamClient {
namespace NSRecordClient {

/******************************************************************************
*   Image Info
*******************************************************************************/
struct RecImgInfo : public LightRefBase<RecImgInfo>
{
    String8                         ms8ImgName;
    bool                            mbMetaMode;
    String8                         ms8ImgFormat;
    uint32_t                        mu4BufUsage;
    uint32_t                        mu4ImgFormat;
    uint32_t                        mu4ImgWidth;
    uint32_t                        mu4ImgHeight;
    uint32_t                        mu4ImgStrideY;
    uint32_t                        mu4ImgStrideU;
    uint32_t                        mu4ImgStrideV;
    uint32_t                        mu4BitsPerPixel;
    size_t                          mImgBufSize;
    int32_t                         mi4BufSecu;
    int32_t                         mi4BufCohe;
    //
                                    RecImgInfo(
                                        bool const          bMetaMode,
                                        uint32_t const      u4BufUsage,
                                        uint32_t const      u4ImgFormat,
                                        uint32_t const      u4ImgWidth,
                                        uint32_t const      u4ImgHeight,
                                        uint32_t const      u4ImgStrideY,
                                        uint32_t const      u4ImgStrideU,
                                        uint32_t const      u4ImgStrideV,
                                        char const*const    ImgFormat,
                                        char const*const    pImgName,
                                        int32_t const       i4BufSecu,
                                        int32_t const       i4BufCohe
                                    )
                                        : ms8ImgName(pImgName)
                                        , mbMetaMode(bMetaMode)
                                        , ms8ImgFormat(ImgFormat)
                                        , mu4BufUsage(u4BufUsage)
                                        , mu4ImgFormat(u4ImgFormat)
                                        , mu4ImgWidth(u4ImgWidth)
                                        , mu4ImgHeight(u4ImgHeight)
                                        , mu4ImgStrideY(u4ImgStrideY)
                                        , mu4ImgStrideU(u4ImgStrideU)
                                        , mu4ImgStrideV(u4ImgStrideV)
                                        , mi4BufSecu(i4BufSecu)
                                        , mi4BufCohe(i4BufCohe)
                                    {
                                        if(mbMetaMode)
                                        {
                                            mu4BitsPerPixel = 0;
                                            mImgBufSize = 0;
                                        }
                                        else
                                        {
                                            mu4BitsPerPixel = NSCam::Utils::Format::queryImageBitsPerPixel(NSCam::Utils::Format::queryImageFormat(ms8ImgFormat.string()));
                                            //Y size
                                            if(mu4ImgWidth % u4ImgStrideY)
                                            {
                                                mImgBufSize = (mu4ImgWidth/u4ImgStrideY+1)*u4ImgStrideY*mu4ImgHeight;
                                            }
                                            else
                                            {
                                                mImgBufSize = mu4ImgWidth*mu4ImgHeight;
                                            }
                                            //U size
                                            if((mu4ImgWidth/2) % mu4ImgStrideU)
                                            {
                                                mImgBufSize += ((mu4ImgWidth/2)/mu4ImgStrideU+1)*mu4ImgStrideU*(mu4ImgHeight/2);
                                            }
                                            else
                                            {
                                                mImgBufSize += (mu4ImgWidth/2)*(mu4ImgHeight/2);
                                            }
                                            //V size
                                            if((mu4ImgWidth/2) % mu4ImgStrideV)
                                            {
                                                mImgBufSize += ((mu4ImgWidth/2)/mu4ImgStrideV+1)*mu4ImgStrideV*(mu4ImgHeight/2);
                                            }
                                            else
                                            {
                                                mImgBufSize += (mu4ImgWidth/2)*(mu4ImgHeight/2);
                                            }
                                            //
                                            CAM_LOGD(
                                                "[RecImgInfo::RecImgInfo] [%s](%s@%dx%d@%d-bit@%d)",
                                                ms8ImgName.string(),
                                                ms8ImgFormat.string(),
                                                mu4ImgWidth,
                                                mu4ImgHeight,
                                                mu4BitsPerPixel,
                                                mImgBufSize
                                            );
                                        }
                                    }
};


/******************************************************************************
*   image buffer for record callback
*******************************************************************************/
class RecImgBuf : public ICameraImgBuf
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IMemBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual int64_t                 getTimestamp() const                    { return mi8Timestamp; }
    virtual void                    setTimestamp(int64_t const timestamp)   { mi8Timestamp = timestamp; }
    //
public:     ////                    Attributes.
    virtual const char*             getBufName() const                      { return mpImgInfo->ms8ImgName.string(); }
    virtual size_t                  getBufSize() const                      { return mpImgInfo->mImgBufSize; }
    //
    virtual void                    setVirAddr(void *va)                    { mpBufVA = va; }
    virtual void*                   getVirAddr() const
                                    {
                                        if(mpImgInfo->mbMetaMode)
                                        {
                                            return mpBufVA;
                                        }
                                        else
                                        {
                                            return mCamMem.data;
                                        }
                                    }
    virtual void*                   getPhyAddr() const
{
                                        if(mpImgInfo->mbMetaMode)
                                        {
                                            return mpBufPA;
                                        }
                                        else
                                        {
                                            return NULL;
                                        }
                                    }
    virtual int                     getIonFd() const
                                    {
                                        if(mpImgInfo->mbMetaMode)
                                        {
                                            return mi4BufIonFd;
                                        }
                                        else
                                        {
                                            return mIonBufFd;
                                        }
                                    }
    virtual int                     getBufSecu() const                      { return mpImgInfo->mi4BufSecu; }
    virtual int                     getBufCohe() const                      { return mpImgInfo->mi4BufCohe; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImgBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual String8 const&          getImgFormat()      const               { return mpImgInfo->ms8ImgFormat; }
    virtual uint32_t                getImgWidth()       const               { return mpImgInfo->mu4ImgWidth;  }
    virtual uint32_t                getImgHeight()      const               { return mpImgInfo->mu4ImgHeight; }
    virtual uint32_t                getImgWidthStride(
                                        uint_t const uPlaneIndex = 0
                                    )  const
                                    {
                                        if(mpImgInfo->mbMetaMode)
                                        {
                                            switch(uPlaneIndex)
                                            {
                                                case 0:
                                                {
                                                    return mpImgInfo->mu4ImgStrideY;
                                                }
                                                case 1:
                                                {
                                                    return mpImgInfo->mu4ImgStrideU;
                                                }
                                                case 2:
                                                {
                                                    return mpImgInfo->mu4ImgStrideV;
                                                }
                                                default:
                                                {
                                                    CAM_LOGE("[RecImgBuf]error uPlaneIndex(%d)",uPlaneIndex);
                                                    return 0;
                                                }
                                            }
                                        }
                                        else
                                        {
                                          uint32_t width = getImgWidth();
                                          uint32_t stride = getImgWidth();
                                          uint32_t strideUnit = mpImgInfo->mu4ImgStrideY;
                                          //
                                          switch(uPlaneIndex)
                                          {
                                              case 0:
                                              {
                                                  //Do nothing.
                                                  break;
                                              }
                                              case 1:
                                              {
                                                  width /= 2;
                                                  stride = width;
                                                  strideUnit = mpImgInfo->mu4ImgStrideU;
                                                  break;
                                              }
                                              case 2:
                                              {
                                                  width /= 2;
                                                  stride = width;
                                                  strideUnit = mpImgInfo->mu4ImgStrideV;
                                                  break;
                                              }
                                              default:
                                              {
                                                  CAM_LOGE("[RecImgBuf]error uPlaneIndex(%d)",uPlaneIndex);
                                                  return 0;
                                              }
                                          }
                                          //
                                          if(width%strideUnit != 0)
                                          {
                                              stride = ((width/strideUnit+1)*strideUnit);
                                          }
                                          return stride;
                                        }
                                    }
    virtual uint32_t                getBitsPerPixel()   const               { return mpImgInfo->mu4BitsPerPixel; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual uint_t                  getBufIndex() const                     { return 0; }
    virtual camera_memory_t*        get_camera_memory()                     { return &mCamMem; }
    virtual void*                   get_camera_memory_virAddr()             { return mCamMem.data; }
    //add gralloc interface
    virtual sp<GraphicBuffer>&      getGrallocBuffer()                      { return mpGraphicBuffer; }
    buffer_handle_t                 getGrallocBufferHandle()
                                    {
                                        if(mpGraphicBuffer != NULL)
                                        {
                                            return mpGraphicBuffer->handle;
                                        }
                                        else
                                        {
                                            return 0;
                                        }
                                    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static RecImgBuf*               alloc(
                                        camera_request_memory   requestMemory,
                                        sp<RecImgInfo>&         rpImgInfo
                                    );

public:     ////                    Instantiation.
                                    RecImgBuf(
                                        camera_memory_t const&      rCamMem,
                                        sp<RecImgInfo>&             rpImgInfo,
                                        int32_t                     IonDevFd,
                                        int32_t                     IonBufFd,
                                        ion_user_handle_t           IonHandle,
                                        sp<GraphicBuffer>           spGraphicBuffer
                                    );
    virtual                         ~RecImgBuf();

public:     ////                    Debug.
    void                            dump() const;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Memory.
    sp<RecImgInfo>          mpImgInfo;
    int64_t                 mi8Timestamp;
    //
    camera_memory_t         mCamMem;
    //
    int32_t                 mIonDevFd;
    int32_t                 mIonBufFd;
    ion_user_handle_t       mIonHandle;
    //
    sp<GraphicBuffer>       mpGraphicBuffer;
    //
    uint32_t                mu4BufUsage;
    int32_t                 mi4BufIonFd;
    void*                   mpBufVA;
    void*                   mpBufPA;
};


/******************************************************************************
*
*******************************************************************************/
class RecBufManager : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static RecBufManager*           alloc(
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
                                    );

public:     ////                    Attributes.
    //
    virtual char const*             getName() const             { return ms8Name.string(); }
    sp<RecImgBuf>const&             getBuf(size_t index) const  { return mvImgBuf[index]; }
    int32_t                         getBufIonFd(size_t index) const  { return mvImgBufIonFd[index]; }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Instantiation.
    virtual                         ~RecBufManager();

protected:  ////                    Instantiation.
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
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    //
    bool                            init();
    void                            uninit();

protected:  ////                    Data Members.
    //
    bool                            mbMetaMode;
    String8                         ms8Name;
    String8                         ms8ImgFormat;
    uint32_t                        mu4BufUsage;
    uint32_t                        mu4ImgFormat;
    uint32_t                        mu4ImgWidth;
    uint32_t                        mu4ImgHeight;
    uint32_t                        mu4ImgStrideY;
    uint32_t                        mu4ImgStrideU;
    uint32_t                        mu4ImgStrideV;
    uint32_t                        mu4BufCount;
    //
    Vector< sp<RecImgBuf> >         mvImgBuf;
    camera_request_memory           mRequestMemory;
    Vector<int32_t>                 mvImgBufIonFd;
    //
    int32_t                         mi4BufSecu;
    int32_t                         mi4BufCohe;
};


}; // namespace NSRecordClient
}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_HAL_CAMCLIENT_RECORD_RECBUFMANAGER_H_

