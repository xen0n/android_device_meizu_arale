#define LOG_TAG "MtkCam/VSSScen"
//
#include <vector>
using namespace std;
//
#include <utils/Vector.h>
#include <mtkcam/common.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
//
#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/IXdpPipe.h>
//
#include <mtkcam/drv_common/imem_drv.h>

#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>
using namespace NSImageio;
using namespace NSIspio;
//
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
#include <mtkcam/v1/hwscenario/IhwScenario.h>
#include "hwUtility.h"
#include "VSSScenario.h"
//
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <mtkcam/v1/camutils/CamInfo.h>
#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#endif
#include <cutils/properties.h>
#include <mtkcam/v1/camutils/CamMisc.h>
using namespace MtkCamUtils;
//
#define ROUND_TO_2X(x) ((x) & (~0x1))
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");
#define ERROR_LOG               MY_LOGE("Error");
//
#define _PASS1_CQ_CONTINUOUS_MODE_
#define ENABLE_LOG_PER_FRAME    (1)
//
#ifdef HAVE_AEE_FEATURE
    #define AEE_ASSERT(String) \
        do { \
            aee_system_exception( \
                "VSSScenario", \
                NULL, \
                DB_OPT_DEFAULT, \
                String); \
        } while(0)
#else
    #define AEE_ASSERT(String)
#endif
//
/*******************************************************************************
*
********************************************************************************/
VSSScenario*
VSSScenario::createInstance(EScenarioFmt rSensorType, MUINT32 const &dev, ERawPxlID const &bitorder)
{
    return new VSSScenario(rSensorType, dev, bitorder);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::destroyInstance()
{
    //
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
VSSScenario::VSSScenario(EScenarioFmt rSensorType, MUINT32 const &dev, ERawPxlID const &bitorder)
            : mpCamIOPipe(NULL)
            , mpPostProcPipe(NULL)
            , mpXdpPipe(NULL)
            , mpIMemDrv(NULL)
            , mSensorType(rSensorType)
            , mSensorDev(dev)
            , mSensorBitOrder(bitorder)
            , mModuleMtx()
            , mpExtImgProcHw(NULL)
            , mbTwoRunPass2(MFALSE)
            , mbTwoRunPass2Dispo(MTRUE)
            , debugScanLine(DebugScanLine::createInstance())
{
    mSensorId = dev == SENSOR_DEV_MAIN ? 0 : 1;
    MINT32 wantedOri = android::MtkCamUtils::DevMetaInfo::queryDeviceWantedOrientation(mSensorId);
    MINT32 realOri = android::MtkCamUtils::DevMetaInfo::queryDeviceSetupOrientation(mSensorId);
    if(wantedOri == realOri)
    {
        mbTwoRunRot = MFALSE;
    }
    else
    {
        mbTwoRunRot = MTRUE;
    }
    MY_LOGD("mSensorId(%d),wantedOri(%d),realOri(%d),mbTwoRunRot(%d)",
            mSensorId,
            wantedOri,
            realOri,
            mbTwoRunRot);
    //
    MY_LOGD("mSensorBitOrder(%d),this(%p),sizeof(%d)",
            mSensorBitOrder,
            this,
            sizeof(VSSScenario));
}


/*******************************************************************************
*
********************************************************************************/
VSSScenario::~VSSScenario()
{
    if( debugScanLine != NULL )
    {
        debugScanLine->destroyInstance();
        debugScanLine = NULL;
    }
    MY_LOGD("");
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::
init()
{
    CAM_TRACE_NAME("VSSScen::init");
    FUNCTION_LOG_START;
    //(1)
    mpCamIOPipe = ICamIOPipe::createInstance(eScenarioID_VSS, mSensorType);
    CAM_TRACE_BEGIN("CamIOPipe::init");
    if ( ! mpCamIOPipe || ! mpCamIOPipe->init())
    {
        MY_LOGE("ICamIOPipe init error");
        return MFALSE;
    }
    CAM_TRACE_END();
    //(2)
    CAM_TRACE_BEGIN("PostProcPipe::init");
    mpPostProcPipe = IPostProcPipe::createInstance(eScenarioID_VSS, mSensorType);
    if ( ! mpPostProcPipe || ! mpPostProcPipe->init())
    {
        MY_LOGE("IPostProcPipe init error");
        CAM_TRACE_END();
        return MFALSE;
    }
    CAM_TRACE_END();
    //(3)
    CAM_TRACE_BEGIN("CamIOPipe::EPIPECmd_SET_CQ_CHANNEL");
    mpExtImgProcHw = ExtImgProcHw::createInstance();
    if(!mpExtImgProcHw || !mpExtImgProcHw->init())
    {
        MY_LOGE("IExtImgProc init error");
        return MFALSE;
    }
    //
    mpXdpPipe = NSCamPipe::IXdpPipe::createInstance(NSCamPipe::eSWScenarioID_VSS, NSCamPipe::eScenarioFmt_YUV);
    if (!mpXdpPipe || !mpXdpPipe->init())
    {
        MY_LOGE("IXdpPipe init error");
        return MFALSE;
    }
    //
    mpIMemDrv = IMemDrv::createInstance();
    if(!mpIMemDrv || !mpIMemDrv->init())
    {
        MY_LOGE("IMemDrv init error");
        return MFALSE;
    }
    //
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS1_CQ0, 0, 0);
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("CamIOPipe::EPIPECmd_SET_CQ_TRIGGER_MODE");
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                            (MINT32)EPIPE_PASS1_CQ0,
                            (MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                            (MINT32)EPIPECQ_TRIG_BY_START);
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("PostProcPipe::EPIPECmd_SET_CQ_CHANNEL");
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,
                               (MINT32)EPIPE_PASS2_CQ1, 0, 0);
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("CamIOPipe::EPIPECmd_SET_CONFIG_STAGE");
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init, 0, 0);
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("PostProcPipe::EPIPECmd_SET_CONFIG_STAGE");
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init, 0, 0);
    CAM_TRACE_END();
    //
    msPass1OutFmt = "UNKNOWN";
    msPass2DispoOutFmt = "UNKNOWN";
    msPass2VidoOutFmt = "UNKNOWN";
    //
    mTwoRunPass2Info.tempBuf.memID = -1;
    mTwoRunPass2Info.tempBuf.virtAddr = 0;
    mTwoRunPass2Info.tempBuf.phyAddr = 0;
    mTwoRunPass2Info.tempBuf.size = 0;
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::uninit()
{
    CAM_TRACE_NAME("VSSScen::uninit");
    FUNCTION_LOG_START;
    //
    MBOOL ret = MTRUE;
    //(1)
    if ( mpCamIOPipe )
    {
        CAM_TRACE_BEGIN("CamIOPipe::uninit");
        if ( ! mpCamIOPipe->uninit())
        {
            MY_LOGE("mpCamIOPipe uninit fail");
            CAM_TRACE_END();
            ret = MFALSE;
        }
        CAM_TRACE_END();
        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
    }
    //(2)
    if ( mpPostProcPipe )
    {
        CAM_TRACE_BEGIN("PostProcPipe::uninit");
        if ( ! mpPostProcPipe->uninit())
        {
            MY_LOGE("mpPostProcPipe uninit fail");
            CAM_TRACE_END();
            ret = MFALSE;
        }
        CAM_TRACE_END();
        mpPostProcPipe->destroyInstance();
        mpPostProcPipe = NULL;
    }
    //
    if(mpExtImgProcHw != NULL)
    {
        mpExtImgProcHw->uninit();
        mpExtImgProcHw->destroyInstance();
        mpExtImgProcHw = NULL;
    }
    //
    if(mpXdpPipe != NULL)
    {
        if (!mpXdpPipe->uninit())
        {
            MY_LOGE("mpXdpPipe uninit fail");
            ret = MFALSE;
        }
        mpXdpPipe->destroyInstance();
        mpXdpPipe = NULL;
    }
    //
    freeTwoRunPass2TempBuf();
    //
    if(mpIMemDrv != NULL)
    {
        mpIMemDrv->uninit();
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
    }
    //
    FUNCTION_LOG_END;
    //
    return ret;
}


