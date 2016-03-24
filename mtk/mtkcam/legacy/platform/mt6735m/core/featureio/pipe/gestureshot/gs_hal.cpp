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
** $Log: GS_hal.cpp $
 *
*/
#define LOG_TAG "mHalGS"

#include <string.h>
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/atomic.h>
#include <mtkcam/common/faces.h>
#include "gs_hal.h"
#include <mtkcam/Log.h>

#include "MTKGD.h"
#include "camera_custom_gesture.h"

#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
using namespace android;

#define DUMP_IMAGE (0)

//****************************//
#define GS_SCALES 11

static MUINT32 image_width_array[GS_SCALES] = {320, 256, 204, 160, 128, 102, 80, 64, 50, 40, 32};
static MUINT32 image_height_array[GS_SCALES] = {240, 192, 152, 120, 96, 76, 60, 48, 38, 30, 24};

static MUINT32 IMAGE_MAX_WIDTH  = image_width_array[0];
static MUINT32 IMAGE_MAX_HEIGHT = image_height_array[0];
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
static void CreateScaleImagesFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray);
static void CreateScaleImagesFromPreview_Y(MUINT8* image_buffer_preview, MUINT8* image_buffer, MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray);

#define MHAL_NO_ERROR 0
#define MHAL_INPUT_SIZE_ERROR 1

#define MAX_GS_NUM 15

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
//  Global gesture shot related parameter  //
//-------------------------------------------//
//
static halGSBase *pHalGS = NULL;

static MINT32 GSResult_Num;
static GS_RESULT GSResult[15];

MUINT32 g_GSW = 0;
MUINT32 g_GSH = 0;

volatile MINT32     mGSCount = 0;
static Mutex       gsLock;
static Mutex       gInitLock;


static unsigned short int  g_uwDst_width =  IMAGE_MAX_WIDTH;
static unsigned short int  g_uwDst_height = IMAGE_MAX_HEIGHT;

//int g_count = 0;

/*******************************************************************************
*
********************************************************************************/
halGSBase*
halGS::
getInstance()
{
    Mutex::Autolock _l(gsLock);
    MY_LOGD("[Create] &mGSCount:%p &gsLock:%p", &mGSCount, &gsLock);
    int const oldCount = ::android_atomic_inc(&mGSCount);

    int g_oldCount = oldCount;
    if(g_oldCount < 0)
    {
        MY_LOGW("Warning! [Create] count:%d->%d pHalGS:%p", oldCount, mGSCount, pHalGS);
        g_oldCount = 0;
        mGSCount = 1;
        pHalGS = NULL;
    }

    MY_LOGD("[Create] g_oldCount:%d->%d pHalGS:%p", g_oldCount, mGSCount, pHalGS);
    if  ( 0 == oldCount )
    {
        if  ( ! pHalGS )
        {
            MY_LOGW("Get Instance Warning!");
        }
        pHalGS = new halGS();
    }
    return  pHalGS;

/*
    MY_LOGD("[Create] Before:%d,", mGSCount);
    //MHAL_LOG("[halGS] getInstance \n");
    if (pHalGS == NULL) {
        pHalGS = new halGS();
    }

    android_atomic_inc(&mGSCount);

    MY_LOGD("[Create] After:%d,", mGSCount);

    return pHalGS;
*/
}

/*******************************************************************************
*
********************************************************************************/
void
halGS::
destroyInstance()
{
    Mutex::Autolock _l(gsLock);
    MY_LOGD("[Delete] &mGSCount:%p &gsLock:%p", &mGSCount, &gsLock);
    int const oldCount = ::android_atomic_dec(&mGSCount);
    MY_LOGD("[Delete] count:%d->%d pHalGS:%p", oldCount, mGSCount, pHalGS);
    if  ( 0 == oldCount || ! pHalGS )
    {
        MY_LOGW("Delete Instance Warning!");
    }

    if  ( 1 == oldCount )
    {
        delete pHalGS;
        pHalGS = NULL;
    }

/*
     MY_LOGD("[Delete] Before:%d,", mGSCount);

    // More than one user
    if (mGSCount > 0)
          android_atomic_dec(&mGSCount);

   if (mGSCount == 0)
   {
        if (pHalGS)
        {
            delete pHalGS;
        }
        pHalGS = NULL;
   }

   MY_LOGD("[Delete] After:%d,", mGSCount);
*/
}

/*******************************************************************************
*
********************************************************************************/
halGS::halGS()
{
    m_pMTKGSObj = NULL;

    m_GSW = 0;
    m_GSH = 0;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;
    //mGSCount = 0;
    m_Inited = 0;

    m_pMTKGSObj = MTKGestureDetector::createInstance(DRV_GD_OBJ_SW);

    //MHAL_LOG("m_pMTKGSObj = MTKDetection::createInstance(DRV_GS_OBJ_GSFT_SW)\n");
}


halGS::~halGS()
{
    m_GSW = 0;
    m_GSH = 0;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;

    if (m_pMTKGSObj) {
        m_pMTKGSObj->destroyInstance();
    }
    m_pMTKGSObj = NULL;
}


