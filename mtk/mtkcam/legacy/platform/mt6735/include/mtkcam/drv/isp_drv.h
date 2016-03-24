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
#ifndef _ISP_DRV_H_
#define _ISP_DRV_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1


#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/drv/tpipe_config.h>
#include "utils/Mutex.h"    // For android::Mutex.
#include "isp_reg.h"


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

// ------------------------------------------------------------------------
//CAM_CTL_INT_P1_STATUS
#define CAM_CTL_INT_P1_STATUS_VS1_INT_ST                (1L<<0)
#define CAM_CTL_INT_P1_STATUS_TG1_INT1_ST               (1L<<1)
#define CAM_CTL_INT_P1_STATUS_TG1_INT2_ST               (1L<<2)
#define CAM_CTL_INT_P1_STATUS_EXPDON1_ST                (1L<<3)
#define CAM_CTL_INT_P1_STATUS_TG1_ERR_ST                (1L<<4)
#define CAM_CTL_INT_P1_STATUS_TG1_GBERR                 (1L<<5)
#define CAM_CTL_INT_P1_STATUS_CQ0_ERR                   (1L<<6)
#define CAM_CTL_INT_P1_STATUS_CQ0_VS_ERR_ST             (1L<<7)
#define CAM_CTL_INT_P1_STATUS_IMGO_DROP_FRAME_ST        (1L<<8)
#define CAM_CTL_INT_P1_STATUS_RRZO_DROP_FRAME_ST        (1L<<9)
#define CAM_CTL_INT_P1_STATUS_PASS1_DON_ST              (1L<<10)
#define CAM_CTL_INT_P1_STATUS_rsv_11                    (1L<<11)
#define CAM_CTL_INT_P1_STATUS_SOF1_INT_ST               (1L<<12)
#define CAM_CTL_INT_P1_STATUS_rsv_13                    (1L<<13)
#define CAM_CTL_INT_P1_STATUS_AF_DON_ST                 (1L<<16)
#define CAM_CTL_INT_P1_STATUS_FLK_DON_ST                (1L<<17)
#define CAM_CTL_INT_P1_STATUS_rsv_18                    (1L<<18)
#define CAM_CTL_INT_P1_STATUS_IMGO_ERR_ST               (1L<<20)
#define CAM_CTL_INT_P1_STATUS_AAO_ERR_ST                (1L<<21)
#define CAM_CTL_INT_P1_STATUS_LCSO_ERR_ST               (1L<<22)
#define CAM_CTL_INT_P1_STATUS_RRZO_ERR_ST               (1L<<23)
#define CAM_CTL_INT_P1_STATUS_ESFKO_ERR_ST              (1L<<24)
#define CAM_CTL_INT_P1_STATUS_FLK_ERR_ST                (1L<<25)
#define CAM_CTL_INT_P1_STATUS_LSC_ERR_ST                (1L<<26)
#define CAM_CTL_INT_P1_STATUS_FBC_RRZO_DON_ST           (1L<<27)
#define CAM_CTL_INT_P1_STATUS_FBC_IMGO_DON_ST           (1L<<28)
#define CAM_CTL_INT_P1_STATUS_rsv_29                    (1L<<29)
#define CAM_CTL_INT_P1_STATUS_DMA_ERR_ST                (1L<<30)
#define CAM_CTL_INT_P1_STATUS_rsv_31                    (1L<<31)
//CAM_CTL_INT_P1_STATUS2
#define CAM_CTL_INT_P1_STATUS2_IMGO_DONE_ST              (1L<<0)
#define CAM_CTL_INT_P1_STATUS2_UFEO_DONE_ST              (1L<<1)
#define CAM_CTL_INT_P1_STATUS2_RRZO_DONE_ST              (1L<<2)
#define CAM_CTL_INT_P1_STATUS2_ESFKO_DONE_ST             (1L<<3)
#define CAM_CTL_INT_P1_STATUS2_LCSO_DONE_ST              (1L<<4)
#define CAM_CTL_INT_P1_STATUS2_AAO_DONE_ST               (1L<<5)
#define CAM_CTL_INT_P1_STATUS2_BPCI_DONE_ST              (1L<<6)
#define CAM_CTL_INT_P1_STATUS2_LSCI_DONE_ST              (1L<<7)
#define CAM_CTL_INT_P1_STATUS2_AF_TAR_DONE_ST            (1L<<8)
#define CAM_CTL_INT_P1_STATUS2_AF_FLO1_DONE_ST           (1L<<9)
#define CAM_CTL_INT_P1_STATUS2_AF_FLO2_DONE_ST           (1L<<10)
#define CAM_CTL_INT_P1_STATUS2_AF_FLO3_DONE_ST           (1L<<11)
#define CAM_CTL_INT_P1_STATUS2_rsv_12                    (1L<<12)
//CAM_REG_CTL_INT_P1_STATUS_D
#define CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST                (1L<<0)
#define CAM_CTL_INT_P1_STATUS_D_TG1_INT1_ST               (1L<<1)
#define CAM_CTL_INT_P1_STATUS_D_TG1_INT2_ST               (1L<<2)
#define CAM_CTL_INT_P1_STATUS_D_EXPDON1_ST                (1L<<3)
#define CAM_CTL_INT_P1_STATUS_D_TG1_ERR_ST                (1L<<4)
#define CAM_CTL_INT_P1_STATUS_D_TG1_GBERR                 (1L<<5)
#define CAM_CTL_INT_P1_STATUS_D_CQ0_ERR                   (1L<<6)
#define CAM_CTL_INT_P1_STATUS_D_CQ0_VS_ERR_ST             (1L<<7)
#define CAM_CTL_INT_P1_STATUS_D_IMGO_DROP_FRAME_ST        (1L<<8)
#define CAM_CTL_INT_P1_STATUS_D_RRZO_DROP_FRAME_ST        (1L<<9)
#define CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST              (1L<<10)
#define CAM_CTL_INT_P1_STATUS_D_rsv_11                    (1L<<11)
#define CAM_CTL_INT_P1_STATUS_D_SOF1_INT_ST               (1L<<12)
#define CAM_CTL_INT_P1_STATUS_D_rsv_13                    (1L<<13)
#define CAM_CTL_INT_P1_STATUS_D_AF_DON_ST                 (1L<<16)
#define CAM_CTL_INT_P1_STATUS_D_rsv_17                    (1L<<17)
#define CAM_CTL_INT_P1_STATUS_D_IMGO_ERR_ST               (1L<<20)
#define CAM_CTL_INT_P1_STATUS_D_AAO_ERR_ST                (1L<<21)
#define CAM_CTL_INT_P1_STATUS_D_LCSO_ERR_ST               (1L<<22)
#define CAM_CTL_INT_P1_STATUS_D_RRZO_ERR_ST               (1L<<23)
#define CAM_CTL_INT_P1_STATUS_D_AFO_ERR_ST                (1L<<24)
#define CAM_CTL_INT_P1_STATUS_D_rsv_25                    (1L<<25)
#define CAM_CTL_INT_P1_STATUS_D_LSC_ERR_ST                (1L<<26)
#define CAM_CTL_INT_P1_STATUS_D_FBC_RRZO_DON_ST           (1L<<27)
#define CAM_CTL_INT_P1_STATUS_D_FBC_IMGO_DON_ST           (1L<<28)
#define CAM_CTL_INT_P1_STATUS_D_rsv_29                    (1L<<29)
#define CAM_CTL_INT_P1_STATUS_D_DMA_ERR_ST                (1L<<30)
#define CAM_CTL_INT_P1_STATUS_D_rsv_31                    (1L<<31)
//CAM_REG_CTL_INT_P1_STATUS2_D
#define CAM_CTL_INT_P1_STATUS2_D_IMGO_D_DONE_ST            (1L<<0)
#define CAM_CTL_INT_P1_STATUS2_D_rsv_1                     (1L<<1)
#define CAM_CTL_INT_P1_STATUS2_D_RRZO_D_DONE_ST            (1L<<2)
#define CAM_CTL_INT_P1_STATUS2_D_AFO_D_DONE_ST             (1L<<3)
#define CAM_CTL_INT_P1_STATUS2_D_LCSO_D_DONE_ST            (1L<<4)
#define CAM_CTL_INT_P1_STATUS2_D_AAO_D_DONE_ST             (1L<<5)
#define CAM_CTL_INT_P1_STATUS2_D_BPCI_D_DONE_ST            (1L<<6)
#define CAM_CTL_INT_P1_STATUS2_D_LSCI_D_DONE_ST            (1L<<7)
#define CAM_CTL_INT_P1_STATUS2_D_AF_TAR_DONE_ST            (1L<<8)
#define CAM_CTL_INT_P1_STATUS2_D_AF_FLO1_DONE_ST           (1L<<9)
#define CAM_CTL_INT_P1_STATUS2_D_AF_FLO2_DONE_ST           (1L<<10)
#define CAM_CTL_INT_P1_STATUS2_D_AF_FLO3_DONE_ST           (1L<<11)
#define CAM_CTL_INT_P1_STATUS2_D_rsv_12                    (1L<<12)
//CAM_REG_CTL_INT_P2_STATUS
#define CAM_CTL_INT_P2_STATUS_CQ_ERR_ST                 (1L<<0)
#define CAM_CTL_INT_P2_STATUS_PASS2_DON_ST              (1L<<1)
#define CAM_CTL_INT_P2_STATUS_TILE_DON_ST               (1L<<2)
#define CAM_CTL_INT_P2_STATUS_CQ_DON_ST                 (1L<<3)
#define CAM_CTL_INT_P2_STATUS_TDR_ERR_ST                (1L<<4)
#define CAM_CTL_INT_P2_STATUS_PASS2A_DON_ST             (1L<<5)
#define CAM_CTL_INT_P2_STATUS_PASS2B_DON_ST             (1L<<6)
#define CAM_CTL_INT_P2_STATUS_PASS2C_DON_ST             (1L<<7)
#define CAM_CTL_INT_P2_STATUS_CQ1_DONE_ST               (1L<<8)
#define CAM_CTL_INT_P2_STATUS_CQ2_DONE_ST               (1L<<9)
#define CAM_CTL_INT_P2_STATUS_CQ3_DONE_ST               (1L<<10)
#define CAM_CTL_INT_P2_STATUS_PASS2A_ERR_TRIG_ST        (1L<<11)
#define CAM_CTL_INT_P2_STATUS_PASS2B_ERR_TRIG_ST        (1L<<12)
#define CAM_CTL_INT_P2_STATUS_PASS2C_ERR_TRIG_ST        (1L<<13)
#define CAM_CTL_INT_P2_STATUS_IMGI_DONE_ST              (1L<<15)
#define CAM_CTL_INT_P2_STATUS_UFDI_DONE_ST              (1L<<16)
#define CAM_CTL_INT_P2_STATUS_VIPI_DONE_ST              (1L<<17)
#define CAM_CTL_INT_P2_STATUS_VIP2I_DONE_ST             (1L<<18)
#define CAM_CTL_INT_P2_STATUS_VIP3I_DONE_ST             (1L<<19)
#define CAM_CTL_INT_P2_STATUS_LCEI_DONE_ST              (1L<<20)
#define CAM_CTL_INT_P2_STATUS_MFBO_DONE_ST              (1L<<21)
#define CAM_CTL_INT_P2_STATUS_IMG2O_DONE_ST             (1L<<22)
#define CAM_CTL_INT_P2_STATUS_IMG3O_DONE_ST             (1L<<23)
#define CAM_CTL_INT_P2_STATUS_IMG3BO_DONE_ST            (1L<<24)
#define CAM_CTL_INT_P2_STATUS_IMG3CO_DONE_ST            (1L<<25)
#define CAM_CTL_INT_P2_STATUS_FEO_DONE_ST               (1L<<26)
#define CAM_CTL_INT_P2_STATUS_rsv_27                    (1L<<27)
//CAM_CTL_INT_STATUSX //P1 AND P2
#define CAM_CTL_INT_STATUSX_VS1_INT_ST                (1L<<0)
#define CAM_CTL_INT_STATUSX_TG1_INT1_ST               (1L<<1)
#define CAM_CTL_INT_STATUSX_TG1_INT2_ST               (1L<<2)
#define CAM_CTL_INT_STATUSX_EXPDON1_ST                (1L<<3)
#define CAM_CTL_INT_STATUSX_TG1_ERR_ST                (1L<<4)
#define CAM_CTL_INT_STATUSX_TG1_GBERR                 (1L<<5)
#define CAM_CTL_INT_STATUSX_CQ0_ERR                   (1L<<6)
#define CAM_CTL_INT_STATUSX_CQ0_VS_ERR_ST             (1L<<7)
#define CAM_CTL_INT_STATUSX_IMGO_DROP_FRAME_ST        (1L<<8)
#define CAM_CTL_INT_STATUSX_RRZO_DROP_FRAME_ST        (1L<<9)
#define CAM_CTL_INT_STATUSX_PASS1_DON_ST              (1L<<10)
#define CAM_CTL_INT_STATUSX_rsv_11                    (1L<<11)
#define CAM_CTL_INT_STATUSX_SOF1_INT_ST               (1L<<12)
#define CAM_CTL_INT_STATUSX_CQ_ERR_ST                 (1L<<13)
#define CAM_CTL_INT_STATUSX_PASS2_DON_ST              (1L<<14)
#define CAM_CTL_INT_STATUSX_TILE_DON_ST               (1L<<15)
#define CAM_CTL_INT_STATUSX_AF_DON_ST                 (1L<<16)
#define CAM_CTL_INT_STATUSX_FLK_DON_ST                (1L<<17)
#define CAM_CTL_INT_STATUSX_rsv_18                      (1L<<18)
#define CAM_CTL_INT_STATUSX_CQ_DON_ST                 (1L<<19)
#define CAM_CTL_INT_STATUSX_IMGO_ERR_ST               (1L<<20)
#define CAM_CTL_INT_STATUSX_AAO_ERR_ST                (1L<<21)
#define CAM_CTL_INT_STATUSX_LCSO_ERR_ST               (1L<<22)
#define CAM_CTL_INT_STATUSX_RRZO_ERR_ST               (1L<<23)
#define CAM_CTL_INT_STATUSX_ESFKO_ERR_ST              (1L<<24)
#define CAM_CTL_INT_STATUSX_FLK_ERR_ST                (1L<<25)
#define CAM_CTL_INT_STATUSX_LSC_ERR_ST                (1L<<26)
#define CAM_CTL_INT_STATUSX_FBC_RRZO_DON_ST           (1L<<27)
#define CAM_CTL_INT_STATUSX_FBC_IMGO_DON_ST           (1L<<28)
#define CAM_CTL_INT_STATUSX_rsv_29                    (1L<<29)
#define CAM_CTL_INT_STATUSX_DMA_ERR_ST                (1L<<30)
#define CAM_CTL_INT_STATUSX_rsv_31                    (1L<<31)
//CAM_CTL_INT_STATUS2X //P1_D
#define CAM_CTL_INT_STATUS2X_VS1_INT_ST                (1L<<0)
#define CAM_CTL_INT_STATUS2X_TG1_INT1_ST               (1L<<1)
#define CAM_CTL_INT_STATUS2X_TG1_INT2_ST               (1L<<2)
#define CAM_CTL_INT_STATUS2X_EXPDON1_ST                (1L<<3)
#define CAM_CTL_INT_STATUS2X_TG1_ERR_ST                (1L<<4)
#define CAM_CTL_INT_STATUS2X_TG1_GBERR                 (1L<<5)
#define CAM_CTL_INT_STATUS2X_CQ0_ERR                   (1L<<6)
#define CAM_CTL_INT_STATUS2X_CQ0_VS_ERR_ST             (1L<<7)
#define CAM_CTL_INT_STATUS2X_IMGO_DROP_FRAME_ST        (1L<<8)
#define CAM_CTL_INT_STATUS2X_RRZO_DROP_FRAME_ST        (1L<<9)
#define CAM_CTL_INT_STATUS2X_PASS1_DON_ST              (1L<<10)
#define CAM_CTL_INT_STATUS2X_rsv_11                    (1L<<11)
#define CAM_CTL_INT_STATUS2X_SOF1_INT_ST               (1L<<12)
#define CAM_CTL_INT_STATUS2X_rsv_13                    (1L<<13)
#define CAM_CTL_INT_STATUS2X_rsv_14                    (1L<<14)
#define CAM_CTL_INT_STATUS2X_rsv_15                    (1L<<15)
#define CAM_CTL_INT_STATUS2X_AF_DON_ST                 (1L<<16)
#define CAM_CTL_INT_STATUS2X_rsv_17                    (1L<<17)
#define CAM_CTL_INT_STATUS2X_rsv_18                       (1L<<18)
#define CAM_CTL_INT_STATUS2X_rsv_19                    (1L<<19)
#define CAM_CTL_INT_STATUS2X_IMGO_ERR_ST               (1L<<20)
#define CAM_CTL_INT_STATUS2X_AAO_ERR_ST                (1L<<21)
#define CAM_CTL_INT_STATUS2X_LCSO_ERR_ST               (1L<<22)
#define CAM_CTL_INT_STATUS2X_RRZO_ERR_ST               (1L<<23)
#define CAM_CTL_INT_STATUS2X_AFO_ERR_ST                (1L<<24)
#define CAM_CTL_INT_STATUS2X_rsv_25                    (1L<<25)
#define CAM_CTL_INT_STATUS2X_LSC_ERR_ST                (1L<<26)
#define CAM_CTL_INT_STATUS2X_FBC_RRZO_DON_ST           (1L<<27)
#define CAM_CTL_INT_STATUS2X_FBC_IMGO_DON_ST           (1L<<28)
#define CAM_CTL_INT_STATUS2X_rsv_29                    (1L<<29)
#define CAM_CTL_INT_STATUS2X_DMA_ERR_ST                (1L<<30)
#define CAM_CTL_INT_STATUS2X_rsv_31                    (1L<<31)
//CAM_CTL_INT_STATUS3X //ALL DMA
#define CAM_CTL_INT_STATUS3X_IMGO_DONE_ST              (1L<<0)
#define CAM_CTL_INT_STATUS3X_UFEO_DONE_ST              (1L<<1)
#define CAM_CTL_INT_STATUS3X_RRZO_DONE_ST              (1L<<2)
#define CAM_CTL_INT_STATUS3X_ESFKO_DONE_ST             (1L<<3)
#define CAM_CTL_INT_STATUS3X_LCSO_DONE_ST              (1L<<4)
#define CAM_CTL_INT_STATUS3X_AAO_DONE_ST               (1L<<5)
#define CAM_CTL_INT_STATUS3X_BPCI_DONE_ST              (1L<<6)
#define CAM_CTL_INT_STATUS3X_LSCI_DONE_ST              (1L<<7)
#define CAM_CTL_INT_STATUS3X_IMGO_D_DONE_ST            (1L<<8)
#define CAM_CTL_INT_STATUS3X_RRZO_D_DONE_ST            (1L<<9)
#define CAM_CTL_INT_STATUS3X_AFO_D_DONE_ST             (1L<<10)
#define CAM_CTL_INT_STATUS3X_LCSO_D_DONE_ST            (1L<<11)
#define CAM_CTL_INT_STATUS3X_AAO_D_DONE_ST             (1L<<12)
#define CAM_CTL_INT_STATUS3X_BPCI_D_DONE_ST            (1L<<13)
#define CAM_CTL_INT_STATUS3X_LCSI_D_DONE_ST            (1L<<14)
#define CAM_CTL_INT_STATUS3X_IMGI_DONE_ST              (1L<<15)
#define CAM_CTL_INT_STATUS3X_UFDI_DONE_ST              (1L<<16)
#define CAM_CTL_INT_STATUS3X_VIPI_DONE_ST              (1L<<17)
#define CAM_CTL_INT_STATUS3X_VIP2I_DONE_ST               (1L<<18)
#define CAM_CTL_INT_STATUS3X_VIP3I_DONE_ST             (1L<<19)
#define CAM_CTL_INT_STATUS3X_LCEI_DONE_ST              (1L<<20)
#define CAM_CTL_INT_STATUS3X_MFBO_DONE_ST              (1L<<21)
#define CAM_CTL_INT_STATUS3X_IMG2O_DONE_ST             (1L<<22)
#define CAM_CTL_INT_STATUS3X_IMG3O_DONE_ST             (1L<<23)
#define CAM_CTL_INT_STATUS3X_IMG3BO_DONE_ST            (1L<<24)
#define CAM_CTL_INT_STATUS3X_IMG3CO_DONE_ST            (1L<<25)
#define CAM_CTL_INT_STATUS3X_FEO_DONE_ST               (1L<<26)
#define CAM_CTL_INT_STATUS3X_rsv_27                    (1L<<27)
#define CAM_CTL_INT_STATUS3X_rsv_28                    (1L<<28)
#define CAM_CTL_INT_STATUS3X_rsv_29                    (1L<<29)
#define CAM_CTL_INT_STATUS3X_rsv_30                    (1L<<30)
#define CAM_CTL_INT_STATUS3X_rsv_31                    (1L<<31)
//REG_SENINF1_MUX_INTSTA
#define SENINF1_MUX_OVERRUN_IRQ_STA                       (1L<<0)
#define SENINF1_MUX_CRCERR_IRQ_STA                       (1L<<1)
#define SENINF1_MUX_FSMERR_IRQ_STA                       (1L<<2)
#define SENINF1_MUX_VSIZEERR_IRQ_STA                   (1L<<3)
#define SENINF1_MUX_HSIZEERR_IRQ_STA                   (1L<<4)
#define SENINF1_MUX_SENSOR_VSIZEERR_IRQ_STA               (1L<<5)
#define SENINF1_MUX_SENSOR_HSIZEERR_IRQ_STA               (1L<<6)
//REG_SENINF2_MUX_INTSTA
#define SENINF2_MUX_OVERRUN_IRQ_STA                       (1L<<0)
#define SENINF2_MUX_CRCERR_IRQ_STA                       (1L<<1)
#define SENINF2_MUX_FSMERR_IRQ_STA                       (1L<<2)
#define SENINF2_MUX_VSIZEERR_IRQ_STA                   (1L<<3)
#define SENINF2_MUX_HSIZEERR_IRQ_STA                   (1L<<4)
#define SENINF2_MUX_SENSOR_VSIZEERR_IRQ_STA               (1L<<5)
#define SENINF2_MUX_SENSOR_HSIZEERR_IRQ_STA               (1L<<6)
//REG_SENINF3_MUX_INTSTA
#define SENINF3_MUX_OVERRUN_IRQ_STA                       (1L<<0)
#define SENINF3_MUX_CRCERR_IRQ_STA                       (1L<<1)
#define SENINF3_MUX_FSMERR_IRQ_STA                       (1L<<2)
#define SENINF3_MUX_VSIZEERR_IRQ_STA                   (1L<<3)
#define SENINF3_MUX_HSIZEERR_IRQ_STA                   (1L<<4)
#define SENINF3_MUX_SENSOR_VSIZEERR_IRQ_STA               (1L<<5)
#define SENINF3_MUX_SENSOR_HSIZEERR_IRQ_STA               (1L<<6)
//REG_SENINF4_MUX_INTSTA
#define SENINF4_MUX_OVERRUN_IRQ_STA                       (1L<<0)
#define SENINF4_MUX_CRCERR_IRQ_STA                       (1L<<1)
#define SENINF4_MUX_FSMERR_IRQ_STA                       (1L<<2)
#define SENINF4_MUX_VSIZEERR_IRQ_STA                   (1L<<3)
#define SENINF4_MUX_HSIZEERR_IRQ_STA                   (1L<<4)
#define SENINF4_MUX_SENSOR_VSIZEERR_IRQ_STA               (1L<<5)
#define SENINF4_MUX_SENSOR_HSIZEERR_IRQ_STA               (1L<<6)
//REG_CAMSV_INT_STATUS
#define CAMSV_INT_STATUS_VS1_ST                        (1L<<0)
#define CAMSV_INT_STATUS_TG_ST1                        (1L<<1)
#define CAMSV_INT_STATUS_TG_ST2                        (1L<<2)
#define CAMSV_INT_STATUS_EXPDON1_ST                    (1L<<3)
#define CAMSV_INT_STATUS_TG_ERR_ST                    (1L<<4)
#define CAMSV_INT_STATUS_TG_GBERR_ST                (1L<<5)
#define CAMSV_INT_STATUS_TG_DROP_ST                    (1L<<6)
#define CAMSV_INT_STATUS_TG_SOF1_ST                    (1L<<7)
#define CAMSV_INT_STATUS_rsv_8                        (1L<<8)
#define CAMSV_INT_STATUS_rsv_9                        (1L<<9)
#define CAMSV_INT_STATUS_PASS1_DON_ST                (1L<<10)
#define CAMSV_INT_STATUS_rsv_11                        (1L<<11)
#define CAMSV_INT_STATUS_rsv_12                        (1L<<12)
#define CAMSV_INT_STATUS_rsv_13                        (1L<<13)
#define CAMSV_INT_STATUS_rsv_14                        (1L<<14)
#define CAMSV_INT_STATUS_rsv_15                        (1L<<15)
#define CAMSV_INT_STATUS_IMGO_ERR_ST                (1L<<16)
#define CAMSV_INT_STATUS_IMGO_OVERR_ST                (1L<<17)
#define CAMSV_INT_STATUS_rsv_18                        (1L<<18)
#define CAMSV_INT_STATUS_IMGO_DROP_ST                (1L<<19)
//REG_CAMSV2_INT_STATUS
#define CAMSV2_INT_STATUS_VS1_ST                    (1L<<0)
#define CAMSV2_INT_STATUS_TG_ST1                    (1L<<1)
#define CAMSV2_INT_STATUS_TG_ST2                    (1L<<2)
#define CAMSV2_INT_STATUS_EXPDON1_ST                (1L<<3)
#define CAMSV2_INT_STATUS_TG_ERR_ST                    (1L<<4)
#define CAMSV2_INT_STATUS_TG_GBERR_ST                (1L<<5)
#define CAMSV2_INT_STATUS_TG_DROP_ST                (1L<<6)
#define CAMSV2_INT_STATUS_TG_SOF1_ST                (1L<<7)
#define CAMSV2_INT_STATUS_rsv_8                        (1L<<8)
#define CAMSV2_INT_STATUS_rsv_9                        (1L<<9)
#define CAMSV2_INT_STATUS_PASS1_DON_ST                (1L<<10)
#define CAMSV2_INT_STATUS_rsv_11                    (1L<<11)
#define CAMSV2_INT_STATUS_rsv_12                    (1L<<12)
#define CAMSV2_INT_STATUS_rsv_13                    (1L<<13)
#define CAMSV2_INT_STATUS_rsv_14                    (1L<<14)
#define CAMSV2_INT_STATUS_rsv_15                    (1L<<15)
#define CAMSV2_INT_STATUS_IMGO_ERR_ST                (1L<<16)
#define CAMSV2_INT_STATUS_IMGO_OVERR_ST                (1L<<17)
#define CAMSV2_INT_STATUS_rsv_18                    (1L<<18)
#define CAMSV2_INT_STATUS_IMGO_DROP_ST                (1L<<19)

