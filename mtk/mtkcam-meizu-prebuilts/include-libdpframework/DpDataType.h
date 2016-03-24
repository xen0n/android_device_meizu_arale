#ifndef __DP_DATA_TYPE_H__
#define __DP_DATA_TYPE_H__

#ifndef __KERNEL__
#include "DpConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#endif
#if CONFIG_FOR_OS_WINDOWS
    #include <stddef.h>
    typedef signed char     int8_t;
    typedef unsigned char   uint8_t;
    typedef signed short    int16_t;
    typedef unsigned short  uint16_t;
    typedef signed int      int32_t;
    typedef unsigned int    uint32_t;
    typedef signed long long     int64_t;
    typedef unsigned long long   uint64_t;
#ifndef __unused
    #define __unused
#endif

inline float roundf(float x)
{
    float t;

    if (x >= 0.0) {
        t = floorf(x);
        if (t - x <= -0.5)
            t += 1.0;
        return (t);
    } else {
        t = floorf(-x);
        if (t + x <= -0.5)
            t += 1.0;
        return (-t);
    }
}
#endif  // CONFIG_FOR_OS_WINDOWS

#ifndef MAX
    #define MAX(x, y)   ((x) >= (y))? (x): (y)
#endif // MAX

#ifndef MIN
    #define MIN(x, y)   ((x) <= (y))? (x): (y)
#endif  // MIN

#ifndef __KERNEL__
class DpStream;
class DpChannel;

class DpBasicBufferPool;
class DpAutoBufferPool;
class DpCommand;
class DpBufferPool;
#endif

typedef unsigned long long DpJobID;
typedef int DpEngineType;

typedef enum DP_STATUS_ENUM
{
    DP_STATUS_ABORTED_BY_USER   =  4,
    DP_STATUS_ALL_TEST_DONE     =  3,
    DP_STATUS_ALL_TPIPE_DONE    =  2,
    DP_STATUS_BUFFER_DONE       =  1,
    DP_STATUS_RETURN_SUCCESS    =  0,
    DP_STATUS_INVALID_PARAX     = -1,
    DP_STATUS_INVALID_PORT      = -2,
    DP_STATUS_INVALID_PATH      = -3,
    DP_STATUS_INVALID_FILE      = -4,
    DP_STATUS_INVALID_CHANNEL   = -5,
    DP_STATUS_INVALID_BUFFER    = -6,
    DP_STATUS_INVALID_STATE     = -7,
    DP_STATUS_INVALID_ENGINE    = -8,
    DP_STATUS_INVALID_FORMAT    = -9,
    DP_STATUS_INVALID_X_INPUT   = -10,
    DP_STATUS_INVALID_Y_INPUT   = -11,
    DP_STATUS_INVALID_X_OUTPUT  = -12,
    DP_STATUS_INVALID_Y_OUTPUT  = -13,
    DP_STATUS_INVALID_X_ALIGN   = -14,
    DP_STATUS_INVALID_Y_ALIGN   = -15,
    DP_STATUS_INVALID_WIDTH     = -16,
    DP_STATUS_INVALID_HEIGHT    = -17,
    DP_STATUS_INVALID_CROP      = -18,
    DP_STATUS_INVALID_ANGLE     = -19,
    DP_STATUS_INVALID_EVENT     = -20,
    DP_STATUS_INVALID_OPCODE    = -21,
    DP_STATUS_CAN_NOT_MERGE     = -22,
    DP_STATUS_OUT_OF_MEMORY     = -23,
    DP_STATUS_BUFFER_FULL       = -24,
    DP_STATUS_BUFFER_EMPTY      = -25,
    DP_STATUS_OPERATION_FAILED  = -26,
    DP_STATUS_OVER_MAX_BRANCH   = -27,
    DP_STATUS_OVER_MAX_ENGINE   = -28,
    DP_STATUS_OVER_MAX_BACKUP   = -29,
    DP_STATUS_SCHEDULE_ERROR    = -30,
    DP_STATUS_OVER_MAX_WIDTH    = -31,
    DP_STATUS_OVER_MAX_HEIGHT   = -32,
    DP_STATUS_LEFT_EDGE_ERROR   = -33,
    DP_STATUS_RIGHT_EDGE_ERROR  = -34,
    DP_STATUS_TOP_EDGE_ERROR    = -35,
    DP_STATUS_BOTTOM_EDGE_ERROR = -36,
    DP_STATUS_X_LESS_THAN_LAST  = -37,
    DP_STATUS_Y_LESS_THAN_LAST  = -38,
    DP_STATUS_UNWANTED_X_CAL    = -39,
    DP_STATUS_LOSS_OVER_WIDTH   = -40,
    DP_STATUS_LOSS_OVER_HEIGHT  = -41,
    DP_STATUS_X_ALIGN_ERROR     = -42,
    DP_STATUS_Y_ALIGN_ERROR     = -43,
    DP_STATUS_X_OUT_OVERLAP     = -44,
    DP_STATUS_Y_OUT_OVERLAP     = -45,
    DP_STATUS_BACK_LE_FORWARD   = -46,
    DP_STATUS_UNKNOWN_ERROR     = -47,
} DP_STATUS_ENUM;


