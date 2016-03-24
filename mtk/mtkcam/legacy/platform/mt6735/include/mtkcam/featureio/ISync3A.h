/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _AAA_N3D_SYNC_MGR_H_
#define _AAA_N3D_SYNC_MGR_H_

#include <semaphore.h>

#include <mtkcam/common.h>
#include <mtkcam/featureio/aaa_hal_common.h>

#include <utils/threads.h>
//#include <utils/List.h>

using namespace NSCam;
using namespace android;

namespace NS3A
{
/******************************************************************************
 *  ISync3A Interface.
 ******************************************************************************/
class ISync3A
{
public:
    /**
     * get singleton.
     */
    static ISync3A* getInstance(MINT32 i4Id=0);

    /**
     * Explicitly init 3A N3D Sync manager by MW.
     */
    virtual MBOOL init(MINT32 i4Policy, MINT32 i4Master, MINT32 i4Slave, const char* strName) = 0;

    /**
     * Explicitly uninit 3A N3D Sync manager by MW.
     */
    virtual MBOOL uninit() = 0;

    /**
     * Functions for 3A sync control
     */
    virtual MINT32 sync(MINT32 i4Sensor, MINT32 i4Param) = 0;

    virtual MVOID syncEnable(MBOOL fgOnOff) = 0;

    virtual MBOOL isSyncEnable() const = 0;

    virtual MINT32 getFrameCount() const = 0;

    virtual MBOOL chkMagicNum(MINT32 i4SensorDev, MINT32 i4MagicNum) = 0;

    virtual MVOID setChkSyncEnable(MBOOL fgOnOff) = 0;

    virtual MBOOL getChkSyncEnable() const = 0;

    virtual MBOOL isAeStable() const = 0;

    virtual MBOOL setDebugInfo(void* prDbgInfo) const = 0;

    enum
    {
        E_SYNC3A_DO_AE        = (1<<0),
        E_SYNC3A_DO_AE_PRECAP = (1<<1),
        E_SYNC3A_DO_AWB       = (1<<2),
        E_SYNC3A_BYP_AE       = (1<<3)
    };

protected:

    virtual ~ISync3A(){}
};

/******************************************************************************
 *  ISync3A Interface.
 ******************************************************************************/
class ISync3AMgr
{
public:
    enum
    {
        E_SYNC3AMGR_PRVIEW  = 0,
        E_SYNC3AMGR_CAPTURE = 1
    };

    enum E_SYNC3AMGR_CAPMODE
    {
        E_SYNC3AMGR_CAPMODE_3D = 0,
        E_SYNC3AMGR_CAPMODE_2D = 1
    };
    /**
     * get singleton.
     */
    static ISync3AMgr* getInstance();

    static ISync3A* getSync3A(MINT32 i4Id=0);

    /**
     * Explicitly init 3A N3D Sync manager by MW.
     */
    virtual MBOOL init(MINT32 i4Policy, MINT32 i4MasterIdx, MINT32 i4SlaveIdx) = 0;

    /**
     * Explicitly uninit 3A N3D Sync manager by MW.
     */
    virtual MBOOL uninit() = 0;

    virtual MBOOL isActive() const = 0;

    virtual MINT32 getMasterDev() const = 0;

    virtual MINT32 getSlaveDev() const = 0;

    virtual MVOID setCaptureMode(E_SYNC3AMGR_CAPMODE eCapMode) = 0;

    virtual E_SYNC3AMGR_CAPMODE getCaptureMode() const = 0;

    virtual MVOID setAFState(MINT32 i4AfState) = 0;

    virtual MINT32 getAFState() const = 0;

protected:
    virtual ~ISync3AMgr(){}
};

}

#endif /* _AAA_N3D_SYNC_MGR_H_ */

