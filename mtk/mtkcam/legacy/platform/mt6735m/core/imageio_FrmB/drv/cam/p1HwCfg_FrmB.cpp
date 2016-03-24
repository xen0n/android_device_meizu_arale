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
#define LOG_TAG "iio/p1HwCfg_FrmB"

//#include "mtkcam/imageio/p1HwCfg.h"
#include "isp_function_FrmB.h"
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

using namespace NSDrvCam_FrmB;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

/**************************************************************************
 *                      BODY                       *
 **************************************************************************/
#define HWRWCTRL_INIT_PATTERN 0x5A5A5A5A
#define HWRWCTRL_DBG_DUMP     0
#define HWRWCTRL_DBG_T_CHK        1
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
        case CAM_ISP_AE:
        case CAM_ISP_AWB:
        case CAM_ISP_FLK:
        case CAM_ISP_AF:
        case CAM_DMA_LSCI:
        case CAM_DMA_AAO:
        case CAM_DMA_ESFKO:
        case CAM_DMA_AFO:
        case CAM_DMA_EISO:
        case CAM_ISP_SGG:
   //     case CAM_ISP_RPG:
//        case CAM_ISP_SGG1:
   //     case CAM_ISP_SGG2:
   //     case CAM_DMA_BPCI:
   //     case CAM_DMA_LCSO:
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


    m_hwAccessObj[CAM_TOP_CTL_01] = new ObjAccessHW(this, CAM_TOP_CTL_01);
//    m_hwAccessObj[CAM_TOP_CTL_SEL_P1] = new ObjAccessHW(this, CAM_TOP_CTL_SEL_P1);
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
    m_hwAccessObj[CAM_TOP_CTL_01]->initModuleRegVal();
    m_hwAccessObj[CAM_TOP_CTL_01]->setCfgStatus(HWCFG_STATUS_NONE);
//    m_hwAccessObj[CAM_TOP_CTL_SEL_P1]->initModuleRegVal();
//    m_hwAccessObj[CAM_TOP_CTL_SEL_P1]->setCfgStatus(HWCFG_STATUS_NONE);

#if defined(__SUPPORT_RETRY_FLUSH2CQ__)
      m_vUpdatedHwMod.push_back(m_hwAccessObj[CAM_TOP_CTL_01]);
//      m_vUpdatedHwMod.push_back(m_hwAccessObj[CAM_TOP_CTL_EN_P1]);
#endif

    m_Init = MTRUE;
    AAO_TIMING_CHK[0] = AAO_TIMING_CHK[1] = 0;
    P1HWRW_DBG("HwRWCtrl::init()-");

    return MTRUE;
}




