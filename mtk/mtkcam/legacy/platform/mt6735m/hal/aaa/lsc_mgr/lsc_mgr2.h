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
#ifndef _LSC_MGR2_H_
#define _LSC_MGR2_H_

#include <utils/threads.h>
#include <utils/List.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/atomic.h>

#include <aaa_types.h>
#include <ispif.h>
#include <camera_custom_nvram.h>
#include <mtkcam/drv_common/imem_drv.h>
#include <isp_tuning.h>
#include <Local.h>
#include <string>
#include <mtkcam/v3/hal/aaa_hal_common.h>
#include <mtkcam/algorithm/libtsf/MTKTsf.h>

#include "camera_custom_cam_cal.h"

#define USING_BUILTIN_LSC   0
#define DEBUG_ALIGN_FUNC    0

#define TSF_BUILD

namespace NSIspTuningv3 {

using namespace NSIspTuning;

#define EN_LSC_LOG_UPDATE       (0x1 << 0)
#define EN_LSC_LOG_GET_CT       (0x1 << 1)
#define EN_LSC_LOG_SET_CT       (0x1 << 2)
#define EN_LSC_LOG_SET_TABLE    (0x1 << 3)
#define EN_LSC_LOG_THREAD       (0x1 << 4)
#define EN_LSC_LOG_TSF_SET_PROC (0x1 << 5)
#define EN_LSC_LOG_TSF_RUN      (0x1 << 6)
#define EN_LSC_LOG_TSF_BATCH    (0x1 << 7)
#define EN_LSC_LOG_TSF_DUMP     (0x1 << 8)
#define EN_LSC_LOG_TSF_REINIT   (0x1 << 9)

/*******************************************************************************
 * LSC Manager
 *******************************************************************************/
class LscMgr2
{
public:
    typedef struct
    {
        MUINT32 m_u4CCT;
        MINT32  m_i4LV;
        MINT32  m_RGAIN;
        MINT32  m_GGAIN;
        MINT32  m_BGAIN;
        MINT32  m_FLUO_IDX;
        MINT32  m_DAY_FLUO_IDX;
    } TSF_AWB_INFO;

    typedef struct
    {
        TSF_AWB_INFO rAwbInfo;
        MUINT32 u4SizeAwbStat;
        MUINT8* prAwbStat;
    } TSF_REF_INFO_T;

    typedef enum
    {
        LSC_SCENARIO_PRV   = 0,    //     ESensorMode_Preview,
        LSC_SCENARIO_CAP   = 1,    //     ESensorMode_Capture,
        LSC_SCENARIO_VDO   = 2,    //     ESensorMode_Video,
        LSC_SCENARIO_SLIM1 = 3,    //     ESensorMode_SlimVideo1,
        LSC_SCENARIO_SLIM2 = 4,    //     ESensorMode_SlimVideo2,
        LSC_SCENARIO_CUST1 = 5,    //     ESensorMode_Custom1,
        LSC_SCENARIO_CUST2 = 6,    //     ESensorMode_Custom2,
        LSC_SCENARIO_CUST3 = 7,    //     ESensorMode_Custom3,
        LSC_SCENARIO_CUST4 = 8,    //     ESensorMode_Custom4,
        LSC_SCENARIO_CUST5 = 9,    //     ESensorMode_Custom5,
        LSC_SCENARIO_NUM
    } ELscScenario_T;

    typedef enum
    {
        E_TSF_CMD_IDLE      = 0,
        E_TSF_CMD_INIT      = 1,
        E_TSF_CMD_RUN       = 2,
        E_TSF_CMD_BATCH     = 3,
        E_TSF_CMD_BATCH_CAP = 4,
        E_TSF_CMD_CHG       = 5,
    } E_TSF_CMD_T;

    static LscMgr2*         createInstance(ESensorDev_T const eSensorDev, MINT32 i4SensorIdx, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram);
    static LscMgr2*         getInstance(ESensorDev_T sensor);

    MBOOL                   destroyInstance();

    MBOOL                   init();
    MBOOL                   uninit();

