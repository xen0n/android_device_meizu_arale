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

#ifndef _MTK_DNGOP_H
#define _MTK_DNGOP_H

#include "MTKDngOpType.h"
#include "MTKDngOpErrCode.h"

typedef enum DRVDngOpObject_s {
    DRV_DNGOP_OBJ_NONE = 0,
    DRV_DNGOP_PACK_OBJ_SW,
    DRV_DNGOP_UNPACK_OBJ_SW,
    DRV_DNGOP_OBJ_UNKNOWN = 0xFF,
} DrvDngOpObject_e;

/*****************************************************************************
    PACK Define and State Machine
******************************************************************************/
#define DNGOP_BUFFER_SIZE(stride, hight)    ( (stride)*(hight) )

typedef enum DNGOP_STATE_ENUM
{
    DNGOP_STATE_STANDBY=0,    // After Create Obj or Reset
    DNGOP_STATE_INIT,         // After Called PackInit
    DNGOP_STATE_PROC,         // After Called PackMain
    DNGOP_STATE_PROC_READY,   // After Finish PackMain
    DNGOP_STATE_READY         // After Finish PackMerge
} DNGOP_STATE_ENUM;

//========================================================================================
/* Debug option */
#define DUMP        // define dump intermediate data, undefine dump only error and info
//========================================================================================


/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum DNGOP_FEATURE_ENUM
{
    MTKDNGOP_FEATURE_BEGIN,
    MTKDNGOP_FEATURE_ADD_IMAGE,
    MTKDNGOP_FEATURE_GET_RESULT,
    MTKDNGOP_FEATURE_GET_LOG,
    MTKDNGOP_FEATURE_GET_WORKBUF_SIZE,
    MTKDNGOP_FEATURE_SET_WORKBUF_ADDR,
    MTKDNGOP_FEATURE_MAX
}   DNGOP_FEATURE_ENUM;

#if 0
struct DngOpInitInfo
{
    MUINT32            WorkingBuffSize;
    void            *WorkingBuffAddr;       // default size : PACK_WORKING_BUFFER_SIZE
};
#endif

// DNGOP_FEATURE_ADD_IMAGE,
// Input    : UnpackImageInfo
// Output   : NONE
struct DngOpImageInfo
{
    MUINT32     Width;                  // input image width
    MUINT32     Height;                 // input image height
    MUINT32        Stride_src;                // byte
    MUINT32        Stride_dst;                // byte
    MUINT32        BIT_NUM;                // Bit number for unpack/pack mode
    MUINT32        Bit_Depth;                // Bit per pixel in ISP, no use for unpack
    MUINT32        Buff_size;
    void     *srcAddr;
};

struct DngOpResultInfo
{
    MUINT8          ErrPattern;         // Returned error/Warning bit pattern
    MRESULT         RetCode;            // return warning
    void            *ResultAddr;
};

/*******************************************************************************
*
********************************************************************************/
class MTKDngOp {
public:
    static MTKDngOp* createInstance(DrvDngOpObject_e eobject);
    virtual void  destroyInstance(MTKDngOp* obj) = 0;

    virtual ~MTKDngOp(){}
#if 0
    // Feature Control
    virtual MRESULT DngOpFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
    // Process Control
    virtual MRESULT DngOpInit(MUINT32 *InitInData);   // Env/Cb setting
    virtual MRESULT DngOpReset(void);                 // RESET
#endif
    virtual MRESULT DngOpMain(void *pImgInfo, void *pResultInfo); // START

private:

};

class AppDngOpTmp : public MTKDngOp {
public:
    //
    static MTKDngOp* getInstance();
    virtual void destroyInstance();
    //
    AppDngOpTmp() {};
    virtual ~AppDngOpTmp() {};
};

#endif

