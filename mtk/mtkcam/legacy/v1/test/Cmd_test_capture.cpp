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
#include <cutils/memory.h>
//
/*
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
*/
//
#include <gui/Surface.h>
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
#if defined(HAVE_COMMAND_test_capture)
//
using namespace android;
//
//
//
/******************************************************************************
 *  Command
 *      test_capture <-h> <-shot-mode=testshot> <-shot-count=1> <-picture-size=2560x1920> <-preview-size=640x480> <-display-orientation=90>
 *
 *      -h:             help
 *      -shot-count:    shot count; 1 by default.
 *      -picture-size:  picture size; 2560x1920 by default.
 *      -preview-size:  preview size; 640x480 by default.
 *      -shot-mode:     shot mode; testshot by default.
 *                      For example: "normal", "hdr", "continuousshot", ......
 *      -display-orientation:   display orientation; 90 by default.
 *
 ******************************************************************************/
namespace NSCmd_test_capture {
struct CmdImp : public CmdBase, public CameraListener
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
//  CameraListener Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
    virtual void                notify(int32_t msgType, int32_t ext1, int32_t ext2);
    virtual void                postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata);
    virtual void                postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Implementation.
    virtual bool                onParseCommand(Vector<String8>& rvCmd);
    virtual bool                onReset();
    virtual bool                onExecute();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Camera)
    virtual bool                connectCamera(int id = 0);
    virtual void                disconnectCamera();
    virtual bool                setupParameters(sp<Camera> spCamera);

protected:  ////                Data Members (Camera)
    sp<Camera>                  mpCamera;

protected:  ////
    String8                     mShotMode;
    int32_t                     mShotCount;
    Size                        mPictureSize;
    Size                        mPreviewSize;
    int32_t                     mDisplayOrientation;
    int32_t                     mPictureOrientation;

protected:  ////
    Mutex                       mMutex;
    Condition                   mCond;
    int32_t volatile            mi4ShutterCallbackCount;
    int32_t volatile            mi4CompressedImageCallbackCount;

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

};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_test_capture, new CmdImp(HAVE_COMMAND_test_capture));
};  // NSCmd_test_capture
using namespace NSCmd_test_capture;


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    return  onParseCommand(rvCmd) ? onExecute() : false;
}


/******************************************************************************
 *  Command
 *      test_capture <-h> <-shot-mode=testshot> <-shot-count=1> <-picture-size=2560x1920> <-preview-size=640x480> <-display-orientation=90>
 *
 *      -h:             help
 *      -shot-count:    shot count; 1 by default.
 *      -picture-size:  picture size; 2560x1920 by default.
 *      -preview-size:  preview size; 640x480 by default.
 *      -shot-mode:     shot mode; testshot by default.
 *                      For example: "normal", "hdr", "continuousshot", ......
 *      -display-orientation:   display orientation; 90 by default.
 *
 ******************************************************************************/
bool
CmdImp::
onParseCommand(Vector<String8>& rvCmd)
{
    //  (1) Set default.
    mShotMode = "testshot";
    mShotCount = 1;
    mPictureSize = Size(2560, 1920);
    mPreviewSize = Size(640, 480);
    mDisplayOrientation = 90;
    mPictureOrientation = 0;

    //  (2) Start to parse commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        if  ( rvCmd[i] == "-h" ) {
            String8 text;
            text += "\n";
            text += "\n   test_capture <-h> <-shot-mode=testshot> <-shot-count=1> <-picture-size=2560x1920> <-preview-size=640x480> <-display-orientation=90 -jpeg-orienation=0>";
            text += "\n   -h:             help";
            text += "\n   -shot-count:    shot count; 1 by default.";
            text += "\n   -picture-size:  picture size; 2560x1920 by default.";
            text += "\n   -preview-size:  preview size; 640x480 by default.";
            text += "\n   -shot-mode:     shot mode; testshot by default.";
            text += "\n                   For example: normal, hdr, continuousshot, ......";
            text += "\n   -display-orientation:   display orientation; 90 by default.";
            text += "\n   -pic-orientation: jpeg orienation; 0 by default.";
            MY_LOGD("%s", text.string());
            return  false;
        }
        //
        String8 key, val;
        parseOneCmdArgument(rvCmd[i], key, val);
