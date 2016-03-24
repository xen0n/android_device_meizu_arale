#define LOG_TAG "iio/pathp1"
//
//
#include "cam_path.h"
#include <mtkcam/imageio/ispio_utility.h>
#include "mtkcam/imageio/p1HwCfg.h" //inclucde this is for EIS moulde

//



#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(path);
EXTERN_DBG_LOG_VARIABLE(path);
using namespace NSImageio;
using namespace NSIspio;


/*/////////////////////////////////////////////////////////////////////////////
  CamPathPass1
  /////////////////////////////////////////////////////////////////////////////*/
int CamPathPass1::config( struct CamPathPass1Parameter* p_parameter )
{
    ISP_PATH_DBG("CamPathPass1::config E");
    MUINT32 cnt=0;
    Mutex   *_localVar;
    //Notice :
    //Config TG1 IRZ & RG2 IRZ shall share the same lock to vaoid config error
    if(p_parameter->path == ISP_PASS1)
        _localVar = &this->ispTopCtrl.m_pIspDrvShell->gPass1TopCtrlLock;
    else if( (p_parameter->path == ISP_PASS1_CAMSV) || (p_parameter->pathD == ISP_PASS1_D) || (p_parameter->pathD == ISP_PASS1_CAMSV_D) )
        _localVar = &this->ispTopCtrl.m_pIspDrvShell->gPass1TopCtrlLock;//&this->ispTopCtrl.m_pIspDrvShell->gPass1_dLock; // avoid top_config,whcih in config2, have racing issue under multi-sensor(pass1 && pass1d config at the same time)
    else
        _localVar = &this->ispTopCtrl.m_pIspDrvShell->gPass1TopCtrlLock;
    Mutex::Autolock lock(_localVar);

    /*function List*/
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMACQ;
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMACQ_D;
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&ispTopCtrl;
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&ispRawPipe;
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMAImgo;
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMARrzo;
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMAImgo_d;
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMARrzo_d;
    if(MFALSE == p_parameter->bypass_camsv_imgo) {
        m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMACamsvImgo;
    }
    if(MFALSE == p_parameter->bypass_camsv2_imgo) {
    m_isp_function_list[cnt++ ] = (IspFunction_B*)&DMACamsv2Imgo;
    }
    //m_isp_function_list[9 ] = (IspFunction_B*)&ispPass1Ctrl;
    m_isp_function_count = cnt;

    /***Setup isp function parameter***/
    //top
    ispTopCtrl.path         = p_parameter->path;
    ispTopCtrl.pathD        = p_parameter->pathD;
    ispTopCtrl.sub_mode      = p_parameter->sub_mode;
    ispTopCtrl.sub_mode_D     = p_parameter->sub_mode_D;
    ispTopCtrl.CQ    = p_parameter->CQ;
    ispTopCtrl.CQ_D  = p_parameter->CQ_D;
    ispTopCtrl.isTwinMode   = p_parameter->isTwinMode;
    
    // C++11 do not allow this kind of operation, use memcpy for workaround
    //ispTopCtrl.isp_top_ctl  = p_parameter->isp_top_ctl;
    memcpy((void *) &ispTopCtrl.isp_top_ctl, (void *) &p_parameter->isp_top_ctl, sizeof(struct stIspTopCtl));
    //
    //ispTopCtrl.pix_id       = p_parameter->isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.PIX_ID;
    ispTopCtrl.b_continuous[0]  = p_parameter->b_continuous[0];
    ispTopCtrl.b_continuous[1]  = p_parameter->b_continuous[1];

    //
    //this infor. have including p1 & p1_d ??
    ispPass1Ctrl.path = ispTopCtrl.path;
    ispPass1Ctrl.path_D = p_parameter->pathD;
    ispPass1Ctrl.CQ = ispTopCtrl.CQ;
    ispPass1Ctrl.CQ_D = p_parameter->CQ_D;
    //isp pipe
    ispRawPipe.CQ               = p_parameter->CQ;
    ispRawPipe.CQ_D             = p_parameter->CQ_D;
    ispRawPipe.bypass           = p_parameter->bypass_ispRawPipe;
    ispRawPipe.path             = p_parameter->path;
    ispRawPipe.pathD            = p_parameter->pathD;
    ispRawPipe.sub_mode         = p_parameter->sub_mode;
    ispRawPipe.sub_mode_D        = p_parameter->sub_mode_D;
    ispRawPipe.isTwinMode       = p_parameter->isTwinMode;
    ispRawPipe.en_p1            = p_parameter->isp_top_ctl.CAM_CTL_EN_P1.Raw;
    ispRawPipe.en_p1_d          = p_parameter->isp_top_ctl.CAM_CTL_EN_P1_D.Raw;
    ispRawPipe.rrz_in_size      = p_parameter->rrz_in_size;
    ispRawPipe.rrz_in_roi       = p_parameter->rrz_in_roi;
    ispRawPipe.rrz_out_size     = p_parameter->rrzo.size;
    ispRawPipe.rrz_d_in_size    = p_parameter->rrz_d_in_size; //raw-d
    ispRawPipe.rrz_d_in_roi     = p_parameter->rrz_d_in_roi;  //raw-d
    ispRawPipe.rrz_d_out_size   = p_parameter->rrzo_d.size;   //raw-d
    ispRawPipe.twin_lsc_crop_offx = p_parameter->twin_lsc_crop_offx;
    ispRawPipe.twin_lsc_crop_offy = p_parameter->twin_lsc_crop_offy;
    ispRawPipe.src_img_w        = p_parameter->tg_out_size.w; //not used for Pass1
    ispRawPipe.src_img_h        = p_parameter->tg_out_size.h; //not used for Pass1
    if( p_parameter->path == ISP_PASS1 ) {
        ispRawPipe.img_sel      = p_parameter->isp_top_ctl.CAM_CTL_SEL_P1.Bits.IMG_SEL;
    }
    else if( p_parameter->pathD == ISP_PASS1_D ) {
        ispRawPipe.img_sel      = p_parameter->isp_top_ctl.CAM_CTL_SEL_P1_D.Bits.IMG_SEL_D;
    }
    //
    ispRawPipe.pPass1Ctrl       =  &ispPass1Ctrl;


    ISP_PATH_INF("ispTopCtrl path/pathD/Twin = (0x%x/0x%x/%d/)", ispTopCtrl.path, ispTopCtrl.pathD, ispTopCtrl.isTwinMode);
    /*ISP_PATH_INF("ispTopCtrl EN_P1[0x%x], EN_P1_DMA[0x%x], EN_P1_D[0x%x], EN_P1_DMA_D[0x%x], CQ_EN[0x%x], SCENARIO[0x%x]",\
                  ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P1, ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P1_DMA, \
                  ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P1_D, ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P1_DMA_D,\
                  ispTopCtrl.isp_top_ctl.CAM_CTL_CQ_EN, ispTopCtrl.isp_top_ctl.CAM_CTL_SCENARIO  );*/


    ISP_PATH_INF("ispRawPipe bypass/path/pathD/Twin = (%d/0x%x/0x%x/%d)", \
                 ispRawPipe.bypass, ispRawPipe.path, ispRawPipe.pathD, ispRawPipe.isTwinMode);


    ISP_PATH_INF("p_parameter rrz_in_roi_size(%d/%d/%d/%d/%d/%d)",\
                                        p_parameter->rrz_in_roi.x, \
                                        p_parameter->rrz_in_roi.y, \
                                        p_parameter->rrz_in_roi.floatX, \
                                        p_parameter->rrz_in_roi.floatY, \
                                        p_parameter->rrz_in_roi.w, \
                                        p_parameter->rrz_in_roi.h);

    ISP_PATH_INF("p_parameter rrzD_in_roi_size(%d/%d/%d/%d/%d/%d)",\
                                        p_parameter->rrz_in_roi.x, \
                                        p_parameter->rrz_d_in_roi.y, \
                                        p_parameter->rrz_d_in_roi.floatX, \
                                        p_parameter->rrz_d_in_roi.floatY, \
                                        p_parameter->rrz_d_in_roi.w, \
                                        p_parameter->rrz_d_in_roi.h);

    /*ISP_PATH_INF("ispRawPipe rrz_in_size(%d/%d/%d/%d/%d/%d)",ispRawPipe.rrz_in_roi.x, \
                                        ispRawPipe.rrz_in_roi.y, \
                                        ispRawPipe.rrz_in_roi.floatX, \
                                        ispRawPipe.rrz_in_roi.floatY, \
                                        ispRawPipe.rrz_in_roi.w, \
                                        ispRawPipe.rrz_in_roi.h);*/


    DMAImgo.isTwinMode = p_parameter->isTwinMode;
    DMAImgo.dma_cfg      = p_parameter->imgo;
    DMAImgo.dupCqIdx     = this->DupQIdx;
    DMAImgo.burstQueIdx  = this->BurstQNum;
    DMARrzo.isTwinMode = p_parameter->isTwinMode;
    DMARrzo.dma_cfg      = p_parameter->rrzo;
    DMARrzo.dupCqIdx     = this->DupQIdx;
    DMARrzo.burstQueIdx  = this->BurstQNum;
    DMAImgo_d.dma_cfg    = p_parameter->imgo_d;
    DMAImgo_d.dupCqIdx   = this->DupQIdx;
    DMAImgo_d.burstQueIdx    = this->BurstQNum;
    DMARrzo_d.dma_cfg    = p_parameter->rrzo_d;
    DMARrzo_d.dupCqIdx   = this->DupQIdx;
    DMARrzo_d.burstQueIdx    = this->BurstQNum;
    DMACamsvImgo.isTwinMode = p_parameter->isTwinMode;
    DMACamsvImgo.dma_cfg = p_parameter->camsv_imgo;
    DMACamsv2Imgo.isTwinMode = p_parameter->isTwinMode;
    DMACamsv2Imgo.dma_cfg = p_parameter->camsv2_imgo;

    //buffer control path
    ispBufCtrl.path   = p_parameter->path;
    ispBufCtrl.path_D = p_parameter->pathD;
    ispBufCtrl.CQ   = p_parameter->CQ;
    ispBufCtrl.CQ_D = p_parameter->CQ_D;
    ispBufCtrl.pPass1Ctrl = &ispPass1Ctrl;
    ispBufCtrl.init(0);

    //
    DMACQ.bypass = 0;
    DMACQ.CQ = CAM_ISP_CQ_NONE;   //CQIdx=2
    DMACQ.CQ_D = ISP_DRV_CQ0C_D;  //CQIdx=5
    DMACQ.cqTrigSrc_D = p_parameter->cqTrigSrc_D;
    DMACQ.config(); /////will switch to virtual-reg after config

    DMACQ.bypass = 0;
    DMACQ.CQ = ISP_DRV_CQ0C;   //CQIdx=2
    DMACQ.CQ_D = CAM_ISP_CQ_NONE;
    DMACQ.cqTrigSrc = p_parameter->cqTrigSrc;
    DMACQ.config();

    //pass1 commandQ
    ISP_PATH_INF("ispRawPipe CQ/CQ0D=[0x%x,0x%x]",ispRawPipe.CQ, ispRawPipe.CQ_D);




    //raw
    this->CQ         = p_parameter->CQ;
    DMACQ.CQ         = p_parameter->CQ;
    DMACQ.CQ_D         = p_parameter->CQ_D;
    DMAImgo.CQ       = p_parameter->CQ;
    DMARrzo.CQ       = p_parameter->CQ; //sophie add
    DMAImgo.CQ_D     = CAM_ISP_CQ_NONE;
    DMARrzo.CQ_D     = CAM_ISP_CQ_NONE;
    DMACQ.bypass     = 1;
    DMAImgo.bypass   = p_parameter->bypass_imgo;
    DMARrzo.bypass   = p_parameter->bypass_rrzo;

    //raw-d
    this->CQ_D       = p_parameter->CQ_D;
    DMACQ_D.CQ       = p_parameter->CQ;
    DMACQ_D.CQ_D     = p_parameter->CQ_D; //used to config CQ0-D
    DMACQ_D.cqTrigSrc = p_parameter->cqTrigSrc;
    DMACQ_D.cqTrigSrc_D = p_parameter->cqTrigSrc_D;
    DMAImgo_d.CQ     = CAM_ISP_CQ_NONE;
    DMARrzo_d.CQ     = CAM_ISP_CQ_NONE; //sophie add
    DMAImgo_d.CQ_D     = p_parameter->CQ_D;
    DMARrzo_d.CQ_D     = p_parameter->CQ_D; //sophie add
    DMACQ_D.bypass   = 1;
    DMAImgo_d.bypass = p_parameter->bypass_imgo_d;
    DMARrzo_d.bypass = p_parameter->bypass_rrzo_d;

    //camsv
    DMACamsvImgo.CQ = p_parameter->CQ;
    //camsv2
    DMACamsv2Imgo.CQ = p_parameter->CQ;

    if ( CAM_ISP_CQ_NONE != p_parameter->CQ ) {
        DMACQ.bypass   = 0; //

        if (p_parameter->isTwinMode) {
            //Twin Mode:  use CQ0 to config raw and raw-d
            //Twin Mode will not use imgo-d and rrzo-d
        }
    }

    if ( CAM_ISP_CQ_NONE != p_parameter->CQ_D ) {
        //PIP(Main+Sub) Mode: raw/raw-d has their relative CQ
        DMACQ_D.bypass   = 0;
    }


#if 0
    //real time buffer control
    if ( 0 < p_parameter->imgo.memBuf.cnt) {
        ispBufCtrl.init(ISP_DMA_IMGO);
    }
    //
    if ( 0 < p_parameter->img2o.memBuf.cnt) {
        ispBufCtrl.init(ISP_DMA_IMGO_D);
    }
#endif
    //update static scenario register(for p2 reference)
    if(ispTopCtrl.m_pIspDrvShell->getPhyIspDrv())
    {
        MUINT32 regScen=0xa5a5a5a5;
        regScen=p_parameter->isp_top_ctl.CAM_CTL_SCENARIO.Raw;
        regScen &= 0x07; //fixed to raw
        bool ret=false;
        ret=ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->updateScenarioValue(regScen);
        if(!ret)
        {
            ISP_PATH_ERR("updateSce ERr(0x%x)",regScen);
        }
    }
    //
    this->_config(NULL);
    //add for pass1 dup cmdQ.
    //ring here is inorder to maintain dma api consistance( other dma channel no init issue)
    for(int i=1;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
        if(DMAImgo.bypass != 1){
            DMAImgo.dupCqIdx = i;
            DMAImgo.config();
            DMAImgo.write2CQ();
        }
        if(DMARrzo.bypass != 1){
            DMARrzo.dupCqIdx = i;
            DMARrzo.config();
            DMARrzo.write2CQ();
        }
        if(DMAImgo_d.bypass != 1){
            DMAImgo_d.dupCqIdx = i;
            DMAImgo_d.config();
            DMAImgo_d.write2CQ();
        }
        if(DMARrzo_d.bypass != 1){
            DMARrzo_d.dupCqIdx = i;
            DMARrzo_d.config();
            DMARrzo_d.write2CQ();
        }
    }
    ISP_PATH_DBG("CamPathPass1::config X");

    return 0;
}

