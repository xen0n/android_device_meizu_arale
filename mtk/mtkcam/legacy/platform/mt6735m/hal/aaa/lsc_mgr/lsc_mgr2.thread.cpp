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

#define LOG_TAG "lsc_mgr2_thread"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include "lsc_mgr2.h"

#include <cutils/properties.h>
#include <aaa_log.h>

#include <mtkcam/v1/config/PriorityDefs.h>

#include <mtkcam/hwutils/CameraProfile.h>

#include <shading_tuning_custom.h>

#include <mtkcam/algorithm/libtsf/MTKTsf.h>
#include <mtkcam/algorithm/liblsctrans/ShadingTblTransform.h>

#include <awb_param.h>
#include <awb_tuning_custom.h>
#include <isp_tuning_mgr.h>

using namespace NSIspTuning;

namespace NSIspTuningv3 {

#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))

#define TSF_SCN_DFT LscMgr2::LSC_SCENARIO_PRV

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

#define LSC_LOG_BEGIN(fmt, arg...)  MY_LOG("[%s +] " fmt, __FUNCTION__, ##arg)
#define LSC_LOG_END(fmt, arg...)    MY_LOG("[%s -] " fmt, __FUNCTION__, ##arg)
#define LSC_LOG(fmt, arg...)        MY_LOG("[%s] " fmt, __FUNCTION__, ##arg)
#define LSC_ERR(fmt, arg...)        MY_ERR("[%s] " fmt, __FUNCTION__, ##arg)

#define TSF_THREAD_BUILD

#define TSF_LOCK() ::pthread_mutex_lock(&m_Mutex)
#define TSF_UNLOCK() ::pthread_mutex_unlock(&m_Mutex)

MBOOL
LscMgr2::
createThread()
{
#ifdef TSF_BUILD
    LSC_LOG_BEGIN();
    initThreadFunc();
    ::pthread_mutex_init(&m_Mutex, NULL);
#ifdef TSF_THREAD_BUILD
    m_fgThreadLoop = MTRUE;
    ::sem_init(&m_Sema, 0, 0);
    ::pthread_create(&m_Thread, NULL, threadLoop, this);
#endif
    LSC_LOG_END("Create TSF m_Thread(0x%08x)\n", (MUINT32) m_Thread);
#endif
    return MTRUE;
}

MBOOL
LscMgr2::
destroyThread()
{
#ifdef TSF_BUILD
    LSC_LOG_BEGIN();
#ifdef TSF_THREAD_BUILD
    TSF_LOCK();
    m_fgThreadLoop = MFALSE;
    m_eTsfCmd = E_TSF_CMD_IDLE;
    TSF_UNLOCK();
    ::sem_post(&m_Sema);
    ::pthread_join(m_Thread, NULL);
    ::sem_destroy(&m_Sema);
#endif
    ::pthread_mutex_destroy(&m_Mutex);
    uninitThreadFunc();
    LSC_LOG_END();
#endif
    return MTRUE;
}

MVOID
LscMgr2::
changeThreadSetting()
{
#ifdef TSF_BUILD
    // (1) set name
    ::prctl(PR_SET_NAME, "F858THREAD", 0, 0, 0);

    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    // (2) set policy/priority
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_F858_THREAD;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //  get
    ::sched_getparam(0, &sched_p);
#else
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_TSF;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, policy, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, priority);
    //  get
    sched_p.sched_priority = ::getpriority(PRIO_PROCESS, 0);
#endif
    //
    LSC_LOG(
        "sensor(%d), tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%08x, 0x%08x)"
        , m_eSensorDev, ::gettid()
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );

#endif
}

MVOID
LscMgr2::
updateTsf(E_TSF_CMD_T eCmd, const TSF_AWB_INFO* prTsfAwb, const MVOID* prStat)
{
#if defined(TSF_BUILD) && !defined(LSC_DBG)
    //LSC_LOG_BEGIN("sensor(%d)", m_eSensorDev);
    TSF_LOCK();
    if (m_bTSFInit && getTsfOnOff())
    {
        // AWB info
        if (prTsfAwb)
        {
            ::memcpy(&m_rTsfRefInfo.rAwbInfo, prTsfAwb, sizeof(TSF_AWB_INFO));
        }

        // AWB STAT
        if (prStat)
        {
            MINT32 i4NumY = m_rTsfEnvInfo.ImgHeight;
            MINT32 i4NumX = m_rTsfEnvInfo.ImgWidth;
            MINT32 i4SkipAeSize  = ((i4NumX + 3)/4) * 4;
            MINT32 i4LineSize    = i4NumX * 4 + i4SkipAeSize;
            MINT32 i4AwbLineSize = i4NumX * 4;

            const MUINT8* pAWBStat = reinterpret_cast<const MUINT8*>(prStat);
            MUINT8* pDst = m_rTsfRefInfo.prAwbStat;

            for (MINT32 y = 0; y < i4NumY; y++)
            {
                ::memcpy((MUINT8*) pDst+(y*i4AwbLineSize),
                         (MUINT8*) pAWBStat+(y*i4LineSize),
                         i4AwbLineSize);
            }
        }
        #ifdef TSF_THREAD_BUILD
        if (E_TSF_CMD_RUN == eCmd)
        {
            m_eTsfCmd = E_TSF_CMD_RUN;
            ::sem_post(&m_Sema);
        }
        else
        {
            tsfPostCmd(eCmd);
        }
        #else
        tsfPostCmd(eCmd);
        #endif
    }
    TSF_UNLOCK();
    //LSC_LOG_END();
#else
    if (getTsfOnOff())
        ::sem_post(&m_Sema);
#endif
}

