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
#define LOG_TAG "lsc_mgr2_rto"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include "lsc_mgr2.h"

#include "nvram_drv.h"
#include "nvram_drv_mgr.h"

#include <mtkcam/hwutils/CameraProfile.h>
#include <mtkcam/hal/IHalSensor.h>

#include "shading_tuning_custom.h"
#include <isp_mgr.h>

#include <mtkcam/algorithm/libtsf/MTKTsf.h>

#include "tsf_tuning_custom.h"

#include "cam_cal_drv.h"

#include "kd_imgsensor_define.h"
#include "nvbuf_util.h"
#include <isp_tuning_mgr.h>

namespace NSIspTuning {

using namespace NS3A;
using namespace std;

#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))
#define ROUND(a)  ((a) > 0.0f ? (int)(a + 0.5f):(int)(a - 0.5f))

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

//=============================================================================================
//
//=============================================================================================
class LscRatioImp : public ILscRatio
{
public:
    LscRatioImp();
    ~LscRatioImp();
    virtual MBOOL updateCfg(const ISP_NVRAM_LSC_T& rLscCfg);
    virtual MBOOL genHwTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest);
    virtual MBOOL genGainTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest);
private:
    // ra
    MTKLscUtil*                     m_pLscUtilInterface;
    LSC_PARAM_T                     m_rRaLscConfig;
    LSC_RA_STRUCT                   m_rRaConfig;
    MUINT32*                        m_pu4RaResultTbl;
};

LscRatioImp::
LscRatioImp()
    : m_pLscUtilInterface(NULL)
    , m_pu4RaResultTbl(NULL)
{
    m_pLscUtilInterface         = MTKLscUtil::createInstance();

    m_rRaConfig.working_buf	    = (int*)new MUINT8[LSC_RA_BUFFER_SIZE];
    m_rRaConfig.in_data_type    = SHADING_TYPE_COEFF;
    m_rRaConfig.out_data_type   = SHADING_TYPE_COEFF; // TSF 4x frame, use coef table
    m_rRaConfig.pix_id          = (int)MTK_BAYER_B;
    m_rRaConfig.ra              = 32;
    //m_rRaConfig.lsc_config      = m_rRaLscConfig;

    if (m_pLscUtilInterface)
    {
        m_pLscUtilInterface->LscRaSwInit((void*)m_rRaConfig.working_buf);
    }
    else
    {
        LSC_ERR("Fail to create MTKLscUtil!");
    }
}

LscRatioImp::
~LscRatioImp()
{
    if (m_rRaConfig.working_buf)
    {
        delete [] (MUINT8*) m_rRaConfig.working_buf;
        m_rRaConfig.working_buf = NULL;
    }

    if (m_pLscUtilInterface)
    {
        m_pLscUtilInterface->destroyInstance(m_pLscUtilInterface);
        m_pLscUtilInterface = NULL;
    }
}

MBOOL
LscRatioImp::
updateCfg(const ISP_NVRAM_LSC_T& rLscCfg)
{
    MUINT32 u4W, u4H;

    u4W = ((MUINT32) rLscCfg.ctl2.bits.SDBLK_WIDTH * rLscCfg.ctl2.bits.SDBLK_XNUM + rLscCfg.lblock.bits.SDBLK_lWIDTH) * 2;
    u4H = ((MUINT32) rLscCfg.ctl3.bits.SDBLK_HEIGHT * rLscCfg.ctl3.bits.SDBLK_YNUM + rLscCfg.lblock.bits.SDBLK_lHEIGHT) * 2;

    m_rRaLscConfig.raw_wd        = u4W;
    m_rRaLscConfig.raw_ht        = u4H;
    m_rRaLscConfig.crop_ini_x    = rLscCfg.ctl1.bits.SDBLK_XOFST;
    m_rRaLscConfig.crop_ini_y    = rLscCfg.ctl1.bits.SDBLK_YOFST;
    m_rRaLscConfig.block_wd      = rLscCfg.ctl2.bits.SDBLK_WIDTH;
    m_rRaLscConfig.block_ht      = rLscCfg.ctl3.bits.SDBLK_HEIGHT;
    m_rRaLscConfig.x_grid_num    = rLscCfg.ctl2.bits.SDBLK_XNUM+2;
    m_rRaLscConfig.y_grid_num    = rLscCfg.ctl3.bits.SDBLK_YNUM+2;
    m_rRaLscConfig.block_wd_last = rLscCfg.lblock.bits.SDBLK_lWIDTH;
    m_rRaLscConfig.block_ht_last = rLscCfg.lblock.bits.SDBLK_lHEIGHT;

    m_rRaConfig.lsc_config       = m_rRaLscConfig;
    return MTRUE;
}

