/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/ISC"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/RefBase.h>
using namespace android;
//
#include <list>
#include <queue>
using namespace std;
//
#include <aee.h>
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_utility.h>
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
using namespace NSCam::NSIoPipe::NSCamIOPipe;
//
#include <mtkcam/hwutils/HwMisc.h>
//
#include "./inc/IspSyncControlHw.h"
//
#include <core/camshot/inc/CamShotUtils.h>
//
#define PASS2_ZOOM_ENABLE   (0)
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//
#define AEE_ASSERT(String)    \
    do {                      \
        aee_system_exception( \
            LOG_TAG,          \
            NULL,             \
            DB_OPT_DEFAULT,   \
            String);          \
    } while(0)
/******************************************************************************
*
*******************************************************************************/
namespace NSCamNode {

class IspSyncControlImp : public IspSyncControlHw
{
    public:

        static IspSyncControlImp*    createInstance(MUINT32 const sensorIdx);
        virtual MVOID               destroyInstance();

    protected:

        IspSyncControlImp(MUINT32 const sensorIdx);
        virtual ~IspSyncControlImp();

    public: //operations

        MUINT32 getSensorIdx() const { return muSensorIdx; };

        virtual MVOID   addZoomRatio(MUINT32 ratio);
        virtual MINT32  popEndZoomRatio();
        virtual MUINT32 getZoomRatio();
        virtual MVOID   setCurZoomRatio(MUINT32 ratio);
        virtual MBOOL   setSensorInfo(
                            MUINT32 scenario,
                            MUINT32 width,
                            MUINT32 height,
                            MUINT32 type);
        virtual MBOOL   getSensorInfo(
                            MUINT32& scenario,
                            MUINT32& width,
                            MUINT32& height,
                            MUINT32& type);
        virtual MBOOL   calRrzoMaxZoomRatio();
        virtual MBOOL   setPass1InitRrzoSize(
                            MUINT32 width,
                            MUINT32 height);
        virtual MBOOL   setPreviewSize(
                            MUINT32 width,
                            MUINT32 height);
        virtual MBOOL   setVideoSize(
                            MUINT32 width,
                            MUINT32 height);
        virtual MBOOL   setRrzoMinSize(
                            MUINT32 width,
                            MUINT32 height);
        virtual MBOOL   getPreviewSize(
                            MUINT32& width,
                            MUINT32& height);
        /**
         * To fix RRZ in/out setting.
         */
        virtual MBOOL   setInitialRrzoSize(
                            MRect&          crop,
                            MSize&          size);
        virtual MBOOL   getInitialRrzoSize(
                            EImageFormat    fmt,
                            MUINT32         pixelMode,
                            MRect&          crop,
                            MSize&          size);
        virtual MBOOL   getPass1Crop(
                            MUINT32         inMagicNum,
                            MBOOL           inIsRrzo,
                            MRect&          outInCrop,
                            MSize&          outOutSize);
        virtual MBOOL   queryPass1CropInfo(
                            MUINT32         inMagicNum,
                            MUINT32&        currentZoomRatio,
                            MUINT32&        targetZoomRatio,
                            MRect&          outInCrop,
                            MSize&          outOutSize);
        virtual MBOOL   addPass1Info(
                            MUINT32         inMagicNum,
                            IImageBuffer*   inpImgBuf,
                            ResultMetadata  inMetadata,
                            MBOOL           inIsRrzo);
        virtual MBOOL   getPass2Info(
                            IImageBuffer*   inpImgBuf,
                            MSize           inOutImgSize,
                            MUINT32&        outMagicNum,
                            MVOID*&         outpPrivateData,
                            MUINT32&        outPrivateDataSize,
                            MCropRect&      outInCrop);
        virtual MBOOL   dropPass2Info(IImageBuffer* pImgBuf);
        virtual MBOOL   queryImgBufInfo(
                            IImageBuffer*   inpImgBuf,
                            MUINT32&        outMagicNum,
                            MBOOL&          outIsRrzo,
                            MVOID*&         outpPrivateData,
                            MUINT32&        outPrivateDataSize);

        virtual MUINT32 getMagicNum(MBOOL bInit);
        virtual MBOOL   send3AUpdateCmd(UPDATE_CMD cmd);
        virtual MBOOL   clearTable();

        // replace buffer for imgo
        virtual MBOOL   addPass1ReplaceBuffer(MUINT32 const fullcnt, MUINT32 const resizedcnt);
        virtual MUINT32 getPass1ReplaceCnt(MBOOL const bFull) const;
        // pass1 update infos: buf <-> size/pointer
        //

        //MBOOL isReadyToUpdate() = 0;
        //MBOOL setReadyToUpdate() = 0;

        virtual MBOOL   lockHw(MUINT32 use);
        virtual MBOOL   unlockHw(MUINT32 use);

        virtual MUINT32 setHdrState(MUINT32 state);
        virtual MUINT32 getHdrState();

