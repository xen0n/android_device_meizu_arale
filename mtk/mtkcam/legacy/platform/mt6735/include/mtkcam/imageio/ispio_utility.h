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
#ifndef _ISPIO_PIPE_UTILITY_H_
#define _ISPIO_PIPE_UTILITY_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#include <mtkcam/imageio/ispio_pipe_ports.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {

/*******************************************************************************
*
********************************************************************************/

#include <cutils/log.h>

#include <mtkcam/ImageFormat.h>

#define ISPUTIL_MSG(fmt, arg...)    //ALOGD("[%s]"          fmt, __FUNCTION__,           ##arg)
#define ISPUTIL_WRN(fmt, arg...)    ALOGW("[%s]WRN(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define ISPUTIL_ERR(fmt, arg...)    ALOGE("[%s]ERR(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)



/*******************************************************************************
*
********************************************************************************/
static MUINT32
queryRawStride(MUINT32 const imgFmt, MUINT32 const& imgWidth)
{
    MUINT32 stride = imgWidth;

    //
    if ( imgWidth % 2) {
        ISPUTIL_ERR("width and stride should be even number");
    }
    //
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
            stride = imgWidth;
            //by TC. should be 4 bytes align(pass2 imgi constrain).20130911,1530
            stride = (stride+3)&(~0x03);
            break;

        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
            if ( imgWidth % 8 ) {
                stride = imgWidth + 8 - (imgWidth % 8);
            }
            if ( imgWidth > stride ) {
                ISPUTIL_ERR(" RAW10 STRIDE SHOULD BE MULTIPLE OF 8(%d)->(%d)", imgWidth, stride);
            }
            break;

        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
            if ( stride % 4 ) {
                stride = stride + 4 - (stride % 4);
            }
            if ( imgWidth > stride ) {
                ISPUTIL_ERR(" RAW12 STRIDE SHOULD BE MULTIPLE OF 4(%d)->(%d)", imgWidth, stride);
            }
            break;

        default:
            ISPUTIL_WRN("NOT SUPPORT imgFmt(%d)",imgFmt);
            break;
    }

    ISPUTIL_MSG("imgFmt(%d), imgWidth(%d), stride(%d)", imgFmt, imgWidth, stride);
    return stride;
}

/**
    query bytes per pixel under differ. format
*/
static MUINT32  queryBitPerPix(MUINT32 const imgFmt){
        MUINT32 pixDepth;
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
        case eImgFmt_Y8:              //weighting table
        case eImgFmt_FG_BAYER8:
            pixDepth = 8;
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
        case eImgFmt_FG_BAYER10:
            pixDepth = 10;
            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
        case eImgFmt_FG_BAYER12:
            pixDepth = 12;
            break;
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            pixDepth = 16;//?
            break;
        case eImgFmt_UFO_BAYER10:
            pixDepth = 10;
            break;
        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            pixDepth = 16;
            break;
        case eImgFmt_NV16:  //user would set format_nv16 to all of 2 planes if multi-plane(img3o~img3bo)
        case eImgFmt_NV12:
        case eImgFmt_YV12:  //user would set format_yv12 to all of 3 planes if multi-plane(img3o~img3co)
        case eImgFmt_I420:
            pixDepth = 8;   //temp use pixDepth 8
            break;
        case eImgFmt_RGB565:
            pixDepth = 16;
            break;
        case eImgFmt_RGB888:
            pixDepth = 24;
            break;
        case eImgFmt_JPEG:
            pixDepth = 8;
            break;
        default:
            ISPUTIL_ERR("eImgFmt:[%d]NOT Support",imgFmt);
            return -1;
    }
    if( imgFmt == eImgFmt_FG_BAYER8 ||
        imgFmt == eImgFmt_FG_BAYER10 ||
        imgFmt == eImgFmt_FG_BAYER12 ||
        imgFmt == eImgFmt_UFO_BAYER10)
    {
        pixDepth = (pixDepth*3)/2;
    }
    return pixDepth;
}


