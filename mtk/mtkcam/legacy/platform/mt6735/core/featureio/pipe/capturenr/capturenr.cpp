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
#define LOG_TAG "capturenr"
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
#include <mtkcam/featureio/capturenr.h>
#include <mtkcam/algorithm/libnr/MTKNR.h>
//
#include <utils/Mutex.h>
#include <list>
#include <math.h>

#include <sys/mman.h>
//
#include <camera_custom_capture_nr.h>
// custom tuning
#include <mtkcam/hal/IHalSensor.h>
#include <camera_custom_nvram.h>
#include <mtkcam/featureio/nvbuf_util.h>
//
#include <aaa/aaa_types.h>
#include <aaa/aaa_log.h>
#include <isp_tuning/isp_tuning.h>
#include <isp_tuning/isp_tuning_cam_info.h>
#include <isp_tuning/isp_tuning_idx.h>
#include <isp_tuning/isp_tuning_custom.h>
#include <isp_tuning/isp_tuning_custom_swnr.h>
#include <mtkcam/algorithm/lib3a/isp_interpolation.h>

//
#include <cutils/properties.h>
//
#define PASS2_TIMEOUT       ((MINT64)5000000000) //ns
#define MAGIC_NUM           (0x80000000|0x3FFFFFFF)

/*******************************************************************************
*
********************************************************************************/
static Mutex                    g_SwParamlock;
static list<SwNRParam*>         g_SwParamlist;
/*******************************************************************************
*
********************************************************************************/

MBOOL doHwNR(
        MUINT32 const openId,
        IImageBuffer* in,
        IImageBuffer* out0,
        IImageBuffer* out1,
        MRect const crop,
        MUINT32 const transform_0,
        NS3A::EIspProfile_T const profile
        )
{
    MY_LOGD("idx %d, in %p, out %p, %p, profile %d", openId, in, out0, out1, profile);
    if( out0 )
    {
        MY_LOGD("out0: %dx%d fmt 0x%x trans %d",
                out0->getImgSize().w,
                out0->getImgSize().h,
                out0->getImgFormat(),
                transform_0);
    }
    if( out1 )
    {
        MY_LOGD("out1: %dx%d fmt 0x%x",
                out1->getImgSize().w,
                out1->getImgSize().h,
                out1->getImgFormat() );
    }

    MBOOL                   ret = MFALSE;
    IHal3A*                 pHal3A = NULL;
    IHalPostProcPipe*       pPostProcPipe = NULL;
    MUINT32 const           magic = MAGIC_NUM;

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

    if( profile == NSIspTuning::EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
        profile == NSIspTuning::EIspProfile_VSS_Capture_MultiPass_ANR_2 ||
        profile == NSIspTuning::EIspProfile_VSS_MFB_MultiPass_ANR_1 ||
        profile == NSIspTuning::EIspProfile_VSS_MFB_MultiPass_ANR_2 )
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
        enqueParams.mFrameNo = magic; // use un-used magic #

        {   // input
            Input src;
            src.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
            src.mBuffer = in;
            enqueParams.mvIn.push_back(src);
        }

        if( out0 )
        {   // output
            Output dst;
            dst.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WROTO, 1);
            dst.mPortID.capbility   = NSIoPipe::EPortCapbility_Cap;
            //
            dst.mBuffer = out0;
            dst.mTransform  = transform_0;
            //
            enqueParams.mvOut.push_back(dst);
        }

        if( out1 )
        {   // output
            Output dst;
            dst.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);
            if( out1->getImgFormat() != eImgFmt_YUY2 )
            {
                dst.mPortID.capbility   = NSIoPipe::EPortCapbility_Disp;
            }
            else
            {   // workaround: Disp will enable tdsharp. However, thumbnail is too small
                dst.mPortID.capbility   = NSIoPipe::EPortCapbility_Cap;
            }
            //
            dst.mBuffer = out1;
            dst.mTransform  = 0;
            //
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
            crop2.mCropRect.s = crop.s;
            crop2.mCropRect.p_integral = crop.p;
            crop2.mCropRect.p_fractional = MPoint(0);

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


