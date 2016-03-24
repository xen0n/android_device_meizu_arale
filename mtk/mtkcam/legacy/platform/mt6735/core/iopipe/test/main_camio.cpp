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



#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>


#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;
#include <mtkcam/camnode/test/testNodes.h>
#include <utils/Mutex.h>
using namespace android;
#include <queue>
using namespace std;


#include <mtkcam/common.h>
//#include <mtkcam/camnode/AllocBufHandler.h>
//
//
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>

using namespace NSCam::NSIoPipe::NSCamIOPipe;
#include <core/iopipe/CamIO/PortMap.h>
using namespace NSCam::NSIoPipe;
using namespace NSCam;
//
#include "mtkcam/hwutils/HwMisc.h"
using namespace NSCamHW;
#include <mtkcam/imageio/ispio_utility.h>
#include "mtkcam/imageio/ispio_pipe_ports.h"
#include <mtkcam/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/ImageBufferHeap.h>
#include <mtkcam/drv/imem_drv.h>
#include <semaphore.h>
#include <pthread.h>
#include <mtkcam/v1/config/PriorityDefs.h>
#include <utils/threads.h>
#include <mtkcam/imageio/ispio_utility.h>

// For property_get().
#include <cutils/properties.h>
//for EPIPECmd
enum EPIPECmd{
    EPIPECmd_SET_P1_UPDATE              = 0x101B,
    EPIPECmd_GET_CUR_SOF_IDX            = 0x111a,
    EPIPECmd_AE_SMOOTH                  = 0x1105
};

#undef LOG_TAG
#define LOG_TAG "camiopipetest"


#define bitmap(bit)({\
        UINT32 _fmt = 0;\
        switch(bit){\
            case 8: _fmt = eImgFmt_BAYER8; break;\
            case 10: _fmt = eImgFmt_BAYER10; break;\
            case 12: _fmt = eImgFmt_BAYER12; break;\
            default:                break;\
        }\
        _fmt;})

//number of buf
#define Enque_buf (3)
#define Dummy_buf (3)
#define replace_buf (1)

#define SEN_PIX_BITDEPTH    (10)
#define RRZ_SCALING_RATIO   (2) //divisor
#define __RRZO 0
#define __IMGO 1
#define __CAMSV2 2
#define __CAMSV  3
#define __MAXDMAO (__CAMSV + 1)//(__RRZO + __IMGO + 1)
#define __RRZO_ENABLE (0x1)
#define __IMGO_ENABLE (0x2)
#define __CAMSV2_ENABLE (0x4)
#define __CAMSV_ENABLE (0x8)

#define _TG_1_  0
#define _TG_2_  1
#define _MAXTG_ 2

#define CAM_V1            (0)//0: cam3, else; cam1
#define CAM_V3            (1)
#define AE_SMOOTHING_TEST (0)
#define SUB_TEST_MODEL    (0)//1: sub sensor uses test model, 8M
#define ALWAYS_PURE_RAW_TEST (0)//0: processed raw & could dynamic change
//symetric cropping
#if 0
#define __CROP_Start(in,crop,fmt) ({\
    NSImageio::NSIspio::ISP_QUERY_RST queryRst;\
    NSImageio::NSIspio::ISP_QuerySize(_BY_PASSS_PORT,NSImageio::NSIspio::ISP_QUERY_CROP_START_X,\
            (EImageFormat)fmt,(in.w-crop.w)/2,queryRst);\
    MPoint _point = MPoint(queryRst.crop_x,(in.h-crop.h)/2);\
    _point;})
#endif
MPoint __CROP_Start(MSize in,MSize crop,MINT32 fmt, MUINT32 cropsel)
{
    NSImageio::NSIspio::ISP_QUERY_RST queryRst;
    MUINT32 width;
    MUINT32 height;
    switch (cropsel)
    {
        case 0:
            width = (in.w-crop.w)/2;
            height = (in.h-crop.h)/2;
            break;
        case 1:
            //support arbitray cropping
            width = ((in.w-crop.w)/2-5);
            height = (in.h-crop.h)/2;
            break;
        default:
            width = (in.w-crop.w)/2;
            height = (in.h-crop.h)/2;
            break;
    }
    NSImageio::NSIspio::ISP_QuerySize(_BY_PASSS_PORT,NSImageio::NSIspio::ISP_QUERY_CROP_START_X,
            (EImageFormat)fmt,width,queryRst);
    MPoint _point = MPoint(queryRst.crop_x,height);
    return _point;
}


//croping size have an alignment-rule in imgo/imgo_d (no resizer dma port)
//rrzo do nothing, because of rrzo not using dmao cropping.
//note :
//     the query operation, if cropsize != inputsize , use ISP_QUERY_CROP_X_PIX to query.
#define __CROP_SIZE(portId,fmt,size,pixmode) ({\
    MSize __size;\
    __size = MSize((size.w/RRZ_SCALING_RATIO + 64),(size.h/RRZ_SCALING_RATIO + 64));\
    if(portId == __IMGO){\
         __size = MSize((size.w),(size.h));\
        NSImageio::NSIspio::E_ISP_QUERY _op;\
        _op = (size.w != __size.w)?(NSImageio::NSIspio::ISP_QUERY_CROP_X_PIX):(NSImageio::NSIspio::ISP_QUERY_X_PIX);\
        NSImageio::NSIspio::ISP_QUERY_RST queryRst;\
        NSImageio::NSIspio::ISP_QuerySize(NSImageio::NSIspio::EPortIndex_IMGO,_op,\
            (EImageFormat)fmt,__size.w,queryRst,pixmode);\
            __size.w = queryRst.x_pix;\
    }\
    if(portId == __RRZO){\
    }\
    __size;})

#define __RRZ_FMT(fmt)({\
    MUINT32 __fmt;\
    switch(fmt){\
        case eImgFmt_BAYER8: __fmt = eImgFmt_FG_BAYER8; break;  \
        case eImgFmt_BAYER10: __fmt = eImgFmt_FG_BAYER10; break; \
        case eImgFmt_BAYER12: __fmt = eImgFmt_FG_BAYER12; break; \
        default: __fmt = eImgFmt_FG_BAYER10; break;             \
    }\
    __fmt;})


//imgo do nothing, because of imgo have no resizer
#define __SCALE_SIZE(portId,fmt,size,Ratio,pixmode)({\
    MSize __size;\
    __size = MSize((size.w/Ratio),(size.h/Ratio));\
    if(portId == __RRZO){\
        NSImageio::NSIspio::ISP_QUERY_RST queryRst;\
        NSImageio::NSIspio::ISP_QuerySize(NSImageio::NSIspio::EPortIndex_RRZO,NSImageio::NSIspio::ISP_QUERY_X_PIX,\
            (EImageFormat)__RRZ_FMT(fmt),__size.w,queryRst,pixmode);\
            __size.w = queryRst.x_pix;\
    }\
    __size;})



typedef struct HwPortConfig{
    PortID          mPortID;
    EImageFormat    mFmt;
    MSize           mSize;
    MRect           mCrop;
    MBOOL           mPureRaw;
    MUINT32         mStrideInByte[3];
}HwPortConfig_t;

static int filecnt = 0;
static int ISPversion = CAM_V1;//0:v1 / 1:v3

void onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const sensorid)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
    char dumppath[256];
    sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt, sid)           \
    do{                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
        char filename[256];                                    \
        sprintf(filename, "%s%s_%d_%04dx%4d_%04d_%02d_%d.%s",     \
                dumppath,                                      \
                type,                                         \
                0,                                \
                buffer->getImgSize().w,buffer->getImgSize().h, \
                buffer->getBufStridesInBytes(0),               \
                filecnt++,                                    \
                sid,                                       \
                fileExt                                        \
               );                                              \
        buffer->saveToFile(filename);                          \
    }while(0)

    if(!makePath(dumppath,0660))
    {
        printf("makePath [%s] fail",dumppath);
        return;
    }

    switch( data )
    {
        case 0:
            DUMP_IImageBuffer( "i", buf, "raw", sensorid);//imgo
            break;
        case 1:
            DUMP_IImageBuffer( "r", buf, "raw", sensorid);//rrzo
            break;
        default:
            printf("not handle this yet data(%d)", data);
            break;
    }

}
class _test_camio
{
    _test_camio()
    {
        m_pNormalP_FrmB = NULL;
        m_pNormalP = NULL;
        for(int i=0;i<__MAXDMAO;i++)
            m_prepalce_Buf[i]=NULL;
        m_enablePort=0x0;m_bStop = MFALSE;m_TgSize = MSize(0,0);
        m_CropTestSel=0;m_sensoridx=0;
    };
    ~_test_camio(){};
    public:
        INormalPipe_FrmB*   m_pNormalP_FrmB;
        INormalPipe*        m_pNormalP;
        IImageBuffer*       m_prepalce_Buf[__MAXDMAO];
        MUINT32             m_enablePort;
        MUINT32             m_sensoridx;
        MSize               m_TgSize;
        NSImageio::NSIspio::E_ISP_PIXMODE mPixMode;

