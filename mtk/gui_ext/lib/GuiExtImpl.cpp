#define LOG_TAG "GuiExt"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#define MTK_LOG_ENABLE 1
#include <dlfcn.h>
#include <stdlib.h>
#include <utils/Timers.h>
#include <utils/Trace.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <math.h>

#include <ui/DisplayInfo.h>
#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <gui/IProducerListener.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/CpuConsumer.h>
#include <hardware/gralloc.h>
#include <hardware/hwcomposer_defs.h>
#include <binder/IInterface.h>
#include <binder/IPCThreadState.h>

#include <hardware/gralloc_extra.h>

#ifdef TOUCH_ION_BUFFER
#include <linux/ion_drv.h>
#include <ion/ion.h>
#endif

#include "GuiExtImpl.h"
#include "GuiExtDataType.h"

#include "graphics_mtk_defs.h"
#include "gralloc_mtk_defs.h"

#ifdef CONFIG_FOR_SOURCE_PQ
#include <sys/ioctl.h>
#include <linux/disp_session.h>
#endif

namespace android {

#define GUIEXT_LOGV(x, ...) ALOGV("[GuiExtI] " x, ##__VA_ARGS__)
#define GUIEXT_LOGD(x, ...) ALOGD("[GuiExtI] " x, ##__VA_ARGS__)
#define GUIEXT_LOGI(x, ...) ALOGI("[GuiExtI] " x, ##__VA_ARGS__)
#define GUIEXT_LOGW(x, ...) ALOGW("[GuiExtI] " x, ##__VA_ARGS__)
#define GUIEXT_LOGE(x, ...) ALOGE("[GuiExtI] " x, ##__VA_ARGS__)

#define ION_DEV_NODE "/dev/ion"

#define MAX_ALLOC_SIZE          10
#define MAX_GLES_DEQUEUED_NUM   3
#define MAX_HWC_DEQUEUED_NUM    3
#define LOCK_FOR_USAGE          (GRALLOC_USAGE_SW_READ_RARELY | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE)
#define GUIEXT_ION_CLIENT_ID    0

#define POOL_USAGE_SHIFT(usage)     (usage << 6)
#define POOL_TYPE_SHIFT(type)     (type << 3)
#define POOL_COMBINED_ID(usage, type, idx)   (POOL_USAGE_SHIFT(usage)|POOL_TYPE_SHIFT(type)|idx)

const char* szUsageName[2] = {"GuiExt_Gpu", "GuiExt_Hwc"};
const char* szDisplayType[HWC_NUM_DISPLAY_TYPES] = {"Primary", "External", "Virtual"};

uint32_t gAcquiredFormat[GUI_EXT_USAGE_MAX] = {HAL_PIXEL_FORMAT_YV12, HAL_PIXEL_FORMAT_YUYV};

#ifdef CONFIG_FOR_SOURCE_PQ
class DispDevice: public Singleton<DispDevice>
{
    int mDevFd;
    int mVideoRefCount;
    disp_caps_info mCaps;

public:
    DispDevice();
    ~DispDevice();

