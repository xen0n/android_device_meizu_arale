/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
 /*
** $Log: fdvt_hal.cpp $
 *
*/
#define LOG_TAG "mHalFDVT"

#define FDVT_DDP_SUPPORT

#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/atomic.h>
//#include "MediaHal.h"
//#include <mhal/inc/camera/faces.h>
#include <mtkcam/common/faces.h>
//#include "MediaLog.h"
//#include "MediaAssert.h"
#include "fdvt_hal.h"
#include <mtkcam/Log.h>

#include "MTKDetection.h"
#include "camera_custom_fd.h"

#ifdef FDVT_DDP_SUPPORT
#include <DpBlitStream.h>
#endif

#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
using namespace android;

#define DUMP_IMAGE (0)

//****************************//
//Detection Distance Definition
#define ORIGINAL_VER (1)
#define FAR_V1       (0)
#define FAR_V2       (0)

//****************************//
#define FD_SCALES 14

#if(ORIGINAL_VER)
//static MUINT32 image_width_array[FD_SCALES] = {320, 256, 204, 160, 128, 102, 80, 64, 50, 40, 32};
//static MUINT32 image_height_array[FD_SCALES] = {240, 192, 152, 120, 96, 76, 60, 48, 38, 30, 24};
static MUINT32 image_width_array[FD_SCALES];
static MUINT32 image_height_array[FD_SCALES];

#endif

#if(FAR_V1)
static MUINT32 image_width_array[FD_SCALES] = {360, 280, 220, 172, 134, 106, 84, 68, 54, 44, 34, 0, 0, 0};
static MUINT32 image_height_array[FD_SCALES] = {270, 210, 164, 128, 100, 80, 64, 50, 40, 32, 26, 0, 0, 0};
#endif

#if(FAR_V2)
static MUINT32 image_width_array[FD_SCALES] = {400, 304, 240, 192, 152, 120, 96, 80, 64, 48, 34, 0, 0, 0};
static MUINT32 image_height_array[FD_SCALES] = {300, 228, 180, 144, 114, 90, 72, 60, 48, 36, 26, 0, 0, 0};
#endif

static MUINT32 image_width_array_v1[FD_SCALES] = {320, 256, 204, 160, 128, 102, 80, 64, 50, 40, 32, 0, 0, 0};
static MUINT32 image_height_array_v1[FD_SCALES] = {240, 192, 152, 120, 96, 76, 60, 48, 38, 30, 24, 0, 0, 0};
static MUINT32 image_width_array_v2[FD_SCALES] = {360, 280, 220, 172, 134, 106, 84, 68, 54, 44, 34, 0, 0, 0};
static MUINT32 image_height_array_v2[FD_SCALES] = {270, 210, 164, 128, 100, 80, 64, 50, 40, 32, 26, 0, 0, 0};
static MUINT32 image_width_array_v3[FD_SCALES] = {400, 304, 240, 192, 152, 120, 96, 80, 64, 48, 34, 0, 0, 0};
static MUINT32 image_height_array_v3[FD_SCALES] = {300, 228, 180, 144, 114, 90, 72, 60, 48, 36, 26, 0, 0, 0};

static MUINT32 sd_image_width_array[FD_SCALES] = {320, 256, 204, 160, 128, 102, 80, 64, 50, 40, 32, 0, 0, 0};
static MUINT32 sd_image_height_array[FD_SCALES] = {240, 192, 152, 120, 96, 76, 60, 48, 38, 30, 24, 0, 0, 0};

static MUINT32 fd_image_width_array[FD_SCALES] = {400, 304, 240, 192, 152, 120, 96, 80, 64, 48, 34, 0, 0, 0};
static MUINT32 fd_image_height_array[FD_SCALES] = {300, 228, 180, 144, 114, 90, 72, 60, 48, 36, 26, 0, 0, 0};
//static MUINT32 fd_image_width_array[FD_SCALES] = {320, 256, 204, 160, 128, 102, 80, 64, 50, 40, 32, 0, 0, 0};
//static MUINT32 fd_image_height_array[FD_SCALES] = {240, 192, 152, 120, 96, 76, 60, 48, 38, 30, 24, 0, 0, 0};

//static MUINT32 IMAGE_MAX_WIDTH  = image_width_array[0];
//static MUINT32 IMAGE_MAX_HEIGHT = image_height_array[0];
static MUINT32 IMAGE_MAX_WIDTH;
static MUINT32 IMAGE_MAX_HEIGHT;

static MUINT32 ASD_IMAGE_WIDTH = 320;
static MUINT32 ASD_IMAGE_HEIGHT = 240;

//****************************//

static const MUINT32 image_input_width_vga = 640;
static const MUINT32 image_input_height_vga = 480;
static const MUINT32 image_input_height_buffer = 640;
static MUINT32 ImageScaleTotalSize = 0;
static MUINT8 *ImageScaleBuffer;
static MBOOL EnableSWResizerFlag;
static MUINT8 *ImageVGABuffer;
static bool g_input_vga = 0;
static bool g_input_4_3 = 0;
static bool g_input_16_9 = 0;
static void Create640WidthImage_Y(MUINT8* src_img, MUINT32 src_w, MUINT32 src_h, MUINT8* dst_img);
static void CreateScaleImagesFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer,MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles);
static void CreateScaleImagesFromPreview_Y(MUINT8* image_buffer_preview, MUINT8* image_buffer, MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles);
static void CreateSingleImageFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles);

#define MHAL_NO_ERROR 0
#define MHAL_INPUT_SIZE_ERROR 1
#define MHAL_UNINIT_ERROR 2
#define MHAL_REINIT_ERROR 3

#define MAX_FACE_NUM 15

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//
//
static halFDBase *pHalFD = NULL;

static MINT32 FdResult_Num;
static FD_RESULT FdResult[15];

MUINT32 g_Lcm_rotate = 0;
MUINT32 g_Sensor_rotate = 0;
MUINT32 g_CameraTYPE = 0;

MUINT32 g_FDW = 0;
MUINT32 g_FDH = 0;

static float g_FDSizeRatio = 0;

volatile static MINT32     mFDCount = 0;
static Mutex       gLock;
static Mutex       gInitLock;

unsigned char         g_BufferGroup = 0;
unsigned char         g_ucPlane=1;
//unsigned short int  g_uwDst_width =  IMAGE_MAX_WIDTH;
//unsigned short int  g_uwDst_height = IMAGE_MAX_HEIGHT;
unsigned short int  g_uwDst_width =  0;
unsigned short int  g_uwDst_height = 0;

//int g_count = 0;

static MBOOL g_CurrentStatus = 0;

typedef struct
{
    MUINT8 *srcAddr;
    MUINT32 srcWidth;
    MUINT32 srcHeight;
    MUINT8 *dstAddr;
    MUINT32 dstWidth;
    MUINT32 dstHeight;
} PIPE_BILINEAR_Y_RESIZER_STRUCT, *P_PIPE_BILINEAR_Y_RESIZER_STRUCT;

#define PIPE_IUL_I_TO_X(i)                ((i) << 16)                        ///< Convert from integer to S15.16 fixed-point
#define PIPE_IUL_X_TO_I(x)                 (((x) + (1 << 15)) >> 16)      ///< Convert from S15.16 fixed-point to integer (round)
#define PIPE_IUL_X_TO_I_CHOP(x)        ((x) >> 16)                        ///< Convert from S15.16 fixed-point to integer (chop)
#define PIPE_IUL_X_TO_I_CARRY(x)        (((x) + 0x0000FFFF) >> 16) ///< Convert from S15.16 fixed-point to integer (carry)
#define PIPE_IUL_X_FRACTION(x)        ((x) & 0x0000FFFF)

#define PIPE_LINEAR_INTERPOLATION(val1, val2, weighting2)   \
   PIPE_IUL_X_TO_I((val1) * (PIPE_IUL_I_TO_X(1) - (weighting2)) + (val2) * (weighting2))

