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
#ifndef _ISP_MGR_AE_STAT_H_
#define _ISP_MGR_AE_STAT_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AE_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AE_STAT_CONFIG    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MUINT32 m_u4StartAddr;
    MBOOL m_bTG1Init;
    MBOOL m_bTG2Init;

    // TG1
    enum
    {
        ERegInfo_CAM_AE_HST_CTL,
        ERegInfo_CAM_AE_GAIN2_0,
        ERegInfo_CAM_AE_GAIN2_1,
        ERegInfo_CAM_AE_LMT2_0,
        ERegInfo_CAM_AE_LMT2_1,
        ERegInfo_CAM_AE_RC_CNV_0,
        ERegInfo_CAM_AE_RC_CNV_1,
        ERegInfo_CAM_AE_RC_CNV_2,
        ERegInfo_CAM_AE_RC_CNV_3,
        ERegInfo_CAM_AE_RC_CNV_4,
        ERegInfo_CAM_AE_YGAMMA_0,
        ERegInfo_CAM_AE_YGAMMA_1,
        ERegInfo_CAM_AE_HST_SET,
        ERegInfo_CAM_AE_HST0_RNG,
        ERegInfo_CAM_AE_HST1_RNG,
        ERegInfo_CAM_AE_HST2_RNG,
        ERegInfo_CAM_AE_HST3_RNG,
        ERegInfo_CAM_AE_SPARE,
        ERegInfo_NUM
    };

    // TG2
    enum
    {
        ERegInfo_CAM_AE_D_HST_CTL,
        ERegInfo_CAM_AE_D_GAIN2_0,
        ERegInfo_CAM_AE_D_GAIN2_1,
        ERegInfo_CAM_AE_D_LMT2_0,
        ERegInfo_CAM_AE_D_LMT2_1,
        ERegInfo_CAM_AE_D_RC_CNV_0,
        ERegInfo_CAM_AE_D_RC_CNV_1,
        ERegInfo_CAM_AE_D_RC_CNV_2,
        ERegInfo_CAM_AE_D_RC_CNV_3,
        ERegInfo_CAM_AE_D_RC_CNV_4,
        ERegInfo_CAM_AE_D_YGAMMA_0,
        ERegInfo_CAM_AE_D_YGAMMA_1,
        ERegInfo_CAM_AE_D_HST_SET,
        ERegInfo_CAM_AE_D_HST0_RNG,
        ERegInfo_CAM_AE_D_HST1_RNG,
        ERegInfo_CAM_AE_D_HST2_RNG,
        ERegInfo_CAM_AE_D_HST3_RNG,
        ERegInfo_CAM_AE_D_SPARE
    };

    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    AWB_GAIN_T  m_rIspAEPreGain2; // 1x = 512

