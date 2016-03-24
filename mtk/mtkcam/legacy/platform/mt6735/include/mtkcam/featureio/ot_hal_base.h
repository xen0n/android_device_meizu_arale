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
** $Log: ot_hal_base.h $
 *
*/

#ifndef _OT_HAL_BASE_H_
#define _OT_HAL_BASE_H_

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
enum HalOTObject_e {
    HAL_OT_OBJ_NONE = 0,
    HAL_OT_OBJ_SW,
    HAL_OT_OBJ_HW,
    HAL_OT_OBJ_UNKNOWN = 0xFF
} ;

struct OT_RESULT {

    MINT32 rect[4];
    MINT32 pass;
};

/*******************************************************************************
*
********************************************************************************/
class halOTBase {
public:
    //
    static halOTBase* createInstance(HalOTObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halOTBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalOTInit () -
    //! \brief init object tracking
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halOTInit(MUINT32 OTw, MUINT32 OTh, MUINT32 WorkingBuffer, MUINT32 WorkingBufferSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalOTDo () -
    //! \brief process object tracking
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halOTDo(MUINT8 *ImageBuffer1, MINT32 a_InitargetX, MINT32 a_InitargetY) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalOTUninit () -
    //! \brief uninit object tracking
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halOTUninit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalOTGetResult () -
    //! \brief get object tracking result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halOTGetResult(MtkCameraFaceMetadata * ot_result) {return 0;}

   /////////////////////////////////////////////////////////////////////////
    //
    // halOTBufferCreate () -
    //! \brief create object tracking buffer
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halOTBufferCreate(MUINT8 *dstAddr, MUINT8 *srcAddr, MUINT8  ucBufferGroup) {return 0;}

};

class halOTTmp : public halOTBase {
public:
    //
    static halOTBase* getInstance();
    virtual void destroyInstance();
    //
    halOTTmp() {};
    virtual ~halOTTmp() {};
};

#endif

