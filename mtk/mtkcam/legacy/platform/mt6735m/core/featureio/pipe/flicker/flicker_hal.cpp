#define LOG_TAG "flicker_hal.cpp"
#include <stdlib.h>
#include <stdio.h>
#include <mtkcam/featureio/pipe_types.h>
#include <mtkcam/algorithm/libflicker/Flicker_type.h>
#include <utils/threads.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include "mtkcam/common.h"
//#include <asm/arch/mt6589_sync_write.h> // For dsb() in isp_reg.h.
#include <mtkcam/drv_common/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
#include "mtkcam/hal/aaa_hal_base.h"
#include <mtkcam/featureio/eis_hal_base.h>

#include "camera_custom_nvram.h"

#include "mtkcam/common.h"
using namespace NSCam;
#include "awb_param.h"
#include "ae_param.h"
#include "af_param.h"
#include "camera_custom_AEPlinetable.h"
#include "dbg_aaa_param.h"
#include "dbg_flicker_param.h"
#include "dbg_ae_param.h"
#include "ae_mgr_if.h"

#include <mtkcam/algorithm/libflicker/sequential_testing.h>
#include <mtkcam/algorithm/libflicker/FlickerDetection.h>
//#include "isp_sysram_drv.h"
//#include "sensor_drv.h"

#include "isp_mgr.h"
#include "mcu_drv.h"
#include "aaa_hal.h"
#include "af_mgr_if.h"
#include "camera_custom_flicker.h"
//#include <content/IContentManager.h>
#include "flicker_hal.h"
#include "camera_custom_nvram.h"
#include <nvram_drv.h>
#include "flicker_util.h"
#include "aaa_sensor_mgr.h"
#include "camera_custom_flicker_para.h"

using namespace NSCamCustom;
//=====================================================
// define
//=====================================================
#define FLIKCER_DEBUG 0
#define FLICKER_MAX_LENG  4096
#define MHAL_FLICKER_WORKING_BUF_SIZE (FLICKER_MAX_LENG*4*3)    // flicker support max size
#define FLICKER_SUPPORT_MAX_SIZE (FLICKER_MAX_LENG*2*3)

#define DEF_PROP_DEF_HZ "z.flk_def_hz"  //50, 60
#define DEF_PROP_VERBOSE_EN "z.flk_verbose_en"

