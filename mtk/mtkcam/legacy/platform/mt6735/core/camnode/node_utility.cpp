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
#define LOG_TAG "CamNode/utilty"
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/common.h>
//
#include <utils/StrongPointer.h>
using namespace android;
//
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe;
//
#include <mtkcam/camnode/IspSyncControl.h>
//
#include <mtkcam/camnode/node_utility.h>
#include <mtkcam/algorithm/libnr/MTKNR.h>
//
#include <camera_custom_capture_nr.h>
// custom tuning
#include <mtkcam/hal/IHalSensor.h>
#include <camera_custom_nvram.h>
#include <mtkcam/featureio/nvbuf_util.h>
//
#define PASS2_TIMEOUT       ((MINT64)5000000000) //ns

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

MBOOL doHwNR(MUINT32 const openId, IImageBuffer* in, IImageBuffer* out, EIspProfile_T const profile)
{
    MY_LOGD("idx %d, in %p, out %p, profile %d", openId, in, out, profile);

    MBOOL                   ret = MFALSE;
    IHal3A*                 pHal3A = NULL;
    IHalPostProcPipe*       pPostProcPipe = NULL;
    MUINT32                 magic = SetCap(MAGIC_NUM_INVALID);

    // check params
    if( in->getImgFormat() != eImgFmt_YUY2 ){
        MY_LOGE("not supported src fmt %d", in->getImgFormat() );
        goto lbExit;
    }

    pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, openId, LOG_TAG);
    if(pHal3A == NULL) {
        MY_LOGE("IHal3A:createInstance fail");
        goto lbExit;
    }

    {
        ParamIspProfile_T _3A_profile( profile, magic, MTRUE, ParamIspProfile_T::EParamValidate_P2Only);
        MY_LOGD("set ispProfile %d, # 0x%x", profile, magic);
        pHal3A->setIspProfile(_3A_profile);
    }

    if( profile == EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
        profile == EIspProfile_VSS_Capture_MultiPass_ANR_2 )
    {
        pPostProcPipe = INormalStream::createInstance(LOG_TAG, ENormalStreamTag_Vss, openId);
    } else {
        pPostProcPipe = INormalStream::createInstance(LOG_TAG, ENormalStreamTag_Cap, openId);
    }

    if(pPostProcPipe == NULL) {
        MY_LOGE("INormalStream::createInstance fail");
        goto lbExit;
    }

    if( !pPostProcPipe->init() ) {
        MY_LOGE("INormalStream::init fail");
        goto lbExit;
    }

    {
        QParams enqueParams;
        enqueParams.mvIn.reserve(1);
        enqueParams.mvOut.reserve(1);
        enqueParams.mFrameNo = SetCap(MAGIC_NUM_INVALID); // use un-used magic #

        {   // input
            Input src;
            src.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
            src.mBuffer = in;
            enqueParams.mvIn.push_back(src);
        }

        {   // output
            Output dst;
            dst.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);
            dst.mBuffer = out;
            enqueParams.mvOut.push_back(dst);
        }

        {   // crop
            MCrpRsInfo crop1;
            crop1.mGroupID    = 1;
            crop1.mCropRect.s = in->getImgSize();
            crop1.mResizeDst  = in->getImgSize();
            //
            MCrpRsInfo crop2;
            crop2.mGroupID    = 2;
            crop2.mCropRect.s = out->getImgSize();

            enqueParams.mvCropRsInfo.push_back( crop1 );
            enqueParams.mvCropRsInfo.push_back( crop2 );
        }

        if( !pPostProcPipe->enque(enqueParams) )
        {
            MY_LOGE("enque pass2 failed");
            goto lbExit;
        }
    }

    {
        QParams dequeParams;
        if( !pPostProcPipe->deque(dequeParams, PASS2_TIMEOUT) )
        {
            MY_LOGE("deque pass2: deque fail");
            goto lbExit;
        }
    }

    if( !pPostProcPipe->uninit() ){
        MY_LOGE("INormalStream::uninit fail");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    if( pHal3A )
        pHal3A->destroyInstance(LOG_TAG);

    if( pPostProcPipe )
        pPostProcPipe->destroyInstance(LOG_TAG);

    return ret;
}


