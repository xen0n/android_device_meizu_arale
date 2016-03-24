
/**
* @file cam_path_FrmB.h
*
* cam_path Header File
*/

#ifndef __CAM_PATH_FRMB_H__
#define __CAM_PATH_FRMB_H__

#include "isp_function_FrmB.h"

#define CONFIG_FOR_SYSTRACE     0   // default:0
#if CONFIG_FOR_SYSTRACE
#define ATRACE_TAG ATRACE_TAG_CAMERA
#define ISP_TRACE_CALL() ATRACE_CALL()
#else
#define ISP_TRACE_CALL()
#endif // CONFIG_FOR_SYSTRACE

namespace NSDrvCam_FrmB {

#define ISP_MAX_TPIPE_SIMPLE_CONF_SIZE  (128*sizeof(int))

#define DEBUG_STR_BEGIN "EEEEEEEEEEEEEEEEEEEEE"
#define DEBUG_STR_END   "XXXXXXXXXXXXXXXXXXXXX"

/**
*@brief  Path free buffer mode
*/
enum EPathFreeBufMode
{
    ePathFreeBufMode_SINGLE = 0x0000,
    ePathFreeBufMode_ALL    = 0x0001
};


class IspFunction_B; //pre declaration

/**
*@brief  CamPath basic class
*/
class CamPath_B
{
public:
    ISP_TUNING_CTRL    ispTuningCtrl;
    ISP_TOP_CTRL        ispTopCtrl;
    ISP_BUF_CTRL        ispBufCtrl;
    int CQ;
    int CQ_D;

    //common
    ISP_RAW_PIPE    ispRawPipe;
    ISP_RGB_PIPE    ispRgbPipe;
    ISP_YUV_PIPE    ispYuvPipe;
    ISP_MDP_PIPE    ispMdpPipe;
    CAM_CDP_PIPE    cdpPipe;
    CAM_TDRI_PIPE   tdriPipe;
    DMA_CQ          DMACQ;
    //for pass1
    ISP_PASS1_CTRL  ispPass1Ctrl;
    DMA_IMGO        DMAImgo;
    DMA_IMG2O       DMAImg2o;
    //for pass2
    DMA_IMGI        DMAImgi;
    DMA_TDRI        DMATdri;
    DMA_VIPI        DMAVipi;
    DMA_VIP2I       DMAVip2i;
    DMA_LSCI        DMALsci;
    DMA_LCEI        DMALcei;
//SL TEST_MDP removed. Should no used    DMA_DISPO   DMADispo; //SL
//SL TEST_MDP removed. Should no used        DMA_VIDO    DMAVido; //SL

/*

    DMA_FLKI    if_DMAFlki;
    DMA_DISPO   if_DMADispo;
    DMA_VIDO    if_DMAVido;
*/

public:

    /**
      *@brief  Constructor
      */
    CamPath_B();

    /**
      *@brief  Destructor
      */
    virtual ~CamPath_B(){};

public:

    /**
      *@brief  Trigger cam path
      */
    int start( void* pParam );

    /**
      *@brief  Stop cam path
      */
    int stop( void* pParam );

    /**
      *@brief  Wait irq
      */
    inline int waitIrq( int type, unsigned int irq ){return this->_waitIrq( type,irq );} //mt6582
    inline int waitIrq( NSImageio_FrmB::NSIspio_FrmB::Irq_t irq ){return this->_waitIrq( irq );}
    inline int waitIrq( int type,unsigned int irq , int userNumber){return this->_waitIrq( type,irq,(int)(NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_WAIT),userNumber );}
    inline int waitIrq( int type,unsigned int irq , int irqclear,int userNumber){return this->_waitIrq( type,irq,irqclear,userNumber );}

    /**
      *@brief  Register irq
      */
    inline int registerIrq( NSImageio_FrmB::NSIspio_FrmB::Irq_t irq ){return this->registerIrq( irq );}

    /**
      *@brief  Write register
      */
    inline int writeReg( unsigned long offset, unsigned long value ){return ispTopCtrl.writeReg(offset,value);}

    /**
      *@brief  Read register
      */
    inline unsigned long readReg( unsigned long offset ){return ispTopCtrl.readReg(offset);}

    /**
      *@brief  Read irq
      */
    inline int readIrq(NSIspDrv_FrmB::ISP_DRV_READ_IRQ_STRUCT *pReadIrq){return ispTopCtrl.readIrq(pReadIrq);}

