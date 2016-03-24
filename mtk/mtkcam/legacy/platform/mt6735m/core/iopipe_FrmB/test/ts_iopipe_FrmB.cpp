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

#include <mtkcam/common.h>
//#include <mtkcam/camnode/AllocBufHandler.h>
//
//
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>


#include <iopipe_FrmB/CamIO/PortMap_FrmB.h>
//

#include <mtkcam/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/ImageBufferHeap.h>
#include <mtkcam/drv_common/imem_drv.h>
#include <semaphore.h>
#include <pthread.h>
#include <mtkcam/v1/config/PriorityDefs.h>
#include <utils/threads.h>
#include <mtkcam/imageio/ispio_utility.h>

#define USE_MM_DVFS     (1)
#if USE_MM_DVFS == 1
#include <bandwidth_control.h>
#endif

#undef LOG_TAG
#define LOG_TAG "camiopipetest"

using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

using namespace NSImageio_FrmB::NSIspio_FrmB;
using namespace NSImageio_FrmB;


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
#define HRZ_SCALING_RATIO    (2)    //divisor
#define __IMG2O 0
#define __IMGO 1
#define __MAXDMAO    (__IMG2O + __IMGO + 1)
#define __IMG2O_ENABLE (0x1)
#define __IMGO_ENABLE (0x2)

//because support only symetric cropping.
#define __CROP_Start(in,crop,fmt) ({\
    NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_RST queryRst;\
    MPoint _point;\
    NSImageio_FrmB::NSIspio_FrmB::ISP_QuerySize(_BY_PASSS_PORT,NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_CROP_START_X,\
            (EImageFormat)fmt,(in.w-crop.w)/2,queryRst);\
    _point = MPoint(queryRst.crop_x,(in.h-crop.h)/2);\
    _point;})

//croping size have an alignment-rule in imgo/imgo_d (no resizer dma port)
//img2o do nothing, because of img2o not using dmao cropping.
//note :
//     the query operation, if cropsize != inputsize , use ISP_QUERY_CROP_X_PIX to query.
#define __CROP_SIZE(portId,fmt,size,pixmode) ({\
    MSize ___size;\
    if(portId == __IMGO){\
        ___size = MSize((size.w/HRZ_SCALING_RATIO + 64),(size.h/HRZ_SCALING_RATIO + 64));\
        NSImageio_FrmB::NSIspio_FrmB::E_ISP_QUERY __op;\
        __op = (size.w != ___size.w)?(NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_CROP_X_PIX):(NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_X_PIX);\
        NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_RST queryRst;\
        NSImageio_FrmB::NSIspio_FrmB::ISP_QuerySize(NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGO,__op,\
            (EImageFormat)fmt,___size.w,queryRst,pixmode);\
            ___size.w = queryRst.x_pix;\
    }\
    if(portId == __IMG2O){\
        ___size = MSize(size.w,size.h);\
    }\
    ___size;})

#define __HRZ_FMT(fmt)({\
    MUINT32 _fmt;\
    switch(fmt){\
        case eImgFmt_BAYER8: _fmt = eImgFmt_BAYER8; break;    \
        case eImgFmt_BAYER10: _fmt = eImgFmt_BAYER10; break; \
        case eImgFmt_BAYER12: _fmt = eImgFmt_BAYER12; break; \
        default: _fmt = eImgFmt_BAYER10; break;                \
    }\
    _fmt;})


//imgo do nothing, because of imgo have no resizer
//img2o have no vertical scaler
#define __SCALE_SIZE(portId,fmt,size,Ratio,pixmode)({\
    MSize __size;\
    __size = MSize((size.w/Ratio),size.h);\
    if(portId == __IMG2O){\
        NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_RST queryRst;\
        NSImageio_FrmB::NSIspio_FrmB::ISP_QuerySize(NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMG2O,NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_X_PIX,\
            (EImageFormat)__HRZ_FMT(fmt),__size.w,queryRst,pixmode);\
            __size.w = queryRst.x_pix;\
    }\
    __size;})



typedef struct HwPortConfig{
    NSCam::NSIoPipe::PortID          mPortID;
    EImageFormat    mFmt;
    MSize           mSize;
    MRect           mCrop;
    MBOOL           mPureRaw;
    MUINT32         mStrideInByte[3];
}HwPortConfig_t;

