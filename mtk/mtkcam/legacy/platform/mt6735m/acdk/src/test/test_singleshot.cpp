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

#if (MTKCAM_BASIC_PACKAGE != 1)
#include <linux/cache.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
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

// Refer AcdkMain.cpp to add header files for adding preview flow (need add 3A)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>

#include <cutils/properties.h>

extern "C" {
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <semaphore.h>
//#include "mtkfb.h"  //TO Do : Temp for MT6595DEV, marked by Jessy to avoid the build error
}
//#include <cutils/pmem.h>

#include <mtkcam/imageio/IPipe.h>
using namespace NSImageio;
using namespace NSIspio;
#include "mtkcam/hal/IHalSensor.h"
#include "kd_imgsensor_define.h"
#include <mtkcam/drv_common/imem_drv.h>
//for normal pipe function
#include <mtkcam/iopipe/CamIO/INormalPipe.h>

#include <mtkcam/camshot/ICamShot.h>
#include "mtkcam/exif/IBaseCamExif.h"
#include "mtkcam/exif/CamExif.h"

#include <binder/IMemory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/RefBase.h>
#include <system/camera.h>
#include <hardware/camera.h>
#include <dlfcn.h>
#include <camera/MtkCamera.h>



namespace
{
    camera_module_t *mModule = NULL;
    camera_device_t *mDevice = NULL;
    int mNumberOfCameras = 0;
    IHalSensor* m_pSensorHalObj = NULL;
    MUINT32 msensorArray[1] = {0};

}


using namespace android;
using namespace NSCamShot;

//#include <mtkcam/featureio/IHal3A.h>
//using namespace NS3A;




/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

static IHalSensor *g_pIHalSensor = NULL;

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
static MUINT32 g_u4ShotCnt = 3; // Take 3 images & compare their CRC values
static MUINT32 u4CapCnt = 0;
static MUINT32 u4RawType = 1;

// For CRC
static MUINT32 u4CRC = 0;
static MUINT32 u4CRCRef = 1368415439; // The reference CRC value of test pattern

//static acdkObserver g_acdkMhalObserver;
static MBOOL mCaptureDone = MFALSE;


/*******************************************************************************
*  Calculates the CRC-8 of the first len bits in data
********************************************************************************/
static const MUINT32 ACDK_CRC_Table[256]=
{
    0x0,        0x4C11DB7,  0x9823B6E,  0xD4326D9,  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,  0x54BF6A4,  0x808D07D,  0xCC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x315D626,  0x7D4CB91,  0xA97ED48,  0xE56F0FF,  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x29F3D35,  0x65E2082,  0xB1D065B,  0xFDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};  // Table of 8-bit remainders