MBOOL doTwoPassHwNR( MUINT32 openId, IImageBuffer* buf, EIspProfile_T const profile0, EIspProfile_T const profile1)
{
    MY_LOGD("idx %d, buf %p, profile %d/%d", openId, buf, profile0, profile1);
    MBOOL                     ret = MFALSE;
    sp<IImageBuffer>          pInternalBuf;

    // check params
    if( buf->getImgFormat() != eImgFmt_YUY2 ){
        MY_LOGE("not supported src fmt %d", buf->getImgFormat() );
        goto lbExit;
    }

    {
        MY_LOGD("allocate memory");
        // allocate internal memory
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();

        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        MUINT32 bufStrideInByte[3] = { buf->getBufStridesInBytes(0), 0, 0 };

        IImageBufferAllocator::ImgParam imgParam(
                buf->getImgFormat(),
                buf->getImgSize(),
                bufStrideInByte,
                bufBoundaryInBytes,
                buf->getPlaneCount()
                );

        pInternalBuf = allocator->alloc_ion(LOG_TAG, imgParam);

        if ( !pInternalBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
        {
            MY_LOGE("lock Buffer failed\n");
            goto lbExit;
        }

        if ( !pInternalBuf->flushCache( eCACHECTRL_INVALID ) )
        {
            MY_LOGE("flushCache failed\n");
            goto lbExit;
        }

    }

    // 1st run
    if( !doHwNR(openId, buf, pInternalBuf.get(), profile0) ) {
        MY_LOGE("1st run NR failed");
        goto lbExit;
    }

    // 2nd run
    if( !doHwNR(openId, pInternalBuf.get(), buf, profile1) ) {
        MY_LOGE("2nd run NR failed");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    // release internal buf
    if( pInternalBuf.get() ) {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        if( !pInternalBuf->unlockBuf( LOG_TAG ) )
        {
            MY_LOGE("unlock Buffer failed\n");
            ret = MFALSE;
        }
        allocator->free(pInternalBuf.get());
        pInternalBuf = NULL;
    }

    return ret;
}


SwNR::
SwNR(MUINT32 const openId)
    : muOpenId(openId)
    , mpMTKNR(NULL)
    , mpNRTuningInfo(NULL)
    , muWorkingBufSize(0)
    , mpWorkingBuf(NULL)
{
    mpNRInitInfo   = (NRInitInfo*)malloc(sizeof(NRInitInfo));
    mpNRImageInfo  = (NRImageInfo*)malloc(sizeof(NRImageInfo));
    mpNRResultInfo = (NRResultInfo*)malloc(sizeof(NRResultInfo));
    // initial
    mpNRInitInfo->WorkingBuffAddr = 0;
    mpNRInitInfo->WorkingBuffSize = 0;
    mpNRInitInfo->CoreNumber = 4;
    mpNRInitInfo->NumOfExecution = 1;
    mpNRInitInfo->pTuningInfo = NULL;
}


SwNR::
~SwNR()
{
#define MY_FREE( ptr ) \
    if( ptr )          \
    {                  \
        free(ptr);     \
        ptr = NULL;    \
    }

    MY_FREE(mpNRInitInfo);
    MY_FREE(mpNRTuningInfo);
    MY_FREE(mpNRImageInfo);
    MY_FREE(mpNRResultInfo);
    MY_FREE(mpWorkingBuf);

#undef MY_FREE
}


MBOOL
SwNR::
loadDefaultTuning()
{
    if( sizeof(SWNRTuningInfo) != sizeof(NRTuningInfo) )
    {
        MY_LOGE("custom file is not sync with algo.");
        return MFALSE;
    }

    SWNRTuningInfo defaultTuning;
    if( !get_swnr_default_tuning(&defaultTuning) )
    {
        MY_LOGE("can't get default tuning");
        return MFALSE;
    }
    memcpy(mpNRTuningInfo, &defaultTuning, sizeof(NRTuningInfo) );

    return MTRUE;
}


MBOOL
SwNR::
getTuningInfo()
{
    MBOOL ret = MFALSE;

    if( mpNRTuningInfo )
        return MTRUE;

    mpNRTuningInfo = (NRTuningInfo*)malloc(sizeof(NRTuningInfo));

    // load default tuning first
    if( !loadDefaultTuning() )
        goto lbExit;

    // partially overwrite
    if( !updateFromNvram() )
        goto lbExit;

    ret = MTRUE;
lbExit:
    mpNRInitInfo->pTuningInfo = mpNRTuningInfo;
    return ret;
}


MBOOL
SwNR::
updateFromNvram()
{
    // load some setting from nvram
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT* pNvram;

    MUINT sensorDev = SENSOR_DEV_NONE;
    {
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        sensorDev = pHalSensorList->querySensorDevIdx(muOpenId);
    }

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)pNvram);

    if( err != 0 )
    {
        MY_LOGE("getBufAndRead fail, err=%d", err);
        return MFALSE;
    }

    // update
    mpNRTuningInfo->ITUNE_ANR_PTU_STD      = pNvram->swnr.ITUNE_ANR_PTU_STD;
    mpNRTuningInfo->ITUNE_ANR_PTV_STD      = pNvram->swnr.ITUNE_ANR_PTV_STD;
    mpNRTuningInfo->ANR_ACT_TH_C           = pNvram->swnr.ANR_ACT_TH_C;
    mpNRTuningInfo->ANR_ACT_BLD_BASE_C     = pNvram->swnr.ANR_ACT_BLD_BASE_C;
    mpNRTuningInfo->ANR_ACT_BLD_TH_C       = pNvram->swnr.ANR_ACT_BLD_TH_C;
    mpNRTuningInfo->ANR_ACT_SLANT_C        = pNvram->swnr.ANR_ACT_SLANT_C;

    return MTRUE;
}


