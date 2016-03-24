/**
* @file isp_function_FrmB.h
*
* isp_function Header File
*/

#ifndef __ISP_FUNCTION_FRMB_H__
#define __ISP_FUNCTION_FRMB_H__

#include <vector>
#include <map>
#include <list>
using namespace std;

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

#include "isp_datatypes_FrmB.h"
#include <mtkcam/drv_common/isp_reg.h>
#include <mtkcam/drv_FrmB/isp_drv_FrmB.h>


#include "cdp_drv_FrmB.h"
#include <mtkcam/drv_common/imem_drv.h>
#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/imageio/ispio_pipe_scenario.h>    // For enum EScenarioID.
#include <mtkcam/Log.h>
#include "camera_isp_FrmB_D2.h"

//
using namespace NSIspDrv_FrmB;

//add this for p1 tuning notification
#include "mtkcam/imageio/p1HwCfg.h"
using namespace NSCam::NSIoPipe::NSCamIOPipe;

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

#if 1
#if defined(USING_MTK_ION)
    #define __ISP_USE_ION__
#else
    #define __ISP_USE_PMEM__
#endif
#else //_mt6593fpga_dvt_use_
    #define __ISP_USE_PMEM__
#endif

#if defined(__ISP_USE_PMEM__)
    #if defined(__ISP_USE_STD_M4U__) || defined(__ISP_USE_ION__)
        #error "MULTIPLE MEMORY TYPE DEFINED"
    #endif
#elif defined(__ISP_USE_STD_M4U__)
    #if defined(__ISP_USE_PMEM__) || defined(__ISP_USE_ION__)
        #error "MULTIPLE MEMORY TYPE DEFINED"
    #endif
#elif defined(__ISP_USE_ION__)
    #if defined(__ISP_USE_PMEM__) || defined(__ISP_USE_STD_M4U__)
        #error "MULTIPLE MEMORY TYPE DEFINED"
    #endif
#else
    #error "NO MEMORY TYPE DEFINED"
#endif

/*/////////////////////////////////////////////////////////////////////////////
    [v3] for HalPipeWrapper.Thread_FrmB.cpp
  /////////////////////////////////////////////////////////////////////////////*/

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
#define _USING_FRAME_BASED_
#ifdef _USING_FRAME_BASED_
/*/////////////////////////////////////////////////////////////////////////////*/

#define ISP_P1_CQ(cq) (NSIspDrv_FrmB::CAM_ISP_CQ0 == cq || NSIspDrv_FrmB::CAM_ISP_CQ0B == cq || NSIspDrv_FrmB::CAM_ISP_CQ0C == cq)
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

#endif /* _USING_FRAME_BASED_ */
/*/////////////////////////////////////////////////////////////////////////////
    end: [v3]
/////////////////////////////////////////////////////////////////////////////*/


/*
    //path
        -IC
            -IC_PASS1
                -TG->Dram(imgo)
                    |
                    ->isp(raw)->lcs0/aao/flko/afo/eiso
            -IC_PASS2(CQ/Tpipe)
                -Dram->isp(raw/rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram
        -VR
            -VR_PASS1
                -TG->isp(raw)->isp(rgb)->isp(yuv)->cdrz->Dram
            -VR_PASS2(CQ)
                -Dram->curz->prz/vrz->Dram
        -IP
            -IP_PASS2(CQ/Tpipe)
                -Dram->isp(raw/rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram
        -ZSD
            -ZSD_PASS1
                -TG->isp(raw)->isp(rgb)->isp(yuv)->cdrz->Dram(img2o)
                    |
                    ->Dram(imgo)
            -ZSD_PASS2(CQ)
                -Dram->curz->prz/vrz->dispo/vido/vrzo->Dram
        -N3D_IC
            -N3D_IC_PASS1
                -TG-> Dram(imgo)
                    |
                    ->Dram(img2o)
                    |
                    ->isp(raw)->Dram(lsco/aao/afo/eiso)
            -N3D_IC_PASS2(CQ1/CQ2/Tpipe)
                -Dram->isp(raw)->isp(rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram
        -N3D_VR
            -N3D_VR_PASS1
                -TG-> Dram(imgo)
                    |
                    ->Dram(img2o)
                    |
                    ->isp(raw)->Dram(lsco/aao/nr3o/afo/eiso)
            -N3D_VR_PASS2(CQ1/CQ2)
                -Dram->isp(raw)->isp(rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram


    //CQ
        -set 1 register
            -0x00004000 -> {command_set[5:0],command_cnt[9:0],ini_addr[15:0]}
            -0x12345678 -> register set 0xC2084000 = 0x12345678
            -0xFC000000 -> End CQ release imgi,trigger camera
        -set 3 registers
            -0x00024100 -> {command_set[5:0],command_cnt[9:0],ini_addr[15:0]}
            -0x01234567 -> register set 0xC2084100 = 0x12345678
            -0x12345678 -> register set 0xC2084104 = 0x12345678
            -0x23456789 -> register set 0xC2084108 = 0x23456789
            -0xFC000000 -> End CQ release imgi,trigger camera

    //TDR
        -put tpipe_map data in Dram
        -set CAM_TDRI_BASE_ADDR
        -set CAM_TDRI_OFST_ADDR = 0;
        -set CAM_TDRI_XSIZE = sizeof(tile_map)
        -enable CAM_CTL_TCM_EN[31]
        -enable specific DMA
            -CAM_CTL_DMA_EN

    //DMA programming guide
        -enable specific DMA
            -CAM_CTL_DMA_EN
        -enable DMA internal error/OTF overflow error interrupt
            -CAM_CTL_INT_EN[30][24:20]
        -enable DMA status interrupt
            -CAM_CTL_DMA_INT[24:16][8:0]
        -config used i/o DMA
            -input data format for alignment check(BASE_ADDR/OFST_ADDR/STRIDE)
                -YUV422 one plane - 4 Bytes
                -YUV420/422 two plane - 2 Bytes
                -JPG - 16 Bytes
                -CAM_CTL_FMT_SEL[10:8]
            -CAM_IMGI_BASE_ADDR
            -CAM_IMGI_OFST_ADDR
            -CAM_IMGI_XSIZE (from 0)
                -multiple of data bus width(6589_ISP_DMA_Description.xls)
            -CAM_IMGI_YSIZE (from 0)
            -CAM_IMGI_STRIDE (>=(XSIZE+1))
            -CAM_IMGI_CON = 0x08404040
                -max_burst_len = {1,2,4,8}

            -CAM_IMGO_BASE_ADDR
            -CAM_IMGO_OFST_ADDR
            -CAM_IMGO_XSIZE (from 0)
            -CAM_IMGO_YSIZE (from 0)
            -CAM_IMGO_STRIDE (>=(XSIZE+1))
            -CAM_IMGO_CON = 0x08505050
                -max_burst_len = {1,2,4,8}
            -CAM_IMGO_CROP
    //SW RST
        -CAM_CTL_SW_CTL[0] = 1
        -wait
            -CAM_CTL_SW_CTL[1] = 1
            -CAM_DMA_SOFT_RSTSTAT = 0x003F01FE
        -CAM_CTL_SW_CTL[2] = 1
        -delay
        -CAM_CTL_SW_CTL[0] = 0
        -CAM_CTL_SW_CTL[2] = 0
*/

