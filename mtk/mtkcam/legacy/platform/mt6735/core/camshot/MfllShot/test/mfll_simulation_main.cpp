#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define MTK_LOG_ENABLE 1
#include <string.h>
#include <ctype.h>

#include <MfllCommon.h>

#if TEST_ALGORITHM
#include "AppMfbll.h"
#include "MTKMfbll.h"
#endif

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
static char input_sf_yuv[200] = PREFIX "dip_bee.yuv";//"Result211_sf.yuv";
static char input_blend_yuv[200] = PREFIX "dip_base.yuv";//"Result211_blend.yuv";
static char input_bld_w[200] = PREFIX "BLD_W2.dat";
static char input_LL_MIXER_M0[32]="0.25";
static char input_LL_MIXER_M1[32]="0.6";
static char input_LL_MIXER_B0[32]="0";
static char input_LL_MIXER_B1[32]="1";
const MUINT32 imgW = 2592;//3204;
const MUINT32 imgH = 1968;//2406;

/*------------------------------------------------------------------------------
    MEMC Parameters
------------------------------------------------------------------------------*/
static char input_width[32]="1296";
static char input_height[32]="984";
char base_name[256]= PREFIX "qdip0.yuv";
char ref_name[256]= PREFIX "qdip1.yuv";
char out_name[256]= PREFIX "test_rec2.yuv";
char out_final[256]= PREFIX "Final_2592x1968.yv16";
static char input_mvx[32]="0";
static char input_mvy[32]="0";
/*------------------------------------------------------------------------------
    Static Variable
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Local Functions
------------------------------------------------------------------------------*/

