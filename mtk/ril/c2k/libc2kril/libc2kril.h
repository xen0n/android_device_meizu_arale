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

/* //device/libs/telephony/ril_event.h
**
** Copyright 2008, The Android Open Source Project
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

// Max number of fd's we watch at any one time.  Increase if necessary.
#ifndef LIB_VIA_RIL_H
#define LIB_VIA_RIL_H 1

#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <cutils/record_stream.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <pthread.h>
#include <binder/Parcel.h>
#include <cutils/jstring.h>

#include <ril_event.h>
#include <telephony/via-ril.h>

#define RIL_CHANNEL_1     AT_CHANNEL
#define RIL_CHANNEL_2     DATA_CHANNEL
#define RIL_CHANNEL_3     MISC_CHANNEL

extern "C" const char * proxyIdToString(int id);
extern "C" const char * requestToString(int request);

namespace android {

enum WakeType {DONT_WAKE, WAKE_PARTIAL};

typedef struct {
    int requestNumber;
    void (*dispatchFunction) (Parcel &p, struct RequestInfo *pRI);
    int(*responseFunction) (Parcel &p, void *response, size_t responselen);
//#ifdef ADD_MTK_REQUEST_URC
    RILChannelId proxyId;
//#endif /* ADD_MTK_REQUEST_URC */
} CommandInfo;

typedef struct {
    int requestNumber;
    int (*responseFunction) (Parcel &p, void *response, size_t responselen);
    WakeType wakeType;
} UnsolResponseInfo;

typedef struct RequestInfo {
    int32_t token;      //this is not RIL_Token
    CommandInfo *pCI;
    struct RequestInfo *p_next;
    char cancelled;
    char local;         // responses to local commands do not go back to command process
    char owner;         // 0 - normal, 1 - rpc
    RIL_SOCKET_ID socket_id;
#ifdef ADD_MTK_REQUEST_URC
    RILChannelId cid;    // For command dispatch after onRequest()
#endif /* ADD_MTK_REQUEST_URC */
} RequestInfo;

typedef struct UserCallbackInfo {
    RIL_TimedCallback p_callback;
    void *userParam;
    struct ril_event event;
    struct UserCallbackInfo *p_next;
#ifdef ADD_MTK_REQUEST_URC
    RILChannelId cid;    // For command dispatch after onRequest()
#endif /* ADD_MTK_REQUEST_URC */
} UserCallbackInfo;

typedef struct RequestInfoProxy {
    struct RequestInfoProxy *p_next;
    RequestInfo * pRI;
    UserCallbackInfo *pUCI;
    Parcel* p;
    clock_t t;
} RequestInfoProxy;

typedef struct CurExecCmd {
    int requestNum;
    int cid;
    clock_t t;
} CurExecCmd;

#ifdef ADD_MTK_REQUEST_URC

void *proxyLoop(void *param);
int enqueueProxyList(RequestInfoProxy ** pProxyList,
        RequestInfoProxy *pRequest);
RequestInfoProxy * dequeueProxyList(RequestInfoProxy ** pProxyList);
void RIL_startRILProxys(void);

#endif /* ADD_MTK_REQUEST_URC */

static pthread_mutex_t s_wakeTimeoutMutex = PTHREAD_MUTEX_INITIALIZER;

}

typedef struct SocketListenParam {
    RIL_SOCKET_ID socket_id;
    int fdListen;
    int fdCommand;
    char* processName;
    struct ril_event* commands_event;
    struct ril_event* listen_event;
    void (*processCommandsCallback)(int fd, short flags, void *param);
    RecordStream *p_rs;
} SocketListenParam;

#endif /* LIB_VIA_RIL_H */
