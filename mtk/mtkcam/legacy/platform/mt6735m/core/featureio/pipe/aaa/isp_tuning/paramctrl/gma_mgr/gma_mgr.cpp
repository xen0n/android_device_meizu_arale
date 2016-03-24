/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "gma_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "gma_mgr.h"
#include <isp_tuning_mgr.h>

using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class GmaMgrDev : public GmaMgr
{
public:
    static
    GmaMgr*
    getInstance(ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, MTK_GMA_ENV_INFO_STRUCT* pCustomEnv)
    {
        static GmaMgrDev<eSensorDev> singleton(rIspNvramReg, pCustomEnv);
        return &singleton;
    }
    virtual MVOID destroyInstance() {}

    GmaMgrDev(ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, MTK_GMA_ENV_INFO_STRUCT* pCustomEnv)
        : GmaMgr(eSensorDev, rIspNvramReg, pCustomEnv)
    {}

    virtual ~GmaMgrDev() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INSTANTIATE(_dev_id) \
    case _dev_id: return  GmaMgrDev<_dev_id>::getInstance(rIspNvramReg, pCustomEnv)

GmaMgr*
GmaMgr::
createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, MTK_GMA_ENV_INFO_STRUCT* pCustomEnv)
{
    switch  (eSensorDev)
    {
    INSTANTIATE(ESensorDev_Main);       //  Main Sensor
    INSTANTIATE(ESensorDev_MainSecond); //  Main Second Sensor
    INSTANTIATE(ESensorDev_Sub);        //  Sub Sensor
    default:
        break;
    }

    return  MNULL;
}

GmaMgr::GmaMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, MTK_GMA_ENV_INFO_STRUCT* pCustomEnv)
    : m_rIspNvramReg  (rIspNvramReg)
    , m_eSensorDev  (eSensorDev)
    , m_pCustomEnv (pCustomEnv)
    //, m_pGmaAlgo (MTKGma::createInstance())
    , mGmaScenario (0) //E_GMA_SCENARIO_PREVIEW
    , mbAEStable (MFALSE)
    , mLogEn (0)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.gma_algo.enable", value, "1");
    mAlgoEn = atoi(value);

    MY_LOG_IF(1, "[%s()] before MTKGma::createInstance", __FUNCTION__);
    
    if (mAlgoEn) m_pGmaAlgo = MTKGma::createInstance(static_cast<eGMASensorDev_T>(m_eSensorDev));
    else m_pGmaAlgo = NULL;

    MY_LOG_IF(1, "[%s()] m_pGmaAlgo(%p), m_eSensorDev(%d), m_pCustomEnv(%d)", __FUNCTION__, m_pGmaAlgo, m_eSensorDev, m_pCustomEnv);
        
    if (mAlgoEn) m_pGmaAlgo->GmaReset();

    MY_LOG_IF(1, "[%s()] after GmaReset", __FUNCTION__);
    
    for (int tb_num = 0; tb_num<GMA_TBL_NUM; tb_num++)
    {
        for (int i = 0; i<GGM_LUT_SIZE; i++)
        {
            mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = m_rIspNvramReg.GGM[tb_num].g_gmt.lut[i].G_GAMMA;
            //How about R and B channels
        }
    }
    mEnv.rTuningGmaParam = m_pCustomEnv->rTuningGmaParam;

    //FIXME!!!!!!!!!!!!!!!
    //mEnv.rTuningGmaParam.eGMAMode = eFIXED_GMA_MODE;

    MY_LOG_IF(1, "[%s()] before GmaInit", __FUNCTION__);
    
    if (mAlgoEn) m_pGmaAlgo->GmaInit(&mEnv, NULL);

    MY_LOG_IF(1, "[%s()] after GmaInit", __FUNCTION__);
    
}


GmaMgr::~GmaMgr() 
{
   MY_LOG_IF(1, "[%s()] before GmaExit (%p)", __FUNCTION__, m_pGmaAlgo);
   if (mAlgoEn) m_pGmaAlgo->GmaExit();
   MY_LOG_IF(1, "[%s()] after GmaExit", __FUNCTION__);
   if (mAlgoEn) m_pGmaAlgo->destroyInstance(m_pGmaAlgo);
   MY_LOG_IF(1, "[%s()] after destroyInstance (%p)", __FUNCTION__, m_pGmaAlgo);
}

