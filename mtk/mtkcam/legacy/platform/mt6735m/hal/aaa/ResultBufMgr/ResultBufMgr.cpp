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

/**
* @file ResultBufMgr.cpp
* @brief Implementation of Result Buffer Manager
*/

#define LOG_TAG "ResultBufMgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_log.h>

#include <hal/aaa/ResultBufMgr/ResultBufMgr.h>

using namespace NS3Av3;
using namespace android;

ResultBufMgr::
ResultBufMgr(MUINT32 u4Capacity)
    : m_u4Capacity(u4Capacity)
    , m_Lock()
{
}

ResultBufMgr::
~ResultBufMgr()
{
    clearAll();
}

MVOID
ResultBufMgr::
clearAll()
{
    Mutex::Autolock lock(m_Lock);
    MY_LOG("[%s] Size(%d)", __FUNCTION__, m_rResultQueue.size());
    m_rResultQueue.clear();
}

MINT32
ResultBufMgr::
updateResult(MINT32 i4FrmId, const Result_T& rResult)
{
    MINT32 i4Ret = -1;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rResultQueue.size();
    ResultList_T::iterator it = m_rResultQueue.begin();
    for (; it != m_rResultQueue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            *it = rResult;
            i4Ret = 1;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist, add
        m_rResultQueue.push_back(rResult);
        i4Ret = 0;
    }

    // remove item
    if (m_rResultQueue.size() > m_u4Capacity)
    {
        Mutex::Autolock lock(m_Lock);
        m_rResultQueue.erase(m_rResultQueue.begin());
    }

    return i4Ret;
}

MINT32
ResultBufMgr::
getResult(MINT32 i4FrmId, Result_T& rResult)
{
    Mutex::Autolock lock(m_Lock);

    MINT32 i4Ret = 0;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rResultQueue.size();
    ResultList_T::iterator it = m_rResultQueue.begin();
    for (; it != m_rResultQueue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            rResult = *it;
            //MY_LOG("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        MY_LOG("[%s] NG i4Pos(%d)", __FUNCTION__, i4Pos);
        i4Ret = -1;
    }

    return i4Ret;
}

Result_T*
ResultBufMgr::getResult(MINT32 i4FrmId)
{
    Mutex::Autolock lock(m_Lock);

    Result_T* pBuf = NULL;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rResultQueue.size();
    ResultList_T::iterator it = m_rResultQueue.begin();
    for (; it != m_rResultQueue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            pBuf = &(*it);
            //MY_LOG("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        MY_LOG("[%s] NG i4Pos(%d)", __FUNCTION__, i4Pos);
    }

    return pBuf;
}
