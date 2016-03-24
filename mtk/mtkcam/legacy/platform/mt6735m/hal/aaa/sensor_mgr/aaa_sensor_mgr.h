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
#ifndef _AAA_SENSOR_MGR_H_
#define _AAA_SENSOR_MGR_H_

#include <mtkcam/hal/IHalSensor.h>
#include <utils/Mutex.h>

namespace NS3Av3
{

using namespace NSCam;

typedef struct
{
  MUINT16 u2SensorPreviewWidth;
  MUINT16 u2SensorPreviewHeight;
  MUINT16 u2SensorFullWidth;
  MUINT16 u2SensorFullHeight;
  MUINT16 u2SensorVideoWidth;
  MUINT16 u2SensorVideoHeight;
  MUINT32 u2SensorVideo1Width;
  MUINT32 u2SensorVideo1Height;
  MUINT32 u2SensorVideo2Width;
  MUINT32 u2SensorVideo2Height;
} SENSOR_RES_INFO_T;

typedef struct
{
  MUINT16 u2VCModeSelect;                // 0 : Auto, 1 : Manual
  MUINT16 u2VCShutterRatio;              // 1/1, 1/2, 1/4, 1/8
  MUINT16 u2VCOBvalue;                     // OB value
  MUINT16 u2VCStatWidth;
  MUINT16 u2VCStatHeight;
} SENSOR_VC_INFO_T;


/*******************************************************************************
*
*******************************************************************************/
class AAASensorMgr
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AAASensorMgr(AAASensorMgr const&);
    //  Copy-assignment operator is disallowed.
    AAASensorMgr& operator=(AAASensorMgr const&);

public:  ////
    AAASensorMgr();
    ~AAASensorMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AAASensorMgr& getInstance();
    MRESULT init(MINT32 const i4SensorIdx);
    MRESULT uninit();
    MRESULT getSensorSyncinfo(MINT32 i4SensorDev, MINT32 *i4SutterDelay, MINT32 *i4SensorGainDelay, MINT32 *i4IspGainDelay, MINT32 *i4SensorModeDelay);
    MRESULT getSensorWidthHeight(MINT32 i4SensorDev, SENSOR_RES_INFO_T* a_rSensorResolution);
    MRESULT getSensorMaxFrameRate(MINT32 i4SensorDev, MUINT32 rSensorMaxFrmRate[NSIspTuning::ESensorMode_NUM]);
    MRESULT setSensorExpTime(MINT32 i4SensorDev, MUINT32 a_u4ExpTime);
    MRESULT setSensorGain(MINT32 i4SensorDev, MUINT32 a_u4SensorGain);
    MRESULT setSensorFrameRate(MINT32 i4SensorDev, MUINT32 a_u4SensorFrameRate);
    MRESULT setSensorExpLine(MINT32 i4SensorDev, MUINT32 a_u4ExpLine);
    MRESULT setFlickerFrameRateActive(MINT32 i4SensorDev, MBOOL a_bFlickerFPSAvtive);
    MRESULT setPreviewParams(MINT32 i4SensorDev, MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain);
    MRESULT setCaptureParams(MINT32 i4SensorDev, MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain);
    MRESULT setPreviewLineBaseParams(MINT32 i4SensorDev, MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain);
    MRESULT setCaptureLineBaseParams(MINT32 i4SensorDev, MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain);

    MRESULT setPreviewMaxFrameRate(MINT32 i4SensorDev, MUINT32 frameRate, MUINT32 u4SensorMode);
    MUINT32 getPreviewDefaultFrameRate(MINT32 i4SensorDev, MUINT32 u4SensorMode);
    MRESULT set2ShutterParams(MINT32 i4SensorDev, MUINT32 a_u4LEExpTime, MUINT32 a_u4SEExpTime, MUINT32 a_u4SensorGain);
    MRESULT setSensorAWBGain(MINT32 i4SensorDev, strSensorAWBGain *strSensorAWBInfo);
    MRESULT getSensorVCinfo(MINT32 i4SensorDev, SENSOR_VC_INFO_T *a_rSensorVCInfo);
    MRESULT getSensorModeDelay(MINT32 i4SensorMode, MINT32 *i4SensorModeDelay);
    MRESULT setSensorOBLock(MINT32 i4SensorDev, MBOOL bLockSensorOB);
    MRESULT getRollingShutter(MINT32 i4SensorDev, MUINT32& tline, MUINT32& vsize) const;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IHalSensor *m_pHalSensorObj;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL    m_bDebugEnable;
    MBOOL    m_bFlickerState;
    MUINT32 m_u4SensorFrameRate;
    MINT32 m_pSensorModeDelay[11];
};

};  //  namespace NS3Av3
#endif // _AAA_SENSOR_MGR_H_

