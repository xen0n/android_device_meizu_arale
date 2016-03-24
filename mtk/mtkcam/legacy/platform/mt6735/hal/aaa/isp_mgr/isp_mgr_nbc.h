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
#ifndef _ISP_MGR_NBC_H_
#define _ISP_MGR_NBC_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NBC (ANR + CCR)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_NBC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NBC    MyType;
private:
    MBOOL m_bANR_ENYEnable;
    MBOOL m_bANR_ENCEnable;
    MBOOL m_bCCREnable;
    MBOOL m_bCCREnBackup;
    MBOOL m_bANRENCBackup;
    MBOOL m_bANRENYBackup;
    MBOOL m_bCCTCCREnable;
    MBOOL m_bCCTANREnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x0A20

    enum
    {
        ERegInfo_CAM_ANR_CON1,
        ERegInfo_CAM_ANR_CON2,
        ERegInfo_CAM_ANR_CON3,
        ERegInfo_CAM_ANR_YAD1,
        ERegInfo_CAM_ANR_YAD2,
        ERegInfo_CAM_ANR_4LUT1,
        ERegInfo_CAM_ANR_4LUT2,
        ERegInfo_CAM_ANR_4LUT3,
        ERegInfo_CAM_ANR_PTY,
        ERegInfo_CAM_ANR_CAD,
        ERegInfo_CAM_ANR_PTC,
        ERegInfo_CAM_ANR_LCE1,
        ERegInfo_CAM_ANR_LCE2,
        ERegInfo_CAM_ANR_HP1,
        ERegInfo_CAM_ANR_HP2,
        ERegInfo_CAM_ANR_HP3,
        ERegInfo_CAM_ANR_ACTY,
        ERegInfo_CAM_ANR_ACTC,
        ERegInfo_CAM_CCR_CON,
        ERegInfo_CAM_CCR_YLUT,
        ERegInfo_CAM_CCR_UVLUT,
        ERegInfo_CAM_CCR_YLUT2,
        ERegInfo_CAM_CCR_SAT_CTRL,
        ERegInfo_CAM_CCR_UVLUT_SP,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_NBC(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bANR_ENYEnable(MTRUE)
        , m_bANR_ENCEnable(MTRUE)
        , m_bCCREnable(MTRUE)
        , m_bCCREnBackup(MFALSE)
        , m_bANRENCBackup(MFALSE)
        , m_bANRENYBackup(MFALSE)
        , m_bCCTCCREnable(MTRUE)
        , m_bCCTANREnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_ANR_CON1))
    {
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_ANR_CON1);  // CAM+0x0A20
        INIT_REG_INFO_ADDR(CAM_ANR_CON2);  // CAM+0x0A24
        INIT_REG_INFO_ADDR(CAM_ANR_CON3);  // CAM+0x0A28
        INIT_REG_INFO_ADDR(CAM_ANR_YAD1);  // CAM+0x0A2C
        INIT_REG_INFO_ADDR(CAM_ANR_YAD2);  // CAM+0x0A30
        INIT_REG_INFO_ADDR(CAM_ANR_4LUT1); // CAM+0x0A34
        INIT_REG_INFO_ADDR(CAM_ANR_4LUT2); // CAM+0x0A38
        INIT_REG_INFO_ADDR(CAM_ANR_4LUT3); // CAM+0x0A3C
        INIT_REG_INFO_ADDR(CAM_ANR_PTY);   // CAM+0x0A40
        INIT_REG_INFO_ADDR(CAM_ANR_CAD);   // CAM+0x0A44
        INIT_REG_INFO_ADDR(CAM_ANR_PTC);   // CAM+0x0A48
        INIT_REG_INFO_ADDR(CAM_ANR_LCE1);  // CAM+0x0A4C
        INIT_REG_INFO_ADDR(CAM_ANR_LCE2);  // CAM+0x0A50
        INIT_REG_INFO_ADDR(CAM_ANR_HP1);   // CAM+0x0A54
        INIT_REG_INFO_ADDR(CAM_ANR_HP2);   // CAM+0x0A58
        INIT_REG_INFO_ADDR(CAM_ANR_HP3);   // CAM+0x0A5C
        INIT_REG_INFO_ADDR(CAM_ANR_ACTY);  // CAM+0x0A60
        INIT_REG_INFO_ADDR(CAM_ANR_ACTC);  // CAM+0x0A64
        INIT_REG_INFO_ADDR(CAM_CCR_CON);   // CAM+0x0A80
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT);  // CAM+0x0A84
        INIT_REG_INFO_ADDR(CAM_CCR_UVLUT); // CAM+0x0A88
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT2); // CAM+0x0A8C
        INIT_REG_INFO_ADDR(CAM_CCR_SAT_CTRL); // CAM+0x0A90
        INIT_REG_INFO_ADDR(CAM_CCR_UVLUT_SP); // CAM+0x0A94
    }

    virtual ~ISP_MGR_NBC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isANR_ENYEnable()
    {
        return m_bANR_ENYEnable;
    }

    MBOOL
    isANR_ENCEnable()
    {
        return m_bANR_ENCEnable;
    }

    MBOOL
    isCCREnable()
    {
        return m_bCCREnable;
    }

    MBOOL
    isCCTCCREnable()
    {
        return m_bCCTCCREnable;
    }

    MBOOL
    isCCTANREnable()
    {
        return m_bCCTANREnable;
    }

    MVOID
    setANREnable(MBOOL bEnable)
    {
         setANR_ENYEnable(bEnable);
         setANR_ENCEnable(bEnable);
    }

    MVOID
    setANR_ENYEnable(MBOOL bEnable)
    {
        m_bANR_ENYEnable = bEnable;
    }

    MVOID
    setANR_ENCEnable(MBOOL bEnable)
    {
        m_bANR_ENCEnable = bEnable;
    }

    MVOID
    setCCREnable(MBOOL bEnable)
    {
        m_bCCREnable = bEnable;
    }

    MVOID
    setCCTANREnable(MBOOL bEnable)
    {
         m_bCCTANREnable = bEnable;
    }

    MVOID
    setCCTCCREnable(MBOOL bEnable)
    {
        m_bCCTCCREnable = bEnable;
    }


    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);


} ISP_MGR_NBC_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_NBC_DEV : public ISP_MGR_NBC_T
{
public:
    static
    ISP_MGR_NBC_T&
    getInstance()
    {
        static ISP_MGR_NBC_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_NBC_DEV()
        : ISP_MGR_NBC_T(eSensorDev)
    {}

    virtual ~ISP_MGR_NBC_DEV() {}

};


#endif