/*******************************************************************************
* wait hardware interrupt
********************************************************************************/
MVOID
VSSScenario::wait(EWaitType rType)
{
    CAM_TRACE_NAME("VSSScen::wait");
    switch(rType)
    {
        case eIRQ_VS:
            mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
        break;
        default:
        break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::start()
{
    CAM_TRACE_NAME("VSSScen::start");
    FUNCTION_LOG_START;
    // (1) start CQ
    CAM_TRACE_BEGIN("CamIOPipe::startCQ0");
    mpCamIOPipe->startCQ0();
    CAM_TRACE_END();
#if defined(_PASS1_CQ_CONTINUOUS_MODE_)
    CAM_TRACE_BEGIN("CamIOPipe::EPIPECmd_SET_CQ_TRIGGER_MODE");
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                             (MINT32)EPIPE_PASS1_CQ0,
                             (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                             (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
    CAM_TRACE_END();
#else
    CAM_TRACE_BEGIN("CamIOPipe::EPIPECmd_SET_CQ_CHANNEL");
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_CQ_NONE, 0, 0);
    CAM_TRACE_END();
#endif
    // (2) pass1 start
    CAM_TRACE_BEGIN("CamIOPipe::start");
    if ( ! mpCamIOPipe->start())
    {
        MY_LOGE("mpCamIOPipe->start() fail");
        CAM_TRACE_END();
        return MFALSE;
    }
    CAM_TRACE_END();
    // align to Vsync
    CAM_TRACE_BEGIN("CamIOPipe::irq::EPipePass_PASS1_TG1");
    mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
    CAM_TRACE_END();
    MY_LOGD("- wait IRQ: ISP_DRV_IRQ_INT_STATUS_VS1_ST");
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::stop()
{
    CAM_TRACE_NAME("VSSScen::stop");
    FUNCTION_LOG_START;
    //
    PortID rPortID;
    mapPortCfg(eID_Pass1Out, rPortID);
    PortQTBufInfo dummy(eID_Pass1Out);
    CAM_TRACE_BEGIN("CamIOPipe::dequeOutBuf");
    mpCamIOPipe->dequeOutBuf(rPortID, dummy.bufInfo);
    CAM_TRACE_END();
    //
    CAM_TRACE_BEGIN("CamIOPipe::stop");
    if ( ! mpCamIOPipe->stop())
    {
        MY_LOGE("mpCamIOPipe->stop() fail");
        CAM_TRACE_END();
        return MFALSE;
    }
    CAM_TRACE_END();
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::
setConfig(vector<PortImgInfo> *pImgIn)
{
    if ( ! pImgIn )
    {
        MY_LOGE("pImgIn==NULL");
        return MFALSE;
    }

    bool isPass1 = false;

    defaultSetting();

    for (MUINT32 i = 0; i < pImgIn->size(); i++)
    {
        PortImgInfo rSrc = pImgIn->at(i);
        //
        // Pass 1 config will be fixed. Pass 2 config can be updated later.
        //
        if (rSrc.ePortIdx == eID_Pass1In)
        {
            isPass1 = true;
            mapFormat(rSrc.sFormat, mSettingPorts.tgi.eImgFmt);
            mSettingPorts.tgi.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.tgi.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.tgi.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.tgi.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.tgi.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.tgi.crop.x = rSrc.crop.x;
            mSettingPorts.tgi.crop.y = rSrc.crop.y;
            mSettingPorts.tgi.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.tgi.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.tgi.crop.w = rSrc.crop.w;
            mSettingPorts.tgi.crop.h = rSrc.crop.h;
        }
        else if (rSrc.ePortIdx == eID_Pass1Out)
        {
            msPass1OutFmt = rSrc.sFormat;
            mapFormat(rSrc.sFormat, mSettingPorts.imgo.eImgFmt);
            mSettingPorts.imgo.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.imgo.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.imgo.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.imgo.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.imgo.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.imgo.crop.x = rSrc.crop.x;
            mSettingPorts.imgo.crop.y = rSrc.crop.y;
            mSettingPorts.imgo.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.imgo.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.imgo.crop.w = rSrc.crop.w;
            mSettingPorts.imgo.crop.h = rSrc.crop.h;
        }
        else if (rSrc.ePortIdx == eID_Pass2In)
        {
            mapFormat(rSrc.sFormat, mSettingPorts.imgi.eImgFmt);
            mSettingPorts.imgi.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.imgi.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.imgi.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.imgi.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.imgi.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.imgi.crop.x = rSrc.crop.x;
            mSettingPorts.imgi.crop.y = rSrc.crop.y;
            mSettingPorts.imgi.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.imgi.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.imgi.crop.w = rSrc.crop.w;
            mSettingPorts.imgi.crop.h = rSrc.crop.h;
            //
            if( mbTwoRunRot &&
                calRotation() != eImgRot_180)
            {
                calCrop(
                    mSettingPorts.imgi.u4ImgWidth,
                    mSettingPorts.imgi.u4ImgHeight,
                    mSettingPorts.imgi.crop.w,
                    mSettingPorts.imgi.crop.h,
                    mSettingPorts.imgi.crop.x,
                    mSettingPorts.imgi.crop.y);
            }
        }
        else if (rSrc.ePortIdx == eID_Pass2DISPO)
        {
            msPass2DispoOutFmt = rSrc.sFormat;
            mapFormat(rSrc.sFormat, mSettingPorts.dispo.eImgFmt);
            mSettingPorts.dispo.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.dispo.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.dispo.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.dispo.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.dispo.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.dispo.crop.x = rSrc.crop.x;
            mSettingPorts.dispo.crop.y = rSrc.crop.y;
            mSettingPorts.dispo.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.dispo.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.dispo.crop.w = rSrc.crop.w;
            mSettingPorts.dispo.crop.h = rSrc.crop.h;
            mSettingPorts.dispo.eImgRot = calRotation();
            //
            if(mbTwoRunRot)
            {
                mSettingPorts.dispo.eImgRot = calRotation();
            }
        }
        else if (rSrc.ePortIdx == eID_Pass2VIDO)
        {
            msPass2VidoOutFmt = rSrc.sFormat;
            mapFormat(rSrc.sFormat, mSettingPorts.vido.eImgFmt);
            mSettingPorts.vido.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.vido.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.vido.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.vido.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.vido.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.vido.eImgRot = rSrc.eRotate;
            mSettingPorts.vido.crop.x = rSrc.crop.x;
            mSettingPorts.vido.crop.y = rSrc.crop.y;
            mSettingPorts.vido.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.vido.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.vido.crop.w = rSrc.crop.w;
            mSettingPorts.vido.crop.h = rSrc.crop.h;
            //
            if(mbTwoRunRot)
            {
                mSettingPorts.vido.eImgRot = calRotation(rSrc.eRotate);
            }

        }
        else
        {
            MY_LOGE("Not done yet!!");
        }
    }

    //mSettingPorts.dump();

    if (isPass1)
    {
        // Note:: must to config cameraio pipe before irq
        //        since cameio pipe won't be changed later, do it here
        vector<PortInfo const*> vCamIOInPorts;
        vector<PortInfo const*> vCamIOOutPorts;
        vCamIOInPorts.push_back(&mSettingPorts.tgi);
        vCamIOOutPorts.push_back(&mSettingPorts.imgo);
        CAM_TRACE_BEGIN("VSSScen::CamIOPipe::configPipe");
        mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts);
        CAM_TRACE_END();
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::sDefaultSetting_Ports::
dump()
{
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[TG]:F(%d),W(%d),H(%d),Str(%d)",
        tgi.eImgFmt,
        tgi.u4ImgWidth,
        tgi.u4ImgHeight,
        tgi.u4Stride[ESTRIDE_1ST_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[IMGI]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
        imgi.eImgFmt,
        imgi.u4ImgWidth,
        imgi.u4ImgHeight,
        imgi.u4Stride[ESTRIDE_1ST_PLANE],
        imgi.u4Stride[ESTRIDE_2ND_PLANE],
        imgi.u4Stride[ESTRIDE_3RD_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"[DISPO]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
        dispo.eImgFmt,
        dispo.u4ImgWidth,
        dispo.u4ImgHeight,
        dispo.u4Stride[ESTRIDE_1ST_PLANE],
        dispo.u4Stride[ESTRIDE_2ND_PLANE],
        dispo.u4Stride[ESTRIDE_3RD_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"[VIDO]:F(%d),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
        vido.eImgFmt,
        vido.u4ImgWidth,
        vido.u4ImgHeight,
        vido.u4Stride[ESTRIDE_1ST_PLANE],
        vido.u4Stride[ESTRIDE_2ND_PLANE],
        vido.u4Stride[ESTRIDE_3RD_PLANE],
        vido.eImgRot);
}

/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::
defaultSetting()
{
    CAM_TRACE_NAME("VSSScen::defaultSetting");
    ////////////////////////////////////////////////////////////////////
    //      Pass 1 setting (default)                                  //
    ////////////////////////////////////////////////////////////////////

    // (1.1) tgi
    PortInfo &tgi = mSettingPorts.tgi;
    tgi.eRawPxlID = mSensorBitOrder; //only raw looks this
    tgi.type = EPortType_Sensor;
    tgi.inout  = EPortDirection_In;
    tgi.index = EPortIndex_TG1I;

    // (1.2) imgo
    PortInfo &imgo = mSettingPorts.imgo;
    imgo.type = EPortType_Memory;
    imgo.index = EPortIndex_IMGO;
    imgo.inout  = EPortDirection_Out;


    ////////////////////////////////////////////////////////////////////
    //Pass 2 setting (default)
    ////////////////////////////////////////////////////////////////////

    //(2.1)
    PortInfo &imgi = mSettingPorts.imgi;
    imgi.eImgFmt = eImgFmt_UNKNOWN;
    imgi.type = EPortType_Memory;
    imgi.index = EPortIndex_IMGI;
    imgi.inout = EPortDirection_In;
    imgi.pipePass = EPipePass_PASS2;
    imgi.u4ImgWidth = 0;
    imgi.u4ImgHeight = 0;
    imgi.u4Offset = 0;
    imgi.u4Stride[0] = 0;
    imgi.u4Stride[1] = 0;
    imgi.u4Stride[2] = 0;

    //(2.2)
    PortInfo &dispo = mSettingPorts.dispo;
    dispo.eImgFmt = eImgFmt_UNKNOWN;
    dispo.eImgRot = eImgRot_0;                  //dispo NOT support rotation
    dispo.eImgFlip = eImgFlip_OFF;              //dispo NOT support flip
    dispo.type = EPortType_DISP_RDMA;           //EPortType
    dispo.index = EPortIndex_DISPO;
    dispo.inout  = EPortDirection_Out;
    dispo.u4ImgWidth = 0;
    dispo.u4ImgHeight = 0;
    dispo.u4Offset = 0;
    dispo.u4Stride[0] = 0;
    dispo.u4Stride[1] = 0;
    dispo.u4Stride[2] = 0;

    //(2.3)
    PortInfo &vido = mSettingPorts.vido;
    vido.eImgFmt = eImgFmt_UNKNOWN;
    vido.eImgRot = eImgRot_0;
    vido.eImgFlip = eImgFlip_OFF;
    vido.type = EPortType_VID_RDMA;
    vido.index = EPortIndex_VIDO;
    vido.inout  = EPortDirection_Out;
    vido.u4ImgWidth = 0;
    vido.u4ImgHeight = 0;
    vido.u4Offset = 0;
    vido.u4Stride[0] = 0;
    vido.u4Stride[1] = 0;
    vido.u4Stride[2] = 0;
}


/*******************************************************************************
*  enque:
********************************************************************************/
MBOOL
VSSScenario::
enque(vector<IhwScenario::PortQTBufInfo> const &in)
{
    MY_LOGW_IF(in.size() > 1, "in.size() > 1");  //shouldn't happen
    //
    if(in.size() == 0)
    {
        MY_LOGE("Size is 0");
        return MFALSE;
    }
    //
    vector<IhwScenario::PortBufInfo> vEnBufPass1Out;

    for (MUINT32 i = 0; i < in.at(0).bufInfo.vBufInfo.size(); i++)
    {
        IhwScenario::PortBufInfo one(eID_Pass1Out,
                          in.at(0).bufInfo.vBufInfo.at(i).u4BufVA,
                          in.at(0).bufInfo.vBufInfo.at(i).u4BufPA,
                          in.at(0).bufInfo.vBufInfo.at(i).u4BufSize,
                          in.at(0).bufInfo.vBufInfo.at(i).memID,
                          in.at(0).bufInfo.vBufInfo.at(i).bufSecu,
                          in.at(0).bufInfo.vBufInfo.at(i).bufCohe);

        vEnBufPass1Out.push_back(one);
    };

    enque(NULL, &vEnBufPass1Out);

    return MTRUE;
}


/*******************************************************************************
*  enque:
********************************************************************************/
MBOOL
VSSScenario::
enque( vector<PortBufInfo> *pBufIn, vector<PortBufInfo> *pBufOut)
{
    if ( !pBufIn ) // pass 1
    {
        enquePass1(pBufOut);
    }
    else  // pass 2
    {
        if(mbTwoRunRot)
        {
            enquePass2TwoRunRot(pBufIn, pBufOut);
        }
        else
        {
            if(mbTwoRunPass2)
            {
                enquePass2TwoRunPass2(pBufIn, pBufOut);
            }
            else
            {
                enquePass2(pBufIn,pBufOut);
            }
        }
    }
    return MTRUE;
}


/*******************************************************************************
*  enque Pass 1:
********************************************************************************/
MBOOL
VSSScenario::
enquePass1(vector<PortBufInfo> *pBufOut)
{
    CAM_TRACE_NAME("VSSScen::enquePass1");
    // Note:: can't update config, but address
    //
    MUINT32 size = pBufOut->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufOut->at(i), rPortID, rQbufInfo);
        CAM_TRACE_BEGIN("CamIO enque");
        mpCamIOPipe->enqueOutBuf(rPortID, rQbufInfo);
        CAM_TRACE_END();
        //
        if(size > 1)
        {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1(%d-%d/0x%08X)",
                i,
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1(%d/0x%08X)",
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
    }
    return MTRUE;
}


/*******************************************************************************
*  enque Pass
********************************************************************************/
MBOOL
VSSScenario::
enquePass2(
    vector<PortBufInfo> *pBufIn,
    vector<PortBufInfo> *pBufOut)
{
    CAM_TRACE_NAME("VSSScen::enquePass2");
    //(1)
    MUINT32 size = 0;
    // [pass 2 In]
    vector<PortInfo const*> vPostProcInPorts;
    vPostProcInPorts.push_back(&mSettingPorts.imgi);
    // [pass 2 Out]
    vector<PortInfo const*> vPostProcOutPorts;
    vector<PortID> vPortID;
    vector<QBufInfo> vQbufInfo;
    //
    size = pBufOut->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufOut->at(i), rPortID, rQbufInfo);
        vPortID.push_back(rPortID);
        vQbufInfo.push_back(rQbufInfo);
        //
        if (rPortID.index == EPortIndex_DISPO)
        {
            vPostProcOutPorts.push_back(&mSettingPorts.dispo);
        }
        else if (rPortID.index == EPortIndex_VIDO)
        {
            vPostProcOutPorts.push_back(&mSettingPorts.vido);
        }
    }
    //
    CAM_TRACE_BEGIN("PostProc configPipe");
    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
    CAM_TRACE_END();
    // (2)
    size = pBufIn->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufIn->at(i), rPortID, rQbufInfo);
        CAM_TRACE_BEGIN("PostProc enque in");
        mpPostProcPipe->enqueInBuf(rPortID, rQbufInfo);
        CAM_TRACE_END();
        //
        if(size > 1)
        {
            MY_LOGD_IF(0, "P2I(%d-%d/0x%08X)",
                        i,
                        rQbufInfo.vBufInfo.at(0).memID,
                        rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(0, "P2I(%d/0x%08X)",
                        rQbufInfo.vBufInfo.at(0).memID,
                        rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
    }
    //
    size = vPortID.size();
    for (MUINT32 i = 0; i < size; i++)
    {
        CAM_TRACE_BEGIN("PostProc enque out");
        mpPostProcPipe->enqueOutBuf(vPortID.at(i), vQbufInfo.at(i));
        CAM_TRACE_END();
        //
        if(size > 1)
        {
            MY_LOGD_IF(0, "P2O(%d-%d/0x%08X)",
                        i,
                        vQbufInfo.at(i).vBufInfo.at(0).memID,
                        vQbufInfo.at(i).vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(0, "P2O(%d/0x%08X)",
                        vQbufInfo.at(i).vBufInfo.at(0).memID,
                        vQbufInfo.at(i).vBufInfo.at(0).u4BufVA);
        }
    }
    // revise config to "update" mode after the first configPipe
    CAM_TRACE_BEGIN("PostProc Trigge");
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
    CAM_TRACE_END();
    //
    return MTRUE;
}


/*******************************************************************************
*  enque Pass 2 Two Run Rot:
********************************************************************************/
MBOOL
VSSScenario::
enquePass2TwoRunRot(
    vector<PortBufInfo> *pBufIn,
    vector<PortBufInfo> *pBufOut)
{
    MUINT32 size;
    // [pass 2 In]
    vector<PortInfo const*> vPostProcInPorts;
    vPostProcInPorts.push_back(&mSettingPorts.imgi);
    // [pass 2 Out]
    vector<PortInfo const*> vPostProcOutPorts;
    vector<PortID> vPortID;
    vector<QBufInfo> vQbufInfo;
    //
    size = pBufOut->size();
    if (size == 2)
     {
        MY_LOGD("DISPO&VIDO");
        //
        PortID rPortID;
        // (.1) keep dispo to be used later
        mTwoRunRotInfo.outBuf.vBufInfo.clear();
        //
        int dispo_idx = 0;
        pBufOut->at(0).ePortIndex == eID_Pass2DISPO ? dispo_idx = 0 : 1;
        //
        mapConfig(pBufOut->at(dispo_idx), rPortID, mTwoRunRotInfo.outBuf);
        mapPortCfg(eID_Pass2VIDO, rPortID);
        //
        mTwoRunRotInfo.outPort = mSettingPorts.dispo;
        mTwoRunRotInfo.outPort.type = EPortType_VID_RDMA;
        mTwoRunRotInfo.outPort.index = EPortIndex_VIDO;
        // (.2) use vido right now
        QBufInfo rQbufInfo;
        mapConfig(pBufOut->at(1-dispo_idx), rPortID, rQbufInfo);
        vPortID.push_back(rPortID);
        vQbufInfo.push_back(rQbufInfo);
        //
        vPostProcOutPorts.push_back(&mSettingPorts.vido);
     }
     else
     if (size == 1)
     {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufOut->at(0), rPortID, rQbufInfo);
        // case (2): DISPO
        if (rPortID.index == EPortIndex_DISPO)
        {
            MY_LOGD("DISPO");
            // (.1) revise its port index to vido
            mapPortCfg(eID_Pass2VIDO, rPortID);
            vPortID.push_back(rPortID);
            vQbufInfo.push_back(rQbufInfo);
            // (.2) revise its port index to vido
            mTwoRunRotInfo.outPort = mSettingPorts.dispo;
            mTwoRunRotInfo.outPort.type = EPortType_VID_RDMA;
            mTwoRunRotInfo.outPort.index = EPortIndex_VIDO;
            // (.3)
            vPostProcOutPorts.push_back(&mTwoRunRotInfo.outPort);
        }
        // case (3): VIDO
        else
        if (rPortID.index == EPortIndex_VIDO)
        {
            MY_LOGD("VIDO");
            //
            vPortID.push_back(rPortID);
            vQbufInfo.push_back(rQbufInfo);
            //
            vPostProcOutPorts.push_back(&mSettingPorts.vido);
        }
     }
    //
    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
    //enque pass 2 in
    size = pBufIn->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufIn->at(i), rPortID, rQbufInfo);
        mpPostProcPipe->enqueInBuf(rPortID, rQbufInfo);
        //
        if(size > 1)
        {
            MY_LOGD_IF(1, "P2I(%d-%d/0x%08X)",
                i,
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(1, "P2I(%d/0x%08X)",
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        //
        mTwoRunRotInfo.inBuf = rQbufInfo;
    }
    //enque pass 2 out
    mpPostProcPipe->enqueOutBuf(vPortID.at(0), vQbufInfo.at(0));
    // revise config to "update" mode after the first configPipe
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
    //
    return MTRUE;
}

/*******************************************************************************
*  enque Pass 2 Two Run Pass2:
********************************************************************************/
MBOOL
VSSScenario::
enquePass2TwoRunPass2(
    vector<PortBufInfo> *pBufIn,
    vector<PortBufInfo> *pBufOut)
{
    MUINT32 i,size;
    // [pass 2 In]
    vector<PortInfo const*> vPostProcInPorts;
    vPostProcInPorts.push_back(&mSettingPorts.imgi);
    // [pass 2 Out]
    vector<PortInfo const*> vPostProcOutPorts;
    vector<PortID> vPortID;
    vector<QBufInfo> vQbufInfo;
    //
    mTwoRunPass2Info.dispo.bufInfo.vBufInfo.clear();
    mTwoRunPass2Info.vido.bufInfo.vBufInfo.clear();
    //
    for(i=0; i<pBufOut->size(); i++)
    {
        if(pBufOut->at(i).ePortIndex == eID_Pass2DISPO)
        {
            mapConfig(pBufOut->at(i), mTwoRunPass2Info.dispo.portId, mTwoRunPass2Info.dispo.bufInfo);
            mTwoRunPass2Info.dispo.portInfo = mSettingPorts.dispo;
        }
        else
        if(pBufOut->at(i).ePortIndex == eID_Pass2VIDO)
        {
            mapConfig(pBufOut->at(i), mTwoRunPass2Info.vido.portId, mTwoRunPass2Info.vido.bufInfo);
            mTwoRunPass2Info.vido.portInfo = mSettingPorts.vido;
        }
    }
    //

    if( mTwoRunPass2Info.dispo.bufInfo.vBufInfo.size() > 0 )
    {
        mbTwoRunPass2Dispo = MTRUE;
        vPortID.push_back(mTwoRunPass2Info.dispo.portId);
        vQbufInfo.push_back(mTwoRunPass2Info.dispo.bufInfo);
        mTwoRunPass2Info.tempPort.portInfo = mSettingPorts.dispo;
        size = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufSize;
    }
    else
    {
        mbTwoRunPass2Dispo = MFALSE;
        vPortID.push_back(mTwoRunPass2Info.vido.portId);
        vQbufInfo.push_back(mTwoRunPass2Info.vido.bufInfo);
        mTwoRunPass2Info.tempPort.portInfo= mSettingPorts.vido;
        size = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufSize;
    }
    //
    allocTwoRunPass2TempBuf(size);
    //
    vQbufInfo[0].vBufInfo[0].memID = mTwoRunPass2Info.tempBuf.memID;
    vQbufInfo[0].vBufInfo[0].u4BufVA = mTwoRunPass2Info.tempBuf.virtAddr;
    vQbufInfo[0].vBufInfo[0].u4BufPA = mTwoRunPass2Info.tempBuf.phyAddr;
    //
    mTwoRunPass2Info.tempPort.portId = vPortID[0];
    mTwoRunPass2Info.tempPort.bufInfo = vQbufInfo[0];
    mTwoRunPass2Info.tempPort.portInfo.eImgRot = eImgRot_0;
    vPostProcOutPorts.push_back(&mTwoRunPass2Info.tempPort.portInfo);
    //
    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
    //enque pass 2 in
    size = pBufIn->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufIn->at(i), rPortID, rQbufInfo);
        mpPostProcPipe->enqueInBuf(rPortID, rQbufInfo);
        //
        if(size > 1)
        {
            MY_LOGD_IF(1, "P2I(%d-%d/0x%08X)",
                i,
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(1, "P2I(%d/0x%08X)",
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        //
        mTwoRunRotInfo.inBuf = rQbufInfo;
    }
    //enque pass 2 out
    mpPostProcPipe->enqueOutBuf(vPortID.at(0), vQbufInfo.at(0));

    // revise config to "update" mode after the first configPipe
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
    //
    return MTRUE;
}


/*******************************************************************************
*  deque:
********************************************************************************/
MBOOL
VSSScenario::
deque(
    EHwBufIdx port,
    vector<PortQTBufInfo> *pBufIn)
{
    MBOOL result = MTRUE;
    if ( ! pBufIn )
    {
        MY_LOGE("pBufIn==NULL");
        return MFALSE;
    }

    if ( port == eID_Unknown )
    {
        MY_LOGE("port == eID_Unknown");
        return MFALSE;
    }

    MY_LOGD_IF(0, "+ port(0x%X)",port);

    //(1.1) wait pass 1 done
    if (port & eID_Pass1Out)
    {
        result = dequePass1(port, pBufIn);
    }
    //(1.2) wait pass 2 done
    if ((port & eID_Pass2DISPO) || (port & eID_Pass2VIDO))
    {
        if(mbTwoRunRot)
        {
            result = dequePass2TwoRunRot(port, pBufIn);
        }
        else
        {
            if(mbTwoRunPass2)
            {
                result = dequePass2TwoRunPass2(port, pBufIn);
            }
            else
            {
                result = dequePass2(port, pBufIn);
            }
        }
    }
    MY_LOGD_IF(0, "- port(0x%X)",port);
    //
    return result;
}


/*******************************************************************************
*  deque:
********************************************************************************/
MBOOL
VSSScenario::
dequePass1(
    EHwBufIdx port,
    vector<PortQTBufInfo> *pBufIn)
{
    //for debug scan line
    char propertyValue[PROPERTY_VALUE_MAX] = {'\0'};

    CAM_TRACE_NAME("VSSScen::dequePass1");
    MUINT32 size;
    PortID rPortID;
    mapPortCfg(eID_Pass1Out, rPortID);
    PortQTBufInfo one(eID_Pass1Out);
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQP1");
    //
    CAM_TRACE_BEGIN("CamIOPipe deque");
    if ( ! mpCamIOPipe->dequeOutBuf(rPortID, one.bufInfo))
    {
        MY_LOGE("mpCamIOPipe->dequeOutBuf fail");
        AEE_ASSERT("ISP deque fail:sensor may not output enough data!");
        return MFALSE;
    }
    CAM_TRACE_END();
    //
    pBufIn->push_back(one);
    //
    MY_LOGE_IF(one.bufInfo.vBufInfo.size()==0, "Pass 1 deque without buffer");
    //
    size = one.bufInfo.vBufInfo.size();
    for (MUINT32 i = 0; i < size; i++)
    {
        if(size > 1)
        {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1(%d-%d/0x%08X/0x%08X/%d/%d.%06d)",
                i,
                one.bufInfo.vBufInfo.at(i).memID,
                one.bufInfo.vBufInfo.at(i).u4BufVA,
                one.bufInfo.vBufInfo.at(i).u4BufPA,
                one.bufInfo.vBufInfo.at(i).u4BufSize,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_sec,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_us);
        }
        else
        {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1(%d/0x%08X/0x%08X/%d/%d.%06d)",
                one.bufInfo.vBufInfo.at(i).memID,
                one.bufInfo.vBufInfo.at(i).u4BufVA,
                one.bufInfo.vBufInfo.at(i).u4BufPA,
                one.bufInfo.vBufInfo.at(i).u4BufSize,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_sec,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_us);
        }
        if(one.ePortIndex == eID_Pass1Out)
        {
            if(mpExtImgProcHw != NULL)
            {
                if(mpExtImgProcHw->getImgMask() & ExtImgProcHw::BufType_ISP_VSS_P1_Out)
                {
                    IExtImgProc::ImgInfo img;
                    img.bufType     = ExtImgProcHw::BufType_ISP_VSS_P1_Out;
                    img.format      = msPass1OutFmt;
                    img.width       = mSettingPorts.imgo.u4ImgWidth;
                    img.height      = mSettingPorts.imgo.u4ImgHeight;
                    img.stride[0]   = mSettingPorts.imgo.u4Stride[ESTRIDE_1ST_PLANE];
                    img.stride[1]   = mSettingPorts.imgo.u4Stride[ESTRIDE_2ND_PLANE];
                    img.stride[2]   = mSettingPorts.imgo.u4Stride[ESTRIDE_3RD_PLANE];
                    img.virtAddr    = one.bufInfo.vBufInfo.at(0).u4BufVA;
                    img.bufSize     = one.bufInfo.vBufInfo.at(0).u4BufSize;
                    //
                    mpExtImgProcHw->doImgProc(img);
                }
            }
        }

        //for debug scan line
        property_get( "debug.cam.scanline.P1", propertyValue, "0");
        if( atoi(propertyValue) == 1 && debugScanLine != NULL )
        {
            debugScanLine->drawScanLine(mSettingPorts.imgo.u4ImgWidth,
                                        mSettingPorts.imgo.u4ImgHeight,
                                        (void*)one.bufInfo.vBufInfo.at(i).u4BufVA,
                                        one.bufInfo.vBufInfo.at(i).u4BufSize,
                                        mSettingPorts.imgo.u4Stride[0]);
        }
    }
    //
    return MTRUE;
}


/*******************************************************************************
*  deque Pass 2 Simple:
********************************************************************************/
MBOOL
VSSScenario::
dequePass2(
    EHwBufIdx port,
    vector<PortQTBufInfo> *pBufIn)
{
    //for debug scan line
    char propertyValue[PROPERTY_VALUE_MAX] = {'\0'};
    int32_t dispoBufSize = 0, vidoBufSize = 0;
    void *dispoVirtAddr = 0, *vidoVirtAddr = 0;

    CAM_TRACE_NAME("VSSScen::dequePass2");
    MY_LOGD_IF(0, "SCB");
    //
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
    //
    if (port & eID_Pass2DISPO)
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
    }
    if (port & eID_Pass2VIDO)
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "SR");
    CAM_TRACE_BEGIN("PostProc start");
    mpPostProcPipe->start();
    CAM_TRACE_END();
    MY_LOGD_IF(0, "IRQ");
    mpPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);
    //
    if (port & eID_Pass2DISPO)
    {
        PortID rPortID;
        mapPortCfg(eID_Pass2DISPO, rPortID);
        PortQTBufInfo one(eID_Pass2DISPO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQD");
        CAM_TRACE_BEGIN("PostProc deque disp");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            goto EXIT;
        }
        CAM_TRACE_END();
        pBufIn->push_back(one);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DISPO(%d/0x%08X)",
            one.bufInfo.vBufInfo.at(0).memID,
            one.bufInfo.vBufInfo.at(0).u4BufVA);
        //for debug scan line
        dispoVirtAddr = (void*)one.bufInfo.vBufInfo.at(0).u4BufVA;
        dispoBufSize = one.bufInfo.vBufInfo.at(0).u4BufSize;
    }
    if (port & eID_Pass2VIDO)
    {
        PortID rPortID;
        mapPortCfg(eID_Pass2VIDO, rPortID);
        PortQTBufInfo one(eID_Pass2VIDO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQV");
        CAM_TRACE_BEGIN("PostProc deque vido");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            goto EXIT;
        }
        CAM_TRACE_END();
        pBufIn->push_back(one);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VIDO(%d/0x%08X)",
            one.bufInfo.vBufInfo.at(0).memID,
            one.bufInfo.vBufInfo.at(0).u4BufVA);
        //for debug scan line
        vidoVirtAddr = (void*)one.bufInfo.vBufInfo.at(0).u4BufVA;
        vidoBufSize = one.bufInfo.vBufInfo.at(0).u4BufSize;
    }
    // deque out pass2 in buffer
    {
        PortID rPortID;
        mapPortCfg(eID_Pass2In, rPortID);
        QTimeStampBufInfo dummy;
        MY_LOGD_IF(0, "DQI");
        CAM_TRACE_BEGIN("PostProc deque imgi");
        mpPostProcPipe->dequeInBuf(rPortID, dummy);
        CAM_TRACE_END();
    }
    //
    EXIT:
    MY_LOGD_IF(0, "SP");
    CAM_TRACE_BEGIN("PostProc stop");
    mpPostProcPipe->stop();
    CAM_TRACE_END();
    //for debug scan line
    property_get( "debug.cam.scanline.P2", propertyValue, "0");
    if( atoi(propertyValue) == 1 && debugScanLine != NULL )
    {
        if (port & eID_Pass2DISPO)
        {
            debugScanLine->drawScanLine(mSettingPorts.dispo.u4ImgWidth,
                                        mSettingPorts.dispo.u4ImgHeight,
                                        dispoVirtAddr,
                                        dispoBufSize,
                                        mSettingPorts.dispo.u4Stride[0]);
        }
        if (port & eID_Pass2VIDO)
        {
            debugScanLine->drawScanLine(mSettingPorts.vido.u4ImgWidth,
                                        mSettingPorts.vido.u4ImgHeight,
                                        vidoVirtAddr,
                                        vidoBufSize,
                                        mSettingPorts.vido.u4Stride[0]);
        }
    }
    //
    return MTRUE;
}


/*******************************************************************************
*  deque Pass2 Two Run Rot:
********************************************************************************/
MBOOL
VSSScenario::
dequePass2TwoRunRot(
    EHwBufIdx port,
    vector<PortQTBufInfo> *pBufIn)
{
    MBOOL bTwoRun = MFALSE;
    PortQTBufInfo one(eID_Pass2VIDO);
    //
    MY_LOGD_IF(0, "SCB");
    //
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "SR");
    mpPostProcPipe->start();
    MY_LOGD_IF(0, "IRQ");
    mpPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);
    // case (1): DISPO, VIDO
    if ( (port & eID_Pass2DISPO) && (port & eID_Pass2VIDO))
    {
        MY_LOGD_IF(0, "DISPO&VIDO");
        PortID rPortID;
        // (.1) deque VIDO
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQD");
        mapPortCfg(eID_Pass2VIDO, rPortID);
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            goto EXIT;
        }
        pBufIn->push_back(one);
        // (.2) deque pass2 in buffer
        mapPortCfg(eID_Pass2In, rPortID);
        QTimeStampBufInfo pass2InBuf;
        mpPostProcPipe->dequeInBuf(rPortID, pass2InBuf);
        // (.3) pass 2nd run: pass2-in  --> DISPO buffer (use VIDO port)
        //gInterInfo.inBuf.vBufInfo = pass2InBuf.vBufInfo;
        bTwoRun = MTRUE;
    }
    // case (2): DISPO
    // case (3): VIDO
    else
    {
        MY_LOGD_IF(0, "VIDO");
        // (.1) no matter case 2 or 3, take from vido.
        PortID rPortID;
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQV");
        mapPortCfg(eID_Pass2VIDO, rPortID);
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            goto EXIT;
        }
        pBufIn->push_back(one);
        // (.2) deque pass2 in
        mapPortCfg(eID_Pass2In, rPortID);
        QTimeStampBufInfo dummy;
        mpPostProcPipe->dequeInBuf(rPortID, dummy);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VIDO(%d/0x%08X)",
        one.bufInfo.vBufInfo.at(0).memID,
        one.bufInfo.vBufInfo.at(0).u4BufVA);
    // --[diff]
    EXIT:
    mpPostProcPipe->stop();
    //
    if(bTwoRun)
    {
        PortID rPortID;
        QTimeStampBufInfo dummy;
        vector<PortInfo const*> vPostProcInPorts;
        vector<PortInfo const*> vPostProcOutPorts;
        //
        vPostProcInPorts.push_back(&mSettingPorts.imgi);
        vPostProcOutPorts.push_back(&mTwoRunRotInfo.outPort);
        //
        MY_LOGD_IF(0, "TR:configPipe");
        mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
        //
        MY_LOGD_IF(0, "TR:enqueInBuf");
        mapPortCfg(eID_Pass2In, rPortID);
        mpPostProcPipe->enqueInBuf(rPortID, mTwoRunRotInfo.inBuf);
        MY_LOGD_IF(1, "TR:In(%d/0x%08X)",
            mTwoRunRotInfo.inBuf.vBufInfo.at(0).memID,
            mTwoRunRotInfo.inBuf.vBufInfo.at(0).u4BufVA);
        //
        MY_LOGD_IF(0, "TR:enqueOutBuf");
        mapPortCfg(eID_Pass2VIDO, rPortID);
        mpPostProcPipe->enqueOutBuf(rPortID, mTwoRunRotInfo.outBuf);
        MY_LOGD_IF(1, "TR:Out(%d/0x%08X)",
            mTwoRunRotInfo.outBuf.vBufInfo.at(0).memID,
            mTwoRunRotInfo.outBuf.vBufInfo.at(0).u4BufVA);
        //
        mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "TR:SR");
        mpPostProcPipe->start();
        MY_LOGD_IF(0, "TR:IRQ");
        mpPostProcPipe->irq(EPipePass_PASS2, EPIPEIRQ_PATH_DONE);
        MY_LOGD_IF(0, "TR:DONE");
        //
        mapPortCfg(eID_Pass2VIDO, rPortID);
        PortQTBufInfo out(eID_Pass2VIDO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "TR:DQV");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, out.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            goto EXIT2;
        }
        //
        mapPortCfg(eID_Pass2In, rPortID);
        MY_LOGD_IF(0, "TR:DQI");
        mpPostProcPipe->dequeInBuf(rPortID, dummy);
        //
        pBufIn->push_back(out);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "TR:VIDO(%d/0x%08X)",
            out.bufInfo.vBufInfo.at(0).memID,
            out.bufInfo.vBufInfo.at(0).u4BufVA);
        //
        EXIT2:
        MY_LOGD_IF(0, "TR:SP");
        mpPostProcPipe->stop();
    }
    //
    return MTRUE;
}

