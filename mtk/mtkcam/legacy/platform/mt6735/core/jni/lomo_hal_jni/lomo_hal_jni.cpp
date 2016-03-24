#define LOG_TAG "lomohj_v0912_00_L"  //eCACHECTRL_INVALID after allocation pHeap->createImageBuffer

#define MTK_LOG_ENABLE 1
#include <cutils/atomic.h>
#include <cutils/properties.h>  // For property_get().
#include "lomo_hal_jni_imp.h"


#include <stdio.h>
#include <stdarg.h>
#include <utils/StrongPointer.h>
using namespace android;
//

#include <mtkcam/camnode/ICamBufHandler.h>   //for AllocInfo
using namespace NSCamNode;  //for AllocInfo

#include <IFeatureStream.h>
#include <IPortEnum.h>
using namespace NSCam::NSIoPipe::NSPostProc;

#include <sys/prctl.h>

#include <mtkcam/utils/ImageBufferHeap.h> //for PortBufInfo_v1
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;


#include "tuning_mgr.h" //SL#include <mtkcam/featureio/tuning_mgr.h> //SL

//#include "camera_custom_lomo_param.h"

#include "camera_custom_lomo_param_jni.h"
/*************************************************************************************
* for BlitSteam
*************************************************************************************/
#include "DpBlitStream.h"

/*************************************************************************************
* Log Utility
*************************************************************************************/

#undef   DBG_LOG_TAG    // Decide a Log TAG for current file.
#define  DBG_LOG_TAG    "{LomoHalJniImp} "
//
#include "drv_log.h"    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#include "aaa_types.h"
#include "aaa_log.h"
#include "camera_custom_nvram.h"
#include "isp_tuning.h"
#include "isp_tuning_cam_info.h"
#include "isp_tuning_idx.h"
#include "isp_tuning_custom.h"
#include "isp_tuning_custom_instance.h"

using namespace NSIspTuning;

#include <utils/Trace.h>
#define LOMO_CONFIG_FOR_SYSTRACE 1
#if LOMO_CONFIG_FOR_SYSTRACE

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#endif

#define ATRACE_TAG ATRACE_TAG_CAMERA //ATRACE_TAG_ALWAYS
#define LOMO_TRACE_CALL()                     ATRACE_CALL()
//#define DP_TRACE_BEGIN(name)                Tracer::traceBegin(ATRACE_TAG, name)
//#define DP_TRACE_END()                      Tracer::traceEnd(ATRACE_TAG)
#else
#define LOMO_TRACE_CALL()
//#define DP_TRACE_BEGIN(name)
//#define DP_TRACE_END()
#endif  // LOMO_CONFIG_FOR_SYSTRACE

//#define LOMO_FLUSH_AP 1
#define LOMO_FLUSH_TH 1

DECLARE_DBG_LOG_VARIABLE(lomohalJ);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST

//ALOGD(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...

#define MY_LOG(fmt, arg...) ALOGD(fmt, ##arg)

#define LOG_VRB(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (lomohalJ_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define USE_IMAGE_BUFFER_HEAP 1
#define USE_FEATURE_STREAM_TO_DST 1
#define TUNING_MGR_JNI    1
#define LOMO_EFFECT_DST_BUFF_NUM 48
#define LOMO_EFFECT_MAGICNUM_SHIFT 10000
#define LOMO_EFFECT_TIME_OUT 5000000000 //5 sec
//#define LOMO_EFFECT_RAWDUMP_SHIFT 100


#define PIXEL2BYTE(PLANE, PIXEL, FORMAT)                                        \
    (PIXEL * NSCam::Utils::Format::queryPlaneBitsPerPixel(FORMAT, PLANE)) >> 3         \


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
   static IImageBufferAllocator* allocator;// = IImageBufferAllocator::getInstance(); //    //IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
   static  DpBlitStream   *pStream = NULL;//new DpBlitStream();
   sp<IImageBuffer> pBufSrc ;
   sp<IImageBuffer> pBufDst ;
   sp<IImageBuffer> pBufDstHeap[LOMO_EFFECT_DST_BUFF_NUM] ;
   static MUINT8* pBufDstHeapBuffIdx[LOMO_EFFECT_DST_BUFF_NUM]={0};
   volatile MINT32  mInitCountLomoHalJni=0;
   //        mutable Mutex mLockLomoHalJni;
   static IspTuningCustom*  m_pIspTuningCustom =NULL;
   static IFeatureStream* mpIStream=NULL;  //For Pass2
   static MBOOL brawdumpEnable = MFALSE;
   static MUINT32 lu32BuffeffectIdx = 0;
   static MBOOL bAddrDumpEnable = MFALSE;
/**************************************************************************
*
**************************************************************************/
LomoHalJniImp::LomoHalJniImp()
            :LomoHalJni()
            , gu32pvCBWidth(0)
            , gu32pvCBHeight(0)
            , gu32pvCBformat(0)
            , gu32ppSrcImgWidth(0)
            , gu32ppSrcImgHeight(0)
{
    LOG_INF("+");
    gu32pvCBVA[0]=NULL;
    gu32pvCBMVA[0]=NULL;
    gu32ppSrcImgVA[0]=NULL;
    gu32ppSrcImgMVA[0]=NULL;

    allocator = IImageBufferAllocator::getInstance();
    if(m_pIspTuningCustom==NULL)
    {
       ESensorDev_T m_sesnorDev = ESensorDev_Main;
       MY_LOG("IspTuningCustom::createInstance(m_sesnorDev, 0x0306)");
        m_pIspTuningCustom = IspTuningCustom::createInstance(m_sesnorDev, 0x0306);
    }
    if(pStream == NULL)
    {
       MY_LOG("new DpBlitStream()");
        pStream = new DpBlitStream();
    }

    mpIStream=NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance("Lomo_Hal_Jni",EFeatureStreamTag_Stream,0xffff);  //0xffff means pure p2
    mpIStream->init();
    MY_LOG("LomoHalJniImp - mpIStream->createInstance()");

    TuningMgr::getInstance().init();
    MY_LOG("LomoHalJniImp - TuningMgr::getInstance().init()");
    mState = LOMO_STATE_NONE;

    LOG_INF("-");
}