//
#define ISP_DRV_TURNING_TOP_RESET                   (0xffffffff)
//
//enable CAM_REG_CTL_EN_P1   reg_4004
#define CAM_CTL_EN_P1_TG1_EN                    (1L<<0)
#define CAM_CTL_EN_P1_DMX_EN                    (1L<<1)
#define CAM_CTL_EN_P1_WBN_EN                    (1L<<2)
#define CAM_CTL_EN_P1_W2G_EN                    (1L<<3)
#define CAM_CTL_EN_P1_OB_EN                     (1L<<4)
#define CAM_CTL_EN_P1_BNR_EN                    (1L<<5)
#define CAM_CTL_EN_P1_LSC_EN                    (1L<<6)
#define CAM_CTL_EN_P1_RPG_EN                    (1L<<7)
#define CAM_CTL_EN_P1_RRZ_EN                    (1L<<8)
#define CAM_CTL_EN_P1_RMX_EN                    (1L<<9)
#define CAM_CTL_EN_P1_PAKG_EN                   (1L<<10)
#define CAM_CTL_EN_P1_BMX_EN                    (1L<<11)
#define CAM_CTL_EN_P1_PAK_EN                    (1L<<12)
#define CAM_CTL_EN_P1_UFE_EN                    (1L<<13)
#define CAM_CTL_EN_P1_LCS_EN                    (1L<<14)
#define CAM_CTL_EN_P1_SGG1_EN                   (1L<<15)
#define CAM_CTL_EN_P1_AF_EN                     (1L<<16)
#define CAM_CTL_EN_P1_FLK_EN                    (1L<<17)
#define CAM_CTL_EN_P1_HBIN_EN                   (1L<<18)
#define CAM_CTL_EN_P1_AA_EN                     (1L<<19)
#define CAM_CTL_EN_P1_SGG2_EN                   (1L<<20)
#define CAM_CTL_EN_P1_EIS_EN                    (1L<<21)
#define CAM_CTL_EN_P1_RMG_EN                    (1L<<22)
#define CAM_CTL_EN_P1_rsv_23                    (1L<<23)
//enable CAM_CTL_EN_P1_DMA   reg_4008
#define CAM_CTL_EN_P1_DMA_NONE_EN                   (0)
#define CAM_CTL_EN_P1_DMA_IMGO_EN                   (1L<<0)
#define CAM_CTL_EN_P1_DMA_UFEO_EN                   (1L<<1)
#define CAM_CTL_EN_P1_DMA_RRZO_EN                   (1L<<2)
#define CAM_CTL_EN_P1_DMA_ESFKO_EN                  (1L<<3)
#define CAM_CTL_EN_P1_DMA_LCSO_EN                   (1L<<4)
#define CAM_CTL_EN_P1_DMA_AAO_EN                    (1L<<5)
#define CAM_CTL_EN_P1_DMA_BPCI_EN                   (1L<<6)
#define CAM_CTL_EN_P1_DMA_LSCI_EN                   (1L<<7)
#define CAM_CTL_EN_P1_DMA_rsv_8                     (1L<<0)
//enable CAM_REG_CTL_EN_P1_D   reg_4010
#define CAM_CTL_EN_P1_D_TG1_D_EN                  (1L<<0)
#define CAM_CTL_EN_P1_D_rsv_1                     (1L<<1)
#define CAM_CTL_EN_P1_D_WBN_D_EN                  (1L<<2)
#define CAM_CTL_EN_P1_D_W2G_D_EN                  (1L<<3)
#define CAM_CTL_EN_P1_D_OB_D_EN                   (1L<<4)
#define CAM_CTL_EN_P1_D_BNR_D_EN                  (1L<<5)
#define CAM_CTL_EN_P1_D_LSC_D_EN                  (1L<<6)
#define CAM_CTL_EN_P1_D_RPG_D_EN                  (1L<<7)
#define CAM_CTL_EN_P1_D_RRZ_D_EN                  (1L<<8)
#define CAM_CTL_EN_P1_D_rsv_9                     (1L<<9)
#define CAM_CTL_EN_P1_D_PAKG_D_EN                 (1L<<10)
#define CAM_CTL_EN_P1_D_rsv_11                    (1L<<11)
#define CAM_CTL_EN_P1_D_PAK_D_EN                  (1L<<12)
#define CAM_CTL_EN_P1_D_rsv_13                    (1L<<13)
#define CAM_CTL_EN_P1_D_LCS_D_EN                  (1L<<14)
#define CAM_CTL_EN_P1_D_SGG1_D_EN                 (1L<<15)
#define CAM_CTL_EN_P1_D_AF_D_EN                   (1L<<16)
#define CAM_CTL_EN_P1_D_rsv_17                    (1L<<17)
#define CAM_CTL_EN_P1_D_AA_D_EN                   (1L<<19)
#define CAM_CTL_EN_P1_D_rsv_20                    (1L<<20)
#define CAM_CTL_EN_P1_D_RMG_EN                  (1L<<22)
#define CAM_CTL_EN_P1_D_rsv_23                    (1L<<23)
//enable CAM_CTL_EN_P1_DMA_D   reg_4014
#define CAM_CTL_EN_P1_DMA_D_IMGO_D_EN                 (1L<<0)
#define CAM_CTL_EN_P1_DMA_D_rsv_1                     (1L<<1)
#define CAM_CTL_EN_P1_DMA_D_RRZO_D_EN                 (1L<<2)
#define CAM_CTL_EN_P1_DMA_D_AFO_D_EN                  (1L<<3)
#define CAM_CTL_EN_P1_DMA_D_LCSO_D_EN                 (1L<<4)
#define CAM_CTL_EN_P1_DMA_D_AAO_D_EN                  (1L<<5)
#define CAM_CTL_EN_P1_DMA_D_BPCI_D_EN                 (1L<<6)
#define CAM_CTL_EN_P1_DMA_D_LSCI_D_EN                 (1L<<7)
#define CAM_CTL_EN_P1_DMA_D_rsv_8                     (1L<<8)
//enable CAM_CTL_EN_P2   reg_4018
#define CAM_CTL_EN_P2_UFD_EN                    (1L<<0)
#define CAM_CTL_EN_P2_PGN_EN                    (1L<<1)
#define CAM_CTL_EN_P2_SL2_EN                    (1L<<2)
#define CAM_CTL_EN_P2_CFA_EN                    (1L<<3)
#define CAM_CTL_EN_P2_CCL_EN                    (1L<<4)
#define CAM_CTL_EN_P2_G2G_EN                    (1L<<5)
#define CAM_CTL_EN_P2_GGM_EN                    (1L<<6)
#define CAM_CTL_EN_P2_MFB_EN                    (1L<<7)
#define CAM_CTL_EN_P2_C24_EN                    (1L<<8)
#define CAM_CTL_EN_P2_G2C_EN                    (1L<<9)
#define CAM_CTL_EN_P2_C42_EN                    (1L<<10)
#define CAM_CTL_EN_P2_NBC_EN                    (1L<<11)
#define CAM_CTL_EN_P2_PCA_EN                    (1L<<12)
#define CAM_CTL_EN_P2_SEEE_EN                   (1L<<13)
#define CAM_CTL_EN_P2_LCE_EN                    (1L<<14)
#define CAM_CTL_EN_P2_NR3D_EN                   (1L<<15)
#define CAM_CTL_EN_P2_SL2B_EN                   (1L<<16)
#define CAM_CTL_EN_P2_SL2C_EN                   (1L<<17)
#define CAM_CTL_EN_P2_SRZ1_EN                   (1L<<18)
#define CAM_CTL_EN_P2_SRZ2_EN                   (1L<<19)
#define CAM_CTL_EN_P2_CRZ_EN                    (1L<<20)
#define CAM_CTL_EN_P2_MIX1_EN                   (1L<<21)
#define CAM_CTL_EN_P2_MIX2_EN                   (1L<<22)
#define CAM_CTL_EN_P2_MIX3_EN                   (1L<<23)
#define CAM_CTL_EN_P2_MIR1_EN                   (1L<<24)
#define CAM_CTL_EN_P2_MIR2_EN                   (1L<<25)
#define CAM_CTL_EN_P2_CRSP_EN                   (1L<<26)
#define CAM_CTL_EN_P2_C24B_EN                   (1L<<27)
#define CAM_CTL_EN_P2_MDPCROP_EN                (1L<<28)
#define CAM_CTL_EN_P2_FE_EN                     (1L<<29)
#define CAM_CTL_EN_P2_UNP_EN                    (1L<<30)
#define CAM_CTL_EN_P2_C02_EN                    (1L<<31)
//enable CAM_CTL_EN_P2_DMA   reg_401C
#define CAM_CTL_EN_P2_DMA_NONE_EN                   (0)
#define CAM_CTL_EN_P2_DMA_IMGI_EN                   (1L<<0)
#define CAM_CTL_EN_P2_DMA_UFDI_EN                   (1L<<1)
#define CAM_CTL_EN_P2_DMA_VIPI_EN                   (1L<<2)
#define CAM_CTL_EN_P2_DMA_VIP2I_EN                  (1L<<3)
#define CAM_CTL_EN_P2_DMA_VIP3I_EN                  (1L<<4)
#define CAM_CTL_EN_P2_DMA_LCEI_EN                   (1L<<5)
#define CAM_CTL_EN_P2_DMA_MFBO_EN                   (1L<<6)
#define CAM_CTL_EN_P2_DMA_IMG2O_EN                  (1L<<7)
#define CAM_CTL_EN_P2_DMA_IMG3O_EN                  (1L<<8)
#define CAM_CTL_EN_P2_DMA_IMG3BO_EN                 (1L<<9)
#define CAM_CTL_EN_P2_DMA_IMG3CO_EN                 (1L<<10)
#define CAM_CTL_EN_P2_DMA_FEO_EN                    (1L<<11)
#define CAM_CTL_EN_P2_DMA_MDP_IMGXO_IMGO_EN         (1L<<20)  // not isp register (reserved register)
#define CAM_CTL_EN_P2_DMA_MDP_WROTO_EN              (1L<<21)  // not isp register (reserved register)
#define CAM_CTL_EN_P2_DMA_MDP_WDMAO_EN              (1L<<22)  // not isp register (reserved register)
#define CAM_CTL_EN_P2_DMA_MDP_JPEGO_EN              (1L<<23)  // not isp register (reserved register)
#define CAM_CTL_EN_P2_DMA_MDP_VENC_EN               (1L<<24)  // not isp register (reserved register)



