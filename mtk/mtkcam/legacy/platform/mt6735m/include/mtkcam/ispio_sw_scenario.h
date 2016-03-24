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

#ifndef _ISPIO_SW_SCENARIO_H_
#define _ISPIO_SW_SCENARIO_H_
/******************************************************************************
 *  Camera Definitions.
 ******************************************************************************/


 /**
 * @enum ESoftwareScenario
 * @brief software scenario Enumeration.
 *
 */
enum ESoftwareScenario
{
    eSoftwareScenario_Main_Normal_Stream,
    eSoftwareScenario_Main_Normal_Capture,
    eSoftwareScenario_Main_VSS_Capture,
    eSoftwareScenario_Main_ZSD_Capture,
    //eSoftwareScenario_Main_Mfb_Capture,
    //eSoftwareScenario_Main_Mfb_Blending,
    //eSoftwareScenario_Main_Mfb_Mixing,
    //eSoftwareScenario_Main_Vfb_Stream_1,
    //eSoftwareScenario_Main_Vfb_Stream_2,
    eSoftwareScenario_Main_Pure_Raw_Stream,
    //eSoftwareScenario_Main_CC_Raw_Stream,
    //eSoftwareScenario_Main2_N3D_Stream,
    eSoftwareScenario_Sub_Normal_Stream ,
    eSoftwareScenario_Sub_Normal_Capture,
    eSoftwareScenario_Sub_VSS_Capture,
    eSoftwareScenario_Sub_ZSD_Capture,
    //eSoftwareScenario_Sub_Mfb_Capture,
    //eSoftwareScenario_Sub_Mfb_Blending,
    //eSoftwareScenario_Sub_Mfb_Mixing,
    //eSoftwareScenario_Sub_Vfb_Stream_1,
    //eSoftwareScenario_Sub_Vfb_Stream_2,
    eSoftwareScenario_Sub_Pure_Raw_Stream,
    //eSoftwareScenario_Sub_CC_Raw_Stream,
    eSoftwareScenario_total_num
};


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_ISPIO_SW_SCENARIO_H_

