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
#define LOG_TAG "iio/p1HwCfg"


#include "mtkcam/imageio/p1HwCfg.h"
#include "../inc/isp_function.h"
#include <cutils/properties.h>  // For property_get().

DECLARE_DBG_LOG_VARIABLE(_p1HwCfg);

// Clear previous define, use our own define.
#undef P1HWRW_VRB
#undef P1HWRW_DBG
#undef P1HWRW_INF
#undef P1HWRW_WRN
#undef P1HWRW_ERR
#undef P1HWRW_AST
#define P1HWRW_VRB(fmt, arg...)        do { if (_p1HwCfg_DbgLogEnable_VERBOSE) { BASE_LOG_VRB("{#%d:%s}: " fmt,__LINE__,"", ##arg); } } while(0)
#define P1HWRW_DBG(fmt, arg...)        do { if (_p1HwCfg_DbgLogEnable_DEBUG) { BASE_LOG_DBG("{#%d:%s}: " fmt,__LINE__,"", ##arg); } } while(0)
#define P1HWRW_INF(fmt, arg...)        do { if (_p1HwCfg_DbgLogEnable_INFO) { BASE_LOG_INF("{#%d:%s}: " fmt,__LINE__,"", ##arg); } } while(0)
#define P1HWRW_WRN(fmt, arg...)        do { if (_p1HwCfg_DbgLogEnable_WARN) { BASE_LOG_WRN("{#%d:%s}: " fmt,__LINE__,"", ##arg); } } while(0)
#define P1HWRW_ERR(fmt, arg...)        do { if (_p1HwCfg_DbgLogEnable_ERROR) { BASE_LOG_ERR("{#%d:%s}: " fmt,__LINE__,"", ##arg); } } while(0)
//#define P1HWRW_AST(cond, fmt, arg...)  do { if (1) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


//namespace NSImageio {
//namespace NSIspio {
#define DISABLE_P1HWCFG 0

/**************************************************************************
 *                      BODY                       *
 **************************************************************************/
#define HWRWCTRL_INIT_PATTERN 0x5A5A5A5A
#define HWRWCTRL_DBG_DUMP     0
#define HWRWCTRL_DBG_T_CHK      1
static MUINT32 AAO_TIMING_CHK[2]   = {0};
//static
//HwRWCtrl* pP1HwRWCtrl = NULL;

MBOOL _isValidP1Module(CAM_MODULE_ENUM module)
{

    switch ( module ) {
        //raw
        case CAM_ISP_OBC:
        case CAM_ISP_BNR:
        case CAM_ISP_LSC:
        case CAM_ISP_RPG:
        case CAM_ISP_AE:
        case CAM_ISP_AWB:
        case CAM_ISP_SGG1:
        case CAM_ISP_FLK:
        case CAM_ISP_AF:
        case CAM_ISP_SGG2:
        case CAM_DMA_BPCI:
        case CAM_DMA_LSCI:
        case CAM_DMA_AAO:
        case CAM_DMA_ESFKO:
        case CAM_DMA_AFO:
        case CAM_DMA_EISO:
        case CAM_DMA_LCSO:
        //raw_d
        case CAM_ISP_OBC_D:
        case CAM_ISP_BNR_D:
        case CAM_ISP_LSC_D:
        case CAM_ISP_RPG_D:
        case CAM_DMA_BPCI_D:
        case CAM_DMA_LSCI_D:
        case CAM_ISP_AE_D:
        case CAM_ISP_AWB_D:
        case CAM_ISP_SGG1_D:
        case CAM_ISP_AF_D:
        case CAM_DMA_AAO_D:
        case CAM_DMA_AFO_D:
        case CAM_DMA_LCSO_D:
           break;
        default:
            P1HWRW_DBG("HwRWCtrl::_isValidP1Module: Invalid Module=%d", module);
            return MFALSE;
    }
    return MTRUE;
}

#if 0
MBOOL HwRWCtrl::_syncCq0TopToHwRwCtrl(void){


}
#endif

HwRWCtrl::HwRWCtrl(void):
              m_pPhyIspDrv(NULL),
              m_pDrvShell(NULL),
              m_CQ(ISP_DRV_BASIC_CQ_NUM),
              waitReFlushedMdlCnt(0),
              m_DupCqIdx(0),
              m_BurstQNum(0),
              m_BNRUser(0),
              m_EFSKOUser(0),
              m_AAUser(0),
              m_BNR_DUser(0),
              m_AA_DUser(0)
{
    //P1HWRW_DBG("HwRWCtrl::HwRWCtrl()+");
    Lock();
    for (MUINT32 i = 0; i < CAM_MODULE_MAX; i++) {
       m_hwAccessObj[i] = NULL;
    }


    m_hwAccessObj[CAM_TOP_CTL_EN_P1] = new ObjAccessHW(this, CAM_TOP_CTL_EN_P1);
    m_hwAccessObj[CAM_TOP_CTL_SEL_P1] = new ObjAccessHW(this, CAM_TOP_CTL_SEL_P1);
    m_hwAccessObj[CAM_TOP_CTL_EN_P1_D] = new ObjAccessHW(this, CAM_TOP_CTL_EN_P1);
    m_hwAccessObj[CAM_TOP_CTL_SEL_P1_D] = new ObjAccessHW(this, CAM_TOP_CTL_SEL_P1);

    m_Init = MFALSE;
    UnLock();
    //P1HWRW_DBG("HwRWCtrl::HwRWCtrl(): 0x%8x-", this);
}

HwRWCtrl::~HwRWCtrl(void)
{
#if defined(__SUPPORT_RETRY_FLUSH2CQ__)
    m_vUpdatedHwMod.clear();
#endif

    Mutex::Autolock lock(mLock);
    for (MUINT32 i = 0; i < CAM_MODULE_MAX; i++)
    {
        if (NULL != m_hwAccessObj[i])
           {
              P1HWRW_DBG("[0x%8x]delete m_hwAccessObj[%d]=0x%8x",this, i, m_hwAccessObj[i]);
              delete m_hwAccessObj[i];
           }
    }
    P1HWRW_DBG("HwRWCtrl::~HwRWCtrl()-");
    return;
}