typedef enum _QUERYOP{
    ISP_QUERY_X_PIX                 = 0x0001,
    ISP_QUERY_STRIDE_PIX            = 0x0002,
    ISP_QUERY_STRIDE_BYTE           = 0x0004,
    ISP_QUERY_XSIZE_BYTE            = 0x0008,
    ISP_QUERY_CROP_X_PIX            = 0x0010,   //query when using cropping, (cropping size != input size)
    ISP_QUERY_CROP_X_BYTE           = 0x0020,   //query when using cropping, (cropping size != input size)
    ISP_QUERY_CROP_START_X          = 0x0040,   //query when using cropping, the unit of crop start x is pixel.
}E_ISP_QUERY;

typedef enum _QUERY_PIXMODE{
    ISP_QUERY_UNKNOWN_PIX_MODE  = 0x0,      // default use 2 pixmode constraint if unknown pix mode being asignned
    ISP_QUERY_1_PIX_MODE        = 0x1,      // 1 pix mode
    ISP_QUERY_2_PIX_MODE        = 0x2,      // 2 pix mode
    ISP_QUERY_MAX_PIX_MODE      = 0x3
}E_ISP_PIXMODE;

typedef struct _ISP_QUERY_RST{
    MUINT32 x_pix;                  //horizontal resolution, unit:pix
    MUINT32 stride_pix;             //stride, uint:pix. this is a approximative value under pak mode
    MUINT32 stride_byte;            //stride, uint:byte
    MUINT32 xsize_byte;
    MUINT32 crop_x;                 //crop start point-x , unit:pix
    _ISP_QUERY_RST(
                MUINT32 _x_pix = 0,
                MUINT32 _stride_pix = 0,
                MUINT32 _stride_byte = 0,
                MUINT32 _xsize_byte = 0,
                MUINT32 _crop_x = 0
            )
        : x_pix(_x_pix)
        , stride_pix(_stride_pix)
        , stride_byte(_stride_byte)
        , xsize_byte(_xsize_byte)
        , crop_x(_crop_x)
    {}
}ISP_QUERY_RST,*PISP_QUERY_RST;

static MUINT32 _query_pix_mode_constraint(MUINT32 size,MUINT32 pixMode){
    /*
    1 pix mode => 2n
    2 pix mode => 4n
    but current flow , user have no pix mode information, so fix 4n for safety
    */
    MUINT32 divisor = 0x3;
    switch(pixMode){
        case ISP_QUERY_UNKNOWN_PIX_MODE:
        case ISP_QUERY_2_PIX_MODE:
            divisor = 0x3;
            break;
        case ISP_QUERY_1_PIX_MODE:
            divisor = 0x1;
            break;
        default:
            ISPUTIL_ERR("pix mode error(%d)",pixMode);
            break;
    }
    if(size & divisor)
        return 0;
    else
        return 1;
}

static MUINT32 _query_p2_constraint(MUINT32 size){
    if(size&0x1)
        return 0;
    else
        return 1;
}

//only add this constraint at imgo is because of rrzo have 8-alignment constraint. but imgo have only 2-alignment under 1-pixmode
#define _query_p2_stride_constraint( stride){\
    stride = (stride + 0x3) & (~0x3);\
}

static MUINT32 _query_fg_constraint(MUINT32 size){
    if(size&0x3)
        return 0;
    else
        return 1;
}


static MUINT32 _queryCropStart(EImageFormat imgFmt,MUINT32 input,ISP_QUERY_RST &QueryRst)
{
    switch(imgFmt){
        case eImgFmt_BAYER10:
            //if bus size 16bit, the inputer must be 8 alignment,
            QueryRst.crop_x = (input >> 3) << 3;
            break;
        case eImgFmt_BAYER12:
            //if bus size 16bit, the inputer must be 4 alignment,
            QueryRst.crop_x = (input >> 2) << 2;
            break;
        case eImgFmt_BAYER8:
        case eImgFmt_FG_BAYER8:
        case eImgFmt_FG_BAYER10:
        case eImgFmt_FG_BAYER12:
            QueryRst.crop_x = input;
            break;
        default:
            QueryRst.crop_x = 0;
            ISPUTIL_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            return 0;
            break;
    }
    return 1;
}

