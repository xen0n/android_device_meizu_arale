#define LOG_TAG "iio/pathp2_FrmB"
//
//
#include "cam_path_FrmB.h"
//

/*******************************************************************************
*
********************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(path);
EXTERN_DBG_LOG_VARIABLE(path);

static int P2_UT_BypassMDP_TPIPE = 0;


//
using namespace NSDrvCam_FrmB;
using namespace NSIspDrv_FrmB;
//


// the size only affect tpipe table
#define MAX_TPIPE_WIDTH                  (768)
#define MAX_TPIPE_HEIGHT                 (8192)
#define CONCURRENCY_RING_TPIPE_HEIGHT    (384)
#define CONCURRENCY_CAP_TPIPE_HEIGHT     (960)


// tpipe irq mode
#define TPIPE_IRQ_FRAME     (0)
#define TPIPE_IRQ_LINE      (1)
#define TPIPE_IRQ_TPIPE     (2)


// tpipe perform information
#define TPIPE_MAX_THROUGHPUT_PER_VSYNC      (6990506)  // 200M/30fps
#define TPIPE_THROUGHPUT_FOR_JPEG           ((1024*1024)>>1)   // 0.5M


// digital zoom setting
#define DIGITAL_ZOOM_FP_NUM     (10)
#define CDRZ_MIN_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)/128)
#define CDRZ_MAX_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define CURZ_MIN_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)/2)
#define CURZ_MAX_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define PRZ_MIN_SCALE_RATIO     ((1 << DIGITAL_ZOOM_FP_NUM)/128)
#define PRZ_MAX_SCALE_RATIO     ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define BASIC_SCALE_RATIO       (1 << DIGITAL_ZOOM_FP_NUM)

#define CROP_MAX_RATIO      (1000 << DIGITAL_ZOOM_FP_NUM)


#define IS_ERROR_BLOCKING   (0)   // 1:blocking the current thread

#if IS_ERROR_BLOCKING
#define FUNCTION_BLOCKING   do{ISP_PATH_ERR("[Error]blocking\n");}while(1);
#else
#define FUNCTION_BLOCKING
#endif


/*******************************************************************************
*
********************************************************************************/
/*/////////////////////////////////////////////////////////////////////////////
  CamPathPass2
  /////////////////////////////////////////////////////////////////////////////*/
