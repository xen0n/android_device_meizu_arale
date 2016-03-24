#ifndef __ISP_P1_HW_CFG_H__
#define __ISP_P1_HW_CFG_H__


#include <vector>
//#include <map>
//#include <list>

//using namespace std;
//#include "imageio_types.h"    // For type definitions.
#include "mtkcam/imageio/ispio_stddef.h"
#include "imageio_log.h"
#include <mtkcam/drv_common/isp_reg.h>
#include "mtkcam/drv/isp_drv.h"
//#include <utils/Mutex.h>    // For android::Mutex.
//-----------------------------------------------------------------------------
using namespace android;

/******************************************************************************
 *
 ******************************************************************************/

//namespace NSImageio {
//namespace NSIspio   {
///////////////////////////////////////////////////////////////////////////////

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


enum HWCFG_STATUS_ENUM
{
   HWCFG_STATUS_NONE = 0,
   HWCFG_STATUS_UPDATING,
   HWCFG_STATUS_UPDATED,
   HWCFG_STATUS_FLUSH_DONE,
   HWCFG_STATUS_RSVD,
};

class HwRWCtrl;

class ObjAccessHW
{
public:
    ObjAccessHW(
         HwRWCtrl* pHwRWCtrl,
         CAM_MODULE_ENUM  Module
        );

    ~ObjAccessHW(void);

    inline void setCfgStatus(HWCFG_STATUS_ENUM status) {m_status = status;}
    HWCFG_STATUS_ENUM getCfgStatus(void) {return m_status;}

    MINT32 getModuleOftAndRegSize(void);
    void Lock(void) {mLock.lock();}
    void UnLock(void){mLock.unlock();}

