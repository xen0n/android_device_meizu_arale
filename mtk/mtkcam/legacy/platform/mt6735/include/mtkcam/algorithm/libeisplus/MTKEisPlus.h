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
#ifndef _MTK_EIS_PLUS_H
#define _MTK_EIS_PLUS_H

#include "MTKEisPlusType.h"
#include "MTKEisPlusErrCode.h"
//#include "EisPlusCore.h"

//#define EIS_PLUS_DEBUG

#define EIS_WIN_NUM 32

typedef enum
{
    EIS_PLUS_STATE_STANDBY,
    EIS_PLUS_STATE_INIT,
    EIS_PLUS_STATE_PROC,
    EIS_PLUS_STATE_FINISH,
    EIS_PLUS_STATE_IDLE
}EIS_PLUS_STATE_ENUM;

typedef enum
{
    EIS_PLUS_PROCESS_OK,
    EIS_PLUS_PROCESS_ERROR_STATE,
    EIS_PLUS_PROCESS_WRONG_MEMORY
} EIS_PLUS_ERROR_ENUM;

typedef enum
{
    EIS_PLUS_FEATURE_BEGIN = 0,

    EIS_PLUS_FEATURE_SET_PROC_INFO,
    EIS_PLUS_FEATURE_GET_PROC_INFO,
    EIS_PLUS_FEATURE_SET_WORK_BUF_INFO,
    EIS_PLUS_FEATURE_GET_RESULT_INFO,
    EIS_PLUS_FEATURE_SET_DEBUG_INFO,
    EIS_PLUS_FEATURE_GET_EIS_STATE,
    EIS_PLUS_FEATURE_SAVE_LOG,

    EIS_PLUS_FEATURE_MAX

}    EIS_PLUS_FEATURE_ENUM;

typedef struct
{
    void* extMemStartAddr; //working buffer start address
    MUINT32 extMemSize;
} EIS_PLUS_SET_WORKING_BUFFER_STRUCT, *P_EIS_PLUS_SET_WORKING_BUFFER_STRUCT;

typedef struct
{
    MINT32 warping_mode; //0: 6 coefficient , 1: 4 coefficient, 2: 6/4 adaptive (default), 3: 2 coefficient
    MINT32 effort; //0~2, 0: high, 1:middle, 2:low
    MINT32 search_range_x;
    MINT32 search_range_y;
    MINT32 crop_ratio; //10~40
    MFLOAT stabilization_strength;
}EIS_PLUS_TUNING_PARA_STRUCT, *P_EIS_PLUS_TUNING_PARA_STRUCT;

typedef struct
{
    EIS_PLUS_TUNING_PARA_STRUCT eis_plus_tuning_data;
    MINT32* wide_angle_lens; //reserved for wide angle lens matrix
    MBOOL debug;
} EIS_PLUS_SET_ENV_INFO_STRUCT, *P_EIS_PLUS_SET_ENV_INFO_STRUCT;


typedef struct
{
    MUINT16* FE_X;
    MUINT16* FE_Y;
    MUINT16* FE_RES;
    MUINT16* FE_DES;
    MUINT8* FE_VALID;
}EIS_PLUS_FE_INFO_STRUCT, *P_EIS_PLUS_FE_INFO_STRUCT;

typedef struct
{
    MINT32 eis_gmv_conf[2];
    MFLOAT eis_gmv[2];
}EIS_PLUS_EIS_INFO_STRUCT, *P_EIS_PLUS_EIS_INFO_STRUCT;

typedef struct
{
    EIS_PLUS_FE_INFO_STRUCT fe_info;
    EIS_PLUS_EIS_INFO_STRUCT eis_info;
    MINT32 block_size;
    MINT32 imgiWidth;
    MINT32 imgiHeight;
    MINT32 CRZoWidth;
    MINT32 CRZoHeight;
    MINT32 SRZoWidth;
    MINT32 SRZoHeight;
    MINT32 oWidth;
    MINT32 oHeight;
    MINT32 TargetWidth;
    MINT32 TargetHeight;
    MINT32 cropX;
    MINT32 cropY;
    MFLOAT AcceInfo[3];
    MFLOAT GyroInfo[3];
}EIS_PLUS_SET_PROC_INFO_STRUCT, *P_EIS_PLUS_SET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT32 ext_mem_size; //working buffer size
    MUINT32                 Grid_W;                  // Grid_W = 2;
    MUINT32                 Grid_H;                  // Grid_H = 2;
}EIS_PLUS_GET_PROC_INFO_STRUCT, *P_EIS_PLUS_GET_PROC_INFO_STRUCT;


typedef struct
{
    MINT32*                  GridX;                // Grid X[Grid_W*Grid_H]
    MINT32*                  GridY;                // Grid Y[Grid_W*Grid_H]
    MUINT32                 ClipX;    // image offset X
    MUINT32                 ClipY;    // image offset Y
}EIS_PLUS_RESULT_INFO_STRUCT, *P_EIS_PLUS_RESULT_INFO_STRUCT;

class MTKEisPlus
{
public:
    static MTKEisPlus* createInstance();
    virtual void   destroyInstance(MTKEisPlus* obj) = 0;

    virtual ~MTKEisPlus(){};
    // Process Control
    virtual MRESULT EisPlusInit(void* InitInData);
    virtual MRESULT EisPlusMain(EIS_PLUS_RESULT_INFO_STRUCT *EisPlusResult);    // START
    virtual MRESULT EisPlusReset();   //Reset

    // Feature Control
    virtual MRESULT EisPlusFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
};


#endif
