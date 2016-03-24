#define LOG_TAG "iio/ifunc_FrmB"


#include "isp_function_FrmB.h"
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
//

#include "mdp_mgr_FrmB.h"

#include <cutils/properties.h>  // For property_get().

#include "m4u_lib.h"

//
using namespace NSDrvCam_FrmB;


//
//digital zoom
#define CAM_ISP_ZOOMRARIO_GAIN (100)


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{ispf}"
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(function);
//EXTERN_DBG_LOG_VARIABLE(function);

// Clear previous define, use our own define.
#undef ISP_FUNC_VRB
#undef ISP_FUNC_DBG
#undef ISP_FUNC_INF
#undef ISP_FUNC_WRN
#undef ISP_FUNC_ERR
#undef ISP_FUNC_AST 
#define ISP_FUNC_VRB(fmt, arg...)        do { if (function_DbgLogEnable_VERBOSE && MTKCAM_LOGENABLE_DEFAULT) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_FUNC_DBG(fmt, arg...)        do { if (function_DbgLogEnable_DEBUG && MTKCAM_LOGENABLE_DEFAULT ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_FUNC_INF(fmt, arg...)        do { if (function_DbgLogEnable_INFO && MTKCAM_LOGENABLE_DEFAULT ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_FUNC_WRN(fmt, arg...)        do { if (function_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_FUNC_ERR(fmt, arg...)        do { if (function_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_FUNC_AST(cond, fmt, arg...)  do { if (function_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
 


#define ISP_FUNC_ORG_ISPDTREAM_FUNC 0 // avoid build err with new I/F of DpIspStream::dequeueSrcBuffer and dequeueDstBuffer
#define ISP_FUNC_MVA_ISPDTREAM_FUNC 1 //

#ifdef _rtbc_buf_que_2_0_
    static MUINT8 dma_array[_rt_dma_max_] = {0};//for fw rcnt controller
#endif

////////////////////////////////////
/////temp used for isponly mode UT
static int P2_UT_BypassMDP_TPIPE = 0;
static pthread_mutex_t p2_enque_deque_Mutex = PTHREAD_MUTEX_INITIALIZER;
static bool enque_running=false;
/////////////////////////////////////
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
 MdpMgr_FrmB *g_pMdpMgr_FrmB = NULL;
 IMEM_BUF_INFO  gMdpBufInfo_FrmB;
 MDPMGR_CFG_STRUCT_FRMB ***gpMdpMgrCfgData_FrmB;

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
namespace NSDrvCam_FrmB {

NSIspDrv_FrmB::IspDrv*         IspDrv_B::m_pPhyIspDrv = NULL;
NSIspDrv_FrmB::IspDrv*         IspDrv_B::m_pP1IspDrv = NULL;
NSIspDrv_FrmB::IspDrv*         IspDrv_B::m_pP1IspDrvD = NULL;
isp_reg_t*                     IspDrv_B::m_pP1CurCQDIspReg;
NSIspDrv_FrmB::IspDrv*         IspDrv_B::m_pP2IspDrv = NULL;
NSIspDrv_FrmB::IspDrv*         NSDrvCam_FrmB::IspDrv_B::m_pIspDrv = NULL;
isp_reg_t*      IspDrv_B::m_pIspReg;


};



#if defined(USING_MTK_LDVT)
    //debug path, imagio will config all engine by CQ
    default_func    IspDrv_B::default_setting_function[] = {NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,IspDrv_B::cam_isp_cfa_cfg,NULL,IspDrv_B::cam_isp_g2g_cfg,
                                                            IspDrv_B::cam_isp_dgm_cfg,NULL,NULL,NULL,
                                                            IspDrv_B::cam_isp_mfb_cfg,IspDrv_B::cam_isp_c24_cfg,NULL,NULL};
#else
    #if 1
    default_func    IspDrv_B::default_setting_function[] = {NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL};

    #else
    //MP path, do not config engine by imageio
    default_func    IspDrv_B::default_setting_function[] = {NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,IspDrv_B::cam_isp_g2g_cfg,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,IspDrv_B::cam_isp_c24_cfg,NULL,NULL};
    #endif
#endif