MVOID
LscMgr2::
updateTsfCfg(UINT32 u4W, UINT32 u4H, const ISP_NVRAM_LSC_T& rLscCfg, ELscScenario_T eLscScn, MBOOL fgForce)
{
#if defined(TSF_BUILD) && !defined(LSC_DBG)
    LSC_LOG_BEGIN();

    TSF_LOCK();

    MBOOL fgBlkChg =
        (m_rTsfLscParam.x_grid_num != rLscCfg.ctl2.bits.SDBLK_XNUM+2)||
        (m_rTsfLscParam.y_grid_num != rLscCfg.ctl3.bits.SDBLK_YNUM+2);
    MBOOL fgAspectChg =
        (m_rTsfLscParam.raw_wd*u4H != m_rTsfLscParam.raw_ht*u4W);

    m_rTsfLscParam.raw_wd        = u4W;
    m_rTsfLscParam.raw_ht        = u4H;

    m_rTsfLscParam.x_offset      = rLscCfg.ctl1.bits.SDBLK_XOFST;
    m_rTsfLscParam.y_offset      = rLscCfg.ctl1.bits.SDBLK_YOFST;
    m_rTsfLscParam.block_wd      = rLscCfg.ctl2.bits.SDBLK_WIDTH;
    m_rTsfLscParam.block_ht      = rLscCfg.ctl3.bits.SDBLK_HEIGHT;
    m_rTsfLscParam.x_grid_num    = rLscCfg.ctl2.bits.SDBLK_XNUM+2;
    m_rTsfLscParam.y_grid_num    = rLscCfg.ctl3.bits.SDBLK_YNUM+2;
    m_rTsfLscParam.block_wd_last = rLscCfg.lblock.bits.SDBLK_lWIDTH;
    m_rTsfLscParam.block_ht_last = rLscCfg.lblock.bits.SDBLK_lHEIGHT;

    LSC_LOG("raw_wd(%d), raw_ht(%d), block_wd(%d), block_ht(%d), xgrid(%d), ygrid(%d), wd_last(%d), ht_last(%d)",
        m_rTsfLscParam.raw_wd,
        m_rTsfLscParam.raw_ht,
        m_rTsfLscParam.block_wd,
        m_rTsfLscParam.block_ht,
        m_rTsfLscParam.x_grid_num,
        m_rTsfLscParam.y_grid_num,
        m_rTsfLscParam.block_wd_last,
        m_rTsfLscParam.block_ht_last);

    //
    if (!m_bTSFInit || fgBlkChg || fgAspectChg || fgForce)
    {
        LSC_LOG("Need to reset table, eLscScn(%d), fgBlkChg(%d), fgAspectChg(%d)", eLscScn, fgBlkChg, fgAspectChg);
        tsfResetTbl(eLscScn);
    }
    else
    {
        LSC_LOG("No need to reset table");
    }

    //if (getTsfOnOff())
    {
        if (!m_bTSFInit || fgAspectChg || fgBlkChg || fgForce)
        {
            tsfPostCmd(E_TSF_CMD_INIT);
            tsfPostCmd(E_TSF_CMD_CHG);
            m_bTSFInit = MTRUE;
        }
        else
        {
            tsfPostCmd(E_TSF_CMD_CHG);
        }
    }

    m_u4PerFrameStep = 0;

    TSF_UNLOCK();
    LSC_LOG_END();
#endif
}


#ifdef LSC_DBG

static MFLOAT *m_pGainTbl = NULL;
static MFLOAT *m_pGainTbl_Mod = NULL;


