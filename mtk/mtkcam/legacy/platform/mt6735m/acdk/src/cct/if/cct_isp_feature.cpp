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
#define MTK_LOG_ENABLE 1
#define LOG_TAG "isp_cctop"
//
#include <utils/Errors.h>
#include <cutils/log.h>

#include <mtkcam/acdk/cct_feature.h>
#include <mtkcam/drv/isp_drv.h>
#include "cct_main.h"
#include "cct_imp.h"
#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "flash_mgr.h"
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include "lsc_mgr2.h"
#include "nvbuf_util.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CUSTOM_NVRAM_REG_INDEX comes from isp_tuning_idx.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct CUSTOM_NVRAM_REG_INDEX
{
    MUINT16  OBC;
    MUINT16  BPC;
    MUINT16  NR1;
    MUINT16  CFA;
    MUINT16  GGM;
    MUINT16  ANR;
    MUINT16  CCR;
    MUINT16  EE;
    MUINT16  NR3D;
    MUINT16  MFB;
    MUINT16  LCE;
} CUSTOM_NVRAM_REG_INDEX_T;


using namespace NSIspTuning;
using namespace NS3A;
/*******************************************************************************
*
********************************************************************************/
#define MY_LOG(fmt, arg...)    { ALOGD(fmt, ##arg);printf(fmt, ##arg);}
#define MY_LOG(fmt, arg...)    ALOGD(fmt, ##arg)
#define MY_ERR(fmt, arg...)    ALOGE("Err: %5d: " fmt, __LINE__, ##arg)

#define FIXED 0
/*******************************************************************************
*
********************************************************************************/
MBOOL  CctCtrl::updateIspRegs(MUINT32 const u4Category/*= 0xFFFFFFFF*/, MUINT32 const u4Index/*= 0xFFFFFFFF*/)
{
#if 0
    MBOOL fgRet = MFALSE;

    MBOOL fgIsDynamicISPEnabled = false;
    MBOOL fgIsDynamicCCMEnabled = false;
    MBOOL fgDisableDynamic = false; //  Disable Dynamic Data.

    //  (1) Save all index.
    ISP_NVRAM_REG_INDEX_T BackupIdx = m_rISPRegsIdx;

    //  (2) Modify a specific index.
#define MY_SET_ISP_REG_INDEX(_category)\
    case EIspReg_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  MFALSE;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch (u4Category)
    {
    MY_SET_ISP_REG_INDEX(LSC);
    MY_SET_ISP_REG_INDEX(OBC);
    MY_SET_ISP_REG_INDEX(CFA);
    MY_SET_ISP_REG_INDEX(BPC);
    MY_SET_ISP_REG_INDEX(NR1);
    MY_SET_ISP_REG_INDEX(ANR);
    MY_SET_ISP_REG_INDEX(EE);
    //MY_SET_ISP_REG_INDEX(Saturation);
    //MY_SET_ISP_REG_INDEX(Contrast);
    //MY_SET_ISP_REG_INDEX(Hue);
    MY_SET_ISP_REG_INDEX(CCM);
    MY_SET_ISP_REG_INDEX(GGM);
    default:
        break;
    }

    //  (3) Save the current dynamic ISP flag.
    NSIspTuning::CmdArg_T cmd_GetDynamicISP;
    cmd_GetDynamicISP.eCmd        = NSIspTuning::ECmd_GetDynamicTuning;
    cmd_GetDynamicISP.pOutBuf     = &fgIsDynamicISPEnabled;
    cmd_GetDynamicISP.u4OutBufSize= sizeof(MBOOL);
    //  (4) Save the current dynamic CCM flag.
    NSIspTuning::CmdArg_T cmd_GetDynamicCCM;
    cmd_GetDynamicCCM.eCmd        = NSIspTuning::ECmd_GetDynamicCCM;
    cmd_GetDynamicCCM.pOutBuf     = &fgIsDynamicCCMEnabled;
    cmd_GetDynamicCCM.u4OutBufSize= sizeof(MBOOL);
    //  (5) Disable the dynamic ISP.
    NSIspTuning::CmdArg_T cmd_DisableDynamicISP;
    cmd_DisableDynamicISP.eCmd       = NSIspTuning::ECmd_SetDynamicTuning;
    cmd_DisableDynamicISP.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (6) Disable the dynamic CCM.
    NSIspTuning::CmdArg_T cmd_DisableDynamicCCM;
    cmd_DisableDynamicCCM.eCmd       = NSIspTuning::ECmd_SetDynamicCCM;
    cmd_DisableDynamicCCM.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicCCM.u4InBufSize= sizeof(MBOOL);
    //  (8) Restore the dynamic ISP flag.
    NSIspTuning::CmdArg_T cmd_RestoreDynamicISP;
    cmd_RestoreDynamicISP.eCmd       = NSIspTuning::ECmd_SetDynamicTuning;
    cmd_RestoreDynamicISP.pInBuf     = &fgIsDynamicISPEnabled;
    cmd_RestoreDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (9) Restore the dynamic CCM flag.
    NSIspTuning::CmdArg_T cmd_RestoreDynamicCCM;
    cmd_RestoreDynamicCCM.eCmd       = NSIspTuning::ECmd_SetDynamicCCM;
    cmd_RestoreDynamicCCM.pInBuf     = &fgIsDynamicCCMEnabled;
    cmd_RestoreDynamicCCM.u4InBufSize= sizeof(MBOOL);


    if  (
            0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicISP))     //(3)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicCCM))     //(4)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicISP)) //(5)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicCCM)) //(6)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_VALIDATE_FRAME, true)                                  //(7) Validate
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicISP)) //(8)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicCCM)) //(9)
        )
    {
        goto lbExit;
    }

    MY_LOG("dynamic flags:(isp, ccm)=(%d, %d)", fgIsDynamicISPEnabled, fgIsDynamicCCMEnabled);

    fgRet = MTRUE;

lbExit:
    //  (10) Restore all index.
    m_rISPRegsIdx = BackupIdx;
#endif

    return  MTRUE;

}


/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPReadIspReg(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen)
{

    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoOut = (PACDK_CCT_REG_RW_STRUCT)puParaOut;
    ISP_DRV_REG_IO_STRUCT isp_reg;

    MY_LOG("ACDK_CCT_OP_ISP_READ_REG\n");

    isp_reg.Addr = pIspRegInfoIn->RegAddr;
    isp_reg.Data = 0xFFFFFFFF;

    err = m_pIspDrv->readRegs(&isp_reg, 1);

    //if (err < 0)
    if (!err) //enter if false
    {
        MY_ERR("[CCTOPReadIspReg] readIspRegs() error");
        return err;
    }

    pIspRegInfoOut->RegData = isp_reg.Data;
    *pu4RealParaOutLen = sizeof(ACDK_CCT_REG_RW_STRUCT);
    MY_LOG("[CCTOPReadIspReg] regAddr = %x, regData = %x\n", (MUINT32)isp_reg.Addr, (MUINT32)isp_reg.Data);

    //return err;
    return CCTIF_NO_ERROR;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPWriteIspReg(MVOID *puParaIn)
{

    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    ISP_DRV_REG_IO_STRUCT isp_reg;

    MY_LOG("ACDK_CCT_OP_ISP_WRITE_REG\n");

    isp_reg.Addr = pIspRegInfoIn->RegAddr;
    isp_reg.Data = pIspRegInfoIn->RegData;

    err = m_pIspDrv->writeRegs(&isp_reg, 1);
    if (err < 0) {
        MY_ERR("[CCTOPWriteIspReg]writeRegs() error");
        return err;
    }

    MY_LOG("[CCTOPWriteIspReg] regAddr = %x, regData = %x\n", (MUINT32)isp_reg.Addr, (MUINT32)isp_reg.Data);

    return err;

}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID )
{
    if  ( sizeof(MUINT32) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    *reinterpret_cast<MUINT32 *>(puParaOut) = sizeof(MUINTPTR) == 4 ? 0x67350232 : 0x67350264; // 00 means 32bit; FF means 64bit
    *pu4RealParaOutLen = sizeof(MUINT32);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID ) done, 0x%08X\n", *reinterpret_cast<MUINT32 *>(puParaOut));
    return CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_READ_REG )
{
    return CCTOPReadIspReg((MVOID *)puParaIn, (MVOID *)puParaOut, pu4RealParaOutLen);
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_WRITE_REG )
{
    return CCTOPWriteIspReg((MVOID *)puParaIn);
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX )
{
    typedef ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;

#define MY_SET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  CCTIF_BAD_PARAM;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch  (eCategory)
    {

        MY_SET_TUNING_INDEX(OBC);
#if 1//FIXED==1
        MY_SET_TUNING_INDEX(BPC);
#endif
        MY_SET_TUNING_INDEX(NR1);
        MY_SET_TUNING_INDEX(SL2);
        MY_SET_TUNING_INDEX(CFA);
        MY_SET_TUNING_INDEX(ANR);
        MY_SET_TUNING_INDEX(CCR);
        MY_SET_TUNING_INDEX(EE);
        //MY_SET_TUNING_INDEX(NR3D);
        //MY_SET_TUNING_INDEX(MFB);

        default:
            MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX ) Warning!! eCategory(%d) is not supported, u4Index=%d", eCategory, u4Index);
            break;//return  CCTIF_BAD_PARAM;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX ) done, (u4Index, eCategory) = (%d, %d)", u4Index, eCategory);
    return  CCTIF_NO_ERROR;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY   i_type;
    typedef MUINT32                     o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    o_type&      rIndex    = *reinterpret_cast<o_type*>(puParaOut);


#define MY_GET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        rIndex = m_rISPRegsIdx._category;\
        break

    switch  (eCategory)
    {
        MY_GET_TUNING_INDEX(OBC);
#if 1//FIXED==1
        MY_GET_TUNING_INDEX(BPC);
#endif
        MY_GET_TUNING_INDEX(NR1);
        MY_GET_TUNING_INDEX(SL2);
        MY_GET_TUNING_INDEX(CFA);
        MY_GET_TUNING_INDEX(ANR);
        MY_GET_TUNING_INDEX(CCR);
        MY_GET_TUNING_INDEX(EE);
        //MY_GET_TUNING_INDEX(NR3D);
        //MY_GET_TUNING_INDEX(MFB);

        default:
            MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX ) Warning!! eCategory(%d) is not supported, rIndex=%d", eCategory, rIndex);
            rIndex = -1;
            break;//return  CCTIF_BAD_PARAM;
    }

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX ) done, (eCategory, rIndex) = (%d, %d)", eCategory, rIndex);

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
pu4RealParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM )
{
#if 1//FIXED==1
    typedef ISP_NVRAM_MFB_MIXER_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_MFB_MIXER_PARAM_T *rRegs = reinterpret_cast<o_type *>(puParaOut)->param;

    ::memcpy(rRegs, (const void*)&m_rBuf_ISP.ISPMfbMixer.param, sizeof(o_type));
    *pu4RealParaOutLen = sizeof(o_type);
#endif
    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM )
{
#if 1//FIXED==1
    typedef ISP_NVRAM_MFB_MIXER_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_MFB_MIXER_PARAM_T *rRegs = reinterpret_cast<i_type *>(puParaIn)->param;
    ::memcpy((void*)&m_rBuf_ISP.ISPMfbMixer.param, rRegs, sizeof(i_type));
#endif
    return  CCTIF_NO_ERROR;

}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
pu4RealParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF )
{
#if 1//FIXED==1
    typedef ISP_NVRAM_CCM_POLY22_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_CCM_POLY22_STRUCT *rRegs = reinterpret_cast<o_type *>(puParaOut);

    ::memcpy(rRegs, &m_rBuf_ISP.ISPMulitCCM.Poly22, sizeof(o_type));
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF ) done\n");
#endif
    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF )
{
#if 1//FIXED==1
    typedef ISP_NVRAM_CCM_POLY22_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_CCM_POLY22_STRUCT *rRegs = reinterpret_cast<i_type *>(puParaIn);
    ::memcpy(&m_rBuf_ISP.ISPMulitCCM.Poly22, rRegs, sizeof(i_type));
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF ) done\n");
#endif
    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ACDK_CCT_ISP_GET_TUNING_PARAS