int CamPathPass1::_waitIrq( int type, unsigned int irq , int userNumber)
{
    int ret = 0;
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)type;
    WaitIrq.Status = irq;
    WaitIrq.UserNumber = userNumber;
    WaitIrq.Timeout = 500;
    if(userNumber == ISP_DRV_IRQ_USER_ISPDRV)
        WaitIrq.UserName = "pass1";
    else
        WaitIrq.UserName = "unknown";
    WaitIrq.bDumpReg = 0xFE;

    ISP_PATH_DBG("CamPathPass1::_waitIrq+, type=%x, irq=%x username:%s", type, irq,WaitIrq.UserName);

    if ( MFALSE == (MBOOL)ispTopCtrl.waitIrq(&WaitIrq) ) {
        ret = -1;
    }

    ISP_PATH_DBG("ret(%d)",ret);

    return ret;
}

int CamPathPass1::_registerIrq( NSImageio::NSIspio::Irq_t irq )
{
int ret = 0;
#if 0
ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass1Lock);

    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)type;
    WaitIrq.Status = irq;
    WaitIrq.Timeout = CAM_INT_WAIT_TIMEOUT_MS;//ms ,0 means pass through.

    if ( MFALSE == (MBOOL)ispTopCtrl.waitIrq(&WaitIrq) ) {
        ret = -1;
    }

    ISP_PATH_DBG("ret(%d)",ret);
