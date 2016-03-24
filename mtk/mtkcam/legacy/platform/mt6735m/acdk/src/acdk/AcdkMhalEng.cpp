/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
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

//! \file  AcdkMhalEng.cpp

#define LOG_TAG "AcdkMhalEng"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <linux/rtpm_prio.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <vector>
#include <sys/resource.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
//#include <linux/cache.h>
#include <cutils/properties.h>
#include <semaphore.h>

#include <mtkcam/v1/config/PriorityDefs.h>

//#include <cutils/pmem.h>

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkErrCode.h"
#include "AcdkLog.h"
#include "mtkcam/acdk/AcdkCommon.h"
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
#include "AcdkBase.h"
#include "AcdkMhalBase.h"
#include "AcdkUtility.h"

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

#include "camera_custom_nvram.h"
#include <flash_mgr/flash_mgr.h>

using namespace NSACDK;
using namespace NSAcdkMhal;

namespace
{
    camera_module_t *mModule = NULL;
    camera_device_t *mDevice = NULL;
    int mNumberOfCameras = 0;
}

using namespace android;
#include "AcdkMhalEng.h"

#define MEDIA_PATH "/data"
#define ACDK_MT6582_MDP_WO_IRQ 1 //SL test ACDK_6582_MDP without IRQ don

/*******************************************************************************
* Global variable
*******************************************************************************/
static sem_t g_SemMainHigh, g_SemMainHighBack, g_SemMainHighEnd;
static pthread_t g_threadMainHigh;
static AcdkMhalEng *g_pAcdkMHalEngObj = NULL;
static acdkObserver g_acdkMhalObserver;
static acdkObserver g_acdkMhalCapObserver;

static MUINT32 is_yuv_sensor = 0;

static MINT32 g_acdkMhalEngDebug = 0;
static MBOOL mCaptureDone = MFALSE;

void AcdkMhalLoadModule()
{
    if(mNumberOfCameras)
        return;
    if (hw_get_module(CAMERA_HARDWARE_MODULE_ID,
                (const hw_module_t **)&mModule) < 0) {
        ACDK_LOGD("Could not load camera HAL module");
        mNumberOfCameras = 0;
    }
    else {
        ACDK_LOGD("Loaded \"%s\" camera module", mModule->common.name);
        mNumberOfCameras = mModule->get_number_of_cameras();
    }
}

status_t AcdkMhalOpenDevice(camera_module_t *module, const char *name)
{
    String8 const mName(name);
    ACDK_LOGD("Opening camera %s + \n", mName.string());
    int rc = module->open_legacy((hw_module_t *)module, mName.string(), CAMERA_DEVICE_API_VERSION_1_0, (hw_device_t **)&mDevice);
    if (rc != OK) {
        ACDK_LOGE("Could not open camera %s: %d", mName.string(), rc);
        return rc;
    }

    ACDK_LOGD("Opening camera %s - \n", mName.string());



    return rc;
}

void AcdkMhalReleaseDevice() {
    ACDK_LOGD(" AcdkMhalReleaseDevice \n");
    if (mDevice->ops->release)
    {
        mDevice->ops->release(mDevice);

    }

    //ACDK_LOGD("Destroying camera %s", mName.string());
    if(mDevice) {
    int rc = mDevice->common.close(&mDevice->common);
    if (rc != OK)
        ACDK_LOGD("Could not close camera %d", rc);
    }
    mDevice = NULL;
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

static camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
                                     void *user __attribute__((unused)))
{
    ACDK_LOGD("%s  __get_memory +", __FUNCTION__);


    CameraHeapMemory *mem;
    if (fd < 0)
        mem = new CameraHeapMemory(buf_size, num_bufs);
    else
        mem = new CameraHeapMemory(fd, buf_size, num_bufs);
    mem->incStrong(mem);

    ACDK_LOGD("%s  __get_memory -", __FUNCTION__);
    return &mem->handle;
}

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
    ACDK_LOGD("%s", __FUNCTION__);
}

