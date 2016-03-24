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
** $Log: gs_hal_base.h $
 *
*/

#ifndef _GS_HAL_H_
#define _GS_HAL_H_

#include "gs_hal_base.h"

class MTKGestureDetector;
/*******************************************************************************
*
********************************************************************************/
class halGS: public halGSBase
{
public:
    //
    static halGSBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halGSBase () -
    //! \brief GS Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////
    halGS();

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~halGS();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSInit () -
    //! \brief init gesture shot
    //
    /////////////////////////////////////////////////////////////////////////
    //virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH, MUINT32 WorkingBuffer, MUINT32 WorkingBufferSize, MBOOL   SWResizerEnable);
    virtual MINT32 halGSInit(MUINT32 fdW, MUINT32 fdH, MUINT8* WorkingBuffer, MUINT32 WorkingBufferSize);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSDo () -
    //! \brief process gesture shot
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halGSDo(MUINT8* ImageBuffer2, MINT32 rRotation_Info);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSUninit () -
    //! \brief GS uninit
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halGSUninit();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSGetGestureResult () -
    //! \brief get gesture shot result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halGSGetGestureResult(MtkCameraFaceMetadata * gs_result);

protected:

    MTKGestureDetector* m_pMTKGSObj;

    MUINT32 m_GSW;
    MUINT32 m_GSH;
    MUINT32 m_DispW;
    MUINT32 m_DispH;
    MUINT32 m_DispX;
    MUINT32 m_DispY;
    MUINT32 m_DispRoate;
    MUINT32 m_RegisterBuff;
    MUINT32 m_BuffCount;
    MUINT8 m_DetectPara;
    MBOOL   m_Inited;
};

#endif

