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
#define LOG_TAG "MtkCam/BShot"
//
#include <mtkcam/Log.h>
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __func__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
//
#include <mtkcam/common.h>
//
#include <mtkcam/camnode/ICamGraphNode.h>
#include <mtkcam/camnode/pass1node.h>
#include <mtkcam/camnode/pass2node.h>
#include <mtkcam/camnode/JpegEncNode.h>
#include <mtkcam/camnode/ShotCallbackNode.h>
#include <mtkcam/camnode/AllocBufHandler.h>
#include "../inc/BShotCtrlNode.h"

using namespace NSCamNode;

#include <mtkcam/featureio/aaa_hal_common.h>
using namespace NS3A;
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>
//
#include "../inc/CamShotImp.h"
#include "../inc/BurstShot.h"
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
// buffer usage
#include <mtkcam/iopipe/Port.h>

// for debug dump
#include <cutils/properties.h>
using namespace android;
#include <mtkcam/hal/IHalSensor.h>

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

//used nodes
enum
{
    PASS1NODE    = 0x0 ,
    CTRLNODE           ,
    CAPPASS2NODE       ,
    HWJPEGNODE         ,
    SWJPEGNODE         ,
    CALLBACKNODE       ,
};

enum
{
    DATA_FULLRAW             = 0x1   ,
    DATA_RESIZEDRAW          = 0x2   ,
    DATA_MAINYUV             = 0x4   ,
    DATA_THUMBNAILYUV        = 0x8   ,
    DATA_POSTVIEW            = 0x10  ,
    DATA_MAINJPEG            = 0x20  ,
    DATA_THUMBNAILJPEG       = 0x40  ,
    DATA_DEBUGINFO           = 0x80  ,
    DATA_DUMP_IMG3O          = 0x100 ,
    // extend for connection
    DATA_FULLRAW_CB          = 0x200 ,
    DATA_MAINYUV_CB          = 0x400 ,
    DATA_POSTVIEW_CB         = 0x800 ,
    DATA_MAINJPEG_CB         = 0x1000,
    DATA_THUMBNAILJPEG_CB    = 0x2000,
    DATA_EIS_CB              = 0x4000,
};
#define MAP_TYPE2BIT(datatype, bit) \
    static const MUINT32 datatype##_bit = (1<<(bit));
#define bitOf(datatype) (datatype##_bit)

MAP_TYPE2BIT(PASS1_FULLRAW        , 0);
MAP_TYPE2BIT(PASS1_RESIZEDRAW     , 1);
MAP_TYPE2BIT(PASS2_PRV_SRC        , 2);
MAP_TYPE2BIT(PASS2_PRV_DST_0      , 3);
MAP_TYPE2BIT(PASS2_PRV_DST_1      , 4);
MAP_TYPE2BIT(PASS2_CAP_SRC        , 5);
MAP_TYPE2BIT(PASS2_CAP_DST_0      , 6);
MAP_TYPE2BIT(PASS2_CAP_DST_1      , 7);
MAP_TYPE2BIT(PASS2_CAP_DST_2      , 8);
MAP_TYPE2BIT(JPEG_ENC_SW_SRC      , 9);
MAP_TYPE2BIT(JPEG_ENC_SW_DST      , 10);
MAP_TYPE2BIT(JPEG_ENC_HW_SRC      , 11);
MAP_TYPE2BIT(JPEG_ENC_HW_DST      , 12);
MAP_TYPE2BIT(CONTROL_FULLRAW      , 13);
MAP_TYPE2BIT(CONTROL_RESIZEDRAW   , 14);
MAP_TYPE2BIT(CONTROL_PRV_SRC      , 15);
MAP_TYPE2BIT(CONTROL_CAP_SRC      , 16);
MAP_TYPE2BIT(CONTROL_DBGINFO      , 17);
MAP_TYPE2BIT(CONTROL_EISDATA      , 18);
MAP_TYPE2BIT(SHOTCB_RAWDATA_SRC   , 19);
MAP_TYPE2BIT(SHOTCB_YUVDATA_SRC   , 20);
MAP_TYPE2BIT(SHOTCB_POSTVIEW_SRC  , 21);
MAP_TYPE2BIT(SHOTCB_JPEG_SRC      , 22);
MAP_TYPE2BIT(SHOTCB_DBGINFO_SRC   , 23);
MAP_TYPE2BIT(SHOTCB_EISDATA_SRC   , 24);
MAP_TYPE2BIT(SHOTCB_THUMBNAIL_SRC , 25);
MAP_TYPE2BIT(SHOTCB_RAWDATA_DST   , 26);
MAP_TYPE2BIT(SHOTCB_YUVDATA_DST   , 27);
MAP_TYPE2BIT(SHOTCB_POSTVIEW_DST  , 28);

