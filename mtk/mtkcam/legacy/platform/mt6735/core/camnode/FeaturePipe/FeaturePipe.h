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

#ifndef _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_H_
#define _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_H_
//

#include "HalPipeWrapper.h"
#include <IFeaturePipe.h>
#include <AllocBufHandler.h>
#include <ICamGraphNode.h>
#include <utils/threads.h>

#define USING_MTK_PERFSERVICE

using namespace NSImageio;
using namespace NSIspio;
namespace NSImageio{
namespace NSIspio{
    class IPostProcPipe;
};
};
using namespace NSCamNode;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {

class FeaturePipeNode;

/******************************************************************************
 *
 * @class INormalStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details
 * The data path will be Mem --> ISP--XDP --> Mem.
 *
 ******************************************************************************/
class FeaturePipe : public IFeaturePipe
{
    friend  class IFeaturePipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    /**
     * @brief destroy the pipe instance
     *
     * @details
     *
     * @note
     */
    virtual MVOID                   destroyInstance(char const* szCallerName);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
                                    FeaturePipe(MUINT32 openedSensorIndex);
public:
    virtual                         ~FeaturePipe();
public:
    /**
     * @brief init the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   init();

    /**
     * @brief uninit the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   uninit();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:     ////

    /**
     * @brief En-queue a request into the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a request of QParams structure.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    virtual MBOOL                   enque(
                                        QParams const& rParams
                                    );
    /**
     * @brief De-queue a result from the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a result of QParams structure.
     *
     * @param[in] i8TimeoutNs: timeout in nanoseconds \n
     *      If i8TimeoutNs > 0, a timeout is specified, and this call will \n
     *      be blocked until a result is ready. \n
     *      If i8TimeoutNs = 0, this call must return immediately no matter \n
     *      whether any buffer is ready or not. \n
     *      If i8TimeoutNs = -1, an infinite timeout is specified, and this call
     *      will block forever.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   deque(
                                        QParams& rParams,
                                        MINT64 i8TimeoutNs = -1
                                    );
    /**
      * @brief notify start video record for slow motion support.
      *
      * @param[in] wd: width
      * @param[in] ht: height
      *
      * @details
      *
      * @note
      *
      * @return
      *      - [true]
      */
     virtual MBOOL                  startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps=120);
     /**
      * @brief notify stop video record for slow motion support.
      *
      * @details
      *
      * @note
      *
      * @return
      *      - [true]
      */
     virtual MBOOL                  stopVideoRecord();


    /**
     * @brief get the last error code
     *
     * @details
     *
     * @note
     *
     * @return
     * - The last error code
     *
     */
    virtual MERROR                  getLastErrorCode() const {return 0;}
    /**
     * @brief get the supported crop paths
     *
     * @details
     *
     * @note
     *
     * @return
     * -
     *
     */

public:

    /**
     * @brief Configure features.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   setFeature(MUINT32 featureMask);

    /**
     * @brief Tuning per frame.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   setTuning(FeaturePipeTuningInfo& tuningInfo);

    /**
     * @brief User touch X-Y coordination
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   setTouch(MINT32 touchX, MINT32 touchY);

    /**
     * @brief Set parameters
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   set(FeaturePipeSetType setType, const MINTPTR setData);

    /**
     * @brief Set camera message-callback information.
     *
     * @details
     *
     * @note
     *
     * @return
     */
    virtual void                    setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo);

    virtual void                    enableMsgType(int32_t msgType);

    virtual void                    disableMsgType(int32_t msgType);

    virtual MBOOL                   performCallback(int32_t msgType, camera_frame_metadata_t* msgData);

    virtual MBOOL                   performCallback(int32_t msgType, int32_t msgExt);

    virtual void                    performCallback(QParams& rParams);

