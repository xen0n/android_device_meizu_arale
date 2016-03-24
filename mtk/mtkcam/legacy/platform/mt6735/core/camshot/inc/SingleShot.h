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
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_
//
#include <mtkcam/camnode/ShotCallbackNode.h>
namespace NSCamNode{
    class AllocBufHandler;
    class ICamGraph;
    class ICamNode;
    class Pass1Node;
    class ShotCallbackNode;
    class Pass2Node;
    class JpegEncNode;
};
using namespace NSCamNode;
#include <mtkcam/featureio/aaa_hal_common.h>

#include <semaphore.h>
#include <vector>
using namespace std;
//
#include <utils/Mutex.h>
using namespace android;
/*******************************************************************************
*
********************************************************************************/

namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/

class SShotCtrlNode;
class CamShotImp;
struct map_data2node;
struct connect_data_rule;
struct connect_notify_rule;

/*******************************************************************************
*
********************************************************************************/
class SingleShot : public CamShotImp
{
public:     ////    Constructor/Destructor.
                    SingleShot(
                        EShotMode const eShotMode,
                        char const*const szCamShotName
                    );

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start(SensorParam const & rSensorParam);
    virtual MBOOL   startAsync(SensorParam const & rSensorParam);
    virtual MBOOL   startOne(SensorParam const  & rSensorParam);
    virtual MBOOL   startOne(SensorParam const & rSensorParam, IImageBuffer const *pImgBuffer);
    virtual MBOOL   startOne(SensorParam const & rSensorParam,
                            IImageBuffer const *pImgBuf_0, IImageBuffer const *pImgBuf_1,
                            IImageBuffer const *pBufPrv_0=NULL, IImageBuffer const *pBufPrv_1=NULL) { return MTRUE; }
    virtual MBOOL   stop();

public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam);
    virtual MBOOL   setJpegParam(JpegParam const & rParam);

public:     ////    buffer setting.
    virtual MBOOL   registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer);

public:     ////    Info.
    virtual MBOOL   setPrvBufHdl(MVOID* pCamBufHdl);
    virtual MBOOL   setCapBufMgr(MVOID* pCapBufMgr) { return MTRUE; }

public:     ////    Old style commnad.
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

protected:  ////    callback
    static  MBOOL   fgNodeNotifyCallback(MVOID* user, NodeNotifyInfo const msg);
    static  MBOOL   fgNodeDataCallback(MVOID* user, NodeDataInfo const msg);

protected:  //// helper function
    //
    MBOOL           updateProfile(MUINT32 profile);
    MBOOL           updateNeededData(MUINT32* pNeededData, MUINT32* pNeededCb) const;
    MBOOL           updateNeededNodeData(MUINT32 neededData, MUINT32* pNodeData) const;
    //
    MBOOL           createNodes(
                        MUINT32 const NodeDataSet,
                        MUINT32 const NodeCbSet,
                        IImageBuffer const *pImgBuffer);
    MBOOL           destroyNodes();
    MBOOL           connectNodes(MUINT32 const NodeDataSet);
    //
    MUINT32         mapToNodeData(MUINT32 dataType) const;
    //
    MVOID           getRotatedPicSize(MUINT32 *pW, MUINT32 *pH, MUINT32 *pThumbW, MUINT32 *pThumbH) const;
    MVOID           getAspectAlignedThumbSize(MUINT32 *pThumbW, MUINT32 *pThumbH) const;
    MVOID           getRotatedAspectAlignedThumbSize(MUINT32 *pThumbW, MUINT32 *pThumbH) const;

    ICamNode*       getNode(MUINT32 node);

    MBOOL           doRegisterBuffers();
    MBOOL           enableData(MUINT32 const dataType);
    MBOOL           prepareMemory(
                        MUINT32 const dataType,
                        MUINT32 const _w, MUINT32 const _h, EImageFormat const _fmt,
                        MUINT32 const _trans,
                        MINT32 const usage = 0);

    MVOID           updateFinishDataMsg(MUINT32 datamsg);
    MBOOL           reset();

    MBOOL           doDebugDump();
    MBOOL           getSensorSizeFmt(MSize* pSize);

private:    //// data members

    enum
    {
        SHOT_PROFILE_NORMAL,
        SHOT_PROFILE_ZSD,
        SHOT_PROFILE_PURERAW,
        SHOT_PROFILE_YUV,
    };

    typedef struct regbuf
    {
        MUINT32                  muDATA;
        IImageBuffer const*      mBuffer;
    } regbuf_t;

    ////      parameters
    SensorParam              mSensorParam;
    ShotParam                mShotParam;
    JpegParam                mJpegParam;

    MUINT32                  muRegisteredBufType;
    AllocBufHandler*         mpAllocBufHandler;
    ICamBufHandler*          mpPrvBufHandler;

    //        register buffers
    vector<regbuf_t>         mvRegBuf;

    //        graph data mapping/connection
    MUINT32                        muCapStyle;
    MUINT32                        mShotProfile;
    map_data2node const*           mpMap2Node;
    connect_data_rule const*       mpConnectDataRule;
    connect_notify_rule const*     mpConnectNotifyRule;

    //        graph/node
    ICamGraph*               mpGraph;
    Pass1Node*               mpPass1;
    SShotCtrlNode*           mpCtrl;
    ShotCallbackNode*        mpCallback;
    Pass2Node*               mpCapPass2;
    Pass2Node*               mpPrvPass2;
    JpegEncNode*             mpJpegencHw;
    JpegEncNode*             mpJpegencSw;

    //        shot related control
    Mutex                    mLock;
    MBOOL                    mbDoShutterCb;
    MUINT32                  mFinishedData;
    sem_t                    mShotDone;

    MBOOL                    mbPureRaw;
    MUINT32                  muNRType;

    // vhdr use
    MBOOL                    mbVHdr;
    NS3A::EIspProfile_T      mVhdrProfile;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_

