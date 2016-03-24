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
//! \file  mdp_mgr.h

#ifndef _MDP_MGR_H_
#define _MDP_MGR_H_

#include "DpIspStream.h"
#include "DpVEncStream.h"


#define PLANE_NUM 3


/**
*@enum eMDPMGR_OUTPORT_INDEX
*/
typedef enum eMDPMGR_OUTPORT_INDEX
{
    ISP_MDP_DL_WDMAO = 0,
    ISP_MDP_DL_WROTO,
    ISP_MDP_DL_JPEGO,
    ISP_MDP_DL_VENCO = ISP_MDP_DL_JPEGO,
    ISP_MDP_DL_IMGXO = 3,
    ISP_MDP_DL_NUM
}MDPMGR_OUTPORT_INDEX;


/**
*@enum eMDPMGR_DEQUEUE_INDEX
*/
typedef enum eMDPMGR_DEQUEUE_INDEX
{
    MDPMGR_DEQ_SRC = 0,
    MDPMGR_DEQ_WDMA,
    MDPMGR_DEQ_WROT,
    MDPMGR_DEQ_JPEG,
    MDPMGR_DEQ_VENCO = MDPMGR_DEQ_JPEG,
    MDPMGR_DEQ_IMGXO = 4,
    MDPMGR_DEQ_NUM
}MDPMGR_DEQUEUE_INDEX;


/**
*@enum eMDPMGR_OUTPORT_INDEX
*/
typedef enum eMDPMGR_RETURN_TYPE
{
    MDPMGR_VSS_NOT_READY    =  1,
    MDPMGR_NO_ERROR         =  0,
    MDPMGR_API_FAIL         = -1,
    MDPMGR_DPFRAMEWORK_FAIL = -2,
    MDPMGR_NULL_OBJECT      = -3,
    MDPMGR_WRONG_PARAM      = -4,
    MDPMGR_STILL_USERS      = -5,
}MDPMGR_RETURN_TYPE;



/**
 *@struct MDPMGR_CFG_STRUCT_s
 *@brief  Used for store MdpMgr configure information
*/
typedef struct MDPMGR_CFG_STRUCT_s
{
    MUINT32 drvScenario;                    //! ISP driver scenario ID
    MUINT32 cqIdx;                          //! index of pass2 cmdQ
    MBOOL   isWaitBuf;                      //! wait for buffer ready
    EImageFormat mdpSrcFmt;                 //! format of input image
    MUINT32 mdpSrcW;                        //! width of mdp input image
    MUINT32 mdpSrcH;                        //! height of mdp input image
    MUINT32 mdpSrcYStride;                  //! Y stride of mdp input image
    MUINT32 mdpSrcUVStride;                 //! UV stride of mdp input image
    MUINT32 mdpSrcBufSize;                  //! buffer size of mdp input image
    MUINT32 mdpSrcCBufSize;                 //! buffer size of mdp input image
    MUINT32 mdpSrcVBufSize;                 //! buffer size of mdp input image
    MUINT32 mdpSrcPlaneNum;                    //! source plane number
    MUINT32 srcVirAddr;                     //! virtual address of input image
    MUINT32 srcPhyAddr;                     //! physical address (MVA) of input image
    MINT32  imgxoEnP2;                      //! record which isp dma be set as imgxo

//Jpg parameter
    JpgParaCfg dstJpgParaCfg;
    MUINT32 dstPortCfg[ISP_MDP_DL_NUM];     //! index of which output DMA port is configured
    MdpRotDMACfg dstDma[ISP_MDP_DL_NUM];    //! configure dma infomation of mdp
    MUINT32 dstRotation;                    //! rotaion angle of output image
    MBOOL   dstFlip;                        //! flip or not of output image
    MUINT32 srcCropX;                       //! X integer start position for cropping
    MUINT32 srcCropFloatX;                  //! X float start position for cropping
    MUINT32 srcCropY;                       //! Y integer start position for crpping
    MUINT32 srcCropFloatY;                  //! Y float start position for cropping
    MUINT32 srcCropW;                       //! width of cropped image
    MUINT32 srcCropH;                       //! height of cropped image
    MUINT32 dstCropH;                       //! height of cropped image
    ISP_TPIPE_CONFIG_STRUCT ispTpipeCfgInfo;    //! Tpipe configure infomation
}MDPMGR_CFG_STRUCT;

/**
 *@class MdpMgr
 *@brief MDP manager for IspFunction
*/
class MdpMgr
{
    public :

        /**
              *@brief MdpMgr constructor
             */
        MdpMgr () {};


        /**
              *@brief Create MdpMgr Object
             */
        static MdpMgr *createInstance( void );

        /**
               *@brief Destory MdpMgr Object
             */
        virtual MVOID destroyInstance( void );

        /**
               *@brief Initialize function
               *@note Must call this function after createInstance and before other functions
               *
               *@return
               *-MTURE indicates success, otherwise indicates fail
             */
        virtual MINT32 init () = 0;

        /**
               *@brief Uninitialize function
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 uninit() = 0;

        /**
               *@brief Configure and trigger MDP
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 startMdp(MDPMGR_CFG_STRUCT cfgData) = 0;

        /**
               *@brief Stop MDP
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 stopMdp(MDPMGR_CFG_STRUCT cfgData) = 0;

        /**
               *@brief Dequeue input or output buffer
               *
               *@param[in] deqIndex : indicate input or output
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 dequeueBuf(MDPMGR_DEQUEUE_INDEX deqIndex, MDPMGR_CFG_STRUCT cfgData) = 0;

        /**
               *@brief execute it at end of deque buffer
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 dequeueFrameEnd(MDPMGR_CFG_STRUCT cfgData) = 0;

        /**
               *@brief get the real size of jpeg
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 getJPEGSize(MDPMGR_CFG_STRUCT cfgData) = 0;
        /**
               *@brief notify that start video record for slow motion
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MBOOL   startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps,MUINT32 drvScenario,MUINT32 cqIdx)=0;
        /**
               *@brief notify that stop video record for slow motion
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MBOOL   stopVideoRecord(MUINT32 drvScenario,MUINT32 cqIdx)=0;

    protected:
        /**
              *@brief MdpMgr destructor
             */
        virtual ~MdpMgr() {};
};

#endif