MBOOL
LscRatioImp::
genHwTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest)
{
    MBOOL fgRet = MTRUE;
    LSC_RESULT ra_ret;

    if (m_pLscUtilInterface == NULL || m_rRaConfig.working_buf == NULL)
    {
        LSC_ERR("Null Util(%p, %p)", m_pLscUtilInterface, m_rRaConfig.working_buf);
        goto lbExit;
    }

    if (pSrc == NULL || pDest == NULL)
    {
        LSC_ERR("Null pointer(%p, %p)", pSrc, pDest);
        goto lbExit;
    }

    if (ratio > 32)
    {
        LSC_ERR("ratio(%d)", ratio);
        goto lbExit;
    }

    m_rRaConfig.in_data_type    = SHADING_TYPE_COEFF;
    m_rRaConfig.out_data_type   = SHADING_TYPE_COEFF;
    m_rRaConfig.in_tbl          = (int*)pSrc;
    m_rRaConfig.out_tbl         = (int*)pDest;
    m_rRaConfig.ra              = ratio;
    // do ratio
    ra_ret = m_pLscUtilInterface->LscRaSwMain((void*)&m_rRaConfig);

lbExit:
    return fgRet;
}

MBOOL
LscRatioImp::
genGainTbl(MUINT32 ratio, const MUINT32* pSrc, MUINT32* pDest)
{
    MBOOL fgRet = MTRUE;
    LSC_RESULT ra_ret;

    if (m_pLscUtilInterface == NULL || m_rRaConfig.working_buf == NULL)
    {
        LSC_ERR("Null Util(%p, %p)", m_pLscUtilInterface, m_rRaConfig.working_buf);
        goto lbExit;
    }

    if (pSrc == NULL || pDest == NULL)
    {
        LSC_ERR("Null pointer(%p, %p)", pSrc, pDest);
        goto lbExit;
    }

    if (ratio > 32)
    {
        LSC_ERR("ratio(%d)", ratio);
        goto lbExit;
    }

    m_rRaConfig.in_data_type    = SHADING_TYPE_COEFF;
    m_rRaConfig.out_data_type   = SHADING_TYPE_GAIN;
    m_rRaConfig.in_tbl          = (int*)pSrc;
    m_rRaConfig.out_tbl         = (int*)pDest;
    m_rRaConfig.ra              = ratio;
    // do ratio
    ra_ret = m_pLscUtilInterface->LscRaSwMain((void*)&m_rRaConfig);

lbExit:
    return fgRet;
}

//=============================================================================================
//
//=============================================================================================
#define SIZE_DOUBLE_BUF getPerLutSize(LSC_SCENARIO_CAP)

