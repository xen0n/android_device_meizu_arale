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
#ifndef _ISP_MGR_GGM_H_
#define _ISP_MGR_GGM_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_GGM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_GGM    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0x1480

    enum
    {
        ERegInfo_CAM_GGM_CTRL,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo_GGM_RB[GGM_LUT_SIZE];
    RegInfo_T m_rIspRegInfo_GGM_G[GGM_LUT_SIZE];

protected:
    ISP_MGR_GGM(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_GGM_CTRL))
    {
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_GGM_CTRL); // CAM+0x1480

        // CAM_GGM_RB_GMT
        MUINT32 u4StartAddr = REG_ADDR(CAM_GGM_LUT_RB[0]);
        for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
            m_rIspRegInfo_GGM_RB[i].addr = u4StartAddr + 4*i;
            //MY_LOG("m_rIspRegInfo_GGM_RB[%d].addr = 0x%8x", i, m_rIspRegInfo_GGM_RB[i].addr);
        }

        // CAM_GGM_G_GMT
        u4StartAddr = REG_ADDR(CAM_GGM_LUT_G[0]);
        for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
            m_rIspRegInfo_GGM_G[i].addr = u4StartAddr + 4*i;
            //MY_LOG("m_rIspRegInfo_GGM_G[%d].addr = 0x%8x", i, m_rIspRegInfo_GGM_G[i].addr);
        }
    }

    virtual ~ISP_MGR_GGM() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

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

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_GGM_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_GGM_DEV : public ISP_MGR_GGM_T
{
public:
    static
    ISP_MGR_GGM_T&
    getInstance()
    {
        static ISP_MGR_GGM_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_GGM_DEV()
        : ISP_MGR_GGM_T(eSensorDev)
    {}

    virtual ~ISP_MGR_GGM_DEV() {}

};


#endif