typedef enum DP_MEMORY_ENUM
{
    DP_MEMORY_VA,
    DP_MEMORY_ION,
    DP_MEMORY_PHY,
    DP_MEMORY_MVA
} DP_MEMORY_ENUM;

typedef struct DpJPEGEnc_Config_st // for JPEG port only
{
    int32_t     fileDesc;
    uint32_t    size;
    uint32_t    fQuality;
    uint32_t    soi_en;
    void        *memSWAddr[3];
} DpJPEGEnc_Config;

typedef struct DpVEnc_Config // for VENC port only
{
    /* Venc Modify + */
    unsigned long rVencDrvHandle;
    /* Venc Modify - */
    uint32_t    memYUVMVAAddr[3];
    uint32_t    memYUVMVASize[3];
    void        *memYUVSWAddr[3];
    void        *memOutputSWAddr[3];

    uint32_t*   pNumPABuffer;
    uint32_t*   pPABuffer;
    uint64_t*   pConfigFrameCount;
    uint64_t*   pDequeueFrameCount;
    DpCommand*  pVEncCommander;
} DpVEnc_Config;


#ifndef __KERNEL__
class DpRect
{
public:

    enum
    {
        eINVALID_VALUE = -1,
        eINITIAL_VALUE = 0   //TBD, why to set as "0"?
    };

    inline DpRect(void)
        : x(eINITIAL_VALUE), sub_x(eINITIAL_VALUE),
          y(eINITIAL_VALUE), sub_y(eINITIAL_VALUE),
          w(eINITIAL_VALUE), h(eINITIAL_VALUE)
    {}

    inline DpRect(int32_t in_x, int32_t in_y, int32_t in_w, int32_t in_h,
                  int32_t in_sub_x = 0, int32_t in_sub_y = 0)
        : x(in_x),
          sub_x(in_sub_x),
          y(in_y),
          sub_y(in_sub_y),
          w(in_w),
          h(in_h)
    {}

    inline DpRect(const DpRect& rt)
        : x(rt.x),
          sub_x(rt.sub_x),
          y(rt.y),
          sub_y(rt.sub_y),
          w(rt.w),
          h(rt.h)
    {}

    ~DpRect(void) {}

    inline DpRect& operator=  (const DpRect rval)
    {
        x = rval.x;
        sub_x = rval.sub_x;
        y = rval.y;
        sub_y = rval.sub_y;
        w = rval.w;
        h = rval.h;
        return *this;
    }

    int32_t x;
    int32_t sub_x;
    int32_t y;
    int32_t sub_y;
    int32_t w;
    int32_t h;
};
#endif