namespace NSDrvCam_FrmB {


// for crop function
#define CROP_FLOAT_PECISE_BIT   31    // precise 31 bit
#define CROP_TPIPE_PECISE_BIT   20
#define CROP_CDP_HARDWARE_BIT   8

#define ISP_PASS1    (0x01)
#define ISP_PASS2    (0x02)
#define ISP_PASS2B   (0x04)
#define ISP_PASS2C   (0x08)
#define ISP_PASS2FMT (0x10)

//start bit
#define CAM_ISP_PASS1_START         0
#define CAM_ISP_PASS2_START         1
#define CAM_ISP_PASS2B_START        2
#define CAM_ISP_PASS2C_START        3
#define CAM_ISP_FMT_START           4
#define CAM_ISP_PASS1_CQ0_START     5
#define CAM_ISP_PASS1_CQ0B_START    6

#define ISP_TOP             ( 1u << 0 )
#define ISP_RAW             ( 1u << 1 )
#define ISP_RGB             ( 1u << 2 )
#define ISP_YUV             ( 1u << 3 )
#define ISP_MDP             ( 1u << 4 )
#define ISP_CDP             ( 1u << 5 )
#define ISP_TDRI            ( 1u << 6 )
#define ISP_DMA_IMGI        ( 1u << 7 )
#define ISP_DMA_IMGCI       ( 1u << 8 )
#define ISP_DMA_FLKI        ( 1u << 9 )          //CHRISTOPHER: need to be removed
#define ISP_DMA_LSCI        ( 1u << 10)
#define ISP_DMA_LCEI        ( 1u << 11)          //CHRISTOPHER: need to be removed
#define ISP_DMA_VIPI        ( 1u << 12)          //CHRISTOPHER: need to be removed
#define ISP_DMA_VIP2I       ( 1u << 13)          //CHRISTOPHER: need to be removed
#define ISP_DMA_CQI         ( 1u << 14)
#define ISP_DMA_TDRI        ( 1u << 15)
#define ISP_DMA_IMGO        ( 1u << 16)
#define ISP_DMA_IMG2O       ( 1u << 17)
#define ISP_DMA_LCSO        ( 1u << 18)          //CHRISTOPHER: need to be removed
#define ISP_DMA_AAO         ( 1u << 19)
#define ISP_DMA_ESFKO       ( 1u << 20)
#define ISP_DMA_AFO         ( 1u << 21)          //CHRISTOPHER: need to be removed
#define ISP_DMA_EISO        ( 1u << 22)          //CHRISTOPHER: need to be removed
#define ISP_DMA_WDMAO       ( 1u << 23)
#define ISP_DMA_WROTO       ( 1u << 24)
#define ISP_DMA_FDO         ( 1u << 25)          //CHRISTOPHER: need to be removed
#define ISP_BUFFER          ( 1u << 26)
#define ISP_TURNING         ( 1u << 27)
#define ISP_PASS1_CTL       ( 1u << 28)

#define ISP_FUNCTION_MAX_NUM (29) /*28*/

//Tpipe Driver
#define ISP_MAX_TDRI_HEX_SIZE           (TPIPE_DRV_MAX_TPIPE_HEX_SIZE)  //36K
#define ISP_MAX_RING_TDRI_SIZE          (768000) // 750K

//isp top register range
#define MT6589_ISP_TOP_BASE             ISP_BASE_HW
#define MT6589_ISP_TOP_REG_RANGE        ISP_BASE_RANGE

//ISP_RAW
#define MT6589_ISP_RAW_BASE             (MT6589_ISP_TOP_BASE + 0x0)
#define MT6589_ISP_RAW_REG_RANGE        MT6589_ISP_TOP_REG_RANGE
//ISP_RGB
#define MT6589_ISP_RGB_BASE             (MT6589_ISP_TOP_BASE + 0x0)
#define MT6589_ISP_RGB_REG_RANGE        MT6589_ISP_TOP_REG_RANGE
//ISP_YUV
#define MT6589_ISP_YUV_BASE             (MT6589_ISP_TOP_BASE + 0x0)
#define MT6589_ISP_YUV_REG_RANGE        MT6589_ISP_TOP_REG_RANGE
//ISP_MDP
#define MT6593_ISP_MDP_BASE             (MT6589_ISP_TOP_BASE + 0x0)
#define MT6593_ISP_MDP_REG_RANGE        MT6589_ISP_TOP_REG_RANGE
//ISP_CDP
#define MT6589_ISP_CDP_BASE             (MT6589_ISP_TOP_BASE + 0xB00)
#define MT6589_ISP_CDP_REG_RANGE        0x1000
//ISP_DMA_IMGI
#define MT6589_DMA_IMGI_BASE            (MT6589_ISP_TOP_BASE + 0x230)
#define MT6589_DMA_IMGI_REG_RANGE       0x20
//ISP_DMA_IMGCI
#define MT6589_DMA_IMGCI_BASE           (MT6589_ISP_TOP_BASE + 0x24C)
#define MT6589_DMA_IMGCI_REG_RANGE      0x20
//ISP_DMA_FLKI
#define MT6589_DMA_FLKI_BASE            (MT6589_ISP_TOP_BASE + 0x290)
#define MT6589_DMA_FLKI_REG_RANGE       0x20
//ISP_DMA_LSCI
#define MT6589_DMA_LSCI_BASE            (MT6589_ISP_TOP_BASE + 0x278)
#define MT6589_DMA_LSCI_REG_RANGE       0x20
//ISP_DMA_LCEI
#define MT6589_DMA_LCEI_BASE            (MT6589_ISP_TOP_BASE + 0x2A8)
#define MT6589_DMA_LCEI_REG_RANGE       0x20
//ISP_DMA_VIPI
#define MT6589_DMA_VIPI_BASE            (MT6589_ISP_TOP_BASE + 0x2C0)
#define MT6589_DMA_VIPI_REG_RANGE       0x20
//ISP_DMA_VIP2I
#define MT6589_DMA_VIP2I_BASE            (MT6589_ISP_TOP_BASE + 0x2D8)
#define MT6589_DMA_VIP2I_REG_RANGE       0x20
//ISP_DMA_IMGO
#define MT6589_DMA_IMGO_BASE            (MT6589_ISP_TOP_BASE + 0x2F0)
#define MT6589_DMA_IMGO_REG_RANGE       0x20
//ISP_DMA_IMG2O
#define MT6589_DMA_IMG2O_BASE            (MT6589_ISP_TOP_BASE + 0x30C)
#define MT6589_DMA_IMG2O_REG_RANGE       0x20
//ISP_DMA_LCSO
#define MT6589_DMA_LCSO_BASE            (MT6589_ISP_TOP_BASE + 0x328)
#define MT6589_DMA_LCSO_REG_RANGE       0x20
//ISP_DMA_AAO
#define MT6589_DMA_AAO_BASE            (MT6589_ISP_TOP_BASE + 0x368)
#define MT6589_DMA_AAO_REG_RANGE       0x20
//ISP_DMA_ESFKO
#define MT6589_DMA_ESFKO_BASE            (MT6589_ISP_TOP_BASE + 0x350)
#define MT6589_DMA_ESFKO_REG_RANGE       0x20
//ISP_DMA_AFO
#define MT6589_DMA_AFO_BASE            (MT6589_ISP_TOP_BASE + 0x348)
#define MT6589_DMA_AFO_REG_RANGE       0x20
//ISP_DMA_EISO
#define MT6589_DMA_EISO_BASE            (MT6589_ISP_TOP_BASE + 0x340)
#define MT6589_DMA_EISO_REG_RANGE       0x20
//ISP_DMA_WDMAO
#define MT6589_DMA_WDMAO_BASE            (MT6589_ISP_TOP_BASE + 0xD40)
#define MT6589_DMA_WDMAO_REG_RANGE       0x100
//ISP_DMA_WROTO
#define MT6589_DMA_WROTO_BASE            (MT6589_ISP_TOP_BASE + 0xCC0)
#define MT6589_DMA_WROTO_REG_RANGE       0x100
//ISP_DMA_CQ0I
#define MT6589_DMA_CQI_BASE            (MT6589_ISP_TOP_BASE + 0x210)
#define MT6589_DMA_CQI_REG_RANGE       0x08
//ISP_DMA_TDRI
#define MT6589_DMA_TDRI_BASE            (MT6589_ISP_TOP_BASE + 0x204)
#define MT6589_DMA_TDRI_REG_RANGE       0x20

#define MT6582_HRZ_BASE                 (MT6589_ISP_TOP_BASE + 0x580)
#define MT6582_HRZ_BASE_REG_RANGE       (0x08)

#define ISP_MAX_RINGBUFFER_CNT  16


#define ISP_HW_SCENARIO_IP         3
#define ISP_HW_SCENARIO_CC         6
#define ISP_HW_SCENARIO_NUM        2
//-->definition compatible to mt6589_scenario.xlsx
//cam_fmt_sel
//->//
#define ISP_SCENARIO_IC         0
#define ISP_SCENARIO_VR         1
#define ISP_SCENARIO_ZSD        2
#define ISP_SCENARIO_IP         3
#define ISP_SCENARIO_VEC        4
#define ISP_SCENARIO_RESERVE01  5
#define ISP_SCENARIO_N3D_IC     6
#define ISP_SCENARIO_N3D_VR     7
#define ISP_SCENARIO_MAX        8
//->//
#define ISP_SUB_MODE_RAW        0
#define ISP_SUB_MODE_YUV        1
#define ISP_SUB_MODE_RGB        2
#define ISP_SUB_MODE_JPG        3
#define ISP_SUB_MODE_MFB        4
#define ISP_SUB_MODE_VEC        0
#define ISP_SUB_MODE_RGB_LOAD   3
#define ISP_SUB_MODE_MAX        5
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
#define CAM_FMT_SEL_BAYER8       0
#define CAM_FMT_SEL_BAYER10      1
#define CAM_FMT_SEL_BAYER12      2
//->//
#define CAM_FMT_SEL_TG_SW_UYVY      0
#define CAM_FMT_SEL_TG_SW_YUYV      1
#define CAM_FMT_SEL_TG_SW_VYUY      2
#define CAM_FMT_SEL_TG_SW_YVYU      3
#define CAM_FMT_SEL_TG_SW_RGB       0
#define CAM_FMT_SEL_TG_SW_BGR       2
//pixel id
#define CAM_PIX_ID_B    0
#define CAM_PIX_ID_Gb   1
#define CAM_PIX_ID_Gr   2
#define CAM_PIX_ID_R    3
//


//#define WDMA_PORT (0)
//#define WROT_PORT (1)
//#define IMG2O_PORT (2)
/*/////////////////////////////////////////////////////////////////////////////
    TODO: temp from tg_common
  /////////////////////////////////////////////////////////////////////////////*/
typedef int (*default_func)(void);
struct stCam_Id_Enable {
    int id;
    int en_bit;
};
//
#define CAM_ISP_SETTING_DONT_CARE 0
//CDP
#define CAM_CDP_CDRZ_8_TAP 0
#define CAM_CDP_CDRZ_N_TAP 1
#define CAM_CDP_CDRZ_4N_TAP 2
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
#define CAM_ISP_CQ_NONE (-1)
#define CAM_ISP_CQ0     ISP_DRV_CQ0
#define CAM_ISP_CQ0B    ISP_DRV_CQ0B
#define CAM_ISP_CQ0C    ISP_DRV_CQ0C
#define CAM_ISP_CQ1     ISP_DRV_CQ01
#define CAM_ISP_CQ2     ISP_DRV_CQ02
#define CAM_ISP_CQ3     ISP_DRV_CQ03
//
#define CAM_CQ_SINGLE_IMMEDIATE_TRIGGER CQ_SINGLE_IMMEDIATE_TRIGGER
#define CAM_CQ_SINGLE_EVENT_TRIGGER     CQ_SINGLE_EVENT_TRIGGER
#define CAM_CQ_CONTINUOUS_EVENT_TRIGGER CQ_CONTINUOUS_EVENT_TRIGGER
//
#define CAM_CQ_TRIG_BY_START        CQ_TRIG_BY_START
#define CAM_CQ_TRIG_BY_PASS1_DONE   CQ_TRIG_BY_PASS1_DONE
#define CAM_CQ_TRIG_BY_PASS2_DONE   CQ_TRIG_BY_PASS2_DONE
#define CAM_CQ_TRIG_BY_IMGO_DONE    CQ_TRIG_BY_IMGO_DONE
#define CAM_CQ_TRIG_BY_IMG2O_DONE   CQ_TRIG_BY_IMG2O_DONE
#define CAM_CQ_TRIG_BY_NONE         (-1)

//
#define CAM_ISP_PIXEL_BYTE_FP 2
//isp line buffer
#define CAM_ISP_MAX_LINE_BUFFER_IN_PIXEL (2304)
//
#define CAM_CDP_PRZ_CONN_TO_DISPO 0
#define CAM_CDP_PRZ_CONN_TO_VIDO  1
//interrupt timeout time
#define CAM_INT_WAIT_TIMEOUT_MS 2000
#define CAM_INT_PASS2_WAIT_TIMEOUT_MS 2000





class TdriBnrCfg
{
public:
    int bpc_en; ///< bpc enable
    //int bpc_tbl_en;
    //int bpc_tbl_size;/* bad pixel table width */
};

class TdriLscCfg
{
public:
    int sdblk_width;       ///< sdblk_width block width in x direction
    int sdblk_xnum;        ///< sdblk_xnum x directional number of block
    int sdblk_last_width;  ///< sdblk_last_width last block width
    int sdblk_height;      ///< sdblk_height block height in y direction
    int sdblk_ynum;        ///< sdblk_ynum block width in x direction
    int sdblk_last_height;///< sdblk_last_height last block height
};

class TdriNbcCfg
{
public:
    //int anr_en;
    int anr_eny;       ///< anr_eny Enable Y ANR
    int anr_enc;       ///< anr_enc Enable C ANR
    int anr_iir_mode;  ///< anr_iir_mode In tdri mode, no IIR
    int anr_scale_mode;///< anr_scale_mode Different in tdri mode and frame mode
};

/**
 * @brief SEEE configuration for tdri
 */

class TdriSeeeCfg
{
public:
    int se_edge;  ///< se_edge Select source of edge enhancement
    //int usm_over_shrink_en;
};

class TdriImgoCfg
{
public:
    int imgo_stride;  ///< imgo stride
    int imgo_crop_en; ///< imgo crop enable
    int imgo_xoffset; ///< imgo offset in x direction
    int imgo_yoffset; ///< imgo offset in y direction
    int imgo_xsize;   ///< imgo x size
    int imgo_ysize;   ///< imgo y size
    int imgo_mux_en;  ///< imgo mux enable
    int imgo_mux;     ///< imgo mux
};

class TdriCdrzCfg
{
public:
    int cdrz_input_crop_width;           ///< cdrz input cropping width
    int cdrz_input_crop_height;          ///< cdrz input cropping height
    int cdrz_output_width;               ///< cdrz output width
    int cdrz_output_height;              ///< cdrz output height
    int cdrz_horizontal_integer_offset;  ///< integer offset in horizontal direction for luma
    int cdrz_horizontal_subpixel_offset;///< sub-pixel offset in horizontal direction for luma
    int cdrz_vertical_integer_offset;    ///<  integer offset in vertical direction for luma
    int cdrz_vertical_subpixel_offset;   ///< sub-pixel offset in vertical direction for luma
    int cdrz_horizontal_luma_algorithm;  ///< horizontal luma algorithm
    int cdrz_vertical_luma_algorithm;    ///< vertical luma algorithm
    int cdrz_horizontal_coeff_step;      ///< horizontal coefficience step
    int cdrz_vertical_coeff_step;        ///< vertical coefficience step
};

class TdriImg2oCfg
{
public:
    int img2o_stride;  ///< img2o stride
    int img2o_crop_en; ///< img2o crop enable
    int img2o_xoffset; ///< img2o offset in x direction
    int img2o_yoffset; ///< img2o offset in y direction
    int img2o_xsize;   ///< img2o x size
    int img2o_ysize;   ///< img2o y size
    int img2o_mux_en;  ///< img2o mux enable
    int img2o_mux;     ///< img2o mux
};

class TdriCfaCfg
{
public:
    int bayer_bypass; ///< Bypass bayer to rgb process
};

class TdriTopCfg
{
public:
    int scenario;    ///< Isp pipeline scenario
    int mode;        ///< Isp pipeline sub mode
    int debug_sel;   ///< Debug mode selection for all scenarios
    int pixel_id;    ///< Bayer sequence , it is for RAW and RGB module
    int cam_in_fmt;  ///< Pass2 path input format
    int tcm_load_en; ///< TDR manual setting enable
    int ctl_extension_en;///< 0: normal,suggested
    int rsp_en;       ///< Resample tile enable, for direct link MDP use
    int mdp_crop_en;  ///< MDP corp enable
    int imgi_en;      ///< enable switch of IMGI
    int lsci_en;     ///< enable switch of LSCI
    int unp_en;      ///< enable switch of unpacked
    int bnr_en;      ///< enable switch of BNR
    int lsc_en;      ///< enable switch of LSC
    int sl2_en;      ///< enable switch of SL2
    int c24_en;     ///< enable switch of c24
    int cfa_en;     ///< enable switch of cfa
    int c42_en;     ///< enable switch of c42
    int nbc_en;     ///< enable switch of nbc
    int seee_en;    ///< enable switch of SEEE
    int imgo_en;    ///< enable switch of IMGO
    int img2o_en;   ///< enable switch of IMG2O
    int cdrz_en;   ///< enable switch of CDRZ
    int mdp_sel;   ///< 0 : from CDRZ 1: from YUV
    int interlace_mode;///< doesn't use it
    TdriTopCfg()
     : scenario(0)
     , mode(0)
     , debug_sel(0)
     , pixel_id(0)
     , cam_in_fmt(0)
     , tcm_load_en(0)
     , ctl_extension_en(0)
     , rsp_en(0)
     , mdp_crop_en(0)
     , imgi_en(0)
     , lsci_en(0)
     , unp_en(0)
     , bnr_en(0)
     , lsc_en(0)
     , sl2_en(0)
     , c24_en(0)
     , cfa_en(0)
     , c42_en(0)
     , nbc_en(0)
     , seee_en(0)
     , imgo_en(0)
     , img2o_en(0)
     , cdrz_en(0)
     , mdp_sel(0)
     , interlace_mode(0)
     {}
};


/**
 * @brief software perform configuration for TDRI
 */

class TdriPerformCfg //TdriswCfg
{
public:
    int log_en;     ///< doesn't use it
    int src_width;  ///< width of source image
    int src_height;///< height of source image
    int tpipeWidth; ///< width of tpipe
    int tpipeHeight;///< height of tpipe
    int irqMode;    ///< interrupt mode
};

/**
 * @brief software DMA configuration for tDRI
 */

class TdriDMACfg
{
public:
    int srcWidth;  ///< width of source image
    int srcHeight; ///< height of source image
    int tpipeTabSize; ///< doesn't use it
    int baseVa;       ///< base virtual address
};

/**
 * @brief IMGI configuration
 */

class TdriRingInDMACfg
{
public:
    int stride;  ///< image stride of IMGI
   // int ring_en;//need to remove
   // int ring_size;//need to remode
};

class TdriLsciCfg
{
public:
    int lsci_stride;
};


class Tdrisl2Cfg //TdriswCfg
{
public:
      int sl2_hrz_comp; ///< SL2_HRZ_COMP = f = 1/HRZ_Scaling, Q0.3.11
};


class TdriTuningCfg
{
public:
    TdriBnrCfg bnr; ///< BNR configuration for tdri
    TdriLscCfg lsc; ///< LSC configuration for tdri
    TdriLsciCfg lsci; ///< LSCI configuration for tdri
    TdriNbcCfg nbc; ///< NBC configuration for tdri
    TdriSeeeCfg seee;///< SEEE configuration for tdri
    TdriCfaCfg cfa; ///< CFA configuration for tdri
    Tdrisl2Cfg sl2; ///< SL2 configuration for tdri

};

/**
 * @brief tdri driver configuration
 */

class TdriDrvCfg
{
public:
    //enable table
//    TdriUpdateCfg updateTdri;       ///< update type for TDRI
    TdriTopCfg top;                 ///< top configuration for tdri
    TdriDMACfg tdriCfg;             ///< software DMA configuration for TDRI
    TdriPerformCfg  tdriPerformCfg;///< software perform configuration for TDRI

