#ifndef MTK_GRALLOC_EXTRA_H
#define MTK_GRALLOC_EXTRA_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <system/window.h>

#include <hardware/gralloc.h>

__BEGIN_DECLS

enum {
    GRALLOC_EXTRA_OK,
    GRALLOC_EXTRA_UNKNOWN,
    GRALLOC_EXTRA_NOT_SUPPORTED,
    GRALLOC_EXTRA_NO_IMPLEMENTATION,
    GRALLOC_EXTRA_NOT_INIT,
    GRALLOC_EXTRA_INVALIDE_PARAMS,
    GRALLOC_EXTRA_INVALIDE_OPERATION,
    GRALLOC_EXTRA_ERROR,
};

typedef int GRALLOC_EXTRA_RESULT;

/* enum for query() */
typedef enum {
    GRALLOC_EXTRA_GET_ION_FD = 1,    /* int */
    GRALLOC_EXTRA_GET_FB_MVA,        /* uintptr_t, deprecated */
    GRALLOC_EXTRA_GET_SECURE_HANDLE, /* uint32_t */

    /* output: int */
    GRALLOC_EXTRA_GET_WIDTH = 10,
    GRALLOC_EXTRA_GET_HEIGHT,
    GRALLOC_EXTRA_GET_STRIDE,
    GRALLOC_EXTRA_GET_VERTICAL_STRIDE,
    GRALLOC_EXTRA_GET_ALLOC_SIZE,
    GRALLOC_EXTRA_GET_FORMAT,
    GRALLOC_EXTRA_GET_USAGE,
    GRALLOC_EXTRA_GET_VERTICAL_2ND_STRIDE,
    GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE,        

    /* output: gralloc_extra_sf_info_t */
    GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO = 100,

    /* output: gralloc_extra_buf_debug_t */
    GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG,

    /* output: uint32_t */
    GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC,

    /* output: gralloc_rotate_info_t */
    GRALLOC_EXTRA_GET_ROTATE_INFO,

    /* output: gralloc_gpu_compression_info_t */
    GRALLOC_EXTRA_GET_GPU_COMPRESSION_INFO,

    /* output: gralloc_gpu_yuyv rotation */
    GRALLOC_EXTRA_GET_ORIENTATION,
    
} GRALLOC_EXTRA_ATTRIBUTE_QUERY;

/* enum for perform() */
typedef enum {
    /* input: gralloc_extra_sf_info_t * */
    GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO = 100,

    /* input: gralloc_extra_buf_debug_t * */
    GRALLOC_EXTRA_SET_IOCTL_ION_DEBUG,

    /* input: NULL */
    GRALLOC_EXTRA_ALLOC_SECURE_BUFFER_HWC,
    GRALLOC_EXTRA_FREE_SEC_BUFFER_HWC,

    /* input: gralloc_gpu_yuyv rotation */
    GRALLOC_EXTRA_SET_ORIENTATION,
} GRALLOC_EXTRA_ATTRIBUTE_PERFORM;

int gralloc_extra_query(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void *out_pointer);

int gralloc_extra_perform(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void *in_pointer);

int gralloc_extra_free_sec(buffer_handle_t handle);


#define GRALLOC_EXTRA_MAKE_BIT(start_bit, index)        ( (index) << (start_bit) )
#define GRALLOC_EXTRA_MAKE_MASK(start_bit, end_bit)     ( ( ((unsigned int)-1) >> (sizeof(int) * __CHAR_BIT__ - 1 - (end_bit) + (start_bit) ) ) << (start_bit) )

#define GRALLOC_EXTRA_UNMAKE_BIT(start_bit, index)        ( (index) >> (start_bit) )


/* bits in sf_info.status */
enum {
    /* TYPE: bit 0~1 */
    GRALLOC_EXTRA_BIT_TYPE_CPU          = GRALLOC_EXTRA_MAKE_BIT(0,0),
    GRALLOC_EXTRA_BIT_TYPE_GPU          = GRALLOC_EXTRA_MAKE_BIT(0,1),
    GRALLOC_EXTRA_BIT_TYPE_VIDEO        = GRALLOC_EXTRA_MAKE_BIT(0,2),
    GRALLOC_EXTRA_BIT_TYPE_CAMERA       = GRALLOC_EXTRA_MAKE_BIT(0,3),
    GRALLOC_EXTRA_MASK_TYPE             = GRALLOC_EXTRA_MAKE_MASK(0,1),

