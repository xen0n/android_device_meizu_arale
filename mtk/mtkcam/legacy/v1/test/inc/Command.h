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

#ifndef _MTK_CAMERA_TEST_COMMAND_H_
#define _MTK_CAMERA_TEST_COMMAND_H_
//
/******************************************************************************
*
*******************************************************************************/
    #define HAVE_COMMAND_exit           "exit"                  //  exit
//    #define HAVE_COMMAND_test_surface   "test_surface"          //  test surface
    #define HAVE_COMMAND_property       "property"              //  property
    #define HAVE_COMMAND_test_preview   "test_preview"          //  test preview
    #define HAVE_COMMAND_test_prv_cb    "test_prv_cb"           //  test preview callback
    #define HAVE_COMMAND_test_capture   "test_capture"          //  test capture
    #define HAVE_COMMAND_test_record    "test_record"           //  test record
    #define HAVE_COMMAND_test_params    "test_params"           //  test parameters
    #define HAVE_COMMAND_test_engineer  "test_engineer"         //  test engineer


namespace android {
/******************************************************************************
*
*******************************************************************************/
class CmdBase : public virtual RefBase
{
protected:  ////
    String8 const   ms8CmdName;

public:     ////    Instantiation.
    virtual         ~CmdBase() {}
                    CmdBase(char const* szCmdName)
                        : ms8CmdName(String8(szCmdName))
                    {
                    }

public:     ////    Utils.
    String8 const&  getName() const { return ms8CmdName; }

    bool            parseOneCmdArgument(String8 const& rCmdArg, String8& rKey, String8& rVal)
                    {
                        const char *a = rCmdArg.string();
                        const char *b;
                        //
                        // Find the bounds of the key name.
                        b = ::strchr(a, '=');
                        if (b == 0)
                            return false;

                        // Create the key string.
                        rKey = String8(a, (size_t)(b-a));

                        // Find the value.
                        a = b+1;
                        rVal = String8(a);
                        return  true;
                    }

public:     ////    Interfaces.
    virtual bool    execute(Vector<String8>& rvCmd) = 0;
};


/******************************************************************************
*
*******************************************************************************/
class CmdMap
{
protected:  ////    Data Members.
    typedef DefaultKeyedVector< String8, sp<CmdBase> >  CmdMap_t;
    CmdMap_t        mvCmdMap;

public:     ////    Interfaces.
    static CmdMap&  inst();
    bool            execute(Vector<String8>& rvCmd);
    void            help();
    bool            addCommand(char const*const pszName, CmdBase*const pCmdInstance);

public:     ////    Instantiation.
                    CmdMap();
};


/******************************************************************************
*
*******************************************************************************/
};  // namespace android
#endif  //_MTK_CAMERA_TEST_COMMAND_H_

