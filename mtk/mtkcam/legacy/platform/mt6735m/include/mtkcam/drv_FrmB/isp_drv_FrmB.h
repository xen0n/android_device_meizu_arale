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
#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#ifndef _ISP_DRV_FRMB_H_
#define _ISP_DRV_FRMB_H_

#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/drv_FrmB/tpipe_config.h>
#include "utils/Mutex.h"    // For android::Mutex.
#include <mtkcam/drv_common/isp_reg.h>

//*******************************************************************
// Note: Please include following files before including isp_drv_FrmB.h:
//      #include "utils/Mutex.h"    // For android::Mutex.
//      #include <asm/arch/mt6589_sync_write.h> // For dsb() in isp_drv_FrmB.h.
//*******************************************************************

namespace NSIspDrv_FrmB {


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#ifndef USING_MTK_LDVT   // Not using LDVT.
    #include <cutils/log.h>
    #define TEMP_LOG_DBG    ALOGD
    #define TEMP_LOG_INF    ALOGI
    #define TEMP_LOG_WRN    ALOGW
#else   // Using LDVT.
    #include "uvvf.h"
    #define TEMP_LOG_DBG    VV_MSG
    #define TEMP_LOG_INF    VV_MSG
    #define TEMP_LOG_WRN    VV_MSG
#endif  // USING_MTK_LDVT

#define ISP_DRV_RWREG_MODE_RW 1
#define ISP_DRV_RWREG_MODE_RO 0
#define ISP_DRV_RWREG_CALLFROM_MACRO 1
typedef enum
{
    ISP_DRV_USER_OTHER=0x0,
    ISP_DRV_USER_ISPF=0x1,    //isp_function
    ISP_DRV_USER_SENF=0x2,    //seninf
    ISP_DRV_USER_FD=0x3    //fd
}ISP_DRV_USER_ENUM;


// ------------------ ISP Register Read/Write Macros ----------------------
////////temp remove later////////////////////////////////////////////////////////////////
#define ISP_READ_REG(IspDrvPtr,RegName,...)                                \
({                                                                                    \
    MBOOL regV=0;                                                                    \
    printf("[ERROR]please use MACRO ISP_READ_REG_NOPROTECT \n");\
    regV;                                                                            \
})
#define ISP_READ_BITS(IspDrvPtr,RegName,...)                                \
({                                                                                    \
    MBOOL regV=0;                                                                    \
    printf("[ERROR]please use MACRO ISP_READ_BITS_NOPROTECT \n");\
    regV;                                                                            \
})
/////////////////////////////////////////////////////////////////////////////////////////

#define PARSING_USER_                      (0)
#define PARSING_USER_ISP_DRV_USER_OTHER    (0)
#define PARSING_USER_ISP_DRV_USER_ISPF     (1)
#define PARSING_USER_ISP_DRV_USER_SENF     (2)
#define PARSING_USER_ISP_DRV_USER_FD       (3)
#define ISP_PARSE_USER(...) (PARSING_USER_ ## __VA_ARGS__)

#define ISP_BURST_READ_REGS(IspDrvPtr,RegStruct,RegCount,...)                       \
({                                                                                  \
    MBOOL ret=0;                                                                    \
    pthread_mutex_lock(&IspOtherRegMutex);                                            \
    ret=IspDrvPtr->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,ISP_DRV_RWREG_CALLFROM_MACRO);\
    pthread_mutex_unlock(&IspOtherRegMutex);                                          \
    ret;                                                                            \
})
#define ISP_BURST_WRITE_REGS(IspDrvPtr,RegStruct,RegCount,...)                      \
({                                                                                  \
    MBOOL ret=0;                                                                    \
    pthread_mutex_lock(&IspOtherRegMutex);                                            \
    ret=IspDrvPtr->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,ISP_PARSE_USER(__VA_ARGS__),ISP_DRV_RWREG_CALLFROM_MACRO);\
    pthread_mutex_unlock(&IspOtherRegMutex);                                          \
    ret;                                                                            \
})
#define ISP_READ_REG_NOPROTECT(IspDrvPtr,RegName,...)                                \
({                                                                                    \
    MUINT32 addrOffset=(MUINT8*)(&(IspDrvPtr->mpIspVirRegMap->RegName))-(MUINT8*)(IspDrvPtr->mpIspVirRegMap);\
    IspDrvPtr->readReg(addrOffset,ISP_DRV_RWREG_CALLFROM_MACRO);                    \
})

#define ISP_READ_BITS_NOPROTECT(IspDrvPtr,RegName,FieldName,...)                    \
({                                                                                    \
    MUINT32 addrOffset=(MUINT8*)(&(IspDrvPtr->mpIspVirRegMap->RegName))-(MUINT8*)(IspDrvPtr->mpIspVirRegMap);\
    IspDrvPtr->mpIspVirRegMap->RegName.Raw = IspDrvPtr->readReg(addrOffset,ISP_DRV_RWREG_CALLFROM_MACRO);\
    IspDrvPtr->mpIspVirRegMap->RegName.Bits.FieldName;                               \
})

#define ISP_WRITE_REG(IspDrvPtr,RegName,Value,...)                                  \
do{                                                                                    \
    MBOOL ret=0;                                                                    \
    MUINT32 addrOffset=(MUINT8*)(&(IspDrvPtr->mpIspVirRegMap->RegName))-(MUINT8*)(IspDrvPtr->mpIspVirRegMap);\
    ret=IspDrvPtr->checkTopReg(addrOffset);                                         \
    if(ret==1)                                                                      \
    {                                                                                \
        pthread_mutex_lock(&IspTopRegMutex);                                        \
        IspDrvPtr->writeReg(addrOffset, Value,ISP_PARSE_USER(__VA_ARGS__),ISP_DRV_RWREG_CALLFROM_MACRO);\
        pthread_mutex_unlock(&IspTopRegMutex);                                      \
    }                                                                                \
    else                                                                            \
    {                                                                                \
        pthread_mutex_lock(&IspOtherRegMutex);                                      \
        IspDrvPtr->writeReg(addrOffset, Value,ISP_PARSE_USER(__VA_ARGS__),ISP_DRV_RWREG_CALLFROM_MACRO);\
        pthread_mutex_unlock(&IspOtherRegMutex);                                    \
    }                                                                                \
}while(0)

#define ISP_WRITE_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)                    \
do{                                                                                    \
    MBOOL ret=0;                                                                    \
    MUINT32 addrOffset=(MUINT8*)(&(IspDrvPtr->mpIspVirRegMap->RegName))-(MUINT8*)(IspDrvPtr->mpIspVirRegMap);\
    ret=IspDrvPtr->checkTopReg(addrOffset);                                         \
    if(ret==1)                                                                      \
    {                                                                                \
        pthread_mutex_lock(&IspTopRegMutex);                                        \
        IspDrvPtr->mpIspVirRegMap->RegName.Raw=IspDrvPtr->readReg(addrOffset,ISP_DRV_RWREG_CALLFROM_MACRO);\
        IspDrvPtr->mpIspVirRegMap->RegName.Bits.FieldName = FieldValue;                \
        IspDrvPtr->writeReg(addrOffset, IspDrvPtr->mpIspVirRegMap->RegName.Raw,ISP_PARSE_USER(__VA_ARGS__),ISP_DRV_RWREG_CALLFROM_MACRO);\
        pthread_mutex_unlock(&IspTopRegMutex);                                      \
    }                                                                                \
    else                                                                            \
    {                                                                                \
        pthread_mutex_lock(&IspOtherRegMutex);                                      \
        IspDrvPtr->mpIspVirRegMap->RegName.Raw=IspDrvPtr->readReg(addrOffset,ISP_DRV_RWREG_CALLFROM_MACRO);\
        IspDrvPtr->mpIspVirRegMap->RegName.Bits.FieldName = FieldValue;                \
        IspDrvPtr->writeReg(addrOffset, IspDrvPtr->mpIspVirRegMap->RegName.Raw,ISP_PARSE_USER(__VA_ARGS__),ISP_DRV_RWREG_CALLFROM_MACRO);\
        pthread_mutex_unlock(&IspOtherRegMutex);                                    \
    }                                                                                \
}while(0)

