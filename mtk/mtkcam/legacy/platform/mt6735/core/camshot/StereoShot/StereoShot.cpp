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
#define LOG_TAG "MtkCam/SShot"
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
#include <mtkcam/camnode/stereonode.h>
#include <mtkcam/camnode/StereoCtrlNode.h>
#include <mtkcam/camnode/ImgTransformNode.h>
#include <mtkcam/camnode/DualImgTransformNode.h>
#include <mtkcam/camnode/JpegEncNode.h>
#include <mtkcam/camnode/ShotCallbackNode.h>
#include <mtkcam/camnode/AllocBufHandler.h>
#include "../inc/StereoShotCtrlNode.h"

using namespace NSCamNode;
//
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>
//
#include "../inc/CamShotImp.h"
#include "../inc/StereoShot.h"
#include "../inc/CapBufNode.h"
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
using namespace NSCam;
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;
//
#include <mtkcam/featureio/stereo_hal_base.h>
//
// buffer usage
#include <mtkcam/imageio/ispio_pipe_ports.h>
//
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
    P1NODE_MAIN   = 0x0 ,   // 0
    P1NODE_MAIN2        ,
    CTRLNODE            ,
    CTRLNODE_MAIN2      ,
    SNODE_CAPTURE       ,
    SNODE_MAIN          ,   // 5
    SNODE_MAIN2         ,
    SCTRLNODE           ,
    ITNODE              ,
    DUALITNODE          ,
    HWJPEGNODE          ,   // 10
    HWJPSNODE           ,
    SWJPEGNODE          ,
    CALLBACKNODE        ,
};

enum
{
    DATA_FULLRAW            = 0x00000001,   // p1 out
    DATA_RESIZEDRAW         = 0x00000002,

    DATA_MAINYUV            = 0x00000004,   //
    DATA_RESIZEDYUV         = 0x00000008,

    DATA_SIMG               = 0x00000010,   // SNode out
    DATA_SFEO               = 0x00000020,
    DATA_SRGB               = 0x00000040,
    DATA_SMAIN              = 0x00000080,

    DATA_SCMAIN             = 0x00000100,   // SCNode out
    DATA_SCIMG              = 0x00000200,   // SCNode out

    DATA_ITMAINYUV          = 0x00000400,   // for main image yuv

    DATA_DUALITSBS          = 0x00001000,   // for SBS image yuv
    DATA_DUALIT_M           = 0x00002000,   // for main thumnail yuv

    DATA_MAINJPEG           = 0x00010000,   // main jpeg
    DATA_THUMBJPEG          = 0x00020000,   // thumb jpeg
    DATA_MAINJPS            = 0x00040000,

    DATA_DEBUGINFO          = 0x00080000,   // main debug info
    DATA_DEBUGINFO_S        = 0x00100000,   // main2 debug info
    // extend for connection
    DATA_MAINJPEG_CB        = 0x00200000,
    DATA_THUMBNAILJPEG_CB   = 0x00400000,
    DATA_MAINJPS_CB         = 0x00800000,
};

#define MAP_TYPE2BIT(datatype, bit) \
    static const MUINT32 datatype##_bit = (1<<(bit));
#define bitOf(datatype) (datatype##_bit)


MAP_TYPE2BIT(CONTROL_PRV_SRC        , 1);
MAP_TYPE2BIT(CONTROL_CAP_SRC        , 2);
MAP_TYPE2BIT(PASS2_CAP_SRC          , 3);
MAP_TYPE2BIT(PASS2_CAP_DST_0        , 4);
MAP_TYPE2BIT(STEREO_SRC             , 5);
MAP_TYPE2BIT(STEREO_CTRL_DST_M      , 6);
MAP_TYPE2BIT(STEREO_CTRL_DST_S      , 7);
MAP_TYPE2BIT(STEREO_CTRL_MAIN_DST   , 8);
MAP_TYPE2BIT(TRANSFORM_SRC          , 9);
MAP_TYPE2BIT(TRANSFORM_DST_0        , 10);
MAP_TYPE2BIT(TRANSFORM_DST_1        , 11);
MAP_TYPE2BIT(DUALIT_SRC_0           , 12);
MAP_TYPE2BIT(DUALIT_SRC_1           , 13);
MAP_TYPE2BIT(DUALIT_DST_0           , 14);
MAP_TYPE2BIT(DUALIT_DST_1           , 15);
MAP_TYPE2BIT(DUALIT_DST_2           , 16);
MAP_TYPE2BIT(JPEG_ENC_SW_SRC        , 17);
MAP_TYPE2BIT(JPEG_ENC_SW_DST        , 18);
MAP_TYPE2BIT(JPEG_ENC_HW_SRC        , 19);
MAP_TYPE2BIT(JPEG_ENC_HW_DST        , 20);
MAP_TYPE2BIT(SHOTCB_JPEG_SRC        , 21);
MAP_TYPE2BIT(SHOTCB_JPS_SRC         , 22);
MAP_TYPE2BIT(SHOTCB_THUMBNAIL_SRC   , 23);
//
MAP_TYPE2BIT(CONTROL_DBGINFO        , 24);
MAP_TYPE2BIT(SHOTCB_DBGINFO_SRC     , 25);
MAP_TYPE2BIT(SHOTCB_DBGINFO_1_SRC   , 26);


struct map_data2node
{
    MUINT32 data;
    MUINT32 nodedata_memory;
    MUINT32 datapath;
};

struct connect_notify_rule
{
    MUINT32 notify;
    MUINT32 nodesrc;
    MUINT32 nodedst;
};