class DpColorMatrix
{
    /* Define the color matrix.
     *
     * Color matrix would be used in the following transforms:
     * YUV2RGB
     * / R \   / c00 c01 c02 \   / Y - i0 \
     * | G | = | c10 c11 c12 | * | U - i1 |
     * \ B /   \ c20 c21 c22 /   \ V - i2 /
     * YUV2YUV
     * / Y \   / c00 c01 c02 \   / Y - i0 \   / o0 \
     * | U | = | c10 c11 c12 | * | U - i1 | + | o1 |
     * \ V /   \ c20 c21 c22 /   \ V - i2 /   \ o2 /
     *
     * Coefficient value range: -4.00 ~ 3.99
     */

public:
    bool enable;
    float c00; float c01; float c02;
    float c10; float c11; float c12;
    float c20; float c21; float c22;

    inline DpColorMatrix(void)
        : enable(false),
          c00(1.0), c01(0.0), c02(0.0),
          c10(0.0), c11(1.0), c12(0.0),
          c20(0.0), c21(0.0), c22(1.0)
    {}

    inline DpColorMatrix(float in_c00, float in_c01, float in_c02,
                         float in_c10, float in_c11, float in_c12,
                         float in_c20, float in_c21, float in_c22,
                         bool in_enable = true)
        : enable(in_enable),
          c00(in_c00), c01(in_c01), c02(in_c02),
          c10(in_c10), c11(in_c11), c12(in_c12),
          c20(in_c20), c21(in_c21), c22(in_c22)
    {}

    inline DpColorMatrix(const DpColorMatrix& m)
        : enable(m.enable),
          c00(m.c00), c01(m.c01), c02(m.c02),
          c10(m.c10), c11(m.c11), c12(m.c12),
          c20(m.c20), c21(m.c21), c22(m.c22)
    {}

    ~DpColorMatrix(void) {}

    inline DpColorMatrix& operator= (const DpColorMatrix& r)
    {
        if (this != &r)
        {
            enable = r.enable;
            c00 = r.c00; c01 = r.c01; c02 = r.c02;
            c10 = r.c10; c11 = r.c11; c12 = r.c12;
            c20 = r.c20; c21 = r.c21; c22 = r.c22;
        }

        return *this;
    }

    inline DpColorMatrix operator* (const DpColorMatrix& r)
    {
        if (!enable)
        {
            if (!r.enable)
            {
                return DpColorMatrix();
            }
            return DpColorMatrix(r);
        }
        if (!r.enable)
        {
            return DpColorMatrix(*this);
        }

        return DpColorMatrix(
            c00 * r.c00 + c01 * r.c10 + c02 * r.c20, c00 * r.c01 + c01 * r.c11 + c02 * r.c21, c00 * r.c02 + c01 * r.c12 + c02 * r.c22,
            c10 * r.c00 + c11 * r.c10 + c12 * r.c20, c10 * r.c01 + c11 * r.c11 + c12 * r.c21, c10 * r.c02 + c11 * r.c12 + c12 * r.c22,
            c20 * r.c00 + c21 * r.c10 + c22 * r.c20, c20 * r.c01 + c21 * r.c11 + c22 * r.c21, c20 * r.c02 + c21 * r.c12 + c22 * r.c22);
    }

    inline bool operator== (const DpColorMatrix& r)
    {
        if (this == &r)
        {
            return true;
        }
        if (!enable && !r.enable)
        {
            return true;
        }
        return (enable == r.enable) &&
            (c00 == r.c00) && (c01 == r.c01) && (c02 == r.c02) &&
            (c10 == r.c10) && (c11 == r.c11) && (c12 == r.c12) &&
            (c20 == r.c20) && (c21 == r.c21) && (c22 == r.c22);
    }

    inline bool operator!= (const DpColorMatrix& r)
    {
        return !(*this == r);
    }
};

inline DpColorMatrix operator* (const float l, const DpColorMatrix& r)
{
    return DpColorMatrix(
        l * r.c00, l * r.c01, l * r.c02,
        l * r.c10, l * r.c11, l * r.c12,
        l * r.c20, l * r.c21, l * r.c22,
        r.enable);
}


typedef enum DP_PROFILE_ENUM
{
    DP_PROFILE_BT601, //Limited range
    DP_PROFILE_BT709,
    DP_PROFILE_JPEG,
    DP_PROFILE_FULL_BT601 = DP_PROFILE_JPEG
} DP_PROFILE_ENUM;