        static _test_camio*  create(void);
        void             destroy(void);
        void             startThread(void);
        void             stopThread(void);
        void             ArbitrayCropSel(MUINT32 CropSel);
        //void             saveFile(void);
        //void onDumpBuffer(const char* usr, MUINT32 const data, MUINT32 const buf, MUINT32 const ext);

    private:
        static MVOID*    _infiniteloop(void* arg);
        MBOOL            m_bStop;
        MUINT32          m_CropTestSel;

        pthread_t        m_Thread;
        sem_t            m_semThread;
};

_test_camio* _test_camio::create(void)
{
    return new _test_camio();
}

void _test_camio::destroy(void)
{
    delete this;
}



void _test_camio::startThread(void)
{
    // Init semphore
    ::sem_init(&this->m_semThread, 0, 0);

    // Create main thread for preview and capture
    printf("error:temp borrow ispdequeuethread priority, need to create self priority\n");
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, PRIO_RT_ISPDEQUEUE_THREAD};
    pthread_create(&this->m_Thread, &attr, this->_infiniteloop, this);
}

void _test_camio::stopThread(void)
{
    pthread_join(this->m_Thread, NULL);
    this->m_bStop = MTRUE;
    ::sem_wait(&this->m_semThread);
}

void _test_camio::ArbitrayCropSel(MUINT32 CropSel)
{
    m_CropTestSel = CropSel;
}

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

static MINT32 Camera_check_crc(MUINT8 *out_buffer, MUINT32 crc, MUINT32 size)
{
  register MUINT32 crc_accum = 0;

  printf("[Camera_check_crc]\n");
  while (size-- > 0)
  {
    crc_accum = (crc_accum << 8) ^ ACDK_CRC_Table[(MUINT8)(crc_accum >> 24) ^ (*out_buffer++)];
  }
  printf("CRC <0x%x>, CRC_accum <0x%x>\n",crc,~crc_accum);

  if (~crc_accum == crc) return 1;

  return 0;
}

