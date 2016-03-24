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
#ifndef _ISP_MGR_NR3D_H_
#define _ISP_MGR_NR3D_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_NR3D : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NR3D    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr;

    enum
    {
        ERegInfo_CAM_NR3D_BLEND,
        ERegInfo_CAM_NR3D_LMT_CPX,
        ERegInfo_CAM_NR3D_LMT_Y_CON1,
        ERegInfo_CAM_NR3D_LMT_Y_CON2,
        ERegInfo_CAM_NR3D_LMT_Y_CON3,
        ERegInfo_CAM_NR3D_LMT_U_CON1,
        ERegInfo_CAM_NR3D_LMT_U_CON2,
        ERegInfo_CAM_NR3D_LMT_U_CON3,
        ERegInfo_CAM_NR3D_LMT_V_CON1,
        ERegInfo_CAM_NR3D_LMT_V_CON2,
        ERegInfo_CAM_NR3D_LMT_V_CON3,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_NR3D(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MFALSE)
        , m_u4StartAddr(REG_ADDR(CAM_NR3D_BLEND))
    {
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_NR3D_BLEND);      // CAM+0x0CC0
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_CPX);    // CAM+0x0CD0
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_Y_CON1); // CAM+0x0CD4
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_Y_CON2); // CAM+0x0CD8
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_Y_CON3); // CAM+0x0CDC
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_U_CON1); // CAM+0x0CE0
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_U_CON2); // CAM+0x0CE4
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_U_CON3); // CAM+0x0CE8
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_V_CON1); // CAM+0x0CEC
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_V_CON2); // CAM+0x0CF0
        INIT_REG_INFO_ADDR(CAM_NR3D_LMT_V_CON3); // CAM+0x0CF4
    }

    virtual ~ISP_MGR_NR3D() {}

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

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);

} ISP_MGR_NR3D_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_NR3D_DEV : public ISP_MGR_NR3D_T
{
public:
    static
    ISP_MGR_NR3D_T&
    getInstance()
    {
        static ISP_MGR_NR3D_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_NR3D_DEV()
        : ISP_MGR_NR3D_T(eSensorDev)
    {}

    virtual ~ISP_MGR_NR3D_DEV() {}

};


#endif

