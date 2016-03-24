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

#ifndef _MTK_NR_H
#define _MTK_NR_H

#include "MTKNRType.h"
#include "MTKNRErrCode.h"

typedef enum DRVNRObject_s {
    DRV_NR_OBJ_NONE = 0,
    DRV_NR_OBJ_SW,
    DRV_NR_OBJ_IIR,
    DRV_NR_OBJ_UNKNOWN = 0xFF,
} DrvNRObject_e;

#define NR_TIME_PROF

/*****************************************************************************
    NR Define and State Machine
******************************************************************************/
#define NR_MAX_IMG_NUM          (25)
#define NR_MAX_CORE_NUM         (8)
#define NR_BUFFER_UNSET         (0xFFFF)
#define NR_BUFFER_SCALE         (6.50f)
#define NR_BUFFER_SIZE(w, h)    ((w) * (h) * NR_BUFFER_SCALE + 1048576*2)
//#define NR_DEBUG
#define NR_MULTI_CORE_OPT   (1)
#define NR_MAX_CORE_NO      (8)

/*****************************************************************************
    Performance Control
******************************************************************************/
#define NR_MAX_GEAR_NUM     (10)
static const MUINT32 NRPerfGearOption[NR_MAX_GEAR_NUM][2] =
{
    {4, 1300000},
    {4, 1170000},
    {4,  819000},
    {4, 1300000},   // reserved
    {4, 1300000},   // reserved
    {4, 1300000},   // reserved
    {4, 1300000},   // reserved
    {4, 1300000},   // reserved
    {4, 1300000},   // reserved
    {4, 1300000}    // reserved
};

/*****************************************************************************
    Process Control
******************************************************************************/
typedef enum NR_STATE_ENUM
{
    NR_STATE_STANDBY=0,            // After Create Obj or Reset
    NR_STATE_INIT,                 // After Called NRInit
    NR_STATE_PROCESS,              // After Called NRMain
    NR_STATE_PROCESS_DONE,         // After Finish NRMain
} NR_STATE_ENUM;

typedef enum MTK_NR_IMAGE_FMT_ENUM
{
    NR_IMAGE_RGB565=1,
    NR_IMAGE_BGR565,
    NR_IMAGE_RGB888,
    NR_IMAGE_BGR888,
    NR_IMAGE_ARGB888,
    NR_IMAGE_ABGR888,
    NR_IMAGE_BGRA8888,
    NR_IMAGE_RGBA8888,
    NR_IMAGE_YUV444,
    NR_IMAGE_YUV422,
    NR_IMAGE_YUV420,
    NR_IMAGE_YUV411,
    NR_IMAGE_YUV400,
    NR_IMAGE_PACKET_UYVY422,
    NR_IMAGE_PACKET_YUY2,
    NR_IMAGE_PACKET_YVYU,
    NR_IMAGE_NV21,
    NR_IMAGE_YV12,

    NR_IMAGE_RAW8=100,
    NR_IMAGE_RAW10,
    NR_IMAGE_EXT_RAW8,
    NR_IMAGE_EXT_RAW10,
    NR_IMAGE_JPEG=200
} MTK_NR_IMAGE_FMT_ENUM;

typedef enum NR_IO_ENUM
{
    NR_INPUT,
    NR_OUTPUT,
    NR_IO_NUM
} NR_IO_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    NR_FEATURE_BEGIN,              // minimum of feature id
    NR_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    NR_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    NR_FEATURE_ADD_IMG,            // feature id to set image info
    NR_FEATURE_GET_RESULT,         // feature id to get result
    NR_FEATURE_GET_LOG,            // feature id to get debugging information
    NR_FEATURE_MAX                 // maximum of feature id
}   NR_FEATURE_ENUM;