    TdriRingInDMACfg imgi;          ///< IMGI configuration for tdri
    TdriLsciCfg lsci;        ///< LSCI stride for tdri
    TdriCdrzCfg cdrz;              ///< CDRZ configuration for tdri
    TdriImg2oCfg img2o;            ///< IMG2O configuration for tdri
    TdriImgoCfg imgo;              ///< IMGO configuration for tdri

    TdriTuningCfg tuningFunc;      ///< isp tuning configuration for tdri
};


class IspImageioPackage
{
public:
    MUINT32 cq;
    MINT32 subMode;
    MBOOL isApplyTuning;
    MUINT32 enFgMode;
    MUINT32 finalEn1;
    MUINT32 finalEn2;
    // for sl2
    MFLOAT hrzRatio;
    MUINT32 hrzCropX;
    MUINT32 hrzCropY;
};


/**
*@brief  ISP enable configure struct
*/
struct stIspTopEnTbl{
    unsigned int enable1;
    unsigned int enable2;
    unsigned int dma;
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
};

/**
*@brief  ISP Top mapping table
*/
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


/**
*@brief  ISP interrupt configure struct
*/
struct stIspTopINT{
    unsigned int int_en;
    unsigned int int_status;
    unsigned int dma_int;
    unsigned int intb_en;
    unsigned int intb_status;
    unsigned int dmab_int;
    unsigned int intc_en;
    unsigned int intc_status;
    unsigned int dmac_int;
    unsigned int int_statusx;
    unsigned int dma_intx;
};

/**
*@brief  ISP format select configure struct
*/
struct stIspTopFmtSel {
    union {
        struct {
            unsigned int scenario       :3;
            unsigned int dummy0         :1;
            unsigned int sub_mode       :3;
            unsigned int dummy1         :1;
            unsigned int cam_in_fmt     :4;
            unsigned int cam_out_fmt    :4;
            unsigned int tg1_fmt        :3;
            unsigned int dummy2         :1;
            unsigned int dummy3        :4;
            unsigned int two_pix        :1;
            unsigned int dummy4       :1;
            unsigned int tg1_sw         :2;
            unsigned int dummy5         :4;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top select configure struct
*/
struct stIspTopSel {
    union {
        struct {
            unsigned int dummy0        :2;
            unsigned int cq0_cont   :1;
            unsigned int cq0b_cont         :1;
            unsigned int pass1_db_en        :1;
            unsigned int pass2_db_en        :1;
            unsigned int tdr_sel            :1;
            unsigned int cq0c_imgo_sel             :1;
            unsigned int dummy1             :3;
            unsigned int cqob_sel            :1;
            unsigned int deb_sel            :3;
            unsigned int eis_sel        :1;
            unsigned int eis_raw_sel        :1;
            unsigned int cq0_mode           :1;
            unsigned int dummy2   :4;
            unsigned int cq0c_img2o_sel        :1;
            unsigned int disp_vid_sel       :1;
            unsigned int dummy3     :1;
            unsigned int cq0b_mode       :1;
            unsigned int dummy4             :5;
            unsigned int cq1_int_sel          :1;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top mux select configure struct
*/
struct stIspTopMuxSel {
    union {
        struct {
            unsigned int rsv_0        : 1;
            unsigned int rsv_1        : 1;
            unsigned int UNP_SEL        : 1;
            unsigned int AA_SEL         : 1;
            unsigned int rsv_4        : 1;
            unsigned int rsv_5        : 1;
            unsigned int SGG_SEL        : 2;
            unsigned int BIN_SEL        : 1;
            unsigned int rsv_9          : 1;
            unsigned int C02_SEL        : 2;
            unsigned int G2G_SEL        : 1;
            unsigned int rsv_13        : 1;
            unsigned int rsv_14        : 1;
            unsigned int rsv_15        : 1;
            unsigned int rsv_16        : 1;
            unsigned int rsv_17        : 1;
            unsigned int rsv_18        : 1;
            unsigned int UNP_SEL_EN     : 1;
            unsigned int AA_SEL_EN      : 1;
            unsigned int rsv_21        : 1;
            unsigned int SGG_SEL_EN     : 1;
            unsigned int BIN_SEL_EN     : 1;
            unsigned int SOF_SEL_EN     : 1;
            unsigned int C02_SEL_EN     : 1;
            unsigned int G2G_SEL_EN     : 1;
            unsigned int rsv_27        : 1;
            unsigned int rsv_28        : 1;
            unsigned int rsv_29        : 1;
            unsigned int rsv_30        : 1;
            unsigned int rsv_31         : 1;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top mux select 2 configure struct
*/
struct stIspTopMuxSel2 {
    union {
        struct {
            unsigned int CCL_SEL            : 2;
            unsigned int BIN_OUT_SEL        : 2;
            unsigned int IMGO_MUX           : 1;
            unsigned int rsv_5        : 1;
            unsigned int IMG2O_MUX          : 1;
            unsigned int rsv_7        : 1;
            unsigned int PASS1_DONE_MUX     : 5;
            unsigned int PASS2_DONE_MUX     : 5;
            unsigned int CCL_SEL_EN         : 1;
            unsigned int BIN_OUT_SEL_EN     : 1;
            unsigned int IMGO_MUX_EN        : 1;
            unsigned int IMG2O_MUX_EN       : 1;
            unsigned int rsv_22        : 1;
            unsigned int rsv_23        : 1;
            unsigned int rsv_24        : 1;
            unsigned int IMGI_MUX_EN        : 1;
            unsigned int rsv_26        : 1;
            unsigned int rsv_27        : 1;
            unsigned int LSCI_SOF_SEL       : 1;
            unsigned int LSCI_SOF_SEL_EN    : 1;
            unsigned int PASS1_DONE_MUX_EN  : 1;
            unsigned int PASS2_DONE_MUX_EN  : 1;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top sram mux configure struct
*/
struct stIspTopSramMuxCfg {
    union {
        struct {
            unsigned int SRAM_MUX_SCENARIO  : 3;
            unsigned int rsv_3              : 1;
            unsigned int SRAM_MUX_MODE      : 3;
            unsigned int SRAM_MUX_TPIPE     : 1;
            unsigned int SRAM_MUX_SET_EN    : 1;
            unsigned int rsv_9              : 1;
            unsigned int IMGO_SOF_SEL       : 1;
            unsigned int rsv_11        : 1;
            unsigned int ESFKO_SOF_SEL      : 1;
            unsigned int AAO_SOF_SEL        : 1;
            unsigned int RGB_SOF_SEL        : 1;
            unsigned int rsv_15             : 1;
            unsigned int rsv_16        : 1;
            unsigned int ESFKO_SOF_SEL_EN   : 1;
            unsigned int AAO_SOF_SEL_EN     : 1;
            unsigned int RGB_SOF_SEL_EN     : 1;
            unsigned int rsv_20        : 1;
            unsigned int rsv_21        : 1;
            unsigned int rsv_22        : 1;
            unsigned int rsv_23        : 1;
            unsigned int PREGAIN_SEL        : 1;
            unsigned int rsv_25        : 1;
            unsigned int rsv_26        : 1;
            unsigned int rsv_27        : 1;
            unsigned int SGG_HRZ_SEL        : 1;
            unsigned int rsv_29             : 1;
            unsigned int IMG2O_SOF_SEL      : 2;
        }bit_field;
        unsigned int reg_val;
    };
};

struct stIspRegPixId {
    union {
        struct {
            unsigned int PIX_ID                    : 2;
            unsigned int TG_PIX_ID                 : 2;
            unsigned int TG_PIX_ID_EN              : 1;
            unsigned int BPC_TILE_EDGE_SEL         : 1;
            unsigned int BPC_TILE_EDGE_SEL_EN      : 1;
            unsigned int rsv_7                     : 9;
            unsigned int CDP_EDGE                  : 4;
            unsigned int MDP_SEL                   : 1;
            unsigned int CTL_EXTENSION_EN          : 1;
            unsigned int LSC_EARLY_SOF_EN          : 1;
            unsigned int ISP_FRAME_DONE_SEL        : 1;
            unsigned int CQ0A_MASK_CQ0B_EN         : 1;
            unsigned int CQ0A_MASK_CQ0C_EN         : 1;
            unsigned int rsv_26                    : 6;
        } bit_field;
        unsigned int reg_val;
    };
};


/**
*@brief  ISP top enable class
*/
class IspEnFunc
{
public:
    struct stIspTopEnTbl        en_Top;
public:

    /**
      *@brief  Constructor
      */
    IspEnFunc()
    {en_Top.enable1 = 0;en_Top.enable2 = 0;en_Top.dma = 0;}
/*    {en_Raw.u.pipe=0;en_Rgb.u.pipe=0;en_Yuv.u.pipe=0;
     en_Cdp.u.pipe=0;en_DMA.u.pipe=0;};*/
};

/**
*@brief  ISP top interrupt class
*/
class IspEnInt
{
public:
    struct stIspTopINT en_Int;
public:

    /**
      *@brief  Constructor
      */
    IspEnInt()
    {en_Int.int_en=0;en_Int.intb_en=0;en_Int.intc_en=0;en_Int.dma_int=0;en_Int.dmab_int=0;en_Int.dmac_int=0;}
};

/**
*@brief  ISP data source enum
*/
typedef enum  _IspInDataSrc_
{
    ISP_IN_DATA_SRC_SENSOR = 0,
    ISP_IN_DATA_SRC_MEM
}IspInDataSrc;

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
#if 1
        /**
             *@brief  Constructor
             */
        IspDrv_B():
            m_pPhyIspDrv_bak(NULL),
            m_pPhyIspReg_bak(NULL),
            m_pP2CdrzDrv(NULL),
            m_pVirtIspDrv(NULL),
            m_pVirtIspReg(NULL)
            {}

        /**
             *@brief  Destructor
             */
        virtual ~IspDrv_B(){}
    public:
        static NSIspDrv_FrmB::IspDrv*      m_pIspDrv;
        static isp_reg_t*   m_pIspReg;
        static NSIspDrv_FrmB::IspDrv*      m_pPhyIspDrv;
        static NSIspDrv_FrmB::IspDrv*      m_pP2IspDrv;
        static NSIspDrv_FrmB::IspDrv*      m_pP1IspDrv;
        static NSIspDrv_FrmB::IspDrv*      m_pP1IspDrvD;
    public:
        /*** virtual isp driver for CQ ***/
        NSIspDrv_FrmB::IspDrv**         m_pVirtIspDrv;
        isp_reg_t**      m_pVirtIspReg;
        //static isp_reg_t*   m_pCurCQIspReg;
        static isp_reg_t*   m_pP1CurCQDIspReg;
        NSIspDrv_FrmB::IspDrv*         m_pPhyIspDrv_bak;
        isp_reg_t*      m_pPhyIspReg_bak;
        /*** crz driver ***/
        CdpDrv*         m_pP2CdrzDrv;      ///< cdp driver
        //TpipeDrv*       m_pTpipeDrv;    ///< tdri driver
        /*imem*/
        IMemDrv*        m_pIMemDrv ;
        IspEventThread* m_pIspEventThread;  ///< ispEventThread
    public:
        static default_func default_setting_function[32];
#endif

