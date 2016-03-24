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

#define LOG_TAG "LensSensorDrv"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset
#include <stdio.h> //sprintf
#include <cutils/log.h>

#include "MediaTypes.h"
#include "mcu_drv.h"
#include "lens_sensor_drv.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"

#define DEBUG_LENS_SENSOR_DRV
#ifdef DEBUG_LENS_SENSOR_DRV
#define DRV_DBG(fmt, arg...) ALOGD(LOG_TAG fmt, ##arg)
#define DRV_ERR(fmt, arg...) ALOGE(LOG_TAG "Err: %5d:, " fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif

/*******************************************************************************
*
********************************************************************************/
MCUDrv*
LensSensorDrv::getInstance()
{
    DRV_DBG("[ImgSensorDrv] getInstance \n");
    static LensSensorDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
LensSensorDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
LensSensorDrv::LensSensorDrv()
    : MCUDrv()
    , m_fdMCU_main(-1)
    , m_fdMCU_main2(-1)
    , m_fdMCU_sub(-1)
    , m_userCnt_main(0)
    , m_userCnt_main2(0)
    , m_userCnt_sub(0)
    , m_i4FocusPos_main(0)
    , m_i4FocusPos_main2(0)
    , m_i4FocusPos_sub(0)
{
    DRV_DBG("[LensSensorDrv() construct]\n");
}

/*******************************************************************************
*
********************************************************************************/
LensSensorDrv::~LensSensorDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::init(unsigned int a_u4CurrSensorDev)
{
    DRV_DBG("[init()]\n");
    char  cBuf[64];
    unsigned int a_u4CurrLensIdx, a_userCnt;
    int a_fdMCU=0;
    int err = 0;
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MUINT16 FeaturePara = 0;
    MUINT32 FeatureParaLen = 0;

    if(a_u4CurrSensorDev==MCU_DEV_MAIN)
    {
        a_fdMCU   = m_fdMCU_main;
        a_userCnt = m_userCnt_main;
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_main;
        sprintf(cBuf, "/dev/%s", MCUDrv::m_LensInitFunc_main[a_u4CurrLensIdx].LensDrvName);
        DRV_DBG("[Main Lens Driver]%s\n", cBuf);
    }
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)
    {
        a_fdMCU   = m_fdMCU_main2;
        a_userCnt = m_userCnt_main2;
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_main2;
        sprintf(cBuf, "/dev/%s", MCUDrv::m_LensInitFunc_main2[a_u4CurrLensIdx].LensDrvName);
        DRV_DBG("[Main2 Lens Driver]%s\n", cBuf);

    }
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)
    {
        a_fdMCU   = m_fdMCU_sub;
        a_userCnt = m_userCnt_sub;
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_sub;
        sprintf(cBuf, "/dev/%s", MCUDrv::m_LensInitFunc_sub[a_u4CurrLensIdx].LensDrvName);
        DRV_DBG("[Sub Lens Driver]%s\n", cBuf);
    }
    else return MCUDrv::MCU_INVALID_DRIVER;

    Mutex::Autolock lock(mLock);
    if (a_userCnt <= 0) {
        if (a_fdMCU == -1) {
            a_fdMCU = open(cBuf, O_RDWR);
            if (a_fdMCU < 0)
            {
                DRV_ERR("error opening %s: %s", cBuf, strerror(errno));
                return MCUDrv::MCU_INVALID_DRIVER;
            }
            featureCtrl.FeatureId = SENSOR_FEATURE_INITIALIZE_AF;
            featureCtrl.pFeaturePara = (MUINT8*)&FeaturePara;
            featureCtrl.pFeatureParaLen = &FeatureParaLen;
            err = ioctl(a_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
            if (err < 0) {
                DRV_ERR("[initMCU] ioctl - SENSOR_FEATURE_INITIALIZE_AF, error %s",  strerror(errno));
            }
        }
    }
    a_userCnt++;

    if(a_u4CurrSensorDev==MCU_DEV_MAIN)           m_userCnt_main = a_userCnt;
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)     m_userCnt_main2 = a_userCnt;
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)         m_userCnt_sub = a_userCnt;

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::uninit(unsigned int a_u4CurrSensorDev)
{
    int a_userCnt;
    int a_fdMCU=0;
    if(a_u4CurrSensorDev==MCU_DEV_MAIN)
    {
        a_fdMCU   = m_fdMCU_main;
        a_userCnt = m_userCnt_main;
        DRV_DBG("[Main Lens uninit()]\n");
    }
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)
    {
        a_fdMCU   = m_fdMCU_main2;
        a_userCnt = m_userCnt_main2;
        DRV_DBG("[Main2 Lens uninit()]\n");
    }
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)
    {
        a_fdMCU   = m_fdMCU_sub;
        a_userCnt = m_userCnt_sub;
        DRV_DBG("[Sub Lens uninit()]\n");
    }
    else return MCUDrv::MCU_INVALID_DRIVER;


    Mutex::Autolock lock(mLock);
    if (a_userCnt > 0)
    {
        if (a_fdMCU > 0)     close(a_fdMCU);
        a_fdMCU = -1;
    }
    a_userCnt --;
    if (a_userCnt < 0)   {a_userCnt = 0;}

    if(a_u4CurrSensorDev==MCU_DEV_MAIN)           m_userCnt_main = a_userCnt;
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)     m_userCnt_main2 = a_userCnt;
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)         m_userCnt_sub = a_userCnt;

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::moveMCU(int a_i4FocusPos, unsigned int a_u4CurrSensorDev)
{
    //DRV_DBG("moveMCU() - pos = %d \n", a_i4FocusPos);
    int err = 0;
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MUINT16 FeaturePara = 0;
    MUINT32 FeatureParaLen = 0;

    int a_fdMCU=0;
    if(a_u4CurrSensorDev==MCU_DEV_MAIN)           {a_fdMCU = m_fdMCU_main;}
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)     {a_fdMCU = m_fdMCU_main2;}
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)         {a_fdMCU = m_fdMCU_sub;}

    if (a_fdMCU < 0) {
        DRV_ERR("moveMCU() invalid m_fdMCU =%d\n", a_fdMCU);
        return MCUDrv::MCU_INVALID_DRIVER;
    }

    FeaturePara = (MUINT16)a_i4FocusPos;
    FeatureParaLen = sizeof(MUINT16);

    featureCtrl.FeatureId = SENSOR_FEATURE_MOVE_FOCUS_LENS;
    featureCtrl.pFeaturePara = (MUINT8*)&FeaturePara;
    featureCtrl.pFeatureParaLen = &FeatureParaLen;

    err = ioctl(a_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);

    if (err < 0) {
        DRV_ERR("[moveMCU] ioctl - SENSOR_FEATURE_MOVE_FOCUS_LENS, error %s",  strerror(errno));
        return err;
    }

    if(a_u4CurrSensorDev==MCU_DEV_MAIN)           {m_i4FocusPos_main=a_i4FocusPos;}
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)     {m_i4FocusPos_main2=a_i4FocusPos;}
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)         {m_i4FocusPos_sub=a_i4FocusPos;}
    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::getMCUInfo(mcuMotorInfo *a_pMotorInfo, unsigned int a_u4CurrSensorDev)

