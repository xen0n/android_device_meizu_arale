#ifndef __ISP_DATATYPES_H__
#define __ISP_DATATYPES_H__


//Kernel Includes
#include "camera_isp.h" //For ISP Function ID,and Kernel Flag
#include "crz_drv.h"
#include <mtkcam/common.h>
using namespace NSCam;


/*-----------------------------------------------------------------------------
    ISP ERROR CODE
  -----------------------------------------------------------------------------*/

#define ISP_ERROR_CODE_OK                      (0)
#define ISP_ERROR_CODE_FAIL                    (-1)

#define ISP_ERROR_CODE_FAIL_00                 (-1 & ~(0x1<<0))  /*-2 , (0xFFFFFFFE)*/
#define ISP_ERROR_CODE_FAIL_01                 (-1 & ~(0x1<<1))  /*-3 , (0xFFFFFFFD)*/
#define ISP_ERROR_CODE_FAIL_02                 (-1 & ~(0x1<<2))  /*-5 , (0xFFFFFFFB)*/
#define ISP_ERROR_CODE_FAIL_03                 (-1 & ~(0x1<<3))  /*-9 , (0xFFFFFFF7)*/
#define ISP_ERROR_CODE_FAIL_04                 (-1 & ~(0x1<<4))  /*-17 , (0xFFFFFFEF)*/
#define ISP_ERROR_CODE_LOCK_RESOURCE_FAIL      (-1 & ~(0x1<<5))  /*-33 , (0xFFFFFFDF)*/
#define ISP_ERROR_CODE_FAIL_06                 (-1 & ~(0x1<<6))  /*-65 , (0xFFFFFFBF)*/
#define ISP_ERROR_CODE_FAIL_07                 (-1 & ~(0x1<<7))  /*-129 , (0xFFFFFF7F)*/
#define ISP_ERROR_CODE_FAIL_08                 (-1 & ~(0x1<<8))  /*-257 , (0xFFFFFEFF)*/
#define ISP_ERROR_CODE_FAIL_09                 (-1 & ~(0x1<<9))  /*-513 , (0xFFFFFDFF)*/
#define ISP_ERROR_CODE_FAIL_10                 (-1 & ~(0x1<<10))  /*-1025 , (0xFFFFFBFF)*/
#define ISP_ERROR_CODE_FAIL_11                 (-1 & ~(0x1<<11))  /*-2049 , (0xFFFFF7FF)*/
#define ISP_ERROR_CODE_FAIL_12                 (-1 & ~(0x1<<12))  /*-4097 , (0xFFFFEFFF)*/
#define ISP_ERROR_CODE_FAIL_13                 (-1 & ~(0x1<<13))  /*-8193 , (0xFFFFDFFF)*/
#define ISP_ERROR_CODE_FAIL_14                 (-1 & ~(0x1<<14))  /*-16385 , (0xFFFFBFFF)*/
#define ISP_ERROR_CODE_FAIL_15                 (-1 & ~(0x1<<15))  /*-32769 , (0xFFFF7FFF)*/
#define ISP_ERROR_CODE_FAIL_16                 (-1 & ~(0x1<<16))  /*-65537 , (0xFFFEFFFF)*/
#define ISP_ERROR_CODE_FAIL_17                 (-1 & ~(0x1<<17))  /*-131073 , (0xFFFDFFFF)*/
#define ISP_ERROR_CODE_FAIL_18                 (-1 & ~(0x1<<18))  /*-262145 , (0xFFFBFFFF)*/
#define ISP_ERROR_CODE_FAIL_19                 (-1 & ~(0x1<<19))  /*-524289 , (0xFFF7FFFF)*/
#define ISP_ERROR_CODE_FAIL_20                 (-1 & ~(0x1<<20))  /*-1048577 , (0xFFEFFFFF)*/
#define ISP_ERROR_CODE_FAIL_21                 (-1 & ~(0x1<<21))  /*-2097153 , (0xFFDFFFFF)*/
#define ISP_ERROR_CODE_FAIL_22                 (-1 & ~(0x1<<22))  /*-4194305 , (0xFFBFFFFF)*/
#define ISP_ERROR_CODE_FAIL_23                 (-1 & ~(0x1<<23))  /*-8388609 , (0xFF7FFFFF)*/
#define ISP_ERROR_CODE_FAIL_24                 (-1 & ~(0x1<<24))  /*-16777217 , (0xFEFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_25                 (-1 & ~(0x1<<25))  /*-33554433 , (0xFDFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_26                 (-1 & ~(0x1<<26))  /*-67108865 , (0xFBFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_27                 (-1 & ~(0x1<<27))  /*-134217729 , (0xF7FFFFFF)*/
#define ISP_ERROR_CODE_FAIL_28                 (-1 & ~(0x1<<28))  /*-268435457 , (0xEFFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_29                 (-1 & ~(0x1<<29))  /*-536870913 , (0xDFFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_30                 (-1 & ~(0x1<<30))  /*-1073741825 , (0xBFFFFFFF)*/


