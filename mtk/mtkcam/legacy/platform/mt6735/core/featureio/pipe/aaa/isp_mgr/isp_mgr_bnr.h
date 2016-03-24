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
#ifndef _ISP_MGR_BNR_H_
#define _ISP_MGR_BNR_H_

typedef class ISP_MGR_BNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BNR    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MBOOL m_bBPCEnable;
    MBOOL m_bCTEnable;
    MBOOL m_bCCTBPCEnable;
    MBOOL m_bCCTCTEnable;
    MUINT32 m_u4StartAddr;
    MINT32 m_i4RMM_Rgain;
    MINT32 m_i4RMM_Ggain;
    MINT32 m_i4RMM_Bgain;
    MFLOAT m_u4AeExpRatio;

    // TG1
    enum
    {
        ERegInfo_CAM_BPC_CON,
        ERegInfo_CAM_BPC_TH1,
        ERegInfo_CAM_BPC_TH2,
        ERegInfo_CAM_BPC_TH3,
        ERegInfo_CAM_BPC_TH4,
        ERegInfo_CAM_BPC_DTC,
        ERegInfo_CAM_BPC_COR,
        ERegInfo_CAM_BPC_TBLI1,
        ERegInfo_CAM_BPC_TBLI2,
        ERegInfo_CAM_BPC_TH1_C,
        ERegInfo_CAM_BPC_TH2_C,
        ERegInfo_CAM_BPC_TH3_C,
        ERegInfo_CAM_BPC_RMM1,
        ERegInfo_CAM_BPC_RMM2,
        ERegInfo_CAM_BPC_RMM_REVG_1,
        ERegInfo_CAM_BPC_RMM_REVG_2,
        ERegInfo_CAM_BPC_RMM_LEOS,
        ERegInfo_CAM_BPC_RMM_GCNT,
        ERegInfo_CAM_NR1_CON,
        ERegInfo_CAM_NR1_CT_CON,
        ERegInfo_NUM
    };

    // TG2
    enum
    {
        ERegInfo_CAM_BPC_D_CON,
        ERegInfo_CAM_BPC_D_TH1,
        ERegInfo_CAM_BPC_D_TH2,
        ERegInfo_CAM_BPC_D_TH3,
        ERegInfo_CAM_BPC_D_TH4,
        ERegInfo_CAM_BPC_D_DTC,
        ERegInfo_CAM_BPC_D_COR,
        ERegInfo_CAM_BPC_D_TBLI1,
        ERegInfo_CAM_BPC_D_TBLI2,
        ERegInfo_CAM_BPC_D_TH1_C,
        ERegInfo_CAM_BPC_D_TH2_C,
        ERegInfo_CAM_BPC_D_TH3_C,
        ERegInfo_CAM_BPC_D_RMM1,
        ERegInfo_CAM_BPC_D_RMM2,
        ERegInfo_CAM_BPC_D_RMM_REVG_1,
        ERegInfo_CAM_BPC_D_RMM_REVG_2,
        ERegInfo_CAM_BPC_D_RMM_LEOS,
        ERegInfo_CAM_BPC_D_RMM_GCNT,
        ERegInfo_CAM_NR1_D_CON,
        ERegInfo_CAM_NR1_D_CT_CON
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

    MVOID updateRMM();

protected:
    ISP_MGR_BNR(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_bBPCEnable(MTRUE)
        , m_bCTEnable(MTRUE)
        , m_bCCTBPCEnable(MTRUE)
        , m_bCCTCTEnable(MTRUE)
        , m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_BPC_CON) : REG_ADDR(CAM_BPC_D_CON))
        , m_i4RMM_Rgain(1024)
        , m_i4RMM_Ggain(1024)
        , m_i4RMM_Bgain(1024)
        , m_u4AeExpRatio(1.0)
    {
        // register info addr init
        if (eSensorTG == ESensorTG_1) {
            INIT_REG_INFO_ADDR(CAM_BPC_CON);        // CAM+0x0800
            INIT_REG_INFO_ADDR(CAM_BPC_TH1);        // CAM+0x0804
            INIT_REG_INFO_ADDR(CAM_BPC_TH2);        // CAM+0x0808
            INIT_REG_INFO_ADDR(CAM_BPC_TH3);        // CAM+0x080C
            INIT_REG_INFO_ADDR(CAM_BPC_TH4);        // CAM+0x0810
            INIT_REG_INFO_ADDR(CAM_BPC_DTC);        // CAM+0x0814
            INIT_REG_INFO_ADDR(CAM_BPC_COR);        // CAM+0x0818
            INIT_REG_INFO_ADDR(CAM_BPC_TBLI1);      // CAM+0x081C
            INIT_REG_INFO_ADDR(CAM_BPC_TBLI2);      // CAM+0x0820
            INIT_REG_INFO_ADDR(CAM_BPC_TH1_C);      // CAM+0x0824
            INIT_REG_INFO_ADDR(CAM_BPC_TH2_C);      // CAM+0x0828
            INIT_REG_INFO_ADDR(CAM_BPC_TH3_C);      // CAM+0x082C
            INIT_REG_INFO_ADDR(CAM_BPC_RMM1);       // CAM+0x0830
            INIT_REG_INFO_ADDR(CAM_BPC_RMM2);       // CAM+0x0834
            INIT_REG_INFO_ADDR(CAM_BPC_RMM_REVG_1); // CAM+0x0838
            INIT_REG_INFO_ADDR(CAM_BPC_RMM_REVG_2); // CAM+0x083C
            INIT_REG_INFO_ADDR(CAM_BPC_RMM_LEOS);   // CAM+0x0840
            INIT_REG_INFO_ADDR(CAM_BPC_RMM_GCNT);   // CAM+0x0844
            INIT_REG_INFO_ADDR(CAM_NR1_CON);        // CAM+0x0850
            INIT_REG_INFO_ADDR(CAM_NR1_CT_CON);     // CAM+0x0854
        }
        else {
            INIT_REG_INFO_ADDR(CAM_BPC_D_CON);        // CAM+0x2800
            INIT_REG_INFO_ADDR(CAM_BPC_D_TH1);        // CAM+0x2804
            INIT_REG_INFO_ADDR(CAM_BPC_D_TH2);        // CAM+0x2808
            INIT_REG_INFO_ADDR(CAM_BPC_D_TH3);        // CAM+0x280C
            INIT_REG_INFO_ADDR(CAM_BPC_D_TH4);        // CAM+0x2810
            INIT_REG_INFO_ADDR(CAM_BPC_D_DTC);        // CAM+0x2814
            INIT_REG_INFO_ADDR(CAM_BPC_D_COR);        // CAM+0x2818
            INIT_REG_INFO_ADDR(CAM_BPC_D_TBLI1);      // CAM+0x281C
            INIT_REG_INFO_ADDR(CAM_BPC_D_TBLI2);      // CAM+0x2820
            INIT_REG_INFO_ADDR(CAM_BPC_D_TH1_C);      // CAM+0x2824
            INIT_REG_INFO_ADDR(CAM_BPC_D_TH2_C);      // CAM+0x2828
            INIT_REG_INFO_ADDR(CAM_BPC_D_TH3_C);      // CAM+0x282C
            INIT_REG_INFO_ADDR(CAM_BPC_D_RMM1);       // CAM+0x2830
            INIT_REG_INFO_ADDR(CAM_BPC_D_RMM2);       // CAM+0x2834
            INIT_REG_INFO_ADDR(CAM_BPC_D_RMM_REVG_1); // CAM+0x2838
            INIT_REG_INFO_ADDR(CAM_BPC_D_RMM_REVG_2); // CAM+0x283C
            INIT_REG_INFO_ADDR(CAM_BPC_D_RMM_LEOS);   // CAM+0x2840
            INIT_REG_INFO_ADDR(CAM_BPC_D_RMM_GCNT);   // CAM+0x2844
            INIT_REG_INFO_ADDR(CAM_NR1_D_CON);        // CAM+0x2850
            INIT_REG_INFO_ADDR(CAM_NR1_D_CT_CON);     // CAM+0x2854
        }
    }

    virtual ~ISP_MGR_BNR() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isBPCEnable()
    {
        return m_bBPCEnable;
    }

    MBOOL
    isCTEnable()
    {
        return m_bCTEnable;
    }

    MBOOL
    isCCTBPCEnable()
    {
        return m_bCCTBPCEnable;
    }

    MBOOL
    isCCTCTEnable()
    {
        return m_bCCTCTEnable;
    }

    MVOID
    setBPCEnable(MBOOL bEnable)
    {
        m_bBPCEnable = bEnable;
    }

    MVOID
    setCTEnable(MBOOL bEnable)
    {
        m_bCTEnable = bEnable;
    }

    MVOID
    setCCTBPCEnable(MBOOL bEnable)
    {
        m_bCCTBPCEnable = bEnable;
    }

    MVOID
    setCCTCTEnable(MBOOL bEnable)
    {
        m_bCCTCTEnable = bEnable;
    }

    MVOID
    setRmmGain(AWB_GAIN_T awbGain)
    {
        m_i4RMM_Rgain = 512 * 1024 / awbGain.i4R;
        m_i4RMM_Ggain = 512 * 1024 / awbGain.i4G;
        m_i4RMM_Bgain = 512 * 1024 / awbGain.i4B;
    }

    MVOID
    setRmmAeExpRatio(MINT32 aeRatio)
    {
        m_u4AeExpRatio = aeRatio/100.0;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_BNR_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_BNR_DEV : public ISP_MGR_BNR_T
{
public:
    static
    ISP_MGR_BNR_T&
    getInstance()
    {
        static ISP_MGR_BNR_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_BNR_DEV()
        : ISP_MGR_BNR_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_BNR_DEV() {}

};

#endif