// New macro for write following registers:
//  CAM_CTL_START       0x00004000
//  CAM_CTL_EN1         0x00004004
//  CAM_CTL_EN2         0x00004008
//  CAM_CTL_DMA_EN      0x0000400C
//  CAM_CTL_EN1_SET     0x00004080
//  CAM_CTL_EN1_CLR     0x00004084
//  CAM_CTL_EN2_SET     0x00004088
//  CAM_CTL_EN2_CLR     0x0000408C
//  CAM_CTL_DMA_EN_SET  0x00004090
//  CAM_CTL_DMA_EN_CLR  0x00004094
//CAM_CTL_INT_STATUS
#define ISP_DRV_IRQ_INT_STATUS_VS1_ST               ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_INT_STATUS_TG1_ST1              ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_INT_STATUS_TG1_ST2              ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_INT_STATUS_EXPDON1_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_INT_STATUS_TG1_ERR_ST           ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST     ((MUINT32)1 << 10)
#define ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST          ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_INT_STATUS_CQ_ERR_ST            ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST         ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INT_STATUS_TPIPE_DON_ST         ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INT_STATUS_AF_DON_ST            ((MUINT32)1 << 16)
#define ISP_DRV_IRQ_INT_STATUS_FLK_DON_ST           ((MUINT32)1 << 17)
#define ISP_DRV_IRQ_INT_STATUS_FMT_DON_ST           ((MUINT32)1 << 18)
#define ISP_DRV_IRQ_INT_STATUS_CQ_DON_ST            ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INT_STATUS_IMGO_ERR_ST          ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INT_STATUS_AAO_ERR_ST           ((MUINT32)1 << 21)
#define ISP_DRV_IRQ_INT_STATUS_LCSO_ERR_ST          ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INT_STATUS_IMG2O_ERR_ST         ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INT_STATUS_ESFKO_ERR_ST         ((MUINT32)1 << 24)
#define ISP_DRV_IRQ_INT_STATUS_FLK_ERR_ST           ((MUINT32)1 << 25)
#define ISP_DRV_IRQ_INT_STATUS_LSC_ERR_ST           ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INT_STATUS_DROP1_ERR_ST          ((MUINT32)1 << 27)
#define ISP_DRV_IRQ_INT_STATUS_FBC_IMG_DONE_ST           ((MUINT32)1 << 28)
#define ISP_DRV_IRQ_INT_STATUS_DMA_ERR_ST           ((MUINT32)1 << 30)

//CAM_CTL_DMA_INT
#define ISP_DRV_IRQ_DMA_INT_IMGO_DONE_ST            ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMA_INT_IMG2O_DONE_ST           ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMA_INT_AAO_DONE_ST             ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMA_INT_LCSO_DONE_ST            ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMA_INT_ESFKO_DONE_ST           ((MUINT32)1 << 4)
//#define ISP_DRV_IRQ_DMA_INT_DISPO_DONE_ST           ((MUINT32)1 << 5)
//#define ISP_DRV_IRQ_DMA_INT_VIDO_DONE_ST            ((MUINT32)1 << 6)
#define ISP_DRV_IRQ_DMA_INT_CQ0_VR_SNAP_ST          ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMA_INT_CQ0_ERR_ST            ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMA_INT_CQ0_DONE_ST             ((MUINT32)1 << 9)
#define ISP_DRV_IRQ_DMA_INT_TG1_GBERR_ST            ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_DMA_INT_CQ0C_DONE_ST            ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_DMA_INT_CQ0B_DONE_ST            ((MUINT32)1 << 15)


//CAM_CTL_INTB_STATUS
#define ISP_DRV_IRQ_INTB_STATUS_CQ_ERR_ST           ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST        ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INTB_STATUS_TPIPE_DON_ST        ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INTB_STATUS_CQ_DON_ST           ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INTB_STATUS_IMGO_ERR_ST         ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INTB_STATUS_LCSO_ERR_ST         ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INTB_STATUS_IMG2O_ERR_ST        ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INTB_STATUS_LSC_ERR_ST          ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INTB_STATUS_LCE_ERR_ST          ((MUINT32)1 << 29)
#define ISP_DRV_IRQ_INTB_STATUS_DMA_ERR_ST          ((MUINT32)1 << 30)
//CAM_CTL_DMAB_INT
#define ISP_DRV_IRQ_DMAB_INT_IMGO_DONE_ST           ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMAB_INT_IMG2O_DONE_ST          ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMAB_INT_AAO_DONE_ST            ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMAB_INT_LCSO_DONE_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMAB_INT_ESFKO_DONE_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_DMAB_INT_DISPO_DONE_ST          ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_DMAB_INT_VIDO_DONE_ST           ((MUINT32)1 << 6)
//#define ISP_DRV_IRQ_DMAB_INT_VRZO_DONE_ST           ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMAB_INT_NR3O_DONE_ST           ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMAB_INT_NR3O_ERR_ST            ((MUINT32)1 << 9)
//CAM_CTL_INTC_STATUS
#define ISP_DRV_IRQ_INTC_STATUS_CQ_ERR_ST           ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INTC_STATUS_PASS2_DON_ST        ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INTC_STATUS_TPIPE_DON_ST        ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INTC_STATUS_CQ_DON_ST           ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INTC_STATUS_IMGO_ERR_ST         ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INTC_STATUS_LCSO_ERR_ST         ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INTC_STATUS_IMG2O_ERR_ST        ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INTC_STATUS_LSC_ERR_ST          ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INTC_STATUS_BPC_ERR_ST          ((MUINT32)1 << 28)
#define ISP_DRV_IRQ_INTC_STATUS_LCE_ERR_ST          ((MUINT32)1 << 29)
#define ISP_DRV_IRQ_INTC_STATUS_DMA_ERR_ST          ((MUINT32)1 << 30)
//CAM_CTL_DMAC_INT
#define ISP_DRV_IRQ_DMAC_INT_IMGO_DONE_ST           ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMAC_INT_IMG2O_DONE_ST          ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMAC_INT_AAO_DONE_ST            ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMAC_INT_LCSO_DONE_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMAC_INT_ESFKO_DONE_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_DMAC_INT_DISPO_DONE_ST          ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_DMAC_INT_VIDO_DONE_ST           ((MUINT32)1 << 6)
//#define ISP_DRV_IRQ_DMAC_INT_VRZO_DONE_ST           ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMAC_INT_NR3O_DONE_ST           ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMAC_INT_NR3O_ERR_ST            ((MUINT32)1 << 9)
//CAM_CTL_INT_STATUSX
#define ISP_DRV_IRQ_INTX_STATUS_VS1_ST              ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_INTX_STATUS_TG1_ST1             ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_INTX_STATUS_TG1_ST2             ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_INTX_STATUS_EXPDON1_ST          ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_INTX_STATUS_TG1_ERR_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_INTX_STATUS_VS2_ST              ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_INTX_STATUS_TG2_ST1             ((MUINT32)1 << 6)
#define ISP_DRV_IRQ_INTX_STATUS_TG2_ST2             ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_INTX_STATUS_EXPDON2_ST          ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_INTX_STATUS_TG2_ERR_ST          ((MUINT32)1 << 9)
#define ISP_DRV_IRQ_INTX_STATUS_PASS1_TG1_DON_ST    ((MUINT32)1 << 10)
#define ISP_DRV_IRQ_INTX_STATUS_PASS1_TG2_DON_ST    ((MUINT32)1 << 11)
#define ISP_DRV_IRQ_INTX_STATUS_VEC_DON_ST          ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_INTX_STATUS_CQ_ERR_ST           ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INTX_STATUS_PASS2_DON_ST        ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INTX_STATUS_TPIPE_DON_ST        ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INTX_STATUS_AF_DON_ST           ((MUINT32)1 << 16)
#define ISP_DRV_IRQ_INTX_STATUS_FLK_DON_ST          ((MUINT32)1 << 17)
#define ISP_DRV_IRQ_INTX_STATUS_FMT_DON_ST          ((MUINT32)1 << 18)
#define ISP_DRV_IRQ_INTX_STATUS_CQ_DON_ST           ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INTX_STATUS_IMGO_ERR_ST         ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INTX_STATUS_AAO_ERR_ST          ((MUINT32)1 << 21)
#define ISP_DRV_IRQ_INTX_STATUS_LCSO_ERR_ST         ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INTX_STATUS_IMG2O_ERR_ST        ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INTX_STATUS_ESFKO_ERR_ST        ((MUINT32)1 << 24)
#define ISP_DRV_IRQ_INTX_STATUS_FLK_ERR_ST          ((MUINT32)1 << 25)
#define ISP_DRV_IRQ_INTX_STATUS_LSC_ERR_ST          ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INTX_STATUS_LSC2_ERR_ST         ((MUINT32)1 << 27)
#define ISP_DRV_IRQ_INTX_STATUS_BPC_ERR_ST          ((MUINT32)1 << 28)
#define ISP_DRV_IRQ_INTX_STATUS_LCE_ERR_ST          ((MUINT32)1 << 29)
#define ISP_DRV_IRQ_INTX_STATUS_DMA_ERR_ST          ((MUINT32)1 << 30)
//CAM_CTL_DMA_INTX
#define ISP_DRV_IRQ_DMAX_INT_IMGO_DONE_ST           ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMAX_INT_IMG2O_DONE_ST          ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMAX_INT_AAO_DONE_ST            ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMAX_INT_LCSO_DONE_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMAX_INT_ESFKO_DONE_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_DMAX_INT_DISPO_DONE_ST          ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_DMAX_INT_VIDO_DONE_ST           ((MUINT32)1 << 6)
//#define ISP_DRV_IRQ_DMAX_INT_VRZO_DONE_ST           ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMAX_INT_NR3O_DONE_ST           ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMAX_INT_NR3O_ERR_ST            ((MUINT32)1 << 9)
#define ISP_DRV_IRQ_DMAX_INT_CQ_ERR_ST              ((MUINT32)1 << 10)
#define ISP_DRV_IRQ_DMAX_INT_BUF_OVL_ST             ((MUINT32)1 << 11)
#define ISP_DRV_IRQ_DMAX_INT_TG1_GBERR_ST           ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_DMAX_INT_TG2_GBERR_ST           ((MUINT32)1 << 13)
//
#define ISP_DRV_TURNING_TOP_RESET                   (0xffffffff)


