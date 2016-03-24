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
#ifndef _CRZ_DRV_IMP_H_
#define _CRZ_DRV_IMP_H_
//-----------------------------------------------------------------------------
#include "crz_drv.h"
//#include "imageio_log.h"


//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

//-----------------------------------------------------------------------------
#define CRZ_DRV_ISP_DEV_NAME    "/dev/camera-isp"
#define CRZ_DRV_INIT_MAX        1
#define CRZ_DRV_BUF_PMEM        0
#define CRZ_DRV_BUF_SYSRAM      1
#define CRZ_DRV_BUF_SYSRAM_BANK0_SIZE       32768   // Bytes. 32KB. i.e. 0x8000.    // NEED_TUNING_BY_CHIP.
#define CRZ_DRV_BUF_SYSRAM_BANK1_SIZE       49152   // Bytes. 48KB. i.e. 0xC000.    // NEED_TUNING_BY_CHIP.
#define CRZ_DRV_BUF_SYSRAM_SIZE             (CRZ_DRV_BUF_SYSRAM_BANK0_SIZE + CRZ_DRV_BUF_SYSRAM_BANK1_SIZE)

#define CRZ_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START (0)
#define CRZ_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_END   (CRZ_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START + CRZ_DRV_BUF_SYSRAM_BANK0_SIZE - 1)   // 0x7FFF.
#define CRZ_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_START (CRZ_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START + CRZ_DRV_BUF_SYSRAM_BANK0_SIZE)       // 0x8000.
#define CRZ_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END   (CRZ_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START + CRZ_DRV_BUF_SYSRAM_BANK0_SIZE + CRZ_DRV_BUF_SYSRAM_BANK1_SIZE - 1) // 0x13FFF.

//-----------------------------------------------------------------------------
#define CRZ_DRV_DEFAULT_CRSP_CTRL                   (0x00000003)
#define CRZ_DRV_DEFAULT_DIT_INIT_X                  (0x0)
#define CRZ_DRV_DEFAULT_DIT_INIT_Y                  (0x0)
#define CRZ_DRV_DEFAULT_DIT_XRGB_DUMMY              (0xFF)
#define CRZ_DRV_DEFAULT_DIT_SEED_R                  (0x3E8)
#define CRZ_DRV_DEFAULT_DIT_SEED_G                  (0x7D0)
#define CRZ_DRV_DEFAULT_DIT_SEED_B                  (0xBB8)
#define CRZ_DRV_DEFAULT_DERING_THRESHOLD_1H         (0x4)
#define CRZ_DRV_DEFAULT_DERING_THRESHOLD_1V         (0x6)
#define CRZ_DRV_DEFAULT_DERING_THRESHOLD_2H         (0x8)
#define CRZ_DRV_DEFAULT_DERING_THRESHOLD_2V         (0x100)
#define CRZ_DRV_DEFAULT_TRUNC_BIT                   (0x0)
#define CRZ_DRV_DEFAULT_SOF_RESET                   (0x0)
#define CRZ_DRV_DEFAULT_CAMERA_2_DISP_PADDING_Y     (0x0)
#define CRZ_DRV_DEFAULT_CAMERA_2_DISP_PADDING_U     (0x0)
#define CRZ_DRV_DEFAULT_CAMERA_2_DISP_PADDING_V     (0x0)
#define CRZ_DRV_DEFAULT_FIFO_PRI_LOW_THR            (0x0)
#define CRZ_DRV_DEFAULT_FIFO_PRI_THR                (0x0)
#define CRZ_DRV_DEFAULT_ROTATION_BUF_LINE_NUM       (64)
//-----------------------------------------------------------------------------
#define CRZ_DRV_MASK_TRUNC_BIT              (0x3)
#define CRZ_DRV_MASK_TABLE_SELECT           (0x1F)
#define CRZ_DRV_MASK_IMAGE_SIZE             (0x1FFF)
#define CRZ_DRV_MASK_COEFF_STEP             (0x7FFFFF)
#define CRZ_DRV_MASK_INT_OFFSET             (0x1FFF)
#define CRZ_DRV_MASK_SUB_OFFSET             (0x1FFFFF)
#define CRZ_DRV_MASK_DERING_THRESHOLD_1     (0xF)
#define CRZ_DRV_MASK_DERING_THRESHOLD_2     (0x1FF)
#define CRZ_DRV_MASK_FIFO_PRI_LOW_THR       (0xFFF)
#define CRZ_DRV_MASK_FIFO_PRI_THR           (0xFFF)
#define CRZ_DRV_MASK_MAX_BURST_LEN          (0x1F)
#define CRZ_DRV_MASK_BUF_LINE_NUM           (0x7F)
#define CRZ_DRV_MASK_BUF_WIDTH              (0x1FFF)
#define CRZ_DRV_MASK_BUF_SIZE               (0x1FFF)
#define CRZ_DRV_MASK_ADDR_OFFSET            (0xFFFFFFF)
#define CRZ_DRV_MASK_ADDR_STRIDE            (0xFFFF)
#define CRZ_DRV_MASK_PADDING                (0xFF)
#define CRZ_DRV_MASK_DIT_INIT               (0xF)
#define CRZ_DRV_MASK_DIT_XRGB_DUMMY         (0xFF)
#define CRZ_DRV_MASK_DIT_SEED               (0xFFFFF)
#define CRZ_DRV_MASK_RSP_COEFF_STEP         (0x7)
#define CRZ_DRV_MASK_RSP_OFFSET             (0x3)
//-----------------------------------------------------------------------------
#define CRZ_DRV_YUV420_PLANE_2              (0)
#define CRZ_DRV_YUV420_PLANE_3              (2)
//-----------------------------------------------------------------------------
#define CRZ_DRV_RZ_4N_TAP_TABLE             (15)
#define CRZ_DRV_RZ_N_TAP_TABLE              (0)
#define CRZ_DRV_RZ_TABLE_OFFSET             (20)
#define CRZ_DRV_RZ_TABLE_AMOUNT             (6)
//
#define CRZ_DRV_RZ_4_TAP_RATIO_MAX          (32)
#define CRZ_DRV_RZ_4_TAP_RATIO_MIN          (2)     // For this definition, it really means the denominator of MinRatio (1/2).
//
#define CRZ_DRV_RZ_4N_TAP_RATIO_MAX         (2)     // For this definition, it really means the denominator of MaxRatio (1/2).
#define CRZ_DRV_RZ_4N_TAP_RATIO_MIN         (64)    // For this definition, it really means the denominator of MinRatio (1/64).
//
#define CRZ_DRV_RZ_N_TAP_RATIO_MAX          (64)    // For this definition, it really means the denominator of MaxRatio (1/64).
#define CRZ_DRV_RZ_N_TAP_RATIO_MIX          (256)   // For this definition, it really means the denominator of MinRatio (1/256).    //Vent@20120808: ~1/128 has no quality loss. 1/128~1/256 has quality loss (Confirmed by Joseph Lai).
//-----------------------------------------------------------------------------
// < Supporting Width >
//     CDRZ
#define CRZ_DRV_SUPPORT_WIDTH_TPIPE_CRZ_4_TAP  (768)
#define CRZ_DRV_SUPPORT_WIDTH_TPIPE_CRZ_4N_TAP (384)
#define CRZ_DRV_SUPPORT_WIDTH_TPIPE_CRZ_N_TAP  (768)