//enable CAM_CTL_CQ_EN   reg_4020
#define CAM_CTL_CQ_EN_rsv_0                     (1L<<0)
#define CAM_CTL_CQ_EN_CQ0A_MASK_CQ0B_D_EN       (1L<<1)
#define CAM_CTL_CQ_EN_CQ0A_MASK_CQ0C_D_EN       (1L<<2)
#define CAM_CTL_CQ_EN_CQ0_CONT_D                (1L<<3)
#define CAM_CTL_CQ_EN_CQ0B_CONT_D               (1L<<4)
#define CAM_CTL_CQ_EN_CQ0C_IMGO_D_SEL           (1L<<5)
#define CAM_CTL_CQ_EN_CQ0B_D_SEL                (1L<<6)
#define CAM_CTL_CQ_EN_CQ0_D_MODE                (1L<<7)
#define CAM_CTL_CQ_EN_CQ0C_D_RRZO_SEL           (1L<<8)
#define CAM_CTL_CQ_EN_CQ0B_D_MODE               (1L<<9)
#define CAM_CTL_CQ_EN_CQ0C_D_EN                 (1L<<10)
#define CAM_CTL_CQ_EN_CQ0B_D_EN                 (1L<<11)
#define CAM_CTL_CQ_EN_rsv_12                    (1L<<12)
#define CAM_CTL_CQ_EN_CQ0_D_EN                  (1L<<15)
#define CAM_CTL_CQ_EN_rsv_16                    (1L<<16)
#define CAM_CTL_CQ_EN_CQ0A_MASK_CQ0B_EN         (1L<<17)
#define CAM_CTL_CQ_EN_CQ0A_MASK_CQ0C_EN         (1L<<18)
#define CAM_CTL_CQ_EN_CQ0_CONT                  (1L<<19)
#define CAM_CTL_CQ_EN_CQ0B_CONT                 (1L<<20)
#define CAM_CTL_CQ_EN_CQ0C_IMGO_SEL             (1L<<21)
#define CAM_CTL_CQ_EN_CQ0B_SEL                  (1L<<22)
#define CAM_CTL_CQ_EN_CQ0_MODE                  (1L<<23)
#define CAM_CTL_CQ_EN_CQ0C_RRZO_SEL             (1L<<24)
#define CAM_CTL_CQ_EN_CQ0B_MODE                 (1L<<25)
#define CAM_CTL_CQ_EN_CQ0C_EN                   (1L<<26)
#define CAM_CTL_CQ_EN_CQ0B_EN                   (1L<<27)
#define CAM_CTL_CQ_EN_CQ0_EN                    (1L<<28)
//enable CAM_CTL_FMT_SEL_P2   reg_4030

