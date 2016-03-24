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
/*
** $Log: mcu_drv.h $
 *
*/

#ifndef _MCU_DRV_H_
#define _MCU_DRV_H_

#include "MediaTypes.h"
#include "camera_custom_lens.h"              //in custom folder
#include <linux/ioctl.h>

/*******************************************************************************
*
********************************************************************************/
//Structures
typedef struct {
    //current position
    u32 u4CurrentPosition;
    //macro position
    u32 u4MacroPosition;
    //Infiniti position
    u32 u4InfPosition;
    //Motor Status
    bool          bIsMotorMoving;
    //Motor Open?
    bool          bIsMotorOpen;
    //Slew Rate?
    bool          bIsSupportSR;

} mcuMotorInfo;

typedef struct {
    /* command buffer size */
    u32 uSize;
    /* command buffer 100 Bytes*/
    u8 uCmdBuf[100];
} mcuCmd;

typedef struct {
	u8 uMotorName[32];
} mcuMotorName;

#define mcuIOC_G_MOTORINFO    _IOR('A',0,mcuMotorInfo)
#define mcuIOC_T_MOVETO       _IOW('A',1,u32)
#define mcuIOC_T_SETINFPOS    _IOW('A',2,u32)
#define mcuIOC_T_SETMACROPOS  _IOW('A',3,u32)
#define mcuIOC_T_SETPARA      _IOW('A',5,u32)
#define mcuIOC_S_SENDCMD      _IOW('A',9,mcuCmd)
#define mcuIOC_S_SETDRVNAME   _IOW('A',10,mcuMotorName)

//Structures
typedef struct {
//if have ois hw, disable bit
bool bOIS_disable;
//if have ois hw, gain setting
unsigned long u4ois_gain;
//if have ois hw, freq setting
unsigned long u4ois_freq;
//if have ois hw, other setting1
unsigned long u4ois_setting1;
//if have ois hw, other setting2
unsigned long u4ois_setting2;
} mcuMotorPara;

enum {
    MCU_DEV_NONE    = 0x00,        /*!<No camera device*/
    MCU_DEV_MAIN    = 0x01,        /*!<Main camera device or Rear camera device*/
    MCU_DEV_SUB     = 0x02,        /*!<Sub camera device or Front camera device*/
    MCU_DEV_MAIN_2  = 0x04,        /*!<Main2 camera device (used in 3D scenario) */
    MCU_DEV_MAIN_3D = 0x05,        /*!<3D camera device (Main+Main2)*/
    MCU_DEV_ATV     = 0x08,        /*!<Analog TV device */
};

/*******************************************************************************
*
********************************************************************************/
class MCUDrv {
public:
    /////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////
    typedef enum
    {
        MCU_NO_ERROR = 0,                  ///< The function work successfully
        MCU_UNKNOWN_ERROR = 0x80000000,    ///< Unknown error
        MCU_INVALID_DRIVER   = 0x80000001,
    } MCU_ERROR_ENUM;

protected:
    /////////////////////////////////////////////////////////////////////////
    //
    // ~MCUDrv () -
    //! \brief mhal mcu base descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~MCUDrv() = 0;

public:     //// Interfaces
    /////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////
    static MCUDrv* createInstance(unsigned int a_u4CurrLensId);

    /////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void destroyInstance() = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // init () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int init(unsigned int a_u4CurrSensorDev ) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // uninit () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int uninit(unsigned int a_u4CurrSensorDev ) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // moveMCU () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int moveMCU(int a_i4FocusPos,unsigned int a_u4CurrSensorDev ) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // getMCUInfo () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int  getMCUInfo(mcuMotorInfo *a_pMotorInfo,unsigned int a_u4CurrSensorDev ) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // setMCUInfPos () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int setMCUInfPos(int a_i4FocusPos,unsigned int a_u4CurrSensorDev ) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // setMCUMacroPos () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int setMCUMacroPos(int a_i4FocusPos,unsigned int a_u4CurrSensorDev ) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // setMCUParam () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int setMCUParam(int ois_param,unsigned int a_u4CurrSensorDev ) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // lensSearch () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    static int lensSearch(unsigned int  a_u4CurrSensorDev, unsigned int  a_u4CurrSensorId);

    /////////////////////////////////////////////////////////////////////////
    //
    // getCurrLensID () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    static unsigned int getCurrLensID( unsigned int a_u4CurrSensorDev);
    /////////////////////////////////////////////////////////////////////////
    //
    // sendMCUCmd () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int sendMCUCmd( unsigned int a_u4CurrSensorDev, mcuCmd *a_pMcuCmd) = 0;

    //FIXME, is this need here
    static MSDK_LENS_INIT_FUNCTION_STRUCT m_LensInitFunc_main[MAX_NUM_OF_SUPPORT_LENS] ;
    static MSDK_LENS_INIT_FUNCTION_STRUCT m_LensInitFunc_main2[MAX_NUM_OF_SUPPORT_LENS] ;
    static MSDK_LENS_INIT_FUNCTION_STRUCT m_LensInitFunc_sub[MAX_NUM_OF_SUPPORT_LENS];

    static unsigned int  m_u4CurrLensIdx_main;
    static unsigned int  m_u4CurrLensIdx_main2;
    static unsigned int  m_u4CurrLensIdx_sub;

};

#endif

