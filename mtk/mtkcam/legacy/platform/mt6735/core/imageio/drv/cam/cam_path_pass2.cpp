#define LOG_TAG "iio/pathp2"
//
//
#include "cam_path.h"
#include <utils/Trace.h> //for systrace

//

/*******************************************************************************
*
********************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(path);
EXTERN_DBG_LOG_VARIABLE(path);


// the size only affect tpipe table
#define MAX_TPIPE_WIDTH                  (768)
#define MAX_TPIPE_HEIGHT                 (8192)
#define CONCURRENCY_RING_TPIPE_HEIGHT    (384)
#define CONCURRENCY_CAP_TPIPE_HEIGHT     (480)


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
#define FUNCTION_BLOCKING   do{ISP_PATH_ERR("[Error]blocking ");}while(1);
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
    MUINT32 vidoXCropRatio, vidoYCropRatio, vidoCropRatio;
    MUINT32 dispoXCropRatio, dispoYCropRatio, dispoCropRatio;
    MINT32  p2SubMode;

    ISP_TRACE_CALL();

    ISP_PATH_INF("+,CamPathPass2:tdri(%d),dupCqIdx(%d),burstQueIdx(%d)",p_parameter->tpipe,p_parameter->dupCqIdx,p_parameter->burstQueIdx);

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2Lock);

    /*function List*/
    cnt = 0;
    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMACQ;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispTuningCtrl;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispTopCtrl;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispRawPipe;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispRgbPipe;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispYuvPipe;
        m_isp_function_list[cnt++] = (IspFunction_B*)&crzPipe;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImgi;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAUfdi;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMALcei;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAVipi;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAVip2i;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAVip3i;
        m_isp_function_list[cnt++] = (IspFunction_B*)&tdriPipe;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImgo;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAMfbo;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImg2o;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImg3o;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImg3bo;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImg3co;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAFEO;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispMdpPipe;
        m_isp_function_count = cnt;
    } else { // frame mode (do not use MDP)
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMACQ;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispTuningCtrl;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispTopCtrl;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispRawPipe;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImgi;
        m_isp_function_list[cnt++] = (IspFunction_B*)&DMAImgo;
        m_isp_function_list[cnt++] = (IspFunction_B*)&tdriPipe;
        m_isp_function_list[cnt++] = (IspFunction_B*)&ispMdpPipe;
        m_isp_function_count = cnt;
    }

    //update scenario fro p2(p2 should not revise sub_mode while p1 is running)
    //cuz tpipe operation would ref sub_mode, we do the following sets
    //(1)set fake submode value that may not match with hw to tpipe calculation
    //(2)set the submode value that match with the value p1 set if needed
    //part 1
    p2SubMode = p_parameter->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE;
    //part 2
    if(ispTopCtrl.m_pIspDrvShell->getPhyIspDrv())
    {
        MUINT32 regScen=0xa5a5a5a5;
        bool ret=false;
        ret=ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->queryScenarioValue(regScen);
        if(!ret)
        {
            ISP_PATH_ERR("querySce ERr(0x%8x)",regScen);
        }
        else
        {
            if(regScen==0xa5a5a5a5)    //default value, means no one update it
            {
                ISP_PATH_INF("no pass1 update regScen(0x%8x)",regScen);
            }
            else
            {
                MUINT32 scen=regScen & 0x7;
                MUINT32 submo=(regScen & 0x70) >> 4;
    switch(scen)
    {
        case 6: //cc
        default:
            p_parameter->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE=submo;
            break;
        case 0: //not set yet
        case 3: //ip
            break;
    }
            }
        }
      }
    //

    //top
    switch(p_parameter->CQ){
        case CAM_ISP_CQ2:
            ispTopCtrl.path = ISP_PASS2B;
            ispRawPipe.path = ISP_PASS2B;
            break;
        case CAM_ISP_CQ3:
            ispTopCtrl.path = ISP_PASS2C;
            ispRawPipe.path = ISP_PASS2C;
            break;
        case CAM_ISP_CQ1:
        default:
            ispTopCtrl.path = ISP_PASS2;
            ispRawPipe.path = ISP_PASS2;
            break;
    }
    // C++11 do not allow this kind of operation, use memcpy for workaround
    //ispTopCtrl.isp_top_ctl= p_parameter->isp_top_ctl;
    memcpy((void *) &ispTopCtrl.isp_top_ctl, (void *) &p_parameter->isp_top_ctl, sizeof(struct stIspTopCtl));
    //
    ispTopCtrl.magicNum = p_parameter->magicNum;
    ispTopCtrl.dupCqIdx = p_parameter->dupCqIdx;
    ispTopCtrl.burstQueIdx = p_parameter->burstQueIdx;
    ispTopCtrl.sub_mode = p2SubMode;
    ispTopCtrl.drvScenario = p_parameter->drvScenario;
    ispTopCtrl.isV3 = p_parameter->isV3;
    ispTopCtrl.isApplyTuning = p_parameter->isApplyTuning;
    //
    ispBufCtrl.drvScenario = p_parameter->drvScenario;
    //

//    p_parameter->pix_id = ispTopCtrl.pix_id; /* get pix_id from pass1 */
    //tuning function
    ispTuningCtrl.bypass = 0;

    //isp_raw
    ispRawPipe.bypass = p_parameter->bypass_ispRawPipe;
    ispRawPipe.magicNum = p_parameter->magicNum;
    ispRawPipe.dupCqIdx = p_parameter->dupCqIdx;
    ispRawPipe.burstQueIdx = p_parameter->burstQueIdx;
    ispRawPipe.drvScenario = p_parameter->drvScenario;
    ispRawPipe.sub_mode = p2SubMode;
    ispRawPipe.en_p1 = p_parameter->isp_top_ctl.CAM_CTL_EN_P1.Raw;  // for ip scenario
    ispRawPipe.en_p2 = p_parameter->isp_top_ctl.CAM_CTL_EN_P2.Raw;
    ispRawPipe.img_sel = 0;
    ispRawPipe.src_img_w = p_parameter->src_img_size.w;
    ispRawPipe.src_img_h = p_parameter->src_img_size.h;
    ispRawPipe.isV3 = p_parameter->isV3;
    ispRawPipe.isApplyTuning = p_parameter->isApplyTuning;
    ispRawPipe.pTuningIspReg = p_parameter->pTuningIspReg;

    //isp_rgb
    ispRgbPipe.bypass = p_parameter->bypass_ispRgbPipe;
    ispRgbPipe.magicNum = p_parameter->magicNum;
    ispRgbPipe.dupCqIdx = p_parameter->dupCqIdx;
    ispRgbPipe.burstQueIdx = p_parameter->burstQueIdx;
    ispRgbPipe.drvScenario = p_parameter->drvScenario;
    ispRgbPipe.sub_mode = p2SubMode;
    ispRgbPipe.en_p2 = p_parameter->isp_top_ctl.CAM_CTL_EN_P2.Raw;
    ispRgbPipe.rrzRatio = p_parameter->rrzRatio;
    ispRgbPipe.rrzCropX = p_parameter->rrzCropX;
    ispRgbPipe.rrzCropY = p_parameter->rrzCropY;
    ispRgbPipe.enFgMode = p_parameter->isp_top_ctl.CAM_CTL_FMT_SEL_P2.Bits.FG_MODE;
    ispRgbPipe.isV3 = p_parameter->isV3;
    ispRgbPipe.isApplyTuning = p_parameter->isApplyTuning;
    ispRgbPipe.pTuningIspReg = p_parameter->pTuningIspReg;

    if(CAM_CTL_EN_P2_MFB_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2.Raw)
    {
        ispRgbPipe.mfb_cfg=p_parameter->mfb_cfg;
    }
    //isp_yuv
    ispYuvPipe.bypass = p_parameter->bypass_ispYuvPipe;
    ispYuvPipe.magicNum = p_parameter->magicNum;
    ispYuvPipe.dupCqIdx = p_parameter->dupCqIdx;
    ispYuvPipe.burstQueIdx = p_parameter->burstQueIdx;
    ispYuvPipe.drvScenario = p_parameter->drvScenario;
    ispYuvPipe.sub_mode = p2SubMode;
    ispYuvPipe.en_p2 = p_parameter->isp_top_ctl.CAM_CTL_EN_P2.Raw;
    ispYuvPipe.rrzRatio = p_parameter->rrzRatio;
    ispYuvPipe.rrzCropX = p_parameter->rrzCropX;
    ispYuvPipe.rrzCropY = p_parameter->rrzCropY;
    ispYuvPipe.isV3 = p_parameter->isV3;
    ispYuvPipe.isApplyTuning = p_parameter->isApplyTuning;
    ispYuvPipe.pTuningIspReg = p_parameter->pTuningIspReg;

    if(CAM_CTL_EN_P2_CRSP_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2.Raw)
    {
        ispYuvPipe.crsp_cfg.out.w=p_parameter->img3o.size.w;
        ispYuvPipe.crsp_cfg.out.h=p_parameter->img3o.size.h;
    }
    if(CAM_CTL_EN_P2_SRZ1_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2.Raw)
    {
        p_parameter->srz1_cfg.h_step=( (p_parameter->srz1_cfg.inout_size.in_w-p_parameter->srz1_cfg.crop.x- (p_parameter->srz1_cfg.crop.floatX==0?0:1) -1)*\
                32768+((p_parameter->srz1_cfg.inout_size.out_w-1)>>1) )/(p_parameter->srz1_cfg.inout_size.out_w-1);
        p_parameter->srz1_cfg.v_step=( (p_parameter->srz1_cfg.inout_size.in_h-p_parameter->srz1_cfg.crop.y- (p_parameter->srz1_cfg.crop.floatY==0?0:1) -1)*\
                32768+((p_parameter->srz1_cfg.inout_size.out_h-1)>>1) )/(p_parameter->srz1_cfg.inout_size.out_h-1);
        ispYuvPipe.srz1_cfg=p_parameter->srz1_cfg;
    }
    if(CAM_CTL_EN_P2_SRZ2_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2.Raw)
    {
        p_parameter->srz2_cfg.h_step=( (p_parameter->srz2_cfg.inout_size.in_w-p_parameter->srz2_cfg.crop.x- (p_parameter->srz2_cfg.crop.floatX==0?0:1) -1)*\
                32768+((p_parameter->srz2_cfg.inout_size.out_w-1)>>1) )/(p_parameter->srz2_cfg.inout_size.out_w-1);
        p_parameter->srz2_cfg.v_step=( (p_parameter->srz2_cfg.inout_size.in_h-p_parameter->srz2_cfg.crop.y- (p_parameter->srz2_cfg.crop.floatY==0?0:1) -1)*\
                32768+((p_parameter->srz2_cfg.inout_size.out_h-1)>>1) )/(p_parameter->srz2_cfg.inout_size.out_h-1);
        ispYuvPipe.srz2_cfg=p_parameter->srz2_cfg;
    }
    if(CAM_CTL_EN_P2_C02_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2.Raw)
    {
        ispYuvPipe.c02_cfg.in.w=p_parameter->vipi.size.w;
        ispYuvPipe.c02_cfg.in.h=p_parameter->vipi.size.h;
    }
    if(CAM_CTL_EN_P2_FE_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2.Raw)
    {
        switch(p_parameter->fe.fem_harris_tpipe_mode)
        {
            case 1:
                ispYuvPipe.fe_ctrl = 0xAD;
                break;
            case 2:
                ispYuvPipe.fe_ctrl = 0xAE;
                break;
            default:
                ispYuvPipe.fe_ctrl = 0xAC;
                break;
        }
    }

