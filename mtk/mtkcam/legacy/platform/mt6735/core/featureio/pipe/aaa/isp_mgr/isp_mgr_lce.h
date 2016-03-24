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
#ifndef _ISP_MGR_LCE_H_
#define _ISP_MGR_LCE_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_LCE : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LCE MyType;

private:
    ESensorTG_T const m_eSensorTG;
    MUINT32 m_u4StartAddr;
    MBOOL m_bEnable;
    MUINT32 mLcsOutWidth;
    MUINT32 mLcsOutHeight;
    MUINT32 mLceiBaseAddr;
    MUINT32 mAeStableCnt;
    MUINT32 mLceStrogest;
    MUINT32 mLceWeakest;
    MUINT32 mLcsFinalIdx;
    MUINT32 mLcsIdx;
    MINT32  mLvUpBound;     // 10-based
    MINT32  mLvLowBound;    // 10-based
    MUINT32 mLceQuaReg[11];
    MUINT32 mLceDgc1Reg[11];
    MUINT32 mLceDgc2Reg[11];

    enum
    {
        ERegInfo_CAM_LCE_CON,
        ERegInfo_CAM_LCE_ZR,
        ERegInfo_CAM_LCE_QUA,
        ERegInfo_CAM_LCE_DGC_1,
        ERegInfo_CAM_LCE_DGC_2,
        ERegInfo_CAM_LCE_GM,
        ERegInfo_CAM_LCE_SLM_LB,
        ERegInfo_CAM_LCE_SLM_SIZE,
        ERegInfo_CAM_LCE_OFST,
        ERegInfo_CAM_LCE_BIAS,
        ERegInfo_CAM_LCE_IMAGE_SIZE,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

    MVOID updateLceIdx();

protected:
    ISP_MGR_LCE(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_u4StartAddr(REG_ADDR(CAM_LCE_CON))
        , m_bEnable(MFALSE)
    {
        INIT_REG_INFO_ADDR(CAM_LCE_CON);        // 0x49C0
        INIT_REG_INFO_ADDR(CAM_LCE_ZR);         // 0x49C4
        INIT_REG_INFO_ADDR(CAM_LCE_QUA);        // 0x49C8
        INIT_REG_INFO_ADDR(CAM_LCE_DGC_1);      // 0x49CC
        INIT_REG_INFO_ADDR(CAM_LCE_DGC_2);      // 0x49D0
        INIT_REG_INFO_ADDR(CAM_LCE_GM);         // 0x49D4
        INIT_REG_INFO_ADDR(CAM_LCE_SLM_LB);     // 0x49E0
        INIT_REG_INFO_ADDR(CAM_LCE_SLM_SIZE);   // 0x49E4
        INIT_REG_INFO_ADDR(CAM_LCE_OFST);       // 0x49E8
        INIT_REG_INFO_ADDR(CAM_LCE_BIAS);       // 0x49EC
        INIT_REG_INFO_ADDR(CAM_LCE_IMAGE_SIZE); // 0x49F0

        INIT_REG_INFO_VALUE(CAM_LCE_CON,0xFE);
        INIT_REG_INFO_VALUE(CAM_LCE_QUA,0x201C7);       //default : level 9 : PA=71,PB=3, BA=2
        INIT_REG_INFO_VALUE(CAM_LCE_DGC_1,0x423D301C);  //default : DG_1=28,DG_2=48,DG_3=61,DG_4=66
        INIT_REG_INFO_VALUE(CAM_LCE_DGC_2,0x9A6144);    //default : DG_5=68,DG_6=97,DG_7=154
        INIT_REG_INFO_VALUE(CAM_LCE_GM,0x1080);         //default : GM_LB=128,GM_TH=16
        INIT_REG_INFO_VALUE(CAM_LCE_SLM_LB,0x20);       //default : SLM_LB=32
        INIT_REG_INFO_VALUE(CAM_LCE_OFST,0x0);          //default : 0
        INIT_REG_INFO_VALUE(CAM_LCE_BIAS,0x0);          //default : 0

        mAeStableCnt  = 0;
        mLcsOutWidth  = 0;
        mLcsOutHeight = 0;
        mLceiBaseAddr = 0;
        mLceStrogest  = 8;
        mLceWeakest   = 2;
        mLcsFinalIdx  = 8;
        mLcsIdx       = mLcsFinalIdx;
        mLvUpBound    = 80; // 10-based
        mLvLowBound   = 50; // 10-based

        //====== Tuning Table, Shoule Not Be Modified ======

        mLceQuaReg[0]  = 0;
        mLceQuaReg[1]  = 0x24F95;
        mLceQuaReg[2]  = 0x2431D;
        mLceQuaReg[3]  = 0x238A4;
        mLceQuaReg[4]  = 0x22DAB;
        mLceQuaReg[5]  = 0x225B0;
        mLceQuaReg[6]  = 0x21FB4;
        mLceQuaReg[7]  = 0x217B9;
        mLceQuaReg[8]  = 0x20FBE;
        mLceQuaReg[9]  = 0x20843;
        mLceQuaReg[10] = 0x201C7;

        mLceDgc1Reg[0]  = 0;
        mLceDgc1Reg[1]  = 0x31271F13;
        mLceDgc1Reg[2]  = 0x352B2115;
        mLceDgc1Reg[3]  = 0x372F2417;
        mLceDgc1Reg[4]  = 0x3A322818;
        mLceDgc1Reg[5]  = 0x3D352A18;
        mLceDgc1Reg[6]  = 0x3E372C1A;
        mLceDgc1Reg[7]  = 0x403A2E1C;
        mLceDgc1Reg[8]  = 0x423D301C;
        mLceDgc1Reg[9]  = 0x443F321E;
        mLceDgc1Reg[10] = 0x4641341F;

        mLceDgc2Reg[0]  = 0;
        mLceDgc2Reg[1]  = 0x9A6140;
        mLceDgc2Reg[2]  = 0x9A6140;
        mLceDgc2Reg[3]  = 0x9A6140;
        mLceDgc2Reg[4]  = 0x9A6140;
        mLceDgc2Reg[5]  = 0x9A6140;
        mLceDgc2Reg[6]  = 0x9A6140;
        mLceDgc2Reg[7]  = 0x9A6142;
        mLceDgc2Reg[8]  = 0x9A6144;
        mLceDgc2Reg[9]  = 0x9A6146;
        mLceDgc2Reg[10] = 0x9A6148;


    }

    virtual ~ISP_MGR_LCE() {}

public:
    static MyType &getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1);

public: // Interfaces

    template <class ISP_xxx_T>
    MyType &put(ISP_xxx_T const &rParam);

    template <class ISP_xxx_T>
    MyType &get(ISP_xxx_T &rParam);

    MVOID setLceEnable(MBOOL bEnable);

    MBOOL isEnable()
    {
        return m_bEnable;
    }

    MVOID setLcsoSize(MUINT32 width, MUINT32 height);

    MVOID setLceiAddr(MUINT32 addr);

    MVOID setLceCurLv(MINT32 aCurLv);

    MVOID setAeStableCnt(MUINT32 cnt);

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_LCE_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_LCE_DEV : public ISP_MGR_LCE_T
{
public:
    static ISP_MGR_LCE_T &getInstance()
    {
        static ISP_MGR_LCE_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_LCE_DEV() : ISP_MGR_LCE_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_LCE_DEV() {}
};



#endif

