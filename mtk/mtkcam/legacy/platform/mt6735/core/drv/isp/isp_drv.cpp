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
#define LOG_TAG "IspDrv"
//
#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
//#include <utils/threads.h>
#include <cutils/atomic.h>
//#include <cutils/pmem.h>
#include "sync_write.h" // For dsb()
#include "errata.h"  //for cpu

#include "camera_isp.h"
#include <mtkcam/common.h>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv/imem_drv.h>   // For IMemDrv*.
#include "isp_drv_imp.h"
#include <mtkcam/imageio/ispio_pipe_scenario.h>    // For enum EDrvScenario.

#include <linux/errno.h>
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
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define SIG_ERESTARTSYS 512 //ERESTARTSYS

// for debug CQ virtual table
#define LOG_CQ_VIRTUAL_TABLE(_pIspVirCqVa_,_idx_,_num_) \
{   \
    switch(_num_) {   \
        case 1: \
            LOG_INF("0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2]);   \
            break;  \
        case 2: \
            LOG_INF("0x%08x-0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2], \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+1]); \
            break;  \
        case 3: \
            LOG_INF("0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+2]); \
            break;  \
        case 4: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2],   \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+3]); \
            break;  \
        case 5: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2],    \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+4]); \
            break;  \
        case 6: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2], \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+4],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+5]); \
            break;  \
        case 7: \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+4],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+5],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+6]); \
            break;  \
        case 8: \
        default:    \
            LOG_INF("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",_pIspVirCqVa_[mIspCQModuleInfo[_idx_].addr_ofst >>2],   \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+1],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+2],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+3],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+4],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+5],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+6],  \
                _pIspVirCqVa_[(mIspCQModuleInfo[_idx_].addr_ofst>>2)+7]); \
            break;  \
    }   \
}


// for tuning queue
#define GET_NEXT_TUNING_QUEUE_IDX(_idx_)    ( ((((MINT32)_idx_)+1)>=ISP_TUNING_QUEUE_NUM )?(0):(((MINT32)_idx_)+1) )
#define GET_PREV_TUNING_QUEUE_IDX(_idx_)    ( ((((MINT32)_idx_)-1)>=0 )?(((MINT32)_idx_)-1):(ISP_TUNING_QUEUE_NUM-1) )


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
#define USE_ISPDRV_OPEN_FD    (1)//1: open M4U FD before open camera-isp FD

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/


//-----------------------------------------------------------------------------
pthread_mutex_t IspTopRegMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IspOtherRegMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IspCQinfoMutex = PTHREAD_MUTEX_INITIALIZER;


MUINT32*    IspDrvImp::mpIspHwRegAddr = NULL;
isp_reg_t*  IspDrv::mpIspVirRegMap;
MINT32      IspDrv::mIspVirRegFd;
MUINT32*    IspDrv::mpIspVirRegBufferBaseAddr;
MUINT32     IspDrv::mIspVirRegSize;
MUINT32**   IspDrv::mpIspVirRegAddrVA=NULL;
MUINT32**   IspDrv::mpIspVirRegAddrPA=NULL;
MINT32      IspDrv::mIspCQDescFd;
MUINT32*    IspDrv::mpIspCQDescBufferVirt = NULL;
MUINT32     IspDrv::mIspCQDescSize;
MUINT32*    IspDrv::mpIspCQDescBufferPhy = NULL;    // Fix build warning
ISP_DRV_CQ_CMD_DESC_STRUCT **IspDrv::mpIspCQDescriptorVirt=NULL;
MUINT32**   IspDrv::mpIspCQDescriptorPhy=NULL;
MBOOL       IspDrv::mbBufferAllocated = MFALSE;
MINT32      IspDrv::mCurBurstQNum=1;
// cq order: cq0, cq0b, cq0c, cq0_d, cq0b_d, cq0c_d, burstQ0_cq1_dup0,burstQ0_cq2_dup0,burstQ0_cq3_dup0,burstQ0_cq1_dup1,burstQ0_cq2_dup1,burstQ0_cq3_dup1,
//           burstQ1_cq1_dup0,burstQ1_cq2_dup0,burstQ1_cq3_dup0,burstQ1_cq1_dup1,burstQ1_cq2_dup1,burstQ1_cq3_dup1,...and so on
MINT32      IspDrv::mTotalCQNum=ISP_DRV_BASIC_CQ_NUM;
// for turning update
 stIspTuningQueInf IspDrv::mTuningQueInf[ISP_DRV_P2_CQ_NUM][ISP_TUNING_QUEUE_NUM];
 stIspTuningQueIdx IspDrv::mTuningQueIdx[ISP_DRV_P2_CQ_NUM];


//
#define ISP_CQ_WRITE_INST 0x0
#define ISP_DRV_CQ_END_TOKEN 0xFC000000
#define ISP_DRV_CQ_DUMMY_WR_TOKEN 0x4184
#define ISP_CQ_DUMMY_PA 0x88100000
//


    //
    ISP_DRV_CQ_CMD_DESC_INIT_STRUCT  mIspCQDescInit[CAM_MODULE_MAX];
    //
    ISP_DRV_CQ_MODULE_INFO_STRUCT mIspCQModuleInfo[CAM_MODULE_MAX]
    =   {
            {CAM_TOP_CTL_EN_P1,      0x4004,  2    },
            {CAM_TOP_CTL_EN_P1_D,    0x4010,  2    },
            {CAM_TOP_CTL_EN_P2,      0x4018,  2    },
            {CAM_TOP_CTL_SCEN,       0x4024,  1    },
            {CAM_TOP_CTL_FMT_P1,     0x4028,  1    },
            {CAM_TOP_CTL_FMT_P1_D,   0x402C,  1    },
            {CAM_TOP_CTL_FMT_P2,     0x4030,  1    },
            {CAM_TOP_CTL_SEL_P1,     0x4034,  1    },
            {CAM_TOP_CTL_SEL_P1_D,   0x4038,  1    },
            {CAM_TOP_CTL_SEL_P2,     0x403C,  1    },
            {CAM_CTL_TCM_EN,         0x407C,  1    },
            {CAM_CTL_TDR_EN,         0x4084,  1    },
            {CAM_CTL_IHDR,           0x4104,  1    },
            {CAM_CTL_IHDR_D,         0x4108,  1    },
            {CAM_RAW_TG1_TG2,        0x4410,  56   },
            {CAM_ISP_BIN,            0x44F0,  2    },
            {CAM_ISP_OBC,            0x4500,  8    },
            {CAM_ISP_LSC,            0x4530,  8    },
            {CAM_ISP_RPG,            0x4550,  6    },
            {CAM_ISP_SGG3,           0x4570,  3    },
            {CAM_ISP_SGG2,           0x4580,  3    },
            {CAM_ISP_AWB,            0x45B0,  36   },
            {CAM_ISP_AE,             0x4650,  18   },
            {CAM_ISP_SGG1,           0x46A0,  3    },
            {CAM_ISP_AF,             0x46B0,  34   },
            {CAM_ISP_W2G,            0x4740,  4    },
            {CAM_ISP_WBN,            0x4760,  2    },
            {CAM_ISP_FLK,            0x4770,  4    },
            {CAM_ISP_LCS,            0x4780,  6    },
            {CAM_ISP_RRZ,            0x47A0,  11   },
            {CAM_ISP_BNR,            0x4800,  22   },
            {CAM_ISP_PGN,            0x4880,  6    },
            {CAM_ISP_CFA,            0x48A0,  22   },
            {CAM_ISP_CCL,            0x4910,  3    },
            {CAM_ISP_G2G,            0x4920,  7    },
            {CAM_ISP_UNP,            0x4948,  1    },
            {CAM_ISP_C02,            0x4950,  3    },
            {CAM_ISP_MFB_CTRL,       0x4960,  1    },
            {CAM_ISP_MFB_IMGCON,     0x4964,  1    },
            {CAM_ISP_MFB_TUNECON,    0x4968,  5    },
            {CAM_ISP_LCE,            0x49C0,  12   },
            {CAM_ISP_LCE_IMAGE_SIZE, 0x49F0,  1    },
            {CAM_ISP_C42,            0x4A1C,  1    },
            {CAM_ISP_NBC,            0x4A20,  30   },
            {CAM_ISP_SEEE,           0x4AA0,  24   },
            {CAM_ISP_CRZ,            0x4B00,  15   },
            {CAM_ISP_G2C_CONV,       0x4BA0,  6    },
            {CAM_ISP_G2C_SHADE,      0x4BB8,  5    },
            {CAM_CDP_FE,             0x4C20,  4    },
            {CAM_CDP_SRZ1,           0x4C30,  9    },
            {CAM_CDP_SRZ2,           0x4C60,  9    },
            {CAM_CDP_MIX1,           0x4C90,  3    },
            {CAM_CDP_MIX2,           0x4CA0,  3    },
            {CAM_CDP_MIX3,           0x4CB0,  3    },
            {CAM_CDP_NR3D_BLENDING,  0x4CC0,  1    },
            {CAM_CDP_NR3D_FB,        0x4CC4,  3    },
            {CAM_CDP_NR3D_LMT,       0x4CD0,  10   },
            {CAM_CDP_NR3D_CTRL,      0x4CF8,  3    },
            {CAM_ISP_EIS,            0x4DC0,  9    },
            {CAM_ISP_DMX,            0x4E00,  3    },
            {CAM_ISP_BMX,            0x4E10,  3    },
            {CAM_ISP_RMX,            0x4E20,  3    },
            {CAM_ISP_UFE,            0x4E50,  1    },
            {CAM_ISP_UFD,            0x4E60,  5    },
            {CAM_ISP_SL2,            0x4F40,  7    },
            {CAM_ISP_SL2B,           0x4F60,  7    },
            {CAM_ISP_CRSP,           0x4FA0,  6    },
            {CAM_ISP_SL2C,           0x4FC0,  7    },
            {CAM_ISP_GGM_RB,         0x5000,  144  },
            {CAM_ISP_GGM_G,          0x5240,  144  },
            {CAM_ISP_GGM_TOP,        0x5480,  1    },
            {CAM_ISP_PCA_TBL,        0x5800,  360  },
            {CAM_ISP_PCA_CON,        0x5E00,  2    },
            {CAM_ISP_TILE_RING,      0x6000,  1    },
            {CAM_ISP_IMGI_SIZE,      0x6004,  1    },
            {CAM_TOP_CTL_SEL_GLOBAL_SET, 0x6040,  1    },
            {CAM_ISP_OBC_D,          0x6500,  8    },
            {CAM_ISP_LSC_D,          0x6530,  8    },
            {CAM_ISP_RPG_D,          0x6550,  6    },
            {CAM_ISP_AWB_D,          0x65B0,  36   },
            {CAM_ISP_AE_D,           0x6650,  18   },
            {CAM_ISP_SGG1_D,         0x66A0,  3    },
            {CAM_ISP_AF_D,           0x66B0,  34   },
            {CAM_ISP_W2G_D,          0x6740,  4    },
            {CAM_ISP_WBN_D,          0x6760,  2    },
            {CAM_ISP_LCS_D,          0x6780,  6    },
            {CAM_ISP_RRZ_D,          0x67A0,  11   },
            {CAM_ISP_BNR_D,          0x6800,  22   },
            {CAM_ISP_DMX_D,          0x6E00,  3    },
            {CAM_ISP_BMX_D,          0x6E10,  3    },
            {CAM_ISP_RMX_D,          0x6E20,  3    },
            {CAM_TOP_CTL_SEL_GLOBAL_CLR, 0x7040,  1    },
            {CAM_DMA_TDRI,           0x7204,  3    },
            {CAM_DMA_IMGI_F,         0x7220,  1    },
            {CAM_DMA_IMGI,           0x7230,  7    },
            {CAM_DMA_BPCI,           0x7250,  7    },
            {CAM_DMA_LSCI,           0x726C,  5    },
            {CAM_DMA_UFDI,           0x7288,  7    },
            {CAM_DMA_LCEI,           0x72A4,  7    },
            {CAM_DMA_VIPI,           0x72C0,  8    },
            {CAM_DMA_VIP2I,          0x72E0,  8    },
            {CAM_DMA_IMGO_BASE_ADDR, 0x7300,  1    },
            {CAM_DMA_IMGO,           0x7304,  7    },
            {CAM_DMA_RRZO,           0x7324,  7    },
            {CAM_DMA_LCSO,           0x7340,  7    },
            {CAM_DMA_EISO,           0x735C,  2    },
            {CAM_DMA_AFO,            0x7364,  2    },
            {CAM_DMA_ESFKO,          0x736C,  7    },
            {CAM_DMA_AAO,            0x7388,  7    },
            {CAM_DMA_VIP3I,          0x73A4,  8    },
            {CAM_DMA_UFEO,           0x73C4,  7    },
            {CAM_DMA_MFBO,           0x73E0,  8    },
            {CAM_DMA_IMG3BO,         0x7400,  8    },
            {CAM_DMA_IMG3CO,         0x7420,  8    },
            {CAM_DMA_IMG2O,          0x7440,  8    },
            {CAM_DMA_IMG3O,          0x7460,  8    },
            {CAM_DMA_FEO,            0x7480,  7    },
            {CAM_DMA_BPCI_D,         0x749C,  7    },
            {CAM_DMA_LSCI_D,         0x74B8,  5    },
            {CAM_DMA_IMGO_D,         0x74D8,  7    },
            {CAM_DMA_RRZO_D,         0x74F8,  7    },
            {CAM_DMA_LCSO_D,         0x7514,  7    },
            {CAM_DMA_AFO_D,          0x7530,  7    },
            {CAM_DMA_AAO_D,          0x754C,  7    },
            {CAM_DMA_IMGO_SV,        0x920C,  7    },
            {CAM_DMA_IMGO_SV_D,      0x922C,  7    },
            {CAM_P1_MAGIC_NUM,       0x75DC,  1    },
            {CAM_P1_RRZ_CROP_IN,     0x75E0,  1    },
            {CAM_P1_MAGIC_NUM_D,     0x75E4,  1    },
            {CAM_P1_RRZ_CROP_IN_D,   0x75E8,  1    },
            {CAM_P1_RRZ_OUT_W,       0x4094,  1    },
            {CAM_P1_RRZ_OUT_W_D,     0x409C,  1    },
            {CAM_RESERVED_00,        ISP_DRV_CQ_DUMMY_WR_TOKEN, 1  },
            {CAM_DUMMY_,             ISP_DRV_CQ_DUMMY_WR_TOKEN, 1  }};  // dummy address


 ISP_TURNING_FUNC_BIT_MAPPING gIspTuningFuncBitMapp[eIspTuningMgrFuncBit_Num]
    ={{eIspTuningMgrFuncBit_Obc             ,  eTuningCtlByte_P1,  4, -1, -1, -1, -1, -1, CAM_ISP_OBC,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Obc_d           , eTuningCtlByte_P1D, -1,  4, -1, -1, -1, -1, CAM_ISP_OBC_D,        CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Lsc             ,  eTuningCtlByte_P1,  6, -1, -1,  7, -1, -1, CAM_ISP_LSC,          CAM_DMA_LSCI},
      {eIspTuningMgrFuncBit_Lsc_d           , eTuningCtlByte_P1D, -1,  6, -1, -1,  7, -1, CAM_ISP_LSC_D,        CAM_DMA_LSCI_D},
      {eIspTuningMgrFuncBit_Bnr             ,  eTuningCtlByte_P1,  5, -1, -1,  6, -1, -1, CAM_ISP_BNR,          CAM_DMA_BPCI},
      {eIspTuningMgrFuncBit_Bnr_d           , eTuningCtlByte_P1D, -1,  5, -1, -1,  6, -1, CAM_ISP_BNR_D,        CAM_DMA_BPCI_D},
      {eIspTuningMgrFuncBit_Rpg             ,  eTuningCtlByte_P1,  7, -1, -1, -1, -1, -1, CAM_ISP_RPG,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Rpg_d           , eTuningCtlByte_P1D, -1,  7, -1, -1, -1, -1, CAM_ISP_RPG_D,        CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Rmg             ,  eTuningCtlByte_P1, 22, -1, -1, -1, -1, -1, CAM_CTL_IHDR,         CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Rmg_d           , eTuningCtlByte_P1D, -1, 22, -1, -1, -1, -1, CAM_CTL_IHDR_D,       CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Pgn             ,  eTuningCtlByte_P2, -1, -1,  1, -1, -1, -1, CAM_ISP_PGN,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Sl2             ,  eTuningCtlByte_P2, -1, -1,  2, -1, -1, -1, CAM_ISP_SL2,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Cfa             ,  eTuningCtlByte_P2, -1, -1,  3, -1, -1, -1, CAM_ISP_CFA,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Ccl             ,  eTuningCtlByte_P2, -1, -1,  4, -1, -1, -1, CAM_ISP_CCL,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_G2g             ,  eTuningCtlByte_P2, -1, -1,  5, -1, -1, -1, CAM_ISP_G2G,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Ggm_Rb          ,  eTuningCtlByte_P2, -1, -1,  6, -1, -1, -1, CAM_ISP_GGM_RB,       CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Ggm_G           ,  eTuningCtlByte_P2, -1, -1,  6, -1, -1, -1, CAM_ISP_GGM_G,        CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Mfb_TuneCon     ,  eTuningCtlByte_P2, -1, -1,  7, -1, -1, -1, CAM_ISP_MFB_TUNECON,  CAM_DUMMY_},
      {eIspTuningMgrFuncBit_G2c_Conv        ,  eTuningCtlByte_P2, -1, -1,  9, -1, -1, -1, CAM_ISP_G2C_CONV,     CAM_DUMMY_},
      {eIspTuningMgrFuncBit_G2c_Shade       ,  eTuningCtlByte_P2, -1, -1,  9, -1, -1, -1, CAM_ISP_G2C_SHADE,    CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Nbc             ,  eTuningCtlByte_P2, -1, -1, 11, -1, -1, -1, CAM_ISP_NBC,          CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Pca_Tbl         ,  eTuningCtlByte_P2, -1, -1, 12, -1, -1, -1, CAM_ISP_PCA_TBL,      CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Pca_Con         ,  eTuningCtlByte_P2, -1, -1, 12, -1, -1, -1, CAM_ISP_PCA_CON,      CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Seee            ,  eTuningCtlByte_P2, -1, -1, 13, -1, -1, -1, CAM_ISP_SEEE,         CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Lce             ,  eTuningCtlByte_P2, -1, -1, 14, -1, -1,  5, CAM_ISP_LCE,          CAM_DMA_LCEI},
      {eIspTuningMgrFuncBit_Nr3d_Blending   ,  eTuningCtlByte_P2, -1, -1, 15, -1, -1, -1, CAM_CDP_NR3D_BLENDING,CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Nr3d_Lmt        ,  eTuningCtlByte_P2, -1, -1, 15, -1, -1, -1, CAM_CDP_NR3D_LMT,     CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Sl2b            ,  eTuningCtlByte_P2, -1, -1, 16, -1, -1, -1, CAM_ISP_SL2B,         CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Sl2c            ,  eTuningCtlByte_P2, -1, -1, 17, -1, -1, -1, CAM_ISP_SL2C,         CAM_DUMMY_},
      {eIspTuningMgrFuncBit_Mix3            ,  eTuningCtlByte_P2, -1, -1, 23, -1, -1, -1, CAM_CDP_MIX3,         CAM_DUMMY_}};





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
        LOG_ERR("CQ crash error: enum sequence error\n");
        return -1;
    }
    else if(cqBaseEnum<=ISP_DRV_CQ0C_D){
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
    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
        LOG_DBG("- E. isp_cq[0x%x],[%d]",realCQIdx,moduleId,burstQIdx,dupCqIdx);
    //
    Mutex::Autolock lock(cqVirDesLock);
    //avoid the build error and hw cmdq can only read 32bit memory laybout
    MUINTPTR dummyaddr = (MUINTPTR)((MUINTPTR)mpIspVirRegAddrPA[realCQIdx] + mIspCQModuleInfo[moduleId].addr_ofst);
    cmd = (mIspCQModuleInfo[moduleId].addr_ofst&0xffff)|(((mIspCQModuleInfo[moduleId].reg_num-1)&0x3ff)<<16)|((ISP_CQ_WRITE_INST)<<26);
    //mpIspCQDescriptorVirt[realCQIdx][moduleId].v_reg_addr = &mpIspVirRegAddrPA[realCQIdx][mIspCQModuleInfo[moduleId].addr_ofst >>2 ]; // >>2 for MUINT32* pointer
    mpIspCQDescriptorVirt[realCQIdx][moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
    mpIspCQDescriptorVirt[realCQIdx][moduleId].u.cmd = cmd;
    //
    LOG_DBG("- X.");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::cqDelModule(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    int realCQIdx=getRealCQIndex(cq,burstQIdx,dupCqIdx);
    LOG_DBG(" - E. isp_cq[0x%x],[%d]",realCQIdx,moduleId);
    //
    Mutex::Autolock lock(cqVirDesLock);
    mpIspCQDescriptorVirt[realCQIdx][moduleId].u.cmd = ISP_DRV_CQ_DUMMY_WR_TOKEN;
    //
    LOG_DBG("- X.");
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

//-----------------------------------------------------------------------------
MBOOL IspDrv::setCQTriggerMode(
ISP_DRV_CQ_ENUM cq,
ISP_DRV_CQ_TRIGGER_MODE_ENUM mode,
ISP_DRV_CQ_TRIGGER_SOURCE_ENUM trig_src

)
{
    LOG_DBG("[IspDrv::setCQTriggerMode] - E. [%s], cq: %d, mode: %d, trig_src: %d.",__FUNCTION__, cq, mode, trig_src);

    switch(cq) {
        case ISP_DRV_CQ0:
            //trigger source is pass1_done
            if ( CQ_SINGLE_IMMEDIATE_TRIGGER == mode ) {
                //-Immediately trigger
                //-CQ0_MODE=1, CQ0_CONT=0
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_MODE,1,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_CONT,0,ISP_DRV_USER_ISPF);
            }
            else if ( CQ_SINGLE_EVENT_TRIGGER == mode ) {
                //-Trigger and wait trigger source
                    //-CQ0_MODE=0, CQ0_CONT=0
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_MODE,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_CONT,0,ISP_DRV_USER_ISPF);
            }
            else if ( CQ_CONTINUOUS_EVENT_TRIGGER == mode ) {
                //-Continuous mode support(without trigger)
                    //-CQ0_MODE=x, CQ0_CONT=1
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_MODE,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_CONT,1,ISP_DRV_USER_ISPF);
            }
            break;
        case ISP_DRV_CQ0_D:
            //trigger source is pass1_done
            //LOG_DBG("[IspDrv::setCQTriggerMode][ISP_DRV_CQ0_D]");
            if ( CQ_SINGLE_IMMEDIATE_TRIGGER == mode ) {
                //-Immediately trigger
                //-CQ0_MODE=1, CQ0_CONT=0
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_D_MODE,1,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_CONT_D,0,ISP_DRV_USER_ISPF);
            }
            else if ( CQ_SINGLE_EVENT_TRIGGER == mode ) {
                //-Trigger and wait trigger source
                //-CQ0_MODE=0, CQ0_CONT=0
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_D_MODE,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_CONT_D,0,ISP_DRV_USER_ISPF);
            }
            else if ( CQ_CONTINUOUS_EVENT_TRIGGER == mode ) {
                //-Continuous mode support(without trigger)
                //-CQ0_MODE=x, CQ0_CONT=1
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_D_MODE,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0_CONT_D,1,ISP_DRV_USER_ISPF);
            }
            break;

        case ISP_DRV_CQ0B:
            //-choose trigger source by CQ0B_SEL(0:img2o, 1:pass1_done)(reg_4018[11])
            if ( CQ_TRIG_BY_PASS1_DONE!=trig_src )  {
                LOG_ERR("[%s][ISP_DRV_CQ0B]:NOT Support trigger source(%d)  ",__FUNCTION__,trig_src);
            }

            if (CQ_TRIG_BY_PASS1_DONE == trig_src)
            {
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_SEL,1,ISP_DRV_USER_ISPF);
            }
            else
            {
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_SEL,0,ISP_DRV_USER_ISPF);
            }

            //
            if ( CQ_SINGLE_IMMEDIATE_TRIGGER == mode ) {
                //-Immediately trigger
                    //-CQ0B_MODE=1 , CQ0B_CONT=0
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_MODE,1,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_CONT,0,ISP_DRV_USER_ISPF);
            }
            else if ( CQ_SINGLE_EVENT_TRIGGER == mode ) {
                //-Trigger and wait trigger source
                    //-CQ0B_MODE=0, CQ0B_CONT=0
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_MODE,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_CONT,0,ISP_DRV_USER_ISPF);
            }
            else if ( CQ_CONTINUOUS_EVENT_TRIGGER == mode ) {
                //-Continuous mode support(without trigger)
                    //-CQ0B_MODE=x, CQ0B_CONT=1
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_MODE,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_CONT,1,ISP_DRV_USER_ISPF);
            }
            break;
        case ISP_DRV_CQ0C:
            //-cq0c
            //    -trigger source is imgo_done (CQ0C_IMGO_SEL=1,reg_4020[21])
            //                        OR rrzo_done(CQ0C_RRZO_SEL=1,reg_4020[24]).
            //    -always continuous mode,NO cq0c_start.
            //     If cq0c_en=1, it always load CQ when imgo_done||rrzo_done occur
            if ( CQ_TRIG_BY_IMGO_DONE!=trig_src && CQ_TRIG_BY_RRZO_DONE!=trig_src)  {
                LOG_DBG("[ISP_DRV_CQ0C]:NOT Support trigger source(%d)  ",trig_src);
            }
            else {
                ISP_WRITE_BITS(this,  CAM_CTL_CQ_EN, CQ0C_IMGO_SEL, (CQ_TRIG_BY_IMGO_DONE == trig_src)?1:0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this,  CAM_CTL_CQ_EN, CQ0C_RRZO_SEL, (CQ_TRIG_BY_RRZO_DONE == trig_src)?1:0,ISP_DRV_USER_ISPF);
            }
            break;
        case ISP_DRV_CQ0C_D:
            //-cq0c_d
            //    -trigger source is imgo_done (CQ0C_IMGO_D_SEL=1,reg_4020[5])
            //                        OR rrzo_done(CQ0C_RRZO_D_SEL=1,reg_4020[8]).
            //    -always continuous mode,NO cq0c_start.
            //     If cq0c_en=1, it always load CQ when imgo_done||rrzo_done occur
            if ( CQ_TRIG_BY_IMGO_DONE!=trig_src && CQ_TRIG_BY_RRZO_DONE!=trig_src)  {
                LOG_DBG("[ISP_DRV_CQ0C_D]:NOT Support trigger source(%d)  ",trig_src);
            }
            else {
                ISP_WRITE_BITS(this,  CAM_CTL_CQ_EN, CQ0C_IMGO_D_SEL, (CQ_TRIG_BY_IMGO_DONE == trig_src)?1:0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this,  CAM_CTL_CQ_EN, CQ0C_D_RRZO_SEL, (CQ_TRIG_BY_RRZO_DONE == trig_src)?1:0,ISP_DRV_USER_ISPF);
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
}

