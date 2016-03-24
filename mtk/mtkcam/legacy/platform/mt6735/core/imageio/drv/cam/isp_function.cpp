#define LOG_TAG "iio/ifunc"

#include "isp_function.h"
#include <mtkcam/v1/config/PriorityDefs.h>
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>
//mutex
#include <pthread.h>
//thread
#include <utils/threads.h>
#include <utils/StrongPointer.h>
//tpipe
#include <mtkcam/drv/tpipe_config.h>
//
#include "mdp_mgr.h"

#include <cutils/properties.h>  // For property_get().

#include "m4u_lib.h"


//
//digital zoom
#define CAM_ISP_ZOOMRARIO_GAIN (100)


#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(function);
//EXTERN_DBG_LOG_VARIABLE(function);

// Clear previous define, use our own define.
#undef ISP_FUNC_VRB
#undef ISP_FUNC_DBG
#undef ISP_FUNC_INF
#undef ISP_FUNC_WRN
#undef ISP_FUNC_ERR
#undef ISP_FUNC_AST
#define ISP_FUNC_VRB(fmt, arg...)        do { if (function_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_FUNC_DBG(fmt, arg...)        do { if (function_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_FUNC_INF(fmt, arg...)        do { if (function_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_FUNC_WRN(fmt, arg...)        do { if (function_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_FUNC_ERR(fmt, arg...)        do { if (function_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_FUNC_AST(cond, fmt, arg...)  do { if (function_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#ifdef _rtbc_buf_que_2_0_
    static MUINT8 dma_array[_rt_dma_max_] = {0};//for fw rcnt controller
#endif

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
//        if  (0==mIdx)
//        {
//            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
//        }
//        else
//        {
            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
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




/****************************************************************************
* Global Variable
****************************************************************************/
 MdpMgr *g_pMdpMgr = NULL;
 IMEM_BUF_INFO  gMdpBufInfo;
 MDPMGR_CFG_STRUCT ***gpMdpMgrCfgData;


/****************************************************************************
*
****************************************************************************/
int cam_get_DZ_cfg(
    MINT32 scenario,
    MINT32 tpipe_En,
    MINT32 zoomRatio,
    MUINT32 const cdrz_iW, //src_img_size
    MUINT32 const cdrz_iH,
    MUINT32 const curz_oW, //max output size
    MUINT32 const curz_oH,
    MUINT32 &cdrz_oW,      //imgo
    MUINT32 &cdrz_oH,
    MUINT32 &curz_iW,      //imgi
    MUINT32 &curz_iH,
    MUINT32 &curz_iOfst
)
{
int zoomRatio_W = zoomRatio;
int zoomRatio_H = zoomRatio;

    if ( ISP_SCENARIO_IC == scenario ) {
        if (tpipe_En) {
            //CURZ NOT support tpipeMode

            //CDRZ n-tap NOT support tpipeMode
        }
        else {

        }
    }
    else if ( ISP_SCENARIO_VR == scenario ) {
        //frame mode always
        if (tpipe_En) {
            //ISP_INFO("Error conflict:VR+tpipe");
            return 1;
        }
        // 0:curz->vido , prz->dispo
        // 1:curz->dispo, prz->vido
        //ISP_BITS(g_pIspReg, CAM_CTL_SEL, DISP_VID_SEL) = 0;

        //width
        if ( ((cdrz_iW * CAM_ISP_ZOOMRARIO_GAIN )/zoomRatio_W) >= curz_oW) {
            //after zoom input >= required output
            //use cdrz only
            curz_iW = curz_oW;
            cdrz_oW = ( curz_iW*zoomRatio_W + (CAM_ISP_ZOOMRARIO_GAIN >> 1) ) / CAM_ISP_ZOOMRARIO_GAIN;
        }
        else {
            //use curz
            cdrz_oW = cdrz_iW;
            curz_iW = ( (cdrz_oW * CAM_ISP_ZOOMRARIO_GAIN) + (zoomRatio_W>>1) )/zoomRatio_W;
        }

        //height
        if ( ((cdrz_iH * CAM_ISP_ZOOMRARIO_GAIN )/zoomRatio_H) >= curz_oH) {
            curz_iH = curz_oH;
            cdrz_oH = ( curz_iH*zoomRatio_H + (CAM_ISP_ZOOMRARIO_GAIN >> 1) ) / CAM_ISP_ZOOMRARIO_GAIN;
        }
        else {
            cdrz_oH = cdrz_iH;
            curz_iH = ( (cdrz_oH * CAM_ISP_ZOOMRARIO_GAIN) + (zoomRatio_H>>1) )/zoomRatio_H;
        }

        //make even number
        cdrz_oW &= (~0x01);
        cdrz_oH &= (~0x01);
        curz_iW &= (~0x01);
        curz_iH &= (~0x01);
        curz_iOfst = cdrz_oW * ( ( cdrz_oH - curz_iH ) >> 1 ) + ( ( cdrz_oW - curz_iW ) >> 1 );
    }
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    Isp driver base
/////////////////////////////////////////////////////////////////////////////*/
//
IspDrv*         IspDrv_B::m_pPhyIspDrv = NULL;
IspDrv*         IspDrv_B::m_pP1IspDrv = NULL;
IspDrv*         IspDrv_B::m_pP1IspDrvD = NULL;
isp_reg_t*      IspDrv_B::m_pP1CurCQDIspReg;
IspDrv*         IspDrv_B::m_pP2IspDrv = NULL;




//int ISP_TOP_CTRL::pix_id;
/*** isp module default setting ***/


MINT32
IspDrv_B::
cam_isp_pgn_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){ //0x15004880 ~ 0x15004894
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_PGN){
            m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);
            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_VRB("[PGN](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
MINT32
IspDrv_B::
cam_isp_sl2_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    /*
    SL2_CENTR_X    =      (SL2_CENTR_X_LSC - crop_initial_x )*RRZ_Ratio
    SL2_CENTR_Y    =      (SL2_CENTR_Y_LSC- crop_initial_y) *RRZ_Ratio
    SL2_RR_0       =      SL2_RR_0_LSC *RRZ_Ratio*RRZ_Ratio
    SL2_RR_1       =      SL2_RR_1_LSC *RRZ_Ratio*RRZ_Ratio
    SL2_RR_2       =      SL2_RR_2_LSC *RRZ_Ratio*RRZ_Ratio
    SL2_HRZ_COMP   =      SL2_HRZ_COMP = 2048
    */
    if(imageioPackage.isApplyTuning){ //0x15004F40 ~ 0x15004F58
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        MUINT32 sl2CentrXLsc, sl2CentrYLsc, sl2Rr0Lsc, sl2Rr1Lsc, sl2Rr2Lsc;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_SL2){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_VRB("[SL2](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
            // update sl2
            // get raw sl2 data from CQ set by tuningMgr just now
            sl2CentrXLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_X);
            sl2CentrYLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_Y);
            sl2Rr0Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX0_RR, SL2_RR_0);
            sl2Rr1Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX1_RR, SL2_RR_1);
            sl2Rr2Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX2_RR, SL2_RR_2);
            // calculate the right sl2 data
            sl2CentrXLsc = (sl2CentrXLsc-tuningPackage.rrzCropX)*tuningPackage.rrzRatio;
            sl2CentrYLsc = (sl2CentrYLsc-tuningPackage.rrzCropY)*tuningPackage.rrzRatio;
            sl2Rr0Lsc = sl2Rr0Lsc*tuningPackage.rrzRatio*tuningPackage.rrzRatio;
            sl2Rr1Lsc = sl2Rr1Lsc*tuningPackage.rrzRatio*tuningPackage.rrzRatio;
            sl2Rr2Lsc = sl2Rr2Lsc*tuningPackage.rrzRatio*tuningPackage.rrzRatio;
            ISP_FUNC_DBG("rrzCropX(%d),rrzCropY(%d),rrzRatio(%f)",tuningPackage.rrzCropX,tuningPackage.rrzCropY,tuningPackage.rrzRatio);
            // set the sl2 data caculated to CQ
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_X, sl2CentrXLsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_Y, sl2CentrYLsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_MAX0_RR, SL2_RR_0, sl2Rr0Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_MAX1_RR, SL2_RR_1, sl2Rr1Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_MAX2_RR, SL2_RR_2, sl2Rr2Lsc);
            ISP_FUNC_DBG("[SL2]CentrXLsc(%d),CentrYLsc(%d),Rr0Lsc(%d),Rr1Lsc(%d),Rr2Lsc(%d)",sl2CentrXLsc,sl2CentrYLsc,sl2Rr0Lsc,sl2Rr1Lsc,sl2Rr2Lsc);
        }
    }

    return 0;
}
MINT32
IspDrv_B::
cam_isp_ccl_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){ //0x15004910 ~ 0x15004918
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_CCL){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
MINT32
IspDrv_B::
cam_isp_g2c_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_G2C_CONV){ //(G2C_CONV)0x15004BA0 ~ 0x15004BB4
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_DBG("[G2C](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }

        if(tuningPackage.eTuningCqFunc2==CAM_ISP_G2C_SHADE){ //(G2C_SHADE)0x15004BB8 ~ 0x15004BC8
            m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc2,addrOffset,moduleSize);
            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_DBG("[G2C](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    } else {
        isp_reg_t ispReg;
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_CONV_0A-(MUINT8*)&ispReg)), 0x012D0099, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_CONV_0B-(MUINT8*)&ispReg)), 0x0000003A, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_CONV_1A-(MUINT8*)&ispReg)), 0x075607AA, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_CONV_1B-(MUINT8*)&ispReg)), 0x00000100, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_CONV_2A-(MUINT8*)&ispReg)), 0x072A0100, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_CONV_2B-(MUINT8*)&ispReg)), 0x000007D6, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_SHADE_CON_1-(MUINT8*)&ispReg)), 0x0118000E, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_SHADE_CON_2-(MUINT8*)&ispReg)), 0x0074B740, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_SHADE_CON_3-(MUINT8*)&ispReg)), 0x00000133, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_SHADE_TAR-(MUINT8*)&ispReg)), 0x079F0A5A, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2C_SHADE_SP-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
    }

    return 0;
}
MINT32
IspDrv_B::
cam_isp_mfb_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004968 ~ 0x15004978
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_MFB_TUNECON){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_VRB("[MFB](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }
    //CAM_ISP_MFB_CTRL, 0x15004960
    ISP_FUNC_DBG("+(bld_mode(0x%x),mfbout(%d,%d,%d),out_xofst(%d))", imageioPackage.mfbCfg.bld_mode,imageioPackage.mfbCfg.mfb_out.w,\
        imageioPackage.mfbCfg.mfb_out.h,imageioPackage.mfbCfg.mfb_out.stride,imageioPackage.mfbCfg.out_xofst);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MFB_CON,BLD_MODE,imageioPackage.mfbCfg.bld_mode,ISP_DRV_USER_ISPF);
    //CAM_ISP_MFB_IMGCON, 0x15004964
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MFB_LL_CON1,BLD_LL_OUT_XSIZE,imageioPackage.mfbCfg.mfb_out.w,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MFB_LL_CON1,BLD_LL_OUT_YSIZE,imageioPackage.mfbCfg.mfb_out.h,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MFB_LL_CON1,BLD_LL_OUT_XOFST,imageioPackage.mfbCfg.out_xofst,ISP_DRV_USER_ISPF);
    return 0;
}

MINT32
IspDrv_B::
cam_isp_nbc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004A80 ~ 0x15004A94
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_NBC){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
MINT32
IspDrv_B::
cam_isp_pca_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_PCA_TBL){ //(PCA_TBL)0x15005800 ~ 0x15005D9C
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }

        if(tuningPackage.eTuningCqFunc2==CAM_ISP_PCA_CON){ //(PCA_CON)0x15005E00 ~ 0x15005E04
            m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc2,addrOffset,moduleSize);
            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
MINT32
IspDrv_B::
cam_isp_seee_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004AA0 ~ 0x15004AFC
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_SEEE){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_DBG("[SEEE](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}


MINT32
IspDrv_B::
cam_isp_lce_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {

    MINT32 lceSlmWd, lceSlmHt;
    MINT32 lceBcmkX, lceBcmkY;


    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x150049C0 ~ 0x150049E8
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d),eTuningCqDma(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2,tuningPackage.eTuningCqDma);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_LCE){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
        //
        if(tuningPackage.eTuningCqDma==CAM_DMA_LCEI){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqDma,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
        //update CAM_LCE_ZR(0x49C4)
        //CAM_LCE_ZR(0x49C4) be updated forcedly by path imageio
        //LCE_BCMK_X=floor(((LCE_SLM_WD-1)*32768) / (LCE_IMAGE_WD-1))
        //LCE_BCMK_Y=floor(((LCE_SLM_HT-1)*32768) / (LCE_IMAGE_HT-1))
        lceSlmWd = (tuningPackage.pTuningBuf[CAM_LCE_SLR_SIZE_REG_OFFSET>>2]>>CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_WD_BIT_SHIFT)&CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_WD_BIT_MASK;
        lceSlmHt = (tuningPackage.pTuningBuf[CAM_LCE_SLR_SIZE_REG_OFFSET>>2]&CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_HT_BIT_MASK)>>CAM_LCE_SLM_SIZE_FIELD_LCE_SLM_HT_BIT_SHIFT;
        if((lceSlmWd-1>0) && (lceSlmHt-1>0)){
            lceBcmkX = ((lceSlmWd-1)<<15)/(tuningPackage.lceInputW-1);
            lceBcmkY = ((lceSlmHt-1)<<15)/(tuningPackage.lceInputH-1);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_LCE_ZR, LCE_BCMK_X, lceBcmkX);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_LCE_ZR, LCE_BCMK_Y, lceBcmkY);
            ISP_FUNC_DBG("tuningPackage.lceInput(%u,%u)",tuningPackage.lceInputW,tuningPackage.lceInputH);
            ISP_FUNC_DBG("lceBcmkX(%d),lceBcmkY(%d)",lceBcmkX,lceBcmkY);
        }else if(imageioPackage.finalEnP2 & CAM_CTL_EN_P2_LCE_EN){
            ISP_FUNC_ERR("[Error]size error lceSlmWd(%d),lceSlmHt(%d),finalEnP2(0x%x)",lceSlmWd,lceSlmHt,imageioPackage.finalEnP2);
        }
    }
    //update from path imageio
    //CAM_LCE_IMAGE_SIZE(0x150049F0)
    ISP_WRITE_BITS(m_pP2IspDrv, CAM_LCE_IMAGE_SIZE, LCE_IMAGE_WD, tuningPackage.lceInputW);
    ISP_WRITE_BITS(m_pP2IspDrv, CAM_LCE_IMAGE_SIZE, LCE_IMAGE_HT, tuningPackage.lceInputH);
    //
    return 0;
}
MINT32
IspDrv_B::
cam_isp_nr3d_cfg( IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {

    MUINT32 nr3dLmtOutTh = 0x00;
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        MUINT32 nr3dGain;
        MUINT32 nr3dRndY, nr3dRndU, nr3dRndV;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_CDP_NR3D_BLENDING){ //0x15004CC0~0x15004CC0
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_DBG("[NR3D](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }

        //update NR3D_GAIN/(0x4cc0)
        nr3dGain = (tuningPackage.pTuningBuf[CAM_NR3D_BLEND_OFFSET>>2]>>CAM_NR3D_GAIN_BIT_SHIFT)&CAM_NR3D_GAIN_BIT_MASK;
        nr3dGain = ((nr3dGain*tuningPackage.nr3dGainWeighting)>>8)&CAM_NR3D_GAIN_BIT_MASK;
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_NR3D_BLEND, NR3D_GAIN, nr3dGain);
        nr3dRndY = 0x1f - (nr3dGain&CAM_NR3D_GAIN_BIT_MASK);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_NR3D_BLEND, NR3D_RND_Y, nr3dRndY);
        nr3dRndU = 0x1f - (nr3dGain&CAM_NR3D_GAIN_BIT_MASK);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_NR3D_BLEND, NR3D_RND_U, nr3dRndU);
        nr3dRndV = 0x1f - (nr3dGain&CAM_NR3D_GAIN_BIT_MASK);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_NR3D_BLEND, NR3D_RND_V, nr3dRndV);
        ISP_FUNC_DBG("nr3dGain_old(%d),nr3dGain_new(%d),nr3dGainWeighting(%d),nr3dRndY(%d),nr3dRndU(%d),nr3dRndV(%d)", \
            (tuningPackage.pTuningBuf[CAM_NR3D_BLEND_OFFSET>>2]>>CAM_NR3D_GAIN_BIT_SHIFT)&CAM_NR3D_GAIN_BIT_MASK, \
            nr3dGain,tuningPackage.nr3dGainWeighting,nr3dRndY,nr3dRndU,nr3dRndV);
        //
        if(tuningPackage.eTuningCqFunc2==CAM_CDP_NR3D_LMT){ //0x15004CD0~0x15004CF4 & 15004CF8(NR3D_LMT_OUT_CNT_TH)(bit0:1)
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc2,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
            //get 15004CF8(NR3D_LMT_OUT_CNT_TH)(bit0:1)==> controled by path featureio
            nr3dLmtOutTh = (tuningPackage.pTuningBuf[0x4cf8>>2]&CAM_REG_NR3D_CTRL_LMT_OUT_CNT_TH_BIT_MASK)>>CAM_REG_NR3D_CTRL_LMT_OUT_CNT_TH_BIT_SHIFT;
            ISP_FUNC_DBG("nr3dLmtOutTh(%d)",nr3dLmtOutTh);
        }
    }
    //update from path imageio
    //CAM_CDP_NR3D_FB(0x15004CC4~0x15004CCC)
    ISP_FUNC_DBG("fbcntOff_xoff(%d),fbcntOff_yoff(%d),fbcntSiz_xsiz(%d),fbcntSiz_ysiz(%d),fbCount_cnt(%d)", \
        imageioPackage.nr3dCfg.fbcntOff_xoff,imageioPackage.nr3dCfg.fbcntOff_yoff,imageioPackage.nr3dCfg.fbcntSiz_xsiz,imageioPackage.nr3dCfg.fbcntSiz_ysiz,imageioPackage.nr3dCfg.fbCount_cnt);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_FBCNT_OFF,NR3D_FB_CNT_XOFF,imageioPackage.nr3dCfg.fbcntOff_xoff,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_FBCNT_OFF,NR3D_FB_CNT_YOFF,imageioPackage.nr3dCfg.fbcntOff_yoff,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_FBCNT_SIZ,NR3D_FB_CNT_XSIZ,imageioPackage.nr3dCfg.fbcntSiz_xsiz,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_FBCNT_SIZ,NR3D_FB_CNT_YSIZ,imageioPackage.nr3dCfg.fbcntSiz_ysiz,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_FB_COUNT,NR3D_FB_CNT,imageioPackage.nr3dCfg.fbCount_cnt,ISP_DRV_USER_ISPF);

    //CAM_CDP_NR3D_CTRL(0x15004CF8~0x15004D00)
    ISP_FUNC_DBG("ctrl_lmtOutCntTh(%d),ctrl_onEn(%d),onOff_onOfStX(%d),onOff_onOfStY(%d),onSiz_onWd(%d),onSiz_onHt(%d)", \
        imageioPackage.nr3dCfg.ctrl_lmtOutCntTh,imageioPackage.nr3dCfg.ctrl_onEn,imageioPackage.nr3dCfg.onOff_onOfStX, \
        imageioPackage.nr3dCfg.onOff_onOfStY,imageioPackage.nr3dCfg.onSiz_onWd,imageioPackage.nr3dCfg.onSiz_onHt);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_CTRL,NR3D_LMT_OUT_CNT_TH,imageioPackage.nr3dCfg.ctrl_lmtOutCntTh,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_CTRL,NR3D_ON_EN,imageioPackage.nr3dCfg.ctrl_onEn,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_ON_OFF,NR3D_ON_OFST_X,imageioPackage.nr3dCfg.onOff_onOfStX,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_ON_OFF,NR3D_ON_OFST_Y,imageioPackage.nr3dCfg.onOff_onOfStY,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_ON_SIZ,NR3D_ON_WD,imageioPackage.nr3dCfg.onSiz_onWd,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_NR3D_ON_SIZ,NR3D_ON_HT,imageioPackage.nr3dCfg.onSiz_onHt,ISP_DRV_USER_ISPF);

    return 0;
}
MINT32
IspDrv_B::
cam_isp_sl2b_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    /*
    SL2B_CENTR_X    =      SL2_CENTR_X
    SL2B_CENTR_Y    =      SL2_CENTR_Y
    SL2B_RR_0       =      SL2_RR_0
    SL2B_RR_1       =      SL2_RR_1
    SL2B_RR_2       =      SL2_RR_2
    SL2B_HRZ_COMP   =      SL2_HRZ_COMP = 2048
    */
    if(imageioPackage.isApplyTuning){  //0x150040f60 ~ 0x15004F78
        MUINT32 sl2CentrXLsc, sl2CentrYLsc, sl2Rr0Lsc, sl2Rr1Lsc, sl2Rr2Lsc, sl2HrzComp;
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_SL2B){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
            // update sl2b
            // get raw sl2 data from CQ set by tuningMgr just now
            sl2CentrXLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_X);
            sl2CentrYLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_Y);
            sl2Rr0Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX0_RR, SL2_RR_0);
            sl2Rr1Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX1_RR, SL2_RR_1);
            sl2Rr2Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX2_RR, SL2_RR_2);
            sl2HrzComp = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_HRZ, SL2_HRZ_COMP);
            // set the sl2b data caculated to CQ
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2B_CEN, SL2B_CENTR_X, sl2CentrXLsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2B_CEN, SL2B_CENTR_Y, sl2CentrYLsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2B_MAX0_RR, SL2B_RR_0, sl2Rr0Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2B_MAX1_RR, SL2B_RR_1, sl2Rr1Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2B_MAX2_RR, SL2B_RR_2, sl2Rr2Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2B_HRZ, SL2B_HRZ_COMP, sl2HrzComp);
            ISP_FUNC_DBG("[SL2B]CentrXLsc(%d),CentrYLsc(%d),Rr0Lsc(%d),Rr1Lsc(%d),Rr2Lsc(%d),HrzComp(%d)",sl2CentrXLsc,sl2CentrYLsc,sl2Rr0Lsc,sl2Rr1Lsc,sl2Rr2Lsc,sl2HrzComp);
        }
    }

    return 0;
}
MINT32
IspDrv_B::
cam_isp_sl2c_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    /*
    SL2C_CENTR_X    =      SL2_CENTR_X*CRZ_Ratio
    SL2C_CENTR_Y    =      SL2_CENTR_Y*CRZ_Ratio
    SL2C_RR_0       =      SL2_RR_0*CRZ_Ratio*CRZ_Ratio
    SL2C_RR_1       =      SL2_RR_1*CRZ_Ratio*CRZ_Ratio
    SL2C_RR_2       =      SL2_RR_2*CRZ_Ratio*CRZ_Ratio
    SL2C_HRZ_COMP   =      SL2_HRZ_COMP = 2048
    */
    if(imageioPackage.isApplyTuning){  //0x15004FC0 ~ 0x15004FD8
        MUINT32 sl2CentrXLsc, sl2CentrYLsc, sl2Rr0Lsc, sl2Rr1Lsc, sl2Rr2Lsc, sl2HrzComp;
        MUINT32 sl2cCentrXLsc, sl2cCentrYLsc, sl2cRr0Lsc, sl2cRr1Lsc, sl2cRr2Lsc, sl2cHrzComp;
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_SL2C){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
            // update sl2c
            // get raw sl2c data from CQ set by tuningMgr just now
            sl2CentrXLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_X);
            sl2CentrYLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTR_Y);
            sl2Rr0Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX0_RR, SL2_RR_0);
            sl2Rr1Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX1_RR, SL2_RR_1);
            sl2Rr2Lsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_MAX2_RR, SL2_RR_2);
            sl2HrzComp = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_HRZ, SL2_HRZ_COMP);
            // calculate the right sl2c data
            sl2cCentrXLsc = sl2CentrXLsc*tuningPackage.sl2cCrzRatio;
            sl2cCentrYLsc = sl2CentrYLsc*tuningPackage.sl2cCrzRatio;
            sl2cRr0Lsc = sl2Rr0Lsc*tuningPackage.sl2cCrzRatio*tuningPackage.sl2cCrzRatio;
            sl2cRr1Lsc = sl2Rr1Lsc*tuningPackage.sl2cCrzRatio*tuningPackage.sl2cCrzRatio;
            sl2cRr2Lsc = sl2Rr2Lsc*tuningPackage.sl2cCrzRatio*tuningPackage.sl2cCrzRatio;
            sl2cHrzComp = sl2HrzComp;
            // set the sl2c data caculated to CQ
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2C_CEN, SL2C_CENTR_X, sl2cCentrXLsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2C_CEN, SL2C_CENTR_Y, sl2cCentrYLsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2C_MAX0_RR, SL2C_RR_0, sl2cRr0Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2C_MAX1_RR, SL2C_RR_1, sl2cRr1Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2C_MAX2_RR, SL2C_RR_2, sl2cRr2Lsc);
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2C_HRZ, SL2C_HRZ_COMP, sl2cHrzComp);
            ISP_FUNC_DBG("[SL2C]CentrXLsc(%d),CentrYLsc(%d),Rr0Lsc(%d),Rr1Lsc(%d),Rr2Lsc(%d),HrzComp(%d)",sl2cCentrXLsc,sl2cCentrYLsc,sl2cRr0Lsc,sl2cRr1Lsc,sl2cRr2Lsc,sl2cHrzComp);
        }
    }

    return 0;
}


MINT32
IspDrv_B::
cam_isp_cfa_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+");
    isp_reg_t ispReg;
    MUINT32 fgModeRegBit, reg_cam_dm_o_byp;

    fgModeRegBit = (imageioPackage.enFgMode&0x01)<<4;
    reg_cam_dm_o_byp = ((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_BYP-(MUINT8*)&ispReg));

    if(imageioPackage.isApplyTuning){  //0x150048A0 ~ 0x150048F4
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d),fgModeRegBit(0x%x),reg_cam_dm_o_byp(0x%x)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2,fgModeRegBit,reg_cam_dm_o_byp);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_CFA){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                if(reg_cam_dm_o_byp != addrOffset){
                    regsStruct[i].Addr = addrOffset;
                    regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                } else {  // set fg mode
                    regsStruct[i].Addr = addrOffset;
                    regsStruct[i].Data = (tuningPackage.pTuningBuf[addrOffset>>2] & ~reg_cam_dm_o_byp) | reg_cam_dm_o_byp;
                }
                addrOffset += 4;
                ISP_FUNC_VRB("[CFA](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    } else {

        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_BYP-(MUINT8*)&ispReg)), (0x00000000|fgModeRegBit), ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_ED_FLAT-(MUINT8*)&ispReg)), 0x0000FF03, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_ED_NYQ-(MUINT8*)&ispReg)), 0x0100FFFF, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_ED_STEP-(MUINT8*)&ispReg)), 0x80FF00FF, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_RGB_HF-(MUINT8*)&ispReg)), 0x01084208, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_DOT-(MUINT8*)&ispReg)), 0x0000FFFF, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F1_ACT-(MUINT8*)&ispReg)), 0x0003FE00, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F2_ACT-(MUINT8*)&ispReg)), 0x0003FE00, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F3_ACT-(MUINT8*)&ispReg)), 0x0003FE00, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F4_ACT-(MUINT8*)&ispReg)), 0x0003FE00, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F1_L-(MUINT8*)&ispReg)), 0x01084210, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F2_L-(MUINT8*)&ispReg)), 0x01084210, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F3_L-(MUINT8*)&ispReg)), 0x01084210, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_F4_L-(MUINT8*)&ispReg)), 0x01084210, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_HF_RB-(MUINT8*)&ispReg)), 0x000403FF, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_HF_GAIN-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_HF_COMP-(MUINT8*)&ispReg)), 0xFF048888, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_HF_CORIN_TH-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_ACT_LUT-(MUINT8*)&ispReg)), 0x00842108, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.rsv_48EC-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_SPARE-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
        m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_DM_O_BB-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);
    }
    return 0;
}
//
int
IspDrv_B::
cam_isp_g2g_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {

    ISP_FUNC_DBG("+,m_pP2IspDrv(0x%08x),cq(%d),subMode(%d),isApplyTuning(0x%08x)",m_pP2IspDrv,imageioPackage.cq, imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004920 ~ 0x15004938
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_G2G){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_VRB("[G2G](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    } else {
        isp_reg_t ispReg;
        switch(imageioPackage.subMode){
            case ISP_SUB_MODE_YUV:
                // G2G(YUV -> RGB)
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_1-(MUINT8*)&ispReg)), 0x00000200, ISP_DRV_USER_ISPF, 0);  /* 0x15004920: CAM_G2G_CNV_1 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_2-(MUINT8*)&ispReg)), 0x000002CE, ISP_DRV_USER_ISPF, 0);  /* 0x15004924: CAM_G2G_CNV_2 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_3-(MUINT8*)&ispReg)), 0x1F500200, ISP_DRV_USER_ISPF, 0);  /* 0x15004928: CAM_G2G_CNV_3 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_4-(MUINT8*)&ispReg)), 0x00001E92, ISP_DRV_USER_ISPF, 0);  /* 0x1500492C: CAM_G2G_CNV_4 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_5-(MUINT8*)&ispReg)), 0x038B0200, ISP_DRV_USER_ISPF, 0);  /* 0x15004930: CAM_G2G_CNV_5 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_6-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);  /* 0x15004934: CAM_G2G_CNV_6 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CTRL-(MUINT8*)&ispReg)), 0x00000009, ISP_DRV_USER_ISPF, 0);  /* 0x15004938: CAM_G2G_CTRL */
                break;
            default:
                // G2G(RGB -> RGB)
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_1-(MUINT8*)&ispReg)), 0x00000200, ISP_DRV_USER_ISPF, 0);  /* 0x15004920: CAM_G2G_CNV_1 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_2-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);  /* 0x15004924: CAM_G2G_CNV_2 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_3-(MUINT8*)&ispReg)), 0x02000000, ISP_DRV_USER_ISPF, 0);  /* 0x15004928: CAM_G2G_CNV_3 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_4-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);  /* 0x1500492C: CAM_G2G_CNV_4 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_5-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);  /* 0x15004930: CAM_G2G_CNV_5 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CNV_6-(MUINT8*)&ispReg)), 0x00000200, ISP_DRV_USER_ISPF, 0);  /* 0x15004934: CAM_G2G_CNV_6 */
                m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_G2G_CTRL-(MUINT8*)&ispReg)), 0x00000009, ISP_DRV_USER_ISPF, 0);  /* 0x15004938: CAM_G2G_CTRL */
                break;
        };
    }


    return 0;
}

//
int
IspDrv_B::
cam_isp_ggm_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+");
    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_GGM_RB){ //(GGM_RB)0x15005000 ~ 0x1500523C
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }

        if(tuningPackage.eTuningCqFunc2==CAM_ISP_GGM_G){ //(GGM_G) 0x15005240 ~ 0x1500547C
            m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc2,addrOffset,moduleSize);
            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
//
int
IspDrv_B::
cam_isp_c24_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {

    if((imageioPackage.isApplyTuning) && (CAM_DUMMY_ != tuningPackage.eTuningCqFunc1)){
        /* do nothing */
        ISP_FUNC_ERR("[Error]not support this tuning setting,eTuningCqFunc1(0x%x)",tuningPackage.eTuningCqFunc1);
    }else{
     //TODO
    }

    return 0;
}
//
int
IspDrv_B::
cam_isp_c42_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    isp_reg_t ispReg;

    m_pP2IspDrv->writeReg(((MUINT32)((MUINT8*)&ispReg.CAM_C42_CON-(MUINT8*)&ispReg)), 0x00000000, ISP_DRV_USER_ISPF, 0);

    return 0;
}

//
int
IspDrv_B::
cam_isp_c02_cfg(MUINT32 srcImgW,MUINT32 srcImgH) {
    ISP_FUNC_DBG("cam_isp_c02_cfg +, w(0x%x),h(0x%x)\n",srcImgW,srcImgH);

    ISP_WRITE_BITS(m_pP2IspDrv,CAM_C02_CON,C02_TPIPE_EDGE,0xF,ISP_DRV_USER_ISPF); //default 0xf, tpipemain would update it
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_C02_CROP_CON1,C02_CROP_XSTART,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_C02_CROP_CON1,C02_CROP_XEND,srcImgW-1,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_C02_CROP_CON2,C02_CROP_YSTART,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_C02_CROP_CON2,C02_CROP_YEND,srcImgH-1,ISP_DRV_USER_ISPF);
    return 0;
}
//
MINT32
IspDrv_B::
cam_isp_ob_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004500 ~ 0x1500451C
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_OBC){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
//
MINT32
IspDrv_B::
cam_isp_bnr_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004800 ~ 0x1500482C
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_BNR){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
        //
        if(tuningPackage.eTuningCqDma==CAM_DMA_BPCI){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqDma,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
//
MINT32
IspDrv_B::
cam_isp_lsc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004530 ~ 0x1500454C
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_LSC){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
        //
        if(tuningPackage.eTuningCqDma==CAM_DMA_LSCI){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqDma,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }

    }

    return 0;
}
//
MINT32
IspDrv_B::
cam_isp_rpg_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004550 ~ 0x15004564
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_RPG){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
//
MINT32
IspDrv_B::
cam_isp_rmg_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //0x15004104 ~ 0x15004104
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_CTL_IHDR){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}
//
MINT32
IspDrv_B::
cam_isp_mix3_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){  //15004CB0~15004CB8
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_CDP_MIX3){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    }

    return 0;
}

//

MINT32 IspDrv_B::cam_isp_crsp_cfg(MUINT32 enCrsp, MUINT32 desImgW,MUINT32 desImgH)
{
    ISP_FUNC_DBG("cam_isp_crsp_cfg +,enCrsp(%d),w(0x%x),h(0x%x)\n",enCrsp,desImgW,desImgH);
    //crsp_crop_en=1 in tile mode (0 in frame mode?)
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_CTRL,CRSP_HORI_EN,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_CTRL,CRSP_VERT_EN,0x1,ISP_DRV_USER_ISPF);//the same with crsp_en

    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_STEP_OFST,CRSP_STEP_X,0x4,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_STEP_OFST,CRSP_STEP_Y,0x4,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_STEP_OFST,CRSP_OFST_X,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_STEP_OFST,CRSP_OFST_Y,0x1,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_OUT_IMG,CRSP_WD,desImgW,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_OUT_IMG,CRSP_HT,desImgH,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_CROP_X,CRSP_CROP_EN,enCrsp,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_CROP_X,CRSP_CROP_STR_X,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_CROP_X,CRSP_CROP_END_X,desImgW-1,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_CROP_Y,CRSP_CROP_STR_Y,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_CRSP_CROP_Y,CRSP_CROP_END_Y,desImgH-1,ISP_DRV_USER_ISPF);
    return 0;
}
//
int
IspDrv_B::
cam_isp_rrz_cfg(IspDrv* pCurDrv,IspSize in,IspRect crop,IspSize out,MUINT32 ctrl,MUINT32 h_step,MUINT32 v_step, MUINT32 isRaw_D) {
    CAM_REG_RRZ_MODE_CTL rrz_mode;
    CAM_REG_RRZ_MODE_TH  rrz_th;

    ISP_FUNC_DBG("+");
    // 0x150047A0  CAM_RRZ_CTL
    // 0x150047A4  CAM_RRZ_IN_IMG
    // 0x150047A8  CAM_RRZ_OUT_IMG
    // 0x150047AC  CAM_RRZ_HORI_STEP
    // 0x150047B0  CAM_RRZ_VERT_STEP
    // 0x150047BC  CAM_RRZ_VERT_INT_OFST
    // 0x150047C0  CAM_RRZ_VERT_SUB_OFST
    // 0x150047C4  CAM_RRZ_MODE_TH
    // 0x150047C8  CAM_RRZ_MODE_CTL
    ISP_FUNC_DBG("+[setRRZToCQ](ctrl(0x%x),in(%d,%d),out(%d,%d),step(0x%x,0x%x),ofst(%d,%d,%d,%d/%d,%d))", \
                 ctrl,in.w,in.h,out.w,out.h,h_step,v_step,crop.x,crop.floatX,crop.y,crop.floatY, crop.w,crop.h);

    rrz_mode.Raw = 0;
    rrz_th.Raw = 0;

    rrz_mode.Bits.RRZ_CR_MODE = 1;
    rrz_mode.Bits.RRZ_BLD_SL = 16;

    rrz_th.Bits.RRZ_TH_LO = 80;
    rrz_th.Bits.RRZ_TH_HI = 40;
    rrz_th.Bits.RRZ_TH_MD = 80;
    rrz_th.Bits.RRZ_TH_MD2 = 0;

    if ( !isRaw_D ) {
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_CTL, ctrl, ISP_DRV_USER_ISPF);    //0x150047A0
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_IN_IMG, (in.h<<16) | in.w, ISP_DRV_USER_ISPF);    //0x150047A4
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_OUT_IMG, (out.h<<16) | out.w, ISP_DRV_USER_ISPF);    //0x150047A8
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_HORI_STEP, h_step, ISP_DRV_USER_ISPF);    //0x150047AC
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_VERT_STEP, v_step, ISP_DRV_USER_ISPF);    //0x150047B0
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_HORI_INT_OFST, crop.x, ISP_DRV_USER_ISPF);    //0x150047B4
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_HORI_SUB_OFST, crop.floatX, ISP_DRV_USER_ISPF);    //0x150047B8
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_VERT_INT_OFST, crop.y, ISP_DRV_USER_ISPF);    //0x150047BC
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_VERT_SUB_OFST, crop.floatY, ISP_DRV_USER_ISPF);    //0x150047C0
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_MODE_CTL, rrz_mode.Raw, ISP_DRV_USER_ISPF);    //0x150047C04
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_MODE_TH, rrz_th.Raw, ISP_DRV_USER_ISPF);    //0x150047C8
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_CROP_IN,(crop.h<<16|crop.w),ISP_DRV_USER_ISPF);//0x150075E0
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_OUT_W, out.w,ISP_DRV_USER_ISPF);
    }
    else {
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_CTL, ctrl, ISP_DRV_USER_ISPF);    //0x150047A0
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_IN_IMG, (in.h<<16) | in.w, ISP_DRV_USER_ISPF);    //0x150047A4
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_OUT_IMG, (out.h<<16) | out.w, ISP_DRV_USER_ISPF);    //0x150047A8
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_HORI_STEP, h_step, ISP_DRV_USER_ISPF);    //0x150047AC
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_VERT_STEP, v_step, ISP_DRV_USER_ISPF);    //0x150047B0
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_HORI_INT_OFST, crop.x, ISP_DRV_USER_ISPF);    //0x150047B4
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_HORI_SUB_OFST, crop.floatX, ISP_DRV_USER_ISPF);    //0x150047B8
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_VERT_INT_OFST, crop.y, ISP_DRV_USER_ISPF);    //0x150047BC
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_VERT_SUB_OFST, crop.floatY, ISP_DRV_USER_ISPF);    //0x150047C0
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_MODE_CTL, rrz_mode.Raw, ISP_DRV_USER_ISPF);    //0x150047C04
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_D_MODE_TH, rrz_th.Raw, ISP_DRV_USER_ISPF);    //0x150047C8
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_CROP_IN_D,(crop.h<<16|crop.w),ISP_DRV_USER_ISPF);//0x150075E8
        ISP_WRITE_REG(pCurDrv, CAM_RRZ_OUT_W_D, out.w,ISP_DRV_USER_ISPF);

        // for debug
        /*ISP_FUNC_DBG("Rd CAM_RRZ_D_CTL=0x%x"      , ISP_READ_REG_NOPROTECT(m_pP1IspDrvD,CAM_RRZ_D_CTL));
        ISP_FUNC_DBG("Rd CAM_RRZ_D_IN_IMG=0x%x"   , ISP_READ_REG_NOPROTECT(m_pP1IspDrvD,CAM_RRZ_D_IN_IMG));
        ISP_FUNC_DBG("Rd CAM_RRZ_D_OUT_IMG=0x%x"  , ISP_READ_REG_NOPROTECT(m_pP1IspDrvD,CAM_RRZ_D_OUT_IMG));
        ISP_FUNC_DBG("Rd CAM_RRZ_D_HORI_STEP=0x%x", ISP_READ_REG_NOPROTECT(m_pP1IspDrvD,CAM_RRZ_D_HORI_STEP));
        ISP_FUNC_DBG("Rd CAM_RRZ_D_VERT_STEP=0x%x", ISP_READ_REG_NOPROTECT(m_pP1IspDrvD,CAM_RRZ_D_VERT_STEP));
        */
    }

    ISP_FUNC_DBG("-");
    return 0;
}
//
MINT32 IspDrv_B::cam_isp_mix1_cfg()
{
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX1_CTRL_0,MIX1_WT_SEL,0x1,ISP_DRV_USER_ISPF);    //TODO
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX1_CTRL_0,MIX1_B0,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX1_CTRL_0,MIX1_B1,0xff,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX1_CTRL_0,MIX1_DT,0x1,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX1_CTRL_1,MIX1_M0,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX1_CTRL_1,MIX1_M1,0xff,ISP_DRV_USER_ISPF);
    return 0;
}
MINT32 IspDrv_B::cam_isp_mix2_cfg()
{
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX2_CTRL_0,MIX2_WT_SEL,0x1,ISP_DRV_USER_ISPF);    //TODO
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX2_CTRL_0,MIX2_B0,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX2_CTRL_0,MIX2_B1,0xff,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX2_CTRL_0,MIX2_DT,0x1,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX2_CTRL_1,MIX2_M0,0x0,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_MIX2_CTRL_1,MIX2_M1,0xff,ISP_DRV_USER_ISPF);
    return 0;
}
MINT32 IspDrv_B::cam_isp_srz1_cfg(MUINT32 ctrl, SrzSizeCfg inout_size,IspRect crop,MUINT32 h_step,MUINT32 v_step)
{
    ISP_FUNC_DBG("+(ctrl(0x%x),in(%d,%d),out(%d,%d),step(0x%x,0x%x),ofst(%d,%d,%d,%d,))", \
                 ctrl,inout_size.in_w,inout_size.in_h,inout_size.out_w,inout_size.out_h,h_step,v_step,crop.x,crop.floatX,crop.y,crop.floatY);

    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ1_CONTROL,ctrl,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ1_IN_IMG,SRZ1_IN_WD,inout_size.in_w,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ1_IN_IMG,SRZ1_IN_HT,inout_size.in_h,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ1_OUT_IMG,SRZ1_OUT_WD,inout_size.out_w,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ1_OUT_IMG,SRZ1_OUT_HT,inout_size.out_h,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ1_HORI_STEP,h_step,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ1_VERT_STEP,v_step,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ1_HORI_INT_OFST,crop.x,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ1_HORI_SUB_OFST,crop.floatX,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ1_VERT_INT_OFST,crop.y,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ1_VERT_SUB_OFST,crop.floatY,ISP_DRV_USER_ISPF);
    return 0;
}

MINT32 IspDrv_B::cam_isp_srz2_cfg(MUINT32 ctrl, SrzSizeCfg inout_size,IspRect crop,MUINT32 h_step,MUINT32 v_step)
{
    ISP_FUNC_DBG("+(ctrl(0x%x),in(%d,%d),out(%d,%d),step(0x%x,0x%x),ofst(%d,%d,%d,%d,))", \
                 ctrl,inout_size.in_w,inout_size.in_h,inout_size.out_w,inout_size.out_h,h_step,v_step,crop.x,crop.floatX,crop.y,crop.floatY);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ2_CONTROL,ctrl,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ2_IN_IMG,SRZ2_IN_WD,inout_size.in_w,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ2_IN_IMG,SRZ2_IN_HT,inout_size.in_h,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ2_OUT_IMG,SRZ2_OUT_WD,inout_size.out_w,ISP_DRV_USER_ISPF);
    ISP_WRITE_BITS(m_pP2IspDrv,CAM_SRZ2_OUT_IMG,SRZ2_OUT_HT,inout_size.out_h,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ2_HORI_STEP,h_step,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ2_VERT_STEP,v_step,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ2_HORI_INT_OFST,crop.x,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ2_HORI_SUB_OFST,crop.floatX,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ2_VERT_INT_OFST,crop.y,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_SRZ2_VERT_SUB_OFST,crop.floatY,ISP_DRV_USER_ISPF);
    return 0;
}

//

int
IspDrv_B::
cam_isp_obc_cfg(MUINT32 cq, stIspOBCCtl_t obc)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_bpc_cfg(MUINT32 cq, stIspBPCCtl_t bpc)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_rmm_cfg(MUINT32 cq, stIspRMMCtl_t rmm)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_nr1_cfg(MUINT32 cq, stIspNR1Ctl_t nr1)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_lsc_cfg(MUINT32 cq, stIspLSCCtl_t lsc)
{
    return 0;
}
int
IspDrv_B::
cam_isp_bnr_rpg_cfg(MUINT32 cq, stIspRPGCtl_t rpg)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_ae_cfg(MUINT32 cq, stIspAECtl_t ae)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_awb_cfg(MUINT32 cq, stIspAWBCtl_t awb)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_sgg1_cfg(MUINT32 cq, stIspSGG1Ctl_t sgg1)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_flk_cfg(MUINT32 cq, stIspFLKCtl_t flk)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_af_cfg(MUINT32 cq, stIspAFCtl_t af)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_sgg2_cfg(MUINT32 cq, stIspSGG2Ctl_t sgg2)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_eis_cfg(MUINT32 cq, stIspEISCtl_t eis)
{
    return 0;
}

int
IspDrv_B::
cam_isp_bnr_lcs_cfg(MUINT32 cq, stIspLCSCtl_t lcs)
{
    return 0;
}




/*/////////////////////////////////////////////////////////////////////////////
    IspDrvShellImp
/////////////////////////////////////////////////////////////////////////////*/
#ifdef _CmdQ_HwBug_Patch_
MUINTPTR lsciSwWorkaroundBufPa;
MBOOL   lsciSwWorkaroundBufInit = 0;
#endif

Mutex IspDrvShell::mCQBufferLock;

class IspDrvShellImp:public IspDrvShell
{
    public:
        IspDrvShellImp(){}

        static IspDrvShell* getInstance(NSImageio::NSIspio::EDrvScenario   eScenarioID = NSImageio::NSIspio::eDrvScenario_CC);
        virtual void        destroyInstance(void);
        virtual MBOOL       init(const char* userName="");
        virtual MBOOL       uninit(const char* userName="");
        virtual MBOOL       controlBurstQueBuffer(EIspControlBufferCmd cmd, MUINT32 burstQueueNum);
        virtual MBOOL       updateBurstQueueNum(MINT32 burstQNum);
        virtual MBOOL       getNr3dGain(ESoftwareScenario softScenario, MINT32 magicNum, MINT32 &nr3dGain);
    public://driver object operation
        inline virtual IspDrv*      getPhyIspDrv(){return m_pPhyIspDrv_bak; }

    public://phy<->virt ISP switch
        //inline MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 dupCqIdx);
        inline MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 P1CqD,MINT32 burstQIdx,MINT32 P2DupCqIdx);
        //inline MUINT32     ispDrvGetCQRegTblAddr(MINT32 cq,MINT32 dupCqIdx);
        inline IspDrv*     ispDrvGetCQIspDrv(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx);

    public://commandQ operation
        virtual MBOOL       cam_cq_cfg(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx,MINT32 isP2IspOnly=0);
        virtual MBOOL       cqAddModule(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
        virtual MBOOL       cqDelModule(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
        virtual int         getCqModuleInfo(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
        virtual MBOOL       setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);
        virtual void        CQBufferMutexLock( void );
        virtual void        CQBufferMutexUnLock( void );


    protected:
        volatile MINT32 mInitCount;
        MTKM4UDrv*  mpM4UDrv;

public:
    //lsci
    MUINT32 *lsciSwWorkaroundBufVa;
    MINT32  lsciSwWorkaroundBufFd;
    IMEM_BUF_INFO  lsciSwWorkaroundBufInfo;
    //tpipe
    IMEM_BUF_INFO  tpipeBufInfo[ISP_DVR_MAX_BURST_QUEUE_NUM][ISP_DRV_P2_CQ_NUM][ISP_DRV_P2_CQ_DUPLICATION_NUM];
};
//
IspDrvShell* IspDrvShell::createInstance(NSImageio::NSIspio::EDrvScenario   eScenarioID)
{
    DBG_LOG_CONFIG(imageio, function);
//    ISP_FUNC_DBG("");
    ISP_FUNC_INF("eScenarioID: %d.", eScenarioID);
    return IspDrvShellImp::getInstance(eScenarioID);
}
//
IspDrvShell*
IspDrvShellImp::
getInstance(NSImageio::NSIspio::EDrvScenario eScenarioID)
{
//    ISP_FUNC_DBG("");
    ISP_FUNC_INF("eScenarioID: %d.", eScenarioID);
    static IspDrvShellImp singleton;

    return &singleton;
}
//
void
IspDrvShellImp::
destroyInstance(void)
{

}

//
stISP_BUF_LIST ISP_BUF_CTRL::m_hwbufL[_rt_dma_max_];
static ISP_RAW_CENTRAL_CTRL_INFO rawCentralCtrl;

stISP_BUF_LIST ISP_BUF_CTRL::m_p2HwBufL[ISP_DRV_P2_CQ_NUM][ISP_DRV_P2_CQ_DUPLICATION_NUM][_rt_dma_max_];
//
MBOOL
IspDrvShellImp::
init(const char* userName)
{
int ret = 0;
    Mutex::Autolock lock(mCQBufferLock);

    ISP_FUNC_INF("IspDrvShellImp:mInitCount(%d)", mInitCount);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        return MTRUE;
    }

    memset (&rawCentralCtrl, 0, sizeof(ISP_RAW_CENTRAL_CTRL_INFO));
    /*============================================
        imem driver
    =============================================*/
    m_pIMemDrv = IMemDrv::createInstance();
    ISP_FUNC_DBG("[m_pIMemDrv]:0x%08x ",m_pIMemDrv);
    if ( NULL == m_pIMemDrv ) {
        ISP_FUNC_ERR("IMemDrv::createInstance fail.");
        return -1;
    }
    m_pIMemDrv->init();


    /*============================================
     lsciSwWorkaroundBuf
    =============================================*/
    {
#if defined(__PMEM_ONLY__)
#ifdef _CmdQ_HwBug_Patch_
       lsciSwWorkaroundBufVa = (MUINT32*)pmem_alloc_sync( (300+ (7+1)), &lsciSwWorkaroundBufFd);
       lsciSwWorkaroundBufPa = (MUINT32)pmem_get_phys(lsciSwWorkaroundBufFd);
#endif
#else   // Not PMEM.
#ifdef _CmdQ_HwBug_Patch_
       //
       lsciSwWorkaroundBufInfo.size = 300 + (7+1);
       lsciSwWorkaroundBufInfo.useNoncache = 1; //alloc non-cacheable mem.
       //
       if ( m_pIMemDrv->allocVirtBuf(&lsciSwWorkaroundBufInfo) ) {
           ISP_FUNC_ERR("ERROR:m_pIMemDrv->allocVirtBuf(lsciSwWorkaroundBufInfo)");
       }
       //LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->allocVirtBuf(lsciSwWorkaroundBufInfo)");
       lsciSwWorkaroundBufFd = lsciSwWorkaroundBufInfo.memID;
       lsciSwWorkaroundBufVa = (MUINT32*)( (lsciSwWorkaroundBufInfo.virtAddr+7) & (~7) );
       //
       if ( m_pIMemDrv->mapPhyAddr(&lsciSwWorkaroundBufInfo) ) {
           ISP_FUNC_ERR("ERROR:m_pIMemDrv->mapPhyAddr(lsciSwWorkaroundBufInfo)");
       }
       //CQ decriptor base address should be 8 bytes alignment
       lsciSwWorkaroundBufPa =  ( (lsciSwWorkaroundBufInfo.phyAddr+7) & (~7) );
#endif
#endif  // __PMEM_ONLY__
    }


    /*============================================
     isp driver
    =============================================*/
    m_pPhyIspDrv = IspDrv::createInstance();
    ISP_FUNC_INF("[m_pPhyIspDrv]:0x%08x ",m_pPhyIspDrv);
    if (!m_pPhyIspDrv) {
        ISP_FUNC_ERR("m_pPhyIspDrv::createInstance fail ");
        return ret;
    }
    //
    ret = m_pPhyIspDrv->init(userName);
    if (ret < 0) {
        ISP_FUNC_ERR("m_pPhyIspDrv->init() fail ");
        return ret;
    }
    //backup
    m_pPhyIspDrv_bak = m_pPhyIspDrv;
    /*============================================
     control MDP & Tpipe buffer & virtual isp
    =============================================*/
    MINT32 curBurstQNum=1;
    curBurstQNum=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM);
    controlBurstQueBuffer(eIspControlBufferCmd_Alloc, curBurstQNum);

    /*============================================
     buffer control
    =============================================*/
    //clear all once
    ISP_FUNC_DBG("clear buffer control ");
    for ( int i =0; i < _rt_dma_max_ ;i++ ) {
        ISP_BUF_CTRL::m_hwbufL[i].bufInfoList.clear();
    }

    for ( int i =0; i < ISP_DRV_P2_CQ_NUM ;i++ )
        for ( int j =0; j < ISP_DRV_P2_CQ_DUPLICATION_NUM ;j++ )
            for ( int k =0; k < _rt_dma_max_ ;k++ )
                ISP_BUF_CTRL::m_p2HwBufL[i][j][k].bufInfoList.clear();


    //
    if(m_pPhyIspDrv->m_regRWMode==ISP_DRV_RWREG_MODE_RW)
    {    //camera would get read/write authorization
        if ( 0 != this->m_pPhyIspDrv->m_pRTBufTbl ) {
            //memset(this->m_pPhyIspDrv->m_pRTBufTbl,0x00,sizeof(ISP_RT_BUF_STRUCT));
        }
        else {
            ISP_FUNC_ERR("NULL m_pRTBufTbl");
        }
    }//readonly if other process, and suppose it would only use postproc related variables

    /*============================================
     crz driver
    =============================================*/
    m_pP2CrzDrv = CrzDrv::CreateInstance();
    ISP_FUNC_DBG("[m_pCrzDrv]:0x%08x ",m_pP2CrzDrv);
    if ( NULL == m_pP2CrzDrv ) {
        ISP_FUNC_ERR("CrzDrv::CreateInstance cfail ");
        return -1;
    }
    m_pP2CrzDrv->Init();
    m_pP2CrzDrv->SetIspReg(m_pVirtIspReg[ISP_DRV_CQ01]);  // set a point for cdp CheckReady() of CalAlgoAndCStep()


    /*============================================
     twin driver
    =============================================*/
    m_pTwinDrv = TwinDrv::createInstance();
    ISP_FUNC_DBG("[m_pTwinDrv]:0x%08x ",m_pTwinDrv);
    if ( NULL == m_pTwinDrv ) {
        ISP_FUNC_ERR("TwinDrv::createInstance fail ");
        return -1;
    }
    m_pTwinDrv->init();
    /*============================================
     ispEventThread driver
    =============================================*/
    m_pIspEventThread = IspEventThread::createInstance(m_pPhyIspDrv_bak);
    ISP_FUNC_INF("[m_pIspEventThread]:0x%08x ",m_pIspEventThread);
    if ( NULL == m_pIspEventThread ) {
        ISP_FUNC_ERR("IspEventThread::createInstance fail ");
        return -1;
    }
    //m_pIspEventThread->init();
    //
    ISP_FUNC_INF("X");

    android_atomic_inc(&mInitCount);

    return MTRUE;

}
//
MBOOL
IspDrvShellImp::
uninit(const char* userName)
{
int ret = 0;

    Mutex::Autolock lock(mCQBufferLock);

    ISP_FUNC_INF("IspDrvShellImp:mInitCount(%d)",mInitCount);
    //
    if(mInitCount <= 0)
    {
        // No more users
        return MTRUE;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        return MTRUE;
    }

#if 0 //_mt6593fpga_dvt_use_

    /*============================================
    cdp driver release
    =============================================*/
    //CDRZ
    m_pCdpDrv->CDRZ_Unconfig( );
    //CURZ
    m_pCdpDrv->CURZ_Unconfig( ) ;
    //PRZ
    m_pCdpDrv->PRZ_Unconfig( ) ;

//js_test move to _disable.
    //VIDO
    //m_pCdpDrv->VIDO_Unconfig( ) ;
    //DISPO
    //m_pCdpDrv->DISPO_Unconfig();
    //
    m_pCdpDrv->Uninit();
    m_pCdpDrv->DestroyInstance();
    m_pCdpDrv = NULL;
#endif


    {
#if defined(__PMEM_ONLY__)
        if(lsciSwWorkaroundBufFd >= 0) {
            pmem_free((MUINT8*)lsciSwWorkaroundBufVa,(300+ (ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN+1)),lsciSwWorkaroundBufFd);
            lsciSwWorkaroundBufFd = -1;
        }
#else   // Not PMEM.
        //
        if ( m_pIMemDrv->unmapPhyAddr(&lsciSwWorkaroundBufInfo) ) {
            ISP_FUNC_ERR("ERROR:m_pIMemDrv->unmapPhyAddr (lsciSwWorkaroundBufInfo)");
        }
        //LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->unmapPhyAddr(lsciSwWorkaroundBufInfo)");
        //
        if ( m_pIMemDrv->freeVirtBuf(&lsciSwWorkaroundBufInfo) ) {
            ISP_FUNC_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
        }
        //LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->freeVirtBuf(lsciSwWorkaroundBufInfo)");
        ISP_FUNC_DBG("free/unmap lsciSwWorkaroundBufInfo");
#endif  // __PMEM_ONLY__
    }

    /*============================================
     control MDP & Tpipe buffer & virtual isp
    =============================================*/
    MINT32 curBurstQNum=1;
    curBurstQNum=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM);
    controlBurstQueBuffer(eIspControlBufferCmd_Free, curBurstQNum);

    //IMEM
    m_pIMemDrv->uninit();
    m_pIMemDrv->destroyInstance();
    m_pIMemDrv = NULL;

    //twin driver
    m_pTwinDrv->uninit();
    m_pTwinDrv->destroyInstance();
    m_pTwinDrv = NULL;

    //ispEventThread
#if 1
    //m_pIspEventThread->uninit();
    m_pIspEventThread = NULL;
#endif
    //
    /*============================================
     isp driver
    =============================================*/
    ISP_FUNC_DBG("m_pP1IspDrv(0x%x),m_pP2IspDrv(0x%x),m_pPhyIspDrv_bak(0x%x) ",m_pP1IspDrv,m_pP2IspDrv,m_pPhyIspDrv_bak);
    if (m_pPhyIspDrv_bak) {
        ret = m_pPhyIspDrv_bak->uninit(userName);
        if (ret < 0) {
            ISP_FUNC_ERR("pIspDrv->uninit() fail ");
            return ret;
        }
        m_pPhyIspDrv_bak->destroyInstance();
    }
    else {
        ISP_FUNC_DBG("ispDrvInit,No isp driver object ");
    }

    //
    m_pP1IspDrv  = NULL;
    m_pP2IspDrv  = NULL;
    m_pPhyIspDrv  = NULL;
    //m_pCurCQIspReg=NULL;
    m_pPhyIspDrv_bak  = NULL;

    ISP_FUNC_INF("mInitCount(%d)",mInitCount);
    return MTRUE;
}


IspDrv*
IspDrvShellImp::
ispDrvGetCQIspDrv(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx)
{
    MINT32 index=m_pPhyIspDrv_bak->getRealCQIndex(cq,burstQIdx,dupCqIdx);
    return m_pVirtIspDrv[index];
}


//
MBOOL
IspDrvShellImp::
ispDrvSwitch2Virtual(MINT32 cq,MINT32 p1CqD,MINT32 burstQIdx,MINT32 p2DupCqIdx)
{
int ret = MTRUE;

    //ISP_FUNC_DBG("cq/dupCqIdx [%d]/[%d]/[%d]",cq,cqD,dupCqIdx);
    MINT32 index  =m_pPhyIspDrv_bak->getRealCQIndex(cq,burstQIdx,p2DupCqIdx);
    MINT32 indexD =m_pPhyIspDrv_bak->getRealCQIndex(p1CqD,burstQIdx,p2DupCqIdx);
    MINT32 totalCQNum=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_TOTALCQNUM);

    if (CAM_ISP_CQ_NONE != cq) {
        if (index >= totalCQNum) {
            ISP_FUNC_ERR("[Error]ispDrvSwitch2Virtual: Invalid CQ(%d). Fail", cq);
            return MFALSE;
        }
        //m_pIspDrv       = m_pVirtIspDrv[index];
        //
        if(ISP_P1_CQ(cq)) { // for P1
            m_pP1IspDrv       = m_pVirtIspDrv[index];
        } else { // for P2
            m_pP2IspDrv       = m_pVirtIspDrv[index];
            m_pP2CrzDrv->SetIspReg(m_pVirtIspReg[index]);
            m_pP2CrzDrv->SetIspDrv(m_pP2IspDrv);
        }

    }
    // for P1 only
    if (CAM_ISP_CQ_NONE != p1CqD) {
         if (indexD >= totalCQNum) {
             m_pP1IspDrvD      = NULL;
             m_pP1CurCQDIspReg = NULL;
             ISP_FUNC_ERR("[Error]ispDrvSwitch2Virtual: Invalid p1CqD(%d). Fail",p1CqD);
             return MFALSE;
         }
         m_pP1IspDrvD      = m_pVirtIspDrv[indexD];
         m_pP1CurCQDIspReg = m_pVirtIspReg[indexD];
    }

    return MTRUE;
}//

//
//
MBOOL
IspDrvShellImp::
cam_cq_cfg(
MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx,MINT32 isP2IspOnly
) {
    ISP_FUNC_DBG(":E cq(%d),isP2IspOnly(%d),burstQIdx(%d),m_trigSsrc(%d)",cq,isP2IspOnly,burstQIdx,this->m_trigSsrc);

    switch(cq) {

        case CAM_ISP_CQ0:
            ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0_BASEADDR,\
                (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            ISP_FUNC_DBG("CQ0 addr:0x%x(%d,%d)\n",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),burstQIdx,dupCqIdx);
            //ISP_FUNC_DBG("CAM_ISP_CQ0 IspDrvShellImp:: CQ0C(0x%x)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,dupCqIdx));
            //ISP_FUNC_DBG("CAM_ISP_CQ0 REG(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv_bak, CAM_CTL_CQ0_BASEADDR));
            //
            //m_pPhyIspDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_IMMEDIATE_TRIGGER,CQ_TRIG_BY_START);
            break;
        case CAM_ISP_CQ0B:
            ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0B_BASEADDR,\
                (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            //
            //m_pPhyIspDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_EVENT_TRIGGER,CQ_TRIG_BY_PASS1_DONE);
            break;
        case CAM_ISP_CQ0C:
            ISP_FUNC_DBG("CAM_ISP_CQ0C(0x%x),m_trigSsrc(%d)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),this->m_trigSsrc);

            //set base addr in isp_drv cqRingBuf()
            //ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0C_BASEADDR,\
            //(unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq),ISP_DRV_USER_ISPF);
            //
            m_pPhyIspDrv->setCQTriggerMode(ISP_DRV_CQ0C,CQ_SINGLE_EVENT_TRIGGER,(ISP_DRV_CQ_TRIGGER_SOURCE_ENUM)this->m_trigSsrc);
            //ISP_FUNC_DBG("CAM_ISP_CQ0C IspDrvShellImp:: CQ0C(0x%x)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,dupCqIdx));
            //ISP_FUNC_DBG("CAM_ISP_CQ0C REG(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv_bak, CAM_CTL_CQ0C_BASEADDR));

            break;
        case CAM_ISP_CQ0_D:
            ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0_D_BASEADDR,\
                (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            ISP_FUNC_DBG("CQ0_D addr:0x%x(%d,%d)\n",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),burstQIdx,dupCqIdx);
            //ISP_FUNC_DBG("CAM_ISP_CQ0_D IspDrvShellImp:: CQ0C(0x%x)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,dupCqIdx));
            //ISP_FUNC_DBG("CAM_ISP_CQ0_D REG(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv_bak, CAM_CTL_CQ0_D_BASEADDR));

            break;
        case CAM_ISP_CQ0B_D:
            ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0B_D_BASEADDR,\
                (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            break;
        case CAM_ISP_CQ0C_D:
            //ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0C_D_BASEADDR,\
            //    (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,dupCqIdx),ISP_DRV_USER_ISPF);
            m_pPhyIspDrv->setCQTriggerMode(ISP_DRV_CQ0C_D,CQ_SINGLE_EVENT_TRIGGER,(ISP_DRV_CQ_TRIGGER_SOURCE_ENUM)this->m_trigSsrc);
            //ISP_FUNC_DBG("CAM_ISP_CQ0C_D IspDrvShellImp:: CQ0C(0x%x)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,dupCqIdx));
            //ISP_FUNC_DBG("CAM_ISP_CQ0C_D REG(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv_bak, CAM_CTL_CQ0C_D_BASEADDR));
            break;
        case CAM_ISP_CQ1:
            if(isP2IspOnly){
                ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ1_BASEADDR,\
                    (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            }
            break;
        case CAM_ISP_CQ2:
            if(isP2IspOnly){
                ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ2_BASEADDR,\
                    (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            }
            break;
        case CAM_ISP_CQ3:
            if(isP2IspOnly){
                ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ3_BASEADDR,\
                    (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            }
            break;
        default:
            ISP_FUNC_WRN("[Warning]cq(%d) index",cq);
            break;
        }

    return MTRUE;
}
//
MBOOL
IspDrvShellImp::
cqAddModule(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    //ISP_FUNC_DBG(":E m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x) ,%d",m_pIspDrv,m_pPhyIspDrv_bak,moduleId);
    //
    m_pPhyIspDrv->cqAddModule(cq,burstQIdx,dupCqIdx, moduleId);
    //ISP_FUNC_DBG(":X ");
    return MTRUE;
}

int
IspDrvShellImp::
getCqModuleInfo(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    return m_pPhyIspDrv->getCqModuleInfo( cq,burstQIdx,dupCqIdx, moduleId);
}

//
MBOOL
IspDrvShellImp::
cqDelModule(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    //ISP_FUNC_DBG(":E m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x) ",m_pIspDrv,m_pPhyIspDrv_bak);
    //
    m_pPhyIspDrv->cqDelModule( cq,burstQIdx,dupCqIdx, moduleId);
    //ISP_FUNC_DBG(":X ");
    return MTRUE;
}
//
MBOOL
IspDrvShellImp::
setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src) // for p1 only
{
    ISP_FUNC_DBG("+,m_pPhyIspDrv(0x%x) ",m_pPhyIspDrv_bak);
    //
    m_pPhyIspDrv_bak->setCQTriggerMode((ISP_DRV_CQ_ENUM) cq,
                                (ISP_DRV_CQ_TRIGGER_MODE_ENUM) mode,
                                (ISP_DRV_CQ_TRIGGER_SOURCE_ENUM) trig_src);
    //cq0b only for AE smoothing
    if(cq == CAM_ISP_CQ0B){
        IspDrv* _targetVirDrv = this->ispDrvGetCQIspDrv(CAM_ISP_CQ0B,0, 0);
        _targetVirDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM) cq, \
                    (ISP_DRV_CQ_TRIGGER_MODE_ENUM)mode, \
                    (ISP_DRV_CQ_TRIGGER_SOURCE_ENUM)trig_src);
    }
    ISP_FUNC_DBG("-,");

    return MTRUE;
}

//
// this function should be called before isp configure, due to magic# syn
//
MBOOL
IspDrvShellImp::
getNr3dGain( ESoftwareScenario softScenario, MINT32 magicNum, MINT32 &nr3dGain)
{
    MUINT32* pTuningBuf;
    ISP_DRV_CQ_ENUM cq;

    ISP_FUNC_DBG("softScenario(%d),magicNum(%d)",softScenario,magicNum);

    m_pPhyIspDrv->getCqInfoFromScenario(softScenario, cq);

    if(m_pPhyIspDrv->deTuningQue(eTuningQueAccessPath_imageio_P2,softScenario,magicNum)==MFALSE){
        ISP_FUNC_ERR("[Error]deTuningQue fail");
        return MFALSE;
    }

    pTuningBuf = m_pPhyIspDrv->getTuningBuf(eTuningQueAccessPath_imageio_P2, cq);
    nr3dGain = (pTuningBuf[CAM_NR3D_BLEND_OFFSET>>2]>>CAM_NR3D_GAIN_BIT_SHIFT)&CAM_NR3D_GAIN_BIT_MASK;

    if(m_pPhyIspDrv->enTuningQue(eTuningQueAccessPath_imageio_P2,softScenario,magicNum,eIspTuningMgrFunc_Null)==MFALSE){
        ISP_FUNC_ERR("[Error]enTuningQue fail");
        return MFALSE;
    }

    ISP_FUNC_DBG("nr3dGain(%d)",nr3dGain);

    return MTRUE;
}

void
IspDrvShellImp::
CQBufferMutexLock( void )
{
    this->mCQBufferLock.lock();
}

void
IspDrvShellImp::
CQBufferMutexUnLock( void )
{
    this->mCQBufferLock.unlock();
}


//
//this function should be called before isp start
//
MBOOL
IspDrvShellImp::
updateBurstQueueNum(MINT32 burstQNum)
{
    MBOOL ret=MTRUE;
    Mutex::Autolock lock(mCQBufferLock);

    //[1] free cur cq and mdp/tpipe/virtual isp buffer in isp_function
    MINT32 oldBurstQNum=1;
    oldBurstQNum=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_CURBURSTQNUM);
    ISP_FUNC_INF("3 oldBurstQNum(%d) ",oldBurstQNum);
    if(burstQNum==oldBurstQNum)
    { return MTRUE;}

    //do the following procedure if the burstQnum changes
    ret=m_pPhyIspDrv_bak->cqTableControl(ISP_DRV_CQTABLE_CTRL_DEALLOC,oldBurstQNum);
    if(!ret)
    {
        ISP_FUNC_ERR("free cq table in isp driver fail");
        return MFALSE;
    }
    ret=controlBurstQueBuffer(eIspControlBufferCmd_Free,oldBurstQNum);
    if(!ret)
    {
        ISP_FUNC_ERR("free mdp/tpipe/virtual isp buffer fail");
        return MFALSE;
    }

    //[2] update new info
    ret=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_SET_CURBURSTQNUM,burstQNum);
    ret=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_UPDATE_TOTALCQNUM);
    //[3] re-alloc cq related buffer in isp driver
    ret=m_pPhyIspDrv_bak->cqTableControl(ISP_DRV_CQTABLE_CTRL_ALLOC,burstQNum);
    if(!ret)
    {
        ISP_FUNC_ERR("alloc cq table in isp driver fail");
        return MFALSE;
    }
    //[4] re-alloc mdp/tpipe/virtual isp buffer in isp_function
    ret=controlBurstQueBuffer(eIspControlBufferCmd_Alloc,burstQNum);
    if(!ret)
    {
        ISP_FUNC_ERR("alloc mdp/tpipe/virtual isp buffer fail");
        return MFALSE;
    }
    return ret;
}

//
MBOOL
IspDrvShellImp::
controlBurstQueBuffer(EIspControlBufferCmd cmd, MUINT32 burstQueueNum)
{
    MBOOL ret=MTRUE;
    MUINT32 i, j, k;
    MDPMGR_CFG_STRUCT **pDataLayer1;
    MDPMGR_CFG_STRUCT *pDataLayer2;

    ISP_FUNC_INF("cmd(%d)",cmd);
    if(eIspControlBufferCmd_Alloc == cmd) {
        /*============================================
         MDP Buffer
        =============================================*/
        gMdpBufInfo.size = (burstQueueNum*sizeof(int**)+burstQueueNum*ISP_DRV_P2_CQ_NUM*sizeof(int*) + burstQueueNum*ISP_DRV_P2_CQ_NUM*ISP_DRV_P2_CQ_DUPLICATION_NUM*sizeof(MDPMGR_CFG_STRUCT))+(7+1)+0x100;
        ISP_FUNC_DBG("alloc mdpBuf size(%d)",gMdpBufInfo.size);
        #if defined(__PMEM_ONLY__)
        gMdpBufInfo.virtAddr = (MUINT32*)pmem_alloc_sync( gMdpBufInfo.size, &gMdpBufInfo.memID);
        #else   // Not PMEM.
        gMdpBufInfo.useNoncache = 0; //alloc cacheable mem.
        if ( m_pIMemDrv->allocVirtBuf(&gMdpBufInfo) ) {
            ISP_FUNC_ERR("[ERROR]allocVirtBuf(mdpBufInfo) fail,size(0x%x)",gMdpBufInfo.size);
        }
        gpMdpMgrCfgData = (MDPMGR_CFG_STRUCT ***)((gMdpBufInfo.virtAddr+7+0x100)&(~7));
        pDataLayer1 = (MDPMGR_CFG_STRUCT**)(gpMdpMgrCfgData+burstQueueNum);
        pDataLayer2 = (MDPMGR_CFG_STRUCT*)(gpMdpMgrCfgData+burstQueueNum+ISP_DRV_P2_CQ_NUM*burstQueueNum);
        for(i=0;i<burstQueueNum;i++){
            gpMdpMgrCfgData[i] = pDataLayer1;
            for(j=0;j<ISP_DRV_P2_CQ_NUM;j++){
                gpMdpMgrCfgData[i][j] = pDataLayer2;
                pDataLayer2 += ISP_DRV_P2_CQ_DUPLICATION_NUM;
            }
            pDataLayer1 += (ISP_DRV_P2_CQ_NUM);
        }
        #endif  // __PMEM_ONLY__
        /*============================================
         Tpipe Buffer
        =============================================*/
        for(k=0;k<burstQueueNum;k++){
            for(i=0;i<ISP_DRV_P2_CQ_NUM;i++){
                for(j=0;j<ISP_DRV_P2_CQ_DUPLICATION_NUM;j++){
                    #if defined(__PMEM_ONLY__)
                    gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = (MUINT32*)pmem_alloc_sync(MAX_ISP_TILE_TDR_HEX_NO, &tpipeBufInfo[k][i][j].memID);
                    gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa = (MUINT32)pmem_get_phys(tpipeBufInfo[k][i][j].memID);
                    #else   // Not PMEM.
                    //
                    tpipeBufInfo[k][i][j].size = MAX_ISP_TILE_TDR_HEX_NO;
                    tpipeBufInfo[k][i][j].useNoncache = 1; //alloc non-cacheable mem.
                    //
                    if ( m_pIMemDrv->allocVirtBuf(&tpipeBufInfo[k][i][j]) ) {
                        ISP_FUNC_ERR("ERROR:m_pIMemDrv->allocVirtBuf(tpipeBufInfo)");
                    }
                    gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = (MUINT32*)( (tpipeBufInfo[k][i][j].virtAddr+7) & (~7) );
                    //
                    if ( m_pIMemDrv->mapPhyAddr(&tpipeBufInfo[k][i][j]) ) {
                        ISP_FUNC_ERR("ERROR:m_pIMemDrv->mapPhyAddr(tpipeBufInfo)");
                    }
                    //CQ decriptor base address should be 8 bytes alignment
                    gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa =  ( (tpipeBufInfo[k][i][j].phyAddr+7) & (~7) );
                    ISP_FUNC_INF("[Tpipe]i(%d),j(%d)va(0x%08x),pa(0x%08x)",i,j,gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa,gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa);
                    #endif
                }
            }
        }
        /*============================================
         virtual isp driver for CQ
        =============================================*/
        //[1] free previous memory if needed
        if(m_pVirtIspDrv!=NULL)
        {
            free(m_pVirtIspDrv);
        }
        if(m_pVirtIspReg!=NULL)
        {
            free(m_pVirtIspReg);
        }
        //[2]get current total cq num
        MINT32 totalCQNum=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_TOTALCQNUM);
        //[3]allocate virtual isp
        m_pVirtIspDrv=(IspDrv**)malloc(totalCQNum*sizeof(IspDrv *)+totalCQNum*1*sizeof(IspDrv));
        for( int i=0; i<totalCQNum; i++ )
        {
            m_pVirtIspDrv[i] = ((IspDrv *)(m_pVirtIspDrv+totalCQNum)) + i*1*sizeof(IspDrv);
        }
        if(m_pVirtIspDrv==NULL)
        {
            ISP_FUNC_ERR("malloc m_pVirtIspDrv fail");
            ret = false;
            return ret;
        }
        //allocate virtual register map in virtual isp
        m_pVirtIspReg=(isp_reg_t**)malloc(totalCQNum*sizeof(isp_reg_t *)+totalCQNum*1*sizeof(isp_reg_t));
        for( int i=0; i<totalCQNum; i++ )
        {
            m_pVirtIspReg[i] = ((isp_reg_t *)(m_pVirtIspReg+totalCQNum)) + i*1*sizeof(isp_reg_t);
        }
        if(m_pVirtIspReg==NULL)
        {
            ISP_FUNC_ERR("malloc m_pVirtIspReg fail");
            ret = false;
            return ret;
        }
        //virtual isp/register map initialization
        for(int cq=0;cq<totalCQNum;cq++)
        {
            //virtual isp
            m_pVirtIspDrv[cq] = m_pPhyIspDrv_bak->getCQInstance((ISP_DRV_CQ_ENUM)cq);
            ISP_FUNC_DBG("[m_pVirtIspDrv[cq%d]]:0x%08x ",cq,m_pVirtIspDrv[cq]);
            if (!m_pVirtIspDrv[cq]) {
                ISP_FUNC_ERR("g_pVirtIspDrv::createInstance fail ");
                return ret;
            }
            //
            ret = m_pVirtIspDrv[cq]->init();
            if (ret < 0) {
                ISP_FUNC_ERR("g_pVirtIspDrv[%d]->init() fail ",cq);
                return ret;
            }
            //virtual register map
            m_pVirtIspReg[cq] = (isp_reg_t*)(m_pVirtIspDrv[cq]->getRegAddr());
            ISP_FUNC_DBG("[m_pVirtIspReg[cq%d]]:0x%08x ",cq,m_pVirtIspReg[cq]);
            if ( NULL == m_pVirtIspReg[cq] ) {
                ISP_FUNC_ERR("getVirRegAddr fail ");
                return -1;
            }
        }
    } else {
        /*============================================
         Tpipe Buffer
        =============================================*/
        for(k=0;k<burstQueueNum;k++){
            for(i=0;i<ISP_DRV_P2_CQ_NUM;i++){
                for(j=0;j<ISP_DRV_P2_CQ_DUPLICATION_NUM;j++){
                    #if defined(__PMEM_ONLY__)
                    if(tpipeBufInfo[k][i][j].memID >= 0) {
                        pmem_free((MUINT8*)gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa,(MAX_ISP_TILE_TDR_HEX_NO, tpipeBufInfo[k][i][j].memID);
                        tpipeBufInfo[k][i][j].memID = -1;
                        gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = NULL;
                        gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa = 0;
                    }
                    #else   // Not PMEM.
                    //
                    if ( m_pIMemDrv->unmapPhyAddr(&tpipeBufInfo[k][i][j]) ) {
                        ISP_FUNC_ERR("ERROR:m_pIMemDrv->unmapPhyAddr (tpipeBufInfo)");
                    }
                    //LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->unmapPhyAddr(tpipeBufInfo)");
                    //
                    if ( m_pIMemDrv->freeVirtBuf(&tpipeBufInfo[k][i][j]) ) {
                        ISP_FUNC_ERR("ERROR:m_pIMemDrv->freeVirtBuf(tpipeBufInfo)");
                    }
                    gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = NULL;
                    gpMdpMgrCfgData[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa = 0;
                    //LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->freeVirtBuf(tpipeBufInfo)");
                    ISP_FUNC_DBG("free/unmap tpipeBufInfo");
                    #endif  // __PMEM_ONLY__
                }
            }
        }
        /*============================================
         MDP Buffer
        =============================================*/
        #if defined(__PMEM_ONLY__)
        if(gMdpBufInfo.memID >= 0) {
            pmem_free((MUINT8*)gMdpBufInfo.virtAddr, gMdpBufInfo.size, gMdpBufInfo.memID);
            gMdpBufInfo.memID = -1;
        }
        gpMdpMgrCfgData = NULL;
        gMdpBufInfo.size = 0;
        #else   // Not PMEM.
        if ( m_pIMemDrv->freeVirtBuf(&gMdpBufInfo) ) {
            ISP_FUNC_ERR("[ERROR]freeVirtBuf(mdpBufInfo) fail,size(0x%x)",gMdpBufInfo.size);
        }
        gpMdpMgrCfgData = NULL;
        gMdpBufInfo.size = 0;
        ISP_FUNC_DBG("free/unmap mdpBufInfo");
        #endif  // __PMEM_ONLY__
        /*============================================
         virtual isp driver for CQ
        =============================================*/
        MINT32 cqtotalNum=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_TOTALCQNUM);
        for (int i = 0; i<cqtotalNum; i++ ) {
            if (m_pVirtIspDrv[i]) {
                ret = m_pVirtIspDrv[i]->uninit();
                if (ret < 0) {
                    ISP_FUNC_ERR("g_pVirtIspDrv[%d]->uninit() fail ",i);
                    return ret;
                }
            }
            else {
                ISP_FUNC_DBG("g_pVirtIspDrv->uninit(?,No isp driver object ");
            }
        }
        if(m_pVirtIspDrv!=NULL)
        {
            free(m_pVirtIspDrv);
            m_pVirtIspDrv=NULL;
        }
        if(m_pVirtIspReg!=NULL)
        {
            free(m_pVirtIspReg);
            m_pVirtIspReg=NULL;
        }
    }

    return MTRUE;
}

/*/////////////////////////////////////////////////////////////////////////////
    IspFunction_B
  /////////////////////////////////////////////////////////////////////////////*/
unsigned long   IspFunction_B::m_Isp_Top_Reg_Base_Addr = MT6593_ISP_TOP_BASE; /*Use physical base address as default address*/
IspDrvShell*    IspFunction_B::m_pIspDrvShell = NULL;
IspDrv*         IspFunction_B::m_pP1IspDrv = NULL; // avoid to use this global para, can't used under multi-thread with p1 cmdQCtrl
IspDrv*         IspFunction_B::m_pP2IspDrv = NULL;
//IspDrv*         IspFunction_B::m_pIspDrv = NULL;
IspDrv*         IspFunction_B::m_pPhyIspDrv = NULL;
//isp_reg_t*       IspFunction_B::m_pCurCQIspReg = NULL;
MUINT32         IspFunction_B::m_fps = 30;
MUINT32         IspFunction_B::m_bKDBG = 0;
#define         FPS_BOUNDARY    90


unsigned long IspFunction_B::dec_Id( void )//Decimal ID number
{
    unsigned long dec_Id = 0;
    unsigned long id = this->id();  //virtual

    while( id )
    {
        dec_Id++;
        id = id >> 1;
    }

    return (dec_Id-1);
}

int IspFunction_B::config( void )
{
    int retval;
    //ISP_FUNC_DBG("<%2d> config Pa=0x%x Va=0x%x",(int)this->dec_Id(), (unsigned int)this->reg_Base_Addr_Phy(), (unsigned int)reg_Base_Addr() );
    retval = this->_config();       //virtual
    if( retval != 0 )   return retval;
    return 0;
}

int IspFunction_B::enable( void* pParam )
{
    int retval;

    //ISP_FUNC_DBG("<%2d> enable",(int)this->dec_Id());
    retval = this->_enable(pParam);       //virtual
    if( retval != 0 )   return retval;
    return 0;
}

int IspFunction_B::disable( void )
{
    int retval;
    //ISP_FUNC_DBG("<%2d> disable",(int)this->dec_Id());
    retval = this->_disable();       //virtual
    if( retval != 0 )   return retval;
    return 0;
}

int IspFunction_B::write2CQ(void)
{
int retval;
    //ISP_FUNC_DBG("<%s> write2CQ",this->name_Str());
    retval = this->_write2CQ();       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("this->_write2CQ fail ");
    }
    return retval;

}
//
int IspFunction_B::setZoom( void )
{
    int retval;
    //ISP_FUNC_DBG("<%2d> config Pa=0x%x Va=0x%x",(int)this->dec_Id(), (unsigned int)this->reg_Base_Addr_Phy(), (unsigned int)reg_Base_Addr() );
    retval = this->_setZoom();       //virtual
    if( retval != 0 )   return retval;
    return 0;
}

//
MBOOL
IspFunction_B::
ispDrvSwitch2Virtual(MINT32 cq,MINT32 p1CqD,MINT32 burstQIdx,MINT32 P2DupCqIdx)
{
   //Mutex::Autolock lock(mLock);

    //ISP_FUNC_DBG("E cq(%d,%d) ",cq, cqD);
    this->m_pIspDrvShell->CQBufferMutexLock();
    this->m_pIspDrvShell->ispDrvSwitch2Virtual(cq, p1CqD,burstQIdx, P2DupCqIdx);
    //m_pIspDrv  = this->m_pIspDrvShell->m_pIspDrv;
    //m_pCurCQIspReg  = this->m_pIspDrvShell->m_pCurCQIspReg;

    if ( cq==CAM_ISP_CQ1 || cq==CAM_ISP_CQ2 || cq==CAM_ISP_CQ3){
        m_pP2IspDrv  = this->m_pIspDrvShell->m_pP2IspDrv;
    } else if( cq==CAM_ISP_CQ0 || cq==CAM_ISP_CQ0B || cq==CAM_ISP_CQ0C) {
        m_pP1IspDrv  = this->m_pIspDrvShell->m_pP1IspDrv;
    } else if(p1CqD==CAM_ISP_CQ0_D || p1CqD==CAM_ISP_CQ0B_D || p1CqD==CAM_ISP_CQ0C_D) {
        m_pP1IspDrvD      = this->m_pIspDrvShell->m_pP1IspDrvD;
        m_pP1CurCQDIspReg = this->m_pIspDrvShell->m_pP1CurCQDIspReg;
    } else {
        ISP_FUNC_ERR("[Error]incorrect parameter cq(%d),p1CqD(%d)",cq,p1CqD);
    }
    this->m_pIspDrvShell->CQBufferMutexUnLock();

    //ISP_FUNC_DBG("X ");
    return MTRUE;
}
//
MBOOL
IspFunction_B::
getEnTuningTag
        (MBOOL isTopEngine,
        ISP_DRV_CQ_ENUM cq,
        MUINT32 drvScenario,
        MUINT32 subMode,
        MUINT32 &tuningEn1Tag,
        MUINT32 &tuningEn2Tag,
        MUINT32 &tuningDmaTag)
{
    MBOOL ret = MTRUE;
    ISP_FUNC_DBG("+,isTopEngine(%d),cq(%d),drvScenario(%d),subMode(%d)",
            isTopEngine,cq,drvScenario,subMode);
    //
    // check and set tuning en and DMA tag
    switch (drvScenario){
            case NSImageio::NSIspio::eDrvScenario_CC_vFB_Normal:
            case NSImageio::NSIspio::eDrvScenario_CC_vFB_FB:
            case NSImageio::NSIspio::eDrvScenario_CC_CShot:
            case NSImageio::NSIspio::eDrvScenario_CC_MFB_Blending:
            case NSImageio::NSIspio::eDrvScenario_CC_MFB_Mixing:
            case NSImageio::NSIspio::eDrvScenario_CC:
            case NSImageio::NSIspio::eDrvScenario_CC_SShot:
            case NSImageio::NSIspio::eDrvScenario_VSS:
            if(isTopEngine==MTRUE){
                tuningEn1Tag = gIspTurningTopTag_cc[subMode].cam_ctl_en_p1;
                tuningEn2Tag = gIspTurningTopTag_cc[subMode].cam_ctl_en_p2;
                tuningDmaTag = gIspTurningTopTag_cc[subMode].cam_ctl_en_p1_dma;
            } else {
                tuningEn1Tag = gIspTurningFieldTag_cc[subMode].cam_ctl_en_p1;
                tuningEn2Tag = gIspTurningFieldTag_cc[subMode].cam_ctl_en_p2;
                tuningDmaTag = gIspTurningFieldTag_cc[subMode].cam_ctl_en_p1_dma;
            }
            break;
            case NSImageio::NSIspio::eDrvScenario_IP:
            case NSImageio::NSIspio::eDrvScenario_CC_RAW:
            if(isTopEngine==MTRUE){
                tuningEn1Tag = gIspTurningTopTag_ip[subMode].cam_ctl_en_p1;
                tuningEn2Tag = gIspTurningTopTag_ip[subMode].cam_ctl_en_p2;
                tuningDmaTag = gIspTurningTopTag_ip[subMode].cam_ctl_en_p1_dma;
            } else {
                tuningEn1Tag = gIspTurningFieldTag_ip[subMode].cam_ctl_en_p1;
                tuningEn2Tag = gIspTurningFieldTag_ip[subMode].cam_ctl_en_p2;
                tuningDmaTag = gIspTurningFieldTag_ip[subMode].cam_ctl_en_p1_dma;
            }
            break;
        default:
            tuningEn1Tag = 0x00;
            tuningEn2Tag = 0x00;
            tuningDmaTag = 0x00;
            ISP_FUNC_ERR("[Error]Not support this drvScenario(%d)",drvScenario);
            ret = MFALSE;
            break;
    }
    if(subMode >= ISP_SUB_MODE_MAX){
        tuningEn1Tag = 0x00;
        tuningEn2Tag = 0x00;
        tuningDmaTag = 0x00;
        ISP_FUNC_ERR("[Error]Not support this subMode(%d)",subMode);
        ret = MFALSE;
    }

    ISP_FUNC_DBG("tunStas(%d),drvScenario(%d),subMode(%d),tuningEn1Tag(0x%08x),tuningEn2Tag(0x%08x),tuningDmaTag(0x%08x)",m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)cq),drvScenario,subMode,tuningEn1Tag,tuningEn2Tag,tuningDmaTag);

    return ret;
}

MBOOL
IspFunction_B::
getP1EnTuningTag
        (MBOOL isTopEngine,
        ISP_DRV_CQ_ENUM cq,
        MUINT32 magicNum,
        MUINT32 drvScenario,
        MUINT32 subMode,
        MUINT32 &tuningEnTag,
        MUINT32 &tuningDmaTag)
{
    MBOOL ret = MTRUE;
    //
    if (m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)cq)) {
        // check and set tuning en and DMA tag
        switch (drvScenario){
            case NSImageio::NSIspio::eDrvScenario_CC_vFB_Normal:
            case NSImageio::NSIspio::eDrvScenario_CC_vFB_FB:
            case NSImageio::NSIspio::eDrvScenario_CC_CShot:
            case NSImageio::NSIspio::eDrvScenario_CC_MFB_Blending:
            case NSImageio::NSIspio::eDrvScenario_CC_MFB_Mixing:
            case NSImageio::NSIspio::eDrvScenario_CC:
            case NSImageio::NSIspio::eDrvScenario_CC_SShot:
            case NSImageio::NSIspio::eDrvScenario_VSS:
            case NSImageio::NSIspio::eDrvScenario_VSS_MFB_Blending:
            case NSImageio::NSIspio::eDrvScenario_VSS_MFB_Mixing:
                if(isTopEngine==MTRUE){
                    tuningEnTag = gIspTurningTopTag_cc[subMode].cam_ctl_en_p1;
                    tuningDmaTag = gIspTurningTopTag_cc[subMode].cam_ctl_en_p1_dma;
                } else {
                    tuningEnTag = gIspTurningFieldTag_cc[subMode].cam_ctl_en_p1;
                    tuningDmaTag = gIspTurningFieldTag_cc[subMode].cam_ctl_en_p1_dma;
                }
                break;
            case NSImageio::NSIspio::eDrvScenario_IP:
            case NSImageio::NSIspio::eDrvScenario_CC_RAW:
                if(isTopEngine==MTRUE){
                    tuningEnTag = gIspTurningTopTag_ip[subMode].cam_ctl_en_p1;
                    tuningDmaTag = gIspTurningTopTag_ip[subMode].cam_ctl_en_p1_dma;
                } else {
                    tuningEnTag = gIspTurningFieldTag_ip[subMode].cam_ctl_en_p1;
                    tuningDmaTag = gIspTurningFieldTag_ip[subMode].cam_ctl_en_p1_dma;
                }
                break;
            default:
                tuningEnTag = 0x00;
                tuningDmaTag = 0x00;
                ISP_FUNC_ERR("[Error]Not support this drvScenario(%d)",drvScenario);
                ret = MFALSE;
                break;
        }
        if(subMode >= ISP_SUB_MODE_MAX){
            tuningEnTag = 0x00;
            tuningDmaTag = 0x00;
            ISP_FUNC_ERR("[Error]Not support this subMode(%d)",subMode);
            ret = MFALSE;
        }
    } else {
        tuningEnTag = 0x00;
        tuningDmaTag = 0x00;
    }
    ISP_FUNC_DBG("tunStas(%d),drvScenario(%d),subMode(%d),tuningEnTag(0x%08x),tuningDmaTag(0x%08x)",m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)cq),drvScenario,subMode,tuningEnTag,tuningDmaTag);

    return ret;
}


//
MBOOL
IspFunction_B::
getP2EnTuningTag
        (MBOOL isTopEngine,
        ISP_DRV_CQ_ENUM cq,
        MUINT32 magicNum,
        MUINT32 drvScenario,
        MUINT32 subMode,
        MUINT32 &tuningEnTag,
        MUINT32 &tuningDmaTag)
{
    MBOOL ret = MTRUE;
    //
    if (m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)cq)) {
        // check and set tuning en and DMA tag
        switch (drvScenario){
            case NSImageio::NSIspio::eDrvScenario_CC_vFB_Normal:
            case NSImageio::NSIspio::eDrvScenario_CC_CShot:
            case NSImageio::NSIspio::eDrvScenario_CC_MFB_Blending:
            case NSImageio::NSIspio::eDrvScenario_CC_MFB_Mixing:
            case NSImageio::NSIspio::eDrvScenario_CC:
            case NSImageio::NSIspio::eDrvScenario_CC_SShot:
            case NSImageio::NSIspio::eDrvScenario_VSS:
            case NSImageio::NSIspio::eDrvScenario_VSS_MFB_Blending:
            case NSImageio::NSIspio::eDrvScenario_VSS_MFB_Mixing:
                if(isTopEngine==MTRUE){
                    tuningEnTag = gIspTurningTopTag_cc[subMode].cam_ctl_en_p2;
                    tuningDmaTag = gIspTurningTopTag_cc[subMode].cam_ctl_en_p2_dma;
                } else {
                    tuningEnTag = gIspTurningFieldTag_cc[subMode].cam_ctl_en_p2;
                    tuningDmaTag = gIspTurningFieldTag_cc[subMode].cam_ctl_en_p2_dma;
                }
                break;
            case NSImageio::NSIspio::eDrvScenario_IP:
                if(isTopEngine==MTRUE){
                    tuningEnTag = gIspTurningTopTag_ip[subMode].cam_ctl_en_p2;
                    tuningDmaTag = gIspTurningTopTag_ip[subMode].cam_ctl_en_p2_dma;
                } else {
                    tuningEnTag = gIspTurningFieldTag_ip[subMode].cam_ctl_en_p2;
                    tuningDmaTag = gIspTurningFieldTag_ip[subMode].cam_ctl_en_p2_dma;
                }
                break;
            case NSImageio::NSIspio::eDrvScenario_CC_vFB_FB:
                if(isTopEngine==MTRUE){
                    tuningEnTag = gIspTurningTopTag_fb[subMode].cam_ctl_en_p2;
                    tuningDmaTag = gIspTurningTopTag_fb[subMode].cam_ctl_en_p2_dma;
                } else {
                    tuningEnTag = gIspTurningFieldTag_fb[subMode].cam_ctl_en_p2;
                    tuningDmaTag = gIspTurningFieldTag_fb[subMode].cam_ctl_en_p2_dma;
                }
                break;
            default:
                tuningEnTag = 0x00;
                tuningDmaTag = 0x00;
                ISP_FUNC_ERR("[Error]Not support this drvScenario(%d)",drvScenario);
                ret = MFALSE;
                break;
        }
        if(subMode >= ISP_SUB_MODE_MAX){
            tuningEnTag = 0x00;
            tuningDmaTag = 0x00;
            ISP_FUNC_ERR("[Error]Not support this subMode(%d)",subMode);
            ret = MFALSE;
        }
    } else {
        tuningEnTag = 0x00;
        tuningDmaTag = 0x00;
    }
    ISP_FUNC_DBG("tunStas(%d),drvScenario(%d),subMode(%d),tuningEnTag(0x%08x),tuningDmaTag(0x%08x)",m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)cq),drvScenario,subMode,tuningEnTag,tuningDmaTag);

    return ret;
}




//
/*/////////////////////////////////////////////////////////////////////////////
    DMAI_B
  /////////////////////////////////////////////////////////////////////////////*/
int DMAI_B::_config( void )
{
    IspDrv* _targetVirDrv = NULL;
    unsigned long pBase = 0;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MINT32 index=0;
    MUINT32 cq = 0;
    MUINT32 _targetDupQIdx = 0;

    ISP_FUNC_DBG("DMAI(%d),[%s]:m_pP2IspDrv(0x%08x),pa(0x%08X),ofst(0x%08X),size(%d,%d,%d,%d),pixel_byte(%d),crop(%d,%d), flip(%d),isTwin(%d),CQ(0x%x/0x%x) ", \
        this->id(), \
        this->name_Str(), \
        m_pP2IspDrv, \
        this->dma_cfg.memBuf.base_pAddr, \
        this->dma_cfg.memBuf.ofst_addr,
        this->dma_cfg.size.w, \
        this->dma_cfg.size.h, \
        this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
        this->dma_cfg.pixel_byte, \
        this->dma_cfg.crop.x, \
        this->dma_cfg.crop.y, \
        this->dma_cfg.v_flip_en,\
        this->isTwinMode,\
        this->CQ,this->CQ_D);

#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(this->CQ == CAM_ISP_CQ0){
            if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
                _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
            else
                _targetDupQIdx = this->dupCqIdx;
        }
        else if(this->CQ_D == CAM_ISP_CQ0_D){
            if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN) == 1)
                _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
            else
                _targetDupQIdx = this->dupCqIdx;
        }
        else
            _targetDupQIdx = this->dupCqIdx;
#else
        _targetDupQIdx = this->dupCqIdx;
#endif
    switch(this->id())
    {
        case ISP_DMA_TDRI:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_TDRI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            //
            ISP_FUNC_DBG("CAM_TDRI_BASE_ADDR = 0x%x",this->dma_cfg.memBuf.base_pAddr);
            break;
        case ISP_DMA_CQI:
            if (this->CQ != CAM_ISP_CQ_NONE) {
               this->m_pIspDrvShell->m_trigSsrc = this->cqTrigSrc;
               this->m_pIspDrvShell->cam_cq_cfg(this->CQ,this->burstQueIdx,this->dupCqIdx,this->isP2IspOnly);
            }
            if (this->CQ_D != CAM_ISP_CQ_NONE) {
               this->m_pIspDrvShell->m_trigSsrc = this->cqTrigSrc_D;
               this->m_pIspDrvShell->cam_cq_cfg(this->CQ_D,this->burstQueIdx,this->dupCqIdx,this->isP2IspOnly);
            }
            this->ispDrvSwitch2Virtual(this->CQ, this->CQ_D,this->burstQueIdx,this->dupCqIdx);
            ISP_FUNC_DBG("[afterCQSwitch]m_pP2IspDrv(0x%x)",m_pP2IspDrv);
            break;
        case ISP_DMA_UFDI:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_UFDI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_UFDI_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_UFDI_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_UFDI_STRIDE,SWAP,this->dma_cfg.swap,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_UFDI_STRIDE,FORMAT_EN,this->dma_cfg.format_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_UFDI_STRIDE,FORMAT,this->dma_cfg.format,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_UFDI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_UFDI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_UFDI_STRIDE,STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_UFDI_CON,0x80202020,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_UFDI_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //
            ISP_FUNC_DBG("CAM_UFDI_BASE_ADDR=[0x%08X] ",this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
            break;
        case ISP_DMA_BPCI:
            if(this->m_pIspDrvShell == NULL){
                ISP_FUNC_ERR("m_pIspDrvShell is NULL\n");
                return -1;
            }
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);

            //currently , slow motion will map to this case
            if(this->m_fps > FPS_BOUNDARY){
            }
            break;
        case ISP_DMA_BPCI_D:
            if(this->m_pIspDrvShell == NULL){
                ISP_FUNC_ERR("m_pIspDrvShell is NULL\n");
                return -1;
            }
            if(this->isTwinMode)
                _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            else
                _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_BPCI_D_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_LSCI:
            if(this->m_pIspDrvShell == NULL){
                ISP_FUNC_ERR("m_pIspDrvShell is NULL\n");
                return -1;
            }
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);

            //currently , slow motion will map to this case
            if(this->m_fps > FPS_BOUNDARY){
            }
            break;
           case ISP_DMA_LSCI_D:
               if(this->m_pIspDrvShell == NULL){
                ISP_FUNC_ERR("m_pIspDrvShell is NULL\n");
                return -1;
            }
            if(this->isTwinMode)
                _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            else
                _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_D_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
               break;
        case ISP_DMA_LCEI:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_LCEI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_LCEI_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_LCEI_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_LCEI_STRIDE,SWAP,this->dma_cfg.swap,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_LCEI_STRIDE,FORMAT_EN,this->dma_cfg.format_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_LCEI_STRIDE,FORMAT,this->dma_cfg.format,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_LCEI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_LCEI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_LCEI_STRIDE,STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_LCEI_CON,0x80202020,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_LCEI_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //
            ISP_FUNC_DBG("CAM_LCEI_BASE_ADDR=[0x%08X] ",this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
            break;
        case ISP_DMA_IMGI:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_BASE_ADDR,(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_IMGI_STRIDE,SWAP,this->dma_cfg.swap,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_IMGI_STRIDE,FORMAT_EN,this->dma_cfg.format_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_IMGI_STRIDE,FORMAT,this->dma_cfg.format,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_IMGI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_IMGI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_IMGI_STRIDE,STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VERTICAL_FLIP_EN,IMGI_V_FLIP_EN,this->dma_cfg.v_flip_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VERTICAL_FLIP_EN,LCEI_V_FLIP_EN,this->dma_cfg.v_flip_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_CON,0x80a0a0a0,ISP_DRV_USER_ISPF);//default 0x80a0a0a0
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_CON2,0x00a0a000,ISP_DRV_USER_ISPF);//default 0x00a0a000
            //
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
            // collect src image info for MdpMgr
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcFmt    = this->p2MdpSrcFmt;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcW      = this->p2MdpSrcW;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcH      = this->p2MdpSrcH;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcYStride = this->p2MdpSrcYStride;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcUVStride = this->p2MdpSrcUVStride;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcBufSize = this->p2MdpSrcSize;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcCBufSize = this->p2MdpSrcCSize;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcVBufSize = this->p2MdpSrcVSize;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcPlaneNum = this->p2MdpSrcPlaneNum;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].srcVirAddr  = (this->dma_cfg.memBuf.base_vAddr + this->dma_cfg.memBuf.ofst_addr);
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].srcPhyAddr  = (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);

            ISP_FUNC_DBG("[imgi]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);


            ISP_FUNC_DBG("[imgi]p2cq(%d),cq(%d),dupCqIdx(%d),burstQueIdx(%d)",p2Cq,gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].cqIdx,this->dupCqIdx,this->burstQueIdx);

            ISP_FUNC_DBG("[mdp]Fmt(0x%x),W(%d),H(%d),stride(%d,%d),VA(0x%8x),PA(0x%8x),Size(0x%8x)",gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcFmt, \
                        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcW, gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcH, \
                        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcYStride, gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcYStride, \
                        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].srcVirAddr, \
                        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].srcPhyAddr, gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcBufSize);

            ISP_FUNC_DBG("DMAI(%d),(%s):reg(0x%08) ", \
                this->id(), \
                this->name_Str(), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_CQ_EN));
            break;
        case ISP_DMA_VIPI:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIPI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIPI_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIPI_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_STRIDE,SWAP,this->dma_cfg.swap,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_STRIDE,FORMAT_EN,this->dma_cfg.format_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_STRIDE,FORMAT,this->dma_cfg.format,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_STRIDE,STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIPI_CON,0x80505050,ISP_DRV_USER_ISPF);//default 0x80505050
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIPI_CON2,0x00505000,ISP_DRV_USER_ISPF);//default 0x00505000
            #if 0
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_RING_BUF,RING_SIZE, this->dma_cfg.ring_size);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIPI_RING_BUF,EN, this->dma_cfg.ring_en);
            #endif
            //
            ISP_FUNC_DBG("ISP_DMA_VIPI = 0x%x",this->dma_cfg.memBuf.base_pAddr);
            break;
        case ISP_DMA_VIP2I:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP2I_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP2I_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP2I_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP2I_STRIDE,FORMAT_EN,this->dma_cfg.format_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP2I_STRIDE,FORMAT,this->dma_cfg.format,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP2I_STRIDE,STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP2I_CON,0x80404040,ISP_DRV_USER_ISPF);//default 0x80404040
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP2I_CON2,0x00404000,ISP_DRV_USER_ISPF);//default 0x00404000
            #if 0
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP2I_RING_BUF,RING_SIZE, this->dma_cfg.ring_size);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP2I_RING_BUF,EN, this->dma_cfg.ring_en);
            #endif
            //
            ISP_FUNC_DBG("CAM_VIP2I_BASE_ADDR = 0x%x",this->dma_cfg.memBuf.base_pAddr);
            break;
        case ISP_DMA_VIP3I:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP3I_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP3I_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP3I_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP3I_STRIDE,FORMAT_EN,this->dma_cfg.format_en,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP3I_STRIDE,FORMAT,this->dma_cfg.format,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP3I_STRIDE,STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP3I_CON,0x80404040,ISP_DRV_USER_ISPF);//default 0x80404040
            ISP_WRITE_REG(m_pP2IspDrv,CAM_VIP3I_CON2,0x00404000,ISP_DRV_USER_ISPF);//default 0x00404000
            #if 0
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP3I_RING_BUF,RING_SIZE, this->dma_cfg.ring_size);
            ISP_WRITE_BITS(m_pP2IspDrv,CAM_VIP3I_RING_BUF,EN, this->dma_cfg.ring_en);
            #endif
            //
            ISP_FUNC_DBG("CAM_VIP3I_BASE_ADDR = 0x%x",this->dma_cfg.memBuf.base_pAddr);
            break;

        default:
            break;
    }


    ISP_FUNC_DBG("DMAI_B::_config:X ");
    return 0;
}

/*
int DMAI_B::_enable( void* pParam )
{
    ISP_FUNC_DBG("DMAI_B::_enable:E ");
    ISP_FUNC_DBG("[id:0x%08d] ",this->id());
    ISP_FUNC_DBG("DMAI_B::_enable:X ");
    return 0;
}
*/

int DMAI_B::_disable( void )
{
    ISP_FUNC_DBG("DMAI_B::_disable:E ");
    ISP_FUNC_DBG("[id:0x%08d] ",this->id());

#if 0
    switch(this->id()) {
        case ISP_DMA_CQI:
            break;
        default:
            break;
    }
#else
ISP_FUNC_DBG("DMAI_B::_disable:NO IMPLEMENTATION! ");
#endif

    ISP_FUNC_DBG("DMAI_B::_disable:X ");
    return 0;
}

/*
int DMAI_B::checkBusy( unsigned long* param  )
{
unsigned long u4Result;
    ISP_FUNC_DBG("DMAI_B::checkBusy:E ");
    ISP_FUNC_DBG("DMAI_B::checkBusy:X ");
    return (u4Result & 0x1 ? 0 : 1);
}
*/

int DMAI_B::_write2CQ(void)
{
unsigned int cq_module_id = 0xFF;

    ISP_FUNC_DBG("_,[id:0X%08X] ",this->id());
    unsigned int myCq = CAM_ISP_CQ_NONE;

     switch(this->id())
     {
        case ISP_DMA_IMGI:
            cq_module_id = CAM_DMA_IMGI;
            break;
        case ISP_DMA_TDRI:
            cq_module_id = CAM_DMA_TDRI;
            break;
        case ISP_DMA_VIPI:
            cq_module_id = CAM_DMA_VIPI;
            break;
        case ISP_DMA_VIP2I:
            cq_module_id = CAM_DMA_VIP2I;
            break;
        case ISP_DMA_VIP3I:
            cq_module_id = CAM_DMA_VIP3I;
            break;
        case ISP_DMA_BPCI:
            cq_module_id = CAM_DMA_BPCI;
            break;
        case ISP_DMA_BPCI_D:
            cq_module_id = CAM_DMA_BPCI_D;
            break;
        case ISP_DMA_LSCI:
            cq_module_id = CAM_DMA_LSCI;
            break;
        case ISP_DMA_LSCI_D:
            cq_module_id = CAM_DMA_LSCI_D;
            break;
        case ISP_DMA_LCEI:
            cq_module_id = CAM_DMA_LCEI;
            break;
        case ISP_DMA_UFDI:
            cq_module_id = CAM_DMA_UFDI;
            break;
        case ISP_DMA_CQI:
        default:
            ISP_FUNC_DBG("DMAI: NOT push to CQ ");
            break;
    }
    //
    if (CAM_ISP_CQ_NONE!= this->CQ) {
        myCq = this->CQ;
    } else if (CAM_ISP_CQ_NONE!= this->CQ_D) {
        myCq = this->CQ_D;
    }

    if (0xFF != cq_module_id) {
        if (CAM_DMA_IMGI == cq_module_id)
        {
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_DMA_IMGI_F);
        }
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)myCq,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)cq_module_id);
    }
    //
    ISP_FUNC_DBG("-,");
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    DMAO_B
/////////////////////////////////////////////////////////////////////////////*/
//turn off, this sram change have hw defect, it will cause white line defect on image
#define SRAM_SWITCH 0
int DMAO_B::_config( void )
{
    MINT32 index=0;
    MUINT32 C2,C3,C4,C5,C6,C7,size;
    MUINT32 stride = 0;
    MUINT32 x = 0;
    MUINT32 offset = 0;
    IspDrv* pCommonIspDrv;
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQIdx = 0;
    ISP_FUNC_DBG("DMAO(%d),[%s]:cq(%d/%d),pa(0x%08X),ofst(0x%X),size(%d,%d,%d,%d), pixel_byte(%d),cropW(%d,%d,%d,%d),format(0x%x),fps(%d),cropX_inbussize(%d)", \
        this->id(), \
        this->name_Str(), \
        this->CQ, this->CQ_D,\
        this->dma_cfg.memBuf.base_pAddr, \
        this->dma_cfg.memBuf.ofst_addr,
        this->dma_cfg.size.w, \
        this->dma_cfg.size.h, \
        this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
        this->dma_cfg.pixel_byte, \
        this->dma_cfg.crop.x, \
        this->dma_cfg.crop.y,\
        this->dma_cfg.crop.w,\
        this->dma_cfg.crop.h,\
        this->dma_cfg.format,\
        this->dma_cfg.tgFps,\
        GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size));
    //
    ispDrvSwitch2Virtual(this->CQ, this->CQ_D,this->burstQueIdx, this->dupCqIdx);

    // check isp path and assign pCommonIspDrv address
    switch (this->CQ){
        case CAM_ISP_CQ0:
        case CAM_ISP_CQ0B:
        case CAM_ISP_CQ0C:
        case CAM_ISP_CQ0_D:
        case CAM_ISP_CQ0B_D:
        case CAM_ISP_CQ0C_D:
            pCommonIspDrv = m_pP1IspDrv;
            break;
        case CAM_ISP_CQ1:
        case CAM_ISP_CQ2:
        case CAM_ISP_CQ3:
            pCommonIspDrv = m_pP2IspDrv;
            break;
        case CAM_ISP_CQ_NONE:
            switch (this->CQ_D) {
                case CAM_ISP_CQ0_D:
                case CAM_ISP_CQ0B_D:
                case CAM_ISP_CQ0C_D:
                pCommonIspDrv = m_pP1IspDrvD;
                break;
            }
            break;
        default:
            ISP_FUNC_ERR("[Error]Not set the correct CQ(0x%x)",this->CQ);
            pCommonIspDrv = NULL;
            break;
    }
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
    if(this->CQ == CAM_ISP_CQ0){
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
    }
    else if(this->CQ_D == CAM_ISP_CQ0_D){
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN) == 1)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
    }
    else if((this->CQ == CAM_ISP_CQ1) || (this->CQ == CAM_ISP_CQ2)||(this->CQ == CAM_ISP_CQ3))
    {
        _targetDupQIdx = this->dupCqIdx;
    }
#else
    _targetDupQIdx = this->dupCqIdx;
#endif
    if((this->CQ == CAM_ISP_CQ0)  ||  (this->CQ_D == CAM_ISP_CQ0_D)){
        if(this->burstQueIdx > 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("pass1 support no burstQNum");
        }
    }
    switch(this->id())
    {
        case ISP_DMA_EISO:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_EISO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_EISO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_AFO:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_AFO_D:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AFO_D_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_AAO:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_AAO_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            if(this->m_fps > FPS_BOUNDARY)
                ISP_WRITE_REG(_targetVirDrv,CAM_AAO_CON,0x80000040,ISP_DRV_USER_ISPF); // ultra-
            else
                ISP_WRITE_REG(_targetVirDrv,CAM_AAO_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);

            //currently , slow motion will map to this case
            if(this->m_fps > FPS_BOUNDARY){
                ISP_WRITE_REG(_targetVirDrv,CAM_AAO_CON,0x80000040,ISP_DRV_USER_ISPF); // ultra-
            }
            break;
        case ISP_DMA_AAO_D:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_D_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            if(this->m_fps > FPS_BOUNDARY){
                ISP_FUNC_ERR("aao_d can't enable when fps over boundry\n");
            }
            break;
        case ISP_DMA_ESFKO:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);

            //currently , slow motion will map to this case
            if(this->m_fps > FPS_BOUNDARY){
            }
            break;
        case ISP_DMA_LCSO:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);

            //currently , slow motion will map to this case
            if(this->m_fps > FPS_BOUNDARY){
                ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_CON,0x80000060,ISP_DRV_USER_ISPF); // ultra-
            }
            break;
        case ISP_DMA_LCSO_D:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_D_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_D_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_D_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_D_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_D_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LCSO_D_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_IMGO:
            this->m_fps = this->dma_cfg.tgFps;
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_CON,0x807878F0,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_CON2,0x00B0B000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_CON,0x801010B0,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_CROP,(this->dma_cfg.crop.y << 16) | GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size),ISP_DRV_USER_ISPF);
            //CAM_IMGO_BASE_ADDR only for P2 use
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            //currently , slow motion will map to this case
            if(this->m_fps > FPS_BOUNDARY){
#if SRAM_SWITCH
                ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_CON,0x817070E0,ISP_DRV_USER_ISPF); // ultra-
#endif
                {//special case to reduce kernel log
                    MUINT32 p1_log[6] = {0};
                    if(this->m_bKDBG){
                        p1_log[3] = 1;
                        p1_log[4] = 1;
                        p1_log[5] = 1;
                    }
                    p1_log[1] = 1;
                    m_pPhyIspDrv->dumpDBGLog(p1_log,NULL);
                }
            }
            //
            break;
        case ISP_DMA_RRZO:
            this->m_fps = this->dma_cfg.tgFps;
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            //3 We use RRZ to corp, thus almost the RRZ crop width & height is 0
            //x = ((in_w*3)/2)*( raw8?1:( (raw10)?1.25:1.5 ) )
            //x size should be 8 alignment
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_XSIZE, this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);

            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_CON,0x80202080,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(this->m_pIspDrvShell->m_pPhyIspDrv_bak,CAM_IMGI_SLOW_DOWN,0x0c000000,ISP_DRV_USER_ISPF); // ultra-

            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_CON2,0x00808000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_CROP, (this->dma_cfg.crop.y << 16) | GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size), ISP_DRV_USER_ISPF);
            ISP_FUNC_DBG("ISP_DMA_RRZO, pxl_byte=%d, w=0x%x,x=0x%x,stride=0x%x, cropXY=%d,%d",this->dma_cfg.pixel_byte, this->dma_cfg.size.w, x, stride,this->dma_cfg.crop.y, this->dma_cfg.crop.x);
            //currently , slow motion will map to this case
            if(this->m_fps > FPS_BOUNDARY){
#if SRAM_SWITCH
                ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_CON,0x816060FF,ISP_DRV_USER_ISPF); // ultra-
                ISP_WRITE_REG(this->m_pIspDrvShell->m_pPhyIspDrv_bak,CAM_IMGI_SLOW_DOWN,0x04010000,ISP_DRV_USER_ISPF); // ultra-
#else
                ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_CON,0x81202080,ISP_DRV_USER_ISPF); // ultra-
#endif
                {//special case to reduce kernel log
                    MUINT32 p1_log[6] = {0};
                    if(this->m_bKDBG){
                        p1_log[3] = 1;
                        p1_log[4] = 1;
                        p1_log[5] = 1;
                    }
                    p1_log[3] = 1;
                    p1_log[1] = 1;
                    m_pPhyIspDrv->dumpDBGLog(p1_log,NULL);
                }
                m_pPhyIspDrv->SetFPS(this->m_fps);
            }
            //
            break;
        case ISP_DMA_UFEO:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_UFEO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_UFEO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize; same as imgo.h
            ISP_WRITE_REG(_targetVirDrv,CAM_UFEO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_UFEO_CON,0x80202020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_UFEO_CON2,0x00202000,ISP_DRV_USER_ISPF);
            //
            ISP_WRITE_REG(_targetVirDrv,CAM_UFE_CON,0,ISP_DRV_USER_ISPF); //[0] = 0:compression 1: no compression paked 10

            ISP_FUNC_DBG("[ISP_BUF]:WR[0]/RO[1](%d)",pCommonIspDrv->m_regRWMode);

            break;

        case ISP_DMA_IMGO_D:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_D_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_D_STRIDE, this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_D_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_D_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_D_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_D_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_D_CON,0x807878F0,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_D_CON,0x801010B0,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_D_CON2,0x00B0B000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_D_CROP,(this->dma_cfg.crop.y << 16) | GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size),ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_RRZO_D:
            if(this->m_fps > FPS_BOUNDARY){
                ISP_FUNC_ERR("rrzo_d can't enable when fps over boundry\n");
            }
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_D_XSIZE, this->dma_cfg.size.xsize - 1, ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_D_STRIDE,  this->dma_cfg.size.stride, ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_D_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_D_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_D_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_RRZO_D_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_D_CON,0x806060c0,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_D_CON,0x80202080,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_D_CON2,0x00808000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_RRZO_D_CROP,(this->dma_cfg.crop.y << 16) | GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size),ISP_DRV_USER_ISPF);
            //
            ISP_FUNC_DBG("ISP_DMA_RRZOD, pxl_byte=%d, w=0x%x,x=0x%x,stride=0x%x, cropXY=%d,%d",\
               this->dma_cfg.pixel_byte, this->dma_cfg.size.w, x, this->dma_cfg.size.stride,\
               this->dma_cfg.crop.y, this->dma_cfg.crop.x);

                break;
        case ISP_DMA_MFBO:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_MFBO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_MFBO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_MFBO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pP2IspDrv,CAM_MFBO_STRIDE,0x00900000 | this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_MFBO_CON,0x80202020,ISP_DRV_USER_ISPF);//default 0x80404040
            ISP_WRITE_REG(m_pP2IspDrv,CAM_MFBO_CON2,0x00202000,ISP_DRV_USER_ISPF);//default 0x00404000
            ISP_WRITE_REG(m_pP2IspDrv,CAM_MFBO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_IMG2O:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG2O_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG2O_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG2O_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG2O_STRIDE,0x00900000 | this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG2O_CON,0x80404040,ISP_DRV_USER_ISPF);//default 0x80404040
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG2O_CON2,0x00404000,ISP_DRV_USER_ISPF);//default 0x00404000
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG2O_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            break;

        case ISP_DMA_IMG3O:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3O_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3O_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3O_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3O_STRIDE,0x00900000 | this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3O_CON,0x80404040,ISP_DRV_USER_ISPF);//default 0x80404040
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3O_CON2,0x00404000,ISP_DRV_USER_ISPF);//default 0x00404000
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3O_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_IMG3BO:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3BO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3BO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3BO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3BO_STRIDE,0x00900000 | this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3BO_CON,0x80202020,ISP_DRV_USER_ISPF);//default 0x80202020
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3BO_CON2,0x00202000,ISP_DRV_USER_ISPF);//default 0x00202000
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3BO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_IMG3CO:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3CO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3CO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3CO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3CO_STRIDE,0x00900000 | this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3CO_CON,0x80202020,ISP_DRV_USER_ISPF);//default 0x80202020
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3CO_CON2,0x00202000,ISP_DRV_USER_ISPF);//default 0x00202000
            ISP_WRITE_REG(m_pP2IspDrv,CAM_IMG3CO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_FEO:
            ISP_WRITE_REG(m_pP2IspDrv,CAM_FEO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_FEO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_FEO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pP2IspDrv,CAM_FEO_STRIDE,0x00900000 | this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_FEO_CON,0x80202020,ISP_DRV_USER_ISPF);//default 0x80202020
            ISP_WRITE_REG(m_pP2IspDrv,CAM_FEO_CON2,0x00202000,ISP_DRV_USER_ISPF);//default 0x00202000
            break;

        case ISP_DMA_CAMSV_IMGO:
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_CON,0x08404040,ISP_DRV_USER_ISPF); // ultra- 0x08404040
            //enlarge untra
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_CON,0x08404080,ISP_DRV_USER_ISPF); // ultra- 0x08404040
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_CON2,0x00808000,ISP_DRV_USER_ISPF); // 0x00404000
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);

            //
            ISP_FUNC_DBG("[ISP_BUF]:WR[0]/RO[1](%d)",m_pPhyIspDrv->m_regRWMode);
            break;
        case ISP_DMA_CAMSV2_IMGO:
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_D_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_D_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_D_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv, CAMSV_IMGO_SV_D_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_CON,0x08404040,ISP_DRV_USER_ISPF);   // ultra- 0x08404040
            //enlarge ultra
            //ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_CON,0x08404040,ISP_DRV_USER_ISPF);   // ultra- 0x08404040
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_CON,0x08404080,ISP_DRV_USER_ISPF);   // ultra- 0x08404040
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_CON2,0x00808000,ISP_DRV_USER_ISPF);  //0x00404000
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_IMGO_SV_D_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            //
            ISP_FUNC_DBG("[ISP_BUF]:WR[0]/RO[1](%d)",m_pPhyIspDrv->m_regRWMode);

            break;
        default:
            break;
    }

    //ISP_FUNC_DBG("DMAO_B::_config:X ");
    return 0;
}


int DMAO_B::_write2CQ(void)
{
    unsigned int cq_module_id = 0xFF;
    unsigned int cq_module_id2 = 0xFF;
    unsigned int myCq = CAM_ISP_CQ_NONE;
    ISP_FUNC_DBG("+,");
    ISP_FUNC_DBG("[id:0X%08X],CQ(%d)",this->id(),this->CQ);

     switch(this->id())
     {
        case ISP_DMA_IMGO:
            cq_module_id = CAM_DMA_IMGO;
            if(this->CQ==ISP_DRV_CQ01 || this->CQ==ISP_DRV_CQ02 || this->CQ==ISP_DRV_CQ03){
                cq_module_id2 = CAM_DMA_IMGO_BASE_ADDR;
            }
            break;
        case ISP_DMA_RRZO:
            cq_module_id = CAM_DMA_RRZO;
            break;
        case ISP_DMA_UFEO:
            cq_module_id = CAM_DMA_UFEO;
            break;
        case ISP_DMA_IMGO_D:
            cq_module_id = CAM_DMA_IMGO_D;
            break;
        case ISP_DMA_RRZO_D:
            cq_module_id = CAM_DMA_RRZO_D;
            break;
        case ISP_DMA_MFBO:
            cq_module_id = CAM_DMA_MFBO;
            break;
        case ISP_DMA_IMG2O:
            cq_module_id = CAM_DMA_IMG2O;
            break;
        case ISP_DMA_IMG3O:
            cq_module_id = CAM_DMA_IMG3O;
            break;
        case ISP_DMA_IMG3BO:
            cq_module_id = CAM_DMA_IMG3BO;
            break;
        case ISP_DMA_IMG3CO:
            cq_module_id = CAM_DMA_IMG3CO;
            break;
        case ISP_DMA_FEO:
            cq_module_id = CAM_DMA_FEO;
            break;
        case ISP_DMA_EISO:    cq_module_id = CAM_DMA_EISO;
            break;
        case ISP_DMA_AFO:    cq_module_id = CAM_DMA_AFO;
            break;
        case ISP_DMA_AFO_D:    cq_module_id = CAM_DMA_AFO_D;
            break;
        case ISP_DMA_AAO:    cq_module_id = CAM_DMA_AAO;
            break;
        case ISP_DMA_AAO_D:    cq_module_id = CAM_DMA_AAO_D;
            break;
        case ISP_DMA_ESFKO:    cq_module_id = CAM_DMA_ESFKO;
            break;
        case CAM_DMA_LCSO:    cq_module_id = CAM_DMA_LCSO;
            break;
        case CAM_DMA_LCSO_D:    cq_module_id = CAM_DMA_LCSO_D;
            break;
        default:
            ISP_FUNC_DBG(" NOT push to CQ ");
            break;
    }
    if (CAM_ISP_CQ_NONE!= this->CQ) {
        myCq = this->CQ;
    } else if (CAM_ISP_CQ_NONE!= this->CQ_D) {
        myCq = this->CQ_D;
    }

    if (0xFF != cq_module_id) {
        if((myCq == CAM_ISP_CQ0) || (myCq == CAM_ISP_CQ0_D)){
            MUINT32 _targetDupQIdx = 0;

#ifdef _PASS1_CQ_CONTINUOUS_MODE_
            if(this->CQ == CAM_ISP_CQ0){
                if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
                    _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
                else
                    _targetDupQIdx = this->dupCqIdx;
            }
            else if(this->CQ_D == CAM_ISP_CQ0_D){
                if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN) == 1)
                    _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
                else
                    _targetDupQIdx = this->dupCqIdx;
            }
#else
            _targetDupQIdx = this->dupCqIdx;
#endif
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)myCq,this->burstQueIdx,_targetDupQIdx,(CAM_MODULE_ENUM)cq_module_id);
        }else
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)myCq,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)cq_module_id);
    }
    if (0xFF != cq_module_id2) {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)cq_module_id2);
    }

    ISP_FUNC_DBG("-,");

    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    ISP_RAW_CENTRAL_CTRL_INFO
  /////////////////////////////////////////////////////////////////////////////*/
#define CAM_CTL_EN_P1_MASK 0x007FFFFF
#define CAM_CTL_EN_P1_D_MASK 0x0049DFFF
#define CAM_CTL_CQ_EN_MASK   0x1FFF8FFE
#define CAM_CTL_FMT_SEL_P1_MASK_No_TWIN   0x3787F033
#define CAM_CTL_FMT_SEL_P1_D_MASK_No_TWIN 0x3787F033
#define CAM_CTL_SEL_GLOBAL_MASK_0 0x001FEFEB

MBOOL
ISP_RAW_CENTRAL_CTRL_INFO::config2( ISP_TOP_CTRL * pSrcTopCtrl )
{
    struct stIspTopCtl  *pIspTop = &mTopCtrl.isp_top_ctl;

    if (( ISP_PASS1 == pSrcTopCtrl->path         || ISP_PASS1_D == pSrcTopCtrl->pathD) ||
        ( ISP_PASS1_CAMSV == pSrcTopCtrl->path   || ISP_PASS1_CAMSV == pSrcTopCtrl->pathD) ||
        ( ISP_PASS1_CAMSV_D == pSrcTopCtrl->path || ISP_PASS1_CAMSV_D == pSrcTopCtrl->pathD))  {

        ISP_FUNC_DBG("RAW_CENTRAL::config2 +. path/pathD(%x_%x)", pSrcTopCtrl->path, pSrcTopCtrl->pathD);

        if (NULL == mpPhyIspDrv) {
           mpPhyIspDrv = m_pIspDrvShell->getPhyIspDrv();
           if (NULL == mpPhyIspDrv) {
              ISP_FUNC_INF("RAW_CENTRAL NULL == mpPhyIspDrv");
           }
        }
        if ((mEnablePath & ISP_PASS1) && (mEnablePath & ISP_PASS1_D))  {
           //the 2 pathes have been opened...
           ISP_FUNC_ERR("Error RAW_CENTRAL the 2 pathes have been opened");
           return MFALSE;
        }

        if (0 == mEnablePath) {
            ISP_FUNC_INF("RAW_CENTRAL:: P1 path");

            //No path is enabled now.
            //1. update cfg-setting to this calss. And 2.Call Top Ctrl to set HWs
            memcpy(&mTopCtrl, pSrcTopCtrl, sizeof(ISP_TOP_CTRL));
        }
    } else {
        //ISP_FUNC_DBG("ISP_RAW_CENTRAL_CTRL_INFO:: Not-P1-Path config2 -");
        return MTRUE;
    }

    if ( ISP_PASS1 == pSrcTopCtrl->path || ISP_PASS1_D == pSrcTopCtrl->pathD ) {

       if (0 == mEnablePath) {

           //We need the mRawIspDrv even the TG2 is configured first,
           //we don't use mTopCtrl.CQ to get isp_drv
           //but hardcode:ISP_DRV_CQ0 to give ISP_DRV_CQ0 as parameter here....
           //MW might config TG2 firdst, then TG1...
           //But we hope all the common control registers are written to HW by ISP_DRV_CQ0
           mRawIspDrv = pSrcTopCtrl->m_pIspDrvShell->ispDrvGetCQIspDrv(ISP_DRV_CQ0/*mTopCtrl.CQ*/,0 /*mTopCtrl.burstQueIdx*/, 0/*mTopCtrl.dupCqIdx*/);
           mRawDIspDrv = pSrcTopCtrl->m_pIspDrvShell->ispDrvGetCQIspDrv(ISP_DRV_CQ0_D/*mTopCtrl.CQ_D*/,0/*mTopCtrl.burstQueIdx*/, 0/*mTopCtrl.dupCqIdx*/);

           if (ISP_PASS_NONE != pSrcTopCtrl->path) {
               mEnablePath |= pSrcTopCtrl->path;
               if (NULL == mRawIspDrv) {
                   ISP_FUNC_ERR("RAW_CENTRAL=> Err NULL == mRawIspDrv");
               }
           }

           if (ISP_PASS_NONE != pSrcTopCtrl->pathD) {
               mEnablePath |= pSrcTopCtrl->pathD;
               if (NULL == mRawDIspDrv) {
                   ISP_FUNC_ERR("RAW_CENTRAL=>Err NULL == mRaw-DIspDrv");
               }
           }
           ISP_FUNC_INF("RAW_CENTRAL, ispdrv/ispdrvD=0x%8x/0x%8x\n", mRawIspDrv, mRawDIspDrv);

       }
       else {
           ISP_FUNC_INF("RAW_CENTRAL[PIP] 2nd Cfg,path:%x,%x\n", pSrcTopCtrl->path, pSrcTopCtrl->pathD);

           if ((ISP_PASS1 == pSrcTopCtrl->path) && (mEnablePath & ISP_PASS1)) {
               ISP_FUNC_INF("RAW_CENTRAL:ISP_PASS1 has been congif\n");
               return MTRUE;
           } else if((ISP_PASS1_D == pSrcTopCtrl->pathD) && (mEnablePath & ISP_PASS1_D)) {
               ISP_FUNC_INF("RAW_CENTRAL:ISP_PASS1D has been congif\n");
               return MTRUE;
           } else {

           }

           //Currently, one path is runing. Be careful to handle related registers
           if (( MTRUE == mTwinMode ) || \
               ((MTRUE == pSrcTopCtrl->isTwinMode) && (ISP_PASS1_D == pSrcTopCtrl->pathD))){
              //Donesn't support TG2 Twin Mode
              ISP_FUNC_ERR("RAW_CENTRAL=>PIP NotSupport Twin Mode\n");
              return MFALSE;
           }

           //ISP_FUNC_DBG("RAW_CENTRAL PIP 2ndSensor cfg");
           MUINT32 mask = 0, temp;


           if (ISP_PASS1 == pSrcTopCtrl->path) {
               mEnablePath   |= pSrcTopCtrl->path;
               mTopCtrl.path = pSrcTopCtrl->path;
               mTopCtrl.CQ  = pSrcTopCtrl->CQ;
               mTopCtrl.sub_mode = pSrcTopCtrl->sub_mode;
               mRawIspDrv = pSrcTopCtrl->m_pIspDrvShell->ispDrvGetCQIspDrv(mTopCtrl.CQ,mTopCtrl.burstQueIdx, mTopCtrl.dupCqIdx);
#if 1
               pIspTop->CAM_CTL_EN_P1.Raw = pSrcTopCtrl->isp_top_ctl.CAM_CTL_EN_P1.Raw;
               pIspTop->CAM_CTL_EN_P1_DMA.Raw = pSrcTopCtrl->isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw;
               ISP_FUNC_INF("RAW_CENTRAL:: P1_DMA=0x%x", pIspTop->CAM_CTL_EN_P1_DMA.Raw);

               pIspTop->CAM_CTL_CQ_EN.Raw = ((pSrcTopCtrl->isp_top_ctl.CAM_CTL_CQ_EN.Raw | \
                           ISP_READ_REG_NOPROTECT(mRawIspDrv, CAM_CTL_CQ_EN)) & CAM_CTL_CQ_EN_MASK) | \
                          (ISP_READ_REG_NOPROTECT(mpPhyIspDrv, CAM_CTL_CQ_EN) & (~CAM_CTL_CQ_EN_MASK));
               ISP_FUNC_INF("RAW_CENTRAL:: CQ_EN SwVal=0x%x, CqVal=0x%x, HwVal=0x%x ", \
                   pSrcTopCtrl->isp_top_ctl.CAM_CTL_CQ_EN.Raw, \
                   ISP_READ_REG_NOPROTECT(mRawIspDrv, CAM_CTL_CQ_EN), \
                   ISP_READ_REG_NOPROTECT(mpPhyIspDrv, CAM_CTL_CQ_EN));
               ISP_FUNC_INF("RAW_CENTRAL:: Final CQ_EN=0x%x", pIspTop->CAM_CTL_CQ_EN.Raw);

               mask = 0x3787F033;
               if (MTRUE == mTwinMode) {
                  mask = 0x7F033;
               }
               pIspTop->CAM_CTL_FMT_SEL_P1.Raw = pSrcTopCtrl->isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw & mask;
               ISP_FUNC_INF("RAW_CENTRAL:: FMT_SEL_P1=0x%08x", pIspTop->CAM_CTL_FMT_SEL_P1.Raw);

               pIspTop->CAM_CTL_FMT_SEL_P1_MASK = mask;
               pIspTop->CAM_CTL_SEL_P1.Raw   = pSrcTopCtrl->isp_top_ctl.CAM_CTL_SEL_P1.Raw;
               ISP_FUNC_INF("RAW_CENTRAL:: SEL_P1=0x%08x", pIspTop->CAM_CTL_SEL_P1.Raw);
               pIspTop->CAM_CTL_SEL_GLOBAL.Raw = ((pSrcTopCtrl->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw  | \
                     ISP_READ_REG_NOPROTECT(mRawIspDrv,  CAM_CTL_SEL_GLOBAL)) & CAM_CTL_SEL_GLOBAL_MASK_0) | \
                     (ISP_READ_REG_NOPROTECT(mpPhyIspDrv, CAM_CTL_SEL_GLOBAL) & (~CAM_CTL_SEL_GLOBAL_MASK_0));
               ISP_FUNC_INF("RAW_CENTRAL:: SEL_GLOBAL=0x%08x", pIspTop->CAM_CTL_SEL_GLOBAL.Raw);


               pIspTop->CAM_CTL_INT_P1_EN.Raw  = pSrcTopCtrl->isp_top_ctl.CAM_CTL_INT_P1_EN.Raw;
               //ISP_FUNC_INF("RAW_CENTRAL:: CAM_CTL_INT_P1_EN=0x%08x", pIspTop->CAM_CTL_INT_P1_EN.Raw);
               pIspTop->CAM_CTL_INT_P1_EN2.Raw = pSrcTopCtrl->isp_top_ctl.CAM_CTL_INT_P1_EN2.Raw;
               //ISP_FUNC_INF("RAW_CENTRAL:: CAM_CTL_INT_P1_EN2=0x%08x", pIspTop->CAM_CTL_INT_P1_EN2.Raw);
#else

#endif

           }

           if (ISP_PASS1_D == pSrcTopCtrl->pathD) {
               mEnablePath   |= pSrcTopCtrl->pathD;
               mTopCtrl.pathD = pSrcTopCtrl->pathD;
               mTopCtrl.CQ_D  = pSrcTopCtrl->CQ_D;
               mTopCtrl.sub_mode_D = pSrcTopCtrl->sub_mode_D;
               mRawDIspDrv    = pSrcTopCtrl->m_pIspDrvShell->ispDrvGetCQIspDrv(mTopCtrl.CQ_D,mTopCtrl.burstQueIdx, mTopCtrl.dupCqIdx);

               pIspTop->CAM_CTL_EN_P1_D.Raw |= pSrcTopCtrl->isp_top_ctl.CAM_CTL_EN_P1_D.Raw;
               ISP_FUNC_INF("RAW_CENTRAL::EN_P1_D=0x%x", pIspTop->CAM_CTL_EN_P1_D.Raw);

               pIspTop->CAM_CTL_EN_P1_DMA_D.Raw = pSrcTopCtrl->isp_top_ctl.CAM_CTL_EN_P1_DMA_D.Raw;
               ISP_FUNC_DBG("RAW_CENTRAL:: EN_P1_DMA_D=0x%x", pIspTop->CAM_CTL_EN_P1_DMA_D.Raw);

               pIspTop->CAM_CTL_CQ_EN.Raw = ((pSrcTopCtrl->isp_top_ctl.CAM_CTL_CQ_EN.Raw | \
                           ISP_READ_REG_NOPROTECT(mRawIspDrv, CAM_CTL_CQ_EN)) & CAM_CTL_CQ_EN_MASK) | \
                          (ISP_READ_REG_NOPROTECT(mpPhyIspDrv, CAM_CTL_CQ_EN) & (~CAM_CTL_CQ_EN_MASK));
               ISP_FUNC_DBG("RAW_CENTRAL::D Final CQ_EN=0x%x, input:0x%x", pIspTop->CAM_CTL_CQ_EN.Raw, pSrcTopCtrl->isp_top_ctl.CAM_CTL_CQ_EN.Raw);

               mask = 0x3787F033;
               if (MTRUE == mTwinMode) {
                  mask = 0x7F033;
               }
               pIspTop->CAM_CTL_FMT_SEL_P1_D.Raw = (~mask)&pIspTop->CAM_CTL_FMT_SEL_P1_D.Raw;
               pIspTop->CAM_CTL_FMT_SEL_P1_D.Raw |= pSrcTopCtrl->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Raw & mask;
               ISP_FUNC_INF("RAW_CENTRAL:: FMT_SEL_P1_D=0x%08x - 0x%x", pIspTop->CAM_CTL_FMT_SEL_P1_D.Raw,pIspTop->CAM_CTL_FMT_SEL_P1_D.Raw);

               pIspTop->CAM_CTL_FMT_SEL_P1_D_MASK = mask;
               pIspTop->CAM_CTL_SEL_P1_D.Raw   = pSrcTopCtrl->isp_top_ctl.CAM_CTL_SEL_P1_D.Raw;
               ISP_FUNC_DBG("RAW_CENTRAL:: SEL_P1_D=0x%08x", pIspTop->CAM_CTL_SEL_P1_D.Raw);

               pIspTop->CAM_CTL_SEL_GLOBAL.Raw |= pSrcTopCtrl->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw;
               ISP_FUNC_DBG("RAW_CENTRAL:: SEL_GLOBAL=0x%08x", pIspTop->CAM_CTL_SEL_GLOBAL.Raw);

               pIspTop->CAM_CTL_INT_P1_EN_D.Raw |= pSrcTopCtrl->isp_top_ctl.CAM_CTL_INT_P1_EN_D.Raw;
               ISP_FUNC_DBG("RAW_CENTRAL:: INT_P1_EN_D=0x%08x", pIspTop->CAM_CTL_INT_P1_EN_D.Raw);

               pIspTop->CAM_CTL_INT_P1_EN2_D.Raw |= pSrcTopCtrl->isp_top_ctl.CAM_CTL_INT_P1_EN2_D.Raw;
               ISP_FUNC_DBG("RAW_CENTRAL:: INT_P1_EN2_D=0x%08x", pIspTop->CAM_CTL_INT_P1_EN2_D.Raw);

           }
       }
   }

   if ( ISP_PASS1_CAMSV == pSrcTopCtrl->path || ISP_PASS1_CAMSV == pSrcTopCtrl->pathD ){

       if (ISP_PASS1_CAMSV & mEnablePath) {
           //CamSV has been occupied
           return MFALSE;
       }
       mEnablePath |= ISP_PASS1_CAMSV;
       pIspTop->CAMSV_CAMSV_MODULE_EN.Raw    = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw;
       pIspTop->CAMSV_CAMSV_FMT_SEL.Raw      = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV_FMT_SEL.Raw;
       pIspTop->CAMSV_CAMSV_FMT_SEL_MASK     = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV_FMT_SEL_MASK;
       pIspTop->CAMSV_CAMSV_INT_EN.Raw       = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV_INT_EN.Raw;
       pIspTop->CAMSV_CAMSV_PAK_MASK         = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV_PAK_MASK;
       pIspTop->CAMSV_CAMSV_PAK.Raw          = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV_PAK.Raw;
   }

   if ( ISP_PASS1_CAMSV_D == pSrcTopCtrl->path || ISP_PASS1_CAMSV_D == pSrcTopCtrl->pathD ){

       if (ISP_PASS1_CAMSV_D & mEnablePath) {
           //CamSV-D has been occupied
           return MFALSE;
       }

       mEnablePath |= ISP_PASS1_CAMSV_D;
       pIspTop->CAMSV_CAMSV2_MODULE_EN.Raw    = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw;
       pIspTop->CAMSV_CAMSV2_FMT_SEL.Raw      = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV2_FMT_SEL.Raw;
       pIspTop->CAMSV_CAMSV2_FMT_SEL_MASK     = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV2_FMT_SEL_MASK;
       pIspTop->CAMSV_CAMSV2_INT_EN.Raw       = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV2_INT_EN.Raw;
       pIspTop->CAMSV_CAMSV2_PAK_MASK         = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV2_PAK_MASK;
       pIspTop->CAMSV_CAMSV2_PAK.Raw          = pSrcTopCtrl->isp_top_ctl.CAMSV_CAMSV2_PAK.Raw;
   }

   ISP_FUNC_DBG("RAW_CENTRAL -");
   return MTRUE;
}



/*/////////////////////////////////////////////////////////////////////////////
    ISP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_TOP_CTRL::_config( void )
{
    IspDrv* pCommonIspDrv;
    MUINT32 logPath=ISP_PASS_NONE;
    IspDrv* _tmpRawIspDrv  = NULL;
    IspDrv* _tmpRawDIspDrv = NULL;
    ISP_FUNC_DBG("ISP_TOP_CTRL +,path(%d,%d),CQ(%d),twin(%d),drvScenario(%d),hwScenario(%d),HW_SUB_MODE(%d),sub_mode(%d)", \
                                                          this->path, this->pathD,\
                                                          this->CQ, \
                                                          this->isTwinMode, \
                                                          this->drvScenario, \
                                                          this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO, \
                                                          this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE,
                                                          this->sub_mode);

    //fixed submode type
    //raw sensor ok with submode_raw, fail with submode_yuv
    //yuv sensor ok with both submode_raw,submode_yuv
    if ( ISP_SUB_MODE_YUV == this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE ) {
        this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE = ISP_SUB_MODE_RAW;
        ISP_FUNC_INF("set submode to raw");
    }

    if ( CAM_ISP_CQ_NONE != this->CQ ) {
        ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_MAX,(_isp_dma_enum_)this->CQ,0,0};
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
    }
    if ( CAM_ISP_CQ_NONE != this->CQ_D ) {
        ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_MAX,(_isp_dma_enum_)this->CQ_D,0,0};
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
    }

    // check isp path and assign pCommonIspDrv address
    switch (this->path){
        case ISP_PASS1:
        case ISP_PASS1_CAMSV:
        case ISP_PASS1_CAMSV_D:
            pCommonIspDrv = m_pP1IspDrv;
            logPath = ISP_PASS1;
            {
                char value[PROPERTY_VALUE_MAX] = {'\0'};
                property_get("debug.isp", value, "0");
                this->m_bKDBG = atoi(value);
            }
            break;
        case ISP_PASS2:
        case ISP_PASS2B:
        case ISP_PASS2C:
            pCommonIspDrv = m_pP2IspDrv;
            logPath = ISP_PASS2;
            break;
        case ISP_PASS_NONE:
            switch (this->pathD) {
                case ISP_PASS1_D:
                case ISP_PASS1_CAMSV_D:
                    pCommonIspDrv = m_pP1IspDrvD;
                    logPath = ISP_PASS1_D;
                    {
                        char value[PROPERTY_VALUE_MAX] = {'\0'};
                        property_get("debug.isp", value, "0");
                        this->m_bKDBG = atoi(value);
                    }
                    break;
                 default:
                    ISP_FUNC_ERR("[Error]Not set the correct pathD(0x%x)",this->pathD);
                    pCommonIspDrv = NULL;
                    break;
            }
            break;
        default:
            ISP_FUNC_ERR("[Error]Not set the correct path(0x%x)",this->path);
            pCommonIspDrv = NULL;
            break;
    }


    if(logPath == ISP_PASS2) { // P2

        ISP_FUNC_INF("[P2]en(0x%08x),EnDma(0x%08x),ctlSel(0x%08x),intEn(0x%08x),fmtSel(0x%08x),cqEn(0x%08x)", \
                        this->isp_top_ctl.CAM_CTL_EN_P2.Raw, \
                        this->isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw, \
                        this->isp_top_ctl.CAM_CTL_SEL_P2.Raw, \
                        this->isp_top_ctl.CAM_CTL_INT_P2_EN.Raw, \
                        this->isp_top_ctl.CAM_CTL_FMT_SEL_P2.Raw, \
                        this->isp_top_ctl.CAM_CTL_CQ_EN_P2.Raw);

    }else{ // P1
        ISP_FUNC_INF("[P1]en(0x%08x/0x%08x),dma(0x%08x/0x%08x),fmtSel(0x%08x/0x%08x),sel(0x%08x),cqEn(0x%08x),intEn(0x%08x),SelGlo(0x%08x)", \
                        this->isp_top_ctl.CAM_CTL_EN_P1.Raw, \
                        this->isp_top_ctl.CAM_CTL_EN_P1_D.Raw, \
                        this->isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw, \
                        this->isp_top_ctl.CAM_CTL_EN_P1_DMA_D.Raw, \
                        this->isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw, \
                        this->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Raw, \
                        this->isp_top_ctl.CAM_CTL_SEL_P1.Raw, \
                        this->isp_top_ctl.CAM_CTL_CQ_EN.Raw, \
                        this->isp_top_ctl.CAM_CTL_INT_P1_EN.Raw, \
                        this->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw);
        ISP_FUNC_INF("[CAMSV]en(0x%08x),2en(0x%08x),intEn(0x%08x),2intEn(0x%08x),fmtSel(0x%08x),2fmtSel(0x%08x)", \
                        this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw, \
                        this->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw, \
                        this->isp_top_ctl.CAMSV_CAMSV_INT_EN.Raw, \
                        this->isp_top_ctl.CAMSV_CAMSV2_INT_EN.Raw, \
                        this->isp_top_ctl.CAMSV_CAMSV_FMT_SEL.Raw, \
                        this->isp_top_ctl.CAMSV_CAMSV2_FMT_SEL.Raw);
        #if 0
        ISP_FUNC_INF("[CAMSV]EN(0x%08x/0x%08x),fmt(0x%08x/0x%08x)\n", \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_MODULE_EN),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_MODULE_EN),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_FMT_SEL),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_FMT_SEL));
        #endif
    }
#ifdef _FBC_IN_IPRAW
    if(this->path == ISP_PASS1){
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,RCNT_INC,1,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,RCNT_INC,1,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,FB_NUM,0,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,FB_NUM,0,ISP_DRV_USER_ISPF);
    }
    if(this->pathD == ISP_PASS1_D){
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_D_FBC,RCNT_INC,1,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC,RCNT_INC,1,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_D_FBC,FB_NUM,0,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC,FB_NUM,0,ISP_DRV_USER_ISPF);
    }
#endif
    ISP_FUNC_VRB("m_pP1IspDrv(0x%x),m_pP2IspDrv(0x%x),m_pPhyIspDrv(0x%x)",m_pP1IspDrv,m_pP2IspDrv,m_pPhyIspDrv);


    //This is sw workaround to fix LSCI read address 0 data when it is disabled
    if ((ISP_PASS1 == this->path)||(ISP_PASS1_D == this->pathD)){
#ifdef _CmdQ_HwBug_Patch_
           MUINT32 temp = lsciSwWorkaroundBufPa;
           IspDrv* mRawIspDrv  = this->m_pIspDrvShell->ispDrvGetCQIspDrv(ISP_DRV_CQ0,0/*mTopCtrl.burstQueIdx*/, 0/*mTopCtrl.dupCqIdx*/);
           IspDrv* mRawDIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(ISP_DRV_CQ0_D,0, 0/*mTopCtrl.burstQueIdx*/);

            if (!temp) {
                ISP_FUNC_DBG("lsciSwWorkaroundBufPa = NULL");
                temp = 0x15007204;
            }
            if(rawCentralCtrl.mEnablePath == 0){
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);
            }
            else if(ISP_PASS1 == this->path){
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);
            }
            else if(ISP_PASS1_D == this->pathD){
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);
            }

            ISP_FUNC_DBG("TOP_CONFIG LSCI: baseA=0x%x/0x%x, x=0x%x/0x%x, y = 0x%x/0x%x ",
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_LSCI_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_LSCI_D_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_LSCI_XSIZE),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_LSCI_D_XSIZE),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_LSCI_YSIZE),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_LSCI_D_YSIZE));
#endif

            if(this->burstQueIdx >= ISP_DRV_DEFAULT_BURST_QUEUE_NUM){
                this->burstQueIdx = ISP_DRV_DEFAULT_BURST_QUEUE_NUM;
                ISP_FUNC_ERR("p1 support no burstQ\n");
            }
#if 0 //write2 cmdQ is not needed, write2 phy is enough! cmdQ is only needed in p1tunecfg if lsci descriptor been applied!
            //fill all basic p1 cfg on dupCmdQ
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                _tmpRawIspDrv  = this->m_pIspDrvShell->ispDrvGetCQIspDrv(ISP_DRV_CQ0,this->burstQueIdx, i);
                _tmpRawDIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(ISP_DRV_CQ0_D,this->burstQueIdx, i);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_LSCI_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_LSCI_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_LSCI_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_LSCI_D_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_LSCI_D_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_LSCI_D_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);

                ISP_WRITE_REG(_tmpRawDIspDrv,CAM_LSCI_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawDIspDrv,CAM_LSCI_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawDIspDrv,CAM_LSCI_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawDIspDrv,CAM_LSCI_D_BASE_ADDR, temp,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawDIspDrv,CAM_LSCI_D_XSIZE, 0x7 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawDIspDrv,CAM_LSCI_D_YSIZE, 0x0 ,ISP_DRV_USER_ISPF);
            }
#endif
    }
    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /*** write to physical register directly ***/
    //NOTE!!! should check CQ FIRST, otherwise virtual ISP may be NULL.
    if (( CAM_ISP_CQ_NONE != this->CQ ) || ( CAM_ISP_CQ_NONE != this->CQ_D )){
        //
        if ((ISP_PASS1 == this->path)||(ISP_PASS1_D == this->pathD)) {

            //For PIP Capture, raw or raw-D path will be stop first, and then switch to capture mode, and start again.
            //if (rawCentralCtrl.mEnablePath == 0) will make the 2nd start operation fail due to  CQ0-C CQ0C-D
            //can't be enabled again by this condition check.
            //
            //if (rawCentralCtrl.mEnablePath == 0)
            {
               #if defined(_rtbc_use_cq0c_)
                //no matter Raw-D is enable or not, we will enable CQ0-D here with CQ0
                //When Raw-D is enabled, the camioPipe->Start function will make raw-D start to run,
                //otherwise, Raw-D will no effect even the CQ0D configured as enable
                this->isp_top_ctl.CAM_CTL_CQ_EN.Raw |= (CAM_CTL_CQ_EN_CQ0C_EN|CAM_CTL_CQ_EN_CQ0C_D_EN);
              #endif
                //CQ enable
                ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_CQ_EN, \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_CQ_EN)|this->isp_top_ctl.CAM_CTL_CQ_EN.Raw,ISP_DRV_USER_ISPF);
            }
        }
        else /*if (ISP_PASS2 == this->path)*/ {
            if ( CAM_CTL_CQ_EN_P2_CQ1_EN & this->isp_top_ctl.CAM_CTL_CQ_EN_P2.Raw ) {
                 ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN_P2,CQ1_EN,1,ISP_DRV_USER_ISPF);
            }
            if ( CAM_CTL_CQ_EN_P2_CQ2_EN & this->isp_top_ctl.CAM_CTL_CQ_EN_P2.Raw ) {
                 ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN_P2,CQ2_EN,1,ISP_DRV_USER_ISPF);
            }
            if ( CAM_CTL_CQ_EN_P2_CQ3_EN & this->isp_top_ctl.CAM_CTL_CQ_EN_P2.Raw ) {
                 ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN_P2,CQ3_EN,1,ISP_DRV_USER_ISPF);
            }
        }
        this->ispDrvSwitch2Virtual(this->CQ,this->CQ_D,this->burstQueIdx,this->dupCqIdx);
    }
    //ISP CLK ENABLE
    ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_CLK_EN,0x0000000F,ISP_DRV_USER_ISPF);

    if ( this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Bits.TG_EN ) {
        ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_CLK_EN,0x00008005,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_INT_EN ,this->isp_top_ctl.CAMSV_CAMSV_INT_EN.Raw,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG_VF_CON,SINGLE_MODE,0,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_MODULE_EN,this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_PAK,0x03,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_FMT_SEL,0x01000000, ISP_DRV_USER_ISPF);
    }


    //calling rawCentralCtrl._config(~) after ispDrvSwitch2Virtual(~);
    rawCentralCtrl.config2(this);

    ISP_TOP_CTRL* ptop = rawCentralCtrl.getTopCtrl();
    IspDrv*       raw_IspDrv = rawCentralCtrl.getRawIspDrv();
    IspDrv*      rawD_IspDrv = rawCentralCtrl.getRawDIspDrv();
    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /********************************************************************************************************************/
    MUINT32 p2PathMux=ISP_PASS2|ISP_PASS2B|ISP_PASS2C;

    if ( ISP_PASS1   == this->path || ISP_PASS1_D == this->pathD ) {
        MUINT32 temp = 0;
        //MUINT32 isp_global_set_tmp = 0;

        if ((ISP_PASS1   == this->path) && (NULL == raw_IspDrv)) {
            ISP_FUNC_ERR("Err: NULL == raw_IspDrv");
            return MFALSE;
        }
        if ((ISP_PASS1_D   == this->path) && (NULL == rawD_IspDrv)) {
            ISP_FUNC_ERR("Err: NULL == rawD_IspDrv");
            return MFALSE;
        }

        //INT enable
        //write to physical register due to INT_EN control not include in CQ
        if (ISP_PASS1 == this->path) {
            ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_CQ_EN, \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_CQ_EN)|ptop->isp_top_ctl.CAM_CTL_CQ_EN.Raw,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_INT_P1_EN ,ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_INT_P1_EN)  |ptop->isp_top_ctl.CAM_CTL_INT_P1_EN.Raw,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_INT_P1_EN2,ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_INT_P1_EN2) |ptop->isp_top_ctl.CAM_CTL_INT_P1_EN2.Raw,ISP_DRV_USER_ISPF);

            ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_CON2,0x00202000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            if (MTRUE == isTwinMode)
            {
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_CON2,0x00202000,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-
            }

            //Hw/Sw reset will not clear the 2 registers, it is better to reset MNum here to
            //make sure Kernel ISP_SOF_Buf_Get function can get the valid MNum value
            // for the SOF_0
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_RAW_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);
            ISP_FUNC_INF("[PIP][TopCtrl]ISP_PASS1 Init MNum to 0\n");
        }
        if (ISP_PASS1_D == this->pathD) {
            //CQ enable
            ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_CQ_EN, \
            ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_CQ_EN)|ptop->isp_top_ctl.CAM_CTL_CQ_EN.Raw,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_INT_P1_EN_D ,ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_INT_P1_EN_D)  |ptop->isp_top_ctl.CAM_CTL_INT_P1_EN_D.Raw,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_INT_P1_EN2_D,ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_INT_P1_EN2_D) |ptop->isp_top_ctl.CAM_CTL_INT_P1_EN2_D.Raw,ISP_DRV_USER_ISPF);

            ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_CON2,0x00202000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_LSCI_D_CON,0x80000020,ISP_DRV_USER_ISPF); // ultra-

            //Hw/Sw reset will not clear the 2 registers, it is better to reset MNum here to
            //make sure Kernel ISP_SOF_Buf_Get function can get the valid MNum value
            // for the SOF_0
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_RAW_D_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);
        }

        //common register for Raw&Raw-D, only use CQ0 to config this register to avoid race condition
        temp = ptop->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw;
#if 0
        isp_global_set_tmp = temp;
        if ( ISP_PASS1   == this->path)
        {
            isp_global_set_tmp = isp_global_set_tmp | 0x01;
        }
        else if (ISP_PASS1_D == this->pathD)
        {
            isp_global_set_tmp = isp_global_set_tmp | 0x02;
        }
#endif

        //may caused R/W racing issue in p1_d cfg. but if W to next CmqBuf, may face the prob if p1_D start before CmqBuf addr switch.
        //if force CmqBuf switch before p1_d start, may face tuning para. mismatch issue.
        {
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                _tmpRawIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx, i);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_SEL_GLOBAL_SET, (temp) & this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_SEL_GLOBAL_CLR,(~temp) & this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK,ISP_DRV_USER_ISPF);
                //for twin driver use
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_SEL_GLOBAL, temp, ISP_DRV_USER_ISPF);
                //add here for duplicated cmd,
                ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_SCENARIO,this->isp_top_ctl.CAM_CTL_SCENARIO.Raw,ISP_DRV_USER_ISPF);
            }
        }
        if ((rawCentralCtrl.mEnablePath == ISP_PASS1_D) && (ISP_PASS1_D == this->pathD)) {
           //PIP Case:if the 1st cfg sensor is sub-cam -> write the TopCtrl to hw directly.
           ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL_SET, temp & this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK,ISP_DRV_USER_ISPF);
           ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL_CLR,(~temp) & this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK,ISP_DRV_USER_ISPF);
           ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_SCENARIO, this->isp_top_ctl.CAM_CTL_SCENARIO.Raw,ISP_DRV_USER_ISPF);

           ISP_FUNC_INF("[PIP][TopCtrl]Sub-Cam: CAM_CTL_SCENARIO :[h]0x%x/[s]0x%x\n", \
                                        ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_SCENARIO),\
                                        this->isp_top_ctl.CAM_CTL_SCENARIO.Raw);
        }

        int path[2];
        path[0] = this->path;
        path[1] = this->pathD;
        MUINT32 p1_log[6] = {0};
        ISP_BUFFER_CTRL_STRUCT buf_ctrl;
        //MUINT32 dummy;
        ISP_RT_BUF_INFO_STRUCT dummy;
        p1_log[1] = 1;
        p1_log[2] = function_DbgLogEnable_VERBOSE;
        p1_log[3] = function_DbgLogEnable_INFO;//function_DbgLogEnable_DEBUG;
        p1_log[4] = function_DbgLogEnable_INFO;
        p1_log[5] = p1_log[4];
        for (int ii=0; ii < 2; ii++)  {

           if (ISP_PASS_NONE == path[ii])
                continue;

           switch ( path[ii] ) {
               case ISP_PASS1:
                    //clear kernel status to avoid abnormal exit at previous camera operation
                    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
                    buf_ctrl.buf_id = (_isp_dma_enum_)_imgo_;
                    //buf_ctrl.data_ptr = (MUINT32)&dummy;
                    buf_ctrl.data_ptr = &dummy;
                    buf_ctrl.pExtend = 0;
                    if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                        ISP_FUNC_ERR("ERROR:rtBufCtrl");
                        return -1;
                    }
                    buf_ctrl.buf_id = (_isp_dma_enum_)_rrzo_;
                    if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                        ISP_FUNC_ERR("ERROR:rtBufCtrl");
                        return -1;
                    }
                   //init kernel log flg
                   p1_log[0] = 0;
                   m_pPhyIspDrv->dumpDBGLog(p1_log,NULL);
                   //--TG setting
                    for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                        _tmpRawIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx, i);
                        ISP_WRITE_BITS(_tmpRawIspDrv,CAM_TG_VF_CON,SINGLE_MODE, ptop->b_continuous[ii] ? 0:1,ISP_DRV_USER_ISPF);

                        ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_EN_P1, ptop->isp_top_ctl.CAM_CTL_EN_P1.Raw,ISP_DRV_USER_ISPF);
                        ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_EN_P1_DMA,ptop->isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw,ISP_DRV_USER_ISPF);
                    }
                   //
                   temp = ptop->isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw;
                   for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                        _tmpRawIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx, i);
                        ISP_WRITE_REG(_tmpRawIspDrv, CAM_CTL_FMT_SEL_P1, temp, ISP_DRV_USER_ISPF);
                        ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_SEL_P1,ptop->isp_top_ctl.CAM_CTL_SEL_P1.Raw,ISP_DRV_USER_ISPF);
                        ISP_WRITE_REG(_tmpRawIspDrv,CAM_CTL_P1_DONE_BYP,(0x0),ISP_DRV_USER_ISPF);
                        if ( this->isTwinMode ) {
                            //ISP_WRITE_REG(m_pP1IspDrv,  CAM_CTL_FMT_SEL_P1_D, \
                            //    ( ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_FMT_SEL_P1_D)&(~this->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D_MASK) ) | this->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Raw,ISP_DRV_USER_ISPF);
                            ISP_WRITE_REG(_tmpRawIspDrv,  CAM_CTL_FMT_SEL_P1_D, ptop->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Raw,ISP_DRV_USER_ISPF);
                        }
                        ISP_WRITE_REG(_tmpRawIspDrv,CAM_RAW_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);
                   }

                   ISP_FUNC_INF("[PIP][TopCtrl]ISP_PASS1 P1:EN(0x%x/0x%x),DMA(0x%x/0x%x),fmt(0x%x/0x%x),sel(0x%x/0x%x),BYP(0x%x/0x%x),GLO(0x%x/0x%x/0x%x)\n", \
                       ISP_READ_REG_NOPROTECT(raw_IspDrv,CAM_CTL_EN_P1),\
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1_D),\
                       ISP_READ_REG_NOPROTECT(raw_IspDrv,CAM_CTL_EN_P1_DMA),\
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1_DMA_D),
                       ISP_READ_REG_NOPROTECT(raw_IspDrv,CAM_CTL_FMT_SEL_P1),\
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_FMT_SEL_P1_D),\
                       ISP_READ_REG_NOPROTECT(raw_IspDrv,CAM_CTL_SEL_P1),\
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_SEL_P1_D),\
                       ISP_READ_REG_NOPROTECT(raw_IspDrv,CAM_CTL_P1_DONE_BYP),\
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_P1_DONE_BYP_D),\
                       ISP_READ_REG_NOPROTECT(raw_IspDrv,CAM_CTL_SEL_GLOBAL_SET),\
                       ISP_READ_REG_NOPROTECT(raw_IspDrv,CAM_CTL_SEL_GLOBAL_CLR),\
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL));
                    //inform kernel which dma ch r opened
                    #ifdef _rtbc_buf_que_2_0_
                    {
                        dma_array[_imgo_] = (this->isp_top_ctl.CAM_CTL_EN_P1_DMA.Bits.IMGO_EN)?(1):(0);
                        dma_array[_rrzo_] = (this->isp_top_ctl.CAM_CTL_EN_P1_DMA.Bits.RRZO_EN)?(1):(0);
                        //ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,_rrzo_,(unsigned int)dma_array,0};
                        ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,_rrzo_,0,0,&dma_array[0]};
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
                    }
                    #endif
                   break;
               case ISP_PASS1_D:
                       //clear kernel status to avoid abnormal exit at previous camera operation
                    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
                    buf_ctrl.buf_id = (_isp_dma_enum_)_imgo_d_;
                    buf_ctrl.data_ptr = &dummy;
                    buf_ctrl.pExtend = 0;
                    if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                        ISP_FUNC_ERR("ERROR:rtBufCtrl");
                        return -1;
                    }
                    buf_ctrl.buf_id = (_isp_dma_enum_)_rrzo_d_;
                    if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                        ISP_FUNC_ERR("ERROR:rtBufCtrl");
                        return -1;
                    }
                      //init kernel log flg
                   p1_log[0] = 1;
                   m_pPhyIspDrv->dumpDBGLog(p1_log,NULL);
                   //--TG setting
                   if (NULL == rawD_IspDrv) {
                       ISP_FUNC_ERR("Err: NULL == rawD_IspDrv");
                       return MFALSE;
                   }
                   for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                        _tmpRawDIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx, i);
                       ISP_WRITE_BITS(_tmpRawDIspDrv,CAM_TG2_VF_CON,SINGLE_MODE,ptop->b_continuous[ii]?0:1,ISP_DRV_USER_ISPF);

                       ISP_WRITE_REG(_tmpRawDIspDrv,CAM_CTL_EN_P1_D,ptop->isp_top_ctl.CAM_CTL_EN_P1_D.Raw,ISP_DRV_USER_ISPF);
                       ISP_WRITE_REG(_tmpRawDIspDrv,CAM_CTL_EN_P1_DMA_D,ptop->isp_top_ctl.CAM_CTL_EN_P1_DMA_D.Raw,ISP_DRV_USER_ISPF);

                       //ISP_WRITE_REG(rawD_IspDrv, CAM_CTL_FMT_SEL_P1_D, \
                       //    ( ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_FMT_SEL_P1_D)&(~this->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D_MASK) ) | this->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Raw, ISP_DRV_USER_ISPF);
                       ISP_WRITE_REG(_tmpRawDIspDrv, CAM_CTL_FMT_SEL_P1_D, ptop->isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Raw,ISP_DRV_USER_ISPF);
                       ISP_WRITE_REG(_tmpRawDIspDrv,CAM_CTL_SEL_P1_D,ptop->isp_top_ctl.CAM_CTL_SEL_P1_D.Raw,ISP_DRV_USER_ISPF);
                       ISP_WRITE_REG(_tmpRawDIspDrv,CAM_CTL_P1_DONE_BYP_D,(0x0),ISP_DRV_USER_ISPF);
                       ISP_WRITE_REG(_tmpRawDIspDrv,CAM_RAW_D_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);
                   }
                   #ifdef _rtbc_buf_que_2_0_
                   {
                       dma_array[_imgo_d_] = (this->isp_top_ctl.CAM_CTL_EN_P1_DMA_D.Bits.IMGO_D_EN)?(1):(0);
                       dma_array[_rrzo_d_] = (this->isp_top_ctl.CAM_CTL_EN_P1_DMA_D.Bits.RRZO_D_EN)?(1):(0);
                       //ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,_rrzo_d_,(unsigned int)dma_array,0};
                       ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,_rrzo_d_,0,0,&dma_array[0]};
                       this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
                   }
                   #endif
                   ISP_FUNC_INF("[PIP][TopCtrl]ISP_PASS1_D P1:EN(0x%08x/0x%08x),DMA(0x%08x/0x%08x),fmt(0x%08x/0x%08x),sel(0x%08x/0x%08x),BYP(0x%08x/0x%08x),GLO(0x%08x/0x%08x/0x%08x)\n", \
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1),\
                       ISP_READ_REG_NOPROTECT(rawD_IspDrv,CAM_CTL_EN_P1_D),\
                       ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1_DMA),\
                       ISP_READ_REG_NOPROTECT(rawD_IspDrv,CAM_CTL_EN_P1_DMA_D),
                               ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_FMT_SEL_P1),\
                               ISP_READ_REG_NOPROTECT(rawD_IspDrv,CAM_CTL_FMT_SEL_P1_D),\
                               ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_SEL_P1),\
                               ISP_READ_REG_NOPROTECT(rawD_IspDrv,CAM_CTL_SEL_P1_D),\
                               ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_P1_DONE_BYP),\
                               ISP_READ_REG_NOPROTECT(rawD_IspDrv,CAM_CTL_P1_DONE_BYP_D),\
                               ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL_SET),\
                               ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL_CLR),\
                               ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL));
                   break;
               default:
                   ISP_FUNC_ERR("NOT SUPPORT PATH, path[%d] = %d", ii,  path[ii]);
                   break;
           }
        }

    }
    else if ( ISP_PASS1_CAMSV == this->path || ISP_PASS1_CAMSV_D == this->pathD ) {
        MUINT32 temp = 0;

        if (ISP_PASS1_CAMSV == this->path) {
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_INT_EN,ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_INT_EN)  |ptop->isp_top_ctl.CAMSV_CAMSV_INT_EN.Raw,ISP_DRV_USER_ISPF);
        } else if (ISP_PASS1_CAMSV_D == this->pathD) {
            ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV2_INT_EN,ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_INT_EN)|ptop->isp_top_ctl.CAMSV_CAMSV2_INT_EN.Raw,ISP_DRV_USER_ISPF);
        }

        switch(this->path)
        {
            case ISP_PASS2:
            case ISP_PASS2B:
            case ISP_PASS2C:
                goto RUN_P2_CFG;
                break;
            default:
                break;
        }

        int path[2];
        path[0] = this->path;
        path[1] = this->pathD;
        MUINT32 p1_log[6];
        p1_log[1] = 1;
        p1_log[2] = function_DbgLogEnable_VERBOSE;
        p1_log[3] = function_DbgLogEnable_INFO;//function_DbgLogEnable_DEBUG;
        p1_log[4] = function_DbgLogEnable_INFO;
        p1_log[5] = p1_log[4];
        for (int ii=0; ii < 2; ii++)  {
            ISP_BUFFER_CTRL_STRUCT buf_ctrl;
           //MUINT32 dummy = 0;
           ISP_RT_BUF_INFO_STRUCT dummy;
           if (ISP_PASS_NONE == path[ii])
                continue;

           switch ( path[ii] ) {
           case ISP_PASS1_CAMSV:
                //clear kernel status to avoid abnormal exit at previous camera operation
                buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
                buf_ctrl.buf_id = (_isp_dma_enum_)_camsv_imgo_;
                buf_ctrl.data_ptr = &dummy;
                buf_ctrl.pExtend = 0;
                if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                    ISP_FUNC_ERR("ERROR:rtBufCtrl");
                    return -1;
                }
               ISP_FUNC_INF("ISP_PASS1_CAMSV");
               //init kernel log flg
               p1_log[0] = 2;
               m_pPhyIspDrv->dumpDBGLog(p1_log,NULL);
               //--TG setting
               ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG_VF_CON,SINGLE_MODE,this->b_continuous?0:1,ISP_DRV_USER_ISPF);
               ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_MODULE_EN,this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw,ISP_DRV_USER_ISPF);
               #ifdef _rtbc_buf_que_2_0_
                dma_array[_camsv_imgo_] = (this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Bits.IMGO_EN_)?(1):(0);
               #endif
               if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_MODULE_EN,TG_EN)){
                 ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_CLK_EN,0x8005,ISP_DRV_USER_ISPF);
               }
               ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV_PAK, \
                     ( ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_PAK)&(~this->isp_top_ctl.CAMSV_CAMSV_PAK_MASK) ) | \
                     this->isp_top_ctl.CAMSV_CAMSV_PAK.Raw,ISP_DRV_USER_ISPF);
                     ISP_WRITE_REG(m_pPhyIspDrv, \
                     CAMSV_CAMSV_FMT_SEL, \
                     ( ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_FMT_SEL)&(~this->isp_top_ctl.CAMSV_CAMSV_FMT_SEL_MASK) ) | \
                     this->isp_top_ctl.CAMSV_CAMSV_FMT_SEL.Raw, ISP_DRV_USER_ISPF);
               break;
           case ISP_PASS1_CAMSV_D:
                //clear kernel status to avoid abnormal exit at previous camera operation
                buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
                buf_ctrl.buf_id = (_isp_dma_enum_)_camsv2_imgo_;
                buf_ctrl.data_ptr = &dummy;
                buf_ctrl.pExtend = 0;
                if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                    ISP_FUNC_ERR("ERROR:rtBufCtrl");
                    return -1;
                }
               ISP_FUNC_INF("ISP_PASS1_CAMSV_D");
               //init kernel log flg
               p1_log[0] = 3;
               m_pPhyIspDrv->dumpDBGLog(p1_log,NULL);
               //--TG setting
               ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG2_VF_CON,SINGLE_MODE,this->b_continuous?0:1,ISP_DRV_USER_ISPF);
               ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV2_MODULE_EN,this->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw,ISP_DRV_USER_ISPF);
               #ifdef _rtbc_buf_que_2_0_
                   dma_array[_camsv2_imgo_] = (this->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Bits.IMGO_EN_)?(1):(0);
               #endif
               if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_MODULE_EN,TG_EN)){
                   ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV2_CLK_EN,0x8005,ISP_DRV_USER_ISPF);
               }
               ISP_WRITE_REG(m_pPhyIspDrv,CAMSV_CAMSV2_PAK, \
                      ( ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_PAK)&(~this->isp_top_ctl.CAMSV_CAMSV2_PAK_MASK) ) | \
                      this->isp_top_ctl.CAMSV_CAMSV2_PAK.Raw,ISP_DRV_USER_ISPF);
                      ISP_WRITE_REG(m_pPhyIspDrv, \
                      CAMSV_CAMSV2_FMT_SEL, \
                      ( ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_FMT_SEL)&(~this->isp_top_ctl.CAMSV_CAMSV2_PAK_MASK) ) | \
                      this->isp_top_ctl.CAMSV_CAMSV2_FMT_SEL.Raw, ISP_DRV_USER_ISPF);
               break;
            default:
               ISP_FUNC_ERR("NOT SUPPORT PATH, path[%d] = %d", ii,  path[ii]);
               break;
           }
        }
        //inform kernel which dma ch r opened
        #ifdef _rtbc_buf_que_2_0_
             //ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ,(unsigned int)dma_array,0};
             ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ,0,0,&dma_array[0]};
             this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
        #endif

        ISP_FUNC_INF("[CAMSV]EN(0x%08x/0x%08x),fmt(0x%08x/0x%08x)\n", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_MODULE_EN),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_MODULE_EN),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_FMT_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_FMT_SEL));

    }

RUN_P2_CFG:
    //ISP_FUNC_INF("p2PathMux(0x%x),this->path (0x%x), & (0x%x)",p2PathMux,this->path,p2PathMux & this->path);
    if ((this->path==ISP_PASS2)||(this->path==ISP_PASS2B)||(this->path==ISP_PASS2C))
    {
        MUINT32 p2TuningEnTag=0x00, p2TuningDmaTag=0x00;
        MUINT32 p2TuningEn=0x00, p2TuningDma=0x00;
        MUINT32 p2TuningP1EnTag=0x00, p2TuningP1DmaTag=0x00;
        MUINT32 p2TuningP1En=0x00, p2TuningP1Dma=0x00;
        MUINT32 p2EnConfig, P2EnDmaConfig;
        MUINT32 p1EnConfig, P1EnDmaConfig;
        MUINT32 ggmEnBit;
        MBOOL isApplyTuning;
        //
        #if 0
        ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_SET,this->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw,ISP_DRV_USER_ISPF); //tmp for interrupt merge
        //
        #else
        //default, mfb_mode in 15004040 should be off
        ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_SET,this->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_CLR,this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK,ISP_DRV_USER_ISPF);
        if(this->isp_top_ctl.CAM_CTL_EN_P2.Raw & CAM_CTL_EN_P2_MIX3_EN)
        {   //mfb mixing, mfb on/mix3 on, mfb_mode in 15004040 should be off
            ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_CLR,this->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw & this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK,ISP_DRV_USER_ISPF);
        }
        else if(this->isp_top_ctl.CAM_CTL_EN_P2.Raw & CAM_CTL_EN_P2_MFB_EN)
        {   //mfb blending, mfb on/mix3 off, mfb_mode in 15004040 should be on
            ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_SET,this->isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw | this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_CLR,ISP_READ_REG_NOPROTECT(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_CLR) & (~this->isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK),ISP_DRV_USER_ISPF);
        }
        #endif
        //
        //ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_GLOBAL_CLR,(~0x0),ISP_DRV_USER_ISPF);
        //
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_INT_P2_EN,ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_INT_P2_EN)|this->isp_top_ctl.CAM_CTL_INT_P2_EN.Raw,ISP_DRV_USER_ISPF);
        //
        if (this->isV3)
        {
            p2EnConfig =  this->isp_top_ctl.CAM_CTL_EN_P2.Raw; //QQ??
            P2EnDmaConfig = this->isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw; //QQ??

            if(this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO==ISP_HW_SCENARIO_IP)
            {
                p1EnConfig = this->isp_top_ctl.CAM_CTL_EN_P1.Raw;//QQ??
                P1EnDmaConfig = this->isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw;//QQ??
            }
            ISP_FUNC_DBG("cq(%d),P2EnDmaConfig(0x%08x),p2EnConfig(0x%08x)",
                    this->CQ,P2EnDmaConfig,p2EnConfig);

        }
        else
        {
        isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
        if (isApplyTuning) {
            if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE){
                ISP_FUNC_ERR("[Error]deTuningQue fail");
                return -1;
            }
            //
            getP2EnTuningTag(MTRUE,
                           (ISP_DRV_CQ_ENUM)this->CQ,
                           this->magicNum,
                           this->drvScenario,
                           this->sub_mode,
                           p2TuningEnTag,
                           p2TuningDmaTag); // get P2 tuning En & Dma Tag
            //
            p2TuningEn = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
            p2TuningDma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);
            //
            if(this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO==ISP_HW_SCENARIO_IP){
                getP1EnTuningTag(MTRUE,
                               (ISP_DRV_CQ_ENUM)this->CQ,
                               this->magicNum,
                               this->drvScenario,
                               this->sub_mode,
                               p2TuningP1EnTag,
                               p2TuningP1DmaTag); // get P2 tuning En & Dma Tag

                p2TuningP1En = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p1, this->CQ, this->magicNum);
                p2TuningP1Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p1_dma, this->CQ, this->magicNum);
            }
        }


        p2EnConfig = ((this->isp_top_ctl.CAM_CTL_EN_P2.Raw & ~p2TuningEnTag)|(p2TuningEn & p2TuningEnTag));
        P2EnDmaConfig = ((this->isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw & ~p2TuningDmaTag)|(p2TuningDma & p2TuningDmaTag));
        //
        // workaround for lce and lcei sync
        // lcei must be enable if lce be enable
//        if((p2EnConfig&CAM_CTL_EN_P2_LCE_EN)&&((P2EnDmaConfig&CAM_CTL_EN_P2_DMA_LCEI_EN)==0)  ) {
//            ISP_FUNC_WRN("lce & lcei not be sync, p2EnConfig(0x%x),P2EnDmaConfig(0x%x),disable lce",p2EnConfig,P2EnDmaConfig);
//            p2EnConfig = (p2EnConfig)&(~CAM_CTL_EN_P2_LCE_EN);
//        }

        //
        if(this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO==ISP_HW_SCENARIO_IP){
            p1EnConfig = ((this->isp_top_ctl.CAM_CTL_EN_P1.Raw & ~p2TuningP1EnTag)|(p2TuningP1En & p2TuningP1EnTag));
            P1EnDmaConfig = ((this->isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw & ~p2TuningP1DmaTag)|(p2TuningP1Dma & p2TuningP1DmaTag));
            ISP_FUNC_INF("p1EnConfig(0x%08x),P1EnDmaConfig(0x%0x)",p1EnConfig,P1EnDmaConfig);
        }

        ISP_FUNC_DBG("cq(%d),isApplyTuning(%d),p2TuningEn(0x%08x),p2TuningEnTag(0x%08x),P2EnDmaConfig(0x%08x),p2TuningDmaTag(0x%08x),p2EnConfig(0x%08x),p2TuningDma(0x%08x)",
                this->CQ,isApplyTuning,p2TuningEn,p2TuningEnTag,P2EnDmaConfig,p2TuningDmaTag,p2EnConfig,p2TuningDma);
        //
        }

        /*** tpipe processing ***/
        /*** SHOULD BE AFTER CQ setting ***/
        ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_EN_P2, p2EnConfig, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_EN_P2_DMA, P2EnDmaConfig, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_SEL_P2, this->isp_top_ctl.CAM_CTL_SEL_P2.Raw, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_FMT_SEL_P2, this->isp_top_ctl.CAM_CTL_FMT_SEL_P2.Raw, ISP_DRV_USER_ISPF);
        //
        // GGM control by reg 0x5480
        ggmEnBit = ((p2EnConfig & CAM_CTL_EN_P2_GGM_EN)?(0x01):(0x00));
        ISP_WRITE_REG(m_pP2IspDrv, CAM_GGM_CTRL, ggmEnBit, ISP_DRV_USER_ISPF);
        //
        // for scenario IP
        if(this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO==ISP_HW_SCENARIO_IP){
            ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_EN_P1, p1EnConfig,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_EN_P1_DMA, P1EnDmaConfig,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_FMT_SEL_P1, this->isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw, ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_SEL_P1,this->isp_top_ctl.CAM_CTL_SEL_P1.Raw,ISP_DRV_USER_ISPF);
        }
        if(isApplyTuning==MTRUE){
            if (!this->isV3){
            if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE){
                ISP_FUNC_ERR("[Error]enTuningQue fail");
            }
        }
    }
    }

    //common
    ISP_WRITE_REG(pCommonIspDrv,CAM_CTL_SCENARIO,this->isp_top_ctl.CAM_CTL_SCENARIO.Raw,ISP_DRV_USER_ISPF);
    //
    ISP_FUNC_DBG("-");
    return 0;
}


int ISP_TOP_CTRL::_enable( void* pParam  )
{

    MUINT32 *start = (MUINT32*)pParam;
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 size;
    MINT32 errSta;
    IspDrv* _tmpIspDrv  = NULL;
    ISP_DRV_WAIT_IRQ_STRUCT irq_TG_ClrDone;
    irq_TG_ClrDone.Clear=ISP_DRV_IRQ_CLEAR_STATUS;
    irq_TG_ClrDone.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
    irq_TG_ClrDone.Status=CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    irq_TG_ClrDone.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
    irq_TG_ClrDone.Timeout=CAM_INT_WAIT_TIMEOUT_MS;
    irq_TG_ClrDone.UserName="pass1";
    irq_TG_ClrDone.irq_TStamp=0x0;
    irq_TG_ClrDone.bDumpReg=    0x0;
    irq_TG_ClrDone.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
    irq_TG_ClrDone.UserInfo.Status=CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    irq_TG_ClrDone.UserInfo.UserKey=0;

    // added for checking CQ status
    MUINT32 CQ_status=0, CQ_debugset=0;
    MBOOL readCQstatus = MFALSE;
    Mutex   *_localVar;
    // lock for pass1 and pass2 respectively
    if ( CAM_ISP_PASS2_START == *start || CAM_ISP_PASS2B_START == *start || CAM_ISP_PASS2C_START == *start ){
        ISP_FUNC_INF("path(%d),start(%d),isP2IspOnly(%d),dupCqIdx(%d),burstQueIdx(%d),SCEN(%d)",this->path,*start,this->isP2IspOnly,this->dupCqIdx,this->burstQueIdx,this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO);
        _localVar = &this->m_pIspDrvShell->gPass2Lock;
    } else {
        if( ( CAM_CTL_EN_P1_TG1_EN & this->isp_top_ctl.CAM_CTL_EN_P1.Raw) || ( CAM_ISP_PASS1_CQ0_START == (*start) ) || ( CAM_ISP_PASS1_CQ0B_START == (*start) ))
            _localVar = &this->m_pIspDrvShell->gPass1Lock;
        else
            _localVar = &this->m_pIspDrvShell->gPass1_dLock;
    }
    Mutex::Autolock lock(_localVar);
    //if( CAM_ISP_PASS2_START == *start || CAM_ISP_PASS2B_START == *start || CAM_ISP_PASS2C_START == *start )
    //{
        //ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_DBG_SET,0x6000,ISP_DRV_USER_ISPF);
        //readCQstatus = MTRUE;
    //}
    //

    #if 0 //kk test default:0
    if (this->m_pIspDrvShell->m_pIMemDrv)    // If this->m_pIspDrvShell->m_pIMemDrv not null (i.e. not GDMA mode). // No IMem in GDMA mode so don't have to flush.
    {
        //m4u flush before HW working
        this->m_pIspDrvShell->m_pIMemDrv->cacheFlushAll();
    }
    #endif

    //
    if( readCQstatus )
    {
        CQ_status=ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_DBG_PORT);
        CQ_debugset=ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_DBG_SET);
    }
    //
    if ( CAM_ISP_PASS1_START == (*start) ) {

#if 0 //defined(_rtbc_use_cq0c_)
        //fbc enable before VF_EN
        //IMGO
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE;
        buf_ctrl.buf_id = _imgo_;
        buf_ctrl.data_ptr = (MUINT32)&size;
        ISP_FUNC_INF("ctrl(%d),id(%d),ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
        if ( MTRUE != this->m_pIspDrvShell->getPhyIspDrv()->rtBufCtrl((void*)&buf_ctrl) ) {
            ISP_FUNC_ERR("ERROR:rtBufCtrl");
            return -1;
        }
        ISP_FUNC_INF("dma(%d),size(%d)",_imgo_,size);

        //ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, DROP_INT_EN, 1);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,FB_NUM,size);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,FBC_EN,0);
        usleep(10);
        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,FBC_EN,1);
        //IMG2O
        if ( CAM_CTL_EN_P1_DMA_RRZO_EN & this->isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw) {
            buf_ctrl.buf_id = _rrzo_;
            buf_ctrl.data_ptr = (MUINT32)&size;
            ISP_FUNC_INF("ctrl(%d),id(%d),ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->getPhyIspDrv()->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }

            ISP_FUNC_INF("dma(%d),size(%d)",_rrzo_,size);

            //ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_RRZO_FBC, DROP_INT_EN, 1);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,FB_NUM,size);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,FBC_EN,0);
            usleep(10);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,FBC_EN,1);
        }

        //SENINF1_INT_EN
        this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x00008014, 0x0000007F);    /* 0x150048A0: CAM_CFA_BYPASS */

        //SENINF1_CTRL //allan
        //FIFO overrun reset enable
        //this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x00008010,
              //this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00008010) | (0x1 << 11) );
        ISP_FUNC_DBG("FBC uses CQ0C");
        //enable CQ0C for rtbc
        if ( 1 < size ) {
            ISP_FUNC_DBG("ENABLE CAM_ISP_CQ0C(0x%x)",this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C)+ sizeof(CQ_RING_CMD_ST));
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_CQ0C_BASEADDR,\
                (unsigned int)( this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C) + sizeof(CQ_RING_CMD_ST) ),ISP_DRV_USER_ISPF);
        }
        else {
            ISP_FUNC_DBG("ENABLE CAM_ISP_CQ0C(0x%x)",this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C));
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_CQ0C_BASEADDR,\
                (unsigned int)( this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C)),ISP_DRV_USER_ISPF );
        }
        //
        ISP_FUNC_DBG("FB_NUM:(%d/%d)",ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,FB_NUM),\
            ISP_READ_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,FB_NUM));
#endif
        //CAMSV FBC
        if( CAMSV_CAMSV_MODULE_IMGO_EN & this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw){
#if 0 //disable camsv_imgo fbc
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE;
            buf_ctrl.buf_id = _camsv_imgo_;
            buf_ctrl.data_ptr = (MUINT32)&size;
            ISP_FUNC_INF("ctrl(%d),id(%d),ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->getPhyIspDrv()->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            ISP_FUNC_INF("dma(%d),size(%d)",_camsv_imgo_,size);

            //ISP_WRITE_BITS(this->m_pPhyIspReg, CAMSV_CAMSV_IMGO_FBC, DROP_INT_EN, 1);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV_IMGO_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV_IMGO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
            usleep(10);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV_IMGO_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
#endif
        }
        //CAMSV2 FBC
        if( CAMSV_CAMSV2_MODULE_IMGO_EN & this->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw){
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE;
            buf_ctrl.buf_id = _camsv2_imgo_;
            buf_ctrl.data_ptr = 0;
            buf_ctrl.pExtend = (unsigned char*)&size;
            ISP_FUNC_INF("ctrl(%d),id(%d),ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->getPhyIspDrv()->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            ISP_FUNC_INF("dma(%d),size(%d)",_camsv2_imgo_,size);

            //ISP_WRITE_BITS(this->m_pPhyIspReg, CAMSV_CAMSV2_IMGO_FBC, DROP_INT_EN, 1);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV2_IMGO_FBC,FB_NUM,size,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV2_IMGO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
            usleep(10);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV2_IMGO_FBC,FBC_EN,1,ISP_DRV_USER_ISPF);
        }
        //TG1
        if ( CAM_CTL_EN_P1_TG1_EN & this->isp_top_ctl.CAM_CTL_EN_P1.Raw) {
              //
              if ( MFALSE == this->m_pPhyIspDrv->waitIrq( &irq_TG_ClrDone )) {
                   ISP_FUNC_ERR("clr irq status fail");
                   return -1;
               }
              for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                  _tmpIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx, i);
                  ISP_WRITE_REG(_tmpIspDrv,CAM_CTL_SEL_GLOBAL_SET, 0x01);
              }
              ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL_SET, 0x1, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG_VF_CON, VFDATA_EN, 1,ISP_DRV_USER_ISPF);
        }
        //TG1_D
        if ( CAM_CTL_EN_P1_D_TG1_D_EN & this->isp_top_ctl.CAM_CTL_EN_P1_D.Raw) {
              irq_TG_ClrDone.Status = CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
              irq_TG_ClrDone.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
              irq_TG_ClrDone.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
              irq_TG_ClrDone.UserName = "pass1_d";
              //
              if ( MFALSE == this->m_pPhyIspDrv->waitIrq( &irq_TG_ClrDone )) {
                   ISP_FUNC_ERR("clr irq status fail");
                   return -1;
              }
              for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                  _tmpIspDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx, i);
                  ISP_WRITE_REG(_tmpIspDrv,CAM_CTL_SEL_GLOBAL_SET, 0x02);
              }
              ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_SEL_GLOBAL_SET, 0x2, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG2_VF_CON, VFDATA_EN, 1,ISP_DRV_USER_ISPF);
        }

        //CAMSV
        if( CAMSV_CAMSV_MODULE_TG_EN & this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw){
              irq_TG_ClrDone.Status = CAMSV_CAMSV_INT_STATUS_PASS1_DON_ST;
              irq_TG_ClrDone.Type = ISP_DRV_IRQ_TYPE_INT_CAMSV;
              irq_TG_ClrDone.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
              irq_TG_ClrDone.UserName = "camsv";
              //
              if ( MFALSE == this->m_pPhyIspDrv->waitIrq( &irq_TG_ClrDone )) {
                  ISP_FUNC_ERR("clr irq status fail");
                  return -1;
              }
              ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG_VF_CON, VFDATA_EN, 1,ISP_DRV_USER_ISPF);
         }
         //CAMSV2
         if( CAMSV_CAMSV2_MODULE_TG_EN & this->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw){
               irq_TG_ClrDone.Status = CAMSV_CAMSV2_INT_STATUS_PASS1_DON_ST;
               irq_TG_ClrDone.Type = ISP_DRV_IRQ_TYPE_INT_CAMSV2;
               irq_TG_ClrDone.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
               irq_TG_ClrDone.UserName = "camsv_d";
               //
               if ( MFALSE == this->m_pPhyIspDrv->waitIrq( &irq_TG_ClrDone )) {
                    ISP_FUNC_ERR("clr irq status fail");
                    return -1;
               }
               ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG2_VF_CON, VFDATA_EN, 1,ISP_DRV_USER_ISPF);
          }
          if ( MFALSE == this->m_pPhyIspDrv->ISPWakeLockCtrl(MTRUE)) {
              ISP_FUNC_ERR("ISPWakeLockCtrl enable fail!!");
              return -1;
         }
    }
    else if ( CAM_ISP_PASS2_START == *start ) {
        if( (CQ_status & 0x0000000F) != 0x001 )
        {
            //ISP_FUNC_ERR("CQ1 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        //
        if(this->isP2IspOnly){
            MBOOL ret = MTRUE;
            ISP_DRV_WAIT_IRQ_STRUCT irqPass2Done;
            irqPass2Done.Clear=ISP_DRV_IRQ_CLEAR_NONE;
            irqPass2Done.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;
            irqPass2Done.Status=CAM_CTL_INT_P2_STATUS_PASS2_DON_ST;
            irqPass2Done.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
            irqPass2Done.Timeout=CAM_INT_PASS2_WAIT_TIMEOUT_MS;
            irqPass2Done.UserName="isp_function";
            irqPass2Done.irq_TStamp=0;
            irqPass2Done.bDumpReg=0x0;
                irqPass2Done.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;
                irqPass2Done.UserInfo.Status=CAM_CTL_INT_P2_STATUS_PASS2_DON_ST;
                irqPass2Done.UserInfo.UserKey=0;
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000001,ISP_DRV_USER_ISPF);
        }else{
            errSta = g_pMdpMgr->startMdp(gpMdpMgrCfgData[this->burstQueIdx][ISP_DRV_P2_CQ1][this->dupCqIdx]);
            if(MDPMGR_NO_ERROR != errSta){
                ISP_FUNC_ERR("pass2 CQ1 g_pMdpMgr->startMdp fail(%d)",errSta);
                return -1;
            }
        }
    }
    else if ( CAM_ISP_PASS2B_START == *start ) {
        if( (CQ_status & 0x000000F0) != 0x010 )
        {
            //ISP_FUNC_ERR("CQ2 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        //
        if(this->isP2IspOnly){
            MBOOL ret = MTRUE;
            ISP_DRV_WAIT_IRQ_STRUCT irqPass2Done;
            irqPass2Done.Clear=ISP_DRV_IRQ_CLEAR_NONE;
            irqPass2Done.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;
            irqPass2Done.Status=CAM_CTL_INT_P2_STATUS_PASS2_DON_ST;
            irqPass2Done.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
            irqPass2Done.Timeout=CAM_INT_PASS2_WAIT_TIMEOUT_MS;
            irqPass2Done.UserName="isp_function";
            irqPass2Done.irq_TStamp=0;
            irqPass2Done.bDumpReg=0x0;
            irqPass2Done.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;
                irqPass2Done.UserInfo.Status=CAM_CTL_INT_P2_STATUS_PASS2_DON_ST;
                irqPass2Done.UserInfo.UserKey=0;
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000002,ISP_DRV_USER_ISPF);
        }else{
            errSta = g_pMdpMgr->startMdp(gpMdpMgrCfgData[this->burstQueIdx][ISP_DRV_P2_CQ2][this->dupCqIdx]);
            if(MDPMGR_NO_ERROR != errSta){
                ISP_FUNC_ERR("pass2 CQ2 g_pMdpMgr->startMdp fail(%d)",errSta);
                return -1;
            }
        }
    }
    else if ( CAM_ISP_PASS2C_START == *start ) {
        if( (CQ_status & 0x00000F00) != 0x100 )
        {
            //ISP_FUNC_ERR("CQ3 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        //
        if(this->isP2IspOnly){
            MBOOL ret = MTRUE;
            ISP_DRV_WAIT_IRQ_STRUCT irqPass2Done;
           irqPass2Done.Clear=ISP_DRV_IRQ_CLEAR_NONE;
            irqPass2Done.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;
            irqPass2Done.Status=CAM_CTL_INT_P2_STATUS_PASS2_DON_ST;
            irqPass2Done.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
            irqPass2Done.Timeout=CAM_INT_PASS2_WAIT_TIMEOUT_MS;
            irqPass2Done.UserName="isp_function";
            irqPass2Done.irq_TStamp=0;
            irqPass2Done.bDumpReg=0x0;
            irqPass2Done.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P2_ST;
            irqPass2Done.UserInfo.Status=CAM_CTL_INT_P2_STATUS_PASS2_DON_ST;
            irqPass2Done.UserInfo.UserKey=0;
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000010,ISP_DRV_USER_ISPF);

        }else{
            errSta = g_pMdpMgr->startMdp(gpMdpMgrCfgData[this->burstQueIdx][ISP_DRV_P2_CQ3][this->dupCqIdx]);
            if(MDPMGR_NO_ERROR != errSta){
                ISP_FUNC_ERR("pass2 CQ3 g_pMdpMgr->startMdp fail(%d)",errSta);
                return -1;
            }
        }
    }
    else if ( CAM_ISP_PASS1_CQ0_START == *start ) {
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000020,ISP_DRV_USER_ISPF);
    }
    else if ( CAM_ISP_PASS1_CQ0B_START == *start ) {
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000040,ISP_DRV_USER_ISPF);
    }
    else if ( CAM_ISP_PASS1_CQ0_D_START == *start ) {
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00001000,ISP_DRV_USER_ISPF);
    }
    else if ( CAM_ISP_PASS1_CQ0B_D_START == *start ) {
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00002000,ISP_DRV_USER_ISPF);
    }

    //register dump after start()
    if ( MTRUE == function_DbgLogEnable_INFO) {
    static int pass2_cnt = 0;

        ISP_FUNC_VRB("dumpReg");
        if (CAM_ISP_PASS1_START == *start) {
            usleep(1000);
            //pass1 DMA base_addr
            ISP_FUNC_INF("P1_dma_en(0x%x),addr(0x%x)(0x%x)(0x%x)(0x%x)", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1_DMA),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMGO_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMGO_OFST_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_RRZO_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_RRZO_OFST_ADDR));
            //CAMSV DMA base_addr
            ISP_FUNC_INF("camsv: imgo_dma_en(0x%x/0x%x),addr(0x%x/0x%x)(0x%x/0x%x)", \
                ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_MODULE_EN,IMGO_EN_),\
                ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV2_MODULE_EN,IMGO_EN_), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_IMGO_SV_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_IMGO_SV_D_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_IMGO_SV_OFST_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_IMGO_SV_D_OFST_ADDR));
            //
            if ( MTRUE == function_DbgLogEnable_VERBOSE) {
                usleep(1000);
                this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();
                pass2_cnt = 0;
            }
        }
        else if ( CAM_ISP_PASS2_START == *start ||
                  CAM_ISP_PASS2B_START == *start ||
                  CAM_ISP_PASS2C_START == *start) {
            //usleep(1000);
            //pass2 DMA base_addr
            ISP_FUNC_DBG("P2_dma_en(0x%x),imgiPa(0x%x),imgiOffset(0x%x),img2oPa(0x%x),img2oOffset(0x%x),img3oPa(0x%x),img3oOffset(0x%x)", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P2_DMA),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMGI_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMGI_OFST_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMG2O_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMG2O_OFST_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMG3O_BASE_ADDR),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_IMG3O_OFST_ADDR));
            if ( 10==pass2_cnt && MTRUE == function_DbgLogEnable_VERBOSE) {
                this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();
            }
        }
        pass2_cnt++;
    }
    //
    return 0;

}

int ISP_TOP_CTRL::_disable( void )
{
    MBOOL ret = MTRUE;
    MINT32 errSta;

    ISP_DRV_WAIT_IRQ_STRUCT irq_vs_int;
    irq_vs_int.Clear= ISP_DRV_IRQ_CLEAR_WAIT;
    irq_vs_int.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
    irq_vs_int.Status=CAM_CTL_INT_P1_STATUS_VS1_INT_ST;
    irq_vs_int.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
    irq_vs_int.Timeout=CAM_INT_WAIT_TIMEOUT_MS;
    irq_vs_int.UserName="TOP_CTL";
    irq_vs_int.irq_TStamp=0x0;
    irq_vs_int.bDumpReg=0x0;
    irq_vs_int.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
    irq_vs_int.UserInfo.Status=CAM_CTL_INT_P1_STATUS_VS1_INT_ST;
    irq_vs_int.UserInfo.UserKey=0;
    ISP_DRV_WAIT_IRQ_STRUCT irq_vs2_int;
    irq_vs2_int.Clear= ISP_DRV_IRQ_CLEAR_WAIT;
    irq_vs2_int.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    irq_vs2_int.Status=CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
    irq_vs2_int.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
    irq_vs2_int.Timeout=CAM_INT_WAIT_TIMEOUT_MS;
    irq_vs2_int.UserName="TOP_CTL";
    irq_vs2_int.irq_TStamp=0x0;
    irq_vs2_int.bDumpReg=0x0;
    irq_vs2_int.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    irq_vs2_int.UserInfo.Status=CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
    irq_vs2_int.UserInfo.UserKey=0;
    ISP_DRV_WAIT_IRQ_STRUCT irq_csv_int;
    irq_csv_int.Clear= ISP_DRV_IRQ_CLEAR_WAIT;
    irq_csv_int.Type=ISP_DRV_IRQ_TYPE_INT_CAMSV;
    irq_csv_int.Status=CAMSV_CAMSV_INT_STATUS_PASS1_DON_ST;
    irq_csv_int.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
    irq_csv_int.Timeout=CAM_INT_WAIT_TIMEOUT_MS;
    irq_csv_int.UserName="TOP_CTL";
    irq_csv_int.irq_TStamp=0x0;
    irq_csv_int.bDumpReg=0x0;
    irq_csv_int.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_CAMSV;
    irq_csv_int.UserInfo.Status=CAMSV_CAMSV_INT_STATUS_PASS1_DON_ST;
    irq_csv_int.UserInfo.UserKey=0;
    ISP_DRV_WAIT_IRQ_STRUCT irq_csv2_int;
    irq_csv2_int.Clear=ISP_DRV_IRQ_CLEAR_WAIT;
    irq_csv2_int.Type=ISP_DRV_IRQ_TYPE_INT_CAMSV2;
    irq_csv2_int.Status=CAMSV_CAMSV2_INT_STATUS_PASS1_DON_ST;
    irq_csv2_int.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
    irq_csv2_int.Timeout=CAM_INT_WAIT_TIMEOUT_MS;
    irq_csv2_int.UserName="TOP_CTL";
    irq_csv2_int.irq_TStamp=0x0;
    irq_csv2_int.bDumpReg=0x0;
    irq_csv2_int.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_CAMSV2;
    irq_csv2_int.UserInfo.Status=CAMSV_CAMSV2_INT_STATUS_PASS1_DON_ST;
    irq_csv2_int.UserInfo.UserKey=0;
    ISP_DRV_WAIT_IRQ_STRUCT *pactive_irq_vs_int[2]  = {NULL,NULL};

    MBOOL bP1Path = ( ISP_PASS1         == this->path  || \
                      ISP_PASS1_D       == this->pathD || \
                      ISP_PASS1_CAMSV   == this->path  || \
                      ISP_PASS1_CAMSV_D == this->path );


    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    //MUINT32 dummy;
    ISP_RT_BUF_INFO_STRUCT dummy;
    MUINT32 wait_vs_num = 1;
    MBOOL   disable_2_path = bP1Path && (ISP_PASS_NONE!= this->path) && (ISP_PASS_NONE!= this->pathD);
    Mutex   *_localVar;
    ISP_FUNC_INF("ISP_TOP_CTRL::_disable:E path(%x/%x)",this->path,this->pathD);

    // lock for pass1 and pass2 respectively
    if(ISP_PASS1 == this->path)
        _localVar = &this->m_pIspDrvShell->gPass1TopCtrlLock;
    else if(ISP_PASS1_D == this->pathD || ISP_PASS1_CAMSV == this->path || ISP_PASS1_CAMSV_D == this->pathD)
        _localVar = &this->m_pIspDrvShell->gPass1TopCtrlLock;//&this->m_pIspDrvShell->gPass1_dLock; // avoid stop have racing issue under multi-sensor(2nd stop use reset when 1st stop r during fbc workaround rst)
    else {
        _localVar = &this->m_pIspDrvShell->gPass2Lock;
    }
    Mutex::Autolock lock(_localVar);
    if (ISP_PASS1 == this->path) {
         pactive_irq_vs_int[0] = &irq_vs_int;
    } else if (ISP_PASS1_D == this->pathD) {
         pactive_irq_vs_int[0] = &irq_vs2_int;
    } else if (ISP_PASS1_CAMSV == this->path) {
         pactive_irq_vs_int[0] = &irq_csv_int;
    } else if (ISP_PASS1_CAMSV_D == this->path) {
         pactive_irq_vs_int[0] = &irq_csv2_int;
    }

    if ( bP1Path ) {

        if (ISP_PASS1 == this->path) {

            //IspDrv* mRawDIspDrv = pSrcTopCtrl->m_pIspDrvShell->ispDrvGetCQIspDrv(ISP_DRV_CQ0_D/*mTopCtrl.CQ_D*/,0/*mTopCtrl.burstQueIdx*/, 0/*mTopCtrl.dupCqIdx*/);

            rawCentralCtrl.clearPath(ISP_PASS1);
            //Raw
            if ( CAM_CTL_EN_P1_TG1_EN & this->isp_top_ctl.CAM_CTL_EN_P1.Raw) {
                ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN,0,ISP_DRV_USER_ISPF);
            }

            //no matter camsv imgo enable or not.
            //if( CAMSV_CAMSV_MODULE_TG_EN & this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw)
            {
                ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG_VF_CON, VFDATA_EN, 0,ISP_DRV_USER_ISPF);
                ISP_FUNC_INF("cam_sv vf:(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_TG_VF_CON));
                ISP_FUNC_INF("cam_sv module_en:(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_CAMSV_MODULE_EN));
            }

            ISP_FUNC_INF("[TG1]wait %d VD till DMA stop",wait_vs_num);
            //
            {
                while ((pactive_irq_vs_int[0]) && ( wait_vs_num-- )) {
                    ret = this->m_pPhyIspDrv->waitIrq( pactive_irq_vs_int[0] );
                    if ( MFALSE == ret ) {
                        ISP_FUNC_ERR("waitIrq( irq_TG1_DONE ) fail");
                    }
                    else{
                        //check if TG1 is IDLE
                        if(ISP_READ_BITS_NOPROTECT(this->m_pPhyIspDrv,CAM_TG_INTER_ST,TG_CAM_CS) != 1){
                            wait_vs_num++;
                            ISP_FUNC_INF("[TG1]TG is not idle, wait %d VD again.\n", wait_vs_num);
                            continue;
                         }
                    }
                }
            }

            #if defined(_rtbc_use_cq0c_)
                //fbc disable before VF_EN
                //disable IMGO + RRZO + rtbc
                IspDrv* _virDrv = NULL;
                for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                    _virDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ, this->burstQueIdx, i);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_IMGO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_RRZO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_CQ_EN,CQ0C_EN  ,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_CQ_EN,CQ0_EN   ,0,ISP_DRV_USER_ISPF);

                    #ifdef _CmdQ_HwBug_Patch_
                    if (rawCentralCtrl.mEnablePath == 0) {
                        ISP_WRITE_BITS(_virDrv,CAM_CTL_CQ_EN,CQ0C_D_EN ,0,ISP_DRV_USER_ISPF);
                        ISP_WRITE_BITS(_virDrv,CAM_CTL_CQ_EN,CQ0_D_EN ,0,ISP_DRV_USER_ISPF);
                    }
                    #endif

///////////////////S remove later after fbc ECO
                    ISP_WRITE_REG(_virDrv,CAM_CTL_EN_P1_DMA, 0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_virDrv,CAM_CTL_EN_P1, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_virDrv,CAM_RAW_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);
///////////////////E

                }

                ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0C_EN  ,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0_EN   ,0,ISP_DRV_USER_ISPF);
                #ifdef _CmdQ_HwBug_Patch_
                if (rawCentralCtrl.mEnablePath == 0) {
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0C_D_EN ,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0_D_EN ,0,ISP_DRV_USER_ISPF);
                }
                #endif
                ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);

///////////////////S remove later after fbc ECO
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_EN_P1_DMA, 0,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_EN_P1, 1,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_RAW_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);

                if (rawCentralCtrl.mEnablePath != 0) {  //workaround

                    //the other path is still alive, we can't do the sw reset.
                    //use this method to reset FBC

                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG_VF_CON,SINGLE_MODE,1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN,1,ISP_DRV_USER_ISPF);
                    ISP_FUNC_INF("wait2 VD till FBC Rst");

                    {
                        wait_vs_num = 2;
                        while ((pactive_irq_vs_int[0]) && ( wait_vs_num-- )) {
                            ret = this->m_pPhyIspDrv->waitIrq( pactive_irq_vs_int[0] );
                            if ( MFALSE == ret ) {
                                ISP_FUNC_ERR("waitIrq( irq_TG1_DONE ) fail");
                            }
                        }
                        //Polling
                        wait_vs_num = 10;
                        MUINT32 temp = (ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_FBC))&0xFF0000F;
                        while ((temp != 0x1100000) && (wait_vs_num-- > 0)) {
                                ISP_FUNC_INF("[PIP]RRZO FBC=0x%x", temp);
                                temp = (ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_FBC))&0xFF0000F;
                        }
                    }

                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG_VF_CON,SINGLE_MODE,0,ISP_DRV_USER_ISPF);
#ifdef    _FBC_IN_IPRAW
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_FBC,FB_NUM,1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_FBC,FB_NUM,1,ISP_DRV_USER_ISPF);
#endif
                }
///////////////////E

                ISP_FUNC_INF("[PIP][TopCtrl:Disable]P1_en(0x%x/0x%x),CQ-En(0x%x),FBC(0x%x/0x%x),camsv(0x%x)", \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1_DMA),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_CQ_EN),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_IMGO_FBC),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_FBC),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAMSV_TG_VF_CON));


                if (this->isTwinMode) {
                     //disable IMGO_D + RRZO_D + rtbc
                     ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_D_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                     ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                     ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0C_D_EN  ,0,ISP_DRV_USER_ISPF);
                }
            #endif
            #ifdef _rtbc_buf_que_2_0_
            dma_array[_imgo_] = 0;
            dma_array[_rrzo_] = 0;
            //ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ,(unsigned int)dma_array,0};
            ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)_imgo_,0,0,&dma_array[0]};
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
            #endif
            //reset p1 cmdQ descriptor
            for(int j=0;j<ISP_DRV_P1_CQ_DUPLICATION_NUM;j++){
                for(int i=0; i<CAM_DUMMY_; i++){
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->cqDelModule(this->CQ, this->burstQueIdx, j, (CAM_MODULE_ENUM)i);
                }
            }
        }


        if (ISP_PASS1_D == this->pathD) {

            rawCentralCtrl.clearPath(ISP_PASS1_D);
            if ( CAM_CTL_EN_P1_D_TG1_D_EN & this->isp_top_ctl.CAM_CTL_EN_P1_D.Raw ) {
                ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN,0,ISP_DRV_USER_ISPF);
            }

            ISP_FUNC_INF("[TG2]wait %d VD till DMA stop",wait_vs_num);
            //
            {
                wait_vs_num = 1;
                while ((pactive_irq_vs_int[0]) && ( wait_vs_num-- )) {
                    ret = this->m_pPhyIspDrv->waitIrq( pactive_irq_vs_int[0] );
                    if ( MFALSE == ret ) {
                        ISP_FUNC_ERR("waitIrq( irq_TG2_DONE ) fail");
                    }
                    else{
                        //check if TG2 is IDLE
                        if(ISP_READ_BITS_NOPROTECT(this->m_pPhyIspDrv,CAM_TG2_INTER_ST,TG_CAM_CS) != 1){
                            wait_vs_num++;
                            ISP_FUNC_INF("[TG2]TG is not idle, wait %d VD again.\n", wait_vs_num);
                            continue;
                         }
                    }
                }
            }

            #if defined(_rtbc_use_cq0c_)
                //fbc disable before VF_EN
                //disable IMGO + RRZO + rtbc
                IspDrv* _virDrv = NULL;
                for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                    _virDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D, this->burstQueIdx, i);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_IMGO_D_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_RRZO_D_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_CQ_EN,CQ0C_D_EN  ,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_virDrv,CAM_CTL_CQ_EN,CQ0_D_EN   ,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_virDrv,CAM_CTL_EN_P1_DMA_D, 0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_virDrv,CAM_CTL_EN_P1_D, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_virDrv,CAM_RAW_D_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);
                }

                {  //workaround
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0C_D_EN  ,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0_D_EN   ,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_D_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_EN_P1_DMA_D, 0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_EN_P1_D, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(m_pPhyIspDrv,CAM_RAW_D_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);

#ifdef _CmdQ_HwBug_Patch_
                    if (rawCentralCtrl.mEnablePath == 0) {
                        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0C_EN  ,0,ISP_DRV_USER_ISPF);
                        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_CQ_EN,CQ0_EN   ,0,ISP_DRV_USER_ISPF);
                    }
#endif
                    //
                    if (rawCentralCtrl.mEnablePath != 0){
                        //the other path is still alive, we can't do the sw reset.
                        //use this method to reset FBC
                        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG2_VF_CON,SINGLE_MODE,1,ISP_DRV_USER_ISPF);
                        ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN,1,ISP_DRV_USER_ISPF);
                        ISP_FUNC_INF("D:wait2 VD till FBC Rst");

                        wait_vs_num = 2;
                        while ((pactive_irq_vs_int[0]) && ( wait_vs_num-- )) {
                            ret = this->m_pPhyIspDrv->waitIrq( pactive_irq_vs_int[0] );
                            if ( MFALSE == ret ) {
                                ISP_FUNC_ERR("waitIrq( irq_TG2_DONE ) fail");
                            }
                        }

                        //Polling to check
                        wait_vs_num = 10;
                        MUINT32 temp = (ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC))&0xFF0000F;
                        while ((temp != 0x1100000) && (wait_vs_num-- > 0)) {
                                ISP_FUNC_INF("[PIP]FBC_D=0x%x", temp);
                                temp = (ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC))&0xFF0000F;
                        }
                    }
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN,0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_TG2_VF_CON,SINGLE_MODE,0,ISP_DRV_USER_ISPF);
#ifdef    _FBC_IN_IPRAW
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_IMGO_D_FBC,FB_NUM,1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC,FB_NUM,1,ISP_DRV_USER_ISPF);
#endif
                }

                ISP_FUNC_INF("[PIP][TopCtrl:Disable]P1D_en(0x%x/0x%x),CQ-En(0x%x),FBC(0x%x/0x%x)", \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1_D),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_EN_P1_DMA_D),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_CQ_EN),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_IMGO_D_FBC),\
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC));
            #endif

            #ifdef _rtbc_buf_que_2_0_
            dma_array[_imgo_d_] = 0;
            dma_array[_rrzo_d_] = 0;
            //ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ_D,(unsigned int)dma_array,0};
            ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ_D,0,0,&dma_array[0]};
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
            #endif
            //reset p1_d cmdQ descriptor
            for(int j=0;j<ISP_DRV_P1_CQ_DUPLICATION_NUM;j++){
                for(int i=0; i<CAM_DUMMY_; i++){
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->cqDelModule(this->CQ_D, this->burstQueIdx, j, (CAM_MODULE_ENUM)i);
                }
            }
        }


        if ( (ISP_PASS1_CAMSV == this->path) || (ISP_PASS1_CAMSV == this->pathD)) {
            rawCentralCtrl.clearPath(ISP_PASS1_CAMSV);

            if( CAMSV_CAMSV_MODULE_TG_EN & this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw){
                ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG_VF_CON, VFDATA_EN,0,ISP_DRV_USER_ISPF);
                irq_vs_int.Status = CAMSV_CAMSV_INT_STATUS_VS1_ST;
            }
            if(CAMSV_CAMSV_MODULE_IMGO_EN & this->isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw) {
                ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV_IMGO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
            }
            #ifdef _rtbc_buf_que_2_0_
            dma_array[_camsv_imgo_] = 0;
            //ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ,(unsigned int)dma_array,0};
            ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ,0,0,&dma_array[0]};
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
            #endif
        }

        if ((ISP_PASS1_CAMSV_D == this->path) || (ISP_PASS1_CAMSV_D == this->pathD)) {
            rawCentralCtrl.clearPath(ISP_PASS1_CAMSV_D);

            if( CAMSV_CAMSV2_MODULE_TG_EN & this->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw){
                ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_TG2_VF_CON, VFDATA_EN,0,ISP_DRV_USER_ISPF);
            }
            if(CAMSV_CAMSV2_MODULE_IMGO_EN & this->isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw) {
                ISP_WRITE_BITS(m_pPhyIspDrv,CAMSV_CAMSV2_IMGO_FBC,FBC_EN,0,ISP_DRV_USER_ISPF);
            }

            #ifdef _rtbc_buf_que_2_0_
            dma_array[_camsv2_imgo_] = 0;
            //ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ,(unsigned int)dma_array,0};
            ISP_BUFFER_CTRL_STRUCT buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN,(_isp_dma_enum_)this->CQ,0,0,&dma_array[0]};
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
            #endif
        }
        //Trigger Reset Bit to replace waiting for P1-Done


        if (rawCentralCtrl.mEnablePath == 0) {
            MUINT32 RawFBC, RawDFBC;
            ISP_FUNC_INF("[PIP][TopCtrl::Disable] Sw Reset");
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_SW_CTL, HW_RST, 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_SW_CTL, HW_RST, 0,ISP_DRV_USER_ISPF);
            RawFBC = (ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_FBC));
            RawDFBC = (ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_RRZO_D_FBC));
            ISP_FUNC_INF("[PIP]FBC=0x%x/0x%x", RawFBC, RawDFBC);

            if ( MFALSE == this->m_pPhyIspDrv->ISPWakeLockCtrl(MFALSE)) {
                 ISP_FUNC_ERR("ISPWakeLockCtrl disable fail!!");
                 return -1;
            }
        }

        MUINT32 _clrPort = 0;
        MUINT32 _dma[2] = {0};
        if(ISP_PASS1 == this->path){
            _clrPort = 2;
            _dma[0] = _imgo_;
            _dma[1] = _rrzo_;
        }
        else if(ISP_PASS1_D == this->pathD){
            _clrPort = 2;
            _dma[0] = _imgo_d_;
            _dma[1] = _rrzo_d_;
        }
        else if(ISP_PASS1_CAMSV == this->path){
            _clrPort = 1;
            _dma[0] = _camsv_imgo_;
        }
        else if(ISP_PASS1_CAMSV_D == this->pathD){
            _clrPort = 1;
            _dma[0] = _camsv2_imgo_;
        }
        else
            ISP_FUNC_ERR("path error 0x%x_0x%x\n",this->path,this->pathD);

        while(_clrPort--){
            ISP_FUNC_DBG("[TopCtrl::Disable][before]rt_dma(%d),size:hw(%d)",_dma[_clrPort], ISP_BUF_CTRL::m_hwbufL[_dma[_clrPort]].bufInfoList.size());
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
            buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
            buf_ctrl.data_ptr = &dummy;
            buf_ctrl.pExtend = 0;
            ISP_FUNC_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            ISP_FUNC_DBG("[TopCtrl::Disable][after]rt_dma(%d),size:hw(%d) ",_dma[_clrPort], ISP_BUF_CTRL::m_hwbufL[_dma[_clrPort]].bufInfoList.size());
        }

    } else {  // for p2
        ISP_DRV_P2_CQ_ENUM p2Cq;
        if (ISP_PASS2 == this->path) {
            p2Cq = ISP_DRV_P2_CQ1;
            if(this->drvScenario != NSImageio::NSIspio::eDrvScenario_IP){
                errSta = g_pMdpMgr->stopMdp(gpMdpMgrCfgData[this->burstQueIdx][ISP_DRV_P2_CQ1][this->dupCqIdx]);
                if(MDPMGR_NO_ERROR != errSta){
                    ISP_FUNC_ERR("pass2 CQ1 g_pMdpMgr->stopMdp fail(%d)",errSta);
                    return -1;
                }
            } else {
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000000,ISP_DRV_USER_ISPF);
            }
        }
        else if (ISP_PASS2B == this->path) {
            p2Cq = ISP_DRV_P2_CQ2;
            if(this->drvScenario != NSImageio::NSIspio::eDrvScenario_IP){
                errSta = g_pMdpMgr->stopMdp(gpMdpMgrCfgData[this->burstQueIdx][ISP_DRV_P2_CQ2][this->dupCqIdx]);
                if(MDPMGR_NO_ERROR != errSta){
                    ISP_FUNC_ERR("pass2 CQ2 g_pMdpMgr->stopMdp fail(%d)",errSta);
                    return -1;
                }
            } else {
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000000,ISP_DRV_USER_ISPF);
            }
        }
        else if (ISP_PASS2C == this->path) {
            p2Cq = ISP_DRV_P2_CQ3;
            if(this->drvScenario != NSImageio::NSIspio::eDrvScenario_IP){
                errSta = g_pMdpMgr->stopMdp(gpMdpMgrCfgData[this->burstQueIdx][ISP_DRV_P2_CQ3][this->dupCqIdx]);
                if(MDPMGR_NO_ERROR != errSta){
                    ISP_FUNC_ERR("pass2 CQ3 g_pMdpMgr->stopMdp fail(%d)",errSta);
                    return -1;
                }
            } else {
                ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x00000000,ISP_DRV_USER_ISPF);
            }
        } else {
            ISP_FUNC_ERR("[Error]unknown path(0x%x)",this->path);
            return -1;
        }
        //clear buffer queue
        //clear all buffer list
        ISP_FUNC_DBG("clr buf list,p2Cq(%d),dupCqIdx(%d)",p2Cq,this->dupCqIdx);
        for ( int rt_dma =_imgi_; rt_dma < _rt_dma_max_ ;rt_dma++ ) {
            ISP_BUF_CTRL::m_p2HwBufL[p2Cq][this->dupCqIdx][rt_dma].bufInfoList.clear();
        }
    }

    //
    if ( MTRUE == function_DbgLogEnable_INFO) {
        //
        if ( ISP_PASS1 == this->path ) {
            usleep(1000);
            //pass1 DMA base_addr
            ISP_FUNC_INF("P1 start(0x%x)", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_START));
        }
        else if ( ISP_PASS2 == this->path ||
                  ISP_PASS2B == this->path ||
                  ISP_PASS2C == this->path ) {
            //usleep(1000);
            //pass2 DMA base_addr
            ISP_FUNC_INF("P2 start(0x%x)", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_CTL_START));
        }
    }

    ISP_FUNC_DBG("ISP_TOP_CTRL::_disable:X ");

    return 0;
}

int ISP_TOP_CTRL::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("ISP_TOP_CTRL::checkBusy:E ");
    ISP_FUNC_DBG("ISP_TOP_CTRL::checkBusy:X ");
    return int_done_status?0:1;
}

int ISP_TOP_CTRL::_write2CQ(void)
{
    ISP_FUNC_DBG("TOP_CTRL+,path(0x%x/0x%x),hwScenario(%d)",this->path,this->pathD,this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO);
    //TOP
    if (ISP_PASS1 == this->path) { //will update raw
        for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_EN_P1);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_FMT_P1);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_P1);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_P1_MAGIC_NUM);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_P1_RRZ_CROP_IN);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_P1_RRZ_OUT_W);
            if (isTwinMode){
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_EN_P1_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_FMT_P1_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_P1_D);
                //this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_P1_MAGIC_NUM_D);   No need
            }
        }
    }

    if (ISP_PASS1_D == this->pathD) { //will update rawD

        //this logic is error, should not happen
        if (isTwinMode){
            //This Case only TRUE under N3D
            if (ISP_PASS1 != this->path) {
                ISP_FUNC_ERR("ERR: This condition can be true only under N3D case.");
                return MFALSE;
            }

            //TODO: check for YUV sensor case
            //using CQ0 to config twin mode registers to avoid race condition
            MUINT32 raw_CQ = rawCentralCtrl.getRawPathCQ();
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                if (this->CQ!= raw_CQ) {
                   ISP_FUNC_ERR("[ISP_TOP_CTRL]ERR: this->CQ!= raw_CQ:shall not happen");
                }

                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)raw_CQ    ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_FMT_P1_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)raw_CQ    ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_P1_D);
                //Magic Number shall be updated by relative CQ
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_P1_MAGIC_NUM_D);
            }
        }
        else{
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                //PIP Mode
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_EN_P1_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_FMT_P1_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_P1_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_P1_RRZ_CROP_IN_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_P1_RRZ_OUT_W_D);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_P1_MAGIC_NUM_D);
            }
        }
    }

    if (ISP_PASS2 == this->path || ISP_PASS2B == this->path || ISP_PASS2C == this->path) {
        //tpipe config only in pass2
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_EN_P2);
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_FMT_P2);
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_P2);
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_ISP_GGM_TOP);  // ggm top control by reg. 0x5480
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_ISP_CRSP); //need to disable for scenario without nr3d
        //
        if(this->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO==ISP_HW_SCENARIO_IP){
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_EN_P1);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_FMT_P1);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_P1);
        }
    }

    // for both P1 and P2
    //if pathD == pass1_D, mean this cfg is 2nd cfg, following descriptor is already on CQ0. beside in 2ndcfg, this->CQ is untrustful.
    if (ISP_PASS1_D != this->pathD) {
        if(ISP_PASS2 == this->path || ISP_PASS2B == this->path || ISP_PASS2C == this->path){
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_SCEN);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_GLOBAL_SET);
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_GLOBAL_CLR);
        }
        else{
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_SCEN);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_GLOBAL_SET);
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(CAM_MODULE_ENUM)CAM_TOP_CTL_SEL_GLOBAL_CLR);
            }
        }
    }

    //
    ISP_FUNC_DBG("-,");

    return 0;
}

MBOOL ISP_TOP_CTRL::setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src)
{
    ISP_FUNC_DBG("ISP_TOP_CTRL::setCQTriggerMode:E ");
    //
    this->m_pIspDrvShell->setCQTriggerMode(cq, mode,trig_src);
    //
    ISP_FUNC_DBG("ISP_TOP_CTRL::setCQTriggerMode:X ");

    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_RAW_PIPE
  /////////////////////////////////////////////////////////////////////////////*/

int ISP_RAW_PIPE::_config( void )
{

    ISP_FUNC_DBG("+(ISP_RAW_PIPE),p(0x%x/0x%x),cq(%d/%d),drvScenario(%d),sub_mode(%d/%d),en_p2(0x%08x)",\
        this->path,this->pathD,this->CQ,this->CQ_D,this->drvScenario,this->sub_mode,this->sub_mode_D,this->en_p2);

    if ( ISP_PASS2 == this->path || \
         ISP_PASS2B == this->path || \
         ISP_PASS2C == this->path
    ) {
        MUINT32 i, tuningUpdateBit=eIspTuningMgrFunc_Null;
        MUINT32 tuningEnP2=0x00, tuningEnP2Dma=0x00, tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00;
        MUINT32 tuningEnP1=0x00, tuningEnP1Dma=0x00, tuningEnP1Tag=0x00, tuningEnP1DmaTag=0x00;
        MUINT32 finalEnP2 = 0x00;
        MUINT32 finalEnP1 = 0x00;
        IspTuningPackage tuningPackage;
        IspImageioPackage imageioPackage;
        //
        //

        if (this->isV3)  //QQ??
        {
            //getEnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->p2SubMode, tuningEn1Tag, tuningEn2Tag, tuningDmaTag);
            imageioPackage.isApplyTuning = this->isApplyTuning;
            imageioPackage.cq = this->CQ;
            imageioPackage.subMode = this->sub_mode;
			imageioPackage.enFgMode = 0;

            if(imageioPackage.isApplyTuning==MTRUE){
                tuningUpdateBit = 0xffffffff;
                tuningPackage.pTuningBuf = (MUINT32*)this->pTuningIspReg;
                if (tuningPackage.pTuningBuf == NULL){
                    ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
                    return 0;
                }
                ISP_FUNC_DBG("tuningUpdateBit(0x%08x),pTuningBuf(0x%08x)",tuningUpdateBit,tuningPackage.pTuningBuf);
            }

            getP2EnTuningTag(MTRUE, (ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag

            if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario){
                getP1EnTuningTag(MFALSE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP1Tag, tuningEnP1DmaTag); // get P1 tuning En & Dma Tag
                finalEnP1 = ((~tuningEnP1Tag)&this->en_p1)|(tuningEnP1Tag&tuningEnP1);
            }
            finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
            imageioPackage.finalEnP2 = finalEnP2;

            ISP_FUNC_DBG("(ISP_RAW_PIPE),isApplyTuning(0x%x),finalEnP2(0x%x),tuningUpdateBit(0x%x)",this->isApplyTuning,finalEnP2,tuningUpdateBit);
        }
        else
        {
        getP2EnTuningTag(MTRUE, (ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
        imageioPackage.isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
        imageioPackage.cq = this->CQ;
        imageioPackage.subMode = this->sub_mode;
		imageioPackage.enFgMode = 0;

        if(imageioPackage.isApplyTuning==MTRUE){
            if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE){
                ISP_FUNC_ERR("[Error]deTuningQue fail");
                return -1;
            }
                       tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2,this->magicNum, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario);
            tuningEnP2 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
            tuningEnP2Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);
            tuningPackage.pTuningBuf = m_pPhyIspDrv->getTuningBuf(eTuningQueAccessPath_imageio_P2, this->CQ);
            if (tuningPackage.pTuningBuf == NULL){
                ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
                return 0;
            }
            //
            if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario){
                getP1EnTuningTag(MFALSE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP1Tag, tuningEnP1DmaTag); // get P1 tuning En & Dma Tag
                tuningEnP1 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p1, this->CQ, this->magicNum);
                tuningEnP1Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p1_dma, this->CQ, this->magicNum);
            }
            ISP_FUNC_DBG("tuningUpdateBit(0x%08x),tuningEnP2(0x%08x),tuningEnP2Dma(0x%08x),pTuningBuf(0x%08x)",tuningUpdateBit,tuningEnP2,tuningEnP2Dma,tuningPackage.pTuningBuf);
        }
        if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario){
            finalEnP1 = ((~tuningEnP1Tag)&this->en_p1)|(tuningEnP1Tag&tuningEnP1);
            ISP_FUNC_DBG("(ISP_RAW_PIPE),en_p1(0x%x),tuningEnP1Tag(0x%x),tuningEnP1(0x%x),finalEnP1(0x%x)",this->en_p1,tuningEnP1Tag,tuningEnP1,finalEnP1);
        }
        finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
        }


        ISP_FUNC_DBG("(ISP_RAW_PIPE),cq(%d),drvScenario(%d),sub_mode(%d),en_p2(0x%08x),finalEnP2(0x%08x)",this->CQ,this->drvScenario,this->sub_mode,this->en_p2,finalEnP2);
        //
        for(i=0;i<CHECK_TUNING_P2_EN_BITS;i++){
            if(     ((finalEnP2 & gIspModuleMappingTableP2En[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2)) &&
                    (gIspModuleMappingTableP2En[i].eModule==eIspModuleRaw) &&
                    (gIspModuleMappingTableP2En[i].default_func!=NULL)) {
                //
                if(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)
                    tuningPackage.eTuningCqFunc1 = gIspModuleMappingTableP2En[i].eTuningCqFunc1;
                else
                    tuningPackage.eTuningCqFunc1 = CAM_DUMMY_;
                //
                if(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2)
                    tuningPackage.eTuningCqFunc2 = gIspModuleMappingTableP2En[i].eTuningCqFunc2;
                else
                    tuningPackage.eTuningCqFunc2 = CAM_DUMMY_;
                //
                if(((tuningPackage.eTuningCqFunc1!=CAM_DUMMY_)||(tuningPackage.eTuningCqFunc2!=CAM_DUMMY_))&&(gIspModuleMappingTableP2En[i].eTuningCqDma!=CAM_DUMMY_))
                    tuningPackage.eTuningCqDma = gIspModuleMappingTableP2En[i].eTuningCqDma;
                else
                    tuningPackage.eTuningCqDma = CAM_DUMMY_;

				tuningPackage.rrzRatio = 0;

                gIspModuleMappingTableP2En[i].default_func(imageioPackage, tuningPackage);
                //
            }
        }

        if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario){ // only for scenario ip
            for(i=0;i<CHECK_TUNING_P1_EN_BITS;i++){
                if((finalEnP1 & gIspModuleMappingTableP1En[i].ctrlByte) &&
                        (gIspModuleMappingTableP1En[i].eModule==eIspModuleRaw) &&
                        (gIspModuleMappingTableP1En[i].default_func!=NULL)) {
                    //
                    if(tuningUpdateBit & gIspModuleMappingTableP1En[i].eTuningFunc1)
                        tuningPackage.eTuningCqFunc1 = gIspModuleMappingTableP1En[i].eTuningCqFunc1;
                    else
                        tuningPackage.eTuningCqFunc1 = CAM_DUMMY_;
                    //
                    if(tuningUpdateBit & gIspModuleMappingTableP1En[i].eTuningFunc2)
                        tuningPackage.eTuningCqFunc2 = gIspModuleMappingTableP1En[i].eTuningCqFunc2;
                    else
                        tuningPackage.eTuningCqFunc2 = CAM_DUMMY_;
                    //
                    if(((tuningPackage.eTuningCqFunc1!=CAM_DUMMY_)||(tuningPackage.eTuningCqFunc2!=CAM_DUMMY_))&&(tuningEnP1Dma&gIspModuleMappingTableP1En[i].dmaCtrlByte))
                        tuningPackage.eTuningCqDma = gIspModuleMappingTableP1En[i].eTuningCqDma;
                    else
                        tuningPackage.eTuningCqDma = CAM_DUMMY_;

                    gIspModuleMappingTableP1En[i].default_func(imageioPackage, tuningPackage);
                    //
                }
            }
        }
    if (!this->isV3)
    {
        if(imageioPackage.isApplyTuning==MTRUE){
            if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE){
                ISP_FUNC_ERR("[Error]enTuningQue fail");
            }
        }
    }
        ISP_WRITE_REG(m_pP2IspDrv,CAM_UNP_OFST, 0x00,ISP_DRV_USER_ISPF);
        //
    }
    else if ( ISP_PASS1   == this->path || ISP_PASS1_D == this->pathD) {
        if (!this->isV3)
        {

        //config rrz
        ISP_PASS1_CTRL  ispPass1Ctrl;
        IspRrzCfg rrz_cfg;
        MUINT32 rrz_ctl = 0;
        MUINT32 mgNum  = this->m_num_0;
        MUINT32 mgNumd = this->m_num_0_d;
        int ispPath[2] = {this->path, this->pathD};
        IspDrv* _targetVirDrv = NULL;
        MUINT32 _targetDupQIdx = 0;
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        MUINT32 bVF_en = MFALSE;

        if(this->path == ISP_PASS1)
            bVF_en = ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN);
        else if(this->pathD == ISP_PASS1_D)
            bVF_en = ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN);
        if(bVF_en)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
#else
            _targetDupQIdx = this->dupCqIdx;
#endif
        if(this->burstQueIdx > 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("pass1 support no burstQNum");
        }
        if( ISP_PASS1   == this->path)
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
        if( ISP_PASS1_D   == this->pathD)
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR("_targetVirDrv == NULL, _targetDupQIdx = 0x%x\n",_targetDupQIdx);
            return -1;
        }
        //ISP_FUNC_DBG("ISP_RAW_PIPE+,Pass1 p(%d/%d)", this->path,this->pathD);

        ispDrvSwitch2Virtual(this->CQ, this->CQ_D,this->burstQueIdx, this->dupCqIdx);

#if 1        //for runtime switch
        //when you want to enable PDAF func, please check-in with Middleware at the same time
        ISP_FUNC_DBG("img_sel(0x%x)",this->img_sel);
        if( ISP_PASS1   == this->path) {
            ISP_WRITE_BITS(_targetVirDrv,CAM_CTL_SEL_P1, IMG_SEL, this->img_sel,ISP_DRV_USER_ISPF);
        }
        if( ISP_PASS1_D   == this->pathD) {
            ISP_WRITE_BITS(_targetVirDrv,CAM_CTL_SEL_P1_D, IMG_SEL_D, this->img_sel,ISP_DRV_USER_ISPF);
        }
#endif

        if(this->isTwinMode && (this->en_p1 &CAM_CTL_EN_P1_HBIN_EN)){
            ISP_FUNC_DBG("isTwinMode,path(0x%x), tg w/h=0x%x,0x%x",path, this->src_img_w,this->src_img_h);

            ISP_WRITE_REG(_targetVirDrv,CAM_HBN_SIZE, (this->src_img_h<<16)||this->src_img_w,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_HBN_MODE, (0x1<<4),ISP_DRV_USER_ISPF);
        }
        //twin mode
        if(CAM_CTL_EN_P1_RRZ_EN & this->en_p1 && 1 == this->isTwinMode) {

            ISP_FUNC_DBG("isTwinMode,path(0x%x), m_pIspDrvShell=0x%x, m_pTwinDrv=0x%x",path, m_pIspDrvShell, this->m_pIspDrvShell->m_pTwinDrv);
            //when Twin Mode is enabled, Raw/RawD RRZ are both dedicated for TG1,  => Use TG1 configuration.
            //EX. The YUV of TG2 sensor data is TG-In -> IMGO out. This case will never use RRZ-D.
            //For Dual Raw, (Raw+JPEG) TwinMode is FALSE.
            MUINT32 rrz_h_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(this->rrz_in_size.w,this->rrz_in_roi.w,this->rrz_in_roi.x,this->rrz_in_roi.floatX,this->rrz_out_size.w);
            MUINT32 rrz_v_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(this->rrz_in_size.h,this->rrz_in_roi.h,this->rrz_in_roi.y,this->rrz_in_roi.floatY,this->rrz_out_size.h);;

            rrz_ctl = (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(this->rrz_in_roi.w,this->rrz_out_size.w)<<16) | \
                      (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(this->rrz_in_roi.h,this->rrz_out_size.h)<<21) | \
                      0x03; //enable
            this->m_pIspDrvShell->cam_isp_rrz_cfg(_targetVirDrv,this->rrz_in_size,this->rrz_in_roi,this->rrz_out_size,rrz_ctl,rrz_h_step,rrz_v_step, 0);

            MUINT32 grabPxl, grabLin;
            grabPxl= ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_TG_SEN_GRAB_PXL);
            grabLin= ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,CAM_TG_SEN_GRAB_LIN);
            //ISP_FUNC_DBG("ISP_RAW_PIPE::_config: twin mode. phyIspDrv=0x%x, grabPxl=0x%x, grabLin=0x%x", m_pPhyIspDrv, grabPxl, grabLin);
            ISP_WRITE_REG(_targetVirDrv,CAM_TG_SEN_GRAB_PXL, grabPxl,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_TG_SEN_GRAB_LIN, grabLin,ISP_DRV_USER_ISPF);

            // 0x15004E08  CAM_DMX_VSIZE = TG out h
            ISP_WRITE_REG(_targetVirDrv, CAM_DMX_VSIZE, this->src_img_h, ISP_DRV_USER_ISPF);    //0x15004E08

            //TODO check with TC&Yunyu why need below cp?
            ISP_WRITE_REG(_targetVirDrv,CAM_LSC_D_CTL1,ISP_READ_REG_NOPROTECT(_targetVirDrv,CAM_LSC_CTL1),ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSC_D_CTL3,ISP_READ_REG_NOPROTECT(_targetVirDrv,CAM_LSC_CTL3),ISP_DRV_USER_ISPF);

            //ispPass1Ctrl.isTwinMode = 1;
            pPass1Ctrl->isTwinMode = 1;

            rrz_cfg.isTwinMode  = this->isTwinMode;
            rrz_cfg.pass1_path  = this->path;
            rrz_cfg.rrz_in_size.w       = this->rrz_in_size.w;
            rrz_cfg.rrz_in_size.h       = this->rrz_in_size.h;
            rrz_cfg.rrz_in_size.stride  = this->rrz_in_size.stride;
            rrz_cfg.rrz_in_roi.x      = this->rrz_in_roi.x;
            rrz_cfg.rrz_in_roi.y      = this->rrz_in_roi.y;
            rrz_cfg.rrz_in_roi.floatX = this->rrz_in_roi.floatX;
            rrz_cfg.rrz_in_roi.floatY = this->rrz_in_roi.floatY;
            rrz_cfg.rrz_in_roi.w      = this->rrz_in_roi.w;
            rrz_cfg.rrz_in_roi.h      = this->rrz_in_roi.h;
            rrz_cfg.rrz_out_size.w       = this->rrz_out_size.w;
            rrz_cfg.rrz_out_size.h       = this->rrz_out_size.h;
            rrz_cfg.rrz_out_size.stride  = this->rrz_out_size.stride;
            rrz_cfg.lsc_en              = (ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_EN_P1, ISP_DRV_USER_ISPF) & CAM_CTL_EN_P1_LSC_EN)?1:0; //ISP_READ_REG
            rrz_cfg.twin_lsc_crop_offx  = this->twin_lsc_crop_offx;
            rrz_cfg.twin_lsc_crop_offy  = this->twin_lsc_crop_offy;


            /*
                    0x15004534          LSC_CTL2          16         12         LSC_SDBLK_XNUM
                    0x15004534          LSC_CTL2          11         0         LSC_SDBLK_WIDTH
                */
            rrz_cfg.sd_lwidth = this->m_twin_lblkW;
            rrz_cfg.sd_xnum = this->m_twin_sdnumX;
            rrz_cfg.m_num_0 = this->m_num_0;

            ISP_FUNC_INF("ISP_RAW_PIPE::_config(setP1RrzCfg:0x%x_0x%x):Twin lsc_en=%d E:path(%d),rrz_in_w/h(%d/%d),rrz_crop_x/y/fx/fy/w/h(%d/%d/%d/%d/%d/%d),rrz_out_w/h(%d/%d), sd_lw/xn(%d/%d),m_num(0x%x)", \
                this->path,this->pathD,\
                rrz_cfg.lsc_en,\
                rrz_cfg.pass1_path, \
                rrz_cfg.rrz_in_size.w, \
                rrz_cfg.rrz_in_size.h, \
                rrz_cfg.rrz_in_roi.x, \
                rrz_cfg.rrz_in_roi.y, \
                rrz_cfg.rrz_in_roi.floatX, \
                rrz_cfg.rrz_in_roi.floatY, \
                rrz_cfg.rrz_in_roi.w, \
                rrz_cfg.rrz_in_roi.h, \
                rrz_cfg.rrz_out_size.w, \
                rrz_cfg.rrz_out_size.h, \
                rrz_cfg.sd_lwidth, \
                rrz_cfg.sd_xnum, \
                rrz_cfg.m_num_0);
            if ( 0 != pPass1Ctrl->keepP1RrzCfg(rrz_cfg) ) {
                  ISP_FUNC_ERR("ERROR keepP1RrzCfg");
            }

            if ( 0 != pPass1Ctrl->setP1RrzCfg(rrz_cfg) ) {
                ISP_FUNC_ERR("ERROR setP1RrzCfg");
            }
        }
        else { //single mode

            //ispPass1Ctrl.isTwinMode = 0;
            pPass1Ctrl->isTwinMode = 0;
             rrz_cfg.isTwinMode  = this->isTwinMode;

            if( ISP_PASS1   == this->path){
                rrz_cfg.pass1_path  = this->path;
                rrz_cfg.lsc_en              = (ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_EN_P1, ISP_DRV_USER_ISPF) & CAM_CTL_EN_P1_LSC_EN)?1:0; //ISP_READ_REG
                rrz_cfg.rrz_in_size.w       = this->rrz_in_size.w;
                rrz_cfg.rrz_in_size.h       = this->rrz_in_size.h;
                rrz_cfg.rrz_in_size.stride  = this->rrz_in_size.stride;
                rrz_cfg.rrz_in_roi.x      = this->rrz_in_roi.x;
                rrz_cfg.rrz_in_roi.y      = this->rrz_in_roi.y;
                rrz_cfg.rrz_in_roi.floatX = this->rrz_in_roi.floatX;
                rrz_cfg.rrz_in_roi.floatY = this->rrz_in_roi.floatY;
                rrz_cfg.rrz_in_roi.w      = this->rrz_in_roi.w;
                rrz_cfg.rrz_in_roi.h      = this->rrz_in_roi.h;
                rrz_cfg.rrz_out_size.w       = this->rrz_out_size.w;
                rrz_cfg.rrz_out_size.h       = this->rrz_out_size.h;
                rrz_cfg.rrz_out_size.stride  = this->rrz_out_size.stride;
                rrz_cfg.twin_lsc_crop_offx  = this->twin_lsc_crop_offx;
                rrz_cfg.twin_lsc_crop_offy  = this->twin_lsc_crop_offy;
            }
            else if(ISP_PASS1_D == this->pathD){
                rrz_cfg.pass1_path  = this->pathD;
                rrz_cfg.lsc_en              = (ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_EN_P1_D, ISP_DRV_USER_ISPF) & CAM_CTL_EN_P1_D_LSC_D_EN)?1:0; //ISP_READ_REG
                rrz_cfg.rrz_in_size.w       = this->rrz_d_in_size.w;
                rrz_cfg.rrz_in_size.h       = this->rrz_d_in_size.h;
                rrz_cfg.rrz_in_size.stride  = this->rrz_d_in_size.stride;
                rrz_cfg.rrz_in_roi.x      = this->rrz_d_in_roi.x;
                rrz_cfg.rrz_in_roi.y      = this->rrz_d_in_roi.y;
                rrz_cfg.rrz_in_roi.floatX = this->rrz_d_in_roi.floatX;
                rrz_cfg.rrz_in_roi.floatY = this->rrz_d_in_roi.floatY;
                rrz_cfg.rrz_in_roi.w      = this->rrz_d_in_roi.w;
                rrz_cfg.rrz_in_roi.h      = this->rrz_d_in_roi.h;
                rrz_cfg.rrz_out_size.w       = this->rrz_d_out_size.w;
                rrz_cfg.rrz_out_size.h       = this->rrz_d_out_size.h;
                rrz_cfg.rrz_out_size.stride  = this->rrz_d_out_size.stride;
                rrz_cfg.twin_lsc_crop_offx  = this->twin_lsc_crop_offx;
                rrz_cfg.twin_lsc_crop_offy  = this->twin_lsc_crop_offy;
            }
            ISP_FUNC_INF("ISP_RAW_PIPE::_config(setP1RrzCfg:0x%x_0x%x):NonTwin lsc_en=%d E:path(%d),rrz_in_w/h(%d/%d),rrz_crop_x/y/fx/fy/w/h(%d/%d/%d/%d/%d/%d),rrz_out_w/h(%d/%d), sd_lw/xn(%d/%d),m_num(0x%x)", \
                this->path,this->pathD,\
                rrz_cfg.lsc_en,\
                rrz_cfg.pass1_path, \
                rrz_cfg.rrz_in_size.w, \
                rrz_cfg.rrz_in_size.h, \
                rrz_cfg.rrz_in_roi.x, \
                rrz_cfg.rrz_in_roi.y, \
                rrz_cfg.rrz_in_roi.floatX, \
                rrz_cfg.rrz_in_roi.floatY, \
                rrz_cfg.rrz_in_roi.w, \
                rrz_cfg.rrz_in_roi.h, \
                rrz_cfg.rrz_out_size.w, \
                rrz_cfg.rrz_out_size.h, \
                rrz_cfg.sd_lwidth, \
                rrz_cfg.sd_xnum, \
                rrz_cfg.m_num_0);
            if ( 0 != pPass1Ctrl->keepP1RrzCfg(rrz_cfg) ) {
                  ISP_FUNC_ERR("ERROR keepP1RrzCfg");
            }

            if( ( ISP_PASS1   == this->path) && (CAM_CTL_EN_P1_RRZ_EN & this->en_p1)){
                MUINT32 rrz_h_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(this->rrz_in_size.w,this->rrz_in_roi.w,this->rrz_in_roi.x,this->rrz_in_roi.floatX,this->rrz_out_size.w);
                MUINT32 rrz_v_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(this->rrz_in_size.h,this->rrz_in_roi.h,this->rrz_in_roi.y,this->rrz_in_roi.floatY,this->rrz_out_size.h);;
                rrz_ctl = (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(this->rrz_in_roi.w,this->rrz_out_size.w)<<16) | \
                          (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(this->rrz_in_roi.h,this->rrz_out_size.h)<<21) | \
                          0x03; //enable
                this->m_pIspDrvShell->cam_isp_rrz_cfg(_targetVirDrv,this->rrz_in_size,this->rrz_in_roi,this->rrz_out_size,rrz_ctl,rrz_h_step,rrz_v_step,0);
            }
            else if( (ISP_PASS1_D == this->pathD) && (CAM_CTL_EN_P1_D_RRZ_D_EN & this->en_p1_d)){
                MUINT32 rrz_h_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(this->rrz_d_in_size.w,this->rrz_d_in_roi.w,this->rrz_d_in_roi.x,this->rrz_d_in_roi.floatX,this->rrz_d_out_size.w);
                MUINT32 rrz_v_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(this->rrz_d_in_size.h,this->rrz_d_in_roi.h,this->rrz_d_in_roi.y,this->rrz_d_in_roi.floatY,this->rrz_d_out_size.h);;
                rrz_ctl = (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(this->rrz_d_in_roi.w,this->rrz_d_out_size.w)<<16) | \
                          (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(this->rrz_d_in_roi.h,this->rrz_d_out_size.h)<<21) | \
                          0x03; //enable
                this->m_pIspDrvShell->cam_isp_rrz_cfg(_targetVirDrv,this->rrz_d_in_size,this->rrz_d_in_roi,this->rrz_d_out_size,rrz_ctl,rrz_h_step,rrz_v_step,1);
           }
        }

#if 0    //remove, rrzo won't be always enabled, magicnum need to be set independently.
        if ( ISP_PASS1   == this->path) {
            ISP_FUNC_DBG("ISP_RAW_PIPE PASS1: Bef setP1MagicNum %d", this->m_num_0);
            if ( 0 != pPass1Ctrl->setP1MagicNum(this->path, mgNum, 1)) {
                ISP_FUNC_DBG("setP1MagicNum Raw fail");
            }
        }

        if ( ISP_PASS1_D   == this->pathD) {
            ISP_FUNC_DBG("ISP_RAW_PIPE PASS1D: Bef setP1MagicNum %d", this->m_num_0_d);

            if ( 0 != pPass1Ctrl->setP1MagicNum(this->pathD, mgNumd, 1)) {
                ISP_FUNC_DBG("setP1MagicNum Raw-D fail");
            }
        }
#endif
     }
    } else {
        ISP_FUNC_ERR("[ERROR]Not support this path(%d)",this->path);
    }
    ISP_FUNC_DBG("-,");
    return 0;

}


int ISP_RAW_PIPE::_write2CQ(void)
{

    ISP_FUNC_DBG("+(ISP_RAW_PIPE),cq(%d/%d),drvScenario(%d),sub_mode(%d),en_p2(0x%08x)",this->CQ,this->CQ_D,this->drvScenario,this->sub_mode,this->en_p2);

    if ( ISP_PASS2 == this->path || \
         ISP_PASS2B == this->path || \
         ISP_PASS2C == this->path
    ) {
        MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
        MUINT32 finalEnP2 = 0x00, tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00, tuningEnP2=0x00, tuningEnP2Dma=0x00;
        MUINT32 finalEnP1 = 0x00, tuningEnP1Tag=0x00, tuningEnP1DmaTag=0x00, tuningEnP1=0x00, tuningEnP1Dma=0x00;
        MUINT32 i;
        MBOOL isApplyTuning;

        //

        if(this->isV3)
        {
            ISP_FUNC_INF("ISP_RGB_PIPE::_write2CQ");
            getP1EnTuningTag(MTRUE, (ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP1Tag, tuningEnP1DmaTag);
            getP2EnTuningTag(MTRUE, (ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
            isApplyTuning = this->isApplyTuning;
            if(isApplyTuning==MTRUE){
                tuningUpdateBit = 0xffffffff;
                //ISP_FUNC_DBG("tuningUpdateBit(0x%08x)",tuningUpdateBit);
            }
            finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);


            if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario)
                finalEnP1 = ((~tuningEnP1Tag)&this->en_p2)|(tuningEnP1Tag&tuningEnP1);


            ISP_FUNC_DBG("finalEnP1(0x%08x),finalEnP2(0x%08x)",finalEnP1,finalEnP2);
        }
        else
        {
        getP1EnTuningTag(MTRUE, (ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP1Tag, tuningEnP1DmaTag);
        getP2EnTuningTag(MTRUE, (ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
        isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
        if(isApplyTuning==MTRUE){
            if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE){
                ISP_FUNC_ERR("[Error]deTuningQue fail");
                return -1;
            }
                    tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2,this->magicNum, (ISP_DRV_CQ_ENUM)this->CQ,this->drvScenario);
            tuningEnP2 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
            tuningEnP2Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);
            //
            if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario){
                tuningEnP1 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p1, this->CQ, this->magicNum);
                tuningEnP1Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p1_dma, this->CQ, this->magicNum);
                ISP_FUNC_DBG("tuningEnP1(0x%08x),tuningEnP1Dma(0x%08x)",tuningEnP1,tuningEnP1Dma);
            }

                if (! this->isV3)
                {
                    if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE)
                ISP_FUNC_ERR("[Error]enTuningQue fail");
            }
            ISP_FUNC_DBG("tuningUpdateBit(0x%08x),tuningEnP2(0x%08x),tuningEnP2Dma(0x%08x)",tuningUpdateBit,tuningEnP2,tuningEnP2Dma);
        }
        if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario){
            finalEnP1 = ((~tuningEnP1Tag)&this->en_p2)|(tuningEnP1Tag&tuningEnP1);
            ISP_FUNC_DBG("finalEnP1(0x%08x),tuningEnP1Tag(0x%08x)",finalEnP1,tuningEnP1Tag);
        }
        finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
        ISP_FUNC_DBG("finalEnP2(0x%08x),tuningEnP2Tag(0x%08x)",finalEnP2,tuningEnP2Tag);

        }

        //
        for(i=0;i<CHECK_TUNING_P2_EN_BITS;i++){
            if(((finalEnP2 & gIspModuleMappingTableP2En[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2))
                    &&(gIspModuleMappingTableP2En[i].eModule==eIspModuleRaw)){  // check bit en
                if(gIspModuleMappingTableP2En[i].eTuningCqFunc1!=CAM_DUMMY_)
                    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqFunc1);
                if(gIspModuleMappingTableP2En[i].eTuningCqFunc2!=CAM_DUMMY_)
                    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqFunc2);
                if(gIspModuleMappingTableP2En[i].eTuningCqDma!=CAM_DUMMY_)
                    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqDma);
            }
        }
        if(NSImageio::NSIspio::eDrvScenario_IP == this->drvScenario){
            for(i=0;i<CHECK_TUNING_P1_EN_BITS;i++){
                if(((finalEnP1 & gIspModuleMappingTableP1En[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableP1En[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableP1En[i].eTuningFunc2))
                        &&(gIspModuleMappingTableP1En[i].eModule==eIspModuleRaw)){  // check bit en
                    if(gIspModuleMappingTableP1En[i].eTuningCqFunc1!=CAM_DUMMY_)
                        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP1En[i].eTuningCqFunc1);
                    if(gIspModuleMappingTableP1En[i].eTuningCqFunc2!=CAM_DUMMY_)
                        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP1En[i].eTuningCqFunc2);
                    if(gIspModuleMappingTableP1En[i].eTuningCqDma!=CAM_DUMMY_)
                        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP1En[i].eTuningCqDma);
                }
            }
        }

        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, CAM_ISP_UNP);

    }
    else if ( ISP_PASS1 == this->path || ISP_PASS1_D == this->pathD)
    {
        MUINT32 cq;
        if (ISP_PASS1 == this->path)    cq = this->CQ;
        if (ISP_PASS1_D == this->pathD) cq = this->CQ_D;

        IspDrv* pIspDrv_cqx = NULL;
        //twin mode
        if ( 1 == this->isTwinMode ) {



            if (ISP_PASS1_D == this->pathD) {
                ISP_FUNC_DBG("+(ISP_RAW_PIPE), Error: Twin Mode is not support on RawD");
                return -1;
            }
            ISP_FUNC_DBG("+(ISP_RAW_PIPE),isTwinMode = 1");
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
            //set CQ
                pIspDrv_cqx = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,i);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_TOP_CTL_EN_P1);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_LSC);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_RRZ);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_BNR);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_DMX);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_BMX);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_RMX);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_BIN);

                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_OBC_D);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_LSC_D);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_RPG_D);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_RRZ_D);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_BNR_D);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_DMX_D);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_BMX_D);
                pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_RMX_D);
            }
        }
        else {
            ISP_FUNC_DBG("+(ISP_RAW_PIPE),isTwinMode = 0");

            if(this->CQ == ISP_DRV_CQ0){
                for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                    pIspDrv_cqx = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,i);
                    pIspDrv_cqx = (IspDrv*)this->m_pIspDrvShell->m_pPhyIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)this->CQ);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_TOP_CTL_EN_P1);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_LSC);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_RRZ);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_BNR);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_DMX);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_BMX);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_RMX);
                }
            }
            else if(this->CQ_D == ISP_DRV_CQ0_D)
                for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                    pIspDrv_cqx = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,i);
                    pIspDrv_cqx = (IspDrv*)this->m_pIspDrvShell->m_pPhyIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)this->CQ_D);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,CAM_TOP_CTL_EN_P1_D);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,CAM_ISP_LSC_D);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,CAM_ISP_RRZ_D);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,CAM_ISP_BNR_D);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,CAM_ISP_DMX_D);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,CAM_ISP_BMX_D);
                    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,i,CAM_ISP_RMX_D);
                }
            else{
                ISP_FUNC_ERR("CQ is none");
            }

        }
    }
    else {

        ISP_FUNC_ERR("[ERROR]Not support this path(%d)",this->path);
    }


    ISP_FUNC_DBG("-(ISP_RAW_PIPE)");
    return 0;
}

MUINT32 ISP_RAW_PIPE::setEIS(void){
    MUINT32 ret = 1;
    if(this->pP1Tuning){
        EIS_REG_CFG _eis_cfg;
        EIS_INPUT_INFO input;
        if(this->isTwinMode)//hbin en is combine with twin
            input.sHBINOut.w = this->src_img_w/2;
        else
            input.sHBINOut.w = this->src_img_w;
        input.sHBINOut.h = this->src_img_h;
        input.sTGOut.w = this->src_img_w;
        input.sTGOut.h = this->src_img_h;
        input.sRMXOut.w = this->rrz_out_size.w;
        input.sRMXOut.h = this->rrz_out_size.h;
        input.bIsTwin = this->isTwinMode;
        if(this->sub_mode == 1)
            input.bYUVFmt = MTRUE;
        else
            input.bYUVFmt = MFALSE;
        ISP_FUNC_DBG("Update EIS,w/h(%d_%d,%d_%d,%d_%d),isTwin=%d\n,submode=%d",input.sHBINOut.w,input.sHBINOut.h,input.sTGOut.w,input.sTGOut.h,input.sRMXOut.w,input.sRMXOut.h,this->isTwinMode,this->sub_mode);
        ISP_FUNC_DBG("+");

        this->pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_eis_cfg);
        ISP_FUNC_DBG("-");
        if(_eis_cfg.bEIS_Bypass == 0){
            IspDrv* _targetVirDrv = NULL;
            MUINT32 _targetDupQIdx = 0;
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
            if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
                _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
            else
                _targetDupQIdx = this->dupCqIdx;
#else
            _targetDupQIdx = this->dupCqIdx;
#endif
            if(this->burstQueIdx > 0){
                this->burstQueIdx = 0;
                ISP_FUNC_ERR("pass1 support no burstQNum");
            }

            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            if(_targetVirDrv == NULL){
                ISP_FUNC_ERR("_targetVirDrv == NULL, _targetDupQIdx = 0x%x\n",_targetDupQIdx);
                return 0;
            }
            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQIdx,CAM_ISP_EIS);
            if(_eis_cfg.bEIS_EN){
                ISP_DRV_CQ_MODULE_INFO_STRUCT outInfo;
                _targetVirDrv->getIspCQModuleInfo(CAM_ISP_EIS, outInfo);
                memcpy((((MUINT8*)(isp_reg_t*) _targetVirDrv->getRegAddr()) + (outInfo.addr_ofst)), \
                      (MUINT8*)&_eis_cfg._EIS_REG.CTRL_1, \
                      (outInfo.reg_num)*sizeof(UINT32));
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_SEL_P1, EIS_SEL, _eis_cfg.EIS_SEL,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, EIS_EN, 1,ISP_DRV_USER_ISPF);
            }
            else
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, EIS_EN, 0,ISP_DRV_USER_ISPF);


            if(_eis_cfg.bSGG_Bypass == 0){
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_SEL_P1, SGG_SEL, _eis_cfg.SGG_SEL,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_SEL_P1, SGG_SEL_EN, _eis_cfg.bSGG_EN,ISP_DRV_USER_ISPF);
            }
        }
    }
    return ret;
}

MUINT32 ISP_RAW_PIPE::setLCS(void){
    MUINT32 ret = 1;
    if(this->pP1Tuning)
    {
        LCS_REG_CFG     _lcs_cfg;
        LCS_INPUT_INFO     input;
        if(this->path == ISP_PASS1)
        {
            input.sRRZOut.w = this->rrz_out_size.w;
            input.sRRZOut.h = this->rrz_out_size.h;
        }
        else if(this->pathD == ISP_PASS1_D)
        {
            input.sRRZOut.w = this->rrz_d_out_size.w;
            input.sRRZOut.h = this->rrz_d_out_size.h;
        }
        else
        {
            ISP_FUNC_ERR("path err(0x%x_0x%x)\n",this->path,this->pathD);
            return 0;
        }

        if(this->isTwinMode)
        {
            input.bIsHbin = MTRUE;
            input.sHBINOut.w = this->src_img_w/2;
        }
        else
        {
            input.bIsHbin = MFALSE;
            input.sHBINOut.w = this->src_img_w;
        }
        input.sHBINOut.h = this->src_img_h;
        ISP_FUNC_DBG("+");
        this->pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_lcs_cfg);
        ISP_FUNC_DBG("-");
        ISP_FUNC_DBG("update LCS(0x%x_0x%x),in:%d_%d, LCS_REG(%d): 0x%x,0x%x\n",this->path,this->pathD,input.sRRZOut.w,input.sRRZOut.h,\
                                                                                _lcs_cfg.bLCS_Bypass,_lcs_cfg._LCS_REG.LCS_ST,_lcs_cfg._LCS_REG.LCS_AWS);
        if(_lcs_cfg.bLCS_Bypass == 0)
        {
            IspDrv* _targetVirDrv = NULL;
            MUINT32 _targetDupQIdx = 0;
            MINT32 cq,vf_en;
            if(this->path == ISP_PASS1)
            {
                cq = this->CQ;
                vf_en = ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN);
            }
            else
            {
                //pass1_d case,flow won't run to here if path err
                cq = this->CQ_D;
                vf_en = ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN);
            }
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
            if(vf_en == 1)
                _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
            else
                _targetDupQIdx = this->dupCqIdx;
#else
            _targetDupQIdx = this->dupCqIdx;
#endif
            if(this->burstQueIdx > 0)
            {
                this->burstQueIdx = 0;
                ISP_FUNC_ERR("pass1 support no burstQNum");
            }

            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(cq,this->burstQueIdx,_targetDupQIdx);
            if(_targetVirDrv == NULL)
            {
                ISP_FUNC_ERR("_targetVirDrv == NULL, _targetDupQIdx = 0x%x\n",_targetDupQIdx);
                return 0;
            }

            if(this->path == ISP_PASS1)
            {
                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQIdx,CAM_ISP_LCS);
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, LCS_EN, _lcs_cfg.bLCS_EN,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_ST, _lcs_cfg._LCS_REG.LCS_ST , ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_AWS, _lcs_cfg._LCS_REG.LCS_AWS ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_LRZR_1, _lcs_cfg._LCS_REG.LCS_LRZR_1 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_LRZR_2, _lcs_cfg._LCS_REG.LCS_LRZR_2 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_CON, _lcs_cfg._LCS_REG.LCS_CON ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_FLR, _lcs_cfg._LCS_REG.LCS_FLR ,ISP_DRV_USER_ISPF);
            }
            else
            {
                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQIdx,CAM_ISP_LCS_D);
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, LCS_D_EN, _lcs_cfg.bLCS_EN,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_D_ST, _lcs_cfg._LCS_REG.LCS_ST , ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_D_AWS, _lcs_cfg._LCS_REG.LCS_AWS ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_D_LRZR_1, _lcs_cfg._LCS_REG.LCS_LRZR_1 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_D_LRZR_2, _lcs_cfg._LCS_REG.LCS_LRZR_2 ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_D_CON, _lcs_cfg._LCS_REG.LCS_CON ,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_targetVirDrv, CAM_LCS_D_FLR, _lcs_cfg._LCS_REG.LCS_FLR ,ISP_DRV_USER_ISPF);
            }
        }
    }
    return ret;
}

int ISP_RAW_PIPE::checkLscLsciCfg(MUINT32* pLscCfg, MUINT32* pLsciCfg)
{
   MUINT32 lsc_xnum, lsc_ynum, temp;
   CAM_REG_LSC_CTL2 *pLsc_ctl2;
   CAM_REG_LSC_CTL3 *pLsc_ctl3;
   MUINT32 lsci_x, lsci_y, lsci_stride;

   pLsc_ctl2 = (CAM_REG_LSC_CTL2*)(pLscCfg + 1);
   pLsc_ctl3 = (CAM_REG_LSC_CTL3*)(pLscCfg + 2);

   lsci_x = *(pLsciCfg + 2);
   lsci_y = *(pLsciCfg + 3);
   lsci_stride = *(pLsciCfg + 4);
   temp = ((pLsc_ctl2->Bits.LSC_SDBLK_XNUM+1)<<6) - 1 ;
   if (lsci_x != temp) {
       ISP_FUNC_ERR("[Error] LSCI_x != (LSC_x+1)<<6 -1. (%d,%d), XNUM=%d", lsci_x, temp, pLsc_ctl2->Bits.LSC_SDBLK_XNUM);
       return -1;
   }

   if (lsci_y != pLsc_ctl3->Bits.LSC_SDBLK_YNUM) {
       ISP_FUNC_ERR("[Error] LSCI_y != LSC_y. (%d,%d)", lsci_y, pLsc_ctl3->Bits.LSC_SDBLK_YNUM);
       return -1;
   }

   if ((lsci_x+1) != lsci_stride) {
       ISP_FUNC_ERR("[Error] lsci_x+1 != lsci_stride. (%d,%d)", lsci_x, lsci_stride);
       return -1;
   }
   return 0;
}

int ISP_RAW_PIPE::_suspendTuneCfg(MUINT32 mNum)
{
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQueIdx = 0;
    if(mNum & _DUMMY_MAGIC){
        if (NULL == m_pPhyIspDrv) {
            ISP_FUNC_ERR("NULL == m_pPhyIspDrv");
            return -1;
        }

        if(this->burstQueIdx != 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("p1 support no BurstQue");
        }
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 0){
            _targetDupQueIdx = this->dupCqIdx;
        }else{
            _targetDupQueIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        }
#else
        _targetDupQueIdx = this->dupCqIdx;
#endif

        MUINT32 _reg_set = 0x0;
        MUINT32 _reg,_dma_en;
        MUINT32 _cq;
        MUINT32 path = 0;
        if(ISP_PASS1 == this->path){
            _dma_en = (CAM_CTL_EN_P1_DMA_IMGO_EN|CAM_CTL_EN_P1_DMA_RRZO_EN|CAM_CTL_EN_P1_DMA_LSCI_EN|CAM_CTL_EN_P1_DMA_BPCI_EN);
            _cq = this->CQ;
            path = this->path;
        }
        else if(ISP_PASS1_D == this->pathD){
            _dma_en = (CAM_CTL_EN_P1_DMA_D_IMGO_D_EN|CAM_CTL_EN_P1_DMA_D_RRZO_D_EN|CAM_CTL_EN_P1_DMA_D_LSCI_D_EN|CAM_CTL_EN_P1_DMA_D_BPCI_D_EN);
            _cq = this->CQ_D;
            path = this->pathD;
        }
        else{
            ISP_FUNC_ERR("path error:0x%x_0x%x\n",this->path,this->pathD);
            return 0;
        }

        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(_cq,this->burstQueIdx,_targetDupQueIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR(" dupCmdQ == NULL, target dupCqIdx = 0x%x",_targetDupQueIdx);
            return -1;
        }

        if(path == ISP_PASS1){
            _reg = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_EN_P1_DMA);
        }
        else{
            _reg = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_EN_P1_DMA_D);
        }

        if(this->m_Dmao_Reg[_targetDupQueIdx] == 0xffffffff){

            this->m_Dmao_Reg[_targetDupQueIdx] = _reg;
            _reg_set = (this->m_Dmao_Reg[_targetDupQueIdx] & _dma_en);
            ISP_FUNC_DBG("path:0x%x A:(0x%x):0x%x\n",path,_targetDupQueIdx,_reg_set);

            if(ISP_PASS1 == path)
                ISP_WRITE_REG(_targetVirDrv, CAM_CTL_EN_P1_DMA, _reg_set,ISP_DRV_USER_ISPF);
            else if(ISP_PASS1_D == path)
                ISP_WRITE_REG(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, _reg_set,ISP_DRV_USER_ISPF);
        }else
            ISP_FUNC_DBG("path:0x%x B:(0x%x):0x%x\n",path,_targetDupQueIdx,_reg);
        return 1;
    }
    return 0;
}

void ISP_RAW_PIPE::_resumeTuneCfg(IspDrv* _targetVirDrv,MUINT32 _targetDupQueIdx)
{
    MUINT32 _tmp = 0,path =0;
    if(this->m_Dmao_Reg[_targetDupQueIdx] != 0xFFFFFFFF){
        if(this->path == ISP_PASS1){
            _tmp = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_EN_P1_DMA);
            path = this->path;
        }
        else if(ISP_PASS1_D == this->pathD){
            _tmp = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_EN_P1_DMA_D);
            path = this->pathD;
        }
        else{
            ISP_FUNC_ERR("path error:0x%x_0x%x\n",this->path,this->pathD);
            return;
        }
        ISP_FUNC_DBG("path:0x%x bef:0x%x\n",path,this->m_Dmao_Reg[_targetDupQueIdx]);

        //in case of dmao have been enabled through p1hwcfg
        this->m_Dmao_Reg[_targetDupQueIdx] |= _tmp;

        ISP_FUNC_DBG("path:0x%x aft:0x%x\n",path,this->m_Dmao_Reg[_targetDupQueIdx]);
        if(path == ISP_PASS1)
            ISP_WRITE_REG(_targetVirDrv,CAM_CTL_EN_P1_DMA, this->m_Dmao_Reg[_targetDupQueIdx], ISP_DRV_USER_ISPF);
        else
            ISP_WRITE_REG(_targetVirDrv,CAM_CTL_EN_P1_DMA_D, this->m_Dmao_Reg[_targetDupQueIdx], ISP_DRV_USER_ISPF);

        this->m_Dmao_Reg[_targetDupQueIdx] = 0xFFFFFFFF;
    }
}


int ISP_RAW_PIPE::setP1TuneCfg(MUINT32 mNum, MINT32 senDev)
{
    ISP_DRV_CQ_MODULE_INFO_STRUCT outInfo;

    IspP1TuneCfg cfg;
    ISP_DRV_CQ_ENUM cq;
    MUINT32* pTuningBuf;
    MUINT32 i, tuningUpdateBit;
    MUINT32 tuningEnP1=0x00, tuningEnP1Dma=0x00, tuningEnP1Tag=0x00, tuningEnP1DmaTag=0x00;
    MUINT32 tuningEnP1D=0x00, tuningEnP1DmaD=0x00, tuningEnP1DTag=0x00, tuningEnP1DmaDTag=0x00;
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQueIdx = 0;
    MBOOL isApplyTuning;
#if 0 //close resume/suspend mechanism
    //dummy frame , disable all dmao but main img_port, other regs. setting remained previous setting
    if(this->_suspendTuneCfg(mNum)){
        return 0;
    }
#else
    if(mNum & _DUMMY_MAGIC){
        return 0;
    }
#endif
    m_pPhyIspDrv->getCqInfoFromScenario((ESoftwareScenario)senDev, cq);

    isApplyTuning = m_pPhyIspDrv->getP2TuningStatus(cq);
    if(isApplyTuning){

        if(MFALSE == m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum)){
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            return -1;
        }

        cfg.tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P1,this->magicNum, cq, this->drvScenario);
        cfg.tuningEnP1      = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P1, eTuningTopEn_p1, cq, 0);
        cfg.tuningEnP1Dma   = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P1, eTuningTopEn_p1_dma, cq, 0);
        cfg.tuningEnP1D     = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P1, eTuningTopEn_p1_d, cq, 0);
        cfg.tuningEnP1DmaD  = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P1, eTuningTopEn_p1_dma_d, cq, 0);
        cfg.pTuningBuf      = m_pPhyIspDrv->getTuningBuf(eTuningQueAccessPath_imageio_P1, cq);
    }
    else{
        ISP_FUNC_INF("warning:tuning mgr in CQ:0x%x still in idle status,bypass this tuning update\n",cq);
        return 0;
    }

    ISP_FUNC_DBG("P1Dbg_RPG: senDev=%d, mNum=0x%x, UpdateBit=0x%x, EnP1=0x%x, EnP1Dma=0x%x, pBuf=0x%x", senDev, mNum, cfg.tuningUpdateBit, cfg.tuningEnP1,cfg.tuningEnP1Dma, cfg.pTuningBuf);

    if (!cfg.pTuningBuf) {
        ISP_FUNC_ERR(" pTuningBuf=0x%x",  cfg.pTuningBuf);
        return -1;
    }

    if (ISP_PASS1 == this->path){
        if (NULL == m_pPhyIspDrv) {
            ISP_FUNC_ERR("NULL == m_pPhyIspDrv");
            return -1;
        }

        if(this->burstQueIdx != 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("p1 support no BurstQue");
        }
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 0){
            _targetDupQueIdx = this->dupCqIdx;
        }else{
            _targetDupQueIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        }
#else
        _targetDupQueIdx = this->dupCqIdx;
#endif
        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQueIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR(" dupCmdQ == NULL, target dupCqIdx = 0x%x",_targetDupQueIdx);
            return -1;
        }
        isp_reg_t*  cqVirtReg = (isp_reg_t*) _targetVirDrv->getRegAddr();
        if (!cqVirtReg) {
            ISP_FUNC_ERR("cqVirtReg=NULL");
        }

        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Obc) {
            ISP_FUNC_DBG("Obc");

            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_OBC, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_ISP_OBC");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }

            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_OBC);
            if(this->isTwinMode){
                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx, CAM_ISP_OBC_D);
            }
            if (cfg.tuningEnP1 & CAM_CTL_EN_P1_OB_EN) {
               //Enable
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf))  + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, OB_EN, 1,ISP_DRV_USER_ISPF);
               ISP_FUNC_DBG("Enable Obc");

            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, OB_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_FUNC_DBG("Disable Obc");
            }
        }

        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Rmg) {
            ISP_FUNC_DBG("RMG");

            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_CTL_IHDR, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_CTL_IHDR");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }

            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_CTL_IHDR);
            if(this->isTwinMode){
                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx, CAM_CTL_IHDR_D);
            }
            if (cfg.tuningEnP1 & CAM_CTL_EN_P1_RMG_EN) {
               //Enable
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf))  + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, RMG_EN, 1,ISP_DRV_USER_ISPF);

               ISP_FUNC_DBG("Enable RMG");

            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, RMG_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_FUNC_DBG("Disable RMG");
            }
#if 0
            {
                isp_reg_t*  cqVirtReg = (isp_reg_t*) _targetVirDrv->getRegAddr();
                ISP_DRV_CQ_MODULE_INFO_STRUCT outInfo;
                memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
                _targetVirDrv->getIspCQModuleInfo(CAM_CTL_IHDR, outInfo);
                MUINT32* ptr = (MUINT32*) ((((MUINT8*)cqVirtReg) + (outInfo.addr_ofst)));
                for (int xx = 0; xx < outInfo.reg_num; xx++, ptr++) {
                    ISP_FUNC_DBG("IHDR[%d] = 0x%x", xx, *ptr);
                }
            }
#endif
        }
        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Bnr) {
            //bnr && bpci have no sync issue because of auto bnr
            ISP_FUNC_DBG("BNR");
            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_BNR, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_ISP_BNR");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }


            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_BNR);
            if(this->isTwinMode){
                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx, CAM_ISP_BNR_D);
            }
            if (cfg.tuningEnP1 & CAM_CTL_EN_P1_BNR_EN) {
               //Enable
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, BNR_EN, 1,ISP_DRV_USER_ISPF);
            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, BNR_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
            }

            //BPCI
            ISP_FUNC_DBG("BPCI");
            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_DMA_BPCI, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_ISP_OBC");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }

            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx, CAM_DMA_BPCI);
            if(this->isTwinMode){
                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx, CAM_DMA_BPCI_D);
            }
            if (cfg.tuningEnP1Dma & CAM_CTL_EN_P1_DMA_BPCI_EN) {
                //Enable
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA, BPCI_EN, 1,ISP_DRV_USER_ISPF);
                //special case for twin_drv, special case: bpci|leci dma en && baseaddr
                if(this->isTwinMode){
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, BPCI_D_EN, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_targetVirDrv, CAM_BPCI_D_BASE_ADDR, ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_BPCI_BASE_ADDR), ISP_DRV_USER_ISPF);
                    //ISP_FUNC_ERR("enable bpci_d_0x%x\n",ISP_READ_REG_NOPROTECT(m_pIspDrv, CAM_BPCI_BASE_ADDR));
                   }
            }else {
                //Disable
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA, BPCI_EN, 0,ISP_DRV_USER_ISPF);

                //special case for twin_drv, special case: bpci|leci dma en && baseaddr
                if(this->isTwinMode){
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, BPCI_D_EN, 0,ISP_DRV_USER_ISPF);
                    //ISP_FUNC_ERR("disable bpci_d\n");
                }
            }
            DMA_BPCI bpci;
            isp_reg_t* _pSrcReg = (isp_reg_t *)cfg.pTuningBuf;
            //bpci.isTwinMode = this->isTwinMode;
            bpci.dma_cfg.memBuf.base_pAddr = _pSrcReg->CAM_BPCI_BASE_ADDR.Raw;
            bpci.dma_cfg.memBuf.ofst_addr = _pSrcReg->CAM_BPCI_OFST_ADDR.Raw;
            bpci.dma_cfg.size.h = _pSrcReg->CAM_BPCI_YSIZE.Raw + 1;
            bpci.dma_cfg.size.stride = _pSrcReg->CAM_BPCI_STRIDE.Raw;
            bpci.dma_cfg.size.xsize = _pSrcReg->CAM_BPCI_XSIZE.Raw + 1;
            bpci.CQ = this->CQ;
            bpci.dupCqIdx = this->dupCqIdx;
            bpci.burstQueIdx = this->burstQueIdx;
            bpci.config();
        }


        {
            MUINT32* pLscCfg;
            MUINT32* pLsciCfg;
            MBOOL    lsc_en, lsci_en;
            ISP_DRV_CQ_MODULE_INFO_STRUCT lscOutInfo;
            ISP_DRV_CQ_MODULE_INFO_STRUCT lsciOutInfo;

            if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Lsc) {
                //LSC
                ISP_FUNC_DBG("LSC");
                memset(&lscOutInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
                if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_LSC, lscOutInfo)) {
                    ISP_FUNC_ERR("[ERROR] CAM_ISP_LSC");
                    if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                        ISP_FUNC_ERR("[Error]enTuningQue fail");
                    }
                    return -1;
                }

                pLscCfg = (MUINT32*)(((MUINT8*)(cfg.pTuningBuf)) + (lscOutInfo.addr_ofst));
                lsc_en = cfg.tuningEnP1 & CAM_CTL_EN_P1_LSC_EN?1:0;


                //LSCI
                memset(&lsciOutInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
                if (!_targetVirDrv->getIspCQModuleInfo(CAM_DMA_LSCI, lsciOutInfo)) {
                    //ISP_FUNC_ERR("[ERROR] CAM_ISP_LSCI");
                    if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                        ISP_FUNC_ERR("[Error]enTuningQue fail");
                    }
                    return -1;
                }

                pLsciCfg = (MUINT32*)(((MUINT8*)(cfg.pTuningBuf)) + (lsciOutInfo.addr_ofst));
                lsci_en = cfg.tuningEnP1Dma & CAM_CTL_EN_P1_DMA_LSCI_EN?1:0;


                if (lsci_en ^ lsc_en) {
                    ISP_FUNC_ERR("[Error] Lsc & Lsci shall be enable/disable together. %d,%d", lsc_en, lsci_en);
                } else {

                    if (checkLscLsciCfg(pLscCfg, pLsciCfg)) {
                         ISP_FUNC_ERR("[Error] Lsc & Lsci cfg is Invalid");
                    } else {

                        m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_LSC);
                        m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_DMA_LSCI);
                        if(this->isTwinMode){
                            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx, CAM_ISP_LSC_D);
                            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_DMA_LSCI_D);
                        }

                        if (lsc_en) {
                            ISP_FUNC_DBG("LSC+LSCI Enable");
                            DMA_LSCI lsci;
                            isp_reg_t* _pSrcReg = (isp_reg_t *)cfg.pTuningBuf;
                            //lsci.isTwinMode = this->isTwinMode;
                            lsci.dma_cfg.memBuf.base_pAddr = _pSrcReg->CAM_LSCI_BASE_ADDR.Raw;
                            lsci.dma_cfg.memBuf.ofst_addr = _pSrcReg->CAM_LSCI_OFST_ADDR.Raw;
                            lsci.dma_cfg.size.h = _pSrcReg->CAM_LSCI_YSIZE.Raw + 1;
                            lsci.dma_cfg.size.stride = _pSrcReg->CAM_LSCI_STRIDE.Raw;
                            lsci.dma_cfg.size.xsize = _pSrcReg->CAM_LSCI_XSIZE.Raw + 1;
                            lsci.CQ = this->CQ;
                            lsci.dupCqIdx = this->dupCqIdx;
                            lsci.burstQueIdx = this->burstQueIdx;
                            lsci.config();
                            memcpy((((MUINT8*)cqVirtReg)        + (lscOutInfo.addr_ofst)), \
                                  (((MUINT8*)(cfg.pTuningBuf)) + (lscOutInfo.addr_ofst)), \
                                  (lscOutInfo.reg_num)*sizeof(UINT32));
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA, LSCI_EN, 1,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, LSC_EN, 1,ISP_DRV_USER_ISPF);
                            if(this->isTwinMode){
                               //special case for twin_drv, special case: bpci|leci dma en && baseaddr
                               ISP_WRITE_REG(_targetVirDrv, CAM_LSCI_D_BASE_ADDR, ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_LSCI_BASE_ADDR), ISP_DRV_USER_ISPF);    //0x407c
                               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, LSCI_D_EN, 1,ISP_DRV_USER_ISPF);
                            }
                        }else {
                            ISP_FUNC_DBG("LSC+LSCI Disable");
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, LSC_EN, 0,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA, LSCI_EN, 0,ISP_DRV_USER_ISPF);
#ifdef _CmdQ_HwBug_Patch_
                            DMA_LSCI lsci;
                            //Set LSCI cfg as Default value to avoid Hw issue
                            isp_reg_t* _pSrcReg = (isp_reg_t *)cfg.pTuningBuf;
                            //lsci.isTwinMode = this->isTwinMode;
                            lsci.dma_cfg.memBuf.base_pAddr = lsciSwWorkaroundBufPa;
                            lsci.dma_cfg.memBuf.ofst_addr = _pSrcReg->CAM_LSCI_OFST_ADDR.Raw;
                            lsci.dma_cfg.size.h = 0x0 + 1;
                            lsci.dma_cfg.size.stride = 0;
                            lsci.dma_cfg.size.xsize = 0x7 + 1;
                            lsci.CQ = this->CQ;
                            lsci.dupCqIdx = this->dupCqIdx;
                            lsci.burstQueIdx = this->burstQueIdx;
                            lsci.config();
#endif
                            //lsc
                            memcpy((((MUINT8*)cqVirtReg)        + (lscOutInfo.addr_ofst)), \
                                (((MUINT8*)(cfg.pTuningBuf)) + (lscOutInfo.addr_ofst)), \
                                (lscOutInfo.reg_num)*sizeof(UINT32));
#ifdef _CmdQ_HwBug_Patch_
                            if(this->isTwinMode){
                                DMA_LSCI_D lsci_d;
                                //Set LSCI cfg as Default value to avoid Hw issue
                                isp_reg_t* _pSrcReg = (isp_reg_t *)cfg.pTuningBuf;
                                lsci_d.isTwinMode = this->isTwinMode;
                                lsci_d.dma_cfg.memBuf.base_pAddr = lsciSwWorkaroundBufPa;
                                lsci_d.dma_cfg.memBuf.ofst_addr = _pSrcReg->CAM_LSCI_D_OFST_ADDR.Raw;
                                lsci_d.dma_cfg.size.h = 0x0 + 1;
                                lsci_d.dma_cfg.size.stride = 0;
                                lsci_d.dma_cfg.size.xsize = 0x7 + 1;
                                lsci_d.CQ = this->CQ;
                                lsci_d.dupCqIdx = this->dupCqIdx;
                                lsci_d.burstQueIdx = this->burstQueIdx;
                                lsci_d.config();
                                //special case for twin_drv, special case: bpci|leci dma en && baseaddr
                                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, LSCI_D_EN, 0,ISP_DRV_USER_ISPF);
                            }
#endif
                        }
                        if(this->isTwinMode){
                            this->m_twin_sdnumX = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_LSC_CTL2)>>12 & 0x1f;
                            this->m_twin_lblkW = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_LSC_LBLOCK)>>16 & 0xFFF;
                        }
                    }
                }
            }
        }

        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Rpg) {

            ISP_FUNC_DBG("Rpg");

            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_RPG, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_ISP_BNR");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }

            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_RPG);
            if(this->isTwinMode){
                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx, CAM_ISP_RPG_D);
            }
            if (cfg.tuningEnP1 & CAM_CTL_EN_P1_RPG_EN) {
               //Enable
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, RPG_EN, 1,ISP_DRV_USER_ISPF);
            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1, RPG_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
            }
        }
    }

    //raw-D
    if (ISP_PASS1_D == this->pathD){

        if (NULL == m_pPhyIspDrv) {
            ISP_FUNC_ERR("NULL == m_pPhyIspDrv (p1_d)");
        }
        if(this->burstQueIdx != 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("p1_D support no BurstQue_D");
        }
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN) == 1){
            _targetDupQueIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        }else{
            _targetDupQueIdx = this->dupCqIdx;
        }
#else
        _targetDupQueIdx = this->dupCqIdx;
#endif
        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQueIdx);
        ISP_FUNC_INF("_targetVirDrv_D = 0x%x",_targetVirDrv);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR(" dupCmdQ == NULL, target dupCqIdx_D = 0x%x",_targetDupQueIdx);
            return -1;
        }

        isp_reg_t*  cqDVirtReg = (isp_reg_t*) _targetVirDrv->getRegAddr();


        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Obc_d) {
            ISP_FUNC_DBG("OBC_D");
            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_OBC_D, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_ISP_OBC_D");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }

            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_OBC_D);
            if (cfg.tuningEnP1D & CAM_CTL_EN_P1_D_OB_D_EN) {
               //Enable
               memcpy((((MUINT8*)cqDVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, OB_D_EN, 1,ISP_DRV_USER_ISPF);
            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, OB_D_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqDVirtReg)       + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
            }
        }

        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Rmg_d) {

            ISP_FUNC_DBG("RMG_D");

            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_CTL_IHDR_D, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_CTL_IHDR_D");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }

            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQueIdx,CAM_CTL_IHDR_D);

            if (cfg.tuningEnP1 & CAM_CTL_EN_P1_D_RMG_EN) {
               //Enable
               memcpy((((MUINT8*)cqDVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf))  + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, RMG_D_EN, 1,ISP_DRV_USER_ISPF);

               ISP_FUNC_DBG("Enable RMG_D");

            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, RMG_D_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqDVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_FUNC_DBG("Disable RMG_D");
            }

        }

        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Bnr_d) {
            ISP_FUNC_DBG("BNR_D");
            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_BNR_D, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_ISP_BNR");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }


            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_BNR_D);
            if (cfg.tuningEnP1D & CAM_CTL_EN_P1_D_BNR_D_EN) {
               //Enable
               memcpy((((MUINT8*)cqDVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, BNR_D_EN, 1,ISP_DRV_USER_ISPF);
            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, BNR_D_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqDVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
            }


            //BPCI
            {
                memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
                if (!_targetVirDrv->getIspCQModuleInfo(CAM_DMA_BPCI_D, outInfo)) {
                    ISP_FUNC_ERR("[ERROR] CAM_DMA_BPCI_D");
                    if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                        ISP_FUNC_ERR("[Error]enTuningQue fail");
                    }
                    return -1;
                }

                m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQueIdx, CAM_DMA_BPCI_D);
                if (cfg.tuningEnP1DmaD & CAM_CTL_EN_P1_DMA_D_BPCI_D_EN) {

                    //Enable
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, BPCI_D_EN, 1,ISP_DRV_USER_ISPF);
                }else {
                    //Disable
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, BPCI_D_EN, 0,ISP_DRV_USER_ISPF);
                }
                DMA_BPCI_D bpci_d;
                isp_reg_t* _pSrcReg = (isp_reg_t *)cfg.pTuningBuf;
                //bpci_d.isTwinMode = this->isTwinMode;
                bpci_d.dma_cfg.memBuf.base_pAddr = _pSrcReg->CAM_BPCI_D_BASE_ADDR.Raw;
                bpci_d.dma_cfg.memBuf.ofst_addr = _pSrcReg->CAM_BPCI_D_OFST_ADDR.Raw;
                bpci_d.dma_cfg.size.h = _pSrcReg->CAM_BPCI_D_YSIZE.Raw + 1;
                bpci_d.dma_cfg.size.stride = _pSrcReg->CAM_BPCI_D_STRIDE.Raw;
                bpci_d.dma_cfg.size.xsize = _pSrcReg->CAM_BPCI_D_XSIZE.Raw + 1;
                bpci_d.CQ_D = this->CQ_D;
                bpci_d.dupCqIdx = this->dupCqIdx;
                bpci_d.burstQueIdx = this->burstQueIdx;
                bpci_d.config();
            }

        }

        {
            MUINT32* pLscDCfg;
            MUINT32* pLsciDCfg;
            MBOOL    lscD_en, lsciD_en;
            ISP_DRV_CQ_MODULE_INFO_STRUCT lscDOutInfo;
            ISP_DRV_CQ_MODULE_INFO_STRUCT lsciDOutInfo;

            if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Lsc_d) {
                //LSC
                ISP_FUNC_DBG("LSC_D");
                memset(&lscDOutInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
                if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_LSC_D, lscDOutInfo)) {
                    ISP_FUNC_ERR("[ERROR] CAM_ISP_LSCD");
                    if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                        ISP_FUNC_ERR("[Error]enTuningQue fail");
                    }
                    return -1;
                }

                pLscDCfg = (MUINT32*)(((MUINT8*)(cfg.pTuningBuf)) + (lscDOutInfo.addr_ofst));
                lscD_en = cfg.tuningEnP1D & CAM_CTL_EN_P1_D_LSC_D_EN?1:0;


                //LSCI
                memset(&lsciDOutInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
                if (!_targetVirDrv->getIspCQModuleInfo(CAM_DMA_LSCI_D, lsciDOutInfo)) {
                    //ISP_FUNC_ERR("[ERROR] CAM_ISP_LSCI");
                    if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                        ISP_FUNC_ERR("[Error]enTuningQue fail");
                    }
                    return -1;
                }

                pLsciDCfg = (MUINT32*)(((MUINT8*)(cfg.pTuningBuf)) + (lsciDOutInfo.addr_ofst));
                lsciD_en = cfg.tuningEnP1DmaD & CAM_CTL_EN_P1_DMA_D_LSCI_D_EN?1:0;

                if (lsciD_en ^ lscD_en) {
                    ISP_FUNC_ERR("[Error] LscD & LsciD shall be enable/disable together. %d,%d", lscD_en, lsciD_en);
                } else {

                    if (checkLscLsciCfg(pLscDCfg, pLsciDCfg)) {
                         ISP_FUNC_ERR("[Error] LscD & LsciD cfg is Invalid");
                    } else {

                        m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_LSC_D);
                        m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQueIdx,CAM_DMA_LSCI_D);

                        if (lscD_en) {
                            ISP_FUNC_DBG("LSCD+LSCID Enable");

                            DMA_LSCI_D lsci_d;
                            isp_reg_t* _pSrcReg = (isp_reg_t *)cfg.pTuningBuf;
                            //lsci_d.isTwinMode = this->isTwinMode;
                            lsci_d.dma_cfg.memBuf.base_pAddr = _pSrcReg->CAM_LSCI_D_BASE_ADDR.Raw;
                            lsci_d.dma_cfg.memBuf.ofst_addr = _pSrcReg->CAM_LSCI_D_OFST_ADDR.Raw;
                            lsci_d.dma_cfg.size.h = _pSrcReg->CAM_LSCI_D_YSIZE.Raw + 1;
                            lsci_d.dma_cfg.size.stride = _pSrcReg->CAM_LSCI_D_STRIDE.Raw;
                            lsci_d.dma_cfg.size.xsize = _pSrcReg->CAM_LSCI_D_XSIZE.Raw + 1;
                            lsci_d.CQ_D = this->CQ_D;
                            lsci_d.dupCqIdx = this->dupCqIdx;;
                            lsci_d.burstQueIdx = this->burstQueIdx;
                            lsci_d.config();

                            memcpy((((MUINT8*)cqDVirtReg)       + (lscDOutInfo.addr_ofst)), \
                                  (((MUINT8*)(cfg.pTuningBuf)) + (lscDOutInfo.addr_ofst)), \
                                  (lscDOutInfo.reg_num)*sizeof(UINT32));

                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, LSCI_D_EN, 1,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, LSC_D_EN, 1,ISP_DRV_USER_ISPF);
                        }else {
                            ISP_FUNC_DBG("LSCD+LSCID Disable");
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, LSC_D_EN, 0,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_DMA_D, LSCI_D_EN, 0,ISP_DRV_USER_ISPF);

                            memcpy((((MUINT8*)cqDVirtReg)       + (lscDOutInfo.addr_ofst)), \
                                (((MUINT8*)(cfg.pTuningBuf)) + (lscDOutInfo.addr_ofst)), \
                                (lscDOutInfo.reg_num)*sizeof(UINT32));
#ifdef _CmdQ_HwBug_Patch_
                            //Set LSCI cfg as Default value to avoid Hw issue
                            DMA_LSCI_D lsci_d;
                            isp_reg_t* _pSrcReg = (isp_reg_t *)cfg.pTuningBuf;
                            //lsci_d.isTwinMode = this->isTwinMode;
                            lsci_d.dma_cfg.memBuf.base_pAddr = lsciSwWorkaroundBufPa;
                            lsci_d.dma_cfg.memBuf.ofst_addr = _pSrcReg->CAM_LSCI_D_OFST_ADDR.Raw;
                            lsci_d.dma_cfg.size.h = 0 + 1;
                            lsci_d.dma_cfg.size.stride = 0;
                            lsci_d.dma_cfg.size.xsize = 0x7 + 1;
                            lsci_d.CQ_D = this->CQ_D;
                            lsci_d.dupCqIdx = this->dupCqIdx;;
                            lsci_d.burstQueIdx = this->burstQueIdx;
                            lsci_d.config();
                            //ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN_P1_DMA_D, LSCI_D_EN, 0,ISP_DRV_USER_ISPF);
#endif
                        }
                    }
                }
            }
        }



        if (cfg.tuningUpdateBit & eIspTuningMgrFunc_Rpg_d) {
            ISP_FUNC_DBG("RPG_D");
            memset(&outInfo, 0, sizeof(ISP_DRV_CQ_MODULE_INFO_STRUCT));
            if (!_targetVirDrv->getIspCQModuleInfo(CAM_ISP_RPG_D, outInfo)) {
                ISP_FUNC_ERR("[ERROR] CAM_ISP_BNR");
                if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
                    ISP_FUNC_ERR("[Error]enTuningQue fail");
                }
                return -1;
            }

            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ_D,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_RPG_D);
            if (cfg.tuningEnP1D & CAM_CTL_EN_P1_D_RPG_D_EN) {
               //Enable
               memcpy((((MUINT8*)cqDVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, RPG_D_EN, 1,ISP_DRV_USER_ISPF);
            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN_P1_D, RPG_D_EN, 0,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqDVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
            }
        }
    }


    if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum , eIspTuningMgrFunc_Null)==MFALSE){
        ISP_FUNC_ERR("[Error]enTuningQue fail");
    }
    return 0;
}




/*/////////////////////////////////////////////////////////////////////////////
    ISP_RGB_PIPE
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_RGB_PIPE::_config( void )
{
    MUINT32 i;
    MUINT32 tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00, tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 tuningEnP2=0x00, tuningEnP2Dma=0x00;
    MUINT32 finalEnP2 = 0x00;
    IspTuningPackage tuningPackage;
    IspImageioPackage imageioPackage;
    //
    ISP_FUNC_DBG("+(ISP_RGB_PIPE),cq(%d),drvScenario(%d),sub_mode(%d),en_p2(0x%08x),enFgMode(%d)",this->CQ,this->drvScenario,this->sub_mode,this->en_p2,this->enFgMode);
    //
    if(this->isV3)
    {
        getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
        imageioPackage.isApplyTuning = this->isApplyTuning;
        imageioPackage.cq = this->CQ;
        imageioPackage.subMode = this->sub_mode;

        if(imageioPackage.isApplyTuning==MTRUE){
            tuningUpdateBit = 0xffffffff;
            tuningPackage.pTuningBuf = (MUINT32*)this->pTuningIspReg;
            if (tuningPackage.pTuningBuf == NULL){
                ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
                return 0;
            }
            tuningPackage.rrzRatio = this->rrzRatio;
            tuningPackage.rrzCropX = this->rrzCropX;
            tuningPackage.rrzCropY = this->rrzCropY;


            ISP_FUNC_DBG("tuningUpdateBit(0x%08x),pTuningBuf(0x%08x),hrz_crop(%d,%d)", \
                tuningUpdateBit,tuningPackage.pTuningBuf,tuningPackage.rrzCropX,tuningPackage.rrzCropY);
        }
        finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
        imageioPackage.finalEnP2 = finalEnP2;

        ISP_FUNC_INF("(ISP_RGB_PIPE),isApplyTuning(0x%x),finalEnP2(0x%x),tuningUpdateBit(0x%x)",this->isApplyTuning,finalEnP2,tuningUpdateBit);
    }
    else
    {
    getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
    imageioPackage.isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
    imageioPackage.cq = this->CQ;
    imageioPackage.subMode = this->sub_mode;
    imageioPackage.enFgMode = this->enFgMode;
    if(imageioPackage.isApplyTuning==MTRUE){
        if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE){
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            return -1;
        }
                tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2,this->magicNum, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario);
        tuningEnP2 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
        tuningEnP2Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);
        tuningPackage.pTuningBuf = m_pPhyIspDrv->getTuningBuf(eTuningQueAccessPath_imageio_P2, this->CQ);
        if (tuningPackage.pTuningBuf == NULL){
            ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
            return -1;
        }
        tuningPackage.rrzRatio = this->rrzRatio;
        tuningPackage.rrzCropX = this->rrzCropX;
        tuningPackage.rrzCropY = this->rrzCropY;
        ISP_FUNC_DBG("tuningUpdateBit(0x%08x),tuningEnP2(0x%08x),tuningEnP2Dma(0x%08x),pTuningBuf(0x%08x)",tuningUpdateBit,tuningEnP2,tuningEnP2Dma,tuningPackage.pTuningBuf);
    }
    finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
    }

    //mfb
    if(this->en_p2 & CAM_CTL_EN_P2_MFB_EN)
    {
        imageioPackage.mfbCfg=this->mfb_cfg;
        //ISP_FUNC_DBG("[TEST] this->mfb_cfg.bld_mode(%d),x(%d),y(%x),xofst(%d)",this->mfb_cfg.bld_mode,this->mfb_cfg.mfb_out.w,this->mfb_cfg.mfb_out.h,this->mfb_cfg.out_xofst);
        ISP_FUNC_DBG("[TEST] imagioPak->mfb_cfg.bld_mode(%d),x(%d),y(%d),xofst(%d)",imageioPackage.mfbCfg.bld_mode,imageioPackage.mfbCfg.mfb_out.w,\
            imageioPackage.mfbCfg.mfb_out.h,imageioPackage.mfbCfg.out_xofst);
    }
    //
    for(i=0;i<CHECK_TUNING_P2_EN_BITS;i++){
        if(     ((finalEnP2 & gIspModuleMappingTableP2En[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2)) &&
                (gIspModuleMappingTableP2En[i].eModule==eIspModuleRgb) &&
                (gIspModuleMappingTableP2En[i].default_func!=NULL)) {
            //
            if(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)
                tuningPackage.eTuningCqFunc1 = gIspModuleMappingTableP2En[i].eTuningCqFunc1;
            else
                tuningPackage.eTuningCqFunc1 = CAM_DUMMY_;
            //
            if(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2)
                tuningPackage.eTuningCqFunc2 = gIspModuleMappingTableP2En[i].eTuningCqFunc2;
            else
                tuningPackage.eTuningCqFunc2 = CAM_DUMMY_;
            //
            if(((tuningPackage.eTuningCqFunc1!=CAM_DUMMY_)||(tuningPackage.eTuningCqFunc2!=CAM_DUMMY_))&&(gIspModuleMappingTableP2En[i].eTuningCqDma!=CAM_DUMMY_))
                tuningPackage.eTuningCqDma = gIspModuleMappingTableP2En[i].eTuningCqDma;
            else
                tuningPackage.eTuningCqDma = CAM_DUMMY_;

			tuningPackage.sl2cCrzRatio = 0;

            gIspModuleMappingTableP2En[i].default_func(imageioPackage, tuningPackage);
            //
        }
    }

    if (! this->isV3)
    {
    if(imageioPackage.isApplyTuning==MTRUE){
        if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE){
            ISP_FUNC_ERR("[Error]enTuningQue fail");
        }
    }
    }
    //
    ISP_FUNC_DBG("-(ISP_RGB_PIPE),p2TuningEnTag(0x%08x),p2EnConfig(0x%08x)",tuningEnP2Tag,finalEnP2);
    return 0;
}

int ISP_RGB_PIPE::_write2CQ(void)
{
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00;
    MUINT32 tuningEnP2=0x00, tuningEnP2Dma=0x00;
    MUINT32 finalEnP2 = 0x00;
    MUINT32 i;
    MBOOL isApplyTuning;

    ISP_FUNC_DBG("+(ISP_RGB_PIPE),cq(%d),drvScenario(%d),sub_mode(%d),en_p2(0x%08x)",this->CQ,this->drvScenario,this->sub_mode,this->en_p2);
    //
    if(this->isV3)
    {
        ISP_FUNC_INF("ISP_RGB_PIPE::_write2CQ");

        getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
        isApplyTuning = this->isApplyTuning;
        if(isApplyTuning==MTRUE){
            tuningUpdateBit = 0xffffffff;
            ISP_FUNC_DBG("tuningUpdateBit(0x%08x)",tuningUpdateBit);
        }
        finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
    }
    else
   {
    getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
    isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
    if(isApplyTuning==MTRUE){
        if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE){
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            return -1;
        }
                    tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2,this->magicNum, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario);
        tuningEnP2 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
        tuningEnP2Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);
        if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE){
            ISP_FUNC_ERR("[Error]enTuningQue fail");
        }
    }
    finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
    }
    //
    for(i=0;i<CHECK_TUNING_P2_EN_BITS;i++){
        if(((finalEnP2 & gIspModuleMappingTableP2En[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2))
                &&(gIspModuleMappingTableP2En[i].eModule==eIspModuleRgb)){  // check bit en
            if(gIspModuleMappingTableP2En[i].eTuningCqFunc1!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqFunc1);
            if(gIspModuleMappingTableP2En[i].eTuningCqFunc2!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqFunc2);
            if(gIspModuleMappingTableP2En[i].eTuningCqDma!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqDma);
        }
    }
    //imageio setting
    if(this->en_p2 & CAM_CTL_EN_P2_MFB_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_MFB_CTRL);
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_MFB_IMGCON);
    }
    //
    ISP_FUNC_DBG("-(ISP_RGB_PIPE),p2TuningEnTag(0x%08x),p2EnConfig(0x%08x)",tuningEnP2Tag,finalEnP2);
    //
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    ISP_YUV_PIPE
/////////////////////////////////////////////////////////////////////////////*/
int ISP_YUV_PIPE::_config( void )
{
    MUINT32 i;
    MUINT32 tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00, tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 tuningEnP2=0x00, tuningEnP2Dma=0x00;
    MUINT32 finalEnP2 = 0x00;
    MUINT32 enCrsp=0x00;
    IspTuningPackage tuningPackage;
    IspImageioPackage imageioPackage;
    //
    ISP_FUNC_DBG("+(ISP_YUV_PIPE),cq(%d),scenario(%d),sub_mode(%d),en_p2(0x%08x)",this->CQ,this->drvScenario,this->sub_mode,this->en_p2);

    //
    
    if (this->isV3)
    {
        getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
        imageioPackage.isApplyTuning = this->isApplyTuning;
        imageioPackage.cq = this->CQ;
        imageioPackage.subMode = this->sub_mode;
		imageioPackage.enFgMode = 0;

        if(imageioPackage.isApplyTuning==MTRUE){
            tuningUpdateBit = 0xffffffff;
            tuningPackage.pTuningBuf = (MUINT32*)this->pTuningIspReg;
            if (tuningPackage.pTuningBuf == NULL){
                ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
                return 0;
            }
			tuningPackage.rrzRatio = 0;
            tuningPackage.rrzCropX = this->rrzCropX; //QQ
            tuningPackage.rrzCropY = this->rrzCropY;
            tuningPackage.sl2cCrzRatio = this->sl2cCrzRatio;
            tuningPackage.lceInputW = this->lceInputW;
            tuningPackage.lceInputH = this->lceInputH;
            tuningPackage.nr3dGainWeighting = this->nr3dCfg.gain_weighting;
            memcpy(&imageioPackage.nr3dCfg, &this->nr3dCfg, sizeof(Nr3DCfg));

            ISP_FUNC_DBG("tuningUpdateBit(0x%08x),tuningEnP2(0x%08x),tuningEnP2Dma(0x%08x),pTuningBuf(0x%08x),nr3dGainWeighting(0x%x)",tuningUpdateBit,tuningEnP2,tuningEnP2Dma,tuningPackage.pTuningBuf,tuningPackage.nr3dGainWeighting);
        }
        finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
        imageioPackage.finalEnP2 = finalEnP2;

        ISP_FUNC_INF("(ISP_YUV_PIPE),isApplyTuning(0x%x),finalEnP2(0x%x),tuningUpdateBit(0x%x)",this->isApplyTuning,finalEnP2,tuningUpdateBit);
    }
    else
    {
    getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
    imageioPackage.isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
    imageioPackage.cq = this->CQ;
    imageioPackage.subMode = this->sub_mode;
	imageioPackage.enFgMode = 0;

    if(imageioPackage.isApplyTuning==MTRUE){
        if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE){
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            return -1;
        }
            tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2,this->magicNum, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario);
        tuningEnP2 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
        tuningEnP2Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);
        tuningPackage.pTuningBuf = m_pPhyIspDrv->getTuningBuf(eTuningQueAccessPath_imageio_P2, this->CQ);
        if (tuningPackage.pTuningBuf == NULL){
            ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
            return 0;
        }
		
		tuningPackage.rrzRatio = 0;
        tuningPackage.rrzCropX = this->rrzCropX;
        tuningPackage.rrzCropY = this->rrzCropY;
        tuningPackage.sl2cCrzRatio = this->sl2cCrzRatio;
        tuningPackage.lceInputW = this->lceInputW;
        tuningPackage.lceInputH = this->lceInputH;
        tuningPackage.nr3dGainWeighting = this->nr3dCfg.gain_weighting;
        memcpy(&imageioPackage.nr3dCfg, &this->nr3dCfg, sizeof(Nr3DCfg));

        ISP_FUNC_DBG("tuningUpdateBit(0x%08x),tuningEnP2(0x%08x),tuningEnP2Dma(0x%08x),pTuningBuf(0x%08x),nr3dGainWeighting(0x%x)",tuningUpdateBit,tuningEnP2,tuningEnP2Dma,tuningPackage.pTuningBuf,tuningPackage.nr3dGainWeighting);
    }
    finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
    imageioPackage.finalEnP2 = finalEnP2;
    }

    //nr3d
    if(this->en_p2 & CAM_CTL_EN_P2_MFB_EN)
    {
        imageioPackage.nr3dCfg=this->nr3dCfg;
    }
    // error check for lcei
    if(this->en_p2&CAM_CTL_EN_P2_SRZ2_EN &&  this->en_p2&CAM_CTL_EN_P2_LCE_EN){
        ISP_FUNC_ERR("[Error]Both srz2 and lce be enabled");
        return 0;
    }

    //
    for(i=0;i<CHECK_TUNING_P2_EN_BITS;i++){
        if(     ((finalEnP2 & gIspModuleMappingTableP2En[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2)) &&
                (gIspModuleMappingTableP2En[i].eModule==eIspModuleYuv) &&
                (gIspModuleMappingTableP2En[i].default_func!=NULL)) {
            //
            if(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)
                tuningPackage.eTuningCqFunc1 = gIspModuleMappingTableP2En[i].eTuningCqFunc1;
            else
                tuningPackage.eTuningCqFunc1 = CAM_DUMMY_;
            //
            if(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2)
                tuningPackage.eTuningCqFunc2 = gIspModuleMappingTableP2En[i].eTuningCqFunc2;
            else
                tuningPackage.eTuningCqFunc2 = CAM_DUMMY_;
            //
            if(((tuningPackage.eTuningCqFunc1!=CAM_DUMMY_)||(tuningPackage.eTuningCqFunc2!=CAM_DUMMY_))&&(gIspModuleMappingTableP2En[i].eTuningCqDma!=CAM_DUMMY_))
                tuningPackage.eTuningCqDma = gIspModuleMappingTableP2En[i].eTuningCqDma;
            else
                tuningPackage.eTuningCqDma = CAM_DUMMY_;

            gIspModuleMappingTableP2En[i].default_func(imageioPackage, tuningPackage);
            //
        }
    }
    if (! this->isV3)
    {
    if(imageioPackage.isApplyTuning==MTRUE){
        if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE){
            ISP_FUNC_ERR("[Error]enTuningQue fail");
        }
    }
    }
    // CRSP control
    enCrsp = (this->en_p2 & CAM_CTL_EN_P2_CRSP_EN)?(0x01):(0x00);
    this->m_pIspDrvShell->cam_isp_crsp_cfg(enCrsp,this->crsp_cfg.out.w,this->crsp_cfg.out.h);

    // C02 control
    if(this->en_p2 & CAM_CTL_EN_P2_C02_EN)
    {
        this->m_pIspDrvShell->cam_isp_c02_cfg(this->c02_cfg.in.w,this->c02_cfg.in.h);
    }
    // vfb related
    if(this->en_p2 & CAM_CTL_EN_P2_MIX1_EN)
    {
        this->m_pIspDrvShell->cam_isp_mix1_cfg();
    }
    if(this->en_p2 & CAM_CTL_EN_P2_MIX2_EN)
    {
        this->m_pIspDrvShell->cam_isp_mix2_cfg();
    }
    if(this->en_p2 & CAM_CTL_EN_P2_SRZ1_EN)
    {
        MUINT32 ctrl=0x3;   //srz1_vert_en=1 & srz1_hori_en=1
        if(this->srz1_cfg.inout_size.out_w>this->srz1_cfg.inout_size.in_w)
        {
            ctrl |= 0x10;
        }
        this->m_pIspDrvShell->cam_isp_srz1_cfg(ctrl,this->srz1_cfg.inout_size,this->srz1_cfg.crop,this->srz1_cfg.h_step,this->srz1_cfg.v_step);
    }
    if(this->en_p2 & CAM_CTL_EN_P2_SRZ2_EN)
    {
        MUINT32 ctrl=0x3;   //srz2_vert_en=1 & srz2_hori_en=1
        if(this->srz2_cfg.inout_size.out_w>this->srz2_cfg.inout_size.in_w)
        {
            ctrl |= 0x10;
        }
        this->m_pIspDrvShell->cam_isp_srz2_cfg(ctrl,this->srz2_cfg.inout_size,this->srz2_cfg.crop,this->srz2_cfg.h_step,this->srz2_cfg.v_step);
    }
#if 0
    //FE
    if(this->en_p2 & CAM_CTL_EN_P2_FE_EN)
    {
        ISP_WRITE_REG(m_pP2IspDrv,CAM_FE_CTRL,this->fe_ctrl,ISP_DRV_USER_ISPF);
    }
#endif
    //
    ISP_FUNC_DBG("-(ISP_YUV_PIPE),p2TuningEnTag(0x%08x),p2EnConfig(0x%08x)",tuningEnP2Tag,finalEnP2);
    return 0;

}


int ISP_YUV_PIPE::_write2CQ(void)
{
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00;
    MUINT32 tuningEnP2=0x00, tuningEnP2Dma=0x00;
    MUINT32 finalEnP2 = 0x00;
    MUINT32 i;
    MBOOL isApplyTuning;
    //

    ISP_FUNC_DBG("+(ISP_YUV_PIPE),cq(%d),drvScenario(%d),sub_mode(%d),en_p2(0x%08x)",this->CQ,this->drvScenario,this->sub_mode,this->en_p2);
    //
    if (this->isV3)
    {
         ISP_FUNC_INF("ISP_YUV_PIPE::_write2CQ");
        getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
           isApplyTuning = this->isApplyTuning;
        if(isApplyTuning==MTRUE){
            tuningUpdateBit = 0xffffffff;
            ISP_FUNC_DBG("tuningUpdateBit(0x%08x)",tuningUpdateBit);
        }
        finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
    }
    else
    {
    getP2EnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->magicNum, this->drvScenario, this->sub_mode, tuningEnP2Tag, tuningEnP2DmaTag); // get P2 tuning En & Dma Tag
    isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
    if(isApplyTuning==MTRUE){
        if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE){
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            return -1;
        }
            tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2,this->magicNum, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario );
        tuningEnP2 = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
        tuningEnP2Dma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);

        if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE){
            ISP_FUNC_ERR("[Error]enTuningQue fail");
        }
    }
    finalEnP2 = ((~tuningEnP2Tag)&this->en_p2)|(tuningEnP2Tag&tuningEnP2);
    }
    //
    for(i=0;i<CHECK_TUNING_P2_EN_BITS;i++){
        if(((finalEnP2 & gIspModuleMappingTableP2En[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableP2En[i].eTuningFunc2))
                &&(gIspModuleMappingTableP2En[i].eModule==eIspModuleYuv)){  // check bit en
            if(gIspModuleMappingTableP2En[i].eTuningCqFunc1!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqFunc1);
            if(gIspModuleMappingTableP2En[i].eTuningCqFunc2!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqFunc2);
            if(gIspModuleMappingTableP2En[i].eTuningCqDma!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableP2En[i].eTuningCqDma);
        }
    }
    //
    //imageio setting
    if(this->en_p2 & CAM_CTL_EN_P2_LCE_EN){
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_LCE_IMAGE_SIZE);
    }
    // CRSP control
    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_CRSP); //need to disable for scenario without nr3d
    //
    // C02 control
    if(this->en_p2 & CAM_CTL_EN_P2_C02_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_C02);
    }
    // vfb related
    if(this->en_p2 & CAM_CTL_EN_P2_MIX1_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_CDP_MIX1);
    }
    if(this->en_p2 & CAM_CTL_EN_P2_MIX2_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_CDP_MIX2);
    }
    if(this->en_p2 & CAM_CTL_EN_P2_SRZ1_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_CDP_SRZ1);
    }
    if(this->en_p2 & CAM_CTL_EN_P2_SRZ2_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_CDP_SRZ2);
    }
    //NR3D
    if(this->en_p2 & CAM_CTL_EN_P2_NR3D_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_CDP_NR3D_CTRL);
    }
    //FE
    if(this->en_p2 & CAM_CTL_EN_P2_FE_EN)
    {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_CDP_FE);
      }
    ISP_FUNC_DBG("-(ISP_YUV_PIPE),p2TuningEnTag(0x%08x),p2EnConfig(0x%08x)",tuningEnP2Tag,finalEnP2);


    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    ISP_MDP_PIPE
/////////////////////////////////////////////////////////////////////////////*/
MBOOL ISP_MDP_PIPE::createMdpMgr( void )
{
    ISP_FUNC_INF("+");
    MBOOL err = MTRUE;

   // if(g_pMdpMgr == NULL){
        g_pMdpMgr = MdpMgr::createInstance();

        if(g_pMdpMgr == NULL){
            err = MFALSE;
            ISP_FUNC_ERR("[Error]fail to create MDP instance");
        }

        // init MdpMgr
        if(MDPMGR_NO_ERROR != g_pMdpMgr->init()){
            ISP_FUNC_ERR("[Error]g_pMdpMgr->init fail");
            err = MFALSE;
        }
   // }

    ISP_FUNC_INF("-");

    return err;
}

MBOOL ISP_MDP_PIPE::destroyMdpMgr( void )
{
    ISP_FUNC_INF("+");
    MBOOL err = MTRUE;
    MINT32 mdpreturn=0;

    if(g_pMdpMgr)
    {
        // uninit MdpMgr
        mdpreturn=g_pMdpMgr->uninit();
        if((MDPMGR_NO_ERROR != mdpreturn) && MDPMGR_STILL_USERS != mdpreturn){
            ISP_FUNC_ERR("[Error]g_pMdpMgr->uninit fail");
            err = MFALSE;
        }
        if(MDPMGR_STILL_USERS != mdpreturn)
        {
            g_pMdpMgr->destroyInstance();
            g_pMdpMgr = NULL;
        }
        else
        {
            ISP_FUNC_INF("still users");
        }
    }
    else
    {
        err = MFALSE;
        ISP_FUNC_ERR("[Error]g_pMdpMgr = NULL");
    }

    ISP_FUNC_INF("-");

    return err;
}
MBOOL ISP_MDP_PIPE::startVideoRecord( MINT32 wd,MINT32 ht, MINT32 fps,NSImageio::NSIspio::EDrvScenario drvScen,MUINT32 cqIdx)
{
    ISP_FUNC_INF("+");
    MBOOL err = MTRUE;
    if(g_pMdpMgr)
    {
        g_pMdpMgr->startVideoRecord(wd,ht,fps,(MUINT32)(drvScen),cqIdx);
    }
    else
    {
        err = MFALSE;
        ISP_FUNC_ERR("[Error]g_pMdpMgr = NULL");
    }
    ISP_FUNC_INF("-");
    return err;
}
MBOOL ISP_MDP_PIPE::stopVideoRecord( NSImageio::NSIspio::EDrvScenario drvScen,MUINT32 cqIdx)
{
    ISP_FUNC_INF("+");
    MBOOL err = MTRUE;
    if(g_pMdpMgr)
    {
        g_pMdpMgr->stopVideoRecord((MUINT32)(drvScen),cqIdx);
    }
    else
    {
        err = MFALSE;
        ISP_FUNC_ERR("[Error]g_pMdpMgr = NULL");
    }
    ISP_FUNC_INF("-");
    return err;
}
int ISP_MDP_PIPE::_config( void )
{
    MUINT32 ret = ISP_ERROR_CODE_OK;
    MBOOL enPureMdpOut = MFALSE;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    //
    this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
    ISP_FUNC_DBG("+,CQ(%d),p2Cq(%d),burstQueIdx(%d),dupCqIdx(%d),drvScenario(%d),p2Dma(0x%x),isWaitBuf(%d),imgxoEnP2(0x%x)",this->CQ,p2Cq,this->burstQueIdx,this->dupCqIdx,this->drvScenario,this->p2_dma_enable,this->isWaitBuf,this->mdp_imgxo_p2_en);

//    ISP_FUNC_INF("QQ ISP_MDP_PIPE,crop(x:%d,y:%d,w:%d,h:%d)", \
//        this->src_crop.x, \
//        this->src_crop.y, \
//        this->src_crop.w, \
//        this->src_crop.h);

    if(this->CQ!=ISP_DRV_CQ01 && this->CQ!=ISP_DRV_CQ02 && this->CQ!=ISP_DRV_CQ03){
        ISP_FUNC_ERR("[Error]MDP not support this cq(%d)",this->CQ);
        ret = ISP_ERROR_CODE_FAIL;
    } else{
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].ispTpipeCfgInfo.drvinfo.p2Cq = (ISP_TPIPE_P2_CQ_ENUM)p2Cq;
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].cqIdx = this->CQ;
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].drvScenario = this->drvScenario;
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].isWaitBuf = this->isWaitBuf;
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].imgxoEnP2 = this->mdp_imgxo_p2_en;
    }

    // initialize
    for(MINT32 i = 0; i < ISP_MDP_DL_NUM; i++){
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[i] = 0;
    }

    // WROTO
    if(CAM_CTL_EN_P2_DMA_MDP_WROTO_EN & this->p2_dma_enable)
    {
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_WROTO] = 1;
        memcpy(&gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_WROTO], &this->wroto_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // WDMAO
    if(CAM_CTL_EN_P2_DMA_MDP_WDMAO_EN & this->p2_dma_enable)
    {
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_WDMAO] = 1;
        memcpy(&gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_WDMAO], &this->wdmao_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // WJPEGO
    if(CAM_CTL_EN_P2_DMA_MDP_JPEGO_EN & this->p2_dma_enable)
    {
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_JPEGO] = 1;
        memcpy(&gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_JPEGO], &this->jpego_out, sizeof(MdpRotDMACfg));
        memcpy(&gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstJpgParaCfg, &this->jpgpara_out, sizeof(JpgParaCfg));
        enPureMdpOut = MTRUE;
    }
    //VENCO
    if(CAM_CTL_EN_P2_DMA_MDP_VENC_EN & this->p2_dma_enable)
    {
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_VENCO] = 1;
        memcpy(&gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_VENCO], &this->venco_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // ISP IMGXO
    if(enPureMdpOut == MFALSE){
        ISP_FUNC_DBG("imgxo va(0x%p),pa(0x%x),size(%d)",this->imgxo_out.memBuf.base_vAddr,this->imgxo_out.memBuf.base_pAddr,this->imgxo_out.memBuf.size);
        gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_IMGXO] = 1;
        memcpy(&gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO], &this->imgxo_out, sizeof(MdpRotDMACfg));
//        ISP_FUNC_INF("QQ,burstQueIdx (%d),p2Cq (%d),dupCqIdx (%d)",this->burstQueIdx,p2Cq,this->dupCqIdx);

//        ISP_FUNC_INF("QQ,imgxo srcCropW(%d),srcCropH(%d)",gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropW ,gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropH);

    // collect crop info for MdpMgr

        if (this->drvScenario != NSImageio::NSIspio::eDrvScenario_IP)
        {
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropX       = this->src_crop.x;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropFloatX  = this->src_crop.floatX;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropY       = this->src_crop.y;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropFloatY  = this->src_crop.floatY;

            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropW       = this->src_crop.w;
            gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO].srcCropH       = this->src_crop.h;
        }

        ISP_FUNC_INF("QQ,use this srcCropW(%d),srcCropH(%d)",this->src_crop.w,this->src_crop.h);
    }

    ISP_FUNC_DBG("-,");

    return ret;

}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_TURNING_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_TURNING_CTRL::_config( void )
{

    #if 0
    ISP_FUNC_DBG("[CQ:%d] isApplyTurn(0x%x)E ",this->CQ,m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ));

    if(this->CQ == CAM_ISP_CQ1 && m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ))
        m_pPhyIspDrv->updateTurningCq1();
    else if(this->CQ == CAM_ISP_CQ2 && m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ))
        m_pPhyIspDrv->updateTurningCq2();

    ISP_FUNC_DBG("_config:X ");
    #endif

    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    CAM_CRZ_PIPE
  /////////////////////////////////////////////////////////////////////////////*/



//
int CAM_CRZ_PIPE::_disable( void )
{
    ISP_FUNC_DBG("CAM_CRZ_PIPE::_disable:E,path(0x%x) ",this->path);
//    if (ISP_PASS1 != this->path) {
//    // Release all ROTDMA SYSRAM buffer.
//    for (MUINT32 RotDma = 0; RotDma < CDP_DRV_ROTDMA_AMOUNT; RotDma++)
//        {
//          this->m_pIspDrvShell->m_pCdpDrv->FreeRotationBuf((CDP_DRV_ROTDMA_ENUM)RotDma);
//        }
//    }
    return 0;
}

//
int CAM_CRZ_PIPE::_config( void )
{
#if 1 //_mt6593fpga_dvt_use_

    CRZ_DRV_IMG_SIZE_STRUCT sizeIn;
    CRZ_DRV_IMG_SIZE_STRUCT sizeOut;
    CRZ_DRV_IMG_CROP_STRUCT crop;
    MBOOL crz_result = MTRUE;

    ISP_FUNC_DBG("CAM_CRZ_PIPE::_config,tcm(%d):E",this->tpipeMode);

    if ( this->conf_cdrz ) {
        ISP_FUNC_DBG("CDRZ_Config ");

        //CDRZ
        sizeIn.Width = this->crz_in.w;
        sizeIn.Height = this->crz_in.h;
        sizeOut.Width = this->crz_out.w;
        sizeOut.Height = this->crz_out.h;
        // crop
        crop.Width.Start = this->crz_crop.x;
        crop.Width.Size = this->crz_crop.w;
        crop.Height.Start = this->crz_crop.y;
        crop.Height.Size = this->crz_crop.h;
        //
        crz_result = this->m_pIspDrvShell->m_pP2CrzDrv->CDRZ_Config(CRZ_DRV_MODE_TPIPE,sizeIn,sizeOut,crop);
        if (MFALSE == crz_result) {
            ISP_FUNC_ERR(" CDRZ_Config");
            return -1;
        }
    }

#endif

    ISP_FUNC_DBG("CAM_CRZ_PIPE::_config:X ");
    return 0;
}

//
int CAM_CRZ_PIPE::_write2CQ(void)
{
    ISP_FUNC_DBG("CAM_CRZ_PIPE::_write2CQ:E ");

#if 1 //_mt6593fpga_dvt_use_
    /*
    //CRZ
    CAM_ISP_CRZ,        //15   15004B00~15004B38
    */
    if ( this->conf_cdrz ) {
        //crz
        ISP_FUNC_DBG("CQ:push CAM_ISP_CRZ ");
        //this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_ISP_CRZ);
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_ISP_CRZ);

    }

#endif

    ISP_FUNC_DBG("CAM_CRZ_PIPE::_write2CQ:X ");
    return 0;
}
//
int CAM_CRZ_PIPE::_setZoom( void )
{
    unsigned int *data_ptr = NULL;

    ISP_FUNC_DBG(":E ");
    ISP_FUNC_DBG("0x%08x ",this->conf_cdrz);

    if ( this->conf_cdrz ) {
        ISP_FUNC_DBG("data_ptr:0x%08x",data_ptr);
    }

    ISP_FUNC_DBG(":X ");
    return 0;
}




/*/////////////////////////////////////////////////////////////////////////////
    CAM_TDRI_PIPE
/////////////////////////////////////////////////////////////////////////////*/
MBOOL CAM_TDRI_PIPE::configTdriSetting(
    TdriDrvCfg* pSrcImageio)
{
    MBOOL ret = MTRUE;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    ISP_TPIPE_CONFIG_STRUCT *pDst = NULL;
    MBOOL isApplyTuning=MFALSE;
    MUINT32 p2TuningTopEnTag=0x00, p2TuningTopDmaTag=0x00;
    MUINT32 p1TuningTopEnTag=0x00, p1TuningTopDmaTag=0x00;
    MUINT32 p2TuningFieldEnTag=0x00, p2TuningFieldDmaTag=0x00;
    MUINT32 p1TuningFieldEnTag=0x00, p1TuningFieldDmaTag=0x00;
    MUINT32 p2TuningEn=0x00, p2TuningDma=0x00;
    MUINT32 p1TuningEn=0x00, p1TuningDma=0x00;
    //MINT32 tmpSrc=-1, tmpDst=-1;
    //
    //ISP_FUNC_INF("this->burstQueIdx(%d),p2Cq(%d),this->dupCqIdx(%d)",this->burstQueIdx,p2Cq,this->dupCqIdx);


    this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
    pDst = &gpMdpMgrCfgData[this->burstQueIdx][p2Cq][this->dupCqIdx].ispTpipeCfgInfo;

    /* update imageio parameters */
    pDst->top.scenario = pSrcImageio->top.scenario;
    pDst->top.mode = pSrcImageio->top.mode;
    pDst->top.pixel_id = pSrcImageio->top.pixel_id;
    pDst->top.cam_in_fmt = pSrcImageio->top.cam_in_fmt;
    pDst->top.ctl_extension_en = pSrcImageio->top.ctl_extension_en;
    pDst->top.fg_mode = pSrcImageio->top.fg_mode;
    pDst->top.ufdi_fmt = pSrcImageio->top.ufdi_fmt;
    pDst->top.vipi_fmt = pSrcImageio->top.vipi_fmt;
    pDst->top.img3o_fmt = pSrcImageio->top.img3o_fmt;
    pDst->top.imgi_en = pSrcImageio->top.imgi_en;
    pDst->top.ufdi_en = pSrcImageio->top.ufdi_en;
    pDst->top.unp_en = pSrcImageio->top.unp_en;
    pDst->top.ufd_en = pSrcImageio->top.ufd_en;
    pDst->top.ufd_crop_en = pSrcImageio->top.ufd_crop_en;
    pDst->top.c24_en = pSrcImageio->top.c24_en;
    pDst->top.cfa_en = pSrcImageio->top.cfa_en;
    pDst->top.vipi_en = pSrcImageio->top.vipi_en;
    pDst->top.vip2i_en = pSrcImageio->top.vip2i_en;
    pDst->top.vip3i_en = pSrcImageio->top.vip3i_en;
    pDst->top.mfb_en = pSrcImageio->top.mfb_en;
    pDst->top.mfbo_en = pSrcImageio->top.mfbo_en;
    pDst->top.g2c_en = pSrcImageio->top.g2c_en;
    pDst->top.c42_en = pSrcImageio->top.c42_en;
    pDst->top.srz1_en = pSrcImageio->top.srz1_en;
    pDst->top.mix1_en = pSrcImageio->top.mix1_en;
    pDst->top.srz2_en = pSrcImageio->top.srz2_en;
    pDst->top.mix2_en = pSrcImageio->top.mix2_en;
    pDst->top.mix3_en = pSrcImageio->top.mix3_en;
    pDst->top.crz_en = pSrcImageio->top.crz_en;
    pDst->top.img2o_en = pSrcImageio->top.img2o_en;
    pDst->top.fe_en = pSrcImageio->top.fe_en;
    pDst->top.feo_en = pSrcImageio->top.feo_en;
    pDst->top.c02_en = pSrcImageio->top.c02_en;
    pDst->top.crsp_en = pSrcImageio->top.crsp_en;
    pDst->top.img3o_en = pSrcImageio->top.img3o_en;
    pDst->top.img3bo_en = pSrcImageio->top.img3bo_en;
    pDst->top.img3co_en = pSrcImageio->top.img3co_en;
    pDst->top.c24b_en = pSrcImageio->top.c24b_en;
    pDst->top.mdp_crop_en = pSrcImageio->top.mdp_crop_en;
    pDst->top.interlace_mode = pSrcImageio->top.interlace_mode;
    pDst->top.ufd_sel = pSrcImageio->top.ufd_sel;
    pDst->top.ccl_sel = pSrcImageio->top.ccl_sel;
    pDst->top.ccl_sel_en = pSrcImageio->top.ccl_sel_en;
    pDst->top.g2g_sel = pSrcImageio->top.g2g_sel;
    pDst->top.g2g_sel_en = pSrcImageio->top.g2g_sel_en;
    pDst->top.c24_sel = pSrcImageio->top.c24_sel;
    pDst->top.srz1_sel = pSrcImageio->top.srz1_sel;
    pDst->top.mix1_sel = pSrcImageio->top.mix1_sel;
    pDst->top.crz_sel = pSrcImageio->top.crz_sel;
    pDst->top.nr3d_sel = pSrcImageio->top.nr3d_sel;
    pDst->top.fe_sel = pSrcImageio->top.fe_sel;
    pDst->top.mdp_sel = pSrcImageio->top.mdp_sel;
    pDst->top.pca_sel = pSrcImageio->top.pca_sel;
    pDst->top.imgi_v_flip_en = pSrcImageio->top.imgi_v_flip_en;
    pDst->top.lcei_v_flip_en = pSrcImageio->top.lcei_v_flip_en;
    pDst->top.ufdi_v_flip_en = pSrcImageio->top.ufdi_v_flip_en;
    /* update featureio parameters */
    pDst->top.sl2c_en = pSrcImageio->top.sl2c_en;
    pDst->top.seee_en = pSrcImageio->top.seee_en;
    pDst->top.lcei_en = pSrcImageio->top.lcei_en;
    pDst->top.lce_en = pSrcImageio->top.lce_en;
    pDst->top.sl2b_en = pSrcImageio->top.sl2b_en;
    pDst->top.nbc_en = pSrcImageio->top.nbc_en;
    pDst->top.sl2_en = pSrcImageio->top.sl2_en;
    pDst->top.bnr_en = pSrcImageio->top.bnr_en;
    pDst->top.lsci_en = pSrcImageio->top.lsci_en;
    pDst->top.lsc_en = pSrcImageio->top.lsc_en;
    pDst->top.nr3d_en = pSrcImageio->top.nr3d_en;

    /* config dma */
    #if 1
    pDst->imgi.imgi_stride = pSrcImageio->imgi.imgi_stride;
    //
    pDst->vipi.vipi_xsize = pSrcImageio->vipi.vipi_xsize;
    pDst->vipi.vipi_ysize = pSrcImageio->vipi.vipi_ysize;
    pDst->vipi.vipi_stride = pSrcImageio->vipi.vipi_stride;
    //
    pDst->vip2i.vip2i_xsize = pSrcImageio->vip2i.vip2i_xsize;
    pDst->vip2i.vip2i_ysize = pSrcImageio->vip2i.vip2i_ysize;
    pDst->vip2i.vip2i_stride = pSrcImageio->vip2i.vip2i_stride;
    //
    pDst->vip3i.vip3i_xsize = pSrcImageio->vip3i.vip3i_xsize;
    pDst->vip3i.vip3i_ysize = pSrcImageio->vip3i.vip3i_ysize;
    pDst->vip3i.vip3i_stride = pSrcImageio->vip3i.vip3i_stride;
    //
    pDst->mfb.bld_mode = pSrcImageio->mfb.bld_mode;
    pDst->mfb.bld_deblock_en = pSrcImageio->mfb.bld_deblock_en;
    pDst->mfb.bld_brz_en = pSrcImageio->mfb.bld_brz_en;
    //
    pDst->mfbo.mfbo_stride = pSrcImageio->mfbo.mfbo_stride;
    pDst->mfbo.mfbo_xoffset = pSrcImageio->mfbo.mfbo_xoffset;
    pDst->mfbo.mfbo_yoffset = pSrcImageio->mfbo.mfbo_yoffset;
    pDst->mfbo.mfbo_xsize = pSrcImageio->mfbo.mfbo_xsize;
    pDst->mfbo.mfbo_ysize = pSrcImageio->mfbo.mfbo_ysize;
    //
    pDst->cdrz.cdrz_input_crop_width = pSrcImageio->crz.cdrz_input_crop_width;
    pDst->cdrz.cdrz_input_crop_height = pSrcImageio->crz.cdrz_input_crop_height;
    pDst->cdrz.cdrz_output_width = pSrcImageio->crz.cdrz_output_width;
    pDst->cdrz.cdrz_output_height = pSrcImageio->crz.cdrz_output_height;
    pDst->cdrz.cdrz_luma_horizontal_integer_offset = pSrcImageio->crz.cdrz_luma_horizontal_integer_offset;
    pDst->cdrz.cdrz_luma_horizontal_subpixel_offset = pSrcImageio->crz.cdrz_luma_horizontal_subpixel_offset;
    pDst->cdrz.cdrz_luma_vertical_integer_offset = pSrcImageio->crz.cdrz_luma_vertical_integer_offset;
    pDst->cdrz.cdrz_luma_vertical_subpixel_offset = pSrcImageio->crz.cdrz_luma_vertical_subpixel_offset;
    pDst->cdrz.cdrz_horizontal_luma_algorithm = pSrcImageio->crz.cdrz_horizontal_luma_algorithm;
    pDst->cdrz.cdrz_vertical_luma_algorithm = pSrcImageio->crz.cdrz_vertical_luma_algorithm;
    pDst->cdrz.cdrz_horizontal_coeff_step = pSrcImageio->crz.cdrz_horizontal_coeff_step;
    pDst->cdrz.cdrz_vertical_coeff_step = pSrcImageio->crz.cdrz_vertical_coeff_step;
    //
    pDst->img2o.img2o_stride = pSrcImageio->img2o.img2o_stride;
    pDst->img2o.img2o_xoffset = pSrcImageio->img2o.img2o_xoffset;
    pDst->img2o.img2o_yoffset = pSrcImageio->img2o.img2o_yoffset;
    pDst->img2o.img2o_xsize = pSrcImageio->img2o.img2o_xsize;
    pDst->img2o.img2o_ysize = pSrcImageio->img2o.img2o_ysize;
    //
    pDst->srz1.srz_input_crop_width = pSrcImageio->srz1.srz_input_crop_width;
    pDst->srz1.srz_input_crop_height = pSrcImageio->srz1.srz_input_crop_height;
    pDst->srz1.srz_output_width = pSrcImageio->srz1.srz_output_width;
    pDst->srz1.srz_output_height = pSrcImageio->srz1.srz_output_height;
    pDst->srz1.srz_luma_horizontal_integer_offset = pSrcImageio->srz1.srz_luma_horizontal_integer_offset;
    pDst->srz1.srz_luma_horizontal_subpixel_offset = pSrcImageio->srz1.srz_luma_horizontal_subpixel_offset;
    pDst->srz1.srz_luma_vertical_integer_offset = pSrcImageio->srz1.srz_luma_vertical_integer_offset;
    pDst->srz1.srz_luma_vertical_subpixel_offset = pSrcImageio->srz1.srz_luma_vertical_subpixel_offset;
    pDst->srz1.srz_horizontal_coeff_step = pSrcImageio->srz1.srz_horizontal_coeff_step;
    pDst->srz1.srz_vertical_coeff_step = pSrcImageio->srz1.srz_vertical_coeff_step;
    //
    pDst->srz2.srz_input_crop_width = pSrcImageio->srz2.srz_input_crop_width;
    pDst->srz2.srz_input_crop_height = pSrcImageio->srz2.srz_input_crop_height;
    pDst->srz2.srz_output_width = pSrcImageio->srz2.srz_output_width;
    pDst->srz2.srz_output_height = pSrcImageio->srz2.srz_output_height;
    pDst->srz2.srz_luma_horizontal_integer_offset = pSrcImageio->srz2.srz_luma_horizontal_integer_offset;
    pDst->srz2.srz_luma_horizontal_subpixel_offset = pSrcImageio->srz2.srz_luma_horizontal_subpixel_offset;
    pDst->srz2.srz_luma_vertical_integer_offset = pSrcImageio->srz2.srz_luma_vertical_integer_offset;
    pDst->srz2.srz_luma_vertical_subpixel_offset = pSrcImageio->srz2.srz_luma_vertical_subpixel_offset;
    pDst->srz2.srz_horizontal_coeff_step = pSrcImageio->srz2.srz_horizontal_coeff_step;
    pDst->srz2.srz_vertical_coeff_step = pSrcImageio->srz2.srz_vertical_coeff_step;
    //
//    pDst->fe.fe_mode = pSrcImageio->fe.fe_mode;
    //
//    pDst->feo.feo_stride = pSrcImageio->feo.feo_stride;
    //
    pDst->nr3d.nr3d_on_en = pSrcImageio->nr3d.nr3d_on_en;
    pDst->nr3d.nr3d_on_xoffset = pSrcImageio->nr3d.nr3d_on_xoffset;
    pDst->nr3d.nr3d_on_yoffset = pSrcImageio->nr3d.nr3d_on_yoffset;
    pDst->nr3d.nr3d_on_width = pSrcImageio->nr3d.nr3d_on_width;
    pDst->nr3d.nr3d_on_height = pSrcImageio->nr3d.nr3d_on_height;
    //
    pDst->crsp.crsp_ystep = pSrcImageio->crsp.crsp_ystep;
    pDst->crsp.crsp_xoffset = pSrcImageio->crsp.crsp_xoffset;
    pDst->crsp.crsp_yoffset = pSrcImageio->crsp.crsp_yoffset;
    //
    pDst->img3o.img3o_stride = pSrcImageio->img3o.img3o_stride;
    pDst->img3o.img3o_xoffset = pSrcImageio->img3o.img3o_xoffset;
    pDst->img3o.img3o_yoffset = pSrcImageio->img3o.img3o_yoffset;
    pDst->img3o.img3o_xsize = pSrcImageio->img3o.img3o_xsize;
    pDst->img3o.img3o_ysize = pSrcImageio->img3o.img3o_ysize;
    //
    pDst->img3bo.img3bo_stride = pSrcImageio->img3bo.img3bo_stride;
    pDst->img3bo.img3bo_xsize = pSrcImageio->img3bo.img3bo_xsize;
    pDst->img3bo.img3bo_ysize = pSrcImageio->img3bo.img3bo_ysize;
    //
    pDst->img3co.img3co_stride = pSrcImageio->img3co.img3co_stride;
    pDst->img3co.img3co_xsize = pSrcImageio->img3co.img3co_xsize;
    pDst->img3co.img3co_ysize = pSrcImageio->img3co.img3co_ysize;
    //
    pDst->ufdi.ufdi_stride = pSrcImageio->ufdi.ufdi_stride;
    #else
    ::memcpy( (char*)&pDst->imgi, (char*)&pSrcImageio->imgi, sizeof(ISP_TPIPE_CONFIG_IMGI_STRUCT));
    ::memcpy( (char*)&pDst->vipi, (char*)&pSrcImageio->vipi, sizeof(ISP_TPIPE_CONFIG_VIPI_STRUCT));
    ::memcpy( (char*)&pDst->vip2i, (char*)&pSrcImageio->vip2i, sizeof(ISP_TPIPE_CONFIG_VIP2I_STRUCT));
    ::memcpy( (char*)&pDst->vip3i, (char*)&pSrcImageio->vip3i, sizeof(ISP_TPIPE_CONFIG_VIP3I_STRUCT));
    ::memcpy( (char*)&pDst->mfb, (char*)&pSrcImageio->mfb, sizeof(ISP_TPIPE_CONFIG_MFB_STRUCT));
    ::memcpy( (char*)&pDst->mfbo, (char*)&pSrcImageio->mfbo, sizeof(ISP_TPIPE_CONFIG_MFBO_STRUCT));
    ::memcpy( (char*)&pDst->cdrz, (char*)&pSrcImageio->crz, sizeof(ISP_TPIPE_CONFIG_CDRZ_STRUCT));
    ::memcpy( (char*)&pDst->img2o, (char*)&pSrcImageio->img2o, sizeof(ISP_TPIPE_CONFIG_IMG2O_STRUCT));
    ::memcpy( (char*)&pDst->srz1, (char*)&pSrcImageio->srz1, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    ::memcpy( (char*)&pDst->srz2, (char*)&pSrcImageio->srz2, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    ::memcpy( (char*)&pDst->fe, (char*)&pSrcImageio->fe, sizeof(ISP_TPIPE_CONFIG_FE_STRUCT));
    ::memcpy( (char*)&pDst->feo, (char*)&pSrcImageio->feo, sizeof(ISP_TPIPE_CONFIG_FEO_STRUCT));
    ::memcpy( (char*)&pDst->nr3d, (char*)&pSrcImageio->nr3d, sizeof(ISP_TPIPE_CONFIG_NR3D_STRUCT));
    ::memcpy( (char*)&pDst->crsp, (char*)&pSrcImageio->crsp, sizeof(ISP_TPIPE_CONFIG_CRSP_STRUCT));
    ::memcpy( (char*)&pDst->img3o, (char*)&pSrcImageio->img3o, sizeof(ISP_TPIPE_CONFIG_IMG3O_STRUCT));
    ::memcpy( (char*)&pDst->img3bo, (char*)&pSrcImageio->img3bo, sizeof(ISP_TPIPE_CONFIG_IMG3BO_STRUCT));
    ::memcpy( (char*)&pDst->img3co, (char*)&pSrcImageio->img3co, sizeof(ISP_TPIPE_CONFIG_IMG3CO_STRUCT));
    ::memcpy( (char*)&pDst->ufdi, (char*)&pSrcImageio->ufdi, sizeof(ISP_TPIPE_CONFIG_UFDI_STRUCT));
    #endif

    /* tuning from imageio */
    #if 1
    pDst->bnr.bpc_en = pSrcImageio->tuningFunc.bnr.bpc_en;
    pDst->bnr.bpc_tbl_en = pSrcImageio->tuningFunc.bnr.bpc_tbl_en;
    //
    pDst->lsc.sdblk_width = pSrcImageio->tuningFunc.lsc.sdblk_width;
    pDst->lsc.sdblk_xnum = pSrcImageio->tuningFunc.lsc.sdblk_xnum;
    pDst->lsc.sdblk_last_width = pSrcImageio->tuningFunc.lsc.sdblk_last_width;
    pDst->lsc.sdblk_height = pSrcImageio->tuningFunc.lsc.sdblk_height;
    pDst->lsc.sdblk_ynum = pSrcImageio->tuningFunc.lsc.sdblk_ynum;
    pDst->lsc.sdblk_last_height = pSrcImageio->tuningFunc.lsc.sdblk_last_height;
    //
    pDst->sl2.sl2_hrz_comp = pSrcImageio->tuningFunc.sl2.sl2_hrz_comp;
    //
    pDst->lce.lce_bc_mag_kubnx = pSrcImageio->tuningFunc.lce.lce_bc_mag_kubnx;
    pDst->lce.lce_slm_width = pSrcImageio->tuningFunc.lce.lce_slm_width;
    pDst->lce.lce_bc_mag_kubny = pSrcImageio->tuningFunc.lce.lce_bc_mag_kubny;
    pDst->lce.lce_slm_height = pSrcImageio->tuningFunc.lce.lce_slm_height;
    //
    pDst->lcei.lcei_stride = pSrcImageio->tuningFunc.lcei.lcei_stride;
    //
    pDst->nbc.anr_eny = pSrcImageio->tuningFunc.nbc.anr_eny;
    pDst->nbc.anr_enc = pSrcImageio->tuningFunc.nbc.anr_enc;
    pDst->nbc.anr_iir_mode = pSrcImageio->tuningFunc.nbc.anr_iir_mode;
    pDst->nbc.anr_scale_mode = pSrcImageio->tuningFunc.nbc.anr_scale_mode;
    //
    pDst->seee.se_edge = pSrcImageio->tuningFunc.seee.se_edge;
    pDst->seee.usm_over_shrink_en = pSrcImageio->tuningFunc.seee.usm_over_shrink_en;
    //
    pDst->cfa.bayer_bypass = pSrcImageio->tuningFunc.cfa.bayer_bypass;
    pDst->cfa.dm_fg_mode = pSrcImageio->tuningFunc.cfa.dm_fg_mode;
    //
    pDst->g2c.g2c_shade_en = pSrcImageio->tuningFunc.g2c.g2c_shade_en;
    //
    pDst->sl2b.sl2b_hrz_comp = pSrcImageio->tuningFunc.sl2b.sl2b_hrz_comp;
    //
    pDst->sl2c.sl2c_hrz_comp = pSrcImageio->tuningFunc.sl2c.sl2c_hrz_comp;
    #else
    ::memcpy( (char*)&pDst->bnr, (char*)&pSrcImageio->tuningFunc.bnr, sizeof(ISP_TPIPE_CONFIG_BNR_STRUCT));
    ::memcpy( (char*)&pDst->lsc, (char*)&pSrcImageio->tuningFunc.lsc, sizeof(ISP_TPIPE_CONFIG_LSC_STRUCT));
    ::memcpy( (char*)&pDst->sl2, (char*)&pSrcImageio->tuningFunc.sl2, sizeof(ISP_TPIPE_CONFIG_SL2_STRUCT));
    ::memcpy( (char*)&pDst->lce, (char*)&pSrcImageio->tuningFunc.lce, sizeof(ISP_TPIPE_CONFIG_LCE_STRUCT));
    ::memcpy( (char*)&pDst->lcei, (char*)&pSrcImageio->tuningFunc.lcei, sizeof(ISP_TPIPE_CONFIG_LCEI_STRUCT));
    ::memcpy( (char*)&pDst->nbc, (char*)&pSrcImageio->tuningFunc.nbc, sizeof(ISP_TPIPE_CONFIG_NBC_STRUCT));
    ::memcpy( (char*)&pDst->seee, (char*)&pSrcImageio->tuningFunc.seee, sizeof(ISP_TPIPE_CONFIG_SEEE_STRUCT));
    ::memcpy( (char*)&pDst->cfa, (char*)&pSrcImageio->tuningFunc.cfa, sizeof(ISP_TPIPE_CONFIG_CFA_STRUCT));
    ::memcpy( (char*)&pDst->g2c, (char*)&pSrcImageio->tuningFunc.g2c, sizeof(ISP_TPIPE_CONFIG_G2C_STRUCT));
    ::memcpy( (char*)&pDst->sl2b, (char*)&pSrcImageio->tuningFunc.sl2b, sizeof(ISP_TPIPE_CONFIG_SL2B_STRUCT));
    ::memcpy( (char*)&pDst->sl2c, (char*)&pSrcImageio->tuningFunc.sl2c, sizeof(ISP_TPIPE_CONFIG_SL2C_STRUCT));
    #endif

    /* update for tuning data extra */
    isApplyTuning = m_pPhyIspDrv->getP2TuningStatus((ISP_DRV_CQ_ENUM)this->CQ);
    if(isApplyTuning==MTRUE){
        stIspTuningTpipeFieldInf tuningField;
        MUINT32* pTuningBuf;

        if (this->isV3)
        {
          }
          else
          {
             if(m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum)==MFALSE)
              {
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            ret = MFALSE;
        }

        //
        getP2EnTuningTag(MTRUE,
                       (ISP_DRV_CQ_ENUM)this->CQ,
                       this->magicNum,
                       this->drvScenario,
                       this->sub_mode,
                       p2TuningTopEnTag,
                       p2TuningTopDmaTag); // get P2 tuning En & Dma Tag
        getP2EnTuningTag(MFALSE,
                       (ISP_DRV_CQ_ENUM)this->CQ,
                       this->magicNum,
                       this->drvScenario,
                       this->sub_mode,
                       p2TuningFieldEnTag,
                       p2TuningFieldDmaTag); // get P2 tuning En & Dma Tag

        //
        pTuningBuf = m_pPhyIspDrv->getTuningBuf(eTuningQueAccessPath_imageio_P2, this->CQ);
        p2TuningEn = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2, this->CQ, this->magicNum);
        p2TuningDma = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P2, eTuningTopEn_p2_dma, this->CQ, this->magicNum);
        if(m_pPhyIspDrv->getTuningTpipeFiled(p2Cq, pTuningBuf, tuningField)==MFALSE){
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            ret = MFALSE;
        }
        ISP_FUNC_DBG("[Tag]p2TuningTopEn(0x%08x),p2TuningTopDma(0x%08x),p2TuningFieldEn(0x%08x),p2TuningFieldDma(0x%08x)"
                ,p2TuningTopEnTag,p2TuningTopDmaTag,p2TuningFieldEnTag,p2TuningFieldDmaTag);
        //
        //set for p2_en top
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_UFD_EN ){
            pDst->top.ufd_en = ((p2TuningEn&CAM_CTL_EN_P2_UFD_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_SL2_EN ){
            pDst->top.sl2_en = ((p2TuningEn&CAM_CTL_EN_P2_SL2_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_CFA_EN ){
            pDst->top.cfa_en = ((p2TuningEn&CAM_CTL_EN_P2_CFA_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_MFB_EN ){
            pDst->top.mfb_en = ((p2TuningEn&CAM_CTL_EN_P2_MFB_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_C24_EN ){
            pDst->top.c24_en = ((p2TuningEn&CAM_CTL_EN_P2_C24_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_G2C_EN ){
            pDst->top.g2c_en = ((p2TuningEn&CAM_CTL_EN_P2_G2C_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_C42_EN ){
            pDst->top.c42_en = ((p2TuningEn&CAM_CTL_EN_P2_C42_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_NBC_EN ){
            pDst->top.nbc_en = ((p2TuningEn&CAM_CTL_EN_P2_NBC_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_PCA_EN ){
            pDst->top.pca_en = ((p2TuningEn&CAM_CTL_EN_P2_PCA_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_SEEE_EN){
            pDst->top.seee_en = ((p2TuningEn&CAM_CTL_EN_P2_SEEE_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_LCE_EN ){
            pDst->top.lce_en = ((p2TuningEn&CAM_CTL_EN_P2_LCE_EN)?(1):(0));

            // workaround for lce and lcei sync
            // lcei must be enable if lce be enable
//            if((pDst->top.lce_en)&&((pDst->top.lcei_en)==0)  ) {
//                ISP_FUNC_WRN("lce & lcei not be sync, lce_en(0x%x),lcei_en(0x%x),disable lce",pDst->top.lce_en,pDst->top.lcei_en);
//                pDst->top.lce_en = 0;
//            }

        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_NR3D_EN){
            pDst->top.nr3d_en = ((p2TuningEn&CAM_CTL_EN_P2_NR3D_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_SL2B_EN){
            pDst->top.sl2b_en = ((p2TuningEn&CAM_CTL_EN_P2_SL2B_EN)?(1):(0));
        }
        if(p2TuningTopEnTag & CAM_CTL_EN_P2_SL2C_EN){
            pDst->top.sl2c_en = ((p2TuningEn&CAM_CTL_EN_P2_SL2C_EN)?(1):(0));
        }
        //set for p2_dma top
        if(p2TuningTopDmaTag & CAM_CTL_EN_P2_DMA_LCEI_EN ){
            pDst->top.lcei_en = ((p2TuningDma&CAM_CTL_EN_P2_DMA_LCEI_EN)?(1):(0));
        }
        //set for p2_en field
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_SL2_EN ){
            #if 1
            pDst->sl2.sl2_hrz_comp = tuningField.sl2.sl2_hrz_comp;
            #else
            ::memcpy( (char*)&pDst->sl2, (char*)&tuningField.sl2, sizeof(ISP_TPIPE_CONFIG_SL2_STRUCT));
            #endif
        }
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_CFA_EN ){
            // dm_fg_mode be set from path imageio
            pDst->cfa.bayer_bypass = tuningField.cfa.bayer_bypass;
        }
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_MFB_EN ){
            //::memcpy( (char*)&pDst->mfb, (char*)&tuningField.mfb, sizeof(ISP_TPIPE_CONFIG_MFB_STRUCT));
            //bld_mode would updated by imagio insteading of featureio
            pDst->mfb.bld_deblock_en=tuningField.mfb.bld_deblock_en;
            pDst->mfb.bld_brz_en=tuningField.mfb.bld_brz_en;
            ISP_FUNC_DBG("[MFB Tdr] bld_deblock_en/bld_brz_en TDR(0x%x/0x%x), TUN(0x%x/0x%x)",\
                pDst->mfb.bld_deblock_en,pDst->mfb.bld_brz_en,tuningField.mfb.bld_deblock_en,tuningField.mfb.bld_brz_en);
        }
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_G2C_EN ){
            #if 1
            pDst->g2c.g2c_shade_en = tuningField.g2c.g2c_shade_en;
            #else
            ::memcpy( (char*)&pDst->g2c, (char*)&tuningField.g2c, sizeof(ISP_TPIPE_CONFIG_G2C_STRUCT));
            #endif
        }
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_NBC_EN ){
            #if 1
            pDst->nbc.anr_eny = tuningField.nbc.anr_eny;
            pDst->nbc.anr_enc = tuningField.nbc.anr_enc;
            pDst->nbc.anr_iir_mode = tuningField.nbc.anr_iir_mode;
            pDst->nbc.anr_scale_mode = tuningField.nbc.anr_scale_mode;
            #else
            ::memcpy( (char*)&pDst->nbc, (char*)&tuningField.nbc, sizeof(ISP_TPIPE_CONFIG_NBC_STRUCT));
            #endif
        }
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_SEEE_EN){
            #if 0
            //ISP_FUNC_INF("kk:12345");
            ::memcpy( (char*)&pDst->seee, (char*)&tuningField.seee, sizeof(ISP_TPIPE_CONFIG_SEEE_STRUCT));
            //tmpSrc = tuningField.seee.se_edge;
            //tmpDst = pDst->seee.se_edge;

            //if(pDst->seee.se_edge != tuningField.seee.se_edge)
            //    ISP_FUNC_INF("kk:12345");
            #else
            pDst->seee.se_edge = tuningField.seee.se_edge;
            pDst->seee.usm_over_shrink_en = tuningField.seee.usm_over_shrink_en;
            #endif
        }
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_LCE_EN ){
            //update CAM_LCE_ZR(0x49C4)
            //CAM_LCE_ZR(0x49C4) be updated forcedly by path imageio
            //LCE_BCMK_X=floor(((LCE_SLM_WD-1)*32768) / (LCE_IMAGE_WD-1))
            //LCE_BCMK_Y=floor(((LCE_SLM_HT-1)*32768) / (LCE_IMAGE_HT-1))
            pDst->lce.lce_slm_width = tuningField.lce.lce_slm_width;
            pDst->lce.lce_slm_height = tuningField.lce.lce_slm_height;

            if((pDst->lce.lce_slm_width-1>0) && (pDst->lce.lce_slm_height-1>0)){
                pDst->lce.lce_bc_mag_kubnx = ((pDst->lce.lce_slm_width-1)<<15)/(this->lceInputW-1);
                pDst->lce.lce_bc_mag_kubny = ((pDst->lce.lce_slm_height-1)<<15)/(this->lceInputH-1);
            }else if(pDst->top.lce_en){
                ISP_FUNC_ERR("[Error]size error lce_slm_width(%d),lce_slm_height(%d)",pDst->lce.lce_slm_width,pDst->lce.lce_slm_height);
            }

            ISP_FUNC_DBG("lce_slm_width(%d),lce_slm_height(%d),lce_bc_mag_kubnx(%d),lce_bc_mag_kubny(%d)",
                pDst->lce.lce_slm_width, pDst->lce.lce_slm_height, pDst->lce.lce_bc_mag_kubnx, pDst->lce.lce_bc_mag_kubny);

        }
        #if 0 // set from path imageio
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_NR3D_EN){
            ::memcpy( (char*)&pDst->nr3d, (char*)&tuningField.nr3d, sizeof(ISP_TPIPE_CONFIG_NR3D_STRUCT));
        }
        #endif
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_SL2B_EN){
            pDst->sl2b.sl2b_hrz_comp = tuningField.sl2.sl2_hrz_comp;
            //::memcpy( (char*)&pDst->sl2b, (char*)&tuningField.sl2b, sizeof(ISP_TPIPE_CONFIG_SL2B_STRUCT));
        }
        if(p2TuningFieldEnTag & CAM_CTL_EN_P2_SL2C_EN){
            pDst->sl2c.sl2c_hrz_comp = tuningField.sl2.sl2_hrz_comp;
            //::memcpy( (char*)&pDst->sl2c, (char*)&tuningField.sl2c, sizeof(ISP_TPIPE_CONFIG_SL2C_STRUCT));
        }
        //set for p2_dma field
        if(p2TuningFieldDmaTag & CAM_CTL_EN_P2_DMA_LCEI_EN ){
            #if 1
            pDst->lcei.lcei_stride = tuningField.lcei.lcei_stride;
            #else
            ::memcpy( (char*)&pDst->lcei, (char*)&tuningField.lcei, sizeof(ISP_TPIPE_CONFIG_LCEI_STRUCT));
            #endif
        }
        //
        // set for scenario ip
        if(this->drvScenario == NSImageio::NSIspio::eDrvScenario_IP)
        {
            getP1EnTuningTag(MTRUE,
                           (ISP_DRV_CQ_ENUM)this->CQ,
                           this->magicNum,
                           this->drvScenario,
                           this->sub_mode,
                           p1TuningTopEnTag,
                           p1TuningTopDmaTag); // get P1 tuning En & Dma Tag
            getP1EnTuningTag(MFALSE,
                           (ISP_DRV_CQ_ENUM)this->CQ,
                           this->magicNum,
                           this->drvScenario,
                           this->sub_mode,
                           p1TuningFieldEnTag,
                           p1TuningFieldDmaTag); // get P1 tuning En & Dma Tag
            //
            //set for p1_en top
            if(p1TuningTopEnTag & CAM_CTL_EN_P1_BNR_EN ){
                pDst->top.bnr_en = ((p2TuningEn&CAM_CTL_EN_P1_BNR_EN)?(1):(0));
            }
            if(p1TuningTopEnTag & CAM_CTL_EN_P1_LSC_EN ){
                pDst->top.lsc_en = ((p2TuningEn&CAM_CTL_EN_P1_LSC_EN)?(1):(0));
            }
            //set for p1_dma top
            if(p1TuningTopDmaTag & CAM_CTL_EN_P1_DMA_LSCI_EN ){
                pDst->top.lsci_en = ((p1TuningDma&CAM_CTL_EN_P1_DMA_LSCI_EN)?(1):(0));
            }
            //
            //set for p1_en
            if(p1TuningFieldEnTag & CAM_CTL_EN_P1_BNR_EN ){
                #if 1
                pDst->bnr.bpc_en = tuningField.bnr.bpc_en;
                pDst->bnr.bpc_tbl_en = tuningField.bnr.bpc_tbl_en;
                #else
                ::memcpy( (char*)&pDst->bnr, (char*)&tuningField.bnr, sizeof(ISP_TPIPE_CONFIG_BNR_STRUCT));
                #endif
            }
            if(p1TuningFieldEnTag & CAM_CTL_EN_P1_LSC_EN ){
                #if 1
                pDst->lsc.sdblk_width = tuningField.lsc.sdblk_width;
                pDst->lsc.sdblk_xnum = tuningField.lsc.sdblk_xnum;
                pDst->lsc.sdblk_last_width = tuningField.lsc.sdblk_last_width;
                pDst->lsc.sdblk_height = tuningField.lsc.sdblk_height;
                pDst->lsc.sdblk_ynum = tuningField.lsc.sdblk_ynum;
                pDst->lsc.sdblk_last_height = tuningField.lsc.sdblk_last_height;
                #else
                ::memcpy( (char*)&pDst->lsc, (char*)&tuningField.lsc, sizeof(ISP_TPIPE_CONFIG_LSC_STRUCT));
                #endif
            }
            //set for p1_dma
            if(p1TuningFieldDmaTag & CAM_CTL_EN_P1_DMA_LSCI_EN ){
                #if 1
                pDst->lsci.lsci_stride = tuningField.lsci.lsci_stride;
                #else
                ::memcpy( (char*)&pDst->lsci, (char*)&tuningField.lsci, sizeof(ISP_TPIPE_CONFIG_LSCI_STRUCT));
                #endif
            }
        }
      }
    }
        //
        if (! this->isV3)
        {
        if(m_pPhyIspDrv->enTuningQueByCq(eTuningQueAccessPath_imageio_P2,(ISP_DRV_CQ_ENUM)this->CQ,this->magicNum,eIspTuningMgrFunc_Null)==MFALSE){
            ISP_FUNC_ERR("[Error]enTuningQue fail");
            ret = MFALSE;
        }
        }

        //ISP_FUNC_INF("kk:S(%d),D(%d),p2TuningFieldEnTag(0x%x),tmpSrc(%d),tmpDst(%d)-0x%x-0x%x"
        //    ,tuningField.seee.se_edge,pDst->seee.se_edge,p2TuningFieldEnTag,tmpSrc,tmpDst,&pDst->seee,&pDst->seee.se_edge);


    /* software tpipe setting */
#if 1  //kk test default:1
    if(function_DbgLogEnable_DEBUG==true)
        pDst->drvinfo.debugRegDump = 1;
    else
        pDst->drvinfo.debugRegDump = 0;
#else
    pDst->drvinfo.debugRegDump = 1;
#endif

    pDst->sw.log_en = 1;


    pDst->sw.src_width = pSrcImageio->tdriCfg.srcWidth;
    pDst->sw.src_height = pSrcImageio->tdriCfg.srcHeight;
    pDst->sw.tpipe_irq_mode = pSrcImageio->tdriPerformCfg.irqMode;
    pDst->sw.tpipe_width = pSrcImageio->tdriPerformCfg.tpipeWidth;
    pDst->sw.tpipe_height = pSrcImageio->tdriPerformCfg.tpipeHeight;

    // get extra information
    pDst->drvinfo.DesCqPa = (unsigned long)(this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(this->CQ,this->burstQueIdx,this->dupCqIdx));
    pDst->drvinfo.DesCqVa = (unsigned int*)(this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufVirAddr(this->CQ,this->burstQueIdx,this->dupCqIdx));
    pDst->drvinfo.VirCqVa = (unsigned int*)(this->m_pIspDrvShell->getPhyIspDrv()->getCQVirBufVirAddr(this->CQ,this->burstQueIdx,this->dupCqIdx));
    pDst->drvinfo.dupCqIdx = this->dupCqIdx;
    pDst->drvinfo.p2Cq = (ISP_TPIPE_P2_CQ_ENUM)p2Cq;

    ISP_FUNC_DBG("VirCqPa(0x%x),dupCqIdx(%d)",pDst->drvinfo.DesCqPa,pDst->drvinfo.dupCqIdx);



    return ret;
}

#if 0
MBOOL CAM_TDRI_PIPE::runTpipeDbgLog( void )
{
    MBOOL tdri_result = MTRUE;
    //
    ISP_FUNC_INF("enTdri(%d)",this->enTdri);
    //

    #if 0
    if ( this->enTdri ) {
        tdri_result = this->m_pIspDrvShell->m_pTpipeDrv->runDbgTpipeMain();
        if (MFALSE == tdri_result) {
            ISP_FUNC_ERR("[Error]runTpipeDbgLog");
            return tdri_result;
        }
    }
    #endif

    return tdri_result;
}
#endif

int CAM_TDRI_PIPE::_config( void )
{
    MBOOL tdri_result = MTRUE;
    MUINT32 p2Cq;
    //
    ISP_FUNC_DBG("+,enTdri(%d),tcm_en(0x%08x),tdr_en(0x%08x),cq(%d),dupCqIdx(%d),burstQueIdx(%d)",this->enTdri,this->tcm_en,this->tdr_en,this->CQ,this->dupCqIdx,this->burstQueIdx);
    //
    ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_TILE, this->tcm_en, ISP_DRV_USER_ISPF);    //0x407c
    ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_TCM_EN, this->tdr_en, ISP_DRV_USER_ISPF);  //0x4084
    if ( this->enTdri ) {
        configTdriSetting(&tdri);
    }

    ISP_FUNC_DBG("-,");

    return 0;
}
int CAM_TDRI_PIPE::_write2CQ( void )
{
    ISP_FUNC_DBG("+,cq(%d)",this->CQ);

    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_CTL_TCM_EN);
    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(CAM_MODULE_ENUM)CAM_CTL_TDR_EN);

    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    ISP_PASS1_CTRL
/////////////////////////////////////////////////////////////////////////////*/
/* input
why???
0x15006530  CAM_LSC_D_CTL1
               FIELD LSC_D_PRC_MODE            : 1;
               FIELD LSC_SPARE2                : 1;
0x15006538  CAM_LSC_D_CTL3
               FIELD LSC_SPARE                 : 15;
0x150067A4  CAM_RRZ_D_IN_IMG


0x15004004  CAM_CTL_EN_P1               R  CAM_TOP_CTL_EN_P1
0x15004010  CAM_CTL_EN_P1_D             RW
0x15004040  CAM_CTL_SEL_GLOBAL          R
0x15004418  CAM_TG_SEN_GRAB_PXL         R
0x1500441C  CAM_TG_SEN_GRAB_LIN         R
0x15004500  CAM_OBC_OFFST0              R  CAM_ISP_OBC
0x15004504  CAM_OBC_OFFST1              R
0x15004508  CAM_OBC_OFFST2              R
0x1500450C  CAM_OBC_OFFST3              R
0x15004510  CAM_OBC_GAIN0               R
0x15004514  CAM_OBC_GAIN1               R
0x15004518  CAM_OBC_GAIN2               R
0x1500451C  CAM_OBC_GAIN3               R
0x15004530  CAM_LSC_CTL1                R  CAM_ISP_LSC
0x15004534  CAM_LSC_CTL2                RW
0x15004538  CAM_LSC_CTL3                R
0x1500453C  CAM_LSC_LBLOCK              RW
0x15004540  CAM_LSC_RATIO               R
0x15004544  CAM_LSC_TPIPE_OFST          RW
0x15004548  CAM_LSC_TPIPE_SIZE          RW
0x1500454C  CAM_LSC_GAIN_TH             R
0x15004550  CAM_RPG_SATU_1              R  CAM_ISP_RPG
0x15004554  CAM_RPG_SATU_2              R
0x15004558  CAM_RPG_GAIN_1              R
0x1500455C  CAM_RPG_GAIN_2              R
0x15004560  CAM_RPG_OFST_1              R
0x15004564  CAM_RPG_OFST_2              R
0x150047A0  CAM_RRZ_CTL                 R  CAM_ISP_RRZ
0x150047A4  CAM_RRZ_IN_IMG              RW
0x150047A8  CAM_RRZ_OUT_IMG             RW
0x150047AC  CAM_RRZ_HORI_STEP           RW
0x150047B0  CAM_RRZ_VERT_STEP           RW
0x150047BC  CAM_RRZ_VERT_INT_OFST       R
0x150047C0  CAM_RRZ_VERT_SUB_OFST       R
0x150047C4  CAM_RRZ_MODE_TH             R
0x150047C8  CAM_RRZ_MODE_CTL            R
0x15004800  CAM_BPC_CON                 R  CAM_ISP_BNR_BPC
0x15004804  CAM_BPC_TH1                 R
0x15004808  CAM_BPC_TH2                 R
0x1500480C  CAM_BPC_TH3                 R
0x15004810  CAM_BPC_TH4                 R
0x15004814  CAM_BPC_DTC                 R
0x15004818  CAM_BPC_COR                 R
0x15004824  CAM_BPC_TH1_C               R
0x15004828  CAM_BPC_TH2_C               R
0x1500482C  CAM_BPC_TH3_C               R
0x15004830  CAM_BPC_RMM1                R  CAM_ISP_BNR_RMM
0x15004834  CAM_BPC_RMM2                R
0x15004838  CAM_BPC_RMM_REVG_1          R
0x1500483C  CAM_BPC_RMM_REVG_2          R
0x15004840  CAM_BPC_RMM_LEOS            R
0x15004844  CAM_BPC_RMM_GCNT            R
0x15004850  CAM_NR1_CON                 R  CAM_ISP_BNR_NR1
0x15004854  CAM_NR1_CT_CON              R
0x15004E08  CAM_DMX_VSIZE               R  CAM_ISP_DMX
0x15006530  CAM_LSC_D_CTL1              RW CAM_ISP_LSC_D
0x15006538  CAM_LSC_D_CTL3              RW
0x15007278  CAM_LSCI_YSIZE              R  CAM_DMA_LSCI
0x1500727C  CAM_LSCI_STRIDE             R
0x15007280  CAM_LSCI_CON                R
0x15007284  CAM_LSCI_CON2               R

*/
/* output
0x15004010  CAM_CTL_EN_P1_D     RW CAM_TOP_CTL_EN_P1
0x15004534  CAM_LSC_CTL2    RW CAM_ISP_LSC
0x1500453C  CAM_LSC_LBLOCK  RW
0x15004544  CAM_LSC_TPIPE_OFST  RW
0x15004548  CAM_LSC_TPIPE_SIZE  RW
0x150047A4  CAM_RRZ_IN_IMG  RW CAM_ISP_RRZ
0x150047A8  CAM_RRZ_OUT_IMG     RW
0x150047AC  CAM_RRZ_HORI_STEP   RW
0x150047B0  CAM_RRZ_VERT_STEP   RW
0x150047B4  CAM_RRZ_HORI_INT_OFST   W
0x150047B8  CAM_RRZ_HORI_SUB_OFST   W
0x1500481C  CAM_BPC_TBLI1   W  CAM_ISP_BNR_BPC
0x15004820  CAM_BPC_TBLI2   W
0x15004E00  CAM_DMX_CTL     W  CAM_ISP_DMX
0x15004E04  CAM_DMX_CROP    W
0x15004E10  CAM_BMX_CTL     W  CAM_ISP_BMX
0x15004E14  CAM_BMX_CROP    W
0x15004E18  CAM_BMX_VSIZE   W
0x15004E20  CAM_RMX_CTL     W  CAM_ISP_RMX
0x15004E24  CAM_RMX_CROP    W
0x15004E28  CAM_RMX_VSIZE   W
0x15006500  CAM_OBC_D_OFFST0    W  CAM_ISP_OBC_D
0x15006504  CAM_OBC_D_OFFST1    W
0x15006508  CAM_OBC_D_OFFST2    W
0x1500650C  CAM_OBC_D_OFFST3    W
0x15006510  CAM_OBC_D_GAIN0     W
0x15006514  CAM_OBC_D_GAIN1     W
0x15006518  CAM_OBC_D_GAIN2     W
0x1500651C  CAM_OBC_D_GAIN3     W
0x15006530  CAM_LSC_D_CTL1  RW CAM_ISP_LSC_D
0x15006534  CAM_LSC_D_CTL2  W
0x15006538  CAM_LSC_D_CTL3  RW
0x1500653C  CAM_LSC_D_LBLOCK    W
0x15006540  CAM_LSC_D_RATIO     W
0x15006544  CAM_LSC_D_TPIPE_OFST    W
0x15006548  CAM_LSC_D_TPIPE_SIZE    W
0x1500654C  CAM_LSC_D_GAIN_TH   W
0x15006550  CAM_RPG_D_SATU_1    W  CAM_ISP_RPG_D
0x15006554  CAM_RPG_D_SATU_2    W
0x15006558  CAM_RPG_D_GAIN_1    W
0x1500655C  CAM_RPG_D_GAIN_2    W
0x15006560  CAM_RPG_D_OFST_1    W
0x15006564  CAM_RPG_D_OFST_2    W
0x150067A0  CAM_RRZ_D_CTL   W  CAM_ISP_RRZ_D
0x150067A4  CAM_RRZ_D_IN_IMG      W
0x150067A8  CAM_RRZ_D_OUT_IMG   W
0x150067AC  CAM_RRZ_D_HORI_STEP     W
0x150067B0  CAM_RRZ_D_VERT_STEP     W
0x150067B4  CAM_RRZ_D_HORI_INT_OFST     W
0x150067B8  CAM_RRZ_D_HORI_SUB_OFST     W
0x150067BC  CAM_RRZ_D_VERT_INT_OFST     W
0x150067C0  CAM_RRZ_D_VERT_SUB_OFST     W
0x150067C4  CAM_RRZ_D_MODE_TH   W
0x150067C8  CAM_RRZ_D_MODE_CTL  W
0x15006800  CAM_BPC_D_CON   W  CAM_ISP_BNR_BPC_D
0x15006804  CAM_BPC_D_TH1   W
0x15006808  CAM_BPC_D_TH2   W
0x1500680C  CAM_BPC_D_TH3   W
0x15006810  CAM_BPC_D_TH4   W
0x15006814  CAM_BPC_D_DTC   W
0x15006818  CAM_BPC_D_COR   W
0x1500681C  CAM_BPC_D_TBLI1     W
0x15006820  CAM_BPC_D_TBLI2     W
0x15006824  CAM_BPC_D_TH1_C     W
0x15006828  CAM_BPC_D_TH2_C     W
0x1500682C  CAM_BPC_D_TH3_C     W
0x15006830  CAM_BPC_D_RMM1  W
0x15006834  CAM_BPC_D_RMM2  W
0x15006838  CAM_BPC_D_RMM_REVG_1    W
0x1500683C  CAM_BPC_D_RMM_REVG_2    W
0x15006840  CAM_BPC_D_RMM_LEOS  W
0x15006844  CAM_BPC_D_RMM_GCNT  W
0x15006850  CAM_NR1_D_CON   W
0x15006854  CAM_NR1_D_CT_CON    W
0x15006E00  CAM_DMX_D_CTL   W  CAM_ISP_DMX_D
0x15006E04  CAM_DMX_D_CROP  W
0x15006E08  CAM_DMX_D_VSIZE     W
0x15006E10  CAM_BMX_D_CTL   W  CAM_ISP_BMX_D
0x15006E14  CAM_BMX_D_CROP  W
0x15006E18  CAM_BMX_D_VSIZE     W
0x15006E20  CAM_RMX_D_CTL   W  CAM_ISP_RMX_D
0x15006E24  CAM_RMX_D_CROP  W
0x15006E28  CAM_RMX_D_VSIZE     W
0x15007270  CAM_LSCI_OFST_ADDR  W  CAM_DMA_LSCI
0x150074BC  CAM_LSCI_D_OFST_ADDR    W  CAM_DMA_LSCI_D
0x150074C4  CAM_LSCI_D_YSIZE    W
0x150074C8  CAM_LSCI_D_STRIDE   W
0x150074CC  CAM_LSCI_D_CON  W
0x150074D0  CAM_LSCI_D_CON2     W

*/

int ISP_PASS1_CTRL::_config( void )
{
    ISP_FUNC_DBG("+");
#if 0
    IspRrzCfg rrz_cfg;

    if ( 0 != setP1RrzCfg( rrz_cfg ) ) {
        ISP_FUNC_ERR("ERROR setP1RrzCfg");
    }
#endif
    ISP_FUNC_DBG("-");
    return 0;
}

int ISP_PASS1_CTRL::_write2CQ(void)
{
    ISP_FUNC_DBG("+");
#if 0
    IspDrv* pIspDrv_cqx = (IspDrv*)this->m_pIspDrvShell->m_pPhyIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)this->CQ);

    //set CQ
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_TOP_CTL_EN_P1);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_LSC);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_RRZ);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BNR_BPC);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BNR_RMM);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BNR_NR1);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_DMX);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BMX);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_RMX);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_DMA_LSCI);

    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_OBC_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_LSC_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_RPG_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_RRZ_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BNR_BPC_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BNR_RMM_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BNR_NR1_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_DMX_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_BMX_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_ISP_RMX_D);
    pIspDrv_cqx->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,CAM_DMA_LSCI_D);
#endif

    ISP_FUNC_DBG("-");
    return 0;
}

MBOOL ISP_PASS1_CTRL::runTwinDbgLog( void )
{
    MBOOL twin_result = MTRUE;
    //
    //if ( this->enTwin ) {
        twin_result = this->m_pIspDrvShell->m_pTwinDrv->runDbgTwinMain();
        if (MFALSE == twin_result) {
            ISP_FUNC_ERR("[Error]runTwinDbgLog");
            return twin_result;
        }
    //}
    return twin_result;
}

extern void dual_copy_reg_to_input(ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, const isp_reg_t *ptr_isp_reg);
extern void dual_copy_output_to_reg(isp_reg_t *ptr_isp_reg, const ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);

#define TWIN_GET_IN_CFG() ((ISP_DUAL_IN_CONFIG_STRUCT*)this->m_pIspDrvShell->m_pTwinDrv->getInCfg())
#define TWIN_GET_OUT_CFG() ((ISP_DUAL_OUT_CONFIG_STRUCT*)this->m_pIspDrvShell->m_pTwinDrv->getOutCfg())


void  ISP_PASS1_CTRL::resetP1MagicNum(MUINT32 dma){
    switch (dma)
    {
       case _imgo_:
          {
           Mutex::Autolock lock(m_p1MagicNum_IMGO_LLock);
           m_p1MagicNum_IMGO_list.resize(0);
          }
           break;
       case _rrzo_:
          {
           Mutex::Autolock lock(m_p1MagicNum_RRZO_LLock);
           m_p1MagicNum_RRZO_list.resize(0);
          }
           break;

       case _imgo_d_:
           {
           Mutex::Autolock lock(m_p1MagicNum_IMGOD_LLock);
           m_p1MagicNum_IMGOD_list.resize(0);
           }
           break;

       case _rrzo_d_:
           {
           Mutex::Autolock lock(m_p1MagicNum_RRZOD_LLock);
           m_p1MagicNum_RRZOD_list.resize(0);
           }
           break;

    }

}

int ISP_PASS1_CTRL::setP1MagicNum( MUINT32 pass1_path, MUINT32 MagicNum, MUINT32 magicNumCnt)
{
    MUINT32 cq = ISP_DRV_CQ0;
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQIdx =  0;

    if (( ISP_PASS1 != pass1_path ) && ( ISP_PASS1_D != pass1_path )) {
        ISP_FUNC_DBG("Invalid path:(%d)", pass1_path);
        return -1;
    }

    if (magicNumCnt < 1){
        ISP_FUNC_ERR("Invalid magic number ");
        return -1;
    }

    if ( ISP_PASS1 == pass1_path ) {
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
#else
        _targetDupQIdx = this->dupCqIdx;
#endif

        if(this->burstQueIdx > 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("pass1 support no burstQNum");
        }

        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR("_targetVirDrv == NULL, _targetDupQIdx = 0x%x\n",_targetDupQIdx);
            return -1;
        }

        ISP_WRITE_REG(_targetVirDrv, CAM_RAW_MAGIC_NUM0, MagicNum, ISP_DRV_USER_ISPF);
        //ISP_FUNC_DBG("MagicNum [0x%x/0x%x]", pMagicNum[0], pMagicNum[1]);
    }
    else if ( ISP_PASS1_D == pass1_path ) {
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN) == 1)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
#else
        _targetDupQIdx = this->dupCqIdx;
#endif
        if(this->burstQueIdx > 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("pass1_D support no burstQNum");
        }

        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR("_targetVirDrv_D == NULL, _targetDupQIdx_D = 0x%x\n",_targetDupQIdx);
            return -1;
        }

        ISP_WRITE_REG(_targetVirDrv, CAM_RAW_D_MAGIC_NUM0, MagicNum, ISP_DRV_USER_ISPF);

        //ISP_FUNC_INF("[3ASync]D setP1MagicNum:MNum [0x%x/0x%x]", pMagicNum[0], pMagicNum[1]);
    }

    return 0;
}

int ISP_PASS1_CTRL::keepP1RrzCfg( IspRrzCfg const rrz_cfg )
{
    if ( ISP_PASS1 == rrz_cfg.pass1_path ) {
        this->m_RrzCfg = rrz_cfg;
    }
    else if ( ISP_PASS1_D == rrz_cfg.pass1_path ) {
        this->m_RrzDCfg = rrz_cfg;
    }
    else {
        ISP_FUNC_ERR("rrz keeper path err 0x%x",rrz_cfg.pass1_path);
        return -1;
    }
    return 0;
}

int ISP_PASS1_CTRL::setP1RrzCfg( IspRrzCfg const rrz_cfg )
{
    MBOOL twin_result = MTRUE;
    MUINT32 rrz_ctl = 0;
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQIdx = 0;

    ISP_FUNC_DBG("ISP_PASS1_CTRL::setP1RrzCfg E:path(%d),rrz_in_w/h(%d/%d),rrz_crop_x/y/fx/fy/w/h(%d/%d/%d/%d/%d/%d),rrz_out_w/h(%d/%d), sd_lw/xn(%d/%d),m_num(0x%x)", \
        rrz_cfg.pass1_path, \
        rrz_cfg.rrz_in_size.w, \
        rrz_cfg.rrz_in_size.h, \
        rrz_cfg.rrz_in_roi.x, \
        rrz_cfg.rrz_in_roi.y, \
        rrz_cfg.rrz_in_roi.floatX, \
        rrz_cfg.rrz_in_roi.floatY, \
        rrz_cfg.rrz_in_roi.w, \
        rrz_cfg.rrz_in_roi.h, \
        rrz_cfg.rrz_out_size.w, \
        rrz_cfg.rrz_out_size.h, \
        rrz_cfg.sd_lwidth, \
        rrz_cfg.sd_xnum, \
        rrz_cfg.m_num_0);

    ISP_FUNC_DBG("twin(%d),path(%d)",rrz_cfg.isTwinMode,rrz_cfg.pass1_path);

    if ( ISP_PASS1 == rrz_cfg.pass1_path ) {
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
#else
        _targetDupQIdx = this->dupCqIdx;
#endif
        if(this->burstQueIdx > 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("pass1 support no burstQNum");
        }

        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR("_targetVirDrv == NULL, _targetDupQIdx = 0x%x\n",_targetDupQIdx);
            return -1;
        }
    }
    else if ( ISP_PASS1_D == rrz_cfg.pass1_path ) {
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN) == 1)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
#else
        _targetDupQIdx = this->dupCqIdx;
#endif
        if(this->burstQueIdx > 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("pass1_D support no burstQNum");
        }

        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ_D,this->burstQueIdx,_targetDupQIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR("_targetVirDrv_D == NULL, _targetDupQIdx_D = 0x%x\n",_targetDupQIdx);
            return -1;
        }
    }
    else {
        ISP_FUNC_ERR("Not support pass1 path(%d)",rrz_cfg.pass1_path);
    }

    //
    if ( rrz_cfg.isTwinMode) {
        if((rrz_cfg.rrz_in_roi.x*2 + rrz_cfg.rrz_in_roi.w) > rrz_cfg.rrz_in_size.w){
            ISP_FUNC_DBG("rrz input crop out of range: (x,y,floatx,floatxy,w,h = %d, %d, %d, %d, %d, %d),(size: %d, %d)",\
                rrz_cfg.rrz_in_roi.x,rrz_cfg.rrz_in_roi.y,rrz_cfg.rrz_in_roi.floatX,rrz_cfg.rrz_in_roi.floatY,rrz_cfg.rrz_in_roi.w,rrz_cfg.rrz_in_roi.h,\
                rrz_cfg.rrz_in_size.w,rrz_cfg.rrz_in_size.h);
        }
        else if((rrz_cfg.rrz_in_roi.x*2 + rrz_cfg.rrz_in_roi.w) < rrz_cfg.rrz_in_size.w){
            ISP_FUNC_DBG("rrz view-angle issue: (x,y,floatx,floatxy,w,h = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x),(size:0x%x,0x%x)",\
                rrz_cfg.rrz_in_roi.x,rrz_cfg.rrz_in_roi.y,rrz_cfg.rrz_in_roi.floatX,rrz_cfg.rrz_in_roi.floatY,rrz_cfg.rrz_in_roi.w,rrz_cfg.rrz_in_roi.h,\
                rrz_cfg.rrz_in_size.w,rrz_cfg.rrz_in_size.h);
        }
        //m_twinCfg.rrz_cfg = rrz_cfg;
        TWIN_GET_IN_CFG()->DEBUG.DUAL_LOG_EN = 1;
        TWIN_GET_IN_CFG()->DEBUG.DUAL_LOG_ID = 1;
        TWIN_GET_IN_CFG()->SW.TWIN_MODE_SDBLK_XNUM_ALL = rrz_cfg.sd_xnum;
        TWIN_GET_IN_CFG()->SW.TWIN_MODE_SDBLK_lWIDTH_ALL = rrz_cfg.sd_lwidth;
        TWIN_GET_IN_CFG()->SW.TWIN_RRZ_IN_CROP_HT = rrz_cfg.rrz_in_roi.h; /* must be larger than one */
        TWIN_GET_IN_CFG()->SW.TWIN_RRZ_HORI_INT_OFST = rrz_cfg.rrz_in_roi.x;
        //TWIN_GET_IN_CFG()->SW.TWIN_RRZ_HORI_INT_OFST_LAST = rrz_cfg.rrz_in_roi.x; //Support central crop only
        TWIN_GET_IN_CFG()->SW.TWIN_RRZ_HORI_INT_OFST_LAST = rrz_cfg.rrz_in_size.w - rrz_cfg.rrz_in_roi.x -rrz_cfg.rrz_in_roi.w; //Support central crop only
        TWIN_GET_IN_CFG()->SW.TWIN_RRZ_HORI_SUB_OFST = rrz_cfg.rrz_in_roi.floatX;
        TWIN_GET_IN_CFG()->SW.TWIN_RRZ_OUT_WD = rrz_cfg.rrz_out_size.w;
        TWIN_GET_IN_CFG()->SW.TWIN_LSC_CROP_OFFX = rrz_cfg.twin_lsc_crop_offx;/* by pixel, default zero */
        TWIN_GET_IN_CFG()->SW.TWIN_LSC_CROP_OFFY = rrz_cfg.twin_lsc_crop_offy;/* by line, default zero */


        //update v_ratio by twin driver
        ((isp_reg_t *)_targetVirDrv->getRegAddr())->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = 0;
        ((isp_reg_t *)_targetVirDrv->getRegAddr())->CAM_RRZ_VERT_STEP.Raw = 0;
        //debug in param
        //ISP_FUNC_DBG("debug in CAM_RRZ_OUT_IMG (0x%x)",((isp_reg_t *)pIspDrv_cqx->getRegAddr())->CAM_RRZ_OUT_IMG.Raw);
        dual_copy_reg_to_input((ISP_DUAL_IN_CONFIG_STRUCT*)TWIN_GET_IN_CFG(), (const isp_reg_t *)_targetVirDrv->getRegAddr());
        twin_result = this->m_pIspDrvShell->m_pTwinDrv->configTwinPara(NULL);
        if (MFALSE == twin_result) {
            ISP_FUNC_ERR("[Error]configTwinPara Fail");
            return -1;
        }

        dual_copy_output_to_reg((isp_reg_t*)_targetVirDrv->getRegAddr(), (const ISP_DUAL_OUT_CONFIG_STRUCT*)TWIN_GET_OUT_CFG());
        //ISP_WRITE_BITS(m_pP1IspDrv, CAM_CTL_EN_P1_D, RMG_D_EN, ISP_READ_BITS_NOPROTECT(m_pP1IspDrv,CAM_CTL_EN_P1,RMG_EN),ISP_DRV_USER_ISPF);
        //ISP_WRITE_REG(m_pP1IspDrv,CAM_CTL_IHDR_D,ISP_READ_REG_NOPROTECT(m_pP1IspDrv, CAM_CTL_IHDR),ISP_DRV_USER_ISPF);
        //debug out param
        //ISP_FUNC_DBG("debug CAM_RRZ_OUT_IMG out(0x%x)",((isp_reg_t *)pIspDrv_cqx->getRegAddr())->CAM_RRZ_OUT_IMG.Raw);

    }
    else {
        // this part has been applied in ISP_RAW_PIPE::_config
        // which is called in CamPathPass1::setP1RrzCfg
        #if 0
        IspSize rrz_in_size = (IspSize)rrz_cfg.rrz_in_size;
        //single mode
        ISP_FUNC_INF("single mode,path(%d)",this->path);
        MUINT32 rrz_h_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(rrz_cfg.rrz_in_size.w,rrz_cfg.rrz_in_roi.w,rrz_cfg.rrz_in_roi.x,rrz_cfg.rrz_in_roi.floatX,rrz_cfg.rrz_out_size.w);
        MUINT32 rrz_v_step = this->m_pIspDrvShell->m_pTwinDrv->calCoefStep(rrz_cfg.rrz_in_size.h,rrz_cfg.rrz_in_roi.h,rrz_cfg.rrz_in_roi.y,rrz_cfg.rrz_in_roi.floatY,rrz_cfg.rrz_out_size.h);;

        rrz_ctl = (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(rrz_cfg.rrz_in_roi.w,rrz_cfg.rrz_out_size.w)<<16) | (this->m_pIspDrvShell->m_pTwinDrv->getCoefTbl(rrz_cfg.rrz_in_roi.h,rrz_cfg.rrz_out_size.h)<<21);

        if ( ISP_PASS1 == this->path ) {
            //this->m_pIspDrvShell->cam_isp_rrz_cfg((IspSize)rrz_cfg.rrz_in_size,(IspRect)rrz_cfg.rrz_in_roi,(IspSize)rrz_cfg.rrz_out_size,rrz_ctl,rrz_h_step,rrz_v_step,0);
        }
        else if ( ISP_PASS1_D == this->path ) {
            //this->m_pIspDrvShell->cam_isp_rrz_cfg((IspSize)rrz_cfg.rrz_in_size,(IspRect)rrz_cfg.rrz_in_roi,(IspSize)rrz_cfg.rrz_out_size,rrz_ctl,rrz_h_step,rrz_v_step,1);
        }
        #endif
    }

    ISP_FUNC_DBG("ISP_PASS1_CTRL::_config:End ");

    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
//
#define isp_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define isp_container_of(ptr, type, member) ({ \
     const typeof( ((type *)0)->member ) *__mptr = (ptr); \
     (type *)( (char *)__mptr - isp_offsetof(type,member) );})
int
ISP_BUF_CTRL::
init( MUINT32 dmaChannel )
{
    ISP_FUNC_DBG("ISP_BUF_CTRL: E");
    //avoid to reset working path, can't reset all dma one at a time, e.g.:PIP preview/capture/vr ...etc
    if(this->path == ISP_PASS1){
        memset (&m_p1DeqedBufHeaderL[_imgo_], 0, sizeof(stISP_BUF_HEADER_LIST));
        memset (&m_p1DeqedBufHeaderL[_rrzo_], 0, sizeof(stISP_BUF_HEADER_LIST));
        m_hwbufL[_imgo_].bufInfoList.clear();
        m_hwbufL[_rrzo_].bufInfoList.clear();

        pPass1Ctrl->resetP1MagicNum(_imgo_);
        pPass1Ctrl->resetP1MagicNum(_rrzo_);
    }
    else if(this->path == ISP_PASS1_CAMSV){
        memset (&m_p1DeqedBufHeaderL[_camsv_imgo_], 0, sizeof(stISP_BUF_HEADER_LIST));

        m_hwbufL[_camsv_imgo_].bufInfoList.clear();
    }
    else if(this->path_D == ISP_PASS1_D){
        memset (&m_p1DeqedBufHeaderL[_imgo_d_], 0, sizeof(stISP_BUF_HEADER_LIST));
        memset (&m_p1DeqedBufHeaderL[_rrzo_d_], 0, sizeof(stISP_BUF_HEADER_LIST));
        m_hwbufL[_imgo_d_].bufInfoList.clear();
        m_hwbufL[_rrzo_d_].bufInfoList.clear();

        pPass1Ctrl->resetP1MagicNum(_imgo_d_);
        pPass1Ctrl->resetP1MagicNum(_rrzo_d_);
    }
    else if(this->path_D == ISP_PASS1_CAMSV_D){
        memset (&m_p1DeqedBufHeaderL[_camsv2_imgo_], 0, sizeof(stISP_BUF_HEADER_LIST));

        m_hwbufL[_camsv2_imgo_].bufInfoList.clear();
    }
    else{
        MINT32 rt_dma = getDmaBufIdx(dmaChannel);
        ISP_FUNC_DBG("0x%08x/%dn",dmaChannel,rt_dma);
        if ( -1 == rt_dma ) {
            ISP_FUNC_ERR("dma channel error ");
            return -1;
        }

        ISP_DRV_P2_CQ_ENUM p2Cq;
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
        m_p2HwBufL[p2Cq][this->dupCqIdx][rt_dma].bufInfoList.clear();
    }
    //
    ISP_FUNC_DBG("X");
    return 0;
}
//
MBOOL
ISP_BUF_CTRL::
waitBufReady( MUINT32 dmaChannel )
{
    MBOOL ret = MTRUE;
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 bWaitBufRdy;
    MUINT32 loopCnt = 0;
    #define _CAM_INT_WAIT_TIMEOUT_MS (1000)
    #define MAX_CNT (CAM_INT_WAIT_TIMEOUT_MS/_CAM_INT_WAIT_TIMEOUT_MS)

    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error ");
        return MFALSE;
    }
    //
    if ( _imgo_ == rt_dma || _imgo_d_ == rt_dma || _rrzo_ == rt_dma || _rrzo_d_ == rt_dma || _camsv_imgo_ == rt_dma || _camsv2_imgo_ == rt_dma) {
        //
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_IS_RDY;
        buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
        buf_ctrl.data_ptr = 0;
        buf_ctrl.pExtend = (unsigned char*)&bWaitBufRdy;
        ISP_FUNC_INF("waitBufReady[%d]:[%x,%x] rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)",rt_dma,this->path, this->path_D,buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
        //
        ret = this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
        //
        if ( MFALSE == ret ) {
            ISP_FUNC_ERR("rtBufCtrl fail:ISP_RT_BUF_CTRL_IS_RDY");
            return ret;
        }
        //
        if (bWaitBufRdy) {
            //ISP_FUNC_INF("waitBufReady[%d]\n",rt_dma);
            //NO BUFFER FILLED
            //wait pass1 done
            ISP_DRV_WAIT_IRQ_STRUCT irq_TG_DONE;
            irq_TG_DONE.Clear= ISP_DRV_IRQ_CLEAR_WAIT;
            irq_TG_DONE.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
            irq_TG_DONE.Status=CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
            irq_TG_DONE.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
            irq_TG_DONE.Timeout=_CAM_INT_WAIT_TIMEOUT_MS;
            irq_TG_DONE.UserName="pass1";
            irq_TG_DONE.irq_TStamp=0x0;
            irq_TG_DONE.bDumpReg=0XFE;
            irq_TG_DONE.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
            irq_TG_DONE.UserInfo.Status=CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
            irq_TG_DONE.UserInfo.UserKey=0;
            //
            if (( _imgo_d_ == rt_dma ) || (_rrzo_d_ == rt_dma)) {
                irq_TG_DONE.Status = CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
                irq_TG_DONE.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
                irq_TG_DONE.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
                irq_TG_DONE.UserInfo.Status=CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
                irq_TG_DONE.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
                irq_TG_DONE.UserName = "pass1_d";
            }
            else if( _camsv_imgo_ == rt_dma ) {
                irq_TG_DONE.Status = CAMSV_CAMSV_INT_STATUS_PASS1_DON_ST;
                irq_TG_DONE.Type = ISP_DRV_IRQ_TYPE_INT_CAMSV;
                irq_TG_DONE.UserInfo.Status=CAMSV_CAMSV_INT_STATUS_PASS1_DON_ST;
                irq_TG_DONE.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_CAMSV;
                irq_TG_DONE.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
                irq_TG_DONE.UserName = "camsv";
            }
            else if( _camsv2_imgo_ == rt_dma ) {
                irq_TG_DONE.Status = CAMSV_CAMSV2_INT_STATUS_PASS1_DON_ST;
                irq_TG_DONE.Type = ISP_DRV_IRQ_TYPE_INT_CAMSV2;
                irq_TG_DONE.UserInfo.Status=CAMSV_CAMSV2_INT_STATUS_PASS1_DON_ST;
                irq_TG_DONE.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_CAMSV2;
                irq_TG_DONE.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
                irq_TG_DONE.UserName = "camsv_d";
            }
            //
            while(loopCnt++ < MAX_CNT){//(this->m_pPhyIspDrv->registerIrq(irq_TG_DONE)){
                if(loopCnt == MAX_CNT)
                    irq_TG_DONE.bDumpReg = 0x1;
                buf_ctrl.ctrl = ISP_RT_BUF_CTRL_IS_RDY;
                this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
                if(bWaitBufRdy == MFALSE){
                    ret = MTRUE;
                    break;
                }
                ret = this->m_pPhyIspDrv->waitIrq( &irq_TG_DONE );
                if ( MFALSE == ret ) {
                    if (irq_TG_DONE.Status == CAM_CTL_INT_P1_STATUS_PASS1_DON_ST) {
                         ISP_FUNC_INF("still no p1 done,dma[%d]",rt_dma);
                    } else  if (irq_TG_DONE.Status == CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST) {
                        ISP_FUNC_INF("still no p1_d done,dma[%d]",rt_dma);
                    }
                    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CUR_STATUS;
                    buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
                    buf_ctrl.data_ptr = 0;
                    buf_ctrl.pExtend = (unsigned char*)&bWaitBufRdy;//useless
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
                }
                else
                    break;//wait successfully
                irq_TG_DONE.Clear = ISP_DRV_IRQ_CLEAR_NONE;// waitirq chg to use non-clr wait after 1st wait
            }
            if(ret != MTRUE)
            {
                MUINT32 _ret2 = MTRUE;
                ISP_DRV_WAIT_IRQ_STRUCT irq_TG_SOF;
                                            irq_TG_SOF.Clear= ISP_DRV_IRQ_CLEAR_WAIT;
                                            irq_TG_SOF.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
                                            irq_TG_SOF.Status=CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;
                                            irq_TG_SOF.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
                                            irq_TG_SOF.Timeout=300;
                                            irq_TG_SOF.UserName="pass1";
                                            irq_TG_SOF.irq_TStamp=0x0;
                                            irq_TG_SOF.bDumpReg=0x0;
                                            irq_TG_SOF.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT_P1_ST;
                                            irq_TG_SOF.UserInfo.Status=CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;
                                            irq_TG_SOF.UserInfo.UserKey=0;
                if (( _imgo_d_ == rt_dma ) || (_rrzo_d_ == rt_dma)) {
                    irq_TG_SOF.Status = CAM_CTL_INT_P1_STATUS_D_SOF1_INT_ST;
                    irq_TG_SOF.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
                    irq_TG_SOF.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
                    irq_TG_SOF.UserName = "pass1_d";
                }
                else if( _camsv_imgo_ == rt_dma ) {
                    irq_TG_SOF.Status = CAMSV_CAMSV_INT_STATUS_TG_SOF1_INT_ST;
                    irq_TG_SOF.Type = ISP_DRV_IRQ_TYPE_INT_CAMSV;
                    irq_TG_SOF.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
                    irq_TG_SOF.UserName = "camsv";
                }
                else if( _camsv2_imgo_ == rt_dma ) {
                    irq_TG_SOF.Status = CAMSV_CAMSV2_INT_STATUS_TG_SOF1_INT_ST;
                    irq_TG_SOF.Type = ISP_DRV_IRQ_TYPE_INT_CAMSV2;
                    irq_TG_SOF.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
                    irq_TG_SOF.UserName = "camsv_d";
                }
                irq_TG_SOF.bDumpReg = 0x1;
                _ret2 = this->m_pPhyIspDrv->waitIrq( &irq_TG_SOF );
                if(_ret2 == MFALSE){
                    MUINT32 _flag[6] = {0};
                    _flag[5] =1;
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(_flag,NULL);
                    _flag[5] = 0;
                    _flag[4] = 1;
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(_flag,NULL);
                    ISP_FUNC_ERR("wait p1 done fail,cause isp no response, no SOF\n");
                    if(_flag[4] != 0)
                        ISP_FUNC_ERR("no p1 done caused by dma port error\n");

                    ISP_FUNC_ERR("wait p1 done fail,cause isp no response, no SOF\n");
                }
                else{
                    MUINT32 _flag[6] = {0};
                    _flag[5] =1;
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(_flag,NULL);
                    _flag[5] = 0;
                    _flag[4] = 1;
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(_flag,NULL);
                    if(_flag[4] != 0)
                        ISP_FUNC_ERR("no p1 done caused by dma port error\n");
                    else
                        ISP_FUNC_ERR("wait p1 done fail,user space enque record(cur dma:%d imgo cnt:%d_%d,rrzo cnt:%d_%d)\n",rt_dma,P1DeQCnt[_imgo_],P1DeQCnt[_imgo_d_],P1DeQCnt[_rrzo_],P1DeQCnt[_rrzo_d_]);
                }
            }
            //
         }
    }
    //
    return ret;
}

//
/*
description: move filled current buffer to empty buffer list
*/
#define _DMA_TRANS(rt_dma,path){\
    switch(rt_dma){\
        case _imgo_:\
        case _rrzo_:\
            path = 1;\
            break;\
        case _imgo_d_:\
        case _rrzo_d_:\
            path = 3;\
            break;\
        case _camsv_imgo_:\
            path = 4;\
            break;\
        case _camsv2_imgo_:\
            path = 5;\
            break;\
        default:\
            path = 2;\
            break;\
    }\
}

#define ISP_QUE_INF(rt_dma,fmt,arg...){\
    UINT32 path;\
    _DMA_TRANS(rt_dma,path);\
    if(path == 1){\
        ISP_FUNC_INF("pass1:" fmt,##arg);\
    }\
    else if(path == 3){\
        ISP_FUNC_INF("pass1d:" fmt,##arg);\
    }\
    else if(path == 4){\
        ISP_FUNC_INF("camsv:" fmt,##arg);\
    }\
    else if(path == 5){\
        ISP_FUNC_INF("camsv2:" fmt,##arg);\
    }\
    else {\
        ISP_FUNC_INF("pass2:" fmt,##arg);\
    }\
}

#define ISP_QUE_DBG(rt_dma,fmt,arg...){\
    UINT32 path;\
    _DMA_TRANS(rt_dma,path);\
    if(path == 1){\
        ISP_FUNC_DBG("pass1:" fmt,##arg);\
    }\
    else {\
        ISP_FUNC_DBG("pass2:" fmt,##arg);\
    }\
}

MINT32
ISP_BUF_CTRL::
enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo,MINT32 enqueCq,MINT32 dupCqIdx,MBOOL bImdMode)
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 end = 0;
    IMEM_BUF_INFO buf_info;
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    ISP_RT_BUF_INFO_STRUCT  rt_buf_info;
    ISP_RT_BUF_INFO_STRUCT  ex_rt_buf_info;
    MUINT32 size;
    MUINT32 fbc[2];
    MUINT32 vf=0;
	fbc[0] = 0;
	fbc[1] = 0;

    //
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error(0x%x) ",dmaChannel);
        return -1;
    }
    //
    buf_info.size = bufInfo.size;
    buf_info.memID = bufInfo.memID;
    buf_info.virtAddr = bufInfo.base_vAddr;
    buf_info.phyAddr = bufInfo.base_pAddr;
    buf_info.bufSecu = bufInfo.bufSecu;
    buf_info.bufCohe = bufInfo.bufCohe;
    //
    //
    if ( ISP_PASS1   == this->path || \
         ISP_PASS1_D == this->path_D || \
         ISP_PASS1_CAMSV   == this->path || \
         ISP_PASS1_CAMSV_D == this->path_D) {
        //check is full
        //ISP_FUNC_INF("[js_test]:sizeof(ISP_RT_BUF_STRUCT)(%d) ",sizeof(ISP_RT_BUF_STRUCT));
        //
        if( NULL != bufInfo.next ){
            ISP_QUE_INF(rt_dma,"dma(%d),id(%d),size(0x%x),VA(0x%x),PA(0x%x),S/C(%d/%d),bufidx(%d),replace:new(0x%x),bufidx(%d),bImdMode(%d)",\
                        rt_dma,\
                        buf_info.memID,\
                        buf_info.size,\
                        buf_info.virtAddr,\
                        buf_info.phyAddr,\
                        buf_info.bufSecu,\
                        buf_info.bufCohe,\
                        bufInfo.mBufIdx,\
                        bufInfo.next->base_pAddr,\
                        bufInfo.next->mBufIdx,\
                        bImdMode);
        }
        else{
            ISP_QUE_INF(rt_dma,"dma(%d),id(%d),size(0x%x),VA(0x%x),PA(0x%x),S/C(%d/%d),bufidx(%d),bImdMode(%d)",\
                        rt_dma,\
                        buf_info.memID,\
                        buf_info.size,\
                        buf_info.virtAddr,\
                        buf_info.phyAddr,\
                        buf_info.bufSecu,\
                        buf_info.bufCohe,\
                        bufInfo.mBufIdx,\
                        bImdMode);
        }
        if ( _imgo_ == rt_dma || \
             _rrzo_ == rt_dma || \
             _imgo_d_ == rt_dma || \
             _rrzo_d_ == rt_dma || \
             _camsv_imgo_ == rt_dma || \
             _camsv2_imgo_ == rt_dma) {
            if( (_imgo_ == rt_dma) || (_rrzo_ == rt_dma) ){
                fbc[0] = readReg(0x000040f0);
                fbc[1] = readReg(0x000040f4);
                vf = readReg(0x00004414);
            }
            else if( (_imgo_d_ == rt_dma)|| (_rrzo_d_ == rt_dma)){
                fbc[0] = readReg(0x000040f8);
                fbc[1] = readReg(0x000040fc);
                vf = readReg(0x00006414);
            }
            else if (_camsv_imgo_ == rt_dma) { //( (_camsv_imgo_ == rt_dma) || (_camsv2_imgo_))
                fbc[0] = readReg(0x0000901C);
                fbc[1] = 0x0;
                vf = readReg(0x00009414);
            }
            else if (_camsv2_imgo_ == rt_dma){ //( (_camsv_imgo_ == rt_dma) || (_camsv2_imgo_))
                fbc[0] = 0x0;
                fbc[1] = readReg(0x0000981C);
                vf = readReg(0x00009C14);
            }

            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.data_ptr = 0;
            buf_ctrl.pExtend = (unsigned char*)&size;
            //ISP_FUNC_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            //
            if ( size == ISP_RT_CQ0C_BUF_SIZE ) {
                ISP_FUNC_ERR("real time buffer number FULL:rt_dma(%d)/size(%d)",rt_dma,size);
                return -1;
            }
        }
        //
        //bufInfo.base_pAddr =  (MUINT32)buf_info.phyAddr;
        bufInfo.status = ISP_BUF_EMPTY;
        //
        //marked. Avoid memory leak!
        //m_hwbufL[rt_dma].bufInfoList.push_back(bufInfo);
        //
        //put to ring buffer in kernel
        rt_buf_info.memID = buf_info.memID;
        rt_buf_info.size = buf_info.size;
        rt_buf_info.base_vAddr = (long long)buf_info.virtAddr;
        rt_buf_info.base_pAddr = buf_info.phyAddr;
        rt_buf_info.bufIdx     = bufInfo.mBufIdx;
        //ISP_QUE_INF(rt_dma,"rt_buf_info.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",rt_buf_info.memID,rt_buf_info.size,rt_buf_info.base_vAddr,rt_buf_info.base_pAddr);
        if ( 0 == rt_buf_info.base_pAddr ) {
            ISP_FUNC_ERR("NULL PA");
        }
        //
        if(bImdMode == MTRUE){
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_ENQUE_IMD;
        }
        else
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_ENQUE;

        buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
        buf_ctrl.data_ptr = &rt_buf_info;
        buf_ctrl.ex_data_ptr = (MUINT32)0;
        buf_ctrl.pExtend = 0;
        //ISP_QUE_INF(rt_dma,"[rtbc][ENQUQ]+(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
        //enque exchanged buffer
        if ( NULL != bufInfo.next ) {
            ex_rt_buf_info.memID = bufInfo.next->memID;
            ex_rt_buf_info.size = bufInfo.next->size;
            ex_rt_buf_info.base_vAddr = (long long)bufInfo.next->base_vAddr;
            ex_rt_buf_info.base_pAddr = bufInfo.next->base_pAddr;
            ex_rt_buf_info.bufIdx     = bufInfo.next->mBufIdx;//for replace buffer

            //
            //ISP_QUE_INF(rt_dma,"exchange 1st buf. by 2nd buf. and enque it.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",ex_rt_buf_info.memID,ex_rt_buf_info.size,ex_rt_buf_info.base_vAddr,ex_rt_buf_info.base_pAddr);
            //
            if ( 0 == ex_rt_buf_info.base_pAddr ) {
                ISP_FUNC_ERR("NULL Ex-PA");
            }
            //
            //buf_ctrl.ctrl = ISP_RT_BUF_CTRL_EXCHANGE_ENQUE;
            buf_ctrl.ex_data_ptr = &ex_rt_buf_info;
            //
        }
        //
        if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
            ISP_FUNC_ERR("ERROR:rtBufCtrl");
            return -1;
        }
        P1DeQCnt[rt_dma] += 1;
        if( (_imgo_ == rt_dma) || (_rrzo_ == rt_dma) ){
            ISP_QUE_DBG(rt_dma,"imgo/rrzo, vf=0x%x fbc: 0x%x/0x%x_0x%x/0x%x\n",vf,fbc[0],fbc[1],readReg(0x000040f0),readReg(0x000040f4));
        }
        else if( (_imgo_d_ == rt_dma)|| (_rrzo_d_ == rt_dma)){
            ISP_QUE_DBG(rt_dma,"imgo_d/rrzo_d,vf=0x%x fbc: 0x%x/0x%x_0x%x/0x%x\n",vf,fbc[0],fbc[1],readReg(0x000040f8),readReg(0x000040fc));
        }
        else {//( (_camsv_imgo_ == rt_dma) || (_camsv2_imgo_))
            ISP_QUE_DBG(rt_dma,"camsv/camsv2,vf=0x%x fbc: 0x%x/0x%x_0x%x/0x%x\n",vf,fbc[0],fbc[1],readReg(0x0000901C),readReg(0x0000981C));
        }

    }
    else {
        ISP_DRV_P2_CQ_ENUM p2Cq;
        Mutex::Autolock lock(this->m_pIspDrvShell->gPass2Lock);
        //
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)enqueCq, p2Cq);
        #if 0
        if ( ISP_RT_BUF_SIZE <= m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.size() ) {
            ISP_FUNC_ERR("Max buffer size(0x%x),p2Cq(%d),dupCqIdx(%d),rt_dma(%d)",
                    m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.size(),p2Cq,dupCqIdx,rt_dma);
            return -1;
        }
        #endif
        //
        #if 0 //user should make sure PA if using hw unless the va is not from ion(malloc va, ap buffer)
        //do this in iopipe
        if(buf_info.memID==-1)
        {
        if ( this->m_pIspDrvShell->m_pIMemDrv->mapPhyAddr(&buf_info) ) {
            ISP_FUNC_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
            return -1;
        }
        }
        #endif
        //
        bufInfo.memID = (MUINT32)buf_info.memID;
        bufInfo.size =  (MUINT32)buf_info.size;
        bufInfo.base_vAddr =  buf_info.virtAddr;
        bufInfo.base_pAddr =  buf_info.phyAddr;
        bufInfo.status = ISP_BUF_EMPTY;
        //
        m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.push_back(bufInfo);
        //
        ISP_FUNC_DBG("enqueCq(%d),p2Cq(%d),dupCqIdx(%d),dma(%d),memID(0x%x),size(%d),vAddr(0x%p),pAddr(0x%x)", \
                                    enqueCq,p2Cq,dupCqIdx,rt_dma, \
                                    m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.back().memID, \
                                    m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.size(), \
                                    m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.back().base_vAddr,
                                    m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.back().base_pAddr);
        //
    }
    //
    return 0;
}


#define ENABLE_ATTACH_BUF_INFO 1

MINT32
ISP_BUF_CTRL::
enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data ,MBOOL bImdMode)
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    stISP_BUF_HEADER_LIST *plist = NULL;
    MUINT32 i = ISP_BUF_INFO_NODE_MAX_CNT;

    if (_rt_dma_max_ <= rt_dma) {
       ISP_FUNC_ERR("enqueueHwBuf:: Invalid rt_dma=%d", rt_dma);
       return -1;
    }

    returnDeqBufInfo(rt_dma,&bufInfo);
    if (0){
        //only for dbg
        if ((ISP_PASS1 == this->path)||(ISP_PASS1_D == this->path_D)) {
            this->ispDrvSwitch2Virtual(this->CQ,this->CQ_D,this->burstQueIdx,0);
        }

        //ISP_FUNC_DBG("[MyDeQ]EnQHwBuf::ispDrv=0x%x/0x%x, CmdQ-IN/OUT Z=[0x%x,0x%x], ZD=[0x%x,0x%x], /// HW Z=[0x%x,0x%x]. ZD=[0x%x,0x%x]", \
        ISP_QUE_INF(rt_dma,"[MyDeQ]EnQHwBuf::ispDrv=0x%x/0x%x, CmdQ-IN/OUT Z=[0x%x,0x%x], HW Z=[0x%x,0x%x].", \
           m_pP1IspDrv, m_pP1IspDrvD, \
           ISP_READ_REG_NOPROTECT(m_pP1IspDrv, CAM_RRZ_IN_IMG),\
           ISP_READ_REG_NOPROTECT(m_pP1IspDrv, CAM_RRZ_OUT_IMG), \
           ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_RRZ_IN_IMG),\
           ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_RRZ_OUT_IMG));

           /*ISP_READ_REG_NOPROTECT(m_pP1IspDrvD, CAM_RRZ_D_IN_IMG), \
           ISP_READ_REG_NOPROTECT(m_pP1IspDrvD, CAM_RRZ_D_OUT_IMG),\
           ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_RRZ_D_IN_IMG),\
           ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_RRZ_D_OUT_IMG)*/
    }
    if(bImdMode == MTRUE)
        return  enqueueHwBuf( dmaChannel, bufInfo ,0,0,bImdMode);
    else
        return  enqueueHwBuf( dmaChannel, bufInfo);
}


//
MINT32
ISP_BUF_CTRL::
dequeueMdpFrameEnd( MINT32 drvScenario )
{
    if(drvScenario != NSImageio::NSIspio::eDrvScenario_IP){
        ISP_DRV_P2_CQ_ENUM p2Cq;

        this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->dequeCq, p2Cq);
        if(MDPMGR_NO_ERROR != g_pMdpMgr->dequeueFrameEnd(gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx])){
            ISP_FUNC_ERR("g_pMdpMgr->dequeueMdpFrameEnd for dispo fail");
            return -1;
        }
    }

    return 0;
}



MINT32
ISP_BUF_CTRL::returnDeqBufInfo( MINT32 rt_dma, stISP_BUF_INFO* bufInfo)
{
    stISP_BUF_HEADER_LIST *plist = NULL;
    MUINT32 i;
    MBOOL bVF_en= MFALSE;
    if(this->path == ISP_PASS1)
        bVF_en = ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN);
    else if(this->path_D == ISP_PASS1_D)
        bVF_en = ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG2_VF_CON,VFDATA_EN);

    plist = &m_p1DeqedBufHeaderL[rt_dma];
    Mutex::Autolock lock(plist->mLLock);
#if (ENABLE_ATTACH_BUF_INFO)

    if ( NULL == bufInfo->next ){
        //This enQ is not an ex-change buffer request
        for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i++) {
             if (bufInfo->base_pAddr == plist->mN[i].mInfo.base_pAddr)
                break;
        }

        if (i < ISP_BUF_INFO_NODE_MAX_CNT) {
            ISP_QUE_DBG(rt_dma,"[MyDeQ]returnDeqBufInfo:: node=0x%8x", plist->mN[i].mInfo.base_pAddr);
            plist->mN[i].mOccupied = MFALSE;
            plist->mN[i].mInfo.base_pAddr = 0x0;
        }
        else
        {
            if(bVF_en){
                ISP_FUNC_ERR("dma:0x%x: find no match baseaddr, 0x%x\n",rt_dma,bufInfo->base_pAddr);
                ISP_FUNC_ERR("current available base addr\n");
                for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i++){
                    if(plist->mN[i].mOccupied)
                        ISP_FUNC_ERR("0x%x",plist->mN[i].mInfo.base_pAddr);
                }
            }
        }
    }
    else {
        //This is an ex-change buffer behavior. Release the BufInfoNode corresponding to the new buffer
        //(that is the NEXT buffer in the struct),
        //
        ISP_QUE_DBG(rt_dma,"enqueueHwBuf::exchange Buff");

        for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i++) {
             if (bufInfo->next->base_vAddr == plist->mN[i].mInfo.base_vAddr)
                break;
        }

        if (i < ISP_BUF_INFO_NODE_MAX_CNT) {
            ISP_QUE_DBG(rt_dma,"returnDeqBufInfo::idx:%d\n",i);

            //This enQ is not an ex-change buffer request
            for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i++) {
                 if (bufInfo->next->base_pAddr == plist->mN[i].mInfo.base_pAddr)
                    break;
            }

            if (i < ISP_BUF_INFO_NODE_MAX_CNT) {
                ISP_QUE_DBG(rt_dma,"[MyDeQ]returnDeqBufInfo:: node=0x%8x", plist->mN[i].mInfo.base_pAddr);
                plist->mN[i].mOccupied = MFALSE;
                plist->mN[i].mInfo.base_pAddr = 0x0;
            }
            else
            {
                if(bVF_en){
                    ISP_QUE_DBG(rt_dma,"dma:0x%x: find no match baseaddr, 0x%p\n",rt_dma,bufInfo->next->base_vAddr);
                    ISP_QUE_DBG(rt_dma,"current available base addr\n");
                    for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i++){
                        if(plist->mN[i].mOccupied)
                            ISP_QUE_DBG(rt_dma,"0x%x",plist->mN[i].mInfo.base_pAddr);
                    }
                }
            }
        } else {
            ISP_QUE_DBG(rt_dma,"dma:0x%x:find no match baseaddr,caused this is exchange buf, 0x%x\n",rt_dma,bufInfo->next->base_pAddr);
            ISP_QUE_DBG(rt_dma,"current available base addr\n");
            for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i++){
                if(plist->mN[i].mOccupied)
                    ISP_QUE_DBG(rt_dma,"0x%x",plist->mN[i].mInfo.base_pAddr);
            }
        }
    }
#endif

    return 0;
}



stISP_BUF_INFO*
ISP_BUF_CTRL::
saveDeqBufInfo( MINT32 rt_dma )
{
    stISP_BUF_INFO *ptr = NULL;
    stISP_BUF_HEADER_LIST *plist = NULL;
    int i;

    if (_rt_dma_max_ <= rt_dma) {
       ISP_FUNC_ERR("saveDeqBufInfo:: Invalid rt_dma=%d", rt_dma);
       return NULL;
    }

#if (ENABLE_ATTACH_BUF_INFO)
    plist = &m_p1DeqedBufHeaderL[rt_dma];
    Mutex::Autolock lock(plist->mLLock);

    for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i++) {

        if (!plist->mN[i].mOccupied) {
            ptr = &plist->mN[i].mInfo;
            plist->mN[i].mOccupied = MTRUE;
            plist->mIdx = i;//(plist->mIdx+1) % ISP_BUF_INFO_NODE_MAX_CNT;
            ISP_QUE_DBG(rt_dma,"saveDeqBufInfo[A] plist=0x%x,[%d]PA=0x%8x, mIdx=%d", plist,i,ptr->base_pAddr, plist->mIdx);
            return ptr;
        }
    }
#endif
    if (1) {
        for (i = 0; i < ISP_BUF_INFO_NODE_MAX_CNT; i+=8) {

            ISP_FUNC_ERR("saveDeqBufInfo[C] Fail. dma=%d,plist=0x%x, mOccupied:[%d[0x%x]/%d[0x%x]/%d[0x%x]/%d[0x%x]/%d[0x%x]/%d[0x%x]/%d[0x%x]/%d[0x%x]]",rt_dma,plist,\
             plist->mN[i+0].mOccupied,plist->mN[i+0].mInfo.base_pAddr, plist->mN[i+1].mOccupied,plist->mN[i+1].mInfo.base_pAddr, plist->mN[i+2].mOccupied,plist->mN[i+2].mInfo.base_pAddr, plist->mN[i+3].mOccupied,plist->mN[i+3].mInfo.base_pAddr,\
                      plist->mN[i+4].mOccupied,plist->mN[i+4].mInfo.base_pAddr, plist->mN[i+5].mOccupied,plist->mN[i+5].mInfo.base_pAddr, plist->mN[i+6].mOccupied,plist->mN[i+6].mInfo.base_pAddr, plist->mN[i+7].mOccupied,plist->mN[i+7].mInfo.base_pAddr);
        }
    }

    //if can't find a variable node, then get a oldest node to use
    if (0) {
        ptr = &plist->mN[plist->mIdx].mInfo;
        plist->mN[i].mOccupied = MTRUE;
        plist->mIdx = (plist->mIdx+1) % ISP_BUF_INFO_NODE_MAX_CNT;

        ISP_QUE_DBG(rt_dma,"saveDeqBufInfo[D][Error] plist=0x%x,ptr=0x%8x, mIdx=%d", plist,ptr, plist->mIdx);
    }
    if(ptr!= NULL)
        ISP_QUE_DBG(rt_dma,"[MyDeQ]saveDeqBufInfo: node=0x%x", ptr->base_pAddr);
    return ptr;
}
//


//
/*
description: move FILLED buffer from hw to sw list
called at passx_done
*/
EIspRetStatus
ISP_BUF_CTRL::
dequeueHwBuf( MUINT32 dmaChannel, stISP_FILLED_BUF_LIST& bufList )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 count = 1;
    MUINT32 start;
    EIspRetStatus ret = eIspRetStatus_Success;
    //ISP_RT_BUF_STRUCT* pstRTBuf = (ISP_RT_BUF_STRUCT*)this->m_pIspDrvShell->m_pPhyIspDrv->m_pRTBufTbl;
    //stISP_BUF_INFO bufInfo;
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    ISP_DEQUE_BUF_INFO_STRUCT deque_buf;

    //ISP_QUE_INF(rt_dma,"dmaChannel(%d),path(%d/%d)",dmaChannel,this->path,this->path_D);
    //Mutex::Autolock lock(queHwLock);
    //
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error ");
        ret = eIspRetStatus_Failed;
        goto EXIT;
    }
    //
    if ( ISP_PASS1   == this->path || \
         ISP_PASS1_D == this->path_D || \
         ISP_PASS1_CAMSV   == this->path || \
         ISP_PASS1_CAMSV_D == this->path_D
        ) {
        //
#if (ENABLE_ATTACH_BUF_INFO)

        stISP_BUF_INFO bufInfoTemp;
        stISP_BUF_INFO *bufInfo = &bufInfoTemp;
        memset(bufInfo, 0, sizeof(stISP_BUF_INFO));

        stISP_BUF_INFO *bufHeaderInfo = saveDeqBufInfo(rt_dma);

#else
        static stISP_BUF_INFO bufInfoTemp;
        stISP_BUF_INFO *bufInfo = &bufInfoTemp;
        memset(bufInfo, 0, sizeof(stISP_BUF_INFO));

#endif

        if (!bufInfo) {

            ISP_FUNC_ERR("[Error] Get BufInfo Entry Fail. ");
            ret = eIspRetStatus_Failed;
            goto EXIT;
        }


        if ( _imgo_ == rt_dma || \
             _ufeo_ == rt_dma || \
             _rrzo_ == rt_dma || \
             _imgo_d_ == rt_dma || \
             _rrzo_d_ == rt_dma || \
             _camsv_imgo_ == rt_dma || \
             _camsv2_imgo_ == rt_dma
           ) {
            UINT32 fbc[2];
            UINT32 vf=0;

            if( (_imgo_ == rt_dma) || (_rrzo_ == rt_dma) ){
                fbc[0] = readReg(0x000040f0);
                fbc[1] = readReg(0x000040f4);
                vf = readReg(0x00004414);
            }
            else if( (_imgo_d_ == rt_dma)|| (_rrzo_d_ == rt_dma)){
                fbc[0] = readReg(0x000040f8);
                fbc[1] = readReg(0x000040fc);
                vf = readReg(0x00006414);
            }
            else {//( (_camsv_imgo_ == rt_dma) || (_camsv2_imgo_))
                fbc[0] = readReg(0x0000901C);
                fbc[1] = readReg(0x0000981C);
            }
            //deque filled buffer
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_DEQUE;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.data_ptr = 0;
            buf_ctrl.pExtend = (unsigned char*)&deque_buf;
            //ISP_QUE_INF(rt_dma,"dma(%d),ctrl(%d),id(%d),ptr(0x%x)",rt_dma,buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                ret = eIspRetStatus_Failed;
                goto EXIT;
            }
            P1DeQCnt[rt_dma] -= 1;
            if( (_imgo_ == rt_dma) || (_rrzo_ == rt_dma) ){
                ISP_QUE_DBG(rt_dma,"imgo/rrzo: vf=0x%x, fbc: 0x%x/0x%x_0x%x/0x%x\n",vf,fbc[0],fbc[1],readReg(0x000040f0),readReg(0x000040f4));
            }
            else if( (_imgo_d_ == rt_dma)|| (_rrzo_d_ == rt_dma)){
                ISP_QUE_DBG(rt_dma,"imgo_d/rrzo_d:vf=0x%x, fbc: 0x%x/0x%x_0x%x/0x%x\n",vf,fbc[0],fbc[1],readReg(0x000040f8),readReg(0x000040fc));
            }
            else {//( (_camsv_imgo_ == rt_dma) || (_camsv2_imgo_))
                ISP_QUE_DBG(rt_dma,"camsv/camsv2 fbc: 0x%x/0x%x_0x%x/0x%x\n",fbc[0],fbc[1],readReg(0x0000901C),readReg(0x0000981C));
            }
            //
            count = deque_buf.count;
            //ISP_FUNC_DBG("deque_buf.count(%d)",deque_buf.count);
            if ( ISP_RT_BUF_SIZE < count ) {
                ISP_FUNC_ERR("ERROR:deque_buf.count(%d)",deque_buf.count);
                ret = eIspRetStatus_Failed;
                goto EXIT;
            }
            //
            for(MUINT32 i=0;i<count;i++) {
                ISP_QUE_INF(rt_dma,"i(%d),dma(%d),id(0x%x),size(0x%x),xsize(0x%x),VA(0x%llx),PA(0x%x),crop(0x%x,0x%x,0x%x,0x%x),count(%d),cur sof(%d),frm_cnt(%d),mag(0x%x),rawType(%d)",
                    i, \
                    rt_dma, \
                    deque_buf.data[i].memID, \
                    deque_buf.data[i].size, \
                    deque_buf.data[i].image.xsize,\
                    deque_buf.data[i].base_vAddr, \
                    deque_buf.data[i].base_pAddr, \
                    deque_buf.data[i].dmaoCrop.x, \
                    deque_buf.data[i].dmaoCrop.y, \
                    deque_buf.data[i].dmaoCrop.w, \
                    deque_buf.data[i].dmaoCrop.h, \
                    deque_buf.count,\
                    deque_buf.sof_cnt,\
                    deque_buf.data[i].image.frm_cnt,\
                    deque_buf.data[i].image.m_num_0,\
                    deque_buf.data[i].bProcessRaw);
                //
                bufInfo->status      = ISP_BUF_FILLED;
                bufInfo->memID       = deque_buf.data[i].memID;
                bufInfo->size        = deque_buf.data[i].size;
                bufInfo->base_vAddr  = (MUINTPTR)deque_buf.data[i].base_vAddr;
                bufInfo->base_pAddr  = deque_buf.data[i].base_pAddr;
                bufInfo->timeStampS  = deque_buf.data[i].timeStampS;
                bufInfo->timeStampUs = deque_buf.data[i].timeStampUs;

                bufInfo->img_w       = deque_buf.data[i].image.w;    //current w is tg grab width, rrzo plz reference dstW
                bufInfo->img_h       = deque_buf.data[i].image.h;
                bufInfo->img_stride  = deque_buf.data[i].image.stride;
                bufInfo->img_fmt    = deque_buf.data[i].image.fmt;
                bufInfo->img_pxl_id = deque_buf.data[i].image.pxl_id;
                bufInfo->img_wbn    = deque_buf.data[i].image.wbn;
                bufInfo->img_ob     = deque_buf.data[i].image.ob;
                bufInfo->img_lsc    = deque_buf.data[i].image.lsc;
                bufInfo->img_rpg    = deque_buf.data[i].image.rpg;
                bufInfo->m_num_0    = deque_buf.data[i].image.m_num_0;
                bufInfo->m_num_1    = deque_buf.data[i].image.m_num_0;
                bufInfo->frm_cnt    = deque_buf.data[i].image.frm_cnt;
                bufInfo->raw_type    = deque_buf.data[i].bProcessRaw;
                bufInfo->rrz_info.srcX = deque_buf.data[i].rrzInfo.srcX;
                bufInfo->rrz_info.srcY = deque_buf.data[i].rrzInfo.srcY;
                bufInfo->rrz_info.srcW = deque_buf.data[i].rrzInfo.srcW;
                bufInfo->rrz_info.srcH = deque_buf.data[i].rrzInfo.srcH;
                bufInfo->rrz_info.dstW = deque_buf.data[i].rrzInfo.dstW;
                bufInfo->rrz_info.dstH = deque_buf.data[i].rrzInfo.dstH;
                bufInfo->dma_crop.x= GetCropXUnitPixel(deque_buf.data[i].dmaoCrop.x, deque_buf.data[i].image.fmt,deque_buf.data[i].image.bus_size);
                bufInfo->dma_crop.y = deque_buf.data[i].dmaoCrop.y;
                bufInfo->dma_crop.floatX = 0;
                bufInfo->dma_crop.floatY = 0;
                //dmao crop support no rrzo
                bufInfo->dma_crop.w= deque_buf.data[i].dmaoCrop.w;
                bufInfo->dma_crop.h = deque_buf.data[i].dmaoCrop.h;
                //fmt based on coda 0x15004028_out_fmt
                switch(bufInfo->img_fmt){
                    case 0://bayer 8
                        break;
                    case 1://bayer 10
                        bufInfo->dma_crop.w = bufInfo->dma_crop.w * 8 / 10;
                        break;
                    case 2://bayer 12
                        bufInfo->dma_crop.w = bufInfo->dma_crop.w * 8 / 12;
                        break;
                    default:
                        ISP_FUNC_ERR("error:fmt r not supported(0x%x)\n",bufInfo->img_fmt);
                        break;
                }
                if((_rrzo_ == rt_dma) || (_rrzo_d_ == rt_dma))
                    bufInfo->dma_crop.w = bufInfo->dma_crop.w * 2 / 3;


                bufInfo->dma = dmaChannel;

                if((bufInfo->timeStampS==0) && (bufInfo->timeStampUs==0) ){
                    ISP_FUNC_ERR("error:timestamp err\n");
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();

                    ISP_FUNC_ERR("\n CQ0:\n");
                    for(int i=0;i<3;i++){
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)ISP_DRV_CQ0,0, i,(ISP_DRV_CQ_CMD_DESC_STRUCT*)this->m_pPhyIspDrv->getCQDescBufVirAddr(ISP_DRV_CQ0,0,i),\
                        this->m_pPhyIspDrv->getCQVirBufVirAddr(ISP_DRV_CQ0,0,i));
                    }
                    ISP_FUNC_ERR("\n CQ0_D:\n");
                    for(int i=0;i<3;i++){
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)ISP_DRV_CQ0_D,0, i,(ISP_DRV_CQ_CMD_DESC_STRUCT*)this->m_pPhyIspDrv->getCQDescBufVirAddr(ISP_DRV_CQ0_D,0,i),\
                        this->m_pPhyIspDrv->getCQVirBufVirAddr(ISP_DRV_CQ0_D,0,i));
                    }
                }
#if ENABLE_ATTACH_BUF_INFO

                if(bufHeaderInfo)
                    memcpy(bufHeaderInfo, bufInfo, sizeof(stISP_BUF_INFO));
                else
                    ISP_FUNC_ERR("bufHeaderInfo is NULL,this will cause NE issue\n");
                bufInfo->header_info = (MVOID*)bufHeaderInfo; //point to self
                bufInfo->header_size = sizeof(stISP_BUF_INFO);
                bufList.pBufList->push_back((*bufInfo));
#else
                bufInfo->header_info = (MVOID*)bufList; //point to self
                bufInfo->header_size = sizeof(stISP_BUF_INFO);
                bufList.pBufList->push_back((*bufInfo));
#endif

#if 0
                ISP_QUE_INF(rt_dma,"bufInfoAddr=0x%x,d(%d)/s(%d)/id(0x%x)/bs(0x%x)/va(0x%x)/pa(0x%x)/t(%d.%d)/img(%d,%d,%d,%d,%d,%d,%d,%d,%d)/m(%d,%d)/fc(%d)", \
                        bufInfo,
                        rt_dma, \
                        count, \
                        bufInfo->memID, \
                        bufInfo->size, \
                        bufInfo->base_vAddr, \
                        bufInfo->base_pAddr, \
                        bufInfo->timeStampS, \
                        bufInfo->timeStampUs, \
                        bufInfo->img_w, \
                        bufInfo->img_h, \
                        bufInfo->img_stride, \
                        bufInfo->img_fmt, \
                        bufInfo->img_pxl_id, \
                        bufInfo->img_wbn, \
                        bufInfo->img_ob, \
                        bufInfo->img_lsc, \
                        bufInfo->img_rpg, \
                        bufInfo->m_num_0, \
                        bufInfo->m_num_1, \
                        bufInfo->frm_cnt);
#endif
            }
        }
    }
    else { // Pass2
        MDPMGR_DEQUEUE_INDEX mdpQueueIdx;
        MBOOL isDequeMdp = MTRUE;
        ISP_DRV_P2_CQ_ENUM p2Cq;
        MUINT32 bufSize = 0;
        MUINT32 mdpMgrCfgDataImgxoEnP2;
        MDPMGR_CFG_STRUCT mdpMgrCfgStructData;
        //
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->dequeCq, p2Cq);

        while(count--) {
            this->m_pIspDrvShell->gPass2Lock.lock();
            bufSize = m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.size();
            mdpMgrCfgDataImgxoEnP2 = gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].imgxoEnP2;
            memcpy(&mdpMgrCfgStructData, &gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx], sizeof(MDPMGR_CFG_STRUCT));
            this->m_pIspDrvShell->gPass2Lock.unlock();
            if (bufSize) {
                //
                // MDP flow
                //
                ISP_FUNC_DBG("drvScenario(%d),rt_dma(%d)",this->drvScenario,rt_dma);
                switch(rt_dma){
                    case _imgi_:
                        if(this->drvScenario != NSImageio::NSIspio::eDrvScenario_IP){
                            mdpQueueIdx = MDPMGR_DEQ_SRC;
                        } else {
                            isDequeMdp = MFALSE;
                        }
                        break;
                    case _wrot_:
                        mdpQueueIdx = MDPMGR_DEQ_WROT;
                        break;
                    case _wdma_:
                        mdpQueueIdx = MDPMGR_DEQ_WDMA;
                        break;
                    case _jpeg_:
                        mdpQueueIdx = MDPMGR_DEQ_JPEG;
                        break;
                    case _venc_stream_:
                        mdpQueueIdx = MDPMGR_DEQ_VENCO;
                        break;
                    case _img2o_: /* for compatible MDP flow */
                        if(mdpMgrCfgDataImgxoEnP2 == CAM_CTL_EN_P2_DMA_IMG2O_EN){
                            ISP_QUE_INF(rt_dma,"MDP-IMGXO-IMG2O");
                            mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                        }else{
                            isDequeMdp = MFALSE;
                        }
                        break;
                    case _img3o_: /* for compatible MDP flow */
                        if(mdpMgrCfgDataImgxoEnP2 == CAM_CTL_EN_P2_DMA_IMG3O_EN){
                            ISP_QUE_INF(rt_dma,"MDP-IMGXO-IMG3O");
                            mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                        }else{
                            isDequeMdp = MFALSE;
                        }
                        break;
                    case _imgo_: /* for compatible MDP flow */
                        // need to run mdp dequeue funciton at lease one time
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                        #if 0
                        if(mdpMgrCfgDataImgxoEnP2 == CAM_CTL_EN_P2_DMA_MDP_IMGXO_IMGO_EN){
                            ISP_QUE_INF(rt_dma,"MDP-IMGXO-IMGO");
                            mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                        }else{
                            isDequeMdp = MFALSE;
                        }
                        #endif
                        break;
                    default:
                        isDequeMdp = MFALSE;
                        break;
                };

                #if 0 //kk test default:0
                {
                    MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
                    char* pWorkingBuffer =  NULL;
                    isp_reg_t *pIspReg;
                    IspDumpDbgLogP2Package p2DumpPackage;
                    ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
                    // dump CQ vir/des table
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)0,0, this->dequeDupCqIdx);
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)this->dequeCq,this->burstQueIdx, this->dequeDupCqIdx);
                    // dump difference between tpipe and hardware registers
                    pIspReg = (isp_reg_t*)this->m_pIspDrvShell->m_pPhyIspDrv_bak->getCurHWRegValues();
                    pWorkingBuffer = new char[tpipeWorkingSize];
                    //
                    this->m_pIspDrvShell->gPass2Lock.lock();
                    if((pWorkingBuffer!=NULL)&&(pIspReg!=NULL)) {
                        tpipe_main_platform(&gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspReg);
                        delete pWorkingBuffer;
                        pWorkingBuffer = NULL;
                    } else {
                        ISP_FUNC_ERR("[Error]pWorkingBuffer(0x%08x),pIspReg(0x%08x)",pWorkingBuffer,pIspReg);
                    }
                    // dump hardware debug information
                    p2DumpPackage.tpipeTableVa = gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa;
                    p2DumpPackage.tpipeTablePa = gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa;
                    this->m_pIspDrvShell->gPass2Lock.unlock();
                    //
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(NULL,&p2DumpPackage);
                }
                #endif


                if(isDequeMdp==MTRUE){
                    MDPMGR_RETURN_TYPE mdpRet;
                    //
                    ISP_FUNC_DBG("dequeBurstQueIdx(%d),p2Cq(%d),this->dequeDupCqIdx(%d)",this->dequeBurstQueIdx,p2Cq,this->dequeDupCqIdx);

                    mdpRet = (MDPMGR_RETURN_TYPE)g_pMdpMgr->dequeueBuf(mdpQueueIdx, mdpMgrCfgStructData);
                    if(mdpRet == MDPMGR_NO_ERROR){
                        ret = eIspRetStatus_Success;
                    } else if(mdpRet == MDPMGR_VSS_NOT_READY){
                        ret = eIspRetStatus_VSS_NotReady;
                    } else {
                        MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
                        char* pWorkingBuffer =  NULL;
                        isp_reg_t *pIspReg;
                        IspDumpDbgLogP2Package p2DumpPackage;
                        ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
                        // dump CQ vir/des table
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)0,0, this->dequeDupCqIdx);
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)this->dequeCq,this->burstQueIdx, this->dequeDupCqIdx);
                        // dump difference between tpipe and hardware registers
                        pIspReg = (isp_reg_t*)this->m_pIspDrvShell->m_pPhyIspDrv_bak->getCurHWRegValues();
                        pWorkingBuffer = new char[tpipeWorkingSize];
                        //
                        this->m_pIspDrvShell->gPass2Lock.lock();
                        if((pWorkingBuffer!=NULL)&&(pIspReg!=NULL)) {
                            tpipe_main_platform(&gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspReg);
                            delete pWorkingBuffer;
                            pWorkingBuffer = NULL;
                        } else {
                            ISP_FUNC_ERR("[Error]pWorkingBuffer(0x%08x),pIspReg(0x%08x)",pWorkingBuffer,pIspReg);
							if(pWorkingBuffer!=NULL)
								delete pWorkingBuffer;
                        }
                        // dump hardware debug information
                        p2DumpPackage.tpipeTableVa = gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa;
                        p2DumpPackage.tpipeTablePa = gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa;
                        this->m_pIspDrvShell->gPass2Lock.unlock();
                        //
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(NULL,&p2DumpPackage);
                        //
                        ret = eIspRetStatus_Failed;

                        ISP_FUNC_ERR("[Error]dequeueBuf fail mdpRet(%d)",mdpRet);
                        goto EXIT;
                    }
                }
                #if 0
                else if(rt_dma==_imgo_) {
                    // we need to enque/deque from isp fucntion only for frame mode.
                    // In this circumstance only dma imgo need to be dequed

                    MBOOL waitIrqRet = MTRUE;
                    //wait pass2 done
                    ISP_DRV_WAIT_IRQ_STRUCT irqPass2Done = {ISP_DRV_IRQ_CLEAR_WAIT,
                                                              ISP_DRV_IRQ_TYPE_INT_P2_ST,
                                                              CAM_CTL_INT_P2_STATUS_PASS2_DON_ST,
                                                              (int)ISP_DRV_IRQ_USER_ISPDRV,
                                                              CAM_INT_PASS2_WAIT_TIMEOUT_MS,
                                                              "isp_function",
                                                              0};

                    ISP_FUNC_INF("deque by isp self");
                    waitIrqRet = this->m_pPhyIspDrv->waitIrq( &irqPass2Done );
                    if (MFALSE==waitIrqRet || function_DbgLogEnable_VERBOSE==true) {
                        // dump CQ vir/des table
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)this->dequeCq,this->burstQueIdx, this->dequeDupCqIdx);
                    }
                    if ( MFALSE == waitIrqRet ) {
                        ISP_FUNC_ERR("waitIrq( irqPass2Done ) fail");
                        ret = eIspRetStatus_Failed;
                        goto EXIT;
                    }

                    #if 0 //kk test default:0 (for ip raw debug)
                    {
                    MUINT32 i;

                    for(i=0x4;i<=0x200;i+=4){
                        ISP_FUNC_INF("[0x%04x,0x%08x]",(i+0x4000),this->m_pIspDrvShell->getPhyIspDrv()->readReg(i+0x4000));
                    }

                    for(i=0x3000;i<=0x3600;i+=4){
                        ISP_FUNC_INF("[0x%04x,0x%08x]",(i+0x4000),this->m_pIspDrvShell->getPhyIspDrv()->readReg(i+0x4000));
                    }

                    }
                    #endif
                }
                #endif

                this->m_pIspDrvShell->gPass2Lock.lock();
                //change type
                m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front().status = ISP_BUF_FILLED;
                if (_jpeg_ == rt_dma)
                {
                    m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front().jpg_size = g_pMdpMgr->getJPEGSize(gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx]);
                    ISP_QUE_INF(rt_dma,"jpeg real size(0x%x)", m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front().jpg_size);
                }
                //
                //add to bufList
                //all element at the end
                bufList.pBufList->push_back(m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front());
                //
#if defined(__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
                ISP_FUNC_DBG("unmapPhyAddr");
                //m4u only
                IMEM_BUF_INFO buf_info;
                buf_info.memID = m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.back().memID;
                buf_info.size = m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.back().size;
                buf_info.virtAddr = m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.back().base_vAddr;
                buf_info.phyAddr = m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.back().base_pAddr;
                //free mva
                #if 0
                //user should make sure PA if using hw unless the va is not from ion(malloc va, ap buffer)
                //do this in iopipe
                if(buf_info.memID==-1)
                {
                     if ( this->m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info) ) {
                         ISP_FUNC_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
                         ret = eIspRetStatus_Failed;
                         goto EXIT;
                     }
                }
                #endif
#endif
                if(isDequeMdp==true && ret == eIspRetStatus_VSS_NotReady)
                {}
                else
                {
                    //delete from hw list
                    m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.pop_front();
                }
                //
                ISP_FUNC_DBG("dma(%d),hwsize(%d),p2Cq(%d),dequeDupCqIdx(%d),imgxoEnP2(0x%x),memID(0x%x),size(%d),vaddr(0x%p),paddr(0x%x)",rt_dma,m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.size(),p2Cq,this->dequeDupCqIdx, \
                        gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].imgxoEnP2, \
                        bufList.pBufList->back().memID,bufList.pBufList->size(),bufList.pBufList->back().base_vAddr,bufList.pBufList->back().base_pAddr);

                this->m_pIspDrvShell->gPass2Lock.unlock();

            } else {
                ISP_FUNC_ERR("empty HW buffer");
                ret = eIspRetStatus_Failed;
                goto EXIT;
            }
        }
    }
    //
EXIT:
    return ret;
}

//

#if 0
/*
description: move current buffer to filled buffer list
*/
MINT32
ISP_BUF_CTRL::
dequeueSwBuf( MUINT32 dmaChannel, stISP_FILLED_BUF_LIST& bufList )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 cnt = 0;
    struct _isp_buf_list_ *old_hw_head = NULL;
    //
    Mutex::Autolock lock(queSwLock);
    //
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    //
    if ( 0 == m_swbufL[rt_dma].bufInfoList.size() ) {
        //wait semephore till
        ISP_FUNC_ERR("empty SW buffer");
        return -1;
    }
    //
    while ( m_swbufL[rt_dma].bufInfoList.size() ) {
        //all element at the end
        bufList.pBufList->push_back(m_swbufL[rt_dma].bufInfoList.front());
        //delete first element
        m_swbufL[rt_dma].bufInfoList.pop_front();
        //
        ISP_FUNC_DBG("05_dma(%d)/memID(0x%x)/size(%d)/vaddr(0x%p)/paddr(0x%x)",rt_dma,
                                    bufList.pBufList->back().memID, \
                                    bufList.pBufList->size(), \
                                    bufList.pBufList->back().base_vAddr,
                                    bufList.pBufList->back().base_pAddr);
    }
    //
    return 0;
}
#endif

//
MUINT32
ISP_BUF_CTRL::
getCurrHwBuf( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 retAddr = NULL;
    //
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error ");
        return 0;
    }
    //
    if ( ISP_PASS1   == this->path || \
         ISP_PASS1_D == this->path_D || \
         ISP_PASS1_CAMSV   == this->path || \
         ISP_PASS1_CAMSV_D == this->path
        ) {
        if (0 == m_hwbufL[rt_dma].bufInfoList.size()) {
            ISP_FUNC_ERR("ERROR:No buffer in queue");
            return 0;
        }
        retAddr = m_hwbufL[rt_dma].bufInfoList.front().base_pAddr;
        //
        ISP_FUNC_DBG("dma:(%d)/memID(0x%x)/vAddr:(0x%p)/pAddr:(0x%x) ",rt_dma, \
                                                            m_hwbufL[rt_dma].bufInfoList.front().memID, \
                                                            m_hwbufL[rt_dma].bufInfoList.front().base_vAddr, \
                                                            m_hwbufL[rt_dma].bufInfoList.front().base_pAddr);
    } else { // Pass2
        ISP_DRV_P2_CQ_ENUM p2Cq;
        //
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
        if (0 == m_p2HwBufL[p2Cq][this->dupCqIdx][rt_dma].bufInfoList.size()) {
            ISP_FUNC_ERR("ERROR:No buffer in queue");
            return 0;
        }
        retAddr = m_p2HwBufL[p2Cq][this->dupCqIdx][rt_dma].bufInfoList.front().base_pAddr;
    }
    //
    return retAddr;
}
//
MUINT32
ISP_BUF_CTRL::
getNextHwBuf( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    ISP_BUF_INFO_L::iterator it;
    MUINT32 base_pAddr = 0;
    //
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("ERROR:dma channel error ");
        return -1;
    }
    //
    if (0 == m_hwbufL[rt_dma].bufInfoList.size()) {
        ISP_FUNC_ERR("ERROR:No buffer in queue");
        return 0;
    }
    //
    if ( 1 < m_hwbufL[rt_dma].bufInfoList.size() ) {
        it = m_hwbufL[rt_dma].bufInfoList.begin();
        it++;
        base_pAddr = it->base_pAddr;
    }

    ISP_FUNC_DBG("dma:[%d]/base_pAddr:[0x%x] ",rt_dma,base_pAddr);

    return base_pAddr;
}
//
MUINT32
ISP_BUF_CTRL::
freeSinglePhyBuf(
    stISP_BUF_INFO bufInfo)
{
    MUINT32 ret = 0;
#if 0

    IMEM_BUF_INFO buf_info;

    ISP_FUNC_DBG(":E");
    //
    //this->m_pIspDrvShell->m_pIMemDrv->freePhyMem(virtAddr,memID,size,phyAddr);

    //buf_info.type = (EBUF_TYPE)bufInfo.type;
    buf_info.size = bufInfo.size;
    buf_info.memID = bufInfo.memID;
//    buf_info.ionFd = bufInfo.ionFD;
    buf_info.virtAddr = bufInfo.base_vAddr;
    buf_info.phyAddr = bufInfo.base_pAddr;
    if ( this->m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info) ) {
        ISP_FUNC_ERR("ERROR:m_pIMemDrv->unmapPhyAddr ");
    }

    ISP_FUNC_DBG(":X");
#else
    ISP_FUNC_ERR("ERROR:NOT SUPPORT YET!");
#endif
    return ret;
}
//
MUINT32
ISP_BUF_CTRL::
freeAllPhyBuf( void )
{
    MUINT32 ret = 0;
#if 0
    ISP_FUNC_DBG(":E");
    //
    this->m_pIspDrvShell->m_pIMemDrv->reset();

    ISP_FUNC_DBG(":X");
#else
    ISP_FUNC_ERR("ERROR:NOT SUPPORT YET!");
#endif
    return ret;
}
//
int
ISP_BUF_CTRL::
getDmaBufIdx( MUINT32 dmaChannel )
{
    _isp_dma_enum_ dma;

    switch(dmaChannel) {
        case ISP_DMA_IMGI:  dma = _imgi_;   break;
        case ISP_DMA_UFDI:  dma = _ufdi_;   break;
        case ISP_DMA_LCEI:  dma = _lcei_;   break;
        case ISP_DMA_VIPI:  dma = _vipi_;   break;
        case ISP_DMA_VIP2I: dma = _vip2i_;  break;
        case ISP_DMA_VIP3I: dma = _vip3i_;  break;
        case ISP_DMA_IMGO:  dma = _imgo_;   break;
        case ISP_DMA_RRZO:  dma = _rrzo_;   break;
        case ISP_DMA_IMGO_D:dma = _imgo_d_; break;
        case ISP_DMA_RRZO_D:dma = _rrzo_d_; break;
        case ISP_DMA_MFBO: dma = _mfbo_;  break;
        case ISP_DMA_IMG2O: dma = _img2o_;  break;
        case ISP_DMA_IMG3O: dma = _img3o_;  break;
        case ISP_DMA_IMG3BO:dma = _img3bo_; break;
        case ISP_DMA_IMG3CO: dma = _img3co_;  break;
        case ISP_DMA_WDMAO:dma = _wdma_; break;
        case ISP_DMA_WROTO:dma = _wrot_; break;
        case ISP_DMA_JPEGO:dma = _jpeg_; break;
        case ISP_DMA_VENC_STREAMO:dma = _venc_stream_; break;
        case ISP_DMA_CAMSV_IMGO: dma = _camsv_imgo_; break;
        case ISP_DMA_CAMSV2_IMGO: dma = _camsv2_imgo_; break;
        case ISP_DMA_FEO:dma = _feo_; break;
        default:
            ISP_FUNC_ERR("Invalid dma channel(%d)",dmaChannel);
            return 0;
    }

    return (int)dma;
}
//
int
ISP_BUF_CTRL::
debugPrint( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    ISP_BUF_INFO_L::iterator it;

    ISP_FUNC_DBG("E");
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    //
    for ( it = m_hwbufL[rt_dma].bufInfoList.begin(); it != m_hwbufL[rt_dma].bufInfoList.end(); it++ ) {
        ISP_FUNC_DBG("m_hwbufL[%d].base_vAddr:[0x%p]/base_pAddr:[0x%x] ",rt_dma,it->base_vAddr,it->base_pAddr);
    }
    ISP_FUNC_DBG("X");

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    isp event thread
  /////////////////////////////////////////////////////////////////////////////*/
class IspEventThreadImp : public IspEventThread
{
    IspEventThreadImp(){}
    //
    public:     ////        Instantiation.
        static IspEventThread*  getInstance(IspDrv* pIspDrv);
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        virtual MVOID    Start(void);
    private:
        static MVOID* ispEventThread(void *arg);
    private:
        sem_t    m_semIspEventthread;
        //
        pthread_t       m_ispEventthread;
        mutable Mutex   mLock;
        volatile MINT32 mInitCount;
        IspDrv*            m_pPhyIspDrv;
        MBOOL            m_bBreak;
};

//
IspEventThread*  IspEventThread::createInstance(IspDrv* pIspDrv)
{
    return IspEventThreadImp::getInstance(pIspDrv);
}
//
IspEventThread*  IspEventThreadImp::getInstance(IspDrv* pIspDrv)
{
    ISP_FUNC_DBG("E: pIspDrv(0x%08x)",pIspDrv);
    //
    //static IspEventThreadImp singleton;
    static IspEventThreadImp singleton;
    //
    singleton.m_pPhyIspDrv = pIspDrv;
    //
    return &singleton;
}
//
MBOOL IspEventThreadImp::init(void)
{
    Mutex::Autolock lock(mLock);
    ISP_FUNC_DBG("mInitCount(%d) ", mInitCount);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        return MTRUE;
    }

    // Init semphore
    ::sem_init(&m_semIspEventthread, 0, 0);

    // Create main thread for preview and capture
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, PRIO_RT_ISP_EVENT_THREAD};
    pthread_create(&m_ispEventthread, &attr, ispEventThread, this);
    //
    android_atomic_inc(&mInitCount);
    m_bBreak = MFALSE;
    ISP_FUNC_DBG("X");
    return MTRUE;
}
//
MBOOL IspEventThreadImp::uninit(void)
{
    Mutex::Autolock lock(mLock);
    ISP_FUNC_DBG("mInitCount(%d)",mInitCount);
    //
    if(mInitCount <= 0)
    {
        // No more users
        return MTRUE;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        return MTRUE;
    }

    //
    pthread_join(m_ispEventthread, NULL);
    m_bBreak = MTRUE;
    //ISP_FUNC_DBG("[ispEventthread] Wait m_semIspEventthread");
    ::sem_wait(&m_semIspEventthread); // wait here until someone use sem_post() to wake this semaphore up
    //ISP_FUNC_DBG("[ispEventthread] Got m_semIspEventthread");
    ISP_FUNC_DBG("X");

    return MTRUE;
}

MVOID IspEventThreadImp::Start(void){
    ::sem_post(&m_semIspEventthread);
}

//
MVOID* IspEventThreadImp::ispEventThread(void *arg)
{
    IspEventThreadImp *_this = reinterpret_cast<IspEventThreadImp*>(arg);
    ISP_DRV_WAIT_IRQ_STRUCT irq_SOF;
    ISP_DRV_WAIT_IRQ_STRUCT irq_VS;
    ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_DONE;


    ISP_FUNC_DBG("E: tid=%d m_pPhyIspDrv(0x%x)", gettid(),_this->m_pPhyIspDrv);

    ::prctl(PR_SET_NAME,"ispEventthread",0,0,0);

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());
    //
    irq_SOF.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    irq_SOF.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
    irq_SOF.Status = CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;
    irq_SOF.UserNumber=(int)ISP_DRV_IRQ_USER_ISPDRV;
    irq_SOF.Timeout = CAM_INT_WAIT_TIMEOUT_MS;
    irq_SOF.UserName = "isp_eventT";
    //ISP_FUNC_DBG("[ispEventthread] Wait m_semIspEventthread");
    ::sem_wait(&_this->m_semIspEventthread); // wait here until someone use sem_post() to wake this semaphore up
    //ISP_FUNC_DBG("[ispEventthread] Got m_semIspEventthread");
    //
    while (1)
    {

        if(_this->m_bBreak){
            break;
        }
        //1:SOF
        _this->m_pPhyIspDrv->waitIrq( &irq_SOF );
        ISP_FUNC_DBG("SOF");
    }
    ::sem_post(&_this->m_semIspEventthread);
    //

    ISP_FUNC_DBG("X");

    return NULL;
}


/*/////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////*/
//
#define ISP_KERNEL_MOTIFY_SINGAL_TEST
#ifdef ISP_KERNEL_MOTIFY_SINGAL_TEST
int isp_trigger_signal(int *pid)
{
int ret = 0;
int mFd=0;
    mFd = open("/dev/camera-isp", O_RDWR);
    ret = ioctl(mFd,ISP_SET_USER_PID,pid);
    return 0;
}
#endif