    MBOOL                   setSensorMode(ESensorMode_T eSensorMode, MBOOL fgForce=MFALSE);
    ESensorMode_T           getSensorMode() const;
    ESensorMode_T           getPrevSensorMode() const;

    MUINT32                 getCTIdx();
    MBOOL                   setCTIdx(MUINT32 const u4CTIdx);
    MVOID                   setOnOff(MBOOL fgOnOff);
    MBOOL                   getOnOff();
    MBOOL                   setConfig(ELscScenario_T eLscScn, const ISP_NVRAM_LSC_T& rLscCfg);
    const ISP_NVRAM_LSC_T*  getConfig(ELscScenario_T eLscScn) const;
    MUINT32                 getRegIdx() const {return m_eLscScenario;}
    //MBOOL                   setSwNr();

    //MBOOL                   setTgInfo(MINT32 const i4TGInfo);
    ESensorDev_T            getSensorDev() const { return m_eSensorDev; }

    MVOID                   loadLutToSysram();              // NVRAM ALL to IMEM
    MBOOL                   syncLut(ELscScenario_T eLscScn);    // NVRAM eLscScn to IMEM
    MUINT32*                getLut(ELscScenario_T eLscScn) const;
    MUINT32                 getTotalLutSize(ELscScenario_T eLscScn) const;
    MUINT32                 getPerLutSize(ELscScenario_T eLscScn) const;

    MBOOL                   readNvramTbl(MBOOL fgForce);
    MBOOL                   writeNvramTbl(void);

    MINT32                  getGainTable(MUINT32 u4Bayer, MUINT32 u4GridNumX, MUINT32 u4GridNumY, MFLOAT* pGainTbl);
    MINT32                  setGainTable(MUINT32 u4GridNumX, MUINT32 u4GridNumY, MUINT32 u4Width, MUINT32 u4Height, float* pGainTbl);
    MINT32                  setCoefTable(const MUINT32* pCoefTbl);

    MRESULT                 getDebugInfo(DEBUG_RESERVEA_INFO_T &rShadingDbgInfo);
    //MRESULT                 getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl, DEBUG_SHAD_ARRAY_2_T& rShadRestTbl);

    MRESULT                 CCTOPSetSdblkFileCfg(MBOOL fgSave, const char* filename);
    MRESULT                 CCTOPSetBypass123(MBOOL fgBypass);
    MRESULT                 setTsfForceAwb(MBOOL fgForce);

    MVOID                   updateLsc();
    MVOID                   updateTsf(E_TSF_CMD_T eCmd, const TSF_AWB_INFO* prTsfAwb, const MVOID* prStat);
    MVOID                   setTsfOnOff(MBOOL fgOnOff);
    MBOOL                   getTsfOnOff();
protected:
    typedef struct
    {
        MUINT32 u4SensorPreviewWidth;
        MUINT32 u4SensorPreviewHeight;
        MUINT32 u4SensorCaptureWidth;
        MUINT32 u4SensorCaptureHeight;
        MUINT32 u4SensorVideoWidth;
        MUINT32 u4SensorVideoHeight;
        MUINT32 u4SensorVideo1Width;
        MUINT32 u4SensorVideo1Height;
        MUINT32 u4SensorVideo2Width;
        MUINT32 u4SensorVideo2Height;
        MUINT32 u4SensorCustom1Width;   // new for custom
        MUINT32 u4SensorCustom1Height;
        MUINT32 u4SensorCustom2Width;
        MUINT32 u4SensorCustom2Height;
        MUINT32 u4SensorCustom3Width;
        MUINT32 u4SensorCustom3Height;
        MUINT32 u4SensorCustom4Width;
        MUINT32 u4SensorCustom4Height;
        MUINT32 u4SensorCustom5Width;
        MUINT32 u4SensorCustom5Height;
    } SENSOR_RESOLUTION_INFO_T;