MINT32
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
   //P1HWRW_DBG("handle=0x%x, Caller=%s", handle, szCallerName);

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
            if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, AA_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, AA_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, AA_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, AA_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
            break;
        case CAM_ISP_AWB:
            if (en){
                m_AAUser |= HWRWCTRL_AA_AWB;
            }
            else{
                m_AAUser &= (~HWRWCTRL_AA_AWB);
            }
            en = m_AAUser ? MTRUE: MFALSE;
            if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, AA_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, AA_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, AA_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, AA_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
             break;
        case CAM_ISP_FLK:
            if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, FLK_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, FLK_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, FLK_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, FLK_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
             break;
        case CAM_ISP_AF:
            if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, AF_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, AF_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, AF_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, AF_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
             break;

        case CAM_DMA_AAO:
             if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, AAO_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, AAO_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, AAO_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, AAO_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
             break;
        case CAM_DMA_AFO:
            if (en){
                m_EFSKOUser |= HWRWCTRL_EFSKO_AFO;
            }
            else{
                m_EFSKOUser &= (~HWRWCTRL_EFSKO_AFO);
            }
            en = m_EFSKOUser ? MTRUE: MFALSE;
            if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, ESFKO_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, ESFKO_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
            break;
        case CAM_DMA_ESFKO:
            if (en){
                m_EFSKOUser |= HWRWCTRL_EFSKO_AFO;
            }
            else{
                m_EFSKOUser &= (~HWRWCTRL_EFSKO_AFO);
            }
            en = m_EFSKOUser ? MTRUE: MFALSE;
            if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, ESFKO_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, ESFKO_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
            break;
        case CAM_DMA_EISO:
            if (en){
                m_EFSKOUser |= HWRWCTRL_EFSKO_EISO;
            }
            else{
                m_EFSKOUser &= (~HWRWCTRL_EFSKO_EISO);
            }
            en = m_EFSKOUser ? MTRUE: MFALSE;
            if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, ESFKO_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_DMA_EN_CLR, ESFKO_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
            break;
        case CAM_ISP_SGG:
             if(en){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, SGG_EN_SET, 1, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, SGG_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_SET, SGG_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_EN1_CLR, SGG_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
             break;
        case CAM_ISP_OBC:
        case CAM_ISP_BNR:
        case CAM_ISP_LSC:
        case CAM_DMA_LSCI:
        default:
            P1HWRW_DBG("HwRWCtrl::SetModuleSel- NotSupportModId: %d", module);
            break;
        }

        P1HWRW_DBG("after update: EN_P1(0x%x). EN_P1_DMA(0x%x)",\
               ISP_READ_REG_NOPROTECT(_VirDrv, CAM_CTL_EN1_SET),\
               ISP_READ_REG_NOPROTECT(_VirDrv, CAM_CTL_DMA_EN_SET));
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
            if(val > 3){
                P1HWRW_ERR("error:SGG sel error(0x%x)\n",val);
                return MFALSE;
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_SET, SGG_SEL_SET, val, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_CLR, SGG_SEL_CLR, (3-val), ISP_DRV_USER_ISPF);
            }
             break;
        case HWRWCTRL_P1SEL_SGG_EN:
             if(val){
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_SET, SGG_SEL_EN_SET, 1, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_CLR, SGG_SEL_EN_CLR, 0, ISP_DRV_USER_ISPF);
             }
             else{
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_SET, SGG_SEL_EN_SET, 0, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_CLR, SGG_SEL_EN_CLR, 1, ISP_DRV_USER_ISPF);
             }
             break;
        case HWRWCTRL_P1SEL_SGG_HRZ:
            if(val){
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_SRAM_MUX_CFG_SET, SGG_HRZ_SEL_SET, 1, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_SRAM_MUX_CFG_CLR, SGG_HRZ_SEL_CLR, 0, ISP_DRV_USER_ISPF);
             }
             else{
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_SRAM_MUX_CFG_SET, SGG_HRZ_SEL_SET, 0, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_SRAM_MUX_CFG_CLR, SGG_HRZ_SEL_CLR, 1, ISP_DRV_USER_ISPF);
             }
            break;
        case HWRWCTRL_P1SEL_AA:
            if(val){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_SET, AA_SEL_SET, 1, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_CLR, AA_SEL_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_SET, AA_SEL_SET, 0, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_CLR, AA_SEL_CLR, 1, ISP_DRV_USER_ISPF);
            }
            break;
        case HWRWCTRL_P1SEL_AA_EN:
            if(val){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_SET, AA_SEL_EN_SET, 1, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_CLR, AA_SEL_EN_CLR, 0, ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_SET, AA_SEL_EN_SET, 0, ISP_DRV_USER_ISPF);
                 ISP_WRITE_BITS(_VirDrv, CAM_CTL_MUX_SEL_CLR, AA_SEL_EN_CLR, 1, ISP_DRV_USER_ISPF);
            }
            break;
        //HW bug, for 82 only!!!!!!
        case HWRWCTRL_P1SEL_EIS_DB:
            if (MFALSE == this->m_pPhyIspDrv->cqAddModule(this->m_CQ, ((this->m_DupCqIdx+i)%ISP_DRV_P1_CQ_DUPLICATION_NUM), this->m_DupCqIdx, CAM_ISP_EIS_DB))
            {
                P1HWRW_ERR("error: EIS_DB add module fail\n");
                return MFALSE;
            }
            if(val){
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_SPARE3, EIS_DB_LD_SEL, 1, ISP_DRV_USER_ISPF);//raw sensor
            }
            else{
                ISP_WRITE_BITS(_VirDrv, CAM_CTL_SPARE3, EIS_DB_LD_SEL, 0, ISP_DRV_USER_ISPF);//yuv sensor
            }
            break;
        //
        default: P1HWRW_DBG("HwRWCtrl::SetModuleSel- Not support ID: %d", module);
        }

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
            AAO_TIMING_CHK[0] = _flag[3];
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
            aao.dma_cfg.memBuf.base_pAddr = this->m_pVirtIspReg.CAM_AAO_BASE_ADDR.Raw;
            aao.dma_cfg.memBuf.ofst_addr = this->m_pVirtIspReg.CAM_AAO_OFST_ADDR.Raw;
            aao.dma_cfg.size.h = this->m_pVirtIspReg.CAM_AAO_YSIZE.Raw + 1;    //mw have do -1 operation
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
        }else if(mod == CAM_DMA_AFO){
            DMA_AFO AFO;
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
        }else if(mod == CAM_DMA_EISO){
            DMA_EISO EISO;
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
        }
#if 0

        else if(mod == CAM_DMA_LCSO) {
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
        }
#endif
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
            if(AAO_TIMING_CHK[0] != _flag[3]){
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
                    if(_ptr[j].u.token.osft_addr == 0x4388) {
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
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irq_TG_DONE;
    irq_TG_DONE.Clear = NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_WAIT;
    irq_TG_DONE.UserInfo.Type = NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
    irq_TG_DONE.UserInfo.Status = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    irq_TG_DONE.UserInfo.UserKey=0;
    irq_TG_DONE.Timeout=30000;

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


