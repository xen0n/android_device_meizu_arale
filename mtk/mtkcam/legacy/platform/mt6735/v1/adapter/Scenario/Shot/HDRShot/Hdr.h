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
#ifndef _HDR_H_
#define _HDR_H_

#include <vector>

#include <pthread.h>
#include <semaphore.h>

#include <mtkcam/common.h>
#include <mtkcam/camshot/_params.h>
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/drv/imem_drv.h>
#include <IShot.h>
#include <ImpShot.h>

#include <mtkcam/featureio/hdr_hal_base.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/UITypes.h>
#include <mtkcam/exif/IDbgInfoContainer.h>
#include <mtkcam/camshot/CapBufMgr.h>


namespace std {
    class string;
};



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum {
    HDR_STATE_INIT                    = 0x0000,
    HDR_STATE_NORMALIZATION            = 0x0001,
    HDR_STATE_FEATURE_EXTRACITON    = 0x0002,
    HDR_STATE_ALIGNMENT                = 0x0003,
    HDR_STATE_BLEND                    = 0x0004,
    HDR_STATE_UNINIT                = 0x0800,
} HdrState_e;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class CamExif;
namespace NS3A {
    class CaptureParam_T;
};

using namespace NSCamShot;
namespace NSCamShot {
    class CapBufMgr;
}

namespace android {
namespace NSShot {
class HdrShot : public ImpShot
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//kidd: remove this block
public:     ////    Interfaces.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////                    Instantiation.
    virtual         ~HdrShot();
                    HdrShot(char const*const pszShotName
                            , uint32_t const u4ShotMode
                            , int32_t const i4OpenId
                            );

public:     ////                    Operations.

    //  This function is invoked when this object is firstly created.
    //  All resources can be allocated here.
    virtual bool    onCreate();

    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
    virtual void    onDestroy();

    virtual MBOOL   setCapBufMgr(MVOID* pCapBufMgr);
    virtual bool    sendCommand(uint32_t const  cmd
                                , MUINTPTR const  arg1
                                , uint32_t const  arg2
                                , uint32_t const  arg3 = 0
                                );
    virtual bool    setShotParam(void const* pParam, size_t const size);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual MBOOL   init_imageParams();
    virtual MBOOL   init_decideCaptureMode();
    virtual MBOOL   init_compoment();

protected:  ////                    Operations.
    virtual bool    onCmd_reset();
    virtual bool    onCmd_capture();
    static  MVOID*  onCmd_captureTask(MVOID *arg);
    virtual bool    mainflow();
    virtual void    onCmd_cancel();

protected:  ////                    callbacks
    static MBOOL    fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg);
    static MBOOL    fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg);