/*******************************************************************************
*  deque Pass2 Two Run Pass2:
********************************************************************************/
MBOOL
VSSScenario::
dequePass2TwoRunPass2(
    EHwBufIdx port,
    vector<PortQTBufInfo> *pBufIn)
{
    PortID rPortID;
    vector<NSCamPipe::PortInfo const*> vInPorts;
    vector<NSCamPipe::PortInfo const*> vOutPorts;
    //
    MY_LOGD_IF(0, "SCB");
    //
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
    //
    if(mbTwoRunPass2Dispo)
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
    }
    else
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "SR");
    mpPostProcPipe->start();
    MY_LOGD_IF(0, "IRQ");
    mpPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);
    //
    if(mbTwoRunPass2Dispo)
    {
        mapPortCfg(eID_Pass2DISPO, rPortID);
        PortQTBufInfo dispoBuf(eID_Pass2DISPO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQT");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, dispoBuf.bufInfo))
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            mpPostProcPipe->stop();
            return MFALSE;
        }
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Temp(%d/0x%08X)",
            dispoBuf.bufInfo.vBufInfo.at(0).memID,
            dispoBuf.bufInfo.vBufInfo.at(0).u4BufVA);
    }
    else
    {
        mapPortCfg(eID_Pass2VIDO, rPortID);
        PortQTBufInfo vidoBuf(eID_Pass2VIDO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQT");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, vidoBuf.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            mpPostProcPipe->stop();
            return MFALSE;
        }
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Temp(%d/0x%08X)",
            vidoBuf.bufInfo.vBufInfo.at(0).memID,
            vidoBuf.bufInfo.vBufInfo.at(0).u4BufVA);
    }
    //
    MY_LOGD("add mpPostProcPipe->dequeInBuf(rPortID, dummy)");
    mapPortCfg(eID_Pass2In, rPortID);
    QTimeStampBufInfo dummy;
    mpPostProcPipe->dequeInBuf(rPortID, dummy);

    MY_LOGD_IF(0, "SP");
    mpPostProcPipe->stop();
    //
    mpIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &(mTwoRunPass2Info.tempBuf));

    if(mpExtImgProcHw != NULL)
    {
        if(mpExtImgProcHw->getImgMask() & ExtImgProcHw::BufType_ISP_VSS_P2_TwoRun_In)
        {
            IExtImgProc::ImgInfo img;
            //
            img.bufType     = ExtImgProcHw::BufType_ISP_VSS_P2_TwoRun_In;
            if(mbTwoRunPass2Dispo)
            {
                img.format      = msPass2DispoOutFmt;
            }
            else
            {
                img.format      = msPass2VidoOutFmt;
            }
            img.width       = mTwoRunPass2Info.tempPort.portInfo.u4ImgWidth;
            img.height      = mTwoRunPass2Info.tempPort.portInfo.u4ImgHeight;
            img.stride[0]   = mTwoRunPass2Info.tempPort.portInfo.u4Stride[ESTRIDE_1ST_PLANE];
            img.stride[1]   = mTwoRunPass2Info.tempPort.portInfo.u4Stride[ESTRIDE_2ND_PLANE];
            img.stride[2]   = mTwoRunPass2Info.tempPort.portInfo.u4Stride[ESTRIDE_3RD_PLANE];
            img.virtAddr    = mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufVA;
            img.bufSize     = mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufSize;

            mpExtImgProcHw->doImgProc(img);
        }
    }
    MY_LOGD_IF(0, "2nd:SCB");
    mpXdpPipe->setCallbacks(NULL, NULL, NULL);
    //IMGI
    NSCamPipe::MemoryInPortInfo rMemInPort(
                                    NSCamHW::ImgInfo(
                                        mTwoRunPass2Info.tempPort.portInfo.eImgFmt,
                                        mTwoRunPass2Info.tempPort.portInfo.u4ImgWidth,
                                        mTwoRunPass2Info.tempPort.portInfo.u4ImgHeight),
                                    0,
                                    mTwoRunPass2Info.tempPort.portInfo.u4Stride,
                                    Rect(
                                        mTwoRunPass2Info.tempPort.portInfo.crop.x,
                                        mTwoRunPass2Info.tempPort.portInfo.crop.y,
                                        mTwoRunPass2Info.tempPort.portInfo.crop.w,
                                        mTwoRunPass2Info.tempPort.portInfo.crop.h));
    rMemInPort.u4Stride[0] = mTwoRunPass2Info.tempPort.portInfo.u4Stride[0];
    rMemInPort.u4Stride[1] = mTwoRunPass2Info.tempPort.portInfo.u4Stride[1];
    rMemInPort.u4Stride[2] = mTwoRunPass2Info.tempPort.portInfo.u4Stride[2];
    vInPorts.push_back(&rMemInPort);
    //DISPO
    NSCamPipe::MemoryOutPortInfo rDispPort(
                                    NSCamHW::ImgInfo(
                                        mTwoRunPass2Info.dispo.portInfo.eImgFmt,
                                        mTwoRunPass2Info.dispo.portInfo.u4ImgWidth,
                                        mTwoRunPass2Info.dispo.portInfo.u4ImgHeight),
                                    mTwoRunPass2Info.dispo.portInfo.u4Stride,
                                    0,
                                    0);
    rDispPort.u4Stride[0] = mTwoRunPass2Info.dispo.portInfo.u4Stride[0];
    rDispPort.u4Stride[1] = mTwoRunPass2Info.dispo.portInfo.u4Stride[1];
    rDispPort.u4Stride[2] = mTwoRunPass2Info.dispo.portInfo.u4Stride[2];
    rDispPort.index = 0;
    if(port & eID_Pass2DISPO)
    {
        vOutPorts.push_back(&rDispPort);
    }
    //VIDO
    NSCamPipe::MemoryOutPortInfo rVdoPort(
                                    NSCamHW::ImgInfo(
                                        mTwoRunPass2Info.vido.portInfo.eImgFmt,
                                        mTwoRunPass2Info.vido.portInfo.u4ImgWidth,
                                        mTwoRunPass2Info.vido.portInfo.u4ImgHeight),
                                    mTwoRunPass2Info.vido.portInfo.u4Stride,
                                    mTwoRunPass2Info.vido.portInfo.eImgRot,
                                    0);
    rVdoPort.u4Stride[0] = mTwoRunPass2Info.vido.portInfo.u4Stride[0];
    rVdoPort.u4Stride[1] = mTwoRunPass2Info.vido.portInfo.u4Stride[1];
    rVdoPort.u4Stride[2] = mTwoRunPass2Info.vido.portInfo.u4Stride[2];
    rVdoPort.index = 1;
    if(port & eID_Pass2VIDO)
    {
        vOutPorts.push_back(&rVdoPort);
    }
    //
    mpXdpPipe->configPipe(
                    vInPorts,
                    vOutPorts);
    //IMGI
    MY_LOGD_IF(0,"2nd:EQI");
    NSCamPipe::QBufInfo rInQBuf;
    NSCamHW::BufInfo rInBufInfo(
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufSize,
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufVA,
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufPA,
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].memID);
    rInQBuf.vBufInfo.push_back(rInBufInfo);
    mpXdpPipe->enqueBuf(
                NSCamPipe::PortID(
                    NSCamPipe::EPortType_MemoryIn,
                    0,
                    0),
                rInQBuf);
    //DISPO
    NSCamPipe::QBufInfo rDispQBuf;
    NSCamHW::BufInfo rDispBufInfo;
    if(port & eID_Pass2DISPO)
    {
        rDispBufInfo.u4BufSize  = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufSize;
        rDispBufInfo.u4BufVA    = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufVA;
        rDispBufInfo.u4BufPA    = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufPA;
        rDispBufInfo.i4MemID    = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].memID;
        rDispQBuf.vBufInfo.push_back(rDispBufInfo);
        MY_LOGD_IF(0,"2nd:EQD");
        mpXdpPipe->enqueBuf(
                    NSCamPipe::PortID(
                        NSCamPipe::EPortType_MemoryOut,
                        0,
                        1),
                    rDispQBuf);
    }
    //VIDO
    NSCamPipe::QBufInfo rVdoQBuf;
    NSCamHW::BufInfo rVdoBufInfo;
    if(port & eID_Pass2VIDO)
    {
        rVdoBufInfo.u4BufSize   = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufSize;
        rVdoBufInfo.u4BufVA     = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufVA;
        rVdoBufInfo.u4BufPA     = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufPA;
        rVdoBufInfo.i4MemID     = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].memID;
        rVdoQBuf.vBufInfo.push_back(rVdoBufInfo);
        MY_LOGD_IF(0,"2nd:EQV");
        mpXdpPipe->enqueBuf(
                    NSCamPipe::PortID(
                        NSCamPipe::EPortType_MemoryOut,
                        1,
                        1),
                    rVdoQBuf);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "2nd:SR");

    mpIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &(mTwoRunPass2Info.tempBuf));
    //
    mpXdpPipe->start();
    //
    if(port & eID_Pass2DISPO)
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"2nd:DQD");
        NSCamPipe::QTimeStampBufInfo rQDispOutBuf;
        if(mpXdpPipe->dequeBuf(NSCamPipe::PortID(NSCamPipe::EPortType_MemoryOut,0,1),rQDispOutBuf))
        {
            PortQTBufInfo one(eID_Pass2DISPO);
            NSIspio::BufInfo dispoBufInfo;
            //
            dispoBufInfo.u4BufSize          = rQDispOutBuf.vBufInfo[0].u4BufSize;
            dispoBufInfo.u4BufVA            = rQDispOutBuf.vBufInfo[0].u4BufVA;
            dispoBufInfo.u4BufPA            = rQDispOutBuf.vBufInfo[0].u4BufPA;
            dispoBufInfo.memID              = rQDispOutBuf.vBufInfo[0].i4MemID;
            dispoBufInfo.bufSecu            = rQDispOutBuf.vBufInfo[0].i4BufSecu;
            dispoBufInfo.bufCohe            = rQDispOutBuf.vBufInfo[0].i4BufCohe;
            dispoBufInfo.i4TimeStamp_sec    = 0;
            dispoBufInfo.i4TimeStamp_us     = 0;
            //
            one.bufInfo.i4TimeStamp_sec = rQDispOutBuf.i4TimeStamp_sec;
            one.bufInfo.i4TimeStamp_us  = rQDispOutBuf.i4TimeStamp_us;
            one.bufInfo.u4User          = rQDispOutBuf.u4User;
            one.bufInfo.u4Reserved      = rQDispOutBuf.u4Reserved;
            one.bufInfo.u4BufIndex      = 0;
            one.bufInfo.vBufInfo.push_back(dispoBufInfo);
            //
            pBufIn->push_back(one);
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DISPO(%d/0x%08X)",
                one.bufInfo.vBufInfo.at(0).memID,
                one.bufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            mpXdpPipe->stop();
            return MFALSE;
        }
    }
    //
    if(port & eID_Pass2VIDO)
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"2nd:DQV");
        NSCamPipe::QTimeStampBufInfo rQVdoOutBuf;
        if(mpXdpPipe->dequeBuf(NSCamPipe::PortID(NSCamPipe::EPortType_MemoryOut,1,1),rQVdoOutBuf))
        {
            PortQTBufInfo one(eID_Pass2VIDO);
            NSIspio::BufInfo vidoBufInfo;
            //
            vidoBufInfo.u4BufSize       = rQVdoOutBuf.vBufInfo[0].u4BufSize;
            vidoBufInfo.u4BufVA         = rQVdoOutBuf.vBufInfo[0].u4BufVA;
            vidoBufInfo.u4BufPA         = rQVdoOutBuf.vBufInfo[0].u4BufPA;
            vidoBufInfo.memID           = rQVdoOutBuf.vBufInfo[0].i4MemID;
            vidoBufInfo.bufSecu         = rQVdoOutBuf.vBufInfo[0].i4BufSecu;
            vidoBufInfo.bufCohe         = rQVdoOutBuf.vBufInfo[0].i4BufCohe;
            vidoBufInfo.i4TimeStamp_sec = 0;
            vidoBufInfo.i4TimeStamp_us  = 0;
            //
            one.bufInfo.i4TimeStamp_sec = rQVdoOutBuf.i4TimeStamp_sec;
            one.bufInfo.i4TimeStamp_us  = rQVdoOutBuf.i4TimeStamp_us;
            one.bufInfo.u4User          = rQVdoOutBuf.u4User;
            one.bufInfo.u4Reserved      = rQVdoOutBuf.u4Reserved;
            one.bufInfo.u4BufIndex      = 0;
            one.bufInfo.vBufInfo.push_back(vidoBufInfo);
            //
            pBufIn->push_back(one);
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VIDO(%d/0x%08X)",
                one.bufInfo.vBufInfo.at(0).memID,
                one.bufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            mpXdpPipe->stop();
            return MFALSE;
        }
    }
    //
    MY_LOGD_IF(0,"2nd:DQI");
    NSCamPipe::QTimeStampBufInfo rQInBuf;
    mpXdpPipe->dequeBuf(
                    NSCamPipe::PortID(
                        NSCamPipe::EPortType_MemoryIn,
                        0,
                        0),
                    rQInBuf);
    //
    MY_LOGD_IF(0,"2nd:SP");
    mpXdpPipe->stop();

    mpIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &(mTwoRunPass2Info.tempBuf));
    //
    return MTRUE;
}




