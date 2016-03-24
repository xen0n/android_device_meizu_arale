#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <string.h>
#include <ctype.h>

#include <cutils/log.h>    // for log

#include "../../../camshot/inc/MfllCommon.h"  //@todo for TEST_MEMC_YUY2, remove this later
#ifndef TEST_MEMC_YUY2
    #error "must define TEST_MEMC_YUY2"
#endif
#define MTK_LOG_ENABLE 1
#include <mtkcam/algorithm/libmfbll/AppMfbll.h>
#include "mfll_hal.h"

/*------------------------------------------------------------------------------
    enum
------------------------------------------------------------------------------*/
enum MFLLHAL_DUMP {
    MFLLHAL_DUMP_MEMC       = 0x01,
    MFLLHAL_DUMP_SWMIXER    = 0x02,
};

/*------------------------------------------------------------------------------
    Porting
------------------------------------------------------------------------------*/
#define MFBLL_LOGD  ALOGD
#define MFBLL_LOGV  ALOGV
#define MTRUE       1
#define MFALSE      0

#define MFBLL_HAL_TAG       "{MfbllHal} "
#define MY_LOGD(fmt, arg...)        {ALOGD(MFBLL_HAL_TAG fmt, ##arg);printf(MFBLL_HAL_TAG fmt "\n", ##arg);}
#define FUNCTION_LOG_START      MY_LOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        MY_LOGD("[%s] - X.", __FUNCTION__)


/*------------------------------------------------------------------------------
    Mixer Parameters
------------------------------------------------------------------------------*/
#define PREFIX "/sdcard/data/"

/*------------------------------------------------------------------------------
    MEMC Parameters
------------------------------------------------------------------------------*/
#if TEST_MEMC_YUY2
    static char base_name[256]= PREFIX "qdip0.yuv_1600x1200.yuy2";
    static char input_name[256]= PREFIX "qdip0.yuv_input_1600x1200.yv16";
    static char ref_name[256]= PREFIX "qdip1.yuv_1600x1200.yuy2";
    static char out_name[256]= PREFIX "dump_mc1.yuv_1600x1200.yuy2";
#else
    static char base_name[256]= PREFIX "qdip0.yuv_1600x1200.yv16";
    static char input_name[256]= PREFIX "qdip0.yuv_input_1600x1200.yv16";
    static char ref_name[256]= PREFIX "qdip1.yuv_1600x1200.yv16";
    static char out_name[256]= PREFIX "dump_mc1.yuv_1600x1200.yv16";
#endif


/*------------------------------------------------------------------------------
    Static Variable
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Local Functions
------------------------------------------------------------------------------*/
MfbllHal* MfbllHal::getInstance(char const *userName)
{
    FUNCTION_LOG_START;
    //@TODO
    FUNCTION_LOG_END;
    return NULL;
}


void MfbllHal::destroyInstance(char const *userName)
{
    FUNCTION_LOG_START;
    //@TODO
    FUNCTION_LOG_END;
}


MfbllHal::MfbllHal()
{
    FUNCTION_LOG_START;
    //@TODO
    FUNCTION_LOG_END;
}


MfbllHal::~MfbllHal()
{
    FUNCTION_LOG_START;
    //@TODO
    FUNCTION_LOG_END;
}


//----------------------------------------------------------------------------------------

