/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "sync3a"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_log.h>
#include <mtkcam/featureio/ISync3A.h>
#include <mtkcam/algorithm/libsync3a/MTKSyncAe.h>
#include <mtkcam/algorithm/libsync3a/MTKSyncAwb.h>

#include <Local.h>
#include <ae_mgr_if.h>
#include <awb_mgr_if.h>

#include "nvbuf_util.h"
#include <aaa_scheduling_custom.h>
#include <cutils/properties.h>

#include "n3d_sync2a_tuning_param.h"
#include <camera_custom_imgsensor_cfg.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/UITypes.h>
#include <isp_tuning.h>

#include <debug_exif/dbg_id_param.h>        // for DEBUG_XXX_KEY_ID
#include <debug_exif/cam/dbg_cam_param.h>
#include <debug_exif/cam/dbg_cam_n3d_param.h>

#include <string>
#include <math.h>

#include "aaa_scheduler.h"
#include "aaa_hal.h"

using namespace NS3A;
using namespace android;
using namespace NSIspTuning;

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

#define MIN(a,b)  ((a) < (b) ? (a) : (b))

static NSCam::MRect
get2AStatROI(
    float main1_FOV_vertical,
    float main1_FOV_horizontal,
    float main2_FOV_vertical,
    float main2_FOV_horizontal,
    int tg_size_w, int tg_size_h)
{
    NSCam::MRect CropResult;

    int width, height;
    int offset_x, offset_y;

    width  = (int)( ::tan(main1_FOV_horizontal/180.0f*(float)::acos(0.0f)) / ::tan(main2_FOV_horizontal/180.0f*(float)::acos(0.0f)) * tg_size_w/2 )*2 ;
    height = (int)( ::tan(main1_FOV_vertical/180.0f*(float)::acos(0.0f)) / ::tan(main2_FOV_vertical/180.0f*(float)::acos(0.0f)) * tg_size_h/2 )*2 ;
    offset_x = ((( tg_size_w - width )>>2 )<<1) ;
    offset_y = ((( tg_size_h - height)>>2 )<<1) ;

    MY_LOG("[%s] FOV1_V(%3.3f), FOV1_H(%3.3f), FOV2_V(%3.3f), FOV2_H(%3.3f), tg(%d, %d)", __FUNCTION__,
        main1_FOV_vertical, main1_FOV_horizontal, main2_FOV_vertical, main2_FOV_horizontal, tg_size_w, tg_size_h);

    CropResult.p.x = offset_x;
    CropResult.p.y = offset_y;

    CropResult.s.w = width;
    CropResult.s.h = height;

    return CropResult;
}

/******************************************************************************
 *  Sync3AWrapper
 ******************************************************************************/
class Sync3AWrapper
{
public:
    static Sync3AWrapper*       getInstance();

    virtual MBOOL               init(MINT32 i4Master, MINT32 i4Slave);

    virtual MBOOL               uninit();

    virtual MBOOL               syncCalc(MINT32 i4Opt);

    virtual MBOOL               setDebugInfo(void* prDbgInfo) const;

    virtual MBOOL               isAeStable() const {return (m_fgMasterAeStable && m_fgSlaveAeStable);}

protected:
    Sync3AWrapper();
    virtual ~Sync3AWrapper(){}

    virtual MBOOL               update(MINT32 i4Opt);

    MBOOL                       syncAeInit();
    MBOOL                       syncAwbInit();

    MBOOL                       syncAeMain();
    MBOOL                       syncAwbMain();

    mutable Mutex               m_Lock;
    MINT32                      m_i4User;

    MINT32                      m_i4Master;
    MINT32                      m_i4Slave;

    MBOOL                       m_fgMasterAeStable;
    MBOOL                       m_fgSlaveAeStable;

    MTKSyncAe*                  m_pMTKSyncAe;
    MTKSyncAwb*                 m_pMTKSyncAwb;

    SYNC_AWB_INPUT_INFO_STRUCT m_rAwbSyncInput;
    SYNC_AWB_OUTPUT_INFO_STRUCT m_rAwbSyncOutput;
    SYNC_AE_OUTPUT_STRUCT       m_rAeSyncOutput;
};

/******************************************************************************
 *  Sync3AWrapper Implementation
 ******************************************************************************/
Sync3AWrapper::
Sync3AWrapper()
    : m_Lock()
    , m_i4User(0)
{}

Sync3AWrapper*
Sync3AWrapper::getInstance()
{
    static Sync3AWrapper _rWrap;
    return &_rWrap;
}

MBOOL
Sync3AWrapper::
init(MINT32 i4Master, MINT32 i4Slave)
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User > 0)
    {

    }
    else
    {
        MY_LOG("[%s] User(%d), eSensor(%d,%d)", __FUNCTION__, m_i4User, i4Master, i4Slave);
        m_i4Master = i4Master;
        m_i4Slave = i4Slave;
        m_fgMasterAeStable = MFALSE;
        m_fgSlaveAeStable = MFALSE;
        syncAeInit();
        syncAwbInit();
    }

    m_i4User++;

    return MTRUE;
}

