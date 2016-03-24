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

/**
* @file FeaturePipe.EIS.h
*
* FeaturePipe EIS header file
*
*/

#ifndef _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_EIS_H_
#define _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_EIS_H_

namespace NSCamNode
{
/**
*@brief FeaturePipeEisNode
*/
class FeaturePipeEisNode : public FeaturePipeNode
{
public:

    /**
        *@brief Create FeaturePipeEisNode object
        *@param[in] featureOnMask : feature combination
        *@param[in] imageWidth : image's width
        *@param[in] imageHeight : image's height
        *@return
        *-FeaturePipeEisNode object
       */
    static FeaturePipeEisNode *createInstance(MUINT32 featureOnMask,MUINT32 mOpenedSensorIndex);

    /**
        *@brief Destroy FeaturePipeEisNode object
       */
    virtual MVOID destroyInstance();

    /**
         *@brief Configure EIS
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] aEisConfig : EIS config data
         *@return
         *-MTRUE indicates success, otherwise indicates fail
       */
    MBOOL ConfigEis(const EIS_PASS_ENUM &aEisPass,const EIS_HAL_CONFIG_DATA &aEisConfig);

    /**
         *@brief Get EIS algorithm result (CMV)
         *@param[out] a_CMV_X_Int : EIS algo result of X-direction integer part
         *@param[out] a_CMV_X_Flt  : EIS algo result of X-direction float part
         *@param[out] a_CMV_Y_Int : EIS algo result of Y-direction integer part
         *@param[out] a_CMV_Y_Flt  : EIS algo result of Y-direction float part
         *@param[out] a_TarWidth    : EIS width crop size
         *@param[out] a_TarHeight   : EIS height crop size
       */
    MVOID GetEisResult(MUINT32 &a_CMV_X_Int,
                          MUINT32 &a_CMV_X_Flt,
                          MUINT32 &a_CMV_Y_Int,
                          MUINT32 &a_CMV_Y_Flt,
                          MUINT32 &a_TarWidth,
                          MUINT32 &a_TarHeight);

    /**
         *@brief Get EIS GMV
         *@details The value is 256x
         *@param[out] aGMV_X : x-direction global motion vector between two frames
         *@param[out] aGMV_Y : y-direction global motion vector between two frames
         *@param[out] aEisInW  : width of EIS input image (optional)
         *@param[out] aEisInH  : height of EIS input image (optional)
       */
    MVOID GetEisGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *aEisInW=NULL,MUINT32 *aEisInH=NULL);

    /**
         *@brief Get FEO register setting
         *@param[in,out] aFeoRegInfo : FEO_REG_INFO
       */
    MVOID GetFeoRegInfo(FEO_REG_INFO *aFeoRegInfo);

    /**
         *@brief Get EIS plus crop ratio
         *@return
         *-EIS plus crop ratio
       */
    MUINT32 GetEisPlusCropRatio();

    /**
         *@brief Get EIS plus target size
         *@param[out] aWidth : target width
         *@param[out] aHeight : target height
         *@return
         *-EIS plus crop ratio
       */
    MVOID GetEisPlusTargetSize(MUINT32 &aWidth,MUINT32 &aHeight);

    /**
         *@brief Get EIS pass1 result at pass2
         *@param[in] userName : user name
         *@param[in,out] apEisResult : pointer to EIS_P1_RESULT_INFO
         *@param[in] aTimeStamp : time stamp of current frame
         *@param[in] lastUser : last user or not
       */
    MVOID GetEisInfoAtP2(char const *userName,EIS_P1_RESULT_INFO *apEisResult,const MINT64 &aTimeStamp,MBOOL lastUser);

    /**
         *@brief  Flush memory
         *@param[in] aDma : DMA port : EISO or FEO
          *@param[in] aFlush : flush type
       */
    MVOID FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush);

protected:   // to be implemented by subclass

    /**
        *@brief FeaturePipeEisNode's constructor
       */
    FeaturePipeEisNode( const char *name,
                              eThreadNodeType type,
                              int policy,
                              int priority,
                              MUINT32 featureOnMask,
                              MUINT32 mOpenedSensorIndex);

    DECLARE_ICAMTHREADNODE_INTERFACES()

private:

    MVOID RegisterEisP2User();

    //----------------------------------------------------------------------------

    EisHal *mpEisHal;

    //queue<FeaturePipeNodeJob_s> mqJobInput;

    EIS_PLUS_GPU_INFO mEisPlusGpuInfo;
    MINT32 *mpClearWarpMapX;
    MINT32 *mpClearWarpMapY;

    // EIS Pass2 Config Data
    queue<EIS_HAL_CONFIG_DATA> mEisHalCfgData;
    queue<MUINT32> mFrameNum;
    queue<MUINT32> mGpuTargetW;
    queue<MUINT32> mGpuTargetH;
};
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //_MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_EIS_H_

