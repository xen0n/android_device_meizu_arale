#ifndef MTK_GRALLOC_EXTRA_DEVICE_H
#define MTK_GRALLOC_EXTRA_DEVICE_H

#include <system/window.h>
#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <ui/gralloc_extra.h>

__BEGIN_DECLS

#define GRALLOC_HARDWARE_EXTRA "extra"

typedef struct extra_device_t {
    struct hw_device_t common;

    /** Deprecated methods and struct. START */
    int (*getIonFd)(struct extra_device_t* dev, buffer_handle_t handle, int *idx, int *num);
    int (*getBufInfo)(struct extra_device_t *dev, buffer_handle_t handle, gralloc_buffer_info_t* bufInfo);
    int (*getSecureBuffer)(struct extra_device_t* dev, buffer_handle_t handle, int *type, int *hBuffer);
    int (*setBufParameter)(struct extra_device_t* dev, buffer_handle_t handle, int mask, int value);
    int (*getMVA)(struct extra_device_t* dev, buffer_handle_t handle, int32_t *mvaddr);
    int (*setBufInfo)(struct extra_device_t* dev, buffer_handle_t handle, const char* str);
    /** Deprecated methods and struct. END */

    int (*query)(struct extra_device_t* dev,
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void * out_pointer);

    int (*perform)(struct extra_device_t* dev,
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void * in_pointer);
    #if 0
    int (*free_sec)(struct extra_device_t* dev, buffer_handle_t handle);
    #endif
	
    void* reserved_proc[2];
} extra_device_t;


/** convenience API for opening and closing a supported device */

static inline int gralloc_extra_open(const struct hw_module_t* module,
        struct extra_device_t** device) {
    return module->methods->open(module,
            GRALLOC_HARDWARE_EXTRA, (struct hw_device_t**)device);
}

static inline int gralloc_extra_close(struct extra_device_t* device) {
    return device->common.close(&device->common);
}

/* common function to init sf_info */
int gralloc_extra_sf_init(buffer_handle_t handle, gralloc_extra_ion_sf_info_t * sf_info);

__END_DECLS

#endif // MTK_GRALLOC_EXTRA_DEVICE_H
