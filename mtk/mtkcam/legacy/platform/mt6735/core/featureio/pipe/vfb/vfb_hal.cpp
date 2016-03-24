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
#define LOG_TAG "mhalVFB"

#include <Log.h>
#include "MTKVideoFaceBeauty.h"
#include <common/faces.h>
#include "vfb_hal.h"
#include "camera_custom_vfb.h"

#define MY_LOGD(fmt, arg...)    CAM_LOGD(fmt,##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE(fmt,##arg)


/*******************************************************************************
*
********************************************************************************/
static halVFBBase *pHalVFB = NULL;

/*******************************************************************************
*
********************************************************************************/
halVFBBase*
halVFB::
getInstance()
{
    MY_LOGD("[halVFB] getInstance \n");
    if (pHalVFB == NULL) {
        pHalVFB = new halVFB();
    }
    return pHalVFB;
}

/*******************************************************************************
*
********************************************************************************/
void
halVFB::
destroyInstance()
{
    if (pHalVFB) {
        delete pHalVFB;
    }
    pHalVFB = NULL;
}

/*******************************************************************************
*
********************************************************************************/
halVFB::halVFB()
{
    mpMTKVideoFaceBeautyObj = NULL;

    if (mpMTKVideoFaceBeautyObj) {
        MY_LOGE("[halVFB] Already created \n");
    }

    /*  Create MTKVideoFaceBeauty Interface  */
    mpMTKVideoFaceBeautyObj = MTKVideoFaceBeauty::createInstance();
    if(!mpMTKVideoFaceBeautyObj)
    {
        MY_LOGE("[halVFB] createInstance fail\n");
    }
    else
        MY_LOGD("[halVFB] createInstance done\n");

    VFB_Customize_PARA_STRUCT custData;
    get_VFB_CustomizeData(&custData);
    mTemporalSmLevel = custData.temporal_smooth_level;
    mTimeout = custData.lost_face_timeout;
    mSortFaceWei = custData.sort_face_weight;
    mRuddyLevel = custData.ruddy_level;
    mWorkingBufferSize = 0;
}