//////kk test
    ispYuvPipe.nr3dCfg.fbcntOff_xoff = 0;
    ispYuvPipe.nr3dCfg.fbcntOff_yoff = 0;
    ispYuvPipe.nr3dCfg.fbcntSiz_xsiz = 0;
    ispYuvPipe.nr3dCfg.fbcntSiz_ysiz = 0;
    ispYuvPipe.nr3dCfg.fbCount_cnt = 0;
    ispYuvPipe.nr3dCfg.ctrl_lmtOutCntTh = p_parameter->nr3d_cfg.ctrl_lmtOutCntTh;
    ispYuvPipe.nr3dCfg.ctrl_onEn = p_parameter->nr3d_cfg.ctrl_onEn;
    ispYuvPipe.nr3dCfg.onOff_onOfStX = p_parameter->nr3d_cfg.onOff_onOfStX;
    ispYuvPipe.nr3dCfg.onOff_onOfStY = p_parameter->nr3d_cfg.onOff_onOfStY;
    ispYuvPipe.nr3dCfg.onSiz_onWd = p_parameter->nr3d_cfg.onSiz_onWd;
    ispYuvPipe.nr3dCfg.onSiz_onHt = p_parameter->nr3d_cfg.onSiz_onHt;
    ispYuvPipe.nr3dCfg.gain_weighting = p_parameter->nr3d_cfg.gain_weighting;
//////kk test


    /* config dupCqIdx & burstQueIdx */
    DMACQ.dupCqIdx = p_parameter->dupCqIdx;
    DMAImgi.dupCqIdx = p_parameter->dupCqIdx;
    DMAUfdi.dupCqIdx = p_parameter->dupCqIdx;
    DMALcei.dupCqIdx = p_parameter->dupCqIdx;
    DMAVipi.dupCqIdx = p_parameter->dupCqIdx;
    DMAVip2i.dupCqIdx = p_parameter->dupCqIdx;
    DMAVip3i.dupCqIdx = p_parameter->dupCqIdx;
    DMATdri.dupCqIdx = p_parameter->dupCqIdx;
    DMAImgo.dupCqIdx = p_parameter->dupCqIdx;
    DMAMfbo.dupCqIdx = p_parameter->dupCqIdx;
    DMAImg2o.dupCqIdx = p_parameter->dupCqIdx;
    DMAImg3o.dupCqIdx = p_parameter->dupCqIdx;
    DMAImg3bo.dupCqIdx = p_parameter->dupCqIdx;
    DMAImg3co.dupCqIdx = p_parameter->dupCqIdx;
    DMAFEO.dupCqIdx = p_parameter->dupCqIdx;
    //
    DMACQ.burstQueIdx = p_parameter->burstQueIdx;
    DMAImgi.burstQueIdx = p_parameter->burstQueIdx;
    DMAUfdi.burstQueIdx = p_parameter->burstQueIdx;
    DMALcei.burstQueIdx = p_parameter->burstQueIdx;
    DMAVipi.burstQueIdx = p_parameter->burstQueIdx;
    DMAVip2i.burstQueIdx = p_parameter->burstQueIdx;
    DMAVip3i.burstQueIdx = p_parameter->burstQueIdx;
    DMATdri.burstQueIdx = p_parameter->burstQueIdx;
    DMAImgo.burstQueIdx = p_parameter->burstQueIdx;
    DMAMfbo.burstQueIdx = p_parameter->burstQueIdx;
    DMAImg2o.burstQueIdx = p_parameter->burstQueIdx;
    DMAImg3o.burstQueIdx = p_parameter->burstQueIdx;
    DMAImg3bo.burstQueIdx = p_parameter->burstQueIdx;
    DMAImg3co.burstQueIdx = p_parameter->burstQueIdx;
    DMAFEO.burstQueIdx = p_parameter->burstQueIdx;

    ISP_PATH_INF("cq(%d),drvScenario(%d),hwScenario(%d),sub_mode(%d) p2_en(0x%08x),p2_dma(0x%08x),burstQueIdx(%d)", \
            p_parameter->CQ,p_parameter->drvScenario,p_parameter->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO,p_parameter->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE, \
            p_parameter->isp_top_ctl.CAM_CTL_EN_P2.Raw, p_parameter->isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw, \
            p_parameter->burstQueIdx);



    //
//    cdpPipe.conf_cdrz = 0;
    //
    ISP_PATH_DBG("[mdpcrop] imgi in[%d, %d] mdpCrop[%d, %d, %d, %d]_f(0x%x, 0x%x)",p_parameter->imgi.size.w,p_parameter->imgi.size.h, \
                p_parameter->croppara.mdp_crop.x,p_parameter->croppara.mdp_crop.y,p_parameter->croppara.mdp_crop.w,p_parameter->croppara.mdp_crop.h, \
                p_parameter->croppara.mdp_crop.floatX,p_parameter->croppara.mdp_crop.floatY);
    //Check the parameter is correct or not.
