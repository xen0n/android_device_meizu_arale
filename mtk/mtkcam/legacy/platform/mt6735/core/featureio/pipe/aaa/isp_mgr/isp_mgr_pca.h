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
#ifndef _ISP_MGR_PCA_H_
#define _ISP_MGR_PCA_H_

#include <aaa_types.h>
#include <aaa_log.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PCA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_PCA : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_PCA    MyType;
private:
    EPCAMode_T const m_ePCAMode;
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0x1E00

    enum
    {
        ERegInfo_CAM_PCA_CON1,
        ERegInfo_CAM_PCA_CON2,
        ERegInfo_PCA_CON_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_PCA_CON_NUM]; // PCA_CON
    RegInfo_T m_rIspRegInfo_PCA_LUT[PCA_BIN_NUM*2]; // compatible with 360BIN

protected:
    ISP_MGR_PCA(ESensorDev_T const eSensorDev, EPCAMode_T const ePCAMode)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_PCA_CON_NUM, m_u4StartAddr, eSensorDev)
        , m_ePCAMode(ePCAMode)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_PCA_CON1))
    {
        // PCA_CON
        INIT_REG_INFO_ADDR(CAM_PCA_CON1); // CAM+0x1E00
        INIT_REG_INFO_ADDR(CAM_PCA_CON2); // CAM+0x1E04

        // PCA_LUT
        MUINT32 u4StartAddr = REG_ADDR(CAM_PCA_TBL[0]);
        MINT32 i4PCA_BIN_NUM = (m_ePCAMode == EPCAMode_180BIN) ? PCA_BIN_NUM : PCA_BIN_NUM*2;
        //MY_LOG("i4PCA_BIN_NUM = %d", i4PCA_BIN_NUM);

        for (MINT32 i = 0; i < i4PCA_BIN_NUM; i++) {
            m_rIspRegInfo_PCA_LUT[i].addr = u4StartAddr + 4*i;
            //MY_LOG("m_rIspRegInfo_PCA_LUT[%d].addr = 0x%8x", i, m_rIspRegInfo_PCA_LUT[i].addr);
        }
    }

    virtual ~ISP_MGR_PCA() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev, EPCAMode_T const ePCAMode = EPCAMode_180BIN);

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

    inline MVOID loadLut(MUINT32* pPcaLut)
    {
        MINT32 i4PCA_BIN_NUM = (m_ePCAMode == EPCAMode_180BIN) ? PCA_BIN_NUM : PCA_BIN_NUM*2;
        //MY_LOG("i4PCA_BIN_NUM = %d", i4PCA_BIN_NUM);

        for (MINT32 i = 0; i < i4PCA_BIN_NUM; i++) {
            m_rIspRegInfo_PCA_LUT[i].val = pPcaLut[i];
        }
    }

    inline MVOID getLut(MUINT32* pPcaLut)
    {
        MINT32 i4PCA_BIN_NUM = (m_ePCAMode == EPCAMode_180BIN) ? PCA_BIN_NUM : PCA_BIN_NUM*2;
        //MY_LOG("i4PCA_BIN_NUM = %d", i4PCA_BIN_NUM);

        for (MINT32 i = 0; i < i4PCA_BIN_NUM; i++) {
            pPcaLut[i] = m_rIspRegInfo_PCA_LUT[i].val;
        }
    }

} ISP_MGR_PCA_T;

template <ESensorDev_T const eSensorDev, EPCAMode_T const ePCAMode = EPCAMode_180BIN>
class ISP_MGR_PCA_DEV : public ISP_MGR_PCA_T
{
public:
    static
    ISP_MGR_PCA_T&
    getInstance()
    {
        static ISP_MGR_PCA_DEV<eSensorDev, ePCAMode> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_PCA_DEV()
        : ISP_MGR_PCA_T(eSensorDev, ePCAMode)
    {}

    virtual ~ISP_MGR_PCA_DEV() {}

};


#endif

