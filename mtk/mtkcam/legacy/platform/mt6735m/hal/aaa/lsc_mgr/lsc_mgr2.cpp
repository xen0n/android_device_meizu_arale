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

#define LOG_TAG "lsc_mgr2"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_log.h>

#include "nvram_drv.h"
#include "nvram_drv_mgr.h"

#include <mtkcam/hwutils/CameraProfile.h>
#include <mtkcam/hal/IHalSensor.h>

#include "shading_tuning_custom.h"
#include "tsf_tuning_custom.h"
#include <mtkcam/algorithm/libtsf/MTKTsf.h>
#include <mtkcam/algorithm/liblsctrans/ShadingTblTransform.h>
#include "cam_cal_drv.h"

#include "kd_imgsensor_define.h"
#include <mtkcam/featureio/nvbuf_util.h>
//#include <nvram_drv_mgr.h>
//#include <isp_tuning_mgr.h>
#include "lsc_mgr2.h"
#include <isp_mgr.h>


#ifdef LSC_DBG
#include "lsc_data2.h"
#endif

namespace NSIspTuningv3 {

using namespace NS3Av3;
using namespace NSCam;
using namespace std;
using namespace NSIspTuning;

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

#define SIZE_DOUBLE_BUF getPerLutSize(LSC_SCENARIO_CAP)

static LscMgr2* _pLscMain   = NULL;
static LscMgr2* _pLscMain2  = NULL;
static LscMgr2* _pLscSub    = NULL;

static MUINT32 _u4MemBufSize[LscMgr2::LSC_SCENARIO_NUM] =
{
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
    MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM
};

LscMgr2*
LscMgr2::
createInstance(ESensorDev_T const eSensorDev, MINT32 i4SensorIdx, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram)
{
    LSC_LOG_BEGIN("eSensorDev(0x%02x)\n", (MUINT32)eSensorDev);

    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static LscMgr2 singleton_main(ESensorDev_Main, i4SensorIdx, rIspNvram);
        _pLscMain = &singleton_main;
        LSC_LOG_END("ESensorDev_Main(%p), i4SensorIdx(%d)", _pLscMain, i4SensorIdx);
        return _pLscMain;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static LscMgr2 singleton_mainsecond(ESensorDev_MainSecond, i4SensorIdx, rIspNvram);
        _pLscMain2 = &singleton_mainsecond;
        LSC_LOG_END("ESensorDev_MainSecond(%p), i4SensorIdx(%d)", _pLscMain2, i4SensorIdx);
        return _pLscMain2;
    case ESensorDev_Sub:        //  Sub Sensor
        static LscMgr2 singleton_sub(ESensorDev_Sub, i4SensorIdx, rIspNvram);
        _pLscSub = &singleton_sub;
        LSC_LOG_END("ESensorDev_Sub(%p), i4SensorIdx(%d)", _pLscSub, i4SensorIdx);
        return _pLscSub;
    }
}

LscMgr2*
LscMgr2::
getInstance(ESensorDev_T eSensorDev)
{
    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        return _pLscMain;
    case ESensorDev_MainSecond: //  Main Second Sensor
        return _pLscMain2;
    case ESensorDev_Sub:        //  Sub Sensor
        return _pLscSub;
    }
}

LscMgr2::
LscMgr2(ESensorDev_T eSensorDev, MINT32 i4SensorIdx, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram)
    : m_eSensorDev(eSensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_eLscScenario(LSC_SCENARIO_PRV)
    , m_eSensorMode(ESensorMode_Preview)
    , m_ePrevSensorMode(ESensorMode_NUM)
    , m_u4NumSensorModes(0)
    , m_fgOnOff(MTRUE)
    , m_fgInit(MFALSE)
    , m_fgSensorCropInfoNull(MFALSE)
    , m_bIsEEPROMImported(MFALSE)
    , m_fgCtIdxExcd(MFALSE)
    , m_fgBypass1to3(MFALSE)
    , m_fg1to3(E_LSC_123_USE_CCT)
    , m_i4OtpState(E_LSC_NO_OTP)
    , m_u4CTIdx(0)
    , m_pBaseAddr(NULL)
    , m_u4LogEn(0)
    , m_bDumpSdblk(MFALSE)
    , m_pTsfResultInfo(NULL)
    , m_pIMemDrv(NULL)
    , m_u4DoubleBufIdx(0)
    , m_fgUserSetTbl(MFALSE)
    , m_rIspLscCfg(rIspNvram.ISPRegs.LSC)
    , m_rIspNvram(rIspNvram)
    //, m_rIspSl2Cfg(rIspNvram.ISPRegs.SL2)
    , m_prNvram3A(NULL)
    , m_prShadingLut(NULL)
    , m_bTSF(MFALSE)
    , m_bTsfForceAwb(MFALSE)
{
    LSC_LOG("Enter LscMgr\n");
    getNvramData();
    getTsfCfgTbl();
    m_i4OtpState = importEEPromData();

    if (m_i4OtpState == E_LSC_WITH_MTK_OTP)
    {
        // write unit gain table to NVRAM
        LSC_LOG("Write Unit Gain to NVRAM buffer");
        const CAM_CAL_LSC_MTK_TYPE& rMtkLsc = m_rOtp.LscTable.MtkLcsData;
        MUINT32 u4XNum  = ((rMtkLsc.CapIspReg[1] >> 28) & 0x0000000F);
        MUINT32 u4YNum  = ((rMtkLsc.CapIspReg[1] >> 12) & 0x0000000F);
        MUINT32 u4GainTblSize = (u4XNum + 2)*(u4YNum + 2)*4*2;  // in byte (x*y*4ch*2byte)
        m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] = rMtkLsc.CapIspReg[1];
        m_prShadingLut->SensorGoldenCalTable.IspLSCReg[3] = rMtkLsc.CapIspReg[3];
        m_prShadingLut->SensorGoldenCalTable.IspLSCReg[4] = rMtkLsc.CapIspReg[4];
        m_prShadingLut->SensorGoldenCalTable.TblSize = u4GainTblSize;
        ::memcpy((void*)m_prShadingLut->SensorGoldenCalTable.UnitGainTable,
                (void*)rMtkLsc.CapTable, u4GainTblSize);
    }
}

LscMgr2::
~LscMgr2()
{
    LSC_LOG("Exit LscMgr");
}

MBOOL
LscMgr2::
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
    setTsfOnOff(m_rTsfCfgTbl.TSF_CFG.isTsfEn ? MTRUE : MFALSE);

    createThread();

    LSC_LOG_END();

    return MTRUE;
}

MBOOL
LscMgr2::
uninit()
{
    LSC_LOG_BEGIN("SensorDev(%d)", m_eSensorDev);

    //m_u4CTIdx = 0;

    destroyThread();

    RawLscTblUnInit();

    IspDebug::getInstance().uninit();

    return MTRUE;
}

MBOOL
LscMgr2::
loadTableFlow(MBOOL fgForceRestore)
{
    MINT32 i4Opt123 = 0;

    GET_PROP("debug.lsc_mgr.opt123", "0", i4Opt123);

    LSC_LOG_BEGIN("SensorDev(%d), byp123(%d), i4Opt123(%d)", m_eSensorDev, m_fgBypass1to3, i4Opt123);

    if (i4Opt123 & 0x1 || fgForceRestore || m_fgBypass1to3)
    {
        restoreNvram();
    }

    loadLutToSysram();
    RawLscTblDump("lscOrg");
    MBOOL fgBypass123 = (i4Opt123 & 0x2) ? MTRUE : MFALSE;
    if (!check123InNvram())
    {
        if (fgBypass123 || m_fgBypass1to3)
        {
            LSC_LOG("LSC 1-to-3 is bypassed for debug purpose");
            //restoreNvram();
            //loadLutToSysram();
            m_fg1to3 = E_LSC_123_USE_CCT;
        }
        else
        {
            MINT32 i4Otp = m_i4OtpState;
            switch (i4Otp)
            {
            case E_LSC_NO_OTP:
                if (0 /*isLscTransformFromCapture(m_eSensorDev)*/)
                {
                    LSC_LOG("No OTP: Do 1-to-3 from capture.");
                    // new path: only calibration in capture mode, and transform to the others.
                    //restoreNvram();
                    //loadLutToSysram();
                    if (doShadingTrfm())
                    {
                        copyRawLscToNvram(CAL_DATA_LOAD+1);
                        m_fg1to3 = E_LSC_123_NO_OTP_OK;
                    }
                    else
                    {
                        // load original table again.
                        loadLutToSysram();
                        m_fg1to3 = E_LSC_123_NO_OTP_ERR;
                    }
                }
                else
                {
                    LSC_LOG("No OTP: Use CCT calibrated table");
                    m_fg1to3 = E_LSC_123_USE_CCT;
                }

                break;
            case E_LSC_WITH_MTK_OTP:
                LSC_LOG("MTK OTP: Do 1-to-3 align + transform");
                //restoreNvram();
                //loadLutToSysram();
                if (doShadingAlign((MVOID*) &m_rOtp.LscTable.MtkLcsData))
                {
                    if (doShadingTrfm())
                    {
                        copyRawLscToNvram(CAL_DATA_LOAD);
                        m_fg1to3 = E_LSC_123_WITH_MTK_OTP_OK;
                    }
                    else
                    {
                        // load original table again.
                        loadLutToSysram();
                        m_fg1to3 = E_LSC_123_WITH_MTK_OTP_ERR2;
                    }
                }
                else
                {
                    // load original table again.
                    loadLutToSysram();
                    m_fg1to3 = E_LSC_123_WITH_MTK_OTP_ERR1;
                }

                break;
            default:
            case E_LSC_OTP_ERROR:
                m_fg1to3 = E_LSC_123_OTP_ERR;
                break;
            }
        }
    }
    else
    {
        if (m_rIspNvram.ISPComm.CommReg[CAL_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD)
        {
            m_fg1to3 = E_LSC_123_WITH_MTK_OTP_OK;
        }
        else
        {
            m_fg1to3 = E_LSC_123_NO_OTP_OK;
        }
        LSC_LOG("123 Tables already exist in NVRAM (0x%08x)", m_fg1to3);
    }
    RawLscTblDump("lsc123");

    LSC_LOG_END();

    return MTRUE;
}

MVOID
LscMgr2::
getNvramData(void)
{
    LSC_LOG_BEGIN("m_eSensorDev(0x%02x)\n", (MINT32)m_eSensorDev);
    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;
    NVRAM_CAMERA_3A_STRUCT *pNvram3A = NULL;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev, (void*&)pNvram_Shading);
    if(err!=0)
    {
        LSC_ERR("Fail to init NvramDrvMgr, use default LSC table");
            goto lbExit;
        }

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)pNvram3A);
    if(err!=0)
    {
        LSC_ERR("read 3a fail");
        goto lbExit;
    }

    m_prShadingLut = &pNvram_Shading->Shading;
    LSC_LOG("m_prShadingLut(%p)", m_prShadingLut);

    m_prNvram3A = pNvram3A;
    LSC_LOG("m_prNvram3A(%p)", m_prNvram3A);

lbExit:
    //NvramDrvMgr::getInstance().uninit();
    LSC_LOG_END();
}

