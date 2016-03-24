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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IDUMMYIMAGEBUFFERHEAP_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IDUMMYIMAGEBUFFERHEAP_H_
//


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

struct PortBufInfo_dummy
{
    MINT32      memID;
    MUINTPTR    virtAddr[3];
    MUINTPTR    phyAddr[3];
    MINT32      nocache;
    MINT32      security;
    MINT32      coherence;

    PortBufInfo_dummy(
        MINT32 const    _memID,
        MUINTPTR const  _virtAddr[],
        MUINTPTR const  _phyAddr[],
        MUINT32 const   _planeCount,
        MINT32  _nocache = 0, 
        MINT32  _security = 0, 
        MINT32  _coherence = 0
    )
        : memID(_memID)
        , nocache(_nocache)
        , security(_security)
        , coherence(_coherence)
    {
        for (MUINT32 i = 0; i < _planeCount; ++i)
        {
            virtAddr[i] = _virtAddr[i];
            phyAddr[i]  = _phyAddr[i];
        }
    }
};

/******************************************************************************
 *  Image Buffer Heap (Dummy).
 ******************************************************************************/
class IDummyImageBufferHeap : public virtual IImageBufferHeap
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Params for Allocations.
    typedef IImageBufferAllocator::ImgParam ImgParam_t;


public:     ////                    Creation.
    static  IDummyImageBufferHeap*  create(
                                        char const* szCallerName,
                                        ImgParam_t const& rImgParam, 
                                        PortBufInfo_dummy const& rPortBufInfo,
                                        MBOOL const enableLog = MTRUE
                                    );

protected:  ////                    Destructor/Constructors.
    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IDummyImageBufferHeap() {}

public:     ////                    Attributes.
    static  char const*             magicName() { return "DummyHeap"; }

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IDUMMYIMAGEBUFFERHEAP_H_