MBOOL doTwoPassHwNR(
        MUINT32 openId,
        IImageBuffer* buf,
        NS3A::EIspProfile_T const profile0,
        NS3A::EIspProfile_T const profile1)
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

        if ( !pInternalBuf->syncCache( eCACHECTRL_INVALID ) )
        {
            MY_LOGE("syncCache failed\n");
            goto lbExit;
        }

    }

    // 1st run
    if( !doHwNR(openId, buf, NULL, pInternalBuf.get(), MRect(MPoint(0), buf->getImgSize()),0, profile0) ) {
        MY_LOGE("1st run NR failed");
        goto lbExit;
    }

    // 2nd run
    if( !doHwNR(openId, pInternalBuf.get(), NULL, buf, MRect(MPoint(0), buf->getImgSize()), 0, profile1) ) {
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


SwNRParam*
SwNRParam::
getInstance(MUINT32 openId)
{
    Mutex::Autolock _l(g_SwParamlock);

    SwNRParam* pParam = NULL;
    list<SwNRParam*>::const_iterator iter;
    for( iter = g_SwParamlist.begin() ; iter != g_SwParamlist.end(); iter++ )
    {
        if( (*iter)->mOpenId == openId )
            pParam = (*iter);
    }

    if( pParam == NULL )
    {
        //create new
        pParam = new SwNRParam(openId);
        g_SwParamlist.push_back(pParam);
    }

    return pParam;
}


SwNRParam::
SwNRParam(MUINT32 openId)
    : mOpenId(openId)
    , mIso(0)
    , mbMfll(0)
    , miPerfLevel(-1)
    , mANR_LCE_LINK(0)
    , mSL2B_CENTR_X(0)
    , mSL2B_CENTR_Y(0)
    , mSL2B_RR_0(0)
    , mSL2B_RR_1(0)
    , mSL2B_RR_2(0)
    , mCurIdx(0)
{
}


SwNRParam::
~SwNRParam()
{
}


MVOID
SwNRParam::
setParam(MUINT32 const iso, MBOOL const isMfll, MINT32 const perfLevel)
{
    MY_LOGD("iso = %d, mfll %d, perf lv %d", iso, isMfll, perfLevel);
    mIso = iso;
    mbMfll = isMfll;
    miPerfLevel = perfLevel;
}


MVOID
SwNRParam::
setANR_LCE_LINK(MBOOL const bOn)
{
    mANR_LCE_LINK = bOn;
}


MVOID
SwNRParam::
setSL2B(
    MUINT32 const centrx,
    MUINT32 const centry,
    MUINT32 const rr0,
    MUINT32 const rr1,
    MUINT32 const rr2
    )
{
    mSL2B_CENTR_X = centrx;
    mSL2B_CENTR_Y = centry;
    mSL2B_RR_0 = (MUINT32)sqrt( (double)rr0 );
    mSL2B_RR_1 = (MUINT32)sqrt( (double)rr1 );
    mSL2B_RR_2 = (MUINT32)sqrt( (double)rr2 );
}


MBOOL
SwNRParam::
getTuningFromNvram(MUINT32 idx, MVOID** ppNRTuningInfo)
{
    if( idx >= eNUM_OF_SWNR_IDX )
    {
        MY_LOGE("wrong nvram idx %d", idx);
        return MFALSE;
    }

    mCurIdx = idx;

    // load some setting from nvram
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT* pNvram;

    MUINT sensorDev = SENSOR_DEV_NONE;
    {
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    }

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)pNvram);

    if( err != 0 )
    {
        MY_LOGE("getBufAndRead fail, err=%d", err);
        return MFALSE;
    }

    *ppNRTuningInfo = &(pNvram->swnr[idx]);