    /* UFO alignment: bit 2~3 Temporary add back to prevent 6795 build fail. 
       Remove after 6795 SF/HWC finishes porting
     */
    GRALLOC_EXTRA_BIT_UFO_16_32         = GRALLOC_EXTRA_MAKE_BIT(2,1),
    GRALLOC_EXTRA_BIT_UFO_32_32         = GRALLOC_EXTRA_MAKE_BIT(2,2),
    GRALLOC_EXTRA_BIT_UFO_64_64         = GRALLOC_EXTRA_MAKE_BIT(2,3),
    GRALLOC_EXTRA_MASK_UFO_ALIGN        = GRALLOC_EXTRA_MAKE_MASK(2,3),

    /* ColorFormat: bit 2~6 */
    GRALLOC_EXTRA_BIT_CM_YV12           = GRALLOC_EXTRA_MAKE_BIT(2,0),
    GRALLOC_EXTRA_BIT_CM_YUYV           = GRALLOC_EXTRA_MAKE_BIT(2,1),
    GRALLOC_EXTRA_BIT_CM_I420           = GRALLOC_EXTRA_MAKE_BIT(2,2),
    GRALLOC_EXTRA_BIT_CM_NV12           = GRALLOC_EXTRA_MAKE_BIT(2,3),
    GRALLOC_EXTRA_BIT_CM_YUV420_FLEX    = GRALLOC_EXTRA_MAKE_BIT(2,4),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK       = GRALLOC_EXTRA_MAKE_BIT(2,5),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM   = GRALLOC_EXTRA_MAKE_BIT(2,6),
    GRALLOC_EXTRA_BIT_CM_UFO            = GRALLOC_EXTRA_MAKE_BIT(2,7),
    GRALLOC_EXTRA_BIT_CM_PRIVATE        = GRALLOC_EXTRA_MAKE_BIT(2,8),
    GRALLOC_EXTRA_BIT_CM_YV12_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,9),
    GRALLOC_EXTRA_BIT_CM_YUYV_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,10),
    GRALLOC_EXTRA_BIT_CM_I420_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,11),
    GRALLOC_EXTRA_BIT_CM_YUV420_FLEX_10BIT      = GRALLOC_EXTRA_MAKE_BIT(2,12),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H       = GRALLOC_EXTRA_MAKE_BIT(2,13),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V       = GRALLOC_EXTRA_MAKE_BIT(2,14),
    GRALLOC_EXTRA_BIT_CM_UFO_10BIT_H    = GRALLOC_EXTRA_MAKE_BIT(2,15),
    GRALLOC_EXTRA_BIT_CM_UFO_10BIT_V    = GRALLOC_EXTRA_MAKE_BIT(2,16),
    GRALLOC_EXTRA_BIT_CM_NV12_PRIVATE_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,17),
    GRALLOC_EXTRA_MASK_CM               = GRALLOC_EXTRA_MAKE_MASK(2,6),

    /* Secure switch: bit 7 */
    GRALLOC_EXTRA_BIT_NORMAL            = GRALLOC_EXTRA_MAKE_BIT(7,0),
    GRALLOC_EXTRA_BIT_SECURE            = GRALLOC_EXTRA_MAKE_BIT(7,1),
    GRALLOC_EXTRA_MASK_SECURE           = GRALLOC_EXTRA_MAKE_MASK(7,7),

    /* S3D Mode: bit 8~9 */
    GRALLOC_EXTRA_BIT_S3D_2D            = GRALLOC_EXTRA_MAKE_BIT(8,0),
    GRALLOC_EXTRA_BIT_S3D_SBS           = GRALLOC_EXTRA_MAKE_BIT(8,1),
    GRALLOC_EXTRA_BIT_S3D_TAB           = GRALLOC_EXTRA_MAKE_BIT(8,2),
    GRALLOC_EXTRA_MASK_S3D              = GRALLOC_EXTRA_MAKE_MASK(8,9),

    /* DIRTY: bit 10, 11 */
    GRALLOC_EXTRA_BIT_UNDIRTY           = GRALLOC_EXTRA_MAKE_BIT(10,0),
    GRALLOC_EXTRA_BIT_DIRTY             = GRALLOC_EXTRA_MAKE_MASK(10,11),
    GRALLOC_EXTRA_MASK_DIRTY            = GRALLOC_EXTRA_MAKE_MASK(10,11),
    /* DIRTY: SF, 10 */
    GRALLOC_EXTRA_BIT_SF_DIRTY          = GRALLOC_EXTRA_MAKE_MASK(10,10),
    GRALLOC_EXTRA_MASK_SF_DIRTY         = GRALLOC_EXTRA_MAKE_MASK(10,10),
    /* DIRTY: AUX, 11 */
    GRALLOC_EXTRA_BIT_AUX_DIRTY         = GRALLOC_EXTRA_MAKE_MASK(11,11),
    GRALLOC_EXTRA_MASK_AUX_DIRTY        = GRALLOC_EXTRA_MAKE_MASK(11,11),

    /* Orientation: bit 12~15 */
    GRALLOC_EXTRA_MASK_ORIENT           = GRALLOC_EXTRA_MAKE_MASK(12,15),

    /* Alpha: bit 16~23 */
    GRALLOC_EXTRA_MASK_ALPHA            = GRALLOC_EXTRA_MAKE_MASK(16,23),

    /* Blending: bit 24~25 */
    GRALLOC_EXTRA_MASK_BLEND            = GRALLOC_EXTRA_MAKE_MASK(24,25),

    /* Dirty Param: bit 26 */
    GRALLOC_EXTRA_MASK_DIRTY_PARAM      = GRALLOC_EXTRA_MAKE_MASK(26,26),

    /* FLUSH: bit 27 */
    GRALLOC_EXTRA_BIT_FLUSH             = GRALLOC_EXTRA_MAKE_BIT(27,0),
    GRALLOC_EXTRA_BIT_NOFLUSH           = GRALLOC_EXTRA_MAKE_BIT(27,1),
    GRALLOC_EXTRA_MASK_FLUSH            = GRALLOC_EXTRA_MAKE_MASK(27,27),

    /* YUV Color SPACE: bit 29~31 */
    GRALLOC_EXTRA_BIT_YUV_NOT_SET       = GRALLOC_EXTRA_MAKE_BIT(29,0),
    GRALLOC_EXTRA_BIT_YUV_BT601_NARROW  = GRALLOC_EXTRA_MAKE_BIT(29,1),
    GRALLOC_EXTRA_BIT_YUV_BT601_FULL    = GRALLOC_EXTRA_MAKE_BIT(29,2),
    GRALLOC_EXTRA_BIT_YUV_BT709_NARROW  = GRALLOC_EXTRA_MAKE_BIT(29,3),
    GRALLOC_EXTRA_BIT_YUV_BT709_FULL    = GRALLOC_EXTRA_MAKE_BIT(29,4),
    GRALLOC_EXTRA_BIT_YUV_BT2020_NARROW = GRALLOC_EXTRA_MAKE_BIT(29,5),
    GRALLOC_EXTRA_MASK_YUV_COLORSPACE   = GRALLOC_EXTRA_MAKE_MASK(29,31),
};