MVOID* _test_camio::_infiniteloop(void* arg)
{
    _test_camio* _this = (_test_camio*)arg;
    QBufInfo dequeBufInfo;
    QBufInfo _replace;
    dequeBufInfo.mvOut.reserve(2);
    printf("start infiniteloop\n");
    MUINT32 _loop = 0;
    MUINT32 magicNum =7;
    MBOOL bRet = MTRUE;
    //
    char infvalue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.p1ut.infinite", infvalue, "0");
    int UTinfinite=atoi(infvalue);
    //
    MINTPTR _sof_idx;//for v3
    QFrameParam currQFrameParam;//for configFrame
    MUINT32 sensorDev = (_this->m_sensoridx == 0)?
                        eSoftwareScenario_Main_Normal_Stream://main
                        eSoftwareScenario_Sub_Normal_Stream;//sub

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    _replace.mvOut.clear();
    if( _this->m_enablePort & __IMGO_ENABLE) {
        //enque
        BufInfo OutBuf(PORT_IMGO, 0);
        dequeBufInfo.mvOut.push_back(OutBuf);
        MSize _cropsize = __CROP_SIZE(__IMGO,_this->m_prepalce_Buf[__IMGO]->getImgFormat(),_this->m_TgSize,_this->mPixMode);
        //replace
        //in replace testing , use mag num:7
        BufInfo InBuf(PORT_IMGO, _this->m_prepalce_Buf[__IMGO],_cropsize,MRect(__CROP_Start(_this->m_TgSize,_cropsize,_this->m_prepalce_Buf[__IMGO]->getImgFormat(), 0),_cropsize),(replace_buf + Enque_buf + Dummy_buf));
        _replace.mvOut.push_back(InBuf);
        //IMGO only, for configFrame
        if(!(_this->m_enablePort & __RRZO_ENABLE)){
            FrameParam currFrameParam = FrameParam(
                                        PORT_IMGO,
                                        InBuf.FrameBased.mDstSize,
                                        InBuf.FrameBased.mCropRect);
            currQFrameParam.mvOut.push_back(currFrameParam);
        }
    }
    if( _this->m_enablePort & __RRZO_ENABLE) {
        //enque
        BufInfo OutBuf(PORT_RRZO, 0);
        dequeBufInfo.mvOut.push_back(OutBuf);
        MSize _cropsize = __CROP_SIZE(__RRZO,_this->m_prepalce_Buf[__RRZO]->getImgFormat(),_this->m_TgSize,_this->mPixMode);

        //replace
        MSize _rrz = __SCALE_SIZE(__RRZO,_this->m_prepalce_Buf[__RRZO]->getImgFormat(),_this->m_TgSize,RRZ_SCALING_RATIO,_this->mPixMode);
        printf("_rrz size : 0x%x 0x%x (0x%x 0x%x)\n",_rrz.w,_rrz.h,_this->m_TgSize.w,_this->m_TgSize.h);
        //in replace testing , use mag num:7
        BufInfo InBuf(PORT_RRZO, _this->m_prepalce_Buf[__RRZO],_rrz,MRect(__CROP_Start(_this->m_TgSize,_cropsize,_this->m_prepalce_Buf[__RRZO]->getImgFormat(), _this->m_CropTestSel),_cropsize),(replace_buf + Enque_buf + Dummy_buf));
        _replace.mvOut.push_back(InBuf);
        //for configFrame
        FrameParam currFrameParam = FrameParam(
                                        PORT_RRZO,
                                        InBuf.FrameBased.mDstSize,
                                        InBuf.FrameBased.mCropRect);
        currQFrameParam.mvOut.push_back(currFrameParam);
    }

#if !ALWAYS_PURE_RAW_TEST
    currQFrameParam.mRawOutFmt = EPipe_PROCESSED_RAW;
#endif


    while(1){
        if(_this->m_bStop == MTRUE){
            printf("stop enque/deque\n");
            break;
        }
        _loop++;
        printf("[senidx:%d] loop=%d\n",_this->m_sensoridx, _loop);
        if(CAM_V1 == ISPversion)
            bRet = _this->m_pNormalP->deque(dequeBufInfo);
        else
            bRet = _this->m_pNormalP_FrmB->deque(dequeBufInfo);

        if( bRet == MTRUE){
            for(int i=0;i<dequeBufInfo.mvOut.size();i++){
                BufInfo __buf = dequeBufInfo.mvOut.at(i);
                printf("[%d_E_L deque]: dma:0x%x, PA:0x%x, crop:0x%x,0x%x,0x%x,0x%x_0x%x,0x%x,0x%x,0x%x, size:0x%x,0x%x, mag:0x%x, dummy:0x%x, bufidx:0x%x\n",\
                _this->m_sensoridx,\
                dequeBufInfo.mvOut.at(i).mPortID.index,dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0),\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.y,\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.h,\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.y,\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.h,\
                dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.w,dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.h,\
                dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_tuning,dequeBufInfo.mvOut.at(i).mMetaData.m_bDummyFrame,\
                dequeBufInfo.mvOut.at(i).mBufIdx);

                //If you save file in here, you will face that you don't enque any request, so the test will be stop.
                //Because the UT test is only one thread.
#if 0
                if (dequeBufInfo.mvOut.at(i).mPortID.index == NSImageio::NSIspio::EPortIndex_IMGO)
                {
                    onDumpBuffer("camio", 0, (MUINT32)dequeBufInfo.mvOut.at(i).mBuffer, 0);
                }
                else
                {
                    onDumpBuffer("camio", 1, (MUINT32)dequeBufInfo.mvOut.at(i).mBuffer, 0);
                }
#endif

            }

#if 0
            usleep(1000);//sleep 1ms to cross over vsync and simulate 3a calculation
            if(_loop % 100)
                usleep(20000);//make drop frame status happened intenionally every 100 frames
#endif

            if(CAM_V3 == ISPversion)
            {
                //check drop frame or not, if current frame is drop frame.if current frame is drop frame,
                //can't enque this frame into drv in order to prevent sensor Gain/ExpT latch timing async with isp setting.
                //not opened this option is because of this test case having only 1 thread.
#if 0
                NormalPipe_FRM_STATUS _drop_status = _drop_frame_status;
                while(_drop_status == _drop_frame_status){
                    _this->m_pNormalP_FrmB->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINT32)&_drop_status,0,0)
                    usleep(5000);
                }
#endif
            }

            if(CAM_V1 == ISPversion){//no replace buf
                magicNum++;
                for(int i=0;i<dequeBufInfo.mvOut.size();i++){
                    if(dequeBufInfo.mvOut.at(i).mPortID == PORT_IMGO)
                        dequeBufInfo.mvOut.at(i).FrameBased.mCropRect = dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d;
                    else if(dequeBufInfo.mvOut.at(i).mPortID == PORT_RRZO)
                        dequeBufInfo.mvOut.at(i).FrameBased.mCropRect = dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s;
                    dequeBufInfo.mvOut.at(i).FrameBased.mDstSize = dequeBufInfo.mvOut.at(i).mMetaData.mDstSize;
                    dequeBufInfo.mvOut.at(i).FrameBased.mMagicNum_tuning = magicNum;
                    dequeBufInfo.mvOut.at(i).mBufIdx = 0xffff;//must be, caused bufidx is that a output data of deque, this value will be reset by line:294
                    //
                    printf("[%d_E_L enque]: dma:0x%x, PA:0x%x, crop:0x%x,0x%x,0x%x,0x%x, size:0x%x,0x%x, mag:0x%x, bufidx:0x%x\n",\
                    _this->m_sensoridx,\
                    dequeBufInfo.mvOut.at(i).mPortID.index,dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0),\
                    dequeBufInfo.mvOut.at(i).FrameBased.mCropRect.p.x,dequeBufInfo.mvOut.at(i).FrameBased.mCropRect.p.y,\
                    dequeBufInfo.mvOut.at(i).FrameBased.mCropRect.s.w,dequeBufInfo.mvOut.at(i).FrameBased.mCropRect.s.h,\
                    dequeBufInfo.mvOut.at(i).FrameBased.mDstSize.w,dequeBufInfo.mvOut.at(i).FrameBased.mDstSize.h,\
                    dequeBufInfo.mvOut.at(i).FrameBased.mMagicNum_tuning,\
                    dequeBufInfo.mvOut.at(i).mBufIdx);
                }
                /* - - Update Magic# - -
                 * EPIPECmd_SET_P1_UPDATE + configFrame
                ----------------------------------------------------------*/
                if(_this->m_pNormalP->sendCommand(EPIPECmd_SET_P1_UPDATE, _this->m_sensoridx, (MINTPTR)magicNum, sensorDev) == MFALSE)
                    printf("[%d] ERROR: EPIPECmd_SET_P1_UPDATE\n",_this->m_sensoridx);
                currQFrameParam.mMagicNum = magicNum;
                _this->m_pNormalP->configFrame(currQFrameParam);
                //
                _this->m_pNormalP->enque(dequeBufInfo);
            }
            else
            {
                _this->m_pNormalP_FrmB->sendCommand(EPIPECmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx,0,0);
                //use replace buffer to enque,simulate new request in camera3
                for(int i=0;i<_replace.mvOut.size();i++){
                    //HDR viewFinder
                    _replace.mvOut.at(i).FrameBased.mSOFidx = _sof_idx;
                    printf("[%d_E_L enque]: dma:0x%x, PA:0x%x, crop:0x%x,0x%x,0x%x,0x%x, size:0x%x,0x%x, mag:0x%x, bufidx:0x%x, sofidx:0x%x\n",\
                        _this->m_sensoridx,\
                        _replace.mvOut.at(i).mPortID.index,_replace.mvOut.at(i).mBuffer->getBufPA(0),\
                        _replace.mvOut.at(i).FrameBased.mCropRect.p.x,_replace.mvOut.at(i).FrameBased.mCropRect.p.y,\
                        _replace.mvOut.at(i).FrameBased.mCropRect.s.w,_replace.mvOut.at(i).FrameBased.mCropRect.s.h,\
                        _replace.mvOut.at(i).FrameBased.mDstSize.w,_replace.mvOut.at(i).FrameBased.mDstSize.h,\
                        _replace.mvOut.at(i).FrameBased.mMagicNum_tuning,\
                        _replace.mvOut.at(i).mBufIdx, _replace.mvOut.at(i).FrameBased.mSOFidx);
                }
                //
                _this->m_pNormalP_FrmB->enque(_replace);

#if AE_SMOOTHING_TEST
                if((_loop%3)==2){
                    MINT32 OBgain = 1024;
                    printf("[%d] EPIPECmd_AE_SMOOTH (0x%x)\n",_this->m_sensoridx, OBgain);
                    _this->m_pNormalP_FrmB->sendCommand(EPIPECmd_AE_SMOOTH, (MINT32) &OBgain, 0, 0);
                }
                else{
                    MINT32 OBgain = 512;
                    printf("[%d] EPIPECmd_AE_SMOOTH (0x%x)\n",_this->m_sensoridx, OBgain);
                    _this->m_pNormalP_FrmB->sendCommand(EPIPECmd_AE_SMOOTH, (MINT32) &OBgain, 0, 0);
                }
#endif

                //update replace buffer to previous deque result
                _replace = dequeBufInfo;
                //note: must asign result to input
                magicNum++;
                for(int i=0;i<dequeBufInfo.mvOut.size();i++){
                    if(dequeBufInfo.mvOut.at(i).mPortID == PORT_IMGO)
                        _replace.mvOut.at(i).FrameBased.mCropRect = dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d;
                    else if(dequeBufInfo.mvOut.at(i).mPortID == PORT_RRZO)
                        _replace.mvOut.at(i).FrameBased.mCropRect = dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s;

                    _replace.mvOut.at(i).FrameBased.mDstSize = dequeBufInfo.mvOut.at(i).mMetaData.mDstSize;
                    //_replace.mvOut.at(i).FrameBased.mMagicNum_tuning = dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_tuning;
                    _replace.mvOut.at(i).FrameBased.mMagicNum_tuning = magicNum;
                    _replace.mvOut.at(i).mBufIdx = 0xffff;//must be, caused bufidx is that a output data of deque, this value will be reset by line:294
                }
            }
        }
        else{
            MBOOL _break = MFALSE;

            if(CAM_V1 == ISPversion){
                _break= MTRUE;
            }
            else{
                //if deque fail is because of current dequed frame is dummy frame, bypass this error return.
                for(int i=0;i<dequeBufInfo.mvOut.size();i++){
                    if(dequeBufInfo.mvOut.at(i).mMetaData.m_bDummyFrame == MFALSE)
                        _break = MTRUE;
                    else
                       printf("[senidx:%d] deque dummy\n",_this->m_sensoridx);
                }
            }
            if(_break == MTRUE){
                printf("[senidx:%d] deque fail, leave infinite_loop\n",_this->m_sensoridx);
                break;
            }
        }

        if (!UTinfinite && _loop == 30)
        {
            printf("[senidx:%d] test count is 600, break the while loop!!\n", _this->m_sensoridx);
            break;
        }
    }

    if(!UTinfinite)
        ::sem_post(&_this->m_semThread);

    return NULL;
}


MBOOL getSensorPixelMode(MUINT32* pPixelMode,MUINT32 sensorIdx,MUINT32 scenario,MUINT32 sensorFps)
{
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    if( !pHalSensorList ) {
        printf("error:pHalSensorList == NULL\n");
        return MFALSE;
    }

    pSensorHalObj = pHalSensorList->createSensor(
            LOG_TAG,
            sensorIdx);
    //
    if( pSensorHalObj == NULL )
    {
        printf("error:pSensorHalObj is NULL\n");
        return MFALSE;
    }

    pSensorHalObj->sendCommand(
            pHalSensorList->querySensorDevIdx(sensorIdx),
            SENSOR_CMD_GET_SENSOR_PIXELMODE,
            (MUINTPTR)&scenario,
            (MUINTPTR)&sensorFps,
            (MUINTPTR)pPixelMode);
    printf("sensorScenario(%d),sensorFps(%d),pixelMode(%d)\n",
            scenario,
            sensorFps,
            *pPixelMode);

    pSensorHalObj->destroyInstance(LOG_TAG);

    if( *pPixelMode != 0 && *pPixelMode != 1 )
    {
        printf("error: Un-supported pixel mode %d\n", *pPixelMode);
        return MFALSE;
    }

    return MTRUE;
}


