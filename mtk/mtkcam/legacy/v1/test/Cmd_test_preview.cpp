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

//
#include <utils/Log.h>
#include <binder/Parcel.h>
#include <cutils/memory.h>

//
#include <gui/Surface.h>
//#include <gui/ISurface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/IGraphicBufferProducer.h>
//
#include <camera/ICamera.h>
#include <camera/CameraParameters.h>
#include <camera/MtkCameraParameters.h>
//
#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/Command.h"
#if defined(HAVE_COMMAND_test_preview)
//
using namespace android;
//
#include <sys/prctl.h>
//
//
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCmd_test_preview {
char const
gPromptText[] = {
    "\n"
    "\n test_preview <action> <optional arguments...>"
    "\n "
    "\n where <action> may be one of the following:"
    "\n  <-h>                       help"
    "\n  <startPreview>             start preview and test start."
    "\n  <stopPreview>              stop preview and test exit."
    "\n  <autoFocus>                autofocus."
    "\n  <cancelAutoFocus>          cancelAutofocus."
    "\n  <startFaceDetection>       start face detection"
    "\n  <stopFaceDetection>        stop face detection"
    "\n  <updateParameter>          update parameter"
    "\n "
    "\n where <optional arguments...> may be a combination of the followings:"
    "\n  <-app-mode=Default>        app mode; 'Default' by default."
    "\n                             -> 'Default' 'MtkEng' 'MtkAtv' 'MtkStereo' 'MtkVt'"
    "\n  <-open-id=0>               open id; 0 by default (main camera)."
    "\n  <-cam-mode=1>              camera mode in KEY_CAMERA_MODE; 0 by default."
    "\n                             TODO: should modify this definition!!!"
    "\n                             '0' refers to CAMERA_MODE_NORMAL"
    "\n                             '1' refers to CAMERA_MODE_MTK_PRV"
    "\n                             '2' refers to CAMERA_MODE_MTK_VDO"
    "\n                             '3' refers to CAMERA_MODE_MTK_VT"
    "\n  <-preview-size=640x480>    preview size; 640x480 by default."
    "\n  <-display=on>              display 'on' / 'off'; 'on' by default."
    "\n  <-display-orientation=90>  display orientation; 90 by default."
    "\n  <-scene=auto>              scene mode; 'auto','night','portrait','landscape'..."
    "\n  <-zoomStep=0>              Zoom step from '0' to '10'(total 11 scale)"
    "\n  <-awb=auto>                AWB mode set default as 'auto'; other choices:"
    "\n                             'incandescent'"
    "\n                             'fluorescent'"
    "\n                             'warm-fluorescent'"
    "\n                             'daylight'"
    "\n                             'cloudy-daylight'"
    "\n                             'twilight'"
    "\n                             'shade'"
    "\n  <-effect=none>             effect mode set default as 'none'; other choices:"
    "\n                             'mono'"
    "\n                             'solarize'"
    "\n                             'sepia'"
    "\n                             'posterize'"
    "\n                             'whiteboard'"
    "\n                             'blackboard'"
    "\n                             'aqua'"
    "\n  <-formt=NV21>              preview format: YV12, NV21(default)"
};
struct CmdImp : public CmdBase, public Thread, public CameraListener
{
    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Good place to do one-time initializations
    virtual status_t            readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CameraListener Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
    virtual void                notify(int32_t msgType, int32_t ext1, int32_t ext2) {}
    virtual void                postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {}
    virtual void                postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                                struct Argument : LightRefBase<Argument>
                                {
                                    String8                     ms8AppMode;
                                    int32_t                     mi4CamMode;
                                    int32_t                     mOpenId;
                                    Size                        mPreviewSize;
                                    bool                        mDisplayOn;
                                    int32_t                     mDisplayOrientation;
                                    String8                     ms8SceneMode;
                                    int32_t                     mZoomStep;
                                    String8                     ms8AWBmode;
                                    String8                     ms8EFFECTmode;
                                    String8                     ms8PrvFmt;
                                };

protected:  ////                Implementation.
    virtual bool                onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);

    virtual bool                onStartPreview(sp<Argument> pArgument);
    virtual bool                onStopPreview();
    virtual bool                onAutoFocus();
    virtual bool                onCancelAutoFocus();
    virtual bool                onStartFaceDetection();
    virtual bool                onStopFaceDetection();
    virtual bool                onUpdateParameter(sp<Argument> pArgument);
    virtual bool                onReset();

protected:  ////                Implementation.

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Surface)
    virtual bool                initSurface();
    virtual void                uninitSurface();