/*******************************************************************************
*  replaceQue:
********************************************************************************/
MBOOL
VSSScenario::
replaceQue(vector<PortBufInfo> *pBufOld, vector<PortBufInfo> *pBufNew)
{
    PortID rPortID;
    QBufInfo rQbufInfo;
    //
    mapConfig(pBufOld->at(0), rPortID, rQbufInfo);
    mapConfig(pBufNew->at(0), rPortID, rQbufInfo);
    mpCamIOPipe->enqueOutBuf(rPortID, rQbufInfo);
    //
    MY_LOGD("P1:Old(%d/0x%08X),New(%d/0x%08X)",
            rQbufInfo.vBufInfo.at(0).memID,
            rQbufInfo.vBufInfo.at(0).u4BufVA,
            rQbufInfo.vBufInfo.at(1).memID,
            rQbufInfo.vBufInfo.at(1).u4BufVA);
    //
    return MTRUE;
}


/******************************************************************************
* This is used to check whether width or height exceed limitations of HW.
*******************************************************************************/
#define VSS_HW_LIMIT_LINE_BUF      (3264)
#define VSS_HW_LIMIT_FRAME_PIXEL   (3264*1836)
//
MVOID
VSSScenario::
getHwValidSize(MUINT32 &width, MUINT32 &height)
{
    MY_LOGD("In:W(%d),H(%d)",width,height);
    //
    if(width > VSS_HW_LIMIT_LINE_BUF)
    {
        MY_LOGW("W(%d) is larger than limitation(%d)",
                width,
                VSS_HW_LIMIT_LINE_BUF);
        width = VSS_HW_LIMIT_LINE_BUF;
        MY_LOGW("W is forced to set %d",width);
    }
    //
    if((width * height) > VSS_HW_LIMIT_FRAME_PIXEL)
    {
        MY_LOGW("Frame pixel(%d x %d = %d) is larger than limitation(%d)",
                width,
                height,
                (width * height),
                VSS_HW_LIMIT_FRAME_PIXEL);
        height = (width*9/16)&(~0x1);
        MY_LOGW("H is forced to set %d in 16:9 ratio",height);
    }
    //
    MY_LOGD("Out:W(%d),H(%d)",width,height);
}


