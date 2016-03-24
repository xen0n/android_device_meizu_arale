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
#define LOG_TAG "IspDrv_FrmB"

//
#include <stdlib.h>
//
#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
//#include <utils/threads.h>
#include <cutils/atomic.h>
//#include <cutils/pmem.h>

#include "camera_isp_FrmB_D2.h"
//#include "mm_proprietary.h"
#include <linux/mman-proprietary.h>
#include <mtkcam/drv_common/isp_reg.h>
#include "isp_drv_imp_FrmB.h"

#include <cutils/properties.h>  // For property_get().
#include <mtkcam/imageio/ispio_pipe_scenario.h>    // For enum EDrvScenario.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{IspDrv} "
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv);
//EXTERN_DBG_LOG_VARIABLE(isp_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST

#define LOG_VRB(fmt, arg...)        do { if (isp_drv_DbgLogEnable_VERBOSE && MTKCAM_LOGENABLE_DEFAULT) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_DbgLogEnable_DEBUG && MTKCAM_LOGENABLE_DEFAULT) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_DbgLogEnable_INFO && MTKCAM_LOGENABLE_DEFAULT ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


///////////////
using namespace NSIspDrv_FrmB;


// for debug CQ virtual table
static char CQDump_str[512];
#define LOG_CQ_VIRTUAL_TABLE(_pIspVirCqVa_,_idx_,_num_) \
{   \
    CQDump_str[0] = '\0';\
    char* _ptr = CQDump_str;\
    for(int j=0;j<_num_;j++){\
        if(j > 20)\
            break;\
        else{\
            sprintf(_ptr,"0x%08x-",_pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst >>2)+j]);\
            while(*_ptr++ != '\0');\
            _ptr -= 1;\
        }\
    }\
    LOG_INF("%s\n",CQDump_str);\
    CQDump_str[0] = '\0';\
}


// for tuning queue
#define GET_NEXT_TUNING_QUEUE_IDX(_idx_)    ( ((((MINT32)_idx_)+1)>=ISP_TUNING_QUEUE_NUM )?(0):(((MINT32)_idx_)+1) )
#define GET_PREV_TUNING_QUEUE_IDX(_idx_)    ( ((((MINT32)_idx_)-1)>=0 )?(((MINT32)_idx_)-1):(ISP_TUNING_QUEUE_NUM-1) )


// for checking the difference between tpipe and hw register
#define CHECH_TPIPE_REG(_tpipePoint_ ,_tpipeField_, _regPoint_, _regElement1_, _regElement2)    \
{                                                                                               \
    if(_tpipePoint_->_tpipeField_ != (int)_regPoint_->_regElement1_.Bits._regElement2){         \
        int oriVal, parseVal, i;                                                                \
        int intSize = 32;                                                              \
        /* backup the original value */                                                         \
        oriVal = _regPoint_->_regElement1_.Raw;                                                 \
        /* start to parse union bits */                                                         \
        _regPoint_->_regElement1_.Raw = 0;                                                      \
        _regPoint_->_regElement1_.Bits._regElement2 = 0xffffffff;                               \
        parseVal = _regPoint_->_regElement1_.Raw;                                               \
        /*LOG_INF("parseVal(0x%x)",parseVal);*/                                                 \
        for(i=0;i<intSize;i++){                                                                 \
            if(parseVal & 0x01<<i)                                                              \
                break;                                                                          \
        }                                                                                       \
        /* recover the original value */                                                        \
        _regPoint_->_regElement1_.Raw = oriVal;                                                 \
        LOG_INF("[Diff][0x%x][Bit-%d]",                                                         \
                (MUINTPTR)(&_regPoint_->_regElement1_)-(MUINTPTR)(&_regPoint_->rsv_0000[0]), i);          \
    }                                                                                           \
}



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/


class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
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
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
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
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
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

#define ISP_CQ_WRITE_INST           0x0
#define ISP_DRV_CQ_END_TOKEN       0xFC000000
#define ISP_DRV_CQ_DUMMY_WR_TOKEN  0x00004060
#define ISP_CQ_DUMMY_PA             0x88100000


#define ISP_INT_BIT_NUM 32
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/


//-----------------------------------------------------------------------------
/**************************************************************************
* Member Variable Initilization
**************************************************************************/

namespace NSIspDrv_FrmB {
pthread_mutex_t IspTopRegMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IspOtherRegMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IspCQinfoMutex = PTHREAD_MUTEX_INITIALIZER;

MUINT32*    IspDrvImp::mpIspHwRegAddr = NULL;

isp_reg_t*  IspDrv::mpIspVirRegMap;
MINT32      IspDrv::mIspVirRegFd;
MUINT32*    IspDrv::mpIspVirRegBufferBaseAddr;
MUINT32     IspDrv::mIspVirRegSize;
MUINT32**   IspDrv::mpIspVirRegAddrVA=NULL; // CQ(VA) framework size
MUINT32**   IspDrv::mpIspVirRegAddrPA=NULL; // CQ(PA) framework size
MUINT32     IspDrv::mIspVirRegAddrVAFd;
MUINT32     IspDrv::mIspVirRegAddrPAFd;
MINT32      IspDrv::mIspCQDescFd;
MUINT32*    IspDrv::mpIspCQDescBufferVirt = NULL;
MUINT32     IspDrv::mIspCQDescSize;
MUINT32*     IspDrv::mpIspCQDescBufferPhy = NULL;    // Fix build warning
ISP_DRV_CQ_CMD_DESC_STRUCT **IspDrv::mpIspCQDescriptorVirt=NULL;
MUINT32** IspDrv::mpIspCQDescriptorPhy=NULL;
MINT32      IspDrv::mCurBurstQNum=1;
// cq order: cq0, cq0b, cq0c, cq0_d, cq0b_d, cq0c_d, burstQ0_cq1_dup0,burstQ0_cq2_dup0,burstQ0_cq3_dup0,burstQ0_cq1_dup1,burstQ0_cq2_dup1,burstQ0_cq3_dup1,
//           burstQ1_cq1_dup0,burstQ1_cq2_dup0,burstQ1_cq3_dup0,burstQ1_cq1_dup1,burstQ1_cq2_dup1,burstQ1_cq3_dup1,...and so on
MINT32      IspDrv::mTotalCQNum=ISP_DRV_BASIC_CQ_NUM;
// for turning update
 stIspTuningQueInf IspDrv::mTuningQueInf[ISP_DRV_P2_CQ_NUM][ISP_TUNING_QUEUE_NUM];
 stIspTuningQueIdx IspDrv::mTuningQueIdx[ISP_DRV_P2_CQ_NUM];

}



ISP_DRV_CQ_CMD_DESC_INIT_STRUCT  mIspCQDescInit[CAM_MODULE_MAX]
#if 1
= {{ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_END_TOKEN,(MUINT32)0}};
#endif

ISP_DRV_CQ_MODULE_INFO_STRUCT mIspCQModuleInfo[CAM_MODULE_MAX]
=   { {CAM_TOP_CTL,               0x4050, 2  },
      {CAM_TOP_CTL_01,            0x4080, 10 },
      {CAM_TOP_CTL_02,            0x40C0, 8  },
      {CAM_ISP_MDP_CROP,          0x4110, 2  },
      {CAM_DMA_TDRI,              0x4204, 3  },
      {CAM_DMA_IMGI,              0x4230, 7/*8*/  }, //0x4240/*0x4230*/, 3 /*8*/  },//0x4234/*0x4230*/, 6 /*8*/  }, //remove BASE/OFST/XS/YX/RSV from 4230 to 4244 just IMGI_CON.CON2 {CAM_DMA_IMGI,              0x4230, 8  }, //SL TEST_MDP
      {CAM_DMA_LSCI,              0x426C, 7  },
      {CAM_DMA_IMGO_BASEADDR,     0x4300, 1  },
      {CAM_DMA_IMGO,              0x4304, 7  },
      {CAM_DMA_IMG2O_BASEADDR,    0x4320, 1  },
      {CAM_DMA_IMG2O,             0x4324, 7  },
      {CAM_DMA_EISO,              0x435C, 2  },
      {CAM_DMA_AFO,               0x4364, 2  },
      {CAM_DMA_ESFKO,             0x436C, 7  },
      {CAM_DMA_AAO,               0x4388, 7  },
      {CAM_RAW_TG1_TG2,           0x4410, 30 },// MT6582 only TG1
      {CAM_ISP_BIN,               0x44F0, 2  },
      {CAM_ISP_OBC,               0x4500, 8  },
      {CAM_ISP_LSC,               0x4530, 8  },
      {CAM_ISP_HRZ,               0x4580, 2  },
      {CAM_ISP_AWB,               0x45B0, 36 },
      {CAM_ISP_AE,                0x4650, 18 },
      {CAM_ISP_SGG,               0x46A0, 2  },
      {CAM_ISP_AF,                0x46B0, 23 },
      {CAM_ISP_FLK,               0x4770, 4  },
      {CAM_ISP_BNR,               0x4800, 18 },
      {CAM_ISP_PGN,               0x4880, 6  },
      {CAM_ISP_CFA,               0x48A0, 22},//22 },
      {CAM_ISP_CCL,               0x4910, 3  },
      {CAM_ISP_G2G,               0x4920, 7  },
      {CAM_ISP_UNP,               0x4948, 1  },
      {CAM_ISP_G2C,               0x4A00, 6  },
      {CAM_ISP_C42,               0x4A1C, 1  },
      {CAM_ISP_NBC,               0x4A20, 32 },
      {CAM_ISP_SEEE,              0x4AA0, 24 },
      {CAM_CDP_CDRZ,              0x4B00, 15 },
      {CAM_ISP_EIS,               0x4DC0, 9  },
      {CAM_CDP_SL2_FEATUREIO,     0x4F40, 7  },
      {CAM_ISP_GGMRB,             0x5000, 144},
      {CAM_ISP_GGMG,              0x5300, 144},
      {CAM_ISP_GGM_CTL,           0x5600, 1  },
      {CAM_ISP_PCA,               0x5800, 360},
      {CAM_ISP_PCA_CON,           0x5E00, 2  },
      {CAM_P1_MAGIC_NUM,          0x43DC, 1  },
      {CAM_ISP_EIS_DB,            0x406C, 1  },
      {CAM_ISP_EIS_DCM,           0x419C, 1  },
      {CAM_DUMMY_,                0x4780, 1  }};  // dummy address 4780//0x4600, 1  }};  // dummy address



 ISP_TURNING_FUNC_BIT_MAPPING gIspTuningFuncBitMapp[eIspTuningMgrFuncBit_Num]
   ={{eIspTuningMgrFuncBit_Obc             ,  eTuningCtlByte_P1,  3, -1, -1, CAM_ISP_OBC,          CAM_DUMMY_},
     {eIspTuningMgrFuncBit_Lsc             ,  eTuningCtlByte_P1,  5, -1,  1, CAM_ISP_LSC,          CAM_DMA_LSCI},
     {eIspTuningMgrFuncBit_Bnr             ,  eTuningCtlByte_P1,  7, -1, -1, CAM_ISP_BNR,          CAM_DUMMY_},
     {eIspTuningMgrFuncBit_Pgn             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_PGN,          CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Cfa             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_CFA,          CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Ccl             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_CCL,          CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_G2g             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_G2G,          CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_G2c             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_G2C,          CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Nbc             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_NBC,          CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Seee            ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_SEEE,         CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Sl2             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_CDP_SL2_FEATUREIO,CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Ggmrb           ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_GGMRB,        CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Ggmg            ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_GGMG,         CAM_DUMMY_}, // not support this setting through tuningMgr
     {eIspTuningMgrFuncBit_Pca             ,  eTuningCtlByte_P2, -1, -1, -1, CAM_ISP_PCA,          CAM_DUMMY_}}; // not support this setting through tuningMgr





ISP_DRV_CQ_BIT_3GP_MAPPING gIspTurnEn1Mapp[ISP_INT_BIT_NUM]
= { {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 0
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 1
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 2
    {CAM_ISP_OBC,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 3
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 4
    {CAM_ISP_LSC,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 5
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 6
    {CAM_ISP_BNR,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 7
    {CAM_CDP_SL2_FEATUREIO,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 8
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 9
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 10
    {CAM_ISP_PGN,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 11
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 12
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 13
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 14
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 15
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 16
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 17
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 18
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 19
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 20
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 21
    {CAM_ISP_CCL,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 22
    {CAM_ISP_G2G,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 23
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 24
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_}, // bit 25
    {CAM_ISP_GGMRB,       CAM_ISP_GGMG,       CAM_ISP_GGM_CTL},     // bit 26 (GGM controled by 0x5600)
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 27
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 28
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 29
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 30
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_}};         // bit 31

ISP_DRV_CQ_BIT_2GP_MAPPING gIspTurnEn2Mapp[ISP_INT_BIT_NUM]
= { {CAM_ISP_G2C,  CAM_DUMMY_},       // bit 0
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 1
    {CAM_ISP_NBC,  CAM_DUMMY_},       // bit 2
    {CAM_ISP_PCA,  CAM_ISP_PCA_CON},  // bit 3
    {CAM_ISP_SEEE, CAM_DUMMY_},       // bit 4
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 5
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 6
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 7
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 8
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 9
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 10
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 11
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 12
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 13
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 14
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 15
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 16
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 17
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 18
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 19
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 20
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 21
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 22
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 23
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 24
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 25
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 26
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 27
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 28
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 29
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 30
    {CAM_DUMMY_,   CAM_DUMMY_}};      // bit 31

ISP_DRV_CQ_BIT_1GP_MAPPING gIspTurnDmaMapp[ISP_INT_BIT_NUM]
= { {CAM_DUMMY_   },  // bit 0
    {CAM_DMA_LSCI },  // bit 1
    {CAM_DUMMY_   },  // bit 2
    {CAM_DUMMY_   },  // bit 3
    {CAM_DUMMY_   },  // bit 4
    {CAM_DUMMY_   },  // bit 5
    {CAM_DUMMY_   },  // bit 6
    {CAM_DUMMY_   },  // bit 7
    {CAM_DUMMY_   },  // bit 8
    {CAM_DUMMY_   },  // bit 9
    {CAM_DUMMY_   },  // bit 10
    {CAM_DUMMY_   },  // bit 11
    {CAM_DUMMY_   },  // bit 12
    {CAM_DUMMY_   },  // bit 13
    {CAM_DUMMY_   },  // bit 14
    {CAM_DUMMY_   },  // bit 15
    {CAM_DUMMY_   },  // bit 16
    {CAM_DUMMY_   },  // bit 17
    {CAM_DUMMY_   },  // bit 18
    {CAM_DUMMY_   },  // bit 19
    {CAM_DUMMY_   },  // bit 20
    {CAM_DUMMY_   },  // bit 21
    {CAM_DUMMY_   },  // bit 22
    {CAM_DUMMY_   },  // bit 23
    {CAM_DUMMY_   },  // bit 24
    {CAM_DUMMY_   },  // bit 25
    {CAM_DUMMY_   },  // bit 26
    {CAM_DUMMY_   },  // bit 27
    {CAM_DUMMY_   },  // bit 28
    {CAM_DUMMY_   },  // bit 29
    {CAM_DUMMY_   },  // bit 30
    {CAM_DUMMY_   }}; // bit 31




//-----------------------------------------------------------------------------
IspDrv* IspDrv::createInstance()
{
    DBG_LOG_CONFIG(drv, isp_drv);
    return IspDrvImp::getInstance();
}
//-----------------------------------------------------------------------------
IspDrv*  IspDrv::getCQInstance(MINT32 cq)
{
    //LOG_DBG("");
    return IspDrvVirImp::getInstance(cq,mpIspVirRegAddrVA[cq],mpIspVirRegMap);
}
//-----------------------------------------------------------------------------
MINT32  IspDrv::getRealCQIndex(MINT32 cqBaseEnum,MINT32 burstQIdx,MINT32 dupCqIdx)
{
    MINT32 realcqIdx=0;
    if(cqBaseEnum == 0xffff)//CAM_ISP_CQ_NONE
        return 0;
    if(cqBaseEnum>ISP_DRV_CQ03)
    {
        LOG_ERR("CQ crash error: enum sequence error, (%d/%d)\n",cqBaseEnum,ISP_DRV_CQ03);
        return -1;
    }
    else if(cqBaseEnum<=ISP_DRV_CQ0C)
    {//pass1 cq
        if(burstQIdx != 0){
            burstQIdx = 0;
            LOG_ERR("p1 suppoort no burstQidx");
        }
        realcqIdx=(burstQIdx*cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_P1DUPCQNUM) * ISP_DRV_P1_PER_CQ_SET_NUM)+(cqBaseEnum + dupCqIdx*ISP_DRV_P1_PER_CQ_SET_NUM);
    }
    else
    {//pass2 cq
        cqBaseEnum -= ISP_DRV_CQ01;
        realcqIdx=cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM)*ISP_DRV_P1_PER_CQ_SET_NUM*cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_P1DUPCQNUM) + \
        (burstQIdx*cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_P2DUPCQNUM) * ISP_DRV_P2_PER_CQ_SET_NUM)+\
        (cqBaseEnum+dupCqIdx*ISP_DRV_P2_PER_CQ_SET_NUM);
    }
    LOG_DBG("realcqIdx(%d),cqBaseEnum(%d),burstQIdx(%d),dupCqIdx(%d)",realcqIdx,cqBaseEnum,burstQIdx,dupCqIdx);
    return realcqIdx;
}
//-----------------------------------------------------------------------------
int IspDrv::getCqModuleInfo(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    int cmd;
    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
    LOG_DBG("- E. isp_cq[0x%x],[%d]",realCQIdx,moduleId);
    //
    Mutex::Autolock lock(cqVirDesLock);
    return mpIspCQDescriptorVirt[realCQIdx][moduleId].u.cmd;
}

MBOOL IspDrv::cqAddModule(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    int cmd;
    MUINTPTR dummyaddr;
    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
        LOG_DBG("- E. isp_cq[0x%x],[%d]",realCQIdx,moduleId,burstQIdx,dupCqIdx);
    //
    Mutex::Autolock lock(cqVirDesLock);

    dummyaddr = (MUINTPTR)((MUINTPTR)mpIspVirRegAddrPA[realCQIdx] + mIspCQModuleInfo[moduleId].addr_ofst); //kk test
    cmd = (mIspCQModuleInfo[moduleId].addr_ofst&0xffff)|(((mIspCQModuleInfo[moduleId].reg_num-1)&0x3ff)<<16)|((ISP_CQ_WRITE_INST)<<26);
    mpIspCQDescriptorVirt[realCQIdx][moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF;// >>2 for MUINT32* pointer
    mpIspCQDescriptorVirt[realCQIdx][moduleId].u.cmd = cmd;
    //
    LOG_DBG("- X.");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::cqDelModule(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    LOG_DBG("cq(%d),burstQIdx(%d),dupCqIdx(%d),moduleId(%d)",cq,burstQIdx,dupCqIdx,moduleId);

    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
    LOG_DBG("+,isp_cq[0x%x],[%d]",realCQIdx,moduleId);
    //
    Mutex::Autolock lock(cqVirDesLock);
    mpIspCQDescriptorVirt[realCQIdx][moduleId].u.cmd = ISP_DRV_CQ_DUMMY_WR_TOKEN;
    //
    LOG_DBG("-,");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MUINT32* IspDrv::getCQDescBufPhyAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx)
{
    Mutex::Autolock lock(cqPhyDesLock);
    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
    LOG_DBG("cq(%d),dupCqIdx(%d),realCQIdx(%d)",cq,dupCqIdx,realCQIdx);
    return (MUINT32*)mpIspCQDescriptorPhy[realCQIdx];
}
//-----------------------------------------------------------------------------
MUINT32* IspDrv::getCQDescBufVirAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx)
{
    Mutex::Autolock lock(cqVirDesLock);
    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
    LOG_DBG("cq(%d),dupCqIdx(%d),realCQIdx(%d)",cq,dupCqIdx,realCQIdx);
    return (MUINT32*)mpIspCQDescriptorVirt[realCQIdx];
}
//-----------------------------------------------------------------------------
MUINT32* IspDrv::getCQVirBufVirAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx)
{
    Mutex::Autolock lock(cqVirDesLock);
    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
    LOG_DBG("cq(%d),dupCqIdx(%d),realCQIdx(%d)",cq,dupCqIdx,realCQIdx);
    return (MUINT32*)mpIspVirRegAddrVA[realCQIdx];


}

/*******************************************************************************
*
********************************************************************************/
MBOOL IspDrv::setCQTriggerMode(ISP_DRV_CQ_ENUM cq,
                                      ISP_DRV_CQ_TRIGGER_MODE_ENUM mode,
                                      ISP_DRV_CQ_TRIGGER_SOURCE_ENUM trig_src)
{
    //isp_reg_t *pIspReg = (isp_reg_t *)getRegAddr(); //no need in this function

    LOG_DBG("+,[%s],cq(%d),mode(%d),trig_src(%d)",__FUNCTION__, cq, mode, trig_src);

    switch(cq)
    {
        case ISP_DRV_CQ0:
            //trigger source is pass1_done
            if(CQ_SINGLE_IMMEDIATE_TRIGGER == mode)
            {
                //-Immediately trigger
                //-CQ0_MODE=1(reg_4018[17]), CQ0_CONT=0(reg_4018[2])
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0_MODE, 1,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0_CONT, 0,ISP_DRV_USER_ISPF);

                ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0_MODE_SET, 1,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0_CONT_SET, 0,ISP_DRV_USER_ISPF);
            }
            else if(CQ_SINGLE_EVENT_TRIGGER == mode)
            {
                //-Trigger and wait trigger source
                //-CQ0_MODE=0, CQ0_CONT=0
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0_MODE, 0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0_CONT, 0,ISP_DRV_USER_ISPF);

                ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0_MODE_SET, 0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0_CONT_SET, 0,ISP_DRV_USER_ISPF);
            }
            else if(CQ_CONTINUOUS_EVENT_TRIGGER == mode)
            {
                //-Continuous mode support(without trigger)
                //-CQ0_MODE=x, CQ0_CONT=1
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0_MODE, 0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0_CONT, 1,ISP_DRV_USER_ISPF);

                ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0_MODE_SET, 0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0_CONT_SET, 1,ISP_DRV_USER_ISPF);
            }
            break;
        case ISP_DRV_CQ0B:
            //-choose trigger source by CQ0B_SEL(0:img2o, 1:pass1_done)(reg_4018[11])
            if(CQ_TRIG_BY_PASS1_DONE != trig_src && CQ_TRIG_BY_IMGO_DONE != trig_src)
            {
                LOG_ERR("[%s][ISP_DRV_CQ0B]:NOT Support trigger source",__FUNCTION__);
            }

            if(CQ_TRIG_BY_PASS1_DONE == trig_src)
            {
                    ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_SEL, 1,ISP_DRV_USER_ISPF);
            }
            else
            {
                    ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_SEL, 0,ISP_DRV_USER_ISPF);
            }

            if(CQ_SINGLE_IMMEDIATE_TRIGGER == mode)
            {
                //-Immediately trigger
                //-CQ0B_MODE=1 reg_4018[25], CQ0B_CONT=0 reg_4018[3]
                    ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_MODE, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_CONT, 0,ISP_DRV_USER_ISPF);
            }
            else if(CQ_SINGLE_EVENT_TRIGGER == mode)
            {
                //-Trigger and wait trigger source
                //-CQ0B_MODE=0, CQ0B_CONT=0
                    ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_MODE, 0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_CONT, 0,ISP_DRV_USER_ISPF);
            }
            else if(CQ_CONTINUOUS_EVENT_TRIGGER == mode)
            {
                //-Continuous mode support(without trigger)
                //-CQ0B_MODE=x, CQ0B_CONT=1
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_MODE, 0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_SEL, CQ0B_CONT, 1,ISP_DRV_USER_ISPF);
            }
            break;
        case ISP_DRV_CQ0C:
            //-cq0c
            //    -trigger source is imgo_done (CQ0C_IMGO_SEL_SET=1,reg_40A0[7])
            //                    OR img20_done(CQ0C_IMGO_SEL_SET=1,reg_40A0[22]).
            //    -always continuous mode,NO cq0c_start.
            //     If cq0c_en=1, it always load CQ when imgo_done||img2o_done occur
            if ( CQ_TRIG_BY_IMGO_DONE != trig_src && CQ_TRIG_BY_IMG2O_DONE != trig_src)
            {
                LOG_ERR("[%s][ISP_DRV_CQ0C]:NOT Support trigger source\n",__FUNCTION__);
            }

            if(CQ_TRIG_BY_IMGO_DONE == trig_src)
            {
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0C_IMGO_SEL_SET, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_CLR, CQ0C_IMGO_SEL_CLR, 0,ISP_DRV_USER_ISPF);
            }
            else
            {
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0C_IMGO_SEL_SET, 0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_CLR, CQ0C_IMGO_SEL_CLR, 1,ISP_DRV_USER_ISPF);
            }

            if(CQ_TRIG_BY_IMG2O_DONE == trig_src)
            {
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0C_IMG2O_SEL_SET, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_CLR, CQ0C_IMG2O_SEL_CLR, 0,ISP_DRV_USER_ISPF);
            }
            else
            {
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_SET, CQ0C_IMG2O_SEL_SET, 0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(this, CAM_CTL_SEL_CLR, CQ0C_IMG2O_SEL_CLR, 1,ISP_DRV_USER_ISPF);
            }

            break;
        case ISP_DRV_CQ01:
        case ISP_DRV_CQ02:
        case ISP_DRV_CQ03:
        default:
            //-cq1/2/3
            //    -load one time. working time is at right after set pass2x_start=1
            //    -Immediately trigger ONLY
            break;
    }

    return MTRUE;
}