protected:
    virtual MBOOL   handleYuvData(MUINT8* const puBuf, MUINT32 const u4Size);
    virtual MBOOL   handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size);
    virtual MBOOL   handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT32 const u4Index, MBOOL bFinal);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Buffers.
    virtual MBOOL   allocBuffer(IImageBuffer** ppBuf, MUINT32 w, MUINT32 h, MUINT32 fmt);
    virtual void    deallocBuffer(IImageBuffer* pBuf);

    virtual IImageBuffer*   makeBufferAlias(IImageBuffer *pBaseBuf, MINT32 w, MINT32 h, MINT32 imgFormat);
    virtual MBOOL   removeBufferAlias(IImageBuffer *pBase, IImageBuffer *pAlias);

    virtual MBOOL   uninitMemoryMap(void);

    static  MVOID*  allocateMemoryTask(MVOID* arg);
    static  MVOID*  allocateCapBufMemoryTask(MVOID* arg);
    static  MVOID*  allocateCaptureMemoryTask_First(MVOID* arg);
    static  MVOID*  allocateCaptureMemoryTask_Others(MVOID* arg);
    static  MVOID*  allocateProcessMemoryTask(MVOID* arg);

    virtual MBOOL   requestSourceImgBuf(void);
    virtual MBOOL   releaseSourceRawImgBuf(void);
    virtual MBOOL   releaseSourceImgBuf(void);
    virtual MBOOL   requestSmallImgBuf(void);
    virtual MBOOL   releaseSmallImgBuf(void);
    virtual MBOOL   requestSEImgBuf(void);
    virtual MBOOL   releaseSEImgBuf(void);
    virtual MBOOL   requestHdrWorkingBuf(void);
    virtual MBOOL   releaseHdrWorkingBuf(void);
    virtual MBOOL   requestOriWeightMapBuf(void);
    virtual MBOOL   releaseOriWeightMapBuf(void);
    virtual MBOOL   requestBlurredWeightMapBuf(void);
    virtual MBOOL   releaseBlurredWeightMapBuf(void);
    virtual MBOOL   requestDownSizedWeightMapBuf(void);
    virtual MBOOL   releaseDownSizedWeightMapBuf(void);
    virtual MBOOL   requestPostviewImgBuf(void);
    virtual MBOOL   releasePostviewImgBuf(void);

    virtual MBOOL   requestNormalJpegBuf(void);
    virtual MBOOL   releaseNormalJpegBuf(void);
    virtual MBOOL   requestNormalThumbnailJpegBuf(void);
    virtual MBOOL   releaseNormalThumbnailJpegBuf(void);
    virtual MBOOL   requestHdrJpegBuf(void);
    virtual MBOOL   releaseHdrJpegBuf(void);
    virtual MBOOL   requestHdrThumbnailJpegBuf(void);
    virtual MBOOL   releaseHdrThumbnailJpegBuf(void);

    virtual MBOOL   requestBlendingBuf(void);
    virtual MBOOL   releaseBlendingBuf(void);

    virtual MBOOL   updateYUVBufferAddress(void);

protected:  ////    CDP.
    virtual MBOOL   update3AExif(CamExif *pCamExif);
    virtual MBOOL   updateThumbnailExif(CamExif *pCamExif, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT8* puExifBuf, MUINT32 &u4FinalExifSize);

    virtual MBOOL   CDPResize(IImageBuffer* pInputBuf, IImageBuffer* pOutputBuf, MUINT32 transform=0);
    virtual MBOOL   CDPResize_simple(IImageBuffer* pInputBuf, IImageBuffer* pOutputBuf, MUINT32 transform=0);
    static  MBOOL   GetStride(MUINT32 srcWidth, EImageFormat srcFormat, MUINT32 *pStride);
    static  MUINT32 getAlignedSize(MUINT32 const u4Size);

protected:  ////    Save.
    static unsigned int    dumpToFile(char const *fname, unsigned char *pbuf, unsigned int size);

public:
    static  MBOOL   getCaptureInfo(MUINT32 sensorId, std::vector<NS3A::CaptureParam_T> & vCap3AParam, MUINT32 &hdrFrameNum);

protected:  ////    Misc.
    virtual MBOOL   getCaptureParams(NS3A::CaptureParam_T *pCap3AParam);
    virtual MBOOL   getCaptureExposureSettings(HDRExpSettingOutputParam_T &strHDROutputSetting);


    //flow
    virtual MBOOL    configureForSingleRun(void);

    virtual MBOOL    EVBracketCapture(void);
    virtual MBOOL    ImageRegistratoin(void);
    virtual MBOOL    WeightingMapGeneration(void);
    virtual MBOOL    Blending(void);

    virtual MBOOL    createSourceAndSmallImg(void);
    virtual MBOOL    createSEImg(void);
    virtual MBOOL	encodeRawToYuv(IImageBuffer *rawBuffer
                                    , IImageBuffer *yuvBuffer
                                    , IImageBuffer *smallyuvBuffer
                                    , NS3A::EIspProfile_T profile
                                    , void *privateData
                                    );
    virtual MBOOL   encodeJpeg(IImageBuffer *pSrcImgBufInfo
                                , NSCamShot::JpegParam const & rJpgParm
                                , MUINT32 const u4Transform
                                , IImageBuffer *pJpgImgBufInfo
                                );

    virtual MVOID*  encodeNormalJpeg(MVOID *arg);
    virtual MVOID*  encodeNormalThumbnailJpeg(MVOID *arg);
    virtual MVOID*  saveNormalJpeg(MVOID *arg);

    static  MVOID*  createHdrJpegImgTask(MVOID* arg);
    virtual MVOID*  encodeHdrJpeg(MVOID *arg);
    virtual MVOID*  encodeHdrThumbnailJpeg(MVOID *arg);
    virtual MVOID*  saveHdrJpeg(MVOID *arg);

    virtual MBOOL   convertImage(IImageBuffer *pSrcImgBuf
                                , IImageBuffer *pDesImgBuf
                                , MUINT32 const u4Transform = 0
                                , MBOOL bIsCrop = MFALSE
                                , MRect cropRect = {MPoint(0,0),MSize(0,0)}
                                );
    virtual MRect   calCrop(MRect const &rSrc
                        , MRect const &rDst
                        , uint32_t ratio
                        );

    virtual MBOOL	do_Normalization(unsigned int method); // 1: rank image, 0: normalization
    virtual MBOOL    do_SE(void);
    virtual MBOOL    do_FeatureExtraction(void);
    virtual MBOOL    do_Alignment(void);
    virtual MBOOL    do_OriWeightMapGet(void);
    virtual MBOOL   do_SetBmapBuffer(void);

    virtual MBOOL    do_DownScaleWeightMap(void);
    virtual MBOOL    do_UpScaleWeightMap(void);
    virtual MBOOL    do_Fusion(void);
    virtual MBOOL    do_HdrCroppedResultGet(void);
    virtual MBOOL    do_HdrNR(void);
    virtual MBOOL    do_CroppedPostviewResize(void);

    virtual MBOOL    do_HdrSettingClear(void);

