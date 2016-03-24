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
#define LOG_TAG "isp_debug"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <mtkcam/drv/isp_drv.h>
#include "isp_mgr.h"
#include <mtkcam/featureio/tuning_mgr.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspDebug&
IspDebug::
getInstance()
{
    static  IspDebug singleton;
    return  singleton;
}

IspDebug::
IspDebug()
    : m_pIspDrv(MNULL)
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
{
}

MBOOL
IspDebug::
init()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.isp_debug.enable", value, "1");
    m_bDebugEnable = atoi(value);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return MTRUE;
    }

    m_pIspDrv = IspDrv::createInstance();

    if (!m_pIspDrv) {
        MY_ERR("IspDrv::createInstance() fail \n");
        return MFALSE;
    }

    m_pIspDrv->init("isp_debug");

    android_atomic_inc(&m_Users);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspDebug::
uninit()
{
    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return MTRUE;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        if (m_pIspDrv) {
            m_pIspDrv->uninit("isp_debug");
        }

        m_pIspDrv = MNULL;
    }
    else    // There are still some users.
    {
        MY_LOG("Still %d users \n", m_Users);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_DMA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.dma", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4DMAOption = atoi(value);

    // DMA status check: TG1
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, AA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, AA_EN));
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_DMA, AAO_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_DMA, AAO_EN));
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, HBIN_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, HBIN_EN));

    if (i4DMAOption & 1) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_BASE_ADDR, BASE_ADDR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_BASE_ADDR, BASE_ADDR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_OFST_ADDR, OFFSET_ADDR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_OFST_ADDR, OFFSET_ADDR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_XSIZE, XSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_XSIZE, XSIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_STRIDE, BUS_SIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_STRIDE, BUS_SIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_CON));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_CON2));
    }

    // DMA status check: TG2
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, AA_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, AA_D_EN));
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_DMA_D, AAO_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_DMA_D, AAO_D_EN));

    if (i4DMAOption & 2) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_D_BASE_ADDR, BASE_ADDR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_BASE_ADDR, BASE_ADDR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_D_OFST_ADDR, OFFSET_ADDR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_OFST_ADDR, OFFSET_ADDR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_D_XSIZE, XSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_XSIZE, XSIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_D_STRIDE, BUS_SIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_STRIDE, BUS_SIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_D_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_D_CON));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AAO_D_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_D_CON2));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_AWB()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.awb", value, "0"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4AWBOption = atoi(value);

    // AWB statistics: TG1
    if (i4AWBOption & 1) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_ORG, AWB_W_HORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_ORG, AWB_W_HORG));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_ORG, AWB_W_VORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_ORG, AWB_W_VORG));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_SIZE, AWB_W_HSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_SIZE, AWB_W_HSIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_SIZE, AWB_W_VSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_SIZE, AWB_W_VSIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_PIT, AWB_W_HPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_PIT, AWB_W_HPIT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_PIT, AWB_W_VPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_PIT, AWB_W_VPIT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_NUM, AWB_W_HNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_NUM, AWB_W_HNUM));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_WIN_NUM, AWB_W_VNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_NUM, AWB_W_VNUM));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_GAIN1_0, AWB_GAIN1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_GAIN1_0, AWB_GAIN1_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_GAIN1_0, AWB_GAIN1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_GAIN1_0, AWB_GAIN1_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_GAIN1_1, AWB_GAIN1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_GAIN1_1, AWB_GAIN1_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_LMT1_0, AWB_LMT1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LMT1_0, AWB_LMT1_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_LMT1_0, AWB_LMT1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LMT1_0, AWB_LMT1_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_LMT1_1, AWB_LMT1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LMT1_1, AWB_LMT1_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_LOW_THR, AWB_LOW_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LOW_THR, AWB_LOW_THR0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_LOW_THR, AWB_LOW_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LOW_THR, AWB_LOW_THR1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_LOW_THR, AWB_LOW_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LOW_THR, AWB_LOW_THR2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_HI_THR, AWB_HI_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_HI_THR, AWB_HI_THR0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_HI_THR, AWB_HI_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_HI_THR, AWB_HI_THR1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_HI_THR, AWB_HI_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_HI_THR, AWB_HI_THR2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_PIXEL_CNT0, AWB_PIXEL_CNT0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_PIXEL_CNT0, AWB_PIXEL_CNT0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_PIXEL_CNT1, AWB_PIXEL_CNT1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_PIXEL_CNT1, AWB_PIXEL_CNT1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_PIXEL_CNT2, AWB_PIXEL_CNT2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_PIXEL_CNT2, AWB_PIXEL_CNT2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_ERR_THR, AWB_ERR_THR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ERR_THR, AWB_ERR_THR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_ERR_THR, AWB_ERR_SFT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ERR_THR, AWB_ERR_SFT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_ROT, AWB_C) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ROT, AWB_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_ROT, AWB_S) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ROT, AWB_S));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L0_X, AWB_L0_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_X, AWB_L0_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L0_X, AWB_L0_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_X, AWB_L0_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L0_Y, AWB_L0_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_Y, AWB_L0_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L0_Y, AWB_L0_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_Y, AWB_L0_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L1_X, AWB_L1_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_X, AWB_L1_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L1_X, AWB_L1_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_X, AWB_L1_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L1_Y, AWB_L1_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_Y, AWB_L1_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L1_Y, AWB_L1_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_Y, AWB_L1_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L2_X, AWB_L2_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_X, AWB_L2_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L2_X, AWB_L2_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_X, AWB_L2_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L2_Y, AWB_L2_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_Y, AWB_L2_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L2_Y, AWB_L2_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_Y, AWB_L2_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L3_X, AWB_L3_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_X, AWB_L3_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L3_X, AWB_L3_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_X, AWB_L3_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L3_Y, AWB_L3_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_Y, AWB_L3_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L3_Y, AWB_L3_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_Y, AWB_L3_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L4_X, AWB_L4_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_X, AWB_L4_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L4_X, AWB_L4_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_X, AWB_L4_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L4_Y, AWB_L4_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_Y, AWB_L4_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L4_Y, AWB_L4_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_Y, AWB_L4_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L5_X, AWB_L5_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_X, AWB_L5_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L5_X, AWB_L5_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_X, AWB_L5_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L5_Y, AWB_L5_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_Y, AWB_L5_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L5_Y, AWB_L5_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_Y, AWB_L5_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L6_X, AWB_L6_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_X, AWB_L6_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L6_X, AWB_L6_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_X, AWB_L6_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L6_Y, AWB_L6_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_Y, AWB_L6_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L6_Y, AWB_L6_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_Y, AWB_L6_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L7_X, AWB_L7_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_X, AWB_L7_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L7_X, AWB_L7_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_X, AWB_L7_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L7_Y, AWB_L7_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_Y, AWB_L7_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L7_Y, AWB_L7_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_Y, AWB_L7_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L8_X, AWB_L8_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_X, AWB_L8_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L8_X, AWB_L8_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_X, AWB_L8_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L8_Y, AWB_L8_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_Y, AWB_L8_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L8_Y, AWB_L8_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_Y, AWB_L8_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L9_X, AWB_L9_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_X, AWB_L9_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L9_X, AWB_L9_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_X, AWB_L9_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L9_Y, AWB_L9_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_Y, AWB_L9_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_L9_Y, AWB_L9_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_Y, AWB_L9_Y_UP));
    }

    // AWB statistics: TG2
    if (i4AWBOption & 2) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_ORG, AWB_W_HORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_ORG, AWB_W_HORG));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_ORG, AWB_W_VORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_ORG, AWB_W_VORG));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_PIT, AWB_W_HPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_PIT, AWB_W_HPIT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_PIT, AWB_W_VPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_PIT, AWB_W_VPIT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_NUM, AWB_W_HNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_NUM, AWB_W_HNUM));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_NUM, AWB_W_VNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_NUM, AWB_W_VNUM));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_GAIN1_0, AWB_GAIN1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_GAIN1_0, AWB_GAIN1_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_GAIN1_0, AWB_GAIN1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_GAIN1_0, AWB_GAIN1_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_GAIN1_1, AWB_GAIN1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_GAIN1_1, AWB_GAIN1_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_LMT1_0, AWB_LMT1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LMT1_0, AWB_LMT1_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_LMT1_0, AWB_LMT1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LMT1_0, AWB_LMT1_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_LMT1_1, AWB_LMT1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LMT1_1, AWB_LMT1_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_LOW_THR, AWB_LOW_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LOW_THR, AWB_LOW_THR0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_LOW_THR, AWB_LOW_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LOW_THR, AWB_LOW_THR1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_LOW_THR, AWB_LOW_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LOW_THR, AWB_LOW_THR2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_HI_THR, AWB_HI_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_HI_THR, AWB_HI_THR0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_HI_THR, AWB_HI_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_HI_THR, AWB_HI_THR1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_HI_THR, AWB_HI_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_HI_THR, AWB_HI_THR2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_PIXEL_CNT0, AWB_PIXEL_CNT0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_PIXEL_CNT0, AWB_PIXEL_CNT0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_PIXEL_CNT1, AWB_PIXEL_CNT1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_PIXEL_CNT1, AWB_PIXEL_CNT1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_PIXEL_CNT2, AWB_PIXEL_CNT2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_PIXEL_CNT2, AWB_PIXEL_CNT2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_ERR_THR, AWB_ERR_THR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ERR_THR, AWB_ERR_THR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_ERR_THR, AWB_ERR_SFT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ERR_THR, AWB_ERR_SFT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_ROT, AWB_C) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ROT, AWB_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_ROT, AWB_S) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ROT, AWB_S));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L0_X, AWB_L0_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_X, AWB_L0_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L0_X, AWB_L0_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_X, AWB_L0_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L0_Y, AWB_L0_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_Y, AWB_L0_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L0_Y, AWB_L0_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_Y, AWB_L0_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L1_X, AWB_L1_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_X, AWB_L1_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L1_X, AWB_L1_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_X, AWB_L1_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L1_Y, AWB_L1_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_Y, AWB_L1_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L1_Y, AWB_L1_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_Y, AWB_L1_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L2_X, AWB_L2_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_X, AWB_L2_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L2_X, AWB_L2_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_X, AWB_L2_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L2_Y, AWB_L2_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_Y, AWB_L2_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L2_Y, AWB_L2_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_Y, AWB_L2_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L3_X, AWB_L3_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_X, AWB_L3_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L3_X, AWB_L3_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_X, AWB_L3_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L3_Y, AWB_L3_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_Y, AWB_L3_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L3_Y, AWB_L3_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_Y, AWB_L3_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L4_X, AWB_L4_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_X, AWB_L4_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L4_X, AWB_L4_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_X, AWB_L4_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L4_Y, AWB_L4_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_Y, AWB_L4_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L4_Y, AWB_L4_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_Y, AWB_L4_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L5_X, AWB_L5_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_X, AWB_L5_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L5_X, AWB_L5_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_X, AWB_L5_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L5_Y, AWB_L5_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_Y, AWB_L5_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L5_Y, AWB_L5_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_Y, AWB_L5_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L6_X, AWB_L6_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_X, AWB_L6_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L6_X, AWB_L6_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_X, AWB_L6_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L6_Y, AWB_L6_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_Y, AWB_L6_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L6_Y, AWB_L6_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_Y, AWB_L6_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L7_X, AWB_L7_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_X, AWB_L7_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L7_X, AWB_L7_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_X, AWB_L7_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L7_Y, AWB_L7_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_Y, AWB_L7_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L7_Y, AWB_L7_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_Y, AWB_L7_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L8_X, AWB_L8_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_X, AWB_L8_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L8_X, AWB_L8_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_X, AWB_L8_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L8_Y, AWB_L8_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_Y, AWB_L8_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L8_Y, AWB_L8_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_Y, AWB_L8_Y_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L9_X, AWB_L9_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_X, AWB_L9_X_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L9_X, AWB_L9_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_X, AWB_L9_X_UP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L9_Y, AWB_L9_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_Y, AWB_L9_Y_LOW));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AWB_D_L9_Y, AWB_L9_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_Y, AWB_L9_Y_UP));
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_OBC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.obc", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4OBCOption = atoi(value);

    // OBC: TG1
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, OB_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, OB_EN));

    if (i4OBCOption & 1) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_OFFST0, OBC_OFST_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST0, OBC_OFST_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_OFFST1, OBC_OFST_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST1, OBC_OFST_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_OFFST2, OBC_OFST_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST2, OBC_OFST_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_OFFST3, OBC_OFST_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST3, OBC_OFST_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_GAIN0, OBC_GAIN_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN0, OBC_GAIN_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_GAIN1, OBC_GAIN_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN1, OBC_GAIN_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_GAIN2, OBC_GAIN_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN2, OBC_GAIN_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_GAIN3, OBC_GAIN_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN3, OBC_GAIN_R));
    }

    // OBC: TG2
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, OB_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, OB_D_EN));

    if (i4OBCOption & 2) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST0, OBC_D_OFST_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST0, OBC_D_OFST_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST1, OBC_D_OFST_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST1, OBC_D_OFST_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST2, OBC_D_OFST_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST2, OBC_D_OFST_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST3, OBC_D_OFST_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST3, OBC_D_OFST_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN0, OBC_D_GAIN_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN0, OBC_D_GAIN_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN1, OBC_D_GAIN_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN1, OBC_D_GAIN_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN2, OBC_D_GAIN_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN2, OBC_D_GAIN_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN3, OBC_D_GAIN_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN3, OBC_D_GAIN_R));
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_BNR()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.bnr", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4BNROption = atoi(value);

    // BNR: TG1
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, BNR_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, BNR_EN));

    if (i4BNROption & 1) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_CON));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TH1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TH2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TH3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TH4) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH4));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_DTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_DTC));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_COR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_COR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TBLI1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TBLI1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TBLI2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TBLI2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TH1_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH1_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TH2_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH2_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_TH3_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH3_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_RMM1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_RMM2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_RMM_REVG_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_REVG_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_RMM_REVG_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_REVG_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_RMM_LEOS) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_LEOS));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_RMM_GCNT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_GCNT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR1_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_CON));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR1_CT_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_CT_CON));
    }

    // BNR: TG2
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, BNR_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, BNR_D_EN));

    if (i4BNROption & 2) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_CON));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TH1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TH2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TH3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TH4) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH4));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_DTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_DTC));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_COR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_COR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TBLI1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TBLI1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TBLI2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TBLI2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TH1_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH1_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TH2_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH2_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_TH3_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH3_C));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_RMM1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_RMM2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_REVG_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_REVG_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_REVG_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_REVG_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_LEOS) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_LEOS));
        MY_LOG_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_GCNT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_GCNT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR1_D_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_D_CON));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR1_D_CT_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_D_CT_CON));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_LSC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.lsc", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4LSCOption = atoi(value);

    // LSC: TG1
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, LSC_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, LSC_EN));

    if (i4LSCOption & 1) {
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_BA     = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_BASE_ADDR));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_XSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_XSIZE));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_YSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_YSIZE));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_STRIDE = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_STRIDE));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_CON    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_CON2   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON2));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_CTL1    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL1));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_CTL2    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL2));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_CTL3    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL3));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_LBLOCK  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_LBLOCK));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_RATIO   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_RATIO));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_GAIN_TH = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_GAIN_TH));
    }

    // LSC: TG2
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, LSC_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, LSC_D_EN));

    if (i4LSCOption & 2) {
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_D_BA     = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_BASE_ADDR));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_D_XSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_XSIZE));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_D_YSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_YSIZE));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_D_STRIDE = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_STRIDE));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_D_CON    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_CON));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSCI_D_CON2   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_CON2));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_D_CTL1    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_CTL1));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_D_CTL2    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_CTL2));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_D_CTL3    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_CTL3));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_D_LBLOCK  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_LBLOCK));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_D_RATIO   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_RATIO));
        MY_LOG_IF(m_bDebugEnable,"CAM_LSC_D_GAIN_TH = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_GAIN_TH));
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_RPG()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.rpg", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4RPGOption = atoi(value);

    // RPG: TG1
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, RPG_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, RPG_EN));

    if (i4RPGOption & 1) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_SATU_1, RPG_SATU_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_1, RPG_SATU_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_SATU_1, RPG_SATU_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_1, RPG_SATU_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_SATU_2, RPG_SATU_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_2, RPG_SATU_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_SATU_2, RPG_SATU_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_2, RPG_SATU_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_GAIN_1, RPG_GAIN_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_1, RPG_GAIN_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_GAIN_1, RPG_GAIN_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_1, RPG_GAIN_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_GAIN_2, RPG_GAIN_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_2, RPG_GAIN_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_GAIN_2, RPG_GAIN_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_2, RPG_GAIN_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_OFST_1, RPG_OFST_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_1, RPG_OFST_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_OFST_1, RPG_OFST_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_1, RPG_OFST_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_OFST_2, RPG_OFST_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_2, RPG_OFST_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_OFST_2, RPG_OFST_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_2, RPG_OFST_R));
    }

    // RPG: TG2
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, RPG_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, RPG_D_EN));

    if (i4RPGOption & 2) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_1, RPG_SATU_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_1, RPG_SATU_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_1, RPG_SATU_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_1, RPG_SATU_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_2, RPG_SATU_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_2, RPG_SATU_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_2, RPG_SATU_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_2, RPG_SATU_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_1, RPG_GAIN_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_1, RPG_GAIN_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_1, RPG_GAIN_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_1, RPG_GAIN_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_2, RPG_GAIN_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_2, RPG_GAIN_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_2, RPG_GAIN_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_2, RPG_GAIN_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_1, RPG_OFST_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_1, RPG_OFST_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_1, RPG_OFST_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_1, RPG_OFST_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_2, RPG_OFST_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_2, RPG_OFST_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_2, RPG_OFST_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_2, RPG_OFST_R));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_PGN()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.pgn", value, "1"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, PGN_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, PGN_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_SATU_1, PGN_SATU_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_1, PGN_SATU_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_SATU_1, PGN_SATU_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_1, PGN_SATU_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_SATU_2, PGN_SATU_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_2, PGN_SATU_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_SATU_2, PGN_SATU_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_2, PGN_SATU_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_GAIN_1, PGN_GAIN_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_1, PGN_GAIN_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_GAIN_1, PGN_GAIN_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_1, PGN_GAIN_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_GAIN_2, PGN_GAIN_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_2, PGN_GAIN_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_GAIN_2, PGN_GAIN_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_2, PGN_GAIN_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_OFST_1, PGN_OFST_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_1, PGN_OFST_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_OFST_1, PGN_OFST_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_1, PGN_OFST_GB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_OFST_2, PGN_OFST_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_2, PGN_OFST_GR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PGN_OFST_2, PGN_OFST_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_2, PGN_OFST_R));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_CFA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.cfa", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, CFA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, CFA_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_BYP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_BYP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_ED_FLAT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ED_FLAT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_ED_NYQ) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ED_NYQ));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_ED_STEP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ED_STEP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_RGB_HF) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_RGB_HF));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_DOT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_DOT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F1_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F1_ACT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F2_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F2_ACT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F3_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F3_ACT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F4_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F4_ACT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F1_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F1_L));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F2_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F2_L));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F3_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F3_L));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_F4_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F4_L));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_HF_RB) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_RB));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_HF_GAIN) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_GAIN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_HF_COMP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_COMP));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_HF_CORIN_TH) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_CORIN_TH));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_ACT_LUT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ACT_LUT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_SPARE) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_SPARE));
        MY_LOG_IF(m_bDebugEnable,"(CAM_DM_O_BB) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_BB));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_CCM()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.ccm", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, G2G_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, G2G_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_1, G2G_CNV_00) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_1, G2G_CNV_00));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_1, G2G_CNV_01) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_1, G2G_CNV_01));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_2, G2G_CNV_02) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_2, G2G_CNV_02));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_3, G2G_CNV_10) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_3, G2G_CNV_10));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_3, G2G_CNV_11) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_3, G2G_CNV_11));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_4, G2G_CNV_12) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_4, G2G_CNV_12));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_5, G2G_CNV_20) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_5, G2G_CNV_20));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_5, G2G_CNV_21) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_5, G2G_CNV_21));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CNV_6, G2G_CNV_22) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_6, G2G_CNV_22));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2G_CTRL, G2G_ACC) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CTRL, G2G_ACC));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_GGM()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.ggm", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, GGM_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, GGM_EN));
    MY_LOG_IF(m_bDebugEnable,"(CAM_GGM_CTRL, GAMMA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_GGM_CTRL, GGM_EN));

    if (bDebugEnable) {
        for (MINT32 i = 0; i < 144; i++) {
            MY_LOG_IF(m_bDebugEnable,"(CAM_GGM_LUT_RB[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_GGM_LUT_RB[i]));
            MY_LOG_IF(m_bDebugEnable,"(CAM_GGM_LUT_G[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_GGM_LUT_G[i]));
        }
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_G2C()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.g2c", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, G2C_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, G2C_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_CONV_0A) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_0A));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_CONV_0B) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_0B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_CONV_1A) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_1A));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_CONV_1B) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_1B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_CONV_2A) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_2A));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_CONV_2B) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_2B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_SHADE_CON_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_CON_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_SHADE_CON_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_CON_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_SHADE_CON_3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_CON_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_SHADE_TAR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_TAR));
        MY_LOG_IF(m_bDebugEnable,"(CAM_G2C_SHADE_SP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_SP));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_NBC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.nbc", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, NBC_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, NBC_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CON1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CON2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CON3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_YAD1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_YAD1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_YAD2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_YAD2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_4LUT1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_4LUT1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_4LUT2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_4LUT2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_4LUT3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_4LUT3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_PTY) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_PTY));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_CAD) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CAD));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_PTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_PTC));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_LCE1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_LCE1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_LCE2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_LCE2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_HP1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_HP1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_HP2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_HP2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_HP3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_HP3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_ACTY) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_ACTY));
        MY_LOG_IF(m_bDebugEnable,"(CAM_ANR_ACTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_ACTC));
        MY_LOG_IF(m_bDebugEnable,"(CAM_CCR_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_CON));
        MY_LOG_IF(m_bDebugEnable,"(CAM_CCR_YLUT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_YLUT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_CCR_UVLUT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_UVLUT));
        MY_LOG_IF(m_bDebugEnable,"(CAM_CCR_YLUT2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_YLUT2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_CCR_SAT_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_SAT_CTRL));
        MY_LOG_IF(m_bDebugEnable,"(CAM_CCR_UVLUT_SP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_UVLUT_SP));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_PCA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.pca", value, "0"); // 0: disable, 1: 180BIN, 2: 360BIN
    MINT32 i4PCAOption = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, PCA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, PCA_EN));

    if (i4PCAOption & 1) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_PCA_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PCA_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON2));

        for (MINT32 i = 0; i < 180; i++) {
            MY_LOG_IF(m_bDebugEnable,"(CAM_PCA_TBL[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_TBL[i]));
        }
    }

    if (i4PCAOption & 2) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_PCA_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_PCA_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON2));

        for (MINT32 i = 0; i < 360; i++) {
            MY_LOG_IF(m_bDebugEnable,"(CAM_PCA_TBL[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_TBL[i]));
        }
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_SEEE()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.seee", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SEEE_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SEEE_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SRK_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SRK_CTRL));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_CLIP_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_CLIP_CTRL));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_FLT_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_FLT_CTRL_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_FLT_CTRL_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_FLT_CTRL_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_01) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_01));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_02) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_02));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_03) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_03));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_04) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_04));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_05) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_05));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_06) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_06));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_EDTR_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_EDTR_CTRL));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_07) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_07));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_08) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_08));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_09) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_09));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_10) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_10));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_11) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_11));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_OUT_EDGE_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_OUT_EDGE_CTRL));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SE_Y_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_Y_CTRL));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SE_EDGE_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_EDGE_CTRL_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SE_EDGE_CTRL_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_EDGE_CTRL_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SE_EDGE_CTRL_3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_EDGE_CTRL_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SE_SPECL_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_SPECL_CTRL));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SE_CORE_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_CORE_CTRL_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_SEEE_SE_CORE_CTRL_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_CORE_CTRL_2));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_AE()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.ae", value, "0"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4AEOption = atoi(value);

    // AE statistics: TG1
    if (i4AEOption & 1) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST0_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST0_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST1_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST1_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST2_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST2_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST3_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST3_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_GAIN2_0, AE_GAIN2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_GAIN2_0, AE_GAIN2_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_GAIN2_0, AE_GAIN2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_GAIN2_0, AE_GAIN2_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_GAIN2_1, AE_GAIN2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_GAIN2_1, AE_GAIN2_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_LMT2_0, AE_LMT2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_LMT2_0, AE_LMT2_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_LMT2_0, AE_LMT2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_LMT2_0, AE_LMT2_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_LMT2_1, AE_LMT2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_LMT2_1, AE_LMT2_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_0, AE_RC_CNV00) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_0, AE_RC_CNV00));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_0, AE_RC_CNV01) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_0, AE_RC_CNV01));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_1, AE_RC_CNV02) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_1, AE_RC_CNV02));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_1, AE_RC_CNV10) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_1, AE_RC_CNV10));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_2, AE_RC_CNV11) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_2, AE_RC_CNV11));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_2, AE_RC_CNV12) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_2, AE_RC_CNV12));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_3, AE_RC_CNV20) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_3, AE_RC_CNV20));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_3, AE_RC_CNV21) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_3, AE_RC_CNV21));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_4, AE_RC_CNV22) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_4, AE_RC_CNV22));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_4, AE_RC_ACC) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_4, AE_RC_ACC));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR4) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR4));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_1, Y_GMR5) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_1, Y_GMR5));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_X_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_X_LOW_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_X_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_X_HI_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_Y_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_Y_LOW_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_Y_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_Y_HI_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_X_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_X_LOW_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_X_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_X_HI_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_Y_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_Y_LOW_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_Y_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_Y_HI_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_X_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_X_LOW_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_X_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_X_HI_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_Y_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_Y_LOW_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_Y_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_Y_HI_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_X_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_X_LOW_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_X_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_X_HI_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_Y_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_Y_LOW_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_Y_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_Y_HI_3));
     }

    // AWB statistics: TG2
    if (i4AEOption & 2) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST0_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST0_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST1_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST1_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST2_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST2_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST3_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST3_EN));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_GAIN2_0, AE_GAIN2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_GAIN2_0, AE_GAIN2_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_GAIN2_0, AE_GAIN2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_GAIN2_0, AE_GAIN2_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_GAIN2_1, AE_GAIN2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_GAIN2_1, AE_GAIN2_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_LMT2_0, AE_LMT2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_LMT2_0, AE_LMT2_R));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_LMT2_0, AE_LMT2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_LMT2_0, AE_LMT2_G));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_LMT2_1, AE_LMT2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_LMT2_1, AE_LMT2_B));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_0, AE_RC_CNV00) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_0, AE_RC_CNV00));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_0, AE_RC_CNV01) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_0, AE_RC_CNV01));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_1, AE_RC_CNV02) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_1, AE_RC_CNV02));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_1, AE_RC_CNV10) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_1, AE_RC_CNV10));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_2, AE_RC_CNV11) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_2, AE_RC_CNV11));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_2, AE_RC_CNV12) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_2, AE_RC_CNV12));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_3, AE_RC_CNV20) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_3, AE_RC_CNV20));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_3, AE_RC_CNV21) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_3, AE_RC_CNV21));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_4, AE_RC_CNV22) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_4, AE_RC_CNV22));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_4, AE_RC_ACC) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_4, AE_RC_ACC));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR4) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR4));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_1, Y_GMR5) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_1, Y_GMR5));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_X_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_X_LOW_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_X_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_X_HI_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_Y_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_Y_LOW_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_Y_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_Y_HI_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_X_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_X_LOW_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_X_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_X_HI_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_Y_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_Y_LOW_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_Y_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_Y_HI_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_X_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_X_LOW_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_X_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_X_HI_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_Y_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_Y_LOW_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_Y_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_Y_HI_2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_X_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_X_LOW_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_X_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_X_HI_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_Y_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_Y_LOW_3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_Y_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_Y_HI_3));
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_NR3D()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.nr3d", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, NR3D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, NR3D_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_BLEND) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_BLEND));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_CPX) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_CPX));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_Y_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_Y_CON1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_Y_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_Y_CON2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_Y_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_Y_CON3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_U_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_U_CON1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_U_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_U_CON2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_U_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_U_CON3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_V_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_V_CON1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_V_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_V_CON2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_NR3D_LMT_V_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_V_CON3));
    }

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_MFB()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.mfb", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, MFB_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, MFB_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_MFB_LL_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON2));
        MY_LOG_IF(m_bDebugEnable,"(CAM_MFB_LL_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON3));
        MY_LOG_IF(m_bDebugEnable,"(CAM_MFB_LL_CON4) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON4));
        MY_LOG_IF(m_bDebugEnable,"(CAM_MFB_LL_CON5) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON5));
        MY_LOG_IF(m_bDebugEnable,"(CAM_MFB_LL_CON6) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON6));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_MIX3()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.debug.mix3", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, MIX3_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, MIX3_EN));

    if (bDebugEnable) {
        MY_LOG_IF(m_bDebugEnable,"(CAM_MIX3_CTRL_0) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MIX3_CTRL_0));
        MY_LOG_IF(m_bDebugEnable,"(CAM_MIX3_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MIX3_CTRL_1));
        MY_LOG_IF(m_bDebugEnable,"(CAM_MIX3_SPARE) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MIX3_SPARE));
    }

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspDebug::
dumpIspDebugMessage()
{
    MY_LOG_IF("%s()\n", __FUNCTION__);

    dumpIspDebugMessage_DMA();
    dumpIspDebugMessage_AWB();
    dumpIspDebugMessage_OBC();
    dumpIspDebugMessage_LSC();
    dumpIspDebugMessage_BNR();
    dumpIspDebugMessage_RPG();
    dumpIspDebugMessage_PGN();
    dumpIspDebugMessage_CFA();
    dumpIspDebugMessage_CCM();
    dumpIspDebugMessage_GGM();
    dumpIspDebugMessage_G2C();
    dumpIspDebugMessage_NBC();
    dumpIspDebugMessage_PCA();
    dumpIspDebugMessage_SEEE();
    dumpIspDebugMessage_NR3D();
    dumpIspDebugMessage_AE();
    dumpIspDebugMessage_MFB();
    dumpIspDebugMessage_MIX3();

    // LSC
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, LSC_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, LSC_EN));
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, LSC_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, LSC_D_EN));
    #if 0
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_BA)    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_BASE_ADDR));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_XSIZE) = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_XSIZE));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_YSIZE) = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_YSIZE));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_STRIDE)= 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_STRIDE));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON2)  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON2));

    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL1)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL1));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL2)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL2));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL3)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL3));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_LBLOCK) = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_LBLOCK));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_RATIO)  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_RATIO));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_GAIN_TH)= 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_GAIN_TH));
    #endif

    // SL2
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SL2_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SL2_EN));
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SL2B_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SL2B_EN));
    MY_LOG_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SL2C_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SL2C_EN));
    #if 0
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_CEN) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_CEN));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX0_RR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX0_RR));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX1_RR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX1_RR));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX2_RR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX2_RR));
    #endif

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspDebug::
readLsciAddr(MUINT32 u4TgInfo)
{
    MUINT32 u4Addr;
    switch (u4TgInfo)
    {
    case ESensorTG_1:
        u4Addr = ISP_READ_REG_NOPROTECT(m_pIspDrv, CAM_LSCI_BASE_ADDR);
        break;
    case ESensorTG_2:
        u4Addr = ISP_READ_REG_NOPROTECT(m_pIspDrv, CAM_LSCI_D_BASE_ADDR);
        break;
    }
    return u4Addr;
}