static void PipeBilinearResizer(P_PIPE_BILINEAR_Y_RESIZER_STRUCT pUtlRisizerInfo)
{
    if(pUtlRisizerInfo->srcWidth == 0 || pUtlRisizerInfo->srcHeight==0)
        return;
    if(pUtlRisizerInfo->dstWidth == 0 || pUtlRisizerInfo->dstHeight==0)
        return;

    const MUINT32 srcPitch = pUtlRisizerInfo->srcWidth;
    const MUINT32 srcStepX = PIPE_IUL_I_TO_X(pUtlRisizerInfo->srcWidth) /pUtlRisizerInfo->dstWidth;
    const MUINT32 srcStepY = PIPE_IUL_I_TO_X(pUtlRisizerInfo->srcHeight) /pUtlRisizerInfo->dstHeight;
    const MUINT32 img_w = pUtlRisizerInfo->dstWidth;

    MUINT8 *const src_buffer = pUtlRisizerInfo->srcAddr;
    MUINT8 *dstAddr= pUtlRisizerInfo->dstAddr;
    MUINT32 srcCoordY = 0;
    MINT32 h = pUtlRisizerInfo->dstHeight;

    while (--h >= 0)
    {
        MINT32 w = img_w;
        MUINT32 srcCoordX = 0;

        MINT32 srcOffset_1;
        MINT32 srcOffset_2;
        MUINT8 *src_ptr_1;
        MUINT8 *src_ptr_2;

        MINT32 y_carry = PIPE_IUL_X_TO_I_CARRY(srcCoordY);
        MINT32 y_chop  = PIPE_IUL_X_TO_I_CHOP(srcCoordY);

        if(y_carry < 0 || y_carry >= (MINT32)pUtlRisizerInfo->srcHeight)
            return;
        if(y_chop < 0 || y_chop >= (MINT32)pUtlRisizerInfo->srcHeight)
            return;


        srcOffset_1 = y_chop * srcPitch;
        srcOffset_2 = y_carry * srcPitch;
        src_ptr_1 = src_buffer + srcOffset_1;
        src_ptr_2 = src_buffer + srcOffset_2;

        while (--w >= 0)
        {
            MUINT8 pixel_1, pixel_2;
            MINT32 y, y1;

            MINT32 x_carry = PIPE_IUL_X_TO_I_CARRY(srcCoordX);
            MINT32 x_chop = PIPE_IUL_X_TO_I_CHOP(srcCoordX);

            MINT32 weighting2;

            weighting2 = PIPE_IUL_X_FRACTION(srcCoordX);

            /// 1st horizontal interpolation.
            pixel_1 = *(src_ptr_1 + x_chop);
            pixel_2 = *(src_ptr_1 + x_carry);
            y = PIPE_LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

            /// 2nd horizontal interpolation.
            pixel_1 = *(src_ptr_2 + x_chop);
            pixel_2 = *(src_ptr_2 + x_carry);
            y1 = PIPE_LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

            /// Vertical interpolation.
            weighting2 = PIPE_IUL_X_FRACTION(srcCoordY);

            y = PIPE_LINEAR_INTERPOLATION(y, y1, weighting2);

            *dstAddr++ = (MUINT8)y;

            srcCoordX += srcStepX;
        }
        srcCoordY += srcStepY;
    }
}

/*******************************************************************************
*
********************************************************************************/
halFDBase*
halFDVT::
getInstance()
{
    Mutex::Autolock _l(gLock);
    MY_LOGD("[Create] &mFDCount:%p &gLock:%p", &mFDCount, &gLock);
    int const oldCount = ::android_atomic_inc(&mFDCount);

    int g_oldCount = oldCount;
    if(g_oldCount < 0)
    {
        MY_LOGW("Warning! [Create] count:%d->%d pHalFD:%p", oldCount, mFDCount, pHalFD);
        g_oldCount = 0;
        mFDCount = 1;
        pHalFD = NULL;
    }

    MY_LOGD("[Create] g_oldCount:%d->%d pHalFD:%p", g_oldCount, mFDCount, pHalFD);
    if  ( 0 == g_oldCount )
    {
        if  ( ! pHalFD )
        {
            MY_LOGW("Get Instance Warning!");
        }
        pHalFD = new halFDVT();
    }
    //else
    //{
        //mFDCount--;
        //return NULL;
    //}
    return  pHalFD;

/*
    MY_LOGD("[Create] Before:%d,", mFDCount);
    //MHAL_LOG("[halFDVT] getInstance \n");
    if (pHalFD == NULL) {
        pHalFD = new halFDVT();
    }

    android_atomic_inc(&mFDCount);

    MY_LOGD("[Create] After:%d,", mFDCount);

    return pHalFD;
*/
}

/*******************************************************************************
*
********************************************************************************/
void
halFDVT::
destroyInstance()
{
    Mutex::Autolock _l(gLock);
    MY_LOGD("[Delete] &mFDCount:%p &gLock:%p", &mFDCount, &gLock);
    int const oldCount = ::android_atomic_dec(&mFDCount);
    MY_LOGD("[Delete] count:%d->%d pHalFD:%p", oldCount, mFDCount, pHalFD);

    if  ( 0 == oldCount || ! pHalFD )
    {
        MY_LOGW("Delete Instance Warning!");
    }

    if  ( 1 == oldCount )
    {
        delete pHalFD;
        pHalFD = NULL;
    }

/*
     MY_LOGD("[Delete] Before:%d,", mFDCount);

    // More than one user
    if (mFDCount > 0)
          android_atomic_dec(&mFDCount);

   if (mFDCount == 0)
   {
        if (pHalFD)
        {
            delete pHalFD;
        }
        pHalFD = NULL;
   }

   MY_LOGD("[Delete] After:%d,", mFDCount);
*/
}

/*******************************************************************************
*
********************************************************************************/
halFDVT::halFDVT()
{
    m_pMTKFDVTObj = NULL;

    m_FDW = 0;
    m_FDH = 0;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;
    m_Inited = 0;
    //mFDCount = 0;

    m_pMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_FDFT_SW);
    //m_pMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_HW);
}


halFDVT::~halFDVT()
{
    m_FDW = 0;
    m_FDH = 0;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;

    if (m_pMTKFDVTObj) {
        m_pMTKFDVTObj->destroyInstance();
    }
    m_pMTKFDVTObj = NULL;
}


