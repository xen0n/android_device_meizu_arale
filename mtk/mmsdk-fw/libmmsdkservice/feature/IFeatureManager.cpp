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

//#include <cutils/xlog.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <gui/IGraphicBufferProducer.h>
#include <mmsdk/IEffectHal.h>


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
using namespace android;


enum {
    SET_PARAMETER = IBinder::FIRST_CALL_TRANSACTION,
    GET_PARAMETER,
    SET_UP,
    TEAR_DOWN,
    GET_EFFECT_FACTORY,
};


class BpFeatureManager : public BpInterface<IFeatureManager>
{
public:
    BpFeatureManager(const sp<IBinder>& impl)
        : BpInterface<IFeatureManager>(impl)
        , mpEffectFactory(0)
    {
        //@todo implement this - use dlopen
        //mpEffectFactory = new EffectFactory();
    }


    virtual ~BpFeatureManager()
    {
        mpEffectFactory = NULL;
    }


    virtual status_t setParameter(android::String8 key, android::String8 value)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IFeatureManager::getInterfaceDescriptor());
        data.writeString8(key);
        data.writeString8(value);
        status_t result = remote()->transact(SET_PARAMETER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        return reply.readInt32();
    }


    virtual android::String8 getParameter(android::String8 key)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IFeatureManager::getInterfaceDescriptor());
        data.writeString8(key);

        String8 value;
        data.writeString8(value);

        status_t result = remote()->transact(GET_PARAMETER, data, &reply);

        #if 0   //@todo check error
        if (result != NO_ERROR) {
            return result;
        }
        #endif
        value = reply.readString8();
        //return reply.readInt32();
        return value;
    }


    virtual status_t setUp(EffectHalVersion const &nameVersion)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IFeatureManager::getInterfaceDescriptor());
        data.writeString8(nameVersion.effectName);
        data.writeInt32(nameVersion.major);
        data.writeInt32(nameVersion.minor);

        status_t result = remote()->transact(SET_UP, data, &reply);

        if (result != NO_ERROR) {
            return result;
        }
        return reply.readInt32();
    }


    virtual status_t tearDown(EffectHalVersion const &nameVersion)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IFeatureManager::getInterfaceDescriptor());
        data.writeString8(nameVersion.effectName);
        data.writeInt32(nameVersion.major);
        data.writeInt32(nameVersion.minor);

        status_t result = remote()->transact(TEAR_DOWN, data, &reply);

        if (result != NO_ERROR) {
            return result;
        }
        return reply.readInt32();
    }


    virtual status_t getEffectFactory(sp<IEffectFactory> &effectFactory)
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IFeatureManager::getInterfaceDescriptor());
        remote()->transact(GET_EFFECT_FACTORY, data, &reply);

        if (reply.readExceptionCode()) return -EPROTO;
        status_t status = reply.readInt32();
        if (reply.readInt32() != 0)
        {
            effectFactory = interface_cast<NSCam::IEffectFactory>(reply.readStrongBinder());
        }

        FUNCTION_LOG_END;
        return status;
    }

private:
    sp<IEffectFactory>   mpEffectFactory;
};

IMPLEMENT_META_INTERFACE(FeatureManager, "com.mediatek.mmsdk.IFeatureManager");


// ----------------------------------------------------------------------

status_t BnFeatureManager::onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    FUNCTION_LOG_START;
    ALOGD("[%s] - code=%d", __FUNCTION__, code);
    switch(code) {
        case SET_PARAMETER: {
            CHECK_INTERFACE(IFeatureManager, data, reply);

            String8 key = data.readString8();
            String8 value = data.readString8();
            ALOGD("key[%s] - value=%s", key.string(), value.string());
            status_t result = setParameter(key, value);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;

        case GET_PARAMETER: {
            CHECK_INTERFACE(IFeatureManager, data, reply);

            #if 0   //@todo implement this
            String8 key = data.readString8();
            String8 value = data.readString8();

            status_t result = getParameter(key, value);
            status_t err = reply->writeString8(value);
            if(err) return err;
            reply->writeInt32(result);
            #endif
            return NO_ERROR;
        } break;

        case SET_UP: {
            CHECK_INTERFACE(IFeatureManager, data, reply);

            EffectHalVersion version;
            version.effectName = data.readString8();
            version.major = data.readInt32();
            version.minor = data.readInt32();

            status_t result = setUp(version);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;

        case TEAR_DOWN: {
            CHECK_INTERFACE(IFeatureManager, data, reply);

            EffectHalVersion version;
            version.effectName = data.readString8();
            version.major = data.readInt32();
            version.minor = data.readInt32();

            status_t result = tearDown(version);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;

        case GET_EFFECT_FACTORY: {
            CHECK_INTERFACE(IFeatureManager, data, reply);

            sp<NSCam::IEffectFactory> client;
            status_t status = getEffectFactory(client);
            reply->writeNoException();
            reply->writeInt32(status);
            if (client != 0)
            {
                reply->writeInt32(1);
                //reply->writeStrongBinder(client->asBinder());
                reply->writeStrongBinder(IInterface::asBinder(client));
            }
            else
            {
                reply->writeInt32(0);
            }
            return NO_ERROR;
        } break;
    }
    FUNCTION_LOG_END;
    return BBinder::onTransact(code, data, reply, flags);
}