static void handleMtkExtDataCompressedImage(const sp<IMemory>& dataPtr)
{
    MtkCamMsgExtDataHelper MtkExtDataHelper;
    if  ( ! MtkExtDataHelper.init(dataPtr) ) {
        ACDK_LOGE("[%s] MtkCamMsgExtDataHelper::init fail -  \r\n", __FUNCTION__);
        return;
    }
    //
    uint_t const*const pExtParam = (uint_t const*)MtkExtDataHelper.getExtParamBase();
    uint_t const      uShutIndex = pExtParam[0];
    //
    size_t const    imageSize   = MtkExtDataHelper.getExtParamSize()    - sizeof(uint_t) * 1;
    ssize_t const   imageOffset = MtkExtDataHelper.getExtParamOffset()  + sizeof(uint_t) * 1;
    // sp<MemoryBase> image = new MemoryBase(MtkExtDataHelper.getHeap(), imageOffset, imageSize);
    uint8_t* va = (((uint8_t*)MtkExtDataHelper.getHeap()->getBase())+imageOffset);
    ACDK_LOGD("%s  va=%p, size=%d, offset=%d", __FUNCTION__, va, imageSize,imageOffset);

    g_acdkMhalCapObserver.notify(ACDK_CB_CAPTURE,va, 0, (MUINT32)imageSize,0);
    //
    MtkExtDataHelper.uninit();
}

static void __data_cb(int32_t msg_type,
                      const camera_memory_t *data, unsigned int index,
                      camera_frame_metadata_t *metadata,
                      void *user)
{
    ACDK_LOGD("%s type %x", __FUNCTION__,msg_type);

    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory *>(data->handle));
    if (index >= mem->mNumBufs) {
        ACDK_LOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }

    ssize_t offset = 0;
    size_t size = 0;
    sp<IMemoryHeap> heap = mem->mBuffers[index]->getMemory(&offset, &size);
    uint8_t* va = (((uint8_t*)heap->getBase())+offset);
    ACDK_LOGD("%s  va=%p, size=%d, offset=%d", __FUNCTION__, va, size,offset);


    switch  (msg_type & ~CAMERA_MSG_PREVIEW_METADATA)
    {
        case CAMERA_MSG_PREVIEW_FRAME:
             //Callback to upper layer
             g_acdkMhalObserver.notify(ACDK_CB_PREVIEW,va, 0,(MUINT32)size,0);
            break;
        case CAMERA_MSG_POSTVIEW_FRAME:
             //Callback to upper layer
              ACDK_LOGD("CAMERA_MSG_POSTVIEW_FRAME=%d",CAMERA_MSG_POSTVIEW_FRAME);
             g_acdkMhalObserver.notify(ACDK_CB_PREVIEW,va, 0,(MUINT32)size,0);
            break;
        case MTK_CAMERA_MSG_EXT_DATA:{
                MtkCamMsgExtDataHelper MtkExtDataHelper;

                if  ( ! MtkExtDataHelper.init(mem->mBuffers[index]) ) {
                    ACDK_LOGE("[%s] MtkCamMsgExtDataHelper::init fail", __FUNCTION__);
                    return;
                }

                void*   const pvExtParam   = MtkExtDataHelper.getExtParamBase();
                size_t  const ExtParamSize = MtkExtDataHelper.getExtParamSize();
                switch  (MtkExtDataHelper.getExtMsgType())
                {
                    case MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE:
                        handleMtkExtDataCompressedImage(mem->mBuffers[index]);
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

/*******************************************************************************
* AcdkMhalEng
* brif : Constructor
*******************************************************************************/
AcdkMhalEng::AcdkMhalEng()
{
    mAcdkMhalState = ACDK_MHAL_NONE;



    mFocusDone = MFALSE;
    mFocusSucceed = MFALSE;

    mReadyForCap = MFALSE;
    mCaptureDone = MFALSE;

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));

    g_pAcdkMHalEngObj = this;

}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
MVOID AcdkMhalEng::acdkMhalSetState(acdkMhalState_e newState)
{
    Mutex::Autolock lock(mLock);

    ACDK_LOGD("Now(0x%04x), Next(0x%04x)", mAcdkMhalState, newState);

    if(newState == ACDK_MHAL_ERROR)
    {
        goto ACDK_MHAL_SET_STATE_EXIT;
    }

    switch(mAcdkMhalState)
    {
    case ACDK_MHAL_NONE:
        switch(newState)
        {
        case ACDK_MHAL_INIT:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_NONE");
            ACDK_LOGE("State error ACDK_MHAL_NONE");
            break;
        }
        break;
    case ACDK_MHAL_INIT:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_INIT");
            ACDK_LOGE("State error ACDK_MHAL_INIT");
            break;
        }
        break;
    case ACDK_MHAL_IDLE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_CAPTURE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_IDLE");
            ACDK_LOGE("State error ACDK_MHAL_IDLE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_PRE_CAPTURE:
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PREVIEW");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW");
            break;
        }
        break;
    case ACDK_MHAL_PRE_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PRE_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_PRE_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW_STOP:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_PREVIEW_STOP");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW_STOP");
            break;
        }
        break;
    case ACDK_MHAL_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_ERROR:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_ERROR");
            ACDK_LOGE("State error ACDK_MHAL_ERROR");
            break;
        }
        break;
    default:
        //ACDK_ASSERT(0, "Unknown state");
        ACDK_LOGE("Unknown state");
        break;
    }