#endif
    return ret;
}

int CamPathPass1::_waitIrq( NSImageio::NSIspio::Irq_t irq )
{
int ret = 0;
#if 0
ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass1Lock);

    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)type;
    WaitIrq.Status = irq;
    WaitIrq.Timeout = CAM_INT_WAIT_TIMEOUT_MS;//ms ,0 means pass through.

    if ( MFALSE == (MBOOL)ispTopCtrl.waitIrq(&WaitIrq) ) {
        ret = -1;
    }

    ISP_PATH_DBG("ret(%d)",ret);
#endif
    return ret;
}

//
int CamPathPass1::setCdrz( IspSize out_size )
{
    int ret = 0;
#if 0
    ISP_PATH_DBG("E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass1Lock);

    this->cdpPipe.cdrz_out = out_size;
    this->cdpPipe.conf_cdrz = 1;
    this->cdpPipe.conf_rotDMA = 0;

    ret = this->cdpPipe.config();
    if ( 0 != ret ) {
        ISP_PATH_ERR("ERROR config cdrz ");
    }
    ISP_PATH_DBG("X");
#endif
    return ret;
}
//
int CamPathPass1::setDMAImgo( IspDMACfg const out_dma )
{
int ret = 0;

    ISP_PATH_DBG("E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass1Lock);

    if (CAM_ISP_CQ_NONE == this->DMAImgo.CQ) {
        ISP_PATH_ERR("setDMAImgo:Imgo ERROR: CAM_ISP_CQ_NONE ");
        return -1;
    }

    this->DMAImgo.dma_cfg = out_dma;
    //
    ret = this->DMAImgo.config();
    //
    if ( 0 != ret ) {
        ISP_PATH_ERR("ERROR config imgo ");
    }
    ISP_PATH_DBG("X");
    return ret;
}

//
int CamPathPass1::dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo )
{
    int ret = 0;
    Mutex   *_localVar;
    //ISP_PATH_DBG("E");

    // TODO:
    //Seprate the gPass1Lock to be Tg1 & TG2 lock.
    //Wait TG1 IRZ & RG2 IRZ shall not share the same lock...

    if( (dmaChannel == ISP_DMA_RRZO) || (dmaChannel == ISP_DMA_IMGO) )
        _localVar = &this->ispTopCtrl.m_pIspDrvShell->gPass1Lock;
    else
        _localVar = &this->ispTopCtrl.m_pIspDrvShell->gPass1_dLock;
    Mutex::Autolock lock(_localVar);


    //check if there is already filled buffer
    if ( MFALSE == this->ispBufCtrl.waitBufReady(dmaChannel) ) {
        ISP_PATH_ERR("waitBufReady fail");
        return -1;
    }
    //move FILLED buffer from hw to sw list
    if ( eIspRetStatus_Success != this->ispBufCtrl.dequeueHwBuf( dmaChannel, bufInfo ) ) {
        ISP_PATH_ERR("ERROR:dequeueHwBuf");
        return -1;
    }
    //delete all after move sw list to bufInfo.
    //if ( 0 != this->ispBufCtrl.dequeueSwBuf( dmaChannel, bufInfo ) ) {
    //    ISP_PATH_ERR("ERROR:dequeueSwBuf");
    //    return -1;
    //}
    //
    //ISP_PATH_DBG("[0x%x] ",bufInfo.pBufList->front().base_vAddr);
    //
    //ISP_PATH_DBG("X");
    return ret;
}
//
int CamPathPass1::setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src)
{
    int ret = 0;
    Mutex   *_localVar;
    ISP_PATH_DBG("E:%d/%d/%d ",cq,mode,trig_src);
    if( (cq == ISP_DRV_CQ0) || (cq == ISP_DRV_CQ0B) || (cq == ISP_DRV_CQ0C))
        _localVar = &this->ispTopCtrl.m_pIspDrvShell->gPass1Lock;
    else
        _localVar = &this->ispTopCtrl.m_pIspDrvShell->gPass1_dLock;
    Mutex::Autolock lock(_localVar);

    this->ispTopCtrl.setCQTriggerMode(cq,mode,trig_src);
    ISP_PATH_DBG("X");
    return ret;
}