MVOID printAEInfo(AE_INFO_T const & rAEInfo, const char* username)
{
        MY_LOG("%s: [%s] rAEInfo: %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d;"
            , __FUNCTION__
            , username
            
            , rAEInfo.u4AETarget
            , rAEInfo.u4AECurrentTarget
            , rAEInfo.u4Eposuretime
            , rAEInfo.u4AfeGain
            , rAEInfo.u4IspGain
            
            , rAEInfo.u4RealISOValue
            , rAEInfo.i4LightValue_x10
            , rAEInfo.u4AECondition
            , rAEInfo.i2FlareOffset
            , rAEInfo.i4GammaIdx
            
            , rAEInfo.i4LESE_Ratio
            , rAEInfo.u4SWHDR_SE
            , rAEInfo.u4MaxISO
            , rAEInfo.u4AEStableCnt
            , rAEInfo.u4OrgExposuretime
            
            , rAEInfo.u4OrgRealISOValue
            , rAEInfo.bGammaEnable
            , rAEInfo.u4EVRatio
            );
    
    /*
        typedef struct {
            MUINT32 u4AETarget;
            MUINT32 u4AECurrentTarget;
            MUINT32 u4Eposuretime;   //!<: Exposure time in ms
            MUINT32 u4AfeGain;           //!<: raw gain
            MUINT32 u4IspGain;           //!<: sensor gain
            MUINT32 u4RealISOValue;
            MINT32  i4LightValue_x10;
            MUINT32 u4AECondition;
            MINT16  i2FlareOffset;
            MINT32  i4GammaIdx;   // next gamma idx
            MINT32  i4LESE_Ratio;    // LE/SE ratio
            MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
            MUINT32 u4MaxISO;
            MUINT32 u4AEStableCnt;
            MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
            MUINT32 u4OrgRealISOValue;
            MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
            MBOOL bGammaEnable;
        
            MINT32 i4AEStable;
            MINT32 i4EVRatio;
        } GMA_AE_INFO_T;
    */
    
        const MUINT16* pHist = &rAEInfo.u2Histogrm[0];
        for (int i=0; i<120; i+=10)
        {
            MY_LOG( "%s: rAEInfo.u2Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                , __FUNCTION__
                , i
                , i+9
                , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
                , pHist[i+5], pHist[i+6], pHist[i+7], pHist[i+8], pHist[i+9]);
        }
        //(i == 120)
        {
            int i=120;
            MY_LOG( "%s: rAEInfo.u2Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x)"
                , __FUNCTION__
                , i
                , i+7
                , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
                , pHist[i+5], pHist[i+6], pHist[i+7]);
        }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID 
GmaMgr::
stop()
{
    MY_LOG_IF(mLogEn, "[%s()] mGmaScenario(%d)", __FUNCTION__, mGmaScenario);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID 
GmaMgr::
start()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.dynamic_gma.log", value, "0");
    mLogEn = atoi(value);


    switch (mGmaScenario)
    {
    case IspTuningMgr::E_GMA_SCENARIO_PREVIEW:
        mInInfo.eSensorMode = eGMA_PREVIEW_MODE;
        break;
    case IspTuningMgr::E_GMA_SCENARIO_CAPTURE:
        mInInfo.eSensorMode = eGMA_CAPTURE_MODE;
        break;
    case IspTuningMgr::E_GMA_SCENARIO_VIDEO:
        mInInfo.eSensorMode = eGMA_VIDEO_MODE;
        break;
    default:
        MY_ERR("%s: scenario not supported (%d)", __FUNCTION__, mGmaScenario);
        break;
    }
    MY_LOG_IF(mLogEn, "[%s()] m_eSensorDev(%d), mGmaScenario(%d)", __FUNCTION__, m_eSensorDev, mGmaScenario);

/*
    typedef struct {
        MUINT32 u4AETarget;
        MUINT32 u4AECurrentTarget;
        MUINT32 u4Eposuretime;   //!<: Exposure time in ms
        MUINT32 u4AfeGain;           //!<: raw gain
        MUINT32 u4IspGain;           //!<: sensor gain
        MUINT32 u4RealISOValue;
        MINT32  i4LightValue_x10;
        MUINT32 u4AECondition;
        MINT16  i2FlareOffset;
        MINT32  i4GammaIdx;   // next gamma idx
        MINT32  i4LESE_Ratio;    // LE/SE ratio
        MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
        MUINT32 u4MaxISO;
        MUINT32 u4AEStableCnt;
        MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
        MUINT32 u4OrgRealISOValue;
        MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
        MBOOL bGammaEnable;
    
        MINT32 i4AEStable;
        MINT32 i4EVRatio;
    } GMA_AE_INFO_T;
*/


    mInInfo.rGMAAEInfo.u4AETarget= mAEInfo.u4AETarget;
    mInInfo.rGMAAEInfo.u4AECurrentTarget= mAEInfo.u4AECurrentTarget;
    mInInfo.rGMAAEInfo.u4Eposuretime= mAEInfo.u4Eposuretime;
    mInInfo.rGMAAEInfo.u4AfeGain= mAEInfo.u4AfeGain;
    mInInfo.rGMAAEInfo.u4IspGain= mAEInfo.u4IspGain;
    mInInfo.rGMAAEInfo.u4RealISOValue= mAEInfo.u4RealISOValue;
    mInInfo.rGMAAEInfo.i4LightValue_x10= mAEInfo.i4LightValue_x10;
    mInInfo.rGMAAEInfo.u4AECondition= mAEInfo.u4AECondition;
    mInInfo.rGMAAEInfo.i2FlareOffset= mAEInfo.i2FlareOffset;
    mInInfo.rGMAAEInfo.i4GammaIdx= mAEInfo.i4GammaIdx;
    mInInfo.rGMAAEInfo.i4LESE_Ratio= mAEInfo.i4LESE_Ratio;
    mInInfo.rGMAAEInfo.u4SWHDR_SE= mAEInfo.u4SWHDR_SE;
    mInInfo.rGMAAEInfo.u4MaxISO= mAEInfo.u4MaxISO;
    mInInfo.rGMAAEInfo.u4AEStableCnt= mAEInfo.u4AEStableCnt;
    mInInfo.rGMAAEInfo.u4OrgExposuretime= mAEInfo.u4OrgExposuretime;
    mInInfo.rGMAAEInfo.u4OrgRealISOValue= mAEInfo.u4OrgRealISOValue;

    for (int i = 0; i<GMA_AE_HISTOGRAM_BIN; i++)
    {    
        mInInfo.rGMAAEInfo.u2Histogrm[i]= mAEInfo.u2Histogrm[i];
    }
    for (int i = 0; i<GGM_LUT_SIZE; i++)
    {
        mInInfo.i4CurrEncGMA[i] = m_rIspNvramReg.GGM[0].g_gmt.lut[i].G_GAMMA;
    }
        
    mInInfo.rGMAAEInfo.bGammaEnable = mAEInfo.bGammaEnable;

    mInInfo.rGMAAEInfo.i4AEStable = mbAEStable;
    mInInfo.rGMAAEInfo.i4EVRatio = mAEInfo.u4EVRatio; //1024; //FIXME: 1024


    for (int tb_num = 0; tb_num<GMA_TBL_NUM; tb_num++)
    {
        for (int i = 0; i<GGM_LUT_SIZE; i++)
        {
            mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = m_rIspNvramReg.GGM[tb_num].g_gmt.lut[i].G_GAMMA;
            //How about R and B channels
        }
    }
    mEnv.rTuningGmaParam = m_pCustomEnv->rTuningGmaParam;


    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_ENV_INFO, &mEnv, NULL);
    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_PROC_INFO, &mInInfo, NULL);

    MY_LOG_IF(mLogEn, "[%s()] after MTKGMA_FEATURE_SET_PROC_INFO", __FUNCTION__);
    
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID 
GmaMgr::
calculateGGM(ISP_NVRAM_GGM_T* pGGMReg, NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo* pGmaExif)
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.dynamic_gma.log", value, "0");
    mLogEn = atoi(value);


    switch (mGmaScenario)
    {
    case IspTuningMgr::E_GMA_SCENARIO_PREVIEW:
        mInInfo.eSensorMode = eGMA_PREVIEW_MODE;
        break;
    case IspTuningMgr::E_GMA_SCENARIO_CAPTURE:
        mInInfo.eSensorMode = eGMA_CAPTURE_MODE;
        break;
    case IspTuningMgr::E_GMA_SCENARIO_VIDEO:
        mInInfo.eSensorMode = eGMA_VIDEO_MODE;
        break;
    default:
        MY_ERR("%s: scenario not supported (%d)", __FUNCTION__, mGmaScenario);
        break;
    }
    MY_LOG_IF(mLogEn, "[%s()] (m_eSensorDev, mGmaScenario, pGGMReg, pGmaExif) = (%d, %d, %p, %p)", __FUNCTION__, m_eSensorDev, mGmaScenario, pGGMReg, pGmaExif);