/* bits in sf_info.status2 */
enum {
    /* TYPE: bit 0~2 */
    GRALLOC_EXTRA_BIT2_LAYER_NORMAL      = 0x0,
    GRALLOC_EXTRA_BIT2_LAYER_NAV         = 0x1, // Navigation bar
    GRALLOC_EXTRA_MASK2_LAYER_TYPE       = GRALLOC_EXTRA_MAKE_MASK(0,2),

    /* YUY2 for Camera Rotation : bit 3~5 */
    GRALLOC_EXTRA_BIT2_YUY2_ROT0            = GRALLOC_EXTRA_MAKE_BIT(3,0),
    GRALLOC_EXTRA_BIT2_YUY2_R1_FLIP_H      = GRALLOC_EXTRA_MAKE_BIT(3,1),    
    GRALLOC_EXTRA_BIT2_YUY2_R2_FLIP_V      = GRALLOC_EXTRA_MAKE_BIT(3,2),    
    GRALLOC_EXTRA_BIT2_YUY2_R3_ROT180      = GRALLOC_EXTRA_MAKE_BIT(3,3),
    GRALLOC_EXTRA_BIT2_YUY2_R4_ROT90       = GRALLOC_EXTRA_MAKE_BIT(3,4),
    GRALLOC_EXTRA_BIT2_YUY2_R5              = GRALLOC_EXTRA_MAKE_BIT(3,5),
    GRALLOC_EXTRA_BIT2_YUY2_R6              = GRALLOC_EXTRA_MAKE_BIT(3,6),
    GRALLOC_EXTRA_BIT2_YUY2_R7_ROT270      = GRALLOC_EXTRA_MAKE_BIT(3,7),
    GRALLOC_EXTRA_MASK2_YUY2_ROT            = GRALLOC_EXTRA_MAKE_MASK(3,5),
    
};

