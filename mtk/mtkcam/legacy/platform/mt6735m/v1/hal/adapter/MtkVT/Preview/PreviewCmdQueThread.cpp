/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/PrvCQT"

#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
#include <adapter/inc/ImgBufProvidersManager.h>
//
#include <utils/List.h>
#include <vector>
using namespace std;
//
#include <inc/IState.h>
#include <inc/PreviewCmdQueThread.h>
using namespace android::NSMtkVTCamAdapter;
//
#include <mtkcam/featureio/eis_hal_base.h>
#include <mtkcam/drv/res_mgr_drv.h>
//
#include <mtkcam/hal/aaa_hal_base.h>
using namespace NS3A;
#include <mtkcam/hal/IHalSensor.h>
#include <kd_imgsensor_define.h>
//
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>
#include <mtkcam/imageio/ispio_stddef.h>
using namespace NSImageio::NSIspio;
#include <mtkcam/v1/hwscenario/IhwScenario.h>
//
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
#include <cutils/atomic.h>

#include <camera_custom_vt.h>
//
#define DUMP
#ifdef DUMP
#include <cutils/properties.h>
#endif
#define ENABLE_LOG_PER_FRAME        (1)
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN               MY_LOGD("+")
#define FUNCTION_OUT              MY_LOGD("-")

#define ROUND_TO_2X(x) ((x) & (~0x1))
/******************************************************************************
*
*******************************************************************************/
static IhwScenario::Rect_t calCrop(IhwScenario::Rect_t const &rSrc,
                                   IhwScenario::Rect_t const &rDst,
                                   uint32_t ratio = 100);
/******************************************************************************
*
*******************************************************************************/
static void mapNode2BufInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortBufInfo &dst);
static void mapNode2ImgInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortImgInfo &dst, camera_info minfo);
static void mapNode2ImgInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortImgInfo &dst);
static bool mapQT2BufInfo(EHwBufIdx ePort, vector<IhwScenario::PortQTBufInfo> const &src, vector<IhwScenario::PortBufInfo> &dst);
static bool dumpBuffer(vector<IhwScenario::PortQTBufInfo> &src, char const*const tag, char const * const filetype, uint32_t filenum, int flag);
static bool dumpImg(MUINT8 *addr, MUINT32 size, char const * const tag, char const * const filetype, uint32_t filenum, int flag);
/******************************************************************************
*
*******************************************************************************/

namespace android {
namespace NSMtkVTCamAdapter {

/******************************************************************************
*
******************************************************************************/
struct globalInfo{
    globalInfo()
        : openId(-1)
    {}
    int32_t openId;
} gInfo;

/******************************************************************************
*
******************************************************************************/
class sensorInfo{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Construction interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    sensorInfo()
        : meSensorDev(SENSOR_DEV_NONE)
        , mu4TgOutW(0)
        , mu4TgOutH(0)
        , mu4MemOutW(0)
        , mu4MemOutH(0)
        , mpSensorHal(NULL)
        , mpResMgrDrv(NULL)
    {}

