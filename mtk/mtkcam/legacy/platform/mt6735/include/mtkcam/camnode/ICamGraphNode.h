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
#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_GRAPH_NODE_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_GRAPH_NODE_H_
//
#include <sched.h>
//
#include <mtkcam/camnode/nodeDataTypes.h>
#include <mtkcam/camnode/ICamBufHandler.h>
//
#include <vector>
using namespace std;
//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode{


/*******************************************************************************
*
********************************************************************************/
/* ICamNode */

#define DECLARE_ICAMNODE_INTERFACES()                             \
    protected:  /*ICamNode functions*/                            \
        virtual MBOOL onInit();                                   \
        virtual MBOOL onUninit();                                 \
        virtual MBOOL onStart();                                  \
        virtual MBOOL onStop();                                   \
        virtual MBOOL onNotify(                                   \
                        MUINT32 const msg,                        \
                        MUINT32 const ext1,                       \
                        MUINT32 const ext2                        \
                        );                                        \
        virtual MBOOL onPostBuffer(                               \
                        MUINT32 const data,                       \
                        MUINTPTR const buf,                        \
                        MUINT32 const ext                         \
                        );                                        \
        virtual MBOOL onReturnBuffer(                             \
                        MUINT32 const data,                       \
                        MUINTPTR const buf,                        \
                        MUINT32 const ext                         \
                        );                                        \
        virtual MVOID onDumpBuffer(                               \
                        const char*   usr,                        \
                        MUINT32 const data,                       \
                        MUINTPTR const buf,                        \
                        MUINT32 const ext                         \
                        );

/* ICamThreadNode */

#define DECLARE_ICAMTHREADNODE_INTERFACES()                       \
    protected:  /*ICamNode functions*/                            \
        virtual MBOOL onInit();                                   \
        virtual MBOOL onUninit();                                 \
        virtual MBOOL onStart();                                  \
        virtual MBOOL onStop();                                   \
        virtual MBOOL onNotify(                                   \
                        MUINT32 const msg,                        \
                        MUINT32 const ext1,                       \
                        MUINT32 const ext2                        \
                        );                                        \
        virtual MBOOL onPostBuffer(                               \
                        MUINT32 const data,                       \
                        MUINTPTR const buf,                        \
                        MUINT32 const ext                         \
                        );                                        \
        virtual MBOOL onReturnBuffer(                             \
                        MUINT32 const data,                       \
                        MUINTPTR const buf,                        \
                        MUINT32 const ext                         \
                        );                                        \
        virtual MVOID onDumpBuffer(                               \
                        const char*   usr,                        \
                        MUINT32 const data,                       \
                        MUINTPTR const buf,                        \
                        MUINT32 const ext                         \
                        );                                        \
    protected:  /*Thread functions*/                              \
        virtual MBOOL threadLoopUpdate();      /* loop event   */

/*******************************************************************************
*
********************************************************************************/
//
//
class ICamNodeImpl;
class ICamThreadImpl;
class ICamGraphImpl;
//
class ICamNode
{
    friend class ICamNodeImpl;
    friend class ICamThreadImpl;
    friend class ICamGraphImpl;

    public:

        virtual void    destroyInstance() = 0;

    protected:   ////    Constructor/Destructor.

        ICamNode();
        ICamNode(const char* name, ICamNodeImpl* const impl = NULL);
        virtual ~ICamNode();

    public:

        MBOOL setSensorIdx(MINT32 const idx);

    protected:   ////   common utilities for nodes

        const char* getName() const { return msName; }

        MINT32 getSensorIdx() const;

        MBOOL addDataSupport(
                MBOOL const src_dst,
                MUINT32 const data
                );

        MBOOL addNotifySupport(
                MUINT32 const msg
                );

        MBOOL isDataConnected(
                MUINT32 const data
                ) const;

        vector<MUINT32> getDataConnected() const;

        MBOOL handlePostBuffer(
                MUINT32 const data,
                MUINTPTR const buf,
                MUINT32 const ext = 0
                ) const;

        MBOOL handleReturnBuffer(
                MUINT32 const data,
                MUINTPTR const buf,
                MUINT32 const ext = 0
                ) const;

