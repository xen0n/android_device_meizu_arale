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
/**
 * @file pip_hal.h
 * @brief PIP Hal Header File
 * @details PIP Hal Header File
 */
#ifndef _PIP_HAL_H_
#define _PIP_HAL_H_






/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
/**
 * @brief PIP Hal class used by scenario
 * @details PIP Hal class used by scenario
 */
class PipHal
{
public:

    /**
     * @brief PipHal constructor
     */
    PipHal() {};

    /**
     * @brief Create PipHal object
     * @param[in] userName : user name,i.e. who create PIP HAL object
     * @param[in] aSensorIdx : sensor index
     * @return
     * - PipHal object
     */
    static PipHal *createInstance(const MUINT32 &aSensorIdx);

    /**
     * @brief Destroy PipHal object
     * @param[in] userName : user name, i.e. who destroy Pip HAL object
     */
    virtual MVOID destroyInstance(char const *userName) = 0;

    /**
     * @brief Initialization function
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL Init() = 0;

    /**
     * @brief Unitialization function
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL Uninit() = 0;

    /**
     * @brief Get PIP PV/VR ZSD/non-ZSD frame rate limit.
     * @param[out] ZsdFrameRate      : Max supporting frame rate when ZSD on  in PIP mode.
     * @param[out] NonZsdFrameRate   : Max supporting frame rate when ZSD off in PIP mode.
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL GetMaxFrameRate(MUINT32 &ZsdFrameRate, MUINT32 &NonZsdFrameRate) = 0;

    /**
     * @brief Get PIP PV/VR ZSD/non-ZSD HW frame rate limit.
     * @param[out] i4SensorDev          : Input Sensor Device ID.
     * @param[out] ZsdHwMaxFrameRate    : HW Max supporting frame rate when ZSD on  in PIP mode.
     * @param[out] NonZsdHwMaxFrameRate : HW Max supporting frame rate when ZSD off in PIP mode.
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL GetHwMaxFrameRate(MINT32 i4SensorDev, MUINT32 &ZsdHwMaxFrameRate, MUINT32 &NonZsdHwMaxFrameRate) = 0;

    /**
     * @brief Get PIP PV/VR ZSD/non-ZSD Sensor frame rate limit.
     * @param[out] i4SensorDev          : Input Sensor Device ID.
     * @param[out] ZsdHwMaxFrameRate    : Sensor Max supporting frame rate when ZSD on  in PIP mode.
     * @param[out] NonZsdHwMaxFrameRate : Sensor Max supporting frame rate when ZSD off in PIP mode.
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL GetSensorMaxFrameRate(MINT32 i4SensorDev, MUINT32 &ZsdSensorMaxFrameRate, MUINT32 &NonZsdSensorMaxFrameRate) = 0;

protected:

    /**
     * @brief PipHal destructor
     */
    virtual ~PipHal() {};
};

#endif  // _PIP_HAL_H_

