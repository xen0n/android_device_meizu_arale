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

#define LOG_TAG "mmsdk/EffectListener"
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

#define USE_TWO_WAY             0


using namespace NSCam;
using namespace android;

enum {
    ON_PREPARED = IBinder::FIRST_CALL_TRANSACTION,
    ON_INPUT_FRAME_PROCESSED,
    ON_OUTPUT_FRAME_PROCESSED,
    ON_COMPLETED,
    ON_ABORTED,
    ON_FAILED,
};


class BpEffectListener : public BpInterface<IEffectListener>
{
public:
    BpEffectListener(const sp<IBinder>& impl)
        : BpInterface<IEffectListener>(impl)
    {
    }

public:
    //TTT4 virtual void    onPrepared(const sp<IEffectHal> effect, const EffectResult& result) const
    virtual void    onPrepared(const IEffectHalClient* effectClient, const EffectResult& result) const
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());
        //data.writeStrongBinder((effectClient!=0)?const_cast<IEffectHalClient*>(effectClient)->asBinder():0);
        data.writeStrongBinder((effectClient!=0)?IInterface::asBinder(const_cast<IEffectHalClient*>(effectClient)):0);

        data.writeInt32(1);
        data.write(result);
        remote()->transact(ON_PREPARED, data, &reply);

        #if 0   //USE_TWO_WAY
        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (exceptionCode) {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        #endif

        FUNCTION_LOG_END;
    }


    //TTT4 virtual void    onInputFrameProcessed(const sp<IEffectHal> effect, const sp <EffectParameter> parameter, EffectResult partialResult) const
    virtual void    onInputFrameProcessed(const IEffectHalClient* effectClient, const sp <EffectParameter> parameter, EffectResult partialResult)
    {
        //#if USE_TWO_WAY
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());

        if (effectClient==0)
        {
            ALOGD("[%s] effectClient is null\n", __FUNCTION__);
        }
        //data.writeStrongBinder((effectClient!=0)?const_cast<IEffectHalClient*>(effectClient)->asBinder():0);
        data.writeStrongBinder((effectClient!=0)?IInterface::asBinder(const_cast<IEffectHalClient*>(effectClient)):0);

        if (parameter == 0)
        {
            ALOGD("[%s] EffectParameter is null\n", __FUNCTION__);
            data.writeInt32(0);
        }
        else
        {
            ALOGD("[%s] EffectParameter is not null\n", __FUNCTION__);
            data.writeInt32(1);
            data.write(*(parameter.get()));
        }

        data.writeInt32(1);
        data.write(partialResult);
        remote()->transact(ON_INPUT_FRAME_PROCESSED, data, &reply);

        FUNCTION_LOG_END;

        // #else
        // FUNCTION_LOG_START;
        // //@todo implement this
        // FUNCTION_LOG_END;
        // #endif
    }


    //TTT4 virtual void    onOutputFrameProcessed(const sp<IEffectHal> effect, const sp<EffectParameter> parameter, EffectResult partialResult) const
    virtual void    onOutputFrameProcessed(const IEffectHalClient* effectClient, const sp<EffectParameter> parameter, EffectResult partialResult)
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());
        //data.writeStrongBinder((effectClient!=0)?const_cast<IEffectHalClient*>(effectClient)->asBinder():0);
        data.writeStrongBinder((effectClient!=0)?IInterface::asBinder(const_cast<IEffectHalClient*>(effectClient)):0);

        data.writeInt32(1);
        data.write(*(parameter.get()));

        data.writeInt32(1);
        data.write(partialResult);

        remote()->transact(ON_OUTPUT_FRAME_PROCESSED, data, &reply);
        FUNCTION_LOG_END;



        /*#if USE_TWO_WAY
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());
        data.writeStrongBinder((effectClient!=0)?const_cast<IEffectHalClient*>(effectClient)->asBinder():0);
        remote()->transact(ON_OUTPUT_FRAME_PROCESSED, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode) {
            //@todo implement this
            //if(reply.readInt32() != 0)
                //reply.read(*parameter);
            if(reply.readInt32() != 0)
                reply.read(partialResult);
        } else {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }

        FUNCTION_LOG_END;
        #else
        FUNCTION_LOG_START;
        //@todo implement this
        FUNCTION_LOG_END;
        #endif*/
    }


    //TTT4 virtual void    onCompleted(const sp<IEffectHal> effect, const EffectResult& partialResult, uint64_t uid) const
    virtual void    onCompleted(const IEffectHalClient* effectClient, const EffectResult& partialResult, uint64_t uid) const
    {
        #if 0   //USE_TWO_WAY
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());
        data.writeStrongBinder((effect!=0)?const_cast<IEffectHal*>(effect)->asBinder():0);
        data.writeInt64(uid);
        remote()->transact(ON_COMPLETED, data, &reply);

        //status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode) {
            if(reply.readInt32() != 0)
                reply.read(partialResult);
        } else {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }

        FUNCTION_LOG_END;
        #else
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());
        data.writeStrongBinder((effectClient!=0)?IInterface::asBinder(const_cast<IEffectHalClient*>(effectClient)):0);

        data.writeInt32(1);
        data.write(partialResult);

        ALOGD("[%s] uid=%d \n", __FUNCTION__, uid);
        data.writeInt64(uid);
        remote()->transact(ON_COMPLETED, data, &reply);

        #if 0   //USE_TWO_WAY
        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (exceptionCode) {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        #endif

        FUNCTION_LOG_END;
        #endif
    }


    //TTT4 virtual void    onAborted(const sp<IEffectHal> effect, const EffectResult& result) const
    virtual void    onAborted(const IEffectHalClient* effectClient, const EffectResult& result)
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());
        //data.writeStrongBinder((effectClient!=0)?const_cast<IEffectHalClient*>(effectClient)->asBinder():0);
        data.writeStrongBinder((effectClient!=0)?IInterface::asBinder(const_cast<IEffectHalClient*>(effectClient)):0);

        data.writeInt32(1);
        data.write(result);
        remote()->transact(ON_ABORTED, data, &reply);

        #if 0   //USE_TWO_WAY
        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (exceptionCode) {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        #endif

        FUNCTION_LOG_END;
    }


    //TTT4 virtual void    onFailed(const sp<IEffectHal> effect, const EffectResult& result) const
    virtual void    onFailed(const IEffectHalClient* effectClient, const EffectResult& result) const
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectListener::getInterfaceDescriptor());
        //data.writeStrongBinder((effectClient!=0)?const_cast<IEffectHalClient*>(effectClient)->asBinder():0);
        data.writeStrongBinder((effectClient!=0)?IInterface::asBinder(const_cast<IEffectHalClient*>(effectClient)):0);

        data.writeInt32(1);
        data.write(result);
        remote()->transact(ON_FAILED, data, &reply);

        #if 0   //USE_TWO_WAY
        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (exceptionCode) {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        #endif

        FUNCTION_LOG_END;
    }
};


