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

#ifndef _MTK_CAMERA_TEST_MYCAMERA_H_
#define _MTK_CAMERA_TEST_MYCAMERA_H_
//


/******************************************************************************
*
*******************************************************************************/


namespace android {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*
*******************************************************************************/
class MyCamera : public CameraListener
{
public:     ////    Instantiation.
                    MyCamera();
    virtual         ~MyCamera() {}

public:     ////    Interfaces.
    static  bool    open(sp<MyCamera> rpMyCamera, int id = 0);
    void            close();
    //
    bool            setPreviewSize(int const w, int const h);
    bool            setPreviewFormat(char const* format);


public:     ////    Implementations.

    virtual void    notify(int32_t msgType, int32_t ext1, int32_t ext2) {}
    virtual void    postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {}
    virtual void    postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {}

public:     ////    Operators.
    sp<Camera>      get()           { return mpCamera.get(); }

protected:  ////
    sp<Camera>      mpCamera;
    int32_t         mi4PreviewWidth;
    int32_t         mi4PreviewHeight;
};


/******************************************************************************
*
*******************************************************************************/
};  // namespace android
#endif  //_MTK_CAMERA_TEST_MYCAMERA_H_

