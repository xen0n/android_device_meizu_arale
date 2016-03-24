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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief

#include <stdio.h>
//
#include <errno.h>
#include <fcntl.h>

#include <stdlib.h>

#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/acdk/AcdkCommon.h>
#include <mtkcam/acdk/AcdkIF.h>

#define MY_LOGV(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)

using namespace NSCam;



MBOOL gCapCBDone = MFALSE;


void vAutoCapCb_patg(void *a_pParam);

/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    int bRet = 0;
    MUINT32 srcDev = 0, u4RetLen = 0, u32checksum = 0, previewTimeMs = 0;
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
    ACDK_PREVIEW_STRUCT rACDKPrvConfig;
    ACDK_CAPTURE_STRUCT rACDKCapConfig;

    (void)(argc);
    (void)(argv);
    /*
        ACDK_CMD_SET_SRC_DEV
        ACDK_CMD_PREVIEW_START
        ACDK_CMD_GET_CHECKSUM
        ACDK_CMD_CAPTURE
        ACDK_CMD_PREVIEW_STOP
        ACDK_CMD_RESET_LAYER_BUFFER
    */

    srcDev = SENSOR_DEV_MAIN;

    bRet = MDK_Open();
    if (!bRet)
    {
        MY_LOGE("open fail");
        goto EXIT_MAIN;
    }

    rAcdkFeatureInfo.puParaIn = (MUINT8 *)&srcDev;
    rAcdkFeatureInfo.u4ParaInLen = sizeof(srcDev);
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = MDK_IOControl(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGD("ACDK_FEATURE_SET_SRC_DEV Fail: %d\n", srcDev);
        goto EXIT_MAIN;
    }


    bRet = MDK_Init();
    if (!bRet)
    {
        MY_LOGE("init fail");
        goto EXIT_MAIN;
    }


    rACDKPrvConfig.fpPrvCB = NULL;
    rACDKPrvConfig.u4PrvW = 320;
    rACDKPrvConfig.u4PrvH = 240;
    rACDKPrvConfig.u16HDRModeEn = 0;
    rACDKPrvConfig.u16PreviewTestPatEn = 1;
    rACDKPrvConfig.u4OperaType = 0;
    rACDKPrvConfig.eOperaMode = ACDK_OPT_FACTORY_MODE;

    rAcdkFeatureInfo.puParaIn     = (MUINT8*)&rACDKPrvConfig;
    rAcdkFeatureInfo.u4ParaInLen  = sizeof(rACDKPrvConfig);
    rAcdkFeatureInfo.puParaOut    = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = MDK_IOControl(ACDK_CMD_PREVIEW_START, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl preview start fail");
        goto EXIT_MAIN;
    }


    previewTimeMs = 5000;
    for (; previewTimeMs; previewTimeMs--)
        usleep(1000);


    rAcdkFeatureInfo.puParaIn     = NULL;
    rAcdkFeatureInfo.u4ParaInLen  = 0;
    rAcdkFeatureInfo.puParaOut    = (MUINT8 *)&u32checksum;
    rAcdkFeatureInfo.u4ParaOutLen = sizeof(u32checksum);
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = MDK_IOControl(ACDK_CMD_GET_CHECKSUM, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl get checksum fail");
        goto EXIT_MAIN;
    }


    rACDKCapConfig.eOutputFormat = PURE_RAW10_TYPE;
    rACDKCapConfig.eCameraMode   = CAPTURE_MODE;
    rACDKCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;
    rACDKCapConfig.u2JPEGEncWidth  = 0;
    rACDKCapConfig.u2JPEGEncHeight = 0;
    rACDKCapConfig.fpCapCB = vAutoCapCb_patg;
    rACDKCapConfig.i4IsSave = 0;    // 0-no save, 1-save
    rACDKCapConfig.bUnPack = MFALSE;
    rACDKCapConfig.MFLL_En = MFALSE;
    rACDKCapConfig.HDRModeEn = 0;
    rACDKCapConfig.eMultiNR = EMultiNR_Off;
    rACDKCapConfig.u4OperaType = 0;

    rAcdkFeatureInfo.puParaIn     = (MUINT8*)&rACDKCapConfig;
    rAcdkFeatureInfo.u4ParaInLen  = sizeof(rACDKCapConfig);
    rAcdkFeatureInfo.puParaOut    = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

    gCapCBDone = MFALSE;
    bRet = MDK_IOControl(ACDK_CMD_CAPTURE, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl capture fail");
        goto EXIT_MAIN;
    }

    MY_LOGD("wait capture done\n");
    while(!gCapCBDone)
    {
        usleep(1000);
    }


    rAcdkFeatureInfo.puParaIn     = NULL;
    rAcdkFeatureInfo.u4ParaInLen  = 0;
    rAcdkFeatureInfo.puParaOut    = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = MDK_IOControl(ACDK_CMD_PREVIEW_STOP, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl preview stop fail");
        goto EXIT_MAIN;
    }


    rAcdkFeatureInfo.puParaIn = NULL;
    rAcdkFeatureInfo.u4ParaInLen = 0;
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;
    bRet = MDK_IOControl(ACDK_CMD_RESET_LAYER_BUFFER, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGE("ioctl reset layer fail");
        goto EXIT_MAIN;
    }


    MDK_DeInit();
    MDK_Close();


EXIT_MAIN:

    return bRet;
}


void vAutoCapCb_patg(void *a_pParam)
{
    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    MY_LOGD("Capture CB %p !", a_pParam);
    gCapCBDone = MTRUE;
}