//-----------------------------------------------------------------------------
IspDrvImp::IspDrvImp()
{
    int i;
    GLOBAL_PROFILING_LOG_START(Event_IspDrv);   // Profiling Start.

    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    mInitCount = 0;

    mFd = -1;
    this->m_FdNd.clear();
    mIspOpenRetry = MFALSE;
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
    mFd = -1;
    this->m_FdNd.clear();
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
MINT32 IspDrvImp::isp_fd_open(const char* userName)
{
    MINT32 mIspFd = -1;
    MINT32 mIonFd = -1;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_IspFD_Open);
    //
    Mutex::Autolock lock(mFdLock);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to open isp fd\n");
        goto EXIT;
    }
    //
    LOG_INF(" - E. curUser(%s).", userName);
    //
#if defined (__ISP_USE_ION__)
    //////////////////////////////////////////////////////
    // we initial m4udrv and open ion device when local count is 1,
    // and config m4v ports when global count is 1
      mIonFd = mt_ion_open(userName);
      if (mIonFd < 0)
      {
         LOG_ERR("ion device open FAIL");
         goto EXIT;
      }
      LOG_INF("open ion id(%d).\n", mIonFd);
#endif

    // Open isp driver
    mIspOpenRetry = MFALSE;
    mIspFd = open(ISP_DRV_DEV_NAME, O_RDWR);
                                                LOCAL_PROFILING_LOG_PRINT("1st open(ISP_DRV_DEV_NAME, O_RDWR)");
    if (mIspFd < 0)    // 1st time open failed.
    {
        mIspOpenRetry = MTRUE;
        LOG_WRN("ISP kernel open 1st attempt fail, errno(%d):%s.", errno, strerror(errno));

        // Try again, using "Read Only".
        mIspFd = open(ISP_DRV_DEV_NAME, O_RDONLY);
                                                LOCAL_PROFILING_LOG_PRINT("2nd open(ISP_DRV_DEV_NAME, O_RDONLY)");
        if (mIspFd < 0) // 2nd time open failed.
        {
            LOG_ERR("ISP kernel open 2nd attempt fail, errno(%d):%s.", errno, strerror(errno));
            goto EXIT;
        }
        m_regRWMode=ISP_DRV_RWREG_MODE_RO;
                                                LOCAL_PROFILING_LOG_PRINT("mpIspVirRegMap malloc()");
    }
    LOG_INF("open isp mFd(%d).\n", mIspFd);

EXIT:
    //isp fd open fail, close ion fd
#if defined (__ISP_USE_ION__)
    if(mIspFd < 0 && mIonFd >= 0)
    {
        // we have to handle local ion drv here
        // if process A open ionID, then process B open ionID before process A call ImemDrv_uninit,
        // process A would not do close ionID.
        LOG_INF("close ion id(%d).\n",mIonFd);
        ion_close(mIonFd);
    }
#endif
    //keep Fd node list
    if(mIspFd >= 0 && mIonFd >= 0)
    {
        ISP_DRV_FD_NODE_STRUCT m_FdNode;
        m_FdNode.mIspFd = mIspFd;
        m_FdNode.mIonFd = mIonFd;
        this->m_FdNd.push_back(m_FdNode);
    }
    return mIspFd;
}
//-----------------------------------------------------------------------------
MINT32 IspDrvImp::isp_fd_close(MINT32 mIspFd)
{
    MBOOL Result = MTRUE;
    MINT32 mIonFd = -1;
    //
    Mutex::Autolock lock(mFdLock);
    LOG_INF(" - E. isp fd(%d).\n",mIspFd);
    //
    if(mIspFd >= 0)
    {
        LOG_INF("close isp mFd(%d).\n", mIspFd);
        //get ion FD, and erase it from list
        std::list<ISP_DRV_FD_NODE_STRUCT>::iterator it=this->m_FdNd.begin();
        for(int i=0;i<this->m_FdNd.size();i++){
            if(it->mIspFd == mIspFd)
            {
                mIonFd = it->mIonFd;
                LOG_INF("mIonFd(%d),at(%d/%d).\n", mIonFd,i,this->m_FdNd.size());
                this->m_FdNd.erase(it);
                break;
            }
            it++;
        }
        close(mIspFd);
    }
    //
#if defined (__ISP_USE_ION__)
      // we have to handle local ion drv here
      // if process A open ionID, then process B open ionID before process A call ImemDrv_uninit,
      // process A would not do close ionID.
    if (mIonFd >= 0)
      {
        LOG_INF("close ion id(%d).\n",mIonFd);
        ion_close(mIonFd);
      }
#endif
    //
    return Result;
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
#if (USE_ISPDRV_OPEN_FD)//let M4U opened before camera-isp

    mFd = isp_fd_open(userName);
    if(mFd < 0)
    {
        goto EXIT;
    }
    if(MFALSE == mIspOpenRetry)// 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
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
#else
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
#endif

    // Increase ISP global reference count, and reset if 1st user.
#if defined(_use_kernel_ref_cnt_)
    LOG_INF("use kernel ref. cnt.mFd(%d)",mFd);

    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    MINT32 count;   // For ISP global reference count.

    ref_cnt.ctrl = ISP_REF_CNT_GET;
    ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC;
    ref_cnt.data_ptr = &count;
    //
    if ( MTRUE == kRefCntCtrl(&ref_cnt) ) {
        //
        if (0==count) {
            LOG_DBG("DO ISP HW RESET");
            reset(ISP_DRV_RST_CAM_P1); // Do IMGSYS SW RST.
            reset(ISP_DRV_RST_CAM_P2);
            reset(ISP_DRV_RST_CAMSV);
            reset(ISP_DRV_RST_CAMSV2);
        }
        //
        ref_cnt.ctrl = ISP_REF_CNT_INC;
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
        mIspCQDescInit[i].v_reg_addr = ISP_CQ_DUMMY_PA;
    }
    mIspCQDescInit[CAM_MODULE_MAX-1].cmd_set = ISP_DRV_CQ_END_TOKEN;
    mIspCQDescInit[CAM_MODULE_MAX-1].v_reg_addr = 0x0;
    //inital total cq nun
    mCurBurstQNum=1;
    mTotalCQNum=(cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_P1DUPCQNUM) * ISP_DRV_P1_PER_CQ_SET_NUM)+\
    (cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM) * cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_P2DUPCQNUM) * ISP_DRV_P2_PER_CQ_SET_NUM);
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
        mTuningQueIdx[i].keepP2PreviewQue.pTuningQue.queSize = ISP_BASE_RANGE;
        mTuningQueIdx[i].keepP2PreviewQue.pTuningQue.pTuningQue = (MUINT32*)pmem_alloc_sync(mTuningQueIdx[i].keepP2PreviewQue.queSize, &mTuningQueIdx[i].keepP2PreviewQue.queFd);
        memset((MUINT8*)mTuningQueIdx[i].keepP2PreviewQue.pTuningQue,0,mTuningQueIdx[i].keepP2PreviewQue.queSize);
        //
        mTuningQueIdx[i].keepP1Que.pTuningQue.queSize = ISP_BASE_RANGE;
        mTuningQueIdx[i].keepP1Que.pTuningQue.pTuningQue = (MUINT32*)pmem_alloc_sync(mTuningQueIdx[i].keepP1Que.queSize, &mTuningQueIdx[i].keepP1Que.queFd);
        memset((MUINT8*)mTuningQueIdx[i].keepP1Que.pTuningQue,0,mTuningQueIdx[i].keepP1Que.queSize);
        //
        mTuningQueIdx[i].isApplyTuning = MFALSE;
        mTuningQueIdx[i].pCurReadP1TuningQue = NULL;
        mTuningQueIdx[i].pCurReadP2TuningQue = NULL;
        mTuningQueIdx[i].pCurWriteTuningQue = NULL;
        mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].eCurReadP2UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepP2PreviewUpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepReadP1CtlEnP1 = 0;
        mTuningQueIdx[i].keepReadP1CtlEnP1D = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP2 = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP1 = 0;
        mTuningQueIdx[i].keepReadP1CtlEnP1Dma = 0;
        mTuningQueIdx[i].keepReadP1CtlEnP1DDma = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP2Dma = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP1Dma = 0;
        mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curReadP1Idx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curReadP2Idx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curPreviewApplyP2Idx = ISP_TUNING_INIT_IDX;
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
        mTuningQueIdx[i].keepP2PreviewQue.queSize = ISP_BASE_RANGE;
        m_ispTmpBufInfo.size = mTuningQueIdx[i].keepP2PreviewQue.queSize;
        m_ispTmpBufInfo.useNoncache = 0; //alloc cacheable mem.
        if ( m_pIMemDrv->allocVirtBuf(&m_ispTmpBufInfo) ) {
            LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
        }
        mTuningQueIdx[i].keepP2PreviewQue.queFd = m_ispTmpBufInfo.memID;
        mTuningQueIdx[i].keepP2PreviewQue.pTuningQue = (MUINT32*)m_ispTmpBufInfo.virtAddr;
        //LOG_INF("p2cq(%d):keepP2PreviewQue(0x%x),keepP2Que(0x%x)",i,mTuningQueIdx[i].keepP2PreviewQue.pTuningQue,mTuningQueIdx[i].keepP2PreviewQue.queSize);
        memset((MUINT8*)mTuningQueIdx[i].keepP2PreviewQue.pTuningQue,0,mTuningQueIdx[i].keepP2PreviewQue.queSize);
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
        LOG_INF("p2cq(%d):keepP2PreviewQue(0x%x),keepP1Que(0x%x)",i,mTuningQueIdx[i].keepP2PreviewQue.pTuningQue,mTuningQueIdx[i].keepP1Que.pTuningQue);
        //
        mTuningQueIdx[i].isApplyTuning = MFALSE;
        mTuningQueIdx[i].pCurReadP1TuningQue = NULL;
        mTuningQueIdx[i].pCurReadP2TuningQue = NULL;
        mTuningQueIdx[i].pCurWriteTuningQue = NULL;
        mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].eCurReadP2UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepP2PreviewUpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
        mTuningQueIdx[i].keepReadP1CtlEnP1 = 0;
        mTuningQueIdx[i].keepReadP1CtlEnP1D = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP2 = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP1 = 0;
        mTuningQueIdx[i].keepReadP1CtlEnP1Dma = 0;
        mTuningQueIdx[i].keepReadP1CtlEnP1DDma = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP2Dma = 0;
        mTuningQueIdx[i].keepReadP2CtlEnP1Dma = 0;
        mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curReadP1Idx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curReadP2Idx = ISP_TUNING_INIT_IDX;
        mTuningQueIdx[i].curPreviewApplyP2Idx = ISP_TUNING_INIT_IDX;
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
#if (USE_ISPDRV_OPEN_FD)//let M4U opened before camera-isp
        isp_fd_close(mFd);
        mFd = -1;
