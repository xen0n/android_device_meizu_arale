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


#ifndef _MTK_FACEBEAUTY_H
#define _MTK_FACEBEAUTY_H
#define MAXFACENUM 15
#define VFB_BUFFER_NUM 3

#include "MTKVideoFaceBeautyType.h"
#include "MTKVideoFaceBeautyErrCode.h"
#include "MTKFeatureSet.h"
//#include "pthread.h"

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
    MTK_VIDEO_FACE_BEAUTY_FEATURE_BEGIN,

    MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_WORK_BUF_INFO,
    MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_WORK_BUF_INFO,
    MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_WARP_GRID_INFO,

    MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_PROC_INFO,

    MTK_VIDEO_FACE_BEAUTY_FEATURE_SET_TUNING_PARA,

    MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_RESULT,

    MTK_VIDEO_FACE_BEAUTY_FEATURE_GET_LOG,

    MTK_VIDEO_FACE_BEAUTY_FEATURE_MAX
}    MTK_VIDEO_FACE_BEAUTY_FEATURE_ENUM;

typedef enum
{
    MTK_VIDEO_FACE_BEAUTY_IMAGE_YUV422,
    MTK_VIDEO_FACE_BEAUTY_IMAGE_YUV420,
    MTK_VIDEO_FACE_BEAUTY_IMAGE_MAX
} MTK_VIDEO_FACE_BEAUTY_IMAGE_FORMAT_ENUM;

typedef enum
{
    MTK_VIDEO_FACE_BEAUTY_CAMERA_OFF,
    MTK_VIDEO_FACE_BEAUTY_PREVIEW,
    MTK_VIDEO_FACE_BEAUTY_RECORD
} MTK_VIDEO_FACE_BEAUTY_SCENARIO;

typedef struct
{
    MINT32 SmoothLevel;
    MINT32 BrightLevel;
    MINT32 RuddyLevel;
    MINT32 EnlargeEyeLevel; //Set warp_level = EnlargeEyeLevel @MTKWarp
    MINT32 SlimFaceLevel;

    MINT32 GlobalFaceNum; //for multiple face pkg
    MINT32 MinFaceRatio;
    MBOOL  ExtremeBeauty; //1:enable warp, 1 face only; 0:no warp, multiple face
    MINT32 SortFaceWei; // 1~9, 1:  face size, 9: position
    MINT32 TemporalSmLevel; // 1~9, 1: fast, 9: slow
    MINT32 Timeout; //default 50 frames

    MINT32 TouchX; //Touch FB face postion, (x,y)
    MINT32 TouchY;

}MTKVideoFaceBeautyTuningParaStruct;

typedef struct
{
    MUINT32 Features;                                // Current feature combination
    MUINT32 feature_order[MTK_MAX_FEATURE_NUM];

    //pthread_attr_t pthreadAttr;//thread priority and policy
    MINT32 thread_policy;
        MINT32 thread_priority;

        MUINT16  SrcImgWidth;
    MUINT16  SrcImgHeight;

    MUINT16  DsImgWidth;
    MUINT16  DsImgHeight;
    MUINT16  FDWidth;
    MUINT16  FDHeight;
    MTK_VIDEO_FACE_BEAUTY_SCENARIO   senario;
    //MTK_VIDEO_FACE_BEAUTY_IMAGE_FORMAT_ENUM SrcImgFormat; //should be 420
    MTK_VIDEO_FACE_BEAUTY_IMAGE_FORMAT_ENUM DsImgFormat;  //should be 422

}MTKVideoFaceBeautyEnvInfoStruct;

typedef struct
{
    MUINT32 ext_mem_size;
    void* ext_mem_start_addr; //working buffer start address
}MTKVideoFaceBeautyWorkBufInfoStruct;

typedef struct
{
    MUINT32 WarpGridWidth;
    MUINT32 WarpGridHeight;
}MTKVideoFaceBeautyWarpGridInfoStruct;

typedef struct
{
    //Input
    MUINT8* DsImgAddr;
    MINT32     FDLeftTopPointX1[MAXFACENUM];
    MINT32     FDLeftTopPointY1[MAXFACENUM];
    MINT32     FDBoxSize[MAXFACENUM];
    MINT32     FDPose[MAXFACENUM];
    MINT32  FaceCount;
    MTKVideoFaceBeautyTuningParaStruct *pTuningPara; //Tuning para.
    //EIS
    //old version
    MINT32 ClipWidth;
    MINT32 ClipHeight;
    MINT32 ClipStartX;//dummy
    MINT32 ClipStartY;//dummy
    MFLOAT Hmtx[9];//dummy
    //new version
    MINT32 DsClipWidth;
    MINT32 DsClipHeight;
    MUINT32 PreWarpMapSize[2]; //EIS warp map size, 0:X, 1:Y
    MINT32* PreWarpMapX; //EIS warp map
    MINT32* PreWarpMapY; //EIS warp map

    MUINT16  SrcImgWidth;
    MUINT16  SrcImgHeight;
    MUINT16  DsImgWidth;
    MUINT16  DsImgHeight;
    MUINT16  FDWidth;
    MUINT16  FDHeight;

    //Output
    MUINT8* WarpedDsImage;
    MUINT8* AlphaCL;
    MUINT8* AlphaNR;
    MINT32* PCATable;
    MINT32* WarpMapX; //to GPU local warp
    MINT32* WarpMapY; //to GPU loacl warp

}MTKVideoFaceBeautyProcInfoStruct;

typedef struct
{
    MUINT8* WarpedDsImage;
    MUINT8* AlphaCL;
    MUINT8* AlphaNR;
    MINT32* PCATable;
    MINT32* WarpMapX; //to GPU local warp
    MINT32* WarpMapY; //to GPU loacl warp

    //pass to CFB for VFB & CFB sync
    MUINT32 fb_pos[MAXFACENUM][2];
    MUINT16  FDWidth;
    MUINT16  FDHeight;
}MTKVideoFaceBeautyResultInfoStruct;

class MTKVideoFaceBeauty {
public:
    static MTKVideoFaceBeauty* createInstance();
    virtual void   destroyInstance(MTKVideoFaceBeauty* obj) = 0;

    virtual ~MTKVideoFaceBeauty(){}
    // Process Control
    virtual MRESULT VideoFaceBeautyInit(void *InitInData, void *InitOutData);
    virtual MRESULT VideoFaceBeautyMain(void);
    virtual MRESULT VideoFaceBeautyReset(void);

    // Feature Control
    virtual MRESULT VideoFaceBeautyFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};


#endif