//enable table EN1 (4004)
#define CAM_CTL_EN1_TG1_EN  (1L<<0)
#define CAM_CTL_EN1_BIN_EN  (1L<<2)
#define CAM_CTL_EN1_OB_EN   (1L<<3)
#define CAM_CTL_EN1_LSC_EN  (1L<<5)
#define CAM_CTL_EN1_BNR_EN  (1L<<7)
#define CAM_CTL_EN1_SL2_EN  (1L<<8)
#define CAM_CTL_EN1_HRZ_EN  (1L<<9)
#define CAM_CTL_EN1_PGN_EN  (1L<<11)
#define CAM_CTL_EN1_PAK_EN  (1L<<12)
#define CAM_CTL_EN1_PAK2_EN (1L<<13)
#define CAM_CTL_EN1_SGG_EN  (1L<<15)
#define CAM_CTL_EN1_AF_EN   (1L<<16)
#define CAM_CTL_EN1_FLK_EN  (1L<<17)
#define CAM_CTL_EN1_AA_EN   (1L<<18)
#define CAM_CTL_EN1_UNP_EN  (1L<<20)
#define CAM_CTL_EN1_CFA_EN  (1L<<21)
#define CAM_CTL_EN1_CCL_EN  (1L<<22)
#define CAM_CTL_EN1_G2G_EN  (1L<<23)
#define CAM_CTL_EN1_GGM_EN  (1L<<26)
#define CAM_CTL_EN1_C24_EN  (1L<<29)
#define CAM_CTL_EN1_CAM_EN  (1L<<30)

//enable table EN2 (4008)
#define CAM_CTL_EN2_G2C_EN     (1L<<0)
#define CAM_CTL_EN2_C42_EN     (1L<<1)
#define CAM_CTL_EN2_NBC_EN     (1L<<2)
#define CAM_CTL_EN2_PCA_EN     (1L<<3)
#define CAM_CTL_EN2_SEEE_EN    (1L<<4)
#define CAM_CTL_EN2_CQ0C_EN    (1L<<14)
#define CAM_CTL_EN2_CQ0B_EN    (1L<<15)
#define CAM_CTL_EN2_EIS_EN     (1L<<16)
#define CAM_CTL_EN2_CDRZ_EN    (1L<<17)
#define CAM_CTL_EN2_PRZ_EN     (1L<<21)
#define CAM_CTL_EN2_UV_CRSA_EN (1L<<23)
#define CAM_CTL_EN2_FMT_EN     (1L<<26)
#define CAM_CTL_EN2_CQ1_EN     (1L<<27)
#define CAM_CTL_EN2_CQ2_EN     (1L<<28)
#define CAM_CTL_EN2_CQ3_EN     (1L<<29)
#define CAM_CTL_EN2_CQ0_EN     (1L<<31)

//enable table DMA_EN   reg_400C
#define CAM_CTL_DMA_EN_NONE_EN  (0)
#define CAM_CTL_DMA_EN_IMGO_EN  (1L<<0)
#define CAM_CTL_DMA_EN_LSCI_EN  (1L<<1)
#define CAM_CTL_DMA_EN_ESFKO_EN (1L<<3)
#define CAM_CTL_DMA_EN_AAO_EN   (1L<<5)
//#define CAM_CTL_DMA_EN_LCSO_EN  (1L<<6)
#define CAM_CTL_DMA_EN_IMGI_EN  (1L<<7)
#define CAM_CTL_DMA_EN_IMGCI_EN (1L<<8)
#define CAM_CTL_DMA_EN_IMG2O_EN (1L<<10)
#define CAM_CTL_DMA_EN_MDP_IMGXO_IMGO_EN         (1L<<18)  // not isp register (reserved register)
#define CAM_CTL_DMA_EN_WDMAO_EN  (1L<<19)
#define CAM_CTL_DMA_EN_WROTO_EN (1L<<21)


//enable CAM_CTL_PIX_ID  reg_401C
#define CAM_CTL_PIX_ID_PIX_ID               ((1L<<0) + (1L<<1))
#define CAM_CTL_PIX_ID_TG_PIX_ID            ((1L<<2) + (1L<<3))
#define CAM_CTL_PIX_ID_TG_PIX_ID_EN         (1L<<4)
#define CAM_CTL_PIX_ID_BPC_TILE_EDGE_SEL    (1L<<5)
#define CAM_CTL_PIX_ID_BPC_TILE_EDGE_SEL_EN (1L<<6)
#define CAM_CTL_PIX_ID_CDP_EDGE             ((1L<<16) + (1L<<17) + (1L<<18) + (1L<<19))
#define CAM_CTL_PIX_ID_MDP_SEL              (1L<<20)
#define CAM_CTL_PIX_ID_CTL_EXTENSION_EN     (1L<<21)
#define CAM_CTL_PIX_ID_LSC_EARLYSOF_EN      (1L<<21)




//enable table DMA_INT   reg_4028
#define CAM_CTL_DMA_INT_IMGO_DONE_EN    (1L<<16)
#define CAM_CTL_DMA_INT_IMG2O_DONE_EN   (1L<<17)
#define CAM_CTL_DMA_INT_AAO_DONE_EN     (1L<<18)
#define CAM_CTL_DMA_INT_ESFKO_DONE_EN   (1L<<20)
//#define CAM_CTL_DMA_INT_DISPO_DONE_EN   (1L<<21)
//#define CAM_CTL_DMA_INT_VIDO_DONE_EN    (1L<<22)
#define CAM_CTL_DMA_INT_CQ0_VR_SNAP_EN  (1L<<23)
#define CAM_CTL_DMA_INT_CQ0_ERR_EN      (1L<<24)
#define CAM_CTL_DMA_INT_CQ0_DONE_EN     (1L<<25)
#define CAM_CTL_DMA_INT_TG1_GBERR_EN    (1L<<28)
#define CAM_CTL_DMA_INT_CQ0C_DONE_EN    (1L<<30)
#define CAM_CTL_DMA_INT_CQ0B_DONE_EN    (1L<<31)


//enable table DMA_INT   reg_4054
#define CAM_CTL_TCM_CTL_EN              (1L<<0)
#define CAM_CTL_TCM_RSP_EN              (26L<<0)