/******************************************************************************
*
*******************************************************************************/
static
MVOID allocImageBuffer(sp<IImageBuffer> * pImageBuffer, MUINT32 w, MUINT32 h, MUINT32 fmt)
{
    printf("alloc ImageBuffer %dx%d, fmt 0x%x", w, h, fmt);
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

    printf("save to file (name, buf, size) = (%s, %p, %d)\n", fname, buf, size);
    MY_LOGD("save to file (name, buf, size) = (%s, %p, %d)", fname, buf, size);

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

    /*
    char fileName[256] = {'\0'};
    sprintf(fileName, "/data/bayer_%ux%u_%02d.raw", g_u4SensorWidth, g_u4SensorHeight, u4CapCnt);
    saveBufToFile(fileName, msg.puData, msg.u4Size);
    */


    // For CRC Check
    register MUINT32 crc_accum = 0;
    MUINT32 size = msg.u4Size;
    MUINT8 *out_buffer = (MUINT8*)msg.puData;

    //MY_LOGD("[Camera_check_crc]\n");
    MY_LOGD("[Camera_check_crc]\n");


    while (size-- > 0)
    {
        crc_accum = (crc_accum << 8) ^ ACDK_CRC_Table[(MUINT8)(crc_accum >> 24) ^ (*out_buffer++)];
    }

#if 0
    if(u4CapCnt ==0)
    {
        u4CRCRef = ~crc_accum;
        printf("CRC_accum 0x%x\n", ~crc_accum);
    }
    else
#endif

    u4CRC = ~crc_accum;

    MY_LOGD("Current capture count:%2d, CRC value:%d\n", u4CapCnt, u4CRC);
    if(u4CRC != u4CRCRef)
    {
        MY_LOGD("=========CRC Different!=========\nCRC value:%d, Ref CRC value:%d\n", u4CRC, u4CRCRef);
    }

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

    saveBufToFile(fileName ,msg.puData , msg.u4Size);
//    msg.pBuffer->saveToFile(fileName);
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
    saveBufToFile(fileName ,msg.puData , msg.u4Size);
//    msg.pBuffer->saveToFile(fileName);
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
    saveBufToFile(fileName ,msg.puData , msg.u4Size);
//    saveBufToFile(fileName, (MUINT8*)msg.pBuffer->getBufVA(0), msg.pBuffer->getBitstreamSize());
#if 0
    IImageBuffer* pThumbImg = reinterpret_cast<IImageBuffer*>(msg.ext1);
    memset(fileName, '\0', 256);
    sprintf(fileName, "/data/thumb%02d.jpg", u4CapCnt);
    saveBufToFile(fileName, (MUINT8*)pThumbImg->getBufVA(0), pThumbImg->getBitstreamSize());
#endif
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
        case ECamShot_DATA_MSG_BAYER:
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
    if (u4EnableMsg & ECamShot_BUF_TYPE_BAYER)
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

    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_BAYER);
    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_YUV);
    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_POSTVIEW);
    PrintDataEnabled( "data", g_u4EnableMsg, ECamShot_DATA_MSG_JPEG);

    PrintDataEnabled( "register", g_u4RegisterDataMsg, ECamShot_DATA_MSG_BAYER);
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
              /*  case 0:
                    g_u4EnableMsg = 0;
                    break;*/
                case  ECamShot_BUF_TYPE_BAYER:
                    g_u4EnableMsg |= ECamShot_DATA_MSG_BAYER;
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
#if 0
//static MUINT32 u4Capture_Cmd(int argc, char** argv)
static MUINT32 captureOneImage()
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
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_BAYER)
        pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_BAYER, g_ImgBufRaw.get());
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_YUV )
        pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_YUV, g_ImgBufYuv.get());
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_POSTVIEW )
        pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_POSTVIEW, g_ImgBufPostview.get());
    if( g_u4RegisterDataMsg & ECamShot_BUF_TYPE_JPEG )
        pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_JPEG, g_ImgBufJpeg.get());


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

    MY_LOGD("RawType:\t%d\n0:pureraw, 1:processedraw, 2:test pattern", u4RawType);


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
        MY_LOGD("startOne count(0x%x)+\n", i);
        pSingleShot->startOne(rSensorParam);
        MY_LOGD("startOne count(0x%x)-\n", i);
        u4CapCnt++;
    }
    //
    pSingleShot->uninit();
    //
    pSingleShot->destroyInstance();

    MY_LOGD("Done acdk singleshot test!");

    freeMem();

    return 0;
}
#endif

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
    //{ "exe"       , " execute                                                 " , u4Capture_Cmd    } ,
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

void setSingleshotParas()
{
    g_u4Mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    g_u4EnableMsg = 0x1; // raw
    g_u4RegisterDataMsg = 0x0;
    g_u4Rot = 0;
    g_u4PostviewWidth = 640;
    g_u4PostviewHeight = 480;
    g_u4ShotCnt = 1;
    u4RawType = 2; // Test pattern

}

void loadModule()
{
    if (hw_get_module(CAMERA_HARDWARE_MODULE_ID,
                (const hw_module_t **)&mModule) < 0) {
        MY_LOGD("Could not load camera HAL module");
        mNumberOfCameras = 0;
    }
    else {
        MY_LOGD("Loaded \"%s\" camera module\n", mModule->common.name);
        mNumberOfCameras = mModule->get_number_of_cameras();
        MY_LOGD("number_of_cameras: %d\n", mNumberOfCameras);
    }
}

void init()
{
    MY_LOGD("sensor list get\n");
    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    MY_LOGD("sensor search\n");
    pHalSensorList->searchSensors();
    MY_LOGD("search sensor end\n");

    g_sensorNum = pHalSensorList->queryNumberOfSensors();

    g_pSensorInfo = new SensorStaticInfo[g_sensorNum];

    for( int i = 0 ; i < g_sensorNum ; i++ )
    {
        pHalSensorList->querySensorStaticInfo(
                             pHalSensorList->querySensorDevIdx(i),
                             g_pSensorInfo + i);
    }


    //vHelp();

    //get sensor test pattern checksum value

    MINT32 err = 0;

    m_pSensorHalObj = pHalSensorList->createSensor("SLT SingleShot", 1, &msensorArray[0]);
    // Open sensor number decide by sensorArray
    m_pSensorHalObj->powerOn("SLT SingleShot", 1, &msensorArray[0]);
}

