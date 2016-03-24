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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM1DEVICEBASE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM1DEVICEBASE_H_
//
#include <v1/common.h>
using namespace android::MtkCamUtils;
//
#include <v1/IParamsManager.h>
#include <v1/ICamAdapter.h>
#include <v1/ICamClient.h>
#include <v1/IDisplayClient.h>
//
#include "Cam1Device.h"
//


/******************************************************************************
 *
 ******************************************************************************/
namespace android {


/******************************************************************************
 *
 ******************************************************************************/
class Cam1DeviceBase : public NSCam::Cam1Device, protected ICam1DeviceCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Device Info.
    //
    NSCam::ICamDeviceManager*       mpDeviceManager;    //  device manager.
    String8 const                   mDevName;           //  device name.
    int32_t const                   mi4OpenId;          //  device open ID.
    sp<CamMsgCbInfo>                mpCamMsgCbInfo;
    //
protected:  ////
    //
    sp<IParamsManager>              mpParamsMgr;
    sp<ICamAdapter>                 mpCamAdapter;
    sp<ICamClient>                  mpCamClient;
    sp<IDisplayClient>              mpDisplayClient;
    Vector<sp<ICamClient> >         vmpCamClient;
    //
    bool                            mIsPreviewEnabled;
    bool                            mIsRaw16CBEnabled;
    //
    int32_t                         mOrientation;
    //
    int32_t                         mLastEnableMsg;
    //
public:
    void                            startPreviewImp();
    bool                            waitStartPreviewDone();
    static void*                    startPreviewThread(void* arg);
    pthread_t                       mStartPreviewTThreadHandle;
    bool                            mbWindowReady;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Info.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
                                    struct CommandInfo
                                    {
                                        int32_t     cmd;
                                        int32_t     arg1;
                                        int32_t     arg2;
                                        CommandInfo(
                                            int32_t _cmd  = 0,
                                            int32_t _arg1 = 0,
                                            int32_t _arg2 = 0
                                        )
                                            : cmd(_cmd)
                                            , arg1(_arg1)
                                            , arg2(_arg2)
                                        {}
                                    };
    typedef KeyedVector<int32_t, CommandInfo>   CommandInfoMap_t;
    CommandInfoMap_t                mTodoCmdMap;
    Mutex                           mTodoCmdMapLock;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    virtual                         ~Cam1DeviceBase();
                                    Cam1DeviceBase(
                                        String8 const&          rDevName,
                                        int32_t const           i4OpenId
                                    );

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                    Operations.

    virtual int32_t                 queryDisplayBufCount() const    { return 3; }

    virtual bool                    queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height);

    virtual bool                    initCameraAdapter();

    virtual status_t                initDisplayClient(preview_stream_ops* window);
    virtual status_t                enableDisplayClient();
    virtual void                    disableDisplayClient();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  [Template method] Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.

    /**
     * [Template method] Called by initialize().
     * Initialize the device resources owned by this object.
     */
    virtual bool                    onInit();

    /**
     * [Template method] Called by uninitialize().
     * Uninitialize the device resources owned by this object. Note that this is
     * *not* done in the destructor.
     */
    virtual bool                    onUninit();

    /**
     * [Template method] Called by startPreview().
     */
    virtual bool                    onStartPreview()    { return true; }

    /**
     * [Template method] Called by stopPreview().
     */
    virtual void                    onStopPreview()     {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Cam1Device Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual status_t                closeDevice();

    virtual status_t                initialize();
    virtual void                    uninitialize();
    //
    virtual void                    setCallbacks(
                                        camera_notify_callback notify_cb,
                                        camera_data_callback data_cb,
                                        camera_data_timestamp_callback data_cb_timestamp,
                                        camera_request_memory get_memory,
                                        void *user
                                    );
    virtual void                    setCallbacks(
                                        mtk_camera_metadata_callback metadata_cb,
                                        void *user
                                    );
    virtual void                    enableMsgType(int32_t msgType);
    virtual void                    disableMsgType(int32_t msgType);
    virtual bool                    msgTypeEnabled(int32_t msgType);
    //
    virtual status_t                setPreviewWindow(preview_stream_ops* window);
    //
    virtual status_t                startPreview();
    virtual void                    stopPreview();
    virtual bool                    previewEnabled();
    //
    virtual status_t                storeMetaDataInBuffers(bool enable);
    virtual status_t                startRecording();
    virtual void                    stopRecording();
    virtual bool                    recordingEnabled();
    virtual void                    releaseRecordingFrame(const void *opaque);
    //
    virtual status_t                autoFocus();
    virtual status_t                cancelAutoFocus();
    //
    virtual status_t                takePicture();
    virtual status_t                cancelPicture();
    //
    virtual status_t                setParameters(const char* params);
    virtual char*                   getParameters();
    virtual void                    putParameters(char *params);
    //
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);
    virtual status_t                dump(int fd);
    //
    virtual status_t                addCamClient(sp<ICamClient> const& client, const char* name);
    virtual status_t                removeCamClient(sp<ICamClient> const& client);
    //
    virtual bool                    disableWaitSensorThread(bool diable) { return false; }
    virtual bool                    waitThreadInitDone() { return true; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICam1DeviceCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual void                    onCam1Device_NotifyCb(
                                        int32_t const   msgType,
                                        intptr_t const  ext1
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICamDevice Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual char const*             getDevName() const  { return mDevName.string(); }
    virtual int32_t                 getOpenId() const   { return mi4OpenId; }

    virtual void                    setDeviceManager(NSCam::ICamDeviceManager* manager);
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace android
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM1DEVICEBASE_H_

