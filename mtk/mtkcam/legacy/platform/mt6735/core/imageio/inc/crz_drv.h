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
#ifndef _CRZ_DRV_H_
#define _CRZ_DRV_H_
//-----------------------------------------------------------------------------
#include <mtkcam/BuiltinTypes.h>    // For type definitions.
//#include <asm/arch/mt6593_sync_write.h> // For dsb() in isp_reg.h. Location::alps\bionic\libc\kernel\arch-arm\asm\arch
#include <mtkcam/drv/isp_drv.h>        // for IspDrv class.
//#include <mtkcam/drv/isp_reg.h>   // For isp_reg_t.


//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum
{
    CRZ_DRV_MODE_FRAME,
    CRZ_DRV_MODE_TPIPE,
    CRZ_DRV_MODE_VRMRG
} CRZ_DRV_MODE_ENUM;

typedef enum
{
    CRZ_DRV_RZ_CRZ,
    CRZ_DRV_RZ_CDRZ,
    CRZ_DRV_RZ_CURZ,
    CRZ_DRV_RZ_PRZ,
    CRZ_DRV_RZ_AMOUNT
} CRZ_DRV_RZ_ENUM;

typedef enum
{
//    CRZ_DRV_ROTDMA_VRZO,  //js_test remove below later
    CRZ_DRV_ROTDMA_VIDO,
    CRZ_DRV_ROTDMA_DISPO,   // DISPO rotation is removed in 83.
    CRZ_DRV_ROTDMA_AMOUNT
} CRZ_DRV_ROTDMA_ENUM;

typedef enum
{
//    CRZ_DRV_ALGO_8_TAP,
//    CRZ_DRV_ALGO_4_TAP, // For 83, only 4-tap.
    CRZ_DRV_ALGO_6_TAP, // For 95, only 6-tap.
    CRZ_DRV_ALGO_N_TAP,
    CRZ_DRV_ALGO_4N_TAP,
    CRZ_DRV_ALGO_AMOUNT
} CRZ_DRV_ALGO_ENUM;

typedef enum    // PRZ_OPT_SEL. PRZ optional path selection. 0(from PRZ mux ouput)/1(from DIPI output)/2(from before CDRZ).
{
    CRZ_DRV_PRZ_SRC_AFTER_CURZ,
    CRZ_DRV_PRZ_SRC_BEFORE_CURZ,
    CRZ_DRV_PRZ_SRC_BEFORE_CDRZ
} CRZ_DRV_PRZ_SRC_ENUM;

typedef enum    // DISP_VID_SEL. Display and video dma input selection. 0(Video is from CRZ, display is from PRZ)/1(Video is from PRZ, display is from CRZ).
{
    CRZ_DRV_DISPO_SRC_PRZ,  //DISPO source is PRZ, and VIDO source is RSP
    CRZ_DRV_DISPO_SRC_RSP   //DISPO source is RSP, and VIDO source is PRZ
} CRZ_DRV_DISPO_SRC_ENUM;

typedef enum
{
    CRZ_DRV_FORMAT_YUV422,
    CRZ_DRV_FORMAT_YUV420,
    CRZ_DRV_FORMAT_JPEG_YUV420,
    CRZ_DRV_FORMAT_JPEG_YUV422,
    CRZ_DRV_FORMAT_Y,
    CRZ_DRV_FORMAT_RGB888,
    CRZ_DRV_FORMAT_RGB565,
    CRZ_DRV_FORMAT_XRGB8888,
    CRZ_DRV_FORMAT_RAW8,    // only for imgo output
    CRZ_DRV_FORMAT_RAW10,   // only for imgo output
    CRZ_DRV_FORMAT_RAW12    // only for imgo output
} CRZ_DRV_FORMAT_ENUM;

typedef enum
{
    CRZ_DRV_PLANE_1,
    CRZ_DRV_PLANE_2,
    CRZ_DRV_PLANE_3
} CRZ_DRV_PLANE_ENUM;

typedef enum
{
    // For YUV422 format 1-plane/3-plane.
    CRZ_DRV_SEQUENCE_YVYU   = 0,
    CRZ_DRV_SEQUENCE_YUYV   = 1,
    CRZ_DRV_SEQUENCE_VYUY   = 2,
    CRZ_DRV_SEQUENCE_UYVY   = 3,

    // For YUV422 format 2-plane, or YUV420 format.
    CRZ_DRV_SEQUENCE_VUVU   = 0,
    CRZ_DRV_SEQUENCE_UVUV   = 1,

    // For RGB888 format, or RGB565 format .
    CRZ_DRV_SEQUENCE_RGB    = 0,
    CRZ_DRV_SEQUENCE_BGR    = 1,

    // For XRGB8888 format.
    CRZ_DRV_SEQUENCE_XRGB   = 0,
    CRZ_DRV_SEQUENCE_XBGR   = 1,
    CRZ_DRV_SEQUENCE_RGBX   = 2,
    CRZ_DRV_SEQUENCE_BGRX   = 3,

    // For YOnly format.
    CRZ_DRV_SEQUENCE_Y      = CRZ_DRV_SEQUENCE_YVYU,

} CRZ_DRV_SEQUENCE_ENUM;