void uinit()
{

    if(m_pSensorHalObj != NULL)
    {
        m_pSensorHalObj->powerOff("SLT SingleShot", 1, &msensorArray[0]);
        m_pSensorHalObj->destroyInstance("SLT SingleShot");
        m_pSensorHalObj= NULL;
    }
}


void getTestPatternCRCValue()
{
    init();

    MY_LOGD("Get sensor test pattern checksum value start!\n");

    // Get test pattern CRC value
    MINT32 err = m_pSensorHalObj->sendCommand( SENSOR_DEV_MAIN,
                                    SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,
                                    (MUINTPTR)&u4CRCRef,
                                    0,
                                    0);

    MY_LOGD("Get sensor test pattern checksum value done!\n");
    MY_LOGD("CRC address:%p, CRC value from sensorHal:%d\n", &u4CRCRef, u4CRCRef);

    /*
    // Set to test pattern output
    MINT32 u32Enable = 1;
    err = m_pSensorHalObj->sendCommand(SENSOR_DEV_MAIN,
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MINT32)&u32Enable,
                                           0,
                                           0);

    MY_LOGD("Set sensor test pattern output done!");
    */

    uinit();


}

void outputTestPattern()
{

     // Set to test pattern output
    MINT32 u32Enable = 1;
    MINT32 err = m_pSensorHalObj->sendCommand(SENSOR_DEV_MAIN,
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MUINTPTR)&u32Enable,
                                           0,
                                           0);

    MY_LOGD("Set sensor test pattern output done!");

    //uinit();
}

void set(DefaultKeyedVector<String8,String8>& rMap, const char *key, const char *value)
{
    // XXX i think i can do this with strspn()
    if (strchr(key, '=') || strchr(key, ';')) {
        //XXX ALOGE("Key \"%s\"contains invalid character (= or ;)", key);
        return;
    }

    if (strchr(value, '=') || strchr(value, ';')) {
        //XXX ALOGE("Value \"%s\"contains invalid character (= or ;)", value);
        return;
    }

    rMap.replaceValueFor(String8(key), String8(value));
}

status_t openDevice(hw_module_t *module, const char *name)
{
    String8 const mName(name);
    MY_LOGD("Opening camera %s + \n", mName.string());

    // [TODO] This state has problem, need to check Jessy @4/10
    int rc = module->methods->open(module, mName.string(),
                                   (hw_device_t **)&mDevice);
    if (rc != OK) {
        MY_LOGE("Could not open camera %s: %d", mName.string(), rc);
        return rc;
    }

    MY_LOGD("Opening camera %s - \n", mName.string());



    return rc;
}

static void __put_memory(camera_memory_t *data);

class CameraHeapMemory : public RefBase {
public:
    CameraHeapMemory(int fd, size_t buf_size, uint_t num_buffers = 1) :
                     mBufSize(buf_size),
                     mNumBufs(num_buffers)
    {
        mHeap = new MemoryHeapBase(fd, buf_size * num_buffers);
        commonInitialization();
    }

    CameraHeapMemory(size_t buf_size, uint_t num_buffers = 1) :
                     mBufSize(buf_size),
                     mNumBufs(num_buffers)
    {
        mHeap = new MemoryHeapBase(buf_size * num_buffers);
        commonInitialization();
    }

    void commonInitialization()
    {
        handle.data = mHeap->base();
        handle.size = mBufSize * mNumBufs;
        handle.handle = this;

        mBuffers = new sp<MemoryBase>[mNumBufs];
        for (uint_t i = 0; i < mNumBufs; i++)
            mBuffers[i] = new MemoryBase(mHeap,
                                         i * mBufSize,
                                         mBufSize);

        handle.release = __put_memory;
    }

    virtual ~CameraHeapMemory()
    {
        delete [] mBuffers;
    }

    size_t mBufSize;
    uint_t mNumBufs;
    sp<MemoryHeapBase> mHeap;
    sp<MemoryBase> *mBuffers;

    camera_memory_t handle;
};

static void __put_memory(camera_memory_t *data)
{
    if (!data)
        return;

    CameraHeapMemory *mem = static_cast<CameraHeapMemory *>(data->handle);
    mem->decStrong(mem);
}


static void __notify_cb(int32_t msg_type, int32_t ext1,
                        int32_t ext2, void *user)
{
    MY_LOGD("%s", __FUNCTION__);
}


