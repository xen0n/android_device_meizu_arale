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

#ifndef _MTK_HAL_CAMADAPTER_MTKSTEREO_INC_MTKSTEREOCAMADAPTER_H_
#define _MTK_HAL_CAMADAPTER_MTKSTEREO_INC_MTKSTEREOCAMADAPTER_H_
//
/*******************************************************************************
*
*******************************************************************************/
#include "inc/IState.h"
//
//
#if '1'==MTKCAM_HAVE_3A_HAL
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
#include <Callback/ICallBack3A.h>
#endif
//
#include "inc/CaptureCmdQueThread.h"
#include "inc/ZipImageCallbackThread.h"
//
#include <Scenario/Shot/IShot.h>
using namespace NSShot;
//
#include <vector>
using namespace std;
//
#include <mtkcam/camnode/AllocBufHandler.h>
#include <mtkcam/camnode/ICamGraphNode.h>
#include <mtkcam/camnode/pass1node.h>
#include <mtkcam/camnode/pass2node.h>
#include <mtkcam/camnode/SyncNode.h>
#include <mtkcam/camnode/stereonode.h>
#include <mtkcam/camnode/StereoCtrlNode.h>
#include <mtkcam/camnode/ImgTransformNode.h>
using namespace NSCamNode;
//
#include <CamNode/DefaultCtrlNode.h>
#include <CamNode/DefaultBufHandler.h>
#include <CamNode/RecBufSnapshotlNode.h>
//
#include <ThermalMonitor/ThermalMonitor.h>
#include <mtkcam/drv/res_mgr_drv.h>
//
namespace android {
namespace NSMtkStereoCamAdapter {
//
class CamAdapter : public BaseCamAdapter
                 , public IStateHandler
                 , public ICaptureCmdQueThreadHandler
                 , public IShotCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICamAdapter Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * Initialize the device resources owned by this object.
     */
    virtual bool                    init();

    /**
     * Uninitialize the device resources owned by this object. Note that this is
     * *not* done in the destructor.
     */
    virtual bool                    uninit();

    /**
     * Start preview mode.
     */
    virtual status_t                startPreview();

    /**
     * Stop a previously started preview.
     */
    virtual void                    stopPreview();

    /**
     * Returns true if preview is enabled.
     */
    virtual bool                    previewEnabled() const;

    /**
     * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
     * message is sent with the corresponding frame. Every record frame must be released
     * by a cameral hal client via releaseRecordingFrame() before the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
     * to manage the life-cycle of the video recording frames, and the client must
     * not modify/access any video recording frames.
     */
    virtual status_t                startRecording();

    /**
     * Stop a previously started recording.
     */
    virtual void                    stopRecording();

    /**
     * Returns true if recording is enabled.
     */
    virtual bool                    recordingEnabled() const;

    /**
     * Start auto focus, the notification callback routine is called
     * with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
     * will be called again if another auto focus is needed.
     */
    virtual status_t                autoFocus();

    /**
     * Cancels auto-focus function. If the auto-focus is still in progress,
     * this function will cancel it. Whether the auto-focus is in progress
     * or not, this function will return the focus position to the default.
     * If the camera does not support auto-focus, this is a no-op.
     */
    virtual status_t                cancelAutoFocus();

    /**
     * Returns true if capture is on-going.
     */
    virtual bool                    isTakingPicture() const;

    /**
     * Take a picture.
     */
    virtual status_t                takePicture();

    /**
     * Cancel a picture that was started with takePicture.  Calling this
     * method when no picture is being taken is a no-op.
     */
    virtual status_t                cancelPicture();

    /**
     * set continuous shot speed
     */
    virtual status_t                setCShotSpeed(int32_t i4CShotSpeeed);

    /**
     * Set the camera parameters. This returns BAD_VALUE if any parameter is
     * invalid or not supported.
     */
    virtual status_t                setParameters();

    /**
     * Send command to camera driver.
     */
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStateHandler Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual status_t                onHandleStartPreview();
    virtual status_t                onHandleStopPreview();
    //
    virtual status_t                onHandlePreCapture();
    virtual status_t                onHandleCapture();
    virtual status_t                onHandleCaptureDone();
    virtual status_t                onHandleCancelCapture();
    //
    virtual status_t                onHandlePreviewCapture();
    virtual status_t                onHandlePreviewCaptureDone();
    virtual status_t                onHandleCancelPreviewCapture();
    //
    virtual status_t                onHandleStartRecording();
    virtual status_t                onHandleStopRecording();
    virtual status_t                onHandleVideoSnapshot();
    virtual status_t                onHandleVideoSnapshotDone();
    virtual status_t                onHandleCancelVideoSnapshot();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICaptureCmdQueThreadHandler Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual bool                    onCaptureThreadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IShotCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    //  Directly include this inline file to reduce file dependencies since the
    //  interfaces in this file may often vary.
    #include "inc/ImpShotCallback.inl"

private:
    //  [Focus Callback]
    status_t                        init3A();
    void                            uninit3A();
    void                            enableAFMove(bool flag);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    virtual                         ~CamAdapter();
                                    CamAdapter(
                                        String8 const&      rName,
                                        int32_t const       i4OpenId,
                                        sp<IParamsManager>  pParamsMgr
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    bool                            updateShotInstance();
    status_t                        prepareToCapture();
    bool                            waitForShotDone();
    status_t                        forceStopAndCleanPreview();

    int32_t                         getOpenId_Main() const      { return mSensorId_Main; }
    int32_t                         getOpenId_Main2() const     { return mSensorId_Main2; }

    static void*                    doThreadPrecap(void* arg);

    bool                            main2precapture();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    //
    int32_t                         mSensorId_Main;
    int32_t                         mSensorId_Main2;
    //
    IStateManager*                  mpStateManager;
    //
    sp<ICaptureCmdQueThread>        mpCaptureCmdQueThread;
    wp<ZipImageCallbackThread>      mpZipCallbackThread;
    //
    sp<IShot>                       mpShot;
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    ICallBack3A*                    mpCallback3a;
    #endif
    //
    String8                         mUserName;
    //
    ICamGraph*                      mpCamGraph;
    AllocBufHandler*                mpAllocBufHdl;
    AllocBufHandler*                mpAllocBufHdl_Main2;
    DefaultBufHandler*              mpDefaultBufHdl;
    //
    Pass1Node*                      mpPass1Node;
    Pass1Node*                      mpPass1Node_Main2;
    DefaultCtrlNode*                mpDefaultCtrlNode;
    DefaultCtrlNode*                mpDefaultCtrlNode_Main2;
    //
    SyncNode*                       mpSyncNode;
    StereoNode*                     mpStereoNode;
    StereoNode*                     mpStereoNode_Main2;
    StereoCtrlNode*                 mpStereoCtrlNode;
    ImgTransformNode*               mpImgTransformNode;
    //
    CapBufMgr*                      mpCapBufMgr;
    CapBufMgr*                      mpCapBufMgr_Main2;
    //
    bool                            mbZsdMode;
    bool                            mbTakePicPrvNotStop;
    bool                            mbFixFps;
    uint32_t                        mCamMode;
    uint32_t                        mShotMode;
    //
    int32_t                         mLastVdoWidth;
    int32_t                         mLastVdoHeight;
    //
    sp<ThermalMonitor>              mspThermalMonitor;
    ResMgrDrv*                      mpResMgrDrv;
    //
    pthread_t                       mThreadHandle;
};


};  // namespace NSMtkStereoCamAdapter
};  // namespace android
#endif  //_MTK_HAL_CAMADAPTER_MTKSTEREO_INC_MTKSTEREOCAMADAPTER_H_