ACDK_MHAL_SET_STATE_EXIT:

    mAcdkMhalState = newState;

    ACDK_LOGD("X, state(0x%04x)", mAcdkMhalState);
}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
acdkMhalState_e AcdkMhalEng::acdkMhalGetState()
{
    Mutex::Autolock _l(mLock);
    return mAcdkMhalState;
}

/*******************************************************************************
* acdkMhalReadyForCap
* brif : get status of mReadyForCap falg
*******************************************************************************/
MBOOL AcdkMhalEng::acdkMhalReadyForCap()
{
    return mReadyForCap;
}

/*******************************************************************************
* acdkMhalProcLoop
* brif : preview and capture thread executing function
*******************************************************************************/
static MVOID *acdkMhalProcLoop(MVOID *arg)
{
    ::prctl(PR_SET_NAME,"acdkMhalProcLoop",0,0,0);

    MINT32 const policy = SCHED_OTHER;
    MINT32 const priority = NICE_CAMERA_PASS1;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    ::sched_getparam(0, &sched_p);
    ACDK_LOGD("policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d),tid=%d", policy,
                                                                                            ::sched_getscheduler(0),
                                                                                            priority,
                                                                                            sched_p.sched_priority,
                                                                                            ::gettid());

     //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    acdkMhalState_e eState;

    eState = g_pAcdkMHalEngObj->acdkMhalGetState();
    while(eState != ACDK_MHAL_UNINIT)
    {
        ACDK_LOGD("Wait semMainHigh");
        ::sem_wait(&g_SemMainHigh); // wait here until someone use sem_post() to wake this semaphore up
        ACDK_LOGD("Got semMainHigh");

        eState = g_pAcdkMHalEngObj->acdkMhalGetState();

        switch(eState)
        {
        case ACDK_MHAL_PREVIEW:
            g_pAcdkMHalEngObj->acdkMhalPreviewProc();
            ::sem_post(&g_SemMainHighBack);
            break;
        case ACDK_MHAL_CAPTURE:
            g_pAcdkMHalEngObj->acdkMhalCaptureProc();
            break;
        case ACDK_MHAL_UNINIT:
            break;
        default:
            ACDK_LOGD("T.B.D");
            break;
        }

        eState = g_pAcdkMHalEngObj->acdkMhalGetState();
    }


    ::sem_post(&g_SemMainHighEnd);
    ACDK_LOGD("-");

    return NULL;
}