protected:
    virtual MBOOL                   initFeatureGraph();
    virtual MBOOL                   uninitFeatureGraph();
    virtual MBOOL                   enqueFeatureGraph(QParams const& rParams);
    virtual MBOOL                   dequeFeatureGraph(QParams& rParams, MINT64 i8TimeoutNs);

#ifdef USING_MTK_PERFSERVICE
    virtual MBOOL                   initPerf();
    virtual MBOOL                   uninitPerf();
    virtual MBOOL                   enablePerfService(MUINT32 scen);
    virtual MBOOL                   disablePerfService(MUINT32 scen);
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    FeaturePipeNode* mpP2aNode;
    FeaturePipeNode* mpVfbSwNode;
    FeaturePipeNode* mpEisSwNode;

private:
    MBOOL            mInited;
    ICamGraph*       mpCamGraph;
    FeaturePipeNode* mpSwitcherNode;
    FeaturePipeNode* mpFdNode;
    FeaturePipeNode* mpGpuNode;
    FeaturePipeNode* mpMdpNode;
    FeaturePipeNode* mpP2bNode;
    Mutex            mModuleMtx;
    const char       *mpName;
    MUINT32          mOpenedSensorIndex;
    //vector<QParams>         mDequeuedBufList;
    MUINT32          mFeatureOnMask;
    FeaturePipeBufHandler* mpAllocBufHdl;
    sp<CamMsgCbInfo> mpCamMsgCbInfo;         //  Pointer to Camera Message-Callback Info.
    Mutex            mReconfigMtx;
    volatile MINT32  mEnqueCount;
    MBOOL            mRecordingHint;
    MBOOL            misPerfService;

};

typedef enum FeaturePipeNodeObject_s
{
    FEATURE_STREAM_NODE_CONTROL,
    FEATURE_STREAM_NODE_P2A,
    FEATURE_STREAM_NODE_EISSW,
    FEATURE_STREAM_NODE_VFBSW,
    FEATURE_STREAM_NODE_FD,
    FEATURE_STREAM_NODE_GPU,
    FEATURE_STREAM_NODE_MDP,
    FEATURE_STREAM_NODE_P2B,
    FEATURE_STREAM_NODE_SWITCHER,
} FeaturePipeNodeObject_e;

/******************************************************************************
 *
 * @class FeaturePipeNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeNode : public ICamThreadNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeNode*       createInstance(FeaturePipeNodeObject_e eobject, MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe* pFeaturePipeObj);
    virtual void       destroyInstance() { delete this; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    FeaturePipeNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            MUINT32 openedSensorIndex,
            FeaturePipe* pFeaturePipeObj = NULL
            )
        : ICamThreadNode(name, type, policy, priority),
          mFeatureOnMask(featureOnMask),
          mOpenedSensorIndex(openedSensorIndex),
          mpFeaturePipeObj(pFeaturePipeObj),
          pBufHandler(NULL)
          {};

protected:   ////     to be implemented by subclass

    virtual MBOOL threadLoopUpdate() = 0;  //loop event
    virtual MBOOL onInit() = 0;
    virtual MBOOL onUninit() = 0;
    virtual MBOOL onStart() = 0;
    virtual MBOOL onStop() = 0;
    virtual MBOOL onNotify(
                    MUINT32 const datamsg,
                    MUINT32 const ext1,
                    MUINT32 const ext2
                    ) = 0;
    virtual MBOOL onPostBuffer(
                    MUINT32 const data,
                    MUINTPTR const buf,
                    MUINT32 const ext
                    ) = 0;
    virtual MBOOL onReturnBuffer(
                    MUINT32 const data,
                    MUINTPTR const buf,
                    MUINT32 const ext
                    ) = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MUINT32          mFeatureOnMask;
    MUINT32          mOpenedSensorIndex;
    FeaturePipe*     mpFeaturePipeObj;

protected:
    Mutex            mLock;
    Mutex            mUninitMtx;
    ICamBufHandler*  pBufHandler;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_H_

