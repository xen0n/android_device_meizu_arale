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
#define LOG_TAG "CdpDrv_FrmB"

#include <utils/Errors.h>
//#include <cutils/log.h>
#include "camera_sysram_D2.h"  // For SYSRAM.
//#include <cutils/pmem.h>
//#include <cutils/memutil.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <sys/ioctl.h>

#include "imageio_types.h"    // For type definitions.
#include <mtkcam/drv_common/isp_reg.h>    // For ISP register structures.
//#include <mtkcam/drv/isp_drv.h>    // For ISP_[READ|WRITE]_[BITS|REG] macros.
#include "cdp_drv_imp_FrmB.h"

#include <cutils/properties.h>  // For property_get().

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{CdpDrv} "
//#undef   DBG_LOG_LEVEL                      // Decide Debug Log level for current file. Can only choose from 2~8.
//#define  DBG_LOG_LEVEL      2               // 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(cdp_drv);
//EXTERN_DBG_LOG_VARIABLE(cdp_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (cdp_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define VRZ_EXIST   0
#define RSP_EXIST   0
#define VRZO_EXIST  0
#define DISPO_HAS_ROTATE    0

class CdpDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    CdpDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
        if  (0==mIdx)
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};

#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 0   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog(G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr(G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable(EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr(G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 1   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   CdpDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   CdpDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static NSIspDrv_FrmB::IspDrv* m_pIspDrv;

static MUINT32 CdpDrvRzUint[CDP_DRV_ALGO_AMOUNT] =
{
    32768,
    1048576,
    1048576
};

static MUINT32 CdpDrvRzTable[CDP_DRV_RZ_TABLE_AMOUNT] =
{
    32768,
    36408,
    40960,
    46811,
    54613,
    65536
};

/**************************************************************************
*       P R I V A T E    F U N C T I O N    D E C L A R A T I O N
**************************************************************************/


/**************************************************************************
*
**************************************************************************/
CdpDrvImp::CdpDrvImp()
{
    MUINT32 i, j;
    GLOBAL_PROFILING_LOG_START(Event_CdpDrv);    // Profiling Start.

    LOG_INF("");

    mInitCount  = 0;
    mSysramUsageCount = 0;
    mpIspReg        = NULL;
    m_pIspDrv    = NULL;
    m_pPhyIspReg    = NULL;
    mFdSysram       = -1;

    SysramAlloc.Alignment = 0;
    SysramAlloc.Size = 0;
    SysramAlloc.User = SYSRAM_USER_VIDO;
    SysramAlloc.Addr = 0;
    SysramAlloc.TimeoutMS = 100;

    for (i = 0; i < CDP_DRV_ROTDMA_AMOUNT; i++)
    {
        for (j = 0; j < CDP_DRV_LC_AMOUNT; j++)
        {
            mRotationBuf[i][j].Fd = -1;
        }
    }
}


/**************************************************************************
*
**************************************************************************/
CdpDrvImp::~CdpDrvImp()
{
    LOG_INF("");
    GLOBAL_PROFILING_LOG_END();     // Profiling End.
}


/**************************************************************************
*
**************************************************************************/
CdpDrv *CdpDrv::CreateInstance(MBOOL fgIsGdmaMode)
{
    DBG_LOG_CONFIG(imageio, cdp_drv);

    LOG_INF("fgIsGdmaMode: %d.", fgIsGdmaMode);

    return CdpDrvImp::GetInstance(false);
}


/**************************************************************************
*
**************************************************************************/
CdpDrv *CdpDrvImp::GetInstance(MBOOL fgIsGdmaMode)
{
    static CdpDrvImp Singleton;

    //LOG_INF("");
    LOG_INF("fgIsGdmaMode: %d.", fgIsGdmaMode);

    Singleton.m_fgIsGdmaMode = fgIsGdmaMode;

    return &Singleton;
}


/**************************************************************************
*
**************************************************************************/
MVOID CdpDrvImp::DestroyInstance()
{
    LOG_INF("+");
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::Init()
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_CdpDrv_Init);

    Mutex::Autolock lock(mLock);    // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

    LOG_INF("+,mInitCount(%d),mSysramUsageCount(%d),m_fgIsGdmaMode(%d)", mInitCount, mSysramUsageCount, (int)m_fgIsGdmaMode);

    // Increase CDR Drv count.
    android_atomic_inc(&mInitCount);
    LOCAL_PROFILING_LOG_PRINT("atomic_inc");

lbEXIT:

    LOG_INF("-,Result(%d),mInitCount(%d),mFdSysram(%d),mSysramUsageCount(%d)", Result, mInitCount, mFdSysram, mSysramUsageCount);

    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::Uninit()
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_CdpDrv_Uninit);

    Mutex::Autolock lock(mLock);

    LOG_INF("+,mInitCount(%d),mSysramUsageCount(%d)", mInitCount, mSysramUsageCount);

    android_atomic_dec(&mInitCount);
    LOCAL_PROFILING_LOG_PRINT("atomic_dec");
    if(mInitCount <0)
    {
        // No more users
        LOG_ERR("NO more users");
    }

EXIT:

    LOG_INF("-.Result(%d),mInitCount(%d),mFdSysram(%d),mSysramUsageCount(%d)", Result, mInitCount, mFdSysram, mSysramUsageCount);
    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;
}

