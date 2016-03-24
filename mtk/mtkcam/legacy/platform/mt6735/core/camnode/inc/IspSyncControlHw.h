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

#ifndef _MTK_CAMERA_CAMNODE_INCLUDE_ISPSYNCCONTROLHW_H_
#define _MTK_CAMERA_CAMNODE_INCLUDE_ISPSYNCCONTROLHW_H_
//
#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
using namespace NSCam::NSIoPipe::NSCamIOPipe;
#include <mtkcam/camnode/IspSyncControl.h>
#include <utils/Mutex.h>
using namespace android;
#include <list>
using namespace std;
/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

class IspSyncControlHw : public IspSyncControl
{
    public:

        // used in lockPASS()/unlockPASS()
        enum
        {
            HW_PASS1   = 0x1,
            HW_PASS2   = 0x2,
            HW_PURERAW = 0x3,
        };

    public:

        static IspSyncControlHw*    createInstance(MUINT32 const sensorIdx);
        virtual MVOID               destroyInstance() = 0;

    protected:

        IspSyncControlHw(){};
        virtual ~IspSyncControlHw(){};

    public: //operations

        virtual MBOOL   setSensorInfo(
                            MUINT32 scenario,
                            MUINT32 width,
                            MUINT32 height,
                            MUINT32 type)                       = 0;
        virtual MBOOL   setInitialRrzoSize(
                            MRect&          crop,
                            MSize&          size)               = 0;
        virtual MBOOL   getInitialRrzoSize(
                            EImageFormat    fmt,
                            MUINT32         pixelMode,
                            MRect&          crop,
                            MSize&          size)               = 0;
        virtual MBOOL   getPass1Crop(
                            MUINT32         inMagicNum,
                            MBOOL           inIsRrzo,
                            MRect&          outInCrop,
                            MSize&          outOutSize)         = 0;
        virtual MBOOL   addPass1Info(
                            MUINT32         inMagicNum,
                            IImageBuffer*   inpImgBuf,
                            ResultMetadata  inMetadata,
                            MBOOL           inIsRrzo)           = 0;
        virtual MBOOL   getPass2Info(
                            IImageBuffer*   inpImgBuf,
                            MSize           inOutImgSize,
                            MUINT32&        outMagicNum,
                            MVOID*&         outpPrivateData,
                            MUINT32&        outPrivateDataSize,
                            MCropRect&      outInCrop)          = 0;
        virtual MBOOL   clearTable()                            = 0;
        virtual MUINT32 getMagicNum(MBOOL bInit)                = 0;
        virtual MUINT32 getPass1ReplaceCnt(
                            MBOOL const bFull) const            = 0;
        virtual MBOOL   setIspEnquePeriod(MUINT32 period)       = 0;
        virtual MUINT32 getIspEnquePeriod()                     = 0;
        virtual MBOOL   isPass1PureRawEnable(void)              = 0;
        virtual MBOOL   getPass1PureRawStatus(MUINT32 magicNum) = 0;

        // to lock the hw pass
        virtual MBOOL   lockHw(MUINT32 const use)               = 0;
        virtual MBOOL   unlockHw(MUINT32 const use)             = 0;
};


/*******************************************************************************
*
********************************************************************************/

}; // namespace NSCamNode

#endif  //_MTK_CAMERA_CAMNODE_INCLUDE_ISPSYNCCONTROLHW_H_