protected:
    ISP_MGR_AE_STAT_CONFIG(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_AE_HST_CTL) : REG_ADDR(CAM_AE_D_HST_CTL))
        , m_bTG1Init(0)
        , m_bTG2Init(0)
    {
        // register info addr init
        if (eSensorTG == ESensorTG_1) {
            INIT_REG_INFO_ADDR(CAM_AE_HST_CTL);    // CAM+0x0650
            INIT_REG_INFO_ADDR(CAM_AE_GAIN2_0);    // CAM+0x0654
            INIT_REG_INFO_ADDR(CAM_AE_GAIN2_1);    // CAM+0x0658
            INIT_REG_INFO_ADDR(CAM_AE_LMT2_0);      // CAM+0x065C
            INIT_REG_INFO_ADDR(CAM_AE_LMT2_1);      // CAM+0x0660
            INIT_REG_INFO_ADDR(CAM_AE_RC_CNV_0);  // CAM+0x0664
            INIT_REG_INFO_ADDR(CAM_AE_RC_CNV_1);  // CAM+0x0668
            INIT_REG_INFO_ADDR(CAM_AE_RC_CNV_2);  // CAM+0x066C
            INIT_REG_INFO_ADDR(CAM_AE_RC_CNV_3);  // CAM+0x0670
            INIT_REG_INFO_ADDR(CAM_AE_RC_CNV_4);  // CAM+0x0674
            INIT_REG_INFO_ADDR(CAM_AE_YGAMMA_0); // CAM+0x0678
            INIT_REG_INFO_ADDR(CAM_AE_YGAMMA_1); // CAM+0x067C
            INIT_REG_INFO_ADDR(CAM_AE_HST_SET);    // CAM+0x0680
            INIT_REG_INFO_ADDR(CAM_AE_HST0_RNG);  // CAM+0x0684
            INIT_REG_INFO_ADDR(CAM_AE_HST1_RNG);  // CAM+0x0688
            INIT_REG_INFO_ADDR(CAM_AE_HST2_RNG);  // CAM+0x068C
            INIT_REG_INFO_ADDR(CAM_AE_HST3_RNG);  // CAM+0x0690
            INIT_REG_INFO_ADDR(CAM_AE_SPARE);        // CAM+0x0694
        }
        else {
            INIT_REG_INFO_ADDR(CAM_AE_D_HST_CTL);    // CAM+0x2650
            INIT_REG_INFO_ADDR(CAM_AE_D_GAIN2_0);   // CAM+0x2654
            INIT_REG_INFO_ADDR(CAM_AE_D_GAIN2_1);    // CAM+0x2658
            INIT_REG_INFO_ADDR(CAM_AE_D_LMT2_0);    // CAM+0x265C
            INIT_REG_INFO_ADDR(CAM_AE_D_LMT2_1);    // CAM+0x2660
            INIT_REG_INFO_ADDR(CAM_AE_D_RC_CNV_0);    // CAM+0x2664
            INIT_REG_INFO_ADDR(CAM_AE_D_RC_CNV_1);     // CAM+0x2668
            INIT_REG_INFO_ADDR(CAM_AE_D_RC_CNV_2);     // CAM+0x266C
            INIT_REG_INFO_ADDR(CAM_AE_D_RC_CNV_3);    // CAM+0x2670
            INIT_REG_INFO_ADDR(CAM_AE_D_RC_CNV_4);     // CAM+0x2674
            INIT_REG_INFO_ADDR(CAM_AE_D_YGAMMA_0); // CAM+0x2678
            INIT_REG_INFO_ADDR(CAM_AE_D_YGAMMA_1); // CAM+0x267C
            INIT_REG_INFO_ADDR(CAM_AE_D_HST_SET); // CAM+0x2680
            INIT_REG_INFO_ADDR(CAM_AE_D_HST0_RNG);    // CAM+0x2684
            INIT_REG_INFO_ADDR(CAM_AE_D_HST1_RNG);        // CAM+0x2688
            INIT_REG_INFO_ADDR(CAM_AE_D_HST2_RNG);       // CAM+0x268C
            INIT_REG_INFO_ADDR(CAM_AE_D_HST3_RNG);       // CAM+0x2690
            INIT_REG_INFO_ADDR(CAM_AE_D_SPARE);       // CAM+0x2694
        }

        m_rIspAEPreGain2.i4R = m_rIspAEPreGain2.i4G = m_rIspAEPreGain2.i4B = 512;
    }

    virtual ~ISP_MGR_AE_STAT_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1);

public: //    Interfaces
    MBOOL config(MINT32 i4SensorIndex, AE_STAT_PARAM_T & rAEStatConfig);

    MBOOL setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T& rIspAWBGain);

    inline AWB_GAIN_T getIspAEPreGain2()
    {
        return m_rIspAEPreGain2;
    }

    MBOOL apply_TG1(MINT32 i4SensorIndex);
    MBOOL apply_TG2(MINT32 i4SensorIndex);

} ISP_MGR_AE_STAT_CONFIG_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_AE_STAT_CONFIG_DEV : public ISP_MGR_AE_STAT_CONFIG_T
{
public:
    static
    ISP_MGR_AE_STAT_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_AE_STAT_CONFIG_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AE_STAT_CONFIG_DEV()
        : ISP_MGR_AE_STAT_CONFIG_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_AE_STAT_CONFIG_DEV() {}

};

#endif
