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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_CAMIO_INORMALPIPE_WRAP_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_CAMIO_INORMALPIPE_WRAP_H_
//

#include "mtkcam/imageio/p1HwCfg_wrap.h" //for wrapper
//#include "mtkcam/drv/isp_drv.h" //include by p1HwCfg_wrap.h

#define IOPIPE_SET_MODUL_REG(handle,RegName,Value) HWRWCTL_SET_MODUL_REG(handle,RegName,Value)
#define IOPIPE_SET_MODUL_REGS(handle, StartRegName, size, ValueArray) HWRWCTL_SET_MODUL_REGS(handle, StartRegName, size, ValueArray)

#include "IHalCamIO.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

#define NORMALPIPE_MAX_INST_CNT 3

/******************************************************************************
 *
 * @class INormalStream
 * @brief CamIO Pipe Interface for Normalpipe, each Obj mapping to only 1 sensor.
 * @details
 * The data path will be Sensor --> ISP-- Mem.
 *
 ******************************************************************************/
class INormalPipe : public IHalCamIO
{
public:     ////                    Instantiation.

    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static  INormalPipe* createInstance(MUINT32 *pSensorIndex, MUINT32 sCnt,char const* szCallerName, MINT32 burstQnum=1);
    static  INormalPipe* createInstance(MUINT32 sensorIndex,char const* szCallerName, MINT32 burstQnum=1);
    //void                 destroyInstance(MUINT32 sensorIndex,char const* szCallerName);
    //Temp
    static  INormalPipe* createInstance(char const* szCallerName, MINT32 burstQnum=1);
    void                 destroyInstance(char const* szCallerName);
    virtual MBOOL   configPipe(QInitParam const& vInPorts, NSImageio::NSIspio::EScenarioID const eSWScenarioID) = 0;//for v1 wrapper
public:     ////                    Attributes.

    /**
     * @brief Query pipe capability.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    static IMetadata const&         queryCapability(MUINT32 iOpenId);

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    static  char const*             pipeName() { return "CamIO::NormalPipe"; }

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const { return pipeName(); }


    virtual MBOOL sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3) = 0;

    virtual MBOOL DummyFrame(QBufInfo const& rQBuf)   = 0;
};

/*****************************************************************************
*
* @class INormalPipe_FrmB
* @brief CamIO Pipe Interface for Normal pipe in frame based mode.
* @details
* The data path will be Sensor --> ISP-- Mem.
*
******************************************************************************/
class INormalPipe_FrmB : public IHalCamIO
{
public:     ////                    Instantiation.
    static  INormalPipe_FrmB*   createInstance(MUINT32 sensorIndex,char const* szCallerName, MINT32 burstQnum=1);
    static  INormalPipe_FrmB*   createInstance(MUINT32 *pSensorIndex, MUINT32 sCnt,char const* szCallerName, MINT32 burstQnum=1);
    static  INormalPipe_FrmB*   createInstance(char const* szCallerName, MINT32 burstQnum=1);
    void                 destroyInstance(char const* szCallerName);

public:     ////                    Attributes.

    /**
    * @brief Query pipe capability.
    *
    * @details
    *
    * @note
    *
    * @return
    * - MTRUE indicates success; MFALSE indicates failure.
    */
    static IMetadata const&         queryCapability(MUINT32 iOpenId);

    /**
    * @brief get the pipe name
    *
    * @details
    *
    * @note
    *
    * @return
    * A null-terminated string indicating the name of the pipe.
    *
    */
    static  char const*             pipeName() { return "CamIO::NormalPipe_FrmB"; }

    /**
    * @brief get the pipe name
    *
    * @details
    *
    * @note
    *
    * @return
    * A null-terminated string indicating the name of the pipe.
    *
    */
    virtual char const*             getPipeName() const { return pipeName(); }


    virtual MBOOL sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) = 0;

    /**
        @brief Push dummy working buffer into driver to replace if no available request in driver to prevent drop frame

        @details

        @note Regular service between configPipe() and start(). and total dummy buffer number need as much as ring buffer.
        Can push multi-port at once, and each port can support multi-dummy buffer.

        @param

        @return Ture if sucess
    */
    virtual MBOOL DummyFrame(QBufInfo const& rQBuf)   = 0;
};


typedef struct {
   MBOOL  mSupported;

}NormalPipe_EIS_Info;


typedef struct {
    MBOOL  mEnabled;
    MSize  size;

}NormalPipe_HBIN_Info;

/**
    enum for current frame status.
    mapping to kernel status
*/
typedef enum{
    _normal_status          = 0,
    _drop_frame_status      = 1,
    _last_working_frame     = 2,
}NormalPipe_FRM_STATUS;

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCamIO
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_INORMALSTREAM_H_