MVOID
LscMgr2::
getTsfCfgTbl(void)
{
    LSC_LOG_BEGIN("m_eSensorDev(0x%02x)", (MINT32)m_eSensorDev);

    CAMERA_TSF_TBL_STRUCT*      pDftTsf     = NULL;
    MBOOL           fgLoadOK                = MFALSE;
    MINT32          i4TsfEn                 = isEnableTSF(m_eSensorDev);
    MINT32          i4TsfCtIdx              = getTSFD65Idx();
    const MINT32*   pi4TsfAwbForceSetting   = getTSFAWBForceInput();
    MUINT32*        pu4TsfData              = (MUINT32*)getTSFTrainingData();
    MINT32*         pi4TsfPara              = (MINT32*)getTSFTuningData();

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_TSF_TABLE, m_eSensorDev, (void*&)pDftTsf);
    if(err!=0)
    {
        LSC_ERR("Fail to read tsf data");
    }
    else
    {
        fgLoadOK = MTRUE;
    }

    if (fgLoadOK)
    {
        ::memcpy(&m_rTsfCfgTbl, pDftTsf, sizeof(CAMERA_TSF_TBL_STRUCT));
        LSC_LOG("Load TSF table OK, TSF(%d), CtIdx(%d), data(%p), para(%p)",
            m_rTsfCfgTbl.TSF_CFG.isTsfEn, m_rTsfCfgTbl.TSF_CFG.tsfCtIdx,
            m_rTsfCfgTbl.TSF_DATA,
            m_rTsfCfgTbl.TSF_PARA);
    }
    else
    {
        m_rTsfCfgTbl.TSF_CFG.isTsfEn = i4TsfEn;
        m_rTsfCfgTbl.TSF_CFG.tsfCtIdx = i4TsfCtIdx;
        ::memcpy(m_rTsfCfgTbl.TSF_CFG.rAWBInput, pi4TsfAwbForceSetting, sizeof(MINT32)*8);
        ::memcpy(m_rTsfCfgTbl.TSF_PARA, pi4TsfPara, sizeof(MINT32)*1620);
        ::memcpy(m_rTsfCfgTbl.TSF_DATA, pu4TsfData, sizeof(MUINT32)*16000);
        LSC_ERR("Load TSF table Fail, use default: TSF(%d), CtIdx(%d), data(%p), para(%p)",
            m_rTsfCfgTbl.TSF_CFG.isTsfEn, m_rTsfCfgTbl.TSF_CFG.tsfCtIdx,
            m_rTsfCfgTbl.TSF_DATA,
            m_rTsfCfgTbl.TSF_PARA);
    }

    LSC_LOG_END();
}

MBOOL
LscMgr2::restoreNvram(void)
{
    LSC_LOG_BEGIN("m_eSensorDev(0x%02x)", (MINT32)m_eSensorDev);
    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;

    MBOOL fgRet;
    //MINT err = NvBufUtil::getInstance().readDefault(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev);
    MINT err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev, (void*&)pNvram_Shading, MTRUE);

    if (err != 0)
    {
        LSC_ERR("Error to read default");
        fgRet = MFALSE;
    }
    else
    {
        LSC_LOG("Read deafault OK, restore shading NVRAM, m_prShadingLut(%p)",
            m_prShadingLut);
        m_rIspNvram.ISPComm.CommReg[CAL_INFO_IN_COMM_LOAD] = 0;
        m_prShadingLut = &pNvram_Shading->Shading;
        LSC_LOG("m_prShadingLut(%p)", m_prShadingLut);
        fgRet = MTRUE;
    }

    LSC_LOG_END();

    return fgRet;
}

MINT32
LscMgr2::
saveToNVRAM(void)
{
    MINT32 ret = 0;

    LSC_LOG_BEGIN();

    if (0 != NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev))
    {
        LSC_ERR("Fail to write shading table to NVRAM");
        ret = 1;
    }

    if (0 != NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_ISP, m_eSensorDev))
    {
        LSC_ERR("Fail to write ISP to NVRAM");
        ret |= 2;
    }

    LSC_LOG_END("Writing to NVRAM, ret(0x%08x)", ret);

    return ret;
}

MBOOL
LscMgr2::readNvramTbl(MBOOL fgForce)
{
    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev, (void*&)pNvram_Shading, fgForce);
    if(err!=0)
    {
        LSC_ERR("Fail to init NvramDrvMgr, use default LSC table");
        return MFALSE;
    }

    ISP_SHADING_STRUCT* prShadingLut = &pNvram_Shading->Shading;
    if (m_prShadingLut != prShadingLut)
    {
        LSC_LOG("m_prShadingLut(%p) <= prShadingLut(%p)", m_prShadingLut, prShadingLut);
        m_prShadingLut = prShadingLut;
    }

    LSC_LOG("OK");
    return MTRUE;
}

MBOOL
LscMgr2::writeNvramTbl(void)
{
    if (0 != NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev))
    {
        LSC_ERR("Fail to write shading table to NVRAM");
        return MFALSE;
    }
    return MTRUE;
}

MUINT32
LscMgr2::
getPerLutSize(ELscScenario_T eLscScn) const
{
#if USING_BUILTIN_LSC
    if (eLscScn == LSC_SCENARIO_CAP) {
        LSC_LOG("USING_BUILTIN_LSC size %d", sizeof(def_coef_cap));
        return sizeof(def_coef_cap);
    } else {
        LSC_LOG("USING_BUILTIN_LSC size %d", sizeof(def_coef));
        return sizeof(def_coef);
    }
#else

    if (eLscScn < SHADING_SUPPORT_OP_NUM)
    {
        return _u4MemBufSize[eLscScn]/SHADING_SUPPORT_CT_NUM;
    }
    else
    {
        LSC_ERR("Wrong eLscScn(%d)\n", eLscScn);
        return 0;
    }

#endif
}

MUINT32
LscMgr2::
getTotalLutSize(ELscScenario_T eLscScn) const
{
#if USING_BUILTIN_LSC
    if (eLscScn == LSC_SCENARIO_CAP) {
        LSC_LOG("USING_BUILTIN_LSC size %d", sizeof(def_coef_cap));
        return sizeof(def_coef_cap);
    } else {
        LSC_LOG("USING_BUILTIN_LSC size %d", sizeof(def_coef));
        return sizeof(def_coef);
    }
#else

    if (eLscScn < SHADING_SUPPORT_OP_NUM)
    {
        return _u4MemBufSize[eLscScn];
    }
    else
    {
        LSC_ERR("Wrong eLscScn(%d)\n", eLscScn);
        return 0;
    }

#endif
}


MUINT32*
LscMgr2::
getLut(ELscScenario_T eLscScn) const
{
#if USING_BUILTIN_LSC
    if (eLscScn == LSC_SCENARIO_CAP)
        return def_coef_cap;
    else
        return def_coef;
#else
    switch (eLscScn)
    {
    case LSC_SCENARIO_PRV:
        return &m_prShadingLut->PrvTable[0][0];
    case LSC_SCENARIO_CAP:
        return &m_prShadingLut->CapTable[0][0];
    case LSC_SCENARIO_VDO:
        return &m_prShadingLut->VdoTable[0][0];
    case LSC_SCENARIO_SLIM1:
        return &m_prShadingLut->Sv1Table[0][0];
    case LSC_SCENARIO_SLIM2:
        return &m_prShadingLut->Sv2Table[0][0];
    case LSC_SCENARIO_CUST1:
        return &m_prShadingLut->Cs1Table[0][0];
    case LSC_SCENARIO_CUST2:
        return &m_prShadingLut->Cs2Table[0][0];
    case LSC_SCENARIO_CUST3:
        return &m_prShadingLut->Cs3Table[0][0];
    case LSC_SCENARIO_CUST4:
        return &m_prShadingLut->Cs4Table[0][0];
    case LSC_SCENARIO_CUST5:
        return &m_prShadingLut->Cs5Table[0][0];
    default:
        LSC_ERR("Wrong eLscScn(%d)\n", eLscScn);
        break;
    }
    return NULL;
#endif
}

