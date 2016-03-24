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

//
#include <utils/Log.h>
#include <utils/Vector.h>
#include <binder/Parcel.h>
#include <cutils/memory.h>

//
#include "inc/log.h"
#include "inc/utils.h"
#include "inc/command.h"

#include <binder/IServiceManager.h>
#include <mmsdk/IMMSdkService.h>

#include <mmsdk/IHeartrateUser.h>


#if defined(HAVE_COMMAND_client_heartrate)
// 
using namespace android;
using namespace NSMMSdk;
using namespace NSHeartrate;

//
//
//
/******************************************************************************
 *  Command
 *      property <set -k=key -v=value> <get -k=key> <-h>
 *
 *      -h:                     help
 *      <set -k=key -v=value>   set camera property by a pair of key/value
 *      <get -k=key>            get camera property by a key
 *
 ******************************************************************************/
namespace NSCmd_client_heartrate {
struct CmdImp : public CmdBase
{
    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

protected:  ////                Implementation.
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd);

    virtual bool                onHRClientConnect(); 

    virtual bool                onHRClientDisConnect(); 

    virtual bool                onHRClientStart(); 

    virtual bool                onHRClientStop();

    virtual bool                onHRClientQuery();
    virtual bool                onHRClientSetmode();
protected:  ////                Implementation.
    virtual sp<IMMSdkService>     getMMSdkService();
    sp<IHeartrateUser>    mHRClient;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Data Members (Parameters)

};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_client_heartrate, new CmdImp(HAVE_COMMAND_client_heartrate));
};  // NSCmd_client_heartrate
using namespace NSCmd_client_heartrate;


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    return  onParseActionCommand(rvCmd);
}




/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseActionCommand(Vector<String8>& rvCmd)
{
    //  (1) Start to parse ACTION commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        //
        if  ( s8Cmd == "-h" ) {
            String8 text;
            text += "\n";
            text += "\n   property <set -k=key -v=value> <get -k=key> <-h>";
            text += "\n   <-h>:                   help";
            text += "\n   <set -k=key -v=value>   set camera property by a pair of key/value";
            text += "\n   <get -k=key>            get camera property by a key";
            text += "\n";
            text += "\n   ex: set -k=KEY -v=VALUE";
            text += "\n   ex: get -k=KEY";
            MY_LOGD("%s", text.string());
            return  true;
        }
        //
        if  ( s8Cmd == "connect" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onHRClientConnect(); 
        }
        if  ( s8Cmd == "disconnect" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onHRClientDisConnect(); 
        }
        if  ( s8Cmd == "start" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onHRClientStart(); 
        }
        if  ( s8Cmd == "stop" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onHRClientStop(); 
        }
        if  ( s8Cmd == "query" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onHRClientQuery(); 
        }
        if  ( s8Cmd == "setmode" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onHRClientSetmode(); 
        }
        //
    }
    return  false;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onHRClientConnect()
{
    // 
    MMSDK_LOGD("onHRClientConnect++++"); 
    sp<IMMSdkService> cs = getMMSdkService(); 
    MMSDK_LOGD("onHRClientConnect++++2"); 
    
    cs->connectHeartrate(mHRClient); 
    // 
    if (mHRClient == 0)
    {
        MMSDK_LOGE("connect fail"); 
        return false; 
    }

    MMSDK_LOGD("onHRClientConnect- %p", mHRClient.get());

    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onHRClientDisConnect()
{
    // 
    MMSDK_LOGD("onHRClientDisConnect+"); 
    sp<IMMSdkService> cs = getMMSdkService();

    cs->disconnectHeartrate(); 
    // 

    bool ret = false; 

    mHRClient = NULL;
    //

    //
    MMSDK_LOGD("onHRClientDisConnect-"); 

    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onHRClientStart()
{
    // 
    MMSDK_LOGD("onHRClientStart+"); 
    
    mHRClient->start();

    MMSDK_LOGD("onHRClientStart-");  

    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onHRClientStop()
{
    // 
    MMSDK_LOGD("onHRClientStop+");

    mHRClient->stop();

    MMSDK_LOGD("onHRClientStop-"); 

    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onHRClientQuery()
{
    // 
    MMSDK_LOGD("onHRClientQuery+");

    int isdev, ishr;

    mHRClient->query(&isdev, &ishr);

    MMSDK_LOGD("Has Camera? : %d, HR enabled? : %d", isdev, ishr);

    MMSDK_LOGD("onHRClientQuery-"); 

    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onHRClientSetmode()
{
    // 
    MMSDK_LOGD("onHRClientSetmode+");

    static int mode = 0;

    mHRClient->setmode(mode);

    mode = !mode;

    MMSDK_LOGD("onHRClientsetmode-"); 

    return 0; 
}



/******************************************************************************
 *
 ******************************************************************************/
sp<IMMSdkService>
CmdImp::
getMMSdkService()
{
    MMSDK_LOGD("get the service mananger+++");
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == 0)
    {
        MMSDK_LOGE("Can not get the service mananger"); 
    }
    //
    sp<IBinder> binder = sm->getService(String16("media.mmsdk"));
    if (binder == 0)
    {
        MMSDK_LOGE("Can not get mmsdk service"); 
        return 0; 
    }
    //
    sp<IMMSdkService> service = interface_cast<IMMSdkService>(binder);
    if (service == 0)
    {
        MMSDK_LOGE("Null mmsdk service"); 
        return 0; 
    }
    MMSDK_LOGD("get the service mananger---");
    return service;
}



/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