MBOOL
Sync3AWrapper::
uninit()
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User > 0)
    {
        // More than one user, so decrease one User.
        m_i4User--;

        if (m_i4User == 0) // There is no more User after decrease one User
        {
            MY_LOG("[%s]", __FUNCTION__);
            if (m_pMTKSyncAe)
            {
                m_pMTKSyncAe->destroyInstance();
                m_pMTKSyncAe = NULL;
            }
            if (m_pMTKSyncAwb)
            {
                m_pMTKSyncAwb->destroyInstance();
                m_pMTKSyncAwb = NULL;
            }
        }
        else    // There are still some users.
        {
            //CAM_LOGD(m_bDebugEnable,"Still %d users \n", mi4User);
        }
    }

    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncCalc(MINT32 i4Opt)
{
    if (i4Opt & (ISync3A::E_SYNC3A_DO_AE|ISync3A::E_SYNC3A_DO_AE_PRECAP))
    {
        if (!syncAeMain())
            MY_ERR("syncAeMain fail");
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        if (!syncAwbMain())
            MY_ERR("syncAwbMain fail");
    }

    return update(i4Opt);
}


MBOOL
Sync3AWrapper::
syncAeInit()
{
    MY_LOG("[%s] +", __FUNCTION__);

    m_pMTKSyncAe = MTKSyncAe::createInstance();
    if (!m_pMTKSyncAe)
    {
        MY_ERR("MTKSyncAe::createInstance() fail");
        return MFALSE;
    }

    SYNC_AE_INIT_STRUCT rAEInitData;
    ::memset(&rAEInitData, 0x0, sizeof(SYNC_AE_INIT_STRUCT));  //  initialize, Set all to 0
    rAEInitData.SyncScenario = SYNC_AE_SCENARIO_N3D;
    rAEInitData.SyncNum = 2; //N3D 2 cam
    rAEInitData.eSyncMode = eN3DAE_SYNC_TV;
    rAEInitData.SyncAeTuningParam.syncPolicy = (SYNC_AE_POLICY) getSyncAePolicy();

    m_rAeSyncOutput.main_ch.u4SyncGain = 1024;
    m_rAeSyncOutput.sub_ch.u4SyncGain = 1024;
    NVRAM_CAMERA_3A_STRUCT* p3aNvram;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Master, (void*&)p3aNvram, 0);
    rAEInitData.main_param.pAeNvram = &p3aNvram->rAENVRAM;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Slave, (void*&)p3aNvram, 0);
    rAEInitData.sub_param.pAeNvram = &p3aNvram->rAENVRAM;

    m_pMTKSyncAe->SyncAeInit(reinterpret_cast<void*>(&rAEInitData));

    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAwbInit()
{
    MY_LOG("[%s] +", __FUNCTION__);

    m_pMTKSyncAwb = MTKSyncAwb::createInstance();
    if (!m_pMTKSyncAwb)
    {
        MY_ERR("MTKSyncAwb::createInstance() fail");
        return MFALSE;
    }

    SYNC_AWB_INIT_INFO_STRUCT rAWBInitData;
    ::memset(&rAWBInitData, 0x0, sizeof(SYNC_AWB_INIT_INFO_STRUCT));  //  initialize, Set all to 0

    rAWBInitData.SyncAwbTuningParam.SyncMode = getSyncAwbMode();
    //rAWBInitData.SyncAwbTuningParam.CCTDiffTh = getSyncCCTDiffTh();
    ::memcpy(rAWBInitData.SyncAwbTuningParam.GainRatioTh, getSyncGainRatioTh(), sizeof(MUINT32)*4);
    ::memcpy(rAWBInitData.SyncAwbTuningParam.CCTDiffTh, getSyncCCTDiffTh(), sizeof(MUINT32)*5);

    rAWBInitData.SyncAwbTuningParam.PP_method_valid_block_num_ratio =  getSyncAwbStatNumRatio();
    rAWBInitData.SyncAwbTuningParam.PP_method_Y_threshold = getSynAwbStatYTh();


    rAWBInitData.SyncScenario = SYNC_AWB_SCENARIO_N3D;
    IAwbMgr::getInstance().CCTOPAWBGetNVRAMParam(m_i4Master, AWB_NVRAM_IDX_NORMAL, &rAWBInitData.main_param);
    IAwbMgr::getInstance().CCTOPAWBGetNVRAMParam(m_i4Slave, AWB_NVRAM_IDX_NORMAL, &rAWBInitData.sub_param);

    m_pMTKSyncAwb->SyncAwbInit(reinterpret_cast<void*>(&rAWBInitData));

    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAeMain()
{
    MY_LOG("[%s] +", __FUNCTION__);

    SYNC_AE_INPUT_STRUCT rAeSyncInput;
    rAeSyncInput.SyncCamScenario = SYNC_AE_CAM_SCENARIO_PREVIEW;
    AE_MODE_CFG_T _a_rAEOutput_main, _a_rAEOutput_main2;
    strAETable strPreviewAEPlineTableMain, strCaptureAEPlineTabMain;
    strAETable strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2;
    strAETable strStrobeAEPlineTableMain, strStrobeAEPlineTableMain2;
    strAFPlineInfo strStrobeAFPlineTabMain, strStrobeAFPlineTabMain2;
    LCEInfo_T rLCEInfoMain, rLCEInfoMain2;
    MUINT32 u4AEConditionMain, u4AEConditionMain2;

    IAeMgr::getInstance().getPreviewParams(m_i4Master, _a_rAEOutput_main);
    IAeMgr::getInstance().getPreviewParams(m_i4Slave, _a_rAEOutput_main2);
    IAeMgr::getInstance().getCurrentPlineTable(m_i4Master, strPreviewAEPlineTableMain, strCaptureAEPlineTabMain, strStrobeAEPlineTableMain, strStrobeAFPlineTabMain);
    IAeMgr::getInstance().getCurrentPlineTable(m_i4Slave, strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2, strStrobeAEPlineTableMain2, strStrobeAFPlineTabMain2);
    IAeMgr::getInstance().getLCEPlineInfo(m_i4Master, rLCEInfoMain);
    IAeMgr::getInstance().getLCEPlineInfo(m_i4Slave, rLCEInfoMain2);
    m_fgMasterAeStable = IAeMgr::getInstance().IsAEStable(m_i4Master);
    m_fgSlaveAeStable  = IAeMgr::getInstance().IsAEStable(m_i4Slave);
    u4AEConditionMain = ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_BACKLIGHT)) == MTRUE ? AE_CONDITION_BACKLIGHT : 0 ) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_OVEREXPOSURE)) == MTRUE ? AE_CONDITION_OVEREXPOSURE : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_HIST_STRETCH)) == MTRUE ? AE_CONDITION_HIST_STRETCH : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_SATURATION_CHECK)) == MTRUE ? AE_CONDITION_SATURATION_CHECK : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_FACEAE)) == MTRUE ? AE_CONDITION_FACEAE : 0);
    u4AEConditionMain2 = ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_BACKLIGHT)) == MTRUE ? AE_CONDITION_BACKLIGHT : 0 ) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_OVEREXPOSURE)) == MTRUE ? AE_CONDITION_OVEREXPOSURE : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_HIST_STRETCH)) == MTRUE ? AE_CONDITION_HIST_STRETCH : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_SATURATION_CHECK)) == MTRUE ? AE_CONDITION_SATURATION_CHECK : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_FACEAE)) == MTRUE ? AE_CONDITION_FACEAE : 0);


    rAeSyncInput.main_ch.Input.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4Master);
    rAeSyncInput.main_ch.Input.EvSetting.u4Eposuretime = _a_rAEOutput_main.u4Eposuretime;
    rAeSyncInput.main_ch.Input.EvSetting.u4AfeGain = _a_rAEOutput_main.u4AfeGain;
    rAeSyncInput.main_ch.Input.EvSetting.u4IspGain = _a_rAEOutput_main.u4IspGain;
    rAeSyncInput.main_ch.Input.EvSetting.uIris = 0;  // Iris fix
    rAeSyncInput.main_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
    rAeSyncInput.main_ch.Input.EvSetting.uFlag = 0; // No hypersis
    rAeSyncInput.main_ch.Input.Bv = IAeMgr::getInstance().getBVvalue(m_i4Master);
    rAeSyncInput.main_ch.Input.u4AECondition = u4AEConditionMain;
    rAeSyncInput.main_ch.Input.i4DeltaBV = rLCEInfoMain.i4AEidxNext - rLCEInfoMain.i4AEidxCur;
    rAeSyncInput.main_ch.Input.u4ISO = _a_rAEOutput_main.u4RealISO;
    rAeSyncInput.main_ch.Input.u2FrameRate = _a_rAEOutput_main.u2FrameRate;
    rAeSyncInput.main_ch.Input.i2FlareOffset = _a_rAEOutput_main.i2FlareOffset;
    rAeSyncInput.main_ch.Input.i2FlareGain = _a_rAEOutput_main.i2FlareGain;
    rAeSyncInput.main_ch.Input.i2FaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4Master);
    rAeSyncInput.main_ch.Input.i4AEidxCurrent = rLCEInfoMain.i4AEidxCur;
    rAeSyncInput.main_ch.Input.i4AEidxNext = rLCEInfoMain.i4AEidxNext;
    rAeSyncInput.main_ch.Input.u4CWValue = _a_rAEOutput_main.u4CWValue;
    rAeSyncInput.main_ch.u4SyncGain = m_rAeSyncOutput.main_ch.u4SyncGain;
    rAeSyncInput.main_ch.u4ExposureMode = _a_rAEOutput_main.u4ExposureMode;   // exposure time
    rAeSyncInput.main_ch.pAETable = &strPreviewAEPlineTableMain;
    rAeSyncInput.sub_ch.Input.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4Slave);
    rAeSyncInput.sub_ch.Input.EvSetting.u4Eposuretime = _a_rAEOutput_main2.u4Eposuretime;
    rAeSyncInput.sub_ch.Input.EvSetting.u4AfeGain = _a_rAEOutput_main2.u4AfeGain;
    rAeSyncInput.sub_ch.Input.EvSetting.u4IspGain = _a_rAEOutput_main2.u4IspGain;
    rAeSyncInput.sub_ch.Input.EvSetting.uIris = 0;  // Iris fix
    rAeSyncInput.sub_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
    rAeSyncInput.sub_ch.Input.EvSetting.uFlag = 0; // No hypersis
    rAeSyncInput.sub_ch.Input.Bv = IAeMgr::getInstance().getBVvalue(m_i4Slave);
    rAeSyncInput.sub_ch.Input.u4AECondition = u4AEConditionMain2;
    rAeSyncInput.sub_ch.Input.i4DeltaBV = rLCEInfoMain2.i4AEidxNext - rLCEInfoMain2.i4AEidxCur;
    rAeSyncInput.sub_ch.Input.u4ISO = _a_rAEOutput_main.u4RealISO;
    rAeSyncInput.sub_ch.Input.u2FrameRate = _a_rAEOutput_main2.u2FrameRate;
    rAeSyncInput.sub_ch.Input.i2FlareOffset = _a_rAEOutput_main2.i2FlareOffset;
    rAeSyncInput.sub_ch.Input.i2FlareGain = _a_rAEOutput_main2.i2FlareGain;
    rAeSyncInput.sub_ch.Input.i2FaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4Slave);
    rAeSyncInput.sub_ch.Input.i4AEidxCurrent = rLCEInfoMain2.i4AEidxCur;
    rAeSyncInput.sub_ch.Input.i4AEidxNext = rLCEInfoMain2.i4AEidxNext;
    rAeSyncInput.sub_ch.Input.u4CWValue = _a_rAEOutput_main2.u4CWValue;
    rAeSyncInput.sub_ch.u4SyncGain = m_rAeSyncOutput.sub_ch.u4SyncGain;
    rAeSyncInput.sub_ch.u4ExposureMode = _a_rAEOutput_main2.u4ExposureMode;   // exposure time
    rAeSyncInput.sub_ch.pAETable = &strPreviewAEPlineTableMain2;

    m_pMTKSyncAe->SyncAeMain(&rAeSyncInput, &m_rAeSyncOutput);

    MY_LOG("[%s] -", __FUNCTION__);

    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAwbMain()
{
    MY_LOG("[%s] +", __FUNCTION__);

    SYNC_AWB_INPUT_INFO_STRUCT rAwbSyncInput;
    AWB_OUTPUT_T rAwbMaster, rAwbSlave;

    AWB_PARENT_BLK_STAT_T& rAwbParentStatMaster = rAwbMaster.rAWBSyncInput_N3D.rAwbParentStatBlk;
    AWB_PARENT_BLK_STAT_T& rAwbParentStatSlave = rAwbSlave.rAWBSyncInput_N3D.rAwbParentStatBlk;
    MINT32& iParentBlkNumX_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumY;
    MINT32& iParentBlkNumX_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumY;

    IAwbMgr::getInstance().getAWBOutput(m_i4Master, rAwbMaster);
    IAwbMgr::getInstance().getAWBOutput(m_i4Slave, rAwbSlave);

    IAwbMgr::getInstance().getAWBParentStat(m_i4Master, rAwbParentStatMaster, &iParentBlkNumX_Main, &iParentBlkNumY_Main);
    IAwbMgr::getInstance().getAWBParentStat(m_i4Slave, rAwbParentStatSlave, &iParentBlkNumX_Sub, &iParentBlkNumY_Sub);

    ::memcpy(&m_rAwbSyncInput.main_ch, &rAwbMaster.rAWBSyncInput_N3D, sizeof(AWB_SYNC_INPUT_N3D_T));
    ::memcpy(&m_rAwbSyncInput.sub_ch, &rAwbSlave.rAWBSyncInput_N3D, sizeof(AWB_SYNC_INPUT_N3D_T));

#if 0
    rAwbSyncInput.main_ch.alg_gain      = rAwbMaster.rAWBSyncInput_N3D.rAlgGain;
    rAwbSyncInput.main_ch.curr_gain     = rAwbMaster.rAWBSyncInput_N3D.rCurrentGain;
    rAwbSyncInput.main_ch.target_gain   = rAwbMaster.rAWBSyncInput_N3D.rTargetGain;
    rAwbSyncInput.main_ch.m_i4CCT       = rAwbMaster.rAWBSyncInput_N3D.i4CCT;
    rAwbSyncInput.main_ch.m_i4LightMode = rAwbMaster.rAWBSyncInput_N3D.i4LightMode;
    rAwbSyncInput.main_ch.m_i4SceneLV   = rAwbMaster.rAWBSyncInput_N3D.i4SceneLV;
    rAwbSyncInput.main_ch.rLightProb    = rAwbMaster.rAWBSyncInput_N3D.rLightProb;
    rAwbSyncInput.sub_ch.alg_gain       = rAwbSlave.rAWBSyncInput_N3D.rAlgGain;
    rAwbSyncInput.sub_ch.curr_gain      = rAwbSlave.rAWBSyncInput_N3D.rCurrentGain;
    rAwbSyncInput.sub_ch.target_gain    = rAwbSlave.rAWBSyncInput_N3D.rTargetGain;
    rAwbSyncInput.sub_ch.m_i4CCT        = rAwbSlave.rAWBSyncInput_N3D.i4CCT;
    rAwbSyncInput.sub_ch.m_i4LightMode  = rAwbSlave.rAWBSyncInput_N3D.i4LightMode;
    rAwbSyncInput.sub_ch.m_i4SceneLV    = rAwbSlave.rAWBSyncInput_N3D.i4SceneLV;
    rAwbSyncInput.sub_ch.rLightProb     = rAwbSlave.rAWBSyncInput_N3D.rLightProb;
#endif
    MY_LOG("[%s] In Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
        m_rAwbSyncInput.main_ch.curr_gain.i4R, m_rAwbSyncInput.main_ch.curr_gain.i4G, m_rAwbSyncInput.main_ch.curr_gain.i4B,
        m_rAwbSyncInput.sub_ch.curr_gain.i4R, m_rAwbSyncInput.sub_ch.curr_gain.i4G, m_rAwbSyncInput.sub_ch.curr_gain.i4B);
    m_pMTKSyncAwb->SyncAwbMain(&m_rAwbSyncInput, &m_rAwbSyncOutput);
    MY_LOG("[%s] Out Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
        m_rAwbSyncOutput.main_ch.rAwbGain.i4R, m_rAwbSyncOutput.main_ch.rAwbGain.i4G, m_rAwbSyncOutput.main_ch.rAwbGain.i4B,
        m_rAwbSyncOutput.sub_ch.rAwbGain.i4R, m_rAwbSyncOutput.sub_ch.rAwbGain.i4G, m_rAwbSyncOutput.sub_ch.rAwbGain.i4B);

    MY_LOG("[%s] -", __FUNCTION__);

    return MTRUE;
}


