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
#ifndef _MTK_SYNCAWB_H
#define _MTK_SYNCAWB_H

#include "MTKSync3AType.h"
#include <camera_custom_nvram.h>
#include <camera_custom_AEPlinetable.h>
#include "MTKSyncAwbErrCode.h"
#include <awb_param.h>

//#define SYNC_AWB_DEBUG

typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4G; // G gain
    MINT32 i4B; // B gain
}AWB_GAIN_STRUCT;


typedef enum
{
    SYNC_AWB_SCENARIO_PIP,
    SYNC_AWB_SCENARIO_N3D
}SYNC_AWB_SCENARIO_ENUM;


typedef enum
{
    SYNC_AWB_STATE_STANDBY,
    SYNC_AWB_STATE_INIT,
    SYNC_AWB_STATE_PROC,
    SYNC_AWB_STATE_READY,
    SYNC_AWB_STATE_IDLE,
    SYNC_AWB_STATE_MAX
}SYNC_AWB_STATE_ENUM;


typedef enum
{
    SYNC_AWB_FEATURE_SET_PROC_INFO,
    SYNC_AWB_FEATURE_GET_DEBUG_INFO,
    SYNC_AWB_FEATURE_GET_STATE
}SYNC_AWB_FEATURE_CONTROL_ENUM;



// === Init struct ===//
typedef enum
{
    eHorizon_Light = 0,
    eA_Light,
    eTL84_Light,
    eDNP_Light,
    eD65_Light,
    eEND_Light
}eLIGHT_SOURCE_NUM;

// Light source definition
typedef enum
{
    N3D_AWB_LIGHT_STROBE = 0,                  // Strobe
    N3D_AWB_LIGHT_TUNGSTEN,                    // Tungsten
    N3D_AWB_LIGHT_WARM_FLUORESCENT,            // Warm fluorescent
    N3D_AWB_LIGHT_FLUORESCENT,                 // Fluorescent (TL84)
    N3D_AWB_LIGHT_CWF,                         // CWF
    N3D_AWB_LIGHT_DAYLIGHT,                    // Daylight
    N3D_AWB_LIGHT_SHADE,                       // Shade
    N3D_AWB_LIGHT_DAYLIGHT_FLUORESCENT,        // Daylight fluorescent
    N3D_AWB_LIGHT_NUM,                         // Light source number
    N3D_AWB_LIGHT_NONE = N3D_AWB_LIGHT_NUM,        // None: not neutral block
    //AWB_LIGHT_DONT_CARE = 0xFF             // Don't care: don't care the light source of block
} N3D_AWB_LIGHT_T;

// Light source probability
typedef struct
{
    MINT32 i4P0[N3D_AWB_LIGHT_NUM]; // Probability 0
    MINT32 i4P1[N3D_AWB_LIGHT_NUM]; // Probability 1
    MINT32 i4P2[N3D_AWB_LIGHT_NUM]; // Probability 2
    MINT32 i4P[N3D_AWB_LIGHT_NUM];  // Probability
} N3D_AWB_LIGHT_PROBABILITY_T;


typedef struct
{
    AWB_GAIN_T NormalWB_Gain[eEND_Light];
}NORMAL_GAIN_INIT_INPUT_PARAM;


typedef struct
{
    AWB_GAIN_T Golden_Gain;
    AWB_GAIN_T Unit_Gain;
}SYNC_GAIN_INIT_INPUT_PARAM;


typedef struct
{
    INT32 i4CCT[eEND_Light];
}SYNC_CTC_INIT_PARAM;


typedef enum
{
    SYNC_AWB_CCT_TH_METHOD = 0,
    SYNC_AWB_GAIN_INTERPOLATION_METHOD,
    SYNC_AWB_ADV_PP_METHOD,
    SYNC_AWB_FREE_RUN
}eSYNC_AWB_METHOD;


typedef struct
{
    MUINT32 isMasterSlaveMode; //0, 1
    MUINT32 CCTDiffTh[eEND_Light]; //5000,5000,5000,5000
    MUINT32 SyncMode;  //Choose use which kind of sync method
    MUINT32 GainRatioTh[4];

    MUINT32 PP_method_Y_threshold;
    MUINT32 PP_method_valid_block_num_ratio; // %precentage
}SYNC_AWB_TUNNING_PARAM_STRUCT;


typedef struct
{
    SYNC_AWB_SCENARIO_ENUM SyncScenario;

    AWB_NVRAM_T main_param;
    AWB_NVRAM_T sub_param;

    SYNC_AWB_TUNNING_PARAM_STRUCT SyncAwbTuningParam;
}SYNC_AWB_INIT_INFO_STRUCT;
//


//Input and Output info
typedef struct
{
    AWB_GAIN_T alg_gain;
    AWB_GAIN_T curr_gain;
    AWB_GAIN_T target_gain; //target_gain = alg_gain * output_cal_gain
    MINT32 m_i4CCT;
    MINT32 m_i4LightMode;
    MINT32  m_i4SceneLV;
    N3D_AWB_LIGHT_PROBABILITY_T rLightProb;
    AWB_GAIN_T rAwbGainNoPref; //AWB gain without preference
    AWB_PARENT_BLK_STAT_T rAwbParentStatBlk;
    MINT32 ParentBlkNumX;
    MINT32 ParentBlkNumY;
}SYNC_AWB_INPUT_PROP_STRUCT;

typedef struct
{
    AWB_GAIN_T rAwbGain;
    MINT32 i4CCT;
}SYNC_AWB_OUTPUT;


typedef struct
{
    SYNC_AWB_INPUT_PROP_STRUCT main_ch;
    SYNC_AWB_INPUT_PROP_STRUCT sub_ch;
}SYNC_AWB_INPUT_INFO_STRUCT;


typedef struct
{
    SYNC_AWB_OUTPUT main_ch;
    SYNC_AWB_OUTPUT sub_ch;
}SYNC_AWB_OUTPUT_INFO_STRUCT;
//


//Process info
typedef struct
{
    MINT32 m_i4AWBState;
    MBOOL m_bIsStrobeFired;
}SYNC_AWB_PROC_INFO_STRUCT, *P_SYNC_AWB_PROC_INFO_STRUCT;
//


class MTKSyncAwb{
public:
    static MTKSyncAwb* createInstance();
    virtual void destroyInstance();

    virtual ~MTKSyncAwb(){};

    virtual MRESULT SyncAwbInit(void* InitData);
    virtual MRESULT SyncAwbMain(SYNC_AWB_INPUT_INFO_STRUCT *pAwbSyncInput, SYNC_AWB_OUTPUT_INFO_STRUCT *pAwbSyncOutput);

    virtual MRESULT SyncAwbFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
};


#endif