MBOOL
LscMgr2::
getSensorResolution()
{
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    SensorStaticInfo rSensorStaticInfo;
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("lsc_mgr", m_i4SensorIdx);
    pIHalSensorList->querySensorStaticInfo(m_eSensorDev, &rSensorStaticInfo);

    MINT32 u4NumSensorModes = MIN(rSensorStaticInfo.SensorModeNum, (MINT32)ESensorMode_NUM);

    m_rSensorResolution.u4SensorPreviewWidth  = rSensorStaticInfo.previewWidth;
    m_rSensorResolution.u4SensorPreviewHeight = rSensorStaticInfo.previewHeight;
    m_rSensorResolution.u4SensorCaptureWidth  = rSensorStaticInfo.captureWidth;
    m_rSensorResolution.u4SensorCaptureHeight = rSensorStaticInfo.captureHeight;
    m_rSensorResolution.u4SensorVideoWidth    = rSensorStaticInfo.videoWidth;
    m_rSensorResolution.u4SensorVideoHeight   = rSensorStaticInfo.videoHeight;
    m_rSensorResolution.u4SensorVideo1Width   = rSensorStaticInfo.video1Width;
    m_rSensorResolution.u4SensorVideo1Height  = rSensorStaticInfo.video1Height;
    m_rSensorResolution.u4SensorVideo2Width   = rSensorStaticInfo.video2Width;
    m_rSensorResolution.u4SensorVideo2Height  = rSensorStaticInfo.video2Height;
    m_rSensorResolution.u4SensorCustom1Width  = rSensorStaticInfo.SensorCustom1Width;
    m_rSensorResolution.u4SensorCustom1Height = rSensorStaticInfo.SensorCustom1Height;
    m_rSensorResolution.u4SensorCustom2Width  = rSensorStaticInfo.SensorCustom2Width;
    m_rSensorResolution.u4SensorCustom2Height = rSensorStaticInfo.SensorCustom2Height;
    m_rSensorResolution.u4SensorCustom3Width  = rSensorStaticInfo.SensorCustom3Width;
    m_rSensorResolution.u4SensorCustom3Height = rSensorStaticInfo.SensorCustom3Height;
    m_rSensorResolution.u4SensorCustom4Width  = rSensorStaticInfo.SensorCustom4Width;
    m_rSensorResolution.u4SensorCustom4Height = rSensorStaticInfo.SensorCustom4Height;
    m_rSensorResolution.u4SensorCustom5Width  = rSensorStaticInfo.SensorCustom5Width;
    m_rSensorResolution.u4SensorCustom5Height = rSensorStaticInfo.SensorCustom5Height;

    LSC_LOG("u4NumSensorModes(%d)", u4NumSensorModes);
    LSC_LOG("i4SensorPreviewWidth (%d)", m_rSensorResolution.u4SensorPreviewWidth );
    LSC_LOG("i4SensorPreviewHeight(%d)", m_rSensorResolution.u4SensorPreviewHeight);
    LSC_LOG("i4SensorCaptureWidth (%d)", m_rSensorResolution.u4SensorCaptureWidth );
    LSC_LOG("i4SensorCaptureHeight(%d)", m_rSensorResolution.u4SensorCaptureHeight);
    LSC_LOG("i4SensorVideoWidth   (%d)", m_rSensorResolution.u4SensorVideoWidth   );
    LSC_LOG("i4SensorVideoHeight  (%d)", m_rSensorResolution.u4SensorVideoHeight  );
    LSC_LOG("i4SensorVideo1Width  (%d)", m_rSensorResolution.u4SensorVideo1Width  );
    LSC_LOG("i4SensorVideo1Height (%d)", m_rSensorResolution.u4SensorVideo1Height );
    LSC_LOG("i4SensorVideo2Width  (%d)", m_rSensorResolution.u4SensorVideo2Width  );
    LSC_LOG("i4SensorVideo2Height (%d)", m_rSensorResolution.u4SensorVideo2Height );
    LSC_LOG("i4SensorCustom1Width (%d)", m_rSensorResolution.u4SensorCustom1Width );
    LSC_LOG("i4SensorCustom1Height(%d)", m_rSensorResolution.u4SensorCustom1Height);
    LSC_LOG("i4SensorCustom2Width (%d)", m_rSensorResolution.u4SensorCustom2Width );
    LSC_LOG("i4SensorCustom2Height(%d)", m_rSensorResolution.u4SensorCustom2Height);
    LSC_LOG("i4SensorCustom3Width (%d)", m_rSensorResolution.u4SensorCustom3Width );
    LSC_LOG("i4SensorCustom3Height(%d)", m_rSensorResolution.u4SensorCustom3Height);
    LSC_LOG("i4SensorCustom4Width (%d)", m_rSensorResolution.u4SensorCustom4Width );
    LSC_LOG("i4SensorCustom4Height(%d)", m_rSensorResolution.u4SensorCustom4Height);
    LSC_LOG("i4SensorCustom5Width (%d)", m_rSensorResolution.u4SensorCustom5Width );
    LSC_LOG("i4SensorCustom5Height(%d)", m_rSensorResolution.u4SensorCustom5Height);

    MINT32 i;
    for (i = 0; i < u4NumSensorModes; i++)
    {
        MUINT32 scenario = i;
        SensorCropWinInfo rSensorCropInfo;

        ::memset(&rSensorCropInfo, 0, sizeof(SensorCropWinInfo));

        MINT32 err = pIHalSensor->sendCommand(m_eSensorDev,
                SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                (MUINTPTR)&scenario,
                (MUINTPTR)&rSensorCropInfo,
                0);

        if (err != 0 || rSensorCropInfo.full_w == 0)
        {
            LSC_LOG("SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO is not implemented well (%d), use predefined rule.", err);
            m_fgSensorCropInfoNull = MTRUE;
            break;
        }
        else
        {
            m_fgSensorCropInfoNull = MFALSE;
        }

        SensorCropInfo_T rCrop;
        rCrop.w0 = rSensorCropInfo.full_w;
        rCrop.h0 = rSensorCropInfo.full_h;
        rCrop.x1 = rSensorCropInfo.x0_offset;
        rCrop.y1 = rSensorCropInfo.y0_offset;
        rCrop.w1 = rSensorCropInfo.w0_size;
        rCrop.h1 = rSensorCropInfo.h0_size;
        rCrop.w1r= rSensorCropInfo.scale_w;
        rCrop.h1r= rSensorCropInfo.scale_h;
        rCrop.x2 = rSensorCropInfo.x1_offset + rSensorCropInfo.x2_tg_offset;
        rCrop.y2 = rSensorCropInfo.y1_offset + rSensorCropInfo.y2_tg_offset;
        rCrop.w2 = rSensorCropInfo.w2_tg_size;
        rCrop.h2 = rSensorCropInfo.h2_tg_size;
        rCrop.u4W= rSensorCropInfo.w2_tg_size;
        rCrop.u4H= rSensorCropInfo.h2_tg_size;
        LSC_LOG("SensorMode(%d), full_0(%d,%d) crop1(%d,%d,%d,%d) resize(%d,%d) crop2(%d,%d,%d,%d) final size(%d,%d)",
            scenario, rCrop.w0, rCrop.h0, rCrop.x1, rCrop.y1, rCrop.w1, rCrop.h1, rCrop.w1r, rCrop.h1r,
            rCrop.x2, rCrop.y2, rCrop.w2, rCrop.h2, rCrop.u4W, rCrop.u4H);
        m_rSensorCropWin[i] = rCrop;
    }

    pIHalSensor->destroyInstance("lsc_mgr");

    m_rSensorCropWin[ESensorMode_Preview].u4W    = rSensorStaticInfo.previewWidth;
    m_rSensorCropWin[ESensorMode_Preview].u4H    = rSensorStaticInfo.previewHeight;
    m_rSensorCropWin[ESensorMode_Capture].u4W    = rSensorStaticInfo.captureWidth;
    m_rSensorCropWin[ESensorMode_Capture].u4H    = rSensorStaticInfo.captureHeight;
    m_rSensorCropWin[ESensorMode_Video].u4W      = rSensorStaticInfo.videoWidth;
    m_rSensorCropWin[ESensorMode_Video].u4H      = rSensorStaticInfo.videoHeight;
    m_rSensorCropWin[ESensorMode_SlimVideo1].u4W = rSensorStaticInfo.video1Width;
    m_rSensorCropWin[ESensorMode_SlimVideo1].u4H = rSensorStaticInfo.video1Height;
    m_rSensorCropWin[ESensorMode_SlimVideo2].u4W = rSensorStaticInfo.video2Width;
    m_rSensorCropWin[ESensorMode_SlimVideo2].u4H = rSensorStaticInfo.video2Height;
    m_rSensorCropWin[ESensorMode_Custom1].u4W    = rSensorStaticInfo.SensorCustom1Width;
    m_rSensorCropWin[ESensorMode_Custom1].u4H    = rSensorStaticInfo.SensorCustom1Height;
    m_rSensorCropWin[ESensorMode_Custom2].u4W    = rSensorStaticInfo.SensorCustom2Width;
    m_rSensorCropWin[ESensorMode_Custom2].u4H    = rSensorStaticInfo.SensorCustom2Height;
    m_rSensorCropWin[ESensorMode_Custom3].u4W    = rSensorStaticInfo.SensorCustom3Width;
    m_rSensorCropWin[ESensorMode_Custom3].u4H    = rSensorStaticInfo.SensorCustom3Height;
    m_rSensorCropWin[ESensorMode_Custom4].u4W    = rSensorStaticInfo.SensorCustom4Width;
    m_rSensorCropWin[ESensorMode_Custom4].u4H    = rSensorStaticInfo.SensorCustom4Height;
    m_rSensorCropWin[ESensorMode_Custom5].u4W    = rSensorStaticInfo.SensorCustom5Width;
    m_rSensorCropWin[ESensorMode_Custom5].u4H    = rSensorStaticInfo.SensorCustom5Height;

    for (i = 0; i < u4NumSensorModes; i++)
    {
        convertSensorCrop(!m_fgSensorCropInfoNull, m_rSensorCropWin[ESensorMode_Capture], m_rSensorCropWin[i], m_rSensorCropCfg[i]);
    }

    m_u4NumSensorModes = u4NumSensorModes;

    return MTRUE;
}


MBOOL
LscMgr2::
convertSensorCrop(MBOOL fgWithSensorCropInfo, const SensorCropInfo_T& rFullInfo, const SensorCropInfo_T& rCropInfo, ShadingTblCropCfg_T& rCropCfg)
{
    if (!fgWithSensorCropInfo)
    {
        MFLOAT fScale = 1.0f;

        // fit horizontal
        if (rFullInfo.u4W < rFullInfo.u4H || rCropInfo.u4W < rCropInfo.u4H)
        {
            LSC_ERR("Impossible Case.");
        }

        LSC_LOG("No sensor crop info, use magic rule");
        fScale = (MFLOAT) rCropInfo.u4W / rFullInfo.u4W;
        rCropCfg.u4Width  = rFullInfo.u4W;
        rCropCfg.u4Height = rFullInfo.u4H;
        if (rCropInfo.u4W*10 > rFullInfo.u4W*7)
        {
            // crop case
            LSC_LOG("Crop case, scale(%3.3f)", fScale);
            rCropCfg.u4OutputWidth  = rFullInfo.u4W;
            rCropCfg.u4OutputHeight = rFullInfo.u4H;
            rCropCfg.rCrop.u4W      = rCropInfo.u4W;
            rCropCfg.rCrop.u4H      = rCropInfo.u4H;
            rCropCfg.rCrop.u4X      = (rCropCfg.u4OutputWidth >= rCropCfg.rCrop.u4W) ? ((rCropCfg.u4OutputWidth - rCropCfg.rCrop.u4W) / 2) : 0;
            rCropCfg.rCrop.u4Y      = (rCropCfg.u4OutputHeight >= rCropCfg.rCrop.u4H) ? ((rCropCfg.u4OutputHeight - rCropCfg.rCrop.u4H) / 2) : 0;
        }
        else
        {
            // resize case
            LSC_LOG("Resize case, scale(%3.3f)", fScale);
            rCropCfg.u4OutputWidth  = rCropInfo.u4W;
            rCropCfg.u4OutputHeight = rFullInfo.u4H*fScale;
            rCropCfg.rCrop.u4W      = rCropInfo.u4W;
            rCropCfg.rCrop.u4H      = rCropInfo.u4H;
            rCropCfg.rCrop.u4X      = 0;
            rCropCfg.rCrop.u4Y      = (rCropCfg.u4OutputHeight >= rCropCfg.rCrop.u4H) ? ((rCropCfg.u4OutputHeight - rCropCfg.rCrop.u4H) / 2) : 0;
        }
    }
    else
    {
        // calculate crop with respect to capture
        // rw = w1r/w1, rh = h1r/h1,
        // x2'= x1*r+x2
        // x2_cap'' = x2c' * rw_scn / rw_cap
        MFLOAT rwc = (MFLOAT) rFullInfo.w1r / rFullInfo.w1;
        MFLOAT rhc = (MFLOAT) rFullInfo.h1r / rFullInfo.h1;
        MFLOAT x2c_ = (MFLOAT) rFullInfo.x1*rwc + rFullInfo.x2;
        MFLOAT y2c_ = (MFLOAT) rFullInfo.y1*rhc + rFullInfo.y2;
        LSC_LOG("rwc(%3.3f), rhc(%3.3f), x2c_(%3.3f), y2c_(%3.3f)", rwc, rhc, x2c_, y2c_);

        MFLOAT rws = (MFLOAT) rCropInfo.w1r / rCropInfo.w1;
        MFLOAT rhs = (MFLOAT) rCropInfo.h1r / rCropInfo.h1;
        MFLOAT x2s_ = (MFLOAT) rCropInfo.x1*rws + rCropInfo.x2;
        MFLOAT y2s_ = (MFLOAT) rCropInfo.y1*rhs + rCropInfo.y2;
        LSC_LOG("rws(%3.3f), rhs(%3.3f), x2s_(%3.3f), y2s_(%3.3f)", rws, rhs, x2s_, y2s_);

        MFLOAT x2c__ = x2c_ * rws / rwc;
        MFLOAT y2c__ = y2c_ * rhs / rhc;

        MFLOAT x_sc = x2s_ - x2c__;
        MFLOAT y_sc = y2s_ - y2c__;
        MFLOAT out_w = (MFLOAT) rFullInfo.w2 * rws / rwc;
        MFLOAT out_h = (MFLOAT) rFullInfo.h2 * rhs / rhc;
        MFLOAT w2s = rCropInfo.w2;
        MFLOAT h2s = rCropInfo.h2;

        LSC_LOG("x2c__(%3.3f), y2c__(%3.3f), x_sc(%3.3f), y_sc(%3.3f), out_w(%3.3f), out_h(%3.3f)",
            x2c__, y2c__, x_sc, y_sc, out_w, out_h);

        if (x_sc < 0.0f)
        {
            LSC_LOG("Negative x_sc(%3.3f)", x_sc);
            x_sc = 0.0f;
        }

        if ((MFLOAT)x_sc + w2s > out_w)
        {
            LSC_LOG("Exceed right side, x_sc(%3.3f), w2s(%d), out_w(%3.3f)", x_sc, w2s, out_w);
            //w2s = out_w - x_sc;
            x_sc = 0.0f;
            w2s = out_w;
        }

        if (y_sc < 0.0f)
        {
            LSC_LOG("Negative y_sc(%3.3f)", y_sc);
            y_sc = 0.0f;
        }

        if ((MFLOAT)y_sc + h2s > out_h)
        {
            LSC_LOG("Exceed bottom side, y_sc(%3.3f), h2s(%d), out_h(%3.3f)", y_sc, h2s, out_h);
            //h2s = out_h - y_sc;
            y_sc = 0.0f;
            h2s = out_h;
        }

        rCropCfg.u4Width        = rFullInfo.w2;
        rCropCfg.u4Height       = rFullInfo.h2;
        rCropCfg.u4OutputWidth  = ROUND(out_w);
        rCropCfg.u4OutputHeight = ROUND(out_h);
        rCropCfg.rCrop.u4W      = w2s;
        rCropCfg.rCrop.u4H      = h2s;
        rCropCfg.rCrop.u4X      = x_sc;
        rCropCfg.rCrop.u4Y      = y_sc;
    }

    LSC_LOG("Crop CFG for lib ImgWH(%d,%d), outWH(%d,%d), Crop(%d,%d,%d,%d)",
        rCropCfg.u4Width, rCropCfg.u4Height, rCropCfg.u4OutputWidth, rCropCfg.u4OutputHeight,
        rCropCfg.rCrop.u4X, rCropCfg.rCrop.u4Y, rCropCfg.rCrop.u4W, rCropCfg.rCrop.u4H);

    return MTRUE;
}

