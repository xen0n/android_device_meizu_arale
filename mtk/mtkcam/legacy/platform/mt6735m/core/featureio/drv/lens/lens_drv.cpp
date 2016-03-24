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

#define LOG_TAG "LensDrv"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset
#include <stdio.h> //sprintf
#include <cutils/log.h>

#include "MediaTypes.h"
#include "mcu_drv.h"
#include "lens_drv.h"

#define DEBUG_MCU_DRV
#ifdef DEBUG_MCU_DRV
#define DRV_DBG(fmt, arg...) ALOGD(fmt, ##arg)
#define DRV_ERR(fmt, arg...) ALOGE("Err: %5d:, " fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif

/*******************************************************************************
*
********************************************************************************/
MCUDrv*
LensDrv::getInstance()
{
    static LensDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
LensDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
LensDrv::LensDrv()
    : MCUDrv()
    , m_fdMCU_main(-1)
    , m_fdMCU_main2(-1)
    , m_fdMCU_sub(-1)
    , m_userCnt_main(0)
    , m_userCnt_main2(0)
    , m_userCnt_sub(0)
{
    DRV_DBG("LensDrv()\n");
}

/*******************************************************************************
*
********************************************************************************/
LensDrv::~LensDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::init(unsigned int a_u4CurrSensorDev )
{
    unsigned int a_u4CurrLensIdx;

    if(a_u4CurrSensorDev==MCU_DEV_MAIN)
    {
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_main;

        DRV_DBG("main lens init() [m_userCnt]%d  +\n", m_userCnt_main);
        DRV_DBG("[main Lens Driver]%s\n", MCUDrv::m_LensInitFunc_main[a_u4CurrLensIdx].LensDrvName);

        Mutex::Autolock lock(mLock);

        if (m_userCnt_main == 0)
        {
            if (m_fdMCU_main == -1)
            {
                m_fdMCU_main = open("/dev/MAINAF", O_RDWR);

                if (m_fdMCU_main < 0)
                {
                    if (a_u4CurrLensIdx == 0)
                    {    // no error log for dummy lens
                        return MCUDrv::MCU_NO_ERROR;
                    }
                    else
                    {
                        DRV_ERR("main Lens error opening %s", strerror(errno));
                        return MCUDrv::MCU_INVALID_DRIVER;
                    }
                }

                mcuMotorName motorName;
                memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc_main[a_u4CurrLensIdx].LensDrvName, 32);

                int err = ioctl(m_fdMCU_main,mcuIOC_S_SETDRVNAME,&motorName);
                if (err <= 0)
                {
                    DRV_ERR("[mcuIOC_S_SETDRVNAME] please check kernel driver");
                    close(m_fdMCU_main);
                    m_fdMCU_main = -1;
                    return MCUDrv::MCU_NO_ERROR;
                }
            }
        }
        m_userCnt_main++;
        DRV_DBG("main lens init() [m_userCnt]%d [fdMCU_main]%d - \n", m_userCnt_main,m_fdMCU_main);
    }
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)
    {
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_main2;

        DRV_DBG("main2 lens init() [m_userCnt]%d  +\n", m_userCnt_main2);
        DRV_DBG("[main2 Lens Driver]%s\n", MCUDrv::m_LensInitFunc_main2[a_u4CurrLensIdx].LensDrvName);

        Mutex::Autolock lock(mLock);

        if (m_userCnt_main2 == 0)
        {
            if (m_fdMCU_main2 == -1)
            {
                m_fdMCU_main2 = open("/dev/MAIN2AF", O_RDWR);

                if (m_fdMCU_main2 < 0)
                {
                    if (a_u4CurrLensIdx == 0)
                    {    // no error log for dummy lens
                        return MCUDrv::MCU_NO_ERROR;
                    }
                    else
                    {
                        DRV_ERR("main2 Lens error opening %s", strerror(errno));
                        return MCUDrv::MCU_INVALID_DRIVER;
                    }
                }

                mcuMotorName motorName;
                memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc_main2[a_u4CurrLensIdx].LensDrvName, 32);
                DRV_DBG("Select Driver Name : %s\n", motorName.uMotorName);

                int err = ioctl(m_fdMCU_main2,mcuIOC_S_SETDRVNAME,&motorName);
                if (err <= 0)
                {
                    DRV_ERR("[mcuIOC_S_SETDRVNAME] please check kernel driver");
                    close(m_fdMCU_main2);
                    m_fdMCU_main2 = -1;
                    return MCUDrv::MCU_NO_ERROR;
                }
            }
        }
        m_userCnt_main2++;
        DRV_DBG("main2 lens init() [m_userCnt]%d [fdMCU_main2]%d - \n", m_userCnt_main2,m_fdMCU_main2);
    }
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)
    {
        a_u4CurrLensIdx=MCUDrv::m_u4CurrLensIdx_sub;

        DRV_DBG("sub lens init() [m_userCnt]%d  +\n", m_userCnt_sub);
        DRV_DBG("[sub Lens Driver]%s\n", MCUDrv::m_LensInitFunc_sub[a_u4CurrLensIdx].LensDrvName);

        Mutex::Autolock lock(mLock);

        if (m_userCnt_sub == 0)
        {
            if (m_fdMCU_sub == -1)
            {
                m_fdMCU_sub = open("/dev/SUBAF", O_RDWR);

                if (m_fdMCU_sub < 0)
                {
                    if (a_u4CurrLensIdx == 0)
                    {    // no error log for dummy lens
                        return MCUDrv::MCU_NO_ERROR;
                    }
                    else
                    {
                        DRV_ERR("Sub Lens error opening %s", strerror(errno));
                        return MCUDrv::MCU_INVALID_DRIVER;
                    }
                }

                mcuMotorName motorName;
                memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc_sub[a_u4CurrLensIdx].LensDrvName, 32);
                DRV_DBG("Select Driver Name : %s\n", motorName.uMotorName);

                int err = ioctl(m_fdMCU_sub,mcuIOC_S_SETDRVNAME,&motorName);
                if (err <= 0)
                {
                    DRV_ERR("[mcuIOC_S_SETDRVNAME] please check kernel driver");
                    close(m_fdMCU_sub);
                    m_fdMCU_sub = -1;
                    return MCUDrv::MCU_NO_ERROR;
                }
            }
        }
        m_userCnt_sub++;
        DRV_DBG("sub lens init() [m_userCnt]%d [fdMCU_sub]%d - \n", m_userCnt_sub,m_fdMCU_sub);
    }
    else
        return MCUDrv::MCU_INVALID_DRIVER;

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::uninit(unsigned int a_u4CurrSensorDev )
{
    if(a_u4CurrSensorDev==MCU_DEV_MAIN)
    {
        DRV_DBG("main lens uninit() [m_userCnt]%d [fdMCU_main]%d + \n", m_userCnt_main,m_fdMCU_main);
        Mutex::Autolock lock(mLock);
        if (m_userCnt_main == 1)
        {
            if (m_fdMCU_main > 0) close(m_fdMCU_main);
            m_fdMCU_main = -1;
        }
        m_userCnt_main--;

        if (m_userCnt_main < 0)   {m_userCnt_main = 0;}

        DRV_DBG("main lens uninit() [fdMCU_main] %d  - \n", m_fdMCU_main);
    }
    else if(a_u4CurrSensorDev==MCU_DEV_MAIN_2)
    {
        DRV_DBG("main2 lens uninit() [m_userCnt]%d [fdMCU_main2]%d + \n", m_userCnt_main2, m_fdMCU_main2);
    Mutex::Autolock lock(mLock);
        if (m_userCnt_main2 == 1)
        {
            if (m_fdMCU_main2 > 0) close(m_fdMCU_main2);
            m_fdMCU_main2 = -1;
        }
        m_userCnt_main2--;

        if (m_userCnt_main2 < 0)   {m_userCnt_main2 = 0;}

        DRV_DBG("main2 lens uninit() [fdMCU_main2]%d - \n", m_fdMCU_main2);
        }
    else if(a_u4CurrSensorDev==MCU_DEV_SUB)
    {
        DRV_DBG("sub lens uninit() [m_userCnt]%d [fdMCU_sub]%d + \n", m_userCnt_sub, m_fdMCU_sub);
        Mutex::Autolock lock(mLock);
        if (m_userCnt_sub == 1)
        {
            if (m_fdMCU_sub > 0) close(m_fdMCU_sub);
            m_fdMCU_sub = -1;
    }
        m_userCnt_sub--;

        if (m_userCnt_sub < 0)   {m_userCnt_sub = 0;}

        DRV_DBG("sub lens uninit() [fdMCU_sub]%d - \n", m_fdMCU_sub);
    }
    else
        return MCUDrv::MCU_INVALID_DRIVER;

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::moveMCU(int a_i4FocusPos,unsigned int a_u4CurrSensorDev )
{
    //DRV_DBG("moveMCU() - pos = %d \n", a_i4FocusPos);
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
        return MCUDrv::MCU_INVALID_DRIVER;


    if (a_fdMCU < 0) {

        if (a_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {
            DRV_ERR("[moveMCU] invalid m_fdMCU =%d\n", a_fdMCU);
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU,mcuIOC_T_MOVETO,(unsigned long)a_i4FocusPos);
    if (err < 0) {
        DRV_ERR("[moveMCU] ioctl - mcuIOC_T_MOVETO, error %s",  strerror(errno));
        return err;
    }

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::getMCUInfo(mcuMotorInfo *a_pMotorInfo, unsigned int a_u4CurrSensorDev )
{
    //DRV_DBG("getMCUInfo() - E \n");
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
        return MCUDrv::MCU_INVALID_DRIVER;

    mcuMotorInfo motorInfo;
    memset(&motorInfo, 0, sizeof(mcuMotorInfo));

    if (a_fdMCU < 0) {

        if (a_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            a_pMotorInfo->bIsMotorOpen = 0;
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {
            DRV_ERR("[getMCUInfo] invalid m_fdMCU =%d\n", a_fdMCU);
            a_pMotorInfo->bIsMotorOpen = 0;
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU,mcuIOC_G_MOTORINFO, &motorInfo);
    if (err < 0) {
        DRV_ERR("[getMCUInfo] ioctl - mcuIOC_G_MOTORINFO, error %s",  strerror(errno));
        return err;
    }

    a_pMotorInfo->bIsMotorOpen = 1;
    a_pMotorInfo->bIsMotorMoving = motorInfo.bIsMotorMoving;
    a_pMotorInfo->u4CurrentPosition = motorInfo.u4CurrentPosition;
    a_pMotorInfo->u4MacroPosition = motorInfo.u4MacroPosition;
    a_pMotorInfo->u4InfPosition = motorInfo.u4InfPosition;
    a_pMotorInfo->bIsSupportSR = motorInfo.bIsSupportSR;

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::setMCUInfPos(int a_i4FocusPos,unsigned int a_u4CurrSensorDev )
{
    DRV_DBG("setMCUInfPos() - pos = %d \n", a_i4FocusPos );
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
        return MCUDrv::MCU_INVALID_DRIVER;

    if (a_fdMCU < 0) {

        if (a_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {
            DRV_ERR("[setMCUInfPos] invalid m_fdMCU =%d\n", a_fdMCU);
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU,mcuIOC_T_SETINFPOS,(unsigned long)a_i4FocusPos);
    if (err  < 0) {
        DRV_ERR("[setMCUInfPos] ioctl - mcuIOC_T_SETINFPOS, error %s",  strerror(errno));
        return err;
    }


    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::setMCUMacroPos(int a_i4FocusPos,unsigned int a_u4CurrSensorDev )
{
    DRV_DBG("setMCUMacroPos() - pos = %d \n", a_i4FocusPos);
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
        return MCUDrv::MCU_INVALID_DRIVER;

    if (a_fdMCU < 0) {
        if (a_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {
            DRV_ERR("[setMCUMacroPos] invalid m_fdMCU =%d\n", a_fdMCU);
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU,mcuIOC_T_SETMACROPOS,(unsigned long)a_i4FocusPos);
    if (err < 0) {
        DRV_ERR("[setMCUMacroPos] ioctl - mcuIOC_T_SETMACROPOS, error %s",  strerror(errno));
        return err;
    }

    return MCUDrv::MCU_NO_ERROR;
}

int
LensDrv::setMCUParam(int ois_param,unsigned int a_u4CurrSensorDev )
{
    int err, a_fdMCU,a_u4CurrLensIdx;
    DRV_DBG("setMCUParam() - bOIS_disable = %d \n",   (ois_param&1));
    DRV_DBG("setMCUParam() - bOIS_movieMode = %d \n", (ois_param&2));
    //DRV_DBG("setMCUParam() - u4ois_freq = %d \n",     ois_param->u4ois_freq);
    //DRV_DBG("setMCUParam() - u4ois_setting1 = %d \n", ois_param->u4ois_setting1);
    //DRV_DBG("setMCUParam() - u4ois_setting2 = %d \n", ois_param->u4ois_setting2);
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
        return MCUDrv::MCU_INVALID_DRIVER;

    if (a_fdMCU < 0) {
        if (a_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {
            DRV_ERR("[setMCUMacroPos] invalid m_fdMCU =%d\n", a_fdMCU);
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU,mcuIOC_T_SETPARA,ois_param);
    if (err < 0) {
        DRV_ERR("[setMCUMacroPos] ioctl - mcuIOC_T_SETPARA, error %s",  strerror(errno));
        return err;
    }

    return MCUDrv::MCU_NO_ERROR;
}
/*******************************************************************************
* Send command buffer to MCU, command buffer size is 100 Byte
********************************************************************************/
int
LensDrv::sendMCUCmd( unsigned int a_u4CurrSensorDev, mcuCmd *a_pMcuCmd)
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

