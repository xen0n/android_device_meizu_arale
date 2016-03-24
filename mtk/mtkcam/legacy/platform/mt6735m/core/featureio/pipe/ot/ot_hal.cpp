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
** $Log: ot_hal.cpp $
 *
*/
#define LOG_TAG "mHalOT"
#include <utils/Errors.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/atomic.h>
#include <mtkcam/common/faces.h>
#include "ot_hal.h"
#include <mtkcam/Log.h>

#include "MTKOT.h"
#include "camera_custom_ot.h"
#include <DpBlitStream.h>
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>

#define DUMP_IMAGE (0)

#define MHAL_NO_ERROR 0
#define MHAL_ERROR 1

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
//  Global object tracking related parameter //
//-------------------------------------------//
//
static halOTBase *pHalOT = NULL;
volatile MINT32   mOTCount = 0;

MUINT32 g_OTw = 0;
MUINT32 g_OTh = 0;

static unsigned short int  g_tempW = 160;
static unsigned short int  g_tempH = 120;
static unsigned short int frmW = 320;
static unsigned short int frmH = 240;
static unsigned short int  g_Dst_width =640;
static unsigned short int  g_Dst_height = 480;
unsigned char  g_Plane=1;

MTKOT* pOTInterface;
MTKOTProcInfo OTProcInfo;
MTKOTGetProcInfo OTGetProcInfo;
MTKOTEnvInfo OTEnvInfo;
MTKOTTuningPara OTTuningPara;
MTKOTResultInfo OTResultInfo;
MRESULT Retcode = 0;


static int InitargetFlag = 0;
static int InitargetX = 0;
static int InitargetY = 0;

static int First_Frame_Numiter_shape_I = 6;
static int MaxObjHalfSize = 85;
static int MinObjHalfSize = 12;
static int CrSch_StepSize_I = 0;
static float ARFA = 0.0;
static int only_reshape_at_begining = 0;
static int IniwinW = 15;
static int IniwinH = 15;
static int incre = 7;
static int Ob_BoundaryMargin = 21;
static int Tsmth_ChangeStepSize_i = 2;
static int Numiter_position_I = 5;
static int Numiter_shape_I = 1;
static int Numiter_shape_F = 1;
static float LtOcOb_ColorSimilarity_TH = 0.45;
static int StartfrmIdx = 0;
static float AreaParam_sigma = 3.0;
static float COARSE_SEARCH_NON_CENTOR_TH = 1.0;

static int EIS_GMV_CTRL = 0;
static int KmF_SkipFrm = 5;
static int KmF_TotalFrm = 0;
static int ColorSpace = 0;
static int Tsmth_frmNum = 7;
//static int OBLoseTrackingFrm = 5;
//static int OCLoseTrackingFrm = 5;
//static int OBLoseTrackingFrm = 100000;
//static int OCLoseTrackingFrm = 100000;
static int OBLoseTrackingFrm = 60;
static int OCLoseTrackingFrm = 60;
static int LightResistance = 1;

MUINT8 *dstAddr1 = 0;

unsigned char* working_buffer;

static int g_StartOT_X = -1000;
static int g_StartOT_Y = -1000;
/*******************************************************************************
*
********************************************************************************/
halOTBase*
halOT::
getInstance()
{
    MY_LOGD("[Create] Before:%d,", mOTCount);
    //MHAL_LOG("[halOT] getInstance \n");
    if (pHalOT == NULL) {
        pHalOT = new halOT();
    }

    android_atomic_inc(&mOTCount);

    MY_LOGD("[Create] After:%d,", mOTCount);

    return pHalOT;
}

/*******************************************************************************
*
********************************************************************************/
void
halOT::
destroyInstance()
{
     MY_LOGD("[Delete] Before:%d,", mOTCount);

    // More than one user
    if (mOTCount > 0)
          android_atomic_dec(&mOTCount);

   if (mOTCount == 0)
   {
        if (pHalOT)
        {
            delete pHalOT;
        }
        pHalOT = NULL;
   }

   MY_LOGD("[Delete] After:%d,", mOTCount);
}

