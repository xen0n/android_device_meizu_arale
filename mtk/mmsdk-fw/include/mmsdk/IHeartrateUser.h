/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef _MEDIATEK_MMSDK_IHEARTRATE_USER_H_
#define _MEDIATEK_MMSDK_IHEARTRATE_USER_H_

#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {
namespace NSMMSdk {
namespace NSHeartrate {

typedef enum {
    MMSDK_HRD_Mode_Adult = 0,
    MMSDK_HRD_Mode_Baby,
    MMSDK_HRD_Mode_Adult_Finger,
    MMSDK_HRD_Mode_Baby_Finger,
    MMSDK_HRD_Mode_Unknown
} MMSDK_HRD_Mode_T;


class IHeartrateDetectionListener;
//
class IHeartrateUser : public IInterface
{
public:
    DECLARE_META_INTERFACE(HeartrateUser);

    // disconnect from mmsdk service
    virtual void            disconnect() = 0;

	virtual bool            start() = 0;

	virtual void            stop() = 0;

	virtual void            query(int *HasCamDev, int *IsHRStart) = 0;

	virtual void            setmode(int mode) = 0;

    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    virtual bool            addHeartrateDetectionListener(const sp<IHeartrateDetectionListener> &listener,
                                                     int const arg) = 0;
    //
    virtual bool            removeHeartrateDetectionListener(const sp<IHeartrateDetectionListener> &listener,
                                                        int const arg) = 0;
};

// ----------------------------------------------------------------------------

class BnHeartrateUser: public BnInterface<IHeartrateUser>
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

#endif   // _MEDIATEK_MMSDK_IHEARTRATE_USER_H_