/*** isp module default setting ***/
MINT32
IspDrv_B::
cam_isp_hrz_cfg(IspDrv* pCurDrv,MINT32 iHeight,MINT32 resize,MINT32 oSize)
{
    ISP_FUNC_DBG("hrz reg: 0x%x,0x%x,0x%x\n",iHeight,resize,oSize);

/*
HRZ_INPUT_HEIGHT:
    "HRZ input size
    if (tg_sel = 0) height=image0
    else height = image1"

HRZ_RESIZE:
    "HRZ resize value
    Get from 32768/decimation ratio
    decimation x1.0: hrz_resize should be 32768
    decimation x0.5: hrz_resize should be 65536
    decimation x0.25: hrz_resize should be 131072 "

HRZ_OUTSIZE:
    "HRZ output size
    And this value get from
    X1 = input size * 32768 / hrz_resize
    hbn_outsize = 2* (X1>>1)"
*/
    //m_pIspDrv->writeReg(0x00004580, 0x000116E2);   /* 0x15004580: CAM_HRZ_RES */
    //m_pIspDrv->writeReg(0x00004584, 0x00080040);   /* 0x15004584: CAM_HRZ_OUT */
        ISP_WRITE_BITS(pCurDrv, CAM_HRZ_RES,HRZ_INPUT_HEIGHT ,iHeight,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(pCurDrv, CAM_HRZ_RES,HRZ_RESIZE, resize,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(pCurDrv, CAM_HRZ_OUT,HRZ_OUTSIZE, oSize,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(pCurDrv, CAM_HRZ_OUT,HRZ_BPCTH, 0,ISP_DRV_USER_ISPF);

    return 0;
}

int
IspDrv_B::
cam_isp_cfa_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+");
    isp_reg_t ispReg;

    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_CFA){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
                ISP_FUNC_DBG("[CFA](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    } else {
        ISP_FUNC_DBG("cam_isp_cfa_cfg m_pIspDrv(0x%x)\n",m_pP2IspDrv);
        m_pP2IspDrv->writeReg(0x000048A0, 0x00000000, ISP_DRV_USER_ISPF, 0);    /* 0x150048A0: CAM_CFA_BYPASS */
        m_pP2IspDrv->writeReg(0x000048A4, 0x00000C03, ISP_DRV_USER_ISPF, 0);    /* 0x150048A4: CAM_CFA_ED_F */
        m_pP2IspDrv->writeReg(0x000048A8, 0x01082006, ISP_DRV_USER_ISPF, 0);    /* 0x150048A8: CAM_CFA_ED_NYQ */
        m_pP2IspDrv->writeReg(0x000048AC, 0x80081008, ISP_DRV_USER_ISPF, 0);    /* 0x150048AC: CAM_CFA_ED_STEP */
        m_pP2IspDrv->writeReg(0x000048B0, 0x37084208, ISP_DRV_USER_ISPF, 0);    /* 0x150048B0: CAM_CFA_RGB_HF */
        m_pP2IspDrv->writeReg(0x000048B4, 0x1806200A, ISP_DRV_USER_ISPF, 0);    /* 0x150048B4: CAM_CFA_BW_BB */
        m_pP2IspDrv->writeReg(0x000048B8, 0x0010A020, ISP_DRV_USER_ISPF, 0);    /* 0x150048B8: CAM_CFA_F1_ACT */
        m_pP2IspDrv->writeReg(0x000048BC, 0x0015C020, ISP_DRV_USER_ISPF, 0);    /* 0x150048BC: CAM_CFA_F2_ACT */
        m_pP2IspDrv->writeReg(0x000048C0, 0x0015C040, ISP_DRV_USER_ISPF, 0);    /* 0x150048C0: CAM_CFA_F3_ACT */
        m_pP2IspDrv->writeReg(0x000048C4, 0x00350050, ISP_DRV_USER_ISPF, 0);    /* 0x150048C4: CAM_CFA_F4_ACT */
        m_pP2IspDrv->writeReg(0x000048C8, 0x00A41440, ISP_DRV_USER_ISPF, 0);    /* 0x150048C8: CAM_CFA_F1_L */
        m_pP2IspDrv->writeReg(0x000048CC, 0x00421084, ISP_DRV_USER_ISPF, 0);    /* 0x150048CC: CAM_CFA_F2_L */
        m_pP2IspDrv->writeReg(0x000048D0, 0x01484185, ISP_DRV_USER_ISPF, 0);    /* 0x150048D0: CAM_CFA_F3_L */
        m_pP2IspDrv->writeReg(0x000048D4, 0x00410417, ISP_DRV_USER_ISPF, 0);    /* 0x150048D4: CAM_CFA_F4_L */
        m_pP2IspDrv->writeReg(0x000048D8, 0x000203FF, ISP_DRV_USER_ISPF, 0);    /* 0x150048D8: CAM_CFA_HF_RB */
        m_pP2IspDrv->writeReg(0x000048DC, 0x00000008, ISP_DRV_USER_ISPF, 0);    /* 0x150048DC: CAM_CFA_HF_GAIN */
        m_pP2IspDrv->writeReg(0x000048E0, 0xE0088888, ISP_DRV_USER_ISPF, 0);    /* 0x150048E0: CAM_CFA_HF_COMP */
        m_pP2IspDrv->writeReg(0x000048E4, 0x00000010, ISP_DRV_USER_ISPF, 0);    /* 0x150048E4: CAM_CFA_HF_CORING_TH */
        m_pP2IspDrv->writeReg(0x000048E8, 0x0052A5FF, ISP_DRV_USER_ISPF, 0);    /* 0x150048E8: CAM_CFA_ACT_LUT */
        m_pP2IspDrv->writeReg(0x000048F0, 0x781F55D4, ISP_DRV_USER_ISPF, 0);    /* 0x150048F0: CAM_CFA_SPARE */
        m_pP2IspDrv->writeReg(0x000048F4, 0x00018006, ISP_DRV_USER_ISPF, 0);    /* 0x150048F4: CAM_CFA_BB*/
    }
    return 0;
}
//
int
IspDrv_B::
cam_isp_g2g_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {

    ISP_FUNC_DBG("+,m_pP2IspDrv(0x%08x),cq(%d),subMode(%d),isApplyTuning(0x%08x)",m_pP2IspDrv,imageioPackage.cq, imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
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
                ISP_FUNC_DBG("[G2G](0x%08x,0x%08x)",regsStruct[i].Addr,regsStruct[i].Data);
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    } else {
        isp_reg_t ispReg;
        switch(imageioPackage.subMode){
            case ISP_SUB_MODE_YUV:  // G2G(YUV -> RGB)
                m_pP2IspDrv->writeReg(0x00004920, 0x00000200, ISP_DRV_USER_ISPF, 0);    /* 0x15004920: CAM_G2G_CONV0A */
                m_pP2IspDrv->writeReg(0x00004924, 0x000002CE, ISP_DRV_USER_ISPF, 0);    /* 0x15004924: CAM_G2G_CONV0B */
                m_pP2IspDrv->writeReg(0x00004928, 0x07500200, ISP_DRV_USER_ISPF, 0);    /* 0x15004928: CAM_G2G_CONV1A */
                m_pP2IspDrv->writeReg(0x0000492C, 0x00000692, ISP_DRV_USER_ISPF, 0);    /* 0x1500492C: CAM_G2G_CONV1B */
                m_pP2IspDrv->writeReg(0x00004930, 0x038B0200, ISP_DRV_USER_ISPF, 0);    /* 0x15004930: CAM_G2G_CONV2A */
                m_pP2IspDrv->writeReg(0x00004934, 0x00000000, ISP_DRV_USER_ISPF, 0);    /* 0x15004934: CAM_G2G_CONV2B */
                m_pP2IspDrv->writeReg(0x00004938, 0x00000009, ISP_DRV_USER_ISPF, 0);    /* 0x15004938: CAM_G2G_ACC */
                break;
            default: // G2G(RGB -> RGB)
                m_pP2IspDrv->writeReg(0x00004920, 0x00000200, ISP_DRV_USER_ISPF, 0);    /* 0x15004920: CAM_G2G_CONV0A */
                m_pP2IspDrv->writeReg(0x00004924, 0x00000000, ISP_DRV_USER_ISPF, 0);    /* 0x15004924: CAM_G2G_CONV0B */
                m_pP2IspDrv->writeReg(0x00004928, 0x02000000, ISP_DRV_USER_ISPF, 0);    /* 0x15004928: CAM_G2G_CONV1A */
                m_pP2IspDrv->writeReg(0x0000492C, 0x00000000, ISP_DRV_USER_ISPF, 0);    /* 0x1500492C: CAM_G2G_CONV1B */
                m_pP2IspDrv->writeReg(0x00004930, 0x00000000, ISP_DRV_USER_ISPF, 0);    /* 0x15004930: CAM_G2G_CONV2A */
                m_pP2IspDrv->writeReg(0x00004934, 0x00000200, ISP_DRV_USER_ISPF, 0);    /* 0x15004934: CAM_G2G_CONV2B */
                m_pP2IspDrv->writeReg(0x00004938, 0x00000009, ISP_DRV_USER_ISPF, 0);    /* 0x15004938: CAM_G2G_ACC */
                break;
        };
    }
    return 0;
}
//
int
IspDrv_B::
cam_isp_dgm_cfg(void) {
    #if 0//not support
    ISP_FUNC_DBG("cam_isp_dgm_cfg ");
    m_pIspDrv->writeReg(0x00004E30, 0x00200010, ISP_DRV_USER_ISPF, 0);    /* 0x15004E30: CAM_DGM_B2 */
    m_pIspDrv->writeReg(0x00004E34, 0x00400030, ISP_DRV_USER_ISPF, 0);    /* 0x15004E34: CAM_DGM_B4 */
    m_pIspDrv->writeReg(0x00004E38, 0x00800060, ISP_DRV_USER_ISPF, 0);    /* 0x15004E38: CAM_DGM_B6 */
    m_pIspDrv->writeReg(0x00004E3C, 0x00C000A0, ISP_DRV_USER_ISPF, 0);    /* 0x15004E3C: CAM_DGM_B8 */
    m_pIspDrv->writeReg(0x00004E40, 0x00E000D0, ISP_DRV_USER_ISPF, 0);    /* 0x15004E40: CAM_DGM_B10 */
    m_pIspDrv->writeReg(0x00004E44, 0x000000F0, ISP_DRV_USER_ISPF, 0);    /* 0x15004E44: CAM_DGM_B11 */
    #endif
    return 0;
}
//
int
IspDrv_B::
cam_isp_obc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
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
int
IspDrv_B::
cam_isp_bnr_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
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
    }
    return 0;
}
//
int
IspDrv_B::
cam_isp_lsc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
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
int
IspDrv_B::
cam_isp_sl2_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    /*
    SL2_CENTR_X    =      SL2_CENTR_X_fullSize - crop_initial_x/HRZ_Ratio
    SL2_CENTR_Y    =      SL2_CENTR_Y_fullSize - crop_initial_y
    SL2_RR_0       =      SL2_RR_0
    SL2_RR_1       =      SL2_RR_1
    SL2_RR_2       =      SL2_RR_2
    SL2_HRZ_COMP   =      2048/HRZ_Ratio
    */

    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        MUINT32 sl2CentrXLsc, sl2CentrYLsc, sl2HrzComp;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_CDP_SL2_FEATUREIO){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
            //
            sl2CentrXLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTER_X);
            sl2CentrYLsc = ISP_READ_BITS_NOPROTECT(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTER_Y);
            ISP_FUNC_DBG("[1]sl2CentrXLsc(%d),sl2CentrYLsc(%d)",sl2CentrXLsc,sl2CentrYLsc);
            // calculate the right sl2 data
            if(imageioPackage.hrzRatio){
                sl2CentrXLsc = sl2CentrXLsc-imageioPackage.hrzCropX/imageioPackage.hrzRatio;
                sl2CentrYLsc = sl2CentrYLsc-imageioPackage.hrzCropY;
                sl2HrzComp = 2048 / imageioPackage.hrzRatio;
                ISP_FUNC_DBG("[SL2]sl2CentrXLsc(%d),sl2CentrYLsc(%d),sl2HrzComp(%d),crop(%d,%d)",  \
                    sl2CentrXLsc,sl2CentrYLsc,sl2HrzComp,imageioPackage.hrzCropX,imageioPackage.hrzCropY);
                ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTER_X, sl2CentrXLsc);
                ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_CEN, SL2_CENTER_Y, sl2CentrYLsc);
                ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, sl2HrzComp);
            } else {
                ISP_FUNC_ERR("[Error]hrzRatio=%d",imageioPackage.hrzRatio);
            }
        }
        //
    }
    return 0;
}
//
int
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
//
int
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
//
int
IspDrv_B::
cam_isp_nbc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
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
//
int
IspDrv_B::
cam_isp_pca_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d),eTuningCqFunc2(%d)",tuningPackage.eTuningCqFunc1,tuningPackage.eTuningCqFunc2);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_PCA){
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }

        if(tuningPackage.eTuningCqFunc2==CAM_ISP_PCA_CON){
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
cam_isp_seee_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
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
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_GGMRB){ //(GGM_RB)0x15005000 ~ 0x1500523C
           m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc1,addrOffset,moduleSize);

            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }

        if(tuningPackage.eTuningCqFunc2==CAM_ISP_GGMG){ //(GGM_G) 0x15005240 ~ 0x1500547C
            m_pP2IspDrv->getCqModuleInf(tuningPackage.eTuningCqFunc2,addrOffset,moduleSize);
            ISP_FUNC_DBG("addrOffset(0x%x),moduleSize(%d)",addrOffset,moduleSize);
            for(MUINT32 i=0;i<moduleSize;i++){
                regsStruct[i].Addr = addrOffset;
                regsStruct[i].Data = tuningPackage.pTuningBuf[addrOffset>>2];
                addrOffset += 4;
            }
            ISP_BURST_WRITE_REGS(m_pP2IspDrv, regsStruct, moduleSize);
        }
    } else {
        m_pP2IspDrv->writeReg(0x00005000, 0x3000F400, ISP_DRV_USER_ISPF, 0);   /* 0x15005000: CAM_GGM_RB_GMT[0] */
        m_pP2IspDrv->writeReg(0x00005004, 0x240C5C3D, ISP_DRV_USER_ISPF, 0);   /* 0x15005004: CAM_GGM_RB_GMT[1] */
        m_pP2IspDrv->writeReg(0x00005008, 0x1C154454, ISP_DRV_USER_ISPF, 0);   /* 0x15005008: CAM_GGM_RB_GMT[2] */
        m_pP2IspDrv->writeReg(0x0000500C, 0x181C3865, ISP_DRV_USER_ISPF, 0);   /* 0x1500500C: CAM_GGM_RB_GMT[3] */
        m_pP2IspDrv->writeReg(0x00005010, 0x18223073, ISP_DRV_USER_ISPF, 0);   /* 0x15005010: CAM_GGM_RB_GMT[4] */
        m_pP2IspDrv->writeReg(0x00005014, 0x14282C7F, ISP_DRV_USER_ISPF, 0);   /* 0x15005014: CAM_GGM_RB_GMT[5] */
        m_pP2IspDrv->writeReg(0x00005018, 0x142D288A, ISP_DRV_USER_ISPF, 0);   /* 0x15005018: CAM_GGM_RB_GMT[6] */
        m_pP2IspDrv->writeReg(0x0000501C, 0x14322494, ISP_DRV_USER_ISPF, 0);   /* 0x1500501C: CAM_GGM_RB_GMT[7] */
        m_pP2IspDrv->writeReg(0x00005020, 0x1437249D, ISP_DRV_USER_ISPF, 0);   /* 0x15005020: CAM_GGM_RB_GMT[8] */
        m_pP2IspDrv->writeReg(0x00005024, 0x143C20A6, ISP_DRV_USER_ISPF, 0);   /* 0x15005024: CAM_GGM_RB_GMT[9] */
        m_pP2IspDrv->writeReg(0x00005028, 0x10411CAE, ISP_DRV_USER_ISPF, 0);   /* 0x15005028: CAM_GGM_RB_GMT[10] */
        m_pP2IspDrv->writeReg(0x0000502C, 0x144520B5, ISP_DRV_USER_ISPF, 0);   /* 0x1500502C: CAM_GGM_RB_GMT[11] */
        m_pP2IspDrv->writeReg(0x00005030, 0x104A1CBD, ISP_DRV_USER_ISPF, 0);   /* 0x15005030: CAM_GGM_RB_GMT[12] */
        m_pP2IspDrv->writeReg(0x00005034, 0x104E18C4, ISP_DRV_USER_ISPF, 0);   /* 0x15005034: CAM_GGM_RB_GMT[13] */
        m_pP2IspDrv->writeReg(0x00005038, 0x10521CCA, ISP_DRV_USER_ISPF, 0);   /* 0x15005038: CAM_GGM_RB_GMT[14] */
        m_pP2IspDrv->writeReg(0x0000503C, 0x105618D1, ISP_DRV_USER_ISPF, 0);   /* 0x1500503C: CAM_GGM_RB_GMT[15] */
        m_pP2IspDrv->writeReg(0x00005040, 0x105A18D7, ISP_DRV_USER_ISPF, 0);   /* 0x15005040: CAM_GGM_RB_GMT[16] */
        m_pP2IspDrv->writeReg(0x00005044, 0x105E14DD, ISP_DRV_USER_ISPF, 0);   /* 0x15005044: CAM_GGM_RB_GMT[17] */
        m_pP2IspDrv->writeReg(0x00005048, 0x106218E2, ISP_DRV_USER_ISPF, 0);   /* 0x15005048: CAM_GGM_RB_GMT[18] */
        m_pP2IspDrv->writeReg(0x0000504C, 0x0C6618E8, ISP_DRV_USER_ISPF, 0);   /* 0x1500504C: CAM_GGM_RB_GMT[19] */
        m_pP2IspDrv->writeReg(0x00005050, 0x106914EE, ISP_DRV_USER_ISPF, 0);   /* 0x15005050: CAM_GGM_RB_GMT[20] */
        m_pP2IspDrv->writeReg(0x00005054, 0x106D14F3, ISP_DRV_USER_ISPF, 0);   /* 0x15005054: CAM_GGM_RB_GMT[21] */
        m_pP2IspDrv->writeReg(0x00005058, 0x0C7114F8, ISP_DRV_USER_ISPF, 0);   /* 0x15005058: CAM_GGM_RB_GMT[22] */
        m_pP2IspDrv->writeReg(0x0000505C, 0x107414FD, ISP_DRV_USER_ISPF, 0);   /* 0x1500505C: CAM_GGM_RB_GMT[23] */
        m_pP2IspDrv->writeReg(0x00005060, 0x0C781502, ISP_DRV_USER_ISPF, 0);   /* 0x15005060: CAM_GGM_RB_GMT[24] */
        m_pP2IspDrv->writeReg(0x00005064, 0x107B1107, ISP_DRV_USER_ISPF, 0);   /* 0x15005064: CAM_GGM_RB_GMT[25] */
        m_pP2IspDrv->writeReg(0x00005068, 0x0C7F150B, ISP_DRV_USER_ISPF, 0);   /* 0x15005068: CAM_GGM_RB_GMT[26] */
        m_pP2IspDrv->writeReg(0x0000506C, 0x0C821110, ISP_DRV_USER_ISPF, 0);   /* 0x1500506C: CAM_GGM_RB_GMT[27] */
        m_pP2IspDrv->writeReg(0x00005070, 0x10851514, ISP_DRV_USER_ISPF, 0);   /* 0x15005070: CAM_GGM_RB_GMT[28] */
        m_pP2IspDrv->writeReg(0x00005074, 0x0C891119, ISP_DRV_USER_ISPF, 0);   /* 0x15005074: CAM_GGM_RB_GMT[29] */
        m_pP2IspDrv->writeReg(0x00005078, 0x0C8C111D, ISP_DRV_USER_ISPF, 0);   /* 0x15005078: CAM_GGM_RB_GMT[30] */
        m_pP2IspDrv->writeReg(0x0000507C, 0x108F1521, ISP_DRV_USER_ISPF, 0);   /* 0x1500507C: CAM_GGM_RB_GMT[31] */
        m_pP2IspDrv->writeReg(0x00005080, 0x0C931126, ISP_DRV_USER_ISPF, 0);   /* 0x15005080: CAM_GGM_RB_GMT[32] */
        m_pP2IspDrv->writeReg(0x00005084, 0x0C96112A, ISP_DRV_USER_ISPF, 0);   /* 0x15005084: CAM_GGM_RB_GMT[33] */
        m_pP2IspDrv->writeReg(0x00005088, 0x0C99112E, ISP_DRV_USER_ISPF, 0);   /* 0x15005088: CAM_GGM_RB_GMT[34] */
        m_pP2IspDrv->writeReg(0x0000508C, 0x0C9C1132, ISP_DRV_USER_ISPF, 0);   /* 0x1500508C: CAM_GGM_RB_GMT[35] */
        m_pP2IspDrv->writeReg(0x00005090, 0x0C9F0D36, ISP_DRV_USER_ISPF, 0);   /* 0x15005090: CAM_GGM_RB_GMT[36] */
        m_pP2IspDrv->writeReg(0x00005094, 0x0CA21139, ISP_DRV_USER_ISPF, 0);   /* 0x15005094: CAM_GGM_RB_GMT[37] */
        m_pP2IspDrv->writeReg(0x00005098, 0x0CA5113D, ISP_DRV_USER_ISPF, 0);   /* 0x15005098: CAM_GGM_RB_GMT[38] */
        m_pP2IspDrv->writeReg(0x0000509C, 0x0CA81141, ISP_DRV_USER_ISPF, 0);   /* 0x1500509C: CAM_GGM_RB_GMT[39] */
        m_pP2IspDrv->writeReg(0x000050A0, 0x0CAB0D45, ISP_DRV_USER_ISPF, 0);   /* 0x150050A0: CAM_GGM_RB_GMT[40] */
        m_pP2IspDrv->writeReg(0x000050A4, 0x0CAE1148, ISP_DRV_USER_ISPF, 0);   /* 0x150050A4: CAM_GGM_RB_GMT[41] */
        m_pP2IspDrv->writeReg(0x000050A8, 0x0CB10D4C, ISP_DRV_USER_ISPF, 0);   /* 0x150050A8: CAM_GGM_RB_GMT[42] */
        m_pP2IspDrv->writeReg(0x000050AC, 0x0CB4114F, ISP_DRV_USER_ISPF, 0);   /* 0x150050AC: CAM_GGM_RB_GMT[43] */
        m_pP2IspDrv->writeReg(0x000050B0, 0x0CB70D53, ISP_DRV_USER_ISPF, 0);   /* 0x150050B0: CAM_GGM_RB_GMT[44] */
        m_pP2IspDrv->writeReg(0x000050B4, 0x0CBA1156, ISP_DRV_USER_ISPF, 0);   /* 0x150050B4: CAM_GGM_RB_GMT[45] */
        m_pP2IspDrv->writeReg(0x000050B8, 0x0CBD0D5A, ISP_DRV_USER_ISPF, 0);   /* 0x150050B8: CAM_GGM_RB_GMT[46] */
        m_pP2IspDrv->writeReg(0x000050BC, 0x0CC00D5D, ISP_DRV_USER_ISPF, 0);   /* 0x150050BC: CAM_GGM_RB_GMT[47] */
        m_pP2IspDrv->writeReg(0x000050C0, 0x0CC31160, ISP_DRV_USER_ISPF, 0);   /* 0x150050C0: CAM_GGM_RB_GMT[48] */
        m_pP2IspDrv->writeReg(0x000050C4, 0x08C60D64, ISP_DRV_USER_ISPF, 0);   /* 0x150050C4: CAM_GGM_RB_GMT[49] */
        m_pP2IspDrv->writeReg(0x000050C8, 0x0CC80D67, ISP_DRV_USER_ISPF, 0);   /* 0x150050C8: CAM_GGM_RB_GMT[50] */
        m_pP2IspDrv->writeReg(0x000050CC, 0x0CCB0D6A, ISP_DRV_USER_ISPF, 0);   /* 0x150050CC: CAM_GGM_RB_GMT[51] */
        m_pP2IspDrv->writeReg(0x000050D0, 0x0CCE116D, ISP_DRV_USER_ISPF, 0);   /* 0x150050D0: CAM_GGM_RB_GMT[52] */
        m_pP2IspDrv->writeReg(0x000050D4, 0x0CD10D71, ISP_DRV_USER_ISPF, 0);   /* 0x150050D4: CAM_GGM_RB_GMT[53] */
        m_pP2IspDrv->writeReg(0x000050D8, 0x08D40D74, ISP_DRV_USER_ISPF, 0);   /* 0x150050D8: CAM_GGM_RB_GMT[54] */
        m_pP2IspDrv->writeReg(0x000050DC, 0x0CD60D77, ISP_DRV_USER_ISPF, 0);   /* 0x150050DC: CAM_GGM_RB_GMT[55] */
        m_pP2IspDrv->writeReg(0x000050E0, 0x0CD90D7A, ISP_DRV_USER_ISPF, 0);   /* 0x150050E0: CAM_GGM_RB_GMT[56] */
        m_pP2IspDrv->writeReg(0x000050E4, 0x08DC0D7D, ISP_DRV_USER_ISPF, 0);   /* 0x150050E4: CAM_GGM_RB_GMT[57] */
        m_pP2IspDrv->writeReg(0x000050E8, 0x0CDE0D80, ISP_DRV_USER_ISPF, 0);   /* 0x150050E8: CAM_GGM_RB_GMT[58] */
        m_pP2IspDrv->writeReg(0x000050EC, 0x0CE10D83, ISP_DRV_USER_ISPF, 0);   /* 0x150050EC: CAM_GGM_RB_GMT[59] */
        m_pP2IspDrv->writeReg(0x000050F0, 0x08E40D86, ISP_DRV_USER_ISPF, 0);   /* 0x150050F0: CAM_GGM_RB_GMT[60] */
        m_pP2IspDrv->writeReg(0x000050F4, 0x0CE60D89, ISP_DRV_USER_ISPF, 0);   /* 0x150050F4: CAM_GGM_RB_GMT[61] */
        m_pP2IspDrv->writeReg(0x000050F8, 0x0CE9098C, ISP_DRV_USER_ISPF, 0);   /* 0x150050F8: CAM_GGM_RB_GMT[62] */
        m_pP2IspDrv->writeReg(0x000050FC, 0x08EC0D8E, ISP_DRV_USER_ISPF, 0);   /* 0x150050FC: CAM_GGM_RB_GMT[63] */
        m_pP2IspDrv->writeReg(0x00005100, 0x18EE1991, ISP_DRV_USER_ISPF, 0);   /* 0x15005100: CAM_GGM_RB_GMT[64] */
        m_pP2IspDrv->writeReg(0x00005104, 0x14F41597, ISP_DRV_USER_ISPF, 0);   /* 0x15005104: CAM_GGM_RB_GMT[65] */
        m_pP2IspDrv->writeReg(0x00005108, 0x14F9199C, ISP_DRV_USER_ISPF, 0);   /* 0x15005108: CAM_GGM_RB_GMT[66] */
        m_pP2IspDrv->writeReg(0x0000510C, 0x14FE15A2, ISP_DRV_USER_ISPF, 0);   /* 0x1500510C: CAM_GGM_RB_GMT[67] */
        m_pP2IspDrv->writeReg(0x00005110, 0x150315A7, ISP_DRV_USER_ISPF, 0);   /* 0x15005110: CAM_GGM_RB_GMT[68] */
        m_pP2IspDrv->writeReg(0x00005114, 0x150819AC, ISP_DRV_USER_ISPF, 0);   /* 0x15005114: CAM_GGM_RB_GMT[69] */
        m_pP2IspDrv->writeReg(0x00005118, 0x150D15B2, ISP_DRV_USER_ISPF, 0);   /* 0x15005118: CAM_GGM_RB_GMT[70] */
        m_pP2IspDrv->writeReg(0x0000511C, 0x151215B7, ISP_DRV_USER_ISPF, 0);   /* 0x1500511C: CAM_GGM_RB_GMT[71] */
        m_pP2IspDrv->writeReg(0x00005120, 0x151715BC, ISP_DRV_USER_ISPF, 0);   /* 0x15005120: CAM_GGM_RB_GMT[72] */
        m_pP2IspDrv->writeReg(0x00005124, 0x111C11C1, ISP_DRV_USER_ISPF, 0);   /* 0x15005124: CAM_GGM_RB_GMT[73] */
        m_pP2IspDrv->writeReg(0x00005128, 0x152015C5, ISP_DRV_USER_ISPF, 0);   /* 0x15005128: CAM_GGM_RB_GMT[74] */
        m_pP2IspDrv->writeReg(0x0000512C, 0x152515CA, ISP_DRV_USER_ISPF, 0);   /* 0x1500512C: CAM_GGM_RB_GMT[75] */
        m_pP2IspDrv->writeReg(0x00005130, 0x152A15CF, ISP_DRV_USER_ISPF, 0);   /* 0x15005130: CAM_GGM_RB_GMT[76] */
        m_pP2IspDrv->writeReg(0x00005134, 0x112F11D4, ISP_DRV_USER_ISPF, 0);   /* 0x15005134: CAM_GGM_RB_GMT[77] */
        m_pP2IspDrv->writeReg(0x00005138, 0x153315D8, ISP_DRV_USER_ISPF, 0);   /* 0x15005138: CAM_GGM_RB_GMT[78] */
        m_pP2IspDrv->writeReg(0x0000513C, 0x153815DD, ISP_DRV_USER_ISPF, 0);   /* 0x1500513C: CAM_GGM_RB_GMT[79] */
        m_pP2IspDrv->writeReg(0x00005140, 0x113D11E2, ISP_DRV_USER_ISPF, 0);   /* 0x15005140: CAM_GGM_RB_GMT[80] */
        m_pP2IspDrv->writeReg(0x00005144, 0x154115E6, ISP_DRV_USER_ISPF, 0);   /* 0x15005144: CAM_GGM_RB_GMT[81] */
        m_pP2IspDrv->writeReg(0x00005148, 0x154611EB, ISP_DRV_USER_ISPF, 0);   /* 0x15005148: CAM_GGM_RB_GMT[82] */
        m_pP2IspDrv->writeReg(0x0000514C, 0x114B11EF, ISP_DRV_USER_ISPF, 0);   /* 0x1500514C: CAM_GGM_RB_GMT[83] */
        m_pP2IspDrv->writeReg(0x00005150, 0x154F15F3, ISP_DRV_USER_ISPF, 0);   /* 0x15005150: CAM_GGM_RB_GMT[84] */
        m_pP2IspDrv->writeReg(0x00005154, 0x115411F8, ISP_DRV_USER_ISPF, 0);   /* 0x15005154: CAM_GGM_RB_GMT[85] */
        m_pP2IspDrv->writeReg(0x00005158, 0x115811FC, ISP_DRV_USER_ISPF, 0);   /* 0x15005158: CAM_GGM_RB_GMT[86] */
        m_pP2IspDrv->writeReg(0x0000515C, 0x155C1200, ISP_DRV_USER_ISPF, 0);   /* 0x1500515C: CAM_GGM_RB_GMT[87] */
        m_pP2IspDrv->writeReg(0x00005160, 0x11611204, ISP_DRV_USER_ISPF, 0);   /* 0x15005160: CAM_GGM_RB_GMT[88] */
        m_pP2IspDrv->writeReg(0x00005164, 0x15651208, ISP_DRV_USER_ISPF, 0);   /* 0x15005164: CAM_GGM_RB_GMT[89] */
        m_pP2IspDrv->writeReg(0x00005168, 0x116A160C, ISP_DRV_USER_ISPF, 0);   /* 0x15005168: CAM_GGM_RB_GMT[90] */
        m_pP2IspDrv->writeReg(0x0000516C, 0x116E1211, ISP_DRV_USER_ISPF, 0);   /* 0x1500516C: CAM_GGM_RB_GMT[91] */
        m_pP2IspDrv->writeReg(0x00005170, 0x15721215, ISP_DRV_USER_ISPF, 0);   /* 0x15005170: CAM_GGM_RB_GMT[92] */
        m_pP2IspDrv->writeReg(0x00005174, 0x11770E19, ISP_DRV_USER_ISPF, 0);   /* 0x15005174: CAM_GGM_RB_GMT[93] */
        m_pP2IspDrv->writeReg(0x00005178, 0x117B121C, ISP_DRV_USER_ISPF, 0);   /* 0x15005178: CAM_GGM_RB_GMT[94] */
        m_pP2IspDrv->writeReg(0x0000517C, 0x157F1220, ISP_DRV_USER_ISPF, 0);   /* 0x1500517C: CAM_GGM_RB_GMT[95] */
        m_pP2IspDrv->writeReg(0x00005180, 0x21842224, ISP_DRV_USER_ISPF, 0);   /* 0x15005180: CAM_GGM_RB_GMT[96] */
        m_pP2IspDrv->writeReg(0x00005184, 0x218C1E2C, ISP_DRV_USER_ISPF, 0);   /* 0x15005184: CAM_GGM_RB_GMT[97] */
        m_pP2IspDrv->writeReg(0x00005188, 0x25942233, ISP_DRV_USER_ISPF, 0);   /* 0x15005188: CAM_GGM_RB_GMT[98] */
        m_pP2IspDrv->writeReg(0x0000518C, 0x219D1E3B, ISP_DRV_USER_ISPF, 0);   /* 0x1500518C: CAM_GGM_RB_GMT[99] */
        m_pP2IspDrv->writeReg(0x00005190, 0x21A51E42, ISP_DRV_USER_ISPF, 0);   /* 0x15005190: CAM_GGM_RB_GMT[100] */
        m_pP2IspDrv->writeReg(0x00005194, 0x21AD1E49, ISP_DRV_USER_ISPF, 0);   /* 0x15005194: CAM_GGM_RB_GMT[101] */
        m_pP2IspDrv->writeReg(0x00005198, 0x21B51E50, ISP_DRV_USER_ISPF, 0);   /* 0x15005198: CAM_GGM_RB_GMT[102] */
        m_pP2IspDrv->writeReg(0x0000519C, 0x21BD1E57, ISP_DRV_USER_ISPF, 0);   /* 0x1500519C: CAM_GGM_RB_GMT[103] */
        m_pP2IspDrv->writeReg(0x000051A0, 0x21C51E5E, ISP_DRV_USER_ISPF, 0);   /* 0x150051A0: CAM_GGM_RB_GMT[104] */
        m_pP2IspDrv->writeReg(0x000051A4, 0x21CD1E65, ISP_DRV_USER_ISPF, 0);   /* 0x150051A4: CAM_GGM_RB_GMT[105] */
        m_pP2IspDrv->writeReg(0x000051A8, 0x21D51A6C, ISP_DRV_USER_ISPF, 0);   /* 0x150051A8: CAM_GGM_RB_GMT[106] */
        m_pP2IspDrv->writeReg(0x000051AC, 0x1DDD1E72, ISP_DRV_USER_ISPF, 0);   /* 0x150051AC: CAM_GGM_RB_GMT[107] */
        m_pP2IspDrv->writeReg(0x000051B0, 0x21E41A79, ISP_DRV_USER_ISPF, 0);   /* 0x150051B0: CAM_GGM_RB_GMT[108] */
        m_pP2IspDrv->writeReg(0x000051B4, 0x21EC1E7F, ISP_DRV_USER_ISPF, 0);   /* 0x150051B4: CAM_GGM_RB_GMT[109] */
        m_pP2IspDrv->writeReg(0x000051B8, 0x1DF41A86, ISP_DRV_USER_ISPF, 0);   /* 0x150051B8: CAM_GGM_RB_GMT[110] */
        m_pP2IspDrv->writeReg(0x000051BC, 0x21FB1A8C, ISP_DRV_USER_ISPF, 0);   /* 0x150051BC: CAM_GGM_RB_GMT[111] */
        m_pP2IspDrv->writeReg(0x000051C0, 0x1E031A92, ISP_DRV_USER_ISPF, 0);   /* 0x150051C0: CAM_GGM_RB_GMT[112] */
        m_pP2IspDrv->writeReg(0x000051C4, 0x220A1A98, ISP_DRV_USER_ISPF, 0);   /* 0x150051C4: CAM_GGM_RB_GMT[113] */
        m_pP2IspDrv->writeReg(0x000051C8, 0x1E121A9E, ISP_DRV_USER_ISPF, 0);   /* 0x150051C8: CAM_GGM_RB_GMT[114] */
        m_pP2IspDrv->writeReg(0x000051CC, 0x22191AA4, ISP_DRV_USER_ISPF, 0);   /* 0x150051CC: CAM_GGM_RB_GMT[115] */
        m_pP2IspDrv->writeReg(0x000051D0, 0x1E211AAA, ISP_DRV_USER_ISPF, 0);   /* 0x150051D0: CAM_GGM_RB_GMT[116] */
        m_pP2IspDrv->writeReg(0x000051D4, 0x1E281AB0, ISP_DRV_USER_ISPF, 0);   /* 0x150051D4: CAM_GGM_RB_GMT[117] */
        m_pP2IspDrv->writeReg(0x000051D8, 0x1E2F1AB6, ISP_DRV_USER_ISPF, 0);   /* 0x150051D8: CAM_GGM_RB_GMT[118] */
        m_pP2IspDrv->writeReg(0x000051DC, 0x223616BC, ISP_DRV_USER_ISPF, 0);   /* 0x150051DC: CAM_GGM_RB_GMT[119] */
        m_pP2IspDrv->writeReg(0x000051E0, 0x1E3E1AC1, ISP_DRV_USER_ISPF, 0);   /* 0x150051E0: CAM_GGM_RB_GMT[120] */
        m_pP2IspDrv->writeReg(0x000051E4, 0x1E451AC7, ISP_DRV_USER_ISPF, 0);   /* 0x150051E4: CAM_GGM_RB_GMT[121] */
        m_pP2IspDrv->writeReg(0x000051E8, 0x1E4C16CD, ISP_DRV_USER_ISPF, 0);   /* 0x150051E8: CAM_GGM_RB_GMT[122] */
        m_pP2IspDrv->writeReg(0x000051EC, 0x1E531AD2, ISP_DRV_USER_ISPF, 0);   /* 0x150051EC: CAM_GGM_RB_GMT[123] */
        m_pP2IspDrv->writeReg(0x000051F0, 0x1E5A16D8, ISP_DRV_USER_ISPF, 0);   /* 0x150051F0: CAM_GGM_RB_GMT[124] */
        m_pP2IspDrv->writeReg(0x000051F4, 0x1E6116DD, ISP_DRV_USER_ISPF, 0);   /* 0x150051F4: CAM_GGM_RB_GMT[125] */
        m_pP2IspDrv->writeReg(0x000051F8, 0x1E681AE2, ISP_DRV_USER_ISPF, 0);   /* 0x150051F8: CAM_GGM_RB_GMT[126] */
        m_pP2IspDrv->writeReg(0x000051FC, 0x1E6F16E8, ISP_DRV_USER_ISPF, 0);   /* 0x150051FC: CAM_GGM_RB_GMT[127] */
        m_pP2IspDrv->writeReg(0x00005200, 0x6E7656ED, ISP_DRV_USER_ISPF, 0);   /* 0x15005200: CAM_GGM_RB_GMT[128] */
        m_pP2IspDrv->writeReg(0x00005204, 0x6E915302, ISP_DRV_USER_ISPF, 0);   /* 0x15005204: CAM_GGM_RB_GMT[129] */
        m_pP2IspDrv->writeReg(0x00005208, 0x6AAC4F16, ISP_DRV_USER_ISPF, 0);   /* 0x15005208: CAM_GGM_RB_GMT[130] */
        m_pP2IspDrv->writeReg(0x0000520C, 0x6AC64F29, ISP_DRV_USER_ISPF, 0);   /* 0x1500520C: CAM_GGM_RB_GMT[131] */
        m_pP2IspDrv->writeReg(0x00005210, 0x6AE04F3C, ISP_DRV_USER_ISPF, 0);   /* 0x15005210: CAM_GGM_RB_GMT[132] */
        m_pP2IspDrv->writeReg(0x00005214, 0x66FA4B4F, ISP_DRV_USER_ISPF, 0);   /* 0x15005214: CAM_GGM_RB_GMT[133] */
        m_pP2IspDrv->writeReg(0x00005218, 0x67134761, ISP_DRV_USER_ISPF, 0);   /* 0x15005218: CAM_GGM_RB_GMT[134] */
        m_pP2IspDrv->writeReg(0x0000521C, 0x672C4772, ISP_DRV_USER_ISPF, 0);   /* 0x1500521C: CAM_GGM_RB_GMT[135] */
        m_pP2IspDrv->writeReg(0x00005220, 0x63454783, ISP_DRV_USER_ISPF, 0);   /* 0x15005220: CAM_GGM_RB_GMT[136] */
        m_pP2IspDrv->writeReg(0x00005224, 0x635D4394, ISP_DRV_USER_ISPF, 0);   /* 0x15005224: CAM_GGM_RB_GMT[137] */
        m_pP2IspDrv->writeReg(0x00005228, 0x637543A4, ISP_DRV_USER_ISPF, 0);   /* 0x15005228: CAM_GGM_RB_GMT[138] */
        m_pP2IspDrv->writeReg(0x0000522C, 0x5F8D43B4, ISP_DRV_USER_ISPF, 0);   /* 0x1500522C: CAM_GGM_RB_GMT[139] */
        m_pP2IspDrv->writeReg(0x00005230, 0x5FA43FC4, ISP_DRV_USER_ISPF, 0);   /* 0x15005230: CAM_GGM_RB_GMT[140] */
        m_pP2IspDrv->writeReg(0x00005234, 0x5FBB3FD3, ISP_DRV_USER_ISPF, 0);   /* 0x15005234: CAM_GGM_RB_GMT[141] */
        m_pP2IspDrv->writeReg(0x00005238, 0x5FD23FE2, ISP_DRV_USER_ISPF, 0);   /* 0x15005238: CAM_GGM_RB_GMT[142] */
        m_pP2IspDrv->writeReg(0x0000523C, 0x5BE93BF1, ISP_DRV_USER_ISPF, 0);   /* 0x1500523C: CAM_GGM_RB_GMT[143] */
        m_pP2IspDrv->writeReg(0x00005300, 0x00008400, ISP_DRV_USER_ISPF, 0);   /* 0x15005300: CAM_GGM_G_GMT[0] */
        m_pP2IspDrv->writeReg(0x00005304, 0x00003C21, ISP_DRV_USER_ISPF, 0);   /* 0x15005304: CAM_GGM_G_GMT[1] */
        m_pP2IspDrv->writeReg(0x00005308, 0x00003030, ISP_DRV_USER_ISPF, 0);   /* 0x15005308: CAM_GGM_G_GMT[2] */
        m_pP2IspDrv->writeReg(0x0000530C, 0x00002C3C, ISP_DRV_USER_ISPF, 0);   /* 0x1500530C: CAM_GGM_G_GMT[3] */
        m_pP2IspDrv->writeReg(0x00005310, 0x00002447, ISP_DRV_USER_ISPF, 0);   /* 0x15005310: CAM_GGM_G_GMT[4] */
        m_pP2IspDrv->writeReg(0x00005314, 0x00002050, ISP_DRV_USER_ISPF, 0);   /* 0x15005314: CAM_GGM_G_GMT[5] */
        m_pP2IspDrv->writeReg(0x00005318, 0x00002058, ISP_DRV_USER_ISPF, 0);   /* 0x15005318: CAM_GGM_G_GMT[6] */
        m_pP2IspDrv->writeReg(0x0000531C, 0x00001C60, ISP_DRV_USER_ISPF, 0);   /* 0x1500531C: CAM_GGM_G_GMT[7] */
        m_pP2IspDrv->writeReg(0x00005320, 0x00001C67, ISP_DRV_USER_ISPF, 0);   /* 0x15005320: CAM_GGM_G_GMT[8] */
        m_pP2IspDrv->writeReg(0x00005324, 0x00001C6E, ISP_DRV_USER_ISPF, 0);   /* 0x15005324: CAM_GGM_G_GMT[9] */
        m_pP2IspDrv->writeReg(0x00005328, 0x00001875, ISP_DRV_USER_ISPF, 0);   /* 0x15005328: CAM_GGM_G_GMT[10] */
        m_pP2IspDrv->writeReg(0x0000532C, 0x0000187B, ISP_DRV_USER_ISPF, 0);   /* 0x1500532C: CAM_GGM_G_GMT[11] */
        m_pP2IspDrv->writeReg(0x00005330, 0x00001881, ISP_DRV_USER_ISPF, 0);   /* 0x15005330: CAM_GGM_G_GMT[12] */
        m_pP2IspDrv->writeReg(0x00005334, 0x00001887, ISP_DRV_USER_ISPF, 0);   /* 0x15005334: CAM_GGM_G_GMT[13] */
        m_pP2IspDrv->writeReg(0x00005338, 0x0000148D, ISP_DRV_USER_ISPF, 0);   /* 0x15005338: CAM_GGM_G_GMT[14] */
        m_pP2IspDrv->writeReg(0x0000533C, 0x00001892, ISP_DRV_USER_ISPF, 0);   /* 0x1500533C: CAM_GGM_G_GMT[15] */
        m_pP2IspDrv->writeReg(0x00005340, 0x00001498, ISP_DRV_USER_ISPF, 0);   /* 0x15005340: CAM_GGM_G_GMT[16] */
        m_pP2IspDrv->writeReg(0x00005344, 0x0000149D, ISP_DRV_USER_ISPF, 0);   /* 0x15005344: CAM_GGM_G_GMT[17] */
        m_pP2IspDrv->writeReg(0x00005348, 0x000014A2, ISP_DRV_USER_ISPF, 0);   /* 0x15005348: CAM_GGM_G_GMT[18] */
        m_pP2IspDrv->writeReg(0x0000534C, 0x000014A7, ISP_DRV_USER_ISPF, 0);   /* 0x1500534C: CAM_GGM_G_GMT[19] */
        m_pP2IspDrv->writeReg(0x00005350, 0x000010AC, ISP_DRV_USER_ISPF, 0);   /* 0x15005350: CAM_GGM_G_GMT[20] */
        m_pP2IspDrv->writeReg(0x00005354, 0x000014B0, ISP_DRV_USER_ISPF, 0);   /* 0x15005354: CAM_GGM_G_GMT[21] */
        m_pP2IspDrv->writeReg(0x00005358, 0x000010B5, ISP_DRV_USER_ISPF, 0);   /* 0x15005358: CAM_GGM_G_GMT[22] */
        m_pP2IspDrv->writeReg(0x0000535C, 0x000014B9, ISP_DRV_USER_ISPF, 0);   /* 0x1500535C: CAM_GGM_G_GMT[23] */
        m_pP2IspDrv->writeReg(0x00005360, 0x000010BE, ISP_DRV_USER_ISPF, 0);   /* 0x15005360: CAM_GGM_G_GMT[24] */
        m_pP2IspDrv->writeReg(0x00005364, 0x000010C2, ISP_DRV_USER_ISPF, 0);   /* 0x15005364: CAM_GGM_G_GMT[25] */
        m_pP2IspDrv->writeReg(0x00005368, 0x000010C6, ISP_DRV_USER_ISPF, 0);   /* 0x15005368: CAM_GGM_G_GMT[26] */
        m_pP2IspDrv->writeReg(0x0000536C, 0x000014CA, ISP_DRV_USER_ISPF, 0);   /* 0x1500536C: CAM_GGM_G_GMT[27] */
        m_pP2IspDrv->writeReg(0x00005370, 0x000010CF, ISP_DRV_USER_ISPF, 0);   /* 0x15005370: CAM_GGM_G_GMT[28] */
        m_pP2IspDrv->writeReg(0x00005374, 0x000010D3, ISP_DRV_USER_ISPF, 0);   /* 0x15005374: CAM_GGM_G_GMT[29] */
        m_pP2IspDrv->writeReg(0x00005378, 0x000010D7, ISP_DRV_USER_ISPF, 0);   /* 0x15005378: CAM_GGM_G_GMT[30] */
        m_pP2IspDrv->writeReg(0x0000537C, 0x00000CDB, ISP_DRV_USER_ISPF, 0);   /* 0x1500537C: CAM_GGM_G_GMT[31] */
        m_pP2IspDrv->writeReg(0x00005380, 0x000010DE, ISP_DRV_USER_ISPF, 0);   /* 0x15005380: CAM_GGM_G_GMT[32] */
        m_pP2IspDrv->writeReg(0x00005384, 0x000010E2, ISP_DRV_USER_ISPF, 0);   /* 0x15005384: CAM_GGM_G_GMT[33] */
        m_pP2IspDrv->writeReg(0x00005388, 0x000010E6, ISP_DRV_USER_ISPF, 0);   /* 0x15005388: CAM_GGM_G_GMT[34] */
        m_pP2IspDrv->writeReg(0x0000538C, 0x00000CEA, ISP_DRV_USER_ISPF, 0);   /* 0x1500538C: CAM_GGM_G_GMT[35] */
        m_pP2IspDrv->writeReg(0x00005390, 0x000010ED, ISP_DRV_USER_ISPF, 0);   /* 0x15005390: CAM_GGM_G_GMT[36] */
        m_pP2IspDrv->writeReg(0x00005394, 0x00000CF1, ISP_DRV_USER_ISPF, 0);   /* 0x15005394: CAM_GGM_G_GMT[37] */
        m_pP2IspDrv->writeReg(0x00005398, 0x000010F4, ISP_DRV_USER_ISPF, 0);   /* 0x15005398: CAM_GGM_G_GMT[38] */
        m_pP2IspDrv->writeReg(0x0000539C, 0x00000CF8, ISP_DRV_USER_ISPF, 0);   /* 0x1500539C: CAM_GGM_G_GMT[39] */
        m_pP2IspDrv->writeReg(0x000053A0, 0x000010FB, ISP_DRV_USER_ISPF, 0);   /* 0x150053A0: CAM_GGM_G_GMT[40] */
        m_pP2IspDrv->writeReg(0x000053A4, 0x00000CFF, ISP_DRV_USER_ISPF, 0);   /* 0x150053A4: CAM_GGM_G_GMT[41] */
        m_pP2IspDrv->writeReg(0x000053A8, 0x00001102, ISP_DRV_USER_ISPF, 0);   /* 0x150053A8: CAM_GGM_G_GMT[42] */
        m_pP2IspDrv->writeReg(0x000053AC, 0x00000D06, ISP_DRV_USER_ISPF, 0);   /* 0x150053AC: CAM_GGM_G_GMT[43] */
        m_pP2IspDrv->writeReg(0x000053B0, 0x00000D09, ISP_DRV_USER_ISPF, 0);   /* 0x150053B0: CAM_GGM_G_GMT[44] */
        m_pP2IspDrv->writeReg(0x000053B4, 0x0000110C, ISP_DRV_USER_ISPF, 0);   /* 0x150053B4: CAM_GGM_G_GMT[45] */
        m_pP2IspDrv->writeReg(0x000053B8, 0x00000D10, ISP_DRV_USER_ISPF, 0);   /* 0x150053B8: CAM_GGM_G_GMT[46] */
        m_pP2IspDrv->writeReg(0x000053BC, 0x00000D13, ISP_DRV_USER_ISPF, 0);   /* 0x150053BC: CAM_GGM_G_GMT[47] */
        m_pP2IspDrv->writeReg(0x000053C0, 0x00000D16, ISP_DRV_USER_ISPF, 0);   /* 0x150053C0: CAM_GGM_G_GMT[48] */
        m_pP2IspDrv->writeReg(0x000053C4, 0x00000D19, ISP_DRV_USER_ISPF, 0);   /* 0x150053C4: CAM_GGM_G_GMT[49] */
        m_pP2IspDrv->writeReg(0x000053C8, 0x00000D1C, ISP_DRV_USER_ISPF, 0);   /* 0x150053C8: CAM_GGM_G_GMT[50] */
        m_pP2IspDrv->writeReg(0x000053CC, 0x0000111F, ISP_DRV_USER_ISPF, 0);   /* 0x150053CC: CAM_GGM_G_GMT[51] */
        m_pP2IspDrv->writeReg(0x000053D0, 0x00000D23, ISP_DRV_USER_ISPF, 0);   /* 0x150053D0: CAM_GGM_G_GMT[52] */
        m_pP2IspDrv->writeReg(0x000053D4, 0x00000D26, ISP_DRV_USER_ISPF, 0);   /* 0x150053D4: CAM_GGM_G_GMT[53] */
        m_pP2IspDrv->writeReg(0x000053D8, 0x00000D29, ISP_DRV_USER_ISPF, 0);   /* 0x150053D8: CAM_GGM_G_GMT[54] */
        m_pP2IspDrv->writeReg(0x000053DC, 0x00000D2C, ISP_DRV_USER_ISPF, 0);   /* 0x150053DC: CAM_GGM_G_GMT[55] */
        m_pP2IspDrv->writeReg(0x000053E0, 0x00000D2F, ISP_DRV_USER_ISPF, 0);   /* 0x150053E0: CAM_GGM_G_GMT[56] */
        m_pP2IspDrv->writeReg(0x000053E4, 0x00000D32, ISP_DRV_USER_ISPF, 0);   /* 0x150053E4: CAM_GGM_G_GMT[57] */
        m_pP2IspDrv->writeReg(0x000053E8, 0x00000D35, ISP_DRV_USER_ISPF, 0);   /* 0x150053E8: CAM_GGM_G_GMT[58] */
        m_pP2IspDrv->writeReg(0x000053EC, 0x00000938, ISP_DRV_USER_ISPF, 0);   /* 0x150053EC: CAM_GGM_G_GMT[59] */
        m_pP2IspDrv->writeReg(0x000053F0, 0x00000D3A, ISP_DRV_USER_ISPF, 0);   /* 0x150053F0: CAM_GGM_G_GMT[60] */
        m_pP2IspDrv->writeReg(0x000053F4, 0x00000D3D, ISP_DRV_USER_ISPF, 0);   /* 0x150053F4: CAM_GGM_G_GMT[61] */
        m_pP2IspDrv->writeReg(0x000053F8, 0x00000D40, ISP_DRV_USER_ISPF, 0);   /* 0x150053F8: CAM_GGM_G_GMT[62] */
        m_pP2IspDrv->writeReg(0x000053FC, 0x00000D43, ISP_DRV_USER_ISPF, 0);   /* 0x150053FC: CAM_GGM_G_GMT[63] */
        m_pP2IspDrv->writeReg(0x00005400, 0x00001546, ISP_DRV_USER_ISPF, 0);   /* 0x15005400: CAM_GGM_G_GMT[64] */
        m_pP2IspDrv->writeReg(0x00005404, 0x0000194B, ISP_DRV_USER_ISPF, 0);   /* 0x15005404: CAM_GGM_G_GMT[65] */
        m_pP2IspDrv->writeReg(0x00005408, 0x00001551, ISP_DRV_USER_ISPF, 0);   /* 0x15005408: CAM_GGM_G_GMT[66] */
        m_pP2IspDrv->writeReg(0x0000540C, 0x00001956, ISP_DRV_USER_ISPF, 0);   /* 0x1500540C: CAM_GGM_G_GMT[67] */
        m_pP2IspDrv->writeReg(0x00005410, 0x0000155C, ISP_DRV_USER_ISPF, 0);   /* 0x15005410: CAM_GGM_G_GMT[68] */
        m_pP2IspDrv->writeReg(0x00005414, 0x00001561, ISP_DRV_USER_ISPF, 0);   /* 0x15005414: CAM_GGM_G_GMT[69] */
        m_pP2IspDrv->writeReg(0x00005418, 0x00001566, ISP_DRV_USER_ISPF, 0);   /* 0x15005418: CAM_GGM_G_GMT[70] */
        m_pP2IspDrv->writeReg(0x0000541C, 0x0000156B, ISP_DRV_USER_ISPF, 0);   /* 0x1500541C: CAM_GGM_G_GMT[71] */
        m_pP2IspDrv->writeReg(0x00005420, 0x00001570, ISP_DRV_USER_ISPF, 0);   /* 0x15005420: CAM_GGM_G_GMT[72] */
        m_pP2IspDrv->writeReg(0x00005424, 0x00001575, ISP_DRV_USER_ISPF, 0);   /* 0x15005424: CAM_GGM_G_GMT[73] */
        m_pP2IspDrv->writeReg(0x00005428, 0x0000157A, ISP_DRV_USER_ISPF, 0);   /* 0x15005428: CAM_GGM_G_GMT[74] */
        m_pP2IspDrv->writeReg(0x0000542C, 0x0000157F, ISP_DRV_USER_ISPF, 0);   /* 0x1500542C: CAM_GGM_G_GMT[75] */
        m_pP2IspDrv->writeReg(0x00005430, 0x00001584, ISP_DRV_USER_ISPF, 0);   /* 0x15005430: CAM_GGM_G_GMT[76] */
        m_pP2IspDrv->writeReg(0x00005434, 0x00001589, ISP_DRV_USER_ISPF, 0);   /* 0x15005434: CAM_GGM_G_GMT[77] */
        m_pP2IspDrv->writeReg(0x00005438, 0x0000158E, ISP_DRV_USER_ISPF, 0);   /* 0x15005438: CAM_GGM_G_GMT[78] */
        m_pP2IspDrv->writeReg(0x0000543C, 0x00001193, ISP_DRV_USER_ISPF, 0);   /* 0x1500543C: CAM_GGM_G_GMT[79] */
        m_pP2IspDrv->writeReg(0x00005440, 0x00001597, ISP_DRV_USER_ISPF, 0);   /* 0x15005440: CAM_GGM_G_GMT[80] */
        m_pP2IspDrv->writeReg(0x00005444, 0x0000159C, ISP_DRV_USER_ISPF, 0);   /* 0x15005444: CAM_GGM_G_GMT[81] */
        m_pP2IspDrv->writeReg(0x00005448, 0x000011A1, ISP_DRV_USER_ISPF, 0);   /* 0x15005448: CAM_GGM_G_GMT[82] */
        m_pP2IspDrv->writeReg(0x0000544C, 0x000015A5, ISP_DRV_USER_ISPF, 0);   /* 0x1500544C: CAM_GGM_G_GMT[83] */
        m_pP2IspDrv->writeReg(0x00005450, 0x000011AA, ISP_DRV_USER_ISPF, 0);   /* 0x15005450: CAM_GGM_G_GMT[84] */
        m_pP2IspDrv->writeReg(0x00005454, 0x000015AE, ISP_DRV_USER_ISPF, 0);   /* 0x15005454: CAM_GGM_G_GMT[85] */
        m_pP2IspDrv->writeReg(0x00005458, 0x000011B3, ISP_DRV_USER_ISPF, 0);   /* 0x15005458: CAM_GGM_G_GMT[86] */
        m_pP2IspDrv->writeReg(0x0000545C, 0x000011B7, ISP_DRV_USER_ISPF, 0);   /* 0x1500545C: CAM_GGM_G_GMT[87] */
        m_pP2IspDrv->writeReg(0x00005460, 0x000015BB, ISP_DRV_USER_ISPF, 0);   /* 0x15005460: CAM_GGM_G_GMT[88] */
        m_pP2IspDrv->writeReg(0x00005464, 0x000011C0, ISP_DRV_USER_ISPF, 0);   /* 0x15005464: CAM_GGM_G_GMT[89] */
        m_pP2IspDrv->writeReg(0x00005468, 0x000011C4, ISP_DRV_USER_ISPF, 0);   /* 0x15005468: CAM_GGM_G_GMT[90] */
        m_pP2IspDrv->writeReg(0x0000546C, 0x000015C8, ISP_DRV_USER_ISPF, 0);   /* 0x1500546C: CAM_GGM_G_GMT[91] */
        m_pP2IspDrv->writeReg(0x00005470, 0x000011CD, ISP_DRV_USER_ISPF, 0);   /* 0x15005470: CAM_GGM_G_GMT[92] */
        m_pP2IspDrv->writeReg(0x00005474, 0x000011D1, ISP_DRV_USER_ISPF, 0);   /* 0x15005474: CAM_GGM_G_GMT[93] */
        m_pP2IspDrv->writeReg(0x00005478, 0x000011D5, ISP_DRV_USER_ISPF, 0);   /* 0x15005478: CAM_GGM_G_GMT[94] */
        m_pP2IspDrv->writeReg(0x0000547C, 0x000011D9, ISP_DRV_USER_ISPF, 0);   /* 0x1500547C: CAM_GGM_G_GMT[95] */
        m_pP2IspDrv->writeReg(0x00005480, 0x000021DD, ISP_DRV_USER_ISPF, 0);   /* 0x15005480: CAM_GGM_G_GMT[96] */
        m_pP2IspDrv->writeReg(0x00005484, 0x000021E5, ISP_DRV_USER_ISPF, 0);   /* 0x15005484: CAM_GGM_G_GMT[97] */
        m_pP2IspDrv->writeReg(0x00005488, 0x000021ED, ISP_DRV_USER_ISPF, 0);   /* 0x15005488: CAM_GGM_G_GMT[98] */
        m_pP2IspDrv->writeReg(0x0000548C, 0x000021F5, ISP_DRV_USER_ISPF, 0);   /* 0x1500548C: CAM_GGM_G_GMT[99] */
        m_pP2IspDrv->writeReg(0x00005490, 0x000021FD, ISP_DRV_USER_ISPF, 0);   /* 0x15005490: CAM_GGM_G_GMT[100] */
        m_pP2IspDrv->writeReg(0x00005494, 0x00002205, ISP_DRV_USER_ISPF, 0);   /* 0x15005494: CAM_GGM_G_GMT[101] */
        m_pP2IspDrv->writeReg(0x00005498, 0x00001E0D, ISP_DRV_USER_ISPF, 0);   /* 0x15005498: CAM_GGM_G_GMT[102] */
        m_pP2IspDrv->writeReg(0x0000549C, 0x00002214, ISP_DRV_USER_ISPF, 0);   /* 0x1500549C: CAM_GGM_G_GMT[103] */
        m_pP2IspDrv->writeReg(0x000054A0, 0x00001E1C, ISP_DRV_USER_ISPF, 0);   /* 0x150054A0: CAM_GGM_G_GMT[104] */
        m_pP2IspDrv->writeReg(0x000054A4, 0x00001E23, ISP_DRV_USER_ISPF, 0);   /* 0x150054A4: CAM_GGM_G_GMT[105] */
        m_pP2IspDrv->writeReg(0x000054A8, 0x0000222A, ISP_DRV_USER_ISPF, 0);   /* 0x150054A8: CAM_GGM_G_GMT[106] */
        m_pP2IspDrv->writeReg(0x000054AC, 0x00001E32, ISP_DRV_USER_ISPF, 0);   /* 0x150054AC: CAM_GGM_G_GMT[107] */
        m_pP2IspDrv->writeReg(0x000054B0, 0x00001E39, ISP_DRV_USER_ISPF, 0);   /* 0x150054B0: CAM_GGM_G_GMT[108] */
        m_pP2IspDrv->writeReg(0x000054B4, 0x00001E40, ISP_DRV_USER_ISPF, 0);   /* 0x150054B4: CAM_GGM_G_GMT[109] */
        m_pP2IspDrv->writeReg(0x000054B8, 0x00001E47, ISP_DRV_USER_ISPF, 0);   /* 0x150054B8: CAM_GGM_G_GMT[110] */
        m_pP2IspDrv->writeReg(0x000054BC, 0x00001E4E, ISP_DRV_USER_ISPF, 0);   /* 0x150054BC: CAM_GGM_G_GMT[111] */
        m_pP2IspDrv->writeReg(0x000054C0, 0x00001A55, ISP_DRV_USER_ISPF, 0);   /* 0x150054C0: CAM_GGM_G_GMT[112] */
        m_pP2IspDrv->writeReg(0x000054C4, 0x00001E5B, ISP_DRV_USER_ISPF, 0);   /* 0x150054C4: CAM_GGM_G_GMT[113] */
        m_pP2IspDrv->writeReg(0x000054C8, 0x00001E62, ISP_DRV_USER_ISPF, 0);   /* 0x150054C8: CAM_GGM_G_GMT[114] */
        m_pP2IspDrv->writeReg(0x000054CC, 0x00001A69, ISP_DRV_USER_ISPF, 0);   /* 0x150054CC: CAM_GGM_G_GMT[115] */
        m_pP2IspDrv->writeReg(0x000054D0, 0x00001E6F, ISP_DRV_USER_ISPF, 0);   /* 0x150054D0: CAM_GGM_G_GMT[116] */
        m_pP2IspDrv->writeReg(0x000054D4, 0x00001E76, ISP_DRV_USER_ISPF, 0);   /* 0x150054D4: CAM_GGM_G_GMT[117] */
        m_pP2IspDrv->writeReg(0x000054D8, 0x00001A7D, ISP_DRV_USER_ISPF, 0);   /* 0x150054D8: CAM_GGM_G_GMT[118] */
        m_pP2IspDrv->writeReg(0x000054DC, 0x00001A83, ISP_DRV_USER_ISPF, 0);   /* 0x150054DC: CAM_GGM_G_GMT[119] */
        m_pP2IspDrv->writeReg(0x000054E0, 0x00001E89, ISP_DRV_USER_ISPF, 0);   /* 0x150054E0: CAM_GGM_G_GMT[120] */
        m_pP2IspDrv->writeReg(0x000054E4, 0x00001A90, ISP_DRV_USER_ISPF, 0);   /* 0x150054E4: CAM_GGM_G_GMT[121] */
        m_pP2IspDrv->writeReg(0x000054E8, 0x00001A96, ISP_DRV_USER_ISPF, 0);   /* 0x150054E8: CAM_GGM_G_GMT[122] */
        m_pP2IspDrv->writeReg(0x000054EC, 0x00001E9C, ISP_DRV_USER_ISPF, 0);   /* 0x150054EC: CAM_GGM_G_GMT[123] */
        m_pP2IspDrv->writeReg(0x000054F0, 0x00001AA3, ISP_DRV_USER_ISPF, 0);   /* 0x150054F0: CAM_GGM_G_GMT[124] */
        m_pP2IspDrv->writeReg(0x000054F4, 0x00001AA9, ISP_DRV_USER_ISPF, 0);   /* 0x150054F4: CAM_GGM_G_GMT[125] */
        m_pP2IspDrv->writeReg(0x000054F8, 0x00001AAF, ISP_DRV_USER_ISPF, 0);   /* 0x150054F8: CAM_GGM_G_GMT[126] */
        m_pP2IspDrv->writeReg(0x000054FC, 0x00001AB5, ISP_DRV_USER_ISPF, 0);   /* 0x150054FC: CAM_GGM_G_GMT[127] */
        m_pP2IspDrv->writeReg(0x00005500, 0x000062BB, ISP_DRV_USER_ISPF, 0);   /* 0x15005500: CAM_GGM_G_GMT[128] */
        m_pP2IspDrv->writeReg(0x00005504, 0x00005ED3, ISP_DRV_USER_ISPF, 0);   /* 0x15005504: CAM_GGM_G_GMT[129] */
        m_pP2IspDrv->writeReg(0x00005508, 0x00005AEA, ISP_DRV_USER_ISPF, 0);   /* 0x15005508: CAM_GGM_G_GMT[130] */
        m_pP2IspDrv->writeReg(0x0000550C, 0x00005B00, ISP_DRV_USER_ISPF, 0);   /* 0x1500550C: CAM_GGM_G_GMT[131] */
        m_pP2IspDrv->writeReg(0x00005510, 0x00005B16, ISP_DRV_USER_ISPF, 0);   /* 0x15005510: CAM_GGM_G_GMT[132] */
        m_pP2IspDrv->writeReg(0x00005514, 0x0000572C, ISP_DRV_USER_ISPF, 0);   /* 0x15005514: CAM_GGM_G_GMT[133] */
        m_pP2IspDrv->writeReg(0x00005518, 0x00005341, ISP_DRV_USER_ISPF, 0);   /* 0x15005518: CAM_GGM_G_GMT[134] */
        m_pP2IspDrv->writeReg(0x0000551C, 0x00005755, ISP_DRV_USER_ISPF, 0);   /* 0x1500551C: CAM_GGM_G_GMT[135] */
        m_pP2IspDrv->writeReg(0x00005520, 0x00004F6A, ISP_DRV_USER_ISPF, 0);   /* 0x15005520: CAM_GGM_G_GMT[136] */
        m_pP2IspDrv->writeReg(0x00005524, 0x0000537D, ISP_DRV_USER_ISPF, 0);   /* 0x15005524: CAM_GGM_G_GMT[137] */
        m_pP2IspDrv->writeReg(0x00005528, 0x00004F91, ISP_DRV_USER_ISPF, 0);   /* 0x15005528: CAM_GGM_G_GMT[138] */
        m_pP2IspDrv->writeReg(0x0000552C, 0x00004FA4, ISP_DRV_USER_ISPF, 0);   /* 0x1500552C: CAM_GGM_G_GMT[139] */
        m_pP2IspDrv->writeReg(0x00005530, 0x00004FB7, ISP_DRV_USER_ISPF, 0);   /* 0x15005530: CAM_GGM_G_GMT[140] */
        m_pP2IspDrv->writeReg(0x00005534, 0x00004BCA, ISP_DRV_USER_ISPF, 0);   /* 0x15005534: CAM_GGM_G_GMT[141] */
        m_pP2IspDrv->writeReg(0x00005538, 0x00004BDC, ISP_DRV_USER_ISPF, 0);   /* 0x15005538: CAM_GGM_G_GMT[142] */
        m_pP2IspDrv->writeReg(0x0000553C, 0x000047EE, ISP_DRV_USER_ISPF, 0);   /* 0x1500553C: CAM_GGM_G_GMT[143] */
        m_pP2IspDrv->writeReg(0x00005600, 0x00000001, ISP_DRV_USER_ISPF, 0);   /* 0x15005600: CAM_GGM_CTRL */
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
cam_isp_g2c_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage) {
    ISP_FUNC_DBG("+,cq(%d),subMode(%d),isApplyTuning(%d)",imageioPackage.cq,imageioPackage.subMode,imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning){
        ISP_DRV_REG_IO_STRUCT regsStruct[ISP_DRV_MAX_CQ_GROUP_SIZE];
        MUINT32 addrOffset, moduleSize;
        //
        ISP_FUNC_DBG("eTuningCqFunc1(%d)",tuningPackage.eTuningCqFunc1);
        //
        if(tuningPackage.eTuningCqFunc1==CAM_ISP_G2C){
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
    } else {
        m_pP2IspDrv->writeReg(0x00004A00, 0x012D0099, ISP_DRV_USER_ISPF, 0);    /* 0x15004A00: CAM_G2C_CONV_0A */
        m_pP2IspDrv->writeReg(0x00004A04, 0x0000003A, ISP_DRV_USER_ISPF, 0);    /* 0x15004A04: CAM_G2C_CONV_0B */
        m_pP2IspDrv->writeReg(0x00004A08, 0x075607AA, ISP_DRV_USER_ISPF, 0);    /* 0x15004A08: CAM_G2C_CONV_1A */
        m_pP2IspDrv->writeReg(0x00004A0C, 0x00000100, ISP_DRV_USER_ISPF, 0);    /* 0x15004A0C: CAM_G2C_CONV_1B */
        m_pP2IspDrv->writeReg(0x00004A10, 0x072A0100, ISP_DRV_USER_ISPF, 0);    /* 0x15004A10: CAM_G2C_CONV_2A */
        m_pP2IspDrv->writeReg(0x00004A14, 0x000007D6, ISP_DRV_USER_ISPF, 0);    /* 0x15004A14: CAM_G2C_CONV_2B */
    }

    return 0;
}
//
int
IspDrv_B::
cam_isp_c42_cfg(void) {
    ISP_FUNC_DBG("cam_isp_c42_cfg m_pIspDrv(0x%x)\n",m_pIspDrv);
    m_pP2IspDrv->writeReg(0x00004A1C, 0x00000000, ISP_DRV_USER_ISPF, 0);    /* 0x15004A1C: CAM_C42_CON */
    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    IspDrvShellImp
/////////////////////////////////////////////////////////////////////////////*/
namespace NSDrvCam_FrmB {
class IspDrvShellImp:public IspDrvShell
{
    public:
        static IspDrvShell* getInstance(NSImageio_FrmB::NSIspio_FrmB::EDrvScenario eScenarioID = NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC);
        virtual void        destroyInstance(void);
        virtual MBOOL       init(const char* userName="");
        virtual MBOOL       uninit(const char* userName="");
        virtual MBOOL       controlBurstQueBuffer(EIspControlBufferCmd cmd, MUINT32 burstQueueNum);
        virtual MBOOL       updateBurstQueueNum(MINT32 burstQNum);

    public://driver object operation
        inline virtual NSIspDrv_FrmB::IspDrv*      getPhyIspDrv(){return m_pPhyIspDrv_bak; }

    public://phy<->virt ISP switch
        inline MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 P1CqD,MINT32 burstQIdx,MINT32 P2DupCqIdx);
        inline NSIspDrv_FrmB::IspDrv*     ispDrvGetCQIspDrv(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx);
    public://commandQ operation
        virtual MBOOL       cam_cq_cfg(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx,MINT32 isP2IspOnly=0);
        virtual MBOOL       cqAddModule(NSIspDrv_FrmB::ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, NSIspDrv_FrmB::CAM_MODULE_ENUM moduleId);
        virtual MBOOL       cqDelModule(NSIspDrv_FrmB::ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, NSIspDrv_FrmB::CAM_MODULE_ENUM moduleId);
        virtual int         getCqModuleInfo(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId);
        virtual MBOOL       setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);
    protected:
        volatile MINT32 mInitCount;
        volatile bool m_fgIsGdmaMode;
        MTKM4UDrv*  mpM4UDrv;
};
    //tpipe
    IMEM_BUF_INFO  tpipeBufInfo[ISP_DVR_MAX_BURST_QUEUE_NUM][ISP_DRV_P2_CQ_NUM][ISP_DRV_P2_CQ_DUPLICATION_NUM];
}; //NSDrvCam_FrmB
//
IspDrvShell* IspDrvShell::createInstance(NSImageio_FrmB::NSIspio_FrmB::EDrvScenario eScenarioID)
{
    DBG_LOG_CONFIG(imageio, function);
//    ISP_FUNC_DBG("");
    ISP_FUNC_INF("eScenarioID: %d.", eScenarioID);
    return IspDrvShellImp::getInstance(eScenarioID);
}
//
IspDrvShell*
IspDrvShellImp::
getInstance(NSImageio_FrmB::NSIspio_FrmB::EDrvScenario eScenarioID)
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
stISP_BUF_LIST ISP_BUF_CTRL::m_swbufL[_rt_dma_max_];
stISP_BUF_LIST ISP_BUF_CTRL::m_p2HwBufL[ISP_DRV_P2_CQ_NUM][ISP_DRV_P2_CQ_DUPLICATION_NUM][_rt_dma_max_];