#define ISP_IS_ERROR_CODE( _retval_ , _errorcode_ ) \
            (   _retval_ >= 0    ?    0    :    ( (( _retval_|_errorcode_)==_errorcode_) ? 1 : 0 )     )



/*-----------------------------------------------------------------------------
    DATA STRUCTURE
  -----------------------------------------------------------------------------*/


class IspSize
{
public:
    unsigned long w;
    unsigned long h;
    unsigned long stride;
    unsigned long xsize;    //unit:byte
public:
    IspSize():
        w(0),h(0), stride(0), xsize(0)
        {};


    IspSize(unsigned long _w, unsigned long _h )
        {
            w = _w; h = _h;
        };
};

class IspPoint
{
public:
    long    x;
    long    y;

public:
    IspPoint():
        x(0),y(0)
        {};

   IspPoint(unsigned long _x, unsigned long _y )
       {
           x = _x; y = _y;
       };
};

class IspRect
{
public:
    MUINT32         x;
    MUINT32         y;
    MUINT32         floatX; /* x float precise - 32 bit */
    MUINT32         floatY; /* y float precise - 32 bit */
    unsigned long   w;
    unsigned long   h;


public:
    IspRect():
        x(0),y(0),floatX(0),floatY(0),w(0),h(0)
        {};

   IspRect(long _x, long _y, long _floatX, long _floatY, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; floatX = _floatX; floatY = _floatY; w = _w; h = _h;
        };

};

class IspYuvAddr
{
public:
    unsigned long   y;
    unsigned long   u;
    unsigned long   v;

    /*User need not fill in the data below!-----------------------------------------------------------*/
    /*Below data is auto fill by ISP driver calculation!----------------------------------------------*/

    unsigned long   y_buffer_size;
    unsigned long   u_buffer_size;
    unsigned long   v_buffer_size;

public:
    IspYuvAddr():
        y(0), u(0), v(0),
        y_buffer_size(0), u_buffer_size(0), v_buffer_size(0)
        {};
};


class IspMemBuffer
{
public:
    MUINT32 size;
    MUINTPTR base_vAddr;
    MUINTPTR base_pAddr;
    MUINT32 ofst_addr;
    MUINT32 alignment;
public:
    IspMemBuffer():
        size(0),base_vAddr(0),base_pAddr(0),ofst_addr(0), alignment(16)
        {};
};

class IspOffset
{
public:
    MUINT32 x;
    MUINT32 y;
public:
    IspOffset():
        x(0),y(0)
        {};
};