u4ParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS )
{
    typedef ACDK_CCT_ISP_GET_TUNING_PARAS o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ACDK_CCT_ISP_NVRAM_REG& rRegs = reinterpret_cast<o_type*>(puParaOut)->stIspNvramRegs;

#define MY_GET_TUNING_PARAS(_category)\
    ::memcpy(rRegs._category, m_rISPRegs._category, sizeof(rRegs._category))

    MY_GET_TUNING_PARAS(OBC);
#if 1//FIXED==1
    MY_GET_TUNING_PARAS(BPC);
#endif
    MY_GET_TUNING_PARAS(NR1);
    MY_GET_TUNING_PARAS(SL2);
    MY_GET_TUNING_PARAS(CFA);
    MY_GET_TUNING_PARAS(ANR);
    MY_GET_TUNING_PARAS(CCR);
    MY_GET_TUNING_PARAS(EE);
    //MY_GET_TUNING_PARAS(NR3D);
    //MY_GET_TUNING_PARAS(MFB);


    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS ) done\n");
    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_SET_TUNING_PARAS
u4ParaInLen
    sizeof(ACDK_CCT_ISP_SET_TUNING_PARAS);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS )
{
#if 1//FIXED==1
    typedef ACDK_CCT_ISP_SET_TUNING_PARAS type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;
    ACDK_CCT_ISP_BUFFER_ACCESS_STRUCT const&   bufAccess       = reinterpret_cast<type const*>(puParaIn)->bufAccess;

#define CHECK_INDEX_RANGE(_category)\
    if (IspNvramRegMgr::NUM_##_category <= u4Index)\
        return CCTIF_BAD_PARAM;\
    m_rISPRegs._category[u4Index] = ((ISP_NVRAM_##_category##_T*)(bufAccess.pBuffer))[u4Index];\
    m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);

    switch(eCategory) {

        case EIsp_Category_OBC:
            CHECK_INDEX_RANGE(OBC);
            ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_OBC_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //Temp. Mark
            NSIspTuning::IspTuningMgr::getInstance().setPureOBCInfo((MINT32)m_eSensorEnum, &((ISP_NVRAM_OBC_T*)(bufAccess.pBuffer))[u4Index]);
            //NSIspTuning::ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_BPC:
            CHECK_INDEX_RANGE(BPC);
            ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BPC_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_NR1:
            CHECK_INDEX_RANGE(NR1);
            NSIspTuning::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_NR1_T*)(bufAccess.pBuffer))[u4Index]);
            //NSIspTuning::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

/*
        case EIsp_Category_LSC:
            CHECK_INDEX_RANGE(LSC);
            NSIspTuning::ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_LSC_T*)g_set_isp_para_buffer)[u4Index]);
            NSIspTuning::ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;
*/
        case EIsp_Category_SL2:
            CHECK_INDEX_RANGE(SL2);
            ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_SL2_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_CFA:
            CHECK_INDEX_RANGE(CFA);
            ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_CFA_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

//        case EIsp_Category_CCM:
//        case EIsp_Category_GGM:
//        case EIsp_Category_IHDR_GGM:
        case EIsp_Category_ANR:
            CHECK_INDEX_RANGE(ANR);
            ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_ANR_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_CCR:
            CHECK_INDEX_RANGE(CCR);
            ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_CCR_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_EE:
            CHECK_INDEX_RANGE(EE);
            ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_EE_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;
#if 0
        case EIsp_Category_NR3D:
            CHECK_INDEX_RANGE(NR3D);
            //Temp. Mark
            ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_NR3D_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_MFB:
            CHECK_INDEX_RANGE(MFB);
            //Temp. Mark
            ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_MFB_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_LCE:
            //CHECK_INDEX_RANGE(LCE);
            //Temp. Mark
            //ISP_MGR_LCE_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_LCE_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_LCE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuning::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
            break;
#endif
        default:
            MY_ERR("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS ) eCategory(%d) is not supported, u4Index=%d\n", eCategory, u4Index);
            return  CCTIF_BAD_PARAM; // show error msg, no effect, don't return error

    }
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS ) done, (u4Index, eCategory) = (%d, %d)\n", u4Index, eCategory);
#endif
    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE )
{

    MY_LOG("Enable Dynamic Bypass!!\n");
#if 1//FIXED==1
    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass((MINT32)m_eSensorEnum, MTRUE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;
#else
    return CCTIF_NO_ERROR;
#endif
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE )
{

    MY_LOG("Disable Dynamic Bypass!!\n");
#if 1//FIXED==1
    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass((MINT32)m_eSensorEnum, MFALSE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;
#else
    return CCTIF_NO_ERROR;
#endif
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF )
{

    MINT32 bypass_en;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;
#if 1//FIXME==1
    bypass_en = NSIspTuning::IspTuningMgr::getInstance().getDynamicBypass((MINT32)m_eSensorEnum);
#endif
    if(bypass_en < 0)
    {
        return CCTIF_UNKNOWN_ERROR;
    }

    reinterpret_cast<o_type*>(puParaOut)->Enable = bypass_en;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF ) done, %d\n", bypass_en);
    return CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE )
{
#if 1//FIXED==1
    typedef ACDK_CCT_GAMMA_ACCESS_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    typedef ACDK_CCT_GAMMA_ACCESS_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    o_type*const pGammaAccessOut = reinterpret_cast<o_type*>(puParaOut);
    i_type*const pGammaAccessIn = reinterpret_cast<i_type*>(puParaIn);
    MUINT32 index;
    pGammaAccessOut->index = index = pGammaAccessIn->index;
    MBOOL bIHDR;
    pGammaAccessOut->bIHDR = bIHDR = pGammaAccessIn->bIHDR;
    ISP_NVRAM_GGM_T* pGGM;
    /*if (bIHDR) pGGM = &m_rISPRegs.IHDR_GGM[index];
    else*/ pGGM = &m_rISPRegs.GGM[index];

    int limit = 0;
    if (index >= (limit = /*bIHDR ? NVRAM_IHDR_GGM_TBL_NUM : */NVRAM_GGM_TBL_NUM))
    {
        MY_ERR("index(%d) exceeds limit(%d), bIHDR(%d)\n", index, limit, bIHDR);
        return  CCTIF_BAD_PARAM;
    }
    MY_LOG("index(%d), limit(%d), bIHDR(%d) are checked ok\n", index, limit, bIHDR);

    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).get(ggm);

    for(int i=0;i<GAMMA_STEP_NO; i++) {
        pGammaAccessOut->gamma.r_tbl[i] = pGGM->rb_gmt.lut[i].R_GAMMA;
        pGammaAccessOut->gamma.g_tbl[i] = pGGM->g_gmt.lut[i].G_GAMMA;
        pGammaAccessOut->gamma.b_tbl[i] = pGGM->rb_gmt.lut[i].B_GAMMA;
        MY_LOG("(r, g, b)[%d] = (%d, %d, %d)", i, pGammaAccessOut->gamma.r_tbl[i], pGammaAccessOut->gamma.g_tbl[i], pGammaAccessOut->gamma.b_tbl[i]);
    }

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE ) done\n");
#endif
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE )
{
#if 1//FIXED==1
    typedef ACDK_CCT_GAMMA_ACCESS_STRUCT i_type;
    int i;

    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pGammaAccess = reinterpret_cast<i_type*>(puParaIn);
    MUINT32 const setIndex = pGammaAccess->index;
    MBOOL bIHDR = pGammaAccess->bIHDR;
    /*if (bIHDR) m_rISPRegsIdx.IHDR_GGM = setIndex;
    else*/ m_rISPRegsIdx.GGM = setIndex;
    ISP_NVRAM_GGM_T ggm;

    MY_LOG("[ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE] setIndex, bIHDR, m_rISPRegsIdx.GGM = (%d, %d, %d)\n"
        , setIndex, bIHDR, m_rISPRegsIdx.GGM);


    for(i=0;i<GAMMA_STEP_NO; i++) {
/*
        m_rISPRegs.GGM[index].rb_gmt.lut[i].R_GAMMA = pGamma->r_tbl[i];
        m_rISPRegs.GGM[index].g_gmt.lut[i].G_GAMMA = pGamma->g_tbl[i];
        m_rISPRegs.GGM[index].rb_gmt.lut[i].B_GAMMA = pGamma->b_tbl[i];
*/
        /*if (pGammaAccess->bIHDR)
        {
            ggm.lut_rb.lut[i].GGM_R = m_rISPRegs.IHDR_GGM[setIndex].lut_rb.lut[i].GGM_R = pGammaAccess->gamma.r_tbl[i];
            ggm.lut_g.lut[i].GGM_G = m_rISPRegs.IHDR_GGM[setIndex].lut_g.lut[i].GGM_G = pGammaAccess->gamma.g_tbl[i];
            ggm.lut_rb.lut[i].GGM_B = m_rISPRegs.IHDR_GGM[setIndex].lut_rb.lut[i].GGM_B = pGammaAccess->gamma.b_tbl[i];
        }
        else*/
        {
            ggm.rb_gmt.lut[i].R_GAMMA = m_rISPRegs.GGM[setIndex].rb_gmt.lut[i].R_GAMMA = pGammaAccess->gamma.r_tbl[i];
            ggm.g_gmt.lut[i].G_GAMMA = m_rISPRegs.GGM[setIndex].g_gmt.lut[i].G_GAMMA = pGammaAccess->gamma.g_tbl[i];
            ggm.rb_gmt.lut[i].B_GAMMA = m_rISPRegs.GGM[setIndex].rb_gmt.lut[i].B_GAMMA = pGammaAccess->gamma.b_tbl[i];
        }
        MY_LOG("(r, g, b)[%d] = (%d, %d, %d)", i, pGammaAccess->gamma.r_tbl[i], pGammaAccess->gamma.g_tbl[i], pGammaAccess->gamma.b_tbl[i]);
    }

    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(ggm);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //NSIspTuning::ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE ) done\n");
#endif
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS )
{

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MBOOL fgEnable = reinterpret_cast<type*>(puParaIn)->Enable ? MFALSE : MTRUE;
#if 1//FIXED==1
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgEnable);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS ) done, %d\n", fgEnable);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG )
{
#if 1//FIXED==1
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    reinterpret_cast<o_type*>(puParaOut)->Enable
        = (ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable()) ? MFALSE : MTRUE;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG ) done, %d\n", reinterpret_cast<o_type*>(puParaOut)->Enable);