MBOOL
HwRWCtrl::init(
   ISP_DRV_CQ_ENUM cq,
   MUINT32* pDrvShell)
{
    DBG_LOG_CONFIG(p1HwCfg, _p1HwCfg);
    if ((cq >= ISP_DRV_BASIC_CQ_NUM) || (NULL == pDrvShell))
    {
       P1HWRW_ERR("Error HwRWCtrl::init: INVALID Parameters!!!!!!!!!!!!!!! ");
       return MFALSE;
    }

    Mutex::Autolock lock(mLock);
    m_pDrvShell = pDrvShell;
    m_CQ = cq;
    m_DupCqIdx = 0;
    m_pPhyIspDrv = ((IspDrvShell*)m_pDrvShell)->getPhyIspDrv();
    m_pVirIspDrv = m_pPhyIspDrv->getCQInstance(cq);
    P1HWRW_DBG("HwRWCtrl::init: cq=%d, pPhyIspDrv=0x%x, m_pVirIspDrv=0x%x ", cq, m_pPhyIspDrv, m_pVirIspDrv);

    if (NULL == m_pVirIspDrv)
    {
      P1HWRW_ERR("Error HwRWCtrl::init: INVALID m_pVirIspDrv!!!!!!!!!!!!!!! ");
      return MFALSE;
    }

   //Raw
    m_hwAccessObj[CAM_TOP_CTL_EN_P1]->initModuleRegVal();
    m_hwAccessObj[CAM_TOP_CTL_EN_P1]->setCfgStatus(HWCFG_STATUS_NONE);
    m_hwAccessObj[CAM_TOP_CTL_SEL_P1]->initModuleRegVal();
    m_hwAccessObj[CAM_TOP_CTL_SEL_P1]->setCfgStatus(HWCFG_STATUS_NONE);

   //RawD
     m_hwAccessObj[CAM_TOP_CTL_EN_P1_D]->initModuleRegVal();
     m_hwAccessObj[CAM_TOP_CTL_EN_P1_D]->setCfgStatus(HWCFG_STATUS_NONE);
     m_hwAccessObj[CAM_TOP_CTL_SEL_P1_D]->initModuleRegVal();
     m_hwAccessObj[CAM_TOP_CTL_SEL_P1_D]->setCfgStatus(HWCFG_STATUS_NONE);


#if defined(__SUPPORT_RETRY_FLUSH2CQ__)
      m_vUpdatedHwMod.push_back(m_hwAccessObj[CAM_TOP_CTL_EN_P1]);
      m_vUpdatedHwMod.push_back(m_hwAccessObj[CAM_TOP_CTL_EN_P1]);
#endif

    m_Init = MTRUE;
    AAO_TIMING_CHK[0] = AAO_TIMING_CHK[1] = 0;
    P1HWRW_DBG("HwRWCtrl::init()-");

    return MTRUE;
}




MINTPTR
HwRWCtrl::getModuleHandle(
    CAM_MODULE_ENUM module,
    char const* szCallerName)
{
   //P1HWRW_DBG("+; %s,m_CQ(%d), Module=%d", szCallerName, m_CQ, module);
   ObjAccessHW* handle = NULL;

    //Lock();
    //
    Mutex::Autolock lock(mLock);
    //P1HWRW_DBG("after lock");

    if (!m_Init) {
        P1HWRW_ERR("%s:HwRwCtrl NotReady, Call Camio->cfgPipe first", szCallerName);
        return NULL;
    }

    //P1HWRW_DBG("bef check variables");

   if ((!_isValidP1Module(module)) || (NULL == m_pPhyIspDrv))
   {
        P1HWRW_ERR("%s:NotReady, m_CQ(%d), m_pPhyIspDrv=0x%x, m(%d)", szCallerName, m_CQ, m_pPhyIspDrv, module);
        //UnLock();
       return NULL;
   }

    //P1HWRW_DBG("bef new");

    if (module < CAM_MODULE_MAX) {

        if (NULL == m_hwAccessObj[module]) {
            P1HWRW_DBG("new m_hwAccessObj");

            m_hwAccessObj[module] = new ObjAccessHW(this, module);
            handle = m_hwAccessObj[module];
            handle->initModuleRegVal();
        }
    }
    handle = m_hwAccessObj[module];

    //Module Critical Section: From GetHandle~ReleaseHandle
    handle->Lock();

    if (HWCFG_STATUS_NONE == handle->getCfgStatus())
    {
#if defined(__SUPPORT_RETRY_FLUSH2CQ__)
        m_vUpdatedHwMod.push_back(handle);
#endif
    }
    else if (HWCFG_STATUS_UPDATED == handle->getCfgStatus())
    {
        //...the previous setting have not updated to CQ, but someone try to overwrite these setting....
        //...Print warning message here...
        P1HWRW_DBG("the previous setting have not updated to CQ, but someone try to overwrite these ");
    }
    else
    {
    }
    handle->setCfgStatus(HWCFG_STATUS_UPDATING);

    //UnLock();

    //P1HWRW_DBG("-");
    return ((MINTPTR)handle);
}



MBOOL
HwRWCtrl::releaseModuleHandle(MINTPTR handle, char const* szCallerName)
{
   P1HWRW_DBG("handle=0x%lx, Caller=%s", handle, szCallerName);

   if (0 == handle)
   {
       return MTRUE;
   }

   ObjAccessHW* accessObj = (ObjAccessHW*)handle;
   accessObj->setCfgStatus(HWCFG_STATUS_FLUSH_DONE);
   //Release Lock here
   accessObj->UnLock();
   //P1HWRW_DBG("-");
   return MTRUE;
}



