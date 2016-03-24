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
#define LOG_TAG "mmsdk/IEffectHalClient"

#include <sys/types.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#include <mmsdk/IEffectHal.h>


#include <utils/List.h>


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
using namespace android;

enum
{
//public: // may change state
    INIT = IBinder::FIRST_CALL_TRANSACTION,
    UNINIT,
    CONFIGURE,
    UNCONFIGURE,
    START,
    ABORT,
//public: // would not change state
    GET_NAME_VERSION,
    SET_EFFECT_LISTENER,
    SET_PARAMETER,
    SET_PARAMETERS,
    GET_CAPTURE_REQUIREMENT,
    PREPARE,
    RELEASE,
//public: // for client
    //for buffer queue
    GET_INPUT_SURFACES,
    SET_OUTPUT_SURFACES,
    ADD_INPUT_PARAMETER,
    ADD_OUTPUT_PARAMETER,
    SET_INPUT_SYNC_MODE,
    GET_INPUT_SYNC_MODE,
    SET_OUTPUT_SYNC_MODE,
    GET_OUTPUT_SYNC_MODE,
    SET_BASEPARAMETER,  //21
    DEQUEUE_AND_QUEUE,
};


namespace
{
// Read empty strings without printing a false error message.
String16 readMaybeEmptyString16(const Parcel &parcel)
{
    size_t len;
    const char16_t *str = parcel.readString16Inplace(&len);
    if (str != NULL)
    {
        return String16(str, len);
    }
    else
    {
        return String16();
    }
}
};


class BpEffectHalClient : public BpInterface<IEffectHalClient>
{
public:
    BpEffectHalClient(const sp<IBinder> &impl)
        : BpInterface<IEffectHalClient>(impl)
    {
    }

public: // may change state
    virtual status_t   init()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(INIT, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }

        FUNCTION_LOG_END;
        return _result;
    }


    virtual status_t   uninit()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(UNINIT, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return _result;
    }


    virtual status_t   configure()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(CONFIGURE, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return _result;
    }


    virtual status_t   unconfigure()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());

        remote()->transact(UNCONFIGURE, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return _result;
    }


    virtual uint64_t   start()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(START, data, &reply);

        uint64_t uid = 0;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            uid = reply.readInt64();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return uid;
    }


    virtual status_t   abort(EffectParameter const *parameter)
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        if(parameter != NULL)
        {
            data.writeInt32(1);
            data.write(*parameter);
        }
        else
        {
            data.writeInt32(0);
        }
        remote()->transact(ABORT, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if(!exceptionCode)
        {
            _result = reply.readInt32();
        }
        FUNCTION_LOG_END;
        return _result;
    }


public: // would not change state
    virtual status_t   getNameVersion(EffectHalVersion &nameVersion) const
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(GET_NAME_VERSION, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
            if (reply.readInt32() != 0)
            {
                reply.read(nameVersion);
            }
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return _result;
    }


    virtual status_t   setEffectListener(const wp<IEffectListener> &listener)
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        sp<IEffectListener> spListener = listener.promote();
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        //data.writeStrongBinder((spListener!=NULL)?spListener->asBinder():NULL);
        data.writeStrongBinder((spListener!=NULL)?IInterface::asBinder(spListener):NULL);
        remote()->transact(SET_EFFECT_LISTENER, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }

        FUNCTION_LOG_END;
        return _result;
    }


    virtual status_t   setParameter(String8 &key, String8 &object)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        data.writeString8(key);
        data.writeString8(object);
        remote()->transact(SET_PARAMETER, data, &reply);
        FUNCTION_LOG_END;
        return reply.readInt32();
    }

    virtual status_t   setParameters(const sp<EffectParameter> parameter)
    {
        return OK;
    }

    virtual void   setStaticMetadata(sp <BasicParameters> staticMetadata)
    {
    }

    virtual status_t   getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(GET_CAPTURE_REQUIREMENT, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        // int32_t exceptionCode = reply.readExceptionCode();
        // if (!exceptionCode) {
        //     _result = reply.readInt32();
        //     if (reply.readInt32() != 0) {
        //         reply.read(requirement);
        //     }
        // } else {
        //     // An exception was thrown back; fall through to return failure
        //     ALOGE("caught exception %d\n", exceptionCode);
        // }

        FUNCTION_LOG_END;
        return _result;
    }


    //non-blocking
    virtual status_t   prepare()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(PREPARE, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return _result;
    }


    virtual status_t   release()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(RELEASE, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            _result = reply.readInt32();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return _result;
    }


    //non-blocking
    virtual status_t   addInputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter=NULL)
    {
        return OK;
    }


    //non-blocking
    virtual status_t   addOutputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter=NULL)
    {
        return OK;
    }
    //non-blocking
    virtual status_t   updateEffectRequest(const sp<EffectRequest> request)
    {
        return OK;
    }