#else
        if(mFd >= 0)
        {
            close(mFd);
            mFd = -1;
            LOCAL_PROFILING_LOG_PRINT("close isp mFd");
        }
#endif
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
            // free keepP2PreviewQue
            if((mTuningQueIdx[i].keepP2PreviewQue.queFd>=0) && (mTuningQueIdx[i].keepP2PreviewQue.pTuningQue!=0)){
                pmem_free((MUINT8*)mTuningQueIdx[i].keepP2PreviewQue.pTuningQue,mTuningQueIdx[i].keepP2PreviewQue.queSize,mTuningQueIdx[i].keepP2PreviewQue.queFd);
                mTuningQueIdx[i].keepP2PreviewQue.queFd = -1;
                mTuningQueIdx[i].keepP2PreviewQue.pTuningQue = NULL;
            }else{
                LOG_ERR("[Error]free keepP2PreviewQue error i(%d),fd(%d),va(0x%08x)",i,mTuningQueIdx[i].keepP2PreviewQue.queFd,mTuningQueIdx[i].keepP2PreviewQue.pTuningQue);
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
            mTuningQueIdx[i].pCurReadP2TuningQue = NULL;
            mTuningQueIdx[i].pCurWriteTuningQue = NULL;
            mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].eCurReadP2UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepP2PreviewUpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepReadP1CtlEnP1 = 0;
            mTuningQueIdx[i].keepReadP1CtlEnP1D = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP2 = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP1 = 0;
            mTuningQueIdx[i].keepReadP1CtlEnP1Dma = 0;
            mTuningQueIdx[i].keepReadP1CtlEnP1DDma = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP2Dma = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP1Dma = 0;
            mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
            mTuningQueIdx[i].curReadP1Idx = ISP_TUNING_INIT_IDX;
            mTuningQueIdx[i].curReadP2Idx = ISP_TUNING_INIT_IDX;
            mTuningQueIdx[i].curPreviewApplyP2Idx = ISP_TUNING_INIT_IDX;
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
            // free keepP2PreviewQue
            if((mTuningQueIdx[i].keepP2PreviewQue.queFd>=0) && (mTuningQueIdx[i].keepP2PreviewQue.pTuningQue!=0)){
                m_ispTmpBufInfo.size = mTuningQueIdx[i].keepP2PreviewQue.queSize;
                m_ispTmpBufInfo.memID = mTuningQueIdx[i].keepP2PreviewQue.queFd;
                m_ispTmpBufInfo.virtAddr = (MUINTPTR)mTuningQueIdx[i].keepP2PreviewQue.pTuningQue;
                if ( m_pIMemDrv->freeVirtBuf(&m_ispTmpBufInfo) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
                }
                mTuningQueIdx[i].keepP2PreviewQue.queFd = -1;
                mTuningQueIdx[i].keepP2PreviewQue.pTuningQue = NULL;
            }else{
                LOG_ERR("[Error]free keepP2PreviewQue error i(%d),fd(%d),va(0x%08x)",i,mTuningQueIdx[i].keepP2PreviewQue.queFd,mTuningQueIdx[i].keepP2PreviewQue.pTuningQue);
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
            mTuningQueIdx[i].pCurReadP2TuningQue = NULL;
            mTuningQueIdx[i].pCurWriteTuningQue = NULL;
            mTuningQueIdx[i].eCurReadP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].eCurReadP2UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepP2PreviewUpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepP1UpdateFuncBit = eIspTuningMgrFunc_Null;
            mTuningQueIdx[i].keepReadP1CtlEnP1 = 0;
            mTuningQueIdx[i].keepReadP1CtlEnP1D = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP2 = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP1 = 0;
            mTuningQueIdx[i].keepReadP1CtlEnP1Dma = 0;
            mTuningQueIdx[i].keepReadP1CtlEnP1DDma = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP2Dma = 0;
            mTuningQueIdx[i].keepReadP2CtlEnP1Dma = 0;
            mTuningQueIdx[i].curWriteIdx = ISP_TUNING_INIT_IDX;
            mTuningQueIdx[i].curReadP1Idx = ISP_TUNING_INIT_IDX;
            mTuningQueIdx[i].curReadP2Idx = ISP_TUNING_INIT_IDX;
            mTuningQueIdx[i].curPreviewApplyP2Idx = ISP_TUNING_INIT_IDX;
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
    mpIspCQDescBufferPhy = NULL;
    //
    if(mFd >= 0)
    {
        #if defined(_use_kernel_ref_cnt_)
        ISP_REF_CNT_CTRL_STRUCT ref_cnt;
        MINT32 count;   // For ISP global reference count.

        ref_cnt.ctrl = ISP_REF_CNT_DEC;
        ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC;
        ref_cnt.data_ptr = &count;
        //
        if ( MFALSE == kRefCntCtrl(&ref_cnt) ) {
            //
            LOG_ERR("ISP_REF_CNT_GET fail, errno(%d):%s.", errno, strerror(errno));
        }
        LOG_INF("ISP Global Count: %d.", count);
        #endif

#if (USE_ISPDRV_OPEN_FD)//
        isp_fd_close(mFd);
#else
        close(mFd);
#endif
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

    //enable DMA error check
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMGI_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_BPCI_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_LSCI_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_UFDI_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);

#if 0
    writeReg((((UINT32)&ispReg.CAM_LCEI_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
#endif
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_VIPI_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_VIP2I_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_VIP3I_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMGO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_RRZO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_LCSO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_ESFKO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_AAO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_UFEO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_MFBO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG3BO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG3CO_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG2O_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG3O_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
#if 0
    writeReg((((UINT32)&ispReg.CAM_FEO_ERR_STAT-(UINT32)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
#endif
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_BPCI_D_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_LSCI_D_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMGO_D_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_RRZO_D_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_LCSO_D_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_AFO_D_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_AAO_D_ERR_STAT-(MUINT8*)&ispReg)), 0xFFFF0000, ISP_DRV_USER_ISPF, 0);
    //disable p2 DB
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_SEL_GLOBAL_P2-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
    //
    LOG_INF("-");

    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::checkCQBufAllocated(void)
{
    return mbBufferAllocated;
}


// burst Queue num are only supported in p2
MBOOL IspDrvImp::cqTableControl(ISP_DRV_CQTABLE_CTRL_ENUM cmd,int burstQnum)
{
    MUINTPTR dummyaddr;
    bool ret=true;

    switch(cmd)
    {
        case ISP_DRV_CQTABLE_CTRL_ALLOC:
            //[1] virtual isp(virtual register map)
            mIspVirRegSize = sizeof(MUINT32) * ISP_BASE_RANGE * mTotalCQNum;
            if(mpIspVirRegAddrPA!=NULL)
            {
                free(mpIspVirRegAddrPA);
            }
            if(mpIspVirRegAddrVA!=NULL)
            {
                free(mpIspVirRegAddrVA);
            }
            mpIspVirRegAddrPA=(MUINT32 **)malloc(mTotalCQNum*sizeof(MUINT32*));
#if 0
            for( int i=0; i<mTotalCQNum; i++ )
            {
                mpIspVirRegAddrPA[i] = ((MUINT32 *)(mpIspVirRegAddrPA+mTotalCQNum)) + i*1*sizeof(MUINT32);
            }
#endif
            if(mpIspVirRegAddrPA==NULL)
            {
                LOG_ERR("malloc mpIspVirRegAddrPA fail");
                ret = false;
                break;
            }
            mpIspVirRegAddrVA=(MUINT32 **)malloc(mTotalCQNum*sizeof(MUINT32 *));
#if 0
            for( int i=0; i<mTotalCQNum; i++ )
            {
                mpIspVirRegAddrVA[i] = ((MUINT32 *)(mpIspVirRegAddrVA+mTotalCQNum)) + i*1*sizeof(MUINT32);
            }
#endif
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
            mpIspVirRegBufferBaseAddr = (MUINT32*)( (((unsigned long)m_ispVirRegBufInfo.virtAddr) + ISP_DRV_VIR_ADDR_ALIGN) & (~ISP_DRV_VIR_ADDR_ALIGN) );
            //
            if ( m_pIMemDrv->mapPhyAddr(&m_ispVirRegBufInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
                ret=false;
                break;
            }
            //virtual isp base adrress should be 4 bytes alignment
            mpIspVirRegAddrPA[0] =  (MUINT32*)( (((unsigned long)m_ispVirRegBufInfo.phyAddr) + ISP_DRV_VIR_ADDR_ALIGN)  & (~ISP_DRV_VIR_ADDR_ALIGN) );
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
            //Bug Fix.
            if(mpIspCQDescriptorPhy!=NULL)
            {
                free(mpIspCQDescriptorPhy);
            }
            mpIspCQDescriptorVirt=(ISP_DRV_CQ_CMD_DESC_STRUCT **)malloc(mTotalCQNum*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT *));
#if 0 //
            for( int i=0; i<mTotalCQNum; i++ )
            {
                mpIspCQDescriptorVirt[i] = ((ISP_DRV_CQ_CMD_DESC_STRUCT *)(mpIspCQDescriptorVirt+mTotalCQNum)) + i*1*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT);
            }
#endif
            if(mpIspCQDescriptorVirt==NULL)
            {
                LOG_ERR("malloc mpIspCQDescriptorVirt fail");
                ret = false;
                break;
            }
            mpIspCQDescriptorPhy=(MUINT32 **)malloc(mTotalCQNum*sizeof(MUINT32*));
            if(mpIspCQDescriptorPhy==NULL)
            {
                LOG_ERR("malloc mpIspCQDescriptorPhy fail");
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
            mpIspCQDescBufferVirt = (MUINT32*)( (((unsigned long)m_ispCQDescBufInfo.virtAddr)+ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) & (~ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) );
            if ( m_pIMemDrv->mapPhyAddr(&m_ispCQDescBufInfo) ) {
                LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
                ret=false;
                break;
            }
            //CQ decriptor base address should be 8 bytes alignment
            mpIspCQDescBufferPhy =  (MUINT32*)( (((unsigned long)m_ispCQDescBufInfo.phyAddr)+ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) & (~ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) );

            LOG_DBG("cq(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)",mIspCQDescSize,m_ispCQDescBufInfo.size, \
                                                            m_ispCQDescBufInfo.virtAddr,m_ispCQDescBufInfo.phyAddr, \
                                                            mpIspCQDescBufferVirt,mpIspCQDescBufferPhy);
            #endif  // __PMEM_ONLY__

            // initialization
            dummyaddr =(MUINTPTR)((MUINTPTR)mpIspVirRegAddrPA[0] + ISP_DRV_CQ_DUMMY_WR_TOKEN);
            for (int i = 0; i < (CAM_MODULE_MAX-1); i++) {
              //avoid the build error and hw cmdq can only read 32bit memory laybout
              //mIspCQDescInit[i].v_reg_addr = (&mpIspVirRegAddrPA[0][(ISP_DRV_CQ_DUMMY_WR_TOKEN/4)]);     //why use mpIspVirRegAddrPA[0] for all initialization
              mIspCQDescInit[i].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF;     //why use mpIspVirRegAddrPA[0] for all initialization
            }
            for (int i = 0; i < mTotalCQNum; i++)
            {
                if ( 0 == i ) {
                    mpIspCQDescriptorVirt[i] = (ISP_DRV_CQ_CMD_DESC_STRUCT*)mpIspCQDescBufferVirt;
                    mpIspCQDescriptorPhy[i] = mpIspCQDescBufferPhy;
                }
                else {
                    mpIspCQDescriptorVirt[i] = (ISP_DRV_CQ_CMD_DESC_STRUCT *)((unsigned long)mpIspCQDescriptorVirt[i-1] + sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_MODULE_MAX);
                    mpIspCQDescriptorPhy[i] = (MUINT32*)(((unsigned long)mpIspCQDescriptorPhy[i-1]) + ((sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_MODULE_MAX)));
                }
                memcpy((MUINT8*)mpIspCQDescriptorVirt[i], mIspCQDescInit, sizeof(mIspCQDescInit));
                LOG_INF("cq:[%d],mpIspCQDescriptor:Virt[0x%08x]/Phy[0x%08x],size/num(%d/%d/%d)",i,mpIspCQDescriptorVirt[i],mpIspCQDescriptorPhy[i],sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT),CAM_MODULE_MAX,sizeof(mIspCQDescInit));
            }

            #if defined(_rtbc_use_cq0c_)
            //reset cq0c all 0 for rtbc
            if (sizeof(CQ_RTBC_RING_ST) <= sizeof(mIspCQDescInit)) {
                memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C], 0, sizeof(CQ_RTBC_RING_ST));
                memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C_D], 0, sizeof(CQ_RTBC_RING_ST));
                memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C_CAMSV], 0, sizeof(CQ_RTBC_RING_ST));
                memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C_CAMSV2], 0, sizeof(CQ_RTBC_RING_ST));
            }
            else {
                LOG_ERR("rtbc data too large(%d)>(%d)",sizeof(CQ_RTBC_RING_ST),sizeof(mIspCQDescInit));
                   return MFALSE;
            }
            #endif  // _rtbc_use_cq0c_
            mbBufferAllocated = MTRUE;
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
            if(mpIspVirRegAddrPA!=NULL)
            {
                free(mpIspVirRegAddrPA);
            }
            mpIspVirRegAddrPA=NULL;
            if(mpIspVirRegAddrVA!=NULL)
            {
                free(mpIspVirRegAddrVA);
            }
            mpIspVirRegAddrVA=NULL;
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
            mbBufferAllocated = MFALSE;
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
            //hint: burstQnum may not sync in user/kernel space cuz mw flow that user space thread all release isp driver
            //       but imem driver may not be released (some buffer would be keeped)
            //action, all burstQ number are based on user space
            ret = ioctl(mFd,ISP_QUERY_BURSTQNUM,&value);
            if(ret<0)
            {
                LOG_ERR("ISP_QUERY_BURSTQNUM fail(%d).", ret);
            }
            if(value != mCurBurstQNum)
            {
                LOG_WRN("update kernel bQ cuz timing(%d,%d)",mCurBurstQNum,value);
                value=mCurBurstQNum;
                ret=ioctl(mFd,ISP_UPDATE_BURSTQNUM,&value);
                if(ret<0)
                {
                    LOG_ERR("ISP_UPDATE_BURSTQNUM fail(%d).", ret);
                }
            }
            num=mCurBurstQNum;
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
            mCurBurstQNum=newValue;
            //update value in kernel layer
            value=newValue;
            ret=ioctl(mFd,ISP_UPDATE_BURSTQNUM,&value);
            if(ret<0)
            {
                LOG_ERR("ISP_UPDATE_BURSTQNUM fail(%d).", ret);
            }
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

    //check tpipe
    LOG_INF("CAM_CTL_TCM_EN=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_TCM_EN-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_TDR_DBG_STATUS=0x%08x",readReg((((MUINT8*)&ispReg.CAM_CTL_TDR_DBG_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    //
    //check DMA error
    LOG_INF("CAM_IMGI_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMGI_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_BPCI_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_BPCI_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_LSCI_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_LSCI_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_UFDI_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_UFDI_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
//
//QQ    LOG_INF("CAM_LCEI_ERR_STAT=0x%08x",readReg((((UINT32)&ispReg.CAM_LCEI_ERR_STAT-(UINT32)&ispReg)), ISP_DRV_USER_ISPF));

    LOG_INF("CAM_VIPI_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_VIPI_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_VIP2I_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_VIP2I_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_VIP3I_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_VIP3I_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_ESFKO_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_ESFKO_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_MFBO_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_MFBO_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_IMG3BO_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG3BO_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_IMG3CO_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG3CO_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_IMG2O_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG2O_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_IMG3O_ERR_STAT=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_IMG3O_ERR_STAT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
//QQ    LOG_INF("CAM_FEO_ERR_STAT=0x%08x",readReg((((UINT32)&ispReg.CAM_FEO_ERR_STAT-(UINT32)&ispReg)), ISP_DRV_USER_ISPF));
    //
    //check RAW
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x1030, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RAW]raw_cfa_debug_0=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x1031, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RAW]raw_cfa_debug_1=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x1032, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RAW]raw_cfa_debug_2=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x1033, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RAW]raw_cfa_debug_3=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x2107, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RAW]mfb_debug_data=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    //
    //check YUV
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x3016, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]lce_out_debug_0=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x3017, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]lce_out_debug_1=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x3018, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]lce_out_debug_2=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x3019, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]lce_out_debug_3=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x301A, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]crsp_out_debug_0=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x301B, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]crsp_out_debug_1=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x301C, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]crsp_out_debug_2=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x301D, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]crsp_out_debug_3=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x301E, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]c24b_out_debug_0=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x301F, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]c24b_out_debug_1=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x3020, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]c24b_out_debug_2=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x3021, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]c24b_out_debug_3=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x3014, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[YUV]mdp_crop_debug=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    //
    //check RGB
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x2010, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RGB]rgb_RGB_debug_0=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x2011, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RGB]rgb_RGB_debug_1=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x2012, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RGB]rgb_RGB_debug_2=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x2013, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RGB]rgb_RGB_debug_3=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x2017, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[RGB]mfb_debug_data=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    //
    //check TOP
    LOG_INF("CAM_CTL_RAW_REQ_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_RAW_REQ_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_DMA_REQ_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DMA_REQ_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_RGB_REQ_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_RGB_REQ_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_YUV_REQ_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_YUV_REQ_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_RAW_RDY_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_RAW_RDY_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_DMA_RDY_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DMA_RDY_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_RGB_RDY_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_RGB_RDY_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("CAM_CTL_YUV_RDY_STATUS=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_YUV_RDY_STATUS-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    //
    //check CQ
    writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_SET-(MUINT8*)&ispReg)), 0x6000, ISP_DRV_USER_ISPF, 0);
    LOG_INF("[CQ]cq status=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_DBG_PORT-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    //
    //check MDP
    LOG_INF("[MDP]CAM_MDP_CROP_X=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_MDP_CROP_X-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    LOG_INF("[MDP]CAM_MDP_CROP_Y=0x%08x",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_MDP_CROP_Y-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
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
    LOG_INF("(0x%04x,0x%08x)(0x%04x,0x%08x)",0x5ff0,readReg(0x5ff0, ISP_DRV_USER_ISPF),0x5ff4,readReg(0x5ff4, ISP_DRV_USER_ISPF));

    for(i=0x7000;i<=0x7600;i+=20){
        LOG_INF("(0x%04x,0x%08x)(0x%04x,0x%08x)(0x%04x,0x%08x)(0x%04x,0x%08x)(0x%04x,0x%08x)",
            i,readReg(i, ISP_DRV_USER_ISPF),i+4,readReg(i+4, ISP_DRV_USER_ISPF),i+8,readReg(i+8, ISP_DRV_USER_ISPF),i+12,readReg(i+12, ISP_DRV_USER_ISPF),i+16,readReg(i+16, ISP_DRV_USER_ISPF));
    }




    return MTRUE;
}

#define DMA_CHK(dma,out){\
    out = 0;\
    if(dma[0] & 0xffff){\
        LOG_ERR("IMGI ERR:0x%x\n",dma[0]);\
        out = 1;\
    }\
    if(dma[1] & 0xffff){\
        LOG_ERR("BPCI ERR:0x%x\n",dma[1]);\
        out = 1;\
    }\
    if(dma[2] & 0xffff){\
        LOG_ERR("LSCI ERR:0x%x\n",dma[2]);\
        out = 1;\
    }\
    if(dma[3] & 0xffff){\
        LOG_ERR("UFDI ERR:0x%x\n",dma[3]);\
        out = 1;\
    }\
    if(dma[4] & 0xffff){\
        LOG_ERR("LCEI ERR:0x%x\n",dma[4]);\
        out = 1;\
    }\
    if(dma[5] & 0xffff){\
        LOG_ERR("IMGO ERR:0x%x\n",dma[5]);\
        out = 1;\
    }\
    if(dma[6] & 0xffff){\
        LOG_ERR("RRZO ERR:0x%x\n",dma[6]);\
        out = 1;\
    }\
    if(dma[7] & 0xffff){\
        LOG_ERR("LCSO ERR:0x%x\n",dma[7]);\
        out = 1;\
    }\
    if(dma[8] & 0xffff){\
        LOG_ERR("ESFKO ERR:0x%x\n",dma[8]);\
        out = 1;\
    }\
    if(dma[9] & 0xffff){\
        LOG_ERR("AAO ERR:0x%x\n",dma[9]);\
        out = 1;\
    }\
    if(dma[10] & 0xffff){\
        LOG_ERR("UFEO ERR:0x%x\n",dma[10]);\
        out = 1;\
    }\
    if(dma[11] & 0xffff){\
        LOG_ERR("BPCI_D ERR:0x%x\n",dma[11]);\
        out = 1;\
    }\
    if(dma[12] & 0xffff){\
        LOG_ERR("LSCI_D ERR:0x%x\n",dma[12]);\
        out = 1;\
    }\
    if(dma[13] & 0xffff){\
        LOG_ERR("IMGO_D ERR:0x%x\n",dma[13]);\
        out = 1;\
    }\
    if(dma[14] & 0xffff){\
        LOG_ERR("RRZO_D ERR:0x%x\n",dma[14]);\
        out = 1;\
    }\
    if(dma[15] & 0xffff){\
        LOG_ERR("LSCO_D ERR:0x%x\n",dma[15]);\
        out = 1;\
    }\
    if(dma[16] & 0xffff){\
        LOG_ERR("AFO_D ERR:0x%x\n",dma[16]);\
        out = 1;\
    }\
    if(dma[17] & 0xffff){\
        LOG_ERR("AAO_D ERR:0x%x\n",dma[17]);\
        out = 1;\
    }\
}

