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
#define LOG_TAG "Hal3AAdapter3"

#include "Hal3AAdapter3.h"

#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <utils/Atomic.h>

using namespace android;
using namespace NS3A;
using namespace NSCam;

/*******************************************************************************
*
********************************************************************************/
Hal3AAdapter3*
Hal3AAdapter3::
createInstance(MINT32 const i4SensorIdx, const char* strUser)
{
    switch (i4SensorIdx)
    {
    case 0:
        {
            static Hal3AAdapter3 _singleton(0);
            _singleton.init();
            return &_singleton;
        }
    case 1:
        {
            static Hal3AAdapter3 _singleton(1);
            _singleton.init();
            return &_singleton;
        }
    case 2:
        {
            static Hal3AAdapter3 _singleton(2);
            _singleton.init();
            return &_singleton;
        }
    case 3:
        {
            static Hal3AAdapter3 _singleton(3);
            _singleton.init();
            return &_singleton;
        }
    default:
        return NULL;
    }
}

Hal3AAdapter3::
Hal3AAdapter3(MINT32 const i4SensorIdx)
    : mpHal3aObj(NULL)
    , mi4FrmId(-1)
    , mi4SensorIdx(i4SensorIdx)
    , mi4User(0)
    , mLock()
    , mu4MetaResultQueueCapacity(6)
{

    if (! buildStaticInfo(mMetaStaticInfo))
    {
        CAM_LOGE("[%s] Static Info load error.", __FUNCTION__);
    }

    CAM_LOGW("[%s] mi4SensorIdx(0x%04x)\n",
        __FUNCTION__, i4SensorIdx);
}

MVOID
Hal3AAdapter3::
destroyInstance(const char* strUser)
{
    uninit();
}

MBOOL
Hal3AAdapter3::
init()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {

    }
    else
    {
        mpHal3aObj = Hal3AIf::createInstance(mi4SensorIdx);
        CAM_LOGD("[%s] mi4SensorIdx(0x%04x), mpHal3aObj(0x%08x)\n",
            __FUNCTION__, mi4SensorIdx, mpHal3aObj);
    }

    android_atomic_inc(&mi4User);

    return MTRUE;
}

MBOOL
Hal3AAdapter3::
uninit()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        // More than one user, so decrease one User.
        android_atomic_dec(&mi4User);

        if (mi4User == 0) // There is no more User after decrease one User
        {
            mpHal3aObj->destroyInstance();
            mpHal3aObj = NULL;
            CAM_LOGD("[%s] mi4SensorIdx(0x%04x)\n", __FUNCTION__, mi4SensorIdx);
        }
        else    // There are still some users.
        {
            //CAM_LOGD(m_bDebugEnable,"Still %d users \n", mi4User);
        }
    }

    return MTRUE;
}


const IMetadata*
Hal3AAdapter3::
queryStaticInfo() const
{
    return &mMetaStaticInfo;
}