int getSensorSize(MSize* pSize,SensorStaticInfo mSensorInfo,MUINT32 SenScenario)
{
    MBOOL ret = MTRUE;
    // sensor size
#define scenario_case(scenario, KEY, pSize)       \
        case scenario:                            \
            (pSize)->w = mSensorInfo.KEY##Width;  \
            (pSize)->h = mSensorInfo.KEY##Height; \
            break;
    switch(SenScenario)
    {
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, pSize );
        default:
            printf("not support sensor scenario(0x%x)\n", SenScenario);
            ret = MFALSE;
            break;
    }
#undef scenario_case

lbExit:
    return ret;
}

MBOOL getOutputFmt(PortID port,MUINT32 bitDepth,SensorStaticInfo mSensorInfo,EImageFormat* pFmt)
{
    MBOOL ret = MFALSE;
    // sensor fmt
#define case_Format( order_bit, mappedfmt, pFmt) \
        case order_bit:                          \
            (*(pFmt)) = mappedfmt;               \
        break;

    if( mSensorInfo.sensorType == SENSOR_TYPE_YUV )
    {
        switch( mSensorInfo.sensorFormatOrder )
        {
            case_Format( SENSOR_FORMAT_ORDER_UYVY, eImgFmt_UYVY, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_VYUY, eImgFmt_VYUY, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_YUYV, eImgFmt_YUY2, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_YVYU, eImgFmt_YVYU, pFmt);
            default:
            printf("formatOrder not supported, 0x%x\n", mSensorInfo.sensorFormatOrder);
            goto lbExit;
            break;
        }
        //printf("sensortype:(0x%x), fmt(0x%x)\n", mSensorInfo.sensorType, *pFmt);
    }
    else if( mSensorInfo.sensorType == SENSOR_TYPE_RAW )
    {
        if(port == PORT_IMGO ||  //imgo
           port == PORT_CAMSV_IMGO || //camsv_imgo
           port == PORT_CAMSV2_IMGO )//camsv2_imgo
        {
            switch( bitDepth )
            {
                case_Format(  8, eImgFmt_BAYER8 , pFmt);
                case_Format( 10, eImgFmt_BAYER10, pFmt);
                case_Format( 12, eImgFmt_BAYER12, pFmt);
                case_Format( 14, eImgFmt_BAYER14, pFmt);
                default:
                printf("bitdepth not supported, 0x%x\n", bitDepth);
                goto lbExit;
                break;
            }
        }
        else // rrzo
        {
            switch( bitDepth)
            {
                case_Format(  8, eImgFmt_FG_BAYER8 , pFmt);
                case_Format( 10, eImgFmt_FG_BAYER10, pFmt);
                case_Format( 12, eImgFmt_FG_BAYER12, pFmt);
                case_Format( 14, eImgFmt_FG_BAYER14, pFmt);
                default:
                printf("bitdepth not supported, 0x%x\n", bitDepth);
                goto lbExit;
                break;
            }
        }
        //printf("sensortype: 0x%x, port(0x%x), fmt(0x%x), order(%d)\n",
         //       mSensorInfo.sensorType, port.index, *pFmt, mSensorInfo.sensorFormatOrder);
    }
    else
    {
        printf("sensorType not supported yet(0x%x)\n", mSensorInfo.sensorType);
        goto lbExit;
    }
    ret = MTRUE;
#undef case_Format

lbExit:
    return ret;
}