MUINT32 CamPathPass1::setP1ImgoCfg(DMACfg const imgo_cfg){
    MUINT32 ret = 0;
    ISP_QUERY_RST queryRst;
    E_ISP_PIXMODE e_PixMode;
    if(this->CQ == CAM_ISP_CQ0){
        ISP_PATH_DBG("update imgo cropping\n");
        e_PixMode = (DMAImgo.isTwinMode == MTRUE)? (ISP_QUERY_2_PIX_MODE):(ISP_QUERY_1_PIX_MODE);
        //xsize/ysize r in byte domain, need to be transfered
        if(ISP_QuerySize(EPortIndex_IMGO,ISP_QUERY_CROP_X_BYTE,(EImageFormat)DMAImgo.dma_cfg.lIspColorfmt,imgo_cfg.crop.w,queryRst,e_PixMode) == 0){
            ISP_PATH_ERR("cropping size err(0x%x)\n",imgo_cfg.crop.w);
            return 1;
        }
        DMAImgo.dupCqIdx = this->DupQIdx;
        DMAImgo.burstQueIdx = this->BurstQNum;
        DMAImgo.dma_cfg.size.xsize =  queryRst.xsize_byte;
        DMAImgo.dma_cfg.size.h = imgo_cfg.crop.h;
        //crop r in pix domain
        DMAImgo.dma_cfg.crop.x = imgo_cfg.crop.x;
        DMAImgo.dma_cfg.crop.y = imgo_cfg.crop.y;

        DMAImgo.config();
        DMAImgo.write2CQ();
    }
    else if(this->CQ_D == CAM_ISP_CQ0_D){
        ISP_PATH_DBG("update imgo_d cropping\n");
        //xsize/ysize r in byte domain, need to be transfered
        e_PixMode = (DMAImgo_d.isTwinMode == MTRUE)? (ISP_QUERY_2_PIX_MODE):(ISP_QUERY_1_PIX_MODE);
        if(ISP_QuerySize(EPortIndex_IMGO,ISP_QUERY_CROP_X_BYTE,(EImageFormat)DMAImgo_d.dma_cfg.lIspColorfmt,imgo_cfg.crop.w,queryRst,e_PixMode) == 0){
            ISP_PATH_ERR("cropping size err(0x%x)\n",imgo_cfg.crop.w);
            return 1;
        }
        DMAImgo_d.dupCqIdx = this->DupQIdx;
        DMAImgo_d.burstQueIdx = this->BurstQNum;
        DMAImgo_d.dma_cfg.size.xsize =  queryRst.xsize_byte;
        DMAImgo_d.dma_cfg.size.h = imgo_cfg.crop.h;
        //crop r in pix domain
        DMAImgo_d.dma_cfg.crop.x = imgo_cfg.crop.x;
        DMAImgo_d.dma_cfg.crop.y = imgo_cfg.crop.y;

        DMAImgo_d.config();
        DMAImgo_d.write2CQ();
    }

    return ret;
}

