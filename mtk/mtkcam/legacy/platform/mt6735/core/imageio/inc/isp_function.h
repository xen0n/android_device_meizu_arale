#ifndef __ISP_FUNCTION_H__
#define __ISP_FUNCTION_H__
//
#include <vector>
#include <map>
#include <list>
using namespace std;
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//#include <cutils/pmem.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
//
#include "isp_datatypes.h"
#include "camera_isp.h" //For Isp function ID
//#include "isp_drv_imp.h"
#include "crz_drv.h"
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv/twin_drv.h>
#include <mtkcam/drv/dual_isp_config.h>
#include <mtkcam/drv/imem_drv.h>

#include <mtkcam/imageio/ispio_pipe_scenario.h>    // For enum EDrvScenario.
#include "mtkcam/imageio/p1HwCfg.h" //add this for p1 tuning notification

//patch hw CmdQ bug.
//solution: config lsci && lsci_d even if lsci_en /lsci_d_en is disabled. this bug is caused by CmdQ
#define _CmdQ_HwBug_Patch_

//pass1 CQ0/CQ0_D working behavior,
//CQ0/cq0_D will run in event_trigger at p1_done if not defined.
//and have continus fw trigger when retrieve magic0 & magic 1
#define _PASS1_CQ_CONTINUOUS_MODE_

//patch fbc hw bug
//legacy: fbc can't reset in PIP, so use single mode to reset fbc. but if p2 run in ip-raw, although fbc_en is off, wcnt still be added to.
//            So, set FBC_NUM to 1 in order to clamp wcnt at 1, and then set rcnt_inc at top_cfg to clear FBC_CNT ,and finally set FBC_NUM back to 0.
#define _FBC_IN_IPRAW

//for FrameBased mode dummy frame magic number
#define _DUMMY_MAGIC    0x20000000


/*/////////////////////////////////////////////////////////////////////////////
    ISP Function ID
  /////////////////////////////////////////////////////////////////////////////*/
#define ISP_MAX_CONCURRENT_TG_NUM 3

// for crop function
#define CROP_FLOAT_PECISE_BIT   31    // precise 31 bit
#define CROP_TPIPE_PECISE_BIT   20
#define CROP_CDP_HARDWARE_BIT   8

//ISP path
#define ISP_PASS1       (0x01)
#define ISP_PASS1_D     (0x02)
#define ISP_PASS1_CAMSV     (0x04)
#define ISP_PASS1_CAMSV_D   (0x08)
#define ISP_PASS2    (0x10)
#define ISP_PASS2B   (0x20)
#define ISP_PASS2C   (0x40)
#define ISP_PASS_NONE (0xFFFFFFFF)

//start bit
enum {
    CAM_ISP_PASS1_START =0,
    CAM_ISP_PASS2_START,
    CAM_ISP_PASS2B_START,
    CAM_ISP_PASS2C_START,
    CAM_ISP_PASS1_CQ0_START,
    CAM_ISP_PASS1_CQ0B_START,
    CAM_ISP_PASS1_CQ0_D_START,
    CAM_ISP_PASS1_CQ0B_D_START,

    CAM_ISP_NONE_START,
};

//
enum {
    ISP_TOP = 0,    //0
    ISP_RAW,
    ISP_RGB,
    ISP_YUV,
    ISP_MDP,
    ISP_CDP,        //5
    ISP_TDRI,
    ISP_DMA_CQI,
    ISP_DMA_TDRI,
    ISP_DMA_IMGI,
    ISP_DMA_BPCI,   //10
    ISP_DMA_LSCI,
    ISP_DMA_BPCI_D,
    ISP_DMA_LSCI_D,
    ISP_DMA_UFDI,
    ISP_DMA_LCEI,   //15
    ISP_DMA_VIPI,
    ISP_DMA_VIP2I,
    ISP_DMA_VIP3I,
    ISP_DMA_IMGO,
    ISP_DMA_IMGO_D, //20
    ISP_DMA_UFEO,
    ISP_DMA_MFBO,
    ISP_DMA_IMG3O,
    ISP_DMA_IMG3BO,
    ISP_DMA_IMG3CO, //25
    ISP_DMA_RRZO,
    ISP_DMA_RRZO_D,
    ISP_DMA_LCSO,
    ISP_DMA_LCSO_D,
    ISP_DMA_ESFKO,  //30
    ISP_DMA_AFO_D,
    ISP_DMA_AAO,
    ISP_DMA_AAO_D,
    ISP_DMA_FEO,
    ISP_DMA_IMG2O,  //35
    ISP_DMA_CAMSV_IMGO,
    ISP_DMA_CAMSV2_IMGO,
    ISP_DMA_WDMAO,
    ISP_DMA_WROTO,
    ISP_DMA_JPEGO,
    ISP_DMA_VENC_STREAMO,
    ISP_BUFFER,
    ISP_TURNING,
    ISP_TWIN,
    ISP_CAMSV,      //45
    ISP_CRZ,
    ISP_DMA_AFO,
    ISP_DMA_EISO,
    ISP_FUNCTION_MAX_NUM
};

//isp top register range
#define MT6593_ISP_TOP_BASE             (ISP_BASE_HW + ISP_BASE_OFFSET)
#define MT6593_ISP_TOP_REG_RANGE        (ISP_BASE_RANGE - ISP_BASE_OFFSET)
//ISP_RAW
#define MT6593_ISP_RAW_BASE             (MT6593_ISP_TOP_BASE + 0x0)
#define MT6593_ISP_RAW_REG_RANGE        MT6593_ISP_TOP_REG_RANGE
//ISP_RGB
#define MT6593_ISP_RGB_BASE             (MT6593_ISP_TOP_BASE + 0x0)
#define MT6593_ISP_RGB_REG_RANGE        MT6593_ISP_TOP_REG_RANGE
//ISP_YUV
#define MT6593_ISP_YUV_BASE             (MT6593_ISP_TOP_BASE + 0x0)
#define MT6593_ISP_YUV_REG_RANGE        MT6593_ISP_TOP_REG_RANGE
//ISP_MDP
#define MT6593_ISP_MDP_BASE             (MT6593_ISP_TOP_BASE + 0x0)
#define MT6593_ISP_MDP_REG_RANGE        MT6593_ISP_TOP_REG_RANGE
//ISP_CRZ
#define MT6593_ISP_CRZ_BASE             (MT6593_ISP_TOP_BASE + 0x0)
#define MT6593_ISP_CRZ_REG_RANGE        0x1000
//ISP_DMA_TWIN
#define MT6593_ISP_TWIN_BASE            (MT6593_ISP_TOP_BASE + 0x6000)
#define MT6593_ISP_TWIN_REG_RANGE       0x0
//ISP_DMA_IMGI
#define MT6593_DMA_IMGI_BASE            (MT6593_ISP_TOP_BASE + 0x3230)
#define MT6593_DMA_IMGI_REG_RANGE       0x20
//ISP_DMA_BPCI
#define MT6593_DMA_BPCI_BASE           (MT6593_ISP_TOP_BASE + 0x3250)
#define MT6593_DMA_BPCI_REG_RANGE      0x1C
//ISP_DMA_LSCI
#define MT6593_DMA_LSCI_BASE            (MT6593_ISP_TOP_BASE + 0x326C)
#define MT6593_DMA_LSCI_REG_RANGE       0x1C
//ISP_DMA_UFDI
#define MT6593_DMA_UFDI_BASE            (MT6593_ISP_TOP_BASE + 0x3288)
#define MT6593_DMA_UFDI_REG_RANGE       0x1C
//ISP_DMA_LCEI
#define MT6593_DMA_LCEI_BASE            (MT6593_ISP_TOP_BASE + 0x32A4)
#define MT6593_DMA_LCEI_REG_RANGE       0x1C
//ISP_DMA_VIPI
#define MT6593_DMA_VIPI_BASE            (MT6593_ISP_TOP_BASE + 0x32C0)
#define MT6593_DMA_VIPI_REG_RANGE       0x20
//ISP_DMA_VIP2I
#define MT6593_DMA_VIP2I_BASE            (MT6593_ISP_TOP_BASE + 0x32E0)
#define MT6593_DMA_VIP2I_REG_RANGE       0x20
//ISP_DMA_IMGO
#define MT6593_DMA_IMGO_BASE            (MT6593_ISP_TOP_BASE + 0x3300)
#define MT6593_DMA_IMGO_REG_RANGE       0x20
//ISP_DMA_RRZO
#define MT6593_DMA_RRZO_BASE            (MT6593_ISP_TOP_BASE + 0x3320)
#define MT6593_DMA_RRZO_REG_RANGE       0x20
//ISP_DMA_RRZO_D
#define MT6593_DMA_RRZO_D_BASE            (MT6593_ISP_TOP_BASE + 0x2F0)
#define MT6593_DMA_RRZO_D_REG_RANGE       0x20
//ISP_DMA_LCSO
#define MT6593_DMA_LCSO_BASE            (MT6593_ISP_TOP_BASE + 0x3340)
#define MT6593_DMA_LCSO_REG_RANGE       0x1C
//ISP_DMA_EISO
#define MT6593_DMA_EISO_BASE            (MT6593_ISP_TOP_BASE + 0x335C)
#define MT6593_DMA_EISO_REG_RANGE       0x8
//ISP_DMA_AFO
#define MT6593_DMA_AFO_BASE             (MT6593_ISP_TOP_BASE + 0x3364)
#define MT6593_DMA_AFO_REG_RANGE        0x8
//ISP_DMA_ESFKO
#define MT6593_DMA_ESFKO_BASE            (MT6593_ISP_TOP_BASE + 0x336C)
#define MT6593_DMA_ESFKO_REG_RANGE       0x1C
//ISP_DMA_AAO
#define MT6593_DMA_AAO_BASE            (MT6593_ISP_TOP_BASE + 0x3388)
#define MT6593_DMA_AAO_REG_RANGE       0x1C
//ISP_DMA_VIP3I
#define MT6593_DMA_VIP3I_BASE            (MT6593_ISP_TOP_BASE + 0x33A4)
#define MT6593_DMA_VIP3I_REG_RANGE       0x20
//ISP_DMA_UFEO
#define MT6593_DMA_UFEO_BASE            (MT6593_ISP_TOP_BASE + 0x33C4)
#define MT6593_DMA_UFEO_REG_RANGE       0x1C
//ISP_DMA_MFBO
#define MT6593_DMA_MFBO_BASE            (MT6593_ISP_TOP_BASE + 0x33E0)
#define MT6593_DMA_MFBO_REG_RANGE       0x20
//ISP_DMA_IMG3BO
#define MT6593_DMA_IMG3BO_BASE            (MT6593_ISP_TOP_BASE + 0x3400)
#define MT6593_DMA_IMG3BO_REG_RANGE       0x20
//ISP_DMA_IMG3CO
#define MT6593_DMA_IMG3CO_BASE            (MT6593_ISP_TOP_BASE + 0x3420)
#define MT6593_DMA_IMG3CO_REG_RANGE       0x1C
//ISP_DMA_IMG2O
#define MT6593_DMA_IMG2O_BASE            (MT6593_ISP_TOP_BASE + 0x3440)
#define MT6593_DMA_IMG2O_REG_RANGE       0x20
//ISP_DMA_IMG3O
#define MT6593_DMA_IMG3O_BASE            (MT6593_ISP_TOP_BASE + 0x3460)
#define MT6593_DMA_IMG3O_REG_RANGE       0x20
//ISP_DMA_FEO
#define MT6593_DMA_FEO_BASE            (MT6593_ISP_TOP_BASE + 0x3480)
#define MT6593_DMA_FEO_REG_RANGE       0x1C
//ISP_DMA_BPCI_D
#define MT6593_DMA_BPCI_D_BASE            (MT6593_ISP_TOP_BASE + 0x349C)
#define MT6593_DMA_BPCI_D_REG_RANGE       0x1C
//ISP_DMA_LSCI_D
#define MT6593_DMA_LSCI_D_BASE            (MT6593_ISP_TOP_BASE + 0x34B8)
#define MT6593_DMA_LSCI_D_REG_RANGE       0x1C
//ISP_DMA_IMGO_D
#define MT6593_DMA_IMGO_D_BASE            (MT6593_ISP_TOP_BASE + 0x34D4)
#define MT6593_DMA_IMGO_D_REG_RANGE       0x20
//ISP_DMA_RRZO_D
#define MT6593_DMA_RRZO_D_BASE            (MT6593_ISP_TOP_BASE + 0x34F4)
#define MT6593_DMA_RRZO_D_REG_RANGE       0x20
//ISP_DMA_LCSO_D
#define MT6593_DMA_LCSO_D_BASE            (MT6593_ISP_TOP_BASE + 0x3514)
#define MT6593_DMA_LCSO_D_REG_RANGE       0x1C
//ISP_DMA_AFO_D
#define MT6593_DMA_AFO_D_BASE            (MT6593_ISP_TOP_BASE + 0x3530)
#define MT6593_DMA_AFO_D_REG_RANGE       0x1C
//ISP_DMA_AAO_D
#define MT6593_DMA_AAO_D_BASE            (MT6593_ISP_TOP_BASE + 0x354C)
#define MT6593_DMA_AAO_D_REG_RANGE       0x1C
//ISP_DMA_CQ0I
#define MT6593_DMA_CQI_BASE            (MT6593_ISP_TOP_BASE + 0x3210)
#define MT6593_DMA_CQI_REG_RANGE       0x08
//ISP_DMA_TDRI
#define MT6593_DMA_TDRI_BASE            (MT6593_ISP_TOP_BASE + 0x3204)
#define MT6593_DMA_TDRI_REG_RANGE       0xC
//ISP_DMA_CAMSV_IMGO
#define MT6593_DMA_CAMSV_IMGO_BASE          (MT6593_ISP_TOP_BASE + (0x9208 ))
#define MT6593_DMA_CAMSV_IMGO_REG_RANGE     (0x20)
//ISP_DMA_CAMSV2_IMGO
#define MT6593_DMA_CAMSV2_IMGO_BASE         (MT6593_ISP_TOP_BASE + (0X9228 ) )
#define MT6593_DMA_CAMSV2_IMGO_REG_RANGE    (0x20)


