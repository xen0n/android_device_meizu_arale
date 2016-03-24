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
/*
** $Log: vfb_hal.h $
 *
*/

#ifndef _VFB_HAL_H_
#define _VFB_HAL_H_

#include "featureio/vfb_hal_base.h"
#include "MTKVideoFaceBeauty.h"

class MTKVfb;

/*******************************************************************************
*
********************************************************************************/
class halVFB: public halVFBBase
{
public:
    //
    static halVFBBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////
    halVFB();

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~halVFB();

    /**
     * @brief init the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   mHalVFBInit(MTKPipeVfbEnvInfo VfbEnvInfo);

    /**
     * @brief uninit the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   mHalVFBUninit();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalVFBProc () -
    //! \brief Vfb process
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MBOOL mHalVFBProc(MTKPipeVfbProcInfo VfbProcInfo, MTKPipeVfbResultInfo* VfbResultInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // Get Working buffer size () -
    //!
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalVFBGetWorkingBufSize();

    /////////////////////////////////////////////////////////////////////////
    //
    // Set Working buffer () -
    //!
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MBOOL mHalVFBSetWorkingBuf(void* WorkingBufAddr, MINT32 WorkingBufSize);

protected:


private:
    MTKVideoFaceBeauty*     mpMTKVideoFaceBeautyObj;
    MUINT32                 mWorkingBufferSize;
    MINT32                  mTemporalSmLevel;
    MINT32                  mTimeout;
    MINT32                  mSortFaceWei;
    MINT32                  mRuddyLevel;
};

#endif