//        MY_LOGD("<key/val>=<%s/%s>", key.string(), val.string());
        //
        //
        if  ( key == "-shot-mode" ) {
            mShotMode = val;
            continue;
        }
        //
        if  ( key == "-shot-count" ) {
            mShotCount = ::atoi(val);
            continue;
        }
        //
        if  ( key == "-picture-size" ) {
            ::sscanf(val.string(), "%dx%d", &mPictureSize.width, &mPictureSize.height);
            MY_LOGD("picture-size : %d %d", mPictureSize.width, mPictureSize.height);
            continue;
        }
        //
        if  ( key == "-preview-size" ) {
            ::sscanf(val.string(), "%dx%d", &mPreviewSize.width, &mPreviewSize.height);
            MY_LOGD("preview-size : %d %d", mPreviewSize.width, mPreviewSize.height);
            continue;
        }
        //
        if  ( key == "-display-orientation" ) {
            mDisplayOrientation = ::atoi(val);
            continue;
        }
        //
        if  (key == "-pic-orientation" )  {
            mPictureOrientation = ::atoi(val);
            printf("picture orientation = %d\n", mPictureOrientation);
            continue;
        }
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onReset()
{
    mi4ShutterCallbackCount = 0;
    mi4CompressedImageCallbackCount = 0;
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onExecute()
{
    bool ret = false;
    status_t status = OK;
    //
    ret =
            onReset()
        &&  initSurface()
        &&  connectCamera(0)
        &&  setupParameters(mpCamera)
            ;
    if  ( ! ret )
    {
        goto lbExit;
    }
    //
    //
    mpCamera->setPreviewTarget(mpGbp);
    if  ( OK != (status = mpCamera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, mDisplayOrientation, 0)) ) {
        MY_LOGE("sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION), status[%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }
    if  ( OK != (status = mpCamera->startPreview()) ) {
        MY_LOGE("startPreview(), status[%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }
    ::sleep(2);
    mpCamera->takePicture(CAMERA_MSG_SHUTTER | CAMERA_MSG_COMPRESSED_IMAGE);
    {
        Mutex::Autolock _lock(mMutex);
        while   (
                    mShotCount > ::android_atomic_release_load(&mi4ShutterCallbackCount)
                ||  mShotCount > ::android_atomic_release_load(&mi4CompressedImageCallbackCount)
                )
        {
            nsecs_t nsTimeoutToWait = mShotCount * 60LL*1000LL*1000LL*1000LL;//wait 10 sec x n.
            MY_LOGD("Start to wait %lld sec...", nsTimeoutToWait/1000000000LL);
            status_t status = mCond.waitRelative(mMutex, nsTimeoutToWait);
            if  ( OK != status ) {
                CAM_LOGE("status[%s(%d)]\n", ::strerror(-status), -status);
                break;
            }
        }
    }
    mpCamera->stopPreview();
    //
    //
lbExit:
    disconnectCamera();
    uninitSurface();
    //
    return  true;
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
    params.set(MtkCameraParameters::KEY_CAMERA_MODE, MtkCameraParameters::CAMERA_MODE_NORMAL);
    //
    params.setPreviewSize(mPreviewSize.width, mPreviewSize.height);
    params.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420P);    //  YV12
    params.setPictureSize(mPictureSize.width, mPictureSize.height);

    params.setPictureFormat("jpeg");
    params.set(MtkCameraParameters::KEY_JPEG_QUALITY, "100");
    params.set(MtkCameraParameters::KEY_CAPTURE_MODE, mShotMode.string());

    params.set(MtkCameraParameters::KEY_BURST_SHOT_NUM, mShotCount);
    params.set(MtkCameraParameters::KEY_CAPTURE_PATH, "sdcard/");

    params.set(MtkCameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, "100");
    params.set(MtkCameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, "160");
    params.set(MtkCameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, "120");

    params.set(MtkCameraParameters::KEY_ROTATION, mPictureOrientation);
    params.set(MtkCameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, "120");

    params.set(MtkCameraParameters::KEY_GPS_LATITUDE, "25.032146");
    params.set(MtkCameraParameters::KEY_GPS_LONGITUDE, "121.564448");
    params.set(MtkCameraParameters::KEY_GPS_ALTITUDE, "21.0");
    params.set(MtkCameraParameters::KEY_GPS_TIMESTAMP, "1251192757");
    params.set(MtkCameraParameters::KEY_GPS_PROCESSING_METHOD, "GPS");
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
void
CmdImp::
notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
    if  ( msgType == CAMERA_MSG_SHUTTER )
    {
        CAM_LOGD("CAMERA_MSG_SHUTTER \n");
        Mutex::Autolock _lock(mMutex);
        ::android_atomic_inc(&mi4ShutterCallbackCount);
        mCond.broadcast();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdImp::
postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
    uint8_t* pBase = (uint8_t *)heap->base() + offset;
    MY_LOGD("msgType=%x CAMERA_MSG_PREVIEW_FRAME?%d base/size=%p/%d", msgType, (msgType & CAMERA_MSG_PREVIEW_FRAME), pBase, size);
    //
    if  ( msgType == CAMERA_MSG_COMPRESSED_IMAGE )
    {
        CAM_LOGD("CAMERA_MSG_COMPRESSED_IMAGE \n");
        Mutex::Autolock _lock(mMutex);
        ::android_atomic_inc(&mi4CompressedImageCallbackCount);
        mCond.broadcast();
        //
        static int i = 0;
        String8 filename = String8::format("/data/cap%02d.jpeg", i);
        saveBufToFile(filename, pBase, size);
        i++;
    }
}


/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