//
//#define ISP_MAX_RINGBUFFER_CNT  16


/*/////////////////////////////////////////////////////////////////////////////
    DMA sharing table
  /////////////////////////////////////////////////////////////////////////////*/
//-->definition compatible to mt6593_scenario.xlsx
#define ISP_HW_SCENARIO_IP         3
#define ISP_HW_SCENARIO_CC         6
#define ISP_HW_SCENARIO_NUM        2
//->//
#define ISP_SUB_MODE_RAW        0
#define ISP_SUB_MODE_YUV        1
#define ISP_SUB_MODE_RGB        2
#define ISP_SUB_MODE_JPG        3
#define ISP_SUB_MODE_MAX        4



#if 1  //remove it later
//cam_fmt_sel
//->//
#define ISP_SCENARIO_IC         0
#define ISP_SCENARIO_VR         1
#define ISP_SCENARIO_ZSD        2
#define ISP_SCENARIO_VEC        4
#define ISP_SCENARIO_RESERVE01  5
#define ISP_SCENARIO_N3D_IC     6
#define ISP_SCENARIO_N3D_VR     7
#define ISP_SCENARIO_MAX        8

#define ISP_SUB_MODE_MFB        5

#endif


//->//
#define CAM_FMT_SEL_TG_FMT_RAW8   0
#define CAM_FMT_SEL_TG_FMT_RAW10  1
#define CAM_FMT_SEL_TG_FMT_RAW12  2
#define CAM_FMT_SEL_TG_FMT_YUV422 3
#define CAM_FMT_SEL_TG_FMT_RGB565 5
#define CAM_FMT_SEL_TG_FMT_JPEG   7
//->//
#define CAM_FMT_SEL_YUV420_2P    0
#define CAM_FMT_SEL_YUV420_3P    1
#define CAM_FMT_SEL_YUV422_1P    2
#define CAM_FMT_SEL_YUV422_2P    3
#define CAM_FMT_SEL_YUV422_3P    4
#define CAM_FMT_SEL_Y_ONLY       12

#define CAM_FMT_SEL_RGB565       0
#define CAM_FMT_SEL_RGB888       1
#define CAM_FMT_SEL_XRGB8888     2
#define CAM_FMT_SEL_RGB101010    3
#define CAM_FMT_SEL_RGB121212    4
#define CAM_FMT_SEL_BAYER8       0
#define CAM_FMT_SEL_BAYER10      1
#define CAM_FMT_SEL_BAYER12      2
#define CAM_FMT_SEL_BAYER14      3

//->//
#define CAM_FMT_SEL_TG_SW_UYVY      0
#define CAM_FMT_SEL_TG_SW_YUYV      1
#define CAM_FMT_SEL_TG_SW_VYUY      2
#define CAM_FMT_SEL_TG_SW_YVYU      3
#define CAM_FMT_SEL_TG_SW_RGB       0
#define CAM_FMT_SEL_TG_SW_BGR       2
//camsv_fmt
#define CAMSV_FMT_SEL_IMGO_OTHER    0
#define CAMSV_FMT_SEL_IMGO_YUV      1
#define CAMSV_FMT_SEL_IMGO_JPG      2

#define CAMSV_FMT_SEL_TG_SW_UYVY    0
#define CAMSV_FMT_SEL_TG_SW_YUYV    1
#define CAMSV_FMT_SEL_TG_SW_VYUY    2
#define CAMSV_FMT_SEL_TG_SW_YVYU    3
#define CAMSV_FMT_SEL_TG_SW_RGB     0
#define CAMSV_FMT_SEL_TG_SW_BGR     2

#define CAMSV_FMT_SEL_TG_FMT_RAW8   0
#define CAMSV_FMT_SEL_TG_FMT_RAW10  1
#define CAMSV_FMT_SEL_TG_FMT_RAW12  2
#define CAMSV_FMT_SEL_TG_FMT_YUV422 3
#define CAMSV_FMT_SEL_TG_FMT_RAW14  4
#define CAMSV_FMT_SEL_TG_FMT_JPEG   7

#define CAMSV_PAK_MODE_FMT_RAW14 0
#define CAMSV_PAK_MODE_FMT_RAW12 1
#define CAMSV_PAK_MODE_FMT_RAW10 2
#define CAMSV_PAK_MODE_FMT_RAW8  3


//img3o_fmt
#define CAM_IMG3O_FMT_YUV420_3P    0
#define CAM_IMG3O_FMT_YUV420_2P    1
#define CAM_IMG3O_FMT_YUV422_1P    2
//vipi_fmt
#define CAM_VIPI_FMT_YUV420_3P                  0
#define CAM_VIPI_FMT_YUV420_2P                  1
#define CAM_VIPI_FMT_YUV422_1P                  2
#define CAM_VIPI_FMT_MFB_LOWLIGHT_POSTBLENDING  3
#define CAM_VIPI_FMT_MFB_MULTIMOTION            4
//ufdi fmt
#define CAM_UFDI_FMT_UFO_LENGTH     0
#define CAM_UFDI_FMT_WEIGHTING      1
//pixel id
#define CAM_PIX_ID_B    0
#define CAM_PIX_ID_Gb   1
#define CAM_PIX_ID_Gr   2
#define CAM_PIX_ID_R    3
//fg_mode
#define CAM_FG_MODE_DISABLE  0
#define CAM_FG_MODE_ENABLE   1


/*/////////////////////////////////////////////////////////////////////////////
    TODO: temp from tg_common
  /////////////////////////////////////////////////////////////////////////////*/
//
#define CAM_ISP_SETTING_DONT_CARE 0
//CDP
#define CAM_CRZ_6_TAP  0
#define CAM_CRZ_N_TAP  1
#define CAM_CRZ_4N_TAP 2
//
#define CAM_MODE_FRAME  0
#define CAM_MODE_TPIPE  1
//
#define _FMT_YUV420_2P_ 0
#define _FMT_YUV420_3P_ 1
#define _FMT_YUV422_1P_ 2
#define _FMT_YUV422_2P_ 3
#define _FMT_YUV422_3P_ 4

#define _FMT_SEQ_422_UYVY_  0
#define _FMT_SEQ_422_VYUY_  1
#define _FMT_SEQ_422_YUYV_  2
#define _FMT_SEQ_422_YVYU_  3
#define _FMT_SEQ_420_UV_    0
#define _FMT_SEQ_420_VU_    1

//
#define CAM_ISP_CQ_NONE (0xFFFF)
#define CAM_ISP_CQ0     ISP_DRV_CQ0
#define CAM_ISP_CQ0B    ISP_DRV_CQ0B
#define CAM_ISP_CQ0C    ISP_DRV_CQ0C
#define CAM_ISP_CQ0_D   ISP_DRV_CQ0_D
#define CAM_ISP_CQ0B_D  ISP_DRV_CQ0B_D
#define CAM_ISP_CQ0C_D  ISP_DRV_CQ0C_D
#define CAM_ISP_CQ1     ISP_DRV_CQ01
#define CAM_ISP_CQ2     ISP_DRV_CQ02
#define CAM_ISP_CQ3     ISP_DRV_CQ03

#define ISP_P1_CQ(cq) (CAM_ISP_CQ0 == cq || CAM_ISP_CQ0B == cq || CAM_ISP_CQ0C == cq || CAM_ISP_CQ0_D == cq || CAM_ISP_CQ0B_D == cq || CAM_ISP_CQ0C_D == cq)

//
#define CAM_CQ_SINGLE_IMMEDIATE_TRIGGER CQ_SINGLE_IMMEDIATE_TRIGGER
#define CAM_CQ_SINGLE_EVENT_TRIGGER     CQ_SINGLE_EVENT_TRIGGER
#define CAM_CQ_CONTINUOUS_EVENT_TRIGGER CQ_CONTINUOUS_EVENT_TRIGGER
//
#define CAM_CQ_TRIG_BY_START        CQ_TRIG_BY_START
#define CAM_CQ_TRIG_BY_PASS1_DONE   CQ_TRIG_BY_PASS1_DONE
#define CAM_CQ_TRIG_BY_PASS2_DONE   CQ_TRIG_BY_PASS2_DONE
#define CAM_CQ_TRIG_BY_IMGO_DONE    CQ_TRIG_BY_IMGO_DONE
#define CAM_CQ_TRIG_BY_RRZO_DONE    CQ_TRIG_BY_RRZO_DONE
#define CAM_CQ_TRIG_BY_NONE         (-1)

//
#define CAM_ISP_PIXEL_BYTE_FP               (3)
#define CAM_ISP_PIXEL_BYTE_MAX_TRUNCATION   ((1<<3)-1)
//isp line buffer
#define CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL (2304)
//
#define CAM_CDP_PRZ_CONN_TO_DISPO 0
#define CAM_CDP_PRZ_CONN_TO_VIDO  1
//interrupt timeout time
#define CAM_INT_WAIT_TIMEOUT_MS 2000
#define CAM_INT_PASS2_WAIT_TIMEOUT_MS 3000

enum EIspUpdateTgtDev
{
    eIspUpdateTgtDev_Main  = 1,
    eIspUpdateTgtDev_Sub   = 2,
    eIspUpdateTgtDev_Main2 = 3,
};

enum EIspRetStatus
{
    eIspRetStatus_VSS_NotReady  = 1,
    eIspRetStatus_Success       = 0,
    eIspRetStatus_Failed        = -1,
};

// TPIPE
class TdriBnrCfg
{
public:
    int bpc_en;
    int bpc_tbl_en;
};

class TdriLscCfg
{
public:
    int sdblk_width;
    int sdblk_xnum;
    int sdblk_last_width;
    int sdblk_height;
    int sdblk_ynum;
    int sdblk_last_height;
};

class TdriLsciCfg
{
public:
    int lsci_stride;
};

class TdriSl2Cfg
{
public:
    int sl2_hrz_comp;
};
class TdriLceiCfg
{
public:
    int lcei_stride;
};
class TdriLceCfg
{
public:
    int lce_bc_mag_kubnx;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_slm_height;
};
class TdriNbcCfg
{
public:
    int anr_eny;
    int anr_enc;
    int anr_iir_mode;
    int anr_scale_mode;
};
class TdriSeeeCfg
{
public:
    int se_edge;
    int usm_over_shrink_en;
};
class TdriImg3oCfg
{
public:
    int img3o_stride;
    int img3o_xoffset;
    int img3o_yoffset;
    int img3o_xsize;
    int img3o_ysize;

};

class TdriImg3boCfg
{
public:
    int img3bo_stride;
    int img3bo_xsize;
    int img3bo_ysize;
};

class TdriImg3coCfg
{
public:
    int img3co_stride;
    int img3co_xsize;
    int img3co_ysize;
};

class TdriEsfkoCfg
{
public:
    int esfko_stride;
};
class TdriAaoCfg
{
public:
    int aao_stride;
};
class TdriLcsoCfg
{
public:
    int lcso_stride;
    int lcso_crop_en;
};

class TdriCdrzCfg
{
public:
    int cdrz_input_crop_width;
    int cdrz_input_crop_height;
    int cdrz_output_width;
    int cdrz_output_height;
    int cdrz_luma_horizontal_integer_offset;/* pixel base */
    int cdrz_luma_horizontal_subpixel_offset;/* 20 bits base */
    int cdrz_luma_vertical_integer_offset;/* pixel base */
    int cdrz_luma_vertical_subpixel_offset;/* 20 bits base */
    int cdrz_horizontal_luma_algorithm;
    int cdrz_vertical_luma_algorithm;
    int cdrz_horizontal_coeff_step;
    int cdrz_vertical_coeff_step;
};
class TdriCurzCfg
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
class TdriFeCfg
{
public:
    int fem_harris_tpipe_mode;
};

class TdriFeoCfg
{
public:
    int feo_stride;
};

class TdriNr3dCfg
{
public:
    int nr3d_on_en;
    int nr3d_on_xoffset;
    int nr3d_on_yoffset;
    int nr3d_on_width;
    int nr3d_on_height;
};

class TdriCrspCfg
{
public:
    int crsp_ystep;
    int crsp_xoffset;
    int crsp_yoffset;

};

class TdriImg2oCfg
{
public:
    int img2o_stride;
    int img2o_xoffset;
    int img2o_yoffset;
    int img2o_xsize;
    int img2o_ysize;

};

class TdriSrzCfg
{
public:
    int srz_input_crop_width;
    int srz_input_crop_height;
    int srz_output_width;
    int srz_output_height;
    int srz_luma_horizontal_integer_offset;/* pixel base */
    int srz_luma_horizontal_subpixel_offset;/* 20 bits base */
    int srz_luma_vertical_integer_offset;/* pixel base */
    int srz_luma_vertical_subpixel_offset;/* 20 bits base */
    int srz_horizontal_coeff_step;
    int srz_vertical_coeff_step;
};