protected:  ////                Data Members (Surface)
    int32_t                     mi4SurfaceID;
    sp<SurfaceComposerClient>   mpSurfaceClient;
    sp<SurfaceControl>          mpSurfaceControl;
    sp<Surface>                 mpSurface;
    sp<IGraphicBufferProducer>  mpGbp;
    ANativeWindow*              mpWindow;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Camera)
    virtual bool                connectCamera(int id = 0);
    virtual void                disconnectCamera();
    virtual bool                setupParameters(sp<Camera> spCamera);

protected:  ////                Data Members (Camera)
    sp<Camera>                  mpCamera;

protected:  ////                Data Members (Parameters)
    sp<Argument>                mpArgument;

    Mutex                       mThreadMutex;
    Condition                   mThraedCond;
    int32_t volatile            mThraedRunning;

};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_test_preview, new CmdImp(HAVE_COMMAND_test_preview));
};  // NSCmd_test_preview
using namespace NSCmd_test_preview;


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    sp<Argument> pArgument = new Argument;
    onParseArgumentCommand(rvCmd, pArgument);
    return  onParseActionCommand(rvCmd, pArgument);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    //  (1) Set default.
    pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
    pArgument->mi4CamMode = MtkCameraParameters::CAMERA_MODE_MTK_PRV;
    pArgument->mOpenId = 0;
    pArgument->mPreviewSize = Size(640, 480);
    pArgument->mDisplayOrientation = 90;
    pArgument->mDisplayOn = true;
    pArgument->ms8SceneMode = String8("auto");
    pArgument->mZoomStep = 0;
    pArgument->ms8AWBmode = String8(CameraParameters::WHITE_BALANCE_AUTO);
    pArgument->ms8EFFECTmode = String8(CameraParameters::EFFECT_NONE);
    pArgument->ms8PrvFmt = String8(CameraParameters::PIXEL_FORMAT_YUV420SP);

    //  (2) Start to parse commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        String8 key, val;
        if  ( ! parseOneCmdArgument(s8Cmd, key, val) ) {
            continue;
        }
//        MY_LOGD("<key/val>=<%s/%s>", key.string(), val.string());
        //
        //
        if  ( key == "-app-mode" ) {
            if  ( val == "MtkEng" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ENG);
                continue;
            }
            if  ( val == "MtkAtv" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ATV);
                continue;
            }
            if  ( val == "MtkStereo" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_STEREO);
                continue;
            }
            if  ( val == "MtkVt" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_VT);
                continue;
            }
#if 0
            if  ( val == "MtkPhoto" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO);
                continue;
            }
            if  ( val == "MtkVideo" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO);
                continue;
            }
            if  ( val == "MtkZsd" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ZSD);
                continue;
            }
