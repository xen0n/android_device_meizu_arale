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
#define LOG_TAG "aaa_sensor_buf_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#define ENABLE_AE_MVHDR_STAT       (1)

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <cutils/atomic.h>

#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <awb_tuning_custom.h>
#include <ispdrv_mgr.h>
#include <mtkcam/featureio/tuning_mgr.h>
#include <isp_tuning.h>
#include <linux/cache.h>
#include <utils/threads.h>
#include <list>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include "aaa_sensor_buf_mgr.h"
#include <sys/mman.h>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/imageio/ispio_utility.h>
#include <core/iopipe/CamIO/PortMap.h>
#include "kd_imgsensor.h"

using namespace std;
using namespace android;
using namespace NS3A;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSCam::NSIoPipe;

typedef list<BufInfo_T> BufInfoList_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AAO buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AAO_OFFSET_ADDR (0)
#define AAO_YSIZE (0)
#define AAO_STRIDE_BUS_SIZE (3)

#define MAX_AAO_BUFFER_CNT (2)



typedef struct
{
    MUINT8                 m_u1NumBit; //8 bit:3, 10bit:2, 12bit:1, 14bit:0
    MUINT32                m_u4AAOBufSize;
    MUINT32                m_u4AAOXSize;
    MUINT32                m_u4AAOYSize;
    BufInfoList_T          m_rHwBufList;
    BufInfo_T              m_rAAOBufInfo[MAX_AAO_BUFFER_CNT];
} SAE_BUF_CTRL_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AEBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    AEBufMgr(AEBufMgr const&);
    //  Copy-assignment operator is disallowed.
    AEBufMgr& operator=(AEBufMgr const&);

public:
    AEBufMgr(ESensorDev_T const eSensorDev);
    ~AEBufMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL debugPrint();
    MBOOL allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize);
    MBOOL newBuf();
    MBOOL freeBuf(BufInfo_T &rBufInfo);
    MBOOL deleteBuf();