/*
    typedef struct {
        MUINT32 u4AETarget;
        MUINT32 u4AECurrentTarget;
        MUINT32 u4Eposuretime;   //!<: Exposure time in ms
        MUINT32 u4AfeGain;           //!<: raw gain
        MUINT32 u4IspGain;           //!<: sensor gain
        MUINT32 u4RealISOValue;
        MINT32  i4LightValue_x10;
        MUINT32 u4AECondition;
        MINT16  i2FlareOffset;
        MINT32  i4GammaIdx;   // next gamma idx
        MINT32  i4LESE_Ratio;    // LE/SE ratio
        MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
        MUINT32 u4MaxISO;
        MUINT32 u4AEStableCnt;
        MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
        MUINT32 u4OrgRealISOValue;
        MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
        MBOOL bGammaEnable;
    
        MINT32 i4AEStable;
        MINT32 i4EVRatio;
    } GMA_AE_INFO_T;
*/


    mInInfo.rGMAAEInfo.u4AETarget= mAEInfo.u4AETarget;
    mInInfo.rGMAAEInfo.u4AECurrentTarget= mAEInfo.u4AECurrentTarget;
    mInInfo.rGMAAEInfo.u4Eposuretime= mAEInfo.u4Eposuretime;
    mInInfo.rGMAAEInfo.u4AfeGain= mAEInfo.u4AfeGain;
    mInInfo.rGMAAEInfo.u4IspGain= mAEInfo.u4IspGain;
    mInInfo.rGMAAEInfo.u4RealISOValue= mAEInfo.u4RealISOValue;
    mInInfo.rGMAAEInfo.i4LightValue_x10= mAEInfo.i4LightValue_x10;
    mInInfo.rGMAAEInfo.u4AECondition= mAEInfo.u4AECondition;
    mInInfo.rGMAAEInfo.i2FlareOffset= mAEInfo.i2FlareOffset;
    mInInfo.rGMAAEInfo.i4GammaIdx= mAEInfo.i4GammaIdx;
    mInInfo.rGMAAEInfo.i4LESE_Ratio= mAEInfo.i4LESE_Ratio;
    mInInfo.rGMAAEInfo.u4SWHDR_SE= mAEInfo.u4SWHDR_SE;
    mInInfo.rGMAAEInfo.u4MaxISO= mAEInfo.u4MaxISO;
    mInInfo.rGMAAEInfo.u4AEStableCnt= mAEInfo.u4AEStableCnt;
    mInInfo.rGMAAEInfo.u4OrgExposuretime= mAEInfo.u4OrgExposuretime;
    mInInfo.rGMAAEInfo.u4OrgRealISOValue= mAEInfo.u4OrgRealISOValue;

    for (int i = 0; i<GMA_AE_HISTOGRAM_BIN; i++)
    {    
        mInInfo.rGMAAEInfo.u2Histogrm[i]= mAEInfo.u2Histogrm[i];
    }
    for (int i = 0; i<GGM_LUT_SIZE; i++)
    {
        mInInfo.i4CurrEncGMA[i] = pGGMReg->g_gmt.lut[i].G_GAMMA;
    }
    
    mInInfo.rGMAAEInfo.bGammaEnable = mAEInfo.bGammaEnable;

    mInInfo.rGMAAEInfo.i4AEStable = mbAEStable;
    mInInfo.rGMAAEInfo.i4EVRatio = mAEInfo.u4EVRatio; //1024; //FIXME: 1024


    if (mLogEn) printInfo();
    
    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
    if (mAlgoEn) m_pGmaAlgo->GmaMain();
    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_RESULT, NULL, &mOutGGM);
    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_EXIF, NULL, &mOutExif);

    if (mLogEn) 
    {
        MY_LOG_IF(1, "[%s()] m_eSensorDev(%d) after GmaMain()", __FUNCTION__, m_eSensorDev);
        printInfo();
        printAEInfo(mAEInfo, "mAEInfo_afterAlgo");
    }

    if (mAlgoEn)
    {
        for (int i = 0; i<(GGM_LUT_SIZE); i++)
        {
            pGGMReg->rb_gmt.lut[i].R_GAMMA = pGGMReg->rb_gmt.lut[i].B_GAMMA = pGGMReg->g_gmt.lut[i].G_GAMMA = mOutGGM.i4EncGMA[i];
            pGGMReg->g_gmt.lut[i].rsv_16 = 0;

            if (mLogEn)
            {
                if (((i%10) == 0) && (i!=140))
                {
                    MY_LOG( "%s: mOutGGM.i4EncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                        , __FUNCTION__
                        , i
                        , i+9
                        , mOutGGM.i4EncGMA[i], mOutGGM.i4EncGMA[i+1], mOutGGM.i4EncGMA[i+2], mOutGGM.i4EncGMA[i+3], mOutGGM.i4EncGMA[i+4]
                        , mOutGGM.i4EncGMA[i+5], mOutGGM.i4EncGMA[i+6], mOutGGM.i4EncGMA[i+7], mOutGGM.i4EncGMA[i+8], mOutGGM.i4EncGMA[i+9]);
                }
                else if (i == 140)
                {
                    MY_LOG( "%s: mOutGGM.i4EncGMA[%d-%d]=(%x,%x,%x,%x)"
                        , __FUNCTION__
                        , i
                        , i+3
                        , mOutGGM.i4EncGMA[i], mOutGGM.i4EncGMA[i+1], mOutGGM.i4EncGMA[i+2], mOutGGM.i4EncGMA[i+3]);

                }
            }
        }
        ::memcpy(pGmaExif, &mOutExif, sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo));

        if (mLogEn)
        {
            MY_LOG("%s: GMA EXIF: %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x;"
                , __FUNCTION__

                , mOutExif.i4GMAMode
                , mOutExif.i4SensorMode
                , mOutExif.i4EVRatio
                , mOutExif.i4LowContrastThr
                , mOutExif.i4EVLowContrastThr
                
                , mOutExif.i4Contrast
                , mOutExif.i4ContrastY
                , mOutExif.i4EVContrastY
                , mOutExif.i4ContrastWeight
                , mOutExif.i4LV
                
                , mOutExif.i4LVWeight
                , mOutExif.i4SmoothEnable
                , mOutExif.i4SmoothSpeed
                , mOutExif.i4SmoothWaitAE
                , mOutExif.i4GMACurveEnable
                
                , mOutExif.i4CenterPt
                , mOutExif.i4LowCurve
                , mOutExif.i4SlopeL
                , mOutExif.i4FlareEnable
                , mOutExif.i4FlareOffset);

        /*
        typedef struct
        {
            MINT32 i4GMAMode;
            MINT32 i4SensorMode;
            MINT32 i4EVRatio;
            MINT32 i4LowContrastThr;
            MINT32 i4EVLowContrastThr;
            MINT32 i4Contrast;
            MINT32 i4ContrastY;
            MINT32 i4EVContrastY;
            MINT32 i4ContrastWeight;
            MINT32 i4LV;
            MINT32 i4LVWeight;
            MINT32 i4SmoothEnable;
            MINT32 i4SmoothSpeed;
            MINT32 i4SmoothWaitAE;
            MINT32 i4GMACurveEnable;
            MINT32 i4CenterPt;
            MINT32 i4LowCurve;
            MINT32 i4SlopeL;
            MINT32 i4FlareEnable;
            MINT32 i4FlareOffset;
        } MTK_GMA_EXIF_INFO_STRUCT, *P_MTK_GMA_EXIF_INFO_STRUCT;
        */


            
        }
    }
}