MBOOL MfbllHal::init(void *pInitInData)
{
    FUNCTION_LOG_START;

#if 1
    // settings for dump buffer
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("mediatek.mfllhal.dump", value, "0");
    mDump = atoi(value);
    //
    property_get("mediatek.mfllhal.dump.memc", value, "0");
    MBOOL dumpMemc = atoi(value);
    property_get("mediatek.mfllhal.dump.swmixer", value, "0");
    MBOOL dumpSwMixer = atoi(value);
    //
    if(dumpMemc)
        mDump |= MFLLHAL_DUMP_MEMC;
    if(dumpSwMixer)
        mDump |= MFLLHAL_DUMP_SWMIXER;
    MY_LOGD("mDump=0x%x", mDump);
#endif

    MFBLL_PIPE_INIT_INFO *pInfo = (MFBLL_PIPE_INIT_INFO *)pInitInData;
    mTuning = *pInfo;
    MY_LOGD("input_width=%d, input_height=%d", pInfo->input_width, pInfo->input_height);
    MY_LOGD("iso_value=%d, iso_high_th=%d, iso_low_th=%d", pInfo->iso_value, pInfo->iso_high_th, pInfo->iso_low_th);
    MY_LOGD("max_frame_number=%d, bss_clip_th=%d, memc_bad_mv_range=%d, memc_bad_mv_rate_th=%d", pInfo->max_frame_number, pInfo->bss_clip_th, pInfo->memc_bad_mv_range, pInfo->memc_bad_mv_rate_th);

    imgW = pInfo->input_width;
    imgH = pInfo->input_height;

    // Common declaration
    m_pMfbllDrv = MTKMfbll::createInstance(DRV_MFBLL_OBJ_SW);
    if(m_pMfbllDrv==NULL)
    {
        MY_LOGD("Mfbll get instance error");
        return -1;
    }

    // Mixer Parameters declaration
    ImgSize = ((imgW*imgH+31)>>5)<<5; // 32byte alignment   //2592*1968

    // Init MET
    // MEMC Init Parameters
    inMEWidth  = imgW/2;
    inMEWidth &= (~0x01);
    inMEHeight = imgH/2;
    inMEHeight &= (~0x01);
    MfbllInitPara.Proc1_Init.Proc1_imgW = inMEWidth  ;
    MfbllInitPara.Proc1_Init.Proc1_imgH = inMEHeight ;
    MY_LOGD("Proc1_imgW = %d", MfbllInitPara.Proc1_Init.Proc1_imgW);
    MY_LOGD("Proc1_imgH = %d", MfbllInitPara.Proc1_Init.Proc1_imgH);
    //
    MfbllInitPara.Proc1_Init.Proc1_tuning.core_num = 4;
    MfbllInitPara.Proc1_Init.Proc1_tuning.ISO_Value = pInfo->iso_value;         //1600
    MfbllInitPara.Proc1_Init.Proc1_tuning.ISO_HiTh = pInfo->iso_high_th;        //400
    MfbllInitPara.Proc1_Init.Proc1_tuning.ISO_LowTh = pInfo->iso_low_th;        //400
    //
    MfbllInitPara.Proc1_Init.Proc1_tuning.MaxFrmNum = pInfo->max_frame_number;  //4

    //
    MY_LOGD("Proc1_tuning.core_num = %d", MfbllInitPara.Proc1_Init.Proc1_tuning.core_num);
    MY_LOGD("Proc1_tuning.ISO_Value = %d", MfbllInitPara.Proc1_Init.Proc1_tuning.ISO_Value);
    MY_LOGD("Proc1_tuning.ISO_HiTh = %d", MfbllInitPara.Proc1_Init.Proc1_tuning.ISO_HiTh);
    MY_LOGD("Proc1_tuning.ISO_LowTh = %d", MfbllInitPara.Proc1_Init.Proc1_tuning.ISO_LowTh);
    MY_LOGD("Proc1_tuning.MaxFrmNum = %d", MfbllInitPara.Proc1_Init.Proc1_tuning.MaxFrmNum);

#if TEST_SWMIXER
    MfbllInitPara.Proc1_Init.Proc1_ISO_value = pInfo->iso_value;
    MY_LOGD("Proc1_ISO_value = %d", MfbllInitPara.Proc1_Init.Proc1_ISO_value);
#endif  //#if TEST_SWMIXER

    MemcImgsize = inMEHeight*inMEWidth;
    MemcImgBufsize = ((MemcImgsize+31)>>5)<<5;// 32byte alignment   //1296*984
    //mbNum = (inMEWidth*inMEHeight) >> 8;
    //matching_times = 122400 - mbNum * BLOCK_MATCHING_RESERVED_PER_MB_LOCAL;

#if TEST_SWMIXER
    // Mixer Init Parameters
    MfbllInitPara.Proc2_Init.Proc2_imgH = imgH;
    MfbllInitPara.Proc2_Init.Proc2_imgW = imgW;
    MfbllInitPara.Proc2_Init.Proc2_tuning.Level = 0;
    MY_LOGD("Proc2_imgH = %d", MfbllInitPara.Proc2_Init.Proc2_imgH);
    MY_LOGD("Proc2_imgW = %d", MfbllInitPara.Proc2_Init.Proc2_imgW);
    MY_LOGD("Proc2_tuning.Level = %d", MfbllInitPara.Proc2_Init.Proc2_tuning.Level);
#endif  //#if TEST_SWMIXER

    // Initial Mfbll
    MRESULT ret = m_pMfbllDrv->MfbllInit(&MfbllInitPara, NULL);
    if(ret) {
        MY_LOGD("MfbllInit error = %d", ret);
    }

    /*--------------------------------------Img Padding--------------------------------------*/

    /*-------------------------------------------Memc Start-----------------------------------*/
    // MEMC Image/Working Buffer Allocation
    m_pMfbllDrv->MfbllFeatureCtrl(MFBLL_FTCTRL_GET_PROC_INFO, &Proc1_get_proc_info, &Proc1_get_proc_info);
    MY_LOGD("Ext_mem_size = %d", Proc1_get_proc_info.Ext_mem_size);
    //@TODO remove this after merging last software algorithm
    Proc1_get_proc_info.Mfbll_frame_num = pInfo->max_frame_number;  //getFrameNumber(pInfo->iso_value);
    MY_LOGD("Mfbll_frame_num = %d", Proc1_get_proc_info.Mfbll_frame_num);
    FUNCTION_LOG_END;

    return MTRUE;
}


