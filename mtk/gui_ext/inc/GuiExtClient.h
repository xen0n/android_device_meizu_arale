#ifndef __GUIEXT_SERVICE_CLIENT_H__
#define __GUIEXT_SERVICE_CLIENT_H__

#include <stdint.h>
#include <sys/types.h>

#include <binder/Binder.h>
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>
#include <utils/KeyedVector.h>

#include "IGuiExtService.h"

namespace android {

class IGuiExtService;
class GuiExtClient
{
protected:
    GuiExtClient();
    virtual ~GuiExtClient(){}

    // DeathRecipient interface
    void serviceDied();
    virtual void serviceDiedLocked() = 0;

    status_t assertStateLocked() const;

    mutable Mutex mLock;
    mutable sp<IGuiExtService> mGuiExtService;
    mutable sp<IBinder::DeathRecipient> mDeathObserver;
};

};

#endif