MINT32
Hal3AAdapter3::
set(const List<IMetadata>& controls)
{
    MINT32 i;
    MINT32 i4FrmId = 0;
    MINT32 i4AfTrigId = 0;
    MINT32 i4AePreCapId = 0;
    MINT32 i4AeSensitivity = 0;
    MINT64 i8AeExposureTime = 0;
    MUINT8 u1AfTrig = MTK_CONTROL_AF_TRIGGER_IDLE;
    MUINT8 u1AePrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
    MUINT8 u1CapIntent = -1;

    CAM_LOGW("[%s] mi4SensorIdx(0x%04x)\n", __FUNCTION__, mi4SensorIdx);

    MetadataList_T::const_iterator it = controls.begin();

    for (i = 0; i < 4 && it != controls.end(); i++, it++)
    {
        MINT32 j;
        const IMetadata& metadata = *it;
        MINT32 cnt = metadata.count();

        CAM_LOGW("[%s] metadata[%d].count(%d)\n", __FUNCTION__, i, cnt);
        //
        for (j = 0; j < cnt; j++)
        {
            IMetadata::IEntry const& entry = metadata.entryAt(j);
            MUINT32 tag = entry.tag();

            // for control only, ignore; for control+dynamic, not ignore.
            MBOOL fgControlOnly = MTRUE;

            // Only update 3A setting when delay matches.
            if (i != getDelay(tag)) continue;

            // convert metadata tag into 3A settings.
            switch (tag)
            {
            case MTK_REQUEST_FRAME_COUNT:   // dynamic
                {
                    // set magic number
                    i4FrmId = entry.itemAt(0, Type2Type< MINT32 >());
                    setFrameId(i4FrmId);
                    fgControlOnly = MFALSE;
                }
                break;
            case MTK_CONTROL_MODE:  // dynamic
                //FIX ME:
                //mParams.u4ControlMode = entry.itemAt(0, Type2Type< MUINT8 >());
                fgControlOnly = MFALSE;
                break;
            case MTK_CONTROL_CAPTURE_INTENT:
                u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                break;

            // AWB
            case MTK_CONTROL_AWB_LOCK:
                {
                    MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.bIsAWBLock = (bLock==MTK_CONTROL_AWB_LOCK_ON) ? MTRUE : MFALSE;
                }
                break;
            case MTK_CONTROL_AWB_MODE:  // dynamic
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AwbMode = u1Mode;
                    fgControlOnly = MFALSE;
                }
                break;


            // AE
            case MTK_CONTROL_AE_ANTIBANDING_MODE:
                {
                    MINT32 i4Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AntiBandingMode = i4Mode;
                }
                break;
            case MTK_CONTROL_AE_EXPOSURE_COMPENSATION:
                {
                    MINT32 i4ExpIdx = entry.itemAt(0, Type2Type< MINT32 >());
                    mParams.i4ExpIndex = i4ExpIdx;
                }
                break;
            case MTK_CONTROL_AE_LOCK:
                {
                    MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.bIsAELock = (bLock==MTK_CONTROL_AE_LOCK_ON) ? MTRUE : MFALSE;
                }
                break;
            case MTK_CONTROL_AE_MODE:
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AeMode = u1Mode;
                }
                break;
            case MTK_CONTROL_AE_REGIONS:    // dynamic
            case MTK_CONTROL_AWB_REGIONS:
                {
                    MINT32 numRgns = entry.count() / 5;
                    mParams.rMeteringAreas.u4Count = numRgns;
                    for (MINT32 k = 0; k < numRgns; k++)
                    {
                        CameraArea_T& rArea = mParams.rMeteringAreas.rAreas[k];
                        rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                        rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                        rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                        rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                        rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                    }
                    fgControlOnly = MFALSE;
                }
                break;
            case MTK_CONTROL_AE_TARGET_FPS_RANGE:
                {
                    mParams.i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
                    mParams.i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
                }
                break;
            case MTK_CONTROL_AE_PRECAPTURE_TRIGGER:
                {
                    u1AePrecapTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                }
                break;
            case MTK_CONTROL_AE_PRECAPTURE_ID:
                {
                    i4AePreCapId = entry.itemAt(0, Type2Type< MINT32 >());
                }
                break;

            case MTK_FLASH_MODE:
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4StrobeMode = u1Mode;
                }
                break;

            // Sensor
            case MTK_SENSOR_EXPOSURE_TIME:
                {
                    i8AeExposureTime = entry.itemAt(0, Type2Type< MINT64 >());
                }
                break;
            case MTK_SENSOR_SENSITIVITY:
                {
                    i4AeSensitivity = entry.itemAt(0, Type2Type< MINT32 >());
                }
                break;
            case MTK_SENSOR_FRAME_DURATION:
                break;

            // AF
            case MTK_CONTROL_AF_MODE:
                {
                    MUINT8 u1AfMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AfMode = u1AfMode;
                }
                break;
            case MTK_CONTROL_AF_REGIONS:    // dynamic
                {
                    MINT32 numRgns = entry.count() / 5;
                    mParams.rFocusAreas.u4Count = numRgns;
                    for (MINT32 k = 0; k < numRgns; k++)
                    {
                        CameraArea_T& rArea = mParams.rFocusAreas.rAreas[k];
                        rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                        rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                        rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                        rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                        rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                    }
                    fgControlOnly = MFALSE;
                }
                break;
            case MTK_CONTROL_AF_TRIGGER:
                {
                    u1AfTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                }
                break;
            case MTK_CONTROL_AF_TRIGGER_ID:
                {
                    i4AfTrigId = entry.itemAt(0, Type2Type< MINT32 >());
                }
                break;


            // ISP
            case MTK_CONTROL_EFFECT_MODE:
                {
                    MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4EffectMode = u1EffectMode;
                }
                break;
            case MTK_CONTROL_SCENE_MODE:
                {
                    MUINT8 u1SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4SceneMode = u1SceneMode;
                }
                break;

            default:
                break;
            }

            if (!fgControlOnly && (1 == i))
            {
                // update result for control/dynamic, including MTK_REQUEST_FRAME_COUNT
                mMetaResult.update(tag, entry);
            }
        }
    }

    // set m_Params
    mpHal3aObj->setFrameId(i4FrmId);
    mpHal3aObj->setParams(mParams);

    if (mParams.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
    {
        // FIX ME: should be the same delay frame?

    }

    // trigger
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)
    {
        // todo: trigger id
        mpHal3aObj->autoFocus();
    }
    else if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_CANCEL)
    {
        // todo: trigger id
        mpHal3aObj->cancelAutoFocus();
    }

    if (u1AePrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE)
    {
        // todo
    }

    // todo: u1CapIntent

    return 0;
}