static void __data_cb(int32_t msg_type,
                      const camera_memory_t *data, unsigned int index,
                      camera_frame_metadata_t *metadata,
                      void *user)
{
    MY_LOGD("%s type %x", __FUNCTION__,msg_type);

    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory *>(data->handle));
    if (index >= mem->mNumBufs) {
        MY_LOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }

    ssize_t offset = 0;
    size_t size = 0;
    sp<IMemoryHeap> heap = mem->mBuffers[index]->getMemory(&offset, &size);
    uint8_t* va = (((uint8_t*)heap->getBase())+offset);
    MY_LOGD("%s  va=%p, size=%d, offset=%d", __FUNCTION__, va, size,offset);


    switch  (msg_type & ~CAMERA_MSG_PREVIEW_METADATA)
    {
        case CAMERA_MSG_PREVIEW_FRAME:
             //Callback to upper layer
             //g_acdkMhalObserver.notify(ACDK_CB_PREVIEW,(MUINT32)va,(MUINT32)size,(MUINT32)size,0);
            break;
        case MTK_CAMERA_MSG_EXT_DATA:{
                MtkCamMsgExtDataHelper MtkExtDataHelper;

                if  ( ! MtkExtDataHelper.init(mem->mBuffers[index]) ) {
                    MY_LOGE("[%s] MtkCamMsgExtDataHelper::init fail", __FUNCTION__);
                    return;
                }

                void*   const pvExtParam   = MtkExtDataHelper.getExtParamBase();
                size_t  const ExtParamSize = MtkExtDataHelper.getExtParamSize();
                switch  (MtkExtDataHelper.getExtMsgType())
                {
                    case MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE:
                        //handleMtkExtDataCompressedImage(mem->mBuffers[index]);
                        mCaptureDone = MTRUE;
                        break;

                    default:
                        break;
                }

            }
            break;
        default:
            return;
    }


#if 0
    MINT32 i4WriteCnt = 0;
    char szFileName[256];

    ACDK_LOGD("%s  va=%p, size=%d, offset=%d", __FUNCTION__, va, size,offset);

    sprintf(szFileName, "%s/acdkCap.yuv",MEDIA_PATH);

    FILE *pFp = fopen(szFileName, "wb");

    if(NULL == pFp)
    {
        ACDK_LOGE("Can't open file to save image");
    }

    i4WriteCnt = fwrite(va, 1, size, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        fclose(pFp);
    }
#endif

}

static void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                         const camera_memory_t *data, unsigned index,
                         void *user)
{
}


static camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
                                     void *user __attribute__((unused)))
{
    MY_LOGD("%s  __get_memory +", __FUNCTION__);



    CameraHeapMemory *mem;

    if (fd < 0)
        mem = new CameraHeapMemory(buf_size, num_bufs);
    else
        mem = new CameraHeapMemory(fd, buf_size, num_bufs);
    mem->incStrong(mem);

    MY_LOGD("Buf Size:%d, Num of bufs:%d", buf_size, num_bufs);

    MY_LOGD("%s  __get_memory -", __FUNCTION__);
    return &mem->handle;
}

String8 flatten(DefaultKeyedVector<String8,String8>const& rMap)
{
    String8 flattened("");
    size_t size = rMap.size();

    for (size_t i = 0; i < size; i++) {
        String8 k, v;
        k = rMap.keyAt(i);
        v = rMap.valueAt(i);

        flattened += k;
        flattened += "=";
        flattened += v;
        if (i != size-1)
            flattened += ";";
    }

    return flattened;
}


void unflatten(const String8 &params, DefaultKeyedVector<String8,String8>& rMap)
{
    const char *a = params.string();
    const char *b;

    rMap.clear();

    for (;;) {
        // Find the bounds of the key name.
        b = strchr(a, '=');
        if (b == 0)
            break;

        // Create the key string.
        String8 k(a, (size_t)(b-a));

        // Find the value.
        a = b+1;
        b = strchr(a, ';');
        if (b == 0) {
            // If there's no semicolon, this is the last item.
            String8 v(a);
            rMap.add(k, v);
            break;
        }

        String8 v(a, (size_t)(b-a));
        rMap.add(k, v);
        a = b+1;
    }
}


void cameraDeviceUinit()
{
    if (mDevice->ops->release)
    {
        mDevice->ops->release(mDevice);

    }

    MY_LOGD("Destroying camera %s", "0");
    if(mDevice) {
        int rc = mDevice->common.close(&mDevice->common);
        if (rc != OK)
            MY_LOGD("Could not close camera %d", rc);
    }
    mDevice = NULL;
}