#endif
    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);
    ISP_NVRAM_CCM_T ccm;

    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).get(ccm);
#if 1//FIXED==1
    pDst->M11 = ccm.conv0a.bits.G2G_CNV_00;
    pDst->M12 = ccm.conv0a.bits.G2G_CNV_01;
    pDst->M13 = ccm.conv0b.bits.G2G_CNV_02;
    pDst->M21 = ccm.conv1a.bits.G2G_CNV_10;
    pDst->M22 = ccm.conv1a.bits.G2G_CNV_11;
    pDst->M23 = ccm.conv1b.bits.G2G_CNV_12;
    pDst->M31 = ccm.conv2a.bits.G2G_CNV_20;
    pDst->M32 = ccm.conv2a.bits.G2G_CNV_21;
    pDst->M33 = ccm.conv2b.bits.G2G_CNV_22;
#endif
    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM]\n");
    MY_LOG("M11 0x%03X\n", pDst->M11);
    MY_LOG("M12 0x%03X\n", pDst->M12);
    MY_LOG("M13 0x%03X\n", pDst->M13);
    MY_LOG("M21 0x%03X\n", pDst->M21);
    MY_LOG("M22 0x%03X\n", pDst->M22);
    MY_LOG("M23 0x%03X\n", pDst->M23);
    MY_LOG("M31 0x%03X\n", pDst->M31);
    MY_LOG("M32 0x%03X\n", pDst->M32);
    MY_LOG("M33 0x%03X\n", pDst->M33);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM ) done\n");

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_CCM_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_CCM_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM]\n");

    ISP_NVRAM_CCM_T rDst;
    type*const       pSrc = reinterpret_cast<type*>(puParaIn);

    rDst.conv0a.bits.G2G_CNV_00 = pSrc->M11;
    rDst.conv0a.bits.G2G_CNV_01 = pSrc->M12;
    rDst.conv0b.bits.G2G_CNV_02 = pSrc->M13;
    rDst.conv1a.bits.G2G_CNV_10 = pSrc->M21;
    rDst.conv1a.bits.G2G_CNV_11 = pSrc->M22;
    rDst.conv1b.bits.G2G_CNV_12 = pSrc->M23;
    rDst.conv2a.bits.G2G_CNV_20 = pSrc->M31;
    rDst.conv2a.bits.G2G_CNV_21 = pSrc->M32;
    rDst.conv2b.bits.G2G_CNV_22 = pSrc->M33;

#if 0//FIXED==1
    rDst.cnv_1.bits.G2G_CNV_00 = pSrc->M11;
    rDst.cnv_1.bits.G2G_CNV_01 = pSrc->M12;
    rDst.cnv_2.bits.G2G_CNV_02 = pSrc->M13;
    rDst.cnv_3.bits.G2G_CNV_10 = pSrc->M21;
    rDst.cnv_3.bits.G2G_CNV_11 = pSrc->M22;
    rDst.cnv_4.bits.G2G_CNV_12 = pSrc->M23;
    rDst.cnv_5.bits.G2G_CNV_20 = pSrc->M31;
    rDst.cnv_5.bits.G2G_CNV_21 = pSrc->M32;
    rDst.cnv_6.bits.G2G_CNV_22 = pSrc->M33;
#endif
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rDst);
#if 1//FIXED==1
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //NSIspTuning::ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    usleep(200000);
    MY_LOG("M11 0x%03X", pSrc->M11);
    MY_LOG("M12 0x%03X", pSrc->M12);
    MY_LOG("M13 0x%03X", pSrc->M13);
    MY_LOG("M21 0x%03X", pSrc->M21);
    MY_LOG("M22 0x%03X", pSrc->M22);
    MY_LOG("M23 0x%03X", pSrc->M23);
    MY_LOG("M31 0x%03X", pSrc->M31);
    MY_LOG("M32 0x%03X", pSrc->M32);
    MY_LOG("M33 0x%03X", pSrc->M33);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM ) done\n");
    return  CCTIF_NO_ERROR;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    MUINT32 u4Index
u4ParaInLen
    sizeof(MUINT32)
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    typedef MUINT32             i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const index = *reinterpret_cast<i_type*>(puParaIn);
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rSrc = m_rISPRegs.CCM[index];
    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);
#if 1//FIXED==1
    pDst->M11 = rSrc.conv0a.bits.G2G_CNV_00;
    pDst->M12 = rSrc.conv0a.bits.G2G_CNV_01;
    pDst->M13 = rSrc.conv0b.bits.G2G_CNV_02;
    pDst->M21 = rSrc.conv1a.bits.G2G_CNV_10;
    pDst->M22 = rSrc.conv1a.bits.G2G_CNV_11;
    pDst->M23 = rSrc.conv1b.bits.G2G_CNV_12;
    pDst->M31 = rSrc.conv2a.bits.G2G_CNV_20;
    pDst->M32 = rSrc.conv2a.bits.G2G_CNV_21;
    pDst->M33 = rSrc.conv2b.bits.G2G_CNV_22;