    status_t setOverlaySessionMode(DISP_MODE mode);
    uint32_t getPqNum() const {
        if (mCaps.max_pq_num == 0) {
            GUIEXT_LOGE("[PQ] Failed to get num of PQ");
        }

        return mCaps.max_pq_num;
    }
};

ANDROID_SINGLETON_STATIC_INSTANCE(DispDevice);

DispDevice::DispDevice()
    : mDevFd(-1)
    , mVideoRefCount(0)
{
    memset(&mCaps, 0, sizeof(disp_caps_info));

    char filename[256];
    mDevFd = open(filename, O_RDONLY);
    sprintf(filename, "/dev/%s", DISP_SESSION_DEVICE);
    if (mDevFd < 0) {
        GUIEXT_LOGE("[PQ] Failed to open display device(%s): fd=%d", filename, mDevFd);
    } else {
        GUIEXT_LOGD("[PQ] open display device(%s) successfully", filename);
        int err = ioctl(mDevFd, DISP_IOCTL_GET_DISPLAY_CAPS, &mCaps);
        if (err < 0) {
            GUIEXT_LOGE("[PQ] Failed to get disp capability err=%d", err);
        }
    }
}

DispDevice::~DispDevice()
{
    if (mDevFd >= 0)
        close(mDevFd);
}

status_t DispDevice::setOverlaySessionMode(DISP_MODE mode)
{
    const bool decouple = (mode == DISP_SESSION_DECOUPLE_MODE);
    GUIEXT_LOGD("[PQ] decouple=%d, VideoRefCnt=%d \n", decouple, mVideoRefCount);

    mVideoRefCount = decouple ? mVideoRefCount + 1: mVideoRefCount - 1;
    // If not the first VP start or last VP end, do nothing
    if ( !((mVideoRefCount == 1 && decouple) || (mVideoRefCount == 0 && !decouple)) ) {
        return NO_ERROR;
    }

    GUIEXT_LOGD("[PQ] Dynamic Switch to (mode=%d)", mode);

    disp_session_config config;
    memset(&config, 0, sizeof(disp_session_config));
    config.device_id  = HWC_DISPLAY_PRIMARY;
    config.type = DISP_SESSION_PRIMARY;
    config.session_id = MAKE_DISP_SESSION(config.type, config.device_id);
    config.mode       = mode;
    config.user       = SESSION_USER_GUIEXT;

    int err = ioctl(mDevFd, DISP_IOCTL_SET_SESSION_MODE, &config);
    if (err < 0)
    {
        GUIEXT_LOGE("[PQ] Failed to set DispSession (mode=%d, errno:%d)!!", mode, err);
        return BAD_VALUE;
    }
    return NO_ERROR;
}
#endif // CONFIG_FOR_SOURCE_PQ

// --------------------------------------------------------

GuiExtPool::GuiExtPool()
    : mPoolId(0)
    , mPoolList(NULL)
    , mDefaultDisplayWidth(0)
    , mDefaultDisplayHeight(0)
{
    GUIEXT_LOGI("GuiExtPool ctor");
}

GuiExtPool::~GuiExtPool()
{
}

status_t GuiExtPool::alloc(const sp<IBinder>& token, uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t * poolId)
{
    Mutex::Autolock _l(mLock);
    ATRACE_CALL();
    GUIEXT_LOGD("  alloc, gralloc_usage=%x, w=%d, h=%d", gralloc_usage, w, h);

    const nsecs_t time_start = systemTime();

    uint32_t size = mPoolList.size();
    if (size >= MAX_ALLOC_SIZE) {
        GUIEXT_LOGW("  alloc pools size is reaching %d, can't alloc", size);
        *poolId = 0;
        return NOT_ENOUGH_DATA;
    }

    if (size == 0) {
        DisplayInfo dinfo;
        sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
                ISurfaceComposer::eDisplayIdMain);
        SurfaceComposerClient::getDisplayInfo(display, &dinfo);

        mDefaultDisplayWidth = dinfo.w;
        mDefaultDisplayHeight = dinfo.h;

        sp<DispInfo> disp = new DispInfo;
        disp->type = 0;
        disp->w = mDefaultDisplayWidth;
        disp->h = mDefaultDisplayHeight;
        disp->bufNum = MAX_HWC_DEQUEUED_NUM;
        mDisplayList.add((uint32_t)HWC_DISPLAY_PRIMARY, disp);

        GUIEXT_LOGI("pool list size=0, try to get default display(w x h) = (%d x %d)", disp->w, disp->h);
    }

    *poolId = getPoolId();
    bool isHwcNeeded = gralloc_usage & GRALLOC_USAGE_HW_COMPOSER;

    class ProducerDeathObserver : public IBinder::DeathRecipient {
        GuiExtPool & mPool;
        virtual void binderDied(const wp<IBinder>& who) {
            uint32_t size = mPool.mPoolList.size();
            GUIEXT_LOGI("producer died, pool size=%d, binder ptr=[%p]", size, who.unsafe_get());

            if (size == 0) {
                GUIEXT_LOGV("producer died [%p], pool size is zero", who.unsafe_get());
            } else {
                for (uint32_t i = size-1 ;; i--) {
                    GUIEXT_LOGV("   [p] compare index[%d], p=[%p]", i, mPool.mPoolList[i]->mProducerToken.get());
                    if (mPool.mPoolList[i]->mProducerToken.get() == who.unsafe_get()) {
                        GUIEXT_LOGW("   [p] found index[%d], p=[%p], id=%d", i, mPool.mPoolList[i]->mProducerToken.get(), mPool.mPoolList[i]->mId);
                        mPool.mPoolList[i]->mProducerToken = NULL;
                        mPool.mPoolList[i]->mProducerPid = -1;
                        mPool.removePoolItem(mPool.mPoolList[i]->mId);
                    }

                    if (i == 0)
                        break;
                }

                GUIEXT_LOGV("producer died done");
            }
        }
    public:
        ProducerDeathObserver(GuiExtPool & pool) : mPool(pool) { }
    };

    sp<IBinder::DeathRecipient> observer = new ProducerDeathObserver(*const_cast<GuiExtPool*>(this));
    token->linkToDeath(observer);

    sp<GuiExtPoolItem> item = new GuiExtPoolItem(token, isHwcNeeded, *poolId, w, h, mDisplayList, observer);
    status_t err;
    err = item->prepareBuffer(gralloc_usage);
    if (err != NO_ERROR) {
        GUIEXT_LOGE("  alloc fail, isHWcNeeded=%d, w=%d, h=%d, usage=%x, poolId=%d", isHwcNeeded, w, h, gralloc_usage, *poolId);
        *poolId = 0;
        return err;
    }

    mPoolList.add(*poolId, item);

    const nsecs_t time_end = systemTime();
    GUIEXT_LOGI("  alloc cost time=%" PRId64 " ms, gralloc_usage=%x, w=%d, h=%d, poolId=%d", ns2ms(time_end - time_start),
        gralloc_usage, w, h, *poolId);

    return err;
}