typedef enum DP_STREAM_ID_ENUM
{
    DP_BLITSTREAM       = 0x10000000,
    DP_FRAGSTREAM       = 0x20000000,
    DP_ISPSTREAM        = 0x30000000,
    DP_ASYNCBLITSTREAM  = 0x40000000,
    DP_VENCSTREAM       = 0x50000000,
    DP_UNKNOWN_STREAM   = 0xF0000000,
} DP_STREAM_ID_ENUM;

typedef enum DP_MEDIA_TYPE_ENUM
{
    MEDIA_UNKNOWN,
    MEDIA_VIDEO,
    MEDIA_PICTURE,
    MEDIA_ISP_PREVIEW
} DP_MEDIA_TYPE_ENUM;

typedef struct
{
    uint32_t id;
    uint32_t timeStamp;
    uint32_t reserved[28];   // padding and reserved
} DpVideoParam;

typedef struct
{
    bool withHist;
    uint32_t info[20];
    uint32_t iso;
    uint32_t reserved[8];  // padding and reserved
} DpImageParam;

typedef struct
{
    uint32_t iso;
    uint32_t reserved[29];
} DpIspParam;


struct DpPqParam {
    bool enable;
    DP_MEDIA_TYPE_ENUM scenario;

    union {
        DpVideoParam video;
        DpImageParam image;
        DpIspParam isp;
    } u;
};

struct DpPqConfig {
    uint32_t enSharp;
    uint32_t enDC;
    uint32_t enColor;
};


// Format group: 0-RGB, 1-YUV, 2-Bayer raw, 3-compressed format
#define DP_COLORFMT_PACK(PACKED, LOOSE, VIDEO, PLANE, COPLANE, HFACTOR, VFACTOR, BITS, GROUP ,SWAP_ENABLE, UNIQUEID)  \
    ((PACKED        << 31) |                                                             \
     (LOOSE         << 30) |                                                             \
     (VIDEO         << 27) |                                                             \
     (PLANE         << 24) |                                                             \
     (COPLANE       << 22) |                                                             \
     (HFACTOR       << 20) |                                                             \
     (VFACTOR       << 18) |                                                             \
     (BITS          << 8)  |                                                             \
     (GROUP         << 6)  |                                                             \
     (SWAP_ENABLE   << 5)  |                                                             \
     (UNIQUEID      << 0))

#define DP_COLOR_GET_10BIT_PACKED(color)        ((0x80000000 & color) >> 31)
#define DP_COLOR_GET_10BIT_LOOSE(color)        (((0xC0000000 & color) >> 30) == 1)
#define DP_COLOR_GET_10BIT_TILE_MODE(color)    (((0xC0000000 & color) >> 30) == 3)
#define DP_COLOR_GET_UFP_ENABLE(color)          ((0x20000000 & color) >> 29)
#define DP_COLOR_GET_INTERLACED_MODE(color)     ((0x10000000 & color) >> 28)
#define DP_COLOR_GET_BLOCK_MODE(color)          ((0x08000000 & color) >> 27)
#define DP_COLOR_GET_PLANE_COUNT(color)         ((0x07000000 & color) >> 24)
#define DP_COLOR_IS_UV_COPLANE(color)           ((0x00C00000 & color) >> 22)
#define DP_COLOR_GET_H_SUBSAMPLE(color)         ((0x00300000 & color) >> 20)
#define DP_COLOR_GET_V_SUBSAMPLE(color)         ((0x000C0000 & color) >> 18)
#define DP_COLOR_BITS_PER_PIXEL(color)          ((0x0003FF00 & color) >>  8)
#define DP_COLOR_GET_COLOR_GROUP(color)         ((0x000000C0 & color) >>  6)
#define DP_COLOR_GET_SWAP_ENABLE(color)         ((0x00000020 & color) >>  5)
#define DP_COLOR_GET_UNIQUE_ID(color)           ((0x0000001F & color) >>  0)
#define DP_COLOR_GET_HW_FORMAT(color)           ((0x0000001F & color) >>  0)