class IspDMACfg
{
public:
    IspMemBuffer    memBuf;
    IspSize         size;
    IspOffset       offset;
    IspRect         crop;
    int             pixel_byte;
    int             swap;
    int             format_en;
    int             format;
    int             bus_size_en;
    int             bus_size;
    int             ring_en;
    int             ring_size;
    int             memBuf_c_ofst;
    int             memBuf_v_ofst;
    int             v_flip_en;
    EImageFormat    lIspColorfmt;  //SL TEST_MDP_YUV
    MUINT32         capbility;      //port capbility
    MUINT32  tgFps;          //tgFps
    IspDMACfg():
        format_en(0),format(0),bus_size_en(0),bus_size(0),pixel_byte(1),capbility(0x00),tgFps(30)
        {};
};

class MdpRotDMACfg
{
public:
    IspMemBuffer    memBuf;
    IspSize         size;
    IspMemBuffer    memBuf_c;
    IspSize         size_c;
    IspMemBuffer    memBuf_v;
    IspSize         size_v;
    int             crop_en;    //dma crop
    IspRect         crop;       //dma crop
    int             pixel_byte;
    int             uv_plane_swap;
    CRZ_DRV_FORMAT_ENUM         Format; /* DISPO_FORMAT_1 */
    CRZ_DRV_PLANE_ENUM          Plane;  /* DISPO_FORMAT_3 */
    CRZ_DRV_SEQUENCE_ENUM       Sequence;
    CRZ_DRV_ROTATION_ENUM       Rotation;
    /* crop information for the hw module before the dma(if support) */
    MBOOL withCropM;                    // there is crop module before the dma port or not
    MBOOL enSrcCrop;                    // enable src crop in the previous hw module
    MUINT32 srcCropX;                       //! X integer start position for cropping
    MUINT32 srcCropFloatX;                  //! X float start position for cropping
    MUINT32 srcCropY;                       //! Y integer start position for crpping
    MUINT32 srcCropFloatY;                  //! Y float start position for cropping
    MUINT32 srcCropW;                       //! width of cropped image
    MUINT32 srcCropH;                       //! height of cropped image
    MBOOL Flip;
    MUINT32         capbility;      //port capbility
    public:
        MdpRotDMACfg():
            pixel_byte(1),capbility(0x00) {};
};

class JpgParaCfg
{
public:
    MINT32          memID;
    IspMemBuffer    workingMemBuf;
    int             soi_en;
    MUINT32         fQuality;
};


class IspCapTdriCfg
{
public:
    int isCalculateTpipe;
    MUINT32 setSimpleConfIdxNumVa;
    MUINT32 segSimpleConfBufVa;
};