MBOOL
LscMgr2::
showResolutionInfo()
{
    UINT32 i, u4NumSensorModes;
    u4NumSensorModes = m_u4NumSensorModes;

    LSC_LOG("u4NumSensorModes(%d)", u4NumSensorModes);
    LSC_LOG("i4SensorPreviewWidth (%d)", m_rSensorResolution.u4SensorPreviewWidth);
    LSC_LOG("i4SensorPreviewHeight(%d)", m_rSensorResolution.u4SensorPreviewHeight);
    LSC_LOG("i4SensorCaptureWidth (%d)", m_rSensorResolution.u4SensorCaptureWidth);
    LSC_LOG("i4SensorCaptureHeight(%d)", m_rSensorResolution.u4SensorCaptureHeight);
    LSC_LOG("i4SensorVideoWidth   (%d)", m_rSensorResolution.u4SensorVideoWidth);
    LSC_LOG("i4SensorVideoHeight  (%d)", m_rSensorResolution.u4SensorVideoHeight);
    LSC_LOG("i4SensorVideo1Width  (%d)", m_rSensorResolution.u4SensorVideo1Width);
    LSC_LOG("i4SensorVideo1Height (%d)", m_rSensorResolution.u4SensorVideo1Height);
    LSC_LOG("i4SensorVideo2Width  (%d)", m_rSensorResolution.u4SensorVideo2Width);
    LSC_LOG("i4SensorVideo2Height (%d)", m_rSensorResolution.u4SensorVideo2Height);
    LSC_LOG("i4SensorCustom1Width (%d)", m_rSensorResolution.u4SensorCustom1Width );
    LSC_LOG("i4SensorCustom1Height(%d)", m_rSensorResolution.u4SensorCustom1Height);
    LSC_LOG("i4SensorCustom2Width (%d)", m_rSensorResolution.u4SensorCustom2Width );
    LSC_LOG("i4SensorCustom2Height(%d)", m_rSensorResolution.u4SensorCustom2Height);
    LSC_LOG("i4SensorCustom3Width (%d)", m_rSensorResolution.u4SensorCustom3Width );
    LSC_LOG("i4SensorCustom3Height(%d)", m_rSensorResolution.u4SensorCustom3Height);
    LSC_LOG("i4SensorCustom4Width (%d)", m_rSensorResolution.u4SensorCustom4Width );
    LSC_LOG("i4SensorCustom4Height(%d)", m_rSensorResolution.u4SensorCustom4Height);
    LSC_LOG("i4SensorCustom5Width (%d)", m_rSensorResolution.u4SensorCustom5Width );
    LSC_LOG("i4SensorCustom5Height(%d)", m_rSensorResolution.u4SensorCustom5Height);

    if (!m_fgSensorCropInfoNull)
    {
        for (i = 0; i < u4NumSensorModes; i++)
        {
            const SensorCropInfo_T& rCrop = m_rSensorCropWin[i];
            LSC_LOG("SensorMode(%d), full_0(%d,%d) crop1(%d,%d,%d,%d) resize(%d,%d) crop2(%d,%d,%d,%d) final size(%d,%d)",
                i, rCrop.w0, rCrop.h0, rCrop.x1, rCrop.y1, rCrop.w1, rCrop.h1, rCrop.w1r, rCrop.h1r,
                rCrop.x2, rCrop.y2, rCrop.w2, rCrop.h2, rCrop.u4W, rCrop.u4H);
        }
    }

    for (i = 0; i < u4NumSensorModes; i++)
    {
        const ShadingTblCropCfg_T& rCropCfg = m_rSensorCropCfg[i];
        LSC_LOG("Crop CFG for lib ImgWH(%d,%d), outWH(%d,%d), Crop(%d,%d,%d,%d)",
            rCropCfg.u4Width, rCropCfg.u4Height, rCropCfg.u4OutputWidth, rCropCfg.u4OutputHeight,
            rCropCfg.rCrop.u4X, rCropCfg.rCrop.u4Y, rCropCfg.rCrop.u4W, rCropCfg.rCrop.u4H);
    }


    return MTRUE;
}

MBOOL
LscMgr2::
getResolution(ESensorMode_T eSensorMode, SensorCropInfo_T& rInfo)
{
    if (eSensorMode >= ESensorMode_NUM)
    {
        LSC_ERR("Sensor Mode(%d) doesn't exist!", eSensorMode);
        rInfo = m_rSensorCropWin[ESensorMode_Preview];
        return MFALSE;
    }

    rInfo = m_rSensorCropWin[eSensorMode];

    LSC_LOG("eSensorMode(%d), W(%d), H(%d)", eSensorMode, rInfo.u4W, rInfo.u4H);

    return MTRUE;
}

MBOOL
LscMgr2::
getResolution(ELscScenario_T eScn, SensorCropInfo_T& rInfo)
{
    if (eScn >= LSC_SCENARIO_NUM)
    {
        LSC_ERR("eScn(%d) >= %d", eScn, LSC_SCENARIO_NUM);
        eScn = LSC_SCENARIO_PRV;
    }

    rInfo = m_rSensorCropWin[eScn];
    LSC_LOG("eScn(%d), W(%d), H(%d)", eScn, rInfo.u4W, rInfo.u4H);

    return MTRUE;
}


LscMgr2::ELscScenario_T
LscMgr2::
getLscScenario(ESensorMode_T eSensorMode)
{
    if (eSensorMode >= ESensorMode_NUM)
    {
        LSC_ERR("eSensorMode(%d) >= %d", eSensorMode, ESensorMode_NUM);
        eSensorMode = ESensorMode_Preview;
    }

    return static_cast<ELscScenario_T>(eSensorMode);
}

static MVOID _LogShadingSpec(const char* str, const SHADING_TBL_SPEC& rSpec)
{
    LSC_LOG("%s-------------\n"
            "img_width  = %d\n"
            "img_height = %d\n"
            "offset_x   = %d\n"
            "offset_y   = %d\n"
            "crop_width = %d\n"
            "crop_height= %d\n"
            "bayer      = %d\n"
            "grid_x     = %d\n"
            "grid_y     = %d\n"
            "lwidth     = %d\n"
            "lheight    = %d\n"
            "ratio_idx  = %d\n"
            "grgb_same  = %d\n"
            "data_type  = %d\n"
            "table      = 0x%08x\n", str,
            rSpec.img_width, rSpec.img_height, rSpec.offset_x, rSpec.offset_y,
            rSpec.crop_width, rSpec.crop_height, rSpec.bayer,
            rSpec.grid_x, rSpec.grid_y, rSpec.lwidth, rSpec.lheight,
            rSpec.ratio_idx, rSpec.grgb_same, rSpec.data_type, rSpec.table);
}

