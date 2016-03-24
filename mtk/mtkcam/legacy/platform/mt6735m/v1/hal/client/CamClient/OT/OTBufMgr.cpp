#define LOG_TAG "MtkCam/CamClient/OTClient"
//
#include "../inc/CamUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include <stdlib.h>
#if (MTKCAM_BASIC_PACKAGE != 1)
#include <linux/cache.h>
#endif
//
#include "OTBufMgr.h"
//
#include <cutils/atomic.h>
//
/******************************************************************************
*
*******************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")

/******************************************************************************
*
*******************************************************************************/
void
OTBuffer::
createBuffer()
{
    FUNCTION_IN;
    //
    //mbufSize = 640*900*2; //Video WallPaper: Height > Width
    mbufSize = (mbufSize + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    mbufInfo.size = mbufSize;

    mpIMemDrv = IMemDrv::createInstance();
    if ( ! mpIMemDrv || ! mpIMemDrv->init() ) {
        MY_LOGE("mpIMemDrv->init() error");
    }

    if ( ! mpIMemDrv || mpIMemDrv->allocVirtBuf(&mbufInfo) < 0) {
        MY_LOGE("mpIMemDrv->allocVirtBuf() error");
    }

    //
    MY_LOGW_IF( mbufInfo.size & (L1_CACHE_BYTES-1), "bufSize(%d) not aligned!", mbufInfo.size);
    MY_LOGW_IF( mbufInfo.virtAddr & (L1_CACHE_BYTES-1), "bufAddr(%d) not aligned!", mbufInfo.virtAddr);

    MY_LOGD("OTClient: Virtual Addr: 0x%x, Phyical Addr: 0x%x, bufSize:%d", mbufInfo.virtAddr, mbufInfo.phyAddr, mbufInfo.size);
    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
void
OTBuffer::
destroyBuffer()
{
    FUNCTION_IN;
    //
    if (0 == mbufInfo.virtAddr)
    {
        MY_LOGD("[OT Buffer doesn't exist]");
        return;
    }

    if ( ! mpIMemDrv || mpIMemDrv->freeVirtBuf(&mbufInfo) < 0) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }

    if ( ! mpIMemDrv || ! mpIMemDrv->uninit() ) {
        MY_LOGE("m_pIMemDrv->uninit error");
    }
    //
    FUNCTION_OUT;
}