MBOOL MfbllHal::uninit()
{
    FUNCTION_LOG_START;
    m_pMfbllDrv->destroyInstance();
    FUNCTION_LOG_END;
    return MTRUE;
}


MBOOL MfbllHal::queryWorkingBufferSize(MUINT32 &workingBufferSize)
{
    FUNCTION_LOG_START;
    workingBufferSize = Proc1_get_proc_info.Ext_mem_size;
    FUNCTION_LOG_END;
    return MTRUE;
}


MBOOL MfbllHal::queryMemcBufferSize(MUINT32 &imageBufferSize)
{
    FUNCTION_LOG_START;
    imageBufferSize = MemcImgBufsize * 2;
    FUNCTION_LOG_END;
    return MTRUE;
}


MBOOL MfbllHal::querySwMixerBufferSize(MUINT32 &imageBufferSize)
{
    FUNCTION_LOG_START;
    imageBufferSize = ImgSize * 2;
    FUNCTION_LOG_END;
    return MTRUE;
}



MBOOL MfbllHal::setWorkingBuffer(MUINT8 *workingBuffer)
{
    FUNCTION_LOG_START;
    if(workingBuffer == NULL) {
        MY_LOGD("workingBuffer = %d", workingBuffer);
        return MFALSE;
    }
    WorkingBuffer = workingBuffer;
    FUNCTION_LOG_END;
    return MTRUE;
}