status_t GuiExtPool::free(uint32_t poolId)
{
    Mutex::Autolock _l(mLock);
    ATRACE_CALL();
    GUIEXT_LOGD("  free, poolId=%d", poolId);

    sp<GuiExtPoolItem> item = mPoolList.valueFor(poolId);
    if (item == NULL) {
        GUIEXT_LOGE("free a non-exist pool item, poolId=%d", poolId);
        return BAD_INDEX;
    }

    item->mProducerToken = NULL;
    item->mProducerPid = -1;
    removePoolItemLocked(poolId);
    return NO_ERROR;
}

status_t GuiExtPool::acquire(const sp<IBinder>& token, uint32_t poolId, uint32_t usage, uint32_t type, int *buf)
{
    Mutex::Autolock _l(mLock);
    ATRACE_CALL();
    sp<GuiExtPoolItem> item = mPoolList.valueFor(poolId);
    if (item == NULL) {
        *buf = -1;
        GUIEXT_LOGW("  acquire fail (pool not found), poolId=%d, usage=%s, type=%s, buf=%d", poolId, szUsageName[usage], szDisplayType[type], *buf);
        return BAD_INDEX;
    }

    class BinderDiedListenerImpl : public BinderDiedListener {
        GuiExtPool & mPool;
        virtual void binderDied(uint32_t poolId) {
            GUIEXT_LOGV("  [cb] all consumers has disconnected +, poolId=%d", poolId);
            mPool.removePoolItem(poolId);
            GUIEXT_LOGV("  [cb] consumer has died -");
        }
    public:
        BinderDiedListenerImpl(GuiExtPool & pool) : mPool(pool) { }
    };

    sp<BinderDiedListener> listener = new BinderDiedListenerImpl(*const_cast<GuiExtPool*>(this));

    status_t err = item->acquire(token, usage, type, listener, buf);
    GUIEXT_LOGD("  acquire done, token=%p, poolId=%d, usage=%s, type=%s, buf=%d", token.get(), poolId, szUsageName[usage], szDisplayType[type], *buf);
    return err;
}

status_t GuiExtPool::request(uint32_t poolId, uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer)
{
    Mutex::Autolock _l(mLock);
    ATRACE_CALL();
    sp<GuiExtPoolItem> item = mPoolList.valueFor(poolId);
    if (item == NULL) {
        GUIEXT_LOGW("  request fail (pool not found), poolId=%d, usage=%s, type=%s", poolId, szUsageName[usage], szDisplayType[type]);
        return BAD_INDEX;
    }
    status_t err = item->request(usage, type, buf, buffer);
    GUIEXT_LOGD("  request done, poolId=%d, usage=%s, type=%s, buf=%d, buffer=%p", poolId, szUsageName[usage], szDisplayType[type], buf, (*buffer).get());
    return err;
}

status_t GuiExtPool::release(uint32_t poolId, uint32_t usage, uint32_t type, int buf)
{
    Mutex::Autolock _l(mLock);
    ATRACE_CALL();
    sp<GuiExtPoolItem> item = mPoolList.valueFor(poolId);
    if (item == NULL) {
        GUIEXT_LOGW("  release fail (pool not found), poolId=%d, usage=%s, type=%s", poolId, szUsageName[usage], szDisplayType[type]);
        return BAD_INDEX;
    }
    status_t err = item->release(usage, type, buf);
    GUIEXT_LOGD("  release done, poolId=%d, usage=%s, type=%s, buf=%d", poolId, szUsageName[usage], szDisplayType[type], buf);
    return err;
}

status_t GuiExtPool::disconnect(uint32_t poolId, uint32_t usage, uint32_t type)
{
    Mutex::Autolock _l(mLock);
    ATRACE_CALL();
    sp<GuiExtPoolItem> item = mPoolList.valueFor(poolId);
    if (item == NULL) {
        GUIEXT_LOGW("  disconnect fail (pool not found), poolId=%d, usage=%s, type=%s", poolId, szUsageName[usage], szDisplayType[type]);
        return BAD_INDEX;
    }
    status_t err = item->disconnect(usage, type);
    removePoolItemLocked(poolId);
    GUIEXT_LOGD("  disconnect done, poolId=%d, usage=%s, type=%s", poolId, szUsageName[usage], szDisplayType[type]);
    return err;
}

