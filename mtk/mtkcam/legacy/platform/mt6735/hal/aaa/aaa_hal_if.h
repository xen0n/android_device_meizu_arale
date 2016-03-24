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

#ifndef _AAA_HAL_IF_H_
#define _AAA_HAL_IF_H_

#include <utils/threads.h>
#include <utils/List.h>

#include <mtkcam/common.h>
//#include <mtkcam/exif/IDbgInfoContainer.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
#include "aaa_result.h"

using namespace NSCam;
using namespace android;
using namespace NS3Av3;

//class IBaseCamExif;

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void UtilConvertCamInfo(const NSIspTuning::RAWIspCamInfo_U& rCamInfoU, NSIspTuning::RAWIspCamInfo& rCamInfoT);
void UtilConvertCamInfo(const NSIspTuning::RAWIspCamInfo& rCamInfoT, NSIspTuning::RAWIspCamInfo_U& rCamInfoU);

class I3ACallBack
{
public:

    virtual             ~I3ACallBack() {}

public:

    virtual void        doNotifyCb (
                           MINT32  _msgType,
                           MINTPTR _ext1,
                           MINTPTR _ext2,
                           MINTPTR _ext3
                        ) = 0;

    virtual void        doDataCb (
                           int32_t _msgType,
                           void*   _data,
                           uint32_t _size
                        ) = 0;
public:

    enum ECallBack_T
    {
        eID_NOTIFY_AF_FOCUSED,
        eID_NOTIFY_AF_MOVING,
        eID_DATA_AF_FOCUSED,
        eID_NOTIFY_3APROC_FINISH,
        eID_NOTIFY_VSYNC_DONE,
        eID_NOTIFY_READY2CAP,
        eID_NOTIFY_CURR_RESULT,
        eID_MSGTYPE_NUM
    };

    enum E3APROC_FINISH_BIT
    {
        e3AProcOK           = 0,
        e3APvInitReady    = 1,
        e3AProcNum
    };
};

class Hal3AIf
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3AIf() {}
    virtual ~Hal3AIf() {}

private: // disable copy constructor and copy assignment operator
    Hal3AIf(const Hal3AIf&);
    Hal3AIf& operator=(const Hal3AIf&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    static Hal3AIf* createInstance(MINT32 const i4SensorOpenIndex);
    virtual MVOID destroyInstance() {}
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg = 0) {return MTRUE;}
    virtual MBOOL setParams(Param_T const &rNewParam) {return MTRUE;}
    virtual MINT32 getResult(MUINT32 u4FrmId, Result_T& rResult) {return 0;}
    virtual MINT32 getResultCur(MUINT32 u4FrmId, Result_T& rResult) {return 0;}
    virtual MBOOL autoFocus() {return MTRUE;}
    virtual MBOOL cancelAutoFocus() {return MTRUE;}
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height) {return MTRUE;}
    virtual MBOOL getDebugInfo(android::Vector<MINT32>& keys, android::Vector< android::Vector<MUINT8> >& data) const {return MTRUE;}
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const {return 0;}
    virtual MINT32 attachCb(I3ACallBack* cb) {return 0;}
    virtual MINT32 detachCb(I3ACallBack* cb) {return 0;}
    virtual MBOOL setIspPass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf) {return MTRUE;}
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo) {return 0;}
    virtual MINT32 isNeedFiringFlash(MBOOL bEnCal) {return 0;}
    virtual MVOID setSensorMode(MINT32 i4SensorMode) {}
    virtual MVOID setFrameId(MINT32 i4FrmId) {}
    virtual MINT32 getFrameId() const {return 0;}
    virtual MUINT32 getSensorDev() const {return 0;}
    virtual MVOID setFDEnable(MBOOL fgEnable){}
    virtual MBOOL setFDInfo(MVOID* prFaces){return 0;}
    virtual MVOID notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg = 0){}

    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2) {return 0;}
};
}; // namespace NS3Av3

#endif