LscMgr2Rto*
LscMgr2Rto::
createInstance(ESensorDev_T const eSensorDev,
                 MINT32 i4SensorIdx,
                 NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram)
{
    LscMgr2Rto* pLscMgrRto = NULL;
    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static LscMgr2Rto singleton_main(ESensorDev_Main, i4SensorIdx, rIspNvram);
        pLscMgrRto = &singleton_main;
        LSC_LOG_END("RTO ESensorDev_Main(0x%08x), i4SensorIdx(%d)", pLscMgrRto, i4SensorIdx);
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static LscMgr2Rto singleton_mainsecond(ESensorDev_MainSecond, i4SensorIdx, rIspNvram);
        pLscMgrRto = &singleton_mainsecond;
        LSC_LOG_END("RTO ESensorDev_MainSecond(0x%08x), i4SensorIdx(%d)", pLscMgrRto, i4SensorIdx);
        break;
    case ESensorDev_Sub:        //  Sub Sensor
        static LscMgr2Rto singleton_sub(ESensorDev_Sub, i4SensorIdx, rIspNvram);
        pLscMgrRto = &singleton_sub;
        LSC_LOG_END("RTO ESensorDev_Sub(0x%08x), i4SensorIdx(%d)", pLscMgrRto, i4SensorIdx);
        break;
    }

    return pLscMgrRto;
}

LscMgr2Rto::
LscMgr2Rto(ESensorDev_T eSensorDev, MINT32 i4SensorIdx, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram)
    : m_pLscRatio(NULL)
    , m_pTsfRatio(NULL)
    , m_bTSF1stGroup(MFALSE)
    , LscMgr2(eSensorDev, i4SensorIdx, rIspNvram)
{
    LSC_LOG("Enter LscMgr2Rto");
}

LscMgr2Rto::
~LscMgr2Rto()
{
    LSC_LOG("Exit LscMgr2Rto");
}

MBOOL
LscMgr2Rto::
init()
{
    LSC_LOG_BEGIN("SensorDev(%d), byp123(%d)", m_eSensorDev, m_fgBypass1to3);

    GET_PROP("debug.lsc_mgr.log", "0", m_u4LogEn);

    IspDebug::getInstance().init();

    //CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);
    if (!RawLscTblInit())
    {
        LSC_ERR("FATAL WRONG m_pIMemDrv new Fail.\n");
    }
    //CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);

    if (0 == m_u4NumSensorModes)
    {
        // get resolution at the 1st time
        LSC_LOG("The 1st time, getSensorResolution");
        getSensorResolution();
    }
    else
    {
        LSC_LOG("Not the 1st time, show info only");
        showResolutionInfo();
    }
    
    loadTableFlow(MFALSE);

    m_fgCtIdxExcd = MFALSE;
    m_fgUserSetTbl = MFALSE;
    m_fgInit = MFALSE;

    m_pLscRatio = new LscRatioImp;
    m_pTsfRatio = new LscRatioImp;

    setTsfOnOff(m_rTsfCfgTbl.TSF_CFG.isTsfEn ? MTRUE : MFALSE);

    createThread();

    LSC_LOG_END();

    return MTRUE;
}

MBOOL
LscMgr2Rto::
uninit()
{
    LSC_LOG_BEGIN("SensorDev(%d)", m_eSensorDev);

    //m_u4CTIdx = 0;

    destroyThread();

    if (m_pTsfRatio)
    {
        delete m_pTsfRatio;
        m_pTsfRatio = NULL;
    }

    if (m_pLscRatio)
    {
        delete m_pLscRatio;
        m_pLscRatio = NULL;
    }

    RawLscTblUnInit();

    IspDebug::getInstance().uninit();

    return MTRUE;
}