MBOOL
Sync3AWrapper::
update(MINT32 i4Opt)
{
    MY_LOG("[%s] OPT(%d) +", __FUNCTION__, i4Opt);

    if (i4Opt & (ISync3A::E_SYNC3A_DO_AE|ISync3A::E_SYNC3A_DO_AE_PRECAP))
    {
        AE_MODE_CFG_T rAEInfoMaster, rAEInfoSlave;
        rAEInfoMaster.u4ExposureMode    = 0;   // us
        rAEInfoMaster.u4Eposuretime     = m_rAeSyncOutput.main_ch.Output.EvSetting.u4Eposuretime;
        rAEInfoMaster.u4AfeGain         = m_rAeSyncOutput.main_ch.Output.EvSetting.u4AfeGain;
        rAEInfoMaster.u4IspGain         = m_rAeSyncOutput.main_ch.Output.EvSetting.u4IspGain;
        rAEInfoMaster.u4RealISO         = m_rAeSyncOutput.main_ch.Output.u4ISO;
        rAEInfoMaster.u2FrameRate       = m_rAeSyncOutput.main_ch.Output.u2FrameRate;
        rAEInfoMaster.u4CWValue         = m_rAeSyncOutput.main_ch.Output.u4CWValue;
        rAEInfoMaster.i2FlareGain       = m_rAeSyncOutput.main_ch.Output.i2FlareGain;
        rAEInfoMaster.i2FlareOffset     = m_rAeSyncOutput.main_ch.Output.i2FlareOffset;
        rAEInfoSlave.u4ExposureMode     = 0;   // us
        rAEInfoSlave.u4Eposuretime      = m_rAeSyncOutput.sub_ch.Output.EvSetting.u4Eposuretime;
        rAEInfoSlave.u4AfeGain          = m_rAeSyncOutput.sub_ch.Output.EvSetting.u4AfeGain;
        rAEInfoSlave.u4IspGain          = m_rAeSyncOutput.sub_ch.Output.EvSetting.u4IspGain;
        rAEInfoSlave.u4RealISO          = m_rAeSyncOutput.sub_ch.Output.u4ISO;
        rAEInfoSlave.u2FrameRate        = m_rAeSyncOutput.sub_ch.Output.u2FrameRate;
        rAEInfoSlave.u4CWValue          = m_rAeSyncOutput.sub_ch.Output.u4CWValue;
        rAEInfoSlave.i2FlareGain        = m_rAeSyncOutput.sub_ch.Output.i2FlareGain;
        rAEInfoSlave.i2FlareOffset      = m_rAeSyncOutput.sub_ch.Output.i2FlareOffset;

        if (i4Opt & ISync3A::E_SYNC3A_DO_AE)
        {
            IAeMgr::getInstance().updatePreviewParams(m_i4Master, rAEInfoMaster, m_rAeSyncOutput.main_ch.Output.i4AEidxNext);
            IAeMgr::getInstance().updatePreviewParams(m_i4Slave, rAEInfoSlave, m_rAeSyncOutput.sub_ch.Output.i4AEidxNext);
        }
        if (i4Opt & ISync3A::E_SYNC3A_DO_AE_PRECAP)
        {
            IAeMgr::getInstance().updateCaptureParams(m_i4Master, rAEInfoMaster);
            IAeMgr::getInstance().updateCaptureParams(m_i4Slave, rAEInfoSlave);
        }
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        AWB_SYNC_OUTPUT_N3D_T rAwbSyncMaster, rAwbSyncSlave;
        rAwbSyncMaster.rAWBGain = m_rAwbSyncOutput.main_ch.rAwbGain;
        rAwbSyncMaster.i4CCT    = m_rAwbSyncOutput.main_ch.i4CCT;
        rAwbSyncSlave.rAWBGain  = m_rAwbSyncOutput.sub_ch.rAwbGain;
        rAwbSyncSlave.i4CCT     = m_rAwbSyncOutput.sub_ch.i4CCT;

        IAwbMgr::getInstance().applyAWB(m_i4Master, rAwbSyncMaster);
        IAwbMgr::getInstance().applyAWB(m_i4Slave, rAwbSyncSlave);
    }

    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
setDebugInfo(void* prDbgInfo) const
{
    MY_LOG("[%s] +", __FUNCTION__);

    N3D_DEBUG_INFO_T* prN3dDbgInfo = reinterpret_cast<N3D_DEBUG_INFO_T*>(prDbgInfo);
    if (!prN3dDbgInfo)
    {
        MY_ERR("prN3dDbgInfo is NULL!");
        return MFALSE;
    }
    m_pMTKSyncAe->SyncAeFeatureCtrl(SYNC_AE_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAEDebugInfo, NULL);
    m_pMTKSyncAwb->SyncAwbFeatureCtrl(SYNC_AWB_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAWBDebugInfo, NULL);

    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}
/******************************************************************************
 *  Sync3A
 ******************************************************************************/
class Sync3A : public ISync3A
{
public:
    static Sync3A*              getInstance(MINT32 i4Id);

    virtual MBOOL               init(MINT32 i4Policy, MINT32 i4Master, MINT32 i4Slave, const char* strName);

    virtual MBOOL               uninit();

    virtual MINT32              sync(MINT32 i4Sensor, MINT32 i4Param);

    virtual MVOID               syncEnable(MBOOL fgOnOff);

    virtual MBOOL               isSyncEnable() const;

    virtual MINT32              getFrameCount() const {return m_i4SyncFrmCount;}

    virtual MBOOL               chkMagicNum(MINT32 i4SensorDev, MINT32 i4MagicNum);

    virtual MVOID               setChkSyncEnable(MBOOL fgOnOff){m_fgChkSyncEnable = fgOnOff;}

    virtual MBOOL               getChkSyncEnable() const {return m_fgChkSyncEnable;}

    virtual MBOOL               isAeStable() const {return m_pSync3AWrap->isAeStable();}

    virtual MBOOL               setDebugInfo(void* prDbgInfo) const {return m_pSync3AWrap->setDebugInfo(prDbgInfo);}
protected:
    Sync3A();
    virtual ~Sync3A();

    MBOOL                       wait(MINT32 i4Sensor);
    MBOOL                       post(MINT32 i4Sensor);
    MBOOL                       resetScheduler();

    sem_t                       m_SemPairReady;
    mutable Mutex               m_Lock;
    Condition                   m_Condition;
    MBOOL                       m_fgSyncEnable;
    MBOOL                       m_fgChkSyncEnable;
    MINT32                      m_i4Count;
    MINT32                      m_i4SyncFrmCount;
    MINT32                      m_i4OptFirstIn;
    MINT32                      m_i4Policy;
    MINT32                      m_i4Master;
    MINT32                      m_i4Slave;
    MINT32                      m_i4MasterMagic;
    MINT32                      m_i4SlaveMagic;
    std::string                 m_strName;
    Sync3AWrapper*              m_pSync3AWrap;
};

/******************************************************************************
 *  ISync3A Implementation
 ******************************************************************************/
ISync3A*
ISync3A::
getInstance(MINT32 i4Id)
{
    return Sync3A::getInstance(i4Id);
}

/******************************************************************************
 *  Sync3A Implementation
 ******************************************************************************/
Sync3A::
Sync3A()
    : m_Lock()
    , m_fgSyncEnable(MFALSE)
    , m_fgChkSyncEnable(MFALSE)
    , m_i4Count(0)
    , m_i4SyncFrmCount(0)
    , m_i4OptFirstIn(0)
    , m_i4Policy(0)
    , m_i4Master(ESensorDevId_Main)
    , m_i4Slave(ESensorDevId_MainSecond)
    , m_i4MasterMagic(0)
    , m_i4SlaveMagic(0)
    , m_pSync3AWrap(NULL)
{
}

Sync3A::
~Sync3A()
{}

Sync3A*
Sync3A::
getInstance(MINT32 i4Id)
{
    switch (i4Id)
    {
    case 1:
        {
            static Sync3A _rSync3ACap;
            return &_rSync3ACap;
        }
    default:
    case 0:
        {
            static Sync3A _rSync3A;
            return &_rSync3A;
        }
    }
}

MBOOL
Sync3A::
init(MINT32 i4Policy, MINT32 i4Master, MINT32 i4Slave, const char* strName)
{
    m_i4Count = 0;
    m_i4SyncFrmCount = 0;
    m_i4OptFirstIn = 0;
    m_i4Policy = i4Policy;
    m_i4Master = i4Master;
    m_i4Slave = i4Slave;
    m_i4MasterMagic = 0;
    m_i4SlaveMagic = 0;
    m_fgSyncEnable = MFALSE;
    m_fgChkSyncEnable = MFALSE;

    m_strName = strName;

    m_pSync3AWrap = Sync3AWrapper::getInstance();
    m_pSync3AWrap->init(i4Master, i4Slave);

    MY_LOG("[%s] %s: policy(%d), masterDev(%d), slaveDev(%d), m_pSync3AWrap(0x%08x)",
        __FUNCTION__, m_strName.c_str(), i4Policy, i4Master, i4Slave, m_pSync3AWrap);
    return MTRUE;
}

MBOOL
Sync3A::
uninit()
{
    m_fgSyncEnable = MFALSE;

    m_pSync3AWrap->uninit();
    m_pSync3AWrap = NULL;

    MY_LOG("[%s] %s: OK", __FUNCTION__, m_strName.c_str());
    return MTRUE;
}

MVOID
Sync3A::
syncEnable(MBOOL fgOnOff)
{
    Mutex::Autolock lock(m_Lock);

    if (fgOnOff == MFALSE)
    {
        if (m_i4Count < 0)
        {
            m_i4Count ++;
            m_Condition.signal();
            MY_LOG("[%s] %s: Disable Sync: post(%d)", __FUNCTION__, m_strName.c_str(), m_i4Count);
        }
    }
    MY_LOG("[%s] %s Sync OnOff(%d)", __FUNCTION__, m_strName.c_str(), fgOnOff);

    m_fgSyncEnable = fgOnOff;
}

MBOOL
Sync3A::
isSyncEnable() const
{
    return m_fgSyncEnable;
}

MINT32
Sync3A::
sync(MINT32 i4Sensor, MINT32 i4Param)
{
    MINT32 i4Ret = 0;
    MINT32 i4SemVal;

    Mutex::Autolock autoLock(m_Lock);
    i4SemVal = m_i4Count;
    if (i4SemVal >= 0)
    {
        m_i4Count --;
        MY_LOG("[%s](0) %s: eSensor(%d) Wait(%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_i4Count, i4Param);
        i4Ret = 0;
    }
    else
    {
        m_i4Count ++;
        MY_LOG("[%s](2) %s: eSensor(%d) Post(%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_i4Count, i4Param);
        i4Ret = 1;
    }

#if 1
    if (i4Ret == 0)
    {
        m_i4OptFirstIn = i4Param;
        wait(i4Sensor);
    }
    else
    {
        if ((i4Param|m_i4OptFirstIn) & ISync3A::E_SYNC3A_BYP_AE)
        {
            MY_LOG("%s: Ignore AEAWB: eSensor(%d), Opt(0x%08x), another Opt(0x%08x)", m_strName.c_str(), i4Sensor, i4Param, m_i4OptFirstIn);
            i4Param &= (~(ISync3A::E_SYNC3A_DO_AE|ISync3A::E_SYNC3A_DO_AWB));
        }
        else if (i4Param != m_i4OptFirstIn)
        {
            MY_ERR("%s: Phase Missmatch: eSensor(%d), Opt(0x%08x), another Opt(0x%08x)", m_strName.c_str(), i4Sensor, i4Param, m_i4OptFirstIn);
            resetScheduler();
            i4Param &= (~ISync3A::E_SYNC3A_DO_AE);
        }

        MINT32 i4BypSyncCalc = 0;
        GET_PROP("debug.sync2a.byp", "0", i4BypSyncCalc);

        // calculation
        MY_LOG("[%s](3) %s: Sync 2A: Sensor(%d) Calculation", __FUNCTION__, m_strName.c_str(), i4Sensor);

        // 2a sync do calculation
        MINT32 i4Opt = i4Param & (~i4BypSyncCalc);
        m_pSync3AWrap->syncCalc(i4Opt);
        post(i4Sensor);
    }
#endif

    return i4Ret;
}

MBOOL
Sync3A::
wait(MINT32 i4Sensor)
{
    MBOOL fgRet = MTRUE;
    MY_LOG("[%s](1) %s: + eSensor(%d), m_fgSyncEnable(%d)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_fgSyncEnable);
    if (m_fgSyncEnable)
    {
        m_Condition.wait(m_Lock);
    }

    if (!m_fgSyncEnable)
        fgRet = MFALSE;
    MY_LOG("[%s](5) %s: - eSensor(%d), m_fgSyncEnable(%d)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_fgSyncEnable);
    return fgRet;
}

MBOOL
Sync3A::
post(MINT32 i4Sensor)
{
    MY_LOG("[%s](4) %s: eSensor(%d)", __FUNCTION__, m_strName.c_str(), i4Sensor);
    m_i4SyncFrmCount ++;
    m_Condition.signal();
    return MTRUE;
}
MBOOL
Sync3A::
chkMagicNum(MINT32 i4SensorDev, MINT32 i4MagicNum)
{
    if (i4SensorDev == m_i4Master)
    {
        m_i4MasterMagic = i4MagicNum;
    }
    if (i4SensorDev == m_i4Slave)
    {
        m_i4SlaveMagic = i4MagicNum;
    }

    return (m_i4MasterMagic == m_i4SlaveMagic);
}

MBOOL
Sync3A::
resetScheduler()
{
    MY_LOG("[%s]", __FUNCTION__);
    Hal3A* p3AMaster = Hal3A::getInstance(m_i4Master);
    Hal3A* p3ASlave  = Hal3A::getInstance(m_i4Slave);

    p3AMaster->mpScheduler->resetCycleCnt();
    p3ASlave->mpScheduler->resetCycleCnt();

    return MTRUE;
}

/******************************************************************************
 *  ISync3AMgr Implementation
 ******************************************************************************/
class Sync3AMgr : public ISync3AMgr
{
public:
    /**
     * get singleton.
     */
    static Sync3AMgr*           getInstance();
    /**
     * Explicitly init 3A N3D Sync manager by MW.
     */
    virtual MBOOL               init(MINT32 i4Policy, MINT32 i4MasterIdx, MINT32 i4SlaveIdx);

    /**
     * Explicitly uninit 3A N3D Sync manager by MW.
     */
    virtual MBOOL               uninit();

    virtual MBOOL               isActive() const;

    virtual MINT32              getMasterDev() const {return m_i4Master;}

    virtual MINT32              getSlaveDev() const {return m_i4Slave;}

    virtual MVOID               setCaptureMode(E_SYNC3AMGR_CAPMODE eCapMode);

    virtual E_SYNC3AMGR_CAPMODE getCaptureMode() const {return m_eCapMode;}

    virtual MVOID               setAFState(MINT32 i4AfState);

    virtual MINT32              getAFState() const {return m_fgAfState;}

protected:
    Sync3AMgr()
        : m_fgIsActive(MFALSE)
        , m_fgAfState(MFALSE)
        , m_i4Master(ESensorDevId_Main)
        , m_i4Slave(ESensorDevId_MainSecond)
        , m_eCapMode(E_SYNC3AMGR_CAPMODE_2D)
        {}
    virtual ~Sync3AMgr(){}

    MBOOL                       m_fgIsActive;
    MBOOL                       m_fgAfState;
    MINT32                      m_i4Master;
    MINT32                      m_i4Slave;
    E_SYNC3AMGR_CAPMODE         m_eCapMode;
};

ISync3AMgr*
ISync3AMgr::
getInstance()
{
    return Sync3AMgr::getInstance();
}

Sync3AMgr*
Sync3AMgr::
getInstance()
{
    static Sync3AMgr _rSync3AMgr;
    return &_rSync3AMgr;
}

ISync3A*
ISync3AMgr::
getSync3A(MINT32 i4Id)
{
    return ISync3A::getInstance(i4Id);
}

MBOOL
Sync3AMgr::
init(MINT32 i4Policy, MINT32 i4MasterIdx, MINT32 i4SlaveIdx)
{
    m_fgIsActive = MTRUE;

    IHalSensorList* pHalSensorList = IHalSensorList::get();
    if (!pHalSensorList) return MFALSE;

    m_i4Master = pHalSensorList->querySensorDevIdx(i4MasterIdx);
    m_i4Slave = pHalSensorList->querySensorDevIdx(i4SlaveIdx);
    MY_LOG("[%s] Master(%d), Slave(%d)", __FUNCTION__, m_i4Master, m_i4Slave);

    ISync3A::getInstance(E_SYNC3AMGR_PRVIEW)->init(i4Policy, m_i4Master, m_i4Slave, "Preview");
    ISync3A::getInstance(E_SYNC3AMGR_CAPTURE)->init(i4Policy, m_i4Master, m_i4Slave, "Capture");

    // awb ROI for main2
    NSCamCustomSensor::SensorViewAngle_T rVangle = NSCamCustomSensor::getSensorViewAngle();


    if ((rVangle.Main2SensorVerFOV < rVangle.MainSensorVerFOV) ||
        (rVangle.Main2SensorHorFOV < rVangle.MainSensorHorFOV))
    {
        MY_LOG("[%s] Not necessary to config window MainFOV(%d,%d) Main2FOV(%d,%d)",
            __FUNCTION__, rVangle.MainSensorHorFOV, rVangle.MainSensorVerFOV, rVangle.Main2SensorHorFOV, rVangle.Main2SensorVerFOV);
    }
    else
    {
        MINT32 i;
        SensorStaticInfo rSensorStaticInfo;
        pHalSensorList->querySensorStaticInfo(m_i4Slave, &rSensorStaticInfo);
        MINT32 u4NumSensorModes = MIN(rSensorStaticInfo.SensorModeNum, (MINT32)ESensorMode_NUM);

        NSCam::MSize rSize[ESensorMode_NUM];
        rSize[ESensorMode_Preview].w    = rSensorStaticInfo.previewWidth;
        rSize[ESensorMode_Preview].h    = rSensorStaticInfo.previewHeight;
        rSize[ESensorMode_Capture].w    = rSensorStaticInfo.captureWidth;
        rSize[ESensorMode_Capture].h    = rSensorStaticInfo.captureHeight;
        rSize[ESensorMode_Video].w      = rSensorStaticInfo.videoWidth;
        rSize[ESensorMode_Video].h      = rSensorStaticInfo.videoHeight;
        rSize[ESensorMode_SlimVideo1].w = rSensorStaticInfo.video1Width;
        rSize[ESensorMode_SlimVideo1].h = rSensorStaticInfo.video1Height;
        rSize[ESensorMode_SlimVideo2].w = rSensorStaticInfo.video2Width;
        rSize[ESensorMode_SlimVideo2].h = rSensorStaticInfo.video2Height;

        MY_LOG("[%s] u4NumSensorModes(%d), prv(%d,%d), cap(%d,%d), vdo(%d,%d), vdo1(%d,%d), vdo2(%d,%d)", __FUNCTION__, u4NumSensorModes,
            rSensorStaticInfo.previewWidth,
            rSensorStaticInfo.previewHeight,
            rSensorStaticInfo.captureWidth,
            rSensorStaticInfo.captureHeight,
            rSensorStaticInfo.videoWidth,
            rSensorStaticInfo.videoHeight,
            rSensorStaticInfo.video1Width,
            rSensorStaticInfo.video1Height,
            rSensorStaticInfo.video2Width,
            rSensorStaticInfo.video2Height
            );

        for (i = 0; i < u4NumSensorModes; i++)
        {
            NSCam::MRect rCrop = get2AStatROI(
                rVangle.MainSensorVerFOV,
                rVangle.MainSensorHorFOV,
                rVangle.Main2SensorVerFOV,
                rVangle.Main2SensorHorFOV,
                rSize[i].w, rSize[i].h);

            IAwbMgr::getInstance().setAWBStatCropRegion(
                m_i4Slave,
                i,
                rCrop.p.x, rCrop.p.y, rCrop.s.w, rCrop.s.h);
        }
    }
    return MTRUE;
}

MBOOL
Sync3AMgr::
uninit()
{
    if (m_fgIsActive)
    {
        MY_LOG("[%s] Master(%d), Slave(%d)", __FUNCTION__, m_i4Master, m_i4Slave);
        ISync3A::getInstance(E_SYNC3AMGR_PRVIEW)->uninit();
        ISync3A::getInstance(E_SYNC3AMGR_CAPTURE)->uninit();
        m_fgIsActive = MFALSE;
    }
    return MTRUE;
}

MBOOL
Sync3AMgr::
isActive() const
{
    return m_fgIsActive;
}

MVOID
Sync3AMgr::
setCaptureMode(E_SYNC3AMGR_CAPMODE eCapMode)
{
    MY_LOG("[%s] eCapMode(%d)", __FUNCTION__, eCapMode);
    m_eCapMode = eCapMode;
}

MVOID
Sync3AMgr::
setAFState(MINT32 i4AfState)
{
    MY_LOG("[%s] i4AfState(%d)", __FUNCTION__, i4AfState);
    IAeMgr::getInstance().enableAEOneShotControl(m_i4Slave, (i4AfState != 0)?MTRUE:MFALSE);
    m_fgAfState = i4AfState;
}