/**************************************************************************
*
**************************************************************************/
LomoHalJniImp::~LomoHalJniImp()
{
    LOG_INF("+");

//    Mutex::Autolock lock(mLockLomoHalJni);
    if(pStream != NULL)
    {
        delete pStream;
        MY_LOG("delete pStream");
        pStream = NULL;
    }

    TuningMgr::getInstance().uninit();
    MY_LOG("~LomoHalJniImp - TuningMgr::getInstance().uninit()");
    mpIStream->uninit();
    mpIStream->destroyInstance("Lomo_Hal_Jni");
    MY_LOG("~LomoHalJniImp - mpIStream->destroyInstance()");
    LOG_INF("-");
}

/**************************************************************************
*
**************************************************************************/
LomoHalJni *LomoHalJni::createInstance( void )
{
    LOG_INF("+");

    DBG_LOG_CONFIG(featureio, lomohalJ);
    static LomoHalJniImp singleton;

    LOG_INF("-");
    return &singleton;
}

/**************************************************************************
*
**************************************************************************/
MVOID LomoHalJni::destroyInstance( void )
{

}

/**************************************************************************
*
**************************************************************************/
MINT32 LomoHalJniImp::init()
{
    LOG_INF("+,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);
//    Mutex::Autolock lock(mLockLomoHalJni);
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

    char valuerawdump[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("lomo.jni.rawdump", valuerawdump, "0");
    brawdumpEnable = atoi(valuerawdump);


    char valueIdx[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("lomo.jni.effectidx", valueIdx, "0");
    lu32BuffeffectIdx = atoi(valueIdx);


    char valueAddrDump[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("lomo.jni.addrdump", valueAddrDump, "0");
    /*MBOOL */bAddrDumpEnable = atoi(valueAddrDump);


    android_atomic_inc(&mInitCountLomoHalJni);
    //====== Set State ======

    SetLomoState(LOMO_STATE_ALIVE);
    //====== Create Thread  ======

    // init semphore
    ::sem_init(&mDequeSem, 0, 0);
    ::sem_init(&mDequeDoneSem, 0, 0);
    ::sem_init(&mDequeSemEnd, 0, 0);

    // create thread
    pthread_create(&mLomoDequeThread, NULL, LomoDequeThreadLoop, this);

//    android_atomic_inc(&mUsers);


INIT_EXIT:
    LOG_INF("-,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);

    return err;
}

/**************************************************************************
*
**************************************************************************/
MINT32 LomoHalJniImp::uninit()
{

    LOG_INF("+,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);
    //
//    Mutex::Autolock lock(mLockLomoHalJni);
    //
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    //
    android_atomic_dec(&mInitCountLomoHalJni);
    //
    if(mInitCountLomoHalJni > 0) {
        err = LOMOHALJNI_STILL_USERS;
        goto UNINIT_EXIT;
    }
    //====== Set State ======
    SetLomoState(LOMO_STATE_UNINIT);
    //====== Destroy Thread ======
    MY_LOG("post mDequeSem");
    if(1)
    {
        int mu32Val;
        ::sem_getvalue(&mDequeSem, &mu32Val);
        MY_LOG("mDequeSem %d",mu32Val);
    }
    ::sem_post(&mDequeSem);
    //wait sem
    MY_LOG("wait mDequeSemEnd");
    ::sem_wait(&mDequeSemEnd);
    MY_LOG("got mDequeSemEnd");

    pthread_join(mLomoDequeThread, NULL);
    MY_LOG("close thread :  mLomoDequeThread");

UNINIT_EXIT:
    LOG_INF("-,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);
    return err;
}


MINT32 LomoHalJniImp::AllocLomoSrcImage(MUINT32 pvCBWidth, \
                                                                     MUINT32 pvCBHeight, \
                                                                     MUINT32 pvCBformat, \
                                                                     MUINT32 ppSrcImgWidth, \
                                                                     MUINT32 ppSrcImgHeight)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    MUINT32 InPA, InVA;
    MUINT32 plane=1;
    MUINT32 bufStridesInBytes[3]={0};
    MUINT32 imgFormat;
    MUINT32 i ;
    MUINT32 count ;
     MSize bufStridesInPixels[3] = {0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

    gu32pvCBWidth = pvCBWidth;
    gu32pvCBHeight = pvCBHeight;
    gu32pvCBformat = pvCBformat;
    gu32ppSrcImgWidth = ppSrcImgWidth;
    gu32ppSrcImgHeight = ppSrcImgHeight;
    gu32ppDstImgWidth = gu32ppSrcImgWidth;
    gu32ppDstImgHeight = gu32ppSrcImgHeight;

    MY_LOG("gu32pvCBWidth = %d",gu32pvCBWidth);
    MY_LOG("gu32pvCBHeight = %d",gu32pvCBHeight);
    MY_LOG("gu32ppDstImgWidth = %d",gu32ppDstImgWidth);
    MY_LOG("gu32ppDstImgHeight = %d\n",gu32ppDstImgHeight);

    AllocInfo allocinfoIn(ppSrcImgWidth, ppSrcImgHeight, \
        NSCam::eImgFmt_YUY2, \
        eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );


    AllocInfo allocinfoOut(gu32ppDstImgWidth, gu32ppDstImgHeight, \
        NSCam::eImgFmt_YV12, \
        eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );


    for ( i = 0; i < plane; i++)
    {
        bufStridesInPixels[i] =
                MSize(queryPlaneWidthInPixels(allocinfoIn.fmt,i, allocinfoIn.w),
                queryPlaneHeightInPixels(allocinfoIn.fmt,i, allocinfoIn.h));
        bufStridesInBytes[i]  = PIXEL2BYTE(i, bufStridesInPixels[i].w, allocinfoIn.fmt);
    }

    IImageBufferAllocator::ImgParam imgParam(
        allocinfoIn.fmt,
        MSize(allocinfoIn.w,allocinfoIn.h),
        bufStridesInBytes, //SL for new arguments bufStridesInPixels,
        bufBoundaryInBytes,
        plane
        );
    pBufSrc = allocator->alloc_ion(LOG_TAG, imgParam);
    pBufSrc->lockBuf( LOG_TAG, allocinfoIn.usage ) ;
    for ( count = 0; count < 1/*just YUY2 1 plane LOMO_HAL_JNI_MAX_PLANE*/; count++)
    {
        gu32ppSrcImgVA [count] = (MUINT8*)pBufSrc.get() ->getBufVA(count);  //This is the source image VA ADDR to postprocess (YUY2)
        gu32ppSrcImgMVA[count] =  (MUINT8*)pBufSrc.get() ->getBufPA(count);//This is the source image MVA ADDR to postprocess (YUY2)
    }


//Output Image
#ifndef USE_IMAGE_BUFFER_HEAP
    plane = 3;
    for ( i = 0; i < plane; i++)
    {
        bufStridesInPixels[i] =
            MSize(queryPlaneWidthInPixels(allocinfoOut.fmt,i, allocinfoOut.w),
            queryPlaneHeightInPixels(allocinfoOut.fmt,i, allocinfoOut.h));
        bufStridesInBytes[i]  = PIXEL2BYTE(i, bufStridesInPixels[i].w, allocinfoOut.fmt);

    }
    IImageBufferAllocator::ImgParam imgParamOut(
        allocinfoOut.fmt,
        MSize(allocinfoOut.w,allocinfoOut.h),
        bufStridesInBytes, //SL for new arguments bufStridesInPixels,
        bufBoundaryInBytes,
        plane
        );
    pBufDst = allocator->alloc_ion(LOG_TAG, imgParamOut);
    pBufDst->lockBuf( LOG_TAG, allocinfoOut.usage ) ;
    for ( count = 0; count < LOMO_HAL_JNI_MAX_PLANE; count++)
    {
        gu32ppDstImgVA [count] =  (MUINT8*)pBufDst.get() ->getBufVA(count);  //This is the source image VA ADDR to postprocess (YUY2)
        gu32ppDstImgMVA[count] = (MUINT8*)pBufDst.get() ->getBufPA(count);//This is the source image MVA ADDR to postprocess (YUY2)
    }
/////
#endif //USE_IMAGE_BUFFER_HEAP
    return err;
}


MINT32 LomoHalJniImp::AllocLomoDstImage(MUINT32 Number, \
                                                                     MUINT8** ppDstImgVA \
                                                                       )
{

    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    if(pBufDstHeapBuffIdx[Number]!=NULL)
    {
        if(pBufDstHeapBuffIdx[Number]!= ppDstImgVA[0])
        {
            LOG_ERR("ERROR !! pBufDstHeapBuffIdx[%d] is NOT matching SOURCE 0x%p",Number,(MUINT8*)ppDstImgVA[0]);
            err = LOMOHALJNI_WRONG_PARAM;
        }
        return err;
    }
#ifdef USE_IMAGE_BUFFER_HEAP
                AllocInfo allocinfoIn(gu32ppDstImgWidth, gu32ppDstImgHeight, \
                                            NSCam::eImgFmt_YV12, \
                                            eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );

                  MSize bufStridesInPixels[3] = {0};
                  MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                  MUINT32 bufStridesInBytes[3]={0};
                MUINT32 plane = 3;
                MUINT32 i, count;
                   for ( i = 0; i < plane; i++)
                   {
                       bufStridesInPixels[i] =
                           MSize(queryPlaneWidthInPixels(allocinfoIn.fmt,i, allocinfoIn.w),
                                   queryPlaneHeightInPixels(allocinfoIn.fmt,i, allocinfoIn.h));
                       bufStridesInBytes[i]  = PIXEL2BYTE(i, bufStridesInPixels[i].w, allocinfoIn.fmt);
                   }
                   /*for ( i = 0; i < plane; i++)
                   {
                       MY_LOG("bufStridesInBytes[%d] = %d",i,bufStridesInBytes[i]);
                   }*/
                   IImageBufferAllocator::ImgParam imgParam(
                      allocinfoIn.fmt,
                      MSize(allocinfoIn.w,allocinfoIn.h),
                      bufStridesInBytes, //SL for new arguments bufStridesInPixels,
                      bufBoundaryInBytes,
                      plane
                      );

        PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                        (-1), (MUINTPTR)ppDstImgVA[0],
                                        0, 0, 0);


        //
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
                                                        LOG_TAG,
                                                        imgParam,
                                                        portBufInfo);
        if(pHeap == 0)
        {
            MY_LOG("pHeap is NULL- skip this time");
            return MFALSE;
        }
        //
        //IImageBuffer* tempBuffer = pHeap->createImageBuffer(); //
        pBufDstHeap[Number] = pHeap->createImageBuffer();
        pBufDstHeap[Number]->incStrong(pBufDstHeap[Number].get());
        pBufDstHeap[Number]->lockBuf(
                        LOG_TAG,
                        allocinfoIn.usage );// eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
/////
        if (pBufDstHeap[Number].get() == NULL)
        {
            MY_LOG("NULL Buffer!- skip this time");
            return LOMOHALJNI_NULL_OBJECT;
        }
        pBufDstHeapBuffIdx[Number]= (MUINT8*)(pBufDstHeap[Number].get() ->getBufVA(0));
        //140522>>
        pBufDstHeap[Number].get()->syncCache(eCACHECTRL_INVALID);
        //140522<<
        if(pBufDstHeap[Number].get() ->getBufVA(0)==0) //NULL
        {
                    MY_LOG("No MVA addr !!! - skip this time");
                    return LOMOHALJNI_WRONG_PARAM;
        }
        #ifdef     USE_IMAGE_BUFFER_HEAP
        if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
        {
            for(MUINT32 NumberIdx=0; NumberIdx<LOMO_EFFECT_DST_BUFF_NUM; NumberIdx++)
            {
                MY_LOG("pBufDstHeapBuffIdx[%d]=0x%p",NumberIdx,(MUINT8*)pBufDstHeapBuffIdx[NumberIdx]);
            }
        }
        #endif
/////

#endif //USE_IMAGE_BUFFER_HEAP
    return err;
}
MINT32 LomoHalJniImp::UploadLomoSrcImage(MUINT8* pvCBVA)
{

    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    void           *pBuffer[3];
    void           *pBufferMva[3];
    MUINT32       size[3];
    DpRect         tmpRect;
    MUINT i,count;
    LomoTimer ldequeTmr("DpBlitStream deque",size[0],1);
    LOG_INF("+");
    LOMO_TRACE_CALL();
    if(pStream == NULL)
    {
        pStream = new DpBlitStream();
    }
    // Setup buffer address
    pBuffer[0] = (void*)pvCBVA;  //YV12 YUV420
    pBuffer[1] = (void*)((MUINTPTR)pBuffer[0]+((gu32pvCBWidth*gu32pvCBHeight))); //YV12 YUV420
    pBuffer[2] = (void*)((MUINTPTR)pBuffer[1]+((gu32pvCBWidth*gu32pvCBHeight)>>2));//YV12 YUV420

    // Setup buffer size
    size[0] = (gu32pvCBWidth*gu32pvCBHeight);
    size[1] = (gu32pvCBWidth*gu32pvCBHeight) >> 2;
    size[2] = (gu32pvCBWidth*gu32pvCBHeight) >> 2;

    pStream->setSrcBuffer(pBuffer, size, 3);

    tmpRect.x = 0;
    tmpRect.y = 0;
    tmpRect.w = gu32pvCBWidth;
    tmpRect.h = gu32pvCBHeight;
    pStream->setSrcConfig(gu32pvCBWidth, gu32pvCBHeight, DP_COLOR_YV12, eInterlace_None, &tmpRect);

    // Setup Src Image buffer address
    for(i=0;i<LOMO_HAL_JNI_MAX_PLANE;i++)
    {
        pBuffer[i]=(void*)gu32ppSrcImgVA[i];
        pBufferMva[i]=(void*)gu32ppSrcImgMVA[i];
    }
    // Setup buffer size
    size[0] = (gu32ppSrcImgWidth*gu32ppSrcImgHeight)<<1;
    size[1] = 0;
    size[2] = 0;

    pStream->setDstBuffer(pBuffer,pBufferMva, size, 1);
    pStream->setDstConfig(gu32ppSrcImgWidth, gu32ppSrcImgHeight, DP_COLOR_YUY2);

//    pBufSrc.get()->syncCache(eCACHECTRL_FLUSH);
//    MY_LOG("pBufSrc(eCACHECTRL_FLUSH)");
    ldequeTmr.start("DpBlitStream deque",size[0],1);
    pStream->invalidate();
    ldequeTmr.printTime();

    LOG_INF("-");
    return err;
}



/**
*/
MINT32 LomoHalJniImp::FreeLomoSrcImage(void)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    if(pBufSrc.get()==NULL)
    {
        LOG_WRN("pBufSrc.get() is NULL & failure in allocator->free(pBufSrc.get()!");
    }
    else
    {
        if( !pBufSrc->unlockBuf( LOG_TAG ) )
        {
            MY_LOG("!pBufSrc->unlockBuf( LOG_TAG ) failed !");
        }
        allocator->free(pBufSrc.get());
        MY_LOG("FreePostProcessSrcImage()");
    }
    for (MUINT32 count = 0; count < LOMO_HAL_JNI_MAX_PLANE; count++)
    {
        MY_LOG("Free\n gu32ppSrcImgVA[%d] 0x%p",count,gu32ppSrcImgVA[count]);
        MY_LOG("gu32ppSrcImgVA[%d] 0x%p",count,gu32ppSrcImgVA[count]);
        gu32ppSrcImgVA[count] = NULL;
        gu32ppSrcImgVA[count] = NULL;
    }
    pBufSrc = NULL;

    return err;
}


MINT32 LomoHalJniImp::LomoImageEnque(MUINT8** ppDstImgVA, MINT32 ppEffectIdx)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
//    DpBlitStream   *pStream;
    void           *pBuffer[3];
    void           *pBufferMva[3];
    MUINT32       size[3];
    DpRect         tmpRect;
    MUINT i,count;
//    IFeatureStream* mpIStream=NULL;  //For Pass2
    QParams params;
    QParams dequeParams;
    MINT64 timeout=LOMO_EFFECT_TIME_OUT;  //5s,unit is nsec
    MBOOL deqRet=0; // 1   ok, 0 NG
    MUINT32 magicNum=LOMO_EFFECT_MAGICNUM_SHIFT; //for tuning
    MBOOL NoMatchingVA= MFALSE; // 1   ok, 0 NG

    NSCam::NSIoPipe::NSPostProc::Input minput;
    NSCam::NSIoPipe::NSPostProc::Output moutput;


    LomoTimer lLomoTmr("LomoTmr",ppEffectIdx,1);
//    LomoTimer lSettingTmr("EffectSetting",ppEffectIdx,1);
    LomoTimer lenqueTmr("lomo->enque",magicNum,1);
//    LomoTimer lDstFlushTmr("DstFlush",magicNum,0);
    LOMO_TRACE_CALL();
    if(ppEffectIdx!=(-1))//0xFFFFFFFF)
   {
    MY_LOG("ppEffectIdx[%d]",ppEffectIdx);
#if 1
#ifdef     USE_IMAGE_BUFFER_HEAP
    for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
    {
        if(pBufDstHeapBuffIdx[Number]==ppDstImgVA[0])
        {
//          MY_LOG("Buffer #%d is matching",Number);
            moutput.mBuffer = const_cast<IImageBuffer*>(pBufDstHeap[Number].get());
            break;
        }
        else if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
        {
            NoMatchingVA= MTRUE;
            MY_LOG("Buffer #%d is NOT matching 0x%p!",Number,ppDstImgVA[0]);
            return LOMOHALJNI_WRONG_PARAM;
        }
    }
#else
    moutput.mBuffer = const_cast<IImageBuffer*>(pBufDst.get());
#endif
#endif
    TuningMgr::getInstance().flushSetting(eSoftwareScenario_Main_Normal_Stream);

    magicNum+=ppEffectIdx;


//    lSettingTmr.start("EffectSetting",ppEffectIdx,1);
    TuningMgr::getInstance().dequeBuffer(eSoftwareScenario_Main_Normal_Stream,magicNum);
    ColorEffectSetting(ppEffectIdx);
    TuningMgr::getInstance().enqueBuffer(eSoftwareScenario_Main_Normal_Stream,magicNum);
//    lSettingTmr.printTime();

    params.mvIn.clear();
    minput.mBuffer = const_cast<IImageBuffer*>(pBufSrc.get());
    minput.mPortID.inout=0; //in
    minput.mPortID.index=NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMGI;
    minput.mPortID.type=NSCam::NSIoPipe::EPortType_Memory;
    params.mvIn.push_back(minput);

    // Setup Dst Image buffer address
    params.mvOut.clear();
    moutput.mPortID.inout=1; //out
    moutput.mPortID.index=NSCam::NSIoPipe::NSPostProc::EPipePortIndex_WDMAO;//SL NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMG3O;//SL//NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMG2O;
    moutput.mPortID.type=NSCam::NSIoPipe::EPortType_Memory;
    params.mvOut.push_back(moutput);

    params.mFrameNo=magicNum;
    lenqueTmr.start("lomo->enque",magicNum,1);
    mpIStream->enque(params);
    lenqueTmr.printTime();
    LomoDequeTrigger();

    ///deque
//    dequeParams.mFrameNo=magicNum;
//    MY_LOG("%d= SL deque ===>",magicNum);
//    deqRet=mpIStream->deque(dequeParams, timeout);
//    MY_LOG("%d= SL deque ===<",magicNum);

  }
    return err;
}


MINT32 LomoHalJniImp::LomoImageDeque(MUINT8** ppDstImgMVA, MINT32 ppEffectIdx)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    LOMO_TRACE_CALL();
#ifdef USE_IMAGE_BUFFER_HEAP
    QParams dequeParams;
    MINT64 timeout=LOMO_EFFECT_TIME_OUT;  //5s,unit is nsec
    MUINT32 magicNum=LOMO_EFFECT_MAGICNUM_SHIFT; //for tuning
    MBOOL deqRet=0; // 1   ok, 0 NG
    NSCam::NSIoPipe::NSPostProc::Output moutput;
    MBOOL NoMatchingVA= MFALSE; // 1   ok, 0 NG
    MUINT8* mu32VaAddr= NULL; //
    MUINT8* mu32FlsuhVaAddr= NULL; //


    static MUINT32 frameCnt=0;
    static MUINT32 frameCntEffectIdx=0xFFFF;
    if(brawdumpEnable==0)
    {
        frameCntEffectIdx=0xFFFF;
    }
    if(ppEffectIdx!=(-1))//0xFFFFFFFF)
    {
        LomoTimer ldequeTmr("lomo->deque_AP",magicNum,1);

        magicNum+=ppEffectIdx;

        ///deque
        ldequeTmr.start("lomo->deque_AP",magicNum,1);
        //deqRet=mpIStream->deque(dequeParams, timeout);

        ::sem_wait(&mDequeDoneSem); // wait here until someone use sem_post() to wake this semaphore up
///
        ldequeTmr.printTime();
        for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
        {
                if(pBufDstHeapBuffIdx[Number]==ppDstImgMVA[0])
                {

                    #ifdef LOMO_FLUSH_AP
                    LomoTimer lDstFlushTmr("DstFlush_AP",magicNum,0);
                    //lDstFlushTmr.start("DstFlush_AP",magicNum,1);
                    moutput.mBuffer = const_cast<IImageBuffer*>(pBufDstHeap[Number].get());
                    pBufDstHeap[Number].get()->syncCache(eCACHECTRL_INVALID);
                    lDstFlushTmr.printTime();
                    #endif
                    mu32FlsuhVaAddr = ppDstImgMVA[0];

                    if(brawdumpEnable)
                    {
                        char fileName[64];
                        int fileSize=0;
                        MUINT32 shiftIdx = 0;
                        if(ppEffectIdx!=(-1))
                        {
                            shiftIdx = (MUINT32)ppEffectIdx;
                        }
                        if((lu32BuffeffectIdx>>shiftIdx)&0x01)
                        {
                            if(frameCntEffectIdx==shiftIdx)
                            {
                                MY_LOG(" lu32BuffIdx=0x%8x (%d)",lu32BuffeffectIdx,lu32BuffeffectIdx);
                                frameCnt++;
                                sprintf(fileName, "/sdcard/lomo/lomo%4dx%4d_%2d_%5d_YV12_s.bin", gu32ppDstImgWidth,gu32ppDstImgHeight,shiftIdx,frameCnt);
                            }
                            else
                            {
                                sprintf(fileName, "/sdcard/lomo/lomo%4dx%4d_%2d_%5d_YV12.bin", gu32ppDstImgWidth,gu32ppDstImgHeight,shiftIdx,frameCnt);
                            }
                            FILE *fp = fopen(fileName, "wb");
                            for(MUINT8 count=0;count<3;count++)
                            {
                                fileSize +=pBufDstHeap[Number].get() ->getBufSizeInBytes(count);
                                //MY_LOG("fileSize[%d] = %d", count, fileSize);
                            }
                            fileSize = gu32ppDstImgWidth*gu32ppDstImgHeight*3/2;
                            if (NULL == fp)
                            {
                                LOG_ERR("fail to open file to save img: %s", fileName);
                                MINT32 error = mkdir("/sdcard/lomo", S_IRWXU | S_IRWXG | S_IRWXO);
                                MY_LOG("error = %d", error);
                            }
                            else
                            {
                                fwrite(reinterpret_cast<void *>(mu32FlsuhVaAddr), 1, fileSize, fp);
                                fclose(fp);
                            }
                            if(frameCntEffectIdx==0xFFFF)
                            {
                                frameCntEffectIdx=shiftIdx;
                                MY_LOG("frameCntEffectIdx = %d\n", frameCntEffectIdx);
                            }
                        }
                    }
                    break;
                }
                else if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
                {
                    NoMatchingVA= MTRUE;
                    MY_LOG("deque buffer no matching! VA : 0x%p",mu32VaAddr);
                    return LOMOHALJNI_WRONG_PARAM;
                }
        }
    }
#else
    MUINT i,count;;
    for (count = 0; count < LOMO_HAL_JNI_MAX_PLANE; count++)
    {
        memcpy((void*)ppDstImgMVA[count],(void*)pBufDst.get() ->getBufVA(count),pBufDst.get() ->getBufSizeInBytes(count));
    }
#endif
    return err;
}

MINT32 LomoHalJniImp::FreeLomoDstImage(void)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

#ifdef USE_IMAGE_BUFFER_HEAP
///////////////////
//                    IImageBuffer* tempBuffer = const_cast<IImageBuffer*>(pImageBuffer);
    for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
    {
        if(pBufDstHeap[Number].get()!=NULL)
        {
            MY_LOG("Free\n pBufDstHeap[%d] 0x%p",Number,(void*)pBufDstHeap[Number].get());
            pBufDstHeap[Number]->unlockBuf(LOG_TAG);
            pBufDstHeap[Number]->decStrong(pBufDstHeap[Number].get());
            pBufDstHeap[Number] = NULL;
            pBufDstHeapBuffIdx[Number]=0;
        }
    }
///////////////////
#else
    allocator->free(pBufDst.get());
    MY_LOG("FreePostProcessDstImage()");
    if( !pBufDst->unlockBuf( LOG_TAG ) )
    {
        MY_LOG("!pBufSrc->unlockBuf( LOG_TAG ) failed !");
    }
    else
    {
        for (MUINT32 count = 0; count < LOMO_HAL_JNI_MAX_PLANE; count++)
        {
            MY_LOG("Free\n gu32ppDstImgVA[%d] 0x%p",count,gu32ppDstImgVA[count]);
            MY_LOG("gu32ppDstImgVA[%d] 0x%p",count,gu32ppDstImgVA[count]);
            gu32ppDstImgVA[count] = NULL;
            gu32ppDstImgVA[count] = NULL;
        }
    }
    pBufDst = NULL;
#endif
    return err;
}

MUINT32 LomoHalJniImp::ColorEffectSetting(MUINT32 caseNum)
{

    TUNING_MGR_REG_IO_STRUCT  regIo[500];
    MUINT32 i;
    MUINT32 eIdx_Effect = caseNum;//(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+(caseNum%7);
    RAWIspCamInfo m_rIspCamInfo = NSIspTuning::RAWIspCamInfo();
    EIndex_Effect_T m_eIdx_Effect = (EIndex_Effect_T)eIdx_Effect;
    IspUsrSelectLevel_T m_IspUsrSelectLevel;// = NSIspTuning::IspUsrSelectLevel_T();
    ISP_NVRAM_G2C_T g2c;
    ISP_NVRAM_G2C_SHADE_T g2c_shade;
    ISP_NVRAM_SE_T se;
    ISP_NVRAM_GGM_T ggm;
    ESensorDev_T m_sesnorDev = ESensorDev_Sub;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("lomo.jni.regdump", value, "0");
    MBOOL bRegDumpEnable = atoi(value);

    TUNING_MGR_REG_IO_STRUCT g2gDataCase1[] =
        {
        {0x00004920, 0x00000200},
        {0x00004924, 0x000002CE},
        {0x00004928, 0x1F500200},
        {0x0000492C, 0x00001E92},
        {0x00004930, 0x038B0200},
        {0x00004934, 0x00000000},
        {0x00004938, 0x00000009}};

    if(bRegDumpEnable)
    {
        MY_LOG("lomo.jni.regdump=%d ",bRegDumpEnable);
        MY_LOG("eIdx_Effect(%d) #0",eIdx_Effect);
    }

    if(m_pIspTuningCustom==NULL)
    {
        MY_LOG("IspTuningCustom::createInstance(m_sesnorDev, 0x0924)");
        m_pIspTuningCustom = IspTuningCustom::createInstance(m_sesnorDev, 0x0924);
    }
    //Fix effect unstable in WhiteBoard/BlackBoard/Posterize Effect    140821 >>
    if(bAddrDumpEnable)
    {
        MY_LOG("se.edge_ctrl_1.val=0x%8x \n",se.edge_ctrl_1.val);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_SEL=%d \n",se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_SEL);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_C=%d \n",se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_C);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_SEL=%d \n",se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_SEL);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_C=%d \n",se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_C);
    }
    se.out_edge_ctrl.val=0;
    se.y_ctrl.val=0;
    se.edge_ctrl_1.val=0;
    se.edge_ctrl_2.val=0;
    se.edge_ctrl_3.val=0;
    se.special_ctrl.val=0;
    se.core_ctrl_1.val=0;
    se.core_ctrl_2.val=0;

    m_IspUsrSelectLevel.eIdx_Edge = MTK_CONTROL_ISP_EDGE_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Hue = MTK_CONTROL_ISP_HUE_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Sat = MTK_CONTROL_ISP_SATURATION_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Bright = MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Contrast = MTK_CONTROL_ISP_CONTRAST_MIDDLE;

    //memset(&se, 0, sizeof(ISP_NVRAM_SE_T)); //Test case 140821
    //Fix effect unstable in WhiteBoard/BlackBoard/Posterize Effect    140821 <<
    m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfo, m_eIdx_Effect, m_IspUsrSelectLevel, g2c, g2c_shade, se, ggm);
    m_pIspTuningCustom->userSetting_EFFECT_GGM_JNI(m_rIspCamInfo, m_eIdx_Effect, ggm);

    //G2G
    TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_G2g);
    TUNING_MGR_WRITE_ENABLE_BITS(eSoftwareScenario_Main_Normal_Stream, CAM_CTL_EN_P2, G2G_EN, 0x01);
    for(i=0;i<sizeof(g2gDataCase1)/sizeof(TUNING_MGR_REG_IO_STRUCT);i++){
        regIo[i].Addr = g2gDataCase1[i].Addr;
        regIo[i].Data = g2gDataCase1[i].Data;
    }
    TuningMgr::getInstance().tuningMgrWriteRegs(eSoftwareScenario_Main_Normal_Stream, regIo, sizeof(g2gDataCase1)/sizeof(TUNING_MGR_REG_IO_STRUCT));
    //
    //G2C conv
    TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_G2c_Conv);
    TUNING_MGR_WRITE_ENABLE_BITS(eSoftwareScenario_Main_Normal_Stream, CAM_CTL_EN_P2, G2C_EN, 0x01);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_CONV_0A, g2c.conv_0a.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_CONV_0B, g2c.conv_0b.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_CONV_1A, g2c.conv_1a.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_CONV_1B, g2c.conv_1b.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_CONV_2A, g2c.conv_2a.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_CONV_2B, g2c.conv_2b.val);
    if(bRegDumpEnable)
    {
        MY_LOG("eIdx_Effect(%d) - 0x4BA0 0x%8x",eIdx_Effect,g2c.conv_0a.val&ISP_NVRAM_G2C_CONV_0A_T::MASK);
        MY_LOG("eIdx_Effect(%d) - 0x4BA4 0x%8x",eIdx_Effect,g2c.conv_0b.val&ISP_NVRAM_G2C_CONV_0B_T::MASK);
        MY_LOG("eIdx_Effect(%d) - 0x4BA8 0x%8x",eIdx_Effect,g2c.conv_1a.val&ISP_NVRAM_G2C_CONV_1A_T::MASK);
        MY_LOG("eIdx_Effect(%d) - 0x4BAC 0x%8x",eIdx_Effect,g2c.conv_1b.val&ISP_NVRAM_G2C_CONV_1B_T::MASK);
    }
    TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_G2c_Shade);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_SHADE_CON_1, g2c_shade.con_1.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_SHADE_CON_2, g2c_shade.con_2.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_SHADE_CON_3, g2c_shade.con_3.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_SHADE_TAR, g2c_shade.tar.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_G2C_SHADE_SP, g2c_shade.sp.val);

    //SEEE
    TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_Seee);
    TUNING_MGR_WRITE_ENABLE_BITS(eSoftwareScenario_Main_Normal_Stream, CAM_CTL_EN_P2, SEEE_EN, 0x01);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_OUT_EDGE_CTRL, se.out_edge_ctrl.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_SE_Y_CTRL, se.y_ctrl.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_SE_EDGE_CTRL_1, se.edge_ctrl_1.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_SE_EDGE_CTRL_2, se.edge_ctrl_2.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_SE_EDGE_CTRL_3, se.edge_ctrl_3.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_SE_SPECL_CTRL, se.special_ctrl.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_SE_CORE_CTRL_1, se.core_ctrl_1.val);
    TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream, CAM_SEEE_SE_CORE_CTRL_2, se.core_ctrl_2.val);

    switch ((EIndex_Effect_T)eIdx_Effect)
    {

        case MTK_CONTROL_EFFECT_MODE_NASHVILLE:
        case MTK_CONTROL_EFFECT_MODE_HEFE:
        case MTK_CONTROL_EFFECT_MODE_VALENCIA :
        case MTK_CONTROL_EFFECT_MODE_XPROII :
        case MTK_CONTROL_EFFECT_MODE_LOFI :
        case MTK_CONTROL_EFFECT_MODE_SIERRA :
        case MTK_CONTROL_EFFECT_MODE_KELVIN :
        case MTK_CONTROL_EFFECT_MODE_WALDEN :
        case MTK_CONTROL_EFFECT_MODE_F1977 :
            //GGM_RB
            TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_Ggm_Rb);
            TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_Ggm_G);
            TUNING_MGR_WRITE_ENABLE_BITS(eSoftwareScenario_Main_Normal_Stream, CAM_CTL_EN_P2, GGM_EN, 0x01);
            for(int idx=0; idx<CUSTOM_LOMO_GGM_GAIN_NUM;idx++)
            {
                TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream,CAM_GGM_LUT_RB[idx],LomoFilterGGMJni[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_BR][idx]);
                TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream,CAM_GGM_LUT_G[idx],LomoFilterGGMJni[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_G][idx]);
            }
        break;
        case MTK_CONTROL_EFFECT_MODE_MONO:
        case MTK_CONTROL_EFFECT_MODE_NEGATIVE:
        case MTK_CONTROL_EFFECT_MODE_SOLARIZE:
        case MTK_CONTROL_EFFECT_MODE_SEPIA:
        case MTK_CONTROL_EFFECT_MODE_POSTERIZE:
        case MTK_CONTROL_EFFECT_MODE_WHITEBOARD:
        case MTK_CONTROL_EFFECT_MODE_BLACKBOARD:
        case MTK_CONTROL_EFFECT_MODE_AQUA:  //,
        case MTK_CONTROL_EFFECT_MODE_OFF:
        default:
            //GGM_G
