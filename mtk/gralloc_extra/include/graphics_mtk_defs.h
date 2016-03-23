#ifndef __GRAPHICS_MTK_DEFS_H__
#define __GRAPHICS_MTK_DEFS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
   [Important]
   Define the OMX MTK specific enum values in order to avoid "polute" AOSP files(system/core/graphics.h)
   The format enum value definition must be align to the enum value in the following file
   frameworks\av\media\libstagefright\include\omx_core\OMX_IVCommon.h
*/
#define OMX_COLOR_FormatVendorMTKYUV     0x7F000001
#define OMX_COLOR_FormatVendorMTKYUV_FCM 0x7F000002
#define OMX_COLOR_FormatVendorMTKYUV_10BIT_H 0x7F000004
#define OMX_COLOR_FormatVendorMTKYUV_10BIT_V 0x7F000005
#define OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H 0x7F000006
#define OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V 0x7F000007

/**
 * MTK pixel format definitions
 */
enum {
#ifndef MTK_BUILD_IMG_DDK
    HAL_PIXEL_FORMAT_BGRX_8888      = 0xFFFF0002,
#endif

    HAL_PIXEL_FORMAT_I420           = 0x32315659 + 0x10,                 /// MTK I420
    HAL_PIXEL_FORMAT_YUV_PRIVATE    = 0x32315659 + 0x20,                 /// I420 or NV12_BLK or NV12_BLK_FCM
    HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT    = 0x32315659 + 0x30,           /// I420 or NV12_BLK or NV12_BLK_FCM - 10bit

    HAL_PIXEL_FORMAT_NV12_BLK       = OMX_COLOR_FormatVendorMTKYUV,      /// MTK NV12 block progressive mode
    HAL_PIXEL_FORMAT_NV12_BLK_FCM   = OMX_COLOR_FormatVendorMTKYUV_FCM,  /// MTK NV12 block field mode
    HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H = OMX_COLOR_FormatVendorMTKYUV_10BIT_H,  /// MTK NV12 10 bit horizontal mode
    HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V = OMX_COLOR_FormatVendorMTKYUV_10BIT_V,  /// MTK NV12 10 bit vertical mode
    HAL_PIXEL_FORMAT_UFO_10BIT_H    = OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H,  /// MTK NV12 10 bit UFO horizontal mode
    HAL_PIXEL_FORMAT_UFO_10BIT_V    = OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V,  /// MTK NV12 10 bit UFO vertical mode

/**
 * IMG Driver use 0x1X0-0x1X6 as they "vendor formats"
 */
    HAL_PIXEL_FORMAT_IMG0           = 0x00000100,
    HAL_PIXEL_FORMAT_IMG1_BGRX_8888 = 0x00000101,
    HAL_PIXEL_FORMAT_IMG2           = 0x00000102,
    HAL_PIXEL_FORMAT_IMG3           = 0x00000103,
    HAL_PIXEL_FORMAT_IMG4           = 0x00000105,
    HAL_PIXEL_FORMAT_IMG5           = 0x00000106,

    HAL_PIXEL_FORMAT_YUYV           = 0x0000010c,                        /// for HWC output (bpp=2)
    HAL_PIXEL_FORMAT_I420_DI        = 0x0000010d,                        /// MTK I420 for deinterlace
    HAL_PIXEL_FORMAT_YV12_DI        = 0x0000010e,                        /// MTK YV12 for deinterlace
    HAL_PIXEL_FORMAT_UFO            = 0x0000010f,                        /// MTK UFO
    HAL_PIXEL_FORMAT_NV12           = 0x00000110,                        /// NV12

/**
 * Sapphire Driver use 0x120-0x121 as their "vendor formats"
 */
    HAL_PIXEL_FORMAT_SPH0           = 0x00000120,
    HAL_PIXEL_FORMAT_SPH1           = 0x00000121,
};

#ifdef __cplusplus
}
#endif

#endif /* __GRAPHICS_MTK_DEFS_H__ */