typedef enum {
    /* normal secure buffer */
    GRALLOC_EXTRA_SECURE_BUFFER_TYPE_NORMAL,

    /* zero-initialized secure buffer */
    GRALLOC_EXTRA_SECURE_BUFFER_TYPE_ZERO
} GRALLOC_EXTRA_SECURE_BUFFER_TYPE;

typedef struct _crop_t {
    int32_t x, y, w, h;

#ifdef __cplusplus
    _crop_t():x(0),y(0),w(0),h(0) {}
#endif
} _crop_t;

/* extension data */
typedef struct gralloc_extra_ion_sf_info_t {
    /* magic number to make sure that obj is created by query() */
    uint32_t magic;

    /* used by SurfaceFlinger*/
    _crop_t src_crop;
    _crop_t dst_crop;

    /* buffer flags, please change this value by sf_set_status() */
    int32_t status;

    /* used for pre-allocate buffer */
    int32_t pool_id;

    /* used for profiling latency */
    uint32_t sequence;

    /* secure handle allocated by GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC */
    uint32_t secure_handle_hwc;

    /* used for video frame */
    uint32_t timestamp;

    /* buffer flags, please change this value by sf_set_status2() */
    int32_t status2;

    /* video buffer status
       bit 31: valid bit, 0 = invalid (default), 1 = valid

       bit 30-25: Y stride alignment
       bit 24-19: CbCr stride alignment
       bit 18-13: height alignment
       alignment interpretation

       Y:
       y_align = (videobuffer_status & 0x7FFFFFFF) >> 25;
       if (0 == y_align) y_align = 1;
       else y_align *= 2;
       CbCr:
       cbcr_align = (videobuffer_status & 0x01FFFFFF) >> 19;
       if (0 == cbcr_align) cbcr_align = 1;
       else cbcr_align *= 2;
       Height:
       h_align = (videobuffer_status & 0x0007FFFF) >> 13;
       if (0 == h_align) h_align = 1;
       else h_align *= 2;

       bit 12: deinterlace 0 = no, 1 = yes
    */
    int32_t videobuffer_status;

#ifdef __cplusplus
    gralloc_extra_ion_sf_info_t():
        magic(0),src_crop(),dst_crop(),
        status(0),pool_id(0),sequence(0),
        secure_handle_hwc(0),
        timestamp(0), status2(0)
    {}
#endif
} gralloc_extra_ion_sf_info_t;

int gralloc_extra_sf_set_status(gralloc_extra_ion_sf_info_t *sf_info, int32_t mask, int32_t value);

int gralloc_extra_sf_set_status2(gralloc_extra_ion_sf_info_t *sf_info, int32_t mask, int32_t value);

/* debug data, to facilitate information while debugging */
typedef struct gralloc_extra_ion_debug_t {
    int data[4];
    char name[16];

#ifdef __cplusplus
    gralloc_extra_ion_debug_t():
        data(),
        name()
    {}
#endif
} gralloc_extra_ion_debug_t;

typedef struct gralloc_rotate_info_t {
    //rotate
    int rotate_mode;
    int rotate_dx;
    int rotate_dy;
} gralloc_rotate_info_t;

typedef struct gralloc_gpu_compression_info_t {
    /* GPU info */
    //UFO
    unsigned char ufo_enable;
    //unsigned int ufo_format;
    //unsigned char ufo_is_linear;
    unsigned int ufo_length_buffer_base;
    unsigned char ufo_pattern_replace_en;
    unsigned int ufo_pattern;
} gralloc_gpu_compression_info_t;

/* Deprecated methods and struct. START */
typedef struct gralloc_buffer_info_t {
    /* static number, never change */
    int width;
    int height;
    int stride;
    int format;
    int vertical_stride;
    int usage;

    /* change by setBufParameter() */
    int status;
} gralloc_buffer_info_t;

int gralloc_extra_getIonFd(buffer_handle_t handle, int *idx, int *num);
int gralloc_extra_getBufInfo(buffer_handle_t handle, gralloc_buffer_info_t* bufInfo);
int gralloc_extra_getSecureBuffer(buffer_handle_t handle, int *type, int *hBuffer);
int gralloc_extra_setBufParameter(buffer_handle_t handle, int mask, int value);
int gralloc_extra_getMVA(buffer_handle_t handle, int32_t *mvaddr);
int gralloc_extra_setBufInfo(buffer_handle_t handle, const char * str);
/* Deprecated methods and struct. END */

__END_DECLS

#endif /* MTK_GRALLOC_EXTRA_H */