        /**
             *@brief  hrz configure
             *@param[in] iHeight : input height
             *@param[in] resize : resize
             *@param[in] oSize : ouput height
             */
        static MINT32 cam_isp_hrz_cfg(IspDrv* pCurDrv,MINT32 iHeight,MINT32 resize,MINT32 oSize);
        static MINT32 cam_isp_cfa_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_g2g_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_dgm_cfg(void);
        static MINT32 cam_isp_ggm_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_c24_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_g2c_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_obc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_bnr_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_lsc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_sl2_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_pgn_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_ccl_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_nbc_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_pca_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_seee_cfg(IspImageioPackage imageioPackage, IspTuningPackage tuningPackage);
        static MINT32 cam_isp_c42_cfg(void);
};


/*/////////////////////////////////////////////////////////////////////////////
    Tuning Function Control
/////////////////////////////////////////////////////////////////////////////*/
#define CHECK_TUNING_EN1_BITS   9
#define CHECK_TUNING_EN2_BITS   4

const stIspTopEnTbl gIspTurningTopTag_cc[ISP_SUB_MODE_MAX]
={   {0x07E00900, 0x0000001D, 0x00000000  },  // N3D_IC_RAW(6)
     {0x07C009a8, 0x0000001D, 0x00000000  },  // N3D_IC_YUV
     {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
     {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
     {0x00000000, 0x00000000, 0x00000000  }
}; // DUMMY

const stIspTopEnTbl gIspTurningFieldTag_cc[ISP_SUB_MODE_MAX]
={   {0x07E00900, 0x0000001D, 0x00000000  },  // N3D_IC_RAW(6)
     {0x07C009a8, 0x0000001D, 0x00000000  },  // N3D_IC_YUV
     {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
     {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
     {0x00000000, 0x00000000, 0x00000000  }
}; // DUMMY

const stIspTopEnTbl gIspTurningTopTag_ip[ISP_SUB_MODE_MAX]
={   {0x07E009a8, 0x0000001D, 0x00000002  },  // IP_RAW(3)
     {0x07C009a8, 0x0000001D, 0x00000002  },  // IP_YUV
     {0x07C008a8, 0x0000001D, 0x00000002  },  // IP_RGBW
     {0x07C008a8, 0x0000001D, 0x00000002  },  // IP_RGB_LOAD
     {0x07C008a8, 0x0000001D, 0x00000002  }
}; // IP_MFB

const stIspTopEnTbl gIspTurningFieldTag_ip[ISP_SUB_MODE_MAX]
={   {0x07E009a8, 0x0000001D, 0x00000002  },  // IP_RAW(3)
     {0x07C009a8, 0x0000001D, 0x00000002  },  // IP_YUV
     {0x07C008a8, 0x0000001D, 0x00000002  },  // IP_RGBW
     {0x07C008a8, 0x0000001D, 0x00000002  },  // IP_RGB_LOAD
     {0x07C008a8, 0x0000001D, 0x00000002  }
}; // IP_MFB

const IspTopModuleMappingTable gIspModuleMappingTableEn1[CHECK_TUNING_EN1_BITS]
={ {CAM_CTL_EN1_OB_EN , CAM_CTL_DMA_EN_NONE_EN, eIspModuleRaw, eIspTuningMgrFunc_Obc,   eIspTuningMgrFunc_Null, CAM_ISP_OBC,              CAM_DUMMY_,             CAM_DUMMY_,  IspDrv_B::cam_isp_obc_cfg},
   {CAM_CTL_EN1_LSC_EN, CAM_CTL_DMA_EN_LSCI_EN, eIspModuleRaw, eIspTuningMgrFunc_Bnr,   eIspTuningMgrFunc_Null, CAM_ISP_BNR,              CAM_DUMMY_,             CAM_DUMMY_,  IspDrv_B::cam_isp_bnr_cfg},
   {CAM_CTL_EN1_BNR_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleRaw, eIspTuningMgrFunc_Lsc,   eIspTuningMgrFunc_Null, CAM_ISP_LSC,              CAM_DUMMY_,             CAM_DMA_LSCI,IspDrv_B::cam_isp_lsc_cfg},
   {CAM_CTL_EN1_SL2_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleRgb, eIspTuningMgrFunc_Sl2,   eIspTuningMgrFunc_Null, CAM_CDP_SL2_FEATUREIO,    CAM_DUMMY_,             CAM_DUMMY_,  IspDrv_B::cam_isp_sl2_cfg},
   {CAM_CTL_EN1_PGN_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleRaw, eIspTuningMgrFunc_Pgn,   eIspTuningMgrFunc_Null, CAM_ISP_PGN,              CAM_DUMMY_,             CAM_DUMMY_,  IspDrv_B::cam_isp_pgn_cfg},
   {CAM_CTL_EN1_CFA_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleRgb, eIspTuningMgrFunc_Cfa,   eIspTuningMgrFunc_Null, CAM_ISP_CFA,              CAM_DUMMY_,             CAM_DUMMY_,  IspDrv_B::cam_isp_cfa_cfg},
   {CAM_CTL_EN1_CCL_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleRgb, eIspTuningMgrFunc_Ccl,   eIspTuningMgrFunc_Null, CAM_ISP_CCL,              CAM_DUMMY_,             CAM_DUMMY_,  IspDrv_B::cam_isp_ccl_cfg},
   {CAM_CTL_EN1_G2G_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleRgb, eIspTuningMgrFunc_G2g,   eIspTuningMgrFunc_Null, CAM_ISP_G2G,              CAM_DUMMY_,             CAM_DUMMY_,  IspDrv_B::cam_isp_g2g_cfg},
   {CAM_CTL_EN1_GGM_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleRgb, eIspTuningMgrFunc_Ggmrb, eIspTuningMgrFunc_Ggmg, CAM_ISP_GGMRB,            CAM_ISP_GGMG,           CAM_DUMMY_,  IspDrv_B::cam_isp_ggm_cfg},
 };

const IspTopModuleMappingTable gIspModuleMappingTableEn2[CHECK_TUNING_EN2_BITS]
={ {CAM_CTL_EN2_G2C_EN , CAM_CTL_DMA_EN_NONE_EN, eIspModuleYuv, eIspTuningMgrFunc_G2c,  eIspTuningMgrFunc_Null, CAM_ISP_G2C,  CAM_DUMMY_,      CAM_DUMMY_, IspDrv_B::cam_isp_g2c_cfg},
   {CAM_CTL_EN2_NBC_EN,  CAM_CTL_DMA_EN_NONE_EN, eIspModuleYuv, eIspTuningMgrFunc_Nbc,  eIspTuningMgrFunc_Null, CAM_ISP_NBC,  CAM_DUMMY_,      CAM_DUMMY_, IspDrv_B::cam_isp_nbc_cfg},
   {CAM_CTL_EN2_PCA_EN,  CAM_CTL_DMA_EN_NONE_EN, eIspModuleYuv, eIspTuningMgrFunc_Pca,  eIspTuningMgrFunc_Null, CAM_ISP_PCA,  CAM_ISP_PCA_CON, CAM_DUMMY_, IspDrv_B::cam_isp_pca_cfg},
   {CAM_CTL_EN2_SEEE_EN, CAM_CTL_DMA_EN_NONE_EN, eIspModuleYuv, eIspTuningMgrFunc_Seee, eIspTuningMgrFunc_Null, CAM_ISP_SEEE, CAM_DUMMY_,      CAM_DUMMY_, IspDrv_B::cam_isp_seee_cfg},
 };



/*/////////////////////////////////////////////////////////////////////////////
    CommandQ control
/////////////////////////////////////////////////////////////////////////////*/

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
//    #define CAM_CTL_EN1_FOR_TURN    0x00D008a8// TEST_MDPjust for RAW_POSTPROC 0x00C008a8 //SL TEST_MDP remove CLEAR GGM 0x07C008a8      //0x4004
//    #define CAM_CTL_EN2_FOR_TURN    0x0000003C      //0x4008
//    #define CAM_CTL_DMA_FOR_TURN    0x00001102      //0x400C
    //
    #define CAM_CTL_EN1_FOR_ISP     0x38303200      //0x4004 //org
//    #define CAM_CTL_EN1_FOR_ISP     0x3D203200//SL TEST_MDP 0x3D303200      //0x4004
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
    #define CAM_CTL_EN2_FOR_CDP     0x43A40000      //0x4008  // NR3D control by isp
//    #define CAM_CTL_EN2_FOR_CDP     0x43A60020      //0x4008

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

/**
*@brief  Isp Driver Shell class
*/
class IspDrvShell:virtual public IspDrv_B
{

    protected:
        IspDrvShell():m_trigSsrc(-1){}
        /**
             *@brief  Destructor
             */
        virtual             ~IspDrvShell() {};
    public:

        /**
             *@brief  Create Isp Driver Shell class object
             *@param[in] eScenarioID : scenario ID enum
             */
        static IspDrvShell* createInstance(NSImageio_FrmB::NSIspio_FrmB::EDrvScenario eScenarioID = NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC);


        /**
             *@brief  Destory Isp Driver Shell class object
             */
        virtual void        destroyInstance(void) = 0;

        /**
             *@brief  Initialize
             */
        virtual MBOOL       init(const char* userName="") = 0;

#if 1

        /**
             *@brief  Uninitialize
             */
        virtual MBOOL       uninit(const char* userName="") = 0;

        /**
             *@brief  Get ISP physical driver
             */
        virtual NSIspDrv_FrmB::IspDrv*     getPhyIspDrv() = 0;
        virtual MBOOL       updateBurstQueueNum(MINT32 burstQNum)=0;

        friend class IspFunction_B;
#endif
    private:


        /**
             *@brief  phy<->virt ISP switch
             */
        //virtual MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 dupCqIdx) = 0;
        virtual MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 p1CqD,MINT32 burstQIdx,MINT32 p2DupCqIdx) = 0;
    public://commandQ operation
        virtual NSIspDrv_FrmB::IspDrv*      ispDrvGetCQIspDrv(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx)  = 0;
        /**
             *@brief  Command queue configure
             *@param[in] cq : cq enum
             */
        virtual MBOOL       cam_cq_cfg(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx,MINT32 isP2IspOnly=0) = 0;

        /**
             *@brief  Add module to command queue
             *@param[in] cq : cq enum
             *@param[in] moduleId : module enum
             */
        virtual MBOOL       cqAddModule(NSIspDrv_FrmB::ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, NSIspDrv_FrmB::CAM_MODULE_ENUM moduleId) = 0;


        /**
             *@brief  Delete module to command queue
             *@param[in] cq : cq enum
             *@param[in] moduleId : module enum
             */
        virtual MBOOL cqDelModule(NSIspDrv_FrmB::ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, NSIspDrv_FrmB::CAM_MODULE_ENUM moduleId) = 0;

        /**
             *@brief  Set command queue trigger mode
             *@param[in] cq : cq
             *@param[in] moduleId : trigger mode
             *@param[in] moduleId : trigger source
             */
        virtual MBOOL       setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src) = 0;
        virtual int         getCqModuleInfo(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId) =0;

        mutable Mutex   mLock;
        mutable Mutex   gLock;  // for multi-thread
        mutable Mutex   gPass1Lock;  // global lock for pass1
        mutable Mutex   gPass1_dLock; //global lock for pass1_d,camsv,camsv2
        mutable Mutex   gPass2Lock;  // global lock for pass2
        mutable Mutex   gPass2DequeLock;  // global dequeue lock for pass2
        //mutable Mutex   g_cam_path_Lock;  // for dual isp
        //
        MINT32 m_trigSsrc;

};

/**
*@brief  Isp Function basicl class
*/
class IspFunction_B
{
private:
    //isp top camera base address
    static unsigned long    m_Isp_Top_Reg_Base_Addr;   //maybe virtual address
    //function base address
    unsigned long           m_Reg_Base_Addr;          //maybe virtual address
    mutable Mutex           mLock;
protected://isp driver operation
    static NSIspDrv_FrmB::IspDrv*          m_pIspDrv;
    static isp_reg_t*       m_pIspReg;
    static NSIspDrv_FrmB::IspDrv*          m_pPhyIspDrv;
    static NSIspDrv_FrmB::IspDrv*          m_pP1IspDrv;
    static NSIspDrv_FrmB::IspDrv*          m_pP2IspDrv;
    static isp_reg_t*       m_pPhyIspReg;
     isp_reg_t*       m_pP1CurCQDIspReg;
public://isp driver operation
    int                     bypass;
    static IspDrvShell*     m_pIspDrvShell;
    static MUINT32          m_fps;          //for dma fifo cfg
public:

    /**
      *@brief  Constructor
      */
    IspFunction_B(): m_Reg_Base_Addr(0),bypass(0){};

    /**
      *@brief  Destructor
      */
    virtual ~IspFunction_B(){};
public:

    /**
      *@brief  Check busy
      */
    virtual int checkBusy( unsigned long* param ) = 0;

    /**
      *@brief  Get ID
      *@return
      *- ID
      */
    virtual unsigned long id( void ) = 0;

    /**
      *@brief Name string of module
      */
    virtual const char* name_Str( void ) = 0;

    /**
      *@brief  Phy register base address, usually a fixed value
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) = 0;

    /**
      *@brief  Register range
      */
    virtual unsigned long reg_Range( void ) = 0;

    /**
      *@brief  Check module is bypass or not
      */
    virtual int is_bypass( void ) {return bypass;}

    /**
      *@brief  Decimal ID number
      */
    unsigned long           dec_Id( void );

    /**
      *@brief  Get isp top register base addr
      *@return
      *- isp top register base addr
      */
    static unsigned long isp_Top_Reg_Base_Addr( void ) {   return m_Isp_Top_Reg_Base_Addr;   }

    /**
      *@brief  Get isp top register physical base addr
      *@return
      *- isp top register physical base addr
      */
    static unsigned long isp_Top_Reg_Base_Addr_Phy( void ) {   return MT6589_ISP_TOP_BASE;  }//Phy reg base address, usually a fixed value

    /**
      *@brief  Get isp top register range
      *@return
      *- isp top register range
      */
    static unsigned long isp_Top_Reg_Range( void )  {   return MT6589_ISP_TOP_REG_RANGE;  }

    /**
      *@brief  Get register base address
      *@return
      *- register base address
      */
    unsigned long reg_Base_Addr( void )  {   return m_Reg_Base_Addr;   }

    /**
      *@brief  Remap top register base address
      */
    static void remap_Top_Reg_Base_Addr( unsigned long new_addr )  {   m_Isp_Top_Reg_Base_Addr = new_addr;}

    /**
      *@brief  Remap register base address
      */
    void  remap_Reg_Base_Addr( unsigned long new_addr )  {   m_Reg_Base_Addr = new_addr; }
public: //isp driver operation

    /**
      *@brief  Set ISP driver shell
      */
    static  void            setIspDrvShell(IspDrvShell* pIspDrvShell){m_pIspDrvShell = pIspDrvShell;m_pPhyIspDrv = pIspDrvShell->getPhyIspDrv();
                                                                      m_pPhyIspDrv = pIspDrvShell->getPhyIspDrv();}

    /**
      *@brief  Wait irq
      *@param[in] WaitIrq : ISP_DRV_WAIT_IRQ_STRUCT
      */
    inline int waitIrq( NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT* WaitIrq ){return (int)m_pPhyIspDrv->waitIrq( WaitIrq );}
    /**
      *@brief  Write register
      *@param[in] offset : register offset
      *@param[in] value : write value
      */
    inline int writeReg( unsigned long offset, unsigned long value ){return (int)m_pPhyIspDrv->writeReg(offset,value);}

    /**
      *@brief  Read register
      *@param[in] offset : register offset
      *@return
      *- register value
      */
    inline unsigned long readReg( unsigned long offset ){return (unsigned long)m_pPhyIspDrv->readReg(offset);}

    /**
      *@brief  Read irq
      *@param[in] pReadIrq : ISP_DRV_READ_IRQ_STRUCT
      */
    inline int readIrq(NSIspDrv_FrmB::ISP_DRV_READ_IRQ_STRUCT *pReadIrq){return (int)m_pPhyIspDrv->readIrq(pReadIrq);}

    /**
      *@brief  Check irq
      *@param[in] CheckIrq : ISP_DRV_CHECK_IRQ_STRUCT
      */
    inline int checkIrq(NSIspDrv_FrmB::ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){return (int)m_pPhyIspDrv->checkIrq(CheckIrq);}

    /**
      *@brief  Clear irq
      *@param[in] ClearIrq : ISP_DRV_CLEAR_IRQ_STRUCT
      */
    inline int clearIrq(NSIspDrv_FrmB::ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){return (int)m_pPhyIspDrv->clearIrq(ClearIrq);}

    /**
      *@brief  Dump register
      */
    inline int dumpRegister( int mode ){return (int)m_pPhyIspDrv->dumpReg();}

    mutable Mutex           queHwLock;
    mutable Mutex           queSwLock;
    MBOOL                   getEnTuningTag(MBOOL isTopEngine, ISP_DRV_CQ_ENUM cq, MUINT32 drvScenario, MUINT32 subMode, MUINT32 &tuningEn1Tag, MUINT32 &tuningEn2Tag, MUINT32 &tuningDmaTag);

    /**
      *@brief  Check if virtual isp driver or not
      */
    //MBOOL                 ispDrvSwitch2Virtual(MINT32 cq,MINT32 p2dupCqIdx);
    MBOOL                   ispDrvSwitch2Virtual(MINT32 cq,MINT32 p1CqD,MINT32 burstQIdx,MINT32 p2DupCqIdx);//only used for TopCTrl & RawCtrl

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

/**
*@brief  Input DMA basic class
*/
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
    MUINT32 sceID;
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
private:

public:
    DMAI_B():dupCqIdx(0),burstQueIdx(0),CQ(CAM_ISP_CQ_NONE),sceID(0),isP2IspOnly(0),p2MdpSrcFmt(eImgFmt_YV12),p2MdpSrcW(0),p2MdpSrcH(0),p2MdpSrcYStride(0),p2MdpSrcUVStride(0),p2MdpSrcSize(0),p2MdpSrcCSize(0),p2MdpSrcVSize(0),cqTrigSrc(-1),cqTrigSrc_D(-1)
        {dma_cfg.pixel_byte = 1;};
public: /*[IspFunction_B]*/

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}
private: /*[IspFunction_B]*/

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void );

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void );
public: /*[DMAI_B]*/

private: /*[DMAI_B]*/

};

/**
*@brief  Output DMA basic class
*/
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