typedef enum DP_COLOR_ENUM
{
    DP_COLOR_UNKNOWN        = 0,
    DP_COLOR_FULLG8         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0,  8, 3,  0, 20),
    DP_COLOR_FULLG10        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 10, 3,  0, 21),
    DP_COLOR_FULLG12        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 12, 3,  0, 22),
    DP_COLOR_FULLG14        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 14, 3,  0, 26),
    DP_COLOR_UFO10          = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 10, 3,  0, 27),

    DP_COLOR_BAYER8         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0,  8, 2,  0, 20),
    DP_COLOR_BAYER10        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 10, 2,  0, 21),
    DP_COLOR_BAYER12        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 12, 2,  0, 22),

    // Unified format
    DP_COLOR_GREY           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0,  8, 1,  0, 7),

    DP_COLOR_RGB565         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 16, 0,  0, 0),
    DP_COLOR_BGR565         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 16, 0,  1, 0),
    DP_COLOR_RGB888         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24, 0,  1, 1),
    DP_COLOR_BGR888         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24, 0,  0, 1),
    DP_COLOR_RGBA8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  1, 2),
    DP_COLOR_BGRA8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  0, 2),
    DP_COLOR_ARGB8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  1, 3),
    DP_COLOR_ABGR8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  0, 3),

    DP_COLOR_UYVY           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  0, 4),
    DP_COLOR_VYUY           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  1, 4),
    DP_COLOR_YUYV           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  0, 5),
    DP_COLOR_YVYU           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  1, 5),

    DP_COLOR_I420           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 1,  8, 1,  0, 8),
    DP_COLOR_YV12           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 1,  8, 1,  1, 8),
    DP_COLOR_I422           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 0,  8, 1,  0, 9),
    DP_COLOR_YV16           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 0,  8, 1,  1, 9),
    DP_COLOR_I444           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 0, 0,  8, 1,  0, 10),
    DP_COLOR_YV24           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 0, 0,  8, 1,  1, 10),

    DP_COLOR_NV12           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 1,  8, 1,  0, 12),
    DP_COLOR_NV21           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 1,  8, 1,  1, 12),
    DP_COLOR_NV16           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 0,  8, 1,  0, 13),
    DP_COLOR_NV61           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 0,  8, 1,  1, 13),
    DP_COLOR_NV24           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 0, 0,  8, 1,  0, 14),
    DP_COLOR_NV42           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 0, 0,  8, 1,  1, 14),

    // Mediatek proprietary format
    DP_COLOR_420_BLKP_UFO   = DP_COLORFMT_PACK(0, 0,  5,   2,  1, 1, 1, 256, 1, 0, 12),//Frame mode + Block mode
    DP_COLOR_420_BLKP       = DP_COLORFMT_PACK(0, 0,  1,   2,  1, 1, 1, 256, 1, 0, 12),//Frame mode + Block mode
    DP_COLOR_420_BLKI       = DP_COLORFMT_PACK(0, 0,  3,   2,  1, 1, 1, 256, 1, 0, 12),//Field mode + Block mode
    DP_COLOR_422_BLKP       = DP_COLORFMT_PACK(0, 0,  1,   1,  0, 1, 0, 512, 1, 0, 4), //Frame mode

    DP_COLOR_PARGB8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 26),
    DP_COLOR_XARGB8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 27),
    DP_COLOR_PABGR8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 28),
    DP_COLOR_XABGR8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 29),

    DP_COLOR_IYU2           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24,  1, 0, 25),
    DP_COLOR_YUV444         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24,  1, 0, 30),

    // Mediatek proprietary 10bit format
    DP_COLOR_RGBA1010102    = DP_COLORFMT_PACK(1, 0,  0,   1,  0, 0, 0, 32,  0, 1, 2),
    DP_COLOR_BGRA1010102    = DP_COLORFMT_PACK(1, 0,  0,   1,  0, 0, 0, 32,  0, 0, 2),
    DP_COLOR_UYVY_10P       = DP_COLORFMT_PACK(1, 0,  0,   1,  0, 1, 0, 20,  1, 0, 4),//Packed 10bit UYVY
    DP_COLOR_NV21_10P       = DP_COLORFMT_PACK(1, 0,  0,   2,  1, 1, 1, 10,  1, 1, 12),//Packed 10bit NV21
    DP_COLOR_420_BLKP_10_H  = DP_COLORFMT_PACK(1, 0,  1,   2,  1, 1, 1, 320, 1, 0, 12),//Frame mode + Block mode
    DP_COLOR_420_BLKP_10_V  = DP_COLORFMT_PACK(1, 1,  1,   2,  1, 1, 1, 320, 1, 0, 12),//Frame mode + HEVC tile mode
    DP_COLOR_420_BLKP_UFO_10_H  = DP_COLORFMT_PACK(1, 0,  5,   2,  1, 1, 1, 320, 1, 0, 12),//Frame mode + Block mode
    DP_COLOR_420_BLKP_UFO_10_V  = DP_COLORFMT_PACK(1, 1,  5,   2,  1, 1, 1, 320, 1, 0, 12),//Frame mode + HEVC tile mode

    // Loose 10bit format
    DP_COLOR_UYVY_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 0, 4),
    DP_COLOR_VYUY_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 1, 4),
    DP_COLOR_YUYV_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 0, 5),
    DP_COLOR_YVYU_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 1, 5),
    DP_COLOR_NV12_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 1, 10,  1, 0, 12),
    DP_COLOR_NV21_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 1, 10,  1, 1, 12),
    DP_COLOR_NV16_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 0, 10,  1, 0, 13),
    DP_COLOR_NV61_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 0, 10,  1, 1, 13),
    DP_COLOR_YV12_10L       = DP_COLORFMT_PACK(0, 1,  0,   3,  0, 1, 1, 10,  1, 1, 8),
    DP_COLOR_I420_10L       = DP_COLORFMT_PACK(0, 1,  0,   3,  0, 1, 1, 10,  1, 0, 8),