typedef struct NRTuningInfo
{
    // quality parameters
    MINT32 ANR_Y_LUMA_SCALE_RANGE;
    MINT32 ANR_C_CHROMA_SCALE;
    MINT32 ANR_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3;
    MINT32 ANR_Y_SCALE_CPY4;
    MINT32 ANR_Y_CPX1;
    MINT32 ANR_Y_CPX2;
    MINT32 ANR_Y_CPX3;
    MINT32 ANR_CEN_GAIN_LO_TH;
    MINT32 ANR_CEN_GAIN_HI_TH;
    MINT32 ANR_PTY_GAIN_TH;
    MINT32 ANR_KSIZE_LO_TH;
    MINT32 ANR_KSIZE_HI_TH;
    MINT32 ANR_KSIZE_LO_TH_C;
    MINT32 ANR_KSIZE_HI_TH_C;
    MINT32 ITUNE_ANR_PTY_STD;
    MINT32 ITUNE_ANR_PTU_STD;
    MINT32 ITUNE_ANR_PTV_STD;
    MINT32 ANR_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_BLD_TH_Y;
    MINT32 ANR_ACT_SLANT_Y;
    MINT32 ANR_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_BLD_TH_C;
    MINT32 ANR_ACT_SLANT_C;
    MINT32 RADIUS_H;
    MINT32 RADIUS_V;
    MINT32 RADIUS_H_C;
    MINT32 RADIUS_V_C;
    MINT32 ANR_PTC_HGAIN;
    MINT32 ANR_PTY_HGAIN;
    MINT32 ANR_LPF_HALFKERNEL;
    MINT32 ANR_LPF_HALFKERNEL_C;
    MINT32 ANR_ACT_MODE;
    MINT32 ANR_LCE_SCALE_GAIN;
    MINT32 ANR_LCE_C_GAIN;
    MINT32 ANR_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3;
    MINT32 ANR_MEDIAN_LOCATION;
    MINT32 ANR_CEN_X;
    MINT32 ANR_CEN_Y;
    MINT32 ANR_R1;
    MINT32 ANR_R2;
    MINT32 ANR_R3;
    MINT32 LUMA_ON_OFF;
} NRTuningInfo;

typedef struct IIRTuningInfo
{
    MINT32 NR_STRENGTH;
    MINT32 BLEND_W;
    MINT32 reserved[48];
} IIRTuningInfo;

typedef enum NR_PERF_GEAR_ENUM
{
    NR_PERF_GEAR_PERF_FIRST,    // fastest
    NR_PERF_GEAR_CUSTOM_0,      // trade-off between fastest & slowest
    NR_PERF_GEAR_POWER_FIRST,   // slowest
} NR_PERF_FEAR_ENUM;

typedef struct NRPerfGear
{
    MBOOL enable;
    NR_PERF_GEAR_ENUM option;
} NRPerfGear;

typedef struct NRInitInfo
{
    void*       pWorkingBuff;                // Working buffer start address
    MUINT32     WorkingBuffSize;
    MUINT32     CoreNumber;                 // valid value = {1 ~ NR_MAX_CORE_NUM}
    MUINT32     NumOfExecution; // profiling purpose
    union
    {
        NRTuningInfo    *pTuningInfo;
        IIRTuningInfo   *pIIRTuningInfo;
    };
    NRPerfGear  *pPerfInfo;
    MUINT32     ForcedFreq;
} NRInitInfo;

// NR_STATE_GEN_DEPTH & NR_STATE_DEFOCUS
// Input    : NRImageInfo
// Output   : NONE
typedef struct NRImageInfo
{
    void*                       pImg[NR_MAX_IMG_NUM];      // input image address array
    MUINT32                     ImgNum;                    // input image number
    MTK_NR_IMAGE_FMT_ENUM       ImgFmt;                    // input image format
    MUINT32                     Width;                     // input image width
    MUINT32                     Height;                    // input image height

} NRImageInfo;

// NR_FEATURE_GET_RESULT
// Input    : NONE
// Output   : NRResultInfo
struct NRResultInfo
{
    MUINT32     Width;                          // output image width for NR
    MUINT32     Height;                         // output image hieght for NR
    MRESULT     RetCode;                        // returned status
};
/*******************************************************************************
*
********************************************************************************/
class MTKNR {
public:
    static MTKNR* createInstance(DrvNRObject_e eobject);
    virtual void   destroyInstance(MTKNR* obj) = 0;

    virtual ~MTKNR(){}
    // Process Control
    virtual MRESULT NRInit(MUINT32 *InitInData, MUINT32 *InitOutData);    // Env/Cb setting
    virtual MRESULT NRMain(void);                                         // START
    virtual MRESULT NRReset(void);                                        // RESET

    // Feature Control
    virtual MRESULT NRFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppNRTmp : public MTKNR {
public:
    //
    static MTKNR* getInstance();
    virtual void destroyInstance(MTKNR* obj) = 0;
    //
    AppNRTmp() {};
    virtual ~AppNRTmp() {};
};

#endif