/*******************************************************************************
*
********************************************************************************/
halVFB::~halVFB()
{
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
halVFB::mHalVFBInit(MTKPipeVfbEnvInfo VfbEnvInfo)
{
    MY_LOGD("[mHalVFBInit] + \n");

    MRESULT Retcode = S_VIDEO_FACEBEAUTY_OK;
    MTKVideoFaceBeautyEnvInfoStruct vFaceBeautyEnvInfo;
    vFaceBeautyEnvInfo.Features = VfbEnvInfo.Features;
    vFaceBeautyEnvInfo.thread_policy = VfbEnvInfo.ThreadPolicy;
    vFaceBeautyEnvInfo.thread_priority = VfbEnvInfo.ThreadPriority;
#define FORMAT_CONVERSION(fmt) \
    (((fmt) == NSCam::eImgFmt_YV12)? MTK_VIDEO_FACE_BEAUTY_IMAGE_YUV420: \
    (((fmt) == NSCam::eImgFmt_YV16)? MTK_VIDEO_FACE_BEAUTY_IMAGE_YUV422: ((MTK_VIDEO_FACE_BEAUTY_IMAGE_FORMAT_ENUM)(MUINT32) -1)))
    vFaceBeautyEnvInfo.DsImgFormat = FORMAT_CONVERSION(VfbEnvInfo.DsImgFormat);
    //vFaceBeautyEnvInfo.SrcImgFormat = FORMAT_CONVERSION(VfbEnvInfo.SrcImgFormat);
    vFaceBeautyEnvInfo.DsImgHeight = VfbEnvInfo.DsImgHeight;
    vFaceBeautyEnvInfo.DsImgWidth = VfbEnvInfo.DsImgWidth;
    vFaceBeautyEnvInfo.FDHeight = VfbEnvInfo.DsImgHeight;
    vFaceBeautyEnvInfo.FDWidth = VfbEnvInfo.DsImgWidth;
    vFaceBeautyEnvInfo.SrcImgHeight = VfbEnvInfo.SrcImgHeight;
    vFaceBeautyEnvInfo.SrcImgWidth = VfbEnvInfo.SrcImgWidth;
    vFaceBeautyEnvInfo.senario = (VfbEnvInfo.RecordingHint)? MTK_VIDEO_FACE_BEAUTY_RECORD: MTK_VIDEO_FACE_BEAUTY_PREVIEW;
    Retcode = mpMTKVideoFaceBeautyObj->VideoFaceBeautyInit(&vFaceBeautyEnvInfo, 0);
    if (Retcode != S_VIDEO_FACEBEAUTY_OK)
    {
        MY_LOGE("[mHalVFBInit] VideoFaceBeautyInit failed (%d)\n", Retcode);
        return false;
    }

    MY_LOGD("[mHalVFBInit] - \n");

    return true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halVFB::mHalVFBUninit()
{
    MY_LOGD("[mHalVFBUninit] +\n");

    if (mpMTKVideoFaceBeautyObj) {
        mpMTKVideoFaceBeautyObj->VideoFaceBeautyReset();
        mpMTKVideoFaceBeautyObj->destroyInstance(mpMTKVideoFaceBeautyObj);
    }
    mpMTKVideoFaceBeautyObj = NULL;

    MY_LOGD("[mHalVFBUninit] -\n");

    return S_VIDEO_FACEBEAUTY_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halVFB::mHalVFBGetWorkingBufSize()
{
    MY_LOGD("[mhalVFBGetWorkingBuffSize] +\n");

    MRESULT Retcode = S_VIDEO_FACEBEAUTY_OK;
    MTKVideoFaceBeautyWorkBufInfoStruct vFaceBeautyWorkBufInfo;
    Retcode = mpMTKVideoFaceBeautyObj->VideoFaceBeautyFeatureCtrl(MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_WORK_BUF_INFO, NULL, &vFaceBeautyWorkBufInfo);
    if (Retcode != S_VIDEO_FACEBEAUTY_OK)
    {
        MY_LOGE("[mhalVFBGetWorkingBuffSize] MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_WORK_BUF_INFO failed (%d)\n", Retcode);
        return false;
    }

    return mWorkingBufferSize = vFaceBeautyWorkBufInfo.ext_mem_size;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
halVFB::mHalVFBSetWorkingBuf(void* WorkingBufAddr, MINT32 WorkingBufSize)
{
    MY_LOGD("[mHalVFBSetWorkingBuf] +\n");

    MRESULT Retcode = S_VIDEO_FACEBEAUTY_OK;
    MTKVideoFaceBeautyWorkBufInfoStruct vFaceBeautyWorkBufInfo;
    vFaceBeautyWorkBufInfo.ext_mem_start_addr = WorkingBufAddr;
    vFaceBeautyWorkBufInfo.ext_mem_size = WorkingBufSize;
    Retcode = mpMTKVideoFaceBeautyObj->VideoFaceBeautyFeatureCtrl(MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_WORK_BUF_INFO, &vFaceBeautyWorkBufInfo, NULL);
    if (Retcode != S_VIDEO_FACEBEAUTY_OK)
    {
        MY_LOGE("[mHalVFBSetWorkingBuf] MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_WORK_BUF_INFO failed (%d)\n", Retcode);
        return false;
    }

    MY_LOGD("[mhalVFBGetWorkingBuffSize] -\n");
    return true;
}

/////////////////////////////////////////////////////////////////////////
//
// mHalVFBProc () -
//! \brief Vfb process
//
/////////////////////////////////////////////////////////////////////////
MBOOL
halVFB::mHalVFBProc(MTKPipeVfbProcInfo VfbProcInfo, MTKPipeVfbResultInfo* VfbResultInfo)
{
    MRESULT Retcode = S_VIDEO_FACEBEAUTY_OK;
    MY_LOGD("[mHalVFBProc] + \n");

    /* Set Tuning Parameters (every frame) */
    MTKVideoFaceBeautyProcInfoStruct vFaceBeautyProcInfo;
    MTKVideoFaceBeautyTuningParaStruct vFaceBeautyTuningInfo;
    vFaceBeautyProcInfo.pTuningPara = &vFaceBeautyTuningInfo;
    vFaceBeautyProcInfo.pTuningPara->SmoothLevel = VfbProcInfo.SmoothLevel + 13;
    vFaceBeautyProcInfo.pTuningPara->BrightLevel = VfbProcInfo.SkinColor + 13;
    vFaceBeautyProcInfo.pTuningPara->RuddyLevel = mRuddyLevel;
    vFaceBeautyProcInfo.pTuningPara->EnlargeEyeLevel = VfbProcInfo.EnlargeEyeLevel + 13;
    vFaceBeautyProcInfo.pTuningPara->SlimFaceLevel = VfbProcInfo.SlimFaceLevel + 13;
    vFaceBeautyProcInfo.pTuningPara->GlobalFaceNum = GLOBAL_FACE_NUM;
    vFaceBeautyProcInfo.pTuningPara->MinFaceRatio = MIN_FACE_RATIO;
    vFaceBeautyProcInfo.pTuningPara->ExtremeBeauty = VfbProcInfo.ExtremeBeauty;
    vFaceBeautyProcInfo.pTuningPara->SortFaceWei = mSortFaceWei;
    vFaceBeautyProcInfo.pTuningPara->TemporalSmLevel = mTemporalSmLevel;
    vFaceBeautyProcInfo.pTuningPara->Timeout = mTimeout;
    vFaceBeautyProcInfo.pTuningPara->TouchX = VfbProcInfo.TouchX;
    vFaceBeautyProcInfo.pTuningPara->TouchY = VfbProcInfo.TouchY;
    MY_LOGD("[mHalVFBProc] tuning %s mode smooth(%d) bright(%d) ruddy(%d) eye(%d) slim(%d) touch(%d,%d)\n",
            (vFaceBeautyProcInfo.pTuningPara->ExtremeBeauty == true)? "extreme": "normal",
            vFaceBeautyProcInfo.pTuningPara->SmoothLevel,
            vFaceBeautyProcInfo.pTuningPara->BrightLevel,
            vFaceBeautyProcInfo.pTuningPara->RuddyLevel,
            vFaceBeautyProcInfo.pTuningPara->EnlargeEyeLevel,
            vFaceBeautyProcInfo.pTuningPara->SlimFaceLevel,
            vFaceBeautyProcInfo.pTuningPara->TouchX,
            vFaceBeautyProcInfo.pTuningPara->TouchY);
    Retcode = mpMTKVideoFaceBeautyObj->VideoFaceBeautyFeatureCtrl(MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_TUNING_PARA, vFaceBeautyProcInfo.pTuningPara, NULL);
    if (Retcode != S_VIDEO_FACEBEAUTY_OK)
    {
        MY_LOGE("[mHalVFBProc] MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_TUNING_PARA failed (%d)\n", Retcode);
        return false;
    }

    //vFaceBeautyProcInfo.SrcImgAddr = gImageBuffer;
    vFaceBeautyProcInfo.DsImgAddr = (MUINT8*) VfbProcInfo.DsImgAddr;
    vFaceBeautyProcInfo.AlphaCL = (MUINT8*) VfbProcInfo.AlphaCL;
    vFaceBeautyProcInfo.AlphaNR = (MUINT8*) VfbProcInfo.AlphaNR;
    vFaceBeautyProcInfo.PCATable = VfbProcInfo.PCATable;
    vFaceBeautyProcInfo.WarpedDsImage = (MUINT8*) VfbProcInfo.WarpedDsImage;
    //vFaceBeautyProcInfo.WarpedFullImage = (MUINT8*) VfbProcInfo.WarpedFullImage;
    vFaceBeautyProcInfo.WarpMapX = VfbProcInfo.WarpMapX;
    vFaceBeautyProcInfo.WarpMapY = VfbProcInfo.WarpMapY;
    MtkCameraFaceMetadata *pFaceMetadata = (MtkCameraFaceMetadata*) VfbProcInfo.FaceMetadata;
    for(int i=0;i<pFaceMetadata->number_of_faces;i++)
    {
        vFaceBeautyProcInfo.FDLeftTopPointX1[i] = pFaceMetadata->faces[i].rect[0];
        vFaceBeautyProcInfo.FDLeftTopPointY1[i] = pFaceMetadata->faces[i].rect[1];
        vFaceBeautyProcInfo.FDBoxSize[i] = (pFaceMetadata->faces[i].rect[2]-pFaceMetadata->faces[i].rect[0]);
        vFaceBeautyProcInfo.FDPose[i] = pFaceMetadata->posInfo[i].rip_dir;
    }
    vFaceBeautyProcInfo.FaceCount = pFaceMetadata->number_of_faces;
    vFaceBeautyProcInfo.ClipWidth = VfbProcInfo.ClipWidth;
    vFaceBeautyProcInfo.ClipHeight = VfbProcInfo.ClipHeight;
    vFaceBeautyProcInfo.DsClipWidth = VfbProcInfo.DsClipWidth;
    vFaceBeautyProcInfo.DsClipHeight = VfbProcInfo.DsClipHeight;
    vFaceBeautyProcInfo.SrcImgWidth = VfbProcInfo.SrcImgWidth;
    vFaceBeautyProcInfo.SrcImgHeight = VfbProcInfo.SrcImgHeight;
    vFaceBeautyProcInfo.DsImgWidth = VfbProcInfo.DsImgWidth;
    vFaceBeautyProcInfo.DsImgHeight = VfbProcInfo.DsImgHeight;
    vFaceBeautyProcInfo.FDWidth = VfbProcInfo.FDWidth;
    vFaceBeautyProcInfo.FDHeight = VfbProcInfo.FDHeight;
    vFaceBeautyProcInfo.PreWarpMapSize[0] = VfbProcInfo.PreWarpMapSize[0];
    vFaceBeautyProcInfo.PreWarpMapSize[1] = VfbProcInfo.PreWarpMapSize[1];
    vFaceBeautyProcInfo.PreWarpMapX = VfbProcInfo.PreWarpMapX;
    vFaceBeautyProcInfo.PreWarpMapY = VfbProcInfo.PreWarpMapY;
    Retcode = mpMTKVideoFaceBeautyObj->VideoFaceBeautyFeatureCtrl(MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_PROC_INFO, &vFaceBeautyProcInfo, NULL);
    if (Retcode != S_VIDEO_FACEBEAUTY_OK)
    {
        MY_LOGE("[mHalVFBProc] MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_PROC_INFO failed (%d)\n", Retcode);
        return false;
    }

    Retcode = mpMTKVideoFaceBeautyObj->VideoFaceBeautyMain();
    if (Retcode != S_VIDEO_FACEBEAUTY_OK)
    {
        MY_LOGE("[mHalVFBProc] VideoFaceBeautyMain failed (%d)\n", Retcode);
        return false;
    }

    MTKVideoFaceBeautyResultInfoStruct vFaceBeautyResultInfo;
    Retcode = mpMTKVideoFaceBeautyObj->VideoFaceBeautyFeatureCtrl(MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_RESULT, NULL, &vFaceBeautyResultInfo);
    if (Retcode != S_VIDEO_FACEBEAUTY_OK)
    {
        MY_LOGE("[mHalVFBProc] MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_RESULT failed (%d)\n", Retcode);
        return false;
    }

    //VfbResultInfo->WarpedFullImage = vFaceBeautyResultInfo.WarpedFullImage;
    VfbResultInfo->WarpedDsImage = vFaceBeautyResultInfo.WarpedDsImage;
    VfbResultInfo->AlphaCL = vFaceBeautyResultInfo.AlphaCL;
    VfbResultInfo->AlphaNR = vFaceBeautyResultInfo.AlphaNR;
    VfbResultInfo->PCATable = vFaceBeautyResultInfo.PCATable;
    for(int i=0;i<vFaceBeautyProcInfo.FaceCount;i++)
    {
        VfbResultInfo->FacePos[i][0] = vFaceBeautyResultInfo.fb_pos[i][0];
        VfbResultInfo->FacePos[i][1] = vFaceBeautyResultInfo.fb_pos[i][1];
    }
    VfbResultInfo->WarpMapX = vFaceBeautyResultInfo.WarpMapX;
    VfbResultInfo->WarpMapY = vFaceBeautyResultInfo.WarpMapY;

    MY_LOGD("[mHalVFBProc] - Retcode %d",Retcode);
    return true;
}

halVFBTuning& halVFBTuning::getInstance() {
    static halVFBTuning singleton;
    return singleton;
}

//#define _DEBUG_DUMP
#ifdef _DEBUG_DUMP
#include <fcntl.h>
#include <sys/stat.h>

static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s]\n", size, fname);
    ::close(fd);
    return true;
}
#endif

void halVFBTuning::mHalVFBTuningSetPCA(MINT32* PCAAdr) {
    MY_LOGD("[mHalVFBTuningSetPCA] Set vFB PCA@0x%p", PCAAdr);
    for (int i=0;i<360;i++)
    {
        mPCALut[4*i+2] = (MINT8) PCAAdr[i];
        mPCALut[4*i+1] = (MINT8) PCAAdr[360+i];
        mPCALut[4*i+0] = (MINT8) PCAAdr[720+i];
    }
#ifdef _DEBUG_DUMP
    static int count = 0;
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/pca_algo_%04d.bin", count);
    saveBufToFile(szFileName, (MUINT8*)PCAAdr, 360 * 3 * 4);
    ::sprintf(szFileName, "/sdcard/pca_isp_%04d.bin", count++);
    saveBufToFile(szFileName, (MUINT8*)mPCALut, 360 * 4);
#endif
}
MUINT32* halVFBTuning::mHalVFBTuningGetPCA() {
    MY_LOGD("[mHalVFBTuningGetPCA] Get vFB PCA@0x%p", mPCALut);
    return (MUINT32*) mPCALut;
}
void halVFBTuning::mHalVFBTuningSetLCE(MINT32* LCEAdr) {
    MY_LOGD("[mHalVFBTuningSetPCA] Set vFB LCE@0x%p", LCEAdr);
    mLCEAdr = LCEAdr;
}
MINT32* halVFBTuning::mHalVFBTuningGetLCE() {
    MY_LOGD("[mHalVFBTuningGetPCA] Get vFB LCE@0x%p", mLCEAdr);
    return mLCEAdr;
}

#if 0
/*******************************************************************************
*
********************************************************************************/
MINT32
halVFB::mHalSTEP1(void* ImgSrcAddr, void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP1] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;

    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP1] cancel \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }

    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP1;
    FaceBeautyProcInfo.Step1SrcImgAddr = (MUINT8*)ImgSrcAddr;
    MY_LOGD("[mHalSTEP1] SrcImgAddr 0x%x \n",(MUINT32)ImgSrcAddr);

    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyMain();
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);

    MY_LOGD("[mHalSTEP1] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halVFB::mHalSTEP3(void* ImgSrcAddr, void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP3] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;

    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP3] cancel \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }

    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP3;
    FaceBeautyProcInfo.Step2SrcImgAddr = (MUINT8*)ImgSrcAddr;
    MY_LOGD("[mHalSTEP3] SrcImgAddr 0x%x \n",(MUINT32)ImgSrcAddr);

    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyMain();
    MY_LOGD("[mHalSTEP3] - ret %d\n",Retcode);
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);

    MY_LOGD("[mHalSTEP3] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halVFB::mHalSTEP4(void* ImgSrcAddr,void* BlurResultAdr,void* AplhaMapBuffer,void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP4] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;

      if(CANCEL)
    {
        MY_LOGD("[mHalSTEP4] cancel \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }

    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP4;
    FaceBeautyProcInfo.SrcImgAddr = (MUINT8*)ImgSrcAddr;
    FaceBeautyProcInfo.Step4SrcImgAddr_1 = (MUINT8*)BlurResultAdr;
    FaceBeautyProcInfo.Step4SrcImgAddr_2 = (MUINT8*)AplhaMapBuffer;

    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyMain();
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);

    MY_LOGD("[mHalSTEP4] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halVFB::mHalSTEP5(void* ImgSrcAddr,void* AplhaMapColorBuffer,void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP5] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;

    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP5] cancel \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }

      MTKPipeFaceBeautyResultInfo* VfbResultInfo = (MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP5;
    FaceBeautyProcInfo.Step5SrcImgAddr = (MUINT8*)AplhaMapColorBuffer;

    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyMain();
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);

    MY_LOGD("[mHalSTEP5] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halVFB::mHalSTEP6(void* ImgSrcAddr,void* WarpWorkBufAdr,void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP6] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;

    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP6] cancel \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }

      MTKPipeFaceBeautyResultInfo* VfbResultInfo = (MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP6;
    FaceBeautyProcInfo.Step6TempAddr = (MUINT8*)WarpWorkBufAdr;

    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyMain();
    Retcode = mpMTKVideoFaceBeautyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);

    MY_LOGD("[mHalSTEP6] - ret %d\n",Retcode);
    return Retcode;
}
#endif
