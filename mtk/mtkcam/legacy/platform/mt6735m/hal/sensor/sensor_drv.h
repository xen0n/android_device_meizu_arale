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
#ifndef _SENSOR_DRV_H
#define _SENSOR_DRV_H

#include <mtkcam/common.h>
#include "kd_imgsensor_define.h"
#include <mtkcam/hal/IHalSensor.h>

/*******************************************************************************
*
********************************************************************************/

//////////////////////////////////////////////////////////////////////////
//
//  Sensor Feature
//
//////////////////////////////////////////////////////////////////////////
using namespace NSCam;


typedef enum {
    CMD_SENSOR_SET_SENSOR_EXP_TIME            = 0x1000,
    CMD_SENSOR_SET_SENSOR_EXP_LINE,
    CMD_SENSOR_SET_SENSOR_GAIN,
    CMD_SENSOR_SET_FLICKER_FRAME_RATE,
    CMD_SENSOR_SET_VIDEO_FRAME_RATE,
    CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC,
    CMD_SENSOR_SET_CCT_FEATURE_CONTROL,
    CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA,
    CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO,
    CMD_SENSOR_SET_TEST_PATTERN_OUTPUT,
    CMD_SENSOR_SET_N3D_I2C_STREAM_REGDATA,
    CMD_SENSOR_SET_N3D_START_STREAMING,
    CMD_SENSOR_SET_N3D_STOP_STREAMING,
    CMD_SENSOR_SET_ESHUTTER_GAIN,
    CMD_SENSOR_SET_OB_LOCK,
    CMD_SNESOR_SET_SENSOR_OTP_AWB_CMD,
    CMD_SNESOR_SET_SENSOR_OTP_LSC_CMD,
    CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT    = 0x2000,
    CMD_SENSOR_GET_INPUT_BIT_ORDER,
    CMD_SENSOR_GET_PAD_PCLK_INV,
    CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE,
    CMD_SENSOR_GET_SENSOR_FACING_DIRECTION,
    CMD_SENSOR_GET_PIXEL_CLOCK_FREQ,
    CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM,
    CMD_SENSOR_GET_SENSOR_FEATURE_INFO,
    CMD_SENSOR_GET_ATV_DISP_DELAY_FRAME,
    CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
    CMD_SENSOR_GET_FAKE_ORIENTATION,
    CMD_SENSOR_GET_SENSOR_VIEWANGLE,
    CMD_SENSOR_GET_MCLK_CONNECTION,
    CMD_SENSOR_GET_MIPI_SENSOR_PORT,
    CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE,
    CMD_SENSOR_GET_TEMPERATURE_VALUE,
    CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO,
    CMD_SENSOR_GET_SENSOR_VC_INFO,
    CMD_SENSOR_GET_SENSOR_PDAF_INFO,
    CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME,
    CMD_SENSOR_SET_YUV_FEATURE_CMD          = 0x3000,
    CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE,
    CMD_SENSOR_SET_YUV_CANCEL_AF,
    CMD_SENSOR_SET_YUV_CONSTANT_AF,
    CMD_SENSOR_SET_YUV_INFINITY_AF,
    CMD_SENSOR_SET_YUV_AF_WINDOW,
    CMD_SENSOR_SET_YUV_AE_WINDOW,
    CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE,
    CMD_SENSOR_SET_YUV_3A_CMD,
    CMD_SENSOR_GET_YUV_AF_STATUS            = 0x4000,
    CMD_SENSOR_GET_YUV_AE_STATUS,
    CMD_SENSOR_GET_YUV_AWB_STATUS,
    CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN,
    CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN,
    CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,
    CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS,
    CMD_SENSOR_GET_YUV_EXIF_INFO,
    CMD_SENSOR_GET_YUV_DELAY_INFO,
    CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO,
    CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO,
    CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO,
    CMD_SENSOR_GET_YUV_SENSOR_CAPTURE_OUTPUT_JPEG,
    CMD_SENSOR_SET_YUV_AUTOTEST,
    CMD_SENSOR_SET_FRAMERATE,
    CMD_SENSOR_SET_IHDR_SHUTTER_GAIN,
    CMD_SENSOR_SET_HDR_SHUTTER,
    CMD_SENSOR_SET_SENSOR_AWB_GAIN,
    CMD_SENSOR_SET_MIN_MAX_FPS,
    CMD_SENSOR_GET_PDAF_DATA,
    CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY,
    CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER,
    CMD_SENSOR_MAX                 = 0xFFFF
} CMD_SENSOR_ENUM;

