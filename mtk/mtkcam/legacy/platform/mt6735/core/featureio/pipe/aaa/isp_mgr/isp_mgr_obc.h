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
#ifndef _ISP_MGR_OBC_H_
#define _ISP_MGR_OBC_H_

typedef class ISP_MGR_OBC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_OBC    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MINT32 m_u4IspAEGain;
    MUINT32 m_u4StartAddr;

    // TG1
    enum
    {
        ERegInfo_CAM_OBC_OFFST0,
        ERegInfo_CAM_OBC_OFFST1,
        ERegInfo_CAM_OBC_OFFST2,
        ERegInfo_CAM_OBC_OFFST3,
        ERegInfo_CAM_OBC_GAIN0,
        ERegInfo_CAM_OBC_GAIN1,
        ERegInfo_CAM_OBC_GAIN2,
        ERegInfo_CAM_OBC_GAIN3,
        ERegInfo_NUM
    };

    // TG2
    enum
    {
        ERegInfo_CAM_OBC_D_OFFST0,
        ERegInfo_CAM_OBC_D_OFFST1,
        ERegInfo_CAM_OBC_D_OFFST2,
        ERegInfo_CAM_OBC_D_OFFST3,
        ERegInfo_CAM_OBC_D_GAIN0,
        ERegInfo_CAM_OBC_D_GAIN1,
        ERegInfo_CAM_OBC_D_GAIN2,
        ERegInfo_CAM_OBC_D_GAIN3
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_OBC(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        , m_u4IspAEGain(512)
        , m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_OBC_OFFST0) : REG_ADDR(CAM_OBC_D_OFFST0))
    {
        // register info addr init
        if (eSensorTG == ESensorTG_1) {
            INIT_REG_INFO_ADDR(CAM_OBC_OFFST0); // CAM+0x0500
            INIT_REG_INFO_ADDR(CAM_OBC_OFFST1); // CAM+0x0504
            INIT_REG_INFO_ADDR(CAM_OBC_OFFST2); // CAM+0x0508
            INIT_REG_INFO_ADDR(CAM_OBC_OFFST3); // CAM+0x050C
            INIT_REG_INFO_ADDR(CAM_OBC_GAIN0);  // CAM+0x0510
            INIT_REG_INFO_ADDR(CAM_OBC_GAIN1);  // CAM+0x0514
            INIT_REG_INFO_ADDR(CAM_OBC_GAIN2);  // CAM+0x0518
            INIT_REG_INFO_ADDR(CAM_OBC_GAIN3);  // CAM+0x051C
        }
        else {
            INIT_REG_INFO_ADDR(CAM_OBC_D_OFFST0); // CAM+0x2500
            INIT_REG_INFO_ADDR(CAM_OBC_D_OFFST1); // CAM+0x2504
            INIT_REG_INFO_ADDR(CAM_OBC_D_OFFST2); // CAM+0x2508
            INIT_REG_INFO_ADDR(CAM_OBC_D_OFFST3); // CAM+0x250C
            INIT_REG_INFO_ADDR(CAM_OBC_D_GAIN0);  // CAM+0x2510
            INIT_REG_INFO_ADDR(CAM_OBC_D_GAIN1);  // CAM+0x2514
            INIT_REG_INFO_ADDR(CAM_OBC_D_GAIN2);  // CAM+0x2518
            INIT_REG_INFO_ADDR(CAM_OBC_D_GAIN3);  // CAM+0x251C
        }
    }

    virtual ~ISP_MGR_OBC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    MVOID setIspAEGain(MUINT32 u4IspAEGain)
    {
        m_u4IspAEGain = u4IspAEGain;
    }

    MVOID getIspAEGain(MUINT32 *u4IspAEGain)
    {
        *u4IspAEGain = m_u4IspAEGain;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_OBC_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_OBC_DEV : public ISP_MGR_OBC_T
{
public:
    static
    ISP_MGR_OBC_T&
    getInstance()
    {
        static ISP_MGR_OBC_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_OBC_DEV()
        : ISP_MGR_OBC_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_OBC_DEV() {}

};

#endif

