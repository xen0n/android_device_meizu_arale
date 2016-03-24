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
#ifndef _MTK_CAMERA_V1_ADAPTER_SCENARIO_SHOT_INC_SCENARIOSHOTUTIL_H_
#define _MTK_CAMERA_V1_ADAPTER_SCENARIO_SHOT_INC_SCENARIOSHOTUTIL_H_

#include <mtkcam/featureio/aaa_hal_common.h>

namespace android {
namespace NSShot {
////////////////////////////////////////////////////////////////////////////////

/**
 *  Get capture information for collect frames for HDR feature.
 *  The pre-condition: The target sensor running on preview while you call this function. Because we nee
 *  3A alive to provide information for further calculation.
 *  @param[in] sensorId            The id of working sensor
 *  @param[out] vCap3AParam   A vector of <NS3A::CaptureParam_T>, HDR will fill target 2 or 3 frame exposure condition in this vector.
 *  @param[out] hdrFrameNum   The frame count for collect capture frames, should be 2 or 3
 */

MBOOL   getHDRCapInfo(MUINT32 sensorId, std::vector<NS3A::CaptureParam_T> & vCap3AParam, MUINT32 & hdrFrameNum);

////////////////////////////////////////////////////////////////////////////////
};  // NSShot
};// android
#endif  //  _MTK_CAMERA_V1_ADAPTER_SCENARIO_SHOT_INC_SCENARIOSHOTUTIL_H_