/*******************************************************************************
*  calRotation:
*  need to crop frame buffer from 4:3 rectangle to be 3:4
********************************************************************************/
MVOID
VSSScenario::
calCrop(
    MUINT32     srcW,
    MUINT32     SrcH,
    MUINT32&    rCropW,
    MUINT32&    rCropH,
    MUINT32&    rCropX,
    MUINT32&    rCropY)
{
    MUINT32 cropW,cropH;
    MUINT32 cropX,cropY;
    MUINT32 ratio_w = 100 * (float)srcW/rCropW;
    MUINT32 ratio_h = 100 * (float)SrcH/rCropH;
    MUINT32 ratio_h_w = 100 * (float)rCropH/rCropW;
    MUINT32 ratio_zoom = ratio_w < ratio_h ? ratio_w : ratio_h;
    //
    MY_LOGD_IF(1,"In:Src(%d/%d),Crop(%d,%d,%d,%d)",
        srcW,
        SrcH,
        rCropX,
        rCropY,
        rCropW,
        rCropH);
    MY_LOGD_IF(0,"R(%d/%d),H/W(%d),RZ(%d)",
        ratio_w,
        ratio_h,
        ratio_h_w,
        ratio_zoom);
    //
    cropH = SrcH;
    cropW = cropH * ratio_h_w;
    //
    cropH = ROUND_TO_2X(100 * cropH/ratio_zoom);
    cropW = ROUND_TO_2X(cropW/ratio_zoom);
    //
    cropX = (srcW - cropW) / 2;
    cropY = (SrcH - cropH) / 2;
    //
    rCropW = cropW;
    rCropH = cropH;
    rCropX = cropX;
    rCropY = cropY;
    //
    MY_LOGD_IF(1,"Out:Crop(%d,%d,%d,%d)",
        rCropW,
        rCropH,
        rCropX,
        rCropY);
}


