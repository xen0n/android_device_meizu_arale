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
#ifndef _ISP_MGR_AWB_STAT_H_
#define _ISP_MGR_AWB_STAT_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AWB statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AWB_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AWB_STAT_CONFIG    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MUINT32 m_u4StartAddr;
    MINT32 m_i4SensorIndex;
    MINT32 m_i4IspAEGain;
    AWB_STAT_CONFIG_T m_rAWBStatConfig;

    // TG1
    enum
    {
        ERegInfo_CAM_AWB_WIN_ORG,
        ERegInfo_CAM_AWB_WIN_SIZE,
        ERegInfo_CAM_AWB_WIN_PIT,
        ERegInfo_CAM_AWB_WIN_NUM,
        ERegInfo_CAM_AWB_GAIN1_0,
        ERegInfo_CAM_AWB_GAIN1_1,
        ERegInfo_CAM_AWB_LMT1_0,
        ERegInfo_CAM_AWB_LMT1_1,
        ERegInfo_CAM_AWB_LOW_THR,
        ERegInfo_CAM_AWB_HI_THR,
        ERegInfo_CAM_AWB_PIXEL_CNT0,
        ERegInfo_CAM_AWB_PIXEL_CNT1,
        ERegInfo_CAM_AWB_PIXEL_CNT2,
        ERegInfo_CAM_AWB_ERR_THR,
        ERegInfo_CAM_AWB_ROT,
        ERegInfo_CAM_AWB_L0_X,
        ERegInfo_CAM_AWB_L0_Y,
        ERegInfo_CAM_AWB_L1_X,
        ERegInfo_CAM_AWB_L1_Y,
        ERegInfo_CAM_AWB_L2_X,
        ERegInfo_CAM_AWB_L2_Y,
        ERegInfo_CAM_AWB_L3_X,
        ERegInfo_CAM_AWB_L3_Y,
        ERegInfo_CAM_AWB_L4_X,
        ERegInfo_CAM_AWB_L4_Y,
        ERegInfo_CAM_AWB_L5_X,
        ERegInfo_CAM_AWB_L5_Y,
        ERegInfo_CAM_AWB_L6_X,
        ERegInfo_CAM_AWB_L6_Y,
        ERegInfo_CAM_AWB_L7_X,
        ERegInfo_CAM_AWB_L7_Y,
        ERegInfo_CAM_AWB_L8_X,
        ERegInfo_CAM_AWB_L8_Y,
        ERegInfo_CAM_AWB_L9_X,
        ERegInfo_CAM_AWB_L9_Y,
        ERegInfo_NUM
    };

    // TG2
    enum
    {
        ERegInfo_CAM_AWB_D_WIN_ORG,
        ERegInfo_CAM_AWB_D_WIN_SIZE,
        ERegInfo_CAM_AWB_D_WIN_PIT,
        ERegInfo_CAM_AWB_D_WIN_NUM,
        ERegInfo_CAM_AWB_D_GAIN1_0,
        ERegInfo_CAM_AWB_D_GAIN1_1,
        ERegInfo_CAM_AWB_D_LMT1_0,
        ERegInfo_CAM_AWB_D_LMT1_1,
        ERegInfo_CAM_AWB_D_LOW_THR,
        ERegInfo_CAM_AWB_D_HI_THR,
        ERegInfo_CAM_AWB_D_PIXEL_CNT0,
        ERegInfo_CAM_AWB_D_PIXEL_CNT1,
        ERegInfo_CAM_AWB_D_PIXEL_CNT2,
        ERegInfo_CAM_AWB_D_ERR_THR,
        ERegInfo_CAM_AWB_D_ROT,
        ERegInfo_CAM_AWB_D_L0_X,
        ERegInfo_CAM_AWB_D_L0_Y,
        ERegInfo_CAM_AWB_D_L1_X,
        ERegInfo_CAM_AWB_D_L1_Y,
        ERegInfo_CAM_AWB_D_L2_X,
        ERegInfo_CAM_AWB_D_L2_Y,
        ERegInfo_CAM_AWB_D_L3_X,
        ERegInfo_CAM_AWB_D_L3_Y,
        ERegInfo_CAM_AWB_D_L4_X,
        ERegInfo_CAM_AWB_D_L4_Y,
        ERegInfo_CAM_AWB_D_L5_X,
        ERegInfo_CAM_AWB_D_L5_Y,
        ERegInfo_CAM_AWB_D_L6_X,
        ERegInfo_CAM_AWB_D_L6_Y,
        ERegInfo_CAM_AWB_D_L7_X,
        ERegInfo_CAM_AWB_D_L7_Y,
        ERegInfo_CAM_AWB_D_L8_X,
        ERegInfo_CAM_AWB_D_L8_Y,
        ERegInfo_CAM_AWB_D_L9_X,
        ERegInfo_CAM_AWB_D_L9_Y
    };

    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_AWB_STAT_CONFIG(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_AWB_WIN_ORG) : REG_ADDR(CAM_AWB_D_WIN_ORG))
        , m_i4SensorIndex(0)
        , m_i4IspAEGain(512)
        , m_rAWBStatConfig()
    {
        // register info addr init
        if (eSensorTG == ESensorTG_1) {
            INIT_REG_INFO_ADDR(CAM_AWB_WIN_ORG);    // CAM+0x05B0
            INIT_REG_INFO_ADDR(CAM_AWB_WIN_SIZE);   // CAM+0x05B4
            INIT_REG_INFO_ADDR(CAM_AWB_WIN_PIT);    // CAM+0x05B8
            INIT_REG_INFO_ADDR(CAM_AWB_WIN_NUM);    // CAM+0x05BC
            INIT_REG_INFO_ADDR(CAM_AWB_GAIN1_0);    // CAM+0x05C0
            INIT_REG_INFO_ADDR(CAM_AWB_GAIN1_1);    // CAM+0x05C4
            INIT_REG_INFO_ADDR(CAM_AWB_LMT1_0);     // CAM+0x05C8
            INIT_REG_INFO_ADDR(CAM_AWB_LMT1_1);     // CAM+0x05CC
            INIT_REG_INFO_ADDR(CAM_AWB_LOW_THR);    // CAM+0x05D0
            INIT_REG_INFO_ADDR(CAM_AWB_HI_THR);     // CAM+0x05D4
            INIT_REG_INFO_ADDR(CAM_AWB_PIXEL_CNT0); // CAM+0x05D8
            INIT_REG_INFO_ADDR(CAM_AWB_PIXEL_CNT1); // CAM+0x05DC
            INIT_REG_INFO_ADDR(CAM_AWB_PIXEL_CNT2); // CAM+0x05E0
            INIT_REG_INFO_ADDR(CAM_AWB_ERR_THR);    // CAM+0x05E4
            INIT_REG_INFO_ADDR(CAM_AWB_ROT);        // CAM+0x05E8
            INIT_REG_INFO_ADDR(CAM_AWB_L0_X);       // CAM+0x05EC
            INIT_REG_INFO_ADDR(CAM_AWB_L0_Y);       // CAM+0x05F0
            INIT_REG_INFO_ADDR(CAM_AWB_L1_X);       // CAM+0x05F4
            INIT_REG_INFO_ADDR(CAM_AWB_L1_Y);       // CAM+0x05F8
            INIT_REG_INFO_ADDR(CAM_AWB_L2_X);       // CAM+0x05FC
            INIT_REG_INFO_ADDR(CAM_AWB_L2_Y);       // CAM+0x0600
            INIT_REG_INFO_ADDR(CAM_AWB_L3_X);       // CAM+0x0604
            INIT_REG_INFO_ADDR(CAM_AWB_L3_Y);       // CAM+0x0608
            INIT_REG_INFO_ADDR(CAM_AWB_L4_X);       // CAM+0x060C
            INIT_REG_INFO_ADDR(CAM_AWB_L4_Y);       // CAM+0x0610
            INIT_REG_INFO_ADDR(CAM_AWB_L5_X);       // CAM+0x0614
            INIT_REG_INFO_ADDR(CAM_AWB_L5_Y);       // CAM+0x0618
            INIT_REG_INFO_ADDR(CAM_AWB_L6_X);       // CAM+0x061C
            INIT_REG_INFO_ADDR(CAM_AWB_L6_Y);       // CAM+0x0620
            INIT_REG_INFO_ADDR(CAM_AWB_L7_X);       // CAM+0x0624
            INIT_REG_INFO_ADDR(CAM_AWB_L7_Y);       // CAM+0x0628
            INIT_REG_INFO_ADDR(CAM_AWB_L8_X);       // CAM+0x062C
            INIT_REG_INFO_ADDR(CAM_AWB_L8_Y);       // CAM+0x0630
            INIT_REG_INFO_ADDR(CAM_AWB_L9_X);       // CAM+0x0634
            INIT_REG_INFO_ADDR(CAM_AWB_L9_Y);       // CAM+0x0638
        }
        else {
            INIT_REG_INFO_ADDR(CAM_AWB_D_WIN_ORG);    // CAM+0x25B0
            INIT_REG_INFO_ADDR(CAM_AWB_D_WIN_SIZE);   // CAM+0x25B4
            INIT_REG_INFO_ADDR(CAM_AWB_D_WIN_PIT);    // CAM+0x25B8
            INIT_REG_INFO_ADDR(CAM_AWB_D_WIN_NUM);    // CAM+0x25BC
            INIT_REG_INFO_ADDR(CAM_AWB_D_GAIN1_0);    // CAM+0x25C0
            INIT_REG_INFO_ADDR(CAM_AWB_D_GAIN1_1);    // CAM+0x25C4
            INIT_REG_INFO_ADDR(CAM_AWB_D_LMT1_0);     // CAM+0x25C8
            INIT_REG_INFO_ADDR(CAM_AWB_D_LMT1_1);     // CAM+0x25CC
            INIT_REG_INFO_ADDR(CAM_AWB_D_LOW_THR);    // CAM+0x25D0
            INIT_REG_INFO_ADDR(CAM_AWB_D_HI_THR);     // CAM+0x25D4
            INIT_REG_INFO_ADDR(CAM_AWB_D_PIXEL_CNT0); // CAM+0x25D8
            INIT_REG_INFO_ADDR(CAM_AWB_D_PIXEL_CNT1); // CAM+0x25DC
            INIT_REG_INFO_ADDR(CAM_AWB_D_PIXEL_CNT2); // CAM+0x25E0
            INIT_REG_INFO_ADDR(CAM_AWB_D_ERR_THR);    // CAM+0x25E4
            INIT_REG_INFO_ADDR(CAM_AWB_D_ROT);        // CAM+0x25E8
            INIT_REG_INFO_ADDR(CAM_AWB_D_L0_X);       // CAM+0x25EC
            INIT_REG_INFO_ADDR(CAM_AWB_D_L0_Y);       // CAM+0x25F0
            INIT_REG_INFO_ADDR(CAM_AWB_D_L1_X);       // CAM+0x25F4
            INIT_REG_INFO_ADDR(CAM_AWB_D_L1_Y);       // CAM+0x25F8
            INIT_REG_INFO_ADDR(CAM_AWB_D_L2_X);       // CAM+0x25FC
            INIT_REG_INFO_ADDR(CAM_AWB_D_L2_Y);       // CAM+0x2600
            INIT_REG_INFO_ADDR(CAM_AWB_D_L3_X);       // CAM+0x2604
            INIT_REG_INFO_ADDR(CAM_AWB_D_L3_Y);       // CAM+0x2608
            INIT_REG_INFO_ADDR(CAM_AWB_D_L4_X);       // CAM+0x260C
            INIT_REG_INFO_ADDR(CAM_AWB_D_L4_Y);       // CAM+0x2610
            INIT_REG_INFO_ADDR(CAM_AWB_D_L5_X);       // CAM+0x2614
            INIT_REG_INFO_ADDR(CAM_AWB_D_L5_Y);       // CAM+0x2618
            INIT_REG_INFO_ADDR(CAM_AWB_D_L6_X);       // CAM+0x261C
            INIT_REG_INFO_ADDR(CAM_AWB_D_L6_Y);       // CAM+0x2620
            INIT_REG_INFO_ADDR(CAM_AWB_D_L7_X);       // CAM+0x2624
            INIT_REG_INFO_ADDR(CAM_AWB_D_L7_Y);       // CAM+0x2628
            INIT_REG_INFO_ADDR(CAM_AWB_D_L8_X);       // CAM+0x262C
            INIT_REG_INFO_ADDR(CAM_AWB_D_L8_Y);       // CAM+0x2630
            INIT_REG_INFO_ADDR(CAM_AWB_D_L9_X);       // CAM+0x2634
            INIT_REG_INFO_ADDR(CAM_AWB_D_L9_Y);       // CAM+0x2638
        }

    }

    virtual ~ISP_MGR_AWB_STAT_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1);

    MVOID setIspAEGain(MINT32 i4IspAEGain)
    {
        m_i4IspAEGain = i4IspAEGain;
    }

    MVOID getIspAEGain(MUINT32 *u4IspAEGain)
    {
        *u4IspAEGain = m_i4IspAEGain;
    }

public: //    Interfaces
    MBOOL config(AWB_STAT_CONFIG_T& rAWBStatConfig, MBOOL bHBIN2Enable, MINT32 i4SensorIndex);
    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply_TG1();
    MBOOL apply_TG2();

} ISP_MGR_AWB_STAT_CONFIG_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_AWB_STAT_CONFIG_DEV : public ISP_MGR_AWB_STAT_CONFIG_T
{
public:
    static
    ISP_MGR_AWB_STAT_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_AWB_STAT_CONFIG_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AWB_STAT_CONFIG_DEV()
        : ISP_MGR_AWB_STAT_CONFIG_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_AWB_STAT_CONFIG_DEV() {}

};

#endif