MVOID
LscMgr2::
initThreadFunc()
{
    m_pGainTbl = m_pGainTbl_Mod = NULL;
    FILE* pFid = fopen("/sdcard/DestinationGain.bin", "rb");
    if (pFid == NULL)
    {
        m_pGainTbl = 0;
    }
    else
    {
        m_pGainTbl = new MFLOAT[17*17*4];
        m_pGainTbl_Mod = new MFLOAT[17*17*4*11];
        fread(m_pGainTbl, 17*17*4*4, 1, pFid);
        fclose(pFid);

        for (int i = 0; i < 17*17; i++)
        {
            MY_LOG("%s(): %f, %f, %f, %f\n", __FUNCTION__,
                m_pGainTbl[4*i+0], m_pGainTbl[4*i+1], m_pGainTbl[4*i+2], m_pGainTbl[4*i+3]);
        }

        if (m_pGainTbl && m_pGainTbl_Mod)
        {
            for (int j = 0; j < 11; j++)
            {
                MUINT32 offset = 17*17*4*j;
                MFLOAT a = (MFLOAT) j / 10.0f;
                for (int i = 0; i < 17*17*4; i++)
                {
                    m_pGainTbl_Mod[offset+i] = m_pGainTbl[i]*(1.0f-a) + 1.0f*a;
                }
            }
        }
    }

}

MVOID
LscMgr2::
uninitThreadFunc()
{
    if (m_pGainTbl)
    {
        delete [] m_pGainTbl;
        m_pGainTbl = NULL;
    }
    if (m_pGainTbl_Mod)
    {
        delete [] m_pGainTbl_Mod;
        m_pGainTbl_Mod = NULL;
    }
}

MVOID
LscMgr2::
doThreadFunc()
{
    MINT32 i4Step = 0;
    MBOOL fgPos = 1;

    LSC_LOG_BEGIN();

    while (m_fgThreadLoop)
    {
        ::sem_wait(&m_Sema);
        TSF_LOCK();
        MINT32 fgFreeze = 0;
        GET_PROP("debug.lsc_mgr.freeze", "0", fgFreeze);
        if (!fgFreeze)
        {
            if (fgPos)
            {
                i4Step ++;
                if (i4Step >= 10)
                    fgPos = 0;
            }
            else
            {
                i4Step --;
                if (i4Step <= 0)
                    fgPos = 1;
            }
        }

        if (m_pGainTbl_Mod)
        {
            setGainTable(17, 17, 3200, 2400, m_pGainTbl_Mod+(17*17*4*i4Step));
            updateLsc();
            IspTuningMgr::getInstance().forceValidate();
            LSC_LOG("i4Step(%d)", i4Step);
        }
        TSF_UNLOCK();
    }

    LSC_LOG_END();
}
#else

MVOID
LscMgr2::
initThreadFunc()
{
    LSC_LOG_BEGIN();

    m_u4PerFrameStep = 0;
    m_prTsf = MTKTsf::createInstance();

    // create tsf instance
    if (!m_prTsf)
    {
        LSC_ERR("NULL TSF instance");
        m_eTsfCmd = E_TSF_CMD_IDLE;
        m_fgThreadLoop = 0;
    }
    else
    {
        AWB_STAT_PARAM_T rAwbStatParma;

        switch (m_eSensorDev)
        {
        default:
        case ESensorDev_Main:
            rAwbStatParma = getAWBStatParam<ESensorDev_Main>();
            break;
        case ESensorDev_Sub:
            rAwbStatParma = getAWBStatParam<ESensorDev_Sub>();
            break;
        case ESensorDev_MainSecond:
            rAwbStatParma = getAWBStatParam<ESensorDev_MainSecond>();
            break;
        }

        m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_ENV_INFO, 0, &m_rTsfGetEnvInfo);
        LSC_LOG("MTKTSF_FEATURE_GET_ENV_INFO, buffer size(%d)", m_rTsfGetEnvInfo.WorkingBuffSize);

        // allocate working buffer
        MUINT8* gWorkinBuffer = new MUINT8[m_rTsfGetEnvInfo.WorkingBuffSize];
        ::memset(gWorkinBuffer, 0, m_rTsfGetEnvInfo.WorkingBuffSize);

        // allocate AWB info
        m_rTsfRefInfo.u4SizeAwbStat = (rAwbStatParma.i4WindowNumX * rAwbStatParma.i4WindowNumY * 4);
        m_rTsfRefInfo.prAwbStat = new MUINT8[m_rTsfRefInfo.u4SizeAwbStat];
        m_prAwbStat = new MUINT8[m_rTsfRefInfo.u4SizeAwbStat];
        LSC_LOG("AWB stat alloc size(%d), tmp(%p), algo(%p)", m_rTsfRefInfo.u4SizeAwbStat, m_rTsfRefInfo.prAwbStat, m_prAwbStat);

        // allocate table
        m_pu4ProcTbl = new MUINT32[MAX_SHADING_CapTil_SIZE];
        m_pu4ResultTbl = new MUINT32[MAX_SHADING_CapTil_SIZE];

        // construct data relationship
        m_rTsfEnvInfo.ImgWidth    = rAwbStatParma.i4WindowNumX;
        m_rTsfEnvInfo.ImgHeight   = rAwbStatParma.i4WindowNumY;
        m_rTsfEnvInfo.BayerOrder  = MTK_BAYER_B;

        m_rTsfEnvInfo.WorkingBufAddr = (MUINT32*)gWorkinBuffer;
        m_rTsfEnvInfo.pLscConfig = &m_rTsfLscParam;
        m_rTsfEnvInfo.BaseShadingTbl = (MINT32*)m_pu4ProcTbl;
        m_rTsfEnvInfo.ImgAddr = m_prAwbStat;

        tsfResetTbl(TSF_SCN_DFT);

        m_rTsfTbl.pLscConfig = &m_rTsfLscParam;
        m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4ProcTbl;

        m_rTsfSetProc.ShadingTbl = (MINT32*)m_pu4ProcTbl;
        m_rTsfResult.ShadingTbl = m_pu4ResultTbl;

        // for exif
        m_pTsfResultInfo = &m_rTsfResult;

        // golden/unit alignment
        if (m_i4OtpState == E_LSC_WITH_MTK_OTP /*m_rTsfEnvInfo.WithOTP*/)
        {
            loadOtpDataForTsf();
        }
        else
        {
            LSC_LOG("No OTP Data");
            m_rTsfEnvInfo.ShadingTbl.Golden.Tbl = NULL;
            m_rTsfEnvInfo.ShadingTbl.Unit.Tbl = NULL;
        }

        m_bTSFInit = MFALSE;
    }

    LSC_LOG_END();
}