MBOOL
SwNR::
setImageInfo(IImageBuffer* pBuf)
{
    MBOOL ret = MFALSE;
    MTK_NR_IMAGE_FMT_ENUM NRFmt = NR_IMAGE_PACKET_YUY2;
    switch( pBuf->getImgFormat() )
    {
        case eImgFmt_YUY2:
            NRFmt = NR_IMAGE_PACKET_YUY2;
            break;
        case eImgFmt_I420:
            NRFmt = NR_IMAGE_YUV420;
            break;
        default:
            MY_LOGE("not supported format: 0x%x", pBuf->getImgFormat() );
            goto lbExit;
    }

    // set image info
    mpNRImageInfo->Width = pBuf->getImgSize().w;
    mpNRImageInfo->Height = pBuf->getImgSize().h;
    mpNRImageInfo->ImgAddr[0] = pBuf->getBufVA(0);
    if( NRFmt = NR_IMAGE_YUV420 )
    {
        mpNRImageInfo->ImgAddr[1] = pBuf->getBufVA(1);
        mpNRImageInfo->ImgAddr[2] = pBuf->getBufVA(2);
    }
    else
    {
        mpNRImageInfo->ImgAddr[1] = 0;
        mpNRImageInfo->ImgAddr[2] = 0;
    }
    mpNRImageInfo->ImgNum = 1;
    mpNRImageInfo->ImgFmt = NRFmt;

    ret = MTRUE;
lbExit:
    return ret;
}


MBOOL
SwNR::
allocWorkingBuf(MUINT32 size)
{
    MBOOL ret = MFALSE;
    // allocate working buffer
    if( size != muWorkingBufSize )
    {
        if( mpWorkingBuf )
            free(mpWorkingBuf);
        mpWorkingBuf = (MUINT8*) malloc( sizeof(MUINT8) * size );
        if( mpWorkingBuf == NULL )
        {
            MY_LOGE("allocate working buffer failed");
            goto lbExit;
        }
        MY_LOGD("allocate working buffer %p, size %d", mpWorkingBuf, size);
        muWorkingBufSize = size;
    }

    mpNRInitInfo->WorkingBuffAddr = (MUINT32) mpWorkingBuf;
    mpNRInitInfo->WorkingBuffSize = muWorkingBufSize;

    ret = MTRUE;
lbExit:
    return ret;
}


MBOOL
SwNR::
doSwNR(IImageBuffer* pBuf)
{
#define CHECK_NR_RET( NR_ret )                     \
    do{                                            \
        MRESULT nr_ret = (NR_ret);                 \
        if( nr_ret != S_NR_OK ) {                  \
            MY_LOGE("NR with error 0x%x", nr_ret); \
            goto lbExit;                           \
        }                                          \
    }                                              \
    while(0);

    MY_LOGD("idx %d, buf %p", muOpenId, pBuf);
    MBOOL ret = MFALSE;
    MUINT32 workingbufsize = 0;

    mpMTKNR = MTKNR::createInstance(DRV_NR_OBJ_SW);

    // get tuning info
    if( !getTuningInfo() )
        goto lbExit;
    //
    CHECK_NR_RET( mpMTKNR->NRInit((MUINT32*) mpNRInitInfo, 0) );

    if( !setImageInfo(pBuf) )
        goto lbExit;

    CHECK_NR_RET(
            mpMTKNR->NRFeatureCtrl(
                NR_FEATURE_GET_WORKBUF_SIZE,
                (void *)mpNRImageInfo,
                (void *)&workingbufsize)
            );

    if( !allocWorkingBuf(workingbufsize) )
        goto lbExit;

    CHECK_NR_RET(
            mpMTKNR->NRFeatureCtrl(
                NR_FEATURE_SET_WORKBUF_ADDR,
                (void *)&mpNRInitInfo->WorkingBuffAddr,
                NULL)
            );

    // add img
    CHECK_NR_RET(
            mpMTKNR->NRFeatureCtrl(
                NR_FEATURE_ADD_IMG,
                (void *)mpNRImageInfo,
                NULL)
            );

    MY_LOGD("start+");
    // main
    CHECK_NR_RET( mpMTKNR->NRMain() );
    MY_LOGD("start-");

    CHECK_NR_RET(
            mpMTKNR->NRFeatureCtrl(
                NR_FEATURE_GET_RESULT,
                NULL,
                (void *)mpNRResultInfo)
            );

    pBuf->flushCache(eCACHECTRL_FLUSH);

    mpMTKNR->NRReset();

#undef CHECK_NR_RET
    ret = MTRUE;
lbExit:
    if( mpMTKNR ) {
        mpMTKNR->destroyInstance(mpMTKNR);
        mpMTKNR = NULL;
    }

    return ret;
}