MINT32
Hal3AAdapter3::
get(MINT32 frmId, IMetadata& result)
{
    CAM_LOGW("[%s] frmId(%d)\n", __FUNCTION__, frmId);

    // 1: get result from 3AMgr
    Result_T rResult;
    mpHal3aObj->getResult(frmId, rResult);

    // 2: convert result into metadata
    MINT32 i4FrmId = mMetaResult.entryFor(MTK_REQUEST_FRAME_COUNT).itemAt(0, Type2Type< MINT32>());
    CAM_LOGW("[%s] Current result will be put to R#%d\n", __FUNCTION__, i4FrmId);
    convertResultToMeta(rResult, mMetaResult);

    // 3. put current metadata result into queue
    mMetaResultQueue.push_back(mMetaResult);

    // 4. copy result at frmId to MW
    MetadataList_T::iterator it = mMetaResultQueue.begin();
    for (; it != mMetaResultQueue.end(); it++)
    {
        MINT32 i4Id = it->entryFor(MTK_REQUEST_FRAME_COUNT).itemAt(0, Type2Type< MINT32 >());
        if (frmId == i4Id)
        {
            CAM_LOGW("[%s] Got i4Id(%d)\n", __FUNCTION__, i4Id);
            //result = *it;
            IMetadata& metaResult = *it;
            for (MUINT32 i = 0; i < metaResult.count(); i++)
            {
                const IMetadata::IEntry& entry = metaResult.entryAt(i);
                result.update(entry.tag(), entry);
            }
            break;
        }
    }

    // fail to get the specified results, use current.
    if (it == mMetaResultQueue.end())
    {
        MINT32 i4Id = mMetaResult.entryFor(MTK_REQUEST_FRAME_COUNT).itemAt(0, Type2Type< MINT32 >());
        CAM_LOGW("[%s] fail to get the specified results, use current. i4Id(%d)\n", __FUNCTION__, i4Id);
        for (MUINT32 i = 0; i < mMetaResult.count(); i++)
        {
            const IMetadata::IEntry& entry = mMetaResult.entryAt(i);
            result.update(entry.tag(), entry);
        }
    }

    // remove
    if (mMetaResultQueue.size() > mu4MetaResultQueueCapacity)
    {
        CAM_LOGW("[%s] Queue fulls (%d). Remove front\n", __FUNCTION__, mMetaResultQueue.size());
        mMetaResultQueue.erase(mMetaResultQueue.begin());
    }

    return 0;
}


MINT32
Hal3AAdapter3::
convertResultToMeta(const Result_T& rResult, IMetadata& rMetaResult) const
{
    // convert result into metadata
    MUINT8 u1AwbState = rResult.eAwbState;
    IMetadata::IEntry entryAwbState(MTK_CONTROL_AWB_STATE);
    entryAwbState.push_back(u1AwbState, Type2Type< MUINT8 >());
    rMetaResult.update(MTK_CONTROL_AWB_STATE, entryAwbState);

    MUINT8 u1AeState = rResult.eAeState;
    IMetadata::IEntry entryAeState(MTK_CONTROL_AE_STATE);
    entryAeState.push_back(u1AeState, Type2Type< MUINT8 >());
    rMetaResult.update(MTK_CONTROL_AE_STATE, entryAeState);

    MUINT8 u1AfState = rResult.eAfState;
    IMetadata::IEntry entryAfState(MTK_CONTROL_AF_STATE);
    entryAfState.push_back(u1AfState, Type2Type< MUINT8 >());
    rMetaResult.update(MTK_CONTROL_AF_STATE, entryAfState);

    return 0;
}

MINT32
Hal3AAdapter3::
getDelay(IMetadata& delay_info) const
{
    return 0;
}

MINT32
Hal3AAdapter3::
getDelay(MUINT32 tag) const
{
    MINT32 i4Delay = 0;
    // temp
    switch (tag)
    {
    case MTK_REQUEST_FRAME_COUNT:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AE_REGIONS:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AF_MODE:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AF_REGIONS:
        i4Delay = 3;
        break;
    //case MTK_CONTROL_AF_APERTURE:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_FOCALLENGTH:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_FOCUSDISTANCE:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_OIS:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_SHARPNESSMAPMODE:
    //    i4Delay = 1;
    //    break;

    case MTK_CONTROL_AWB_REGIONS:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AWB_MODE:
        i4Delay = 3;
        break;
    }
    return i4Delay;
}

MVOID
Hal3AAdapter3::
setSensorMode(MINT32 i4SensorMode)
{
    mpHal3aObj->setSensorMode(i4SensorMode);
}

MBOOL
Hal3AAdapter3::
sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg)
{
    // todo
    return MFALSE;
}

MVOID
Hal3AAdapter3::
setFrameId(MINT32 i4FrmId)
{
    // todo
}

MINT32
Hal3AAdapter3::
getFrameId() const
{
    // todo
    return 0;
}


