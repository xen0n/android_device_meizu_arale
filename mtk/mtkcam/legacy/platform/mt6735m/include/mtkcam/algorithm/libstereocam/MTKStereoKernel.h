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
#ifndef _MTK_STEREO_KERNEL_H_
#define _MTK_STEREO_KERNEL_H_

#include "MTKStereoKernelScenario.h"
#include "MTKUtilType.h"
#include "MTKStereoKernelErrCode.h"
//#include "dbg_cam_param.h"      // For DEBUG_CAM_MF_MID. It also includes "dbg_cam_mf_param.h (for DEBUG_MF_INFO_T)".

#define MTK_STEREO_KERNEL_NVRAM_LENGTH      (16*12*10*3+40)  // xSlots * ySlots * Level * dataLens + others(refDAC/...) = 5800
#define MAX_FRAME_NUM                       5
#define CAPTURE_MAX_FRAME_NUM               1

typedef enum
{
    STEREO_KERNEL_FEATURE_BEGIN = 0,
    STEREO_KERNEL_FEATURE_GET_RESULT,
    STEREO_KERNEL_FEATURE_GET_STATUS,
    STEREO_KERNEL_FEATURE_SAVE_LOG,

    STEREO_KERNEL_FEATURE_SET_PROC_INFO,

    STEREO_KERNEL_FEATURE_GET_WORK_BUF_INFO,
    STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO,

    STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING,

    STEREO_KERNEL_FEATURE_MAX
}
STEREO_KERNEL_FEATURE_ENUM;

/////////////////////////////
//    For Tuning Benson ??
/////////////////////////////
typedef struct
{
    MUINT8  conv_min_deg_h      ;   // 1~5, default 3
    MUINT8  conv_max_deg_h      ;   // 1~5, default 2
    MINT8   conv_def_position   ;   // -128~127, default -60, for N3D

    MUINT8  alg_color           ;   // 0: none, 1: weak, 2: normal, 3: strong, default 3
    MUINT8  cc_thr              ;   // 0~100, default 85
    MINT8   cc_protect_gap      ;   // -127~128, default -3

    MUINT8  learn_tolerance     ;   // 0~100, default 20
}
STEREO_KERNEL_TUNING_PARA_STRUCT, *P_STEREO_KERNEL_TUNING_PARA_STRUCT ;

/////////////////////////////
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
}
STEREO_KERNEL_COORD_REMAP_INFO_STRUCT ;

typedef struct
{
    STEREO_KERNEL_SCENARIO_ENUM scenario ;

    // MAIN CAMERA IMAGE CAPTURE RESOLUTION
    MUINT16 main_source_image_width  ; // for IMAGE REFOCUS
    MUINT16 main_source_image_height ;
    MUINT16 main_crop_image_width  ;
    MUINT16 main_crop_image_height ;

    // ALGORITHM INPUT and SbS OUTPUT
    MUINT16 algo_source_image_width  ;
    MUINT16 algo_source_image_height ;
    MUINT16 algo_crop_image_width    ; // for IMAGE REFOCUS/N3D
    MUINT16 algo_crop_image_height   ;

    // HWFE INPUT - the actual size for HWFE (after SRZ)
    MUINT16 fefm_image_width    ;
    MUINT16 fefm_image_height   ;

    // COLOR CORRECTION INPUT
    MUINT16 rgba_image_width    ;
    MUINT16 rgba_image_height   ;
    MUINT16 rgba_image_stride   ;//Benson ??

    MUINT8  hwfe_block_size     ;

    // Learning
    MUINT32 *learning_data      ;   // On-Line Learning global data
    MUINT32 *learning_data_af   ;    //Benson learning_data & ??
    MUINT8  main_cam_fov        ;   // default 65
    MFLOAT  stereo_baseline     ;   // default 32mm Benson ??
    MUINT16 dac_convert_info[6] ;   // for convert dac to disparity or distance
    MUINT16 af_dac_start        ;   // Start Current Benson ?? DAC Ref

    // WARPING / CROPING
    MUINT8 enable_cc           ;
    MUINT8 enable_gpu          ;
    MUINT8 warp_index          ;    // 0: LEFT, 1: RIGHT

    // Learning Coordinates RE-MAPPING
    STEREO_KERNEL_COORD_REMAP_INFO_STRUCT remap_main ;
    STEREO_KERNEL_COORD_REMAP_INFO_STRUCT remap_minor ;

    // OUTPUT after Initialization
    MUINT32 working_buffer_size ;
    STEREO_KERNEL_TUNING_PARA_STRUCT *ptuning_para;

    void* eglDisplay;
    void* InputGB;
    void* OutputGB;
    MUINT32 cl_correction;

/*
    // if COORDINATE REMAP can be done in the firmware level
    // from HWFE -> LEARNING COORDINATES (Main Pixel Array)
    MUINT16 remap_width ;
    MUINT16 remap_height ;
    MUINT16 main_remap_offset_x ;
    MUINT16 main_remap_offset_y ;
    MUINT16 minor_remap_offset_x ;
    MUINT16 minpr_remap_offset_y ;
*/


}
STEREO_KERNEL_SET_ENV_INFO_STRUCT ;

typedef struct
{
    MUINT32 ext_mem_size;
    MUINT32* ext_mem_start_addr; //working buffer start address
}
STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT, *P_STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT ;

typedef struct
{
    // IMAGE CAPTURE
    //MUINT32 algo_source_image_left_addr ;     // No use (image with smaller view angle)
    MUINT8* algo_source_image_right_addr ;      // for Warping (image with large view angle)
    MUINT8* warped_image_addr ;                 // for output warped image

    MUINT8* rgba_image_left_addr  ;             // for Photometric Correction
    MUINT8* rgba_image_right_addr ;             // for Photometric Correction

    STEREO_KERNEL_GRAPHIC_BUFFER_STRUCT src_gb; //soruce graphic buffer
    STEREO_KERNEL_GRAPHIC_BUFFER_STRUCT dst_gb; //output graphic buffer

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
}
STEREO_KERNEL_SET_PROC_INFO_STRUCT, *P_STEREO_KERNEL_SET_PROC_INFO_STRUCT;

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
}
STEREO_KERNEL_RESULT_STRUCT, *P_STEREO_KERNEL_RESULT_STRUCT ;

/*
    CLASS
*/
class MTKStereoKernel{
public:
    static MTKStereoKernel* createInstance();
    virtual void   destroyInstance() = 0;

    virtual ~MTKStereoKernel(){};
    // Process Control
    virtual MRESULT StereoKernelInit(void* InitInData);
    virtual MRESULT StereoKernelMain(); // START
    virtual MRESULT StereoKernelReset();

    // Feature Control
    virtual MRESULT StereoKernelFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

    // N3D AC Debug Info.
    // For N3D AC algo. to set debug tag.
    /*
    inline void setDebugTag(DEBUG_MF_INFO_T &N3dAcDebugInfo, MINT32 i4ID, MINT32 i4Value)
    {
        N3dAcDebugInfo.Tag[i4ID].u4FieldID = CAMTAG(DEBUG_CAM_MF_MID, i4ID, 0);
        N3dAcDebugInfo.Tag[i4ID].u4FieldValue = i4Value;
    }
    */
    // For N3D HAL to get debug info.
    //virtual MRESULT getDebugInfo(DEBUG_MF_INFO_T &rN3dAcDebugInfo);

private:

};


#endif
