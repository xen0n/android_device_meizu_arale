/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief

#define LOG_TAG "CamShotTest"


#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>

#include <utils/StrongPointer.h>
using namespace android;
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
using namespace NSCamShot;

extern "C" {
#include <pthread.h>
}

//
#include <mtkcam/hal/IHalSensor.h>
using namespace NSCam;


/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

static pthread_t g_CliKeyThreadHandle;
static MBOOL g_bIsCLITest = MTRUE;

static EImageFormat g_eImgFmt[] = {
    eImgFmt_YUY2,
    eImgFmt_NV21,
    eImgFmt_I420,
    eImgFmt_YV16,
    eImgFmt_I422,
    eImgFmt_JPEG
} ;

static sp<IImageBuffer> g_ImgBufRaw;
static sp<IImageBuffer> g_ImgBufYuv;
static sp<IImageBuffer> g_ImgBufPostview;
static sp<IImageBuffer> g_ImgBufJpeg;

//sensor infos
static MUINT32 g_sensorNum = 0;
static MUINT32 g_sensorIdx = 0;
static SensorStaticInfo* g_pSensorInfo = NULL;

static MUINT32 g_u4Mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
static MUINT32 g_u4Width = 3200;
static MUINT32 g_u4Height = 2400;
static MUINT32 g_u4EnableMsg = 0x8;    //default jpeg
static MUINT32 g_u4RegisterDataMsg = 0x0;
static MUINT32 g_u4RegYuvFmt = 0x0;
static MUINT32 g_u4Rot = 0;
static MUINT32 g_u4SensorWidth = 0; //should query from sensor
static MUINT32 g_u4SensorHeight = 0;
static MUINT32 g_u4PostviewWidth = 640;
static MUINT32 g_u4PostviewHeight = 480;
static MUINT32 g_u4ShotCnt = 1;
static MUINT32 u4CapCnt = 0;
static MUINT32 u4RawType = 1;

/******************************************************************************
*
*******************************************************************************/
static
MVOID allocImageBuffer(sp<IImageBuffer> * pImageBuffer, MUINT32 w, MUINT32 h, MUINT32 fmt)
{
    printf("alloc ImageBuffer %dx%d, fmt 0x%x\n", w, h, fmt);
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    sp<IImageBuffer> pBuf;
    //allocate buffer

    MUINT32 bufStridesInBytes[3] = {0};
    MUINT32 plane = queryPlaneCount(fmt);

    for (MUINT32 i = 0; i < plane; i++)
    {
        bufStridesInBytes[i] =
            (queryPlaneWidthInPixels(fmt,i, w) * queryPlaneBitsPerPixel(fmt,i)) >> 3;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator::ImgParam imgParam(
            fmt, MSize(w,h), bufStridesInBytes, bufBoundaryInBytes, plane
            );

    *pImageBuffer = allocator->alloc_ion(LOG_TAG, imgParam);
    if  ( pImageBuffer->get() == 0 )
    {
        MY_LOGE("NULL Buffer\n");
    }

    if ( !pImageBuffer->get()->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
    {
        MY_LOGE("lock Buffer failed\n");
    }
}


static
MVOID freeImageBuffer(sp<IImageBuffer> * pImageBuffer)
{
    if( !pImageBuffer->get()->unlockBuf( LOG_TAG ) )
    {
        CAM_LOGE("unlock Buffer failed\n");
    }
    pImageBuffer = NULL;
}
/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    if( size )
    {
        printf("save to file (name, buf, size) = (%s, 0x%x, %d)\n", fname, buf, size);
        MY_LOGD("save to file (name, buf, size) = (%s, 0x%x, %d)", fname, buf, size);

        MY_LOGD("opening file [%s]\n", fname);
        int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
        if (fd < 0) {
            MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
            return false;
        }

        MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
        while (written < size) {
            nw = ::write(fd,
                    buf + written,
                    size - written);
            if (nw < 0) {
                MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
                break;
            }
            written += nw;
            cnt++;
        }
        MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
        ::close(fd);
    }
    else
    {
        printf("skip due to size == 0");
        MY_LOGD("skip due to size == 0");
    }
    return true;
}


/******************************************************************************
*   read the file to the buffer
*******************************************************************************/
static uint32_t
loadFileToBuf(char const*const fname, uint8_t*const buf, uint32_t size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    MY_LOGD("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                    buf + readCnt,
                    size - readCnt);
        if (nr < 0) {
            MY_LOGE("failed to read from file [%s]: %s",
                        fname, strerror(errno));
            break;
        }
        readCnt += nr;
        cnt++;
    }
    MY_LOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}