/*******************************************************************************
* destroyInstanc
* brif : destroy AcdkMhalEng object
*******************************************************************************/
void AcdkMhalEng::destroyInstance()
{
    g_pAcdkMHalEngObj = NULL;
    delete this;
}

/*******************************************************************************
* acdkMhalInit
* brif : initialize camera
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalInit()
{
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32  err = ACDK_RETURN_NO_ERROR;

    //====== Acdk Mhal State Setting ======

    mAcdkMhalState = ACDK_MHAL_INIT;    //set state to Init state

    //====== Get Debug Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.acdk.debug", value, "0");
    g_acdkMhalEngDebug = atoi(value);

    ACDK_LOGD("g_acdkMhalEngDebug(%d)",g_acdkMhalEngDebug);

    //====== Init Thread for Preview and Capture ======

    // Init semphore
    ::sem_init(&g_SemMainHigh, 0, 0);
    ::sem_init(&g_SemMainHighBack, 0, 0);
    ::sem_init(&g_SemMainHighEnd, 0, 0);

    // Create main thread for preview and capture
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_PASS1};
    int min = sched_get_priority_min( SCHED_RR );
    int max = sched_get_priority_max( SCHED_RR );
    ACDK_LOGD("(min,max)=(%d,%d) PRIO_RT_CAMERA_PREVIEW(%d) RTPM_PRIO_CAMERA_TOPBASE(%d)", min, max, PRIO_RT_CAMERA_PREVIEW, RTPM_PRIO_CAMERA_TOPBASE);
      
    int result = pthread_create(&g_threadMainHigh, &attr, acdkMhalProcLoop, NULL);
    ACDK_LOGD("result of pthread_create : %d", result);
    //=== Set State to Idle State ======

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalUninit
* brif : Uninitialize camera
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalUninit()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e eState;

    //====== Uninitialization ======

    // Check it is in the idle mode
    // If it is not, has to wait until idle
    eState = acdkMhalGetState();

    ACDK_LOGD("eState(0x%x)",eState);

    if(eState != ACDK_MHAL_NONE)
    {
        if((eState != ACDK_MHAL_IDLE) && (eState != ACDK_MHAL_ERROR))
        {
            ACDK_LOGD("Camera is not in the idle state");
            if(eState & ACDK_MHAL_PREVIEW_MASK)
            {
                err = acdkMhalPreviewStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalPreviewStop fail(0x%x)",err);
                }
            }
            else if(eState & ACDK_MHAL_CAPTURE_MASK)
            {
                err = acdkMhalCaptureStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalCaptureStop fail(0x%x)",err);
                }
            }

            // Polling until idle
            while(eState != ACDK_MHAL_IDLE)
            {
                // Wait 10 ms per time
                usleep(10000);
                eState = acdkMhalGetState();
            }
            ACDK_LOGD("Now camera is in the idle state");
        }

        //====== Set State to Uninit State ======

        acdkMhalSetState(ACDK_MHAL_UNINIT);

        //====== Semephore Process ======

        //post sem
        ACDK_LOGD("post g_SemMainHigh");
        ::sem_post(&g_SemMainHigh);

        //wait sem
        ACDK_LOGD("wait for g_SemMainHighEnd");
        ::sem_wait(&g_SemMainHighEnd);
        ACDK_LOGD("got g_SemMainHighEnd");
    }
    else
    {
        acdkMhalSetState(ACDK_MHAL_UNINIT);
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalCBHandle
* brif : callback handler
*******************************************************************************/
MVOID AcdkMhalEng::acdkMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2, MUINT32 const a_dataSize)
{
    ACDK_LOGD_DYN(g_acdkMhalEngDebug,"+");

    if(!g_acdkMhalObserver)
    {
        ACDK_LOGE("callback is NULL");
    }

    // Callback to upper layer
    g_acdkMhalObserver.notify(a_type, a_addr1, a_addr2, a_dataSize);
}

