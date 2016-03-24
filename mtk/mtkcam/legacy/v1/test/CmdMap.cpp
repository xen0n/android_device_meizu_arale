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

#include <utils/Log.h>
#include <cutils/memory.h>
//
//#include <dlfcn.h>
//
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

//
#include <camera/ICamera.h>
#include <camera/CameraParameters.h>
//
#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/Command.h"
//
using namespace android;
//
//
/******************************************************************************
 *
 ******************************************************************************/
CmdMap&
CmdMap::
inst()
{
    static CmdMap inst;
    return inst;
}


/******************************************************************************
 *
 ******************************************************************************/
CmdMap::
CmdMap()
    : mvCmdMap(0)
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdMap::
addCommand(char const*const pszName, CmdBase*const pCmdInstance)
{
    mvCmdMap.add(String8(pszName), pCmdInstance);
    printf("    +=: <%s>\n", pszName);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdMap::
execute(Vector<String8>& rvCmd)
{
    sp<CmdBase> pCmdBase = mvCmdMap.valueFor(*rvCmd.begin());
    if  (pCmdBase != 0)
    {
//                            printf("<%s>\n", pCmdBase->getName().string());
        return  pCmdBase->execute(rvCmd);
    }
    MY_LOGW("no command found!");
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdMap::
help()
{
    printf("\n");
    printf("    ++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("    +                     MENU                         +\n");
    printf("    ++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("    Command: <-h>\n");
    for (size_t i = 0; i < mvCmdMap.size(); i++)
    {
        printf("    Command: <%s>\n", mvCmdMap.keyAt(i).string());
    }
    printf("    ....................................................\n");
}

