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

#define LOG_TAG "MtkCam/HwPipelineSC"
//
#include "ScenarioControl.h"
#include <bandwidth_control.h>
#include <sys/ioctl.h>
#include <camera_pipe_mgr_D2.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define DRV_DEVNAME_PIPE_MGR            "/dev/camera-pipemgr"

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
#define DUMP_SCENARIO_PARAM(_str, _param)                 \
    do{                                                   \
        MY_LOGD_IF(1, "%s: scenario %d: size %dx%d@%d",   \
                _str,                                     \
                _param.scenario,                          \
                _param.sensorSize.w, _param.sensorSize.h, \
                _param.sensorFps                          \
                );                                        \
    } while(0)


/******************************************************************************
 *
 ******************************************************************************/
BWC_PROFILE_TYPE mapToBWCProfile(ScenarioControl::ePipelineScenario const scenario)
{
    switch(scenario)
    {
        case ScenarioControl::Scenario_NormalStreaming:
            return BWCPT_CAMERA_ZSD;
        case ScenarioControl::Scenario_VideoRecording:
            return BWCPT_VIDEO_RECORD_CAMERA;
        default:
            MY_LOGE("not supported scenario %d", scenario);
    }
    return BWCPT_NONE;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IScenarioControl>
ScenarioControl::
create(MINT32 const openId)
{
    return new ScenarioControl(openId);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControl::
setScenario(
    MVOID *pBWCparam
)
{
    BWC_Scenario_Param *bwcparam = static_cast<BWC_Scenario_Param*>(pBWCparam);
    MERROR err = enterScenario(*bwcparam);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
ScenarioControl::
ScenarioControl(MINT32 const openId)
    : mOpenId(openId)
    , mbVencPllHigh(MFALSE)
{
    mFdCamPipeMgr = open(DRV_DEVNAME_PIPE_MGR, O_RDONLY, 0);
    if(mFdCamPipeMgr < 0)
        MY_LOGE("CamPipeMgr kernel open fail, errno(%d):%s",errno,strerror(errno));
}


/******************************************************************************
 *
 ******************************************************************************/
ScenarioControl::
~ScenarioControl()
{
    // reset
    if( mCurParam.scenario != Scenario_None ) {
        exitScenario();
    }
    //
    if( mFdCamPipeMgr >= 0 ) {
        close(mFdCamPipeMgr);
        mFdCamPipeMgr = -1;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControl::
enterScenario(
    BWC_Scenario_Param const & param
)
{
    if( mCurParam.scenario != Scenario_None ) {
        MY_LOGD("exit previous setting");
        exitScenario();
    }
    DUMP_SCENARIO_PARAM("enter:", param);
    //
    BWC_PROFILE_TYPE type = mapToBWCProfile(param.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
    BWC BwcIns;
    BwcIns.Profile_Change(type,true);
    //
    mmdvfs_set(
            type,
            MMDVFS_SENSOR_SIZE,         param.sensorSize.size(),
            MMDVFS_SENSOR_FPS,          param.sensorFps,
            MMDVFS_PARAMETER_EOF);
    // keep param
    mCurParam = param;
    //
    if( param.sensorSize.size() >= (3264*2448) ) {
        MERROR err = vencpllCtrl(MTRUE);
        if( err != OK )
            return err;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControl::
exitScenario()
{
    if( mCurParam.scenario == Scenario_None ) {
        MY_LOGD("already exit");
        return OK;
    }
    DUMP_SCENARIO_PARAM("exit:", mCurParam);
    BWC_PROFILE_TYPE type = mapToBWCProfile(mCurParam.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
    BWC BwcIns;
    BwcIns.Profile_Change(type,false);
    //
    // reset param
    mCurParam.scenario = Scenario_None;
    //
    {
        MERROR err = vencpllCtrl(MFALSE);
        if( err != OK )
            return err;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControl::
vencpllCtrl(MBOOL const setHigh)
{
    MERROR err = UNKNOWN_ERROR;
    if( mFdCamPipeMgr < 0 ) {
        MY_LOGE("fd < 0, %s not opened", DRV_DEVNAME_PIPE_MGR);
        goto lbExit;
    }
    //
    if( setHigh )
    {
        if( mbVencPllHigh )
            return OK;
        //
        MY_LOGD("set high");
        //
        CAM_PIPE_MGR_CMD_VECNPLL_CTRL_ENUM ctrlEnum = CAM_PIPE_MGR_CMD_VECNPLL_CTRL_SET_HIGH;
        if(ioctl(mFdCamPipeMgr, CAM_PIPE_MGR_VENCPLL_CTRL, &ctrlEnum) < 0) {
            MY_LOGE("CAM_PIPE_MGR_VENCPLL_CTRL set high fail, errno(%d):%s",errno,strerror(errno));
            goto lbExit;
        }
        //
        mbVencPllHigh = MTRUE;
    }
    else
    {
        if( !mbVencPllHigh )
            return OK;
        //
        MY_LOGD("set low");
        //
        CAM_PIPE_MGR_CMD_VECNPLL_CTRL_ENUM ctrlEnum = CAM_PIPE_MGR_CMD_VECNPLL_CTRL_SET_LOW;
        if(ioctl(mFdCamPipeMgr, CAM_PIPE_MGR_VENCPLL_CTRL, &ctrlEnum) < 0) {
            MY_LOGE("CAM_PIPE_MGR_VENCPLL_CTRL set high fail, errno(%d):%s",errno,strerror(errno));
            goto lbExit;
        }
        //
        mbVencPllHigh = MFALSE;
    }
    //
    err = OK;
lbExit:
    return err;
}
