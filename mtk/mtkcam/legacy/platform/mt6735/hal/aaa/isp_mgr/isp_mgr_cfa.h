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
#ifndef _ISP_MGR_CFA_H_
#define _ISP_MGR_CFA_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CFA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CFA : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CFA    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0x08A0

    enum
    {
        ERegInfo_CAM_DM_O_BYP,
        ERegInfo_CAM_DM_O_ED_FLAT,
        ERegInfo_CAM_DM_O_ED_NYQ,
        ERegInfo_CAM_DM_O_ED_STEP,
        ERegInfo_CAM_DM_O_RGB_HF,
        ERegInfo_CAM_DM_O_DOT,
        ERegInfo_CAM_DM_O_F1_ACT,
        ERegInfo_CAM_DM_O_F2_ACT,
        ERegInfo_CAM_DM_O_F3_ACT,
        ERegInfo_CAM_DM_O_F4_ACT,
        ERegInfo_CAM_DM_O_F1_L,
        ERegInfo_CAM_DM_O_F2_L,
        ERegInfo_CAM_DM_O_F3_L,
        ERegInfo_CAM_DM_O_F4_L,
        ERegInfo_CAM_DM_O_HF_RB,
        ERegInfo_CAM_DM_O_HF_GAIN,
        ERegInfo_CAM_DM_O_HF_COMP,
        ERegInfo_CAM_DM_O_HF_CORIN_TH,
        ERegInfo_CAM_DM_O_ACT_LUT,
        ERegInfo_CAM_DM_O_SPARE,
        ERegInfo_CAM_DM_O_BB,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_CFA(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_DM_O_BYP))
    {
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_DM_O_BYP);          // CAM+0x08A0
        INIT_REG_INFO_ADDR(CAM_DM_O_ED_FLAT);      // CAM+0x08A4
        INIT_REG_INFO_ADDR(CAM_DM_O_ED_NYQ);       // CAM+0x08A8
        INIT_REG_INFO_ADDR(CAM_DM_O_ED_STEP);      // CAM+0x08AC
        INIT_REG_INFO_ADDR(CAM_DM_O_RGB_HF);       // CAM+0x08B0
        INIT_REG_INFO_ADDR(CAM_DM_O_DOT);          // CAM+0x08B4
        INIT_REG_INFO_ADDR(CAM_DM_O_F1_ACT);       // CAM+0x08B8
        INIT_REG_INFO_ADDR(CAM_DM_O_F2_ACT);       // CAM+0x08BC
        INIT_REG_INFO_ADDR(CAM_DM_O_F3_ACT);       // CAM+0x08C0
        INIT_REG_INFO_ADDR(CAM_DM_O_F4_ACT);       // CAM+0x08C4
        INIT_REG_INFO_ADDR(CAM_DM_O_F1_L);         // CAM+0x08C8
        INIT_REG_INFO_ADDR(CAM_DM_O_F2_L);         // CAM+0x08CC
        INIT_REG_INFO_ADDR(CAM_DM_O_F3_L);         // CAM+0x08D0
        INIT_REG_INFO_ADDR(CAM_DM_O_F4_L);         // CAM+0x08D4
        INIT_REG_INFO_ADDR(CAM_DM_O_HF_RB);        // CAM+0x08D8
        INIT_REG_INFO_ADDR(CAM_DM_O_HF_GAIN);      // CAM+0x08DC
        INIT_REG_INFO_ADDR(CAM_DM_O_HF_COMP);      // CAM+0x08E0
        INIT_REG_INFO_ADDR(CAM_DM_O_HF_CORIN_TH);  // CAM+0x08E4
        INIT_REG_INFO_ADDR(CAM_DM_O_ACT_LUT);      // CAM+0x08E8
        INIT_REG_INFO_ADDR(CAM_DM_O_SPARE);        // CAM+0x08F0
        INIT_REG_INFO_ADDR(CAM_DM_O_BB);           // CAM+0x08F4

    }

    virtual ~ISP_MGR_CFA() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

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

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);

} ISP_MGR_CFA_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CFA_DEV : public ISP_MGR_CFA_T
{
public:
    static
    ISP_MGR_CFA_T&
    getInstance()
    {
        static ISP_MGR_CFA_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CFA_DEV()
        : ISP_MGR_CFA_T(eSensorDev)
    {}

    virtual ~ISP_MGR_CFA_DEV() {}

};


#endif

