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
#define LOG_TAG "ispdrv_mgr_v3"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <aaa_error_code.h>
#include <mtkcam/drv_common/isp_reg.h>
#include <mtkcam/drv_FrmB/isp_drv_FrmB.h>
#include "ispdrv_mgr.h"

using namespace android;
using namespace NS3Av3;
using namespace NSIspDrv_FrmB;

/*******************************************************************************
* ISP Driver Manager Context
*******************************************************************************/
class IspDrvMgrCtx_v3 : public IspDrvMgr
{
    friend  IspDrvMgr& IspDrvMgr::getInstance();
protected:  ////    Data Members.
    IspDrv*         m_pIspDrv;
    isp_reg_t*      m_pIspReg;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;

private:    ////    Ctor/Dtor
    IspDrvMgrCtx_v3();
    ~IspDrvMgrCtx_v3();

public:     ////    Interfaces.
    virtual void*           getIspReg() const;
    virtual MBOOL           readRegs(ISPREG_INFO_T*const pRegInfos, MUINT32 const count);
    virtual MBOOL           writeRegs(ISPREG_INFO_T*const pRegInfos, MUINT32 const count);
    virtual MERROR_ENUM_T   init();
    virtual MERROR_ENUM_T   uninit();
    //virtual MERROR_ENUM_T   reinit();

};


IspDrvMgr&
IspDrvMgr::
getInstance()
{
    static IspDrvMgrCtx_v3 singleton;
    return singleton;
}


IspDrvMgrCtx_v3::
IspDrvMgrCtx_v3()
    : IspDrvMgr()
    , m_pIspDrv(MNULL)
    , m_pIspReg(MNULL)
    , m_Users(0)
    , m_Lock()
{
}


IspDrvMgrCtx_v3::
~IspDrvMgrCtx_v3()
{
}


IspDrvMgr::MERROR_ENUM_T
IspDrvMgrCtx_v3::
init()
{
    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return IspDrvMgr::MERR_OK;
    }

    // for ISPDRV_MODE_ISP
    m_pIspDrv = IspDrv::createInstance();
    if (!m_pIspDrv) {
        MY_ERR("IspDrv::createInstance() fail \n");
        return IspDrvMgr::MERR_BAD_ISP_DRV;
    }

    if (FAILED(m_pIspDrv->init(LOG_TAG))) {
        MY_ERR("pIspDrv->init() fail \n");
        return IspDrvMgr::MERR_BAD_ISP_DRV;
    }

    m_pIspReg = (isp_reg_t*)m_pIspDrv->getRegAddr();

    android_atomic_inc(&m_Users);

    return IspDrvMgr::MERR_OK;
}


IspDrvMgr::MERROR_ENUM_T
IspDrvMgrCtx_v3::
uninit()
{
    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return IspDrvMgr::MERR_OK;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        if (m_pIspDrv) {
            if (FAILED(m_pIspDrv->uninit(LOG_TAG))) {
                MY_ERR("m_pIspDrv->uninit() fail \n");
                return IspDrvMgr::MERR_BAD_ISP_DRV;
            }
        }

        m_pIspReg = MNULL;
        m_pIspDrv = MNULL;
    }
    else    // There are still some users.
    {
        MY_LOG("Still %d users \n", m_Users);
    }

    return IspDrvMgr::MERR_OK;
}

/*
IspDrvMgr::MERROR_ENUM_T
IspDrvMgrCtx::
reinit()
{
    IspDrvMgr::MERROR_ENUM_T err = IspDrvMgr::MERR_OK;

    int const ec = m_pIspDrv->sendCommand   (
        CMD_GET_ISP_ADDR, (int)&m_pIspReg
    );
    if  ( ec < 0 || ! m_pIspReg )
    {
        err = IspDrvMgr::MERR_BAD_ISP_ADDR;
        MY_LOG(
            "[reinit][IspDrv][CMD_GET_ISP_ADDR]"
            "(m_pIspDrv, m_pIspReg, ec)=(%p, %p, %d)"
            , m_pIspDrv, m_pIspReg, ec
        );
        goto lbExit;
    }

    err = IspDrvMgr::MERR_OK;
lbExit:
    return  err;
}
*/
void* IspDrvMgrCtx_v3::getIspReg() const
{
    return (void*) m_pIspReg;
}

MBOOL
IspDrvMgrCtx_v3::
readRegs(ISPREG_INFO_T*const pRegInfos, MUINT32 const count)
{
    if  (! m_pIspDrv)
        return  MFALSE;
    return (m_pIspDrv->readRegs(reinterpret_cast<ISP_DRV_REG_IO_STRUCT*>(pRegInfos), count) < 0) ? MFALSE : MTRUE;
}


MBOOL
IspDrvMgrCtx_v3::
writeRegs(ISPREG_INFO_T*const pRegInfos, MUINT32 const count)
{
    MBOOL fgRet = MTRUE;

    if  (! m_pIspDrv)
        return  MFALSE;
    fgRet = m_pIspDrv->writeRegs(reinterpret_cast<ISP_DRV_REG_IO_STRUCT*>(pRegInfos), count);

    return fgRet;
}