    MBOOL cqAddModule(IspDrv* pVirIspDrv,MINT32 cq,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
    MBOOL cqDelModule(IspDrv* pVirIspDrv, MINT32 cq,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
    void initModuleRegVal(void);

    mutable Mutex     mLock;         // protection m_hwAccessObj & m_vUpdatedHwMod
    MUINT32           m_iOffset;
    MUINT32           m_iSize;
    MBOOL             m_bInitFromHwReg;   //init done or not
    HWCFG_STATUS_ENUM m_status; //enable: 1 /disable: 0
    CAM_MODULE_ENUM   m_Module;
    HwRWCtrl*         m_pHwRWCtrl;

};

typedef enum
{
   HWRWCTRL_BNR_BPC = 1,
   HWRWCTRL_BNR_NR1 = 2,
   HWRWCTRL_BNR_RMM = 4
}HWRWCTRL_BNR_MODULE;

typedef enum
{
   HWRWCTRL_AA_AE = 1,
   HWRWCTRL_AA_AWB = 2,
}HWRWCTRL_AA_MODULE;


typedef enum
{
   HWRWCTRL_EFSKO_AFO = 1,
   HWRWCTRL_EFSKO_EFSKO = 2,
   HWRWCTRL_EFSKO_EISO = 4,
}HWRWCTRL_EFSKO_MODULE;


typedef enum
{
   HWRWCTRL_P1SEL_SGG = 0,
   HWRWCTRL_P1SEL_SGG_EN,
   HWRWCTRL_P1SEL_LSC,
   HWRWCTRL_P1SEL_LSC_EN,
   HWRWCTRL_P1SEL_IMG,
   HWRWCTRL_P1SEL_UFE,
   HWRWCTRL_P1SEL_EIS,
   HWRWCTRL_P1SEL_W2G,

   //raw-D
   HWRWCTRL_P1SEL_SGG_D = 0xFF,
   HWRWCTRL_P1SEL_SGG_EN_D,
   HWRWCTRL_P1SEL_LSC_D,
   HWRWCTRL_P1SEL_LSC_EN_D,
   HWRWCTRL_P1SEL_IMG_D,
   HWRWCTRL_P1SEL_UFE_D,
   HWRWCTRL_P1SEL_EIS_D,
   HWRWCTRL_P1SEL_W2G_D,

   HWRWCTRL_P1SEL_RSVD = 0xFFFFFFFF,

}HWRWCTRL_P1SEL_MODULE;



class HwRWCtrl
{
public:

   HwRWCtrl(void);
   ~HwRWCtrl(void);

   MBOOL init(ISP_DRV_CQ_ENUM cq, MUINT32* pDrvShell);
   MINT32 getModuleHandle(CAM_MODULE_ENUM module, char const* szCallerName); //ISP_DRV_CQ_ENUM cq
   MBOOL releaseModuleHandle(MINTPTR handle, char const* szCallerName);
   MBOOL finishModuleCfg(MINTPTR handle);

   MBOOL setModuleEn(CAM_MODULE_ENUM module, MBOOL en);
   MBOOL setModuleSel(HWRWCTRL_P1SEL_MODULE module, MUINT8 val);
   void Lock(void) {mLock.lock();}
   void UnLock(void){mLock.unlock();}
   MBOOL dbgDump(MINTPTR handle, MUINT32 tgIdx);


protected:
   MBOOL _flushHwCfg2CQ(ObjAccessHW* accessObj, MUINT32 TarDupIdx, IspDrv* pTarVirDrv);
   MBOOL reFlushPendingCfgToCQ(void);

public:
   mutable Mutex     mLock;         //protection m_hwAccessObj & m_vUpdatedHwMod

   IspDrv*           m_pPhyIspDrv;  //used to get register offset
   IspDrv*           m_pVirIspDrv;  //used to get register offset

   isp_reg_t         m_pVirtIspReg; //used to collect the HW config from users
   MUINT32           mSensorIndex; //0/1/2
   ISP_DRV_CQ_ENUM   m_CQ;
   MUINT32           m_DupCqIdx;
   MUINT32           m_BurstQNum;
   ObjAccessHW*      m_hwAccessObj[CAM_MODULE_MAX]; //ptr point to the body of Module-Access_Block
   std::vector<ObjAccessHW*> m_vUpdatedHwMod; //ptr to ObjAccessHW*
   MINT32            waitReFlushedMdlCnt;

   MBOOL             m_Init;//indicate if this struct is initialized or not
   //const char        *userName;
   //raw
   MUINT32           m_BNRUser;
   MUINT32           m_EFSKOUser;
   MUINT32           m_AAUser;
   //rawD
   MUINT32           m_BNR_DUser;
   MUINT32           m_AA_DUser;
   MUINT32           m_isTwinMode;
private:
   MUINT32*          m_pDrvShell;   //iopipe drv shell
};

//};
//};

#define HWRWCTL_HANDLE2RWCTL(handle)    (  ((ObjAccessHW*)handle)->m_pHwRWCtrl  )

//single regs
#define HWRWCTL_SET_MODUL_REG(handle,RegName,Value)    \
do{                                                    \
    if (!handle){                                \
    }                                                  \
    if (HWCFG_STATUS_UPDATED == ((ObjAccessHW*)handle)->getCfgStatus())     \
    {                                                                       \
        /*...the previous setting have not updated to CQ, but someone try to overwrite these setting....*/ \
        /*...Print warning message here...*/                                \
        ((ObjAccessHW*)handle)->setCfgStatus(HWCFG_STATUS_UPDATING);        \
    }                                                                       \
    HwRWCtrl* pHwRWCtrl = ((ObjAccessHW*)handle)->m_pHwRWCtrl;\
    pHwRWCtrl->m_pVirtIspReg.RegName.Raw = Value; \
}while(0)
//    MUINT32 offset = (MUINT32) (&(pHwRWCtrl->m_pVirtIspReg.RegName)) - (MUINT32)(&(pHwRWCtrl->m_pVirtIspReg));\
//    P1HWRW_DBG("val=0x%x; RegName.Raw=0x%x; offset=0x%x\n", Value, pHwRWCtrl->m_pVirtIspReg.RegName.Raw, offset);     \


//multiple regs
#define HWRWCTL_SET_MODUL_REGS(handle, StartRegName, size, ValueArray)     \
do{                                                                        \
    if ((!handle) || (size > ((ObjAccessHW*)handle)->m_iSize) * 4)      \
    {                                                                      \
    }                                                                      \
    if (HWCFG_STATUS_UPDATED == ((ObjAccessHW*)handle)->getCfgStatus())    \
    {                                                                      \
         /*...the previous setting have not updated to CQ, but someone try to overwrite these setting....*/ \
         /*...Print warning message here...*/                              \
         ((ObjAccessHW*)handle)->setCfgStatus(HWCFG_STATUS_UPDATING);      \
    }                                                                      \
    memcpy((MUINT8*)(&((ObjAccessHW*)handle)->m_pHwRWCtrl->m_pVirtIspReg.StartRegName),(MUINT8*)ValueArray, size);\
}while(0)


#if 0
//MODIFY ME by CQ&ISP-Drv
#define HWRWCTL_READ_MODUL_REG(handle,RegName)      \
({                                                  \
  MUINT32 regV=0;                                   \
  MUINT32 cq = 0;                                   \
    if (NULL == handle)                             \
    {                                               \
    }                                               \
    cq= ((ObjAccessHW*)handle)->m_cq;               \
    regV = ISP_READ_REG_NOPROTECT(((ObjAccessHW*)handle)->m_pHwRWCtrl->m_pPhyIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)cq),RegName);\
    regV;                                           \
})
#endif