#define bitS2(a, b)         (bitOf(a)|bitOf(b))
// for memory allocation
// 1. normal capture
static const struct map_data2node gMapData2Node_Raw[] =
{
    { DATA_MAINYUV          , PASS2_CAP_DST_0       , bitS2(CONTROL_CAP_SRC, PASS2_CAP_SRC)             } ,
    { DATA_RESIZEDYUV       , STEREO_SRC            , bitS2(CONTROL_PRV_SRC, STEREO_SRC)                } ,

    { DATA_ITMAINYUV        , TRANSFORM_DST_1       , bitS2(STEREO_CTRL_MAIN_DST, TRANSFORM_SRC)        } ,
    { DATA_DUALIT_M         , DUALIT_DST_0          , bitS2(STEREO_CTRL_DST_M, DUALIT_SRC_0)            } ,
    { DATA_DUALITSBS        , DUALIT_DST_2          , bitS2(STEREO_CTRL_DST_M, DUALIT_SRC_0)            } ,

    { DATA_MAINJPEG         , JPEG_ENC_HW_DST       , bitS2(TRANSFORM_DST_1, JPEG_ENC_HW_SRC)           } ,
    { DATA_THUMBJPEG        , JPEG_ENC_SW_DST       , bitS2(DUALIT_DST_0, JPEG_ENC_SW_SRC)              } ,
    { DATA_MAINJPS          , JPEG_ENC_HW_DST       , bitS2(DUALIT_DST_2, JPEG_ENC_HW_SRC)              } ,

    { DATA_DEBUGINFO        , 0                     , bitS2(CONTROL_DBGINFO, SHOTCB_DBGINFO_SRC)        } ,
    { DATA_DEBUGINFO_S      , 0                     , bitS2(CONTROL_DBGINFO, SHOTCB_DBGINFO_1_SRC)      } ,
    { DATA_MAINJPEG_CB      , 0                     , bitS2(JPEG_ENC_HW_DST, SHOTCB_JPEG_SRC)           } ,
    { DATA_THUMBNAILJPEG_CB , 0                     , bitS2(JPEG_ENC_SW_DST, SHOTCB_THUMBNAIL_SRC)      } ,
    { DATA_MAINJPS_CB       , 0                     , bitS2(JPEG_ENC_HW_DST, SHOTCB_JPS_SRC)            } ,
    { 0, 0, 0 } ,
};

static const struct connect_notify_rule gNotifyConnectRule[] =
{
    {  PASS1_SOF | PASS1_EOF | PASS1_START_ISP | PASS1_STOP_ISP , P1NODE_MAIN   , CTRLNODE      } ,
    {  PASS1_SOF | PASS1_EOF | PASS1_START_ISP | PASS1_STOP_ISP , P1NODE_MAIN2  , CTRLNODE_MAIN2} ,

    {  CONTROL_SHUTTER                                          , CTRLNODE      , CALLBACKNODE  } ,
    {  CONTROL_STOP_PASS1                                       , CTRLNODE      , P1NODE_MAIN   } ,
    {  CONTROL_STOP_PASS1                                       , CTRLNODE_MAIN2, P1NODE_MAIN2  } ,
    { 0, 0, 0 },
};

static const struct connect_notify_rule gNotifyConnectRule_ZSD[] =
{
    {  PASS1_SOF | PASS1_EOF                   , P1NODE_MAIN    , CTRLNODE      } ,
    {  PASS1_SOF | PASS1_EOF                   , P1NODE_MAIN2   , CTRLNODE_MAIN2} ,
    {  CONTROL_SHUTTER                         , CTRLNODE       , CALLBACKNODE  } ,
    {  CONTROL_STOP_PASS1                      , CTRLNODE       , P1NODE_MAIN   } ,
    {  CONTROL_STOP_PASS1                      , CTRLNODE_MAIN2 , P1NODE_MAIN2  } ,
    { 0, 0, 0 },
};

/*******************************************************************************
*
********************************************************************************/
StereoShot::
StereoShot(
    EShotMode const eShotMode,
    char const*const szCamShotName
)
    : CamShotImp(eShotMode, szCamShotName)
    , mSensorId_Main(-1)
    , mSensorId_Main2(-1)
    , muRegisteredBufType(0x0)
    , mpAllocBufHandler(NULL)
    , mpAllocBufHandler_Main2(NULL)
    , mpPrvBufHandler(NULL)
    , muCapStyle(ECamShot_CMD_STYLE_NORMAL)
    , mShotProfile(-1)
    , mpMap2Node(NULL)
    , mpConnectNotifyRule(NULL)
    //
    , mpGraph(NULL)
    , mpPass1(NULL)
    , mpPass1_Main2(NULL)
    , mpCtrl(NULL)
    , mpCtrl_Main2(NULL)
    , mpStereoCapNode(NULL)
    , mpStereoNode(NULL)
    , mpStereoNode_Main2(NULL)
    , mpStereoCtrlNode(NULL)
    , mpITNode(NULL)
    , mpDualITNode(NULL)
    , mpCallback(NULL)
    , mpJpegencHw(NULL)
    , mpJpsEncHw(NULL)
    , mpJpegencSw(NULL)
    //
    , mpStereoHal(NULL)
    , mbDoShutterCb(MTRUE)
    , mFinishedData(0x0)
    , mbPureRaw(MFALSE)
    , muNRType(ECamShot_NRTYPE_NONE)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