public: //for buffer queue
    //
    virtual status_t   getInputSurfaces(Vector< sp<IGraphicBufferProducer> > &input)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        remote()->transact(GET_INPUT_SURFACES, data, &reply);

        status_t result = reply.readInt32();
        if (result == NO_ERROR)
        {
            size_t numSurfaces = static_cast<size_t>(reply.readInt32());
            input.clear();
            input.resize(numSurfaces);
            for (size_t c = 0; c < numSurfaces; ++c)
            {
                input.add(interface_cast<IGraphicBufferProducer>(reply.readStrongBinder()));
            }
        }
        FUNCTION_LOG_END;
        return result;
    }

    virtual status_t   setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &input, Vector<sp<EffectParameter> > &effectParam)
    {
        FUNCTION_LOG_START;
        FUNCTION_LOG_END;
        return OK;
    }

    //
    virtual status_t   addInputParameter(int index, sp<EffectParameter> &parameter, int64_t timestamp, bool repeat=false)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        data.writeInt32(index);

        //reference to IEffectUser.cpp apply function
        data.write(&parameter, sizeof(EffectParameter));
        data.writeInt64(timestamp);
        data.writeInt32(repeat ? 1 : 0);
        remote()->transact(ADD_INPUT_PARAMETER, data, &reply);
        \
        FUNCTION_LOG_END;
        return reply.readInt32();

    }

    virtual status_t   addOutputParameter(int index, EffectParameter &parameter, int64_t timestamp, bool repeat=false)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        data.writeInt32(index);
        //reference to IEffectUser.cpp apply function
        data.write(&parameter, sizeof(EffectParameter));
        data.writeInt64(timestamp);
        data.writeInt32(repeat ? 1 : 0);
        remote()->transact(ADD_OUTPUT_PARAMETER, data, &reply);
        FUNCTION_LOG_END;
        return reply.readInt32();

    }

    //
    virtual status_t   setInputsyncMode(int index, bool sync)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt32(sync ? 1 : 0);
        remote()->transact(SET_INPUT_SYNC_MODE, data, &reply);
        FUNCTION_LOG_END;
        return reply.readInt32();
    }

    virtual bool       getInputsyncMode(int index)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        data.writeInt32(index);
        remote()->transact(GET_INPUT_SYNC_MODE, data, &reply);
        bool sync = bool(data.readInt32());
        FUNCTION_LOG_END;
        return sync;
    }

    virtual status_t   setOutputsyncMode(int index, bool sync)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt32(sync ? 1 : 0);
        remote()->transact(SET_OUTPUT_SYNC_MODE, data, &reply);
        FUNCTION_LOG_END;
        return reply.readInt32();
    }

    virtual bool       getOutputsyncMode(int index)
    {
        //david add
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        data.writeInt32(index);
        remote()->transact(GET_OUTPUT_SYNC_MODE, data, &reply);
        bool sync = bool(data.readInt32());
        FUNCTION_LOG_END;
        return sync;
    }

    virtual status_t    setBaseParameter(BasicParameters const *parameter)
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectHalClient::getInterfaceDescriptor());
        if(parameter != NULL)
        {
            data.writeInt32(1);
            data.write(*parameter);
        }
        else
        {
            data.writeInt32(0);
        }
        remote()->transact(ABORT, data, &reply);

        status_t _result = UNKNOWN_ERROR;
        int32_t exceptionCode = reply.readExceptionCode();
        if(!exceptionCode)
        {
            _result = reply.readInt32();
        }
        FUNCTION_LOG_END;
        return _result;
    }

    virtual void       dequeueAndQueueBuf(int64_t timestamp)
    {
    }


public: //callback
    virtual void       onInputFrameAvailable()
    {
    }

    virtual void       onInputSurfacesChanged(EffectResult partialResult)
    {
    }
};


IMPLEMENT_META_INTERFACE(EffectHalClient, "com.mediatek.mmsdk.IEffectHalClient");


status_t BnEffectHalClient::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    FUNCTION_LOG_START;
    ALOGD("[%s] - code=%d, %d,  %d", __FUNCTION__, code, data.dataSize(), data.dataPosition());
    data.setDataPosition(0);
    switch(code)
    {
//public: // may change state
    case INIT:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        status_t _result = init();
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

    case UNINIT:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        status_t _result = uninit();
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

    case CONFIGURE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        status_t _result = configure();
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

    case UNCONFIGURE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        status_t _result = unconfigure();
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

    case START:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        uint64_t uid = start();
        reply->writeNoException();
        reply->writeInt64(uid);
        return OK;
    }
    break;

    case ABORT:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        EffectParameter parameter;
        if(data.readInt32() != 0)
        {
            data.read(parameter);
        }
        status_t _result = abort(&parameter);
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