MINT32
halFDVT::halFDInit(
    MUINT32 fdW,
    MUINT32 fdH,
    MUINT8 *WorkingBuffer,
    MUINT32 WorkingBufferSize,
    MBOOL   SWResizerEnable,
    MUINT8  Current_mode  //0:FD, 1:SD, 2:vFB  3:CFB
)
{
    Mutex::Autolock _l(gInitLock);
    MUINT32 i;
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* Cdata=NULL;
    MTKFDFTInitInfo FDFTInitInfo;
    FD_Customize_PARA FDCustomData;

    if(m_Inited) {
        MY_LOGW("Warning!!! FDVT HAL OBJ is already inited!!!!");
        MY_LOGW("Old Width/Height : %d/%d, Parameter Width/Height : %d/%d", m_FDW, m_FDH, fdW, fdH);
        return MHAL_REINIT_ERROR;
    }
//**********************************************************************//
    g_CurrentStatus = Current_mode;
    MY_LOGD("[mHalFDInit] Current_mode:%d, SrcW:%d, SrcH:%d, ",Current_mode, fdW, fdH);
   /* if(SD_mode!=1)
    {
        MY_LOGD("[mHalFDInit] V3_mode,");
        for(i=0;i<FD_SCALES;i++)
        {
            image_width_array[i] = image_width_array_v3[i];
            image_height_array[i] = image_height_array_v3[i];
        }
    }

    else
    {
        MY_LOGD("[mHalFDInit] V1(sd)_mode,");
        for(i=0;i<FD_SCALES;i++)
        {
            image_width_array[i] = image_width_array_v1[i];
            image_height_array[i] = image_height_array_v1[i];
        }
    }*/
    MY_LOGD("[mHalFDInit] V1(sd)_mode,");
    for(i=0;i<FD_SCALES;i++)
    {
        image_width_array[i] = image_width_array_v1[i];
        image_height_array[i] = image_height_array_v1[i];
    }


    IMAGE_MAX_WIDTH  = image_width_array[0];
    IMAGE_MAX_HEIGHT = image_height_array[0];
    g_uwDst_width =  IMAGE_MAX_WIDTH;
    g_uwDst_height = IMAGE_MAX_HEIGHT;
//**********************************************************************//

    g_FDW = fdW;
    g_FDH = fdH;
    m_FDW = fdW;
    m_FDH = fdH;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;

    //m_FDW = 640;
    //m_FDH = 480;

    if(m_FDW == image_input_width_vga && m_FDH == image_input_height_vga)
    {
        g_input_vga = 1;
        g_input_4_3 = 0;
        g_input_16_9 = 0;
    }
    else if( m_FDW*3 == m_FDH*4)
    {
        g_input_vga = 0;
        g_input_4_3 = 1;
        g_input_16_9 = 0;

    }
    //else if( m_FDW*9 == m_FDH*16)
    else if( (m_FDW*9 == m_FDH*16) || (m_FDW*3 == m_FDH*5))
    {
        g_input_vga = 0;
        g_input_4_3 = 0;
        g_input_16_9 = 1;
    }
    else
    {
        g_input_vga = 0;
        g_input_4_3 = 0;
        g_input_16_9 = 1;
    }
    //MHAL_ASSERT(m_pMTKFDVTObj != NULL, "m_pMTKFDVTObj is NULL");

    get_fd_CustomizeData(&FDCustomData);


    //g_FDSizeRatio = FDCustomData.FDSizeRatio;
    g_FDSizeRatio = 0;
    MY_LOGD("FDSizeRatio = %f", g_FDSizeRatio);

    FDFTInitInfo.WorkingBufAddr = WorkingBuffer;
    FDFTInitInfo.WorkingBufSize = WorkingBufferSize;
    FDFTInitInfo.FDThreadNum = FDCustomData.FDThreadNum;
    FDFTInitInfo.FDThreshold = FDCustomData.FDThreshold;
    //FDFTInitInfo.FDThreshold = 256;
    FDFTInitInfo.MajorFaceDecision = FDCustomData.MajorFaceDecision;
    FDFTInitInfo.OTRatio = FDCustomData.OTRatio;
    //FDFTInitInfo.OTRatio = 960;
    FDFTInitInfo.SmoothLevel = FDCustomData.SmoothLevel;
    if(g_CurrentStatus==2)
        FDFTInitInfo.FDSkipStep = 4;   //FB mode
    else
        FDFTInitInfo.FDSkipStep = FDCustomData.FDSkipStep;

    FDFTInitInfo.FDRectify = FDCustomData.FDRectify;

    FDFTInitInfo.OTFlow = FDCustomData.OTFlow;
    if(g_CurrentStatus==2) {

        FDFTInitInfo.OTFlow = 0;
        FDCustomData.OTFlow = 0;
        if(FDCustomData.OTFlow==0)
            FDFTInitInfo.FDRefresh = 90;   //FB mode
        else
            FDFTInitInfo.FDRefresh = 10;   //FB mode
    }
/*
    else if(g_CurrentStatus==1)
    {
       FDFTInitInfo.OTFlow = 0;
       FDFTInitInfo.FDRectify = 5;
       FDFTInitInfo.FDRefresh = 20;
       FDFTInitInfo.FDSkipStep = 4;
    }
*/
    else{
        FDFTInitInfo.FDRefresh = FDCustomData.FDRefresh;
    }

    //FDFTInitInfo.FDRefresh = 1;
    //FDFTInitInfo.FDBufWidth = fdW;
    //FDFTInitInfo.FDBufHeight = fdH;

    FDFTInitInfo.FDBufWidth = image_width_array[0]; //
    FDFTInitInfo.FDBufHeight = image_height_array[0];
    FDFTInitInfo.FDSrcWidth = fdW;
    FDFTInitInfo.FDSrcHeight = fdH;

    FDFTInitInfo.FDTBufWidth = image_width_array[0];// µ¹OT¥Îªº, 320x240
    FDFTInitInfo.FDTBufHeight = image_height_array[0];
    FDFTInitInfo.FDImageArrayNum = 14;
    FDFTInitInfo.FDImageWidthArray = image_width_array;
    FDFTInitInfo.FDImageHeightArray = image_height_array;
    FDFTInitInfo.FDCurrent_mode = g_CurrentStatus;
    FDFTInitInfo.FDModel = FDCustomData.FDModel;

    FDFTInitInfo.FDMinFaceLevel = 0;
    FDFTInitInfo.FDMaxFaceLevel = 13;
    //FDFTInitInfo.FDImgFmtCH1 = FACEDETECT_IMG_Y_SCALES;
    FDFTInitInfo.FDImgFmtCH1 = FACEDETECT_IMG_Y_SINGLE;
    FDFTInitInfo.FDImgFmtCH2 = FACEDETECT_IMG_RGB565;
    FDFTInitInfo.SDImgFmtCH1 = FACEDETECT_IMG_Y_SCALES;
    FDFTInitInfo.SDImgFmtCH2 = FACEDETECT_IMG_Y_SINGLE;
    FDFTInitInfo.SDThreshold = FDCustomData.SDThreshold;
    FDFTInitInfo.SDMainFaceMust = FDCustomData.SDMainFaceMust;
    FDFTInitInfo.GSensor = FDCustomData.GSensor;
    FDFTInitInfo.GenScaleImageBySw = 1;

    FDFTInitInfo.FDTBufHeight =  FDFTInitInfo.FDTBufWidth * fdH/fdW;
    for(int j=0;j<FD_SCALES;j++)
    {
        image_height_array[j] = image_width_array[j]*fdH/fdW;
    }


    MY_LOGD("WorkingBufAddr = %d", FDFTInitInfo.WorkingBufAddr);
    MY_LOGD("WorkingBufSize = %d", FDFTInitInfo.WorkingBufSize);
    MY_LOGD("FDThreadNum = %d", FDFTInitInfo.FDThreadNum);
    MY_LOGD("FDThreshold = %d", FDFTInitInfo.FDThreshold);
    MY_LOGD("MajorFaceDecision = %d", FDFTInitInfo.MajorFaceDecision);
    MY_LOGD("OTRatio = %d", FDFTInitInfo.OTRatio);
    MY_LOGD("SmoothLevel = %d", FDFTInitInfo.SmoothLevel);
    MY_LOGD("FDSkipStep = %d", FDFTInitInfo.FDSkipStep);
    MY_LOGD("FDRectify = %d", FDFTInitInfo.FDRectify);
    MY_LOGD("FDRefresh = %d", FDFTInitInfo.FDRefresh);
    MY_LOGD("FDBufWidth = %d", FDFTInitInfo.FDBufWidth);
    MY_LOGD("FDBufHeight = %d", FDFTInitInfo.FDBufHeight);
    MY_LOGD("FDTBufWidth = %d", FDFTInitInfo.FDTBufWidth);
    MY_LOGD("FDTBufHeight = %d", FDFTInitInfo.FDTBufHeight);

    MY_LOGD("FDImageArrayNum = %d", FDFTInitInfo.FDImageArrayNum);
    MY_LOGD("FDImageWidthArray = ");
    for (int i = 0; i < FD_SCALES; i++)
    {
        MY_LOGD("%d, ",FDFTInitInfo.FDImageWidthArray[i]);
    }
    MY_LOGD("\n");
    MY_LOGD("FDImageHeightArray = ");
    for (int i = 0; i < FD_SCALES; i++)
    {
        MY_LOGD("%d, ",FDFTInitInfo.FDImageHeightArray[i]);
    }
    MY_LOGD("\n");
    MY_LOGD("FDMinFaceLevel = %d", FDFTInitInfo.FDMinFaceLevel);
    MY_LOGD("FDMaxFaceLevel = %d", FDFTInitInfo.FDMaxFaceLevel);
    MY_LOGD("FDImgFmtCH1 = %d", FDFTInitInfo.FDImgFmtCH1);
    MY_LOGD("FDImgFmtCH2 = %d", FDFTInitInfo.FDImgFmtCH2);
    MY_LOGD("SDImgFmtCH1 = %d", FDFTInitInfo.SDImgFmtCH1);
    MY_LOGD("SDImgFmtCH2 = %d", FDFTInitInfo.SDImgFmtCH2);
    MY_LOGD("SDThreshold = %d", FDFTInitInfo.SDThreshold);
    MY_LOGD("SDMainFaceMust = %d", FDFTInitInfo.SDMainFaceMust);
    MY_LOGD("GSensor = %d", FDFTInitInfo.GSensor);
    MY_LOGD("GenScaleImageBySw = %d", FDFTInitInfo.GenScaleImageBySw);


    m_pMTKFDVTObj->FDVTInit(&FDFTInitInfo);

    EnableSWResizerFlag = SWResizerEnable;
    if(EnableSWResizerFlag)
    {
        ImageScaleTotalSize = 0;
        for(i=0; i<FD_SCALES;i++)
        {
            ImageScaleTotalSize += image_width_array[i]*image_height_array[i];
        }
        ImageScaleBuffer = new unsigned char[ImageScaleTotalSize];
    }

    if(g_input_vga == 0)
    {
        ImageVGABuffer = new unsigned char[image_input_width_vga*image_input_height_buffer];
        memset(ImageVGABuffer, 0, image_input_width_vga*image_input_height_buffer);
    }
        MY_LOGD("[%s] End", __FUNCTION__);
    m_Inited = 1;
    return err;
}