/////////////////////////////////////////////////////////////////////////
//! Nucamera commands
/////////////////////////////////////////////////////////////////////////
typedef struct CLICmd_t
{
    //! Command string, include shortcut key
    const char *pucCmdStr;

    //! Help string, include functionality and parameter description
    const char *pucHelpStr;

    //! Handling function
    //! \param a_u4Argc  [IN] Number of arguments plus 1
    //! \param a_pprArgv [IN] Array of command and arguments, element 0 is
    //!                       command string
    //! \return error code
    //FIXME: return MRESULT is good?
    MUINT32 (*handleCmd)(const int argc, char** argv);

} CLICmd;



/******************************************************************************
*
*******************************************************************************/
static MBOOL fgCamShotNotifyCb(MVOID* user, CamShotNotifyInfo const msg)
{
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
static MBOOL handleBayerDataCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handleBayerDataCallback");
    char fileName[256] = {'\0'};
    sprintf(fileName, "/data/bayer_%ux%u_%02d.raw", g_u4SensorWidth, g_u4SensorHeight, u4CapCnt);
    msg.pBuffer->saveToFile(fileName);
    printf("save to file (name, pImgBuf) = (%s, 0x%x)\n",
            fileName, msg.pBuffer);
    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MBOOL handleYuvDataCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handleYuvDataCallback");
    char fileName[256] = {'\0'};
    sprintf(fileName, "/data/yuv_%ux%u_%02d.yuv", g_u4Width, g_u4Height, u4CapCnt);
    printf("save to file (name, pImgBuf) = (%s, 0x%x)\n",
            fileName, msg.pBuffer);
    msg.pBuffer->saveToFile(fileName);
    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MBOOL handlePostViewCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handlePostViewCallback");
    char fileName[256] = {'\0'};
    sprintf(fileName, "/data/postview_%ux%u_%02d.yuv",
            g_u4PostviewWidth, g_u4PostviewHeight, u4CapCnt);
    printf("save to file (name, pImgBuf) = (%s, 0x%x)\n",
            fileName, msg.pBuffer);
    msg.pBuffer->saveToFile(fileName);
    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MBOOL handleJpegCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handleJpegCallback");
    char fileName[256] = {'\0'};
    sprintf(fileName, "/data/jpeg%02d.jpg", u4CapCnt);
    saveBufToFile(fileName, (MUINT8*)msg.pBuffer->getBufVA(0), msg.pBuffer->getBitstreamSize());

    IImageBuffer* pThumbImg = reinterpret_cast<IImageBuffer*>(msg.ext1);
    memset(fileName, '\0', 256);
    sprintf(fileName, "/data/thumb%02d.jpg", u4CapCnt);
    saveBufToFile(fileName, (MUINT8*)pThumbImg->getBufVA(0), pThumbImg->getBitstreamSize());

    return 0;
}



/******************************************************************************
*
*******************************************************************************/
static MBOOL fgCamShotDataCb(MVOID* user, CamShotDataInfo const msg)
{
    CamShotDataInfo rDataInfo = msg;

    switch (rDataInfo.msgType)
    {
        case ECamShot_DATA_MSG_RAW:
            handleBayerDataCallback(msg);
        break;
        case ECamShot_DATA_MSG_YUV:
            handleYuvDataCallback(msg);
        break;
        case ECamShot_DATA_MSG_POSTVIEW:
            handlePostViewCallback(msg);
        break;
        case ECamShot_DATA_MSG_JPEG:
            handleJpegCallback(msg);
        break;

    }

    return MTRUE;
}

