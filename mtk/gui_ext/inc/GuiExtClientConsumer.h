#ifndef __GUIEXT_SERVICE_CLIENT_CONSUMER_H__
#define __GUIEXT_SERVICE_CLIENT_CONSUMER_H__

#include "GuiExtClient.h"
#include "GuiExtDataType.h"

namespace android {

class IGuiExtService;
class GuiExtClientConsumer :
        public GuiExtClient,
        public Singleton<GuiExtClientConsumer>
{
    friend class Singleton<GuiExtClientConsumer>;

public:
    // acquire and release is used by consumer
    status_t acquire(android_native_buffer_t** buffer, uint32_t poolId, uint32_t usage, uint32_t type = 0);
    status_t release(android_native_buffer_t* buffer, uint32_t poolId, uint32_t usage, uint32_t type = 0);

    // for release shadow GraphicBuffer reference
    status_t disconnect(uint32_t poolId, uint32_t usage, uint32_t type = 0);
    // for HWC to config multi-display information
    status_t configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum = 3);
    // check this buffer is in Shadow List or not
    bool isValidBuffer(android_native_buffer_t* buffer);
    // for dump shadow GraphicBuffer information
    void dump(String8& result, char* buffer, size_t SIZE) const;

private:
    GuiExtClientConsumer();
    virtual ~GuiExtClientConsumer();

    // DeathRecipient interface
    virtual void serviceDiedLocked();

    class BufferSlot : public virtual RefBase {
        public:
            sp<GraphicBuffer> buffer;
            uint32_t poolId;
            uint32_t usage;
            uint32_t type;
            uint32_t idx;
            bool isAcquired;
    };

    DefaultKeyedVector< uint32_t, sp<BufferSlot> > mSlots;

    // private function
    int getSlotFromBufferLocked(android_native_buffer_t* buffer) const;

    status_t releaseLocked(android_native_buffer_t* buffer, uint32_t poolId, uint32_t usage, uint32_t type);
};

};

#endif