        MBOOL handleNotify(
                MUINT32 const msg,
                MUINT32 const ext1 = 0,
                MUINT32 const ext2 = 0
                ) const;

        MBOOL handleBroadcast(
                MUINT32 const msg,
                MUINT32 const ext1 = 0,
                MUINT32 const ext2 = 0
                ) const;

        ICamBufHandler* getBufferHandler(MUINT32 const data) const;

    protected:   ////     to be implemented by subclass

        virtual MBOOL onInit()                    = 0;
        virtual MBOOL onUninit()                  = 0;
        virtual MBOOL onStart()                   = 0;
        virtual MBOOL onStop()                    = 0;
        virtual MBOOL onNotify(
                MUINT32 const msg,
                MUINT32 const ext1,
                MUINT32 const ext2
                )                                 = 0;
        virtual MBOOL onPostBuffer(
                MUINT32 const data,
                MUINTPTR const buf,
                MUINT32 const ext
                )                                 = 0;
        virtual MBOOL onReturnBuffer(
                MUINT32 const data,
                MUINTPTR const buf,
                MUINT32 const ext
                )                                 = 0;
        virtual MVOID onDumpBuffer(
                const char*   usr,
                MUINT32 const data,
                MUINTPTR const buf,
                MUINT32 const ext
                ){};

    private:

        const char* const                      msName;

    protected:
        ICamNodeImpl* const                    mpImpl;
};


class ICamGraph
{
    friend class ICamNodeImpl;
    friend class ICamThreadImpl;
    friend class ICamGraphImpl;

    public:

        static ICamGraph*  createInstance(MINT32 const sensorIdx, const char* usr);
        virtual void       destroyInstance();

    private:

        ICamGraph();

    protected:     ////    Constructor/Destructor.

        ICamGraph(MINT32 const sensorIdx, const char* usr);
        virtual ~ICamGraph();

    public:     ////    flow control functions

        MBOOL connectData(
                MUINT32 const updata,
                MUINT32 const downdata,
                ICamNode* const pUpNode,
                ICamNode* const pDownNode
                );
        MBOOL connectNotify(
                MUINT32 const msg,
                ICamNode* const pUpNode,
                ICamNode* const pDownNode
                );
        MBOOL disconnect();
        MBOOL init();
        MBOOL uninit();
        MBOOL start();
        MBOOL stop();
        MBOOL broadcast(
                MUINT32 const msg,
                MUINT32 const ext1,
                MUINT32 const ext2
                );

        //    MBOOL ReplaceNode(ICamNode* const pOld, ICamNode* const pNew);

        MBOOL setBufferHandler(MUINT32 const data, ICamBufHandler* bufHdl, MINT32 idx = -1);

    protected:   ////   utility functions

        const char* getName() const { return msName; }

        MINT32 getSensorIdx() const;

    private:

        const char* const                      msName;
        ICamGraphImpl* const                   mpImpl;
};


typedef enum ThreadNodeType
{
    SingleTrigger     = 0x0,
    ContinuousTrigger = 0x1,
    NO_THREAD         = 0x2,
}eThreadNodeType;


class ICamThreadNode : public ICamNode
{
    friend class ICamNodeImpl;
    friend class ICamThreadImpl;
    friend class ICamGraphImpl;

    private:

        ICamThreadNode();

    protected:     ////    Constructor/Destructor.

        ICamThreadNode(
                const char* name,
                eThreadNodeType type,
                int policy = SCHED_OTHER,
                int priority = 0
                );
        virtual ~ICamThreadNode();

    protected:    //// thread function

        virtual MBOOL threadLoopUpdate() = 0;                  //loop event
        virtual MBOOL threadInit() { return MTRUE; };         //init in thread
        virtual MBOOL threadUninit() { return MTRUE; };       //uninit in thread

    protected:   ////   utility functions

        //  thread control
        eThreadNodeType       getThreadNodeType() const;
        MBOOL                 triggerLoop() const;
        MBOOL                 syncWithThread() const;

    private:

};
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //  _MTK_CAMERA_INCLUDE_CAMNODE_GRAPH_NODE_H_