//
struct stIspTopCtl {
    CAM_REG_CTL_EN_P1          CAM_CTL_EN_P1;
    CAM_REG_CTL_EN_P1_DMA      CAM_CTL_EN_P1_DMA;
    CAM_REG_CTL_EN_P1_D        CAM_CTL_EN_P1_D;
    CAM_REG_CTL_EN_P1_DMA_D    CAM_CTL_EN_P1_DMA_D;
    CAM_REG_CTL_EN_P2          CAM_CTL_EN_P2;
    CAM_REG_CTL_EN_P2_DMA      CAM_CTL_EN_P2_DMA;
    CAM_REG_CTL_CQ_EN          CAM_CTL_CQ_EN;
    CAM_REG_CTL_SCENARIO       CAM_CTL_SCENARIO;
    CAM_REG_CTL_FMT_SEL_P1     CAM_CTL_FMT_SEL_P1;
    MUINT32                    CAM_CTL_FMT_SEL_P1_MASK;
    CAM_REG_CTL_FMT_SEL_P1_D   CAM_CTL_FMT_SEL_P1_D;
    MUINT32                    CAM_CTL_FMT_SEL_P1_D_MASK;
    CAM_REG_CTL_FMT_SEL_P2     CAM_CTL_FMT_SEL_P2;
    CAM_REG_CTL_SEL_P1         CAM_CTL_SEL_P1;
    CAM_REG_CTL_SEL_P1_D       CAM_CTL_SEL_P1_D;
    CAM_REG_CTL_SEL_P2         CAM_CTL_SEL_P2;
    CAM_REG_CTL_SEL_GLOBAL     CAM_CTL_SEL_GLOBAL;
    MUINT32                    CAM_CTL_SEL_GLOBAL_MASK;
    CAM_REG_CTL_INT_P1_EN      CAM_CTL_INT_P1_EN;
    CAM_REG_CTL_INT_P1_EN2     CAM_CTL_INT_P1_EN2;
    CAM_REG_CTL_INT_P1_EN_D    CAM_CTL_INT_P1_EN_D;
    CAM_REG_CTL_INT_P1_EN2_D   CAM_CTL_INT_P1_EN2_D;
    CAM_REG_CTL_INT_P2_EN      CAM_CTL_INT_P2_EN;
    CAM_REG_CTL_CQ_EN_P2       CAM_CTL_CQ_EN_P2;
    CAMSV_REG_CAMSV_MODULE_EN  CAMSV_CAMSV_MODULE_EN;
    CAMSV_REG_CAMSV2_MODULE_EN CAMSV_CAMSV2_MODULE_EN;
    CAMSV_REG_CAMSV_FMT_SEL       CAMSV_CAMSV_FMT_SEL;
    MUINT32                       CAMSV_CAMSV_FMT_SEL_MASK;
    CAMSV_REG_CAMSV_FMT_SEL       CAMSV_CAMSV2_FMT_SEL;
    MUINT32                       CAMSV_CAMSV2_FMT_SEL_MASK;
    CAMSV_REG_CAMSV_INT_EN       CAMSV_CAMSV_INT_EN;
    CAMSV_REG_CAMSV2_INT_EN       CAMSV_CAMSV2_INT_EN;
    MUINT32                       CAMSV_CAMSV_PAK_MASK;
    CAMSV_REG_CAMSV_PAK           CAMSV_CAMSV_PAK;
    MUINT32                       CAMSV_CAMSV2_PAK_MASK;
    CAMSV_REG_CAMSV2_PAK       CAMSV_CAMSV2_PAK;

};

typedef struct stIspOBCCtl {
    CAM_REG_OBC_OFFST0              CAM_OBC_OFFST0;                 //4500
    CAM_REG_OBC_OFFST1              CAM_OBC_OFFST1;                 //4504
    CAM_REG_OBC_OFFST2              CAM_OBC_OFFST2;                 //4508
    CAM_REG_OBC_OFFST3              CAM_OBC_OFFST3;                 //450C
    CAM_REG_OBC_GAIN0               CAM_OBC_GAIN0;                  //4510
    CAM_REG_OBC_GAIN1               CAM_OBC_GAIN1;                  //4514
    CAM_REG_OBC_GAIN2               CAM_OBC_GAIN2;                  //4518
    CAM_REG_OBC_GAIN3               CAM_OBC_GAIN3;                  //451C
}stIspOBCCtl_t;

typedef struct stIspBPCCtl {
    MUINT32 dmmy;
}stIspBPCCtl_t;

typedef struct stIspRMMCtl {
    MUINT32 dmmy;
}stIspRMMCtl_t;

typedef struct stIspNR1Ctl {
    MUINT32 dmmy;

}stIspNR1Ctl_t;

typedef struct stIspLSCCtl {
    MUINT32 dmmy;

}stIspLSCCtl_t;

typedef struct stIspRPGCtl {
    MUINT32 dmmy;

}stIspRPGCtl_t;

typedef struct stIspAECtl {
    MUINT32 dmmy;

}stIspAECtl_t;

typedef struct stIspAWBCtl {
    MUINT32 dmmy;

}stIspAWBCtl_t;

typedef struct stIspSGG1Ctl {
    MUINT32 dmmy;

}stIspSGG1Ctl_t;

typedef struct stIspFLKCtl {
    MUINT32 dmmy;

}stIspFLKCtl_t;