MBOOL
HwRWCtrl::setModuleEn(CAM_MODULE_ENUM module, MBOOL en)
{
    P1HWRW_DBG("module=%d, en=%d, m_pVirIspDrv=0x%x", module, en, m_pVirIspDrv);
    IspDrv* _VirDrv = NULL;
    #define HWRWCTRL_WRITE_BIT(reg, bitField) \
    ISP_WRITE_BITS(_VirDrv, reg, bitField, en, ISP_DRV_USER_ISPF)
    Mutex::Autolock lock(mLock);
    if (!m_Init) {
        P1HWRW_ERR("HwRwCtrl NotReady, Call Camio->cfgPipe first. %d", module);
        return MFALSE;
    }

    for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
    _VirDrv = this->m_pPhyIspDrv->getCQInstance(this->m_pPhyIspDrv->getRealCQIndex(this->m_CQ,this->m_BurstQNum,((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM)));

        //if (m_hwAccessObj[module].m_status == HWCFG_STATUS_UPDATED) { }

#if (DISABLE_P1HWCFG)
        return MTRUE;
#endif

        switch (module)
        {
        case CAM_ISP_AE:
            if (en){
                m_AAUser |= HWRWCTRL_AA_AE;
            }
            else{
                m_AAUser &= (~HWRWCTRL_AA_AE);
            }
            en = m_AAUser ? MTRUE: MFALSE;
            HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1, AA_EN);
            break;
        case CAM_ISP_AWB:
            if (en){
                m_AAUser |= HWRWCTRL_AA_AWB;
            }
            else{
                m_AAUser &= (~HWRWCTRL_AA_AWB);
            }
            en = m_AAUser ? MTRUE: MFALSE;
            P1HWRW_DBG("before en AWB: en=%d", en);
            HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1, AA_EN);
             break;
        case CAM_ISP_SGG1:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1, SGG1_EN);
             break;
        case CAM_ISP_FLK:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1, FLK_EN);
             break;
        case CAM_ISP_AF:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1, AF_EN);
             break;
        case CAM_ISP_SGG2:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1, SGG2_EN);
             break;

        case CAM_DMA_AAO:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA, AAO_EN);
             break;
        case CAM_DMA_AFO:
            if (en){
                m_EFSKOUser |= HWRWCTRL_EFSKO_AFO;
            }
            else{
                m_EFSKOUser &= (~HWRWCTRL_EFSKO_AFO);
            }
            en = m_EFSKOUser ? MTRUE: MFALSE;
            HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA, ESFKO_EN);
            break;
        case CAM_DMA_ESFKO:
            if (en){
                m_EFSKOUser |= HWRWCTRL_EFSKO_AFO;
            }
            else{
                m_EFSKOUser &= (~HWRWCTRL_EFSKO_AFO);
            }
            en = m_EFSKOUser ? MTRUE: MFALSE;
            HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA, ESFKO_EN);
            break;
        case CAM_DMA_EISO:
            if (en){
                m_EFSKOUser |= HWRWCTRL_EFSKO_EISO;
            }
            else{
                m_EFSKOUser &= (~HWRWCTRL_EFSKO_EISO);
            }
            en = m_EFSKOUser ? MTRUE: MFALSE;
            HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA, ESFKO_EN);
            break;

        case CAM_DMA_LCSO:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA, LCSO_EN);
             break;

        case CAM_DMA_IMGO_SV:
            ISP_WRITE_REG(_VirDrv,CAMSV_CAMSV_CLK_EN,0x00008005,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_VirDrv,CAMSV_CAMSV_INT_EN ,0x481,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_VirDrv,CAMSV_TG_VF_CON,SINGLE_MODE,0,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_VirDrv,CAMSV_CAMSV_MODULE_EN,40000015,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_VirDrv,CAMSV_CAMSV_PAK,0x03,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_VirDrv,CAMSV_CAMSV_FMT_SEL,0x01000000, ISP_DRV_USER_ISPF);
            break;

        //raw_d
        case CAM_ISP_AE_D:
            if (en){
                m_AA_DUser |= HWRWCTRL_AA_AE;
            }
            else{
                m_AA_DUser &= (~HWRWCTRL_AA_AE);
            }
            en = m_AA_DUser ? MTRUE: MFALSE;
            HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_D, AA_D_EN);
            break;

        case CAM_ISP_AWB_D:
            if (en){
                m_AA_DUser |= HWRWCTRL_AA_AWB;
            }
            else{
                m_AA_DUser &= (~HWRWCTRL_AA_AWB);
            }
            en = m_AA_DUser ? MTRUE: MFALSE;
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_D, AA_D_EN);
             break;

        case CAM_ISP_SGG1_D:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_D, SGG1_D_EN);
             break;
        case CAM_ISP_AF_D:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_D, AF_D_EN);
             break;
        case CAM_DMA_LCSO_D:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA_D, LCSO_D_EN);
             break;
        case CAM_DMA_AFO_D:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA_D, AFO_D_EN);
             break;
        case CAM_DMA_AAO_D:
             HWRWCTRL_WRITE_BIT(CAM_CTL_EN_P1_DMA_D, AAO_D_EN);
             break;
        case CAM_ISP_OBC:
        case CAM_ISP_BNR:
        case CAM_ISP_LSC:
        case CAM_ISP_RPG:
        case CAM_CTL_IHDR:
        case CAM_DMA_BPCI:
        case CAM_DMA_LSCI:

        case CAM_ISP_OBC_D:
        case CAM_ISP_BNR_D:
        case CAM_ISP_LSC_D:
        case CAM_ISP_RPG_D:
        case CAM_DMA_BPCI_D:
        case CAM_CTL_IHDR_D:
        case CAM_DMA_LSCI_D:
        default: P1HWRW_DBG("HwRWCtrl::SetModuleSel- NotSupportModId: %d", module);
        }

