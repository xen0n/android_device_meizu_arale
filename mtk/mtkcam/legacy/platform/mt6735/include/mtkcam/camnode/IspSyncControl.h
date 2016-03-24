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

#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_ISPSYNCCONTROL_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_ISPSYNCCONTROL_H_

#define MAGIC_NUM_CAP_MASK          (0x80000000)
#define MAGIC_NUM_UNKNOWN_MASK      (0x40000000)
#define MAGIC_NUM_INVALID           (0x3FFFFFFF)
#define MAGIC_NUM_MAX               (0x3FFFFFFE)
#define SetCap(num)                 ((num)|MAGIC_NUM_CAP_MASK)
namespace NSCam {
    class IImageBuffer;
};
using NSCam::IImageBuffer;

#include <vector>
#include "../algorithm/libeis/MTKEis.h"
using namespace std;

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

class IspSyncControl
{
    public:

        typedef enum
        {
            UPDATE_CMD_OK,
            UPDATE_CMD_FAIL,
            UPDATE_CMD_STOP,
            UPDATE_CMD_START,
        } UPDATE_CMD;

    public:

        static IspSyncControl*  createInstance(MUINT32 const sensorIdx);
        virtual MVOID           destroyInstance() = 0;

    protected:

        IspSyncControl(){};
        virtual ~IspSyncControl(){};

    public: //operations
        typedef MBOOL (*CbZoomFunc)(
                            MVOID*,
                            MINT32,
                            MINT32,
                            MINT32);
        typedef struct
        {
            MBOOL       enable;
            MFLOAT      scaleFactor;
            MUINT32     xInt;
            MUINT32     yInt;
            MUINT32     xFlt;
            MUINT32     yFlt;
        }EIS_CROP_INFO;

        virtual MVOID   addZoomRatio(MUINT32 ratio)                 = 0;
        virtual MINT32  popEndZoomRatio()                           = 0;
        virtual MUINT32 getZoomRatio()                              = 0;
        virtual MVOID   setCurZoomRatio(MUINT32 ratio)              = 0;
        virtual MBOOL   getSensorInfo(
                            MUINT32& scenario,
                            MUINT32& width,
                            MUINT32& height,
                            MUINT32& type)                          = 0;
        virtual MBOOL   calRrzoMaxZoomRatio()                       = 0;
        virtual MBOOL   setPass1InitRrzoSize(
                            MUINT32 width,
                            MUINT32 height)                         = 0;
        virtual MBOOL   setPreviewSize(
                            MUINT32 width,
                            MUINT32 height)                         = 0;
        virtual MBOOL   setVideoSize(
                            MUINT32 width,
                            MUINT32 height)                         = 0;
        virtual MBOOL   setRrzoMinSize(
                            MUINT32 width,
                            MUINT32 height)                         = 0;
        virtual MBOOL   getPreviewSize(
                            MUINT32& width,
                            MUINT32& height)                        = 0;
        virtual MBOOL   queryPass1CropInfo(
                            MUINT32         inMagicNum,
                            MUINT32&        currentZoomRatio,
                            MUINT32&        targetZoomRatio,
                            MRect&          outInCrop,
                            MSize&          outOutSize)             = 0;
        virtual MBOOL   dropPass2Info(IImageBuffer* pImgBuf)        = 0;
        virtual MBOOL   queryImgBufInfo(
                            IImageBuffer*   inpImgBuf,
                            MUINT32&        outMagicNum,
                            MBOOL&          outIsRrzo,
                            MVOID*&         outpPrivateData,
                            MUINT32&        outPrivateDataSize)     = 0;
        virtual MBOOL   send3AUpdateCmd(UPDATE_CMD cmd)             = 0;
        virtual MBOOL   addPass1ReplaceBuffer(
                            MUINT32 const fullcnt,
                            MUINT32 const resizedcnt)               = 0;
        virtual MUINT32 setHdrState(MUINT32 state)                  = 0;
        virtual MUINT32 getHdrState()                               = 0;
        virtual MBOOL   setZoomRatioList(vector<MUINT32> zoomRatios)= 0;
        virtual MBOOL   setEisResult(
                            IImageBuffer*           pImageBuffer,
                            EIS_CROP_INFO           eisCrop,
                            EIS_STATISTIC_STRUCT    eisStatistic)   = 0;
        virtual MBOOL   getEisStatistic(
                            IImageBuffer*           pImageBuffer,
                            EIS_STATISTIC_STRUCT&   eisStatistic)   = 0;
        virtual MBOOL   set3AUpdatePeriod(MUINT32 period)           = 0;
        virtual MBOOL   setCallbackZoom(
                            MVOID*      pObj,
                            CbZoomFunc  pFunc)                      = 0;
        virtual MBOOL   getCurPass2ZoomInfo(
                            MUINT32&    zoomRatio,
                            MUINT32&    cropX,
                            MUINT32&    cropY,
                            MUINT32&    cropW,
                            MUINT32&    cropH)                      = 0;
        virtual MBOOL   updatePass2ZoomRatio(MINT32 targetZoomRatio)= 0;
        virtual MBOOL   setPass1PureRaw(
                            MBOOL     enable,
                            MUINT32    period)                         = 0;
};


/*******************************************************************************
*
********************************************************************************/

}; // namespace NSCamNode

#endif  //_MTK_CAMERA_INCLUDE_CAMNODE_ISPSYNCCONTROL_H_