    struct SensorCropInfo_T
    {
        // TBD
        MUINT32  w0;    // original full width
        MUINT32  h0;    // original full height
        MUINT32  x1;    // crop_1 x offset from full_0
        MUINT32  y1;    // crop_1 y offset from full_0
        MUINT32  w1;    // crop_1 width from full_0
        MUINT32  h1;    // crop_1 height from full_0
        MUINT32  w1r;   // scaled width from crop_1, w1 * r
        MUINT32  h1r;   // scaled height from crop_1, h1 * r
        MUINT32  x2;    // crop_2 x offset from scaled crop_1
        MUINT32  y2;    // crop_2 y offset from scaled crop_1
        MUINT32  w2;    // crop_2 width from scaled crop_1
        MUINT32  h2;    // crop_2 height from scaled crop_1

        MUINT32  u4W;   // input size of LSC, w2*r2, r2 must be 1
        MUINT32  u4H;   // input size of LSC, h2*r2, r2 must be 1
    };

    struct CropCfg_T
    {
        MUINT32  u4X;
        MUINT32  u4Y;
        MUINT32  u4W;
        MUINT32  u4H;
    };

    struct ShadingTblAlignCfg_T
    {
        MUINT32  u4Width;
        MUINT32  u4Height;
        // golden, unit gain table
        MUINT8   u1GainBayer;
        MUINT8   u1GainXNum;
        MUINT8   u1GainYNum;
        // input coef table
        MUINT8   u1CoefXNum;
        MUINT8   u1CoefYNum;
        // tables
        MUINT32* pu4GoldenGain;
        MUINT32* pu4UnitGain;
        MUINT32* pu4CoefTbl;
        // output crop
        CropCfg_T rCrop;
    };

    struct ShadingTblInput_T
    {
        // input coef table
        MUINT8   u1CoefXNum;
        MUINT8   u1CoefYNum;
        // table
        MUINT32* pu4CoefTbl;
    };

    struct ShadingTblCropCfg_T
    {
        MUINT32  u4Width;
        MUINT32  u4Height;
        MUINT32  u4OutputWidth;
        MUINT32  u4OutputHeight;
        // input coef table
        MUINT8   u1CoefXNum;
        MUINT8   u1CoefYNum;
        // tables
        MUINT32* pu4CoefTbl;
        // output crop
        CropCfg_T rCrop;
    };

    LscMgr2(ESensorDev_T eSensorDev, MINT32 i4SensorIdx, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram);
    ~LscMgr2();

    enum
    {
        E_LSC_OTP_ERROR         = 0,
        E_LSC_NO_OTP            = 1,
        E_LSC_WITH_MTK_OTP      = 2
    };

    enum
    {
        E_LSC_123_OTP_ERR           = -1,   // use default table
        E_LSC_123_USE_CCT           = 0,    // use default table
        E_LSC_123_NO_OTP_OK         = 1,    // use transformed table
        E_LSC_123_NO_OTP_ERR        = 2,    // use default table
        E_LSC_123_WITH_MTK_OTP_OK   = 3,    // use transformed table
        E_LSC_123_WITH_MTK_OTP_ERR1 = 4,    // use default table
        E_LSC_123_WITH_MTK_OTP_ERR2 = 5,    // use default table
    };

    // NVRAM, eeprom -> 123 -> IMEM
    MBOOL                   setSensorShading(MVOID* pSensorLsc);
    MINT32                  importEEPromData();
    MBOOL                   check123InNvram();
    MBOOL                   loadTableFlow(MBOOL fgForceRestore);
    // read NVRAM
    MVOID                   getNvramData(void);
    MVOID                   getTsfCfgTbl(void);

    MBOOL                   saveToNVRAM(void);                            // flush to NVRAM
    MBOOL                   restoreNvram(void);                             // default to NVRAM buf
    MBOOL                   copyRawLscToNvram(MUINT32 u4Pattern);       // IMEM to NVRAM buf

    ELscScenario_T          getLscScenario(ESensorMode_T eSensorMode);