/*******************************************************************************
* acdkMhal3ASetParm
* brif : set 3A parameter
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhal3ASetParam(MINT32 devID, MUINT8 IsFactory)
{
    ACDK_LOGD("devID(%d)",devID);

    return ACDK_RETURN_NO_ERROR;
}
#define PASS2_FULLG

/*******************************************************************************
* acdkMhalPreviewStart
* brif : Start preview
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalPreviewStart(MVOID *a_pBuffIn)
{
    ACDK_LOGD("+");
    char szUser[32] = "acdk_preview";

    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "[acdkMhalPreviewStart] Camera State is not IDLE");

    //====== Local Variable Declaration ======
    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkMhalPrvParam, a_pBuffIn, sizeof(acdkMhalPrvParam_t));

    ACDK_LOGD("%s  test Cam Device", __FUNCTION__);

    AcdkMhalLoadModule();

    char const* sym = "MtkCam_setProperty";
    void* pfn = ::dlsym(mModule->common.dso, sym);
    if ( ! pfn ) {
       ACDK_LOGE("Cannot find symbol: %s", sym);
      //  return INVALID_OPERATION;
    }
    String8 key = String8("client.appmode");
    String8 value = String8("MtkEng");
    reinterpret_cast<status_t(*)(String8 const&, String8 const&)>(pfn)(key, value);

    if(mAcdkMhalPrvParam.sensorIndex == 0)
        AcdkMhalOpenDevice(mModule, "0");
    else
        AcdkMhalOpenDevice(mModule, "1");

    ACDK_LOGD("+ set_callbacks \n");


    if (mDevice->ops->set_callbacks) {
        mDevice->ops->set_callbacks(mDevice,
                               __notify_cb,
                               __data_cb,
                               __data_cb_timestamp,
                               __get_memory,
                               &mDevice);
    }

    ACDK_LOGD("- set_callbacks");


    //if (mDevice->ops->enable_msg_type)
    //    mDevice->ops->enable_msg_type(mDevice, CAMERA_MSG_PREVIEW_FRAME);

    if (mDevice->ops->enable_msg_type)
        mDevice->ops->enable_msg_type(mDevice, CAMERA_MSG_PREVIEW_FRAME |CAMERA_MSG_COMPRESSED_IMAGE | CAMERA_MSG_POSTVIEW_FRAME);


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

    String8 sPrvSize = String8("");
    if( 1 == mAcdkMhalPrvParam.frmParam.orientation % 2 )
    {
        //odd means 90 or 270 rotation
        sPrvSize = String8::format("%dx%d", mAcdkMhalPrvParam.frmParam.h, mAcdkMhalPrvParam.frmParam.w);
    }
    else
    {
        //even means 0 or 180 rotation
        sPrvSize = String8::format("%dx%d", mAcdkMhalPrvParam.frmParam.w, mAcdkMhalPrvParam.frmParam.h);
    }

    String8 sPrvList = map.valueFor( String8("preview-size-values") );
    ACDK_LOGD("Prv Size: %s; PrvList: %s", sPrvSize.string(), sPrvList.string() );
    if( NULL == strstr( sPrvList, sPrvSize ) )
    {
        ACDK_LOGE("Prv Size : %s is not supported. Try 640x480", sPrvSize.string());
        sPrvSize = String8("640x480");
        if( 1 == mAcdkMhalPrvParam.frmParam.orientation % 2 )
        {
            mAcdkMhalPrvParam.frmParam.h = 640;
            mAcdkMhalPrvParam.frmParam.w = 480;
        }
        else
        {
            mAcdkMhalPrvParam.frmParam.h = 480;
            mAcdkMhalPrvParam.frmParam.w = 640;
        }

        if( NULL == strstr( sPrvList, sPrvSize ) )
        {
            ACDK_LOGE("640x480 is not supported.");
            err = ACDK_RETURN_INVALID_PARA;
            goto acdkMhalPreviewStart_Exit;
        }
    }

    set(map, "preview-size", sPrvSize);

    /*if(mAcdkMhalPrvParam.frmParam.orientation == 1)
    {
        if(mAcdkMhalPrvParam.frmParam.h > 1280)
            set(map, "preview-size", "1440x1080");
        else if(mAcdkMhalPrvParam.frmParam.h > 800)
            set(map, "preview-size", "1280x720");
        else
            set(map, "preview-size", "800x600");
    }
    else
    {
        if(mAcdkMhalPrvParam.frmParam.w > 1280)
            set(map, "preview-size", "1440x1080");
        else if(mAcdkMhalPrvParam.frmParam.w > 800)
            set(map, "preview-size", "1280x720");
        else
            set(map, "preview-size", "800x600");
    }*/


    set(map, "preview-format", "yuv420p");

    switch( mAcdkMhalPrvParam.mHDR_EN )
    {
        case 1: //ivhdr: only video mode supports
            set(map, "video-hdr", "on");
            set(map, "rawsave-mode", "4");
            break;
        case 2: //mvhdr: only preview/capture modes support
            set(map, "video-hdr", "on");
            set(map, "rawsave-mode", "1");
            break;
        default:
            set(map, "video-hdr", "off");
            ACDK_LOGD("PreviewSensorScenario=%d", mAcdkMhalPrvParam.PreviewSensorScenario);
            switch(mAcdkMhalPrvParam.PreviewSensorScenario)
            {
                case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                    set(map, "rawsave-mode", "1");
                break;
                case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                    set(map, "rawsave-mode", "2");
                break;
                case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                    set(map, "rawsave-mode", "4");
                break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                    set(map, "rawsave-mode", "5");
                break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                    set(map, "rawsave-mode", "6");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM1:
                    set(map, "rawsave-mode", "7");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM2:
                    set(map, "rawsave-mode", "8");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM3:
                    set(map, "rawsave-mode", "9");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM4:
                    set(map, "rawsave-mode", "10");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM5:
                    set(map, "rawsave-mode", "11");
                break;
                default:
                    set(map, "rawsave-mode", "1");
                break;
            }
    }

    mDevice->ops->set_parameters(mDevice, flatten(map).string());


    //====== Setting Callback ======
    g_acdkMhalObserver = mAcdkMhalPrvParam.acdkMainObserver;

    //====== Set State to Preview State ======

    acdkMhalSetState(ACDK_MHAL_PREVIEW);

    //====== Post Sem ======

    ::sem_post(&g_SemMainHigh);

