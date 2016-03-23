#define LOG_TAG "GuiExt"

#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <hardware/hwcomposer_defs.h>

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include "GuiExtClientConsumer.h"
#include "GuiExtDataType.h"

namespace android {

#define GUIEXT_LOGV(x, ...) ALOGV("[GuiExtC] " x, ##__VA_ARGS__)
#define GUIEXT_LOGD(x, ...) ALOGD("[GuiExtC] " x, ##__VA_ARGS__)
#define GUIEXT_LOGI(x, ...) ALOGI("[GuiExtC] " x, ##__VA_ARGS__)
#define GUIEXT_LOGW(x, ...) ALOGW("[GuiExtC] " x, ##__VA_ARGS__)
#define GUIEXT_LOGE(x, ...) ALOGE("[GuiExtC] " x, ##__VA_ARGS__)

#define POOL_ID_SHIFT(id)           (id << 10)
#define POOL_USAGE_SHIFT(usage)     (usage << 6)
#define POOL_TYPE_SHIFT(type)       (type << 3)
#define POOL_COMBINED_ID(id, usage, type, idx)   (POOL_ID_SHIFT(id)|POOL_USAGE_SHIFT(usage)|POOL_TYPE_SHIFT(type)|idx)

ANDROID_SINGLETON_STATIC_INSTANCE(GuiExtClientConsumer);

GuiExtClientConsumer::GuiExtClientConsumer()
    : mSlots(NULL)
{
    assertStateLocked();
}

GuiExtClientConsumer::~GuiExtClientConsumer()
{
}

void GuiExtClientConsumer::serviceDiedLocked()
{
    GUIEXT_LOGI("[%s]", __func__);

    mSlots.clear();
}

status_t GuiExtClientConsumer::acquire(android_native_buffer_t** buffer, uint32_t poolId, uint32_t usage, uint32_t type)
{
    status_t err;
    Mutex::Autolock _l(mLock);

    if (usage >= GUI_EXT_USAGE_MAX) {
        GUIEXT_LOGE("[%s] usage=%x is not support", __func__, usage);
        *buffer = NULL;
        return BAD_VALUE;
    }

    if (type > HWC_DISPLAY_VIRTUAL) {
        GUIEXT_LOGE("[%s] type=%d is not support", __func__, type);
        *buffer = NULL;
        return BAD_VALUE;
    }

    err = assertStateLocked();
    if (err != NO_ERROR) {
        GUIEXT_LOGE("[%s] assertStateLocked fail", __func__);
        *buffer = NULL;
        return err;
    }

    GUIEXT_LOGV("acquire +, poolId=%d, usage=%x, type=%d", poolId, usage, type);
    int buf = -1;
    static sp<BBinder> sLife = new BBinder();

    err = mGuiExtService->acquire(sLife, poolId, usage, type, &buf);
    if (err < 0) {
        for (uint32_t i = 0; i < mSlots.size(); i++) {
            if (mSlots[i] != NULL &&
                mSlots[i]->poolId == poolId &&
                mSlots[i]->usage == usage &&
                mSlots[i]->type == type &&
                !mSlots[i]->isAcquired) {
                mSlots[i]->isAcquired = true;
                *buffer = mSlots[i]->buffer.get();
                GUIEXT_LOGW("acquire fail=%d but find backup buffer: (poolId=%d, usage=%x, type=%d, idx=%d, buffer=%p) fail=%d",
                    err, poolId, usage, type, mSlots[i]->idx, *buffer);
                return NO_ERROR;
            }
        }

        GUIEXT_LOGE("acquire fail: (poolId=%d, usage=%x, type=%d) fail=%d", poolId, usage, type, err);
        *buffer = NULL;
        return err;
    }

    GUIEXT_LOGV("acquire done, poolId=%d, usage=%x, type=%d, buf=%d", poolId, usage, type, buf);

    uint32_t combine_id = POOL_COMBINED_ID(poolId, usage, type, buf);
    sp<BufferSlot> slot = mSlots.valueFor(combine_id);
    if (slot == 0) {
        slot = new BufferSlot();
        sp<GraphicBuffer> gbuf = slot->buffer;
        slot->poolId = poolId;
        slot->usage = usage;
        slot->type = type;
        slot->idx = buf;
        slot->isAcquired = true;
        err = mGuiExtService->request(poolId, usage, type, buf, &gbuf);
        if (err != NO_ERROR) {
            ALOGE("acquire: request failed: %d", err);
            *buffer = NULL;
            return err;
        }
        slot->buffer = gbuf;
        GUIEXT_LOGV("request done, poolId=%d, usage=%x, type=%d, buf=%d, buffer=%p, slot id=%x",
            poolId, usage, type, buf, slot->buffer.get(), combine_id);

        mSlots.add(combine_id, slot);
    } else
        GUIEXT_LOGV("acquire an exist shadow buffer, poolId=%d, usage=%x, type=%d, buf=%d, buffer=%p, slot id=%x",
            poolId, usage, type, buf, *buffer, combine_id);

    *buffer = slot->buffer.get();

    GUIEXT_LOGV("acquire -, poolId=%d, usage=%x, type=%d, buf=%d, buffer=%p", poolId, usage, type, buf, *buffer);

    return NO_ERROR;
}

status_t GuiExtClientConsumer::release(android_native_buffer_t* buffer, uint32_t poolId, uint32_t usage, uint32_t type)
{
    status_t err;
    Mutex::Autolock _l(mLock);

    if (usage >= GUI_EXT_USAGE_MAX) {
        GUIEXT_LOGE("[%s] usage=%x is not support", __func__, usage);
        return BAD_VALUE;
    }

    if (type > HWC_DISPLAY_VIRTUAL) {
        GUIEXT_LOGE("[%s] type=%d is not support", __func__, type);
        return BAD_VALUE;
    }

    err = assertStateLocked();
    if (err != NO_ERROR) {
        GUIEXT_LOGE("[%s] assertStateLocked fail", __func__);
        return err;
    }

    if (buffer == NULL) {
        GUIEXT_LOGW("[%s] release fail, buffer is NULL", __func__);
        return BAD_VALUE;
    }

    return releaseLocked(buffer, poolId, usage, type);
}

int GuiExtClientConsumer::getSlotFromBufferLocked(
        android_native_buffer_t* buffer) const {
    for (uint32_t i = 0; i < mSlots.size(); i++) {
        if (mSlots[i] != NULL && mSlots[i]->buffer->handle == buffer->handle) {
            return mSlots[i]->idx;
        }
    }
    GUIEXT_LOGW("getSlotFromBufferLocked: unknown buffer: %p", buffer);
    return BAD_VALUE;
}

status_t GuiExtClientConsumer::releaseLocked(android_native_buffer_t* buffer, uint32_t poolId, uint32_t usage, uint32_t type)
{
    int idx = getSlotFromBufferLocked(buffer);
    if (idx < 0) {
        GUIEXT_LOGE("releaseLocked fail, poolId=%d, usage=%x, type=%d, idx=%d", poolId, usage, type, idx);
        return idx;
    }

    uint32_t combine_id = POOL_COMBINED_ID(poolId, usage, type, idx);

    GUIEXT_LOGV("releaseLocked, poolId=%d, usage=%x, type=%d, idx=%d, slot id=%x", poolId, usage, type, idx, combine_id);

    sp<BufferSlot> slot = mSlots.valueFor(combine_id);
    if (slot != 0)
        slot->isAcquired = false;
    else
        GUIEXT_LOGW("releaseLocked, can't find shadow buffer, slot id=%x", combine_id);

    return mGuiExtService->release(poolId, usage, type, idx);
}

status_t GuiExtClientConsumer::disconnect(uint32_t poolId, uint32_t usage, uint32_t type)
{
    status_t err;
    Mutex::Autolock _l(mLock);

    if (usage >= GUI_EXT_USAGE_MAX) {
        GUIEXT_LOGE("[%s] usage=%x is not support", __func__, usage);
        return BAD_VALUE;
    }

    if (type > HWC_DISPLAY_VIRTUAL) {
        GUIEXT_LOGE("[%s] type=%d is not support", __func__, type);
        return BAD_VALUE;
    }

    err = assertStateLocked();
    if (err != NO_ERROR) {
        GUIEXT_LOGE("[%s] assertStateLocked fail", __func__);
        return err;
    }

    GUIEXT_LOGV("disconnect, poolId=%d, usage=%x, type=%d", poolId, usage, type);

    uint32_t size = mSlots.size();
    if (size == 0) {
        GUIEXT_LOGE("[%s] slot size is zero, no need to disconnect", __func__);
        return BAD_VALUE;
    }
    for (uint32_t i = size-1;; i--) {
        if (mSlots[i] != NULL && mSlots[i]->poolId == poolId && mSlots[i]->usage == usage && mSlots[i]->type == type) {
            uint32_t idx = mSlots[i]->idx;
            GUIEXT_LOGI("disconnect, find poolId=%d, usage=%x, type=%d, index=%d, isAcquired=%d",
                poolId, usage, type, idx, mSlots[i]->isAcquired);
            if (mSlots[i]->isAcquired)
                releaseLocked(mSlots[i]->buffer.get(), poolId, usage, type);
            uint32_t combine_id = POOL_COMBINED_ID(poolId, usage, type, idx);

            mSlots.removeItem(combine_id);
        }

        if (i == 0)
            break;
    }

    return mGuiExtService->disconnect(poolId, usage, type);
}

status_t GuiExtClientConsumer::configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum)
{
    status_t err;
    Mutex::Autolock _l(mLock);

    err = assertStateLocked();
    if (err != NO_ERROR) {
        GUIEXT_LOGE("[%s] assertStateLocked fail", __func__);
        return err;
    }

    return mGuiExtService->configDisplay(type, enable, w, h, bufNum);
}