MINT32
LscMgr2::
shadingTblAlign(const ShadingTblAlignCfg_T& rAlignCfg, MUINT32* pu4OutputCoefTbl)
{
    MINT32 i4Ret = 0;
    SHADIND_ALIGN_CONF rSdAlignCfg;
    MUINT8* gWorkinBuffer = new MUINT8[SHADIND_FUNC_WORKING_BUFFER_SIZE];

    if (!gWorkinBuffer)
    {
        LSC_ERR("Fail to allocate gWorkinBuffer");
        return -1;
    }
    else
    {
        LSC_LOG("gWorkinBuffer(%p)", gWorkinBuffer);
    }

    rSdAlignCfg.working_buff_addr   = (void*) gWorkinBuffer;
    rSdAlignCfg.working_buff_size   = SHADIND_FUNC_WORKING_BUFFER_SIZE;

    rSdAlignCfg.golden.img_width    = rAlignCfg.u4Width;
    rSdAlignCfg.golden.img_height   = rAlignCfg.u4Height;
    rSdAlignCfg.golden.offset_x     = 0;
    rSdAlignCfg.golden.offset_y     = 0;
    rSdAlignCfg.golden.crop_width   = rAlignCfg.u4Width;
    rSdAlignCfg.golden.crop_height  = rAlignCfg.u4Height;
    rSdAlignCfg.golden.bayer        = (BAYER_ID_T)rAlignCfg.u1GainBayer;
    rSdAlignCfg.golden.grid_x       = rAlignCfg.u1GainXNum;
    rSdAlignCfg.golden.grid_y       = rAlignCfg.u1GainYNum;
    rSdAlignCfg.golden.lwidth       = 0;
    rSdAlignCfg.golden.lheight      = 0;
    rSdAlignCfg.golden.ratio_idx    = 0;
    rSdAlignCfg.golden.grgb_same    = SHADING_GRGB_SAME_NO;
    rSdAlignCfg.golden.data_type    = SHADING_TYPE_GAIN;
    rSdAlignCfg.golden.table        = rAlignCfg.pu4GoldenGain;

    rSdAlignCfg.cali.img_width      = rAlignCfg.u4Width;
    rSdAlignCfg.cali.img_height     = rAlignCfg.u4Height;
    rSdAlignCfg.cali.offset_x       = 0;
    rSdAlignCfg.cali.offset_y       = 0;
    rSdAlignCfg.cali.crop_width     = rAlignCfg.u4Width;
    rSdAlignCfg.cali.crop_height    = rAlignCfg.u4Height;
    rSdAlignCfg.cali.bayer          = (BAYER_ID_T)rAlignCfg.u1GainBayer;
    rSdAlignCfg.cali.grid_x         = rAlignCfg.u1GainXNum;
    rSdAlignCfg.cali.grid_y         = rAlignCfg.u1GainYNum;
    rSdAlignCfg.cali.lwidth         = 0;
    rSdAlignCfg.cali.lheight        = 0;
    rSdAlignCfg.cali.ratio_idx      = 0;
    rSdAlignCfg.cali.grgb_same      = SHADING_GRGB_SAME_NO;
    rSdAlignCfg.cali.data_type      = SHADING_TYPE_GAIN;
    rSdAlignCfg.cali.table          = rAlignCfg.pu4UnitGain;

    rSdAlignCfg.input.img_width     = rAlignCfg.u4Width;
    rSdAlignCfg.input.img_height    = rAlignCfg.u4Height;
    rSdAlignCfg.input.offset_x      = 0;
    rSdAlignCfg.input.offset_y      = 0;
    rSdAlignCfg.input.crop_width    = rAlignCfg.u4Width;
    rSdAlignCfg.input.crop_height   = rAlignCfg.u4Height;
    rSdAlignCfg.input.bayer         = BAYER_B;
    rSdAlignCfg.input.grid_x        = rAlignCfg.u1CoefXNum;     // Golden Coef
    rSdAlignCfg.input.grid_y        = rAlignCfg.u1CoefYNum;     // Golden Coef
    rSdAlignCfg.input.lwidth        = 0;
    rSdAlignCfg.input.lheight       = 0;
    rSdAlignCfg.input.ratio_idx     = 0;
    rSdAlignCfg.input.grgb_same     = SHADING_GRGB_SAME_NO;
    rSdAlignCfg.input.data_type     = SHADING_TYPE_COEFF;       // coef
    rSdAlignCfg.input.table         = rAlignCfg.pu4CoefTbl;     // Golden Coef

    rSdAlignCfg.output.img_width    = rAlignCfg.u4Width;
    rSdAlignCfg.output.img_height   = rAlignCfg.u4Height;
    rSdAlignCfg.output.offset_x     = rAlignCfg.rCrop.u4X;      // crop
    rSdAlignCfg.output.offset_y     = rAlignCfg.rCrop.u4Y;      // crop
    rSdAlignCfg.output.crop_width   = rAlignCfg.rCrop.u4W;      // crop
    rSdAlignCfg.output.crop_height  = rAlignCfg.rCrop.u4H;      // crop
    rSdAlignCfg.output.bayer        = BAYER_B;
    rSdAlignCfg.output.grid_x       = rAlignCfg.u1CoefXNum;     // Golden Coef
    rSdAlignCfg.output.grid_y       = rAlignCfg.u1CoefYNum;     // Golden Coef
    rSdAlignCfg.output.lwidth       = 0;
    rSdAlignCfg.output.lheight      = 0;
    rSdAlignCfg.output.ratio_idx    = 0;
    rSdAlignCfg.output.grgb_same    = SHADING_GRGB_SAME_NO;
    rSdAlignCfg.output.data_type    = SHADING_TYPE_COEFF;       // coef
    rSdAlignCfg.output.table        = pu4OutputCoefTbl;         // Golden Coef

    //_LogShadingSpec("golden", rSdAlignCfg.golden);
    //_LogShadingSpec("cali",   rSdAlignCfg.cali);
    //_LogShadingSpec("input",  rSdAlignCfg.input);
    //_LogShadingSpec("output", rSdAlignCfg.output);

    LSC_RESULT result = S_LSC_CONVERT_OK;

    result = shading_align_golden(rSdAlignCfg);
    if (S_LSC_CONVERT_OK != result)
    {
        LSC_ERR("Align Error(%d)", result);
        i4Ret = -1;
    }
    else
    {
        LSC_LOG("Align done.");
    }

    delete [] gWorkinBuffer;
    return i4Ret;
}

MINT32
LscMgr2::
shadingTblCrop(const ShadingTblCropCfg_T& rCropCfg, const ShadingTblInput_T& rInput, MUINT32* pu4OutputCoefTbl)
{
    MINT32 i4Ret = 0;
    SHADIND_TRFM_CONF rTrfm;
    MUINT8* gWorkinBuffer = new MUINT8[SHADIND_FUNC_WORKING_BUFFER_SIZE];
    if (!gWorkinBuffer)
    {
        LSC_ERR("Fail to allocate gWorkinBuffer");
        return -1;
    }
    else
    {
        LSC_LOG("gWorkinBuffer(%p)", gWorkinBuffer);
    }

    rTrfm.working_buff_addr     = gWorkinBuffer;
    rTrfm.working_buff_size     = SHADIND_FUNC_WORKING_BUFFER_SIZE;
    rTrfm.afn                   = SHADING_AFN_R0D;

    rTrfm.input.img_width       = rCropCfg.u4Width;
    rTrfm.input.img_height      = rCropCfg.u4Height;
    rTrfm.input.offset_x        = 0;
    rTrfm.input.offset_y        = 0;
    rTrfm.input.crop_width      = rCropCfg.u4Width;
    rTrfm.input.crop_height     = rCropCfg.u4Height;
    rTrfm.input.bayer           = BAYER_B;
    rTrfm.input.grid_x          = rInput.u1CoefXNum;        // Input Coef
    rTrfm.input.grid_y          = rInput.u1CoefYNum;        // Input Coef
    rTrfm.input.lwidth          = 0;
    rTrfm.input.lheight         = 0;
    rTrfm.input.ratio_idx       = 0;
    rTrfm.input.grgb_same       = SHADING_GRGB_SAME_NO;
    rTrfm.input.data_type       = SHADING_TYPE_COEFF;       // coef
    rTrfm.input.table           = rInput.pu4CoefTbl;        // Input Coef

    rTrfm.output.img_width      = rCropCfg.u4OutputWidth;  // output width, resize from input width
    rTrfm.output.img_height     = rCropCfg.u4OutputHeight; // output height, resize from input height
    rTrfm.output.offset_x       = rCropCfg.rCrop.u4X;      // crop
    rTrfm.output.offset_y       = rCropCfg.rCrop.u4Y;      // crop
    rTrfm.output.crop_width     = rCropCfg.rCrop.u4W;      // crop
    rTrfm.output.crop_height    = rCropCfg.rCrop.u4H;      // crop
    rTrfm.output.bayer          = BAYER_B;
    rTrfm.output.grid_x         = 17;                       // output Coef (alwasy 16x16)
    rTrfm.output.grid_y         = 17;                       // output Coef
    rTrfm.output.lwidth         = 0;
    rTrfm.output.lheight        = 0;
    rTrfm.output.ratio_idx      = 0;
    rTrfm.output.grgb_same      = SHADING_GRGB_SAME_NO;
    rTrfm.output.data_type      = SHADING_TYPE_COEFF;       // coef
    rTrfm.output.table          = pu4OutputCoefTbl;         // Golden Coef

    //_LogShadingSpec("input",  rTrfm.input);
    //_LogShadingSpec("output", rTrfm.output);

    LSC_RESULT result = S_LSC_CONVERT_OK;

    result = shading_transform(rTrfm);
    if (S_LSC_CONVERT_OK != result)
    {
        LSC_ERR("Transform Error(%d)", result);
        i4Ret = -1;
    }
    else
    {
        LSC_LOG("Transform done.");
    }

    delete [] gWorkinBuffer;
    return i4Ret;
}

MBOOL
LscMgr2::
RawLscTblInit()
{
    LSC_LOG_BEGIN("m_eSensorDev(0x%02x)\n", (MINT32)m_eSensorDev);

    MBOOL ret = MFALSE;

    MUINT32 i = 0;

    ret = MTRUE;
    if (!m_pIMemDrv)
    {
        m_pIMemDrv = IMemDrv::createInstance();

        if (!m_pIMemDrv)
        {
            LSC_LOG("m_pIMemDrv create Fail.\n");
            ret = MFALSE;
        }
        else
        {
            LSC_LOG("m_pIMemDrv(%p) create OK\n", m_pIMemDrv);
            ret = m_pIMemDrv->init();
            if (ret == MTRUE)
            {
                LSC_LOG("m_pIMemDrv init OK!\n");

                for (i = 0; i < LSC_SCENARIO_NUM; i++)
                {
                    LSC_LOG("-------------stRawLscInfo[%d] ---------------", i);
                    if (!allocMemBuf(stRawLscInfo[i], _u4MemBufSize[i]))
                    {
                        LSC_LOG("allocMemBuf(%d) FAILED\n", i);
                    }
                    else
                    {
                        showMemBufInfo(stRawLscInfo[i]);
                    }
                }

                for (i = 0; i < 2; i++)
                {
                    LSC_LOG("-------------m_rBufInfo[%d] ---------------", i);
                    if (!allocMemBuf(m_rBufInfo[i], SIZE_DOUBLE_BUF))
                    {
                        LSC_LOG("allocMemBuf(%d) FAILED\n", i);
                    }
                    else
                    {
                        showMemBufInfo(m_rBufInfo[i]);
                    }
                }
            }
            else
            {
                LSC_LOG("m_pIMemDrv init Fail!\n");
            }
        }
    }
    else
    {
        LSC_LOG("m_pIMemDrv(%p) already exists.\n", m_pIMemDrv);
    }

    LSC_LOG_END();
    return ret;
}