#define REMOVELATER_DUPIDX 0 //remove later

//-----------------------------------------------------------------------------
MBOOL IspDrv::checkTopReg(MUINT32 Addr)
{
    #if 0  //fix it later
    switch(Addr)
    {
        case 0x00004000:
            return true;
        case 0x00004004:
            return true;
        case 0x00004008:
            return true;
        case 0x00004010:
            return true;
        case 0x00004014:
            return true;
        case 0x00004018:
            return true;
        case 0x0000401C:
            return true;
        case 0x00004020:
            return true;
        case 0x00004034:
            return true;
        case 0x00004038:
            return true;
        case 0x0000403C:
            return true;
        case 0x00004040:
            return true;
        default:
            return false;
    }
    #else
        return false;
    #endif
}

//-----------------------------------------------------------------------------

IspDrvImp::IspDrvImp()
{
    int i;
    GLOBAL_PROFILING_LOG_START(Event_IspDrv);   // Profiling Start.

    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    mInitCount = 0;

    mFd = -1;
    m_pIMemDrv = NULL;
    m_pRTBufTbl = NULL;
    mpIspVirRegMap = NULL;
    mpTempIspHWRegValues = NULL;
    m_regRWMode=ISP_DRV_RWREG_MODE_RW;
    for(int i=0;i<((ISP_DRV_P2_CQ_DUPLICATION_NUM-1)*ISP_DRV_P2_PER_CQ_SET_NUM);i++)
    {
        ISP_DRV_CQ_MAPPING map;
        map.virtualAddrCq=ISP_DRV_BASIC_CQ_NUM+i;
        map.descriptorCq=ISP_DRV_DESCRIPTOR_BASIC_CQ_NUM+i;
    }

}
//-----------------------------------------------------------------------------
IspDrvImp::~IspDrvImp()
{
    LOG_INF("");
    GLOBAL_PROFILING_LOG_END();     // Profiling End.
}
//-----------------------------------------------------------------------------

static IspDrvImp singleton;