//static ISP_RAW_CENTRAL_CTRL_INFO rawCentralCtrl;

//
MBOOL
IspDrvShellImp::
init(const char* userName)
{
int ret = 0;
    Mutex::Autolock lock(mLock);

    ISP_FUNC_INF("IspDrvShellImp:mInitCount(%d)", mInitCount);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        return MTRUE;
    }

//    memset (&rawCentralCtrl, 0, sizeof(ISP_RAW_CENTRAL_CTRL_INFO));
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
     isp driver
    =============================================*/
    m_pPhyIspDrv = NSIspDrv_FrmB::IspDrv::createInstance();
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
        cdp driver
    =============================================*/
    m_pP2CdrzDrv = CdpDrv::CreateInstance(m_fgIsGdmaMode);
    ISP_FUNC_DBG("[m_pP2CrzDrv]:0x%08x ",m_pP2CdrzDrv);
    if ( NULL == m_pP2CdrzDrv ) {
        ISP_FUNC_ERR("CdpDrv::CreateInstance cfail ");
        return -1;
    }
    m_pP2CdrzDrv->Init();
    m_pP2CdrzDrv->SetIspReg(m_pVirtIspReg[ISP_DRV_CQ01]);  // set a point for cdp CheckReady() of CalAlgoAndCStep()

    /*============================================
        buffer control
    =============================================*/
    //clear all once
    ISP_FUNC_DBG("clear buffer control ");
    for ( int i =0; i < _rt_dma_max_ ;i++ ) {
        ISP_BUF_CTRL::m_hwbufL[i].bufInfoList.clear();
        ISP_BUF_CTRL::m_swbufL[i].bufInfoList.clear();
    }

    /*============================================
     ispEventThread driver
    =============================================*/
    m_pIspEventThread = IspEventThread::createInstance(m_pPhyIspDrv_bak);
    ISP_FUNC_INF("[m_pIspEventThread]:0x%08x ",m_pIspEventThread);
    if ( NULL == m_pIspEventThread ) {
        ISP_FUNC_ERR("IspEventThread::createInstance fail ");
        return -1;
    }
    m_pIspEventThread->init();
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

    Mutex::Autolock lock(mLock);

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

    /*============================================
    cdp driver release
    =============================================*/
    //CDRZ
    m_pP2CdrzDrv->Uninit();
    m_pP2CdrzDrv->DestroyInstance();
    m_pP2CdrzDrv = NULL;


    /*
    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        //TDRI
        m_pTpipeDrv->uninit();
        m_pTpipeDrv->destroyInstance();
        m_pTpipeDrv = NULL;
    }
    */

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

    //ispEventThread
    m_pIspEventThread->uninit();
    m_pIspEventThread = NULL;
    //
    /*============================================
    isp driver
    =============================================*/
    ISP_FUNC_DBG("m_pIspDrv(0x%x) m_pPhyIspDrv_bak(0x%x)\n",m_pIspDrv,m_pPhyIspDrv_bak);
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

    /*============================================
    virtual isp driver for CQ
    =============================================*/
    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        for (int i = 0; i<ISP_DRV_BASIC_CQ_NUM; i++ ) {
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
            m_pVirtIspDrv[i] = NULL;
            m_pVirtIspReg[i] = NULL;
        }
    }

    //
    m_pIspDrv  = NULL;
    m_pIspReg  = NULL; //mmap
    m_pPhyIspDrv_bak  = NULL;
    m_pPhyIspReg_bak  = NULL;

    ISP_FUNC_INF("mInitCount(%d)",mInitCount);
    return MTRUE;
}


NSIspDrv_FrmB::IspDrv*
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

    //ISP_FUNC_DBG("cq/dupCqIdx [%d]/[%d]/[%d]",cq,cqD,p2DupCqIdx);
    MINT32 index  =0;
    if(cq != CAM_ISP_CQ_NONE)
    {
        index=m_pPhyIspDrv_bak->getRealCQIndex(cq,burstQIdx,p2DupCqIdx);
    }
    MINT32 totalCQNum=m_pPhyIspDrv_bak->cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_GET_TOTALCQNUM);

    if (CAM_ISP_CQ_NONE != cq) {
        if (index >= totalCQNum) {
            ISP_FUNC_ERR("[Error]ispDrvSwitch2Virtual: Invalid CQ(%d). Fail index/total(%d,%d)", cq,index,totalCQNum);
            return MFALSE;
        }
        //m_pIspDrv       = m_pVirtIspDrv[index];
        //
        if(ISP_P1_CQ(cq)) { // for P1
            m_pP1IspDrv       = m_pVirtIspDrv[index];
        } else { // for P2
            m_pP2IspDrv       = m_pVirtIspDrv[index];
            m_pP2CdrzDrv->SetIspReg(m_pVirtIspReg[index]);
            m_pP2CdrzDrv->SetIspDrv(m_pP2IspDrv);
        }

    }

    return MTRUE;
}//