MINT32
LscMgr2Rto::
setCoefTable(const MUINT32* pCoefTbl)
{
    MUINT32 u4Addr;
    MUINT32 u4Idx;
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_SET_TABLE) ? MTRUE : MFALSE;

    if (!pCoefTbl)
    {
        LSC_ERR("pCoefTbl is NULL");
        return -1;
    }

    u4Addr = IspDebug::getInstance().readLsciAddr();
    //u4Addr = m_rBufInfo[m_u4DoubleBufIdx].phyAddr;
    u4Idx = (m_u4DoubleBufIdx == 0 ? 1 : 0);
    
    if (u4Addr != m_rBufInfo[m_u4DoubleBufIdx].phyAddr)
    {
        u4Idx = m_u4DoubleBufIdx;
        MY_LOG_IF(fgLogEn, "[%s +] Error u4Idx(%d) Addr(0x%08x)\n", __FUNCTION__, u4Idx, u4Addr);
    }
    else
    {
        MY_LOG_IF(fgLogEn, "[%s +] OK u4Idx(%d) Addr(0x%08x)\n", __FUNCTION__, u4Idx, u4Addr);
    }
    
    ::memcpy((MUINT32*)m_rBufInfo[u4Idx].virtAddr, pCoefTbl, SIZE_DOUBLE_BUF);

    // flush to DRAM
    //m_pIMemDrv->cacheFlushAll();  
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &m_rBufInfo[u4Idx]);

    m_u4DoubleBufIdx = u4Idx;
    
    m_fgUserSetTbl = 1;
    
    MY_LOG_IF(fgLogEn, "[%s -]\n", __FUNCTION__);

    return 0;
}

MBOOL
LscMgr2Rto::
setSensorMode(ESensorMode_T eSensorMode, MBOOL fgForce)
{
    ESensorMode_T ePrevSensorMode;

    ePrevSensorMode = m_eSensorMode;

    if (eSensorMode != ePrevSensorMode || m_fgInit == MFALSE || fgForce)
    {
        m_fgInit = MTRUE;
        m_eSensorMode = eSensorMode;
        m_ePrevSensorMode = ePrevSensorMode;

        // update config
        SensorCropInfo_T rCrop;
        ELscScenario_T eLscScn = getLscScenario(eSensorMode);
        LSC_LOG("update config for eLscScn(%d), fgForce(%d)", eLscScn, fgForce);
        getResolution(eLscScn, rCrop);
        const ISP_NVRAM_LSC_T& rIspLscCfg = *getConfig(eLscScn);
        calLscConfig(
            rCrop,
            rIspLscCfg.ctl2.bits.SDBLK_XNUM,
            rIspLscCfg.ctl3.bits.SDBLK_YNUM,
            m_rLscCfg);
        calSl2Config(rCrop, m_rSl2Cfg);
        m_rCurSensorCrop = rCrop;
        m_eLscScenario = eLscScn;
        updateTsfCfg(rCrop.u4W, rCrop.u4H, m_rLscCfg, eLscScn, fgForce);
        if (m_pLscRatio)
        {
            m_pLscRatio->updateCfg(m_rLscCfg);
        }
    }
    
    return MTRUE;
}

MBOOL
LscMgr2Rto::
getTsfOnOff(void)
{
    MINT32 dbg_enable = 0;
    GET_PROP("debug.lsc_mgr.manual_tsf", "-1", dbg_enable);   

    if (dbg_enable != -1)
    {
        // overwrite for debug
        m_bTSF = dbg_enable == 0 ? MFALSE : MTRUE;
        if (!m_bTSF && !m_pLscRatio)
        {
            m_fgUserSetTbl = MFALSE;
        }
    }

    return m_bTSF;
}

MVOID
LscMgr2Rto::
setTsfOnOff(MBOOL fgOnOff)
{
    LSC_LOG(" (%d)", fgOnOff);
    m_bTSF = fgOnOff;
    if (!m_bTSF && !m_pLscRatio)
    {
        m_fgUserSetTbl = MFALSE;
    }
}

MINT32
LscMgr2Rto::
setLscRatio(MINT32 i4Rto)
{
    i4Rto = LscMgr2::setLscRatio(i4Rto);
    
    if (!m_bTSF && m_pLscRatio)
    {
        MUINT32 u4Ofst = getPerLutSize(m_eLscScenario) * m_u4CTIdx;
        MUINT32* pSrc = (MUINT32*)((MUINT8*) getLut(m_eLscScenario) + u4Ofst);
        m_pLscRatio->genHwTbl(i4Rto, pSrc, m_pu4RaResultTbl);
        setCoefTable(m_pu4RaResultTbl);
    }
    return i4Rto;
}

