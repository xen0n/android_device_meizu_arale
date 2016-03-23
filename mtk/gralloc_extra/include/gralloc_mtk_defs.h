#ifndef __GRALLOC_MTK_DEFS_H__
#define __GRALLOC_MTK_DEFS_H__

#include <stdint.h>
#include <hardware/gralloc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
   [Important]
   Define the MTK specific enumeration values for gralloc usage in order to avoid "polute" AOSP file
   (hardware/libhardware/include/hardware/gralloc.h)
   The enumeration value definition must not be conflict with the gralloc.h in original AOSP file
*/
enum {

#if 0
    /* buffer is never read in software */
    GRALLOC_USAGE_SW_READ_NEVER         = 0x00000000,
    /* buffer is rarely read in software */
    GRALLOC_USAGE_SW_READ_RARELY        = 0x00000002,
    /* buffer is often read in software */
    GRALLOC_USAGE_SW_READ_OFTEN         = 0x00000003,
    /* mask for the software read values */
    GRALLOC_USAGE_SW_READ_MASK          = 0x0000000F,

    /* buffer is never written in software */
    GRALLOC_USAGE_SW_WRITE_NEVER        = 0x00000000,
    /* buffer is rarely written in software */
    GRALLOC_USAGE_SW_WRITE_RARELY       = 0x00000020,
    /* buffer is often written in software */
    GRALLOC_USAGE_SW_WRITE_OFTEN        = 0x00000030,
    /* mask for the software write values */
    GRALLOC_USAGE_SW_WRITE_MASK         = 0x000000F0,

    /* buffer will be used as an OpenGL ES texture */
    GRALLOC_USAGE_HW_TEXTURE            = 0x00000100,
    /* buffer will be used as an OpenGL ES render target */
    GRALLOC_USAGE_HW_RENDER             = 0x00000200,
    /* buffer will be used by the 2D hardware blitter */
    GRALLOC_USAGE_HW_2D                 = 0x00000400,
    /* buffer will be used by the HWComposer HAL module */
    GRALLOC_USAGE_HW_COMPOSER           = 0x00000800,
    /* buffer will be used with the framebuffer device */
    GRALLOC_USAGE_HW_FB                 = 0x00001000,
    /* buffer will be used with the HW video encoder */
    GRALLOC_USAGE_HW_VIDEO_ENCODER      = 0x00010000,
    /* buffer will be written by the HW camera pipeline */
    GRALLOC_USAGE_HW_CAMERA_WRITE       = 0x00020000,
    /* buffer will be read by the HW camera pipeline */
    GRALLOC_USAGE_HW_CAMERA_READ        = 0x00040000,
    /* buffer will be used as part of zero-shutter-lag queue */
    GRALLOC_USAGE_HW_CAMERA_ZSL         = 0x00060000,
    /* mask for the camera access values */
    GRALLOC_USAGE_HW_CAMERA_MASK        = 0x00060000,
    /* mask for the software usage bit-mask */
    GRALLOC_USAGE_HW_MASK               = 0x00071F00,

    /* buffer will be used as a RenderScript Allocation */
    GRALLOC_USAGE_RENDERSCRIPT          = 0x00100000,

    /* buffer should be displayed full-screen on an external display when
     * possible
     */
    GRALLOC_USAGE_EXTERNAL_DISP         = 0x00002000,

    /* Must have a hardware-protected path to external display sink for
     * this buffer.  If a hardware-protected path is not available, then
     * either don't composite only this buffer (preferred) to the
     * external sink, or (less desirable) do not route the entire
     * composition to the external sink.
     */
    GRALLOC_USAGE_PROTECTED             = 0x00004000,

    /* implementation-specific private usage flags */
    GRALLOC_USAGE_PRIVATE_0             = 0x10000000,
    GRALLOC_USAGE_PRIVATE_1             = 0x20000000,
    GRALLOC_USAGE_PRIVATE_2             = 0x40000000,
    GRALLOC_USAGE_PRIVATE_3             = 0x80000000,
    GRALLOC_USAGE_PRIVATE_MASK          = 0xF0000000,
#endif /// #if 0

    /// the following define the extended gralloc enumeration value of
    GRALLOC_USAGE_NULL_BUFFER           = GRALLOC_USAGE_PRIVATE_3,
    GRALLOC_USAGE_SECURE                = 0x01000000,
    GRALLOC_USAGE_CAMERA_ORIENTATION   = 0x02000000,
};

#ifdef __cplusplus
}
#endif

#endif /* __GRALLOC_MTK_DEFS_H__ */
