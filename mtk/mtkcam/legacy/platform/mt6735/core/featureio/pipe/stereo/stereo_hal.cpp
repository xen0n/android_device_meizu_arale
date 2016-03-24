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

#define LOG_TAG "stereo_hal"

#include <stdlib.h>     // for rand()

#include <mtkcam/Log.h>
#include "MTKStereoKernelScenario.h"    // For MTKStereoKernel class/INPUT_FORMAT_ENUM/STEREO_KERNEL_TUNING_PARA_STRUCT. Must be included before stereo_hal_base.h/stereo_hal.h.
#include "MTKStereoKernel.h"    // For MTKStereoKernel class/INPUT_FORMAT_ENUM/STEREO_KERNEL_TUNING_PARA_STRUCT. Must be included before stereo_hal_base.h/stereo_hal.h.
#include "stereo_hal.h"            // For StereoHal class.

#include "IHalSensor.h"
#include "camera_custom_nvram.h"
#include "nvbuf_util.h"
#include "camera_custom_stereo.h"  // For CUST_STEREO_* definitions.
using namespace NSCam;
using android::Mutex;           // For android::Mutex in stereo_hal.h.

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define ENABLE_GPU                  0

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static StereoHalBase *pStereoHal = NULL;

STEREO_KERNEL_TUNING_PARA_STRUCT        gStereoKernelTuningParaInfo;
STEREO_KERNEL_SET_ENV_INFO_STRUCT       gStereoKernelInitInfo;
STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT  gStereoKernelWorkBufInfo;   // Working Buffer information.
STEREO_KERNEL_SET_PROC_INFO_STRUCT      gStereoKernelProcInfo;      // Setprocess information. Mostly addresses.
STEREO_KERNEL_RESULT_STRUCT             gStereoKernelResultInfo;    // STEREO algorithm results.

MINT32 gDataArray[MTK_STEREO_KERNEL_NVRAM_LENGTH];    // STORE the Learning Information / One-Shot Calibration
MUINT32 gNVRAM_AF[1024] ;                             // STORE the AF Learning Information (DAC to Disparity)
MUINT main1_FOV_horizontal = 65;
MUINT main1_FOV_vertical   = 65;
MUINT main1_Capture_Width  = 4096;
MUINT main1_Capture_Height = 3072;
MUINT main1_Capture_Offsetx = 0;
MUINT main1_Capture_Offsety = 0;
MUINT main2_FOV_horizontal = 65;
MUINT main2_FOV_vertical   = 65;
MUINT main2_Capture_Width  = 4096;
MUINT main2_Capture_Height = 3072;
MUINT main2_Capture_Offsetx = 0;
MUINT main2_Capture_Offsety = 0;
MUINT PV_720P_W        = 1280;
MUINT PV_720P_H        = 720;
MUINT FE_BLOCK_NUM     = 1;
MUINT FE_IMAGE_WIDTH   = 1536;
MUINT FE_IMAGE_HEIGHT  = 864;
MUINT RGBA_IMAGE_WIDTH = 160;
MUINT RGBA_IMAGE_HEIGHT= 90;
MUINT main1_sensor_index = 0;
//
// Stereo ratio
#define DEFAULT_CROP_PRERCENT       20  // control the cropping region
#define STEREO_FACTOR 120
//
/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
#define ENABLE_ALGO 0

/*******************************************************************************
*
********************************************************************************/
StereoHal::
StereoHal()
    : mUsers(0)
    , m_pStereoDrv(NULL)
    , mScenario(STEREO_SCENARIO_UNKNOWN) // default value
    , mAlgoSize()
    , mMainSize()
    , mFEImgSize(MSize(PV_720P_W, PV_720P_H)*STEREO_FACTOR/100)
{
}


/*******************************************************************************
*
********************************************************************************/
StereoHal::~StereoHal()
{

}

StereoHal* StereoHal::
createInstance()
{
    StereoHal *pStereoHal = StereoHal::getInstance();
    pStereoHal->init();
    return pStereoHal;
}

/*******************************************************************************
*
********************************************************************************/
StereoHal* StereoHal::
getInstance()
{
    CAM_LOGD("StereoHal getInstance.");
    static StereoHal singleton;
    return &singleton;
}