    MBOOL                   calLscConfig(const SensorCropInfo_T& rCrop, MUINT8 u1XNum, MUINT8 u1YNum, ISP_NVRAM_LSC_T& rLscCfg);
    MBOOL                   calSl2Config(const SensorCropInfo_T& rCrop, ISP_NVRAM_SL2_T& rSl2Cfg);

    MBOOL                   getSensorResolution();
    MBOOL                   getResolution(ESensorMode_T eSensorMode, SensorCropInfo_T& rInfo);
    MBOOL                   getResolution(ELscScenario_T eScn, SensorCropInfo_T& rInfo);
    MBOOL                   showResolutionInfo();

    MBOOL                   RawLscTblSetPhyVirAddr(MUINT32 const u8LscIdx, MVOID* pPhyAddr, MVOID* pVirAddr);
    MUINT32                 RawLscTblGetPhyAddr(MUINT32 const u8LscIdx);
    MUINT32                 RawLscTblGetVirAddr(MUINT32 const u8LscIdx);

    MBOOL                   RawLscTblInit();
    MBOOL                   RawLscTblUnInit();
    MVOID                   RawLscTblClear(ELscScenario_T Scenario, UINT8 ColorTemp);
    MVOID                   RawLscTblDump(const char* filename);
    MINT32                  dumpSdblk(const char* table_name, const ISP_NVRAM_LSC_T& LscConfig, const MUINT32 *ShadingTbl);

    MVOID                   copyLutToMemBuf(IMEM_BUF_INFO& rBufInfo, const MUINT8* pLut, MUINT32 u4Size);
    MBOOL                   allocMemBuf(IMEM_BUF_INFO& rBufInfo, MUINT32 const u4Size);
    MINT32                  freeMemBuf(IMEM_BUF_INFO& rBufInfo);
    MBOOL                   showMemBufInfo(IMEM_BUF_INFO& rBufInfo) const;

    MBOOL                   doShadingAlign(MVOID* pMtkLsc);
    MBOOL                   doShadingTrfm();
    // golden/unit gain table xg*yg
    // input/output coef table xc*yc
    MINT32                  shadingTblAlign(const ShadingTblAlignCfg_T& rAlignCfg, MUINT32* pu4OutputCoefTbl);
    MINT32                  shadingTblCrop(const ShadingTblCropCfg_T& rCropCfg, const ShadingTblInput_T& rInput, MUINT32* pu4OutputCoefTbl);

    MBOOL                   convertSensorCrop(MBOOL fgWithSensorCropInfo, const SensorCropInfo_T& rFullInfo, const SensorCropInfo_T& rCropInfo, ShadingTblCropCfg_T& rCropCfg);

    MBOOL                   createThread();
    MBOOL                   destroyThread();
    MVOID                   changeThreadSetting();
    MVOID                   doThreadFunc();
    MVOID                   initThreadFunc();
    MVOID                   uninitThreadFunc();
    MBOOL                   loadOtpDataForTsf();
    static MVOID *          threadLoop(void *arg);

    MVOID                   updateTsfCfg(UINT32 u4W, UINT32 u4H, const ISP_NVRAM_LSC_T& rLscCfg, ELscScenario_T eLscScn, MBOOL fgForce=MFALSE);

    MBOOL                   tsfInit();
    MBOOL                   tsfRun();
    MBOOL                   tsfBatch();
    MBOOL                   tsfBatchCap();
    MBOOL                   tsfCfgChg();
    MBOOL                   tsfResetTbl(ELscScenario_T eLscScn);
    MBOOL                   tsfSetTbl(const MUINT32* pu4Tbl, MBOOL fgOverWriteInput=MTRUE);
    MBOOL                   tsfSetSL2(const MTK_TSF_SL2_PARAM_STRUCT& rSL2);
    MBOOL                   tsfSetProcInfo(MTK_TSF_SET_PROC_INFO_STRUCT& rProcInfo, MBOOL fgDump = MFALSE, const char* filename = NULL);
    MBOOL                   tsfPostCmd(E_TSF_CMD_T eCmd);

    ESensorDev_T                    m_eSensorDev;
    MINT32                          m_i4SensorIdx;
    //ESensorTG_T                     m_eSensorTG;