struct map_data2node
{
    MUINT32 data;
    MUINT32 nodedata_memory;
    MUINT32 datapath;
};

struct connect_data_rule
{
    // connect if condition is fulfilled
    // data_src , data_dst ,
    // node_src , node_dst ,
    MUINT32 condition;
    MUINT32 src;
    MUINT32 dst;
    MUINT32 nodesrc;
    MUINT32 nodedst;
    MUINT8  next_T; //offset to next rule when condition is fulfilled
    MUINT8  next_F; //offset to next rule when condition is not fulfilled
};

struct connect_notify_rule
{
    MUINT32 notify;
    MUINT32 nodesrc;
    MUINT32 nodedst;
};

#define bitS2(a, b)         (bitOf(a)|bitOf(b))
#define bitS3(a, b, c)      (bitS2(a,b)|bitOf(c))
#define bitS4(a, b, c, d)   (bitS3(a,b,c)|bitOf(d))
// for memory allocation
static const struct map_data2node gMapData2Node_Cap[] =
{
    { DATA_FULLRAW       , PASS1_FULLRAW   , bitS2(PASS1_FULLRAW, CONTROL_FULLRAW)                } ,
    { DATA_MAINYUV       , PASS2_CAP_DST_1 ,
        bitS3(CONTROL_CAP_SRC, PASS2_CAP_SRC, PASS2_CAP_DST_1)                                    } ,
    { DATA_POSTVIEW      , PASS2_CAP_DST_0 ,
        bitS3(CONTROL_CAP_SRC, PASS2_CAP_SRC, PASS2_CAP_DST_0)                                    } ,
    { DATA_THUMBNAILJPEG , JPEG_ENC_SW_DST , bitS2(PASS2_CAP_DST_0, JPEG_ENC_SW_SRC)              } ,
    { DATA_DEBUGINFO     , 0               , bitS2(CONTROL_DBGINFO, SHOTCB_DBGINFO_SRC)           } ,
    { DATA_EIS_CB        , 0               , bitS2(CONTROL_EISDATA, SHOTCB_EISDATA_SRC)           } ,
    { DATA_DUMP_IMG3O    , PASS2_CAP_DST_2 , bitS2(CONTROL_CAP_SRC, PASS2_CAP_SRC)                } ,
    { DATA_FULLRAW_CB       , 0 , bitS3(CONTROL_CAP_SRC, SHOTCB_RAWDATA_SRC , SHOTCB_RAWDATA_DST) } ,
    { DATA_MAINYUV_CB       , 0 , bitS3(PASS2_CAP_DST_1, SHOTCB_YUVDATA_SRC , SHOTCB_YUVDATA_DST) } ,
    { DATA_POSTVIEW_CB      , 0 , bitS2(PASS2_CAP_DST_0, SHOTCB_POSTVIEW_SRC)                     } ,
    { DATA_MAINJPEG_CB      , 0 , bitS2(JPEG_ENC_HW_DST, SHOTCB_JPEG_SRC)                         } ,
    { DATA_THUMBNAILJPEG_CB , 0 , bitS2(JPEG_ENC_SW_DST, SHOTCB_THUMBNAIL_SRC)                    } ,
    { 0, 0, 0 } ,
};

#define RULE( src, dst, nodesrc, nodedst, nextT, nextF ) \
{ (bitOf(src)|bitOf(dst)), src, dst, nodesrc, nodedst, nextT, nextF }