MVOID
LscMgr2::
uninitThreadFunc()
{
    LSC_LOG_BEGIN();

    m_prTsf->TsfExit();
    m_prTsf->destroyInstance(m_prTsf);

    delete [] m_pu4ProcTbl;
    delete [] m_pu4ResultTbl;
    delete [] m_prAwbStat;
    delete [] m_rTsfRefInfo.prAwbStat;
    delete [] (MUINT8*)m_rTsfEnvInfo.WorkingBufAddr;

    if (m_rTsfEnvInfo.ShadingTbl.Golden.Tbl)
        delete [] m_rTsfEnvInfo.ShadingTbl.Golden.Tbl;

    if (m_rTsfEnvInfo.ShadingTbl.Unit.Tbl)
        delete [] m_rTsfEnvInfo.ShadingTbl.Unit.Tbl;

    LSC_LOG_END();
}

MBOOL
LscMgr2::
tsfResetTbl(ELscScenario_T eLscScn)
{
    // reset proc shading table and result shading table
    MUINT32* pSrc = (MUINT32*)((MUINT8*)(getLut(eLscScn))+m_rTsfCfgTbl.TSF_CFG.tsfCtIdx*getPerLutSize(eLscScn));
    LSC_LOG("eLscScn(%d), m_pu4ProcTbl(%p), m_pu4ResultTbl(%p), src(%p)", eLscScn, m_pu4ProcTbl, m_pu4ResultTbl, pSrc);
    ::memcpy(m_pu4ProcTbl, pSrc, getPerLutSize(eLscScn));
    ::memcpy(m_pu4ResultTbl, pSrc, getPerLutSize(eLscScn));
    return MTRUE;
}

MBOOL
LscMgr2::
tsfSetTbl(const MUINT32* pu4Tbl, MBOOL fgOverWriteInput)
{
    if (fgOverWriteInput)
    {
        ::memcpy(m_pu4ProcTbl, pu4Tbl, getPerLutSize(TSF_SCN_DFT));
    }

    if (getTsfOnOff())
        setCoefTable(pu4Tbl);

    return MTRUE;
}

MBOOL
LscMgr2::
tsfSetSL2(const MTK_TSF_SL2_PARAM_STRUCT& rSL2)
{
    if (getTsfOnOff())
    {
        m_rSl2Cfg.cen.bits.SL2_CENTER_X = rSL2.SL2_CENTR_X;
        m_rSl2Cfg.cen.bits.SL2_CENTER_Y = rSL2.SL2_CENTR_Y;
        m_rSl2Cfg.max0_rr.val = rSL2.SL2_RR_0;
        m_rSl2Cfg.max1_rr.val = rSL2.SL2_RR_1;
        m_rSl2Cfg.max2_rr.val = rSL2.SL2_RR_2;
    }
    return MTRUE;
}

