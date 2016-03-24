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

#ifndef _MEDIATEK_MMSDK_TEST_SERVICE_LOG_H_
#define _MEDIATEK_MMSDK_TEST_SERVICE_LOG_H_


/*******************************************************************************
*
********************************************************************************/
#if 1
//
#include <cutils/log.h>
#define MMSDK_LOGV(fmt, arg...)       printf("V/ " fmt"\r\n", ##arg)
#define MMSDK_LOGD(fmt, arg...)       printf("D/ " fmt"\r\n", ##arg)
#define MMSDK_LOGI(fmt, arg...)       printf("I/ " fmt"\r\n", ##arg)
#define MMSDK_LOGW(fmt, arg...)       printf("W/ " fmt"\r\n", ##arg)
#define MMSDK_LOGE(fmt, arg...)       printf("E/ " fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)
//
#define MY_LOGV(fmt, arg...)        MMSDK_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        MMSDK_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        MMSDK_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        MMSDK_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        MMSDK_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#else
//
#include <cutils/log.h>
#define MMSDK_LOGV(fmt, arg...)       LOGV(fmt"\r\n", ##arg)
#define MMSDK_LOGD(fmt, arg...)       LOGD(fmt"\r\n", ##arg)
#define MMSDK_LOGI(fmt, arg...)       LOGI(fmt"\r\n", ##arg)
#define MMSDK_LOGW(fmt, arg...)       LOGW(fmt"\r\n", ##arg)
#define MMSDK_LOGE(fmt, arg...)       LOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)
//
#endif
//
//  ASSERT
#define MMSDK_LOGA(...) \
    do { \
        MMSDK_LOGE("[Assert] "__VA_ARGS__); \
        while(1) { ::usleep(500 * 1000); } \
    } while (0)
//
//
//  FATAL
#define MMSDK_LOGF(...) \
    do { \
        MMSDK_LOGE("[Fatal] "__VA_ARGS__); \
        LOG_ALWAYS_FATAL_IF(1, "(%s){#%d:%s}""\r\n", __FUNCTION__, __LINE__, __FILE__); \
    } while (0)


/*******************************************************************************
*
********************************************************************************/
#define MMSDK_LOGV_IF(cond, ...)      do { if ( (cond) ) { MMSDK_LOGV(__VA_ARGS__); } }while(0)
#define MMSDK_LOGD_IF(cond, ...)      do { if ( (cond) ) { MMSDK_LOGD(__VA_ARGS__); } }while(0)
#define MMSDK_LOGI_IF(cond, ...)      do { if ( (cond) ) { MMSDK_LOGI(__VA_ARGS__); } }while(0)
#define MMSDK_LOGW_IF(cond, ...)      do { if ( (cond) ) { MMSDK_LOGW(__VA_ARGS__); } }while(0)
#define MMSDK_LOGE_IF(cond, ...)      do { if ( (cond) ) { MMSDK_LOGE(__VA_ARGS__); } }while(0)
#define MMSDK_LOGA_IF(cond, ...)      do { if ( (cond) ) { MMSDK_LOGA(__VA_ARGS__); } }while(0)
#define MMSDK_LOGF_IF(cond, ...)      do { if ( (cond) ) { MMSDK_LOGF(__VA_ARGS__); } }while(0)


/*******************************************************************************
*
********************************************************************************/
#endif  //_MEDIATEK_MMSDK_TEST_SERVICE_LOG_H_