//    DP_COLOR_YUV422I        = DP_COLORFMT_PACK(1,  0, 1, 0, 16, 1, 41),//Dup to DP_COLOR_YUYV
//    DP_COLOR_Y800           = DP_COLORFMT_PACK(1,  0, 1, 0, 8, 1, 42),//Dup to DP_COLOR_GREY
//    DP_COLOR_COMPACT_RAW1   = DP_COLORFMT_PACK(1,  0, 1, 0, 10, 2, 43),//Dup to Bayer10
//    DP_COLOR_420_3P_YVU     = DP_COLORFMT_PACK(3,  0, 1, 1,  8, 1, 44),//Dup to DP_COLOR_YV12
} DP_COLOR_ENUM;

// Legacy for 6589 compatible
typedef DP_COLOR_ENUM DpColorFormat;

#define eYUV_420_3P             DP_COLOR_I420
#define eYUV_420_2P_YUYV        DP_COLOR_YUYV
#define eYUV_420_2P_UYVY        DP_COLOR_UYVY
#define eYUV_420_2P_YVYU        DP_COLOR_YVYU
#define eYUV_420_2P_VYUY        DP_COLOR_VYUY
#define eYUV_420_2P_ISP_BLK     DP_COLOR_420_BLKP
#define eYUV_420_2P_VDO_BLK     DP_COLOR_420_BLKI
#define eYUV_422_3P             DP_COLOR_I422
#define eYUV_422_2P             DP_COLOR_NV16
#define eYUV_422_I              DP_COLOR_YUYV
#define eYUV_422_I_BLK          DP_COLOR_422_BLKP
#define eYUV_444_3P             DP_COLOR_I444
#define eYUV_444_2P             DP_COLOR_NV24
#define eYUV_444_1P             DP_COLOR_YUV444
#define eBAYER8                 DP_COLOR_BAYER8
#define eBAYER10                DP_COLOR_BAYER10
#define eBAYER12                DP_COLOR_BAYER12
#define eRGB565                 DP_COLOR_RGB565
#define eBGR565                 DP_COLOR_BGR565
#define eRGB888                 DP_COLOR_RGB888
#define eBGR888                 DP_COLOR_BGR888
#define eARGB8888               DP_COLOR_ARGB8888
#define eABGR8888               DP_COLOR_ABGR8888
#define DP_COLOR_XRGB8888       DP_COLOR_ARGB8888
#define DP_COLOR_XBGR8888       DP_COLOR_ABGR8888
#define eRGBA8888               DP_COLOR_RGBA8888
#define eBGRA8888               DP_COLOR_BGRA8888
#define eXRGB8888               DP_COLOR_XRGB8888
#define eXBGR8888               DP_COLOR_XBGR8888
#define DP_COLOR_RGBX8888       DP_COLOR_RGBA8888
#define DP_COLOR_BGRX8888       DP_COLOR_BGRA8888
#define eRGBX8888               DP_COLOR_RGBX8888
#define eBGRX8888               DP_COLOR_BGRX8888
#define ePARGB8888              DP_COLOR_PARGB8888
#define eXARGB8888              DP_COLOR_XARGB8888
#define ePABGR8888              DP_COLOR_PABGR8888
#define eXABGR8888              DP_COLOR_XABGR8888
#define eGREY                   DP_COLOR_GREY
#define eI420                   DP_COLOR_I420
#define eYV12                   DP_COLOR_YV12
#define eIYU2                   DP_COLOR_IYU2