//log
#define logI(fmt, arg...) ALOGD(fmt, ##arg)
#define logV(fmt, arg...) {if(logv_en) logI(fmt, ##arg);}
#define logE(fmt, arg...) ALOGD("MError: func=%s line=%d: " fmt, __FUNCTION__, __LINE__, ##arg)
//=====================================================
// variable
//=====================================================
static int g_frmCntPre=0;
FLICKER_EXT_PARA g_flickerExtPara;
static int g_maxDetExpUs=70000;
static int g_flickerHz=0;
static int logv_en=0;
//=====================================================
void debugSetting()
{
#if FLIKCER_DEBUG
    logv_en = 1;
#else
    logv_en = FlickerUtil::getPropInt(DEF_PROP_VERBOSE_EN,0);
#endif
}
/*
MBOOL FlickerHal::pipeSendCommand(MINT32 cmd, MUINTPTR arg1, MUINTPTR arg2, MUINTPTR arg3)
{
    int ret;
    INormalPipe*   pPipe;
    pPipe = INormalPipe::createInstance(m_sensorId,"flicker");
    ret = pPipe->sendCommand(cmd, arg1, arg2, arg3);
    pPipe->destroyInstance("flicker");
    return ret;
}
*/
FlickerHal* FlickerHal::getInstance(int sensorDev)
{
    static FlickerHal singletonMain(ESensorDev_Main);
    static FlickerHal singletonSub(ESensorDev_Sub);
    static FlickerHal singletonMain2(ESensorDev_MainSecond);
    if(sensorDev==ESensorDev_Main)
        return &singletonMain;
    else if(sensorDev==ESensorDev_Sub)
        return &singletonSub;
    else  //if(sensorDev==ESensorDev_MainSecond)
        return &singletonMain2;
}
//=====================================================
int FlickerHal::setFlickerModeSub(int mode)
{
    switch(mode)
    {
    case MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ:
        logI("setflickermode 60");
            m_flickerMode = LIB3A_AE_FLICKER_MODE_60HZ;
            break;
    case MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ:
        logI("setflickermode 50");
            m_flickerMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
    case MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO:
        logI("setflickermode auto");
            m_flickerMode = LIB3A_AE_FLICKER_MODE_AUTO;
            break;
    case MTK_CONTROL_AE_ANTIBANDING_MODE_OFF:
        logI("setflickermode off");
            m_flickerMode = LIB3A_AE_FLICKER_MODE_OFF;
            break;
        default:
        logI("setflickermode others (error mode)");
            m_flickerMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
    }
    return 0;
}
//=====================================================
MINT32 FlickerHal::createBufSub()
{
    if(m_pVectorData1 != 0)
        logE("m_pVectorData1 is not empty. The release buffer should be called first!");
    m_pVectorData1 = (MINT32*)malloc(MHAL_FLICKER_WORKING_BUF_SIZE); //vector data, sw buffer
        if(m_pVectorData1 == NULL)
        {
        logE("memory1 is not enough");
            return -1;
        }

        m_pVectorData2 = (MINT32*)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);
        if(m_pVectorData2 == NULL)
        {
        logE("memory2 is not enough");
            return -1;
        }

        mpIMemDrv = IMemDrv::createInstance();
        if(mpIMemDrv == NULL)
        {
        logE("mpIMemDrv is NULL");
            return -1;
        }

        for(int i = 0; i < 2; i++)
        {
            flkbufInfo[i].size = flkbufInfo[i].virtAddr = flkbufInfo[i].phyAddr = 0;
        flkbufInfo[i].useNoncache = 0;   // improve the performance
            flkbufInfo[i].memID = -1;
        }

        if(!mpIMemDrv->init())
        {
        logE(" mpIMemDrv->init() error");
            return -1;
        }

        for(int i = 0; i < 2; i++)
        {
            flkbufInfo[i].size = FLICKER_SUPPORT_MAX_SIZE;
            if(mpIMemDrv->allocVirtBuf(&flkbufInfo[i]) < 0)
            {
            logE("mpIMemDrv->allocVirtBuf(): id=%d", i);
                return -1;

            }

            if(mpIMemDrv->mapPhyAddr(&flkbufInfo[i]) < 0)
            {
            logE("mapPhyAddr id=%d",i);
                if (mpIMemDrv->freeVirtBuf(&flkbufInfo[i]) < 0)
                logE("freeVirtBuf, id=%d",i);
            return -1;
                }
                }
    //isp
    m_pIspDrv2 = IspDrv::createInstance();
    if (!m_pIspDrv2) {
        logE("createInstance IspDrv fail");
                return -1;
            }
    if( m_pIspDrv2->init()<0)
    {
           logE("ISP init fail");
        return -1;
    }
    else
    {
           m_pIspRegMap=( isp_reg_t*)m_pIspDrv2->getRegAddr();
    }
    return 0;
}
MVOID FlickerHal::releaseBufSub()
{
     if (m_pIspDrv2)
    {
        m_pIspDrv2->uninit();
        m_pIspDrv2->destroyInstance();
        m_pIspDrv2 = NULL;
    }
        if(m_pVectorData1 != NULL)
        {
            free(m_pVectorData1);
            m_pVectorData1 = NULL;
        }

        if(m_pVectorData2 != NULL)
        {
            free(m_pVectorData2);
            m_pVectorData2 = NULL;
        }

        if(mpIMemDrv)
        {
            for(MINT32 i = 0; i < 2; ++i)
            {
            if(flkbufInfo[i].virtAddr!=0)
                {
                if(mpIMemDrv->unmapPhyAddr(&flkbufInfo[i]) < 0)
                {
                    logE("unmapPhyAddr id=%d",i);
                }

                if (mpIMemDrv->freeVirtBuf(&flkbufInfo[i]) < 0)
                {
                    logE("freeVirtBuf id=%d",i);
                }
                }
            }
            mpIMemDrv->uninit();
            mpIMemDrv->destroyInstance();
        }
}
//=====================================================
FlickerHal::FlickerHal(int sensorDev)
{
    logI("func=%s line=%d",__FUNCTION__, __LINE__);
    mIsEnable=0;
    m_pVectorData1 = 0;
    m_pVectorData2 = 0;
    m_u4SensorPixelClkFreq = 0;
    m_flkAlgStatus = INCONCLUSIVE;
    m_pIspRegMap=NULL;
    m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
    m_u4FlickerWidth = 0;
    m_u4FlickerHeight = 0;
    m_flickerMode=LIB3A_AE_FLICKER_MODE_AUTO;
    m_sensorDev =sensorDev;
    mSensorMode=0; //Modify
    mIsInited=0;
    }
//=====================================================
FlickerHal::~FlickerHal()
{
    logI("func=%s line=%d",__FUNCTION__, __LINE__);
}
//=====================================================
void setFlickerStateHal(EV_TABLE val)
  {
    set_flicker_state(val);
    if(val==Hz50)
        g_flickerHz=50;
    else
        g_flickerHz=60;
  }
