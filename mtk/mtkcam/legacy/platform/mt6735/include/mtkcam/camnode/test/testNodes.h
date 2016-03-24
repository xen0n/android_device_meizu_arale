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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MTK_CAMERA_INC_CAMNODE_TESTNODES_H_
#define _MTK_CAMERA_INC_CAMNODE_TESTNODES_H_

#define BUILD_IN_TESTNODE 0
//
//
#include <mtkcam/camnode/ICamGraphNode.h>
#include <mtkcam/camnode/pass1node.h>
#include <mtkcam/camnode/pass2node.h>
/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {


#if BUILD_IN_TESTNODE
/*******************************************************************************
*
********************************************************************************/

#define FAKE_PASS1_INTERMAL 100000 //us
#define FAKE_PASS2_INTERMAL 100000 //us

/*******************************************************************************
*   CamNodes
********************************************************************************/
class TestPass1Node : public Pass1Node
{
    public:     /*createInstance()*/
        static Pass1Node* createInstance(Pass1NodeInitCfg const initcfg);
        virtual void      destroyInstance();
    protected:  /*ctor&dtor*/
                 TestPass1Node();
        virtual  ~TestPass1Node();

    private:
};

//TODO: not implement yet
class TestPass2Node : public Pass2Node
{
    public:     /*createInstance()*/
        static Pass2Node* createInstance(Pass2NodeType const type = PASS2_PREVIEW);
        virtual void      destroyInstance();
    protected:  /*ctor&dtor*/
                 TestPass2Node();
        virtual  ~TestPass2Node();

    private:
};


// print msg when buf is done
class TestDumpNode : public ICamNode
{
    public:     /*createInstance()*/
        static TestDumpNode* createInstance();
        virtual void      destroyInstance();
    protected:  /*ctor&dtor*/
               TestDumpNode();
        virtual ~TestDumpNode();

    private:
};

#else

class TestPass1Node : public Pass1Node
{
    public:     /*createInstance()*/
        static Pass1Node* createInstance(Pass1NodeInitCfg const initcfg)
                            { return NULL;};
        virtual void      destroyInstance() {};
    protected:  /*ctor&dtor*/
                 TestPass1Node() {};
        virtual  ~TestPass1Node() {};

    private:
};

//TODO: not implement yet
class TestPass2Node : public Pass2Node
{
    public:     /*createInstance()*/
        static Pass2Node* createInstance(Pass2NodeType const type = PASS2_PREVIEW)
                            { return NULL; };
        virtual void      destroyInstance() {};
    protected:  /*ctor&dtor*/
                 TestPass2Node() : Pass2Node(PASS2_PREVIEW) {};
        virtual  ~TestPass2Node() {};

    private:
};


// print msg when buf is done
class TestDumpNode : public ICamNode
{
    public:     /*createInstance()*/
        static TestDumpNode* createInstance() {return NULL;};
        virtual void      destroyInstance() {};
    protected:  /*ctor&dtor*/
               TestDumpNode() : ICamNode("TestDumpNode") {};
        virtual ~TestDumpNode() {};

    private:
};

#endif

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //  _MTK_CAMERA_INC_CAMNODE_TESTNODES_H_