//public: // would not change state
    case GET_NAME_VERSION:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        EffectHalVersion version;
        status_t _result = getNameVersion(version);
        reply->writeNoException();
        if(_result == OK)
        {
            reply->writeInt32(1);
            reply->write(version);
        }
        else
        {
            reply->writeInt32(0);
            ALOGD("GET_NAME_VERSION _result=%d", _result);
        }
        return OK;
    }
    break;

    case SET_EFFECT_LISTENER:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        sp<IEffectListener> listener = interface_cast<IEffectListener>(data.readStrongBinder());
        status_t _result = setEffectListener(listener);
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

    case SET_PARAMETER:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);

        String8 paramKey = String8(data.readString16());
        String8 paramVlaue = String8(data.readString16());
        //String8 paramKey = data.readString8();
        //String8 paramVlaue = data.readString8();
        ALOGD("SET_PARAMETER key=%s, value:%s", paramKey.string(), paramKey.string());
        //reply->writeInt32(setParameter(paramKey, reinterpret_cast<void*>(&value)));
        reply->writeNoException();
        reply->writeInt32(setParameter(paramKey, paramVlaue));
        return OK;
    }
    break;

#if 1
    case SET_PARAMETERS:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);

        sp<EffectParameter> parameter = new EffectParameter();
        int size = data.readInt32();
        if(size != 0)
        {
            const String8 param = String8(data.readString16());
            ALOGD("param string=%s", param.string());
            parameter->unflatten(param);
        }

        reply->writeNoException();
        reply->writeInt32(setParameters(parameter));
        return OK;
    }
    break;
# endif

    case GET_CAPTURE_REQUIREMENT:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        EffectParameter inputParam;
        Vector<EffectCaptureRequirement> requirements;

        int size = data.readInt32();
        if(size != 0)
        {
            const String8 param = String8(data.readString16());
            //ALOGD("GET_CAPTURE_REQUIREMENT, param=%s", param.string());
            inputParam.unflatten(param);
        }

        status_t result = getCaptureRequirement(&inputParam, requirements);


        reply->writeNoException();
        reply->writeInt32(result);
        reply->writeInt32(requirements.size());
        ALOGD("requirements size=%d", requirements.size());
        for (int i=0; i<requirements.size(); ++i)
        {
            reply->writeInt32(1);
            reply->write(requirements[i]);
        }
        return OK;
    }
    break;

    case PREPARE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        status_t _result = prepare();
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

    case RELEASE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        status_t _result = release();
        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;