void startPreview()
{
    MY_LOGD("Start preview \n");


    MY_LOGD("loadModule start \n");
    loadModule();
    MY_LOGD("loadModule done \n");

    char const* sym = "MtkCam_setProperty";
    void* pfn = ::dlsym(mModule->common.dso, sym);
    if ( ! pfn ) {
       MY_LOGE("Cannot find symbol: %s", sym);
      //  return INVALID_OPERATION;
    }
    String8 key = String8("client.appmode");
    String8 value = String8("MtkEng");
    reinterpret_cast<status_t(*)(String8 const&, String8 const&)>(pfn)(key, value);

    MY_LOGD("openDevice start \n");
    openDevice((hw_module_t *)mModule, "0");
    MY_LOGD("openDevice done \n");

    // Set callbacks
    MY_LOGD("+ set_callbacks \n");

    if (mDevice->ops->set_callbacks) {
       mDevice->ops->set_callbacks(mDevice,
                              __notify_cb,
                              __data_cb,
                              __data_cb_timestamp,
                              __get_memory,
                              &mDevice);
       }

    MY_LOGD("- set_callbacks");

    if (mDevice->ops->enable_msg_type)
        mDevice->ops->enable_msg_type(mDevice, CAMERA_MSG_PREVIEW_FRAME | CAMERA_MSG_COMPRESSED_IMAGE);

    DefaultKeyedVector<String8,String8> map;

    if (mDevice->ops->get_parameters) {
        char *temp = mDevice->ops->get_parameters(mDevice);
        String8 str_parms(temp);
        if (mDevice->ops->put_parameters)
            mDevice->ops->put_parameters(mDevice, temp);
        else
            free(temp);
        unflatten(str_parms, map);
    }

    mDevice->ops->set_parameters(mDevice, flatten(map).string());

    //g_acdkMhalObserver = mAcdkMhalPrvParam.acdkMainObserver;

    MY_LOGD("+ start_preview \n");
    mDevice->ops->start_preview(mDevice);
    MY_LOGD("- start_preview \n");
}

void stopPreview()
{
    mDevice->ops->stop_preview(mDevice);
}

void startCapture()
{
    MY_LOGD("Start capture \n");

    //MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 CaptureMode = g_u4Mode;
    //eACDK_CAP_FORMAT eImgType;
    char szName[256];

    sprintf(szName, "%04dx%04d ", g_pSensorInfo[g_sensorIdx].captureWidth, g_pSensorInfo[g_sensorIdx].captureHeight);

    // set parameter
    DefaultKeyedVector<String8,String8> map;
    if (mDevice->ops->get_parameters) {
        char *temp = mDevice->ops->get_parameters(mDevice);
        String8 str_parms(temp);
        if (mDevice->ops->put_parameters)
            mDevice->ops->put_parameters(mDevice, temp);
        else
            free(temp);
        unflatten(str_parms, map);
    }
    //Set picture size
    set(map, "picture-size", szName);

    //   rawsave-mode :  1,preview 2,Capture 3,jpeg only 4,video  5,slim video1 6,slim video2
    //   isp-mode   : 0: process raw, 1:pure raw
    set(map, "camera-mode", "2");
    set(map, "afeng_raw_dump_flag", "1");
    set(map, "rawfname", "/data/");

    set(map, "isp-mode", "1");
    set(map, "rawsave-mode", "2");

    switch(CaptureMode)
    {
        case 0:
            set(map, "rawsave-mode", "1");
            break;
        case 1:
            set(map, "rawsave-mode", "2");
            break;
        case 2:
            set(map, "rawsave-mode", "3");
            break;
        case 3:
            set(map, "rawsave-mode", "5");
            break;
        case 4:
            set(map, "rawsave-mode", "6");
            break;
        default:
            set(map, "rawsave-mode", "1");
            break;

    }

     mDevice->ops->set_parameters(mDevice, flatten(map).string());

    //====== Setting Callback ======
    //g_acdkMhalCapObserver = mAcdkMhalCapParam.acdkMainObserver;
    mDevice->ops->take_picture(mDevice);

}

void stopCapture()
{
    while(mCaptureDone == MFALSE)
    {
        usleep(200);
    }
    MY_LOGD("stop capture done");

    mCaptureDone = MFALSE;
}

void createMemBuf(MUINT32 memSize, MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MY_LOGD("bufCnt(%d)", bufCnt);

    MINT32 err ;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);
    IMemDrv *m_pIMemDrv =  IMemDrv::createInstance();

    MY_LOGD("memSize(%u),alingSize(%u)", memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                MY_LOGD("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                MY_LOGE("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            MY_LOGE("m_pIMemDrv->allocVirtBuf() error");
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            MY_LOGE("m_pIMemDrv->mapPhyAddr() error");
        }
    }

    MY_LOGD("-");
}