    /**
      *@brief  Check irq
      */
    inline int checkIrq(NSIspDrv_FrmB::ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){return ispTopCtrl.checkIrq(CheckIrq);}

    /**
      *@brief  Clear irq
      */
    inline int clearIrq(NSIspDrv_FrmB::ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){return ispTopCtrl.clearIrq(ClearIrq);}

    /**
      *@brief  Dump register
      */
    int dumpRegister( void* pRaram );

    /**
      *@brief
      */
    int end( void* pParam );

protected:

    /**
      *@brief  Get isp function list
      *@return
      *-Pointer to isp function
      */
    virtual IspFunction_B**  isp_function_list() = 0;

    /**
      *@brief  Count how many isp function
      *@return
      *-Number of isp function
      */
    virtual int isp_function_count() = 0;
public:

    /**
      *@brief Return name string
      */
    virtual const char* name_Str(){ return  "IspPath";  }
protected:

    /**
      *@brief Implementation of config
      *@param[in] pParam : configure data
      */
    virtual int _config( void* pParam );

    /**
      *@brief Implementation of start
      *@param[in] pParam : trigger data
      */
    virtual int _start( void* pParam );

    /**
      *@brief Implementation of stop
      *@param[in] pParam : trigger data
      */
    virtual int _stop( void* pParam );

    /**
      *@brief Implementation of waitIrq
      *@param[in] type : wait type
      *@param[in] irq : irq type
      */
    virtual int _waitIrq( int type,unsigned int irq ); //mt6582
    virtual int _waitIrq( int type, unsigned int irq ,int userEnum) = 0;
    virtual int _waitIrq( int type, unsigned int irq ,int irqclear,int userEnum) = 0;
    virtual int _waitIrq( NSImageio_FrmB::NSIspio_FrmB::Irq_t irq );
    virtual int _registerIrq( NSImageio_FrmB::NSIspio_FrmB::Irq_t irq );

    /**
      *@brief Implementation of end
      *@param[in] pParam : data
      */
    virtual int _end( void* pParam );

    /**
      *@brief Implementation of setZoom
      *@param[in] pParam : zoom data
      */
    virtual int _setZoom( void* pParam );
public:

    /**
      *@brief Flush command queue descriptor
      *@param[in] cq : specific command queue
      */
    virtual int flushCqDescriptor( MUINT32 cq );
    virtual int flushCqDescriptor( MUINT32 cq, MUINT32 burstQueIdx, MUINT32 dupCqIdx );

    /**
      *@brief Set current DMA buffer
       *@param[in] dmaChannel : dma channel
      */
    virtual int setDMACurrBuf( MUINT32 const dmaChannel );

    /**
      *@brief Set next DMA buffer
      *@param[in] dmaChannel : dma channel
      */
    virtual int setDMANextBuf( MUINT32 const dmaChannel );

    /**
      *@brief Enqueue buffer
      *@param[in] dmaChannel : dma channel
      *@param[in] bufInfo : buffer info
      */
    //virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo); //mt6582
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data,MBOOL bImdMode  );
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo, MINT32 enqueCq=0,MINT32 dupCqIdx=0);

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
    //scenario/sub_mode
    int path;
    int scenario;
    int sub_mode;

    int CQ; //raw
    int cqTrigSrc;
    int isTwinMode;
    int bypass_imgo;
    int bypass_img2o;

    /*mt6582*/
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;


    //enable table
    struct stIspTopEnTbl      en_Top;
    struct stIspTopINT        ctl_int;
    struct stIspTopFmtSel     fmt_sel;
    struct stIspTopSel        ctl_sel;
    struct stIspTopMuxSel     ctl_mux_sel;
    struct stIspTopMuxSel2    ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    struct stIspTopCtl        isp_top_ctl;
    //update function mask
    struct stIspTopFmtSel fixed_mask_cdp_fmt;
    //
    int                       pix_id;
    //source -> from TG
    IspSize         tg_out_size;
    IspSize         hrz_in_size;
    IspSize         hrz_out_size;

    IspSize         src_img_size;
    IspRect         src_img_roi;
    IspColorFormat  src_color_format;
    //
    /*===DMA===*/
    IspDMACfg imgo;     //dst00
    IspDMACfg img2o;    //dst01
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

    //