#undef HWRWCTRL_WRITE_BIT(reg, bitField)

        P1HWRW_DBG("after update: EN_P1(0x%x/0x%x). EN_P1_DMA(0x%x/0x%x)",\
               ISP_READ_REG_NOPROTECT(_VirDrv, CAM_CTL_EN_P1),\
               ISP_READ_REG_NOPROTECT(_VirDrv, CAM_CTL_EN_P1_D),\
               ISP_READ_REG_NOPROTECT(_VirDrv, CAM_CTL_EN_P1_DMA),\
               ISP_READ_REG_NOPROTECT(_VirDrv, CAM_CTL_EN_P1_DMA_D));
    }
    P1HWRW_DBG("-");
    return MTRUE;
}


MBOOL
HwRWCtrl::setModuleSel(HWRWCTRL_P1SEL_MODULE module, MUINT8 val)
{
    P1HWRW_DBG("HwRWCtrl::SetModuleSel+ module=%d, val=%d, m_pVirIspDrv=0x%x", module, val, m_pVirIspDrv);

    //Only use CQ-0 to control Top-Ctrl part
    IspDrv*  _VirDrv = NULL;

#define HWRWCTRL_WRITE_BIT(reg, bitField) \
        ISP_WRITE_BITS(_VirDrv, reg, bitField, val, ISP_DRV_USER_ISPF)

    Mutex::Autolock lock(mLock);
    if (!m_Init) {
        P1HWRW_ERR("HwRwCtrl NotReady, Call Camio->cfgPipe first. %d", module);
        return MFALSE;
    }

    for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
    _VirDrv = this->m_pPhyIspDrv->getCQInstance(this->m_pPhyIspDrv->getRealCQIndex(this->m_CQ,this->m_BurstQNum,((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM)));
        //if (m_hwAccessObj[module].m_status == HWCFG_STATUS_UPDATED) { }
#if (DISABLE_P1HWCFG)
            return MTRUE;
#endif

        switch (module)
        {

        case HWRWCTRL_P1SEL_SGG:
             if (val > 2) val = 2;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1, SGG_SEL);
             break;
        case HWRWCTRL_P1SEL_SGG_EN:
             val &= 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1, SGG_SEL_EN);
             break;
        case HWRWCTRL_P1SEL_LSC:
             if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1, LCS_SEL);
             break;
        case HWRWCTRL_P1SEL_LSC_EN:
             if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1, LCS_SEL_EN);
             break;
        case HWRWCTRL_P1SEL_IMG:
            if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1, IMG_SEL);
             break;
        case HWRWCTRL_P1SEL_UFE:
            if (val > 2) val = 2;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1, UFE_SEL);
             break;

        case HWRWCTRL_P1SEL_W2G:
            if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1, W2G_SEL);
             break;

        //raw-d
        case HWRWCTRL_P1SEL_SGG_D:
             if (val > 2) val = 2;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1_D, SGG_SEL_D);
             break;
        case HWRWCTRL_P1SEL_SGG_EN_D:
             val &= 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1_D, SGG_SEL_EN_D);
             break;
        case HWRWCTRL_P1SEL_LSC_D:
             if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1_D, LCS_SEL_D);
             break;
        case HWRWCTRL_P1SEL_LSC_EN_D:
             if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1_D, LCS_SEL_EN_D);
             break;
        case HWRWCTRL_P1SEL_IMG_D:
            if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1_D, IMG_SEL_D);
             break;
        case HWRWCTRL_P1SEL_UFE_D:
            if (val > 2) val = 2;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1_D, UFE_SEL_D);
             break;

        case HWRWCTRL_P1SEL_W2G_D:
            if (val > 1) val = 1;
             HWRWCTRL_WRITE_BIT(CAM_CTL_SEL_P1_D, W2G_SEL_D);
             break;
        default: P1HWRW_DBG("HwRWCtrl::SetModuleSel- Not support ID: %d", module);
        }

#undef HWRWCTRL_WRITE_BIT(reg, bitField)

    }
    P1HWRW_DBG("HwRWCtrl::SetModuleSel-");
    return MTRUE;
}




MBOOL
HwRWCtrl::finishModuleCfg(MINTPTR handle)
{
    //P1HWRW_DBG("finishModuleCfg+ handle=0x%x", handle);
    if (NULL == handle) {
        P1HWRW_ERR("HwRWCtrl::finishModuleCfg NULL handle");
        return MFALSE;
    }


    Mutex::Autolock lock(mLock);
    if ((ISP_DRV_BASIC_CQ_NUM == m_CQ) || (NULL == m_pPhyIspDrv) || (NULL == m_pVirIspDrv)) {

        P1HWRW_ERR("_flushHwCfg2CQ: HwRWCtrl Not Ready!!!!!!!!!!!!!!!!!!1");

#if defined(__SUPPORT_RETRY_FLUSH2CQ__)
        waitReFlushedMdlCnt++;
#endif
        return MFALSE;
    }


    ObjAccessHW* accessObj = (ObjAccessHW*)handle;
    // TODO: if not P1Done ~ SOF interval, near far from P1_Done, skip this setting to next frame


    accessObj->m_status = HWCFG_STATUS_UPDATED;

    CAM_MODULE_ENUM mod = accessObj->m_Module;
    //m_pVirIspDrv became useless because of CmdQMgr.
    //this module defect is no information of last update per frame. so , it can't sync with CmdQMgr.
    //workaround solution: force to write to all CmdQ per set.
    //latent crisis: racing issue. in order to lower this crisis, write behavior need to start with current working CmdQ
    //isp_reg_t*  m_cqVirtReg = (isp_reg_t*) m_pVirIspDrv->getRegAddr();
    IspDrv* _VirDrv;

    if(HWRWCTRL_DBG_T_CHK){
        MUINT32 _flag[6] = {0};
        if (CAM_DMA_AAO== mod){
            _flag[3] = 1;
            this->m_pPhyIspDrv->dumpDBGLog(_flag,MFALSE);
            AAO_TIMING_CHK[0] = _flag[0];
            AAO_TIMING_CHK[1] = ISP_READ_REG_NOPROTECT(this->m_pPhyIspDrv, CAM_AAO_BASE_ADDR);
            P1HWRW_DBG("+current SOF = %d\n",_flag[3]);
            P1HWRW_DBG("+current aao base = 0x%x\n",AAO_TIMING_CHK[1]);
        }
    }

#ifdef _PASS1_CQ_CONTINUOUS_MODE_
    for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++)