int CamPathPass2::config( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;    // 0: success. -1: error.
    MUINT32 cnt;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 hAlgo,vAlgo;
    MUINT32 hTable, vTable;
    MUINT32 hCoeffStep,vCoeffStep;
    MUINT32 wrotoXCropRatio, wrotoYCropRatio, wrotoCropRatio;
    MUINT32 wdmaoXCropRatio, wdmaoYCropRatio, wdmaoCropRatio;

    ISP_PATH_DBG("CamPathPass2::config tdri(%d) E",p_parameter->tpipe);

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    /*function List*/
    cnt = 0;
    m_isp_function_list[cnt++] = (IspFunction_B*)&DMACQ;
    m_isp_function_list[cnt++] = (IspFunction_B*)&ispTuningCtrl;
    m_isp_function_list[cnt++] = (IspFunction_B*)&ispTopCtrl;
    m_isp_function_list[cnt++] = (IspFunction_B*)&ispRawPipe;
    m_isp_function_list[cnt++] = (IspFunction_B*)&ispRgbPipe;
    m_isp_function_list[cnt++] = (IspFunction_B*)&ispYuvPipe;
    m_isp_function_list[cnt++] = (IspFunction_B*)&cdpPipe;
    m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImgi;
    m_isp_function_list[cnt++] = (IspFunction_B*)&tdriPipe;
    m_isp_function_list[cnt++] = (IspFunction_B*)&DMALsci;
    m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImgo;
    m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImg2o;
    m_isp_function_list[cnt++] = (IspFunction_B*)&ispMdpPipe;
    m_isp_function_count = cnt;

    //tuning
    ispTuningCtrl.bypass = 0;

    /////////////////////////////////////////////
    //CHRISTOPHER: temp bypass mdp for UT, temp disable TPIPE
    if(P2_UT_BypassMDP_TPIPE)
    {
        //ispMdpPipe.bypass=1;
        p_parameter->tpipe=CAM_MODE_FRAME;
        DMAImg2o.dma_cfg = p_parameter->img2o;
    }
    /////////////////////////////////////////////

    //top
    switch(p_parameter->CQ){
        case NSIspDrv_FrmB::CAM_ISP_CQ2:
            ispTopCtrl.path = ISP_PASS2B;
            ispRawPipe.path = ISP_PASS2B;
            break;
        case NSIspDrv_FrmB::CAM_ISP_CQ3:
            ispTopCtrl.path = ISP_PASS2C;
            ispRawPipe.path = ISP_PASS2C;
            break;
        case NSIspDrv_FrmB::CAM_ISP_CQ1:
        default:
            ispTopCtrl.path = ISP_PASS2;
            ispRawPipe.path = ISP_PASS2;
            break;
    }

    ispTopCtrl.en_Top  = p_parameter->en_Top;
    ispTopCtrl.ctl_int = p_parameter->ctl_int;
    ispTopCtrl.fmt_sel = p_parameter->fmt_sel;
    ispTopCtrl.ctl_sel = p_parameter->ctl_sel;
    ispTopCtrl.ctl_mux_sel = p_parameter->ctl_mux_sel;
    ispTopCtrl.ctl_mux_sel2 = p_parameter->ctl_mux_sel2;
    ispTopCtrl.ctl_sram_mux_cfg = p_parameter->ctl_sram_mux_cfg;
    ispTopCtrl.isEn1C24StatusFixed = p_parameter->isEn1C24StatusFixed;
    ispTopCtrl.isEn1CfaStatusFixed = p_parameter->isEn1CfaStatusFixed;
    ispTopCtrl.isEn1HrzStatusFixed = p_parameter->isEn1HrzStatusFixed;
    ispTopCtrl.isEn2CdrzStatusFixed = p_parameter->isEn2CdrzStatusFixed;
    ispTopCtrl.isEn2G2cStatusFixed = p_parameter->isEn2G2cStatusFixed;
    ispTopCtrl.isEn2C42StatusFixed = p_parameter->isEn2C42StatusFixed;
    ispTopCtrl.isImg2oStatusFixed = p_parameter->isImg2oStatusFixed;
    ispTopCtrl.isImgoStatusFixed = p_parameter->isImgoStatusFixed;
    ispTopCtrl.isAaoStatusFixed = p_parameter->isAaoStatusFixed;
    ispTopCtrl.isEsfkoStatusFixed = p_parameter->isEsfkoStatusFixed;
    ispTopCtrl.isFlkiStatusFixed = p_parameter->isFlkiStatusFixed;
    ispTopCtrl.isLcsoStatusFixed = p_parameter->isLcsoStatusFixed;
    ispTopCtrl.isEn1AaaGropStatusFixed = p_parameter->isEn1AaaGropStatusFixed;
    ispTopCtrl.isShareDmaCtlByTurn = p_parameter->isShareDmaCtlByTurn;
    ispTopCtrl.isConcurrency = p_parameter->isConcurrency;
    p_parameter->pix_id = ispTopCtrl.pix_id; /* get pix_id from pass1 */
    p_parameter->ctl_pix_id.bit_field.PIX_ID =  ispTopCtrl.pix_id; //kk test
    ispTopCtrl.ctl_pix_id = p_parameter->ctl_pix_id.reg_val;
    ispTopCtrl.isIspOn = p_parameter->isIspOn;
    ispTopCtrl.isV3 = p_parameter->isV3;
    ispTopCtrl.isApplyTuning = p_parameter->isApplyTuning;
    //isp_raw
    ispRawPipe.bypass = p_parameter->bypass_ispRawPipe;
    ispRawPipe.enable1 = p_parameter->en_Top.enable1;
    ispRawPipe.enable2 = p_parameter->en_Top.enable2;
    ispRawPipe.src_img_w = p_parameter->src_img_size.w;
    ispRawPipe.src_img_h = p_parameter->src_img_size.h;
    ispRawPipe.isV3 = p_parameter->isV3;
    ispRawPipe.isApplyTuning = p_parameter->isApplyTuning;
    ispRawPipe.p2SubMode = p_parameter->p2SubMode;
    ispRawPipe.pTuningIspReg = p_parameter->pTuningIspReg;
    //isp_rgb
    ispRgbPipe.bypass = p_parameter->bypass_ispRgbPipe;
    ispRgbPipe.enable1 = p_parameter->en_Top.enable1;
    ispRgbPipe.enable2 = p_parameter->en_Top.enable2;
    ispRgbPipe.src_img_h = p_parameter->src_img_size.h;
    ispRgbPipe.isV3 = p_parameter->isV3;
    ispRgbPipe.isApplyTuning = p_parameter->isApplyTuning;
    ispRgbPipe.p2SubMode = p_parameter->p2SubMode;
    ispRgbPipe.pTuningIspReg = p_parameter->pTuningIspReg;
    ispRgbPipe.hrzRatio = p_parameter->hrzRatio;
    ispRgbPipe.hrzCropX = p_parameter->hrzCropX;
    ispRgbPipe.hrzCropY = p_parameter->hrzCropY;

    //isp_yuv
    ispYuvPipe.bypass = p_parameter->bypass_ispYuvPipe;
    ispYuvPipe.enable1 = p_parameter->en_Top.enable1;
    ispYuvPipe.enable2 = p_parameter->en_Top.enable2;
    ispYuvPipe.isV3 = p_parameter->isV3;
    ispYuvPipe.isApplyTuning = p_parameter->isApplyTuning;
    ispYuvPipe.p2SubMode = p_parameter->p2SubMode;
    ispYuvPipe.pTuningIspReg = p_parameter->pTuningIspReg;

    ISP_PATH_DBG("enable1(0x%x) enable2(0x%x) dma(0x%x),cq(%d)\n",p_parameter->en_Top.enable1,p_parameter->en_Top.enable2,p_parameter->en_Top.dma,p_parameter->CQ);


    /*
     * RESIZER
     */
    switch (p_parameter->tpipe)
    {
        case CAM_MODE_FRAME:
        cdpPipe.tpipeMode = CDP_DRV_MODE_FRAME;
        break;

        case CAM_MODE_TPIPE:
        cdpPipe.tpipeMode = CDP_DRV_MODE_TPIPE;
        break;
    }

    //
    //cdpPipe, need revised
    cdpPipe.disp_vid_sel = CAM_CDP_PRZ_CONN_TO_DISPO;
    cdpPipe.enable2 = p_parameter->en_Top.enable2;
    cdpPipe.dma_enable = p_parameter->en_Top.dma;
    cdpPipe.bypass = 1;
    cdpPipe.conf_rotDMA = 1;
    cdpPipe.isV3 = p_parameter->isV3;
    cdpPipe.isApplyTuning = p_parameter->isApplyTuning;
    cdpPipe.conf_cdrz = 0;
    //
    #if 0 //82 no need to take care if PRZ neect to VIDO or DISPO //     CAM_CDP_PRZ_CONN_TO_DISPO or CAM_CDP_PRZ_CONN_TO_VIDO
    getCdpMuxSetting(*p_parameter, &cdpPipe.disp_vid_sel); // get mux "disp_vid_sel" value
    #endif
    ISP_PATH_DBG("disp_vid_sel(%d) \n",cdpPipe.disp_vid_sel);
    ISP_PATH_DBG("[imgi] in[%d, %d] imgiCrop[%d, %d, %d, %d]_f(0x%x, 0x%x)\n",p_parameter->imgi.size.w,p_parameter->imgi.size.h, \
                p_parameter->imgi.crop.x,p_parameter->imgi.crop.y,p_parameter->imgi.crop.w,p_parameter->imgi.crop.h, \
                p_parameter->imgi.crop.floatX,p_parameter->imgi.crop.floatY);


    if(CAM_CTL_EN2_CDRZ_EN & p_parameter->en_Top.enable2 )
    {
        ISP_PATH_DBG("[imgi] in(%d,%d),out(%d,%d),imgiCrop(%d,%d,%d,%d),CropFloat(0x%x,0x%x)\n", p_parameter->croppara.cdrz_in.w,
                                                                                       p_parameter->croppara.cdrz_in.h,
                                                                                       p_parameter->croppara.cdrz_out.h,
                                                                                       p_parameter->croppara.cdrz_out.h,
                                                                                       p_parameter->croppara.cdrz_crop.x,
                                                                                       p_parameter->croppara.cdrz_crop.y,
                                                                                       p_parameter->croppara.cdrz_crop.w,
                                                                                       p_parameter->croppara.cdrz_crop.h,
                                                                                       p_parameter->croppara.cdrz_crop.floatX,
                                                                                       p_parameter->croppara.cdrz_crop.floatY);
    cdpPipe.bypass = 0;
        cdpPipe.conf_cdrz = 1;

        cdpPipe.cdrz_in   = p_parameter->croppara.cdrz_in;
        cdpPipe.cdrz_out  = p_parameter->croppara.cdrz_out;

        cdpPipe.cdrz_crop.x      = p_parameter->croppara.cdrz_crop.x;
        cdpPipe.cdrz_crop.floatX = p_parameter->croppara.cdrz_crop.floatX;
        cdpPipe.cdrz_crop.y      = p_parameter->croppara.cdrz_crop.y;
        cdpPipe.cdrz_crop.floatY = p_parameter->croppara.cdrz_crop.floatY;
        cdpPipe.cdrz_crop.w      = p_parameter->croppara.cdrz_crop.w;
        cdpPipe.cdrz_crop.h      = p_parameter->croppara.cdrz_crop.h;

    }


    //disp_vid_sel set for tpipe
    p_parameter->ctl_sel.bit_field.disp_vid_sel = ispTopCtrl.ctl_sel.bit_field.disp_vid_sel;

    //mdp
    ispMdpPipe.wdmao_out = p_parameter->wdmao;
    ispMdpPipe.wroto_out = p_parameter->wroto;
    ispMdpPipe.enable2 = p_parameter->en_Top.enable2;
    ispMdpPipe.dma_enable = p_parameter->en_Top.dma;
    ispMdpPipe.isV3 = p_parameter->isV3;
    ispMdpPipe.isApplyTuning = p_parameter->isApplyTuning;
    ispMdpPipe.isMdpCropEn = p_parameter->isMdpCropEn;

    /////////////////////////////////////////////////////////////////
    //DMA
    //imgi
    DMAImgi.dma_cfg = p_parameter->imgi;
    ISP_PATH_DBG("[imgi](%d , %d , %d) vidoRotate(%d)\n", p_parameter->imgi.size.w, p_parameter->imgi.size.h, p_parameter->imgi.size.stride,ispMdpPipe.wroto_out.Rotation);

    //mdpi
    DMAImgi.p2MdpSrcFmt = p_parameter->croppara.mdp_src_fmt;
    DMAImgi.p2MdpSrcW = p_parameter->croppara.mdp_Yin.w;
    DMAImgi.p2MdpSrcH = p_parameter->croppara.mdp_Yin.h;
    DMAImgi.p2MdpSrcYStride = p_parameter->croppara.mdp_Yin.stride;
    DMAImgi.p2MdpSrcUVStride = p_parameter->croppara.mdp_UVin.stride;
    DMAImgi.p2MdpSrcSize = p_parameter->croppara.mdp_src_size;
    DMAImgi.p2MdpSrcCSize = p_parameter->croppara.mdp_src_Csize;
    DMAImgi.p2MdpSrcVSize = p_parameter->croppara.mdp_src_Vsize;
    DMAImgi.p2MdpSrcPlaneNum = p_parameter->croppara.mdp_src_Plane_Num;


    ISP_PATH_DBG("[imgi_mdp]p2MdpSrcFmt(0x%x),[Pa]dispo(0x%x) vido(0x%x) imgi(0x%x) \n", \
        DMAImgi.p2MdpSrcFmt,p_parameter->wdmao.memBuf.base_pAddr,p_parameter->wroto.memBuf.base_pAddr,p_parameter->imgi.memBuf.base_pAddr);

    //imgci
    DMALsci.bypass = 1;
    DMAImgo.bypass = 1;
    DMAImg2o.bypass = 1;
    //lsci
    if ( CAM_CTL_DMA_EN_LSCI_EN & ispTopCtrl.en_Top.dma ) {
        DMALsci.bypass = 0;
        DMALsci.dma_cfg = p_parameter->lsci;
    }
    //imgo
    if ( CAM_CTL_DMA_EN_IMGO_EN & ispTopCtrl.en_Top.dma ) {
        DMAImgo.bypass = 0;
        DMAImgo.dma_cfg = p_parameter->imgo;
    }
    //img2o
    if ( CAM_CTL_DMA_EN_IMG2O_EN & ispTopCtrl.en_Top.dma ) {
        DMAImg2o.bypass = 0;
        DMAImg2o.dma_cfg = p_parameter->img2o;
    }

    /* config MDP pipe */
    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        ispMdpPipe.drvScenario = p_parameter->drvScenario;
        ispMdpPipe.src_crop.x = p_parameter->croppara.mdp_crop.x;
        ispMdpPipe.src_crop.floatX = ((p_parameter->croppara.mdp_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.src_crop.y = p_parameter->croppara.mdp_crop.y;
        ispMdpPipe.src_crop.floatY = ((p_parameter->croppara.mdp_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.src_crop.w = p_parameter->croppara.mdp_crop.w;
        ispMdpPipe.src_crop.h = p_parameter->croppara.mdp_crop.h;
        ispMdpPipe.isWaitBuf = p_parameter->isWaitBuf;
        ispMdpPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispMdpPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispMdpPipe.mdp_imgxo_p2_en = p_parameter->mdp_imgxo_p2_en;
        ispMdpPipe.wroto_out = p_parameter->wroto; //wroto
        ispMdpPipe.wdmao_out = p_parameter->wdmao; //wdmao
        ispMdpPipe.imgxo_out = p_parameter->imgxo; //imgxo
        ISP_PATH_DBG("[mdp_src](x,y,w,h)=(%d,%d,%d,%d)",ispMdpPipe.src_crop.x,ispMdpPipe.src_crop.y,ispMdpPipe.src_crop.w,ispMdpPipe.src_crop.h);
    }


    // tpipemain
    tdriPipe.enTdri = p_parameter->tpipe;
    tdriPipe.isV3 = p_parameter->isV3;
    tdriPipe.isApplyTuning = p_parameter->isApplyTuning;

    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        DMATdri.bypass = 0;
        tdriPipe.bypass = 0;
        //n-tap NOT support tpipe mode
        cdpPipe.cdrz_filter = CAM_CDP_CDRZ_8_TAP;

        ISP_PATH_DBG("p_parameter->tcm_en = (0x%8x)\n", p_parameter->tcm_en);
        this->configTpipeData(p_parameter);

        DMATdri.dma_cfg = p_parameter->tdri;
        ISP_PATH_DBG("ispTopCtrl.en_Top.enable1 = (0x%8x)\n", ispTopCtrl.en_Top.enable1);

        tdriPipe.tcm_en = p_parameter->tcm_en;
        tdriPipe.tdr_en = (p_parameter->tdr_en&0xc0000000)\
            |(tdriPipe.tdri.tdriPerformCfg.tpipeWidth&0x3ff)\
            |(tdriPipe.tdri.tdriPerformCfg.tpipeHeight&0x7FFC00);

        // update tpipe width/height
        cdpPipe.tpipe_w = tdriPipe.tdri.tdriPerformCfg.tpipeWidth;
        ispTopCtrl.tpipe_w = tdriPipe.tdri.tdriPerformCfg.tpipeWidth;
        ispTopCtrl.tpipe_h = tdriPipe.tdri.tdriPerformCfg.tpipeHeight;
    }
    else {  // CAM_MODE_FRAME
        tdriPipe.tcm_en = 0x0;
        DMATdri.bypass = 1;
        //tdriPipe.bypass = 1;
        tdriPipe.tdr_en = 0x1F;
    }

    //buffer control path
    ispBufCtrl.path = ispTopCtrl.path;

    //pass2 commandQ
    if ( CAM_ISP_CQ_NONE != p_parameter->CQ )
    {
        //CAM_ISP_CQ0 is illegal
        DMACQ.bypass = 0; //
        DMACQ.dma_cfg = p_parameter->cqi;
        ispTuningCtrl.CQ = p_parameter->CQ;
        ispTopCtrl.CQ = p_parameter->CQ;
        ispRawPipe.CQ = p_parameter->CQ;
        ispRgbPipe.CQ = p_parameter->CQ;
        ispYuvPipe.CQ = p_parameter->CQ;
        ispMdpPipe.CQ = p_parameter->CQ;
        cdpPipe.CQ = p_parameter->CQ;
        DMAImgi.CQ = p_parameter->CQ;
    DMAImgo.CQ = p_parameter->CQ;
    DMAImg2o.CQ = p_parameter->CQ;
        DMATdri.CQ = p_parameter->CQ;
        DMACQ.CQ = p_parameter->CQ;
        tdriPipe.CQ = p_parameter->CQ;
        DMALsci.CQ = p_parameter->CQ;
        DMAImgo.CQ = p_parameter->CQ;
        this->CQ = p_parameter->CQ;//for path config

    DMACQ.dupCqIdx = p_parameter->dupCqIdx;
        ispTopCtrl.dupCqIdx = p_parameter->dupCqIdx;
        ispRawPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispRgbPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispYuvPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispMdpPipe.dupCqIdx = p_parameter->dupCqIdx;
        cdpPipe.dupCqIdx = p_parameter->dupCqIdx;
        DMAImgi.dupCqIdx = p_parameter->dupCqIdx;
    DMAImg2o.dupCqIdx = p_parameter->dupCqIdx;
        DMATdri.dupCqIdx = p_parameter->dupCqIdx;
        tdriPipe.dupCqIdx = p_parameter->dupCqIdx;
        DMALsci.dupCqIdx = p_parameter->dupCqIdx;
        DMAImgo.dupCqIdx = p_parameter->dupCqIdx;

        //burstQ number
    DMACQ.burstQueIdx = p_parameter->burstQueIdx;
        ispTopCtrl.burstQueIdx = p_parameter->burstQueIdx;
        ispRawPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispRgbPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispYuvPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispMdpPipe.burstQueIdx = p_parameter->burstQueIdx;
        cdpPipe.burstQueIdx = p_parameter->burstQueIdx;
        DMAImgi.burstQueIdx = p_parameter->burstQueIdx;
        DMATdri.burstQueIdx = p_parameter->burstQueIdx;
        tdriPipe.burstQueIdx = p_parameter->burstQueIdx;
        DMALsci.burstQueIdx = p_parameter->burstQueIdx;
        DMAImgo.burstQueIdx = p_parameter->burstQueIdx;
        DMAImg2o.burstQueIdx = p_parameter->burstQueIdx;

        ispTopCtrl.drvScenario = p_parameter->drvScenario;
        ispRawPipe.drvScenario = p_parameter->drvScenario;
        ispRgbPipe.drvScenario = p_parameter->drvScenario;
        ispYuvPipe.drvScenario = p_parameter->drvScenario;
        ispMdpPipe.drvScenario = p_parameter->drvScenario;

    }
    else
    {
        DMACQ.bypass = 1;
    }

    // set scenario ID
    DMACQ.DMAI_B::sceID    = p_parameter->scenario;
    DMAImgi.DMAI_B::sceID  = p_parameter->scenario;
    DMATdri.DMAI_B::sceID  = p_parameter->scenario;
    DMALsci.DMAI_B::sceID  = p_parameter->scenario;

    // config
    this->_config((void*)p_parameter);

EXIT:

    ISP_PATH_DBG("X");
    return ret;
}



int CamPathPass2::getTpipePerform( struct CamPathPass2Parameter* p_parameter )
{

    tdriPipe.tdri.tdriPerformCfg.tpipeWidth = MAX_TPIPE_WIDTH;
    tdriPipe.tdri.tdriPerformCfg.tpipeHeight = MAX_TPIPE_HEIGHT;

    // check and set irq mode
    if( p_parameter->en_Top.dma & (CAM_CTL_DMA_EN_WDMAO_EN|CAM_CTL_DMA_EN_WROTO_EN)){
        tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_TPIPE; // for ISP-MDP or ISP-MDP-JPEG
    } else {
        tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_FRAME; // only for ISP only
    }


    ISP_PATH_DBG("tpipeWidth(%d) tpipeHeight(%d) irqMode(%d)\n", \
                tdriPipe.tdri.tdriPerformCfg.tpipeWidth,tdriPipe.tdri.tdriPerformCfg.tpipeHeight, \
                tdriPipe.tdri.tdriPerformCfg.irqMode);

    return MTRUE;
}

int CamPathPass2::getCdpMuxSetting(struct CamPathPass2Parameter pass2Parameter, MINT32 *pDispVidSel)
{
    MINT32 ret = 0;
    MUINT32 wrotoXCropRatio, wrotoYCropRatio, wrotoCropRatio;
    MUINT32 wdmaoXCropRatio, wdmaoYCropRatio, wdmaoCropRatio;
    MUINT32 wrotoAbsRatio, wdmaoAbsRatio;
    MUINT32 xAbsRatio, yAbsRatio;

    MBOOL enVido = CAM_CTL_DMA_EN_WDMAO_EN & pass2Parameter.en_Top.dma;
    MBOOL endispo = CAM_CTL_DMA_EN_WROTO_EN & pass2Parameter.en_Top.dma;
    MBOOL enCdrz = CAM_CTL_EN2_CDRZ_EN & pass2Parameter.en_Top.enable2;
    MBOOL enPrz = CAM_CTL_EN2_PRZ_EN & pass2Parameter.en_Top.enable2;


    *pDispVidSel = -1; // set wrong initial value for debug

    /* check prz, cdrz and curz enable setting */
    if((!enPrz) && (!enCdrz)  ) {
        *pDispVidSel = 0;
        ISP_PATH_WRN("[warning]enPrz(%d) enCdrz(%d\n",enPrz,enCdrz);
        return ret;
    } else if((!enPrz) || (enCdrz) ) {
        ISP_PATH_ERR("[error]enPrz(%d) enCdrz(%d)\n",enPrz,enCdrz);
        FUNCTION_BLOCKING;
    }


    if(enVido && endispo) { // vido:(1), dispo(1)

        /* get and check crop ratio of dispo */
        wdmaoXCropRatio = (pass2Parameter.wdmao.size.w << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.w;
        wdmaoYCropRatio = (pass2Parameter.wdmao.size.h << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.h;
        if(BASIC_SCALE_RATIO <= wdmaoXCropRatio && BASIC_SCALE_RATIO <= wdmaoYCropRatio) { // scale up

            wdmaoCropRatio = (wdmaoXCropRatio >= wdmaoYCropRatio) ? (wdmaoXCropRatio) : (wdmaoYCropRatio);

            wdmaoAbsRatio = wdmaoCropRatio - BASIC_SCALE_RATIO;

        } else if (BASIC_SCALE_RATIO >= wdmaoXCropRatio && BASIC_SCALE_RATIO >= wdmaoYCropRatio) { // scale down

            wdmaoCropRatio = (wdmaoXCropRatio <= wdmaoYCropRatio) ? (wdmaoXCropRatio) : (wdmaoYCropRatio);

            wdmaoAbsRatio = BASIC_SCALE_RATIO - wdmaoCropRatio;

        } else {  // x and y are not the same direction
            if(wdmaoXCropRatio > wdmaoYCropRatio) {  // x(scale up), y(scale down)
                xAbsRatio = wdmaoXCropRatio - BASIC_SCALE_RATIO;
                yAbsRatio = BASIC_SCALE_RATIO - wdmaoYCropRatio;

                if(xAbsRatio >= yAbsRatio) {
                    wdmaoCropRatio = wdmaoXCropRatio;
                    wdmaoAbsRatio = wdmaoCropRatio - BASIC_SCALE_RATIO;
                } else {
                    wdmaoCropRatio = wdmaoYCropRatio;
                    wdmaoAbsRatio = BASIC_SCALE_RATIO - wdmaoCropRatio;
                }
            } else { // x(scale down), y(scale up)
                xAbsRatio = BASIC_SCALE_RATIO - wdmaoXCropRatio;
                yAbsRatio = wdmaoYCropRatio - BASIC_SCALE_RATIO;

                if(xAbsRatio >= yAbsRatio) {
                    wdmaoCropRatio = wdmaoXCropRatio;
                    wdmaoAbsRatio = BASIC_SCALE_RATIO - wdmaoCropRatio;
                } else {
                    wdmaoCropRatio = wdmaoYCropRatio;
                    wdmaoAbsRatio = wdmaoCropRatio - BASIC_SCALE_RATIO;
                }
            }
            //
            ISP_PATH_WRN("[warning]x,y direction is not the same[Imgi]=[%d, %d], [dispo]=[%d, %d]\n",
                    pass2Parameter.imgi.size.w,pass2Parameter.imgi.size.h,pass2Parameter.wdmao.size.w,pass2Parameter.wdmao.size.h);

            ISP_PATH_DBG("[dispo]xAbsRatio(%d) yAbsRatio(%d) AbsRatio(%d) CropRatio(%d)\n",xAbsRatio,yAbsRatio,wdmaoAbsRatio,wdmaoCropRatio);
        }

        ISP_PATH_DBG("[Imgi]=[%d,%d],[Imgi_crop]=[%d,%d,%d,%d],[dispo]=[%d,%d]\n",pass2Parameter.imgi.size.w,pass2Parameter.imgi.size.h,
            pass2Parameter.imgi.crop.x,pass2Parameter.imgi.crop.y,pass2Parameter.imgi.crop.w,pass2Parameter.imgi.crop.h,
            pass2Parameter.wdmao.size.w,pass2Parameter.wdmao.size.h);

        ISP_PATH_DBG("[dispo]cropRatiox(0x%x), cropRatioy(0x%x), cropRatio(0x%x)\n",
                    wdmaoXCropRatio,wdmaoYCropRatio,wdmaoCropRatio);

        /* get and check crop ratio of vido */
        wrotoXCropRatio = (pass2Parameter.wroto.size.w << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.w;
        wrotoYCropRatio = (pass2Parameter.wroto.size.h << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.h;
        if(BASIC_SCALE_RATIO <= wrotoXCropRatio && BASIC_SCALE_RATIO <= wrotoYCropRatio) { // scale up

            wrotoCropRatio = (wrotoXCropRatio >= wrotoYCropRatio) ? (wrotoXCropRatio) : (wrotoYCropRatio);
            wrotoAbsRatio = wrotoCropRatio - BASIC_SCALE_RATIO;

        } else if (BASIC_SCALE_RATIO >= wrotoXCropRatio && BASIC_SCALE_RATIO >= wrotoYCropRatio) { // scale down

            wrotoCropRatio = (wrotoXCropRatio <= wrotoYCropRatio) ? (wrotoXCropRatio) : (wrotoYCropRatio);
            wrotoAbsRatio = BASIC_SCALE_RATIO - wrotoCropRatio;

        } else {
            if(wrotoXCropRatio > wrotoYCropRatio) {  // x(scale up), y(scale down)
                xAbsRatio = wrotoXCropRatio - BASIC_SCALE_RATIO;
                yAbsRatio = BASIC_SCALE_RATIO - wrotoYCropRatio;

                if(xAbsRatio >= yAbsRatio) {
                    wrotoCropRatio = wrotoXCropRatio;
                    wrotoAbsRatio = wrotoCropRatio - BASIC_SCALE_RATIO;
                } else {
                    wrotoCropRatio = wrotoYCropRatio;
                    wrotoAbsRatio = BASIC_SCALE_RATIO - wrotoCropRatio;
                }
            } else { // x(scale down), y(scale up)
                xAbsRatio = BASIC_SCALE_RATIO - wrotoXCropRatio;
                yAbsRatio = wrotoYCropRatio - BASIC_SCALE_RATIO;

                if(xAbsRatio >= yAbsRatio) {
                    wrotoCropRatio = wrotoXCropRatio;
                    wrotoAbsRatio = BASIC_SCALE_RATIO - wrotoCropRatio;
                } else {
                    wrotoCropRatio = wrotoYCropRatio;
                    wrotoAbsRatio = wrotoCropRatio - BASIC_SCALE_RATIO;
                }
            }
            //

            ISP_PATH_WRN("[warning]x,y direction is not the same[ImgiCrop]=[%d, %d],[vido]=[%d, %d]\n",
                    pass2Parameter.imgi.crop.w,pass2Parameter.imgi.crop.h,pass2Parameter.wroto.size.w,pass2Parameter.wroto.size.h);
            ISP_PATH_DBG("[vido]xAbsRatio(%d) yAbsRatio(%d) AbsRatio(%d) CropRatio(%d)\n",xAbsRatio,yAbsRatio,wrotoAbsRatio,wrotoCropRatio);
        }

        ISP_PATH_DBG("[Imgi]=[%d,%d],[Imgi_crop]=[%d,%d,%d,%d],[vido]=[%d,%d]\n",pass2Parameter.imgi.size.w,pass2Parameter.imgi.size.h,
            pass2Parameter.imgi.crop.x,pass2Parameter.imgi.crop.y,pass2Parameter.imgi.crop.w,pass2Parameter.imgi.crop.h,
            pass2Parameter.wroto.size.w,pass2Parameter.wroto.size.h);

        ISP_PATH_DBG("[vido]cropRatiox(0x%x), cropRatioy(0x%x), cropRatio(0x%x)\n",
                    wrotoXCropRatio,wrotoYCropRatio,wrotoCropRatio);

        /* check scale ratio first due to CURZ & CDRZ limitation */
        if(enCdrz) {
            if(wdmaoAbsRatio>=wrotoAbsRatio && wdmaoCropRatio>=PRZ_MIN_SCALE_RATIO && wdmaoCropRatio<=PRZ_MAX_SCALE_RATIO
                            && wrotoCropRatio>=CDRZ_MIN_SCALE_RATIO && wrotoCropRatio<=CDRZ_MAX_SCALE_RATIO) {
                *pDispVidSel = CAM_CDP_PRZ_CONN_TO_DISPO; // source ratio is approch 1
            }else if(wdmaoAbsRatio<wrotoAbsRatio && wdmaoCropRatio>=CDRZ_MIN_SCALE_RATIO && wdmaoCropRatio<=CDRZ_MAX_SCALE_RATIO
                                && wrotoCropRatio>=PRZ_MIN_SCALE_RATIO && wrotoCropRatio<=PRZ_MAX_SCALE_RATIO) {
                *pDispVidSel = CAM_CDP_PRZ_CONN_TO_VIDO; // source ratio is approch 1
            }else {
                ISP_PATH_ERR("[Error]scale ratio(2) out of range wdmaoAbsRatio(0x%x), wrotoAbsRatio(0x%x)\n",wdmaoAbsRatio,wrotoAbsRatio);
                FUNCTION_BLOCKING;
            }
        } else {
            ISP_PATH_ERR("[ERROR] enCdrz(%d)\n",enCdrz);
            FUNCTION_BLOCKING;
        }
    } else if(enVido==0 && endispo) { // vido:(0), dispo(1)
        *pDispVidSel = CAM_CDP_PRZ_CONN_TO_DISPO;

    } else if(endispo==0 && enVido) { // vido:(1), dispo(0)
        *pDispVidSel = CAM_CDP_PRZ_CONN_TO_VIDO;

    } else {
        ISP_PATH_ERR("[ERROR]en_Top.dma(0x%x)\n",pass2Parameter.en_Top.dma);
        FUNCTION_BLOCKING;
    }


    if(*pDispVidSel!=CAM_CDP_PRZ_CONN_TO_DISPO && *pDispVidSel!=CAM_CDP_PRZ_CONN_TO_VIDO) {
        ISP_PATH_ERR("[ERROR]*pDispVidSel(%d)\n",*pDispVidSel);
        FUNCTION_BLOCKING;
    }


    return ret;
}



int CamPathPass2::configTpipeData( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;
    CAM_REG_CTL_EN1 *pEn1;
    CAM_REG_CTL_EN2 *pEn2;
    CAM_REG_CTL_DMA_EN *pEnDMA;

    pEn1 = (CAM_REG_CTL_EN1*)&(p_parameter->en_Top.enable1);
    pEn2 = (CAM_REG_CTL_EN2*)&(p_parameter->en_Top.enable2);
    pEnDMA = (CAM_REG_CTL_DMA_EN*)&(p_parameter->en_Top.dma);
    tdriPipe.enTdri = p_parameter->tpipe;

    /* config top_en */
    if(p_parameter->isConcurrency &&
            (CAM_CTL_EN2_CDRZ_EN & p_parameter->en_Top.enable2) == 0x00) {
        /* Due to hardware flexible setting for scenario and mode,
           so we need a corrected setting for tpipe main (only for cdp yuv input) */
        tdriPipe.tdri.top.scenario = 2; // ZSD:2
        tdriPipe.tdri.top.mode = 1; // ZSD_YUV:1
        ISP_PATH_WRN("[Warning]run scenario ZSD tpipe to repace scenario VSS");
    } else {
        tdriPipe.tdri.top.scenario = p_parameter->fmt_sel.bit_field.scenario;
        tdriPipe.tdri.top.mode = p_parameter->p2SubMode;
    }


   // tdriPipe.tdri.top.debug_sel = p_parameter->ctl_sel.bit_field.dbg_sel;
    tdriPipe.tdri.top.pixel_id = p_parameter->pix_id;
    tdriPipe.tdri.top.cam_in_fmt = p_parameter->fmt_sel.bit_field.cam_in_fmt;

    tdriPipe.tdri.top.tcm_load_en = (p_parameter->tcm_en&CAM_CTL_TCM_CTL_EN)?(1):(0);
    tdriPipe.tdri.top.ctl_extension_en = p_parameter->ctl_pix_id.bit_field.CTL_EXTENSION_EN;
    tdriPipe.tdri.top.rsp_en = (p_parameter->tcm_en&CAM_CTL_TCM_RSP_EN)?(1):(0);
    tdriPipe.tdri.top.mdp_crop_en = p_parameter->isMdpCropEn;
    tdriPipe.tdri.top.mdp_sel = p_parameter->ctl_pix_id.bit_field.MDP_SEL;
    tdriPipe.tdri.top.interlace_mode = 0; // set to 0


    ISP_PATH_DBG("[Top]scenario(%d) mode(%d) debug_sel(%d) pixel_id(%d) cam_in_fmt(%d)",
        tdriPipe.tdri.top.scenario,tdriPipe.tdri.top.mode,tdriPipe.tdri.top.debug_sel,
        tdriPipe.tdri.top.pixel_id,tdriPipe.tdri.top.cam_in_fmt);

    tdriPipe.tdri.top.imgi_en = pEnDMA->Bits.IMGI_EN;
    tdriPipe.tdri.top.lsci_en = pEnDMA->Bits.LSCI_EN;
    tdriPipe.tdri.top.unp_en = pEn1->Bits.UNP_EN;
    tdriPipe.tdri.top.bnr_en = pEn1->Bits.BNR_EN;
    tdriPipe.tdri.top.lsc_en = pEn1->Bits.LSC_EN;
    tdriPipe.tdri.top.sl2_en = pEn1->Bits.SL2_EN;
    tdriPipe.tdri.top.c24_en = pEn1->Bits.C24_EN;
    tdriPipe.tdri.top.cfa_en = pEn1->Bits.CFA_EN;
    tdriPipe.tdri.top.c42_en = pEn2->Bits.C42_EN;
    tdriPipe.tdri.top.nbc_en = pEn2->Bits.NBC_EN;
    tdriPipe.tdri.top.seee_en = pEn2->Bits.SEEE_EN;
    tdriPipe.tdri.top.imgo_en = pEnDMA->Bits.IMGO_EN;
    tdriPipe.tdri.top.img2o_en = pEnDMA->Bits.IMG2O_EN;
    tdriPipe.tdri.top.cdrz_en = pEn2->Bits.CDRZ_EN;

    ISP_PATH_DBG("cfa(%d)",tdriPipe.tdri.top.cfa_en);

    /* config tpipe setting */
    //imgi
    tdriPipe.tdri.imgi.stride = p_parameter->imgi.size.stride;
    //imgo
    //::memcpy( (char*)&tdriPipe.tdri.imgo, (char*)&p_parameter->imgo, sizeof(IspImgoCfg)); /* IMGO */
    tdriPipe.tdri.imgo.imgo_stride=p_parameter->imgo.size.stride;  ///< imgo stride
    tdriPipe.tdri.imgo.imgo_crop_en=0; ///< imgo crop enable
    tdriPipe.tdri.imgo.imgo_xoffset=0; ///< imgo offset in x direction
    tdriPipe.tdri.imgo.imgo_yoffset=0; ///< imgo offset in y direction
    tdriPipe.tdri.imgo.imgo_xsize=p_parameter->imgo.size.xsize-1;   ///< imgo x size
    tdriPipe.tdri.imgo.imgo_ysize=p_parameter->imgo.size.h-1;   ///< imgo y size
    tdriPipe.tdri.imgo.imgo_mux_en=p_parameter->imgo_mux_en;  ///< imgo mux enable
    tdriPipe.tdri.imgo.imgo_mux=p_parameter->imgo_mux;     ///< imgo mux
    //cdrz
    ::memcpy( (char*)&tdriPipe.tdri.cdrz, (char*)&p_parameter->cdrz, sizeof(IspCdrzCfg)); /* CDRZ */
    //img2o
    //::memcpy( (char*)&tdriPipe.tdri.img2o, (char*)&p_parameter->img2o, sizeof(IspImg2oCfg)); /* IMG2O */
    tdriPipe.tdri.img2o.img2o_stride=p_parameter->img2o.size.stride;  ///< img2o stride
    tdriPipe.tdri.img2o.img2o_crop_en=0; ///< img2o crop enable
    tdriPipe.tdri.img2o.img2o_xoffset=0; ///< img2o offset in x direction
    tdriPipe.tdri.img2o.img2o_yoffset=0; ///< img2o offset in y direction
    tdriPipe.tdri.img2o.img2o_xsize=p_parameter->img2o.size.xsize-1;   ///< img2o x size
    tdriPipe.tdri.img2o.img2o_ysize=p_parameter->img2o.size.h-1;   ///< img2o y size
    tdriPipe.tdri.img2o.img2o_mux_en=p_parameter->img2o_mux_en;  ///< img2o mux enable
    tdriPipe.tdri.img2o.img2o_mux=p_parameter->img2o_mux;     ///< img2o mux

    if(p_parameter->isV3 == MTRUE && p_parameter->isApplyTuning==MTRUE){
        //bnr
        tdriPipe.tdri.tuningFunc.bnr.bpc_en = p_parameter->pTuningIspReg->CAM_BPC_CON.Bits.BPC_ENABLE;
        //lsc
        tdriPipe.tdri.tuningFunc.lsc.sdblk_width = p_parameter->pTuningIspReg->CAM_LSC_CTL2.Bits.SDBLK_WIDTH;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_xnum = p_parameter->pTuningIspReg->CAM_LSC_CTL2.Bits.SDBLK_XNUM;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_last_width = p_parameter->pTuningIspReg->CAM_LSC_LBLOCK.Bits.SDBLK_lWIDTH;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_height = p_parameter->pTuningIspReg->CAM_LSC_CTL3.Bits.SDBLK_HEIGHT;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_ynum = p_parameter->pTuningIspReg->CAM_LSC_CTL3.Bits.SDBLK_YNUM;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_last_height = p_parameter->pTuningIspReg->CAM_LSC_LBLOCK.Bits.SDBLK_lHEIGHT;
        //lsci
        tdriPipe.tdri.tuningFunc.lsci.lsci_stride = p_parameter->pTuningIspReg->CAM_LSCI_STRIDE.Bits.STRIDE;
        //nbc
        tdriPipe.tdri.tuningFunc.nbc.anr_eny = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_ENY;
        tdriPipe.tdri.tuningFunc.nbc.anr_enc = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_ENC;
        tdriPipe.tdri.tuningFunc.nbc.anr_iir_mode = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_IIR_MODE;
        tdriPipe.tdri.tuningFunc.nbc.anr_scale_mode = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_SCALE_MODE;
        //seee
        tdriPipe.tdri.tuningFunc.seee.se_edge = p_parameter->pTuningIspReg->CAM_SEEE_EDGE_CTRL.Bits.SE_EDGE;
        //cfa
        tdriPipe.tdri.tuningFunc.cfa.bayer_bypass = p_parameter->pTuningIspReg->CAM_CFA_BYPASS.Bits.BAYER_BYPASS;
        //sl2
        tdriPipe.tdri.tuningFunc.sl2.sl2_hrz_comp = p_parameter->pTuningIspReg->CAM_SL2_HRZ_COMP.Bits.SL2_HRZ_COMP;
    }
    else
   {    //avoid pass2 fail when bypass tuning in cam3 ut code, cuz tpipe should refer some module setting from cfa, arn,...
        tdriPipe.tdri.tuningFunc.bnr.bpc_en = 0;
        //lsc
        tdriPipe.tdri.tuningFunc.lsc.sdblk_width = 0;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_xnum = 0;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_last_width = 0;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_height = 0;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_ynum = 0;
        tdriPipe.tdri.tuningFunc.lsc.sdblk_last_height = 0;
        //lsci
        tdriPipe.tdri.tuningFunc.lsci.lsci_stride = 0;
        //nbc
        tdriPipe.tdri.tuningFunc.nbc.anr_eny = 0;
        tdriPipe.tdri.tuningFunc.nbc.anr_enc = 0;
        tdriPipe.tdri.tuningFunc.nbc.anr_iir_mode = 0;
        tdriPipe.tdri.tuningFunc.nbc.anr_scale_mode = 0;
        //seee
        tdriPipe.tdri.tuningFunc.seee.se_edge = 0;
        //cfa
        tdriPipe.tdri.tuningFunc.cfa.bayer_bypass =0;
        //sl2
        tdriPipe.tdri.tuningFunc.sl2.sl2_hrz_comp = 0;
   }

    /* software tpipe setting */
    tdriPipe.tdri.tdriCfg.srcWidth = p_parameter->imgi.size.w;
    tdriPipe.tdri.tdriCfg.srcHeight = p_parameter->imgi.size.h;
    tdriPipe.tdri.tdriCfg.baseVa = p_parameter->tdri.memBuf.base_vAddr;

    // get tpipe perform information
    getTpipePerform(p_parameter);


    ISP_PATH_DBG("[Tdri]srcWidth(%d) srcHeight(%d)",tdriPipe.tdri.tdriCfg.srcWidth,tdriPipe.tdri.tdriCfg.srcHeight);

EXIT:
    return ret;

}

int CamPathPass2::setZoom( MUINT32 zoomRatio )
{
int ret = 0;

    ISP_PATH_DBG(":E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);


    /*function List*/
     m_isp_function_list[0 ] = (IspFunction_B*)&cdpPipe;
     m_isp_function_list[1 ] = (IspFunction_B*)&DMAImgi;
     m_isp_function_list[2 ] = (IspFunction_B*)&DMAVipi;
     m_isp_function_list[3 ] = (IspFunction_B*)&DMAVip2i;
     m_isp_function_list[4 ] = (IspFunction_B*)&DMATdri;
    m_isp_function_count=5;

    //cdp
    cdpPipe.CQ = NSIspDrv_FrmB::CAM_ISP_CQ1;

    this->_setZoom(NULL);

    ISP_PATH_DBG(":X");

    return ret;

}

int CamPathPass2::_waitIrq( int type, unsigned int irq, int irqclear, int userNumber )
{
    int ret = 0;
    return ret;
}

int CamPathPass2::_waitIrq( int type, unsigned int irq, int userNumber )
{
    int ret = 0;
    return ret;
}


int CamPathPass2::_waitIrq(int type, unsigned int irq )
{
int ret = 0;
NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    ISP_PATH_DBG(":E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    WaitIrq.Clear = NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_NONE;
    WaitIrq.UserInfo.Type = (NSIspDrv_FrmB::ISP_DRV_IRQ_TYPE_ENUM)type;//ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.UserInfo.Status = irq;//ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    WaitIrq.Timeout = CAM_INT_PASS2_WAIT_TIMEOUT_MS;//ms ,0 means pass through.
#if 0 //SL skip this irQ to wait MDP done by dequeueDstBuffer
    if (0 == ispTopCtrl.waitIrq(WaitIrq) ) {
        ISP_PATH_ERR("waitIrq fail");
        ret = -1;
    }
#else
    ISP_PATH_DBG("No Pass2 _waitIrq()\n");
    ISP_PATH_DBG("return true, immediately\n");
    ISP_PATH_DBG("Let dequeueDstBuffer() to wait\n");

#endif
    ISP_PATH_DBG(":X");

    return ret;
}

EIspRetStatus CamPathPass2::dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo, MINT32 const eDrvSce, MINT32 dequeCq,MINT32 dequeDupCqIdx, MUINT32 dequeBurstQueIdx)
{
    EIspRetStatus ret;

    ISP_PATH_DBG("+,dequeCq(%d),dequeBurstQueIdx(%d)",dequeCq,dequeBurstQueIdx);

//    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2Lock);
    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2DequeLock);

    this->ispBufCtrl.dequeCq = dequeCq;
    this->ispBufCtrl.dequeDupCqIdx = dequeDupCqIdx;
    this->ispBufCtrl.dequeBurstQueIdx = dequeBurstQueIdx;
    this->ispBufCtrl.drvScenario = eDrvSce;

    //move FILLED buffer from hw to sw list
    ret = this->ispBufCtrl.dequeueHwBuf( dmaChannel, bufInfo );

    ISP_PATH_DBG("-,");
    return ret;
}
//
int CamPathPass2::dequeueMdpFrameEnd(MINT32 const eDrvSce)
{
    int ret = 0;
    ISP_PATH_DBG("+,");

        if ( 0!= this->ispBufCtrl.dequeueMdpFrameEnd(eDrvSce) ) {
            ISP_PATH_ERR("ERROR:dequeueMdpFrameEnd");
            return -1;
    }
    ISP_PATH_DBG("-,");

    return ret;
}