void destroyMemBuf(MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MY_LOGD("bufCnt(%d)", bufCnt);

    IMemDrv *m_pIMemDrv =  IMemDrv::createInstance();

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                MY_LOGD("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                MY_LOGD("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                MY_LOGE("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            MY_LOGD("Buffer doesn't exist");
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        }
    }

    MY_LOGD("-");
    return ;
}


void rawImgUnpack(IMEM_BUF_INFO srcImem,
                   IMEM_BUF_INFO dstImem,
                   MUINT32 a_imgW,
                   MUINT32 a_imgH,
                   MUINT32 a_bitDepth,
                   MUINT32 a_Stride)
{

    MY_LOGD("srcImem : VA(0x%x),PA(0x%x),ID(%d),SZ(%u)",srcImem.virtAddr,
                                                           srcImem.phyAddr,
                                                           srcImem.memID,
                                                           srcImem.size);

    MY_LOGD("dstImem : VA(0x%x),PA(0x%x),ID(%d),SZ(%u)",dstImem.virtAddr,
                                                           dstImem.phyAddr,
                                                           dstImem.memID,
                                                           dstImem.size);

    MY_LOGD("imgW(%u),imgH(%u),bitDepth(%u),stride(%u)",a_imgW,
                                                           a_imgH,
                                                           a_bitDepth,
                                                           a_Stride);

    //====== Unpack ======

    MUINT8 *pSrcBuf = (MUINT8 *)srcImem.virtAddr;
    MUINT16 *pDstBuf = (MUINT16 *)dstImem.virtAddr;

    if(a_bitDepth == 8)
    {
        MUINT8 pixelValue;
        for(MUINT32 i = 0; i < (a_imgW * a_imgH); ++i)
        {
            pixelValue = *(pSrcBuf++);
            *(pDstBuf) = pixelValue;
        }
    }
    else if(a_bitDepth == 10)
    {
        MUINT8 *lineBuf;

        for(MUINT32 i = 0; i < a_imgH; ++i)
        {
            lineBuf = pSrcBuf + i * a_Stride;

            for(MUINT32 j = 0; j < (a_imgW / 4); ++j)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));

                *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0x3) << 8));
                *(pDstBuf++) = (MUINT16)(((byte1 & 0xFC) >> 2) + ((byte2 & 0xF) << 6));
                *(pDstBuf++) = (MUINT16)(((byte2 & 0xF0) >> 4) + ((byte3 & 0x3F) << 4));
                *(pDstBuf++) = (MUINT16)(((byte3 & 0xC0) >> 6) + (byte4 << 2));
            }

            //process last pixel in the width
            if((a_imgW % 4) != 0)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));

                for(MUINT32 j = 0; j < (a_imgW % 4); ++j)
                {
                    switch(j)
                    {
                        case 0 : *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0x3) << 8));
                            break;
                        case 1 : *(pDstBuf++) = (MUINT16)(((byte1 & 0x3F) >> 2) + ((byte2 & 0xF) << 6));
                            break;
                        case 2 : *(pDstBuf++) = (MUINT16)(((byte2 & 0xF0) >> 4) + ((byte3 & 0x3F) << 6));
                            break;
                        case 3 : *(pDstBuf++) = (MUINT16)(((byte3 & 0xC0) >> 6) + (byte4 << 2));
                            break;
                    }
                }
            }
        }
    }
    else if(a_bitDepth == 12)
    {
        MUINT8 *lineBuf;

        for(MUINT32 i = 0; i < a_imgH; ++i)
        {
            lineBuf = pSrcBuf + i * a_Stride;

            for(MUINT32 j = 0; j < (a_imgW / 4); ++j)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));
                MUINT8 byte5 = (MUINT8)(*(lineBuf++));

                *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0xF) << 8));
                *(pDstBuf++) = (MUINT16)((byte1 >> 4) + (byte2 << 4));
                *(pDstBuf++) = (MUINT16)(byte3 + ((byte4 & 0xF) << 8));
                *(pDstBuf++) = (MUINT16)((byte4 >> 4) + (byte5 << 4));
            }

             //process last pixel in the width
            if((a_imgW % 4) != 0)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));
                MUINT8 byte5 = (MUINT8)(*(lineBuf++));

                for(MUINT32 j = 0; j < (a_imgW % 4); ++j)
                {
                    switch(j)
                    {
                        case 0 : *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0xF) << 8));
                            break;
                        case 1 : *(pDstBuf++) = (MUINT16)((byte1 >> 4) + (byte2 << 4));
                            break;
                        case 2 : *(pDstBuf++) = (MUINT16)(byte3 + ((byte4 & 0xF) << 8));
                            break;
                        case 3 : *(pDstBuf++) = (MUINT16)((byte4 >> 4) + (byte5 << 4));
                            break;
                    }
                }
            }
        }
    }

    MY_LOGD("-");
}

