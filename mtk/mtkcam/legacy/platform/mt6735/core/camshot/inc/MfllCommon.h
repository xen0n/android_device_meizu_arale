#ifndef __MFLL_COMMON__
#define __MFLL_COMMON__

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

//should be 0
#define TEST_OFFLINE    0
#define TEST_SWMIXER    0
#define TEST_MEMC_YUY2  1
//should be 0
#define TEST_AQS_TUNING 0
#define TEST_AQS_15RAW  0
//should be 0
#define TEST_DUMP       0
#define TEST_DUMP_RAW   (0|TEST_AQS_TUNING)
#define TEST_DUMP_YUV   0
#define TEST_DUMP_MFB   0
#define TEST_DUMP_MIXER 0
#define TEST_DUMP_JPEG  0
#define TEST_DUMP_POSTVIEW  0
#define TEST_DUMP_EXIF  (0|TEST_AQS_TUNING|TEST_AQS_15RAW)
#define TEST_DUMP_15RAW (0|TEST_AQS_15RAW)
#if TEST_AQS_TUNING || TEST_AQS_15RAW
#define TEST_DUMP_TO_SDCARD "1"
#else
#define TEST_DUMP_TO_SDCARD "0"
#endif
//should be 1
#define TEST_ALGORITHM  1
#define TEST_ISHOT      1
#define TEST_CAPTURE    1
#define TEST_EIS        1
#define TEST_MFB        1
#define TEST_MIXING     1
#define TEST_POSTVIEW   1
#define TEST_JPEG       1
#define TEST_MEMORY     1
#define TEST_SENSOR     1
#define TEST_3A         1
#define TEST_MEMC       1
#define TEST_DBGINFO    1
#define TEST_BSS        1
//should be 0
#define TEST_BITTURE    0
#define TEST_BITTURE_LDVT    0  //remember modifiy those files
                                //[mixer3 regs] camera_tuning_para_imx135mipiraw.cpp
                                //[ee/ang regs] camera_isp_regs_imx135mipiraw.h
                                //[ee/anr enable] isp_tuning_idx.cpp

/* If using GMO optimization, set these configurations as default */
#if MFLL_GMO_RAM_OPTIMIZE
    #define MFLL_REDUCE_MEMORY_USAGE        2 // Balance mode
#endif

/* TODO: no idea why memory usage is not enough therefore apply MRP as default */
#ifndef MFLL_REDUCE_MEMORY_USAGE
    #define MFLL_REDUCE_MEMORY_USAGE 2
#endif

/**
 *  Memory Reduce Plan (MRP)
 *
 *  MFNR v1.2 provides MRP to reduce memory usage peak. To use MRP will reduce
 *  memory usage peak but may slow done performance.
 *
 *  There're some modes for use:
 *
 *  Mode 0: Not apply MRP, this is the best performance, just like before.
 *  Mode 1: Lowest memory plan, this mode is for the lowest memory usage but
 *          decrease performance.
 *  Mode 2: Balance mode, reduce memory usage but decrease a little performance.
 *
 *  MRP also can be configured dynamically, you could specify
 *
 *  $ adb shell setprop mediatek.mfll.mrp VALUE
 *
 *  where VALUE can be 0, 1, or 2 to the modes we just described.
 */
#ifndef MFLL_REDUCE_MEMORY_USAGE
#define MFLL_REDUCE_MEMORY_USAGE        0
#endif

/* Defines MFLL supports reading properties or not*/
#ifndef MFLL_READ_PROPERTY
#define MFLL_READ_PROPERTY              1
#endif


#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>
#include <mtkcam/v1/camutils/CamInfo.h>
#include <mtkcam/v1/camutils/CamFormat.h>
//#include <camshot/ICamShot.h>
//#include <IShot.h>

#include <mtkcam/hal/IHalSensor.h>

#include <mtkcam/camnode/AllocBufHandler.h>
#include <mtkcam/camnode/ICamGraphNode.h>
#include <mtkcam/camnode/pass1node.h>
#include <mtkcam/camnode/pass2node.h>
#include "MfllNode.h"
#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <mtkcam/iopipe/PostProc/IPortEnum.h>
#include <mtkcam/featureio/IHal3A.h>

#include <cutils/log.h>    // for log
#include <cutils/properties.h>

#include <camera_custom_mfll.h>


#define MAX_CAPTURE_FRAMES              4
#define MAX_REFERENCE_FRAMES            3



#endif  //__MFLL_COMMON__