acdkMhalPreviewStart_Exit:


    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalPreviewStop
* brif : stop preview
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalPreviewStop()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e state = acdkMhalGetState();

    //====== Check State ======

    //check AcdkMhal state
    ACDK_LOGD("state(%d)", state);
    if(state == ACDK_MHAL_IDLE)
    {
        ACDK_LOGD("is in IDLE state");
        return err;
    }
    else if(state != ACDK_MHAL_PREVIEW_STOP)
    {
        if(state & ACDK_MHAL_PREVIEW_MASK)
        {
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_PRE_CAPTURE)
        {
            // In preCapture, has to wait 3A ready flag
            ACDK_LOGD("it is ACDK_MHAL_PRE_CAPTURE state");
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_CAPTURE)
        {
            // It is in capture flow now, preview has been already stopped
            ACDK_LOGD("it is ACDK_MHAL_CAPTURE state");
            state = acdkMhalGetState();
            while(state == ACDK_MHAL_CAPTURE)
            {
                usleep(20);
                state = acdkMhalGetState();
            }

            acdkMhalSetState(ACDK_MHAL_IDLE);
        }
        else
        {
            // Unknown
            ACDK_LOGE("un know state(%d)", state);
        }
    }

    //====== Wait Semaphore ======

    ACDK_LOGD("Wait g_SemMainHighBack");
    ::sem_wait(&g_SemMainHighBack);
    ACDK_LOGD("Got g_SemMainHighBack");

    //====== Stop Preview ======

    //====== Initialize Member Variable =====

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));

    //====== Set Acdk Mhal State ======

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalPreCapture
* brif : change ACDK mhal state to preCapture
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalPreCapture()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Change State ======

    acdkMhalSetState(ACDK_MHAL_PRE_CAPTURE);
    mReadyForCap = MTRUE;

    ACDK_LOGD("-");

    return err;
}