#else
    int i=0;
#endif
    {
        _VirDrv = this->m_pPhyIspDrv->getCQInstance(this->m_pPhyIspDrv->getRealCQIndex(this->m_CQ,this->m_BurstQNum,((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM)));
        //if twin mode enable, aao fifo pri thres ctrl must be modified
        if(mod == CAM_DMA_AAO) {
            DMA_AAO aao;
            aao.isTwinMode = this->m_isTwinMode;
            aao.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_AAO_BASE_ADDR.Raw;
            aao.dma_cfg.memBuf.ofst_addr = this->m_pVirtIspReg.CAM_AAO_OFST_ADDR.Raw;
            aao.dma_cfg.size.h = this->m_pVirtIspReg.CAM_AAO_YSIZE.Raw + 1; //mw have do -1 operation
            aao.dma_cfg.size.stride = this->m_pVirtIspReg.CAM_AAO_STRIDE.Raw;
            aao.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_AAO_XSIZE.Raw + 1;//mw have do -1 operation
            aao.CQ = this->m_CQ;
            aao.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            aao.burstQueIdx = this->m_BurstQNum;
            aao.config();

            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }else if(mod == CAM_DMA_AAO_D){
            DMA_AAO_D AAO_D;
            AAO_D.isTwinMode = this->m_isTwinMode;
            AAO_D.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_AAO_D_BASE_ADDR.Raw;
            AAO_D.dma_cfg.memBuf.ofst_addr = this->m_pVirtIspReg.CAM_AAO_D_OFST_ADDR.Raw;
            AAO_D.dma_cfg.size.h = this->m_pVirtIspReg.CAM_AAO_D_YSIZE.Raw + 1;
            AAO_D.dma_cfg.size.stride = this->m_pVirtIspReg.CAM_AAO_D_STRIDE.Raw;
            AAO_D.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_AAO_D_XSIZE.Raw + 1;
            AAO_D.CQ_D = this->m_CQ;
            AAO_D.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            AAO_D.burstQueIdx = this->m_BurstQNum;
            AAO_D.config();
            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }else if(mod == CAM_DMA_AFO){
            DMA_AFO AFO;
            AFO.isTwinMode = this->m_isTwinMode;
            AFO.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_AFO_BASE_ADDR.Raw;
            AFO.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_AFO_XSIZE.Raw + 1;
            AFO.CQ = this->m_CQ;
            AFO.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            AFO.burstQueIdx = this->m_BurstQNum;
            AFO.config();
            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }else if(mod == CAM_DMA_AFO_D){
            DMA_AFO_D AFO_D;
            AFO_D.isTwinMode = this->m_isTwinMode;
            AFO_D.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_AFO_D_BASE_ADDR.Raw;
            AFO_D.dma_cfg.memBuf.ofst_addr = this->m_pVirtIspReg.CAM_AFO_D_OFST_ADDR.Raw;
            AFO_D.dma_cfg.size.h = this->m_pVirtIspReg.CAM_AFO_D_YSIZE.Raw + 1;
            AFO_D.dma_cfg.size.stride = this->m_pVirtIspReg.CAM_AFO_D_STRIDE.Raw;
            AFO_D.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_AFO_D_XSIZE.Raw + 1;
            AFO_D.CQ_D = this->m_CQ;
            AFO_D.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            AFO_D.burstQueIdx = this->m_BurstQNum;
            AFO_D.config();
            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }else if(mod == CAM_DMA_EISO){
            DMA_EISO EISO;
            EISO.isTwinMode = this->m_isTwinMode;
            EISO.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_EISO_BASE_ADDR.Raw;
            EISO.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_EISO_XSIZE.Raw + 1;
            EISO.CQ = this->m_CQ;
            EISO.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            EISO.burstQueIdx = this->m_BurstQNum;
            EISO.config();
            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }else if(mod == CAM_DMA_ESFKO) {
            DMA_ESFKO esfko;
            esfko.isTwinMode = this->m_isTwinMode;
            esfko.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_ESFKO_BASE_ADDR.Raw;
            esfko.dma_cfg.memBuf.ofst_addr = this->m_pVirtIspReg.CAM_ESFKO_OFST_ADDR.Raw;
            esfko.dma_cfg.size.h = this->m_pVirtIspReg.CAM_ESFKO_YSIZE.Raw + 1;
            esfko.dma_cfg.size.stride = this->m_pVirtIspReg.CAM_ESFKO_STRIDE.Raw;
            esfko.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_ESFKO_XSIZE.Raw + 1;
            //esfko.m_pIspDrvShell = (IspDrvShell*)this->m_pDrvShell;
            esfko.CQ = this->m_CQ;
            esfko.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            esfko.burstQueIdx = this->m_BurstQNum;
            esfko.config();

            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }else if(mod == CAM_DMA_LCSO) {
            DMA_LCSO lcso;
            lcso.isTwinMode = this->m_isTwinMode;
            lcso.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_LCSO_BASE_ADDR.Raw;
            lcso.dma_cfg.memBuf.ofst_addr = this->m_pVirtIspReg.CAM_LCSO_OFST_ADDR.Raw;
            lcso.dma_cfg.size.h = this->m_pVirtIspReg.CAM_LCSO_YSIZE.Raw + 1;
            lcso.dma_cfg.size.stride = this->m_pVirtIspReg.CAM_LCSO_STRIDE.Raw;
            lcso.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_LCSO_XSIZE.Raw + 1;
            //lcso.m_pIspDrvShell = (IspDrvShell*)this->m_pDrvShell;
            lcso.CQ = this->m_CQ;
            lcso.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            lcso.burstQueIdx = this->m_BurstQNum;
            lcso.config();

            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }else if(mod == CAM_DMA_LCSO_D) {
            DMA_LCSO_D lcso_d;
            lcso_d.isTwinMode = this->m_isTwinMode;
            lcso_d.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_LCSO_D_BASE_ADDR.Raw;
            lcso_d.dma_cfg.memBuf.ofst_addr = this->m_pVirtIspReg.CAM_LCSO_D_OFST_ADDR.Raw;
            lcso_d.dma_cfg.size.h = this->m_pVirtIspReg.CAM_LCSO_D_YSIZE.Raw + 1;
            lcso_d.dma_cfg.size.stride = this->m_pVirtIspReg.CAM_LCSO_D_STRIDE.Raw;
            lcso_d.dma_cfg.size.xsize = this->m_pVirtIspReg.CAM_LCSO_D_XSIZE.Raw + 1;
            //lcso_d.m_pIspDrvShell = (IspDrvShell*)this->m_pDrvShell;
            lcso_d.CQ_D = this->m_CQ;
            lcso_d.dupCqIdx = ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM);
            lcso_d.burstQueIdx = this->m_BurstQNum;
            lcso_d.config();

            if (MFALSE == accessObj->cqAddModule(_VirDrv, this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), mod))
            {
                P1HWRW_ERR("cqAddModule Fail !!!!!!!!!!");
                return MFALSE;
            }
        }
        else{
            //non dma setting, use org flush mechanism
            if (_flushHwCfg2CQ(accessObj,((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM),_VirDrv) < 0) {
                //error happened
                 P1HWRW_DBG("flushGroupHwsCfg2CQ+, _flushHwCfg2CQ(%d) Fail", mod);
                 return MFALSE;
            }

            if (HWRWCTRL_DBG_DUMP) {
                isp_reg_t*  cqVirtReg = (isp_reg_t*) _VirDrv->getRegAddr();//CQ reg
                MUINT32* iDstPtr = (MUINT32*) (accessObj->m_iOffset + (MUINT8*)cqVirtReg);
                MUINT32* iSrcPtr = (MUINT32*) (accessObj->m_iOffset + (MUINT8*)(&m_pVirtIspReg));

                P1HWRW_DBG("flushGroupHwsCfg2CQ: cqVirtReg=0x%x, Offset=0x%x, ptr=0x%x, size=%d", cqVirtReg, accessObj->m_iOffset, iDstPtr, accessObj->m_iSize);
                if(1){
                    if (CAM_ISP_OBC == mod) {
                        P1HWRW_DBG("==============OBC Dbg=================");
                    } else if (CAM_ISP_AWB == mod){
                        P1HWRW_DBG("==============AWB Dbg=================");
                    } else if (CAM_DMA_AAO== mod){
                        P1HWRW_DBG("==============AAO Dbg=================");
                    }

                    for (MINT32 i = 0; i < (accessObj->m_iSize); i++, iDstPtr++, iSrcPtr++)
                    {
                        P1HWRW_DBG("[%d]: [src/dst]=[0x%x/0x%x]", i,(*iDstPtr), (*iSrcPtr));
                    }
                }
            }
        }
        //reFlushPendingCfgToCQ();
    }

    if(HWRWCTRL_DBG_T_CHK){
        MUINT32 _flag[6] = {0};
        if (CAM_DMA_AAO== mod){
            MBOOL _match = MFALSE;
            _flag[3] = 1;
            this->m_pPhyIspDrv->dumpDBGLog(_flag,MFALSE);
            if(AAO_TIMING_CHK[0] != _flag[0]){
                P1HWRW_INF("warning:aao cfg duration over 1 sof (%d,%d)\n",AAO_TIMING_CHK[0],_flag[3]);
            }
            if(AAO_TIMING_CHK[1] != ISP_READ_REG_NOPROTECT(this->m_pPhyIspDrv, CAM_AAO_BASE_ADDR)){
                P1HWRW_INF("warning:cfg right on Done (%d,%d)\n",AAO_TIMING_CHK[0],_flag[3]);
            }


            P1HWRW_DBG("-current SOF = %d\n",_flag[0]);
            P1HWRW_DBG("-current aao base = 0x%x\n",ISP_READ_REG_NOPROTECT(this->m_pPhyIspDrv, CAM_AAO_BASE_ADDR));
            P1HWRW_DBG("-current cq base = 0x%x\n",ISP_READ_REG_NOPROTECT(this->m_pPhyIspDrv, CAM_CTL_CUR_CQ0_BASEADDR));
            ISP_DRV_CQ_CMD_DESC_STRUCT *_ptr;
            for(int i=0;i<3;i++){
                _match = MFALSE;
                _VirDrv = this->m_pPhyIspDrv->getCQInstance(this->m_pPhyIspDrv->getRealCQIndex(this->m_CQ,this->m_BurstQNum,((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM)));
                _ptr = (ISP_DRV_CQ_CMD_DESC_STRUCT *)this->m_pPhyIspDrv->getCQDescBufVirAddr(this->m_CQ,this->m_BurstQNum,((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM));
                for(int j=0;j<=CAM_MODULE_MAX;j++){
                    if(_ptr[j].u.token.osft_addr == 0x7388) {
                        _match = MTRUE;
                        break;
                    }
                }
                if(_match == MFALSE){
                    P1HWRW_DBG("NO AAO descriptor\n");
                }
                P1HWRW_DBG("-target aao base = 0x%x(%d,0x%x)\n",ISP_READ_REG_NOPROTECT(_VirDrv, CAM_AAO_BASE_ADDR),this->m_DupCqIdx,_VirDrv);

            }
        }
    }
    //P1HWRW_DBG("finishModuleCfg- mod=%d",mod);
    return MTRUE;
}




MINT32
HwRWCtrl::_flushHwCfg2CQ(
   ObjAccessHW* accessObj, //Source
   MUINT32 TarDupIdx,
   IspDrv* pTarVirDrv)//Dst
{
    // 1. Copy to CmdQ
    //1.1 get Cq Virtual Reg Base
    //1.2 Copy the setting to Cq Virtual Reg

    //P1HWRW_DBG("_flushHwCfg2CQ+");

    if ((0 == accessObj->m_iSize) || ((accessObj->m_iSize*4) > sizeof(isp_reg_t)))
    {
        //error parameter
        P1HWRW_ERR("_flushHwCfg2CQ: (0 == accessObj->m_iSize(%d)) || ((accessObj->m_iSize*4)(0x%x) > sizeof(isp_reg_t))(0x%x)",accessObj->m_iSize, (accessObj->m_iSize*4), sizeof(isp_reg_t));
        return MFALSE;
    }

    //MUINT32 *SrcPtr = (MUINT32 *)(((MUINT8*)m_cqVirtReg)       + (accessObj->m_iOffset));
    //MUINT32 *DstPtr = (MUINT32 *)(((MUINT8*)(&m_pVirtIspReg))  + (accessObj->m_iOffset));
    //P1HWRW_DBG("_flushHwCfg2CQ: memCpy: dst=0x%x; src=0x%x, SrcVal[0]=", DstPtr, SrcPtr, SrcPtr[0]);
#if (DISABLE_P1HWCFG)

#else
    memcpy((((MUINT8*)pTarVirDrv->getRegAddr())       + (accessObj->m_iOffset)), \
           (((MUINT8*)(&m_pVirtIspReg))  + (accessObj->m_iOffset)), \
           (accessObj->m_iSize)*sizeof(MUINT32));

    // 2. Add Cq-Module
    if (MFALSE == accessObj->cqAddModule(pTarVirDrv, m_CQ, TarDupIdx, accessObj->m_Module))
    {
        P1HWRW_ERR("_flushHwCfg2CQ: cqAddModule Fail !!!!!!!!!!");
        return MFALSE;
    }
#endif

    accessObj->m_status = HWCFG_STATUS_FLUSH_DONE;

    //P1HWRW_DBG("_flushHwCfg2CQ-");

   return MTRUE;
}



//this api should phase out
MBOOL
HwRWCtrl::reFlushPendingCfgToCQ(void)
{
#if defined(__SUPPORT_RETRY_FLUSH2CQ__)

    MBOOL _dump_reg = MTRUE;
    ObjAccessHW* accessObj = NULL;
    isp_reg_t*  m_cqVirtReg = (isp_reg_t*) m_pVirIspDrv->getRegAddr();
    CAM_MODULE_ENUM mod ;

    P1HWRW_DBG("flushGroupHwsCfg2CQ+, UpdateDModuleCnt=%d", m_vUpdatedHwMod.size());
    for (int i=0; i < m_vUpdatedHwMod.size();i++)
    {
        //P1HWRW_DBG("flushGroupHwsCfg2CQ+, i=%d", i);
        accessObj = (ObjAccessHW*) m_vUpdatedHwMod.at(i);
        mod = accessObj->m_Module;
        if ((mod > CAM_MODULE_MAX) || (NULL == accessObj))
        {
            P1HWRW_DBG("flushGroupHwsCfg2CQ+, (mod > CAM_MODULE_MAX)||(NULL == accessObj)");
            return MFALSE;
        }

        // 2 Flush related Modules' setting to CQ
        if (accessObj->m_status == HWCFG_STATUS_UPDATED)
        {
             //P1HWRW_DBG("flushGroupHwsCfg2CQ+, m_bGroup & HWCFG_STATUS_UPDATED");
            if (_flushHwCfg2CQ(accessObj, m_cqVirtReg,0) < 0)
            {
                 //error happened
                 P1HWRW_DBG("flushGroupHwsCfg2CQ+, _flushHwCfg2CQ(%d) Fail", mod);
                 continue;
            }

             if (_dump_reg)
             {
                 isp_reg_t*  cqVirtReg = (isp_reg_t*) m_pVirIspDrv->getRegAddr();
                 MUINT32* iDstPtr = (MUINT32*) (accessObj->m_iOffset + (MUINT8*)cqVirtReg);
                 MUINT32* iSrcPtr = (MUINT32*) (accessObj->m_iOffset + (MUINT8*)(&m_pVirtIspReg));

                 P1HWRW_DBG("flushGroupHwsCfg2CQ: cqVirtReg=0x%x, Offset=0x%x, ptr=0x%x, size=%d", cqVirtReg, accessObj->m_iOffset, iPtr, accessObj->m_iSize);
                 if (CAM_ISP_OBC == mod) {
                     P1HWRW_DBG("==============OBC Dbg=================");
                 } else if (CAM_ISP_AWB == mod){
                     P1HWRW_DBG("==============AWB Dbg=================");
                 }

                 for (MINT32 i = 0; i < (accessObj->m_iSize); i++, iDstPtr++, iSrcPtr++)
                 {
                     P1HWRW_DBG("[%d]: [src/dst]=[0x%x/0x%x]", i,(*iDstPtr), (*iSrcPtr));
                 }
             }
        }
        waitReFlushedMdlCnt--;
    }

    P1HWRW_DBG("flushGroupHwsCfg2CQ-");
#endif
    return MTRUE;
}



MBOOL
HwRWCtrl::dbgDump(MINTPTR handle, MUINT32 tgIdx)
{
    MBOOL ret = MTRUE;
    MUINT32 temp;
    if (0 == handle) {
        P1HWRW_ERR("HwRWCtrl::finishModuleCfg NULL handle");
        return MFALSE;
    }

    ObjAccessHW* accessObj = (ObjAccessHW*)handle;
    ISP_DRV_WAIT_IRQ_STRUCT irq_TG_DONE;
    irq_TG_DONE.Clear =  ISP_DRV_IRQ_CLEAR_WAIT;
    irq_TG_DONE.Type= ISP_DRV_IRQ_TYPE_INT_P1_ST;
    irq_TG_DONE.Status= CAM_CTL_INT_P1_STATUS_VS1_INT_ST;
    irq_TG_DONE.UserNumber= ISP_DRV_IRQ_USER_ISPDRV;
    irq_TG_DONE.Timeout= 30000;
    irq_TG_DONE.UserName= "HwRWCtrl";
    if (2 == tgIdx) {
        irq_TG_DONE.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
        irq_TG_DONE.Status = CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
    }

    ret = this->m_pPhyIspDrv->waitIrq( &irq_TG_DONE );
    if ( MFALSE == ret ) {
        P1HWRW_DBG("dbgDump waitIrq( VS1_INT_ST ) fail");
        return MFALSE;
    }

    for (MUINT32 ii=0; ii < accessObj->m_iSize; ii++) {
        temp = accessObj->m_pHwRWCtrl->m_pPhyIspDrv->readReg((accessObj->m_iOffset + ii*4), ISP_DRV_RWREG_CALLFROM_MACRO);
        P1HWRW_DBG("HWRWInitModule(%d) [%d]=0x%x",accessObj->m_Module, ii, temp);
    }
    return ret;
}

/*********************************************************************
*       ObjAccessHW
*********************************************************************/
//#include "mtkcam/core/drv/isp/isp_drv_imp.h"

ObjAccessHW::ObjAccessHW(
     HwRWCtrl* pHwRWCtrl,
     CAM_MODULE_ENUM  Module
    )
    :m_pHwRWCtrl(pHwRWCtrl),
     m_Module(Module)
{
     m_status = HWCFG_STATUS_NONE;
}

ObjAccessHW::~ObjAccessHW(void){
    P1HWRW_DBG("~ObjAccessHW, this=0x%8x, sz=%d", this, sizeof(ObjAccessHW));

}


MBOOL
ObjAccessHW::getModuleOftAndRegSize(void)
{
    ISP_DRV_CQ_MODULE_INFO_STRUCT outInfo;

    if ((ISP_DRV_BASIC_CQ_NUM == m_pHwRWCtrl->m_CQ) ||
        (NULL == m_pHwRWCtrl->m_pPhyIspDrv) ||
        (NULL == m_pHwRWCtrl->m_pVirIspDrv)){

        P1HWRW_ERR("getModuleOftAndRegSize: HwRWCtrl Not Ready!!!!!!!!!!!!!!!!!!");
        return MFALSE;
    }

    if(MFALSE == m_pHwRWCtrl->m_pVirIspDrv->getIspCQModuleInfo(m_Module,outInfo)){
        P1HWRW_DBG("Not support module: %d\n",m_Module);
        m_iOffset = m_iSize = 0;
        return MFALSE;
    }
    else {
        m_iOffset = outInfo.addr_ofst;
        m_iSize   = outInfo.reg_num;
        if (m_iOffset > 0x10000) {
            P1HWRW_DBG("Invalid  module: %d, Offset:0x%x\n", m_Module, m_iOffset);
            m_iOffset = m_iSize = 0;
            return MFALSE;
        }
    }
    P1HWRW_DBG("module_%d: ofs:0x%x, Size:%d\n",m_Module,m_iOffset, m_iSize);
    return MTRUE;
}



void
ObjAccessHW::initModuleRegVal(void)
{
    MUINT32* pPtr = 0;

    getModuleOftAndRegSize();

    pPtr = (MUINT32*) (  ((unsigned long)&(m_pHwRWCtrl->m_pVirtIspReg))  + m_iOffset);

    if (HWCFG_STATUS_NONE == getCfgStatus()) {

        //Initialize from HW default setting
        P1HWRW_DBG("m_pVirtIspReg: 0x%x, m_iOffset=0x%x", &m_pHwRWCtrl->m_pVirtIspReg, m_iOffset);
        for (MUINT32 ii=0; ii < m_iSize; ii++) {

            *pPtr = m_pHwRWCtrl->m_pPhyIspDrv->readReg((m_iOffset + ii*4), ISP_DRV_RWREG_CALLFROM_MACRO);
            //P1HWRW_DBG("HWRWInitModule 0x%x[%d]=0x%x", pPtr,ii, *pPtr);
            pPtr++;
        }

        m_bInitFromHwReg = MTRUE;
        return;
    }

#if 0
    if (HWCFG_STATUS_UPDATED == getCfgStatus()) {

        //user have config these module, just need to init those un-init registers
        for (MUINT32 ii=0; ii < m_iSize; ii++)
        {
            if (HWRWCTRL_INIT_PATTERN == *pPtr)
            {
                 //User didn't init this HW reg, init it by reading from HW register
                 *pPtr = m_pHwRWCtrl->m_pPhyIspDrv->readReg((m_iOffset + ii), ISP_DRV_RWREG_CALLFROM_MACRO);
                 P1HWRW_DBG("HWRWInitModule [%d]=0x%x ", ii, *pPtr);
            }
            pPtr++;
        }
    }
#endif
}


#if 0
ISP_DRV_CQ_ENUM
ObjAccessHW::getCqIdxOfModule(MINT32 handle)
{
    ObjAccessHW* accessObj = (ObjAccessHW*)handle;
    if (NULL == accessObj)
    {
        P1HWRW_ERR("ObjAccessHW::getCqIdxOfModule NULL handle");
        return MFALSE;
    }

    return accessObj->m_pHwRWCtrl->m_CQ;
}
#endif


//};
//};


