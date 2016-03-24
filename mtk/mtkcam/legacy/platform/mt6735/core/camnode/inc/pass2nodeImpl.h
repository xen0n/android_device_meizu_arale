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
#ifndef _MTK_CAMERA_CAMNODE_INCLUDE_PASS2NODEIMPL_H_
#define _MTK_CAMERA_CAMNODE_INCLUDE_PASS2NODEIMPL_H_
//
#include <semaphore.h>
using namespace std;
//
#include <utils/RefBase.h>
#include <mtkcam/v1/camutils/CamInfo.h>
#include <mtkcam/v1/IParamsManager.h>
using namespace android;
//
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
using namespace NSCam::NSIoPipe::NSPostProc;
#include <mtkcam/camnode/IFeaturePipe.h>
//
#include <mtkcam/common.h>
//
// for EPort
#include <mtkcam/imageio/ispio_pipe_ports.h>
//
#include <mtkcam/camnode/pass2node.h>
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

class SwNR;

namespace NSCamNode {

static const PortID IMGI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
static const PortID WDMAO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);
static const PortID WROTO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WROTO, 1);
static const PortID IMGO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGO, 1);
static const PortID IMG2O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG2O, 1);
static const PortID IMG3O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3O, 1);
static const PortID VENC( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_VENC_STREAMO, 1);

/*******************************************************************************
*
********************************************************************************/
#define MULTI_FRAME_ENABLE      (1)
#define PASS2_CALLBACL_ENABLE   (1)
/*******************************************************************************
*
********************************************************************************/
class Pass2NodeImpl : public Pass2Node
{
    public: // ctor & dtor

        Pass2NodeImpl(Pass2NodeType const type);
        virtual ~Pass2NodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

        // for capture only: type <0:none, 1:hwnr, 2:swnr>
        virtual MBOOL  setNRType(MUINT32 const type) {return MFALSE;};

    protected: // operations

        #define MAX_DST_PORT_NUM        (4)
        #define MULTI_FRAME_NUM_MAX     (4)

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;
        }PostBufInfo;

        MBOOL getDstBuffer(
                MUINT32         nodeData,
                ImgRequest*     pImgReq);

        typedef struct
        {
            PostBufInfo           src;
            vector<ImgRequest>    vDstReq;
            vector<MUINT32>       vDstData;
            MSize                 dstSize;
            MBOOL                 doCrop;
        }p2data;

        virtual MBOOL  getPass2Buffer(vector<p2data>& vP2data) = 0;
        MBOOL   enquePass2(MBOOL const doCallback);
        MBOOL   dequePass2();


        // use callback
        MBOOL   pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);

        // template methods
        virtual MBOOL   moduleInit() { return MTRUE; }
        virtual MBOOL   moduleStop() { return MTRUE; }
        virtual MBOOL   moduleStart() { return MTRUE; }
        virtual MBOOL   isReadyToEnque() const { return MTRUE; }
        virtual MBOOL   handleP2Done(QParams& rParams);
        virtual MBOOL   isPreviewPath() const { return MTRUE; }
        // for feature settings
        virtual void    configFeature() {}

    protected:

        // callback function
        static MVOID   pass2CbFunc(QParams& rParams);

        PortID   mapToPortID(MUINT32 const nodeDataType);
        MUINT32  mapToNodeDataType(PortID const portId);

    protected:
        //     init config
        Pass2NodeType const     mPass2Type;
        IHalPostProcPipe*       mpPostProcPipe;
        IspSyncControlHw*       mpIspSyncCtrlHw;

        // === mLock protect following ===
        mutable Mutex           mLock;
        MUINT32                 muPostFrameCnt;
        MUINT32                 muEnqFrameCnt;
        MUINT32                 muDeqFrameCnt;
        Condition               mCondEnque;
        Condition               mCondDeque;
        list<PostBufInfo>       mlPostBufData;

        //workaround for start/stop recording
        mutable Mutex           mRecordingLock;

        // dst deque order, should be overwriten by subclass
        MUINT32                 muDequeOrder[MAX_DST_PORT_NUM];
};


class PrvPass2 : public Pass2NodeImpl
{
    public: // ctor & dtor

        static PrvPass2* createInstance(Pass2NodeType const type);

        PrvPass2(Pass2NodeType const type);
        virtual ~PrvPass2();