//MUX_SEL CAM_CTL_MUX_SEL2   reg_4078
#define CAM_CTL_MUX_SEL2_BIN_OUT_SEL_0  (0)
#define CAM_CTL_MUX_SEL2_BIN_OUT_SEL_1  (1L<<2)
#define CAM_CTL_MUX_SEL2_BIN_OUT_SEL_2  (1L<<3)
#define CAM_CTL_MUX_SEL2_BIN_OUT_SEL_3  ((1L<<2) + (1L<<3))
#define CAM_CTL_MUX_SEL2_IMGO_SEL_0     (0)
#define CAM_CTL_MUX_SEL2_IMGO_SEL_1     (1L<<4)
#define CAM_CTL_MUX_SEL2_IMG2O_SEL_0    (0)
#define CAM_CTL_MUX_SEL2_IMG2O_SEL_1    (1L<<6)
#define CAM_CTL_MUX_SEL2_P1_DONE_MUX_0  (1L<<8)
#define CAM_CTL_MUX_SEL2_P1_DONE_MUX_1  (1L<<9)
#define CAM_CTL_MUX_SEL2_P1_DONE_MUX_2  (1L<<10)
#define CAM_CTL_MUX_SEL2_P1_DONE_MUX_3  (1L<<11)
#define CAM_CTL_MUX_SEL2_P1_DONE_MUX_3  (1L<<12)
#define CAM_CTL_MUX_SEL2_BIN_OUT_EN     (1L<<19)
#define CAM_CTL_MUX_SEL2_IMGO_SEL_EN    (1L<<20)
#define CAM_CTL_MUX_SEL2_IMG2O_SEL_EN   (1L<<21)
#define CAM_CTL_MUX_SEL2_LSCI_SOF_SEL_1 (1L<<28)
#define CAM_CTL_MUX_SEL2_LSCI_SOF_SEL_EN (1L<<29)
#define CAM_CTL_MUX_SEL2_P1_DONE_MUX_EN (1L<<30)

//MUX_SEL CAM_CTL_SRAM_MUX_CFG  reg_407c
#define CAM_CTL_SRAM_MUX_CFG_IMG2O_SOF_SEL_1 (1L<<30)
#define CAM_CTL_SRAM_MUX_CFG_SGG_SEL_1  (1L<<28)
#define CAM_CTL_SRAM_MUX_CFG_PGN_SEL_1  (1L<<24)
//#define CAM_CTL_SRAM_MUX_CFG_RGB_SOF_SEL_EN (1L<<19)
#define CAM_CTL_SRAM_MUX_CFG_AAO_SOF_SEL_EN (1L<<18)
#define CAM_CTL_SRAM_MUX_CFG_ESFKO_SOF_SEL_EN (1L<<17)
//#define CAM_CTL_SRAM_MUX_CFG_RGB_SOF_SEL_1  (1L<<14)
#define CAM_CTL_SRAM_MUX_CFG_AAO_SOF_SEL_1  (1L<<13)
#define CAM_CTL_SRAM_MUX_CFG_ESFKO_SOF_SEL  (1L<<12)
#define CAM_CTL_SRAM_MUX_CFG_IMGO_SOF_SEL   (1L<<10)
#define CAM_CTL_SRAM_MUX_CFG_SRAM_MUX_SEL_EN (1L<<8)
#define CAM_CTL_SRAM_MUX_CFG_SRAM_MUX_TILE_EN (1L<<7)
#define CAM_CTL_SRAM_MUX_CFG_SRAM_MUX_MODE_SCENARIO (0x77)

//MUX_SEL CAM_CTL_MUX_SEL reg_4074
#define CAM_CTL_MUX_SEL_BIN_SEL_EN  (1L<<23)
#define CAM_CTL_MUX_SEL_SGG_SEL_EN  (1L<<22)
#define CAM_CTL_MUX_SEL_AA_SEL_EN   (1L<<20)
#define CAM_CTL_MUX_SEL_UNP_SEL_EN  (1L<<19)
#define CAM_CTL_MUX_SEL_BIN_SEL_0   (0)
#define CAM_CTL_MUX_SEL_BIN_SEL_1   (1L<<8)
#define CAM_CTL_MUX_SEL_SGG_SEL_0   (0)
#define CAM_CTL_MUX_SEL_SGG_SEL_1   (1L<<6)
#define CAM_CTL_MUX_SEL_SGG_SEL_2   (1L<<7)
#define CAM_CTL_MUX_SEL_SGG_SEL_3   ( (1L<<6) + (1L<<7) )
#define CAM_CTL_MUX_SEL_AA_SEL_0    (0)
#define CAM_CTL_MUX_SEL_AA_SEL_1    (1L<<3)
#define CAM_CTL_MUX_SEL_UNP_SEL_0   (0)
#define CAM_CTL_MUX_SEL_UNP_SEL_1   (1L<<2)



//
#define ISP_DVR_MAX_BURST_QUEUE_NUM             16
#define ISP_DRV_P2_CQ_DUPLICATION_NUM           2
#define ISP_DRV_P1_CQ_DUPLICATION_NUM           3   //for tuning pingpong
#define ISP_DRV_MAX_CQ_GROUP_SIZE               370
#define ISP_DRV_DEFAULT_BURST_QUEUE_NUM         1
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/*/////////////////////////////////////////////////////////////////////
    CommandQ control
/////////////////////////////////////////////////////////////////////*/
#define ISP_TUNING_QUEUE_NUM        17
#define ISP_TUNING_INIT_IDX         -1
#define ISP_MAGIC_NUM_CAPTURE_SIGN  0x80000000

typedef enum
{
    ISP_DRV_CQTABLE_CTRL_ALLOC  = 0,
    ISP_DRV_CQTABLE_CTRL_DEALLOC,
    ISP_DRV_CQTABLE_CTRL_NUM
}ISP_DRV_CQTABLE_CTRL_ENUM;

typedef enum
{
    ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM  = 0,   //get current supported burst queue number
    ISP_DRV_CQNUMINFO_CTRL_GET_TOTALCQNUM,          //get total cq number including p1 cq and p2 cq
    ISP_DRV_CQNUMINFO_CTRL_GET_P2DUPCQNUM,          //get duplicate number of p2 cq
    ISP_DRV_CQNUMINFO_CTRL_SET_CURBURSTQNUM,        //set current supported burst queue number
    ISP_DRV_CQNUMINFO_CTRL_SET_P2DUPCQNUM,          //set duplicate number of p2 cq
    ISP_DRV_CQNUMINFO_CTRL_UPDATE_TOTALCQNUM,       //update total cq number including p1 cq and p2 cq
    ISP_DRV_CQNUMINFO_CTRL_GET_P1DUPCQNUM,
    ISP_DRV_CQNUMINFO_CTRL_SET_P1DUPCQNUM,          //no user currently
    ISP_DRV_CQNUMINFO_CTRL_NUM
}ISP_DRV_CQNUMINFO_CTRL_ENUM;


/*
-DON'T program by CQ
    -0x04~18,0x74~0x7C
-config first in advance
    -CQ0_MODE
     CQ0B_MODE
     IMGI_EN
     FLKI_EN
     CQ0_BASEADDR
     CQ0B_BASEADDR
     CQ1_BASEADDR
     CQ2_BASEADDR
     CQ3_BASEADDR
*/

typedef enum
{
    ISP_DRV_P2_CQ1 = 0,
    ISP_DRV_P2_CQ2,
    ISP_DRV_P2_CQ3,
    ISP_DRV_P2_CQ_NUM
}ISP_DRV_P2_CQ_ENUM;


typedef enum
{
    ISP_DRV_CQ0 = 0,
    ISP_DRV_CQ0B,
    ISP_DRV_CQ0C,
    ISP_DRV_CQ01,
    ISP_DRV_CQ02,
    ISP_DRV_CQ03,
    ISP_DRV_BASIC_CQ_NUM    //baisc set, pass1 cqs and 1 set of p2 cq
}ISP_DRV_CQ_ENUM;



#define ISP_DRV_P1_PER_CQ_SET_NUM 3//cq0, cq0B, cq0C
#define ISP_DRV_P2_PER_CQ_SET_NUM 3    //cq1,cq2,cq3,cq1_2,cq2_2,cq3_2,...and so on
#define ISP_DRV_P1_DEFAULT_DUPCQIDX 0 //default define for current compile

typedef enum
{
    ISP_DRV_DESCRIPTOR_CQ0 = 0,
    ISP_DRV_DESCRIPTOR_CQ0B,
    ISP_DRV_DESCRIPTOR_CQ0C,
    ISP_DRV_DESCRIPTOR_CQ01,
    ISP_DRV_DESCRIPTOR_CQ02,
    ISP_DRV_DESCRIPTOR_CQ03,
    ISP_DRV_DESCRIPTOR_BASIC_CQ_NUM
}ISP_DRV_DESCRIPTOR_CQ_ENUM;


