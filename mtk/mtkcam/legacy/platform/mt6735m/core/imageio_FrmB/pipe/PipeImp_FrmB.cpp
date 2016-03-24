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
#define LOG_TAG "iio/pimp_FrmB"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include <cutils/properties.h>  // For property_get().

#include "PipeImp_FrmB.h"
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#include <mtkcam/imageio/ispio_utility.h>
//

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio_FrmB {
namespace NSIspio_FrmB   {
////////////////////////////////////////////////////////////////////////////////

#if 0
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{Pipe}"
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(pipe);
//EXTERN_DBG_LOG_VARIABLE(pipe);

// Clear previous define, use our own define.
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (pipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (pipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (pipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (pipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (pipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (pipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
#else

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);

#endif

/*******************************************************************************
*
********************************************************************************/
PipeImp::
PipeImp(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EDrvScenario const eScenarioID,
    EScenarioFmt const eScenarioFmt
)
    : mszPipeName(szPipeName)
    , mePipeID(ePipeID)
    //
    , mi4ErrorCode(0)
    //
    , mpCbUser(NULL)
    , mi4NotifyMsgSet(0)
    , mNotifyCb(NULL)
    , mi4DataMsgSet(0)
    , mDataCb(NULL)
    //
    , meScenarioID(eScenarioID)
    , meScenarioFmt(eScenarioFmt)
    //, meScenarioFmtD(eScenarioFmt)
    //
{
    DBG_LOG_CONFIG(imageio, pipe);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
PipeImp::
setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user)
{
    PIPE_DBG("(notify_cb, data_cb, user)=(%p, %p, %p)", notify_cb, data_cb, user);
    mpCbUser = user;
    mNotifyCb = notify_cb;
    mDataCb = data_cb;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PipeImp::
onNotifyCallback(PipeNotifyInfo const& msg) const
{
    MBOOL   ret = MTRUE;
    //
    if  ( mNotifyCb )
    {
        mNotifyCb(mpCbUser, msg);
        ret = MTRUE;
    }
    else
    {
        PIPE_WRN("Notify Callback is NULL");
        ret = MFALSE;
    }
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PipeImp::
onDataCallback(PipeDataInfo const& msg) const
{
    MBOOL   ret = MTRUE;
    //
    if  ( mDataCb )
    {
        mDataCb(mpCbUser, msg);
        ret = MTRUE;
    }
    else
    {
        PIPE_WRN("Data Callback is NULL");
        ret = MFALSE;
    }
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
PipeImp::
mapScenarioFormatToSubmode(EDrvScenario const eScenarioID, EScenarioFmt const eScenarioFmt)
{
#define FMT_TO_SUBMODE(_submode, _format)  case _format: return _submode

    switch  (eScenarioID)
    {
#if 0 //_mt6593fpga_dvt_use_
    case eScenarioID_IC:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        FMT_TO_SUBMODE(3, eScenarioFmt_JPG);
        default: break;
        }
        break;

    //
    case eScenarioID_CONFIG_FMT: //  Config FMT
    case eScenarioID_VR:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        default: break;
        }
        break;
    //
    case eScenarioID_ZSD:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        default: break;
        }
        break;
    //
    case eScenarioID_VEC:
        return  0;
    //
    case eScenarioID_N3D_VR:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        default: break;
        }
        break;

#endif

    //
    case eDrvScenario_IP:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        FMT_TO_SUBMODE(3, eScenarioFmt_RGB_LOAD);
        FMT_TO_SUBMODE(4, eScenarioFmt_MFB);
        default: break;
        }
        break;
    //
    case eDrvScenario_CC:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        default: break;
        }
        break;
    //
    default:
        break;
    }

#undef  FMT_TO_SUBMODE

    PIPE_DBG("- bad (eScenarioID, eScenarioFmt)=(%d, %d)", eScenarioID, eScenarioFmt);
    return  -1;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PipeImp::
configMdpOutPort(PortInfo const* oImgInfo, MdpRotDMACfg &a_rotDma)
{
    MUINT32 plane_num = 1;
    MUINT32 uv_resample = 1;
    MUINT32 uv_h_ratio = 1;
    MUINT32 uv_v_ratio = 1;
    MUINT32 y_plane_size = 0;
    MUINT32 u_plane_size = 0;
    //
    PIPE_DBG("[oImgInfo]w(%d),h(%d),stride(%d/%d/%d),crop(%d,%d,%d,%d)_f(0x%x, 0x%x),rot(%d) eImgFmt(%d)",
        oImgInfo->u4ImgWidth,oImgInfo->u4ImgHeight,oImgInfo->u4Stride[ESTRIDE_1ST_PLANE],oImgInfo->u4Stride[ESTRIDE_2ND_PLANE],oImgInfo->u4Stride[ESTRIDE_3RD_PLANE], \
        oImgInfo->crop1.x,oImgInfo->crop1.y,oImgInfo->crop1.w,oImgInfo->crop1.h,oImgInfo->crop1.floatX,oImgInfo->crop1.floatY, \
        oImgInfo->eImgRot,oImgInfo->eImgFmt );
    //
    if ( ePipeID_1x1_Sensor_Tg_Mem == getPipeId() ) {
        PIPE_ERR("Pipe Not Support");
        return MFALSE;
    }
    //
    a_rotDma.uv_plane_swap = 0;
    //
    switch( oImgInfo->eImgFmt ) {
       case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane(YVU)
       case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
           a_rotDma.Format = CDP_DRV_FORMAT_YUV420;
           a_rotDma.Plane = CDP_DRV_PLANE_3;
           a_rotDma.Sequence = CDP_DRV_SEQUENCE_UVUV; //don't care, but cdp_drv need to set this value , why?
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 2;
           a_rotDma.uv_plane_swap = (eImgFmt_YV12==oImgInfo->eImgFmt)?1:0;
           break;
        case eImgFmt_I422:      //422 format, 3 plane(YUV)
           a_rotDma.Format = CDP_DRV_FORMAT_YUV422;
           a_rotDma.Plane = CDP_DRV_PLANE_3;
           a_rotDma.Sequence = CDP_DRV_SEQUENCE_UVUV; //don't care, but cdp_drv need to set this value , why?
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 1;
           break;
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
            a_rotDma.Format = CDP_DRV_FORMAT_YUV420;
            a_rotDma.Plane = CDP_DRV_PLANE_2;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_UVUV; //MSN->LSB
            plane_num = 2;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
            a_rotDma.Format = CDP_DRV_FORMAT_YUV420;
            a_rotDma.Plane = CDP_DRV_PLANE_2;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_VUVU; //MSN->LSB
            plane_num = 2;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_rotDma.Format = CDP_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_VYUY; //MSN->LSB
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = CDP_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_YVYU; //MSN->LSB
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
            a_rotDma.Format = CDP_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CDP_DRV_PLANE_3;
            a_rotDma.Sequence = (CDP_DRV_SEQUENCE_ENUM)0; //MSN->LSB
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
            a_rotDma.Format = CDP_DRV_FORMAT_YUV422;
            a_rotDma.Plane = CDP_DRV_PLANE_2;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_VUVU; //MSN->LSB
            plane_num = 2;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            a_rotDma.Format = CDP_DRV_FORMAT_RGB565;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_RGB; //MSN->LSB
            break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            a_rotDma.Format = CDP_DRV_FORMAT_RGB888;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_BGR; //MSN->LSB
            break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            a_rotDma.Format = CDP_DRV_FORMAT_XRGB8888;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_XBGR; //MSN->LSB
            break;
        case eImgFmt_Y800:        //= 0x040000, //Y plane only
            a_rotDma.Format = CDP_DRV_FORMAT_Y;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            a_rotDma.Sequence = CDP_DRV_SEQUENCE_Y; //MSN->LSB
            break;
        case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit     // for imgo
            a_rotDma.Format = CDP_DRV_FORMAT_RAW8;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            break;
        case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit    // for imgo
            a_rotDma.Format = CDP_DRV_FORMAT_RAW10;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            break;
        case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit    // for imgo
            a_rotDma.Format = CDP_DRV_FORMAT_RAW12;
            a_rotDma.Plane = CDP_DRV_PLANE_1;
            break;
        case eImgFmt_JPG_I420:
        case eImgFmt_JPG_I422:
        case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
        default:
            PIPE_ERR("vOutPorts[]->eImgFmt:Format(%d) NOT Support",oImgInfo->eImgFmt);
            break;
    }
    //
    //ROTATION, stride is after, others are before
    a_rotDma.Rotation = (CDP_DRV_ROTATION_ENUM)oImgInfo->eImgRot;
    a_rotDma.Flip = (eImgFlip_ON == oImgInfo->eImgFlip)?MTRUE:MFALSE;
    //dma port capbility
    a_rotDma.capbility=oImgInfo->capbility;
    //Y
    a_rotDma.memBuf.base_pAddr = oImgInfo->u4BufPA[0];
    a_rotDma.memBuf.base_vAddr = oImgInfo->u4BufVA[0];
    a_rotDma.memBuf.size = oImgInfo->u4BufSize[0];
    a_rotDma.memBuf.ofst_addr = 0;
    a_rotDma.memBuf.alignment = 0;
    //after ROT
    a_rotDma.size.w = oImgInfo->u4ImgWidth;
    a_rotDma.size.h = oImgInfo->u4ImgHeight;
    //stride info after ROT
    a_rotDma.size.stride = oImgInfo->u4Stride[ESTRIDE_1ST_PLANE];
    //
    y_plane_size = oImgInfo->u4BufSize[ESTRIDE_1ST_PLANE];
    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}
    //
    if ( 2<=plane_num) {
        //U
        a_rotDma.memBuf_c.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_2ND_PLANE];
        a_rotDma.memBuf_c.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_2ND_PLANE];
        a_rotDma.memBuf_c.ofst_addr = 0;
        a_rotDma.memBuf_c.alignment = 0;
        //after ROT
        a_rotDma.size_c.w =         a_rotDma.size.w/uv_h_ratio;
        a_rotDma.size_c.h =         a_rotDma.size.h/uv_v_ratio;
        //stride info after ROT
        a_rotDma.size_c.stride =    oImgInfo->u4Stride[ESTRIDE_2ND_PLANE];
        //
        u_plane_size = oImgInfo->u4BufSize[ESTRIDE_2ND_PLANE];
        //tpipemain lib need image info before ROT. stride info after ROT
        //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //    a_rotDma.size_c.w = a_rotDma.size.h/uv_v_ratio;
        //    a_rotDma.size_c.h = a_rotDma.size.w/uv_h_ratio;
        //}
        //
        if ( 3 == plane_num ) {
            //V
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.ofst_addr = 0;
            a_rotDma.memBuf_v.alignment = 0;
            //after ROT
            a_rotDma.size_v.w =        a_rotDma.size_c.w;
            a_rotDma.size_v.h =        a_rotDma.size_c.h;
            //stride info after ROT
            a_rotDma.size_v.stride =   oImgInfo->u4Stride[ESTRIDE_3RD_PLANE];
            //tpipemain lib need image info BEFORE ROT. stride info AFTER ROT
            //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
            //    a_rotDma.size_v.w = a_rotDma.size_c.h;
            //    a_rotDma.size_v.h = a_rotDma.size_c.w;
            //}
        }
        //
        /*
        if ( a_rotDma.uv_plane_swap ) {
            a_rotDma.memBuf_c.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_c.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_2ND_PLANE];
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_2ND_PLANE];
        }
        */
    }

    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
    //    //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}

    /*
        * DONOT USE CRZ CROP due to throughtput issue
        */
    //default set ENABLE, if need to be programmable, check cdp driver as well.
    a_rotDma.crop_en = 1; //always enable crop for rotation issue.
    //
    a_rotDma.crop.x = 0;
    a_rotDma.crop.floatX = 0;
    a_rotDma.crop.y = 0;
    a_rotDma.crop.floatY = 0;
    a_rotDma.crop.w = 0;
    a_rotDma.crop.h = 0;
    //
    PIPE_DBG("[a_rotDma]w(%d),h(%d),stride(pxl)(%d),pa(0x%x),va(0x%x),size(%d)",a_rotDma.size.w,a_rotDma.size.h,a_rotDma.size.stride,a_rotDma.memBuf.base_pAddr,a_rotDma.memBuf.base_vAddr,a_rotDma.memBuf.size);
    PIPE_DBG("[a_rotDma]crop(%d,%d,%d,%d)_f(0x%x,0x%x)",a_rotDma.crop.x,a_rotDma.crop.y,a_rotDma.crop.w,a_rotDma.crop.h,a_rotDma.crop.floatX,a_rotDma.crop.floatY);
    PIPE_DBG("[a_rotDma]rot(%d),fmt(%d),plane(%d),seq(%d),pxlByte((0x%x))",a_rotDma.Rotation,a_rotDma.Format,a_rotDma.Plane,a_rotDma.Sequence,a_rotDma.pixel_byte);
    PIPE_DBG("[a_rotDma]c_ofst_addr(0x%x),v_ofst_addr(0x%x),uv_plane_swap(%d)",a_rotDma.memBuf_c.ofst_addr,a_rotDma.memBuf_v.ofst_addr,a_rotDma.uv_plane_swap);
    PIPE_DBG("[a_rotDma]va[0x%x,0x%x,0x%x]--pa[0x%x,0x%x,0x%x]",
        a_rotDma.memBuf.base_vAddr,a_rotDma.memBuf_c.base_vAddr,a_rotDma.memBuf_v.base_vAddr,a_rotDma.memBuf.base_pAddr,a_rotDma.memBuf_c.base_pAddr,a_rotDma.memBuf_v.base_pAddr);


    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PipeImp::configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum)
{

    ISP_QUERY_RST queryRst;
    //
    a_dma.memBuf.size        = portInfo->u4BufSize[planeNum];
    a_dma.memBuf.base_vAddr  = portInfo->u4BufVA[planeNum];
    a_dma.memBuf.base_pAddr  = portInfo->u4BufPA[planeNum];
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->u4ImgWidth;
    a_dma.size.h            = portInfo->u4ImgHeight;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->u4Stride[planeNum];
    //dma starting offset
    a_dma.offset.x          =  portInfo->xoffset;
    a_dma.offset.y          =  portInfo->yoffset;
    //
    #if 0
    a_dma.format = portInfo->eImgFmt;
    #endif
    //dma port capbility
    a_dma.capbility=portInfo->capbility;
    //input xsize unit:byte
    ISP_QuerySize(portInfo->index,ISP_QUERY_STRIDE_BYTE,(EImageFormat)portInfo->eImgFmt,a_dma.size.w,queryRst);

    a_dma.size.xsize        =  queryRst.stride_byte;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%d) < image width(%d) byte size",a_dma.size.stride,a_dma.size.w);
    }
    //
    a_dma.crop.x            = portInfo->crop1.x;
    a_dma.crop.floatX        = portInfo->crop1.floatX;
    a_dma.crop.y            = portInfo->crop1.y;
    a_dma.crop.floatY        = portInfo->crop1.floatY;
    a_dma.crop.w            = portInfo->crop1.w;
    a_dma.crop.h            = portInfo->crop1.h;
    //
    a_dma.swap = swap;
    //
    if(isBypassOffset) // for tpipe mode
        a_dma.memBuf.ofst_addr = 0;
    else
        a_dma.memBuf.ofst_addr   = ( ( ( portInfo->crop1.y ? (portInfo->crop1.y- 1) : 0 )*(portInfo->u4ImgWidth)+portInfo->crop1.x ) * pixel_Byte ) >> CAM_ISP_PIXEL_BYTE_FP;
    //

    switch( portInfo->eImgFmt ) {
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=1;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=0;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=3;
            break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=2;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            a_dma.format_en=1;
            a_dma.format=2;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            a_dma.format_en=1;
            a_dma.format=2;
            a_dma.bus_size_en=1;
            a_dma.bus_size=2;
            break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            a_dma.format_en=1;
            a_dma.format=2;
            a_dma.bus_size_en=1;
            a_dma.bus_size=3;
            break;
        case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
        case eImgFmt_BAYER10:   /*!< Bayer format, 10-bit */
        case eImgFmt_BAYER12:   /*!< Bayer format, 12-bit */
        case eImgFmt_BAYER14:   /*!< Bayer format, 14-bit */
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=1;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV16:      //422 format, 2 plane
            PIPE_ERR("NOT support this format(0x%x) in ISP port",portInfo->eImgFmt);
            break;
        case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
        case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
        default:
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=0;
            break;
    }
    //

    PIPE_DBG("w(%d),h(%d),stride(%d),xsize(%d),crop(%d,%d,%d,%d)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%x),pa(0x%x),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        a_dma.memBuf.base_vAddr, a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",portInfo->eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

