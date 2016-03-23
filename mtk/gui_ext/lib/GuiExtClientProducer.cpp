#define LOG_TAG "GuiExt"

#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include "GuiExtClientProducer.h"

namespace android {

#define GUIEXT_LOGV(x, ...) ALOGV("[GuiExtP] " x, ##__VA_ARGS__)
#define GUIEXT_LOGD(x, ...) ALOGD("[GuiExtP] " x, ##__VA_ARGS__)
#define GUIEXT_LOGI(x, ...) ALOGI("[GuiExtP] " x, ##__VA_ARGS__)
#define GUIEXT_LOGW(x, ...) ALOGW("[GuiExtP] " x, ##__VA_ARGS__)
#define GUIEXT_LOGE(x, ...) ALOGE("[GuiExtP] " x, ##__VA_ARGS__)

ANDROID_SINGLETON_STATIC_INSTANCE(GuiExtClientProducer);

GuiExtClientProducer::GuiExtClientProducer()
{
    assertStateLocked();
}

GuiExtClientProducer::~GuiExtClientProducer()
{
}

void GuiExtClientProducer::serviceDiedLocked()
{
    GUIEXT_LOGI("[%s]", __func__);
}

status_t GuiExtClientProducer::alloc(uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t *poolId)
{
    status_t err;
    Mutex::Autolock _l(mLock);
    err = assertStateLocked();
    if (err != NO_ERROR) {
        GUIEXT_LOGE("[%s] assertStateLocked fail", __func__);
        return err;
    }

    GUIEXT_LOGV("alloc +, gralloc_usage=%x, w=%d, h=%d", gralloc_usage, w, h);
    static sp<BBinder> sLife = new BBinder();
    err = mGuiExtService->alloc(sLife, gralloc_usage, w, h, poolId);
    GUIEXT_LOGV("alloc -, poolId=%d", *poolId);

    return err;
}

status_t GuiExtClientProducer::free(uint32_t poolId)
{
    status_t err;
    Mutex::Autolock _l(mLock);
    err = assertStateLocked();
    if (err != NO_ERROR) {
        GUIEXT_LOGE("[%s] assertStateLocked fail", __func__);
        return err;
    }

    GUIEXT_LOGV("free +, poolId=%d", poolId);
    err = mGuiExtService->free(poolId);
    GUIEXT_LOGV("free -, poolId=%d, ret=%d", poolId, err);

    return err;
}

};
