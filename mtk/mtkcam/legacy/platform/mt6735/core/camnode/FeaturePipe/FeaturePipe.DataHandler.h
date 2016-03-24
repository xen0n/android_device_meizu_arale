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

#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_FEATUREPIPEDATABUFHANDLER_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_FEATUREPIPEDATABUFHANDLER_H_

#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
using namespace android;
#include <semaphore.h>

#include <vector>
#include <queue>
using namespace std;

#include <mtkcam/camnode/ICamBufHandler.h>
#include <ui/GraphicBuffer.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

class FeaturePipeDataHandler
{
    public:

        enum {
            AllocType,
            RegisterType,
        };

    public:

        FeaturePipeDataHandler(MUINT32 const type, MUINT32 const data);
        ~FeaturePipeDataHandler();

        MBOOL   init();
        MBOOL   uninit();

        MBOOL   allocBuffer(
                AllocInfo const & info
                );

        MBOOL   registerBuffer(
                IImageBuffer const * pImageBuffer
                );

        MBOOL   getBuffer(
                ImgRequest * pImgReq
                );

        MBOOL   returnBuffer(
                IImageBuffer const * pImageBuffer
                );

        MBOOL   updateRequest(
                MINT32 const transform,
                MINT32 const usage
                );

        MUINT32 getType() const { return mType; };

        MBOOL   resizeBuffer(
                IImageBuffer const ** pImageBuffer,
                MUINT32 width,
                MUINT32 height
                );

        MBOOL   allocBuffer_GB(
                AllocInfo const & info,
                sp<GraphicBuffer>* pGraphicBuffer
                );

        MBOOL   getGraphicBuffer(
                IImageBuffer const * pImageBuffer,
                sp<GraphicBuffer>** pGraphicBuffer);

    public:

        const MUINT32                         mType;
        const MUINT32                         mData;
        MINT32                                mTransform;
        MINT32                                mUsage;
        mutable Mutex                         mLock;
        sem_t                                 msemBuf;

    public:
        IMemDrv*                              mpIMemDrv;

        typedef enum
        {
            IMEM_TYPE,
            GRAPHICBUFFER_TYPE,
        } ImageBufferMapType;
        typedef struct
        {
            ImageBufferMapType type;
            IImageBuffer* pImgBuf;
            sp<GraphicBuffer>* pGB;
            IMEM_BUF_INFO memBuf;
            MINT usage;
        } ImageBufferMap;

        vector<ImageBufferMap>                mvDequeBuf;
        queue<ImageBufferMap>                 mqAvailableBuf;
};


/*******************************************************************************
*
********************************************************************************/

}; // namespace NSCamNode

#endif  //_MTK_CAMERA_INCLUDE_CAMNODE_FEATUREPIPEDATABUFHANDLER_H_