IspDrv* IspDrvImp::getInstance()
{
    LOG_DBG("singleton[0x%08x].", &singleton);

    return &singleton;
}
//-----------------------------------------------------------------------------
void IspDrvImp::destroyInstance(void)
{
}
//-----------------------------------------------------------------------------
//#define __PMEM_ONLY__
MBOOL IspDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    IMEM_BUF_INFO   m_ispTmpBufInfo;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_IspDrv_Init);

    //
    Mutex::Autolock lock(mLock);
    //
    LOG_INF(" - E. mInitCount(%d), curUser(%s).", mInitCount,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }
    #if 0 //todo, check debug
    m_userNameList.push_back(userName);
    #endif
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
                                                LOCAL_PROFILING_LOG_PRINT("atomic_inc");
        goto EXIT;
    }
    //
    mpIspVirRegMap = (isp_reg_t*)malloc(sizeof(isp_reg_t));    //always allocate this(be used to get register offset in macro)
    mpTempIspHWRegValues = (isp_reg_t*)malloc(sizeof(isp_reg_t));    //always allocate this(be used to get register offset in macro)
                                                LOCAL_PROFILING_LOG_PRINT("mpIspVirRegMap and mpTempIspHWRegValues malloc");
    // Open isp driver
    mFd = open(ISP_DRV_DEV_NAME, O_RDWR);
                                                LOCAL_PROFILING_LOG_PRINT("1st open(ISP_DRV_DEV_NAME, O_RDWR)");
    if (mFd < 0)    // 1st time open failed.
    {
        LOG_WRN("ISP kernel open 1st attempt fail, errno(%d):%s.", errno, strerror(errno));

        // Try again, using "Read Only".
        mFd = open(ISP_DRV_DEV_NAME, O_RDONLY);
                                                LOCAL_PROFILING_LOG_PRINT("2nd open(ISP_DRV_DEV_NAME, O_RDONLY)");
        if (mFd < 0) // 2nd time open failed.
        {
            LOG_ERR("ISP kernel open 2nd attempt fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        m_regRWMode=ISP_DRV_RWREG_MODE_RO;
                                                LOCAL_PROFILING_LOG_PRINT("mpIspVirRegMap malloc()");
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // mmap isp reg
            mpIspHwRegAddr = (MUINT32 *) mmap(0, ISP_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mFd, ISP_BASE_HW);
            if(mpIspHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("ISP mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
                                                    LOCAL_PROFILING_LOG_PRINT("mpIspHwRegAddr mmap()");
            //pass1 buffer control shared mem.
            m_RTBufTblSize = RT_BUF_TBL_NPAGES * getpagesize();
            m_pRTBufTbl = (MUINT32 *)mmap(0, m_RTBufTblSize, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED| MAP_LOCKED, mFd, m_RTBufTblSize);
            LOG_DBG("m_RTBufTblSize(0x%x),m_pRTBufTbl(0x%x)",m_RTBufTblSize,m_pRTBufTbl);
            if (m_pRTBufTbl == MAP_FAILED)
            {
                LOG_ERR("m_pRTBufTbl mmap FAIL");
                Result = MFALSE;
                goto EXIT;
            }
            m_regRWMode=ISP_DRV_RWREG_MODE_RW;
                                                    LOCAL_PROFILING_LOG_PRINT("m_pRTBufTbl mmap()");
    }


    // Increase ISP global reference count, and reset if 1st user.
#if defined(_use_kernel_ref_cnt_)
    LOG_INF("use kernel ref. cnt.mFd(%d)",mFd);

    ISP_REF_CNT_CTRL_STRUCT_FRMB ref_cnt;
    MINT32 count;   // For ISP global reference count.

    ref_cnt.ctrl = ISP_REF_CNT_GET_FRMB;
    ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC_FRMB;
    ref_cnt.data_ptr = &count;
    //
    if ( MTRUE == kRefCntCtrl(&ref_cnt) ) {
        //
        if (0==count) {
            LOG_DBG("DO ISP HW RESET");
            reset(ISP_DRV_RST_CAM_P1); // Do IMGSYS SW RST.
            reset(ISP_DRV_RST_CAM_P2);
        }
        //
        ref_cnt.ctrl = ISP_REF_CNT_INC_FRMB;
        if ( MFALSE == kRefCntCtrl(&ref_cnt) ) {
            LOG_ERR("ISP_REF_CNT_INC fail, errno(%d):%s.", errno, strerror(errno));
        }
        LOG_INF("ISP Global Count: %d.", count);
    }
    else {
        LOG_ERR("ISP_REF_CNT_GET fail, errno(%d):%s.", errno, strerror(errno));
    }
                                                LOCAL_PROFILING_LOG_PRINT("kRefCntCtrl and ISP reset()");
#else
    LOG_DBG("DO ISP HW RESET");
    reset(ISP_DRV_RST_CAM_P1); // Do IMGSYS SW RST, which will also enable CAM/SEN/JPGENC/JPGDEC clock.
    reset(ISP_DRV_RST_CAM_P2);
#endif

    /*------------------------
        Floria /
        Set Camera version:
        0 means camera 1;
        1 for camera 3
    ------------------------*/
        CAM_HAL_VER_IS3 = 1;
    if(ioctl(mFd,ISP_SET_CAM_VERSION,&CAM_HAL_VER_IS3) < 0){
        LOG_ERR("SetCamVer error\n");
    }

    /*============================================
        imem driver
        =============================================*/
    m_pIMemDrv = IMemDrv::createInstance();
    LOG_DBG("[m_pIMemDrv]:0x%08x", m_pIMemDrv);    //Vent@20121107: Fix build warning: format '%x' expects argument of type 'unsigned int', but argument 5 has type 'IMemDrv*' [-Wformat]
    if ( NULL == m_pIMemDrv ) {
        LOG_DBG("IMemDrv::createInstance fail");
        return -1;
    }
    LOCAL_PROFILING_LOG_PRINT("IMemDrv::createInstance()");
    m_pIMemDrv->init();
    LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->init()");

    //
    //virtual CQ
    //set default CQ desc
    for (int i=0; i<CAM_MODULE_MAX; i++ ) {
        mIspCQDescInit[i].cmd_set = ISP_DRV_CQ_DUMMY_WR_TOKEN;
        mIspCQDescInit[i].v_reg_addr = (MUINT32)ISP_CQ_DUMMY_PA;
    }
    mIspCQDescInit[CAM_MODULE_MAX-1].cmd_set = ISP_DRV_CQ_END_TOKEN;
    mIspCQDescInit[CAM_MODULE_MAX-1].v_reg_addr = (MUINT32)0;
    //inital total cq nun
    mCurBurstQNum=1;
    mTotalCQNum=(cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_P1DUPCQNUM) * ISP_DRV_P1_PER_CQ_SET_NUM)+\
    (cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM) * cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_P2DUPCQNUM) * ISP_DRV_P2_PER_CQ_SET_NUM);
    LOG_DBG("mTotalCQNum(%d)",mTotalCQNum);

    //initial cq related table(descriptor and virtual reg map)
    Result=cqTableControl(ISP_DRV_CQTABLE_CTRL_ALLOC,mCurBurstQNum);
    if(Result == MFALSE)
    {
        LOG_ERR("cqTableControl fail");
        goto EXIT;
    }
    //
#if defined(__PMEM_ONLY__)
    // For Tuning Queue
    LOG_INF("alloc tuning Queue");
    for(int i=0;i<ISP_DRV_P2_CQ_NUM;i++){
        for(int j=0;j<ISP_TUNING_QUEUE_NUM;j++){
            mTuningQueInf[i][j].queSize = ISP_BASE_RANGE;
            mTuningQueInf[i][j].pTuningQue = (MUINT32*)pmem_alloc_sync(mTuningQueInf[i][j].queSize, &mTuningQueInf[i][j].queFd);
            memset((MUINT8*)mTuningQueInf[i][j].pTuningQue,0,mTuningQueInf[i][j].queSize);
        }
        //
        mTuningQueIdx[i].keepP1Que.pTuningQue.queSize = ISP_BASE_RANGE;
        mTuningQueIdx[i].keepP1Que.pTuningQue.pTuningQue = (MUINT32*)pmem_alloc_sync(mTuningQueIdx[i].keepP1Que.queSize, &mTuningQueIdx[i].keepP1Que.queFd);
        memset((MUINT8*)mTuningQueIdx[i].keepP1Que.pTuningQue,0,mTuningQueIdx[i].keepP1Que.queSize);
        //
        mTuningQueIdx[i].isApplyTuning = MFALSE;
        mTuningQueIdx[i].pCurReadP1TuningQue = NULL;
        mTuningQueIdx[i].pCurWriteTuningQue = NULL;
        mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepReadP1CtlEn1 = 0;
        mTuningQueIdx[i].keepReadP1CtlEn2 = 0;
        mTuningQueIdx[i].keepReadP1CtlDmaEn = 0;
        //
        mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curReadP1Idx = ISP_TUNING_INIT_IDX;
        //mTuningQueIdx[i].isInitP2 = MFALSE;
        mTuningQueIdx[i].isInitP1 = MFALSE;
    }

#else   // Not PMEM.
    // For Tuning Queue
    LOG_INF("alloc tuning Queue");
    for(int i=0;i<ISP_DRV_P2_CQ_NUM;i++){
        for(int j=0;j<ISP_TUNING_QUEUE_NUM;j++){
            mTuningQueInf[i][j].queSize = ISP_BASE_RANGE;
            m_ispTmpBufInfo.size = mTuningQueInf[i][j].queSize;
            m_ispTmpBufInfo.useNoncache = 0; //alloc cacheable mem.
            if ( m_pIMemDrv->allocVirtBuf(&m_ispTmpBufInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
            }
            mTuningQueInf[i][j].queFd = m_ispTmpBufInfo.memID;
            mTuningQueInf[i][j].pTuningQue = (MUINT32*)m_ispTmpBufInfo.virtAddr;
            memset((MUINT8*)mTuningQueInf[i][j].pTuningQue,0,mTuningQueInf[i][j].queSize);
            LOG_INF("p2cq(%d),que(%d):pTuningQue(0x%x)",i,j,mTuningQueInf[i][j].pTuningQue);
        }
        //
        mTuningQueIdx[i].keepP1Que.queSize = ISP_BASE_RANGE;
        m_ispTmpBufInfo.size = mTuningQueIdx[i].keepP1Que.queSize;
        m_ispTmpBufInfo.useNoncache = 0; //alloc cacheable mem.
        if ( m_pIMemDrv->allocVirtBuf(&m_ispTmpBufInfo) ) {
            LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
        }
        mTuningQueIdx[i].keepP1Que.queFd = m_ispTmpBufInfo.memID;
        mTuningQueIdx[i].keepP1Que.pTuningQue = (MUINT32*)m_ispTmpBufInfo.virtAddr;
        memset((MUINT8*)mTuningQueIdx[i].keepP1Que.pTuningQue,0,mTuningQueIdx[i].keepP1Que.queSize);
        LOG_INF("p2cq(%d),keepP1Que(0x%x)",i,mTuningQueIdx[i].keepP1Que.pTuningQue);
        //
        mTuningQueIdx[i].isApplyTuning = MFALSE;
        mTuningQueIdx[i].pCurReadP1TuningQue = NULL;
        mTuningQueIdx[i].pCurWriteTuningQue = NULL;
        mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepReadP1CtlEn1 = 0;
        mTuningQueIdx[i].keepReadP1CtlEn2 = 0;
        mTuningQueIdx[i].keepReadP1CtlDmaEn = 0;
        //
        mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curReadP1Idx = ISP_TUNING_INIT_IDX;
        //mTuningQueIdx[i].isInitP2 = MFALSE;
        mTuningQueIdx[i].isInitP1 = MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->mapPhyAddr()");
    //
#endif  // __PMEM_ONLY__

    //
    loadInitSetting(); // load default setting

    //
    android_atomic_inc(&mInitCount);
    LOCAL_PROFILING_LOG_PRINT("atomic_inc");
    //
EXIT:

    if (!Result)    // If some init step goes wrong.
    {
        if(mFd >= 0)
        {
            close(mFd);
            mFd = -1;
            LOCAL_PROFILING_LOG_PRINT("close isp mFd");
        }
    }

    LOG_INF(" - X. ret: %d. mInitCount: %d.", Result, mInitCount);
    LOCAL_PROFILING_LOG_PRINT("Exit");

    return Result;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    IMEM_BUF_INFO   m_ispTmpBufInfo;
    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_IspDrv_Uninit);
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_INF(" - E. mInitCount(%d), curUser(%s)", mInitCount,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }
    //
    #if 0 //todo, check debug
    if(strlen(userName)==0)
    {
    }
    else
    {
        m_userNameList.remove(userName);
        list<const char*>::iterator iter;
        char* userlist=(char*)malloc(128*sizeof(char));
        for (iter = m_userNameList.begin(); iter != m_userNameList.end(); ++iter)
        {
           strcat(userlist,*iter);
           strcat(userlist,",");
        }
        LOG_INF("ISPDrv RestUsers:%s\n",userlist);
        free(userlist);
    }
    #endif
    //
    if(mInitCount <= 0)
    {
        // No more users
        goto EXIT;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    LOCAL_PROFILING_LOG_PRINT("atomic_dec");

    if(mInitCount > 0)    // If there are still users, exit.
    {
        goto EXIT;
    }

    munmap(mpIspHwRegAddr, ISP_BASE_RANGE);
    mpIspHwRegAddr = NULL;
    LOCAL_PROFILING_LOG_PRINT("munmap(mpIspHwRegAddr)");
    //
    munmap(m_pRTBufTbl, m_RTBufTblSize);
    m_pRTBufTbl = NULL;
    LOCAL_PROFILING_LOG_PRINT("munmap(m_pRTBufTbl)");

    //free cq descriptor and virtual isp
    cqTableControl(ISP_DRV_CQTABLE_CTRL_DEALLOC,mCurBurstQNum);

    // Tuning Queue
    #if defined(__PMEM_ONLY__)
        LOG_DBG("Free Tuning Queue");
        for(int i=0;i<ISP_DRV_P2_CQ_NUM;i++){
            for(int j=0;j<ISP_TUNING_QUEUE_NUM;j++){
                if((mTuningQueInf[i][j].queFd>=0) && (mTuningQueInf[i][j].pTuningQue!=0)){
                    pmem_free((MUINT8*)mTuningQueInf[i][j].pTuningQue,mTuningQueInf[i][j].queSize,mTuningQueInf[i][j].queFd);
                    mTuningQueInf[i][j].queFd = -1;
                    mTuningQueInf[i][j].pTuningQue = NULL;
                }else{
                    LOG_ERR("[Error]free TuningQue error i(%d),j(%d),fd(%d),va(0x%08x)",i,j,mTuningQueInf[i][j].queFd,mTuningQueInf[i][j].pTuningQue);
                }
            }
            // free keepP1Que
            if((mTuningQueIdx[i].keepP1Que.queFd>=0) && (mTuningQueIdx[i].keepP1Que.pTuningQue!=0)){
                pmem_free((MUINT8*)mTuningQueIdx[i].keepP1Que.pTuningQue,mTuningQueIdx[i].keepP1Que.queSize,mTuningQueIdx[i].keepP1Que.queFd);
                mTuningQueIdx[i].keepP1Que.queFd = -1;
                mTuningQueIdx[i].keepP1Que.pTuningQue = NULL;
            }else{
                LOG_ERR("[Error]free keepP1Que error i(%d),fd(%d),va(0x%08x)",i,mTuningQueIdx[i].keepP1Que.queFd,mTuningQueIdx[i].keepP1Que.pTuningQue);
            }
            mTuningQueIdx[i].isApplyTuning = MFALSE;
            mTuningQueIdx[i].pCurReadP1TuningQue = NULL;
            mTuningQueIdx[i].pCurWriteTuningQue = NULL;
            mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepReadP1CtlEn1 = 0;
            mTuningQueIdx[i].keepReadP1CtlEn2 = 0;
            mTuningQueIdx[i].keepReadP1CtlDmaEn = 0;
            mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
            mTuningQueIdx[i].curReadP1Idx = ISP_TUNING_INIT_IDX;
            //mTuningQueIdx[i].isInitP2 = MFALSE;
            mTuningQueIdx[i].isInitP1 = MFALSE;
        }
    #else
        LOG_DBG("Free Tuning Queue");
        for(int i=0;i<ISP_DRV_P2_CQ_NUM;i++){
            for(int j=0;j<ISP_TUNING_QUEUE_NUM;j++){
                if((mTuningQueInf[i][j].queFd>=0) && (mTuningQueInf[i][j].pTuningQue!=0)){
                    m_ispTmpBufInfo.size = mTuningQueInf[i][j].queSize;
                    m_ispTmpBufInfo.memID = mTuningQueInf[i][j].queFd;
                    m_ispTmpBufInfo.virtAddr = (MUINTPTR)mTuningQueInf[i][j].pTuningQue;
                    if ( m_pIMemDrv->freeVirtBuf(&m_ispTmpBufInfo) ) {
                        LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
                    }
                    mTuningQueInf[i][j].queFd = -1;
                    mTuningQueInf[i][j].pTuningQue = NULL;
                }else{
                    LOG_ERR("[Error]free TuningQue error i(%d),j(%d),fd(%d),va(0x%08x)",i,j,mTuningQueInf[i][j].queFd,mTuningQueInf[i][j].pTuningQue);
                }
            }
            // free keepP1Que
            if((mTuningQueIdx[i].keepP1Que.queFd>=0) && (mTuningQueIdx[i].keepP1Que.pTuningQue!=0)){
                m_ispTmpBufInfo.size = mTuningQueIdx[i].keepP1Que.queSize;
                m_ispTmpBufInfo.memID = mTuningQueIdx[i].keepP1Que.queFd;
                m_ispTmpBufInfo.virtAddr = (MUINTPTR)mTuningQueIdx[i].keepP1Que.pTuningQue;
                if ( m_pIMemDrv->freeVirtBuf(&m_ispTmpBufInfo) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
                }
                mTuningQueIdx[i].keepP1Que.queFd = -1;
                mTuningQueIdx[i].keepP1Que.pTuningQue = NULL;
            }else{
                LOG_ERR("[Error]free keepP1Que error i(%d),fd(%d),va(0x%08x)",i,mTuningQueIdx[i].keepP1Que.queFd,mTuningQueIdx[i].keepP1Que.pTuningQue);
            }

            mTuningQueIdx[i].isApplyTuning = MFALSE;
            mTuningQueIdx[i].pCurReadP1TuningQue = NULL;
            mTuningQueIdx[i].pCurWriteTuningQue = NULL;
            mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepReadP1CtlEn1 = 0;
            mTuningQueIdx[i].keepReadP1CtlEn2 = 0;
            mTuningQueIdx[i].keepReadP1CtlDmaEn = 0;
            mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
            //mTuningQueIdx[i].isInitP2 = MFALSE;
            mTuningQueIdx[i].isInitP1 = MFALSE;
        }
    #endif

    //
    //
    //IMEM
    m_pIMemDrv->uninit();
    LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->uninit()");
    m_pIMemDrv->destroyInstance();
    LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->destroyInstance()");
    m_pIMemDrv = NULL;
    //

    mpIspHwRegAddr = NULL;
    mIspVirRegFd = 0;
    mpIspVirRegBufferBaseAddr = NULL;
    mIspVirRegSize = 0;
    mIspCQDescFd = -1;
    mpIspCQDescBufferVirt = NULL;
    mIspCQDescSize = 0;
    mpIspCQDescBufferPhy = 0;
    //
    if(mFd >= 0)
    {
        //revert kernel flag
    CAM_HAL_VER_IS3 = 0;
    if(ioctl(mFd,ISP_SET_CAM_VERSION,&CAM_HAL_VER_IS3) < 0){
        LOG_ERR("SetCamVer error\n");
    }
    //
        #if defined(_use_kernel_ref_cnt_)
        ISP_REF_CNT_CTRL_STRUCT_FRMB ref_cnt;
        MINT32 count;   // For ISP global reference count.

        ref_cnt.ctrl = ISP_REF_CNT_DEC_FRMB;
        ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC_FRMB;
        ref_cnt.data_ptr = &count;
        //
        if ( MFALSE == kRefCntCtrl(&ref_cnt) ) {
            //
            LOG_ERR("ISP_REF_CNT_GET fail, errno(%d):%s.", errno, strerror(errno));
        }
        LOG_INF("ISP Global Count: %d.", count);
        #endif

        close(mFd);
        mFd = -1;
    }
    LOCAL_PROFILING_LOG_PRINT("close isp mFd");
    //
    if(mpIspVirRegMap != NULL)
    {
        free((MUINT32*)mpIspVirRegMap);
        mpIspVirRegMap = NULL;
    }
    LOCAL_PROFILING_LOG_PRINT("free(mpIspVirRegMap)");
    if(mpTempIspHWRegValues != NULL)
    {
        free((MUINT32*)mpTempIspHWRegValues);
        mpTempIspHWRegValues = NULL;
    }
    LOCAL_PROFILING_LOG_PRINT("free(mpTempIspHWRegValues)");

    //
EXIT:

    LOG_INF(" - X. ret: %d. mInitCount: %d.", Result, mInitCount);
    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::loadInitSetting(void)
{
    isp_reg_t ispReg;

    LOG_INF("+");

    #if 0  // fix it later
    //enable DMA error check
    writeReg((((UINT32)&ispReg.CAM_IMGI_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_BPCI_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_LSCI_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_UFDI_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_LCEI_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_VIPI_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_VIP2I_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_VIP3I_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_IMGO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_RRZO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_LCSO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_ESFKO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_AAO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_UFEO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_MFBO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_IMG3BO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_IMG3CO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_IMG2O_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_IMG3O_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_FEO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_BPCI_D_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_LSCI_D_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_IMGO_D_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_RRZO_D_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_LCSO_D_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_AFO_D_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg((((UINT32)&ispReg.CAM_AAO_D_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    //disable p2 DB
    writeReg((((UINT32)&ispReg.CAM_CTL_SEL_GLOBAL_P2-(UINT32)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
    //
    #endif

    LOG_INF("-");

    return MTRUE;
}

//-----------------------------------------------------------------------------
// burst Queue num are only supported in p2
MBOOL IspDrvImp::cqTableControl(ISP_DRV_CQTABLE_CTRL_ENUM cmd,int burstQnum)
{
    MUINTPTR dummyaddr;
    bool ret=true;
    IMEM_BUF_INFO ispTmpImemInfo;

    LOG_INF("cmd(%d),burstQnum(%d),mTotalCQNum(%d)",cmd,burstQnum,mTotalCQNum);

    switch(cmd)
    {
        case ISP_DRV_CQTABLE_CTRL_ALLOC:
            //[1] virtual isp(virtual register map)
            mIspVirRegSize = sizeof(MUINT32) * ISP_BASE_RANGE * mTotalCQNum;

            if(mpIspVirRegAddrPA!=NULL)
            {
                LOG_ERR("mpIspVirRegAddrPA not a null point");
                return -1;
            }
            if(mpIspVirRegAddrVA!=NULL)
            {
                LOG_ERR("mpIspVirRegAddrPA not a null point");
                return -1;
            }


#if defined(__PMEM_ONLY__)
            mpIspVirRegAddrPA=(MUINT32 **)pmem_alloc_sync((mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32)), &mIspVirRegAddrPAFd);
#else
            ispTmpImemInfo.size = mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32);
            ispTmpImemInfo.useNoncache = 1; //alloc non-cacheable mem.

            if ( m_pIMemDrv->allocVirtBuf(&ispTmpImemInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
                ret=false;
                break;
            }
            mIspVirRegAddrPAFd = ispTmpImemInfo.memID;
            mpIspVirRegAddrPA = (MUINT32**)ispTmpImemInfo.virtAddr;
#endif

            for( int i=0; i<mTotalCQNum; i++ )
            {
                mpIspVirRegAddrPA[i] = ((MUINT32 *)(mpIspVirRegAddrPA+mTotalCQNum)) + i*1*sizeof(MUINT32);
            }
            if(mpIspVirRegAddrPA==NULL)
            {
                LOG_ERR("malloc mpIspVirRegAddrPA fail");
                ret = false;
                break;
            }

#if defined(__PMEM_ONLY__)
            mpIspVirRegAddrVA=(MUINT32 **)pmem_alloc_sync((mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32)), &mIspVirRegAddrVAFd);
#else
            ispTmpImemInfo.size = mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32);
            ispTmpImemInfo.useNoncache = 1; //alloc non-cacheable mem.

            if ( m_pIMemDrv->allocVirtBuf(&ispTmpImemInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
                ret=false;
                break;
            }
            mIspVirRegAddrVAFd = ispTmpImemInfo.memID;
            mpIspVirRegAddrVA = (MUINT32**)ispTmpImemInfo.virtAddr;
#endif


            for( int i=0; i<mTotalCQNum; i++ )
            {
                mpIspVirRegAddrVA[i] = ((MUINT32 *)(mpIspVirRegAddrVA+mTotalCQNum)) + i*1*sizeof(MUINT32);
            }
            if(mpIspVirRegAddrVA==NULL)
            {
                LOG_ERR("malloc mpIspVirRegAddrVA fail");
                ret = false;
                break;
            }

            // allocation
            #if defined(__PMEM_ONLY__)
            mpIspVirRegBufferBaseAddr = (MUINT32*)pmem_alloc_sync((mIspVirRegSize+(ISP_DRV_VIR_ADDR_ALIGN+1)), &mIspVirRegFd);
            mpIspVirRegAddrPA[0] = (MUINT32*)pmem_get_phys(mIspVirRegFd);
            #else
            m_ispVirRegBufInfo.size = mIspVirRegSize + (ISP_DRV_VIR_ADDR_ALIGN+1);
            m_ispVirRegBufInfo.useNoncache = 1; //alloc non-cacheable mem.
            LOG_INF("m_ispVirRegBufInfo.size: %d.", m_ispVirRegBufInfo.size);
            //
            if ( m_pIMemDrv->allocVirtBuf(&m_ispVirRegBufInfo) ){
                LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
                ret=false;
                break;
            }
            #endif
            mIspVirRegFd = m_ispVirRegBufInfo.memID;
            //virtual isp base adrress should be 4 bytes alignment
            mpIspVirRegBufferBaseAddr = (MUINT32*)( (m_ispVirRegBufInfo.virtAddr + ISP_DRV_VIR_ADDR_ALIGN) & (~ISP_DRV_VIR_ADDR_ALIGN) );
            //
            if ( m_pIMemDrv->mapPhyAddr(&m_ispVirRegBufInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
                ret=false;
                break;
            }
            //virtual isp base adrress should be 4 bytes alignment
            mpIspVirRegAddrPA[0] =  (MUINT32*)( (m_ispVirRegBufInfo.phyAddr + ISP_DRV_VIR_ADDR_ALIGN)  & (~ISP_DRV_VIR_ADDR_ALIGN) );
            LOG_VRB("v(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)",mIspVirRegSize,m_ispVirRegBufInfo.size, \
                                                            m_ispVirRegBufInfo.virtAddr,m_ispVirRegBufInfo.phyAddr, \
                                                            mpIspVirRegBufferBaseAddr,mpIspVirRegAddrPA[0]);

            //
            if(mpIspVirRegBufferBaseAddr == NULL)
            {
                LOG_ERR("mem alloc fail, size(%d)",mIspVirRegSize);
                ret = false;
                break;
            }

            // initialization
            memset((MUINT8*)mpIspVirRegBufferBaseAddr,ISP_DRV_VIR_DEFAULT_DATA,mIspVirRegSize);
            for (int i = 0; i < mTotalCQNum; i++) {
                mpIspVirRegAddrVA[i] = mpIspVirRegBufferBaseAddr + i * ISP_BASE_RANGE;
                mpIspVirRegAddrPA[i] = mpIspVirRegAddrPA[0] + i * ISP_BASE_RANGE;
                LOG_INF("cq:[%d],virtIspAddr:virt[0x%08x]/phy[0x%08x]",i,mpIspVirRegAddrVA[i], mpIspVirRegAddrPA[i] );
            }


            //[3]CQ descriptor
            if(mpIspCQDescriptorVirt!=NULL)
            {
                free(mpIspCQDescriptorVirt);
            }
            if(mpIspCQDescriptorVirt!=NULL)
            {
                free(mpIspCQDescriptorPhy);
            }
            mpIspCQDescriptorVirt=(ISP_DRV_CQ_CMD_DESC_STRUCT **)malloc(mTotalCQNum*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT *)+
                                    mTotalCQNum*1*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
            for( int i=0; i<mTotalCQNum; i++ )
            {
                mpIspCQDescriptorVirt[i] = ((ISP_DRV_CQ_CMD_DESC_STRUCT *)(mpIspCQDescriptorVirt+mTotalCQNum)) + i*1*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT);
            }
            if(mpIspCQDescriptorVirt==NULL)
            {
                LOG_ERR("malloc mpIspCQDescriptorVirt fail");
                ret = false;
                break;
            }
            mpIspCQDescriptorPhy=(MUINT32 **)malloc(mTotalCQNum*sizeof(MUINT32*));
            if(mpIspCQDescriptorPhy==NULL)
            {
                LOG_ERR("malloc mpIspCQDescriptorVirt fail");
                ret = false;
                break;
            }

            mIspCQDescSize = sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT) * mTotalCQNum * CAM_MODULE_MAX ; //
            // allocation
            #if defined(__PMEM_ONLY__)
            mpIspCQDescBufferVirt = (MUINT32*)pmem_alloc_sync( (mIspCQDescSize+ (ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN+1)), &mIspCQDescFd);
            mpIspCQDescBufferPhy = (MUINT32)pmem_get_phys(mIspCQDescFd);
            #else
            //
            m_ispCQDescBufInfo.size = mIspCQDescSize+ (ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN+1);
            m_ispCQDescBufInfo.useNoncache = 1; //alloc non-cacheable mem.
            if ( m_pIMemDrv->allocVirtBuf(&m_ispCQDescBufInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
                ret=false;
                break;
            }
            mIspCQDescFd = m_ispCQDescBufInfo.memID;
            //CQ decriptor base address should be 8 bytes alignment
            mpIspCQDescBufferVirt = (MUINT32*)( (m_ispCQDescBufInfo.virtAddr+ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) & (~ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) );
            if ( m_pIMemDrv->mapPhyAddr(&m_ispCQDescBufInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
                ret=false;
                break;
            }
            //CQ decriptor base address should be 8 bytes alignment
            mpIspCQDescBufferPhy = (MUINT32*) ( (m_ispCQDescBufInfo.phyAddr+ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) & (~ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) );

            LOG_DBG("cq(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)",mIspCQDescSize,m_ispCQDescBufInfo.size, \
                                                            m_ispCQDescBufInfo.virtAddr,m_ispCQDescBufInfo.phyAddr, \
                                                            mpIspCQDescBufferVirt,mpIspCQDescBufferPhy);
            #endif  // __PMEM_ONLY__

            // initialization
            dummyaddr =(MUINTPTR)((MUINTPTR)mpIspVirRegAddrPA[0] + ISP_DRV_CQ_DUMMY_WR_TOKEN);
            for (int i = 0; i < (CAM_MODULE_MAX-1); i++) {
              mIspCQDescInit[i].v_reg_addr =(MUINT32)dummyaddr & 0xFFFFFFFF;      //why use mpIspVirRegAddrPA[0] for all initialization
            }
            for (int i = 0; i < mTotalCQNum; i++)
            {
                if ( 0 == i ) {
                    mpIspCQDescriptorVirt[i] = (ISP_DRV_CQ_CMD_DESC_STRUCT*)mpIspCQDescBufferVirt;
                    mpIspCQDescriptorPhy[i] = mpIspCQDescBufferPhy;
                }
                else {
                    mpIspCQDescriptorVirt[i] = (ISP_DRV_CQ_CMD_DESC_STRUCT *)((unsigned long)mpIspCQDescriptorVirt[i-1] + sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_MODULE_MAX);
                    mpIspCQDescriptorPhy[i] = (MUINT32*)(( (unsigned long)mpIspCQDescriptorPhy[i-1]) + (sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_MODULE_MAX));
                }
                memcpy((MUINT8*)mpIspCQDescriptorVirt[i], mIspCQDescInit, sizeof(mIspCQDescInit));
                LOG_INF("cq:[%d],mpIspCQDescriptor:Virt[0x%08x]/Phy[0x%08x],size/num(%d/%d/%d)",i,mpIspCQDescriptorVirt[i],mpIspCQDescriptorPhy[i],sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT),CAM_MODULE_MAX,sizeof(mIspCQDescInit));
            }

            #if defined(_rtbc_use_cq0c_)
            //reset cq0c all 0 for rtbc
            if (sizeof(CQ_RTBC_RING_ST_FRMB) <= sizeof(mIspCQDescInit)) {
                memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C], 0, sizeof(CQ_RTBC_RING_ST_FRMB));
            }
            else {
                LOG_ERR("rtbc data too large(%d)>(%d)",sizeof(CQ_RTBC_RING_ST_FRMB),sizeof(mIspCQDescInit));
                   return MFALSE;
            }
            #endif  // _rtbc_use_cq0c_
            break;
        case ISP_DRV_CQTABLE_CTRL_DEALLOC:
            //virtual CQ
            if ( 0 != mpIspVirRegBufferBaseAddr )
            {
                #if defined(__PMEM_ONLY__)
                if(mIspVirRegFd >= 0) {
                    pmem_free((MUINT8*)mpIspVirRegBufferBaseAddr,mIspVirRegSize,mIspVirRegFd);
                    mIspVirRegFd = -1;
                }
                #else
                if ( m_pIMemDrv->unmapPhyAddr(&m_ispVirRegBufInfo) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
                }
                //
                if ( m_pIMemDrv->freeVirtBuf(&m_ispVirRegBufInfo) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
                }
                LOG_DBG("free/unmap mpIspVirRegBuffer");
                #endif  //__PMEM_ONLY__
                mpIspVirRegBufferBaseAddr = NULL;
            }
            //
#if defined(__PMEM_ONLY__)
            if(mIspVirRegAddrPAFd >= 0) {
                pmem_free((MUINT8*)mpIspVirRegAddrPA,mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32),mIspVirRegAddrPAFd);
                mIspVirRegAddrPAFd = -1;
            }
            mIspVirRegAddrPAFd = NULL;
            //
            if(mIspVirRegAddrVAFd >= 0) {
                pmem_free((MUINT8*)mpIspVirRegAddrVA,mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32),mIspVirRegAddrVAFd);
                mIspVirRegAddrVAFd = -1;
            }
            mIspVirRegAddrVAFd = NULL;
#else
            ispTmpImemInfo.size = mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32);
            ispTmpImemInfo.memID = mIspVirRegAddrPAFd;
            ispTmpImemInfo.virtAddr = (MUINTPTR)mpIspVirRegAddrPA;
            if ( m_pIMemDrv->freeVirtBuf(&ispTmpImemInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
            }
            mIspVirRegAddrPAFd = NULL;
            mpIspVirRegAddrPA = NULL;
            //
            ispTmpImemInfo.size = mTotalCQNum*sizeof(MUINT32 *)+mTotalCQNum*1*sizeof(MUINT32);
            ispTmpImemInfo.memID = mIspVirRegAddrVAFd;
            ispTmpImemInfo.virtAddr = (MUINTPTR)mpIspVirRegAddrVA;
            if ( m_pIMemDrv->freeVirtBuf(&ispTmpImemInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
            }
            mIspVirRegAddrVAFd = NULL;
            mpIspVirRegAddrVA = NULL;
#endif

            //descriptor
            if( 0 != mpIspCQDescBufferVirt )
            {

                #if defined(__PMEM_ONLY__)
                if(mIspCQDescFd >= 0) {
                    pmem_free((MUINT8*)mpIspCQDescBufferVirt,mIspCQDescSize,mIspCQDescFd);
                    mIspCQDescFd = -1;
                }
                #else
                if ( m_pIMemDrv->unmapPhyAddr(&m_ispCQDescBufInfo) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
                }
                //
                if ( m_pIMemDrv->freeVirtBuf(&m_ispCQDescBufInfo) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
                }
                LOG_DBG("free/unmap mpIspCQDescBufferVirt");
                #endif
                mpIspCQDescBufferVirt = NULL;
            }
            //
            if(mpIspCQDescriptorVirt!=NULL)
            {
                free(mpIspCQDescriptorVirt);
                mpIspCQDescriptorVirt=NULL;
            }
            if(mpIspCQDescriptorPhy!=NULL)
            {
                free(mpIspCQDescriptorPhy);
                mpIspCQDescriptorPhy=NULL;
            }
            break;
         default:
            LOG_ERR("wrong cmd(%d)",cmd);
            ret=false;
            break;
    }

    return ret;
}
//-----------------------------------------------------------------------------
MINT32 IspDrvImp::cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_ENUM cmd,int newValue)
{
    MINT32 num=0;
    MBOOL ret=MTRUE;
    MUINT32 value;
    pthread_mutex_lock(&IspCQinfoMutex);
    switch(cmd)
    {
        case ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM:   //get current supported burst queue number
            #if 1
            num = 1;   // for 82
            #else
            //hint: burstQnum may not sync in user/kernel space cuz mw flow that user space thread all release isp driver
            //       but imem driver may not be released (some buffer would be keeped)
            //action, all burstQ number are based on user space
            ret = ioctl(mFd,ISP_QUERY_BURSTQNUM_FRMB,&value);
            if(ret<0)
            {
                LOG_ERR("ISP_QUERY_BURSTQNUM_FRMB fail(%d).", ret);
            }
            if(value != mCurBurstQNum)
            {
                LOG_WRN("update kernel bQ cuz timing(%d,%d)",mCurBurstQNum,value);
                value=mCurBurstQNum;
                ret=ioctl(mFd,ISP_UPDATE_BURSTQNUM_FRMB,&value);
                if(ret<0)
                {
                    LOG_ERR("ISP_UPDATE_BURSTQNUM_FRMB fail(%d).", ret);
                }
            }
            num=mCurBurstQNum;
            #endif
            break;
        case ISP_DRV_CQNUMINFO_CTRL_GET_TOTALCQNUM:          //get total cq number including p1 cq and p2 cq
            num=mTotalCQNum;
            break;
        case ISP_DRV_CQNUMINFO_CTRL_GET_P2DUPCQNUM:          //get duplicate number of p2 cq
            num=ISP_DRV_P2_CQ_DUPLICATION_NUM;
            break;
        case ISP_DRV_CQNUMINFO_CTRL_GET_P1DUPCQNUM:          //get duplicate number of p2 cq
            num=ISP_DRV_P1_CQ_DUPLICATION_NUM;
            break;
        case ISP_DRV_CQNUMINFO_CTRL_SET_CURBURSTQNUM:   //set current supported burst queue number
            /* do nothing in 82 */
            if(newValue>1)
            {
                LOG_ERR("do not support burstQ number > 1 ");
            }
            #if 0
            mCurBurstQNum=newValue;
            //update value in kernel layer
            value=newValue;
            ret=ioctl(mFd,ISP_UPDATE_BURSTQNUM_FRMB,&value);
            if(ret<0)
            {
                LOG_ERR("ISP_UPDATE_BURSTQNUM_FRMB fail(%d).", ret);
            }
            #endif
            break;
        case ISP_DRV_CQNUMINFO_CTRL_UPDATE_TOTALCQNUM:          //update total cq number including p1 cq and p2 cq
            mTotalCQNum= mCurBurstQNum * ((ISP_DRV_P1_CQ_DUPLICATION_NUM * ISP_DRV_P1_PER_CQ_SET_NUM)\
            +(ISP_DRV_P2_CQ_DUPLICATION_NUM * ISP_DRV_P2_PER_CQ_SET_NUM));
            break;
        case ISP_DRV_CQNUMINFO_CTRL_SET_P1DUPCQNUM:          //set duplicate number of p1 cq
        case ISP_DRV_CQNUMINFO_CTRL_SET_P2DUPCQNUM:          //set duplicate number of p2 cq
            break;
        default:
            LOG_WRN("wrong cmd");
            break;
    }
    pthread_mutex_unlock(&IspCQinfoMutex);
    return num;
}


//-----------------------------------------------------------------------------
MBOOL IspDrvImp::dumpP2DebugLog(IspDumpDbgLogP2Package* pP2Package)
{
    isp_reg_t ispReg;
    MUINT32 i;
    MUINT32 dumpTpipeLine;
    MUINT32 regTpipePA;
    //isp_reg_t tmpIspReg;

    LOG_INF("pP2Package(0x%x)",pP2Package);
    //
    //dump tpipe
    regTpipePA = readReg((MUINT32)((MUINT8*)&ispReg.CAM_TDRI_BASE_ADDR-(MUINT8*)&ispReg));
    LOG_INF("[Tpipe]va(0x%08x),pa(0x%08x),regPa(0x%08x)",pP2Package->tpipeTableVa,pP2Package->tpipeTablePa,regTpipePA);
    dumpTpipeLine = DUMP_TPIPE_SIZE / DUMP_TPIPE_NUM_PER_LINE;
    for(i=0;i<dumpTpipeLine;i++){
        LOG_INF("[Tpipe](%02d)-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",i,
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+1],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+2],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+3],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+4],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+5],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+6],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+7],pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+8],
            pP2Package->tpipeTableVa[i*DUMP_TPIPE_NUM_PER_LINE+9]);
    }

    //dump isp register
    for(i=0x4000;i<=0x5000;i+=20){
        LOG_INF("(0x%04x,0x%08x)(0x%04x,0x%08x)(0x%04x,0x%08x)(0x%04x,0x%08x)(0x%04x,0x%08x)",
            i,readReg(i, ISP_DRV_USER_ISPF),i+4,readReg(i+4, ISP_DRV_USER_ISPF),i+8,readReg(i+8, ISP_DRV_USER_ISPF),i+12,readReg(i+12, ISP_DRV_USER_ISPF),i+16,readReg(i+16, ISP_DRV_USER_ISPF));
    }
    //compare the difference between isp reg and tpipe
    if(pP2Package->pIspReg || pP2Package->pIspTpipeCfgInfo){
        LOG_INF("start to check the defference between tpipe and hw reg");
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.scenario           ,pP2Package->pIspReg,CAM_CTL_FMT_SEL,SCENARIO);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.mode               ,pP2Package->pIspReg,CAM_CTL_FMT_SEL,SUB_MODE);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.cam_in_fmt         ,pP2Package->pIspReg,CAM_CTL_FMT_SEL,CAM_IN_FMT);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.tcm_load_en        ,pP2Package->pIspReg,CAM_CTL_TCM_EN ,TCM_LOAD_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.ctl_extension_en   ,pP2Package->pIspReg,CAM_CTL_PIX_ID ,CTL_EXTENSION_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.rsp_en             ,pP2Package->pIspReg,CAM_CTL_TCM_EN ,TCM_RSP_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.mdp_crop_en        ,pP2Package->pIspReg,CAM_CTL_CROP_X ,MDP_CROP_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.imgi_en            ,pP2Package->pIspReg,CAM_CTL_DMA_EN ,IMGI_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.lsci_en            ,pP2Package->pIspReg,CAM_CTL_DMA_EN ,LSCI_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.unp_en             ,pP2Package->pIspReg,CAM_CTL_EN1    ,UNP_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.bnr_en             ,pP2Package->pIspReg,CAM_CTL_EN1    ,BNR_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.lsc_en             ,pP2Package->pIspReg,CAM_CTL_EN1    ,LSC_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.sl2_en             ,pP2Package->pIspReg,CAM_CTL_EN1    ,SL2_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.c24_en             ,pP2Package->pIspReg,CAM_CTL_EN1    ,C24_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.cfa_en             ,pP2Package->pIspReg,CAM_CTL_EN1    ,CFA_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.c42_en             ,pP2Package->pIspReg,CAM_CTL_EN2    ,C42_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.nbc_en             ,pP2Package->pIspReg,CAM_CTL_EN2    ,NBC_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.seee_en            ,pP2Package->pIspReg,CAM_CTL_EN2    ,SEEE_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.imgo_en            ,pP2Package->pIspReg,CAM_CTL_DMA_EN ,IMGO_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.img2o_en           ,pP2Package->pIspReg,CAM_CTL_DMA_EN ,IMG2O_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.cdrz_en            ,pP2Package->pIspReg,CAM_CTL_EN2    ,CDRZ_EN);
        CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.mdp_sel            ,pP2Package->pIspReg,CAM_CTL_PIX_ID ,MDP_SEL);
        //CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.pixel_id,pP2Package->pIspReg,CAM_CTL_FMT_SEL,SUB_MODE);
        //CHECH_TPIPE_REG(pP2Package->pIspTpipeCfgInfo,top.interlace_mode     ,pP2Package->pIspReg,CAM_CTL_EN1,SUB_MODE);
        LOG_INF("end to check the defference between tpipe and hw reg");


        #if 0
        if(pP2Package->pIspTpipeCfgInfo->top.scenario != (int)pP2Package->pIspReg->CAM_CTL_FMT_SEL.Bits.SUB_MODE){
            // backup the original value
            oriVal = pP2Package->pIspReg->CAM_CTL_FMT_SEL.Raw;
            // start to parse union bits
            pP2Package->pIspReg->CAM_CTL_FMT_SEL.Raw = 0;
            pP2Package->pIspReg->CAM_CTL_FMT_SEL.Bits.SUB_MODE = 0xffffffff;
            parseVal = pP2Package->pIspReg->CAM_CTL_FMT_SEL.Raw;
            for(i=0;i<intSize;i++){
                if(parseVal & (0x01)<<i)
                    break;
            }
            // recover the original value
            pP2Package->pIspReg->CAM_CTL_FMT_SEL.Raw = oriVal;

            LOG_INF("[Diff][0x%x][Bit-%d]", \
                    (int)(&pP2Package->pIspReg->CAM_CTL_FMT_SEL)-(int)(&pP2Package->pIspReg->rsv_0000[0]), \
                    i);
        }
        #endif

    } else {
        LOG_ERR("[Error] fail to dump tpipe difference");
    }

    //LOG_INF("(0x%04x,0x%08x)(0x%04x,0x%08x)",0x5ff0,readReg(0x5ff0, ISP_DRV_USER_ISPF),0x5ff4,readReg(0x5ff4, ISP_DRV_USER_ISPF));


    return MTRUE;
}

