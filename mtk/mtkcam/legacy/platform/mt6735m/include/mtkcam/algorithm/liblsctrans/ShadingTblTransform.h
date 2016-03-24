#ifndef SHADINGTBLTRANSFORM_H
#define SHADINGTBLTRANSFORM_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "MTKLscType.h"

#define LSCTRANS_SUPPORT_CH_NUM          (4)
//#define LSCTRANS_MAX_FRM_GRID_NUM                (17)
#define LSCTRANS_MAX_TIL_GRID_NUM                (33)
#define GRID_MAX LSCTRANS_MAX_TIL_GRID_NUM
#define BUFFERSIZE (GRID_MAX*GRID_MAX*3 + GRID_MAX*2 + (GRID_MAX-1)*(GRID_MAX-1)*12*4)// in float (4 byte)as unit

#define LSC_RA_BUFFER_SIZE (((GRID_MAX)*(GRID_MAX)*18 + (GRID_MAX)*(GRID_MAX)*48 + (GRID_MAX-1)*(GRID_MAX-1)*12*4 ) *4 ) // in bytes for LSC RA function

#define SHADING_VER 3  // 2, 3, 4

// temporary size *in byte*, added by Maggie, will need to be cleaned up later after the packing method is determined
//#define MAX_FRM_GAIN_SIZE        (LSCTRANS_MAX_FRM_GRID_NUM*LSCTRANS_MAX_FRM_GRID_NUM*LSCTRANS_SUPPORT_CH_NUM*2) // 2 bytes/pixel/channel for each gain
#define MAX_TIL_GAIN_SIZE        (LSCTRANS_MAX_TIL_GRID_NUM*LSCTRANS_MAX_TIL_GRID_NUM*LSCTRANS_SUPPORT_CH_NUM*2) // 2 bytes/pixel/channel for each gain
//#define MAX_FRM_COEFF_SIZE        ((LSCTRANS_MAX_FRM_GRID_NUM-1)*(LSCTRANS_MAX_FRM_GRID_NUM-1)*LSCTRANS_SUPPORT_CH_NUM*12/3*4) // pack every 3 coef into 4 bytes
#define MAX_TIL_COEFF_SIZE        ((LSCTRANS_MAX_TIL_GRID_NUM-1)*(LSCTRANS_MAX_TIL_GRID_NUM-1)*LSCTRANS_SUPPORT_CH_NUM*12/3*4) // pack every 3 coef into 4 bytes

#define SQRT_NORMALIZE_BIT 12

#ifdef FLOAT_VERSION
    #define AVG_PIXEL_SIZE (33)  //  (32) //(16)
#else
#define AVG_PIXEL_SIZE (16)
#endif
#define MAX_PIXEL_GAIN (4)// (each coefficient takes 10, 11 or 12 bits)

#define RATIO_POLY_BIT 8

#define FLOAT_VERSION
#define NEW_COMPENSATION 1
//#define PC_SIM    (0)
#define MEDIA_PATH ".\\data"
#define CUSTOMER_RELEASE 0
#define TOOL_Version    2.3// Add offset for tuning

#define LSC_Debug_Table_Output 0
#define GN_BIT 13