#if 0
    if ((CAM_CTL_EN_P2_DMA_IMG2O_EN | CAM_CTL_EN_P2_DMA_IMG3O_EN) & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw)
    {
        //
        if ((p_parameter->img2o.crop.x != p_parameter->img3o.crop.x) ||
            (p_parameter->img2o.crop.y != p_parameter->img3o.crop.y) ||
            (p_parameter->img2o.crop.w != p_parameter->img3o.crop.w) ||
            (p_parameter->img2o.crop.h != p_parameter->img3o.crop.h) ||
            (p_parameter->img2o.size.w != p_parameter->img3o.size.w) ||
            (p_parameter->img2o.size.h != p_parameter->img3o.size.h))
        {
            ISP_PATH_DBG("img2o_crop.x(%u),img3o_crop.x(%u) ",p_parameter->img2o.crop.x,p_parameter->img3o.crop.x);
            ISP_PATH_DBG("img2o_crop.y(%u),img3o_crop.y(%u) ",p_parameter->img2o.crop.y,p_parameter->img3o.crop.y);
            ISP_PATH_DBG("img2o_crop.w(%u),img3o_crop.w(%u) ",p_parameter->img2o.crop.w,p_parameter->img3o.crop.w);
            ISP_PATH_DBG("img2o_crop.h(%u),img3o_crop.h(%u) ",p_parameter->img2o.crop.h,p_parameter->img3o.crop.h);
            ISP_PATH_DBG("img2o_size.w(%u),img3o_size.w(%u) ",p_parameter->img2o.size.w,p_parameter->img3o.size.w);
            ISP_PATH_DBG("img2o_size.h(%u),img3o_size.h(%u) ",p_parameter->img2o.size.h,p_parameter->img3o.size.h);
        }
    }


    if ( CAM_CTL_EN_P2_DMA_IMG2O_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw){
        //
        crzPipe.conf_cdrz = 1;
        crzPipe.crz_out = p_parameter->img2o.size;
        crzPipe.crz_in = p_parameter->imgi.size;
        crzPipe.crz_crop.x = p_parameter->img2o.crop.x;
        crzPipe.crz_crop.floatX = p_parameter->img2o.crop.floatX;
        crzPipe.crz_crop.y = p_parameter->img2o.crop.y;
        crzPipe.crz_crop.floatY = p_parameter->img2o.crop.floatY;
        crzPipe.crz_crop.w = p_parameter->img2o.crop.w;
        crzPipe.crz_crop.h = p_parameter->img2o.crop.h;
    }
    else if ( CAM_CTL_EN_P2_DMA_IMG3O_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw){
        crzPipe.conf_cdrz = 1;
        crzPipe.crz_out = p_parameter->img3o.size;
        crzPipe.crz_in = p_parameter->imgi.size;
        crzPipe.crz_crop.x = p_parameter->img3o.crop.x;
        crzPipe.crz_crop.floatX = p_parameter->img3o.crop.floatX;
        crzPipe.crz_crop.y = p_parameter->img3o.crop.y;
        crzPipe.crz_crop.floatY = p_parameter->img3o.crop.floatY;
        crzPipe.crz_crop.w = p_parameter->img3o.crop.w;
        crzPipe.crz_crop.h = p_parameter->img3o.crop.h;
    }
    crzPipe.dupCqIdx = p_parameter->dupCqIdx;
    crzPipe.burstQueIdx = p_parameter->burstQueIdx;
