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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MTK_MFBMM_H
#define _MTK_MFBMM_H

#include "MTKStereoDepthType.h"
#include "MTKStereoDepthErrCode.h"

/*****************************************************************************
    GLOBAL CONSTANTS
******************************************************************************/
// NVRAM size
#define MAX_AF_QTBL_LENGTH (8)

typedef enum DRVStereoDepthObject_s
{
    DRV_STEREODEPTH_OBJ_NONE = 0,
    DRV_STEREODEPTH_OBJ_SW,
    DRV_STEREODEPTH_OBJ_SW_NEON,
    DRV_STEREODEPTH_OBJ_UNKNOWN = 0xFF,
} DrvStereoDepthObject_e;

/*****************************************************************************
    Main Module
******************************************************************************/
typedef enum STEREODEPTH_PROC_ENUM
{
    STEREODEPTH_LEARNING_PROC = 0,
    STEREODEPTH_QUERYING_PROC,
    STEREODEPTH_UNKNOWN_PROC,
} STEREODEPTH_PROC_ENUM;

typedef enum STEREODEPTH_STATE_ENUM
{
    STEREODEPTH_STATE_IDLE=0,
    STEREODEPTH_STATE_STANDBY,
    STEREODEPTH_STATE_INIT,
    STEREODEPTH_STATE_PROC,
} STEREODEPTH_STATE_ENUM;


typedef enum STEREODEPTH_FTCTRL_ENUM
{
    STEREODEPTH_FTCTRL_GET_RESULT,
    STEREODEPTH_FTCTRL_GET_LOG,
    STEREODEPTH_FTCTRL_GET_WORKBUF_SIZE,
    STEREODEPTH_FTCTRL_GET_AF_QTBL,
    STEREODEPTH_FTCTRL_GET_DEPTH_LEARNING_TBL,
    STEREODEPTH_FTCTRL_SET_PROC_PARAM,
    STEREODEPTH_FTCTRL_MAX
}    STEREODEPTH_FTCTRL_ENUM;

/*****************************************************************************
    MODULE Structure
******************************************************************************/
typedef struct
{
    MINT32 img_width;
    MINT32 img_height;
} STEREODEPTH_PARAM_STRUCT;


// tuning structure


typedef struct
{
    MUINT16 pixel_array_width ; //Sensor Pixel Array (for Img Center calculation)
    MUINT16 pixel_array_height ;
    MUINT16 sensor_offset_x ;
    MUINT16 sensor_offset_y ;
    MUINT16 sensor_binning ;
    MUINT16 rrz_offset_x ;
    MUINT16 rrz_offset_y ;
    MUINT16 rrz_step_width ;
    MUINT16 rrz_step_height ;
    MUINT16 rrz_out_width ;
    MUINT16 rrz_out_height ;
} STEREODEPTH_COORD_REMAP_INFO_STRUCT ;

typedef struct
{
    MUINT32 frame_count ;
    MUINT16 dac ;
    MUINT16 valid ;
    MUINT16 confi_val ;
} STEREODEPTH_AF_IN_QTBL_STRUCT ;

typedef struct
{
    // HWFE
    MUINT16 *hwfe_data_left  ;                  // Data array for Hardware Feature Extraction, Left  Image
    MUINT16 *hwfe_data_right ;                  // Data array for Hardware Feature Extraction, Right Image


    // AF INFO.
    MUINT16 af_dac_index ;
    MUINT8  af_valid ;
    MFLOAT  af_confidence ;
    MUINT16 af_win_start_x_remap ;   // algorithm input coordinates
    MUINT16 af_win_start_y_remap ;
    MUINT16 af_win_end_x_remap ;
    MUINT16 af_win_end_y_remap ;
} STEREODEPTH_PROC_INPARAM1_STRUCT, *P_STEREODEPTH_PROC_INPARAM1_STRUCT;

typedef struct
{
    // Image Capture Crop
    MUINT32  algo_left_offset_x ;
    MUINT32  algo_left_offset_y ;
    MUINT32  algo_right_offset_x ;
    MUINT32  algo_right_offset_y ;

    // Image Refocus
    MUINT32  main_offset_x ;
    MUINT32  main_offset_y ;

    // HWFE MATCHING
    MUINT16 num_hwfe_match ;
    MFLOAT *hwfe_match_data ;
    MUINT16 len_coord_trans ;
    MFLOAT *coord_trans_para ; // status[1] currentDac[1] hmtx[8] smtx[4](dacStart=dacRef slope u v)

    MUINT8  is_finish_learning ;  // For informing Learning User-Guide
} STEREODEPTH_PROC_INPARAM2_STRUCT, *P_STEREODEPTH_PROC_INPARAM2_STRUCT;

typedef struct
{
    STEREODEPTH_COORD_REMAP_INFO_STRUCT sd_remap_win;
    STEREODEPTH_AF_IN_QTBL_STRUCT sd_af_in_tbl[MAX_AF_QTBL_LENGTH];
    STEREODEPTH_PROC_INPARAM1_STRUCT sd_inparam1;
    STEREODEPTH_PROC_INPARAM2_STRUCT sd_inparam2;
} STEREODEPTH_PROC_INPARAM_STRUCT, *P_STEREODEPTH_PROC_INPARAM_STRUCT;

typedef struct
{
    MUINT32 disparity ;
    MUINT16 dac ;
    MUINT16 object_distance;
    MUINT16 conf_val;
} STEREODEPTH_DEPTH_STRUCT ;

typedef struct
{
    MUINT32 frame_count ;
    MUINT32 result_index ;


} STEREODEPTH_AF_OUT_TBL_STRUCT ;

typedef struct
{
    MUINT32 depth_lut_length ;
    STEREODEPTH_DEPTH_STRUCT *depth_lut ;
    STEREODEPTH_AF_OUT_TBL_STRUCT *depth_af_out_tbl;
} STEREODEPTH_PROC_OUTPARAM_STRUCT, *P_STEREODEPTH_PROC_OUTPARAM_STRUCT;


/*******************************************************************************
*
********************************************************************************/
class MTKStereoDepth
{
public:
    static MTKStereoDepth* createInstance(DrvStereoDepthObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKStereoDepth(){}
    virtual MRESULT StereoDepthInit(void* pParaIn, void* pParaOut);
    virtual MRESULT StereoDepthReset(void);
    virtual MRESULT StereoDepthMain(STEREODEPTH_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT StereoDepthFeatureCtrl(STEREODEPTH_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppStereoDepthTmp : public MTKStereoDepth
{
public:

    static MTKStereoDepth* getInstance();
    virtual void destroyInstance();

    AppStereoDepthTmp() {};
    virtual ~AppStereoDepthTmp() {};
};
#endif