MBOOL MfbllHal::doMemc(MINT32 mvX, MINT32 mvY, MUINT8* baseBufAddr, MUINT8* refBufAddr, MUINT32 *badMvCount)
{
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;
    int err = 0;
    MINT32 j;
    timeval start_time;
    timeval end_time;
    MINT32 ME_time=0;

    //MEMC Processing Data Iinitialization
    //Set Proc Info
    Proc1_set_proc_info.workbuf_addr = WorkingBuffer;
    Proc1_set_proc_info.buf_size     = Proc1_get_proc_info.Ext_mem_size;
    Proc1_set_proc_info.base         = baseBufAddr;
    Proc1_set_proc_info.ref          = refBufAddr;
    Proc1_set_proc_info.Width        = inMEWidth;
    Proc1_set_proc_info.Height       = inMEHeight;
    Proc1_set_proc_info.Proc1_V1     = mvX;
    Proc1_set_proc_info.Proc1_V2     = mvY;
    //#define MEMC_BAD_MV_RANGE   32
    //#define MEMC_BAD_MB_RATE_TH 0.35
    //Proc1_set_proc_info.bad_mv_range = MEMC_BAD_MV_RANGE;
    //Proc1_set_proc_info.bad_mv_TH    = MEMC_BAD_MB_RATE_TH * ((inMEWidth * inMEHeight) >> 8);
    Proc1_set_proc_info.bad_mv_range = mTuning.memc_bad_mv_range;
    //Proc1_set_proc_info.bad_mv_TH    = (mTuning.memc_bad_mv_rate_th / 255.0) * ((inMEWidth * inMEHeight) >> 8);
    Proc1_set_proc_info.bad_mv_TH    = mTuning.memc_bad_mv_rate_th;

    #if TEST_MEMC_YUY2
    //Proc1_set_proc_info.ImgFmt       = PROC1_FMT_YUY2;  //@todo - PROC1_FMT_YV16 has better performance
    Proc1_set_proc_info.ImgFmt       = PROC1_FMT_YV16;  //@todo - PROC1_FMT_YV16 has better performance
    #else
    Proc1_set_proc_info.ImgFmt       = PROC1_FMT_YV16;  //@todo - PROC1_FMT_YV16 has better performance
    #endif


    MfbllMemcResult.Height = inMEHeight;
    MfbllMemcResult.Width = inMEWidth;
    #if TEST_MEMC_YUY2
    MfbllMemcResult.ImgFmt = PROC1_FMT_YUY2;            //@todo - PROC1_FMT_YV16 has better performance
    #else
    MfbllMemcResult.ImgFmt = PROC1_FMT_YV16;            //@todo - PROC1_FMT_YV16 has better performance
    #endif
    MfbllMemcResult.ResultImg = NULL;

    gettimeofday(&start_time, NULL);

    //MEMC Set Processing Data
    m_pMfbllDrv->MfbllFeatureCtrl(MFBLL_FTCTRL_SET_PROC1_INFO, &Proc1_set_proc_info, &Proc1_set_proc_info);

    gettimeofday(&end_time, NULL);
    ME_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec))/1000;
    MY_LOGD("   Clip Time : %d msec",ME_time/1);

    //MEMC Main process
    m_pMfbllDrv->MfbllMain(MFBLL_PROC1, &err, &MfbllMemcResult);
    *badMvCount = MfbllMemcResult.bad_mb_count;

    MUINT8 *ResultImg;
    MUINT32  Width;
    MUINT32  Height;
    PROC1_IMAGE_FORMAT ImgFmt;
    MUINT8   bSkip_MEMC;
    MY_LOGD("ResultImg(%p), Width(%d), Height(%d), ImgFmt(%d), bad_mb_count(%d), bSkip_MEMC(%d)"
            , MfbllMemcResult.ResultImg
            , MfbllMemcResult.Width
            , MfbllMemcResult.Height
            , MfbllMemcResult.ImgFmt
            , MfbllMemcResult.bad_mb_count
            , MfbllMemcResult.bSkip_MEMC
            );
    if(err) {
        MY_LOGD("MfbllMain error = %d", err);
    }

    if(mDump & MFLLHAL_DUMP_MEMC) {
        FILE *fptr;

        //MC Image (sharing same address space as frame1)
        fptr = fopen((const char*)out_name, "wb");
        if( fptr == NULL )
        {
            MY_LOGD("Output MC YUV file open error");
            exit(0);
        }
        fwrite(MfbllMemcResult.ResultImg, sizeof(MUINT8), MfbllMemcResult.Height*MfbllMemcResult.Width*2, fptr);
        fclose(fptr);

        fptr = fopen("mv_out.dat", "w");
        if( fptr == NULL )
        {
            MY_LOGD("Dump MV file open error");
            exit(0);
        }

        fclose(fptr);
    }

    if(MfbllMemcResult.bSkip_MEMC) {
        MY_LOGD("singleout skip memc");
        ret = MFALSE;
    }

    /*---------------------------------------------------------------------------------------*/
    /*---------------------------------------Do HW MFB Here----------------------------------*/
    /*---------------------------------------------------------------------------------------*/

    FUNCTION_LOG_END;
    return ret;
}

