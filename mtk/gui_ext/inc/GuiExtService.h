#ifndef __GUIEXT_SERVICE_H__
#define __GUIEXT_SERVICE_H__

#include <utils/threads.h>
#include "IGuiExtService.h"

namespace android
{

class GuiExtPool;
class String8;
class GuiExtService :
        public BinderService<GuiExtService>,
        public BnGuiExtService
//        public Thread
{
    friend class BinderService<GuiExtService>;
public:

    GuiExtService();
    ~GuiExtService();

    static char const* getServiceName() { return "GuiExtService"; }

    // IGuiExtServic interface
    virtual status_t alloc(const sp<IBinder>& token, uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t *id);
    virtual status_t free(uint32_t id);
    virtual status_t acquire(const sp<IBinder>& token, uint32_t poolId, uint32_t usage, uint32_t type, int *buf);
    virtual status_t request(uint32_t poolId, uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer);
    virtual status_t release(uint32_t poolId, uint32_t usage, uint32_t type, int buf);
    virtual status_t disconnect(uint32_t poolId, uint32_t usage, uint32_t type);
    virtual status_t configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum);
    virtual status_t dump(int fd, const Vector<String16>& args);

    // to register/unregister dump tunnels to/from mDumpTunnels
    virtual status_t regDump(const sp<IDumpTunnel>& tunnel, const String8& key);
    virtual status_t unregDump(const String8& key);

private:
//    virtual void onFirstRef();
//    virtual status_t readyToRun();
//    virtual bool threadLoop();

    mutable Mutex mLock;
    sp<GuiExtPool> mPool;

    // maintain a table to store all dump tunnels
    // each tunnel's key has a prefix which presents types of tunnels
    // ex: BQ-... this is a dump tunnel for BQ
    mutable Mutex mDumpLock;
    KeyedVector<String8, sp<IDumpTunnel> > mDumpTunnels;
};
};
#endif