void GuiExtPool::configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum)
{
    GUIEXT_LOGD("  configDisplay, type=%d, enable=%d, w=%d, h=%d, bufNum=%d", type, enable, w, h, bufNum);
    if (type == HWC_DISPLAY_PRIMARY)
        GUIEXT_LOGI("  configDisplay, try to modify primary display, type=%d, w=%d, h=%d, bufNum=%d", type, w, h, bufNum);

    sp<DispInfo> disp = mDisplayList.valueFor(type);

    if (!enable || !w || !h || !bufNum) {
        if (disp != 0) {
            if (type != HWC_DISPLAY_PRIMARY) {
                GUIEXT_LOGI("  configDisplay, remove type=%d, w=%d, h=%d, bufNum=%d", type, w, h, bufNum);
                mDisplayList.removeItem(type);
            }

            GUIEXT_LOGI("  configDisplay, remove type=%d, enable=%d, w=%d, h=%d, bufNum=%d", type, enable, w, h, bufNum);
            mDisplayList.removeItem(type);
        }
    } else {
        if (disp == 0) {
            disp = new DispInfo();
            GUIEXT_LOGI("  configDisplay, add new type=%d, w=%d, h=%d, bufNum=%d", type, w, h, bufNum);
        } else {
            GUIEXT_LOGI("  configDisplay, change config of type=%d, w=%d, h=%d, bufNum=%d", type, w, h, bufNum);
        }

        disp->type = type;
        disp->w = w;
        disp->h = h;
        disp->bufNum = bufNum;
        mDisplayList.add(type, disp);
    }

    if (mDisplayList.size() == 0)
        GUIEXT_LOGW("  configDisplay, all display config has been removed, type=%d, enable=%d", type, enable);
}

void GuiExtPool::removePoolItemLocked(uint32_t poolId)
{
    sp<GuiExtPoolItem> item = mPoolList.valueFor(poolId);
    if (item == NULL) {
        GUIEXT_LOGW("  remove a non-exist pool item, poolId=%d", poolId);
        return;
    }

    bool hasConsumerExist = false;
    for (uint32_t i = 0; i < GUI_EXT_USAGE_MAX; i++) {
        if (!item->mIsDisconnected[i]) {
            hasConsumerExist = true;
            GUIEXT_LOGV(" find poolId=%d exist consumer[%s]", poolId, szUsageName[i]);
            return;
        }
    }

    if (!hasConsumerExist && item->mProducerToken == NULL) {
        mPoolList.removeItem(poolId);
        GUIEXT_LOGV("remove pool item, id=%d", poolId);
    } else {
        GUIEXT_LOGV("remove pool item fail, id=%d, producer not free yet, token=%p, pid=%d",
            poolId, item->mProducerToken.get(), item->mProducerPid);
    }
}

void GuiExtPool::removePoolItem(uint32_t poolId)
{
    Mutex::Autolock _l(mLock);
    removePoolItemLocked(poolId);
}

void GuiExtPool::dump(String8& result) const
{
    uint32_t size = mPoolList.size();
    uint32_t disp_num = mDisplayList.size();

    result.appendFormat("Display Num: %d\n", disp_num);
    result.append("--------------------------------------------------\n");
    for (uint32_t i = 0; i < disp_num; i++) {
        const sp<DispInfo>& disp = mDisplayList[i];
        result.appendFormat("+ DispInfo[%d]\n", i);
        result.appendFormat("    type=%s, w=%d, h=%d, bufNum=%d\n",
            szDisplayType[disp->type], disp->w, disp->h, disp->bufNum);
    }
    result.append("--------------------------------------------------\n");

    result.appendFormat("\nPool List Size: %d\n", size);
    result.append("--------------------------------------------------\n");
    for (uint32_t i = 0; i < size; i ++) {
        result.appendFormat("+ Pool[%d]\n", i);
        mPoolList[i]->dump(result);
    }
    result.append("--------------------------------------------------\n");
}

uint32_t GuiExtPool::getPoolId()
{
    mPoolId = (mPoolId+1)%MAX_ALLOC_SIZE;
    uint32_t id = mPoolId == 0 ? MAX_ALLOC_SIZE : mPoolId;
    sp<GuiExtPoolItem> item = mPoolList.valueFor(id);
    while (item != NULL) {
        mPoolId = (mPoolId+1)%MAX_ALLOC_SIZE;
        id = mPoolId == 0 ? MAX_ALLOC_SIZE : mPoolId;
        item = mPoolList.valueFor(id);
    }

    mPoolId = id;

    return id;
}

//------------------------------------------------------------------------------

GuiExtPoolItem::GuiExtPoolItem(const sp<IBinder>& token,
                bool isHwcNeeded,
                uint32_t poolId,
                uint32_t /*w*/,
                uint32_t /*h*/,
                DefaultKeyedVector< uint32_t, sp<DispInfo> >& dispList,
                sp<IBinder::DeathRecipient> observer)
                : mConsumerDeathListener(NULL)
                //, mGPUUsedBq(NULL)
                , mGPUUsedProducer(NULL)
                , mGPUUsedConsumer(NULL)
#if !SUPPORT_MULTIBQ_FOR_HWC
                , mHwcUsedBq(NULL)
#endif
                , mId(poolId)
                , mIsHwcNeeded(isHwcNeeded)
                , mGpuUsedBufNum(MAX_GLES_DEQUEUED_NUM)
                , mProducerPid(-1)
                , mProducerToken(token)
                , mProducerDeathObserver(observer)

