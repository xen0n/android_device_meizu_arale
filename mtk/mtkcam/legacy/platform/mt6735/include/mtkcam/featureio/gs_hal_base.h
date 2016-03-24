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

#ifndef _GS_HAL_BASE_H_
#define _GS_HAL_BASE_H_

//#include <system/camera.h>
//#include "MediaHal.h"
//#include <mhal/inc/camera/faces.h>
#include <mtkcam/common/faces.h>
/*******************************************************************************
*
********************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;
typedef unsigned char MUINT8;
typedef signed int    MBOOL;
#ifndef FALSE
#define FALSE (bool) 0
#endif
#ifndef TRUE
#define TRUE (bool) 1
#endif
#ifndef NULL
#define NULL 0
#endif


/*******************************************************************************
*
********************************************************************************/
enum HalGSObject_e {
    HAL_GS_OBJ_NONE = 0,
    HAL_GS_OBJ_SW,
    HAL_GS_OBJ_HW,
    HAL_GS_OBJ_UNKNOWN = 0xFF
} ;


struct GS_RESULT {

    MINT32 rect[4];
    MINT32 score;
    MINT32 rop_dir;
    MINT32 rip_dir;
};

/*******************************************************************************
*
********************************************************************************/
class halGSBase {
public:
    //
    static halGSBase* createInstance(HalGSObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halGSBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSInit () -
    //! \brief init gesture shot
    //
    /////////////////////////////////////////////////////////////////////////
    //virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH, MUINT32 WorkingBuffer, MUINT32 WorkingBufferSize, MBOOL   SWResizerEnable) {return 0;}
    virtual MINT32 halGSInit(MUINT32 fdW, MUINT32 fdH, MUINT8* WorkingBuffer, MUINT32 WorkingBufferSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSDo () -
    //! \brief process gesture shot
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halGSDo(MUINT8* ImageBuffer2, MINT32 rRotation_Info) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSUninit () -
    //! \brief gs uninit
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halGSUninit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalGSGetGestureResult () -
    //! \brief get Gesture Shot result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halGSGetGestureResult(MtkCameraFaceMetadata * gs_result) {return 0;}
};

class halGSTmp : public halGSBase {
public:
    //
    static halGSBase* getInstance();
    virtual void destroyInstance();
    //
    halGSTmp() {};
    virtual ~halGSTmp() {};
};

#endif

