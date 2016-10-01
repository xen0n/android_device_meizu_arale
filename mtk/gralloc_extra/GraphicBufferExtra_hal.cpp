
#define LOG_TAG "GraphicBufferExtra_hal"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdint.h>
#include <errno.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <system/graphics.h>
#include <graphics_mtk_defs.h>

#include <hardware/gralloc.h>
#include <hardware/gralloc_extra.h>

#include <ui/GraphicBufferExtra.h>

/* ----  IMPLEMENTATION macro  ---- */

#define GRALLOC_EXTRA_PROTOTYPE_(...)  ( __VA_ARGS__ )
#define GRALLOC_EXTRA_ARGS_(...)       ( __VA_ARGS__ )
#define GRALLOC_EXTRA_ARGS_2(...)      ( mExtraDev, __VA_ARGS__ )
#define GRALLOC_EXTRA_CHECK_FAIL_RETURN_(API, ERR)                      \
do {                                                                    \
    if (!mExtraDev)                                                     \
    {                                                                   \
        ALOGD("gralloc extra device is not supported");                 \
        return ERR;                                                     \
    }                                                                   \
                                                                        \
    if (!mExtraDev->API)                                                \
    {                                                                   \
        ALOGW("gralloc extra device " #API "(...) is not supported");   \
        return ERR;                                                     \
    }                                                                   \
} while(0)


#define GRALLOC_EXTRA_IMPLEMENTATION_(RET, ERR, API, PROTOTYPE, ARGS)   \
RET android::GraphicBufferExtra:: API PROTOTYPE                         \
{                                                                       \
    ATRACE_CALL();                                                      \
    RET err;                                                            \
                                                                        \
    GRALLOC_EXTRA_CHECK_FAIL_RETURN_(API, ERR);                         \
                                                                        \
    err = mExtraDev-> API GRALLOC_EXTRA_ARGS_2 ARGS ;                   \
                                                                        \
    if (err < 0) ALOGW(#API"(...) failed %d", err);                     \
                                                                        \
    return err;                                                         \
}                                                                       \
extern "C" RET gralloc_extra_##API PROTOTYPE                            \
{                                                                       \
    return android::GraphicBufferExtra::get(). API ARGS;                \
}

/* ----  IMPLEMENTATION start  ---- */

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getIonFd, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int *idx, int *num), 
    GRALLOC_EXTRA_ARGS_(handle, idx, num)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getSecureBuffer, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int *type, int *hBuffer), 
    GRALLOC_EXTRA_ARGS_(handle, type, hBuffer)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getBufInfo, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, gralloc_buffer_info_t* bufInfo), 
    GRALLOC_EXTRA_ARGS_(handle, bufInfo)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, setBufParameter, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int mask, int value), 
    GRALLOC_EXTRA_ARGS_(handle, mask, value)
)
	
GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getMVA, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int *mvaddr), 
    GRALLOC_EXTRA_ARGS_(handle, mvaddr)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, setBufInfo, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, const char * str), 
    GRALLOC_EXTRA_ARGS_(handle, str)
)

/* ----  IMPLEMENTATION start end  ---- */

#undef GRALLOC_EXTRA_CHECK_FAIL_RETURN_
#undef GRALLOC_EXTRA_IMPLEMENTATION_
#undef GRALLOC_EXTRA_PROTOTYPE_
#undef GRALLOC_EXTRA_ARGS_
#undef GRALLOC_EXTRA_ARGS_2

int android::GraphicBufferExtra::query(
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void * out_pointer) 
{ 
    ATRACE_CALL(); 
    int err; 
    if (!mExtraDev || !mExtraDev->query)
    {
        ALOGW("gralloc extra device query(...) is not supported");
        return -GRALLOC_EXTRA_NOT_SUPPORTED;
    }
    err = mExtraDev->query(mExtraDev, handle, attribute, out_pointer);
    if (err < 0) 
        ALOGW("query(0x%x) failed %d", attribute, err);
    return err; 
} 

int android::GraphicBufferExtra::perform(
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void * in_pointer) 
{ 
    ATRACE_CALL(); 
    int err; 
    if (!mExtraDev || !mExtraDev->perform)
    {
        ALOGW("gralloc extra device perform(...) is not supported");
        return -GRALLOC_EXTRA_NOT_SUPPORTED;
    }
    err = mExtraDev->perform(mExtraDev, handle, attribute, in_pointer); 
    if (err < 0)
        ALOGW("perform(0x%x) failed %d", attribute, err); 
    return err; 
} 


#if 0
int android::GraphicBufferExtra::free_sec(
        buffer_handle_t handle)
{
    ATRACE_CALL();
    int err;
    if (!mExtraDev || !mExtraDev->perform)
    {
        ALOGW("gralloc extra device perform(...) is not supported");
        return -GRALLOC_EXTRA_NOT_SUPPORTED;
    }
    err = mExtraDev->free_sec(mExtraDev, handle);
    if (err < 0)
        ALOGW("free_sec failed %d", err);
    return err;
}
#endif


extern "C" int gralloc_extra_query(
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void * out_pointer) 
{ 
    return android::GraphicBufferExtra::get().query(handle, attribute, out_pointer); 
}

extern "C" int gralloc_extra_perform(
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void * in_pointer) 
{ 
    return android::GraphicBufferExtra::get().perform(handle, attribute, in_pointer); 
}

#if 0
extern "C" int gralloc_extra_free_sec(buffer_handle_t handle )
{
    return android::GraphicBufferExtra::get().free_sec(handle);
}
#endif


static int __gralloc_extra_sf_set_int(int *data, int32_t mask, int32_t value)
{
    int32_t old_status;
    int32_t new_status;

    old_status = *data;
    new_status = ( old_status & (~mask) );
    new_status |= ( value & mask );

    /* Force modify the const member */
    *data = new_status;

    return GRALLOC_EXTRA_OK;
}

extern "C" int gralloc_extra_sf_set_status(gralloc_extra_ion_sf_info_t *sf_info, int32_t mask, int32_t value)
{
    return __gralloc_extra_sf_set_int(&sf_info->status, mask, value);
}

extern "C" int gralloc_extra_sf_set_status2(gralloc_extra_ion_sf_info_t *sf_info, int32_t mask, int32_t value)
{
    return __gralloc_extra_sf_set_int(&sf_info->status2, mask, value);
}

/* init the sf_info */
extern "C" int gralloc_extra_sf_init(buffer_handle_t handle, gralloc_extra_ion_sf_info_t * sf_info)
{
    int32_t mask = 0;
    int32_t value = 0;
    int format;

    memset(sf_info, 0, sizeof(gralloc_extra_ion_sf_info_t));

    if (GRALLOC_EXTRA_OK == gralloc_extra_query(handle, GRALLOC_EXTRA_GET_FORMAT, &format))
    {
        if (format == HAL_PIXEL_FORMAT_YV12 ||
            format == HAL_PIXEL_FORMAT_I420 ||
            format == HAL_PIXEL_FORMAT_YUV_PRIVATE ||
            format == HAL_PIXEL_FORMAT_NV12_BLK ||
            format == HAL_PIXEL_FORMAT_NV12_BLK_FCM ||
            format == HAL_PIXEL_FORMAT_YUYV ||
            0 )
        {
            mask |= GRALLOC_EXTRA_MASK_YUV_COLORSPACE;
            value |= GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
        }
    }

    gralloc_extra_sf_set_status(sf_info, mask, value);
    return 0;
}

