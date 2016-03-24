#define LOG_TAG "MtkCam/ExtImgProcHw"
//
#include <utils/threads.h>
//
using namespace android;
//
#include <mtkcam/Log.h>
#include <mtkcam/BuiltinTypes.h>
//
#include <mtkcam/v1/ExtImgProc/IExtImgProc.h>
#include <mtkcam/v1/ExtImgProcHw/ExtImgProcHw.h>
#include <ExtImgProcHwImp.h>
//-----------------------------------------------------------------------------
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s]"             fmt, ::gettid(), __FUNCTION__,           ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s]WRN(%5d):"    fmt, ::gettid(), __FUNCTION__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s]ERR(%5d):"    fmt, ::gettid(), __FUNCTION__, __LINE__, ##arg)
//
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_NAME               MY_LOGD("");
#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")
//-----------------------------------------------------------------------------
ExtImgProcHw*
ExtImgProcHw::
createInstance(void)
{
    return ExtImgProcHwImp::getInstance();
}
//-----------------------------------------------------------------------------
ExtImgProcHw*
ExtImgProcHwImp::
getInstance(void)
{
    static ExtImgProcHwImp Singleton;
    return &Singleton;
}
//-----------------------------------------------------------------------------
void
ExtImgProcHwImp::
destroyInstance(void)
{
}
//-----------------------------------------------------------------------------
ExtImgProcHwImp::
ExtImgProcHwImp()
{
    FUNCTION_NAME;
    //Set which img buf you want to process.
    //For example: mImgMask = BufType_ISP_P2_TwoRun_In;
    mImgMask = 0;
}
//-----------------------------------------------------------------------------
ExtImgProcHwImp::
~ExtImgProcHwImp()
{
    FUNCTION_NAME;
}
//-----------------------------------------------------------------------------
MBOOL
ExtImgProcHwImp::
init(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser == 0)
    {
        MY_LOGD("First user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        android_atomic_inc(&mUser);
        goto EXIT;
    }
    //Add init code
    //[BEGIN]

    //[END]
    //
    android_atomic_inc(&mUser);
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL
ExtImgProcHwImp::
uninit(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user(%d)",mUser);
        goto EXIT;
    }
    //
    android_atomic_dec(&mUser);
    //
    if(mUser == 0)
    {
        MY_LOGD("Last user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        goto EXIT;
    }
    //Add uninit code
    //[BEGIN]

    //[END]
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MUINT32
ExtImgProcHwImp::
getImgMask()
{
    Mutex::Autolock lock(mLock);
    //
    return mImgMask;
}
//-----------------------------------------------------------------------------
MBOOL
ExtImgProcHwImp::
doImgProc(
    MUINT32             bufType,
    IImageBuffer* const pImgBuf)
{
    MBOOL Result = MTRUE;
    //
    {
        Mutex::Autolock lock(mLock);
        //
        if(mUser <= 0)
        {
            MY_LOGW("No user");
            Result = MFALSE;
            goto EXIT;
        }
    }
    //Doubel check to avoid user send buffer here but they don't need extra image process.
    if((mImgMask & bufType) != bufType)
    {
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(pImgBuf->getPlaneCount() == 1)
    {
        MY_LOGD("Img:Type(0x%08X),FMT(%d),ImgSize(%dx%d),PC(%d),Stride(%d),VA(0x%08X),BufSuze(%d)",
                bufType,
                pImgBuf->getImgFormat(),
                pImgBuf->getImgSize().w,
                pImgBuf->getImgSize().h,
                pImgBuf->getPlaneCount(),
                pImgBuf->getBufStridesInBytes(0),
                pImgBuf->getBufVA(0),
                pImgBuf->getBufSizeInBytes(0));
    }
    else
    if(pImgBuf->getPlaneCount() == 2)
    {
        MY_LOGD("Img:Type(0x%08X),FMT(%d),ImgSize(%dx%d),PC(%d),Stride(%d/%d),VA(0x%08X/0x%08X),BufSuze(%d/%d)",
                bufType,
                pImgBuf->getImgFormat(),
                pImgBuf->getImgSize().w,
                pImgBuf->getImgSize().h,
                pImgBuf->getBufStridesInBytes(0),
                pImgBuf->getBufStridesInBytes(1),
                pImgBuf->getBufVA(0),
                pImgBuf->getBufVA(1),
                pImgBuf->getBufSizeInBytes(0),
                pImgBuf->getBufSizeInBytes(1));
    }
    else
    if(pImgBuf->getPlaneCount() == 3)
    {
        MY_LOGD("Img:Type(0x%08X),FMT(%d),ImgSize(%dx%d),PC(%d),Stride(%d/%d/%d),VA(0x%08X/0x%08X/0x%08X),BufSuze(%d/%d/%d)",
                bufType,
                pImgBuf->getImgFormat(),
                pImgBuf->getImgSize().w,
                pImgBuf->getImgSize().h,
                pImgBuf->getBufStridesInBytes(0),
                pImgBuf->getBufStridesInBytes(1),
                pImgBuf->getBufStridesInBytes(2),
                pImgBuf->getBufVA(0),
                pImgBuf->getBufVA(1),
                pImgBuf->getBufVA(2),
                pImgBuf->getBufSizeInBytes(0),
                pImgBuf->getBufSizeInBytes(1),
                pImgBuf->getBufSizeInBytes(2));
    }
    //
    //Add image process code
    switch(bufType)
    {
        case BufType_ISP_P2_TwoRun_In:
        {
            //[BEGIN]

            //[END]
            break;
        }
        default:
        {
            MY_LOGW("Unknown bufType(0x%08X)",bufType);
            break;
        }
    }
    //
    EXIT:
    return Result;
}