class TdriPrzCfg
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

class TdriMfbCfg
{
public:
    int bld_mode;
    int bld_deblock_en;
    int bld_brz_en;
};

class TdriMfboCfg
{
public:
    int mfbo_stride;
    int mfbo_xoffset;
    int mfbo_yoffset;
    int mfbo_xsize;
    int mfbo_ysize;
};


class TdriFlkiCfg
{
public:
    int flki_stride;
};
class TdriCfaCfg
{
public:
    int bayer_bypass;
    int dm_fg_mode;
};


class TdriG2cCfg
{
public:
    int g2c_shade_en;
};

class TdriSl2bCfg
{
public:
    int sl2b_hrz_comp;
};

class TdriSl2cCfg
{
public:
    int sl2c_hrz_comp;
};

class TdriTopCfg
{
public:
    int scenario;
    int mode;
    int pixel_id;
    int cam_in_fmt;
    int ctl_extension_en;
    int fg_mode;
    int ufdi_fmt;
    int vipi_fmt;
    int img3o_fmt;
    int imgi_en;
    int ufdi_en;
    int unp_en;
    int ufd_en;
    int ufd_crop_en;
    int bnr_en;
    int lsci_en;
    int lsc_en;
    int c24_en;
    int sl2_en;
    int cfa_en;
    int vipi_en;
    int vip2i_en;
    int vip3i_en;
    int mfb_en;
    int mfbo_en;
    int g2c_en;
    int c42_en;
    int sl2b_en;
    int nbc_en;
    int srz1_en;
    int mix1_en;
    int srz2_en;
    int mix2_en;
    int sl2c_en;
    int seee_en;
    int lcei_en;
    int lce_en;
    int mix3_en;
    int crz_en;
    int img2o_en;
    int fe_en;
    int feo_en;
    int c02_en;
    int nr3d_en;
    int crsp_en;
    int img3o_en;
    int img3bo_en;
    int img3co_en;
    int c24b_en;
    int mdp_crop_en;
    int interlace_mode;
    int ufd_sel;
    int ccl_sel;
    int ccl_sel_en;
    int g2g_sel;
    int g2g_sel_en;
    int c24_sel;
    int srz1_sel;
    int mix1_sel;
    int crz_sel;
    int nr3d_sel;
    int fe_sel;
    int mdp_sel;
    int pca_sel;
    int imgi_v_flip_en;
    int lcei_v_flip_en;
    int ufdi_v_flip_en;
};



class TdriPerformCfg
{
public:
    int tpipeWidth;
    int tpipeHeight;
    int irqMode;
};

class TdriDMACfg
{
public:
    int srcWidth;
    int srcHeight;
    int tpipeTabSize;
    int baseVa;
    //
    int setSimpleConfIdxNumVa;
    int segSimpleConfBufVa;
};

class TdriImgiCfg
{
public:
    int imgi_stride;
};


class TdriVipiCfg
{
public:
    int vipi_xsize;
    int vipi_ysize;
    int vipi_stride;
};

class TdriUfdiCfg
{
public:
    int ufdi_stride;
};


class TdriVip2iCfg
{
public:
    int vip2i_xsize;
    int vip2i_ysize;
    int vip2i_stride;
};

class TdriVip3iCfg
{
public:
    int vip3i_xsize;
    int vip3i_ysize;
    int vip3i_stride;
};


class TdriTuningCfg
{
public:
    TdriBnrCfg bnr;
    TdriLscCfg lsc;
    TdriLsciCfg lsci;
    TdriSl2Cfg sl2;
    TdriLceCfg lce;
    TdriLceiCfg lcei;
    TdriNbcCfg nbc;
    TdriSeeeCfg seee;
    TdriCfaCfg cfa;
//    TdriUfdiCfg ufdi;
    TdriG2cCfg g2c;
    TdriSl2bCfg sl2b;
    TdriSl2cCfg sl2c;
};

class TdriDrvCfg
{
public:
    //enable table
    TdriTopCfg top;
    TdriDMACfg tdriCfg;
    TdriPerformCfg  tdriPerformCfg;
    //
    TdriImgiCfg imgi;
    TdriUfdiCfg ufdi;
    TdriVipiCfg vipi;
    TdriVip2iCfg vip2i;
    TdriVip3iCfg vip3i;
    TdriMfbCfg mfb;
    TdriMfboCfg mfbo;
    TdriCdrzCfg crz;
    TdriImg2oCfg img2o;
    TdriSrzCfg srz1;
    TdriSrzCfg srz2;
    TdriFeCfg fe;
    TdriFeoCfg feo;
    TdriNr3dCfg nr3d;
    TdriCrspCfg crsp;
    TdriImg3oCfg img3o;
    TdriImg3boCfg img3bo;
    TdriImg3coCfg img3co;
    //
    TdriTuningCfg tuningFunc;
};
 class CrspCfg
 {
 public:
     IspSize out;
 };
 class C02Cfg
 {
 public:
     IspSize in;
 };



 class MfbCfg
 {
 public:
     MUINT32 bld_mode;
     IspSize mfb_out;        //actually the same with mfbo
     MUINT32 out_xofst;
     MUINT32 bld_ll_db_en;   //set default from imageio, would update via tuning
     MUINT32 bld_ll_brz_en;  //set default from imageio, would update via tuning
 };

 class SrzSizeCfg
 {
  public:
      MUINT32 in_w;
      MUINT32 in_h;
      MUINT32 out_w;
      MUINT32 out_h;

     SrzSizeCfg()
         : in_w(0x0)
         , in_h(0x0)
         , out_w(0x0)
         , out_h(0x0)
     {
     }
 };

 class SrzCfg
 {
 public:
     SrzSizeCfg inout_size;
     IspRect crop;
     MUINT32 h_step;
     MUINT32 v_step;
 };

 class Nr3DCfg
 {
 public:
     MUINT32 fbcntOff_xoff;
     MUINT32 fbcntOff_yoff;
     MUINT32 fbcntSiz_xsiz;
     MUINT32 fbcntSiz_ysiz;
     MUINT32 fbCount_cnt;
     MUINT32 ctrl_lmtOutCntTh;
     MUINT32 ctrl_onEn;
     MUINT32 onOff_onOfStX;
     MUINT32 onOff_onOfStY;
     MUINT32 onSiz_onWd;
     MUINT32 onSiz_onHt;
     MUINT32 gain_weighting; // divided by 256
     MUINT32 vipi_offst;            //in byte
     MUINT32 vipi_readW;        //in pixel
     MUINT32 vipi_readH;        //in pixel

    Nr3DCfg()
        : fbcntOff_xoff(0x0)
        , fbcntOff_yoff(0x0)
        , fbcntSiz_xsiz(0x0)
        , fbcntSiz_ysiz(0x0)
        , fbCount_cnt(0x0)
        , ctrl_lmtOutCntTh(0x0)
        , ctrl_onEn(0x0)
        , onOff_onOfStX(0x0)
        , onOff_onOfStY(0x0)
        , onSiz_onWd(0x0)
        , onSiz_onHt(0x0)
        , gain_weighting(0x100)
        , vipi_offst(0x0)
        , vipi_readW(0x0)
        , vipi_readH(0x0)
    {
    }
 };

 struct StaDataCfg
 {
     IMEM_BUF_INFO   bufInfo;
     MINT32          w;
     MINT32          h;
     MUINT32         stride;
     MUINT32         port_idx;
     MUINT32         port_type;
     MUINT32         port_inout;

     StaDataCfg(
         MINT32     _w = 0,
         MINT32     _h = 0,
         MUINT32    _stride = 0)
     : bufInfo(IMEM_BUF_INFO())
     , w(_w)
     , h(_h)
     , stride(_stride)
     , port_idx(0)
     , port_type(0)
     , port_inout(0)
     {}
 };



 class IspImageioPackage
 {
 public:
     MUINT32 cq;
     MINT32 subMode;
     MBOOL isApplyTuning;
     MUINT32 enFgMode;
     MfbCfg mfbCfg;
     Nr3DCfg nr3dCfg;
     MUINT32 finalEnP2;
 };


//// for tuning function
 class IspTopEnTable
 {
 public:
    MUINT32 cam_ctl_en_p1;
    MUINT32 cam_ctl_en_p1_dma;
    MUINT32 cam_ctl_en_p1_d;
    MUINT32 cam_ctl_en_p1_dma_d;
    MUINT32 cam_ctl_en_p2;
    MUINT32 cam_ctl_en_p2_dma;
 };

 typedef enum
 {
     eIspModuleNull= 0x00000000,
     eIspModuleRaw = 0x00000001,
     eIspModuleRgb = 0x00000002,
     eIspModuleYuv = 0x00000004,
 }EIspModule;

 class IspTuningPackage{
 public:
    MUINT32* pTuningBuf;
    CAM_MODULE_ENUM eTuningCqFunc1;
    CAM_MODULE_ENUM eTuningCqFunc2;
    CAM_MODULE_ENUM eTuningCqDma;
    // for sl2/sl2b/sl2c
    MFLOAT rrzRatio;
    MUINT32 rrzCropX;
    MUINT32 rrzCropY;
    MFLOAT sl2cCrzRatio;
    MUINT32 lceInputW;
    MUINT32 lceInputH;
    MUINT32 nr3dGainWeighting;
 };
 class IspTopModuleMappingTable{
 public:
    MUINT32 ctrlByte;  // isp register byte
    MUINT32 dmaCtrlByte; // only for tuning DMA
    EIspModule eModule;  // module
    EIspTuningMgrFunc eTuningFunc1; // mapping to EIspTuningMgrFunc
    EIspTuningMgrFunc eTuningFunc2; // mapping to EIspTuningMgrFunc
    CAM_MODULE_ENUM eTuningCqFunc1;  // cq function case1
    CAM_MODULE_ENUM eTuningCqFunc2;  // cq function case2
    CAM_MODULE_ENUM eTuningCqDma;  // tuning cq dma
    MINT32 (*default_func)(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage); // run individual function
 };

enum EIspControlBufferCmd
{
    eIspControlBufferCmd_Alloc  = 0,
    eIspControlBufferCmd_Free,
    eIspControlBufferCmd_Num
};


//
/**/
class IspFunction_B;
class IspEventThread;
/*/////////////////////////////////////////////////////////////////////////////
    Isp driver object
/////////////////////////////////////////////////////////////////////////////*/
class IspDrv_B
{    public:
        IspDrv_B():
            m_pPhyIspDrv_bak(NULL),
            m_pP2CrzDrv(NULL),
            m_pVirtIspDrv(NULL),
            m_pVirtIspReg(NULL)
            {}
        virtual ~IspDrv_B(){}
    public://
        static IspDrv*      m_pPhyIspDrv;
        static IspDrv*      m_pP2IspDrv;
        static IspDrv*      m_pP1IspDrv;
        static IspDrv*      m_pP1IspDrvD;

    public://
        /*** virtual isp driver for CQ ***/
        IspDrv**         m_pVirtIspDrv;
        isp_reg_t**      m_pVirtIspReg;
        //static isp_reg_t*   m_pCurCQIspReg;
        static isp_reg_t*   m_pP1CurCQDIspReg;

        IspDrv*         m_pPhyIspDrv_bak;
        /*** crz driver ***/
        CrzDrv*         m_pP2CrzDrv;

        /*imem*/
        IMemDrv*        m_pIMemDrv ;
        /*twin*/
        TwinDrv*        m_pTwinDrv ;
        /*ispEventThread*/
        IspEventThread* m_pIspEventThread;
    public://
        static MINT32 cam_isp_pgn_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_sl2_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_ccl_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_g2c_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_nbc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_mfb_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_pca_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_seee_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_lce_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_nr3d_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_sl2b_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_sl2c_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_cfa_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_g2g_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_ggm_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_c24_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_c42_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_ob_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_bnr_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_lsc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_rpg_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_rmg_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_mix3_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        //
        static MINT32 cam_isp_c02_cfg(MUINT32 srcImgW,MUINT32 srcImgH);
        static MINT32 cam_isp_crsp_cfg(MUINT32 enCrsp, MUINT32 desImgW,MUINT32 desImgH);
        static MINT32 cam_isp_rrz_cfg(IspDrv* pCurDrv,IspSize in,IspRect crop,IspSize out,MUINT32 ctrl,MUINT32 h_step,MUINT32 v_step, MUINT32 isRaw_D);
        static MINT32 cam_isp_mix1_cfg();
        static MINT32 cam_isp_mix2_cfg();
        static MINT32 cam_isp_srz1_cfg(MUINT32 ctrl,SrzSizeCfg inout_size,IspRect crop,MUINT32 h_step,MUINT32 v_step);
        static MINT32 cam_isp_srz2_cfg(MUINT32 ctrl,SrzSizeCfg inout_size,IspRect crop,MUINT32 h_step,MUINT32 v_step);
        static MINT32 cam_isp_obc_cfg(MUINT32 cq, stIspOBCCtl_t obc);
        static MINT32 cam_isp_bnr_bpc_cfg(MUINT32 cq, stIspBPCCtl_t bpc);
        static MINT32 cam_isp_bnr_rmm_cfg(MUINT32 cq, stIspRMMCtl_t rmm);
        static MINT32 cam_isp_bnr_nr1_cfg(MUINT32 cq, stIspNR1Ctl_t nr1);
        static MINT32 cam_isp_bnr_lsc_cfg(MUINT32 cq, stIspLSCCtl_t lsc);
        static MINT32 cam_isp_bnr_rpg_cfg(MUINT32 cq, stIspRPGCtl_t rpg);
        static MINT32 cam_isp_bnr_ae_cfg(MUINT32 cq, stIspAECtl_t ae);
        static MINT32 cam_isp_bnr_awb_cfg(MUINT32 cq, stIspAWBCtl_t awb);
        static MINT32 cam_isp_bnr_sgg1_cfg(MUINT32 cq, stIspSGG1Ctl_t sgg1);
        static MINT32 cam_isp_bnr_flk_cfg(MUINT32 cq, stIspFLKCtl_t flk);
        static MINT32 cam_isp_bnr_af_cfg(MUINT32 cq, stIspAFCtl_t af);
        static MINT32 cam_isp_bnr_sgg2_cfg(MUINT32 cq, stIspSGG2Ctl_t sgg2);
        static MINT32 cam_isp_bnr_eis_cfg(MUINT32 cq, stIspEISCtl_t eis);
        static MINT32 cam_isp_bnr_lcs_cfg(MUINT32 cq, stIspLCSCtl_t lcs);


};