// Added By Janice, working buffer size of each function
//=================== Working Buffer Size Counting (in bytes) =====================//
#define SHADIND_FUNC_WORKING_BUFFER_SIZE                \
/*g_p_sdblk int  [(MAX_TIL_BLK_NUM-1)*(MAX_TIL_BLK_NUM-1)*12*4] (184512)*/			(((LSCTRANS_MAX_TIL_GRID_NUM-1)*(LSCTRANS_MAX_TIL_GRID_NUM-1)*12*4))*4\
/*g_coef_t_1d int[4*(MAX_TIL_BLK_NUM-1)*(MAX_TIL_BLK_NUM-1)*12] (184512) */			+(4*((LSCTRANS_MAX_TIL_GRID_NUM-1)*(LSCTRANS_MAX_TIL_GRID_NUM-1))*12)*4\
/*g_p_til_gain_1 int[MAX_SHADING_TIL_GAIN_SIZE/sizeof(UINT32)]	(8192) */				+MAX_TIL_GAIN_SIZE\
/*g_p_til_gain_2 int[MAX_SHADING_TIL_GAIN_SIZE/sizeof(UINT32)]	(8192) */				+MAX_TIL_GAIN_SIZE\
/*g_frm_coef int[MAX_SHADING_FRM_COEFF_SIZE/sizeof(UINT32)]		(14400)*/				+MAX_TIL_COEFF_SIZE\
/*g_zh2_1d float[MAX_TIL_BLK_NUM*MAX_TIL_BLK_NUM]				(4096)*/				+(LSCTRANS_MAX_TIL_GRID_NUM)*(LSCTRANS_MAX_TIL_GRID_NUM)*4\
/*g_zv2_1d float[MAX_TIL_BLK_NUM*MAX_TIL_BLK_NUM]				(4096)*/				+(LSCTRANS_MAX_TIL_GRID_NUM)*(LSCTRANS_MAX_TIL_GRID_NUM)*4\
/*g_zz_1d float[4*MAX_TIL_BLK_NUM*MAX_TIL_BLK_NUM]			(16384)*/				+(LSCTRANS_MAX_TIL_GRID_NUM)*(LSCTRANS_MAX_TIL_GRID_NUM)*4*4\


typedef enum
{
    BAYER_B,
    BAYER_GB,
    BAYER_GR,
    BAYER_R
} BAYER_ID_T;

typedef enum
{
  SHADING_TYPE_GAIN,  // gain table
  SHADING_TYPE_COEFF  // coeff table
} SHADING_TYPE;

// change the id order, so that 0~3 is native to the coordinate flipping
// low bit: flip x (horizontal) coordinate
// high bit: flip y (vertical) coordinate
typedef enum
{
  SHADING_AFN_R0D,    // clockwise rotation 0 degree,    rotation id: 0
  SHADING_AFN_MIRROR, // left-right,                    rotation id: 1
  SHADING_AFN_FLIP,    // up-down,                        rotation id: 2
  SHADING_AFN_R180D,  // clockwise rotation 180 degree,    rotation id: 3
  SHADING_AFN_R90D,   // clockwise rotation 90 degree,    not being supported
  SHADING_AFN_R270D,  // clockwise rotation 270 degree, not being supported
  SHADING_AFN_MAX
} SHADING_AFN_T;

typedef enum
{
  SHADING_GRGB_SAME_NO,
  SHADING_GRGB_SAME_YES
} SHADING_GRGB_SAME;    // if to lock the gr and gb to the same gain. Default is no.

typedef struct SHADING_TBL_SPEC
{
  unsigned int            img_width;   // orig/target img width
  unsigned int            img_height;  // orig/target img height
  unsigned int            offset_x;    // (0,0) at the upper left corner
  unsigned int            offset_y;    // (0,0) at the upper left corner
  unsigned int            crop_width;  // actual width of the input image
  unsigned int            crop_height; // actual height of the input image
  BAYER_ID_T        bayer;       // bayer id of the input image
  unsigned int            grid_x;      // input/output table x grid number
  unsigned int            grid_y;      // input/output table y grid number
  unsigned int            lwidth;      // input/output table last block width
  unsigned int            lheight;     // input/output table last block height
  unsigned int            ratio_idx;     // index for the compensation strength (0~20)//(0~10)
  SHADING_GRGB_SAME grgb_same;     // whether to use the same gains for Gr and Gb
  unsigned int            *table;      // input/output table
  SHADING_TYPE      data_type;   // input/output table type
} SHADING_TBL_SPEC;

typedef struct
{
  SHADING_TBL_SPEC  input;
  SHADING_TBL_SPEC  output;
  SHADING_AFN_T     afn;
  void                *working_buff_addr;
  unsigned int        working_buff_size;
} SHADIND_TRFM_CONF;