#endif
    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to get: %d", index);

    MY_LOG("M11 0x%03X", pDst->M11);
    MY_LOG("M12 0x%03X", pDst->M12);
    MY_LOG("M13 0x%03X", pDst->M13);
    MY_LOG("M21 0x%03X", pDst->M21);
    MY_LOG("M22 0x%03X", pDst->M22);
    MY_LOG("M23 0x%03X", pDst->M23);
    MY_LOG("M31 0x%03X", pDst->M31);
    MY_LOG("M32 0x%03X", pDst->M32);
    MY_LOG("M33 0x%03X", pDst->M33);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM ) done\n");
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_SET_NVRAM_CCM
u4ParaInLen
    sizeof(ACDK_CCT_SET_NVRAM_CCM)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM )
{
    typedef ACDK_CCT_SET_NVRAM_CCM type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type& rInParam = *reinterpret_cast<type*>(puParaIn);
    MUINT32 const index = rInParam.u4Index;
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rDst = m_rISPRegs.CCM[index];

    rDst.conv0a.bits.G2G_CNV_00 = rInParam.ccm.M11;
    rDst.conv0a.bits.G2G_CNV_01 = rInParam.ccm.M12;
    rDst.conv0b.bits.G2G_CNV_02 = rInParam.ccm.M13;
    rDst.conv1a.bits.G2G_CNV_10 = rInParam.ccm.M21;
    rDst.conv1a.bits.G2G_CNV_11 = rInParam.ccm.M22;
    rDst.conv1b.bits.G2G_CNV_12 = rInParam.ccm.M23;
    rDst.conv2a.bits.G2G_CNV_20 = rInParam.ccm.M31;
    rDst.conv2a.bits.G2G_CNV_21 = rInParam.ccm.M32;
    rDst.conv2b.bits.G2G_CNV_22 = rInParam.ccm.M33;

#if 0//FIXED==1
    rDst.cnv_1.bits.G2G_CNV_00 = rInParam.ccm.M11;
    rDst.cnv_1.bits.G2G_CNV_01 = rInParam.ccm.M12;
    rDst.cnv_2.bits.G2G_CNV_02 = rInParam.ccm.M13;
    rDst.cnv_3.bits.G2G_CNV_10 = rInParam.ccm.M21;
    rDst.cnv_3.bits.G2G_CNV_11 = rInParam.ccm.M22;
    rDst.cnv_4.bits.G2G_CNV_12 = rInParam.ccm.M23;
    rDst.cnv_5.bits.G2G_CNV_20 = rInParam.ccm.M31;
    rDst.cnv_5.bits.G2G_CNV_21 = rInParam.ccm.M32;
    rDst.cnv_6.bits.G2G_CNV_22 = rInParam.ccm.M33;
#endif
    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to set: %d", index);
    for (MUINT32 i = 0; i < ISP_NVRAM_CCM_T::COUNT; i++)
    {
        MY_LOG("CCM: [%d] 0x%06X", i, rDst.set[i]);
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM ) done\n");
    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
pu4RealParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA )
{
    typedef ACDK_CCT_NVRAM_CCM_PARA o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
        #if 1//FIXED==1
        pDst->ccm[i].M11 = m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_00;
        pDst->ccm[i].M12 = m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_01;
        pDst->ccm[i].M13 = m_rISPRegs.CCM[i].conv0b.bits.G2G_CNV_02;
        pDst->ccm[i].M21 = m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_10;
        pDst->ccm[i].M22 = m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_11;
        pDst->ccm[i].M23 = m_rISPRegs.CCM[i].conv1b.bits.G2G_CNV_12;
        pDst->ccm[i].M31 = m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_20;
        pDst->ccm[i].M32 = m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_21;
        pDst->ccm[i].M33 = m_rISPRegs.CCM[i].conv2b.bits.G2G_CNV_22;
        #endif

        MY_LOG("pDst->ccm[%d].M11 = %d", i, pDst->ccm[i].M11);
        MY_LOG("pDst->ccm[%d].M12 = %d", i, pDst->ccm[i].M12);
        MY_LOG("pDst->ccm[%d].M13 = %d", i, pDst->ccm[i].M13);
        MY_LOG("pDst->ccm[%d].M21 = %d", i, pDst->ccm[i].M21);
        MY_LOG("pDst->ccm[%d].M22 = %d", i, pDst->ccm[i].M22);
        MY_LOG("pDst->ccm[%d].M23 = %d", i, pDst->ccm[i].M23);
        MY_LOG("pDst->ccm[%d].M31 = %d", i, pDst->ccm[i].M31);
        MY_LOG("pDst->ccm[%d].M32 = %d", i, pDst->ccm[i].M32);
        MY_LOG("pDst->ccm[%d].M33 = %d", i, pDst->ccm[i].M33);

        MY_LOG("pDst->ccm[%d].conv0a.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv0a.bits.rsv_11);
        MY_LOG("pDst->ccm[%d].conv0a.bits.rsv_27 = %d", i, m_rISPRegs.CCM[i].conv0a.bits.rsv_27);
        MY_LOG("pDst->ccm[%d].conv0b.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv0b.bits.rsv_11);
        MY_LOG("pDst->ccm[%d].conv1a.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv1a.bits.rsv_11);
        MY_LOG("pDst->ccm[%d].conv1a.bits.rsv_27 = %d", i, m_rISPRegs.CCM[i].conv1a.bits.rsv_27);
        MY_LOG("pDst->ccm[%d].conv1b.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv1b.bits.rsv_11);
        MY_LOG("pDst->ccm[%d].conv2a.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv2a.bits.rsv_11);
        MY_LOG("pDst->ccm[%d].conv2a.bits.rsv_27 = %d", i, m_rISPRegs.CCM[i].conv2a.bits.rsv_27);
        MY_LOG("pDst->ccm[%d].conv2b.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv2b.bits.rsv_11);
    }

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA ) done\n");
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaInLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA )
{
    typedef ACDK_CCT_NVRAM_CCM_PARA i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const       pSrc = reinterpret_cast<i_type*>(puParaIn);

    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
        memset(&m_rISPRegs.CCM[i],0,sizeof(ISP_NVRAM_CCM_T));

        m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_00 = pSrc->ccm[i].M11;
        m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_01 = pSrc->ccm[i].M12;
        m_rISPRegs.CCM[i].conv0b.bits.G2G_CNV_02 = pSrc->ccm[i].M13;
        m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_10 = pSrc->ccm[i].M21;
        m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_11 = pSrc->ccm[i].M22;
        m_rISPRegs.CCM[i].conv1b.bits.G2G_CNV_12 = pSrc->ccm[i].M23;
        m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_20 = pSrc->ccm[i].M31;
        m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_21 = pSrc->ccm[i].M32;
        m_rISPRegs.CCM[i].conv2b.bits.G2G_CNV_22 = pSrc->ccm[i].M33;

        MY_LOG("pSrc->ccm[%d].M11 = %d", i, pSrc->ccm[i].M11);
        MY_LOG("pSrc->ccm[%d].M12 = %d", i, pSrc->ccm[i].M12);
        MY_LOG("pSrc->ccm[%d].M13 = %d", i, pSrc->ccm[i].M13);
        MY_LOG("pSrc->ccm[%d].M21 = %d", i, pSrc->ccm[i].M21);
        MY_LOG("pSrc->ccm[%d].M22 = %d", i, pSrc->ccm[i].M22);
        MY_LOG("pSrc->ccm[%d].M23 = %d", i, pSrc->ccm[i].M23);
        MY_LOG("pSrc->ccm[%d].M31 = %d", i, pSrc->ccm[i].M31);
        MY_LOG("pSrc->ccm[%d].M32 = %d", i, pSrc->ccm[i].M32);
        MY_LOG("pSrc->ccm[%d].M33 = %d", i, pSrc->ccm[i].M33);

        MY_LOG("pSrc->ccm[%d].conv0a.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv0a.bits.rsv_11);
        MY_LOG("pSrc->ccm[%d].conv0a.bits.rsv_27 = %d", i, m_rISPRegs.CCM[i].conv0a.bits.rsv_27);
        MY_LOG("pSrc->ccm[%d].conv0b.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv0b.bits.rsv_11);
        MY_LOG("pSrc->ccm[%d].conv1a.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv1a.bits.rsv_11);
        MY_LOG("pSrc->ccm[%d].conv1a.bits.rsv_27 = %d", i, m_rISPRegs.CCM[i].conv1a.bits.rsv_27);
        MY_LOG("pSrc->ccm[%d].conv1b.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv1b.bits.rsv_11);
        MY_LOG("pSrc->ccm[%d].conv2a.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv2a.bits.rsv_11);
        MY_LOG("pSrc->ccm[%d].conv2a.bits.rsv_27 = %d", i, m_rISPRegs.CCM[i].conv2a.bits.rsv_27);
        MY_LOG("pSrc->ccm[%d].conv2b.bits.rsv_11 = %d", i, m_rISPRegs.CCM[i].conv2b.bits.rsv_11);

#if 0//FIXED==1
        m_rISPRegs.CCM[i].cnv_1.bits.G2G_CNV_00 = pSrc->ccm[i].M11;
        m_rISPRegs.CCM[i].cnv_1.bits.G2G_CNV_01 = pSrc->ccm[i].M12;
        m_rISPRegs.CCM[i].cnv_2.bits.G2G_CNV_02 = pSrc->ccm[i].M13;
        m_rISPRegs.CCM[i].cnv_3.bits.G2G_CNV_10 = pSrc->ccm[i].M21;
        m_rISPRegs.CCM[i].cnv_3.bits.G2G_CNV_11 = pSrc->ccm[i].M22;
        m_rISPRegs.CCM[i].cnv_4.bits.G2G_CNV_12 = pSrc->ccm[i].M23;
        m_rISPRegs.CCM[i].cnv_5.bits.G2G_CNV_20 = pSrc->ccm[i].M31;
        m_rISPRegs.CCM[i].cnv_5.bits.G2G_CNV_21 = pSrc->ccm[i].M32;
        m_rISPRegs.CCM[i].cnv_6.bits.G2G_CNV_22 = pSrc->ccm[i].M33;
#endif
    }

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA ) done\n");
    return  CCTIF_NO_ERROR;

}


/*
puParaIn
    ISP_NVRAM_CCM_AWB_GAIN_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_CCM_AWB_GAIN_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
#if 1//FIXED==1
IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_WB )
{
    typedef ISP_NVRAM_CCM_AWB_GAIN_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const       pSrc = reinterpret_cast<i_type*>(puParaIn);

    m_rBuf_ISP.ISPMulitCCM.AWBGain = *pSrc;
    MY_LOG("pSrc->rA.i4R = %d, nvram buf = %d\n", pSrc->rA.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4R);
    MY_LOG("pSrc->rA.i4G = %d, nvram buf = %d\n", pSrc->rA.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4G);
    MY_LOG("pSrc->rA.i4B = %d, nvram buf = %d\n", pSrc->rA.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4B);
    MY_LOG("pSrc->rD65.i4R = %d, nvram buf = %d\n", pSrc->rD65.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4R);
    MY_LOG("pSrc->rD65.i4G = %d, nvram buf = %d\n", pSrc->rD65.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4G);
    MY_LOG("pSrc->rD65.i4B = %d, nvram buf = %d\n", pSrc->rD65.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4B);


    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_WB ) done\n");
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ISP_NVRAM_CCM_AWB_GAIN_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_CCM_AWB_GAIN_STRUCT)
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_WB )
{
    typedef ISP_NVRAM_CCM_AWB_GAIN_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type* const pDst = reinterpret_cast<o_type*>(puParaOut);

    *pDst = m_rBuf_ISP.ISPMulitCCM.AWBGain;
    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("pDst->rA.i4R = %d, nvram buf = %d\n", pDst->rA.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4R);
    MY_LOG("pDst->rA.i4G = %d, nvram buf = %d\n", pDst->rA.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4G);
    MY_LOG("pDst->rA.i4B = %d, nvram buf = %d\n", pDst->rA.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4B);
    MY_LOG("pDst->rD65.i4R = %d, nvram buf = %d\n", pDst->rD65.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4R);
    MY_LOG("pDst->rD65.i4G = %d, nvram buf = %d\n", pDst->rD65.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4G);
    MY_LOG("pDst->rD65.i4B = %d, nvram buf = %d\n", pDst->rD65.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4B);


    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_WB ) done\n");
    return  CCTIF_NO_ERROR;

}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM )
{
#if 0
    MBOOL fgEnableDynamicCCM = true;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif

    MY_LOG("Enable Dynamic CCM!!\n");
#if 1//FIXED==1
    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicCCM((MINT32)m_eSensorEnum, MTRUE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;
#else
    return CCTIF_NO_ERROR;
#endif

}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM )
{

    MY_LOG("Disable Dynamic CCM!!\n");
#if 1//FIXED==1
    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicCCM((MINT32)m_eSensorEnum, MFALSE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;
#else
    return CCTIF_NO_ERROR;
#endif
}

/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS )
{
    return  CCTIF_NO_ERROR;
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS )
{

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    MINT32 en;
    CCTIF_ERROR_ENUM err_status;

    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS]\n");
#if 1//FIXED==1
    en = NSIspTuning::IspTuningMgr::getInstance().getDynamicCCM((MINT32)m_eSensorEnum);
#endif
    if(en < 0)
    {
        err_status = CCTIF_UNKNOWN_ERROR;
    }
    else {
        reinterpret_cast<o_type*>(puParaOut)->Enable = en;
        err_status = CCTIF_NO_ERROR;
    }

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS ) done, getDynamicCCM() = %d\n", en);

    return err_status;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(MUINT32);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE )
{
    typedef MUINT32 i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const u4Index = *reinterpret_cast<i_type const*>(puParaIn);

    if  ( IspNvramRegMgr::NUM_CCM <= u4Index )
    {
        return  CCTIF_BAD_PARAM;
    }

    m_rISPRegsIdx.CCM = static_cast<MUINT8>(u4Index);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE ) done, CCM Index: (old, new)=(%d, %d)", m_rISPRegsIdx.CCM, u4Index);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
u4ParaInLen
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_MODE )
{
    typedef MUINT32 o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    *reinterpret_cast<o_type*>(puParaOut) = m_rISPRegsIdx.CCM;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("[ACDK_CCT_OP_GET_CCM_MODE] Current CCM Index: %d", m_rISPRegsIdx.CCM);
    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CctCtrl::
dumpIspReg(MUINT32 const u4Addr) const
{
#if 1
    MY_LOG("[dumpIspReg] isp reg:%04X = 0x%08X", u4Addr, m_pIspDrv->readReg(u4Addr));
#endif
}


MVOID
CctCtrl::
setIspOnOff_OBC(MBOOL const fgOn)
{
    #if 1//FIXED==1
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
        #endif
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_BPC(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BPC;

    //should not modify NVRAM value
    //m_rISPRegs.BPC[u4Index].con.bits.BPC_ENABLE = fgOn;
#if 1//FIXED==1
    //m_rISPRegs.BPC[u4Index].con.bits.BPC_EN = fgOn; //definition change: CAM_BPC_CON CAM+0800H

    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setBPCEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctCtrl::
setIspOnOff_NR1(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.NR1;

    //should not modify NVRAM value
    //m_rISPRegs.NR1[u4Index].con.bits.NR1_CT_EN = fgOn;
#if 1//FIXED==1
    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCTEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
        #endif
    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_SL2(MBOOL const fgOn)
{
    #if 1//FIXED==1
    ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
        #endif
    ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctCtrl::
setIspOnOff_CFA(MBOOL const fgOn)
{
    MUINT32 u4Index = m_rISPRegsIdx.CFA;
#if 1//FIXED==1
    ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);

    (fgOn == MTRUE) ? (ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).put(m_rISPRegs.CFA[u4Index]))
                    : (ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).put(m_rISPRegs.CFA[NVRAM_CFA_TBL_NUM - 1]));

    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctCtrl::
setIspOnOff_CCM(MBOOL const fgOn)
{
#if 1//FIXED==1
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_GGM(MBOOL const fgOn)
{
#if 1//FIXED==1
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_PCA(MBOOL const fgOn)
{
#if 1//FIXED==1
    ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}



MVOID
CctCtrl::
setIspOnOff_ANR(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.ANR;
    ISP_CAM_ANR_CON1_T& ANRCtrl1 = m_rISPRegs.ANR[u4Index].con1.bits;
#if 0 //should not modify NVRAM value
    ANRCtrl1.ANR_ENC = fgOn;
    ANRCtrl1.ANR_ENY = fgOn;
#endif
#if 1//FIXED==1
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setANREnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_CCR(MBOOL const fgOn)
{
#if 1//FIXED==1
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCREnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctCtrl::
setIspOnOff_EE(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.EE;

    ISP_CAM_SEEE_SRK_CTRL_T &EECtrl_SRK = m_rISPRegs.EE[u4Index].srk_ctrl.bits;
    ISP_CAM_SEEE_CLIP_CTRL_T &EECtrl_Clip = m_rISPRegs.EE[u4Index].clip_ctrl.bits;

    //should not modify NVRAM value
    //EECtrl_SRK.USM_OVER_SHRINK_EN = fgOn;
    //EECtrl_Clip.USM_OVER_CLIP_EN = fgOn;
#if 1//FIXED==1
    //should not modify NVRAM value
    //EECtrl_Clip.SEEE_OVRSH_CLIP_EN = fgOn;//definition change: CAM_SEEE_CLIP_CTRL CAM+AA4H

    ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_NR3D(MBOOL const fgOn)
{
    //N.A.
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_MFB(MBOOL const fgOn)
{
    //Temp. Mark
#if 0//FIXED==1
    ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
#endif
    //ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_LCE(MBOOL const fgOn)
{
    //ISP_MGR_LCE_T::getInstance((ESensorDev_T)m_eSensorEnum).setLceEnable(fgOn);
    //NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_LCE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    //MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MBOOL
CctCtrl::
getIspOnOff_OBC() const
{
    #if 1//FIXME==1
    return ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
    #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_BPC() const
{
    #if 1//FIXED==1
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isBPCEnable();
    #else
    return 0;
    #endif
}


MBOOL
CctCtrl::
getIspOnOff_NR1() const
{
    #if 1//FIXED==1
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCTEnable();
            #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_SL2() const
{
    #if 1//FIXED==1
    return  ISP_MGR_SL2::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
            #else
    return 0;
    #endif
}


MBOOL
CctCtrl::
getIspOnOff_CFA() const
{
    #if 1//FIXED==1
    return  ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
            #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_CCM() const
{
        #if 1//FIXED==1
    return ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
            #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_GGM() const
{
        #if 1//FIXED==1
    return  ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
            #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_PCA() const
{
        #if 1//FIXED==1
    return  ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
            #else
    return 0;
    #endif
}



MBOOL
CctCtrl::
getIspOnOff_ANR() const
{
        #if 1//FIXED==1
    return (ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isANREnable());
            #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_CCR() const
{
    #if 1//FIXED==1
    return ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCREnable();
    #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_EE() const
{
    #if 1//FIXED==1
    return ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
    #else
    return 0;
    #endif
}

MBOOL
CctCtrl::
getIspOnOff_NR3D() const
{
    return MTRUE;
}

MBOOL
CctCtrl::
getIspOnOff_MFB() const
{
    return MTRUE;
    //Temp. Mark
    //return ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MBOOL
CctCtrl::
getIspOnOff_LCE() const
{
    //return  ISP_MGR_LCE::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
    return MFALSE;
}


MINT32
CctCtrl::
setIspOnOff(MUINT32 const u4Category, MBOOL const fgOn)
{
#define SET_ISP_ON_OFF(_category)\
    case EIsp_Category_##_category:\
        setIspOnOff_##_category(fgOn);\
        MY_LOG("[setIspOnOff] < %s >", #_category);\
        break

    switch  ( u4Category )
    {
        SET_ISP_ON_OFF(OBC);
        SET_ISP_ON_OFF(BPC);
        SET_ISP_ON_OFF(NR1);
        SET_ISP_ON_OFF(SL2);
        SET_ISP_ON_OFF(CFA);
        SET_ISP_ON_OFF(CCM); // only for set/get on/off, not for set/get tuning index/para
        //SET_ISP_ON_OFF(GGM);
        //SET_ISP_ON_OFF(IHDR_GGM);
        SET_ISP_ON_OFF(ANR);
        SET_ISP_ON_OFF(CCR);
        SET_ISP_ON_OFF(EE);
        //SET_ISP_ON_OFF(NR3D);
        //SET_ISP_ON_OFF(MFB);
        //SET_ISP_ON_OFF(LCE);

        default:
            MY_ERR("[setIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }
    MY_LOG("[%s] (u4Category, fgOn) = (%d, %d)", __FUNCTION__, u4Category, fgOn);
    return  CCTIF_NO_ERROR;
}


MINT32
CctCtrl::
getIspOnOff(MUINT32 const u4Category, MBOOL& rfgOn) const
{
#define GET_ISP_ON_OFF(_category)\
    case EIsp_Category_##_category:\
        MY_LOG("[getIspOnOff] < %s >", #_category);\
        rfgOn = getIspOnOff_##_category();\
        break

    switch  ( u4Category )
    {
        GET_ISP_ON_OFF(OBC);
        GET_ISP_ON_OFF(BPC);
        GET_ISP_ON_OFF(NR1);
        //GET_ISP_ON_OFF(LSC);
        GET_ISP_ON_OFF(SL2);
        GET_ISP_ON_OFF(CFA);
        GET_ISP_ON_OFF(CCM); // only for set/get on/off, not for set/get tuning index/para
        //GET_ISP_ON_OFF(GGM);
        //GET_ISP_ON_OFF(IHDR_GGM);
        GET_ISP_ON_OFF(ANR);
        GET_ISP_ON_OFF(CCR);
        GET_ISP_ON_OFF(EE);
        //GET_ISP_ON_OFF(NR3D);
        //GET_ISP_ON_OFF(MFB);
        //GET_ISP_ON_OFF(LCE);

        default:
            MY_ERR("[getIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }
    MY_LOG("[%s] (u4Category, rfgOn) = (%d, %d)", __FUNCTION__, u4Category, rfgOn);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_ON )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 1);

    MY_LOG("[-ACDK_CCT_OP_SET_ISP_ON] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_OFF )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 0);

    MY_LOG("[ACDK_CCT_OP_SET_ISP_OFF] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_ISP_ON_OFF )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY       i_type;
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    MBOOL&       rfgEnable = reinterpret_cast<o_type*>(puParaOut)->Enable;

    MINT32 const err = getIspOnOff(eCategory, rfgEnable);

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[-ACDK_CCT_OP_GET_ISP_ON_OFF] (eCategory, rfgEnable)=(%d, %d)", eCategory, rfgEnable);
    return  err;
}



/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )");

    void* pBuf; //temp used
    //int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, m_eSensorEnum, (void*&)pBuf, 1);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_3A_LOAD_FROM_NVRAM] CAMERA_NVRAM_DATA_ISP read fail err=%d(0x%x)\n",err, err);
        return  err;
    }
    //
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM ) done");
    return  err;
}

IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM )");

    void* pBuf; //temp used
    //int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorEnum, (void*&)pBuf, 1);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_3A_LOAD_FROM_NVRAM] CAMERA_NVRAM_DATA_3A read fail err=%d(0x%x)\n",err, err);
        return  err;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM ) done 3A");
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_eSensorEnum, (void*&)pBuf, 1);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_3A_LOAD_FROM_NVRAM] CAMERA_NVRAM_DATA_LENS read fail err=%d(0x%x)\n",err, err);
        return  err;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM ) done LENS");
    return  err;
}



IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )");

    //int err;
    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_ISP, m_eSensorEnum);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SAVE_TO_NVRAM] write fail err=%d(0x%x)\n", err, err);
        return  err;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM ) done");
    return  err;
}

#if 1
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE )
{

    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaIn);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;
#if 1//FIXED==1
    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA, 2 : flash PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_lo[u4Offset] : pAccess->u8Index == 1 ? &m_rISPPca.PCA_LUTS.lut_lo[u4Offset] : &m_rISPPca.PCA_LUTS.lut_lo3[u4Offset];
        break;
    case 1:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA, 2 : flash PCA  */) ?
            &m_rISPPca.PCA_LUTS.lut_md[u4Offset] : pAccess->u8Index == 1 ? &m_rISPPca.PCA_LUTS.lut_md[u4Offset] : &m_rISPPca.PCA_LUTS.lut_md3[u4Offset];
        break;
    case 2:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA, 2 : flash PCA  */) ?
            &m_rISPPca.PCA_LUTS.lut_hi[u4Offset] : pAccess->u8Index == 1 ? &m_rISPPca.PCA_LUTS.lut_hi[u4Offset] : &m_rISPPca.PCA_LUTS.lut_hi3[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }
#endif
    ::memcpy(pBuf_fw, pBuf_pc, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));

    for (int i=0; i<u4Count/10; i++)
    {
        MY_LOG("pBuf_fw[%d-%d] = (%x, %x, %x, %x, %x, %x, %x, %x, %x, %x)\n", i*10+0, i*10+9
            , pBuf_fw[i*10+0]
            , pBuf_fw[i*10+1]
            , pBuf_fw[i*10+2]
            , pBuf_fw[i*10+3]
            , pBuf_fw[i*10+4]
            , pBuf_fw[i*10+5]
            , pBuf_fw[i*10+6]
            , pBuf_fw[i*10+7]
            , pBuf_fw[i*10+8]
            , pBuf_fw[i*10+9]);
    }


    ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).loadLut((MUINT32 *)pBuf_fw);
#if FIXME==1
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
    //NSIspTuning::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE ) done, (u4Offset, u4Count, u8ColorTemperature, u8Index) = (%d, %d, %d, %d)"
        , u4Offset, u4Count, u8ColorTemperature, pAccess->u8Index);
#endif
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE )
{
    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaOut);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;
#if 1//FIXED==1
    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA, 2 : flash PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_lo[u4Offset] : pAccess->u8Index == 1 ? &m_rISPPca.PCA_LUTS.lut_lo[u4Offset] : &m_rISPPca.PCA_LUTS.lut_lo3[u4Offset];
        break;
    case 1:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA, 2 : flash PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_md[u4Offset] : pAccess->u8Index == 1 ? &m_rISPPca.PCA_LUTS.lut_md[u4Offset] : &m_rISPPca.PCA_LUTS.lut_md3[u4Offset];
        break;
    case 2:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA, 2 : flash PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_hi[u4Offset] : pAccess->u8Index == 1 ? &m_rISPPca.PCA_LUTS.lut_hi[u4Offset] : &m_rISPPca.PCA_LUTS.lut_hi3[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }
#endif
    ::memcpy(pBuf_pc, pBuf_fw, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));

    for (int i=0; i<u4Count/10; i++)
    {
        MY_LOG("pBuf_pc[%d-%d] = (%x, %x, %x, %x, %x, %x, %x, %x, %x, %x)\n", i*10+0, i*10+9
            , pBuf_pc[i*10+0]
            , pBuf_pc[i*10+1]
            , pBuf_pc[i*10+2]
            , pBuf_pc[i*10+3]
            , pBuf_pc[i*10+4]
            , pBuf_pc[i*10+5]
            , pBuf_pc[i*10+6]
            , pBuf_pc[i*10+7]
            , pBuf_pc[i*10+8]
            , pBuf_pc[i*10+9]);
    }

    *pu4RealParaOutLen = sizeof(type);