MBOOL
LscMgr2::
tsfSetProcInfo(MTK_TSF_SET_PROC_INFO_STRUCT& rProcInfo, MBOOL fgDump, const char* filename)
{
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_SET_PROC) ? MTRUE : fgDump;

    const MINT32* pAwbForceParam = m_rTsfCfgTbl.TSF_CFG.rAWBInput;

    if (m_bTsfForceAwb && pAwbForceParam)
    {
        rProcInfo.ParaL           = pAwbForceParam[0];
        rProcInfo.ParaC           = pAwbForceParam[1];
        rProcInfo.FLUO_IDX        = pAwbForceParam[2];
        rProcInfo.DAY_FLUO_IDX    = pAwbForceParam[3];
    }
    else
    {
        rProcInfo.ParaL           = m_rTsfRefInfo.rAwbInfo.m_i4LV;
        rProcInfo.ParaC           = m_rTsfRefInfo.rAwbInfo.m_u4CCT;
        rProcInfo.FLUO_IDX        = m_rTsfRefInfo.rAwbInfo.m_FLUO_IDX;
        rProcInfo.DAY_FLUO_IDX    = m_rTsfRefInfo.rAwbInfo.m_DAY_FLUO_IDX;
    }
    rProcInfo.Gain.i4R = m_rTsfRefInfo.rAwbInfo.m_RGAIN;
    rProcInfo.Gain.i4G = m_rTsfRefInfo.rAwbInfo.m_GGAIN;
    rProcInfo.Gain.i4B = m_rTsfRefInfo.rAwbInfo.m_BGAIN;

    // AWB stat
    const MUINT8* pSrc = m_rTsfRefInfo.prAwbStat;
    MUINT8* pDst = m_prAwbStat;
    ::memcpy(pDst, pSrc, m_rTsfRefInfo.u4SizeAwbStat);

    if (fgDump)
    {
        FILE* fptr = fopen(filename, "wb");

        if (fptr)
        {
            fwrite(pSrc, m_rTsfRefInfo.u4SizeAwbStat, 1, fptr);
            fwrite(&rProcInfo, sizeof(MTK_TSF_SET_PROC_INFO_STRUCT), 1, fptr);
            fclose(fptr);
        }
    }

    MY_LOG_IF(fgLogEn, "[%s] ForceAwb(%d), L(%d), C(%d), F(%d), DF(%d), R(%d), G(%d), B(%d)\n",
        __FUNCTION__, m_bTsfForceAwb, rProcInfo.ParaL, rProcInfo.ParaC, rProcInfo.FLUO_IDX, rProcInfo.DAY_FLUO_IDX, rProcInfo.Gain.i4R, rProcInfo.Gain.i4G, rProcInfo.Gain.i4B);
    MY_LOG_IF(fgLogEn, "[%s] m_prAwbStat(%p), tmpBuf(%p), size(%d)",
        __FUNCTION__, pDst, pSrc, m_rTsfRefInfo.u4SizeAwbStat);

    return MTRUE;
}

MBOOL
LscMgr2::
loadOtpDataForTsf()
{
#define UNIT_GAIN_FROM_NVRAM 0

    MTK_BAYER_ORDER_ENUM eBayerGain = MTK_BAYER_B;

    switch (m_prShadingLut->SensorGoldenCalTable.PixId)
    {
    case 1: eBayerGain = MTK_BAYER_B;    break;
    case 2: eBayerGain = MTK_BAYER_Gb;   break;
    case 4: eBayerGain = MTK_BAYER_Gr;   break;
    case 8: eBayerGain = MTK_BAYER_R;    break;
    }

    MUINT32 u4XNum, u4YNum, u4BlkW, u4BlkH, u4LastW, u4LastH;
    #if UNIT_GAIN_FROM_NVRAM
    u4XNum  = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] >> 28) & 0x0000000F);
    u4YNum  = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] >> 12) & 0x0000000F);
    u4BlkW  = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] >> 16) & 0x00000FFF);
    u4BlkH  = ( m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1]        & 0x00000FFF);
    u4LastW = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[3] >> 16) & 0x00000FFF);
    u4LastH = ( m_prShadingLut->SensorGoldenCalTable.IspLSCReg[3]        & 0x00000FFF);
    #else
    const CAM_CAL_LSC_MTK_TYPE& rMtkLsc = m_rOtp.LscTable.MtkLcsData;
    u4XNum  = ((rMtkLsc.CapIspReg[1] >> 28) & 0x0000000F);
    u4YNum  = ((rMtkLsc.CapIspReg[1] >> 12) & 0x0000000F);
    u4BlkW  = ((rMtkLsc.CapIspReg[1] >> 16) & 0x00000FFF);
    u4BlkH  = ( rMtkLsc.CapIspReg[1]        & 0x00000FFF);
    u4LastW = ((rMtkLsc.CapIspReg[3] >> 16) & 0x00000FFF);
    u4LastH = ( rMtkLsc.CapIspReg[3]        & 0x00000FFF);
    #endif
    // Golden/Unit
    // for TSF
    GAIN_TBL& rTsfGainGolden    = m_rTsfEnvInfo.ShadingTbl.Golden;
    GAIN_TBL& rTsfGainUnit      = m_rTsfEnvInfo.ShadingTbl.Unit;
    rTsfGainGolden.bayer        = eBayerGain;
    rTsfGainGolden.offset_x     = 0;
    rTsfGainGolden.offset_y     = 0;
    rTsfGainGolden.crop_width   = u4BlkW*u4XNum + u4LastW;
    rTsfGainGolden.crop_height  = u4BlkH*u4YNum + u4LastH;
    rTsfGainGolden.grid_x       = u4XNum + 2;
    rTsfGainGolden.grid_y       = u4YNum + 2;
    rTsfGainUnit = rTsfGainGolden;

    MUINT32 u4GainTblSize = (u4XNum + 2)*(u4YNum + 2)*4*2;  // in byte (x*y*4ch*2byte)

    rTsfGainGolden.Tbl          = new MUINT32[u4GainTblSize/4];
    rTsfGainUnit.Tbl            = new MUINT32[u4GainTblSize/4];
    ::memcpy(rTsfGainGolden.Tbl, m_prShadingLut->SensorGoldenCalTable.GainTable, u4GainTblSize);
    #if UNIT_GAIN_FROM_NVRAM
    ::memcpy(rTsfGainUnit.Tbl, m_prShadingLut->SensorGoldenCalTable.UnitGainTable, u4GainTblSize);
    #else
    ::memcpy(rTsfGainUnit.Tbl, rMtkLsc.CapTable, u4GainTblSize);
    #endif

    LSC_LOG("Golden(%p), Unit(%p)", rTsfGainGolden.Tbl, rTsfGainUnit.Tbl);
    LSC_LOG("Bayer(%d), Crop(%d,%d,%d,%d), Grid(%d,%d)",
        eBayerGain, rTsfGainGolden.offset_x, rTsfGainGolden.offset_y, rTsfGainGolden.crop_width, rTsfGainGolden.crop_height,
        rTsfGainGolden.grid_x, rTsfGainGolden.grid_y);

    return MTRUE;
}

