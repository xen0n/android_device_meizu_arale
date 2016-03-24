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
#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_MFLLNODES_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_MFLLNODES_H_
//
//
//#include <semaphore.h>
#include <utils/StrongPointer.h>
using namespace android;
//
#include <mtkcam/camnode/ICamGraphNode.h>
#include <mtkcam/featureio/eis_hal.h>

/*******************************************************************************
*
********************************************************************************/


/*******************************************************************************
*   foward declaration
********************************************************************************/
namespace NSCam{
    namespace NSIoPipe{
        namespace NSPostProc{
            class IHalPostProcPipe;
        }
    }
}
using NSCam::NSIoPipe::NSPostProc::IHalPostProcPipe;

namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
typedef MBOOL   (*NodeCallback_t)(MVOID* user);


/*******************************************************************************
*   CamNodes
********************************************************************************/

//class MfllNode : public ICamThreadNode
class MfllNode : public ICamNode
{
    public:     /*createInstance()*/
        static MfllNode*    createInstance() {return new MfllNode();}
        virtual void        destroyInstance() {delete this;}

    protected:  /*ctor&dtor*/
                            MfllNode();
        virtual             ~MfllNode();

    DECLARE_ICAMNODE_INTERFACES();

    // extended functions
    public:
        virtual void        setCallback(MVOID *user, NodeCallback_t callback);

    private:
        MUINT32             mPass1Cnt;
        MUINT32             mPass2Cnt;
        //
        MVOID               *mUser;
        NodeCallback_t      mCallback;
        // EIS
        EisHal              *mpEisHal;
};


class DumpNode : public ICamNode
{
    public:     /*createInstance()*/
        static DumpNode*    createInstance() {return new DumpNode();}
        virtual void        destroyInstance() {delete this;}

    protected:  /*ctor&dtor*/
                            DumpNode() : ICamNode("DumpNode") {
                                //
                                addDataSupport( ENDPOINT_SRC, PASS1_FULLRAW);
                                addDataSupport( ENDPOINT_SRC, PASS1_RESIZEDRAW);
                                //
                                addDataSupport( ENDPOINT_SRC, PASS2_PRV_DST_0); //@TODO - this or NODECAT_BASIC_NOTIFY
                                addDataSupport( ENDPOINT_SRC, PASS2_PRV_DST_1); //@TODO - this or NODECAT_BASIC_NOTIFY
                                addDataSupport( ENDPOINT_SRC, PASS2_CAP_DST_0); //@TODO - this or NODECAT_BASIC_NOTIFY
                                addDataSupport( ENDPOINT_SRC, PASS2_CAP_DST_1); //@TODO - this or NODECAT_BASIC_NOTIFY
                            }
                            ~DumpNode() {};

    DECLARE_ICAMNODE_INTERFACES();
    private:
        MUINT32  count;
};

class TestPass1Node : public ICamThreadNode
{
    public:     /*createInstance()*/
        static TestPass1Node* createInstance()
                          {
                              return new TestPass1Node();
                          }
        virtual void      destroyInstance()
                          {
                              delete this;
                          }
    protected:  /*ctor&dtor*/
               TestPass1Node() : ICamThreadNode("TestPass1Node",ContinuousTrigger,0, 0)
                               , mPort(PASS1_FULLRAW)
                               , mSize(320,240)
               {
                   addDataSupport(ENDPOINT_DST, PASS1_FULLRAW);
                   addDataSupport(ENDPOINT_DST, PASS1_RESIZEDRAW);
               }
               ~TestPass1Node() {}

    DECLARE_ICAMTHREADNODE_INTERFACES();

    public:

        MVOID useData(MUINT32 w, MUINT32 h, MUINT32 data)
        {
            mSize = MSize(w,h);
            mPort = data;
        }

    private:
    MUINT32  count;
    MUINT32  mPort;
    MSize    mSize;
};
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode


#endif  //  _MTK_CAMERA_INCLUDE_CAMNODE_MFLLNODES_H_

