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

#ifndef _MTK_HARDWARE_MTKCAM_V1_CLIENT_SDKCLIENT_GESTURE_H_
#define _MTK_HARDWARE_MTKCAM_V1_CLIENT_SDKCLIENT_GESTURE_H_
//
using namespace android;
using namespace MtkCamUtils;
#include <v1/IParamsManager.h>
#include <v1/ICamClient.h>
#include <v1/sdkClient/IGestureClient.h>

#include <mmsdk_common.h>
#include <gesture_device.h>
//
#include <utils/ImageBufferHeap.h>

namespace android {
namespace NSSdkClient {
namespace NSGestureClient {
/******************************************************************************
 *
 ******************************************************************************/
class ImgBufManager;

/******************************************************************************
 *
 ******************************************************************************/
class ProfileTool
{
protected:  ////        Data Members.
    String8             ms8SubjectName;
                        //
    float               mfFps;
    nsecs_t             mnsDuration;
                        //
    int32_t             mi4Count;
    nsecs_t             mnsStart;
    nsecs_t             mnsEnd;

public:     ////        Interfaces.
                        ProfileTool(char const*const pszSubjectName);
                        //
    nsecs_t             getDuration() const     { return mnsDuration; }
    float               getFps() const          { return mfFps; }
                        //
    void                pulse();
    void                pulse(nsecs_t nsTimestamp);
                        //
    void                reset();
    void                reset(nsecs_t nsInitTimestamp);
                        //
    void                updateFps();
                        //
    void                showFps() const;
};


/******************************************************************************
 *  Preview Client Handler.
 ******************************************************************************/
class GestureClient : public IGestureClient
                    , public Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    //
                                    GestureClient();
    virtual                         ~GestureClient();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual bool                    init();
    virtual bool                    init(sp<IParamsManager> pParamsMgr);
    virtual bool                    uninit();

    virtual bool                    setImgBufProviderClient(
                                        sp<IImgBufProviderClient>const& rpClient
                                    );

    virtual void                    setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo);
    //
    virtual void                    enableMsgType(int32_t msgType);
    virtual void                    disableMsgType(int32_t msgType);
    virtual bool                    msgTypeEnabled(int32_t msgType);
    //
    virtual bool                    startPreview();
    virtual void                    stopPreview();
    virtual bool                    previewEnabled();
    //
    virtual void                    takePicture();
    //
    virtual bool                    startRecording() {return true;};
    virtual void                    stopRecording()  {};
    virtual void                    releaseRecordingFrame(const void *opaque)  {};

    //
    virtual status_t                sendCommand(
                                        int32_t cmd,
                                        int32_t arg1,
                                        int32_t arg2
                                    );

    //
    virtual status_t                dump(int fd, Vector<String8>& args);
    //
    //
    virtual bool                      start();
    //
    virtual bool                      stop();
    //
    virtual void                  setCallbacks(GestureCallback_t gesture_cb, MVOID* user);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                    requestExit();

    // Good place to do one-time initializations
    virtual status_t                readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                    threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Queue.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Definitions.
                                    struct Command
                                    {
                                        //  Command ID.
                                        enum EID
                                        {
                                            eID_EXIT,
                                            eID_WAKEUP,
                                            eID_PREVIEW_FRAME,
                                            eID_POSTVIEW_FRAME
                                        };
                                        //
                                        //  Operations.
                                        Command(EID const _eId = eID_WAKEUP)
                                            : eId(_eId)
                                        {}
                                        //
                                        static  char const* getName(EID const _eId);
                                        inline  char const* name() const    { return getName(eId); }
                                        //
                                        //  Data Members.
                                        EID     eId;
                                    };

protected:  ////                    Operations.

    virtual void                    postCommand(Command const& rCmd);
    virtual bool                    getCommand(Command& rCmd);
    //
    virtual void                    onClientThreadLoop(Command const& rCmd);
    //
    inline  int32_t                 getThreadId() const    { return mi4ThreadId; }


protected:  ////                    Data Members.
    List<Command>                   mCmdQue;
    Mutex                           mCmdQueMtx;
    Condition                       mCmdQueCond;    //  Condition to wait: [ ! exitPending() && mCmdQue.empty() ]
    int32_t                         mi4ThreadId;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    //  enable if both preview started && message enabled; otherwise disable.
    virtual bool                    isEnabledState() const;
    bool                            onStateChanged();
    void                            updateMsg(int32_t const oldMsgType, int32_t const newMsgType);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                    Common Info.

