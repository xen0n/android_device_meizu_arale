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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef RIL_NW_H
#define RIL_NW_H 1

typedef enum {
    NET_TYPE_NO_SERVICE = 0,
    NET_TYPE_CDMA_ONLY = 2,
    NET_TYPE_EVDO_ONLY = 4,
    NET_TYPE_CDMA_EVDO_HYBRID = 8
} VIA_NETWORK_TYPE;

typedef enum {
    NETWORK_MODE_WCDMA_PREF     = 0, /* GSM/WCDMA (WCDMA preferred) */
    NETWORK_MODE_GSM_ONLY       = 1, /* GSM only */
    NETWORK_MODE_WCDMA_ONLY     = 2, /* WCDMA only */
    NETWORK_MODE_GSM_UMTS       = 3, /* GSM/WCDMA (auto mode, according to PRL)
                                            AVAILABLE Application Settings menu*/
    NETWORK_MODE_HYBRID           = 4, /* CDMA and EvDo (auto mode, according to PRL)
                                            AVAILABLE Application Settings menu*/
    NETWORK_MODE_CDMA_NO_EVDO   = 5, /* CDMA only */
    NETWORK_MODE_EVDO_NO_CDMA   = 6, /* EvDo only */
    NETWORK_MODE_GLOBAL         = 7, /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
                                            AVAILABLE Application Settings menu*/
} Network_mode;

/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
typedef enum RoamingMode {
    ROAMING_MODE_HOME = 0,
    ROAMING_MODE_NORMAL_ROAMING = 1,
    ROAMING_MODE_JPKR_CDMA = 2,     // only for 4M version.
    ROAMING_MODE_UNKNOWN = 3,
} roaming_mode;
/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}

typedef struct{
	char *at_command;
	char *info;
}RIL_Eng_Mode_Info;

int rilNwMain(int request, void *data, size_t datalen, RIL_Token t);
int rilNwUnsolicited(const char *s, const char *sms_pdu);
/// M: [C2K][IR] for get switching state. 0 is not switching. 1 is switching @{
int getRatSwitching();
/// M: [C2K][IR] for get switching state. 0 is not switching. 1 is switching @}

#endif /* RIL_NW_H */