#if 0
    if( sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) != sizeof(NRTuningInfo) )
    {
        MY_LOGE("NVRAM_CAMERA_FEATURE_SWNR_STRUCT size %d, NRTuningInfo size %d",
                sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT), sizeof(NRTuningInfo) );
        return MFALSE;
    }
#endif
    // update
    //memcpy(pNRTuningInfo, &(pNvram->swnr[idx]), sizeof(NRTuningInfo) );

    return MTRUE;
}


MBOOL
SwNRParam::
getTuning(NRTuningInfo* pTuning)
{
    MUINT sensorDev = SENSOR_DEV_NONE;
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);

    SensorStaticInfo rSensorStaticInfo;
    MUINT32 u4SensorID;
    ESensorTG_T eSensorTG;
    switch  ( sensorDev )
    {
    case SENSOR_DEV_MAIN:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;
    case SENSOR_DEV_SUB:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;
    case SENSOR_DEV_MAIN_2:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;
    default:    //  Shouldn't happen.
        MY_ERR("Invalid sensor device: %d", sensorDev);
    }
    u4SensorID = rSensorStaticInfo.sensorDevID;
    MY_LOGD("sensorDev(%d), u4SensorID(%d)", sensorDev, u4SensorID);

    if( sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) < sizeof(NRTuningInfo) )
    {
        MY_LOGE("NVRAM_CAMERA_FEATURE_SWNR_STRUCT size %d < NRTuningInfo size %d",
                sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT), sizeof(NRTuningInfo) );
        return MFALSE;
    }

    if( is_to_invoke_swnr_interpolation(mbMfll, mIso) )
    {
        EIdxSwNR idx_low = map_ISO_value_to_index(mIso, mbMfll, 1);
        EIdxSwNR idx_high = map_ISO_value_to_index(mIso, mbMfll, 2);
        MUINT32 iso_low = map_index_to_ISO_value(idx_low);
        MUINT32 iso_high = map_index_to_ISO_value(idx_high);

        if( !mbMfll )
        {
            if( map_index_to_ISO_value(idx_low) < get_normal_SWNR_ENC_enable_ISO_threshold() &&
                map_index_to_ISO_value(idx_high) >= get_normal_SWNR_ENC_enable_ISO_threshold() )
            {
                if( mIso > get_normal_SWNR_ENC_enable_ISO_threshold() )
                {
                    idx_low = eIDX_SWNR_SINGLE_ISO_ANR_ENC_ON;
                }
                else
                {
                    idx_high = eIDX_SWNR_SINGLE_ISO_ANR_ENC_OFF;
                }
            }
        }
        else
        {
            if( map_index_to_ISO_value(idx_low) < get_MFB_SWNR_ENC_enable_ISO_threshold() &&
                map_index_to_ISO_value(idx_high) >= get_MFB_SWNR_ENC_enable_ISO_threshold() )
            {
                if( mIso > get_MFB_SWNR_ENC_enable_ISO_threshold() )
                {
                    idx_low = eIDX_SWNR_MFLL_ISO_ANR_ENC_ON;
                }
                else
                {
                    idx_high = eIDX_SWNR_MFLL_ISO_ANR_ENC_OFF;
                }
            }
        }
        MY_LOGD("%d: iso %d, isMfll %d, thres %d, idx low/high %d/%d",
                mOpenId,
                mIso,
                mbMfll,
                mbMfll ? get_normal_SWNR_ENC_enable_ISO_threshold() :
                         get_MFB_SWNR_ENC_enable_ISO_threshold(),
                idx_low,
                idx_high
               );
        MVOID* pNvramLow = NULL;
        MVOID* pNvramHigh = NULL;
        NVRAM_CAMERA_FEATURE_SWNR_STRUCT interpolated;
        if( !getTuningFromNvram(idx_low, &pNvramLow) ||
            !getTuningFromNvram(idx_high, &pNvramHigh) )
        {
            MY_LOGE("update from nvram failed");
            return MFALSE;
        }
        // interpolation
        MY_LOGD("sensorDev(%d), u4SensorID(%d)", sensorDev, u4SensorID);
        IspTuningCustom* pIspTuningCustom = IspTuningCustom::createInstance(static_cast<NSIspTuning::ESensorDev_T>(sensorDev), u4SensorID);
        SmoothSWNR(pIspTuningCustom->remap_ISO_value(mIso),
                   iso_high,
                   iso_low,
                   *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramHigh),
                   *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramLow),
                   interpolated);
        pIspTuningCustom->destroyInstance();

        // update
        memcpy(pTuning, &interpolated, sizeof(NRTuningInfo) );
    }
    else
    {
        MUINT32 idx = map_ISO_value_to_index(mIso, mbMfll);
        MY_LOGD("%d: iso %d, isMfll %d, idx %d",
                mOpenId,
                mIso,
                mbMfll,
                idx
               );

        // get nvram
        MVOID* pTuningNvram = NULL;
        if( !getTuningFromNvram(idx, &pTuningNvram) )
        {
            MY_LOGE("update from nvram failed");
            return MFALSE;
        }

        // update
        memcpy(pTuning, pTuningNvram, sizeof(NRTuningInfo) );
    }

    MY_LOGD("LCE_LINK %d", mANR_LCE_LINK);
    if( mANR_LCE_LINK == 0 )
    {
        pTuning->ANR_LCE_GAIN0 = 8;
        pTuning->ANR_LCE_GAIN1 = 8;
        pTuning->ANR_LCE_GAIN2 = 8;
        pTuning->ANR_LCE_GAIN3 = 8;
    }

    MY_LOGD("sl2b %d, %d, %d, %d, %d",
            mSL2B_CENTR_X,
            mSL2B_CENTR_Y,
            mSL2B_RR_0,
            mSL2B_RR_1,
            mSL2B_RR_2);

    pTuning->ANR_CEN_X = mSL2B_CENTR_X;
    pTuning->ANR_CEN_Y = mSL2B_CENTR_Y;
    pTuning->ANR_R1    = mSL2B_RR_0;
    pTuning->ANR_R2    = mSL2B_RR_1;
    pTuning->ANR_R3    = mSL2B_RR_2;

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug )
        {
#define DUMP_PARAM( param )                                  \
            do {                                             \
                MY_LOGD("%s:%d", #param, pTuning->param);  \
            } while(0)
            DUMP_PARAM(ANR_Y_LUMA_SCALE_RANGE);
            DUMP_PARAM(ANR_C_CHROMA_SCALE);
            DUMP_PARAM(ANR_Y_SCALE_CPY0);
            DUMP_PARAM(ANR_Y_SCALE_CPY1);
            DUMP_PARAM(ANR_Y_SCALE_CPY2);
            DUMP_PARAM(ANR_Y_SCALE_CPY3);
            DUMP_PARAM(ANR_Y_SCALE_CPY4);
            DUMP_PARAM(ANR_Y_CPX1);
            DUMP_PARAM(ANR_Y_CPX2);
            DUMP_PARAM(ANR_Y_CPX3);
            DUMP_PARAM(ANR_CEN_GAIN_LO_TH);
            DUMP_PARAM(ANR_CEN_GAIN_HI_TH);
            DUMP_PARAM(ANR_PTY_GAIN_TH);
            DUMP_PARAM(ANR_KSIZE_LO_TH);
            DUMP_PARAM(ANR_KSIZE_HI_TH);
            DUMP_PARAM(ANR_KSIZE_LO_TH_C);
            DUMP_PARAM(ANR_KSIZE_HI_TH_C);
            DUMP_PARAM(ITUNE_ANR_PTY_STD);
            DUMP_PARAM(ITUNE_ANR_PTU_STD);
            DUMP_PARAM(ITUNE_ANR_PTV_STD);
            DUMP_PARAM(ANR_ACT_TH_Y);
            DUMP_PARAM(ANR_ACT_BLD_BASE_Y);
            DUMP_PARAM(ANR_ACT_BLD_TH_Y);
            DUMP_PARAM(ANR_ACT_SLANT_Y);
            DUMP_PARAM(ANR_ACT_TH_C);
            DUMP_PARAM(ANR_ACT_BLD_BASE_C);
            DUMP_PARAM(ANR_ACT_BLD_TH_C);
            DUMP_PARAM(ANR_ACT_SLANT_C);
            DUMP_PARAM(RADIUS_H);
            DUMP_PARAM(RADIUS_V);
            DUMP_PARAM(RADIUS_H_C);
            DUMP_PARAM(RADIUS_V_C);
            DUMP_PARAM(ANR_PTC_HGAIN);
            DUMP_PARAM(ANR_PTY_HGAIN);
            DUMP_PARAM(ANR_LPF_HALFKERNEL);
            DUMP_PARAM(ANR_LPF_HALFKERNEL_C);
            DUMP_PARAM(ANR_ACT_MODE);
            DUMP_PARAM(ANR_LCE_SCALE_GAIN);
            DUMP_PARAM(ANR_LCE_C_GAIN);
            DUMP_PARAM(ANR_LCE_GAIN0);
            DUMP_PARAM(ANR_LCE_GAIN1);
            DUMP_PARAM(ANR_LCE_GAIN2);
            DUMP_PARAM(ANR_LCE_GAIN3);
            DUMP_PARAM(ANR_MEDIAN_LOCATION);
            DUMP_PARAM(ANR_CEN_X);
            DUMP_PARAM(ANR_CEN_Y);
            DUMP_PARAM(ANR_R1);
            DUMP_PARAM(ANR_R2);
            DUMP_PARAM(ANR_R3);
            DUMP_PARAM(LUMA_ON_OFF);
        }
    }

    return MTRUE;
}