MBOOL
SwNR::
dumpParamsToFile(char * const filename)
{
    if( !getTuningInfo() )
        return MFALSE;

    // write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        MY_LOGE("fopen fail: %s", filename);
        return MFALSE;
    }
    else {
#define FWRITE_PARAM( param )                                       \
        do {                                                        \
            fprintf(fp, "%s:%d\n", #param, mpNRTuningInfo->param);  \
        } while(0)
        //fwrite(mpNRTuningInfo, sizeof(NRTuningInfo), 1, fp);
        FWRITE_PARAM(ANR_Y_LUMA_SCALE_RANGE);
        FWRITE_PARAM(ANR_C_CHROMA_SCALE);
        FWRITE_PARAM(ANR_Y_SCALE_CPY0);
        FWRITE_PARAM(ANR_Y_SCALE_CPY1);
        FWRITE_PARAM(ANR_Y_SCALE_CPY2);
        FWRITE_PARAM(ANR_Y_SCALE_CPY3);
        FWRITE_PARAM(ANR_Y_SCALE_CPY4);
        FWRITE_PARAM(ANR_Y_CPX1);
        FWRITE_PARAM(ANR_Y_CPX2);
        FWRITE_PARAM(ANR_Y_CPX3);
        FWRITE_PARAM(ANR_CEN_GAIN_LO_TH);
        FWRITE_PARAM(ANR_CEN_GAIN_HI_TH);
        FWRITE_PARAM(ANR_PTY_GAIN_TH);
        FWRITE_PARAM(ANR_KSIZE_LO_TH);
        FWRITE_PARAM(ANR_KSIZE_HI_TH);
        FWRITE_PARAM(ANR_KSIZE_LO_TH_C);
        FWRITE_PARAM(ANR_KSIZE_HI_TH_C);
        FWRITE_PARAM(ITUNE_ANR_PTY_STD);
        FWRITE_PARAM(ITUNE_ANR_PTU_STD);
        FWRITE_PARAM(ITUNE_ANR_PTV_STD);
        FWRITE_PARAM(ANR_ACT_TH_Y);
        FWRITE_PARAM(ANR_ACT_BLD_BASE_Y);
        FWRITE_PARAM(ANR_ACT_BLD_TH_Y);
        FWRITE_PARAM(ANR_ACT_SLANT_Y);
        FWRITE_PARAM(ANR_ACT_TH_C);
        FWRITE_PARAM(ANR_ACT_BLD_BASE_C);
        FWRITE_PARAM(ANR_ACT_BLD_TH_C);
        FWRITE_PARAM(ANR_ACT_SLANT_C);
        FWRITE_PARAM(RADIUS_H);
        FWRITE_PARAM(RADIUS_V);
        FWRITE_PARAM(RADIUS_H_C);
        FWRITE_PARAM(RADIUS_V_C);
        FWRITE_PARAM(ANR_PTC_HGAIN);
        FWRITE_PARAM(ANR_PTY_HGAIN);
        FWRITE_PARAM(ANR_LPF_HALFKERNEL);
        FWRITE_PARAM(ANR_LPF_HALFKERNEL_C);
        FWRITE_PARAM(ANR_ACT_MODE);
        FWRITE_PARAM(ANR_LCE_SCALE_GAIN);
        FWRITE_PARAM(ANR_LCE_C_GAIN);
        FWRITE_PARAM(ANR_LCE_GAIN0);
        FWRITE_PARAM(ANR_LCE_GAIN1);
        FWRITE_PARAM(ANR_LCE_GAIN2);
        FWRITE_PARAM(ANR_LCE_GAIN3);
        FWRITE_PARAM(ANR_MEDIAN_LOCATION);
        FWRITE_PARAM(ANR_CEN_X);
        FWRITE_PARAM(ANR_CEN_Y);
        FWRITE_PARAM(ANR_R1);
        FWRITE_PARAM(ANR_R2);
        FWRITE_PARAM(ANR_R3);
        FWRITE_PARAM(LUMA_ON_OFF);

        fclose(fp);
    }

    return MTRUE;
}


};  //namespace NSCamNode