#endif

    if (MTRUE == p_parameter->croppara.crz_enable)
    {
        crzPipe.conf_cdrz = 1;
        crzPipe.crz_out = p_parameter->croppara.crz_out;
        crzPipe.crz_in = p_parameter->croppara.crz_in;
        crzPipe.crz_crop.x = p_parameter->croppara.crz_crop.x;
        crzPipe.crz_crop.floatX = p_parameter->croppara.crz_crop.floatX;
        crzPipe.crz_crop.y = p_parameter->croppara.crz_crop.y;
        crzPipe.crz_crop.floatY = p_parameter->croppara.crz_crop.floatY;
        crzPipe.crz_crop.w = p_parameter->croppara.crz_crop.w;
        crzPipe.crz_crop.h = p_parameter->croppara.crz_crop.h;
        crzPipe.dupCqIdx = p_parameter->dupCqIdx;
        crzPipe.burstQueIdx = p_parameter->burstQueIdx;
        crzPipe.isV3 = p_parameter->isV3;
        crzPipe.isApplyTuning = p_parameter->isApplyTuning;
        //
        // for sl2c tuning
        if(p_parameter->isSl2cAheadCrz==MTRUE){
            ispYuvPipe.sl2cCrzRatio = 1;
        } else {
            // use height to avoid alignment issue
            ispYuvPipe.sl2cCrzRatio = (MFLOAT)crzPipe.crz_out.h/(MFLOAT)crzPipe.crz_crop.h;
        }
        // for lce tuning
        if(p_parameter->isLceAheadCrz==MTRUE){
            ispYuvPipe.lceInputW = p_parameter->imgi.size.w;
            ispYuvPipe.lceInputH = p_parameter->imgi.size.h;
            tdriPipe.lceInputW = p_parameter->imgi.size.w;
            tdriPipe.lceInputH = p_parameter->imgi.size.h;
        } else {
            ispYuvPipe.lceInputW = crzPipe.crz_out.w;
            ispYuvPipe.lceInputH = crzPipe.crz_out.h;
            tdriPipe.lceInputW = crzPipe.crz_out.w;
            tdriPipe.lceInputH = crzPipe.crz_out.h;

        }
    } else {
        crzPipe.conf_cdrz = 0;
        ispYuvPipe.sl2cCrzRatio = 1; // for sl2c tuning
        ispYuvPipe.lceInputW = p_parameter->imgi.size.w;
        ispYuvPipe.lceInputH = p_parameter->imgi.size.h;
        tdriPipe.lceInputW = p_parameter->imgi.size.w;
        tdriPipe.lceInputH = p_parameter->imgi.size.h;
    }


    ISP_PATH_DBG("[CRZ]en(%d),in[%d,%d],out[%d,%d],sl2cCrzRatio(%f)",p_parameter->croppara.crz_enable,crzPipe.crz_in.w,crzPipe.crz_in.h,crzPipe.crz_out.w,crzPipe.crz_out.h,ispYuvPipe.sl2cCrzRatio);
    ISP_PATH_DBG("[CRZ][CROP](x,y,w,h)=(%d,%d,%d,%d),floatx,y=(%u,%u)",crzPipe.crz_crop.x,crzPipe.crz_crop.y,crzPipe.crz_crop.w,crzPipe.crz_crop.h,crzPipe.crz_crop.floatX,crzPipe.crz_crop.floatY);
    ISP_PATH_DBG("[LCE]isLceAheadCrz(%d),lceInputW(%d),lceInputH(%d)",p_parameter->isLceAheadCrz,ispYuvPipe.lceInputW,ispYuvPipe.lceInputH);


    //crz set for tpipe input
    p_parameter->crz.crz_input_crop_width = crzPipe.crz_crop.w;
    p_parameter->crz.crz_input_crop_height = crzPipe.crz_crop.h;
    p_parameter->crz.crz_output_width = crzPipe.crz_out.w;
    p_parameter->crz.crz_output_height = crzPipe.crz_out.h;
    p_parameter->crz.crz_horizontal_integer_offset = crzPipe.crz_crop.x;/* pixel base */
    p_parameter->crz.crz_horizontal_subpixel_offset = \
        ((crzPipe.crz_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
    p_parameter->crz.crz_vertical_integer_offset = crzPipe.crz_crop.y;/* pixel base */
    p_parameter->crz.crz_vertical_subpixel_offset = \
        ((crzPipe.crz_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */

    ISP_PATH_DBG("CRZ:crz_vertical_integer_offset(%u),crz_vertical_subpixel_offset(%u) ",p_parameter->crz.crz_vertical_integer_offset,p_parameter->crz.crz_vertical_subpixel_offset);
    ISP_PATH_DBG("CRZ:crz_horizontal_integer_offset(%u),crrz_horizontal_subpixel_offset(%u) ",p_parameter->crz.crz_horizontal_integer_offset,p_parameter->crz.crz_horizontal_subpixel_offset);

    Result = ispTopCtrl.m_pIspDrvShell->m_pP2CrzDrv->CalAlgoAndCStep((CRZ_DRV_MODE_ENUM)crzPipe.tpipeMode,
                                                          CRZ_DRV_RZ_CRZ,
                                                          crzPipe.crz_in.w,
                                                          crzPipe.crz_in.h,
                                                          crzPipe.crz_crop.w,
                                                          crzPipe.crz_crop.h,
                                                          crzPipe.crz_out.w,
                                                          crzPipe.crz_out.h,
                                                          (CRZ_DRV_ALGO_ENUM*)&hAlgo,
                                                          (CRZ_DRV_ALGO_ENUM*)&vAlgo,
                                                          &hTable,
                                                          &vTable,
                                                          &hCoeffStep,
                                                          &vCoeffStep);

    p_parameter->crz.crz_horizontal_luma_algorithm = hAlgo;
    p_parameter->crz.crz_vertical_luma_algorithm = vAlgo;
    p_parameter->crz.crz_horizontal_coeff_step = hCoeffStep;
    p_parameter->crz.crz_vertical_coeff_step = vCoeffStep;


    ISP_PATH_DBG("CDRZ:in[%d, %d] crop[%d %d %d %d]_f(0x%x, 0x%x) out[%d,%d] ",crzPipe.crz_in.w,crzPipe.crz_in.h, \
            crzPipe.crz_crop.x,crzPipe.crz_crop.y, crzPipe.crz_crop.w,crzPipe.crz_crop.h, \
            crzPipe.crz_crop.floatX,crzPipe.crz_crop.floatY,crzPipe.crz_out.w,crzPipe.crz_out.h);
    ISP_PATH_DBG("hCoeffStep(%d) vCoeffStep(%d) ",hCoeffStep,vCoeffStep);


    //imgi
    DMAImgi.dma_cfg = p_parameter->imgi;
    ISP_PATH_DBG("[imgi](%d , %d , %d) vidoRotate(%d),pa(0x%x) ", p_parameter->imgi.size.w, p_parameter->imgi.size.h, p_parameter->imgi.size.stride,crzPipe.vido_out.Rotation,p_parameter->imgi.memBuf.base_pAddr);

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


    ISP_PATH_DBG("[imgi_mdp]p2MdpSrcFmt(0x%x),w(%d),h(%d),stride(%d,%d),size(%d,%d,%d),planeN(%d)", \
        DMAImgi.p2MdpSrcFmt,DMAImgi.p2MdpSrcW,DMAImgi.p2MdpSrcH,DMAImgi.p2MdpSrcYStride,DMAImgi.p2MdpSrcUVStride,DMAImgi.p2MdpSrcSize,DMAImgi.p2MdpSrcCSize,DMAImgi.p2MdpSrcVSize,DMAImgi.p2MdpSrcPlaneNum);

    //ufdi
    DMAUfdi.dma_cfg = p_parameter->ufdi;
    DMAUfdi.bypass =( 0 == DMAUfdi.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //lcei for vfb p2b path
    // lcei be set from path imageio
    if ( CAM_CTL_EN_P2_DMA_LCEI_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw) {
        DMALcei.dma_cfg = p_parameter->lcei;
        DMALcei.bypass = 0;
    } else {
        DMALcei.bypass = 1;
    }
    ISP_PATH_DBG("[LCEI]bypass(%d),size(0x%x),Va(0x%x),Pa(0x%x),(w,h,s)=(%d,%d,%d),crop(x,y,w,h)=(%d,%d,%d,%d)", \
            DMALcei.bypass, DMALcei.dma_cfg.memBuf.size, DMALcei.dma_cfg.memBuf.base_vAddr, \
            DMALcei.dma_cfg.memBuf.base_pAddr,DMALcei.dma_cfg.size.w, DMALcei.dma_cfg.size.h, \
            DMALcei.dma_cfg.size.stride, DMALcei.dma_cfg.crop.x, DMALcei.dma_cfg.crop.y, \
            DMALcei.dma_cfg.crop.w, DMALcei.dma_cfg.crop.h);


    //vipi
    DMAVipi.dma_cfg = p_parameter->vipi;
    DMAVipi.bypass =( 0 == DMAVipi.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //vip2i
    DMAVip2i.dma_cfg = p_parameter->vip2i;
    DMAVip2i.bypass =( 0 == DMAVip2i.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //vip3i
    DMAVip3i.dma_cfg = p_parameter->vip3i;
    DMAVip3i.bypass =( 0 == DMAVip3i.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //mfbo
    DMAMfbo.dma_cfg = p_parameter->mfbo;
    DMAMfbo.bypass =( 0 == DMAMfbo.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //img3o
    if ( CAM_CTL_EN_P2_DMA_IMG3O_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw)
    {
        DMAImg3o.bypass = 0;
        DMAImg3o.dma_cfg = p_parameter->img3o;
        ISP_PATH_DBG("[IMG3O]bypass(%d),size(0x%x),Va(0x%x),Pa(0x%x),(w,h,s)=(%d,%d,%d),crop(x,y,w,h)=(%d,%d,%d,%d)", \
                DMAImg3o.bypass, DMAImg3o.dma_cfg.memBuf.size, DMAImg3o.dma_cfg.memBuf.base_vAddr, \
                DMAImg3o.dma_cfg.memBuf.base_pAddr,DMAImg3o.dma_cfg.size.w, DMAImg3o.dma_cfg.size.h, \
                DMAImg3o.dma_cfg.size.stride, DMAImg3o.dma_cfg.crop.x, DMAImg3o.dma_cfg.crop.y, \
                DMAImg3o.dma_cfg.crop.w, DMAImg3o.dma_cfg.crop.h);
    }
    else
    {   DMAImg3o.bypass = 1;    }
    //img3bo
    DMAImg3bo.dma_cfg = p_parameter->img3bo;
    DMAImg3bo.bypass =( 0 == DMAImg3bo.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //img3co
    DMAImg3co.dma_cfg = p_parameter->img3co;
    DMAImg3co.bypass =( 0 == DMAImg3co.dma_cfg.memBuf.base_pAddr )? 1 : 0;

    //imgo
    DMAImgo.dma_cfg = p_parameter->imgo;
    DMAImgo.bypass =( 0 == DMAImgo.dma_cfg.memBuf.base_pAddr )? 1 : 0;

    //img2o
    if ( CAM_CTL_EN_P2_DMA_IMG2O_EN & ispTopCtrl.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw){
        DMAImg2o.bypass = 0;
        DMAImg2o.dma_cfg = p_parameter->img2o;

        ISP_PATH_DBG("[IMG2O]bypass(%d),size(0x%x),Va(0x%x),Pa(0x%x),(w,h,s)=(%d,%d,%d),crop(x,y,w,h)=(%d,%d,%d,%d)", \
                DMAImg2o.bypass, DMAImg2o.dma_cfg.memBuf.size, DMAImg2o.dma_cfg.memBuf.base_vAddr, \
                DMAImg2o.dma_cfg.memBuf.base_pAddr,DMAImg2o.dma_cfg.size.w, DMAImg2o.dma_cfg.size.h, \
                DMAImg2o.dma_cfg.size.stride, DMAImg2o.dma_cfg.crop.x, DMAImg2o.dma_cfg.crop.y, \
                DMAImg2o.dma_cfg.crop.w, DMAImg2o.dma_cfg.crop.h);
    } else {
        DMAImg2o.bypass = 1;
    }

    //feo
    DMAFEO.dma_cfg = p_parameter->feo;
    DMAFEO.bypass =( 0 == DMAFEO.dma_cfg.memBuf.base_pAddr )? 1 : 0;

    /* config MDP pipe */
    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        ispMdpPipe.drvScenario = p_parameter->drvScenario;
        ispMdpPipe.isV3 = p_parameter->isV3;
        ispMdpPipe.isApplyTuning = p_parameter->isApplyTuning;
        ispMdpPipe.src_crop.x = p_parameter->croppara.mdp_crop.x;
        ispMdpPipe.src_crop.floatX = ((p_parameter->croppara.mdp_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.src_crop.y = p_parameter->croppara.mdp_crop.y;
        ispMdpPipe.src_crop.floatY = ((p_parameter->croppara.mdp_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.src_crop.w = p_parameter->croppara.mdp_crop.w;
        ispMdpPipe.src_crop.h = p_parameter->croppara.mdp_crop.h;
        ispMdpPipe.isWaitBuf = p_parameter->isWaitBuf;
        ispMdpPipe.p2_dma_enable = p_parameter->isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw;
        ispMdpPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispMdpPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispMdpPipe.mdp_imgxo_p2_en = p_parameter->mdp_imgxo_p2_en;
        ispMdpPipe.venco_out = p_parameter->venco; //venco
        ispMdpPipe.wroto_out = p_parameter->wroto; //wroto
        ispMdpPipe.wdmao_out = p_parameter->wdmao; //wdmao
        ispMdpPipe.jpego_out = p_parameter->jpego; //jpego
        ispMdpPipe.imgxo_out = p_parameter->imgxo; //imgxo
        ispMdpPipe.jpgpara_out = p_parameter->jpgpara; //imgxo
    } else {
        // we need to use dpframework with GCE to run the frame mode,
        // so set a small pseudo source size to execute tpipe algorithm for getting one tile only

        ispMdpPipe.drvScenario = p_parameter->drvScenario;
        ispMdpPipe.isWaitBuf = p_parameter->isWaitBuf;
        ispMdpPipe.p2_dma_enable = p_parameter->isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw;
        ispMdpPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispMdpPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispMdpPipe.mdp_imgxo_p2_en = p_parameter->mdp_imgxo_p2_en;
        ispMdpPipe.venco_out = p_parameter->venco; //venco
        ispMdpPipe.wroto_out = p_parameter->wroto; //wroto
        ispMdpPipe.wdmao_out = p_parameter->wdmao; //wdmao
        ispMdpPipe.jpego_out = p_parameter->jpego; //jpego


        ispMdpPipe.imgxo_out = p_parameter->imgxo; //imgxo
        ispMdpPipe.imgxo_out.withCropM=true;    //fake for ip-raw path
        ispMdpPipe.imgxo_out.enSrcCrop=true;
        ispMdpPipe.imgxo_out.srcCropX= 0;
        ispMdpPipe.imgxo_out.srcCropFloatX= 0;/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.imgxo_out.srcCropY= 0;
        ispMdpPipe.imgxo_out.srcCropFloatY= 0;/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.imgxo_out.srcCropW= 160;
        ispMdpPipe.imgxo_out.srcCropH= 120;
        ispMdpPipe.imgxo_out.size.w = 160; //hardcode setting
        ispMdpPipe.imgxo_out.size.h = 120; //hardcode setting
        ispMdpPipe.imgxo_out.size.stride = 320; //hardcode setting

        ispMdpPipe.jpgpara_out = p_parameter->jpgpara; //imgxo
    }

    /* tpipemain */
    tdriPipe.CQ = p_parameter->CQ;
    tdriPipe.isV3 = p_parameter->isV3;
    tdriPipe.isApplyTuning = p_parameter->isApplyTuning;
    tdriPipe.dupCqIdx = p_parameter->dupCqIdx;
    tdriPipe.burstQueIdx = p_parameter->burstQueIdx;
    tdriPipe.sub_mode = p2SubMode;
    tdriPipe.tdri.top.mode = p2SubMode;
    tdriPipe.magicNum = p_parameter->magicNum;
    tdriPipe.drvScenario = p_parameter->drvScenario;
    tdriPipe.bypass = 0; // to clean legacy setting
    tdriPipe.tcm_en = p_parameter->tcm_en;

    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        tdriPipe.enTdri = 1;
        DMATdri.bypass = 0;
        this->configTpipeData(p_parameter);
        DMATdri.dma_cfg = p_parameter->tdri;
        tdriPipe.tdr_en = (p_parameter->tdr_en&0xc0000000)\
            |(tdriPipe.tdri.tdriPerformCfg.tpipeWidth&0x3ff)\
            |(tdriPipe.tdri.tdriPerformCfg.tpipeHeight&0x7FFC00);
        ispTopCtrl.tpipe_w = tdriPipe.tdri.tdriPerformCfg.tpipeWidth;
        ispTopCtrl.tpipe_h = tdriPipe.tdri.tdriPerformCfg.tpipeHeight;
        ispTopCtrl.isP2IspOnly = 0;
        DMACQ.isP2IspOnly = 0;
    }
    else {  // CAM_MODE_FRAME
        tdriPipe.enTdri = 1;  // for campatible dpframework flow with GCE
        DMATdri.bypass = 0;  // for campatible dpframework flow with GCE
        this->configTpipeData(p_parameter); // for campatible dpframework flow with GCE
        ispTopCtrl.isP2IspOnly = 0; // for campatible dpframework flow with GCE
        DMACQ.isP2IspOnly = 0;  // for campatible dpframework flow with GCE
    }

    //buffer control path
    ispBufCtrl.path = ispTopCtrl.path;
    ispBufCtrl.dupCqIdx = p_parameter->dupCqIdx;
    ispBufCtrl.burstQueIdx = p_parameter->burstQueIdx;

    /* config pass2 CQ */
    if ( CAM_ISP_CQ_NONE != p_parameter->CQ ) {
        //CAM_ISP_CQ0 is illegal
        DMACQ.bypass = 0; //
        ispTuningCtrl.CQ = p_parameter->CQ;
        ispTopCtrl.CQ = p_parameter->CQ;
        ispRawPipe.CQ = p_parameter->CQ;
        ispRgbPipe.CQ = p_parameter->CQ;
        ispYuvPipe.CQ = p_parameter->CQ;
        ispMdpPipe.CQ = p_parameter->CQ;
        crzPipe.CQ = p_parameter->CQ;
        DMAImgi.CQ = p_parameter->CQ;
        DMAUfdi.CQ = p_parameter->CQ;
        DMALcei.CQ = p_parameter->CQ;
        DMAVipi.CQ = p_parameter->CQ;
        DMAVip2i.CQ = p_parameter->CQ;
        DMAVip3i.CQ = p_parameter->CQ;
        DMATdri.CQ = p_parameter->CQ;
        DMACQ.CQ = p_parameter->CQ;
        //ispBufCtrl.CQ = p_parameter->CQ;
        DMAMfbo.CQ = p_parameter->CQ;
        //
        DMAImgo.CQ = p_parameter->CQ;
        DMAImg2o.CQ = p_parameter->CQ;
        //
        DMAImg3o.CQ = p_parameter->CQ;
        DMAImg3bo.CQ = p_parameter->CQ;
        DMAImg3co.CQ = p_parameter->CQ;
        //
        DMAFEO.CQ =  p_parameter->CQ;
        //
        this->CQ = p_parameter->CQ;//for path config
        //
    }
    else {
        DMACQ.bypass = 1;
    }

    //
    ret = this->_config((void*)p_parameter);

EXIT:

    ISP_PATH_DBG("-");
    return ret;
}




int CamPathPass2::getTpipePerform( struct CamPathPass2Parameter* p_parameter )
{
    tdriPipe.tdri.tdriPerformCfg.tpipeWidth = MAX_TPIPE_WIDTH;
    //
    // check and set tpipe height
    tdriPipe.tdri.tdriPerformCfg.tpipeHeight = MAX_TPIPE_HEIGHT;

    // check and set irq mode
//    if( p_parameter->isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw & (CAM_CTL_EN_P2_DMA_MDP_WROTO_EN|CAM_CTL_EN_P2_DMA_MDP_WDMAO_EN|CAM_CTL_EN_P2_DMA_MDP_JPEGO_EN|CAM_CTL_EN_P2_DMA_MDP_VENC_EN)){
        tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_TPIPE; // for ISP-MDP or ISP-MDP-JPEG
 //   } else {

//        if(p_parameter->isp_top_ctl.CAM_CTL_EN_P2.Bits.MDPCROP_EN)
//            {
//                tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_FRAME; // only for ISP only
//            }
//            else
//            {
//                tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_TPIPE; // MFB & FD
//            }
 //   }



    ISP_PATH_DBG("tpipeWidth(%d) tpipeHeight(%d) irqMode(%d) ", \
                tdriPipe.tdri.tdriPerformCfg.tpipeWidth,tdriPipe.tdri.tdriPerformCfg.tpipeHeight, \
                tdriPipe.tdri.tdriPerformCfg.irqMode);

    return MTRUE;
}


int CamPathPass2::configTpipeData( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;

#if 1 //_mt6593fpga_dvt_use_
    CAM_REG_CTL_EN_P2 *pP2En;
    CAM_REG_CTL_EN_P2_DMA *pP2DMA;
    CAM_REG_CTL_SEL_P2 *pP2Sel;
    CAM_REG_CTL_FMT_SEL_P2 *pP2FmtSel;
    //
    pP2En = (CAM_REG_CTL_EN_P2*)&(p_parameter->isp_top_ctl.CAM_CTL_EN_P2);
    pP2DMA = (CAM_REG_CTL_EN_P2_DMA*)&(p_parameter->isp_top_ctl.CAM_CTL_EN_P2_DMA);
    pP2Sel = (CAM_REG_CTL_SEL_P2*)&(p_parameter->isp_top_ctl.CAM_CTL_SEL_P2);
    pP2FmtSel = (CAM_REG_CTL_FMT_SEL_P2*)&(p_parameter->isp_top_ctl.CAM_CTL_FMT_SEL_P2);
    //
    //if(p_parameter->tpipe == CAM_MODE_TPIPE)
    //    tdriPipe.enTdri = 1;
    //else
    //    tdriPipe.enTdri = 0;
    //
    tdriPipe.tdri.top.scenario = p_parameter->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO;
    //tdriPipe.tdri.top.mode = p_parameter->isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE; //set in config function
    tdriPipe.tdri.top.pixel_id = pP2FmtSel->Bits.PIX_ID_P2;
    tdriPipe.tdri.top.cam_in_fmt = pP2FmtSel->Bits.IN_FMT;
    tdriPipe.tdri.top.ctl_extension_en = 0; //kk test
    tdriPipe.tdri.top.fg_mode = pP2FmtSel->Bits.FG_MODE;
    tdriPipe.tdri.top.ufdi_fmt = pP2FmtSel->Bits.UFDI_FMT;
    tdriPipe.tdri.top.vipi_fmt = pP2FmtSel->Bits.VIPI_FMT;
    tdriPipe.tdri.top.img3o_fmt = pP2FmtSel->Bits.IMG3O_FMT;
    tdriPipe.tdri.top.imgi_en = pP2DMA->Bits.IMGI_EN;
    tdriPipe.tdri.top.ufdi_en = pP2DMA->Bits.UFDI_EN;
    tdriPipe.tdri.top.unp_en = pP2En->Bits.UNP_EN;
    tdriPipe.tdri.top.ufd_en = pP2En->Bits.UFD_EN;
    tdriPipe.tdri.top.ufd_crop_en = 0;
    tdriPipe.tdri.top.bnr_en = 0; //only for IP
    tdriPipe.tdri.top.lsci_en = 0; //only for IP
    tdriPipe.tdri.top.lsc_en = 0; //only for IP
    tdriPipe.tdri.top.c24_en = pP2En->Bits.C24_EN;
    tdriPipe.tdri.top.sl2_en = pP2En->Bits.SL2_EN;
    tdriPipe.tdri.top.cfa_en = pP2En->Bits.CFA_EN;
    tdriPipe.tdri.top.vipi_en = pP2DMA->Bits.VIPI_EN;
    tdriPipe.tdri.top.vip2i_en = pP2DMA->Bits.VIP2I_EN;
    tdriPipe.tdri.top.vip3i_en = pP2DMA->Bits.VIP3I_EN;
    tdriPipe.tdri.top.mfb_en = pP2En->Bits.MFB_EN;
    tdriPipe.tdri.top.mfbo_en = pP2DMA->Bits.MFBO_EN;
    tdriPipe.tdri.top.g2c_en = pP2En->Bits.G2C_EN;
    tdriPipe.tdri.top.c42_en = pP2En->Bits.C42_EN;
    tdriPipe.tdri.top.sl2b_en = pP2En->Bits.SL2B_EN;
    tdriPipe.tdri.top.nbc_en = pP2En->Bits.NBC_EN;
    tdriPipe.tdri.top.srz1_en = pP2En->Bits.SRZ1_EN;
    tdriPipe.tdri.top.mix1_en = pP2En->Bits.MIX1_EN;
    tdriPipe.tdri.top.srz2_en = pP2En->Bits.SRZ2_EN;
    tdriPipe.tdri.top.mix2_en = pP2En->Bits.MIX2_EN;
    tdriPipe.tdri.top.sl2c_en = pP2En->Bits.SL2C_EN;
    tdriPipe.tdri.top.seee_en = pP2En->Bits.SEEE_EN;
    tdriPipe.tdri.top.lcei_en = pP2DMA->Bits.LCEI_EN;
    tdriPipe.tdri.top.lce_en = pP2En->Bits.LCE_EN;
    tdriPipe.tdri.top.mix3_en = pP2En->Bits.MIX3_EN;
    tdriPipe.tdri.top.crz_en = pP2En->Bits.CRZ_EN;
    tdriPipe.tdri.top.img2o_en = pP2DMA->Bits.IMG2O_EN;
    tdriPipe.tdri.top.fe_en = pP2En->Bits.FE_EN;
    tdriPipe.tdri.top.feo_en = pP2DMA->Bits.FEO_EN;
    tdriPipe.tdri.top.c02_en = pP2En->Bits.C02_EN;
    tdriPipe.tdri.top.nr3d_en = pP2En->Bits.NR3D_EN;
    tdriPipe.tdri.top.crsp_en = pP2En->Bits.CRSP_EN;
    tdriPipe.tdri.top.img3o_en = pP2DMA->Bits.IMG3O_EN;
    tdriPipe.tdri.top.img3bo_en = pP2DMA->Bits.IMG3BO_EN;
    tdriPipe.tdri.top.img3co_en = pP2DMA->Bits.IMG3CO_EN;
    tdriPipe.tdri.top.c24b_en = pP2En->Bits.C24B_EN;
    //
    tdriPipe.tdri.top.mdp_crop_en = pP2En->Bits.MDPCROP_EN;
    tdriPipe.tdri.top.interlace_mode = 0; //kk test
    tdriPipe.tdri.top.ufd_sel = pP2Sel->Bits.UFD_SEL;
    tdriPipe.tdri.top.ccl_sel = pP2Sel->Bits.CCL_SEL;
    tdriPipe.tdri.top.ccl_sel_en = pP2Sel->Bits.CCL_SEL_EN;
    tdriPipe.tdri.top.g2g_sel = pP2Sel->Bits.G2G_SEL;
    tdriPipe.tdri.top.g2g_sel_en = pP2Sel->Bits.G2G_SEL_EN;
    tdriPipe.tdri.top.c24_sel = pP2Sel->Bits.C24_SEL;
    tdriPipe.tdri.top.srz1_sel = pP2Sel->Bits.SRZ1_SEL;
    tdriPipe.tdri.top.mix1_sel = pP2Sel->Bits.MIX1_SEL;
    tdriPipe.tdri.top.crz_sel = pP2Sel->Bits.CRZ_SEL;
    tdriPipe.tdri.top.nr3d_sel = pP2Sel->Bits.NR3D_SEL;
    tdriPipe.tdri.top.fe_sel = pP2Sel->Bits.FE_SEL;
    tdriPipe.tdri.top.mdp_sel = pP2Sel->Bits.MDP_SEL;
    tdriPipe.tdri.top.pca_sel = pP2Sel->Bits.PCA_SEL;
    tdriPipe.tdri.top.imgi_v_flip_en = p_parameter->imgi.v_flip_en;
    tdriPipe.tdri.top.lcei_v_flip_en = p_parameter->imgi.v_flip_en;
    tdriPipe.tdri.top.ufdi_v_flip_en = 0; //kk test

    /* config dma */
    tdriPipe.tdri.imgi.imgi_stride = p_parameter->imgi.size.stride;
    tdriPipe.tdri.vipi.vipi_xsize = p_parameter->vipi.size.xsize-1;
    tdriPipe.tdri.vipi.vipi_ysize = p_parameter->vipi.size.h-1;
    tdriPipe.tdri.vipi.vipi_stride = p_parameter->vipi.size.stride;
    tdriPipe.tdri.vip2i.vip2i_xsize = p_parameter->vip2i.size.xsize-1;
    tdriPipe.tdri.vip2i.vip2i_ysize = p_parameter->vip2i.size.h-1;
    tdriPipe.tdri.vip2i.vip2i_stride = p_parameter->vip2i.size.stride;
    tdriPipe.tdri.vip3i.vip3i_xsize = p_parameter->vip3i.size.xsize-1;
    tdriPipe.tdri.vip3i.vip3i_ysize = p_parameter->vip3i.size.h-1;
    tdriPipe.tdri.vip3i.vip3i_stride = p_parameter->vip3i.size.stride;
    tdriPipe.tdri.ufdi.ufdi_stride = p_parameter->ufdi.size.stride;
    //
    if(p_parameter->isV3 == MTRUE)
    {
        if(p_parameter->isApplyTuning==MTRUE)
        {
            //bnr
            tdriPipe.tdri.tuningFunc.bnr.bpc_en = p_parameter->bnr.bpc_en;
            //lsc
            tdriPipe.tdri.tuningFunc.lsc.sdblk_width = p_parameter->pTuningIspReg->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;//p_parameter->lsc.sdblk_width;
            tdriPipe.tdri.tuningFunc.lsc.sdblk_xnum = p_parameter->pTuningIspReg->CAM_LSC_CTL2.Bits.LSC_SDBLK_XNUM;//p_parameter->lsc.sdblk_xnum;
            tdriPipe.tdri.tuningFunc.lsc.sdblk_last_width = p_parameter->pTuningIspReg->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH;//p_parameter->lsc.sdblk_last_width;
            tdriPipe.tdri.tuningFunc.lsc.sdblk_height =  p_parameter->pTuningIspReg->CAM_LSC_CTL3.Bits.LSC_SDBLK_HEIGHT;//p_parameter->lsc.sdblk_last_height;
            tdriPipe.tdri.tuningFunc.lsc.sdblk_ynum =  p_parameter->pTuningIspReg->CAM_LSC_CTL3.Bits.LSC_SDBLK_YNUM;//p_parameter->lsc.sdblk_ynum;
            tdriPipe.tdri.tuningFunc.lsc.sdblk_last_height =  p_parameter->pTuningIspReg->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lHEIGHT;//p_parameter->lsc.sdblk_last_height;
            //lsci
            tdriPipe.tdri.tuningFunc.lsci.lsci_stride = p_parameter->pTuningIspReg->CAM_LSCI_STRIDE.Bits.STRIDE;//p_parameter->lsci.size.stride;
            //nbc
            tdriPipe.tdri.tuningFunc.nbc.anr_eny = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_ENY;//p_parameter->nbc.anr_eny;
            tdriPipe.tdri.tuningFunc.nbc.anr_enc = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_ENC;//p_parameter->nbc.anr_enc;
            tdriPipe.tdri.tuningFunc.nbc.anr_iir_mode = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_FLT_MODE;//p_parameter->nbc.anr_iir_mode;
            tdriPipe.tdri.tuningFunc.nbc.anr_scale_mode = p_parameter->pTuningIspReg->CAM_ANR_CON1.Bits.ANR_SCALE_MODE;//p_parameter->nbc.anr_scale_mode;
            //seee
            tdriPipe.tdri.tuningFunc.seee.se_edge = p_parameter->pTuningIspReg->CAM_SEEE_OUT_EDGE_CTRL.Bits.SEEE_OUT_EDGE_SEL;//p_parameter->pTuningIspReg->CAM_SEE_EDGE.Bits.SEE_EDGE;//p_parameter->seee.se_edge;
            //cfa
            tdriPipe.tdri.tuningFunc.cfa.bayer_bypass = p_parameter->pTuningIspReg->CAM_DM_O_BYP.Bits.DM_BYP;//p_parameter->pTuningIspReg->CAM_CFA;//p_parameter->cfa.bayer_bypass;
            //sl2
            tdriPipe.tdri.tuningFunc.sl2.sl2_hrz_comp = p_parameter->pTuningIspReg->CAM_SL2_HRZ.Bits.SL2_HRZ_COMP;//p_parameter->sl2.sl2_hrz_comp; //QQ
            tdriPipe.tdri.tuningFunc.sl2b.sl2b_hrz_comp = p_parameter->pTuningIspReg->CAM_SL2_HRZ.Bits.SL2_HRZ_COMP; // sl2b hrz comp get from sl2 hrz comp
            tdriPipe.tdri.tuningFunc.sl2c.sl2c_hrz_comp = p_parameter->pTuningIspReg->CAM_SL2_HRZ.Bits.SL2_HRZ_COMP; // sl2c hrz comp get from sl2 hrz comp
        }
    }
    else
    {
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.bnr, 0x0, sizeof(ISP_TPIPE_CONFIG_BNR_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.lsc, 0x0, sizeof(ISP_TPIPE_CONFIG_LSC_STRUCT));
        ::memset( (char*)&tdriPipe.tdri.tuningFunc.lsci, 0x0, sizeof(ISP_TPIPE_CONFIG_LSCI_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.sl2, 0x0, sizeof(ISP_TPIPE_CONFIG_SL2_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.lce, 0x0, sizeof(ISP_TPIPE_CONFIG_LCE_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.nbc, 0x0, sizeof(ISP_TPIPE_CONFIG_NBC_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.seee, 0x0, sizeof(ISP_TPIPE_CONFIG_SEEE_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.g2c, 0x0, sizeof(ISP_TPIPE_CONFIG_G2C_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.sl2b, 0x0, sizeof(ISP_TPIPE_CONFIG_SL2B_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.sl2c, 0x0, sizeof(ISP_TPIPE_CONFIG_SL2C_STRUCT));
    ::memset( (char*)&tdriPipe.tdri.tuningFunc.lcei, 0x0, sizeof(ISP_TPIPE_CONFIG_LCEI_STRUCT));
    tdriPipe.tdri.tuningFunc.cfa.bayer_bypass = 0;
    tdriPipe.tdri.tuningFunc.cfa.dm_fg_mode = pP2FmtSel->Bits.FG_MODE;


    //lcei stride should be set from imageio in vfb p2b
    tdriPipe.tdri.tuningFunc.lcei.lcei_stride = p_parameter->lcei.size.stride;
    }
    //
    tdriPipe.tdri.mfb.bld_mode = p_parameter->mfb_cfg.bld_mode;
    tdriPipe.tdri.mfb.bld_deblock_en = p_parameter->mfb_cfg.bld_ll_db_en;   //would be updated by tuning
    tdriPipe.tdri.mfb.bld_brz_en = p_parameter->mfb_cfg.bld_ll_brz_en;      //would be updated by tuning
    tdriPipe.tdri.mfbo.mfbo_stride = p_parameter->mfb_cfg.mfb_out.stride;
    tdriPipe.tdri.mfbo.mfbo_xoffset = p_parameter->mfb_cfg.out_xofst;
    tdriPipe.tdri.mfbo.mfbo_yoffset = 0;  //kk test
    tdriPipe.tdri.mfbo.mfbo_xsize = p_parameter->mfb_cfg.mfb_out.w-1;
    tdriPipe.tdri.mfbo.mfbo_ysize = p_parameter->mfb_cfg.mfb_out.h-1;
    //
    tdriPipe.tdri.crz.cdrz_input_crop_width = p_parameter->crz.crz_input_crop_width;
    tdriPipe.tdri.crz.cdrz_input_crop_height = p_parameter->crz.crz_input_crop_height;
    tdriPipe.tdri.crz.cdrz_output_width = p_parameter->crz.crz_output_width;
    tdriPipe.tdri.crz.cdrz_output_height = p_parameter->crz.crz_output_height;
    tdriPipe.tdri.crz.cdrz_luma_horizontal_integer_offset = p_parameter->crz.crz_horizontal_integer_offset;
    tdriPipe.tdri.crz.cdrz_luma_horizontal_subpixel_offset = p_parameter->crz.crz_horizontal_subpixel_offset;
    tdriPipe.tdri.crz.cdrz_luma_vertical_integer_offset = p_parameter->crz.crz_vertical_integer_offset;
    tdriPipe.tdri.crz.cdrz_luma_vertical_subpixel_offset = p_parameter->crz.crz_vertical_subpixel_offset;
    tdriPipe.tdri.crz.cdrz_horizontal_luma_algorithm = p_parameter->crz.crz_horizontal_luma_algorithm;
    tdriPipe.tdri.crz.cdrz_vertical_luma_algorithm = p_parameter->crz.crz_vertical_luma_algorithm;
    tdriPipe.tdri.crz.cdrz_horizontal_coeff_step = p_parameter->crz.crz_horizontal_coeff_step;
    tdriPipe.tdri.crz.cdrz_vertical_coeff_step = p_parameter->crz.crz_vertical_coeff_step;
    //
    tdriPipe.tdri.img2o.img2o_stride = p_parameter->img2o.size.stride;
    tdriPipe.tdri.img2o.img2o_xoffset = 0;  //kk test
    tdriPipe.tdri.img2o.img2o_yoffset = 0;  //kk test
    tdriPipe.tdri.img2o.img2o_xsize = p_parameter->img2o.size.xsize-1;
    tdriPipe.tdri.img2o.img2o_ysize = p_parameter->img2o.size.h-1;
    //
    tdriPipe.tdri.srz1.srz_input_crop_width = p_parameter->srz1_cfg.inout_size.in_w;
    tdriPipe.tdri.srz1.srz_input_crop_height = p_parameter->srz1_cfg.inout_size.in_h;
    tdriPipe.tdri.srz1.srz_output_width = p_parameter->srz1_cfg.inout_size.out_w;
    tdriPipe.tdri.srz1.srz_output_height = p_parameter->srz1_cfg.inout_size.out_h;
    tdriPipe.tdri.srz1.srz_luma_horizontal_integer_offset = p_parameter->srz1_cfg.crop.x;
    tdriPipe.tdri.srz1.srz_luma_horizontal_subpixel_offset = p_parameter->srz1_cfg.crop.floatX;
    tdriPipe.tdri.srz1.srz_luma_vertical_integer_offset = p_parameter->srz1_cfg.crop.y;
    tdriPipe.tdri.srz1.srz_luma_vertical_subpixel_offset = p_parameter->srz1_cfg.crop.floatY;
    tdriPipe.tdri.srz1.srz_horizontal_coeff_step = p_parameter->srz1_cfg.h_step;
    tdriPipe.tdri.srz1.srz_vertical_coeff_step = p_parameter->srz1_cfg.v_step;
    //
    tdriPipe.tdri.srz2.srz_input_crop_width = p_parameter->srz2_cfg.inout_size.in_w;
    tdriPipe.tdri.srz2.srz_input_crop_height = p_parameter->srz2_cfg.inout_size.in_h;
    tdriPipe.tdri.srz2.srz_output_width = p_parameter->srz2_cfg.inout_size.out_w;
    tdriPipe.tdri.srz2.srz_output_height = p_parameter->srz2_cfg.inout_size.out_h;
    tdriPipe.tdri.srz2.srz_luma_horizontal_integer_offset = p_parameter->srz2_cfg.crop.x;
    tdriPipe.tdri.srz2.srz_luma_horizontal_subpixel_offset = p_parameter->srz2_cfg.crop.floatX;
    tdriPipe.tdri.srz2.srz_luma_vertical_integer_offset = p_parameter->srz2_cfg.crop.y;
    tdriPipe.tdri.srz2.srz_luma_vertical_subpixel_offset = p_parameter->srz2_cfg.crop.floatY;
    tdriPipe.tdri.srz2.srz_horizontal_coeff_step = p_parameter->srz2_cfg.h_step;
    tdriPipe.tdri.srz2.srz_vertical_coeff_step = p_parameter->srz2_cfg.v_step;
    //
    MUINT8 block_size;
    //tdriPipe.tdri.fe.fem_harris_tpipe_mode = 2; /* fe_mode */  //kk test
    //tdriPipe.tdri.fe.fem_harris_tpipe_mode = 1; /* fe_mode */  //kk test
    tdriPipe.tdri.fe.fem_harris_tpipe_mode=p_parameter->fe.fem_harris_tpipe_mode;
    switch (tdriPipe.tdri.fe.fem_harris_tpipe_mode)
    {
       case 0: block_size = 32; break; // 32x32
       case 1: block_size = 16; break; // 16x16
       default: // 2
               block_size = 8; break;  // 8x8
    }
    //tdriPipe.tdri.feo.feo_stride = (p_parameter->feo.size.stride/block_size)*56;// 448 bit/8(old stride in pixel)
    tdriPipe.tdri.feo.feo_stride = p_parameter->feo.size.stride; // stride in byte
    //
    tdriPipe.tdri.nr3d.nr3d_on_en = p_parameter->nr3d_cfg.ctrl_onEn;
    tdriPipe.tdri.nr3d.nr3d_on_xoffset = p_parameter->nr3d_cfg.onOff_onOfStX;
    tdriPipe.tdri.nr3d.nr3d_on_yoffset = p_parameter->nr3d_cfg.onOff_onOfStY;
    tdriPipe.tdri.nr3d.nr3d_on_width = p_parameter->nr3d_cfg.onSiz_onWd;
    tdriPipe.tdri.nr3d.nr3d_on_height =p_parameter->nr3d_cfg.onSiz_onHt;
    //
    tdriPipe.tdri.crsp.crsp_ystep = 0x4;
    tdriPipe.tdri.crsp.crsp_xoffset = 0x0;
    tdriPipe.tdri.crsp.crsp_yoffset = 0x1;
    //
    tdriPipe.tdri.img3o.img3o_stride = p_parameter->img3o.size.stride;
    tdriPipe.tdri.img3o.img3o_xoffset = p_parameter->img3o.offset.x;  //it is actually crop offset, not buffer offset
    tdriPipe.tdri.img3o.img3o_yoffset = p_parameter->img3o.offset.y;  //it is actually crop offset, not buffer offset
    tdriPipe.tdri.img3o.img3o_xsize = p_parameter->img3o.size.xsize-1;
    tdriPipe.tdri.img3o.img3o_ysize = p_parameter->img3o.size.h-1;
    //
    tdriPipe.tdri.img3bo.img3bo_stride = p_parameter->img3bo.size.stride;
    tdriPipe.tdri.img3bo.img3bo_xsize = p_parameter->img3bo.size.xsize-1;
    tdriPipe.tdri.img3bo.img3bo_ysize = p_parameter->img3bo.size.h-1;
    //
    tdriPipe.tdri.img3co.img3co_stride = p_parameter->img3co.size.stride;
    tdriPipe.tdri.img3co.img3co_xsize = p_parameter->img3co.size.xsize-1;
    tdriPipe.tdri.img3co.img3co_ysize = p_parameter->img3co.size.h-1;

    /* software tpipe setting */
    if(p_parameter->tpipe == CAM_MODE_TPIPE){
        tdriPipe.tdri.tdriCfg.srcWidth = p_parameter->imgi.size.w;
        tdriPipe.tdri.tdriCfg.srcHeight = p_parameter->imgi.size.h;
    } else {  // hardcode for frame mode
        // we need to use dpframework with GCE to run the frame mode,
        // so set a small pseudo source size to execute tpipe algorithm for getting one tile only
        tdriPipe.tdri.tdriCfg.srcWidth = 160;
        tdriPipe.tdri.tdriCfg.srcHeight = 120;
        //
        tdriPipe.tdri.top.img2o_en = 1; // need to set one output DMA for dpframework(hardcode)
    }
    tdriPipe.tdri.tdriCfg.baseVa = p_parameter->tdri.memBuf.base_vAddr;
    tdriPipe.tdri.tdriCfg.setSimpleConfIdxNumVa = p_parameter->capTdriCfg.setSimpleConfIdxNumVa;
    tdriPipe.tdri.tdriCfg.segSimpleConfBufVa = p_parameter->capTdriCfg.segSimpleConfBufVa;
    //
    // get tpipe perform information
    getTpipePerform(p_parameter);

    ISP_PATH_DBG("setSimpleConfIdxNumVa(0x%x) segSimpleConfBufVa(0x%x) ",tdriPipe.tdri.tdriCfg.setSimpleConfIdxNumVa,
            tdriPipe.tdri.tdriCfg.segSimpleConfBufVa);

    ISP_PATH_DBG("[Tdri]tpipe(%d),srcWidth(%d) srcHeight(%d)",p_parameter->tpipe,tdriPipe.tdri.tdriCfg.srcWidth,tdriPipe.tdri.tdriCfg.srcHeight);
#endif
EXIT:
    return ret;

}

int CamPathPass2::setZoom( MUINT32 zoomRatio )
{
int ret = 0;

    ISP_PATH_DBG(":E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2Lock);


    /*function List*/
     m_isp_function_list[0 ] = (IspFunction_B*)&crzPipe;
     m_isp_function_list[1 ] = (IspFunction_B*)&DMAImgi;
     m_isp_function_list[2 ] = (IspFunction_B*)&DMAVipi;
     m_isp_function_list[3 ] = (IspFunction_B*)&DMAVip2i;
     m_isp_function_list[4 ] = (IspFunction_B*)&DMATdri;
    m_isp_function_count=5;

    //cdp
    crzPipe.CQ = CAM_ISP_CQ1;

    this->_setZoom(NULL);

    ISP_PATH_DBG(":X");

    return ret;

}

int CamPathPass2::_waitIrq(int type, unsigned int irq, int userNumber )
{
int ret = 0;
ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    ISP_PATH_DBG("+,");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2Lock);

    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)type;//ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.UserNumber= userNumber;
    WaitIrq.Status = irq;//ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    WaitIrq.Timeout = CAM_INT_PASS2_WAIT_TIMEOUT_MS;//ms ,0 means pass through.

    if (0 == ispTopCtrl.waitIrq(&WaitIrq) ) {
        ISP_PATH_ERR("waitIrq fail");
        ret = -1;
    }

    ISP_PATH_DBG("-,");

    return ret;
}


//
EIspRetStatus CamPathPass2::dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo, MINT32 const eDrvSce, MINT32 dequeCq,MINT32 dequeDupCqIdx, MUINT32 dequeBurstQueIdx)
{
    EIspRetStatus ret;

    ISP_PATH_DBG("+,dequeCq(%d),dequeBurstQueIdx(%d)",dequeCq,dequeBurstQueIdx);

//    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2Lock);  //kk test
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