#define DMA_CHK(dma,out){\
    out = 0;\
    if(dma[0] & 0xffff){\
        LOG_ERR("IMGI ERR:0x%x\n",dma[0]);\
        out = 1;\
    }\
    if(dma[1] & 0xffff){\
        LOG_ERR("LSCI ERR:0x%x\n",dma[1]);\
        out = 1;\
    }\
    if(dma[2] & 0xffff){\
        LOG_ERR("IMGO ERR:0x%x\n",dma[2]);\
        out = 1;\
    }\
    if(dma[3] & 0xffff){\
        LOG_ERR("IMG2O ERR:0x%x\n",dma[3]);\
        out = 1;\
    }\
    if(dma[4] & 0xffff){\
        LOG_ERR("ESFKO ERR:0x%x\n",dma[4]);\
        out = 1;\
    }\
    if(dma[5] & 0xffff){\
        LOG_ERR("AAO ERR:0x%x\n",dma[5]);\
        out = 1;\
    }\
}

//idx0:0 for p1 , 1 for p1_d , 2 for camsv, 3 for camsv_d
//idx1:debug flag init.    MTRUE/FALSE, if idx1 is true , idx2  map to function_DbgLogEnable_VERBOSE,
//                                       idx3:function_DbgLogEnable_DEBUG, idx4 map to function_DbgLogEnable_INFO
//idx2:prt log.                MTRUE/FALSE
//idx3:get sof
//idx4:get dma_err msg
//idx5:1 for get int err
//idx5:2 for get drop frame status, which idx0: in/out param, in for TG, out for frame status,0:normal, 1:drop frame, 2:last working frame
MBOOL IspDrvImp::dumpP1DebugLog(MUINT32* p1)
{
    if(mFd >= 0 ){
        MUINT32 _flag[2] = {0x0};
        //enable kernel debug log
        if(p1[1] == MTRUE){
            if(p1[2]){
                _flag[0] |= 0x400;//ISP_DBG_INT_2
            }
            if(p1[3]){//(isp_drv_DbgLogEnable_DEBUG){
                _flag[0] |= 0x1;//ISP_DBG_INT
            }
            if(p1[4]){
                _flag[0] |= 0x100;//ISP_DBG_BUF_CTRL
            }
            if(p1[5])
                _flag[0] |= 0x800;//isp_dbg_int_3 //timestamp interval
            _flag[1] = p1[0];//_IRQ
            if(ioctl(mFd,ISP_DEBUG_FLAG_FRMB,_flag) < 0){
                LOG_ERR("kernel log enable error\n");
            }
        }
        if(p1[2] == MTRUE){
            _flag[0] = p1[0];
            LOG_DBG("prt kernel log");
            if(ioctl(mFd,ISP_DUMP_ISR_LOG_FRMB,_flag) < 0){
                LOG_ERR("kernel log enable error\n");
            }
        }
        else if(p1[3] == MTRUE){
            if(ioctl(mFd,ISP_GET_CUR_SOF_FRMB,p1) < 0){
                LOG_ERR("dump sof error\n");
            }
        }
        else if(p1[4] == MTRUE){
            MUINT32 _flag[18] = {0};
            if(ioctl(mFd,ISP_GET_DMA_ERR_FRMB,_flag) < 0){
                LOG_ERR("dump sof error\n");
            }
            DMA_CHK(_flag,p1[4]);
        }
        else if(p1[5] == 1){
            MUINT32 _flag[4] = {0};
            if(ioctl(mFd,ISP_GET_INT_ERR_FRMB,_flag) < 0){
                LOG_ERR("dump int_err error\n");
            }else
                LOG_ERR("p1 int err: 0x%x,0x%x,0x%x,0x%x",_flag[0],_flag[1],_flag[2],_flag[3]);
        }
        else if(p1[5] == 2){
            if(ioctl(mFd,ISP_GET_DROP_FRAME_FRMB,p1) < 0){
                LOG_ERR("dump drop frame status error\n");
            }
        }
        else{
            if(p1[1]!= MTRUE)
                LOG_ERR("p1 dump log err");
        }
    }
    return MTRUE;
}



MBOOL IspDrvImp::dumpDBGLog(MUINT32* P1,IspDumpDbgLogP2Package* pP2Package){
    if(pP2Package)
        dumpP2DebugLog(pP2Package);
    if(P1!= NULL)
        dumpP1DebugLog(P1);
    return MTRUE;
}