{
    GUIEXT_LOGV("GuiExtPoolItem ctor, poolId=%d, isHwcNeeded=%d, token=%p", poolId, isHwcNeeded, token.get());

    //String8 name(szUsageName[0]);
    //name.appendFormat("_%d", poolId);
    //mGPUUsedBq = createBufferQueue(w, h, mGpuUsedBufNum, name);
    //createBufferQueue(w, h, mGpuUsedBufNum, name, &mGPUUsedProducer, &mGPUUsedConsumer);

#ifdef CONFIG_FOR_SOURCE_PQ
    if (DispDevice::getInstance().getPqNum() == 1)
    {
        DispDevice::getInstance().setOverlaySessionMode(DISP_SESSION_DECOUPLE_MODE);
    }
#endif
    if (mIsHwcNeeded) {
        String8 name(szUsageName[1]);
        name.appendFormat("_%d", poolId);
#if SUPPORT_MULTIBQ_FOR_HWC
        uint32_t disp_size = dispList.size();
        for (uint32_t i = 0; i < disp_size; i++) {
            uint32_t type = dispList[i]->type;
            String8 extname(name);
            extname.appendFormat("_%d", type);
            sp<HwcBqSlot> slot = new HwcBqSlot();
            //slot->bq = createBufferQueue(dispList[i]->w, dispList[i]->h, dispList[i]->bufNum, extname);
            createBufferQueue(dispList[i]->w, dispList[i]->h, dispList[i]->bufNum, extname, &slot->mProducer, &slot->mConsumer);
            slot->type = type;
            slot->bufNum = dispList[i]->bufNum;
            mHwcUsedBqList.add(type, slot);
        }
#else
        mHwcUsedBq = createBufferQueue(dispList[0]->w, dispList[0]->h, dispList[i]->bufNum, name);
#endif
    }

    mProducerPid = (NULL != token->localBinder())
                 ? getpid()
                 : IPCThreadState::self()->getCallingPid();

    for (uint32_t i = 0; i < GUI_EXT_USAGE_MAX; i++)
        mIsDisconnected[i] = true;
}

GuiExtPoolItem::~GuiExtPoolItem()
{
    //mGPUUsedBq.clear();
    mGPUUsedProducer.clear();
    mGPUUsedConsumer.clear();
#if SUPPORT_MULTIBQ_FOR_HWC
    mHwcUsedBqList.clear();
#else
    mHwcUsedBq.clear();
#endif

#ifdef CONFIG_FOR_SOURCE_PQ
    if (DispDevice::getInstance().getPqNum() == 1)
    {
        DispDevice::getInstance().setOverlaySessionMode(DISP_SESSION_DIRECT_LINK_MODE);
    }
#endif
}

void GuiExtPoolItem::createBufferQueue(uint32_t w, uint32_t h, uint32_t num, String8 name,
    sp<IGraphicBufferProducer>* p, sp<IGraphicBufferConsumer>* c)
{
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);

    consumer->setDefaultBufferSize(w, h);
    consumer->setConsumerName(name);
	producer->setBufferCount(num);
	consumer->setMaxAcquiredBufferCount(2);

    // consumer connect
    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);
    sp<BufferQueue::ProxyConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);
    status_t err = consumer->consumerConnect(proxy, true);
    if (err != NO_ERROR) {
        ALOGE("GuiExtPoolItem: error connecting to BufferQueue: %s (%d)",
                strerror(-err), err);
    }

    // producer connect
    //static sp<BBinder> sLife = new BBinder();
    static sp<IProducerListener> plistener = new DummyProducerListener();
    IGraphicBufferProducer::QueueBufferOutput output;
    err = producer->connect(plistener, NATIVE_WINDOW_API_MEDIA, true, &output);

    *p = producer;
    *c = consumer;
}

status_t GuiExtPoolItem::prepareBuffer(uint32_t gralloc_usage)
{
    status_t err = NO_ERROR;

    //err = prepareBuffer(mGPUUsedProducer, GUI_EXT_USAGE_GPU, 0, gralloc_usage, mGpuUsedBufNum);
    //GUIEXT_LOGV("    prepare %s buffer done", szUsageName[GUI_EXT_USAGE_GPU]);

    if (mIsHwcNeeded) {
#if SUPPORT_MULTIBQ_FOR_HWC
        for (uint32_t i = 0; i < mHwcUsedBqList.size(); i++) {
            err = prepareBuffer(mHwcUsedBqList[i]->mProducer, GUI_EXT_USAGE_HWC, mHwcUsedBqList[i]->type, gralloc_usage, mHwcUsedBqList[i]->bufNum);
            GUIEXT_LOGV("    prepare %s buffer, type=%d done", szUsageName[GUI_EXT_USAGE_HWC], mHwcUsedBqList[i]->type);
        }
#else
        err = prepareBuffer(mHwcUsedBq, GUI_EXT_USAGE_HWC, 0, gralloc_usage, mHwcUsedBqList[i]->bufNum);
        GUIEXT_LOGV("    prepare %s buffer done", szUsageName[GUI_EXT_USAGE_HWC]);
#endif
    }

    return err;
}