/*******************************************************************************
* acdkMhalCaptureStart
* brif : init capture
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalCaptureStart(MVOID *a_pBuffIn)
{
    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 CaptureMode;
    MUINT32 MFLL_EN;
    MUINT32 HDR_EN;
    eACDK_CAP_FORMAT eImgType;
    char szName[256];

    ACDK_LOGD("+");
    memcpy(&mAcdkMhalCapParam, a_pBuffIn, sizeof(acdkMhalCapParam_t));
    CaptureMode = mAcdkMhalCapParam.CaptureMode;
    eImgType = (eACDK_CAP_FORMAT)mAcdkMhalCapParam.mCapType;
    MFLL_EN = mAcdkMhalCapParam.mMFLL_EN;
    HDR_EN = mAcdkMhalCapParam.mHDR_EN;
    EMultiNR_Mode eMultiNR = (EMultiNR_Mode)mAcdkMhalCapParam.mEMultiNR;
    sprintf(szName, "%04dx%04d ", mAcdkMhalCapParam.mCapWidth, mAcdkMhalCapParam.mCapHeight);

    ACDK_LOGD("Picture Size:%s\n", szName);
    ACDK_LOGD("CaptureMode(%d), eImgType(%d),MFLL_En(%d), eMultiNR(%d)", CaptureMode, eImgType, MFLL_EN, eMultiNR);

    // restore flash mode
    MINT32 flashMode = FlashMgr::getInstance().getFlashMode(mAcdkMhalCapParam.sensorID);

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

    // PURE_RAW8_TYPE =0x04, PURE_RAW10_TYPE = 0x08
    // PROCESSED_RAW8_TYPE = 0x10, PROCESSED_RAW10_TYPE = 0x20
    if(eImgType & 0x3C)
    {
        //   rawsave-mode :  1,preview 2,Capture 3,jpeg only 4,video  5,slim video1 6,slim video2
        //                           7,custom1 8,custom2 9,custom3 10,custom4 11,custom5
        //   isp-mode   : 0: process raw, 1:pure raw
        set(map, "camera-mode", "0");
        set(map, "afeng_raw_dump_flag", "1");
        set(map, "rawfname", "/data/");
        switch(eImgType)
        {
            case PURE_RAW8_TYPE:
                set(map, "isp-mode", "1");
                set(map, "rawsave-mode", "2");
                break;
            case PURE_RAW10_TYPE:
                set(map, "isp-mode", "1");
                set(map, "rawsave-mode", "2");
                break;
            case PROCESSED_RAW8_TYPE:
                set(map, "isp-mode", "0");
                set(map, "rawsave-mode", "2");
                break;
            case PROCESSED_RAW10_TYPE:
                set(map, "isp-mode", "0");
                set(map, "rawsave-mode", "2");
                break;
            default:
                break;

        }

    }

    switch(CaptureMode)
    {
         case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            set(map, "rawsave-mode", "1");
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            set(map, "rawsave-mode", "2");
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            set(map, "rawsave-mode", "4");
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            set(map, "rawsave-mode", "5");
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            set(map, "rawsave-mode", "6");
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            set(map, "rawsave-mode", "7");
            break;
    case SENSOR_SCENARIO_ID_CUSTOM2:
        set(map, "rawsave-mode", "8");
        break;
    case SENSOR_SCENARIO_ID_CUSTOM3:
            set(map, "rawsave-mode", "9");
            break;
    case SENSOR_SCENARIO_ID_CUSTOM4:
            set(map, "rawsave-mode", "10");
            break;
    case SENSOR_SCENARIO_ID_CUSTOM5:
            set(map, "rawsave-mode", "11");
            break;
        default:
            set(map, "rawsave-mode", "1");
            break;

    }

    if(MFLL_EN == 1)
    {
        set(map, "mfb", "mfll");
        set(map, "isp-mode", "0");
    }
    else
        set(map, "mfb", "off");


    if(HDR_EN == 1 || HDR_EN == 2)
        set(map, "video-hdr", "on");
    else
        set(map, "video-hdr", "off");


    set(map, "mnr-e", "1");//enable manual multi-NR

    /* manual multi-NR type:
    0: Disable
    1: HWNR
    2: SWNR
    */
    if(eMultiNR == EMultiNR_Off) set(map, "mnr-t", "0");
    else if (eMultiNR == EMultiNR_HW) set(map, "mnr-t", "1");
    else if (eMultiNR == EMultiNR_SW) set(map, "mnr-t", "2");
    else
    {
        ACDK_LOGD("Error: eMultiNR = %d\n", eMultiNR);
        set(map, "mnr-t", "0");
    }

    mDevice->ops->set_parameters(mDevice, flatten(map).string());

    ACDK_LOGD("setFlashMode = %d", flashMode);
    FlashMgr::getInstance().setFlashMode(mAcdkMhalCapParam.sensorID, flashMode);

    //====== Setting Callback ======
    g_acdkMhalCapObserver = mAcdkMhalCapParam.acdkMainObserver;

    mDevice->ops->take_picture(mDevice);

    ACDK_LOGD("-");
    return err;
}