/*/////////////////////////////////////////////////////////////////////////////
    Tuning Function Control
/////////////////////////////////////////////////////////////////////////////*/
#define CHECK_TUNING_P1_EN_BITS       5
#define CHECK_TUNING_P2_EN_BITS       19


#define TUNING_SCENARIO_IP_EN_P1_TAG    0x000000F0


 const IspTopEnTable gIspTurningTopTag_cc[ISP_SUB_MODE_MAX]  //CFA,G2G,G2C,MFB,NR3D top controled by imageio path, field set by featureio path
 ={  {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00037856, 0x00000000 },  // ISP_SUB_MODE_RAW
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00037A76, 0x00000000 },  // ISP_SUB_MODE_YUV
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00037856, 0x00000000 },  // ISP_SUB_MODE_RGB
     {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },  // ISP_SUB_MODE_JPG
 };

 const IspTopEnTable gIspTurningFieldTag_cc[ISP_SUB_MODE_MAX]
 ={  {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003FAFE, 0x00000000 },  // ISP_SUB_MODE_RAW
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003FAFE, 0x00000000 },  // ISP_SUB_MODE_YUV
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003FAFE, 0x00000000 },  // ISP_SUB_MODE_RGB
     {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },  // ISP_SUB_MODE_JPG
 };

 const IspTopEnTable gIspTurningTopTag_fb[ISP_SUB_MODE_MAX]  //CFA,G2G,G2C,MFB,NR3D, LCE & LCEI be controled by imagaio path
 ={  {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00033856, 0x00000000 },  // ISP_SUB_MODE_RAW
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00033A76, 0x00000000 },  // ISP_SUB_MODE_YUV
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00033856, 0x00000000 },  // ISP_SUB_MODE_RGB
     {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },  // ISP_SUB_MODE_JPG
 };

 const IspTopEnTable gIspTurningFieldTag_fb[ISP_SUB_MODE_MAX] //LCE & LCEI be controled by imagaio path
 ={  {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003BAFE, 0x00000000 },  // ISP_SUB_MODE_RAW
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003BAFE, 0x00000000 },  // ISP_SUB_MODE_YUV
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003BAFE, 0x00000000 },  // ISP_SUB_MODE_RGB
     {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },  // ISP_SUB_MODE_JPG
 };

 const IspTopEnTable gIspTurningTopTag_ip[ISP_SUB_MODE_MAX]  //CFA,G2G,G2C,MFB,NR3D top controled by imageio path, field set by featureio path
 ={  {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00037856, 0x00000020 },  // ISP_SUB_MODE_RAW
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00037876, 0x00000020 },  // ISP_SUB_MODE_YUV
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x00037856, 0x00000020 },  // ISP_SUB_MODE_RGB
     {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },  // ISP_SUB_MODE_JPG
 };

 const IspTopEnTable gIspTurningFieldTag_ip[ISP_SUB_MODE_MAX]
 ={  {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003FAFE, 0x00000020 },  // ISP_SUB_MODE_RAW
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003FAFE, 0x00000020 },  // ISP_SUB_MODE_YUV
     {0x000000F0, 0x000000C0, 0x00000070, 0x000000C0, 0x0003FAFE, 0x00000020 },  // ISP_SUB_MODE_RGB
     {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },  // ISP_SUB_MODE_JPG
 };


 const IspTopModuleMappingTable gIspModuleMappingTableP1En[CHECK_TUNING_P1_EN_BITS]
 ={ {CAM_CTL_EN_P1_OB_EN , CAM_CTL_EN_P1_DMA_NONE_EN, eIspModuleRaw, eIspTuningMgrFunc_Obc, eIspTuningMgrFunc_Null, CAM_ISP_OBC,  CAM_DUMMY_, CAM_DUMMY_,   IspDrv_B::cam_isp_ob_cfg},
    {CAM_CTL_EN_P1_BNR_EN, CAM_CTL_EN_P1_DMA_BPCI_EN, eIspModuleRaw, eIspTuningMgrFunc_Bnr, eIspTuningMgrFunc_Null, CAM_ISP_BNR,  CAM_DUMMY_, CAM_DMA_BPCI, IspDrv_B::cam_isp_bnr_cfg},
    {CAM_CTL_EN_P1_LSC_EN, CAM_CTL_EN_P1_DMA_LSCI_EN, eIspModuleRaw, eIspTuningMgrFunc_Lsc, eIspTuningMgrFunc_Null, CAM_ISP_LSC,  CAM_DUMMY_, CAM_DMA_LSCI, IspDrv_B::cam_isp_lsc_cfg},
    {CAM_CTL_EN_P1_RPG_EN, CAM_CTL_EN_P1_DMA_NONE_EN, eIspModuleRaw, eIspTuningMgrFunc_Rpg, eIspTuningMgrFunc_Null, CAM_ISP_RPG,  CAM_DUMMY_, CAM_DUMMY_,   IspDrv_B::cam_isp_rpg_cfg},
    {CAM_CTL_EN_P1_RMG_EN, CAM_CTL_EN_P1_DMA_NONE_EN, eIspModuleRaw, eIspTuningMgrFunc_Rmg, eIspTuningMgrFunc_Null, CAM_CTL_IHDR, CAM_DUMMY_, CAM_DUMMY_,   IspDrv_B::cam_isp_rmg_cfg},
  };


 const IspTopModuleMappingTable gIspModuleMappingTableP2En[CHECK_TUNING_P2_EN_BITS]
 ={ {CAM_CTL_EN_P2_UFD_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRaw,  eIspTuningMgrFunc_Null,          eIspTuningMgrFunc_Null,     CAM_ISP_UFD,          CAM_DUMMY_,        CAM_DUMMY_,   NULL},
    {CAM_CTL_EN_P2_PGN_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRaw,  eIspTuningMgrFunc_Pgn,           eIspTuningMgrFunc_Null,     CAM_ISP_PGN,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_pgn_cfg},
    {CAM_CTL_EN_P2_SL2_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRgb,  eIspTuningMgrFunc_Sl2,           eIspTuningMgrFunc_Null,     CAM_ISP_SL2,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_sl2_cfg},
    {CAM_CTL_EN_P2_CFA_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRgb,  eIspTuningMgrFunc_Cfa,           eIspTuningMgrFunc_Null,     CAM_ISP_CFA,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_cfa_cfg},
    {CAM_CTL_EN_P2_CCL_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRgb,  eIspTuningMgrFunc_Ccl,           eIspTuningMgrFunc_Null,     CAM_ISP_CCL,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_ccl_cfg},
    {CAM_CTL_EN_P2_G2G_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRgb,  eIspTuningMgrFunc_G2g,           eIspTuningMgrFunc_Null,     CAM_ISP_G2G,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_g2g_cfg},
    {CAM_CTL_EN_P2_GGM_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRgb,  eIspTuningMgrFunc_Ggm_Rb,        eIspTuningMgrFunc_Ggm_G,    CAM_ISP_GGM_RB,       CAM_ISP_GGM_G,     CAM_DUMMY_,   IspDrv_B::cam_isp_ggm_cfg},
    {CAM_CTL_EN_P2_MFB_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRgb,  eIspTuningMgrFunc_Mfb_TuneCon,   eIspTuningMgrFunc_Null,     CAM_ISP_MFB_TUNECON,  CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_mfb_cfg},
    {CAM_CTL_EN_P2_C24_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleRgb,  eIspTuningMgrFunc_Null,          eIspTuningMgrFunc_Null,     CAM_DUMMY_ ,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_c24_cfg},
    {CAM_CTL_EN_P2_G2C_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_G2c_Conv,      eIspTuningMgrFunc_G2c_Shade,CAM_ISP_G2C_CONV,     CAM_ISP_G2C_SHADE, CAM_DUMMY_,   IspDrv_B::cam_isp_g2c_cfg},
    {CAM_CTL_EN_P2_C42_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Null,          eIspTuningMgrFunc_Null,     CAM_DUMMY_ ,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_c42_cfg},
    {CAM_CTL_EN_P2_NBC_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Nbc,           eIspTuningMgrFunc_Null,     CAM_ISP_NBC,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_nbc_cfg},
    {CAM_CTL_EN_P2_PCA_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Pca_Tbl,       eIspTuningMgrFunc_Pca_Con,  CAM_ISP_PCA_TBL,      CAM_ISP_PCA_CON,   CAM_DUMMY_,   IspDrv_B::cam_isp_pca_cfg},
    {CAM_CTL_EN_P2_SEEE_EN, CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Seee,          eIspTuningMgrFunc_Null,     CAM_ISP_SEEE,         CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_seee_cfg},
    {CAM_CTL_EN_P2_LCE_EN , CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Lce,           eIspTuningMgrFunc_Null,     CAM_ISP_LCE,          CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_lce_cfg},
    {CAM_CTL_EN_P2_NR3D_EN, CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Nr3d_Blending, eIspTuningMgrFunc_Nr3d_Lmt,      CAM_CDP_NR3D_BLENDING,CAM_CDP_NR3D_LMT,  CAM_DUMMY_,   IspDrv_B::cam_isp_nr3d_cfg},
    {CAM_CTL_EN_P2_SL2B_EN, CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Sl2b,          eIspTuningMgrFunc_Null,     CAM_ISP_SL2B,         CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_sl2b_cfg},
    {CAM_CTL_EN_P2_SL2C_EN, CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Sl2c,          eIspTuningMgrFunc_Null,     CAM_ISP_SL2C,         CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_sl2c_cfg},
    {CAM_CTL_EN_P2_MIX3_EN, CAM_CTL_EN_P2_DMA_NONE_EN, eIspModuleYuv,  eIspTuningMgrFunc_Mix3,          eIspTuningMgrFunc_Null,     CAM_CDP_MIX3,         CAM_DUMMY_,        CAM_DUMMY_,   IspDrv_B::cam_isp_mix3_cfg},
 };

/*/////////////////////////////////////////////////////////////////////////////
    CommandQ control
/////////////////////////////////////////////////////////////////////////////*/
#if 0
    //pass2 fixed
    // bit
    #define CAM_CTL_EN1_C24_BIT               0x20000000  //0x4004
    #define CAM_CTL_EN1_MFB_BIT               0x10000000  //0x4004
    #define CAM_CTL_EN1_C02_BIT               0x08000000  //0x4004
    #define CAM_CTL_EN1_CFA_BIT               0x00200000  //0x4004
    #define CAM_CTL_EN1_HRZ_BIT               0x00000200  //0x4004
    #define CAM_CTL_EN1_AAA_GROP_BIT          0x00000040  //0x4004
    #define CAM_CTL_EN2_CDRZ_BIT              0x00020000  //0x4008
    #define CAM_CTL_EN2_G2C_BIT               0x00000001  //0x4008
    #define CAM_CTL_EN2_C42_BIT               0x00000002  //0x4008
    #define CAM_CTL_EN2_NR3D_BIT              0x00000020  //0x4008
    #define CAM_CTL_DMA_IMG2O_BIT             0x00000400  //0x400c
    #define CAM_CTL_DMA_IMGO_BIT              0x00000001  //0x400c
    #define CAM_CTL_DMA_AAO_BIT               0x00000020  //0x400c
    #define CAM_CTL_DMA_ESFKO_BIT             0x00000008  //0x400c
    #define CAM_CTL_DMA_FLKI_BIT              0x00000800  //0x400c
    #define CAM_CTL_DMA_LCSO_BIT              0x00000040  //0x400c

    #define CAM_CTL_FMT_SCENARIO_BIT          0x00000007  //0x4010
    #define CAM_CTL_FMT_SUB_MODE_BIT          0x00000038  //0x4010
    // register
    #define CAM_CTL_FIXED_DMA_EN_SET_PASS2    0x00000401  //0x400C /* IMGO,IMG2O */
    #define CAM_CTL_FIXED_FMT_SEL_SET_PASS2   0x0000007F  //0x4018 /* SCENARIO, SUB_MODE */
    //
    //
    #define CAM_CTL_EN1_FOR_ISP     0x38303200      //0x4004
    #define CAM_CTL_EN2_FOR_ISP     0x00020023      //0x4008  // NR3D control by isp
    #define CAM_CTL_DMA_EN_FOR_ISP  0x00006DE9      //0x400C
    #define CAM_CTL_FMT_SEL_FOR_ISP 0x0000FF77      //0x4010
    //#define CAM_CTL_SEL_FOR_ISP     0x00000000      //0x4018
    #define CAM_CTL_MUX_SEL_FOR_ISP         0xFFFFFFFF //0x4074
    #define CAM_CTL_MUX_SEL2_FOR_ISP        0xFFFFFFFF //0x4078
    #define CAM_CTL_SRAM_MUX_CFG_FOR_ISP    0xFFFFFFFF //0x407C
    #define CAM_CTL_PIX_ID_FOR_ISP  0x00000003      //0x401C
    //#define CAM_CTL_INT_EN_FOR_ISP  0x00000000      //0x4020
    //#define CAM_CTL_TPIPE_FOR_ISP    0x00000000      //0x4050
    //#define CAM_CTL_TCM_EN_FOR_ISP  0x00000000      //0x4054
    //
    #define CAM_CTL_EN1_FOR_CDP     0x00000000      //0x4004