#if TEST_SWMIXER
MBOOL MfbllHal::doSwMixer(MUINT8 *inputBuffer1, MUINT8 *inputBuffer2, MUINT8 *weightingBuffer)
{
    FUNCTION_LOG_START;

    timeval start_time;
    timeval end_time;

    MINT32 i;
    Proc2_set_proc_info.workbuf_addr = WorkingBuffer;

    //Image buffer allocation
    Proc2_set_proc_info.Proc2_Img1   = inputBuffer1;
    Proc2_set_proc_info.Proc2_Img2   = inputBuffer2;
    Proc2_set_proc_info.Proc2_ImgW   = MfbllInitPara.Proc2_Init.Proc2_imgW;
    Proc2_set_proc_info.Proc2_ImgH   = MfbllInitPara.Proc2_Init.Proc2_imgH;
    Proc2_set_proc_info.weight_table = weightingBuffer;

    //Mixer Set Processing Data
    m_pMfbllDrv->MfbllFeatureCtrl(MFBLL_FTCTRL_SET_PROC2_INFO, &Proc2_set_proc_info, &Proc2_set_proc_info);

    //Main Function : W map LPF & Blending
    gettimeofday(&start_time, NULL);
    int ret;
    m_pMfbllDrv->MfbllMain(MFBLL_PROC2, &ret, &Mfbll_result);
    if(ret) {
        MY_LOGD("MfbllMain error = %d", ret);
    }

    gettimeofday(&end_time, NULL);
    MUINT32 LPF_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec))/1000;
    MY_LOGD("Mixer Time : %d msec",(LPF_time/1));

    if(mDump & MFLLHAL_DUMP_SWMIXER) {
        FILE *fptr;

        //Temp memory allocation just for result check.
        MUINT8 *ChromaBuffer = new MUINT8 [ImgSize];
        MUINT8 *U1Buffer = ChromaBuffer;
        MUINT8 *V1Buffer = U1Buffer + ImgSize/2;
        MUINT16 *Result = new MUINT16 [ImgSize*2];
        MUINT16 *Y_Tmp = Result;
        MUINT16 *U_Tmp = Y_Tmp + ImgSize;
        MUINT16 *V_Tmp = U_Tmp + ImgSize/2;

        fptr = fopen(input_blend_yuv, "rb");
        if(fptr==NULL)
            MY_LOGD("Cannot Open file %s", input_blend_yuv);
        fread(U1Buffer, sizeof(MUINT8), (imgW*imgH)/2, fptr);
        fread(V1Buffer, sizeof(MUINT8), (imgW*imgH)/2, fptr);
        fclose(fptr);

        //Write final result.
        fptr = fopen(out_final, "wb");
        fwrite(Mfbll_result.img_mfbll_result, sizeof(MUINT8), imgW * imgH, fptr);
        fwrite(U1Buffer, sizeof(MUINT8), imgW * imgH/2, fptr);
        fwrite(V1Buffer, sizeof(MUINT8), imgW * imgH/2, fptr);
        fclose(fptr);

        delete [] ChromaBuffer;
        delete [] Result;
    }

    FUNCTION_LOG_END;
    return 0;
}
#endif  //#if TEST_SWMIXER


#if TEST_MEMC_YUY2
static void yuy2_to_yv16(MUINT8* pSrc, MUINT8* pDst, MUINT32 Width, MUINT32 Height)
{
    MUINT8* pTemp = new MUINT8[Width*Height*2];
    MUINT8* pDstLuma = pTemp;
    MUINT8* pDstCb   = pDstLuma + Width*Height;
    MUINT8* pDstCr   = pDstCb   + (Width>>1)*Height;
    MINT32 i;
    MINT32 pixelnum  = Width*Height;

    for(i=0; i<pixelnum; i++)
    {
        *pDstLuma++ = *pSrc++;
        if(i&0x1) //Cr
        {
            *pDstCr++ = *pSrc++;
        }
        else //Cb
        {
            *pDstCb++ = *pSrc++;
        }
    }
    memcpy(pDst, pTemp, sizeof(MUINT8)*pixelnum*2);
    delete pTemp;
}
#endif