status_t GuiExtPoolItem::prepareBuffer(sp<IGraphicBufferProducer> producer, uint32_t usage, uint32_t type, uint32_t gralloc_usage, uint32_t bufNum)
{
    status_t err = NO_ERROR;
    uint32_t usg = (gralloc_usage & GRALLOC_USAGE_SECURE) != 0 ? (LOCK_FOR_USAGE | GRALLOC_USAGE_SECURE) : LOCK_FOR_USAGE;
    uint32_t fmt = gAcquiredFormat[usage];

    GUIEXT_LOGV("prepareBuffer, type=%d, fmt=%x, usage=%x, usg=%x, bufNum=%d", usage, fmt, gralloc_usage, usg, bufNum);

    for (uint32_t i = 0; i < bufNum; i++) {
        int buf = -1;
        sp<Fence> fence;

        producer->dequeueBuffer(&buf, &fence, false, 0, 0, fmt, usg);

        uint32_t combine_id = POOL_COMBINED_ID(usage, type, i);
        sp<ConsumerSlot> consumerSlot = mConsumerList.valueFor(combine_id);
        if (consumerSlot == 0) {
            sp<ConsumerSlot> slot = new ConsumerSlot();
            slot->token = NULL;
            slot->pid = -1;
            slot->usage = usage;
            slot->type = type;
            slot->idx = i;
            mConsumerList.add(combine_id, slot);
        }

#ifdef TOUCH_ION_BUFFER
        sp<GraphicBuffer> gb;
        producer->requestBuffer(buf, &gb);
        touchIonBuffer(gb);
#endif
    }

    for (uint32_t i = 0; i < bufNum; i++) {
        sp<Fence> fence = Fence::NO_FENCE;
        producer->cancelBuffer(i, fence);
    }

    return err;
}

#ifdef TOUCH_ION_BUFFER
buffer_handle_t GuiExtPoolItem::getNativehandle(android_native_buffer_t* buffer)
{
    if ((buffer->common.magic   == ANDROID_NATIVE_BUFFER_MAGIC) &&
        (buffer->common.version == sizeof(android_native_buffer_t)))
    {
        return buffer->handle;
    }
    return 0;
}

void GuiExtPoolItem::touchIonBuffer(sp<GraphicBuffer> gb)
{
    int ion_fd = -1;
    android_native_buffer_t* buffer = gb.get();
    buffer_handle_t hnd = getNativehandle(buffer);
    int err = 0;
    uint32_t bufSize = 0;
    GuiExtIONDevice &device(GuiExtIONDevice::getInstance());

    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_ION_FD, &ion_fd);

    uint32_t ion_hnd = device.import(ion_fd);
    device.touchMMUAddress(0, ion_hnd);
    close(ion_hnd);
}
#endif

status_t GuiExtPoolItem::acquire(const sp<IBinder>& token, uint32_t usage, uint32_t type, sp<BinderDiedListener> listener, int *buf)
{
    if (!mIsHwcNeeded && usage == GUI_EXT_USAGE_HWC) {
        GUIEXT_LOGW("    acquire a pool=%d not alloc hwc usage buffer", mId);
        return INVALID_OPERATION;
    }

    sp<IGraphicBufferProducer> producer = getIGraphicBufferProducer(usage, type);
    if (producer == NULL) {
        GUIEXT_LOGW("    acquire a pool=%d not alloc hwc usage buffer, type=%d", mId, type);
        return BAD_INDEX;
    }
    sp<Fence> fence;
    uint32_t fmt = gAcquiredFormat[usage];
    status_t ret = producer->dequeueBuffer(buf, &fence, false, 0, 0, fmt, LOCK_FOR_USAGE);
    if (ret == WOULD_BLOCK || *buf < 0) {
        GUIEXT_LOGW("    acquire a pool=%d has no free slot", mId);
        return WOULD_BLOCK;
    }

    class ConsumerDeathObserver : public IBinder::DeathRecipient {
        GuiExtPoolItem & mItem;
        virtual void binderDied(const wp<IBinder>& who) {
            uint32_t size = mItem.mConsumerList.size();
            GUIEXT_LOGI("consumer died, list size=%d, binder ptr=[%p]", size, who.unsafe_get());

            if (size == 0) {
                GUIEXT_LOGV("consumer died [%p], pool size is zero", who.unsafe_get());
            } else {
                for (uint32_t i = 0 ; i < size ; i++) {
                    GUIEXT_LOGV("   [c] compare index[%d], p=[%p], usage=%d, type=%d, id=%d", i,
                        mItem.mConsumerList[i]->token.get(),
                        mItem.mConsumerList[i]->usage, mItem.mConsumerList[i]->type, mItem.mConsumerList[i]->idx);
                    if (mItem.mConsumerList[i]->token.get() == who.unsafe_get()) {
                        GUIEXT_LOGW("   [c] found index[%d], p=[%p], usage=%d, type=%d, id=%d", i,
                            mItem.mConsumerList[i]->token.get(),
                            mItem.mConsumerList[i]->usage, mItem.mConsumerList[i]->type, mItem.mConsumerList[i]->idx);

                        mItem.release(mItem.mConsumerList[i]->usage, mItem.mConsumerList[i]->type, mItem.mConsumerList[i]->idx);
                        mItem.mConsumerList[i]->token = NULL;
                        mItem.mConsumerList[i]->pid = -1;
                        mItem.mConsumerList[i]->observer = NULL;
                    }
                }

                for (uint32_t i = 0; i < GUI_EXT_USAGE_MAX; i++) {
                    mItem.mIsDisconnected[i] = true;
                }
                bool isDisconnect = true;
                for (uint32_t i = 0 ; i < size ; i++) {
                    if (mItem.mConsumerList[i]->token != NULL) {
                        mItem.mIsDisconnected[mItem.mConsumerList[i]->usage] = false;
                        isDisconnect = false;
                        GUIEXT_LOGV("   [c] still have consumer, idx=%d, token=%p, pid=%d, usage=%d, type=%d, id=%d", i,
                            mItem.mConsumerList[i]->token.get(), mItem.mConsumerList[i]->pid,
                            mItem.mConsumerList[i]->usage, mItem.mConsumerList[i]->type, mItem.mConsumerList[i]->idx);
                    }
                }

                if (isDisconnect) {
                    mItem.mConsumerDeathListener->binderDied(mItem.mId);
                }

                GUIEXT_LOGV("consumer died done");
            }
        }
    public:
        ConsumerDeathObserver(GuiExtPoolItem & item) : mItem(item) { }
    };

    sp<IBinder::DeathRecipient> observer = new ConsumerDeathObserver(*const_cast<GuiExtPoolItem*>(this));
    token->linkToDeath(observer);

    uint32_t combine_id = POOL_COMBINED_ID(usage, type, *buf);
    sp<ConsumerSlot> consumerSlot = mConsumerList.valueFor(combine_id);
    if (consumerSlot == NULL) {
        GUIEXT_LOGW("    acquire a pool=%d not pre-alloc consumer slot", mId);
        return BAD_INDEX;
    }

    if (consumerSlot->token == NULL) {
        sp<ConsumerSlot> slot = new ConsumerSlot();
        slot->token = token;
        slot->usage = usage;
        slot->type = type;
        slot->idx = *buf;
        slot->pid = (NULL != token->localBinder())
                 ? getpid()
                 : IPCThreadState::self()->getCallingPid();
        slot->observer = observer;
        mConsumerList.add(combine_id, slot);
    }

    mIsDisconnected[usage] = false;
    mConsumerDeathListener = listener;

    return NO_ERROR;
}

