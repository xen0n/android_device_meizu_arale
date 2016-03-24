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
#ifndef _MAHL_JPG_H_
#define _MAHL_JPG_H_

#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <fcntl.h>
#include <linux/cache.h>
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;

#include <mtkcam/common/faces.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>

#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/utils/ImageBufferHeap.h>
#include <mtkcam/hal/IHalSensor.h>
//#include <mtkcam/hal/sensor_hal.h>

#include <Shot/IShot.h>

//#include <mtkcam/camshot/ICamShot.h>
//#include <mtkcam/camshot/ISingleShot.h>
//#include <mtkcam/camshot/ISmartShot.h>
#include <mtkcam/iopipe/SImager/ISImager.h>
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>

//#include "ImpShot.h"
//#include <mtkcam/featureio/facebeautify_hal_base.h>
//#include <mtkcam/sdk/hal/IFaceBeautySDKHal.h>
#include <mtkcam/exif/IDbgInfoContainer.h>

#include <mtkcam/featureio/SDkJPG.h>

//#include "FaceBeautyEffectHal.h"
//#include "SDKFaceBeauty.h"

using namespace android;
using namespace NSShot;
using namespace NSCamShot;
using namespace NSCam;

class my_encode_params
{
	public:
		// buffer
		IImageBuffer*			pSrc;
		IImageBuffer*			pDst;

		// settings
		MUINT32 				transform;
		MRect					crop;
		MUINT32 				isSOI;
		MUINT32 				quality;
		MUINT32 				codecType;
};


/*******************************************************************************
*
*******************************************************************************/

namespace NSCam {


class Mhal_jpg
{
public:

	//IImageBuffer*     mSrcImgBuffer;
    //IImageBuffer*     mDstImgBuffer;

    //	Source.
    IImageBuffer* mpSource;

    //IImageBuffer* mpPostviewImgBuf;

	IMemDrv*        mpIMemDrv;

	IImageBufferAllocator* mpIImageBufAllocator;

	typedef struct
    {
        IImageBuffer* pImgBuf;
        IMEM_BUF_INFO memBuf;
    } ImageBufferMap;
	
    Vector<ImageBufferMap> mvImgBufMap;

    MINT32 JPGRotation;

public:     ////    Interfaces.

    Mhal_jpg();
    virtual ~Mhal_jpg()  {}
	
	virtual MBOOL	init();
	virtual MBOOL   SDKcreateJpegImg();
	virtual IImageBuffer* allocMem(MUINT32 fmt, MUINT32 w, MUINT32 h);
	virtual void    deallocMem(IImageBuffer* pBuf);	
    virtual MBOOL   ImgProcess(IImageBuffer const* Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srctype, IImageBuffer const* Desbufinfo, MUINT32 desWidth, MUINT32 desHeight, EImageFormat destype, MUINT32 transform = 0) const;

};
};

#endif  //  _MAHL_FB_H_

