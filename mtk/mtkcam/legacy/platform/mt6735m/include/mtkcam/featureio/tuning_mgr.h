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
#ifndef _TUNING_MGR_H_
#define _TUNING_MGR_H_
//-----------------------------------------------------------------------------

#include <mtkcam/common.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/drv_FrmB/isp_drv_FrmB.h>



//-----------------------------------------------------------------------------
using namespace NSCam;
using namespace android;
using namespace NSIspDrv_FrmB;
//-----------------------------------------------------------------------------

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/



#define TUNING_MGR_WRITE_ENABLE_BITS(Scenario,RegName,FieldName,FieldValue)                                                                             \
do{                                                                                                                                                     \
    MUINT32 addrOffset=(MUINT8*)(&(TuningMgr::getInstance().mpIspReferenceRegMap->RegName))-(MUINT8*)(TuningMgr::getInstance().mpIspReferenceRegMap);   \
    pthread_mutex_lock(&mQueueTopAccessLock);                                                                                                           \
    TuningMgr::getInstance().mpIspReferenceRegMap->RegName.Raw=TuningMgr::getInstance().tuningMgrReadReg(Scenario,addrOffset);                          \
    TuningMgr::getInstance().mpIspReferenceRegMap->RegName.Bits.FieldName = FieldValue;                                                                 \
    TuningMgr::getInstance().tuningMgrWriteReg(Scenario,addrOffset,TuningMgr::getInstance().mpIspReferenceRegMap->RegName.Raw);                         \
    pthread_mutex_unlock(&mQueueTopAccessLock);                                                                                                         \
}while(0);


#define TUNING_MGR_WRITE_REG(Scenario,RegName,Value)                                                                                                    \
do{                                                                                                                                                     \
    MUINT32 addrOffset=(MUINT8*)(&(TuningMgr::getInstance().mpIspReferenceRegMap->RegName))-(MUINT8*)(TuningMgr::getInstance().mpIspReferenceRegMap);   \
    TuningMgr::getInstance().tuningMgrWriteReg(Scenario,addrOffset, Value);                                                                             \
}while(0);




/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
    enum ETuningMgrFunc  // need mapping to EIspTuningMgrFunc
    {
       eTuningMgrFunc_Null          = NSIspDrv_FrmB::eIspTuningMgrFunc_Null      ,
       eTuningMgrFunc_Obc           = NSIspDrv_FrmB::eIspTuningMgrFunc_Obc       ,
       eTuningMgrFunc_Lsc           = NSIspDrv_FrmB::eIspTuningMgrFunc_Lsc       ,
       eTuningMgrFunc_Bnr           = NSIspDrv_FrmB::eIspTuningMgrFunc_Bnr       ,
       eTuningMgrFunc_Num           = NSIspDrv_FrmB::eIspTuningMgrFunc_Num       ,
    };



typedef struct
{
    MUINT32     Addr;
    MUINT32     Data;
}TUNING_MGR_REG_IO_STRUCT;


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern pthread_mutex_t mQueueTopAccessLock;


/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class TuningMgr
{
    public:

    protected:
        TuningMgr() {};
        virtual ~TuningMgr() {};
    //
    public:
        static TuningMgr& getInstance();
        virtual int     init() = 0;
        virtual int     uninit() = 0;
        //
        virtual MBOOL updateEngine(ESoftwareScenario scenario, ETuningMgrFunc engin) = 0;
        virtual MBOOL dequeBuffer(ESoftwareScenario scenario, MINT32 MagicNum) = 0;
        virtual MBOOL enqueBuffer(ESoftwareScenario scenario, MINT32 MagicNum) = 0;
        virtual MBOOL flushSetting(ESoftwareScenario scenario) = 0;
        virtual MBOOL byPassSetting(ESoftwareScenario scenario, MINT32 MagicNum) = 0;

    public:
        virtual MBOOL tuningMgrWriteRegs(ESoftwareScenario scenario, TUNING_MGR_REG_IO_STRUCT*  pRegIo , MINT32 cnt) = 0;
        virtual MBOOL tuningMgrWriteReg(ESoftwareScenario scenario, MUINT32 addr, MUINT32 data) = 0;
        virtual MUINT32 tuningMgrReadReg(ESoftwareScenario scenario,MUINT32 addr) = 0;

    public:
        //for calculating offset in register read/write macro
        static isp_reg_t*      mpIspReferenceRegMap;
        mutable Mutex       mQueueTopAccessLock;
};


//-----------------------------------------------------------------------------
#endif  // _TUNING_MGR_H_