protected:  ////    Thread.
    inline  MBOOL   wait(pthread_mutex_t *mutex, const char *note);
    inline  MBOOL   announce(pthread_mutex_t *mutex, const char *note);
    inline  static MUINT32 getUs();

    static  MBOOL   SetThreadProp(int policy, int priority);
    static  MBOOL   GetThreadProp(int *policy, int *priority);
    static  MBOOL   PrintThreadProp(const char *name);

protected:  ////    Sensor
    virtual MBOOL   querySensorInfo(NSCam::SensorStaticInfo &mSensorInfo);

protected:  ////    Attributes.
    inline  MUINT32    OutputFrameNumGet() const { return mu4OutputFrameNum ;}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Multi-Frame
    enum    { eMaxOutputFrameNum = 3 };

private:    ////    Resolutions.
    MUINT32             mRaw_Width;
    MUINT32             mRaw_Height;
    MUINT32             mu4W_yuv;        //  YUV Width    // Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32             mu4H_yuv;        //  YUV Height    // Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32             mu4W_small;        //  Small Image Width    // Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32             mu4H_small;        //  Small Image Height    // Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32             mu4W_se;        //  SW EIS Image Width    // Obtained in requestOtherBufs()\QuerySEImgResolution().
    MUINT32             mu4H_se;        //  SW EIS Image Height    // Obtained in requestOtherBufs()\QuerySEImgResolution().
    MUINT32             mu4W_dsmap;        //  Down-sized Weighting Map Width    // Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_width.
    MUINT32             mu4H_dsmap;        //  Down-sized Weighting Map Height    // Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_height.
    MUINT32             mPostviewWidth;
    MUINT32             mPostviewHeight;
    EImageFormat        mPostviewFormat;
    //
    MUINT32             mRotPicWidth;   //  full jpeg width
    MUINT32             mRotPicHeight;  //  full jpeg height
    MUINT32             mRotThuWidth;   //  thumbnail jpeg width
    MUINT32             mRotThuHeight;  //  thumbnail jpeg height


private:    ////    Thread
    MBOOL               mErrorFlag;
    //
    pthread_t           mMainThread;
    //
    pthread_t           mMemoryAllocateThread;
    pthread_mutex_t     mTrigger_alloc_working;
    pthread_mutex_t     mTrigger_alloc_bmap1;
    pthread_mutex_t     mMemoryReady_Capbuf;
    pthread_mutex_t     mMemoryReady_pass2_first;
    pthread_mutex_t     mMemoryReady_pass2_others;
    pthread_mutex_t     mMemoryReady_jpeg_full;
    pthread_mutex_t     mMemoryReady_jpeg_thumbnail;
    pthread_mutex_t     mMemoryReady_working;
    pthread_mutex_t     mMemoryReady_se;
    pthread_mutex_t     mMemoryReady_bmap0;
    pthread_mutex_t     mMemoryReady_bmap1;
    pthread_mutex_t     mMemoryReady_bmap2;
    pthread_mutex_t     mMemoryReady_blending;
    pthread_mutex_t     mMemoryReady_postview;