status_t GuiExtPoolItem::request(uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer)
{
    if (!mIsHwcNeeded && usage == GUI_EXT_USAGE_HWC) {
        GUIEXT_LOGW("    request a pool=%d not alloc hwc usage buffer", mId);
        return INVALID_OPERATION;
    }
    // To-Do: support multiple hwc bq
    sp<IGraphicBufferProducer> producer = getIGraphicBufferProducer(usage, type);
    if (producer == NULL) {
        GUIEXT_LOGW("    request a pool=%d not alloc hwc usage buffer, type=%d", mId, type);
        return BAD_INDEX;
    }
    producer->requestBuffer(buf, buffer);

    return NO_ERROR;
}

status_t GuiExtPoolItem::release(uint32_t usage, uint32_t type, int buf)
{
    if (!mIsHwcNeeded && usage == GUI_EXT_USAGE_HWC) {
        GUIEXT_LOGW("    release a pool=%d not alloc hwc usage buffer", mId);
        return INVALID_OPERATION;
    }
    // To-Do: support multiple hwc bq
    sp<IGraphicBufferProducer> producer = getIGraphicBufferProducer(usage, type);
    if (producer == NULL) {
        GUIEXT_LOGW("    release a pool=%d not alloc hwc usage buffer, type=%d", mId, type);
        return BAD_INDEX;
    }
    sp<Fence> fence = Fence::NO_FENCE;
    producer->cancelBuffer(buf, fence);

    uint32_t combine_id = POOL_COMBINED_ID(usage, type, buf);
    sp<ConsumerSlot> consumerSlot = mConsumerList.valueFor(combine_id);
    if (consumerSlot != 0) {
        consumerSlot->token = NULL;
        consumerSlot->pid = -1;
        consumerSlot->observer = NULL;
        mConsumerList.add(combine_id, consumerSlot);
    }

    return NO_ERROR;
}

sp<IGraphicBufferProducer> GuiExtPoolItem::getIGraphicBufferProducer(uint32_t usage, uint32_t type)
{
    sp<IGraphicBufferProducer> producer = mGPUUsedProducer;
    if (usage == GUI_EXT_USAGE_HWC) {
#if SUPPORT_MULTIBQ_FOR_HWC
        sp<HwcBqSlot> slot = mHwcUsedBqList.valueFor(type);
        if (type >= HWC_NUM_DISPLAY_TYPES || slot == NULL) {
            GUIEXT_LOGE("    try to get HWC BufferQueue not has hwc display type=%d buffer\n", type);
            return NULL;
        }
        producer = slot->mProducer;
#else
        bq = mHwcUsedBq;
#endif
    }

    return producer;
}

status_t GuiExtPoolItem::disconnect(uint32_t usage, uint32_t /*type*/)
{
    if (!mIsHwcNeeded && usage == GUI_EXT_USAGE_HWC) {
        GUIEXT_LOGW("    release a pool=%d not alloc hwc usage buffer", mId);
        return INVALID_OPERATION;
    }

    mIsDisconnected[usage] = true;
    return NO_ERROR;
}

