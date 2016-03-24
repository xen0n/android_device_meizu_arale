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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file vhdr_type.h
*
* VHDR Type Header File
*/


#ifndef _VHDR_TYPE_H_
#define _VHDR_TYPE_H_

#include "mtkcam/common.h"

/*********************************************************************
* Define
*********************************************************************/

#define VHDR_DEBUG_FLAG 0
#define VHDR_LCSO_SYNC_FAIL -99

/*********************************************************************
* ENUM
*********************************************************************/

/**
*@brief Return enum of VHDR
*/
typedef enum
{
    VHDR_RETURN_NO_ERROR       = 0,       //! The function work successfully
    VHDR_RETURN_UNKNOWN_ERROR  = 0x0001,  //! Unknown error
    VHDR_RETURN_INVALID_DRIVER = 0x0002,  //! invalid driver object
    VHDR_RETURN_API_FAIL       = 0x0003,  //! api fail
    VHDR_RETURN_INVALID_PARA   = 0x0004,  //! invalid parameter
    VHDR_RETURN_NULL_OBJ       = 0x0005,  //! null object
    VHDR_RETURN_MEMORY_ERROR   = 0x0006   //! memory error
}VHDR_ERROR_ENUM;

/**
*@brief Return enum of VHDR
*/
typedef enum
{
    VHDR_CAMERA_VER_1,
    VHDR_CAMERA_VER_3
}VHDR_CAMERA_VER_ENUM;

/**
*@brief VHDR state enum
*/
typedef enum VHDR_STATE
{
    VHDR_STATE_NONE = 0,
    VHDR_STATE_ALIVE,
    VHDR_STATE_UNINIT
}VHDR_STATE_ENUM;

/**
*@brief VHDR command enum
*/
typedef enum VHDR_CMD
{
    VHDR_CMD_SET_STATE = 0,
    VHDR_CMD_SET_VR_EIS_ON_OFF,
    VHDR_CMD_SET_VIDEO_SIZE,
    VHDR_CMD_SET_ZOOM_RATIO,
    VHDR_CMD_SET_PASS1_OUT_SIZE
}VHDR_CMD_ENUM;

/*********************************************************************
* Struct
*********************************************************************/

/**
  * @brief VHDR config structure
  *
*/
typedef struct VHDR_HAL_CONFIG
{
    VHDR_CAMERA_VER_ENUM cameraVer;
}VHDR_HAL_CONFIG_DATA;

/**
  *@brief LCEI dma info
*/
typedef struct LCEI_DMA
{
    MUINT32 xSize;
    MUINT32 ySize;
    MUINT32 stride;
    MINT32  memID;
    MUINT32 size;
    MUINT32 va;
    MUINT32 pa;
    MINT32  bufSecu;
    MINT32  bufCohe;
    MINT32  useNoncache;
}LCEI_DMA_INFO;

#endif // _VHDR_TYPE_H_