static MUINT32 _queryXsize_imgo(EImageFormat imgFmt,MUINT32& xsize, MUINT32& outWidth,MUINT32 pixMode,MBOOL bCrop){
    outWidth += (outWidth&0x1);

    switch(imgFmt){
        case eImgFmt_BAYER8:
        case eImgFmt_JPEG:
            while(1){
                xsize = outWidth;
                if(_query_pix_mode_constraint(xsize,pixMode) && _query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_BAYER10:
            while(1){
                if(bCrop == MFALSE){//do nothing
                }
                else{
                    //width need 4 alignment to make sure xsize is 8-alignment
                    while(1){
                        if(outWidth & 0x3)
                            outWidth -=2;
                        else
                            break;
                    }
                }
                xsize = (outWidth*10 + 7) / 8;
                xsize += (xsize & 0x1);
                if(_query_pix_mode_constraint(xsize,pixMode) && _query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_BAYER12:
            while(1){
                //no need to patch under cropping , because outWidth must be at leaset 2-alignment
                xsize = (outWidth*12 + 7) / 8;
                xsize += (xsize & 0x1);
                if(_query_pix_mode_constraint(xsize,pixMode) && _query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
            while(1){
                xsize = outWidth;
                xsize*=2;
                if(_query_pix_mode_constraint(xsize,pixMode) && _query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_RGB888:
        case eImgFmt_RGB565:
            while(1){
                xsize = outWidth;
                xsize*=2;
                if(_query_pix_mode_constraint(xsize,pixMode) && _query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            ISPUTIL_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            break;
        case eImgFmt_BLOB:
            xsize = outWidth;
            break;
        default:
            xsize = outWidth = 0;
            ISPUTIL_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            return 0;
            break;
    }
    return 1;
}

#define query_rrzo_constraint(xsize){\
    xsize = (xsize + 0x7) & (~0x7);\
}

#define RRZ_LIMITATION  2304
static MUINT32 _queryXsize_rrzo(EImageFormat imgFmt,MUINT32& xsize, MUINT32& outWidth,MUINT32 pixMode){
    outWidth += (outWidth&0x1);
    if( (outWidth > RRZ_LIMITATION) && (pixMode == ISP_QUERY_1_PIX_MODE) ){
        ISPUTIL_ERR("rrzo output over 2304, support no 1 pix mode, 2 pix mode will be enabled\n");
        ISPUTIL_ERR("Warning: plz re-check imgo limitation in 2 pix mode if imgo is needed\n");
    }
    switch(imgFmt){
        case eImgFmt_FG_BAYER8:
            while(1){
                xsize = outWidth*3>>1;
                xsize = xsize;
                query_rrzo_constraint(xsize);
                if(_query_fg_constraint(xsize))
                    break;
                else
                {
                    outWidth -= 2;
                    ISPUTIL_MSG("outWidth(%d)", outWidth);
                }
            }
            break;
        case eImgFmt_FG_BAYER10:
            while(1){
                xsize = outWidth*3>>1;
                xsize = (xsize*10 + 7) / 8;
                query_rrzo_constraint(xsize);
                if(_query_fg_constraint(xsize))
                    break;
                else
                {
                    outWidth -= 2;
                    ISPUTIL_MSG("outWidth(%d)", outWidth);
                }
            }
            break;
        case eImgFmt_FG_BAYER12:
            while(1){
                xsize = outWidth*3>>1;
                xsize = (xsize*12 + 7) / 8;
                query_rrzo_constraint(xsize);
                if(_query_fg_constraint(xsize))
                    break;
                else
                {
                    outWidth -= 2;
                    ISPUTIL_MSG("outWidth(%d)", outWidth);
                }
            }
            break;
        default:
            xsize = outWidth = 0;
            ISPUTIL_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            return 0;
            break;
    }
    return 1;
}

static MUINT32 _queryXsize_p2(EImageFormat imgFmt,MUINT32& xsize, MUINT32& outWidth){
    outWidth += (outWidth&0x1);

    switch(imgFmt){
        case eImgFmt_BAYER8:
        case eImgFmt_Y8:
        case eImgFmt_JPEG:
            while(1){
                xsize = outWidth;
                if(_query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_BAYER10:
            while(1){
                xsize = (outWidth*10 + 7) / 8;
                xsize += (xsize & 0x1);
                if( _query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_BAYER12:
            while(1){
                xsize = (outWidth*12 + 7) / 8;
                xsize += (xsize & 0x1);
                if(_query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_FG_BAYER8:
            while(1){
                xsize = outWidth*3>>1;
                xsize = xsize;
                query_rrzo_constraint(xsize);
                if(_query_fg_constraint(xsize))
                    break;
                else
                {
                    outWidth -= 2;
                    ISPUTIL_MSG("outWidth(%d)", outWidth);
                }
            }
            break;
        case eImgFmt_FG_BAYER10:
            while(1){
                xsize = outWidth*3>>1;
                xsize = (xsize*10 + 7) / 8;
                query_rrzo_constraint(xsize);
                if(_query_fg_constraint(xsize))
                    break;
                else
                {
                    outWidth -= 2;
                    ISPUTIL_MSG("outWidth(%d)", outWidth);
                }
            }
            break;
        case eImgFmt_FG_BAYER12:
            while(1){
                xsize = outWidth*3>>1;
                xsize = (xsize*12 + 7) / 8;
                query_rrzo_constraint(xsize);
                if(_query_fg_constraint(xsize))
                    break;
                else
                {
                    outWidth -= 2;
                    ISPUTIL_MSG("outWidth(%d)", outWidth);
                }
            }
            break;
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
            while(1){
                xsize = outWidth;
                xsize*=2;
                if(_query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_NV16:  //user would set format_nv16 to all of 2 planes if multi-plane(img3o~img3bo)
        case eImgFmt_NV12:
            while(1){
                xsize = outWidth;
                if(_query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_YV12:  //user would set format_yv12 to all of 3 planes if multi-plane(img3o~img3co)
        case eImgFmt_I420:
            while(1){
                xsize = outWidth;
                if(_query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            break;
        case eImgFmt_RGB888:
        case eImgFmt_RGB565:
            while(1){
                xsize = outWidth;
                xsize*=2;
                if(_query_p2_constraint(xsize))
                    break;
                else {
                    outWidth -= 2;
                    ISPUTIL_MSG("constraint: xsize(%d),outWidth(%d)",xsize, outWidth);
                }
            }
            ISPUTIL_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            break;
        default:
            xsize = outWidth = 0;
            ISPUTIL_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            return 0;
            break;
    }
    return 1;
}

/**
    query horizontal resolution/stride ...etc information.
    @note:
                a. if Need to bypass portId, plz user define: _BY_PASSS_PORT defined as below.
                b. pix_mode will run in 2-pix mode if 0 is applied on e_PixMode.
    input param:
        QueryInput: a. image cropping start_x(unit:pix) if e_Op == ISP_QUERY_CROP_START_X,
                    b. image horizontal resolution(uint: pix) if e_Op != ISP_QUERY_CROP_START_X.
    output param:
        QueryRst: return query result.
    return value:
        TRUE if no error.
        FALSE if format/dma r not supported.
*/
#define _BY_PASSS_PORT  0xFFFF
static MUINT32 ISP_QueryBusSize(EImageFormat imgFmt,MINT32 &BusSize)
{
    switch(imgFmt){
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
        case eImgFmt_RGB565:
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
        case eImgFmt_BAYER14:
        case eImgFmt_FG_BAYER8:
        case eImgFmt_FG_BAYER10:
        case eImgFmt_FG_BAYER12:
            BusSize = 1;
            break;
        case eImgFmt_RGB888:
            BusSize = 2;
            break;
        case eImgFmt_ARGB8888:
            BusSize = 2;
            break;
        case eImgFmt_NV12:
        case eImgFmt_NV21:
        case eImgFmt_YV12:
        case eImgFmt_I420:
            BusSize = 0;
            break;
        default:
            BusSize = 1;
            ISPUTIL_ERR("warning!! default bus size (0x%x)",BusSize);
            break;
    }
    return 1;
}

static MUINT32 ISP_QuerySize(MUINT32 portId,/*E_ISP_QUERY*/MUINT32 e_Op,EImageFormat imgFmt,MUINT32 QueryInput,ISP_QUERY_RST &QueryRst ,E_ISP_PIXMODE e_PixMode=ISP_QUERY_UNKNOWN_PIX_MODE)
{
    MUINT32 xsize,outWidth=QueryInput;
    bool p2_cfg=false;
    MBOOL _bCrop = MFALSE;
    if(portId == _BY_PASSS_PORT){
        switch(imgFmt){
            case eImgFmt_BAYER8:
            case eImgFmt_UFO_BAYER10:
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
            case eImgFmt_BLOB:
                if(e_Op & (ISP_QUERY_CROP_X_PIX|ISP_QUERY_CROP_X_BYTE))
                    _bCrop = MTRUE;
                else
                    _bCrop = MFALSE;

                if(_queryXsize_imgo(imgFmt,xsize,outWidth,e_PixMode, _bCrop) == 0)
                        return 0;
                if(e_Op & (ISP_QUERY_X_PIX|ISP_QUERY_CROP_X_PIX)){
                    QueryRst.x_pix = outWidth;
                }
                if(e_Op & ISP_QUERY_STRIDE_PIX) {
                    QueryRst.stride_pix = xsize / queryBitPerPix(imgFmt);
                }
                if(e_Op & ISP_QUERY_STRIDE_BYTE){
                    QueryRst.stride_byte = xsize;
                    _query_p2_stride_constraint(QueryRst.stride_byte);
                }
                if(e_Op & (ISP_QUERY_XSIZE_BYTE|ISP_QUERY_CROP_X_BYTE)){
                    QueryRst.xsize_byte = xsize;
                }
                if(e_Op & ISP_QUERY_CROP_START_X){
                    if(_queryCropStart(imgFmt,QueryInput,QueryRst) == 0){
                        ISPUTIL_ERR("unsupported format:0x%x\n",imgFmt);
                    }
                }

                break;
            case eImgFmt_FG_BAYER8:
            case eImgFmt_FG_BAYER10:
            case eImgFmt_FG_BAYER12:
                if(_queryXsize_rrzo(imgFmt,xsize,outWidth,e_PixMode) == 0)
                    return 0;
                if(e_Op & (ISP_QUERY_X_PIX|ISP_QUERY_CROP_X_PIX)){
                    QueryRst.x_pix = outWidth;
                }
                if(e_Op & ISP_QUERY_STRIDE_PIX) {
                    QueryRst.stride_pix = outWidth / queryBitPerPix(imgFmt);
                }
                if(e_Op & ISP_QUERY_STRIDE_BYTE){
                    QueryRst.stride_byte = xsize;
                }
                if(e_Op & (ISP_QUERY_XSIZE_BYTE|ISP_QUERY_CROP_X_BYTE)){
                    QueryRst.xsize_byte = xsize;
                }
                if(e_Op & ISP_QUERY_CROP_START_X){
                    if(_queryCropStart(imgFmt,QueryInput,QueryRst) == 0){
                        ISPUTIL_ERR("unsupported format:0x%x\n",imgFmt);
                    }
                }
            break;
        default:
            {
                if(_queryXsize_p2(imgFmt,xsize,outWidth) == 0)
                    return 0;
                if(e_Op & ISP_QUERY_X_PIX){
                    QueryRst.x_pix = outWidth;
                }
                if(e_Op & ISP_QUERY_STRIDE_PIX) {
                    QueryRst.stride_pix = outWidth / queryBitPerPix(imgFmt);
                }
                if(e_Op & ISP_QUERY_STRIDE_BYTE){
                    QueryRst.stride_byte = xsize;
                }
                if(e_Op & ISP_QUERY_XSIZE_BYTE){
                    QueryRst.xsize_byte = xsize;
                }
            }
            //xsize = outWidth = 0;
            //ISPUTIL_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            break;
        }
    }
    else{
        switch(portId){
            case EPortIndex_IMGO_D:
            case EPortIndex_IMGO:
            case EPortIndex_CAMSV_IMGO:
            case EPortIndex_CAMSV2_IMGO:
                if(e_Op & (ISP_QUERY_CROP_X_PIX|ISP_QUERY_CROP_X_BYTE))
                    _bCrop = MTRUE;
                else
                    _bCrop = MFALSE;

                if(_queryXsize_imgo(imgFmt,xsize,outWidth,e_PixMode,_bCrop) == 0)
                    return 0;
                if(e_Op & (ISP_QUERY_X_PIX|ISP_QUERY_CROP_X_PIX)){
                    QueryRst.x_pix = outWidth;
                }
                if(e_Op & ISP_QUERY_STRIDE_PIX) {
                    QueryRst.stride_pix = xsize / queryBitPerPix(imgFmt);
                }
                if(e_Op & ISP_QUERY_STRIDE_BYTE){
                    QueryRst.stride_byte = xsize;
                    _query_p2_stride_constraint(QueryRst.stride_byte);
                }
                if(ISP_QUERY_XSIZE_BYTE|ISP_QUERY_CROP_X_BYTE){
                    QueryRst.xsize_byte = xsize;
                }
                break;
                if(e_Op & ISP_QUERY_CROP_START_X){
                    if(_queryCropStart(imgFmt,QueryInput,QueryRst) == 0){
                        ISPUTIL_ERR("unsupported format:0x%x\n",imgFmt);
                    }
                }
            case EPortIndex_UFEO:
            case EPortIndex_UFDI:
                MUINT32 C2,C3,C4,C5,C6,C7;
                if(QueryInput & 0x1){
                    QueryInput += 0x1;
                }
                switch(imgFmt)
                {
                    case eImgFmt_BAYER8:    //p2 weighting table input when p2b
                    case eImgFmt_Y8:        //p2 weighting table input when mfb mixing
                    case eImgFmt_BAYER10:
                    case eImgFmt_BAYER12:
                        if(_queryXsize_p2(imgFmt,xsize,outWidth) == 0)
                            return 0;
                        break;
                    default:    //ufo format in(length table)
                        C2 = QueryInput;
                        C3 = 64; //pixel
                        C4 = 4;
                        C5 = (MINT32)((C2+(C3-1))/C3);
                        C6 = C5*C4;
                        C7 = 128; // bits = 16 bytes
                        xsize = ((MINT32)((C6+(C7-1))/C7)*128)/8;
                        break;
                }
                if(e_Op & ISP_QUERY_X_PIX){
                    QueryRst.x_pix = QueryInput;
                }
                if(e_Op & ISP_QUERY_STRIDE_PIX) {
                    QueryRst.stride_pix = xsize / queryBitPerPix(imgFmt);
                }
                if(e_Op & ISP_QUERY_STRIDE_BYTE){
                    QueryRst.stride_byte = xsize;
                }
                if(e_Op & ISP_QUERY_XSIZE_BYTE){
                    QueryRst.xsize_byte = xsize;
                }
                break;
            case EPortIndex_RRZO_D:
            case EPortIndex_RRZO:
                if(_queryXsize_rrzo(imgFmt,xsize,outWidth,e_PixMode) == 0)
                    return 0;
                if(e_Op & (ISP_QUERY_X_PIX|ISP_QUERY_CROP_X_PIX)){
                    QueryRst.x_pix = outWidth;
                }
                if(e_Op & ISP_QUERY_STRIDE_PIX) {
                    QueryRst.stride_pix = outWidth / queryBitPerPix(imgFmt);
                }
                if(e_Op & ISP_QUERY_STRIDE_BYTE){
                    QueryRst.stride_byte = xsize;
                }
                if(ISP_QUERY_XSIZE_BYTE|ISP_QUERY_CROP_X_BYTE){
                    QueryRst.xsize_byte = xsize;
                }
                if(e_Op & ISP_QUERY_CROP_START_X){
                    if(_queryCropStart(imgFmt,QueryInput,QueryRst) == 0){
                        ISPUTIL_ERR("unsupported format:0x%x\n",imgFmt);
                    }
                }
                break;
            case EPortIndex_IMGI:
                switch(imgFmt)
                {
                    case eImgFmt_BAYER8:
                    case eImgFmt_BAYER10:
                    case eImgFmt_BAYER12:
                    case eImgFmt_FG_BAYER8:
                    case eImgFmt_FG_BAYER10:
                    case eImgFmt_FG_BAYER12:
                    case eImgFmt_YUY2:
                    case eImgFmt_RGB888:
                    case eImgFmt_RGB565:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_VIPI:
                switch(imgFmt)
                {
                    case eImgFmt_YUY2:
                    case eImgFmt_YV12:
                    case eImgFmt_I420:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_VIP2I:
                switch(imgFmt)
                {
                    case eImgFmt_NV16:
                    case eImgFmt_NV12:
                    case eImgFmt_YV12:
                    case eImgFmt_I420:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_VIP3I:
                switch(imgFmt)
                {
                    case eImgFmt_NV16:
                    case eImgFmt_NV12:
                    case eImgFmt_YV12:
                    case eImgFmt_I420:
                    case eImgFmt_BAYER8:    //mfb blend/mix path weighting table
                    case eImgFmt_Y8:        //mfb blend/mix path weighting table
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_LCEI:       //lcei would set via imageio, only use p2 weighting table input when p2b
                switch(imgFmt)
                {
                    case eImgFmt_BAYER8:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_IMG2O:
                switch(imgFmt)
                {
                    case eImgFmt_YUY2:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_IMG3O:
                switch(imgFmt)
                {
                    case eImgFmt_YUY2:
                    case eImgFmt_YV12:
                    case eImgFmt_I420:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_IMG3BO:
            case EPortIndex_IMG3CO:
                switch(imgFmt)
                {
                    case eImgFmt_NV16:
                    case eImgFmt_NV12:
                    case eImgFmt_YV12:
                    case eImgFmt_I420:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
            case EPortIndex_MFBO:
                switch(imgFmt)
                {
                    case eImgFmt_BAYER8:
                    case eImgFmt_Y8:
                        p2_cfg=true;
                        break;
                    default:
                        ISPUTIL_ERR("DMA(%d) DO NOT SUPPORT THIS FORMAT(0x%x)",portId,imgFmt);
                        return 0;
                        break;
                }
                break;
           case EPortIndex_FEO:
                //do nothing, the xsize of feo should be calculated in PostProcPipe according to different block number
                break;
#if 0
            case EPortIndex_IMGI:

                break;
#endif
            default:
                ISPUTIL_ERR("current portID(0x%x) r not supported in query\n",portId);
                QueryRst.x_pix = QueryRst.stride_pix = QueryRst.stride_byte = 0;
                return 0;
                break;
        }

        if(p2_cfg)
        {
            if(_queryXsize_p2(imgFmt,xsize,outWidth) == 0)
                return 0;
            if(e_Op & ISP_QUERY_X_PIX){
                QueryRst.x_pix = outWidth;
            }
            if(e_Op & ISP_QUERY_STRIDE_PIX) {
                QueryRst.stride_pix = outWidth / queryBitPerPix(imgFmt);
            }
            if(e_Op & ISP_QUERY_STRIDE_BYTE){
                QueryRst.stride_byte = xsize;
            }
            if(e_Op & ISP_QUERY_XSIZE_BYTE){
                QueryRst.xsize_byte = xsize;
            }
        }
    }
    ISPUTIL_MSG("id:%d fmt:0x%x in_w:%d pixMode:%d pix_x:%d stride_pix:%d stride_byte:%d, xsize_byte:%d\n",portId,\
        imgFmt,QueryInput,e_PixMode,QueryRst.x_pix,QueryRst.stride_pix,QueryRst.stride_byte,QueryRst.xsize_byte);
    return 1;
}


//for backward compatible, use 2-pix mode to query
static MUINT32
queryRRZOXSize(MUINT32 const imgFmt, MUINT32 const imgWidth)
{
    ISP_QUERY_RST QueryRst;
    if(ISP_QuerySize(EPortIndex_RRZO,ISP_QUERY_X_PIX,(EImageFormat)imgFmt,imgWidth,QueryRst) == 0){
        ISPUTIL_ERR("rrzo xsize fail\n");
        return imgWidth;
    }else{
        return QueryRst.x_pix;
    }
}

//for backward compatible
static MUINT32
queryRawBitPerPixel(MUINT32 const imgFmt)
{
    return queryBitPerPix(imgFmt);
}


//for querying suggested burst queue number
static MUINT32
querySuggBurstQNum(MINT32 const fps, MINT32& sugBurstQNum)
{
    switch(fps)
    {
        case 30:
        default:
            sugBurstQNum=1;
            break;
        case 60:
            sugBurstQNum=2;
            break;
        case 120:
            sugBurstQNum=2;
            break;
    }
    return 0;
}


};  //namespace NSIspio
};  //namespace NSImageio

#endif  //  _ISPIO_PIPE_UTILITY_H_



