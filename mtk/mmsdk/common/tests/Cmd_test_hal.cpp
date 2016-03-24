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
#include <hardware/hardware.h>
#include <mmsdk_common.h>

//
#include "inc/log.h"
#include "inc/utils.h"
#include "inc/command.h"


#if defined(HAVE_COMMAND_hal)

#define DEVICE_MAJOR_VERSION(a)   ((a >> 8)& 0xff)
#define DEVICE_MINOR_VERSION(a)   ((a & 0xff))

// 
using namespace android;
//
//
//
/******************************************************************************
 *  Command
 *      service <client> <-h>
 *
 *      -h:                     help
 *      <client>   client connect test 
 *
 ******************************************************************************/
namespace NSCmd_hal {
class CmdImp : public CmdBase
{
//
private:
    mmsdk_module_t              *mModule; 

    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                    , mModule(0)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

protected:  ////                Implementation.
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd);

    virtual bool                onOpen(); 
protected:  ////                Implementation.
    virtual bool                checkHALModuleVersion(); 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Data Members (Parameters)


};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_hal, new CmdImp(HAVE_COMMAND_hal));
};  // NSCmd_test_preview
using namespace NSCmd_hal;


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
            text += "\n   service <client> <-h>";
            text += "\n   <-h>:                   help";
            text += "\n   <client>  client connect test";
            text += "\n";
            MY_LOGD("%s", text.string());
            return  true;
        }
        //
        if  ( s8Cmd == "open" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onOpen(); 
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
onOpen()
{
    // 
    MY_LOGD("onOpen"); 
    checkHALModuleVersion(); 
    // 
    int numDevices = 0; 
    mmsdk_device_info *info; 
    mModule->get_supported_device_info(numDevices, &info); 
    //
    for (int i = 0; i < numDevices; i++)
    {
        //
        MY_LOGD("Device:%d, name:%s, version:%d.%d", i, info[i].device_name, 
                            DEVICE_MAJOR_VERSION(info[i].device_version), 
                            DEVICE_MINOR_VERSION(info[i].device_version)); 
    }

   
    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
bool 
CmdImp::
checkHALModuleVersion()
{
    //
    if (::hw_get_module(MMSDK_HARDWARE_MODULE_ID, 
           (const hw_module_t **)&mModule) < 0)
    {
        MY_LOGE("Could not load mmsdk HAL module");
        return false; 
    }
    // 
    MY_LOGD("Load \"%s\" mmsdk moudle", mModule->common.name); 
    //
    if (mModule->common.module_api_version == MMSDK_MODULE_API_VERSION_1_0)
    {
        MY_LOGD("MMSDK API is 1.0"); 
    }
    return true; 

}


/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