public:     //// constructors.
    CamPathPass1Parameter(
        int const _bypass_imgo     = 1,
        int const _bypass_img2o    = 1
    )
    {
        bypass_imgo = _bypass_imgo;
        bypass_img2o = _bypass_img2o;
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
    P1_TUNING_NOTIFY*         m_p1NotifyObj[p1Notify_node];
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

    /**
      *@brief Wait irq
      *@param[in] type : wait type
      *@param[in] irq : irq type
      */
    virtual int _waitIrq( int type,unsigned int irq , int userNumber);
    virtual int _waitIrq( int type,unsigned int irq , int irqclear, int userNumber);
    //virtual int _waitIrq( int type,unsigned int irq , int userNumber);
    //virtual int _registerIrq( NSImageio_FrmB::NSIspio_FrmB::Irq_t irq );
    //virtual int _waitIrq( NSImageio_FrmB::NSIspio_FrmB::Irq_t irq );
protected:

    /**
      *@brief  Get isp function list
      *@return
      *-Pointer to isp function
      */
    virtual IspFunction_B**  isp_function_list()  { return m_isp_function_list; }

    /**
      *@brief  Count how many isp function
      *@return
      *-Number of isp function
      */
    virtual int isp_function_count() { return m_isp_function_count; }
public:

    /**
      *@brief Return name string
      */
    virtual const char* name_Str(){ return  "CamPathPass1";  }
    int setP1HrzCfg(IspHrzCfg const hrz_cfg);
    int setP1TuneCfg(MUINT32 magicNum, MINT32 senDev);
    MUINT32 setP1ImgoCfg(DMACfg const imgo_cfg);
    MUINT32 setP1MagicNum(UINT32 const magicNum);
    MUINT32 setP1Notify(void);
    bool updateBurstQueueNum(int burstQNum);
    MINT32  updateCQI(void);
    MBOOL   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* obj);
public:

    /**
      *@brief Configure pass1 path
      *@param[in] p_parameter : configure data
      */
    int  config( struct CamPathPass1Parameter* p_parameter );

    /**
      *@brief Configure CDRZ
      *@param[in] out_size : IspSize
      */
    int     setCdrz( IspSize out_size );

    /**
      *@brief Configure IMGO
      *@param[in] out_dma : IspDMACfg
      */
    int     setDMAImgo( IspDMACfg const out_dma );

    /**
      *@brief Dequeue buffer
      *@param[in] dmaChannel : dma channel
      *@param[in] bufInfo : buffer info
      */
    int     dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo );

    /**
      *@brief Set command queue trigger mode
      *@param[in] cq : specific command queue
      *@param[in] mode : trigger mode
      *@param[in] trig_src : trigger source
      */
    int     setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);

    MUINT32 BurstQNum;
    MUINT32 DupQIdx;
private:
};

/**
*@brief  ISP pass2 path  parameter
*/
struct CamPathPass2Parameter
{
    //scenario/sub_mode
    //int subMode;
    unsigned int magicNum;
    unsigned int dupCqIdx;  // for hiding sw overhead
    unsigned int burstQueIdx; // for burst queue number
    MBOOL isWaitBuf;
    int mdp_imgxo_p2_en;
    unsigned int scenario;
    MBOOL isV3;
    MBOOL isApplyTuning;
    MBOOL isMdpCropEn;
    int CQ;
    int tpipe;
    int p2SubMode;
    int tdr_en;
    int tcm_en;
    int isIspOn;
    float hrzRatio; // for sl2
    unsigned int hrzCropX; // for sl2
    unsigned int hrzCropY; // for sl2
    //
    int isConcurrency;
    int isEn1C24StatusFixed;
    int isEn1CfaStatusFixed;
    int isEn1HrzStatusFixed;
    int isEn2CdrzStatusFixed;
    int isEn2G2cStatusFixed;
    int isEn2C42StatusFixed;
    int isImg2oStatusFixed;
    int isImgoStatusFixed;
    int isAaoStatusFixed;
    int isEsfkoStatusFixed;
    int isFlkiStatusFixed;
    int isLcsoStatusFixed;
    int isEn1AaaGropStatusFixed;
    int isShareDmaCtlByTurn;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
    int bypass_ispCdpPipe;

    //enable table
    struct stIspTopEnTbl    en_Top;
    struct stIspTopINT      ctl_int;
    struct stIspTopFmtSel   fmt_sel;
    struct stIspTopSel      ctl_sel;
    struct stIspTopMuxSel     ctl_mux_sel;
    struct stIspTopMuxSel2    ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    struct stIspRegPixId ctl_pix_id;