MBOOL
LscMgr2::
RawLscTblUnInit()
{
    MUINT32 ret = 0;
    MUINT32 i = 0;

    LSC_LOG_BEGIN("m_eSensorDev(0x%02x)\n", (MINT32)m_eSensorDev);

    for (i = 0; i < LSC_SCENARIO_NUM; i++)
    {
        freeMemBuf(stRawLscInfo[i]);
    }

    for (i = 0; i < 2; i++)
    {
        freeMemBuf(m_rBufInfo[i]);
    }

    if (m_pIMemDrv)
    {
        m_pIMemDrv->uninit();
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

    LSC_LOG_END();
    return MTRUE;
}

MVOID
LscMgr2::
RawLscTblDump(const char* filename)
{
    char strfile[128];
    MINT32 Scenario, i4Dbg;
    FILE* fpdebug;

    LSC_LOG_BEGIN();

    GET_PROP("debug.lsc_mgr.dump123", "0", i4Dbg);

    if (!i4Dbg)
    {
        LSC_LOG_END("Not to dump (%s)", filename);
        return;
    }

    sprintf(strfile, "/sdcard/lsc1to3data/%s.log", filename);

    fpdebug = fopen(strfile, "w");

    if ( fpdebug == NULL )
    {
        LSC_ERR("Can't open :%s\n", filename);
        return;
    }

    for (Scenario = 0; Scenario < LSC_SCENARIO_NUM; Scenario++)
    {
        MUINT32 ct = 0;
        MUINT32* Addr = (MUINT32*) stRawLscInfo[Scenario].virtAddr;

        fprintf(fpdebug, "Scenario%d: {\n", Scenario);
        for (ct = 0; ct < 4; ct++)
        {
            MUINT32* AddrEnd = (MUINT32*) Addr + getPerLutSize((ELscScenario_T)Scenario)/4;

            fprintf(fpdebug, "    {\n");
            while (Addr < AddrEnd)
            {
                MUINT32 a, b, c, d;
                a = *Addr++;
                b = *Addr++;
                c = *Addr++;
                d = *Addr++;
                fprintf(fpdebug, "        0x%08x,0x%08x,0x%08x,0x%08x,\n", a, b, c, d);
            }
            fprintf(fpdebug, "    }, // ct%d\n", ct);
        }
        fprintf(fpdebug, "},\n");
    }
    fclose(fpdebug);

    LSC_LOG_END();
}


MBOOL
LscMgr2::
allocMemBuf(IMEM_BUF_INFO& rBufInfo, MUINT32 const u4Size)
{
    MBOOL ret = MFALSE;

    if (!rBufInfo.virtAddr)
    {
        rBufInfo.size = u4Size;
        if (0 == m_pIMemDrv->allocVirtBuf(&rBufInfo))
        {
            if (0 != m_pIMemDrv->mapPhyAddr(&rBufInfo))
            {
                LSC_ERR("mapPhyAddr error, virtAddr(%p), size(%d)\n", rBufInfo.virtAddr, rBufInfo.size);
                ret = MFALSE;
            }
            else
            {
                ret = MTRUE;
            }
        }
        else
        {
            LSC_ERR("allocVirtBuf error, size(%d)\n", rBufInfo.size);
            ret = MFALSE;
        }
    }
    else
    {
        ret = MTRUE;
        LSC_LOG("Already Exist! virtAddr(%p), size(%d)\n", rBufInfo.virtAddr, u4Size);
    }
    return ret;
}

MINT32
LscMgr2::
freeMemBuf(IMEM_BUF_INFO& rBufInfo)
{
    MINT32 ret = MTRUE;

    if (!m_pIMemDrv || rBufInfo.virtAddr == 0)
    {
        LSC_ERR("Null m_pIMemDrv driver \n");
        return MFALSE;
    }

    if (0 == m_pIMemDrv->unmapPhyAddr(&rBufInfo))
    {
        if (0 == m_pIMemDrv->freeVirtBuf(&rBufInfo))
        {
            LSC_LOG("freeVirtBuf OK, memID(%d), virtAddr(%p), phyAddr(%p)\n", rBufInfo.memID, rBufInfo.virtAddr, rBufInfo.phyAddr);
            rBufInfo.virtAddr = 0;
            ret = MTRUE;
        }
        else
        {
            LSC_LOG("freeVirtBuf Fail, memID(%d), virtAddr(%p), phyAddr(%p)\n", rBufInfo.memID, rBufInfo.virtAddr, rBufInfo.phyAddr);
            ret = MFALSE;
        }
    }
    else
    {
        LSC_ERR("memID(%d) unmapPhyAddr error\n", rBufInfo.memID);
        ret = MFALSE;
    }

    return ret;
}


MBOOL
LscMgr2::
showMemBufInfo(IMEM_BUF_INFO& rBufInfo) const
{
    LSC_LOG("memID(%d), virtAddr(%p), phyAddr(%p), size(%d)\n",
        rBufInfo.memID, rBufInfo.virtAddr, rBufInfo.phyAddr, rBufInfo.size);
    return MTRUE;
}

MVOID
LscMgr2::
copyLutToMemBuf(IMEM_BUF_INFO& rBufInfo, const MUINT8* pLut, MUINT32 u4Size)
{
    MUINT8* pDest = reinterpret_cast<MUINT8*>(rBufInfo.virtAddr);

    if (rBufInfo.virtAddr != 0 && pLut != 0)
    {
        LSC_LOG("OK: virtAddr(%p) <- pLut(%p), size(%d)\n", pDest, pLut, u4Size);
        ::memcpy(pDest, pLut, u4Size);
        m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &rBufInfo);
    }
    else
    {
        LSC_ERR("Fail: virtAddr(%p) <- pLut(%p), size(%d)\n", pDest, pLut, u4Size);
    }
}

MBOOL
LscMgr2::
syncLut(ELscScenario_T eLscScn)
{
    MUINT32 u4DstSize = stRawLscInfo[eLscScn].size;
    MUINT32 u4SrcSize = getTotalLutSize(eLscScn);

    if (u4DstSize < u4SrcSize)
    {
        LSC_ERR("stRawLscInfo[%d].size(%d), LutSize(%d), Overflow!!",
                eLscScn, u4DstSize, u4SrcSize);
        return MFALSE;
    }
    else
    {
        LSC_LOG("eLscScn(%d)", eLscScn);
        copyLutToMemBuf(stRawLscInfo[eLscScn], reinterpret_cast<MUINT8*>(getLut(eLscScn)), u4SrcSize);
    }

    return MTRUE;
}

MVOID
LscMgr2::
loadLutToSysram()     //  VA <- LUT
{
    MUINT32 i;

    LSC_LOG_BEGIN();

    for (i = 0; i < LSC_SCENARIO_NUM; i++)
    {
        ELscScenario_T eLscScn = static_cast<ELscScenario_T>(i);
        syncLut(eLscScn);
    }

    // for double buffer
    MUINT32 u4Size = getPerLutSize(LSC_SCENARIO_PRV);
    MUINT32 u4Ofst = u4Size * getCTIdx();
    MUINT8* pLut = reinterpret_cast<MUINT8*>(getLut(LSC_SCENARIO_PRV))+u4Ofst;
    copyLutToMemBuf(m_rBufInfo[0], pLut, u4Size);
    copyLutToMemBuf(m_rBufInfo[1], pLut, u4Size);

    LSC_LOG_END();
}

MBOOL
LscMgr2::
setSensorShading(MVOID* pSensorLsc)
{
#if 1 // FIXME
    return MFALSE;
#else
    MBOOL fgRet = MTRUE;
    CAM_CAL_LSC_SENSOR_TYPE* _pSensorLsc = reinterpret_cast<CAM_CAL_LSC_SENSOR_TYPE*>(pSensorLsc);

    SET_SENSOR_CALIBRATION_DATA_STRUCT rSensorCaliData;

    LSC_LOG_BEGIN("Sensor LSC");

    rSensorCaliData.DataFormat = 0x00010001;
    rSensorCaliData.DataSize = _pSensorLsc->TableSize;

    if (MAX_SHADING_DATA_TBL >= rSensorCaliData.DataSize)
    {
        ::memcpy(&rSensorCaliData.ShadingData, &_pSensorLsc->SensorTable, rSensorCaliData.DataSize);
    }
    else
    {
        LSC_ERR("MAX_SHADING_DATA_TBL(%d), rSensorCaliData.DataSize(%d)",
                MAX_SHADING_DATA_TBL, rSensorCaliData.DataSize);
        ::memcpy(&rSensorCaliData.ShadingData, &_pSensorLsc->SensorTable, MAX_SHADING_DATA_TBL);
    }

    if (!m_pSensorHal)
    {
        LSC_ERR("m_pSensorHal is NULL");
        fgRet = MFALSE;
    }
    else
    {
        m_pSensorHal->sendCommand((halSensorDev_s)m_eSensorDev, (int)SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA,
                (int)(&rSensorCaliData), 0, 0);
    }

    LSC_LOG_END();

    return fgRet;
#endif
}

static MVOID _LogGainTbl(const MUINT8* ptrTbl, MUINT32 u4Size, const char* str)
{
    LSC_LOG("%s", str);
    const MUINT32* pu4Tbl = reinterpret_cast<const MUINT32*>(ptrTbl);
    MUINT32 u32Size = u4Size / 4;
    for (MUINT32 idx = 0; idx < u32Size; idx += 4)
    {
        MY_LOG("0x%08x    0x%08x    0x%08x    0x%08x\n",
            *(pu4Tbl+idx), *(pu4Tbl+idx+1), *(pu4Tbl+idx+2), *(pu4Tbl+idx+3));
    }

    for (MUINT32 idx = 0; idx < u4Size; idx += 16)
    {
        MY_LOG("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
            *(ptrTbl+idx), *(ptrTbl+idx+1), *(ptrTbl+idx+2), *(ptrTbl+idx+3),
            *(ptrTbl+idx+4), *(ptrTbl+idx+5), *(ptrTbl+idx+6), *(ptrTbl+idx+7),
            *(ptrTbl+idx+8), *(ptrTbl+idx+9), *(ptrTbl+idx+10), *(ptrTbl+idx+11),
            *(ptrTbl+idx+12), *(ptrTbl+idx+13), *(ptrTbl+idx+14), *(ptrTbl+idx+15));
    }
}