#if 1//FIXED==1
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE ) done, (u4Offset, u4Count, u8ColorTemperature, u8Index) = (%d, %d, %d, %d)"
        , u4Offset, u4Count, u8ColorTemperature, pAccess->u8Index);
#endif
    return  CCTIF_NO_ERROR;


}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA )
{

    typedef ACDK_CCT_ACCESS_PCA_CONFIG i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pAccess = reinterpret_cast<i_type*>(puParaIn);

    setIspOnOff_PCA((MBOOL)pAccess->EN);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA ) done, %d", pAccess->EN);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA )
{

    typedef ACDK_CCT_ACCESS_PCA_CONFIG o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pAccess = reinterpret_cast<o_type*>(puParaOut);

    pAccess->EN = getIspOnOff_PCA();
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA ) done, %d", pAccess->EN);

    return  CCTIF_NO_ERROR;

}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_PCA_SLIDER
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_PCA_SLIDER)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_SLIDER )
{

    typedef ACDK_CCT_ACCESS_PCA_SLIDER i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pca = reinterpret_cast<i_type*>(puParaIn);
    ::memcpy(&m_rISPPca.Slider, &pca->slider, sizeof(ACDK_CCT_ACCESS_PCA_SLIDER));
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_SLIDER ) done");

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_PCA_SLIDER
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_SLIDER)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_SLIDER)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_SLIDER )
{

    typedef ACDK_CCT_ACCESS_PCA_SLIDER o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pca = reinterpret_cast<o_type*>(puParaOut);
    ::memcpy(&pca->slider, &m_rISPPca.Slider, sizeof(ACDK_CCT_ACCESS_PCA_SLIDER));

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_SLIDER ) done");

    return  CCTIF_NO_ERROR;

}
#endif

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF )
{
    MBOOL fgOnOff;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT i_type;
    if ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);

    fgOnOff = pShadingPara->Enable;
    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    pLscMgr->setOnOff(fgOnOff);
    pLscMgr->updateLsc();
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);

    MY_LOG("[%s] SET_SHADING_ON_OFF(%s)", __FUNCTION__, (fgOnOff ? "On":"Off"));

    return CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF )
{
    MBOOL fgOnOff;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT o_type;
    if ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    fgOnOff = pLscMgr->getOnOff();
    pShadingPara->Enable = fgOnOff;

    MY_LOG("[%s] GET_SHADING_ON_OFF(%s)", __FUNCTION__, (fgOnOff ? "On":"Off"));

    return CCTIF_NO_ERROR;
}
/*******************************************************************************
* Because CCT tool is working on preview mode (Lsc_mgr.m_u4Mode = 0)
* 1.
*    Capture parameters will not be update to isp register
*    (since Lsc_mgr.m_u4Mode doesn't changed) till capture command
* 2.
*     Preivew parameters will be updated to reigster immediately at "prepareHw_PerFrame_Shading()"
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA )
{
    typedef ACDK_CCT_SHADING_COMP_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);
    ESensorMode_T eSensorMode = (ESensorMode_T)pShadingPara->SHADING_MODE;

    MY_LOG("[%s +] SET_SHADING_PARA SensorMode(%d)", __FUNCTION__, eSensorMode);

    ISP_NVRAM_LSC_T rLscCfg;
    rLscCfg.ctl2.bits.SDBLK_XNUM        = pShadingPara->pShadingComp->SHADINGBLK_XNUM - 1;
    rLscCfg.ctl3.bits.SDBLK_YNUM        = pShadingPara->pShadingComp->SHADINGBLK_YNUM - 1;
    rLscCfg.ctl2.bits.SDBLK_WIDTH       = pShadingPara->pShadingComp->SHADINGBLK_WIDTH;
    rLscCfg.ctl3.bits.SDBLK_HEIGHT      = pShadingPara->pShadingComp->SHADINGBLK_HEIGHT;
    rLscCfg.lblock.bits.SDBLK_lWIDTH    = pShadingPara->pShadingComp->SD_LWIDTH;
    rLscCfg.lblock.bits.SDBLK_lHEIGHT   = pShadingPara->pShadingComp->SD_LHEIGHT;
    rLscCfg.ratio.bits.RATIO00              = pShadingPara->pShadingComp->SDBLK_RATIO00;
    rLscCfg.ratio.bits.RATIO01              = pShadingPara->pShadingComp->SDBLK_RATIO01;
    rLscCfg.ratio.bits.RATIO10              = pShadingPara->pShadingComp->SDBLK_RATIO10;
    rLscCfg.ratio.bits.RATIO11              = pShadingPara->pShadingComp->SDBLK_RATIO11;

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    pLscMgr->setConfig((LscMgr2::ELscScenario_T)eSensorMode, rLscCfg);

    // log nvram data
    MY_LOG("[%s -] SET_SHADING_PARA SensorMode(%d): ctl1(0x%08x) ctl2(0x%08x) lbolck(0x%08x) rto(0x%08x)", __FUNCTION__, eSensorMode,
        rLscCfg.ctl2.val, rLscCfg.ctl3.val, rLscCfg.lblock.val, rLscCfg.ratio.val);

    return CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA )
{
    MUINT8 *pCompMode=reinterpret_cast<MUINT8*>(puParaIn);
    MUINT32 u4Mode;
    typedef ACDK_CCT_SHADING_COMP_STRUCT o_type;
    if (! puParaIn || sizeof(o_type) != u4ParaOutLen  || ! puParaOut)
        return CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);
    ESensorMode_T eSensorMode = (ESensorMode_T)(*((UINT8*)puParaIn));

    MY_LOG("[%s +] GET_SHADING_PARA SensorMode(%d)", __FUNCTION__, eSensorMode);

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    const ISP_NVRAM_LSC_T* prLscCfg = pLscMgr->getConfig((LscMgr2::ELscScenario_T)eSensorMode);

    if (prLscCfg == NULL)
    {
        MY_ERR("GET_SHADING_PARA fail to get cfg");
        return CCTIF_BAD_PARAM;
    }

    pShadingPara->pShadingComp->SHADING_EN          = pLscMgr->getOnOff();
    pShadingPara->pShadingComp->SHADINGBLK_XNUM     = prLscCfg->ctl2.bits.SDBLK_XNUM+1;
    pShadingPara->pShadingComp->SHADINGBLK_YNUM     = prLscCfg->ctl3.bits.SDBLK_YNUM+1;
    pShadingPara->pShadingComp->SHADINGBLK_WIDTH    = prLscCfg->ctl2.bits.SDBLK_WIDTH;
    pShadingPara->pShadingComp->SHADINGBLK_HEIGHT   = prLscCfg->ctl3.bits.SDBLK_HEIGHT;
    pShadingPara->pShadingComp->SHADING_RADDR       = 0;
    pShadingPara->pShadingComp->SD_LWIDTH           = prLscCfg->lblock.bits.SDBLK_lWIDTH;
    pShadingPara->pShadingComp->SD_LHEIGHT          = prLscCfg->lblock.bits.SDBLK_lHEIGHT;
    pShadingPara->pShadingComp->SDBLK_RATIO00       = prLscCfg->ratio.bits.RATIO00;
    pShadingPara->pShadingComp->SDBLK_RATIO01       = prLscCfg->ratio.bits.RATIO01;
    pShadingPara->pShadingComp->SDBLK_RATIO10       = prLscCfg->ratio.bits.RATIO10;
    pShadingPara->pShadingComp->SDBLK_RATIO11       = prLscCfg->ratio.bits.RATIO11;

    // log nvram data
    MY_LOG("[%s -] GET_SHADING_PARA SensorMode(%d): ctl1(0x%08x) ctl2(0x%08x) lbolck(0x%08x) rto(0x%08x)", __FUNCTION__, eSensorMode,
        prLscCfg->ctl2.val, prLscCfg->ctl3.val, prLscCfg->lblock.val, prLscCfg->ratio.val);


    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX )
{
    if ( ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 u4CCT = *reinterpret_cast<MUINT8*>(puParaIn);

    MY_LOG("[%s] SET_SHADING_INDEX(%d)", __FUNCTION__, u4CCT);

    IspTuningMgr::getInstance().enableDynamicShading(m_eSensorEnum, MFALSE);
    IspTuningMgr::getInstance().setIndex_Shading(m_eSensorEnum, u4CCT);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX )
{

    if  ( ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MUINT8 *pShadingIndex = reinterpret_cast<MUINT8*>(puParaOut);

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    *pShadingIndex = pLscMgr->getCTIdx();

    MY_LOG("[%s] GET_SHADING_INDEX(%d)", __FUNCTION__, *pShadingIndex);

    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF )
{
    typedef ACDK_CCT_TABLE_SET_STRUCT i_type;
    if  ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingtabledata  = reinterpret_cast<i_type*> (puParaIn);

    LscMgr2::ELscScenario_T eLscScn = (LscMgr2::ELscScenario_T)pShadingtabledata->Mode;
    MUINT8* pSrc = (MUINT8*)(pShadingtabledata->pBuffer);
    MUINT32 u4CtIdx = pShadingtabledata->ColorTemp;
    MUINT32 u4Size = pShadingtabledata->Length;

    MY_LOG("[%s +] SET_SHADING_TABLE: SensorMode(%d),CT(%d),Src(%p),Size(%d)", __FUNCTION__,
        eLscScn, u4CtIdx, pSrc, u4Size);

    if (pSrc == NULL)
    {
        MY_ERR("SET_SHADING_TABLE: NULL pSrc");
        return CCTIF_BAD_PARAM;
    }

    if (u4CtIdx >= 4)
    {
        MY_ERR("SET_SHADING_TABLE: Wrong CtIdx(%d)", u4CtIdx);
        return CCTIF_BAD_PARAM;
    }

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    MUINT8* pDst = ((MUINT8*)(pLscMgr->getLut(eLscScn))+u4CtIdx*pLscMgr->getPerLutSize(eLscScn));
    ::memcpy(pDst, pSrc, u4Size*sizeof(MUINT32));
    if (!pLscMgr->syncLut(eLscScn))
    {
        MY_ERR("Fail to sync LUT to DRAM");
        return CCTIF_UNKNOWN_ERROR;
    }

    MY_LOG("[%s -] SET_SHADING_TABLE", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

#include <nvram_drv_mgr.h>
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF )
{
    typedef ACDK_CCT_TABLE_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingtabledata  = reinterpret_cast<o_type*> (puParaOut);

    LscMgr2::ELscScenario_T eLscScn = (LscMgr2::ELscScenario_T)pShadingtabledata->Mode;
    MUINT8* pDst = (MUINT8*)(pShadingtabledata->pBuffer);
    MUINT32 u4CtIdx = pShadingtabledata->ColorTemp;
    MUINT32 u4Size = pShadingtabledata->Length;

    MY_LOG("[%s +] GET_SHADING_TABLE: SensorMode(%d),CT(%d),Src(%p),Size(%d)", __FUNCTION__,
        eLscScn, u4CtIdx, pDst, u4Size);

    if (pDst == NULL)
    {
        MY_ERR("GET_SHADING_TABLE: NULL pDst");
        return CCTIF_BAD_PARAM;
    }

    if (u4CtIdx >= 4)
    {
        MY_ERR("GET_SHADING_TABLE: Wrong CtIdx(%d)", u4CtIdx);
        return CCTIF_BAD_PARAM;
    }

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    MUINT8* pSrc = ((MUINT8*)(pLscMgr->getLut(eLscScn))+u4CtIdx*pLscMgr->getPerLutSize(eLscScn));
    ::memcpy(pDst, pSrc, u4Size*sizeof(MUINT32));

    MY_LOG("[%s -] GET_SHADING_TABLE", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE )
{
    ACDK_CCT_FUNCTION_ENABLE_STRUCT*const cctEnable = (reinterpret_cast<ACDK_CCT_FUNCTION_ENABLE_STRUCT*>(puParaIn));

    MY_LOG("[%s] + OnOff(%d)", __FUNCTION__, cctEnable->Enable);

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    pLscMgr->setTsfForceAwb(cctEnable->Enable ? MTRUE : MFALSE);

    MY_LOG("[%s] -", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TSF_ONOFF )
{
    UINT32 u4OnOff = *(reinterpret_cast<UINT32*>(puParaIn));

    MY_LOG("[%s] + SET_SHADING_TSF_ONOFF(%d)", __FUNCTION__, u4OnOff);

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    pLscMgr->setTsfOnOff(u4OnOff);
    NSIspTuning::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);

    MY_LOG("[%s] - SET_SHADING_TSF_ONOFF", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TSF_ONOFF )
{
    UINT32 u4OnOff;

    MY_LOG("[%s] + GET_SHADING_TSF_ONOFF", __FUNCTION__);

    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);
    u4OnOff = pLscMgr->getTsfOnOff();
    *(reinterpret_cast<UINT32*>(puParaOut)) = u4OnOff;

    MY_LOG("[%s] - GET_SHADING_TSF_ONOFF(%d)", __FUNCTION__, u4OnOff);

    return CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA )
{
#if 1
    typedef ACDK_CCT_NVRAM_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pCAMERA_NVRAM_DATA  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA]"
        "Mode is %d"
        , pCAMERA_NVRAM_DATA->Mode
        );


    MUINT8* p_cct_output_address = (MUINT8 *)(pCAMERA_NVRAM_DATA->pBuffer);

    switch (pCAMERA_NVRAM_DATA->Mode)
    {
        case CAMERA_NVRAM_DEFECT_STRUCT:
            break;
        case CAMERA_NVRAM_SHADING_STRUCT:
        {
            ::memcpy(p_cct_output_address, &m_rBuf_SD.Shading, sizeof(ISP_SHADING_STRUCT));
            *pu4RealParaOutLen = sizeof(ISP_SHADING_STRUCT);
            MY_LOG("NVRAM Data :%d\n", m_rBuf_SD.Shading.PrvTable[0][0]);
        }
            break;
        case CAMERA_NVRAM_3A_STRUCT:
        {
//            MINT32 err = CCTIF_NO_ERROR;
//            err = m_rBufIf_3A.refresh(m_eSensorEnum, m_u4SensorID);
//            if  ( CCTIF_NO_ERROR != err )
//            {
//                MY_ERR("[CAMERA_NVRAM_3A_STRUCT] m_rBufIf_3A.refresh() fail (0x%x)\n", err);
//                return  err;
//            }
            ::memcpy(p_cct_output_address, &m_rBuf_3A, sizeof(NVRAM_CAMERA_3A_STRUCT));
            *pu4RealParaOutLen = sizeof(NVRAM_CAMERA_3A_STRUCT);
            MY_LOG("m_eSensorEnum(sensor type) = %d\n",  (MUINT32)m_eSensorEnum);
            //MY_LOG("m_u4SensorID = %d\n",  (MUINT32)m_u4SensorID);
            MY_LOG("*pu4RealParaOutLen = %d\n",  *pu4RealParaOutLen);
            //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4R = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4R);
            //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4G = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4G);
            //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4B = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4B);
            //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4R = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4R);
            //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4G = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4G);
            //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4B = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4B);

            MY_LOG("cal data: m_rBuf_3A.rAENVRAM.rDevicesInfo.u4MinGain = %d\n",  m_rBuf_3A.rAENVRAM.rDevicesInfo.u4MinGain);
            MY_LOG("cal data: m_rBuf_3A.rAENVRAM.rDevicesInfo.u4MaxGain = %d\n",  m_rBuf_3A.rAENVRAM.rDevicesInfo.u4MaxGain);
            MY_LOG("cal data: m_rBuf_3A.rAENVRAM.rDevicesInfo.u4MiniISOGain = %d\n",  m_rBuf_3A.rAENVRAM.rDevicesInfo.u4MiniISOGain);
            MY_LOG("cal data: m_rBuf_3A.rAENVRAM.rDevicesInfo.u4PreExpUnit = %d\n",  m_rBuf_3A.rAENVRAM.rDevicesInfo.u4PreExpUnit);

        }
            break;
        case CAMERA_NVRAM_ISP_PARAM_STRUCT:
        {
            ::memcpy(p_cct_output_address, &m_rBuf_ISP, sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            *pu4RealParaOutLen = sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT);
            MY_LOG("m_eSensorEnum(sensor type) = %d\n",  (MUINT32)m_eSensorEnum);
            //MY_LOG("m_u4SensorID = %d\n",  (MUINT32)m_u4SensorID);
            MY_LOG("*pu4RealParaOutLen = %d\n",  *pu4RealParaOutLen);
            MY_LOG("ISP reg: m_rBuf_ISP.SensorId = %d\n",  m_rBuf_ISP.SensorId);
            MY_LOG("ISP reg: m_rBuf_ISP.Version = %d\n",  m_rBuf_ISP.Version);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[0].offst0.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[0].offst0.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[1].offst1.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[1].offst1.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[2].gain0.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[2].gain0.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[3].gain1.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[3].gain1.val);

            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[0].con1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[0].con1.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[10].yad1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[10].yad1.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[20].lut1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[20].lut1.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[30].lce1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[30].lce1.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[40].hp1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[40].hp1.val);

            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCR[0].uvlut.val = %d\n",  m_rBuf_ISP.ISPRegs.CCR[0].uvlut.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCR[3].uvlut.val = %d\n",  m_rBuf_ISP.ISPRegs.CCR[3].uvlut.val);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCR[5].uvlut.val = %d\n",  m_rBuf_ISP.ISPRegs.CCR[5].uvlut.val);

            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M11 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv0a.bits.G2G_CNV_00);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M12 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv0a.bits.G2G_CNV_01);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M13 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv0b.bits.G2G_CNV_02);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M21 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv1a.bits.G2G_CNV_10);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M22 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv1a.bits.G2G_CNV_11);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M23 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv1b.bits.G2G_CNV_12);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M31 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv2a.bits.G2G_CNV_20);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M32 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv2a.bits.G2G_CNV_21);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].M33 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv2b.bits.G2G_CNV_22);

            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv0a.rsv_11 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv0a.bits.rsv_11);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv0a.rsv_27 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv0a.bits.rsv_27);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv0b.rsv_11 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv0b.bits.rsv_11);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv1a.rsv_11 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv1a.bits.rsv_11);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv1a.rsv_27 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv1a.bits.rsv_27);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv1b.rsv_11 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv1b.bits.rsv_11);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv2a.rsv_11 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv2a.bits.rsv_11);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv2a.rsv_27 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv2a.bits.rsv_27);
            MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCM[0].conv2b.rsv_11 = %d\n",  m_rBuf_ISP.ISPRegs.CCM[0].conv2b.bits.rsv_11);
        }
            break;
        case CAMERA_NVRAM_LENS_STRUCT:
        {
//            MINT32 err = CCTIF_NO_ERROR;
//            err = m_rBufIf_LN.refresh(m_eSensorEnum, m_u4SensorID);
//            if  ( CCTIF_NO_ERROR != err )
//            {
//                MY_ERR("[CAMERA_NVRAM_LENS_STRUCT] m_rBufIf_LN.refresh() fail (0x%x)\n", err);
//                return  err;
//            }
            ::memcpy(p_cct_output_address, &m_rBuf_LN, sizeof(NVRAM_LENS_PARA_STRUCT));
            *pu4RealParaOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);
            MY_LOG("m_eSensorEnum(sensor type) = %d\n",  (MUINT32)m_eSensorEnum);
            //MY_LOG("m_u4SensorID = %d\n",  (MUINT32)m_u4SensorID);
            MY_LOG("m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4Offset = %d\n",        m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4Offset);
            MY_LOG("m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum = %d\n",        m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum);
            MY_LOG("m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum = %d\n",        m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);

        }
            break;
        default:
            MY_LOG("[Get Camera NVRAM data]"
                "Not support NVRAM structure\n");
            return CCTIF_BAD_PARAM;
            break;
    }


    return  CCTIF_NO_ERROR;
#else
    return  CCTIF_NO_ERROR;
#endif

}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM )
{
#if 0
    MY_LOG("[%s] ", __FUNCTION__);
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM )");


    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const pBufIF = m_pNvramDrv->getBufIF<NVRAM_CAMERA_SHADING_STRUCT>();

    int sensorId;
    err = nvbufUtil_getSensorId(m_eSensorEnum, &sensorId);
    if(err!=0)
        return err;
    err = pBufIF->refresh(m_eSensorEnum, sensorId);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM] m_rBufIf_SD.refresh() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
#else
    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);

    if (pLscMgr->readNvramTbl(MTRUE))
    {
        pLscMgr->loadLutToSysram();
        MY_LOG("[%s] SDTBL_LOAD_FROM_NVRAM OK", __FUNCTION__);
        return CCTIF_NO_ERROR;
    }
    else
    {
        MY_ERR("SDTBL_LOAD_FROM_NVRAM fail");
        return CCTIF_UNKNOWN_ERROR;
    }

#endif
}


IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM )
{
#if 0

    MY_LOG("[%s] ", __FUNCTION__);
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM )");

    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const pBufIF = m_pNvramDrv->getBufIF<NVRAM_CAMERA_SHADING_STRUCT>();

    int sensorId;
    err = nvbufUtil_getSensorId(m_eSensorEnum, &sensorId);
    if(err!=0)
        return err;
    err = pBufIF->flush(m_eSensorEnum, sensorId);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM] m_rBufIf_SD.flush() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
#else
    LscMgr2* pLscMgr = LscMgr2::getInstance((ESensorDev_T)m_eSensorEnum);

    if (pLscMgr->writeNvramTbl())
    {
        MY_LOG("[%s] SDTBL_SAVE_TO_NVRAM OK", __FUNCTION__);
        return CCTIF_NO_ERROR;
    }
    else
    {
        MY_ERR("SDTBL_SAVE_TO_NVRAM fail");
        return CCTIF_UNKNOWN_ERROR;
    }

    return 0;
#endif

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
*/
#if 1//FIXED==1
IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_GET_SMOOTH_SWITCH )
{
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    reinterpret_cast<o_type*>(puParaOut)->Enable = m_rBuf_ISP.bInvokeSmoothCCM;
    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_GET_SMOOTH_SWITCH ) done, %d\n", m_rBuf_ISP.bInvokeSmoothCCM);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT //1=Smooth CCM; 0=Dynamic CCM
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_SET_SMOOTH_SWITCH )
{
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    m_rBuf_ISP.bInvokeSmoothCCM = reinterpret_cast<i_type*>(puParaIn)->Enable ? 1 : 0;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_SET_SMOOTH_SWITCH ) done, %d\n", m_rBuf_ISP.bInvokeSmoothCCM);
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_QUERY_ISP_INDEX_INPUT_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_QUERY_ISP_INDEX_INPUT_STRUCT)
puParaOut
    CUSTOM_NVRAM_REG_INDEX