////////////////////////////
//EIS structrue
///////////////////////////
typedef struct _EIS_REG_CFG{
    MBOOL   bEIS_Bypass;
    struct{
        MUINT32 CTRL_1;
        MUINT32 CTRL_2;
        MUINT32 LMV_TH;
        MUINT32 FL_ofs;
        MUINT32 MB_ofs;
        MUINT32 MB_int;
        MUINT32 GMV;
        MUINT32 ERR_CTRL;
        MUINT32 IMG_CTRL;
    }_EIS_REG;              //oreder within this sturcutre can't change, and this structure can't add any new element, because of cmd passing
    MUINT32 EIS_SEL;
    MBOOL   bEIS_EN;
    MBOOL   bSGG_Bypass;
    MBOOL   bSGG_EN;
    MUINT32 SGG_SEL;
}EIS_REG_CFG;

typedef struct _EIS_SIZE{
    MUINT32 w;
    MUINT32 h;
}EIS_SIZE;
typedef struct _EIS_INPUT_INFO{
    EIS_SIZE    sRMXOut;
    EIS_SIZE    sHBINOut;
    EIS_SIZE    sTGOut;
    MBOOL       bYUVFmt;
    MBOOL       bIsTwin;
}EIS_INPUT_INFO;

typedef struct _LCS_INPUT_INFO{
    EIS_SIZE    sRRZOut;
    EIS_SIZE    sHBINOut;
    MBOOL       bIsHbin;
}LCS_INPUT_INFO;

typedef struct _LCS_REG_CFG{
    MBOOL   bLCS_EN;
    MBOOL   bLCS_Bypass;    // 1 if no need to update LCS or LCS_D reg
    struct{
        MUINT32 LCS_ST;
        MUINT32 LCS_AWS;
        MUINT32 LCS_LRZR_1;
        MUINT32 LCS_LRZR_2;
        MUINT32 LCS_CON;
        MUINT32 LCS_FLR;
    }_LCS_REG;
}LCS_REG_CFG;

class P1_TUNING_NOTIFY
{
    public:
    virtual ~P1_TUNING_NOTIFY(){}
    virtual const char* TuningName(void) = 0;
    virtual void p1TuningNotify(MVOID *pInput,MVOID *pOutput)= 0;

    MVOID*  m_pClassObj;
};

#endif  //__ISP_P1_HW_CFG_H__

