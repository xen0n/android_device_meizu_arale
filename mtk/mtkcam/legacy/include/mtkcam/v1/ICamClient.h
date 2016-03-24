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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V1_ICAMCLIENT_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V1_ICAMCLIENT_H_
//
#include <v1/common.h>
#include "IParamsManager.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace android {


/******************************************************************************
 *  Camera Client Callback Interface.
 ******************************************************************************/
class ICamClient : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    static  android::sp<ICamClient> createInstance(android::sp<android::IParamsManager> pParamsMgr);

    virtual bool                    init()                                  = 0;
    virtual bool                    init(android::sp<IParamsManager> pParamsMgr)    = 0;
    virtual bool                    uninit()                                = 0;

    /**
     *
     */
    virtual bool                    setImgBufProviderClient(
                                        android::sp<android::MtkCamUtils::IImgBufProviderClient>const& rpClient
                                    )                                       = 0;

    /**
     * Set camera message-callback information.
     */
    virtual void                    setCallbacks(
                                        android::sp<CamMsgCbInfo> const& rpCamMsgCbInfo
                                    )                                       = 0;

    virtual void                    enableMsgType(int32_t msgType)          = 0;
    virtual void                    disableMsgType(int32_t msgType)         = 0;
    virtual bool                    msgTypeEnabled(int32_t msgType)         = 0;

    /**
     *
     */
    virtual bool                    startPreview()                          = 0;
    virtual void                    stopPreview()                           = 0;
    virtual bool                    previewEnabled()                        = 0;

    /**
     *
     */
    virtual void                    takePicture()                           = 0;


    /**
     * Request the camera hal to store meta data or real YUV data in
     * the video buffers send out via CAMERA_MSG_VIDEO_FRRAME for a
     * recording session. If it is not called, the default camera
     * hal behavior is to store real YUV data in the video buffers.
     *
     * This method should be called before startRecording() in order
     * to be effective.
     *
     * If meta data is stored in the video buffers, it is up to the
     * receiver of the video buffers to interpret the contents and
     * to find the actual frame data with the help of the meta data
     * in the buffer. How this is done is outside of the scope of
     * this method.
     *
     * Some camera hal may not support storing meta data in the video
     * buffers, but all camera hal should support storing real YUV data
     * in the video buffers. If the camera hal does not support storing
     * the meta data in the video buffers when it is requested to do
     * do, INVALID_OPERATION must be returned. It is very useful for
     * the camera hal to pass meta data rather than the actual frame
     * data directly to the video encoder, since the amount of the
     * uncompressed frame data can be very large if video size is large.
     *
     * @param enable if true to instruct the camera hal to store
     *      meta data in the video buffers; false to instruct
     *      the camera hal to store real YUV data in the video
     *      buffers.
     *
     * @return OK on success.
     */
    virtual status_t                storeMetaDataInBuffers(bool enable) { return INVALID_OPERATION; }

    /**
     * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
     * message is sent with the corresponding frame. Every record frame must be released
     * by a cameral hal client via releaseRecordingFrame() before the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
     * to manage the life-cycle of the video recording frames, and the client must
     * not modify/access any video recording frames.
     */
    virtual bool                    startRecording()                        = 0;

    /**
     * Stop a previously started recording.
     */
    virtual void                    stopRecording()                         = 0;

    /**
     * Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
     *
     * It is camera hal client's responsibility to release video recording
     * frames sent out by the camera hal before the camera hal receives
     * a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
     * the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
     * responsibility of managing the life-cycle of the video recording
     * frames.
     */
    virtual void                    releaseRecordingFrame(const void *opaque)   = 0;

    /**
     * Send command to camera driver.
     */
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)    = 0;

    /**
     *
     */
    virtual status_t                dump(int fd, android::Vector<android::String8>& args)   = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    //
    virtual                         ~ICamClient()  {}
};
};  //namespace android
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V1_ICAMCLIENT_H_

