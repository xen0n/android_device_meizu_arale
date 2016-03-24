/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_ICAMBUFHANDLER_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_ICAMBUFHANDLER_H_
//
//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>

#include <mtkcam/ImageFormat.h>
using namespace NSCam;

#include <mtkcam/camnode/nodeDataTypes.h>

/******************************************************************************
*
*******************************************************************************/


namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
#define DECLARE_ICAMBUFHANDLER_INTERFACE()                      \
public:     /* Operations. */                                   \
    virtual MBOOL   init();                                     \
    virtual MBOOL   uninit();                                   \
    virtual MBOOL   requestBuffer(                              \
                        MUINT32 const data,                     \
                        AllocInfo const & info                  \
                        );                                      \
                                                                \
    virtual MBOOL   dequeBuffer(                                \
                        MUINT32 const data,                     \
                        ImgRequest * pImgReq                    \
                        );                                      \
                                                                \
    virtual MBOOL   enqueBuffer(                                \
                        MUINT32 const data,                     \
                        IImageBuffer const * pImageBuffer       \
                        );                                      \
                                                                \
    virtual MBOOL   updateRequest(                              \
                        MUINT32 const data,                     \
                        MINT32 const transform,                 \
                        MINT32 const usage                      \
                        );

class AllocInfo
{
    public:
        MUINT32 w;
        MUINT32 h;
        MUINT32 fmt;
        MINT    usage; //IImageBuffer usage: SW R/w
        MUINT32 strideInByte[3];
        MBOOL   isGralloc;
        //to be added
    public:
        AllocInfo(
                MUINT32 const _w,
                MUINT32 const _h,
                MUINT32 const _fmt,
                MINT    const _usage = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN,
                MBOOL   const _gralloc = MFALSE)
            : w(_w)
            , h(_h)
            , fmt(_fmt)
            , usage(_usage)
            , isGralloc(_gralloc)
        {
            for (MUINT32 i = 0; i < 3; i++)
            {
                strideInByte[i] = 0;

            }
        }

        AllocInfo(  MUINT32 const _w,
                MUINT32 const _h,
                MUINT32 const _fmt,
                MINT    const _usage,
                MUINT32 const _strideInByte[],
                MBOOL   const _gralloc = MFALSE )
            : w(_w)
            , h(_h)
            , fmt(_fmt)
            , usage(_usage)
            , isGralloc(_gralloc)
        {
            for (MUINT32 i = 0; i < 3; i++)
            {
                strideInByte[i] = _strideInByte[i];
            }
        }
};

class ImgRequest
{
    public:
        IImageBuffer const* mBuffer;
        MINT32              mTransform;
        MINT32              mUsage;

        ImgRequest(
                IImageBuffer* pBuf     = 0,
                MINT32 const transform = 0,
                MINT32 const usage     = 0
                )
            : mBuffer(pBuf)
            , mTransform(transform)
            , mUsage(usage)
        {
        }
};

enum
{
    FLAG_BUFFER_REUSE   = 0x1,
    FLAG_BUFFER_ONESHOT = 0x2,
};

class ICamBufHandler
{
    protected:  ////    Constructor/Destructor.
        ICamBufHandler(char const* name) : msName(name) {}
        virtual         ~ICamBufHandler() {}

    public:     ////    Instantiation.
        virtual MVOID   destroyInstance()                   = 0;

    public:     ////    Operations.
        virtual MBOOL   init()                              = 0;
        virtual MBOOL   uninit()                            = 0;

        virtual MBOOL   requestBuffer(
                    MUINT32 const data,
                    AllocInfo const & info
                )                                           = 0;

        virtual MBOOL   dequeBuffer(
                    MUINT32 const data,
                    ImgRequest * pImgReq
                )                                           = 0;

        virtual MBOOL   enqueBuffer(
                    MUINT32 const data,
                    IImageBuffer const * pImageBuffer
                )                                           = 0;

        virtual MBOOL   updateRequest(
                    MUINT32 const data,
                    MINT32 const transform,
                    MINT32 const usage
                )                                           = 0;

        virtual MBOOL   updateFlag(
                    MUINT32 const data,
                    MUINT32 const flag
                ) { return MFALSE; }

        virtual MBOOL   isRegistered(
                    MUINT32 const data
                ) { return MFALSE; }

        // [workaround] For registered buffer, to check if buffer still exist.
        virtual MBOOL   isBufferExisted(
                    MUINT32 const data,
                    IImageBuffer const * pImageBuffer
                ) { return MFALSE; }

    public:     ////    Info

        const char* getName() const { return msName; }

    private:

        const char* const       msName;
};

}; // namespace NSCamNode
#endif  //_MTK_CAMERA_INCLUDE_CAMNODE_ICAMBUFHANDLER_H_

