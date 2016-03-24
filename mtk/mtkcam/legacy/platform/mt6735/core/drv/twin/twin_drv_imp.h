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
#ifndef _TWIN_DRV_IMP_H_
#define _TWIN_DRV_IMP_H_
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
// reg_4004
#define TWIN_CTL_EN1_TG1_BIT  (0)

// reg_4008
#define TWIN_CTL_EN2_G2C_BIT     (0)

// reg_400C
#define TWIN_CTL_DMA_IMGO_BIT  (0)
//
#define TWIN_CTL_BIT_EN1(_val_,_bit_)      ((_val_)<<TWIN_CTL_EN1_##_bit_##_BIT)
#define TWIN_CTL_BIT_EN2(_val_,_bit_)      ((_val_)<<TWIN_CTL_EN2_##_bit_##_BIT)
#define TWIN_CTL_BIT_DMA(_val_,_bit_)      ((_val_)<<TWIN_CTL_DMA_##_bit_##_BIT)
//
#define TWIN_DBG_BUFFER_NUM    (3)

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class TwinDrvImp : public TwinDrv
{
    public:

    protected:
        TwinDrvImp();
        ~TwinDrvImp();
    //
    public:
        static TwinDrv*  getInstance(void);
        virtual void    destroyInstance(void);
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        virtual MBOOL   configTwinPara(TwinDrvCfg* pTwinInfo);
        virtual MBOOL   runDbgTwinMain(void);
        MBOOL   runTwinMain(TwinDrvCfg* pTwinInfo);

    public://driver object operation
        inline virtual void*    getInCfg(void) {return (void*)&m_dual_isp_in_config;}
        inline virtual void*    getOutCfg(void) {return (void*)&m_dual_isp_out_config;}
        inline virtual MUINT32  calCoefStep(MUINT32 in, MUINT32 crop, MUINT32 crop_ofst, MUINT32 crop_sub_ofst, MUINT32 out);
        inline virtual MUINT32  getCoefTbl(MUINT32 resize_in, MUINT32 resize_out);
    //
    public:
        ISP_DUAL_IN_CONFIG_STRUCT  m_dual_isp_in_config;
        ISP_DUAL_OUT_CONFIG_STRUCT m_dual_isp_out_config;

    private:
        mutable Mutex       mLock;
        volatile MINT32     mInitCount;
        IMemDrv*        m_pImemDrv;
//        IMEM_BUF_INFO   m_twinDataInfo;
//        IMEM_BUF_INFO   m_WBInfo;
        IMEM_BUF_INFO   m_DbgTwinInfo[TWIN_DBG_BUFFER_NUM];

};

//-----------------------------------------------------------------------------
#endif  // _TWIN_DRV_IMP_H_