/*******************************************************************************
* acdkMhalCaptureStop
* brif : stop capture
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalCaptureStop()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    while(mCaptureDone == MFALSE)
    {
        usleep(200);
    }

    mCaptureDone = MFALSE;

    ACDK_LOGD("-");

    return err;

}


/*******************************************************************************
* acdkMhalPreviewProc
* brif : handle flow control of preview
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalPreviewProc()
{
    ACDK_LOGD_DYN(g_acdkMhalEngDebug,"+");

    ACDK_LOGD("Preview start");
    mDevice->ops->start_preview(mDevice);

    //pass1 continuous
    while((ACDK_MHAL_PREVIEW_STOP != acdkMhalGetState()))
    {
        usleep(500);
    }

    mDevice->ops->stop_preview(mDevice);

    AcdkMhalReleaseDevice();

    ACDK_LOGD_DYN(g_acdkMhalEngDebug,"-");

    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkMhalCaptureProc
* brif : handle flow control of capture
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalCaptureProc()
{
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalEng::acdkMhalGetShutTime()
{
return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID AcdkMhalEng::acdkMhalSetShutTime(MUINT32 a_time)
{
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalEng::acdkMhalGetAFInfo()
{
    MUINT32 u32AFInfo;

    ACDK_LOGD("acdkMhalGetAFInfo");
    u32AFInfo = g_pAcdkMHalEngObj->mFocusSucceed;

    return u32AFInfo;

}
/*******************************************************************************
* doNotifyCb
*******************************************************************************/
void AcdkMhalEng::doNotifyCb(int32_t _msgType,
                                 int32_t _ext1,
                                 int32_t _ext2,
                                 int32_t _ext3)
{
    ACDK_LOGD("_msgType(%d),_ext1(%d)",_msgType,_ext1);

    ACDK_LOGD("-");
}

/*******************************************************************************
* doDataCb
*******************************************************************************/
void AcdkMhalEng::doDataCb(int32_t _msgType,
                               void *_data,
                               uint32_t _size)
{

}