//idx0:0 for p1 , 1 for p1_d , 2 for camsv, 3 for camsv_d
//idx1:debug flag init.    MTRUE/FALSE, if idx1 is true , idx2  map to function_DbgLogEnable_VERBOSE,
//                                       idx3:function_DbgLogEnable_DEBUG, idx4 map to function_DbgLogEnable_INFO
//idx2:prt log.                MTRUE/FALSE
//idx3:get sof            ,return sof at idx0
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
            if(ioctl(mFd,ISP_DEBUG_FLAG,(unsigned char*)&_flag[0]) < 0){
                LOG_ERR("kernel log enable error\n");
            }
            //LOG_INF("ethan test %d_%x(%d %d %d)\n",_flag[1],_flag[0],isp_drv_DbgLogEnable_VERBOSE,isp_drv_DbgLogEnable_DEBUG,isp_drv_DbgLogEnable_INFO);
        }
        if(p1[2] == MTRUE){
            _flag[0] = p1[0];
            LOG_DBG("prt kernel log");
            if(ioctl(mFd,ISP_DUMP_ISR_LOG,_flag) < 0){
                LOG_ERR("kernel log enable error\n");
            }
        }
        else if(p1[3] == MTRUE){
            if(ioctl(mFd,ISP_GET_CUR_SOF,(unsigned char*)p1) < 0){
                LOG_ERR("dump sof error\n");
            }
        }
        else if(p1[4] == MTRUE){
            MUINT32 _flag[18] = {0};
            if(ioctl(mFd,ISP_GET_DMA_ERR,(unsigned char*)&_flag[0]) < 0){
                LOG_ERR("dump sof error\n");
            }
            DMA_CHK(_flag,p1[4]);
        }
        else if(p1[5] == 1){
            MUINT32 _flag[4] = {0};
            if(ioctl(mFd,ISP_GET_INT_ERR,(unsigned char*)&_flag[0]) < 0){
                LOG_ERR("dump int_err error\n");
            }else
                LOG_ERR("p1 int err: 0x%x,0x%x,0x%x,0x%x",_flag[0],_flag[1],_flag[2],_flag[3]);
        }
        else if(p1[5] == 2){
            if(ioctl(mFd,ISP_GET_DROP_FRAME,p1) < 0){
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
    if(ioctl(mFd,ISP_SET_FPS,&_fps) < 0){
        LOG_ERR("SetFPS error\n");
    }
    return MTRUE;
#else
    LOG_DBG("FPS can't pass 2 kernel\n");
    return MFALSE;
#endif
}

//-----------------------------------------------------------------------------
MBOOL IspDrvImp::dumpCQTable(ISP_DRV_CQ_ENUM cq, MINT32 burstQIdx,MUINT32 dupCqIdx, ISP_DRV_CQ_CMD_DESC_STRUCT* cqDesVa, MUINT32* cqVirVa)
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
        case ISP_DRV_CQ0_D:
            cqLabel=0xD0;
            break;
        case ISP_DRV_CQ0B_D:
            cqLabel=0xD0B;
            break;
        case ISP_DRV_CQ0C_D:
            cqLabel=0xD0C;
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
        LOG_INF("[BasePA]CQ1(0x%x),CQ2(0x%x),CQ3(0x%x)",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_CQ1_BASEADDR-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF),
            readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_CQ2_BASEADDR-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF),readReg((((MUINT8*)&ispReg.CAM_CTL_CQ3_BASEADDR-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
        LOG_INF("[REG]P2_EN(0x%x),P2_EN_DMA(0x%x),SCENARIO_REG(0x%x),CQ_EN_P2(0x%x)",readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_EN_P2-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF),
            readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_EN_P2_DMA-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF),readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_SCENARIO-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF),
            readReg(((MUINT32)((MUINT8*)&ispReg.CAM_CTL_CQ_EN_P2-(MUINT8*)&ispReg)), ISP_DRV_USER_ISPF));
    } else { // for dpframe work process
        pIspDesCqVa = (ISP_DRV_CQ_CMD_DESC_STRUCT*)cqDesVa;
        pIspVirCqVa = (MUINT32*)cqVirVa;
    }

    for(i=0;i<=CAM_DMA_IMGO_SV_D;i++){
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
    MUINT32 dumpStatus = (CAM_CTL_INT_P1_STATUS_PASS1_DON_ST|CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST| \
    CAMSV2_INT_STATUS_PASS1_DON_ST|CAMSV_INT_STATUS_PASS1_DON_ST|CAM_CTL_INT_P1_STATUS_SOF1_INT_ST|CAM_CTL_INT_P1_STATUS_D_SOF1_INT_ST| \
    CAMSV_INT_STATUS_TG_SOF1_ST|CAMSV2_INT_STATUS_TG_SOF1_ST|CAM_CTL_INT_P1_STATUS_VS1_INT_ST|CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST);
    //
    IspDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    //
    LOG_DBG(" - E. Type(%d),Status(0x%08x),Timeout(%d).", pwaitIrq->Type, pwaitIrq->Status, pwaitIrq->Timeout);
    //
    ISP_DRV_WAIT_IRQ_STRUCT waitirq;
    waitirq.Clear=pwaitIrq->Clear;
    waitirq.Type=pwaitIrq->Type;
    waitirq.Status=pwaitIrq->Status;
    waitirq.Timeout=pwaitIrq->Timeout;
    waitirq.UserName=pwaitIrq->UserName;
    waitirq.irq_TStamp=pwaitIrq->irq_TStamp;
    waitirq.UserNumber=pwaitIrq->UserNumber;
    waitirq.EisMeta.tLastSOF2P1done_sec=0;
    waitirq.EisMeta.tLastSOF2P1done_usec=0;
    waitirq.SpecUser=pwaitIrq->SpecUser;
        //force driver to use v3 flow
    if(pwaitIrq->UserNumber==0 ||pwaitIrq->UserInfo.UserKey>=0)
    {   //v3
         if((MINT32)pwaitIrq->UserInfo.UserKey >= 0 ) //v3 used
         {
             waitirq.UserInfo=pwaitIrq->UserInfo;
             waitirq.UserNumber=0;
              waitirq.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;
              waitirq.Status=0x0;
          }
           else
           {   //driver in v1, force to user v3
             waitirq.UserInfo.Status=pwaitIrq->Status;
              waitirq.UserInfo.Type=pwaitIrq->Type;
              waitirq.UserInfo.UserKey=0;
           }
    }

    //v3
    waitirq.TimeInfo.tLastEvent_sec=0;
    waitirq.TimeInfo.tLastEvent_usec=0;
    waitirq.TimeInfo.tmark2read_sec=0;
    waitirq.TimeInfo.tmark2read_usec=0;
    waitirq.TimeInfo.tevent2read_sec=0;
    waitirq.TimeInfo.tevent2read_usec=0;
    waitirq.TimeInfo.passedbySigcnt=0;

    //dump only pass1|pass1_d|camsv|camsv2  sof|done
    if((pwaitIrq->bDumpReg == 0xfe) || ((pwaitIrq->Status & dumpStatus) == 0))
        waitirq.bDumpReg = MFALSE;
    else
        waitirq.bDumpReg = MTRUE;

    Ta=dbgTmr.getUs();
    Ret = ioctl(mFd,ISP_WAIT_IRQ,&waitirq);
    Tb=dbgTmr.getUs();  //us
    //receive restart system call signal
    if(Ret== (-SIG_ERESTARTSYS))
    {   //change to non-clear wait and start to wait again
        LOG_INF("ioctrl Ret(%d),Type(%d),Status(0x%08x),Timeout(%d),Tb-Ta(%d us).",Ret,pwaitIrq->Type, pwaitIrq->Status, pwaitIrq->Timeout,Tb-Ta);
        waitirq.Timeout=waitirq.Timeout - ((Tb-Ta)/1000);
        while((waitirq.Timeout > 0) && (Ret== (-SIG_ERESTARTSYS)))//receive restart system call again
        {
            waitirq.Clear=ISP_DRV_IRQ_CLEAR_NONE;
            Ta=dbgTmr.getUs();
                Ret = ioctl(mFd,ISP_WAIT_IRQ,&waitirq);
            Tb=dbgTmr.getUs();
                waitirq.Timeout=waitirq.Timeout - ((Tb-Ta)/1000);
            }
            LOG_INF("Leave ERESTARTSYS,Type(%d),Status(0x%08x),Timeout(%d),Tb-Ta(%d us).",Ret,pwaitIrq->Type, pwaitIrq->Status, pwaitIrq->Timeout,Tb-Ta);
    }

    //err handler
    if(Ret<0 || MTRUE==isp_drv_DbgLogEnable_DEBUG){
        LOG_INF("Type(%d),Status(0x%08x),Timeout(%d).", pwaitIrq->Type, pwaitIrq->Status, pwaitIrq->Timeout);
        if(pwaitIrq->Type==4 && (pwaitIrq->Status==0x80||pwaitIrq->Status==0x40||pwaitIrq->Status==0x20||pwaitIrq->Status==0x2)){
            dumpCQTable(ISP_DRV_CQ0);
            if(pwaitIrq->Status==0x2 || pwaitIrq->Status==0x20){
                dumpCQTable(ISP_DRV_CQ01,0,0);
                dumpCQTable(ISP_DRV_CQ01,0,1);
            }else if(pwaitIrq->Status==0x40){
                dumpCQTable(ISP_DRV_CQ02,0,0);
                dumpCQTable(ISP_DRV_CQ02,0,1);
            }else if(pwaitIrq->Status==0x80){
                dumpCQTable(ISP_DRV_CQ03,0,0);
                dumpCQTable(ISP_DRV_CQ03,0,1);
            }
        }
    }
    if(Ret < 0) {
        LOG_ERR("ISP_WAIT_IRQ fail(%d). Type(%d), Status(0x%08x), Timeout(%d).", Ret, pwaitIrq->Type, pwaitIrq->Status, pwaitIrq->Timeout);
        return MFALSE;
    }
    pwaitIrq->irq_TStamp=waitirq.irq_TStamp;

    if(pwaitIrq->UserNumber==0 ||pwaitIrq->UserInfo.UserKey>=0)
    {   //v3
        //return information
        pwaitIrq->TimeInfo=waitirq.TimeInfo;
        LOG_DBG("christopher-1 sigNum(%d/%d)",waitirq.TimeInfo.passedbySigcnt,pwaitIrq->TimeInfo.passedbySigcnt);
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
    }

    pwaitIrq->EisMeta=waitirq.EisMeta;
    if(pwaitIrq->EisMeta.tLastSOF2P1done_sec < 0)
    { pwaitIrq->EisMeta.tLastSOF2P1done_sec=0;}
    if(pwaitIrq->EisMeta.tLastSOF2P1done_usec < 0)
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
    LOG_INF(" - E. Type(%d),user(%d), Status(%d)",pReadIrq->Type,pReadIrq->UserNumber, pReadIrq->Status);
    //
    ReadIrq.Type = pReadIrq->Type;
    ReadIrq.UserNumber = pReadIrq->UserNumber;
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
    return MTRUE;
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
    LOG_DBG(" - E. Type(%d),user(%d), Status(%d)",CheckIrq.Type,CheckIrq.UserNumber, CheckIrq.Status);
    //
    ReadIrq.Type = CheckIrq.Type;
    ReadIrq.UserNumber = CheckIrq.UserNumber;
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
    LOG_INF(" - E. Type(%d),user(%d), Status(%d)",ClearIrq.Type,ClearIrq.UserNumber, ClearIrq.Status);
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
MBOOL IspDrvImp::unregisterIrq(ISP_DRV_WAIT_IRQ_STRUCT waitIrq)
{
    #if 0
    MINT32 Ret;
    //
    Ret = ioctl(mFd,ISP_UNREGISTER_IRQ,&waitIrq);
    if(Ret < 0) {
        LOG_ERR("ISP_UNREGISTER_IRQ fail(%d). Type(%d), Status(0x%08x), Timeout(%d).", Ret, waitIrq.Type, waitIrq.Status, waitIrq.Timeout);
        return MFALSE;
    }
    #endif
    return MTRUE;
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
    Irqinfo.UserNumber=0;
    Irqinfo.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;//no use in v3
    Irqinfo.Status=0x0;//no use in v3
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
    LOG_INF("flush irq, user key/type/status (%d/%d/0x%x)",Irqinfo.UserInfo.UserKey,Irqinfo.UserInfo.Type,Irqinfo.UserInfo.Status);

    MINT32 Ret;
    Irqinfo.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    Irqinfo.Timeout=0;
    Irqinfo.bDumpReg=0;
    if(Irqinfo.UserNumber < 0){//v3 flow
        Irqinfo.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;    //no use in v3
        Irqinfo.Status=0x0; //no use in v3
    }
    Ret = ioctl(mFd,ISP_FLUSH_IRQ_REQUEST,&Irqinfo);
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
        LOG_DBG("christopher-2 sigNum(%d/%d)",qirq.TimeInfo.passedbySigcnt,Irqinfo->TimeInfo.passedbySigcnt);
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
    //
    LOG_INF("ISP SW RESET[0x%08x]",mFd);
    //
    if(rstpath==ISP_DRV_RST_CAM_P1)
    {
        Ret = ioctl(mFd,ISP_RESET_CAM_P1,NULL);
    }
    else if(rstpath==ISP_DRV_RST_CAM_P2)
    {
        Ret = ioctl(mFd,ISP_RESET_CAM_P2,NULL);
    }
    else if(rstpath==ISP_DRV_RST_CAMSV)
    {
        Ret = ioctl(mFd,ISP_RESET_CAMSV,NULL);
    }
    else if(rstpath==ISP_DRV_RST_CAMSV2)
    {
        Ret = ioctl(mFd,ISP_RESET_CAMSV2,NULL);
    }
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
        SAFE_HEAD;
        //value=*((MUINT32*)((int)this->getMMapRegAddr()+(int)Addr));
        value = pISPmmapAddr[(Addr>>2)];
        SAFE_TAIL;
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
    unsigned long Data,
    MINT32      userEnum,
    MINT32      caller)
{
    //
    //LOG_VRB("Addr(0x%08X),Data(0x%08X)",Addr,Data);
    //
    MUINT32* pISPmmapAddr = this->getMMapRegAddr();
    MBOOL ret=0;
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
                    dsb();
                }
                else
                {
                    ISP_DRV_REG_IO_STRUCT RegIo;
                    RegIo.Addr = Addr;
                    if(!writeRegsviaIO(&RegIo, 1))
                    {
                        return 0;
                    }
                    dsb();
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
    ISP_REG_IO_STRUCT IspRegIo;
    //
    //LOG_DBG("Count(%d)",Count);
    //
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    //
    IspRegIo.pData = (ISP_REG_STRUCT*)pRegIo;
    IspRegIo.Count = Count;
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
    ISP_REG_IO_STRUCT IspRegIo;
    //
    //LOG_MSG("Count(%d) ",Count);
    //
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    //
    IspRegIo.pData = (ISP_REG_STRUCT*)pRegIo;
    IspRegIo.Count = Count;
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
    return NULL;
}
//-----------------------------------------------------------------------------
/////
//temp remove later
isp_reg_t* IspDrvImp::getRegAddrMap(void)
{
    LOG_ERR("We do not support this interface and would remove this later\n");
    return NULL;
}

//-----------------------------------------------------------------------------

#if defined(_use_kernel_ref_cnt_)

MBOOL IspDrvImp::kRefCntCtrl(ISP_REF_CNT_CTRL_STRUCT* pCtrl)
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

    ISP_BUFFER_CTRL_STRUCT *pbuf_ctrl = (ISP_BUFFER_CTRL_STRUCT *)pBuf_ctrl;

    //for debug log ONLY
    if ( ISP_RT_BUF_CTRL_MAX == pbuf_ctrl->ctrl ) {
        LOG_DBG("[rtbc]cq(%d),vIspV/P(0x%x,0x%x),descV/P(0x%x,0x%x),mmap(0x%x)", \
            pbuf_ctrl->buf_id, \
            IspDrv::mpIspVirRegAddrVA[pbuf_ctrl->buf_id], \
            IspDrv::mpIspVirRegAddrPA[pbuf_ctrl->buf_id], \
            IspDrv::mpIspCQDescriptorVirt[pbuf_ctrl->buf_id], \
            IspDrv::mpIspCQDescriptorPhy[pbuf_ctrl->buf_id], \
            this->getMMapRegAddr());
        return MTRUE;
    }

//    if(pbuf_ctrl->buf_id == _camsv_imgo_ || pbuf_ctrl->buf_id == _camsv2_imgo_){
        //no cq
//    }
//    else {
#if defined(_rtbc_use_cq0c_)
        if(MFALSE == cqRingBuf(pBuf_ctrl)) {
            LOG_ERR("cqRingBuf(%d)  ",Ret);
            return MFALSE;
        }
#endif
//    }
    //
    Ret = ioctl(mFd,ISP_BUFFER_CTRL,pBuf_ctrl);
    if(Ret < 0)
    {
        if ((EAGAIN == errno) && (ISP_RT_BUF_CTRL_ENQUE == pbuf_ctrl->ctrl ))
        {
            Ret = ioctl(mFd,ISP_BUFFER_CTRL,pBuf_ctrl);
            if(Ret >= 0)
            {
                LOG_ERR("ISP_BUFFER_CTRL-retry(%d)[errno(%d):%s]  ",Ret, errno, strerror(errno));
                return MTRUE;
            }
        }
        LOG_ERR("ISP_BUFFER_CTRL(%d)[errno(%d):%s]  ",Ret, errno, strerror(errno));
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

    switch (ePipePath){
        case eIspDrvPipePath_P1:
            ref_cnt.id = ISP_REF_CNT_ID_P1_PIPE;
            break;
        case eIspDrvPipePath_P2:
            ref_cnt.id = ISP_REF_CNT_ID_P2_PIPE;
            break;
        case eIspDrvPipePath_Global:
            ref_cnt.id = ISP_REF_CNT_ID_GLOBAL_PIPE;
        default:
            break;
    };

    // Increase global pipe count.
    ref_cnt.ctrl = ISP_REF_CNT_INC;
    ref_cnt.data_ptr = &count;
    ret = ioctl(mFd, ISP_REF_CNT_CTRL, &ref_cnt);
    if (ret < 0)
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

    switch (ePipePath){
        case eIspDrvPipePath_P1:
            ref_cnt.id = ISP_REF_CNT_ID_P1_PIPE;
            ref_cnt.ctrl = ISP_REF_CNT_DEC_AND_RESET_P1_IF_LAST_ONE;
            break;
        case eIspDrvPipePath_P2:
            ref_cnt.id = ISP_REF_CNT_ID_P2_PIPE;
            ref_cnt.ctrl = ISP_REF_CNT_DEC_AND_RESET_P2_IF_LAST_ONE;
            break;
        case eIspDrvPipePath_Global:
            ref_cnt.id = ISP_REF_CNT_ID_GLOBAL_PIPE;
            ref_cnt.ctrl = ISP_REF_CNT_DEC_AND_RESET_P1_P2_IF_LAST_ONE;
        default:
            break;
    };


    ref_cnt.data_ptr = &count;
    ret = ioctl(mFd, ISP_REF_CNT_CTRL, &ref_cnt);
    if (ret < 0)
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

    //LOG_DBG("size of CQ_RTBC_RING_ST is (%d)  ",sizeof(CQ0B_RTBC_RING_ST));
    CQ0B_RTBC_RING_ST *pcqrtbcring_va  = (CQ0B_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0B];
    CQ0B_RTBC_RING_ST *pcqrtbcring_pa  = (CQ0B_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0B];

    LOG_DBG("[rtbc]va(0x%x),pa(0x%x)", pcqrtbcring_va, pcqrtbcring_pa);
    //
    pcqrtbcring_va->rtbc_ring.pNext = &pcqrtbcring_va->rtbc_ring;
    pcqrtbcring_va->rtbc_ring.next_pa = \
        (MUINT32)RTBC_GET_PA_FROM_VA(pcqrtbcring_va->rtbc_ring.pNext,pcqrtbcring_va,pcqrtbcring_pa);

    //vir isp drv
    int realCQIdx = this->getRealCQIndex(ISP_DRV_CQ0B,0,ISP_DRV_P1_DEFAULT_DUPCQIDX);
    IspDrv *_targetVirDrv = this->getCQInstance(ISP_DRV_CQ0B);

    //ob gain
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN0, (*pOB),ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN1, (*pOB),ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN2, (*pOB),ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(_targetVirDrv, CAM_OBC_GAIN3, (*pOB),ISP_DRV_USER_ISPF);

    //cmd
    pcqrtbcring_va->rtbc_ring.cq0b_rtbc.ob.inst = 0x00034510; //reg_4510
    pcqrtbcring_va->rtbc_ring.cq0b_rtbc.ob.data_ptr_pa = \
        (unsigned long)(&mpIspVirRegAddrPA[realCQIdx][0x00004510>>2]); // >>2 for MUINT32* pointer;

    //end
    pcqrtbcring_va->rtbc_ring.cq0b_rtbc.end.inst = 0xFC000000;
    pcqrtbcring_va->rtbc_ring.cq0b_rtbc.end.data_ptr_pa = 0;

    //CQ0B baseaddress
    LOG_DBG("CQ0B(0x%x)\n", this->getCQDescBufPhyAddr(ISP_DRV_CQ0B,0,ISP_DRV_P1_DEFAULT_DUPCQIDX));
    ISP_WRITE_REG(this,CAM_CTL_CQ0B_BASEADDR,(unsigned long)this->getCQDescBufPhyAddr(ISP_DRV_CQ0B,0,ISP_DRV_P1_DEFAULT_DUPCQIDX),ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(this, CAM_CTL_CQ_EN, CQ0B_EN, 1,ISP_DRV_USER_ISPF);

  #if 0
    {
        int *pdata = (int*)&pcqrtbcring_va->rtbc_ring;
        for (int i=0 ; i<(sizeof(CQ0B_RING_CMD_ST)>>2) ; i++ ) {
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

     //LOG_DBG("size of CQ_RTBC_RING_ST is (%d)  ",sizeof(CQ_RTBC_RING_ST));
     CQ_RTBC_RING_ST *pcqrtbcring_va  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C];
     CQ_RTBC_RING_ST *pcqrtbcring_pa  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0C];

     CQ_RTBC_RING_ST *pcqrtbcring_va_d  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C_D];
     CQ_RTBC_RING_ST *pcqrtbcring_pa_d  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0C_D];

     CQ_RTBC_RING_ST *pcqrtbcring_va_camsv  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C_CAMSV];
     CQ_RTBC_RING_ST *pcqrtbcring_pa_camsv  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0C_CAMSV];

     CQ_RTBC_RING_ST *pcqrtbcring_va_camsv2  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C_CAMSV2];
     CQ_RTBC_RING_ST *pcqrtbcring_pa_camsv2  = (CQ_RTBC_RING_ST*)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0C_CAMSV2];


     //isp_reg_t* pIspPhyReg = (isp_reg_t*)getMMapRegAddr();
     ISP_BUFFER_CTRL_STRUCT *pbuf_ctrl = (ISP_BUFFER_CTRL_STRUCT *)pBuf_ctrl;
     ISP_RT_BUF_INFO_STRUCT *pbuf_info = (ISP_RT_BUF_INFO_STRUCT *)pbuf_ctrl->data_ptr;
     ISP_RT_BUF_INFO_STRUCT *pex_buf_info = (ISP_RT_BUF_INFO_STRUCT *)pbuf_ctrl->ex_data_ptr;
     //
     MUINT32 i = 0;
     MUINT32 cam_tg_vf;
     MUINT32 size;

     cam_tg_vf  = ISP_READ_REG_NOPROTECT(this, CAM_TG_VF_CON);
     LOG_DBG("[rtbc] VF(0x%x), mod_id=%d",cam_tg_vf, pbuf_ctrl->buf_id);
     if ( _imgo_d_ == pbuf_ctrl->buf_id || _rrzo_d_ == pbuf_ctrl->buf_id ) {
         pcqrtbcring_va = pcqrtbcring_va_d;
         pcqrtbcring_pa = pcqrtbcring_pa_d;
         cam_tg_vf = ISP_READ_REG_NOPROTECT(this, CAM_TG2_VF_CON);
         LOG_DBG("[rtbc] VF2(0x%x),cQ0c_pa=0x%x",cam_tg_vf, pcqrtbcring_pa);
     }
     else if ( _camsv_imgo_ == pbuf_ctrl->buf_id ) {
         pcqrtbcring_va = pcqrtbcring_va_camsv;
         pcqrtbcring_pa = pcqrtbcring_pa_camsv;
         cam_tg_vf = ISP_READ_REG_NOPROTECT(this, CAMSV_TG_VF_CON);
         LOG_DBG("[rtbc] CAMSV VF(0x%x),cQ0c_camsv_pa=0x%x",cam_tg_vf, pcqrtbcring_pa);
     }
     else if ( _camsv2_imgo_ == pbuf_ctrl->buf_id ){
         pcqrtbcring_va = pcqrtbcring_va_camsv2;
         pcqrtbcring_pa = pcqrtbcring_pa_camsv2;
         cam_tg_vf = ISP_READ_REG_NOPROTECT(this, CAMSV_TG2_VF_CON);
         LOG_DBG("[rtbc] CAMSV VF(0x%x),cQ0c_camsv2_pa=0x%x",cam_tg_vf, pcqrtbcring_pa);
     }

    //VF_EN==0
    if ( 0 == (cam_tg_vf & 0x01) ) {
        //
        LOG_DBG("[rtbc]va(0x%x),pa(0x%x),ctrl(%d),dma(%d),PA(0x%x),o_size(%d),zo_size(%d)", \
                pcqrtbcring_va, \
                pcqrtbcring_pa, \
                pbuf_ctrl->ctrl, \
                pbuf_ctrl->buf_id, \
                pbuf_info->base_pAddr, \
                pcqrtbcring_va->imgo_ring_size,\
                pcqrtbcring_va->rrzo_ring_size);

        //
        if ( _imgo_ == pbuf_ctrl->buf_id || _imgo_d_ == pbuf_ctrl->buf_id || \
             _camsv_imgo_ == pbuf_ctrl->buf_id || _camsv2_imgo_ == pbuf_ctrl->buf_id ) {
            i = pcqrtbcring_va->imgo_ring_size;
        }
        else if (_rrzo_ == pbuf_ctrl->buf_id || _rrzo_d_ == pbuf_ctrl->buf_id) {
            i = pcqrtbcring_va->rrzo_ring_size;
        }
        else {
            LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
            return MFALSE;
        }
        //
        pcqrtbcring_va->rtbc_ring[i].pNext = &pcqrtbcring_va->rtbc_ring[(i>0)?0:i];
        pcqrtbcring_va->rtbc_ring[i].next_pa = \
            (unsigned long)RTBC_GET_PA_FROM_VA(pcqrtbcring_va->rtbc_ring[i].pNext,pcqrtbcring_va,pcqrtbcring_pa);


        //imgo/imgo_d
        if ( 0 == pcqrtbcring_va->imgo_ring_size ) {
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo.inst = ISP_DRV_CQ_DUMMY_WR_TOKEN; //0x00007300
        }
        //imgo/imgo_d
        if ( _imgo_ == pbuf_ctrl->buf_id ) {
           pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo.inst = 0x00007300; //ISP_DRV_CQ_DUMMY_WR_TOKEN; //reg_7300
           pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pbuf_info->base_pAddr; // ISP_CQ_DUMMY_PA; //
           pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_buf_idx = pbuf_info->bufIdx;
        }
        else if (_imgo_d_ == pbuf_ctrl->buf_id) {
           pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo.inst = 0x000074D4; //ISP_DRV_CQ_DUMMY_WR_TOKEN; //reg_74D4
           pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pbuf_info->base_pAddr; // ISP_CQ_DUMMY_PA; //
           pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_buf_idx = pbuf_info->bufIdx;
        }
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo.data_ptr_pa = \
            (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr,pcqrtbcring_va,pcqrtbcring_pa);

        //rrzo/rrzo_d
        if ( 0 == pcqrtbcring_va->rrzo_ring_size ) {
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo.inst = ISP_DRV_CQ_DUMMY_WR_TOKEN; //0x00007320; //reg_7320
        }
        //
        if (_rrzo_ == pbuf_ctrl->buf_id ) {
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo.inst = 0x00007320; //reg_7320
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr = pbuf_info->base_pAddr;
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_buf_idx = pbuf_info->bufIdx;
        }
        else if ( _rrzo_d_ == pbuf_ctrl->buf_id ) {
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo.inst = 0x000074F4; //reg_74F4
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr = pbuf_info->base_pAddr;
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_buf_idx = pbuf_info->bufIdx;
        }
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo.data_ptr_pa = \
            (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr,pcqrtbcring_va,pcqrtbcring_pa);
        //
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_cq0ci.inst = 0x000040B4; //reg_40B4
        if (( _imgo_d_ == pbuf_ctrl->buf_id ) || (_rrzo_d_ == pbuf_ctrl->buf_id )) {
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_cq0ci.inst = 0x000040CC; //reg_40CC
        }
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_cq0ci.data_ptr_pa = \
            (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].next_pa,pcqrtbcring_va,pcqrtbcring_pa);

        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.end.inst = 0xFC000000;
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.end.data_ptr_pa = 0;

        //
        if (i>0) {

            pcqrtbcring_va->rtbc_ring[i-1].pNext = &pcqrtbcring_va->rtbc_ring[i];
            pcqrtbcring_va->rtbc_ring[i-1].next_pa = \
                (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i],pcqrtbcring_va,pcqrtbcring_pa);
            //
            pcqrtbcring_va->rtbc_ring[i-1].cq_rtbc.next_cq0ci.data_ptr_pa = \
                (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i-1].next_pa,pcqrtbcring_va,pcqrtbcring_pa);
        }
        //
        if ( _imgo_ == pbuf_ctrl->buf_id || _imgo_d_ == pbuf_ctrl->buf_id) {
            pcqrtbcring_va->imgo_ring_size++;
            size = pcqrtbcring_va->imgo_ring_size;
            //
            if (_imgo_ == pbuf_ctrl->buf_id) {
                ISP_WRITE_BITS(this,CAM_CTL_IMGO_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this,CAM_CTL_IMGO_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
            }
            else {
                ISP_WRITE_BITS(this,CAM_CTL_IMGO_D_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this,CAM_CTL_IMGO_D_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
            }

        }
        else if ( _rrzo_ == pbuf_ctrl->buf_id || _rrzo_d_ == pbuf_ctrl->buf_id) {
            pcqrtbcring_va->rrzo_ring_size++;
            size = pcqrtbcring_va->rrzo_ring_size;
            //
            if (_rrzo_ == pbuf_ctrl->buf_id) {
                ISP_WRITE_BITS(this,CAM_CTL_RRZO_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this,CAM_CTL_RRZO_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
            }
            else {
                ISP_WRITE_BITS(this,CAM_CTL_RRZO_D_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(this,CAM_CTL_RRZO_D_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
            }
        }
        else if ( _camsv_imgo_ == pbuf_ctrl->buf_id ) {
            pcqrtbcring_va->imgo_ring_size++;
            size = pcqrtbcring_va->imgo_ring_size;

            ISP_WRITE_BITS(this,CAMSV_CAMSV_IMGO_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(this,CAMSV_CAMSV_IMGO_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
        }
        else if ( _camsv2_imgo_ == pbuf_ctrl->buf_id ) {
            pcqrtbcring_va->imgo_ring_size++;
            size = pcqrtbcring_va->imgo_ring_size;

            ISP_WRITE_BITS(this,CAMSV_CAMSV2_IMGO_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(this,CAMSV_CAMSV2_IMGO_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
        }
        //
        if ( _imgo_ == pbuf_ctrl->buf_id || _rrzo_ == pbuf_ctrl->buf_id) {
            //LOG_INF("[rtbc] Update CAM_CTL_CQ0C_BASEADDR");
            ISP_WRITE_REG(this,CAM_CTL_CQ0C_BASEADDR,((unsigned long)this->getCQDescBufPhyAddr(ISP_DRV_CQ0C,0,ISP_DRV_P1_DEFAULT_DUPCQIDX)) + ((1==size)?0:sizeof(CQ_RING_CMD_ST)),ISP_DRV_USER_ISPF);
        }
        else if ( _imgo_d_ == pbuf_ctrl->buf_id || _rrzo_d_ == pbuf_ctrl->buf_id) {
            //LOG_INF("[rtbc] Update CAM_CTL_CQ0C_D_BASEADDR");
            ISP_WRITE_REG(this,CAM_CTL_CQ0C_D_BASEADDR,((unsigned long)this->getCQDescBufPhyAddr(ISP_DRV_CQ0C_D,0,ISP_DRV_P1_DEFAULT_DUPCQIDX)) + ((1==size)?0:sizeof(CQ_RING_CMD_ST)),ISP_DRV_USER_ISPF);
        }
        //
        LOG_DBG("[rtbc]imgo_sz(%d),rrzo_sz(%d), BufIdx=0x%x",pcqrtbcring_va->imgo_ring_size,pcqrtbcring_va->rrzo_ring_size,pbuf_info->bufIdx);
        //
        {
            int *pdata = (int*)&pcqrtbcring_va->rtbc_ring[0];
            for ( i=0 ; i<(sizeof(CQ_RING_CMD_ST)>>2)*size ; i++ ) {
                LOG_DBG("[rtbc][des] addr:(0x%08x) val:(0x%08x)",pdata+i,pdata[i]);
            }
        }
    }
    //VF_EN==1
    else {
        if ( pex_buf_info ) {
            MUINT32 findTarget = 0;

            //
            i = 0;
            if ( _imgo_ == pbuf_ctrl->buf_id || _imgo_d_ == pbuf_ctrl->buf_id) {
                //
                if (pex_buf_info->bufIdx != 0xFFFF) {
                    //LOG_INF("[rtbc]ByIdx: IMGO");
                    //replace the specific buffer with same bufIdx
                    for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size ; i++ ) {
                        if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_buf_idx == pbuf_info->bufIdx) {
                            //LOG_INF("[rtbc]ByIdx: idx(%d) old/new imgo buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr,pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                } else {
                    //replace the specific buffer with same buf Address
                    //LOG_INF("[rtbc]ByAddr: IMGO");
                    for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size ; i++ ) {
                        if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr == pbuf_info->base_pAddr ) {
                            //LOG_INF("[rtbc]ByAddr: idx(%d) old/new imgo buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr,pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                }
                if (!findTarget) {
                    LOG_ERR("[rtbc]error exIMGO-Fail. BufIdx(0x%x/0x%x) new imgo buffer(0x%x)",\
                          pbuf_info->bufIdx, pex_buf_info->bufIdx,pex_buf_info->base_pAddr);
                    for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size ; i++ ) {
                        LOG_DBG("[rtbc][%d] old/new BufIdx(%d/%d) ",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_buf_idx, pbuf_info->bufIdx);
                   }
                }
            }
            else if (_rrzo_ == pbuf_ctrl->buf_id || _rrzo_d_ == pbuf_ctrl->buf_id) {
                //
                if (pex_buf_info->bufIdx != 0xFFFF) {
                    //replace the specific buffer with same bufIdx
                    //LOG_INF("[rtbc]ByIdx: RRZO");
                    for ( i=0 ; i<pcqrtbcring_va->rrzo_ring_size ; i++ ) {
                        //LOG_INF("[rtbc]ByIdx:[%d] old/new BufIdx(%d/%d) ",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_buf_idx, pbuf_info->bufIdx);
                        if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_buf_idx == pbuf_info->bufIdx ) {
                            //LOG_INF("[rtbc]ByIdx: idx(%d) old/new rrzo buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr, pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                } else {
                    //replace the specific buffer with same buf Address
                    //LOG_INF("[rtbc]ByAddr: RRZO");
                    for ( i=0 ; i<pcqrtbcring_va->rrzo_ring_size ; i++ ) {
                        if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr == pbuf_info->base_pAddr ) {
                            //LOG_INF("[rtbc]ByAddr: idx(%d) old/new rrzo buffer(0x%x/0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr, pex_buf_info->base_pAddr);
                            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr = pex_buf_info->base_pAddr;
                            findTarget = 1;
                            break;
                        }
                    }
                }
                if (!findTarget) {
                   LOG_ERR("[rtbc]error exRRZO-Fail. Bufidx(0x%x/0x%x) old/new imgo buffer(0x%x/0x%x)",\
                       pbuf_info->bufIdx,pex_buf_info->bufIdx,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_base_pAddr,pex_buf_info->base_pAddr);
                   for ( i=0 ; i<pcqrtbcring_va->rrzo_ring_size ; i++ ) {
                     LOG_DBG("[rtbc]ByIdx:[%d] old/new BufIdx(%d/%d) ",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.rrzo_buf_idx, pbuf_info->bufIdx);
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
    ISP_BUFFER_CTRL_STRUCT *pbuf_ctrl = (ISP_BUFFER_CTRL_STRUCT *)pBuf_ctrl;


    /*LOG_INF("[rtbc]pcqrtbcring_va/pa(0x%x/0x%x),ctrl(%d),dma(%d),pa(0x%x),imgor_size(%d),rrzor_size(%d)", \
            pcqrtbcring_va, \
            pcqrtbcring_pa, \
            pbuf_ctrl->ctrl, \
            pbuf_ctrl->buf_id, \
            pbuf_info->base_pAddr, \
            pcqrtbcring_va->imgo_ring_size,\
            pcqrtbcring_va->rrzo_ring_size);*/
    //
    //LOG_INF("[rtbc](%d)",sizeof(CQ_RING_CMD_ST));

    //
    switch( pbuf_ctrl->ctrl ) {
        //
        case ISP_RT_BUF_CTRL_ENQUE_IMD:
        case ISP_RT_BUF_CTRL_ENQUE:
            Ret = _updateEnqCqRingBuf(pBuf_ctrl);
            //
            break;
#if 0
        case ISP_RT_BUF_CTRL_EXCHANGE_ENQUE:
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
        case ISP_RT_BUF_CTRL_CLEAR:
            //reset cq0c all 0 for rtbc
            if (sizeof(CQ_RTBC_RING_ST) <= sizeof(mIspCQDescInit)) {
                if ((_imgo_ == pbuf_ctrl->buf_id)||(_rrzo_ == pbuf_ctrl->buf_id)) {
                    LOG_DBG("[PIP]Clr P1 CQ0C");
                    memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C],0,sizeof(CQ_RTBC_RING_ST));
                }

                if ((_imgo_d_ == pbuf_ctrl->buf_id)||(_rrzo_d_ == pbuf_ctrl->buf_id)) {
                    LOG_DBG("[PIP]Clr P1D CQ0CD");
                    memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C_D],0,sizeof(CQ_RTBC_RING_ST));
                }
            }
            else {
                LOG_ERR("rtbc data exceed buffer size(%d)>(%d)",sizeof(CQ_RTBC_RING_ST),sizeof(mIspCQDescInit));
            }

            break;
        default:
            //LOG_ERR("ERROR:ctrl id(%d)",pbuf_ctrl->ctrl);
            break;
    }

    //reset cq0c all 0 for rtbc
    //memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C],0,sizeof(CQ_RTBC_RING_ST));

    return MTRUE;
}
#endif


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
    if((nextQueueIdx==mTuningQueIdx[p2Cq].curReadP1Idx)||(nextQueueIdx==mTuningQueIdx[p2Cq].curReadP2Idx)){
        LOG_ERR("[Error]multi-access the same index==>nextQueueIdx(%d),curReadP1Idx(%d),curReadP2Idx(%d)",
            nextQueueIdx,mTuningQueIdx[p2Cq].curReadP1Idx,mTuningQueIdx[p2Cq].curReadP2Idx);
    }
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
    MUINT32 camCtlEnP1, camCtlEnP1D, camCtlEnP2;
    MUINT32 camCtlEnP1Dma, camCtlEnP1DDma, camCtlEnP2Dma;
    MUINT32 addrOffset, moduleSize;
    MUINT32 i, j, k;
    MINT32 checkNum; //for tuning bypass issue in initial stage
    //
    mapCqToP2Cq(eCq, p2Cq);
    LOG_DBG("+,path(%d),eCq(%d),p2Cq(%d),magic#(0x%x),curRP2Idx(%d),curWIdx(%d)", \
        path,eCq,p2Cq,magicNum,mTuningQueIdx[p2Cq].curReadP2Idx,mTuningQueIdx[p2Cq].curWriteIdx);
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

    } else if(path == eTuningQueAccessPath_imageio_P2) {

        #if 1
        // [1]backward search the tuning que with magic#
        if(searchTuningQue(p2Cq, magicNum, tarQueueIdx) != MTRUE){ // get currQueueIdx
            LOG_ERR("[Error]P2 tarQueueIdx search error");
            ret = MFALSE;
            goto EXIT;
        }

        #else
        if(mTuningQueIdx[p2Cq].isInitP2 == MFALSE){
            checkNum = mTuningQueIdx[p2Cq].curWriteIdx;
            if(checkNum <= 0) {
                LOG_ERR("[Error]P2 TuningMgr not ready yet! checkNum(%d)",checkNum);
                ret = MFALSE;
                goto EXIT;
            }
        } else {
            checkNum = 1;
        }
        LOG_INF("checkNum(%d),curWriteIdx(%d)",checkNum,mTuningQueIdx[p2Cq].curWriteIdx);
        #endif

        do {
            //
            // [2]check and set the real pCurReadP2TuningQue
            if (magicNum & ISP_MAGIC_NUM_CAPTURE_SIGN){ // capture mode
                currQueueIdx = tarQueueIdx;
                mTuningQueIdx[p2Cq].curReadP2Idx = tarQueueIdx;
                mTuningQueIdx[p2Cq].pCurReadP2TuningQue = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue;
                mTuningQueIdx[p2Cq].capReadP2CtlEnP1 = 0;
                mTuningQueIdx[p2Cq].capReadP2CtlEnP1Dma = 0;
                mTuningQueIdx[p2Cq].capReadP2CtlEnP2 = 0;
                mTuningQueIdx[p2Cq].capReadP2CtlEnP2Dma = 0;
            } else { // prview mode
                //LOG_INF("curPreviewApplyP2Idx(%d)-tarQueueIdx(%d)",mTuningQueIdx[p2Cq].curPreviewApplyP2Idx,tarQueueIdx);
                if(mTuningQueIdx[p2Cq].curPreviewApplyP2Idx == tarQueueIdx){
                    currQueueIdx = tarQueueIdx;
                } else {
                    currQueueIdx = GET_NEXT_TUNING_QUEUE_IDX(mTuningQueIdx[p2Cq].curPreviewApplyP2Idx);
                    while(mTuningQueInf[p2Cq][currQueueIdx].magicNum&ISP_MAGIC_NUM_CAPTURE_SIGN){ // skip capture index
                        currQueueIdx = GET_NEXT_TUNING_QUEUE_IDX(currQueueIdx);
                    };
                }
                //LOG_INF("currQueueIdx(%d)-curPreviewApplyP2Idx(%d),tarQueueIdx(%d)",currQueueIdx,mTuningQueIdx[p2Cq].curPreviewApplyP2Idx,tarQueueIdx);

                mTuningQueIdx[p2Cq].curReadP2Idx = currQueueIdx;
                mTuningQueIdx[p2Cq].curPreviewApplyP2Idx = currQueueIdx;
                mTuningQueIdx[p2Cq].pCurReadP2TuningQue = mTuningQueIdx[p2Cq].keepP2PreviewQue.pTuningQue;
            }
            LOG_DBG("tarQueueIdx(%d),currQueueIdx(%d),curPreviewApplyP2Idx(%d),curReadP2Idx(%d),curWriteIdx(%d)", \
                tarQueueIdx,currQueueIdx,mTuningQueIdx[p2Cq].curPreviewApplyP2Idx,mTuningQueIdx[p2Cq].curReadP2Idx,mTuningQueIdx[p2Cq].curWriteIdx);

            //
            // [3]update eCurReadP2UpdateFuncBit & keepP2PreviewUpdateFuncBit
            mTuningQueIdx[p2Cq].eCurReadP2UpdateFuncBit = mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit;
            mTuningQueIdx[p2Cq].keepP2PreviewUpdateFuncBit = (EIspTuningMgrFunc)
                    ((MINT32)mTuningQueIdx[p2Cq].keepP2PreviewUpdateFuncBit | (MINT32)mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit);
            //
            // [4]update the current queue info.
            pIspRegMap = (isp_reg_t*)mTuningQueInf[p2Cq][currQueueIdx].pTuningQue;
            if (NULL == pIspRegMap){
                LOG_ERR("[Error]pIspRegMap NULL");
                ret = MFALSE;
                goto EXIT;
            }
            camCtlEnP1 = pIspRegMap->CAM_CTL_EN_P1.Raw;
            camCtlEnP2 = pIspRegMap->CAM_CTL_EN_P2.Raw;
            camCtlEnP1Dma = pIspRegMap->CAM_CTL_EN_P1_DMA.Raw;
            camCtlEnP2Dma = pIspRegMap->CAM_CTL_EN_P2_DMA.Raw;
            LOG_DBG("camCtlEnP1(0x%x),camCtlEnP2(0x%x),pCurReadP2TuningQue(0x%08x),keepP2PreviewQue(0x%08x),eUpdateFuncBit(0x%08x)", \
                    camCtlEnP1,camCtlEnP2,mTuningQueIdx[p2Cq].pCurReadP2TuningQue,mTuningQueIdx[p2Cq].keepP2PreviewQue.pTuningQue,mTuningQueIdx[p2Cq].eCurReadP2UpdateFuncBit);
            //
            for(i=0;i<eIspTuningMgrFuncBit_Num;i++){
                if((1<<gIspTuningFuncBitMapp[i].eTuningFuncBit) & mTuningQueIdx[p2Cq].eCurReadP2UpdateFuncBit){
                    switch(gIspTuningFuncBitMapp[i].eTuningCtlByte){
                        case eTuningCtlByte_P1:
                            // update keepReadP2CtlEnP1 & keepP2PreviewQue
                            if(gIspTuningFuncBitMapp[i].camCtlEnP1Bit != -1){
                                // update keepP2PreviewQue
                                if ((magicNum & ISP_MAGIC_NUM_CAPTURE_SIGN)==0x00){ // preview mode
                                    mTuningQueIdx[p2Cq].keepReadP2CtlEnP1 =
                                        ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP1Bit))&mTuningQueIdx[p2Cq].keepReadP2CtlEnP1)|
                                        (camCtlEnP1&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1Bit));

                                    if(gIspTuningFuncBitMapp[i].eTuningCqFunc1 != CAM_DUMMY_){
                                        getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc1,addrOffset,moduleSize);
                                        LOG_DBG("[1]P2_P1:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
                                        for(j=0;j<moduleSize;j++){
                                            mTuningQueIdx[p2Cq].keepP2PreviewQue.pTuningQue[addrOffset>>2] = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue[addrOffset>>2];
                                            addrOffset += 4;
                                        }
                                    }
                                    if(gIspTuningFuncBitMapp[i].eTuningCqFunc2 != CAM_DUMMY_){
                                        getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc2,addrOffset,moduleSize);
                                        LOG_DBG("[2]P2_P1:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
                                        for(j=0;j<moduleSize;j++){
                                            mTuningQueIdx[p2Cq].keepP2PreviewQue.pTuningQue[addrOffset>>2] = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue[addrOffset>>2];
                                            addrOffset += 4;
                                        }
                                    }
                                    //
                                    // update keepReadP2CtlEnP1Dma
                                    if(gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit != -1){
                                        mTuningQueIdx[p2Cq].keepReadP2CtlEnP1Dma =
                                            ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit))&mTuningQueIdx[p2Cq].keepReadP2CtlEnP1Dma)|
                                            (camCtlEnP1Dma&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit));
                                    }
                                } else { // capture mode
                                    mTuningQueIdx[p2Cq].capReadP2CtlEnP1 |= (camCtlEnP1&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1Bit));

                                    if(gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit != -1){// update capReadP2CtlEnP1Dma
                                        mTuningQueIdx[p2Cq].capReadP2CtlEnP1Dma =
                                            ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit))&mTuningQueIdx[p2Cq].capReadP2CtlEnP1Dma)|
                                            (camCtlEnP1Dma&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit));
                                    }
                                }
                            } else {
                                LOG_ERR("[Error]gIspTuningFuncBitMapp table error i(%d) = -1",i);
                                ret = MFALSE;
                                goto EXIT;
                            }
                            break;
                        case eTuningCtlByte_P2:
                            // update keepReadP2CtlEnP2 & keepP2PreviewQue
                            if(gIspTuningFuncBitMapp[i].camCtlEnP2Bit != -1){
                                if ((magicNum & ISP_MAGIC_NUM_CAPTURE_SIGN)==0x00){ // preview mode
                                    mTuningQueIdx[p2Cq].keepReadP2CtlEnP2 =
                                        ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP2Bit))&mTuningQueIdx[p2Cq].keepReadP2CtlEnP2)|
                                        (camCtlEnP2&(1<<gIspTuningFuncBitMapp[i].camCtlEnP2Bit));

                                    if(gIspTuningFuncBitMapp[i].eTuningCqFunc1 != CAM_DUMMY_){
                                        getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc1,addrOffset,moduleSize);
                                        LOG_DBG("[1]P2:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
                                        for(j=0;j<moduleSize;j++){
                                            mTuningQueIdx[p2Cq].keepP2PreviewQue.pTuningQue[addrOffset>>2] = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue[addrOffset>>2];
                                            addrOffset += 4;
                                        }
                                    }
                                    if(gIspTuningFuncBitMapp[i].eTuningCqFunc2 != CAM_DUMMY_){
                                        getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc2,addrOffset,moduleSize);
                                        LOG_DBG("[2]P2:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
                                        for(j=0;j<moduleSize;j++){
                                            mTuningQueIdx[p2Cq].keepP2PreviewQue.pTuningQue[addrOffset>>2] = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue[addrOffset>>2];
                                            addrOffset += 4;
                                        }
                                    }
                                    //
                                    // update keepReadP2CtlEnP2Dma
                                    if(gIspTuningFuncBitMapp[i].camCtlEnP2DmaBit != -1){
                                        mTuningQueIdx[p2Cq].keepReadP2CtlEnP2Dma =
                                            ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP2DmaBit))&mTuningQueIdx[p2Cq].keepReadP2CtlEnP2Dma)|
                                            (camCtlEnP2Dma&(1<<gIspTuningFuncBitMapp[i].camCtlEnP2DmaBit));
                                    }
                                } else { // capture mode
                                    mTuningQueIdx[p2Cq].capReadP2CtlEnP2 |= (camCtlEnP2&(1<<gIspTuningFuncBitMapp[i].camCtlEnP2Bit));

                                    if(gIspTuningFuncBitMapp[i].camCtlEnP2DmaBit != -1){// update capReadP2CtlEnP2Dma
                                        mTuningQueIdx[p2Cq].capReadP2CtlEnP2Dma =
                                            ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP2DmaBit))&mTuningQueIdx[p2Cq].capReadP2CtlEnP2Dma)|
                                            (camCtlEnP2Dma&(1<<gIspTuningFuncBitMapp[i].camCtlEnP2DmaBit));
                                    }
                                }

                            } else {
                                LOG_ERR("[Error]gIspTuningFuncBitMapp table error i(%d) = -1",i);
                                ret = MFALSE;
                                goto EXIT;
                            }
                            break;
                        case eTuningCtlByte_P1D:
                        default:
                            /* do nothing */
                            break;
                    };
                    #if 0
                    //update tpipe data
                    if((gIspTuningFuncBitMapp[i].eTuningCtlByte==eTuningCtlByte_P2)||(gIspTuningFuncBitMapp[i].eTuningCtlByte==eTuningCtlByte_P1)){
                        isp_reg_t *pReadP2TuningQue = (isp_reg_t*)mTuningQueInf[p2Cq][currQueueIdx].pTuningQue;

                        LOG_DBG("i(%d)",i);

                        switch(i){
                            case eIspTuningMgrFuncBit_Lsc:
                                mTuningQueIdx[p2Cq].keepTpipeField.lsc.sdblk_width = pReadP2TuningQue->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
                                mTuningQueIdx[p2Cq].keepTpipeField.lsc.sdblk_xnum = pReadP2TuningQue->CAM_LSC_CTL2.Bits.LSC_SDBLK_XNUM;
                                mTuningQueIdx[p2Cq].keepTpipeField.lsc.sdblk_last_width = pReadP2TuningQue->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH;
                                mTuningQueIdx[p2Cq].keepTpipeField.lsc.sdblk_height = pReadP2TuningQue->CAM_LSC_CTL3.Bits.LSC_SDBLK_HEIGHT;
                                mTuningQueIdx[p2Cq].keepTpipeField.lsc.sdblk_ynum = pReadP2TuningQue->CAM_LSC_CTL3.Bits.LSC_SDBLK_YNUM;
                                mTuningQueIdx[p2Cq].keepTpipeField.lsc.sdblk_last_height = pReadP2TuningQue->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lHEIGHT;
                                mTuningQueIdx[p2Cq].keepTpipeField.lsci.lsci_stride = pReadP2TuningQue->CAM_LSCI_STRIDE.Bits.STRIDE;
                                break;
                            case eIspTuningMgrFuncBit_Bnr:
                                mTuningQueIdx[p2Cq].keepTpipeField.bnr.bpc_en = pReadP2TuningQue->CAM_BPC_CON.Bits.BPC_EN;
                                mTuningQueIdx[p2Cq].keepTpipeField.bnr.bpc_tbl_en = pReadP2TuningQue->CAM_BPC_D_CON.Bits.BPC_LUT_EN;
                                break;
                            case eIspTuningMgrFuncBit_Sl2:
                                mTuningQueIdx[p2Cq].keepTpipeField.sl2.sl2_hrz_comp = pReadP2TuningQue->CAM_SL2_HRZ.Bits.SL2_HRZ_COMP;
                                break;
                            case eIspTuningMgrFuncBit_Cfa:
                                mTuningQueIdx[p2Cq].keepTpipeField.cfa.bayer_bypass = pReadP2TuningQue->CAM_DM_O_BYP.Bits.DM_BYP;
//set from path imageio         mTuningQueIdx[p2Cq].keepTpipeField.cfa.dm_fg_mode = pReadP2TuningQue->CAM_DM_O_BYP.Bits.DM_FG_MODE;
                                break;
                            case eIspTuningMgrFuncBit_Mfb_TuneCon:
//set from path imageio       //mTuningQueIdx[p2Cq].keepTpipeField.mfb.bld_mode = pReadP2TuningQue->CAM_MFB_CON.Bits.BLD_MODE;
                                mTuningQueIdx[p2Cq].keepTpipeField.mfb.bld_deblock_en = pReadP2TuningQue->CAM_MFB_LL_CON3.Bits.BLD_LL_DB_EN;
                                mTuningQueIdx[p2Cq].keepTpipeField.mfb.bld_brz_en = pReadP2TuningQue->CAM_MFB_LL_CON3.Bits.BLD_LL_BRZ_EN;
                                break;
                            case eIspTuningMgrFuncBit_G2c_Shade:
                                mTuningQueIdx[p2Cq].keepTpipeField.g2c.g2c_shade_en = pReadP2TuningQue->CAM_G2C_SHADE_CON_1.Bits.G2C_SHADE_EN;
                                break;
                            case eIspTuningMgrFuncBit_Nbc:
                                mTuningQueIdx[p2Cq].keepTpipeField.nbc.anr_eny = pReadP2TuningQue->CAM_ANR_CON1.Bits.ANR_ENY;
                                mTuningQueIdx[p2Cq].keepTpipeField.nbc.anr_enc = pReadP2TuningQue->CAM_ANR_CON1.Bits.ANR_ENC;
                                mTuningQueIdx[p2Cq].keepTpipeField.nbc.anr_iir_mode = pReadP2TuningQue->CAM_ANR_CON1.Bits.ANR_FLT_MODE;
                                mTuningQueIdx[p2Cq].keepTpipeField.nbc.anr_scale_mode = pReadP2TuningQue->CAM_ANR_CON1.Bits.ANR_SCALE_MODE;
                                break;
                            case eIspTuningMgrFuncBit_Seee:
                                mTuningQueIdx[p2Cq].keepTpipeField.seee.se_edge = pReadP2TuningQue->CAM_SEEE_OUT_EDGE_CTRL.Bits.SEEE_OUT_EDGE_SEL;
                                mTuningQueIdx[p2Cq].keepTpipeField.seee.usm_over_shrink_en = pReadP2TuningQue->CAM_SEEE_CLIP_CTRL.Bits.SEEE_OVRSH_CLIP_EN;
                                break;
                            case eIspTuningMgrFuncBit_Lce:
                                mTuningQueIdx[p2Cq].keepTpipeField.lce.lce_bc_mag_kubnx = pReadP2TuningQue->CAM_LCE_ZR.Bits.LCE_BCMK_X;
                                mTuningQueIdx[p2Cq].keepTpipeField.lce.lce_slm_width = pReadP2TuningQue->CAM_LCE_SLM_SIZE.Bits.LCE_SLM_WD;
                                mTuningQueIdx[p2Cq].keepTpipeField.lce.lce_bc_mag_kubny = pReadP2TuningQue->CAM_LCE_ZR.Bits.LCE_BCMK_Y;
                                mTuningQueIdx[p2Cq].keepTpipeField.lce.lce_slm_height = pReadP2TuningQue->CAM_LCE_SLM_SIZE.Bits.LCE_SLM_HT;
                                mTuningQueIdx[p2Cq].keepTpipeField.lcei.lcei_stride = pReadP2TuningQue->CAM_LCEI_STRIDE.Bits.STRIDE;
                                break;
                            case eIspTuningMgrFuncBit_Nr3d_Blending:
                            case eIspTuningMgrFuncBit_Nr3d_Lmt:
//set by imageio path                            mTuningQueIdx[p2Cq].keepTpipeField.nr3d.nr3d_on_en = pReadP2TuningQue->CAM_NR3D_CTRL.Bits.NR3D_ON_EN;
//set by imageio path                            mTuningQueIdx[p2Cq].keepTpipeField.nr3d.nr3d_on_xoffset = pReadP2TuningQue->CAM_NR3D_ON_OFF.Bits.NR3D_ON_OFST_X;
//set by imageio path                            mTuningQueIdx[p2Cq].keepTpipeField.nr3d.nr3d_on_yoffset = pReadP2TuningQue->CAM_NR3D_ON_OFF.Bits.NR3D_ON_OFST_Y;
//set by imageio path                            mTuningQueIdx[p2Cq].keepTpipeField.nr3d.nr3d_on_width = pReadP2TuningQue->CAM_NR3D_ON_SIZ.Bits.NR3D_ON_WD;
//set by imageio path                            mTuningQueIdx[p2Cq].keepTpipeField.nr3d.nr3d_on_height = pReadP2TuningQue->CAM_NR3D_ON_SIZ.Bits.NR3D_ON_HT;
                                break;
                            case eIspTuningMgrFuncBit_Sl2b:
                                mTuningQueIdx[p2Cq].keepTpipeField.sl2b.sl2b_hrz_comp = pReadP2TuningQue->CAM_SL2B_HRZ.Bits.SL2B_HRZ_COMP;
                                break;
                            case eIspTuningMgrFuncBit_Sl2c:
                                mTuningQueIdx[p2Cq].keepTpipeField.sl2c.sl2c_hrz_comp = pReadP2TuningQue->CAM_SL2C_HRZ.Bits.SL2C_HRZ_COMP;
                                break;
                            case eIspTuningMgrFuncBit_Mix3:
                            default:
                                /* do nothing */
                                break;
                        };
                    }
                    #endif
                }
            }
        } while((tarQueueIdx!=mTuningQueIdx[p2Cq].curPreviewApplyP2Idx)&&((magicNum&ISP_MAGIC_NUM_CAPTURE_SIGN)==0x00));

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
        for (k=0;k<checkNum;k++){
            if (mTuningQueIdx[p2Cq].isInitP1==MTRUE){
                // [1]backward search the tuning que with magic#
                if(searchTuningQue(p2Cq, magicNum, currQueueIdx) != MTRUE){
                    LOG_ERR("[Error]curReadP1Idx search error");
                    ret = MFALSE;
                    goto EXIT;
                }
                mTuningQueIdx[p2Cq].curReadP1Idx = currQueueIdx;
            } else {
                LOG_INF("k(%d)",k);
                // [1] search the tuning que by order
#if 1
                if((checkNum == k+1)){ // get currQueueIdx
                    mTuningQueIdx[p2Cq].isInitP1 = MTRUE;
                    LOG_INF("[P1]search ending, and Exit, k(%d)",k);
                    if(k != 0)
                        break;
                }
                currQueueIdx = k;
#else
                if(orderSearchTuningQue(p2Cq, magicNum, k, (mTuningQueIdx[p2Cq].curWriteIdx-1), currQueueIdx) != MTRUE){ // get currQueueIdx
                    mTuningQueIdx[p2Cq].isInitP1 = MTRUE;
                    LOG_INF("search ending, and Exit");
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
            camCtlEnP1 = pIspRegMap->CAM_CTL_EN_P1.Raw;
            camCtlEnP1D = pIspRegMap->CAM_CTL_EN_P1_D.Raw;
            camCtlEnP1Dma = pIspRegMap->CAM_CTL_EN_P1_DMA.Raw;
            camCtlEnP1DDma = pIspRegMap->CAM_CTL_EN_P1_DMA_D.Raw;
            LOG_DBG("camCtlEnP1(0x%x),camCtlEnP1D(0x%x),keepP1UpdateFuncBit(0x%x),eUpdateFuncBit(0x%x)",camCtlEnP1,camCtlEnP1D,mTuningQueIdx[p2Cq].keepP1UpdateFuncBit,mTuningQueInf[p2Cq][currQueueIdx].eUpdateFuncBit);
            //
            for(i=0;i<eIspTuningMgrFuncBit_Num;i++){
                if((1<<gIspTuningFuncBitMapp[i].eTuningFuncBit) & mTuningQueIdx[p2Cq].eCurReadP1UpdateFuncBit){
                    switch(gIspTuningFuncBitMapp[i].eTuningCtlByte){
                        case eTuningCtlByte_P1:
                            if(gIspTuningFuncBitMapp[i].camCtlEnP1Bit != -1){
                                // update for keepReadP1CtlEnP1
                                mTuningQueIdx[p2Cq].keepReadP1CtlEnP1 =
                                    ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP1Bit))&mTuningQueIdx[p2Cq].keepReadP1CtlEnP1)|
                                    (camCtlEnP1&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1Bit));

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
                            // update for keepReadP1CtlEnP1Dma
                            if(gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit != -1){
                                mTuningQueIdx[p2Cq].keepReadP1CtlEnP1Dma =
                                    ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit))&mTuningQueIdx[p2Cq].keepReadP1CtlEnP1Dma)|
                                    (camCtlEnP1Dma&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DmaBit));
                            }
                            break;
                        case eTuningCtlByte_P1D:

                            if(gIspTuningFuncBitMapp[i].camCtlEnP1DBit != -1){
                                // update for keepReadP1DCtlEnP1D
                                mTuningQueIdx[p2Cq].keepReadP1CtlEnP1D =
                                    ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DBit))&mTuningQueIdx[p2Cq].keepReadP1CtlEnP1D)|
                                    (camCtlEnP1D&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DBit));

                                // update keepP1Que
                                if(gIspTuningFuncBitMapp[i].eTuningCqFunc1 != CAM_DUMMY_){
                                    getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc1,addrOffset,moduleSize);
                                    LOG_DBG("[1]P1D:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
                                    for(j=0;j<moduleSize;j++){
                                        mTuningQueIdx[p2Cq].keepP1Que.pTuningQue[addrOffset>>2] = mTuningQueInf[p2Cq][currQueueIdx].pTuningQue[addrOffset>>2];
                                        addrOffset += 4;
                                    }
                                }
                                if(gIspTuningFuncBitMapp[i].eTuningCqFunc2 != CAM_DUMMY_){
                                    getCqModuleInf(gIspTuningFuncBitMapp[i].eTuningCqFunc2,addrOffset,moduleSize);
                                    LOG_DBG("[2]P1D:addrOffset(0x%08x),moduleSize(%d)",addrOffset,moduleSize);
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
                            // update for keepReadP1DCtlEnP1DDma
                            if(gIspTuningFuncBitMapp[i].camCtlEnP1DDmaBit != -1){
                                mTuningQueIdx[p2Cq].keepReadP1CtlEnP1DDma =
                                    ((~(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DDmaBit))&mTuningQueIdx[p2Cq].keepReadP1CtlEnP1DDma)|
                                    (camCtlEnP1DDma&(1<<gIspTuningFuncBitMapp[i].camCtlEnP1DDmaBit));
                            }
                            break;

                        case eTuningCtlByte_P2:
                        default:
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
        if((nextQueueIdx==mTuningQueIdx[p2Cq].curReadP1Idx)||(nextQueueIdx==mTuningQueIdx[p2Cq].curReadP2Idx)){
            LOG_ERR("[Error]multi-access the same index==>nextQueueIdx(%d),curReadP1Idx(%d),curReadP2Idx(%d)",
                nextQueueIdx,mTuningQueIdx[p2Cq].curReadP1Idx,mTuningQueIdx[p2Cq].curReadP2Idx);
            ret = MFALSE;
            goto EXIT;
        }
        mTuningQueIdx[p2Cq].curWriteIdx = nextQueueIdx;
    } else if(path == eTuningQueAccessPath_imageio_P2) {
        //release p2 tuning queue
        mTuningQueIdx[p2Cq].pCurReadP2TuningQue = NULL;
        mTuningQueIdx[p2Cq].curReadP2Idx = -1;
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
    MINT32 magicNum,
    MINT32 cq,
    MUINT32 drvScenario)