u4ParaOutLen
    sizeof(CUSTOM_NVRAM_REG_INDEX)
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_QUERY_ISP_INDEX_SET )
{
    typedef ACDK_CCT_QUERY_ISP_INDEX_INPUT_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    typedef CUSTOM_NVRAM_REG_INDEX o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ACDK_CCT_QUERY_ISP_INDEX_INPUT_STRUCT *pIspIndexInput = reinterpret_cast<i_type*>(puParaIn);

    MVOID* pIndex = IspTuningMgr::getInstance().getDefaultISPIndex(m_eSensorEnum, pIspIndexInput->profile, pIspIndexInput->sensorMode, pIspIndexInput->iso_idx);

    CUSTOM_NVRAM_REG_INDEX *pCustNvramRegIdx = reinterpret_cast<o_type*>(puParaOut);

    ::memcpy( pCustNvramRegIdx, pIndex, sizeof(o_type) );

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_QUERY_ISP_INDEX_SET ) done, OBC(%d) BPC(%d) NR1(%d) CFA(%d) GGM(%d) ANR(%d) CCR(%d) EE(%d) NR3D(%d) MFB(%d) LCE(%d)\n",
    pCustNvramRegIdx->OBC, pCustNvramRegIdx->BPC, pCustNvramRegIdx->NR1, pCustNvramRegIdx->CFA, pCustNvramRegIdx->GGM, pCustNvramRegIdx->ANR, pCustNvramRegIdx->CCR,
    pCustNvramRegIdx->EE, pCustNvramRegIdx->NR3D, pCustNvramRegIdx->MFB, pCustNvramRegIdx->LCE);

    return  CCTIF_NO_ERROR;
}