MINT32
halGS::halGSInit(
    MUINT32 GSW,
    MUINT32 GSH,
    MUINT8* WorkingBuffer,
    MUINT32 WorkingBufferSize
)
{
    MUINT32 i;
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* Cdata=NULL;
    MTKGDInitInfo GSInitInfo;
    GS_Customize_PARA GSCustomData;

    Mutex::Autolock _l(gInitLock);
//**********************************************************************//
    MY_LOGD("[mHalGSInit] Start");

    g_GSW = GSW;
    g_GSH = GSH;
    m_GSW = GSW;
    m_GSH = GSH;

    if(m_GSW == image_input_width_vga && m_GSH == image_input_height_vga)
    {
        g_input_vga = 1;
        g_input_4_3 = 0;
        g_input_16_9 = 0;
    }
    else if( m_GSW*3 == m_GSH*4)
    {
        g_input_vga = 0;
        g_input_4_3 = 1;
        g_input_16_9 = 0;

    }
    //else if( m_GSW*9 == m_GSH*16)
    else if( (m_GSW*9 == m_GSH*16) || (m_GSW*3 == m_GSH*5))
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
    //MHAL_ASSERT(m_pMTKGSObj != NULL, "m_pMTKGSObj is NULL");

    get_gesture_CustomizeData(&GSCustomData);

    GSInitInfo.WorkingBufAddr = WorkingBuffer;
    GSInitInfo.WorkingBufSize = WorkingBufferSize;
//    GSInitInfo.GSThreadNum = GSCustomData.GSThreadNum;
//    GSInitInfo.GSThreshold = GSCustomData.GSThreshold;
//    GSInitInfo.GSSkipStep = GSCustomData.GSSkipStep;
    GSInitInfo.GDThreadNum = 4;
    GSInitInfo.GDThreshold = 32;
    GSInitInfo.GDSkipStep = 4;
    GSInitInfo.GDBufWidth = GSW;
    GSInitInfo.GDBufHeight = GSH;
    GSInitInfo.GSensor = 1;
    GSInitInfo.GDLevel = GSCustomData.GDLevel;
    MY_LOGD("Customer Data: GDLevel:%d", GSInitInfo.GDLevel);

    //MY_LOGD("Customer Data: 1:%d, 2:%d, 3:%d, 4:%d, 5:%d, 6:%d, 7:%d", GSInitInfo.GSThreadNum, GSInitInfo.GSThreshold,  GSInitInfo.MajorFaceDecision,
    //                                                        GSInitInfo.OTRatio, GSInitInfo.SmoothLevel, GSFTInitInfo.GSSkipStep, GSFTInitInfo.GSRefresh);

    m_pMTKGSObj->GdInit(&GSInitInfo);

    EnableSWResizerFlag = 1;
    if(EnableSWResizerFlag)
    {
        ImageScaleTotalSize = 0;
        for(i=0; i<GS_SCALES;i++)
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
    m_Inited = 1;
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
halGS::halGSDo(
MUINT8 *ImageBuffer2, //Preview Size Image
MINT32 rRotation_Info
)
{
    MUINT8* y_vga;

    if( g_input_vga == 0 && g_input_4_3 == 0 && g_input_16_9 == 0)
          return MHAL_INPUT_SIZE_ERROR;

    MY_LOGD("[halGSDo] IN,  %d", mGSCount);
    GD_GSENSOR_DIRECTION direction;
    if( rRotation_Info == 0)
        direction = GD_GSENSOR_DIRECTION_0;
    else if( rRotation_Info == 90)
        direction = GD_GSENSOR_DIRECTION_270;
    else if( rRotation_Info == 270)
        direction = GD_GSENSOR_DIRECTION_90;
    else if( rRotation_Info == 180)
        direction = GD_GSENSOR_DIRECTION_180;
    else
        direction = GD_GSENSOR_DIRECTION_NO_SENSOR;


    //direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;
    int alreay_have_scale_images = 0;
    if(EnableSWResizerFlag)
    {
        if(g_input_vga == 1)
        {
            y_vga = (MUINT8*)ImageBuffer2;
            CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
            alreay_have_scale_images = 1;
        }
        else if(g_input_4_3 == 1)
        {
            CreateScaleImagesFromPreview_Y( ImageBuffer2, ImageScaleBuffer, m_GSW, m_GSH, image_width_array, image_height_array);
        }
        else if(g_input_16_9 == 1)
        {
            Create640WidthImage_Y(ImageBuffer2, m_GSW, m_GSH, ImageVGABuffer);
            y_vga = ImageVGABuffer;
            CreateScaleImagesFromVGA_Y( y_vga, ImageScaleBuffer, image_width_array, image_height_array);
            alreay_have_scale_images = 1;
        }

        m_pMTKGSObj->GdMain( ImageScaleBuffer, MTK_GD_GFD_MODE, direction, 0);

        //result  DetectResult[MAX_FACE_NUM];
        GD_RESULT DetectResult[MAX_FACE_NUM];
        m_pMTKGSObj->GdGetResult( (MUINT8 *)&DetectResult, GD_TRACKING_DISPLAY);

        static int last_face_num;
        if( last_face_num == 1 && DetectResult->face_num ==0 )
        {
            if(g_input_vga == 1)
            {
                y_vga = (MUINT8*)ImageBuffer2;
                if(alreay_have_scale_images == 0)
                {
                    CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
                }
            }
            else if(g_input_4_3 == 1)
            {
                if(alreay_have_scale_images == 0)
                {
                    CreateScaleImagesFromPreview_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_GSW, m_GSH, image_width_array, image_height_array);
                }
            }
            else if(g_input_16_9 == 1)
            {
                Create640WidthImage_Y((MUINT8*)ImageBuffer2, m_GSW, m_GSH, ImageVGABuffer);
                y_vga = ImageVGABuffer;

                if(alreay_have_scale_images == 0)
                {
                    CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
                }
            }
            for(int i=0; i<2;i++)
            {
                m_pMTKGSObj->GdMain( ImageScaleBuffer, MTK_GD_GFD_MODE, direction, i);
                m_pMTKGSObj->GdGetResult( (MUINT8 *)DetectResult, GD_TRACKING_DISPLAY);
                if(DetectResult->face_num > 0 )
                    break;
            }
        }
        last_face_num = DetectResult->face_num;
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halGS::halGSUninit(
)
{
    Mutex::Autolock _l(gInitLock);
    //MHAL_LOG("[halGSUninit] IN \n");
    if(!m_Inited) {
        MY_LOGW("GS HAL Object is already uninited...");
        return MHAL_NO_ERROR;
    }
    m_pMTKGSObj->GdReset();

    if(EnableSWResizerFlag)
    {
         delete[]ImageScaleBuffer;
         EnableSWResizerFlag = 0;
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
halGS::halGSGetGestureResult(
    MtkCameraFaceMetadata *GS_result
)
{
    MINT32 faceCnt = 0;
    MUINT8 pbuf[1024];
    MUINT8 i;

    faceCnt=m_pMTKGSObj->GdGetResult((MUINT8 *) pbuf, GD_TRACKING_DISPLAY);

    m_DispW = image_width_array[0];
    m_DispH = image_height_array[0];;

    m_pMTKGSObj->GdGetICSResult((MUINT8 *) GS_result, (MUINT8 *) pbuf, m_DispW, m_DispH, 0, 0, 0, 0);

    GSResult_Num = GS_result->number_of_faces;

    for(i=0;i<MAX_FACE_NUM;i++)
    {
          GSResult[i].rect[0] = GS_result->faces[i].rect[0];
          GSResult[i].rect[1] = GS_result->faces[i].rect[1];
          GSResult[i].rect[2] = GS_result->faces[i].rect[2];
          GSResult[i].rect[3] = GS_result->faces[i].rect[3];
          GSResult[i].score = GS_result->faces[i].score;
          GSResult[i].rop_dir = GS_result->posInfo[i].rop_dir;
          GSResult[i].rip_dir  = GS_result->posInfo[i].rip_dir;
    }

    return faceCnt;
}

/*******************************************************************************
*
********************************************************************************/

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

static void CreateScaleImagesFromPreview_Y(MUINT8* image_buffer_preview, MUINT8* image_buffer, MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray)
{
    MINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8* dst_ptr;
    dst_ptr = image_buffer;

    for ( current_scale = 0 ; current_scale < GS_SCALES ; current_scale ++ )
    {
          UtlRisizerInfo.srcAddr = image_buffer_preview;
        UtlRisizerInfo.srcWidth= w;
        UtlRisizerInfo.srcHeight= h;
        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[current_scale];
        UtlRisizerInfo.dstHeight = ImageHeightArray[current_scale];
        PipeBilinearResizer(&UtlRisizerInfo);
        dst_ptr+= ImageWidthArray[current_scale]*ImageHeightArray[current_scale];
    }

}

static void CreateScaleImagesFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray)
{
    MINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8 *dst_ptr, *src_ptr;
    dst_ptr = image_buffer;

    for ( current_scale = 0 ; current_scale < GS_SCALES ; current_scale ++ )
    {
#if 0
          UtlRisizerInfo.srcAddr = image_buffer_vga;
        UtlRisizerInfo.srcWidth= image_input_width_vga;
        UtlRisizerInfo.srcHeight= image_input_height_vga;
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
#endif
        if (current_scale == 0)
        {
            UtlRisizerInfo.srcAddr = image_buffer_vga;
            UtlRisizerInfo.srcWidth= image_input_width_vga;
            UtlRisizerInfo.srcHeight= image_input_height_vga;
        }
        else
        {
            UtlRisizerInfo.srcAddr = src_ptr;
            UtlRisizerInfo.srcWidth= ImageWidthArray[current_scale - 1];
            UtlRisizerInfo.srcHeight= ImageHeightArray[current_scale - 1];
        }

        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[current_scale];
        UtlRisizerInfo.dstHeight = ImageHeightArray[current_scale];
        PipeBilinearResizer(&UtlRisizerInfo);

        src_ptr = dst_ptr;
        dst_ptr+= ImageWidthArray[current_scale]*ImageHeightArray[current_scale];
    }

}


