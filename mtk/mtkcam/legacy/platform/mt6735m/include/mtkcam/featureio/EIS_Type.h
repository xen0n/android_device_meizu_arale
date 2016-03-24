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

//!\file  EIS_Type.h

#ifndef _EIS_TYPE_H_
#define _EIS_TYPE_H_

#include "mtkcam/common.h"
#include "../algorithm/libeis/MTKEis.h"


#define EIS_MAX_WIN_NUM EIS_WIN_NUM

/**
*@enum EIS_ERROR_ENUM
*@brief Return enum of EIS
*/
typedef enum
{
    EIS_RETURN_NO_ERROR         = 0,       //! The function work successfully
    EIS_RETURN_UNKNOWN_ERROR    = 0x0001,  //! Unknown error
    EIS_RETURN_INVALID_DRIVER   = 0x0002,  //! invalid driver object
    EIS_RETURN_API_FAIL         = 0x0003,  //! api fail
    EIS_RETURN_INVALID_PARA     = 0x0004,  //! invalid parameter
    EIS_RETURN_NULL_OBJ         = 0x0005,  //! null object
    EIS_RETURN_MEMORY_ERROR     = 0x0006,  //! memory error
    EIS_RETURN_EISO_MISS        = 0x0007   //! EISO data missed
} EIS_ERROR_ENUM;

/**
*@struct EIS_STATISTIC_T
*@brief  EIS statistic data structure
*/
typedef struct
{
    MINT32 i4LMV_X[EIS_MAX_WIN_NUM];
    MINT32 i4LMV_Y[EIS_MAX_WIN_NUM];

    MINT32 i4LMV_X2[EIS_MAX_WIN_NUM];
    MINT32 i4LMV_Y2[EIS_MAX_WIN_NUM];

    MINT32 i4NewTrust_X[EIS_MAX_WIN_NUM];
    MINT32 i4NewTrust_Y[EIS_MAX_WIN_NUM];

    MINT32 i4SAD[EIS_MAX_WIN_NUM];
    MINT32 i4SAD2[EIS_MAX_WIN_NUM];
    MINT32 i4AVG[EIS_MAX_WIN_NUM];
} EIS_STATISTIC_T;

/*********************************************** R chip *********************************************/

#define CIF_WIDTH 352
#define CIF_HEIGHT 320

#define D1_WIDTH 792
#define D1_HEIGHT 528

#define HD_720P_WIDTH 1536
#define HD_720P_HEIGHT 864

#define HD_1080P_WIDTH 2112
#define HD_1080P_HEIGHT 1188

#define HD_8M_WIDTH 3264
#define HD_8M_HEIGHT 2200

#define HD_12M_WIDTH 4000
#define HD_12M_HEIGHT 3000

#define HD_16M_WIDTH 4608
#define HD_16M_HEIGHT 3456

#define HD_20M_WIDTH 5164
#define HD_20M_HEIGHT 3872

#define ALIGN_SIZE(in,align) (in & ~(align-1))

#define VR_1080P_W 1920
#define VR_1080P_H 1088
#define EIS_FE_MAX_INPUT_W 2304
#define EIS_FE_MAX_INPUT_H 1296

#define EIS_EISO_SYNC_FAIL -99

/*********************************************************************
* ENUM
*********************************************************************/

/**
*@brief Sensor type
*/
typedef enum
{
    EIS_RAW_SENSOR,
    EIS_YUV_SENSOR,
    EIS_JPEG_SENSOR,
    EIS_NULL_SENSOR
}EIS_SENSOR_ENUM;

/**
*@brief EIS scenario enum
*/
typedef enum
{
    EIS_SCE_EIS,
    EIS_SCE_EIS_PLUS,
    EIS_SCE_MFBLL,      // multi-frame lowlight
    EIS_SCE_MT,         // motion track
    EIS_SCE_MAV,
    EIS_SCE_NONE,
    EIS_SCE_NUM
}EIS_SCENARIO_ENUM;

/**
*@brief DMA PORT
*/
typedef enum
{
    EIS_DMA_EISO = 0,
    EIS_DMA_FEO
}EIS_DMA_ENUM;

/**
*@brief Flush enum
*/
typedef enum
{
    EIS_FLUSH_HW = 0,
    EIS_FLUSH_SW
}EIS_FLUSH_ENUM;

/**
*@brief EIS software state enum
*/
typedef enum EIS_SW_STATE
{
    EIS_SW_STATE_NONE = 0,
    EIS_SW_STATE_ALIVE,
    EIS_SW_STATE_UNINIT,
    EIS_SW_STATE_UNINIT_CHECK
}EIS_SW_STATE_ENUM;

/**
*@brief EIS command enum
*/
typedef enum EIS_CMD
{
    EIS_CMD_SET_STATE = 0,
    EIS_CMD_CHECK_STATE
}EIS_CMD_ENUM;

/*********************************************************************
* Struct
*********************************************************************/
/**
  * @brief EIS config structure
  *
*/
typedef struct EIS_HAL_CONFIG
{
    MUINT32 sensorType;     // use in pass1
    MUINT32 p1ImgW;         // use in pass1
    MUINT32 p1ImgH;         // use in pass1
    MUINT32 memAlignment;   // pass1 memory alignment
    EIS_SCENARIO_ENUM configSce;
}EIS_HAL_CONFIG_DATA;

#endif // _EIS_TYPE_H_

