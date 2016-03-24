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
/*
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/SurfaceComposerClient.h>
*/
//
#include "inc/log.h"
#include "inc/utils.h"
#include "inc/command.h"
//
using namespace android;
//
//
//
//
//
/*
void
sa_handler_exit(int sig)
{
    printf("(sig, SIGUSR1)=(%d, %d) \n", sig, SIGINT);
//    gbRunning = false;
//    printf("test complete. CTRL+C to finish.\n");
}
*/
//
//
//
static
Vector<String8>
queryCommandVector(char const cCmd[], int fd)
{
    Vector<String8> vCommand;
    //
//    String8 s8Buffer(cCmd);
//    ::write(fd, s8Buffer.string(), s8Buffer.size());
    //
    //  (2) Convert command from string to vector: vCommand
    char const *s1 = cCmd, *s2 = 0;
    while ( 0 != s1 && 0 != (*s1) ) {
        if  ( ' ' == (*s1) ) {
            continue;
        }
        //
        s2 = ::strchr(s1, ' ');
        if  ( s2 == 0 ) {
            // If there's no space, this is the last item.
            vCommand.push_back(String8(s1));
            break;
        }
        vCommand.push_back(String8(s1, (size_t)(s2-s1)));
        s1 = s2 + 1;
    }
    //
#if 0
    for (size_t i = 0; i < vCommand.size(); i++)
    {
        String8 s8Temp = String8::format("[%d] %s\n", i, vCommand[i].string());
        ::write(fd, s8Temp.string(), s8Temp.size());
    }
#endif
    //
    return  vCommand;
}
//
//
//
int main(int argc, char** argv)
{
    char cCmds[256] = {0}; 
    //
    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();
    //
    //
/*
    struct sigaction actions;
    ::memset(&actions, 0, sizeof(actions)); 
    ::sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0; 
    actions.sa_handler = sa_handler_exit;
    err = ::sigaction(SIGINT, &actions, NULL);
    printf("sigaction return (%d)""\n", err);
*/
    //
    //
    //
    CmdMap::inst().help();
    while   (1)
    {
        printf("[mmsdktest]$");
        ::fgets(cCmds, sizeof(cCmds), stdin);
        cCmds[::strlen(cCmds)-1] = '\0'; //  remove the '\n' 
        //
        if  ( 0 == ::strcmp(cCmds, "-h") ) {
            CmdMap::inst().help();
            continue;
        }
        //
        Vector<String8> vCmd = queryCommandVector(cCmds, 1);
        if  ( vCmd.empty() ) {
            continue;
        }
        //
        CmdMap::inst().execute(vCmd);
        printf("\n");
    }
    MY_LOGD("exit while""\n");
    //
    //
    IPCThreadState::self()->joinThreadPool();
    return 0;
}

