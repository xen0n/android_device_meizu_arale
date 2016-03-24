#ifndef __ISP_PATH_H__
#define __ISP_PATH_H__

#include "isp_function.h"
#include "mtkcam/imageio/ispio_stddef.h"
//

#define DEBUG_STR_BEGIN "EEEEEEEEEEEEEEEEEEEEE"
#define DEBUG_STR_END   "XXXXXXXXXXXXXXXXXXXXX"

using android::Mutex;


enum EPathFreeBufMode {
    ePathFreeBufMode_SINGLE         = 0x0000,   //
    ePathFreeBufMode_ALL            = 0x0001,   //
};


#define CONFIG_FOR_SYSTRACE     0   // default:0
#if CONFIG_FOR_SYSTRACE
#define ATRACE_TAG ATRACE_TAG_CAMERA
#define ISP_TRACE_CALL() ATRACE_CALL()
#else
#define ISP_TRACE_CALL()
#endif // CONFIG_FOR_SYSTRACE


/*/////////////////////////////////////////////////////////////////////////////
    IspPath_B
  /////////////////////////////////////////////////////////////////////////////*/
class IspFunction_B; //pre declaration

class CamPath_B
{
public:
    ISP_TURNING_CTRL    ispTuningCtrl;
    ISP_TOP_CTRL        ispTopCtrl;
    ISP_BUF_CTRL        ispBufCtrl;
    int CQ;
    int CQ_D;

    //common
    ISP_RAW_PIPE    ispRawPipe;
    DMA_CQ          DMACQ;
    DMA_CQ          DMACQ_D;

    //for pass1
    DMA_IMGO        DMAImgo;
    DMA_RRZO        DMARrzo;
    DMA_IMGO_D      DMAImgo_d;
    DMA_RRZO_D      DMARrzo_d;
    ISP_PASS1_CTRL  ispPass1Ctrl;
    //for camsv
    DMA_CAMSV_IMGO    DMACamsvImgo;
    DMA_CAMSV2_IMGO DMACamsv2Imgo;
    //for pass2
    ISP_RGB_PIPE    ispRgbPipe;
    ISP_YUV_PIPE    ispYuvPipe;
    ISP_MDP_PIPE    ispMdpPipe;
    CAM_CRZ_PIPE    crzPipe;
    CAM_TDRI_PIPE   tdriPipe;
    DMA_IMGI        DMAImgi;
    DMA_TDRI        DMATdri;
    DMA_VIPI        DMAVipi;
    DMA_VIP2I       DMAVip2i;
    DMA_VIP3I       DMAVip3i;
    DMA_IMG2O       DMAImg2o;
    DMA_IMG3O       DMAImg3o;
    DMA_IMG3BO      DMAImg3bo;
    DMA_IMG3CO      DMAImg3co;
    DMA_UFDI        DMAUfdi;
    DMA_LCEI        DMALcei;
    DMA_MFBO        DMAMfbo;
    DMA_FEO         DMAFEO;
private:
    //static Mutex   mCQBufferLock;  // for multi-thread
    //

public:
    CamPath_B();


    virtual ~CamPath_B(){};
public:
    int             start( void* pParam );
    int             stop( void* pParam );


    inline int              waitIrq( int type,unsigned int irq , int userNumber){return this->_waitIrq( type,irq,userNumber );}
    inline int              registerIrq( NSImageio::NSIspio::Irq_t irq ){return this->registerIrq( irq );}
    inline int              waitIrq( NSImageio::NSIspio::Irq_t irq ){return this->_waitIrq( irq );}
    inline int              writeReg( unsigned long offset, unsigned long value ){return ispTopCtrl.writeReg(offset,value);}
    inline unsigned long    readReg( unsigned long offset ){return ispTopCtrl.readReg(offset);}
    inline int              readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq){return ispTopCtrl.readIrq(pReadIrq);}
    inline int              checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){return ispTopCtrl.checkIrq(CheckIrq);}
    inline int              clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){return ispTopCtrl.clearIrq(ClearIrq);}

    int             dumpRegister( void* pRaram );
    int             end( void* pParam );
protected:
    virtual IspFunction_B**  isp_function_list() = 0;
    virtual int             isp_function_count() = 0;
public:
    virtual const char* name_Str(){     return  "IspPath";  }
protected:
    virtual int _config( void* pParam );
    virtual int _start( void* pParam );
    virtual int _stop( void* pParam );
    virtual int _waitIrq( int type, unsigned int irq , int userNumber) = 0;
    virtual int _registerIrq( NSImageio::NSIspio::Irq_t irq );
    virtual int _waitIrq( NSImageio::NSIspio::Irq_t irq );
    virtual int _end( void* pParam );
    virtual int _setZoom( void* pParam );