class _test_camio
{
    _test_camio()
    {
        m_pNormalP_FrmB = NULL;
        for(int i=0;i<__MAXDMAO;i++)
            m_prepalce_Buf[i]=NULL;
        m_enablePort=0x0;m_bStop = MFALSE;m_TgSize = MSize(0,0);
    };
    ~_test_camio(){};
    public:
        INormalPipe_FrmB*    m_pNormalP_FrmB;
        IImageBuffer*        m_prepalce_Buf[__MAXDMAO];
        MUINT32                 m_enablePort;
        MSize                m_TgSize;
        NSImageio_FrmB::NSIspio_FrmB::E_ISP_PIXMODE mPixMode;

        static _test_camio*     create(void);
        void             destroy(void);
        void             startThread(void);
        void             stopThread(void);
#if NOTIFY_TEST
        static void             jm_test(MUINT32 mag);
#endif
    private:
        static MVOID*    _infiniteloop(void* arg);
        MBOOL              m_bStop;

        pthread_t         m_Thread;
        sem_t            m_semThread;
};
#if NOTIFY_TEST
void _test_camio::jm_test(MUINT32 mag){
    printf("drop enque request:0x%x\n",mag);
}
#endif
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
    ::sem_wait(&this->m_semThread);
}

MVOID* _test_camio::_infiniteloop(void* arg)
{
    _test_camio* _this = (_test_camio*)arg;
    QBufInfo dequeBufInfo;
    QBufInfo _replace;
    dequeBufInfo.mvOut.reserve(2);
    printf("start infiniteloop\n");
    MUINT32 _loop = 0;
    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());
    _replace.mvOut.clear();
    if( _this->m_enablePort & __IMGO_ENABLE) {
        //enque
        NSCam::NSIoPipe::NSCamIOPipe::BufInfo OutBuf(PORT_IMGO, 0);
        OutBuf.FrameBased.mSOFidx = 0;
        dequeBufInfo.mvOut.push_back(OutBuf);
        MSize _cropsize = __CROP_SIZE(__IMGO,_this->m_prepalce_Buf[__IMGO]->getImgFormat(),_this->m_TgSize,_this->mPixMode);

        //replace
        //in replace testing , use mag num:7
        NSCam::NSIoPipe::NSCamIOPipe::BufInfo InBuf(PORT_IMGO, _this->m_prepalce_Buf[__IMGO],_cropsize,MRect(__CROP_Start(_this->m_TgSize,_cropsize,_this->m_prepalce_Buf[__IMGO]->getImgFormat()),_cropsize),(replace_buf + Enque_buf + Dummy_buf));
        InBuf.FrameBased.mSOFidx = 0;
        _replace.mvOut.push_back(InBuf);
    }
    if( _this->m_enablePort & __IMG2O_ENABLE) {
        //enque
        NSCam::NSIoPipe::NSCamIOPipe::BufInfo OutBuf(PORT_IMG2O, 0);
        OutBuf.FrameBased.mSOFidx = 0;
        dequeBufInfo.mvOut.push_back(OutBuf);
        MSize _cropsize = __CROP_SIZE(__IMG2O,_this->m_prepalce_Buf[__IMG2O]->getImgFormat(),\
            __SCALE_SIZE(__IMG2O,_this->m_prepalce_Buf[__IMG2O]->getImgFormat(),_this->m_TgSize,HRZ_SCALING_RATIO,_this->mPixMode),_this->mPixMode);

        //replace
        MSize _hrz = __SCALE_SIZE(__IMG2O,_this->m_prepalce_Buf[__IMG2O]->getImgFormat(),_this->m_TgSize,HRZ_SCALING_RATIO,_this->mPixMode);
        printf("_hrz size : 0x%x 0x%x (0x%x 0x%x)\n",_hrz.w,_hrz.h,_this->m_TgSize.w,_this->m_TgSize.h);
        //in replace testing , use mag num:7
        //note, img2o crop is after scaler, so input is hrz out
        NSCam::NSIoPipe::NSCamIOPipe::BufInfo InBuf(PORT_IMG2O, _this->m_prepalce_Buf[__IMG2O],_hrz,
                MRect(MPoint(0,0), _this->m_TgSize),(replace_buf + Enque_buf + Dummy_buf));
        InBuf.FrameBased.mSOFidx = 0;
        _replace.mvOut.push_back(InBuf);
    }

    static bool shouldPrint = true;

    while(1){
        if(_this->m_bStop == MTRUE){
            printf("stop enque/deque\n");
            break;
        }
        _loop++;
        if(_this->m_pNormalP_FrmB->deque(dequeBufInfo) == MTRUE){
            for(int i=0;i<dequeBufInfo.mvOut.size();i++){
                printf("[E_L deque]: dma:0x%x, PA:0x%x, crop:0x%x,0x%x,0x%x,0x%x_0x%x,0x%x,0x%x,0x%x, size:0x%x,0x%x, mag:0x%x, dummy:0x%x, bufidx:0x%x\n",\
                dequeBufInfo.mvOut.at(i).mPortID.index,dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0),\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.y,\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.h,\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.y,\
                dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.h,\
                dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.w,dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.h,\
                dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_tuning,dequeBufInfo.mvOut.at(i).mMetaData.m_bDummyFrame,\
                dequeBufInfo.mvOut.at(i).mBufIdx);
            }

            //dump ++
            if (shouldPrint)
            {
                char filename[256];
                for(int i=0;i<dequeBufInfo.mvOut.size();i++)
                {
                    sprintf(filename, "/sdcard/pictures/P1_%d_%d_%d_%d.raw", _this->m_enablePort, dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_hal,
                        dequeBufInfo.mvOut.at(i).mBuffer->getImgSize().w,
                        dequeBufInfo.mvOut.at(i).mBuffer->getImgSize().h);
                    NSCam::Utils::saveBufToFile(filename, (unsigned char*)dequeBufInfo.mvOut.at(i).mBuffer->getBufVA(0),
                            dequeBufInfo.mvOut.at(i).mBuffer->getBufSizeInBytes(0));
                    printf("[pee]%d\n", dequeBufInfo.mvOut.at(i).mBuffer->getBufSizeInBytes(0));
                }
                shouldPrint = false;
            }
            // --

            //use replace buffer to enque,simulate new request in camera3
            for(int i=0;i<_replace.mvOut.size();i++){
                printf("[E_L enque]: dma:0x%x, PA:0x%x, crop:0x%x,0x%x,0x%x,0x%x, size:0x%x,0x%x, mag:0x%x, bufidx:0x%x\n",\
                _replace.mvOut.at(i).mPortID.index,_replace.mvOut.at(i).mBuffer->getBufPA(0),\
                _replace.mvOut.at(i).FrameBased.mCropRect.p.x,_replace.mvOut.at(i).FrameBased.mCropRect.p.y,\
                _replace.mvOut.at(i).FrameBased.mCropRect.s.w,_replace.mvOut.at(i).FrameBased.mCropRect.s.h,\
                _replace.mvOut.at(i).FrameBased.mDstSize.w,_replace.mvOut.at(i).FrameBased.mDstSize.h,\
                _replace.mvOut.at(i).FrameBased.mMagicNum_tuning,\
                _replace.mvOut.at(i).mBufIdx);
            }
            usleep(10000);//sleep 10ms to cross over vsync and simulate 3a calculation
            if((_loop % 100) == 99)
            {
                usleep(20000);//make drop frame status happened intenionally every 100 frames
            }
            _this->m_pNormalP_FrmB->enque(_replace);
            //update replace buffer to previous deque result
            _replace = dequeBufInfo;
            //note: must asign result to input
            for(int i=0;i<dequeBufInfo.mvOut.size();i++){
                if(dequeBufInfo.mvOut.at(i).mPortID == PORT_IMGO)
                    _replace.mvOut.at(i).FrameBased.mCropRect = dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d;
                else if(dequeBufInfo.mvOut.at(i).mPortID == PORT_IMG2O)
                    _replace.mvOut.at(i).FrameBased.mCropRect = dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s;
                _replace.mvOut.at(i).FrameBased.mDstSize = dequeBufInfo.mvOut.at(i).mMetaData.mDstSize;
                _replace.mvOut.at(i).FrameBased.mMagicNum_tuning = dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_tuning;
                _replace.mvOut.at(i).mBufIdx = 0xffff;//must be, caused bufidx is that a output data of deque, this value will be reset by line:294
            }
#if NOTIFY_TEST
            _this->m_pNormalP_FrmB->enque(_replace);
#endif
        }
        else{
            MBOOL _break = MFALSE;
            //if deque fail is because of current dequed frame is dummy frame, bypass this error return.
            for(int i=0;i<dequeBufInfo.mvOut.size();i++){
                if(dequeBufInfo.mvOut.at(i).mMetaData.m_bDummyFrame == MFALSE)
                    _break = MTRUE;
            }
            if(_break == MTRUE){
                printf("deque fail, leave infinite_loop\n");
                break;
            }
        }
