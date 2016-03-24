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

#ifndef _MTK_HARDWARE_INCLUDE_MMUTILS_IGRALLOCHELPER_H_
#define _MTK_HARDWARE_INCLUDE_MMUTILS_IGRALLOCHELPER_H_
//
#include <utils/String8.h>
#include <utils/Vector.h>
#include <system/graphics.h>
#include <hardware/gralloc.h>
#include "gralloc_mtk_defs.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace mmutils {


/**
 *
 */
struct GrallocRequest {

    /**
     * The gralloc usage.
     */
    int                     usage;

    /**
     * The image format to request.
     */
    int                     format;

    /**
     * The image width in pixels. For formats where some color channels are
     * subsampled, this is the width of the largest-resolution plane.
     */
    int                     widthInPixels;

    /**
     * The image height in pixels. For formats where some color channels are
     * subsampled, this is the height of the largest-resolution plane.
     */
    int                     heightInPixels;

};


/**
 *
 */
struct GrallocStaticInfo {

    struct Plane {
        /**
         * The size for this color plane, in bytes.
         */
        size_t              sizeInBytes;

        /**
         * The row stride for this color plane, in bytes.
         *
         * This is the distance between the start of two consecutive rows of
         * pixels in the image. The row stride is always greater than 0.
         */
        size_t              rowStrideInBytes;

    };

    /**
     * The resulting image format.
     */
    int                     format;

    /**
     * The image width in pixels. For formats where some color channels are
     * subsampled, this is the width of the largest-resolution plane.
     */
    int                     widthInPixels;

    /**
     * The image height in pixels. For formats where some color channels are
     * subsampled, this is the height of the largest-resolution plane.
     */
    int                     heightInPixels;

    /**
     * A vector of planes.
     */
    android::Vector<Plane>  planes;

};


/**
 *
 */
struct GrallocDynamicInfo {

    /**
     * A vector of ion Fds.
     */
    android::Vector<int>    ionFds;

};


/**
 *
 */
class IGrallocHelper
{
public:     ////                Definitions.

    typedef android::String8    String8;

public:     ////                Instantiation.

    virtual                     ~IGrallocHelper() {}

    /**
     * Get a singleton instance.
     */
    static  IGrallocHelper*     singleton();

public:     ////                Interfaces.

    /**
     * Given a gralloc request, return its information.
     */
    virtual int                 query(
                                    struct GrallocRequest const*    pRequest,
                                    struct GrallocStaticInfo*       pStaticInfo
                                ) const                                     = 0;

    /**
     * Given a gralloc buffer handle, return its information.
     */
    virtual int                 query(
                                    buffer_handle_t                 bufHandle,
                                    struct GrallocStaticInfo*       pStaticInfo,
                                    struct GrallocDynamicInfo*      pDynamicInfo
                                ) const                                     = 0;

    /**
     * Set a given buffer's attribute with dirty camera.
     */
    virtual int                 setDirtyCamera(
                                    buffer_handle_t                 bufHandle
                                ) const                                     = 0;

    /**
     * Set a given buffer's colorspace to JFIF.
     */
    virtual int                 setColorspace_JFIF(
                                    buffer_handle_t                 bufHandle
                                ) const                                     = 0;

    /**
     * Given a HAL_PIXEL_FORMAT_xxx format, return a String8 name.
     */
    virtual String8             queryPixelFormatName(int format)    const   = 0;

    /**
     * Given a combination of usage, return a String8 name.
     */
    virtual String8             queryGrallocUsageName(int usage)    const   = 0;

    /**
     * Dump to the log for debug.
     */
    virtual void                dumpToLog() const                           = 0;

};


};  //namespace mmutils
/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_INCLUDE_MMUTILS_IGRALLOCHELPER_H_