//HDR
typedef enum {
    WAIT_SENSOR_SET_SHUTTER_GAIN_DONE = 0x0,
    WAIT_SENSOR_EVENT_MAX = 0xFFFF
}WAIT_SENSOR_EVENT_ENUM;

/*******************************************************************************
*
********************************************************************************/
typedef enum {
    IMAGE_SENSOR_TYPE_RAW,
    IMAGE_SENSOR_TYPE_YUV,
    IMAGE_SENSOR_TYPE_YCBCR,
    IMAGE_SENSOR_TYPE_RGB565,
    IMAGE_SENSOR_TYPE_RGB888,
    IMAGE_SENSOR_TYPE_JPEG,
    IMAGE_SENSOR_TYPE_RAW8,
    IMAGE_SENSOR_TYPE_RAW12,
    IMAGE_SENSOR_TYPE_RAW14,
    IMAGE_SENSOR_TYPE_UNKNOWN = 0xFFFF,
} IMAGE_SENSOR_TYPE;

typedef enum {
    SENSOR_NONE = 0x00,
    SENSOR_MAIN = 0x01,
    SENSOR_SUB  = 0x02,
    SENSOR_PIP  = 0x03,
    SENSOR_MAIN_2 = 0x04,
    SENSOR_MAIN_3D = 0x05,
    SENSOR_ATV  = 0x08,
} SENSOR_DEV_ENUM;

typedef enum {
    SENSOR_NO_ERROR         = 0,            ///< The function work successfully
    SENSOR_UNKNOWN_ERROR    = 0x80000000,   ///< Unknown error
    SENSOR_INVALID_DRIVER   = 0x80000001,
    SENSOR_NO_SENSOR        = 0x80000002,
    SENSOR_INVALID_SENSOR   = 0x80000003,
    SENSOR_INVALID_PARA     = 0x80000004,
} SENSOR_ERROR_ENUM;

typedef enum {
    SENSOR_SOCKET_1 = 0,
    SENSOR_SOCKET_2 = 1,
    SENSOR_SOCKET_NONE = 0xFF,
}SENSOR_SOCKET_ENUM;

typedef struct
{
    MINT32              sensorID;
    MUINT8              index[MAX_NUM_OF_SUPPORT_SENSOR];
    IMAGE_SENSOR_TYPE   type[MAX_NUM_OF_SUPPORT_SENSOR];
    MUINT32             number;
    MUINT32             position;
    MUINT8              firstRawIndex;
    MUINT8              firstYuvIndex;
} SENSOR_DRIVER_LIST_T, *PSENSOR_DRIVER_LIST_T;

typedef struct
{
    SENSOR_DEV_ENUM sensorDevId;
    MUINT32         sId;
    MUINT32         InitFPS;
    MUINT32         HDRMode;
} SENSOR_DRIVER_SCENARIO_T, *PSENSOR_DRIVER_SCENARIO_T;