MBOOL
LscMgr2::
tsfInit()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN();

    m_prTsf->TsfExit();

    m_rTsfEnvInfo.Para        = m_rTsfCfgTbl.TSF_DATA;
    m_rTsfEnvInfo.pTuningPara = m_rTsfCfgTbl.TSF_PARA;

    m_rTsfEnvInfo.TS_TS       = 1;
    m_rTsfEnvInfo.MA_NUM      = 5;

    m_rTsfEnvInfo.WithOTP = (
        ((m_fg1to3 == E_LSC_123_WITH_MTK_OTP_OK) &&
         (m_i4OtpState==E_LSC_WITH_MTK_OTP))
          ? MTRUE : MFALSE);

    LSC_LOG("WithOTP(%d)", m_rTsfEnvInfo.WithOTP);

    m_rTsfEnvInfo.EnableORCorrection = 0;
    // AWB NVRAM
    m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4R   = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rUnitGain.i4R; // 512;
    m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4G   = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rUnitGain.i4G; // 512;
    m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4B   = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rUnitGain.i4B; // 512;
    m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4R = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rGoldenGain.i4R; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4G = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rGoldenGain.i4G; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4B = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rGoldenGain.i4B; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4R    = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rPredictorGain.rAWBGain_LSC.i4R; //809;
    m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4G    = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rPredictorGain.rAWBGain_LSC.i4G; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4B    = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rPredictorGain.rAWBGain_LSC.i4B; //608;
    LSC_LOG("AwbNvramInfo: UnitGain(%d, %d, %d), GoldenGain(%d, %d, %d), D65Gain(%d, %d, %d)",
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4R  ,
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4G  ,
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4B  ,
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4R,
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4G,
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4B,
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4R   ,
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4G   ,
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4B   );

    // SensorCrop
    // 0: full 4:3 FOV
    // 1: full 16:9 FOV (full horizontal FOV, cropped vertical FOV)
    // 2: general cropping case
    m_rTsfEnvInfo.Raw16_9Mode = 2;
    #if 1
    const ShadingTblCropCfg_T& rCropCfg     = m_rSensorCropCfg[m_eSensorMode];
    m_rTsfEnvInfo.SensorCrop.full_width        = rCropCfg.u4Width;  //m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.full_height    = rCropCfg.u4Height; //m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.resize_width    = rCropCfg.u4OutputWidth; //m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.resize_height    = rCropCfg.u4OutputHeight;//m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_width        = rCropCfg.rCrop.u4W; //m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.crop_height    = rCropCfg.rCrop.u4H; //m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_hor_offs    = rCropCfg.rCrop.u4X; //0;
    m_rTsfEnvInfo.SensorCrop.crop_ver_offs    = rCropCfg.rCrop.u4Y; //0;
    #else
    m_rTsfEnvInfo.SensorCrop.full_width        = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.full_height    = m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.resize_width    = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.resize_height    = m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_width        = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.crop_height    = m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_hor_offs    = 0;
    m_rTsfEnvInfo.SensorCrop.crop_ver_offs    = 0;
    #endif
    LSC_LOG("SensorCrop(%d): Full(%d,%d), Resize(%d,%d), Crop(%d,%d,%d,%d)",
        m_eSensorMode,
        m_rTsfEnvInfo.SensorCrop.full_width      ,
        m_rTsfEnvInfo.SensorCrop.full_height  ,
        m_rTsfEnvInfo.SensorCrop.resize_width ,
        m_rTsfEnvInfo.SensorCrop.resize_height,
        m_rTsfEnvInfo.SensorCrop.crop_hor_offs,
        m_rTsfEnvInfo.SensorCrop.crop_ver_offs,
        m_rTsfEnvInfo.SensorCrop.crop_width   ,
        m_rTsfEnvInfo.SensorCrop.crop_height  );

    // reset shading table
    //tsfResetTbl(m_eLscScenario /*TSF_SCN_DFT*/);

    MINT32 i4TsfSL2En = 0;
    GET_PROP("debug.lsc_mgr.sl2", "-1", i4TsfSL2En);

    if (i4TsfSL2En == -1)
    {
        // 0:disable, 1:TSF's SL2, 2:NVRAM default
        i4TsfSL2En = isEnableSL2(m_eSensorDev);
        LSC_LOG("TSF set SL2 default mode(%d)", i4TsfSL2En);
    }
    else
    {
        LSC_LOG("TSF set SL2 mode(%d)", i4TsfSL2En);
    }

    m_rTsfEnvInfo.EnableSL2   = i4TsfSL2En;

    LSC_LOG("ImgWidth(%d), ImgHeight(%d), BayerOrder(%d), ImgAddr(%p), ShadingTbl(%p), Raw16_9Mode(%d), EnableSL2(%d), pLscConfig(%p)",
        m_rTsfEnvInfo.ImgWidth, m_rTsfEnvInfo.ImgHeight, m_rTsfEnvInfo.BayerOrder, m_rTsfEnvInfo.ImgAddr,
        m_rTsfEnvInfo.BaseShadingTbl, m_rTsfEnvInfo.Raw16_9Mode, m_rTsfEnvInfo.EnableSL2, m_rTsfEnvInfo.pLscConfig);

    // init
    ret = m_prTsf->TsfInit(&m_rTsfEnvInfo, NULL);

    // set proc info
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &m_rTsfSetProc, NULL);

    m_prTsf->TsfReset();

    LSC_LOG_END();

    return MTRUE;
}