public:
    virtual int flushCqDescriptor( MUINT32 cq, MUINT32 burstQueIdx, MUINT32 dupCqIdx );
    virtual int getNr3dGain(ESoftwareScenario softScenario, MINT32 magicNum);
    //virtual int setDMACurrBuf( MUINT32 const dmaChannel );
    //virtual int setDMANextBuf( MUINT32 const dmaChannel );
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo,MINT32 enqueCq=0,MINT32 dupCqIdx=0);
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data,MBOOL bImdMode );
     /**
      *@brief Free physical buffer
      *@param[in] mode : free buffer mode
      *@param[in] bufInfo : buffer info
      */
    virtual int freePhyBuf( MUINT32 const mode, stISP_BUF_INFO bufInfo );
    /**
      *@brief  Get p2 tuning top tag
      */
    virtual MBOOL getEnTuningTag(MBOOL isTopEngine, ISP_DRV_CQ_ENUM cq, MUINT32 drvScenario, MUINT32 subMode, MUINT32 &tuningEn1Tag, MUINT32 &tuningEn2Tag, MUINT32 &tuningDmaTag);

};

/**
*@brief  ISP pass1 path  parameter
*/
struct CamPathPass1Parameter
{
public:     //// fields.
    //scenario/sub_mode
    int path;
    int pathD;
    int scenario;
    int sub_mode;
    int sub_mode_D;

    int CQ; //raw
    int CQ_D;
    int cqTrigSrc;
    int cqTrigSrc_D;
    int isTwinMode;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
//    int bypass_ispCdpPipe;
    int bypass_imgo;
    int bypass_rrzo;
    int bypass_imgo_d;
    int bypass_rrzo_d;
    int bypass_camsv_imgo;
    int bypass_camsv2_imgo;
    int b_continuous[ISP_MAX_CONCURRENT_TG_NUM];

    struct stIspTopCtl  isp_top_ctl;

    IspSize         tg_out_size;
    IspSize         rrz_in_size;
    IspSize         rrz_d_in_size;
    IspRect         rrz_in_roi;
    IspRect         rrz_d_in_roi;
    unsigned int    twin_lsc_crop_offx;
    unsigned int    twin_lsc_crop_offy;

    IspSize         src_img_size;
    //
    /*===DMA===*/
    IspDMACfg imgo;
    IspDMACfg rrzo;
    IspDMACfg imgo_d;
    IspDMACfg rrzo_d;
    IspDMACfg lcso;
    IspDMACfg aao;
    IspDMACfg nr3o;
    IspDMACfg esfko;
    IspDMACfg afo;
    IspDMACfg eiso;
    IspDMACfg imgci;
    IspDMACfg nr3i;
    IspDMACfg flki;
    IspDMACfg lsci;
    IspDMACfg lcei;
    IspDMACfg camsv_imgo;
    IspDMACfg camsv2_imgo;

    //
public:     //// constructors.
    CamPathPass1Parameter(
        int const _bypass_imgo     = 1,
        int const _bypass_rrzo     = 1,
        int const _bypass_imgo_d   = 1,
        int const _bypass_rrzo_d   = 1
    )
    {
        bypass_imgo = _bypass_imgo;
        bypass_rrzo = _bypass_rrzo;
        bypass_imgo_d   = _bypass_imgo_d;
        bypass_rrzo_d   = _bypass_rrzo_d;
    }
    //
public:     //// operations.
    MUINT32 operator()() const
    {
        return  *reinterpret_cast<MUINT32 const*>(this);
    }


};

/**
*@brief  ISP pass1 path class
*/
#define p1Notify_node   10

class CamPathPass1:public CamPath_B
{
private:
    int             m_isp_function_count;
    IspFunction_B*  m_isp_function_list[ISP_FUNCTION_MAX_NUM];
    P1_TUNING_NOTIFY*        m_p1NotifyObj[p1Notify_node];
    static MUINT32  m_p1NotifyTbl[p1Notify_node];
    //
public:

    /**
      *@brief  Constructor
      */
    CamPathPass1() :
        m_isp_function_count(0),
        BurstQNum(0),
        DupQIdx(0)
        {
            for(int i=0;i<p1Notify_node;i++)
                m_p1NotifyObj[i] = 0;
        };
    virtual ~CamPathPass1(){};
private:
    virtual int _waitIrq( int type,unsigned int irq , int userNumber);
    virtual int _registerIrq( NSImageio::NSIspio::Irq_t irq );
    virtual int _waitIrq( NSImageio::NSIspio::Irq_t irq );
protected:
    virtual IspFunction_B**  isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {   return m_isp_function_count; }
public:
    virtual const char* name_Str(){     return  "CamPathPass1";  }
public:
    int config( struct CamPathPass1Parameter* p_parameter );
    int setCdrz( IspSize out_size );
    int setDMAImgo( IspDMACfg const out_dma );
    int dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo );
    int setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);
    int setP1RrzCfg(IspRrzCfg const rrz_cfg);
    int setP1TuneCfg(MUINT32 magicNum, MINT32 senDev);
    MUINT32 setP1ImgoCfg(DMACfg const imgo_cfg);
    MUINT32 setP1MagicNum(MUINT32 const magicNum);
    MUINT32 setP1Notify(void);
    bool updateBurstQueueNum(int burstQNum);
    MINT32  updateCQI(void);
    MBOOL   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj);