static MVOID allocateMem(MUINT32 u4EnableMsg)
{
#warning [TODO] allocate memory according to the format
    if (u4EnableMsg & ECamShot_BUF_TYPE_RAW)
    {
        //FIXME, size shuld query from sensor
        allocImageBuffer(&g_ImgBufRaw, g_u4SensorWidth , g_u4SensorHeight , eImgFmt_BAYER10);
    }

    if (u4EnableMsg & ECamShot_BUF_TYPE_YUV)
    {
        allocImageBuffer(&g_ImgBufYuv, g_u4Width , g_u4Height , g_eImgFmt[g_u4RegYuvFmt]);
    }

    if (u4EnableMsg & ECamShot_BUF_TYPE_POSTVIEW)
    {
        allocImageBuffer(&g_ImgBufPostview, g_u4Width , g_u4Height , eImgFmt_YV12);
    }

    if (u4EnableMsg & ECamShot_BUF_TYPE_JPEG)
    {
        allocImageBuffer(&g_ImgBufJpeg, g_u4Width, g_u4Height, eImgFmt_JPEG);
    }
}

static MVOID freeMem()
{
    if( g_ImgBufRaw.get() )
        freeImageBuffer(&g_ImgBufRaw);

    if( g_ImgBufYuv.get() )
        freeImageBuffer(&g_ImgBufYuv);

    if( g_ImgBufPostview.get() )
        freeImageBuffer(&g_ImgBufPostview);

    if( g_ImgBufJpeg.get() )
        freeImageBuffer(&g_ImgBufJpeg);
}


/******************************************************************************
*
*******************************************************************************/
static MVOID dumpInfos()
{
    printf("================\n");
    printf("sensor idx %d / %d\n",
            g_sensorIdx, g_sensorNum);
    printf("cap mode     : %d,  rawType: %d\n",
            g_u4Mode, u4RawType);
    printf("capture width: %d, height: %d, rot: %d,count: %d\n",
            g_u4Width, g_u4Height, g_u4Rot ,g_u4ShotCnt);
    printf("postview width: %d, height: %d\n",
            g_u4PostviewWidth, g_u4PostviewHeight);

#define PrintDataEnabled( str, enabledmsg, datamsg ) \
    do{                                              \
        if( enabledmsg & datamsg )                   \
        {                                            \
            printf("enable %s %s\n", str,#datamsg);  \
        }                                            \
    }while(0)

    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_RAW);
    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_YUV);
    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_POSTVIEW);
    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_JPEG);

    PrintDataEnabled( "register", g_u4RegisterDataMsg, ECamShot_BUF_TYPE_RAW);
    PrintDataEnabled( "register", g_u4RegisterDataMsg, ECamShot_BUF_TYPE_YUV);
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_YUV )
    {
        printf("register yuv format %d\n", g_u4RegYuvFmt);
    }
    PrintDataEnabled( "register", g_u4RegisterDataMsg, ECamShot_BUF_TYPE_POSTVIEW);
    PrintDataEnabled( "register", g_u4RegisterDataMsg, ECamShot_BUF_TYPE_JPEG);
