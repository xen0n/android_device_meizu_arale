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
#ifndef _ISP_MGR_CTL_H_
#define _ISP_MGR_CTL_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Enable (Pass1@TG1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CTL_EN_P1 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CTL_EN_P1    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0004H

    enum
    {
        ERegInfo_CAM_CTL_EN_P1,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_CTL_EN_P1()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, ESensorDev_Main)
        , m_u4StartAddr(REG_ADDR(CAM_CTL_EN_P1))
    {
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_CTL_EN_P1); // CAM+0004H
        INIT_REG_INFO_VALUE(CAM_CTL_EN_P1,0x00000000);
    }

    virtual ~ISP_MGR_CTL_EN_P1() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

    MVOID
    setEnable_OB(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1))->OB_EN = bEnable;
    }

    MVOID
    setEnable_BNR(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1))->BNR_EN = bEnable;
    }

    MVOID
    setEnable_LSC(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1))->LSC_EN = bEnable;
    }

    MVOID
    setEnable_RPG(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1))->RPG_EN = bEnable;
    }

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_CTL_EN_P1_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CTL_EN_P1_DEV : public ISP_MGR_CTL_EN_P1_T
{
public:
    static
    ISP_MGR_CTL_EN_P1_T&
    getInstance()
    {
        static ISP_MGR_CTL_EN_P1_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CTL_EN_P1_DEV()
        : ISP_MGR_CTL_EN_P1_T()
    {}

    virtual ~ISP_MGR_CTL_EN_P1_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Enable (Pass1@TG2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CTL_EN_P1_D : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CTL_EN_P1_D    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0004H

    enum
    {
        ERegInfo_CAM_CTL_EN_P1_D,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_CTL_EN_P1_D()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, ESensorDev_Main)
        , m_u4StartAddr(REG_ADDR(CAM_CTL_EN_P1_D))
    {
        INIT_REG_INFO_ADDR(CAM_CTL_EN_P1_D); // CAM+0010H
        INIT_REG_INFO_VALUE(CAM_CTL_EN_P1_D,0x00000000);
    }

    virtual ~ISP_MGR_CTL_EN_P1_D() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

    MVOID
    setEnable_OB(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_D_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1_D))->OB_D_EN = bEnable;
    }

    MVOID
    setEnable_BNR(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_D_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1_D))->BNR_D_EN = bEnable;
    }

    MVOID
    setEnable_LSC(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_D_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1_D))->LSC_D_EN = bEnable;
    }

    MVOID
    setEnable_RPG(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P1_D_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P1_D))->RPG_D_EN = bEnable;
    }

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_CTL_EN_P1_D_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CTL_EN_P1_D_DEV : public ISP_MGR_CTL_EN_P1_D_T
{
public:
    static
    ISP_MGR_CTL_EN_P1_D_T&
    getInstance()
    {
        static ISP_MGR_CTL_EN_P1_D_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CTL_EN_P1_D_DEV()
        : ISP_MGR_CTL_EN_P1_D_T()
    {}

    virtual ~ISP_MGR_CTL_EN_P1_D_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Enable (Pass2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CTL_EN_P2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CTL_EN_P2    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0018H

    enum
    {
        ERegInfo_CAM_CTL_EN_P2,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_CTL_EN_P2()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, ESensorDev_Main)
        , m_u4StartAddr(REG_ADDR(CAM_CTL_EN_P2))
    {
        INIT_REG_INFO_ADDR(CAM_CTL_EN_P2); // CAM+0018H
        INIT_REG_INFO_VALUE(CAM_CTL_EN_P2,0x00000000);
    }

    virtual ~ISP_MGR_CTL_EN_P2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

    MVOID
    setEnable_PGN(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->PGN_EN = bEnable;
    }

    MVOID
    setEnable_SL2(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->SL2_EN = bEnable;
    }

    MVOID
    setEnable_CFA(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->CFA_EN = bEnable;
    }

    MVOID
    setEnable_G2G(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->G2G_EN = bEnable;
    }

    MVOID
    setEnable_GGM(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->GGM_EN = bEnable;
    }

    MVOID
    setEnable_MFB(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->MFB_EN = bEnable;
    }

    MVOID
    setEnable_G2C(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->G2C_EN = bEnable;
    }

    MVOID
    setEnable_NBC(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->NBC_EN = bEnable;
    }

    MVOID
    setEnable_PCA(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->PCA_EN = bEnable;
    }

    MVOID
    setEnable_SEEE(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->SEEE_EN = bEnable;
    }

    MVOID
    setEnable_LCE(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->LCE_EN = bEnable;
    }

    MVOID
    setEnable_NR3D(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->NR3D_EN = bEnable;
    }

    MVOID
    setEnable_MIX3(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_P2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN_P2))->MIX3_EN = bEnable;
    }

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_CTL_EN_P2_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CTL_EN_P2_DEV : public ISP_MGR_CTL_EN_P2_T
{
public:
    static
    ISP_MGR_CTL_EN_P2_T&
    getInstance()
    {
        static ISP_MGR_CTL_EN_P2_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CTL_EN_P2_DEV()
        : ISP_MGR_CTL_EN_P2_T()
    {}

    virtual ~ISP_MGR_CTL_EN_P2_DEV() {}

};

#endif