static const struct connect_data_rule gDataConnectRule_Cap[] =
{
    RULE( PASS1_FULLRAW      , CONTROL_FULLRAW      , PASS1NODE    , CTRLNODE     , 1, 1 ) ,
    //RULE( PASS1_RESIZEDRAW   , CONTROL_RESIZEDRAW   , PASS1NODE    , CTRLNODE     , 1, 1 ) ,
// if use raw callback
    RULE( CONTROL_CAP_SRC    , SHOTCB_RAWDATA_SRC   , CTRLNODE     , CALLBACKNODE , 1, 2 ) ,
    RULE( SHOTCB_RAWDATA_DST , PASS2_CAP_SRC        , CALLBACKNODE , CAPPASS2NODE , 2, 2 ) ,
// else
    RULE( CONTROL_CAP_SRC    , PASS2_CAP_SRC        , CTRLNODE     , CAPPASS2NODE , 1, 1 ) ,
// endif
// if use yuv callback
    RULE( PASS2_CAP_DST_1    , SHOTCB_YUVDATA_SRC   , CAPPASS2NODE , CALLBACKNODE , 1, 2 ) ,
    RULE( SHOTCB_YUVDATA_DST , JPEG_ENC_HW_SRC      , CALLBACKNODE , HWJPEGNODE   , 2, 2 ) ,
// else
    RULE( PASS2_CAP_DST_1    , JPEG_ENC_HW_SRC      , CAPPASS2NODE , HWJPEGNODE   , 1, 1 ) ,
// endif
// if use postview callback
    RULE( PASS2_CAP_DST_0    , SHOTCB_POSTVIEW_SRC  , CAPPASS2NODE , CALLBACKNODE , 1, 2 ) ,
    RULE( SHOTCB_POSTVIEW_DST, JPEG_ENC_SW_SRC      , CALLBACKNODE , SWJPEGNODE   , 2, 2 ) ,
// else
    RULE( PASS2_CAP_DST_0    , JPEG_ENC_SW_SRC      , CAPPASS2NODE , SWJPEGNODE   , 1, 1 ) ,
//endif
    RULE( JPEG_ENC_HW_DST    , SHOTCB_JPEG_SRC      , HWJPEGNODE   , CALLBACKNODE , 1, 1 ) ,
    RULE( JPEG_ENC_SW_DST    , SHOTCB_THUMBNAIL_SRC , SWJPEGNODE   , CALLBACKNODE , 1, 1 ) ,
    // debug info
    RULE( CONTROL_DBGINFO    , SHOTCB_DBGINFO_SRC   , CTRLNODE     , CALLBACKNODE , 1, 1 ) ,
    // eis
    RULE( CONTROL_EISDATA    , SHOTCB_EISDATA_SRC   , CTRLNODE     , CALLBACKNODE , 1, 1 ) ,
// terminate
    {0, 0, 0, 0, 0, 0, 0},
};


static const struct connect_notify_rule gNotifyConnectRule[] =
{
    {  PASS1_SOF | PASS1_EOF | PASS1_START_ISP | PASS1_STOP_ISP , PASS1NODE    , CTRLNODE     } ,
    {  CONTROL_SHUTTER                                          , CTRLNODE     , CALLBACKNODE } ,
    {  CONTROL_STOP_PASS1                                       , CTRLNODE     , PASS1NODE    } ,
    { 0, 0, 0 },
};

/*******************************************************************************
*
********************************************************************************/
BurstShot::
BurstShot(
    EShotMode const eShotMode,
    char const*const szCamShotName
)
    : CamShotImp(eShotMode, szCamShotName)
    , mShotCount(0)
    , mCurCount(0)
    , muRegisteredBufType(0x0)
    , mpAllocBufHandler(NULL)
    , mShotProfile(SHOT_PROFILE_BURST)
    , mpMap2Node(NULL)
    , mpConnectDataRule(NULL)
    , mpConnectNotifyRule(NULL)
    , mpGraph(NULL)
    , mpPass1(NULL)
    , mpCtrl(NULL)
    , mpCallback(NULL)
    , mpCapPass2(NULL)
    , mpJpegencHw(NULL)
    , mpJpegencSw(NULL)
    , mbDoShutterCb(MTRUE)
    , mFinishedData(0x0)
    , mIspProfile(EIspProfile_Capture)
    , mpCapParams(NULL)
    , muCapParamCount(0)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
init()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    if( !mpAllocBufHandler )
    {
        mpAllocBufHandler = AllocBufHandler::createInstance();
        ret = mpAllocBufHandler->init();
    }
    FUNC_END;
    //
    return ret && (sem_init(&mShotDone, 0, 0) == 0);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
