//MTK_SWIP_PROJECT_START
#ifndef __MTK_TSF_H__
#define __MTK_TSF_H__

#define TSF_ALGO_VERSION 2.0
#define SHADING_VER 3  // 2, 3, 4

#include "MTKTsfType.h"
#include "MTKTsfErrCode.h"

typedef enum DRVTsfObject_s {
    DRV_TSF_OBJ_NONE = 0,
    DRV_TSF_OBJ_SW,
    DRV_TSF_OBJ_SW_NEON,
    DRV_TSF_OBJ_UNKNOWN = 0xFF,
} DrvTsfObject_e;

typedef enum
{
    MTKTSF_STATE_STANDBY = 0,               // After create Obj or Reset
    MTKTSF_STATE_INIT,                // After init environment info
    MTKTSF_STATE_MAIN,
    MTKTSF_STATE_READY,
	MTKTSF_STATE_OPT_DONE,
    MTKTSF_STATE_ISP_DONE,
    MTKTSF_STATE_MAX
} MTKTSF_STATE_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
    MTKTSF_FEATURE_BEGIN,                // minimum of feature id
    MTKTSF_FEATURE_SET_ENV_INFO,        // feature id to setup environment information
    MTKTSF_FEATURE_GET_ENV_INFO,        // feature id to retrieve environment information
    MTKTSF_FEATURE_SET_PROC_INFO,        // feature id to setup processing information
    MTKTSF_FEATURE_GET_PROC_INFO,        // feature id to retrieve environment information
    MTKTSF_FEATURE_GET_RESULT,            // feature id to get result
    MTKTSF_FEATURE_GEN_CAP_TBL,            // feature id to generate capture image table
    MTKTSF_FEATURE_SET_TBL_CHANGE,        // feature id to set table info if table change
    MTKTSF_FEATURE_FORWARD_CONVERT,
    MTKTSF_FEATURE_BACKWARD_CONVERT,
    MTKTSF_FEATURE_BATCH,
	MTKTSF_FEATURE_MERGE_LUMA_TBL,		// feature id to merge per-frame luma table with the same LSC config, independent with tsf flow.
    MTKTSF_FEATURE_GET_LOG,                // feature id to get debugging log
    MTKTSF_FEATURE_CONFIG_SMOOTH,
    MTKTSF_FEATURE_MAX                    // maximum of feature id
}    MTKTSF_FEATURE_ENUM;

typedef enum
{
    MTK_BAYER_B = 0,
    MTK_BAYER_Gb,
    MTK_BAYER_Gr,
    MTK_BAYER_R,
    MTK_BAYER_NO
} MTK_BAYER_ORDER_ENUM;

typedef struct
{
    int raw_wd;
    int raw_ht;
    int block_wd;
    int block_ht;
    int block_wd_last;
    int block_ht_last;
    int x_grid_num;
    int y_grid_num;
    int x_offset;
    int y_offset;
} MTK_TSF_LSC_PARAM_STRUCT, *P_MTK_TSF_LSC_PARAM_STRUCT;

typedef struct
{
    int SL2_CENTR_X;
    int SL2_CENTR_Y;
    int SL2_RR_0;
    int SL2_RR_1;
    int SL2_RR_2;
} MTK_TSF_SL2_PARAM_STRUCT, *P_MTK_TSF_SL2_PARAM_STRUCT;

typedef struct GAIN_TBL
{
    MUINT32        offset_x;
    MUINT32        offset_y;
    MUINT32        crop_width;
    MUINT32        crop_height;
    MTK_BAYER_ORDER_ENUM  bayer;
    MUINT32        grid_x;
    MUINT32        grid_y;
    MUINT32        *Tbl;
} GAIN_TBL;

// change the id order, so that 0~3 is native to the coordinate flipping
// low bit: flip x (horizontal) coordinate
// high bit: flip y (vertical) coordinate
typedef enum
{
  MTKTSF_AFN_R0D,    // clockwise rotation 0 degree,	rotation id: 0
  MTKTSF_AFN_MIRROR, // left-right,					rotation id: 1
  MTKTSF_AFN_FLIP,    // up-down,						rotation id: 2
  MTKTSF_AFN_R180D,  // clockwise rotation 180 degree,	rotation id: 3
  MTKTSF_AFN_R90D,   // clockwise rotation 90 degree,	not being supported
  MTKTSF_AFN_R270D,  // clockwise rotation 270 degree, not being supported
  MTKTSF_AFN_MAX
} MTKTSF_AFN_T;

typedef struct
{
    GAIN_TBL        Golden;
    GAIN_TBL        Unit;
} MTK_TSF_NVRAM_TBL_STRUCT, *P_MTK_TSF_NVRAM_TBL_STRUCT;