//
/*
MBOOL
IspDrvShellImp::
ispDrvSwitch2Phy()
{
    ISP_FUNC_DBG(":E");
    m_pIspDrv = m_pPhyIspDrv_bak;
    m_pIspReg = m_pPhyIspReg_bak;
    m_pP2CrzDrv->SetIspReg(m_pPhyIspReg_bak);
    ISP_FUNC_DBG(":D");
    return MTRUE;
}//
*/
//
MBOOL
IspDrvShellImp::
cam_cq_cfg(
MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx,MINT32 isP2IspOnly
) {
    ISP_FUNC_DBG(":E cq(%d),isP2IspOnly(%d),burstQIdx(%d),m_trigSsrc(%d)",cq,isP2IspOnly,burstQIdx,this->m_trigSsrc);
    NSIspDrv_FrmB::IspDrv* _targetVirDrv = NULL;
    switch(cq) {
        case NSIspDrv_FrmB::CAM_ISP_CQ0:
            ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0_BASEADDR,\
                (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            ISP_FUNC_DBG("CQ0 addr:0x%x(%d,%d)\n",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),burstQIdx,dupCqIdx);
            //ISP_FUNC_DBG("CAM_ISP_CQ0 IspDrvShellImp:: CQ0C(0x%x)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,dupCqIdx));
            //ISP_FUNC_DBG("CAM_ISP_CQ0 REG(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv_bak, CAM_CTL_CQ0_BASEADDR));
            //
            //m_pPhyIspDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_IMMEDIATE_TRIGGER,CQ_TRIG_BY_START);
            break;
        case NSIspDrv_FrmB::CAM_ISP_CQ0B:
            ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0B_BASEADDR,\
                (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            //
            //m_pPhyIspDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_EVENT_TRIGGER,CQ_TRIG_BY_PASS1_DONE);
            break;
        case NSIspDrv_FrmB::CAM_ISP_CQ0C:
            ISP_FUNC_DBG("CAM_ISP_CQ0C(0x%x),m_trigSsrc(%d)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),this->m_trigSsrc);

            //set base addr in isp_drv cqRingBuf()
            //ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ0C_BASEADDR,\
            //(unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq),ISP_DRV_USER_ISPF);
            //
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                //special case, cause CQ0C trig mode is in the range of cq0 descritpor
                _targetVirDrv = this->ispDrvGetCQIspDrv(NSIspDrv_FrmB::CAM_ISP_CQ0,0, i);
                _targetVirDrv->setCQTriggerMode(NSIspDrv_FrmB::ISP_DRV_CQ0C,NSIspDrv_FrmB::CQ_SINGLE_EVENT_TRIGGER,(NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_SOURCE_ENUM)this->m_trigSsrc);
            }
            m_pPhyIspDrv->setCQTriggerMode(NSIspDrv_FrmB::ISP_DRV_CQ0C,NSIspDrv_FrmB::CQ_SINGLE_EVENT_TRIGGER,(NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_SOURCE_ENUM)this->m_trigSsrc);
            //ISP_FUNC_DBG("CAM_ISP_CQ0C IspDrvShellImp:: CQ0C(0x%x)",m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,dupCqIdx));
            //ISP_FUNC_DBG("CAM_ISP_CQ0C REG(0x%x)",ISP_READ_REG_NOPROTECT(m_pPhyIspDrv_bak, CAM_CTL_CQ0C_BASEADDR));

            break;
        case NSIspDrv_FrmB::CAM_ISP_CQ1:
            if(isP2IspOnly){
                ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ1_BASEADDR,\
                    (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            }
            break;
        case NSIspDrv_FrmB::CAM_ISP_CQ2:
            if(isP2IspOnly){
                ISP_WRITE_REG(m_pPhyIspDrv_bak,CAM_CTL_CQ2_BASEADDR,\
                    (unsigned long)m_pPhyIspDrv_bak->getCQDescBufPhyAddr(cq,burstQIdx,dupCqIdx),ISP_DRV_USER_ISPF);
            }
            break;
        case NSIspDrv_FrmB::CAM_ISP_CQ3:
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
cqAddModule(NSIspDrv_FrmB::ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, NSIspDrv_FrmB::CAM_MODULE_ENUM moduleId)
{
    //ISP_FUNC_DBG(":E m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x) ,%d",m_pIspDrv,m_pPhyIspDrv_bak,moduleId);
    //
    m_pPhyIspDrv->cqAddModule(cq,burstQIdx,dupCqIdx, moduleId);
    //ISP_FUNC_DBG(":X ");
    return MTRUE;
}
//
int
IspDrvShellImp::
getCqModuleInfo(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId)
{
    return m_pPhyIspDrv->getCqModuleInfo( cq,burstQIdx,dupCqIdx, moduleId);
}

//
MBOOL
IspDrvShellImp::
cqDelModule(NSIspDrv_FrmB::ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, NSIspDrv_FrmB::CAM_MODULE_ENUM moduleId)
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
    NSIspDrv_FrmB::IspDrv* _targetVirDrv = NULL;
    ISP_FUNC_DBG("+,m_pPhyIspDrv(0x%x) ",m_pPhyIspDrv_bak);
    //
    m_pPhyIspDrv_bak->setCQTriggerMode((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM) cq,
                                (NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_MODE_ENUM) mode,
                                (NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_SOURCE_ENUM) trig_src);
    if(cq == NSIspDrv_FrmB::CAM_ISP_CQ0){
        for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
            //special case, cause CQ0 trig mode is in the range of cq0 descritpor
            _targetVirDrv = this->ispDrvGetCQIspDrv(NSIspDrv_FrmB::CAM_ISP_CQ0,0, i);
            _targetVirDrv->setCQTriggerMode((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM) cq, \
                        (NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_MODE_ENUM)mode, \
                        (NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_SOURCE_ENUM)trig_src);
        }
    }
    //cq0b only for AE smoothing
    if(cq == NSIspDrv_FrmB::CAM_ISP_CQ0B){
        _targetVirDrv = this->ispDrvGetCQIspDrv(NSIspDrv_FrmB::CAM_ISP_CQ0B,0, 0);
        _targetVirDrv->setCQTriggerMode((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM) cq, \
                    (NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_MODE_ENUM)mode, \
                    (NSIspDrv_FrmB::ISP_DRV_CQ_TRIGGER_SOURCE_ENUM)trig_src);
    }
    ISP_FUNC_DBG("-,");

    return MTRUE;
}
//
//
//this function should be called before isp start
//
MBOOL
IspDrvShellImp::
updateBurstQueueNum(MINT32 burstQNum)
{
     switch(burstQNum)
     {
         case 1:
             //do nothing
             break;
         default:
             ISP_FUNC_ERR("Do not support update burstQ number(%d)>1",burstQNum);
            return MFALSE;
             break;
     }

    MBOOL ret=MTRUE;
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
//*/

//
MBOOL
IspDrvShellImp::
controlBurstQueBuffer(EIspControlBufferCmd cmd, MUINT32 burstQueueNum)
{
    MBOOL ret=MTRUE;
    MUINT32 i, j, k;
    MDPMGR_CFG_STRUCT_FRMB **pDataLayer1;
    MDPMGR_CFG_STRUCT_FRMB *pDataLayer2;

    ISP_FUNC_INF("cmd(%d)",cmd);
    if(eIspControlBufferCmd_Alloc == cmd) {
        /*============================================
         MDP Buffer
        =============================================*/
        gMdpBufInfo_FrmB.size = (burstQueueNum*sizeof(int**)+burstQueueNum*ISP_DRV_P2_CQ_NUM*sizeof(int*) + burstQueueNum*ISP_DRV_P2_CQ_NUM*ISP_DRV_P2_CQ_DUPLICATION_NUM*sizeof(MDPMGR_CFG_STRUCT_FRMB))+(7+1)+0x100;
        ISP_FUNC_DBG("alloc mdpBuf size(%d)",gMdpBufInfo_FrmB.size);
        #if defined(__PMEM_ONLY__)
        gMdpBufInfo_FrmB.virtAddr = (MUINT32*)pmem_alloc_sync( gMdpBufInfo_FrmB.size, &gMdpBufInfo_FrmB.memID);
        #else   // Not PMEM.
        gMdpBufInfo_FrmB.useNoncache = 0; //alloc cacheable mem.
        if ( m_pIMemDrv->allocVirtBuf(&gMdpBufInfo_FrmB) ) {
            ISP_FUNC_ERR("[ERROR]allocVirtBuf(mdpBufInfo) fail,size(0x%x)",gMdpBufInfo_FrmB.size);
        }
        gpMdpMgrCfgData_FrmB = (MDPMGR_CFG_STRUCT_FRMB ***)((gMdpBufInfo_FrmB.virtAddr+7+0x100)&(~7));
        pDataLayer1 = (MDPMGR_CFG_STRUCT_FRMB**)(gpMdpMgrCfgData_FrmB+burstQueueNum);
        pDataLayer2 = (MDPMGR_CFG_STRUCT_FRMB*)(gpMdpMgrCfgData_FrmB+burstQueueNum+ISP_DRV_P2_CQ_NUM*burstQueueNum);
        for(i=0;i<burstQueueNum;i++){
            gpMdpMgrCfgData_FrmB[i] = pDataLayer1;
            for(j=0;j<ISP_DRV_P2_CQ_NUM;j++){
                gpMdpMgrCfgData_FrmB[i][j] = pDataLayer2;
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
                    gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = (MUINT32*)pmem_alloc_sync(MAX_ISP_TILE_TDR_HEX_NO, &tpipeBufInfo[k][i][j].memID);
                    gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa = (MUINT32)pmem_get_phys(tpipeBufInfo[k][i][j].memID);
                    #else   // Not PMEM.
                    //
                    tpipeBufInfo[k][i][j].size = MAX_ISP_TILE_TDR_HEX_NO;
                    tpipeBufInfo[k][i][j].useNoncache = 1; //alloc non-cacheable mem.
                    //
                    if ( m_pIMemDrv->allocVirtBuf(&tpipeBufInfo[k][i][j]) ) {
                        ISP_FUNC_ERR("ERROR:m_pIMemDrv->allocVirtBuf(tpipeBufInfo)");
                    }
                    gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = (MUINT32*)( (tpipeBufInfo[k][i][j].virtAddr+7) & (~7) );
                    //
                    if ( m_pIMemDrv->mapPhyAddr(&tpipeBufInfo[k][i][j]) ) {
                        ISP_FUNC_ERR("ERROR:m_pIMemDrv->mapPhyAddr(tpipeBufInfo)");
                    }
                    //CQ decriptor base address should be 8 bytes alignment
                    gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa =  ( (tpipeBufInfo[k][i][j].phyAddr+7) & (~7) );
                    ISP_FUNC_INF("[Tpipe]i(%d),j(%d)va(0x%08x),pa(0x%08x)",i,j,gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa,gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa);
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
        m_pVirtIspDrv=(NSIspDrv_FrmB::IspDrv**)malloc(totalCQNum*sizeof(NSIspDrv_FrmB::IspDrv *)+totalCQNum*1*sizeof(NSIspDrv_FrmB::IspDrv));
        for( int i=0; i<totalCQNum; i++ )
        {
            m_pVirtIspDrv[i] = ((NSIspDrv_FrmB::IspDrv *)(m_pVirtIspDrv+totalCQNum)) + i*1*sizeof(NSIspDrv_FrmB::IspDrv);
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
            m_pVirtIspDrv[cq] = m_pPhyIspDrv_bak->getCQInstance((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)cq);
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
                        pmem_free((MUINT8*)gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa,(MAX_ISP_TILE_TDR_HEX_NO, tpipeBufInfo[k][i][j].memID);
                        tpipeBufInfo[k][i][j].memID = -1;
                        gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = NULL;
                        gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa = 0;
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

                    gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTableVa = NULL;
                    gpMdpMgrCfgData_FrmB[k][i][j].ispTpipeCfgInfo.drvinfo.tpipeTablePa = 0;

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
        if(gMdpBufInfo_FrmB.memID >= 0) {
            pmem_free((MUINT8*)gMdpBufInfo_FrmB.virtAddr, gMdpBufInfo_FrmB.size, gMdpBufInfo_FrmB.memID);
            gMdpBufInfo_FrmB.memID = -1;
        }
        gpMdpMgrCfgData_FrmB = NULL;
        gMdpBufInfo_FrmB.size = 0;
        #else   // Not PMEM.
        if ( m_pIMemDrv->freeVirtBuf(&gMdpBufInfo_FrmB) ) {
            ISP_FUNC_ERR("[ERROR]freeVirtBuf(mdpBufInfo) fail,size(0x%x)",gMdpBufInfo_FrmB.size);
        }
        gpMdpMgrCfgData_FrmB = NULL;
        gMdpBufInfo_FrmB.size = 0;
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

#if 0 //kk test fix it later
ISP_FUNC_DBG("kk:m_pVirtIspDrv(0x%x)",m_pVirtIspDrv);

        if(m_pVirtIspDrv!=NULL)
        {
            free(m_pVirtIspDrv);
            m_pVirtIspDrv=NULL;
        }

ISP_FUNC_DBG("kk:m_pVirtIspReg(0x%x)",m_pVirtIspReg);
        if(m_pVirtIspReg!=NULL)
        {
            free(m_pVirtIspReg);
            m_pVirtIspReg=NULL;
        }
#endif
    }

    return MTRUE;
}

/*/////////////////////////////////////////////////////////////////////////////
    IspFunction_B
  /////////////////////////////////////////////////////////////////////////////*/
NSIspDrv_FrmB::IspDrv*         IspFunction_B::m_pIspDrv = NULL;
isp_reg_t*      IspFunction_B::m_pIspReg = NULL;
isp_reg_t*      IspFunction_B::m_pPhyIspReg = NULL;


namespace NSDrvCam_FrmB {
unsigned long   IspFunction_B::m_Isp_Top_Reg_Base_Addr = MT6589_ISP_TOP_BASE; /*Use physical base address as default address*/
IspDrvShell*    IspFunction_B::m_pIspDrvShell = NULL;
NSIspDrv_FrmB::IspDrv*         IspFunction_B::m_pP1IspDrv = NULL; // avoid to use this global para, can't used under multi-thread with p1 cmdQCtrl
NSIspDrv_FrmB::IspDrv*         IspFunction_B::m_pP2IspDrv = NULL;
NSIspDrv_FrmB::IspDrv*         IspFunction_B::m_pPhyIspDrv = NULL;
MUINT32         IspFunction_B::m_fps = 30;
};

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
   Mutex::Autolock lock(mLock);

    //ISP_FUNC_DBG("E cq(%d,%d) ",cq, cqD);
    this->m_pIspDrvShell->ispDrvSwitch2Virtual(cq, p1CqD,burstQIdx, P2DupCqIdx);
    //m_pIspDrv  = this->m_pIspDrvShell->m_pIspDrv;
    //m_pCurCQIspReg  = this->m_pIspDrvShell->m_pCurCQIspReg;

    if ( cq==NSIspDrv_FrmB::CAM_ISP_CQ1 || cq==NSIspDrv_FrmB::CAM_ISP_CQ2 || cq==NSIspDrv_FrmB::CAM_ISP_CQ3){
        m_pP2IspDrv  = this->m_pIspDrvShell->m_pP2IspDrv;
    } else if( cq==NSIspDrv_FrmB::CAM_ISP_CQ0 || cq==NSIspDrv_FrmB::CAM_ISP_CQ0B || cq==NSIspDrv_FrmB::CAM_ISP_CQ0C) {
        m_pP1IspDrv  = this->m_pIspDrvShell->m_pP1IspDrv;
    } else {
        ISP_FUNC_ERR("[Error]incorrect parameter cq(%d),p1CqD(%d)",cq,p1CqD);
    }

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
        case NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_VSS:
        case NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC:
            if(isTopEngine==MTRUE){
                tuningEn1Tag = gIspTurningTopTag_cc[subMode].enable1;
                tuningEn2Tag = gIspTurningTopTag_cc[subMode].enable2;
                tuningDmaTag = gIspTurningTopTag_cc[subMode].dma;
            } else {
                tuningEn1Tag = gIspTurningFieldTag_cc[subMode].enable1;
                tuningEn2Tag = gIspTurningFieldTag_cc[subMode].enable2;
                tuningDmaTag = gIspTurningFieldTag_cc[subMode].dma;
            }
            break;
        case NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_IP:
            if(isTopEngine==MTRUE){
                tuningEn1Tag = gIspTurningTopTag_ip[subMode].enable1;
                tuningEn2Tag = gIspTurningTopTag_ip[subMode].enable2;
                tuningDmaTag = gIspTurningTopTag_ip[subMode].dma;
            } else {
                tuningEn1Tag = gIspTurningFieldTag_ip[subMode].enable1;
                tuningEn2Tag = gIspTurningFieldTag_ip[subMode].enable2;
                tuningDmaTag = gIspTurningFieldTag_ip[subMode].dma;
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



//
/*/////////////////////////////////////////////////////////////////////////////
    DMAI_B
  /////////////////////////////////////////////////////////////////////////////*/
int DMAI_B::_config( void )
{
    unsigned long pBase = 0;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MUINT32 mdpmgrIdx = 0;
    NSIspDrv_FrmB::IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQIdx = 0;
    ISP_FUNC_DBG("DMAI(%d),[%s]:cq(%d),pa(0x%08X),ofst(0x%X),size(%d,%d,%d,%d), pixel_byte(%d),cropW(%d,%d,%d,%d),format(0x%x)", \
            this->id(), \
            this->name_Str(), \
            this->CQ, \
            this->dma_cfg.memBuf.base_pAddr, \
            this->dma_cfg.memBuf.ofst_addr,\
            this->dma_cfg.size.w, \
            this->dma_cfg.size.h, \
            this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
            this->dma_cfg.pixel_byte, \
            this->dma_cfg.crop.x, \
            this->dma_cfg.crop.y,\
            this->dma_cfg.crop.w,\
            this->dma_cfg.crop.h,\
            this->dma_cfg.format);
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
    if(this->CQ == CAM_ISP_CQ0){
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
            _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
        else
            _targetDupQIdx = this->dupCqIdx;
    }
    else{
        _targetDupQIdx = this->dupCqIdx;
    }
#else
    _targetDupQIdx = this->dupCqIdx;
#endif

    _targetVirDrv = (NSIspDrv_FrmB::IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,this->dupCqIdx);
     switch(this->id())
     {
        case ISP_DMA_TDRI:
            ISP_WRITE_REG(m_pP2IspDrv, CAM_TDRI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);//baseAddr;
            ISP_FUNC_DBG("CAM_TDRI_BASE_ADDR = 0x%x",this->dma_cfg.memBuf.base_pAddr);
            break;
        case ISP_DMA_CQI:
            if (this->CQ != CAM_ISP_CQ_NONE) {
               this->m_pIspDrvShell->m_trigSsrc = this->cqTrigSrc;
               this->m_pIspDrvShell->cam_cq_cfg(this->CQ,this->burstQueIdx,this->dupCqIdx,this->isP2IspOnly);
            }
            else{
                ISP_FUNC_ERR("unsupported CQ:0x%x\n",this->CQ);
                return -1;
            }
            this->ispDrvSwitch2Virtual(this->CQ, 0,this->burstQueIdx,this->dupCqIdx);
            ISP_FUNC_DBG("[afterCQSwitch]m_pP2IspDrv(0x%x)",m_pP2IspDrv);
           break;
        case ISP_DMA_IMGI:
        ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_BASE_ADDR,(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv,CAM_IMGI_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_IMGI_STRIDE,SWAP, this->dma_cfg.swap,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_IMGI_OFST_ADDR,OFFSET_ADDR, 0,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_IMGI_STRIDE,FORMAT_EN, this->dma_cfg.format_en,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_IMGI_STRIDE,FORMAT, this->dma_cfg.format,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_IMGI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_IMGI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size,ISP_DRV_USER_ISPF);
        ISP_WRITE_BITS(m_pP2IspDrv, CAM_IMGI_STRIDE,STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv, CAM_IMGI_CON, 0x08414140,ISP_DRV_USER_ISPF); // ultra-high cause CQ1/2/3
        ISP_WRITE_REG(m_pP2IspDrv, CAM_IMGI_CON2, 0x00414100,ISP_DRV_USER_ISPF);
            //
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
            // collect src image info for MdpMgr
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcFmt    = this->p2MdpSrcFmt;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcW      = this->p2MdpSrcW;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcH      = this->p2MdpSrcH;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcYStride = this->p2MdpSrcYStride;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcUVStride = this->p2MdpSrcUVStride;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcBufSize = this->p2MdpSrcSize;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcCBufSize = this->p2MdpSrcCSize;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcVBufSize = this->p2MdpSrcVSize;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcPlaneNum = this->p2MdpSrcPlaneNum;
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcVirAddr  = (this->dma_cfg.memBuf.base_vAddr + this->dma_cfg.memBuf.ofst_addr);
            gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcPhyAddr  = (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);

            ISP_FUNC_DBG("[imgi]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);


            ISP_FUNC_DBG("[imgi]p2cq(%d),cq(%d),dupCqIdx(%d),burstQueIdx(%d)",p2Cq,gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].cqIdx,this->dupCqIdx,this->burstQueIdx);

            ISP_FUNC_DBG("[mdp]Fmt(0x%x),W(%d),H(%d),stride(%d,%d),VA(0x%8x),PA(0x%8x),Size(0x%8x)",gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcFmt, \
                        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcW, gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcH, \
                        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcYStride, gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcYStride, \
                        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcVirAddr, \
                        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcPhyAddr, gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].mdpSrcBufSize);

            break;
        case ISP_DMA_LSCI:
            if(this->m_pIspDrvShell == NULL){
                ISP_FUNC_ERR("m_pIspDrvShell is NULL\n");
                return -1;
            }
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv, CAM_LSCI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_LSCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_LSCI_XSIZE, this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_LSCI_YSIZE, this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_BITS(_targetVirDrv, CAM_LSCI_STRIDE,SWAP, this->dma_cfg.swap,ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_LSCI_STRIDE,FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_LSCI_STRIDE,FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_LSCI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_LSCI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_LSCI_STRIDE,STRIDE, this->dma_cfg.size.stride, ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_LSCI_CON, 0x08000010,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_LSCI_CON2, 0x00000000,ISP_DRV_USER_ISPF);
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
            cq_module_id = NSIspDrv_FrmB::CAM_DMA_IMGI;
            break;
        case ISP_DMA_TDRI:
            cq_module_id = NSIspDrv_FrmB::CAM_DMA_TDRI;
            break;
        case ISP_DMA_IMGCI:
            //cq_module_id = CAM_DMA_IMGCI;
            break;
        case ISP_DMA_LSCI:
            cq_module_id = NSIspDrv_FrmB::CAM_DMA_LSCI;
            break;
        case ISP_DMA_CQI:
        default:
            ISP_FUNC_DBG("DMAI: NOT push to CQ ");
            break;
    }

    if (CAM_ISP_CQ_NONE!= this->CQ) {
        myCq = this->CQ;
    }
    if (0xFF != cq_module_id) {
        this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)myCq,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)cq_module_id);
    }
    //
    ISP_FUNC_DBG("-,");
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    DMAO_B
  /////////////////////////////////////////////////////////////////////////////*/
int DMAO_B::_config( void )
{
    //
    NSIspDrv_FrmB::IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQIdx = 0;
    ISP_FUNC_DBG("DMAO(%d),[%s]:cq/dup/burst(%d/%d/%d),pa(0x%08X),ofst(0x%X),size(%d,%d,%d,%d), pixel_byte(%d),cropW(%d,%d,%d,%d),format(0x%x),fps(%d)", \
            this->id(), \
            this->name_Str(), \
            this->CQ, \
            this->dupCqIdx,\
            this->burstQueIdx, \
            this->dma_cfg.memBuf.base_pAddr, \
            this->dma_cfg.memBuf.ofst_addr,\
            this->dma_cfg.size.w, \
            this->dma_cfg.size.h, \
            this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
            this->dma_cfg.pixel_byte, \
            this->dma_cfg.crop.x, \
            this->dma_cfg.crop.y,\
            this->dma_cfg.crop.w,\
            this->dma_cfg.crop.h,\
            this->dma_cfg.format,\
            this->dma_cfg.tgFps);
    ispDrvSwitch2Virtual(this->CQ, 0,this->burstQueIdx, this->dupCqIdx);

#ifdef _PASS1_CQ_CONTINUOUS_MODE_
    if(this->CQ == CAM_ISP_CQ0){
        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
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

    if(this->CQ == CAM_ISP_CQ0){
        if(this->burstQueIdx > 0){
            this->burstQueIdx = 0;
            ISP_FUNC_ERR("pass1 support no burstQNum");
        }
    }

    switch(this->id())
    {
        //p2 ip-raw would use imgo as dma out, and ip-raw would stop pass2 in 82
        case ISP_DMA_IMGO:
            this->m_fps = this->dma_cfg.tgFps;
            _targetVirDrv = (NSIspDrv_FrmB::IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_FUNC_DBG("DAMO-drv(0x%x), x-size(%d),y(%d),stride(%d), baseaddr(0x%x)",_targetVirDrv,this->dma_cfg.size.xsize, this->dma_cfg.size.h, this->dma_cfg.size.stride, this->dma_cfg.memBuf.base_pAddr);
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_YSIZE, this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_STRIDE, this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMGO_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
#if 0
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_CON, 0x08000050,ISP_DRV_USER_ISPF); // ultra-high
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_CON2, 0x00000000,ISP_DRV_USER_ISPF);
#elif 0
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_CON, 0x08010150,ISP_DRV_USER_ISPF); // ultra-highest
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_CON2, 0x00010100,ISP_DRV_USER_ISPF);
#elif 1
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_CON, 0x08100850,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_CON2, 0x00100800,ISP_DRV_USER_ISPF);
#endif
            ISP_WRITE_REG(_targetVirDrv, CAM_IMGO_CROP, (this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            if(this->CQ != CAM_ISP_CQ0){
                //CAM_IMGO_BASE_ADDR only for P2 use (in ip-raw)
                ISP_WRITE_REG(_targetVirDrv,CAM_IMGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            }
            break;
        case ISP_DMA_IMG2O:
            this->m_fps = this->dma_cfg.tgFps;
            _targetVirDrv = (NSIspDrv_FrmB::IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv, CAM_IMG2O_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_IMG2O_YSIZE, this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv, CAM_IMG2O_STRIDE, this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMG2O_STRIDE, FORMAT_EN, this->dma_cfg.format_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMG2O_STRIDE, FORMAT, this->dma_cfg.format, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMG2O_STRIDE, BUS_SIZE_EN, this->dma_cfg.bus_size_en, ISP_DRV_USER_ISPF);
            ISP_WRITE_BITS(_targetVirDrv, CAM_IMG2O_STRIDE, BUS_SIZE, this->dma_cfg.bus_size, ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_IMG2O_CON, 0x08100840,ISP_DRV_USER_ISPF); //ultra-high for pass1, not for 3DNR
            ISP_WRITE_REG(_targetVirDrv, CAM_IMG2O_CON2, 0x00100800,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_IMG2O_CROP, (this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x,ISP_DRV_USER_ISPF);
            if(this->CQ != CAM_ISP_CQ0){
                //for p2
                ISP_WRITE_REG(_targetVirDrv, CAM_IMG2O_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            }
            break;
        case ISP_DMA_ESFKO:
            _targetVirDrv = (NSIspDrv_FrmB::IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,this->dupCqIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_CON2,0x00000000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_CON,0x04000008,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_ESFKO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            break;
        case ISP_DMA_AAO:
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_XSIZE,this->dma_cfg.size.xsize - 1,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_YSIZE,this->dma_cfg.size.h - 1,ISP_DRV_USER_ISPF);//ySize;
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_STRIDE,this->dma_cfg.size.stride,ISP_DRV_USER_ISPF);//stride;
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_CON2,0x00000000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_CON,0x08000020,ISP_DRV_USER_ISPF); // ultra-
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_AAO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr,ISP_DRV_USER_ISPF);
            break;
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
        default:
            ISP_FUNC_ERR("unsupported dmao:0x%x\n",this->id());
            return -1;
            break;
    }

    ISP_FUNC_DBG("DMAO_B::_config:X ");
    return 0;
}

int DMAO_B::_write2CQ(void)
{
    unsigned int cq_module_id = 0xFF;
    unsigned int cq_module_id2 = 0xFF;
    ISP_FUNC_DBG("+,");
    ISP_FUNC_DBG("[id:0X%08X],CQ/dup/burst(%d/%d/%d)",this->id(),this->CQ,this->dupCqIdx,this->burstQueIdx);

     switch(this->id())
     {
        case ISP_DMA_IMGO:
            cq_module_id = NSIspDrv_FrmB::CAM_DMA_IMGO;
            if(this->CQ==ISP_DRV_CQ01 || this->CQ==ISP_DRV_CQ02){
                cq_module_id2 = NSIspDrv_FrmB::CAM_DMA_IMGO_BASEADDR; //ip-raw
                cq_module_id = 0xFF;
            }
            break;
        case ISP_DMA_IMG2O:
            cq_module_id = NSIspDrv_FrmB::CAM_DMA_IMG2O;
            if(this->CQ==ISP_DRV_CQ01 || this->CQ==ISP_DRV_CQ02){
                cq_module_id2 = NSIspDrv_FrmB::CAM_DMA_IMG2O_BASEADDR; //ip-raw
                cq_module_id = 0xFF;
            }
            break;
        case ISP_DMA_EISO:    cq_module_id = CAM_DMA_EISO;
            break;
        case ISP_DMA_AFO:    cq_module_id = CAM_DMA_AFO;
            break;
        case ISP_DMA_AAO:    cq_module_id = CAM_DMA_AAO;
            break;
        case ISP_DMA_ESFKO:    cq_module_id = CAM_DMA_ESFKO;
            break;
        default:
            ISP_FUNC_ERR("unsupported dmao:0x%x\n",this->id());
            return -1;
            break;
    }


    if (0xFF != cq_module_id)
    {
        if(this->CQ == NSIspDrv_FrmB::CAM_ISP_CQ0){
            MUINT32 _targetDupQIdx = 0;

#ifdef _PASS1_CQ_CONTINUOUS_MODE_
            if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN) == 1)
                _targetDupQIdx = (this->dupCqIdx+1)%ISP_DRV_P1_CQ_DUPLICATION_NUM;
            else
                _targetDupQIdx = this->dupCqIdx;
#else
            _targetDupQIdx = this->dupCqIdx;
#endif
            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)cq_module_id);
        }else{
            ISP_FUNC_ERR("unsupported CQ:0x%x\n",this->CQ);
            return -1;
        }
    }
    if (0xFF != cq_module_id2) {
        this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)cq_module_id2);
    }

    ISP_FUNC_DBG("-,");

    return 0;
}

/*
int DMAO_B::_enable( void* pParam )
{
    ISP_FUNC_DBG("DMAO_B::_enable:E ");
    ISP_FUNC_DBG("DMAO_B::_enable:X ");
    return 0;
}

int DMAO_B::_disable( void )
{
    ISP_FUNC_DBG("DMAO_B::_disable:E ");
    ISP_FUNC_DBG("DMAO_B::_disable:X ");
    return 0;
}

int DMAO_B::checkBusy( unsigned long* param  )
{
unsigned long u4Result;
    ISP_FUNC_DBG("DMAO_B::checkBusy:E ");
    ISP_FUNC_DBG("DMAO_B::checkBusy:X ");
    return (u4Result & 0x1 ? 0 : 1);
}
*/
/*/////////////////////////////////////////////////////////////////////////////
    ISP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
#define P1_EN1_MASK    (0x4007B2BD)
#define P1_EN2_MASK    (0x8000C000)
#define P1_DMA_MASK (0x0000042B)
#define P1_FMT_SEL_MASK (0x0000F077)
#define P1_CTL_SEL_MASK    (0x0243F89C)
#define P1_MUX_SEL_MASK (0x01D801CC)
#define P1_MUX_SEL2_MASK (0x70381F5C)
#define P1_MUX_CFG_MASK (0xD00E74F7)
#define P1_PIX_ID_MASK    (0x00000003)
int ISP_TOP_CTRL::pix_id = 0;
MBOOL ISP_TOP_CTRL::bScenario = MFALSE;    //Pass2 can't access these bit until bScenario == MFALSE
MBOOL ISP_TOP_CTRL::bIMG2O_en = MFALSE; //Pass2 can't enable img2o until bIMG2O_en == MFALSE
#define P2_CC_EN1_MASK (0X64F00900)
#define P2_CC_EN2_MASK (0x3882001F)
#define P2_CC_DMA_MASK (0x00000480)    //TODO, should disable img2o bit as default, and judge by p1 dma usage
#define P2_IP_EN1_MASK (0x64F009C8)
#define P2_IP_EN2_MASK (0x3882001F)
#define P2_IP_DMA_MASK (0x00000083)
#define P2_MUX_SEL_MASK (0x06081C04)
#define P2_MUX_SEL2_MASK (0x8237E043)
#define P2_MUX_CFG_MASK (0x010005FF)
#define P2_FMT_SEL_MASK (0x00000F0F)
#define P2_PIX_ID_MASK (0x003F0022)
#define P2_CTL_SEL_MASK (0x00000060)
int ISP_TOP_CTRL::_config( void )
{
    unsigned int reg_tmp, dmaMerg = 0x00;
    unsigned int tuningEn1Tag=0x00, tuningEn2Tag=0x00, tuningDmaTag=0x00; //unable to touch sn
    NSIspDrv_FrmB::IspDrv* _targetVirDrv = NULL;
    char _str[8];
    //
    MUINT32 p2_fmtSel_FnMask=P2_FMT_SEL_MASK;
    MUINT32 p2_enable1_FnMask=P2_CC_EN1_MASK;
    MUINT32 p2_enable2_FnMask=P2_CC_EN2_MASK;
    MUINT32 p2_endma_FnMask=P2_CC_DMA_MASK;
    //

    ISP_FUNC_DBG("ISP_TOP_CTRL:E");
    if(this->path == ISP_PASS1)
        sprintf(_str,"pass1");
    else
        sprintf(_str,"pass2");

    ISP_FUNC_DBG("%s:path(%d),CQ/dup/burst(%d/%d/%d),en1(0x%08x),en2(0x%08x),dma(0x%08x),fmt(0x%08x),ctl(0x%08x),isIspOn(0x%x)", \
                                                              _str,\
                                                              this->path, \
                                                              this->CQ, \
                                                              this->dupCqIdx, \
                                                              this->burstQueIdx, \
                                                              this->en_Top.enable1, \
                                                              this->en_Top.enable2, \
                                                              this->en_Top.dma, \
                                                              this->fmt_sel.reg_val, \
                                                              this->ctl_sel.reg_val, \
                                                              this->isIspOn);
    ISP_FUNC_DBG("int_en(0x%08x),intb_en(0x%08x),intc_en(0x%08x),dma_int(0x%x),dmab_int(0x%x),dmac_int(0x%x),muxS2(0x%x)", \
                                                          this->ctl_int.int_en, \
                                                          this->ctl_int.intb_en, \
                                                          this->ctl_int.intc_en, \
                                                          this->ctl_int.dma_int, \
                                                          this->ctl_int.dmab_int, \
                                                          this->ctl_int.dmac_int, \
                                                          this->ctl_mux_sel2.reg_val);


    _targetVirDrv = (NSIspDrv_FrmB::IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,this->dupCqIdx);

    ISP_FUNC_DBG("m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x),_targetVirDrv(0x%x),m_pP2IspDrv(0x%x)",m_pIspDrv,m_pPhyIspDrv,_targetVirDrv,m_pP2IspDrv);


    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /*** set to physical ***/
    //NOTE!!! should check CQ FIRST, otherwise virtual ISP may be NULL.
    if(CAM_ISP_CQ_NONE != this->CQ)
    {
        //this->ispDrvSwitch2Phy();

        if(ISP_PASS1 == this->path)
        {
            //CQ enable
            if(CAM_CTL_EN2_CQ0_EN & en_Top.enable2)
            {
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_SET, CQ0_EN_SET, 1,ISP_DRV_USER_ISPF);
#if defined(_rtbc_use_cq0c_)
                //enable for rtbc
                en_Top.enable2 |= CAM_CTL_EN2_CQ0C_EN;
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_SET, CQ0C_EN_SET, 1,ISP_DRV_USER_ISPF);
#endif
            }
            else{
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_CLR, CQ0_EN_CLR, 1,ISP_DRV_USER_ISPF);
#if defined(_rtbc_use_cq0c_)
                //enable for rtbc
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_CLR, CQ0C_EN_CLR, 1,ISP_DRV_USER_ISPF);
#endif
            }

            //
            if(CAM_CTL_EN2_CQ0B_EN & en_Top.enable2){
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_SET, CQ0B_EN_SET, 1,ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_CLR, CQ0B_EN_CLR, 1,ISP_DRV_USER_ISPF);
            }
            //
#ifndef _rtbc_use_cq0c_
//if defined _rtbc_use_cq0c_, CQ0C_EN need to be enabled
            if(CAM_CTL_EN2_CQ0C_EN & en_Top.enable2){
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_SET, CQ0C_EN_SET, 1,ISP_DRV_USER_ISPF);
            }else{
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_CLR, CQ0C_EN_CLR, 1,ISP_DRV_USER_ISPF);
            }
#endif
        }
        else /*if (ISP_PASS2 == this->path)*/
        {
            /*** CQ enable should be set to real ***/
            if(this->en_Top.enable2 & CAM_CTL_EN2_CQ1_EN){
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_SET, CQ1_EN_SET, 1,ISP_DRV_USER_ISPF);
            }
            if(this->en_Top.enable2 & CAM_CTL_EN2_CQ2_EN){
                ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_EN2_SET, CQ2_EN_SET, 1,ISP_DRV_USER_ISPF);
            }
            if(this->en_Top.enable2 & CAM_CTL_EN2_CQ3_EN){
                ISP_FUNC_ERR("DO NOT SUPPORT CQ3");
            }
            ISP_WRITE_BITS(m_pPhyIspDrv, CAM_CTL_DMA_EN_SET, IMGI_EN_SET, 1,ISP_DRV_USER_ISPF);
        }
    }

    ISP_FUNC_DBG("pass1: int_en:0x%x,intb_en:0x%x,intc_en:0x%x,dma_int:0x%x,dmab_int:0x%x,dmac_int:0x%x\n",\
        this->ctl_int.int_en,\
        this->ctl_int.intb_en,\
        this->ctl_int.intc_en,\
        this->ctl_int.dma_int,\
        this->ctl_int.dmab_int,\
        this->ctl_int.dmac_int);
    //INT enable
    ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_INT_EN, ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_INT_EN)|this->ctl_int.int_en,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_INTB_EN, ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_INTB_EN)|this->ctl_int.intb_en,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_INTC_EN, ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_INTC_EN)|this->ctl_int.intc_en,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_DMA_INT, ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_DMA_INT)|this->ctl_int.dma_int,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_DMAB_INT, ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_DMAB_INT)|this->ctl_int.dmab_int,ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_DMAC_INT, ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_DMAC_INT)|this->ctl_int.dmac_int,ISP_DRV_USER_ISPF);
    //
    ISP_WRITE_REG(m_pPhyIspDrv, CAM_CTL_SPARE3, CAM_CTL_SPARE3_SET_1,ISP_DRV_USER_ISPF);
    //
    ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_CLK_EN, 0x0001FFFF,ISP_DRV_USER_ISPF);
    ISP_FUNC_DBG("pass1: int_en:0x%x,intb_en:0x%x,intc_en:0x%x,dma_int:0x%x,dmab_int:0x%x,dmac_int:0x%x,en2:0x%x\n", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_INT_EN), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_INTB_EN), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_INTC_EN), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_DMA_INT),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_DMAB_INT),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_DMAC_INT),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN2));

    /********************************************************************************************************************/

    if(ISP_PASS1 == this->path)
    {
        //clk enable
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_CLK_EN, 0x0001FFFF,ISP_DRV_USER_ISPF);
        //continuous mode
        ISP_WRITE_BITS(m_pPhyIspDrv, CAM_TG_VF_CON, SINGLE_MODE, this->b_continuous?0:1, ISP_DRV_USER_ISPF);

        //
        //reset to 0
        //Hw/Sw reset will not clear the 2 registers, it is better to reset MNum here to
        //make sure Kernel ISP_SOF_Buf_Get function can get the valid MNum value
        // for the SOF_0
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_RAW_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);


        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_EN1_CLR,(P1_EN1_MASK&(~this->en_Top.enable1)),ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_EN2_CLR,(P1_EN2_MASK&(~this->en_Top.enable2)),ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_DMA_EN_CLR,(P1_DMA_MASK&(~this->en_Top.dma)),ISP_DRV_USER_ISPF);

        ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_FMT_SEL_CLR,(P1_FMT_SEL_MASK & (~this->fmt_sel.reg_val)),ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_SEL_CLR, (P1_CTL_SEL_MASK & (~this->ctl_sel.reg_val)), ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_MUX_SEL_CLR, (P1_MUX_SEL_MASK & (~this->ctl_mux_sel.reg_val)), ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_MUX_SEL2_CLR, (P1_MUX_SEL2_MASK & (~this->ctl_mux_sel2.reg_val)), ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_SRAM_MUX_CFG_CLR, (P1_MUX_CFG_MASK & (~this->ctl_sram_mux_cfg.reg_val)), ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_PIX_ID_CLR, (P1_PIX_ID_MASK&(~this->pix_id)),ISP_DRV_USER_ISPF);
        ISP_FUNC_DBG("pass1:after clr: en1:0x%x,en2:0x%x,dma_en:0x%x,fmt_sel:0x%x,ctl_sel:0x%x,mux_sel:0x%x,mux_sel2:0x%x,mux_cfg:0x%x, pix_id:0x%x\n", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN1), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN2), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_DMA_EN), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_FMT_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL2),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SRAM_MUX_CFG),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_PIX_ID));
        ////////////////////////////////////////////
        MUINT32 p1_log[6] = {0};
        ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl;
        MUINT32 dummy;
        p1_log[1] = 1;
        p1_log[2] = function_DbgLogEnable_VERBOSE;
        p1_log[3] = function_DbgLogEnable_INFO;//function_DbgLogEnable_DEBUG;
        p1_log[4] = function_DbgLogEnable_INFO;
        p1_log[5] = p1_log[4];
        switch(this->path){
            case ISP_PASS1:
                //clear kernel status to avoid abnormal exit at previous camera operation
                buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR_FRMB;
                buf_ctrl.buf_id = (_isp_dma_enum_)_imgo_;
                buf_ctrl.data_ptr = (ISP_RT_BUF_INFO_STRUCT_FRMB *)&dummy;
                if ( MTRUE != this->m_pPhyIspDrv->rtBufCtrl((void*)&buf_ctrl) ) {
                    ISP_FUNC_ERR("ERROR:rtBufCtrl");
                    return -1;
                }
                buf_ctrl.buf_id = (_isp_dma_enum_)_img2o_;
                if ( MTRUE != this->m_pPhyIspDrv->rtBufCtrl((void*)&buf_ctrl) ) {
                    ISP_FUNC_ERR("ERROR:rtBufCtrl");
                    return -1;
                }
                //init kernel log flg
               p1_log[0] = 0;
               m_pPhyIspDrv->dumpDBGLog(p1_log,NULL);
               //--TG setting
                for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                    _targetVirDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx, i);
                    ISP_WRITE_REG(_targetVirDrv,CAM_CTL_EN1_SET, (P1_EN1_MASK & this->en_Top.enable1),ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_targetVirDrv,CAM_CTL_EN2_SET, (P1_EN2_MASK & this->en_Top.enable2),ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_targetVirDrv,CAM_CTL_DMA_EN_SET, (P1_DMA_MASK & this->en_Top.dma),ISP_DRV_USER_ISPF);

                    ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_FMT_SEL_SET,(P1_FMT_SEL_MASK & this->fmt_sel.reg_val),ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_SEL_SET, (P1_CTL_SEL_MASK & this->ctl_sel.reg_val), ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_MUX_SEL_SET, (P1_MUX_SEL_MASK & this->ctl_mux_sel.reg_val), ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_MUX_SEL2_SET, (P1_MUX_SEL2_MASK & this->ctl_mux_sel2.reg_val), ISP_DRV_USER_ISPF);
                    ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_SRAM_MUX_CFG_SET, (P1_MUX_CFG_MASK & this->ctl_sram_mux_cfg.reg_val), ISP_DRV_USER_ISPF);

                    ISP_WRITE_REG(_targetVirDrv, CAM_CTL_PIX_ID_SET, (P1_PIX_ID_MASK & this->pix_id),ISP_DRV_USER_ISPF);

                    ISP_WRITE_REG(_targetVirDrv,CAM_RAW_MAGIC_NUM0, 0,ISP_DRV_USER_ISPF);
                }
                //
                //avoid p2 access scenario before p1 stop
                //avoid p2 access img2o en bit when img2o occupied by p1
                this->bScenario = MTRUE;
                if(this->en_Top.dma & CAM_CTL_DMA_EN_IMG2O_EN)
                    this->bIMG2O_en = MTRUE;


#ifdef _rtbc_buf_que_2_0_
                {
                    dma_array[_imgo_] = (this->en_Top.dma & CAM_CTL_DMA_EN_IMGO_EN)?(1):(0);
                    dma_array[_img2o_] = (this->en_Top.dma & CAM_CTL_DMA_EN_IMG2O_EN)?(1):(0);
                    ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN_FRMB,_imgo_,0,0,(unsigned char *)dma_array};
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
                }