//enable CAM_CTL_SEL_P1   reg_4034
#define CAM_CTL_SEL_P1_IMG_SEL                  (1L<<6)
#define CAM_CTL_SEL_P1_UFE_SEL                  (1L<<8)
#define CAM_CTL_SEL_P1_UFE_SEL2                    (1L<<9)
//enable CAM_CTL_SEL_P1_D   reg_4038
#define CAM_CTL_SEL_P1_D_IMG_SEL_D              (1L<<6)
#define CAM_CTL_SEL_P1_D_UFE_SEL_D              (1L<<8)
#define CAM_CTL_SEL_P1_D_UFE_SEL2_D                (1L<<9)



//enable CAM_CTL_SEL_P1   reg_4038
#define CAM_CTL_SEL_P1_D_IMG_SEL_D              (1L<<6)



//enable CAM_CTL_INT_P1_EN   reg_4048
#define CAM_CTL_INT_P1_EN_VS1_INT_EN                (1L<<0)
#define CAM_CTL_INT_P1_EN_TG1_INT1_EN               (1L<<1)
#define CAM_CTL_INT_P1_EN_TG1_INT2_EN               (1L<<2)
#define CAM_CTL_INT_P1_EN_EXPDON1_EN                (1L<<3)
#define CAM_CTL_INT_P1_EN_TG1_ERR_EN                (1L<<4)
#define CAM_CTL_INT_P1_EN_TG1_GBERR_EN              (1L<<5)
#define CAM_CTL_INT_P1_EN_CQ0_ERR_EN                (1L<<6)
#define CAM_CTL_INT_P1_EN_CQ0_VS_ERR_EN             (1L<<7)
#define CAM_CTL_INT_P1_EN_IMGO_DROP_FRAME_EN        (1L<<8)
#define CAM_CTL_INT_P1_EN_RRZO_DROP_FRAME_EN        (1L<<9)
#define CAM_CTL_INT_P1_EN_PASS1_DON_EN              (1L<<10)
#define CAM_CTL_INT_P1_EN_rsv_11                    (1L<<11)
#define CAM_CTL_INT_P1_EN_SOF1_INT_EN               (1L<<12)
#define CAM_CTL_INT_P1_EN_rsv_13                    (1L<<13)
#define CAM_CTL_INT_P1_EN_AF_DON_EN                 (1L<<16)
#define CAM_CTL_INT_P1_EN_FLK_DON_EN                (1L<<17)
#define CAM_CTL_INT_P1_EN_rsv_18                    (1L<<18)
#define CAM_CTL_INT_P1_EN_IMGO_ERR_EN               (1L<<20)
#define CAM_CTL_INT_P1_EN_AAO_ERR_EN                (1L<<21)
#define CAM_CTL_INT_P1_EN_LCSO_ERR_EN               (1L<<22)
#define CAM_CTL_INT_P1_EN_RRZO_ERR_EN               (1L<<23)
#define CAM_CTL_INT_P1_EN_ESFKO_ERR_EN              (1L<<24)
#define CAM_CTL_INT_P1_EN_FLK_ERR_EN                (1L<<25)
#define CAM_CTL_INT_P1_EN_LSC_ERR_EN                (1L<<26)
#define CAM_CTL_INT_P1_EN_FBC_RRZO_DON_EN           (1L<<27)
#define CAM_CTL_INT_P1_EN_FBC_IMGO_DON_EN           (1L<<28)
#define CAM_CTL_INT_P1_EN_rsv_29                    (1L<<29)
#define CAM_CTL_INT_P1_EN_DMA_ERR_EN                (1L<<30)
#define CAM_CTL_INT_P1_EN_INT_WCLR_EN               (1L<<31)
//CAM_REG_CTL_INT_P1_EN_D  reg_4058
#define CAM_CTL_INT_P1_EN_D_VS1_INT_EN                (1L<<0)
#define CAM_CTL_INT_P1_EN_D_TG1_INT1_EN               (1L<<1)
#define CAM_CTL_INT_P1_EN_D_TG1_INT2_EN               (1L<<2)
#define CAM_CTL_INT_P1_EN_D_EXPDON1_EN                (1L<<3)
#define CAM_CTL_INT_P1_EN_D_TG1_ERR_EN                (1L<<4)
#define CAM_CTL_INT_P1_EN_D_TG1_GBERR_EN              (1L<<5)
#define CAM_CTL_INT_P1_EN_D_CQ0_ERR_EN                (1L<<6)
#define CAM_CTL_INT_P1_EN_D_CQ0_VS_ERR_EN             (1L<<7)
#define CAM_CTL_INT_P1_EN_D_IMGO_DROP_FRAME_EN        (1L<<8)
#define CAM_CTL_INT_P1_EN_D_RRZO_DROP_FRAME_EN        (1L<<9)
#define CAM_CTL_INT_P1_EN_D_PASS1_DON_EN              (1L<<10)
#define CAM_CTL_INT_P1_EN_D_rsv_11                    (1L<<11)
#define CAM_CTL_INT_P1_EN_D_SOF1_INT_EN               (1L<<12)
#define CAM_CTL_INT_P1_EN_D_rsv_13                    (1L<<13)
#define CAM_CTL_INT_P1_EN_D_AF_DON_EN                 (1L<<16)
#define CAM_CTL_INT_P1_EN_D_rsv_17                    (1L<<17)
#define CAM_CTL_INT_P1_EN_D_IMGO_ERR_EN               (1L<<20)
#define CAM_CTL_INT_P1_EN_D_AAO_ERR_EN                (1L<<21)
#define CAM_CTL_INT_P1_EN_D_LCSO_ERR_EN               (1L<<22)
#define CAM_CTL_INT_P1_EN_D_RRZO_ERR_EN               (1L<<23)
#define CAM_CTL_INT_P1_EN_D_AFO_ERR_EN                (1L<<24)
#define CAM_CTL_INT_P1_EN_D_rsv_25                    (1L<<25)
#define CAM_CTL_INT_P1_EN_D_LSC_ERR_EN                (1L<<26)
#define CAM_CTL_INT_P1_EN_D_FBC_RRZO_DON_EN           (1L<<27)
#define CAM_CTL_INT_P1_EN_D_FBC_IMGO_DON_EN           (1L<<28)
#define CAM_CTL_INT_P1_EN_D_rsv_29                    (1L<<29)
#define CAM_CTL_INT_P1_EN_D_DMA_ERR_EN                (1L<<30)
#define CAM_CTL_INT_P1_EN_D_rsv_31                    (1L<<31)
//
//CAM_REG_CTL_TPIPE   reg_4078
#define CAM_CTL_TPIPE_RSV_0                     (1L<<0)
#define CAM_CTL_TPIPE_RSV_1                     (1L<<1)
#define CAM_CTL_TPIPE_RSV_2                     (1L<<2)
#define CAM_CTL_TPIPE_RSV_3                     (1L<<3)
#define CAM_CTL_TPIPE_RSV_4                     (1L<<4)
#define CAM_CTL_TPIPE_RSV_5                     (1L<<5)
#define CAM_CTL_TPIPE_C02_TCM_EN                     (1L<<6)
#define CAM_CTL_TPIPE_C24B_TCM_EN                     (1L<<7)
#define CAM_CTL_TPIPE_RSV_8                     (1L<<8)
#define CAM_CTL_TPIPE_RSV_9                     (1L<<9)
#define CAM_CTL_TPIPE_RSV_10                     (1L<<10)
#define CAM_CTL_TPIPE_RSV_11                     (1L<<11)
#define CAM_CTL_TPIPE_LCEI_TCM_EN                     (1L<<12)
#define CAM_CTL_TPIPE_UFDI_TCM_EN                     (1L<<13)
#define CAM_CTL_TPIPE_VIPI_TCM_EN                     (1L<<14)
#define CAM_CTL_TPIPE_VIP2I_TCM_EN                     (1L<<15)
#define CAM_CTL_TPIPE_VIP3I_TCM_EN                     (1L<<16)
#define CAM_CTL_TPIPE_FEO_TCM_EN                     (1L<<17)
#define CAM_CTL_TPIPE_IMG2O_TCM_EN                     (1L<<18)
#define CAM_CTL_TPIPE_IMG3O_TCM_EN                     (1L<<19)
#define CAM_CTL_TPIPE_IMG3BO_TCM_EN                     (1L<<20)
#define CAM_CTL_TPIPE_IMG3CO_TCM_EN                     (1L<<21)
#define CAM_CTL_TPIPE_FE_TCM_EN                     (1L<<22)
#define CAM_CTL_TPIPE_CRZ_TCM_EN                     (1L<<23)
#define CAM_CTL_TPIPE_MDPCROP_TCM_EN                     (1L<<24)
#define CAM_CTL_TPIPE_LCE_TCM_EN                     (1L<<25)
#define CAM_CTL_TPIPE_SRZ1_TCM_EN                     (1L<<26)
#define CAM_CTL_TPIPE_SRZ2_TCM_EN                     (1L<<27)
#define CAM_CTL_TPIPE_CRSP_TCM_EN                     (1L<<28)
#define CAM_CTL_TPIPE_NR3D_TCM_EN                     (1L<<29)
#define CAM_CTL_TPIPE_EXTENSION_EN                     (1L<<30)
#define CAM_CTL_TPIPE_MIX3_TCM_EN                     (1L<<31)
//CAM_LCE_IMAGE_SIZE reg_49e4 (bit mask)
#define CAM_LCE_SLR_SIZE_REG_OFFSET                 (0x49E4)
#define CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_WD_BIT_MASK  (0x0000007f)
#define CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_WD_BIT_SHIFT (0)
#define CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_HT_BIT_MASK  (0x00007f00)
#define CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_HT_BIT_SHIFT (8)
//CAM_REG_NR3D_CTRL reg_4cf8 (bit mask)
#define CAM_REG_NR3D_CTRL_LMT_OUT_CNT_TH_BIT_MASK   (0x00000003)
#define CAM_REG_NR3D_CTRL_LMT_OUT_CNT_TH_BIT_SHIFT  (0)
//CAM_NR3D_BLEND (bit mask)
#define CAM_NR3D_BLEND_OFFSET                       (0x4CC0)
#define CAM_NR3D_GAIN_BIT_MASK                      (0x0000001f)
#define CAM_NR3D_GAIN_BIT_SHIFT                     (0)
#define CAM_NR3D_RND_Y_BIT_MASK                     (0x00001f00)
#define CAM_NR3D_RND_Y_BIT_SHIFT                    (8)
#define CAM_NR3D_RND_U_BIT_MASK                     (0x001f0000)
#define CAM_NR3D_RND_U_BIT_SHIFT                    (16)
#define CAM_NR3D_RND_V_BIT_MASK                     (0x1f000000)
#define CAM_NR3D_RND_V_BIT_SHIFT                    (24)