#if 1   //MTK_NR3D_SUPPORT
    #define CAM_CTL_EN2_FOR_CDP     0x43A40020      //0x4008  // NR3D control by isp
#else
    #define CAM_CTL_EN2_FOR_CDP     0x43A40000      //0x4008  // NR3D control by isp
#endif

    #define CAM_CTL_DMA_EN_FOR_CDP  0x00286480      //0x400C
    #define CAM_CTL_FMT_SEL_FOR_CDP CAM_CTL_FMT_SEL_FOR_ISP      //0x4010
    //#define CAM_CTL_SEL_FOR_ISP     0x00000000      //0x4018
    #define CAM_CTL_MUX_SEL_FOR_CDP         0x00000000 //0x4074
    #define CAM_CTL_MUX_SEL2_FOR_CDP        0x00000000 //0x4078
    #define CAM_CTL_SRAM_MUX_CFG_FOR_CDP    0x00000000 //0x407C
    #define CAM_CTL_PIX_ID_FOR_CDP  CAM_CTL_PIX_ID_FOR_ISP      //0x401C
    //#define CAM_CTL_INT_EN_FOR_CDP  0x00000000      //0x4020
    //#define CAM_CTL_TPIPE_FOR_CDP    0x00000000      //0x4050
    //#define CAM_CTL_TCM_EN_FOR_CDP  0x00000000      //0x4054
    //set 1
    #define CAM_CTL_SEL_SET_1       0x80000000  //0x4018  bit31=1(set CQ1 related interrupt include all interrupt)
    #define CAM_CTL_SPARE3_SET_1    0x00000080  //0x406c  bit7=1(merge pass2, pass2b and pass2c)

    /* engine can be clear or not */
    #define CQ_CAM_CTL_CAN_BE_CLR_BITS(a,isp)       ((isp)?CAM_CTL_##a##_FOR_ISP : CAM_CTL_##a##_FOR_CDP)

    #define CQ_CAM_CTL_BIT_FIXED(_module_,_bit_,_en_) ((_en_)? (~CAM_CTL_##_module_##_##_bit_##_BIT) : 0xffffffff)

    // check for turning
    #define CQ_CAM_CTL_CHECK_TURNING_BITS(_a_,_en_turn_)       ((_en_turn_)?CAM_CTL_##_a_##_FOR_TURN : 0x00000000)
#endif

/*/////////////////////////////////////////////////////////////////////////////
    Isp driver shell
/////////////////////////////////////////////////////////////////////////////*/
class IspDrvShell:virtual public IspDrv_B
{
    protected:
        IspDrvShell():m_trigSsrc(-1){}
        virtual             ~IspDrvShell() {};
    public:
        static IspDrvShell* createInstance(NSImageio::NSIspio::EDrvScenario eScenarioID = NSImageio::NSIspio::eDrvScenario_CC);
        virtual void        destroyInstance(void) = 0;
        virtual MBOOL       init(const char* userName="") = 0;
        virtual MBOOL       uninit(const char* userName="") = 0;
        virtual IspDrv*     getPhyIspDrv() = 0;
        virtual MBOOL       updateBurstQueueNum(MINT32 burstQNum)=0;
        virtual MBOOL       getNr3dGain(ESoftwareScenario softScenario, MINT32 magicNum, MINT32 &nr3dGain)=0;
        //
        friend class IspFunction_B;
    private://phy<->virt ISP switch
        //virtual MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 dupCqIdx) = 0;
        virtual MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 p1CqD,MINT32 burstQIdx,MINT32 p2DupCqIdx) = 0;

    public://commandQ operation
        virtual IspDrv*      ispDrvGetCQIspDrv(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx)  = 0;
        virtual MBOOL       cam_cq_cfg(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx,MINT32 isP2IspOnly=0) = 0;
        virtual MBOOL       cqAddModule(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId) = 0;
        virtual MBOOL       cqDelModule(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId) = 0;
        virtual int         getCqModuleInfo(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId) =0;

        virtual MBOOL       setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src) = 0;

        virtual void    CQBufferMutexLock( void ) = 0;
        virtual void    CQBufferMutexUnLock( void ) = 0;
        //
        //mutable Mutex   mLock;
        static Mutex   mCQBufferLock;  // for multi-thread, pass1 node do update burst number, but pass2 node do flushCQdescripter
        //mutable Mutex   gLock;  // for multi-thread
        mutable Mutex   gPass1Lock;  // global lock for pass1
        mutable Mutex   gPass1_dLock; //global lock for pass1_d,camsv,camsv2
        mutable Mutex   gPass1TopCtrlLock;  // one sensor do enque and deque, another sensor do stop. this will casue block.

        mutable Mutex   gPass2Lock;  // global lock for pass2
        mutable Mutex   gPass2DequeLock;  // global dequeue lock for pass2
        //mutable Mutex   g_cam_path_Lock;  // for dual isp
        //
        MINT32 m_trigSsrc;

};

/*/////////////////////////////////////////////////////////////////////////////
    IspFunction_B
  /////////////////////////////////////////////////////////////////////////////*/
class IspFunction_B
{
private:
    //isp top camera base address
    static unsigned long    m_Isp_Top_Reg_Base_Addr;   //maybe virtual address
    //function base address
    unsigned long           m_Reg_Base_Addr;          //maybe virtual address
    mutable Mutex           mLock;
protected://isp driver operation
    static IspDrv*          m_pPhyIspDrv;
    static IspDrv*          m_pP1IspDrv;
    static IspDrv*          m_pP2IspDrv;
    //static isp_reg_t*       m_pCurCQIspReg;


     IspDrv*          m_pP1IspDrvD;
     isp_reg_t*       m_pP1CurCQDIspReg;

public://isp driver operation
    int                     bypass;
    static IspDrvShell*     m_pIspDrvShell;
    static MUINT32          m_fps;          //for dma fifo cfg
    //for twin mode
    MUINT32                 m_twin_sdnumX;
    MUINT32                 m_twin_lblkW;
    static MUINT32                 m_bKDBG;
public:
    IspFunction_B(): m_Reg_Base_Addr(0),bypass(0){};
    virtual ~IspFunction_B(){};
public:
    virtual int checkBusy( unsigned long* param ) = 0;
    virtual unsigned long   id( void ) = 0;
    virtual const char*     name_Str( void ) = 0;   //Name string of module
    virtual unsigned long   reg_Base_Addr_Phy( void ) = 0;      //Phy reg base address, usually a fixed value
    virtual unsigned long   reg_Range( void ) = 0;
    virtual int             is_bypass( void ) {return bypass;}

    unsigned long           dec_Id( void );         //Decimal ID number
    static unsigned long    isp_Top_Reg_Base_Addr( void )    {   return m_Isp_Top_Reg_Base_Addr;   }
    static unsigned long    isp_Top_Reg_Base_Addr_Phy( void ) {   return MT6593_ISP_TOP_BASE;  }//Phy reg base address, usually a fixed value
    static unsigned long    isp_Top_Reg_Range( void )        {   return MT6593_ISP_TOP_REG_RANGE;  }
    unsigned long           reg_Base_Addr( void )           {   return m_Reg_Base_Addr;   }
    static void             remap_Top_Reg_Base_Addr( unsigned long new_addr )      {   m_Isp_Top_Reg_Base_Addr = new_addr;}
    void                    remap_Reg_Base_Addr( unsigned long new_addr )          {   m_Reg_Base_Addr = new_addr;         }
    MBOOL                   getP2EnTuningTag(MBOOL isTopEngine, ISP_DRV_CQ_ENUM cq, MUINT32 magicNum, MUINT32 drvScenario, MUINT32 subMode, MUINT32 &tuningEnTag, MUINT32 &tuningDmaTag);
    MBOOL                   getP1EnTuningTag(MBOOL isTopEngine, ISP_DRV_CQ_ENUM cq, MUINT32 magicNum, MUINT32 drvScenario, MUINT32 subMode, MUINT32 &tuningEnTag, MUINT32 &tuningDmaTag);

public: //isp driver operationr
    static  void            setIspDrvShell(IspDrvShell* pIspDrvShell){m_pIspDrvShell = pIspDrvShell;m_pPhyIspDrv = pIspDrvShell->getPhyIspDrv();
                                                                      m_pPhyIspDrv = pIspDrvShell->getPhyIspDrv();}

    inline int              waitIrq( ISP_DRV_WAIT_IRQ_STRUCT* WaitIrq ){return (int)m_pPhyIspDrv->waitIrq( WaitIrq );}
    inline int              writeReg( unsigned long offset, unsigned long value ){return (int)m_pPhyIspDrv->writeReg(offset,value);}
    inline unsigned long    readReg( unsigned long offset ){return (unsigned long)m_pPhyIspDrv->readReg(offset);}
    inline int              readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq){return (int)m_pPhyIspDrv->readIrq(pReadIrq);}
    inline int              checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){return (int)m_pPhyIspDrv->checkIrq(CheckIrq);}
    inline int              clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){return (int)m_pPhyIspDrv->clearIrq(ClearIrq);}
    inline int              dumpRegister( int mode ){return (int)m_pPhyIspDrv->dumpReg();}

    mutable Mutex           queHwLock;
    mutable Mutex           queSwLock;
    MBOOL                   getEnTuningTag(MBOOL isTopEngine, ISP_DRV_CQ_ENUM cq, MUINT32 drvScenario, MUINT32 subMode, MUINT32 &tuningEn1Tag, MUINT32 &tuningEn2Tag, MUINT32 &tuningDmaTag);

    /**
      *@brief  Check if virtual isp driver or not
      */
    //MBOOL                 ispDrvSwitch2Virtual(MINT32 cq,MINT32 p2dupCqIdx);
    MBOOL                   ispDrvSwitch2Virtual(MINT32 cq,MINT32 p1CqD,MINT32 burstQIdx,MINT32 p2DupCqIdx);//only used for TopCTrl & RawCtrl
    MUINT32                 GetCropXUnitBussize(MUINT32 input, MUINT32 pixelBit, MUINT32 bussize)
    {
        switch(pixelBit){
            case 10:
            case 12:
                if (1 == bussize)  //16bit
                {
                    return ((input * pixelBit) >> 4);
                }
                else  //the other bit (un-used now)
                {
                    return ((input * pixelBit) / ((bussize+1)<<3));
                }
                break;
            default:
                return input;
        }
    }
    MUINT32                 GetCropXUnitPixel(MUINT32 input, MUINT32 pixelBit, MUINT32 bussize)
    {
        //the pixelbit will be 1 (10bit), 2(12bit), because we do some transform in driver side.
        //use 8+ (2*pixelBit) to become 10bit or 12bit.
        switch(pixelBit){
            case 1:
            case 2:
                return ((input * ((bussize+1)<<3)) / (8+(2*pixelBit)));
                break;
            default:
                return input;
        }
    }


public:
    int             config( void );
    int             enable( void* pParam  );
    int             disable( void );
    int             write2CQ( void );
    int             setZoom( void );
protected:
    virtual int     _config( void ) = 0;
    virtual int     _enable( void* pParam  ) = 0;
    virtual int     _disable( void ) = 0;
    virtual int     _write2CQ( void ) {return 0;}
    virtual int     _setZoom( void ) {return 0;}
};

/*/////////////////////////////////////////////////////////////////////////////
    DMAI
  /////////////////////////////////////////////////////////////////////////////*/
class DMAI_B:public IspFunction_B
{
public:/*[member veriable]*/
    /*
        ->image input(imgi,imgci,nr3i,lsci,flki,lcei,vipi,vip2i)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspMemBuffer.ofst_addr           ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZW
                IspSize.stride                          ->stride
        ->others(TDR,...)
                IspMemBuffer.base_pAddr         ->base_pAddr
                IspMemBuffer.ofst_addr          ->ofst_addr
                IspSize.w                              -> XSIZE
        ->others(CQ,...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspSize.w                              -> XSIZE
        */

