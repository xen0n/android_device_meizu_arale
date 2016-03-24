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
#ifndef _MTK_CAMERA_CAMNODE_INCLUDE_STEREONODEIMPL_H_
#define _MTK_CAMERA_CAMNODE_INCLUDE_STEREONODEIMPL_H_
//
#include <semaphore.h>
using namespace std;
//
#include <utils/RefBase.h>
#include <mtkcam/v1/camutils/CamInfo.h>
#include <mtkcam/v1/IParamsManager.h>
using namespace android;
//
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
using namespace NSCam::NSIoPipe::NSPostProc;
#include <mtkcam/camnode/IFeaturePipe.h>
//
// for EPort
#include <mtkcam/imageio/ispio_pipe_ports.h>
//
#include <mtkcam/camnode/stereonode.h>
#include "./inc/IspSyncControlHw.h"
/*******************************************************************************
*
********************************************************************************/
//
#include <aee.h>
#define AEE_ASSERT(String)    \
    do {                      \
        aee_system_exception( \
            LOG_TAG,          \
            NULL,             \
            DB_OPT_DEFAULT,   \
            String);          \
    } while(0)

/*******************************************************************************
*
********************************************************************************/

namespace NSCamNode {

static const PortID IMGI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
static const PortID WDMAO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);
static const PortID IMG3O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3O, 1);
static const PortID FEO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_FEO, 1);

/*******************************************************************************
*
********************************************************************************/
typedef struct HwPortConfig{
    PortID           mPortID;
    EImageFormat     mFmt;
    MSize            mSize;
    MRect            mCrop;
} HwPortConfig_t;


/*******************************************************************************
*
********************************************************************************/
class StereoNodeImpl : public StereoNode
{
    public: // ctor & dtor

        StereoNodeImpl(StereoNodeType const type);
        ~StereoNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public:

        MBOOL               allocBuffers(vector<HwPortConfig_t> &  plPortCfg);

        MBOOL               freeBuffers();

        MBOOL               isReadyToEnque() const { return MTRUE; }

        MBOOL               enquePass2(
                                MUINT32 const data,
                                IImageBuffer* const buf,
                                MUINT32 const ext);
        // use callback
        MBOOL               pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);

        MBOOL               handleP2Done(QParams& rParams);


    protected: // operations

        MUINT32             getHWFEBlock() const            { return mHwFeBlock; }

        MSize const&        getHWFESize() const         { return mHwFeSize; }

        MSize const&        getFEImgSize() const            { return mFeImgSize; }

        MSize const&        getRgbImgSize() const           { return mRgbSize; }

        MSize const&        getAlgoImgSize() const            { return mAlgoImgSize; }


        #define MAX_DST_PORT_NUM        (3)

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;
        }PostBufInfo;

        // callback function
        static  MVOID       pass2CbFunc(QParams& rParams);

        PortID              mapToPortID(MUINT32 const nodeDataType);
        MUINT32             mapToNodeDataType(PortID const portId);

        // ************ buffer control ************ //
        //
        MBOOL           setupPort(MBOOL& cfgImgo, MBOOL& cfgFeo, MBOOL& cfgRgb);
        // **************************************** //

    private:
        //     counter
        MUINT32                                 muFrameCnt;
        //     infos
        MBOOL                                   mbCfgImgo;
        MBOOL                                   mbCfgFeo;
        MBOOL                                   mbCfgRgb;
        //
        MSize                                   mAlgoImgSize;
        MSize                                   mRgbSize;
        MSize                                   mFeImgSize;
        MSize                                   mHwFeSize;
        MUINT32                                 mHwFeBlock;
        //
        StereoHalBase*                          mpStereoHal;
        //
        // ************ buffer control ************ //
        list<IMEM_BUF_INFO>                     mlFeBufQueue;
        //
        IHalPostProcPipe*                       mpPostProcPipe;
        IspSyncControlHw*                       mpIspSyncCtrlHw;

#define BUF_COUNT   5
        StaData*                                mpStaParam[BUF_COUNT];
        SrzSize*                                mpSrzParam[BUF_COUNT];
        //
        // === mLock protect following ===
        mutable Mutex           mLock;
        MUINT32                 muPostFrameCnt;
        MUINT32                 muEnqFrameCnt;
        MUINT32                 muDeqFrameCnt;
        Condition               mCondEnque;
        Condition               mCondDeque;
        list<PostBufInfo>       mlPostBufData;
        //
        MBOOL                   mDebugDumpGB;
};


class PrvStereo : public StereoNodeImpl
{
    public: // ctor & dtor

        static PrvStereo* createInstance(StereoNodeType const type);

        PrvStereo(StereoNodeType const type);
        virtual ~PrvStereo();


    protected:


    protected:

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //  _MTK_CAMERA_CAMNODE_INCLUDE_STEREONODEIMPL_H_

