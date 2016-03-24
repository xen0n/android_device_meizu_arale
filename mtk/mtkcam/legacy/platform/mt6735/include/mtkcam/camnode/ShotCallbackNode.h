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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_SHOTCALLBACKNODES_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_SHOTCALLBACKNODES_H_
//
//
//
#include <mtkcam/camnode/nodeDataTypes.h>
#include <mtkcam/camnode/ICamGraphNode.h>
/*******************************************************************************
*
********************************************************************************/


/*******************************************************************************
*
********************************************************************************/

namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
enum ENodeNotifyMsg
{
    ENode_NOTIFY_MSG_NONE      = 0x0000, /* !< none notify message                          */
    ENode_NOTIFY_MSG_SHUTTER   = 0x0001, /* !< shutter notify  message                      */
    ENode_NOTIFY_MSG_ALL       = 0x0002, /* !< all notify  message                          */
};

enum ENodeDataMsg
{
    ENode_DATA_MSG_NONE        = 0x0000, /* !< none data  callback message                  */
    ENode_DATA_MSG_RAW         = 0x0001, /* !< raw data callback message                    */
    ENode_DATA_MSG_YUV         = 0x0002, /* !< yuv data callback message                    */
    ENode_DATA_MSG_POSTVIEW    = 0x0004, /* !< postview data callback message               */
    ENode_DATA_MSG_JPEG        = 0x0008, /* !< jpeg data(including header) callback message */
    ENode_DATA_MSG_EISDATA     = 0x0010, /* !< eis data callback message                    */
    ENode_DATA_MSG_JPS         = 0x0020, /* !< jps data(including header) callback message */
    ENode_DATA_MSG_ALL         = 0x003F, /* !< all data callback message                    */
};

/*******************************************************************************
*
********************************************************************************/
/**
 * @struct NodeNotifyInfo
 * @brief This structure is the node notify info
 *
 */
struct NodeNotifyInfo
{
    /**
      * @var msgType
      * The notify message type of the node
      */
    MUINT32     msgType;
    /**
      * @var ext1
      * The extended parameter 1.
      */
    MUINT32     ext1;
    /**
      * @var ext2
      * The extended parameter 2.
      */
    MUINT32     ext2;
    //
    NodeNotifyInfo(
        MUINT32 const _msgType = 0,
        MUINT32 const _ext1 = 0,
        MUINT32 const _ext2 = 0
    )
        : msgType(_msgType)
        , ext1(_ext1)
        , ext2(_ext2)
    {}
};

/**
 * @typedef  NodeNotifyCallback_t
 * @brief this is the prototype of node notify callback function pointer.
 *
 */
typedef MBOOL   (*NodeNotifyCallback_t)(MVOID* user, NodeNotifyInfo const msg);


/**
 * @struct NodeDataInfo
 * @brief This structure is the node data info
 *
 */
struct NodeDataInfo
{
    /**
      * @var msgType
      * The data message type of the node
      */
    MUINT32     msgType;
    /**
      * @var ext1
      * The extended parameter 1.
      */
    MUINTPTR    ext1;
    /**
      * @var ext2
      * The extended parameter 2.
      */
    MUINTPTR    ext2;
    /**
      * @var pBuffer
      * Pointer to the callback data.
      */
    IImageBuffer*  pBuffer;
    //
    NodeDataInfo(
        MUINT32 const _msgType     = 0,
        MUINTPTR const _ext1       = 0,
        MUINTPTR const _ext2       = 0,
        IImageBuffer* const _pData = NULL
    )
        : msgType(_msgType)
        , ext1(_ext1)
        , ext2(_ext2)
        , pBuffer(_pData)
    {
    }
};

/**
 * @typedef  NodeDataCallback_t
 * @brief this is the prototype of node data callback function pointer.
 *
 */
typedef MBOOL   (*NodeDataCallback_t)(MVOID* user, NodeDataInfo const msg);


/*******************************************************************************
*
********************************************************************************/
class ShotCallbackNode : public ICamThreadNode
{
    public:     /*createInstance()*/
        static ShotCallbackNode* createInstance(MBOOL createThread);
        void                     destroyInstance();

    protected:  /*ctor&dtor*/
                 ShotCallbackNode(MBOOL createThread);
        virtual  ~ShotCallbackNode() {};

    public:
    /**
     * @brief set the notify/data callbacks
     *
     * @details
     *
     * @note
     *
     * @param[in] notify_cb: The notify callback function
     *
     * @param[in] data_cb: The data callback fucntion
     *
     * @param[in] user: The caller
     **/
        virtual MVOID   setCallbacks(NodeNotifyCallback_t notify_cb,
                                     NodeDataCallback_t data_cb,
                                     MVOID* user)                    = 0;

        virtual MVOID   enableNotifyMsg(MUINT32 const msg)           = 0;
        virtual MVOID   enableDataMsg(MUINT32 const msg)             = 0;
        virtual MVOID   disableNofityMsg(MUINT32 const msg)          = 0;
        virtual MVOID   disableDataMsg(MUINT32 const msg)            = 0;

        virtual MBOOL   stop()                                       = 0;

    private:

};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //  _MTK_CAMERA_INCLUDE_CAMNODE_SHOTCALLBACKNODES_H_