//enable CAM_CTL_CQ_EN_P2   reg_410C
#define CAM_CTL_CQ_EN_P2_CQ1_EN                     (1L<<0)
#define CAM_CTL_CQ_EN_P2_CQ2_EN                     (1L<<1)
#define CAM_CTL_CQ_EN_P2_CQ3_EN                     (1L<<2)
//reg_9000
#define CAMSV_CAMSV_MODULE_TG_EN                    (1L<<0)
#define CAMSV_CAMSV_MODULE_PAK_EN                    (1L<<2)
#define CAMSV_CAMSV_MODULE_PAK_SEL                    (1L<<3)
#define CAMSV_CAMSV_MODULE_IMGO_EN                    (1L<<4)
#define CAMSV_CAMSV_MODULE_DB_EN                    (1L<<30)
#define CAMSV_CAMSV_MODULE_DB_LOCK                    (1L<<31)
//reg_9008
#define CAMSV_CAMSV_INT_EN_VS1_INT_EN                (1L<<0)
#define CAMSV_CAMSV_INT_EN_TG_INT1_EN                (1L<<1)
#define CAMSV_CAMSV_INT_EN_TG_INT2_EN                (1L<<2)
#define CAMSV_CAMSV_INT_EN_EXPDON1_INT_EN            (1L<<3)
#define CAMSV_CAMSV_INT_EN_TG_ERR_INT_EN            (1L<<4)
#define CAMSV_CAMSV_INT_EN_TG_GBERR_INT_EN            (1L<<5)
#define CAMSV_CAMSV_INT_EN_TG_DROP_INT_EN            (1L<<6)
#define CAMSV_CAMSV_INT_EN_TG_SOF_INT_EN            (1L<<7)
#define CAMSV_CAMSV_INT_EN_PASS1_DON_INT_EN            (1L<<10)
#define CAMSV_CAMSV_INT_EN_IMGO_ERR_INT_EN            (1L<<16)
#define CAMSV_CAMSV_INT_EN_IMGO_OVERR_INT_EN        (1L<<17)
#define CAMSV_CAMSV_INT_EN_IMGO_DROP_INT_EN            (1L<<19)
#define CAMSV_CAMSV_INT_EN_INT_WCLR_EN                (1L<<31)
//reg_900c
#define CAMSV_CAMSV_INT_STATUS_VS1_ST               (1L<<0)
#define CAMSV_CAMSV_INT_STATUS_TG_ST1               (1L<<1)
#define CAMSV_CAMSV_INT_STATUS_TG_ST2               (1L<<2)
#define CAMSV_CAMSV_INT_STATUS_EXPDON1_ST           (1L<<3)
#define CAMSV_CAMSV_INT_STATUS_TG_ERR_ST            (1L<<4)
#define CAMSV_CAMSV_INT_STATUS_TG_GBERR_ST          (1L<<5)
#define CAMSV_CAMSV_INT_STATUS_TG_DROP_INT_ST       (1L<<6)
#define CAMSV_CAMSV_INT_STATUS_TG_SOF1_INT_ST       (1L<<7)
#define CAMSV_CAMSV_INT_STATUS_PASS1_DON_ST         (1L<<10)
#define CAMSV_CAMSV_INT_STATUS_IMGO_ERR_ST          (1L<<16)
#define CAMSV_CAMSV_INT_STATUS_IMGO_OVERR_ST        (1L<<17)
#define CAMSV_CAMSV_INT_STATUS_IMGO_DROP_ST         (1L<<19)