int main_mfll_simulation(int argc, char* argv[])
{

#if TEST_ALGORITHM
    //Common declaration
    timeval start_time;
    timeval end_time;
    MINT32 ME_time=0,
           MC_time=0;
    MINT32 i,j;
    FILE *fptr;
    MTKMfbll* m_pMfbllDrv = MTKMfbll::createInstance(DRV_MFBLL_OBJ_SW);

    //Mfbll Parameters (MEMC + Mixer)
    MFBLL_INIT_PARAM_STRUCT MfbllInitPara;

    //Memc Paramters declaration
    MUINT32 inMEWidth, inMEHeight, MemcImgsize, MemcImgBufsize,MemcMvBufsize;
    //frame_s frame1;
    MINT16 *mvx, *mvy;
    MINT32 mbNum,WorkBufSize;
    //MINT32 matching_times;
    int ret;
    MFBLL_GET_PROC_INFO_STRUCT Proc1_get_proc_info;
    MFBLL_SET_PROC1_INFO_STRUCT Proc1_set_proc_info;
    MFBLL_PROC1_OUT_STRUCT MfbllMemcResult;

    //Mixer Parameters declaration
    MUINT32 LPF_time, Blending_time;
    MUINT32 ImgSize = ((imgW*imgH+31)>>5)<<5; // 32byte alignment
    MFBLL_SET_PROC2_INFO_STRUCT Proc2_set_proc_info;
    MFBLL_PROC2_OUT_STRUCT Mfbll_result;

    //Init MET
    //MEMC Init Parameters
    MfbllInitPara.Proc1_Init.Proc1_imgW = inMEWidth  = atoi(input_width);
    MfbllInitPara.Proc1_Init.Proc1_imgH = inMEHeight = atoi(input_height);
    MfbllInitPara.Proc1_Init.Proc1_ISO_value = 800;
    MfbllInitPara.Proc1_Init.Proc1_tuning.max_frame= 4;
    MY_LOGD("Proc1_imgW = %d", MfbllInitPara.Proc1_Init.Proc1_imgW);
    MY_LOGD("Proc1_imgH = %d", MfbllInitPara.Proc1_Init.Proc1_imgH);
    MY_LOGD("Proc1_ISO_value = %d", MfbllInitPara.Proc1_Init.Proc1_ISO_value);
    MY_LOGD("Proc1_tuning.max_frame = %d", MfbllInitPara.Proc1_Init.Proc1_tuning.max_frame);

    MemcImgsize = inMEHeight*inMEWidth;
    MemcImgBufsize = ((MemcImgsize+31)>>5)<<5;// 32byte alignment
    mbNum = (inMEWidth*inMEHeight) >> 8;
    //matching_times = 122400 - mbNum * BLOCK_MATCHING_RESERVED_PER_MB_LOCAL;

    //Mixer Init Parameters
    MfbllInitPara.Proc2_Init.Proc2_imgH = imgH;
    MfbllInitPara.Proc2_Init.Proc2_imgW = imgW;
    MfbllInitPara.Proc2_Init.Proc2_tuning.Level = 0;
    MY_LOGD("Proc2_imgH = %d", MfbllInitPara.Proc2_Init.Proc2_imgH);
    MY_LOGD("Proc2_imgW = %d", MfbllInitPara.Proc2_Init.Proc2_imgW);
    MY_LOGD("Proc2_tuning.Level = %d", MfbllInitPara.Proc2_Init.Proc2_tuning.Level);
    if(m_pMfbllDrv==NULL)
    {
        printf("\nMfbll get instance error\n");
        return -1;
    }


    //Initial Mfbll
    m_pMfbllDrv->MfbllInit(&MfbllInitPara,&ret);


    /*--------------------------------------Img Padding--------------------------------------*/

    /*-------------------------------------------Memc Start-----------------------------------*/
    //MEMC Image/Working Buffer Allocation
    m_pMfbllDrv->MfbllFeatureCtrl(MFBLL_FTCTRL_GET_PROC_INFO,&Proc1_get_proc_info,&Proc1_get_proc_info);
    MY_LOGD("Ext_mem_size = %d", Proc1_get_proc_info.Ext_mem_size);
    MY_LOGD("Mfbll_frame_num = %d", Proc1_get_proc_info.Mfbll_frame_num);
    MUINT8 *WorkingBuffer = new MUINT8 [Proc1_get_proc_info.Ext_mem_size];
    MUINT8 *MemcImgBuffer = new MUINT8 [MemcImgBufsize*4];
    if ((!WorkingBuffer)||(!MemcImgBuffer))
    {
        printf("memory init error");
        return -1;
    }
    //Loop for Memc processing time average

    for(j=0; j<1; j++)
    {

        //MEMC Processing Data Iinitialization
        //Set Proc Info
        Proc1_set_proc_info.workbuf_addr = WorkingBuffer;
        Proc1_set_proc_info.buf_size     = Proc1_get_proc_info.Ext_mem_size;
        Proc1_set_proc_info.base         = MemcImgBuffer;
        Proc1_set_proc_info.ref          = Proc1_set_proc_info.base + (MemcImgBufsize<<1);
        Proc1_set_proc_info.Width        = inMEWidth;
        Proc1_set_proc_info.Height       = inMEHeight;
        Proc1_set_proc_info.Proc1_V1 = (MINT32)((atof(input_mvx) >= 0) ? atof(input_mvx)+0.5 : atof(input_mvx)-0.5);
        Proc1_set_proc_info.Proc1_V2 = (MINT32)((atof(input_mvy) >= 0) ? atof(input_mvy)+0.5 : atof(input_mvy)-0.5);

        //Read input base, reference images
        fptr = fopen(base_name, "rb");
        if( fptr == NULL )
        {
            printf("Fail to Open File \n");
            exit(0);
        }
        fread(Proc1_set_proc_info.base, sizeof(MUINT8), 54, fptr);
        fread(Proc1_set_proc_info.base, sizeof(MUINT8), inMEWidth*inMEHeight*2, fptr);
        fclose(fptr);
        fptr = fopen(ref_name, "rb");
        if( fptr == NULL )
        {
            printf("Fail to Open File \n");
            exit(0);
        }
        fread(Proc1_set_proc_info.ref, sizeof(MUINT8), 54, fptr);
        fread(Proc1_set_proc_info.ref, sizeof(MUINT8), inMEWidth*inMEHeight*2, fptr);
        fclose(fptr);


        gettimeofday(&start_time, NULL);

        //MEMC Set Processing Data
        m_pMfbllDrv->MfbllFeatureCtrl(MFBLL_FTCTRL_SET_PROC1_INFO,&Proc1_set_proc_info,&Proc1_set_proc_info);

        gettimeofday(&end_time, NULL);
        ME_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec))/1000;
        printf("   Clip Time : %d msec\n",ME_time/1);

        //MEMC Main process
        m_pMfbllDrv->MfbllMain(MFBLL_PROC1,&ret, &MfbllMemcResult);


        //MC Image (sharing same address space as frame1)
        fptr = fopen((const char*)out_name, "wb");
        if( fptr == NULL )
        {
            printf("Output MC YUV file open error\n");
            exit(0);
        }
        fwrite(MfbllMemcResult.ResultImg, sizeof(MUINT8), MfbllMemcResult.Height*MfbllMemcResult.Width*2, fptr);
        fclose(fptr);

        fptr = fopen("mv_out.dat", "w");
        if( fptr == NULL )
        {
            printf("Dump MV file open error\n");
            exit(0);
        }

        fclose(fptr);

        /*---------------------------------------------------------------------------------------*/
        /*---------------------------------------Do HW MFB Here----------------------------------*/
        /*---------------------------------------------------------------------------------------*/
    }


    /*-------------------------------------------Mixer Start-----------------------------------*/

    Proc2_set_proc_info.workbuf_addr = WorkingBuffer;

    //Image buffer allocation
    MUINT8 *MixerImgBuffer = new MUINT8 [ImgSize*3];
    Proc2_set_proc_info.Proc2_Img1   = MixerImgBuffer;
    Proc2_set_proc_info.Proc2_Img2   = Proc2_set_proc_info.Proc2_Img1 + ImgSize;
    Proc2_set_proc_info.Proc2_ImgH   = MfbllInitPara.Proc2_Init.Proc2_imgH;
    Proc2_set_proc_info.Proc2_ImgW   = MfbllInitPara.Proc2_Init.Proc2_imgW;
    Proc2_set_proc_info.weight_table = Proc2_set_proc_info.Proc2_Img2 + ImgSize;

    //Temp memory allocation just for result check.
    MUINT8 *ChromaBuffer = new MUINT8 [ImgSize];
    MUINT8 *U1Buffer = ChromaBuffer;
    MUINT8 *V1Buffer = U1Buffer + ImgSize/2;
    MUINT16 *Result = new MUINT16 [ImgSize*2];
    MUINT16 *Y_Tmp = Result;
    MUINT16 *U_Tmp = Y_Tmp + ImgSize;
    MUINT16 *V_Tmp = U_Tmp + ImgSize/2;