init()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    String8 const s8MainIdKey("MTK_SENSOR_DEV_MAIN");
    String8 const s8Main2IdKey("MTK_SENSOR_DEV_MAIN_2");
    Utils::Property::tryGet(s8MainIdKey, mSensorId_Main);
    Utils::Property::tryGet(s8Main2IdKey, mSensorId_Main2);
    //
    mpStereoHal = StereoHalBase::createInstance();
    //
    if( !mpAllocBufHandler )
    {
        mpAllocBufHandler = AllocBufHandler::createInstance();
        ret = mpAllocBufHandler->init();
    }
    if ( !mpAllocBufHandler_Main2 )
    {
        mpAllocBufHandler_Main2 = AllocBufHandler::createInstance();
        ret = mpAllocBufHandler_Main2->init();
    }
    FUNC_END;
    //
    return ret && (sem_init(&mShotDone, 0, 0) == 0);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
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
    if ( mpAllocBufHandler_Main2 )
    {
        mpAllocBufHandler_Main2->uninit();
        mpAllocBufHandler_Main2->destroyInstance();
        mpAllocBufHandler_Main2 = NULL;
    }
    if( mpPrvBufHandler )
    {
        // no need to destroy, since this is passed from adapter
        mpPrvBufHandler = NULL;
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
StereoShot::
start(SensorParam const & rSensorParam)
{
    FUNC_START;
    mSensorParam = rSensorParam;
    //
    dumpSensorParam(mSensorParam);

    FUNC_END;
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
startOne(SensorParam const & rSensorParam)
{
    FUNC_START;

    MBOOL ret = MTRUE;
    MUINT32 RotPicWidth, RotPicHeight;
    MUINT32 RotThuWidth, RotThuHeight;
    MSize mainSize = mpStereoHal->getMainSize( MSize(mShotParam.u4PictureWidth, mShotParam.u4PictureHeight) );
    MY_LOGD("1 mainSize(%dx%d)", mainSize.w, mainSize.h);
    //
    mSensorParam = rSensorParam;
    //
    // dump infos
    dumpSensorParam(mSensorParam);
    //query flash on/off
    {
        IHal3A* pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, mSensorParam.u4OpenID, LOG_TAG);
        if( pHal3A == NULL ) {
            MY_LOGE("create 3A failed");
            return MFALSE;
        }

        MBOOL bFlashOn = pHal3A->isNeedFiringFlash();
        MBOOL isYuv =
            NSCam::IHalSensorList::get()->queryType( mSensorParam.u4OpenID ) == NSCam::NSSensorType::eYUV;
        pHal3A->destroyInstance(LOG_TAG);
        pHal3A = NULL;

        if( !isYuv                                          &&  // not yuv sensor
            (!CamManager::getInstance()->isMultiDevice())   &&  // only single device support pure raw flow
            (bFlashOn && mSensorParam.u4RawType != 3)       ||  // if flash on && not forced pre-processed
            mSensorParam.u4RawType == 0                         // if pure-raw
            )
        {
            mbPureRaw = MTRUE;
            MY_LOGD("flash %d, rawtype %d: use pureraw",
                    bFlashOn, mSensorParam.u4RawType);
        }

        {   //debug
            char value[PROPERTY_VALUE_MAX] = {'\0'};
            int val;
            property_get( "debug.sshot.pureraw", value, "-1");
            val = atoi(value);
            if( val != -1 ) {
                mbPureRaw = val;
                MY_LOGD("force pure-raw %d", mbPureRaw);
            }
        }

        // overwrite rawtype
        if( mbPureRaw && mSensorParam.u4RawType == 1) {
            mSensorParam.u4RawType = 0;
            MY_LOGD("update shot param: use pure raw");
        }
    }
    //
    updateProfile(SHOT_PROFILE_NORMAL);
    //
    getRotatedPicSize(&RotPicWidth, &RotPicHeight, &RotThuWidth, &RotThuHeight);
    //
    MY_LOGD("enable msg (notify, data) = (0x%x,0x%x)", mi4NotifyMsgSet, mi4DataMsgSet);
    if( !isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && !isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_ALL) )
    {
        MY_LOGE("no data/msg enabled");
        return MFALSE;
    }

    MUINT32 dataInBit = 0x0;
    MUINT32 CBDataSet = 0x0;
    MUINT32 nodeDataInBit = 0x0;
    updateNeededData(&dataInBit, &CBDataSet);
    updateNeededNodeData(dataInBit, &nodeDataInBit);

    if( !mpGraph )
    {
        mpGraph = ICamGraph::createInstance(mSensorParam.u4OpenID, "SShot");
    }

    // update registered buffers
    ret = ret && doRegisterBuffers();

    if( dataInBit & DATA_DUALITSBS )
    {
        ret = ret
            && prepareMemory(
                    DATA_DUALITSBS,
                    mJpegParam.u4JpsWidth, mJpegParam.u4JpsHeight,
                    eImgFmt_YUY2,
                    0,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_DUALITSBS )
    {
        ret = ret && enableData(DATA_DUALITSBS);
    }

    //assign buffer handler
    mpGraph->setBufferHandler(   PASS1_RESIZEDRAW   ,   mpAllocBufHandler,          getOpenId_Main());
    mpGraph->setBufferHandler(   PASS1_FULLRAW      ,   mpAllocBufHandler,          getOpenId_Main());
    mpGraph->setBufferHandler(   STEREO_IMG         ,   mpAllocBufHandler,          getOpenId_Main());
    mpGraph->setBufferHandler(   STEREO_RGB         ,   mpAllocBufHandler,          getOpenId_Main());
    mpGraph->setBufferHandler(   PASS1_RESIZEDRAW   ,   mpAllocBufHandler_Main2,    getOpenId_Main2());
    mpGraph->setBufferHandler(   PASS1_FULLRAW      ,   mpAllocBufHandler_Main2,    getOpenId_Main2());
    mpGraph->setBufferHandler(   STEREO_IMG         ,   mpAllocBufHandler_Main2,    getOpenId_Main2());
    mpGraph->setBufferHandler(   STEREO_RGB         ,   mpAllocBufHandler_Main2,    getOpenId_Main2());


    if( dataInBit & DATA_MAINYUV )
    {
        ret = ret && enableData(DATA_MAINYUV);
    }

    if( dataInBit & DATA_RESIZEDYUV )
    {
        ret = ret && enableData(DATA_RESIZEDYUV);
    }

    if( dataInBit & DATA_ITMAINYUV )
    {
        ret = ret && enableData(DATA_ITMAINYUV);
    }

    if( dataInBit & DATA_DUALIT_M )
    {
        ret = ret && enableData(DATA_DUALIT_M);
    }

    if( dataInBit & DATA_MAINJPEG )
    {
        ret = ret && enableData(DATA_MAINJPEG);
    }

    if( dataInBit & DATA_MAINJPS )
    {
        ret = ret && enableData(DATA_MAINJPS);
    }

    if( dataInBit & DATA_THUMBJPEG )
    {
        ret = ret && enableData(DATA_THUMBJPEG);
    }

    ret = ret && createNodes(nodeDataInBit, CBDataSet, NULL, NULL);

    ret = ret && connectNodes(nodeDataInBit);

    ret = ret && mpGraph->init();

    ret = ret && mpGraph->start();

    if( dataInBit & DATA_MAINYUV )
    {
        ret = ret
            && prepareMemory(
                    DATA_MAINYUV,
                    mainSize.w, mainSize.h,
                    eImgFmt_YUY2,
                    0,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_ITMAINYUV )
    {
        ret = ret
            && prepareMemory(
                    DATA_ITMAINYUV,
                    RotPicWidth, RotPicHeight,
                    eImgFmt_YUY2,
                    mShotParam.u4PictureTransform,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_DUALIT_M )
    {
        ret = ret
            && prepareMemory(
                    DATA_DUALIT_M,
                    RotThuWidth, RotThuHeight,
                    eImgFmt_YUY2,
                    mShotParam.u4PictureTransform,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_MAINJPEG )
    {
        ret = ret
            && prepareMemory(
                    DATA_MAINJPEG,
                    RotPicWidth, RotPicHeight,
                    eImgFmt_JPEG,
                    0);
    }

    if( dataInBit & DATA_MAINJPS )
    {
        ret = ret
            && prepareMemory(
                    DATA_MAINJPS,
                    mJpegParam.u4JpsWidth, mJpegParam.u4JpsHeight,
                    eImgFmt_JPEG,
                    0);
    }

    if( dataInBit & DATA_THUMBJPEG )
    {
        ret = ret
            && prepareMemory(
                DATA_THUMBJPEG,
                RotThuWidth, RotThuHeight,
                eImgFmt_JPEG,
                0);
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
StereoShot::
startOne(
    SensorParam const & rSensorParam,
    IImageBuffer const *pImgBuf_0,
    IImageBuffer const *pImgBuf_1,
    IImageBuffer const *pBufPrv_0,
    IImageBuffer const *pBufPrv_1
)
{
    FUNC_START;

    MBOOL ret = MTRUE;
    MUINT32 RotPicWidth, RotPicHeight;
    MUINT32 RotThuWidth, RotThuHeight;
    MSize mainSize = mpStereoHal->getMainSize( MSize(mShotParam.u4PictureWidth, mShotParam.u4PictureHeight) );
    MY_LOGD("2 mainSize(%dx%d)", mainSize.w, mainSize.h);
    //
    updateProfile(SHOT_PROFILE_ZSD);
    mSensorParam = rSensorParam;
    //
    // dump infos
    MY_LOGD("src buf 0x%x, 0x%x, 0x%x, 0x%x", pImgBuf_0, pImgBuf_1, pBufPrv_0, pBufPrv_1);
    dumpSensorParam(mSensorParam);
    //
    getRotatedPicSize(&RotPicWidth, &RotPicHeight, &RotThuWidth, &RotThuHeight);
    //
    MY_LOGD("enable msg (notify, data) = (0x%x,0x%x)", mi4NotifyMsgSet, mi4DataMsgSet);
    if( !isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && !isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_ALL) )
    {
        MY_LOGE("no data/msg enabled");
        return MFALSE;
    }

    {
        Mutex::Autolock _l(mLock);
        mFinishedData = 0x0;
    }

    MUINT32 dataInBit = 0x0;
    MUINT32 CBDataSet = 0x0;
    MUINT32 nodeDataInBit = 0x0;
    updateNeededData(&dataInBit, &CBDataSet);
    updateNeededNodeData(dataInBit, &nodeDataInBit);

    if( !mpGraph )
    {
        mpGraph = ICamGraph::createInstance(mSensorParam.u4OpenID, "SShot");
    }

    // update registered buffers
    ret = ret && doRegisterBuffers();

    if( dataInBit & DATA_DUALITSBS )
    {
        ret = ret
            && prepareMemory(
                    DATA_DUALITSBS,
                    mJpegParam.u4JpsWidth, mJpegParam.u4JpsHeight,
                    eImgFmt_YUY2,
                    0,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_DUALITSBS )
    {
        ret = ret && enableData(DATA_DUALITSBS);
    }

    //assign buffer handler
    mpGraph->setBufferHandler(   PASS1_RESIZEDRAW   ,   mpAllocBufHandler,          getOpenId_Main());
    mpGraph->setBufferHandler(   STEREO_IMG         ,   mpAllocBufHandler,          getOpenId_Main());
    mpGraph->setBufferHandler(   STEREO_RGB         ,   mpAllocBufHandler,          getOpenId_Main());
    mpGraph->setBufferHandler(   PASS1_RESIZEDRAW   ,   mpAllocBufHandler_Main2,    getOpenId_Main2());
    mpGraph->setBufferHandler(   STEREO_IMG         ,   mpAllocBufHandler_Main2,    getOpenId_Main2());
    mpGraph->setBufferHandler(   STEREO_RGB         ,   mpAllocBufHandler_Main2,    getOpenId_Main2());

    if( dataInBit & DATA_MAINYUV )
    {
        ret = ret && enableData(DATA_MAINYUV);
    }

    if( dataInBit & DATA_RESIZEDYUV )
    {
        ret = ret && enableData(DATA_RESIZEDYUV);
    }

    if( dataInBit & DATA_ITMAINYUV )
    {
        ret = ret && enableData(DATA_ITMAINYUV);
    }

    if( dataInBit & DATA_DUALIT_M )
    {
        ret = ret && enableData(DATA_DUALIT_M);
    }

    if( dataInBit & DATA_DUALITSBS )
    {
        ret = ret && enableData(DATA_DUALITSBS);
    }

    if( dataInBit & DATA_MAINJPEG )
    {
        ret = ret && enableData(DATA_MAINJPEG);
    }

    if( dataInBit & DATA_MAINJPS )
    {
        ret = ret && enableData(DATA_MAINJPS);
    }

    if( dataInBit & DATA_THUMBJPEG )
    {
        ret = ret && enableData(DATA_THUMBJPEG);
    }

    ret = ret && createNodes(nodeDataInBit, CBDataSet, pImgBuf_0, pImgBuf_1);

    ret = ret && connectNodes(nodeDataInBit);

    ret = ret && mpGraph->init();

    ret = ret && mpGraph->start();

    // prepare pass2/jpeg buffers
    if( dataInBit & DATA_MAINYUV )
    {
        ret = ret
            && prepareMemory(
                    DATA_MAINYUV,
                    mainSize.w, mainSize.h,
                    eImgFmt_YUY2,
                    0,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_ITMAINYUV )
    {
        ret = ret
            && prepareMemory(
                    DATA_ITMAINYUV,
                    RotPicWidth, RotPicHeight,
                    eImgFmt_YUY2,
                    mShotParam.u4PictureTransform,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_DUALIT_M )
    {
        ret = ret
            && prepareMemory(
                    DATA_DUALIT_M,
                    RotThuWidth, RotThuHeight,
                    eImgFmt_YUY2,
                    mShotParam.u4PictureTransform,
                    NSImageio::NSIspio::EPortCapbility_Cap);
    }

    if( dataInBit & DATA_MAINJPEG )
    {
        ret = ret
            && prepareMemory(
                    DATA_MAINJPEG,
                    RotPicWidth, RotPicHeight,
                    eImgFmt_JPEG,
                    0);
    }

    if( dataInBit & DATA_MAINJPS )
    {
        ret = ret
            && prepareMemory(
                    DATA_MAINJPS,
                    mJpegParam.u4JpsWidth, mJpegParam.u4JpsHeight,
                    eImgFmt_JPEG,
                    0);
    }

    if( dataInBit & DATA_THUMBJPEG )
    {
        ret = ret
            && prepareMemory(
                DATA_THUMBJPEG,
                RotThuWidth, RotThuHeight,
                eImgFmt_JPEG,
                mShotParam.u4PictureTransform);
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
    FUNC_END;
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
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
StereoShot::
stop()
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
StereoShot::
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
StereoShot::
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
StereoShot::
registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer)
{
    MBOOL ret = MTRUE;
    MUINT32 internaldata;

    MY_LOGD("buf type 0x%x, buf 0x%x", eBufType, pImgBuffer);
    switch( eBufType )
    {
        case ECamShot_BUF_TYPE_RAW:
            ret = MFALSE;
            break;
        case ECamShot_BUF_TYPE_YUV:
            internaldata = DATA_MAINYUV;
            break;
        case ECamShot_BUF_TYPE_JPEG:
            internaldata = DATA_MAINJPEG;
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
StereoShot::
setPrvBufHdl(MVOID* pCamBufHdl)
{
    mpPrvBufHandler = (ICamBufHandler*)pCamBufHdl;
    MY_LOGD("prv buf handle 0x%x", mpPrvBufHandler);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    //FUNC_START;
    switch( cmd )
    {
        case ECamShot_CMD_SET_CAPTURE_STYLE:
            muCapStyle = arg1;
            MY_LOGD("capture style 0x%x", muCapStyle);
            break;
        case ECamShot_CMD_SET_NRTYPE:
            muNRType = arg1;
            MY_LOGD("NR type 0x%x", muNRType);
            break;
        default:
            MY_LOGW("unsupport cmd 0x%x", cmd);
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
StereoShot::
fgNodeNotifyCallback(MVOID* user, NodeNotifyInfo const msg)
{
    StereoShot* pStereoShot = reinterpret_cast<StereoShot*>(user);
    if( pStereoShot )
    {
        switch( msg.msgType )
        {
            case NSCamNode::ENode_NOTIFY_MSG_SHUTTER:
                if( pStereoShot->mbDoShutterCb )
                {
                    pStereoShot->mbDoShutterCb = MFALSE;
                    pStereoShot->handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);
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
StereoShot::
fgNodeDataCallback(MVOID* user, NodeDataInfo const msg)
{
    StereoShot* pStereoShot = reinterpret_cast<StereoShot*>(user);
    if( pStereoShot )
    {
        MUINT32 data = 0;
        switch( msg.msgType )
        {
            case NSCamNode::ENode_DATA_MSG_JPEG:
                data = ECamShot_DATA_MSG_JPEG;
                break;
            case NSCamNode::ENode_DATA_MSG_JPS:
                data = ECamShot_DATA_MSG_JPS;
                break;
            default:
                MY_LOGW("unsupport msgType 0x%x", msg.msgType);
                break;
        }

        if( data != 0 )
        {
            pStereoShot->handleDataCallback(
                    data,
                    msg.ext1,
                    msg.ext2,
                    msg.pBuffer
                    );
            pStereoShot->updateFinishDataMsg(data);
        }
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
createNodes(MUINT32 const NodeDataSet, MUINT32 const NodeCbSet, IImageBuffer const *pImgBuf_0, IImageBuffer const *pImgBuf_1)
{
    CamManager* pCamMgr = CamManager::getInstance();
    Pass1NodeInitCfg p1InitCfg( mSensorParam.u4Scenario,                        // scenario
                                3,                                              // ring buf cnt
                                mSensorParam.u4Bitdepth,                        // bit depth
                                15, // TODO: fix hardcode
                                mSensorParam.u4RawType                          // raw type
                                );
    SCNodeInitCfg SCNodeInitCfg( STEREO_CTRL_CAPTURE,
                                 1280,  // TODO:
                                 720,
                                 mShotParam.u4PictureWidth,
                                 mShotParam.u4PictureHeight);
    //always create
    if( pImgBuf_0 == NULL ) {
        mpPass1         = Pass1Node::createInstance(p1InitCfg);
        mpPass1_Main2   = Pass1Node::createInstance(p1InitCfg); // TODO:
        mpPass1->setSensorIdx( getOpenId_Main() );
        mpPass1_Main2->setSensorIdx( getOpenId_Main2() );
    } else {
        //zsd
        CapBufNode* capbuf_0 = CapBufNode::createInstance(p1InitCfg);
        CapBufNode* capbuf_1 = CapBufNode::createInstance(p1InitCfg);
        capbuf_0->setOneImgBuf(const_cast<IImageBuffer*>(pImgBuf_0));
        capbuf_1->setOneImgBuf(const_cast<IImageBuffer*>(pImgBuf_1));
        mpPass1         = capbuf_0;
        mpPass1_Main2   = capbuf_1;
        mpPass1->setSensorIdx( getOpenId_Main() );
        mpPass1_Main2->setSensorIdx( getOpenId_Main2() );
    }

    mpCtrl          = StereoShotCtrlNode::createInstance(mShotParam, muCapStyle, pImgBuf_0 == NULL);
    mpCtrl_Main2    = StereoShotCtrlNode::createInstance(mShotParam, muCapStyle, pImgBuf_1 == NULL);
    mpCtrl->setSensorIdx( getOpenId_Main() );
    mpCtrl_Main2->setSensorIdx( getOpenId_Main2() );

    EIspProfile_T capprofile = EIspProfile_Capture;
    { // isp profile selection
        if( mbPureRaw ) {
            capprofile =  muNRType != ECamShot_NRTYPE_SWNR ?
                EIspProfile_PureRAW_Capture : EIspProfile_PureRAW_Capture_SWNR;
        } else if( muCapStyle == ECamShot_CMD_STYLE_VSS ) {
            capprofile = muNRType != ECamShot_NRTYPE_SWNR ?
                EIspProfile_VSS_Capture : EIspProfile_VSS_Capture_SWNR;
        } else if ( muCapStyle == ECamShot_CMD_STYLE_NORMAL ) {
            capprofile = muNRType != ECamShot_NRTYPE_SWNR ?
                EIspProfile_Capture : EIspProfile_Capture_SWNR;
        }
    }
    mpCtrl->setIspProfile(capprofile);
    mpCtrl_Main2->setIspProfile(capprofile);

    mpStereoCapNode         = Pass2Node::createInstance( PASS2_CAPTURE );
    mpStereoNode            = StereoNode::createInstance(STEREO_PASS2_PREVIEW);
    mpStereoNode_Main2      = StereoNode::createInstance(STEREO_PASS2_PREVIEW);
    mpStereoCtrlNode        = StereoCtrlNode::createInstance(SCNodeInitCfg);
    mpITNode                = ImgTransformNode::createInstance();
    mpDualITNode            = DualImgTransformNode::createInstance( mpStereoHal->getSensorPosition() );

    mpStereoCapNode->setSensorIdx( getOpenId_Main() );
    mpStereoNode->setSensorIdx( getOpenId_Main() );
    mpStereoNode_Main2->setSensorIdx( getOpenId_Main2() );
    mpITNode->setSensorIdx( getOpenId_Main() );

    mpCallback              = ShotCallbackNode::createInstance(MFALSE); //no thread

    mpJpegencHw = JpegEncNode::createInstance(MTRUE);
    mpJpsEncHw  = JpegEncNode::createInstance(MTRUE);
    mpJpegencSw = JpegEncNode::createInstance(MTRUE);

    mpJpegencHw->setEncParam(mJpegParam.fgIsSOI, mJpegParam.u4Quality);
    mpJpsEncHw->setEncParam(mJpegParam.fgIsSOI, mJpegParam.u4Quality);
    mpJpegencSw->setEncParam(mJpegParam.fgThumbIsSOI, mJpegParam.u4ThumbQuality);

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
StereoShot::
destroyNodes()
{

#define NODE_DESTROY( node )     \
    if( node )                   \
    {                            \
        node->destroyInstance(); \
        node = NULL;             \
    }

    NODE_DESTROY(mpPass1)
    NODE_DESTROY(mpPass1_Main2)
    NODE_DESTROY(mpCtrl)
    NODE_DESTROY(mpCtrl_Main2)
    NODE_DESTROY(mpStereoCapNode)
    NODE_DESTROY(mpStereoNode)
    NODE_DESTROY(mpStereoNode_Main2)
    NODE_DESTROY(mpStereoCtrlNode)
    NODE_DESTROY(mpITNode)
    NODE_DESTROY(mpDualITNode)
    NODE_DESTROY(mpCallback)
    NODE_DESTROY(mpJpegencHw)
    NODE_DESTROY(mpJpsEncHw)
    NODE_DESTROY(mpJpegencSw)

#undef NODE_DESTROY
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
updateProfile(MUINT32 profile)
{
    mpMap2Node = NULL;
    mpConnectNotifyRule = NULL;
    mShotProfile = profile;
    MY_LOGD("shot profile %d", mShotProfile);
    switch( mShotProfile )
    {
        case SHOT_PROFILE_NORMAL:
            mpMap2Node          = gMapData2Node_Raw;
            mpConnectNotifyRule = gNotifyConnectRule;
            break;
        case SHOT_PROFILE_ZSD:
            mpMap2Node          = gMapData2Node_Raw;
            mpConnectNotifyRule = gNotifyConnectRule_ZSD;
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
StereoShot::
updateNeededData(MUINT32* pNeededData, MUINT32* pNeededCb) const
{
    // map CamShot_DATA to internel data
    //
#define updateData( SHOT_DATA_MSG, inter_data, cb, need_data, need_cb) \
    if( isDataMsgEnabled(SHOT_DATA_MSG) ){                             \
        (need_data) |= (inter_data);                                   \
        (need_cb) |= (cb);                                             \
    }

    *pNeededData = DATA_FULLRAW | DATA_DEBUGINFO | DATA_DEBUGINFO_S;
    if( mShotProfile == SHOT_PROFILE_NORMAL )
        *pNeededData |= DATA_RESIZEDRAW;

    //
    *pNeededData |= (DATA_MAINYUV | DATA_RESIZEDYUV);
    *pNeededData |= (DATA_SIMG | DATA_SFEO | DATA_SRGB | DATA_SMAIN | DATA_SCMAIN);

    //main jpeg
    updateData(
            ECamShot_DATA_MSG_JPEG,
            DATA_ITMAINYUV | DATA_MAINJPEG | DATA_MAINJPEG_CB,
            ENode_DATA_MSG_JPEG, // callback enable data
            *pNeededData,
            *pNeededCb
            );

    //thumbnail jpeg
    if( mJpegParam.u4ThumbWidth && mJpegParam.u4ThumbHeight )
    {
        MY_LOGD("thumbnail jpeg");
        updateData(
                ECamShot_DATA_MSG_JPEG,
                DATA_DUALIT_M | DATA_THUMBJPEG | DATA_THUMBNAILJPEG_CB,
                ENode_DATA_MSG_JPEG, // callback enable data
                *pNeededData,
                *pNeededCb
                );
    }

    // jps
    updateData(
            ECamShot_DATA_MSG_JPS,
            DATA_DUALITSBS | DATA_MAINJPS | DATA_MAINJPS_CB,
            ENode_DATA_MSG_JPS, // callback enable data
            *pNeededData,
            *pNeededCb
            );

#undef updateData

    MY_LOGD("pNeededData(0x%8x) pNeededCb(0x%8x)", *pNeededData, *pNeededCb);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
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
        MY_LOGD("data(0x%x)", pMap2Node->data);
        if( pMap2Node->data & neededData )
        {
            *pNodeData |= pMap2Node->datapath;
            MY_LOGD("data(0x%x),  datapath(0x%x)", pMap2Node->data, pMap2Node->datapath);
        }
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
StereoShot::
connectNodes(MUINT32 NodeDataSet)
{
    MBOOL ret = MTRUE;

    // [ P1Node -- CtrlNode ]
    mpGraph->connectData(   PASS1_RESIZEDRAW,       CONTROL_RESIZEDRAW,     mpPass1_Main2,          mpCtrl_Main2);
    mpGraph->connectData(   PASS1_RESIZEDRAW,       CONTROL_RESIZEDRAW,     mpPass1,                mpCtrl);
    mpGraph->connectData(   PASS1_FULLRAW,          CONTROL_FULLRAW,        mpPass1_Main2,          mpCtrl_Main2);
    mpGraph->connectData(   PASS1_FULLRAW,          CONTROL_FULLRAW,        mpPass1,                mpCtrl);
    // [ CtrlNode -- SNode ]
    mpGraph->connectData(   CONTROL_CAP_SRC,        STEREO_SRC,             mpCtrl_Main2,           mpStereoNode_Main2);
    mpGraph->connectData(   CONTROL_PRV_SRC,        STEREO_SRC,             mpCtrl,                 mpStereoNode);
    mpGraph->connectData(   CONTROL_CAP_SRC,        PASS2_CAP_SRC,          mpCtrl,                 mpStereoCapNode);
    // [ SNode -- SCNode (do stereo feature algo) ]
    mpGraph->connectData(   STEREO_IMG,             STEREO_CTRL_IMG_1,      mpStereoNode_Main2,     mpStereoCtrlNode);
    mpGraph->connectData(   STEREO_FEO,             STEREO_CTRL_FEO_1,      mpStereoNode_Main2,     mpStereoCtrlNode);
    mpGraph->connectData(   STEREO_RGB,             STEREO_CTRL_RGB_1,      mpStereoNode_Main2,     mpStereoCtrlNode);
    mpGraph->connectData(   STEREO_IMG,             STEREO_CTRL_IMG_0,      mpStereoNode,           mpStereoCtrlNode);
    mpGraph->connectData(   STEREO_FEO,             STEREO_CTRL_FEO_0,      mpStereoNode,           mpStereoCtrlNode);
    mpGraph->connectData(   STEREO_RGB,             STEREO_CTRL_RGB_0,      mpStereoNode,           mpStereoCtrlNode);
    mpGraph->connectData(   PASS2_CAP_DST_0,        STEREO_CTRL_MAIN_SRC,   mpStereoCapNode,        mpStereoCtrlNode);
    // [SCNode -- ITNode (MAIN image) ]
    mpGraph->connectData(   STEREO_CTRL_MAIN_DST,   TRANSFORM_SRC,          mpStereoCtrlNode,       mpITNode);
    // [SCNode -- DualITNode (SBS image) ]
    mpGraph->connectData(   STEREO_CTRL_DST_M,      DUALIT_SRC_0,           mpStereoCtrlNode,       mpDualITNode);
    mpGraph->connectData(   STEREO_CTRL_DST_S,      DUALIT_SRC_1,           mpStereoCtrlNode,       mpDualITNode);
    // [ITNode -- JpegEncNode]
    mpGraph->connectData(   TRANSFORM_DST_1,        JPEG_ENC_HW_SRC,        mpITNode,               mpJpegencHw);
    // [DualITNode -- JpegEncNode]
    mpGraph->connectData(   DUALIT_DST_0,           JPEG_ENC_SW_SRC,        mpDualITNode,           mpJpegencSw);
    // [JpegEncNode -- CBNode]
    mpGraph->connectData(   JPEG_ENC_HW_DST,        SHOTCB_JPEG_SRC,        mpJpegencHw,            mpCallback);
    mpGraph->connectData(   JPEG_ENC_SW_DST,        SHOTCB_THUMBNAIL_SRC,   mpJpegencSw,            mpCallback);
    mpGraph->connectData(   DUALIT_DST_2,           JPEG_ENC_HW_SRC,        mpDualITNode,           mpJpsEncHw);
    mpGraph->connectData(   JPEG_ENC_HW_DST,        SHOTCB_JPS_SRC,         mpJpsEncHw,             mpCallback);
    // [CtrlNode -- CBNode]
    mpGraph->connectData(   CONTROL_DBGINFO,        SHOTCB_DBGINFO_SRC,     mpCtrl,                 mpCallback);
    mpGraph->connectData(   CONTROL_DBGINFO,        SHOTCB_DBGINFO_1_SRC,   mpCtrl_Main2,           mpCallback);


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
StereoShot::
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
StereoShot::
getRotatedPicSize(MUINT32 *pW, MUINT32 *pH, MUINT32 *pThumbW, MUINT32 *pThumbH) const
{
    if( mShotParam.u4PictureTransform == eTransform_ROT_90 ||
            mShotParam.u4PictureTransform == eTransform_ROT_270 )
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
StereoShot::
getNode(MUINT32 node)
{
    switch(node)
    {
        case P1NODE_MAIN:
            return mpPass1;
        case P1NODE_MAIN2:
            return mpPass1_Main2;
        case CTRLNODE:
            return mpCtrl;
        case CTRLNODE_MAIN2:
            return mpCtrl_Main2;
        case SNODE_CAPTURE:
            return mpStereoCapNode;
        case SNODE_MAIN:
            return mpStereoNode;
        case SNODE_MAIN2:
            return mpStereoNode_Main2;
        case SCTRLNODE:
            return mpStereoCtrlNode;
        case ITNODE:
            return mpITNode;
        case DUALITNODE:
            return mpDualITNode;
        case HWJPEGNODE:
            return mpJpegencHw;
        case HWJPSNODE:
            return mpJpsEncHw;
        case CALLBACKNODE:
            return mpCallback;
        case SWJPEGNODE:
            return mpJpegencSw;
        default:
            return NULL;
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
doRegisterBuffers()
{
    MBOOL ret = MTRUE;
    vector<regbuf_t>::const_iterator pRegbuf = mvRegBuf.begin();
    while( pRegbuf != mvRegBuf.end() )
    {
        MUINT32 nodeDataType = mapToNodeData(pRegbuf->muDATA);
        if( nodeDataType ) {
            MY_LOGD("data %d, buf", nodeDataType, pRegbuf->mBuffer);
            ret = ret && mpAllocBufHandler->registerBuffer(nodeDataType, pRegbuf->mBuffer);
            ret = ret && mpAllocBufHandler_Main2->registerBuffer(nodeDataType, pRegbuf->mBuffer);
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
StereoShot::
enableData(MUINT32 const dataType)
{
    MUINT32 nodeDataType = mapToNodeData(dataType);

    if( nodeDataType == 0 ) {
        MY_LOGE("dataType(0x%x) nodeDataType(%d)", dataType, nodeDataType);
        return MTRUE;
    }
    MY_LOGD("dataType(0x%x) nodeDataType(%d)", dataType, nodeDataType);

    MBOOL ret = MTRUE;
    MBOOL bRegistered = (muRegisteredBufType & dataType);
    ret = ret
        && mpGraph->setBufferHandler( nodeDataType, mpAllocBufHandler )
        && (bRegistered ? MTRUE : mpAllocBufHandler->setDataToAlloc(nodeDataType))
        && mpGraph->setBufferHandler( nodeDataType, mpAllocBufHandler_Main2 )
        && (bRegistered ? MTRUE : mpAllocBufHandler_Main2->setDataToAlloc(nodeDataType));

    if( !ret ) {
        MY_LOGE("enableData 0x%x failed", dataType);
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
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
    AllocInfo info(_w, _h, _fmt, bufusage);


    if( !bRegistered )
    {
        ret = ret
            && mpAllocBufHandler->updateRequest( nodeDataType, _trans, usage );

        ret = ret
            && mpAllocBufHandler->updateFlag( nodeDataType, FLAG_BUFFER_ONESHOT )
            && mpAllocBufHandler->requestBuffer( nodeDataType, info );
    }
/*    else if ( dataType == DATA_DUALITSBS )
    {
        MINT32 bufBoundaryInBytes[] = {0,0,0};
        MUINT32 bufStridesInBytes[] = {0,0,0};
        MUINT planeCount = queryPlaneCount(info.fmt);
        for (MUINT32 i = 0; i < planeCount; i++)
        {
            bufStridesInBytes[i] =
                (queryPlaneWidthInPixels(info.fmt,i,info.w)*queryPlaneBitsPerPixel(info.fmt,i))>>3;
        }
        IImageBufferAllocator::ImgParam imgParam(
            info.fmt,
            MSize(info.w,info.h),
            bufStridesInBytes,
            bufBoundaryInBytes,
            planeCount);
        sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create( LOG_TAG, imgParam );
        CHECK_OBJECT(pHeap);
        //
        sp<IImageBuffer> pBuf = pHeap->createImageBuffer();
        CHECK_OBJECT(pBuf);
        if ( !pBuf->lockBuf( LOG_TAG, info.usage ) )
        {
            MY_LOGE("lock Buffer failed");
            return  MFALSE;
        }
        if ( !pBuf->syncCache(eCACHECTRL_INVALID) )
        {
            MY_LOGE("syncCache failed");
            return  MFALSE;
        }
        //
        ret = ret
            && mpAllocBufHandler->registerBuffer(nodeDataType, pBuf.get());

        ret = ret
            && mpAllocBufHandler->updateRequest( nodeDataType, _trans, usage );
    }*/

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
StereoShot::
updateFinishDataMsg(MUINT32 datamsg)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("update finished data: finished(0x%x), data(0x%x)",
            mFinishedData, datamsg);
    mFinishedData |= datamsg;
    //check if all valid data is processed
    if( mFinishedData == (mi4DataMsgSet & ECamShot_DATA_MSG_ALL) )
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


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
reset()
{
    // reset
    muRegisteredBufType = 0x0;
    mbDoShutterCb = MTRUE;
    mFinishedData = 0x0;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
doDebugDump()
{
    MBOOL ret = MTRUE;

    MSize size;
    if( !getSensorSizeFmt(&size) )
        ret = MFALSE;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoShot::
getSensorSizeFmt(MSize* pSize)
{
    MBOOL ret = MTRUE;
    NSCam::SensorStaticInfo sensorInfo;
    //get sensor size
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MINT32 const sensorNum = pHalSensorList->queryNumberOfSensors();

    if(mSensorParam.u4OpenID >= (MUINT32)sensorNum)
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