/*******************************************************************************
*
********************************************************************************/
halOT::halOT()
{
    m_pMTKOTObj = NULL;

    m_OTw = 0;
    m_OTh = 0;

    m_pMTKOTObj = MTKOT::createInstance(DRV_OT_OBJ_SW_SINGLE);

    //MHAL_LOG("m_pMTKOTObj = MTKOT::createInstance(DRV_OT_OBJ_SW_SINGLE)\n");
}

halOT::~halOT()
{
    m_OTw = 0;
    m_OTh = 0;

    if (m_pMTKOTObj) {
        m_pMTKOTObj->destroyInstance();
    }
    m_pMTKOTObj = NULL;
}

MINT32
halOT::halOTInit(
    MUINT32 OTw,
    MUINT32 OTh,
    MUINT32 WorkingBuffer,
    MUINT32 WorkingBufferSize
)
{
    MINT32 err = MHAL_NO_ERROR;

    MY_LOGD("[mHalOTInit] Start \n");
    g_OTw = OTw;
    g_OTh = OTh;
    m_OTw = OTw;
    m_OTh = OTh;

    g_tempH = g_tempW * g_OTh / g_OTw;
    if((g_tempH % 2) != 0)
       g_tempH ++;
    frmH = g_tempH * 2;
    g_Dst_height = g_tempH * 4;

    MY_LOGD("[mHalOTInit]SrcW:%d, SrcH:%d, ", g_OTw, g_OTh);
    MY_LOGD("[mHalOTInit]frmW:%d, frmH:%d, ", frmW, frmH );
    MY_LOGD("[mHalOTInit]Width:%d, Height:%d, ", g_Dst_width, g_Dst_height );

    pOTInterface = MTKOT::createInstance(DRV_OT_OBJ_SW_SINGLE);

    OTProcInfo.SrcImgWidth = frmW;
    OTProcInfo.SrcImgHeight = frmH;

    Retcode = pOTInterface->OTFeatureCtrl(MTKOT_FEATURE_GET_PROC_INFO, &OTProcInfo, &OTGetProcInfo);

    int working_buffer_size = OTGetProcInfo.WorkingBufferSize;
    working_buffer = new unsigned char[working_buffer_size];

    OTEnvInfo.SrcImgWidth = frmW;
    OTEnvInfo.SrcImgHeight = frmH;
    OTEnvInfo.SrcImgFormat = MTKOT_IMAGE_YUV444;
    OTEnvInfo.WorkingBufAddr = working_buffer;
    OTEnvInfo.WorkingBufSize = working_buffer_size;
    OTEnvInfo.DebugInfoFlag = 0;
    OTEnvInfo.pTuningPara = &OTTuningPara;

    //Customer Parameters
    OT_Customize_PARA OTCustomData;
    get_ot_CustomizeData(&OTCustomData);
    OTEnvInfo.pTuningPara->OBLoseTrackingFrm = OTCustomData.OBLoseTrackingFrm;
    OTEnvInfo.pTuningPara->OCLoseTrackingFrm = OTCustomData.OCLoseTrackingFrm;
    OTEnvInfo.pTuningPara->LtOcObTH = OTCustomData.LtOcOb_ColorSimilarity_TH;
    OTEnvInfo.pTuningPara->ARFA = OTCustomData.ARFA;
    OTEnvInfo.pTuningPara->LComputeShapeF = OTCustomData.Numiter_shape_F;
    OTEnvInfo.pTuningPara->LightResistance = OTCustomData.LightResistance;
    OTEnvInfo.pTuningPara->MaxObjHalfSize = OTCustomData.MaxObjHalfSize;
    OTEnvInfo.pTuningPara->MinObjHalfSize = OTCustomData.MinObjHalfSize;
    OTEnvInfo.pTuningPara->IniWinW = OTCustomData.IniwinW;
    OTEnvInfo.pTuningPara->IniWinH = OTCustomData.IniwinH;

    //Non-Customer Prarmeters
    OTEnvInfo.pTuningPara->FirstFrameLComputeShape = First_Frame_Numiter_shape_I;
    OTEnvInfo.pTuningPara->DisplacementRange = CrSch_StepSize_I;
    OTEnvInfo.pTuningPara->OnlyReshapeAtBeginning = only_reshape_at_begining;
    OTEnvInfo.pTuningPara->Incre = incre;
    OTEnvInfo.pTuningPara->OBBoundaryMargin = Ob_BoundaryMargin;
    OTEnvInfo.pTuningPara->TsmthChangeStepSize = Tsmth_ChangeStepSize_i;
    OTEnvInfo.pTuningPara->LComputePositionI = Numiter_position_I;
    OTEnvInfo.pTuningPara->LComputeShapeI = Numiter_shape_I;
    OTEnvInfo.pTuningPara->TsmthfrmNum = Tsmth_frmNum;
    //OTEnvInfo.pTuningPara->IniWinW = IniwinW;
    //OTEnvInfo.pTuningPara->IniWinH = IniwinH;
    //OTEnvInfo.pTuningPara->MaxObjHalfSize = MaxObjHalfSize;
    //OTEnvInfo.pTuningPara->MinObjHalfSize = MinObjHalfSize;

    Retcode = pOTInterface->OTInit(&OTEnvInfo, 0);

    OTProcInfo.InitargetFlag = true;

    g_StartOT_X = -1000;
    g_StartOT_Y = -1000;

    return err;
}