/*******************************************************************************
*
********************************************************************************/
void
StereoHal::
destroyInstance()
{
    uninit();
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::init()
{
    CAM_LOGD("- E. mUsers: %d.", mUsers);
    MBOOL Result = MTRUE;   // TRUE: no error. FALSE: error.

    Mutex::Autolock lock(mLock);

    if (mUsers > 0)
    {
        CAM_LOGD("StereoHal has already inited.");
        goto lb_Normal_Exit;
    }

    // Create StereoDrv instance.
    m_pStereoDrv = MTKStereoKernel::createInstance();
    if (!m_pStereoDrv)
    {
        CAM_LOGE("MTKStereoKernel::createInstance() fail.");
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    // Load learning data from NVRAM.
    Result = LoadFromNvram((int*)gDataArray, MTK_STEREO_KERNEL_NVRAM_LENGTH);
    if (!Result)    // Maybe no data in NVRAM, so read from EEPROM.
    {
        CAM_LOGD("Load from NVRAM fail (Maybe 1st time so no data in NVRAM yet).");
    }

lb_Normal_Exit:
    android_atomic_inc(&mUsers);

    CAM_LOGD("- X. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

lb_Abnormal_Exit:
    // StereoDrv Init failed, destroy StereoDrv instance.
    if (m_pStereoDrv)
    {
        m_pStereoDrv->destroyInstance();
        m_pStereoDrv = NULL;
    }

    CAM_LOGD("- X. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::uninit()
{
    CAM_LOGD("- E. mUsers: %d.", mUsers);
    MBOOL Result = MTRUE;

    Mutex::Autolock lock(mLock);

    if (mUsers > 1)  // More than one user, so decrease one mUsers.
    {
        android_atomic_dec(&mUsers);
    }
    else if (mUsers == 1)   // Last user, must do some un-init procedure.
    {
        android_atomic_dec(&mUsers);

        // Save learning data to NVRAM.
        Result = SaveToNvram((int*)gStereoKernelInitInfo.learning_data, MTK_STEREO_KERNEL_NVRAM_LENGTH);
        if (!Result)
        {
            CAM_LOGD("Save to NVRAM fail.");
        }

        // Destroy StereoDrv instance.
        if (m_pStereoDrv)
        {
            m_pStereoDrv->destroyInstance();
            m_pStereoDrv = NULL;
        }
    }
    else // mUsers <= 0. No StereoHal user, do nothing.
    {
        // do nothing.
        CAM_LOGW("No StereoHal to un-init.");
    }


    CAM_LOGD("- X. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
MUINT32
StereoHal::getSensorPosition() const
{
    // TODO: fix me, query from custom file
    // define:
    //    0 is main-main2 (main in L)
    //    1 is main2-main (main in R)
    return 0;
}


/*******************************************************************************
*
********************************************************************************/
MSize
StereoHal::getMainSize(MSize const imgSize) const
{
    // TODO: fix me
    return MSize(imgSize.w*1.2, imgSize.h*1.2);
}


/*******************************************************************************
*
********************************************************************************/
MSize
StereoHal::getRrzSize(MUINT32 const idx) const
{
    // idx: 0 means main sensor, otherwise, main2 sensor.
    return (0 == idx) ? MSize(1920*1.2, 1080*1.2) : MSize(1280*1.2, 720*1.2);
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const
{
    // TODO: calculate crop info

    OutData.rrz_crop_main1  = MRect(MPoint(0,373), MSize(4192,2358));
    OutData.rrz_size_main1  = getRrzSize(0);
    OutData.rrz_crop_main2  = MRect(MPoint(0,240), MSize(2560,1440));
    OutData.rrz_size_main2  = getRrzSize(1);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOGetInfo(HW_DATA_STEREO_T &OutData) const
{
    MBOOL ret = MTRUE;

    // TODO: fix hardcode

    OutData.rgba_image_width    = 160;
    OutData.rgba_image_height   = 90;
    OutData.fefm_image_width    = getFEImgSize().w/16 * 56;
    OutData.fefm_imgae_height   = getFEImgSize().h/16;
    switch (FE_BLOCK_NUM)
    {
         case 0:
           OutData.hwfe_block_size     = 8;
       break;
       case 1:
           OutData.hwfe_block_size     = 16;
       break;
       case 2:
           OutData.hwfe_block_size     = 32;
       break;
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData)
{
    float ratio_x = 0, ratio_y = 0;
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    MUINT8 *gWorkBuf = NULL ; // Working Buffer
    //
    setAlgoImgSize( InData.algo_image_size );
    setMainImgSize( InData.main_image_size );
    //
#if ENABLE_ALGO
    int sensorDevIdx = 0;
    MUINT sensorIdx;
    main1_sensor_index = InData.main1_sensor_index;
    IHalSensorList* sensorlsit;
    SensorStaticInfo pSensorStaticInfo;
    sensorlsit = IHalSensorList::get();
    sensorIdx = InData.main1_sensor_index;
    sensorDevIdx=sensorlsit->querySensorDevIdx(sensorIdx);
    if(sensorlsit) {
          sensorlsit->querySensorStaticInfo(sensorDevIdx,&pSensorStaticInfo);
          main1_FOV_horizontal=pSensorStaticInfo.horizontalViewAngle;
        main1_FOV_vertical=pSensorStaticInfo.verticalViewAngle;
        main1_Capture_Width  = pSensorStaticInfo.captureWidth;
        main1_Capture_Height = pSensorStaticInfo.captureHeight;
        main1_Capture_Offsetx = pSensorStaticInfo.captureHoizontalOutputOffset;
        main1_Capture_Offsety = pSensorStaticInfo.captureVerticalOutputOffset;
    }

    sensorIdx = InData.main2_sensor_index;
    sensorDevIdx=sensorlsit->querySensorDevIdx(sensorIdx);
    if(sensorlsit) {
          sensorlsit->querySensorStaticInfo(sensorDevIdx,&pSensorStaticInfo);
          main2_FOV_horizontal=pSensorStaticInfo.horizontalViewAngle;
        main2_FOV_vertical=pSensorStaticInfo.verticalViewAngle;
        main2_Capture_Width  = pSensorStaticInfo.captureWidth;
        main2_Capture_Height = pSensorStaticInfo.captureHeight;
        main2_Capture_Offsetx = pSensorStaticInfo.captureHoizontalOutputOffset;
        main2_Capture_Offsety = pSensorStaticInfo.captureVerticalOutputOffset;
    }

    ratio_x = 1.0f + DEFAULT_CROP_PRERCENT/100.0f ;
    ratio_y = 1.0f + DEFAULT_CROP_PRERCENT/100.0f ;

    STEREO_KERNEL_SCENARIO_ENUM algoScenario = STEREO_KERNEL_SCENARIO_IMAGE_PREVIEW;    // default value
    mScenario = InData.eScenario;

    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING, NULL, &gStereoKernelTuningParaInfo);
    if (err)
    {
        CAM_LOGE("StereoKernelFeatureCtrl(GET_DEFAULT_TUNING) fail. error code: %d.", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    switch ( mScenario )
    {
        case STEREO_SCENARIO_PREVIEW:
            algoScenario = STEREO_KERNEL_SCENARIO_IMAGE_PREVIEW;
            break;
        case STEREO_SCENARIO_RECORD:
            algoScenario = STEREO_KERNEL_SCENARIO_VIDEO_RECORD;
            break;
        case STEREO_SCENARIO_CAPTURE:
            algoScenario = STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE_RF;
            break;
        default:
            CAM_LOGE("unsupport scenario(%d)", mScenario);
            break;
    }

    gStereoKernelInitInfo.ptuning_para = &gStereoKernelTuningParaInfo;
    gStereoKernelInitInfo.scenario = (STEREO_KERNEL_SCENARIO_ENUM)algoScenario;

    gStereoKernelInitInfo.main_source_image_width = InData.main_image_size.w * ratio_x; // for IMAGE REFOCUS
    gStereoKernelInitInfo.main_source_image_height = InData.main_image_size.h * ratio_y;
    gStereoKernelInitInfo.main_crop_image_width = InData.main_image_size.w;
    gStereoKernelInitInfo.main_crop_image_height = InData.main_image_size.h;

    gStereoKernelInitInfo.algo_source_image_width = gStereoKernelInitInfo.main_source_image_width;
    gStereoKernelInitInfo.algo_source_image_height = gStereoKernelInitInfo.main_source_image_height;
    gStereoKernelInitInfo.algo_crop_image_width = ((int)(gStereoKernelInitInfo.main_source_image_width  / ratio_x + 0.5f )/2)*2; // for IMAGE REFOCUS/N3D
    gStereoKernelInitInfo.algo_crop_image_height = ((int)(gStereoKernelInitInfo.main_source_image_height  / ratio_y + 0.5f )/2)*2;

    gStereoKernelInitInfo.hwfe_block_size   = FE_BLOCK_NUM;
    gStereoKernelInitInfo.fefm_image_width  = FE_IMAGE_WIDTH; // HWFE size  // main2 x need Modify
    gStereoKernelInitInfo.fefm_image_height = FE_IMAGE_HEIGHT;                // main2 y need Modify

    gStereoKernelInitInfo.rgba_image_width  = RGBA_IMAGE_WIDTH;  //FP need proivde need Modify
    gStereoKernelInitInfo.rgba_image_height = RGBA_IMAGE_HEIGHT;   //FP need proivde need Modify
    gStereoKernelInitInfo.rgba_image_stride = RGBA_IMAGE_WIDTH;  //FP need proivde need Modify

    gStereoKernelInitInfo.remap_main.pixel_array_width  = main1_Capture_Width ;
    gStereoKernelInitInfo.remap_main.pixel_array_height = main1_Capture_Height ; // 4:3=>3072, 16:9=>2304
    gStereoKernelInitInfo.remap_main.sensor_offset_x = 0 ; //Sava should check
    gStereoKernelInitInfo.remap_main.sensor_offset_y = 0 ; //Sava should check
    gStereoKernelInitInfo.remap_main.sensor_binning = 0 ;
    gStereoKernelInitInfo.remap_main.rrz_offset_x = 0 ;
    gStereoKernelInitInfo.remap_main.rrz_offset_y = 192 ; // (3072 - 4096/2bin/16*9)/2
    gStereoKernelInitInfo.remap_main.rrz_out_width = gStereoKernelInitInfo.algo_source_image_width;
    gStereoKernelInitInfo.remap_main.rrz_out_height = gStereoKernelInitInfo.algo_source_image_height;
    gStereoKernelInitInfo.remap_main.rrz_step_width  = (MUINT16)( 32768.0f*2047/(gStereoKernelInitInfo.algo_source_image_width -1) + 0.5f ) ; // (4096/2-1)/(out-1) * 32768 //Benson ?? /2 ?? drop pixel step ?
    gStereoKernelInitInfo.remap_main.rrz_step_height = (MUINT16)( 32768.0f*1151/(gStereoKernelInitInfo.algo_source_image_height -1) + 0.5f ) ; // (2304/2-1)/(out-1) * 32768

    gStereoKernelInitInfo.remap_minor.pixel_array_width  = main2_Capture_Width ;
    gStereoKernelInitInfo.remap_minor.pixel_array_height = main2_Capture_Height ; // 4:3=>1200, 16:9=>900
    gStereoKernelInitInfo.remap_minor.sensor_offset_x = main2_Capture_Offsetx ;
    gStereoKernelInitInfo.remap_minor.sensor_offset_y = main2_Capture_Offsety ;
    gStereoKernelInitInfo.remap_minor.sensor_binning = 0 ;
    gStereoKernelInitInfo.remap_minor.rrz_offset_x = 0 ;
    gStereoKernelInitInfo.remap_minor.rrz_offset_y = 150 ; // (1200 - 1600/1bin/16*9)/2
    gStereoKernelInitInfo.remap_minor.rrz_out_width = FE_IMAGE_WIDTH ;
    gStereoKernelInitInfo.remap_minor.rrz_out_height = FE_IMAGE_HEIGHT ;
    gStereoKernelInitInfo.remap_minor.rrz_step_width  = (MUINT16)( 32768.0f*1599/(FE_IMAGE_WIDTH -1) + 0.5f ) ;
    gStereoKernelInitInfo.remap_minor.rrz_step_height = (MUINT16)( 32768.0f* 899/(FE_IMAGE_HEIGHT-1) + 0.5f ) ;

    gStereoKernelInitInfo.learning_data     = (MUINT32*)gDataArray;
    gStereoKernelInitInfo.learning_data_af  = gNVRAM_AF;

    gStereoKernelInitInfo.warp_index        = 1 ; // 0: warp left, 1: warp right
    gStereoKernelInitInfo.enable_cc         = 1 ; //enable color correction: 1
    gStereoKernelInitInfo.enable_gpu        = ENABLE_GPU;

    gStereoKernelInitInfo.main_cam_fov      = main1_FOV_horizontal;   // default 65
    gStereoKernelInitInfo.stereo_baseline   = STEREO_BASELINE;   // default 2.0 cm Benson ??


//debug ----------------------------------------------------------------------------------------------------------
    CAM_LOGD("ptuning_para %d.",gStereoKernelInitInfo.ptuning_para->learn_tolerance   );
    CAM_LOGD("scenario %d.",gStereoKernelInitInfo.scenario                       );
    CAM_LOGD("main_source_image_width %d.",gStereoKernelInitInfo.main_source_image_width        );
    CAM_LOGD("main_source_image_height %d.",gStereoKernelInitInfo.main_source_image_height       );
    CAM_LOGD("main_crop_image_width %d.",gStereoKernelInitInfo.main_crop_image_width          );
    CAM_LOGD("main_crop_image_height %d.",gStereoKernelInitInfo.main_crop_image_height         );
    CAM_LOGD("algo_source_image_width %d.",gStereoKernelInitInfo.algo_source_image_width        );
    CAM_LOGD("algo_source_image_height %d.",gStereoKernelInitInfo.algo_source_image_height       );
    CAM_LOGD("algo_crop_image_width %d.",gStereoKernelInitInfo.algo_crop_image_width          );
    CAM_LOGD("algo_crop_image_height %d.",gStereoKernelInitInfo.algo_crop_image_height         );
    CAM_LOGD("hwfe_block_size %d.",gStereoKernelInitInfo.hwfe_block_size                );
    CAM_LOGD("fefm_image_width %d.",gStereoKernelInitInfo.fefm_image_width               );
    CAM_LOGD("fefm_image_height %d.",gStereoKernelInitInfo.fefm_image_height              );
    CAM_LOGD("rgba_image_width %d.",gStereoKernelInitInfo.rgba_image_width               );
    CAM_LOGD("rgba_image_height %d.",gStereoKernelInitInfo.rgba_image_height              );
    CAM_LOGD("rgba_image_stride %d.",gStereoKernelInitInfo.rgba_image_stride              );
    CAM_LOGD("pixel_array_width %d.",gStereoKernelInitInfo.remap_main.pixel_array_width   );
    CAM_LOGD("pixel_array_height %d.",gStereoKernelInitInfo.remap_main.pixel_array_height  );
    CAM_LOGD("sensor_offset_x %d.",gStereoKernelInitInfo.remap_main.sensor_offset_x     );
    CAM_LOGD("sensor_offset_y %d.",gStereoKernelInitInfo.remap_main.sensor_offset_y     );
    CAM_LOGD("sensor_binning %d.",gStereoKernelInitInfo.remap_main.sensor_binning      );
    CAM_LOGD("rrz_offset_x %d.",gStereoKernelInitInfo.remap_main.rrz_offset_x        );
    CAM_LOGD("rrz_offset_y %d.",gStereoKernelInitInfo.remap_main.rrz_offset_y        );
    CAM_LOGD("rrz_out_width %d.",gStereoKernelInitInfo.remap_main.rrz_out_width       );
    CAM_LOGD("rrz_out_height %d.",gStereoKernelInitInfo.remap_main.rrz_out_height      );
    CAM_LOGD("rrz_step_width %d.",gStereoKernelInitInfo.remap_main.rrz_step_width      );
    CAM_LOGD("rrz_step_height %d.",gStereoKernelInitInfo.remap_main.rrz_step_height     );
    CAM_LOGD("pixel_array_width %d.",gStereoKernelInitInfo.remap_minor.pixel_array_width  );
    CAM_LOGD("pixel_array_height %d.",gStereoKernelInitInfo.remap_minor.pixel_array_height );
    CAM_LOGD("sensor_offset_x %d.",gStereoKernelInitInfo.remap_minor.sensor_offset_x    );
    CAM_LOGD("sensor_offset_y %d.",gStereoKernelInitInfo.remap_minor.sensor_offset_y    );
    CAM_LOGD("sensor_binning %d.",gStereoKernelInitInfo.remap_minor.sensor_binning     );
    CAM_LOGD("rrz_offset_x %d.",gStereoKernelInitInfo.remap_minor.rrz_offset_x       );
    CAM_LOGD("rrz_offset_y %d.",gStereoKernelInitInfo.remap_minor.rrz_offset_y       );
    CAM_LOGD("rrz_out_width %d.",gStereoKernelInitInfo.remap_minor.rrz_out_width      );
    CAM_LOGD("rrz_out_height %d.",gStereoKernelInitInfo.remap_minor.rrz_out_height     );
    CAM_LOGD("rrz_step_width %d.",gStereoKernelInitInfo.remap_minor.rrz_step_width     );
    CAM_LOGD("rrz_step_height %d.",gStereoKernelInitInfo.remap_minor.rrz_step_height    );
    CAM_LOGD("learning_data %d.",gStereoKernelInitInfo.learning_data                  );
    CAM_LOGD("learning_data_af %d.",gStereoKernelInitInfo.learning_data_af               );
    CAM_LOGD("warp_index %d.",gStereoKernelInitInfo.warp_index                     );
    CAM_LOGD("enable_cc %d.",gStereoKernelInitInfo.enable_cc                      );
    CAM_LOGD("enable_gpu %d.",gStereoKernelInitInfo.enable_gpu                     );
    CAM_LOGD("main_cam_fov %d.",gStereoKernelInitInfo.main_cam_fov                   );
    CAM_LOGD("stereo_baseline %f.",gStereoKernelInitInfo.stereo_baseline                );
//debug ----------------------------------------------------------------------------------------------------------



    err = m_pStereoDrv->StereoKernelInit(&gStereoKernelInitInfo);
    if (err)
    {
        CAM_LOGE("StereoKernelInit() fail. error code: %d.", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }
 #endif

    if ( mScenario == STEREO_SCENARIO_PREVIEW
        || mScenario == STEREO_SCENARIO_RECORD )
    {
        OutData.algoin_size   = InData.main_image_size;
    }
    else
    {
        OutData.algoin_size   = InData.main_image_size*STEREO_FACTOR/100;
    }

#if ENABLE_ALGO
    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_WORK_BUF_INFO, NULL, &gStereoKernelInitInfo.working_buffer_size);
    if (err)
    {
        CAM_LOGE("StereoKernelFeatureCtrl(Get_WORK_BUF_INFO) fail. error code: %d.", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }
    gWorkBuf = (MUINT8*)malloc( sizeof(MUINT8) * gStereoKernelInitInfo.working_buffer_size ) ;
    CAM_LOGD("Working buffer size: %d addr 0x%x. ", gStereoKernelInitInfo.working_buffer_size,gWorkBuf);
    STEREOSetWorkBuf((unsigned int )gWorkBuf);
#endif

lb_Abnormal_Exit:

    //CAM_LOGD("- X. Result: %d. work_buf_size: %d. l_crop_offset W/H: (%d, %d). r_crop_offset W/H: (%d, %d).", Result, gStereoKernelInitInfo.working_buffer_size, OutData.left_crop_offset.width, OutData.left_crop_offset.height, OutData.right_crop_offset.width, OutData.right_crop_offset.height);

    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOSetWorkBuf(unsigned int WorkBufAddr)
{
    CAM_LOGD("- E. WorkBufAddr: 0x%08X.", WorkBufAddr);
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    // Allocate working buffer.
    //     Allocate memory
    //     Set WorkBufInfo
    gStereoKernelWorkBufInfo.ext_mem_size = gStereoKernelInitInfo.working_buffer_size;
    //gStereoKernelWorkBufInfo.ext_mem_start_addr = WorkBufAddr;

    #if 0   // For debug.
    CAM_LOGD("gStereoKernelWorkBufInfo.ext_mem_size      : %d.", gStereoKernelWorkBufInfo.ext_mem_size);
    CAM_LOGD("gStereoKernelWorkBufInfo.ext_mem_start_addr: 0x%08X.", gStereoKernelWorkBufInfo.ext_mem_start_addr);
    #endif  // For debug.

    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO, &gStereoKernelWorkBufInfo, NULL);
    if (err)
    {
        CAM_LOGE("StereoKernelFeatureCtrl(SET_WORK_BUF_INFO) fail. error code: %d.", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

lb_Abnormal_Exit:

    CAM_LOGD("- X. Result: %d.", Result);
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOSetParams(SET_DATA_STEREO_T RunData)
{
    MBOOL Result = MTRUE;
    CAM_LOGD("SetParams in");
    int gDACIndex = 0;
    MINT32 err = 0; // 0: no error. other value: error.
    //gStereoKernelProcInfo.rgba_image_left_addr  = (MUINT32)RunData.mSrcGraphicBuffer;
    //gStereoKernelProcInfo.rgba_image_right_addr = (MUINT32)RunData.mDstGraphicBuffer;
    gStereoKernelProcInfo.src_gb.mEGLImage = RunData.mSrcEGLImage;
    gStereoKernelProcInfo.dst_gb.mEGLImage = RunData.mDstEGLImage;
    // hwfe
    //gStereoKernelProcInfo.hwfe_data_left  = (MUINT16*)RunData.u4FEBufAddr_main1 ; // for store results of HWFE, Left  Image
    //gStereoKernelProcInfo.hwfe_data_right = (MUINT16*)RunData.u4FEBufAddr_main2 ; // for store results of HWFE, Right Image
    // af info.
    gStereoKernelProcInfo.af_dac_index = gDACIndex ;
    gStereoKernelProcInfo.af_confidence = 1 ;
    gStereoKernelProcInfo.af_valid = 1 ;
    gStereoKernelProcInfo.af_win_start_x_remap = 1793 ; // 1/8 windows of 4096 = 512
    gStereoKernelProcInfo.af_win_end_x_remap   = 2304 ; // 2304-1793+1 = 512
    gStereoKernelProcInfo.af_win_start_y_remap = 1345 ; // 1/8 windows of 3072 = 384
    gStereoKernelProcInfo.af_win_end_y_remap   = 1728 ; // 1728-1345+1 = 384
    m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_PROC_INFO, &gStereoKernelProcInfo, NULL);
    CAM_LOGD("SetParams out");
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREORun(OUT_DATA_STEREO_T &OutData,MBOOL EnableAlgo)
{
    CAM_LOGD("Run in (%d)", EnableAlgo);
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
#if ENABLE_ALGO
    // Algorithm main.
    if(EnableAlgo)
    {
        err = m_pStereoDrv->StereoKernelMain();
        CAM_LOGD("Run main out");
        if (err)
        {
            CAM_LOGE("StereoKernelMain() fail. error code: %d.", err);
            Result = MFALSE;
            goto lb_Abnormal_Exit;
        }

        // Get result.
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_RESULT, NULL, &gStereoKernelResultInfo);
        if (err)
        {
            CAM_LOGE("StereoKernelFeatureCtrl(GET_RESULT) fail. error code: %d.", err);
            Result = MFALSE;
            goto lb_Abnormal_Exit;
        }
    }
    CAM_LOGD("Run get result out");
#endif
    // Prepare output data.

lb_Abnormal_Exit:

#if ENABLE_ALGO
    OutData.algo_main1.p.x  = gStereoKernelResultInfo.algo_left_offset_x;  // Image Capture
    OutData.algo_main1.p.y  = gStereoKernelResultInfo.algo_left_offset_y;  // Image Capture
    OutData.algo_main1.s    = getAlgoImgSize();

    OutData.algo_main2.p.x  = gStereoKernelResultInfo.algo_right_offset_x;  // Image Capture
    OutData.algo_main2.p.y  = gStereoKernelResultInfo.algo_right_offset_x;  // Image Capture
    OutData.algo_main2.s    = getAlgoImgSize();

    OutData.main_crop.p.x   = gStereoKernelResultInfo.main_offset_x;  // Image Refocus
    OutData.main_crop.p.y   = gStereoKernelResultInfo.main_offset_y;  // Image Refocus
    OutData.main_crop.s     = getMainImgSize();
#else
    OutData.main_crop.p.x   = 10;  // Image Refocus
    OutData.main_crop.p.y   = 10;  // Image Refocus
    OutData.main_crop.s     = getMainImgSize();

    OutData.algo_main1.p.x   = 5;
    OutData.algo_main1.p.y   = 5;
    OutData.algo_main1.s     = getAlgoImgSize();

    OutData.algo_main2.p.x   = 6;
    OutData.algo_main2.p.y   = 6;
    OutData.algo_main2.s     = getAlgoImgSize();
#endif

    CAM_LOGD("- X. algo_main1(%d,%d,%d,%d), algo_main2(%d,%d,%d,%d), main_crop(%d,%d,%d,%d)",
            OutData.algo_main1.p.x, OutData.algo_main1.p.y,
            OutData.algo_main1.s.w, OutData.algo_main1.s.h,
            OutData.algo_main2.p.x, OutData.algo_main2.p.y,
            OutData.algo_main2.s.w, OutData.algo_main2.s.h,
            OutData.main_crop.p.x, OutData.main_crop.p.y,
            OutData.main_crop.s.w, OutData.main_crop.s.h);

    //CAM_LOGD("- X. Result: %d. OutData.offset W/H: (%d, %d). isFinishLearning: %d.", Result, OutData.offset.width, OutData.offset.height, OutData.isFinishLearning);
//    CAM_LOGD("Run out");
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREODestroy(void)
{
    CAM_LOGD("- E.");
    MBOOL Result = MTRUE;

    // Reset algorithm.
    m_pStereoDrv->StereoKernelReset();

    CAM_LOGD("- X. Result: %d.", Result);
    return Result;

}

/*******************************************************************************
* lenNVRAM: must use "byte" as unit.
* adb shell setprop debuglog.stereo.printloadednvram 1: print arrNVRAM that loaded from NVRAM.
********************************************************************************/
bool
StereoHal::LoadFromNvram(signed int  *arrNVRAM, unsigned int lenNVRAM)
{
    CAM_LOGD("- E. arrNVRAM: 0x%08x. lenNVRAM: %d.", arrNVRAM, lenNVRAM);

    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    char acDbgLogLevel[32] = {'\0'};
    int sensorDevIdx = 0;
    // Check if arrNVRAM is valid.
    if (arrNVRAM == NULL)
    {
        CAM_LOGE("NVRAM array is NULL.");
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }
    IHalSensorList* sensorlsit;
    sensorlsit=IHalSensorList::get();
    sensorDevIdx=sensorlsit->querySensorDevIdx(main1_sensor_index);
    NVRAM_CAMERA_GEOMETRY_STRUCT* pVoidGeoData;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx, (void*&)pVoidGeoData);

lb_Abnormal_Exit:

    CAM_LOGD("- X. Result: %d.", Result);

    return Result;

}


/*******************************************************************************
* lenNVRAM: must use "byte" as unit.
* adb shell setprop debuglog.stereo.erasenvram 1: set to 1 to write all 0's into NVRAM. (Remember to set to 0 after erased.)
* adb shell setprop debuglog.stereo.printsavednvram 1: print arrNVRAM that saved to NVRAM.
********************************************************************************/
bool
StereoHal::SaveToNvram(signed int  *arrNVRAM, unsigned int lenNVRAM)
{
    CAM_LOGD("- E. arrNVRAM: 0x%08x. lenNVRAM: %d.", arrNVRAM, lenNVRAM);

    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    char acDbgLogLevel[32] = {'\0'};

    int sensorDevIdx = 0;
    // Check if arrNVRAM is valid.
    if (arrNVRAM == NULL)
    {
        CAM_LOGE("NVRAM array is NULL.");
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }
    IHalSensorList* sensorlsit;
    sensorlsit=IHalSensorList::get();
    sensorDevIdx=sensorlsit->querySensorDevIdx(main1_sensor_index);

    NVRAM_CAMERA_GEOMETRY_STRUCT* pVoidGeoData;
    //err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx, (void*&)pVoidGeoData);
    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx);

    if (err!=0)
    {
        CAM_LOGE("Write to NVRAM fail.");
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

lb_Abnormal_Exit:

    CAM_LOGD("- X. Result: %d.", Result);

    return Result;

}

