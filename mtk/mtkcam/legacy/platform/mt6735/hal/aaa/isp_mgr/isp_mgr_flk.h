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
#ifndef _ISP_MGR_FLK_H_
#define _ISP_MGR_FLK_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FLK statistics
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_FLK_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_FLK_CONFIG    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4770

    enum
    {
        ERegInfo_CAM_FLK_CON,
        ERegInfo_CAM_CTL_EN1,
        ERegInfo_CAM_CTL_EN1_SET,
        ERegInfo_CAM_CTL_DMA_EN,
        ERegInfo_CAM_CTL_DMA_EN_SET,
        ERegInfo_CAM_CTL_DMA_INT,
        ERegInfo_CAM_CTL_INT_EN,
        ERegInfo_CAM_FLK_WNUM,
        ERegInfo_CAM_FLK_SOFST,
        ERegInfo_CAM_FLK_WSIZE,
        ERegInfo_CAM_ESFKO_XSIZE,
        ERegInfo_CAM_ESFKO_YSIZE,
        ERegInfo_CAM_ESFKO_STRIDE,
        ERegInfo_CAM_ESFKO_OFST_ADDR,
        ERegInfo_CAM_ESFKO_BASE_ADDR,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_FLK_CONFIG()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, ESensorDev_Main) // FIXME
#if 0 //remove later for migration
        , m_u4StartAddr(REG_ADDR(CAM_FLK_CON))
#endif
    {
#if 0 //remove later for migration
        INIT_REG_INFO_ADDR(CAM_FLK_CON);          // 0x4770
        INIT_REG_INFO_ADDR(CAM_CTL_EN1);          // 0x4004
        INIT_REG_INFO_ADDR(CAM_CTL_EN1_SET);      // 0x4080
        INIT_REG_INFO_ADDR(CAM_CTL_DMA_EN);       // 0x400C
        INIT_REG_INFO_ADDR(CAM_CTL_DMA_EN_SET);   //0x 4090
        INIT_REG_INFO_ADDR(CAM_CTL_DMA_INT);      // 0x4028
        INIT_REG_INFO_ADDR(CAM_CTL_INT_EN);      // 0x4020
        INIT_REG_INFO_ADDR(CAM_FLK_WNUM);         // 0x477C
        INIT_REG_INFO_ADDR(CAM_FLK_SOFST);        // 0x4774
        INIT_REG_INFO_ADDR(CAM_FLK_WSIZE);        // 0x4778
        INIT_REG_INFO_ADDR(CAM_ESFKO_XSIZE);      // 0x4370
        INIT_REG_INFO_ADDR(CAM_ESFKO_YSIZE);      // 0x4378
        INIT_REG_INFO_ADDR(CAM_ESFKO_STRIDE);     // 0x437C
        INIT_REG_INFO_ADDR(CAM_ESFKO_OFST_ADDR);  // 0x4374
        INIT_REG_INFO_ADDR(CAM_ESFKO_BASE_ADDR);  // 0x436C
#endif

    }

    virtual ~ISP_MGR_FLK_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //    Interfaces

    MBOOL apply();
    MVOID enableFlk(MBOOL enable);
    MVOID SetFLKWin(MINT32 offsetX,MINT32 offsetY , MINT32 sizeX ,MINT32 sizeY);
    MVOID SetFKO_DMA_Addr(MINT32 address,MINT32 size);

}ISP_MGR_FLK_CONFIG_T;
template <ESensorDev_T const eSensorDev>
class ISP_MGR_FLK_DEV : public ISP_MGR_FLK_CONFIG_T
{
public:
    static
    ISP_MGR_FLK_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_FLK_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_FLK_DEV()
        : ISP_MGR_FLK_CONFIG_T()
    {}

    virtual ~ISP_MGR_FLK_DEV() {}

};


#endif