public:
    MUINT32 BurstQNum;
    MUINT32 DupQIdx;
    MUINT32 pass1_path;
};


/*/////////////////////////////////////////////////////////////////////////////
    IspPathPass2
  /////////////////////////////////////////////////////////////////////////////*/
struct CamPathPass2Parameter
{
    //scenario/sub_mode
    //int scenario;
    //int subMode;
    unsigned int magicNum;
    unsigned int dupCqIdx;  // for hiding sw overhead
    unsigned int burstQueIdx; // for burst queue number
    MBOOL isWaitBuf;
    int mdp_imgxo_p2_en;
    MBOOL isV3;
    MBOOL isApplyTuning;
    int CQ;
    int tpipe;
    int tcm_en;
    int tdr_en;
    int isShareDmaCtlByTurn;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
//    int bypass_ispCdpPipe;
    int bypass_ispImg2o;
    float rrzRatio; // for sl2
    unsigned int rrzCropX; // for sl2
    unsigned int rrzCropY; // for sl2
    MBOOL isSl2cAheadCrz;
    MBOOL isLceAheadCrz;

    //enable table
    struct stIspTopCtl  isp_top_ctl;
    //update function mask
    //struct stIspTopP2FmtSel fixed_mask_cdp_fmt;
    //
    //int                     pix_id;
    //source ->  mem. in
    IspSize         src_img_size;
    IspRect         src_img_roi;
    //
    //IspSize         cdrz_out;
    //srz parameter
    SrzCfg srz1_cfg;
    SrzCfg srz2_cfg;
    //mfb parameter
    MfbCfg mfb_cfg;
    //nr3d
    Nr3DCfg nr3d_cfg;
    /*===DMA===*/
    IspDMACfg tdri;
    IspDMACfg imgi;
    IspDMACfg vipi;
    IspDMACfg vip2i;
    IspDMACfg vip3i;
    IspDMACfg imgci;
    IspDMACfg lcei;
    IspDMACfg ufdi;
    IspDMACfg lsci;
    //
    MdpRotDMACfg wroto;
    MdpRotDMACfg wdmao;
    MdpRotDMACfg jpego;
    MdpRotDMACfg imgxo;
    MdpRotDMACfg venco;
    //Jpeg Parameter
    JpgParaCfg jpgpara;
    //
    IspDMACfg imgo;
    IspDMACfg img2o;
    IspDMACfg img3o;
    IspDMACfg img3bo;
    IspDMACfg img3co;
    IspDMACfg mfbo;
    //
    IspCrzCfg crz;
    IspMdpCropCfg croppara;
    IspDMACfg feo;

    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspLceCfg lce;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspEsfkoCfg esfko;
    IspAaoCfg aao;
    IspLcsoCfg lcso;
    //IspCdrzCfg cdrz;
    IspCurzCfg curz;
    IspFeCfg fe;
    IspPrzCfg prz;
    IspMfbCfg mfb;
    IspFlkiCfg flki;
    IspCfaCfg cfa;
    IspSl2Cfg sl2;
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider

    NSImageio::NSIspio::EDrvScenario drvScenario;   //driver scenario
};


class CamPathPass2:public CamPath_B
{
private:
    int             m_isp_function_count;
    IspFunction_B*   m_isp_function_list[ISP_FUNCTION_MAX_NUM];

public:
    CamPathPass2() :
        m_isp_function_count(0)
        {};
    virtual ~CamPathPass2(){};
private:
    virtual int _waitIrq( int type,unsigned int irq, int userNumber );
protected:
    virtual IspFunction_B**  isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {   return m_isp_function_count; }
public:
    virtual const char* name_Str(){     return  "CamPathPass2";  }
public:
    int     config( struct CamPathPass2Parameter* p_parameter );
    int     setZoom( MUINT32 zoomRatio );
    EIspRetStatus  dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo, MINT32 const eDrvSce,MINT32 dequeCq, MINT32 dequeDupCqIdx, MUINT32 dequeBurstQueIdx );
    int     dequeueMdpFrameEnd( MINT32 const eDrvSce );
private:
    int     configTpipeData( struct CamPathPass2Parameter* p_parameter );
    int     getTpipePerform( struct CamPathPass2Parameter* p_parameter );
};

#endif







