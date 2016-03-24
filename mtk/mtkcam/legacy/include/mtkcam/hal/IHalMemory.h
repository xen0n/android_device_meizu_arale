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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALMEMORY_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALMEMORY_H_
//
#include <BuiltinTypes.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

enum IHALMEM_CACHECTRL_ENUM
{
    IHALMEM_CACHECTRL_ENUM_FLUSH       = 0,
    IHALMEM_CACHECTRL_ENUM_INVALID     = 1
};

/******************************************************************************
 *  Hal Memory Interface.
 ******************************************************************************/
class IHalMemory
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Memory Info.
                                    struct  Info
                                    {
                                    MINTPTR     pa;             // physicall address; in/out
                                    MINTPTR     va;             // virtual address;
                                    MINT32      ionFd;
                                    size_t      size;
                                    MINT32      security;
                                    MINT32      coherence;
                                                //
                                                Info(
                                                    MINTPTR     _pa = 0,
                                                    MINTPTR     _va = 0,
                                                    MINT32      _ionFd = -1,
                                                    size_t      _size = 0,
                                                    MINT32      _security = 0,
                                                    MINT32      _coherence = 0
                                                )
                                                    : pa(_pa)
                                                    , va(_va)
                                                    , ionFd(_ionFd)
                                                    , size(_size)
                                                    , security(_security)
                                                    , coherence(_coherence)
                                                {
                                                }
                                    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor.

    //  Disallowed to directly delete a raw pointer.
    virtual                         ~IHalMemory() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    static  IHalMemory*             createInstance(char const* szCallerName);
    virtual MVOID                   destroyInstance(char const* szCallerName)   = 0;

    virtual MBOOL                   mapPA(
                                        char const* szCallerName,
                                        Info* pInfo
                                    )                                           = 0;
    virtual MBOOL                   unmapPA(
                                        char const* szCallerName,
                                        Info const* pInfo
                                    )                                           = 0;

    virtual MBOOL                   syncCache(
                                        IHALMEM_CACHECTRL_ENUM const ctrl,
                                        Info const* pInfo,
                                        MUINT const num
                                    )                                           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALMEMORY_H_

