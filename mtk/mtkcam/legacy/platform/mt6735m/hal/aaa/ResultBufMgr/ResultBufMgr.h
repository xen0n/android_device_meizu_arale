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
* @file ResultBufMgr.h
* @brief Declarations of Implementation of Result Buffer Manager
*/
#ifndef __RESULT_BUF_MGR_H__
#define __RESULT_BUF_MGR_H__

#include <mtkcam/v3/hal/IHal3A.h>
#include <hal/aaa/aaa_hal_if.h>

#include <utils/Vector.h>
#include <utils/KeyedVector.h>

namespace NS3Av3
{
using namespace android;

class ResultBufMgr
{
public:
    ResultBufMgr(){}
    ResultBufMgr(MUINT32 u4Capacity);
    virtual ~ResultBufMgr();

    virtual MVOID       clearAll();
    virtual MINT32      updateResult(MINT32 i4FrmId, const Result_T& rResult);
    virtual MINT32      getResult(MINT32 i4FrmId, Result_T& rResult);
    virtual Result_T*   getResult(MINT32 i4FrmId);

protected:
    typedef List<Result_T> ResultList_T;

    ResultList_T        m_rResultQueue;
    MUINT32             m_u4Capacity;
    Mutex               m_Lock;
};

}

#endif  //__RESULT_BUF_MGR_H__