typedef enum
{
    CQ_SINGLE_IMMEDIATE_TRIGGER = 0,
    CQ_SINGLE_EVENT_TRIGGER,
    CQ_CONTINUOUS_EVENT_TRIGGER,
    CQ_TRIGGER_MODE_NUM
}ISP_DRV_CQ_TRIGGER_MODE_ENUM;

typedef enum
{
    CQ_TRIG_BY_START = 0,
    CQ_TRIG_BY_PASS1_DONE,
    CQ_TRIG_BY_PASS2_DONE,
    CQ_TRIG_BY_IMGO_DONE,
    CQ_TRIG_BY_IMG2O_DONE,
    CQ_TRIG_BY_RRZO_DONE,
    CQ_TRIG_SRC_NUM
}ISP_DRV_CQ_TRIGGER_SOURCE_ENUM;

typedef enum
{
//TH    CAM_TOP_IMGSYS  = 0, // 99   15000000~15000188
    CAM_TOP_CTL = 0,     // 2    15004050~15004054
    CAM_TOP_CTL_01,      // 10   15004080~150040A4
    CAM_TOP_CTL_02,      // 8    150040C0~150040DC
    CAM_ISP_MDP_CROP,    // 2    15004110~15004114
    CAM_DMA_TDRI,        // 3    15004204~1500420C
    CAM_DMA_IMGI,        // 8    15004230~1500424C
    //CAM_DMA_IMGCI,       // 7    15004250~15004268
    CAM_DMA_LSCI,        // 7    1500426C~15004284
    //CAM_DMA_FLKI,        // 7    15004288~150042A0
    //CAM_DMA_LCEI,        // 7    150042A4~150042BC
    //CAM_DMA_VIPI,        // 8    150042C0~150042DC
    //CAM_DMA_VIP2I,       // 8    150042E0~150042FC
    CAM_DMA_IMGO_BASEADDR,  //1, 15004300
    CAM_DMA_IMGO,        // 7    15004304~1500431C  //remove base_addr
    CAM_DMA_IMG2O_BASEADDR,  //1, 15004320
    CAM_DMA_IMG2O,       // 7    15004324~1500433C  //remove base_addr
    //CAM_DMA_LCSO,        // 7    15004340~15004358
    CAM_DMA_EISO,        // 2    1500435C~15004360
    CAM_DMA_AFO,         // 2    15004364~15004368
    CAM_DMA_ESFKO,       // 7    1500436C~15004384
    CAM_DMA_AAO,         // 7    15004388~150043A0
    CAM_RAW_TG1_TG2,     // 56   15004410~15004494
    CAM_ISP_BIN,         // 4    150044F0~150044F8
    CAM_ISP_OBC,         // 8    15004500~1500451C
    CAM_ISP_LSC,         // 8    15004530~1500454C
    CAM_ISP_HRZ,         // 2    15004580~15004584
    CAM_ISP_AWB,         // 36   150045B0~1500463C
    CAM_ISP_AE,          // 13   15004650~15004694 ..Modify
    CAM_ISP_SGG,         // 2    150046A0~150046A4
    CAM_ISP_AF,          // 17   150046B0~15004708 ..Modify
    CAM_ISP_FLK,         // 4    15004770~1500477C
    //CAM_ISP_LCS,         // 6    15004780~15004794
    CAM_ISP_BNR,         // 18   15004800~15004844
    CAM_ISP_PGN,         // 6    15004880~15004894
    CAM_ISP_CFA,         // 22   150048A0~150048F4
    CAM_ISP_CCL,         // 3    15004910~15004918
    CAM_ISP_G2G,         // 7    15004920~15004938
    CAM_ISP_UNP,         // 1    15004948
    //CAM_ISP_C02,         // 1    15004950
    //CAM_ISP_MFB,         // 3    15004960~15004968
    //CAM_ISP_LCE_BASIC_1, // 2    150049C0~150049C4
    //CAM_ISP_LCE_CUSTOM,  // 2    150049C8~150049CC
    //CAM_ISP_LCE_BASIC_2, // 9    150049D0~150049F0
    CAM_ISP_G2C,         // 6    15004A00~15004A14
    CAM_ISP_C42,         // 1    15004A1C
    CAM_ISP_NBC,         // 32   15004A20~15004A9C
    CAM_ISP_SEEE,        // 24   15004AA0~15004AFC
    CAM_CDP_CDRZ,        // 15   14002B00~14002B38
    //CAM_CDP_CURZ,        // 15   15004B40~15004B78
    //CAM_CDP_PRZ,         // 15   15004BA0~15004BD8
    //CAM_CDP_FE,          // 4    15004C20~15004C2C
    //CAM_CDP_VIDO,        // 30   15004CC0~15004D34
    //CAM_CDP_DISPO,       // 30   15004D40~15004DB4
    CAM_ISP_EIS,         // 9    15004DC0~15004DE0
    //CAM_ISP_DGM,         // 6    15004E30~15004E44
    //CAM_GDMA_FMT,        // 36   15004E50~15004EDC
    //CAM_CDP_G2G2,        // 7    15004EE0~15004EF8
    //CAM_CDP_3DNR,        // 15   15004F00~15004F38
    CAM_CDP_SL2_FEATUREIO, // 7    15004F40~15004F58
    CAM_ISP_GGMRB,       // 144  15005000~1500523C
    CAM_ISP_GGMG,        // 144  15005300~1500553C
    CAM_ISP_GGM_CTL,     // 1    15005600
    CAM_ISP_PCA,         // 360  15005800~15005D9C
    CAM_ISP_PCA_CON,     // 2    15005E00~15005E04
//TH    CAM_CTL_VERIF,       // 1    15006000
/* CAN'T by CQ
    CAM_SENINF_TOP,      // 1    15008000
    CAM_SENINF_CTL,      // 45   15008010~150080C0
    CAM_CSI2_CTL,        // 48   15008100~150081BC
    CAM_SENINF_SCAM,     // 17   15008200~15008240
    CAM_SENINF_TG1,      // 5    15008300~15008310
    CAM_SENINF_TG2,      // 5    150083A0~150083B0
    CAM_SENINF_CCIR656,  // 10   15008400~15008424
    CAM_SENINF_N3D,      // 9    15008500~15008520
    CAM_CTL_MIPI_RX_CON, // 21   1500C000~1500C050
*/
    CAM_P1_MAGIC_NUM,    // 1    150043DC
    CAM_ISP_EIS_DB,   // 1    1500406C
    CAM_ISP_EIS_DCM,         // 1    1500419C
    CAM_DUMMY_,
    CAM_MODULE_MAX
}CAM_MODULE_ENUM;

typedef struct cq_desc_t{
    union {
        struct {
            MUINT32 osft_addr  :16;
            MUINT32 cnt        :10;
            MUINT32 inst       :6;
        } token;
        MUINT32 cmd;
    } u;
    MUINT32 v_reg_addr;
}ISP_DRV_CQ_CMD_DESC_STRUCT;

typedef struct {
    MUINT32 cmd_set;
    MUINT32 v_reg_addr;
}ISP_DRV_CQ_CMD_DESC_INIT_STRUCT;

typedef struct {
    unsigned int id;
    unsigned int addr_ofst;
    unsigned int reg_num;
}ISP_DRV_CQ_MODULE_INFO_STRUCT;

typedef struct {
    CAM_MODULE_ENUM grop1;
}ISP_DRV_CQ_BIT_1GP_MAPPING;

typedef struct {
    CAM_MODULE_ENUM grop1;
    CAM_MODULE_ENUM grop2;
}ISP_DRV_CQ_BIT_2GP_MAPPING;

typedef struct {
    CAM_MODULE_ENUM grop1;
    CAM_MODULE_ENUM grop2;
    CAM_MODULE_ENUM grop3;
}ISP_DRV_CQ_BIT_3GP_MAPPING;
//
typedef enum
{
    ISP_DRV_IRQ_CLEAR_NONE,
    ISP_DRV_IRQ_CLEAR_WAIT,
    ISP_DRV_IRQ_CLEAR_STATUS,
    ISP_DRV_IRQ_CLEAR_ALL
}ISP_DRV_IRQ_CLEAR_ENUM;