public:
    static AEBufMgr& getInstance(MINT32 const i4SensorDev);
    MBOOL AAStatEnable(MBOOL En);
    MBOOL init(MINT32 const i4SensorIdx);
    MBOOL uninit();
    MBOOL DMAInit();
    MBOOL DMAUninit();
    MBOOL enqueueHwBuf(BufInfo_T& rBufInfo, MINT32 PDSel=0);
    MBOOL dequeueHwBuf(BufInfo_T& rBufInfo, MINT32 i4PDSel=0, MUINT32 i4FrmNum=0);
    MBOOL updateDMABaseAddr(MUINT32 u4BaseAddr, MINT32 PDSel=0);
    MUINT32 getCurrHwBuf();
    MUINT32 getNextHwBuf(MINT32 PDSel=0);


    inline MBOOL sendCommandNormalPipe(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
    {
        INormalPipe* pPipe = INormalPipe::createInstance(m_i4SensorIdx, "aao_buf_mgr");
        MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
        pPipe->destroyInstance("aao_buf_mgr");
        return fgRet;
    }

    inline MBOOL setTGInfo(MINT32 const i4TGInfo)
    {
        MY_LOG("[%s()] i4TGInfo: %d \n", __FUNCTION__, i4TGInfo);

        switch (i4TGInfo)
        {
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            break;
        default:
          MY_ERR("i4TGInfo = %d", i4TGInfo);
            return MFALSE;
        }

        return MTRUE;
    }


    inline MBOOL setSensorMode(MINT32 i4NewSensorMode)
    {
        MY_LOG("[%s()] m_eSensorDev: %d Sensor mode: %d \n", __FUNCTION__, m_eSensorDev, i4NewSensorMode);

        m_SensorMode = i4NewSensorMode;

        return MTRUE;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    ESensorDev_T const     m_eSensorDev;
    ESensorTG_T            m_eSensorTG;
    IMemDrv*               m_pIMemDrv;
    IspDrv*                m_pIspDrv;
    MUINT32                m_u4AEStateSize;
    MUINT32                m_u4AEHistSize;
    MUINT32                m_u4AWBStateSize;
    MUINT                  m_pixelMode;
  SAE_BUF_CTRL_T     m_PDBufCtrl;
  SAE_BUF_CTRL_T     m_MVHDRBufCtrl;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
  MBOOL                  m_EnMVHDR;
  MBOOL                  m_EnPDAF;
    MBOOL                  m_bDebugEnable;
    MINT32                 m_i4SensorIdx;
    INormalPipe*           m_pPipe;
    unsigned long *mpIspHwRegAddr;
    int mfd;
  MINT32                 m_i4CurrSensorId;
  MINT32             m_SensorMode;
  MBOOL                  m_FirstTimeCreateBuf;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AEBufMgrDev : public AEBufMgr
{
public:
    static
    AEBufMgr&
    getInstance()
    {
        static AEBufMgrDev<eSensorDev> singleton;
        return singleton;
    }

    AEBufMgrDev()
        : AEBufMgr(eSensorDev)
    {}

    virtual ~AEBufMgrDev() {}
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AEBufMgr::getInstance(ESensorDev_Main).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AEBufMgr::getInstance(ESensorDev_Sub).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AEBufMgr::getInstance(ESensorDev_MainSecond).init(i4SensorIdx);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
uninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
       ret_main = AEBufMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
       ret_sub = AEBufMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
       ret_main2 = AEBufMgr::getInstance(ESensorDev_MainSecond).uninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
DMAInit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AEBufMgr::getInstance(ESensorDev_Main).DMAInit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AEBufMgr::getInstance(ESensorDev_Sub).DMAInit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AEBufMgr::getInstance(ESensorDev_MainSecond).DMAInit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
DMAUninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AEBufMgr::getInstance(ESensorDev_Main).DMAUninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AEBufMgr::getInstance(ESensorDev_Sub).DMAUninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AEBufMgr::getInstance(ESensorDev_MainSecond).DMAUninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
AAStatEnable(MINT32 const i4SensorDev, MBOOL En)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AEBufMgr::getInstance(ESensorDev_Main).AAStatEnable(En);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AEBufMgr::getInstance(ESensorDev_Sub).AAStatEnable(En);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AEBufMgr::getInstance(ESensorDev_MainSecond).AAStatEnable(En);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
enqueueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo, MINT32 PDSel)
{
    if (i4SensorDev & ESensorDev_Main)
        return AEBufMgr::getInstance(ESensorDev_Main).enqueueHwBuf(rBufInfo, PDSel);
    else if (i4SensorDev & ESensorDev_Sub)
        return AEBufMgr::getInstance(ESensorDev_Sub).enqueueHwBuf(rBufInfo, PDSel);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AEBufMgr::getInstance(ESensorDev_MainSecond).enqueueHwBuf(rBufInfo, PDSel);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
dequeueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo, MINT32 i4PDSel, MUINT32 i4FrmNum)
{
    if (i4SensorDev & ESensorDev_Main)
        return AEBufMgr::getInstance(ESensorDev_Main).dequeueHwBuf(rBufInfo, i4PDSel, i4FrmNum);
    else if (i4SensorDev & ESensorDev_Sub)
        return AEBufMgr::getInstance(ESensorDev_Sub).dequeueHwBuf(rBufInfo, i4PDSel, i4FrmNum);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AEBufMgr::getInstance(ESensorDev_MainSecond).dequeueHwBuf(rBufInfo, i4PDSel, i4FrmNum);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
updateDMABaseAddr(MINT32 const i4SensorDev, MINT32 PDSel)
{
    if (i4SensorDev & ESensorDev_Main)
        return AEBufMgr::getInstance(ESensorDev_Main).updateDMABaseAddr(AEBufMgr::getInstance(ESensorDev_Main).getNextHwBuf(PDSel), PDSel);
    else if (i4SensorDev & ESensorDev_Sub)
        return AEBufMgr::getInstance(ESensorDev_Sub).updateDMABaseAddr(AEBufMgr::getInstance(ESensorDev_Sub).getNextHwBuf(PDSel), PDSel);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AEBufMgr::getInstance(ESensorDev_MainSecond).updateDMABaseAddr(AEBufMgr::getInstance(ESensorDev_MainSecond).getNextHwBuf(PDSel), PDSel);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAEBufMgr::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AEBufMgr::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AEBufMgr::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AEBufMgr::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}


MRESULT
IAEBufMgr::
setSensorMode(MINT32 const i4SensorDev, MINT32 i4NewSensorMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AEBufMgr::getInstance(ESensorDev_Main).setSensorMode(i4NewSensorMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AEBufMgr::getInstance(ESensorDev_Sub).setSensorMode(i4NewSensorMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AEBufMgr::getInstance(ESensorDev_MainSecond).setSensorMode(i4NewSensorMode);

    return (ret_main | ret_sub | ret_main2);
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AEBufMgr&
AEBufMgr::
getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  AEBufMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  AEBufMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  AEBufMgrDev<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("i4SensorDev = %d", i4SensorDev);
        return  AEBufMgrDev<ESensorDev_Main>::getInstance();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AEBufMgr::
AEBufMgr(ESensorDev_T const eSensorDev)
    : m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_None)
    , m_pIMemDrv(IMemDrv::createInstance())
    , m_pIspDrv(MNULL)
    , m_u4AEStateSize(0)
    , m_u4AEHistSize(0)
    , m_u4AWBStateSize(0)
    , m_Users(0)
    , m_Lock()
    , m_EnPDAF(MFALSE)
    , m_EnMVHDR(MFALSE)
    , m_bDebugEnable(MFALSE)
    , m_i4SensorIdx(0)
    , m_pPipe(MNULL)
    , m_i4CurrSensorId(0)
    , mpIspHwRegAddr(MNULL)
    , mfd(0)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AEBufMgr::
~AEBufMgr()
{

}

#define CAM_ISP_RANGE 0xA000
#define ISP_DEV_NAME     "/dev/camera-isp"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
init(MINT32 const i4SensorIdx)
{
#if ENABLE_AE_MVHDR_STAT
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aao_buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    MINT32 cam_isp_addr = 0x15000000;

    // sensor index
    m_i4SensorIdx = i4SensorIdx;

  MY_LOG("[%s()] m_eSensorDev: %d, m_i4SensorIdx: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_Users);

  Mutex::Autolock lock(m_Lock);

  if (m_Users > 0)
  {
    MY_LOG("%d has created \n", m_Users);
    android_atomic_inc(&m_Users);
    return MTRUE;
  }

    // imem driver init
    m_pIMemDrv->init();


  //init parameters.
  m_EnPDAF = MFALSE;
  m_EnMVHDR = MFALSE;
  m_FirstTimeCreateBuf = MTRUE;


    // Open isp driver
    mfd = open(ISP_DEV_NAME, O_RDWR);
    if (mfd < 0) {
        MY_ERR("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return -1;
    }

    mpIspHwRegAddr = (unsigned long *) mmap(0, CAM_ISP_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, cam_isp_addr);
    if (mpIspHwRegAddr == MAP_FAILED)
  {
        MY_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -4;
    }
    MY_LOG("%s() m_eSensorDev(%d) ISPAddr:0x%0x\n", __FUNCTION__, m_eSensorDev, mpIspHwRegAddr);



    android_atomic_inc(&m_Users);
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
uninit()
{
#if ENABLE_AE_MVHDR_STAT
    IMEM_BUF_INFO buf_info;

  MY_LOG("[%s()] m_eSensorDev: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_Users);

  Mutex::Autolock lock(m_Lock);

  // If no more users, return directly and do nothing.
  if (m_Users <= 0)
  {
    return MTRUE;
  }

  // More than one user, so decrease one User.
  android_atomic_dec(&m_Users);

  if (m_Users == 0) // There is no more User after decrease one User
  {

     m_FirstTimeCreateBuf = MTRUE;
     deleteBuf();

       // imem driver ininit
       m_pIMemDrv->uninit();

        if ( 0 != mpIspHwRegAddr ) {
            munmap(mpIspHwRegAddr, CAM_ISP_RANGE);
            mpIspHwRegAddr = NULL;
        }

        if(mfd >= 0)
        {
            close(mfd);
            mfd = -1;
        }
    }
  else  // There are still some users.
  {
    MY_LOG_IF(m_bDebugEnable,"Still %d users \n", m_Users);
  }
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
debugPrint()
{
  if( m_bDebugEnable)
  {
      BufInfoList_T::iterator it;

      for (it = m_PDBufCtrl.m_rHwBufList.begin(); it != m_PDBufCtrl.m_rHwBufList.end(); it++ )
          MY_LOG("[PD] memID:[0x%x]/virtAddr:[0x%x]/phyAddr:[0x%x] \n", it->memID, it->virtAddr,it->phyAddr);

      for (it = m_MVHDRBufCtrl.m_rHwBufList.begin(); it != m_MVHDRBufCtrl.m_rHwBufList.end(); it++ )
          MY_LOG("[MVHDR] memID:[0x%x]/virtAddr:[0x%x]/phyAddr:[0x%x] \n", it->memID ,it->virtAddr,it->phyAddr);
  }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
enqueueHwBuf(BufInfo_T& rBufInfo, MINT32 PDSel)
{
  MBOOL ret=MFALSE;

#if ENABLE_AE_MVHDR_STAT

    MY_LOG_IF(m_bDebugEnable,"%s m_eSensorDev %d (PDSel=%d)\n", __FUNCTION__, m_eSensorDev, PDSel);

  if( PDSel==_PD_BUF_SEL_ && m_EnPDAF)
  {
    //memset(reinterpret_cast<void *>(rBufInfo.virtAddr), 0xDB, rBufInfo.size);

      // add element at the end
      m_PDBufCtrl.m_rHwBufList.push_back(rBufInfo);
      m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);
    ret = MTRUE;
  }
  else if( PDSel!=_PD_BUF_SEL_ && m_EnMVHDR)
  {
      // add element at the end
      m_MVHDRBufCtrl.m_rHwBufList.push_back(rBufInfo);
      m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);
    ret = MTRUE;
  }

    debugPrint();
#endif
    return ret;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
dequeueHwBuf(BufInfo_T& rBufInfo, MINT32 i4PDSel, MUINT32 i4FrmNum)
{
  MBOOL ret=MFALSE;

#if ENABLE_AE_MVHDR_STAT

    MY_LOG_IF(m_bDebugEnable,"\n\n%s m_eSensorDev %d (PDSel=%d) FrmNum%d\n", __FUNCTION__, m_eSensorDev, i4PDSel, i4FrmNum);


    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vc.dump.enable", value, "0");
    MBOOL bEnable = atoi(value);


  if( i4PDSel==_PD_BUF_SEL_ && m_EnPDAF)
  {
      if (m_PDBufCtrl.m_rHwBufList.size())
    {
          rBufInfo = m_PDBufCtrl.m_rHwBufList.front();
          m_PDBufCtrl.m_rHwBufList.pop_front();
      ret = MTRUE;
      }

    if (bEnable)
    {
      char fileName[64];
      sprintf(fileName, "/sdcard/vc/%d_dma.raw", i4FrmNum);
      FILE *fp = fopen(fileName, "w");
      if( fp)
      {
        MY_LOG_IF(m_bDebugEnable,"%s\n", fileName);
        fwrite(reinterpret_cast<void *>(rBufInfo.virtAddr), 1, rBufInfo.size, fp);
        fclose(fp);
      }
      else
      {
        MY_ERR("fail to open file to save img: %s", fileName);
        MINT32 err = mkdir("/sdcard/vc", S_IRWXU | S_IRWXG | S_IRWXO);
        MY_LOG("err = %d", err);
      }
    }
  }
  else if( i4PDSel!=_PD_BUF_SEL_ && m_EnMVHDR)
  {
      static MINT32 frameCnt = 0;

    if (m_MVHDRBufCtrl.m_rHwBufList.size())
    {
          rBufInfo = m_MVHDRBufCtrl.m_rHwBufList.front();
          m_MVHDRBufCtrl.m_rHwBufList.pop_front();
      ret = MTRUE;
      }

      if (bEnable)
    {
          char fileName[64];
      sprintf(fileName, "/sdcard/vc/%d_ae.raw", frameCnt++);
          FILE *fp = fopen(fileName, "w");
          if( fp)
          {
        MY_LOG_IF(m_bDebugEnable,"%s\n", fileName);
        fwrite(reinterpret_cast<void *>(rBufInfo.virtAddr), 1, rBufInfo.size, fp);
        fclose(fp);
          }
      else
      {
              MY_ERR("fail to open file to save img: %s", fileName);
        MINT32 err = mkdir("/sdcard/vc", S_IRWXU | S_IRWXG | S_IRWXO);
              MY_LOG("err = %d", err);
      }
      }
      else
    {
      frameCnt = 0;
    }


    }



  if( m_bDebugEnable)
  {
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    MY_LOG_IF(m_bDebugEnable, "%s m_eSensorDev(%d) ISPAddr:0x%0x 0x%0x\n", __FUNCTION__, m_eSensorDev, mpIspHwRegAddr, pisp);

    MY_LOG_IF(m_bDebugEnable, "(0x9414) CAMSV_TG_VF_CON :0x%0x\n", ISP_REG(pisp, CAMSV_TG_VF_CON));
    MY_LOG_IF(m_bDebugEnable, "(0x9414) CAMSV_TG2_VF_CON :0x%0x\n", ISP_REG(pisp, CAMSV_TG2_VF_CON));
    MY_LOG_IF(m_bDebugEnable, "(0x9C48) CAMSV_TG2_FRMSIZE_ST :0x%0x\n", ISP_REG(pisp, CAMSV_TG2_FRMSIZE_ST));

    if( i4PDSel==_PD_BUF_SEL_ && m_EnPDAF)
    {
      MY_LOG_IF(m_bDebugEnable, "(0x9820) CAMSV_CAMSV2_CLK_EN : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV2_CLK_EN));
      MY_LOG_IF(m_bDebugEnable, "(0x9808) CAMSV_CAMSV2_INT_EN : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV2_INT_EN));
      MY_LOG_IF(m_bDebugEnable, "(0x9800) CAMSV_CAMSV2_MODULE_EN : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV2_MODULE_EN));
      MY_LOG_IF(m_bDebugEnable, "(0x983C) CAMSV_CAMSV2_PAK : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV2_PAK));
      MY_LOG_IF(m_bDebugEnable, "(0x9804) CAMSV_CAMSV2_FMT_SEL : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV2_FMT_SEL));
      MY_LOG_IF(m_bDebugEnable, "(0x9228) CAMSV_IMGO_SV_D_BASE_ADDR : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_D_BASE_ADDR));
      MY_LOG_IF(m_bDebugEnable, "(0x922C) CAMSV_IMGO_SV_D_OFST_ADDR : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_D_OFST_ADDR));
      MY_LOG_IF(m_bDebugEnable, "(0x9230) CAMSV_IMGO_SV_D_XSIZE : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_D_XSIZE));
      MY_LOG_IF(m_bDebugEnable, "(0x9234) CAMSV_IMGO_SV_D_YSIZE : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_D_YSIZE));
      MY_LOG_IF(m_bDebugEnable, "(0x9238) CAMSV_IMGO_SV_D_STRIDE : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_D_STRIDE));
      MY_LOG_IF(m_bDebugEnable, "(0x9C48) CAMSV_TG2_FRMSIZE_ST :0x%0x\n", ISP_REG(pisp, CAMSV_TG2_FRMSIZE_ST));
      MY_LOG_IF(m_bDebugEnable, "(0x9C18) CAMSV_TG2_SEN_GRAB_PXL :0x%0x\n", ISP_REG(pisp, CAMSV_TG2_SEN_GRAB_PXL));
      MY_LOG_IF(m_bDebugEnable, "(0x9C1C) CAMSV_TG2_SEN_GRAB_LIN :0x%0x\n", ISP_REG(pisp, CAMSV_TG2_SEN_GRAB_LIN));
    }
    else if( i4PDSel!=_PD_BUF_SEL_ && m_EnMVHDR)
    {
      MY_LOG_IF(m_bDebugEnable, "(0x9020) CAMSV_CAMSV_CLK_EN : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV_CLK_EN));
      MY_LOG_IF(m_bDebugEnable, "(0x9008) CAMSV_CAMSV_INT_EN : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV_INT_EN));
      MY_LOG_IF(m_bDebugEnable, "(0x9000) CAMSV_CAMSV_MODULE_EN : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV_MODULE_EN));
      MY_LOG_IF(m_bDebugEnable, "(0x903C) CAMSV_CAMSV_PAK : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV_PAK));
      MY_LOG_IF(m_bDebugEnable, "(0x9004) CAMSV_CAMSV_FMT_SEL : 0x%0x\n", ISP_REG(pisp, CAMSV_CAMSV_FMT_SEL));
      MY_LOG_IF(m_bDebugEnable, "(0x9208) CAMSV_IMGO_SV_BASE_ADDR : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_BASE_ADDR));
      MY_LOG_IF(m_bDebugEnable, "(0x920C) CAMSV_IMGO_SV_OFST_ADDR : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_OFST_ADDR));
      MY_LOG_IF(m_bDebugEnable, "(0x9210) CAMSV_IMGO_SV_XSIZE : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_XSIZE));
      MY_LOG_IF(m_bDebugEnable, "(0x9214) CAMSV_IMGO_SV_YSIZE : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_YSIZE));
      MY_LOG_IF(m_bDebugEnable, "(0x9218) CAMSV_IMGO_SV_STRIDE : 0x%0x\n", ISP_REG(pisp, CAMSV_IMGO_SV_STRIDE));
    }

  }


    MY_LOG_IF(m_bDebugEnable,"rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);


#endif
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
updateDMABaseAddr(MUINT32 u4BaseAddr, MINT32 PDSel)
{
  MBOOL ret=MFALSE;

#if ENABLE_AE_MVHDR_STAT
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;


    if (!u4BaseAddr)
  {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }

  if( PDSel==_PD_BUF_SEL_ && m_EnPDAF)
    MY_LOG("%s [PD] m_eSensorDev(%d) u4BaseAddr=0x%x ISPAddr:0x%0x 0x%0x\n", __FUNCTION__, m_eSensorDev, u4BaseAddr, mpIspHwRegAddr, pisp);
  else
    MY_LOG("%s [MVHDR] m_eSensorDev(%d) u4BaseAddr=0x%x ISPAddr:0x%0x 0x%0x\n", __FUNCTION__, m_eSensorDev, u4BaseAddr, mpIspHwRegAddr, pisp);



    if (PDSel==_PD_BUF_SEL_ && m_EnPDAF)
    {
    //ISP_REG(pisp, CAMSV_CAMSV2_PAK) = 0x7&((m_pixelMode<<2)|(0x3&m_PDBufCtrl.m_u1NumBit));
      ISP_REG(pisp, CAMSV_IMGO_SV_D_BASE_ADDR) = u4BaseAddr;
      ISP_REG(pisp, CAMSV_IMGO_SV_D_OFST_ADDR) = AAO_OFFSET_ADDR;
      ISP_REG(pisp, CAMSV_IMGO_SV_D_XSIZE) = m_PDBufCtrl.m_u4AAOXSize;
      ISP_REG(pisp, CAMSV_IMGO_SV_D_YSIZE) = m_PDBufCtrl.m_u4AAOYSize;
      if(m_pixelMode==ONE_PIXEL_MODE)
      ISP_REG(pisp, CAMSV_IMGO_SV_D_STRIDE) = m_PDBufCtrl.m_u4AAOXSize+1; //one pixel mode
      else
      ISP_REG(pisp, CAMSV_IMGO_SV_D_STRIDE) = 0x8B0000|(0xffff&(m_PDBufCtrl.m_u4AAOXSize+1)) ;
      MY_LOG("[PD] pixel mode %d, XSIZE=0x%x, YSIZE=0x%x\n",  m_pixelMode, m_PDBufCtrl.m_u4AAOXSize, m_PDBufCtrl.m_u4AAOYSize);
      ret = MTRUE;
    }
  else if( PDSel!=_PD_BUF_SEL_ && m_EnMVHDR)
    {
    //ISP_REG(pisp, CAMSV_CAMSV_PAK) = 0x7&((m_pixelMode<<2)|(0x3&m_MVHDRBufCtrl.m_u1NumBit));
          ISP_REG(pisp, CAMSV_IMGO_SV_BASE_ADDR) = u4BaseAddr;
          ISP_REG(pisp, CAMSV_IMGO_SV_OFST_ADDR) = AAO_OFFSET_ADDR;
      ISP_REG(pisp, CAMSV_IMGO_SV_XSIZE) = m_MVHDRBufCtrl.m_u4AAOXSize;
          ISP_REG(pisp, CAMSV_IMGO_SV_YSIZE) = m_MVHDRBufCtrl.m_u4AAOYSize;
          ISP_REG(pisp, CAMSV_IMGO_SV_STRIDE) = 0x1000;
    if(m_pixelMode==ONE_PIXEL_MODE)
      ISP_REG(pisp, CAMSV_IMGO_SV_STRIDE) = m_MVHDRBufCtrl.m_u4AAOXSize+1; //one pixel mode
    else
      ISP_REG(pisp, CAMSV_IMGO_SV_STRIDE) = 0x8B0000|(0xffff&(m_MVHDRBufCtrl.m_u4AAOXSize+1)) ;
    MY_LOG("[MVHDR] pixel mode %d, XSIZE=0x%x, YSIZE=0x%x\n", m_pixelMode, m_MVHDRBufCtrl.m_u4AAOXSize, m_MVHDRBufCtrl.m_u4AAOYSize);
      ret = MTRUE;
      }



#endif
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
DMAInit()
{
#if ENABLE_AE_MVHDR_STAT
    MY_LOG_IF(m_bDebugEnable,"(%s) m_eSensorDev(%d) m_SensorMode(%d)\n", __FUNCTION__, m_eSensorDev, m_SensorMode);


    if (m_eSensorTG == ESensorTG_1)
  {
    //new buffer.
    newBuf();

    if( m_EnPDAF)
    {
      updateDMABaseAddr(getNextHwBuf(_PD_BUF_SEL_), _PD_BUF_SEL_);
      // FIX ME
      //if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_CAMSV2_IMGO, MTRUE, MNULL))
      {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AAO");
        return MFALSE;
      }

    }

    if( m_EnMVHDR)
    {
      updateDMABaseAddr(getNextHwBuf());
      if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_CAMSV_IMGO, MTRUE, MNULL))
      {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AAO");
        return MFALSE;
      }
    }

    }
    else
  {
        MY_LOG("(%s) m_eSensorDev(%d) No TG:%d\n", __FUNCTION__, m_eSensorDev, m_eSensorTG);
    }

#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
DMAUninit()
{
#if ENABLE_AE_MVHDR_STAT
    MY_LOG_IF(m_bDebugEnable,"%s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);

    if (m_eSensorTG == ESensorTG_1)
  {

    if( m_EnPDAF)
    {
        // FIX ME
          //if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_CAMSV2_IMGO, MFALSE, MNULL))
          {
              //Error Handling
              MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AAO");
              return MFALSE;
          }

    }

    if( m_EnMVHDR)
    {
      if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_CAMSV_IMGO, MFALSE, MNULL))
      {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AAO");
        return MFALSE;
      }
    }

    }
    else
  {
        MY_LOG("%s() m_eSensorDev(%d) No TG:%d\n", __FUNCTION__, m_eSensorDev, m_eSensorTG);
    }

#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AEBufMgr::
getCurrHwBuf()
{
#if ENABLE_AE_MVHDR_STAT
    if (m_PDBufCtrl.m_rHwBufList.size() > 0)
  {
        return m_PDBufCtrl.m_rHwBufList.front().phyAddr;
    }
    else
  { // No free buffer
        MY_ERR("No free buffer\n");
        return 0;
    }
#else
    return 0;
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AEBufMgr::
getNextHwBuf(MINT32 PDSel)
{
#if ENABLE_AE_MVHDR_STAT
    BufInfoList_T::iterator it;

  if(PDSel==_PD_BUF_SEL_ && m_EnPDAF)
  {
      if (m_PDBufCtrl.m_rHwBufList.size() > 1)
    {
        it = m_PDBufCtrl.m_rHwBufList.begin();
          it++;
          return it->phyAddr;
      }
      else
    { // No free buffer
        MY_ERR("No free buffer\n");
        return 0;
      }
  }
  else if( PDSel!=_PD_BUF_SEL_ && m_EnMVHDR)
  {
      if (m_MVHDRBufCtrl.m_rHwBufList.size() > 1)
    {
        it = m_MVHDRBufCtrl.m_rHwBufList.begin();
          it++;
          return it->phyAddr;
      }
      else
    { // No free buffer
         MY_ERR("No free buffer\n");
         return 0;
      }

  }
    return 0;
#else
    return 0;
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
freeBuf(BufInfo_T &rBufInfo)
{
#if ENABLE_AE_MVHDR_STAT
    if (m_pIMemDrv->unmapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }

    if (m_pIMemDrv->freeVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }

    MY_LOG("%s() memID = 0x%x\n", __FUNCTION__, rBufInfo.memID);
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::deleteBuf()
{
  for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++)
  {
    if( m_EnPDAF)  freeBuf(m_PDBufCtrl.m_rAAOBufInfo[i]);
    if( m_EnMVHDR) freeBuf(m_MVHDRBufCtrl.m_rAAOBufInfo[i]);
  }

  return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize)
{
#if ENABLE_AE_MVHDR_STAT
    rBufInfo.size = u4BufSize;
    if (m_pIMemDrv->allocVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->allocVirtBuf() error");
        return MFALSE;
    }

    if (m_pIMemDrv->mapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->mapPhyAddr() error");
        return MFALSE;
    }

    //MY_LOG("%s (memID=0x%x)\n", __FUNCTION__, rBufInfo.memID);
#endif
    return MTRUE;
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::newBuf()
{
    MUINT32 PDBufSize=0,MVHDRBufSize=0;
    MUINT32 PDXSize=0, MVHDRXSize=0;
    MUINT32 PDYSize=0, MVHDRYSize=0;
    MUINT8  PDNumBits=0, MVHDRNumBits=0;

  IHalSensorList* const pIHalSensorList = IHalSensorList::get();
  IHalSensor* pIHalSensor = pIHalSensorList->createSensor("aaa_sensor_buf_mgr", m_i4SensorIdx);
  SensorStaticInfo rSensorStaticInfo;
  SensorDynamicInfo rSensorDynamicInfo;
  MINT32 i4SensorSenraio = m_SensorMode;
  SensorVCInfo rSensorVCInfo;

  //query sensor information.
  switch(m_eSensorDev)
  {
    case ESensorDev_Main:
      pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
      pIHalSensor->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
      pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
    break;

    case ESensorDev_Sub:
      pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
      pIHalSensor->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
      pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
    break;

    case ESensorDev_MainSecond:
      pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
      pIHalSensor->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
      pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
    break;

    default:
      MY_ERR("Invalid sensor device: %d", m_eSensorDev);
  }
  if(pIHalSensor) pIHalSensor->destroyInstance("aaa_sensor_buf_mgr");


  //set information
  m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
  m_pixelMode = rSensorDynamicInfo.pixelMode;




  MY_LOG("SensorID 0x%x, PDAF_Support mode %d, Senraio %d, m_pixelMode %d", m_i4CurrSensorId, rSensorStaticInfo.PDAF_Support, i4SensorSenraio, m_pixelMode);
  MY_LOG("VC_Num %d, ModeSelect %d, EXPO_Ratio %d, ODValue %d, RG_STATSMODE %d", rSensorVCInfo.VC_Num, rSensorVCInfo.ModeSelect, rSensorVCInfo.EXPO_Ratio, rSensorVCInfo.ODValue, rSensorVCInfo.RG_STATSMODE);
  MY_LOG("VC0, ID %d, DataType 0x%04x, SIZEH 0x%04x, SIZEV 0x%04x", rSensorVCInfo.VC0_ID, rSensorVCInfo.VC0_DataType, rSensorVCInfo.VC0_SIZEH, rSensorVCInfo.VC0_SIZEV);

  //for MVHDR
  MY_LOG("VC1, ID %d, DataType 0x%04x, SIZEH 0x%04x, SIZEV 0x%04x", rSensorVCInfo.VC1_ID, rSensorVCInfo.VC1_DataType, rSensorVCInfo.VC1_SIZEH, rSensorVCInfo.VC1_SIZEV);
  if( rSensorVCInfo.VC1_DataType!=0 &&
    rSensorVCInfo.VC1_SIZEH !=0 &&
    rSensorVCInfo.VC1_SIZEV!=0)
  {
    m_EnMVHDR = MTRUE;


    if( rSensorVCInfo.VC1_DataType==0x2b) //10 bit
  {
      MVHDRXSize = (rSensorVCInfo.VC1_SIZEH)*10/8;
      MVHDRNumBits = 0x2;
    }
    else //8 bit
    {
      MVHDRXSize = rSensorVCInfo.VC1_SIZEH;
      MVHDRNumBits = 0x3;
    }
    MVHDRYSize = rSensorVCInfo.VC1_SIZEV;

    //check pixel mode
    if( m_pixelMode==TWO_PIXEL_MODE)
    {
      //align 4
      if( (MVHDRXSize&0x03)!=0)
      {
        MVHDRXSize = MVHDRXSize+(4-(0x03&MVHDRXSize));
      }
    }
    else
    {
      //do nothing.
    }

    MVHDRXSize -= 1;
    MVHDRYSize -= 1;



  }
  else
  {
      m_EnMVHDR = MFALSE;
    MVHDRXSize = 0;
      MVHDRYSize = 0;
  }

  //for PDAF
  MY_LOG("VC2, ID %d, DataType 0x%04x, SIZEH 0x%04x, SIZEV 0x%04x", rSensorVCInfo.VC2_ID, rSensorVCInfo.VC2_DataType, rSensorVCInfo.VC2_SIZEH, rSensorVCInfo.VC2_SIZEV);
  if( rSensorStaticInfo.PDAF_Support==2 &&
    rSensorVCInfo.VC2_DataType!=0 &&
    rSensorVCInfo.VC2_SIZEH !=0 &&
    rSensorVCInfo.VC2_SIZEV!=0)
  {
    m_EnPDAF = MTRUE;

    if( rSensorVCInfo.VC2_DataType==0x2b) //10 bit
    {
      PDXSize = (rSensorVCInfo.VC2_SIZEH)*10/8;
      PDNumBits = 0x2;
    }
    else //8 bit
    {
      PDXSize = rSensorVCInfo.VC2_SIZEH;
      PDNumBits = 0x3;
    }
    PDYSize = rSensorVCInfo.VC2_SIZEV;

    //check pixel mode
    if( m_pixelMode==TWO_PIXEL_MODE)
    {
      //align 4
      if( (PDXSize&0x03)!=0)
      {
        PDXSize = PDXSize+(4-(0x03&PDXSize));
      }
    }
    else
    {
      //do nothing.
    }

    PDXSize -= 1;
    PDYSize -= 1;
  }
  else
  {
      m_EnPDAF = MFALSE;
    PDXSize = 0;
      PDYSize = 0;
  }


  MY_LOG("VC3, ID %d, DataType 0x%04x, SIZEH 0x%04x, SIZEV 0x%04x", rSensorVCInfo.VC3_ID, rSensorVCInfo.VC3_DataType, rSensorVCInfo.VC3_SIZEH, rSensorVCInfo.VC3_SIZEV);


  // DMA buffer init
  if( m_FirstTimeCreateBuf==MTRUE)
  {
    m_FirstTimeCreateBuf = MFALSE;
  }
  else
  {
    deleteBuf();
  }


  // removes all elements from the list container
  m_PDBufCtrl.m_rHwBufList.clear();
  m_MVHDRBufCtrl.m_rHwBufList.clear();

  // removes all elements from the list container
  m_PDBufCtrl.m_rHwBufList.clear();
  m_MVHDRBufCtrl.m_rHwBufList.clear();

  //
  m_PDBufCtrl.m_u4AAOXSize = PDXSize;
  m_PDBufCtrl.m_u4AAOYSize = PDYSize;
  m_PDBufCtrl.m_u1NumBit = PDNumBits;
  m_PDBufCtrl.m_u4AAOBufSize = (m_PDBufCtrl.m_u4AAOXSize+1)*(m_PDBufCtrl.m_u4AAOYSize+1);

  m_MVHDRBufCtrl.m_u4AAOXSize = MVHDRXSize;
  m_MVHDRBufCtrl.m_u4AAOYSize = MVHDRYSize;
  m_MVHDRBufCtrl.m_u1NumBit = MVHDRNumBits;
  m_MVHDRBufCtrl.m_u4AAOBufSize = (m_MVHDRBufCtrl.m_u4AAOXSize+1)*(m_MVHDRBufCtrl.m_u4AAOYSize+1);


    MY_LOG("== PDBufCtrl ==");
    MY_LOG("BufSize   = 0x%04x", m_PDBufCtrl.m_u4AAOBufSize);
    MY_LOG("XSize     = 0x%04x", m_PDBufCtrl.m_u4AAOXSize);
    MY_LOG("YSize     = 0x%04x", m_PDBufCtrl.m_u4AAOYSize);
    MY_LOG("NumBit    = 0x%04x", m_PDBufCtrl.m_u1NumBit);
    MY_LOG("== MVHDRBufCtrl ==");
    MY_LOG("BufSize   = 0x%04x", m_MVHDRBufCtrl.m_u4AAOBufSize);
    MY_LOG("XSize     = 0x%04x", m_MVHDRBufCtrl.m_u4AAOXSize);
    MY_LOG("YSize     = 0x%04x", m_MVHDRBufCtrl.m_u4AAOYSize);
    MY_LOG("NumBit    = 0x%04x", m_MVHDRBufCtrl.m_u1NumBit);

  // allocate and enqueue HW buffer
  for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++)
  {
    if( m_EnPDAF)
    {
      m_PDBufCtrl.m_rAAOBufInfo[i].useNoncache = 0; // improve the performance
      allocateBuf(m_PDBufCtrl.m_rAAOBufInfo[i], m_PDBufCtrl.m_u4AAOBufSize);
      enqueueHwBuf(m_PDBufCtrl.m_rAAOBufInfo[i], _PD_BUF_SEL_);
    }

    if( m_EnMVHDR)
    {
      m_MVHDRBufCtrl.m_rAAOBufInfo[i].useNoncache = 0;  // improve the performance
      allocateBuf(m_MVHDRBufCtrl.m_rAAOBufInfo[i],m_MVHDRBufCtrl.m_u4AAOBufSize);
      enqueueHwBuf(m_MVHDRBufCtrl.m_rAAOBufInfo[i]);
    }

  }

  return MTRUE;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEBufMgr::
AAStatEnable(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"m_eSensorDev(%d) AAStatEnable(%d)\n",m_eSensorDev, En);
    return MTRUE;
}
