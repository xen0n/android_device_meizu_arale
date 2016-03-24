/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MEDIATEK_MMSDK_IMAGE_BUFFER_INFO_H_
#define _MEDIATEK_MMSDK_IMAGE_BUFFER_INFO_H_


namespace android {
namespace NSMMSdk {
//
struct ImageInfo
{
    /*
     * format should be the pixel format definitions in system/graphics.h
     *
     */
    int format;
    int width;
    int height;
    int numOfPlane;
    int stride[3];

    //
    ImageInfo(
    )
    :format(HAL_PIXEL_FORMAT_YCbCr_420_888)
    ,width(0)
    ,height(0)
    ,numOfPlane(3)
    {
        stride[0] = stride[1] = stride[2] = 0;
    }

    //
    ImageInfo(
        int _format,
        int _width,
        int _height,
        int _numOfPlane,
        int _stride[3]
    )
    :format(_format)
    ,width(_width)
    ,height(_height)
    ,numOfPlane(_numOfPlane)
    {
        stride[0] = _stride[0];
        stride[1] = _stride[1];
        stride[2] = _stride[2];
    }
};
}; // namespace NSMMSdk
}; // namespace android

#endif   //_MEDIATEK_MMSDK_IMAGE_BUFFER_INFO_H_