void GuiExtPoolItem::dump(String8& result) const
{
    int w, h;
    result.appendFormat("    "
                        "this=%p mId=%d, mIsHwcNeeded=%d, mProducerPid=%d, mProducerToken=%p\n",
                        this, mId, mIsHwcNeeded, mProducerPid, mProducerToken.get());

    if(mGPUUsedConsumer != NULL){
        result.appendFormat("    << GPU BQ >>\n");
        mGPUUsedConsumer->dump(result, "        ");

        result.appendFormat("    << GPU Consumer >> connected=%s\n",
                        mIsDisconnected[GUI_EXT_USAGE_GPU] ? "false" : "true");
        dumpConsumerSlots(result, GUI_EXT_USAGE_GPU, 0);
    }else{
        result.appendFormat("    << GPU BQ isn't generated  >>\n");
    }

    if (mIsHwcNeeded) {
#if SUPPORT_MULTIBQ_FOR_HWC
        for (uint32_t i = 0; i < mHwcUsedBqList.size(); i++) {
            result.appendFormat("    << HWC BQ >> type=%d\n", mHwcUsedBqList[i]->type);
            mHwcUsedBqList[i]->mConsumer->dump(result, "        ");

            result.appendFormat("    << HWC Consumer >> type=%d connected=%s\n",
                                mHwcUsedBqList[i]->type,
                                mIsDisconnected[GUI_EXT_USAGE_HWC] ? "false" : "true");
            dumpConsumerSlots(result, GUI_EXT_USAGE_HWC, mHwcUsedBqList[i]->type);
        }
#else
        result.appendFormat("    << HWC BQ >>\n");
        mHwcUsedBq->dump(result, "        ");

        result.appendFormat("    << HWC Consumer >> connected=%s\n",
                            mIsDisconnected[GUI_EXT_USAGE_HWC] ? "false" : "true");
        dumpConsumerSlots(result, GUI_EXT_USAGE_HWC, 0);
#endif
    }
}

void GuiExtPoolItem::dumpConsumerSlots(String8 & result, uint32_t usage, uint32_t type) const
{
    uint32_t list_size = mConsumerList.size();
    if (list_size > 0) {
        for (uint32_t i = 0; i < list_size; i++) {
            uint32_t combine_id = POOL_COMBINED_ID(usage, type, i);
            sp<ConsumerSlot> consumerSlot = mConsumerList.valueFor(combine_id);
            if (consumerSlot != 0) {
                result.append("        ");
                consumerSlot->dump(result);
            } else break;
        }
    }
}

void GuiExtPoolItem::ConsumerSlot::dump(String8& result) const
{
    result.appendFormat("[%02d] pid=%d, token=%p, observer=%p\n", idx, pid, token.get(), observer.get());
}

// ---------------------------------------------------------------------------

#ifdef TOUCH_ION_BUFFER
ANDROID_SINGLETON_STATIC_INSTANCE(GuiExtIONDevice);

GuiExtIONDevice::GuiExtIONDevice()
{
    m_dev_fd = open(ION_DEV_NODE, O_RDONLY);
    if (m_dev_fd <= 0)
    {
        GUIEXT_LOGE("Failed to open ION device: %s ", strerror(errno));
    }
}

GuiExtIONDevice::~GuiExtIONDevice()
{
    if (m_dev_fd > 0) close(m_dev_fd);
}

uint32_t GuiExtIONDevice::import(int ion_fd)
{
    if (m_dev_fd <= 0) return 0;

    struct ion_handle* ion_hnd;
    if (ion_import(m_dev_fd, ion_fd, &ion_hnd))
    {
        GUIEXT_LOGE("Failed to import ION handle: %s ", strerror(errno));
        return 0;
    }

    return (unsigned int)ion_hnd;
}

void GuiExtIONDevice::touchMMUAddress(int client, int ion_hnd)
{
    if (m_dev_fd <= 0) return;

    // configure before querying physical address
    struct ion_mm_data mm_data;
    mm_data.mm_cmd                        = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle    = (struct ion_handle *) ion_hnd;
    mm_data.config_buffer_param.eModuleID = client;
    mm_data.config_buffer_param.security  = 0;
    mm_data.config_buffer_param.coherent  = 0;

    int status = ion_custom_ioctl(m_dev_fd, ION_CMD_MULTIMEDIA, &mm_data);
    if ((status > 0) && (status != ION_ERROR_CONFIG_LOCKED))
    {
        GUIEXT_LOGE("Failed to config ION memory: %s", strerror(errno));
        return;
    }

    // get physical address
    struct ion_sys_data sys_data;
    sys_data.sys_cmd               = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = (struct ion_handle *) ion_hnd;
    if (ion_custom_ioctl(m_dev_fd, ION_CMD_SYSTEM, &sys_data))
    {
        GUIEXT_LOGE("Failed to get MVA from ION: %s", strerror(errno));
        return;
    }
}
#endif

}