MUINT32 CamPathPass1::setP1MagicNum(MUINT32 const magicNum)
{
    MUINT32 ret = 0;
    if ((ISP_PASS1 != this->pass1_path)&& (ISP_PASS1_D != this->pass1_path)) {
        ISP_PATH_ERR("Invalid Path to set m# (%d)", this->pass1_path);
        return 0;
    }
    //ISP_PATH_DBG("set magic num(0x%x)%d-%d\n",magicNum, this->BurstQNum, this->DupQIdx);
    ispRawPipe.burstQueIdx      = this->BurstQNum;
    ispRawPipe.dupCqIdx         = this->DupQIdx;
    ispRawPipe.pPass1Ctrl->burstQueIdx = this->BurstQNum;
    ispRawPipe.pPass1Ctrl->dupCqIdx = this->DupQIdx;

    if(0!= ispPass1Ctrl.setP1MagicNum(this->pass1_path,magicNum,1)){
        ret = 1;
        ISP_PATH_ERR("set magic num(0x%x) fail\n",magicNum);
    }
    return ret;
}

int CamPathPass1::setP1RrzCfg(IspRrzCfg const rrz_cfg)
{
    //ISP_PATH_DBG("+");
    ISP_QUERY_RST queryRst = (rrz_cfg.isTwinMode == MTRUE)? (ISP_QUERY_2_PIX_MODE):(ISP_QUERY_1_PIX_MODE);
    E_ISP_PIXMODE e_PixMode;
    if ((ISP_PASS1 != rrz_cfg.pass1_path)&& (ISP_PASS1_D != rrz_cfg.pass1_path)) {
        ISP_PATH_ERR("Invalid Path to use RRZ (%d)", rrz_cfg.pass1_path);
        return 0;
    }

    //4 We use RRZ to corp, thus almost the RRZ's crop width & height is 0
    if ( rrz_cfg.isTwinMode || (ISP_PASS1 == rrz_cfg.pass1_path) ) {
        //rrzo
        DMARrzo.dupCqIdx = this->DupQIdx;
        DMARrzo.burstQueIdx = this->BurstQNum;
        DMARrzo.dma_cfg.crop.x = 0;
        DMARrzo.dma_cfg.crop.y = 0;
        DMARrzo.dma_cfg.crop.floatX = 0;
        DMARrzo.dma_cfg.crop.floatY = 0;
        DMARrzo.dma_cfg.crop.w = 0; //no used
        DMARrzo.dma_cfg.crop.h = 0; //no used
        DMARrzo.dma_cfg.size.w = rrz_cfg.rrz_out_size.w;
        DMARrzo.dma_cfg.size.h = rrz_cfg.rrz_out_size.h;
        //mark, use original stride which is configured at init phase
        //DMARrzo.dma_cfg.size.stride = rrz_cfg.rrz_out_size.stride;
        ISP_QuerySize(EPortIndex_RRZO,ISP_QUERY_XSIZE_BYTE,(EImageFormat)DMARrzo.dma_cfg.lIspColorfmt,DMARrzo.dma_cfg.size.w,queryRst,e_PixMode);
        DMARrzo.dma_cfg.size.xsize      =  queryRst.xsize_byte;
        DMARrzo.config();
        //
        //ispRawPipe.rrz_in_size      = p_parameter->rrz_in_size;
        ispRawPipe.rrz_in_roi       = rrz_cfg.rrz_in_roi;
        ispRawPipe.rrz_out_size     = rrz_cfg.rrz_out_size;
        ispRawPipe.rrz_in_size      = rrz_cfg.rrz_in_size;
        ispRawPipe.m_num_0          = rrz_cfg.m_num_0;
        ispRawPipe.path             = rrz_cfg.pass1_path;

        /*ISP_PATH_DBG("crop[%d,%d], rrz_in_size(%d,%d), outSize(%d,%d), magicNum=%d,%d", \
             DMARrzo.dma_cfg.crop.x ,DMARrzo.dma_cfg.crop.y, \
             ispRawPipe.rrz_in_size.w, ispRawPipe.rrz_in_size.h, rrz_cfg.rrz_out_size.w, rrz_cfg.rrz_out_size.h,\
              rrz_cfg.m_num_0, rrz_cfg.m_num_1);*/
    }
    else if (ISP_PASS1_D == rrz_cfg.pass1_path) {
        //rrzo_d
        DMARrzo_d.dupCqIdx = this->DupQIdx;
        DMARrzo_d.burstQueIdx = this->BurstQNum;
        DMARrzo_d.dma_cfg.crop.x = 0;
        DMARrzo_d.dma_cfg.crop.y = 0;
        DMARrzo_d.dma_cfg.crop.floatX = 0;
        DMARrzo_d.dma_cfg.crop.floatY = 0;
        DMARrzo_d.dma_cfg.crop.w = 0; //no used
        DMARrzo_d.dma_cfg.crop.h = 0; //no used
        DMARrzo_d.dma_cfg.size.w = rrz_cfg.rrz_out_size.w;
        DMARrzo_d.dma_cfg.size.h = rrz_cfg.rrz_out_size.h;
        //mark, use original stride which is configured at init phase
        //DMARrzo_d.dma_cfg.size.stride = rrz_cfg.rrz_out_size.stride;
        ISP_QuerySize(EPortIndex_RRZO_D,ISP_QUERY_XSIZE_BYTE,(EImageFormat)DMARrzo_d.dma_cfg.lIspColorfmt,DMARrzo_d.dma_cfg.size.w,queryRst,e_PixMode);
        DMARrzo_d.dma_cfg.size.xsize        =  queryRst.xsize_byte;
        DMARrzo_d.config();
        //
        //ispRawPipe.rrz_d_in_size      = p_parameter->rrz_in_size;
        ispRawPipe.rrz_d_in_roi       = rrz_cfg.rrz_in_roi;
        ispRawPipe.rrz_d_out_size     = rrz_cfg.rrz_out_size;
        ispRawPipe.rrz_in_size        = rrz_cfg.rrz_in_size;
        ispRawPipe.m_num_0_d   = rrz_cfg.m_num_0;
        ispRawPipe.pathD             = rrz_cfg.pass1_path;

        /*ISP_PATH_DBG("D: crop[%d,%d], rrz_in_size(%d,%d), outSize(%d,%d), magicNum=%d,%d", \
            DMARrzo_d.dma_cfg.crop.x ,DMARrzo_d.dma_cfg.crop.y,\
            ispRawPipe.rrz_in_size.w, ispRawPipe.rrz_in_size.h, rrz_cfg.rrz_out_size.w, rrz_cfg.rrz_out_size.h,\
             rrz_cfg.m_num_0, rrz_cfg.m_num_1);*/

    }
    ispRawPipe.isTwinMode       = rrz_cfg.isTwinMode;
    ispRawPipe.burstQueIdx      = this->BurstQNum;
    ispRawPipe.dupCqIdx         = this->DupQIdx;
    ispRawPipe.pPass1Ctrl->burstQueIdx = this->BurstQNum;
    ispRawPipe.pPass1Ctrl->dupCqIdx = this->DupQIdx;
    ispRawPipe.img_sel          = rrz_cfg.mImgSel;
    //
    ispRawPipe.config();

    //ISP_PATH_DBG("-");
    return 0;
}