uninit()
{
    MBOOL ret = MTRUE;
    FUNC_START;

    if( mpAllocBufHandler )
    {
        mpAllocBufHandler->uninit();
        mpAllocBufHandler->destroyInstance();
        mpAllocBufHandler = NULL;
    }

    FUNC_END;
    //
    if( sem_destroy(&mShotDone) != 0 )
        ret = MFALSE;

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
start(SensorParam const & rSensorParam, MUINT32 count)
{
    FUNC_START;
    //
    updateProfile(SHOT_PROFILE_BURST);
    mSensorParam = rSensorParam;
    mShotCount   = count;
    // dump
    dumpSensorParam(mSensorParam);
    //
    MBOOL ret = MTRUE;
    MUINT32 RotPicWidth, RotPicHeight;
    MUINT32 RotThuWidth, RotThuHeight;
    //
    getRotatedPicSize(&RotPicWidth, &RotPicHeight, &RotThuWidth, &RotThuHeight);
    //
    MY_LOGD("[start] enable msg (notify, data) = (0x%x,0x%x)", mi4NotifyMsgSet, mi4DataMsgSet);
    if( !isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && !isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_ALL) )
    {
        MY_LOGE("[start] no data/msg enabled");
        return MFALSE;
    }

    MUINT32 dataInBit = 0x0;
    MUINT32 CBDataSet = 0x0;
    MUINT32 nodeDataInBit = 0x0;
    updateNeededData(&dataInBit, &CBDataSet);
    updateNeededNodeData(dataInBit, &nodeDataInBit);

    // overwrite rawtype
    if(mSensorParam.u4RawType == 3){ //Eng for process raw
            mSensorParam.u4RawType = 1;
            MY_LOGD("update shot param: use process raw");
    }

    if( !mpGraph )
    {
        mpGraph = ICamGraph::createInstance(mSensorParam.u4OpenID, "BShot");
    }

    // update registered buffers
    ret = ret && doRegisterBuffers();
    //assign buffer handler
    mpGraph->setBufferHandler( PASS1_FULLRAW , mpAllocBufHandler );
    mpGraph->setBufferHandler( PASS1_RESIZEDRAW , mpAllocBufHandler ); //Temp

    if( dataInBit & DATA_MAINYUV )
    {
        ret = ret && enableData(DATA_MAINYUV);
    }

    if( dataInBit & DATA_POSTVIEW )
    {
        ret = ret && enableData(DATA_POSTVIEW);
    }
    else if( dataInBit & DATA_THUMBNAILYUV )
    {
        ret = ret && enableData(DATA_THUMBNAILYUV);
    }

    if( dataInBit & DATA_MAINJPEG )
    {
        ret = ret && enableData(DATA_MAINJPEG);
    }

    if( dataInBit & DATA_THUMBNAILJPEG )
    {
        ret = ret && enableData(DATA_THUMBNAILJPEG);
    }

    if( dataInBit & DATA_DUMP_IMG3O )
    {
        ret = ret && doDebugDump();
    }

    ret = ret && createNodes(nodeDataInBit, CBDataSet);

    ret = ret && connectNodes(nodeDataInBit);

    ret = ret && mpGraph->init();

    ret = ret && mpGraph->start();

    // prepare pass2/jpeg buffers
    if( dataInBit & DATA_MAINYUV )
    {
        for( MUINT32 i = 0 ; i < mShotCount ; i++ )
        {
            ret = ret
                && prepareMemory(
                        DATA_MAINYUV,
                        RotPicWidth, RotPicHeight,
                        eImgFmt_YUY2,
                        mShotParam.u4PictureTransform,
                        NSCam::NSIoPipe::EPortCapbility_Cap);
        }
    }

    if( dataInBit & DATA_POSTVIEW )
    {
        for( MUINT32 i = 0 ; i < mShotCount ; i++ )
        {
            ret = ret
                && prepareMemory(
                        DATA_POSTVIEW,
                        mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight,
                        mShotParam.ePostViewFmt,
                        0,
                        NSCam::NSIoPipe::EPortCapbility_Cap);
        }
    }
    else if( dataInBit & DATA_THUMBNAILYUV )
    {
        for( MUINT32 i = 0 ; i < mShotCount ; i++ )
        {
            ret = ret
                && prepareMemory(
                        DATA_THUMBNAILYUV,
                        mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight,
                        eImgFmt_YUY2,
                        0,
                        NSCam::NSIoPipe::EPortCapbility_Cap);
        }
    }

    if( dataInBit & DATA_MAINJPEG )
    {
        for( MUINT32 i = 0 ; i < mShotCount ; i++ )
        {
            ret = ret
                && prepareMemory(
                        DATA_MAINJPEG,
                        RotPicWidth, RotPicHeight,
                        eImgFmt_JPEG,
                        0);
        }
    }

    if( dataInBit & DATA_THUMBNAILJPEG )
    {
        for( MUINT32 i = 0 ; i < mShotCount ; i++ )
        {
            ret = ret
                && prepareMemory(
                        DATA_THUMBNAILJPEG,
                        RotThuWidth, RotThuHeight,
                        eImgFmt_JPEG,
                        mShotParam.u4PictureTransform);
        }
    }

    if( ret )
    {
        MINT32 sem_ret;
        while( 0 !=( sem_ret = sem_wait( &mShotDone ) ) && errno == EINTR );
        if( sem_ret < 0 )
        {
            MY_LOGE("errno = %d", errno);
            ret = MFALSE;
        }
        MY_LOGD("shot finished");
    }

    mpGraph->stop();

    mpGraph->uninit();

    mpGraph->disconnect();

    destroyNodes();

    mpGraph->destroyInstance();
    mpGraph = NULL;
    //
    reset();
    //
    FUNC_END;
    //
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
startOne(SensorParam const & rSensorParam)
{
    FUNC_START;
    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
startOne(SensorParam const & rSensorParam, IImageBuffer const *pImgBuffer)
{
    FUNC_START;
    //
    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
startAsync(SensorParam const & rSensorParam)
{
    FUNC_START;
    //
    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
stop()
{
    FUNC_START;
//#warning [TODO] for continouous shot

    //TODO
    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
registerCap3AParam(CaptureParam_T* pCapParams, MUINT32 const length)
{
    MY_LOGD("set cap 3A param 0x%x, %d", pCapParams, length);
    mpCapParams = pCapParams;
    muCapParamCount = length;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
setIspProfile(EIspProfile_T const profile)
{
    mIspProfile = profile;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
setShotParam(ShotParam const & rParam)
{
    FUNC_START;
    mShotParam = rParam;
    //
    dumpShotParam(mShotParam);

    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
setJpegParam(JpegParam const & rParam)
{
    FUNC_START;
    mJpegParam = rParam;
    //
    dumpJpegParam(mJpegParam);

    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer)
{
    MBOOL ret = MTRUE;
    MUINT32 internaldata;

    MY_LOGD("buf type 0x%x, buf 0x%x", eBufType, pImgBuffer);
    switch( eBufType )
    {
        case ECamShot_BUF_TYPE_RAW:
            internaldata = DATA_FULLRAW;
            break;
        case ECamShot_BUF_TYPE_YUV:
            internaldata = DATA_MAINYUV;
            break;
        case ECamShot_BUF_TYPE_POSTVIEW:
            internaldata = DATA_POSTVIEW;
            break;
        case ECamShot_BUF_TYPE_JPEG:
            internaldata = DATA_MAINJPEG;
            break;
        case ECamShot_BUF_TYPE_THUMBNAIL:
            internaldata = DATA_THUMBNAILJPEG;
            break;
        default:
            ret = MFALSE;
            break;
    }

    if( ret ) {
        muRegisteredBufType |= internaldata;
        regbuf_t buf = { internaldata, pImgBuffer };
        mvRegBuf.push_back(buf);
    } else {
        MY_LOGE("not support buf type(0x%x), buf 0x%x", eBufType, pImgBuffer);
    }
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    //FUNC_START;
    switch( cmd )
    {
        default:
            break;
    }
    //FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
fgNodeNotifyCallback(MVOID* user, NodeNotifyInfo const msg)
{
    BurstShot* pBurstShot = reinterpret_cast<BurstShot*>(user);
    if( pBurstShot )
    {
        switch( msg.msgType )
        {
            case NSCamNode::ENode_NOTIFY_MSG_SHUTTER:
                if( pBurstShot->mbDoShutterCb )
                {
                    pBurstShot->mbDoShutterCb = MFALSE;
                    pBurstShot->handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);
                }
                break;
            default:
                break;
        }
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
fgNodeDataCallback(MVOID* user, NodeDataInfo const msg)
{
    BurstShot* pBurstShot = reinterpret_cast<BurstShot*>(user);
    if( pBurstShot )
    {
        MUINT32 data = 0;
        switch( msg.msgType )
        {
            case NSCamNode::ENode_DATA_MSG_RAW:
                data = ECamShot_DATA_MSG_RAW;
                break;
            case NSCamNode::ENode_DATA_MSG_YUV:
                data = ECamShot_DATA_MSG_YUV;
                break;
            case NSCamNode::ENode_DATA_MSG_POSTVIEW:
                data = ECamShot_DATA_MSG_POSTVIEW;
                break;
            case NSCamNode::ENode_DATA_MSG_JPEG:
                data = ECamShot_DATA_MSG_JPEG;
                break;
            case NSCamNode::ENode_DATA_MSG_EISDATA:
                data = ECamShot_DATA_MSG_EIS;
                break;
            default:
                break;
        }

        if( data != 0 )
        {
            pBurstShot->handleDataCallback(
                    data,
                    msg.ext1,
                    msg.ext2,
                    msg.pBuffer
                    );
            pBurstShot->updateFinishDataMsg(data);
        }

        pBurstShot->tryUnRegisterBuffer(msg.pBuffer);
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
createNodes(MUINT32 const NodeDataSet, MUINT32 const NodeCbSet)
{
    CamManager* pCamMgr = CamManager::getInstance();
    Pass1NodeInitCfg p1InitCfg( mSensorParam.u4Scenario,                        // scenario
                                3,                                              // ring buf cnt
                                mSensorParam.u4Bitdepth,                        // bit depth
                                pCamMgr->getFrameRate(mSensorParam.u4OpenID),   // frame rate
                                mSensorParam.u4RawType                          // raw type
                                );

    mpPass1    = Pass1Node::createInstance(p1InitCfg);
    mpCtrl     = BShotCtrlNode::createInstance(mShotParam);
    if( muCapParamCount )
        mpCtrl->registerCap3AParam(mpCapParams, muCapParamCount);
    mpCtrl->setIspProfile(mIspProfile);

    mpCallback = ShotCallbackNode::createInstance(MTRUE);

    //optional
    if( NodeDataSet & bitOf(PASS2_CAP_SRC) )
        mpCapPass2  = Pass2Node::createInstance(PASS2_CAPTURE);

    if( NodeDataSet & bitOf(JPEG_ENC_HW_SRC) )
        mpJpegencHw = JpegEncNode::createInstance(MTRUE);

    if( NodeDataSet & bitOf(JPEG_ENC_SW_SRC) )
        mpJpegencSw = JpegEncNode::createInstance(MTRUE);

    if( mpJpegencHw )
        mpJpegencHw->setEncParam(mJpegParam.fgIsSOI, mJpegParam.u4Quality);

    if( mpJpegencSw )
        mpJpegencSw->setEncParam(mJpegParam.fgThumbIsSOI, mJpegParam.u4ThumbQuality);

    // ctrl setting
    mpCtrl->setShotCount(mShotCount);

    // enalbe callback
    mpCallback->setCallbacks(fgNodeNotifyCallback, fgNodeDataCallback, this);
    mpCallback->enableDataMsg(NodeCbSet);

    if( isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_EOF) )
        mpCallback->enableNotifyMsg(ENode_NOTIFY_MSG_SHUTTER);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
destroyNodes()
{

#define NODE_DESTROY( node )     \
    if( node )                   \
    {                            \
        node->destroyInstance(); \
        node = NULL;             \
    }

    NODE_DESTROY(mpPass1)
    NODE_DESTROY(mpCtrl)
    NODE_DESTROY(mpCallback)
    NODE_DESTROY(mpCapPass2)
    NODE_DESTROY(mpJpegencHw)
    NODE_DESTROY(mpJpegencSw)

#undef NODE_DESTROY
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
updateProfile(MUINT32 profile)
{
    mpMap2Node = NULL;
    mpConnectDataRule = NULL;
    mpConnectNotifyRule = NULL;
    mShotProfile = profile;
    MY_LOGD("shot profile %d", mShotProfile);
    switch( mShotProfile )
    {
        case SHOT_PROFILE_BURST:
            mpMap2Node          = gMapData2Node_Cap;
            mpConnectDataRule   = gDataConnectRule_Cap;
            mpConnectNotifyRule = gNotifyConnectRule;
            break;
        default:
            MY_LOGE("wrong profile %d", mShotProfile);
            return MFALSE;
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
updateNeededData(MUINT32* pNeededData, MUINT32* pNeededCb) const
{
    // map CamShot_DATA to internel data
    //
#define updateData( SHOT_DATA_MSG, inter_data, cb, need_data, need_cb) \
    if( isDataMsgEnabled(SHOT_DATA_MSG) ){                             \
        (need_data) |= (inter_data);                                   \
        (need_cb) |= (cb);                                             \
    }

    *pNeededData = DATA_FULLRAW | DATA_DEBUGINFO;

    //main jpeg
    updateData(
            ECamShot_DATA_MSG_JPEG,
            DATA_MAINYUV | DATA_MAINJPEG | DATA_MAINJPEG_CB,
            ENode_DATA_MSG_JPEG, /* callback enable data */
            *pNeededData,
            *pNeededCb
            );

    //thumbnail jpeg
    if( mJpegParam.u4ThumbWidth && mJpegParam.u4ThumbHeight )
    {
        updateData(
                ECamShot_DATA_MSG_JPEG,
                DATA_THUMBNAILYUV | DATA_THUMBNAILJPEG | DATA_THUMBNAILJPEG_CB,
                ENode_DATA_MSG_JPEG, /* callback enable data */
                *pNeededData,
                *pNeededCb
                );
    }

    //main yuv
    updateData(
            ECamShot_DATA_MSG_YUV,
            DATA_MAINYUV | DATA_MAINYUV_CB,
            ENode_DATA_MSG_YUV, /* callback enable data */
            *pNeededData,
            *pNeededCb
            );

    //postview
    updateData(
            ECamShot_DATA_MSG_POSTVIEW,
            DATA_POSTVIEW | DATA_POSTVIEW_CB,
            ENode_DATA_MSG_POSTVIEW, /* callback enable data */
            *pNeededData,
            *pNeededCb
            );

    //raw
    updateData(
            ECamShot_DATA_MSG_RAW,
            DATA_FULLRAW_CB,
            ENode_DATA_MSG_RAW, /* callback enable data */
            *pNeededData,
            *pNeededCb
            );

    //eis
    updateData(
            ECamShot_DATA_MSG_EIS,
            DATA_EIS_CB,
            ENode_DATA_MSG_EISDATA, /* callback enable data */
            *pNeededData,
            *pNeededCb
            );

    { //for debug
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "debug.shot.img3o", value, "0");
        if( atoi(value) ) {
            MY_LOGD("enable img3o dump");
            *pNeededData |= DATA_DUMP_IMG3O;
        }
    }
#undef updateData

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
updateNeededNodeData(MUINT32 neededData, MUINT32* pNodeData) const
{
    if( mpMap2Node == NULL ){
        MY_LOGE("mapMap2Node == NULL!");
        return MFALSE;
    }

    struct map_data2node const* pMap2Node = mpMap2Node;
    // update needed nodedata
    while( pMap2Node->data )
    {
        if( pMap2Node->data & neededData )
            *pNodeData |= pMap2Node->datapath;
        pMap2Node++;
    }
    //
    MY_LOGD("data(0x%x) -> nodedata(0x%x)", neededData, *pNodeData);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
connectNodes(MUINT32 NodeDataSet)
{
    MBOOL ret = MTRUE;

    if( mpConnectDataRule == NULL ){
        MY_LOGE("mpConnectDataRule == NULL!");
        return MFALSE;
    }

    if( mpConnectNotifyRule == NULL ){
        MY_LOGE("mpConnectNotifyRule == NULL!");
        return MFALSE;
    }
    // connect data according to the rule
    struct connect_data_rule const* rule_data = mpConnectDataRule;
    while( rule_data->condition )
    {
        if( (NodeDataSet & rule_data->condition) == rule_data->condition )
        {
            ret = ret &&
                mpGraph->connectData(
                    rule_data->src,
                    rule_data->dst,
                    getNode(rule_data->nodesrc),
                    getNode(rule_data->nodedst)
                    );
            rule_data += rule_data->next_T;
        }
        else
        {
            rule_data += rule_data->next_F;
        }
    }

    // connect notify
    struct connect_notify_rule const* rule_notify = mpConnectNotifyRule;
    while( rule_notify->notify )
    {
        ICamNode* pSrc = getNode(rule_notify->nodesrc);
        ICamNode* pDst = getNode(rule_notify->nodedst);
        if( pSrc && pDst )
        {
            ret = ret &&
                mpGraph->connectNotify(
                        rule_notify->notify,
                        pSrc,
                        pDst
                        );
        }
        rule_notify ++;
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
BurstShot::
mapToNodeData(MUINT32 dataType) const
{
    if( mpMap2Node == NULL ){
        MY_LOGE("mapMap2Node == NULL!");
        return 0;
    }

    map_data2node const* pMap2Node = mpMap2Node;
    while( pMap2Node->data )
    {
        if( pMap2Node->data == dataType )
            return pMap2Node->nodedata_memory;
        pMap2Node++;
    }
    return 0;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
BurstShot::
getRotatedPicSize(MUINT32 *pW, MUINT32 *pH, MUINT32 *pThumbW, MUINT32 *pThumbH) const
{
    if( mShotParam.u4PictureTransform & eTransform_ROT_90 ) //90, 270
    {
        *pW      = mShotParam.u4PictureHeight;
        *pH      = mShotParam.u4PictureWidth;
        *pThumbW = mJpegParam.u4ThumbHeight;
        *pThumbH = mJpegParam.u4ThumbWidth;
    }
    else
    {
        *pW      = mShotParam.u4PictureWidth;
        *pH      = mShotParam.u4PictureHeight;
        *pThumbW = mJpegParam.u4ThumbWidth;
        *pThumbH = mJpegParam.u4ThumbHeight;
    }
}


/*******************************************************************************
*
********************************************************************************/
ICamNode*
BurstShot::
getNode(MUINT32 node)
{
    switch(node)
    {
        case PASS1NODE:
            return mpPass1;
        case CTRLNODE:
            return mpCtrl;
        case CAPPASS2NODE:
            return mpCapPass2;
        case HWJPEGNODE:
            return mpJpegencHw;
        case SWJPEGNODE:
            return mpJpegencSw;
        case CALLBACKNODE:
            return mpCallback;
        default:
            return NULL;
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
doRegisterBuffers()
{
    MBOOL ret = MTRUE;
    vector<regbuf_t>::const_iterator pRegbuf = mvRegBuf.begin();
    while( pRegbuf != mvRegBuf.end() )
    {
        MUINT32 nodeDataType = mapToNodeData(pRegbuf->muDATA);
        if( nodeDataType ) {
            ret = ret && mpAllocBufHandler->registerBuffer(nodeDataType, pRegbuf->mBuffer);
        } else {
            MY_LOGE("map data fail 0x%x", pRegbuf->muDATA);
            ret = MFALSE;
        }
        pRegbuf++;
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
tryUnRegisterBuffer(IImageBuffer* const pBuf)
{
    vector<regbuf_t>::iterator pRegbuf = mvRegBuf.begin();
    while( pRegbuf != mvRegBuf.end() )
    {
        if( pRegbuf->mBuffer == pBuf ) {
            // do unRegister
            MUINT32 const nodeDataType = mapToNodeData(pRegbuf->muDATA);
            if( !mpAllocBufHandler->unRegisterBuffer(nodeDataType, pRegbuf->mBuffer) )
                MY_LOGW("unreg failed %p", pBuf);
            mvRegBuf.erase(pRegbuf);
            return MTRUE;
        }
        pRegbuf++;
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
enableData(MUINT32 const dataType)
{
    MUINT32 nodeDataType = mapToNodeData(dataType);

    if( nodeDataType == 0 ) {
        return MTRUE;
    }

    MBOOL ret = MTRUE;
    MBOOL bRegistered = (muRegisteredBufType & dataType);
    ret = ret
        && mpGraph->setBufferHandler( nodeDataType, mpAllocBufHandler )
        && (bRegistered ? MTRUE : mpAllocBufHandler->setDataToAlloc(nodeDataType));

    if( !ret ) {
        MY_LOGE("enableData %d failed", dataType);
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
prepareMemory(MUINT32 const dataType,
              MUINT32 const _w, MUINT32 const _h, EImageFormat const _fmt,
              MUINT32 const _trans, MINT32 const usage)
{
    const MUINT32 bufusage = eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK;
    MUINT32 nodeDataType = mapToNodeData(dataType);

    if( nodeDataType == 0 ) {
        return MTRUE;
    }

    MY_LOGD("mem: nodedata %d, wxh %dx%d, fmt 0x%x, trans %d",
            nodeDataType, _w, _h, _fmt, _trans);

    MBOOL ret = MTRUE;
    MBOOL bRegistered = (muRegisteredBufType & dataType);

    ret = ret
        && mpAllocBufHandler->updateRequest( nodeDataType, _trans, usage );

    if( !bRegistered )
    {
        AllocInfo info(_w, _h, _fmt, bufusage);
        ret = ret && mpAllocBufHandler->requestBuffer( nodeDataType, info );
    }

    if( !ret )
    {
        MY_LOGE("nodedata %d, wxh %dx%d, fmt 0x%x, trans %d",
                nodeDataType, _w, _h, _fmt, _trans);
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
BurstShot::
updateFinishDataMsg(MUINT32 datamsg)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("update finished data: finished(0x%x), data(0x%x)",
            mFinishedData, datamsg);
    mFinishedData |= datamsg;
    //check if all valid data is processed
    if( mFinishedData == (mi4DataMsgSet & ECamShot_DATA_MSG_ALL) )
    {
        mFinishedData = 0x0;
        MY_LOGD("shot count %d done", mCurCount);
        mCurCount++;
        if( mCurCount == mShotCount )
        {
            mpCallback->disableDataMsg(ENode_DATA_MSG_ALL);
            mpCallback->disableNofityMsg(ENode_NOTIFY_MSG_ALL);

            MY_LOGD("shot done, post sem");
            if( sem_post(&mShotDone) < 0 )
            {
                MY_LOGE("errno = %d", errno);
            }
        }
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
reset()
{
    mShotCount = 0;
    mCurCount = 0;
    muRegisteredBufType = 0x0;
    mbDoShutterCb = MTRUE;
    mFinishedData = 0x0;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
doDebugDump()
{
    MBOOL ret = MTRUE;

    MSize size;
    if( !getSensorSizeFmt(&size) )
        ret = MFALSE;

    ret = ret
        && enableData(DATA_DUMP_IMG3O)
        && prepareMemory(
                DATA_DUMP_IMG3O,
                size.w, size.h,
                eImgFmt_YUY2,
                0,
                NSCam::NSIoPipe::EPortCapbility_Cap);

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BurstShot::
getSensorSizeFmt(MSize* pSize)
{
    MBOOL ret = MTRUE;
    NSCam::SensorStaticInfo sensorInfo;
    //get sensor size
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MINT32 const sensorNum = pHalSensorList->queryNumberOfSensors();

    if(mSensorParam.u4OpenID >= sensorNum)
    {
        MY_LOGE("wrong sensor idx(%d), sensorNum(%d)", mSensorParam.u4OpenID, sensorNum);
        return MFALSE;
    }

    pHalSensorList->querySensorStaticInfo(
            pHalSensorList->querySensorDevIdx(mSensorParam.u4OpenID),
            &sensorInfo);

    // sensor size
#define scenario_case(scenario, KEY, pSize)      \
        case scenario:                           \
            (pSize)->w = sensorInfo.KEY##Width;  \
            (pSize)->h = sensorInfo.KEY##Height; \
            break;
    switch(mSensorParam.u4Scenario)
    {
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, pSize );
        default:
            MY_LOGE("not support sensor scenario(0x%x)", mSensorParam.u4Scenario);
            ret = MFALSE;
            break;
    }
#undef scenario_case

    return ret;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

