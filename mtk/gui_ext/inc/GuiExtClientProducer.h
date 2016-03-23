#ifndef __GUIEXT_SERVICE_CLIENT_PRODUCER_H__
#define __GUIEXT_SERVICE_CLIENT_PRODUCER_H__

#include "GuiExtClient.h"

namespace android {

class GuiExtClientProducer :
        public GuiExtClient,
        public Singleton<GuiExtClientProducer>
{
    friend class Singleton<GuiExtClientProducer>;

public:
    // IGuiExtServic interface
    // alloc & free is used by producer
    status_t alloc(uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t *poolId);
    status_t free(uint32_t poolId);
protected:
    GuiExtClientProducer();
    virtual ~GuiExtClientProducer();

    // DeathRecipient interface
    virtual void serviceDiedLocked();
};

};

#endif