bool doRGB565Buffer_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr)
{
     bool ret = true;

    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;

    unsigned char* pucYBuf;
    unsigned char* pucUVBuf;
    unsigned int i, j, k;
    int Y[4], U, V, R[4], G[4], B[4], r, g, b;
    int dImgW = 640;
    int dImgH = 480;

    pucYBuf = src_yp;
    pucUVBuf = src_yp+dImgW * dImgH;

    for(i=0;i<dImgH;i=i+2)
    {
    for(j=0;j<dImgW;j=j+2)
    {
            Y[0] = *(pucYBuf + ((i+0) * dImgW) + j);
            Y[1] = *(pucYBuf + ((i+0) * dImgW) + j+1) ;
            Y[2] = *(pucYBuf + ((i+1) * dImgW) + j) ;
            Y[3] = *(pucYBuf + ((i+1) * dImgW) + j+1) ;

            Y [0]= (Y[0]+Y[1]+Y[2]+Y[3]) >> 2;
            V  =  *(pucUVBuf + ((i >> 1) * dImgW) + j );
            U  =  *(pucUVBuf + ((i >> 1) * dImgW) + j +1);

            for(k=0;k<1;k++)
            {
                r = (32 * Y[k] + 45 * (V-128) + 16) / 32;
                g = (32 * Y[k] - 11 * (U-128) - 23 * (V-128) + 16) / 32;
                b = (32 * Y[k] + 57 * (U-128) + 16) / 32;

                R[k] = (r<0) ? 0: (r>255) ? 255 : r;
                G[k]= (g<0) ? 0: (g>255) ? 255 : g;
                B[k] = (b<0) ? 0: (b>255) ? 255 : b;
            }

                  *(dst_rgb + ((i>>1)  * dImgW ) + j+0) = ((G[0] & 0x1C) <<3 ) + ((B[0] & 0xF8) >>3 );
            *(dst_rgb + ((i>>1)  * dImgW ) + j+1) = ((G[0] & 0xE0) >>5 ) + ((R[0] & 0xF8));
        }
    }

        return ret;
}

bool doRGB565BufferP1_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 SrcWidth, MUINT32 SrcHeight)
{
     bool ret = true;

    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;

    unsigned char* pucYBuf;
    unsigned char* pucUVBuf;
    unsigned int i, j, k;
    int Y[4], U, V, R[2], G[2], B[2], r, g, b;
    unsigned int dImgW = SrcWidth;
    unsigned int dImgH = SrcHeight;
    //int dImgW = g_FDW;
    //int dImgH = g_FDH;

    pucYBuf = src_yp;

    for(i=0;i<dImgH;i=i+2)
    {
        for(j=0;j<dImgW*2;j=j+4)
        {
            Y[0] = *(pucYBuf + ((i+0) * dImgW*2) + j);
            Y[1] = *(pucYBuf + ((i+0) * dImgW*2) + j+2) ;
            Y[2] = *(pucYBuf + ((i+1) * dImgW*2) + j);
            Y[3] = *(pucYBuf + ((i+1) * dImgW*2) + j+2) ;

            Y [0]= (Y[0]+Y[1]+Y[2]+Y[3]) >> 2;
            U  =  (*(pucYBuf + ((i+0) * dImgW*2) + j+1) + *(pucYBuf + ((i+1) * dImgW*2) + j+1) ) >> 1;
            V  =  (*(pucYBuf + ((i+0) * dImgW*2) + j+3) + *(pucYBuf + ((i+1) * dImgW*2) + j+3) ) >> 1;

            for(k=0;k<1;k++)
            {
                r = (32 * Y[k] + 45 * (V-128) + 16) / 32;
                g = (32 * Y[k] - 11 * (U-128) - 23 * (V-128) + 16) / 32;
                b = (32 * Y[k] + 57 * (U-128) + 16) / 32;

                R[k] = (r<0) ? 0: (r>255) ? 255 : r;
                G[k]= (g<0) ? 0: (g>255) ? 255 : g;
                B[k] = (b<0) ? 0: (b>255) ? 255 : b;
            }

            *(dst_rgb + ((i>>1)  * dImgW ) + (j>>1)+0) = ((G[0] & 0x1C) <<3 ) + ((B[0] & 0xF8) >>3 );
            *(dst_rgb + ((i>>1)  * dImgW ) + (j>>1)+1) = ((G[0] & 0xE0) >>5 ) + ((R[0] & 0xF8));
        }
    }

        return ret;
}

bool doRGB565BufferQVGAP1_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 SrcWidth1, MUINT32 SrcHeight1)
{
     bool ret = true;

    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;

    unsigned char* pucYBuf;
    unsigned char* pucUVBuf;
    unsigned int i, j, k;
    int Y[4], U, V, R[2], G[2], B[2], r, g, b;
    unsigned int dImgW = SrcWidth1;
    unsigned int dImgH = SrcHeight1;
    //int dImgW = g_FDW;
    //int dImgH = g_FDH;

    pucYBuf = src_yp;

    for(i=0;i<dImgH;i++)
    {
        for(j=0;j<dImgW*2;j=j+4)
        {
            Y[0] = *(pucYBuf + (i * dImgW*2) + j+0);
            Y[1] = *(pucYBuf + (i * dImgW*2) + j+2);

            U  =  *(pucYBuf + (i * dImgW*2) + j+1);
            V  =  *(pucYBuf + (i * dImgW*2) + j+3);

            for(k=0;k<2;k++)
            {
                r = (32 * Y[k] + 45 * (V-128) + 16) / 32;
                g = (32 * Y[k] - 11 * (U-128) - 23 * (V-128) + 16) / 32;
                b = (32 * Y[k] + 57 * (U-128) + 16) / 32;

                R[k] = (r<0) ? 0: (r>255) ? 255 : r;
                G[k] = (g<0) ? 0: (g>255) ? 255 : g;
                B[k] = (b<0) ? 0: (b>255) ? 255 : b;
            }

            *(dst_rgb + ((i<<1)  * dImgW ) + j+0) = ((G[0] & 0x1C) <<3 ) + ((B[0] & 0xF8) >>3 );
            *(dst_rgb + ((i<<1)  * dImgW ) + j+1) = ((G[0] & 0xE0) >>5 ) + ((R[0] & 0xF8));

            *(dst_rgb + ((i<<1)  * dImgW ) + j+2) = ((G[1] & 0x1C) <<3 ) + ((B[1] & 0xF8) >>3 );
            *(dst_rgb + ((i<<1)  * dImgW ) + j+3) = ((G[1] & 0xE0) >>5 ) + ((R[1] & 0xF8));

        }
    }

        return ret;
}