MVOID
LscMgr2Rto::
updateLsc()
{
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_UPDATE) ? MTRUE : MFALSE;
    MBOOL fgUserTbl = MFALSE;
    MUINT32 u4PhyAddr, u4CTIdx;
    MUINT8* pVirAddr;

    u4CTIdx = getCTIdx();
   
    switch (m_rTsfCfgTbl.TSF_CFG.isTsfEn)
    {
    default:
    case 0:
    case 1:
        fgUserTbl = m_fgUserSetTbl;
        break;
    case 2:
        if (m_fgCtIdxExcd)
        {
            fgUserTbl = m_fgUserSetTbl;
        }
        else
        {
            fgUserTbl = m_fgUserSetTbl && 0;
        }
        break;
    }

    // update physical address
    if (fgUserTbl)
    {
        u4PhyAddr = m_rBufInfo[m_u4DoubleBufIdx].phyAddr;
        pVirAddr = (MUINT8*) m_rBufInfo[m_u4DoubleBufIdx].virtAddr;
    }
    else
    {
        MUINT32 u4Ofst = getPerLutSize(m_eLscScenario) * u4CTIdx;
        u4PhyAddr = stRawLscInfo[m_eLscScenario].phyAddr + u4Ofst;
        pVirAddr = (MUINT8*) stRawLscInfo[m_eLscScenario].virtAddr + u4Ofst;
    }
    // update virtual address
    m_pBaseAddr = (MUINT32*)pVirAddr;
    m_rLscCfg.baseaddr.val = u4PhyAddr;

    // update ratio
    MUINT32 u4Rto = 32;
    m_i4RtoApplied = u4Rto;
    m_rLscCfg.ratio.val = ((u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto);

    MY_LOG_IF(fgLogEn, "[%s] OnOff(%d), CT(%d)", __FUNCTION__, m_fgOnOff, u4CTIdx);
    MY_LOG_IF(fgLogEn, "[%s] %s: BADDR(0x%08x), CTL1(0x%08x), CTL2(0x%08x), CTL3(0x%08x), LBLOCK(0x%08x), RTO(0x%08x)\n",
        __FUNCTION__, (fgUserTbl ? "User":"LSC"),
        u4PhyAddr,
        m_rLscCfg.ctl1.val, m_rLscCfg.ctl2.val,
        m_rLscCfg.ctl3.val, m_rLscCfg.lblock.val,
        m_rLscCfg.ratio.val);

    // put to isp_mgr buf
    ISP_MGR_LSC_T::getInstance(m_eSensorDev).enableLsc(getOnOff());
    //ISP_MGR_LSC_T::getInstance(m_eSensorDev).putAddr(u4PhyAddr);
    ISP_MGR_LSC_T::getInstance(m_eSensorDev).put(m_rLscCfg);


    // sl2
    ISP_NVRAM_SL2_T rSl2Cfg;
    MBOOL fgSl2OnOff = 0;
    MINT32 i4Idx = 0;  
    MINT32 dbg_enable = 0;
    GET_PROP("debug.lsc_mgr.sl2", "-1", dbg_enable);
    MINT32 i4Case = (dbg_enable != -1) ? dbg_enable : isEnableSL2(m_eSensorDev);
    
    switch (i4Case)
    {
    default:
    case 0:
        fgSl2OnOff = 0;
        i4Idx = m_eSensorMode*SHADING_SUPPORT_CT_NUM+u4CTIdx;
        if (i4Idx >= NVRAM_SL2_TBL_NUM)
        {
            LSC_ERR("0: SL2 idx(%d) exceeds NVRAM_SL2_TBL_NUM", i4Idx);
            return;
        }
        break;
    case 1:
        fgSl2OnOff = 1;
        i4Idx = m_eSensorMode*SHADING_SUPPORT_CT_NUM+u4CTIdx;
        if (i4Idx >= NVRAM_SL2_TBL_NUM)
        {
            LSC_ERR("1: SL2 idx(%d) exceeds NVRAM_SL2_TBL_NUM", i4Idx);
            return;
        }
        break;
    case 2:
        fgSl2OnOff = 1;
        i4Idx = m_eSensorMode*SHADING_SUPPORT_CT_NUM+getTSFD65Idx();
        if (i4Idx >= NVRAM_SL2_TBL_NUM)
        {
            LSC_ERR("2: SL2 idx(%d) exceeds NVRAM_SL2_TBL_NUM", i4Idx);
            return;
        }
        break;
    }

    if (getTsfOnOff())
        rSl2Cfg = m_rSl2Cfg;
    else
        rSl2Cfg = m_rIspNvram.ISPRegs.SL2[i4Idx];

    MY_LOG_IF(fgLogEn, "[%s] SL2 OnOff(%d), Idx(%d)", __FUNCTION__, fgSl2OnOff, i4Idx);
    MY_LOG_IF(fgLogEn, "[%s] SL2 cen(0x%08x), rr0(0x%08x), rr1(0x%08x), rr2(0x%08x)",
        __FUNCTION__, rSl2Cfg.cen, rSl2Cfg.max0_rr, rSl2Cfg.max1_rr, rSl2Cfg.max2_rr);
    if (NSIspTuning::IspTuningMgr::getInstance().getOperMode(m_eSensorDev) != NSIspTuning::EOperMode_Meta)
    {
        ISP_MGR_SL2_T::getInstance(m_eSensorDev).setEnable(fgSl2OnOff);
        ISP_MGR_SL2_T::getInstance(m_eSensorDev).put(rSl2Cfg);
    }
}

