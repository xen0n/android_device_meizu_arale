#define LOG_TAG "iio/pathp1_FrmB"
//
//
#include "cam_path_FrmB.h"
#include "mtkcam/imageio/p1HwCfg.h" //inclucde this is for EIS moulde
#include <mtkcam/imageio/ispio_utility.h>
//

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(path);
EXTERN_DBG_LOG_VARIABLE(path);

//
using namespace NSDrvCam_FrmB;
using namespace NSIspDrv_FrmB;
//
using namespace NSImageio_FrmB;
using namespace NSIspio_FrmB;
//

/*/////////////////////////////////////////////////////////////////////////////
  CamPathPass1
  /////////////////////////////////////////////////////////////////////////////*/
int CamPathPass1::config( struct CamPathPass1Parameter* p_parameter )
{
    ISP_PATH_DBG("CamPathPass1::config E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass1Lock);
    this->m_isp_function_count = 0;
    /*function List*/
    m_isp_function_list[this->m_isp_function_count++ ] = (IspFunction_B*)&ispTopCtrl;    //CQ must be after Top, because CQ reg bit r included in top
    m_isp_function_list[this->m_isp_function_count++ ] = (IspFunction_B*)&DMACQ;
    m_isp_function_list[this->m_isp_function_count++ ] = (IspFunction_B*)&ispRawPipe;
    m_isp_function_list[this->m_isp_function_count++ ] = (IspFunction_B*)&ispRgbPipe;
    m_isp_function_list[this->m_isp_function_count++ ] = (IspFunction_B*)&ispYuvPipe;
    m_isp_function_list[this->m_isp_function_count++ ] = (IspFunction_B*)&DMAImgo;
    m_isp_function_list[this->m_isp_function_count++ ] = (IspFunction_B*)&DMAImg2o;

    /***Setup isp function parameter***/
    //top
    ispTopCtrl.path = p_parameter->path;

    ispTopCtrl.en_Top  = p_parameter->en_Top;
    ispTopCtrl.sub_mode= p_parameter->sub_mode;
    ispTopCtrl.CQ      = p_parameter->CQ;
    ispTopCtrl.ctl_int = p_parameter->ctl_int;
    ispTopCtrl.fmt_sel = p_parameter->fmt_sel;
    ispTopCtrl.ctl_sel = p_parameter->ctl_sel;
    ispTopCtrl.ctl_mux_sel = p_parameter->ctl_mux_sel;
    ispTopCtrl.ctl_mux_sel2 = p_parameter->ctl_mux_sel2;
    ispTopCtrl.ctl_sram_mux_cfg = p_parameter->ctl_sram_mux_cfg;

    ispTopCtrl.isConcurrency = 0;
    ispTopCtrl.pix_id = p_parameter->pix_id;
    ispTopCtrl.b_continuous = 1;
    ispTopCtrl.isIspOn = 1;
    //this infor. have including p1 & p1_d ??
    ispPass1Ctrl.path = ispTopCtrl.path;
    ispPass1Ctrl.CQ = p_parameter->CQ;
    //raw pipe
    ispRawPipe.path   = p_parameter->path;
    ispRawPipe.CQ       = p_parameter->CQ;
    ispRawPipe.bypass = p_parameter->bypass_ispRawPipe;
    ispRawPipe.sub_mode= p_parameter->sub_mode;
    ispRawPipe.enable1 = p_parameter->en_Top.enable1;
    ispRawPipe.enable2 = p_parameter->en_Top.enable2;
    ispRawPipe.src_img_w = p_parameter->hrz_in_size.w;
    ispRawPipe.src_img_h = p_parameter->hrz_in_size.h;
    ispRawPipe.hrz_in = p_parameter->hrz_in_size;
    ispRawPipe.hrz_out = p_parameter->hrz_out_size;
    ispRawPipe.pPass1Ctrl = &ispPass1Ctrl;

    //
    ispRgbPipe.CQ = p_parameter->CQ;
    ispRgbPipe.bypass = p_parameter->bypass_ispRgbPipe;
    ispRgbPipe.enable1 = p_parameter->en_Top.enable1;
    ispRgbPipe.src_img_h = p_parameter->tg_out_size.h;
    //
    ispYuvPipe.CQ = p_parameter->CQ;
    ispYuvPipe.bypass = p_parameter->bypass_ispYuvPipe;
    ispYuvPipe.enable2 = p_parameter->en_Top.enable2;


    //imgo
    DMAImgo.bypass = 1;
    if ( CAM_CTL_DMA_EN_IMGO_EN & ispTopCtrl.en_Top.dma ) {
        DMAImgo.bypass = 0;
        DMAImgo.dma_cfg     = p_parameter->imgo;
        DMAImgo.dupCqIdx    = this->DupQIdx;
        DMAImgo.burstQueIdx    = this->BurstQNum;
        DMAImgo.CQ             = p_parameter->CQ;
    }


    //img2o
    DMAImg2o.bypass = 1;
    if ( CAM_CTL_DMA_EN_IMG2O_EN & ispTopCtrl.en_Top.dma ) {
        DMAImg2o.bypass = 0;
        DMAImg2o.dma_cfg     = p_parameter->img2o;
        DMAImg2o.dupCqIdx    = this->DupQIdx;
        DMAImg2o.burstQueIdx= this->BurstQNum;
        DMAImg2o.CQ            = p_parameter->CQ;
    }


    //CQ0
    DMACQ.bypass = 1;
    if(p_parameter->CQ != CAM_ISP_CQ_NONE){
        DMACQ.bypass = 0;
        DMACQ.CQ = p_parameter->CQ;
    }


    //buffer control path
    ispBufCtrl.path   = p_parameter->path;
    ispBufCtrl.CQ   = p_parameter->CQ;
    ispBufCtrl.init(0);



#if 0
    //real time buffer control
    if ( 0 < p_parameter->imgo.memBuf.cnt) {
        ispBufCtrl.init(ISP_DMA_IMGO);
    }
    //
    if ( 0 < p_parameter->img2o.memBuf.cnt) {
        ispBufCtrl.init(ISP_DMA_IMG2O);
    }
#endif

    //
    this->_config(NULL);
    //CQ0C config after _config is because of cq0C reg bit r included in TOP
    DMACQ.bypass = 0;
    DMACQ.CQ = ISP_DRV_CQ0C;
    DMACQ.cqTrigSrc = p_parameter->cqTrigSrc;
    DMACQ.config();

    //add for pass1 dup cmdQ.
    //ring here is inorder to maintain dma api consistance( other dma channel no init issue)
    for(int i=1;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
        if(DMAImgo.bypass != 1){
            DMAImgo.dupCqIdx = i;
            DMAImgo.config();
            DMAImgo.write2CQ();
        }
        if(DMAImg2o.bypass != 1){
            DMAImg2o.dupCqIdx = i;
            DMAImg2o.config();
            DMAImg2o.write2CQ();
        }
    }
    ISP_PATH_DBG("X");

    return 0;
}
int CamPathPass1::_waitIrq( int type, unsigned int irq, int irqclear, int userNumber)
{
    int ret = 0;
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    WaitIrq.Clear = (NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_ENUM)irqclear;
    WaitIrq.UserInfo.Type = (NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_ENUM)type;//ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.UserInfo.Status = irq;//ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    WaitIrq.UserInfo.UserKey = userNumber;
    WaitIrq.Timeout = 500;//ms ,0 means pass through.
    WaitIrq.bDumpReg = 0xFE;
    ISP_PATH_DBG("CamPathPass1::_waitIrq+, type=%x, irq=%x, clear=%d ", type, irq, irqclear);

    if ( MFALSE == (MBOOL)ispTopCtrl.waitIrq(&WaitIrq) ) {
        ret = -1;
    }

    ISP_PATH_DBG("ret(%d)",ret);

    return ret;
}
//
int CamPathPass1::_waitIrq( int type, unsigned int irq, int userNumber)
{
    int ret = 0;
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;


    WaitIrq.Clear = NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.UserInfo.Type = (NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_ENUM)type;//ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.UserInfo.Status = irq;//ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    WaitIrq.UserInfo.UserKey = userNumber;
    WaitIrq.Timeout = 500;//ms ,0 means pass through.
    WaitIrq.bDumpReg = 0xFE;
    ISP_PATH_DBG("CamPathPass1::_waitIrq+, type=%x, irq=%x ", type, irq);

    if ( MFALSE == (MBOOL)ispTopCtrl.waitIrq(&WaitIrq) ) {
        ret = -1;
    }

    ISP_PATH_DBG("ret(%d)",ret);

    return ret;
}

int CamPathPass1::setCdrz( IspSize out_size )
{
    int ret = 0;
#if 1
    ISP_PATH_ERR("no supported\n");
    ret = -1;
#else

    ISP_PATH_DBG("E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

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
    ISP_PATH_DBG("E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass1Lock);


    //check if there is already filled buffer
    if ( MFALSE == this->ispBufCtrl.waitBufReady(dmaChannel) ) {
        ISP_PATH_ERR("waitBufReady fail");
        return -1;
    }
    if ( 0 != this->ispBufCtrl.dequeueHwBuf( dmaChannel, bufInfo ) ) {
        ISP_PATH_ERR("ERROR:dequeueHwBuf");
        return -1;
    }

    ISP_PATH_DBG("[0x%x] ",bufInfo.pBufList->front().base_vAddr);
    ISP_PATH_DBG("X");
    return ret;
}
//
int CamPathPass1::setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src)
{
    int ret = 0;
    ISP_PATH_DBG("E:%d/%d/%d ",cq,mode,trig_src);

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass1Lock);

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
        e_PixMode = ISP_QUERY_1_PIX_MODE;
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
    else {
        ISP_PATH_ERR("error: unsupported CQ:0x%x\n",this->CQ);
        return 1;
    }

    return ret;
}

MUINT32 CamPathPass1::setP1MagicNum(UINT32 const magicNum){
    MUINT32 ret = 0;
    ispRawPipe.pPass1Ctrl->burstQueIdx = this->BurstQNum;
    ispRawPipe.pPass1Ctrl->dupCqIdx = this->DupQIdx;
    if(0!= ispPass1Ctrl.setP1MagicNum(ispPass1Ctrl.path,magicNum,1)){
        ret = 1;
        ISP_PATH_ERR("set magic num(0x%x) fail\n",magicNum);
    }
    return ret;
}

int CamPathPass1::setP1HrzCfg(IspHrzCfg const hrz_cfg)
{
    //ISP_PATH_DBG("+");
    ISP_QUERY_RST queryRst;
    if (ISP_PASS1 != hrz_cfg.pass1_path) {
        ISP_PATH_ERR("Invalid Path to use HRZ (%d)", hrz_cfg.pass1_path);
        return 0;
    }

    //4 We use HRZ to corp, thus almost the RRZ's crop width & height is 0
    if ( ISP_PASS1 == hrz_cfg.pass1_path) {
        //img2o
        DMAImg2o.dupCqIdx = this->DupQIdx;
        DMAImg2o.burstQueIdx = this->BurstQNum;
        DMAImg2o.dma_cfg.crop.x = hrz_cfg.img2o_crop.x;
        DMAImg2o.dma_cfg.crop.y = hrz_cfg.img2o_crop.y;
        DMAImg2o.dma_cfg.crop.floatX = hrz_cfg.img2o_crop.floatX;
        DMAImg2o.dma_cfg.crop.floatY = hrz_cfg.img2o_crop.floatY;
        DMAImg2o.dma_cfg.crop.w = hrz_cfg.img2o_crop.w;
        DMAImg2o.dma_cfg.crop.h = hrz_cfg.img2o_crop.h;
        DMAImg2o.dma_cfg.size.w = hrz_cfg.hrz_out_size.w;
        DMAImg2o.dma_cfg.size.h = hrz_cfg.hrz_out_size.h;
        //mark, use original stride which is configured at init phase
        //DMAImg2o.dma_cfg.size.stride = hrz_cfg.hrz_out_size.stride;
        ISP_QuerySize(EPortIndex_IMG2O,ISP_QUERY_XSIZE_BYTE,(EImageFormat)DMAImg2o.dma_cfg.lIspColorfmt,DMAImg2o.dma_cfg.crop.w,queryRst);
        DMAImg2o.dma_cfg.size.xsize     =  queryRst.xsize_byte;
        DMAImg2o.config();
        //
        //ispRawPipe.hrz_in_size      = p_parameter->hrz_in_size;
        ispRawPipe.hrz_out             = hrz_cfg.hrz_out_size;
        ispRawPipe.hrz_in            = hrz_cfg.hrz_in_size;
        ispRawPipe.magicNum            = hrz_cfg.m_num_0;
        ispRawPipe.path             = hrz_cfg.pass1_path;

        /*ISP_PATH_DBG("crop[%d,%d], hrz_in_size(%d,%d), outSize(%d,%d), magicNum=%d,%d", \
             DMAHrzo.dma_cfg.crop.x ,DMAHrzo.dma_cfg.crop.y, \
             ispRawPipe.hrz_in_size.w, ispRawPipe.hrz_in_size.h, hrz_cfg.hrz_out_size.w, hrz_cfg.hrz_out_size.h,\
              hrz_cfg.m_num_0, hrz_cfg.m_num_1);*/
    }
    ispRawPipe.burstQueIdx        = this->BurstQNum;
    ispRawPipe.dupCqIdx         = this->DupQIdx;
    ispRawPipe.pPass1Ctrl->burstQueIdx = this->BurstQNum;
    ispRawPipe.pPass1Ctrl->dupCqIdx = this->DupQIdx;

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

MUINT32 CamPathPass1::m_p1NotifyTbl[p1Notify_node] = {    EPIPECmd_SET_EIS_CBFP,    EPIPECmd_SET_LCS_CBFP,        \
                                                         EPIPECmd_MAX,            EPIPECmd_MAX,                \
                                                         EPIPECmd_MAX,            EPIPECmd_MAX,                \
                                                         EPIPECmd_MAX,            EPIPECmd_MAX,                \
                                                         EPIPECmd_MAX,            EPIPECmd_MAX};


//switch case must follow above table
#define _notify_()    {\
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

MBOOL CamPathPass1::P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* obj){
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
                    this->m_p1NotifyObj[tbl_len] = obj;
                break;
            case EPIPECmd_SET_LCS_CBFP:
                this->m_p1NotifyObj[tbl_len] = obj;
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
    ret=this->ispTopCtrl.m_pIspDrvShell->updateBurstQueueNum(burstQNum);
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
        DMACQ.dupCqIdx = this->DupQIdx;
        DMACQ.burstQueIdx = this->BurstQNum;
        return DMACQ.config();
    }
    else
    {
        ISP_PATH_ERR("CmdQ error 0x%x_0x%x\n",this->CQ,this->CQ_D);
    }
    return -1;
}