MBOOL
LscMgr2::
check123InNvram()
{
    MBOOL fgRet =
        (m_rIspNvram.ISPComm.CommReg[CAL_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD) ||
        (m_rIspNvram.ISPComm.CommReg[CAL_INFO_IN_COMM_LOAD] == (CAL_DATA_LOAD+1));
    return fgRet;
}

MBOOL
LscMgr2::
copyRawLscToNvram(MUINT32 u4Pattern)
{
    LSC_LOG_BEGIN("Pattern(0x%08x)", u4Pattern);

    // write pattern to NVRAM buffer to indicate data exists in NVRAM
    m_rIspNvram.ISPComm.CommReg[CAL_INFO_IN_COMM_LOAD] = u4Pattern;
    // write transformed table to NVRAM buffer
    ELscScenario_T eLscScn;
    for (MINT32 i4Scn = 0; i4Scn < LSC_SCENARIO_NUM; i4Scn++)
    {
        eLscScn = (ELscScenario_T) i4Scn;
        ::memcpy(getLut(eLscScn), reinterpret_cast<MVOID*>(stRawLscInfo[eLscScn].virtAddr), getTotalLutSize(eLscScn));
    }

    if (!isByp123ToNvram())
    {
        // flush buffer to NVRAM
        if (0 != saveToNVRAM())
        {
            LSC_ERR("Fail to write NVRAM");
            return MFALSE;
        }
    }
    LSC_LOG_END();
    return MTRUE;
}

MBOOL
LscMgr2::
doShadingAlign(MVOID* pMtkLsc)
{
    LSC_LOG_BEGIN("MTK 1-to-3 Unit/Golden Align");

    MBOOL fgRet = MTRUE;

    CAM_CAL_LSC_MTK_TYPE* _pMtkLsc = reinterpret_cast<CAM_CAL_LSC_MTK_TYPE*>(pMtkLsc);
    BAYER_ID_T eBayerGain = BAYER_B;
    ShadingTblAlignCfg_T rAlign;

    switch (_pMtkLsc->PixId)
    {
    case 1: eBayerGain = BAYER_B;    break;
    case 2: eBayerGain = BAYER_GB;   break;
    case 4: eBayerGain = BAYER_GR;   break;
    case 8: eBayerGain = BAYER_R;    break;
    }

    MUINT32 u4XNum, u4YNum, u4BlkW, u4BlkH, u4LastW, u4LastH;
    u4XNum              = ((_pMtkLsc->CapIspReg[1] >> 28) & 0x0000000F);
    u4YNum              = ((_pMtkLsc->CapIspReg[1] >> 12) & 0x0000000F);
    u4BlkW              = ((_pMtkLsc->CapIspReg[1] >> 16) & 0x00000FFF);
    u4BlkH              = ( _pMtkLsc->CapIspReg[1]        & 0x00000FFF);
    u4LastW             = ((_pMtkLsc->CapIspReg[3] >> 16) & 0x00000FFF);
    u4LastH             = ( _pMtkLsc->CapIspReg[3]        & 0x00000FFF);

    rAlign.u4Width      = u4BlkW*u4XNum + u4LastW;
    rAlign.u4Height     = u4BlkH*u4YNum + u4LastH;
    rAlign.u1GainBayer  = eBayerGain;
    rAlign.u1GainXNum   = u4XNum + 2;
    rAlign.u1GainYNum   = u4YNum + 2;
    rAlign.rCrop.u4X    = 0;
    rAlign.rCrop.u4Y    = 0;
    rAlign.rCrop.u4W    = rAlign.u4Width;
    rAlign.rCrop.u4H    = rAlign.u4Height;

    MUINT32 u4GainTblSize = (u4XNum + 2)*(u4YNum + 2)*4*2;  // in byte (x*y*4ch*2byte)

#if 0 // remove size checking due to tool issue
    if (u4GainTblSize != _pMtkLsc->CaptureTblSize)
    {
        fgRet = MFALSE;
    }
    else
#endif
    {
        ELscScenario_T eLscScn;
        // show golden/unit gain table
        MUINT8* pGoldenGain = m_prShadingLut->SensorGoldenCalTable.GainTable;
        MUINT8* pUnitGain = _pMtkLsc->CapTable;
        _LogGainTbl(pGoldenGain, u4GainTblSize, "Golden Gain Table");
        _LogGainTbl(pUnitGain, u4GainTblSize, "Unit Gain Table");

        MUINT8* pGoldenGainCp = new MUINT8[u4GainTblSize];
        MUINT8* pUnitGainCp = new MUINT8[u4GainTblSize];
        rAlign.pu4GoldenGain = (MUINT32*) pGoldenGainCp;
        rAlign.pu4UnitGain   = (MUINT32*) pUnitGainCp;

        LSC_LOG("Start Align, IMG(%d x %d), Grid(%d x %d), Bayer(%d), GoldenGain(0x%08x), UnitGain(0x%08x)",
            rAlign.u4Width, rAlign.u4Height, rAlign.u1GainXNum, rAlign.u1GainYNum, rAlign.u1GainBayer, pGoldenGainCp, pUnitGainCp);

        // just align for capture
        {
            eLscScn = LSC_SCENARIO_CAP;
            const ISP_NVRAM_LSC_T& rIspLscCfg = *getConfig(eLscScn);
            rAlign.u1CoefXNum = rIspLscCfg.ctl2.bits.SDBLK_XNUM + 2;
            rAlign.u1CoefYNum = rIspLscCfg.ctl3.bits.SDBLK_YNUM + 2;
            for (MUINT32 i = 0; i < SHADING_SUPPORT_CT_NUM; i++)
            {
                MUINT32 u4Size = getPerLutSize(eLscScn);
                MUINT8* pLut = new MUINT8[u4Size];

                ::memcpy(pGoldenGainCp, pGoldenGain, u4GainTblSize);
                ::memcpy(pUnitGainCp, pUnitGain, u4GainTblSize);
                // NVRAM as input
                MUINT8* pAddr = ((MUINT8*) getLut(eLscScn)) + i*u4Size;
                ::memcpy(pLut, pAddr, u4Size);
                rAlign.pu4CoefTbl = (MUINT32*)pLut;
                // IMEM as output
                MUINT8* pOutput = ((MUINT8*) stRawLscInfo[eLscScn].virtAddr) + i*u4Size;
                if (0 == shadingTblAlign(rAlign, (MUINT32*)pOutput))
                {
                    LSC_LOG("Align OK: eLscScn(%d), CT(%d), Grid(%d x %d), Input(%p), Output(%p)",
                        eLscScn, i, rAlign.u1CoefXNum, rAlign.u1CoefYNum, pLut, pOutput);
                }
                else
                {
                    LSC_ERR("Align NG: eLscScn(%d), CT(%d), Grid(%d x %d), Input(%p), Output(%p)",
                        eLscScn, i, rAlign.u1CoefXNum, rAlign.u1CoefYNum, pLut, pOutput);
                }

                delete [] pLut;
            }
            // flush to memory
            LSC_LOG("Flush to memory eLscScn(%d)", eLscScn);
            m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &stRawLscInfo[eLscScn]);
        }

        delete [] pUnitGainCp;
        delete [] pGoldenGainCp;
    }

    LSC_LOG_END();

    return fgRet;
}

MBOOL
LscMgr2::
doShadingTrfm()
{
    LSC_LOG_BEGIN("MTK 1-to-3 Crop Transform");

    ELscScenario_T eLscScn;
    for (MINT32 i4Scn = 0; i4Scn < (MINT32)m_u4NumSensorModes; i4Scn++)
    {
        eLscScn = (ELscScenario_T) i4Scn;
        ShadingTblInput_T rShadingInput;
        ShadingTblCropCfg_T rShadingCrop;
        SensorCropInfo_T rFullInfo, rCropInfo;
        MFLOAT fScale = 1.0f;

        if (eLscScn == LSC_SCENARIO_CAP)
            continue;

        getResolution(LSC_SCENARIO_CAP, rFullInfo);
        getResolution(eLscScn, rCropInfo);
        const ISP_NVRAM_LSC_T& rIspLscCfg = *getConfig(LSC_SCENARIO_CAP);
        rShadingInput.u1CoefXNum    = rIspLscCfg.ctl2.bits.SDBLK_XNUM + 2;
        rShadingInput.u1CoefYNum    = rIspLscCfg.ctl3.bits.SDBLK_YNUM + 2;

        rShadingCrop.u4Width        = m_rSensorCropCfg[eLscScn].u4Width;
        rShadingCrop.u4Height       = m_rSensorCropCfg[eLscScn].u4Height;
        rShadingCrop.u4OutputWidth  = m_rSensorCropCfg[eLscScn].u4OutputWidth;
        rShadingCrop.u4OutputHeight = m_rSensorCropCfg[eLscScn].u4OutputHeight;
        rShadingCrop.rCrop          = m_rSensorCropCfg[eLscScn].rCrop;

        LSC_LOG("Input (%d x %d) -> Output (%d x %d) -> X(%d), Y(%d), (%d x %d)",
            rShadingCrop.u4Width, rShadingCrop.u4Height, rShadingCrop.u4OutputWidth, rShadingCrop.u4OutputHeight,
            rShadingCrop.rCrop.u4X, rShadingCrop.rCrop.u4Y, rShadingCrop.rCrop.u4W, rShadingCrop.rCrop.u4H);

        for (MUINT32 i = 0; i < SHADING_SUPPORT_CT_NUM; i++)
        {
            // NVRAM as input
            MUINT8* pAddr = ((MUINT8*)stRawLscInfo[LSC_SCENARIO_CAP].virtAddr) + i*getPerLutSize(LSC_SCENARIO_CAP);
            // IMEM as output
            MUINT8* pOutput = ((MUINT8*)stRawLscInfo[eLscScn].virtAddr) + i*getPerLutSize(eLscScn);
            rShadingInput.pu4CoefTbl = (MUINT32*) pAddr;
            if (0 == shadingTblCrop(rShadingCrop, rShadingInput, (MUINT32*)pOutput))
            {
                LSC_LOG("Crop OK: eLscScn(%d), CT(%d), Grid(%dx%d), Input(%p) => (17x17), Output(%p)",
                    eLscScn, i, rShadingInput.u1CoefXNum, rShadingInput.u1CoefYNum, pAddr, pOutput);
            }
            else
            {
                LSC_ERR("Crop NG: eLscScn(%d), CT(%d), Grid(%dx%d), Input(%p) => (17x17), Output(%p)",
                    eLscScn, i, rShadingInput.u1CoefXNum, rShadingInput.u1CoefYNum, pAddr, pOutput);
                return MFALSE;
            }
        }

        LSC_LOG("Flush to memory eLscScn(%d)", eLscScn);
        m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &stRawLscInfo[eLscScn]);
    }

    LSC_LOG_END();

    return MTRUE;
}

MINT32
LscMgr2::
importEEPromData()
{
    MINT32 i4Ret = E_LSC_NO_OTP;
    MUINT32 i;
    LSC_LOG_BEGIN();

    MINT32 i4SensorDevID;

    switch (m_eSensorDev)
    {
    case ESensorDev_Main:
        i4SensorDevID = SENSOR_DEV_MAIN;
        break;
    case ESensorDev_Sub:
        i4SensorDevID = SENSOR_DEV_SUB;
        break;
    case ESensorDev_MainSecond:
        i4SensorDevID = SENSOR_DEV_MAIN_2;
        break;
    case ESensorDev_Main3D:
        i4SensorDevID = SENSOR_DEV_MAIN_3D;
        break;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        break;
    }

    CAMERA_CAM_CAL_TYPE_ENUM eCamCalDataType = CAMERA_CAM_CAL_DATA_SHADING_TABLE;
    CAM_CAL_DATA_STRUCT* pCalData = new CAM_CAL_DATA_STRUCT;

    if (pCalData == NULL)
    {
        LSC_ERR("Fail to allocate buffer!");
        return E_LSC_OTP_ERROR;
    }

#ifndef LSC_DBG
    CamCalDrvBase* pCamCalDrvObj = CamCalDrvBase::createInstance();
    if (!pCamCalDrvObj)
    {
        LSC_LOG("pCamCalDrvObj is NULL");
        delete pCalData;
        return E_LSC_NO_OTP;
    }

    MINT32 ret = pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, eCamCalDataType, pCalData);
#else
    MINT32 ret = 0;
    ::memcpy(pCalData, &_rDbgCamCalData, sizeof(CAM_CAL_DATA_STRUCT));