typedef struct stIspAFCtl {
    MUINT32 dmmy;

}stIspAFCtl_t;

typedef struct stIspSGG2Ctl {
    MUINT32 dmmy;

}stIspSGG2Ctl_t;

typedef struct stIspEISCtl {
    MUINT32 dmmy;

}stIspEISCtl_t;

typedef struct stIspLCSCtl {
    MUINT32 dmmy;

}stIspLCSCtl_t;

class IspBnrCfg
{
public:
    int bpc_en;
    int bpc_tbl_en;
};

class IspLscCfg
{
public:
    int sdblk_width;
    int sdblk_xnum;
    int sdblk_last_width;
    int sdblk_height;
    int sdblk_ynum;
    int sdblk_last_height;
};
class IspLceCfg
{
public:
    int lce_bc_mag_kubnx;
    int lce_offset_x;
    int lce_bias_x;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_offset_y;
    int lce_bias_y;
    int lce_slm_height;
};
class IspNbcCfg
{
public:
    int anr_eny;
    int anr_enc;
    int anr_iir_mode;
    int anr_scale_mode;
};
class IspSeeeCfg
{
public:
    int se_edge;
    int usm_over_shrink_en;
};
class IspImgoCfg
{
public:
    int imgo_stride;
    int imgo_crop_en;
};
class IspEsfkoCfg
{
public:
    int esfko_stride;
};
class IspAaoCfg
{
public:
    int aao_stride;
};
class IspLcsoCfg
{
public:
    int lcso_stride;
    int lcso_crop_en;
};

class IspRrzCfg
{
public:
    unsigned long isTwinMode;
    unsigned long pass1_path;
    unsigned long lsc_en;
    unsigned long sd_lwidth;
    unsigned long sd_xnum;
    unsigned long m_num_0;
    unsigned long twin_lsc_crop_offx;
    unsigned long twin_lsc_crop_offy;
    unsigned long mImgSel;
    IspSize rrz_in_size;
    IspRect rrz_in_roi;
    IspSize rrz_out_size;
public:
    IspRrzCfg():mImgSel(0)
    {};
};


class IspP1TuneCfg
{
public:
    unsigned int  tuningUpdateBit;
    unsigned int  tuningEnP1;
    unsigned int  tuningEnP1Dma;
    unsigned int  tuningEnP1D;
    unsigned int  tuningEnP1DmaD;
    unsigned int  *pTuningBuf;
public:
        IspP1TuneCfg(){};

};


class IspP1Cfg
{
public:
    unsigned long bToEnable;
    unsigned long pass1_path;
    unsigned long module;
    void *pConfig;
public:
    IspP1Cfg(){};
};


class IspMdpCropCfg
{
public:
    MBOOL crz_enable;
    IspSize crz_in;
    IspRect    crz_crop;
    IspSize crz_out;
    MBOOL mdp_enable;
    EImageFormat mdp_src_fmt;
    IspSize mdp_Yin;
    IspSize mdp_UVin;
    MUINT32 mdp_src_size;
    MUINT32 mdp_src_Csize;
    MUINT32 mdp_src_Vsize;
    MUINT32 mdp_src_Plane_Num;
    IspRect    mdp_crop;
    IspSize mdp_out;
};


#if 1

class IspCrzCfg
{
public:
    int crz_input_crop_width;
    int crz_input_crop_height;
    int crz_output_width;
    int crz_output_height;
    int crz_horizontal_integer_offset;/* pixel base */
    int crz_horizontal_subpixel_offset;/* 20 bits base */
    int crz_vertical_integer_offset;/* pixel base */
    int crz_vertical_subpixel_offset;/* 20 bits base */
    int crz_horizontal_luma_algorithm;
    int crz_vertical_luma_algorithm;
    int crz_horizontal_coeff_step;
    int crz_vertical_coeff_step;
};
class IspCurzCfg
{
public:
    int curz_input_crop_width;
    int curz_input_crop_height;
    int curz_output_width;
    int curz_output_height;
    int curz_horizontal_integer_offset;/* pixel base */
    int curz_horizontal_subpixel_offset;/* 20 bits base */
    int curz_vertical_integer_offset;/* pixel base */
    int curz_vertical_subpixel_offset;/* 20 bits base */
    int curz_horizontal_coeff_step;
    int curz_vertical_coeff_step;
};