//=====================================================
MINT32 FlickerHal::init(MINT32 i4SensorOpenIndex)
    {
    int sensorId=-1;
    logI("func=%s line=%d mUsers=%d",__FUNCTION__, __LINE__,mUsers);
    int iniFilerHz;
    //1: cust init hz
    cust_getFlickerHalPara(&iniFilerHz, &g_maxDetExpUs);
    //2: last detected hz
    if(g_flickerHz==50)
        iniFilerHz=50;
    else if(g_flickerHz==60)
        iniFilerHz=60;
    else
        ;
    //3: property hz
    int propDefFlicker;
    propDefFlicker = FlickerUtil::getPropInt(DEF_PROP_DEF_HZ,0);
    if(propDefFlicker==50)
        iniFilerHz = 50;
    else if(propDefFlicker==60)
        iniFilerHz = 60;
    else
        ;
    //ini flicker hz
    logI("init flicker Hz: %d",iniFilerHz);
    if(iniFilerHz == 50)
      {
        m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
        setFlickerStateHal(Hz50);
      }
    else
      {
        m_u4FlickerFreq = HAL_FLICKER_AUTO_60HZ;
        setFlickerStateHal(Hz60);
      }
    reset_flicker_queue();
    MINT32 err = 0;
    if (mUsers > 0)
      {
        logE("mUsers %d has created", mUsers);
        android_atomic_inc(&mUsers);
        return 0;
      }
    //sensor
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    IHalSensor* pHalSensorObj;
    pHalSensorObj = pIHalSensorList->createSensor("flicker", i4SensorOpenIndex);
    if(pHalSensorObj == NULL)
    {
        err=-1;
        logI("error pHalSensorObj=0");
        goto create_fail_exit;
    }


    err = pHalSensorObj->sendCommand(m_sensorDev,SENSOR_CMD_GET_PIXEL_CLOCK_FREQ, (MUINTPTR)&m_u4SensorPixelClkFreq, 0, 0);
    if(err != 0)
    {
        logI("No plck. \n");
    }
    logI("[Flicker Hal]init - m_u4SensorPixelClkFreq: %d \n", m_u4SensorPixelClkFreq);

    err = pHalSensorObj->sendCommand(m_sensorDev,SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MUINTPTR)&m_u4PixelsInLine, 0, 0);
    if(err != 0)
{
        logI("No pixels per line. \n");
    }
    m_u4PixelsInLine &= 0x0000FFFF;
    android_atomic_inc(&mUsers);
    m_u4FlickerWidth = 0;
    m_u4FlickerHeight = 0;

    //configure flicker window , it can be a fixed window
    int blkH, blkW;
    setWindowInfo(&blkH, &blkW);

    logI("Flicker parameter config read_freq=%d pixel_line=%d column_length=%d\n", (int)m_u4SensorPixelClkFreq, (int)m_u4PixelsInLine, (int)m_u4FlickerHeight);
    logI("flicker_init %d %d %d %d\n",m_u4PixelsInLine, blkH*3, blkW, (int)m_u4SensorPixelClkFreq);
    int ta;
    int tb;
    FLICKER_CUST_PARA para;
    //--------------------------------------------
    // get sensor info
    SensorStaticInfo rSensorStaticInfo;
    logI("ln=%d %d", __LINE__, m_sensorDev);
    switch  ( m_sensorDev )
  {
  case ESensorDev_Main:
        logI("ln=%d %d", __LINE__, m_sensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
      break;
  case ESensorDev_Sub:
        logI("ln=%d %d", __LINE__, m_sensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
      break;
  case ESensorDev_MainSecond:
        logI("ln=%d %d", __LINE__, m_sensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
      break;
  default:    //  Shouldn't happen.
        logI("ln=%d %d", __LINE__, m_sensorDev);
        logE("Invalid sensor device: %d, id=%d", m_sensorDev, sensorId);
      break;
        // return MFALSE;
  }
    logI("ln=%d %d", __LINE__, rSensorStaticInfo.sensorDevID);
    sensorId=rSensorStaticInfo.sensorDevID;
    //--------------------------------------------
    logI("preview");
    int sensorMode;
    if(mSensorMode==SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
        sensorMode = e_sensorModePreview;
    else if(mSensorMode==SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
        sensorMode = e_sensorModeCapture;
    else if(mSensorMode==SENSOR_SCENARIO_ID_NORMAL_VIDEO)
        sensorMode = e_sensorModeVideoPreview;
    else if(mSensorMode==SENSOR_SCENARIO_ID_SLIM_VIDEO1)
        sensorMode = e_sensorModeVideo1;
    else //if(mSensorMode==SENSOR_SCENARIO_ID_SLIM_VIDEO2   )
        sensorMode = e_sensorModeVideo2;

    nvGetFlickerPara(sensorId, sensorMode, &para);

    g_flickerExtPara.flickerFreq[0]=para.flickerFreq[0];
    g_flickerExtPara.flickerFreq[1]=para.flickerFreq[1];
    g_flickerExtPara.flickerFreq[2]=para.flickerFreq[2];
    g_flickerExtPara.flickerFreq[3]=para.flickerFreq[3];
    g_flickerExtPara.flickerFreq[4]=para.flickerFreq[4];
    g_flickerExtPara.flickerFreq[5]=para.flickerFreq[5];
    g_flickerExtPara.flickerFreq[6]=para.flickerFreq[6];
    g_flickerExtPara.flickerFreq[7]=para.flickerFreq[7];
    g_flickerExtPara.flickerFreq[8]=para.flickerFreq[8];
    g_flickerExtPara.flickerGradThreshold=para.flickerGradThreshold;
    g_flickerExtPara.flickerSearchRange=para.flickerSearchRange;
    g_flickerExtPara.minPastFrames=para.minPastFrames;
    g_flickerExtPara.maxPastFrames=para.maxPastFrames;
    g_flickerExtPara.EV50_L50.m=para.EV50_L50.m;
    g_flickerExtPara.EV50_L50.b_l=para.EV50_L50.b_l;
    g_flickerExtPara.EV50_L50.b_r=para.EV50_L50.b_r;
    g_flickerExtPara.EV50_L50.offset=para.EV50_L50.offset;


    g_flickerExtPara.EV50_L60.m=para.EV50_L60.m;
    g_flickerExtPara.EV50_L60.b_l=para.EV50_L60.b_l;
    g_flickerExtPara.EV50_L60.b_r=para.EV50_L60.b_r;
    g_flickerExtPara.EV50_L60.offset=para.EV50_L60.offset;


    g_flickerExtPara.EV60_L50.m=para.EV60_L50.m;
    g_flickerExtPara.EV60_L50.b_l=para.EV60_L50.b_l;
    g_flickerExtPara.EV60_L50.b_r=para.EV60_L50.b_r;
    g_flickerExtPara.EV60_L50.offset=para.EV60_L50.offset;

    g_flickerExtPara.EV60_L60.m=para.EV60_L60.m;
    g_flickerExtPara.EV60_L60.b_l=para.EV60_L60.b_l;
    g_flickerExtPara.EV60_L60.b_r=para.EV60_L60.b_r;
    g_flickerExtPara.EV60_L60.offset=para.EV60_L60.offset;
    g_flickerExtPara.EV50_thresholds[0]=para.EV50_thresholds[0];
    g_flickerExtPara.EV50_thresholds[1]=para.EV50_thresholds[1];
    g_flickerExtPara.EV60_thresholds[0]=para.EV60_thresholds[0];
    g_flickerExtPara.EV60_thresholds[1]=para.EV60_thresholds[1];
    g_flickerExtPara.freq_feature_index[0]=para.freq_feature_index[0];
    g_flickerExtPara.freq_feature_index[1]=para.freq_feature_index[1];
    flicker_setExtPara(&g_flickerExtPara);



    ta = FlickerUtil::getMs();
    flicker_init(m_u4PixelsInLine, blkH*3, blkW, m_u4SensorPixelClkFreq);
    tb = FlickerUtil::getMs();
    logI("flk algn init time = %d ms",(int)(tb-ta));

    if( pHalSensorObj )
        pHalSensorObj->destroyInstance("flicker");
    mIsInited=1;
    return err;

create_fail_exit:
    if( pHalSensorObj )
        pHalSensorObj->destroyInstance("flicker");

    return err;
}
//=====================================================
MINT32 FlickerHal::uninit()
{
    logI("uninit line=%d mUsers=%d",__LINE__, mUsers);
    MINT32 err = 0;
    if (mUsers <= 0)
    {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    if (mUsers == 0)
    {
        flicker_uninit(); //alg uninit
    enableFlickerDetection(0);
        mIsInited=0;
    }
    else
    {
        logI("Still %d users \n", mUsers);
    }
    return 0;
}
//=====================================================
MINT32 FlickerHal::setFlickerDrv(MBOOL flicker_en)
{
    logI("FFLK func=%s line=%d flicker_en=%d",__FUNCTION__, __LINE__,(int)flicker_en);
    int ret = 0;


    if(flicker_en == 1) {  // enable flicker

    //set flk mode as column vector output
        //ISP_WRITE_BITS(m_pIspRegMap,CAM_FLK_CON,FLK_MODE,0); //Yosen: mark out for compiling error
    // FLK enable
        //ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_EN1,FLK_EN,1);
    //FLK enable set
        ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_EN1_SET,FLK_EN_SET,1);
        ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_EN1_CLR,FLK_EN_CLR,0);
    // FLK DMA enable
    //    ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_DMA_EN,ESFKO_EN,1);
    //    ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_DMA_EN_SET,ESFKO_EN_SET,1);
    // FLK DMA Done interrupt Enable
        //ISP_WRITE_BITS(m_pIspRegMap,CAM_CTL_DMA_INT,ESFKO_DONE_EN,1);
        ISP_WRITE_BITS(m_pIspRegMap,CAM_CTL_INT_EN,FLK_DON_EN,1) ;




        mpIMemDrv->cacheFlushAll();
    } else {   // disable flicker

            // disable flk
            //ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_EN1,FLK_EN,0);
             //disable flk dma
            //ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_DMA_EN,ESFKO_EN,0);
            //FLK enable set
            ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_EN1_SET,FLK_EN_SET,0);
            ISP_WRITE_ENABLE_BITS(m_pIspRegMap,CAM_CTL_EN1_CLR,FLK_EN_CLR,1);
            //disable flk dma done interrupt
            //ISP_WRITE_BITS(m_pIspRegMap,CAM_CTL_DMA_INT,ESFKO_DONE_EN,0);

#if 1
        // enable ESFKO done
        //ISP_WRITE_ENABLE_BITS(m_pIspRegMap , CAM_CTL_INT_EN, FLK_DON_EN, 1);

        // wait FLK  done

/*
        ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
        WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
        WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
        WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_FLK_DON_ST;
        WaitIrq.Timeout = 400; // 400 ms

        m_pIspDrv->waitIrq(WaitIrq);*/
        //usleep(100000);

        logI("FFLK func=%s line=%d ",__FUNCTION__, __LINE__);


        //ISP_WRITE_BITS(m_pIspRegMap,CAM_CTL_INT_EN,FLK_DON_EN,0) ;
#endif


    }






    return ret;
    }
//=====================================================
MINT32 FlickerHal::setFlickerWinConfig(FLKWinCFG_T* ptFlkWinCfg)
        {
    logI("FFLK func=%s line=%d",__FUNCTION__, __LINE__);
    int ret = 0;

    ISP_WRITE_BITS(m_pIspRegMap,CAM_FLK_WNUM,FLK_WNUM_X,ptFlkWinCfg->m_u4NumX);
    ISP_WRITE_BITS(m_pIspRegMap,CAM_FLK_WNUM,FLK_WNUM_Y,ptFlkWinCfg->m_u4NumY);
    ISP_WRITE_BITS(m_pIspRegMap,CAM_FLK_SOFST,FLK_SOFST_X,ptFlkWinCfg->m_u4OffsetX);
    ISP_WRITE_BITS(m_pIspRegMap,CAM_FLK_SOFST,FLK_SOFST_Y,ptFlkWinCfg->m_u4OffsetY);
    ISP_WRITE_BITS(m_pIspRegMap,CAM_FLK_WSIZE,FLK_WSIZE_X,ptFlkWinCfg->m_u4SizeX);
    ISP_WRITE_BITS(m_pIspRegMap,CAM_FLK_WSIZE,FLK_WSIZE_Y,ptFlkWinCfg->m_u4SizeY);


//FLICKER_LOG("[setFlickerConfig]:flicker win No.=0x%08x\n", (int) ISP_READ_REG(m_pIspRegMap, CAM_FLK_WNUM));
logI("[setFlickerConfig]:flicker win (x,y)=0x%08x\n", (int) ISP_READ_REG(m_pIspRegMap, CAM_FLK_SOFST));
logI("[setFlickerConfig]:flicker win (w,h)=0x%08x\n", (int) ISP_READ_REG(m_pIspRegMap, CAM_FLK_WSIZE));

    return ret;
}
//=============================================
MINT32 FlickerHal::setFlickerDMAConfig(
    unsigned long flicker_DMA_address ,MINT32 DMASize )
{
   //FLK_DBG_LOG("FFLK func=%s line=%d",__FUNCTION__, __LINE__);
    int ret = 0;

    ISP_WRITE_REG(m_pIspRegMap, CAM_ESFKO_XSIZE, DMASize);
    ISP_WRITE_REG(m_pIspRegMap, CAM_ESFKO_YSIZE, 0);
    ISP_WRITE_REG(m_pIspRegMap, CAM_ESFKO_STRIDE, DMASize);

    ISP_WRITE_REG(m_pIspRegMap, CAM_ESFKO_BASE_ADDR,flicker_DMA_address);
    ISP_WRITE_REG(m_pIspRegMap, CAM_ESFKO_OFST_ADDR,0);

    //FLICKER_LOG("ESFKO X size:0x%08x \n", ISP_READ_REG(m_pIspRegMap, CAM_ESFKO_XSIZE));
    //FLICKER_LOG("CAM_ESFKO_STRIDE:0x%08x \n",  ISP_READ_REG(m_pIspRegMap, CAM_ESFKO_STRIDE));
    //FLICKER_LOG("[setFlickerDMAConfig]:CAM_ESFKO_BASE_ADDR:0x%08x\n", ISP_READ_REG(m_pIspRegMap, CAM_ESFKO_BASE_ADDR));
    return ret;
        }

MUINT32 FlickerHal:: GetFlicker_CurrentDMA()
{
    return (MUINT32)ISP_READ_REG(m_pIspRegMap, CAM_ESFKO_BASE_ADDR);
}
//=============================================
void focusValueResize(MINT64* inH, MINT64* inV, MINT32* out)
        {
    int i;
    for(i=0; i<9; i++)
        {
        int row;
        int col;
        double tmp;
        row=i/3;
        col=i%3;
        tmp     = inH[(col*2)  +(row*2)*6]+
               inH[(col*2+1)+(row*2)*6]+
               inH[(col*2)  +(row*2+1)*6]+
               inH[(col*2+1)+(row*2+1)*6];
        tmp     +=inV[(col*2)  +(row*2)*6]+
               inV[(col*2+1)+(row*2)*6]+
               inV[(col*2)  +(row*2+1)*6]+
               inV[(col*2+1)+(row*2+1)*6];
        if(tmp > 0x7fffffff)
            tmp=0x7fffffff;
        out[i]=tmp;
        logV("fvr i=%d, fv=%d",i,out[i]);
        }

        }
//=============================================
int FlickerHal::Updated(FlickerInput* in, FlickerOutput* out)
        {
    if(mIsInited==0)
        return -1;
        {
        if(m_flickerMode==LIB3A_AE_FLICKER_MODE_AUTO)
        {
            AAASensorMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 1);
    }
        else
{
            AAASensorMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 0);
        }
    }



    int i;
    static int kk=0;
    MINT32 fv3x3[9];

    logV("U %d",kk);
    kk++;
    //---------------------
    // log input
    logV("exp=%d",in->aeExpTime);

    for(i=0; i<6; i++)
    {
        logV("fvH row%d=%lf %lf %lf %lf %lf %lf",i,
             (double)(in->afFullStat.i8StatH[i*6+0]), (double)(in->afFullStat.i8StatH[i*6+1]), (double)(in->afFullStat.i8StatH[i*6+2]),
             (double)(in->afFullStat.i8StatH[i*6+3]), (double)(in->afFullStat.i8StatH[i*6+4]), (double)(in->afFullStat.i8StatH[i*6+5]) );
    }
        for(i=0; i<6; i++)
    {
        logV("fvV row%d=%lf %lf %lf %lf %lf %lf",i,
             (double)(in->afFullStat.i8StatV[i*6+0]), (double)(in->afFullStat.i8StatV[i*6+1]), (double)(in->afFullStat.i8StatV[i*6+2]),
             (double)(in->afFullStat.i8StatV[i*6+3]), (double)(in->afFullStat.i8StatV[i*6+4]), (double)(in->afFullStat.i8StatV[i*6+5]) );
    }
    focusValueResize(in->afFullStat.i8StatH, in->afFullStat.i8StatV, fv3x3);

        logV("fv3x3 %d %d %d", out[0], out[1], out[2]);
        logV("fv3x3 %d %d %d", out[3], out[4], out[5]);
        logV("fv3x3 %d %d %d", out[6], out[7], out[8]);
    //---------------------------
    MINT32    err = 0;
    MINT32    i4DetectedResult = -1;
    g_frmCntPre++;
    if(g_frmCntPre<2)
    {
        goto lbExit;
    }
    //    (3) Analyze the flicker by passing EIS information.
    err = analyzeFlickerFrequency(in->aeExpTime, fv3x3);
    if    (err !=0)
    {
        logI("Updated] mpFlickerHal->analyzeFlickerFrequency() - (err)=(%x)",  err);
        goto lbExit;
    }
    //    (4) Get the flicker result from flicker hal
    err = getFlickerStatus(&i4DetectedResult);
    if    (err !=0)
    {
        logI("[Updated] getFlickerStatus err=%d", err);
        goto lbExit;
    }
    out->flickerResult = i4DetectedResult;

lbExit:
    return    err;
    }
