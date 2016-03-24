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

#ifndef _MTK_CAMERA_CLIENT_CAMCLIENT_PREVIEWCALLBACK_IMGBUFMANAGER_H_
#define _MTK_CAMERA_CLIENT_CAMCLIENT_PREVIEWCALLBACK_IMGBUFMANAGER_H_
//


namespace android {
namespace NSCamClient {
namespace NSPrvCbClient {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*   Image Info
*******************************************************************************/
struct ImgInfo : public LightRefBase<ImgInfo>
{
public:     ////                    Data Members.
    String8                         ms8ImgName;
    String8                         ms8ImgFormat;
    int32_t                         mi4ImgFormat;
    uint32_t                        mu4ImgWidth;
    uint32_t                        mu4ImgHeight;
    uint32_t                        mu4BitsPerPixel;
    size_t                          mImgBufSize;
    int32_t                         mi4BufSecu;
    int32_t                         mi4BufCohe;
    //
public:     ////                    Operations.
                                    ImgInfo(
                                        uint32_t const u4ImgWidth,
                                        uint32_t const u4ImgHeight,
                                        int32_t const  i4ImgFormat,
                                        char const*const ImgFormat,
                                        char const*const pImgName,
                                        int32_t const  i4BufSecu,
                                        int32_t const  i4BufCohe
                                    );
    uint32_t                        getBufStridesInBytes(MUINT index)   const;
    uint32_t                        getBufStridesInPixels(MUINT index)  const;

};


/******************************************************************************
*   image buffer for preview callback
*******************************************************************************/
class PrvCbImgBuf : public ICameraImgBuf
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
    virtual void*                   getVirAddr() const                      { return mCamMem.data; }
    virtual void*                   getPhyAddr() const                      { return mCamMem.data; }
    virtual int                     getIonFd() const                        { return mIonBufFd; }
    virtual int                     getBufSecu() const                      { return mi4BufSecu; }
    virtual int                     getBufCohe() const                      { return mi4BufCohe; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImgBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual String8 const&          getImgFormat()      const               { return mpImgInfo->ms8ImgFormat; }
    virtual uint32_t                getImgWidth()       const               { return mpImgInfo->mu4ImgWidth;  }
    virtual uint32_t                getImgHeight()      const               { return mpImgInfo->mu4ImgHeight; }
    virtual uint32_t                getImgWidthStride(
                                        uint_t const uPlaneIndex
                                    )  const                                { return mpImgInfo->getBufStridesInPixels(uPlaneIndex); }
    virtual uint32_t                getBitsPerPixel()   const               { return mpImgInfo->mu4BitsPerPixel; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual uint_t                  getBufIndex() const                     { return 0; }
    virtual camera_memory_t*        get_camera_memory()                     { return &mCamMem; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static PrvCbImgBuf*             alloc(
                                        camera_request_memory   requestMemory,
                                        sp<ImgInfo const>const& rpImgInfo
                                    );

public:     ////                    Instantiation.
                                    PrvCbImgBuf(
                                        camera_memory_t const&      rCamMem,
                                        sp<ImgInfo const>const&     rpImgInfo,
                                        int32_t                     IonDevFd,
                                        int32_t                     IonBufFd,
                                        int32_t                     pIonHandle,
                                        int32_t                     bufSecu,
                                        int32_t                     bufCohe
                                    );
    virtual                         ~PrvCbImgBuf();

public:     ////                    Debug.
    void                            dump() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Memory.
    sp<ImgInfo const>       mpImgInfo;
    int64_t                 mi8Timestamp;
    camera_memory_t         mCamMem;
    //
    int32_t                 mIonDevFd;
    int32_t                 mIonBufFd;
    int32_t                 mpIonHandle;
    //
    int32_t                 mi4BufSecu;
    int32_t                 mi4BufCohe;
};


/******************************************************************************
*
*******************************************************************************/
class ImgBufManager : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static ImgBufManager*           alloc(
                                        char const*const        szImgFormat,
                                        uint32_t const          u4ImgWidth,
                                        uint32_t const          u4ImgHeight,
                                        uint32_t const          u4BufCount,
                                        char const*const        szName,
                                        camera_request_memory   requestMemory,
                                        int32_t const           i4BufSecu,
                                        int32_t const           i4BufCohe
                                    );

public:     ////                    Attributes.
    //
    virtual char const*             getName() const             { return ms8Name.string(); }
    sp<ICameraImgBuf>const&         getBuf(size_t index) const  { return mvImgBuf[index]; }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Instantiation.
    virtual                         ~ImgBufManager();

protected:  ////                    Instantiation.
                                    ImgBufManager(
                                        char const*const        szImgFormat,
                                        uint32_t const          u4ImgWidth,
                                        uint32_t const          u4ImgHeight,
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
    String8                         ms8Name;
    String8                         ms8ImgFormat;
    int32_t                         mi4ImgFormat;
    uint32_t                        mu4ImgWidth;
    uint32_t                        mu4ImgHeight;
    uint32_t                        mu4BufCount;
    //
    Vector< sp<ICameraImgBuf> >     mvImgBuf;
    camera_request_memory           mRequestMemory;
    //
    int32_t                         mi4BufSecu;
    int32_t                         mi4BufCohe;
};


}; // namespace NSPrvCbClient
}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_CAMERA_CLIENT_CAMCLIENT_PREVIEWCALLBACK_IMGBUFMANAGER_H_