bool doQVGABuffer_SW(MUINT8 *a_dstAddr1, MUINT8 *a_dstAddr)
{
    bool ret = true;

    int i,j;

    unsigned char* pucYBuf = (unsigned char *)a_dstAddr;
    unsigned char* pucUBuf = pucYBuf + g_Dst_width * g_Dst_height;
    unsigned char* pucVBuf = pucUBuf + frmW * frmH;
    unsigned char* pucY1Buf = (unsigned char *)a_dstAddr1;
    unsigned char* pucU1Buf = pucY1Buf + frmW * frmH;
    unsigned char* pucV1Buf = pucU1Buf + frmW * frmH;

    int Y[4];

    int half_width = g_Dst_width/2;

    for(i=0;i<g_Dst_height;i=i+2)
    {
        for(j=0;j<g_Dst_width;j=j+2)
        {
            Y[0] = *(pucYBuf + ((i+0) * g_Dst_width) + j);
            Y[1] = *(pucYBuf + ((i+0) * g_Dst_width) + j+1) ;
            Y[2] = *(pucYBuf + ((i+1) * g_Dst_width) + j) ;
            Y[3] = *(pucYBuf + ((i+1) * g_Dst_width) + j+1) ;

            Y[0]= (Y[0]+Y[1]+Y[2]+Y[3]) >> 2;
            *(pucY1Buf + ((i/2) * half_width) + j/2) = Y[0];
        }
    }

    memcpy(pucU1Buf, pucUBuf, frmW * frmH);
    memcpy(pucV1Buf, pucVBuf, frmW * frmH);
    return ret;
}