//=============================================
MINT32 FlickerHal::analyzeFlickerFrequency(int exp, MINT32 *i4vAFstatisic)
{
    logV("A ln=%d cnt=%d",__LINE__,g_frmCntPre);
MINT32 i,i4Buff_idx=0;
    MINT32 i4vAFInfo[9];
MINT32 i4DataLen = 0;
    MINT32 u4Height;
MINT32 *m_FickerSW_buff_1;
MINT32 *m_FickerSW_buff_2;

    MINT32* flickerDMAAdr;

           // get the AF statistic information
    i4vAFInfo[0]=i4vAFstatisic[0];
    i4vAFInfo[1]=i4vAFstatisic[1];
    i4vAFInfo[2]=i4vAFstatisic[2];
    i4vAFInfo[3]=i4vAFstatisic[3];
    i4vAFInfo[4]=i4vAFstatisic[4];
    i4vAFInfo[5]=i4vAFstatisic[5];
    i4vAFInfo[6]=i4vAFstatisic[6];
    i4vAFInfo[7]=i4vAFstatisic[7];
    i4vAFInfo[8]=i4vAFstatisic[8];

           if(GetFlicker_CurrentDMA()==flkbufInfo[0].phyAddr)
           {
                   i4Buff_idx=0;
                m_FickerSW_buff_1=m_pVectorData1;  //m_FickerSW_buff_1 is n-1 data
                m_FickerSW_buff_2=m_pVectorData2;  // m_FickerSW_buff_1 is n-2 data
        mpIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &flkbufInfo[1]);
           }
           else
           {
                   i4Buff_idx=1;
                m_FickerSW_buff_1=m_pVectorData2;  //m_FickerSW_buff_1 is n-1 data
                m_FickerSW_buff_2=m_pVectorData1;  //m_FickerSW_buff_2 is n-2 data
        mpIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &flkbufInfo[0]);
           }



    flickerDMAAdr = (MINT32 *) (8*((flkbufInfo[(i4Buff_idx+1)%2].virtAddr + 7)/8));

           if(m_u4FlickerHeight > FLICKER_MAX_LENG)
           {

               i4DataLen = 3*FLICKER_MAX_LENG /2 ;
               u4Height = FLICKER_MAX_LENG;
           }
           else
           {

               i4DataLen = 3*m_u4FlickerHeight /2 ;
               u4Height = m_u4FlickerHeight;
           }

               for(i=0; i<i4DataLen; i++)
               {
        m_FickerSW_buff_1[2*i+0] = flickerDMAAdr[i] &0x0000FFFF;
        m_FickerSW_buff_1[2*i+1] =(flickerDMAAdr[i] &0xFFFF0000)>>16;
               }
    for(i=0; i<u4Height; i+=100)
               {
        logV("sta row =%d, %d %d %d",i,m_FickerSW_buff_1[3*i],m_FickerSW_buff_1[3*i+1],m_FickerSW_buff_1[3*i+2]);
               }
            //switch FKO dst add.  to another buffer
              setFlickerDMAConfig(flkbufInfo[(i4Buff_idx+1)%2].phyAddr,FLK_DMA_Size);

           flkSensorInfo    sensorInfo;
           sensorInfo.pixelClock = m_u4SensorPixelClkFreq;
           sensorInfo.fullLineWidth = m_u4PixelsInLine;

            MINT32 win_wd = ((m_u4FlickerWidth / 3)>>1)<<1;
            MINT32 win_ht = ((u4Height / 3)>>1)<<1;

            flkEISVector    EISvector;
            int curExp;
    curExp = exp;
            flkAEInfo AEInfo;
            AEInfo.previewShutterValue=curExp;
            static int sBinOut=-1;
            int binOut;
            int binOut2;
            int afOn;
            binOut = FlickerUtil::getPropInt("z.flk_bin_out",0);
            binOut2 = FlickerUtil::getPropInt("z.flk_bin_out2",0);
            afOn = FlickerUtil::getPropInt("z.flk_af_on",1);

            if(afOn==0)
            {
        logI("AF OFF");
              int pp;
              for(pp=0;pp<9;pp++)
              {
               i4vAFInfo[pp]=0;
              }

            }

            if(binOut!=1)
            {
              sBinOut = binOut;

            }
    else if( (binOut==1 && sBinOut!=1) || binOut2==1)
            {
              sBinOut=binOut;
              int cnt;
              FlickerUtil::getFileCount("/sdcard/flicker_file_cnt.txt", &cnt, 0);
              FlickerUtil::setFileCount("/sdcard/flicker_file_cnt.txt", cnt+1);
              char s[100];
              FlickerUtil::createDir("/sdcard/flickerdata/");
              sprintf(s,"/sdcard/flickerdata/%03d",cnt);
              FlickerUtil::createDir(s);
              int tmp;
              FILE* fp;
              sprintf(s,"/sdcard/flickerdata/%03d/flk.raw",cnt);
              fp = fopen(s, "wb");
        if(fp)
        {
            tmp = i4DataLen;
              fwrite(&tmp, 1, 4, fp);
              fwrite(m_FickerSW_buff_1, 4, i4DataLen*2, fp);
              fwrite(m_FickerSW_buff_2, 4, i4DataLen*2, fp);
            tmp = win_wd;        fwrite(&tmp, 1, 4, fp);
            tmp = win_ht;        fwrite(&tmp, 1, 4, fp);
            tmp = m_u4FlickerFreq;        fwrite(&tmp, 1, 4, fp);
              fwrite(&sensorInfo, 1, sizeof(flkSensorInfo), fp);
              fwrite(&AEInfo, 1, sizeof(flkAEInfo), fp);
              fwrite(i4vAFInfo, 4, 9, fp);
              fclose(fp);
            logI("line=%d", __LINE__);
            }
            }
             if(   curExp < g_maxDetExpUs &&
                    ( (curExp > 8200 && m_u4FlickerFreq == HAL_FLICKER_AUTO_60HZ ) || (curExp > 9800 && m_u4FlickerFreq == HAL_FLICKER_AUTO_50HZ ) ) )
               {
        //logI("do flicker Sw %d %d %d %d",win_wd, win_ht, curExp, m_u4FlickerFreq);
        m_flkAlgStatus = detectFlicker_SW(m_FickerSW_buff_1, m_FickerSW_buff_2, 3, 3, win_wd, win_ht, m_u4FlickerFreq, sensorInfo, EISvector, AEInfo, i4vAFInfo);

                int propFlickerSwitch;
                propFlickerSwitch = FlickerUtil::getPropInt("z.flk_switch_en",-1);
                if(propFlickerSwitch==0)
                {
            logI("flicker state not changed due to property fixed");
                }
                else
                {
            if(m_flkAlgStatus == FK100 && m_u4FlickerFreq==HAL_FLICKER_AUTO_60HZ) // if the decision is to change exposure table to 50Hz
                  {
                     m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
                        setFlickerStateHal(Hz50);    // defined in "sequential_testing.cpp", have to call these two functions every time we change the flicker table
                                       reset_flicker_queue();
                }
            else if (m_flkAlgStatus == FK120 && m_u4FlickerFreq==HAL_FLICKER_AUTO_50HZ) // if the decision is to change exposure table to 60Hz
                  {
                  m_u4FlickerFreq = HAL_FLICKER_AUTO_60HZ;
                        setFlickerStateHal(Hz60);    // defined in "sequential_testing.cpp", have to call these two functions every time we change the flicker table
                                       reset_flicker_queue();
                }
                }
            }
            else if( curExp >= g_maxDetExpUs)
            {
        logV("The exposure time is too long, skip flicker detection:%d\n", curExp);
            }
            else
            {
        logV("The exposure time is too short, skip flicker detection:%d\n", curExp);
    }
    logI("Status:%d, %d\n",m_u4FlickerFreq,curExp);
    return 0;
}
//=============================================
MINT32 FlickerHal::enableFlickerDetection(MBOOL bEnableFlicker)
{
    MINT32 ret = 0;    // 0: no error.
    logI("enableFlickerDetection() bEnableFlicker= %d\n",bEnableFlicker);
    if(mIsEnable==bEnableFlicker)
    {
        logI("flicker enable state is ame, not set to hardware");
        return 0;
    }
    if(bEnableFlicker)
    {
        if(flkbufInfo[0].virtAddr!=0)
        {
            MVOID * rPhyAddress = NULL;
            rPhyAddress = (MVOID*)flkbufInfo[0].phyAddr;
            /* FLK DMA size:
            2 bytes for per line in one window
            ESFKO_XSIZE = (FLK_WNUM_X * FLK_WNUM_Y * FLK_WSIZE_Y * 2) - 1
            ESFKO_YSIZE = 0 */
            FLK_DMA_Size=(strFlkWinCfg.m_u4NumX*strFlkWinCfg.m_u4NumY*strFlkWinCfg.m_u4SizeY*2)-1;
            setFlickerDMAConfig((MUINTPTR)rPhyAddress ,FLK_DMA_Size);
        }
        else
        {
            logI("!!!UNABLE to update pPhyAddr,pVirAddr!!!\n");
        }
        setFlickerDrv(bEnableFlicker);    // Save the column vector and difference
    }
    else
    {
        setFlickerDrv(bEnableFlicker);    // disable theflicker
    }
    mIsEnable = bEnableFlicker;
    return ret;
}
//=============================================
MINT32 FlickerHal::setWindowInfo(int* blkH, int*blkW)
{
    logI("FFLK func=%s line=%d",__FUNCTION__, __LINE__);
    MUINT32 u4Height;
    MUINT32 u4Width;
    MINT32 u4PixelEnd, u4PixelStart, u4LineEnd, u4LineStart;
    MUINT32 u4ToleranceLine=20;



        u4PixelEnd = ISP_READ_BITS(m_pIspRegMap, CAM_TG_SEN_GRAB_PXL, PXL_E);
        u4PixelStart =  ISP_READ_BITS(m_pIspRegMap, CAM_TG_SEN_GRAB_PXL, PXL_S);
        u4LineEnd = ISP_READ_BITS(m_pIspRegMap, CAM_TG_SEN_GRAB_LIN, LIN_E);
        u4LineStart =  ISP_READ_BITS(m_pIspRegMap, CAM_TG_SEN_GRAB_LIN, LIN_S);
        u4Width =  u4PixelEnd - u4PixelStart + 1 - 4;
        u4Height = u4LineEnd - u4LineStart + 1 -6;

            m_u4FlickerWidth = u4Width;
            m_u4FlickerHeight = u4Height-u4ToleranceLine;
            logI("[setWindowInfo] width:%d ,%d height:%d ,%d\n", u4Width, m_u4FlickerWidth, u4Height, m_u4FlickerHeight);
            if(m_u4FlickerHeight > FLICKER_MAX_LENG-6){
                u4Height = FLICKER_MAX_LENG-6;
            } else {
                u4Height = m_u4FlickerHeight;
            }
            strFlkWinCfg.m_uImageW=u4Width;
            strFlkWinCfg.m_uImageH=u4Height;
            strFlkWinCfg.m_u4NumX=3;
            strFlkWinCfg.m_u4NumY=3;
            strFlkWinCfg.m_u4OffsetX=0;
            strFlkWinCfg.m_u4OffsetY=0+u4ToleranceLine;
            //strFlkWinCfg.m_u4SizeX=(u4Width-strFlkWinCfg.m_u4OffsetX)/3;
            //strFlkWinCfg.m_u4SizeY=(u4Height-strFlkWinCfg.m_u4OffsetY+u4ToleranceLine)/3;
            strFlkWinCfg.m_u4SizeX=((u4Width-strFlkWinCfg.m_u4OffsetX)/6)*2;
            strFlkWinCfg.m_u4SizeY=((u4Height-strFlkWinCfg.m_u4OffsetY+u4ToleranceLine)/6)*2;

            setFlickerWinConfig(&strFlkWinCfg);

            *blkH = strFlkWinCfg.m_u4SizeY;
            *blkW = strFlkWinCfg.m_u4SizeX;
            logI("blkH=%d, blkW = %d",*blkH, *blkW);
            //FLICKER_LOG("[setWindowInfo] m_u4SizeX:%d , m_u4SizeY:%d \n", strFlkWinCfg.m_u4SizeX, strFlkWinCfg.m_u4SizeY );

          // FLICKER_LOG("[setWindowInfo] exist window infor\n");
        return 0;
}
MINT32 FlickerHal::getFlickerStatus(MINT32 *a_flickerStatus)
{
    if(m_flickerMode==LIB3A_AE_FLICKER_MODE_60HZ)
{
        logV("getFlickerStatus 60");
        *a_flickerStatus= HAL_FLICKER_AUTO_60HZ;
}
    else if(m_flickerMode==LIB3A_AE_FLICKER_MODE_50HZ)
    {
        logV("getFlickerStatus 50");
        *a_flickerStatus= HAL_FLICKER_AUTO_50HZ;
    }
    else if(m_flickerMode==AE_FLICKER_MODE_OFF)
{
        logV("getFlickerStatus off");
        *a_flickerStatus = HAL_FLICKER_AUTO_OFF;
}
    else // LIB3A_AE_FLICKER_MODE_AUTO
    {
        logV("getFlickerStatus auto");
        *a_flickerStatus = m_u4FlickerFreq;
    }
    //------------------------------
    logI("getFlickerStatus %d",(int)*a_flickerStatus);
    return 0;
}
int FlickerHal::enable(int en)
    {
    logI("func=%s ln=%d",__FUNCTION__, __LINE__);
    enableFlickerDetection(en);
    return 0;
    }