{
    Mutex::Autolock lock(tuningQueueIdxLock);
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MUINT32 mApplyTuningFuncBit;
    //


    mapCqToP2Cq((ISP_DRV_CQ_ENUM)cq, p2Cq);
    if(ePath == eTuningQueAccessPath_imageio_P2){
        mApplyTuningFuncBit = mTuningQueIdx[p2Cq].keepP2PreviewUpdateFuncBit;
//        mApplyTuningFuncBit = mTuningQueIdx[p2Cq].eCurReadP2UpdateFuncBit;

    } else if(ePath == eTuningQueAccessPath_imageio_P1){
        mApplyTuningFuncBit = mTuningQueIdx[p2Cq].keepP1UpdateFuncBit;
    } else {
        LOG_ERR("[Error]Not support this path(%d)",ePath);
        mApplyTuningFuncBit = 0;
    }
    //
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
    if(ePath == eTuningQueAccessPath_imageio_P2){
        pBuf = mTuningQueIdx[p2Cq].pCurReadP2TuningQue;
    } else if(ePath == eTuningQueAccessPath_imageio_P1) {
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

    if(ePath == eTuningQueAccessPath_imageio_P2){
        if (magicNum & ISP_MAGIC_NUM_CAPTURE_SIGN){ // capture mode
            isPreview = MFALSE;
        }
        //
        switch(top){
            case eTuningTopEn_p1:
                if(isPreview)
                    en = mTuningQueIdx[p2Cq].keepReadP2CtlEnP1;
                else
                    en = mTuningQueIdx[p2Cq].capReadP2CtlEnP1;
                break;
            case eTuningTopEn_p1_dma:
                if(isPreview)
                    en = mTuningQueIdx[p2Cq].keepReadP2CtlEnP1Dma;
                else
                    en = mTuningQueIdx[p2Cq].capReadP2CtlEnP1Dma;
                break;
            case eTuningTopEn_p2:
                if(isPreview)
                    en = mTuningQueIdx[p2Cq].keepReadP2CtlEnP2;
                else
                    en = mTuningQueIdx[p2Cq].capReadP2CtlEnP2;
                break;
            case eTuningTopEn_p2_dma:
                if(isPreview)
                    en = mTuningQueIdx[p2Cq].keepReadP2CtlEnP2Dma;
                else
                    en = mTuningQueIdx[p2Cq].capReadP2CtlEnP2Dma;
                break;
            case eTuningTopEn_p1_d:
            case eTuningTopEn_p1_dma_d:
            default:
                LOG_ERR("[Error]Not support this top(%d) for path(%d)",(MUINT32)top,ePath);
                break;
        }
    } else if(ePath == eTuningQueAccessPath_imageio_P1) {
        switch(top){
            case eTuningTopEn_p1:
                en = mTuningQueIdx[p2Cq].keepReadP1CtlEnP1;
                break;
            case eTuningTopEn_p1_dma:
                en = mTuningQueIdx[p2Cq].keepReadP1CtlEnP1Dma;
                break;
            case eTuningTopEn_p1_d:
                en = mTuningQueIdx[p2Cq].keepReadP1CtlEnP1D;
                break;
            case eTuningTopEn_p1_dma_d:
                en = mTuningQueIdx[p2Cq].keepReadP1CtlEnP1DDma;
                break;
            case eTuningTopEn_p2:
            case eTuningTopEn_p2_dma:
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
            currQueueIdx = prevQueueIdx;
        }while((mTuningQueInf[p2Cq][currQueueIdx].magicNum != magicNum)&&(searchCnt<ISP_TUNING_QUEUE_NUM));

        if(searchCnt>=ISP_TUNING_QUEUE_NUM){
        LOG_INF("[Error]search fail, magic#(0x%x)/Q#(0x%x)/curWriteIdx(0x%x)",magicNum,mTuningQueInf[p2Cq][mTuningQueIdx[p2Cq].curWriteIdx].magicNum,mTuningQueIdx[p2Cq].curWriteIdx);

        if (magicNum < mTuningQueInf[p2Cq][mTuningQueIdx[p2Cq].curWriteIdx].magicNum)
        {
           getQueIdx = mTuningQueIdx[p2Cq].curWriteIdx;     // use oldest tuning parameter
        }
        else
        {
               ret = MFALSE;
        }
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
    MUINT32* pTuningBuf,
    stIspTuningTpipeFieldInf &pTuningField
)
{
    Mutex::Autolock lock(tuningQueueIdxLock);

    MBOOL ret = MTRUE;
    isp_reg_t *pTuningRegMappingQue;

    #if 1
    pTuningRegMappingQue = (isp_reg_t *)pTuningBuf;
    //
    pTuningField.sl2.sl2_hrz_comp = pTuningRegMappingQue->CAM_SL2_HRZ.Bits.SL2_HRZ_COMP;
    //
    pTuningField.cfa.bayer_bypass = pTuningRegMappingQue->CAM_DM_O_BYP.Bits.DM_BYP;
    pTuningField.cfa.dm_fg_mode = pTuningRegMappingQue->CAM_DM_O_BYP.Bits.DM_FG_MODE;   ////this bit be set via path imageio
    //
    pTuningField.mfb.bld_mode = pTuningRegMappingQue->CAM_MFB_CON.Bits.BLD_MODE;
    pTuningField.mfb.bld_deblock_en = pTuningRegMappingQue->CAM_MFB_LL_CON3.Bits.BLD_LL_DB_EN;
    pTuningField.mfb.bld_brz_en = pTuningRegMappingQue->CAM_MFB_LL_CON3.Bits.BLD_LL_BRZ_EN;   ////this bit be set via path imageio
    //
    pTuningField.g2c.g2c_shade_en = pTuningRegMappingQue->CAM_G2C_SHADE_CON_1.Bits.G2C_SHADE_EN;
    //
    pTuningField.sl2b.sl2b_hrz_comp = pTuningRegMappingQue->CAM_SL2B_HRZ.Bits.SL2B_HRZ_COMP;
    //
    pTuningField.nbc.anr_eny = pTuningRegMappingQue->CAM_ANR_CON1.Bits.ANR_ENY;
    pTuningField.nbc.anr_enc = pTuningRegMappingQue->CAM_ANR_CON1.Bits.ANR_ENC;
    pTuningField.nbc.anr_iir_mode = pTuningRegMappingQue->CAM_ANR_CON1.Bits.ANR_FLT_MODE;
    pTuningField.nbc.anr_scale_mode = pTuningRegMappingQue->CAM_ANR_CON1.Bits.ANR_SCALE_MODE;
    //
    pTuningField.sl2c.sl2c_hrz_comp = pTuningRegMappingQue->CAM_SL2C_HRZ.Bits.SL2C_HRZ_COMP;
    //
    pTuningField.seee.se_edge = pTuningRegMappingQue->CAM_SEEE_OUT_EDGE_CTRL.Bits.SEEE_OUT_EDGE_SEL;
    pTuningField.seee.usm_over_shrink_en = pTuningRegMappingQue->CAM_SEEE_CLIP_CTRL.Bits.SEEE_OVRSH_CLIP_EN;
    //
    pTuningField.lce.lce_bc_mag_kubnx = pTuningRegMappingQue->CAM_LCE_ZR.Bits.LCE_BCMK_X;
    pTuningField.lce.lce_slm_width = pTuningRegMappingQue->CAM_LCE_SLM_SIZE.Bits.LCE_SLM_WD;
    pTuningField.lce.lce_bc_mag_kubny = pTuningRegMappingQue->CAM_LCE_ZR.Bits.LCE_BCMK_Y;
    pTuningField.lce.lce_slm_height = pTuningRegMappingQue->CAM_LCE_SLM_SIZE.Bits.LCE_SLM_HT;
    //
    pTuningField.nr3d.nr3d_on_en = pTuningRegMappingQue->CAM_NR3D_CTRL.Bits.NR3D_ON_EN; //this bit be set via path imageio
    pTuningField.nr3d.nr3d_on_xoffset = pTuningRegMappingQue->CAM_NR3D_ON_OFF.Bits.NR3D_ON_OFST_X; //this bit be set via path imageio
    pTuningField.nr3d.nr3d_on_yoffset = pTuningRegMappingQue->CAM_NR3D_ON_OFF.Bits.NR3D_ON_OFST_Y; //this bit be set via path imageio
    pTuningField.nr3d.nr3d_on_width = pTuningRegMappingQue->CAM_NR3D_ON_SIZ.Bits.NR3D_ON_WD; //this bit be set via path imageio
    pTuningField.nr3d.nr3d_on_height = pTuningRegMappingQue->CAM_NR3D_ON_SIZ.Bits.NR3D_ON_HT; //this bit be set via path imageio
    //
    pTuningField.bnr.bpc_en = pTuningRegMappingQue->CAM_BPC_CON.Bits.BPC_EN;
    pTuningField.bnr.bpc_tbl_en = pTuningRegMappingQue->CAM_BPC_D_CON.Bits.BPC_LUT_EN;
    //
    pTuningField.lsc.sdblk_width = pTuningRegMappingQue->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
    pTuningField.lsc.sdblk_xnum = pTuningRegMappingQue->CAM_LSC_CTL2.Bits.LSC_SDBLK_XNUM;
    pTuningField.lsc.sdblk_last_width = pTuningRegMappingQue->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH;
    pTuningField.lsc.sdblk_height = pTuningRegMappingQue->CAM_LSC_CTL3.Bits.LSC_SDBLK_HEIGHT;
    pTuningField.lsc.sdblk_ynum = pTuningRegMappingQue->CAM_LSC_CTL3.Bits.LSC_SDBLK_YNUM;
    pTuningField.lsc.sdblk_last_height = pTuningRegMappingQue->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lHEIGHT;
    //
    pTuningField.lcei.lcei_stride = pTuningRegMappingQue->CAM_LCEI_STRIDE.Bits.STRIDE;
    //
    pTuningField.lsci.lsci_stride = pTuningRegMappingQue->CAM_LSCI_STRIDE.Bits.STRIDE;
    //
    #else
    memcpy((char*)&pTuningField.sl2, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.sl2, sizeof(ISP_TPIPE_CONFIG_SL2_STRUCT));
    memcpy((char*)&pTuningField.cfa, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.cfa, sizeof(ISP_TPIPE_CONFIG_CFA_STRUCT));
    memcpy((char*)&pTuningField.mfb, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.mfb, sizeof(ISP_TPIPE_CONFIG_MFB_STRUCT));
    memcpy((char*)&pTuningField.g2c, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.g2c, sizeof(ISP_TPIPE_CONFIG_G2C_STRUCT));
    memcpy((char*)&pTuningField.sl2b, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.sl2b, sizeof(ISP_TPIPE_CONFIG_SL2B_STRUCT));
    memcpy((char*)&pTuningField.nbc, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.nbc, sizeof(ISP_TPIPE_CONFIG_NBC_STRUCT));
    memcpy((char*)&pTuningField.sl2c, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.sl2c, sizeof(ISP_TPIPE_CONFIG_SL2C_STRUCT));
    memcpy((char*)&pTuningField.seee, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.seee, sizeof(ISP_TPIPE_CONFIG_SEEE_STRUCT));
    memcpy((char*)&pTuningField.lce, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lce, sizeof(ISP_TPIPE_CONFIG_LCE_STRUCT));
    memcpy((char*)&pTuningField.nr3d, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.nr3d, sizeof(ISP_TPIPE_CONFIG_NR3D_STRUCT));
    memcpy((char*)&pTuningField.bnr, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.bnr, sizeof(ISP_TPIPE_CONFIG_BNR_STRUCT));
    memcpy((char*)&pTuningField.lsc, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lsc, sizeof(ISP_TPIPE_CONFIG_LSC_STRUCT));
    memcpy((char*)&pTuningField.lcei, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lcei, sizeof(ISP_TPIPE_CONFIG_LCEI_STRUCT));
    memcpy((char*)&pTuningField.lsci, (char*)&mTuningQueIdx[p2Cq].keepTpipeField.lsci, sizeof(ISP_TPIPE_CONFIG_LSCI_STRUCT));
    #endif

    LOG_DBG("tarCfa.bypass(%d),desCfa.bypass(%d)",mTuningQueIdx[p2Cq].keepTpipeField.cfa.bayer_bypass,pTuningField.cfa.bayer_bypass);


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
        case eSoftwareScenario_Main_Vfb_Stream_1:
        case eSoftwareScenario_Main_Normal_Capture:
        case eSoftwareScenario_Main_Pure_Raw_Stream:
        case eSoftwareScenario_Main_CC_Raw_Stream:
        case eSoftwareScenario_Main_ZSD_Capture:
        case eSoftwareScenario_Main_Mfb_Capture:
        case eSoftwareScenario_Main_Mfb_Blending:
        case eSoftwareScenario_Main_Mfb_Mixing:
        case eSoftwareScenario_Main2_Normal_Stream:
        case eSoftwareScenario_Main2_Vfb_Stream_1:
        case eSoftwareScenario_Main2_Normal_Capture:
        case eSoftwareScenario_Main2_Pure_Raw_Stream:
        case eSoftwareScenario_Main2_CC_Raw_Stream:
        case eSoftwareScenario_Main2_ZSD_Capture:
        case eSoftwareScenario_Main2_Mfb_Capture:
        case eSoftwareScenario_Main2_Mfb_Blending:
        case eSoftwareScenario_Main2_Mfb_Mixing:
        case eSoftwareScenario_Sub_Vfb_Stream_2:
            p2Cq = ISP_DRV_P2_CQ1;
            break;
        case eSoftwareScenario_Main_Vfb_Stream_2:
        case eSoftwareScenario_Sub_Normal_Stream:
        case eSoftwareScenario_Sub_Vfb_Stream_1:
        case eSoftwareScenario_Sub_Pure_Raw_Stream:
        case eSoftwareScenario_Sub_CC_Raw_Stream:
        case eSoftwareScenario_Sub_Normal_Capture:
        case eSoftwareScenario_Sub_ZSD_Capture:
        case eSoftwareScenario_Sub_Mfb_Capture:
        case eSoftwareScenario_Sub_Mfb_Blending:
        case eSoftwareScenario_Sub_Mfb_Mixing:
            p2Cq = ISP_DRV_P2_CQ2;
            break;
        case eSoftwareScenario_Main_VSS_Mfb_Capture:
        case eSoftwareScenario_Main_VSS_Mfb_Blending:
        case eSoftwareScenario_Main_VSS_Mfb_Mixing:
        case eSoftwareScenario_Main2_VSS_Mfb_Capture:
        case eSoftwareScenario_Main2_VSS_Mfb_Blending:
        case eSoftwareScenario_Main2_VSS_Mfb_Mixing:
        case eSoftwareScenario_Sub_VSS_Mfb_Capture:
        case eSoftwareScenario_Sub_VSS_Mfb_Blending:
        case eSoftwareScenario_Sub_VSS_Mfb_Mixing:
        case eSoftwareScenario_Main2_N3D_Stream:
        case eSoftwareScenario_Main_VSS_Capture:
        case eSoftwareScenario_Main2_VSS_Capture:
        case eSoftwareScenario_Sub_VSS_Capture:
            p2Cq = ISP_DRV_P2_CQ3;
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
        case eSoftwareScenario_Main_Vfb_Stream_1:
        case eSoftwareScenario_Main_Normal_Capture:
        case eSoftwareScenario_Main_Pure_Raw_Stream:
        case eSoftwareScenario_Main_ZSD_Capture:
        case eSoftwareScenario_Main_Mfb_Capture:
        case eSoftwareScenario_Main_Mfb_Blending:
        case eSoftwareScenario_Main_Mfb_Mixing:
        case eSoftwareScenario_Main2_Normal_Stream:
        case eSoftwareScenario_Main2_Vfb_Stream_1:
        case eSoftwareScenario_Main2_Normal_Capture:
        case eSoftwareScenario_Main2_Pure_Raw_Stream:
        case eSoftwareScenario_Main2_ZSD_Capture:
        case eSoftwareScenario_Main2_Mfb_Capture:
        case eSoftwareScenario_Main2_Mfb_Blending:
        case eSoftwareScenario_Main2_Mfb_Mixing:
        case eSoftwareScenario_Sub_Vfb_Stream_2:
            cq = ISP_DRV_CQ01;
            break;
        case eSoftwareScenario_Main_Vfb_Stream_2:
        case eSoftwareScenario_Sub_Normal_Stream:
        case eSoftwareScenario_Sub_Vfb_Stream_1:
        case eSoftwareScenario_Sub_Pure_Raw_Stream:
        case eSoftwareScenario_Sub_Normal_Capture:
        case eSoftwareScenario_Sub_ZSD_Capture:
        case eSoftwareScenario_Sub_Mfb_Capture:
        case eSoftwareScenario_Sub_Mfb_Blending:
        case eSoftwareScenario_Sub_Mfb_Mixing:
            cq = ISP_DRV_CQ02;
            break;

        case eSoftwareScenario_Main2_N3D_Stream:
        case eSoftwareScenario_Main_VSS_Mfb_Capture:
        case eSoftwareScenario_Main_VSS_Mfb_Blending:
        case eSoftwareScenario_Main_VSS_Mfb_Mixing:
        case eSoftwareScenario_Main2_VSS_Mfb_Capture:
        case eSoftwareScenario_Main2_VSS_Mfb_Blending:
        case eSoftwareScenario_Main2_VSS_Mfb_Mixing:
        case eSoftwareScenario_Sub_VSS_Mfb_Capture:
        case eSoftwareScenario_Sub_VSS_Mfb_Blending:
        case eSoftwareScenario_Sub_VSS_Mfb_Mixing:
        case eSoftwareScenario_Main_VSS_Capture:
        case eSoftwareScenario_Main2_VSS_Capture:
        case eSoftwareScenario_Sub_VSS_Capture:
            cq = ISP_DRV_CQ03;
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
            p2Cq = ISP_DRV_P2_CQ3;
            break;
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
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL,&edBufQueV);
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
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL,&edBufQueV);
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
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL,&edBufQueV);
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
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL,&edBufQueV);
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
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL,&edBufQueV);
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
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL,&edBufQueV);
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
    ret=ioctl(mFd,ISP_ED_QUEBUF_CTRL,&edBufQueV);
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
    ret=ioctl(mFd,ISP_UPDATE_REGSCEN,&qValue);
    if(ret<0)
    {
        LOG_ERR("ISP_UPDATE_REGSCEN fail(%d).", ret);
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
    ret=ioctl(mFd,ISP_QUERY_REGSCEN,&qValue);
    if(ret<0)
    {
        LOG_ERR("ISP_QUERY_REGSCEN fail(%d).", ret);
        return MFALSE;
    }
    else
    {
        value=qValue;
        return MTRUE;
    }
}
MBOOL IspDrvImp::ISPWakeLockCtrl(MBOOL WakeLockEn)
{
    MBOOL ret=MTRUE;
    MUINT32 wakelock_ctrl = WakeLockEn;
    ret=ioctl(mFd,ISP_WAKELOCK_CTRL,&wakelock_ctrl);
    if(ret<0)
    {
        LOG_ERR("ISP_WAKELOCK_CTRL fail(%d).", ret);
        return MFALSE;
    }
    return MTRUE;
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
IspDrv* IspDrvVirImp::getInstance(MINT32 cq, MUINT32* ispVirRegAddr, isp_reg_t* ispVirRegMap)
{
    LOG_DBG("cq: %d. ispVirRegAddr: 0x%08x.", cq, ispVirRegAddr);
    if ( ((unsigned long)ispVirRegAddr) & ISP_DRV_VIR_ADDR_ALIGN ) {
        LOG_ERR("NOT 8 bytes alignment ");
        return NULL;
    }
    //FIXME, need CHECK
    static IspDrvVirImp singleton[(ISP_DRV_DEFAULT_BURST_QUEUE_NUM*ISP_DRV_P1_CQ_DUPLICATION_NUM*ISP_DRV_P1_PER_CQ_SET_NUM)+(ISP_DVR_MAX_BURST_QUEUE_NUM*ISP_DRV_P2_CQ_DUPLICATION_NUM*ISP_DRV_P2_PER_CQ_SET_NUM)];
    //static IspDrvVirImp singleton[ISP_DRV_TOTAL_CQ_NUM];
    singleton[cq].mpIspVirRegBuffer = ispVirRegAddr;
    singleton[cq].mpIspVirRegMap = ispVirRegMap;
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
    unsigned long     Data,
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