/*******************************************************************************
*  calRotation:
********************************************************************************/
EImageRotation
VSSScenario::
calRotation(EImageRotation rot)
{
    MUINT32 rotation  = rot == eImgRot_0 ? 0
                        : rot == eImgRot_90 ? 270
                        : rot == eImgRot_180 ? 180 : 90;
    // no matter facing
    MUINT32 diff = (android::MtkCamUtils::DevMetaInfo::queryDeviceWantedOrientation(mSensorId)-
                android::MtkCamUtils::DevMetaInfo::queryDeviceSetupOrientation(mSensorId)+ rotation + 360 ) % 360;
    //
    switch (diff)
    {
        case 270:
            return eImgRot_90;
        case 180:
            return eImgRot_180;
        case 90:
            return eImgRot_270;
        default:
            return eImgRot_0;
    }
    //
    return eImgRot_0;
}



/*******************************************************************************
*  enable2RunPass2:
********************************************************************************/
MVOID
VSSScenario::
enableTwoRunPass2(MBOOL en)
{
    MY_LOGD("en(%d)",en);
    mbTwoRunPass2 = en;
}



/*******************************************************************************
*  allocTwoRunPass2TempBuf:
********************************************************************************/
MBOOL
VSSScenario::
allocTwoRunPass2TempBuf(MUINT32 bufSize)
{
    MBOOL result = MTRUE;
    //
    if(bufSize == 0)
    {
        MY_LOGE("bufSize is 0");
        return MFALSE;
    }
    //
    if(mTwoRunPass2Info.tempBuf.size == bufSize)
    {
        //MY_LOGD("Buffer is allcoted already!");
        return MFALSE;
    }
    else
    if(mTwoRunPass2Info.tempBuf.size != 0)
    {
        MY_LOGW("re-allcote %d --> %d",mTwoRunPass2Info.tempBuf.size,bufSize);
        freeTwoRunPass2TempBuf();
    }
    //
    mTwoRunPass2Info.tempBuf.size = bufSize;
    //
    if(mpIMemDrv->allocVirtBuf(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->allocVirtBuf() error");
        result = MFALSE;
        goto EXIT;
    }
    if(mpIMemDrv->mapPhyAddr(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
        result = MFALSE;
        goto EXIT;
    }
    //
    MY_LOGD("Temp(%d,0x%08X/0x%08X,%d)",
            mTwoRunPass2Info.tempBuf.memID,
            mTwoRunPass2Info.tempBuf.virtAddr,
            mTwoRunPass2Info.tempBuf.phyAddr,
            mTwoRunPass2Info.tempBuf.size);
    //
    EXIT:
    return result;
}


/*******************************************************************************
*  freeTwoRunPass2TempBuf:
********************************************************************************/
MBOOL
VSSScenario::
freeTwoRunPass2TempBuf()
{
    MBOOL result = MTRUE;
    //
    if(mTwoRunPass2Info.tempBuf.size == 0)
    {
        MY_LOGW("Buffer is free already!");
        return MFALSE;
    }
    //
    MY_LOGD("Temp(%d,0x%08X/0x%08X,%d)",
            mTwoRunPass2Info.tempBuf.memID,
            mTwoRunPass2Info.tempBuf.virtAddr,
            mTwoRunPass2Info.tempBuf.phyAddr,
            mTwoRunPass2Info.tempBuf.size);
    //
    if(mpIMemDrv->unmapPhyAddr(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->unmapPhyAddr() error");
        result = MFALSE;
        goto EXIT;
    }
    if(mpIMemDrv->freeVirtBuf(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->freeVirtBuf() error");
        result = MFALSE;
        goto EXIT;
    }
    //
    mTwoRunPass2Info.tempBuf.size = 0;
    //
    EXIT:
    return result;
}