//     CURZ
#define CRZ_DRV_SUPPORT_WIDTH_FRAME_CURZ_4_TAP  (1920)
#define CRZ_DRV_SUPPORT_WIDTH_TPIPE_CURZ_4_TAP  (65534)     // There is no supporting width limit for TPIPE mode CURZ 4-tap, so use a very large number to represent it.
#define CRZ_DRV_SUPPORT_WIDTH_VRMRG_CURZ_4_TAP  (65534)     // There is no supporting width limit for VR MRG mode CURZ 4-tap, so use a very large number to represent it.
//     PRZ
#define CRZ_DRV_SUPPORT_WIDTH_FRAME_PRZ_4_TAP   (1280)
#define CRZ_DRV_SUPPORT_WIDTH_FRAME_PRZ_4N_TAP  (640)
#define CRZ_DRV_SUPPORT_WIDTH_FRAME_PRZ_N_TAP   (1280)
#define CRZ_DRV_SUPPORT_WIDTH_TPIPE_PRZ_4_TAP   (768)
#define CRZ_DRV_SUPPORT_WIDTH_TPIPE_PRZ_4N_TAP  (384)
#define CRZ_DRV_SUPPORT_WIDTH_TPIPE_PRZ_N_TAP   (768)
#define CRZ_DRV_SUPPORT_WIDTH_VRMRG_PRZ_4_TAP   (4096)
#define CRZ_DRV_SUPPORT_WIDTH_VRMRG_PRZ_4N_TAP  (2048)
#define CRZ_DRV_SUPPORT_WIDTH_VRMRG_PRZ_N_TAP   (4096)

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum
{
    CRZ_DRV_BLOCK_SCAN_LINE,
    CRZ_DRV_BLOCK_MTK
} CRZ_DRV_BLOCK_ENUM;

typedef enum
{
    CRZ_DRV_PADDING_MODE_8,
    CRZ_DRV_PADDING_MODE_16
} CRZ_DRV_PADDING_MODE_ENUM;

typedef enum
{
    CRZ_DRV_UV_SELECT_EVEN,
    CRZ_DRV_UV_SELECT_ODD,
    CRZ_DRV_UV_SELECT_EVERY,
} CRZ_DRV_UV_SELECT_ENUM;