    //update function mask
    struct stIspTopFmtSel fixed_mask_cdp_fmt;
    //
    int                     pix_id;
    //source ->  mem. in
    IspSize         src_img_size;
    IspRect         src_img_roi;
    IspColorFormat  src_color_format;
    //
    /*===DMA===*/
    IspDMACfg tdri;
    IspDMACfg cqi;
    IspDMACfg imgi;
    IspDMACfg imgci;
    IspDMACfg lsci;
    //
    MdpRotDMACfg wroto;
    MdpRotDMACfg wdmao;
    MdpRotDMACfg imgxo;
    //
    IspCdrzCfg cdrz;
    IspMdpCropCfg croppara;

    IspRingTdriCfg ringTdriCfg;
    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspDMACfg imgo_dma;
    IspEsfkoCfg esfko;
    IspAaoCfg aao;
    IspLcsoCfg lcso;
    IspPrzCfg prz;
    IspFlkiCfg flki;
    IspCfaCfg cfa;
    IspDMACfg imgo;
    IspDMACfg img2o;
    //
    //
    unsigned int imgo_mux_en;
    unsigned int imgo_mux;
    unsigned int img2o_mux_en;
    unsigned int img2o_mux;
    //
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider


    NSImageio_FrmB::NSIspio_FrmB::EDrvScenario drvScenario;   //driver scenario
    //
public:     //// constructors.
    CamPathPass2Parameter(
        int const _imgo_mux_en     = 0,
        int const _imgo_mux    = 0,
        int const _img2o_mux_en    = 0,
        int const _img2o_mux    = 0
    )
    {
        imgo_mux_en = _imgo_mux_en;
        imgo_mux = _imgo_mux;
        img2o_mux_en=_img2o_mux;
        img2o_mux=_img2o_mux;
    }
};

/**
*@brief  ISP pass1 path class
*/
class CamPathPass2:public CamPath_B
{
private:
    int             m_isp_function_count;
    IspFunction_B*   m_isp_function_list[ISP_FUNCTION_MAX_NUM];

public:

    /**
      *@brief  Constructor
      */
    CamPathPass2() :
        m_isp_function_count(0)
        {};

    /**
      *@brief  Destructor
      */
    virtual ~CamPathPass2(){};
private:

    /**
      *@brief Wait irq
      *@param[in] type : wait type
      *@param[in] irq : irq type
      */
    virtual int _waitIrq( int type,unsigned int irq );
    virtual int _waitIrq( int type,unsigned int irq , int userNumber);
    virtual int _waitIrq( int type,unsigned int irq , int irqclear, int userNumber);
protected:

    /**
      *@brief  Get isp function list
      *@return
      *-Pointer to isp function
      */
    virtual IspFunction_B**  isp_function_list()  {   return m_isp_function_list; }

    /**
      *@brief  Count how many isp function
      *@return
      *-Number of isp function
      */
    virtual int isp_function_count() {   return m_isp_function_count; }
public:

    /**
      *@brief Return name string
      */
    virtual const char* name_Str(){     return  "CamPathPass2";  }
public:

    /**
      *@brief Configure pass1 path
      *@param[in] p_parameter : configure data
      */
    int config( struct CamPathPass2Parameter* p_parameter );

    /**
      *@brief Set zoom
      *@param[in] zoomRatio : zoom ratio
      */
    int setZoom( MUINT32 zoomRatio );

    EIspRetStatus  dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo, MINT32 const eDrvSce,MINT32 dequeCq, MINT32 dequeDupCqIdx, MUINT32 dequeBurstQueIdx );

    int dequeueMdpFrameEnd( MINT32 const eDrvSce );
private:

    /**
      *@brief Configure tpipe
      *@param[in] p_parameter : configure data
      */
    int configTpipeData( struct CamPathPass2Parameter* p_parameter );

    /**
      *@brief Get tpipe perform
      *@param[in] p_parameter : configure data
      */
    int getTpipePerform( struct CamPathPass2Parameter* p_parameter );

    /**
      *@brief Get CDP mux setting
      *@param[in] pass2Parameter : configure data
      *@param[in] pDispVidSel : mus setting
      */
    int getCdpMuxSetting(struct CamPathPass2Parameter pass2Parameter, MINT32 *pDispVidSel);
};

};   //NSDrvCam_FrmB

#endif   //__CAM_PATH_FRMB_H__