typedef struct SHADIND_ALIGN_CONF
{
  SHADING_TBL_SPEC  cali;   // per unit calibration data
  SHADING_TBL_SPEC  golden; // golden reference
  SHADING_TBL_SPEC  input;  // golden shading other than reference
  SHADING_TBL_SPEC  output; // per unit compensated golden shading
  void                *working_buff_addr;
  unsigned int        working_buff_size;
  SHADING_AFN_T     afn;    // The rotation angle of the golden, input, output w.r.t cali.
                            // The alignments of the golden, input, output are the same from the phone manafacturer,
                            // The alignment of the unit is from the camara manafacturer, and might be different of the others.
} SHADIND_ALIGN_CONF;

typedef enum
{
    S_LSC_CONVERT_OK,                        // Success to do shading table converstion
    E_LSC_CONVERT_BITS_OVERFLOW,            // overflow
    E_LSC_CONVERT_WRONG_INPUT,                // input info incorrect
    E_LSC_CONVERT_OUT_OF_WORKING_MEM        // working buffer size is not enough
}LSC_RESULT;

typedef struct
{
        float SL2_CENTR_X;
        float SL2_CENTR_Y;
        float SL2_RR_0;
        float SL2_RR_1;
        float SL2_RR_2;
        unsigned int regsetting[4];
} SL2_PARAM_T, *P_SL2_PARAM_T;

typedef struct {
    unsigned int reg_mn;
    unsigned int reg_info0;
    unsigned int reg_info1;
    unsigned int *src_tbl_addr;
    unsigned int *dst_tbl_addr;
    float *src_tbl_addr_float;
    SL2_PARAM_T sl2_setting;
} TBL_INFO_T;

typedef struct {
    TBL_INFO_T tbl_info;
    unsigned short *raw_img_addr;
} LSC_CALI_INFO_T;

 typedef struct
{
    float coef_a;
    float coef_b;
    float coef_c;
    float coef_d;
    float coef_e;
    float coef_f;
    int ratio_poly_flag;
}RATIO_POLY_T;


typedef struct
{
       int i4GridXNUM;
       int i4GridYNUM;
       int i4XINIBorder;
       int i4XENDBorder;
       int i4YINIBorder;
       int i4YENDBorder;
       int i4AvgWinSize;
       unsigned int u4ImgWidth;
       unsigned int u4ImgHeight;
       unsigned short u2BayerStart;
    RATIO_POLY_T poly_coef;
       int i4CompensationIdx;
}LSC_CAL_INI_PARAM_T;


typedef struct
{
    int raw_wd;
    int raw_ht;
    int plane_wd;
    int plane_ht;
    int bayer_order;
    int crop_ini_x;
    int crop_end_x;
    int crop_ini_y;
    int crop_end_y;
    int block_wd;
    int block_ht;
    int block_wd_last;
    int block_ht_last;
    int avg_pixel_size;
    int avg_pixel_size_bit;
    int x_grid_num;
    int y_grid_num;
    int pxl_gain_max;
    RATIO_POLY_T poly_coef;
    float compensationRatio;
}LSC_PARAM_T;

typedef struct _ShadingBlock {
	int				R[12];
	int				Gr[12];
	int				Gb[12];
	int				B[12];
} ShadingBlock;

typedef struct
{
#ifdef FLOAT_VERSION
    float max_val[4];
#else
    int max_val[4];
#endif
    int x_max_pos[4];
    int y_max_pos[4];
}LSC_RESULT_T;