#endif
                break;

            default:
                ISP_FUNC_ERR("unsupported path:0x%x\n",this->path);
                return -1;
            break;
        }
        for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
            _targetVirDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx, i);
            ISP_FUNC_DBG("Pass1: EN1_SET:0x%x,En2_SET:0x%x,DMA_SET:0x%x,FMT_SEL_SET:0x%x,CTL_SEL_SET:0x%x,MUX_SEL_SET:0x%x,MUX_SEL2_SET:0x%x,MUX_CFG_SET:0x%x,PIX_ID_SET:0x%x\n", \
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_EN1_SET), \
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_EN2_SET), \
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_DMA_EN_SET), \
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_FMT_SEL_SET),\
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_SEL_SET),\
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_MUX_SEL_SET),\
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_MUX_SEL2_SET),\
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_SRAM_MUX_CFG_SET),\
                    ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_PIX_ID_SET) );
        }
           ISP_FUNC_DBG("Pass1: EN1:0x%x,En2:0x%x,DMA:0x%x,FMT_SEL:0x%x,CTL_SEL:0x%x,MUX_SEL:0x%x,MUX_SEL2:0x%x,MUX_CFG:0x%x,PIX_ID:0x%x,IMGO_Addr:0x%x\n", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN1), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN2), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_DMA_EN), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_FMT_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL2),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SRAM_MUX_CFG),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_PIX_ID) ,\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMGO_BASE_ADDR ));
    }
    else /*if (ISP_PASS2 == this->path)*/
    {
        MUINT32 ggmEnBit;

    dmaMerg = en_Top.dma;

    //judge that we do not update p1 usage bits
    //if yes, p2 do not update scenario in fmt_sel
    ISP_FUNC_DBG("bScenario(%d),this->bIMG2O_en(%d)",this->bScenario,this->bIMG2O_en);
    if(this->bScenario == MTRUE){
        //p2 only have to update scenario when ip, and need to wait p1 stop at that scenario
        switch(this->fmt_sel.bit_field.scenario)
        {
            case 3:    //ip
            default:    //others
                ISP_FUNC_ERR("p2 can't access scenario before p1 totally stop\n");
                return -1;
                break;
            case 6:    //cc,n3d_c, do not need to update scenario, use p1 result
                p2_fmtSel_FnMask &= 0x00000F00;
                p2_enable1_FnMask = P2_CC_EN1_MASK;
                p2_enable2_FnMask = P2_CC_EN2_MASK;
                p2_endma_FnMask = P2_CC_DMA_MASK;
                break;
        }

        //if pass1 using img2o, pass2 do not touch

        if(this->bIMG2O_en == MTRUE)
        {
            if((dmaMerg & 0x00000400) >> 10)
            {
                ISP_FUNC_ERR("p2 can't access img2o_dmaMerg(0x%x) when p1 using the port\n",dmaMerg);
                return -1;
            }
            else
            {
                p2_endma_FnMask = p2_endma_FnMask & 0xfffffbff;

            }
        }
    }
    else
    {    //p2 only coud set ip-raw related information after p1 stop
        switch(this->fmt_sel.bit_field.scenario)
        {
            case 3:    //ip
            default:    //others
                p2_fmtSel_FnMask &= P2_FMT_SEL_MASK;
                p2_enable1_FnMask = P2_IP_EN1_MASK;
                p2_enable2_FnMask = P2_IP_EN2_MASK;
                p2_endma_FnMask = P2_IP_DMA_MASK;
                break;
            case 6:    //cc,n3d_c, do not need to update scenario, use p1 result
                p2_fmtSel_FnMask &= P2_FMT_SEL_MASK;
                p2_enable1_FnMask = P2_CC_EN1_MASK;
                p2_enable2_FnMask = P2_CC_EN2_MASK;
                p2_endma_FnMask = P2_CC_DMA_MASK;
                break;
        }
    }

        // GGM control by reg 0x5480
        ggmEnBit = ((this->en_Top.enable1 & CAM_CTL_EN1_GGM_EN)?(0x01):(0x00));
        ISP_WRITE_REG(m_pP2IspDrv, CAM_GGM_CTRL, ggmEnBit, ISP_DRV_USER_ISPF);

       //// clear first for bits not used////
       //clear or bits to cover scenario exchange
           ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_EN1_CLR, (~en_Top.enable1)&(p2_enable1_FnMask),ISP_DRV_USER_ISPF);
           ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_EN2_CLR, (~en_Top.enable2)&(p2_enable2_FnMask & 0x00ffffff),ISP_DRV_USER_ISPF);    //do not clear pass2 cq_en
       ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_DMA_EN_CLR, (~dmaMerg)&(p2_endma_FnMask),ISP_DRV_USER_ISPF);
       ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_FMT_SEL_CLR, (~fmt_sel.reg_val)&(p2_fmtSel_FnMask),ISP_DRV_USER_ISPF);
       ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_SEL_CLR, (~ctl_sel.reg_val)&(P2_CTL_SEL_MASK),ISP_DRV_USER_ISPF);
       ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_MUX_SEL_CLR, (~ctl_mux_sel.reg_val)&(P2_MUX_SEL_MASK),ISP_DRV_USER_ISPF);
       ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_MUX_SEL2_CLR, (~ctl_mux_sel2.reg_val)&(P2_MUX_SEL2_MASK),ISP_DRV_USER_ISPF);
       ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_SRAM_MUX_CFG_CLR, (~ctl_sram_mux_cfg.reg_val)&(P2_MUX_CFG_MASK),ISP_DRV_USER_ISPF);

            //// set en1/en2/dma ////
            ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_EN1_SET, this->en_Top.enable1 & p2_enable1_FnMask, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_EN2_SET, this->en_Top.enable2 & p2_enable2_FnMask, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pP2IspDrv,  CAM_CTL_DMA_EN_SET, dmaMerg& p2_endma_FnMask, ISP_DRV_USER_ISPF);

            ISP_FUNC_INF("[Pass2]AppTurn(%d) EN1-Set(0x%x),Clr(0x%x)-En2:Set(0x%x),Clr(0x%x)-DMA:Set(0x%x),Clr(0x%x)\n", \
                    this->isApplyTuning,ISP_READ_REG_NOPROTECT(m_pP2IspDrv,  CAM_CTL_EN1_SET),ISP_READ_REG_NOPROTECT(m_pP2IspDrv,  CAM_CTL_EN1_CLR), \
                    ISP_READ_REG_NOPROTECT(m_pP2IspDrv,  CAM_CTL_EN2_SET),ISP_READ_REG_NOPROTECT(m_pP2IspDrv,  CAM_CTL_EN2_CLR), \
                    ISP_READ_REG_NOPROTECT(m_pP2IspDrv,  CAM_CTL_DMA_EN_SET),ISP_READ_REG_NOPROTECT(m_pP2IspDrv,  CAM_CTL_DMA_EN_CLR));
    }

    /************************************************
            Commmon part
    ************************************************/
    if(ISP_PASS1 == this->path)
{}
    else{
        _targetVirDrv = (NSIspDrv_FrmB::IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,this->dupCqIdx);

        //or p2 would update scenario
        ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_FMT_SEL_SET, ( fmt_sel.reg_val&p2_fmtSel_FnMask ),ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_SEL_SET, ctl_sel.reg_val&P2_CTL_SEL_MASK, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_MUX_SEL_SET, ctl_mux_sel.reg_val & P2_MUX_SEL_MASK,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_MUX_SEL2_SET, ctl_mux_sel2.reg_val & P2_MUX_SEL2_MASK,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_SRAM_MUX_CFG_SET, ctl_sram_mux_cfg.reg_val & P2_MUX_CFG_MASK,ISP_DRV_USER_ISPF);

        //bit wise
        #if 0  //kk test default:0
        ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_PIX_ID_CLR, (~this->pix_id) & P2_PIX_ID_MASK,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(_targetVirDrv, CAM_CTL_PIX_ID_SET, this->pix_id & P2_PIX_ID_MASK, ISP_DRV_USER_ISPF);
        ISP_FUNC_DBG("pix_id(0x%x),ctl_pix_id(0x%x)",this->pix_id,this->ctl_pix_id);
        #else
        ISP_WRITE_REG(_targetVirDrv,  CAM_CTL_PIX_ID_CLR, (~this->ctl_pix_id) & P2_PIX_ID_MASK,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(_targetVirDrv, CAM_CTL_PIX_ID_SET, this->ctl_pix_id & P2_PIX_ID_MASK, ISP_DRV_USER_ISPF);
        #endif
        //ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_PIX_ID_SET, MDP_SEL_SET, 0x1, ISP_DRV_USER_ISPF); //set from postproc pipe
        ISP_FUNC_DBG("CAM_CTL_PIX_ID_SET(0x%8x),CAM_CTL_PIX_ID(0x%8x)",ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_PIX_ID_SET),ISP_READ_REG_NOPROTECT(_targetVirDrv,  CAM_CTL_PIX_ID));
     }
    //
    ISP_FUNC_DBG("ISP_TOP_CTRL::_config:X ");
    return 0;
}


int ISP_TOP_CTRL::_enable( void* pParam  )
{
    ISP_FUNC_DBG("path(%d),start(%d),m_pIspReg(0x%x),m_pPhyIspReg(0x%x),scenario(%d)\n",
                this->path,*((MUINT32*)pParam),this->m_pIspReg,this->m_pPhyIspReg,this->fmt_sel.bit_field.scenario);

    MUINT32 *start = (MUINT32*)pParam;
    ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl;
    MUINT32 size;
    MINT32 errSta;
    ISP_DRV_WAIT_IRQ_STRUCT irq_TG_ClrDone;
    irq_TG_ClrDone.Clear= ISP_DRV_IRQ_CLEAR_STATUS;
    irq_TG_ClrDone.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT;
    irq_TG_ClrDone.UserInfo.Status=ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    irq_TG_ClrDone.UserInfo.UserKey=0;
    irq_TG_ClrDone.TimeInfo.tLastEvent_sec=0x0;
    irq_TG_ClrDone.TimeInfo.tLastEvent_usec=0x0;
    irq_TG_ClrDone.TimeInfo.tmark2read_sec=0x0;
    irq_TG_ClrDone.TimeInfo.tmark2read_usec=0x0;
    irq_TG_ClrDone.TimeInfo.tevent2read_sec=0x0;
    irq_TG_ClrDone.TimeInfo.tevent2read_usec=0x0;
    irq_TG_ClrDone.TimeInfo.passedbySigcnt=0;
    irq_TG_ClrDone.Timeout=CAM_INT_WAIT_TIMEOUT_MS;
    irq_TG_ClrDone.bDumpReg=0xfe;

    // added for checking CQ status
    MUINT32 CQ_status, CQ_debugset;
    MBOOL readCQstatus = MTRUE;  // check both pass1 and pass2
    Mutex   *_localVar;
    // lock for pass1 and pass2 respectively
    if ( CAM_ISP_PASS2_START == *start || CAM_ISP_PASS2B_START == *start || CAM_ISP_PASS2C_START == *start ){
        _localVar = &this->m_pIspDrvShell->gPass2Lock;
    } else {
        if( ( CAM_ISP_PASS1_START == (*start) ) || ( CAM_ISP_PASS1_CQ0_START == (*start) ) || ( CAM_ISP_PASS1_CQ0B_START == (*start) ))
            _localVar = &this->m_pIspDrvShell->gPass1Lock;
        else{
            ISP_FUNC_ERR("ISP_TOP_CTRL _enalbe fail (0x%x)\n",(*start));
            return -1;
        }
    }
    Mutex::Autolock lock(_localVar);

    if( CAM_ISP_PASS2_START == *start || CAM_ISP_PASS2B_START == *start || CAM_ISP_PASS2C_START == *start )
    {

            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_DBG_SET,0x6000,ISP_DRV_USER_ISPF);

        readCQstatus = MTRUE;
    }

    #if 0    //user take care this
    if (this->m_pIspDrvShell->m_pIMemDrv)    // If this->m_pIspDrvShell->m_pIMemDrv not null (i.e. not GDMA mode). // No IMem in GDMA mode so don't have to flush.
    {
        //m4u flush before HW working
        this->m_pIspDrvShell->m_pIMemDrv->cacheFlushAll();
    }
    #endif

    if( readCQstatus )
    {

            CQ_status = ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_DBG_PORT );
            CQ_debugset = ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_DBG_SET );

    }

    if( CAM_ISP_PASS1_START == (*start) )
    {
        ISP_FUNC_INF("pass1: vf_en enable\n");
        //
        if ( MFALSE == this->m_pPhyIspDrv->waitIrq( &irq_TG_ClrDone )) {
            ISP_FUNC_ERR("clr irq status fail");
            return -1;
        }
        //TG1
        if(CAM_CTL_EN1_TG1_EN & this->en_Top.enable1)
            ISP_WRITE_BITS(m_pPhyIspDrv, CAM_TG_VF_CON, VFDATA_EN, 1, ISP_DRV_USER_ISPF);

        if (MFALSE == this->m_pPhyIspDrv->ISPWakeLockCtrl(MTRUE))
        {
            ISP_FUNC_ERR("ISPWakeLockCtrl enable fail!!");
            return -1;
        }

        ISP_FUNC_DBG("Pass1: EN1:0x%x,En2:0x%x,DMA:0x%x,FMT_SEL:0x%x,CTL_SEL:0x%x,MUX_SEL:0x%x,MUX_SEL2:0x%x,MUX_CFG:0x%x,PIX_ID:0x%x,IMGO_Addr:0x%x,IMGO_XSIZE:0x%x\n", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN1), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN2), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_DMA_EN), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_FMT_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL2),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SRAM_MUX_CFG),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_PIX_ID) ,\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMGO_BASE_ADDR ),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_IMGO_XSIZE ));
    }
    else if(CAM_ISP_PASS2_START == *start)
    {
        if((CQ_status & 0x0000000F) != 0x001)
        {    //multi-thread, support enque next buffer while the previous one is still running
            //ISP_FUNC_WRN("CQ1 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }

       if(!P2_UT_BypassMDP_TPIPE)
       {
            ISP_FUNC_DBG("ccc cqidx(%d)",gpMdpMgrCfgData_FrmB[this->burstQueIdx][ISP_DRV_P2_CQ1][this->dupCqIdx].cqIdx);
            errSta = g_pMdpMgr_FrmB->startMdp(gpMdpMgrCfgData_FrmB[this->burstQueIdx][ISP_DRV_P2_CQ1][this->dupCqIdx]);
            if(MDPMGR_NO_ERROR != errSta){
                ISP_FUNC_ERR("pass2 CQ1 g_pMdpMgr_FrmB->startMdp fail(%d)",errSta);
                return -1;
            }
        }
        else
        {
            pthread_mutex_lock(&p2_enque_deque_Mutex);
            if(!enque_running)
            {
                enque_running=true;
                pthread_mutex_unlock(&p2_enque_deque_Mutex);
            }
            else
            {
                //wait signal to do pass2
                pthread_mutex_unlock(&p2_enque_deque_Mutex);
                ISP_FUNC_INF("wait start Semmmmmmmm");

                int running=true;
                while(running)
                {
                    pthread_mutex_lock(&p2_enque_deque_Mutex);
                    running=enque_running;
                    pthread_mutex_unlock(&p2_enque_deque_Mutex);
                    usleep(500);
                }
                enque_running=true;

            }
            //CHRISTOPHER: write CQ1 base address and trigger pass2 to monitor hw (temp bypass mdp to UT)
            ISP_WRITE_BITS(m_pPhyIspDrv,CAM_CTL_EN2,CQ1_EN,0x1,ISP_DRV_USER_ISPF);
            MUINT32 CQPA=(unsigned long)(this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(this->CQ,this->burstQueIdx,this->dupCqIdx));
            ISP_FUNC_INF("C CQPA(0x%x)",CQPA);
        ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_CQ1_BASEADDR,CQPA,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(m_pPhyIspDrv,CAM_CTL_START,0x1,ISP_DRV_USER_ISPF);
        }
    }
    else if(CAM_ISP_PASS2B_START == *start)
    {
        if((CQ_status & 0x000000F0) != 0x010)
        {    //multi-thread, support enque next buffer while the previous one is still running
            ISP_FUNC_WRN("CQ2 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        errSta = g_pMdpMgr_FrmB->startMdp(gpMdpMgrCfgData_FrmB[this->burstQueIdx][ISP_DRV_P2_CQ2][this->dupCqIdx]);
        if(MDPMGR_NO_ERROR != errSta){
            ISP_FUNC_ERR("pass2 CQ2 g_pMdpMgr_FrmB->startMdp fail(%d)",errSta);
            return -1;
        }
    }
    else if(CAM_ISP_PASS2C_START == *start)
    {
        if((CQ_status & 0x00000F00) != 0x100)
        {
            ISP_FUNC_WRN("CQ3 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        errSta = g_pMdpMgr_FrmB->startMdp(gpMdpMgrCfgData_FrmB[this->burstQueIdx][ISP_DRV_P2_CQ3][this->dupCqIdx]);
        if(MDPMGR_NO_ERROR != errSta){
            ISP_FUNC_ERR("pass2 CQ3 g_pMdpMgr_FrmB->startMdp fail(%d)",errSta);
            return -1;
        }

    }
    else if(CAM_ISP_FMT_START == *start)
    {
        //ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_START, 0x00000008, ISP_DRV_USER_ISPF);
        ISP_FUNC_ERR("start (0%x) fail, unsupported\n");
        return -1;
    }
    else if(CAM_ISP_PASS1_CQ0_START == *start)
    {
        ISP_FUNC_INF("pass1: CQ0 immediately trig\n");
        ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_START, 0x00000020, ISP_DRV_USER_ISPF);

        ISP_FUNC_DBG("Pass1: EN1:0x%x,En2:0x%x,DMA:0x%x,FMT_SEL:0x%x,CTL_SEL:0x%x,MUX_SEL:0x%x,MUX_SEL2:0x%x,MUX_CFG:0x%x,PIX_ID:0x%x,IMGO_Addr:0x%x,IMGO_XSIZE:0x%x\n", \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN1), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_EN2), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_DMA_EN), \
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_FMT_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_MUX_SEL2),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_SRAM_MUX_CFG),\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_CTL_PIX_ID) ,\
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_IMGO_BASE_ADDR ),
                ISP_READ_REG_NOPROTECT(m_pPhyIspDrv,  CAM_IMGO_XSIZE ));
    }
    else if(CAM_ISP_PASS1_CQ0B_START == *start)
    {
            ISP_WRITE_REG(m_pPhyIspDrv,  CAM_CTL_START, 0x00000040, ISP_DRV_USER_ISPF);

    }


#if 1
    //register dump after start()
    if ( MTRUE == function_DbgLogEnable_INFO)
    {
        static int pass2_cnt = 0;

        ISP_FUNC_VRB("dumpReg");
        if (CAM_ISP_PASS1_START == *start)
        {
            usleep(1000);
            //pass1 DMA base_addr
                ISP_FUNC_INF("P1 dma_en(0x%x),addr(0x%x)(0x%x)(0x%x)(0x%x)", \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_DMA_EN ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMGO_BASE_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMGO_OFST_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMG2O_BASE_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMG2O_OFST_ADDR ));


            if ( MTRUE == function_DbgLogEnable_VERBOSE)
            {
                usleep(1000);
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();
            pass2_cnt = 0;
            }
        }
        else if ( CAM_ISP_PASS2_START == *start || CAM_ISP_PASS2B_START == *start || CAM_ISP_PASS2C_START == *start)
        {
            //usleep(1000);
            //pass2 DMA base_addr
#if 0
                ISP_FUNC_DBG("P2 dma_en(0x%x),addr(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)", \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_DMA_EN ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMGI_BASE_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_IMGI_OFST_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_VIDO_BASE_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_VIDO_OFST_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_DISPO_BASE_ADDR ), \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_DISPO_OFST_ADDR ));

#endif
            if ( 10==pass2_cnt && MTRUE == function_DbgLogEnable_VERBOSE)
            {
                this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();
            }
        }
        pass2_cnt++;
    }

#endif

    ISP_FUNC_INF("path(%d),start(%d),SCIO(%d),En1(0x%x),En2(0x%x),Dma(0x%x)",
    this->path, \
    *((MUINT32*)pParam), \
    this->fmt_sel.bit_field.scenario,
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004004),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004008),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x0000400C));
    //

    return 0;

}

int ISP_TOP_CTRL::_disable( void )
{
    MBOOL ret = MTRUE;
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_VSYNC;

    irq_TG1_VSYNC.Clear=NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_WAIT;
    irq_TG1_VSYNC.UserInfo.Type=NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
    irq_TG1_VSYNC.UserInfo.Status=ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    irq_TG1_VSYNC.UserInfo.UserKey=0;
    irq_TG1_VSYNC.Timeout=CAM_INT_WAIT_TIMEOUT_MS;
    irq_TG1_VSYNC.bDumpReg=0xFE;


    ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl;
    MUINT32 dummy;
    MUINT32 tgStatus;
    Mutex   *_localVar;
    ISP_FUNC_DBG("ISP_TOP_CTRL::_disable:E 0x%x 0x%x\n",this->m_pIspDrvShell->m_pIspDrv,this->m_pIspDrvShell->m_pPhyIspDrv_bak);
    // lock for pass1 and pass2 respectively
    if(ISP_PASS1 == this->path)
    {
        _localVar = &this->m_pIspDrvShell->gPass1Lock;
    }
    else {
        _localVar = &this->m_pIspDrvShell->gPass2Lock;
    }
    Mutex::Autolock lock(_localVar);
    if (ISP_PASS1 == this->path)
    {
        ISP_FUNC_INF("pass1: stop \n");
        //TG1
        if ( CAM_CTL_EN1_TG1_EN & this->en_Top.enable1 )
        {
            MUINT32 wait_vs_num = 1;
            ISP_WRITE_BITS(m_pPhyIspDrv, CAM_TG_VF_CON, VFDATA_EN, 0, ISP_DRV_USER_ISPF);

            if (MFALSE == this->m_pPhyIspDrv->ISPWakeLockCtrl(MFALSE))
            {
                ISP_FUNC_ERR("ISPWakeLockCtrl disable fail!!");
                return -1;
            }
            ISP_FUNC_INF("[TG1]wait %d VD till DMA stop",wait_vs_num);
            //
            {
                while ( wait_vs_num-- ) {
                    ret = this->m_pPhyIspDrv->waitIrq( &irq_TG1_VSYNC );
                    if ( MFALSE == ret ) {
                        ISP_FUNC_ERR("waitIrq( irq_TG1_VSYNC ) fail");
                    }
                    else{
                        //check TG IDLE
                        if(ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_INTER_ST,TG_CAM_CS) != 1){
                            wait_vs_num++;
                            ISP_FUNC_INF("[TG1]TG is not idle, wait %d VD again.\n", wait_vs_num);
                            continue;
                        }
                        //flush EnqueThread SOF irq
                        ISP_FUNC_INF("[TG1]flush SOF");
                        NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_SOF;
                        irq_TG1_SOF.UserInfo.Type=NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
                        irq_TG1_SOF.UserInfo.Status=ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST;
                        irq_TG1_SOF.UserInfo.UserKey=0;
                        ret = this->m_pPhyIspDrv->flushIrq(irq_TG1_SOF);
                        if ( MFALSE == ret ) {
                            ISP_FUNC_ERR("flushIrq( irq_TG1_SOF ) fail");
                        }
                    }
                    //
                }
            }
        }

        //fbc disable before VF_EN
        //IMGO
        ISP_WRITE_BITS(this->m_pPhyIspDrv, CAM_CTL_IMGO_FBC, FBC_EN, 0, ISP_DRV_USER_ISPF);
        //IMG2O
        ISP_WRITE_BITS(this->m_pPhyIspDrv, CAM_CTL_IMG2O_FBC, FBC_EN, 0, ISP_DRV_USER_ISPF);
        //
        //CLR dma/en1/en2
        {
            MUINT32 _tmp=0;
            _tmp = P1_DMA_MASK;
            if(this->en_Top.dma & CAM_CTL_DMA_EN_IMG2O_EN)
            {}
            else
                _tmp &= (~CAM_CTL_DMA_EN_IMG2O_EN);

            ISP_WRITE_REG(this->m_pPhyIspDrv, CAM_CTL_DMA_EN_CLR, _tmp, ISP_DRV_USER_ISPF);
        }
        ISP_WRITE_REG(this->m_pPhyIspDrv, CAM_CTL_EN1_CLR, P1_EN1_MASK, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(this->m_pPhyIspDrv, CAM_CTL_EN2_CLR, P1_EN2_MASK, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(this->m_pPhyIspDrv, CAM_RAW_MAGIC_NUM0, 0, ISP_DRV_USER_ISPF);
        {
            IspDrv* _virDrv = NULL;
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                _virDrv = this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ, this->burstQueIdx, i);
                ISP_WRITE_BITS(_virDrv, CAM_CTL_IMGO_FBC, FBC_EN, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_virDrv, CAM_CTL_IMG2O_FBC, FBC_EN, 0, ISP_DRV_USER_ISPF);

                ISP_WRITE_REG(_virDrv, CAM_CTL_DMA_EN_CLR, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_virDrv, CAM_CTL_EN1_CLR, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_virDrv, CAM_CTL_EN2_CLR, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_virDrv, CAM_RAW_MAGIC_NUM0, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_virDrv, CAM_CTL_DMA_EN_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_virDrv, CAM_CTL_EN1_SET, 0, ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(_virDrv, CAM_CTL_EN2_SET, 0, ISP_DRV_USER_ISPF);
            }
        }
        //pass1 stop totally
        this->bScenario = MFALSE;
        this->bIMG2O_en = MFALSE;
        //pass1 DMA SHOULD BE inactive.
        //dump debug info
            ISP_FUNC_INF("- P1 stop VF(0x%x),TG_IN(0x%x),STX(0x%x)", \
                ISP_READ_REG_NOPROTECT(this->m_pPhyIspDrv, CAM_TG_VF_CON), \
                ISP_READ_REG_NOPROTECT(this->m_pPhyIspDrv, CAM_TG_INTER_ST), \
                ISP_READ_REG_NOPROTECT(this->m_pPhyIspDrv, CAM_CTL_INT_STATUSX) );



    #ifdef _rtbc_buf_que_2_0_
        dma_array[_imgo_] = 0;
        dma_array[_img2o_] = 0;
        buf_ctrl = {ISP_RT_BUF_CTRL_DMA_EN_FRMB,_imgo_,0,0,(unsigned char *)dma_array};
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
    #endif
        //reset p1 cmdQ descriptor
        for(int j=0;j<ISP_DRV_P1_CQ_DUPLICATION_NUM;j++){
            for(int i=0; i<CAM_DUMMY_; i++){
                this->m_pIspDrvShell->m_pPhyIspDrv_bak->cqDelModule(this->CQ, this->burstQueIdx, j, (CAM_MODULE_ENUM)i);
            }
        }

        MUINT32 _clrPort = 0;
        MUINT32 _dma[2] = {0};
        if(ISP_PASS1 == this->path){
            _clrPort = 1;
            _dma[0] = _imgo_;
            if(this->en_Top.dma & CAM_CTL_DMA_EN_IMG2O_EN){
                _clrPort++;
                _dma[1] = _img2o_;
            }
        }
        else
            ISP_FUNC_ERR("path error 0x%x_0x%x\n",this->path);

        while(_clrPort--){
            ISP_FUNC_DBG("[TopCtrl::Disable][before]rt_dma(%d),size:hw(%d)",_dma[_clrPort], ISP_BUF_CTRL::m_hwbufL[_dma[_clrPort]].bufInfoList.size());
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR_FRMB;
            buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
            buf_ctrl.data_ptr = (ISP_RT_BUF_INFO_STRUCT_FRMB *)&dummy;
            ISP_FUNC_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%p)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            ISP_FUNC_DBG("[TopCtrl::Disable][after]rt_dma(%d),size:hw(%d) ",_dma[_clrPort], ISP_BUF_CTRL::m_hwbufL[_dma[_clrPort]].bufInfoList.size());
        }
    }
    else if (ISP_PASS2 == this->path)
    {
            // remove for MDP trigger             ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2_START, 0);

    }
    else if (ISP_PASS2B == this->path)
    {
            // remove for MDP trigger               ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2B_START, 0);

    }
    else if (ISP_PASS2C == this->path)
    {
            // remove for MDP trigger               ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2C_START, 0);
            ISP_FUNC_DBG(" : No implementation for Pass2C start = 0\n");

    }

#if 1
    if(MTRUE == function_DbgLogEnable_INFO)
    {
        if(ISP_PASS1 == this->path)
        {
            usleep(1000);
            //pass1 DMA base_addr
                ISP_FUNC_INF("P1 start(0x%x)", \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_START ));


        }
        else if ( ISP_PASS2 == this->path ||
                  ISP_PASS2B == this->path ||
                  ISP_PASS2C == this->path )
        {
            //usleep(1000);
            //pass2 DMA base_addr
                ISP_FUNC_INF("P2 start(0x%x)", \
                    ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_CTL_START ));

        }

    }

#endif

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
    //TOP
    ISP_FUNC_DBG("ISP_TOP_CTRL::_write2CQ: path(%d) E ",this->path);

    if(this->path == ISP_PASS1){
        for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(NSIspDrv_FrmB::CAM_MODULE_ENUM)CAM_TOP_CTL_01);
            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,(NSIspDrv_FrmB::CAM_MODULE_ENUM)CAM_TOP_CTL_02);
        }
    }else{
        //tpipe config only in pass2
        this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)CAM_TOP_CTL);
        this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)CAM_ISP_GGM_CTL);
        this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)CAM_TOP_CTL_01);
        this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)CAM_TOP_CTL_02);
    }
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

    MUINT32 i;
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 enCrsp=0x00;
    IspTuningPackage tuningPackage;
    IspImageioPackage imageioPackage;
    //
    ISP_FUNC_DBG("+(ISP_RAW_PIPE),cq(%d),drvScenario(%d),sub_mode(%d),enable1(0x%08x),enable2(0x%08x)",
            this->CQ,this->drvScenario,this->p2SubMode,this->enable1,this->enable2);
    //
    if ( ISP_PASS2 == this->path || \
         ISP_PASS2B == this->path || \
         ISP_PASS2C == this->path
    ) {
        //getEnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->p2SubMode, tuningEn1Tag, tuningEn2Tag, tuningDmaTag);
        imageioPackage.isApplyTuning = this->isApplyTuning;
        imageioPackage.cq = this->CQ;
        imageioPackage.subMode = this->p2SubMode;

        if(imageioPackage.isApplyTuning==MTRUE){
            tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->isV3);
            tuningPackage.pTuningBuf = (MUINT32*)this->pTuningIspReg;
            if (tuningPackage.pTuningBuf == NULL){
                ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
                return 0;
            }
            ISP_FUNC_DBG("tuningUpdateBit(0x%08x),pTuningBuf(0x%08x)",tuningUpdateBit,tuningPackage.pTuningBuf);
        }
        imageioPackage.finalEn1 = this->enable1;
        imageioPackage.finalEn2 = this->enable2;
        //
        for(i=0;i<CHECK_TUNING_EN1_BITS;i++){
            if(     ((this->enable1 & gIspModuleMappingTableEn1[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc2)) &&
                    (gIspModuleMappingTableEn1[i].eModule==eIspModuleRaw) &&
                    (gIspModuleMappingTableEn1[i].default_func!=NULL)) {
                //
                if(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc1)
                    tuningPackage.eTuningCqFunc1 = gIspModuleMappingTableEn1[i].eTuningCqFunc1;
                else
                    tuningPackage.eTuningCqFunc1 = CAM_DUMMY_;
                //
                if(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc2)
                    tuningPackage.eTuningCqFunc2 = gIspModuleMappingTableEn1[i].eTuningCqFunc2;
                else
                    tuningPackage.eTuningCqFunc2 = CAM_DUMMY_;
                //
                if(((tuningPackage.eTuningCqFunc1!=CAM_DUMMY_)||(tuningPackage.eTuningCqFunc2!=CAM_DUMMY_))&&(gIspModuleMappingTableEn1[i].eTuningCqDma!=CAM_DUMMY_))
                    tuningPackage.eTuningCqDma = gIspModuleMappingTableEn1[i].eTuningCqDma;
                else
                    tuningPackage.eTuningCqDma = CAM_DUMMY_;

                gIspModuleMappingTableEn1[i].default_func(imageioPackage, tuningPackage);
                //
            }
        }
    } else if ( ISP_PASS1   == this->path) {
        IspHrzCfg hrz_cfg;
        IspDrv* _targetVirDrv = NULL;
        MUINT32 _targetDupQIdx = 0;
#ifdef _PASS1_CQ_CONTINUOUS_MODE_
        MUINT32 bVF_en = MFALSE;

        if(this->path == ISP_PASS1)
            bVF_en = ISP_READ_BITS_NOPROTECT(m_pPhyIspDrv,CAM_TG_VF_CON,VFDATA_EN);
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
        if( ISP_PASS1   == this->path){
            _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQIdx);
            if(_targetVirDrv == NULL){
                ISP_FUNC_ERR("_targetVirDrv == NULL, _targetDupQIdx = 0x%x\n",_targetDupQIdx);
                return -1;
            }
        }

        hrz_cfg.hrz_in_size = this->hrz_in;
        hrz_cfg.hrz_out_size = this->hrz_out;
        hrz_cfg.m_num_0 = this->magicNum;
        hrz_cfg.pass1_path = this->path;

        if(this->enable1 & CAM_CTL_EN1_BIN_EN) {
            ISP_WRITE_REG(_targetVirDrv,CAM_BIN_MODE, 0x0,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv,CAM_BIN_SIZE, (this->src_img_h<<16)||this->src_img_w,ISP_DRV_USER_ISPF);
        }
        if(this->enable1 & CAM_CTL_EN1_HRZ_EN){
            if(0!=this->pPass1Ctrl->keepP1HrzCfg(hrz_cfg)){
                ISP_FUNC_ERR("keep hrz fail\n");
                return -1;
            }
            if(0!= this->pPass1Ctrl->setP1HrzCfg(hrz_cfg)){
                ISP_FUNC_ERR("set hrz fail\n");
                return -1;
            }
        }