/*
puParaOut
    ISP_NVRAM_DNG_METADATA_T
u4ParaOutLen
    sizeof(ISP_NVRAM_DNG_METADATA_T)
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_D1_OP_ISP_GET_DNG_PARA )
{
    typedef ISP_NVRAM_DNG_METADATA_T o_type;

    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_DNG_METADATA_T *pDNGMetaData = reinterpret_cast<o_type*>(puParaOut);

    memcpy( pDNGMetaData, (void*)&(m_rBuf_ISP.DngMetadata), sizeof(o_type) );

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_D1_SET_DNG_PARA ) done, RefereceIlluminant1(%d) RefereceIlluminant2(%d) NoiseProfile[0].O.a(%.3f) NoiseProfile.[0].O.b(%.3f), NoiseProfile[0].S.a(%.3f) NoiseProfile.[0].S.b(%.3f) NoiseProfile[1].O.a(%.3f) NoiseProfile.[1].O.b(%.3f), NoiseProfile[1].S.a(%.3f) NoiseProfile.[1].S.b(%.3f) NoiseProfile[2].O.a(%.3f) NoiseProfile.[2].O.b(%.3f), NoiseProfile[2].S.a(%.3f) NoiseProfile.[2].S.b(%.3f) NoiseProfile[3].O.a(%.3f) NoiseProfile.[3].O.b(%.3f), NoiseProfile[3].S.a(%.3f) NoiseProfile.[3].S.b(%.3f)\n",
    pDNGMetaData->i4RefereceIlluminant1, pDNGMetaData->i4RefereceIlluminant2,
    pDNGMetaData->rNoiseProfile[0].O.a, pDNGMetaData->rNoiseProfile[0].O.b, pDNGMetaData->rNoiseProfile[0].S.a, pDNGMetaData->rNoiseProfile[0].S.b,
    pDNGMetaData->rNoiseProfile[1].O.a, pDNGMetaData->rNoiseProfile[1].O.b, pDNGMetaData->rNoiseProfile[1].S.a, pDNGMetaData->rNoiseProfile[1].S.b,
    pDNGMetaData->rNoiseProfile[2].O.a, pDNGMetaData->rNoiseProfile[2].O.b, pDNGMetaData->rNoiseProfile[2].S.a, pDNGMetaData->rNoiseProfile[2].S.b,
    pDNGMetaData->rNoiseProfile[3].O.a, pDNGMetaData->rNoiseProfile[3].O.b, pDNGMetaData->rNoiseProfile[3].S.a, pDNGMetaData->rNoiseProfile[3].S.b
    );

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
    ISP_NVRAM_DNG_METADATA_T
u4ParaInLen
    sizeof(ISP_NVRAM_DNG_METADATA_T)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_D1_OP_ISP_SET_DNG_PARA )
{
    typedef ISP_NVRAM_DNG_METADATA_T i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_DNG_METADATA_T *pDNGMetaData = reinterpret_cast<i_type*>(puParaIn);

    memcpy( (void*)&m_rBuf_ISP.DngMetadata, pDNGMetaData, sizeof(i_type) );

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_D1_SET_DNG_PARA ) done, RefereceIlluminant1(%d) RefereceIlluminant2(%d) NoiseProfile[0].O.a(%.3f) NoiseProfile.[0].O.b(%.3f), NoiseProfile[0].S.a(%.3f) NoiseProfile.[0].S.b(%.3f) NoiseProfile[1].O.a(%.3f) NoiseProfile.[1].O.b(%.3f), NoiseProfile[1].S.a(%.3f) NoiseProfile.[1].S.b(%.3f) NoiseProfile[2].O.a(%.3f) NoiseProfile.[2].O.b(%.3f), NoiseProfile[2].S.a(%.3f) NoiseProfile.[2].S.b(%.3f) NoiseProfile[3].O.a(%.3f) NoiseProfile.[3].O.b(%.3f), NoiseProfile[3].S.a(%.3f) NoiseProfile.[3].S.b(%.3f)\n",
    m_rBuf_ISP.DngMetadata.i4RefereceIlluminant1, m_rBuf_ISP.DngMetadata.i4RefereceIlluminant2,
    m_rBuf_ISP.DngMetadata.rNoiseProfile[0].O.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[0].O.b, m_rBuf_ISP.DngMetadata.rNoiseProfile[0].S.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[0].S.b,
    m_rBuf_ISP.DngMetadata.rNoiseProfile[1].O.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[1].O.b, m_rBuf_ISP.DngMetadata.rNoiseProfile[1].S.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[1].S.b,
    m_rBuf_ISP.DngMetadata.rNoiseProfile[2].O.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[2].O.b, m_rBuf_ISP.DngMetadata.rNoiseProfile[2].S.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[2].S.b,
    m_rBuf_ISP.DngMetadata.rNoiseProfile[3].O.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[3].O.b, m_rBuf_ISP.DngMetadata.rNoiseProfile[3].S.a, m_rBuf_ISP.DngMetadata.rNoiseProfile[3].S.b
    );

    return  CCTIF_NO_ERROR;
}
#endif