    ELscScenario_T                  m_eLscScenario;

    ESensorMode_T                   m_eSensorMode;
    ESensorMode_T                   m_ePrevSensorMode;
    MUINT32                         m_u4NumSensorModes;

    SENSOR_RESOLUTION_INFO_T        m_rSensorResolution;
    SensorCropInfo_T                m_rCurSensorCrop;
    SensorCropInfo_T                m_rSensorCropWin[ESensorMode_NUM];
    ShadingTblCropCfg_T             m_rSensorCropCfg[ESensorMode_NUM];

    MBOOL                           m_fgOnOff;
    MBOOL                           m_fgInit;
    MBOOL                           m_fgSensorCropInfoNull;
    MBOOL                           m_bIsEEPROMImported;
    MBOOL                           m_fgCtIdxExcd;
    MBOOL                           m_fgBypass1to3;
    MINT32                          m_fg1to3;
    MINT32                          m_i4OtpState;
    MUINT32                         m_u4CTIdx;
    MUINT32*                        m_pBaseAddr;
    MUINT32                         m_u4LogEn;

    // sdblk, exif
    MBOOL                           m_bDumpSdblk;
    std::string                     m_strSdblkFile;
    MTK_TSF_RESULT_INFO_STRUCT*     m_pTsfResultInfo;

    // memory management
    IMemDrv*                        m_pIMemDrv;
    IMEM_BUF_INFO                   stRawLscInfo[LSC_SCENARIO_NUM];
    IMEM_BUF_INFO                   m_rBufInfo[2];
    MUINT32                         m_u4DoubleBufIdx;
    MBOOL                           m_fgUserSetTbl;

    // NVRAM data
    typedef ISP_NVRAM_LSC_T LSCParameter[LSC_SCENARIO_NUM];
    LSCParameter&                   m_rIspLscCfg;
    NVRAM_CAMERA_ISP_PARAM_STRUCT&  m_rIspNvram;
    NVRAM_CAMERA_3A_STRUCT*         m_prNvram3A;
    ISP_SHADING_STRUCT*             m_prShadingLut;
    CAMERA_TSF_TBL_STRUCT           m_rTsfCfgTbl;
    ISP_NVRAM_LSC_T                 m_rLscCfg;
    ISP_NVRAM_SL2_T                 m_rSl2Cfg;

    // OTP
    CAM_CAL_SINGLE_LSC_STRUCT       m_rOtp;

    // thread
    MBOOL                           m_fgThreadLoop;
    pthread_t                       m_Thread;
    pthread_mutex_t                 m_Mutex;
    sem_t                           m_Sema;

    // tsf
    MBOOL                           m_bTSF;
    MBOOL                           m_bTSFInit;
    MBOOL                           m_bTsfForceAwb;
    E_TSF_CMD_T                     m_eTsfCmd;
    MTKTsf*                         m_prTsf;
    TSF_REF_INFO_T                  m_rTsfRefInfo;
    MTK_TSF_ENV_INFO_STRUCT         m_rTsfEnvInfo;
    MTK_TSF_GET_ENV_INFO_STRUCT     m_rTsfGetEnvInfo;
    MTK_TSF_SET_PROC_INFO_STRUCT    m_rTsfSetProc;
    MTK_TSF_RESULT_INFO_STRUCT      m_rTsfResult;
    MTK_TSF_GET_PROC_INFO_STRUCT    m_rTsfGetProc;
    MTK_TSF_GET_LOG_INFO_STRUCT     m_rTsfGetLog;
    MTK_TSF_LSC_PARAM_STRUCT        m_rTsfLscParam;
    MTKTSF_STATE_ENUM               m_rTsfState;
    MTK_TSF_TBL_STRUCT              m_rTsfTbl;
    MUINT8*                         m_prAwbStat;
    MUINT32*                        m_pu4ProcTbl;
    MUINT32*                        m_pu4ResultTbl;
    MUINT32                         m_u4PerFrameStep;
};

}
#endif // _LSC_MGR2_H_

