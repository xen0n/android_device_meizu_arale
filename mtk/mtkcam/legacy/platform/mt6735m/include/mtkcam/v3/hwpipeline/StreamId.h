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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_STREAMID_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_STREAMID_H_
//
#include <mtkcam/v3/app/StreamId.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 * HW Pipeline Stream ID
 ******************************************************************************/
enum
{
    //==========================================================================
    eSTREAMID_BEGIN_OF_PIPE = (eSTREAMID_END_OF_APP+1UL),
    //==========================================================================

        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,

        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,

        eSTREAMID_IMAGE_PIPE_YUV_JPEG_00,

        eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00,

        eSTREAMID_IMAGE_FD,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_META_PIPE_CONTROL_00_SENSOR,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_META_PIPE_CONTROL,
        eSTREAMID_META_PIPE_DYNAMIC_01,
        eSTREAMID_META_PIPE_DYNAMIC_02,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_META_APP_DYNAMIC_01,
        eSTREAMID_META_APP_DYNAMIC_02,
        eSTREAMID_META_APP_DYNAMIC_FD,
        eSTREAMID_META_APP_DYNAMIC_JPEG,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_IMAGE_HDR,
        eSTREAMID_META_PIPE_DYNAMIC_HDR,
        eSTREAMID_META_APP_DYNAMIC_HDR,

    //==========================================================================
    eSTREAMID_END_OF_PIPE = (0x7FFFFFFFUL),
    //==========================================================================
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_STREAMID_H_