#define eYV21                   DP_COLOR_I420
#define eNV12_BLK               DP_COLOR_420_BLKP
#define eNV12_BLK_FCM           DP_COLOR_420_BLKI
#define eYUV_420_3P_YVU         DP_COLOR_YV12

#define eNV12_BP                DP_COLOR_420_BLKP
#define eNV12_BI                DP_COLOR_420_BLKI
#define eNV12                   DP_COLOR_NV12
#define eNV21                   DP_COLOR_NV21
#define eI422                   DP_COLOR_I422
#define eYV16                   DP_COLOR_YV16
#define eNV16                   DP_COLOR_NV16
#define eNV61                   DP_COLOR_NV61
#define eUYVY                   DP_COLOR_UYVY
#define eVYUY                   DP_COLOR_VYUY
#define eYUYV                   DP_COLOR_YUYV
#define eYVYU                   DP_COLOR_YVYU
#define eUYVY_BP                DP_COLOR_422_BLKP
#define eI444                   DP_COLOR_I444
#define eNV24                   DP_COLOR_NV24
#define eNV42                   DP_COLOR_NV42
#define DP_COLOR_YUY2           DP_COLOR_YUYV
#define eYUY2                   DP_COLOR_YUY2
#define eY800                   DP_COLOR_GREY
//#define eIYU2
#define eMTKYUV                 DP_COLOR_422_BLKP

#define eCompactRaw1            DP_COLOR_BAYER10


enum DpInterlaceFormat
{
    eInterlace_None,
    eTop_Field,
    eBottom_Field
};

enum DpSecure
{
    DP_SECURE_NONE  = 0,
    DP_SECURE       = 1,
    DP_SECURE_SHIFT = 8
};

#define MAX_NUM_READBACK_REGS (20)

#define VENC_ENABLE_FLAG    (0x08967)

#ifndef __KERNEL__
struct DpPortOption
{
    int width;
    int height;
    DpRect ROI;
    DpColorFormat format;
    DpInterlaceFormat interlace;

    enum DpPort
    {
        eLCD0_PORT,
        eLCD1_PORT,
        eHDMI_PORT,
        eTVOUT_PORT,
        eOVERLAY_PORT,
        eVIRTUAL_PORT,
        eMEMORY_PORT
    };

    DpPort port;

    int overlayID;              // setting if choose port = eOVERLAY
    int virtualID;              // setting if choose port = eVIRTUAL_PORT
    DpBufferPool *buffer;       // setting if choose port = eMEMORY
};
#endif  // __KERNEL__

#endif  // __DP_DATA_TYPE_H__