        virtual MBOOL   startPreview(
                            MINT32  width,
                            MINT32  height,
                            MINT32  fps);
        virtual MBOOL   stopPreview();
        virtual MBOOL   startRecording(
                            MUINT32 width,
                            MUINT32 height,
                            MUINT32 fps,
                            MBOOL   isYuv);
        virtual MBOOL   stopRecording();

    protected:

        virtual MBOOL   threadInit();
        virtual MBOOL   threadUninit();
        virtual MBOOL   threadLoopUpdate();
        virtual MBOOL   moduleInit();
        virtual MBOOL   isReadyToEnque() const;
        //
        virtual IHalPostProcPipe* createPipe();
        //
        virtual MBOOL  getPass2Buffer(vector<p2data>& vP2data);

        // optimization for featurepipe
        virtual MBOOL   doInitialization() { return MFALSE; }

    protected:

        MBOOL                   mbRecording;
        MBOOL                   mbPreview;
        MBOOL                   mbRecBufYuv;
        MSize                   mVdoSize;
        MUINT32                 muMultiFrameNum;
};


class FeaturePass2 : public PrvPass2
{
    public:

        FeaturePass2(Pass2NodeType const type);
        ~FeaturePass2() {};

        virtual MBOOL enableFeature(MBOOL en);
        virtual MBOOL enableGDPreview(MBOOL en);

        virtual void setParameters(
                        sp<IParamsManager>  spParamsMgr,
                        MUINT32             openId);
        virtual void setCamMsgCb(sp<CamMsgCbInfo> spCamMsgCbInfo);

    protected:

        #define IMG_1080P_W             (1920)
        #define IMG_1080P_H             (1088)
        #define IMG_1080P_SIZE          (IMG_1080P_W*IMG_1080P_H)

        virtual IHalPostProcPipe* createPipe();
        virtual void configFeature();

        virtual MBOOL moduleStart();
        // optimization for featurepipe
        virtual MBOOL   doInitialization();

    private:
        IFeaturePipe*                           mpFeaturePipe;

        mutable Mutex                           mLock;
        sp<CamMsgCbInfo>                        mspCamMsgCbInfo;
        FeaturePipeTuningInfo                   mFeaturePipeTuning;
        IParamsManager::FACE_BEAUTY_POS         mTouch;
        MSize                                   mVdoSizeBeforePreview;
        MUINT32                                 mFeatureMask;
        MUINT32                                 mSensorType;
        MBOOL                                   mbFeature;
        MBOOL                                   mbGDPreview;
        MBOOL                                   mbRecordingHint;

        // optimization for startPreview featurepipe initialization
        MBOOL                                   mbInit;
};


class CapPass2 : public Pass2NodeImpl
{
    public:

        CapPass2(Pass2NodeType const type);
        virtual ~CapPass2();

        virtual MBOOL  setNRType(MUINT32 const type) { muNRType = type; return MTRUE; };

    protected:

        virtual MBOOL   moduleInit();
        virtual MBOOL   moduleStop();
        // pure-raw utility
        MBOOL pureRawPass(IImageBuffer* src, IImageBuffer* dst);

        MBOOL transform(
                IImageBuffer* src,
                IImageBuffer* dst0,
                IImageBuffer* dst1,
                MRect const crop,
                MINT32 const transform1);

    protected:

        virtual MBOOL threadLoopUpdate();
        virtual MBOOL getPass2Buffer(vector<p2data>& vP2data);
        virtual MBOOL handleP2Done(QParams& rParams);
        virtual MBOOL isPreviewPath() const { return MFALSE; }

        MBOOL createFakeResultMetadata(IImageBuffer* srcbuf, IImageBuffer* dstbuf);
        MBOOL destroyFakeResultMetadata(IImageBuffer* dstbuf);
        MBOOL allocMemory(IImageBuffer* srcbuf, MINT32 const fmt, sp<IImageBuffer>& targetBuf);
        MBOOL freeMemory(sp<IImageBuffer>& targetBuf);

        MVOID dumpBuffer(IImageBuffer* pBuf, const char* filename, const char* fileext);

    private:

        MUINT32                 muNRType;
        //internal memory
        sp<IImageBuffer>        mpInternalNR1Buf;
        sp<IImageBuffer>        mpInternalNR2Buf;
        sp<IImageBuffer>        mpInternalPurerawBuf;
        vector<ImgRequest>      mvOutRequest;
        vector<MUINT32>         mvOutData;
        MRect                   mCapCrop;

        SwNR*                   mpSwNR;

        MUINT32                 muDumpFlag;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //  _MTK_CAMERA_CAMNODE_INCLUDE_PASS2NODEIMPL_H_

