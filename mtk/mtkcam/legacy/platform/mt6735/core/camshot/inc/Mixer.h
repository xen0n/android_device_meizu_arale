

//#include "MTKMfbllType.h"
#include "MTKMfbllErrCode.h"

#define ABS(x)          ((x) < (0) ? -(x) : (x))
#define MIN(x, y)     ((x) <= (y)) ? (x) : (y)
#define MAX(x, y)     ((x) >= (y)) ? (x) : (y)
#define CLIP(v, x, y)  MAX(MIN((v), (y)), (x))
#define SV_ON 1
#define SV_OFF 0
#ifndef MF_POSTPROCESS_H
#define MF_POSTPROCESS_H

typedef struct
{
    MUINT32 ll_mixer_m0;
    MUINT32 ll_mixer_m1;
    MUINT32 ll_mixer_b0;
    MUINT32 ll_mixer_b1;
    MUINT32 img_width;
    MUINT32 img_height;
    MUINT8 *weight_table;
    MUINT8 *img_single;
    MUINT8 *img_mfb_blend;
    MUINT8 *tmpW_buffer;

} MFBLL_MIXER_DATA_STRUCT,*P_MFBLL_MIXER_DATA_STRUCT;


/*------------------------------------------------------------------------------
    Init Structure / Output Structure / Set Info Structure (Type definition should be exactly same as that in MTKMfbll.h)
------------------------------------------------------------------------------*/
typedef struct
{
    MUINT32 Level;
}MIXER_TUNING_PARA_STRUCT;

typedef struct
{
    MUINT16 Proc2_imgW;
    MUINT16 Proc2_imgH;
    MIXER_TUNING_PARA_STRUCT Proc2_tuning;
} MIXER_INIT_PARAM_STRUCT,*P_MIXER_INIT_PARAM_STRUCT;

typedef struct
{
    MUINT8 *img_mfbll_result;
} MIXER_OUT_STRUCT,*P_MIXER_OUT_STRUCT;

typedef struct
{
    MUINT8* Proc2_Img1;
    MUINT8* Proc2_Img2;
    MUINT32 Proc2_ImgW;
    MUINT32 Proc2_ImgH;
    MUINT8* weight_table;
    MUINT8* workbuf_addr;
} MIXER_SET_INFO_STRUCT, *P_MIXER_SET_INFO_STRUCT;


MRESULT MF_PostProcess(P_MIXER_OUT_STRUCT pMfbll_Out);
MRESULT MF_LPF1(void);
void Mixer_Init(P_MIXER_INIT_PARAM_STRUCT init_data);
MUINT32 Mixer_CalauBuffSize(void);
MUINT8 Mixer_SetProcInfo(P_MIXER_SET_INFO_STRUCT pSetProcInfo);

#endif