MUINT32 loadCapImgAndCalCrc()
{
    // Load captured image into imgBuffer & unpack it
    char szFileName[256];
    MINT32 i4WriteCnt = 0;
    MUINT32 captureSize;
    MUINT32 imgCrc;

    //Unpack pure raw size = width*height*2
    captureSize = g_pSensorInfo[g_sensorIdx].captureWidth * g_pSensorInfo[g_sensorIdx].captureHeight * 2;
    sprintf(szFileName, "/data/pure__%dx%d_10_%d",
                   g_pSensorInfo[g_sensorIdx].captureWidth, g_pSensorInfo[g_sensorIdx].captureHeight, g_pSensorInfo[g_sensorIdx].sensorFormatOrder);

    MY_LOGD("Save Raw image file name:%s\n", szFileName);
    //printf("Save Raw image file name:%s\n", szFileName);

    IMEM_BUF_INFO mCapIMemInfo;

    MY_LOGD("captureSize = %u", captureSize);
    createMemBuf(captureSize, 1, &mCapIMemInfo); // Raw buffer
    MY_LOGD("mCapIMemInfo : size(capturesize) = %u", mCapIMemInfo.size);

    //====== Read RAW Data ======
    FILE *pFp = fopen(szFileName, "rb");

    if(NULL == pFp )
    {
        MY_LOGE("Can't open file to save image");
        return 0;
    }

    i4WriteCnt = fread((void *)mCapIMemInfo.virtAddr, 1, captureSize, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        MY_LOGE("fync fail");
    }
    fclose(pFp);

// No need to do unpack, cause the currently image is packed
#if 0
    IMEM_BUF_INFO mRawIMemInfo;

    mRawIMemInfo.size = mRawIMemInfo.virtAddr = mRawIMemInfo.phyAddr = 0;
    mRawIMemInfo.memID = -5;

    MUINT32 imgStride = g_pSensorInfo[g_sensorIdx].captureWidth , unPackSize = 0;
    MUINT32 tempFmt, tempRAWPixelByte;
    IMEM_BUF_INFO unPackIMem;

    tempFmt = eImgFmt_BAYER10;

    createMemBuf(captureSize, 1, &mRawIMemInfo);

    unPackSize = g_pSensorInfo[g_sensorIdx].captureWidth * g_pSensorInfo[g_sensorIdx].captureHeight * 2;
    MY_LOGD("unPackSize(%u),imgStride(%u)",unPackSize,imgStride);

    createMemBuf(unPackSize, 1, &unPackIMem);

    rawImgUnpack(mCapIMemInfo,
                unPackIMem,
                g_pSensorInfo[g_sensorIdx].captureWidth,
                g_pSensorInfo[g_sensorIdx].captureHeight,
                10,
                imgStride);

    MY_LOGD("Calculate the CRC value of image buffer");

    unsigned char * pUnPackIMemPA = reinterpret_cast<unsigned char*>(unPackIMem.phyAddr);
    unsigned char * pUnPackIMemVA = reinterpret_cast<unsigned char*>(unPackIMem.virtAddr);
    unsigned char * pCapIMemInfoPA = reinterpret_cast<unsigned char*>(mCapIMemInfo.phyAddr);
    unsigned char * pCapIMemInfoVA = reinterpret_cast<unsigned char*>(mCapIMemInfo.virtAddr);



    MY_LOGD("Address of mCapIMemInfo, PA(%p), VA(%p), Size(%d)", pCapIMemInfoPA, pCapIMemInfoVA, mCapIMemInfo.size);
    char fileName[256] = {'\0'};
    sprintf(fileName, "/data/raw_%ux%u_%02d.raw", g_pSensorInfo[g_sensorIdx].captureWidth, g_pSensorInfo[g_sensorIdx].captureHeight, 1);
    saveBufToFile(fileName, pCapIMemInfoVA, mCapIMemInfo.size);

    MY_LOGD("Address of UnPackIMem, PA(%p), VA(%p), Size(%d)", pUnPackIMemPA, pUnPackIMemVA, unPackIMem.size);

    sprintf(fileName, "/data/raw_unpack_%ux%u_%02d.raw", g_pSensorInfo[g_sensorIdx].captureWidth, g_pSensorInfo[g_sensorIdx].captureHeight, 1);
    saveBufToFile(fileName, pUnPackIMemVA, unPackIMem.size);
#endif

    unsigned char * pCapIMemInfoVA = reinterpret_cast<unsigned char*>(mCapIMemInfo.virtAddr);

    // For CRC Check
    register MUINT32 crc_accum = 0;
    MUINT32 size = mCapIMemInfo.size;
    MUINT8 *out_buffer = pCapIMemInfoVA;

    //MY_LOGD("[Camera_check_crc]\n");
    MY_LOGD("[Camera_check_crc]\n");


    while (size-- > 0)
    {
        crc_accum = (crc_accum << 8) ^ ACDK_CRC_Table[(MUINT8)(crc_accum >> 24) ^ (*out_buffer++)];
    }

#if 0
    if(u4CapCnt ==0)
    {
        u4CRCRef = ~crc_accum;
        printf("CRC_accum 0x%x\n", ~crc_accum);
    }
    else
#endif

    imgCrc = ~crc_accum;




    //====== Uninit and Release ======
    // free memory
#if 0
    if(mRawIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mRawIMemInfo);
        mRawIMemInfo.size = mRawIMemInfo.virtAddr = mRawIMemInfo.phyAddr = 0;
        mRawIMemInfo.memID = -5;
    }

    if(unPackIMem.virtAddr != 0)
    {
        destroyMemBuf(1, &unPackIMem);
        unPackIMem.size = unPackIMem.virtAddr = unPackIMem.phyAddr = 0;
        unPackIMem.memID = -5;
    }