MBOOL
LscMgr2::
tsfCfgChg()
{
    MRESULT ret = S_TSF_OK;

    // this is for identical sized tables, but block numbers are different.
    // no cropping support.
    // ex. 10x10 -> 16x16

    LSC_LOG_BEGIN();

    // convert current gain table from nxn to mxm (ex. 10x10 to 16x16)
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
        m_prTsf->TsfReset();
        goto lbExit;
    }

    // convert transformed table to HW coef table.
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &m_rTsfTbl, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_GEN_CAP_TBL", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }


lbExit:
    LSC_LOG_END();

    return ret == S_TSF_OK;
}

MBOOL
LscMgr2::
tsfBatch()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN();
    tsfSetProcInfo(m_rTsfSetProc);
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &m_rTsfSetProc, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }

    LSC_LOG_END();

    return ret == S_TSF_OK;
}

MBOOL
LscMgr2::
tsfBatchCap()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN();

    MBOOL fgDumpEn = (m_u4LogEn & EN_LSC_LOG_TSF_DUMP) ? MTRUE : MFALSE;

    if (fgDumpEn)
    {
        ISP_NVRAM_LSC_T rLscCfg;
        rLscCfg.ctl1.bits.SDBLK_XOFST           = m_rTsfLscParam.x_offset;
        rLscCfg.ctl1.bits.SDBLK_YOFST           = m_rTsfLscParam.y_offset;
        rLscCfg.ctl2.bits.SDBLK_WIDTH       = m_rTsfLscParam.block_wd;
        rLscCfg.ctl3.bits.SDBLK_HEIGHT      = m_rTsfLscParam.block_ht;
        rLscCfg.ctl2.bits.SDBLK_XNUM        = m_rTsfLscParam.x_grid_num - 2;
        rLscCfg.ctl3.bits.SDBLK_YNUM        = m_rTsfLscParam.y_grid_num - 2;
        rLscCfg.lblock.bits.SDBLK_lWIDTH    = m_rTsfLscParam.block_wd_last;
        rLscCfg.lblock.bits.SDBLK_lHEIGHT   = m_rTsfLscParam.block_ht_last;

        if (dumpSdblk("/sdcard/tsfInput", rLscCfg, reinterpret_cast<MUINT32*>(m_rTsfSetProc.ShadingTbl)) != 0)
            LSC_ERR("dumpSdblk error");
    }

    if (m_u4LogEn & EN_LSC_LOG_TSF_REINIT)
    {
        tsfInit();
    }
    else
    {
        m_prTsf->TsfReset();
    }
    tsfSetProcInfo(m_rTsfSetProc, fgDumpEn, "/sdcard/tsfAwbStat.bin");
    // disable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)0, 0);

    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &m_rTsfSetProc, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }

    // enable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)1, 0);

    LSC_LOG_END();

    return ret == S_TSF_OK;
}