    IspDMACfg   dma_cfg;
/*
    IspMemBuffer    img_buf;
    IspSize         size;
    IspRect         crop;
    int             pixel_byte;
*/
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int CQ;
    int CQ_D;
    int cqTrigSrc;
    int cqTrigSrc_D;
    int isP2IspOnly;
    EImageFormat p2MdpSrcFmt;
    unsigned int p2MdpSrcW;
    unsigned int p2MdpSrcH;
    unsigned int p2MdpSrcYStride;
    unsigned int p2MdpSrcUVStride;
    unsigned int p2MdpSrcSize;
    unsigned int p2MdpSrcCSize;
    unsigned int p2MdpSrcVSize;
    unsigned int p2MdpSrcPlaneNum;
    MBOOL       isTwinMode;     //for twin fifo pri thres ctrl
private:

public: /*ctor/dtor*/
    DMAI_B():dupCqIdx(0),burstQueIdx(0),CQ(CAM_ISP_CQ_NONE),CQ_D(CAM_ISP_CQ_NONE),isP2IspOnly(0),p2MdpSrcFmt(eImgFmt_YV12),p2MdpSrcW(0),p2MdpSrcH(0),p2MdpSrcYStride(0),p2MdpSrcUVStride(0),p2MdpSrcSize(0),p2MdpSrcCSize(0),p2MdpSrcVSize(0),cqTrigSrc(-1),cqTrigSrc_D(-1),isTwinMode(0)
        {dma_cfg.pixel_byte = 1;};
public: /*[IspFunction_B]*/
    virtual int checkBusy( unsigned long* param ){return 0;}
private:/*[IspFunction_B]*/
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void );
    virtual int _write2CQ( void );
public: /*[DMAI_B]*/


private: /*[DMAI_B]*/


};

/*/////////////////////////////////////////////////////////////////////////////
    DMAO
  /////////////////////////////////////////////////////////////////////////////*/
class DMAO_B:public IspFunction_B
{
public:/*[member veriable]*/
    /*
        ->image ouput(imgo,img2o,)
                IspMemBuffer.base_pAddr           ->base_pAddr
                IspMemBuffer.ofst_addr            ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZE
                IspSize.stride                          ->stride
                IspRect.x                               ->crop xoffset
                IspRect.y                               ->crop yoffset
        ->others(lcso,ESFKO(FLKO),AAO,NR3O,...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspMemBuffer.ofst_addr            ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZE
                IspSize.stride                          ->stride
        ->others(EISO,AFO...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspSize.w                              -> XSIZE
        */
    MBOOL       isTwinMode;     //for twin fifo pri thres ctrl
    IspDMACfg   dma_cfg;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int CQ;
    int CQ_D;

/*
    IspMemBuffer    img_buf;
    IspSize         size;
    IspRect         crop;
    int             pixel_byte;
*/
private:

public:
    DMAO_B():dupCqIdx(0),burstQueIdx(0),CQ(CAM_ISP_CQ_NONE),CQ_D(CAM_ISP_CQ_NONE)
        {dma_cfg.pixel_byte = 1;};

public: /*[IspFunction_B]*/
    virtual int checkBusy( unsigned long* param ){return 0;}

private:/*[IspFunction_B]*/
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void ){return 0;}
    virtual int _write2CQ( void );

public: /*[DMAO_B]*/


private: /*[DMAO_B]*/


};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_TURNING_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
class ISP_TURNING_CTRL:public IspFunction_B
{
public:
    int CQ;

public:
    ISP_TURNING_CTRL():
        CQ(CAM_ISP_CQ_NONE){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ISP_TURNING;  }
    virtual const char*   name_Str( void )              {   return "ISP_TURNING";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return (unsigned long)NULL;  }
    virtual unsigned long reg_Range( void )             {   return (unsigned long)NULL;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void ){ return 0;}
    virtual int _write2CQ( void ){ return 0;}
    virtual int _setZoom( void ){ return 0;}
};



/*/////////////////////////////////////////////////////////////////////////////
    ISP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
class ISP_TOP_CTRL : public IspFunction_B
{
public:
    //static int pix_id;
    //
    struct stIspTopCtl  isp_top_ctl;
    //
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    int path;
    int pathD;
    unsigned int sub_mode;
    unsigned int sub_mode_D;

    int b_continuous[ISP_MAX_CONCURRENT_TG_NUM];
    int tpipe_w;
    int tpipe_h;
    int CQ;
    int CQ_D;
    int isTwinMode;
    int isP2IspOnly;
    MBOOL isV3;
    MBOOL isApplyTuning;
public:
    ISP_TOP_CTRL():
        magicNum(0),
        dupCqIdx(0),
        burstQueIdx(0),
        drvScenario(0),
        path(ISP_PASS_NONE),
        tpipe_w(0),
        tpipe_h(0),
        CQ(CAM_ISP_CQ_NONE),
        CQ_D(CAM_ISP_CQ_NONE),
        isTwinMode(0),
        isP2IspOnly(0),
        isV3(MFALSE),
        isApplyTuning(MFALSE),
        pathD(ISP_PASS_NONE)
        {
            isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw=0; \
            isp_top_ctl.CAM_CTL_SCENARIO.Raw=0;
            b_continuous[0] = 0;
            b_continuous[1] = 0;
            b_continuous[2] = 0;
            pathD = ISP_PASS_NONE;
         };
    virtual unsigned long id( void )                    {   return ISP_TOP;  }
    virtual const char*   name_Str( void )              {   return "ISP_TOP";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return MT6593_ISP_TOP_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_TOP_REG_RANGE;  }
    //virtual int is_bypass( void )                       {   return 0;       }
    virtual int checkBusy( unsigned long* param );
    virtual MBOOL setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam  );
    virtual int _disable( void );
    virtual int _write2CQ( void );
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_RAW_CENTRAL_CTRL_INFO
  /////////////////////////////////////////////////////////////////////////////*/
class ISP_RAW_CENTRAL_CTRL_INFO : public IspFunction_B
{

public:
   ISP_RAW_CENTRAL_CTRL_INFO():
        mEnablePath(0),
        mTwinMode(MFALSE),
        mpPhyIspDrv(NULL),
        mRawIspDrv(NULL),
        mRawDIspDrv(NULL)
   {
        memset(&mTopCtrl, 0, sizeof(mTopCtrl));
   }

     MBOOL    config2( ISP_TOP_CTRL * pSrcTopCtrl );
     MBOOL    isTwinModeEn(void)             { return mTwinMode; }
     void     updateTwinModeInfo(MBOOL en) { mTwinMode |= en;}
     MUINT32  getRawPathCQ(void)             { return mTopCtrl.CQ; }
     MUINT32  getRawDPathCQ(void)           { return mTopCtrl.CQ_D; }
     IspDrv*  getRawIspDrv(void)              { return mRawIspDrv; }
     IspDrv*  getRawDIspDrv(void)            { return mRawDIspDrv; }
     ISP_TOP_CTRL* getTopCtrl( void )           { return &mTopCtrl; }

     MUINT32  getPathVal(void) {
        return mEnablePath;
     }

     void  setPath(MUINT32 path1, MUINT32 path2) {
       if (ISP_PASS_NONE != path1)  mEnablePath |= path1;

       if (ISP_PASS_NONE != path2)  mEnablePath |= path2;
   }

     void  clearPath(MUINT32 path) {
       if (ISP_PASS_NONE != path)  { mEnablePath &= (~path); }

       if (0 == mEnablePath) {
          //all pathes have been disable, reset variable
          memset(&mTopCtrl, 0, sizeof(mTopCtrl));
          mTwinMode = MFALSE;
       }
   }
public:
   mutable Mutex   mLock;

   IspDrv* mRawIspDrv;
   IspDrv* mRawDIspDrv;
   IspDrv* mpPhyIspDrv;


   MBOOL mTwinMode;
   MUINT32 mEnablePath;
   ISP_TOP_CTRL  mTopCtrl;

public:
    virtual int checkBusy( unsigned long* param ){return 0;}
    virtual unsigned long id( void )                    {   return ISP_TOP;  }
    virtual const char*   name_Str( void )              {   return "ISP_RAW_CENTRAL_CTRL_INFO";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return MT6593_ISP_YUV_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_YUV_REG_RANGE;  }
    //virtual int             is_bypass( void )                {return bypass;}

protected:
       virtual int _config( void ){return 0;}
       virtual int _enable( void* pParam){return 0;}
       virtual int _disable( void ){return 0;}
       //virtual int _write2CQ( void ){return 0;}
       //virtual int     _setZoom( void ) {return 0;}
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_RAW_PIPE
  /////////////////////////////////////////////////////////////////////////////*/
class ISP_RAW_PIPE:public IspFunction_B
{
public:
    int path;
    int pathD;
    unsigned int en_p1;
    unsigned int en_p1_d;
    unsigned int en_p2;
    unsigned int m_num_0;
    unsigned int m_num_0_d;
    unsigned int sub_mode;
    unsigned int sub_mode_D;
    unsigned int img_sel;   //sensor or processed raw

    unsigned int magicNum;
    unsigned int dupCqIdx;  //current working Que
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    IspSize rrz_in_size;
    IspRect rrz_in_roi;
    IspSize rrz_out_size;
    IspSize rrz_d_in_size;
    IspRect rrz_d_in_roi;
    IspSize rrz_d_out_size;
    unsigned int twin_lsc_crop_offx;
    unsigned int twin_lsc_crop_offy;
    int CQ;
    int CQ_D;
    int isTwinMode;
    int src_img_w;
    int src_img_h;
    class ISP_PASS1_CTRL * pPass1Ctrl;
    P1_TUNING_NOTIFY* pP1Tuning;
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
     MBOOL isV3;
     MBOOL isApplyTuning;
    MUINT32 m_Dmao_Reg[ISP_DRV_P1_CQ_DUPLICATION_NUM];
    MUINT32 m_Dmao_SET_Reg[ISP_DRV_P1_CQ_DUPLICATION_NUM];
    MUINT32 m_Dmao_CLR_Reg[ISP_DRV_P1_CQ_DUPLICATION_NUM];
public:
    ISP_RAW_PIPE():
        path(ISP_PASS_NONE),
        en_p1(0),
        en_p1_d(0),
        en_p2(0),
        m_num_0(0),
        m_num_0_d(0),
        magicNum(0),
        dupCqIdx(0),
        burstQueIdx(0),
        drvScenario(0),
        sub_mode(0),
        CQ(CAM_ISP_CQ_NONE),
        CQ_D(CAM_ISP_CQ_NONE),
        isTwinMode(0),
        src_img_w(0),
        src_img_h(0),
        twin_lsc_crop_offx(0),
        twin_lsc_crop_offy(0),
        pathD(ISP_PASS_NONE),
        pP1Tuning(NULL),
        isApplyTuning(MFALSE),
        isV3(MFALSE),
        pTuningIspReg(NULL)
        {
            rrz_in_size = IspSize();
            rrz_out_size = IspSize();
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                m_Dmao_SET_Reg[i] = m_Dmao_CLR_Reg[i] = 0xffffffff;
            }
        };
public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ISP_RAW;  }
    virtual const char*   name_Str( void )              {   return "ISP_RAW";}
    virtual unsigned long reg_Base_Addr_Phy( void )      {   return MT6593_ISP_RAW_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_RAW_REG_RANGE;  }
    int setP1TuneCfg(MUINT32 mNum, MINT32 senDev);
    MUINT32     setEIS(void);                       //because of EIS have the requirement of real time sync with rrz. so, need additional process
    MUINT32     setLCS(void);                       //because of LCS of sync requirement when LCS croping r needed.

protected:
    int checkLscLsciCfg(MUINT32* pLscCfg, MUINT32* pLsciCfg);

    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void ){return 0;}
    virtual int _write2CQ( void );
private:
    int _suspendTuneCfg(MUINT32 mNum);
    void _resumeTuneCfg(IspDrv* _targetVirDrv,MUINT32 _targetDupQueIdx);

};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_RGB
  /////////////////////////////////////////////////////////////////////////////*/