//reg_9800
#define CAMSV_CAMSV2_MODULE_TG_EN                    (1L<<0)
#define CAMSV_CAMSV2_MODULE_PAK_EN                    (1L<<2)
#define CAMSV_CAMSV2_MODULE_PAK_SEL                    (1L<<3)
#define CAMSV_CAMSV2_MODULE_IMGO_EN                    (1L<<4)
#define CAMSV_CAMSV2_MODULE_DB_EN                    (1L<<30)
#define CAMSV_CAMSV2_MODULE_DB_LOCK                    (1L<<31)
//reg_9808
#define CAMSV_CAMSV2_INT_EN_VS1_INT_EN                (1L<<0)
#define CAMSV_CAMSV2_INT_EN_TG_INT1_EN                (1L<<1)
#define CAMSV_CAMSV2_INT_EN_TG_INT2_EN                (1L<<2)
#define CAMSV_CAMSV2_INT_EN_EXPDON1_INT_EN            (1L<<3)
#define CAMSV_CAMSV2_INT_EN_TG_ERR_INT_EN            (1L<<4)
#define CAMSV_CAMSV2_INT_EN_TG_GBERR_INT_EN            (1L<<5)
#define CAMSV_CAMSV2_INT_EN_TG_DROP_INT_EN            (1L<<6)
#define CAMSV_CAMSV2_INT_EN_TG_SOF_INT_EN            (1L<<7)
#define CAMSV_CAMSV2_INT_EN_PASS1_DON_INT_EN        (1L<<10)
#define CAMSV_CAMSV2_INT_EN_IMGO_ERR_INT_EN            (1L<<16)
#define CAMSV_CAMSV2_INT_EN_IMGO_OVERR_INT_EN        (1L<<17)
#define CAMSV_CAMSV2_INT_EN_IMGO_DROP_INT_EN        (1L<<19)
#define CAMSV_CAMSV2_INT_EN_INT_WCLR_EN                (1L<<31)
//reg_980c
#define CAMSV_CAMSV2_INT_STATUS_VS1_ST               (1L<<0)
#define CAMSV_CAMSV2_INT_STATUS_TG_ST1               (1L<<1)
#define CAMSV_CAMSV2_INT_STATUS_TG_ST2               (1L<<2)
#define CAMSV_CAMSV2_INT_STATUS_EXPDON1_ST           (1L<<3)
#define CAMSV_CAMSV2_INT_STATUS_TG_ERR_ST            (1L<<4)
#define CAMSV_CAMSV2_INT_STATUS_TG_GBERR_ST          (1L<<5)
#define CAMSV_CAMSV2_INT_STATUS_TG_DROP_INT_ST       (1L<<6)
#define CAMSV_CAMSV2_INT_STATUS_TG_SOF1_INT_ST       (1L<<7)
#define CAMSV_CAMSV2_INT_STATUS_PASS1_DON_ST         (1L<<10)
#define CAMSV_CAMSV2_INT_STATUS_IMGO_ERR_ST          (1L<<16)
#define CAMSV_CAMSV2_INT_STATUS_IMGO_OVERR_ST        (1L<<17)
#define CAMSV_CAMSV2_INT_STATUS_IMGO_DROP_ST         (1L<<19)


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
#define ISP_TUNING_QUEUE_NUM        60
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
    ISP_DRV_CQ0_D,
    ISP_DRV_CQ0B_D,
    ISP_DRV_CQ0C_D,
    ISP_DRV_CQ0C_CAMSV,
    ISP_DRV_CQ0C_CAMSV2,
    ISP_DRV_CQ01,
    ISP_DRV_CQ02,
    ISP_DRV_CQ03,
    ISP_DRV_BASIC_CQ_NUM    //baisc set, pass1 cqs and 1 set of p2 cq
}ISP_DRV_CQ_ENUM;

#define ISP_DRV_P1_PER_CQ_SET_NUM 8
#define ISP_DRV_P2_PER_CQ_SET_NUM 3    //cq1,cq2,cq3,cq1_2,cq2_2,cq3_2,...and so on
#define ISP_DRV_P1_DEFAULT_DUPCQIDX 0 //default define for current compile

typedef enum
{
    ISP_DRV_DESCRIPTOR_CQ0 = 0,
    ISP_DRV_DESCRIPTOR_CQ0B,
    ISP_DRV_DESCRIPTOR_CQ0C,
    ISP_DRV_DESCRIPTOR_CQ0_D,
    ISP_DRV_DESCRIPTOR_CQ0B_D,
    ISP_DRV_DESCRIPTOR_CQ0C_D,
    ISP_DRV_DESCRIPTOR_CQ0C_CAMSV,
    ISP_DRV_DESCRIPTOR_CQ0C_CAMSV2,
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
    CQ_TRIG_BY_RRZO_DONE,
    CQ_TRIG_SRC_NUM
}ISP_DRV_CQ_TRIGGER_SOURCE_ENUM;

//
typedef enum {
//TH    CAM_TOP_IMGSYS  = 0, // 99   15000000~15000188
    CAM_TOP_CTL_EN_P1 = 0,  // 2    15004004~15004008
    CAM_TOP_CTL_EN_P1_D,    // 2    15004010~15004014
    CAM_TOP_CTL_EN_P2,      // 2    15004018~1500401C
    CAM_TOP_CTL_SCEN,       // 1    15004024~15004024
    CAM_TOP_CTL_FMT_P1,     // 1    15004028
    CAM_TOP_CTL_FMT_P1_D,   // 1    1500402C
    CAM_TOP_CTL_FMT_P2,     // 1    15004030~15004030
    CAM_TOP_CTL_SEL_P1,     // 1    15004034
    CAM_TOP_CTL_SEL_P1_D,   // 1    15004038
    CAM_TOP_CTL_SEL_P2,     // 1    1500403C~1500403C
    CAM_CTL_TCM_EN,         // 1    1500407C~1500407C
    CAM_CTL_TDR_EN,         // 1    15004084~15004084
    CAM_CTL_IHDR,           // 1    15004104~15004104
    CAM_CTL_IHDR_D,         // 1    15004108~15004108
    CAM_RAW_TG1_TG2,     // 56   15004410~150044EC
    CAM_ISP_BIN,         // 4    150044F0~150044FC
    CAM_ISP_OBC,         // 8    15004500~1500451C
    CAM_ISP_LSC,         // 8    15004530~1500454C
    CAM_ISP_RPG,         // 6    15004550~15004564
    CAM_ISP_SGG3,        // 3    15004570~15004578
    CAM_ISP_SGG2,        // 3    15004580~15004588
    CAM_ISP_AWB,         // 36   150045B0~1500463C
    CAM_ISP_AE,          // 18   15004650~15004694
    CAM_ISP_SGG1,        // 3    150046A0~150046A8
    CAM_ISP_AF,          // 34   150046B0~15004734
    CAM_ISP_W2G,         // 4   15004740~1500474C
    CAM_ISP_WBN,         // 2   15004760~15004764
    CAM_ISP_FLK,         // 4    15004770~1500477C
    CAM_ISP_LCS,         // 6    15004780~15004794
    CAM_ISP_RRZ,         // 11    150047A0~150047C8
    CAM_ISP_BNR,         // 22   15004800~15004844
    CAM_ISP_PGN,         // 6    15004880~15004894
    CAM_ISP_CFA,         // 22   150048A0~150048F4
    CAM_ISP_CCL,         // 3    15004910~15004918
    CAM_ISP_G2G,         // 7    15004920~15004938
    CAM_ISP_UNP,         // 1    15004948
    CAM_ISP_C02,         // 3    15004950~15004958
    CAM_ISP_MFB_CTRL,    // 1    15004960
    CAM_ISP_MFB_IMGCON,  // 1    15004964
    CAM_ISP_MFB_TUNECON, // 5    15004968~15004978
    CAM_ISP_LCE,                // 12   150049C0~150049E8
    CAM_ISP_LCE_IMAGE_SIZE,     // 1    150049F0
    CAM_ISP_C42,         // 1    15004A1C
    CAM_ISP_NBC,         // 30   15004A20~15004A94
    CAM_ISP_SEEE,        // 24   15004AA0~15004AFC
    CAM_ISP_CRZ,         // 15   15004B00~15004B38
    CAM_ISP_G2C_CONV,    // 6    15004BA0~15004BB4
    CAM_ISP_G2C_SHADE,   // 5    15004BB8~15004BC8
    CAM_CDP_FE,          // 4    15004C20~15004C2C
    CAM_CDP_SRZ1,        // 9    15004C30~15004C50
    CAM_CDP_SRZ2,        // 9    15004C60~15004C80
    CAM_CDP_MIX1,        // 3    15004C90~15004C98
    CAM_CDP_MIX2,        // 3    15004CA0~15004CA8
    CAM_CDP_MIX3,        // 3    15004CB0~15004CB8
    CAM_CDP_NR3D_BLENDING,  // 1    15004CC0~15004CC0
    CAM_CDP_NR3D_FB,        // 3    15004CC4~15004CCC
    CAM_CDP_NR3D_LMT,       // 10   15004CD0~15004CF4
    CAM_CDP_NR3D_CTRL,      // 3    15004CF8~15004D00
    CAM_ISP_EIS,         // 9    15004DC0~15004DE0
    CAM_ISP_DMX,         // 3    15004E00~15004E08
    CAM_ISP_BMX,         // 3    15004E10~15004E18
    CAM_ISP_RMX,         // 3    15004E20~15004E28
    CAM_ISP_UFE,         // 1    15004E50
    CAM_ISP_UFD,         // 5    15004E60~15004E70
    CAM_ISP_SL2,         // 7    15004F40~15004F58
    CAM_ISP_SL2B,        // 7    15004F60~15004F78
    CAM_ISP_CRSP,        // 6    15004FA0~15004FB4
    CAM_ISP_SL2C,        // 7    15004FC0~15004FD8
    CAM_ISP_GGM_RB,      // 144  15005000~1500523C
    CAM_ISP_GGM_G,       // 144  15005240~1500547C
    CAM_ISP_GGM_TOP,     // 1    15005480~15005480
    CAM_ISP_PCA_TBL,     // 360  15005800~15005D9C
    CAM_ISP_PCA_CON,     // 2    15005E00~15005E04
    CAM_ISP_TILE_RING,   // 1   15006000
    CAM_ISP_IMGI_SIZE,   // 1   15006004
    CAM_TOP_CTL_SEL_GLOBAL_SET, // 1    15006040~15006040
    CAM_ISP_OBC_D,       //8   15006500~1500651C
    CAM_ISP_LSC_D,       //8    15006530~1500654C
    CAM_ISP_RPG_D,       //6    15006550~15006564
    CAM_ISP_AWB_D,       // 36   150065B0~1500663C
    CAM_ISP_AE_D,        // 18   15006650~15006694
    CAM_ISP_SGG1_D,      // 3   150066A0~150066A8
    CAM_ISP_AF_D,        //34   150066B0~15006734
    CAM_ISP_W2G_D,       // 4   15006740~1500674C
    CAM_ISP_WBN_D,       // 2   15006760~15006764
    CAM_ISP_LCS_D,       // 6   15006780~15006794
    CAM_ISP_RRZ_D,       // 11  150067A0~150067C8
    CAM_ISP_BNR_D,       // 22   15006800~15006854
    CAM_ISP_DMX_D,       // 3    15006E00~15006E08
    CAM_ISP_BMX_D,       // 3    15006E10~15006E18
    CAM_ISP_RMX_D,       // 3    15006E20~15006E28
    CAM_TOP_CTL_SEL_GLOBAL_CLR, // 1    15007040~15007040
    CAM_DMA_TDRI,        // 3    15007204~1500720C
    CAM_DMA_IMGI_F,      // 1    15007220~15007224
    CAM_DMA_IMGI,        // 8    1500722C~1500724C
    CAM_DMA_BPCI,        // 7    15007250~15007268
    CAM_DMA_LSCI,        // 5    1500726C~1500727C
    CAM_DMA_UFDI,        // 7    15007288~150072A0
    CAM_DMA_LCEI,        // 7    150072A4~150072BC
    CAM_DMA_VIPI,        // 8    150072C0~150072DC
    CAM_DMA_VIP2I,       // 8    150072E0~150072FC
    CAM_DMA_IMGO_BASE_ADDR,// 1  15007300~15007300
    CAM_DMA_IMGO,        // 7    15007304~1500731C  //remove base_addr set by CQ0C
    CAM_DMA_RRZO,        // 7    15007324~1500733C  //remove base_addr set by CQ0C
    CAM_DMA_LCSO,        // 7    15007340~15007358
    CAM_DMA_EISO,        // 2    1500735C~15007360
    CAM_DMA_AFO,         // 2    15007364~15007368
    CAM_DMA_ESFKO,       // 7    1500736C~15007384
    CAM_DMA_AAO,         // 7    15007388~150073A0
    CAM_DMA_VIP3I,       // 8    150073A4~150073C0
    CAM_DMA_UFEO,        // 7    150073C4~150073DC
    CAM_DMA_MFBO,        // 8    150073E0~150073FC
    CAM_DMA_IMG3BO,      // 8    15007400~1500741C
    CAM_DMA_IMG3CO,      // 8    15007420~1500743C
    CAM_DMA_IMG2O,       // 8    15007440~1500745C
    CAM_DMA_IMG3O,       // 8    15007460~1500747C
    CAM_DMA_FEO,         // 7    15007480~15007498
    CAM_DMA_BPCI_D,      // 7    1500749C~150074B4
    CAM_DMA_LSCI_D,      // 5    150074B8~150074C8
    CAM_DMA_IMGO_D,      // 7    150074D8~150074F0  //remove base_addr set by CQ0C
    CAM_DMA_RRZO_D,      // 7    150074F8~15007510  //remove base_addr set by CQ0C
    CAM_DMA_LCSO_D,      // 7    15007514~1500752C
    CAM_DMA_AFO_D,       // 7    15007530~15007548
    CAM_DMA_AAO_D,       // 7    1500754C~15007564
    CAM_DMA_IMGO_SV,     // 7    1500920C~15009224 //remove base_addr set by CQ0C
    CAM_DMA_IMGO_SV_D,   // 7    1500922C~15009244 //remove base_addr set by CQ0C
    CAM_P1_MAGIC_NUM,    // 1    150075DC
    CAM_P1_RRZ_CROP_IN,  // 1    150075E0
    CAM_P1_MAGIC_NUM_D,  // 1    150075E4
    CAM_P1_RRZ_CROP_IN_D,// 1    150075E8
    CAM_P1_RRZ_OUT_W,    // 1    15004094
    CAM_P1_RRZ_OUT_W_D,  // 1    1500409C
    CAM_RESERVED_00,
    CAM_DUMMY_,
    CAM_MODULE_MAX

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
    ISP_DRV_IRQ_TYPE_INT_P1_ST,        //P1
    ISP_DRV_IRQ_TYPE_INT_P1_ST2,    //P1_DMA
    ISP_DRV_IRQ_TYPE_INT_P1_ST_D,    //P1_D
    ISP_DRV_IRQ_TYPE_INT_P1_ST2_D,    //P1_DMA_D
    ISP_DRV_IRQ_TYPE_INT_P2_ST,        //P2
    ISP_DRV_IRQ_TYPE_INT_STATUSX,    //STATUSX, P1 AND P2
    ISP_DRV_IRQ_TYPE_INT_STATUS2X,    //STATUS2X, P1_D
    ISP_DRV_IRQ_TYPE_INT_STATUS3X,    //STATUS3X, ALL DMA
    ISP_DRV_IRQ_TYPE_INT_SENINF1,
    ISP_DRV_IRQ_TYPE_INT_SENINF2,
    ISP_DRV_IRQ_TYPE_INT_SENINF3,
    ISP_DRV_IRQ_TYPE_INT_SENINF4,
    ISP_DRV_IRQ_TYPE_INT_CAMSV,
    ISP_DRV_IRQ_TYPE_INT_CAMSV2,
    ISP_DRV_IRQ_TYPE_AMOUNT
}ISP_DRV_IRQ_TYPE_ENUM;