MBOOL
SwNRParam::
dumpParamsToFile(char * const filename)
{
    // write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        MY_LOGE("fopen fail: %s", filename);
        return MFALSE;
    }
    else {

        fprintf(fp, "%d\n", get_normal_SWNR_ENC_enable_ISO_threshold());
        fprintf(fp, "%d\n", get_MFB_SWNR_ENC_enable_ISO_threshold());
        fprintf(fp, "\n");

        for( MUINT32 i = 0; i < NVRAM_SWNR_TBL_NUM; i++ )
        {
            MVOID* pTuningNvram = NULL;
            NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNRTuning = NULL;
            if( !getTuningFromNvram(i, &pTuningNvram) )
            {
                MY_LOGE("cannot get tuning from nvram");
                break;
            }

            pNRTuning = reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pTuningNvram);
#define FWRITE_PARAM( param )                                      \
            do {                                                   \
                fprintf(fp, "%s:%d\n", #param, pNRTuning->param);  \
            } while(0)
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
            FWRITE_PARAM(SWNR_TH1);
            FWRITE_PARAM(SWNR_TH2);
            FWRITE_PARAM(SWNR_TH3);
            FWRITE_PARAM(SWNR_TH4);
            FWRITE_PARAM(SWNR_TH5);
            fprintf(fp, "\n");
        }

        fclose(fp);
    }

    return MTRUE;
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
    mpNRInitInfo->pWorkingBuff = 0;
    mpNRInitInfo->WorkingBuffSize = 0;
    mpNRInitInfo->CoreNumber = 4;
    mpNRInitInfo->NumOfExecution = 1;
    mpNRInitInfo->pTuningInfo = NULL;
    mpNRInitInfo->pPerfInfo = NULL;
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

    MY_FREE(mpNRInitInfo->pPerfInfo);
    MY_FREE(mpNRInitInfo);
    MY_FREE(mpNRTuningInfo);
    MY_FREE(mpNRImageInfo);
    MY_FREE(mpNRResultInfo);