MVOID GmaMgr::printInfo() const
{

    MY_LOG("%s: mInInfo(%d): %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d;"
        , __FUNCTION__
        , mInInfo.eSensorMode
        
        , mInInfo.rGMAAEInfo.u4AETarget
        , mInInfo.rGMAAEInfo.u4AECurrentTarget
        , mInInfo.rGMAAEInfo.u4Eposuretime
        , mInInfo.rGMAAEInfo.u4AfeGain
        , mInInfo.rGMAAEInfo.u4IspGain
        
        , mInInfo.rGMAAEInfo.u4RealISOValue
        , mInInfo.rGMAAEInfo.i4LightValue_x10
        , mInInfo.rGMAAEInfo.u4AECondition
        , mInInfo.rGMAAEInfo.i2FlareOffset
        , mInInfo.rGMAAEInfo.i4GammaIdx
        
        , mInInfo.rGMAAEInfo.i4LESE_Ratio
        , mInInfo.rGMAAEInfo.u4SWHDR_SE
        , mInInfo.rGMAAEInfo.u4MaxISO
        , mInInfo.rGMAAEInfo.u4AEStableCnt
        , mInInfo.rGMAAEInfo.u4OrgExposuretime
        
        , mInInfo.rGMAAEInfo.u4OrgRealISOValue
        , mInInfo.rGMAAEInfo.bGammaEnable
        , mInInfo.rGMAAEInfo.i4AEStable
        , mInInfo.rGMAAEInfo.i4EVRatio
        );

    /*
        typedef struct {
            MUINT32 u4AETarget;
            MUINT32 u4AECurrentTarget;
            MUINT32 u4Eposuretime;   //!<: Exposure time in ms
            MUINT32 u4AfeGain;           //!<: raw gain
            MUINT32 u4IspGain;           //!<: sensor gain
            MUINT32 u4RealISOValue;
            MINT32  i4LightValue_x10;
            MUINT32 u4AECondition;
            MINT16  i2FlareOffset;
            MINT32  i4GammaIdx;   // next gamma idx
            MINT32  i4LESE_Ratio;    // LE/SE ratio
            MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
            MUINT32 u4MaxISO;
            MUINT32 u4AEStableCnt;
            MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
            MUINT32 u4OrgRealISOValue;
            MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
            MBOOL bGammaEnable;
        
            MINT32 i4AEStable;
            MINT32 i4EVRatio;
        } GMA_AE_INFO_T;
    */

    const MUINT16* pHist = &mInInfo.rGMAAEInfo.u2Histogrm[0];
    for (int i=0; i<120; i+=10)
    {
        MY_LOG( "%s: mInInfo.rGMAAEInfo.u2Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
            , __FUNCTION__
            , i
            , i+9
            , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
            , pHist[i+5], pHist[i+6], pHist[i+7], pHist[i+8], pHist[i+9]);
    }
    //(i == 120)
    {
        int i=120;
        MY_LOG( "%s: mInInfo.rGMAAEInfo.u2Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x)"
            , __FUNCTION__
            , i
            , i+7
            , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
            , pHist[i+5], pHist[i+6], pHist[i+7]);
    }



    for (int i=0; i<140; i+=10)
    {
        MY_LOG( "%s: mInInfo.i4CurrEncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
            , __FUNCTION__
            , i
            , i+9
            , mInInfo.i4CurrEncGMA[i], mInInfo.i4CurrEncGMA[i+1], mInInfo.i4CurrEncGMA[i+2], mInInfo.i4CurrEncGMA[i+3], mInInfo.i4CurrEncGMA[i+4]
            , mInInfo.i4CurrEncGMA[i+5], mInInfo.i4CurrEncGMA[i+6], mInInfo.i4CurrEncGMA[i+7], mInInfo.i4CurrEncGMA[i+8], mInInfo.i4CurrEncGMA[i+9]);
    }
    //(i == 140)
    {
        MY_LOG( "%s: mInInfo.i4CurrEncGMA[%d-%d]=(%x,%x,%x,%x)"
            , __FUNCTION__
            , 140
            , 140+3
            , mInInfo.i4CurrEncGMA[140], mInInfo.i4CurrEncGMA[140+1], mInInfo.i4CurrEncGMA[140+2], mInInfo.i4CurrEncGMA[140+3]);
    
    }

}

MVOID GmaMgr::setAEInfo(AE_INFO_T const & rAEInfo)
{
    mAEInfo = rAEInfo;

    if (!mLogEn) return;

    printAEInfo(rAEInfo, "setAESrc");
    printAEInfo(mAEInfo, "setAEDst");

}