//public: // for client
    case GET_INPUT_SURFACES:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        Vector<sp<IGraphicBufferProducer> > producers;
        status_t result = getInputSurfaces(producers);
        reply->writeNoException();
        reply->writeInt32(result);
        reply->writeInt32(producers.size());
        for (int i=0; i<producers.size(); ++i)
        {
            sp<IGraphicBufferProducer> producer = producers.itemAt(i);
            if (producer != 0)
            {
                ALOGD("GET_INPUT_SURFACES, index=%d, address=%p", i, producer.get());
                reply->writeInt32(1);  // marker that surface is not null

                char str[32];
                sprintf(str, "IEffectHalClient-producer-%d", i);
                reply->writeString16(String16(str)); // name of surface
                //reply->writeStrongBinder(producer->asBinder());
                reply->writeStrongBinder(IInterface::asBinder(producer));
            }
            else
            {
                ALOGE("%s: GET_OUTPUT_SURFACES: surface = unset, name = unset", __FUNCTION__);
                reply->writeInt32(0);
            }
        }
        return OK;
    }
    break;

    case SET_OUTPUT_SURFACES:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        //get vector size
        int size = data.readInt32();
        if(size == -1)
        {
            ALOGD("List is null, size=%d", size);
            return OK;
        }

        Vector<sp<IGraphicBufferProducer> > outputSurfaces;
        int num=0;
        //get surface
        while(num<size)
        {
            String16 name;
            int is_surface = data.readInt32();
            //check each surface is null or not
            if(is_surface != 0)
            {
                // String16 name = readMaybeEmptyString16(data);
                size_t len;
                const char16_t* str = data.readString16Inplace(&len);
                if (str != NULL)
                {
                    name = String16(str, len);
                } else
                {
                    name = String16();
                }
                ALOGD("SET_OUTPUT_SURFACES name=%s", String8(name).string());

                //convert to IGraphicBufferProducer pointer and push_back to outputSurfaces
                sp<IGraphicBufferProducer> surface = interface_cast<IGraphicBufferProducer>(data.readStrongBinder());
                if (surface == NULL)
                {
                    ALOGE("%s: SET_OUTPUT_SURFACES: surface = unset, name= %s", __FUNCTION__, String8(name).string());
                }
                outputSurfaces.push_back(surface);
            }
            else
            {
                ALOGE("%s: SET_OUTPUT_SURFACES: surface = unset, name = unset", __FUNCTION__);
            }
            num++;
        }

        //get effectParameter
        Vector<sp<EffectParameter> > parameters;
        num=0;
        size = data.readInt32();
        if(size == -1)
        {
            ALOGD("EffectParameter List is null, size=%d", size);
            return OK;
        }
        while(num<size)
        {
            sp<EffectParameter> parameter = new EffectParameter();
            //EffectParameter parameter;
            int size = data.readInt32();
            if(size != 0)
            {
                const String8 param = String8(data.readString16());
                ALOGD("[%s]: param string=%s", __FUNCTION__, param.string());
                parameter->unflatten(param);
            }
            parameters.push_back(parameter);
            num++;
        }
        status_t _result;
        //check the amount of surfaces and parameters are same
        if (parameters.size() != outputSurfaces.size())
        {
            ALOGE("[%s]: parameters.size(%d) not equal to outputSurfaces.size(%d)",
                __FUNCTION__, parameters.size(), outputSurfaces.size());
            _result = BAD_VALUE;
        }
        else
        {
            _result = setOutputSurfaces(outputSurfaces, parameters);
        }

        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;

    case ADD_INPUT_PARAMETER:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        sp<EffectParameter> parameter = new EffectParameter();
        int index = data.readInt32();
        int size = data.readInt32();
        if(size != 0)
        {
            const String8 param = String8(data.readString16());
            //ALOGD("params=%s", param.string());
            parameter->unflatten(param);
        }
        int64_t timestamp = data.readInt64();
        int repeat = data.readInt32();
        ALOGD("repeat=%d",repeat);
        status_t result = addInputParameter(index, parameter, timestamp, bool(repeat));
        reply->writeInt32(result);
        reply->writeNoException();
        return OK;
    }
    break;

    case ADD_OUTPUT_PARAMETER:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        EffectParameter parameter;

        int index = data.readInt32();
        int size = data.readInt32();
        if(size != 0)
        {
            const String8 param = String8(data.readString16());
            parameter.unflatten(param);
        }
        int64_t timestamp = data.readInt64();
        int repeat = data.readInt32();
        status_t result = addOutputParameter(index, parameter, timestamp, bool(repeat));
        reply->writeInt32(result);
        reply->writeNoException();
        return OK;
    }
    break;

    case SET_INPUT_SYNC_MODE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        int index = data.readInt32();
        bool sync = bool(data.readInt32());
        status_t result = setInputsyncMode(index, sync);
        reply->writeNoException();
        reply->writeInt32(result);
        return OK;

    }
    break;

    case GET_INPUT_SYNC_MODE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        int index = data.readInt32();
        bool sync = getInputsyncMode(index);
        reply->writeNoException();
        reply->writeInt32(sync ? 1 : 0);
        return sync;
    }
    break;

    case SET_OUTPUT_SYNC_MODE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        int index = data.readInt32();
        bool sync = bool(data.readInt32());
        status_t result = setOutputsyncMode(index, sync);
        reply->writeNoException();
        reply->writeInt32(result);
        return OK;
    }
    break;

    case GET_OUTPUT_SYNC_MODE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        int index = data.readInt32();
        bool sync = getOutputsyncMode(index);
        reply->writeNoException();
        reply->writeInt32(sync ? 1 : 0);
        return sync;
    }
    break;

    case SET_BASEPARAMETER:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        BasicParameters parameter;
        int size = data.readInt32();
        if(size != 0)
        {
            //ALOGD("SET_BASEPARAMETER, size=%d, dataPosition=%d", size, data.dataPosition());
            const String8 param = String8(data.readString16());
            ALOGD("SET_BASEPARAMETER, param=%s", param.string());
            parameter.unflatten(param);
        }
        reply->writeNoException();
        reply->writeInt32(OK);
        return OK;
    }
    break;

    case DEQUEUE_AND_QUEUE:
    {
        CHECK_INTERFACE(IEffectHal, data, reply);
        int64_t timestamp = data.readInt64();
        dequeueAndQueueBuf(timestamp);

        reply->writeNoException();
        reply->writeInt32(OK);
        return OK;
    }
    break;

    }
    FUNCTION_LOG_END;
    return BBinder::onTransact(code, data, reply, flags);
}

