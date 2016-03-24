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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_CAMSHOTUTILS_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_CAMSHOTUTILS_H_

#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/eis_type.h>

namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

MBOOL   IsSupportMfb(MUINT32 mfbMode, MUINT32 sensorId);

/**
 *  Get capture information for collect frames for MFLL/AIS feature.
 *  The pre-condition: The target sensor running on preview while you call this function. Because we nee
 *  3A alive to provide information for further calculation.
 *  @param[in] mfbMode           The multi-frame setting blending setting from CameraParameter
 *  @param[in] sensorId            The id of working sensor
 *  @param[out] Cap3AParam      IN AIS mode, u4AfeGain and u4Eposuretime get modified. and if needUpdateAE is true, caller need update AE with these modified value.
 *  @param[out] mfllFrameNum   The frame count for collect capture frames, should be 3 or 4
 *  @param[out] needUpdateAE   if true, the caller should update 3A with Cap3AParam (especially u4AfeGain and u4Eposuretime).
 */
MBOOL   getMFLLCapInfo(MUINT32 mfbMode, MUINT32 sensorId, NS3A::CaptureParam_T & Cap3AParam, MUINT32 &mfllFrameNum, MBOOL &needUpdateAE);

////////////////////////////////////////////////////////////////////////////////
};  // NSCamShot
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_CAMSHOTUTILS_H_