MBOOL IspDrvImp::SetFPS(MUINT32 _fps){
#ifdef T_STAMP_2_0
    if(ioctl(mFd,ISP_SET_FPS_FRMB,&_fps) < 0){
        LOG_ERR("SetFPS error\n");
    }
    return MTRUE;
#else
    LOG_DBG("FPS can't pass 2 kernel\n");
    return MFALSE;
#endif
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::dumpCQTable(ISP_DRV_CQ_ENUM cq, MINT32 burstQIdx,MUINT32 dupCqIdx, ISP_DRV_CQ_CMD_DESC_STRUCT*  cqDesVa, MUINT32* cqVirVa)
{
    MUINT32 i;
    MUINT32 cqLabel;
    ISP_DRV_DESCRIPTOR_CQ_ENUM cqDesTab;
    MUINT32 *pIspVirCqVa;
    ISP_DRV_CQ_CMD_DESC_STRUCT *pIspDesCqVa;
    isp_reg_t ispReg;
    MINT32 realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);

    LOG_INF("+,realCQIdx(%d),cq(%d),burstQIdx(%d),dupCqIdx(%d),cqDesVa(0x%x),cqVirVa(0x%x)",realCQIdx,cq,burstQIdx,dupCqIdx,cqDesVa,cqVirVa);

    switch(cq){
        case ISP_DRV_CQ0:
            cqLabel=0x0;
            break;
        case ISP_DRV_CQ0B:
            cqLabel=0x0B;
            break;
        case ISP_DRV_CQ0C:
            cqLabel=0x0C;
            break;
        case ISP_DRV_CQ01:
            cqLabel=0x01;
            break;
        case ISP_DRV_CQ02:
            cqLabel=0x02;
            break;
        case ISP_DRV_CQ03:
            cqLabel=0x03;
            break;
        default:
            LOG_ERR("[Error]Not support this CQ(%d)",realCQIdx);
              return MTRUE;
            break;
    }


    cqDesTab=(ISP_DRV_DESCRIPTOR_CQ_ENUM)(realCQIdx);
    if(cqDesVa==0 || cqVirVa==0){
        pIspDesCqVa = mpIspCQDescriptorVirt[cqDesTab];
        pIspVirCqVa = mpIspVirRegAddrVA[realCQIdx];
        LOG_INF("ISP_CQ_PA(0x%x)",mpIspCQDescriptorPhy[realCQIdx]);
        LOG_INF("[BasePA]CQ1(0x%x),CQ2(0x%x),CQ3(0x%x)",readReg(( (MUINT32)((MUINT8*)&ispReg.CAM_CTL_CQ1_BASEADDR-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF),
            readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_CQ2_BASEADDR-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF),readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_CQ3_BASEADDR-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
//        LOG_INF("[REG]P2_EN(0x%x),P2_EN_DMA(0x%x),SCENARIO_REG(0x%x),CQ_EN_P2(0x%x)",readReg((((UINT32)&ispReg.CAM_CTL_EN_P2-(UINT32)&ispReg)), ISP_DRV_USER_ISPF),
//            readReg((((UINT32)&ispReg.CAM_CTL_EN_P2_DMA-(UINT32)&ispReg)), ISP_DRV_USER_ISPF),readReg((((UINT32)&ispReg.CAM_CTL_SCENARIO-(UINT32)&ispReg)), ISP_DRV_USER_ISPF),
//            readReg((((UINT32)&ispReg.CAM_CTL_SEL_P2-(UINT32)&ispReg)), ISP_DRV_USER_ISPF));
    } else { // for dpframe work process
        pIspDesCqVa = (ISP_DRV_CQ_CMD_DESC_STRUCT*)cqDesVa;
        pIspVirCqVa = (MUINT32*)cqVirVa;
    }

    for(i=0;i<=CAM_DUMMY_;i++){
        if(pIspDesCqVa[i].u.cmd != ISP_DRV_CQ_DUMMY_WR_TOKEN) {
            LOG_INF("[CQ%03X]:[%02d][0x%08x]",cqLabel,i,pIspDesCqVa[i].u.cmd);
            LOG_CQ_VIRTUAL_TABLE(pIspVirCqVa,i,mIspCQModuleInfo[i].reg_num);
        }
    }

    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL IspDrvImp::waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pwaitIrq)
{
    MINT32 Ret;
#if 1
    MUINT32 dumpStatus = (ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST| \
        ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST| \
        ISP_DRV_IRQ_INT_STATUS_VS1_ST);
#else  // fix it later
    MUINT32 dumpStatus = (CAM_CTL_INT_P1_STATUS_PASS1_DON_ST|CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST| \
    CAMSV2_INT_STATUS_PASS1_DON_ST|CAMSV_INT_STATUS_PASS1_DON_ST|CAM_CTL_INT_P1_STATUS_SOF1_INT_ST|CAM_CTL_INT_P1_STATUS_D_SOF1_INT_ST| \
    CAMSV_INT_STATUS_TG_SOF1_ST|CAMSV2_INT_STATUS_TG_SOF1_ST|CAM_CTL_INT_P1_STATUS_VS1_INT_ST|CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST);
#endif
    //
    IspDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    //
    LOG_DBG(" - E. Type(%d),Status(0x%08x),Timeout(%d).", pwaitIrq->UserInfo.Type, pwaitIrq->UserInfo.Status, pwaitIrq->Timeout);
    //
    ISP_DRV_WAIT_IRQ_STRUCT waitirq;
    waitirq.Clear=pwaitIrq->Clear;
    waitirq.UserInfo=pwaitIrq->UserInfo;
    waitirq.TimeInfo.tLastEvent_sec=0;
    waitirq.TimeInfo.tLastEvent_usec=0;
    waitirq.TimeInfo.tmark2read_sec=0;
    waitirq.TimeInfo.tmark2read_usec=0;
    waitirq.TimeInfo.tevent2read_sec=0;
    waitirq.TimeInfo.tevent2read_usec=0;
    waitirq.TimeInfo.passedbySigcnt=0;
    waitirq.Timeout=pwaitIrq->Timeout;
    waitirq.EisMeta.tLastSOF2P1done_sec=0;
    waitirq.EisMeta.tLastSOF2P1done_usec=0;
    waitirq.SpecUser=pwaitIrq->SpecUser;
    //dump only pass1|pass1_d|camsv|camsv2  sof|done
    if((pwaitIrq->bDumpReg == 0xfe) || ((waitirq.UserInfo.Status & dumpStatus) == 0))
        waitirq.bDumpReg = MFALSE;
    else
        waitirq.bDumpReg = MTRUE;

    Ta=dbgTmr.getUs();
    Ret = ioctl(mFd,ISP_WAIT_IRQ_FRMB,&waitirq);
    Tb=dbgTmr.getUs();    //us
    //receive restart system call signal
    if(Ret== (-512))//-ERESTARTSYS=-512
    {    //change to non-clear wait and start to wait again
        LOG_INF("ioctrl Ret(%d),Type(%d),Status(0x%08x),Timeout(%d),Tb-Ta(%d us).",Ret,pwaitIrq->UserInfo.Type, pwaitIrq->UserInfo.Status, pwaitIrq->Timeout,Tb-Ta);
        waitirq.Timeout=waitirq.Timeout - ((Tb-Ta)/1000);
        while((waitirq.Timeout > 0) && (Ret== (-512)))//receive restart system call again, -ERESTARTSYS=-512
        {
             waitirq.Clear=ISP_DRV_IRQ_CLEAR_NONE;
             Ta=dbgTmr.getUs();
                Ret = ioctl(mFd,ISP_WAIT_IRQ_FRMB,&waitirq);
            Tb=dbgTmr.getUs();
                waitirq.Timeout=waitirq.Timeout - ((Tb-Ta)/1000);
            }
            LOG_INF("Leave ERESTARTSYS,Type(%d),Status(0x%08x),Timeout(%d),Tb-Ta(%d us).",Ret,pwaitIrq->UserInfo.Type, pwaitIrq->UserInfo.Status, pwaitIrq->Timeout,Tb-Ta);
    }
    //err handler
    if(Ret<0 || MTRUE==isp_drv_DbgLogEnable_DEBUG)
    {
        LOG_INF("Type(%d),Status(0x%08x),Timeout(%d).", pwaitIrq->UserInfo.Type, pwaitIrq->UserInfo.Status, pwaitIrq->Timeout);
        if(pwaitIrq->UserInfo.Type==4 && (pwaitIrq->UserInfo.Status==0x80||pwaitIrq->UserInfo.Status==0x40||pwaitIrq->UserInfo.Status==0x20||pwaitIrq->UserInfo.Status==0x2)){
            dumpCQTable(ISP_DRV_CQ0);
            if(pwaitIrq->UserInfo.Status==0x2 || pwaitIrq->UserInfo.Status==0x20){
                dumpCQTable(ISP_DRV_CQ01,0,0);
                dumpCQTable(ISP_DRV_CQ01,0,1);
            }else if(pwaitIrq->UserInfo.Status==0x40){
                dumpCQTable(ISP_DRV_CQ02,0,0);
                dumpCQTable(ISP_DRV_CQ02,0,1);
            }else if(pwaitIrq->UserInfo.Status==0x80){
                dumpCQTable(ISP_DRV_CQ03,0,0);
                dumpCQTable(ISP_DRV_CQ03,0,1);
            }
        }
    }
    if(Ret < 0) {
        LOG_ERR("ISP_WAIT_IRQ_FRMB fail(%d). Type(%d), Status(0x%08x), Timeout(%d).", Ret, pwaitIrq->UserInfo.Type, pwaitIrq->UserInfo.Status, pwaitIrq->Timeout);
        return MFALSE;
    }

    //return information
    pwaitIrq->TimeInfo=waitirq.TimeInfo;
    pwaitIrq->EisMeta=waitirq.EisMeta;
    if(waitirq.TimeInfo.tLastEvent_sec < 0)
    { pwaitIrq->TimeInfo.tLastEvent_sec=0;}
    if(waitirq.TimeInfo.tLastEvent_usec < 0)
    { pwaitIrq->TimeInfo.tLastEvent_usec=0;}
    if(waitirq.TimeInfo.tmark2read_sec < 0)
    { pwaitIrq->TimeInfo.tmark2read_sec=0;}
    if(waitirq.TimeInfo.tmark2read_usec < 0)
    { pwaitIrq->TimeInfo.tmark2read_usec=0;}
    if(waitirq.TimeInfo.tevent2read_sec < 0)
    { pwaitIrq->TimeInfo.tevent2read_sec=0;}
    if(waitirq.TimeInfo.tevent2read_usec < 0)
    { pwaitIrq->TimeInfo.tevent2read_usec=0;}
    if(waitirq.EisMeta.tLastSOF2P1done_sec < 0)
    { pwaitIrq->EisMeta.tLastSOF2P1done_sec=0;}
    if(waitirq.EisMeta.tLastSOF2P1done_usec < 0)
    { pwaitIrq->EisMeta.tLastSOF2P1done_usec=0;}
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::readIrq(ISP_DRV_READ_IRQ_STRUCT* pReadIrq)
{
    MINT32 Ret;
    #if 0
    ISP_DRV_READ_IRQ_STRUCT ReadIrq;
    //
    LOG_INF(" - E. Type(%d), Status(%d)",pReadIrq->Type, pReadIrq->Status);
    //
    ReadIrq.Type = pReadIrq->Type;
    ReadIrq.Status = (ISP_DRV_IRQ_USER_ENUM)0;
    //
    Ret = ioctl(mFd,ISP_READ_IRQ,&ReadIrq);
    if(Ret < 0)
    {
        LOG_ERR("ISP_READ_IRQ fail(%d)",Ret);
        return MFALSE;
    }
    //
    pReadIrq->Status = ReadIrq.Status;
    #endif
    LOG_ERR("NOT SUPPORT THIS");
    return MFALSE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq)
{
    MINT32 Ret;
    #if 0
    ISP_DRV_READ_IRQ_STRUCT ReadIrq;
    //
    LOG_DBG(" - E. Type(%d), Status(%d)",CheckIrq.Type, CheckIrq.Status);
    //
    ReadIrq.Type = CheckIrq.Type;
    ReadIrq.Status = (ISP_DRV_IRQ_USER_ENUM)0;
    if(!readIrq(&ReadIrq))
    {
        return MFALSE;
    }
    //
    if((CheckIrq.Status & ReadIrq.Status) != CheckIrq.Status)
    {
        LOG_ERR("Status:Check(0x%08X),Read(0x%08X)",CheckIrq.Status,ReadIrq.Status);
        return MFALSE;
    }
    return MTRUE;
    #endif
    LOG_ERR("NOT SUPPORT THIS");
    return MFALSE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq)
{
    MINT32 Ret;
    #if 0
    //
    LOG_INF(" - E. Type(%d),Status(%d)",ClearIrq.Type, ClearIrq.Status);
    //
    Ret = ioctl(mFd,ISP_CLEAR_IRQ,&ClearIrq);
    if(Ret < 0)
    {
        LOG_ERR("ISP_CLEAR_IRQ fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
    #endif
    LOG_ERR("NOT SUPPORT THIS");
    return MFALSE;
}

//-----------------------------------------------------------------------------
MINT32   IspDrvImp::registerIrq(const char* userName)
{
    MINT32 Ret;
    MINT32 key=-1;
    IISP_DRV_USERKEY_STRUCT userKeyStruct;
    userKeyStruct.userName=const_cast<char *>(userName);
    Ret = ioctl(mFd,ISP_REGISTER_IRQ_USER_KEY,&userKeyStruct);
    LOG_INF("userName(%s),userKey(%d)",userKeyStruct.userName,userKeyStruct.userKey);
    if(Ret < 0)
    {
        LOG_ERR("registerIrq fail, no more space for user to do irq operation");
        return -1;
    }

    key=userKeyStruct.userKey;
    if(key<0)
    {
        LOG_ERR("Invalid userKey(%d)",key);
        return -1;
    }

    return key;
}

//-----------------------------------------------------------------------------
MBOOL   IspDrvImp::markIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo)
{
    MINT32 Ret;
    Irqinfo.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    Irqinfo.Timeout=0;
    Irqinfo.bDumpReg=0;
    Ret = ioctl(mFd,ISP_MARK_IRQ_REQUEST,&Irqinfo);
    if(Ret < 0)
    {
        LOG_ERR("mark irq fail, user key/type/status (%d/%d/0x%x)",Irqinfo.UserInfo.UserKey,Irqinfo.UserInfo.Type,Irqinfo.UserInfo.Status);
        return MFALSE;
    }
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL   IspDrvImp::flushIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo)
{
    MINT32 Ret;
    Irqinfo.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    Irqinfo.Timeout=0;
    Irqinfo.bDumpReg=0;
    Ret = ioctl(mFd,ISP_FLUSH_IRQ_REQUEST,&Irqinfo);
    LOG_INF("flush irq, user key/type/status (%d/%d/0x%x)",Irqinfo.UserInfo.UserKey,Irqinfo.UserInfo.Type,Irqinfo.UserInfo.Status);
    if(Ret < 0)
    {
        LOG_ERR("flush irq fail, user key/type/status (%d/%d/0x%x)",Irqinfo.UserInfo.UserKey,Irqinfo.UserInfo.Type,Irqinfo.UserInfo.Status);
        return MFALSE;
    }
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL   IspDrvImp::queryirqtimeinfo(ISP_DRV_WAIT_IRQ_STRUCT* Irqinfo)
{
    MINT32 Ret;
    LOG_DBG(" - E. key(%d),Type(%d),Status(0x%08x).",Irqinfo->UserInfo.UserKey, Irqinfo->UserInfo.Type, Irqinfo->UserInfo.Status);
    ISP_DRV_WAIT_IRQ_STRUCT qirq;
    qirq.UserInfo = Irqinfo->UserInfo;
    qirq.TimeInfo.tLastEvent_sec=0;
    qirq.TimeInfo.tLastEvent_usec=0;
    qirq.TimeInfo.tmark2read_sec=0;
    qirq.TimeInfo.tmark2read_usec=0;
    qirq.TimeInfo.tevent2read_sec=0;
    qirq.TimeInfo.tevent2read_usec=0;
    qirq.TimeInfo.passedbySigcnt=0;

    Ret = ioctl(mFd,ISP_GET_MARK2QUERY_TIME,&qirq);
    if(Ret < 0)
    {
        LOG_ERR("flush irq fail, user key/type/status (%d/%d/0x%x)",Irqinfo->UserInfo.UserKey,Irqinfo->UserInfo.Type,Irqinfo->UserInfo.Status);
        return MFALSE;
    }
    else
    {
        Irqinfo->TimeInfo=qirq.TimeInfo;
        if(qirq.TimeInfo.tLastEvent_sec < 0)
        { Irqinfo->TimeInfo.tLastEvent_sec=0;}
        if(qirq.TimeInfo.tLastEvent_usec < 0)
        { Irqinfo->TimeInfo.tLastEvent_usec=0;}
        if(qirq.TimeInfo.tmark2read_sec < 0)
        { Irqinfo->TimeInfo.tmark2read_sec=0;}
        if(qirq.TimeInfo.tmark2read_usec < 0)
        { Irqinfo->TimeInfo.tmark2read_usec=0;}
        if(qirq.TimeInfo.tevent2read_sec < 0)
        { Irqinfo->TimeInfo.tevent2read_sec=0;}
        if(qirq.TimeInfo.tevent2read_usec < 0)
        { Irqinfo->TimeInfo.tevent2read_usec=0;}
    }

    return MTRUE;
}



//-----------------------------------------------------------------------------
MBOOL IspDrvImp::reset(MINT32 rstpath)
{
    MINT32 Ret;

    LOG_INF("ISP SW RESET[0x%08x]",mFd);

    Ret = ioctl(mFd,ISP_RESET,NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_RESET fail(%d)",Ret);
        return MFALSE;
    }

    return MTRUE;

}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::resetBuf(void)
{
    MINT32 Ret;
    //
    LOG_DBG("");
    //
    Ret = ioctl(mFd,ISP_RESET_BUF,NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_RESET_BUF fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::readRegs(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 Count,
    MINT32                  caller)
{
    MINT32 Ret;
    switch(caller)
    {    //assume that user do not burst read/write top related registers
        case ISP_DRV_RWREG_CALLFROM_MACRO:
            Ret=readRegsviaIO(pRegIo,Count);
            break;
        default:
            pthread_mutex_lock(&IspOtherRegMutex);
            Ret=readRegsviaIO(pRegIo,Count);
            pthread_mutex_unlock(&IspOtherRegMutex);
            break;
    }
    if(Ret < 0)
    {
        LOG_ERR("ISP_READ_REG_NOPROTECT fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MUINT32 IspDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MBOOL ret=0;
    MUINT32 value=0x0;
    MUINT32* pISPmmapAddr = this->getMMapRegAddr();
    if(m_regRWMode==ISP_DRV_RWREG_MODE_RW)
    {
        //value=*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr));
        value = pISPmmapAddr[(Addr>>2)];
    }
    else if(m_regRWMode==ISP_DRV_RWREG_MODE_RO)
    {
        ISP_DRV_REG_IO_STRUCT RegIo;
        RegIo.Addr = Addr;
        if(!readRegsviaIO(&RegIo, 1))
        {
            return 0;
        }
        value=RegIo.Data;
    }
    return value;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::writeRegs(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 Count,
    MINT32                  userEnum,
    MINT32                  caller)
{
    MINT32 Ret;
    switch(userEnum)
    {
        case ISP_DRV_USER_ISPF:
            break;
        case ISP_DRV_USER_SENF:
            break;
        case ISP_DRV_USER_FD:
            break;
        default:
            LOG_ERR("data[0x%x] cn[0x%x] only ispDrv/isp_function/seninf have authorization to directly write hw registers\n",pRegIo,Count);
            return MFALSE;
    }
    switch(caller)
    {
        case ISP_DRV_RWREG_CALLFROM_MACRO:
            Ret=writeRegsviaIO(pRegIo,Count);
            break;
        default:
            pthread_mutex_lock(&IspOtherRegMutex);
            Ret=writeRegsviaIO(pRegIo,Count);
            pthread_mutex_unlock(&IspOtherRegMutex);
            break;
    }
    if(Ret < 0)
    {
        LOG_ERR("ISP_WRITE_REG fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::writeReg(
    MUINT32     Addr,
    MUINT32     Data,
    MINT32      userEnum,
    MINT32      caller)
{
    //
    //LOG_VRB("Addr(0x%08X),Data(0x%08X)",Addr,Data);
    //
    MBOOL ret=0;
    MUINT32* pISPmmapAddr = this->getMMapRegAddr();
    switch(userEnum)
    {
        case ISP_DRV_USER_ISPF:
            break;
        case ISP_DRV_USER_SENF:
            break;
        case ISP_DRV_USER_FD:
            break;
        default:
            LOG_ERR("ad[0x%x] vl[0x%x] only ispDrv/isp_function/seninf have authorization to directly write hw registers\n",Addr,Data);
            return MFALSE;
    }
    switch(caller)
    {
        case ISP_DRV_RWREG_CALLFROM_MACRO:
            if(m_regRWMode==ISP_DRV_RWREG_MODE_RW)
            {
                //*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr))=Data;
                 pISPmmapAddr[(Addr>>2)] = Data;
            }
            else
            {
                ISP_DRV_REG_IO_STRUCT RegIo;
                RegIo.Addr = Addr;
                if(!writeRegsviaIO(&RegIo, 1))
                {
                    return 0;
                }
            }
            break;
        default:
            //check range
            ret=checkTopReg(Addr);
            if(ret==1)
            {
                pthread_mutex_lock(&IspTopRegMutex);
                if(m_regRWMode==ISP_DRV_RWREG_MODE_RW)
                {
                    //*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr))=Data;
                    pISPmmapAddr[(Addr>>2)] = Data;
                    //dsb();
                }
                else
                {
                    ISP_DRV_REG_IO_STRUCT RegIo;
                    RegIo.Addr = Addr;
                    if(!writeRegsviaIO(&RegIo, 1))
                    {
                        return 0;
                    }
                    //dsb();
                }
                pthread_mutex_unlock(&IspTopRegMutex);
            }
            else
            {
                pthread_mutex_lock(&IspOtherRegMutex);
                if(m_regRWMode==ISP_DRV_RWREG_MODE_RW)
                {
                    //*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr))=Data;
                    pISPmmapAddr[(Addr>>2)] = Data;
                }
                else
                {
                     ISP_DRV_REG_IO_STRUCT RegIo;
                     RegIo.Addr = Addr;
                     if(!writeRegsviaIO(&RegIo, 1))
                     {
                         return 0;
                    }
                }
                pthread_mutex_unlock(&IspOtherRegMutex);
            }
            break;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::holdReg(MBOOL En)
{
    MINT32 Ret;
    //
    LOG_DBG("En(%d)",En);
    //
    Ret = ioctl(mFd, ISP_HOLD_REG, &En);
    if(Ret < 0)
    {
        LOG_ERR("ISP_HOLD_REG fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::dumpReg(void)
{
    MINT32 Ret;
    //
    LOG_DBG("");
    //
    Ret = ioctl(mFd, ISP_DUMP_REG, NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_DUMP_REG fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------

isp_reg_t* IspDrvImp::getCurHWRegValues()
{
    MUINT32 size=sizeof(isp_reg_t);
    MUINT32* startAddr=getMMapRegAddr();
    //startAddr+=0x4000;
    //LOG_INF("isp_reg_t size(0x%x),starting Addr(0x%x),size/sizeof(MUINT32) (0x%x),0x4000Addr(0x%x)\n",size,getMMapRegAddr(),size/sizeof(MUINT32),startAddr);

    for(MINT32 i=0;i<size/sizeof(MUINT32);i++)
    {
         //LOG_INF("addr(0x%x) value (0x%x)\n",((MUINT32*)startAddr+i),*((MUINT32*)startAddr+i));
         *((MUINT32*)mpTempIspHWRegValues+i)=*((MUINT32*)startAddr+i);
    }
    return (mpTempIspHWRegValues);
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::readRegsviaIO(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 Count)
{
    MINT32 Ret;
    ISP_REG_IO_STRUCT_FRMB IspRegIo;
    //
    //LOG_DBG("Count(%d)",Count);
    //
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    //
    IspRegIo.pData_FrmB= (ISP_REG_STRUCT_FRMB*)pRegIo;
    IspRegIo.Count_FrmB = Count;
    //
    Ret = ioctl(mFd, ISP_READ_REGISTER, &IspRegIo);
    if(Ret < 0)
    {
        LOG_ERR("ISP_READ_REG_NOPROTECT via IO fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------

MBOOL IspDrvImp::writeRegsviaIO(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 Count)
{
    MINT32 Ret;
    ISP_REG_IO_STRUCT_FRMB IspRegIo;
    //
    //LOG_MSG("Count(%d) ",Count);
    //
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    //
    IspRegIo.pData_FrmB= (ISP_REG_STRUCT_FRMB*)pRegIo;
    IspRegIo.Count_FrmB = Count;
    //
    Ret = ioctl(mFd, ISP_WRITE_REGISTER, &IspRegIo);
    if(Ret < 0)
    {
        LOG_ERR("ISP_WRITE_REG via IO fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------

MUINT32* IspDrvImp::getRegAddr(void)
{
    //only return virtual register struct for user to calculate register offset
    if(mpIspVirRegMap == NULL)
    {
        LOG_ERR("NULL temp register struct for calculating offset\n");
    }
    else
    {
        return (MUINT32*)mpIspVirRegMap;
    }
    return 0;
}
//-----------------------------------------------------------------------------
/////
//temp remove later
isp_reg_t* IspDrvImp::getRegAddrMap(void)
{
    LOG_ERR("We do not support this interface and would remove this later\n");
    return 0;
}

//-----------------------------------------------------------------------------

#if defined(_use_kernel_ref_cnt_)

MBOOL IspDrvImp::kRefCntCtrl(ISP_REF_CNT_CTRL_STRUCT_FRMB* pCtrl)
{
    MINT32 Ret;
    //
    LOG_DBG("(%d)(%d)(0x%x)",pCtrl->ctrl,pCtrl->id,pCtrl->data_ptr);
    //
    Ret = ioctl(mFd,ISP_REF_CNT_CTRL,pCtrl);
    if(Ret < 0)
    {
        LOG_ERR("ISP_REF_CNT_CTRL fail(%d)[errno(%d):%s]  ",Ret, errno, strerror(errno));
        return MFALSE;
    }
    //
    return MTRUE;

}

#endif

//-----------------------------------------------------------------------------

MBOOL IspDrvImp::rtBufCtrl(void *pBuf_ctrl)
{
   MINT32 Ret;
    //
    ISP_BUFFER_CTRL_STRUCT_FRMB *pbuf_ctrl = (ISP_BUFFER_CTRL_STRUCT_FRMB *)pBuf_ctrl;
    //for debug log ONLY
    if ( ISP_RT_BUF_CTRL_MAX_FRMB == pbuf_ctrl->ctrl ) {
        LOG_DBG("[rtbc]cq(%d),vIspV/P(0x%x,0x%x),descV/P(0x%x,0x%x),mmap(0x%x)", \
            pbuf_ctrl->buf_id, \
            IspDrv::mpIspVirRegAddrVA[pbuf_ctrl->buf_id], \
            IspDrv::mpIspVirRegAddrPA[pbuf_ctrl->buf_id], \
            IspDrv::mpIspCQDescriptorVirt[pbuf_ctrl->buf_id], \
            IspDrv::mpIspCQDescriptorPhy[pbuf_ctrl->buf_id], \
            this->getMMapRegAddr());
        return MTRUE;
    }

#if defined(_rtbc_use_cq0c_)
    if(MFALSE == cqRingBuf(pBuf_ctrl)) {
        LOG_ERR("cqRingBuf(%d)  ",Ret);
        return MFALSE;
    }
#endif

    //
    Ret = ioctl(mFd,ISP_BUFFER_CTRL,pBuf_ctrl);
    if(Ret < 0)
    {
        LOG_ERR("ISP_BUFFER_CTRL(%d)  ",Ret);
        return MFALSE;
    }
    //
    return MTRUE;
}

//-----------------------------------------------------------------------------
MUINT32 IspDrvImp::pipeCountInc(EIspDrvPipePath ePipePath)
{
    LOG_INF("+,ePipePath(%d)",ePipePath);

    MBOOL Result = MTRUE;
    MINT32 ret = 0;
    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    MINT32 count;

    // Increase global pipe count.
    ref_cnt.ctrl = ISP_REF_CNT_INC;
    switch(ePipePath)
    {
        case eIspDrvPipePath_P1:
            ref_cnt.id = ISP_REF_CNT_ID_P1_PIPE;
            break;
        case eIspDrvPipePath_P2:
            ref_cnt.id = ISP_REF_CNT_ID_P2_PIPE;
            break;
        case eIspDrvPipePath_Global:
            ref_cnt.id = ISP_REF_CNT_ID_GLOBAL_PIPE;
            break;
        default:
            break;
    }
    ref_cnt.data_ptr = &count;

    ret = ioctl(mFd, ISP_REF_CNT_CTRL, &ref_cnt);
    if(ret < 0)
    {
        LOG_ERR("ISP_REF_CNT_INC fail(%d)[errno(%d):%s]",ret, errno, strerror(errno));
        Result = MFALSE;
    }

    LOG_INF("-,Result(%d),count(%d)", Result, count);

    return Result;

}


MUINT32 IspDrvImp::pipeCountDec(EIspDrvPipePath ePipePath)
{
    LOG_INF("+,ePipePath(%d)",ePipePath);

    MBOOL Result = MTRUE;
    MINT32 ret = 0;
    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    MINT32 count;

    switch(ePipePath)
    {
        case eIspDrvPipePath_P1:
            ref_cnt.ctrl = ISP_REF_CNT_DEC_AND_RESET_P1_IF_LAST_ONE;
            ref_cnt.id = ISP_REF_CNT_ID_P1_PIPE;
            break;
        case eIspDrvPipePath_P2:
            ref_cnt.ctrl = ISP_REF_CNT_DEC_AND_RESET_P2_IF_LAST_ONE;
            ref_cnt.id = ISP_REF_CNT_ID_P2_PIPE;
            break;
        case eIspDrvPipePath_Global:
            ref_cnt.ctrl = ISP_REF_CNT_DEC_AND_RESET_IF_LAST_ONE;
            ref_cnt.id = ISP_REF_CNT_ID_GLOBAL_PIPE;
            break;
        default:
            break;
    }
    ref_cnt.data_ptr = &count;

    ret = ioctl(mFd, ISP_REF_CNT_CTRL, &ref_cnt);
    if(ret < 0)
    {
        LOG_ERR("ISP_REF_CNT_DEC fail(%d)[errno(%d):%s]",ret, errno, strerror(errno));
        Result = MFALSE;
    }

    LOG_INF("-,Result(%d),count(%d)", Result, count);

    return Result;

}

MBOOL IspDrvImp::updateCq0bRingBuf(void *pOBval)
{
    MINT32 Ret = MTRUE;
    MUINT32 *pOB = (MUINT32 *)pOBval;
    //
 #define RTBC_GET_PA_FROM_VA(va,bva,bpa) ( ( (unsigned long)(va) - (unsigned long)(bva) ) + (unsigned long)(bpa) )

    //LOG_DBG("size of CQ_RTBC_RING_ST is (%d)  ",sizeof(CQ0B_RTBC_RING_ST_FRMB));
    CQ0B_RTBC_RING_ST_FRMB *pcqrtbcring_va  = (CQ0B_RTBC_RING_ST_FRMB*)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0B];
    CQ0B_RTBC_RING_ST_FRMB *pcqrtbcring_pa  = (CQ0B_RTBC_RING_ST_FRMB*)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0B];

    LOG_DBG("[rtbc]va(0x%x),pa(0x%x)", pcqrtbcring_va, pcqrtbcring_pa);
    //
    pcqrtbcring_va->rtbc_ring_frmb.pNext_frmb = &pcqrtbcring_va->rtbc_ring_frmb;
    pcqrtbcring_va->rtbc_ring_frmb.next_pa_frmb = \
        (MUINT32)RTBC_GET_PA_FROM_VA(pcqrtbcring_va->rtbc_ring_frmb.pNext_frmb,pcqrtbcring_va,pcqrtbcring_pa);

    //vir isp drv
    int realCQIdx = this->getRealCQIndex(ISP_DRV_CQ0B,0,ISP_DRV_P1_DEFAULT_DUPCQIDX);
    IspDrv *_targetVirDrv = this->getCQInstance(ISP_DRV_CQ0B);

    //ob gain
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN0, (*pOB),ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN1, (*pOB),ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN2, (*pOB),ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN3, (*pOB),ISP_DRV_USER_ISPF);

    //cmd
    pcqrtbcring_va->rtbc_ring_frmb.cq0b_rtbc_frmb.ob_frmb.inst = 0x00034510; //reg_4510
    pcqrtbcring_va->rtbc_ring_frmb.cq0b_rtbc_frmb.ob_frmb.data_ptr_pa = \
        (unsigned long)(&mpIspVirRegAddrPA[realCQIdx][0x00004510>>2]); // >>2 for MUINT32* pointer;

    //end
    pcqrtbcring_va->rtbc_ring_frmb.cq0b_rtbc_frmb.end_frmb.inst = 0xFC000000;
    pcqrtbcring_va->rtbc_ring_frmb.cq0b_rtbc_frmb.end_frmb.data_ptr_pa = 0;

    //CQ0B baseaddress
    LOG_DBG("CQ0B(0x%x)\n", this->getCQDescBufPhyAddr(ISP_DRV_CQ0B,0,ISP_DRV_P1_DEFAULT_DUPCQIDX));
    ISP_WRITE_REG(this,CAM_CTL_CQ0B_BASEADDR,(unsigned long)this->getCQDescBufPhyAddr(ISP_DRV_CQ0B,0,ISP_DRV_P1_DEFAULT_DUPCQIDX),ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(this, CAM_CTL_EN2_SET, CQ0B_EN_SET, 1,ISP_DRV_USER_ISPF);

  #if 0
    {
        int *pdata = (int*)&pcqrtbcring_va->rtbc_ring_frmb;
        for (int i=0 ; i<(sizeof(CQ0B_RING_CMD_ST_FRMB)>>2) ; i++ ) {
            LOG_DBG("[rtbc][des] addr:(0x%08x) val:(0x%08x)",pdata+i,pdata[i]);
        }
        LOG_DBG("X");
    }
  #endif
    return MTRUE;
}

#if defined(_rtbc_use_cq0c_)
MBOOL IspDrvImp::_updateEnqCqRingBuf(void *pBuf_ctrl)
{
    MINT32 Ret = MTRUE;

     //
 #define RTBC_GET_PA_FROM_VA(va,bva,bpa) ( ( (unsigned long)(va) - (unsigned long)(bva) ) + (unsigned long)(bpa) )

     //LOG_DBG("size of CQ_RTBC_RING_ST is (%d)  ",sizeof(CQ_RTBC_RING_ST_FRMB));
     CQ_RTBC_RING_ST_FRMB *pcqrtbcring_va  = (CQ_RTBC_RING_ST_FRMB*)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C];
     CQ_RTBC_RING_ST_FRMB *pcqrtbcring_pa  = (CQ_RTBC_RING_ST_FRMB*)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0C];

     //isp_reg_t* pIspPhyReg = (isp_reg_t*)getMMapRegAddr();
     ISP_BUFFER_CTRL_STRUCT_FRMB *pbuf_ctrl = (ISP_BUFFER_CTRL_STRUCT_FRMB *)pBuf_ctrl;
     ISP_RT_BUF_INFO_STRUCT_FRMB *pbuf_info = (ISP_RT_BUF_INFO_STRUCT_FRMB *)pbuf_ctrl->data_ptr;
     ISP_RT_BUF_INFO_STRUCT_FRMB *pex_buf_info = (ISP_RT_BUF_INFO_STRUCT_FRMB *)pbuf_ctrl->ex_data_ptr;
     //
     MUINT32 i = 0;
     MUINT32 cam_tg_vf;
     MUINT32 size;

     cam_tg_vf  = ISP_READ_REG_NOPROTECT(this, CAM_TG_VF_CON);
     LOG_DBG("[rtbc] VF(0x%x), mod_id=%d",cam_tg_vf, pbuf_ctrl->buf_id);

    //VF_EN==0
    if ( 0 == (cam_tg_vf & 0x01) ) {
        //
        LOG_DBG("[rtbc]va(0x%x),pa(0x%x),ctrl(%d),dma(%d),PA(0x%x),o_size(%d),2o_size(%d)", \
                pcqrtbcring_va, \
                pcqrtbcring_pa, \
                pbuf_ctrl->ctrl, \
                pbuf_ctrl->buf_id, \
                pbuf_info->base_pAddr, \
                pcqrtbcring_va->imgo_ring_size_frmb,\
                pcqrtbcring_va->img2o_ring_size_frmb);

        //
        if ( _imgo_ == pbuf_ctrl->buf_id) {
            i = pcqrtbcring_va->imgo_ring_size_frmb;
        }
        else if (_img2o_ == pbuf_ctrl->buf_id) {
            i = pcqrtbcring_va->img2o_ring_size_frmb;
        }
        else {
            LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
            return MFALSE;
        }
        //
        pcqrtbcring_va->rtbc_ring_frmb[i].pNext_frmb = &pcqrtbcring_va->rtbc_ring_frmb[(i>0)?0:i];
        pcqrtbcring_va->rtbc_ring_frmb[i].next_pa_frmb = \
            (MUINT32)RTBC_GET_PA_FROM_VA(pcqrtbcring_va->rtbc_ring_frmb[i].pNext_frmb,pcqrtbcring_va,pcqrtbcring_pa);


        //imgo/
        if ( 0 == pcqrtbcring_va->imgo_ring_size_frmb ) {
            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_frmb.inst = ISP_DRV_CQ_DUMMY_WR_TOKEN; //0x00007300
        }
        //imgo
        if ( _imgo_ == pbuf_ctrl->buf_id ) {
           pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_frmb.inst = 0x00004300; //reg_4300
           pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_base_pAddr_frmb = pbuf_info->base_pAddr; // ISP_CQ_DUMMY_PA; //
           pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_buf_idx_frmb = pbuf_info->bufIdx;
        }
        pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_frmb.data_ptr_pa = \
            (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_base_pAddr_frmb,pcqrtbcring_va,pcqrtbcring_pa);

        //img2o
        if ( 0 == pcqrtbcring_va->img2o_ring_size_frmb ) {
            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_frmb.inst = ISP_DRV_CQ_DUMMY_WR_TOKEN; //0x00007320; //reg_7320
        }
        //
        if (_img2o_ == pbuf_ctrl->buf_id ) {
            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_frmb.inst = 0x00004320; //reg_4320
            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb = pbuf_info->base_pAddr;
            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_buf_idx_frmb = pbuf_info->bufIdx;
        }
        pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_frmb.data_ptr_pa = \
            (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb,pcqrtbcring_va,pcqrtbcring_pa);

        //cq0c
        pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.next_cq0ci_frmb.inst = 0x000040BC; //reg_40BC
        pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.next_cq0ci_frmb.data_ptr_pa = \
            (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring_frmb[i].next_pa_frmb,pcqrtbcring_va,pcqrtbcring_pa);

        //end
        pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.end_frmb.inst = 0xFC000000;
        pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.end_frmb.data_ptr_pa = 0;

        //
        if (i>0) {
            pcqrtbcring_va->rtbc_ring_frmb[i-1].pNext_frmb = &pcqrtbcring_va->rtbc_ring_frmb[i];
            pcqrtbcring_va->rtbc_ring_frmb[i-1].next_pa_frmb = \
                (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring_frmb[i],pcqrtbcring_va,pcqrtbcring_pa);
            //
            pcqrtbcring_va->rtbc_ring_frmb[i-1].cq_rtbc_frmb.next_cq0ci_frmb.data_ptr_pa = \
                (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring_frmb[i-1].next_pa_frmb,pcqrtbcring_va,pcqrtbcring_pa);
        }
        //
        if ( _imgo_ == pbuf_ctrl->buf_id) {
            pcqrtbcring_va->imgo_ring_size_frmb++;
            size = pcqrtbcring_va->imgo_ring_size_frmb;
            //
            ISP_WRITE_BITS(this,CAM_CTL_IMGO_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(this,CAM_CTL_IMGO_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
        }
        else if ( _img2o_ == pbuf_ctrl->buf_id) {
            pcqrtbcring_va->img2o_ring_size_frmb++;
            size = pcqrtbcring_va->img2o_ring_size_frmb;
            //
            ISP_WRITE_BITS(this,CAM_CTL_IMG2O_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(this,CAM_CTL_IMG2O_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
        }
        //
        if ( _imgo_ == pbuf_ctrl->buf_id || _img2o_ == pbuf_ctrl->buf_id) {
            //LOG_INF("[rtbc] Update CAM_CTL_CQ0C_BASEADDR");
            ISP_WRITE_REG(this,CAM_CTL_CQ0C_BASEADDR,(unsigned long)this->getCQDescBufPhyAddr(ISP_DRV_CQ0C,0,ISP_DRV_P1_DEFAULT_DUPCQIDX) + ((1==size)?0:sizeof(CQ_RING_CMD_ST_FRMB)),ISP_DRV_USER_ISPF);
        }
        //
        LOG_DBG("[rtbc]imgo_sz(%d),img2o_sz(%d), BufIdx=0x%x",pcqrtbcring_va->imgo_ring_size_frmb,pcqrtbcring_va->img2o_ring_size_frmb,pbuf_info->bufIdx);
        //
        {
            int *pdata = (int*)&pcqrtbcring_va->rtbc_ring_frmb[0];
            for ( i=0 ; i<(sizeof(CQ_RING_CMD_ST_FRMB)>>2)*size ; i++ ) {
                LOG_DBG("[rtbc][des] addr:(0x%08x) val:(0x%08x)",pdata+i,pdata[i]);
            }
        }
    }
    //VF_EN==1
    else {
        if ( pex_buf_info ) {
            MUINT32 findTarget = 0;

            LOG_DBG("[rtbc]va(0x%x),pa(0x%x),ctrl(%d),dma(%d),PA(0x%x),o_size(%d),2o_size(%d)", \
                pcqrtbcring_va, \
                pcqrtbcring_pa, \
                pbuf_ctrl->ctrl, \
                pbuf_ctrl->buf_id, \
                pbuf_info->base_pAddr, \
                pcqrtbcring_va->imgo_ring_size_frmb,\
                pcqrtbcring_va->img2o_ring_size_frmb);

            //
            i = 0;
            if ( _imgo_ == pbuf_ctrl->buf_id) {
                //
                if (pex_buf_info->bufIdx != 0xFFFF) {
                    //LOG_INF("[rtbc]ByIdx: IMGO");
                    //replace the specific buffer with same bufIdx
                    for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size_frmb ; i++ ) {
                        if ( pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_buf_idx_frmb == pbuf_info->bufIdx) {
                            //LOG_INF("[rtbc]ByIdx: idx(%d) old/new imgo buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_base_pAddr_frmb,pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_base_pAddr_frmb = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                } else {
                    //replace the specific buffer with same buf Address
                    //LOG_INF("[rtbc]ByAddr: IMGO");
                    for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size_frmb ; i++ ) {
                        if ( pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_base_pAddr_frmb == pbuf_info->base_pAddr ) {
                            //LOG_INF("[rtbc]ByAddr: idx(%d) old/new imgo buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_base_pAddr_frmb,pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_base_pAddr_frmb = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                }
                if (!findTarget) {
                    LOG_ERR("[rtbc]error exIMGO-Fail. BufIdx(0x%x/0x%x) new imgo buffer(0x%x)",\
                          pbuf_info->bufIdx, pex_buf_info->bufIdx,pex_buf_info->base_pAddr);
                    for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size_frmb ; i++ ) {
                        LOG_DBG("[rtbc][%d] old/new BufIdx(%d/%d) ",i,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.imgo_buf_idx_frmb, pbuf_info->bufIdx);
                   }
                }
            }
            else if (_img2o_ == pbuf_ctrl->buf_id) {
                //
                if (pex_buf_info->bufIdx != 0xFFFF) {
                    //replace the specific buffer with same bufIdx
                    //LOG_INF("[rtbc]ByIdx: IMG2O");
                    for ( i=0 ; i<pcqrtbcring_va->img2o_ring_size_frmb ; i++ ) {
                        //LOG_INF("[rtbc]ByIdx:[%d] old/new BufIdx(%d/%d) ",i,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_buf_idx_frmb, pbuf_info->bufIdx);
                        if ( pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_buf_idx_frmb == pbuf_info->bufIdx ) {
                            //LOG_INF("[rtbc]ByIdx: idx(%d) old/new img2o buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb, pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                } else {
                    //replace the specific buffer with same buf Address
                    //LOG_INF("[rtbc]ByAddr: IMG2O");
                    for ( i=0 ; i<pcqrtbcring_va->img2o_ring_size_frmb ; i++ ) {
                        if ( pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb == pbuf_info->base_pAddr ) {
                            //LOG_INF("[rtbc]ByAddr: idx(%d) old/new img2o buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb, pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                }
                if (!findTarget) {
                   LOG_ERR("[rtbc]error exIMG2O-Fail. Bufidx(0x%x/0x%x) old/new img2o buffer(0x%x/0x%x)",\
                       pbuf_info->bufIdx,pex_buf_info->bufIdx,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_base_pAddr_frmb,pex_buf_info->base_pAddr);
                   for ( i=0 ; i<pcqrtbcring_va->img2o_ring_size_frmb ; i++ ) {
                     LOG_DBG("[rtbc]ByIdx:[%d] old/new BufIdx(%d/%d) ",i,pcqrtbcring_va->rtbc_ring_frmb[i].cq_rtbc_frmb.img2o_buf_idx_frmb, pbuf_info->bufIdx);
                   }
                }
            }
            else {
                LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
                return MFALSE;
            }
        }
        else {
            //
        }
    }

    return MTRUE;
}

MBOOL IspDrvImp::cqRingBuf(void *pBuf_ctrl)
{
   MINT32 Ret;
    //
    ISP_BUFFER_CTRL_STRUCT_FRMB *pbuf_ctrl = (ISP_BUFFER_CTRL_STRUCT_FRMB *)pBuf_ctrl;


    /*LOG_INF("[rtbc]pcqrtbcring_va/pa(0x%x/0x%x),ctrl(%d),dma(%d),pa(0x%x),imgor_size(%d),img2or_size(%d)", \
            pcqrtbcring_va, \
            pcqrtbcring_pa, \
            pbuf_ctrl->ctrl, \
            pbuf_ctrl->buf_id, \
            pbuf_info->base_pAddr, \
            pcqrtbcring_va->imgo_ring_size,\
            pcqrtbcring_va->img2o_ring_size);*/
    //
    //LOG_INF("[rtbc](%d)",sizeof(CQ_RING_CMD_ST_FRMB));

    //
    switch( pbuf_ctrl->ctrl ) {
        //
        case ISP_RT_BUF_CTRL_ENQUE_IMD_FRMB:
        case ISP_RT_BUF_CTRL_ENQUE_FRMB:
            Ret = _updateEnqCqRingBuf(pBuf_ctrl);
            //
            break;
#if 0
        case ISP_RT_BUF_CTRL_EXCHANGE_ENQUE_FRMB:
            //
            #if 0
            cam_tg_vf  = ISP_READ_REG_NOPROTECT(this, CAM_TG_VF_CON);
            cam_tg2_vf = ISP_READ_REG_NOPROTECT(this, CAM_TG2_VF_CON);
            #else //fpga ldvt
            cam_tg_vf  = ISP_READ_REG_NOPROTECT(this, CAM_TG_VF_CON);
            cam_tg2_vf = ISP_READ_REG_NOPROTECT(this, CAM_TG2_VF_CON);
            #endif

            //VF on line
            if ( (cam_tg_vf & 0x01) || (cam_tg2_vf & 0x01) ) {
                LOG_INF("[rtbc]exchange 1st buf. by 2nd buf. and enque it(0x%x)",pex_buf_info);
#if 0
                if ( pex_buf_info ) {
                    //
                    if ( _imgo_ == pbuf_ctrl->buf_id ) {
                        //
                        for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size ; i++ ) {
                            if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr == pbuf_info->base_pAddr ) {
                                    LOG_INF("[rtbc]old(%d) imgo buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr);
                                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pex_buf_info->base_pAddr;
                                    LOG_INF("new(%d) imgo buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr);
                                break;
                            }
                        }
                    }
                    else if (_rrzo_ == pbuf_ctrl->buf_id) {
                        //
                        for ( i=0 ; i<pcqrtbcring_va->rrzo_ring_size ; i++ ) {
                            if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr == pbuf_info->base_pAddr ) {
                                    LOG_INF("[rtbc]old(%d) img2o buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr);
                                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr = pex_buf_info->base_pAddr;
                                    LOG_INF("new(%d) img2o buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr);
                                break;
                            }
                        }
                    }
                    else {
                        LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
                        return MFALSE;
                    }
                }
                else {
                    //
                }
#endif
            }
            break;
#endif
        case ISP_RT_BUF_CTRL_CLEAR_FRMB:

            //reset cq0c all 0 for rtbc
            if (sizeof(CQ_RTBC_RING_ST_FRMB) <= sizeof(mIspCQDescInit)) {
                if ((_imgo_ == pbuf_ctrl->buf_id)||(_img2o_ == pbuf_ctrl->buf_id)) {
                    LOG_DBG("[PIP]Clr P1 CQ0C");
                    memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C],0,sizeof(CQ_RTBC_RING_ST_FRMB));
                }
            }
            else {
                LOG_ERR("rtbc data exceed buffer size(%d)>(%d)",sizeof(CQ_RTBC_RING_ST_FRMB),sizeof(mIspCQDescInit));
            }
            break;
        default:
            //LOG_ERR("ERROR:ctrl id(%d)",pbuf_ctrl->ctrl);
            break;
    }

    //reset cq0c all 0 for rtbc
    //memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C],0,sizeof(CQ_RTBC_RING_ST_FRMB));

    return MTRUE;
}
#endif

MBOOL IspDrvImp::ISPWakeLockCtrl(MBOOL WakeLockEn)
{
    MBOOL ret = MTRUE;
    MUINT32 wakelock_ctrl = WakeLockEn;

    ret = ioctl(mFd, ISP_WAKELOCK_CTRL, &wakelock_ctrl);
    if (ret < 0)
    {
        LOG_ERR("ISP_WAKELOCK_CTRL fail(%d).", ret);
        return MFALSE;
    }

    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::bypassTuningQue(
    ESoftwareScenario softScenario,
    MINT32 magicNum
)
{
    Mutex::Autolock lock(tuningQueueIdxLock);
    //
    MINT32 currQueueIdx = -1;
    MINT32 nextQueueIdx = -1;
    ISP_DRV_CQ_ENUM cq;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MBOOL ret = MTRUE;
    //
    LOG_INF("+,softScenario(%d),magicNum(0x%x)",softScenario,magicNum);
    //
    getP2cqInfoFromScenario(softScenario, p2Cq);

    if((mTuningQueIdx[p2Cq].curWriteIdx>=ISP_TUNING_QUEUE_NUM)||(mTuningQueIdx[p2Cq].curWriteIdx<0)){
        LOG_WRN("[Warning]tuning queue index(%d) error",mTuningQueIdx[p2Cq].curWriteIdx);
        mTuningQueIdx[p2Cq].curWriteIdx = 0;
    }
    currQueueIdx = mTuningQueIdx[p2Cq].curWriteIdx;
    //
    // 1.set magic num for tuning queue
    mTuningQueInf[p2Cq][currQueueIdx].magicNum = magicNum;
    //
    // 2.set null point for current tuning queue
    mTuningQueIdx[p2Cq].pCurWriteTuningQue = NULL;
    // 3. set update function bit for tuning Queue
    mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit = eIspTuningMgrFunc_Null;
    nextQueueIdx = GET_NEXT_TUNING_QUEUE_IDX(currQueueIdx);
    LOG_INF("p2Cq(%d),pre-curWriteIdx(%d),next-curWriteIdx(%d)",p2Cq,mTuningQueIdx[p2Cq].curWriteIdx,nextQueueIdx);
    mTuningQueIdx[p2Cq].curWriteIdx = nextQueueIdx;


    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::deTuningQueByCq(
    ETuningQueAccessPath path,
    ISP_DRV_CQ_ENUM eCq,
    MINT32 magicNum
)
{
    Mutex::Autolock lock(tuningQueueIdxLock);

    MBOOL ret = MTRUE;
    MINT32 currQueueIdx = ISP_TUNING_INIT_IDX;
    MINT32 tarQueueIdx = ISP_TUNING_INIT_IDX;

    ISP_DRV_P2_CQ_ENUM p2Cq;
    isp_reg_t*  pIspRegMap;
    MUINT32 camCtlEn1, camCtlEn2, camCtlDmaEn;
    MUINT32 addrOffset, moduleSize;
    MUINT32 i, j, k;
    MINT32 checkNum; //for tuning bypass issue in initial stage
    //
    mapCqToP2Cq(eCq, p2Cq);
    LOG_INF("+,path(%d),eCq(%d),p2Cq(%d),magic#(0x%x),curWIdx(%d)", \
        path,eCq,p2Cq,magicNum,mTuningQueIdx[p2Cq].curWriteIdx);
    //
    if(path == eTuningQueAccessPath_featureio){
        if((mTuningQueIdx[p2Cq].curWriteIdx>=ISP_TUNING_QUEUE_NUM)||(mTuningQueIdx[p2Cq].curWriteIdx<0)){
            LOG_WRN("[Init]init tuningQue curWriteIdx index(%d)",mTuningQueIdx[p2Cq].curWriteIdx);
            mTuningQueIdx[p2Cq].curWriteIdx = 0;
        }
        currQueueIdx = mTuningQueIdx[p2Cq].curWriteIdx;
        //
        // 1.set magic num for tuning queue
        mTuningQueInf[p2Cq][currQueueIdx].magicNum = magicNum;
        //
        // 2.update current tuning queue point of feature
        mTuningQueIdx[p2Cq].pCurWriteTuningQue = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue;
        LOG_DBG("p2Cq(%d),currQueueIdx(%d),pTuningQue(0x%x)",p2Cq,currQueueIdx,mTuningQueInf[p2Cq][currQueueIdx].pTuningQue);

    } else if(path == eTuningQueAccessPath_imageio_P1) {
        if(mTuningQueIdx[p2Cq].isInitP1 == MFALSE){
            checkNum = mTuningQueIdx[p2Cq].curWriteIdx;
            if(checkNum <= 0) {
                LOG_ERR("[Error]P1 TuningMgr not ready yet! checkNum(%d)",checkNum);
                ret = MFALSE;
                goto EXIT;
            }
        } else {
            checkNum = 1;
        }
        LOG_DBG("checkNum(%d),curWriteIdx(%d),isInitP1(%d)",checkNum,mTuningQueIdx[p2Cq].curWriteIdx,mTuningQueIdx[p2Cq].isInitP1);
        for (k=0;k<(MUINT32)checkNum;k++){
            if (mTuningQueIdx[p2Cq].isInitP1==MTRUE){
                // [1]backward search the tuning que with magic#
                if(searchTuningQue(p2Cq, magicNum, currQueueIdx) != MTRUE){
                    LOG_ERR("[Error]curReadP1Idx search error");
                    ret = MFALSE;
                    goto EXIT;
                }
                mTuningQueIdx[p2Cq].curReadP1Idx = currQueueIdx;
            } else {
                MUINT32 searchTar;
                LOG_INF("k(%d)",k);
                // [1] search the tuning que by order
#if 0
                if(((MUINT32)checkNum == k+1)){ // get currQueueIdx
                    mTuningQueIdx[p2Cq].isInitP1 = MTRUE;
                    LOG_INF("[P1]search ending, and Exit, k(%d)",k);
                    if(k != 0)
                        break;
                }
                currQueueIdx = (MINT32)k;
#else
                searchTar = ((mTuningQueIdx[p2Cq].curWriteIdx-1)>=(k+1))?(k+1):(mTuningQueIdx[p2Cq].curWriteIdx-1);
//                if(orderSearchTuningQue(p2Cq, magicNum, k, (mTuningQueIdx[p2Cq].curWriteIdx-1), currQueueIdx) != MTRUE){ // get currQueueIdx
                if(orderSearchTuningQue(p2Cq, magicNum, k, searchTar, currQueueIdx) != MTRUE){ // get currQueueIdx
                    mTuningQueIdx[p2Cq].isInitP1 = MTRUE;
                    LOG_INF("search ending, and Exit,(%d-%d)",k,searchTar);
                    break;
                }
#endif
                mTuningQueIdx[p2Cq].curReadP1Idx = currQueueIdx;

            }
            //
            // [2]set the real pCurReadP1TuningQue
            mTuningQueIdx[p2Cq].pCurReadP1TuningQue = mTuningQueIdx[p2Cq].keepP1Que.pTuningQue;
            //
            // [3]update eCurReadP1UpdateFuncBit & keepP1UpdateFuncBit
            mTuningQueIdx[p2Cq].eCurReadP1UpdateFuncBit = mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit;
            mTuningQueIdx[p2Cq].keepP1UpdateFuncBit = (EIspTuningMgrFunc)
                    ((MINT32)mTuningQueIdx[p2Cq].keepP1UpdateFuncBit | (MINT32)mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit);
            //
            // [4]update the current queue info.
            pIspRegMap = (isp_reg_t*)mTuningQueInf[p2Cq][currQueueIdx].pTuningQue;
            if (NULL == pIspRegMap){
                LOG_ERR("[Error]pIspRegMap NULL");
                ret = MFALSE;
                goto EXIT;
            }

            camCtlEn1 = pIspRegMap->CAM_CTL_EN1.Raw;
            camCtlEn2 = pIspRegMap->CAM_CTL_EN2.Raw;
            camCtlDmaEn = pIspRegMap->CAM_CTL_DMA_EN.Raw;

            LOG_DBG("camCtlEn1(0x%x),camCtlEn2(0x%x),camCtlDmaEn(%d),keepP1UpdateFuncBit(0x%x),eUpdateFuncBit(0x%x)",camCtlEn1,camCtlEn2,camCtlDmaEn,mTuningQueIdx[p2Cq].keepP1UpdateFuncBit,mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit);
            //
            for(i=0;i<eIspTuningMgrFuncBit_Num;i++){
                if((1<<gIspTuningFuncBitMapp[i].eTuningFuncBit) & mTuningQueIdx[p2Cq].eCurReadP1UpdateFuncBit){
                    switch(gIspTuningFuncBitMapp[i].eTuningCtlByte){
                        case eTuningCtlByte_P1:
                            if(gIspTuningFuncBitMapp[i].camCtlEn1 != -1){
                                // update for keepReadP1CtlEn1
                                mTuningQueIdx[p2Cq].keepReadP1CtlEn1 =
                                    ((~(1<<gIspTuningFuncBitMapp[i].camCtlEn1))&mTuningQueIdx[p2Cq].keepReadP1CtlEn1)|
                                    (camCtlEn1&(1<<gIspTuningFuncBitMapp[i].camCtlEn1));

                                // update keepP1Que
                                if(gIspTuningFuncBitMapp[i].eTuningCqFunc1 != CAM_DUMMY_){
                                    getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc1,addrOffset,moduleSize);
                                    LOG_DBG("[1]P1:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
                                    for(j=0;j<moduleSize;j++){
                                        mTuningQueIdx[p2Cq].keepP1Que.pTuningQue[addrOffset>>2] = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue[addrOffset>>2];
                                        addrOffset += 4;
                                    }
                                }
                                if(gIspTuningFuncBitMapp[i].eTuningCqFunc2 != CAM_DUMMY_){
                                    getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc2,addrOffset,moduleSize);
                                    LOG_DBG("[2]P1:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
                                    for(j=0;j<moduleSize;j++){
                                        mTuningQueIdx[p2Cq].keepP1Que.pTuningQue[addrOffset>>2] = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue[addrOffset>>2];
                                        addrOffset += 4;
                                    }
                                }
                            } else {
                                LOG_ERR("[Error]gIspTuningFuncBitMapp table error i(%d) = -1",i);
                                ret = MFALSE;
                                goto EXIT;
                            }
                            //
                            // update for keepReadP1CtlEn2
                            if(gIspTuningFuncBitMapp[i].camCtlEn2 != -1){
                                mTuningQueIdx[p2Cq].keepReadP1CtlEn2 =
                                    ((~(1<<gIspTuningFuncBitMapp[i].camCtlEn2))&mTuningQueIdx[p2Cq].keepReadP1CtlEn2)|
                                    (camCtlEn2&(1<<gIspTuningFuncBitMapp[i].camCtlEn2));
                            }
                            //
                            // update for keepReadP1CtlDmaEn
                            if(gIspTuningFuncBitMapp[i].camCtlDmaEn != -1){
                                mTuningQueIdx[p2Cq].keepReadP1CtlDmaEn =
                                    ((~(1<<gIspTuningFuncBitMapp[i].camCtlDmaEn))&mTuningQueIdx[p2Cq].keepReadP1CtlDmaEn)|
                                    (camCtlDmaEn&(1<<gIspTuningFuncBitMapp[i].camCtlDmaEn));
                            }
                            break;
                        case eTuningCtlByte_P1D:
                        case eTuningCtlByte_P2:
                        default:
                            LOG_ERR("[Error]Not support this path(%d)",gIspTuningFuncBitMapp[i].eTuningCtlByte);
                            /* do nothing */
                            break;
                    };
                }
            }
        }
    } else {
        LOG_ERR("[Error]Not support this path(%d)",path);
        ret = MFALSE;
        goto EXIT;
    }

EXIT:
    LOG_DBG("-,");

    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::deTuningQue(
    ETuningQueAccessPath path,
    ESoftwareScenario softScenario,
    MINT32 magicNum
)
{
    MBOOL ret;
    ISP_DRV_CQ_ENUM cq;
    //
    LOG_DBG("+,path(%d),softScenario(%d),magicNum(0x%x)",path,softScenario,magicNum);
    //
    getCqInfoFromScenario(softScenario, cq);
    ret = deTuningQueByCq(path, cq, magicNum);

    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::enTuningQueByCq(
    ETuningQueAccessPath path,
    ISP_DRV_CQ_ENUM eCq,
    MINT32 magicNum,
    EIspTuningMgrFunc updateFuncBit
)
{
    Mutex::Autolock lock(tuningQueueIdxLock);

    MBOOL ret = MTRUE;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MINT32 currQueueIdx = -1;
    MINT32 nextQueueIdx = -1;
    //
    //
    mapCqToP2Cq(eCq, p2Cq);
    LOG_DBG("path(%d),eCq(%d),p2Cq(%d),magicNum(0x%x),updateFuncBit(0x%08x)",path,eCq,p2Cq,magicNum,updateFuncBit);
    //
    if(path == eTuningQueAccessPath_featureio){
        if((mTuningQueIdx[p2Cq].curWriteIdx>=ISP_TUNING_QUEUE_NUM)||(mTuningQueIdx[p2Cq].curWriteIdx<0)){
            LOG_WRN("[Error]tuning queue index(%d) error",mTuningQueIdx[p2Cq].curWriteIdx);
            mTuningQueIdx[p2Cq].curWriteIdx = 0;
            ret = MFALSE;
            goto EXIT;
        }
        currQueueIdx = mTuningQueIdx[p2Cq].curWriteIdx;
        LOG_DBG("currQueueIdx(%d)",currQueueIdx);
        // set update function bit for tuning Queue
        mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit = updateFuncBit;
        nextQueueIdx = GET_NEXT_TUNING_QUEUE_IDX(currQueueIdx);
        if(nextQueueIdx==mTuningQueIdx[p2Cq].curReadP1Idx){
            LOG_ERR("[Error]multi-access the same index==>nextQueueIdx(%d),curReadP1Idx(%d)",
                nextQueueIdx,mTuningQueIdx[p2Cq].curReadP1Idx);
            ret = MFALSE;
            goto EXIT;
        }
        mTuningQueIdx[p2Cq].curWriteIdx = nextQueueIdx;
    } else if(path == eTuningQueAccessPath_imageio_P1) {
        //release p1 tuning queue
        mTuningQueIdx[p2Cq].pCurReadP1TuningQue = NULL;
        mTuningQueIdx[p2Cq].curReadP1Idx = -1;
    } else {
        LOG_ERR("[Error]Not support this path(%d)",path);
        ret = MFALSE;
        goto EXIT;
    }

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::enTuningQue(
    ETuningQueAccessPath path,
    ESoftwareScenario softScenario,
    MINT32 magicNum,
    EIspTuningMgrFunc updateFuncBit
)
{
    MBOOL ret = MTRUE;
    ISP_DRV_CQ_ENUM cq;
    //
    LOG_DBG("path(%d),softScenario(%d),magicNum(0x%x)",path,softScenario,magicNum);
    //
    getCqInfoFromScenario(softScenario, cq);
    ret = enTuningQueByCq(path, cq, magicNum, updateFuncBit);


    return ret;
}


//-----------------------------------------------------------------------------
MBOOL IspDrvImp::setP2TuningStatus(
    ISP_DRV_CQ_ENUM cq,
    MBOOL en)
{
    Mutex::Autolock lock(tuningQueueIdxLock);

//    en = MFALSE;//kk test
    //
    ISP_DRV_P2_CQ_ENUM p2Cq;
    mapCqToP2Cq(cq, p2Cq);
    LOG_INF("p2Cq(%d),en(%d)",p2Cq,en);

    mTuningQueIdx[p2Cq].isApplyTuning = en;

    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::getP2TuningStatus(ISP_DRV_CQ_ENUM cq)
{
    Mutex::Autolock lock(tuningQueueIdxLock);
    //
    ISP_DRV_P2_CQ_ENUM p2Cq;
    mapCqToP2Cq(cq, p2Cq);
    LOG_DBG("p2Cq(%d),isApplyTuning(%d)",p2Cq,mTuningQueIdx[p2Cq].isApplyTuning);

    return mTuningQueIdx[p2Cq].isApplyTuning;
}


//-----------------------------------------------------------------------------

MUINT32 IspDrvImp::getTuningUpdateFuncBit(
    ETuningQueAccessPath ePath,
    MINT32 cq,
    MUINT32 drvScenario,
    MBOOL isV3)
{
    Mutex::Autolock lock(tuningQueueIdxLock);
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MUINT32 mApplyTuningFuncBit;
    //
    if(isV3) {
        mapCqToP2Cq((ISP_DRV_CQ_ENUM)cq, p2Cq);
        if(ePath == eTuningQueAccessPath_imageio_P2){
            switch (drvScenario){
                case NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_VSS:
                case NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC:
                default:
                    mApplyTuningFuncBit = (eIspTuningMgrFunc_Pgn|eIspTuningMgrFunc_Cfa|eIspTuningMgrFunc_Ccl|eIspTuningMgrFunc_G2g|
                        eIspTuningMgrFunc_G2c|eIspTuningMgrFunc_Nbc|eIspTuningMgrFunc_Seee|eIspTuningMgrFunc_Sl2|
                        eIspTuningMgrFunc_Ggmrb|eIspTuningMgrFunc_Ggmg|eIspTuningMgrFunc_Pca);
                    break;
                case NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_IP:
                    mApplyTuningFuncBit = (eIspTuningMgrFunc_Obc|eIspTuningMgrFunc_Lsc|eIspTuningMgrFunc_Bnr|
                        eIspTuningMgrFunc_Pgn|eIspTuningMgrFunc_Cfa|eIspTuningMgrFunc_Ccl|eIspTuningMgrFunc_G2g|
                        eIspTuningMgrFunc_G2c|eIspTuningMgrFunc_Nbc|eIspTuningMgrFunc_Seee|eIspTuningMgrFunc_Sl2|
                        eIspTuningMgrFunc_Ggmrb|eIspTuningMgrFunc_Ggmg|eIspTuningMgrFunc_Pca);

                    break;
            };
        } else if(ePath == eTuningQueAccessPath_imageio_P1){
            mApplyTuningFuncBit = mTuningQueIdx[p2Cq].keepP1UpdateFuncBit;
        } else {
            LOG_ERR("[Error]Not support this path(%d)",ePath);
            mApplyTuningFuncBit = 0;
        }
    } else {
        LOG_ERR("[Error]Not support this function in V1");
    }
    return mApplyTuningFuncBit;
}

//-----------------------------------------------------------------------------
MUINT32* IspDrvImp::getTuningBuf(
    ETuningQueAccessPath ePath,
    MINT32 cq)
{
    Mutex::Autolock lock(tuningQueueIdxLock);

    MUINT32* pBuf=NULL;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    //
    mapCqToP2Cq((ISP_DRV_CQ_ENUM)cq, p2Cq);
    LOG_DBG("+,ePath(%d),p2Cq(%d)",ePath, p2Cq);
    //
    if(ePath == eTuningQueAccessPath_imageio_P1) {
        pBuf = mTuningQueIdx[p2Cq].pCurReadP1TuningQue;

    } else if(ePath == eTuningQueAccessPath_featureio){
        pBuf = mTuningQueIdx[p2Cq].pCurWriteTuningQue;

    } else {
        LOG_ERR("[Error]Not support this path(%d)",ePath);
    }
    //
    if(pBuf==NULL){
        LOG_WRN("[warning]pBuf=NULL,p2Cq(%d)",p2Cq);
    }

    return pBuf;

}


//-----------------------------------------------------------------------------
MUINT32 IspDrvImp::getTuningTop(
    ETuningQueAccessPath ePath,
    ETuningTopEn top,
    MINT32 cq,
    MUINT32 magicNum)
{
    Mutex::Autolock lock(tuningQueueIdxLock);

    MUINT32 en = 0;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MBOOL isPreview = MTRUE;
    //
    mapCqToP2Cq((ISP_DRV_CQ_ENUM)cq, p2Cq);
    LOG_DBG("+");

   if(ePath == eTuningQueAccessPath_imageio_P1) {
        switch(top){
            case eTuningTopEn1:
                en = mTuningQueIdx[p2Cq].keepReadP1CtlEn1;
                break;
            case eTuningTopEn2:
                en = mTuningQueIdx[p2Cq].keepReadP1CtlEn2;
                break;
            case eTuningTopDmaEn:
                en = mTuningQueIdx[p2Cq].keepReadP1CtlDmaEn;
                break;
            default:
                LOG_ERR("[Error]Not support this top(%d) for path(%d)",(MUINT32)top,ePath);
                break;
        }
    } else {
        LOG_ERR("[Error]Not support this path(%d)",ePath);
    }
    LOG_DBG("-,ePath(%d),top(%d),cq(%d),p2Cq(%d),magicNum(0x%x),en(0x%08x)",ePath,top, cq,p2Cq,magicNum,en);

    return en;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::searchTuningQue(
    ISP_DRV_P2_CQ_ENUM p2Cq,
    MINT32 magicNum,
    MINT32 &getQueIdx
)
{
    MBOOL ret = MTRUE;
    MUINT32 searchCnt;
    MINT32 currQueueIdx = -1;
    MINT32 prevQueueIdx = -1;

    LOG_DBG("+,p2Cq(%d),magicNum(0x%x)",p2Cq,magicNum);

    //backward search
    if((mTuningQueIdx[p2Cq].curWriteIdx<ISP_TUNING_QUEUE_NUM)&&(mTuningQueIdx[p2Cq].curWriteIdx>=0)){
        searchCnt = 0;
        currQueueIdx = mTuningQueIdx[p2Cq].curWriteIdx;
        do{
            searchCnt++;
            prevQueueIdx = GET_PREV_TUNING_QUEUE_IDX(currQueueIdx);
            //LOG_INF("searchCnt(%d),currQueueIdx(%d),prevQueueIdx(%d),pre_magicNum(0x%x)",searchCnt,currQueueIdx,prevQueueIdx,mTuningQueInf[p2Cq][prevQueueIdx].magicNum);
            currQueueIdx = prevQueueIdx;
        }while((mTuningQueInf[p2Cq][currQueueIdx].magicNum != magicNum)&&(searchCnt<ISP_TUNING_QUEUE_NUM));
        if(searchCnt>=ISP_TUNING_QUEUE_NUM){
            LOG_ERR("[Error]search fail, magic#(0x%x)/curWriteIdx(0x%x)-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x",
                magicNum, mTuningQueIdx[p2Cq].curWriteIdx,
                mTuningQueInf[p2Cq][0].magicNum,mTuningQueInf[p2Cq][1].magicNum,mTuningQueInf[p2Cq][2].magicNum,
                mTuningQueInf[p2Cq][3].magicNum,mTuningQueInf[p2Cq][4].magicNum,mTuningQueInf[p2Cq][5].magicNum,
                mTuningQueInf[p2Cq][6].magicNum,mTuningQueInf[p2Cq][7].magicNum,mTuningQueInf[p2Cq][8].magicNum,
                mTuningQueInf[p2Cq][9].magicNum,mTuningQueInf[p2Cq][10].magicNum,mTuningQueInf[p2Cq][11].magicNum,
                mTuningQueInf[p2Cq][12].magicNum,mTuningQueInf[p2Cq][13].magicNum,mTuningQueInf[p2Cq][14].magicNum,
                mTuningQueInf[p2Cq][15].magicNum,mTuningQueInf[p2Cq][16].magicNum);
            ret = MFALSE;
        } else {
            getQueIdx = currQueueIdx;
        }
    } else {
        LOG_ERR("[Error]tuningQue curWriteIdx(%d) not ready",mTuningQueIdx[p2Cq].curWriteIdx);
        ret = MFALSE;
    }
    LOG_DBG("-,getQueIdx(%d),curWriteIdx(%d)",getQueIdx,mTuningQueIdx[p2Cq].curWriteIdx);

    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::orderSearchTuningQue(
    ISP_DRV_P2_CQ_ENUM p2Cq,
    MINT32 magicNum,
    MUINT32 start,
    MUINT32 end,
    MINT32 &getQueIdx
)
{
    MBOOL ret = MTRUE;
    MUINT32 searchCnt;
    MINT32 currQueueIdx = -1;
    MINT32 prevQueueIdx = -1;
    MUINT32 i;

    LOG_INF("+,p2Cq(%d),magicNum(0x%x),start(%d),end(%d)",p2Cq,magicNum,start,end);

    if(start > end){
        getQueIdx = -1;
        LOG_ERR("[Error]search range error, start(%d),end(%d)",start,end);
        ret = MFALSE;
        goto EXIT;
    }

    //order search
    for (i=start;i<=end;i++){
        if (mTuningQueInf[p2Cq][i].magicNum==magicNum){
            getQueIdx = i;
            break;
        }
    }
    if(i > end){
        getQueIdx = -1;
        LOG_WRN("[warning]cant seach the identical magic#(0x%x),start(%d),end(%d)",magicNum,start,end);
        ret = MFALSE;
    }
EXIT:

    LOG_INF("-,getQueIdx(%d),curWriteIdx(%d)",getQueIdx,mTuningQueIdx[p2Cq].curWriteIdx);

    return ret;
}


MBOOL IspDrvImp::getTuningTpipeFiled(
    ISP_DRV_P2_CQ_ENUM p2Cq,
    stIspTuningTpipeFieldInf &pTuningField
)
{
    Mutex::Autolock lock(tuningQueueIdxLock);

    MBOOL ret = MTRUE;

    memcpy((char*)&pTuningField.sl2, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.sl2, sizeof(ISP_TPIPE_CONFIG_SL2_STRUCT));
    memcpy((char*)&pTuningField.cfa, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.cfa, sizeof(ISP_TPIPE_CONFIG_CFA_STRUCT));
    memcpy((char*)&pTuningField.nbc, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.nbc, sizeof(ISP_TPIPE_CONFIG_NBC_STRUCT));
    memcpy((char*)&pTuningField.seee, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.seee, sizeof(ISP_TPIPE_CONFIG_SEEE_STRUCT));
    memcpy((char*)&pTuningField.lce, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lce, sizeof(ISP_TPIPE_CONFIG_LCE_STRUCT));
    memcpy((char*)&pTuningField.bnr, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.bnr, sizeof(ISP_TPIPE_CONFIG_BNR_STRUCT));
    memcpy((char*)&pTuningField.lsc, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lsc, sizeof(ISP_TPIPE_CONFIG_LSC_STRUCT));
    memcpy((char*)&pTuningField.lcei, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lcei, sizeof(ISP_TPIPE_CONFIG_LCEI_STRUCT));
    memcpy((char*)&pTuningField.lsci, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lsci, sizeof(ISP_TPIPE_CONFIG_LSCI_STRUCT));

    LOG_DBG("tarCfa.bypass(%d),desCfa.bypass(%d)",mTuningQueIdx[p2Cq].keepTpipeField.cfa.bayer_bypass,pTuningField.cfa.bayer_bypass);
    LOG_INF("kk:get tuning seee_edge,s:(%d),d:(%d)",mTuningQueIdx[p2Cq].keepTpipeField.seee.se_edge,pTuningField.seee.se_edge);


    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::getP2cqInfoFromScenario(
        ESoftwareScenario softScenario,
        ISP_DRV_P2_CQ_ENUM &p2Cq
)
{
    MBOOL ret = MTRUE;
    switch(softScenario){
        case eSoftwareScenario_Main_Normal_Stream:
        case eSoftwareScenario_Main_Normal_Capture:
        case eSoftwareScenario_Main_Pure_Raw_Stream:
        case eSoftwareScenario_Main_ZSD_Capture:
        case eSoftwareScenario_Sub_Normal_Stream:
        case eSoftwareScenario_Sub_Pure_Raw_Stream:
        case eSoftwareScenario_Sub_Normal_Capture:
        case eSoftwareScenario_Sub_ZSD_Capture:
            p2Cq = ISP_DRV_P2_CQ1;
            break;
        case eSoftwareScenario_Main_VSS_Capture:
        case eSoftwareScenario_Sub_VSS_Capture:
            p2Cq = ISP_DRV_P2_CQ2;
            break;
        default:
            LOG_ERR("[Error]Not support this scenario(%d)",softScenario);
            p2Cq = ISP_DRV_P2_CQ1;
            ret = MFALSE;
            break;
    };
    return ret;
}


//-----------------------------------------------------------------------------
MBOOL IspDrvImp::getCqInfoFromScenario(
        ESoftwareScenario softScenario,
        ISP_DRV_CQ_ENUM &cq
)
{
    MBOOL ret = MTRUE;
    switch(softScenario){
        case eSoftwareScenario_Main_Normal_Stream:
        case eSoftwareScenario_Main_Normal_Capture:
        case eSoftwareScenario_Main_Pure_Raw_Stream:
        case eSoftwareScenario_Main_ZSD_Capture:
        case eSoftwareScenario_Sub_Normal_Stream:
        case eSoftwareScenario_Sub_Pure_Raw_Stream:
        case eSoftwareScenario_Sub_Normal_Capture:
        case eSoftwareScenario_Sub_ZSD_Capture:
            cq = ISP_DRV_CQ01;
            break;
        case eSoftwareScenario_Main_VSS_Capture:
        case eSoftwareScenario_Sub_VSS_Capture:
            cq = ISP_DRV_CQ02;
            break;
        default:
            LOG_ERR("[Error]Not support this scenario(%d)",softScenario);
            cq = ISP_DRV_CQ01;
            ret = MFALSE;
            break;
    };
    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::mapCqToP2Cq(
        ISP_DRV_CQ_ENUM cq,
        ISP_DRV_P2_CQ_ENUM &p2Cq
)
{
    MBOOL ret = MTRUE;

    switch(cq){
        case ISP_DRV_CQ01:
            p2Cq = ISP_DRV_P2_CQ1;
            break;
        case ISP_DRV_CQ02:
            p2Cq = ISP_DRV_P2_CQ2;
            break;
        case ISP_DRV_CQ03:
        default:
            LOG_ERR("[Error]Not support this cq(%d) mapping",cq);
            p2Cq = ISP_DRV_P2_CQ1;
            break;
    };

    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvVirImp::getCqModuleInf(
    CAM_MODULE_ENUM moduleId,
    MUINT32 &addrOffset,
    MUINT32 &moduleSize)
{
    MBOOL ret = MTRUE;

    addrOffset = mIspCQModuleInfo[moduleId].addr_ofst;
    moduleSize = mIspCQModuleInfo[moduleId].reg_num;

    return ret;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::getCqModuleInf(
    CAM_MODULE_ENUM moduleId,
    MUINT32 &addrOffset,
    MUINT32 &moduleSize)
{
    MBOOL ret = MTRUE;

    addrOffset = mIspCQModuleInfo[moduleId].addr_ofst;
    moduleSize = mIspCQModuleInfo[moduleId].reg_num;

    return ret;
}


//-----------------------------------------------------------------------------
MBOOL IspDrvImp::enqueP2Frame(MUINT32 callerID,MINT32 p2burstQIdx,MINT32 p2dupCQIdx)
{
    MBOOL ret=MTRUE;
    ISP_DRV_ED_BUFQUE_STRUCT edBufQueV;
    edBufQueV.ctrl=ISP_DRV_BUFQUE_CTRL_ENQUE_FRAME;
    edBufQueV.processID=0;  //temp 0, would update in kernel
    edBufQueV.callerID=callerID;
    edBufQueV.p2burstQIdx=p2burstQIdx;
    edBufQueV.p2dupCQIdx=p2dupCQIdx;
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL_FRMB,&edBufQueV);
    if(ret<0)
    {
        LOG_ERR("ISP_DRV_BUFQUE_CTRL_ENQUE_FRAME fail(%d). callerID(0x%x).", ret,callerID);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::waitP2Deque()
{
    MBOOL ret=MTRUE;
    ISP_DRV_ED_BUFQUE_STRUCT edBufQueV;
    edBufQueV.ctrl=ISP_DRV_BUFQUE_CTRL_WAIT_DEQUE;
    edBufQueV.processID=0;//temp 0, would update in kernel
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL_FRMB,&edBufQueV);

    if(ret<0)
    {
        LOG_ERR(" ISP_DRV_BUFQUE_CTRL_WAIT_DEQUE fail(%d). callerID(0x%x).", ret,edBufQueV.callerID);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::dequeP2FrameSuccess(MUINT32 callerID,MINT32 p2dupCQIdx)
{
    MBOOL ret=MTRUE;
    ISP_DRV_ED_BUFQUE_STRUCT edBufQueV;
    edBufQueV.ctrl=ISP_DRV_BUFQUE_CTRL_DEQUE_SUCCESS;
    edBufQueV.processID=0;//temp 0, would update in kernel
    edBufQueV.callerID=callerID;
    edBufQueV.p2dupCQIdx=p2dupCQIdx;
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL_FRMB,&edBufQueV);
    if(ret<0)
    {
        LOG_ERR("ISP_DRV_BUFQUE_CTRL_DEQUE_DONE fail(%d). callerID(0x%x).", ret,callerID);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::dequeP2FrameFail(MUINT32 callerID,MINT32 p2dupCQIdx)
{
    MBOOL ret=MTRUE;
    ISP_DRV_ED_BUFQUE_STRUCT edBufQueV;
    edBufQueV.ctrl=ISP_DRV_BUFQUE_CTRL_DEQUE_FAIL;
    edBufQueV.processID=0;//temp 0, would update in kernel
    edBufQueV.callerID=callerID;
    edBufQueV.p2dupCQIdx=p2dupCQIdx;
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL_FRMB,&edBufQueV);
    if(ret<0)
    {
        LOG_ERR("ISP_DRV_BUFQUE_CTRL_DEQUE_DONE fail(%d). callerID(0x%x).", ret,callerID);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::waitP2Frame(MUINT32 callerID,MINT32 p2dupCQIdx,MINT32 timeoutUs)
{
    MBOOL ret=MTRUE;
    ISP_DRV_ED_BUFQUE_STRUCT edBufQueV;
    edBufQueV.ctrl=ISP_DRV_BUFQUE_CTRL_WAIT_FRAME;
    edBufQueV.processID=0;//temp 0, would update in kernel
    edBufQueV.callerID=callerID;
    edBufQueV.p2dupCQIdx=p2dupCQIdx;
    edBufQueV.timeoutUs=timeoutUs;
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL_FRMB,&edBufQueV);
    if(ret<0)
    {
        LOG_ERR("ISP_DRV_BUFQUE_CTRL_WAIT_FRAME fail(%d).callerID(0x%x), Timeout(%d).", ret,callerID, timeoutUs);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::wakeP2WaitedFrames()
{
    MBOOL ret=MTRUE;
    ISP_DRV_ED_BUFQUE_STRUCT edBufQueV;
    edBufQueV.ctrl=ISP_DRV_BUFQUE_CTRL_WAKE_WAITFRAME;
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL_FRMB,&edBufQueV);
    if(ret<0)
    {
        LOG_ERR("ISP_DRV_BUFQUE_CTRL_WAKE_WAITFRAME fail(%d).", ret);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}
//-----------------------------------------------------------------------------
MBOOL IspDrvImp::freeAllP2Frames()
{
    MBOOL ret=MTRUE;
    ISP_DRV_ED_BUFQUE_STRUCT edBufQueV;
    edBufQueV.ctrl=ISP_DRV_BUFQUE_CTRL_CLAER_ALL;
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL_FRMB,&edBufQueV);
    if(ret<0)
    {
        LOG_ERR("ISP_DRV_BUFQUE_CTRL_CLAER_ALL fail(%d).", ret);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}

//-----------------------------------------------------------------------------
//update/query register Scenario
MBOOL IspDrvImp::updateScenarioValue(
    MUINT32 value)
{
    MBOOL ret=MTRUE;
    MUINT32 qValue;
    qValue=value;
    #if 0  // fix it later
    ret=ioctl(mFd,ISP_UPDATE_REGSCEN_FRMB,&qValue);
    #endif
    if(ret<0)
    {
        LOG_ERR("ISP_UPDATE_REGSCEN_FRMB fail(%d).", ret);
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::queryScenarioValue(
    MUINT32& value)
{
    MBOOL ret=MTRUE;
    MUINT32 qValue;
    #if 0  // fix it later
    ret=ioctl(mFd,ISP_QUERY_REGSCEN_FRMB,&qValue);
    #endif
    if(ret<0)
    {
        LOG_ERR("ISP_QUERY_REGSCEN_FRMB fail(%d).", ret);
        return MFALSE;
    }
    else
    {
        value=qValue;
        return MTRUE;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IspDrvVirImp::IspDrvVirImp()
{
    //LOG_INF("");
    mpIspVirRegBuffer = NULL;
}
//-----------------------------------------------------------------------------
IspDrvVirImp::~IspDrvVirImp()
{
    //LOG_INF("");
}
//-----------------------------------------------------------------------------
IspDrv* IspDrvVirImp::getInstance(MINT32 cq, MUINT32* ispVirRegAddr,  isp_reg_t* ispVirRegMap)
{
    LOG_DBG("cq: %d. ispVirRegAddr: 0x%08x.", cq, ispVirRegAddr);
    if ((unsigned long) ispVirRegAddr & ISP_DRV_VIR_ADDR_ALIGN ) {
        LOG_ERR("NOT 8 bytes alignment ");
        return NULL;
    }
    //FIXME, need CHECK
    static IspDrvVirImp singleton[(ISP_DRV_DEFAULT_BURST_QUEUE_NUM*ISP_DRV_P1_CQ_DUPLICATION_NUM*ISP_DRV_P1_PER_CQ_SET_NUM)+(ISP_DVR_MAX_BURST_QUEUE_NUM*ISP_DRV_P2_CQ_DUPLICATION_NUM*ISP_DRV_P2_PER_CQ_SET_NUM)];
    //static IspDrvVirImp singleton[ISP_DRV_TOTAL_CQ_NUM];
    singleton[cq].mpIspVirRegBuffer = ispVirRegAddr;
    singleton[cq].mpIspVirRegMap =ispVirRegMap;
    return &singleton[cq];
}
//-----------------------------------------------------------------------------
void IspDrvVirImp::destroyInstance(void)
{
}
//-----------------------------------------------------------------------------
MBOOL IspDrvVirImp::reset(MINT32 rstpath) {
    //chrsitopher
    //NEEDFIX, we have to seperate the rst info in virtual reg table??
    memset(mpIspVirRegBuffer,0x00,sizeof(MUINT32)*ISP_BASE_RANGE);
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvVirImp::readRegs(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 Count,
    MINT32                  caller)
{
    MUINT32 i;
    //
    //Mutex::Autolock lock(mLock);
    //
    //LOG_DBG("Count(%d)",Count);
    //
    if(mpIspVirRegBuffer == NULL)
    {
        LOG_ERR("mpIspVirRegBuffer is NULL");
        return MFALSE;
    }
    switch(caller)
    {    //assume that user do not burst read/write top related registers
        case ISP_DRV_RWREG_CALLFROM_MACRO:
            for(i=0; i<Count; i++)
            {
                pRegIo[i].Data = mpIspVirRegBuffer[(pRegIo[i].Addr)>>2];
            }
            break;
        default:
            pthread_mutex_lock(&IspOtherRegMutex);
            for(i=0; i<Count; i++)
            {
                pRegIo[i].Data = mpIspVirRegBuffer[(pRegIo[i].Addr)>>2];
            }
            pthread_mutex_unlock(&IspOtherRegMutex);
            break;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MUINT32 IspDrvVirImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MBOOL ret=0;
    MUINT32 regData=0;
    //
    //Mutex::Autolock lock(mLock);
    //
    //LOG_DBG("Addr(0x%08X)",Addr);
    //
    if(mpIspVirRegBuffer == NULL)
    {
        LOG_ERR("mpIspVirRegBuffer is NULL");
        return MFALSE;
    }
    regData = mpIspVirRegBuffer[(Addr)>>2];
    return (regData);
}
//-----------------------------------------------------------------------------
MBOOL IspDrvVirImp::writeRegs(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 Count,
    MINT32                  userEnum,
    MINT32                  caller)
{
    MUINT32 i;
    //
    //Mutex::Autolock lock(mLock);
    //
    //LOG_DBG("Count(%d)",Count);
    //
    if(mpIspVirRegBuffer == NULL)
    {
        LOG_ERR("mpIspVirRegBuffer is NULL");
        return MFALSE;
    }
    switch(caller)
    {
        case ISP_DRV_RWREG_CALLFROM_MACRO:
            for(i=0; i<Count; i++)
            {
                mpIspVirRegBuffer[(pRegIo[i].Addr)>>2] = pRegIo[i].Data;
            }
            break;
        default:
            pthread_mutex_lock(&IspOtherRegMutex);
            for(i=0; i<Count; i++)
               {
                   mpIspVirRegBuffer[(pRegIo[i].Addr)>>2] = pRegIo[i].Data;
            }
            pthread_mutex_unlock(&IspOtherRegMutex);
            break;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrvVirImp::writeReg(
    MUINT32     Addr,
    MUINT32     Data,
    MINT32      userEnum,
    MINT32      caller)
{
    //
    //Mutex::Autolock lock(mLock);
    //
    //LOG_DBG("Addr(0x%08X),Data(0x%08X)",Addr,Data);
    //
    if(mpIspVirRegBuffer == NULL)
    {
        LOG_ERR("mpIspVirRegBuffer is NULL");
        return MFALSE;
    }
    MBOOL ret=0;
    switch(caller)
    {
        case ISP_DRV_RWREG_CALLFROM_MACRO:
            //*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr))=Data;
            mpIspVirRegBuffer[Addr>>2] = Data;
            break;
        default:
            ret=checkTopReg(Addr);
            if(ret==1)
            {
                pthread_mutex_lock(&IspTopRegMutex);
                //*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr))=Data;
                mpIspVirRegBuffer[Addr>>2] = Data;
                pthread_mutex_unlock(&IspTopRegMutex);
            }
            else
            {
                pthread_mutex_lock(&IspOtherRegMutex);
                //*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr))=Data;
                mpIspVirRegBuffer[Addr>>2] = Data;
                pthread_mutex_unlock(&IspOtherRegMutex);
            }
            break;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MUINT32* IspDrvVirImp::getRegAddr(void)
{
    LOG_VRB("mpIspVirRegBuffer(0x%08X)",mpIspVirRegBuffer);
    if(mpIspVirRegBuffer != NULL)
    {
        return mpIspVirRegBuffer;
    }
    else    {
        return NULL;
    }
}
MBOOL IspDrvVirImp::
getIspCQModuleInfo(CAM_MODULE_ENUM eModule,ISP_DRV_CQ_MODULE_INFO_STRUCT &outInfo)
{
    outInfo.id = mIspCQModuleInfo[eModule].id;
    outInfo.addr_ofst = mIspCQModuleInfo[eModule].addr_ofst;
    outInfo.reg_num = mIspCQModuleInfo[eModule].reg_num;

    LOG_DBG("getIspCQModuleInfo: eModule=0x%d, ofst=0x%x, reg_num=0x%x", eModule, outInfo.addr_ofst, outInfo.reg_num);

    return MTRUE;
}

//-----------------------------------------------------------------------------