//=====================================================
int FlickerHal::setSensorMode(int i4NewSensorMode)
{
    mSensorMode = i4NewSensorMode;
    return 0;
}
int FlickerHal::createBuf()
{
    logI("func=%s ln=%d",__FUNCTION__, __LINE__);
    Mutex::Autolock lock(m_lock);
    return createBufSub();
}
int FlickerHal::releaseBuf()
{
    logI("func=%s ln=%d",__FUNCTION__, __LINE__);
    Mutex::Autolock lock(m_lock);
    releaseBufSub();
    return 0;
}
int FlickerHal::open(int sensorId, int tgInfo)
{
/* Modify
    logI("func=%s ln=%d tgInfo=%d (1,2=%d,%d)",__FUNCTION__, __LINE__,tgInfo, CAM_TG_1, CAM_TG_2);
    debugSetting();
    Mutex::Autolock lock(m_lock);
    m_sensorId = sensorId;
    mTgInfo = tgInfo;
    if(mTgInfo!=CAM_TG_1)
    {
        return -1;
    }
*/
    Mutex::Autolock lock(m_lock); //
    if (init(sensorId) != 0)
    {
        logI("singleton.init() fail \n");
        return -1;
    }
    enable(1);
    return 0;
}
int FlickerHal::close()
{
/* Modify
    logI("func=%s ln=%d tgInfo=%d",__FUNCTION__, __LINE__,mTgInfo);
    Mutex::Autolock lock(m_lock);
    if(mTgInfo!=CAM_TG_1)
{
        return -1;
}
*/
    Mutex::Autolock lock(m_lock); //
    enable(0);
    uninit();
    return 0;
}
int FlickerHal::update(FlickerInput* in, FlickerOutput* out)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("slow_motion_debug", value, "0");
    if(atoi(value))
{
        return 0;
}
    Mutex::Autolock lock(m_lock);
/* Modify
    if(mTgInfo!=CAM_TG_1)
    {
        return -1;
    }
*/
    return Updated(in, out);
}

int FlickerHal::setFlickerMode(int mode)
{
    Mutex::Autolock lock(m_lock);
    return setFlickerModeSub(mode);
}
int FlickerHal::getFlickerResult(int& flickerResult)
{
    Mutex::Autolock lock(m_lock);
    flickerResult = m_u4FlickerFreq;
    return 0;
}