/*******************************************************************************
*  Main Function
********************************************************************************/
int test_camio(int argc, char** argv)
{
    int ret = 0;
    printf("supported bit:0x%x, rrz scaling ratio:0x%x\n",SEN_PIX_BITDEPTH,RRZ_SCALING_RATIO);

    if( argc < 3 )
    {
        printf("Usage: test_camio 1 <sensorId> <scenario>\n");
        return -1;
    }

    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    pHalSensorList->searchSensors();

    MUINT32 sensorIdx = atoi(argv[1]);  // 0 for main
    MUINT32 scenario  = atoi(argv[2]);  // prv:0, cap:1
    MUINT32 sensorFps = atoi(argv[3]);  // 30 for 30fps
    MUINT32 enablePort = atoi(argv[4]); // rrzo:0x1, imgo:0x2, rrzo+imgo:0x3
    MUINT32 dualSensor = atoi(argv[5]); // 1 for single , 2 for dual sensor, if dualSensor == 1 , always use main sensor
    MUINT32 croptestsel = atoi(argv[6]);// 0 is symmetric crop test, 1~n is arbitrary cropping
    MUINT32 CheckCRC = atoi(argv[7]); // if CheckCRC == 0, without checkCRC; CheckCRC == 1 for Main; CheckCRC == 2 for Sub(dualSensor must be  2)
    MUINT32 ISPver = atoi(argv[8]); //0 for V1; 1 for V3
    printf("iopipetest 1 %d %d %d %d %d %d %d %d\n",sensorIdx,scenario,sensorFps,enablePort,dualSensor,croptestsel,CheckCRC,ISPver);
    printf("sensorIdx:0x%x,senario:0x%x,sensorFps:0x%x,enablePort = 0x%x,dualsensor = 0x%x,croptestsel = 0x%x,CheckCRC= 0x%x,ISPver = 0x%x\n",sensorIdx,scenario,sensorFps,enablePort,dualSensor,croptestsel,CheckCRC,ISPver);
    uint32_t    sensorArray[2] = {0, 0};
    SensorStaticInfo mSensorInfo[2];
      MUINT32 tpp = 0; //for test pattern, tpp==2
      MUINT   tsM = 0;

    if(CheckCRC==2)//sub CRC
        sensorIdx = 1;

    if(CheckCRC > 0)
        tpp = 2;//test pattern
    //
    if(dualSensor > 2)
        dualSensor = 2;

    if(dualSensor == 2)
        sensorIdx = 0;

    //sub only
    if(sensorIdx == 1)
        dualSensor = 2;

    ISPversion = ISPver;

    /*-----------------------------------------------------------
        use tg3 or tg4 (choose one) //no use
        tg3: CAMSV_IMGO
        tg4: CAMSV2_IMGO
    -----------------------------------------------------------*/
    char tg3value[PROPERTY_VALUE_MAX] = {'\0'};
    char tg4value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.tg3.enable", tg3value, "0");
    property_get("camera.tg4.enable", tg4value, "0");
    int tg3Enable=atoi(tg3value);
    int tg4Enable=atoi(tg4value);
    printf("[property] tg3(%d),tg4(%d)\n", tg3Enable, tg4Enable);
    if(tg3Enable || tg4Enable){
        enablePort = 0;
        if(tg3Enable)
            enablePort |= __CAMSV_ENABLE;

        if(tg4Enable)
            enablePort |= __CAMSV2_ENABLE;
        dualSensor = (enablePort == (__CAMSV_ENABLE | __CAMSV2_ENABLE))? 2: 1;
        printf("sensorIdx':0x%x,senario:0x%x,sensorFps:0x%x,enablePort = 0x%x,dualsensor = 0x%x,croptestsel = 0x%x\n",sensorIdx,scenario,sensorFps,enablePort,dualSensor,croptestsel);
    }
    /*-----------------------------------------------------------
        two pix mode
    -----------------------------------------------------------*/
    MUINT32 pixelMode[2] = {0};
    NSImageio::NSIspio::E_ISP_PIXMODE e_PixMode[2];
    char pix0value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.pixmode0.enable", pix0value, "0");
    pixelMode[0] = atoi(pix0value);
    pixelMode[1] = atoi(pix0value);
    printf("[property] 2pixel(%d)\n", pixelMode[0]);
    /*-----------------------------------------------------------
        jpeg format
    -----------------------------------------------------------*/
    char jpgvalue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.jpeg.enable", jpgvalue, "0");
    int jpgEnable=atoi(jpgvalue);
    printf("[property] jpg(%d)\n", jpgEnable);
    //////////////////////////////////////////////////////
    //powerOn sensor
    IHalSensor* pSensorHalObj[_MAXTG_] = {NULL};
    for(int i=sensorIdx;i<dualSensor;i++){
        pSensorHalObj[i] = pHalSensorList->createSensor(LOG_TAG, i);
        if(pSensorHalObj == NULL)
        {
           printf("mpSensorHalObj is NULL");
        }
        //
        sensorArray[i] = i;
        pSensorHalObj[i]->powerOn(LOG_TAG, 1, &sensorArray[i]);
        e_PixMode[i] =  ((pixelMode[i] == 0) ?  (NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE) : (NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE));
    }
    //
    /*-----------------------------------------------------------
        set 24fps(or <=24fps) /DVT
    -----------------------------------------------------------*/
    char fps24value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.fps24.enable", fps24value, "0");
    int fps24Enable=atoi(fps24value);
    printf("[property] fps24(%d)\n", fps24Enable);
    if(sensorFps <= 24){
        fps24Enable = 1;
        printf("[property] fps24(%d), %dfps\n", fps24Enable, sensorFps);
    }

    /////////////////////////////////////////////////////////////////////////
    //querysensor
    for(int i=sensorIdx;i<dualSensor;i++){
        IHalSensorList* const pHalSensorList = IHalSensorList::get();

        pHalSensorList->querySensorStaticInfo(
                pHalSensorList->querySensorDevIdx(i),
                &mSensorInfo[i]);

        //jpeg must enable two pix mode
        if(mSensorInfo[i].sensorType == SENSOR_TYPE_YUV && jpgEnable){
            pixelMode[i] = 1;
            printf("[property] 2pixel(%d/%d)\n", i, pixelMode[i]);
        }

        printf("sensor info[%d]:type(%d),format(%d),bit(%d),cap_sz(%dx%d),pre_sz(%dx%d),pre_fps(%d)\n", \
                i, mSensorInfo[i].sensorType, mSensorInfo[i].sensorFormatOrder, mSensorInfo[i].rawSensorBit,\
                mSensorInfo[i].captureWidth, mSensorInfo[i].captureHeight,\
                mSensorInfo[i].previewWidth,mSensorInfo[i].previewHeight, \
                mSensorInfo[i].previewFrameRate);
    }
    //////////////////////////////////////////////////////////////////////
    //create/init normalpipe_frmb
    INormalPipe* mpCamIO[2] = {NULL,NULL};
    INormalPipe_FrmB* mpCamIO_FrmB[2] = {NULL,NULL};
    if(CAM_V1 == ISPversion){
        for(int i=sensorIdx;i<dualSensor;i++){
            printf("create NormalPipe[%d]\n", i);
            mpCamIO[i] = INormalPipe::createInstance((MINT32)i, LOG_TAG,1);
            mpCamIO[i]->init();
        }
    }
    else{
        for(int i=sensorIdx;i<dualSensor;i++){
            printf("create NormalPipe_FrmB[%d]\n", i);
            mpCamIO_FrmB[i] = INormalPipe_FrmB::createInstance((MINT32)i, LOG_TAG,1);
            mpCamIO_FrmB[i]->init();
        }
    }

    /////////////////////////////////////////////////////////////////////////
    //prepare sensor cfg
    MSize sensorSize[2];
    vector<IHalSensor::ConfigParam> vSensorCfg[2];
    for(int i=sensorIdx;i<dualSensor;i++){
        //e_PixMode[i] =  ((pixelMode[i] == 0) ?  (NSImageio::NSIspio::ISP_QUERY_UNKNOWN_PIX_MODE) : (NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE));
        getSensorSize(&sensorSize[i],mSensorInfo[i],scenario);
#if SUB_TEST_MODEL
        if(i==0){
            tsM = 0;
        }
        else{//sub, 3264x2448
            tsM = 1;
            sensorSize[i].w = 3264;
            sensorSize[i].h = 2448;
            printf("sub sensor is on test model\n");
        }
#endif
        IHalSensor::ConfigParam sensorCfg =
        {
            (MUINT)i,                       /* index            */
            sensorSize[i],                  /* crop             */
            scenario,                       /* scenarioId       */
            0,                              /* isBypassScenario */
            1,                              /* isContinuous     */
            MFALSE,                         /* iHDROn           */
            sensorFps,                      /* framerate        */
            pixelMode[i],                   /* two pixel on     */
            0,                              /* debugmode        */
        };
        vSensorCfg[i].push_back(sensorCfg);
        printf("senidx:0x%x, sensor:%dx%d, sce:%d, bypass:%d, con:%d, hdr:%d, fps:%d, twopxl:%d\n",
                    i,
                    sensorCfg.crop.w,
                    sensorCfg.crop.h,
                    sensorCfg.scenarioId,
                    sensorCfg.isBypassScenario,
                    sensorCfg.isContinuous,
                    sensorCfg.HDRMode,
                    sensorCfg.framerate,
                    sensorCfg.twopixelOn);
    }
    ////////////////////////////////////////////////////////////////////////
    //isp dmao cfg
    printf("start isp dmao cfg\n");
    EImageFormat fmt;
    NSImageio::NSIspio::ISP_QUERY_RST queryRst;
    NSImageio::NSIspio::ISP_QUERY_RST test_queryRst;
    MSize _cropsize;
    list<HwPortConfig_t> lHwPortCfg[2];
    for(int i=sensorIdx;i<dualSensor;i++){
        if(enablePort & __IMGO_ENABLE){
            printf("senidx:0x%x:port IMGO\n",i);
            if( !getOutputFmt(PORT_IMGO,SEN_PIX_BITDEPTH,mSensorInfo[i], &fmt))
                printf("get pix fmt error\n");
            /*if(jpgEnable)
                fmt = eImgFmt_JPEG;*/
            NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    sensorSize[i].w,
                                    queryRst,
                                    e_PixMode[i]
                                    );

            //_cropsize = __CROP_SIZE(__IMGO,fmt,sensorSize[i],e_PixMode[i]);
            _cropsize = MSize(sensorSize[i].w,sensorSize[i].h);
            NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    _cropsize.w,
                                    test_queryRst,
                                    e_PixMode[i]
                                    );

            printf("test_queryRst.stride_byte:(%d), pixmode(%d),",test_queryRst.stride_byte, e_PixMode[i]);
            printf("sensor size width:(%d) x height:(%d),", sensorSize[i].w, sensorSize[i].h);
            printf("crop size width:(%d) x height:(%d)\n", _cropsize.w, _cropsize.h);
            HwPortConfig_t full = {
                PORT_IMGO,
                fmt,
                _cropsize,
                MRect(__CROP_Start(sensorSize[i],_cropsize,fmt, 0),_cropsize),
                0, //if raw type != 1 -> pure-raw
                { queryRst.stride_byte, 0, 0 }
                //{ test_queryRst.stride_byte, 0, 0 }
            };
#if ALWAYS_PURE_RAW_TEST
            printf("imgo pure raw\n");
            full.mPureRaw = 1;
#endif
            //sprintf("crop size widht:(%d) x height:(%d)", _cropsize.w, _cropsize.h);
            lHwPortCfg[i].push_back(full);
        }
        if(enablePort & __RRZO_ENABLE){
            printf("senidx:0x%x:port RRZO\n",i);
            MSize _size;
            if( !getOutputFmt(PORT_RRZO,SEN_PIX_BITDEPTH,mSensorInfo[i], &fmt))
                printf("get pix fmt error\n");
            //scaled size
            if(scenario == 1){//fit test case, main sensor
                //RRZO max size
                _size = MSize(2304,1296);
                if(sensorSize[i].w < 2304){
                    _size.w = sensorSize[i].w;
                    _size.h = sensorSize[i].h;
                }
            }
            else//subsensor, 5M: 1280x960
            {
                _size = __SCALE_SIZE(__RRZO,fmt,sensorSize[i],RRZ_SCALING_RATIO,e_PixMode[i]);
            }
            NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_RRZO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    _size.w,
                                    queryRst,
                                    e_PixMode[i]
                                    );
            //crop size can't < outsize
            if(scenario == 1){//fit test case, crop size = _size
                HwPortConfig_t resized = {
                    PORT_RRZO,
                    fmt,
                    _size,
                    MRect(MPoint(0,0),_size),//fit test case
                    0, //if raw type != 1 -> pure-raw
                    { queryRst.stride_byte, 0, 0 }
                };
                lHwPortCfg[i].push_back(resized);
                printf("queryRst.stride_byte:(%d), pixmode(%d),",queryRst.stride_byte, e_PixMode[i]);
                printf("size width:(%d) x height:(%d),", _size.w, _size.h);
                printf("crop size width:(%d) x height:(%d)\n", _size.w, _size.h);
            }
            else{
                _cropsize = __CROP_SIZE(__RRZO,fmt,sensorSize[i],e_PixMode[i]);
                HwPortConfig_t resized = {
                    PORT_RRZO,
                    fmt,
                    _size,
                    MRect(__CROP_Start(sensorSize[i],_cropsize,fmt, croptestsel),_cropsize),
                    0, //if raw type != 1 -> pure-raw
                    { queryRst.stride_byte, 0, 0 }
                };
                lHwPortCfg[i].push_back(resized);
                printf("queryRst.stride_byte:(%d), pixmode(%d),",queryRst.stride_byte, e_PixMode[i]);
                printf("size width:(%d) x height:(%d),", _size.w, _size.h);
                printf("crop size width:(%d) x height:(%d)\n", _cropsize.w, _cropsize.h);
            }
        }
        if(enablePort & __CAMSV_ENABLE){
            printf("senidx:0x%x:port CAMSV_IMGO\n",i);
            if( !getOutputFmt(PORT_CAMSV_IMGO,SEN_PIX_BITDEPTH,mSensorInfo[i], &fmt))
                printf("get pix fmt error\n");

            MSize _size;
            NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_CAMSV_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    sensorSize[i].w,
                                    queryRst,
                                    e_PixMode[i]
                                    );

            if(_size.w != queryRst.x_pix)
                _size.w = queryRst.x_pix;

            //cropsize, no crop
            _cropsize = MSize(sensorSize[i].w,sensorSize[i].h);
            NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_CAMSV_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    _cropsize.w,
                                    test_queryRst,
                                    e_PixMode[i]
                                    );

            if(_cropsize.w != queryRst.x_pix)
                _cropsize.w = queryRst.x_pix;

            printf("test_queryRst.stride_byte:(%d)",test_queryRst.stride_byte);
            printf("sensor size width:(%d) x height:(%d)", sensorSize[i].w, sensorSize[i].h);
            printf("crop size width:(%d) x height:(%d)", _cropsize.w, _cropsize.h);
            HwPortConfig_t full = {
                PORT_CAMSV_IMGO,
                fmt,
                _cropsize,
                MRect(__CROP_Start(sensorSize[i],_cropsize,fmt, 0),_cropsize),
                //MRect(aaa,_cropsize),
                0, //if raw type != 1 -> pure-raw
                { queryRst.stride_byte, 0, 0 }
                //{ test_queryRst.stride_byte, 0, 0 }
            };
            //sprintf("crop size widht:(%d) x height:(%d)", _cropsize.w, _cropsize.h);
            lHwPortCfg[i].push_back(full);
        }
        if(enablePort & __CAMSV2_ENABLE){
            printf("senidx:0x%x:port CAMSV2_IMGO\n",i);
            if( !getOutputFmt(PORT_CAMSV2_IMGO,SEN_PIX_BITDEPTH,mSensorInfo[i], &fmt))
                printf("get pix fmt error\n");
            NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_CAMSV2_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    sensorSize[i].w,
                                    queryRst,
                                    e_PixMode[i]
                                    );
            //crop size, no crop
            _cropsize = MSize(sensorSize[i].w,sensorSize[i].h);
            NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_CAMSV2_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    _cropsize.w,
                                    test_queryRst,
                                    e_PixMode[i]
                                    );

            if(_cropsize.w != queryRst.x_pix)
                _cropsize.w = queryRst.x_pix;

            printf("test_queryRst.stride_byte:(%d)",test_queryRst.stride_byte);
            printf("sensor size width:(%d) x height:(%d)", sensorSize[i].w, sensorSize[i].h);
            printf("crop size width:(%d) x height:(%d)", _cropsize.w, _cropsize.h);
            HwPortConfig_t full = {
                PORT_CAMSV2_IMGO,
                fmt,
                _cropsize,
                MRect(__CROP_Start(sensorSize[i],_cropsize,fmt, 0),_cropsize),
                //MRect(aaa,_cropsize),
                0, //if raw type != 1 -> pure-raw
                { queryRst.stride_byte, 0, 0 }
                //{ test_queryRst.stride_byte, 0, 0 }
            };
            //sprintf("crop size widht:(%d) x height:(%d)", _cropsize.w, _cropsize.h);
            lHwPortCfg[i].push_back(full);

        }
        printf("senidx:0x%x:hwport size:0x%x\n",i,lHwPortCfg[i].size());
    }
    ////////////////////////////////////////////////////////////////////////////
    //configpipe
    printf("start configpipe\n");
    vector<portInfo> vPortInfo[2];
    list<HwPortConfig_t>::const_iterator pPortCfg;
    for(int i=sensorIdx;i<dualSensor;i++){
        for( pPortCfg = lHwPortCfg[i].begin(); pPortCfg != lHwPortCfg[i].end(); pPortCfg++ )
        {
            printf("senidx:0x%x id:0x%x, crop:%d,%d,%dx%d(0x%x,0x%x,0x%x*0x%x), size:%dx%d(0x%x*0x%x), fmt:0x%x, stride:%d, pureraw:%d\n",\
            i,\
            pPortCfg->mPortID.index,\
            pPortCfg->mCrop.p.x,pPortCfg->mCrop.p.y,pPortCfg->mCrop.s.w,pPortCfg->mCrop.s.h,\
            pPortCfg->mCrop.p.x,pPortCfg->mCrop.p.y,pPortCfg->mCrop.s.w,pPortCfg->mCrop.s.h,\
            pPortCfg->mSize.w,pPortCfg->mSize.h,\
            pPortCfg->mSize.w,pPortCfg->mSize.h,\
            pPortCfg->mFmt,\
            pPortCfg->mStrideInByte[0],\
            pPortCfg->mPureRaw);
            //
            portInfo OutPort(
                    pPortCfg->mPortID,
                    pPortCfg->mFmt,
                    pPortCfg->mSize, //dst size
                    pPortCfg->mCrop, //crop
                    pPortCfg->mStrideInByte[0],
                    pPortCfg->mStrideInByte[1],
                    pPortCfg->mStrideInByte[2],
                    pPortCfg->mPureRaw, // pureraw
                    MTRUE               //packed
                    );
            vPortInfo[i].push_back(OutPort);
        }
        //
        QInitParam halCamIOinitParam(
                tpp, // 2: sensor uses pattern
                SEN_PIX_BITDEPTH,
                vSensorCfg[i],
                vPortInfo[i]);
        //
#if ALWAYS_PURE_RAW_TEST
        halCamIOinitParam.m_DynamicRawType = MFALSE;
#endif
        //NG_TRACE_BEGIN("configP1");
        if(CAM_V1 == ISPversion){
            if( !mpCamIO[i]->configPipe(halCamIOinitParam) ) {
                printf("senidx:0x%x configPipe failed\n",i);
            }
        }
        else{
            if( !mpCamIO_FrmB[i]->configPipe(halCamIOinitParam) ) {
                printf("senidx:0x%x configPipe failed\n",i);
            }
        }
    }
    /////////////////////////////////////////////////////////////////////////
    //allocate buffer
    // 4 enque buf (3 push to drv before start, 1 for replace)
    // 3 dummy buf
    printf("start allocate buffer\n");

    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    IMEM_BUF_INFO imgiBuf;
    IImageBuffer* pImgBuffer[_MAXTG_][__MAXDMAO][(Enque_buf+Dummy_buf+replace_buf)];

    for(int k=sensorIdx;k<dualSensor;k++){
        for(int i=0;i<(Enque_buf+Dummy_buf+replace_buf);i++){
            if(enablePort & __RRZO_ENABLE){
                printf("RRZO: ");
                MSize _size;
                if(scenario == 1){//fit test case, main sensor
                    _size = MSize(2304,1296);
                    if(sensorSize[k].w < 2304){
                        _size.w = sensorSize[i].w;
                        _size.h = sensorSize[i].h;
                    }
                }
                else{
                    _size = MSize((sensorSize[k].w/RRZ_SCALING_RATIO),(sensorSize[k].h/RRZ_SCALING_RATIO));
                }
                if( !getOutputFmt(PORT_RRZO,SEN_PIX_BITDEPTH,mSensorInfo[k], &fmt))
                    printf("get pix fmt error\n");
                NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_RRZO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    _size.w,
                                    queryRst,
                                    e_PixMode[k]
                                    );


                imgiBuf.size = _size.h* queryRst.stride_byte;
                mpImemDrv->allocVirtBuf(&imgiBuf);
                ////imem buffer 2 image heap
                MUINT32 bufStridesInBytes[3] = {queryRst.stride_byte, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((bitmap(SEN_PIX_BITDEPTH)),
                                                        _size, bufStridesInBytes, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,MTRUE);
                pImgBuffer[k][__RRZO][i] = pHeap->createImageBuffer();
                pImgBuffer[k][__RRZO][i]->incStrong(pImgBuffer);
                pImgBuffer[k][__RRZO][i]->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                printf("senidx:0x%x img buffer(%d): rrzo pa:0x%x\n",k,i,pImgBuffer[k][__RRZO][i]->getBufPA(0));
            }
            if(enablePort & __IMGO_ENABLE){
                printf("IMGO: ");
                if( !getOutputFmt(PORT_IMGO,SEN_PIX_BITDEPTH,mSensorInfo[k], &fmt))
                    printf("get pix fmt error\n");
                NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    sensorSize[k].w,
                                    queryRst,
                                    e_PixMode[k]
                                    );

                imgiBuf.size = sensorSize[k].h* queryRst.stride_byte;
                mpImemDrv->allocVirtBuf(&imgiBuf);
                ////imem buffer 2 image heap
                MUINT32 bufStridesInBytes[3] = {queryRst.stride_byte, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((bitmap(SEN_PIX_BITDEPTH)),
                                                        sensorSize[k], bufStridesInBytes, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,MTRUE);
                pImgBuffer[k][__IMGO][i] = pHeap->createImageBuffer();
                pImgBuffer[k][__IMGO][i]->incStrong(pImgBuffer);
                pImgBuffer[k][__IMGO][i]->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                printf("senidx:0x%x img buffer(%d): imgo pa:0x%x\n",k,i,pImgBuffer[k][__IMGO][i]->getBufPA(0));
            }
            if(enablePort & __CAMSV_ENABLE){
                printf("CAMSV: ");
                if( !getOutputFmt(PORT_CAMSV_IMGO,SEN_PIX_BITDEPTH,mSensorInfo[k], &fmt))
                    printf("get pix fmt error\n");
                NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_CAMSV_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    sensorSize[k].w,
                                    queryRst,
                                    e_PixMode[k]
                                    );


                imgiBuf.size = sensorSize[k].h* queryRst.stride_byte;
                mpImemDrv->allocVirtBuf(&imgiBuf);
                ////imem buffer 2 image heap
                MUINT32 bufStridesInBytes[3] = {queryRst.stride_byte, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((bitmap(SEN_PIX_BITDEPTH)),
                                                        sensorSize[k], bufStridesInBytes, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,MTRUE);
                pImgBuffer[k][__CAMSV][i] = pHeap->createImageBuffer();
                pImgBuffer[k][__CAMSV][i]->incStrong(pImgBuffer);
                pImgBuffer[k][__CAMSV][i]->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                printf("senidx:0x%x img buffer(%d): imgo pa:0x%x\n",k,i,pImgBuffer[k][__CAMSV][i]->getBufPA(0));
            }
            if(enablePort & __CAMSV2_ENABLE){
                printf("CAMSV2: ");
                if( !getOutputFmt(PORT_CAMSV2_IMGO,SEN_PIX_BITDEPTH,mSensorInfo[k], &fmt))
                    printf("get pix fmt error\n");
                NSImageio::NSIspio::ISP_QuerySize(
                                    NSImageio::NSIspio::EPortIndex_CAMSV2_IMGO,
                                    NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                    NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                    fmt,
                                    sensorSize[k].w,
                                    queryRst,
                                    e_PixMode[k]
                                    );


                imgiBuf.size = sensorSize[k].h* queryRst.stride_byte;
                mpImemDrv->allocVirtBuf(&imgiBuf);
                ////imem buffer 2 image heap
                MUINT32 bufStridesInBytes[3] = {queryRst.stride_byte, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((bitmap(SEN_PIX_BITDEPTH)),
                                                        sensorSize[k], bufStridesInBytes, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,MTRUE);
                pImgBuffer[k][__CAMSV2][i] = pHeap->createImageBuffer();
                pImgBuffer[k][__CAMSV2][i]->incStrong(pImgBuffer);
                pImgBuffer[k][__CAMSV2][i]->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                printf("senidx:0x%x img buffer(%d): imgo pa:0x%x\n",k,i,pImgBuffer[k][__CAMSV2][i]->getBufPA(0));
            }

        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //push enque buf/dummy frame into drv before start. to set FBC number
    //enque buf
    //in this example, enque buf with mag : 1,2,3
    printf("start push enque buf/dummy frame into drv\n");
    QBufInfo buf;
    MSize _rrz ;
    for(int k=sensorIdx;k<dualSensor;k++){
        for(int i=0;i<Enque_buf;i++){
            buf.mvOut.clear();
            if(enablePort & __RRZO_ENABLE){
                //note:crop size can't < outsize
                _rrz = __SCALE_SIZE(__RRZO,pImgBuffer[k][__RRZO][i]->getImgFormat(),sensorSize[k],RRZ_SCALING_RATIO,e_PixMode[k]);
                _cropsize = __CROP_SIZE(__RRZO,pImgBuffer[k][__RRZO][i]->getImgFormat(),sensorSize[k],e_PixMode[k]);
                BufInfo _buf = BufInfo(PORT_RRZO,pImgBuffer[k][__RRZO][i],_rrz,MRect(__CROP_Start(sensorSize[k],_cropsize,pImgBuffer[k][__RRZO][i]->getImgFormat(),croptestsel),_cropsize),i+1);
                buf.mvOut.push_back(_buf);
            }
            if(enablePort & __IMGO_ENABLE){
                //_cropsize = __CROP_SIZE(__IMGO,pImgBuffer[k][__IMGO][i]->getImgFormat(),sensorSize[k],e_PixMode[k]);
                _cropsize = MSize(sensorSize[k].w,sensorSize[k].h);//sensor size
                BufInfo _buf = BufInfo(PORT_IMGO,pImgBuffer[k][__IMGO][i],_cropsize,MRect(__CROP_Start(sensorSize[k],_cropsize,pImgBuffer[k][__IMGO][i]->getImgFormat(),0),_cropsize),i+1);
                buf.mvOut.push_back(_buf);
            }
            if(enablePort & __CAMSV_ENABLE){
                _cropsize = MSize(sensorSize[k].w,sensorSize[k].h);
                BufInfo _buf = BufInfo(PORT_CAMSV_IMGO,pImgBuffer[k][__CAMSV][i],_cropsize,MRect(__CROP_Start(sensorSize[k],_cropsize,pImgBuffer[k][__CAMSV][i]->getImgFormat(),0),_cropsize),i+1);
                buf.mvOut.push_back(_buf);
            }
            if(enablePort & __CAMSV2_ENABLE){
                _cropsize = MSize(sensorSize[k].w,sensorSize[k].h);
                BufInfo _buf = BufInfo(PORT_CAMSV2_IMGO,pImgBuffer[k][__CAMSV2][i],_cropsize,MRect(__CROP_Start(sensorSize[k],_cropsize,pImgBuffer[k][__CAMSV2][i]->getImgFormat(),0),_cropsize),i+1);
                buf.mvOut.push_back(_buf);
            }
            if(CAM_V1 == ISPversion)
                mpCamIO[k]->enque(buf);
            else{
                buf.mvOut.at(0).FrameBased.mSOFidx = 0;//initial
                printf("BufIdx(0x%x),Sofidx(0x%x)\n",buf.mvOut.at(0).mBufIdx,buf.mvOut.at(0).FrameBased.mSOFidx);
                mpCamIO_FrmB[k]->enque(buf);
            }
        }
    }
    //in this example, dummy buf with mag : 4,5,6
    if(CAM_V3 == ISPversion){
        for(int k=sensorIdx;k<dualSensor;k++){
            for(int i=Dummy_buf;i<(Enque_buf+Dummy_buf);i++){
                buf.mvOut.clear();
                if(enablePort & __RRZO_ENABLE){
                    _rrz = __SCALE_SIZE(__RRZO,pImgBuffer[k][__RRZO][i]->getImgFormat(),sensorSize[k],RRZ_SCALING_RATIO,e_PixMode[k]);
                    _cropsize = __CROP_SIZE(__RRZO,pImgBuffer[k][__RRZO][i]->getImgFormat(),sensorSize[k],e_PixMode[k]);
                    BufInfo _buf = BufInfo(PORT_RRZO,pImgBuffer[k][__RRZO][i],_rrz,MRect(__CROP_Start(sensorSize[k],_cropsize,pImgBuffer[k][__RRZO][i]->getImgFormat(),croptestsel),_cropsize),i+1);
                    buf.mvOut.push_back(_buf);
                }
                if(enablePort & __IMGO_ENABLE){
                    //_cropsize = __CROP_SIZE(__IMGO,pImgBuffer[k][__IMGO][i]->getImgFormat(),sensorSize[k],e_PixMode[k]);
                    _cropsize = MSize(sensorSize[k].w,sensorSize[k].h);//sensor size
                    BufInfo _buf = BufInfo(PORT_IMGO,pImgBuffer[k][__IMGO][i],_cropsize,MRect(__CROP_Start(sensorSize[k],_cropsize,pImgBuffer[k][__IMGO][i]->getImgFormat(), 0),_cropsize),i+1);
                    buf.mvOut.push_back(_buf);
                }
                buf.mvOut.at(0).FrameBased.mSOFidx = 0;//initial
                printf("BufIdx(0x%x),Sofidx(0x%x)\n",buf.mvOut.at(0).mBufIdx,buf.mvOut.at(0).FrameBased.mSOFidx);
                mpCamIO_FrmB[k]->DummyFrame(buf);
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //start isp
    printf("!!!ISP START\n");
    if(CAM_V1 == ISPversion){
        for(int i=sensorIdx;i<dualSensor;i++)
            mpCamIO[i]->start();
    }
    else{
        for(int i=sensorIdx;i<dualSensor;i++)
            mpCamIO_FrmB[i]->start();
    }

    /**
        note: need to start isp fist(can't use deque first,than use another to start isp, or deadlock will hanppen)
    */
    //start deque/enque thread
    printf("start deque/denque thread\n");
    _test_camio* pCamio[_MAXTG_] = {NULL};
    for(int i=sensorIdx;i<dualSensor;i++){
        pCamio[i] = _test_camio::create();
        pCamio[i]->ArbitrayCropSel(croptestsel);
        pCamio[i]->m_pNormalP = mpCamIO[i];
        pCamio[i]->m_pNormalP_FrmB = mpCamIO_FrmB[i];
        pCamio[i]->m_enablePort = enablePort;
        pCamio[i]->mPixMode = e_PixMode[i];
        if(pCamio[i]->m_enablePort & __RRZO_ENABLE)
            pCamio[i]->m_prepalce_Buf[__RRZO] = pImgBuffer[i][__RRZO][(Enque_buf+Dummy_buf)];
        if(pCamio[i]->m_enablePort & __IMGO_ENABLE)
            pCamio[i]->m_prepalce_Buf[__IMGO] = pImgBuffer[i][__IMGO][(Enque_buf+Dummy_buf)];
        pCamio[i]->m_TgSize = sensorSize[i];
        pCamio[i]->m_sensoridx = i;
        pCamio[i]->startThread();
    }

   /*////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
   // getchar();
   // printf("!!!!!!!! press enter to stop... !!!!!!!!\n");//*/

    //CRC
    if (CheckCRC==0)
    {
        printf("without check_crc\n");
    }
    else
    {
        int CheckCam = CheckCRC-1;
            printf("crop size width:(%d) x height:(%d)\n", sensorSize[CheckCam].w,sensorSize[CheckCam].h);

        if(!getOutputFmt(PORT_IMGO,SEN_PIX_BITDEPTH,mSensorInfo[CheckCam], &fmt))
          printf("get pix fmt error\n");
        NSImageio::NSIspio::ISP_QuerySize(
                            NSImageio::NSIspio::EPortIndex_IMGO,
                            NSImageio::NSIspio::ISP_QUERY_X_PIX|
                            NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                            NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                            fmt,
                            sensorSize[CheckCam].w,
                            queryRst,
                            e_PixMode[CheckCam]
                            );

        imgiBuf.size = sensorSize[CheckCam].h* queryRst.stride_byte;
        printf("!!!!!!!! check_crc start !!!!!!!!\n");
        printf("[%d]imgiBuf.size=%d,size=(%d,%d)\n",CheckCam,imgiBuf.size,sensorSize[CheckCam].w,sensorSize[CheckCam].h);
        MUINT32 mGetCheckSumValue=0;
        MUINT32 check_ret=Camera_check_crc((MUINT8*)pImgBuffer[CheckCam][__IMGO][0]->getBufVA(0), mGetCheckSumValue,imgiBuf.size);

        if(check_ret==1)
        {
            printf("!!!!!!!! Senser Check_crc successed! !!!!!!!!\n");
        }
        else{
            printf("!!!!!!!! Senser Check_crc failed! !!!!!!!!\n");
        }
    }

#if 0
    printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
    getchar();
    printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
#endif

    //sleep
    printf("Sleep 3 sec\n");
    usleep(3000000);

    //Save raw images
    filecnt = 0;
    printf("Save raw images\n");
    //Save File
    for(int k=sensorIdx;k<dualSensor;k++){
        for(int i=0;i<Enque_buf;i++){
            if(enablePort & __RRZO_ENABLE){
                //note:crop size can't < outsize
                onDumpBuffer("camio", 1, (MUINTPTR)pImgBuffer[k][__RRZO][i], k);
            }
            if(enablePort & __IMGO_ENABLE){
                onDumpBuffer("camio", 0, (MUINTPTR)pImgBuffer[k][__IMGO][i], k);
            }
        }
    }
    //in this example, dummy buf with mag : 4,5,6
    if(CAM_V3 == ISPversion){
        for(int k=sensorIdx;k<dualSensor;k++){
            for(int i=Dummy_buf;i<(Enque_buf+Dummy_buf);i++){
                if(enablePort & __RRZO_ENABLE){
                    onDumpBuffer("camio", 1, (MUINTPTR)pImgBuffer[k][__RRZO][i], k);
                }
                if(enablePort & __IMGO_ENABLE){
                    onDumpBuffer("camio", 0, (MUINTPTR)pImgBuffer[k][__IMGO][i], k);
                }
            }
        }
    }

     printf("Stoping ISP\n");
    if(CAM_V1 == ISPversion)
    {
        for(int i=sensorIdx;i<dualSensor;i++){
            printf("[%d] sem_wait(thread)\n", i);
            pCamio[i]->stopThread();
            printf("!!!!!!!! [%d] thread stoped... !!!!!!!!\n", i);
            mpCamIO[i]->stop();
            printf("!!!!!!!! [%d] isp stoped... !!!!!!!!\n", i);

            mpCamIO[i]->uninit();
            mpCamIO[i]->destroyInstance(LOG_TAG);

            //poweroff sensor
            pSensorHalObj[i]->powerOff(LOG_TAG,1, &sensorArray[i]);
        }
    }
    else{
        for(int i=sensorIdx;i<dualSensor;i++){
            printf("[%d] sem_wait(thread)\n", i);
            pCamio[i]->stopThread();
            printf("!!!!!!!! [%d] thread stoped... !!!!!!!!\n", i);
            mpCamIO_FrmB[i]->stop();
            printf("!!!!!!!! [%d] isp stoped... !!!!!!!!\n", i);

            mpCamIO_FrmB[i]->uninit();
            mpCamIO_FrmB[i]->destroyInstance(LOG_TAG);

            //poweroff sensor
            pSensorHalObj[i]->powerOff(LOG_TAG,1, &sensorArray[i]);
        }
    }
    return ret;
}