typedef enum
{
    ISP_DRV_IRQ_TYPE_INT,
    ISP_DRV_IRQ_TYPE_DMA,
    ISP_DRV_IRQ_TYPE_INTB,
    ISP_DRV_IRQ_TYPE_DMAB,
    ISP_DRV_IRQ_TYPE_INTC,
    ISP_DRV_IRQ_TYPE_DMAC,
    ISP_DRV_IRQ_TYPE_INTX,
    ISP_DRV_IRQ_TYPE_DMAX,
    ISP_DRV_IRQ_TYPE_AMOUNT
}ISP_DRV_IRQ_TYPE_ENUM;

typedef enum
{
    ISP_DRV_TURNING_UPDATE_TYPE_TOP_ONLY,
    ISP_DRV_TURNING_UPDATE_TYPE_ENGINE_ONLY,
    ISP_DRV_TURNING_UPDATE_TYPE_TOP_ENGINE,
    ISP_DRV_TURNING_UPDATE_TYPE_NUM
}ISP_DRV_TURNING_UPDATE_TYPE_ENUM;


typedef enum        //remove later...
{
    ISP_DRV_IRQ_USER_ISPDRV =   0,
    ISP_DRV_IRQ_USER_MW     =   1,
    ISP_DRV_IRQ_USER_3A     =   2,
    ISP_DRV_IRQ_USER_HWSYNC =   3,
    ISP_DRV_IRQ_USER_ACDK   =   4,
    ISP_DRV_IRQ_USER_EIS    =   5,
    ISP_DRV_IRQ_USER_VHDR   =   6,
    ISP_DRV_IRQ_USER_MAX
}ISP_DRV_IRQ_USER_ENUM;


typedef enum    //special user for specific operation
{
    ISP_DRV_WAITIRQ_SPEUSER_NONE = 0,
    ISP_DRV_WAITIRQ_SPEUSER_EIS = 1,
    ISP_DRV_WAITIRQ_SPEUSER_NUM
}ISP_DRV_WAITIRQ_SPEUSER_ENUM;
typedef struct ISP_DRV_IRQ_USER_STRUCT
{
    ISP_DRV_IRQ_USER_STRUCT()
    :Type(ISP_DRV_IRQ_TYPE_INT),Status(0x0),UserKey(-1)
    {}
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    unsigned int                          Status;
    int                                           UserKey;                     /* user key for doing interrupt operation */
}ISP_DRV_IRQ_USER_STRUCT;

typedef struct ISP_DRV_IRQ_TIME_STRUCT
{
    ISP_DRV_IRQ_TIME_STRUCT()
    :tLastEvent_sec(0x0),tLastEvent_usec(0x0),tmark2read_sec(0x0),tmark2read_usec(0x0),tevent2read_sec(0x0),tevent2read_usec(0x0),passedbySigcnt(0)
    {}
    unsigned int       tLastEvent_sec;                       /* time stamp of the latest occuring signal*/
    unsigned int       tLastEvent_usec;                  /* time stamp of the latest occuring signal*/
    unsigned int       tmark2read_sec;            /* time period from marking a signal to user try to wait and get the signal*/
    unsigned int       tmark2read_usec;            /* time period from marking a signal to user try to wait and get the signal*/
    unsigned int       tevent2read_sec;         /* time period from latest occuring signal to user try to wait and get the signal*/
    unsigned int       tevent2read_usec;         /* time period from latest occuring signal to user try to wait and get the signal*/
    int                       passedbySigcnt;          /* the count for the signal passed by  */
}ISP_DRV_IRQ_TIME_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_USER_STRUCT  UserInfo;
    ISP_DRV_IRQ_TIME_STRUCT  TimeInfo;
}ISP_DRV_IRQ_TIMEINFO_STRUCT;

typedef struct ISP_DRV_EIS_META_STRUCT
{
    ISP_DRV_EIS_META_STRUCT()
    :tLastSOF2P1done_sec(0x0),tLastSOF2P1done_usec(0x0)
    {}
    unsigned int       tLastSOF2P1done_sec;                       /* time stamp of the last closest occuring sof signal for pass1 done*/
    unsigned int       tLastSOF2P1done_usec;                  /* time stamp of the last closest occuring sof signal for pass1 done*/
}ISP_DRV_EIS_META_STRUCT;


typedef struct ISP_DRV_WAIT_IRQ_STRUCT
{
    ISP_DRV_WAIT_IRQ_STRUCT()
    :Clear(ISP_DRV_IRQ_CLEAR_NONE),SpecUser(ISP_DRV_WAITIRQ_SPEUSER_NONE),Timeout(0x0),bDumpReg(0x0)
    {}
    ISP_DRV_IRQ_CLEAR_ENUM  Clear;
    ISP_DRV_IRQ_USER_STRUCT  UserInfo;
    ISP_DRV_IRQ_TIME_STRUCT  TimeInfo;
    ISP_DRV_EIS_META_STRUCT EisMeta;
    ISP_DRV_WAITIRQ_SPEUSER_ENUM SpecUser;
    unsigned int       Timeout;                     /* time out for waiting for a specific interrupt */
    unsigned int       bDumpReg;
}ISP_DRV_WAIT_IRQ_STRUCT;

typedef struct IISP_DRV_USERKEY_STRUCT
{
    IISP_DRV_USERKEY_STRUCT()
    :userKey(-1),userName("DefaultUserNametoAllocMem")
    {}
    int  userKey;
    char* userName;
}IISP_DRV_USERKEY_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    MUINT32                 Status; //Output
}ISP_DRV_READ_IRQ_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    MUINT32                 Status; //Input
}ISP_DRV_CHECK_IRQ_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    MUINT32                 Status; //Input
}ISP_DRV_CLEAR_IRQ_STRUCT;

typedef struct
{
    MUINT32     Addr;
    MUINT32     Data;
}ISP_DRV_REG_IO_STRUCT;


//struct for enqueue/dequeue control in ihalpipe wrapper
typedef enum
{
    ISP_DRV_BUFQUE_CTRL_ENQUE_FRAME=0,          // 0,signal that a specific buffer is enqueued
    ISP_DRV_BUFQUE_CTRL_WAIT_DEQUE,             // 1,a dequeue thread is waiting to do dequeue
    ISP_DRV_BUFQUE_CTRL_DEQUE_SUCCESS,          // 2,signal that a buffer is dequeued(success)
    ISP_DRV_BUFQUE_CTRL_DEQUE_FAIL,             // 3,signal that a buffer is dequeued(fail)
    ISP_DRV_BUFQUE_CTRL_WAIT_FRAME,             // 4,wait for a specific buffer
    ISP_DRV_BUFQUE_CTRL_WAKE_WAITFRAME,         // 5,wake all sleeped users to check buffer is dequeued or not
    ISP_DRV_BUFQUE_CTRL_CLAER_ALL,              // 6,free all recored dequeued buffer
    ISP_DRV_BUFQUE_CTRL_MAX
}ISP_DRV_ED_BUFQUE_CTRL_ENUM;

typedef struct
{
    ISP_DRV_ED_BUFQUE_CTRL_ENUM  ctrl;
    MUINT32                      processID;
    MUINT32                      callerID;
    MINT32                       p2burstQIdx;
    MINT32                       p2dupCQIdx;
    MUINT32                      timeoutUs;
}ISP_DRV_ED_BUFQUE_STRUCT;


//for unify read/write registers
typedef enum
{
    ISP_DRV_RST_CAM_P1=1,
    ISP_DRV_RST_CAM_P2=2,
    ISP_DRV_RST_CAMSV =3,
    ISP_DRV_RST_CAMSV2=4
}ISP_DRV_RST_PATH;


// for pipe path
enum EIspDrvPipePath
{
    eIspDrvPipePath_P1 = 0,
    eIspDrvPipePath_P2,
    eIspDrvPipePath_Global,
    eIspDrvPipePath_Num
};



// for tuning function
enum ETuningQueAccessPath
{
    eTuningQueAccessPath_imageio_P1 = 0,
    eTuningQueAccessPath_imageio_P2,
    eTuningQueAccessPath_featureio,
    eTuningQueAccessPath_Num
};

