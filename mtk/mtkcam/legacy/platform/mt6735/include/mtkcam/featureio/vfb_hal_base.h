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
/*
** $Log: fd_hal_base.h $
 *
*/

#include "ImageFormat.h"

#ifndef _VFB_HAL_BASE_H_
#define _VFB_HAL_BASE_H_

/*******************************************************************************
*
********************************************************************************/
#define VFBFDWidth      320 //if image is vertical  FD width and Height must be exchanged
#define VFBFDHeight     240 //if image is vertical  FD width and Height must be exchanged

#define GLOBAL_FACE_NUM     3
#define MIN_FACE_RATIO      12

struct MTKPipeVfbEnvInfo
{
    MUINT32 Features;                                // Current feature combination
    MINT32  ThreadPolicy;
    MINT32  ThreadPriority;
    MUINT16 SrcImgWidth;
    MUINT16 SrcImgHeight;
    MUINT16 DsImgWidth;
    MUINT16 DsImgHeight;
    MUINT16 FDWidth;
    MUINT16 FDHeight;
    NSCam::EImageFormat SrcImgFormat;
    NSCam::EImageFormat DsImgFormat;
    MBOOL   RecordingHint;
};
struct MTKPipeVfbProcInfo
{
    //Input
    MINT32  SmoothLevel;
    MINT32  SkinColor;
    MINT32  EnlargeEyeLevel;
    MINT32  SlimFaceLevel;
    MBOOL   ExtremeBeauty; //1:enable warp, 1 face only; 0:no warp, multiple face
    MINT32  TouchX;
    MINT32  TouchY;
    //MUINT8* SrcImgAddr;
    MUINT8* DsImgAddr;
    void*   FaceMetadata;
    MINT32  ClipWidth;
    MINT32  ClipHeight;
    MINT32  DsClipWidth;
    MINT32  DsClipHeight;
    //
    MUINT16  SrcImgWidth;
    MUINT16  SrcImgHeight;
    MUINT16  DsImgWidth;
    MUINT16  DsImgHeight;
    MUINT16  FDWidth;
    MUINT16  FDHeight;
    //
    MUINT32 PreWarpMapSize[2]; //EIS warp map size, 0:X, 1:Y
    MINT32* PreWarpMapX; //EIS warp map
    MINT32* PreWarpMapY; //EIS warp map
    //Output
    //MUINT8* WarpedFullImage;
    MUINT8* WarpedDsImage;
    MUINT8* AlphaCL;
    MUINT8* AlphaNR;
    MINT32* PCATable;
    MINT32* WarpMapX; //to GPU local warp
    MINT32* WarpMapY; //to GPU loacl warp
};
struct MTKPipeVfbResultInfo
{
    //MUINT8* WarpedFullImage;
    MUINT8* WarpedDsImage;
    MUINT8* AlphaCL;
    MUINT8* AlphaNR;
    MINT32* PCATable;
    MUINT32 FacePos[15][2];
    MINT32* WarpMapX; //to GPU local warp
    MINT32* WarpMapY; //to GPU loacl warp
};

/*******************************************************************************
*
********************************************************************************/
class halVFBBase {
public:
    //
    MBOOL CANCEL;

    static halVFBBase* createInstance();
    virtual void      destroyInstance() = 0;
    virtual ~halVFBBase() {};

    /**
     * @brief init the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   mHalVFBInit(MTKPipeVfbEnvInfo VfbEnvInfo) {return false;};

    /**
     * @brief uninit the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   mHalVFBUninit() {return false;};

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalVfbProc () -
    //! \brief Vfb process
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MBOOL mHalVFBProc(MTKPipeVfbProcInfo VfbProcInfo, MTKPipeVfbResultInfo* VfbResultInfo) {return 0;}

public:
    /////////////////////////////////////////////////////////////////////////
    //
    // Get Working buffer size () -
    //!
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalVFBGetWorkingBufSize() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // Set Working buffer () -
    //!
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MBOOL mHalVFBSetWorkingBuf(void* WorkingBufAddr, MINT32 WorkingBufSize) {return false;};
};

class halVFBTmp : public halVFBBase {
public:
    //
    static halVFBBase* getInstance();
    virtual void destroyInstance();
    //
    halVFBTmp() {};
    virtual ~halVFBTmp() {};
};

class halVFBTuning {
public:
    static halVFBTuning &getInstance();
    void mHalVFBTuningSetPCA(MINT32* PCAAdr);
    MUINT32* mHalVFBTuningGetPCA();
    void mHalVFBTuningSetLCE(MINT32* LCEAdr);
    MINT32* mHalVFBTuningGetLCE();

private:
    MINT8   mPCALut[360*4];
    MINT32* mLCEAdr;
};

#endif