// sensor cropping information
typedef struct
{
    MUINT32        full_width;
    MUINT32        full_height;
    MUINT32        resize_width;
    MUINT32        resize_height;
    MUINT32        crop_hor_offs;
    MUINT32        crop_ver_offs;
    MUINT32        crop_width;
    MUINT32        crop_height;
} MTK_TSF_SENSOR_CROP_STRUCT, *P_MTK_TSF_SENSOR_CROP_STRUCT;

// AWB gain
typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4G; // G gain
    MINT32 i4B; // B gain
} MTK_TSF_AWB_GAIN_T;

// AWB NVRAM structure
typedef struct
{
    MTK_TSF_AWB_GAIN_T rUnitGain;
    MTK_TSF_AWB_GAIN_T rGoldenGain;
    MTK_TSF_AWB_GAIN_T rD65Gain;
} MTK_TSF_AWB_NVRAM_T;

typedef struct
{
    MUINT16                        ImgWidth;                        // input image width
    MUINT16                        ImgHeight;                       // input image height
    MTK_BAYER_ORDER_ENUM        BayerOrder;
    MTK_TSF_LSC_PARAM_STRUCT    *pLscConfig;
    MBOOL                        WithOTP;
    MTK_TSF_NVRAM_TBL_STRUCT    ShadingTbl;
    MINT32                        *BaseShadingTbl;
    MUINT32                        *Para;
    MINT32                        *pTuningPara;
    MUINT32                        *WorkingBufAddr;
#ifdef __AWB_10b__ // which defines in VC project property
    unsigned short                        *ImgAddr;
#else
    MUINT8                        *ImgAddr;
#endif
    MUINT8                        TS_TS;            // 0(disable); 1(enable)
    MUINT8                        MA_NUM;            // 3~7
    MUINT8                        Raw16_9Mode;    // 0(4:3); 1(16:9)
    MUINT32                        *DebugAddr;
    MUINT32                        DebugFlag;
    MUINT32                        EnableSL2;
    MBOOL                        EnableORCorrection;            // over-reddish correcton
    MTK_TSF_AWB_NVRAM_T            AwbNvramInfo;
    MTK_TSF_SENSOR_CROP_STRUCT    SensorCrop;
	MTKTSF_AFN_T                afn;
} MTK_TSF_ENV_INFO_STRUCT, *P_MTK_TSF_ENV_INFO_STRUCT;

typedef struct
{
    MTK_TSF_LSC_PARAM_STRUCT    *pLscConfig;
    MINT32                        *ShadingTbl;
} MTK_TSF_TBL_STRUCT, *P_MTK_TSF_TBL_STRUCT;

typedef struct
{
	//MTK_TSF_LSC_PARAM_STRUCT	*pLscConfig;
	MINT32						*InLumaTbl;
	MINT32						*OutCoefTbl;	
} MTK_TSF_MERGE_TBL_STRUCT, *P_MTK_TSF_MERGE_TBL_STRUCT;

typedef struct
{
    MUINT32                WorkingBuffSize;    // for upper layer to query
    MUINT32                DebugBuffSize;        // for upper layer to query
} MTK_TSF_GET_ENV_INFO_STRUCT, *P_MTK_TSF_GET_ENV_INFO_STRUCT;

typedef struct
{
    MTKTSF_STATE_ENUM    TsfState;
} MTK_TSF_GET_PROC_INFO_STRUCT, *P_MTK_TSF_GET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT32                LogBuffAddr;
    MUINT32                LogBuffSize;    // for upper layer to query
    MUINT32                DebugBuffAddr;
    MUINT32                DebugBuffSize;    // for upper layer to query
} MTK_TSF_GET_LOG_INFO_STRUCT, *P_MTK_TSF_GET_LOG_INFO_STRUCT;

typedef struct
{
    MINT32 *ShadingTbl;
    MTK_TSF_AWB_GAIN_T    Gain;
    MINT32 ParaL;
    MINT32 ParaC;
    MINT32 FLUO_IDX;
    MINT32 DAY_FLUO_IDX;
    MINT32                LscRA;
} MTK_TSF_SET_PROC_INFO_STRUCT, *P_MTK_TSF_SET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT32 *ShadingTbl;
    MTK_TSF_SL2_PARAM_STRUCT    SL2Para;
    MINT32                        ExifData[296]; // For TSF V2.0
} MTK_TSF_RESULT_INFO_STRUCT, *P_MTK_TSF_RESULT_INFO_STRUCT;

class MTKTsf
{
public:
    static MTKTsf* createInstance();
    virtual void   destroyInstance(MTKTsf* obj) = 0;

    virtual ~MTKTsf(){}
    // Process Control
    virtual MRESULT TsfInit(void *InitInData, void *InitOutData);
    virtual MRESULT TsfMain(void);
    virtual MRESULT TsfExit(void);
    virtual MRESULT TsfReset(void);                 // RESET for each image

    // Feature Control
    virtual MRESULT TsfFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};


#endif  //__MTK_TSF_H__
//MTK_SWIP_PROJECT_END