bool doYV12Buffer_DDP(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr)
{
    bool ret = true;

    DpBlitStream FDstream;
    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_yp = (unsigned char *)a_dstAddr;
    void* src_addr_list[3];
    unsigned int src_size_list[3];
    void* dst_addr_list[3];
    unsigned int dst_size_list[3];

    int dst_ysize, dst_usize;
    int src_ysize = g_OTw * g_OTh;
    int src_usize, src_vsize;
    src_usize = src_vsize = src_ysize / 4;

    //*****************************************************************************//
    //*******************src  YUY2/NV21/YV12 **************************************//
    //*****************************************************************************//
    if(g_Plane ==1)
    {
        FDstream.setSrcBuffer((void *)src_yp, g_OTw*g_OTh*2);
        FDstream.setSrcConfig(g_OTw, g_OTh, DP_COLOR_YUYV);
    }

    else if(g_Plane ==2)
    {
        //src_addr_list[0] = (unsigned int)src_yp;
        //src_addr_list[1] = (unsigned int)(src_yp + src_ysize);
        src_addr_list[0] = (void *)src_yp;
        src_addr_list[1] = (void *)((unsigned long)src_yp + src_ysize);
        src_size_list[0] = src_ysize;
        src_size_list[1] = src_usize + src_vsize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, g_Plane);
        FDstream.setSrcConfig(g_OTw, g_OTh, DP_COLOR_NV21);
    }

    else if(g_Plane ==3)
    {
        //src_addr_list[0] = (unsigned int)src_yp;
        //src_addr_list[1] = (unsigned int)(src_yp + src_ysize);
        //src_addr_list[2] = (unsigned int)(src_yp + src_ysize + src_usize);
        src_addr_list[0] = (void *)src_yp;
        src_addr_list[1] = (void *)((unsigned long)src_yp + src_ysize);
        src_addr_list[2] = (void *)((unsigned long)src_yp + src_ysize + src_usize);


        src_size_list[0] = src_ysize;
        src_size_list[1] = src_vsize;
        src_size_list[2] = src_usize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, g_Plane);
        FDstream.setSrcConfig(g_OTw, g_OTh, DP_COLOR_YV12);
    }

    //*******************dst (YUV 420 3plane)************************//
    dst_ysize = g_Dst_width*g_Dst_height;
    dst_usize = dst_ysize / 4;

    int plane_num = 3;
    dst_yp = (unsigned char *)a_dstAddr;
    //dst_addr_list[0] = (unsigned int)dst_yp;
    //dst_addr_list[1] = (unsigned int)(dst_yp + dst_ysize);
    //dst_addr_list[2] = (unsigned int)(dst_yp + dst_ysize + dst_usize);
    dst_addr_list[0] = (void *)dst_yp;
    dst_addr_list[1] = (void *)((unsigned long)dst_yp + dst_ysize);
    dst_addr_list[2] = (void *)((unsigned long)dst_yp + dst_ysize + dst_usize);


    dst_size_list[0] = dst_ysize;
    dst_size_list[1] = dst_usize;
    dst_size_list[2] = dst_usize;

    FDstream.setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num);
    FDstream.setDstConfig(g_Dst_width, g_Dst_height, DP_COLOR_YV12);
    FDstream.setRotate(0);

    //***************************dst RGB565********************************//
    //FDstream.setDstBuffer((void *)dst_rgb, g_Dst_width*g_Dst_height*2); // 320*240*2
    //FDstream.setDstConfig(g_Dst_width, g_Dst_height, eRGB565);
    //FDstream.setRotate(0);

    //*****************************************************************************//

    //MY_LOGD("Pipe_DDP_Performance_RGB565 Start");

//***********************************************************************//
//Adjust FD thread priority to RR
//***********************************************************************//
    //int const policy    = SCHED_OTHER;
#if (0)
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
#if (0)
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
#if (0)
     policy    = SCHED_OTHER;
     priority  = 0;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
#endif
//***********************************************************************//

    //MY_LOGD("Pipe_DDP_Performance_RGB565 End");

     return ret;
}

/*******************************************************************************
* Create YV12(YVV420 3plane) VGA for Tracking
********************************************************************************/