#if 1
        if (_loop == 60)
        {
            printf("Stop @ frame 60\n");
            _this->m_bStop = MTRUE;
        }
#endif
    }

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
            (MINTPTR)&scenario,
            (MINTPTR)&sensorFps,
            (MINTPTR)pPixelMode);
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

MBOOL getOutputFmt(NSCam::NSIoPipe::PortID port,MUINT32 bitDepth,SensorStaticInfo mSensorInfo,EImageFormat* pFmt)
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
        if(port == PORT_IMGO) //imgo
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
        else // img2o
        {
            switch( bitDepth)
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
int main_iopipe(int testcaseType,int testcaseNum)
{
    int ret = 0;
    printf("supported bit:0x%x, hrz scaling ratio:0x%x\n",SEN_PIX_BITDEPTH,HRZ_SCALING_RATIO);
    MUINT32 sensorIdx = 0;    //0 for main
    MUINT32 scenario  ;//= atoi(argv[2]); //prv:0, cap:1
    MUINT32 sensorFps = 30;    //30 for 30fps
    MUINT32 enablePort ;//= atoi(argv[4]);//img2o:0x1, imgo:0x2, img2o+imgo:0x3

    #if USE_MM_DVFS == 1
    BWC bwc;
    printf("MM DVFS : HPM\n");
    bwc.Profile_Change(BWCPT_FORCE_MMDVFS, MTRUE);
    #endif

    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    pHalSensorList->searchSensors();
    switch(testcaseNum){
        case 0:    //prv_fps30_imgo
            scenario = 0;
            enablePort = 0x2;
            break;
        case 1:    //prv_fps30_img2o
            scenario = 0;
            enablePort = 0x1;
            break;
        case 2: //prv_fps30_imgo+img2o
            scenario = 0;
            enablePort = 0x3;
            break;
        case 3:    //cap_fps30_imgo
            scenario = 1;
            enablePort = 0x2;
            break;
        case 4: //cap_fps30_img2o
            scenario = 1;
            enablePort = 0x1;
            break;
        case 5:    //cap_fps30_imgo+img2o
            scenario = 1;
            enablePort = 0x3;
            break;
        default:
            printf("unsupported testcaseNum0x%x\n",testcaseNum);
            return 0;
            break;
    }

    printf("sensorIdx:0x%x,senario:0x%x,sensorFps:0x%x,enablePort = 0x%x\n",sensorIdx,scenario,sensorFps,enablePort);
    uint32_t    sensorArray[1];
    SensorStaticInfo mSensorInfo;
    //////////////////////////////////////////////////////
    //powerOn sensor
    IHalSensor* pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, sensorIdx);
    if(pSensorHalObj == NULL)
    {
       printf("mpSensorHalObj is NULL");
    }
    //
    sensorArray[0] = sensorIdx;
    pSensorHalObj->powerOn(LOG_TAG, 1, &sensorArray[0]);
    //


    /////////////////////////////////////////////////////////////////////////
    //querysensor
    {
        IHalSensorList* const pHalSensorList = IHalSensorList::get();

        pHalSensorList->querySensorStaticInfo(
                pHalSensorList->querySensorDevIdx(sensorIdx),
                &mSensorInfo);

    }
    //////////////////////////////////////////////////////////////////////
    //create/init normalpipe_frmb
    INormalPipe_FrmB* mpCamIO = INormalPipe_FrmB::createInstance((MINT32)sensorIdx, LOG_TAG,1);
    mpCamIO->init();

    /////////////////////////////////////////////////////////////////////////
    //prepare sensor cfg
    MSize sensorSize;
    getSensorSize(&sensorSize,mSensorInfo,scenario);
    vector<IHalSensor::ConfigParam> vSensorCfg;
    IHalSensor::ConfigParam sensorCfg =
    {
        (MUINT)sensorIdx,                  /* index            */
        sensorSize,                     /* crop             */
        scenario,                        /* scenarioId       */
        0,                              /* isBypassScenario */
        1,                              /* isContinuous     */
        MFALSE,                         /* iHDROn           */
        sensorFps,                       /* framerate        */
        0,                              /* two pixel on     */
        0,                              /* debugmode        */
    };
    vSensorCfg.push_back(sensorCfg);
    printf("sensor %dx%d, sce %d, bypass %d, con %d, hdr %d, fps %d, twopxl %d\n",
                sensorCfg.crop.w,
                sensorCfg.crop.h,
                sensorCfg.scenarioId,
                sensorCfg.isBypassScenario,
                sensorCfg.isContinuous,
                sensorCfg.HDRMode,
                sensorCfg.framerate,
                sensorCfg.twopixelOn);

    ////////////////////////////////////////////////////////////////////////
    //isp dmao cfg
    printf("start isp dmao cfg\n");
    list<HwPortConfig_t> lHwPortCfg;
    EImageFormat fmt;
    NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_RST queryRst;
    MUINT32 pixelMode = 0;
    MSize _cropsize;
    NSImageio_FrmB::NSIspio_FrmB::E_ISP_PIXMODE e_PixMode =  ((pixelMode == 0) ?  (NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_1_PIX_MODE) : (NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_2_PIX_MODE));
    getSensorPixelMode(&pixelMode,sensorIdx,scenario,sensorFps);

    if(enablePort & __IMGO_ENABLE){
        printf("port IMGO\n");
        if( !getOutputFmt(PORT_IMGO,SEN_PIX_BITDEPTH,mSensorInfo, &fmt))
            printf("get pix fmt error\n");
        NSImageio_FrmB::NSIspio_FrmB::ISP_QuerySize(
                                NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGO,
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_X_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_BYTE,
                                fmt,
                                sensorSize.w,
                                queryRst,
                                e_PixMode
                                );
        _cropsize = __CROP_SIZE(__IMGO,fmt,sensorSize,e_PixMode);
        printf(" imgo crop _size %d %d, stride %d\n", _cropsize.w, _cropsize.h, queryRst.stride_byte);
        HwPortConfig_t full = {
            PORT_IMGO,
            fmt,
            _cropsize,
            MRect(__CROP_Start(sensorSize,_cropsize,fmt),_cropsize),
            0, //if raw type != 1 -> pure-raw
            { queryRst.stride_byte, 0, 0 }
        };
        lHwPortCfg.push_back(full);
    }
    if(enablePort & __IMG2O_ENABLE){
        printf("port IMG2O\n");
        MSize _size;
        if( !getOutputFmt(PORT_IMG2O,SEN_PIX_BITDEPTH,mSensorInfo, &fmt))
            printf("get pix fmt error\n");

        _size = __SCALE_SIZE(__IMG2O,fmt,sensorSize,HRZ_SCALING_RATIO,e_PixMode);
        printf(" img2o scale _size %d %d\n", _size.w, _size.h);
        NSImageio_FrmB::NSIspio_FrmB::ISP_QuerySize(
                                NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMG2O,
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_X_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_BYTE,
                                fmt,
                                _size.w,
                                queryRst,
                                e_PixMode
                                );
        //crop size can't < outsize
        //__CROP_SIZE(sensorSize) must > _size in img2o
        //note, hrz crop is after scaler, so in put is hrz out
        //_cropsize = __CROP_SIZE(__IMG2O,fmt,_size,e_PixMode);
        HwPortConfig_t resized = {
            PORT_IMG2O,
            fmt,
            _size,
            //note ,hrz crop is after scaler, so in put is hrz out
            MRect(MPoint(0,0), sensorSize),
            0, //if raw type != 1 -> pure-raw
            { queryRst.stride_byte, 0, 0 }
        };
        lHwPortCfg.push_back(resized);
    }
    printf("hwport size:0x%x\n",lHwPortCfg.size());
    ////////////////////////////////////////////////////////////////////////////
    //configpipe
    printf("start configpipe\n");
    vector<portInfo> vPortInfo;
    list<HwPortConfig_t>::const_iterator pPortCfg;
    for( pPortCfg = lHwPortCfg.begin(); pPortCfg != lHwPortCfg.end(); pPortCfg++ )
    {
        printf("id:0x%x, crop:%d,%d,%dx%d, size:%dx%d, fmt:0x%x, stride:%d, pureraw:%d\n",\
        pPortCfg->mPortID.index,\
        pPortCfg->mCrop.p.x,pPortCfg->mCrop.p.y,pPortCfg->mCrop.s.w,pPortCfg->mCrop.s.h,\
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
        vPortInfo.push_back(OutPort);
    }
    //
    QInitParam halCamIOinitParam(
            0, // 2: sensor uses pattern
            SEN_PIX_BITDEPTH,
            vSensorCfg,
            vPortInfo);
    //
    //NG_TRACE_BEGIN("configP1");
#if NOTIFY_TEST
    halCamIOinitParam.m_DropCB = _test_camio::jm_test;
#endif
    if( !mpCamIO->configPipe(halCamIOinitParam) ) {
        printf("configPipe failed\n");
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
    IImageBuffer* pImgBuffer[__MAXDMAO][(Enque_buf+Dummy_buf+replace_buf)];


    for(int i=0;i<(Enque_buf+Dummy_buf+replace_buf);i++){
        if(enablePort & __IMG2O_ENABLE){
            MSize _size((sensorSize.w/HRZ_SCALING_RATIO),sensorSize.h);
            if( !getOutputFmt(PORT_IMG2O,SEN_PIX_BITDEPTH,mSensorInfo, &fmt))
                printf("get pix fmt error\n");
            NSImageio_FrmB::NSIspio_FrmB::ISP_QuerySize(
                                NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMG2O,
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_X_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_BYTE,
                                fmt,
                                _size.w,
                                queryRst,
                                pixelMode == 0 ?  NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_1_PIX_MODE :
                                                  NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_2_PIX_MODE
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
            pImgBuffer[__IMG2O][i] = pHeap->createImageBuffer();
            pImgBuffer[__IMG2O][i]->incStrong(pImgBuffer);
            pImgBuffer[__IMG2O][i]->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            printf("img buffer(%d): img2o pa:0x%x\n",i,pImgBuffer[__IMG2O][i]->getBufPA(0));
        }
        if(enablePort & __IMGO_ENABLE){
            if( !getOutputFmt(PORT_IMGO,SEN_PIX_BITDEPTH,mSensorInfo, &fmt))
                printf("get pix fmt error\n");
            NSImageio_FrmB::NSIspio_FrmB::ISP_QuerySize(
                                NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGO,
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_X_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_PIX|
                                NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_STRIDE_BYTE,
                                fmt,
                                sensorSize.w,
                                queryRst,
                                pixelMode == 0 ?  NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_1_PIX_MODE :
                                                  NSImageio_FrmB::NSIspio_FrmB::ISP_QUERY_2_PIX_MODE
                                );


            imgiBuf.size = sensorSize.h* queryRst.stride_byte;
            mpImemDrv->allocVirtBuf(&imgiBuf);
            ////imem buffer 2 image heap
            MUINT32 bufStridesInBytes[3] = {queryRst.stride_byte, 0, 0};
            MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
            PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

            IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((bitmap(SEN_PIX_BITDEPTH)),
                                                    sensorSize, bufStridesInBytes, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,MTRUE);
            pImgBuffer[__IMGO][i] = pHeap->createImageBuffer();
            pImgBuffer[__IMGO][i]->incStrong(pImgBuffer);
            pImgBuffer[__IMGO][i]->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            printf("img buffer(%d): imgo pa:0x%x\n",i,pImgBuffer[__IMGO][i]->getBufPA(0));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //push enque buf/dummy frame into drv before start. to set FBC number
    //enque buf
    //in this example, enque buf with mag : 1,2,3
    printf("start push enque buf/dummy frame into drv\n");
    QBufInfo buf;
    MSize _hrz ;

    for(int i=0;i<Enque_buf;i++){
        buf.mvOut.clear();
        if(enablePort & __IMG2O_ENABLE){
            //note:crop size can't < outsize
            _hrz = __SCALE_SIZE(__IMG2O,pImgBuffer[__IMG2O][i]->getImgFormat(),sensorSize,HRZ_SCALING_RATIO,e_PixMode);
            //_cropsize = __CROP_SIZE(__IMG2O,pImgBuffer[__IMG2O][i]->getImgFormat(),_hrz,e_PixMode);
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo _buf = NSCam::NSIoPipe::NSCamIOPipe::BufInfo(PORT_IMG2O,pImgBuffer[__IMG2O][i],_hrz,
                    MRect(MPoint(0,0),sensorSize),i+1);
            _buf.FrameBased.mSOFidx = 0;
            buf.mvOut.push_back(_buf);
        }
        if(enablePort & __IMGO_ENABLE){
            _cropsize = __CROP_SIZE(__IMGO,pImgBuffer[__IMGO][i]->getImgFormat(),sensorSize,e_PixMode);
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo _buf = NSCam::NSIoPipe::NSCamIOPipe::BufInfo(PORT_IMGO,pImgBuffer[__IMGO][i],
                    _cropsize,MRect(__CROP_Start(sensorSize,_cropsize,pImgBuffer[__IMGO][i]->getImgFormat()),_cropsize),i+1);
            _buf.FrameBased.mSOFidx = 0;
            buf.mvOut.push_back(_buf);
        }
        mpCamIO->enque(buf);
    }
    //in this example, dummy buf with mag : 4,5,6
    for(int i=Dummy_buf;i<(Enque_buf+Dummy_buf);i++){
        buf.mvOut.clear();
        if(enablePort & __IMG2O_ENABLE){
            _hrz = __SCALE_SIZE(__IMG2O,pImgBuffer[__IMG2O][i]->getImgFormat(),sensorSize,HRZ_SCALING_RATIO,e_PixMode);
            //_cropsize = __CROP_SIZE(__IMG2O,pImgBuffer[__IMG2O][i]->getImgFormat(),_hrz,e_PixMode);
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo _buf = NSCam::NSIoPipe::NSCamIOPipe::BufInfo(PORT_IMG2O,pImgBuffer[__IMG2O][i],_hrz,
                    MRect(MPoint(0,0),sensorSize),i+1);
            _buf.FrameBased.mSOFidx = 0;
            buf.mvOut.push_back(_buf);
        }
        if(enablePort & __IMGO_ENABLE){
            _cropsize = __CROP_SIZE(__IMGO,pImgBuffer[__IMGO][i]->getImgFormat(),sensorSize,e_PixMode);
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo _buf = NSCam::NSIoPipe::NSCamIOPipe::BufInfo(PORT_IMGO,pImgBuffer[__IMGO][i],_cropsize,MRect(__CROP_Start(sensorSize,_cropsize,pImgBuffer[__IMGO][i]->getImgFormat()),_cropsize),i+1);
            _buf.FrameBased.mSOFidx = 0;
            buf.mvOut.push_back(_buf);
        }
        mpCamIO->DummyFrame(buf);
    }
#if 1
    NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
    mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
    mpIspDrv->init(LOG_TAG);
    printf(" reg key Test56 : %d\n", mpIspDrv->registerIrq("Test56"));
    printf(" reg key Tst78  : %d\n", mpIspDrv->registerIrq("Tst78"));
    printf(" reg key Test56 : %d\n", mpIspDrv->registerIrq("Test56"));
    printf(" reg key Tst78  : %d\n", mpIspDrv->registerIrq("Tst78"));

    mpIspDrv->uninit(LOG_TAG);
    mpIspDrv->destroyInstance();
#endif

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //start isp
    printf("!!!ISP START\n");
    mpCamIO->start();

    /**
        note: need to start isp fist(can't use deque first,than use another to start isp, or deadlock will hanppen)
    */
    //start deque/enque thread
    printf("start deque/denque thread\n");
    _test_camio* pCamio = _test_camio::create();
    pCamio->m_pNormalP_FrmB = mpCamIO;
    pCamio->m_enablePort = enablePort;
    pCamio->mPixMode = e_PixMode;
    if(pCamio->m_enablePort & __IMG2O_ENABLE)
        pCamio->m_prepalce_Buf[__IMG2O] = pImgBuffer[__IMG2O][(Enque_buf+Dummy_buf)];
    if(pCamio->m_enablePort & __IMGO_ENABLE)
        pCamio->m_prepalce_Buf[__IMGO] = pImgBuffer[__IMGO][(Enque_buf+Dummy_buf)];
    pCamio->m_TgSize = sensorSize;
    pCamio->startThread();


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
    //getchar();
    pCamio->stopThread();
    printf("!!!!!!!! thread stoped... !!!!!!!!\n");
    mpCamIO->stop();
    printf("!!!!!!!! isp stoped... !!!!!!!!\n");


    mpCamIO->uninit();
    mpCamIO->destroyInstance(LOG_TAG);

    //poweroff sensor
    pSensorHalObj->powerOff(LOG_TAG,1, &sensorArray[0]);

    #if USE_MM_DVFS == 1
    printf("MM DVFS : LPM\n");
    bwc.Profile_Change(BWCPT_FORCE_MMDVFS, MFALSE);
    #endif

    return ret;
}