    IspDMACfg   dma_cfg;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int CQ;

/*
    IspMemBuffer    img_buf;
    IspSize         size;
    IspRect         crop;
    int             pixel_byte;
*/
private:

public:
    DMAO_B():dupCqIdx(0),burstQueIdx(0),CQ(CAM_ISP_CQ_NONE)
        {dma_cfg.pixel_byte = 1;};

public: /*[IspFunction_B]*/
    //virtual int is_bypass( void )                   {   return 0;       }

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

private:/*[IspFunction_B]*/

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void );

public: /*[DMAO_B]*/


private: /*[DMAO_B]*/


};

/**
*@brief  ISP tuning class
*/
class ISP_TUNING_CTRL:public IspFunction_B
{
public:
    int CQ;

public:

    /**
      *@brief  Constructor
      */
    ISP_TUNING_CTRL():
        CQ(CAM_ISP_CQ_NONE){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )                    {   return ISP_TURNING;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )              {   return "ISP_TURNING";}

    /**
      *@brief  Get register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return (unsigned long)NULL;  }

    /**
      *@brief  Get register range
      */
    virtual unsigned long reg_Range( void )             {   return (unsigned long)NULL;  }

protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){ return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void ){ return 0;}

    /**
      *@brief  Set zoom
      */
    virtual int _setZoom( void ){ return 0;}
};



/**
*@brief  ISP TOP control class
*/
class ISP_TOP_CTRL : public IspFunction_B
{
public:
    struct stIspTopEnTbl en_Top;
    struct stIspTopINT ctl_int;
    struct stIspTopFmtSel fmt_sel;
    struct stIspTopSel ctl_sel;
    struct stIspTopMuxSel ctl_mux_sel;
    struct stIspTopMuxSel2 ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    static int pix_id;
    static MBOOL bScenario;
    static MBOOL bIMG2O_en;
    unsigned int ctl_pix_id; // reg 0x401c
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    int path;
    unsigned int sub_mode;
    int isConcurrency;
    int isEn1AaaGropStatusFixed;
    int isEn1C24StatusFixed;
    int isEn1C02StatusFixed;//FLORIA
    int isEn1CfaStatusFixed;
    int isEn1HrzStatusFixed;
    int isEn1MfbStatusFixed;//FLORIA
    int isEn2CdrzStatusFixed;
    int isEn2G2cStatusFixed;
    int isEn2C42StatusFixed;
    int isImg2oStatusFixed;
    int isImgoStatusFixed;
    int isAaoStatusFixed;
    int isEsfkoStatusFixed;
    int isFlkiStatusFixed;
    int isLcsoStatusFixed;
    int isShareDmaCtlByTurn;
    int b_continuous;
    int tpipe_w;
    int tpipe_h;
    int CQ;
    int isIspOn;
    int isP2IspOnly;
    MBOOL isV3;
    MBOOL isApplyTuning;
public:

    /**
      *@brief  Constructor
      */
    ISP_TOP_CTRL():
        magicNum(0),
        dupCqIdx(0),
        burstQueIdx(0),
        drvScenario(0),

        path(0),
        sub_mode(0),
        isConcurrency(0),
        isEn1AaaGropStatusFixed(0),
        isEn1C24StatusFixed(0),
        isEn1CfaStatusFixed(0),
        isEn1HrzStatusFixed(0),
        isEn2CdrzStatusFixed(0),
        isEn2G2cStatusFixed(0),
        isEn2C42StatusFixed(0),
        isImg2oStatusFixed(0),
        isImgoStatusFixed(0),
        isAaoStatusFixed(0),
        isEsfkoStatusFixed(0),
        isFlkiStatusFixed(0),
        isLcsoStatusFixed(0),
        isShareDmaCtlByTurn(0),
        b_continuous(0),
        tpipe_w(0),
        tpipe_h(0),
        CQ(CAM_ISP_CQ_NONE),
        isIspOn(0),
        isP2IspOnly(0),
        isV3(MFALSE),
        isApplyTuning(MFALSE)
        {ctl_mux_sel.reg_val=0;ctl_mux_sel2.reg_val=0;ctl_sram_mux_cfg.reg_val=0;};

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return ISP_TOP; }

    /**
      *@brief  Return module name
      */
    virtual const char* name_Str( void )  { return "ISP_TOP";}

    /**
      *@brief  Return TOP register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) {  return MT6589_ISP_TOP_BASE;}

    /**
      *@brief  Return TOP register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_ISP_TOP_REG_RANGE; }
    //virtual int is_bypass( void )                       {   return 0;       }

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param );

    /**
      *@brief  Set command trigger mode
      *@param[in] cq : CQ
      *@param[in] mode : trigger mode
      *@param[in] trig_src : trigger source
      */
    virtual MBOOL setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);


protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam  );

    /**
      *@brief  Disable
      */
    virtual int _disable( void );

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void );
};

/**
*@brief  ISP RAW pipe control class
*/
class ISP_RAW_PIPE:public IspFunction_B
{
public:
    int path;
    unsigned int enable1;
    unsigned int enable2;
    unsigned int sub_mode;
    unsigned int magicNum;
    unsigned int dupCqIdx;  //current working Que
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    int CQ;
    int src_img_w;
    int src_img_h;
    IspSize hrz_out;
    IspSize hrz_in;
    MBOOL isV3;
    MBOOL isApplyTuning;
    int p2SubMode;
    class ISP_PASS1_CTRL * pPass1Ctrl;
    P1_TUNING_NOTIFY* pP1Tuning;
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
    MUINT32 m_Dmao_SET_Reg[ISP_DRV_P1_CQ_DUPLICATION_NUM];
    MUINT32 m_Dmao_CLR_Reg[ISP_DRV_P1_CQ_DUPLICATION_NUM];
public:

    /**
      *@brief  Constructor
      */
    ISP_RAW_PIPE():
        path(ISP_PASS_NONE),
        enable1(0),
        enable2(0),
        magicNum(0),
        dupCqIdx(0),
        burstQueIdx(0),
        drvScenario(0),
        CQ(CAM_ISP_CQ_NONE),
        sub_mode(0),
        src_img_w(0),
        src_img_h(0),
        isV3(MFALSE),
        isApplyTuning(MFALSE),
        p2SubMode(0),
        pTuningIspReg(NULL)
        {
            hrz_in = IspSize();
            hrz_out = IspSize();
            for(int i=0;i<ISP_DRV_P1_CQ_DUPLICATION_NUM;i++){
                m_Dmao_SET_Reg[i] = m_Dmao_CLR_Reg[i] = 0xffffffff;
            }
        };

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {   return ISP_RAW;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_RAW";}

    /**
      *@brief Return RAW pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return MT6589_ISP_RAW_BASE;  }

    /**
      *@brief  Return  RAW pipe register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_ISP_RAW_REG_RANGE;  }

public:
    /**
      *@brief  enTuningQue successed or failed
      */
    int checkLscLsciCfg(MUINT32* pLscCfg, MUINT32* pLsciCfg);
    /**
      *@brief  enTuningQue successed or failed
      */
    int setP1TuneCfg(MUINT32 mNum, MINT32 senDev);
    /**
      *@brief  setEIS successed or failed
      */
    MUINT32     setEIS(void);                       //because of EIS have the requirement of real time sync with rrz. so, need additional process
    /**
      *@brief  setLCS successed or failed
      */
    MUINT32     setLCS(void);                       //because of LCS of sync requirement when LCS croping r needed.

protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void );
private:
    int _suspendTuneCfg(MUINT32 mNum);
    void _resumeTuneCfg(IspDrv* _targetVirDrv,MUINT32 _targetDupQueIdx);
};

/**
*@brief  ISP RGB pipe control class
*/
class ISP_RGB_PIPE:public IspFunction_B
{
public:
    unsigned int enable1;
    unsigned int enable2;
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    //int sub_mode;
    int src_img_h;
    int CQ;
    MFLOAT hrzRatio; //for sl2
    MUINT32 hrzCropX; //for sl2
    MUINT32 hrzCropY; //for sl2
    MBOOL isV3;
    MBOOL isApplyTuning;
    int p2SubMode;
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
public:

    /**
      *@brief  Constructor
      */
    ISP_RGB_PIPE():enable1(0),enable2(0),magicNum(0),dupCqIdx(0),burstQueIdx(0),drvScenario(0),src_img_h(0),CQ(CAM_ISP_CQ_NONE),isV3(MFALSE),isApplyTuning(MFALSE),p2SubMode(0),pTuningIspReg(NULL)
    {};

    /**
      *@brief  Destructor
      */
    virtual ~ISP_RGB_PIPE(){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {   return ISP_RGB;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_RGB";}

    /**
      *@brief Return RGB pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) {   return MT6589_ISP_RGB_BASE;  }

    /**
      *@brief  Return  RGB pipe register range
      */
    virtual unsigned long reg_Range( void ) {   return MT6589_ISP_RGB_REG_RANGE;  }
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void );
};

/**
*@brief  ISP YUV pipe control class
*/
class ISP_YUV_PIPE:public IspFunction_B
{
public:
    unsigned int enable1;
    unsigned int enable2;
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    //int sub_mode;
    int CQ;
    MBOOL isV3;
    MBOOL isApplyTuning;
    int p2SubMode;
    isp_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
public:

    /**
      *@brief  Constructor
      */
    ISP_YUV_PIPE():enable1(0),enable2(0),magicNum(0),dupCqIdx(0),burstQueIdx(0),drvScenario(0),CQ(CAM_ISP_CQ_NONE),isV3(MFALSE),isApplyTuning(MFALSE),p2SubMode(0),pTuningIspReg(NULL)
    {};

    /**
      *@brief  Destructor
      */
    ~ISP_YUV_PIPE(){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {   return ISP_YUV;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_YUV";}

    /**
      *@brief Return YUV pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) {   return MT6589_ISP_YUV_BASE;  }

    /**
      *@brief  Return  YUV pipe register range
      */
    virtual unsigned long reg_Range( void ) {   return MT6589_ISP_YUV_REG_RANGE;  }
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
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
    MdpRotDMACfg imgxo_out;
    int mdp_imgxo_p2_en;
    int CQ;
    unsigned int magicNum;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    int drvScenario;
    MBOOL isWaitBuf;
    int enable2;
    int dma_enable;
    MBOOL isV3;
    MBOOL isApplyTuning;
    MBOOL isMdpCropEn;
public:
    ISP_MDP_PIPE():mdp_imgxo_p2_en(0),CQ(CAM_ISP_CQ_NONE),magicNum(0),dupCqIdx(0),burstQueIdx(0),drvScenario(-1),isWaitBuf(MTRUE),enable2(0),dma_enable(0),isV3(MFALSE),isApplyTuning(MFALSE){};
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
    virtual int _write2CQ( void );
public:
    MBOOL createMdpMgr( void );
    MBOOL destroyMdpMgr( void );

};

/**
*@brief  ISP CDP pipe control class
*/
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
    int tpipe_w;
    int disp_vid_sel;
    int conf_cdrz;
    int conf_rotDMA;
    int CQ;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int enable2;
    unsigned int dma_enable;
    int path;
    CDP_DRV_MODE_ENUM tpipeMode;
    MBOOL isV3;
    MBOOL isApplyTuning;

public:

    /**
      *@brief  Contructor
      */
    CAM_CDP_PIPE():conf_cdrz(0),conf_rotDMA(0),CQ(CAM_ISP_CQ_NONE),isV3(MFALSE),isApplyTuning(MFALSE){};
public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {   return ISP_CDP;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   {   return "ISP_CDP";}

    /**
      *@brief Return CDP pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return MT6589_ISP_CDP_BASE;  }

    /**
      *@brief  Return  CDP pipe register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_ISP_CDP_REG_RANGE;  }
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void );

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void );

    /**
      *@brief  Set zoom
      */
    virtual int _setZoom( void );
};

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
    int p2SubMode;
    MBOOL isV3;
    MBOOL isApplyTuning;

public:
    CAM_TDRI_PIPE():dupCqIdx(0),burstQueIdx(0),enTdri(0),CQ(CAM_ISP_CQ_NONE),tcm_en(0),tdr_en(0),magicNum(0),drvScenario(0),p2SubMode(0),isV3(MFALSE),isApplyTuning(MFALSE){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}
    virtual MBOOL configTdriSetting( TdriDrvCfg* pSrcImageio);

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   {   return ISP_TDRI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   {   return "ISP_TDRI";}

    /**
      *@brief Return TDRI pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return MT6589_DMA_TDRI_BASE;  }

    /**
      *@brief  Return TDRI pipe register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_DMA_TDRI_REG_RANGE;  }

protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){ return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( void );

    /**
      *@brief  Set zoom
      */
    virtual int _setZoom( void ){ return 0;}
};



/**
*@brief DMA IMGI control class
*/
class DMA_IMGI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMGI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {  return   ISP_DMA_IMGI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {  return   "ISP_DMA_IMGI";}

    /**
      *@brief Return DMA IMGI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) { return     MT6589_DMA_IMGI_BASE;  }

    /**
      *@brief  Return DMA IMGI register range
      */
    virtual unsigned long reg_Range( void )  {  return     MT6589_DMA_IMGI_REG_RANGE; }
protected:

};

/**
*@brief DMA IMGCI control class
*/
class DMA_IMGCI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMGCI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   {  return   ISP_DMA_IMGCI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {  return  "ISP_DMA_IMGCI";}

    /**
      *@brief Return DMA IMGCI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {  return  MT6589_DMA_IMGCI_BASE; }

    /**
      *@brief  Return DMA IMGCI register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_IMGCI_REG_RANGE;  }
protected:

};

/**
*@brief  DMA FLKI control class
*/
class DMA_FLKI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_FLKI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {  return  ISP_DMA_FLKI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return   "ISP_DMA_FLKI";}

    /**
      *@brief Return DMA FLKI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_FLKI_BASE;     }

    /**
      *@brief  Return DMA FLKI register range
      */
    virtual unsigned long reg_Range( void )   {  return   MT6589_DMA_FLKI_REG_RANGE;     }
protected:
};

/**
*@brief  DMA LSCI control class
*/
class DMA_LSCI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_LSCI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_LSCI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_LSCI";}

    /**
      *@brief Return DMA LSCI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_LSCI_BASE; }

    /**
      *@brief  Return DMA LSCI register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_LSCI_REG_RANGE; }
protected:
};

/**
*@brief  DMA LSEI control class
*/
class DMA_LCEI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_LCEI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_LCEI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_LCEI";}

    /**
      *@brief Return DMA LSEI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_LCEI_BASE;  }

    /**
      *@brief  Return  DMA LSEI register range
      */
    virtual unsigned long reg_Range( void )  {  return  MT6589_DMA_LCEI_REG_RANGE;  }
protected:
};

/**
*@brief DMA VIPI control class
*/
class DMA_VIPI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_VIPI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_VIPI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return  "ISP_DMA_VIPI";}

    /**
      *@brief Return DMA VIPI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) { return  MT6589_DMA_VIPI_BASE; }

    /**
      *@brief  Return DMA VIPI register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_VIPI_REG_RANGE; }
protected:
};

/**
*@brief  DMA VIP2I control class
*/
class DMA_VIP2I:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_VIP2I(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_VIP2I;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return   "ISP_DMA_VIP2I";}

    /**
      *@brief Return DMA VIP2I register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_VIP2I_BASE; }

    /**
      *@brief  Return DMA VIP2I register range
      */
    virtual unsigned long reg_Range( void )  {  return  MT6589_DMA_VIP2I_REG_RANGE;  }
protected:
};

/**
*@brief  DMA TDRI control class
*/
class DMA_TDRI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_TDRI() {};
public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {  return     ISP_DMA_TDRI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {  return  "ISP_DMA_TDRI";}

    /**
      *@brief Return DMA TDRI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return   MT6589_DMA_TDRI_BASE;     }

    /**
      *@brief  Return DMA TDRI register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_TDRI_REG_RANGE;     }
protected:
    //virtual int _config( void );
    //virtual int _write2CQ( int cq );
};

/**
*@brief  DMA CQ control class
*/
class DMA_CQ:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_CQ(){};
public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_CQI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_CQI";}

    /**
      *@brief Return DMA CQ register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_CQI_BASE;  }

    /**
      *@brief  Return DMA CQ register range
      */
    virtual unsigned long reg_Range( void )   {  return  MT6589_DMA_CQI_REG_RANGE; }
protected:
    //virtual int _enable( void* pParam );
    //virtual int _disable( void );
};


/**
*@brief  DMA IMGO control class
*/
class DMA_IMGO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMGO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_IMGO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_IMGO";}

    /**
      *@brief Return DMA IMGO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_IMGO_BASE; }

    /**
      *@brief  Return DMA IMGO register range
      */
    virtual unsigned long reg_Range( void )   {  return   MT6589_DMA_IMGO_REG_RANGE;  }
protected:
};

/**
*@brief  DMA IMG2O control class
*/
class DMA_IMG2O:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMG2O(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_IMG2O;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_IMG2O";}

    /**
      *@brief Return DMA IMG2O register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_IMG2O_BASE; }

    /**
      *@brief  Return DMA IMG2O register range
      */
    virtual unsigned long reg_Range( void )  {  return  MT6589_DMA_IMG2O_REG_RANGE;  }
protected:
};

/**
*@brief  DMA LCSO control class
*/
class DMA_LCSO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_LCSO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_LCSO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return  "ISP_DMA_LCSO";}

    /**
      *@brief Return DMA LCSO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_LCSO_BASE;     }

    /**
      *@brief  Return DMA LCSO register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_LCSO_REG_RANGE;     }
protected:
};

/**
*@brief  DMA AAO control class
*/
class DMA_AAO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_AAO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_AAO; }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return   "ISP_DMA_AAO";}

    /**
      *@brief Return DMA AAO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_AAO_BASE; }

    /**
      *@brief  Return DMA AAO register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_AAO_REG_RANGE;  }
protected:
};

/**
*@brief  DMA ESFKO control class
*/
class DMA_ESFKO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_ESFKO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return   ISP_DMA_ESFKO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_ESFKO";}

    /**
      *@brief Return DMA ESFKO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {  return  MT6589_DMA_ESFKO_BASE; }

    /**
      *@brief  Return DMA ESFKO register range
      */
    virtual unsigned long reg_Range( void ) {  return  MT6589_DMA_ESFKO_REG_RANGE; }
protected:
};


/**
*@brief  DMA AFO control class
*/
class DMA_AFO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_AFO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
  virtual unsigned long id( void )                {    return     ISP_DMA_AFO;  }

    /**
      *@brief  Return module name
      */
  virtual const char*   name_Str( void )          {    return     "ISP_DMA_AFO";}

    /**
      *@brief Return DMA AFO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_AFO_BASE; }

    /**
      *@brief  Return DMA AFO register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_AFO_REG_RANGE;  }
protected:
};

/**
*@brief  DMA EISO control class
*/
class DMA_EISO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_EISO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {  return  ISP_DMA_EISO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_EISO";}