#if 0    //remove, im2o won't be always enabled, magicnum need to be set independently.
        if ( 0 != this->pPass1Ctrl->setP1MagicNum(this->path, this->magicNum, 1)) {
            ISP_FUNC_ERR("setP1MagicNum Raw fail");
            return -1;
        }
#endif
    } else {
        ISP_FUNC_ERR("[ERROR]Not support this path(%d)",this->path);
    }

    ISP_FUNC_DBG("-,");

    return 0;
}

/*
int ISP_RAW_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("ISP_RAW_PIPE::_enable:E ");
    ISP_FUNC_DBG("ISP_RAW_PIPE::_enable:X ");
    return 0;
}

int ISP_RAW_PIPE::_disable( void )
{
    ISP_FUNC_DBG("ISP_RAW_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_RAW_PIPE::_disable:X ");
    return 0;
}

int ISP_RAW_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("ISP_RAW_PIPE::checkBusy:E ");
    ISP_FUNC_DBG("ISP_RAW_PIPE::checkBusy:X ");
    return int_done_status?0:1;
}
*/
int ISP_RAW_PIPE::_write2CQ(void)
{

    ISP_FUNC_DBG("+(ISP_RAW_PIPE),cq(%d),drvScenario(%d),p2SubMode(%d)",this->CQ,this->drvScenario,this->p2SubMode);

    if ( ISP_PASS2 == this->path || \
         ISP_PASS2B == this->path || \
         ISP_PASS2C == this->path
    ) {
        MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
        MBOOL isApplyTuning;
        MUINT32 i;
        //
        isApplyTuning = this->isApplyTuning;
        if(isApplyTuning==MTRUE){
            tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->isV3);
            ISP_FUNC_DBG("tuningUpdateBit(0x%08x)",tuningUpdateBit);
        }
        //
        for(i=0;i<CHECK_TUNING_EN1_BITS;i++){
            if(((this->enable1 & gIspModuleMappingTableEn1[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc2))
                    &&(gIspModuleMappingTableEn1[i].eModule==eIspModuleRaw)){  // check bit en
                if(gIspModuleMappingTableEn1[i].eTuningCqFunc1!=CAM_DUMMY_)
                    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn1[i].eTuningCqFunc1);
                if(gIspModuleMappingTableEn1[i].eTuningCqFunc2!=CAM_DUMMY_)
                    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn1[i].eTuningCqFunc2);
                if(gIspModuleMappingTableEn1[i].eTuningCqDma!=CAM_DUMMY_)
                    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn1[i].eTuningCqDma);
            }
        }
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, CAM_ISP_UNP);

    } else if ( ISP_PASS1 == this->path) {
        for (int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++)
        {
            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_LSC);
            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_ISP_HRZ);
            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,i,CAM_P1_MAGIC_NUM);
        }
    } else {
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
        input.sHBINOut.w = this->src_img_w;
        input.sHBINOut.h = this->src_img_h;
        input.sTGOut.w = this->src_img_w;
        input.sTGOut.h = this->src_img_h;
        input.sRMXOut.w = this->hrz_out.w;
        input.sRMXOut.h = this->hrz_out.h;
        if(this->sub_mode == 1)
            input.bYUVFmt = MTRUE;
        else
            input.bYUVFmt = MFALSE;
        ISP_FUNC_DBG("Update EIS,w/h(%d_%d,%d_%d,%d_%d),submode=%d",input.sHBINOut.w,input.sHBINOut.h,input.sTGOut.w,input.sTGOut.h,input.sRMXOut.w,input.sRMXOut.h,this->p2SubMode);
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
            //HW bug, 82 only!!!
            m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQIdx,CAM_ISP_EIS_DCM);
            if(_eis_cfg.bEIS_EN){
                ISP_DRV_CQ_MODULE_INFO_STRUCT outInfo;
                _targetVirDrv->getIspCQModuleInfo(CAM_ISP_EIS, outInfo);
                memcpy((((MUINT8*)(isp_reg_t*) _targetVirDrv->getRegAddr()) + (outInfo.addr_ofst)), \
                      (MUINT8*)&_eis_cfg._EIS_REG.CTRL_1, \
                      (outInfo.reg_num)*sizeof(UINT32));
                //EIS sel don't case here, eis is link with SGG
                //ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_SEL_SET, EIS_SEL_SET, _eis_cfg.EIS_SEL,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN2_SET, EIS_EN_SET, 1,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN2_CLR, EIS_EN_CLR, 0,ISP_DRV_USER_ISPF);
                //HW bug for EIS, 82 only!!!!!
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_CDP_DCM_DIS, EIS_DCM_DIS, 1,ISP_DRV_USER_ISPF);
            }
            else{
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN2_SET, EIS_EN_SET, 0,ISP_DRV_USER_ISPF);
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN2_CLR, EIS_EN_CLR, 1,ISP_DRV_USER_ISPF);
                //HW bug for EIS, 82 only!!!!!
                ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_CDP_DCM_DIS, EIS_DCM_DIS, 0,ISP_DRV_USER_ISPF);
            }

            if(_eis_cfg.bSGG_Bypass == 0){
                if(_eis_cfg.bSGG_EN){
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_MUX_SEL_SET, SGG_SEL_EN_SET, 1,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_MUX_SEL_CLR, SGG_SEL_EN_CLR, 0,ISP_DRV_USER_ISPF);
                }
                else{
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_MUX_SEL_SET, SGG_SEL_EN_SET, 0,ISP_DRV_USER_ISPF);
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_MUX_SEL_CLR, SGG_SEL_EN_CLR, 1,ISP_DRV_USER_ISPF);
                }
                if(_eis_cfg.SGG_SEL > 3){
                    ISP_FUNC_ERR("sgg sel(0x%x) err\n",_eis_cfg.SGG_SEL);
                    return 0;
                }
                else{
                    ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_MUX_SEL_SET, SGG_SEL_SET, _eis_cfg.SGG_SEL, ISP_DRV_USER_ISPF);
                     ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_MUX_SEL_CLR, SGG_SEL_CLR, (3-_eis_cfg.SGG_SEL), ISP_DRV_USER_ISPF);
                 }
            }
        }
    }
    return ret;
}

MUINT32 ISP_RAW_PIPE::setLCS(void){
    MUINT32 ret = 1;
#if 0//fix it later
    if(this->pP1Tuning)
    {
        LCS_REG_CFG     _lcs_cfg;
        LCS_INPUT_INFO     input;
        if(this->path == ISP_PASS1)
        {
            input.shrzOut.w = this->hrz_out_size.w;
            input.shrzOut.h = this->hrz_out_size.h;
        }
        else if(this->pathD == ISP_PASS1_D)
        {
            input.shrzOut.w = this->hrz_d_out_size.w;
            input.shrzOut.h = this->hrz_d_out_size.h;
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
        ISP_FUNC_DBG("update LCS(0x%x_0x%x),in:%d_%d, LCS_REG(%d): 0x%x,0x%x\n",this->path,this->pathD,input.shrzOut.w,input.shrzOut.h,\
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
#endif
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
   temp = ((pLsc_ctl2->Bits.SDBLK_XNUM+1)<<6) - 1 ;
   if (lsci_x != temp) {
       ISP_FUNC_ERR("[Error] LSCI_x != (LSC_x+1)<<6 -1. (%d,%d), XNUM=%d", lsci_x, temp, pLsc_ctl2->Bits.SDBLK_XNUM);
       return -1;
   }

   if (lsci_y != pLsc_ctl3->Bits.SDBLK_YNUM) {
       ISP_FUNC_ERR("[Error] LSCI_y != LSC_y. (%d,%d)", lsci_y, pLsc_ctl3->Bits.SDBLK_YNUM);
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
        _targetVirDrv = (IspDrv*)this->m_pIspDrvShell->ispDrvGetCQIspDrv(this->CQ,this->burstQueIdx,_targetDupQueIdx);
        if(_targetVirDrv == NULL){
            ISP_FUNC_ERR(" dupCmdQ == NULL, target dupCqIdx = 0x%x",_targetDupQueIdx);
            return -1;
        }
        if(this->m_Dmao_SET_Reg[_targetDupQueIdx] == 0xffffffff){
            MUINT32 _reg_set = 0x0,_reg_clr = (P1_DMA_MASK - CAM_CTL_DMA_EN_IMGO_EN - CAM_CTL_DMA_EN_IMG2O_EN);
            this->m_Dmao_SET_Reg[_targetDupQueIdx] = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_DMA_EN_SET);
            this->m_Dmao_CLR_Reg[_targetDupQueIdx] = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_DMA_EN_CLR);
            if(this->m_Dmao_SET_Reg[_targetDupQueIdx] & CAM_CTL_DMA_EN_IMGO_EN){
                _reg_set |= CAM_CTL_DMA_EN_IMGO_EN;
            }
            if(this->m_Dmao_SET_Reg[_targetDupQueIdx] & CAM_CTL_DMA_EN_IMG2O_EN){
                _reg_set |= CAM_CTL_DMA_EN_IMG2O_EN;
            }
            ISP_FUNC_DBG("A:(0x%x):0x%x_0x%x\n",_targetDupQueIdx,_reg_set,_reg_clr);
            ISP_WRITE_REG(_targetVirDrv, CAM_CTL_DMA_EN_SET, _reg_set,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(_targetVirDrv, CAM_CTL_DMA_EN_CLR, _reg_clr,ISP_DRV_USER_ISPF);
        }else
            ISP_FUNC_DBG("B:(0x%x):0x%x_0x%x\n",_targetDupQueIdx,ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_DMA_EN_SET),ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_DMA_EN_CLR));
        return 1;
    }
    return 0;
}

void ISP_RAW_PIPE::_resumeTuneCfg(IspDrv* _targetVirDrv,MUINT32 _targetDupQueIdx)
{
    MUINT32 _tmp = 0;
    if(this->m_Dmao_SET_Reg[_targetDupQueIdx] != 0xFFFFFFFF){
        ISP_FUNC_DBG("bef(0x%x):0x%x_0x%x\n",_targetDupQueIdx,this->m_Dmao_SET_Reg[_targetDupQueIdx],this->m_Dmao_CLR_Reg[_targetDupQueIdx]);
        _tmp = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_DMA_EN_SET) & (P1_DMA_MASK - CAM_CTL_DMA_EN_IMGO_EN - CAM_CTL_DMA_EN_IMG2O_EN);
        if(_tmp){
            //means dmao have been enabled through p1hwcfg
            this->m_Dmao_SET_Reg[_targetDupQueIdx] |= _tmp;
        }
        _tmp = ISP_READ_REG_NOPROTECT(_targetVirDrv, CAM_CTL_DMA_EN_CLR) ^ (P1_DMA_MASK - CAM_CTL_DMA_EN_IMGO_EN - CAM_CTL_DMA_EN_IMG2O_EN);
        if(_tmp){
            //means dmao have been disabled through p1hwcfg
            this->m_Dmao_CLR_Reg[_targetDupQueIdx]  -= _tmp;
        }
        ISP_FUNC_DBG("aft:0x%x_0x%x\n",this->m_Dmao_SET_Reg[_targetDupQueIdx],this->m_Dmao_CLR_Reg[_targetDupQueIdx]);
        ISP_WRITE_REG(_targetVirDrv,CAM_CTL_DMA_EN_SET, this->m_Dmao_SET_Reg[_targetDupQueIdx], ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(_targetVirDrv,CAM_CTL_DMA_EN_CLR, this->m_Dmao_CLR_Reg[_targetDupQueIdx], ISP_DRV_USER_ISPF);

        this->m_Dmao_SET_Reg[_targetDupQueIdx] = 0xFFFFFFFF;
        this->m_Dmao_CLR_Reg[_targetDupQueIdx] = 0xFFFFFFFF;
    }
}

int ISP_RAW_PIPE::setP1TuneCfg(MUINT32 mNum, MINT32 senDev)
{
    ISP_DRV_CQ_MODULE_INFO_STRUCT outInfo;

    IspP1TuneCfg cfg;
    ISP_DRV_CQ_ENUM cq;
    ESoftwareScenario softScenario = eSoftwareScenario_Main_Normal_Stream;
    MUINT32* pTuningBuf;
    MUINT32 i, tuningUpdateBit;
    MUINT32 tuningEnP1=0x00, tuningEnP1Dma=0x00, tuningEnP1Tag=0x00, tuningEnP1DmaTag=0x00;
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQueIdx = 0;
    MBOOL isApplyTuning;

#if 0 //close resume/suspend mechanism
    //dummy frame , disable all dmao but main img_port, other regs. setting remained previous setting
    //mark, statistic still output when using dummyframe
    if(this->_suspendTuneCfg(mNum)){
        return 0;
    }
#else
    if(mNum & _DUMMY_MAGIC){
        return 0;
    }
#endif
    if (eIspUpdateTgtDev_Sub== senDev) {
        softScenario = eSoftwareScenario_Sub_Normal_Stream;
    }


    m_pPhyIspDrv->getCqInfoFromScenario(softScenario, cq);

    isApplyTuning = m_pPhyIspDrv->getP2TuningStatus(cq);
    if(isApplyTuning){

        if(MFALSE == m_pPhyIspDrv->deTuningQueByCq(eTuningQueAccessPath_imageio_P1, cq, mNum)){
            ISP_FUNC_ERR("[Error]deTuningQue fail");
            return -1;
        }

        cfg.tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P1, cq);
        cfg.tuningEnP1      = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P1, eTuningTopEn1, cq, 0);
        cfg.tuningEnP1Dma   = m_pPhyIspDrv->getTuningTop(eTuningQueAccessPath_imageio_P1, eTuningTopDmaEn, cq, 0);
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
            return -1;
        }
#if 0    //close resume/suspend mechanism
        //resume dmao if previous cq dmao have been suspend
        //
        //mark, statistic still output when using dummyframe
        this->_resumeTuneCfg(_targetVirDrv,_targetDupQueIdx);
#endif
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
            if (cfg.tuningEnP1 & CAM_CTL_EN1_OB_EN) {
               //Enable
               ISP_FUNC_DBG("Enable Obc, 0x%x/0x%x", cqVirtReg, cfg.pTuningBuf, (outInfo.reg_num)*sizeof(UINT32));
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf))  + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_SET, OB_EN_SET, 1,ISP_DRV_USER_ISPF);
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_CLR, OB_EN_CLR, 0,ISP_DRV_USER_ISPF);

            }else {
               //Disable
               ISP_FUNC_DBG("Disable Obc, 0x%x/0x%x/%d", cqVirtReg, cfg.pTuningBuf, (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_SET, OB_EN_SET, 0,ISP_DRV_USER_ISPF);
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_CLR, OB_EN_CLR, 1,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
            }
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
            if (cfg.tuningEnP1 & CAM_CTL_EN1_BNR_EN) {
               //Enable
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_SET, BNR_EN_SET, 1,ISP_DRV_USER_ISPF);
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_CLR, BNR_EN_CLR, 0,ISP_DRV_USER_ISPF);
            }else {
               //Disable
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_SET, BNR_EN_SET, 0,ISP_DRV_USER_ISPF);
               ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_CLR, BNR_EN_CLR, 1,ISP_DRV_USER_ISPF);
               memcpy((((MUINT8*)cqVirtReg)        + (outInfo.addr_ofst)), \
                      (((MUINT8*)(cfg.pTuningBuf)) + (outInfo.addr_ofst)), \
                      (outInfo.reg_num)*sizeof(UINT32));
            }
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
                lsc_en = cfg.tuningEnP1 & CAM_CTL_EN1_LSC_EN?1:0;


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
                lsci_en = cfg.tuningEnP1Dma & CAM_CTL_DMA_EN_LSCI_EN?1:0;


                if (lsci_en ^ lsc_en) {
                    ISP_FUNC_ERR("[Error] Lsc & Lsci shall be enable/disable together. %d,%d", lsc_en, lsci_en);
                } else {

                    if (checkLscLsciCfg(pLscCfg, pLsciCfg)) {
                         ISP_FUNC_ERR("[Error] Lsc & Lsci cfg is Invalid");
                    } else {

                        m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_ISP_LSC);
                        m_pPhyIspDrv->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,_targetDupQueIdx,CAM_DMA_LSCI);

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
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 1,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_SET, LSC_EN_SET, 1,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_CLR, LSC_EN_CLR, 0,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 0,ISP_DRV_USER_ISPF);
                        }else {
                            ISP_FUNC_DBG("LSC+LSCI Disable");
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 0,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_SET, LSC_EN_SET, 0,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_EN1_CLR, LSC_EN_CLR, 1,ISP_DRV_USER_ISPF);
                            ISP_WRITE_BITS(_targetVirDrv, CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 1,ISP_DRV_USER_ISPF);

                            //lsc
                            memcpy((((MUINT8*)cqVirtReg)        + (lscOutInfo.addr_ofst)), \
                                (((MUINT8*)(cfg.pTuningBuf)) + (lscOutInfo.addr_ofst)), \
                                (lscOutInfo.reg_num)*sizeof(UINT32));
                        }
                    }
                }
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
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 enCrsp=0x00;
    IspTuningPackage tuningPackage;
    IspImageioPackage imageioPackage;
    //
    ISP_FUNC_DBG("+(ISP_RGB_PIPE),cq(%d),scenario(%d),sub_mode(%d),enable1(0x%08x),enable2(0x%08x)",
            this->CQ,this->drvScenario,this->p2SubMode,this->enable1,this->enable2);
    //
    //getEnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->p2SubMode, tuningEn1Tag, tuningEn2Tag, tuningDmaTag);
    imageioPackage.isApplyTuning = this->isApplyTuning;
    imageioPackage.cq = this->CQ;
    imageioPackage.subMode = this->p2SubMode;

    if(imageioPackage.isApplyTuning==MTRUE){
        tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->isV3);
        tuningPackage.pTuningBuf = (MUINT32*)this->pTuningIspReg;
        if (tuningPackage.pTuningBuf == NULL){
            ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
            return 0;
        }
        imageioPackage.hrzRatio = this->hrzRatio;
        imageioPackage.hrzCropX = this->hrzCropX;
        imageioPackage.hrzCropY = this->hrzCropY;

        ISP_FUNC_DBG("tuningUpdateBit(0x%08x),pTuningBuf(0x%08x),hrz_crop(%d,%d)", \
            tuningUpdateBit,tuningPackage.pTuningBuf,imageioPackage.hrzCropX,imageioPackage.hrzCropY);
    }
    imageioPackage.finalEn1 = this->enable1;
    imageioPackage.finalEn2 = this->enable2;
    //
    for(i=0;i<CHECK_TUNING_EN1_BITS;i++){
        if(     ((this->enable1 & gIspModuleMappingTableEn1[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc2)) &&
                (gIspModuleMappingTableEn1[i].eModule==eIspModuleRgb) &&
                (gIspModuleMappingTableEn1[i].default_func!=NULL)) {
            //
            if(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc1)
                tuningPackage.eTuningCqFunc1 = gIspModuleMappingTableEn1[i].eTuningCqFunc1;
            else
                tuningPackage.eTuningCqFunc1 = CAM_DUMMY_;
            //
            if(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc2)
                tuningPackage.eTuningCqFunc2 = gIspModuleMappingTableEn1[i].eTuningCqFunc2;
            else
                tuningPackage.eTuningCqFunc2 = CAM_DUMMY_;
            //
            if(((tuningPackage.eTuningCqFunc1!=CAM_DUMMY_)||(tuningPackage.eTuningCqFunc2!=CAM_DUMMY_))&&(gIspModuleMappingTableEn1[i].eTuningCqDma!=CAM_DUMMY_))
                tuningPackage.eTuningCqDma = gIspModuleMappingTableEn1[i].eTuningCqDma;
            else
                tuningPackage.eTuningCqDma = CAM_DUMMY_;

            gIspModuleMappingTableEn1[i].default_func(imageioPackage, tuningPackage);
            //
        }
    }
    //
    if(this->enable1 & CAM_CTL_EN1_SL2_EN) {
       //TODO : kk test need to check HRZ???
       //if(this->enable1 & CAM_CTL_EN1_HRZ_EN)

       if(0){  //kk test
            MUINT32 resize = 0x8000; //kk test
            /*
            HRZ_INPUT_HEIGHT:
                "HRZ input size
                if (tg_sel = 0) height=image0
                else height = image1"

            HRZ_RESIZE:
                "HRZ resize value
                Get from 32768/decimation ratio
                decimation x1.0: hrz_resize should be 32768
                decimation x0.5: hrz_resize should be 65536
                decimation x0.25: hrz_resize should be 131072 "

            HRZ_OUTSIZE:
                "HRZ output size
                And this value get from
                X1 = input size * 32768 / hrz_resize
                hbn_outsize = 2* (X1>>1)"
            */
            //resize = 32768*10/10;
            //resize = ( this->src_img_w << 15 )/this->hrz_out.w;
            //
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, resize>>4,ISP_DRV_USER_ISPF);  //kk test need to move to p1
        } else {
            ISP_WRITE_BITS(m_pP2IspDrv, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, 0x800,ISP_DRV_USER_ISPF);  //kk test need to move to p1
        }
    }

    return 0;
}

/*
int ISP_RGB_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("ISP_RGB_PIPE::_enable:E ");
    ISP_FUNC_DBG("ISP_RGB_PIPE::_enable:X ");
    return 0;
}

int ISP_RGB_PIPE::_disable( void )
{
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:X ");
    return 0;
}

int ISP_RGB_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:X ");
    return int_done_status?0:1;
}
*/
int ISP_RGB_PIPE::_write2CQ(void)
{
    #if 1
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MBOOL isApplyTuning;
    MUINT32 i;
    //
    ISP_FUNC_DBG("+(ISP_RGB_PIPE),cq(%d),drvScenario(%d),enable1(0x%08x),isApplyTuning(%d)",this->CQ,this->drvScenario,this->enable1,this->isApplyTuning);
    //
    isApplyTuning = this->isApplyTuning;
    if(isApplyTuning==MTRUE){
        tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->isV3);
        ISP_FUNC_DBG("tuningUpdateBit(0x%08x)",tuningUpdateBit);
    }
    //
    for(i=0;i<CHECK_TUNING_EN1_BITS;i++){
        if(((this->enable1 & gIspModuleMappingTableEn1[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableEn1[i].eTuningFunc2))
                &&(gIspModuleMappingTableEn1[i].eModule==eIspModuleRgb)){  // check bit en
            if(gIspModuleMappingTableEn1[i].eTuningCqFunc1!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn1[i].eTuningCqFunc1);
            if(gIspModuleMappingTableEn1[i].eTuningCqFunc2!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn1[i].eTuningCqFunc2);
            if(gIspModuleMappingTableEn1[i].eTuningCqDma!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn1[i].eTuningCqDma);
        }
    }

    #else
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00;
    MUINT32 tuningEnP2=0x00, tuningEnP2Dma=0x00;
    MUINT32 finalEnP2 = 0x00;
    MUINT32 i;
    MBOOL isApplyTuning;

    ISP_FUNC_DBG("+(ISP_RGB_PIPE),cq(%d),drvScenario(%d),sub_mode(%d)",this->CQ,this->drvScenario,this->sub_mode);

#if defined(USING_MTK_LDVT)
    //debug path, imagio will config all engine by CQ
    #define CAM_ISP_RGB_MODULE_COUNT    4
    struct stCam_Id_Enable cq_info[CAM_ISP_RGB_MODULE_COUNT] = { \
                                         {NSIspDrv_FrmB::CAM_DUMMY_,  CAM_CTL_EN1_CFA_EN}, \
                                         {NSIspDrv_FrmB::CAM_DUMMY_,  CAM_CTL_EN1_CCL_EN}, \
                                         {NSIspDrv_FrmB::CAM_ISP_G2G,  CAM_CTL_EN1_G2G_EN}, \
                                         {NSIspDrv_FrmB::CAM_DUMMY_,    CAM_CTL_EN1_GGM_EN}
                                         };
#else
    //MP path, do not config engine by imageio
#define CAM_ISP_RGB_MODULE_COUNT    2
    struct stCam_Id_Enable cq_info[CAM_ISP_RGB_MODULE_COUNT] = { \
                                         {NSIspDrv_FrmB::CAM_ISP_CFA,  CAM_CTL_EN1_CFA_EN}, \
                                         {NSIspDrv_FrmB::CAM_ISP_CCL,  CAM_CTL_EN1_CCL_EN}, \
                                         };

#endif

    for (int i=0;i<CAM_ISP_RGB_MODULE_COUNT;i++)
    {
        if ( this->enable1 & cq_info[i].en_bit)
        {
            ISP_FUNC_DBG("(0x%x)write 0x%08X to CQ: ",enable1,cq_info[i].en_bit);
            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)cq_info[i].id);
        }
    }

    //
    ISP_FUNC_DBG("-(ISP_RGB_PIPE),p2TuningEnTag(0x%08x),p2EnConfig(0x%08x)",tuningEnP2Tag,finalEnP2);
    //
    #endif
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    ISP_YUV_PIPE
  /////////////////////////////////////////////////////////////////////////////*/

int ISP_YUV_PIPE::_config( void )
{
    #if 1
    MUINT32 i;
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MUINT32 enCrsp=0x00;
    IspTuningPackage tuningPackage;
    IspImageioPackage imageioPackage;
    //
    ISP_FUNC_DBG("+(ISP_YUV_PIPE),cq(%d),scenario(%d),sub_mode(%d),enable1(0x%08x),enable2(0x%08x)",
            this->CQ,this->drvScenario,this->p2SubMode,this->enable1,this->enable2);
    //
    //getEnTuningTag(MTRUE,(ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->p2SubMode, tuningEn1Tag, tuningEn2Tag, tuningDmaTag);
    imageioPackage.isApplyTuning = this->isApplyTuning;
    imageioPackage.cq = this->CQ;
    imageioPackage.subMode = this->p2SubMode;

    if(imageioPackage.isApplyTuning==MTRUE){
        tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->isV3);
        tuningPackage.pTuningBuf = (MUINT32*)this->pTuningIspReg;
        if (tuningPackage.pTuningBuf == NULL){
            ISP_FUNC_ERR("[Error]tuningPackage.pTuningBuf is null point");
            return 0;
        }
        ISP_FUNC_DBG("tuningUpdateBit(0x%08x),pTuningBuf(0x%08x)",tuningUpdateBit,tuningPackage.pTuningBuf);
    }
    imageioPackage.finalEn1 = this->enable1;
    imageioPackage.finalEn2 = this->enable2;
    //
    for(i=0;i<CHECK_TUNING_EN2_BITS;i++){
        if(     ((this->enable2 & gIspModuleMappingTableEn2[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableEn2[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableEn2[i].eTuningFunc2)) &&
                (gIspModuleMappingTableEn2[i].eModule==eIspModuleYuv) &&
                (gIspModuleMappingTableEn2[i].default_func!=NULL)) {
            //
            if(tuningUpdateBit & gIspModuleMappingTableEn2[i].eTuningFunc1)
                tuningPackage.eTuningCqFunc1 = gIspModuleMappingTableEn2[i].eTuningCqFunc1;
            else
                tuningPackage.eTuningCqFunc1 = CAM_DUMMY_;
            //
            if(tuningUpdateBit & gIspModuleMappingTableEn2[i].eTuningFunc2)
                tuningPackage.eTuningCqFunc2 = gIspModuleMappingTableEn2[i].eTuningCqFunc2;
            else
                tuningPackage.eTuningCqFunc2 = CAM_DUMMY_;
            //
            if(((tuningPackage.eTuningCqFunc1!=CAM_DUMMY_)||(tuningPackage.eTuningCqFunc2!=CAM_DUMMY_))&&(gIspModuleMappingTableEn2[i].eTuningCqDma!=CAM_DUMMY_))
                tuningPackage.eTuningCqDma = gIspModuleMappingTableEn2[i].eTuningCqDma;
            else
                tuningPackage.eTuningCqDma = CAM_DUMMY_;

            gIspModuleMappingTableEn2[i].default_func(imageioPackage, tuningPackage);
            //
        }
    }
    // C42 control
    if(this->enable2 & CAM_CTL_EN2_C42_EN)
    {
        this->m_pIspDrvShell->cam_isp_c42_cfg();
    }

    #else

    unsigned int enable2 = this->enable2;
    ISP_FUNC_DBG("ISP_YUV_PIPE::_config:E ");
    ISP_FUNC_DBG("[enable2:0x%08X] ",enable2);

    if ( CAM_CTL_EN2_G2C_EN&enable2 )
    {
        //it impact raw color effect
        //tuing will update this engine
        //this->m_pIspDrvShell->cam_isp_g2c_cfg();
#if defined(USING_MTK_LDVT)
        this->m_pIspDrvShell->cam_isp_g2c_cfg();
#endif
    }
    else
    {
#if 0//1//defined(USING_MTK_LDVT)
        this->m_pIspDrvShell->cam_isp_g2c_cfg();
#endif
    }

    if ( CAM_CTL_EN2_C42_EN&enable2 )
    {
        this->m_pIspDrvShell->cam_isp_c42_cfg();
    }
    ISP_FUNC_DBG("ISP_YUV_PIPE::_config:X ");
    #endif

    return 0;
}

/*
int ISP_YUV_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("ISP_YUV_PIPE::_enable:E ");
    ISP_FUNC_DBG("ISP_YUV_PIPE::_enable:X ");
    return 0;
}

int ISP_YUV_PIPE::_disable( void )
{
    ISP_FUNC_DBG("ISP_YUV_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_YUV_PIPE::_disable:X ");
     return 0;
}

int ISP_YUV_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("ISP_YUV_PIPE::checkBusy:E ");
    ISP_FUNC_DBG("ISP_YUV_PIPE::checkBusy:X ");
    return int_done_status?0:1;
}
*/
int ISP_YUV_PIPE::_write2CQ(void)
{
    #if 1
    MUINT32 tuningUpdateBit=eIspTuningMgrFunc_Null;
    MBOOL isApplyTuning;
    MUINT32 i;
    //
    ISP_FUNC_DBG("+(ISP_YUV_PIPE),cq(%d),drvScenario(%d),enable1(0x%08x),isApplyTuning(%d)",this->CQ,this->drvScenario,this->enable1,this->isApplyTuning);
    //
    isApplyTuning = this->isApplyTuning;
    if(isApplyTuning==MTRUE){
        tuningUpdateBit = m_pPhyIspDrv->getTuningUpdateFuncBit(eTuningQueAccessPath_imageio_P2, (ISP_DRV_CQ_ENUM)this->CQ, this->drvScenario, this->isV3);
        ISP_FUNC_DBG("tuningUpdateBit(0x%08x)",tuningUpdateBit);
    }
    //
    for(i=0;i<CHECK_TUNING_EN2_BITS;i++){
        if(((this->enable2 & gIspModuleMappingTableEn2[i].ctrlByte)||(tuningUpdateBit & gIspModuleMappingTableEn2[i].eTuningFunc1)||(tuningUpdateBit & gIspModuleMappingTableEn2[i].eTuningFunc2))
                &&(gIspModuleMappingTableEn2[i].eModule==eIspModuleYuv)){  // check bit en
            if(gIspModuleMappingTableEn2[i].eTuningCqFunc1!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn2[i].eTuningCqFunc1);
            if(gIspModuleMappingTableEn2[i].eTuningCqFunc2!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn2[i].eTuningCqFunc2);
            if(gIspModuleMappingTableEn2[i].eTuningCqDma!=CAM_DUMMY_)
                this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx, gIspModuleMappingTableEn2[i].eTuningCqDma);
        }
    }

    #else
    MUINT32 tuningUpdateBit=NSIspDrv_FrmB::eIspTuningMgrFunc_Null;
    MUINT32 tuningEnP2Tag=0x00, tuningEnP2DmaTag=0x00;
    MUINT32 tuningEnP2=0x00, tuningEnP2Dma=0x00;
    MUINT32 finalEnP2 = 0x00;
    MUINT32 i;
    MBOOL isApplyTuning;
    //

    ISP_FUNC_DBG("+(ISP_YUV_PIPE),cq(%d),drvScenario(%d),p2SubMode(%d)",this->CQ,this->drvScenario,this->p2SubMode);
    //



#if defined(USING_MTK_LDVT)
    //debug path, imagio will config all engine by CQ
    #define CAM_ISP_YUV_MODULE_NUM    5
    struct stCam_Id_Enable cq_info[CAM_ISP_YUV_MODULE_NUM] = { \
                                            {NSIspDrv_FrmB::CAM_ISP_C42,CAM_CTL_EN2_C42_EN},
                                            {NSIspDrv_FrmB::CAM_ISP_PCA,CAM_CTL_EN2_PCA_EN},
                                            {NSIspDrv_FrmB::CAM_ISP_PCA_CON,CAM_CTL_EN2_PCA_EN},
                                            {NSIspDrv_FrmB::CAM_ISP_SEEE,CAM_CTL_EN2_SEEE_EN},
                                            {NSIspDrv_FrmB::CAM_ISP_G2C,CAM_CTL_EN2_G2C_EN}
                                            };
#else
    //MP path, do not config engine by imageio
    #define CAM_ISP_YUV_MODULE_NUM    4
    struct stCam_Id_Enable cq_info[CAM_ISP_YUV_MODULE_NUM] = { \
                                            {NSIspDrv_FrmB::CAM_ISP_C42,CAM_CTL_EN2_C42_EN},
                                            {NSIspDrv_FrmB::CAM_ISP_PCA,CAM_CTL_EN2_PCA_EN},
                                            {NSIspDrv_FrmB::CAM_ISP_PCA_CON,CAM_CTL_EN2_PCA_EN},
                                            {NSIspDrv_FrmB::CAM_ISP_SEEE,CAM_CTL_EN2_SEEE_EN}
                                            };

#endif

    for (int i=0;i<CAM_ISP_YUV_MODULE_NUM;i++)
    {
        if ( this->enable2 & cq_info[i].en_bit)
        {
            ISP_FUNC_DBG("(0x%x)write 0x%08X to CQ: ",enable2,cq_info[i].en_bit);

            this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)cq_info[i].id);
        }
    }

    ISP_FUNC_DBG("ISP_YUV_PIPE::_write2CQ:X ");
    #endif
    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_MDP_PIPE