int CamPathPass1::setP1TuneCfg(MUINT32 magicNum, MINT32 senDev)
{
    int status  = 0;
    //ISP_PATH_DBG("+");

    ispRawPipe.burstQueIdx = this->BurstQNum;
    ispRawPipe.dupCqIdx = this->DupQIdx;
    status = ispRawPipe.setP1TuneCfg(magicNum, senDev);
    //ISP_PATH_DBG("-");
    return status;
}

MUINT32 CamPathPass1::m_p1NotifyTbl[p1Notify_node] = {  EPIPECmd_SET_EIS_CBFP,  EPIPECmd_SET_LCS_CBFP,      \
                                                        EPIPECmd_MAX,           EPIPECmd_MAX,               \
                                                        EPIPECmd_MAX,           EPIPECmd_MAX,               \
                                                        EPIPECmd_MAX,           EPIPECmd_MAX,               \
                                                        EPIPECmd_MAX,           EPIPECmd_MAX};
//switch case must follow above table
#define _notify_()  {\
    ispRawPipe.pP1Tuning = (P1_TUNING_NOTIFY*)this->m_p1NotifyObj[node];\
    ISP_PATH_DBG("P1Notify: %s\n",ispRawPipe.pP1Tuning->TuningName());\
    switch(node){\
        case 0:\
            ret = ispRawPipe.setEIS();\
            break;\
        case 1:\
            ret = ispRawPipe.setLCS();\
            break;\
        default:\
            break;\
    }\
    if(ret != MTRUE)\
        ISP_PATH_ERR("P1Notify_%s fail! \n",ispRawPipe.pP1Tuning->TuningName());\
}

