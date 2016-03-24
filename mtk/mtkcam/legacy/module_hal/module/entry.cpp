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

#define LOG_TAG "MtkCam/module"
//
#include <Log.h>
#include <common.h>
#include <utils/common.h>
#include <device/module.h>
//
using namespace android;
using namespace NSCam;


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
extern "C"
status_t
MtkCam_getProperty(String8 const& key, String8& value)
{
    return  Utils::Property::tryGet(key, value)
        ?   OK
        :   NAME_NOT_FOUND
            ;
}


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
status_t
MtkCam_setProperty(String8 const& key, String8 const& value)
{
    Utils::Property::set(key, value);
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static
mtk_camera_module_methods*
get_mtk_camera_module_methods()
{
    static
    mtk_camera_module_methods
    _methods =
    {
        getProperty : MtkCam_getProperty,
        setProperty : MtkCam_setProperty,
    };
    return  &_methods;
}


/******************************************************************************
 *
 ******************************************************************************/
#include "module.h"
static
mtk_camera_module_methods*
mtk_camera_module_methods_instantiate();


/******************************************************************************
 * Implementation of camera_module
 ******************************************************************************/
mtk_camera_module
HAL_MODULE_INFO_SYM =
{
    common          : get_camera_module(),
    //
    methods_symbols : mtk_camera_module_methods_symbols,
    methods         : mtk_camera_module_methods_instantiate(),
};


/******************************************************************************
 *
 ******************************************************************************/
static
mtk_camera_module_methods*
mtk_camera_module_methods_instantiate()
{
    camera_module const* p_camera_module = &HAL_MODULE_INFO_SYM.common;
    MY_LOGD("module_api_version:0x%x", p_camera_module->common.module_api_version);
    //
    //  (1) Prepare One-shot init.
    Utils::Property::clear();
    //
    for (int i = 0; mtk_camera_module_methods_symbols[i]; i++) {
        MY_LOGD("%02d: %s", i, mtk_camera_module_methods_symbols[i]);
    }
    //
    return get_mtk_camera_module_methods();
}


/******************************************************************************
 *
 ******************************************************************************/
#if 0
static
void
test()
{
    typedef char const*const*   methods_symbols_T;

    typedef android::status_t   (*getProperty_T)(
                                    android::String8 const& key,
                                    android::String8& value
                                );

    typedef android::status_t   (*setProperty_T)(
                                    android::String8 const& key,
                                    android::String8 const& value
                                );

    methods_symbols_T* p_methods_symbols = (methods_symbols_T*)((intptr_t)(&HAL_MODULE_INFO_SYM) + sizeof(camera_module));
    intptr_t methods_base = *(intptr_t*)((intptr_t)p_methods_symbols + sizeof(intptr_t));

    getProperty_T* p_getProperty = (getProperty_T*)(methods_base + sizeof(intptr_t[0]));

    setProperty_T* p_setProperty = (setProperty_T*)(methods_base + sizeof(intptr_t[1]));

    MY_LOGD("%p %p", HAL_MODULE_INFO_SYM.methods_symbols, *p_methods_symbols);
    MY_LOGD("%p %p", HAL_MODULE_INFO_SYM.methods->getProperty, *p_getProperty);
    MY_LOGD("%p %p", HAL_MODULE_INFO_SYM.methods->setProperty, *p_setProperty);

    for (int i = 0; (*p_methods_symbols)[i]; i++) {

        MY_LOGD("%02d: %s", i, (*p_methods_symbols)[i]);

        if  ( 0 == ::strcmp((*p_methods_symbols)[i], "getProperty") )
        {
            p_getProperty = (getProperty_T*)(methods_base + sizeof(intptr_t[i]));

            String8 key("ABCD"), value("EFGH");
            (*p_getProperty)(key, value);
        }
        else
        if  ( 0 == ::strcmp((*p_methods_symbols)[i], "setProperty") )
        {
            p_setProperty = (setProperty_T*)(methods_base + sizeof(intptr_t[i]));

            String8 key("ABCD_XYZ"), value("EFGH_YUV");
            (*p_setProperty)(key, value);
        }
    }
}
struct Test
{
    Test()
    {
        HAL_MODULE_INFO_SYM;
        test();
    }
};
static Test gTest;
#endif