#undef PrintDataEnabled
    printf("================\n");

}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_sensorIdx(int argc, char** argv)
{
    MUINT32 idx = 0;
    if (argc == 1)
    {
        idx = atoi(argv[0]);
        if( idx < g_sensorNum )
        {
            g_sensorIdx = idx;
            return MTRUE;
        }
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_mode(int argc, char** argv)
{
    MUINT32 u4Mode = 0;
    if (argc == 1)
    {
        u4Mode = atoi(argv[0]);
        if( u4Mode == 0 || u4Mode == 1 )
        {
            g_u4Mode = u4Mode;
            return MTRUE;
        }
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_rawType(int argc, char** argv)
{
    MUINT32 u4type= 0;
    if (argc == 1)
    {
        u4type = atoi(argv[0]);
        if( u4type == 0 || u4type == 1 )
        {
            u4RawType = u4type;
            return MTRUE;
        }
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_picsize(int argc, char** argv)
{
    MUINT32 w,h = 0;
    if (argc == 2)
    {
        w = atoi(argv[0]);
        h = atoi(argv[1]);
        if( w > 0 && h > 0 )
        {
            g_u4Width = w;
            g_u4Height = h;
            return MTRUE;
        }
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_data(int argc, char** argv)
{
    MUINT32 msg;
    if (argc == 1)
    {
        sscanf( argv[0],"0x%x", &msg);
        if( msg )
        {
            g_u4EnableMsg = msg;
            return MTRUE;
        }
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_rot(int argc, char** argv)
{
    MUINT32 u4rot = 0;
    if (argc == 1)
    {
        u4rot = atoi(argv[0]);
        g_u4Rot = u4rot;
        return MTRUE;
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_count(int argc, char** argv)
{
    MUINT32 u4cnt = 0;
    if (argc == 1)
    {
        u4cnt = atoi(argv[0]);
        if( u4cnt )
        {
            g_u4ShotCnt = u4cnt;
            return MTRUE;
        }
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Update_reg(int argc, char** argv)
{
    MUINT32 msg;
    if (argc == 1)
    {
        sscanf( argv[0],"0x%x", &msg);
        if( msg )
        {
            g_u4RegisterDataMsg = msg;
            MUINT32 newdata = 0;
            switch(msg)
            {
                case 0:
                    g_u4EnableMsg = 0;
                    break;
                case  ECamShot_BUF_TYPE_RAW:
                    g_u4EnableMsg |= ECamShot_DATA_MSG_RAW;
                    break;
                case  ECamShot_BUF_TYPE_YUV:
                    g_u4EnableMsg |= ECamShot_DATA_MSG_YUV;
                    break;
                case  ECamShot_BUF_TYPE_POSTVIEW:
                    g_u4EnableMsg |= ECamShot_DATA_MSG_POSTVIEW;
                    break;
                case  ECamShot_BUF_TYPE_JPEG:
                    g_u4EnableMsg |= ECamShot_DATA_MSG_JPEG;
                    break;
                case  ECamShot_BUF_TYPE_THUMBNAIL:
                default:
                    printf("wrong buf type (0x%x)", msg);
                    return MFALSE;
                    break;
            }
            return MTRUE;
        }
    }

    printf("wrong input\n");
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Capture_Cmd(int argc, char** argv)
{
    MUINT32 u4Transform;

    switch(g_u4Rot)
    {
        case 0:
            u4Transform = 0;
            break;
        case 1:
            u4Transform = eTransform_ROT_90;
            break;
        case 2:
            u4Transform = eTransform_ROT_180;
            break;
        case 3:
            u4Transform = eTransform_ROT_270;
            break;
        default:
            u4Transform = 0;
            break;
    }

    ISingleShot *pSingleShot = ISingleShot::createInstance(eShotMode_NormalShot, "testshot");
    //
    pSingleShot->init();
    //
    pSingleShot->enableDataMsg( g_u4EnableMsg );
    // set buffer
    //
    // register buffer
    allocateMem(g_u4RegisterDataMsg);
//    //
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_RAW )
        pSingleShot->registerImageBuffer(ECamShot_BUF_TYPE_RAW, g_ImgBufRaw.get());
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_YUV )
        pSingleShot->registerImageBuffer(ECamShot_BUF_TYPE_YUV, g_ImgBufYuv.get());
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_POSTVIEW )
        pSingleShot->registerImageBuffer(ECamShot_BUF_TYPE_POSTVIEW, g_ImgBufPostview.get());
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_JPEG )
        pSingleShot->registerImageBuffer(ECamShot_BUF_TYPE_JPEG, g_ImgBufJpeg.get());

//    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_YUV, g_rYuvBufInfo);
//    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_POSTVIEW, g_rPostViewBufInfo);
//    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_JPEG, g_rJpegBufInfo);


    // shot param
    ShotParam rShotParam(eImgFmt_YUY2,           //yuv format
                         g_u4Width,              //picutre width
                         g_u4Height,             //picture height
                         u4Transform,            //picutre transform
                         eImgFmt_YV12,           //postview format
                         800,                    //postview width
                         480,                    //postview height
                         0,                      //postview transform
                         100                     //zoom
                        );

    // jpeg param
    JpegParam rJpegParam(ThumbnailParam(160, 128, 100, MTRUE),
                         90,                     //Quality
                         MTRUE                   //isSOI
                        );

    // thumbnail param
    ThumbnailParam rThumbnailParam(160,          // thumbnail width
                                   128,          // thumbnail height
                                   100,          // quality
                                   MTRUE         // isSOI
                                  );

    // sensor param
    SensorParam rSensorParam(
            g_sensorIdx,                         //open ID
            g_u4Mode == 0 ?
            SENSOR_SCENARIO_ID_NORMAL_PREVIEW :
            SENSOR_SCENARIO_ID_NORMAL_CAPTURE,   //Scenaio
            10,                                  //bit depth
            MFALSE,                              //bypass delay
            MFALSE,                              //bypass scenario
            u4RawType                            //rawType
            );
    // update sensor's size
    if( g_u4Mode == 0 ) // preview
    {
        g_u4SensorWidth  = g_pSensorInfo[g_sensorIdx].previewWidth;
        g_u4SensorHeight = g_pSensorInfo[g_sensorIdx].previewHeight;
    }
    else if( g_u4Mode == 1 )//capture
    {
        g_u4SensorWidth  = g_pSensorInfo[g_sensorIdx].captureWidth;
        g_u4SensorHeight = g_pSensorInfo[g_sensorIdx].captureHeight;
    }
    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, NULL);
    //
    pSingleShot->setShotParam(rShotParam);
    //
    pSingleShot->setJpegParam(rJpegParam);
    //
    //
    u4CapCnt = 0;
    for (MUINT32 i = 0 ; i < g_u4ShotCnt; i++)
    {
        printf("startOne count(0x%x)+\n", i);
        pSingleShot->startOne(rSensorParam);
        printf("startOne count(0x%x)-\n", i);
        u4CapCnt++;
    }
    //
    pSingleShot->uninit();
    //
    pSingleShot->destroyInstance();

    freeMem();

    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//!  The cli command for the manucalibration
//!
/////////////////////////////////////////////////////////////////////////
static CLICmd g_rTest_Cmds[] =
{
    { "sensor"    , " <sensor idx>                                            " , u4Update_sensorIdx } ,
    { "mode"      , " <mode:0:prv, 1:cap>                                     " , u4Update_mode    } ,
    { "rawtype"   , " <rawType: 0:pureraw, 1:processedraw>                    " , u4Update_rawType } ,
    { "picsize"   , " <width> <height>                                        " , u4Update_picsize } ,
    { "data"      , " <img:0x1:raw, 0x2:yuv, 0x4:postview, 0x8:jpeg>          " , u4Update_data    } ,
    { "rot"       , " <rot:0, 1:90, 2:180, 3:270>                             " , u4Update_rot     } ,
    { "count"     , " <Count>                                                 " , u4Update_count   } ,
    { "reg"       , " <register buf:0x1:raw, 0x2:yuv, 0x4:postview, 0x8:jpeg> " , u4Update_reg     } ,
    { "regYuvFmt" , " <fmt:0:YUY2, 1:NV21, 2:I420, 3:YV16 4:I422>             " , u4Update_reg     } ,
    { "exe"       , " execute                                                 " , u4Capture_Cmd    } ,
    {NULL, NULL, NULL}
};


/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () -
//! @brief the CLI key input thread, wait for CLI command
//! @param sig: The input arguments
/////////////////////////////////////////////////////////////////////////
static void thread_exit_handler(int sig)
{
    printf("This signal is %d \n", sig);
    pthread_exit(0);
}

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
static void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;

    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


//  vHelp () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
static void vHelp()
{
    printf("\n***********************************************************\n");
    printf("* CamShot SingleShot CLI Test                             *\n");
    printf("* Current Support Commands                                *\n");
    printf("===========================================================\n");

    printf("help/h      [ Help ]\n");
    printf("exit/q      [ Exit ]\n");

    int i = 0;
    for (i = 0; ; i++)
    {
        if (NULL == g_rTest_Cmds[i].pucCmdStr)
        {
            break;
        }
        printf("%-10s  [%s]\n",
                g_rTest_Cmds[i].pucCmdStr, g_rTest_Cmds[i].pucHelpStr);
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () -
//! @brief the CLI key input thread, wait for CLI command
//! @param a_pArg: The input arguments
/////////////////////////////////////////////////////////////////////////
static void* cliKeyThread (void *a_pArg)
{
    char urCmds[256] = {0};

    //! ************************************************
    //! Set the signal for kill self thread
    //! this is because android don't support thread_kill()
    //! So we need to creat a self signal to receive signal
    //! to kill self
    //! ************************************************
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    int rc = sigaction(SIGUSR1,&actions,NULL);

    dumpInfos();
    while (1)
    {
        printf("Input Cmd#");
        fgets(urCmds, 256, stdin);

        //remove the '\n'
        urCmds[strlen(urCmds)-1] = '\0';
        char *pCmds = &urCmds[0];
        //remove the space in the front of the string
        vSkipSpace(&pCmds);

        //Ignore blank command
        if (*pCmds == '\0')
        {
            continue;
        }

        //Extract the Command  and arguments where the argV[0] is the command
        MUINT32 u4ArgCount = 0;
        char  *pucStrToken, *pucCmdToken;
        char  *pucArgValues[25];

        pucStrToken = (char *)strtok(pCmds, " ");
        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] =(char*) pucStrToken;
            pucStrToken = (char*)strtok (NULL, " ");
        }

        if (u4ArgCount == 0)
        {
            continue;
        }

        pucCmdToken = (char*) pucArgValues[0];

        //parse the command
        if ((strcmp((char *)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp();
        }
        else if ((strcmp((char *)pucCmdToken, "exit") == 0) ||
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            printf("Exit From CLI\n");
            g_bIsCLITest = MFALSE;
            break;
        }
        else
        {
            MBOOL bIsFoundCmd = MFALSE;
            for (MUINT32 u4CmdIndex = 0; ; u4CmdIndex++)
            {
                if(NULL == g_rTest_Cmds[u4CmdIndex].pucCmdStr)
                {
                    break;
                }
                if (strcmp((char *)pucCmdToken, g_rTest_Cmds[u4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = MTRUE;
                    g_rTest_Cmds[u4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);
                    dumpInfos();
                    break;
                }
            }
            if (bIsFoundCmd == MFALSE)
            {
                printf("Invalid Command\n");
            }
        }

    }

    return 0;
}




/*******************************************************************************
*  Main Function
********************************************************************************/
int main_singleshot(int argc, char** argv)
{
    printf("SingleShot Test \n");

    printf("sensor list get\n");
    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    printf("sensor search\n");
    pHalSensorList->searchSensors();
    printf("search sensor end\n");

    g_sensorNum = pHalSensorList->queryNumberOfSensors();

    g_pSensorInfo = new SensorStaticInfo[g_sensorNum];

    for( int i = 0 ; i < g_sensorNum ; i++ )
    {
        pHalSensorList->querySensorStaticInfo(
                             pHalSensorList->querySensorDevIdx(i),
                             g_pSensorInfo + i);
    }

    vHelp();


    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************
    while (g_bIsCLITest== MTRUE)
    {
        usleep(100000);
    }

    delete [] g_pSensorInfo;

    return 0;
}