bool doRGB565Buffer_DDP(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr0, MUINT8 *a_srcAddr1 = NULL, MUINT8 *a_srcAddr2 = NULL)
{
    bool ret = true;

#ifdef FDVT_DDP_SUPPORT
    DpBlitStream FDstream;
    unsigned char *src_yp = (unsigned char *)a_srcAddr0;
    unsigned char *src_up = (unsigned char *)a_srcAddr1;
    unsigned char *src_vp = (unsigned char *)a_srcAddr2;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;
    void* src_addr_list[3];
    unsigned int src_size_list[3];
    void* dst_addr_list[3];
    unsigned int dst_size_list[3];

    int src_ysize = g_FDW * g_FDH;
    int src_usize, src_vsize;
    src_usize = src_vsize = src_ysize / 4;

    if(src_up == NULL)
        src_up = (src_yp + src_ysize);

    if(src_vp == NULL)
        src_vp = (src_yp + src_ysize * 5/4);

    //*****************************************************************************//
    //*******************src  YUY2/NV21/YV12 **************************************//
    //*****************************************************************************//
    if(g_ucPlane ==1)
    {
        FDstream.setSrcBuffer((void *)src_yp, g_FDW*g_FDH*2);
        FDstream.setSrcConfig(g_FDW, g_FDH, DP_COLOR_YUYV);
    }

    else if(g_ucPlane ==2)
    {
        src_addr_list[0] = (void *)src_yp;
        src_addr_list[1] = src_up;
        src_size_list[0] = src_ysize;
        src_size_list[1] = src_usize + src_vsize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, g_ucPlane);
        //FDstream.setSrcConfig(g_FDW, g_FDH, eYUV_420_2P_YVYU); //89
        FDstream.setSrcConfig(g_FDW, g_FDH, DP_COLOR_NV21); //82&72
    }

    else if(g_ucPlane ==3)
    {
        src_addr_list[0] = (void *)src_yp;
        src_addr_list[1] = (void *)(src_up);
        src_addr_list[2] = (void *)(src_vp);

        src_size_list[0] = src_ysize;
        src_size_list[1] = src_vsize;
        src_size_list[2] = src_usize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, g_ucPlane);
        //FDstream.setSrcConfig(g_FDW, g_FDH, eYUV_420_3P_YVU); //89
        FDstream.setSrcConfig(g_FDW, g_FDH, DP_COLOR_YV12); //82&72
    }

    //***************************dst RGB565********************************//
    FDstream.setDstBuffer((void *)dst_rgb, g_uwDst_width*g_uwDst_height*2); // IMAGE_MAX_WIDTH*IMAGE_MAX_HEIGHT*2
    //FDstream.setDstConfig(g_uwDst_width, g_uwDst_height, g_uwDst_width * 2, eRGB565);
    FDstream.setDstConfig(g_uwDst_width, g_uwDst_height, eRGB565);
    FDstream.setRotate(0);

    //*****************************************************************************//

    //MY_LOGD("Pipe_DDP_Performance_RGB565 Start");

//***********************************************************************//
//Adjust FD thread priority to RR
//***********************************************************************//
    //int const policy    = SCHED_OTHER;
#if MTKCAM_HAVE_RR_PRIORITY
    int policy    = SCHED_RR;
    int priority  = PRIO_RT_FD_THREAD;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
#endif
//***********************************************************************//

    // set & add pipe to stream
    if (0>FDstream.invalidate())  //trigger HW
    {
          MY_LOGD("FDstream invalidate failed");
          //***********************************************************************//
          //Adjust FD thread priority to Normal and return false
          //***********************************************************************//
#if MTKCAM_HAVE_RR_PRIORITY
          policy    = SCHED_OTHER;
          priority  = 0;
          ::sched_getparam(0, &sched_p);
          sched_p.sched_priority = priority;  //  Note: "priority" is nice value
          sched_setscheduler(0, policy, &sched_p);
          setpriority(PRIO_PROCESS, 0, priority);
#endif
          return false;
    }

 //***********************************************************************//
//Adjust FD thread priority to Normal
//***********************************************************************//
#if MTKCAM_HAVE_RR_PRIORITY
     policy    = SCHED_OTHER;
     priority  = 0;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
#endif
//***********************************************************************//

    //MY_LOGD("Pipe_DDP_Performance_RGB565 End");
#endif
     return ret;
}

/*******************************************************************************
* Extract Y from YUYV
********************************************************************************/

MINT32
halFDVT::halFDYUYV2ExtractY(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT32 src_width,
MUINT32 src_height
)
{
    MY_LOGD("DO Extract Y In");
    //Source Width, Height --> g_FDH, g_FDW
    int i,j;

    //int length = g_FDH*g_FDW*2;
    int length = src_width*src_height*2;

    for(i=length;i != 0;i-=2)
    {
      *dstAddr++ = *srcAddr;
      srcAddr+=2;
    }

/*
    for(i=0;i<g_FDH;i++)
    {
        for(j=0;j<g_FDW*2;j=j+2)
        {
            *(dstAddr+i*g_FDW+j/2) = *(srcAddr+i*g_FDW*2+j);
        }
    }
*/
    MY_LOGD("DO Extract Y Out");

   return MHAL_NO_ERROR;
}

/*******************************************************************************
* Create RGB565 QVGA for Tracking
********************************************************************************/

MINT32
halFDVT::halFTBufferCreate(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT8 ucPlane,
MUINT32 src_width,
MUINT32 src_height
)
{
    g_ucPlane = ucPlane;
    g_uwDst_width = IMAGE_MAX_WIDTH;
    g_uwDst_height = IMAGE_MAX_WIDTH * src_height / src_width;
    //g_uwDst_height = IMAGE_MAX_WIDTH * g_FDH / g_FDW;

    if((src_width == 640) && (g_ucPlane ==1))
    {
        MY_LOGD("DO RGB565 (SW) In");
        doRGB565BufferP1_SW(dstAddr, srcAddr, src_width, src_height);
        MY_LOGD("DO RGB565 (SW) Out");
    }
    else if((src_width == 320) && (g_ucPlane ==1))
    {
        MY_LOGD("DO RGB565 (SW_QVGA) In");
        doRGB565BufferQVGAP1_SW(dstAddr, srcAddr, src_width, src_height);
        MY_LOGD("DO RGB565 (SW_QVGA) Out");
    }
    else
    {
        MY_LOGD("DO RGB565 (DDP) In");
        doRGB565Buffer_DDP(dstAddr, srcAddr);
        MY_LOGD("DO RGB565 (DDP) Out");
    }

   return MHAL_NO_ERROR;
}