MINT32
halOT::halOTBufferCreate(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT8  ucPlane
)
{
    //MY_LOGD("halFDBufferCreate Start");

    g_Plane = ucPlane;
    //g_Dst_width = 640;
    //g_Dst_height = 480;
    doYV12Buffer_DDP(dstAddr, srcAddr);

    dstAddr1 = dstAddr + g_Dst_width * g_Dst_height * 2;

    //MY_LOGD("halFDBufferCreate End");

#if (DUMP_IMAGE)
    char szFileName[100]={'\0'};
    char szFileName1[100]={'\0'};
    FILE * pRawFp;
    FILE * pRawFp1;
    int i4WriteCnt;

    sprintf(szFileName1, "/sdcard/srcOK_%04d_%04d_YV12.raw", g_OTw,g_OTh);
    pRawFp1 = fopen(szFileName1, "wb");
    if (NULL == pRawFp1 )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt = fwrite((void *)srcAddr,1, (g_OTw * g_OTh * 1.5),pRawFp1);
    fflush(pRawFp1);
    fclose(pRawFp1);

    sprintf(szFileName, "/sdcard/dstOK_%04d_%04d_YV12.raw", g_Dst_width,g_Dst_height);
    pRawFp = fopen(szFileName, "wb");
    if (NULL == pRawFp )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt = fwrite((void *)dstAddr,1, (g_Dst_width * g_Dst_height * 1.5),pRawFp);
    fflush(pRawFp);
    fclose(pRawFp);
#endif

    doQVGABuffer_SW(dstAddr1, dstAddr); //YUV 444 3 plane

#if (0)
    char szFileName2[100]={'\0'};
    FILE * pRawFp2;
    int i4WriteCnt1;

    sprintf(szFileName2, "/sdcard/otOK_%04d_%04d_YUV444.raw", frmW, frmH);
    pRawFp2 = fopen(szFileName2, "wb");
    if (NULL == pRawFp2 )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt1 = fwrite((void *)dstAddr1,1, (frmW * frmH * 3),pRawFp2);
    fflush(pRawFp2);
    fclose(pRawFp2);
#endif

    //dstAddr = dstAddr1;

    return MHAL_NO_ERROR;

}