{
    int err = 0;
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MUINT32 FeaturePara = 0;
    MUINT32 FeatureParaLen = 4;
    int a_fdMCU=0;
    unsigned long a_i4FocusPos=0;
    if(a_u4CurrSensorDev==MCU_DEV_MAIN)           {a_fdMCU = m_fdMCU_main; a_i4FocusPos=m_i4FocusPos_main;}
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)     {a_fdMCU = m_fdMCU_main2; a_i4FocusPos=m_i4FocusPos_main2;}
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)         {a_fdMCU = m_fdMCU_sub; a_i4FocusPos=m_i4FocusPos_sub;}


    if (a_fdMCU < 0) {
        DRV_ERR("getMCUInfo() invalid m_fdMCU =%d\n", a_fdMCU);
        a_pMotorInfo->bIsMotorOpen = 0;
        return MCUDrv::MCU_INVALID_DRIVER;
    }
    a_pMotorInfo->bIsMotorOpen = 1;
    a_pMotorInfo->u4CurrentPosition = a_i4FocusPos;
    a_pMotorInfo->bIsSupportSR = 0;
    featureCtrl.pFeaturePara = (MUINT8*)&FeaturePara;
    featureCtrl.pFeatureParaLen = &FeatureParaLen;

    featureCtrl.FeatureId = SENSOR_FEATURE_GET_AF_STATUS;
    err = ioctl(a_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
    a_pMotorInfo->bIsMotorMoving = (bool)FeaturePara;

    featureCtrl.FeatureId = SENSOR_FEATURE_GET_AF_INF;
    err = ioctl(a_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
    a_pMotorInfo->u4InfPosition = (MUINT32)FeaturePara;

    featureCtrl.FeatureId = SENSOR_FEATURE_GET_AF_MACRO;
    err = ioctl(a_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
    a_pMotorInfo->u4MacroPosition = (MUINT32)FeaturePara;

    //DRV_DBG("[ImgSensorDrv] [state]%d, [inf]%d, [macro]%d \n", a_pMotorInfo->bIsMotorMoving, a_pMotorInfo->u4InfPosition, a_pMotorInfo->u4MacroPosition);

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::setMCUInfPos(int a_i4FocusPos, unsigned int a_u4CurrSensorDev)
{
    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::setMCUMacroPos(int a_i4FocusPos, unsigned int a_u4CurrSensorDev)
{
    return MCUDrv::MCU_NO_ERROR;
}
int
LensSensorDrv::setMCUParam(int ois_param,unsigned int a_u4CurrSensorDev )
{
    return MCUDrv::MCU_NO_ERROR;
}
/*******************************************************************************
* Send command buffer to MCU, command buffer size is 100 Byte
********************************************************************************/
int
LensSensorDrv::sendMCUCmd( unsigned int a_u4CurrSensorDev, mcuCmd *a_pMcuCmd)
{
    int err, a_fdMCU,a_u4CurrLensIdx;

    if(a_u4CurrSensorDev==MCU_DEV_MAIN)
    {
        a_fdMCU=m_fdMCU_main;
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_main;
    }
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)
    {
        a_fdMCU=m_fdMCU_main2;
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_main2;
    }
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)
    {
        a_fdMCU=m_fdMCU_sub;
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_sub;
    }
    else
    {
        return MCUDrv::MCU_INVALID_DRIVER;
    }

  if (a_fdMCU < 0)
  {
      if (a_u4CurrLensIdx == 0)
      {
            // no error log for dummy lens
          return MCUDrv::MCU_NO_ERROR;
      }
      else
      {
          DRV_ERR("[setMotorCmd] invalid m_fdMCU =%d\n", a_fdMCU);
          return MCUDrv::MCU_INVALID_DRIVER;
      }
  }

    err = ioctl( a_fdMCU, mcuIOC_S_SENDCMD, a_pMcuCmd);
    if (err < 0)
    {
        DRV_ERR("[setPWMCmd] ioctl - mcuIOC_S_SENDCMD, error %s",  strerror(errno));
        return err;
    }

  return MCUDrv::MCU_NO_ERROR;
}


