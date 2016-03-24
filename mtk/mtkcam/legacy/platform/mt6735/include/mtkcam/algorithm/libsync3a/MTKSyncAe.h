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
#ifndef _MTK_SYNCAE_H
#define _MTK_SYNCAE_H

#include <MTKSync3AType.h>
#include <camera_custom_nvram.h>
#include <camera_custom_AEPlinetable.h>
#include <awb_param.h>
#include <ae_param.h>

#include "MTKSyncAeErrCode.h"
//#include "camera_custom_AEPlinetable.h"
//#include "Ae_param.h"

//#define SYNC_AE_DEBUG



typedef enum
{
    SYNC_AE_SCENARIO_PIP,
    SYNC_AE_SCENARIO_N3D

}SYNC_AE_SCENARIO_ENUM;


typedef enum
{
    SYNC_AE_STEREO_QUALITY_PRIORITY = 0,
    SYNC_AE_LUMA_QUALITY_PRIORITY
}SYNC_AE_POLICY;


typedef enum
{
    SYNC_AE_STATE_STANDBY,
    SYNC_AE_STATE_INIT,
    SYNC_AE_STATE_PROC,
    SYNC_AE_STATE_READY,
    SYNC_AE_STATE_IDLE,
    SYNC_AE_STATE_MAX
}SYNC_AE_STATE_ENUM;


typedef enum
{
    SYNC_AE_FEATURE_SET_PROC_INFO,
    SYNC_AE_FEATURE_GET_DEBUG_INFO,
}SYNC_AE_FEATURE_CONTROL_ENUM;



typedef enum
{
    EXPO_TIME = 0,
    EXPO_LINE
} EXPO_TYPE;


typedef enum
{
    eN3DAE_SYNC_TV,
    eN3DAE_SYNC_ASV,
    eN3DAE_SYNC_DSV,
} SYNC_AE_SYNCMODE_ENUM;


typedef enum
{
    SYNC_AE_CAM_SCENARIO_PREVIEW,
    SYNC_AE_CAM_SCENARIO_CAPTURE
}SYNC_AE_CAM_SCENARIO_ENUM;


typedef struct
{
    strAEOutput   Input;
    MUINT32       u4SyncGain;     // AE compensation gain
    strAEOutput   PrevInput;
//  AE_NVRAM_T    *pAENVRAM;      // nvram data according to the sensor
    unsigned int  u4ExposureMode; // specify the unit of the TV (line or ms)
    strAETable    *pAETable;      // the table used to generate Input
//    strAEOutput   Output;         // the result of Input calculation
} SYNC_AE_INPUT_INFO_STRUCT;


typedef struct
{
    SYNC_AE_CAM_SCENARIO_ENUM SyncCamScenario;
    SYNC_AE_INPUT_INFO_STRUCT main_ch;
    SYNC_AE_INPUT_INFO_STRUCT sub_ch;
}SYNC_AE_INPUT_STRUCT;


typedef struct
{
    strAEOutput   Output;         // the result of Input calculation
    MUINT32       u4SyncGain;     // AE compensation gain
}SYNC_AE_OUTPUT_INFO_STRUCT;

typedef struct
{
    SYNC_AE_OUTPUT_INFO_STRUCT main_ch;
    SYNC_AE_OUTPUT_INFO_STRUCT sub_ch;
}SYNC_AE_OUTPUT_STRUCT;


typedef struct
{
    SYNC_AE_POLICY syncPolicy;
}SYNC_AE_TUNNING_PARAM_STRUCT;


typedef struct
{
    AE_NVRAM_T* pAeNvram;
}SYNC_AE_INIT_INPUT_PARAM;


typedef struct
{
    SYNC_AE_SCENARIO_ENUM SyncScenario;
    MINT32 SyncNum;
    SYNC_AE_INIT_INPUT_PARAM main_param;
    SYNC_AE_INIT_INPUT_PARAM sub_param;
    SYNC_AE_TUNNING_PARAM_STRUCT SyncAeTuningParam;
    SYNC_AE_SYNCMODE_ENUM eSyncMode;
}SYNC_AE_INIT_STRUCT;


class MTKSyncAe{
public:
    static MTKSyncAe* createInstance();
    virtual void destroyInstance() = 0;

    virtual ~MTKSyncAe(){};

    virtual MRESULT SyncAeInit(void* InitData);
    virtual MRESULT SyncAeMain(SYNC_AE_INPUT_STRUCT *pAeSyncInput, SYNC_AE_OUTPUT_STRUCT *pAeSyncOutput);

    virtual MRESULT SyncAeFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
};


#endif
