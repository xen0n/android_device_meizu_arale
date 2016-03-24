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
#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_DEFAULT_CTRL_NODES_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_DEFAULT_CTRL_NODES_H_
//
//
//
#include <mtkcam/camshot/CapBufMgr.h>
using namespace NSCamShot;
//
#include <mtkcam/camnode/nodeDataTypes.h>
#include <mtkcam/camnode/ICamGraphNode.h>
/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
#define IMG_720P_W      (1280)
#define IMG_720P_H      (720)
#define IMG_720P_SIZE   (IMG_720P_W*IMG_720P_H)
//
#define IMG_1080P_W     (1920)
#define IMG_1080P_H     (1088)
#define IMG_1080P_SIZE  (IMG_1080P_W*IMG_1080P_H)
//
#define IMG_2160P_W     (1920*2)
#define IMG_2160P_H     (1080*2)
#define IMG_2160P_SIZE  (IMG_2160P_W*IMG_2160P_H)

typedef enum
{
    ZSDEXT_OFF          = 0x0,
    ZSDEXT_HDR          = 0x1,
    ZSDEXT_MFLL         = 0x2,
    ZSDEXT_MFLL_FAST    = 0x3,
}ZSD_EXT_MODE;
/*******************************************************************************
*
********************************************************************************/
class DefaultCtrlNode : public ICamNode
{
    public:     /*createInstance()*/
        typedef enum
        {
            CTRL_NODE_DEFAULT,
            CTRL_NODE_ENG
        }CTRL_NODE_TYPE;
        //
        static DefaultCtrlNode* createInstance(
                                    const char*     userName,
                                    CTRL_NODE_TYPE  ctrlNodeType);
        virtual void            destroyInstance();
    //
    protected:  /*ctor&dtor*/
                 DefaultCtrlNode();
        virtual  ~DefaultCtrlNode();
    //
    public:
        typedef enum
        {
            MODE_IDLE,
            MODE_NORMAL_PREVIEW,
            MODE_NORMAL_CAPTURE,
            MODE_VIDEO_PREVIEW,
            MODE_VIDEO_RECORD,
            MODE_VIDEO_SNAPSHOT,
            MODE_ZSD_PREVIEW,
            MODE_ZSD_CAPTRUE,
            MODE_STEREO_PREVIEW,
            MODE_STEREO_ZSD_PREVIEW
        }MODE_TYPE;
        //
        typedef struct
        {
            sp<IParamsManager>      spParamsMgr;
            sp<CamMsgCbInfo>        spCamMsgCbInfo;
            CapBufMgr*              pCapBufMgr;
            MUINT32                 replaceBufNumResize;
            MUINT32                 replaceBufNumFullsize;
            MUINT32                 rollbackBufNum;
            MUINT32                 rollbackBufPeriod;
            MUINT32                 previewMaxFps;
        }CONFIG_INFO;
        //
        virtual MBOOL   setConfig(CONFIG_INFO configInfo)           = 0;
        virtual MBOOL   updateDelay(EQueryType_T const type)        = 0;
        virtual MBOOL   setParameters()                             = 0;
        virtual MBOOL   startSmoothZoom(MINT32 arg1)                = 0;
        virtual MBOOL   stopSmoothZoom()                            = 0;
        virtual MBOOL   precap()                                    = 0;
        virtual MBOOL   setForceRotation(
                            MBOOL   bIsForceRotation,
                            MUINT32 rotationAnagle)                 = 0;
        virtual MBOOL   setMode(MODE_TYPE mode)                     = 0;
        virtual MUINT32 getShotMode() const                         = 0;
        virtual MBOOL   updateShotMode()                            = 0;
        virtual MBOOL   getVideoSnapshotSize(
                            MINT32& width,
                            MINT32& height)                         = 0;
        virtual MBOOL   isRecBufSnapshot()                          = 0;
        virtual MBOOL   waitPreviewReady()                          = 0;
        // for ZSD Extension mode
        virtual void    startZsdExtShot(ZSD_EXT_MODE extMode)       = 0;
        virtual void    stopZsdExtShot()                            = 0;
        virtual ZSD_EXT_MODE   getZsdExtMode()                      = 0;
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //  _MTK_CAMERA_INCLUDE_CAMNODE_PREVIEW_CVTNODES_H_

