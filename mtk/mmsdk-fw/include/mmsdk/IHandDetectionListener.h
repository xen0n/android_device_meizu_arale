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

#ifndef _MEDIATEK_MMSDK_IHAND_DETECTION_LISTENER_H_
#define _MEDIATEK_MMSDK_IHAND_DETECTION_LISTENER_H_

#include <binder/IInterface.h>
#include <stdint.h>
#include <utils/RefBase.h>
#include <ui/Rect.h>

namespace android {

class Parcel;

namespace NSMMSdk {
namespace NSGesture {

struct HandDetectionEvent
{
    Rect                   boundBox;
    float                   confidence;
    int                      id;
    int                      pose;

public:
    HandDetectionEvent()
    :  boundBox(Rect())
    ,  confidence(0.0)
    ,  id(0)
    ,  pose(0)
    {};

    HandDetectionEvent(
        Rect _boundBox,
        float _confidence,
        int   _id,
        int   _pose
    )
    : boundBox(_boundBox)
    , confidence(_confidence)
    , id(_id)
    , pose(_pose)
    {};
};



class IHandDetectionListener: public IInterface
{
public:
    DECLARE_META_INTERFACE(HandDetectionListener);

    virtual void onHandDetected(HandDetectionEvent const &event) = 0;
};

// ----------------------------------------------------------------------------

class BnHandDetectionListener: public BnInterface<IHandDetectionListener>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSGesture
}; // namespace NSMMSdk
}; // namespace android

#endif