MINT32
halFDVT::halFTBufferCreate2(
MUINT8 *dstAddr,
MUINT8 *srcAddr0,
MUINT8 *srcAddr1,
MUINT8 *srcAddr2,
MUINT8 ucPlane,
MUINT32 src_width,
MUINT32 src_height
)
{
    g_ucPlane = ucPlane;
    g_uwDst_width = IMAGE_MAX_WIDTH;
    g_uwDst_height = IMAGE_MAX_WIDTH * src_height / src_width;
    //g_uwDst_height = IMAGE_MAX_WIDTH * g_FDH / g_FDW;

    if((src_width == 640) && (g_ucPlane ==1))
    {
        MY_LOGD("DO RGB565 (SW) In");
        doRGB565BufferP1_SW(dstAddr, srcAddr0, src_width, src_height);
        MY_LOGD("DO RGB565 (SW) Out");
    }
    else if((src_width == 320) && (g_ucPlane ==1))
    {
        MY_LOGD("DO RGB565 (SW_QVGA) In");
        doRGB565BufferQVGAP1_SW(dstAddr, srcAddr0, src_width, src_height);
        MY_LOGD("DO RGB565 (SW_QVGA) Out");
    }
    else
    {
        MY_LOGD("DO RGB565 (DDP) In");
        doRGB565Buffer_DDP(dstAddr, srcAddr0, srcAddr1, srcAddr2);
        MY_LOGD("DO RGB565 (DDP) Out");
    }

   return MHAL_NO_ERROR;
}


//0: 640x480 (NV21), 1: 4:3 (NV21), 2:  4:3 (YV12), 3: 16:9 (NV21),  4: 16:9(YV12), 5: Not Support!

MINT32
halFDVT::halFDBufferCreate(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT8  ucBufferGroup
)
{
    //MY_LOGD("halFDBufferCreate Start");

    g_BufferGroup = ucBufferGroup;

    switch(ucBufferGroup)
    {
        case 0:
                  g_uwDst_width = IMAGE_MAX_WIDTH;
                  g_uwDst_height = IMAGE_MAX_HEIGHT;
                  doRGB565Buffer_SW(dstAddr, srcAddr);
                  break;
        case 1:
                  g_ucPlane = 2;
                  g_uwDst_width = IMAGE_MAX_WIDTH;
                  g_uwDst_height = IMAGE_MAX_HEIGHT;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 2:
                  g_ucPlane = 3;
                  g_uwDst_width = IMAGE_MAX_WIDTH;
                  g_uwDst_height = IMAGE_MAX_HEIGHT;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 3:
                  g_ucPlane = 2;
                  g_uwDst_width = IMAGE_MAX_WIDTH;
                  g_uwDst_height = IMAGE_MAX_WIDTH*9/16;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 4:
                  g_ucPlane = 3;
                  g_uwDst_width = IMAGE_MAX_WIDTH;
                  g_uwDst_height = IMAGE_MAX_WIDTH*9/16;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 5:
                  g_ucPlane = 2;
                  g_uwDst_width = IMAGE_MAX_WIDTH;
                  g_uwDst_height = IMAGE_MAX_WIDTH*3/5;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 6:
                  g_ucPlane = 3;
                  g_uwDst_width = IMAGE_MAX_WIDTH;
                  g_uwDst_height = IMAGE_MAX_WIDTH*3/5;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        default:
                 MY_LOGD("Unknow Group: %d,", ucBufferGroup);
                 break;
     }

     //MY_LOGD("halFDBufferCreate End");


#if (DUMP_IMAGE)
    char szFileName[100]={'\0'};
    char szFileName1[100]={'\0'};
    FILE * pRawFp;
    FILE * pRawFp1;
    int i4WriteCnt;

    MY_LOGD("ucBufferGroup: %d,", ucBufferGroup);

    sprintf(szFileName1, "/sdcard/srcOK_%04d_%04d_YCC420.raw", g_FDW,g_FDH);
    pRawFp1 = fopen(szFileName1, "wb");
    if (NULL == pRawFp1 )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt = fwrite((void *)srcAddr,1, (g_FDW * g_FDH * 1.5),pRawFp1);
    fflush(pRawFp1);
    fclose(pRawFp1);

    sprintf(szFileName, "/sdcard/dstOK_%04d_%04d_RGB565.raw", g_uwDst_width,g_uwDst_height);
    pRawFp = fopen(szFileName, "wb");
    if (NULL == pRawFp )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt = fwrite((void *)dstAddr,1, (g_uwDst_width * g_uwDst_height * 2),pRawFp);
    fflush(pRawFp);
    fclose(pRawFp);
#endif

     return MHAL_NO_ERROR;

}


/*******************************************************************************
* Create RGB565 QVGA for ASD
********************************************************************************/
//0: 640x480 (NV21), 1: 4:3 (NV21), 2:  4:3 (YV12), 3: 16:9 (NV21),  4: 16:9(YV12), 5: Not Support!

