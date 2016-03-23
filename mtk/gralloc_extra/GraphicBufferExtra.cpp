#define LOG_TAG "GraphicBufferExtra"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <cutils/log.h>
#include <ui/GraphicBufferExtra.h>
#include <graphics_mtk_defs.h>

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferExtra )

static GraphicBufferExtra * s_singleton_obj = NULL;

GraphicBufferExtra::GraphicBufferExtra()
    : mExtraDev(0)
{
    hw_module_t const* module;
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);

    ALOGE_IF(err, "FATAL: can't find the %s module", GRALLOC_HARDWARE_MODULE_ID);
    if (err == 0)
    {
        gralloc_extra_open(module, &mExtraDev);
    }

    s_singleton_obj = this;
}

// dlclose
static void __attribute__((destructor)) gralloc_extra_dlclose(void)
{
    if (s_singleton_obj)
    {
        delete s_singleton_obj;
        s_singleton_obj = NULL;
    }
}

GraphicBufferExtra::~GraphicBufferExtra()
{
    if (mExtraDev)
    {
        gralloc_extra_close(mExtraDev);
    }
    s_singleton_obj = NULL;
}

// ---------------------------------------------------------------------------
}; // namespace android