        virtual MBOOL   setZoomRatioList(vector<MUINT32> zoomRatios);
        virtual MBOOL   setEisResult(
                            IImageBuffer*           pImageBuffer,
                            EIS_CROP_INFO           eisCrop,
                            EIS_STATISTIC_STRUCT    eisStatistic);
        virtual MBOOL   getEisStatistic(
                            IImageBuffer*           pImageBuffer,
                            EIS_STATISTIC_STRUCT&   eisStatistic);
        virtual MBOOL   set3AUpdatePeriod(MUINT32 period);
        virtual MBOOL   setIspEnquePeriod(MUINT32 period);
        virtual MUINT32 getIspEnquePeriod();
        virtual MBOOL   setCallbackZoom(
                            MVOID*      pObj,
                            CbZoomFunc  pFunc);
        virtual MBOOL   getCurPass2ZoomInfo(
                            MUINT32&    zoomRatio,
                            MUINT32&    cropX,
                            MUINT32&    cropY,
                            MUINT32&    cropW,
                            MUINT32&    cropH);
        virtual MBOOL   updatePass2ZoomRatio(MINT32 targetZoomRatio);
        virtual MBOOL   setPass1PureRaw(
                            MBOOL     enable,
                            MUINT32    period);
        virtual MBOOL   isPass1PureRawEnable(void);
        virtual MBOOL   getPass1PureRawStatus(MUINT32 magicNum);
    private:
        virtual MBOOL   calRrzoSize(
                            MRect&  crop,
                            MSize&  size,
                            MUINT32 zoomRatio,
                            MSize   prvSize);
        //
        #define ALIGN_UP_SIZE(in,align)     ((in+align-1) & ~(align-1))
        #define ALIGN_DOWN_SIZE(in,align)   (in & ~(align-1))
        #define RRZO_SCALE_DOWN_RATIO_MAX   (0.4)
        #define TABLE_MAX_SIZE              (20)
        //
        typedef struct
        {
            MUINT32         magicNum;
            MUINT32         currentZoomRatio;
            MUINT32         targetZoomRatio;
            MRect           inCrop;
            MSize           outSize;
            MBOOL           waitRemove;
        }PASS1_OUTSIZE_INFO;
        //
        typedef struct
        {
            MUINT32                 magicNum;
            IImageBuffer*           pImgBuf;
            ResultMetadata          metadata;
            MBOOL                   isRrzo;
            EIS_CROP_INFO           eisCrop;
            EIS_STATISTIC_STRUCT    eisStatistic;
        }PASS1_METADATA_INFO;
        //
        typedef enum
        {
            AAA_UPDATE_OK,
            AAA_UPDATE_FAIL,
            AAA_UPDATE_WAIT,
            AAA_UPDATE_SKIP,
            AAA_UPDATE_STOP,
            AAA_UPDATE_RESET
        }AAA_UPDATE_STATE;
        //
        MUINT32 const                       muSensorIdx;
        MUINT32                             muMagicNum;
        MBOOL                               mbPass1SinglePort;
        // protect by a global lock in .cpp
        MUINT32                             muUserCnt;
        //
        MUINT32                             muCurZoomRatio;
        MUINT32                             muCurPass2ZoomRatio;
        MUINT32                             muLastGetPass2ZoomRatio;
        NSCamHW::Rect                       mCurSensorCrop;
        //
        Mutex                               mLock;
        list<MUINT32>                       mlZoomRatio;
        //
        Mutex                               m3ALock;
        AAA_UPDATE_STATE                    m3AUpdateState;
        //
        MBOOL                               mbInitRrzOn;
        MRect                               mInitRrzCrop;
        MSize                               mInitRrzSize;
        //
        MUINT32                             mHDRState;
        MUINT32                             mSensorScenario;
        MUINT32                             mSensorType;
        MSize                               mSensorSize;
        MSize                               mPreviewSize;
        MSize                               mVideoSize;
        MSize                               mRrzoMinSize;
        MSize                               mMaxRrzoSize;
        MSize                               mPass1InitRrzoSize;
        MUINT32                             muFullReplaceCount;
        MUINT32                             muResizedReplaceCount;
        MUINT32                             mRrzoMaxZoomRatio;
        MUINT32                             mIspEnquePeriod;
        MUINT32                             m3AUpdatePeriod;
        MUINT32                             m3AUpdateCount;
        NSImageio::NSIspio::E_ISP_PIXMODE   mIspPixelMode;
        EImageFormat                        mRrzoFormat;
        list<PASS1_OUTSIZE_INFO>            mlPass1OutSize;
        list<PASS1_METADATA_INFO>           mlPass1Metadata;
        vector<MUINT32>                     mvZoomRatioList;
        MUINT32                             mLastZoomIdx;
        MVOID*                              mpCbZoomObj;
        CbZoomFunc                          mpCbZoomFunc;
        // pure raw variable
        MBOOL                               mbPureRaw;          // 0: processed-raw, 1: pure raw
        MUINT32                             muPureRawPeriod;    // frame count interval between two pure raws
        // lock/unlock hw
        Mutex                               mHwLock;
        Condition                           mHwCond;
        MUINT32                             muHwFlag;
};


/*******************************************************************************
*
********************************************************************************/
static Mutex                    g_lock;
static list<IspSyncControlImp*> g_ctrllist;
/*******************************************************************************
*
********************************************************************************/
IspSyncControl*
IspSyncControl::
createInstance(MUINT32 const sensorIdx)
{
    return IspSyncControlImp::createInstance(sensorIdx);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IspSyncControl::
destroyInstance()
{
    return this->destroyInstance();
}


/*******************************************************************************
*
********************************************************************************/
IspSyncControlHw*
IspSyncControlHw::
createInstance(MUINT32 const sensorIdx)
{
    return IspSyncControlImp::createInstance(sensorIdx);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IspSyncControlHw::
destroyInstance()
{
    return this->destroyInstance();
}



/*******************************************************************************
*
********************************************************************************/
IspSyncControlImp*
IspSyncControlImp::
createInstance(MUINT32 const sensorIdx)
{
    Mutex::Autolock _l(g_lock);

    IspSyncControlImp* pSyncCtrl = NULL;
    list<IspSyncControlImp*>::const_iterator iter;
    for( iter = g_ctrllist.begin() ; iter != g_ctrllist.end(); iter++ )
    {
        if( (*iter)->getSensorIdx() == sensorIdx )
            pSyncCtrl = (*iter);
    }

    if( pSyncCtrl == NULL )
    {
        //create new
        pSyncCtrl = new IspSyncControlImp(sensorIdx);
        g_ctrllist.push_back(pSyncCtrl);
    }

    pSyncCtrl->muUserCnt++;
    MY_LOGD("this(0x%x), userCnt(%d)", pSyncCtrl, pSyncCtrl->muUserCnt );

    return pSyncCtrl;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IspSyncControlImp::
destroyInstance()
{
    Mutex::Autolock _l(g_lock);
    muUserCnt--;
    MY_LOGD("this(0x%x), userCnt(%d)", this, muUserCnt);
    if( muUserCnt == 0 )
    {
        list<IspSyncControlImp*>::iterator iter;
        for( iter = g_ctrllist.begin() ; iter != g_ctrllist.end(); iter++ )
        {
            if( (*iter) == this )
            {
                g_ctrllist.erase(iter);
                break;
            }
        }
        delete this;
    }
}


/*******************************************************************************
*
********************************************************************************/
IspSyncControlImp::
IspSyncControlImp(MUINT32 const sensorIdx)
    : muSensorIdx(sensorIdx)
    , muMagicNum(0)
    , mbPass1SinglePort(MFALSE)
    , muUserCnt(0)
    , muCurZoomRatio(100)
    , muCurPass2ZoomRatio(100)
    , muLastGetPass2ZoomRatio(0)
    , mCurSensorCrop(NSCamHW::Rect(0,0,0,0))
    , m3AUpdateState(AAA_UPDATE_OK)
    , mbInitRrzOn(MFALSE)
    , mInitRrzCrop()
    , mInitRrzSize()
    , mHDRState(SENSOR_VHDR_MODE_NONE)
    , mSensorScenario(0)
    , mSensorSize( MSize(0,0) )
    , mPreviewSize( MSize(0,0) )
    , mVideoSize( MSize(0,0) )
    , mRrzoMinSize( MSize(0,0) )
    , mMaxRrzoSize( MSize(0,0) )
    , mPass1InitRrzoSize( MSize(0,0) )
    , muFullReplaceCount(0)
    , muResizedReplaceCount(0)
    , mRrzoMaxZoomRatio(100)
    , mIspEnquePeriod(0)
    , m3AUpdatePeriod(1)
    , m3AUpdateCount(0)
    , mIspPixelMode(NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE)
    , mRrzoFormat(eImgFmt_UNKNOWN)
    , mLastZoomIdx(0)
    , mpCbZoomFunc(NULL)
    , muHwFlag(0)
    , mbPureRaw(MFALSE)
    , muPureRawPeriod(1)
    , mSensorType(0)
{
    MY_LOGD("new instance(0x%x), sensorIdx(%d)", this, getSensorIdx());
    mlZoomRatio.clear();
    mlPass1OutSize.clear();
    mlPass1Metadata.clear();
    mvZoomRatioList.clear();
}


/*******************************************************************************
*
********************************************************************************/
IspSyncControlImp::
~IspSyncControlImp()
{
    MY_LOGD("release instance(0x%x), sensorIdx(%d)", this, getSensorIdx());
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IspSyncControlImp::
addZoomRatio(MUINT32 ratio)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("ratio(%d)", ratio);
    mlZoomRatio.push_back(ratio);
}


/*******************************************************************************
*
********************************************************************************/
MINT32
IspSyncControlImp::
popEndZoomRatio()
{
    Mutex::Autolock _l(mLock);
    MINT32 zoomRatio;
    //
    if(mlZoomRatio.empty())
    {
        MY_LOGD("mlZoomRatio is empty");
        zoomRatio = -1;
    }
    else
    {
        list<MUINT32>::iterator pZoomRatio = mlZoomRatio.end();
        pZoomRatio--;
        zoomRatio = *pZoomRatio;
        mlZoomRatio.erase(pZoomRatio);
        MY_LOGD("zoomRatio(%d)", zoomRatio);
    }
    //
    return zoomRatio;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
IspSyncControlImp::
getZoomRatio()
{
    Mutex::Autolock _l(mLock);
    //
    MUINT32 zoomRatio;
    //
    if(!mlZoomRatio.empty())
    {
        MY_LOGD_IF(1, "Size(%d)", mlZoomRatio.size());
        zoomRatio = *mlZoomRatio.begin();
        mlZoomRatio.erase(mlZoomRatio.begin());
        MY_LOGD_IF(1, "Ratio(%d)", zoomRatio);
    }
    else
    {
        zoomRatio = muCurZoomRatio;
    }
    //
    //
    return zoomRatio;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IspSyncControlImp::
setCurZoomRatio(MUINT32 ratio)
{
    //Mutex::Autolock _l(mLock);
    MY_LOGD("ratio(%d)",ratio);
    muCurZoomRatio = ratio;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setSensorInfo(
    MUINT32 scenario,
    MUINT32 width,
    MUINT32 height,
    MUINT32 type)
{
    MY_LOGD("S(%d),size(%dx%d),T(%d)",
            scenario,
            width,
            height,
            type);
    //
    if( width == 0 ||
        height == 0)
    {
        MY_LOGE("size is 0");
        return MFALSE;
    }
    //
    mSensorScenario = scenario;
    mSensorSize.w = width;
    mSensorSize.h = height;
    mSensorType = type;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getSensorInfo(
    MUINT32& scenario,
    MUINT32& width,
    MUINT32& height,
    MUINT32& type)
{
    if( mSensorSize.w == 0 ||
        mSensorSize.h == 0)
    {
        MY_LOGE("size is 0");
        return MFALSE;
    }
    //
    scenario = mSensorScenario;
    width = mSensorSize.w;
    height = mSensorSize.h;
    type = mSensorType;
    //
    MY_LOGD("S(%d),size(%dx%d),T(%d)",
            scenario,
            width,
            height,
            type);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
calRrzoMaxZoomRatio()
{
    MRect calCrop;
    MSize calSize, sizeList[3] = {mPreviewSize, mVideoSize, mRrzoMinSize};
    MUINT32 i = 0;
    //
    mMaxRrzoSize.w = 0;
    mMaxRrzoSize.h = 0;
    //
    for(i=0; i<3; i++)
    {
        if(mMaxRrzoSize.w < sizeList[i].w)
        {
           mMaxRrzoSize.w = sizeList[i].w;
        }
        if(mMaxRrzoSize.h < sizeList[i].h)
        {
           mMaxRrzoSize.h = sizeList[i].h;
        }
    }
    MY_LOGD("mMaxRrzoSize(%dx%d)",
            mMaxRrzoSize.w,
            mMaxRrzoSize.h);
    //
    if( mRrzoMinSize.w == 0 ||
        mRrzoMinSize.h == 0)
    {
        if(mvZoomRatioList.size() > 0)
        {
            mRrzoMaxZoomRatio = mvZoomRatioList[mvZoomRatioList.size()-1];
        }
        else
        {
            mRrzoMaxZoomRatio = 100;
        }
        goto EXIT;
    }
    //
    if ( mRrzoMinSize.w == mPreviewSize.w &&
        mRrzoMinSize.h == mPreviewSize.h )
    {
        mRrzoMaxZoomRatio = 100;
        goto EXIT;
    }
    //
    if(mvZoomRatioList.size() == 0)
    {
        mRrzoMaxZoomRatio = 100;
        goto EXIT;
    }
    //
    for(i=0; i<mvZoomRatioList.size(); i++)
    {
        calRrzoSize(
            calCrop,
            calSize,
            mvZoomRatioList[i],
            mMaxRrzoSize);
        //
        MY_LOGD("ZR(%d): %d ? %d",
                mvZoomRatioList[i],
                calCrop.s.w,
                mMaxRrzoSize.w);
        if(calCrop.s.w < mMaxRrzoSize.w)
        {
            break;
        }
    }
    //
    if(i == 0)
    {
        mRrzoMaxZoomRatio = mvZoomRatioList[i];
    }
    else
    {
        mRrzoMaxZoomRatio = mvZoomRatioList[i-1];
    }
    //
    EXIT:
    MY_LOGD("mRrzoMaxZoomRatio(%d)",mRrzoMaxZoomRatio);
    return MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setPass1InitRrzoSize(
    MUINT32 width,
    MUINT32 height)
{
    MY_LOGD("size(%d x %d)",
            width,
            height);
    //
    if( width == 0 ||
        height == 0)
    {
        MY_LOGE("size is 0");
        return MFALSE;
    }
    //
    mPass1InitRrzoSize.w = width;
    mPass1InitRrzoSize.h = height;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setPreviewSize(
    MUINT32 width,
    MUINT32 height)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("size(%d x %d)",
            width,
            height);
    //
    if( width == 0 ||
        height == 0)
    {
        MY_LOGE("size is 0");
        return MFALSE;
    }
    //
    mPreviewSize.w = width;
    mPreviewSize.h = height;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setVideoSize(
    MUINT32 width,
    MUINT32 height)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("size(%d x %d)",
            width,
            height);
    //
    mVideoSize.w = width;
    mVideoSize.h = height;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setRrzoMinSize(
    MUINT32 width,
    MUINT32 height)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("size(%d x %d)",
            width,
            height);
    //
    mRrzoMinSize.w = width;
    mRrzoMinSize.h = height;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getPreviewSize(
    MUINT32& width,
    MUINT32& height)
{
    if( mPreviewSize.w == 0 ||
        mPreviewSize.h == 0)
    {
        MY_LOGE("size is 0");
        return MFALSE;
    }
    //
    width = mPreviewSize.w;
    height = mPreviewSize.h;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
calRrzoSize(
    MRect&  crop,
    MSize&  size,
    MUINT32 zoomRatio,
    MSize   prvSize)
{
    NSCamHW::Rect SrcRect(0, 0, mSensorSize.w, mSensorSize.h);
    NSCamHW::Rect DstRect(0, 0, prvSize.w, prvSize.h);
    NSCamHW::Rect CropRect = MtkCamUtils::calCrop(SrcRect, DstRect, zoomRatio);
    //
    crop.p.x = CropRect.x;
    crop.p.y = CropRect.y;
    crop.s.w = CropRect.w;
    crop.s.h = CropRect.h;
    //
    if( crop.s.w <= prvSize.w ||
        crop.s.h <= prvSize.h)
    {
        size.w = crop.s.w;
        size.h = crop.s.h;
    }
    else
    {
        if( crop.s.w * RRZO_SCALE_DOWN_RATIO_MAX > prvSize.w ||
            crop.s.h * RRZO_SCALE_DOWN_RATIO_MAX > prvSize.h)
        {
            size.w = crop.s.w*RRZO_SCALE_DOWN_RATIO_MAX;
            if(size.w < crop.s.w*RRZO_SCALE_DOWN_RATIO_MAX)
            {
                size.w += 1;
            }
            //
            size.h = crop.s.h*RRZO_SCALE_DOWN_RATIO_MAX;
            if(size.h < crop.s.h*RRZO_SCALE_DOWN_RATIO_MAX)
            {
                size.h += 1;
            }
        }
        else
        {
            size.w = prvSize.w;
            size.h = prvSize.h;
        }
    }
    //
    if( ALIGN_UP_SIZE(size.w, 2) <= crop.s.w &&
        ALIGN_UP_SIZE(size.h, 2) <= crop.s.h)
    {
        size.w = ALIGN_UP_SIZE(size.w, 2);
        size.h = ALIGN_UP_SIZE(size.h, 2);
    }
    else
    {
        size.w = ALIGN_DOWN_SIZE(size.w, 2);
        size.h = ALIGN_DOWN_SIZE(size.h, 2);
    }
    //
    MY_LOGV("%d: F(0x%08X),prvSize(%dx%d),C(%d,%d,%dx%d),S(%dx%d)",
            getSensorIdx(),
            mRrzoFormat,
            prvSize.w,
            prvSize.h,
            crop.p.x,
            crop.p.y,
            crop.s.w,
            crop.s.h,
            size.w,
            size.h);
    //
    return MTRUE;
}


/*******************************************************************************
* To fix RRZ in/out setting.
********************************************************************************/
MBOOL
IspSyncControlImp::
setInitialRrzoSize(
    MRect&          crop,
    MSize&          size)
{
    Mutex::Autolock _l(mLock);

    mbInitRrzOn = MTRUE;
    mInitRrzCrop = crop;
    mInitRrzSize = size;

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getInitialRrzoSize(
    EImageFormat    fmt,
    MUINT32         pixelMode,
    MRect&          crop,
    MSize&          size)
{
    Mutex::Autolock _l(mLock);
    //
    NSImageio::NSIspio::ISP_QUERY_RST queryRst;
    mRrzoFormat = fmt;
    //
    if( mbInitRrzOn )
    {
        crop = mInitRrzCrop;
        size = mInitRrzSize;
    }
    else
    if(mHDRState == SENSOR_VHDR_MODE_IVHDR)
    {
        crop.p.x = 0;
        crop.p.y = 0;
        crop.s.w = mSensorSize.w;
        crop.s.h = mSensorSize.h;
        size.w = crop.s.w/2;
        size.h = crop.s.h/2;
    }
    else
    {
        if( mPass1InitRrzoSize.w > 0 &&
            mPass1InitRrzoSize.h > 0)
        {
            calRrzoSize(
                crop,
                size,
                100,
                mPass1InitRrzoSize);
        }
        else
        {
            if( mMaxRrzoSize.w == 0 ||
                mMaxRrzoSize.h == 0)
            {
                calRrzoMaxZoomRatio();
            }
            //
            calRrzoSize(
                crop,
                size,
                100,
                mMaxRrzoSize);
        }
    }
    //
    switch(pixelMode)
    {
        case 0:
        {
            mIspPixelMode = NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE;
            break;
        }
        case 1:
        {
            mIspPixelMode = NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE;
            break;
        }
        default:
        {
            MY_LOGE("Un-supported pixel mode %d",pixelMode);
            mIspPixelMode = NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE;
            break;
        }
    }
    //
    NSImageio::NSIspio::ISP_QuerySize(
                            NSImageio::NSIspio::EPortIndex_RRZO,
                            NSImageio::NSIspio::ISP_QUERY_X_PIX|NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                            mRrzoFormat,
                            size.w,
                            queryRst,
                            mIspPixelMode);
    size.w = queryRst.x_pix;
    //
    MY_LOGD("%d: F(0x%08X),pixelMode(%d,%d),fixed(%d),C(%d,%d,%dx%d),S(%dx%d)",
            getSensorIdx(),
            mRrzoFormat,
            pixelMode,
            mIspPixelMode,
            mbInitRrzOn,
            crop.p.x,
            crop.p.y,
            crop.s.w,
            crop.s.h,
            size.w,
            size.h);
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
updatePass2ZoomRatio(MINT32 targetZoomRatio)
{
    #if !PASS2_ZOOM_ENABLE
    if(muCurPass2ZoomRatio != targetZoomRatio)
    {
        NSCamHW::Rect SrcRect3A(0, 0, mSensorSize.w, mSensorSize.h);
        NSCamHW::Rect DstRect3A(0, 0, mPreviewSize.w, mPreviewSize.h);
        NSCamHW::Rect CropRect3A = MtkCamUtils::calCrop(SrcRect3A, DstRect3A, targetZoomRatio);
        //
        muCurPass2ZoomRatio = targetZoomRatio;
        mCurSensorCrop = CropRect3A;
    }
    #endif
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getPass1Crop(
    MUINT32         inMagicNum,
    MBOOL           inIsRrzo,
    MRect&          outInCrop,
    MSize&          outOutSize)
{
    MUINT32 zoomIdx, currentZoomRatio = 100, targetZoomRatio;
    NSImageio::NSIspio::ISP_QUERY_RST queryRst;
    //
    #if PASS2_ZOOM_ENABLE
    targetZoomRatio = muCurPass2ZoomRatio;
    #else
    targetZoomRatio = getZoomRatio();
    #endif
    //
    Mutex::Autolock _l(mLock);
    //CAM_TRACE_CALL();
    //
    if(inIsRrzo)
    {
        if(mHDRState == SENSOR_VHDR_MODE_IVHDR)
        {
            outInCrop.p.x = 0;
            outInCrop.p.y = 0;
            outInCrop.s.w = mSensorSize.w;
            outInCrop.s.h = mSensorSize.h;
            outOutSize.w = outInCrop.s.w/2;
            outOutSize.h = outInCrop.s.h/2;
        }
        else
        {
            if(targetZoomRatio > mRrzoMaxZoomRatio)
            {
                currentZoomRatio = mRrzoMaxZoomRatio;
            }
            else
            {
                currentZoomRatio = targetZoomRatio;
            }
            //
            calRrzoSize(
                outInCrop,
                outOutSize,
                currentZoomRatio,
                mMaxRrzoSize);
        }
        //
        NSImageio::NSIspio::ISP_QuerySize(
                                NSImageio::NSIspio::EPortIndex_RRZO,
                                NSImageio::NSIspio::ISP_QUERY_X_PIX|NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                mRrzoFormat,
                                outOutSize.w,
                                queryRst,
                                mIspPixelMode);
        outOutSize.w = queryRst.x_pix;
    }
    else
    {
        currentZoomRatio = 100;
        outInCrop.p.x = 0;
        outInCrop.p.y = 0;
        outInCrop.s.w = mSensorSize.w;
        outInCrop.s.h = mSensorSize.h;
        outOutSize.w = mSensorSize.w;
        outOutSize.h = mSensorSize.h;
    }
    //
    updatePass2ZoomRatio(targetZoomRatio);
    //
    PASS1_OUTSIZE_INFO pass1OutSize;
    pass1OutSize.magicNum = inMagicNum;
    pass1OutSize.currentZoomRatio = currentZoomRatio;
    pass1OutSize.targetZoomRatio = targetZoomRatio;
    pass1OutSize.inCrop.p.x = outInCrop.p.x;
    pass1OutSize.inCrop.p.y = outInCrop.p.y;
    pass1OutSize.inCrop.s.w = outInCrop.s.w;
    pass1OutSize.inCrop.s.h = outInCrop.s.h;
    pass1OutSize.outSize.w = outOutSize.w;
    pass1OutSize.outSize.h = outOutSize.h;
    pass1OutSize.waitRemove = MFALSE;
    mlPass1OutSize.push_back(pass1OutSize);
    //
    if(mlPass1OutSize.size() > TABLE_MAX_SIZE)
    {
        MY_LOGE("Pass1OutSize table overflow(%d)",mlPass1OutSize.size());
        AEE_ASSERT("Pass1OutSize table overflow");
    }
    //
    #if !PASS2_ZOOM_ENABLE
    for(zoomIdx=0; zoomIdx<mvZoomRatioList.size(); zoomIdx++)
    {
        if(targetZoomRatio == mvZoomRatioList[zoomIdx])
        {
            break;
        }
    }
    //
    if( mpCbZoomFunc != NULL &&
        zoomIdx < mvZoomRatioList.size())
    {
        if(zoomIdx != mLastZoomIdx)
        {
            (*mpCbZoomFunc)(
                mpCbZoomObj,
                zoomIdx,
                0,
                0);
            mLastZoomIdx = zoomIdx;
        }
        else
        {
            MY_LOGD("Same zoom idx(%d)",zoomIdx);
        }

    }
    #endif
    //
    MY_LOGD("%d: TS(%d),# 0x%X,IsR(%d),ZR(%d,%d/%d),C(%d,%d,%dx%d),S(%dx%d)",
            getSensorIdx(),
            mlPass1OutSize.size(),
            inMagicNum,
            inIsRrzo,
            zoomIdx,
            currentZoomRatio,
            targetZoomRatio,
            outInCrop.p.x,
            outInCrop.p.y,
            outInCrop.s.w,
            outInCrop.s.h,
            outOutSize.w,
            outOutSize.h);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
queryPass1CropInfo(
    MUINT32         inMagicNum,
    MUINT32&        currentZoomRatio,
    MUINT32&        targetZoomRatio,
    MRect&          outInCrop,
    MSize&          outOutSize)
{
    list<PASS1_OUTSIZE_INFO>::iterator iterOutSize;
    for(iterOutSize = mlPass1OutSize.begin(); iterOutSize != mlPass1OutSize.end(); iterOutSize++)
    {
        if((*iterOutSize).magicNum == inMagicNum)
        {
            currentZoomRatio = (*iterOutSize).currentZoomRatio;
            targetZoomRatio = (*iterOutSize).targetZoomRatio;
            outInCrop.p.x = (*iterOutSize).inCrop.p.x;
            outInCrop.p.y = (*iterOutSize).inCrop.p.y;
            outInCrop.s.w = (*iterOutSize).inCrop.s.w;
            outInCrop.s.h = (*iterOutSize).inCrop.s.h;
            outOutSize.w = (*iterOutSize).outSize.w;
            outOutSize.h = (*iterOutSize).outSize.h;
            //
            MY_LOGV("# 0x%X,ZR(%d/%d),C(%d,%d,%dx%d),S(%dx%d)",
                    inMagicNum,
                    currentZoomRatio,
                    targetZoomRatio,
                    outInCrop.p.x,
                    outInCrop.p.y,
                    outInCrop.s.w,
                    outInCrop.s.h,
                    outOutSize.w,
                    outOutSize.h);
            //
            return MTRUE;
        }
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
addPass1Info(
    MUINT32         inMagicNum,
    IImageBuffer*   inpImgBuf,
    ResultMetadata  inMetadata,
    MBOOL           inIsRrzo)
{
    Mutex::Autolock _l(mLock);
    //CAM_TRACE_CALL();
    //
    if(inpImgBuf == NULL)
    {
        MY_LOGE("inpImgBuf = NULL");
        return MFALSE;
    }
    //
    MBOOL isNewMagicNum = MTRUE, isFindImgSize = MFALSE;
    list<PASS1_OUTSIZE_INFO>::iterator iterOutSize;
    list<PASS1_METADATA_INFO>::iterator iterMetadata;
    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
    {
        if( (*iterMetadata).magicNum == inMagicNum &&
            (*iterMetadata).isRrzo == inIsRrzo)
        {
            MY_LOGV("%d: Old # 0x%X,IsR(%d)",
                    getSensorIdx(),
                    inMagicNum,
                    inIsRrzo);
            isNewMagicNum = MFALSE;
            break;
        }
    }
    //
    for(iterOutSize = mlPass1OutSize.begin(); iterOutSize != mlPass1OutSize.end();)
    {
        if( isFindImgSize == MFALSE &&
            inIsRrzo == MTRUE &&
            (*iterOutSize).magicNum == inMagicNum)
        {
            inpImgBuf->setExtParam((*iterOutSize).outSize);
            isFindImgSize = MTRUE;
            //
            if(!isNewMagicNum)
            {
                break;
            }
        }
        //
        if( (*iterOutSize).waitRemove &&
            isNewMagicNum)
        {
            if((*iterOutSize).magicNum == inMagicNum)
            {
                MY_LOGV("Still using # 0x%X",(*iterOutSize).magicNum);
                (*iterOutSize).waitRemove = MFALSE;
                iterOutSize++;
            }
            else
            {
                MY_LOGV("Remove # 0x%X",(*iterOutSize).magicNum);
                iterOutSize = mlPass1OutSize.erase(iterOutSize);
            }
        }
        else
        {
            iterOutSize++;
        }
    }
    //
    if(isNewMagicNum)
    {
        MY_LOGV("%d: New # 0x%X", getSensorIdx(), inMagicNum);
    }
    //
    PASS1_METADATA_INFO pass1Metadata;
    pass1Metadata.magicNum = inMagicNum;
    pass1Metadata.pImgBuf = inpImgBuf;
    pass1Metadata.metadata = inMetadata;
    pass1Metadata.isRrzo = inIsRrzo;
    pass1Metadata.eisCrop.enable = MFALSE;
    mlPass1Metadata.push_back(pass1Metadata);
    //
    if(mlPass1Metadata.size() > TABLE_MAX_SIZE)
    {
        MY_LOGE("Pass1Metadata table overflow(%d)",mlPass1Metadata.size());
        AEE_ASSERT("Pass1Metadata table overflow");
    }
    //
    MY_LOGD("%d: TS(%d),# 0x%X,VA(0x%X),IsR(%d),IsNew(%d)",
            getSensorIdx(),
            mlPass1Metadata.size(),
            inMagicNum,
            (MUINTPTR)inpImgBuf,
            inIsRrzo,
            isNewMagicNum);
    //
    MY_LOGV("metadata: time %lld, C(%d,%d,%dx%d_(%d,%d,%dx%d)), priv 0x%x/%d",
            inMetadata.mTimeStamp,
            inMetadata.mCrop_s.p.x,
            inMetadata.mCrop_s.p.y,
            inMetadata.mCrop_s.s.w,
            inMetadata.mCrop_s.s.h,
            inMetadata.mCrop_d.p.x,
            inMetadata.mCrop_d.p.y,
            inMetadata.mCrop_d.s.w,
            inMetadata.mCrop_d.s.h,
            inMetadata.mPrivateData,
            inMetadata.mPrivateDataSize);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getPass2Info(
    IImageBuffer*   inpImgBuf,
    MSize           inOutImgSize,
    MUINT32&        outMagicNum,
    MVOID*&         outpPrivateData,
    MUINT32&        outPrivateDataSize,
    MCropRect&      outInCrop)
{
    if(inpImgBuf == NULL)
    {
        MY_LOGE("inpImgBuf = NULL");
        return MFALSE;
    }
    //
    MBOOL eisEnable = MFALSE;
    MINT32 zoomIdx = 0 ,targetZoomRatio = 0, currentZoomRatio = 0;
    #if PASS2_ZOOM_ENABLE
    muCurPass2ZoomRatio = getZoomRatio();
    #endif
    //
    Mutex::Autolock _l(mLock);
    //CAM_TRACE_CALL();
    //
    list<PASS1_OUTSIZE_INFO>::iterator iterOutSize;
    list<PASS1_METADATA_INFO>::iterator iterMetadata;
    //
    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
    {
        if((*iterMetadata).pImgBuf == inpImgBuf)
        {
            outMagicNum = (*iterMetadata).magicNum;
            outpPrivateData = (*iterMetadata).metadata.mPrivateData;
            outPrivateDataSize = (*iterMetadata).metadata.mPrivateDataSize;
            //
            for(iterOutSize = mlPass1OutSize.begin(); iterOutSize != mlPass1OutSize.end(); iterOutSize++)
            {
                if((*iterOutSize).magicNum == (*iterMetadata).magicNum)
                {
                    if((*iterMetadata).isRrzo)
                    {
                        MSize dstSize;
                        targetZoomRatio = (*iterOutSize).targetZoomRatio;
                        currentZoomRatio = (*iterOutSize).currentZoomRatio;
                        //
                        NSCamHW::Rect SrcRectP1(0, 0, mSensorSize.w, mSensorSize.h);
                        NSCamHW::Rect DstRectP1(0, 0, inOutImgSize.w, inOutImgSize.h);
                        NSCamHW::Rect CropRectP1 = MtkCamUtils::calCrop(SrcRectP1, DstRectP1, currentZoomRatio);
                        NSCamHW::Rect CropRect;

                        //
                        if( CropRectP1.w == (*iterOutSize).inCrop.s.w &&
                            CropRectP1.h == (*iterOutSize).inCrop.s.h)
                        {
                            CropRect.x = 0;
                            CropRect.y = 0;
                            CropRect.w = (*iterOutSize).outSize.w;
                            CropRect.h = (*iterOutSize).outSize.h;
                        }
                        else
                        {
                            NSCamHW::Rect SrcRect(0, 0, (*iterOutSize).outSize.w, (*iterOutSize).outSize.h);
                            NSCamHW::Rect DstRect(0, 0, inOutImgSize.w, inOutImgSize.h);
                            CropRect = MtkCamUtils::calCrop(SrcRect, DstRect, 100);
                        }
                        //
                        #if PASS2_ZOOM_ENABLE
                        if((*iterOutSize).targetZoomRatio < muCurPass2ZoomRatio)
                        {
                            dstSize.w = ((CropRect.w*(*iterOutSize).currentZoomRatio)+muCurZoomRatio>>1)/muCurZoomRatio;
                            dstSize.h = ((CropRect.h*(*iterOutSize).currentZoomRatio)+muCurZoomRatio>>1)/muCurZoomRatio;
                        }
                        else
                        #endif
                        {
                            if((*iterOutSize).currentZoomRatio == (*iterOutSize).targetZoomRatio)
                            {
                                dstSize.w = CropRect.w;
                                dstSize.h = CropRect.h;
                            }
                            else
                            {
                                dstSize.w = (((CropRect.w*(*iterOutSize).currentZoomRatio)+(CropRect.w*(*iterOutSize).currentZoomRatio))>>1)/(*iterOutSize).targetZoomRatio;
                                dstSize.h = (((CropRect.h*(*iterOutSize).currentZoomRatio)+(CropRect.h*(*iterOutSize).currentZoomRatio))>>1)/(*iterOutSize).targetZoomRatio;
                            }
                        }
                        outInCrop.p_integral.x = (((*iterOutSize).outSize.w - dstSize.w)+1) / 2;
                        outInCrop.p_integral.y = (((*iterOutSize).outSize.h - dstSize.h)+1) / 2;
                        outInCrop.s.w = ALIGN_UP_SIZE(dstSize.w, 2);
                        outInCrop.s.h = ALIGN_UP_SIZE(dstSize.h, 2);
                    }
                    else
                    {
                        NSCamHW::Rect SrcRect(0, 0, mSensorSize.w, mSensorSize.h);
                        NSCamHW::Rect DstRect(0, 0, inOutImgSize.w, inOutImgSize.h);
                        NSCamHW::Rect CropRect = MtkCamUtils::calCrop(SrcRect, DstRect, (*iterOutSize).targetZoomRatio);
                        //
                        outInCrop.p_integral.x = CropRect.x;
                        outInCrop.p_integral.y = CropRect.y;
                        outInCrop.s.w = ALIGN_UP_SIZE(CropRect.w, 2);
                        outInCrop.s.h = ALIGN_UP_SIZE(CropRect.h, 2);
                    }
                    //
                    eisEnable = (*iterMetadata).eisCrop.enable;
                    //
                    break;
                }
            }
            //
            if(iterOutSize == mlPass1OutSize.end())
            {
                MY_LOGE("Can't find # 0x%X",outMagicNum);
                AEE_ASSERT("Can't find magic number");
            }
            //
            break;
        }
    }
    //
    if(iterMetadata == mlPass1Metadata.end())
    {
        MY_LOGE("Can't find ImgBuf(0x%X)",(MUINTPTR)inpImgBuf);
    }
    else
    {
        if((*iterMetadata).eisCrop.enable)
        {
           MY_LOGV("%d: EIS:X(%d/%10d),Y(%d/%10d)",
                    getSensorIdx(),
                    (*iterMetadata).eisCrop.xInt,
                    (*iterMetadata).eisCrop.xFlt,
                    (*iterMetadata).eisCrop.yInt,
                    (*iterMetadata).eisCrop.yFlt);
            //
            outInCrop.p_integral.x = outInCrop.p_integral.x * (*iterMetadata).eisCrop.scaleFactor + (*iterMetadata).eisCrop.xInt;
            outInCrop.p_integral.y = outInCrop.p_integral.y * (*iterMetadata).eisCrop.scaleFactor + (*iterMetadata).eisCrop.yInt;
            outInCrop.s.w = outInCrop.s.w * (*iterMetadata).eisCrop.scaleFactor;
            outInCrop.s.h = outInCrop.s.h * (*iterMetadata).eisCrop.scaleFactor;
            outInCrop.s.w = ALIGN_DOWN_SIZE(outInCrop.s.w, 2);
            outInCrop.s.h = ALIGN_DOWN_SIZE(outInCrop.s.h, 2);
            outInCrop.p_fractional.x = (*iterMetadata).eisCrop.xFlt;
            outInCrop.p_fractional.y = (*iterMetadata).eisCrop.yFlt;
        }
        else
        {
            outInCrop.p_fractional.x = 0;
            outInCrop.p_fractional.y = 0;
        }
    }
    //
    #if PASS2_ZOOM_ENABLE
    targetZoomRatio = muCurPass2ZoomRatio;
    //
    for(zoomIdx=0; zoomIdx<mvZoomRatioList.size(); zoomIdx++)
    {
        if(targetZoomRatio == mvZoomRatioList[zoomIdx])
        {
            break;
        }
    }
    //
    if( mpCbZoomFunc != NULL &&
        zoomIdx < mvZoomRatioList.size())
    {
        if(zoomIdx != mLastZoomIdx)
        {
            (*mpCbZoomFunc)(
                mpCbZoomObj,
                zoomIdx,
                0,
                0);
            mLastZoomIdx = zoomIdx;
        }
        else
        {
            MY_LOGD("Same zoom idx(%d)",zoomIdx);
        }
    }
    else
    {
        zoomIdx = -1;
    }
    #endif
    //
    if(eisEnable)
    {
        MY_LOGD("%d: ImgBuf(0x%X),# 0x%X,PD(0x%X/%d),ZR(%d,%d/%d),S(%dx%d),C(%d/%10d,%d/%10d,%dx%d),EIS:X(%d/%10d),Y(%d/%10d)",
                getSensorIdx(),
                (MUINTPTR)inpImgBuf,
                outMagicNum,
                (MUINTPTR)outpPrivateData,
                outPrivateDataSize,
                zoomIdx,
                currentZoomRatio,
                targetZoomRatio,
                inOutImgSize.w,
                inOutImgSize.h,
                outInCrop.p_integral.x,
                outInCrop.p_fractional.x,
                outInCrop.p_integral.y,
                outInCrop.p_fractional.y,
                outInCrop.s.w,
                outInCrop.s.h,
                (*iterMetadata).eisCrop.xInt,
                (*iterMetadata).eisCrop.xFlt,
                (*iterMetadata).eisCrop.yInt,
                (*iterMetadata).eisCrop.yFlt);
    }
    else
    {
        MY_LOGD("%d: ImgBuf(0x%X),# 0x%X,PD(0x%X/%d),ZR(%d,%d/%d),S(%dx%d),C(%d,%d,%dx%d)",
                getSensorIdx(),
                (MUINTPTR)inpImgBuf,
                outMagicNum,
                (MUINTPTR)outpPrivateData,
                outPrivateDataSize,
                zoomIdx,
                currentZoomRatio,
                targetZoomRatio,
                inOutImgSize.w,
                inOutImgSize.h,
                outInCrop.p_integral.x,
                outInCrop.p_integral.y,
                outInCrop.s.w,
                outInCrop.s.h);
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
dropPass2Info(IImageBuffer* pImgBuf)
{
    Mutex::Autolock _l(mLock);
    //CAM_TRACE_CALL();
    //
    if(pImgBuf == NULL)
    {
        MY_LOGE("pImgBuf = NULL");
        return MFALSE;
    }
    //
    MBOOL isFind = MFALSE, isEmpty = MTRUE, isMarkNeedRemove = MFALSE, isExistInMetadata = MFALSE;
    MUINT32 magicNum = 0;
    list<PASS1_OUTSIZE_INFO>::iterator iterOutSize, iterOutSize2;
    list<PASS1_METADATA_INFO>::iterator iterMetadata;
    //
    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
    {
        if((*iterMetadata).pImgBuf == pImgBuf)
        {
            MY_LOGD("Erase # 0x%X,ImgBuf(0x%X),IsR(%d)",
                    (*iterMetadata).magicNum,
                    (MUINTPTR)((*iterMetadata).pImgBuf),
                    (*iterMetadata).isRrzo);
            magicNum = (*iterMetadata).magicNum;
            mlPass1Metadata.erase(iterMetadata);
            isFind = MTRUE;
            break;
        }
    }
    //
    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
    {
        if((*iterMetadata).magicNum == magicNum)
        {
            isEmpty = MFALSE;
            break;
        }
    }
    //
    if(isEmpty)
    {
        for(iterOutSize = mlPass1OutSize.begin(); iterOutSize != mlPass1OutSize.end(); iterOutSize++)
        {
            if((*iterOutSize).magicNum == magicNum)
            {
                MY_LOGV("Wait remove # 0x%X",(*iterOutSize).magicNum);
                (*iterOutSize).waitRemove = MTRUE;
                isMarkNeedRemove = MTRUE;
                iterOutSize2 = iterOutSize;
                break;
            }
        }
        //
        if(isMarkNeedRemove)
        {
            for(iterOutSize = mlPass1OutSize.begin(); iterOutSize != iterOutSize2; iterOutSize++)
            {
                if((*iterOutSize).waitRemove == MFALSE)
                {
                    isExistInMetadata = MFALSE;
                    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
                    {
                        if((*iterMetadata).magicNum == (*iterOutSize).magicNum)
                        {
                            isExistInMetadata = MTRUE;
                            break;
                        }
                    }
                    //
                    if(!isExistInMetadata)
                    {
                        MY_LOGV("Wait remove # 0x%X",(*iterOutSize).magicNum);
                        (*iterOutSize).waitRemove = MTRUE;
                    }
                }
            }
        }
    }
    //
    if(!isFind)
    {
        MY_LOGD("Can't find ImgBuf(0x%X)",(MUINTPTR)pImgBuf);
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
queryImgBufInfo(
    IImageBuffer*   inpImgBuf,
    MUINT32&        outMagicNum,
    MBOOL&          outIsRrzo,
    MVOID*&         outpPrivateData,
    MUINT32&        outPrivateDataSize)
{
    Mutex::Autolock _l(mLock);
    //CAM_TRACE_CALL();
    //
    if(inpImgBuf == NULL)
    {
        MY_LOGE("inpImgBuf = NULL");
        return MFALSE;
    }
    //
    list<PASS1_METADATA_INFO>::iterator iterMetadata;
    //
    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
    {
        if((*iterMetadata).pImgBuf == inpImgBuf)
        {
            outMagicNum = (*iterMetadata).magicNum;
            outIsRrzo = (*iterMetadata).isRrzo;
            outpPrivateData = (*iterMetadata).metadata.mPrivateData;
            outPrivateDataSize = (*iterMetadata).metadata.mPrivateDataSize;
            //
            return MTRUE;
        }
    }
    //
    MY_LOGE("cannot find infos for buf(0x%X)",(MUINTPTR)inpImgBuf);
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
IspSyncControlImp::
getMagicNum(MBOOL bInit)
{
    Mutex::Autolock _l(m3ALock);
    MUINT32 cur3AUpdateCount = m3AUpdateCount, ret = MAGIC_NUM_INVALID;
    AAA_UPDATE_STATE nextState = m3AUpdateState;
    //
    if(bInit)
    {
        m3AUpdateCount = 0;
    }
    //
    if(m3AUpdateCount == 0)
    {
        switch(m3AUpdateState)
        {
            case AAA_UPDATE_OK:
                ret = muMagicNum;
                if(muMagicNum == MAGIC_NUM_MAX)
                {
                    muMagicNum = 0;
                }
                else
                {
                    muMagicNum++;
                }
                nextState = AAA_UPDATE_WAIT;
                break;
            case AAA_UPDATE_FAIL:
                nextState = AAA_UPDATE_OK;
                break;
            case AAA_UPDATE_WAIT:
                nextState = AAA_UPDATE_SKIP;
                break;
            case AAA_UPDATE_SKIP:
                break;
            case AAA_UPDATE_STOP:
                break;
            default:
                break;
        }
    }
    //
    if( m3AUpdateCount == 0 &&
        m3AUpdateState != AAA_UPDATE_OK)
    {
        //Do nothing.
    }
    else
    {
        if(!bInit)
        {
            m3AUpdateCount++;
        }
    }
    //
    if(m3AUpdateCount >= m3AUpdatePeriod)
    {
       m3AUpdateCount = 0;
    }
    //
    if(m3AUpdatePeriod > 1)
    {
        MY_LOGD("update init(%d) cnt(%d -> %d, %d) state %d -> %d",
                bInit,
                cur3AUpdateCount,
                m3AUpdateCount,
                m3AUpdatePeriod,
                m3AUpdateState,
                nextState);
    }
    m3AUpdateState = nextState;
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
send3AUpdateCmd(UPDATE_CMD cmd)
{
    Mutex::Autolock _l(m3ALock);
    MBOOL ret = MTRUE;
    AAA_UPDATE_STATE nextState = m3AUpdateState;
    //
    switch(m3AUpdateState)
    {
        case AAA_UPDATE_OK:
            if( cmd == UPDATE_CMD_STOP )
                nextState = AAA_UPDATE_STOP;
            else
                ret = MFALSE;
            break;
        case AAA_UPDATE_FAIL:
            ret = MFALSE;
            break;
        case AAA_UPDATE_WAIT:
            if( cmd == UPDATE_CMD_OK )
                nextState = AAA_UPDATE_OK;
            else if( cmd == UPDATE_CMD_FAIL )
                nextState = AAA_UPDATE_FAIL;
            else if( cmd == UPDATE_CMD_STOP )
                nextState = AAA_UPDATE_STOP;
            else
                ret = MFALSE;
            break;
        case AAA_UPDATE_SKIP:
            if( cmd == UPDATE_CMD_OK || cmd == UPDATE_CMD_FAIL )
                nextState = AAA_UPDATE_OK;
            else if( cmd == UPDATE_CMD_STOP )
                nextState = AAA_UPDATE_STOP;
            else
                ret = MFALSE;
            break;
        case AAA_UPDATE_STOP:
            if( cmd == UPDATE_CMD_START )
                nextState = AAA_UPDATE_OK;
            else
                ret = MFALSE;
            break;
        case AAA_UPDATE_RESET:
            m3AUpdateCount = 0;
            break;
        default:
            break;
    }

    if( !ret )
    {
        MY_LOGE("cur state %d, cmd %d", m3AUpdateState, cmd);
    }

    MY_LOGV("cmd %d: state %d -> %d", cmd, m3AUpdateState, nextState);
    m3AUpdateState = nextState;
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
clearTable()
{
    FUNC_NAME;
    mlPass1OutSize.clear();
    mlPass1Metadata.clear();
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
addPass1ReplaceBuffer(MUINT32 const fullcnt, MUINT32 const resizedcnt)
{
    MY_LOGD("set replace buf count %d, %d", fullcnt, resizedcnt);
    //
    muFullReplaceCount = fullcnt;
    muResizedReplaceCount = resizedcnt;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
IspSyncControlImp::
getPass1ReplaceCnt(MBOOL const bFull) const
{
    return bFull ? muFullReplaceCount : muResizedReplaceCount;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
lockHw(MUINT32 const use)
{
    // only pass1 need to lock
#define P2Sequential (0)
#if P2Sequential
    const MUINT32 lockUse = use & HW_PASS1 & HW_PASS2;
#else
    const MUINT32 lockUse = use & HW_PASS1;
#endif
    Mutex::Autolock _l(mHwLock);
    while( muHwFlag & lockUse ){
        MY_LOGD("wait for 0x%x release, cur 0x%x", use, muHwFlag);
        mHwCond.wait(mHwLock);
    }

    MY_LOGV("lock 0x%x", use);
    muHwFlag |= lockUse;

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
unlockHw(MUINT32 const use)
{
    Mutex::Autolock _l(mHwLock);
    muHwFlag &= ~use;
    mHwCond.broadcast();
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
IspSyncControlImp::
setHdrState(MUINT32 state)
{
    MY_LOGD("%d",state);
    mHDRState = state;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
IspSyncControlImp::
getHdrState()
{
    MY_LOGD("%d",mHDRState);
    return mHDRState;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setZoomRatioList(vector<MUINT32> zoomRatios)
{
    FUNC_NAME;
    mvZoomRatioList = zoomRatios;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setEisResult(
    IImageBuffer*           pImageBuffer,
    EIS_CROP_INFO           eisCrop,
    EIS_STATISTIC_STRUCT    eisStatistic)
{
    Mutex::Autolock _l(mLock);
    /*
    MY_LOGD("EN(%d),VA(0x%X)",
            eisCrop.enable,
            (MUINTPTR)pImageBuffer);
    */
    MBOOL isFind = MFALSE;
    list<PASS1_METADATA_INFO>::iterator iterMetadata;
    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
    {
        if((*iterMetadata).pImgBuf == pImageBuffer)
        {
            (*iterMetadata).eisCrop = eisCrop;
            //MY_LOGD("buf=%x, eisStatistic.i4LMV_X[0]=%d", pImageBuffer, eisStatistic.i4LMV_X[0]);
            (*iterMetadata).eisStatistic = eisStatistic;
            isFind = MTRUE;
            break;
        }
    }
    //
    if(isFind)
    {
        //MY_LOGD("Find VA(0x%X)",(MUINTPTR)pImageBuffer);
        return MTRUE;
    }
    else
    {
        MY_LOGE("Can't find VA(0x%X)",(MUINTPTR)pImageBuffer);
        return MFALSE;
    }
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getEisStatistic(
    IImageBuffer*           pImageBuffer,
    EIS_STATISTIC_STRUCT&   eisStatistic)
{
    //Mutex::Autolock _l(mLock);
    /*
    MY_LOGD("EN(%d),VA(0x%X)",
            eisCrop.enable,
            (MUINTPTR)pImageBuffer);
    */
    MBOOL isFind = MFALSE;
    list<PASS1_METADATA_INFO>::iterator iterMetadata;
    for(iterMetadata = mlPass1Metadata.begin(); iterMetadata != mlPass1Metadata.end(); iterMetadata++)
    {
        if((*iterMetadata).pImgBuf == pImageBuffer)
        {
            eisStatistic = (*iterMetadata).eisStatistic;
            //MY_LOGD("buf=%x, eisStatistic.i4LMV_X[0]=%d", pImageBuffer, eisStatistic.i4LMV_X[0]);
            isFind = MTRUE;
            break;
        }
    }
    //
    if(isFind)
    {
        //MY_LOGD("Find VA(0x%X)",(MUINTPTR)pImageBuffer);
        return MTRUE;
    }
    else
    {
        MY_LOGE("Can't find VA(0x%X)",(MUINTPTR)pImageBuffer);
        return MFALSE;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
set3AUpdatePeriod(MUINT32 period)
{
    MY_LOGD("period(%d)",period);
    m3AUpdatePeriod = period;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setIspEnquePeriod(MUINT32 period)
{
    MY_LOGD("period(%d)",period);
    mIspEnquePeriod = period;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
IspSyncControlImp::
getIspEnquePeriod()
{
    return mIspEnquePeriod;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setCallbackZoom(
    MVOID*      pObj,
    CbZoomFunc  pFunc)
{
    Mutex::Autolock _l(mLock);
    FUNC_NAME;
    MY_LOGD("Obj(0x%X),Func(0x%X)",
            (MUINTPTR)pObj,
            (MUINTPTR)pFunc);
    mpCbZoomObj = pObj;
    mpCbZoomFunc = pFunc;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getCurPass2ZoomInfo(
    MUINT32&    zoomRatio,
    MUINT32&    cropX,
    MUINT32&    cropY,
    MUINT32&    cropW,
    MUINT32&    cropH)
{
    Mutex::Autolock _l(mLock);
    //
    MBOOL ret = MFALSE;
    //
    if( muLastGetPass2ZoomRatio != muCurPass2ZoomRatio &&
        muCurPass2ZoomRatio > 0)
    {
        muLastGetPass2ZoomRatio = muCurPass2ZoomRatio;
        zoomRatio = muCurPass2ZoomRatio;
        //
        if( mCurSensorCrop.w > 0 &&
            mCurSensorCrop.h > 0)
        {
            cropX = mCurSensorCrop.x;
            cropY = mCurSensorCrop.y;
            cropW = mCurSensorCrop.w;
            cropH = mCurSensorCrop.h;
        }
        else
        {
            NSCamHW::Rect SrcRect(0, 0, mSensorSize.w, mSensorSize.h);
            NSCamHW::Rect DstRect(0, 0, mPreviewSize.w, mPreviewSize.h);
            NSCamHW::Rect CropRect = MtkCamUtils::calCrop(SrcRect, DstRect, muCurPass2ZoomRatio);

            cropX = CropRect.x;
            cropY = CropRect.y;
            cropW = CropRect.w;
            cropH = CropRect.h;
        }
        ret = MTRUE;
    }
    MY_LOGD_IF(0,"ret(%d),ZR(%d),Crop(%d,%d,%dx%d)",ret,zoomRatio,cropX,cropY,cropW,cropH);
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
setPass1PureRaw(
    MBOOL       enable,
    MUINT32     period)
{
    Mutex::Autolock _l(mLock);
    FUNC_NAME;
    MY_LOGD("enable(%d),period(%d)",
            (MUINT32)enable,
            (MUINT32)period);
    mbPureRaw = enable;
    muPureRawPeriod= period;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
isPass1PureRawEnable(void)
{
    FUNC_NAME;
    MY_LOGD("enable(%d)", (MUINT32)mbPureRaw);
    return mbPureRaw;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IspSyncControlImp::
getPass1PureRawStatus(MUINT32 magicNum)
{
    FUNC_NAME;
    MBOOL ret = MTRUE;
    if ((magicNum % muPureRawPeriod) == 0)
    {
        ret = MTRUE;
    }
    else
    {
        ret = MFALSE;
    }
    MY_LOGD("magicNum(%d), pureraw(%d)", (MUINT32)magicNum, (MUINT32)ret);
    return ret;
}


}; // namespace NSCamNode