/*******************************************************************************
*
********************************************************************************/
MINT32
halOT::halOTDo(
MUINT8 *ImageBuffer1, //QVGA YUV444 (3plane)
MINT32 a_InitargetX,
MINT32 a_InitargetY
)
{
    if((a_InitargetX<-1000) ||  (a_InitargetX>1000) || (a_InitargetY<-1000) ||  (a_InitargetY>1000))
    {
        MY_LOGD("Error! Incorrect Coordinate\n");
        return MHAL_ERROR;
    }

    dstAddr1 = ImageBuffer1 + g_Dst_width * g_Dst_height * 2;

    //MY_LOGD("g_OTw is %d\n", g_OTw);
    //MY_LOGD("g_OTh is %d\n", g_OTh);

    //Coordinate Transformation (-1000 ~ +1000 -> 320x240)
    InitargetX = (a_InitargetX+1000)*frmW/2000;
    InitargetY = (a_InitargetY+1000)*frmH/2000;

    //InitargetX = 160;
    //InitargetY = 120;

    OTProcInfo.SrcImgChannel1 = dstAddr1;
    OTProcInfo.SrcImgChannel2 = OTProcInfo.SrcImgChannel1 + (frmW * frmH);
    OTProcInfo.SrcImgChannel3 = OTProcInfo.SrcImgChannel2 + (frmW * frmH);

    if((a_InitargetX != g_StartOT_X) || (a_InitargetY != g_StartOT_Y))
    {
        g_StartOT_X = a_InitargetX;
        g_StartOT_Y = a_InitargetY;
        OTProcInfo.InitargetFlag = true;
    }

    OTProcInfo.InitargetCenterX = InitargetX;
    OTProcInfo.InitargetCenterY = InitargetY;

    MY_LOGD("DO OT: OTFlag:%d, InitX:%d, InitY:%d,\n", OTProcInfo.InitargetFlag, OTProcInfo.InitargetCenterX, OTProcInfo.InitargetCenterY);

#if (0)
    char szFileName2[100]={'\0'};
    FILE * pRawFp2;
    int i4WriteCnt1;
    ot_index1++;

    sprintf(szFileName2, "/sdcard/otOK1_%04d_%04d_YUV444_%d.raw", frmW, frmH,ot_index1);
    pRawFp2 = fopen(szFileName2, "wb");
    if (NULL == pRawFp2 )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt1 = fwrite((void *)dstAddr1,1, (frmW * frmH * 3),pRawFp2);
    fflush(pRawFp2);
    fclose(pRawFp2);
#endif

    Retcode = pOTInterface->OTFeatureCtrl(MTKOT_FEATURE_SET_PROC_INFO, &OTProcInfo, 0);
    Retcode = pOTInterface->OTMain();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halOT::halOTUninit(
)
{
    if(pOTInterface != NULL)
    {
        Retcode = pOTInterface->OTExit();
        pOTInterface->destroyInstance();
        pOTInterface = NULL;
    }

    delete[] working_buffer;
    return MHAL_NO_ERROR;
}

MINT32
halOT::halOTGetResult(
MtkCameraFaceMetadata *ot_result
)
{
    MINT32 faceCnt = 1;

    Retcode = pOTInterface->OTFeatureCtrl(MTKOT_FEATURE_GET_RESULT, 0, &OTResultInfo);

    if(OTResultInfo.LoseTrackingFlag == 1)
    {
        ot_result->number_of_faces = 0;
        ot_result->faces[0].score = 0;
        OTProcInfo.InitargetFlag = true;
        MY_LOGD("Tracking fail.\n");
        return ot_result->number_of_faces;
    }
    else
    {
       if(OTResultInfo.OBCondition )
       {
           //object moves to out of boundary
           //Draw Gray rectangle
       }
       else if(OTResultInfo.OCCondition || OTResultInfo.LTCondition)
       {
           //occulusion or temporal lose tracking
           //Draw Black rectangle
       }
       else
       {
           //tracking successe
           //Draw White rectangle
       }
    }

    ot_result->number_of_faces = 1;
    ot_result->faces[0].rect[0] = OTResultInfo.SmoothX0*2000/frmW - 1000;
    ot_result->faces[0].rect[1] = OTResultInfo.SmoothY0*2000/frmH - 1000;
    ot_result->faces[0].rect[2] = OTResultInfo.SmoothX1*2000/frmW - 1000;
    ot_result->faces[0].rect[3] = OTResultInfo.SmoothY1*2000/frmH - 1000;

    if(OTResultInfo.OBCondition || OTResultInfo.OCCondition || OTResultInfo.LTCondition)
        ot_result->faces[0].score = 50;
    else if(OTResultInfo.LoseTrackingFlag == 0)
        ot_result->faces[0].score = 100;

    ot_result->faces[0].id = 0;
    ot_result->faces[0].left_eye[0] = -2000;
    ot_result->faces[0].left_eye[1] = -2000;
    ot_result->faces[0].right_eye[0] = -2000;
    ot_result->faces[0].right_eye[1] = -2000;
    ot_result->faces[0].mouth[0] = -2000;
    ot_result->faces[0].mouth[1] = -2000;

    ot_result->posInfo[0].rop_dir = 0;
    ot_result->posInfo[0].rip_dir = 0;
    OTProcInfo.InitargetFlag = false;

    MY_LOGD("Tracking Success. Flag:%d, X0:%d, X1:%d,", OTProcInfo.InitargetFlag, OTResultInfo.X0, OTResultInfo.X1);
    MY_LOGD("Tracking Success. Score:%d, OB:%d, OC:%d, LT:%d, ", ot_result->faces[0].score, OTResultInfo.OBCondition,
                                                   OTResultInfo.OCCondition, OTResultInfo.LTCondition);



    return ot_result->number_of_faces;
}