#endif

    LSC_LOG("ret(0x%08x)", ret);
    if (ret & CamCalReturnErr[eCamCalDataType])
    {
        LSC_LOG("Error(%s)", CamCalErrString[eCamCalDataType]);
        m_bIsEEPROMImported = MTRUE;
        delete pCalData;
        return E_LSC_NO_OTP;
    }
    else
    {
        LSC_LOG("Get OK");
    }

    MUINT32 u4Rot = 0;
    CAM_CAL_DATA_VER_ENUM eDataType  = pCalData->DataVer;
    CAM_CAL_LSC_DATA*     pLscData   = NULL;    // union struct

    LSC_LOG("eDataType(%d)", eDataType);
    switch (eDataType)
    {
    case CAM_CAL_SINGLE_EEPROM_DATA:
        LSC_LOG("CAM_CAL_SINGLE_EEPROM_DATA");
    case CAM_CAL_SINGLE_OTP_DATA:
        LSC_LOG("CAM_CAL_SINGLE_OTP_DATA");
        pLscData = &pCalData->SingleLsc.LscTable;
        u4Rot = pCalData->SingleLsc.TableRotation;
        break;
    case CAM_CAL_N3D_DATA:
        LSC_LOG("CAM_CAL_N3D_DATA");
        if (ESensorDev_Main == m_eSensorDev)
        {
            pLscData = &pCalData->N3DLsc.Data[0].LscTable;
            u4Rot = pCalData->N3DLsc.Data[0].TableRotation;
            LSC_LOG("CAM_CAL_N3D_DATA MAIN");
        }
        else
        {
            pLscData = &pCalData->N3DLsc.Data[1].LscTable;
            u4Rot = pCalData->N3DLsc.Data[1].TableRotation;
            LSC_LOG("CAM_CAL_N3D_DATA MAIN2");
        }
        break;
    default:
        LSC_ERR("Unknown eDataType(%d)", eDataType);
        m_bIsEEPROMImported = MTRUE;
        delete pCalData;
        return E_LSC_NO_OTP;
    }

    m_rOtp.TableRotation = u4Rot;
    ::memcpy(&m_rOtp.LscTable, pLscData, sizeof(CAM_CAL_LSC_DATA));
    LSC_LOG("u4Rot(%d), pLscData(%p)", u4Rot, pLscData);

    MUINT8 u1TblType = pLscData->MtkLcsData.MtkLscType;

    if (u1TblType & (1<<0))
    {
        // send table via sensor hal
        i4Ret = E_LSC_NO_OTP;
        //setSensorShading((MVOID*) &pLscData->SensorLcsData);
    }
    else if (u1TblType & (1<<1))
    {
        // do 1-to-3
        i4Ret = E_LSC_WITH_MTK_OTP;
        //m_fg1to3 = do123LutToSysram((MVOID*) &pLscData->MtkLcsData);
    }

    m_bIsEEPROMImported = MTRUE;

    delete pCalData;

    LSC_LOG_END();

    return i4Ret;
}


MINT32
LscMgr2::
setGainTable(MUINT32 u4GridNumX, MUINT32 u4GridNumY, MUINT32 u4Width, MUINT32 u4Height, float* pGainTbl)
{
#if 1   // FIXME
    MUINT32 u4Addr;
    MUINT32 u4Idx;
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_SET_TABLE) ? MTRUE : MFALSE;
    float* afWorkingBuf = new float[BUFFERSIZE];
    if (!afWorkingBuf)
    {
        LSC_ERR("Allocate afWorkingBuf Fail");
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

    MUINT32 u4RetLSCHwTbl =
        LscGaintoHWTbl(pGainTbl,
                       (MUINT32*)m_rBufInfo[u4Idx].virtAddr,
                       u4GridNumX,
                       u4GridNumY,
                       u4Width,
                       u4Height,
                       (void*)afWorkingBuf,
                       BUFFERSIZE);

    delete [] afWorkingBuf;

    // flush to DRAM
    //m_pIMemDrv->cacheFlushAll();
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &m_rBufInfo[u4Idx]);

    m_u4DoubleBufIdx = u4Idx;

    m_fgUserSetTbl = 1;

    MY_LOG_IF(fgLogEn, "[%s -]\n", __FUNCTION__);

    return u4RetLSCHwTbl;
#else
    return -1;
#endif
}

MINT32
LscMgr2::
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

    //u4Addr = IspDebug::getInstance().readLsciAddr(m_eSensorTG);
    u4Addr = m_rBufInfo[m_u4DoubleBufIdx].phyAddr;
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
LscMgr2::
calLscConfig(const SensorCropInfo_T& rCrop, MUINT8 u1XNum, MUINT8 u1YNum, ISP_NVRAM_LSC_T& rLscCfg)
{
    MUINT32 u4BlkW, u4BlkH, u4LastW, u4LastH;

    if (u1XNum >= 32 || u1XNum == 0 || u1YNum >= 32 || u1YNum == 0)
    {
        // assert
        LSC_ERR("u1XNum(%d), u1YNum(%d)", u1XNum, u1YNum);
        return MFALSE;
    }

    u4BlkW  = (rCrop.u4W)/(2*(u1XNum+1));
    u4BlkH  = (rCrop.u4H)/(2*(u1YNum+1));
    u4LastW = rCrop.u4W/2 - (u1XNum*u4BlkW);
    u4LastH = rCrop.u4H/2 - (u1YNum*u4BlkH);

    rLscCfg.ctl2.bits.SDBLK_XNUM = u1XNum;
    rLscCfg.ctl3.bits.SDBLK_YNUM = u1YNum;
    rLscCfg.ctl2.bits.SDBLK_WIDTH = u4BlkW;
    rLscCfg.ctl3.bits.SDBLK_HEIGHT = u4BlkH;
    rLscCfg.lblock.bits.SDBLK_lWIDTH = u4LastW;
    rLscCfg.lblock.bits.SDBLK_lHEIGHT = u4LastH;

    rLscCfg.ratio.val = 0x20202020;
//    rLscCfg.gain_th.val = 0x03F000000;

    rLscCfg.ctl1.bits.SD_ULTRA_MODE = 1;

    LSC_LOG("XNum(%d)/YNum(%d)/W(%d)/H(%d)/LW(%d)/LH(%d)/OffX(%d)/OffY(%d)",
            rLscCfg.ctl2.bits.SDBLK_XNUM,
            rLscCfg.ctl3.bits.SDBLK_YNUM,
            rLscCfg.ctl2.bits.SDBLK_WIDTH,
            rLscCfg.ctl3.bits.SDBLK_HEIGHT,
            rLscCfg.lblock.bits.SDBLK_lWIDTH,
            rLscCfg.lblock.bits.SDBLK_lHEIGHT,
            rLscCfg.ctl1.bits.SDBLK_XOFST,
            rLscCfg.ctl1.bits.SDBLK_YOFST);

    return MTRUE;
}

MBOOL
LscMgr2::
calSl2Config(const SensorCropInfo_T& rCrop, ISP_NVRAM_SL2_T& rSl2Cfg)
{
    // default setting
    MUINT32 u4Rx, u4Ry, u4RR;
    u4Rx = rCrop.u4W / 2;
    u4Ry = rCrop.u4H / 2;
    u4RR = u4Rx*u4Rx + u4Ry*u4Ry;

    rSl2Cfg.cen.bits.SL2_CENTER_X = u4Rx;
    rSl2Cfg.cen.bits.SL2_CENTER_Y = u4Ry;
    rSl2Cfg.max0_rr.val = u4RR*9/100;
    rSl2Cfg.max1_rr.val = u4RR*36/100;
    rSl2Cfg.max2_rr.val = u4RR*64/100;

    return MTRUE;
}


MBOOL
LscMgr2::
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
    }

    return MTRUE;
}

ESensorMode_T
LscMgr2::
getSensorMode() const
{
    return m_eSensorMode;
}

ESensorMode_T
LscMgr2::
getPrevSensorMode() const
{
    return m_ePrevSensorMode;
}

MBOOL
LscMgr2::
setConfig(ELscScenario_T eLscScn, const ISP_NVRAM_LSC_T& rLscCfg)
{
    if (eLscScn >= LSC_SCENARIO_NUM)
    {
        LSC_ERR("eLscScn(%d) > %d", eLscScn, LSC_SCENARIO_NUM);
        return MFALSE;
    }
    LSC_LOG("eLscScn(%d)", eLscScn);
    // update NVRAM
    m_rIspLscCfg[eLscScn] = rLscCfg;
    return MTRUE;
}

const ISP_NVRAM_LSC_T*
LscMgr2::getConfig(ELscScenario_T eLscScn) const
{
    if (eLscScn >= LSC_SCENARIO_NUM)
    {
        LSC_ERR("eLscScn(%d) > %d", eLscScn, LSC_SCENARIO_NUM);
        return NULL;
    }
        LSC_LOG("eLscScn(%d)", eLscScn);
    // get NVRAM
    return &m_rIspLscCfg[eLscScn];
}


MUINT32
LscMgr2::
getCTIdx()
{
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_GET_CT) ? MTRUE : MFALSE;
    if (SHADING_SUPPORT_CT_NUM <= m_u4CTIdx)
    {
        LSC_ERR("m_u4CTIdx(%d) exceeds", m_u4CTIdx);
    }

    MY_LOG_IF(fgLogEn, "[%s] CT(%d)\n", __FUNCTION__, m_u4CTIdx);
    return m_u4CTIdx;
}

MBOOL
LscMgr2::
setCTIdx(MUINT32 const u4CTIdx)
{
    MUINT32 _u4CtIdx;
    MINT32 dbg_ct = 0;
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_SET_CT) ? MTRUE : MFALSE;
    GET_PROP("debug.lsc_mgr.ct", "-1", dbg_ct);

    if (dbg_ct != -1)
    {
        LSC_LOG("DEBUG CT(%d)", dbg_ct);
        _u4CtIdx = dbg_ct;
    }
    else
    {
        _u4CtIdx = u4CTIdx;
    }

    if (SHADING_SUPPORT_CT_NUM <= _u4CtIdx)
    {
        m_fgCtIdxExcd = MTRUE;
        LSC_LOG("_u4CtIdx(%d) exceeds", _u4CtIdx);
        return MFALSE;
    }

    m_fgCtIdxExcd = MFALSE;
    m_u4CTIdx = _u4CtIdx;
    MY_LOG_IF(fgLogEn, "[%s] CT(%d)", __FUNCTION__, _u4CtIdx);

    return MTRUE;
}

MBOOL
LscMgr2::
getOnOff(void)
{
    MINT32 dbg_enable = 0;
    GET_PROP("debug.lsc_mgr.enable", "-1", dbg_enable);

    if (dbg_enable != -1)
    {
        // overwrite for debug
        m_fgOnOff = dbg_enable == 0 ? MFALSE : MTRUE;
    }

    return m_fgOnOff;
}

MVOID
LscMgr2::
setOnOff(MBOOL fgOnOff)
{
    m_fgOnOff = fgOnOff;
}

MBOOL
LscMgr2::
getTsfOnOff(void)
{
    MINT32 dbg_enable = 0;
    GET_PROP("debug.lsc_mgr.manual_tsf", "-1", dbg_enable);

    if (dbg_enable != -1)
    {
        // overwrite for debug
        m_bTSF = dbg_enable == 0 ? MFALSE : MTRUE;
        if (!m_bTSF)
        {
            m_fgUserSetTbl = MFALSE;
        }
    }

    return m_bTSF;
}

MVOID
LscMgr2::
setTsfOnOff(MBOOL fgOnOff)
{
    LSC_LOG(" (%d)", fgOnOff);
    m_bTSF = fgOnOff;
    if (!m_bTSF)
    {
        m_fgUserSetTbl = MFALSE;
    }
}

MRESULT
LscMgr2::
setTsfForceAwb(MBOOL fgForce)
{
    LSC_LOG("fgForce(%d)", __FUNCTION__, fgForce);
    m_bTsfForceAwb = fgForce;
    return MTRUE;
}
#if 0
MBOOL
LscMgr2::
setTgInfo(MINT32 const i4TGInfo)
{
    LSC_LOG("(%d)", i4TGInfo);
    m_eSensorTG = (ESensorTG_T) i4TGInfo;
    return MTRUE;
}
#endif

MVOID
LscMgr2::
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
    //if (NSIspTuning::IspTuningMgr::getInstance().getOperMode(m_eSensorDev) != NSIspTuning::EOperMode_Meta)
    {
        ISP_MGR_SL2_T::getInstance(m_eSensorDev).setEnable(fgSl2OnOff);
        ISP_MGR_SL2_T::getInstance(m_eSensorDev).put(rSl2Cfg);
    }
}

}