/**************************************************************************
* @brief Set ISP Reg pointer for CDP Drv to use
* @param [IN]     pIspReg         Pointer of ISP Reg.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::SetIspReg(isp_reg_t *pIspReg)
{
    LOG_DBG("+,pIspReg(0x%08x)", pIspReg);

    if (mInitCount <= 0)
    {
        LOG_ERR("No CDP Drv. Please init one first!");
        return MFALSE;
    }

    if (pIspReg == NULL)
    {
        LOG_ERR("pIspReg is NULL.");
        return MFALSE;
    }

    mpIspReg = pIspReg;

    return MTRUE;
}

/**************************************************************************
* @brief Set ISP Reg pointer for CDP Drv to use.
* @param [IN]     pIspReg         Pointer of ISP Reg.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::SetIspDrv(IspDrv *pPhyIspDrv)
{
    LOG_DBG("+,pPhyIspDrv(0x%08x)", pPhyIspDrv);

    if (mInitCount <= 0)
    {
        LOG_ERR("No CDP Drv. Please init one first!");
        return MFALSE;
    }

    if (pPhyIspDrv == NULL)
    {
        LOG_ERR("pPhyIspDrv is NULL.");
        return MFALSE;
    }

    m_pIspDrv = pPhyIspDrv;

    return MTRUE;
}

/**************************************************************************
* @brief Check if CDR Drv is ready.
* 1. Check CDP Drv instance count. If count <= 0, it means no CDP Drv
* 2. Check if mpIspReg is ready
*
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::CheckReady()
{
    // 1. Check CDP Drv instance count. If count <= 0, it means no CDP Drv.
    if (mInitCount <= 0)
    {
        LOG_ERR("No more CDP Drv user. Please init one first!");
        return MFALSE;
    }

    // 2. Check if mpIspReg is ready.
    if (mpIspReg == NULL)
    {
        LOG_ERR("mpIspReg is NULL.");
        return MFALSE;
    }

    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CalAlgoAndCStep(
    CDP_DRV_MODE_ENUM eFrameOrTpipeOrVrmrg,
    CDP_DRV_RZ_ENUM eRzName,
    MUINT32 SizeIn_H,
    MUINT32 SizeIn_V,
    MUINT32 u4CroppedSize_H,
    MUINT32 u4CroppedSize_V,
    MUINT32 SizeOut_H,
    MUINT32 SizeOut_V,
    CDP_DRV_ALGO_ENUM *pAlgo_H,
    CDP_DRV_ALGO_ENUM *pAlgo_V,
    MUINT32 *pTable_H,
    MUINT32 *pTable_V,
    MUINT32 *pCoeffStep_H,
    MUINT32 *pCoeffStep_V)
{
    MUINT32 Mm1_H, Mm1_V, Nm1_H, Nm1_V;
    MUINT32 u4SupportingWidth_4tap  = 0;
    MUINT32 u4SupportingWidth_4Ntap = 0;
    MUINT32 u4SupportingWidth_Ntap  = 0;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 u4SizeToBeChecked_H = 0;
    MUINT32 u4SizeToBeChecked_V = 0;

    LOG_INF("+,Mode(%d),eRzName(%d),In/Crop/Out:(%d,%d)/(%d,%d)/(%d,%d).",eFrameOrTpipeOrVrmrg, eRzName, SizeIn_H, SizeIn_V, u4CroppedSize_H, u4CroppedSize_V, SizeOut_H, SizeOut_V);

    // Check if CDP Drv is ready.
    if (!CheckReady())
    {
        Result = MFALSE;
        goto lbExit;
    }

    // Calculate Mm1 = u4CroppedSize - 1. //todo: ignore floating crop?
    Mm1_H = u4CroppedSize_H - 1;
    Mm1_V = u4CroppedSize_V - 1;
    // Calculate Nm1 = SizeOut - 1.
    Nm1_H = SizeOut_H - 1;
    Nm1_V = SizeOut_V - 1;

    // Decide Supporting Width.
    switch (eFrameOrTpipeOrVrmrg)
    {
        case CDP_DRV_MODE_FRAME:
        case CDP_DRV_MODE_TPIPE:
        {
            switch (eRzName)
            {
                case CDP_DRV_RZ_CDRZ:
                    u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_4_TAP;
                    u4SupportingWidth_4Ntap = CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_4N_TAP;
                    u4SupportingWidth_Ntap  = CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_N_TAP;
                    break;
                default:
                    LOG_ERR("Not support eRzName. eRzName: %d.", eRzName);
                    Result = MFALSE;
                    goto lbExit;
            }
        }
        break;
        default:
            LOG_ERR("Not support eFrameOrTpipeOrVrmrg. eFrameOrTileOrVrmrg: %d.", eFrameOrTpipeOrVrmrg);
            Result = MFALSE;
            goto lbExit;
    }

    // Calculate horizontal part.
    // Pick the smaller one as u4WidthToBeChecked.
    if (u4CroppedSize_H <= SizeOut_H)
    {
        u4SizeToBeChecked_H = u4CroppedSize_H;
    }
    else
    {
        u4SizeToBeChecked_H = SizeOut_H;
    }

    //====== Calculate Algo/CoeffStep/Table ======

    // 4-tap
    if( (u4CroppedSize_H  * CDP_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_H &&    // 32x >= (Ratio = SizeOut/u4CroppedSize)
         (SizeOut_H * CDP_DRV_RZ_4_TAP_RATIO_MIN) >  u4CroppedSize_H)     // (Ratio = u4CroppedSize/SizeIn) > 1/2
    {
        // Check supporting width.
        if( (eFrameOrTpipeOrVrmrg != CDP_DRV_MODE_TPIPE) && (u4SizeToBeChecked_H > u4SupportingWidth_4tap) )
        {
            LOG_ERR("Exceed supporting width. Mode: %d. eRzName: %d. u4CroppedSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4tap);
            Result = MFALSE;
            goto lbExit;
        }
        //LOG_INF("Using 4_TAP: Mode: %d. eRzName: %d. u4CroppedSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4tap);

        //     Decide Algorithm.
        *pAlgo_H = CDP_DRV_ALGO_4_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Mm1_H * CdpDrvRzUint[*pAlgo_H] + (Nm1_H >> 1)) / Nm1_H);
        //     Find Table.
        for ((*pTable_H) = 0; (*pTable_H) < CDP_DRV_RZ_TABLE_AMOUNT; (*pTable_H)++)
        {
            if ((*pCoeffStep_H) <= CdpDrvRzTable[*pTable_H])
            {
                break;
            }
        }
        // When Table exceed CDP_DRV_RZ_TABLE_AMOUNT, use last table.
        if ((*pTable_H) >= CDP_DRV_RZ_TABLE_AMOUNT)
        {
            (*pTable_H) = CDP_DRV_RZ_TABLE_AMOUNT - 1;
        }
    }
    else if     // 4N-tap
    (   (SizeOut_H * CDP_DRV_RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_H &&    // (Ratio = SizeOut/u4CroppedSize) <= 1/2.    //Vent@20120627: Joseph Lai suggests that when ratio is 1/2, accumulation should be used (i.e. 4n-tap).
        (SizeOut_H * CDP_DRV_RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_H       // (Ratio = SizeOut/u4CroppedSize) >= 1/64.
    )
    {
        // Check supporting width.
        if ( (eFrameOrTpipeOrVrmrg != CDP_DRV_MODE_TPIPE) && (u4SizeToBeChecked_H > u4SupportingWidth_4Ntap) )   // Exceed supporting width, switch to N-tap.
        {
            LOG_DBG("Switch to N-tap because exceed support width. Mode: %d. eRzName: %d. u4CropSize_H: %d. SizeOut_H: %d. 4N-SupportWidth: %d. N-SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4Ntap, u4SupportingWidth_Ntap);
            //     Decide Algorithm.
            *pAlgo_H = CDP_DRV_ALGO_N_TAP;
            //     Calculate CoefStep.
            *pCoeffStep_H = (MUINT32)((Nm1_H * CdpDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
            //     Find Table.
            (*pTable_H) = CDP_DRV_RZ_N_TAP_TABLE;
        }
        else    // Supporting width check passed.
        {
            //LOG_INF("Using 4N-tap. Mode: %d. eRzName: %d. u4CropSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4Ntap);
            //     Decide Algorithm.
            *pAlgo_H = CDP_DRV_ALGO_4N_TAP;
            //     Calculate CoefStep.
            *pCoeffStep_H = (MUINT32)((Nm1_H * CdpDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
            //     Find Table.
            (*pTable_H) = CDP_DRV_RZ_4N_TAP_TABLE;
        }

    }
    else    // Ratio out of range.
    {
        LOG_ERR("Not support ratio. u4CroppedSize_H: %d, SizeOut_H: %d.", u4CroppedSize_H, SizeOut_H);
        Result = MFALSE;
        goto lbExit;
    }


    // Calculate vertical part.
    // Calculate Algo/CoeffStep/Table

    // 4-tap
    if( (u4CroppedSize_V  * CDP_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_V &&        // 32x >= (Ratio = SizeOut/u4CroppedSize).
         (SizeOut_V * CDP_DRV_RZ_4_TAP_RATIO_MIN) >=  u4CroppedSize_V)            // (Ratio = u4CroppedSize/SizeIn) > 1/2.
    {
        //     Decide Algorithm.
        *pAlgo_V = CDP_DRV_ALGO_4_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Mm1_V * CdpDrvRzUint[*pAlgo_V] + (Nm1_V >> 1)) / Nm1_V);
        //     Find Table.
        for ((*pTable_V) = 0; (*pTable_V) < CDP_DRV_RZ_TABLE_AMOUNT; (*pTable_V)++)
        {
            if ((*pCoeffStep_V) <= CdpDrvRzTable[*pTable_V])
            {
                break;
            }
        }

        // When Table exceed CDP_DRV_RZ_TABLE_AMOUNT, use last table.
        if ((*pTable_V) >= CDP_DRV_RZ_TABLE_AMOUNT)
        {
            (*pTable_V) = CDP_DRV_RZ_TABLE_AMOUNT - 1;
        }
    }
    else    // Ratio out of range.
    {
        LOG_ERR("Not support ratio. u4CroppedSize_V(%d),SizeOut_V(%d)", u4CroppedSize_V, SizeOut_V);
        Result = MFALSE;
        goto lbExit;
    }

lbExit:

    LOG_INF("- X. Result(%d),Algo(%d,%d),Table(%d,%d),CoeffStep(%d,%d)", Result, *pAlgo_H, *pAlgo_V, *pTable_H, *pTable_V, *pCoeffStep_H, *pCoeffStep_V);

    return Result;

}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CalOffset(
    CDP_DRV_ALGO_ENUM Algo,
    MBOOL   IsWidth,
    MUINT32 CoeffStep,
    MFLOAT  Offset,
    MUINT32 *pLumaInt,
    MUINT32 *pLumaSub,
    MUINT32 *pChromaInt,
    MUINT32 *pChromaSub)
{
    MUINT32 OffsetInt, OffsetSub;

    LOG_INF("+,Algo(%d),IsWidth(%d),CoeffStep(%d),Offset(%f)", Algo, IsWidth, CoeffStep, Offset);

    // Check if CDP Drv is ready.
    if (!CheckReady())
    {
        return MFALSE;
    }

    OffsetInt = floor(Offset);
    OffsetSub = CdpDrvRzUint[Algo] * (Offset - floor(Offset));

    // Calculate pChromaInt/pChromaSub according current algorithm.
    if (Algo == CDP_DRV_ALGO_4_TAP)
    {
        *pLumaInt = OffsetInt;
        *pLumaSub = OffsetSub;

        //
        if (IsWidth)
        {
            *pChromaInt = floor(Offset / (2.0)); // Because format is YUV422, so the width of chroma is half of Y.
            *pChromaSub = CdpDrvRzUint[Algo] * 2 * (Offset / (2.0) - floor(Offset / (2.0)));
        }
        else
        {
            *pChromaInt = (*pLumaInt);
            *pChromaSub = (*pLumaSub);
        }
    }
    else
    {
        *pLumaInt = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / CdpDrvRzUint[Algo]) / CdpDrvRzUint[Algo];
        *pLumaSub = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / CdpDrvRzUint[Algo]) % CdpDrvRzUint[Algo];
        *pChromaInt = (*pLumaInt);
        *pChromaSub = (*pLumaSub);
    }

    LOG_INF("-, LumaInt/Sub(%d, %d), ChromaInt/Sub(%d, %d).", *pLumaInt, *pLumaSub, *pChromaInt, *pChromaSub);
    return MTRUE;
}

/**************************************************************************
* @brief Allocate memory for rotation.
* 1. Check CDP Drv instance count. If count <= 0, it means no CDP Drv
* 2. Check if mpIspReg is ready.
*
* @param [IN]     Format          Image format of the rotate image.
* @param [IN]     Size            Image size of the rotate image.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::AllocateRotationBuf()
{
    MBOOL Result = MTRUE;


    LOG_INF("+,mSysramUsageCount(%d)", mSysramUsageCount);

#if CDP_DRV_BUF_SYSRAM
//    Mutex::Autolock lock(mLock);      // Already locked in Init().   // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

//    SYSRAM_ALLOC_STRUCT SysramAlloc;

    //TODO
    if (mFdSysram < 0)
    {
        LOG_ERR("No SYSRAM kernel drv.");
        Result = MFALSE;
        goto lbEXIT;
    }

    // Start to allocate SYSRAM.
    if (mSysramUsageCount == 0)    // If first user who allocated SYSRAM buffer.
    {
        SysramAlloc.Alignment = 0;
        //SysramAlloc.Size = LumaBufSize + ChromaBufSize;
        SysramAlloc.Size = CDP_DRV_BUF_SYSRAM_SIZE;    //Vent@20121016: Always allocate max CDP SYSRAM usage, because we only allocate once, but there maybe many thread who uses CDP has diff SYSRAM requirement at the same time.
        SysramAlloc.Addr = 0;
        SysramAlloc.TimeoutMS = 100;

        if (ioctl(mFdSysram, SYSRAM_ALLOC, &SysramAlloc) < 0)   // Allocate fail.
        {
            LOG_ERR("SYSRAM_ALLOC error.");
            Result = MFALSE;
            goto lbEXIT;
        }
        else    // Allocate success.
        {
            LOG_INF("Allocated success. SYSRAM base addr: 0x%08X.", SysramAlloc.Addr);
        }

    }   // End of if (mSysramUsageCount == 0).

    // Allocate/recalculate LumaBufAddr/ChromaBufAddr done. Increase SYSRAM usage count.
    android_atomic_inc(&mSysramUsageCount);

#elif CDP_DRV_BUF_PMEM  // Not used.

    if (
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd   >= 0 ||
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd >= 0
    )
    {
        LOG_DBG("Free memory, RotDma %d, Format %d, Size %d.", RotDma, Format, Size);
        FreeRotationBuf(RotDma);
    }

    //
    switch (Format)
    {
        case CDP_DRV_FORMAT_YUV422:
        case CDP_DRV_FORMAT_YUV420:
        case CDP_DRV_FORMAT_RGB888:
        case CDP_DRV_FORMAT_XRGB8888:
        {
            //Luma
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = Size;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd);
            //Chroma
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size =  mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd);
            break;
        }

        case CDP_DRV_FORMAT_Y:
        {
            //Luma
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = Size;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd);
            //Chroma
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size =  0;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = 0;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr = 0;
            break;
        }

        case CDP_DRV_FORMAT_RGB565:
        {
            //Luma
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = 0;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = 0;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr = 0;
            //Chroma
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size =  Size;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd);
            break;
        }

        default:
        {
            LOG_ERR("Unknown Format: %d.", Format);
            return MFALSE;
        }
    }

#endif  // Use diff memory type.

lbEXIT:
    LOG_INF("- ,ret(%d),mSysramUsageCount(%d)", Result, mSysramUsageCount);

    return Result;

}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::FreeRotationBuf()
{
    MBOOL Result = MTRUE;

    LOG_INF("+,mSysramUsageCount(%d)", mSysramUsageCount);

#if CDP_DRV_BUF_SYSRAM

//    Mutex::Autolock lock(mLock);  // Already locked in Uninit().

    SYSRAM_USER_ENUM SysramUser = SYSRAM_USER_VIDO;

    // Start to free SYSRAM.
    if (mSysramUsageCount > 1)  // More than one user.
    {
        android_atomic_dec(&mSysramUsageCount);
    }
    else if (mSysramUsageCount == 1)   // Last user, must do some un-init procedure.
    {
        android_atomic_dec(&mSysramUsageCount);

        // Free SYSRAM buffer.
        if (mFdSysram >= 0)
        {
            if (ioctl(mFdSysram, SYSRAM_FREE, &SysramUser) < 0)
            {
                LOG_ERR("SYSRAM_FREE error.");
                Result = MFALSE;
            }
        }

        // Reset mRotationBuf[i][j].
        for (MUINT32 i = 0; i < CDP_DRV_ROTDMA_AMOUNT; i++)
        {
            for (MUINT32 j = 0; j < CDP_DRV_LC_AMOUNT; j++)
            {
                // mRotationBuf[i][j].Fd = -1;  // Reset Fd to -1 can only be done after close(mFdSysram).
                mRotationBuf[i][j].Size    = 0;
                mRotationBuf[i][j].VirAddr = 0;
                mRotationBuf[i][j].PhyAddr = 0;
            }
        }

    }
    else // mSysramUsageCount <= 0. No SYSRAM user.
    {
        // do nothing.
        LOG_INF("No SYSRAM to free.");
    }

#elif CDP_DRV_BUF_PMEM

    for (i = 0; i < CDP_DRV_LC_AMOUNT; i++)
    {
        if (mRotationBuf[RotDma][i].Fd >= 0)
        {
            LOG_INF("LC(%d),Size(%d),VirAddr(0x%08X)", i, mRotationBuf[RotDma][i].Size, mRotationBuf[RotDma][i].VirAddr);
            pmem_free((MUINT8*)(mRotationBuf[RotDma][i].VirAddr), mRotationBuf[RotDma][i].Size, mRotationBuf[RotDma][i].Fd);
            mRotationBuf[RotDma][i].Fd = -1;
        }
    }

    //
#endif  // Diff memory type.


lbEXIT:
    LOG_INF("- ,ret(%d),mSysramUsageCount(%d)", Result, mSysramUsageCount);
    return Result;

}

/**************************************************************************
* @brief Change the address of LumaBuf and ChromaBuf in SYSRAM
*
* @param [IN]     RotDma          Use which RotDma
* @param [IN]     Format          Image format of the rotate image
* @param [IN]     Size            Image size of the rotate image
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail
**************************************************************************/
MBOOL CdpDrvImp::RecalculateRotationBufAddr(
    CDP_DRV_ROTDMA_ENUM     RotDma,
    MUINT32                 LumaBufSize,
    MUINT32                 ChromaBufSize
)
{
    MBOOL Result = MTRUE;
    MUINT32 *pLargerBufSize = NULL;
    MUINT32 *pSmallerBufSize = NULL;
    MUINT32 *pLargerBufAddr = NULL;
    MUINT32 *pSmallerBufAddr = NULL;

    LOG_DBG("+,Luma/Chroma BufSize:(%d, %d),mSysramUsageCount(%d)", LumaBufSize, ChromaBufSize, mSysramUsageCount);

    Mutex::Autolock lock(mLock);    // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

    // If exceed total SYSRAM size, exit.
    if ( (LumaBufSize + ChromaBufSize) > CDP_DRV_BUF_SYSRAM_SIZE)
    {
        LOG_ERR("Buffer size (%d) exceeds total SYSRAM size (%d).", (LumaBufSize + ChromaBufSize), CDP_DRV_BUF_SYSRAM_SIZE);
        LOG_ERR("SL_TEST_SMT SKIP  Result = MFALSE");
//SL TEST SMT hang problem        Result = MFALSE;
            goto lbEXIT;
    }

    // Decide mRotationBuf[RotDma][].PhyAddr.
    // Decide which is LargerBuf, which is SmallerBuf.
    if(LumaBufSize >= ChromaBufSize)
    {
        LOG_DBG("LumaBufSize >= ChromaBufSize.");

        pLargerBufSize  = &LumaBufSize;
        pSmallerBufSize = &ChromaBufSize;
        LOG_DBG("*pSmaller/LargerBufSize: %d, %d.", *pSmallerBufSize, *pLargerBufSize);

        pLargerBufAddr  = &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA  ].PhyAddr);
        pSmallerBufAddr = &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr);
        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }
    else    // LumaBufSize < ChromaBufSize
    {
        LOG_DBG("LumaBufSize < ChromaBufSize.");

        pLargerBufSize  = &ChromaBufSize;
        pSmallerBufSize = &LumaBufSize;
        LOG_DBG("*pSmaller/LargerBufSize: %d, %d.", *pSmallerBufSize, *pLargerBufSize);

        pLargerBufAddr  = &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr);
        pSmallerBufAddr = &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA  ].PhyAddr);
        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }

    //     Always put SmallerBuf in Bank0, LargerBuf in Bank1.
    if(*pSmallerBufSize <= CDP_DRV_BUF_SYSRAM_BANK0_SIZE)    // SmallerBuf is smaller than Bank0 (32K), i.e. SmallerBuf can fit in Bank0.
    {
        LOG_DBG("Separate Allocation.");

        *pSmallerBufAddr = SysramAlloc.Addr;   // SmallerBuf at Bank0, SmallerBuf_start align to Bank0_START.

        if (*pLargerBufSize <= CDP_DRV_BUF_SYSRAM_BANK1_SIZE)     // LargerBuf <= Bank1 (48K), i.e. LargerBuf can fit in Bank1.
        {
            *pLargerBufAddr = (SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_START);   // LargerBuf at Bank1, ChromaBuf_start align to Bank1_START.
        }
        else    // LargerBuf can't fit in Bank1.
        {
            *pLargerBufAddr = ((SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END) - *pLargerBufSize) >> 3 << 3;  // VIDO_BUF_BASE_ADDR* uust align to 8-byte.   // LargerBuf at Bank1 (overlap to Bank0), LargerBuf_end align to Bank1_END.

            // Check if overlap SmallerBuf.
            if ((*pSmallerBufAddr + *pSmallerBufSize) > *pLargerBufAddr)
            {
                LOG_ERR("Overlap SmallerBuf after align to 8-byte. S-/L-Addr: (0x%08x, 0x%08x). S-/L-Size: %d, %d.", *pSmallerBufAddr, *pLargerBufAddr, *pSmallerBufSize, *pLargerBufSize);
            }
        }
        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }
    else    // SmallerBuf is larger than Bank0 (32K), i.e. SmallerBuf will overlap to Bank1.
    {
        LOG_DBG("Adjacent Allocation.");

        *pSmallerBufAddr = SysramAlloc.Addr;
        *pLargerBufAddr  = (SysramAlloc.Addr + *pSmallerBufSize + 7 ) >> 3 << 3;    // VIDO_BUF_BASE_ADDR* uust align to 8-byte.

        // Check if exceed SYSRAM size.
        if ((*pLargerBufAddr + *pLargerBufSize) > (SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END))
        {
            LOG_ERR("Exceed SYSRAM size after align to 8-byte. S-/L-Addr: (0x%08x, 0x%08x). S-/L-Size: %d, %d. Limit: 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr, *pSmallerBufSize, *pLargerBufSize, (SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END));
        }

        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }

    // Decide mRotationBuf[RotDma][].Fd/Size/VirAddr
    mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd = mFdSysram;
    mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = LumaBufSize;
    mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = 0;

    mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd = mFdSysram;
    mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size = ChromaBufSize;
    mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = 0;

    LOG_DBG("SysramAlloc::Alignment: %d, Size: %d, User: %d. Addr: 0x%08X. TimeoutMS: %d.", SysramAlloc.Alignment, SysramAlloc.Size, (MUINT32)SysramAlloc.User, SysramAlloc.Addr, SysramAlloc.TimeoutMS);
    LOG_DBG("mRotationBuf[%d][LUMA]::   Fd: %d. Size: %d. VirAddr: 0x%08X. PhyAddr: 0x%08X.",
        RotDma,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr
    );
    LOG_DBG("mRotationBuf[%d][CHROMA]:: Fd: %d. Size: %d. VirAddr: 0x%08X. PhyAddr: 0x%08X.",
        RotDma,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr
    );


lbEXIT:
    LOG_DBG("- ,ret(%d),mSysramUsageCount(%d)", Result, mSysramUsageCount);

    return Result;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::Reset()
{
    LOG_DBG("+");

    if (!CheckReady())
    {
        return MFALSE;
    }

    CDRZ_Enable(MFALSE);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::ResetDefault()
{
    LOG_DBG("+");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    CDRZ_ResetDefault();
    return MTRUE;
}

/**************************************************************************
* @brief Mapping ENUM value used by pipe to ENUM value used by HW register.
*
* @param [IN]     eInFormat       Format ENUM value used by pipe.
* @param [IN]     eInPlane        Plane ENUM value used by pipe.
* @param [OUT]    pu4OutPlane     Plane ENUM value used by HW register.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::RotDmaEnumRemapping(
    CDP_DRV_FORMAT_ENUM eInFormat,
    CDP_DRV_PLANE_ENUM eInPlane,
    MUINT32 *pu4OutPlane)
{
    LOG_DBG("+,eInFormat(%d),eInPlane(%d)", eInFormat, eInPlane);
    MBOOL Result = MTRUE;
    MUINT32 u4OutPlane = 0;

    // Mapping VIDO_FORMAT_1(format)/VIDO_FORMAT_2(block type)/VIDO_FORMAT_3(plane)/VIDO_FORMAT_SEQ(data sequence).
    switch (eInFormat)
    {
        case CDP_DRV_FORMAT_YUV422:
        {
            u4OutPlane = eInPlane;
        }
        break;

        case CDP_DRV_FORMAT_YUV420:
        {
            if (eInPlane == CDP_DRV_PLANE_2)
            {
                u4OutPlane = CDP_DRV_YUV420_PLANE_2;
            }
            else if (eInPlane == CDP_DRV_PLANE_3)
            {
                u4OutPlane = CDP_DRV_YUV420_PLANE_3;
            }
            else    // CDP_DRV_PLANE_1 or other invalid value.
            {
                LOG_ERR("YUV420 format: eInPlane(%d) incorrect, set to default 0 (CDP_DRV_YUV420_PLANE_2).", eInPlane);
                u4OutPlane = CDP_DRV_YUV420_PLANE_2;
            }
        }
        break;

        case CDP_DRV_FORMAT_Y:
        case CDP_DRV_FORMAT_RGB888:
        case CDP_DRV_FORMAT_RGB565:
        case CDP_DRV_FORMAT_XRGB8888:
        {
            u4OutPlane = CDP_DRV_PLANE_1;
        }
        break;

        default:
        {
            LOG_ERR("Unknown eInFormat(%d).", eInFormat);
            Result = MFALSE;
        }
    }

    // Output transformed result.
    *pu4OutPlane = u4OutPlane;
    //
    LOG_DBG("-,Result(%d),u4OutPlane(%d)", *pu4OutPlane);

    return Result;
}

/**************************************************************************
* @brief Mapping ENUM value used by pipe to ENUM value used by HW register.
*
* @param [IN]     eInFormat       Format ENUM value used by pipe.
* @param [IN]     eInPlane        Plane ENUM value used by pipe.
* @param [IN]     eInSequence     Sequence ENUM value used by pipe.
* @param [OUT]    pu4OutPlane     Plane ENUM value used by HW register.
* @param [OUT]    pu4OutSequence  Sequence ENUM value used by HW register.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::InputImgFormatCheck(
    CDP_DRV_FORMAT_ENUM    eInFormat,
    CDP_DRV_PLANE_ENUM     eInPlane,
    CDP_DRV_SEQUENCE_ENUM  eInSequence)
{
    LOG_DBG("+,eInFormat(%d),eInPlane(%d),eInSequence(%d)", eInFormat, eInPlane, eInSequence);
    MBOOL Result = MTRUE;

    // Mapping VIDO_FORMAT_1(format)/VIDO_FORMAT_2(block type)/VIDO_FORMAT_3(plane)/VIDO_FORMAT_SEQ(data sequence).
    switch (eInFormat)
    {
        case CDP_DRV_FORMAT_YUV422:
        {
            if (
                ( eInPlane == CDP_DRV_PLANE_1 || eInPlane == CDP_DRV_PLANE_3 )   &&
                (
                    eInSequence == CDP_DRV_SEQUENCE_YVYU ||
                    eInSequence == CDP_DRV_SEQUENCE_YUYV ||
                    eInSequence == CDP_DRV_SEQUENCE_VYUY ||
                    eInSequence == CDP_DRV_SEQUENCE_UYVY
                )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else if (
                eInPlane == CDP_DRV_PLANE_2   &&
                ( eInSequence == CDP_DRV_SEQUENCE_VUVU || eInSequence == CDP_DRV_SEQUENCE_UVUV )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("YUV422 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_YUV420:
        {
            if (
                ( eInPlane == CDP_DRV_PLANE_2 || eInPlane == CDP_DRV_PLANE_3 )   &&
                ( eInSequence == CDP_DRV_SEQUENCE_VUVU || eInSequence == CDP_DRV_SEQUENCE_UVUV )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("YUV420 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_Y:
        {
            if ( (eInPlane == CDP_DRV_PLANE_1) && (eInSequence == CDP_DRV_SEQUENCE_Y) )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("YOnly format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_RGB888:
        case CDP_DRV_FORMAT_RGB565:
        {
            if (
                (eInPlane == CDP_DRV_PLANE_1) &&
                (eInSequence == CDP_DRV_SEQUENCE_RGB || eInSequence == CDP_DRV_SEQUENCE_BGR)
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("RGB888 and RGB565 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_XRGB8888:
        {
            if (
                (eInPlane == CDP_DRV_PLANE_1) &&
                (
                    eInSequence == CDP_DRV_SEQUENCE_XRGB ||
                    eInSequence == CDP_DRV_SEQUENCE_XBGR ||
                    eInSequence == CDP_DRV_SEQUENCE_RGBX ||
                    eInSequence == CDP_DRV_SEQUENCE_BGRX
                )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("XRGB8888 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        default:
        {
            LOG_ERR("Unknown eInFormat(%d).", eInFormat);
            Result = MFALSE;
        }
    }

    //
    LOG_DBG("-,Result(%d)");

    return Result;

}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DumpReg()
{
    LOG_DBG("+");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    CDRZ_DumpReg();
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    if(!CheckReady())
    {
        LOG_ERR("Please init first!");
        return MFALSE;
    }
    ISP_WRITE_BITS(m_pIspDrv, CAM_CTL_EN2_SET, CDRZ_EN_SET, En, ISP_DRV_USER_ISPF);
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_ResetDefault()
{
    LOG_DBG("+");

    if(!CheckReady())
    {
        return MFALSE;
    }

    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_CONTROL                            , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_INPUT_IMAGE                        , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_OUTPUT_IMAGE                       , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_HORIZONTAL_COEFF_STEP              , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_VERTICAL_COEFF_STEP                , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET     , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET    , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET   , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET       , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET      , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET     , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET    , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_DERING_1                           , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_DERING_2                           , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_DERING_1                           , 0x00000000, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pIspDrv, CAM_CDRZ_DERING_2                           , 0x00000000, ISP_DRV_USER_ISPF);

    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_DumpReg()
{
    LOG_DBG("+");

    if(!CheckReady())
    {
        return MFALSE;
    }

    LOG_DBG("CONTROL                            = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_CONTROL));
    LOG_DBG("INPUT_IMAGE                        = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_INPUT_IMAGE));
    LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_OUTPUT_IMAGE));
    LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_HORIZONTAL_COEFF_STEP));
    LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_VERTICAL_COEFF_STEP));
    LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
    LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
    LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
    LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
    LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET));
    LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
    LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET));
    LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
    LOG_DBG("DERING_1                           = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_DERING_1));
    LOG_DBG("DERING_2                           = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_DERING_2));
    LOG_DBG("DERING_1                           = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_DERING_1));
    LOG_DBG("DERING_2                           = 0x%08X",ISP_READ_REG(m_pIspDrv, CAM_CDRZ_DERING_2));

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);

    if(!CheckReady())
    {
        return MFALSE;
    }


    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_HORIZONTAL_EN, En,ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);

    if(!CheckReady())
    {
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_Vertical_EN, En, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_EnableFirst(MBOOL En)
{
    LOG_DBG("En(%d)",En);

    if(!CheckReady())
    {
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_Vertical_First, En, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetAlgo(CDP_DRV_ALGO_ENUM Algo)
{
    LOG_DBG("Algo(%d)",Algo);

    if(!CheckReady())
    {
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_Horizontal_Algorithm, Algo, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetAlgo(CDP_DRV_ALGO_ENUM Algo)
{
    LOG_DBG("Algo(%d)",Algo);

    if(!CheckReady())
    {
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_Vertical_Algorithm, Algo, ISP_DRV_USER_ISPF);

    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetTruncBit(MUINT32 Bit)
{
    LOG_DBG("Bit(%d)",Bit);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Bit > CDP_DRV_MASK_TRUNC_BIT)
    {
        LOG_ERR("Bit(%d) is out of %d",Bit,CDP_DRV_MASK_TRUNC_BIT);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_Truncation_Bit_H, Bit, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetTruncBit(MUINT32 Bit)
{
    LOG_DBG("Bit(%d)",Bit);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Bit > CDP_DRV_MASK_TRUNC_BIT)
    {
        LOG_ERR("Bit(%d) is out of %d",Bit,CDP_DRV_MASK_TRUNC_BIT);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL,CDRZ_Truncation_Bit_V, Bit,ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_Horizontal_Table_Select, Table, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CONTROL, CDRZ_Vertical_Table_Select, Table, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_W, Size, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_H, Size, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_W, Size, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_H, Size, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_HORIZONTAL_COEFF_STEP, CDRZ_Horizontal_Coeff_Step, CoeffStep, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_VERTICAL_COEFF_STEP, CDRZ_Vertical_Coeff_Step, CoeffStep, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt(%d),LumaSub(%d),ChromaInt(%d),ChromaSub(%d)",LumaInt,LumaSub,ChromaInt,ChromaSub);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }

    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, CDRZ_Luma_Horizontal_Integer_Offset, LumaInt, ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, CDRZ_Luma_Horizontal_Subpixel_Offset, LumaSub, ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET, CDRZ_Chroma_Horizontal_Integer_Offset, ChromaInt, ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET, CDRZ_Chroma_Horizontal_Subpixel_Offset, ChromaSub, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt(%d),LumaSub(%d),ChromaInt(%d),ChromaSub(%d)",LumaInt,LumaSub,ChromaInt,ChromaSub);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }

    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }

    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET, CDRZ_Luma_Vertical_Integer_Offset, LumaInt, ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, CDRZ_Luma_Vertical_Subpixel_Offset, LumaSub, ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET, CDRZ_Chroma_Vertical_Integer_Offset, ChromaInt, ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pIspDrv, CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET, CDRZ_Chroma_Vertical_Subpixel_Offset, ChromaSub, ISP_DRV_USER_ISPF);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_Config(
    CDP_DRV_MODE_ENUM           eFrameOrTileOrVrmrg,
    CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
    CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
    CDP_DRV_IMG_CROP_STRUCT     Crop)
{
    CDP_DRV_ALGO_ENUM Algo_H, Algo_V;
    MUINT32 Table_H, Table_V;
    MUINT32 CoeffStep_H, CoeffStep_V;
    MUINT32 LumaInt;
    MUINT32 LumaSub;
    MUINT32 ChromaInt;
    MUINT32 ChromaSub;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.

    LOG_DBG("In(%d,%d),Crop(%d,%d),Out(%d,%d),CropStart(%f,%f)",SizeIn.Width, SizeIn.Height, Crop.Width.Size, Crop.Height.Size, SizeOut.Width, SizeOut.Height, Crop.Width.Start, Crop.Height.Start);

    if(!CheckReady())
    {
        return MFALSE;
    }

    //Width
    if(Crop.Width.Size == 0)
    {
        Crop.Width.Start = 0;
        Crop.Width.Size = SizeIn.Width;
    }
    else if( Crop.Width.Start < 0 ||
        Crop.Width.Size > SizeIn.Width ||
        (Crop.Width.Start+Crop.Width.Size) > SizeIn.Width)
    {
        LOG_ERR("[CDRZ_Config] Error crop. InWidth(%d),Crop.Width::Start: %f, Size: %d.", SizeIn.Width, Crop.Width.Start, Crop.Width.Size);
        return MFALSE;
    }

    // ====== CDRZ Algo ======

    Result = CalAlgoAndCStep(
                eFrameOrTileOrVrmrg,
                CDP_DRV_RZ_CDRZ,
                SizeIn.Width,
                SizeIn.Height,
                Crop.Width.Size,
                Crop.Height.Size,
                SizeOut.Width,
                SizeOut.Height,
                &Algo_H,
                &Algo_V,
                &Table_H,
                &Table_V,
                &CoeffStep_H,
                &CoeffStep_V);
    if(!Result)
    {
        LOG_ERR("CalAlgoAndCStep fail.");
        return MFALSE;
    }

    //====== Height ======

    if(!CalOffset(Algo_H, MTRUE, CoeffStep_H, Crop.Width.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail.");
        return MFALSE;
    }

    CDRZ_H_EnableScale(MTRUE);
    CDRZ_H_SetAlgo(Algo_H);
    CDRZ_H_SetTruncBit(CDP_DRV_DEFAULT_TRUNC_BIT);
    CDRZ_H_SetTable(Table_H);
    CDRZ_H_SetInputSize(SizeIn.Width);
    CDRZ_H_SetOutputSize(SizeOut.Width);
    CDRZ_H_SetCoeffStep(CoeffStep_H);
    CDRZ_H_SetOffset( LumaInt,LumaSub,ChromaInt,ChromaSub);

    if(Crop.Height.Size == 0)
    {
        Crop.Height.Start = 0;
        Crop.Height.Size = SizeIn.Height;
    }
    else if( Crop.Height.Start < 0 ||
        Crop.Height.Size > SizeIn.Height ||
        (Crop.Height.Start+Crop.Height.Size) > SizeIn.Height)
    {
        LOG_ERR("Error crop. InHeight: %d. Crop.Height::Start: %f, Size: %d.",SizeIn.Height, Crop.Height.Start, Crop.Height.Size);
        return MFALSE;
    }

    //====== Vertical ======

    if(!CalOffset(Algo_V, MFALSE, CoeffStep_V, Crop.Height.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail.");
        return MFALSE;
    }

    CDRZ_V_EnableScale(MTRUE);
    CDRZ_V_EnableFirst(MFALSE);
    CDRZ_V_SetAlgo(Algo_V);
    CDRZ_V_SetTruncBit(CDP_DRV_DEFAULT_TRUNC_BIT);
    CDRZ_V_SetTable(Table_V);
    CDRZ_V_SetInputSize(SizeIn.Height);
    CDRZ_V_SetOutputSize(SizeOut.Height);
    CDRZ_V_SetCoeffStep(CoeffStep_V);
    CDRZ_V_SetOffset( LumaInt,LumaSub,ChromaInt,ChromaSub);

    CDRZ_Enable(MTRUE);

    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_Unconfig()
{
    LOG_DBG("+");

    if(!CheckReady())
    {
        return MFALSE;
    }

    CDRZ_Enable(MFALSE);

    return MTRUE;
}