/////////////////////////////////////////////////////////////////////////////*/
MBOOL ISP_MDP_PIPE::createMdpMgr( void )
{
    ISP_FUNC_INF("+");
    MBOOL err = MTRUE;

   // if(g_pMdpMgr_FrmB == NULL){
        g_pMdpMgr_FrmB = MdpMgr_FrmB::createInstance();

        if(g_pMdpMgr_FrmB == NULL){
            err = MFALSE;
            ISP_FUNC_ERR("[Error]fail to create MDP instance");
        }

        // init MdpMgr
        if(MDPMGR_NO_ERROR != g_pMdpMgr_FrmB->init()){
            ISP_FUNC_ERR("[Error]g_pMdpMgr_FrmB->init fail");
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

    if(g_pMdpMgr_FrmB)
    {
        // uninit MdpMgr
        mdpreturn=g_pMdpMgr_FrmB->uninit();
        if((MDPMGR_NO_ERROR != mdpreturn) && MDPMGR_STILL_USERS != mdpreturn){
            ISP_FUNC_ERR("[Error]g_pMdpMgr_FrmB->uninit fail");
            err = MFALSE;
        }
        if(MDPMGR_STILL_USERS != mdpreturn)
        {
            g_pMdpMgr_FrmB->destroyInstance();
            g_pMdpMgr_FrmB = NULL;
        }
        else
        {
            ISP_FUNC_INF("still users");
        }
    }
    else
    {
        err = MFALSE;
        ISP_FUNC_ERR("[Error]g_pMdpMgr_FrmB = NULL");
    }

    ISP_FUNC_INF("-");

    return err;
}

int ISP_MDP_PIPE::_config( void )
{
    MUINT32 ret = ISP_ERROR_CODE_OK;
    MBOOL enPureMdpOut = MFALSE;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MUINT32 mdpCropXReg=0x00, mdpCropYReg=0x00;
    MUINT32 mdpCropReg=0x00;
    //
    this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
    ISP_FUNC_DBG("+,CQ(%d),p2Cq(%d),burstQueIdx(%d),dupCqIdx(%d),drvScenario(%d),enable2(0x%x),Dma(0x%x),isWaitBuf(%d),imgxoEnP2(0x%x),isMdpCropEn(%d)", \
            this->CQ,p2Cq,this->burstQueIdx,this->dupCqIdx,this->drvScenario,this->enable2,this->dma_enable,this->isWaitBuf,this->mdp_imgxo_p2_en,this->isMdpCropEn);

    ISP_FUNC_DBG("ISP_MDP_PIPE,crop(x:%d,y:%d,w:%d,h:%d)", \
        this->src_crop.x, \
        this->src_crop.y, \
        this->src_crop.w, \
        this->src_crop.h);


    if(this->CQ!=NSIspDrv_FrmB::ISP_DRV_CQ01 && this->CQ!=NSIspDrv_FrmB::ISP_DRV_CQ02){
        ISP_FUNC_ERR("[Error]MDP not support this cq(%d)",this->CQ);
        ret = ISP_ERROR_CODE_FAIL;
    } else{
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].ispTpipeCfgInfo.drvinfo.p2Cq = (ISP_TPIPE_P2_CQ_ENUM)p2Cq;
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].cqIdx = this->CQ;
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].drvScenario = this->drvScenario;
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].isWaitBuf = this->isWaitBuf;
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].imgxoEnP2 = this->mdp_imgxo_p2_en;
    }

    // initialize
    for(MINT32 i = 0; i < ISP_MDP_DL_NUM; i++){
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[i] = 0;
    }

    // collect crop info for MdpMgr
    gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropX        = this->src_crop.x;
    gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropFloatX = this->src_crop.floatX;
    gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropY        = this->src_crop.y;
    gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropFloatY = this->src_crop.floatY;
    gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropW        = this->src_crop.w;
    gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropH        = this->src_crop.h;

    ISP_FUNC_DBG("gpMdpMgrCfgData_FrmB[0]_src_crop : X(%u),FloatX(%u),Y(%u),FloatY(%u),W(%u),H(%u)", gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropX,
                                                                                               gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropFloatX,
                                                                                               gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropY,
                                                                                               gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropFloatY,
                                                                                               gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropW,
                                                                                               gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].srcCropH);
    // WROTO
    if(CAM_CTL_DMA_EN_WROTO_EN & this->dma_enable)
    {
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_WROTO] = 1;
        memcpy(&gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_WROTO], &this->wroto_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // WDMAO
    if(CAM_CTL_DMA_EN_WDMAO_EN & this->dma_enable)
    {
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_WDMAO] = 1;
        memcpy(&gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_WDMAO], &this->wdmao_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // ISP IMGXO
    if(enPureMdpOut == MFALSE){
        ISP_FUNC_DBG("imgxo va(0x%x),pa(0x%x),size(%d)",this->imgxo_out.memBuf.base_vAddr,this->imgxo_out.memBuf.base_pAddr,this->imgxo_out.memBuf.size);
        gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].dstPortCfg[ISP_MDP_DL_IMGXO] = 1;
        memcpy(&gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].dstDma[ISP_MDP_DL_IMGXO], &this->imgxo_out, sizeof(MdpRotDMACfg));
    } else {
        (this->isMdpCropEn==MTRUE)?(mdpCropReg=0xC000):(mdpCropReg=0x00);
        mdpCropXReg = (this->src_crop.x & 0x1fff)|(mdpCropReg)|((this->src_crop.x+this->src_crop.w-1)<<16 & 0x1FFF0000);
        mdpCropYReg = (this->src_crop.y & 0x1fff)|(mdpCropReg)|((this->src_crop.y+this->src_crop.h-1)<<16 & 0x1FFF0000);
    }
    //
    ISP_FUNC_DBG("mdpCropXReg(0x%08x),mdpCropyReg(0x%08x)",mdpCropXReg,mdpCropYReg);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_CROP_X, mdpCropXReg, ISP_DRV_USER_ISPF);
    ISP_WRITE_REG(m_pP2IspDrv,CAM_CTL_CROP_Y, mdpCropYReg, ISP_DRV_USER_ISPF);

    ISP_FUNC_DBG("-,");

    return ret;

}

int ISP_MDP_PIPE::_write2CQ(void)
{

    this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,NSIspDrv_FrmB::CAM_ISP_MDP_CROP);

    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_TUNING_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_TUNING_CTRL::_config( void )
{
    /* do nothing */
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    CAM_CDP_PIPE
  /////////////////////////////////////////////////////////////////////////////*/

/*
int CAM_CDP_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("CAM_CDP_PIPE::_enable:E ");
    ISP_FUNC_DBG("CAM_CDP_PIPE::_enable:X ");

    return 0;
}
int CAM_CDP_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("CAM_CDP_PIPE::checkBusy:E ");
    ISP_FUNC_DBG("CAM_CDP_PIPE::checkBusy:X ");
    return int_done_status?0:1;
}
*/



/**********************************************************************
*
**********************************************************************/
int CAM_CDP_PIPE::_disable( void )
{
    ISP_FUNC_DBG("CAM_CDP_PIPE::_disable:E,path(0x%x) ",this->path);
    return 0;
}

int CAM_CDP_PIPE::_config( void )
{
    ///////////////////////////////////////////////////////
    //only support cdrz + isp only mode
    //////////////////////////////////////////////////////
    bool result=false;
    CDP_DRV_IMG_SIZE_STRUCT sizeIn;
    CDP_DRV_IMG_SIZE_STRUCT sizeOut;
    CDP_DRV_IMG_CROP_STRUCT crop;

        sizeIn.Width   = this->cdrz_in.w;
        sizeIn.Height  = this->cdrz_in.h;
        sizeOut.Width  = this->cdrz_out.w;
        sizeOut.Height = this->cdrz_out.h;

        // crop
        crop.Width.Start  = this->cdrz_crop.x;
        crop.Width.Size   = this->cdrz_crop.w;
        crop.Height.Start = this->cdrz_crop.y;
        crop.Height.Size  = this->cdrz_crop.h;

    result=this->m_pIspDrvShell->m_pP2CdrzDrv->CDRZ_Config(CDP_DRV_MODE_TPIPE,sizeIn,sizeOut,crop);

    if(!result)
    {
        ISP_FUNC_ERR("CAM_CDP_PIPE::_config: ERR ");
        return -1;
    }

    ISP_FUNC_DBG("CAM_CDP_PIPE::_config:X ");
    return 0;
}

int CAM_CDP_PIPE::_write2CQ( void )
{
    ISP_FUNC_DBG("CAM_CDP_PIPE::_write2CQ:E ");
    /*
    //CDP
    CAM_CDP_CDRZ,        //15   15004B00~15004B38
    CAM_CDP_CURZ_EIS,    //23   15004B40~15004B98
    CAM_CDP_PRZ,         //15   15004BA0~15004BD8
    CAM_CDP_VRZ,         //13   15004BE0~15004C10
    CAM_CDP_FE,          //4    15004C20~15004C2C
    CAM_CDP_CRSP,        //4    15004C30~15004C3C //->use default is ok.
    CAM_CDP_VRZO,        //26   15004C50~15004CB4
    CAM_CDP_VIDO,        //29   15004CC0~15004D30
    CAM_CDP_DISPO,       //26   15004D40~15004DA4
    */
    if(this->conf_cdrz)
    {
        //cdrz
        ISP_FUNC_DBG("CQ:push CAM_CDP_CDRZ ");
        this->m_pIspDrvShell->cqAddModule((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ,this->burstQueIdx,this->dupCqIdx,(NSIspDrv_FrmB::CAM_MODULE_ENUM)NSIspDrv_FrmB::CAM_CDP_CDRZ);
    }

    ISP_FUNC_DBG("CAM_CDP_PIPE::_write2CQ:X ");
    return 0;
}
//
int CAM_CDP_PIPE::_setZoom( void )
{
unsigned int *data_ptr = NULL;

    ISP_FUNC_DBG(":E ");
    ISP_FUNC_DBG("0x%08x/0x%08x ",this->conf_cdrz,this->conf_rotDMA);

    if ( this->conf_cdrz )
    {
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
    //ISP_FUNC_INF("this->burstQueIdx(%d),CQ(%d),this->dupCqIdx(%d)",this->burstQueIdx,this->CQ,this->dupCqIdx);

    this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((NSIspDrv_FrmB::ISP_DRV_CQ_ENUM)this->CQ, p2Cq);
    pDst = &gpMdpMgrCfgData_FrmB[this->burstQueIdx][p2Cq][this->dupCqIdx].ispTpipeCfgInfo;

    /* update imageio parameters */
    pDst->top.scenario = pSrcImageio->top.scenario;    ///< Isp pipeline scenario
    pDst->top.mode = pSrcImageio->top.mode;        ///< Isp pipeline sub mode
    pDst->top.debug_sel = pSrcImageio->top.debug_sel;   ///< Debug mode selection for all scenarios
    pDst->top.pixel_id = pSrcImageio->top.pixel_id;    ///< Bayer sequence , it is for RAW and RGB module
    pDst->top.cam_in_fmt = pSrcImageio->top.cam_in_fmt;  ///< Pass2 path input format
    pDst->top.tcm_load_en = pSrcImageio->top.tcm_load_en; ///< TDR manual setting enable
    pDst->top.ctl_extension_en = pSrcImageio->top.ctl_extension_en;///< 0: normal,suggested
    pDst->top.rsp_en = pSrcImageio->top.rsp_en;       ///< Resample tile enable, for direct link MDP use
    pDst->top.mdp_crop_en = pSrcImageio->top.mdp_crop_en;  ///< MDP corp enable
    pDst->top.imgi_en = pSrcImageio->top.imgi_en;      ///< enable switch of IMGI
    pDst->top.lce_en = 0; // LCE do not exist in 82
    pDst->top.lcei_en = 0; // LCE do not exist in 82
    pDst->top.lsci_en = pSrcImageio->top.lsci_en;     ///< enable switch of LSCI
    pDst->top.unp_en = pSrcImageio->top.unp_en;      ///< enable switch of unpacked
    pDst->top.bnr_en = pSrcImageio->top.bnr_en;      ///< enable switch of BNR
    pDst->top.lsc_en = pSrcImageio->top.lsc_en;      ///< enable switch of LSC
    pDst->top.sl2_en = pSrcImageio->top.sl2_en;      ///< enable switch of SL2
    pDst->top.c24_en = pSrcImageio->top.c24_en;     ///< enable switch of c24
    pDst->top.cfa_en = pSrcImageio->top.cfa_en;     ///< enable switch of cfa
    pDst->top.c42_en = pSrcImageio->top.c42_en;     ///< enable switch of c42
    pDst->top.nbc_en = pSrcImageio->top.nbc_en;     ///< enable switch of nbc
    pDst->top.seee_en = pSrcImageio->top.seee_en;    ///< enable switch of SEEE
    pDst->top.imgo_en = pSrcImageio->top.imgo_en;    ///< enable switch of IMGO
    pDst->top.img2o_en = pSrcImageio->top.img2o_en;   ///< enable switch of IMG2O
    pDst->top.cdrz_en = pSrcImageio->top.cdrz_en;   ///< enable switch of CDRZ
    pDst->top.mdp_sel = pSrcImageio->top.mdp_sel;   ///< 0 : from CDRZ 1: from YUV
    pDst->top.interlace_mode = pSrcImageio->top.interlace_mode;///< doesn't use it

    ISP_FUNC_DBG("cfa(%d)",pDst->top.cfa_en);


    /* config tpipe setting */
    ::memcpy( (char*)&pDst->imgi, (char*)&pSrcImageio->imgi, sizeof(ISP_TPIPE_CONFIG_IMGI_STRUCT));
    ::memcpy( (char*)&pDst->lsci, (char*)&pSrcImageio->tuningFunc.lsci, sizeof(ISP_TPIPE_CONFIG_LSCI_STRUCT));
    ::memcpy( (char*)&pDst->bnr, (char*)&pSrcImageio->tuningFunc.bnr, sizeof(ISP_TPIPE_CONFIG_BNR_STRUCT));
    ::memcpy( (char*)&pDst->lsc, (char*)&pSrcImageio->tuningFunc.lsc, sizeof(ISP_TPIPE_CONFIG_LSC_STRUCT));
    ::memcpy( (char*)&pDst->nbc, (char*)&pSrcImageio->tuningFunc.nbc, sizeof(ISP_TPIPE_CONFIG_NBC_STRUCT));
    ::memcpy( (char*)&pDst->seee, (char*)&pSrcImageio->tuningFunc.seee, sizeof(ISP_TPIPE_CONFIG_SEEE_STRUCT));
    ::memcpy( (char*)&pDst->imgo, (char*)&pSrcImageio->imgo, sizeof(ISP_TPIPE_CONFIG_IMGO_STRUCT));
    ::memcpy( (char*)&pDst->cdrz, (char*)&pSrcImageio->cdrz, sizeof(ISP_TPIPE_CONFIG_CDRZ_STRUCT));
    ::memcpy( (char*)&pDst->img2o, (char*)&pSrcImageio->img2o, sizeof(ISP_TPIPE_CONFIG_IMG2O_STRUCT));
    ::memcpy( (char*)&pDst->cfa, (char*)&pSrcImageio->tuningFunc.cfa, sizeof(ISP_TPIPE_CONFIG_CFA_STRUCT));
    ::memcpy( (char*)&pDst->sl2, (char*)&pSrcImageio->tuningFunc.sl2, sizeof(ISP_TPIPE_CONFIG_SL2_STRUCT));


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

int CAM_TDRI_PIPE::_config( void )
{
    MBOOL tdri_result = MTRUE;
    //
    ISP_FUNC_DBG("+,enTdri(%d),tcm_en(0x%08x),tdr_en(0x%08x),cq(%d),dupCqIdx(%d),burstQueIdx(%d)",this->enTdri,this->tcm_en,this->tdr_en,this->CQ,this->dupCqIdx,this->burstQueIdx);
    //
    ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_TILE, this->tdr_en, ISP_DRV_USER_ISPF);    //0x4050
    ISP_WRITE_REG(m_pP2IspDrv, CAM_CTL_TCM_EN, this->tcm_en, ISP_DRV_USER_ISPF);  //0x4054

    if ( this->enTdri ) {
        configTdriSetting(&tdri);
    }
    ISP_FUNC_DBG("-,");

    return 0;
}

int CAM_TDRI_PIPE::_write2CQ( void )
{
    ISP_FUNC_DBG("CAM_TDRI_PIPE::_write2CQ");

    return 0;
}




/*/////////////////////////////////////////////////////////////////////////////
    ISP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_BUF_CTRL::init( MUINT32 dmaChannel )
{
    ISP_FUNC_DBG("ISP_BUF_CTRL: E");
    //avoid to reset working path, can't reset all dma one at a time, e.g.:PIP preview/capture/vr ...etc

    if(this->path == ISP_PASS1){
        memset (&m_p1DeqedBufHeaderL[_imgo_], 0, sizeof(stISP_BUF_HEADER_LIST));
        memset (&m_p1DeqedBufHeaderL[_img2o_], 0, sizeof(stISP_BUF_HEADER_LIST));
        m_hwbufL[_imgo_].bufInfoList.clear();
        m_hwbufL[_img2o_].bufInfoList.clear();
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
    ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl;
    MUINT32 bWaitBufRdy;
    MUINT32 loopCnt = 0;
    #define _CAM_INT_WAIT_TIMEOUT_MS (1000)
    #define MAX_CNT (CAM_INT_WAIT_TIMEOUT_MS/_CAM_INT_WAIT_TIMEOUT_MS)
    if(-1 == rt_dma)
    {
        ISP_FUNC_ERR("dma channel error ");
        return MFALSE;
    }

    if(_imgo_ == rt_dma || _img2o_ == rt_dma)
    {
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_IS_RDY_FRMB;
        buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
        buf_ctrl.pExtend = (unsigned char *)&bWaitBufRdy;
        ISP_FUNC_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%p)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.pExtend);

        ret = this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
        if(MFALSE == ret)
        {
            ISP_FUNC_ERR("rtBufCtrl fail:ISP_RT_BUF_CTRL_IS_RDY");
        }

        if(bWaitBufRdy)
        {
            ISP_FUNC_DBG("wait p1_done");
            //NO BUFFER FILLED
            //wait pass1 done
            NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irq_TG_DONE;
               irq_TG_DONE.Clear=NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_WAIT;
            irq_TG_DONE.UserInfo.Type=NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
            irq_TG_DONE.UserInfo.Status=ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
            irq_TG_DONE.UserInfo.UserKey=0;
            irq_TG_DONE.Timeout=_CAM_INT_WAIT_TIMEOUT_MS;
            irq_TG_DONE.bDumpReg=0xFE;

            while(loopCnt++ < MAX_CNT){//(this->m_pPhyIspDrv->registerIrq(irq_TG_DONE)){
                if(loopCnt == MAX_CNT)
                    irq_TG_DONE.bDumpReg = 0x1;
                buf_ctrl.ctrl = ISP_RT_BUF_CTRL_IS_RDY_FRMB;
                this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
                if(bWaitBufRdy == MFALSE){
                    ret = MTRUE;
                    break;
                }
                ret = this->m_pPhyIspDrv->waitIrq( &irq_TG_DONE );
                if ( MFALSE == ret ) {
                    ISP_FUNC_INF("still no p1 done,dma[%d]",rt_dma);
                    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CUR_STATUS_FRMB;
                    buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
                    buf_ctrl.pExtend = (unsigned char *)&bWaitBufRdy;//useless
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);
                }
                else
                    break;//wait successfully
                irq_TG_DONE.Clear = ISP_DRV_IRQ_CLEAR_NONE;// waitirq chg to use non-clr wait after 1st wait
            }
            if(ret != MTRUE){
                MUINT32 _ret2 = MTRUE;
                ISP_DRV_WAIT_IRQ_STRUCT irq_TG_SOF ;
                irq_TG_SOF.Clear=NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_WAIT;
                irq_TG_SOF.UserInfo.Type=NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
                irq_TG_SOF.UserInfo.Status=ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST;
                irq_TG_SOF.UserInfo.UserKey=0;
                irq_TG_SOF.Timeout=300;
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
                        ISP_FUNC_ERR("wait p1 done fail,user space enque record(cur dma:%d imgo cnt:%d,img2o cnt:%d)\n",rt_dma,P1DeQCnt[_imgo_],P1DeQCnt[_img2o_]);
                }
            }
         }
    }
    if(ret != MTRUE){
        ISP_FUNC_ERR("CQ0 dump:\n");
        for(int i=0;i<3;i++){
            ISP_FUNC_ERR("dup idx:0x%x\n",i);
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)ISP_DRV_CQ0,0, i,(ISP_DRV_CQ_CMD_DESC_STRUCT*)this->m_pPhyIspDrv->getCQDescBufVirAddr(ISP_DRV_CQ0,0,i),\
                        (MUINT32*)this->m_pPhyIspDrv->getCQVirBufVirAddr(ISP_DRV_CQ0,0,i));
        }
    }
    return ret;
}

//
/*
description: move filled current buffer to empty buffer list
*/
#define _DMA_TRANS(rt_dma,path){\
    switch(rt_dma){\
        case _imgo_:\
        case _img2o_:\
            path = 1;\
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
#define ENABLE_ATTACH_BUF_INFO 0

//
MINT32 ISP_BUF_CTRL::enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data,MBOOL bImdMode )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    stISP_BUF_HEADER_LIST *plist = NULL;
    MUINT32 i = ISP_BUF_INFO_NODE_MAX_CNT;

    if (_rt_dma_max_ <= rt_dma) {
        ISP_FUNC_ERR("enqueueHwBuf:: Invalid rt_dma=%d", rt_dma);
        return -1;
    }
#if (ENABLE_ATTACH_BUF_INFO)
    returnDeqBufInfo(rt_dma,&bufInfo);
    if (0){
        //only for dbg
        /*Mark by Ryan - No use in mt6582
        if ((ISP_PASS1 == this->path)||(ISP_PASS1_D == this->path_D)) {
            this->ispDrvSwitch2Virtual(this->CQ,this->CQ_D,this->burstQueIdx,0);
        }
        */

        //ISP_FUNC_DBG("[MyDeQ]EnQHwBuf::ispDrv=0x%x/0x%x, CmdQ-IN/OUT Z=[0x%x,0x%x], ZD=[0x%x,0x%x], /// HW Z=[0x%x,0x%x]. ZD=[0x%x,0x%x]", \
        ISP_QUE_INF(rt_dma,"[MyDeQ]EnQHwBuf::ispDrv=0x%x/0x%x, CmdQ-IN/OUT Z=[0x%x,0x%x], HW Z=[0x%x,0x%x].", \
        m_pP1IspDrv, m_pP1IspDrvD, \
        ISP_READ_REG_NOPROTECT(m_pP1IspDrv, CAM_hrz_IN_IMG),\
        ISP_READ_REG_NOPROTECT(m_pP1IspDrv, CAM_hrz_OUT_IMG), \
        ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_hrz_IN_IMG),\
        ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_hrz_OUT_IMG));

        /*ISP_READ_REG_NOPROTECT(m_pP1IspDrvD, CAM_hrz_D_IN_IMG), \
        ISP_READ_REG_NOPROTECT(m_pP1IspDrvD, CAM_hrz_D_OUT_IMG),\
        ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_hrz_D_IN_IMG),\
        ISP_READ_REG_NOPROTECT(m_pPhyIspDrv, CAM_hrz_D_OUT_IMG)*/
    }
#endif
    if(bImdMode == MTRUE)
        return  enqueueHwBuf( dmaChannel, bufInfo ,0,0,bImdMode);
    else
        return  enqueueHwBuf( dmaChannel, bufInfo);
}