#endif
            pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
            continue;
        }
        //
        if  ( key == "-cam-mode" ) {
            pArgument->mi4CamMode = ::atoi(val);
            continue;
        }
        //
        if  ( key == "-open-id" ) {
            pArgument->mOpenId = ::atoi(val);
            continue;
        }
        //
        if  ( key == "-preview-size" ) {
            ::sscanf(val.string(), "%dx%d", &pArgument->mPreviewSize.width, &pArgument->mPreviewSize.height);
            MY_LOGD("preview-size : %d %d", pArgument->mPreviewSize.width, pArgument->mPreviewSize.height);
            continue;
        }
        //
        if  ( key == "-display-orientation" ) {
            pArgument->mDisplayOrientation = ::atoi(val);
            continue;
        }
        //
        if  ( key == "-display" ) {
            pArgument->mDisplayOn = (val == "on") ? 1 : 0;
            continue;
        }
        //
        if  ( key == "-scene") {
            pArgument->ms8SceneMode  = val;
        }
        //
        if  ( key == "-zoomStep") {
            pArgument->mZoomStep  = ::atoi(val);
            MY_LOGD("zoomStep : %d", pArgument->mZoomStep);
        }
        //
        if  ( key == "-awb") {
            pArgument->ms8AWBmode  = val;
            MY_LOGD("AWBmode : %s", pArgument->ms8AWBmode.string());
        }
        //
        if  ( key == "-effect") {
            pArgument->ms8EFFECTmode  = val;
            MY_LOGD("EFFECTmode : %s", pArgument->ms8EFFECTmode.string());
        }
        //
        if  ( key == "-format") {
            if  ( val == "NV21" ) {
                pArgument->ms8PrvFmt = String8(CameraParameters::PIXEL_FORMAT_YUV420SP);
            }
            if  ( val == "YV12" ) {
                pArgument->ms8PrvFmt = String8(CameraParameters::PIXEL_FORMAT_YUV420P);
            }
            MY_LOGD("format : %s", pArgument->ms8PrvFmt.string());
        }
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    //  (1) Start to parse ACTION commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        //
        if  ( s8Cmd == "-h" ) {
            MY_LOGD("%s", gPromptText);
            return  true;
        }
        //
        if  ( s8Cmd == "startPreview" ) {
            return  onStartPreview(pArgument);
        }
        //
        if  ( s8Cmd == "stopPreview" ) {
            return  onStopPreview();
        }
        //
        if  ( s8Cmd == "startFaceDetection" ) {
            return  onStartFaceDetection();
        }
        //
        if  ( s8Cmd == "stopFaceDetection" ) {
            return  onStopFaceDetection();
        }
        //
        if  ( s8Cmd == "updateParameter" ) {
            return  onUpdateParameter(pArgument);
        }
        //
        if  ( s8Cmd == "autoFocus" ) {
            return onAutoFocus();
        }
        //
        if  ( s8Cmd == "cancelAutoFocus" ) {
            return onCancelAutoFocus();
        }
    }
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onStartPreview(sp<Argument> pArgument)
{
    if  ( 1 == ::android_atomic_release_load(&mThraedRunning) ) {
        MY_LOGW("the test has been starting before...skip this command.");
        return  false;
    }
    //
    onReset();
    ::android_atomic_release_store(1, &mThraedRunning);
    mpArgument = pArgument;
    return  (OK == run());
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onStopPreview()
{
    requestExit();
    {
        Mutex::Autolock _lock(mThreadMutex);
        ::android_atomic_release_store(0, &mThraedRunning);
        mThraedCond.broadcast();
    }
    //
    return  (OK == join());
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onReset()
{
    ::android_atomic_release_store(0, &mThraedRunning);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
CmdImp::
readyToRun()
{
    ::prctl(PR_SET_NAME,"CmdImp", 0, 0, 0);

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_RR;
    int const priority  = 70;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
// Derived class must implement threadLoop(). The thread starts its life
// here. There are two ways of using the Thread object:
// 1) loop: if threadLoop() returns true, it will be called again if
//          requestExit() wasn't called.
// 2) once: if threadLoop() returns false, the thread will exit upon return.
bool
CmdImp::
threadLoop()
{
    status_t status;
    bool ret = false;
    sp<IGraphicBufferProducer> gbp;
    //
    ret =   initSurface()
        &&  connectCamera(mpArgument->mOpenId)
        &&  setupParameters(mpCamera)
            ;
    if  ( ! ret )
    {
        goto lbExit;
    }
    //
    //
    if  ( mpArgument->mDisplayOn ) {
        MY_LOGD("setPreviewDisplay, orientation=%d", mpArgument->mDisplayOrientation);
        mpCamera->setPreviewTarget(mpGbp);
        mpCamera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, mpArgument->mDisplayOrientation, 0);
    }
    mpCamera->startPreview();
    {
        Mutex::Autolock _lock(mThreadMutex);
        while   ( 1 == ::android_atomic_release_load(&mThraedRunning) )
        {
            nsecs_t nsTimeoutToWait = 30LL*1000LL*1000LL*1000LL;//wait 30 sec.
            MY_LOGD("Start to wait %lld sec...", nsTimeoutToWait/1000000000LL);
            status_t status = mThraedCond.waitRelative(mThreadMutex, nsTimeoutToWait);
        }
    }
    mpCamera->stopPreview();
    //
    //
lbExit:
    disconnectCamera();
    uninitSurface();
    onReset();
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
initSurface()
{
    mi4SurfaceID = 0;

    // create a client to surfaceflinger
    mpSurfaceClient = new SurfaceComposerClient();

    mpSurfaceControl = mpSurfaceClient->createSurface(
        String8("surface"), 480, 640, PIXEL_FORMAT_RGBA_8888, 0
    );
    SurfaceComposerClient::openGlobalTransaction();
    mpSurfaceControl->setLayer(100000);
    mpSurfaceControl->show();
    SurfaceComposerClient::closeGlobalTransaction();
    // pretend it went cross-process
    Parcel parcel;
    SurfaceControl::writeSurfaceToParcel(mpSurfaceControl, &parcel);
    parcel.setDataPosition(0);
    //
    mpSurface = mpSurfaceControl->getSurface();
    mpWindow = mpSurface.get();
    //
    if(mpSurface != NULL) {
        mpGbp = mpSurface->getIGraphicBufferProducer();
    }
    //
    CAM_LOGD("setupSurface: %p", mpSurface.get());
    return  (mpSurface != 0);
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdImp::
uninitSurface()
{
    mpWindow = NULL;
    mpSurface = 0;
    mpSurfaceControl = 0;
    mpSurfaceClient = 0;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
connectCamera(int id)
{
    status_t status = OK;
    status = Camera::setProperty(String8(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE), mpArgument->ms8AppMode);
    MY_LOGD("status(%d), app-mode=%s, format=%s", status, mpArgument->ms8AppMode.string(), mpArgument->ms8PrvFmt.string());

    mpCamera = Camera::connect(id, String16("CamTest"), Camera::USE_CALLING_UID);
    if  ( mpCamera == 0 )
    {
        MY_LOGE("Camera::connect, id(%d)", id);
        return  false;
    }
    //
    //
    mpCamera->setListener(this);
    //
    MY_LOGD("Camera::connect, id(%d), camera(%p)", id, mpCamera.get());
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdImp::
disconnectCamera()
{
    if  ( mpCamera != 0 )
    {
        MY_LOGD("Camera::disconnect, camera(%p)", mpCamera.get());
        mpCamera->disconnect();
        mpCamera = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
setupParameters(sp<Camera> spCamera)
{
    CameraParameters params(spCamera->getParameters());
    //
    params.set(MtkCameraParameters::KEY_CAMERA_MODE, mpArgument->mi4CamMode);
    //
    params.setPreviewSize(mpArgument->mPreviewSize.width, mpArgument->mPreviewSize.height);
    //
    params.set(CameraParameters::KEY_SCENE_MODE, mpArgument->ms8SceneMode.string());
    //
    params.set(CameraParameters::KEY_ZOOM, mpArgument->mZoomStep);
    //
    params.set(CameraParameters::KEY_WHITE_BALANCE, mpArgument->ms8AWBmode.string());
    //
    params.set(CameraParameters::KEY_EFFECT, mpArgument->ms8EFFECTmode.string());
    //
    params.set(CameraParameters::KEY_PREVIEW_FORMAT, mpArgument->ms8PrvFmt.string());
    //
    if  (OK != spCamera->setParameters(params.flatten()))
    {
        CAM_LOGE("setParameters\n");
        return  false;
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onStartFaceDetection()
{
    if ( mpCamera != 0 )
    {
        mpCamera->sendCommand(CAMERA_CMD_START_FACE_DETECTION, 0, 0);
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onStopFaceDetection()
{
    if ( mpCamera != 0 )
    {
        mpCamera->sendCommand(CAMERA_CMD_STOP_FACE_DETECTION, 0, 0);
        return true;
    }
    return false;
}


/******************************************************************************
*
*******************************************************************************/
bool
CmdImp::onAutoFocus()
{
    if ( mpCamera != 0 )
    {
        mpCamera->autoFocus();
        return true;
    }
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onCancelAutoFocus()
{
    if ( mpCamera != 0 )
    {
        mpCamera->cancelAutoFocus();
        return true;
    }
    return false;
}


/******************************************************************************
*
*******************************************************************************/
bool
CmdImp::onUpdateParameter(sp<Argument> pArgument)
{
    if (mpCamera != 0)
    {
        CameraParameters params(mpCamera->getParameters());
        //
        CAM_LOGD("%d, %d", mpArgument->mZoomStep, pArgument->mZoomStep);
        //
        if (mpArgument->mZoomStep != pArgument->mZoomStep)
        {
            params.set(CameraParameters::KEY_ZOOM, pArgument->mZoomStep);
            CAM_LOGD("onUpdateParameter: zoomStep(%d)", pArgument->mZoomStep);
        }
        //
        if (mpArgument->ms8AWBmode != pArgument->ms8AWBmode)
        {
            params.set(CameraParameters::KEY_WHITE_BALANCE, pArgument->ms8AWBmode.string());
            CAM_LOGD("onUpdateParameter: AWBmode(%s)", pArgument->ms8AWBmode.string());
        }
        //
        if (mpArgument->ms8EFFECTmode != pArgument->ms8EFFECTmode)
        {
            params.set(CameraParameters::KEY_EFFECT, pArgument->ms8EFFECTmode.string());
            CAM_LOGD("onUpdateParameter: EFFECTmode(%s)", pArgument->ms8EFFECTmode.string());
        }
        //
        params.set(CameraParameters::KEY_SCENE_MODE, pArgument->ms8SceneMode.string());
        if  (OK != mpCamera->setParameters(params.flatten()))
        {
            CAM_LOGE("setParameters\n");
            return  false;
        }
        //
        mpArgument = pArgument;
    }

    return true;
}

/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