typedef struct
{
    // Part 1 : Basic information
    MUINT16 SensorPreviewResolutionX;
    MUINT16 SensorPreviewResolutionY;
    MUINT16 SensorPreviewResolutionX_ZSD;
    MUINT16 SensorPreviewResolutionY_ZSD;
    MUINT16 SensorFullResolutionX;
    MUINT16 SensorFullResolutionY;
    MUINT8  SensorClockFreq;              /* MHz */
    MUINT8  SensorCameraPreviewFrameRate;
    MUINT8  SensorVideoFrameRate;
    MUINT8  SensorStillCaptureFrameRate;
    MUINT8  SensorWebCamCaptureFrameRate;
    MUINT8  SensorClockPolarity;          /* SENSOR_CLOCK_POLARITY_HIGH/SENSOR_CLOCK_POLARITY_Low */
    MUINT8  SensorClockFallingPolarity;
    MUINT8  SensorClockRisingCount;       /* 0..15 */
    MUINT8  SensorClockFallingCount;      /* 0..15 */
    MUINT8  SensorClockDividCount;        /* 0..15 */
    MUINT8  SensorPixelClockCount;        /* 0..15 */
    MUINT8  SensorDataLatchCount;         /* 0..15 */
    MUINT8  SensorHsyncPolarity;
    MUINT8  SensorVsyncPolarity;
    MUINT8  SensorInterruptDelayLines;
    MINT32  SensorResetActiveHigh;
    MUINT32 SensorResetDelayCount;
    ACDK_SENSOR_INTERFACE_TYPE_ENUM     SensroInterfaceType;
    ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM SensorOutputDataFormat;
    ACDK_SENSOR_MIPI_LANE_NUMBER_ENUM   SensorMIPILaneNumber;   /* lane number : 1, 2, 3, 4 */
    MUINT32 CaptureDelayFrame;
    MUINT32 PreviewDelayFrame;
    MUINT32 VideoDelayFrame;
    MUINT32 HighSpeedVideoDelayFrame;
    MUINT32 SlimVideoDelayFrame;
    MUINT32 YUVAwbDelayFrame;
    MUINT32 YUVEffectDelayFrame;
    MUINT32 Custom1DelayFrame;
    MUINT32 Custom2DelayFrame;
    MUINT32 Custom3DelayFrame;
    MUINT32 Custom4DelayFrame;
    MUINT32 Custom5DelayFrame;
    MUINT16 SensorGrabStartX_PRV;       /* MSDK_SCENARIO_ID_CAMERA_PREVIEW */
    MUINT16 SensorGrabStartY_PRV;
    MUINT16 SensorGrabStartX_CAP;       /* MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG & ZSD */
    MUINT16 SensorGrabStartY_CAP;
    MUINT16 SensorGrabStartX_VD;        /* MSDK_SCENARIO_ID_VIDEO_PREVIEW */
    MUINT16 SensorGrabStartY_VD;
    MUINT16 SensorGrabStartX_VD1;       /* MSDK_SCENARIO_ID_VIDEO_PREVIEW */
    MUINT16 SensorGrabStartY_VD1;
    MUINT16 SensorGrabStartX_VD2;       /* MSDK_SCENARIO_ID_VIDEO_PREVIEW */
    MUINT16 SensorGrabStartY_VD2;
    MUINT16 SensorGrabStartX_CST1;      /* SENSOR_SCENARIO_ID_CUSTOM1 */
    MUINT16 SensorGrabStartY_CST1;
    MUINT16 SensorGrabStartX_CST2;      /* SENSOR_SCENARIO_ID_CUSTOM2 */
    MUINT16 SensorGrabStartY_CST2;
    MUINT16 SensorGrabStartX_CST3;       /* SENSOR_SCENARIO_ID_CUSTOM3 */
    MUINT16 SensorGrabStartY_CST3;
    MUINT16 SensorGrabStartX_CST4;       /* SENSOR_SCENARIO_ID_CUSTOM4 */
    MUINT16 SensorGrabStartY_CST4;
    MUINT16 SensorGrabStartX_CST5;       /* SENSOR_SCENARIO_ID_CUSTOM5 */
    MUINT16 SensorGrabStartY_CST5;
    MUINT16 SensorDrivingCurrent;
    MUINT8  SensorMasterClockSwitch;
    MUINT8  AEShutDelayFrame;             /* The frame of setting shutter default 0 for TG int */
    MUINT8  AESensorGainDelayFrame;   /* The frame of setting sensor gain */
    MUINT8  AEISPGainDelayFrame;
    MUINT8  MIPIDataLowPwr2HighSpeedTermDelayCount;
    MUINT8  MIPIDataLowPwr2HSSettleDelayM0;/*Preview Settle delay*/
    MUINT8  MIPIDataLowPwr2HSSettleDelayM1;/*Capture Settle delay*/
    MUINT8  MIPIDataLowPwr2HSSettleDelayM2;/*video Settle delay*/
    MUINT8  MIPIDataLowPwr2HSSettleDelayM3;/*video1 Settle delay*/
    MUINT8  MIPIDataLowPwr2HSSettleDelayM4;/*video2 Settle delay*/
    MUINT8  MIPIDataLowPwr2HighSpeedSettleDelayCount;
    MUINT8  MIPICLKLowPwr2HighSpeedTermDelayCount;
    MUINT8  SensorWidthSampling;
    MUINT8  SensorHightSampling;
    MUINT8  SensorPacketECCOrder;
    MUINT8  iHDR_First_IS_LE;
    MBOOL   iHDRSupport;
    MUINT8  PDAF_Support;
    MUINT8  SensorModeNum;
    MBOOL   virtualChannelSupport;
    SENSOR_MIPI_TYPE_ENUM MIPIsensorType;
    SENSOR_SETTLEDELAY_MODE_ENUM SettleDelayMode;
    MUINT8 IMGSENSOR_DPCM_TYPE_PRE;
    MUINT8 IMGSENSOR_DPCM_TYPE_CAP;
    MUINT8 IMGSENSOR_DPCM_TYPE_VD;
    MUINT8 IMGSENSOR_DPCM_TYPE_VD1;
    MUINT8 IMGSENSOR_DPCM_TYPE_VD2;
    MUINT8 PerFrameCTL_Support;
    SENSOR_SCAM_DATA_CHANNEL_ENUM SCAM_DataNumber;
    MUINT8 SCAM_DDR_En;
    MUINT8 SCAM_CLK_INV;
    // Part 2 : Config information
    ACDK_SENSOR_IMAGE_MIRROR_ENUM   SensorImageMirror;
    MINT32    EnableShutterTansfer;            /* capture only */
    MINT32    EnableFlashlightTansfer;        /* flash light capture only */
    ACDK_SENSOR_OPERATION_MODE_ENUM    SensorOperationMode;
    MUINT16 ImageTargetWidth;        /* image captured width */
    MUINT16 ImageTargetHeight;        /* image captuerd height */
    MUINT16    CaptureShutter;            /* capture only */
    MUINT16    FlashlightDuty;            /* flash light capture only */
    MUINT16    FlashlightOffset;        /* flash light capture only */
    MUINT16    FlashlightShutFactor;    /* flash light capture only */
    MUINT16     FlashlightMinShutter;
    ACDK_CAMERA_OPERATION_MODE_ENUM MetaMode; /* capture only */
    MUINT32 DefaultPclk;                /*Sensor pixel clock(Ex:24000000) */
    MUINT32 Pixels;                     /* Sensor active pixel number */
    MUINT32 Lines;                      /* Sensor active line number */
    MUINT32 Shutter;                    /* ensor current shutter */
    MUINT32 FrameLines;                 /* valid+dummy lines for minimum shutter */
    //Part 3 : Resolution Information
    MUINT16 SensorPreviewWidth;
    MUINT16 SensorPreviewHeight;
    MUINT16 SensorCapWidth;
    MUINT16 SensorCapHeight;
    MUINT16 SensorVideoWidth;
    MUINT16 SensorVideoHeight;
    MUINT16 SensorVideo1Width;
    MUINT16 SensorVideo1Height;
    MUINT16 SensorVideo2Width;
    MUINT16 SensorVideo2Height;
    MUINT16 SensorCustom1Width;   // new for custom
    MUINT16 SensorCustom1Height;
    MUINT16 SensorCustom2Width;
    MUINT16 SensorCustom2Height;
    MUINT16 SensorCustom3Width;
    MUINT16 SensorCustom3Height;
    MUINT16 SensorCustom4Width;
    MUINT16 SensorCustom4Height;
    MUINT16 SensorCustom5Width;
    MUINT16 SensorCustom5Height;
    MUINT16 SensorEffectivePreviewWidth;
    MUINT16 SensorEffectivePreviewHeight;
    MUINT16 SensorEffectiveCapWidth;
    MUINT16 SensorEffectiveCapHeight;
    MUINT16 SensorEffectiveVideoWidth;
    MUINT16 SensorEffectiveVideoHeight;
    MUINT16 SensorPreviewWidthOffset;           /* from effective width to output width*/
    MUINT16 SensorPreviewHeightOffset;          /* rom effective height to output height */
    MUINT16 SensorCapWidthOffset;              /* from effective width to output width */
    MUINT16 SensorCapHeightOffset;             /* rom effective height to output height */
    MUINT16 SensorVideoWidthOffset;             /* from effective width to output width */
    MUINT16 SensorVideoHeightOffset;            /* from effective height to output height */

    //Part4:
} SENSORDRV_INFO_STRUCT, *PSENSORDRV_INFO_STRUCT;


