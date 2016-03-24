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

/**
* @file mdp_mgr_FrmB.h
*
* mdp_mgr Header File
*/

#ifndef _MDP_MGR_FRMB_H_
#define _MDP_MGR_FRMB_H_

#include "DpIspStream.h"
using namespace NSCam;

#define PLANE_NUM 3


/**
*@brief eMDPMGR_OUTPORT_INDEX_FRMB
*/
typedef enum eMDPMGR_OUTPORT_INDEX_FRMB
{
    ISP_MDP_DL_WDMAO = 0,
    ISP_MDP_DL_WROTO,
    ISP_MDP_DL_JPEGO,
    ISP_MDP_DL_VENCO = ISP_MDP_DL_JPEGO,
    ISP_MDP_DL_IMGXO = 3,
    ISP_MDP_DL_NUM
    //

}MDPMGR_OUTPORT_INDEX_FRMB;

/**
*@enum eMDPMGR_DEQUEUE_INDEX_FRMB
*/
typedef enum eMDPMGR_DEQUEUE_INDEX_FRMB
{
    MDPMGR_DEQ_SRC = 0,
    MDPMGR_DEQ_WDMA,
    MDPMGR_DEQ_WROT,
    MDPMGR_DEQ_JPEG,
    MDPMGR_DEQ_VENCO = MDPMGR_DEQ_JPEG,
    MDPMGR_DEQ_IMGXO = 4,
    MDPMGR_DEQ_NUM
}MDPMGR_DEQUEUE_INDEX_FRMB;


/**
*@enum eMDPMGR_RETURN_TYPE_FRMB
*/
typedef enum eMDPMGR_RETURN_TYPE_FRMB
{
    MDPMGR_VSS_NOT_READY    =  1,
    MDPMGR_NO_ERROR         =  0,
    MDPMGR_API_FAIL         = -1,
    MDPMGR_DPFRAMEWORK_FAIL = -2,
    MDPMGR_NULL_OBJECT      = -3,
    MDPMGR_WRONG_PARAM      = -4,
    MDPMGR_STILL_USERS      = -5,

    MDPMGR_VSS_DEQ_FALSE_FAIL  = -6,  // fix it later
}MDPMGR_RETURN_TYPE_FRMB;


/**
 *@struct MDPMGR_CFG_STRUCT_s_FRMB
 *@brief  Used for store MdpMgr configure information
*/
typedef struct MDPMGR_CFG_STRUCT_s_FRMB
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
    //
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

    //
    // remove it below later (v1)
    MUINT32 sceID;                          //! scenario ID of ISP
    //MUINT32 cqIdx;                          //! index of pass2 cmdQ
    //EImageFormat srcFmt;                    //! format of input image
    MUINT32 srcW;                           //! width of input image
    MUINT32 srcH;                           //! height of input image
    MUINT32 srcStride;                      //! stride of input image
    //MUINT32 srcVirAddr;                     //! virtual address of input image
    //MUINT32 srcPhyAddr;                     //! physical address (MVA) of input image
    MUINT32 srcBufSize;                     //! buffer size of input image
    MUINT32 srcBufMemID;                    //! memory ID of input image
    //MUINT32 dstPortCfg[ISP_MDP_DL_NUM];     //! index of which output DMA port is configured
    //CdpRotDMACfg dstDma[ISP_MDP_DL_NUM];    //! configure infomation of dispo and vido
    //MUINT32 dstRotation;                    //! rotaion angle of output image
    //MBOOL   dstFlip;                        //! flip or not of output image
    MUINT32 dstVirAddr[ISP_MDP_DL_NUM];     //! virtual address of output image
    MUINT32 dstPhyAddr[ISP_MDP_DL_NUM];     //! physicla address (MVA) of output image
    MUINT32 dstBufSize[ISP_MDP_DL_NUM];     //! buffer size of output image
    MUINT32 dstBufMemID[ISP_MDP_DL_NUM];    //! memory ID of output image
    MUINT32 dstCropX;                       //! X integer start position for cropping
    MUINT32 dstCropFloatX;                  //! X float start position for cropping
    MUINT32 dstCropY;                       //! Y integer start position for crpping
    MUINT32 dstCropFloatY;                  //! Y float start position for cropping
    MUINT32 dstCropW;                       //! width of cropped image
    //MUINT32 dstCropH;                       //! height of cropped image
    //ISP_TPIPE_CONFIG_STRUCT ispTpipeCfgInfo;    //! Tpipe configure infomation

}MDPMGR_CFG_STRUCT_FRMB;


/**
 *@brief MDP manager for IspFunction
*/
class MdpMgr_FrmB
{
    public :

        /**
              *@brief MdpMgr constructor
             */
        MdpMgr_FrmB () {};

        /**
              *@brief Create MdpMgr Object
             */
        static MdpMgr_FrmB *createInstance();

        /**
               *@brief Destory MdpMgr Object
             */
        virtual MVOID destroyInstance() = 0;

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
        virtual MINT32 startMdp(MDPMGR_CFG_STRUCT_FRMB cfgData) = 0;

        /**
               *@brief Stop MDP
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 stopMdp(MDPMGR_CFG_STRUCT_FRMB cfgData) = 0;

        /**
               *@brief Dequeue input or output buffer
               *
               *@param[in] deqIndex : indicate input or output
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 dequeueBuf(MDPMGR_DEQUEUE_INDEX_FRMB deqIndex, MDPMGR_CFG_STRUCT_FRMB cfgData) = 0;

        /**
               *@brief execute it at end of deque buffer
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 dequeueFrameEnd(MDPMGR_CFG_STRUCT_FRMB cfgData) = 0;

    protected:
        /**
              *@brief MdpMgr destructor
             */
        virtual ~MdpMgr_FrmB() {};
};

#endif  //_MDP_MGR_FRMB_H_