typedef enum
{
    ISP_DRV_TURNING_UPDATE_TYPE_TOP_ONLY,
    ISP_DRV_TURNING_UPDATE_TYPE_ENGINE_ONLY,
    ISP_DRV_TURNING_UPDATE_TYPE_TOP_ENGINE,
    ISP_DRV_TURNING_UPDATE_TYPE_NUM
}ISP_DRV_TURNING_UPDATE_TYPE_ENUM;


typedef enum
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

typedef struct ISP_DRV_IRQ_USER_STRUCT
{
    ISP_DRV_IRQ_USER_STRUCT()
    :Type(ISP_DRV_IRQ_TYPE_INT_P1_ST),Status(0x0),UserKey(-1)
    {}
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    unsigned int                          Status;
    int                                           UserKey;                     /* user key for doing interrupt operation */
}ISP_DRV_IRQ_USER_STRUCT;

typedef enum    //special user for specific operation
{
    ISP_DRV_WAITIRQ_SPEUSER_NONE = 0,
    ISP_DRV_WAITIRQ_SPEUSER_EIS = 1,
    ISP_DRV_WAITIRQ_SPEUSER_NUM
}ISP_DRV_WAITIRQ_SPEUSER_ENUM;

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
    :Clear(ISP_DRV_IRQ_CLEAR_NONE),Type(ISP_DRV_IRQ_TYPE_INT_P1_ST),Status(0x0),UserNumber(-1),Timeout(1000),UserName(""),irq_TStamp(0x0),bDumpReg(0xfe),SpecUser(ISP_DRV_WAITIRQ_SPEUSER_NONE)
    {}
    ISP_DRV_IRQ_CLEAR_ENUM  Clear;          //v1 & v3
    ISP_DRV_IRQ_TYPE_ENUM   Type;           //v1 only
    MUINT32                 Status;                       //v1 only
    MINT32                  UserNumber;             //v1 only
    MUINT32                 Timeout;                  //v1 & v3
    char*                       UserName;   //no use
    MUINT32                    irq_TStamp;   //v1 & v3
    MUINT32                 bDumpReg;//if this value = 0xfe, no dump, //v1 & v3
    ISP_DRV_IRQ_USER_STRUCT  UserInfo;  //v3 only
    ISP_DRV_IRQ_TIME_STRUCT  TimeInfo;  //v3 only
    ISP_DRV_EIS_META_STRUCT EisMeta;    //v1&v3
    ISP_DRV_WAITIRQ_SPEUSER_ENUM SpecUser;
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
    ISP_DRV_IRQ_USER_ENUM   UserNumber;
    MUINT32                 Status; //Output
}ISP_DRV_READ_IRQ_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    ISP_DRV_IRQ_USER_ENUM   UserNumber;
    MUINT32                 Status; //Input
}ISP_DRV_CHECK_IRQ_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    ISP_DRV_IRQ_USER_ENUM   UserNumber;
    MUINT32                 Status; //Input
}ISP_DRV_CLEAR_IRQ_STRUCT;

typedef struct
{
    MUINT32     Addr;
    MUINT32     Data;
}ISP_DRV_REG_IO_STRUCT;

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

enum EIspTuningMgrFuncBit  // need mapping to ETuningMgrFunc / gIspTuningFuncBitMapp
{
    eIspTuningMgrFuncBit_Obc            = 0,
    eIspTuningMgrFuncBit_Obc_d          = 1,
    eIspTuningMgrFuncBit_Lsc            = 2,
    eIspTuningMgrFuncBit_Lsc_d          = 3,
    eIspTuningMgrFuncBit_Bnr            = 4,
    eIspTuningMgrFuncBit_Bnr_d          = 5,
    eIspTuningMgrFuncBit_Rpg            = 6,
    eIspTuningMgrFuncBit_Rpg_d          = 7,
    eIspTuningMgrFuncBit_Rmg            = 8,
    eIspTuningMgrFuncBit_Rmg_d          = 9,
    eIspTuningMgrFuncBit_Pgn            = 10,
    eIspTuningMgrFuncBit_Sl2            = 11,
    eIspTuningMgrFuncBit_Cfa            = 12,
    eIspTuningMgrFuncBit_Ccl            = 13,
    eIspTuningMgrFuncBit_G2g            = 14,
    eIspTuningMgrFuncBit_Ggm_Rb         = 15,
    eIspTuningMgrFuncBit_Ggm_G          = 16,
    eIspTuningMgrFuncBit_Mfb_TuneCon    = 17,
    eIspTuningMgrFuncBit_G2c_Conv       = 18,
    eIspTuningMgrFuncBit_G2c_Shade      = 19,
    eIspTuningMgrFuncBit_Nbc            = 20,
    eIspTuningMgrFuncBit_Pca_Tbl        = 21,
    eIspTuningMgrFuncBit_Pca_Con        = 22,
    eIspTuningMgrFuncBit_Seee           = 23,
    eIspTuningMgrFuncBit_Lce            = 24,
    eIspTuningMgrFuncBit_Nr3d_Blending  = 25,
    eIspTuningMgrFuncBit_Nr3d_Lmt       = 26,
    eIspTuningMgrFuncBit_Sl2b           = 27,
    eIspTuningMgrFuncBit_Sl2c           = 28,
    eIspTuningMgrFuncBit_Mix3           = 29,
    eIspTuningMgrFuncBit_ImgSel         = 30,
    eIspTuningMgrFuncBit_ImgSel_d       = 31,
    eIspTuningMgrFuncBit_Num   //can't over 32
};

