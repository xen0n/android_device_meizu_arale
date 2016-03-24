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
#define LOG_TAG "isp_mgr_ctl"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "isp_mgr.h"

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass1@TG1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CTL_EN_P1_T&
ISP_MGR_CTL_EN_P1_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CTL_EN_P1_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CTL_EN_P1_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CTL_EN_P1_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CTL_EN_P1_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CTL_EN_P1_T&
ISP_MGR_CTL_EN_P1_T::
get(ISP_NVRAM_CTL_EN_P1_T& rParam)
{
    GET_REG_INFO(CAM_CTL_EN_P1, en_p1);

    return  (*this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass1@TG2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CTL_EN_P1_D_T&
ISP_MGR_CTL_EN_P1_D_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CTL_EN_P1_D_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CTL_EN_P1_D_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CTL_EN_P1_D_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CTL_EN_P1_D_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CTL_EN_P1_D_T&
ISP_MGR_CTL_EN_P1_D_T::
get(ISP_NVRAM_CTL_EN_P1_D_T& rParam)
{
    GET_REG_INFO(CAM_CTL_EN_P1_D, en_p1_d);

    return  (*this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CTL_EN_P2_T&
ISP_MGR_CTL_EN_P2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CTL_EN_P2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CTL_EN_P2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CTL_EN_P2_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CTL_EN_P2_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CTL_EN_P2_T&
ISP_MGR_CTL_EN_P2_T::
get(ISP_NVRAM_CTL_EN_P2_T& rParam)
{
    GET_REG_INFO(CAM_CTL_EN_P2, en_p2);

    return  (*this);
}



}