const MINT32 ShadingATNTable[21][6]={
    {     0,             0,             0,             0,       0,       1000000,},    // 0  100%
    { 35556,       -195556,        131111,        -21111,       0,       1000000,},     // 1  95%
    {-640000,       1173333,       -760000,        126667,       0,       1000000,},     // 2  90%
    {-35556,        -17778,       -131111,         34444,       0,       1000000,},    // 3  85%
    {-142222,        355556,       -524444,        111111,       0,       1000000,},     // 4  80%
    {746667,      -1546667,        833333,       -283333,       0,       1000000,},     // 5  75%
    {640000,      -1173333,        440000,       -206667,       0,       1000000,},     // 6  70%
    {533333,       -800000,         46667,       -130000,       0,       1000000,},     // 7  65%
    {284444,       -711111,        408889,       -382222,       0,       1000000,},     // 8  60%
    {888889,      -1902222,       1037778,       -474444,       0,       1000000,},     // 9  55%
    {-497778,       1031111,       -875556,       -157778,       0,       1000000,},     // 10 50%
    {675556,      -2008889,       1531111,       -747778,       0,       1000000,},     // 11 45%
    {2133333,      -4906667,       3386667,      -1213333,       0,       1000000,},     // 12 40%
    {177778,       -977778,        975556,       -825556,       0,       1000000,},     // 13 35%
    { 71111,       -604444,        582222,       -748889,       0,       1000000,},     // 14 30%
    {5511111,     -11751111,       7522222,      -2032222,       0,       1000000,},     // 15 25%
    {3555556,      -7822222,       5111111,      -1644444,       0,       1000000,},     // 16 20%
    {320000,      -1333333,       1180000,      -1016667,       0,       1000000,},     // 17 15%
    {-1066667,       1600000,       -733333,       -700000,       0,       1000000,},     // 18 10%
    {391111,      -1297778,       1122222,      -1165556,       0,       1000000,},     // 19 5%
    {-924444,       1884444,      -1168889,       -771111,       0,       1000000 },     // 20 0%
};


typedef struct
{
	int				*in_tbl;
	SHADING_TYPE    in_data_type;   // input/output table type
	int				*out_tbl;
	SHADING_TYPE    out_data_type;   // input/output table type
	int				pix_id;
	int				ra;
	int*			working_buf;
	LSC_PARAM_T		lsc_config;
}LSC_RA_STRUCT;

typedef struct
{
	unsigned int	*in_tbl_prev;
	unsigned int	*in_tbl_next;
	SHADING_TYPE    in_data_type;   // input/output table type
	int				*out_tbl;
	SHADING_TYPE    out_data_type;   // input/output table type
	char			len_pos;
	int*			working_buf;
	LSC_PARAM_T		lsc_config;
}LSC_ZMITP_STRUCT;

LSC_RESULT shading_transform(SHADIND_TRFM_CONF &shading_conf);
LSC_RESULT shading_align_golden(SHADIND_ALIGN_CONF &trans);
LSC_RESULT LscGaintoHWTbl(float *p_pgn_float, unsigned int *p_lsc_tbl, int grid_x, int grid_y, int RawImgW, int RawImgH, void* WorkBuf, int BufSize);
LSC_RESULT LscSL2Calcu(TBL_INFO_T tbl_cal_info, int grid_x, int grid_y, int RawImgW, int RawImgH);
void vLSC_PARAM_INIT(LSC_CAL_INI_PARAM_T a_rLSCCaliINIParam);
void vLSC_Calibration_INIT(unsigned short* a_u1BufferAddr);
void vLSC_Calibration_END(void);
void vLsc_Remap_to_Bayer0(unsigned short *Src, unsigned short *Dst, unsigned int Width, unsigned int Height, int BayerOrder);
MINT32 mrLSC_Calibrate(LSC_CALI_INFO_T cali_info, MUINT8 a_u1Mode, MUINT16 a_u2SVDTermNum);

class MTKLscUtil
{
public:
    static MTKLscUtil* createInstance();
    virtual void   destroyInstance(MTKLscUtil* obj) = 0;
       
    virtual ~MTKLscUtil(){}
    // Process Control
    virtual void		LscRaSwInit(void *InitInData);	
    virtual LSC_RESULT	LscRaSwMain(void *MainData);

    virtual void		LscZmItpSwInit(void *InitInData);	
    virtual LSC_RESULT	LscZmItpSwMain(void *MainData);

private:
    
};

#endif