class ISP_RGB_PIPE:public IspFunction_B
{
public:
    unsigned int en_p2;//
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    int sub_mode;
    int CQ;
    MFLOAT rrzRatio; //for sl2
    MUINT32 rrzCropX; //for sl2
    MUINT32 rrzCropY; //for sl2
    MUINT32 enFgMode; //for CFA
    MfbCfg  mfb_cfg;
    MBOOL isV3;
    MBOOL isApplyTuning;
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
public:
    ISP_RGB_PIPE():en_p2(0),magicNum(0),dupCqIdx(0),burstQueIdx(0),drvScenario(0),sub_mode(0),CQ(CAM_ISP_CQ_NONE),isV3(MFALSE),isApplyTuning(MFALSE),rrzRatio(1.0),rrzCropX(0),rrzCropY(0),enFgMode(0),pTuningIspReg(NULL)
    {};
    virtual ~ISP_RGB_PIPE(){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ISP_RGB;  }
    virtual const char*   name_Str( void )              {   return "ISP_RGB";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return MT6593_ISP_RGB_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_RGB_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void ){return 0;}
    virtual int _write2CQ( void );
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_YUV
  /////////////////////////////////////////////////////////////////////////////*/
class ISP_YUV_PIPE:public IspFunction_B
{
public:
    unsigned int en_p2;//
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    int sub_mode;
    int CQ;
    MFLOAT rrzRatio;
    MUINT32 rrzCropX;
    MUINT32 rrzCropY;
    MFLOAT sl2cCrzRatio;
    MUINT32 lceInputW;
    MUINT32 lceInputH;
    CrspCfg crsp_cfg;
    C02Cfg  c02_cfg;
    SrzCfg  srz1_cfg;
    SrzCfg  srz2_cfg;
    MUINT32 fe_ctrl;
    Nr3DCfg nr3dCfg;
    MBOOL isV3;
    MBOOL isApplyTuning;
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
public:
    ISP_YUV_PIPE():en_p2(0),magicNum(0),dupCqIdx(0),burstQueIdx(0),drvScenario(0),sub_mode(0),CQ(CAM_ISP_CQ_NONE),isV3(MFALSE),isApplyTuning(MFALSE),sl2cCrzRatio(1.0),lceInputW(0),lceInputH(0),fe_ctrl(0x0),pTuningIspReg(NULL)
    {};
    ~ISP_YUV_PIPE(){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ISP_YUV;  }
    virtual const char*   name_Str( void )              {   return "ISP_YUV";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return MT6593_ISP_YUV_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_YUV_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void ){return 0;}
    virtual int _write2CQ( void );
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_MDP
  /////////////////////////////////////////////////////////////////////////////*/
class ISP_MDP_PIPE:public IspFunction_B
{
public:
    IspRect src_crop;
    MdpRotDMACfg wroto_out;
    MdpRotDMACfg wdmao_out;
    MdpRotDMACfg jpego_out;
    MdpRotDMACfg imgxo_out;
    MdpRotDMACfg venco_out;
    JpgParaCfg jpgpara_out;
    int mdp_imgxo_p2_en;
    int CQ;
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    MBOOL isWaitBuf;
    int p2_dma_enable;
    MBOOL isV3;
    MBOOL isApplyTuning;
public:
    ISP_MDP_PIPE():mdp_imgxo_p2_en(0),CQ(CAM_ISP_CQ_NONE),magicNum(0),dupCqIdx(0),burstQueIdx(0),drvScenario(-1),isWaitBuf(MTRUE),p2_dma_enable(0),isV3(MFALSE),isApplyTuning(MFALSE){};
    ~ISP_MDP_PIPE(){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ISP_MDP;  }
    virtual const char*   name_Str( void )              {   return "ISP_MDP";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return MT6593_ISP_MDP_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_MDP_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void ){return 0;}
    virtual int _write2CQ( void ){return 0;}
public:
    MBOOL createMdpMgr( void );
    MBOOL destroyMdpMgr( void );
    MBOOL startVideoRecord( MINT32 wd,MINT32 ht, MINT32 fps,NSImageio::NSIspio::EDrvScenario drvScen,MUINT32 cqIdx);
    MBOOL stopVideoRecord( NSImageio::NSIspio::EDrvScenario drvScen,MUINT32 cqIdx);
};



/*/////////////////////////////////////////////////////////////////////////////
    ISP_CDP
  /////////////////////////////////////////////////////////////////////////////*/
class CAM_CRZ_PIPE:public IspFunction_B
{
public:
    int cdrz_filter;
    int crz_walgo;
    int crz_halgo;
    IspSize crz_in;
    IspSize crz_out;
    IspRect crz_crop;
    MdpRotDMACfg vido_out;
    MdpRotDMACfg dispo_out;
    int tpipeMode;
    int disp_vid_sel;
    int conf_cdrz;
    int conf_rotDMA;
    int CQ;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int enable2;
    unsigned int dma_enable;
    int path;
    MBOOL isV3;
    MBOOL isApplyTuning;

public:
    CAM_CRZ_PIPE():conf_cdrz(0),conf_rotDMA(0),CQ(CAM_ISP_CQ_NONE),tpipeMode(CRZ_DRV_MODE_TPIPE),isV3(MFALSE),isApplyTuning(MFALSE),dupCqIdx(0),burstQueIdx(0){};
public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ISP_CRZ;  }
    virtual const char*   name_Str( void )              {   return "ISP_CRZ";}
    virtual unsigned long reg_Base_Addr_Phy( void )      {   return MT6593_ISP_CRZ_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_CRZ_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void );
    virtual int _write2CQ( void );
    virtual int _setZoom( void );
};

#if 0
/*/////////////////////////////////////////////////////////////////////////////
    ISP_CDP
  /////////////////////////////////////////////////////////////////////////////*/
class CAM_CDP_PIPE:public IspFunction_B
{
public:
    int cdrz_filter;
    IspSize cdrz_in;
    IspSize cdrz_out;
    IspRect cdrz_crop;
    IspSize prz_in;
    IspSize prz_out;
    IspRect prz_crop;
    IspSize curz_in;
    IspSize curz_out;
    IspRect curz_crop;
    MdpRotDMACfg vido_out;
    MdpRotDMACfg dispo_out;
    int tpipe_w;
    int disp_vid_sel;
    int conf_cdrz;
    int conf_rotDMA;
    int CQ;
    unsigned int enable2;
    unsigned int dma_enable;
    int path;

public:
    CAM_CDP_PIPE():conf_cdrz(0),conf_rotDMA(0),CQ(CAM_ISP_CQ_NONE){};
public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ISP_CDP;  }
    virtual const char*   name_Str( void )              {   return "ISP_CDP";}
    virtual unsigned long reg_Base_Addr_Phy( void )      {   return MT6593_ISP_CDP_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_ISP_CDP_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void );
    virtual int _write2CQ( void );
    virtual int _setZoom( void );
};
#endif
/*/////////////////////////////////////////////////////////////////////////////
    ISP_TDRI
  /////////////////////////////////////////////////////////////////////////////*/
class CAM_TDRI_PIPE:public IspFunction_B
{
public:
    TdriDrvCfg tdri;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int enTdri;
    int CQ;
    int tcm_en;
    int tdr_en;
    unsigned int magicNum;
    int drvScenario;
    int sub_mode;
    MUINT32 lceInputW;
    MUINT32 lceInputH;
    MBOOL isV3;
    MBOOL isApplyTuning;

public:
    CAM_TDRI_PIPE():dupCqIdx(0),burstQueIdx(0),enTdri(0),CQ(CAM_ISP_CQ_NONE),isV3(MFALSE),isApplyTuning(MFALSE),tcm_en(0),tdr_en(0),magicNum(0),drvScenario(0),sub_mode(0),lceInputW(0),lceInputH(0){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}
//    virtual MBOOL runTpipeDbgLog( void );
    virtual MBOOL configTdriSetting( TdriDrvCfg* pSrcImageio);


public:
    virtual unsigned long id( void )                    {   return ISP_TDRI;  }
    virtual const char*   name_Str( void )              {   return "ISP_TDRI";}
    virtual unsigned long reg_Base_Addr_Phy( void )      {   return MT6593_DMA_TDRI_BASE;  }
    virtual unsigned long reg_Range( void )             {   return MT6593_DMA_TDRI_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void ){ return 0;}
    virtual int _write2CQ( void );
    virtual int _setZoom( void ){ return 0;}
};



