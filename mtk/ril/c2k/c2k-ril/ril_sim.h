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

#ifndef RIL_SIM_H
#define RIL_SIM_H 1

typedef enum {
    SIM_MISSING_RESOURCE = 13,
    SIM_NO_SUCH_ELEMENT = 90,
} AT_SIM_ERROR;

static const char PROPERTY_RIL_UICC_TYPE[4][25] = {
    "gsm.ril.uicctype",
    "gsm.ril.uicctype.2",
    "gsm.ril.uicctype.3",
    "gsm.ril.uicctype.4",
};

static const char PROPERTY_RIL_FULL_UICC_TYPE[4][25] = {
    "gsm.ril.fulluicctype",
    "gsm.ril.fulluicctype.2",
    "gsm.ril.fulluicctype.3",
    "gsm.ril.fulluicctype.4",
};

static const char PROPERTY_RIL_CT3G[4][25] = {
    "gsm.ril.ct3g",
    "gsm.ril.ct3g.2",
    "gsm.ril.ct3g.3",
    "gsm.ril.ct3g.4",
};

/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode {*/
typedef void (*UpdateCardState)(void *controller, int card_state);
typedef void (*SetSimPower)(void *controller);
typedef void (*RefreshSimPower)(void *controller, int card_state);
typedef void (*TiggerSetSimPower)(int power);

typedef enum {
    _FALSE = 0,
    _TRUE = 1
} _BOOL;

typedef struct {
    int card_state;
    _BOOL is_poweroff;
    UpdateCardState update;
    SetSimPower execute;
    RefreshSimPower refresh;
    TiggerSetSimPower trigger;
} SimPowerController;

static void updateCardState(void *controller, int card_state);
static void setSimPower(void *controller);
static void refreshSimPower(void *controller, int card_state);
static void triggerSetSimPower(int power);
static void requestSetSimPower(void *controller);
/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode }*/

int rilSimMain(int request, void *data, size_t datalen, RIL_Token t);
int rilSimUnsolicited(const char *s, const char *sms_pdu);

void reportRuimStatus(void* param);
void reportInvalidSimDetected(void);
void reportCdmaSimPlugIn(void);
void reportGSMSimPlugIn(void);
void reportCardType (void);
void reportPbSmsReady(void);

#endif /* RIL_CC_H */