bool GuiExtClientConsumer::isValidBuffer(android_native_buffer_t* buffer)
{
    status_t err;
    Mutex::Autolock _l(mLock);

    int idx = getSlotFromBufferLocked(buffer);
    if (idx < 0) {
        GUIEXT_LOGW("isValidBuffer return false for buffer=%p", buffer);
        return false;
    }

    return true;
}

void GuiExtClientConsumer::dump(String8& result, char* /*buffer*/, size_t /*SIZE*/) const
{
    uint32_t slotSize = mSlots.size();
    result.appendFormat("  -- GuiExtClientConsumer Info --\n"
                        "   Shadow Buffer Size=%d\n"
                        "--------------------------------\n", slotSize);

    for (uint32_t i = 0; i < slotSize; i++) {
        sp<BufferSlot> slot = mSlots[i];
        ANativeWindowBuffer* abuf = slot->buffer.get();
        result.appendFormat("      slot[%d], pool id=%d, usage=%d, type=%d, idx=%d, isAcquired=%d\n"
                            "        gb[ptr=%p, w=%d, h=%d, fmt=%x]\n", i,
                            slot->poolId, slot->usage, slot->type, slot->idx, slot->isAcquired,
                            abuf, abuf->width, abuf->height, abuf->format);
    }

    result.appendFormat("--------------------------------\n");
}

};