#if 1
enum ETuningTopEn
{
    eTuningTopEn1 = 0,
    eTuningTopEn2,
    eTuningTopDmaEn,
    eTuningTopEn_Num,
};
#else
enum ETuningTopEn
{
    eTuningTopEn_p1 = 0,
    eTuningTopEn_p1_dma,
    eTuningTopEn_p1_d,
    eTuningTopEn_p1_dma_d,
    eTuningTopEn_p2,
    eTuningTopEn_p2_dma,
    eTuningTopEn_Num,
};
#endif

enum EIspTuningMgrFuncBit  // need mapping to ETuningMgrFunc / gIspTuningFuncBitMapp
{
    eIspTuningMgrFuncBit_Obc            = 0,
    eIspTuningMgrFuncBit_Lsc            = 1,
    eIspTuningMgrFuncBit_Bnr            = 2,
    eIspTuningMgrFuncBit_Pgn            = 3,
    eIspTuningMgrFuncBit_Cfa            = 4,
    eIspTuningMgrFuncBit_Ccl            = 5,
    eIspTuningMgrFuncBit_G2g            = 6,
    eIspTuningMgrFuncBit_G2c            = 7,
    eIspTuningMgrFuncBit_Nbc            = 8,
    eIspTuningMgrFuncBit_Seee           = 9,
    eIspTuningMgrFuncBit_Sl2            = 10,
    eIspTuningMgrFuncBit_Ggmrb          = 11,
    eIspTuningMgrFuncBit_Ggmg           = 12,
    eIspTuningMgrFuncBit_Pca            = 13,
    eIspTuningMgrFuncBit_Num   //can't over 32
};

enum EIspTuningMgrFunc  // need mapping to ETuningMgrFunc
{
   eIspTuningMgrFunc_Null           = 0x00000000,
   eIspTuningMgrFunc_Obc            = (1<<eIspTuningMgrFuncBit_Obc       ),
   eIspTuningMgrFunc_Lsc            = (1<<eIspTuningMgrFuncBit_Lsc       ),
   eIspTuningMgrFunc_Bnr            = (1<<eIspTuningMgrFuncBit_Bnr       ),
   eIspTuningMgrFunc_Pgn            = (1<<eIspTuningMgrFuncBit_Pgn       ),
   eIspTuningMgrFunc_Cfa            = (1<<eIspTuningMgrFuncBit_Cfa       ),
   eIspTuningMgrFunc_Ccl            = (1<<eIspTuningMgrFuncBit_Ccl       ),
   eIspTuningMgrFunc_G2g            = (1<<eIspTuningMgrFuncBit_G2g       ),
   eIspTuningMgrFunc_G2c            = (1<<eIspTuningMgrFuncBit_G2c       ),
   eIspTuningMgrFunc_Nbc            = (1<<eIspTuningMgrFuncBit_Nbc       ),
   eIspTuningMgrFunc_Seee           = (1<<eIspTuningMgrFuncBit_Seee      ),
   eIspTuningMgrFunc_Sl2            = (1<<eIspTuningMgrFuncBit_Sl2       ),
   eIspTuningMgrFunc_Ggmrb          = (1<<eIspTuningMgrFuncBit_Ggmrb     ),
   eIspTuningMgrFunc_Ggmg           = (1<<eIspTuningMgrFuncBit_Ggmg      ),
   eIspTuningMgrFunc_Pca            = (1<<eIspTuningMgrFuncBit_Pca       ),
   eIspTuningMgrFunc_Num            = (1<<eIspTuningMgrFuncBit_Num       )
};

typedef struct{
    ISP_TPIPE_CONFIG_SL2_STRUCT sl2;
    ISP_TPIPE_CONFIG_CFA_STRUCT cfa;
    ISP_TPIPE_CONFIG_NBC_STRUCT nbc;
    ISP_TPIPE_CONFIG_SEEE_STRUCT seee;
    ISP_TPIPE_CONFIG_LCE_STRUCT lce;
    ISP_TPIPE_CONFIG_BNR_STRUCT bnr;
    ISP_TPIPE_CONFIG_LSC_STRUCT lsc;
    ISP_TPIPE_CONFIG_LCEI_STRUCT lcei;
    ISP_TPIPE_CONFIG_LSCI_STRUCT lsci;
}stIspTuningTpipeFieldInf;


typedef struct{
    MUINT32* pTuningQue; // real tuning queue buffer that be alloaced by isp drv
    MINT32 queFd;
    MINT32 queSize;
    MINT32 magicNum;
    EIspTuningMgrFunc eUpdateFuncBit;
}stIspTuningQueInf;

typedef struct{
    MBOOL isApplyTuning; // if tuning setting by applied
    //
    stIspTuningTpipeFieldInf keepTpipeField; // keep for tuning tpipe
    //
    MUINT32* pCurReadP1TuningQue; // the queue of imageio path be used currently
    EIspTuningMgrFunc eCurReadP1UpdateFuncBit; // fucntion be update by tuning thread
    MUINT32 keepReadP1CtlEn1; //keep current cam_ctl_en1
    MUINT32 keepReadP1CtlEn2; //keep current cam_ctl_en2
    MUINT32 keepReadP1CtlDmaEn; //keep current cam_ctl_dma_en
    //
    stIspTuningQueInf keepP1Que; //keep p1 buffer
    EIspTuningMgrFunc keepP1UpdateFuncBit; // p1 fucntion be updated from tuning thread by now
    //
    MUINT32* pCurWriteTuningQue; // the queue of featureio path be used currently
    //
    MINT32 curWriteIdx;
    MINT32 curReadP1Idx;
    MBOOL  isInitP1;

    MINT32 isCheckP1ReadIdx;
}stIspTuningQueIdx;

////struct for p2 debug dump data
class IspDumpDbgLogP2Package{
public:
    unsigned int *tpipeTableVa;
    unsigned int tpipeTablePa;
    isp_reg_t *pIspReg;
    ISP_TPIPE_CONFIG_STRUCT *pIspTpipeCfgInfo;
};