/**/            TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_Ggm_Rb);
/**/            TuningMgr::getInstance().updateEngine(eSoftwareScenario_Main_Normal_Stream, eTuningMgrFunc_Ggm_G);
            TUNING_MGR_WRITE_ENABLE_BITS(eSoftwareScenario_Main_Normal_Stream, CAM_CTL_EN_P2, GGM_EN, 0x01);
            for(int idx=0; idx<CUSTOM_LOMO_GGM_GAIN_NUM;idx++)
            {
                TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream,CAM_GGM_LUT_RB[idx],LomoFilterGGMJni[0][CUSTOM_LOMO_GGM_CHANNEL_BR][idx]);
                TUNING_MGR_WRITE_REG(eSoftwareScenario_Main_Normal_Stream,CAM_GGM_LUT_G[idx],LomoFilterGGMJni[0][CUSTOM_LOMO_GGM_CHANNEL_G][idx]);
            }
            break;
    }
    return 0;
}


#include "mtkcam/v1/config/PriorityDefs.h"

/*******************************************************************************
*
********************************************************************************/
MVOID *LomoHalJniImp::LomoDequeThreadLoop(MVOID *arg)
{
    LomoHalJniImp *_this = reinterpret_cast<LomoHalJniImp *>(arg);
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

    //====== Change Thread Setting ======

    _this->ChangeThreadSetting();

    //====== Main Loop ======

    LOMO_STATE_ENUM eState = _this->GetLomoState();

    while(eState != LOMO_STATE_UNINIT)
    {
//        MY_LOG("wait mDequeSem");
        ::sem_wait(&_this->mDequeSem); // wait here until someone use sem_post() to wake this semaphore up
//        MY_LOG("got mDequeSem");
        if(0)
        {
            int mu32Val;
            ::sem_getvalue(&_this->mDequeSem, &mu32Val);
            MY_LOG("mDequeSem %d",mu32Val);
        }

        eState = _this->GetLomoState();

        switch(eState)
        {
            case LOMO_STATE_ALIVE:
                    err = _this->LomoDequeBuffer();
                    if(err != LOMOHALJNI_NO_ERROR)
                    {
                        LOG_ERR("Deque LOMO fail(%d)",err);
                    }
                break;
            case LOMO_STATE_UNINIT :
                MY_LOG("UNINIT");
                break;
            default:
                LOG_ERR("State Error(%d)",eState);
        }

        eState = _this->GetLomoState();
    }

    ::sem_post(&_this->mDequeSemEnd);
//    MY_LOG("post mDequeSemEnd");

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MVOID LomoHalJniImp::SetLomoState(const LOMO_STATE_ENUM &aState)
{
    mState = aState;

    MY_LOG("aState(%d),mState(%d)",aState,mState);
}


/*******************************************************************************
*
********************************************************************************/
LOMO_STATE_ENUM LomoHalJniImp::GetLomoState()
{
    return mState;
}

#define PR_SET_NAME    15
/*******************************************************************************
*
********************************************************************************/
MVOID LomoHalJniImp::ChangeThreadSetting()
{
    //> set name

    ::prctl(PR_SET_NAME,"LomoDeqThread", 0, 0, 0);

    //> set policy/priority
    const MINT32 policy   = SCHED_NORMAL;
    const MINT32 priority = NICE_CAMERA_LOMO;

    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    //>  get policy/priority
    ::sched_getparam(0, &sched_p);

    MY_LOG("policy:(expect, result)=(%d,%d), priority:(expect, result)=(%d, %d)", policy,
                                                                                    ::sched_getscheduler(0),
                                                                                    priority,
                                                                                    sched_p.sched_priority);
}


/*******************************************************************************
*
********************************************************************************/
MVOID LomoHalJniImp::LomoDequeTrigger()
{
    MY_LOG("LomoDequeTrigger()");
    ::sem_post(&mDequeSem);
    if(0)
    {
        int mu32Val;
        ::sem_getvalue(&mDequeSem, &mu32Val);
        MY_LOG("mDequeSem %d",mu32Val);
    }

}

/*******************************************************************************
*
********************************************************************************/
LOMOHALJNI_RETURN_TYPE LomoHalJniImp::LomoDequeBuffer()
{
#if 0

    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    return err;
#else

    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    //MY_LOG("LomoDequeBuffer()");
    LOMO_TRACE_CALL();
    QParams dequeParams;
    MINT64 timeout=LOMO_EFFECT_TIME_OUT;  //5s,unit is nsec
    MUINT32 magicNum=LOMO_EFFECT_MAGICNUM_SHIFT*9; //for tuning
    MBOOL deqRet=0; // 1   ok, 0 NG
    MBOOL NoMatchingVA= MFALSE; // 1   ok, 0 NG
    MUINT8* mu32VaAddr= NULL; //
    MUINT8* mu32MVaAddr= NULL; //
    MUINT8* mu32FlsuhVaAddr= NULL; //
    MUINT32 mu32OutPortSize= 0; //
    NSCam::NSIoPipe::NSPostProc::Output moutput;

    LomoTimer ldequeTmr("lomo->deque_THR",magicNum,1);
    ///deque
    //ldequeTmr.start("lomo->deque_THR",magicNum,1);
    deqRet=mpIStream->deque(dequeParams, timeout);
    ldequeTmr.printTime();
    magicNum = dequeParams.mFrameNo;
//    MY_LOG("lomo dequeParams.mFrameNo = %d()",dequeParams.mFrameNo);

//    MY_LOG("lomo magicNum = (%d)",magicNum);
    if(deqRet!=1)
    {
        LOG_ERR("ERROR !! mpIStream->deque err 0x%x dequeParams.mFrameNo=%d",deqRet,dequeParams.mFrameNo);
        LOG_ERR("ERROR !! dequeParams.mvOut[0].mBuffer->getBufVA(0)=0x%p",dequeParams.mvOut[0].mBuffer->getBufVA(0));
        err = LOMOHALJNI_PASS2_NOT_READY;
    }
    mu32OutPortSize = dequeParams.mvOut.size();
//    MY_LOG("lomo mu32OutPortSize = (%d)",mu32OutPortSize);
    for(unsigned int j=0;j<mu32OutPortSize;j++)
    {
        mu32VaAddr = (MUINT8*)dequeParams.mvOut[j].mBuffer->getBufVA(0);
        mu32MVaAddr = (MUINT8*)dequeParams.mvOut[j].mBuffer->getBufPA(0);
        if(j==0)
        {
            mu32FlsuhVaAddr =mu32VaAddr;
        }
        if(bAddrDumpEnable)
        {
            MY_LOG("Deque#%d magicNum(%d) mvOut VA(0x%p) MVA(0x%p)",j,magicNum,mu32VaAddr,mu32MVaAddr);
        }
    }
////
    for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
    {
        if(pBufDstHeapBuffIdx[Number]==mu32FlsuhVaAddr)//ppDstImgMVA[0])
        {
              #ifdef LOMO_FLUSH_TH
              LomoTimer lDstFlushTmr("DstFlush_THR",magicNum,0);
              //lDstFlushTmr.start("DstFlush_THR",magicNum,1);
            moutput.mBuffer = const_cast<IImageBuffer*>(pBufDstHeap[Number].get());
            pBufDstHeap[Number].get()->syncCache(eCACHECTRL_INVALID);
            lDstFlushTmr.printTime();
            #endif

            ::sem_post(&mDequeDoneSem);
            //MY_LOG("post mDequeDoneSem");
            if(0)
            {
                int mu32Val;
                ::sem_getvalue(&mDequeDoneSem, &mu32Val);
                MY_LOG("mDequeDoneSem %d",mu32Val);
            }
            break;
        }
        else if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
        {
            NoMatchingVA= MTRUE;
            LOG_ERR("deque buffer no matching! VA : 0x%p",mu32VaAddr);
            return LOMOHALJNI_WRONG_PARAM;
        }
    }
///
    return err;
#endif
}


