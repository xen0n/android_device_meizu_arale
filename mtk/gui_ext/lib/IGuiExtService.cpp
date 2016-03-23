#define LOG_TAG "GuiExt"

#define MTK_LOG_ENABLE 1
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/Timers.h>
#include <utils/String8.h>

#include <binder/Parcel.h>
#include <binder/IInterface.h>

#include <cutils/log.h>

#include <ui/GraphicBuffer.h>
#include <IDumpTunnel.h>

#include "IGuiExtService.h"

namespace android {

// client : proxy GuiEx class
class BpGuiExtService : public BpInterface<IGuiExtService>
{
public:
    BpGuiExtService(const sp<IBinder>& impl) : BpInterface<IGuiExtService>(impl)
    {
    }

    virtual status_t alloc(const sp<IBinder>& token, uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t *id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeStrongBinder(token);
        data.writeInt32(gralloc_usage);
        data.writeInt32(w);
        data.writeInt32(h);
        status_t result = remote()->transact(GUI_EXT_ALLOC, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("alloc could not contact remote\n");
            return result;
        }
        *id = reply.readInt32();
        result = reply.readInt32();
        return result;
    }

    virtual status_t free(uint32_t id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeInt32(id);
        status_t result = remote()->transact(GUI_EXT_FREE, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("free could not contact remote\n");
            return -1;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t acquire(const sp<IBinder>& token, uint32_t poolId, uint32_t usage, uint32_t type, int *buf)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeStrongBinder(token);
        data.writeInt32(poolId);
        data.writeInt32(usage);
        data.writeInt32(type);
        status_t result = remote()->transact(GUI_EXT_ACQUIRE, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("acquire could not contact remote\n");
            return result;
        }
        *buf = reply.readInt32();
        result = reply.readInt32();
        return result;
    }

    virtual status_t request(uint32_t poolId, uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeInt32(poolId);
        data.writeInt32(usage);
        data.writeInt32(type);
        data.writeInt32(buf);
        status_t result =remote()->transact(GUI_EXT_REQUEST, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("request could not contact remote\n");
            return result;
        }
        bool nonNull = reply.readInt32();
        if (nonNull) {
            *buffer = new GraphicBuffer();
            result = reply.read(**buffer);
            if(result != NO_ERROR) {
                (*buffer).clear();
                return result;
            }
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t release(uint32_t poolId, uint32_t usage, uint32_t type, int buf)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeInt32(poolId);
        data.writeInt32(usage);
        data.writeInt32(type);
        data.writeInt32(buf);
        status_t result = remote()->transact(GUI_EXT_RELEASE, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("release could not contact remote\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t disconnect(uint32_t poolId, uint32_t usage, uint32_t type)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeInt32(poolId);
        data.writeInt32(usage);
        data.writeInt32(type);
        status_t result = remote()->transact(GUI_EXT_DISCONNECT, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("disconnect could not contact remote\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeInt32(type);
        data.writeInt32(enable);
        data.writeInt32(w);
        data.writeInt32(h);
        data.writeInt32(bufNum);
        status_t result = remote()->transact(GUI_EXT_CONFIGDISPLAY, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("disconnect could not contact remote\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }
    virtual status_t regDump(const sp<IDumpTunnel>& tunnel, const String8& key)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeStrongBinder(tunnel->asBinder(tunnel));
        data.writeString8(key);
        remote()->transact(GUI_EXT_REGDUMP, data, &reply);
        return reply.readInt32();
    }

    virtual status_t unregDump(const String8& key)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeString8(key);
        remote()->transact(GUI_EXT_UNREGDUMP, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(GuiExtService, "GuiExtService");

status_t BnGuiExtService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    //ALOGD("receieve the command code %d", code);

    switch(code)
    {
        case GUI_EXT_ALLOC:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            sp<IBinder> token = data.readStrongBinder();
            int gralloc_usage   = data.readInt32();
            int w = data.readInt32();
            int h = data.readInt32();
            uint32_t id;
            status_t ret = alloc(token, gralloc_usage, w, h, &id);
            reply->writeInt32(id);
            reply->writeInt32(ret);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_FREE:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            int id          = data.readInt32();
            status_t ret = free(id);
            reply->writeInt32(ret);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_ACQUIRE:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            sp<IBinder> token = data.readStrongBinder();
            uint32_t poolId = data.readInt32();
            uint32_t usage = data.readInt32();
            uint32_t type = data.readInt32();
            int buf;
            int result = acquire(token, poolId, usage, type, &buf);
            reply->writeInt32(buf);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_REQUEST:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            uint32_t poolId = data.readInt32();
            uint32_t usage = data.readInt32();
            uint32_t type = data.readInt32();
            uint32_t buf = data.readInt32();
            sp<GraphicBuffer> buffer;
            int result = request(poolId, usage, type, buf, &buffer);
            reply->writeInt32(buffer != 0);
            if (buffer != 0) {
                reply->write(*buffer);
            }
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_RELEASE:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            uint32_t poolId = data.readInt32();
            uint32_t usage = data.readInt32();
            uint32_t type = data.readInt32();
            uint32_t buf = data.readInt32();
            int result = release(poolId, usage, type, buf);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_DISCONNECT:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            uint32_t poolId = data.readInt32();
            uint32_t usage = data.readInt32();
            uint32_t type = data.readInt32();
            int result = disconnect(poolId, usage, type);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_CONFIGDISPLAY:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            uint32_t type = data.readInt32();
            bool enable = data.readInt32();
            uint32_t w = data.readInt32();
            uint32_t h = data.readInt32();
            uint32_t bufNum = data.readInt32();
            int result = configDisplay(type, enable, w, h, bufNum);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_REGDUMP:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            sp<IDumpTunnel> tunnel = interface_cast<IDumpTunnel>(data.readStrongBinder());
            String8 key = data.readString8();
            status_t result = regDump(tunnel, key);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_UNREGDUMP:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            String8 key = data.readString8();
            status_t result = unregDump(key);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
    }
    return BBinder::onTransact(code, data, reply, flags);
}

sp<IGuiExtService> checkGuiExtService()
{
    const sp<IServiceManager> sm = defaultServiceManager();
    if (sm != NULL) {
        // use non-blocking way to get GuiExtService with ServiceManager
        sp<IBinder> binder = sm->checkService(String16("GuiExtService"));
        if (binder != NULL) {
            return interface_cast<IGuiExtService>(binder);
        }
        ALOGW("Cannot find GuiExtService");
        return NULL;
    }
    ALOGW("Cannot find default ServiceManager");
    return NULL;
}
};

using namespace android;

bool regDump(const sp<IDumpTunnel>& tunnel, const String8& key)
{
#ifndef MTK_DO_NOT_USE_GUI_EXT
    bool result = false;
    sp<IGuiExtService> guiExt = checkGuiExtService();
    if (guiExt != NULL) {
        result = (guiExt->regDump(tunnel, key) == NO_ERROR);
    }
    return result;
#endif
}

bool unregDump(const String8& key)
{
#ifndef MTK_DO_NOT_USE_GUI_EXT
    bool result = false;
    sp<IGuiExtService> guiExt = checkGuiExtService();
    if (guiExt != NULL) {
        result = (guiExt->unregDump(key) == NO_ERROR);
    }
    return result;
#endif
}
