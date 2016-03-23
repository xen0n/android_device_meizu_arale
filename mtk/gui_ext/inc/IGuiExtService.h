#ifndef __IGUIEXTSERVICE_H__
#define __IGUIEXTSERVICE_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/BinderService.h>

#include <gui/IGraphicBufferAlloc.h>

namespace android
{
//
//  Holder service for pass objects between processes.
//

class IDumpTunnel;
class String8;

class IGuiExtService : public IInterface
{
protected:
    enum {
        GUI_EXT_ALLOC = IBinder::FIRST_CALL_TRANSACTION,
        GUI_EXT_FREE,
        GUI_EXT_ACQUIRE,
        GUI_EXT_REQUEST,
        GUI_EXT_RELEASE,
        GUI_EXT_DISCONNECT,
        GUI_EXT_CONFIGDISPLAY,
        GUI_EXT_REGDUMP,
        GUI_EXT_UNREGDUMP
    };

public:
    DECLARE_META_INTERFACE(GuiExtService);

    virtual status_t alloc(const sp<IBinder>& token, uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t *id) = 0;
    virtual status_t free(uint32_t id) = 0;
    virtual status_t acquire(const sp<IBinder>& token, uint32_t poolId, uint32_t usage, uint32_t type, int *buf) = 0;
    virtual status_t request(uint32_t poolId, uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer) = 0;
    virtual status_t release(uint32_t poolId, uint32_t usage, uint32_t type, int buf) = 0;
    virtual status_t disconnect(uint32_t poolId, uint32_t usage, uint32_t type) = 0;
    virtual status_t configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum) = 0;

    // for dump tunnel
    virtual status_t regDump(const sp<IDumpTunnel>& tunnel, const String8& key) = 0;
    virtual status_t unregDump(const String8& key) = 0;
};

class BnGuiExtService : public BnInterface<IGuiExtService>
{
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

// an easy way to check and get the GuiExtService without blocking
sp<IGuiExtService> checkGuiExtService();
};

using namespace android;

extern "C"
{
    // The circular link must be broken because libgui.so and libgui_ext.so needs to link each other
    // libgui.so uses dlopen() to load libgui_ext.so at runtime
    bool regDump(const sp<IDumpTunnel>& tunnel, const String8& key);
    bool unregDump(const String8& key);
}
#endif