typedef enum
{
    CRZ_DRV_BURST_LEN_1 = 1,
    CRZ_DRV_BURST_LEN_2 = 2,
    CRZ_DRV_BURST_LEN_4 = 4,
    CRZ_DRV_BURST_LEN_8 = 8
} CRZ_DRV_BURST_LEN_ENUM;

typedef enum
{
    CRZ_DRV_DIT_MODE_16X16,
    CRZ_DRV_DIT_MODE_LFSR,
    CRZ_DRV_DIT_MODE_16X16_LFSR
} CRZ_DRV_DIT_MODE_ENUM;

typedef enum
{
    CRZ_DRV_UV_FORMAT_YUV422,
    CRZ_DRV_UV_FORMAT_YUV420,
    CRZ_DRV_UV_FORMAT_YUV422V,
    CRZ_DRV_UV_FORMAT_YUV444
} CRZ_DRV_UV_FORMAT_ENUM;

typedef enum
{
    CRZ_DRV_LC_LUMA,
    CRZ_DRV_LC_CHROMA,
    CRZ_DRV_LC_AMOUNT
} CRZ_DRV_LC_ENUM;

typedef struct
{
    MINT32      Fd;
    MUINT32     VirAddr;
    MUINT32     PhyAddr;
    MUINT32     Size;
} CRZ_DRV_BUF_STRUCT;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class CrzDrvImp : public CrzDrv
{
    protected:
        CrzDrvImp();
        ~CrzDrvImp();
    //
    public:
        static CrzDrv*  GetInstance(bool fgIsGdmaMode = MFALSE);
        virtual void    DestroyInstance(void);
        virtual MBOOL   Init(void);
        virtual MBOOL   Uninit(void);
        virtual MBOOL   SetIspReg(isp_reg_t* pIspReg);
        virtual MBOOL   SetIspDrv(IspDrv* pIspDrv);
        virtual MBOOL   SetPhyIspDrv(IspDrv* pPhyIspDrv);
        virtual MBOOL   CheckReady(void);
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
            MUINT32                 *pCoeffStep_V);
        virtual MBOOL   CalOffset(
            CRZ_DRV_ALGO_ENUM   Algo,
            MBOOL               IsWidth,
            MUINT32             CoeffStep,
            MFLOAT              Offset,
            MUINT32*            pLumaInt,
            MUINT32*            pLumaSub,
            MUINT32*            pChromaInt,
            MUINT32*            pChromaSub);

        virtual MBOOL   Reset(void);
        virtual MBOOL   ResetDefault(void);
        virtual MBOOL   DumpReg(void);

        //CDRZ
        virtual MBOOL   CDRZ_Enable(MBOOL En);
        virtual MBOOL   CDRZ_ResetDefault(void);
        virtual MBOOL   CDRZ_DumpReg(void);
        virtual MBOOL   CDRZ_H_EnableScale(MBOOL En);
        virtual MBOOL   CDRZ_V_EnableScale(MBOOL En);
        virtual MBOOL   CDRZ_V_EnableFirst(MBOOL En);
        virtual MBOOL   CDRZ_H_SetAlgo(CRZ_DRV_ALGO_ENUM Algo);
        virtual MBOOL   CDRZ_V_SetAlgo(CRZ_DRV_ALGO_ENUM Algo);
        virtual MBOOL   CDRZ_H_SetTruncBit(MUINT32 Bit);
        virtual MBOOL   CDRZ_V_SetTruncBit(MUINT32 Bit);
        virtual MBOOL   CDRZ_H_SetTable(MUINT32 Table);
        virtual MBOOL   CDRZ_V_SetTable(MUINT32 Table);
        virtual MBOOL   CDRZ_H_SetInputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_V_SetInputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_H_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   CDRZ_V_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   CDRZ_H_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   CDRZ_V_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   CDRZ_Config(
            CRZ_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CRZ_DRV_IMG_SIZE_STRUCT     SizeIn,
            CRZ_DRV_IMG_SIZE_STRUCT     SizeOut,
            CRZ_DRV_IMG_CROP_STRUCT     Crop);
        virtual MBOOL   CDRZ_Unconfig(void);

    //
    private:
        mutable Mutex       mLock;
        volatile MINT32     mInitCount; // CRZ Drv instance count.
        volatile MINT32     mSysramUsageCount; // SYSRAM Buffer usage count. Record how many users are using SYSRAM.
        SYSRAM_ALLOC_STRUCT SysramAlloc;
        CRZ_DRV_BUF_STRUCT  mRotationBuf[CRZ_DRV_ROTDMA_AMOUNT][CRZ_DRV_LC_AMOUNT];
        IspDrv*             m_pIspDrv;
        isp_reg_t*          mpIspReg;
        IspDrv*             m_pPhyIspDrv;
        isp_reg_t*          m_pPhyIspReg;

        MINT32              mFdSysram;
        bool                m_fgIsGdmaMode;
        bool                m_fgVertFirst;


};

//-----------------------------------------------------------------------------
#endif  // _CRZ_DRV_IMP_H_