int mfll_hal_main(int argc, char* argv[])
{
    MFBLL_PIPE_INIT_INFO MfbllPipeInitInfo;
    MfbllPipeInitInfo.input_width = 3200;
    MfbllPipeInitInfo.input_height = 2400;
    MfbllPipeInitInfo.iso_value = 1600;

    MfbllHal *pMfllHal = new MfbllHal();
    pMfllHal->init(&MfbllPipeInitInfo);


    /*-------------------------------------------Working buffer Start-----------------------------------*/
    MUINT32 WorkingBufferSize;
    pMfllHal->queryWorkingBufferSize(WorkingBufferSize);
    printf("WorkingBufferSize = %d\n", WorkingBufferSize);
    MUINT8 *WorkingBuffer = new MUINT8 [WorkingBufferSize];
    if ((!WorkingBuffer))
    {
        MY_LOGD("memory init error");
        return -1;
    }
    pMfllHal->setWorkingBuffer(WorkingBuffer);

    /*-------------------------------------------MEMC Start-----------------------------------*/
    //Loop for Memc processing time average
    MUINT32 MemcImgBufsize;
    pMfllHal->queryMemcBufferSize(MemcImgBufsize);
    printf("MemcImgBufsize=%d\n", MemcImgBufsize);
    MUINT8* baseBufAddr = new MUINT8 [MemcImgBufsize];
    MUINT8* refBufAddr = new MUINT8 [MemcImgBufsize];
    MUINT32 badMvCount = 0;

    FILE *fptr;

    if(1) { //@TEST
        //Read input base, reference images
        fptr = fopen(base_name, "rb");
        if( fptr == NULL )
        {
            MY_LOGD("Fail to Open File ");
            exit(0);
        }
        //fread(baseBufAddr, sizeof(MUINT8), 54, fptr);
        fread(baseBufAddr, sizeof(MUINT8), MemcImgBufsize, fptr);
        fclose(fptr);
#if TEST_MEMC_YUY2
        yuy2_to_yv16(baseBufAddr, baseBufAddr, 1600, 1200);
#endif



        fptr = fopen(ref_name, "rb");
        if( fptr == NULL )
        {
            MY_LOGD("Fail to Open File");
            exit(0);
        }
        //fread(refBufAddr, sizeof(MUINT8), 54, fptr);
        fread(refBufAddr, sizeof(MUINT8), MemcImgBufsize, fptr);
        fclose(fptr);
#if TEST_MEMC_YUY2
        yuy2_to_yv16(refBufAddr, refBufAddr, 1600, 1200);
#endif
    }


    //Base Image
    fptr = fopen((const char*)input_name, "wb");
    if( fptr == NULL )
    {
        printf("Output MC YUV file open error\n");
        exit(0);
    }
    //fwrite(baseBufAddr, sizeof(MUINT8), MfbllPipeInitInfo.input_width*MfbllPipeInitInfo.input_height*2, fptr);
    fwrite(baseBufAddr, sizeof(MUINT8), 1600*1200*2, fptr);
    fclose(fptr);
    MY_LOGD("Save File %s", input_name);

    //MINT32 frameNumber = pMfllHal->getFrameNumber(MfbllPipeInitInfo.iso)-1;
    pMfllHal->doMemc(0, 0, baseBufAddr, refBufAddr, &badMvCount);

    //MC Image (sharing same address space as frame1)
    fptr = fopen((const char*)out_name, "wb");
    if( fptr == NULL )
    {
        printf("Output MC YUV file open error\n");
        exit(0);
    }
    //fwrite(baseBufAddr, sizeof(MUINT8), MfbllPipeInitInfo.input_width*MfbllPipeInitInfo.input_height*2, fptr);
    fwrite(refBufAddr, sizeof(MUINT8), 1600*1200*2, fptr);
    fclose(fptr);
    MY_LOGD("Save File %s", out_name);

    delete baseBufAddr;
    delete refBufAddr;

    //
    delete WorkingBuffer;   //shared by MEMC & SwMixer

    return 0;
}