typedef MINT32 (*CALLBACKFUNC_ISP_DRV)(MINT32 cqNum, MVOID *user);

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern pthread_mutex_t IspRegMutex; // Original IspRegMutex is defined in isp_drv.cpp.
extern pthread_mutex_t IspTopRegMutex;
extern pthread_mutex_t IspOtherRegMutex;

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class IspDrv
{
    friend class IspDrvShellImp;

    protected:
        IspDrv()
            {
                CAM_HAL_VER_IS3 = 0;//initial: v1
            };

        virtual ~IspDrv() {};

    public:
        /*-------------------------------------------------------
          Set Camera version: 0 :camera 1;1 :camera 3 by FLORIA
        -------------------------------------------------------*/
        bool CAM_HAL_VER_IS3;

    public:
        static IspDrv*  createInstance();
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;
        virtual MBOOL   waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MINT32   registerIrq(const char* userName) = 0;
        virtual MBOOL   markIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo) = 0;
        virtual MBOOL   flushIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo) = 0;
        virtual MBOOL   queryirqtimeinfo(ISP_DRV_WAIT_IRQ_STRUCT* Irqinfo) = 0;
        virtual MBOOL  readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq) = 0;
        virtual MBOOL   checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq) = 0;
        virtual MBOOL   clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq) = 0;
        virtual MBOOL   reset(MINT32 rstpath) = 0;
        virtual MBOOL   resetBuf(void) = 0;
        //wrapper
        virtual MBOOL   readRegs(
            ISP_DRV_REG_IO_STRUCT*  pRegIo,
            MUINT32                 Count,
            MINT32                  caller=0) = 0;
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(
            ISP_DRV_REG_IO_STRUCT*  pRegIo,
            MUINT32                 Count,
            MINT32                  userEnum=ISP_DRV_USER_OTHER,
            MINT32                  caller=0) = 0;
        virtual MBOOL   writeReg(
            MUINT32     Addr,
            MUINT32     Data,
            MINT32      userEnum=ISP_DRV_USER_OTHER,
            MINT32      caller=0) = 0;
        //
        virtual MBOOL   holdReg(MBOOL En) = 0;
        virtual MBOOL   dumpReg(void) = 0;
        virtual MBOOL   checkTopReg(MUINT32 Addr);
        virtual isp_reg_t*   getCurHWRegValues()=0;
        virtual MUINT32* getRegAddr(void) = 0;
        virtual MBOOL   ISPWakeLockCtrl(MBOOL WakeLockEn) = 0;
        //
        //commandQ
        MBOOL   cqDbg(void);

        virtual IspDrv* getCQInstance(MINT32 cq);
        virtual MBOOL   cqAddModule(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
        virtual MBOOL   cqDelModule(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
        virtual int getCqModuleInfo(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
        virtual MUINT32* getCQDescBufPhyAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx);
        virtual MUINT32* getCQDescBufVirAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx);
        virtual MUINT32* getCQVirBufVirAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx);
        virtual MINT32  getRealCQIndex(MINT32 cqBaseEnum,MINT32 burstQIdx, MINT32 dupCqIdx);
        virtual MBOOL   setCQTriggerMode(
            ISP_DRV_CQ_ENUM cq,
            ISP_DRV_CQ_TRIGGER_MODE_ENUM mode,
            ISP_DRV_CQ_TRIGGER_SOURCE_ENUM trig_src);
        //
        virtual MBOOL rtBufCtrl(void *pBuf_ctrl) = 0;

        virtual MUINT32 pipeCountInc(EIspDrvPipePath ePipePath) = 0;
        virtual MUINT32 pipeCountDec(EIspDrvPipePath ePipePath) = 0;
        //
        // load default setting
        virtual MBOOL loadInitSetting(void) = 0;
        // debug information
        virtual MBOOL dumpCQTable(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx=0, MUINT32 dupCqIdx=0, ISP_DRV_CQ_CMD_DESC_STRUCT*  cqDesVa=0, MUINT32* cqVirVa=0) = 0;
        //idx0:0 for p1 , 1 for p1_d , 2 for camsv, 3 for camsv_d
        //idx1:debug flag init.    MTRUE/FALSE
        //idx2:prt log.                MTRUE/FALSE
        virtual MBOOL dumpDBGLog(MUINT32* P1,IspDumpDbgLogP2Package* pP2Packages) = 0;
        // for turning update
        virtual MBOOL   getCqModuleInf(CAM_MODULE_ENUM moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize) = 0;
        virtual MBOOL   bypassTuningQue(ESoftwareScenario softScenario, MINT32 magicNum) = 0;
        virtual MBOOL   deTuningQue(ETuningQueAccessPath path, ESoftwareScenario softScenario, MINT32 magicNum) = 0;
        virtual MBOOL   enTuningQue(ETuningQueAccessPath path, ESoftwareScenario softScenario, MINT32 magicNum, EIspTuningMgrFunc updateFuncBit) = 0;
        virtual MBOOL   deTuningQueByCq(ETuningQueAccessPath path, ISP_DRV_CQ_ENUM eCq, MINT32 magicNum) = 0;
        virtual MBOOL   enTuningQueByCq(ETuningQueAccessPath path, ISP_DRV_CQ_ENUM eCq, MINT32 magicNum, EIspTuningMgrFunc updateFuncBit) = 0;
        virtual MBOOL   setP2TuningStatus(ISP_DRV_CQ_ENUM cq, MBOOL en) = 0;
        virtual MBOOL   getP2TuningStatus(ISP_DRV_CQ_ENUM cq) = 0;
        virtual MUINT32 getTuningUpdateFuncBit(ETuningQueAccessPath ePath, MINT32 cq, MUINT32 drvScenario=0, MBOOL isV3=MTRUE) = 0;
        virtual MUINT32* getTuningBuf(ETuningQueAccessPath ePath, MINT32 cq) = 0;
        virtual MUINT32 getTuningTop(ETuningQueAccessPath ePath, ETuningTopEn top, MINT32 cq, MUINT32 magicNum=0) = 0;
        virtual MBOOL   getCqInfoFromScenario(ESoftwareScenario softScenario, ISP_DRV_CQ_ENUM &cq) = 0;
        virtual MBOOL   getP2cqInfoFromScenario(ESoftwareScenario softScenario, ISP_DRV_P2_CQ_ENUM &p2Cq) = 0;
        virtual MBOOL   getTuningTpipeFiled(ISP_DRV_P2_CQ_ENUM p2Cq, stIspTuningTpipeFieldInf &pTuningField) = 0;
        virtual MBOOL   mapCqToP2Cq(ISP_DRV_CQ_ENUM cq, ISP_DRV_P2_CQ_ENUM &p2Cq) = 0;
        // slow motion feature, support burst Queue control
        virtual MBOOL  cqTableControl(ISP_DRV_CQTABLE_CTRL_ENUM cmd,int burstQnum)=0;
        virtual MINT32 cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_ENUM cmd,int newValue=0)=0;
        //
        // enqueue/dequeue control in ihalpipewarpper
        virtual MBOOL   enqueP2Frame(MUINT32 callerID,MINT32 p2burstQIdx,MINT32 p2dupCQIdx)=0;
        virtual MBOOL   waitP2Deque()=0;
        virtual MBOOL   dequeP2FrameSuccess(MUINT32 callerID,MINT32 p2dupCQIdx)=0;
        virtual MBOOL   dequeP2FrameFail(MUINT32 callerID,MINT32 p2dupCQIdx)=0;
        virtual MBOOL   waitP2Frame(MUINT32 callerID,MINT32 p2dupCQIdx,MINT32 timeoutUs)=0;
        virtual MBOOL   wakeP2WaitedFrames()=0;
        virtual MBOOL   freeAllP2Frames()=0;
        //
        //update/query register Scenario
        virtual MBOOL   updateScenarioValue(MUINT32 value)=0;
        virtual MBOOL   queryScenarioValue(MUINT32& value)=0;
        //
        //temp remove later
        virtual isp_reg_t* getRegAddrMap(void) = 0;

        virtual MBOOL getIspCQModuleInfo(CAM_MODULE_ENUM eModule,ISP_DRV_CQ_MODULE_INFO_STRUCT &outInfo) = 0;
        virtual MBOOL SetFPS(MUINT32 _fps) = 0;
        virtual MBOOL updateCq0bRingBuf(void *pOBval) = 0;
    protected:
        virtual MBOOL dumpP2DebugLog(IspDumpDbgLogP2Package* pP2Package) = 0;
        virtual MBOOL dumpP1DebugLog(MUINT32* P1) = 0;
    private:
        mutable android::Mutex   cqPhyDesLock; // for cq phy address
        mutable android::Mutex   cqVirDesLock; // for cq virtual address

    public:
        MINT32          mFd;

        static MUINT32 *mpIspHwRegAddr;

        //for commandQ
        static MINT32          mIspVirRegFd;
        static MUINT32*        mpIspVirRegBufferBaseAddr;
        static MUINT32         mIspVirRegSize;
        static MUINT32**        mpIspVirRegAddrVA;
        static MUINT32**        mpIspVirRegAddrPA;
        static MUINT32         mIspVirRegAddrVAFd;
        static MUINT32         mIspVirRegAddrPAFd;
        //CQ descriptor buffer
        static MINT32          mIspCQDescFd;
        static MUINT32*        mpIspCQDescBufferVirt;
        static MUINT32         mIspCQDescSize;
        static MUINT32*         mpIspCQDescBufferPhy;
        // slow motion feature, support burst Queue control
        static MINT32          mCurBurstQNum;
        static MINT32          mTotalCQNum;
        // for turning
        static stIspTuningQueInf mTuningQueInf[ISP_DRV_P2_CQ_NUM][ISP_TUNING_QUEUE_NUM];
        static stIspTuningQueIdx mTuningQueIdx[ISP_DRV_P2_CQ_NUM];
        //
        //share same member
        static ISP_DRV_CQ_CMD_DESC_STRUCT **mpIspCQDescriptorVirt;
        static MUINT32** mpIspCQDescriptorPhy;
        //
        CALLBACKFUNC_ISP_DRV mpIspDrvCB;
        MVOID *mCallbackCookie;
        //
        MUINT32*        m_pRTBufTbl;
        MUINT32         m_RTBufTblSize;
        //
        MINT32             m_regRWMode;    //0:rw, 1:ro
        //for mdp and tpipe debug
        isp_reg_t*      mpTempIspHWRegValues;
        //for calculating offset in register read/write macro
        static isp_reg_t*      mpIspVirRegMap;

};

};  //namespace NSIspDrv_FrmB




#endif  // _ISP_DRV_FRMB_H_

