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
#ifndef _STEREO_HAL_BASE_H_
#define _STEREO_HAL_BASE_H_

#include <mtkcam/common.h>
using namespace NSCam;

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum
{
    STEREO_SCENARIO_UNKNOWN     =   0x0000,
    STEREO_SCENARIO_PREVIEW     =   0x0001,     // DepthAF/Image Refocus preview
    STEREO_SCENARIO_RECORD      =   0x0002,     // DepthAF/Image Refocus record
    STEREO_SCENARIO_CAPTURE     =   0x0004,     // Image Refocus
    STEREO_SCENARIO_EIS         =   0x0008,     // EIS
    STEREO_SCENARIO_DISTANCE    =   0x0010,     // Distance measurement

}
STEREO_SCENARIO_ENUM;


struct INIT_DATA_STEREO_IN_T
{
    STEREO_SCENARIO_ENUM    eScenario;
    MSize                   main_image_size;    // JPEG size for image refocus
    MSize                   algo_image_size;    // preview size / JPS(SBS) size
    MINT32                  main1_sensor_index;
    MINT32                  main2_sensor_index;

public:     ////    Operations.
    INIT_DATA_STEREO_IN_T()
        : eScenario(STEREO_SCENARIO_UNKNOWN)
        , main_image_size()
        , algo_image_size()
        , main1_sensor_index(-1)
        , main2_sensor_index(-1) {}

};


struct INIT_DATA_STEREO_OUT_T
{
    MSize   algoin_size;

public:     ////    Operations.
    INIT_DATA_STEREO_OUT_T()
        : algoin_size() {}
};


struct RRZ_DATA_STEREO_T
{
    MRect   rrz_crop_main1;
    MSize   rrz_size_main1;
    MRect   rrz_crop_main2;
    MSize   rrz_size_main2;

public:     ////    Operations.
    RRZ_DATA_STEREO_T()
        : rrz_crop_main1()
        , rrz_size_main1()
        , rrz_crop_main2()
        , rrz_size_main2() {}
};


struct SET_DATA_STEREO_T
{
    void* mSrcGraphicBuffer;
    void* mSrcEGLImage;
    void* mDstGraphicBuffer;
    void* mDstEGLImage;
    void* u4RgbaAddr_main1;        // main1 rgba image address
    void* u4RgbaAddr_main2;        // main2 rgba image address
    void* u4FEBufAddr_main1;    // main1 image feature points data array
    void* u4FEBufAddr_main2;    // main2 image feature points data array

public:     ////    Operations.
    SET_DATA_STEREO_T()  { ::memset(this, 0, sizeof(SET_DATA_STEREO_T)); }
};


struct OUT_DATA_STEREO_T
{
    MRect   algo_main1;     // Image Capture
    MRect   algo_main2;     // Image Capture
    MRect   main_crop;      // Image Refocus
//    MUINT32 is_finish_learning;

public:     ////    Operations.
    OUT_DATA_STEREO_T()
        : algo_main1()
        , algo_main2()
        , main_crop() {}
};


struct HW_DATA_STEREO_T
{
    MUINT32 rgba_image_width; // Image Capture
    MUINT32 rgba_image_height;
    MUINT32 rgba_image_stride;

    MUINT32 hwfe_block_size; // HWFE & Algorithm Image
    MUINT32 fefm_image_width;
    MUINT32 fefm_imgae_height;

public:     ////    Operations.
    HW_DATA_STEREO_T()  { ::memset(this, 0, sizeof(HW_DATA_STEREO_T)); }
};


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class StereoHalBase
{
public:
    static StereoHalBase* createInstance();
    virtual void destroyInstance() = 0;
    virtual bool init() = 0;
    virtual bool uninit() = 0;
    virtual ~StereoHalBase() {};

    virtual MUINT32 getSensorPosition() const = 0;
    virtual MSize getMainSize(MSize const imgSize) const = 0;
    virtual MSize getRrzSize(MUINT32 const idx) const = 0;
    virtual MSize getFEImgSize() const = 0;
    virtual bool STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const = 0;
    virtual bool STEREOGetInfo(HW_DATA_STEREO_T &OutData) const = 0;
    virtual bool STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData) = 0;
    virtual bool STEREOSetParams(SET_DATA_STEREO_T RunData) = 0;
    virtual bool STEREORun(OUT_DATA_STEREO_T &OutData,MBOOL EnableAlgo = MTRUE) = 0;
    virtual bool STEREODestroy(void) = 0;

protected:

private:

};

#endif  // _STEREO_HAL_BASE_H_

