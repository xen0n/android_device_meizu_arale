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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_PORT_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_PORT_H_
//
#include <mtkcam/common.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {

/******************************************************************************
 * @enum EPortType
 * @brief Pipe Port Type.
 ******************************************************************************/
enum EPortType
{
    EPortType_Sensor,                       /*!< Sensor Port Type */
    EPortType_Memory,                       /*!< Memory Port Type */
};

/******************************************************************************
 * @enum EPortCapbility
 * @brief Pipe Port Capbility.
 ******************************************************************************/
enum EPortCapbility
{
    EPortCapbility_None =0x00,                       /*!< no specific */
    EPortCapbility_Cap  =0x01,                        /*!< capture */
    EPortCapbility_Rcrd =0x02,                       /*!< record */
    EPortCapbility_Disp =0x03,                       /*!< display */
};


/******************************************************************************
 * @struct PortID
 * @brief Pipe Port ID (Descriptor).
 ******************************************************************************/
struct PortID
{
public:     ////    fields.
    /**
     * @var index
     *  port index
     */
    MUINT32         index       : 8;

    /**
     * @var type
     * EPortType
     */
    EPortType       type        : 8;

    /**
     * @var inout
     * 0:in/1:out
     */
    MUINT32         inout       : 1;

    /**
     * @var group
     * frame group(for burst queue support).
     */
    MUINT32         group       : 3;
    /**
     * @var capbility
     * port capbility
     */
    EPortCapbility  capbility   : 2;
    //
    /**
     * @var reserved
     * reserved for future use.
     */
    MUINT32         reserved    : 10;
    //

public:     ////    Constructors.

                    PortID(
                        EPortType const _eType,
                        MUINT32 const _index,
                        MUINT32 const _inout,
                        EPortCapbility const _capbility = EPortCapbility_None,
                        MUINT32 const _group = 0
                    )
                    {
                        type    = _eType;
                        index   = _index;
                        inout   = _inout;
                        capbility = _capbility;
                        group   = _group;
                        reserved= 0;
                    }
                    //

                    PortID(
                        MUINT32 const _value = 0
                    )
                    {
                        /*
                        type = 0xFF & (_value >> 0);
                        index= 0xFF & (_value >> 8);
                        inout= 0x01 & (_value >>16);
                        reserved = 0x7FFF & (_value >>17);
                        */
                        *reinterpret_cast<MUINT32*>(this) = _value;
                    }

public:     ////    Operators.

                    operator MUINT32() const { return *reinterpret_cast<MUINT32 const*>(this); }
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_PORT_H_