/*/////////////////////////////////////////////////////////////////////////////
    DMA_IMGI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_IMGI:public DMAI_B
{
public:

public:
    DMA_IMGI(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_IMGI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_IMGI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_IMGI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_IMGI_REG_RANGE;     }
protected:

};
/*/////////////////////////////////////////////////////////////////////////////
    DMA_BPCI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_BPCI:public DMAI_B
{
public:

public:
    DMA_BPCI(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_BPCI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_BPCI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_BPCI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_BPCI_REG_RANGE;     }
protected:

};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_BPCI_D
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_BPCI_D:public DMAI_B
{
public:

public:
    DMA_BPCI_D(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_BPCI_D;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_BPCI_D";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_BPCI_D_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_BPCI_D_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_LSCI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_LSCI:public DMAI_B
{
public:

public:
    DMA_LSCI(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_LSCI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_LSCI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_LSCI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_LSCI_REG_RANGE;     }
protected:
};

class DMA_LSCI_D:public DMAI_B
{
public:

public:
    DMA_LSCI_D(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_LSCI_D;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_LSCI_D";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_LSCI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_LSCI_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_UFDI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_UFDI:public DMAI_B
{
public:

public:
    DMA_UFDI(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_UFDI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_UFDI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_UFDI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_UFDI_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_LCEI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_LCEI:public DMAI_B
{
public:

public:
    DMA_LCEI(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_LCEI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_LCEI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_LCEI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_LCEI_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_VIPI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_VIPI:public DMAI_B
{
public:
public:
    DMA_VIPI(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_VIPI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_VIPI";}
    virtual unsigned long reg_Base_Addr_Phy( void ) {    return     MT6593_DMA_VIPI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_VIPI_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_VIP2I
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_VIP2I:public DMAI_B
{
public:

public:
    DMA_VIP2I(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_VIP2I;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_VIP2I";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_VIP2I_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_VIP2I_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_VIP3I
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_VIP3I:public DMAI_B
{
public:

public:
    DMA_VIP3I(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_VIP3I;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_VIP3I";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return    MT6593_DMA_VIP3I_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_VIP3I_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_TDRI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_TDRI:public DMAI_B
{
public:
public:
    DMA_TDRI()
        {};
public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_TDRI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_TDRI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_TDRI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_TDRI_REG_RANGE;     }
protected:
    //virtual int _config( void );
    //virtual int _write2CQ( int cq );
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_CQI
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_CQ:public DMAI_B
{
public:
public:
    DMA_CQ(){};
public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_CQI;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_CQI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_CQI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_CQI_REG_RANGE;     }
protected:
    //virtual int _enable( void* pParam );
    //virtual int _disable( void );
};



/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMGO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_IMGO:public DMAO_B
{
public:

public:
    DMA_IMGO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_IMGO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_IMGO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_IMGO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_IMGO_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_RRZO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_RRZO:public DMAO_B
{
public:

public:
    DMA_RRZO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_RRZO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_RRZO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_RRZO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_RRZO_REG_RANGE;     }
protected:
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMGO_D
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_IMGO_D:public DMAO_B
{
public:

public:
    DMA_IMGO_D(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_IMGO_D;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_IMGO_D";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_IMGO_D_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_IMGO_D_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_RRZO_D
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_RRZO_D:public DMAO_B
{
public:

public:
    DMA_RRZO_D(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_RRZO_D;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_RRZO_D";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_RRZO_D_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_RRZO_D_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG2O
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_IMG2O:public DMAO_B // for pass2 img2o
{
public:

public:
    DMA_IMG2O(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_IMG2O;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_IMG2O";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_IMG2O_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_IMG2O_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_LCSO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_LCSO:public DMAO_B
{
public:

public:
    DMA_LCSO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_LCSO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_LCSO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_LCSO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_LCSO_REG_RANGE;     }
protected:
};

class DMA_LCSO_D:public DMAO_B
{
public:

public:
    DMA_LCSO_D(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_LCSO_D;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_LCSO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_LCSO_D_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_LCSO_D_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_AAO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_AAO:public DMAO_B
{
public:

public:
    DMA_AAO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_AAO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_AAO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_AAO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_AAO_REG_RANGE;     }
protected:
};

class DMA_AAO_D:public DMAO_B
{
public:

public:
    DMA_AAO_D(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_AAO_D;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_AAO_D";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_AAO_D_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_AAO_D_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_ESFKO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_ESFKO:public DMAO_B
{
public:

public:
    DMA_ESFKO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_ESFKO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_ESFKO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_ESFKO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_ESFKO_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_AFO_D
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_AFO_D:public DMAO_B
{
public:

public:
    DMA_AFO_D(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_AFO_D;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_AFO_D";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_AFO_D_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_AFO_D_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
  ISP_DMA_AFO
/////////////////////////////////////////////////////////////////////////////*/
class DMA_AFO:public DMAO_B
{
public:

public:
  DMA_AFO(){};

public:/*[IspFunction_B]*/
  virtual unsigned long id( void )                {    return     ISP_DMA_AFO;  }
  virtual const char*   name_Str( void )          {    return     "ISP_DMA_AFO";}
  virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_AFO_BASE;     }
  virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_AFO_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
  ISP_DMA_EISO
/////////////////////////////////////////////////////////////////////////////*/
class DMA_EISO:public DMAO_B
{
public:

public:
  DMA_EISO(){};

public:/*[IspFunction_B]*/
  virtual unsigned long id( void )                {    return     ISP_DMA_EISO;  }
  virtual const char*   name_Str( void )          {    return     "ISP_DMA_AFO";}
  virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_EISO_BASE;     }
  virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_EISO_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
ISP_DMA_UFEO
/////////////////////////////////////////////////////////////////////////////*/
class DMA_UDEO:public DMAO_B
{
public:

public:
    DMA_UDEO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_UFEO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_UFEO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_UFEO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_UFEO_REG_RANGE;     }
protected:
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG3O
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_IMG3O:public DMAO_B
{
public:

public:
    DMA_IMG3O(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_IMG3O;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_IMG3O";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_IMG3O_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_IMG3O_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG3BO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_IMG3BO:public DMAO_B
{
public:

public:
    DMA_IMG3BO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_IMG3BO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_IMG3BO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_IMG3BO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_IMG3BO_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG3CO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_IMG3CO:public DMAO_B
{
public:

public:
    DMA_IMG3CO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_IMG3CO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_IMG3CO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_IMG3CO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_IMG3CO_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_FEO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_FEO:public DMAO_B
{
public:

public:
    DMA_FEO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_FEO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_FEO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_FEO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_FEO_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_MFBO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_MFBO:public DMAO_B
{
public:

public:
    DMA_MFBO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_MFBO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_MFBO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_MFBO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_MFBO_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_CAMSV_IMGO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_CAMSV_IMGO:public DMAO_B
{
public:

public:
    DMA_CAMSV_IMGO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_CAMSV_IMGO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_CAMSV_IMGO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_CAMSV_IMGO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_CAMSV_IMGO_REG_RANGE;     }
protected:
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_CAMSV2_IMGO
  /////////////////////////////////////////////////////////////////////////////*/
class DMA_CAMSV2_IMGO:public DMAO_B
{
public:

public:
    DMA_CAMSV2_IMGO(){};

public:/*[IspFunction_B]*/
    virtual unsigned long id( void )                {    return     ISP_DMA_CAMSV2_IMGO;  }
    virtual const char*   name_Str( void )          {    return     "ISP_DMA_CAMSV2_IMGO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     MT6593_DMA_CAMSV2_IMGO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     MT6593_DMA_CAMSV2_IMGO_REG_RANGE;     }
protected:
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_TWIN
  /////////////////////////////////////////////////////////////////////////////*/

typedef enum
{
    ISP_BUF_EMPTY = 0,
    ISP_BUF_FILLED,
}ISP_BUF_STATUS;
//
#if 0
typedef enum
{
    ISP_BUF_TYPE_PMEM    = BUF_TYPE_PMEM,
    ISP_BUF_TYPE_STD_M4U = BUF_TYPE_STD_M4U,
    ISP_BUF_TYPE_ION     = BUF_TYPE_ION,
}ISP_BUF_TYPE;
#endif
//
struct stISP_RRZ_INFO
{
    MUINT32                 srcX;
    MUINT32                 srcY;
    MUINT32                 srcW;
    MUINT32                 srcH;
    MUINT32                 dstW;
    MUINT32                 dstH;
};

//
struct stISP_BUF_INFO
{
    ISP_BUF_STATUS          status;
    MUINTPTR                base_vAddr;
    MUINTPTR                base_pAddr;
    MUINT32                 size;
    MUINT32                 memID;
    MINT32                  bufSecu;
    MINT32                  bufCohe;
    MUINT64                 timeStampS;
    MUINT32                 timeStampUs;
    //
    MUINT32                 img_w;
    MUINT32                 img_h;
    MUINT32                 img_stride;
    MUINT32                 img_fmt;
    MUINT32                 img_pxl_id;
    MUINT32                 img_wbn;
    MUINT32                 img_ob;
    MUINT32                 img_lsc;
    MUINT32                 img_rpg;
    MUINT32                 m_num_0;
    MUINT32                 m_num_1;
    MUINT32                 frm_cnt;
    MUINT32                 raw_type;
    MUINT32                 dma;
    //
    stISP_RRZ_INFO          rrz_info;
    IspRect                 dma_crop;   //dmao cropping
    MVOID                   *header_info; //point to self
    MUINT32                 header_size;
    MINT32                  mBufIdx; // used for replace buffer

    //
    MUINT32                 jpg_size;
    MVOID                   *private_info;
    struct stISP_BUF_INFO   *next;
    //
    stISP_BUF_INFO(
        ISP_BUF_STATUS          _status = ISP_BUF_EMPTY,
        MUINTPTR                 _base_vAddr = 0,
        MUINTPTR                 _base_pAddr = 0,
        MUINT32                 _size = 0,
        MUINT32                 _memID = -1,
        MINT32                  _bufSecu = 0,
        MINT32                  _bufCohe = 0,
        MUINT64                 _timeStampS = 0,
        MUINT32                 _timeStampUs = 0,
        MUINT32                 _img_w = 0,
        MUINT32                 _img_h = 0,
        MUINT32                 _img_fmt = 0,
        MUINT32                 _img_hbin1 = 0,
        MUINT32                 _img_ob = 0,
        MUINT32                 _img_lsc = 0,
        MUINT32                 _img_rpg = 0,
        MUINT32                 jpg_size = 0,
        MVOID                   *_private_info = NULL,
        MUINT32                 dma = _imgo_,
        struct stISP_BUF_INFO   *_next = 0)
    : status(_status)
    , base_vAddr(_base_vAddr)
    , base_pAddr(_base_pAddr)
    , size(_size)
    , memID(_memID)
    , bufSecu(_bufSecu)
    , bufCohe(_bufCohe)
    , timeStampS(_timeStampS)
    , timeStampUs(_timeStampUs)
    , private_info(_private_info)
    , next(_next)
    {}
};

typedef struct _isp_p1_mNum_workaround_node
{
    unsigned int     mVal;
    stISP_RRZ_INFO   rrz_info;
    MBOOL            mFinished;

}  EISP_MNUM_NODE;


class ISP_PASS1_CTRL:public IspFunction_B
{
public:
    //TwinDrvCfg m_twinCfg;
    int isTwinMode;

    MUINT32 CQ;
    MUINT32 CQ_D;

    int path;
    int path_D;

    unsigned int    dupCqIdx;   //current working Que
    unsigned int    burstQueIdx;


    mutable Mutex        m_p1MagicNum_IMGO_LLock;
    list<EISP_MNUM_NODE> m_p1MagicNum_IMGO_list;

    mutable Mutex        m_p1MagicNum_RRZO_LLock;
    list<EISP_MNUM_NODE> m_p1MagicNum_RRZO_list;

    mutable Mutex        m_p1MagicNum_IMGOD_LLock;
    list<EISP_MNUM_NODE> m_p1MagicNum_IMGOD_list;

    mutable Mutex        m_p1MagicNum_RRZOD_LLock;
    list<EISP_MNUM_NODE> m_p1MagicNum_RRZOD_list;

    IspRrzCfg m_RrzCfg;
    IspRrzCfg m_RrzDCfg;

public:
    ISP_PASS1_CTRL():
            isTwinMode(0),
            path(ISP_PASS_NONE),
            path_D(ISP_PASS_NONE),
            dupCqIdx(0),
            burstQueIdx(0),
            CQ(CAM_ISP_CQ_NONE),
            CQ_D(CAM_ISP_CQ_NONE)
            {
                m_p1MagicNum_IMGO_list.resize(0);
                m_p1MagicNum_RRZO_list.resize(0);
                m_p1MagicNum_IMGOD_list.resize(0);
                m_p1MagicNum_RRZOD_list.resize(0);

            }

public:
    virtual int checkBusy( unsigned long* param ){return 0;}
    virtual MBOOL runTwinDbgLog( void );
    void resetP1MagicNum(MUINT32 dma);
    int keepP1RrzCfg( IspRrzCfg const rrz_cfg );

public:
    virtual unsigned long id( void )                       { return ISP_TWIN; }
    virtual const char*   name_Str( void )              { return "ISP_TWIN"; }
    virtual unsigned long reg_Base_Addr_Phy( void ) { return MT6593_ISP_TWIN_BASE; }
    virtual unsigned long reg_Range( void )            { return MT6593_ISP_TWIN_REG_RANGE; }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam  ) {return 0;}
    virtual int _disable( void ) {
        m_p1MagicNum_IMGO_list.resize(0);
        m_p1MagicNum_RRZO_list.resize(0);
        m_p1MagicNum_IMGOD_list.resize(0);
        m_p1MagicNum_RRZOD_list.resize(0);
        return 0;
    }
    virtual int _write2CQ( void );
public:
    virtual int setP1RrzCfg( IspRrzCfg const rrz_cfg );
    int setP1MagicNum( MUINT32 pass1_path, MUINT32 MagicNum, MUINT32 magicNumCnt);

private:


};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
//
#if 0
typedef enum
{
    _imgi_  = 0,
    _imgci_,    // 1
    _vipi_ ,    // 2
    _vip2i_,    // 3
    _imgo_,     // 4
    _rrzo_,    // 5
    _dispo_,    // 6
    _vido_,     // 7
    _fdo_,      // 8
    _rt_dma_max_
}_isp_dma_enum_;
#endif
//

/*************************************************
*************************************************/
//
typedef list<stISP_BUF_INFO> ISP_BUF_INFO_L;
//
typedef struct  _isp_buf_list_{
    MUINT32         filledCnt;            //  fill count
    ISP_BUF_INFO_L  bufInfoList;
}stISP_BUF_LIST;
//
typedef struct  _isp_filled_buf_list_{
    ISP_BUF_INFO_L  *pBufList;
}stISP_FILLED_BUF_LIST;


#define ISP_BUF_INFO_NODE_MAX_CNT 32
typedef struct  _isp_buf_header_node_{
    stISP_BUF_INFO  mInfo; //put this in the begin of the struct to reduce isp_container_of overhead
    MBOOL           mOccupied;
}stISP_BUF_HEADER_NODE;

typedef struct  _isp_buf_header_list_{
    mutable Mutex          mLLock;
    MUINT32                mIdx;
    stISP_BUF_HEADER_NODE  mN[ISP_BUF_INFO_NODE_MAX_CNT];
}stISP_BUF_HEADER_LIST;

//
class ISP_BUF_CTRL : public IspFunction_B
{
friend class IspDrvShellImp;
friend class ISP_TOP_CTRL;

private:

    stISP_BUF_HEADER_LIST m_p1DeqedBufHeaderL[_rt_dma_max_];

    static stISP_BUF_LIST m_hwbufL[_rt_dma_max_];
    static stISP_BUF_LIST m_p2HwBufL[ISP_DRV_P2_CQ_NUM][ISP_DRV_P2_CQ_DUPLICATION_NUM][_rt_dma_max_];

public:
    class ISP_PASS1_CTRL * pPass1Ctrl;
    int path;
    int path_D;
    int drvScenario;
    int dequeCq;  // for P2 deque use
    unsigned int dequeDupCqIdx;  // for P2 deque use
    unsigned int dequeBurstQueIdx;  // for P2 deque use
    int CQ;
    int CQ_D;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    MUINT32      P1DeQCnt[_rt_dma_max_];//for dbg
public:
    ISP_BUF_CTRL():path(0),drvScenario(-1),dequeCq(CAM_ISP_CQ_NONE),dequeDupCqIdx(0),dequeBurstQueIdx(0),CQ(CAM_ISP_CQ_NONE),CQ_D(CAM_ISP_CQ_NONE),path_D(ISP_PASS_NONE),dupCqIdx(0),burstQueIdx(0)
        {/*memset(&m_hwBuf,0x00,sizeof(_isp_buf_status_));
         memset(&m_swBuf,0x00,sizeof(_isp_buf_status_));
         memset(m_Buffer,0x00,sizeof(m_Buffer));*/
         for(int i=0;i<_rt_dma_max_;i++){
            P1DeQCnt[i] = 0;
         }
         };
    virtual unsigned long id( void )                    {   return ISP_BUFFER;  }
    virtual const char*   name_Str( void )              {   return "ISP_BUFFER";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return (unsigned long)NULL;  }
    virtual unsigned long reg_Range( void )             {   return (unsigned long)NULL;  }
    virtual int checkBusy( unsigned long* param ) {return 0;}
    virtual int init( MUINT32 dmaChannel );
    virtual MBOOL waitBufReady( MUINT32 dmaChannel );
    /*
        * enqueueBuf
        *       append new buffer to the end of hwBuf list
        */
    virtual MINT32 enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo,MINT32 enqueCq =0,MINT32 dupCqIdx =0,MBOOL bImdMode = 0 );
    virtual MINT32 enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data,MBOOL bImdMode);
    /*
        * dequeueMdpFrameEnd
        *       set mdp deque frame end for each frame end
        */
    virtual MINT32 dequeueMdpFrameEnd( MINT32 drvScenario );
    /*
        * dequeueHwBuf
        *       set buffer FILLED type and
        *       move filled Hw buffer to sw the end of list.
        */
    virtual EIspRetStatus dequeueHwBuf( MUINT32 dmaChannel, stISP_FILLED_BUF_LIST& bufList );

    /*
        * dequeueBuf
        *       delete all swBuf list after inform caller
        */
    //virtual MINT32 dequeueSwBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufList );
    /*
        * getCurrHwBuf
        *       get 1st NOT filled HW buffer address
        */
    virtual MUINT32 getCurrHwBuf( MUINT32 dmaChannel );
    /*
        * getNextHwBuf
        *       get 2nd NOT filled HW buffer address
        */
    virtual MUINT32 getNextHwBuf( MUINT32 dmaChannel );
    /*
        * freeSinglePhyBuf
        *       free single physical buffer
        */
    virtual MUINT32 freeSinglePhyBuf(
        stISP_BUF_INFO bufInfo);
    /*
        * freeAllPhyBuf
        *       free all physical buffer
        */
    virtual MUINT32 freeAllPhyBuf( void );

private:
    int getDmaBufIdx( MUINT32 dmaChannel );
    int debugPrint( MUINT32 dmaChannel );
    stISP_BUF_INFO * saveDeqBufInfo( MINT32 rt_dma);
    MINT32 returnDeqBufInfo( MINT32 rt_dma, stISP_BUF_INFO* bufInfo);

protected:
    virtual int     _config( void ) {return 0;}
    virtual int     _enable( void* pParam  ) {return 0;}
    virtual int     _disable( void ) {return 0;}
    virtual int     _write2CQ( void ) {return 0;}
    virtual int     _setZoom( void ) {return 0;}

};


//
class IspEventThread
{
    protected:
        IspEventThread(){};
        virtual ~IspEventThread() {};

    public:     ////        Instantiation.
        static  IspEventThread*  createInstance(IspDrv* pIspDrv);

    public:
        virtual MBOOL   init(void) = 0;
        virtual MBOOL   uninit(void) = 0;

};



////////////////////////////////////////////////////////////////////////////////




#endif /*__ISP_FUNCTION_H__*/