    /**
      *@brief Return DMA EISO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_EISO_BASE; }

    /**
      *@brief  Return DMA EISO register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_EISO_REG_RANGE; }
protected:
};

/**
*@brief  DMA DISPO control class
*/
class DMA_DISPO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_DISPO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return   ISP_DMA_WDMAO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return   "ISP_DMA_WDMAO";}

    /**
      *@brief Return DMA DISPO  register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_WDMAO_BASE; }

    /**
      *@brief  Return DMA DISPO  register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_WDMAO_REG_RANGE; }
protected:
};

/**
*@brief  DMA VIDO control class
*/
class DMA_VIDO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_VIDO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_WROTO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return  "ISP_DMA_WROTO";}

    /**
      *@brief Return DMA VIDO  register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_WROTO_BASE; }

    /**
      *@brief  Return DMA VIDO  register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_WROTO_REG_RANGE; }
protected:
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
    _img2o_,    // 5
    _dispo_,    // 6
    _vido_,     // 7
    _fdo_,      // 8
    _rt_dma_max_
}_isp_dma_enum_;
#endif

/**
*@brief  buffer status enum
*/
typedef enum
{
    ISP_BUF_EMPTY = 0,
    ISP_BUF_FILLED,
}ISP_BUF_STATUS;

#if 0
typedef enum
{
    ISP_BUF_TYPE_PMEM    = BUF_TYPE_PMEM,
    ISP_BUF_TYPE_STD_M4U = BUF_TYPE_STD_M4U,
    ISP_BUF_TYPE_ION     = BUF_TYPE_ION,
}ISP_BUF_TYPE;
#endif

/**
*@brief  HRZ info struct
*/

struct stISP_HRZ_INFO
{
    MUINT32                 srcX;
    MUINT32                 srcY;
    MUINT32                 srcW;
    MUINT32                 srcH;
    MUINT32                 dstW;
    MUINT32                 dstH;
};

/**
*@brief  buffer info struct
*/
struct stISP_BUF_INFO
{
    ISP_BUF_STATUS          status;
    MUINT32                 base_vAddr;
    MUINT32                 base_pAddr;
    MUINT32                 size;
    MUINT32                 memID;
    MINT32                  bufSecu;
    MINT32                  bufCohe;
    MUINT64                 timeStampS;
    MUINT32                 timeStampUs;
    ////
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
    MUINT32                 dma;
    //
    stISP_HRZ_INFO          hrz_info;
    IspRect                 dma_crop;   //dmao cropping
    MVOID                   *header_info; //point to self
    MUINT32                 header_size;
    MINT32                  mBufIdx; // used for replace buffer
    //
    MUINT32                 jpg_size;
    ////
    MVOID                   *private_info;
    struct stISP_BUF_INFO   *next;

    /**
      *@brief  Constructor
      */
    stISP_BUF_INFO(
        ISP_BUF_STATUS          _status = ISP_BUF_EMPTY,
        MUINT32                 _base_vAddr = 0,
        MUINT32                 _base_pAddr = 0,
        MUINT32                 _size = 0,
        MUINT32                 _memID = -1,
        MINT32                  _bufSecu = 0,
        MINT32                  _bufCohe = 0,
        MUINT64                 _timeStampS = 0,
        MUINT32                 _timeStampUs = 0,
        MVOID                   *_private_info = NULL,
        ////
        MUINT32                 _img_w = 0,
        MUINT32                 _img_h = 0,
        MUINT32                 _img_fmt = 0,
        MUINT32                 _img_hbin1 = 0,
        MUINT32                 _img_ob = 0,
        MUINT32                 _img_lsc = 0,
        MUINT32                 _img_rpg = 0,
        MUINT32                 jpg_size = 0,
        MUINT32                 dma = _imgo_,
        ////
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

/*************************************************
*************************************************/
//
typedef list<stISP_BUF_INFO> ISP_BUF_INFO_L;

/**
*@brief  buffer list struct
*/
typedef struct  _isp_buf_list_{
    MUINT32         filledCnt;            //  fill count
    ISP_BUF_INFO_L  bufInfoList;
}stISP_BUF_LIST;

/**
*@brief  filled buffer list struct
*/
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

/**
*@brief  Buffer control class
*/
class ISP_BUF_CTRL : public IspFunction_B
{
friend class IspDrvShellImp;
friend class ISP_TOP_CTRL;

private:
    static stISP_BUF_LIST m_hwbufL[_rt_dma_max_];
    static stISP_BUF_LIST m_swbufL[_rt_dma_max_];
    stISP_BUF_HEADER_LIST m_p1DeqedBufHeaderL[_rt_dma_max_];
    static stISP_BUF_LIST m_p2HwBufL[ISP_DRV_P2_CQ_NUM][ISP_DRV_P2_CQ_DUPLICATION_NUM][_rt_dma_max_];

public:
    int path;
    int drvScenario;
    int dequeCq;  // for P2 deque use
    unsigned int dequeDupCqIdx;  // for P2 deque use
    unsigned int dequeBurstQueIdx;  // for P2 deque use
    int CQ;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    MUINT32      P1DeQCnt[_rt_dma_max_];//for dbg

public:

    /**
      *@brief  Constructor
      */
    ISP_BUF_CTRL():path(0),drvScenario(0),dequeCq(0),dequeDupCqIdx(0),dequeBurstQueIdx(0),CQ(0),dupCqIdx(0),burstQueIdx(0)
        {/*memset(&m_hwBuf,0x00,sizeof(_isp_buf_status_));
         memset(&m_swBuf,0x00,sizeof(_isp_buf_status_));
         memset(m_Buffer,0x00,sizeof(m_Buffer));*/
         for(int i=0;i<_rt_dma_max_;i++){
            P1DeQCnt[i] = 0;
         }
         };

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {   return ISP_BUFFER;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_BUFFER";}

    /**
      *@brief Return buffer control register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return (unsigned long)NULL;  }

    /**
      *@brief  Return buffer control  register range
      */
    virtual unsigned long reg_Range( void )   {   return (unsigned long)NULL;  }

    /**
      *@brief Check bust or not
      */
    virtual int checkBusy( unsigned long* param ) {return 0;}

    /**
      *@brief Initialize
      */
    virtual int init( MUINT32 dmaChannel );

    /**
      *@brief Wait buffer ready
      */
    virtual MBOOL waitBufReady( MUINT32 dmaChannel );

    /**
      *@brief Append new buffer to the end of hwBuf list
      */
    virtual MINT32 enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data ,MBOOL bImdMode);
    virtual MINT32 enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo, MINT32 enqueCq =0, MINT32 dupCqIdx =0 ,MBOOL bImdMode = 0 );
    /*
        * dequeueMdpFrameEnd
        *       set mdp deque frame end for each frame end
        */
    virtual MINT32 dequeueMdpFrameEnd( MINT32 drvScenario );

    /**
      *@brief Set buffer FILLED type and move filled Hw buffer to sw the end of list
      */
    //virtual MINT32 dequeueHwBuf( MUINT32 dmaChannel);//disable by FLORIA

    virtual EIspRetStatus dequeueHwBuf( MUINT32 dmaChannel, stISP_FILLED_BUF_LIST& bufList );

    /**
      *@brief  Delete all swBuf list after inform caller
      */
    //virtual MINT32 dequeueSwBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufList );//disable by FLORIA

    /**
      *@brief Get 1st NOT filled HW buffer address
      */
    virtual MUINT32 getCurrHwBuf( MUINT32 dmaChannel );

    /**
      *@brief Get 2nd NOT filled HW buffer address
      */
    virtual MUINT32 getNextHwBuf( MUINT32 dmaChannel );

    /**
      *@brief Free single physical buffer
      */
    virtual MUINT32 freeSinglePhyBuf(
        stISP_BUF_INFO bufInfo);

    /**
      *@brief Free all physical buffer
      */
    virtual MUINT32 freeAllPhyBuf( void );

private:

    /**
      *@brief Get DMA buffer index
      */
    int getDmaBufIdx( MUINT32 dmaChannel );

    /**
      *@brief 82 tpipe function
      */
    int getMdpFmtEnum( MUINT32 fmtEnum );

    /**
      *@brief Debug usage
      */
    int debugPrint( MUINT32 dmaChannel );

    stISP_BUF_INFO * saveDeqBufInfo( MINT32 rt_dma);
    MINT32 returnDeqBufInfo( MINT32 rt_dma, stISP_BUF_INFO* bufInfo);

protected:

    /**
      *@brief  Configure
      */
    virtual int     _config( void ) {return 0;}

    /**
      *@brief  Enable
      */
    virtual int     _enable( void* pParam  ) {return 0;}

    /**
      *@brief  Disable
      */
    virtual int     _disable( void ) {return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int     _write2CQ( void ) {return 0;}

    /**
      *@brief Set zoom
      */
    virtual int     _setZoom( void ) {return 0;}

};

/**
    cfg hrz & magic
*/
class ISP_PASS1_CTRL : public IspFunction_B
{
public:
    MUINT32 CQ;
    int path;
    unsigned int    dupCqIdx;   //current working Que
    unsigned int    burstQueIdx;

    IspHrzCfg m_HrzCfg;

public:
    ISP_PASS1_CTRL():
            path(ISP_PASS_NONE),
            dupCqIdx(0),
            burstQueIdx(0),
            CQ(CAM_ISP_CQ_NONE)
            {}

public:
    virtual int checkBusy( unsigned long* param ){return 0;}
    virtual MBOOL runTwinDbgLog( void ){return MFALSE;};
    //void resetP1MagicNum(MUINT32 dma){return;}
    int keepP1HrzCfg( IspHrzCfg const hrz_cfg );

public:
    virtual unsigned long id( void )                       { return ISP_PASS1_CTL; }
    virtual const char*   name_Str( void )              { return "ISP_PASS1_CTRL"; }
    virtual unsigned long reg_Base_Addr_Phy( void ) { return MT6582_HRZ_BASE; }
    virtual unsigned long reg_Range( void )            { return MT6582_HRZ_BASE_REG_RANGE; }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam  ) {return 0;}
    virtual int _disable( void ) { return 0; }
    virtual int _write2CQ( void );
public:
    virtual int setP1HrzCfg( IspHrzCfg const hrz_cfg );
    int setP1MagicNum( MUINT32 pass1_path, MUINT32 MagicNum, MUINT32 magicNumCnt);

private:


};

/**
*@brief  IspEventThreadl class
*/
class IspEventThread
{
    protected:

        /**
            *@brief Constructor
            */
        IspEventThread(){};

        /**
            *@brief Destructor
            */
        virtual ~IspEventThread() {};

    public:

        /**
            *@brief Create IspEventThread object
            */
        static  IspEventThread*  createInstance(NSIspDrv_FrmB::IspDrv* pIspDrv);

    public:

        /**
            *@brief Initialize
            */
        virtual MBOOL   init(void) = 0;

        /**
            *@brief Uninitialize
            */
        virtual MBOOL   uninit(void) = 0;

};

};  //NSDrvCam_FrmB

#endif /*__ISP_FUNCTION_FRMB_H__*/


