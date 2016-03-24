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
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
///////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "CamPreviewTest"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <mtkcam/acdk/AcdkCommon.h>
#include <mtkcam/acdk/AcdkIF.h>
#include <signal.h>

/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

//For Acdk
MUINT32 srcDev = 1; //the source sensor,1 for main, 2 for sub


/******************************************************************************
*
*  acdkIFInit, init the acdk interface
*******************************************************************************
*/

static int acdkIFInit()
{
    MY_LOGD("%s : Open ACDK\n",__FUNCTION__);

    //====== Local Variable ======

    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
    bool bRet;
    unsigned int u4RetLen;

    //====== Create ACDK Object ======
    if (MDK_Open() == MFALSE)
    {
        MY_LOGD("MDK_Open() Fail \n");
        return -1;
    }

    //====== Select Camera Sensor ======

    rAcdkFeatureInfo.puParaIn = (MUINT8 *)&srcDev;
    rAcdkFeatureInfo.u4ParaInLen = sizeof(int);
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

    MY_LOGD("%s : srcDev:%d\n",__FUNCTION__,srcDev);
    bRet = MDK_IOControl(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);
    if (!bRet)
    {
        MY_LOGD("ACDK_FEATURE_SET_SRC_DEV Fail: %d\n",srcDev);
        return -1;
    }

    //====== Initialize ACDK ======

    MY_LOGD("%s : Init ACDK\n",__FUNCTION__);
    if(MDK_Init() == MFALSE)
    {
        return -1;
    }

    return 0;
}



/******************************************************************************
*

*bSendDataToACDK, wrapper for Mdk_IOControl
*******************************************************************************
*/

static bool bSendDataToACDK(eACDK_COMMAND    FeatureID,
                                     MUINT8*  pInAddr,
                                     MUINT32  nInBufferSize,
                                     MUINT8*  pOutAddr,
                                     MUINT32  nOutBufferSize,
                                     MUINT32* pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn     = pInAddr;
    rAcdkFeatureInfo.u4ParaInLen  = nInBufferSize;
    rAcdkFeatureInfo.puParaOut    = pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;

    return (MDK_IOControl(FeatureID, &rAcdkFeatureInfo));
}


/////////////////////////////////////////////////////////////////////////
//
//   camera_preview_test () -
//!
//!  brief for camera preview test
//
/////////////////////////////////////////////////////////////////////////
static MINT32 camera_preview_test(MUINT16 scenario)
{
    MY_LOGD("%s ,Enter\n", __FUNCTION__);

    ACDK_PREVIEW_STRUCT rACDKPrvConfig;
    rACDKPrvConfig.fpPrvCB = NULL;
    rACDKPrvConfig.u4PrvW = 240;
    rACDKPrvConfig.u4PrvH = 320;
    rACDKPrvConfig.u16PreviewTestPatEn = scenario;//used to change scenario

    MUINT32 u4RetLen = 0;
    rACDKPrvConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;

    bool bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_START,
                                (MUINT8 *)&rACDKPrvConfig,
                                sizeof(ACDK_PREVIEW_STRUCT),
                                NULL,
                                0,
                                &u4RetLen);

    if (!bRet)
    {
        MY_LOGD("ACDK_PREVIEW_STRUCT Fail\n");
        return 1;
    }


    MY_LOGD("%s ,Exit\n", __FUNCTION__);

    return 0;
}

static int camera_preview_stop(void)
{

    MY_LOGD("[camera_preview_stop] Stop Camera Preview\n");

    unsigned int u4RetLen = 0;
    bool bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP, NULL, 0, NULL, 0, &
u4RetLen);


    if (!bRet)
    {
      printf("camera_preview_stop fail\n");
        return -1;
    }

    MY_LOGD("[camera_preview_stop] X\n");
    return 0;
}

/******************************************************************************
*

*  Main Function
*******************************************************************************
*/



int main_acdkpreview(int argc, char** argv)
{

    MUINT32 u4RetLen = 0, oriShutter = 0, u32checksum = 0;
    MBOOL bRet;
    MINT32 chkcnt = 0;
    MUINT32 waitTime = 3;
    MUINT16 scenario = 0;



    printf("[CAM : ACDK SingleShot Test]\n");
    if(argc >1)
        srcDev = atoi(argv[1]); // test main cam
    if(argc >2)
        waitTime = atoi(argv[2]); // time out to stop preview
    if(waitTime < 3)
        waitTime = 3;

    if(argc >3)
        scenario = atoi(argv[3]); // time out to stop preview

    if(srcDev != 1 && srcDev != 2)
        srcDev = 1;

    printf("init start srcDev = %d timeout = %d scenario = %d\n", srcDev, waitTime, scenario);

    bRet=acdkIFInit();//2014.06.19 John, Init the ACDK


    camera_preview_test(scenario);//2014.06.19 John, Acdk will start preiew with test pattern by default

    printf("startPreview done\n");

    while(1)
    {
        usleep(1000000);
        waitTime--;
        if(!waitTime)
            break;

    }
    printf("exit loop\n");


    camera_preview_stop();
    MDK_DeInit();
    MDK_Close();
    return 0;
}
