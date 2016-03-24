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

#define LOG_TAG "aaa/Hal3AAdapter3"
//
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include "Info.h"
#include "../Hal3AAdapter3.h"
//
#include <dlfcn.h>
//

using namespace NS3A;
using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
static
MBOOL
impBuildStaticInfo_by_SymbolName(
    String8 const&  s8Symbol,
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
typedef MBOOL (*PFN_T)(
        Info const& rInfo,
        IMetadata&  rMetadata
    );
    //
    PFN_T pfn = (PFN_T)::dlsym(RTLD_DEFAULT, s8Symbol.string());
    if  ( ! pfn ) {
        MY_LOGW("%s not found", s8Symbol.string());
        return  MFALSE;
    }
    //
    MBOOL const ret = pfn(rInfo, rMetadata);
    MY_LOGW_IF(!ret, "%s returns failure", s8Symbol.string());
    //
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MBOOL
impBuildStaticInfo(
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
    MBOOL ret = MFALSE;

    // lens
    {
        char const*const pTypeName = "LENS";
        //
        String8 const s8Symbol = String8::format("%s_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.LensDrvName().string());
        ret = impBuildStaticInfo_by_SymbolName(s8Symbol, rInfo, rMetadata);
        if  ( ! ret ) {
            MY_LOGE_IF(1, "Fail for %s", s8Symbol.string());
            return  ret;
        }
    }

    // sensor
    {
        char const*const pTypeName = "1";
        //
        String8 const s8Symbol = String8::format("%s_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.sensorDrvName().string());
        ret = impBuildStaticInfo_by_SymbolName(s8Symbol, rInfo, rMetadata);
        if  ( ! ret ) {
            MY_LOGE_IF(1, "Fail for %s", s8Symbol.string());
            return  ret;
        }
    }

    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
Hal3AAdapter3::
buildStaticInfo(IMetadata& rMetadata) const
{
    IHalSensorList* pHalSensorList = IHalSensorList::get();

    int i4SensorType = pHalSensorList->queryType(mi4SensorIdx);
    const char* strSensorDrvName = pHalSensorList->queryDriverName(mi4SensorIdx);

    // FIX ME : get lens driver name
    Info rInfo(mi4SensorIdx, i4SensorType, strSensorDrvName, "OV8825AF");
    IMetadata metadata;

    if  ( ! impBuildStaticInfo(rInfo, metadata) )
    {
        MY_LOGE(
            "Fail to build static info for (%s, %s) index(%d) type(%d)",
            rInfo.sensorDrvName().string(), rInfo.LensDrvName().string(), rInfo.sensorIndex(), rInfo.sensorType()
        );
        return  MFALSE;
    }
    //
    metadata.sort();
    rMetadata = metadata;
    //
    return  MTRUE;
}