IMPLEMENT_META_INTERFACE(EffectListener, "com.mediatek.mmsdk.IEffectListener");


status_t BnEffectListener::onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    FUNCTION_LOG_START;
    ALOGD("[%s] - code=%d", __FUNCTION__, code);
    switch(code) {
        case ON_PREPARED: {
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectResult _arg1;

            if(data.readInt32() != 0)
                data.read(_arg1);

            //TTT4 onPrepared(_arg0, _arg1);
            onPrepared(_arg0.get(), _arg1);

            #if 0   //USE_TWO_WAY
            reply->writeNoException();
            #endif
            return OK;
        } break;

        case ON_INPUT_FRAME_PROCESSED: {
            #if USE_TWO_WAY
            //    virtual void    onInputFrameProcessed(const sp<IEffectHalClient> effect, EffectParameter const *parameter, EffectResult partialResult) const
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectParameter _arg1;
            EffectResult _arg2;
            //TTT4 onInputFrameProcessed(_arg0, &_arg1, _arg2);
            onInputFrameProcessed(_arg0.get(), &_arg1, _arg2);

            reply->writeNoException();
            if(1) {//if(_result == OK) {
                reply->writeInt32(1);
                reply->write(_arg1);
            } else {
                reply->writeInt32(0);
            }
            return OK;
            #else   //one way
            //    virtual void    onInputFrameProcessed(const sp<IEffectHalClient> effect, EffectParameter const *parameter, EffectResult partialResult) const
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectParameter _arg1;
            EffectResult _arg2;
            if(data.readInt32() != 0)
                data.read(_arg1);
            if(data.readInt32() != 0)
                data.read(_arg2);
            #if 0   //TTT3
            onInputFrameProcessed(_arg0, _arg1, _arg2);
            #endif
            return OK;
            #endif
        } break;

        case ON_OUTPUT_FRAME_PROCESSED: {
            #if USE_TWO_WAY
            //    virtual void    onOutputFrameProcessed(const android::sp<IEffectHalClient> effect, EffectParameter const *parameter, EffectResult partialResult) const = 0;
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectParameter _arg1;
            EffectResult _arg2;
            //TTT4 onOutputFrameProcessed(_arg0, &_arg1, _arg2);
            onOutputFrameProcessed(_arg0.get(), &_arg1, _arg2);

            reply->writeNoException();
            if(1) {//if(_result == OK) {
                reply->writeInt32(1);
                reply->write(_arg1);
            } else {
                reply->writeInt32(0);
            }
            return OK;
            #else
            //    virtual void    onOutputFrameProcessed(const android::sp<IEffectHalClient> effect, EffectParameter const *parameter, EffectResult partialResult) const = 0;
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectParameter _arg1;
            EffectResult _arg2;
            if(data.readInt32() != 0)
                data.read(_arg1);
            if(data.readInt32() != 0)
                data.read(_arg2);
            #if 0   //TTT3
            onOutputFrameProcessed(_arg0, _arg1, _arg2);
            #endif
            return OK;
            #endif
        } break;

        case ON_COMPLETED: {
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectResult _arg1;
            if(data.readInt32() != 0)
                data.read(_arg1);
            uint64_t _arg2 = data.readInt64();
            //TTT4 onCompleted(_arg0, _arg1, _arg2);    //TTT3
            onCompleted(_arg0.get(), _arg1, _arg2);    //TTT3

            #if 0   //USE_TWO_WAY
            reply->writeNoException();
            #endif

            return OK;
        } break;

        case ON_ABORTED: {
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectResult _arg1;

            if(data.readInt32() != 0)
                data.read(_arg1);

            //TTT4 onAborted(_arg0, _arg1);
            onAborted(_arg0.get(), _arg1);

            #if 0   //USE_TWO_WAY
            reply->writeNoException();
            #endif
            return OK;
        } break;

        case ON_FAILED: {
            CHECK_INTERFACE(IEffectListener, data, reply);
            sp<IEffectHalClient> _arg0 = interface_cast<IEffectHalClient>(data.readStrongBinder());
            EffectResult _arg1;

            if(data.readInt32() != 0)
                data.read(_arg1);

            //TTT4 onFailed(_arg0, _arg1);
            onFailed(_arg0.get(), _arg1);

            #if 0   //USE_TWO_WAY
            reply->writeNoException();
            #endif
            return OK;
        } break;

    }
    FUNCTION_LOG_END;
    return BBinder::onTransact(code, data, reply, flags);
}