    bool init(uint32_t scenarioId)
    {
        CAM_TRACE_NAME("sensorInfo::init");
        //(1) init
        mpResMgrDrv = ResMgrDrv::CreateInstance();
        if(mpResMgrDrv)
        {
            mpResMgrDrv->Init();
        }

        //(2) main or sub
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        if(pHalSensorList != NULL)
        {
            meSensorDev = pHalSensorList->querySensorDevIdx(gInfo.openId);
            pHalSensorList->querySensorStaticInfo(
                                meSensorDev,
                                &mSensorStaticInfo);
            mpSensorHal = pHalSensorList->createSensor(
                                            LOG_TAG,
                                            gInfo.openId);
            if (NULL == mpSensorHal)
            {
                MY_LOGE("Null sensorHal object");
                return MFALSE;
            }
        }
        else
        {
            MY_LOGE("pHalSensorList == NULL");
            return false;
        }

        //(4) tg/mem size
        uint32_t  u4TgInW = 0;
        uint32_t  u4TgInH = 0;
        switch (scenarioId)
        {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            {
                u4TgInW = mSensorStaticInfo.previewWidth;
                u4TgInH = mSensorStaticInfo.previewHeight;
                if(mpResMgrDrv)
                {
                    mpResMgrDrv->setItem(
                                    RES_MGR_DRV_ITEM_SENSOR_FPS,
                                    mSensorStaticInfo.previewFrameRate/10);
                }
                break;
            }
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            {
                u4TgInW = mSensorStaticInfo.videoWidth;
                u4TgInH = mSensorStaticInfo.videoHeight;
                if(mpResMgrDrv)
                {
                    mpResMgrDrv->setItem(
                                    RES_MGR_DRV_ITEM_SENSOR_FPS,
                                    mSensorStaticInfo.videoFrameRate/10);
                }
                break;
            }
            default:
                break;
        }
        //
        if( !( u4TgInW != 0 && u4TgInH != 0 ) )
        {
            return false;
        }
        //
        if(mpResMgrDrv)
        {
            mpResMgrDrv->setItem(
                            RES_MGR_DRV_ITEM_SENSOR_SIZE,
                            u4TgInW*u4TgInH);
            mpResMgrDrv->triggerCtrl();
        }
        //
        mu4TgOutW = ROUND_TO_2X(u4TgInW);  // in case sensor returns odd weight
        mu4TgOutH = ROUND_TO_2X(u4TgInH);  // in case senosr returns odd height
        mu4MemOutW = mu4TgOutW;
        mu4MemOutH = mu4TgOutH;
        //
        ERawPxlID bitorder;
        switch(mSensorStaticInfo.sensorFormatOrder)
        {
            case SENSOR_FORMAT_ORDER_RAW_B:
            {
                bitorder = ERawPxlID_B;
                break;
            }
            case SENSOR_FORMAT_ORDER_RAW_Gb:
            {
                bitorder = ERawPxlID_Gb;
                break;
            }
            case SENSOR_FORMAT_ORDER_RAW_Gr:
            {
                bitorder = ERawPxlID_Gr;
                break;
            }
            case SENSOR_FORMAT_ORDER_RAW_R:
            {
                bitorder = ERawPxlID_R;
                break;
            }
        }

        IhwScenario* pHwScenario = IhwScenario::createInstance(
                                                    eHW_VSS,
                                                    mSensorStaticInfo.sensorType,
                                                    meSensorDev,
                                                    bitorder);
        pHwScenario->getHwValidSize(mu4MemOutW,mu4MemOutH);
        pHwScenario->destroyInstance();
        pHwScenario = NULL;
        //
        IHalSensor::ConfigParam configParam[2];

        configParam[0] =
        {
            index               : gInfo.openId,
            crop                : MSize(u4TgInW, u4TgInH),
            scenarioId          : scenarioId,
            isBypassScenario    : 0,
            isContinuous        : 1,
            HDRMode             : 0,
            framerate           : 0, //default frame rate
            twopixelOn          : 0,
            debugMode           : 0,
        };
        mpSensorHal->configure(1, &configParam[0]);
        //
        //(5) format
        if(mSensorStaticInfo.sensorType == SENSOR_TYPE_RAW)  // RAW
        {
            switch(mSensorStaticInfo.rawSensorBit)
            {
                case 8 :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_BAYER8;
                break;
                case 10 :
                default :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_BAYER10;
                break;
            }
        }
        else if (mSensorStaticInfo.sensorType == SENSOR_TYPE_YUV){
            switch(mSensorStaticInfo.sensorFormatOrder)
            {
                case SENSOR_FORMAT_ORDER_UYVY :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I_UYVY;
                    break;
                case SENSOR_FORMAT_ORDER_VYUY :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I_VYUY;
                    break;
                case SENSOR_FORMAT_ORDER_YVYU :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I_YVYU;
                    break;
                case SENSOR_FORMAT_ORDER_YUYV :
                default :
                    mFormat = CameraParameters::PIXEL_FORMAT_YUV422I;
                    break;
            }
        }
        else {
            MY_LOGE("Unknown sensor type: %d", mSensorStaticInfo.sensorType);
        }
        MY_LOGD("meSensorDev(%d), meSensorType(%d), mSensorBitOrder(%d), mFormat(%s)",
                meSensorDev,
                mSensorStaticInfo.sensorType,
                mSensorStaticInfo.sensorFormatOrder,
                mFormat);
        return true;
    }
    //
    bool uninit()
    {
        CAM_TRACE_NAME("sensorInfo::uninit");
        if(mpSensorHal) {
            mpSensorHal->destroyInstance();
            mpSensorHal = NULL;
        }
        //
        if(mpResMgrDrv)
        {
            mpResMgrDrv->setItem(
                            RES_MGR_DRV_ITEM_SENSOR_FPS,
                            0);
            mpResMgrDrv->setItem(
                            RES_MGR_DRV_ITEM_SENSOR_SIZE,
                            0);
            mpResMgrDrv->triggerCtrl();
            mpResMgrDrv->Uninit();
            mpResMgrDrv->DestroyInstance();
            mpResMgrDrv = NULL;
        }
        return true;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Member query interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    uint32_t            getSensorType()     const   { return mSensorStaticInfo.sensorType; }
    uint32_t            getImgWidth()       const   { return mu4MemOutW; }
    uint32_t            getImgHeight()      const   { return mu4MemOutH; }
    uint32_t            getSensorWidth()    const   { return mu4TgOutW; }
    uint32_t            getSensorHeight()   const   { return mu4TgOutH; }
    const char*         getImgFormat()      const   { return mFormat;}
    uint32_t            getDelayFrame(int32_t mode) const
    {
        switch(mode)
        {
            default:
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            {
                return mSensorStaticInfo.previewDelayFrame;
            }
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            {
                return mSensorStaticInfo.captureDelayFrame;
            }
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            {
                return mSensorStaticInfo.videoDelayFrame;
            }
        }
    }

    uint32_t            getImgWidthStride(uint_t const uPlaneIndex = 0) const
    {
        return FmtUtils::queryImgWidthStride(mFormat, getImgWidth(), uPlaneIndex);
    }

public:
    bool                isYUV()             const   { return mSensorStaticInfo.sensorType == SENSOR_TYPE_YUV
                                                                            ? true : false; }
    bool                isSub()             const   { return meSensorDev == SENSOR_DEV_SUB
                                                                            ? true : false; }
    void                reset()
    {
        //Not support for IHalSensor
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Member variable
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    uint32_t                                meSensorDev;
    uint32_t                                mu4TgOutW;
    uint32_t                                mu4TgOutH;
    uint32_t                                mu4MemOutW;
    uint32_t                                mu4MemOutH;
    IHalSensor*                             mpSensorHal;
    NSCam::SensorStaticInfo                 mSensorStaticInfo;
    char const*                             mFormat;
    ResMgrDrv*                              mpResMgrDrv;
};


/******************************************************************************
*
*******************************************************************************/
#define PASS1BUFCNT      (3)

class PreviewCmdQueThread : public IPreviewCmdQueThread
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Basic Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();


public:
    static PreviewCmdQueThread* getInstance(sp<IPreviewBufMgrHandler> pHandler,
                                    int32_t const & rSensorid,
                                    sp<IParamsManager> pParamsMgr);

    virtual             ~PreviewCmdQueThread();

    virtual bool        setParameters();
    virtual bool        setZoom(uint32_t zoomValue);

protected:
                        PreviewCmdQueThread(sp<IPreviewBufMgrHandler> pHandler,
                                int32_t const & rSensorid,
                                sp<IParamsManager> pParamsMgr);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public to IPreviewCmdQueThread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual int32_t     getTid()        const   { return mi4Tid; }
    virtual bool        isExitPending() const   { return exitPending(); }
    virtual bool        postCommand(PrvCmdCookie::ECmdType const rcmdType,
                                    PrvCmdCookie::ESemWait const rSemWait = PrvCmdCookie::eSemNone);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Detail operation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    bool                start();
    bool                delay(EQueryType_T type);
    bool                update();
    bool                checkIfDropFrame();
    bool                updateOne();
    bool                updateCheck();
    bool                stop();
    bool                precap();
    void                updateZoom(vector<IhwScenario::PortImgInfo> &pImgIn);
    uint32_t            getZoom();
    bool                dropFrame();
    IhwScenario::Rect_t doCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio = 100);
    void                getCfg(int32_t port, vector<IhwScenario::PortImgInfo> &rvImginfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command-related
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    bool                getCommand(sp<PrvCmdCookie> &rCmd);
    bool                isNextCommand();
    List< sp<PrvCmdCookie> > mCmdCookieQ;
    Mutex               mCmdMtx;
    Condition           mCmdCond;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  other modules
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    IhwScenario*                 getHw()        const    { return mpHwScenario; }
    sp<IParamsManager> const     getParamsMgr() const    { return mspParamsMgr; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    Hal3ABase*                  mp3AHal;
    IhwScenario*                mpHwScenario;
    sp<IPreviewBufMgrHandler>   mspPreviewBufHandler;
    sp<IParamsManager>          mspParamsMgr;
    sensorInfo                  mSensorInfo;

    int32_t                     mi4Tid;
    int32_t                     mbAWBIndicator;
    int32_t                     mbEFFECTIndicator;
    int32_t                     mbEvIndicator;
    int32_t                     mbSatIndicator;
    int32_t                     mbBrightIndicator;
    int32_t                     mbContrastIndicator;
    //
    #define ZOOM_SKIP_STEP      (2)
    Mutex                       mZoomMtx;
    Vector<uint32_t>            mvZoomRatio;
    uint32_t                    mCurZoomValue;
    uint32_t                    mFrameCnt;
    //
    EisHalBase*                 mpEisHal;
    //
    //for VT
    bool                        mRotation;        //For 90,270 rotation only
    camera_info                 cameraInfo;
    //
};

}; // namespace NSMtkVTCamAdapter
}; // namespace android
/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread::PreviewCmdQueThread(sp<IPreviewBufMgrHandler> pHandler,
                                         int32_t const & rSensorid,
                                         sp<IParamsManager> pParamsMgr)
    : mpHwScenario(NULL)
    , mspPreviewBufHandler(pHandler)
    , mspParamsMgr(pParamsMgr)
    , mSensorInfo()
    , mi4Tid(0)
    , mbAWBIndicator(0)
    , mbEFFECTIndicator(0)
    , mbEvIndicator(0)
    , mbSatIndicator(0)
    , mbBrightIndicator(0)
    , mbContrastIndicator(0)
    , mZoomMtx()
    , mvZoomRatio()
    , mpEisHal(NULL)
    , mRotation(0)
    , cameraInfo()
{
    gInfo.openId = rSensorid;
}


/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread::~PreviewCmdQueThread()
{
    MY_LOGD("this=%p, sizeof:%d", this, sizeof(PreviewCmdQueThread));
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::requestExit()
{
    FUNCTION_IN;
    bool isIdle =  IStateManager::inst()->isState(IState::eState_Idle);
    if  ( !isIdle )
    {
        MY_LOGW("stop preview before exiting preview thread.");
        postCommand(PrvCmdCookie::eStop, PrvCmdCookie::eSemAfter);
    }
    //
    Thread::requestExit();
    postCommand(PrvCmdCookie::eExit);
    mCmdCond.broadcast();
    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
status_t
PreviewCmdQueThread::readyToRun()
{
    FUNCTION_IN;
    //
    // (1) set thread name
    ::prctl(PR_SET_NAME,"PreviewCmdQueThread", 0, 0, 0);

    // (2) set thread priority
    // [!!]Priority RR?
    int32_t const policy = SCHED_OTHER;
    int32_t const priority = NICE_CAMERA_PASS1;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    //test
    mi4Tid = ::gettid();
    ::sched_getparam(0, &sched_p);
    MY_LOGD(
        "Tid: %d, policy: %d, priority: %d"
        , mi4Tid, ::sched_getscheduler(0)
        , sched_p.sched_priority
    );
    //
    mFrameCnt = 0;
    //
    FUNCTION_OUT;
    //
    return NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::threadLoop()
{
    FUNCTION_IN;
    //
    bool ret = true;
    //
    sp<PrvCmdCookie> pCmdCookie;
    //
    if (getCommand(pCmdCookie))
    {
        if(pCmdCookie != 0)
        {
            pCmdCookie->postSem(PrvCmdCookie::eSemBefore);
        }
        //
        bool isvalid = true;
        //
        switch (pCmdCookie->getCmd())
        {
            case PrvCmdCookie::eStart:
                isvalid = start();
                break;
            case PrvCmdCookie::eDelay:
                isvalid = delay(EQueryType_Init);
                break;
            case PrvCmdCookie::eUpdate:
                isvalid = update();
                break;
            case PrvCmdCookie::ePrecap:
                isvalid = precap();
                break;
            case PrvCmdCookie::eStop:
                isvalid = stop();
                break;
            case PrvCmdCookie::eExit:
            default:
                break;
        }

        //
        if(pCmdCookie != 0)
        {
            pCmdCookie->setValid(isvalid);
            pCmdCookie->postSem(PrvCmdCookie::eSemAfter);
        }
    }
    //
    FUNCTION_OUT;
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
postCommand(PrvCmdCookie::ECmdType const cmdType, PrvCmdCookie::ESemWait const semWait)
{
    CAM_TRACE_NAME("PrvCQT_PHO::postCommand");
    FUNCTION_IN;
    //
    bool ret = true;
    //
    sp<PrvCmdCookie> cmdCookie(new PrvCmdCookie(cmdType, semWait));
    //
    {
        Mutex::Autolock _l(mCmdMtx);
        //
        MY_LOGD("+ tid(%d), que size(%d)", ::gettid(), mCmdCookieQ.size());

        if (!mCmdCookieQ.empty())
        {
            MY_LOGD("(%d) in the head of queue", (*mCmdCookieQ.begin())->getCmd());
        }

        mCmdCookieQ.push_back(cmdCookie);
        mCmdCond.broadcast();
        MY_LOGD("- new command added(%d):  tid(%d), que size(%d)", cmdType, ::gettid(), mCmdCookieQ.size());
    }
    //
    cmdCookie->waitSem();
    if (!cmdCookie->isValid())
    {
        ret = false;
    }
    //
    FUNCTION_OUT;
    //
    return ret;
}



/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
getCommand(sp<PrvCmdCookie> &rCmdCookie)
{
    FUNCTION_IN;
    //
    bool ret = false;
    //
    Mutex::Autolock _l(mCmdMtx);
    //
    MY_LOGD("+ tid(%d), que size(%d)", ::gettid(), mCmdCookieQ.size());
    //
    while ( mCmdCookieQ.empty() && ! exitPending() )
    {
        mCmdCond.wait(mCmdMtx);
    }
    //
    if ( ! mCmdCookieQ.empty() )
    {
        rCmdCookie = *mCmdCookieQ.begin();
        mCmdCookieQ.erase(mCmdCookieQ.begin());
        ret = true;
        MY_LOGD("Command: %d", rCmdCookie->getCmd());
    }
    //
    MY_LOGD("- tid(%d), que size(%d), ret(%d)", ::gettid(), mCmdCookieQ.size(), ret);
    //
    FUNCTION_OUT;
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
isNextCommand()
{
   Mutex::Autolock _l(mCmdMtx);
   //
   return mCmdCookieQ.empty()? false : true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
start()
{
    CAM_TRACE_NAME("PrvCQT_PHO::start");
    FUNCTION_IN;
    //
    bool ret = false;
    int dum_ang = -1;
    vector<IhwScenario::PortImgInfo> vimgInfo;
    vector<IhwScenario::PortBufInfo> vBufPass1Out;
    ImgBufQueNode Pass1Node;
    IhwScenario::PortBufInfo BufInfo;

    //(0.1) Sensor info init for vt
    dum_ang=get_dummy_angle();
    cameraInfo = MtkCamUtils::DevMetaInfo::queryCameraInfo(gInfo.openId);
    if (cameraInfo.orientation+dum_ang>=360){
           cameraInfo.orientation = cameraInfo.orientation - 360;
    }
    else{
        cameraInfo.orientation = cameraInfo.orientation+dum_ang;
    }

    if (cameraInfo.orientation == 90 ||cameraInfo.orientation == 270){
        mRotation = 1;
    }
    else{
        mRotation = 0;
    }
    MY_LOGD("VTInfo, id(%d),orientation(%d),facing(%d),mRot(%d),dum_ang(%d)",
            gInfo.openId,cameraInfo.orientation, cameraInfo.facing, mRotation,dum_ang);

    //(1) sensor (singleton)
    //
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Init Sensor");
    //
    if ( ! (ret = mSensorInfo.init(SENSOR_SCENARIO_ID_NORMAL_VIDEO)))
    {
        MY_LOGE("Init sensor fail!!");
        goto lbExit;
    }

    //(2) Hw scenario
    //
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Init Hw");
    //
    ERawPxlID bitorder;
    switch(mSensorInfo.mSensorStaticInfo.sensorFormatOrder)
    {
        case SENSOR_FORMAT_ORDER_RAW_B:
        {
            bitorder = ERawPxlID_B;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gb:
        {
            bitorder = ERawPxlID_Gb;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gr:
        {
            bitorder = ERawPxlID_Gr;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_R:
        {
            bitorder = ERawPxlID_R;
            break;
        }
    }

    mpHwScenario = IhwScenario::createInstance(
                                    eHW_VSS,
                                    mSensorInfo.getSensorType(),
                                    mSensorInfo.meSensorDev,
                                    bitorder);

    if(mpHwScenario != NULL)
    {
        if(!(mpHwScenario->init()))
        {
            MY_LOGE("init Hw Scenario fail!!");
            goto lbExit;
        }
    }
    else
    {
        MY_LOGE("mpHwScenario is NULL!!");
        goto lbExit;
    }

    mpHwScenario->enableTwoRunPass2(MFALSE);

    // (2.1) hw config
    //
    getCfg(eID_Pass1In|eID_Pass1Out, vimgInfo);
    getHw()->setConfig(&vimgInfo);

    // (2.2) enque pass 1 buffer
    //     must do this earlier than hw start
    CAM_TRACE_BEGIN("PrvBufHdl::allocBuffer");
    mspPreviewBufHandler->allocBuffer(
                          mSensorInfo.getImgWidth(),
                          mSensorInfo.getImgHeight(),
                          mSensorInfo.getImgFormat(),
                          PASS1BUFCNT);
    CAM_TRACE_END();

    for (int32_t i = 0; i < PASS1BUFCNT; i++)
    {
        mspPreviewBufHandler->dequeBuffer(eID_Pass1Out, Pass1Node);
        mapNode2BufInfo(eID_Pass1Out, Pass1Node, BufInfo);
        vBufPass1Out.push_back(BufInfo);
    }
    getHw()->enque(NULL, &vBufPass1Out);


    //(3) 3A
    //!! must be set after hw->enque; otherwise, over-exposure.
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Init 3A");
    //
    mp3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(gInfo.openId));
    if ( ! mp3AHal )
    {
        MY_LOGE("init 3A fail!!");
        goto lbExit;
    }
    CAM_TRACE_BEGIN("3A::setZoom");
    mp3AHal->setZoom(100, 0, 0, mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("3A::setSensorMode");
    mp3AHal->setSensorMode(SENSOR_SCENARIO_ID_NORMAL_VIDEO);
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("3A::setIspProfile");
    mp3AHal->setIspProfile(EIspProfile_VideoPreview);
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("3A::sendCommand");
    mp3AHal->sendCommand(ECmd_CamcorderPreviewStart);
    CAM_TRACE_END();

#if 0
    // (4) EIS
    //
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Init EIS");
    mpEisHal = EisHalBase::createInstance("mtkphotoAdapter");
    if(mpEisHal != NULL)
    {
        eisHal_config_t eisHalConfig;
        eisHalConfig.imageWidth = mSensorInfo.getImgWidth();
        eisHalConfig.imageHeight = mSensorInfo.getImgHeight();
        CAM_TRACE_BEGIN("EIS::configEIS");
        mpEisHal->configEIS(
                    eHW_VSS,
                    eisHalConfig);
        CAM_TRACE_END();
    }
    else
    {
        MY_LOGE("mpEisHal is NULL");
        goto lbExit;
    }
#endif
    // (5) hw start
    // !!enable pass1 SHOULD BE last step!!
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Hw start");
    //
    if ( ! getHw()->start())
    {
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    //
    FUNCTION_OUT;
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
delay(EQueryType_T type)
{
    FUNCTION_IN;

    bool ret = true;

    //(1)
    switch (type)
    {
        case EQueryType_Init:
        {

            IStateManager::inst()->transitState(IState::eState_Preview);
            //
            //(1) delay by AAA and sensor driver
            //
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t delaySensor = mSensorInfo.getDelayFrame(SENSOR_SCENARIO_ID_NORMAL_VIDEO);
            int32_t delayCnt = 1;

            if(delay3A >= (delaySensor-1))
            {
                delayCnt += delay3A;
            }
            else
            {
                delayCnt += (delaySensor-1);
            }

            //(2) should send update to sw
            // Error Handling: If failure time accumulates up to 2 tims (which equals to 10 secs),
            // leave while loop and return fail.
            int failCnt = 0;
            CAM_TRACE_BEGIN("PrvCQT_PHO::delay::init");
            for (int32_t i = 0; i < delayCnt; i++)
            {
                if ( ! dropFrame() )
                {
                    delayCnt++;
                    failCnt++;

                    if (failCnt >= 2)
                    {
                        return false;
                    }
                    continue;
                }

                failCnt = 0;
            }
            CAM_TRACE_END();
            MY_LOGD("delay(Init):delayCnt(%d),3A(%d),sensor(%d)",delayCnt,delay3A,delaySensor);
        }
        break;

        case EQueryType_Effect:
        {
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            CAM_TRACE_BEGIN("PrvCQT_PHO::delay::Effect");
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbEFFECTIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }
            CAM_TRACE_END();
            MY_LOGD("delay(Effect): (%d), real: (%d)", delay3A, count);
        }
        break;

        case EQueryType_AWB:
        {
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            CAM_TRACE_BEGIN("PrvCQT_PHO::delay::AWB");
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbAWBIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }
            CAM_TRACE_END();
            MY_LOGD("delay(Awb): (%d), real: (%d)", delay3A, count);
        }
        break;

        case EQueryType_Ev:
        {
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            CAM_TRACE_BEGIN("PrvCQT_DEF::delay::Ev");
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbEvIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }
            CAM_TRACE_END();
            MY_LOGD("delay(Ev): (%d), real: (%d)", delay3A, count);
        }
        break;

        case EQueryType_Sat:
        {
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            CAM_TRACE_BEGIN("PrvCQT_DEF::delay::Sat");
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbSatIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }
            CAM_TRACE_END();
            MY_LOGD("delay(Sat): (%d), real: (%d)", delay3A, count);
        }
        break;

        case EQueryType_Bright:
        {
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            CAM_TRACE_BEGIN("PrvCQT_DEF::delay::Bright");
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbBrightIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }
            CAM_TRACE_END();
            MY_LOGD("delay(Bright): (%d), real: (%d)", delay3A, count);
        }
        break;

        case EQueryType_Contrast:
        {
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            CAM_TRACE_BEGIN("PrvCQT_DEF::delay::Contrast");
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbContrastIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }
            CAM_TRACE_END();
            MY_LOGD("delay(Contrast): (%d), real: (%d)", delay3A, count);
        }
        break;
    }

    FUNCTION_OUT;

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
setParameters()
{
    CAM_TRACE_NAME("PrvCQT_PHO::setParameters");
    FUNCTION_IN;

#define UPDATE_PARAMS(param, eMapXXX, key) \
    do { \
        String8 const s = mspParamsMgr->getStr(key); \
        if  ( ! s.isEmpty() ) { \
            param = PARAMSMANAGER_MAP_INST(eMapXXX)->valueFor(s); \
        } \
    } while (0)

    //(0)
    Hal3ABase* p3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(gInfo.openId));
    if ( ! p3AHal )
    {
        MY_LOGE("init 3A fail!!");
        return false;
    }


    //(1) Check awb mode change
    {
        int32_t newParam;
        UPDATE_PARAMS(newParam, eMapWhiteBalance, CameraParameters::KEY_WHITE_BALANCE);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.u4AwbMode;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbAWBIndicator);
            MY_LOGD("AWB mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }

    //(2) check effect mode change
    {
        int32_t newParam;
        UPDATE_PARAMS(newParam, eMapEffect, CameraParameters::KEY_EFFECT);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.u4EffectMode;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbEFFECTIndicator);
            MY_LOGD("EFFECT mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }

    //(3) check Ev change
    {
        int32_t newParam;
        //UPDATE_PARAMS(newParam, eMapEv, CameraParameters::KEY_EXPOSURE_COMPENSATION);
        newParam = mspParamsMgr->getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.i4ExpIndex;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbEvIndicator);
            MY_LOGD("EV mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }

    //(4) check Saturation mode change
    {
        int32_t newParam;
        UPDATE_PARAMS(newParam, eMapLevel, MtkCameraParameters::KEY_SATURATION);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.u4SaturationMode;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbSatIndicator);
            MY_LOGD("SATURATION mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }

    //(5) check Brightness mode change
    {
        int32_t newParam;
        UPDATE_PARAMS(newParam, eMapLevel, MtkCameraParameters::KEY_BRIGHTNESS);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.u4BrightnessMode;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbBrightIndicator);
            MY_LOGD("BRIGHTNESS mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }

    //(6) check Contrast mode change
    {
        int32_t newParam;
        UPDATE_PARAMS(newParam, eMapLevel, MtkCameraParameters::KEY_CONTRAST);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.u4ContrastMode;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbContrastIndicator);
            MY_LOGD("CONTRAST mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }

    //(7) Zoom
    setZoom(getParamsMgr()->getZoomRatio());
    //
    p3AHal->destroyInstance();

    //
    FUNCTION_OUT;

    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
setZoom(uint32_t zoomValue)
{
    FUNCTION_IN;
    //
    Mutex::Autolock _l(mZoomMtx);
    //
    if( mvZoomRatio.empty() ||
        (   !mvZoomRatio.empty() &&
            *mvZoomRatio.end() != zoomValue))
    {
        MY_LOGD("zoomValue(%d)",zoomValue);
        mvZoomRatio.push_back(zoomValue);
    }
    //
    FUNCTION_OUT;
    //
    return true;
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
PreviewCmdQueThread::
getZoom()
{
    //FUNCTION_IN;
    //
    Mutex::Autolock _l(mZoomMtx);
    //
    uint32_t i,ZoomSkip;
    uint32_t zoomValue;
    //
    if(!mvZoomRatio.empty())
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Size(%d)",mvZoomRatio.size());
        if(mvZoomRatio.size() > ZOOM_SKIP_STEP)
        {
            for(i=0; i<ZOOM_SKIP_STEP; i++)
            {
                ZoomSkip = *mvZoomRatio.begin();
                mvZoomRatio.erase(mvZoomRatio.begin());
                MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Skip(%d)",ZoomSkip);
            }
        }
        zoomValue = *mvZoomRatio.begin();
        mvZoomRatio.erase(mvZoomRatio.begin());
    }
    else
    {
        //MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Zoom from params");
        zoomValue = getParamsMgr()->getZoomRatio();
    }
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Value(%d)",zoomValue);
    //
    //FUNCTION_OUT;
    //
    return zoomValue;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
stop()
{
    CAM_TRACE_NAME("PrvCQT_PHO::stop");
    FUNCTION_IN;
    //
    bool ret = true;

    //(1) stop sw
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagSeparator, "stop 3A");
    //
    if(mp3AHal != NULL)
    {
        CAM_TRACE_BEGIN("3A");
        mp3AHal->sendCommand(ECmd_CamcorderPreviewEnd);
        mp3AHal->destroyInstance();
        mp3AHal = NULL;
        CAM_TRACE_END();
    }

    //(2) stop HW scenario
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagSeparator, "stop Hw");
    //
    if (mpHwScenario != NULL)
    {
        CAM_TRACE_BEGIN("HwScenario");
        if ( ! (ret = getHw()->stop()) )
        {
            MY_LOGE("fail");
            CAM_TRACE_END();
            ret = false;
        }
        mpHwScenario->uninit();
        mpHwScenario->destroyInstance();
        mpHwScenario = NULL;
        CAM_TRACE_END();
    }
    //
    if(mspPreviewBufHandler != NULL)
    {
        CAM_TRACE_BEGIN("PrvBufHdl::freeBuffer");
        mspPreviewBufHandler->freeBuffer();
        CAM_TRACE_END();
    }

    //(3) stop sensor
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagSeparator, "stop sensor");
    if ( ! mSensorInfo.uninit() )
    {
        MY_LOGE("uninit sensor fail");
        ret = false;
    }

#if 0
    //(4) stop eis
    //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagSeparator, "stop EIS");
    if(mpEisHal != NULL)
    {
        CAM_TRACE_BEGIN("EIS");
        mpEisHal->destroyInstance("mtkphotoAdapter");
        mpEisHal = NULL;
        CAM_TRACE_END();
    }
#endif

    //(5) change state to idle
    IStateManager::inst()->transitState(IState::eState_Idle);
    //
    ::android_atomic_write(0, &mbAWBIndicator);
    ::android_atomic_write(0, &mbEFFECTIndicator);
    ::android_atomic_write(0, &mbEvIndicator);
    ::android_atomic_write(0, &mbSatIndicator);
    ::android_atomic_write(0, &mbBrightIndicator);
    ::android_atomic_write(0, &mbContrastIndicator);
    mvZoomRatio.clear();


    FUNCTION_OUT;
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
precap()
{
    CAM_TRACE_NAME("PrvCQT_PHO::precap");
    FUNCTION_IN;
#if 0
    //(1) notify sw
    CAM_TRACE_BEGIN("3A::ECmd_PrecaptureStart");
    mp3AHal->sendCommand(ECmd_PrecaptureStart);
    CAM_TRACE_END();

    //(2) stay in preview until 3A permits
    CAM_TRACE_BEGIN("3A::isReadyToCapture");
    while ( ! mp3AHal->isReadyToCapture() )
    {
         //CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_precap, CPTFlagSeparator, "precap_update");
         //
         updateOne();
    }
    CAM_TRACE_END();

    //(3) notify sw
    CAM_TRACE_BEGIN("3A::ECmd_PrecaptureEnd");
    mp3AHal->sendCommand(ECmd_PrecaptureEnd);
    CAM_TRACE_END();

    //(4) change state to precapture state
    IStateManager::inst()->transitState(IState::eState_PreCapture);
#endif
    //
    FUNCTION_OUT;
    //
    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
update()
{
    // Loop: check if next command is comming
    // Next command can be {stop, precap}
    // Do at least 1 frame (in case of going to precapture directly)
    //  --> this works when AE updates in each frame (instead of in 3 frames)

    do{
        CPTLog(Event_Hal_Adapter_MtkPhotoPreview_proc, CPTFlagStart);
        //(1)
        updateOne();

        MY_LOGD_IF(0, "frameCnt(%d)",mFrameCnt);
        mFrameCnt++;

        //(2) do some check in each frame
        updateCheck();

        CPTLog(Event_Hal_Adapter_MtkPhotoPreview_proc, CPTFlagEnd);

    } while( ! isNextCommand() );

    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
updateCheck()
{
    bool ret = false;

    //(1) change AWB/EFFECT delay
    //what if 'AWB and EFFECT mode change' are coming together?
    //only choose one delay? which one? larger one?
#if 0
    MY_LOGW_IF(::android_atomic_release_load(&mbAWBIndicator) &&
               ::android_atomic_release_load(&mbEFFECTIndicator),
               "AWB and effect mode are changing at the same time");
#endif
    if (::android_atomic_release_load(&mbAWBIndicator))
    {
        ret = delay(EQueryType_AWB);
        ::android_atomic_write(0, &mbAWBIndicator);
    }

    if (::android_atomic_release_load(&mbEFFECTIndicator))
    {
        ret = delay(EQueryType_Effect);
        ::android_atomic_write(0, &mbEFFECTIndicator);
    }

    if (::android_atomic_release_load(&mbEvIndicator))
    {
        delay(EQueryType_Ev);
        ::android_atomic_write(0, &mbEvIndicator);
    }

    if (::android_atomic_release_load(&mbSatIndicator))
    {
        delay(EQueryType_Sat);
        ::android_atomic_write(0, &mbSatIndicator);
    }

    if (::android_atomic_release_load(&mbBrightIndicator))
    {
        delay(EQueryType_Bright);
        ::android_atomic_write(0, &mbBrightIndicator);
    }

    if (::android_atomic_release_load(&mbContrastIndicator))
    {
        delay(EQueryType_Contrast);
        ::android_atomic_write(0, &mbContrastIndicator);
    }
    //(2) BV value (3A --> AP)
    FrameOutputParam_T RTParams;
    mp3AHal->getRTParams(RTParams);
    int rt_BV = RTParams.i4BrightValue_x10;
    int rt_FPS = RTParams.u4FRameRate_x10;
    mspParamsMgr->updateBrightnessValue(rt_BV);

    return ret;
}

/******************************************************************************
*
*
*******************************************************************************/
bool
PreviewCmdQueThread::
checkIfDropFrame()
{
    bool ret = false;
    ret =    ::android_atomic_release_load(&mbAWBIndicator)
          || ::android_atomic_release_load(&mbEFFECTIndicator)
          || ::android_atomic_release_load(&mbEvIndicator)
          || ::android_atomic_release_load(&mbSatIndicator)
          || ::android_atomic_release_load(&mbBrightIndicator)
          || ::android_atomic_release_load(&mbContrastIndicator);
    return ret;
}


/******************************************************************************
*
*
*******************************************************************************/
bool
PreviewCmdQueThread::
updateOne()
{
    CAM_TRACE_NAME("PrvCQT_PHO::updateOne");

    bool ret = true;
    int32_t pass1LatestBufIdx = -1;
    int64_t pass1LatestTimeStamp = 0;

    vector<IhwScenario::PortQTBufInfo> vDeBufPass1Out;
    vector<IhwScenario::PortQTBufInfo> vDeBufPass2Out;
    vector<IhwScenario::PortBufInfo> vEnBufPass2In;
    vector<IhwScenario::PortBufInfo> vEnBufPass2Out;
    vector<IhwScenario::PortImgInfo> vPass2Cfg;


    //*************************************************************
    // (1) [PASS 1] sensor ---> ISP --> DRAM(IMGO)
    //*************************************************************
    if ( ! getHw()->deque(eID_Pass1Out, &vDeBufPass1Out) )
    {
        int i, tryCnt = 1;
        for (i = 0; i < tryCnt; i++)
        {
            MY_LOGW("drop frame failed. try reset sensor(%d)", i);
            mSensorInfo.reset();
            if (getHw()->deque(eID_Pass1Out, &vDeBufPass1Out))
            {
                MY_LOGD("success.");
                break;
            }
            else
            {
                MY_LOGE("still failed.");
            }
        }
        //
        if(i == tryCnt)
        {
            return false;
        }
    }

    mapQT2BufInfo(eID_Pass2In, vDeBufPass1Out, vEnBufPass2In);

    mp3AHal->sendCommand(ECmd_Update);
    //
#if 0
    mpEisHal->doEIS();
#endif

    if ( checkIfDropFrame() )
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "IND(%d/%d/%d/%d/%d/%d)",
                    ::android_atomic_release_load(&mbAWBIndicator),
                    ::android_atomic_release_load(&mbEFFECTIndicator),
                    ::android_atomic_release_load(&mbEvIndicator),
                    ::android_atomic_release_load(&mbSatIndicator),
                    ::android_atomic_release_load(&mbBrightIndicator),
                    ::android_atomic_release_load(&mbContrastIndicator));
        getHw()->enque(vDeBufPass1Out);
        return false;
    }

    //
    //*************************************************************
    //(2) [PASS 2] DRAM(IMGI) --> ISP --> CDP --> DRAM (DISPO, VIDO)
    //    if no buffer is available, return immediately.
    //*************************************************************
    int32_t flag = 0;


    //(.1) PASS2-IN
    //
    mCurZoomValue = getZoom();
    getCfg(eID_Pass2In , vPass2Cfg);

    //(.2) PASS2-OUT
    //
    ImgBufQueNode dispNode;
    ImgBufQueNode vidoNode;
    mspPreviewBufHandler->dequeBuffer(eID_Pass2DISPO, dispNode);
    mspPreviewBufHandler->dequeBuffer(eID_Pass2VIDO, vidoNode);

    if ( dispNode.getImgBuf() != 0)
    {
        flag |= eID_Pass2DISPO;
        IhwScenario::PortBufInfo BufInfo;
        IhwScenario::PortImgInfo ImgInfo;
        mapNode2BufInfo(eID_Pass2DISPO, dispNode, BufInfo);
        mapNode2ImgInfo(eID_Pass2DISPO, dispNode, ImgInfo);
        //mapNode2ImgInfo(eID_Pass2DISPO, dispNode, ImgInfo,cameraInfo);
        vEnBufPass2Out.push_back(BufInfo);
        vPass2Cfg.push_back(ImgInfo);
    }

    if ( vidoNode.getImgBuf() != 0)
    {
        flag = flag | eID_Pass2VIDO;
        IhwScenario::PortBufInfo BufInfo;
        IhwScenario::PortImgInfo ImgInfo;
        mapNode2BufInfo(eID_Pass2VIDO, vidoNode, BufInfo);
        mapNode2ImgInfo(eID_Pass2VIDO, vidoNode, ImgInfo,cameraInfo);
        vEnBufPass2Out.push_back(BufInfo);
        vPass2Cfg.push_back(ImgInfo);
    }

    //(.3) no buffer ==> return immediately.
    //
    MY_LOGD_IF(1, "P2(0x%X)", flag);

    if ( ! flag )
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2(0x%X),FC(%d)",flag,mFrameCnt);
        getHw()->enque(vDeBufPass1Out);
        ret = false;
        goto lbExit;
    }

    //(.4) has buffer ==> do pass2 en/deque
    // Note: config must be set earlier than en/de-que
    //
    updateZoom(vPass2Cfg);
    getHw()->setConfig(&vPass2Cfg);

    getHw()->enque(&vEnBufPass2In, &vEnBufPass2Out);
    getHw()->deque((EHwBufIdx)flag, &vDeBufPass2Out);


    //*************************************************************
    // (3) return buffer
    //*************************************************************

    if( vDeBufPass1Out.size() > 0 &&
        vDeBufPass1Out[0].bufInfo.vBufInfo.size() > 0)
    {
         pass1LatestBufIdx = vDeBufPass1Out[0].bufInfo.vBufInfo.size()-1;
         pass1LatestTimeStamp = vDeBufPass1Out[0].bufInfo.vBufInfo[pass1LatestBufIdx].getTimeStamp_ns();
         //MY_LOGD("pass1LatestBufIdx(%d),pass1LatestTimeStamp(%lld)",pass1LatestBufIdx,pass1LatestTimeStamp);
    }

    // (.1) return PASS1
    getHw()->enque(vDeBufPass1Out);

    // (.2) return PASS2
    if (flag & eID_Pass2DISPO)
    {
        dispNode.getImgBuf()->setTimestamp(pass1LatestTimeStamp);
        mspPreviewBufHandler->enqueBuffer(dispNode);
    }
    //
    if (flag & eID_Pass2VIDO)
    {
        vidoNode.getImgBuf()->setTimestamp(pass1LatestTimeStamp);
        mspPreviewBufHandler->enqueBuffer(vidoNode);
    }
    //[T.B.D]
    //'0': "SUPPOSE" DISPO and VIDO gets the same timeStamp
    if( vDeBufPass2Out.size() > 1 )
    {
        MY_LOGW_IF(vDeBufPass2Out.at(0).bufInfo.getTimeStamp_ns() != vDeBufPass2Out.at(1).bufInfo.getTimeStamp_ns(),
        "DISP(%f),VIDO(%f)", vDeBufPass2Out.at(0).bufInfo.getTimeStamp_ns(), vDeBufPass2Out.at(1).bufInfo.getTimeStamp_ns());
    }
    //
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2(0x%X),FC(%d)",flag,mFrameCnt);
    //
    //
lbExit:

#ifdef DUMP
    if (ret)
    {
        dumpBuffer(vDeBufPass1Out, "pass1", "raw", mFrameCnt, eID_Pass1Out );
        if (flag & eID_Pass2DISPO)
        {
            dumpImg(
                (MUINT8*)(dispNode.getImgBuf()->getVirAddr()),
                dispNode.getImgBuf()->getBufSize(),
                "pass2_dispo",
                "yuv",
                mFrameCnt,
                eID_Pass2DISPO);
        }
        if (flag & eID_Pass2VIDO)
        {
            dumpImg(
                (MUINT8*)(vidoNode.getImgBuf()->getVirAddr()),
                vidoNode.getImgBuf()->getBufSize(),
                "pass2_vido",
                "yuv",
                mFrameCnt,
                eID_Pass2VIDO);
        }
    }
#endif

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
updateZoom(vector<IhwScenario::PortImgInfo> &rImgIn)
{
    //   (1) calculate zoom
    //   by  src (from sensor output, or for video it's pass 1 out)
    //   and dst (preview size)
    int32_t PrvWidth = 0;
    int32_t PrvHeight = 0;
    int32_t temp;
    //
    getParamsMgr()->getPreviewSize(&PrvWidth, &PrvHeight);
    if (mRotation){
        temp = PrvHeight;
        PrvHeight = PrvWidth;
        PrvWidth = temp;
    }
    IhwScenario::Rect_t Src(mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    IhwScenario::Rect_t Dst(PrvWidth, PrvHeight);
    IhwScenario::Rect_t Crop = doCrop(Src, Dst, mCurZoomValue);
    //   (2) set to 3A
    mp3AHal->setZoom(mCurZoomValue, Crop.x, Crop.y, Crop.w, Crop.h);

    //   (3) set to hw config
    rImgIn.at(0).crop = Crop;
}


/*******************************************************************************
*
********************************************************************************/
IhwScenario::Rect_t
PreviewCmdQueThread::
doCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio)
{
    if (ratio < 100) {
        MY_LOGW("Attempt (%d) < min zoom(%d)" , ratio, 100);
        ratio = 100;
    }
    if (ratio > 800) {
        MY_LOGW("Attempt (%d) > max zoom(%d)" , ratio, 800);
        ratio = 800;
    }

    IhwScenario::Rect_t rCrop = calCrop(rSrc, rDst, ratio);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "S(%d/%d),D(%d/%d),Z(%d),C(%d,%d,%d,%d)",
                rSrc.w, rSrc.h,
                rDst.w, rDst.h,
                ratio,
                rCrop.x, rCrop.y, rCrop.w, rCrop.h);

    return rCrop;
}


/*******************************************************************************
*
********************************************************************************/
bool
PreviewCmdQueThread::
dropFrame()
{
    bool ret = true;

    vector<IhwScenario::PortQTBufInfo> dummy;

    if ( ! getHw()->deque(eID_Pass1Out, &dummy) )
    {
        int i, tryCnt = 1;
        for (i = 0; i < tryCnt; i++)
        {
            MY_LOGW("drop frame failed. try reset sensor(%d)", i);
            mSensorInfo.reset();
            if (getHw()->deque(eID_Pass1Out, &dummy))
            {
                MY_LOGD("success.");
                break;
            }
            else
            {
                MY_LOGE("still failed.");
            }
        }
        //
        if(i == tryCnt)
        {
            return false;
        }
    }
    //
    getHw()->enque(dummy);
    mp3AHal->sendCommand(ECmd_Update);
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
getCfg(int32_t port, vector<IhwScenario::PortImgInfo> &rvImginfo)
{
    if (port & eID_Pass1In)
    {
        IhwScenario::PortImgInfo imgInfoIn(
            eID_Pass1In,
            mSensorInfo.getImgFormat(),
            mSensorInfo.getSensorWidth(),
            mSensorInfo.getSensorHeight()
        );
        imgInfoIn.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        //
        rvImginfo.push_back(imgInfoIn);
    }

    if (port & eID_Pass1Out)
    {
        IhwScenario::PortImgInfo imgInfoOut(
            eID_Pass1Out,
            mSensorInfo.getImgFormat(),
            mSensorInfo.getImgWidth(),
            mSensorInfo.getImgHeight()
        );
        imgInfoOut.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        IhwScenario::Rect_t SensorSize(mSensorInfo.getSensorWidth(), mSensorInfo.getSensorHeight());
        imgInfoOut.crop = calCrop(SensorSize, imgInfoOut.crop, 100);
        //
        rvImginfo.push_back(imgInfoOut);
    }

    if (port & eID_Pass2In)
    {
        IhwScenario::PortImgInfo imgInfoIn2(
            eID_Pass2In,
            mSensorInfo.getImgFormat(),
            mSensorInfo.getImgWidth(),
            mSensorInfo.getImgHeight()
        );
        imgInfoIn2.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        //
        rvImginfo.push_back(imgInfoIn2);
    }
}


/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread*
PreviewCmdQueThread::
getInstance(sp<IPreviewBufMgrHandler> pHandler, int32_t const & rSensorid, sp<IParamsManager> pParamsMgr)
{
    return  new PreviewCmdQueThread(pHandler, rSensorid, pParamsMgr);
}


/******************************************************************************
*
*******************************************************************************/
IPreviewCmdQueThread*
IPreviewCmdQueThread::
createInstance(sp<IPreviewBufMgrHandler> pHandler, int32_t const & rSensorid, sp<IParamsManager> pParamsMgr)
{
    if  ( pHandler != 0 ) {
        return  PreviewCmdQueThread::getInstance(pHandler, rSensorid, pParamsMgr);
    }

    MY_LOGE("pHandler==NULL");
    return  NULL;
}


/*******************************************************************************
*
********************************************************************************/
static
IhwScenario::Rect_t
calCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio)
{
#if 0
    IhwScenario::Rect_t rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.w * rDst.h < rDst.w * rSrc.h) {
        rCrop.w = rSrc.w;
        rCrop.h = rSrc.w * rDst.h / rDst.w;
    }
    //srcW/srcH > dstW/dstH
    else if (rSrc.w * rDst.h > rDst.w * rSrc.h) {
        rCrop.w = rSrc.h * rDst.w / rDst.h;
        rCrop.h = rSrc.h;
    }
    else {
        rCrop.w = rSrc.w;
        rCrop.h = rSrc.h;
    }
    //
    rCrop.w =  ROUND_TO_2X(rCrop.w * 100 / ratio);
    rCrop.h =  ROUND_TO_2X(rCrop.h * 100 / ratio);
    //
    rCrop.x = (rSrc.w - rCrop.w) / 2;
    rCrop.y = (rSrc.h - rCrop.h) / 2;
#else
    NSCamHW::Rect rHWSrc(rSrc.x, rSrc.y, rSrc.w, rSrc.h);
    NSCamHW::Rect rHWDst(rDst.x, rDst.y, rDst.w, rDst.h);
    NSCamHW::Rect rHWCrop = MtkCamUtils::calCrop(rHWSrc, rHWDst, ratio);

    IhwScenario::Rect_t rCrop(rHWCrop.w, rHWCrop.h, rHWCrop.x, rHWCrop.y );
#endif

    return rCrop;
}


/******************************************************************************
*
*******************************************************************************/
static
bool
dumpBuffer(
    vector<IhwScenario::PortQTBufInfo> &src,
    char const*const tag,
    char const * const filetype,
    uint32_t filenum,
    int flag)
{
#ifdef DUMP
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);
    if (enable == 0)
    {
        return false;
    }
#endif

    for (MUINT32 i = 0; i < src.size(); i++)
    {
        if ( ! src.at(i).bufInfo.vBufInfo.size() )
        {
            MY_LOGE("(%s) src.at(%d).bufInfo.vBufInfo.size() = 0", tag, i);
            continue;
        }

        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "addr: 0x%x, size: %d, time: %f",
           src.at(i).bufInfo.vBufInfo.at(0).u4BufVA,
           src.at(i).bufInfo.vBufInfo.at(0).u4BufSize,
           src.at(i).bufInfo.getTimeStamp_ns());

        if (!dumpImg((MUINT8*)src.at(i).bufInfo.vBufInfo.at(0).u4BufVA,
                  src.at(i).bufInfo.vBufInfo.at(0).u4BufSize,
                  tag, filetype, filenum, flag))
        {
            MY_LOGE("Dump buffer fail");
        }
    }
#endif

    return true;
}

/******************************************************************************
*
*******************************************************************************/
static
bool
dumpImg(
    MUINT8 *addr,
    MUINT32 size,
    char const * const tag,
    char const * const filetype,
    uint32_t filenum,
    int flag)
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);
    if ( (enable & flag) == 0)
    {
        return false;
    }
#endif
    //
    char* filePath = "/sdcard/camera_dump/MtkVT/";
    char fileName[64];
    sprintf(fileName, "%s%s_%d.%s", filePath ,tag, filenum, filetype);

    if(makePath(filePath,0660))
    {
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            MY_LOGE("fail to open file to save img: %s", fileName);
            return false;
        }

        fwrite(addr, 1, size, fp);
        fclose(fp);
    }

    return true;
}


/******************************************************************************
*
*******************************************************************************/
static
bool
mapQT2BufInfo(EHwBufIdx ePort, vector<IhwScenario::PortQTBufInfo> const &src, vector<IhwScenario::PortBufInfo> &dst)
{
    if ( src.size() <= 0 ) {
        MY_LOGE("vector size is 0!");
        return false;
    }

    if ( src.at(0).bufInfo.vBufInfo.empty() ) {
        MY_LOGE("Pass 1 buffer is 0!");
        return false;
    }

    int latest = src.at(0).bufInfo.vBufInfo.size()-1;
    MY_LOGD_IF(latest > 0, "deque size: %d", latest+1);

    IhwScenario::PortBufInfo one(
                     ePort,
                     src.at(0).bufInfo.vBufInfo.at(latest).u4BufVA,
                     src.at(0).bufInfo.vBufInfo.at(latest).u4BufPA,
                     src.at(0).bufInfo.vBufInfo.at(latest).u4BufSize,
                     src.at(0).bufInfo.vBufInfo.at(latest).memID
    );

    dst.push_back(one);

    MY_LOGD_IF(0, "VA(0x%08X),S(%d),Idx(%d),Id(%d)",
             one.virtAddr, one.bufSize, one.ePortIndex, one.memID);

    return true;
}


/******************************************************************************
*
*******************************************************************************/
static void
mapNode2BufInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortBufInfo &dst)
{
    dst.virtAddr   = (MUINTPTR)src.getImgBuf()->getVirAddr();
    dst.phyAddr    = (MUINTPTR)src.getImgBuf()->getPhyAddr();
    dst.bufSize    = src.getImgBuf()->getBufSize();
    dst.ePortIndex = idx;
    dst.memID      = src.getImgBuf()->getIonFd();
    MY_LOGD_IF(0, "VA(0x%08X),S(%d),Idx(%d),Id(%d)",
                   dst.virtAddr, dst.bufSize, dst.ePortIndex, dst.memID);
}


/******************************************************************************
*
*******************************************************************************/
static void
mapNode2ImgInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortImgInfo &dst, camera_info minfo)
{
    dst.ePortIdx = idx;
    dst.sFormat  = src.getImgBuf()->getImgFormat().string();
    dst.u4Width  = src.getImgBuf()->getImgWidth();
    dst.u4Height = src.getImgBuf()->getImgHeight();
    dst.u4Stride[ESTRIDE_1ST_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_1ST_PLANE);
    dst.u4Stride[ESTRIDE_2ND_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_2ND_PLANE);
    dst.u4Stride[ESTRIDE_3RD_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_3RD_PLANE);

    if (minfo.orientation == 270){
       dst.eRotate = eImgRot_270;  //720p sub
    }
    else if (minfo.orientation == 90){
       dst.eRotate = eImgRot_90;   //720p main
    }
    else if (minfo.orientation == 180){
       dst.eRotate = eImgRot_180;
    }
    else{
       dst.eRotate = eImgRot_0;
    }
    //[T.B.D]
    /*
    dst.eRotate  = src.getRotation() == 0 ? eImgRot_0
                 : src.getRotation() == 90 ? eImgRot_90
                 : src.getRotation() == 180 ? eImgRot_180 : eImgRot_270;
    */
    dst.eFlip    = eImgFlip_OFF;
    //
    MY_LOGD_IF(0, "Port(%d),F(%s),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
                   dst.ePortIdx, dst.sFormat, dst.u4Width, dst.u4Height,
                   dst.u4Stride[ESTRIDE_1ST_PLANE], dst.u4Stride[ESTRIDE_2ND_PLANE], dst.u4Stride[ESTRIDE_3RD_PLANE], dst.eRotate,minfo.orientation);
}
/******************************************************************************
*
*******************************************************************************/
static void
mapNode2ImgInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortImgInfo &dst)
{
    dst.ePortIdx = idx;
    dst.sFormat  = src.getImgBuf()->getImgFormat().string();
    dst.u4Width  = src.getImgBuf()->getImgWidth();
    dst.u4Height = src.getImgBuf()->getImgHeight();
    dst.u4Stride[ESTRIDE_1ST_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_1ST_PLANE);
    dst.u4Stride[ESTRIDE_2ND_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_2ND_PLANE);
    dst.u4Stride[ESTRIDE_3RD_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_3RD_PLANE);

    //[T.B.D]
    dst.eRotate  = src.getRotation() == 0 ? eImgRot_0
                 : src.getRotation() == 90 ? eImgRot_90
                 : src.getRotation() == 180 ? eImgRot_180 : eImgRot_270;

    dst.eFlip    = eImgFlip_OFF;
    //
    MY_LOGD_IF(0, "Port(%d),F(%s),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
                   dst.ePortIdx, dst.sFormat, dst.u4Width, dst.u4Height,
                   dst.u4Stride[ESTRIDE_1ST_PLANE], dst.u4Stride[ESTRIDE_2ND_PLANE], dst.u4Stride[ESTRIDE_3RD_PLANE], dst.eRotate);
}