//=============================================================================================
//
//=============================================================================================
LscMgr2RtoCycle*
LscMgr2RtoCycle::
createInstance(ESensorDev_T const eSensorDev,
                 MINT32 i4SensorIdx,
                 NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram)
{
    LscMgr2RtoCycle* pLscMgrRto = NULL;
    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static LscMgr2RtoCycle singleton_main(ESensorDev_Main, i4SensorIdx, rIspNvram);
        pLscMgrRto = &singleton_main;
        LSC_LOG_END("RTOCycle ESensorDev_Main(0x%08x), i4SensorIdx(%d)", pLscMgrRto, i4SensorIdx);
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static LscMgr2RtoCycle singleton_mainsecond(ESensorDev_MainSecond, i4SensorIdx, rIspNvram);
        pLscMgrRto = &singleton_mainsecond;
        LSC_LOG_END("RTOCycle ESensorDev_MainSecond(0x%08x), i4SensorIdx(%d)", pLscMgrRto, i4SensorIdx);
        break;
    case ESensorDev_Sub:        //  Sub Sensor
        static LscMgr2RtoCycle singleton_sub(ESensorDev_Sub, i4SensorIdx, rIspNvram);
        pLscMgrRto = &singleton_sub;
        LSC_LOG_END("RTOCycle ESensorDev_Sub(0x%08x), i4SensorIdx(%d)", pLscMgrRto, i4SensorIdx);
        break;
    }

    return pLscMgrRto;
}

LscMgr2RtoCycle::
LscMgr2RtoCycle(ESensorDev_T eSensorDev, MINT32 i4SensorIdx, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram)
    : LscMgr2Rto(eSensorDev, i4SensorIdx, rIspNvram)
{
    LSC_LOG("Enter LscMgr2RtoCycle");
}

LscMgr2RtoCycle::
~LscMgr2RtoCycle()
{
    LSC_LOG("Exit LscMgr2RtoCycle");
}

};