#if 0
    //Loop for Mixer processing time average
    for(i=0, LPF_time=0, Blending_time=0; i<1; i++)
    {
        //Read single img
        fptr = fopen(input_sf_yuv, "rb");
        if(fptr==NULL)
            printf("Cannot Open file %s\n", input_sf_yuv);
        fread(Proc2_set_proc_info.Proc2_Img1, sizeof(MUINT8), 54, fptr);
        fread(Proc2_set_proc_info.Proc2_Img1, sizeof(MUINT8), imgW*imgH, fptr);

        //Read blend img
        fptr = fopen(input_blend_yuv, "rb");
        if(fptr==NULL)
            printf("Cannot Open file %s\n", input_blend_yuv);
        fread(Proc2_set_proc_info.Proc2_Img2, sizeof(MUINT8), 54, fptr);
        fread(Proc2_set_proc_info.Proc2_Img2, sizeof(MUINT8), imgW*imgH, fptr);
        fread(U1Buffer, sizeof(MUINT8), (imgW*imgH)/2, fptr);
        fread(V1Buffer, sizeof(MUINT8), (imgW*imgH)/2, fptr);
        //Read W map
        fptr = fopen( input_bld_w, "rb");
        if(fptr==NULL)
            printf("Cannot Open file %s\n", input_bld_w);
        fread(Proc2_set_proc_info.weight_table, sizeof(MUINT8), imgW * imgH, fptr);

        //Mixer Set Processing Data
        m_pMfbllDrv->MfbllFeatureCtrl(MFBLL_FTCTRL_SET_PROC2_INFO,&Proc2_set_proc_info,&Proc2_set_proc_info);

        //Main Function : W map LPF & Blending
        gettimeofday(&start_time, NULL);
        m_pMfbllDrv->MfbllMain(MFBLL_PROC2,&ret, &Mfbll_result);

        gettimeofday(&end_time, NULL);
        LPF_time += ((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec))/1000;

        //Write final result.
        fptr = fopen(out_final, "wb");
        fwrite(Mfbll_result.img_mfbll_result, sizeof(MUINT8), imgW * imgH, fptr);
        fwrite(U1Buffer, sizeof(MUINT8), imgW * imgH/2, fptr);
        fwrite(V1Buffer, sizeof(MUINT8), imgW * imgH/2, fptr);
        fclose(fptr);
    }
#endif


    printf("Mixer Time : %d msec\n",(LPF_time/1));

    delete [] WorkingBuffer;
    delete [] MemcImgBuffer;
    delete [] MixerImgBuffer;
    delete [] ChromaBuffer;
    delete [] Result;
#endif  //TEST_ALGORITHM

    return 0;
}