enum EIspTuningMgrFunc  // need mapping to ETuningMgrFunc
{
   eIspTuningMgrFunc_Null           = 0x00000000,
   eIspTuningMgrFunc_Obc            = (1<<eIspTuningMgrFuncBit_Obc       ),
   eIspTuningMgrFunc_Obc_d          = (1<<eIspTuningMgrFuncBit_Obc_d     ),
   eIspTuningMgrFunc_Lsc            = (1<<eIspTuningMgrFuncBit_Lsc       ),
   eIspTuningMgrFunc_Lsc_d          = (1<<eIspTuningMgrFuncBit_Lsc_d     ),
   eIspTuningMgrFunc_Bnr            = (1<<eIspTuningMgrFuncBit_Bnr       ),
   eIspTuningMgrFunc_Bnr_d          = (1<<eIspTuningMgrFuncBit_Bnr_d     ),
   eIspTuningMgrFunc_Rpg            = (1<<eIspTuningMgrFuncBit_Rpg       ),
   eIspTuningMgrFunc_Rpg_d          = (1<<eIspTuningMgrFuncBit_Rpg_d     ),
   eIspTuningMgrFunc_Rmg            = (1<<eIspTuningMgrFuncBit_Rmg       ),
   eIspTuningMgrFunc_Rmg_d          = (1<<eIspTuningMgrFuncBit_Rmg_d     ),
   eIspTuningMgrFunc_Pgn            = (1<<eIspTuningMgrFuncBit_Pgn       ),
   eIspTuningMgrFunc_Sl2            = (1<<eIspTuningMgrFuncBit_Sl2       ),
   eIspTuningMgrFunc_Cfa            = (1<<eIspTuningMgrFuncBit_Cfa       ),
   eIspTuningMgrFunc_Ccl            = (1<<eIspTuningMgrFuncBit_Ccl       ),
   eIspTuningMgrFunc_G2g            = (1<<eIspTuningMgrFuncBit_G2g       ),
   eIspTuningMgrFunc_Ggm_Rb         = (1<<eIspTuningMgrFuncBit_Ggm_Rb    ),
   eIspTuningMgrFunc_Ggm_G          = (1<<eIspTuningMgrFuncBit_Ggm_G     ),
   eIspTuningMgrFunc_Mfb_TuneCon    = (1<<eIspTuningMgrFuncBit_Mfb_TuneCon),
   eIspTuningMgrFunc_G2c_Conv       = (1<<eIspTuningMgrFuncBit_G2c_Conv  ),
   eIspTuningMgrFunc_G2c_Shade      = (1<<eIspTuningMgrFuncBit_G2c_Shade ),
   eIspTuningMgrFunc_Nbc            = (1<<eIspTuningMgrFuncBit_Nbc       ),
   eIspTuningMgrFunc_Pca_Tbl        = (1<<eIspTuningMgrFuncBit_Pca_Tbl   ),
   eIspTuningMgrFunc_Pca_Con        = (1<<eIspTuningMgrFuncBit_Pca_Con   ),
   eIspTuningMgrFunc_Seee           = (1<<eIspTuningMgrFuncBit_Seee      ),
   eIspTuningMgrFunc_Lce            = (1<<eIspTuningMgrFuncBit_Lce       ),
   eIspTuningMgrFunc_Nr3d_Blending  = (1<<eIspTuningMgrFuncBit_Nr3d_Blending      ),
   eIspTuningMgrFunc_Nr3d_Lmt       = (1<<eIspTuningMgrFuncBit_Nr3d_Lmt      ),
   eIspTuningMgrFunc_Sl2b           = (1<<eIspTuningMgrFuncBit_Sl2b      ),
   eIspTuningMgrFunc_Sl2c           = (1<<eIspTuningMgrFuncBit_Sl2c      ),
   eIspTuningMgrFunc_Mix3           = (1<<eIspTuningMgrFuncBit_Mix3      ),
   eIspTuningMgrFunc_ImgSel         = (1<<eIspTuningMgrFuncBit_ImgSel    ),
   eIspTuningMgrFunc_ImgSel_d       = (1<<eIspTuningMgrFuncBit_ImgSel_d  ),
   eIspTuningMgrFunc_Num            = (1<<eIspTuningMgrFuncBit_Num       )
};

typedef struct{
    ISP_TPIPE_CONFIG_SL2_STRUCT sl2;
    ISP_TPIPE_CONFIG_CFA_STRUCT cfa;
    ISP_TPIPE_CONFIG_MFB_STRUCT mfb;
    ISP_TPIPE_CONFIG_G2C_STRUCT g2c;
    ISP_TPIPE_CONFIG_SL2B_STRUCT sl2b;
    ISP_TPIPE_CONFIG_NBC_STRUCT nbc;
    ISP_TPIPE_CONFIG_SL2C_STRUCT sl2c;
    ISP_TPIPE_CONFIG_SEEE_STRUCT seee;
    ISP_TPIPE_CONFIG_LCE_STRUCT lce;
    ISP_TPIPE_CONFIG_NR3D_STRUCT nr3d;
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
    MUINT32 keepReadP1CtlEnP1; //keep current cam_ctl_en_p1
    MUINT32 keepReadP1CtlEnP1Dma; //keep current cam_ctl_en_p1_dma
    MUINT32 keepReadP1CtlEnP1D; //keep current cam_ctl_en_p1_d
    MUINT32 keepReadP1CtlEnP1DDma; //keep current cam_ctl_en_p1_dma_d
    stIspTuningQueInf keepP1Que; //keep p1 buffer
    EIspTuningMgrFunc keepP1UpdateFuncBit; // p1 fucntion be updated from tuning thread by now
    //
    MUINT32* pCurReadP2TuningQue; // the queue of imageio path be used currently
    EIspTuningMgrFunc eCurReadP2UpdateFuncBit; // fucntion be updated from tuning thread
    MUINT32 keepReadP2CtlEnP2; //keep current cam_ctl_en_p2
    MUINT32 keepReadP2CtlEnP2Dma; //keep current cam_ctl_en_p2_dma
    MUINT32 keepReadP2CtlEnP1; //keep current cam_ctl_en_p1 for ip scenario
    MUINT32 keepReadP2CtlEnP1Dma; //keep current cam_ctl_en_p1_dma for ip scenario
    MUINT32 capReadP2CtlEnP2; //current cam_ctl_en_p2 for capture
    MUINT32 capReadP2CtlEnP2Dma; //current cam_ctl_en_p2_dma for capture
    MUINT32 capReadP2CtlEnP1; //current cam_ctl_en_p1 for ip scenario for capture
    MUINT32 capReadP2CtlEnP1Dma; //current cam_ctl_en_p1_dma for ip scenario for capture
    stIspTuningQueInf keepP2PreviewQue; //keep the p2 preview buffer
    EIspTuningMgrFunc keepP2PreviewUpdateFuncBit; // p2 fucntion be updated from tuning thread by now
    //
    MUINT32* pCurWriteTuningQue; // the queue of featureio path be used currently
    //
    MINT32 curWriteIdx;
    MINT32 curReadP1Idx;
    MINT32 curReadP2Idx;
    MINT32 curPreviewApplyP2Idx;
    MBOOL  isInitP1;
    //MBOOL  isInitP2;
    MINT32 isCheckP1ReadIdx;
}stIspTuningQueIdx;

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


////struct for p2 debug dump data
class IspDumpDbgLogP2Package{
public:
    unsigned int *tpipeTableVa;
    unsigned int tpipeTablePa;
};


typedef MINT32 (*CALLBACKFUNC_ISP_DRV)(MINT32 cqNum, MVOID *user);

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
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
        IspDrv(){};
        virtual ~IspDrv() {};
    //
    public:
        //static IspDrv*  createInstance(ISP_DRV_INSTANCE_ENUM Instance);//js_test remove later
        static IspDrv*  createInstance();
        virtual void    destroyInstance(void) = 0;
        virtual MINT32 isp_fd_open(const char* userName="") = 0;
        virtual MINT32 isp_fd_close(MINT32 mIspFd) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;
        virtual MBOOL   waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MINT32   registerIrq(const char* userName) = 0;
        virtual MBOOL   markIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo) = 0;
        virtual MBOOL   flushIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo) = 0;
        virtual MBOOL   queryirqtimeinfo(ISP_DRV_WAIT_IRQ_STRUCT* Irqinfo) = 0;
        virtual MBOOL   readIrq(ISP_DRV_READ_IRQ_STRUCT* pReadIrq) = 0;
        virtual MBOOL   checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq) = 0;
        virtual MBOOL   clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq) = 0;
    virtual MBOOL   unregisterIrq(ISP_DRV_WAIT_IRQ_STRUCT WaitIrq) = 0;
        virtual MBOOL   reset(MINT32 rstpath) = 0;
        virtual MBOOL   resetBuf(void) = 0;
    virtual MBOOL   checkCQBufAllocated(void) = 0;
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
            unsigned long     Data,
            MINT32      userEnum=ISP_DRV_USER_OTHER,
            MINT32      caller=0) = 0;
        //
        virtual MBOOL   holdReg(MBOOL En) = 0;
        virtual MBOOL   dumpReg(void) = 0;
        virtual MBOOL   checkTopReg(MUINT32 Addr);
        virtual isp_reg_t*  getCurHWRegValues()=0;
        virtual MUINT32*  getRegAddr(void) = 0;
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
        virtual MBOOL   ISPWakeLockCtrl(MBOOL WakeLockEn) = 0;

        // load default setting
        virtual MBOOL loadInitSetting(void) = 0;
        // debug information
        virtual MBOOL dumpCQTable(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx=0, MUINT32 dupCqIdx=0, ISP_DRV_CQ_CMD_DESC_STRUCT* cqDesVa=0, MUINT32* cqVirVa=0) = 0;
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
        virtual MUINT32 getTuningUpdateFuncBit(ETuningQueAccessPath ePath, MINT32 magicNum, MINT32 cq,MUINT32 drvScenario) = 0;
        virtual MUINT32* getTuningBuf(ETuningQueAccessPath ePath, MINT32 cq) = 0;
        virtual MUINT32 getTuningTop(ETuningQueAccessPath ePath, ETuningTopEn top, MINT32 cq, MUINT32 magicNum=0) = 0;
        virtual MBOOL   getCqInfoFromScenario(ESoftwareScenario softScenario, ISP_DRV_CQ_ENUM &cq) = 0;
        virtual MBOOL   getP2cqInfoFromScenario(ESoftwareScenario softScenario, ISP_DRV_P2_CQ_ENUM &p2Cq) = 0;
        virtual MBOOL   getTuningTpipeFiled(ISP_DRV_P2_CQ_ENUM p2Cq, MUINT32* pTuningBuf, stIspTuningTpipeFieldInf &pTuningField) = 0;
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
    //
    public:
        MINT32          mFd;

        //for commandQ
        static MINT32          mIspVirRegFd;
        static MUINT32*        mpIspVirRegBufferBaseAddr;
        static MUINT32         mIspVirRegSize;
        static MUINT32**        mpIspVirRegAddrVA;
        static MUINT32**        mpIspVirRegAddrPA;
        static MBOOL           mbBufferAllocated;
        //CQ descriptor buffer
        static MINT32          mIspCQDescFd;
        static MUINT32*        mpIspCQDescBufferVirt;
        static MUINT32         mIspCQDescSize;
        static MUINT32*        mpIspCQDescBufferPhy;
        // slow motion feature, support burst Queue control
        static MINT32          mCurBurstQNum;
        static MINT32          mTotalCQNum;
        // for turning
        static stIspTuningQueInf mTuningQueInf[ISP_DRV_P2_CQ_NUM][ISP_TUNING_QUEUE_NUM];
        static stIspTuningQueIdx mTuningQueIdx[ISP_DRV_P2_CQ_NUM];
        //
        //share same member
        static ISP_DRV_CQ_CMD_DESC_STRUCT **mpIspCQDescriptorVirt;
        static MUINT32**           mpIspCQDescriptorPhy;
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
//----------------------------------------------------------------------------
#endif  // _ISP_DRV_H_