MBOOL CamPathPass1::P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj){
    MBOOL ret = MTRUE;
    MUINT32 tbl_len = 0;

    while(tbl_len < p1Notify_node){
        if(this->m_p1NotifyTbl[tbl_len] == cmd)
            break;
        tbl_len++;
    }
    if(tbl_len < p1Notify_node){
        switch(cmd){
            case EPIPECmd_SET_EIS_CBFP://only pass1 have EIS
                if(this->ispTopCtrl.path == ISP_PASS1)
                    this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_LCS_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            default:
                ret = MFALSE;
                ISP_PATH_ERR("un-supported p1notify_0x%x\n",cmd);
                break;
        }
    }
    return ret;
}

MUINT32 CamPathPass1::setP1Notify(void){
    MUINT32 node = 0;
    MUINT32 ret = MTRUE;
    while(node < p1Notify_node){
        if(this->m_p1NotifyObj[node]!= 0){
            _notify_();
        }
        node++;
    }
    return ret;
}

bool CamPathPass1::updateBurstQueueNum(int burstQNum)
{
    bool ret=MTRUE;
    ISP_PATH_INF("New Burst Queue Num(%d)",burstQNum);
    this->ispTopCtrl.m_pIspDrvShell->CQBufferMutexLock();
    ret=this->ispTopCtrl.m_pIspDrvShell->updateBurstQueueNum(burstQNum);
    this->ispTopCtrl.m_pIspDrvShell->CQBufferMutexLock();
    if(!ret)
    {
        ISP_PATH_ERR("update Burst Queue Num fail!");
    }
    return ret;
}


MINT32 CamPathPass1::updateCQI(void){
    if(this->CQ == CAM_ISP_CQ0){
        ISP_PATH_DBG("pass1 update CQ");
        DMACQ.CQ = this->CQ;
        DMACQ.CQ_D = CAM_ISP_CQ_NONE;
        DMACQ.dupCqIdx = this->DupQIdx;
        DMACQ.burstQueIdx = this->BurstQNum;
        return DMACQ.config();
    }else if(this->CQ_D == CAM_ISP_CQ0_D){
        ISP_PATH_DBG("pass1_d update CQ_D");
        DMACQ_D.CQ = CAM_ISP_CQ_NONE;
        DMACQ_D.CQ_D = this->CQ_D;
        DMACQ_D.dupCqIdx = this->DupQIdx;
        DMACQ_D.burstQueIdx = this->BurstQNum;
        return DMACQ_D.config();
    }
    else
    {
        ISP_PATH_ERR("CmdQ error 0x%x_0x%x\n",this->CQ,this->CQ_D);
    }
    return -1;
}