/*******************************************************************************
*
********************************************************************************/
namespace NSFeature
{
    class SensorInfoBase;
};  //NSFeature

typedef MINT32 (*pfExIdChk)(void);

class SensorDrv {
public:
    //
    //static SensorDrv* createInstance();
    virtual void      destroyInstance() = 0;
    static SensorDrv* get();


protected:
    virtual ~SensorDrv() {};

public:
    virtual MINT32 init(MINT32 sensorIdx) = 0;
    virtual MINT32 uninit() = 0;

    //static MINT32 searchSensor(pfExIdChk pExIdChkCbf);
    virtual MINT32 open(MINT32 sensorIdx) = 0;
    virtual MINT32 close(MINT32 sensorIdx)= 0;

    virtual MINT32 setScenario(SENSOR_DRIVER_SCENARIO_T scenarioconf) = 0;

    virtual MINT32 start() = 0;
    virtual MINT32 stop() = 0;

    virtual MINT32 waitSensorEventDone( MUINT32 EventType, MUINT32 Timeout)= 0;//HDR

    virtual MINT32 getInfo(MUINT32 ScenarioId[2],ACDK_SENSOR_INFO_STRUCT *pSensorInfo[2],ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData[2]) = 0;
    virtual MINT32 getInfo2(SENSOR_DEV_ENUM sensorDevId, SENSORDRV_INFO_STRUCT *pSensorInfo) = 0;
    virtual MINT32 getResolution(ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution[2]) = 0;

    virtual MINT32 sendCommand( SENSOR_DEV_ENUM sensorDevId, MUINT32 cmd, MUINTPTR parg1 = 0, MUINTPTR parg2 = 0, MUINTPTR parg3 = 0) = 0;

    virtual MINT32 setFoundDrvsActive(MUINT32 socketIdxes) = 0;

    virtual MUINT32 getMainSensorID() const = 0;
    virtual MUINT32 getMain2SensorID() const = 0;
    virtual MUINT32 getSubSensorID() const = 0;
    virtual IMAGE_SENSOR_TYPE getCurrentSensorType(SENSOR_DEV_ENUM sensorDevId) = 0;
    virtual NSFeature::SensorInfoBase*  getMainSensorInfo() const = 0;
    virtual NSFeature::SensorInfoBase*  getMain2SensorInfo() const = 0;
    virtual NSFeature::SensorInfoBase*  getSubSensorInfo()  const = 0;
    virtual MINT32 releaseI2CTriggerLock() =0;
//private://6593
    virtual MINT32 impSearchSensor(pfExIdChk pExIdChkCbf) = 0;
};

/*******************************************************************************
*
********************************************************************************/

#endif // _SENSOR_DRV_H