class IspFeCfg
{
public:
    int fem_harris_tpipe_mode;
};
class IspImg2oCfg
{
public:
    int img2o_stride;
    int img2o_crop_en;
    int img2o_xsize; /* byte size - 1 */
    int img2o_ysize; /* byte size - 1 */
};
class IspPrzCfg
{
public:
    int prz_input_crop_width;
    int prz_input_crop_height;
    int prz_output_width;
    int prz_output_height;
    int prz_horizontal_integer_offset;/* pixel base */
    int prz_horizontal_subpixel_offset;/* 20 bits base */
    int prz_vertical_integer_offset;/* pixel base */
    int prz_vertical_subpixel_offset;/* 20 bits base */
    int prz_horizontal_luma_algorithm;
    int prz_vertical_luma_algorithm;
    int prz_horizontal_coeff_step;
    int prz_vertical_coeff_step;
};

class IspMfbCfg
{
public:
    int bld_mode;
    int bld_deblock_en;
    int bld_brz_en;
};


class IspFlkiCfg
{
public:
    int flki_stride;
};
class IspCfaCfg
{
public:
    int bayer_bypass;
};

/**
*@brief  ISP sl2 configure struct
*/
class IspSl2Cfg
{
    public:
          int sl2_hrz_comp;
};

#endif

class IspTopCfg
{
public:
    unsigned int enable1;
    unsigned int enable2;
    unsigned int dma;
};


class IspTdriCfg
{
public:
    //enable table
    IspTopCfg   en_Top;

    /*===DMA===*/
    IspDMACfg imgi;
    IspDMACfg vipi;
    IspDMACfg vip2i;
    IspDMACfg imgci;
    IspDMACfg lcei;
    IspDMACfg lsci;
    MdpRotDMACfg wroto;
    MdpRotDMACfg wdmao;
    MdpRotDMACfg imgxo;

    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspLceCfg lce;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspImgoCfg imgo;
    IspEsfkoCfg esfko;
    IspAaoCfg aao;
    IspLcsoCfg lcso;

//    IspCdrzCfg cdrz;
//    IspCurzCfg curz;
//    IspFeCfg fe;
//    IspImg2oCfg img2o;
//    IspPrzCfg prz;
//    IspMfbCfg mfb;
//    IspFlkiCfg flki;
    IspCfaCfg cfa;
};


/*******************************************************************************
* JPG Info.
********************************************************************************/
class JPGCfg
{
public:
    int             soi_en;
    MUINT32         fQuality;
public:
    JPGCfg(){};
};

/*******************************************************************************
* RRZ Info.
********************************************************************************/

class DMACfg
{
public:
    IspRect crop;
    IspSize out;
    unsigned int m_num;
    unsigned int mImgSel;
public:
    DMACfg():mImgSel(0){};
};

typedef DMACfg RRZCfg;

typedef int (*pfCBFuncCfg)(void);

/*-----------------------------------------------------------------------------
    UTILITY MACRO
  -----------------------------------------------------------------------------*/
#define ISP_IS_ALIGN( _number_, _power_of_2_ )      ( ( _number_ & ( (0x1<<_power_of_2_)-1 ) ) ? 0 : 1 )
#define ISP_ROUND_UP( _number_, _power_of_2_ )      _number_ = ( ( (_number_ + (( 0x1 << _power_of_2_ )-1)) >> _power_of_2_ ) << _power_of_2_ )

#endif


