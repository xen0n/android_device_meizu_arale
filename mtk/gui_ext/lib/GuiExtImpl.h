#ifndef __GUIEXT_IMPL_H__
#define __GUIEXT_IMPL_H__

#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include "GuiExtDataType.h"

namespace android {

class IBinder;
class BufferQueue;
class GraphicBuffer;
class GuiExtPoolItem;

#define SUPPORT_MULTIBQ_FOR_HWC 1
//#define SUPPORT_MULTIBQ_FOR_HWC 0
//#define TOUCH_ION_BUFFER

class BinderDiedListener : public virtual RefBase {
public:
    BinderDiedListener() { }
    virtual ~BinderDiedListener() { }

    virtual void binderDied(uint32_t poolId) = 0; /* Asynchronous */
};
//------------------------------------------------------------------------------

class DispInfo : public virtual RefBase {
    public:
        uint32_t type;
        uint32_t w;
        uint32_t h;
        uint32_t bufNum;
};
//------------------------------------------------------------------------------

// GuiExtPool class
class GuiExtPool : public virtual RefBase {
public:
    GuiExtPool();
    ~GuiExtPool();

    status_t alloc(const sp<IBinder>& token, uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t *poolId);
    status_t free(uint32_t poolId);
    status_t acquire(const sp<IBinder>& token, uint32_t poolId, uint32_t usage, uint32_t type, int *buf);
    status_t request(uint32_t poolId, uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer);
    status_t release(uint32_t poolId, uint32_t usage, uint32_t type, int buf);
    status_t disconnect(uint32_t poolId, uint32_t usage, uint32_t type);
    void configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum);

    void removePoolItemLocked(uint32_t poolId);
    void removePoolItem(uint32_t poolId);
    void dump(String8& result) const;

private:
    uint32_t mPoolId;
    DefaultKeyedVector< uint32_t, sp<GuiExtPoolItem> > mPoolList;
    DefaultKeyedVector< uint32_t, sp<DispInfo> > mDisplayList;

    mutable Mutex mLock;
    mutable sp<BinderDiedListener> mConsumerDeathObserver;

    uint32_t mDefaultDisplayWidth;
    uint32_t mDefaultDisplayHeight;

    uint32_t getPoolId();
};

//------------------------------------------------------------------------------

// GuiExtPoolItem class
class GuiExtPoolItem : public virtual RefBase,
                       protected ConsumerListener {
public:
    GuiExtPoolItem(const sp<IBinder>& token,
                   bool isHwcNeeded,
                   uint32_t id,
                   uint32_t w,
                   uint32_t h,
                   DefaultKeyedVector< uint32_t, sp<DispInfo> >& dispList,
                   sp<IBinder::DeathRecipient> observer);
    ~GuiExtPoolItem();

    status_t prepareBuffer(uint32_t usage);
    status_t acquire(const sp<IBinder>& token, uint32_t usage, uint32_t type, sp<BinderDiedListener> listener, int *buf);
    status_t request(uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer);
    status_t release(uint32_t usage, uint32_t type, int buf);
    status_t disconnect(uint32_t usage, uint32_t type);

    virtual void onFrameAvailable(const BufferItem& item) {};
    virtual void onBuffersReleased() {};
    virtual void onSidebandStreamChanged() {};

    sp<BinderDiedListener> mConsumerDeathListener;

private:
    friend class GuiExtPool;

    //sp<BufferQueue> mGPUUsedBq;
    sp<IGraphicBufferProducer> mGPUUsedProducer;
    sp<IGraphicBufferConsumer> mGPUUsedConsumer;
#if SUPPORT_MULTIBQ_FOR_HWC
    class HwcBqSlot : public virtual RefBase {
        public:
            //sp<BufferQueue> bq;
            sp<IGraphicBufferProducer> mProducer;
            sp<IGraphicBufferConsumer> mConsumer;
            uint32_t type;
            uint32_t bufNum;
    };
    DefaultKeyedVector< uint32_t, sp<HwcBqSlot> > mHwcUsedBqList;
#else
    sp<BufferQueue> mHwcUsedBq;
#endif

    uint32_t mId;
    uint32_t mIsHwcNeeded;
    uint32_t mGpuUsedBufNum;
    int32_t  mProducerPid;
    bool mIsDisconnected[GUI_EXT_USAGE_MAX];

    sp<IBinder> mProducerToken;

    class ConsumerSlot : public virtual RefBase {
        public:
            uint32_t usage;
            uint32_t type;
            uint32_t idx;
            int pid;
            sp<IBinder> token;
            mutable sp<IBinder::DeathRecipient> observer;

            void dump(String8& result) const;
    };
    DefaultKeyedVector< uint32_t, sp<ConsumerSlot> > mConsumerList;

    mutable sp<IBinder::DeathRecipient> mProducerDeathObserver;

    void createBufferQueue(uint32_t w, uint32_t h, uint32_t num, String8 name,
        sp<IGraphicBufferProducer>* p, sp<IGraphicBufferConsumer>* c);
//    sp<BufferQueue> getBufferQueue(uint32_t usage, uint32_t type);
    sp<IGraphicBufferProducer> getIGraphicBufferProducer(uint32_t usage, uint32_t type);

    status_t prepareBuffer(sp<IGraphicBufferProducer> producer, uint32_t usage, uint32_t type, uint32_t gralloc_usage, uint32_t bufNum);
#ifdef TOUCH_ION_BUFFER
    void touchIonBuffer(sp<GraphicBuffer> gb);
    buffer_handle_t getNativehandle(android_native_buffer_t* buffer);
#endif

    void dump(String8& result) const;
    void dumpConsumerSlots(String8& result, uint32_t usage, uint32_t type) const;
};

//------------------------------------------------------------------------------

#ifdef TOUCH_ION_BUFFER
class GuiExtIONDevice : public Singleton<GuiExtIONDevice>
{
public:
    GuiExtIONDevice();
    ~GuiExtIONDevice();

    // import handle from ion share fd
    uint32_t import(int ion_fd);
    void touchMMUAddress(int client, int ion_hnd);

private:
    int m_dev_fd;
};
#endif
}
#endif