MINT32 ISP_BUF_CTRL::enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo, MINT32 enqueCq, MINT32 dupCqIdx,MBOOL bImdMode)
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 end = 0;
    IMEM_BUF_INFO buf_info;
    ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl;
    ISP_RT_BUF_INFO_STRUCT_FRMB rt_buf_info;
    ISP_RT_BUF_INFO_STRUCT_FRMB ex_rt_buf_info;
    MUINT32 size;
    MUINT32 fbc[2];
    MUINT32 vf=0;

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
    if ( ISP_PASS1   == this->path ) {
        //check is full
        //ISP_FUNC_INF("[js_test]:sizeof(ISP_RT_BUF_STRUCT)(%d) ",sizeof(ISP_RT_BUF_STRUCT));
        //

        if( NULL != bufInfo.next ){
            ISP_QUE_DBG(rt_dma,"dma(%d),id(%d),size(0x%x),VA(0x%x),PA(0x%x),S/C(%d/%d),bufidx(%d),replace:new(0x%x),bufidx(%d),bImdMode(%d)",\
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
            ISP_QUE_DBG(rt_dma,"dma(%d),id(%d),size(0x%x),VA(0x%x),PA(0x%x),S/C(%d/%d),bufidx(%d),bImdMode(%d)",\
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
             _img2o_ == rt_dma) {
            if( (_imgo_ == rt_dma) || (_img2o_ == rt_dma) ){
                fbc[0] = readReg(0x000040f4);
                fbc[1] = readReg(0x000040f8);
                vf = readReg(0x00004414);
            }

            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE_FRMB;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.pExtend = (unsigned char *)&size;
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
        //mark to avoid memory leak
        //m_hwbufL[rt_dma].bufInfoList.push_back(bufInfo);
        //
        //put to ring buffer in kernel
        rt_buf_info.memID = buf_info.memID;
        rt_buf_info.size = buf_info.size;
        rt_buf_info.base_vAddr = buf_info.virtAddr;
        rt_buf_info.base_pAddr = buf_info.phyAddr;
        rt_buf_info.bufIdx     = bufInfo.mBufIdx;//FLORIA enable
        //ISP_QUE_INF(rt_dma,"rt_buf_info.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",rt_buf_info.memID,rt_buf_info.size,rt_buf_info.base_vAddr,rt_buf_info.base_pAddr);
        if ( 0 == rt_buf_info.base_pAddr ) {
            ISP_FUNC_ERR("NULL PA");
        }
        if(bImdMode == MTRUE){
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_ENQUE_IMD_FRMB;
        }
        else
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_ENQUE_FRMB;

        buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
        buf_ctrl.data_ptr = (ISP_RT_BUF_INFO_STRUCT_FRMB *)&rt_buf_info;
        buf_ctrl.ex_data_ptr = (ISP_RT_BUF_INFO_STRUCT_FRMB *)0;
        //ISP_QUE_INF(rt_dma,"[rtbc][ENQUQ]+(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
        //enque exchanged buffer
        if ( NULL != bufInfo.next ) {
            ex_rt_buf_info.memID = bufInfo.next->memID;
            ex_rt_buf_info.size = bufInfo.next->size;
            ex_rt_buf_info.base_vAddr = bufInfo.next->base_vAddr;
            ex_rt_buf_info.base_pAddr = bufInfo.next->base_pAddr;
            ex_rt_buf_info.bufIdx     = bufInfo.next->mBufIdx;//for replace buffer //FLORIA enable

            //
            //ISP_QUE_INF(rt_dma,"exchange 1st buf. by 2nd buf. and enque it.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",ex_rt_buf_info.memID,ex_rt_buf_info.size,ex_rt_buf_info.base_vAddr,ex_rt_buf_info.base_pAddr);
            //
            if ( 0 == ex_rt_buf_info.base_pAddr ) {
                ISP_FUNC_ERR("NULL Ex-PA");
            }
            //
            //buf_ctrl.ctrl = ISP_RT_BUF_CTRL_EXCHANGE_ENQUE;
            buf_ctrl.ex_data_ptr = (ISP_RT_BUF_INFO_STRUCT_FRMB *)&ex_rt_buf_info;
            //
        }
        //
        if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
            ISP_FUNC_ERR("ERROR:rtBufCtrl");
            return -1;
        }
        P1DeQCnt[rt_dma] += 1;
        if( (_imgo_ == rt_dma) || (_img2o_ == rt_dma) ){
            ISP_QUE_DBG(rt_dma,"imgo/img2o, vf=0x%x fbc: 0x%x/0x%x_0x%x/0x%x\n",vf,fbc[0],fbc[1],readReg(0x000040f4),readReg(0x000040f8));
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
        bufInfo.memID = (MUINT32)buf_info.memID;
        bufInfo.size =  (MUINT32)buf_info.size;
        bufInfo.base_vAddr =  (MUINT32)buf_info.virtAddr;
        bufInfo.base_pAddr =  (MUINT32)buf_info.phyAddr;
        bufInfo.status = ISP_BUF_EMPTY;
        //

        m_p2HwBufL[p2Cq][dupCqIdx][rt_dma].bufInfoList.push_back(bufInfo);
        //
        ISP_FUNC_DBG("enqueCq(%d),p2Cq(%d),dupCqIdx(%d),dma(%d),memID(0x%x),size(%d),vAddr(0x%x),pAddr(0x%x)", \
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

//
MINT32
ISP_BUF_CTRL::
dequeueMdpFrameEnd( MINT32 drvScenario )
{

        ISP_DRV_P2_CQ_ENUM p2Cq;
    if(!P2_UT_BypassMDP_TPIPE)
    {
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->dequeCq, p2Cq);
        if(MDPMGR_NO_ERROR != g_pMdpMgr_FrmB->dequeueFrameEnd(gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx])){
            ISP_FUNC_ERR("g_pMdpMgr_FrmB->dequeueMdpFrameEnd for dispo fail");
            return -1;
        }
    }
    else
    {//temp simulate, to ensure that another trigger operation should be executed after the previous frame is dequeued done
        pthread_mutex_lock(&p2_enque_deque_Mutex);
        enque_running=false;
            pthread_mutex_unlock(&p2_enque_deque_Mutex);
            ISP_FUNC_INF("post start Semmmmmmmm");
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
                    ISP_QUE_DBG(rt_dma,"dma:0x%x: find no match baseaddr, 0x%x\n",rt_dma,bufInfo->next->base_vAddr);
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
    /*
    if (0) {
        ptr = &plist->mN[plist->mIdx].mInfo;
        plist->mN[i].mOccupied = MTRUE;
        plist->mIdx = (plist->mIdx+1) % ISP_BUF_INFO_NODE_MAX_CNT;

        ISP_QUE_DBG(rt_dma,"saveDeqBufInfo[D][Error] plist=0x%x,ptr=0x%8x, mIdx=%d", plist,ptr, plist->mIdx);
    }
    if(ptr!= NULL)
        ISP_QUE_DBG(rt_dma,"[MyDeQ]saveDeqBufInfo: node=0x%x", ptr->base_pAddr);
        */
    return ptr;
}


//
/*
description: move FILLED buffer from hw to sw list
called at passx_done
*/
// TODO: check
#if 0 //disable by FLORIA  -> using two parameter
MINT32 ISP_BUF_CTRL::dequeueHwBuf( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 count = 1;
    MUINT32 start;
    //ISP_RT_BUF_STRUCT* pstRTBuf = (ISP_RT_BUF_STRUCT*)this->m_pIspDrvShell->m_pPhyIspDrv->m_pRTBufTbl;
    stISP_BUF_INFO bufInfo;
    ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl;
    ISP_DEQUE_BUF_INFO_STRUCT_FRMB deque_buf;

    ISP_FUNC_DBG("path(%d)\n",this->path);
    ISP_FUNC_DBG("dmaChannel(0x%x))\n",dmaChannel);
    ISP_FUNC_DBG("rt_dma(0x%x))\n",rt_dma);

    Mutex::Autolock lock(queHwLock);

    if( -1 == rt_dma )
    {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }

    if( ISP_PASS1 == this->path )
    {
        if( _imgo_ == rt_dma || _img2o_ == rt_dma )
        {
            //deque filled buffer
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_DEQUE_FRMB;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.pExtend = (unsigned char *)&deque_buf;

            ISP_FUNC_DBG("dma(%d),ctrl(%d),id(%d),ptr(0x%p)",rt_dma,buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.pExtend);
            if( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) )
            {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }

            count = deque_buf.count;
            ISP_FUNC_DBG("deque_buf.count(%d)",deque_buf.count);
            if ( ISP_RT_BUF_SIZE < count )
            {
                ISP_FUNC_ERR("ERROR:deque_buf.count(%d)",deque_buf.count);
                return -1;
            }
            //
            m_swbufL[rt_dma].bufInfoList.clear();
            //
            for(MUINT32 i=0;i<count;i++)
            {
                ISP_FUNC_DBG("i(%d),dma(%d),id(0x%x),size(0x%x),VA(0x%x),PA(0x%x),size(%d)",i,rt_dma,deque_buf.data[i].memID,deque_buf.data[i].size,deque_buf.data[i].base_vAddr,deque_buf.data[i].base_pAddr,m_swbufL[rt_dma].bufInfoList.size());
                bufInfo.status = ISP_BUF_FILLED;
                bufInfo.memID = deque_buf.data[i].memID;
                bufInfo.size = deque_buf.data[i].size;
                bufInfo.base_vAddr = deque_buf.data[i].base_vAddr;
                bufInfo.base_pAddr = deque_buf.data[i].base_pAddr;
                bufInfo.timeStampS = deque_buf.data[i].timeStampS;
                bufInfo.timeStampUs = deque_buf.data[i].timeStampUs;
                m_swbufL[rt_dma].bufInfoList.push_back(bufInfo);
            }
        }
    }
    else
    {
        //pass2
        while(count--)
        {
            ISP_FUNC_DBG(" : pass2 (count)=(0x%8x)\n",count);
            ISP_FUNC_DBG(" : m_hwbufL[%d].bufInfoList.size()=(0x%8x)\n",rt_dma,m_hwbufL[rt_dma].bufInfoList.size());

            if(m_hwbufL[rt_dma].bufInfoList.size() && ISP_PASS2 == this->path)
            {
                //change type
                m_hwbufL[rt_dma].bufInfoList.front().status = ISP_BUF_FILLED;

                if(rt_dma==_dispo_)
                {
                    {
                        ISP_FUNC_ERR("g_pMdpMgr_FrmB->dequeueBuf for dispo fail");
                        return -1;
                    }
                }

                if(rt_dma==_vido_)
                {
                    {
                        ISP_FUNC_ERR("g_pMdpMgr_FrmB->dequeueBuf for vido fail");
                        return -1;
                    }
                }

                if(rt_dma==_imgi_)
                {
                }

                //m_hwbufL[rt_dma].bufInfoList.front().timeStampS = pstRTBuf->ring_buf[rt_dma].data[(start-count-1)%ISP_RT_BUF_SIZE].timeStampS;
                //m_hwbufL[rt_dma].bufInfoList.front().timeStampUs = pstRTBuf->ring_buf[rt_dma].data[(start-count-1)%ISP_RT_BUF_SIZE].timeStampUs;

                //add to sw buffer list
                m_swbufL[rt_dma].bufInfoList.push_back(m_hwbufL[rt_dma].bufInfoList.front());
                //delete fomr hw list
                m_hwbufL[rt_dma].bufInfoList.pop_front();
                //
                ISP_FUNC_DBG("dma(%d),swsize(%d) ",rt_dma,m_swbufL[rt_dma].bufInfoList.size());

#if defined(__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
                ISP_FUNC_DBG("unmapPhyAddr");
                //m4u only
                IMEM_BUF_INFO buf_info;
                buf_info.memID = m_swbufL[rt_dma].bufInfoList.back().memID;
                buf_info.size = m_swbufL[rt_dma].bufInfoList.back().size;
                buf_info.virtAddr = m_swbufL[rt_dma].bufInfoList.back().base_vAddr;
                buf_info.phyAddr = m_swbufL[rt_dma].bufInfoList.back().base_pAddr;
                //free mva
                if ( this->m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info) )
                {
                    ISP_FUNC_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
                    return -1;
                }
#endif

            }
            else if(ISP_PASS2B == this->path)
            {
                MUINT32 ret;
                stISP_BUF_INFO bufInfo2B;

                bufInfo2B.status = ISP_BUF_FILLED;

                ISP_FUNC_DBG("ISP_PASS2B dequeue");
                if(rt_dma==_dispo_)
                {


                }

                if(rt_dma==_vido_)
                {
                    if(ret == MDPMGR_VSS_DEQ_FALSE_FAIL)
                    {
                        return -2;
                    }
                    else if(ret != MDPMGR_NO_ERROR)
                    {
                        ISP_FUNC_ERR("g_pMdpMgr_FrmB->dequeueBuf for vido fail");
                        return -1;
                    }

                }

                if(rt_dma==_imgi_)
                {
                    {
                        ISP_FUNC_ERR("g_pMdpMgr_FrmB->dequeueBuf for src fail");
                        return -1;
                    }

                }

                //add to sw buffer list
                m_swbufL[rt_dma].bufInfoList.push_back(bufInfo2B);

#if defined(__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
                ISP_FUNC_DBG("ISP_PASS2B unmapPhyAddr");

                //m4u only
                IMEM_BUF_INFO buf_info;

                buf_info.memID    = m_swbufL[rt_dma].bufInfoList.back().memID;
                buf_info.size     = m_swbufL[rt_dma].bufInfoList.back().size;
                buf_info.virtAddr = m_swbufL[rt_dma].bufInfoList.back().base_vAddr;
                buf_info.phyAddr  = m_swbufL[rt_dma].bufInfoList.back().base_pAddr;

                //free mva
                if(this->m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info))
                {
                    ISP_FUNC_ERR("ERROR:ISP_PASS2B m_pIMemDrv->unmapPhyAddr");
                    return -1;
                }
#endif
            }
            else
            {
                ISP_FUNC_ERR("empty HW buffer");
                return -1;
            }
        }
    }

    ISP_FUNC_INF("path(%d),start(NA),SCIO(NA),En1(0x%x),En2(0x%x),En2(0x%x),fmt_sel(0x%x),ctl_sel(0x%x),tcm_en(0x%x),mux_sel(0x%x),mux_sel2(0x%x),mux_conf(0x%x)",this->path,
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004004),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004008),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x0000400C),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004010),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004014),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004054),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004074),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004078),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x0000407C));
    return 0;
}
#endif
//
EIspRetStatus ISP_BUF_CTRL::dequeueHwBuf( MUINT32 dmaChannel, stISP_FILLED_BUF_LIST& bufList )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 count = 1;
    MUINT32 start;
    EIspRetStatus ret = eIspRetStatus_Success;
    //ISP_RT_BUF_STRUCT* pstRTBuf = (ISP_RT_BUF_STRUCT*)this->m_pIspDrvShell->m_pPhyIspDrv->m_pRTBufTbl;
    //stISP_BUF_INFO bufInfo;
    ISP_BUFFER_CTRL_STRUCT_FRMB buf_ctrl;
    ISP_DEQUE_BUF_INFO_STRUCT_FRMB deque_buf;

    //ISP_QUE_INF(rt_dma,"dmaChannel(%d),path(%d/%d)",dmaChannel,this->path,this->path_D);
    //Mutex::Autolock lock(queHwLock);
    //
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error(0x%x)\n ",rt_dma);
        ret = eIspRetStatus_Failed;
        goto EXIT;
    }
    //
    if ( ISP_PASS1   == this->path) {
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
             _img2o_ == rt_dma
           ) {
               UINT32 fbc[2];
               UINT32 vf=0;

            if( (_imgo_ == rt_dma) || (_img2o_ == rt_dma) ){
                fbc[0] = readReg(0x000040f4);//imgo_FBC
                fbc[1] = readReg(0x000040f8);//img2o_FBC
                vf = readReg(0x00004414);
            }
            //deque filled buffer
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_DEQUE_FRMB;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.pExtend = (unsigned char *)&deque_buf;
            //ISP_QUE_INF(rt_dma,"dma(%d),ctrl(%d),id(%d),ptr(0x%x)",rt_dma,buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                ret = eIspRetStatus_Failed;
                goto EXIT;
            }
            P1DeQCnt[rt_dma] -= 1;
            if( (_imgo_ == rt_dma) || (_img2o_ == rt_dma) ){
                ISP_QUE_DBG(rt_dma,"imgo/img2o: vf=0x%x, fbc: 0x%x/0x%x_0x%x/0x%x\n",vf,fbc[0],fbc[1],readReg(0x000040f4),readReg(0x000040f8));
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
                ISP_QUE_DBG(rt_dma,"i(%d),dma(%d),id(0x%x),size(0x%x),xsize(0x%x),VA(0x%x),PA(0x%x),count(%d),cur sof(%d),frm_cnt(%d),mag(0x%x)",
                    i, \
                    rt_dma, \
                    deque_buf.data[i].memID, \
                    deque_buf.data[i].size, \
                    deque_buf.data[i].image.xsize,\
                    deque_buf.data[i].base_vAddr, \
                    deque_buf.data[i].base_pAddr, \
                    deque_buf.count,\
                    deque_buf.sof_cnt,\
                    deque_buf.data[i].image.frm_cnt,\
                    deque_buf.data[i].image.m_num_0);
                //
                bufInfo->status      = ISP_BUF_FILLED;
                bufInfo->memID       = deque_buf.data[i].memID;
                bufInfo->size        = deque_buf.data[i].size;
                bufInfo->base_vAddr  = deque_buf.data[i].base_vAddr;
                bufInfo->base_pAddr  = deque_buf.data[i].base_pAddr;
                bufInfo->timeStampS  = deque_buf.data[i].timeStampS;
                bufInfo->timeStampUs = deque_buf.data[i].timeStampUs;

                bufInfo->img_w       = deque_buf.data[i].image.w;    //current w is tg grab width, hrzo plz reference dstW
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
                bufInfo->hrz_info.srcX = deque_buf.data[i].HrzInfo.srcX;
                bufInfo->hrz_info.srcY = deque_buf.data[i].HrzInfo.srcY;
                bufInfo->hrz_info.srcW = deque_buf.data[i].HrzInfo.srcW;
                bufInfo->hrz_info.srcH = deque_buf.data[i].HrzInfo.srcH;
                bufInfo->hrz_info.dstW = deque_buf.data[i].HrzInfo.dstW;
                bufInfo->hrz_info.dstH = deque_buf.data[i].HrzInfo.dstH;
                bufInfo->dma_crop.x= deque_buf.data[i].dmaoCrop.x;
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
                bufInfo->dma = dmaChannel;


                if((bufInfo->timeStampS==0) && (bufInfo->timeStampUs==0) ){
                    ISP_FUNC_ERR("error:timestamp err\n");
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();

                    ISP_FUNC_ERR("\n CQ0:\n");
                    for(int i=0;i<3;i++){
                        this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)ISP_DRV_CQ0,0, i,(ISP_DRV_CQ_CMD_DESC_STRUCT*)this->m_pPhyIspDrv->getCQDescBufVirAddr(ISP_DRV_CQ0,0,i),\
                        (MUINT32*)this->m_pPhyIspDrv->getCQVirBufVirAddr(ISP_DRV_CQ0,0,i));
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
                bufInfo->header_info = (MVOID*)bufList.pBufList; //point to self
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
        MDPMGR_DEQUEUE_INDEX_FRMB mdpQueueIdx;
        MBOOL isDequeMdp = MTRUE;
        ISP_DRV_P2_CQ_ENUM p2Cq;
        MUINT32 bufSize = 0;
        MUINT32 mdpMgrCfgDataImgxoEnP2;
        MDPMGR_CFG_STRUCT_FRMB mdpMgrCfgStructData;
        //
        this->m_pIspDrvShell->m_pPhyIspDrv_bak->mapCqToP2Cq((ISP_DRV_CQ_ENUM)this->dequeCq, p2Cq);

        while(count--) {
            this->m_pIspDrvShell->gPass2Lock.lock();
            bufSize = m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.size();
            mdpMgrCfgDataImgxoEnP2 = gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].imgxoEnP2;
            memcpy(&mdpMgrCfgStructData, &gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx], sizeof(MDPMGR_CFG_STRUCT_FRMB));
            this->m_pIspDrvShell->gPass2Lock.unlock();
            if (bufSize) {

        if(!P2_UT_BypassMDP_TPIPE)
               {
                    //
                    // MDP flow
                    //
                    ISP_FUNC_DBG("drvScenario(%d),rt_dma(%d)",this->drvScenario,rt_dma);
                    switch(rt_dma){
                        case _imgi_:
                            if(this->drvScenario != NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_IP){
                                mdpQueueIdx = MDPMGR_DEQ_SRC;
                            } else {
                                isDequeMdp = MFALSE;
                            }
                            break;
                        case _vido_:
                            mdpQueueIdx = MDPMGR_DEQ_WROT;
                            break;
                        case _dispo_:
                            mdpQueueIdx = MDPMGR_DEQ_WDMA;
                            break;
                        case _img2o_: /* for compatible MDP flow */
                            if(mdpMgrCfgDataImgxoEnP2 == CAM_CTL_DMA_EN_IMG2O_EN){
                                ISP_QUE_INF(rt_dma,"MDP-IMGXO-IMG2O");
                                mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                            }else{
                                isDequeMdp = MFALSE;
                            }
                            break;
                        //case _img3o_: /* for compatible MDP flow */
                        //    if(mdpMgrCfgDataImgxoEnP2 == CAM_CTL_EN_P2_DMA_IMG3O_EN){
                        //        ISP_QUE_INF(rt_dma,"MDP-IMGXO-IMG3O");
                        //        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                        //    }else{
                        //        isDequeMdp = MFALSE;
                        //    }
                        //    break;
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

                    if(isDequeMdp==MTRUE){
                        MDPMGR_RETURN_TYPE_FRMB mdpRet;
                        MBOOL isDump=MFALSE;
                        //
                        ISP_FUNC_DBG("dequeBurstQueIdx(%d),p2Cq(%d),this->dequeDupCqIdx(%d)",this->dequeBurstQueIdx,p2Cq,this->dequeDupCqIdx);

                        mdpRet = (MDPMGR_RETURN_TYPE_FRMB)g_pMdpMgr_FrmB->dequeueBuf(mdpQueueIdx, mdpMgrCfgStructData);
                        if(mdpRet == MDPMGR_NO_ERROR){
                            ret = eIspRetStatus_Success;
                        } else if(mdpRet == MDPMGR_VSS_NOT_READY){
                            ret = eIspRetStatus_VSS_NotReady;
                        } else {
                            ret = eIspRetStatus_Failed;
                            isDump = MTRUE;
                            ISP_FUNC_ERR("[Error]dequeueBuf fail mdpRet(%d)",mdpRet);
                        }
                        //
                        if(isDump==MTRUE || MTRUE==function_DbgLogEnable_VERBOSE){
                            MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
                            //char* pWorkingBuffer =  NULL;
                            //isp_reg_t *pIspReg;
                            IspDumpDbgLogP2Package p2DumpPackage;
                            ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
                            //
                            ISP_FUNC_INF("dequeBurstQueIdx(%d),p2Cq(%d),dequeDupCqIdx(%d),tpipeVa(0x%x),tpipePa(0x%x)",
                                this->dequeBurstQueIdx,p2Cq,this->dequeDupCqIdx,gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa,
                                gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa);
                            // dump CQ vir/des table
                            this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)0,0, this->dequeDupCqIdx);
                            this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((ISP_DRV_CQ_ENUM)this->dequeCq,this->burstQueIdx, this->dequeDupCqIdx);
                            // dump the difference between tpipe and hardware registers
                            p2DumpPackage.pIspReg = (isp_reg_t*)this->m_pIspDrvShell->m_pPhyIspDrv_bak->getCurHWRegValues();
                            p2DumpPackage.pIspTpipeCfgInfo = &gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo;
                            //pWorkingBuffer = new char[tpipeWorkingSize];
                            //
                            this->m_pIspDrvShell->gPass2Lock.lock();
                            #if 0
                            if((pWorkingBuffer!=NULL)&&(pIspReg!=NULL)) {
                                //mt6582 don't use pIspReg
                                //tpipe_main_platform(&gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspReg);
                                tpipe_main_platform(&gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize);
                                delete pWorkingBuffer;
                                pWorkingBuffer = NULL;
                            } else {
                                ISP_FUNC_ERR("[Error]pWorkingBuffer(0x%08x),pIspReg(0x%08x)",pWorkingBuffer,pIspReg);
                            }
                            #endif
                            // dump hardware debug information
                            p2DumpPackage.tpipeTableVa = gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa;
                            p2DumpPackage.tpipeTablePa = gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa;
                            this->m_pIspDrvShell->gPass2Lock.unlock();
                            //
                            this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(NULL,&p2DumpPackage);
                            //
                            if(mdpRet < 0)
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
                }
                else
                {    //UT, imgi in/img2o out, bypass MDP & Tpipe
                    //wait interrupt
                    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irqPass2Done;
            irqPass2Done.Clear= NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_NONE;
            irqPass2Done.UserInfo.Type=ISP_DRV_IRQ_TYPE_INT;
            irqPass2Done.UserInfo.Status=ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;
            irqPass2Done.UserInfo.UserKey=0;
            irqPass2Done.Timeout=5000;
            if( (_imgo_ == rt_dma) || (_img2o_ == rt_dma) )
            {    //preview or ip-raw
                        bool rett=this->m_pPhyIspDrv->waitIrq(&irqPass2Done);
                        if(!rett)
                        {
                            ISP_FUNC_ERR("wait P2 done fail");
                        }
                        else
                        {
                            ISP_FUNC_INF("get p2 done");
                        }
                    }
                }

                this->m_pIspDrvShell->gPass2Lock.lock();
                //change type
                m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front().status = ISP_BUF_FILLED;

                /*mt6582 don't have _jpeg_ case
                if (_jpeg_ == rt_dma)
                {
                    m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front().jpg_size = g_pMdpMgr_FrmB->getJPEGSize(gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx]);
                    ISP_QUE_INF(rt_dma,"jpeg real size(0x%x)", m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front().jpg_size);
                }
                */
                //
                //add to bufList
                //all element at the end
                bufList.pBufList->push_back(m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.front());
        if(isDequeMdp==true && ret == eIspRetStatus_VSS_NotReady)
        {}
        else
        {
            //delete from hw list
            m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.pop_front();
        }
        //
                ISP_FUNC_DBG("dma(%d),hwsize(%d),p2Cq(%d),dequeDupCqIdx(%d),imgxoEnP2(0x%x),memID(0x%x),size(%d),vaddr(0x%x),paddr(0x%x)",rt_dma,m_p2HwBufL[p2Cq][this->dequeDupCqIdx][rt_dma].bufInfoList.size(),p2Cq,this->dequeDupCqIdx, \
                        gpMdpMgrCfgData_FrmB[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].imgxoEnP2, \
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
/*
description: move current buffer to filled buffer list
*/
// TODO: check
#if 0//disable by FLORIA
MINT32 ISP_BUF_CTRL::dequeueSwBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufList )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 cnt = 0;
    struct _isp_buf_list_ *old_hw_head = NULL;
    //
    Mutex::Autolock lock(queSwLock);
    //
    if ( -1 == rt_dma )
    {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    //
    if ( 0 == m_swbufL[rt_dma].bufInfoList.size() )
    {
        //wait semephore till
        ISP_FUNC_ERR("empty SW buffer");
        return -1;
    }
    //
    while ( m_swbufL[rt_dma].bufInfoList.size() )
    {
        //all element at the end
        bufList.pBufList->push_back(m_swbufL[rt_dma].bufInfoList.front());
        //delete first element
        m_swbufL[rt_dma].bufInfoList.pop_front();
        //
        ISP_FUNC_DBG("05_dma(%d)/memID(0x%x)/size(%d)/vaddr(0x%x)/paddr(0x%x)",rt_dma,
                                    bufList.pBufList->back().memID, \
                                    bufList.pBufList->size(), \
                                    bufList.pBufList->back().base_vAddr,
                                    bufList.pBufList->back().base_pAddr);
    }
    //
    return 0;
}
#endif

MUINT32 ISP_BUF_CTRL::getCurrHwBuf( MUINT32 dmaChannel )
{
#if 1 //m_hwbufL unused
    return 0;
#else
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    //ISP_RT_BUF_STRUCT* pstRTBuf = (ISP_RT_BUF_STRUCT*)this->m_pIspDrvShell->m_pIspDrv->m_pRTBufTbl;

    if ( -1 == rt_dma )
    {
        ISP_FUNC_ERR("dma channel error ");
        return 0;
    }

    if (0 == m_hwbufL[rt_dma].bufInfoList.size())
    {
        ISP_FUNC_ERR("ERROR:No buffer in queue");
        return 0;
    }

    #if 0
    if ( ISP_PASS1 == this->path )
    {
        if ( _imgo_ == rt_dma || _img2o_ == rt_dma )
        {
            //at initialization
            //fetch pass1 ring buffer
            //start + 1
            if ( 0 < pstRTBuf->ring_buf[rt_dma].count )
            {
                ISP_FUNC_DBG("rt_dma(%d),start(%d),count(%d)",rt_dma,pstRTBuf->ring_buf[rt_dma].start,pstRTBuf->ring_buf[rt_dma].count);
            }
        }
    }
    #endif

    ISP_FUNC_DBG("dma:(%d)/memID(0x%x)/vAddr:(0x%x)/pAddr:(0x%x) ",rt_dma, \
                                                        m_hwbufL[rt_dma].bufInfoList.front().memID, \
                                                        m_hwbufL[rt_dma].bufInfoList.front().base_vAddr, \
                                                        m_hwbufL[rt_dma].bufInfoList.front().base_pAddr);

    return m_hwbufL[rt_dma].bufInfoList.front().base_pAddr;
#endif
}

MUINT32 ISP_BUF_CTRL::getNextHwBuf( MUINT32 dmaChannel )
{
#if 1 //m_hwbufL unused
    return 0;
#else
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    ISP_BUF_INFO_L::iterator it;
    MUINT32 base_pAddr = 0;

    if( -1 == rt_dma )
    {
        ISP_FUNC_ERR("ERROR:dma channel error ");
        return -1;
    }

    if(0 == m_hwbufL[rt_dma].bufInfoList.size())
    {
        ISP_FUNC_ERR("ERROR:No buffer in queue");
        return 0;
    }

    if( 1 < m_hwbufL[rt_dma].bufInfoList.size() )
    {
        it = m_hwbufL[rt_dma].bufInfoList.begin();
        it++;
        base_pAddr = it->base_pAddr;
    }

    ISP_FUNC_DBG("dma:[%d]/base_pAddr:[0x%x] ",rt_dma,base_pAddr);

    return base_pAddr;
#endif
}


MUINT32 ISP_BUF_CTRL::freeSinglePhyBuf(stISP_BUF_INFO bufInfo)
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


MUINT32 ISP_BUF_CTRL::freeAllPhyBuf( void )
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

int ISP_BUF_CTRL::getDmaBufIdx( MUINT32 dmaChannel )
{
    _isp_dma_enum_ dma;

    switch(dmaChannel)
    {
        case ISP_DMA_IMGI:
            dma = _imgi_;
            break;
        case ISP_DMA_IMGCI:
            dma = _imgci_;
            break;
        case ISP_DMA_IMGO:
            dma = _imgo_;
            break;
        case ISP_DMA_IMG2O:
            dma = _img2o_;
            break;
        case ISP_DMA_WDMAO:
            dma = _dispo_;                //CHRISTOPHER: need revised
            break;
        case ISP_DMA_WROTO:
            dma = _vido_;                //CHRISTOPHER: need revised
            break;
        case ISP_DMA_LSCI:
            dma = _lsci_;
            break;
        default:
            ISP_FUNC_ERR("Invalid dma channel(%d)",dmaChannel);
            return -1;
    }

    return (int)dma;
}
//
int
ISP_BUF_CTRL::
debugPrint( MUINT32 dmaChannel )
{
#if 0 //m_hwbufL unused
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    ISP_BUF_INFO_L::iterator it;

    ISP_FUNC_DBG("E");
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    //
    for ( it = m_hwbufL[rt_dma].bufInfoList.begin(); it != m_hwbufL[rt_dma].bufInfoList.end(); it++ ) {
        ISP_FUNC_DBG("m_hwbufL[%d].base_vAddr:[0x%x]/base_pAddr:[0x%x] ",rt_dma,it->base_vAddr,it->base_pAddr);
    }
    //
    for ( it = m_swbufL[rt_dma].bufInfoList.begin(); it != m_swbufL[rt_dma].bufInfoList.end(); it++ ) {
        ISP_FUNC_DBG("m_hwbufL[%d].base_vAddr:[0x%x]/base_pAddr:[0x%x] ",rt_dma,it->base_vAddr,it->base_pAddr);
    }
    //
    ISP_FUNC_DBG("X");
#endif

    return 0;
}
////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////
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

int ISP_PASS1_CTRL::setP1MagicNum( MUINT32 pass1_path, MUINT32 MagicNum, MUINT32 magicNumCnt)
{
    MUINT32 cq = ISP_DRV_CQ0;
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQIdx =  0;

    if ( ISP_PASS1 != pass1_path ) {
        ISP_FUNC_ERR("Invalid path:(%d)", pass1_path);
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
        ISP_FUNC_DBG("MagicNum [0x%x]", MagicNum);
    }

    return 0;
}

int ISP_PASS1_CTRL::keepP1HrzCfg( IspHrzCfg const hrz_cfg )
{
    if ( ISP_PASS1 == hrz_cfg.pass1_path ) {
        this->m_HrzCfg = hrz_cfg;
    }
    else {
        ISP_FUNC_ERR("hrz keeper path err 0x%x",hrz_cfg.pass1_path);
        return -1;
    }
    return 0;
}

int ISP_PASS1_CTRL::setP1HrzCfg( IspHrzCfg const hrz_cfg )
{
    IspDrv* _targetVirDrv = NULL;
    MUINT32 _targetDupQIdx = 0;

    ISP_FUNC_DBG("ISP_PASS1_CTRL::setP1HrzCfg E:path(%d),hrz_in_w/h(%d/%d),hrz_out_w/h(%d/%d),m_num(0x%x)", \
        hrz_cfg.pass1_path, \
        hrz_cfg.hrz_in_size.w, \
        hrz_cfg.hrz_in_size.h, \
        hrz_cfg.hrz_out_size.w, \
        hrz_cfg.hrz_out_size.h, \
        hrz_cfg.m_num_0);

    ISP_FUNC_DBG("twin(%d),path(%d)",hrz_cfg.isTwinMode,hrz_cfg.pass1_path);

    if ( ISP_PASS1 == hrz_cfg.pass1_path ) {
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
    else {
        ISP_FUNC_ERR("Not support pass1 path(%d)",hrz_cfg.pass1_path);
    }

    //
    if ( hrz_cfg.isTwinMode) {
        ISP_FUNC_ERR("support no twin mode\n");
        return -1;
    }
    else {
            MINT32 iHeight,resize, oSize;
            /*
            HRZ_INPUT_HEIGHT:
                "HRZ input size
                if (tg_sel = 0) height=image0
                else height = image1"

            HRZ_RESIZE:
                "HRZ resize value
                Get from 32768/decimation ratio
                decimation x1.0: hrz_resize should be 32768
                decimation x0.5: hrz_resize should be 65536
                decimation x0.25: hrz_resize should be 131072 "

            HRZ_OUTSIZE:
                "HRZ output size
                And this value get from
                X1 = input size * 32768 / hrz_resize
                hbn_outsize = 2* (X1>>1)"
            */
            //
            iHeight = hrz_cfg.hrz_in_size.h;
            //resize = 32768*10/10;
            resize = ( hrz_cfg.hrz_in_size.w << 15 )/hrz_cfg.hrz_out_size.w;
            //oSize = (this->src_img_w * 32768 / resize) & (~0x01);
            oSize = ( ( hrz_cfg.hrz_in_size.w << 15 ) / resize) & (~0x01);
            //
            //
            this->m_pIspDrvShell->cam_isp_hrz_cfg(_targetVirDrv,iHeight, resize, oSize);
        }

    ISP_FUNC_DBG("ISP_PASS1_CTRL::_config:End ");

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
        static IspEventThread*  getInstance(NSIspDrv_FrmB::IspDrv* pIspDrv);
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
    private:
        static MVOID* ispEventThread(void *arg);
    private:
        static NSIspDrv_FrmB::IspDrv  *m_pPhyIspDrv;
        static sem_t    m_semIspEventthread;
        static MBOOL    m_bPass1Start;
        //
        pthread_t       m_ispEventthread;
        mutable Mutex   mLock;
        volatile MINT32 mInitCount;
};
//
NSIspDrv_FrmB::IspDrv *IspEventThreadImp::m_pPhyIspDrv = NULL;
sem_t IspEventThreadImp::m_semIspEventthread;

//
IspEventThread*  IspEventThread::createInstance(NSIspDrv_FrmB::IspDrv* pIspDrv)
{
    return IspEventThreadImp::getInstance(pIspDrv);
}
//
IspEventThread*  IspEventThreadImp::getInstance(NSIspDrv_FrmB::IspDrv* pIspDrv)
{
    ISP_FUNC_DBG("E: pIspDrv(0x%08x)",pIspDrv);
    //
    //static IspEventThreadImp singleton;
    static IspEventThreadImp singleton;
    //
    m_pPhyIspDrv = pIspDrv;
    //
    return &singleton;
}
//
MBOOL IspEventThreadImp::init(void)
{
    ISP_FUNC_DBG("E");

#if 0
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
    pthread_create(&m_ispEventthread, &attr, ispEventThread, NULL);
    //
    android_atomic_inc(&mInitCount);
#endif

    ISP_FUNC_DBG("X");
    return MTRUE;
}
//
MBOOL IspEventThreadImp::uninit(void)
{
    ISP_FUNC_DBG("E");
#if 0
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

    ISP_FUNC_DBG("X");
#endif

    return MTRUE;
}
//
MVOID* IspEventThreadImp::ispEventThread(void *arg)
{
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irq_SOF;
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irq_VS;
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_DONE;


    ISP_FUNC_DBG("E: tid=%d m_pPhyIspDrv(0x%x)", gettid(),m_pPhyIspDrv);

    ::prctl(PR_SET_NAME,"ispEventthread",0,0,0);

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());
    //
    irq_SOF.Clear = NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_NONE;
    irq_SOF.UserInfo.Type = NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
    irq_SOF.UserInfo.Status = ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST;
    irq_SOF.Timeout = CAM_INT_WAIT_TIMEOUT_MS;
    //
    irq_VS.Clear = NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_NONE;
    irq_VS.UserInfo.Type = NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
    irq_VS.UserInfo.Status = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    irq_VS.Timeout = CAM_INT_WAIT_TIMEOUT_MS;
    //
    irq_TG1_DONE.Clear = NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_NONE;
    irq_TG1_DONE.UserInfo.Type = NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_INT;
    irq_TG1_DONE.UserInfo.Status = ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    irq_TG1_DONE.Timeout = CAM_INT_WAIT_TIMEOUT_MS;
    //
    while (1)
    {
        //if (pass1_start) {
            //ISP_FUNC_DBG("[ispEventthread] Wait m_semIspEventthread");
            ::sem_wait(&m_semIspEventthread); // wait here until someone use sem_post() to wake this semaphore up
            //ISP_FUNC_DBG("[ispEventthread] Got m_semIspEventthread");

            //0:VD
            m_pPhyIspDrv->waitIrq( &irq_VS );
            //call back function
            //set semephore
            ISP_FUNC_DBG("irq_VS ");
            ISP_FUNC_DBG("irq_VS ");

            //1:SOF
            m_pPhyIspDrv->waitIrq( &irq_SOF );
            //call back function
            //set semephore
            ISP_FUNC_DBG("irq_SOF ");
            ISP_FUNC_DBG("irq_SOF ");

            //2:EXP_DONE


            //3:PASS1_DONE
            //if (capture_enable) {
                m_pPhyIspDrv->waitIrq( &irq_TG1_DONE );
                //call back function
                //set semephore
                ISP_FUNC_DBG("irq_TG1_DONE ");
                ISP_FUNC_DBG("irq_TG1_DONE ");
            //}
        //}

    }

    //
    //::sem_post(&m_semIspEventthread);

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

