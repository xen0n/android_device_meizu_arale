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

#ifndef _MTK_MFBLL_H
#define _MTK_MFBLL_H

#include "MTKMfbllType.h"
#include "MTKMfbllErrCode.h"

typedef enum DRVMfbllObject_s
{
    DRV_MFBLL_OBJ_NONE = 0,
    DRV_MFBLL_OBJ_SW,
    DRV_MFBLL_OBJ_SW_NEON,
    DRV_MFBLL_OBJ_UNKNOWN = 0xFF,
} DrvMfbllObject_e;

/*****************************************************************************
    Main Module
******************************************************************************/
typedef enum MFBLL_PROC_ENUM
{
    MFBLL_PROC1 = 0,
    MFBLL_UNKNOWN_PROC,
} MFBLL_PROC_ENUM;

typedef enum MFBLL_STATE_ENUM
{
    MFBLL_STATE_IDLE=0,
    MFBLL_STATE_STANDBY,
    MFBLL_STATE_INIT,
    MFBLL_STATE_PROC1_READY,
    MFBLL_STATE_PROC2_READY,
    MFBLL_STATE_PROC1,
    MFBLL_STATE_PROC2,
    MFBLL_STATE_MAX,
} MFBLL_STATE_ENUM;

typedef enum MFBLL_FTCTRL_ENUM
{
    MFBLL_FTCTRL_GET_RESULT,
    MFBLL_FTCTRL_SET_PTHREAD_ATTR,
    MFBLL_FTCTRL_GET_LOG,
    MFBLL_FTCTRL_GET_PROC_INFO,
    MFBLL_FTCTRL_SET_PROC1_INFO,
    MFBLL_FTCTRL_MAX
}    MFBLL_FTCTRL_ENUM;

/*****************************************************************************
  MFBLL INIT
******************************************************************************/
typedef struct
{
    MUINT32 core_num;
    MUINT32 ISO_Value;
    MUINT32 ISO_HiTh;
    MUINT32 ISO_LowTh;
    MUINT32 MaxFrmNum;
}PROC1_TUNING_PARA_STRUCT;

typedef struct
{
    MUINT16 Proc1_imgW;
    MUINT16 Proc1_imgH;
    PROC1_TUNING_PARA_STRUCT Proc1_tuning;
} PROC1_INIT_PARAM_STRUCT,*P_PROC1_INIT_PARAM_STRUCT;

typedef struct
{
    PROC1_INIT_PARAM_STRUCT Proc1_Init;
} MFBLL_INIT_PARAM_STRUCT,*P_MFBLL_INIT_PARAM_STRUCT;



/*****************************************************************************
  PROC1 PART (Type definition should be exactly same as that in core Proc1)
******************************************************************************/
typedef enum PROC1_IMAGE_FORMAT
{
    PROC1_FMT_YV16 = 0,
    PROC1_FMT_YUY2,
    PROC1_FMT_MAX           // maximum image format enum
}    PROC1_IMAGE_FORMAT;

typedef struct
{
    MUINT8 *ResultImg;
    MUINT32  Width;
    MUINT32  Height;
    PROC1_IMAGE_FORMAT ImgFmt;
    MUINT32 bad_mb_count;
    MUINT8   bSkip_MEMC;
} MFBLL_PROC1_OUT_STRUCT,*P_MFBLL_PROC1_OUT_STRUCT;

typedef struct
{
    MUINT32 Ext_mem_size;
    MUINT32 Mfbll_frame_num;
} MFBLL_GET_PROC_INFO_STRUCT, *P_MFBLL_GET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT8  *base;
    MUINT8  *ref;
    MUINT32     Width;
    MUINT32  Height;
    MUINT8  *workbuf_addr;
    MUINT32  buf_size;
    MINT32   Proc1_V1;
    MINT32   Proc1_V2;
    //P_MFBLL_EIS_DATA_STRUCT pEISData;
    MUINT8   bad_mv_range;
    MUINT32  bad_mv_TH;
    PROC1_IMAGE_FORMAT ImgFmt;
} MFBLL_SET_PROC1_INFO_STRUCT, *P_MFBLL_SET_PROC1_INFO_STRUCT;

/*******************************************************************************
*
********************************************************************************/
class MTKMfbll
{
public:
    static MTKMfbll* createInstance(DrvMfbllObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKMfbll(){}
    virtual MRESULT MfbllInit(void* pParaIn, void* pParaOut);
    virtual MRESULT MfbllReset(void);
    virtual MRESULT MfbllMain(MFBLL_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT MfbllFeatureCtrl(MFBLL_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppMfbllTmp : public MTKMfbll
{
public:

    static MTKMfbll* getInstance();
    virtual void destroyInstance();

    AppMfbllTmp() {};
    virtual ~AppMfbllTmp() {};
};
#endif