#endif

    if(mCapIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mCapIMemInfo);
        mCapIMemInfo.size = mCapIMemInfo.virtAddr = mCapIMemInfo.phyAddr = 0;
        mCapIMemInfo.memID = -5;
    }

    return imgCrc;

}




/*******************************************************************************
*  Main Function
********************************************************************************/
int main_acdksingleshot(int argc, char** argv)
{

    MUINT32 capCrc[g_u4ShotCnt];

    printf("[CAM : ACDK SingleShot Test]\n");
    /*
    printf("init start\n");
    init();
    printf("init done\n");
    */

// Marked, wait for the unpacked path can work
// Take three images to compare thrir CRC values temply
    // Get test pattern check sum value of main sensor
    MY_LOGD("getAndSetTestPattern start\n");
    getTestPatternCRCValue();
    MY_LOGD("getAndSetTestPattern done\n");

    // [Temp solution] capture multiple images & compare their CRC values by Jessy @2014/04/15
    for(int i = g_u4ShotCnt; i > 0; i--)
    {

        // Start preview
        MY_LOGD("startPreview start\n");
        startPreview();
        MY_LOGD("startPreview done\n");

        usleep(10*10);

        //stopPreview();

        // Let sensor to output test pattern
        init();
        outputTestPattern();

        printf("startCapture start, captured image: %d\n", g_u4ShotCnt - i);
        startCapture();
        MY_LOGD("startCapture done\n");

        // Wait for capture done
        MY_LOGD("stopCapture start\n");
        stopCapture();
        MY_LOGD("stopCapture done\n");

        // Read saved image & calculate its CRC value
        capCrc[g_u4ShotCnt - i] = loadCapImgAndCalCrc();

        u4CapCnt++;
        printf("Current capture count:%2d, CRC value:%d\n", u4CapCnt, capCrc[g_u4ShotCnt - i]);

        /*
        // Start preview, need this, or cannot execute the next capture action

        printf("startPreview start\n");
        startPreview();
        printf("startPreview done\n");

        usleep(10*10);
        stopPreview();
        */
        cameraDeviceUinit();

    }



    u4CRCRef = capCrc[0];
    MBOOL testPass = true;

    for(int i = g_u4ShotCnt - 1; i > 0; i--)
    {
        if(u4CRCRef != capCrc[i])
        {
            testPass = false;
            MY_LOGD("=========CRC Different!=========\nFrame:%d, CRC value:%d, Ref CRC value:%d\n", i, capCrc[i], u4CRCRef);
            break;
        }
    }


    uinit();

    //if(u4CRC == u4CRCRef)
    printf("=========SingleShot Test Result Start=========\n");
    if(true == testPass)
    {
        printf("Cam Test Pass\n");
    }
    else
    {
        printf("Cam Test Fail\n");
    }
    printf("=========SingleShot Test Result End=========\n");


    //pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL);



    g_bIsCLITest = MFALSE;

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************
    while (g_bIsCLITest == MTRUE)
    {
        usleep(100000);
    }


    delete [] g_pSensorInfo;

    printf("[Test END]\n");

    return 0;
}
