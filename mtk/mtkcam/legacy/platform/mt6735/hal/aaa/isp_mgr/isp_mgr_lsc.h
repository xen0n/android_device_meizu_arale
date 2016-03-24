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
#ifndef _ISP_MGR_LSC_H_
#define _ISP_MGR_LSC_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_LSC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LSC    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MUINT32 m_u4StartAddr;
    MBOOL m_fgOnOff;

    enum
    {
        ERegInfo_CAM_LSCI_BASE_ADDR,
        ERegInfo_CAM_LSCI_OFST_ADDR,
        ERegInfo_CAM_LSCI_XSIZE,
        ERegInfo_CAM_LSCI_YSIZE,
        ERegInfo_CAM_LSCI_STRIDE,
        ERegInfo_CAM_LSC_START,
        ERegInfo_CAM_LSC_CTL1 = ERegInfo_CAM_LSC_START,
        ERegInfo_CAM_LSC_CTL2,
        ERegInfo_CAM_LSC_CTL3,
        ERegInfo_CAM_LSC_LBLOCK,
        ERegInfo_CAM_LSC_RATIO,
        ERegInfo_CAM_LSC_GAIN_TH,
        ERegInfo_NUM
    };

    enum
    {
        ERegInfo_CAM_LSCI_D_BASE_ADDR,
        ERegInfo_CAM_LSCI_D_OFST_ADDR,
        ERegInfo_CAM_LSCI_D_XSIZE,
        ERegInfo_CAM_LSCI_D_YSIZE,
        ERegInfo_CAM_LSCI_D_STRIDE,
        ERegInfo_CAM_LSC_D_START,
        ERegInfo_CAM_LSC_D_CTL1 = ERegInfo_CAM_LSC_D_START,
        ERegInfo_CAM_LSC_D_CTL2,
        ERegInfo_CAM_LSC_D_CTL3,
        ERegInfo_CAM_LSC_D_LBLOCK,
        ERegInfo_CAM_LSC_D_RATIO,
        ERegInfo_CAM_LSC_D_GAIN_TH
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_LSC(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_LSCI_BASE_ADDR) : REG_ADDR(CAM_LSCI_D_BASE_ADDR))
        , m_fgOnOff(MFALSE)
    {
        if (eSensorTG == ESensorTG_1)
        {
            INIT_REG_INFO_ADDR(CAM_LSCI_BASE_ADDR); // 0x1500726c
            INIT_REG_INFO_ADDR(CAM_LSCI_OFST_ADDR); // 0x15007270
            INIT_REG_INFO_ADDR(CAM_LSCI_XSIZE); // 0x15007274
            INIT_REG_INFO_ADDR(CAM_LSCI_YSIZE); // 0x15007278
            INIT_REG_INFO_ADDR(CAM_LSCI_STRIDE); // 0x1500727c
            INIT_REG_INFO_ADDR(CAM_LSC_CTL1);    // 0x15004530
            INIT_REG_INFO_ADDR(CAM_LSC_CTL2);    // 0x15004534
            INIT_REG_INFO_ADDR(CAM_LSC_CTL3);    // 0x15004538
            INIT_REG_INFO_ADDR(CAM_LSC_LBLOCK);  // 0x1500453C
            INIT_REG_INFO_ADDR(CAM_LSC_RATIO);   // 0x15004540
            INIT_REG_INFO_ADDR(CAM_LSC_GAIN_TH); // 0x1500454C
        }
        else
        {
            INIT_REG_INFO_ADDR(CAM_LSCI_D_BASE_ADDR); // 0x150074b8
            INIT_REG_INFO_ADDR(CAM_LSCI_D_OFST_ADDR); // 0x150074bc
            INIT_REG_INFO_ADDR(CAM_LSCI_D_XSIZE); // 0x150074c0
            INIT_REG_INFO_ADDR(CAM_LSCI_D_YSIZE); // 0x150074c4
            INIT_REG_INFO_ADDR(CAM_LSCI_D_STRIDE); // 0x150074c8
            INIT_REG_INFO_ADDR(CAM_LSC_D_CTL1);    // 0x15006530
            INIT_REG_INFO_ADDR(CAM_LSC_D_CTL2);    // 0x15006534
            INIT_REG_INFO_ADDR(CAM_LSC_D_CTL3);    // 0x15006538
            INIT_REG_INFO_ADDR(CAM_LSC_D_LBLOCK);  // 0x1500653C
            INIT_REG_INFO_ADDR(CAM_LSC_D_RATIO);   // 0x15006540
            INIT_REG_INFO_ADDR(CAM_LSC_D_GAIN_TH); // 0x1500654C
        }
    }

    virtual ~ISP_MGR_LSC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MyType& putAddr(MUINT32 u4BaseAddr);
    MUINT32 getAddr();

    // override to do nothing
    MBOOL reset();

    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);
    MVOID enableLsc(MBOOL enable);
    MBOOL isEnable(void);
} ISP_MGR_LSC_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_LSC_DEV : public ISP_MGR_LSC_T
{
public:
    static
    ISP_MGR_LSC_DEV&
    getInstance()
    {
        static ISP_MGR_LSC_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_LSC_DEV()
        : ISP_MGR_LSC_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_LSC_DEV() {}

};


#endif