private:    ////    Pipes.
    HdrHalBase          *mpHdrHal;
    CamExif             *mpCamExif[eMaxOutputFrameNum];

private:    ////    Buffers.
    IMemDrv             *mpIMemDrv;
    IImageBufferAllocator* mpIImageBufAllocator;
    CapBufMgr*      mpCapBufMgr;

    //copy from fb
    typedef struct
    {
        IImageBuffer* pImgBuf;
        IMEM_BUF_INFO memBuf;
    } ImageBufferMap;
    std::vector<ImageBufferMap> mvImgBufMap;

    IImageBufferHeap    *mpHeap;
    MUINT32             mTotalBufferSize;
    MUINT32             mTotalKernelBufferSize;
    MUINT32             mTotalUserBufferSize;

    //buffers
	IImageBuffer        *mpSourceRawImgBuf[eMaxOutputFrameNum];
    IImageBuffer        *mpSourceImgBuf[eMaxOutputFrameNum];
    IImageBuffer        *mpSmallImgBuf[eMaxOutputFrameNum];  //reused as mWeightingBuf, mpBlurredWeightMapBuf
    IImageBuffer        *mpSEImgBuf[eMaxOutputFrameNum];
    IImageBuffer        *mWeightingBuf[eMaxOutputFrameNum];  //reuse buffer of mpSmallImgBuf rather than allocate one cpy
    IImageBuffer        *mpBlurredWeightMapBuf[eMaxOutputFrameNum];  //reuse buffer of mpSmallImgBuf rather than allocate one cpy
    IImageBuffer        *mpDownSizedWeightMapBuf[eMaxOutputFrameNum];
    IImageBuffer        *mBlendingBuf;

    IImageBuffer        *mpPostviewImgBuf;
    //
    IImageBuffer        *mpHdrWorkingBuf;
    IImageBuffer        *mpMavWorkingBuf;
    //
    IImageBuffer        *mNormalJpegBuf;
    IImageBuffer        *mNormalThumbnailJpegBuf;
    IImageBuffer        *mHdrJpegBuf;
    IImageBuffer        *mHdrThumbnailJpegBuf;

    HDR_PIPE_SET_BMAP_INFO mHdrSetBmapInfo;
    HDR_PIPE_WEIGHT_TBL_INFO** OriWeight;
    HDR_PIPE_WEIGHT_TBL_INFO** BlurredWeight;

private:    ////    Parameters.
    static MUINT32      mu4RunningNumber;        // A serial number for file saving. For debug.

    MUINT32                mu4OutputFrameNum;        // Output frame number (2 or 3).    // Do not use mu4OutputFrameNum in code directly, use OutputFrameNumGet() instead.

    MUINT32                mu4FinalGainDiff[2];
    MUINT32                mu4TargetTone;


    HDR_PIPE_HDR_RESULT_STRUCT mrHdrCroppedResult;

    volatile MUINT32    mfgIsForceBreak;        // A flag to indicate whether a cancel capture signal is sent.

    HdrState_e            mHdrState;

    MBOOL               mShutterCBDone;
    MBOOL               mRawCBDone;
    MBOOL               mJpegCBDone;

    MBOOL               mfgIsSkipthumb;         // A flag to indicate if skip thumbnail.

    int                 mCapturePolicy;
    int                 mCapturePriority;

    MUINT32             mCaptueIndex;
    MUINT32             mSensorType;

    MUINT32             mNrtype;
    static MUINT32      mHDRShotMode;

private:    ////    for development.
    MUINT32             mTestMode;
    MUINT32             mDebugMode;
    IDbgInfoContainer *mDebugInfo;

private:  ////    private data
    MVOID           *mPrivateData;
    MUINT32         mPrivateDataSize;
};


}; // namespace NSShot
}; // namespace android
#endif  //  _HDR_H_

