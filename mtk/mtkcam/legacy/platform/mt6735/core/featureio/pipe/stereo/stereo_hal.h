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
#ifndef _STEREO_HAL_H_
#define _STEREO_HAL_H_

#include "common.h"
#include "stereo_hal_base.h"
#include "MTKStereoKernel.h"
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <stdlib.h>

using android::Mutex;

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define NVRAM_LEN       1199        // this value should be the same as defined in StereoKernelCore.h
#define OS_START        632            // the start position of one-shot info that stored in NVRAM
#define OS_DATA_START        (OS_START+17)    // the start position of one-shot data (from module hosue) that stored in NVRAM

// The crop size of image under IMAGE CAPTURE mode  (not horizontal binned)
#define WIDTH_BEFORE_CROP      1600        // the image width  before applying crop
#define WIDTH_AFTER_CROP      1600        // the image width  after  applying crop

#define HEIGHT_BEFORE_CROP    1200        // the image height before applying crop
#define HEIGHT_AFTER_CROP      900            // the image height after  applying crop

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

typedef struct
{
    signed int      x_addr_start        ;
    signed int      y_addr_start        ;
    signed int      x_addr_end            ;
    signed int      y_addr_end            ;
    unsigned char   isPhysicallyRotate    ;
    unsigned char   isSensorBinning_IC    ;        // This is set according to the scenario of IMAGE CAPTURE mode // SETTING of isSensorBining: for Moudle House, 8M: 0, 2M: 0; for Phone Driver, 8M: 1, 2M: 0 (IMAGE CAPTURE MODE).
}SENSOR_SETTING_STRUCT ;


typedef struct
{
    unsigned char             uSensorPos        ; // 0: L:8M R:2M, 1: L:2M, R:8M
    unsigned char             bIsUpscaling_PV    ; // is 2M sensor has been scaled up in Preview Mode
    SENSOR_SETTING_STRUCT    sensorL            ;
    SENSOR_SETTING_STRUCT    sensorR            ;
} STEREO_SENSOR_SETTING_STRUCT ;

typedef struct
{
    signed int  setting[9] ;

    // Left  Part of Pattern
    signed int  pointsL[130] ;
    signed int  statusL[3] ;

    // Right Part of Pattern
    signed int  pointsR[130] ;
    signed int  statusR[3] ;
} ONE_SHOT_DATA_STRUCT ;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class StereoHal: public StereoHalBase
{
protected:
    StereoHal();
    virtual ~StereoHal();

public:
    static StereoHal* createInstance();
    static StereoHal* getInstance();
    virtual void destroyInstance();
    virtual bool init();
    virtual bool uninit();

    virtual void setMainImgSize(MSize const& imgSize)     {   mMainSize = imgSize;   };
    virtual MSize getMainImgSize() const            {   return mMainSize;   };

    virtual void setAlgoImgSize(MSize const& imgSize)     {   mAlgoSize = imgSize;   };
    virtual MSize getAlgoImgSize() const            {   return mAlgoSize;   };

    virtual MUINT32 getSensorPosition() const;
    virtual MSize getMainSize(MSize const imgSize) const;
    virtual MSize getRrzSize(MUINT32 const idx) const;
    virtual MSize getFEImgSize() const              {   return mFEImgSize; };
    virtual bool STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const;
    virtual bool STEREOGetInfo(HW_DATA_STEREO_T &OutData) const;
    virtual bool STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData);
    virtual bool STEREOSetParams(SET_DATA_STEREO_T RunData); //For GPU buffer should in one thread, if not can meger with STEREORun
    virtual bool STEREORun(OUT_DATA_STEREO_T &OutData,MBOOL EnableAlgo = MTRUE);
    virtual bool STEREODestroy(void);

    virtual bool STEREOSetWorkBuf(unsigned int WorkBufAddr);
    virtual bool LoadFromNvram(signed int *arrNVRAM, unsigned int lenNVRAM);
    virtual bool SaveToNvram(  signed int *arrNVRAM, unsigned int lenNVRAM);

private:
    volatile MINT32         mUsers;
    mutable Mutex           mLock;
    //
    MTKStereoKernel*        m_pStereoDrv;
    STEREO_SCENARIO_ENUM    mScenario;
    //
    MSize                   mAlgoSize;
    MSize                   mMainSize;
    MSize                   mFEImgSize;
};

#endif  // _STEREO_HAL_H_

