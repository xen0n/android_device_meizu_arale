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

/* //device/libs/telephony/ril.cpp
**
** Copyright 2006, The Android Open Source Project
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

#include <telephony/ril.h>
#include <utils/Log.h>
#include <libc2kril.h>

const char* proxyIdToString(int id) {
    switch (id) {
    case AT_CHANNEL:
        return "AT_CHANNEL";
    case DATA_CHANNEL:
        return "DATA_CHANNEL";
    case MISC_CHANNEL:
        return "MISC_CHANNEL";
    default:
        return "Unknown channel";
    }
}

namespace android {

extern pthread_mutex_t s_dispatchMutex;
extern pthread_cond_t s_dispatchCond;
extern pthread_mutex_t s_queryThreadMutex;

extern android::RequestInfoProxy* s_Proxy[];
extern pthread_t s_tid_proxy[];

int getRilProxysNum()
{
    return RIL_SUPPORT_CHANNELS;
}

int enqueueProxyList(android::RequestInfoProxy ** pProxyList, android::RequestInfoProxy *pRequest)
{
    LOGI("enqueueProxyList &pProxyList=%d, &s_Proxy[0]=%d, &s_Proxy[1]=%d", pProxyList, &s_Proxy[0], &s_Proxy[1]);
    android::RequestInfoProxy ** ppCur = pProxyList;
    int i = 0;
    int ret;

    pRequest->p_next = NULL;

    /* Multiple channel support */
    ret = pthread_mutex_lock(&s_dispatchMutex);
    assert(ret == 0);

    if (*ppCur == NULL) /* No pending task */
    {
        *ppCur = pRequest;
        if (pRequest->pRI != NULL) {
            LOGD("%s dispatched to %s",
                    ::requestToString(pRequest->pRI->pCI->requestNumber),
                    ::proxyIdToString(pRequest->pRI->cid));
        } else if (pRequest->pUCI != NULL) {
            LOGD("Timed callback dispatched to %s",
                    ::proxyIdToString(pRequest->pUCI->cid));
        }
        pthread_cond_broadcast(&s_dispatchCond);
    } else {
        for (; *ppCur != NULL; ppCur = &((*ppCur)->p_next)) {
            /* Travel to the last one */;
            if ((*ppCur)->pRI != NULL) {
                LOGD("%d:%s is queued in %s", ++i,
                        ::requestToString((*ppCur)->pRI->pCI->requestNumber),
                        ::proxyIdToString((*ppCur)->pRI->cid));
            } else if ((*ppCur)->pUCI != NULL) {
                LOGD("%d:Timed callback is queued in  %s", ++i,
                        ::proxyIdToString((*ppCur)->pUCI->cid));
            }
        }
        *ppCur = pRequest;
    }

    ret = pthread_mutex_unlock(&s_dispatchMutex);
    assert(ret == 0);

    return i; /* how many requests in queue */
}

/*
 * This function should be encompassed by mutex s_dispatchMutex
 * to protect pProxyList manipulation (Only called by proxyLoop)
 */
android::RequestInfoProxy * dequeueProxyList(android::RequestInfoProxy ** pProxyList)
{
    android::RequestInfoProxy * pCur = *pProxyList;
    int ret;

    if (pCur != NULL)
    {
        /* Move mutex protection outside to protect atomic query */

//        ret = pthread_mutex_lock(&s_dispatchMutex);
//        assert(ret == 0);

        *pProxyList =  pCur->p_next;

//        ret = pthread_mutex_unlock(&s_dispatchMutex);
//        assert(ret == 0);
    }
    LOGI("proxy deque done");
    return pCur;
}

void *proxyLoop(void *param)
{
    int ret;
    android::RequestInfoProxy *proxy = NULL;
    int i; /* proxyid */
    const char * proxyName = NULL;
    int proxysNum = getRilProxysNum();
    
    /* Who am I */
    for (i=0; i<proxysNum; i++)
    {
        if (&s_Proxy[i] == (android::RequestInfoProxy **)param)
            break;
    }

    assert(i<proxysNum);
    proxyName = ::proxyIdToString(i);
    LOGI("proxy loop index i=%d, name=%s", i, proxyName);

    do
    {
        /* Self check */
        assert(pthread_equal(s_tid_proxy[i], pthread_self()));

        /* Obtain dispatchMutex for proxyList here */
        ret = pthread_mutex_lock(&s_dispatchMutex);
        assert(ret == 0);

        proxy = dequeueProxyList(&s_Proxy[i]);

        if (proxy != NULL)
                {
                    /* Release dispatchMutex for next blocking call.
                     * We don't want the dispatcher to wait full command execution. */
                    ret = pthread_mutex_unlock(&s_dispatchMutex);
                    assert(ret == 0);

                    if (proxy->pRI != NULL)
                    {
                        LOGD("%s execute on %s using channel %d",::requestToString(proxy->pRI->pCI->requestNumber),proxyName,proxy->pRI->cid);

                        proxy->pRI->pCI->dispatchFunction(*proxy->p, proxy->pRI); /* Blocking call */
                        ret = pthread_mutex_lock(&s_dispatchMutex);
                        assert(ret == 0);
                        delete proxy->p;
                        ret = pthread_mutex_unlock(&s_dispatchMutex);
                        assert(ret == 0);
                    }
                    else if (proxy->pUCI != NULL)
                    {
                        LOGD("Timed callback execute on %s using channel %d, tid:%lu",proxyName,proxy->pUCI->cid, pthread_self());

                        proxy->pUCI->p_callback(proxy->pUCI->userParam);

                        free(proxy->pUCI);
                    }
                    free(proxy);
                }
                else
                {
                    /* No pending pRIs sleep */
                    LOGI("%s sleep",proxyName);
                    //ret = pthread_mutex_lock(&s_dispatchMutex);
                    //assert(ret == 0);

                    pthread_cond_wait(&s_dispatchCond, &s_dispatchMutex);

                    ret = pthread_mutex_unlock(&s_dispatchMutex);
                    assert(ret == 0);

                    LOGI("%s wakeup",proxyName);
                }


    } while(1);

    return NULL;
}

void RIL_startRILProxys(void)
{
    int ret;
    int i;
    int proxysNum = getRilProxysNum();
    
    for (i=0; i<proxysNum; i++)
    {
        pthread_attr_t attr;
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        ret = pthread_create(&s_tid_proxy[i], &attr, proxyLoop,(void*) &s_Proxy[i]);


        if (ret < 0) {
            RLOGE("Failed to create proxy thread errno:%d", errno);
            assert(0);
            return;
        }

        LOGD("proxy %s is up, tid:%d",::proxyIdToString(i),(int) s_tid_proxy[i]);
    }

}
}