    mutable Mutex                   mModuleMtx;
    sp<CamMsgCbInfo>                mpCamMsgCbInfo;         //  Pointer to Camera Message-Callback Info.
    sp<IParamsManager>              mpParamsMgr;            //  Pointer to Parameters Manager.
    volatile int32_t                mIsMsgEnabled;          //  Message Enabled ?
    volatile int32_t                mIsPrvStarted;          //  Preview Started ?
    volatile int32_t                mIsGestureHalStarted;    //GestureHAL started ?

    String8                         ms8PrvTgtFmt;           //  Preview Target Format.
    int32_t                         mi4PrvWidth;            //  Preview Width.
    int32_t                         mi4PrvHeight;           //  Preview Height.

    int32_t                         mGestureRunning;
    int32_t                         mi4BufWidth;
    int32_t                         mi4BufHeight;
    hand_detection_result_t         *mpDetectedResult;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                    Callback.
    //
    int32_t                         mi4CallbackRefCount;    //  Preview callback reference count.
    int64_t                         mi8CallbackTimeInMs;    //  The timestamp in millisecond of last preview callback.
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Image Buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Definitions.
                                    struct ImgBufNode
                                    {
                                        sp<ClientImgBuf>   mpImgBuf;
                                        //
                                        ImgBufNode(sp<ClientImgBuf>const& pImgBuf = NULL)
                                            : mpImgBuf(pImgBuf)
                                        {}
                                        //
                                        sp<ClientImgBuf>const& getImgBuf() const   { return mpImgBuf; }
                                        sp<ClientImgBuf>&      getImgBuf()         { return mpImgBuf; }
                                    };

    typedef List<ImgBufNode>        ImgBufList_t;
    enum                            { eMAX_BUFFER_NUM = 3 };
    //
protected:  ////                    Data Members.
    uint_t                          muImgBufIdx;            //  index for ring buffer of mpImgBufMgr.
    sp<ImgBufManager>               mpImgBufMgr;
    ImgBufList_t                    mImgBufList;
    sp<IImgBufQueue>                mpImgBufQueue;
    //  Pointer to the client of Image Buffer Provider (i.e. a client is a provider-IF user of mpImgBufQueue).
    sp<IImgBufProviderClient>       mpImgBufPvdrClient;

protected:  ////                    Operations.
    //
    bool                            initBuffers();
    void                            uninitBuffers();
    //
    bool                            prepareAllTodoBuffers(sp<IImgBufQueue>const& rpBufQueue, sp<ImgBufManager>const& rpBufMgr);
    bool                            cancelAllUnreturnBuffers();
    bool                            waitAndHandleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue);
    bool                            handleReturnBuffers(Vector<ImgBufQueNode>const& rvQueNode);
    void                            performPreviewCallback(sp<ClientImgBuf>const& pImgBuf, int32_t const msgType);
    // gesture hal device
    virtual bool                    loadHALModule(mmsdk_module_t **module);
    virtual bool                    checkHALModuleVersion(mmsdk_module_t const *module);
    virtual bool                    openGestureDevice(hw_module_t *module, gesture_device_t **device);
    virtual bool                    closeGestureDevice(gesture_device_t  **device);
    virtual mmsdk_module_t*         getHALModule() {return mMMSdkModule;};
    virtual gesture_device_t*       getGestureDevice() {return mGestureDevice;};
    virtual void                    convertImgBufToIImageBuffer(sp<ClientImgBuf> const &pImgBuf, sp<NSCam::IImageBuffer> *pImageBuffer);
    virtual bool                    onGestureCallback(hand_detection_result_t const& result) const;
protected:
    //
    mmsdk_module_t                  *mMMSdkModule;
    gesture_device_t                *mGestureDevice;
    int32_t                         mMaxNumOfGestureDetected;
    GestureCallback_t           mGestureCb;
    void                              *mpGestureUser;

    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Duration Tool
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    ProfileTool                     mProfile_callback;
    ProfileTool                     mProfile_dequeProcessor;
    ProfileTool                     mProfile_buffer_timestamp;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Debug
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    logs
    int32_t                         miLogLevel; //0: silence, 1: debug1, 2: debug2

protected:  ////                    dump
    mutable Mutex                   mDumpMtx;
    List< Vector<uint8_t> >         mDumpImgBufList;
    //
    int32_t                         mi4DumpImgBufCount;
    int32_t                         mi4DumpImgBufIndex;
    String8                         ms8DumpImgBufPath;
    //
protected:  ////                    dump
    void                            dumpImgBuf_If(sp<ClientImgBuf>const& rpImgBuf);
};


}; // namespace NSGestureClient
}; // namespace NSSdkClient
}; // namespace android
#endif  //_MTK_HARDWARE_MTKCAM_V1_CLIENT_SDKCLIENT_GESTURE_H_