typedef enum
{
    CRZ_DRV_ROTATION_0 = 0,
    CRZ_DRV_ROTATION_90,
    CRZ_DRV_ROTATION_180,
    CRZ_DRV_ROTATION_270
}CRZ_DRV_ROTATION_ENUM;

typedef struct
{
    MUINT32     Width;
    MUINT32     Height;
} CRZ_DRV_IMG_SIZE_STRUCT;

typedef struct
{
    MFLOAT      Start;
    MUINT32     Size;
} CRZ_DRV_CROP_STRUCT;

typedef struct
{
    CRZ_DRV_CROP_STRUCT     Width;
    CRZ_DRV_CROP_STRUCT     Height;
} CRZ_DRV_IMG_CROP_STRUCT;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
//-----------------------------------------------------------------------------
/*

    CDRZ ---> 3DNR ---> CURZ ---> VIDO
                    |
                     ---> PRZ ---> DISPO

    Program flow as fllow:

    1.
        CreateInstance()
        Init()
    2.
        CDRZ_Config()               //If CDRZ is enabled...
    3.
        CURZ_Config()               //If CURZ is enabled...
                                                            //4.
                                                            //    VRZ_Config()                //If VRZ is enabled...
                                                            //5.
                                                            //    VRZO_Config()               //If VRZO is enabled...
                                                            //    VRZO_SetOutputAddr()        //If VRZO is enabled...
                                                            //6.
                                                            //    RSP_Enable()
    7.
        VIDO_Config()               //If VIDO is enabled...
        VIDO_SetOutputAddr()        //If VIDO is enabled...
    8.
        PRZ_Config()                //If PRZ is enabled...
    9.
        DISPO_Config()              //If DISPO is enabled...
        DISPO_SetOutputAddr()       //If DISPO is enabled...
    10.
        Do image process...
    11.
        CDRZ_Unconfig()         //If CDRZ was configed...
    12.
        CURZ_Unconfig()         //If CURZ was configed...
                                                            //13.
                                                            //    VRZ_Unconfig()          //If VRZ was configed...
                                                            //14.
                                                            //    VRZO_Unconfig()         //If VRZ was configed...
                                                            //15.
                                                            //    RSP_Enable()            //If PRZ was enabled...
    16
        VIDO_Unconfig()         //If VIDO was configed...
    17.
        PRZ_Unconfig()          //If PRZ was configed...
    18.
        DISPO_Unconfig()        //If DISPO was configed...
    19.
        Uninit()
        DestroyInstance()
*/
//-----------------------------------------------------------------------------

class CrzDrv
{
    protected:
        virtual ~CrzDrv() {};
    //
    public:
        static CrzDrv*  CreateInstance();
        virtual void    DestroyInstance(void) = 0;
        virtual MBOOL   Init(void) = 0;
        virtual MBOOL   Uninit(void) = 0;
        virtual MBOOL   SetIspReg(isp_reg_t* pIspReg) = 0;
        virtual MBOOL   SetIspDrv(IspDrv* pIspDrv) = 0;
        virtual MBOOL   SetPhyIspDrv(IspDrv* pPhyIspDrv) = 0;
//        virtual MBOOL   CalAlgoAndCStep(
//            CRZ_DRV_RZ_ENUM         eRzName,
//            MUINT32                 SizeIn,
//            MUINT32                 u4CroppedSize,
//            MUINT32                 SizeOut,
//            CRZ_DRV_ALGO_ENUM       *pAlgo,
//            MUINT32                 *pTable,
//            MUINT32                 *pCoeffStep) = 0;
        virtual MBOOL   CalAlgoAndCStep(
            CRZ_DRV_MODE_ENUM       eFrameOrTpipeOrVrmrg,
            CRZ_DRV_RZ_ENUM         eRzName,
            MUINT32                 SizeIn_H,
            MUINT32                 SizeIn_V,
            MUINT32                 u4CroppedSize_H,
            MUINT32                 u4CroppedSize_V,
            MUINT32                 SizeOut_H,
            MUINT32                 SizeOut_V,
            CRZ_DRV_ALGO_ENUM       *pAlgo_H,
            CRZ_DRV_ALGO_ENUM       *pAlgo_V,
            MUINT32                 *pTable_H,
            MUINT32                 *pTable_V,
            MUINT32                 *pCoeffStep_H,
            MUINT32                 *pCoeffStep_V) = 0;
        virtual MBOOL   Reset(void) = 0;
        virtual MBOOL   ResetDefault(void) = 0;
        virtual MBOOL   DumpReg(void) = 0;
        //CDRZ
        virtual MBOOL   CDRZ_Config(
            CRZ_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CRZ_DRV_IMG_SIZE_STRUCT     SizeIn,
            CRZ_DRV_IMG_SIZE_STRUCT     SizeOut,
            CRZ_DRV_IMG_CROP_STRUCT     Crop) = 0;
        virtual MBOOL   CDRZ_Unconfig(void) = 0;
        //CURZ

};

//-----------------------------------------------------------------------------
#endif  // _CRZ_DRV_H_

