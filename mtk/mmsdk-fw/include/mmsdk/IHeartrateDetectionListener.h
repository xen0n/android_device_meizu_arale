/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MEDIATEK_MMSDK_IHEARTRATE_DETECTION_LISTENER_H_
#define _MEDIATEK_MMSDK_IHEARTRATE_DETECTION_LISTENER_H_

#include <binder/IInterface.h>
#include <stdint.h>
#include <utils/RefBase.h>
#include <ui/Rect.h>

namespace android {

class Parcel;

namespace NSMMSdk {
namespace NSHeartrate {

struct HeartrateDetectionEvent
{
    Rect                   boundBox;
    int                    confidence;
    int                    facenum;
    int                    heartbeats;
    int                    percentage;
    int                    stoptype;
    int                    replytype;
    int                    waveform[256];
    int                    prev_w;
    int                    prev_h;
    int                    facing;

public:
    HeartrateDetectionEvent()
    :  boundBox(Rect())
    ,  confidence(0.0)
    ,  facenum(0)
    ,  heartbeats(0)
    {};

    HeartrateDetectionEvent(
        Rect _boundBox,
        float _confidence,
        int   _facenum,
        int   _heartbeats
    )
    : boundBox(_boundBox)
    , confidence(_confidence)
    , facenum(_facenum)
    , heartbeats(_heartbeats)
    {};
};

typedef enum {
    MMSDK_HRD_ReplyType_Result = 1,
    MMSDK_HRD_ReplyType_CamConnect = 2,
    MMSDK_HRD_ReplyType_CamDisconnect = 4,
    MMSDK_HRD_ReplyType_CamNotSupport = 8
} MMSDK_HRD_ReplyType_T;

typedef enum {
    MMSDK_HRD_StopType_Normal = 0,
    MMSDK_HRD_StopType_Large_Motion,
    MMSDK_HRD_StopType_Large_SAD,
    MMSDK_HRD_StopType_Bad_Quality,
    MMSDK_HRD_StopType_Other,
} MMSDK_HRD_StopType_T;

typedef enum {
    MMSDK_HRD_State_Stop = 0,
    MMSDK_HRD_State_Need_ROI,
    MMSDK_HRD_State_Estimation
} MMSDK_HRD_State_T;


class IHeartrateDetectionListener: public IInterface
{
public:
    DECLARE_META_INTERFACE(HeartrateDetectionListener);

    virtual void onHeartrateDetected(HeartrateDetectionEvent const &event) = 0;
};

// ----------------------------------------------------------------------------

class BnHeartrateDetectionListener: public BnInterface<IHeartrateDetectionListener>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSHeartrate
}; // namespace NSMMSdk
}; // namespace android

#endif