#undef MY_FREE
    if( mpWorkingBuf )
    {
        munmap(mpWorkingBuf, sizeof(MUINT8) * muWorkingBufSize);
        mpWorkingBuf = NULL;
    }
}


MBOOL
SwNR::
getTuningInfo()
{
    MBOOL ret = MFALSE;
    SwNRParam* pParam = SwNRParam::getInstance(muOpenId);
    MINT32 perfLevel = -1;
    //
    if( !pParam )
    {
        MY_LOGE("cannot get SwNRParam");
        goto lbExit;
    }
    //
    perfLevel = pParam->getPerfLevel();
    //
    if( !mpNRTuningInfo )
    {
        mpNRTuningInfo = (NRTuningInfo*)malloc(sizeof(NRTuningInfo));

        if( !mpNRTuningInfo )
        {
            MY_LOGE("allaoc tuning info failed");
            goto lbExit;
        }
    }
    //
    if( !pParam->getTuning(mpNRTuningInfo) )
    {
        MY_LOGE("getTuning failed");
        goto lbExit;
    }
    //
    ret = MTRUE;
lbExit:
    //
    // set performance level
    if( perfLevel != -1 ) {
        if( mpNRInitInfo->pPerfInfo == NULL ) {
            mpNRInitInfo->pPerfInfo = new NRPerfGear;
        }
        //
        switch(perfLevel)
        {
#define PerfCase(val, level)                             \
            case val:                                    \
                mpNRInitInfo->pPerfInfo->enable = 1;     \
                mpNRInitInfo->pPerfInfo->option = level; \
                break;
            PerfCase(2, NR_PERF_GEAR_PERF_FIRST)
            PerfCase(1, NR_PERF_GEAR_CUSTOM_0)
            PerfCase(0, NR_PERF_GEAR_POWER_FIRST)
            default:
                MY_LOGW("not valid level %d", perfLevel);
                delete mpNRInitInfo->pPerfInfo;
                mpNRInitInfo->pPerfInfo = NULL;
#undef PerfCase
        }
    }
    //
    mpNRInitInfo->pTuningInfo = mpNRTuningInfo;
    return ret;
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
    mpNRImageInfo->pImg[0] = (void*)pBuf->getBufVA(0);
    if( NRFmt == NR_IMAGE_YUV420 )
    {
        mpNRImageInfo->pImg[1] = (void*)pBuf->getBufVA(1);
        mpNRImageInfo->pImg[2] = (void*)pBuf->getBufVA(2);
    }
    else
    {
        mpNRImageInfo->pImg[1] = (void*)0;
        mpNRImageInfo->pImg[2] = (void*)0;
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
        mpWorkingBuf = (MUINT8*)mmap(NULL, sizeof(MUINT8) * size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if( mpWorkingBuf == MAP_FAILED )
        {
            MY_LOGE("allocate working buffer failed");
            goto lbExit;
        }
        MY_LOGD("allocate working buffer %p, size %d", mpWorkingBuf, size);
        muWorkingBufSize = size;
    }

    mpNRInitInfo->pWorkingBuff = mpWorkingBuf;
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

    DrvNRObject_e type = DRV_NR_OBJ_SW;
    switch( get_swnr_type() )
    {
        case eSWNRType_Default:
            type = DRV_NR_OBJ_SW;
            break;
        case eSWNRType_IIR:
            type = DRV_NR_OBJ_IIR;
            break;
        default:
            type = DRV_NR_OBJ_SW;
            break;
    }
    //
    MY_LOGD("swnr type %d", type);
    mpMTKNR = MTKNR::createInstance(type);

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
                mpNRInitInfo->pWorkingBuff,
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

    pBuf->syncCache(eCACHECTRL_FLUSH);

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