MINT32
halFDVT::halFDASDBufferCreate(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT8  ucPlane
)
{
    g_ucPlane = ucPlane;
    g_uwDst_width = ASD_IMAGE_WIDTH;
    g_uwDst_height = ASD_IMAGE_WIDTH * g_FDH / g_FDW;
    doRGB565Buffer_DDP(dstAddr, srcAddr);

    return MHAL_NO_ERROR;
}
static int logidx = 0;
/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDDo(
MUINT8 *ScaleImages,
MUINT8 *ImageBuffer1, //RGB565 QVGA
MUINT8 *ImageBuffer2, //Source Buffer (Y)
MBOOL   SDEnable,
MINT32 rRotation_Info,
MUINT8 *ImageBufferPhy    //Source Buffer Physical
)
{
    Mutex::Autolock _l(gInitLock);
    g_CurrentStatus = SDEnable;
    if( (g_CurrentStatus == 1) && (IMAGE_MAX_WIDTH != 320))
    {
        MY_LOGW("Incorrect halFDDo (SD): SDEnable:%d, img_w:%d, img_h:%d,", g_CurrentStatus, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);
        return MHAL_INPUT_SIZE_ERROR;
    }
    MY_LOGD("SD Status: SDEnable:%d, img_w:%d, img_h:%d, Src_Phy_Addr:0x%x,", g_CurrentStatus, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT, ImageBufferPhy);

    if(g_BufferGroup > 6)
    {
        MY_LOGD("Error BufferGoup:%d,", g_BufferGroup);
        return MHAL_INPUT_SIZE_ERROR;
    }

    MUINT8* y_vga;

    if( g_input_vga == 0 && g_input_4_3 == 0 && g_input_16_9 == 0)
          return MHAL_INPUT_SIZE_ERROR;

    if(!m_Inited) {
        return MHAL_UNINIT_ERROR;
    }

    MY_LOGD("[halFDDo] IN,  %d", mFDCount);
    FACEDETECT_GSENSOR_DIRECTION direction;
    if( rRotation_Info == 0)
        direction = FACEDETECT_GSENSOR_DIRECTION_0;
    else if( rRotation_Info == 90)
        direction = FACEDETECT_GSENSOR_DIRECTION_270;
    else if( rRotation_Info == 270)
        direction = FACEDETECT_GSENSOR_DIRECTION_90;
    else if( rRotation_Info == 180)
        direction = FACEDETECT_GSENSOR_DIRECTION_180;
    else
        direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;



    //direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;
    int alreay_have_scale_images = 0;
    if(EnableSWResizerFlag)
    {
        FDVT_OPERATION_MODE_ENUM mode;
        m_pMTKFDVTObj->FDVTGetMode(&mode);

        //MY_LOGD("FDVTGetMode IN");

        if(g_input_vga == 1)
        {
            y_vga = (MUINT8*)ImageBuffer2;
            if( mode == FDVT_GFD_MODE)
            {
                CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_input_width_vga, image_input_height_vga, image_width_array, image_height_array, 1);
                //CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array, FD_SCALES);
                alreay_have_scale_images = 1;
            }
        }
        else if(g_input_4_3 == 1)
        {
            if(SDEnable)
            {
                Create640WidthImage_Y((MUINT8*)ImageBuffer2, m_FDW, m_FDH, ImageVGABuffer);
                y_vga = ImageVGABuffer;
                CreateScaleImagesFromVGA_Y(y_vga, ImageScaleBuffer,image_input_width_vga, image_input_height_vga, image_width_array, image_height_array, FD_SCALES);
                alreay_have_scale_images = 1;
            }
            else
            {
                if( mode == FDVT_GFD_MODE)
                {
                    CreateScaleImagesFromPreview_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_FDW, m_FDH, image_width_array, image_height_array, 1);
                    //CreateScaleImagesFromPreview_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_FDW, m_FDH, image_width_array, image_height_array, FD_SCALES);

                    alreay_have_scale_images = 1;
                    MY_LOGD("FDVT_GFD_MODE 4:3 out");
                }
            }
        }
        else if(g_input_16_9 == 1)
        {
            if(SDEnable || mode == FDVT_GFD_MODE)
            {
                MY_LOGD("g_input_16_9 IN");
                Create640WidthImage_Y((MUINT8*)ImageBuffer2, m_FDW, m_FDH, ImageVGABuffer);
                /*
                static int count1 = 0;
        FILE* fp;
        char filename[200];
        static int count = 0;
        sprintf(filename, "/data/fd_data/Y_VGA_buffer_%d_%d_%d.raw", m_FDW, m_FDH, count1);
        fp = fopen(filename, "wb");
        fwrite((unsigned char*)ImageVGABuffer, m_FDW*m_FDH, 1, fp);
        fclose(fp);
                count1++;
                */
                y_vga = ImageVGABuffer;
                CreateScaleImagesFromVGA_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_FDW, m_FDH, image_width_array, image_height_array, 1);

                //CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array, FD_SCALES);
                alreay_have_scale_images = 1;
            }
        }

        MY_LOGD("FDVTMain IN");
        m_pMTKFDVTObj->FDVTMain( ImageScaleBuffer, ImageBuffer1, FDVT_GFD_MODE, direction, 0, ImageBufferPhy);
        /*
        FILE* fp;
        char filename[200];
        static int count = 0;
        sprintf(filename, "/data/fd_data/Y_buffer_%d_%d_%d.raw", image_width_array[0], image_height_array[0], count);
        fp = fopen(filename, "wb");
        fwrite((unsigned char*)ImageScaleBuffer, image_width_array[0]*image_height_array[0], 1, fp);
        fclose(fp);

        sprintf(filename, "/data/fd_data/RGB_buffer_%d_%d_%d.rgb565", image_width_array[0], image_height_array[0], count);
        fp = fopen(filename, "wb");
        fwrite((unsigned char*)ImageBuffer1, image_width_array[0]*image_height_array[0]*2, 1, fp);
        fclose(fp);
        count++;
        */
        MY_LOGD("FDVTMain out", mFDCount);

        result  DetectResult[MAX_FACE_NUM];
        m_pMTKFDVTObj->FDVTGetResult( (MUINT8 *)&DetectResult, FACEDETECT_TRACKING_DISPLAY);
        /*
        static int last_face_num;
        if( last_face_num == 1 && DetectResult->face_num ==0 )
        {
            if(g_input_vga == 1)
            {
                  y_vga = (MUINT8*)ImageBuffer2;
                  if(alreay_have_scale_images == 0)
                  {
                      CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer,image_input_width_vga, image_input_height_vga, image_width_array, image_height_array, 1);
                      //CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array, FD_SCALES);
                }
            }
            else if(g_input_4_3 == 1)
            {
                  if(alreay_have_scale_images == 0)
                  {
                    CreateScaleImagesFromPreview_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_FDW, m_FDH, image_width_array, image_height_array, 1);
                    //CreateScaleImagesFromPreview_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_FDW, m_FDH, image_width_array, image_height_array, FD_SCALES);
                }
            }
            else if(g_input_16_9 == 1)
            {
                  Create640WidthImage_Y((MUINT8*)ImageBuffer2, m_FDW, m_FDH, ImageVGABuffer);
                y_vga = ImageVGABuffer;

                if(alreay_have_scale_images == 0)
                  {
                    CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, m_FDW,m_FDH, image_width_array, image_height_array, 1);
                    //CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array, FD_SCALES);
                }
            }
            for(int i=0; i<2;i++)
            {
                m_pMTKFDVTObj->FDVTMain( (MUINT32)ImageScaleBuffer, (MUINT32)ImageBuffer1, FDVT_GFD_MODE, direction, i);
                m_pMTKFDVTObj->FDVTGetResult( (MUINT32)DetectResult, FACEDETECT_TRACKING_DISPLAY);
                if(DetectResult->face_num > 0 )
                    break;
            }

        }
        last_face_num = DetectResult->face_num;
        */

        if(SDEnable)
        {
            MY_LOGD("FDVT_SD_MODE IN");
            m_pMTKFDVTObj->FDVTMain( ImageScaleBuffer, y_vga, FDVT_SD_MODE, direction, 0, ImageBufferPhy);
            MY_LOGD("FDVT_SD_MODE out");
        }
    }
    else
    {
            m_pMTKFDVTObj->FDVTMain( ScaleImages, ImageBuffer1, FDVT_GFD_MODE, direction, 0, ImageBufferPhy);

            if(SDEnable)
            {
                m_pMTKFDVTObj->FDVTMain( ScaleImages,ImageBuffer2, FDVT_SD_MODE, direction, 0, ImageBufferPhy);
          }
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDUninit(
)
{
    //MHAL_LOG("[halFDUninit] IN \n");
    Mutex::Autolock _l(gInitLock);

    if(!m_Inited) {
        MY_LOGW("FD HAL Object is already uninited...");
        return MHAL_NO_ERROR;
    }

    m_pMTKFDVTObj->FDVTReset();

    if(EnableSWResizerFlag)
    {
         delete[]ImageScaleBuffer;
    }
    if(g_input_vga == 0)
    {
        delete[]ImageVGABuffer;
    }

    m_Inited = 0;

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDDrawFaceRect(
    MUINT8 *pbuf
)
{
    MINT32 err = MHAL_NO_ERROR;

    return err;
}

MINT32
halFDVT::halSDDrawFaceRect(
    MUINT8 *pbuf
)
{
    MINT32 err = MHAL_NO_ERROR;

    return err;
}

/*******************************************************************************
*
********************************************************************************/

MINT32
halFDVT::halFDGetFaceInfo(
    MtkCameraFaceMetadata *fd_info_result
)
{
    MUINT8 i;
    MY_LOGD("[GetFaceInfo] NUM_Face:%d,", FdResult_Num);

    if( (FdResult_Num < 0) || (FdResult_Num > 15) )
         FdResult_Num = 0;

    fd_info_result->number_of_faces =  FdResult_Num;

    for(i=0;i<FdResult_Num;i++)
    {
     fd_info_result->faces[i].rect[0] = FdResult[i].rect[0];
    fd_info_result->faces[i].rect[1] = FdResult[i].rect[1];
    fd_info_result->faces[i].rect[2] = FdResult[i].rect[2];
    fd_info_result->faces[i].rect[3] = FdResult[i].rect[3];
    fd_info_result->faces[i].score = FdResult[i].score;
          fd_info_result->posInfo[i].rop_dir = FdResult[i].rop_dir;
          fd_info_result->posInfo[i].rip_dir  = FdResult[i].rip_dir;
    }

    return MHAL_NO_ERROR;
}

MINT32
halFDVT::halFDGetFaceResult(
    MtkCameraFaceMetadata *fd_result,
    MINT32 ResultMode
)
{
    if( (g_CurrentStatus == 1) && (IMAGE_MAX_WIDTH != 320))
    {
        MY_LOGW("Incorrect GetFaceResult(SD): SDEnable:%d, img_w:%d, img_h:%d,", g_CurrentStatus, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);
        return 0;
    }

    MINT32 faceCnt = 0;
    MUINT8 pbuf[1024];
    MUINT8 i;
    MINT8 DrawMode = 0;
    g_Lcm_rotate = g_Sensor_rotate =g_CameraTYPE = 0;

    faceCnt=m_pMTKFDVTObj->FDVTGetResult((MUINT8 *) pbuf, (FACEDETECT_TRACKING_RESULT_TYPE_ENUM)ResultMode);

    m_DispW = image_width_array[0];
    m_DispH = image_height_array[0];;

    MY_LOGD("[%s]m_DispW(%d) m_DispH(%d)", __FUNCTION__, m_DispW, m_DispH);
    m_pMTKFDVTObj->FDVTGetICSResult((MUINT8 *) fd_result, (MUINT8 *) pbuf, m_DispW, m_DispH, g_Lcm_rotate, g_Sensor_rotate, g_CameraTYPE, DrawMode);

    FdResult_Num = fd_result->number_of_faces;

    //MY_LOGD("[GetFaceNum] NUM_Face:%d,", FdResult_Num);

    //Facial Size Filter
    if((FdResult_Num == 1) && (faceCnt ==1))
    {
        //g_FDSizeRatio = 0.3; //forTest
        result* face_info=(result*)pbuf;

        int diff = face_info[0].y1-face_info[0].y0;
        float ratio = (float)diff / (float)m_DispH;

        //MY_LOGD("bin: y1:%d, y0:%d, diff:%d, ratio:%f, ratio_Thr:%f, ", face_info[0].y1, face_info[0].y0, diff, ratio, g_FDSizeRatio);
        if( ratio < g_FDSizeRatio)
        {
            fd_result->number_of_faces = 0;
            FdResult_Num = 0;
            faceCnt = 0;
        }
    }

    for(i=0;i<FdResult_Num;i++)
    {
          FdResult[i].rect[0] = fd_result->faces[i].rect[0];
          FdResult[i].rect[1] = fd_result->faces[i].rect[1];
          FdResult[i].rect[2] = fd_result->faces[i].rect[2];
          FdResult[i].rect[3] = fd_result->faces[i].rect[3];
          //FdResult[i].score = fd_result->faces[i].score;
          FdResult[i].score = fd_result->faces[i].score/2+50; // mapping [0:100]=> [50:100]
          FdResult[i].rop_dir = fd_result->posInfo[i].rop_dir;
          FdResult[i].rip_dir  = fd_result->posInfo[i].rip_dir;
          //MY_LOGD("[GetFaceScore] Score:%d,", FdResult[i].score);
    }
    for(i=FdResult_Num;i<MAX_FACE_NUM;i++)
    {
          FdResult[i].rect[0] = 0;
          FdResult[i].rect[1] = 0;
          FdResult[i].rect[2] = 0;
          FdResult[i].rect[3] = 0;
          FdResult[i].score = 0;
          FdResult[i].rop_dir = 0;
          FdResult[i].rip_dir  = 0;
          //MY_LOGD("[GetFaceScore] Score:%d,", FdResult[i].score);
    }

    return faceCnt;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::mHalFDSetDispInfo(
    MUINT32 x,
    MUINT32 y,
    MUINT32 w,
    MUINT32 h,
    MUINT32 Lcm_rotate,
    MUINT32 Sensor_rotate,
    MINT32 CameraTYPE
)
{
    MINT32 err = MHAL_NO_ERROR;

    // x,y is offset from left-top corner
    // w,h is preview frame width and height seen on LCD
    m_DispX = x;
    m_DispY = y;
    m_DispW = w;
    m_DispH = h;
    m_DispRoate = (Sensor_rotate << 5) | (Lcm_rotate <<2 ) | (CameraTYPE);

    g_Lcm_rotate = Lcm_rotate;
    g_Sensor_rotate = Sensor_rotate;
    g_CameraTYPE = CameraTYPE;

    //LOGE("[FDdraw_SetDisp] Lcm_rotate %d Sensor_rotate %d m_DispRoate %d \n",Lcm_rotate,Sensor_rotate,m_DispRoate);

    return err;
}


/*******************************************************************************
*
********************************************************************************/

MINT32
halFDVT::halSetDetectPara(MUINT8 Para)
{
    MINT32 err = MHAL_NO_ERROR;

    m_DetectPara = Para;
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halSDGetSmileResult()
{
    if( (g_CurrentStatus == 1) && (IMAGE_MAX_WIDTH != 320))
    {
        MY_LOGW("Incorrect GetSmileResult(SD): SDEnable:%d, img_w:%d, img_h:%d,", g_CurrentStatus, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);
        return 0;
    }

    MINT32 SmileCnt = 0;
    MUINT32 pbuf1;

    SmileCnt = m_pMTKFDVTObj->FDVTGetSDResult(pbuf1);

    return SmileCnt;
}


static void Create640WidthImage_Y(MUINT8* src_img, MUINT32 src_w, MUINT32 src_h, MUINT8* dst_img)
{
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    UtlRisizerInfo.srcAddr = src_img;
    UtlRisizerInfo.srcWidth= src_w;
    UtlRisizerInfo.srcHeight= src_h;
    UtlRisizerInfo.dstAddr = dst_img;
    UtlRisizerInfo.dstWidth = image_input_width_vga;
    if(g_input_4_3 == 1)
    {
        UtlRisizerInfo.dstHeight = image_input_height_vga;
    }
    else if(g_input_16_9 == 1)
    {
        UtlRisizerInfo.dstHeight = src_h*image_input_width_vga/src_w;
    }
    PipeBilinearResizer(&UtlRisizerInfo);
}

static void CreateScaleImagesFromPreview_Y(MUINT8* image_buffer_preview, MUINT8* image_buffer, MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles)
{
    MINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8* dst_ptr;
    dst_ptr = image_buffer;

    for ( current_scale = 0 ; current_scale < sacles ; current_scale ++ )
    {

          UtlRisizerInfo.srcAddr = image_buffer_preview;
        UtlRisizerInfo.srcWidth= w;
        UtlRisizerInfo.srcHeight= h;
        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[current_scale];
        UtlRisizerInfo.dstHeight = ImageHeightArray[current_scale];
        if (UtlRisizerInfo.dstWidth == 0 || UtlRisizerInfo.dstHeight==0)
        {
            MY_LOGD("PipeBilinearResizer break %d", current_scale);
            break;
        }
        PipeBilinearResizer(&UtlRisizerInfo);
        dst_ptr+= ImageWidthArray[current_scale]*ImageHeightArray[current_scale];
        MY_LOGD("PipeBilinearResizer %d", current_scale);
    }

}
static void CreateSingleImageFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles)
{
    MINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8* dst_ptr;
    dst_ptr = image_buffer;

        UtlRisizerInfo.srcAddr = image_buffer_vga;
        UtlRisizerInfo.srcWidth= image_input_width_vga;
        UtlRisizerInfo.srcHeight= image_input_height_vga;
        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[0];
        UtlRisizerInfo.dstHeight = ImageHeightArray[0];
        PipeBilinearResizer(&UtlRisizerInfo);

        /*
        FILE* fp;
        char filename[20];
        sprintf(filename, "%d_%dx%d.raw", 0, ImageWidthArray[0], ImageHeightArray[0]);
        fp =fopen(filename, "wb");
        fwrite( (void*)dst_ptr, ImageWidthArray[0]*ImageHeightArray[0], 1, fp);
        fclose(fp);
        */


}


static void CreateScaleImagesFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer,MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles)
{
    MINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8* dst_ptr;
    dst_ptr = image_buffer;

    for ( current_scale = 0 ; current_scale < sacles ; current_scale ++ )
    {
          UtlRisizerInfo.srcAddr = image_buffer_vga;
        UtlRisizerInfo.srcWidth= w;
        UtlRisizerInfo.srcHeight= h;
        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[current_scale];
        UtlRisizerInfo.dstHeight = ImageHeightArray[current_scale];
        PipeBilinearResizer(&UtlRisizerInfo);
        /*
        FILE* fp;
        char filename[20];
        sprintf(filename, "%d_%dx%d.raw", current_scale, ImageWidthArray[current_scale], ImageHeightArray[current_scale]);
        fp =fopen(filename, "wb");
        fwrite( (void*)dst_ptr, ImageWidthArray[current_scale]*ImageHeightArray[current_scale], 1, fp);
        fclose(fp);
        */
        dst_ptr+= ImageWidthArray[current_scale]*ImageHeightArray[current_scale];
    }

}