MBOOL
LscMgr2::
tsfRun()
{
    MRESULT ret = S_TSF_OK;

    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    MY_LOG_IF(fgLogEn, "[%s +]", __FUNCTION__);

    if (m_u4PerFrameStep == 0)
    {
        // only set proc info at the 1st frame.
        tsfSetProcInfo(m_rTsfSetProc);
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &m_rTsfSetProc, NULL);
    }
    MY_LOG_IF(fgLogEn, "[%s](%d)", __FUNCTION__, m_u4PerFrameStep);
    m_u4PerFrameStep ++;

    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_PROC_INFO", ret);
        m_prTsf->TsfReset();
        goto lbExit;
    }
    ret = m_prTsf->TsfMain();
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): TsfMain", ret);
        m_prTsf->TsfReset();
        goto lbExit;
    }

    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_PROC_INFO, 0, &m_rTsfGetProc);
    if (m_rTsfGetProc.TsfState == MTKTSF_STATE_READY)
    {
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_RESULT, NULL, &m_rTsfResult);
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
        m_u4PerFrameStep = 0;
        updateLsc();
        IspTuningMgr::getInstance().forceValidate();
    }

lbExit:
    MY_LOG_IF(fgLogEn, "[%s -](0x%08x)", __FUNCTION__, ret);

    return ret == S_TSF_OK;
}


MVOID
LscMgr2::
doThreadFunc()
{
    LSC_LOG_BEGIN();

    while (m_fgThreadLoop)
    {
        ::sem_wait(&m_Sema);
        TSF_LOCK();
        E_TSF_CMD_T eCmd = m_eTsfCmd;
        tsfPostCmd(eCmd);
        TSF_UNLOCK();
    }

    LSC_LOG_END();
}

MBOOL
LscMgr2::
tsfPostCmd(E_TSF_CMD_T eCmd)
{
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_THREAD) ? MTRUE : MFALSE;
    MBOOL fgOnOff = getTsfOnOff();

    switch (eCmd)
    {
    case E_TSF_CMD_INIT:
        LSC_LOG("TSF(%s), eCmd(E_TSF_CMD_INIT)", (fgOnOff?"On":"Off"));
        tsfInit();
        if (fgOnOff)
        {
            updateLsc();
            IspTuningMgr::getInstance().forceValidate();
        }
        break;
    case E_TSF_CMD_CHG:
        LSC_LOG("TSF(%s), eCmd(E_TSF_CMD_CHG)", (fgOnOff?"On":"Off"));
        tsfCfgChg();
        if (fgOnOff)
        {
            updateLsc();
            IspTuningMgr::getInstance().forceValidate();
        }
        break;
    case E_TSF_CMD_BATCH:
        MY_LOG_IF(fgLogEn, "[%s] TSF(%s), eCmd(E_TSF_CMD_BATCH)", __FUNCTION__, (fgOnOff?"On":"Off"));
        if (fgOnOff)
        {
            tsfBatch();
            updateLsc();
            IspTuningMgr::getInstance().forceValidate();
        }
        break;
    case E_TSF_CMD_BATCH_CAP:
        MY_LOG_IF(fgLogEn, "[%s] TSF(%s), eCmd(E_TSF_CMD_BATCH_CAP)", __FUNCTION__, (fgOnOff?"On":"Off"));
        if (fgOnOff)
        {
            tsfBatchCap();
            updateLsc();
            IspTuningMgr::getInstance().forceValidate();
        }
        break;
    case E_TSF_CMD_RUN:
        if (fgOnOff)
        {
            MY_LOG_IF(fgLogEn, "[%s] SensorDev(%d) TSF ON, eCmd(E_TSF_CMD_RUN)", __FUNCTION__, m_eSensorDev);
            tsfRun();
        }
        break;
    default:
        LSC_LOG("TSF ON, eCmd(E_TSF_CMD_IDLE)");
        break;
    }

    return MTRUE;
}
#endif //LSC_DBG

MVOID*
LscMgr2::
threadLoop(void* arg)
{
#ifdef TSF_THREAD_BUILD
    LscMgr2* _this = reinterpret_cast<LscMgr2*>(arg);

    _this->changeThreadSetting();

    //_this->initThreadFunc();

    _this->doThreadFunc();

    //_this->uninitThreadFunc();
#endif
    return NULL;
}

}
