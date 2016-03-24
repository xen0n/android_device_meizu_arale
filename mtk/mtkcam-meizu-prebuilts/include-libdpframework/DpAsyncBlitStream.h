#ifndef __DP_BLIT_STREAM_H__
#define __DP_BLIT_STREAM_H__

#include "DpDataType.h"
#include "DpMutex.h"
#include "DpCondition.h"
#include <vector>


class DpSync;

#define GET_CURRENT_JOB(pJob, mutex)        \
{                                           \
    mutex->lock();                          \
    if (pJob == NULL)                       \
    {                                       \
        pJob = requireNewJob();             \
                                            \
        if (pJob == NULL)                   \
        {                                   \
            mutex->unlock();                \
            return DP_STATUS_OUT_OF_MEMORY; \
        }                                   \
    }                                       \
    mutex->unlock();                        \
}


class DpAsyncBlitStream
{
public:
    static bool queryHWSupport(uint32_t srcWidth,
                               uint32_t srcHeight,
                               uint32_t dstWidth,
                               uint32_t dstHeight,
                               int32_t  Orientation = 0);

    DpAsyncBlitStream();

    ~DpAsyncBlitStream();

    enum DpOrientation
    {
        ROT_0   = 0x00000000,
        FLIP_H  = 0x00000001,
        FLIP_V  = 0x00000002,
        ROT_90  = 0x00000004,
        ROT_180 = FLIP_H|FLIP_V,
        ROT_270 = ROT_180|ROT_90,
        ROT_INVALID = 0x80
    };

    typedef struct BlitConfig
    {
        // src
        int32_t         srcBuffer;
        int32_t         srcWidth;
        int32_t         srcHeight;
        int32_t         srcYPitch;
        int32_t         srcUVPitch;
        DpColorFormat   srcFormat;
        DP_PROFILE_ENUM srcProfile;
        DpSecure        srcSecure;
        bool            srcFlush;

        // dst
        int32_t         dstBuffer;
        int32_t         dstWidth;
        int32_t         dstHeight;
        int32_t         dstYPitch;
        int32_t         dstUVPitch;
        DpColorFormat   dstFormat;
        DP_PROFILE_ENUM dstProfile;
        DpSecure        dstSecure;
        bool            dstFlush;

        // crop
        int32_t         cropXStart;
        int32_t         cropYStart;
        int32_t         cropWidth;
        int32_t         cropHeight;

        // target offset
        int32_t         targetXStart;
        int32_t         targetYStart;

        int32_t         rotation;
        bool            frameChange;
        bool            flipStatus;
        bool            ditherStatus;
        DpPqConfig      PqConfig;

        //Compatible to 89
        int             Tdshp;
    } BlitConfig;

    typedef enum JOB_CONFIG_STATE_ENUM
    {
        JOB_CONFIGING,
        JOB_CONFIG_DONE
    } JOB_CONFIG_STATE_ENUM;

    typedef struct AsyncBlitJob
    {
        //static int32_t        jobNum;       //
        DpJobID               jobID;        // job id used as timeline val
        int32_t               fenceFd;      // fence FD for signal user
        DpJobID               cmdTaskID;    // cmdq task id for wait complete
        BlitConfig            config;       // job config
        JOB_CONFIG_STATE_ENUM state;        // job state

    } AsyncBlitJob;

    typedef std::vector<AsyncBlitJob *> JobList;

    DP_STATUS_ENUM setSrcBuffer(void     *pVABase,
                                uint32_t size);

    DP_STATUS_ENUM setSrcBuffer(void     **pVAList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber);

    // VA + MVA address interface
    DP_STATUS_ENUM setSrcBuffer(void**   pVAddrList,
                                void**   pMVAddrList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber);

    // for ION file descriptor
    DP_STATUS_ENUM setSrcBuffer(int32_t  fileDesc,
                                uint32_t *sizeList,
                                uint32_t planeNumber);

    DP_STATUS_ENUM setSrcConfig(int32_t           width,
                                int32_t           height,
                                DpColorFormat     format,
                                DpInterlaceFormat field = eInterlace_None,
                                DpRect            *pROI = 0);

    DP_STATUS_ENUM setSrcConfig(int32_t           width,
                                int32_t           height,
                                int32_t           yPitch,
                                int32_t           uvPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpRect            *pROI   = 0,
                                DpSecure          secure  = DP_SECURE_NONE,
                                bool              doFlush = true);

    DP_STATUS_ENUM setDstBuffer(void     *pVABase,
                                uint32_t size);

    DP_STATUS_ENUM setDstBuffer(void     **pVABaseList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber);

    // VA + MVA address interface
    DP_STATUS_ENUM setDstBuffer(void**   pVABaseList,
                                void**   pMVABaseList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber);

    // for ION file descriptor
    DP_STATUS_ENUM setDstBuffer(int32_t  fileDesc,
                                uint32_t *pSizeList,
                                uint32_t planeNumber);

    DP_STATUS_ENUM setDstConfig(int32_t           width,
                                int32_t           height,
                                DpColorFormat     format,
                                DpInterlaceFormat field = eInterlace_None,
                                DpRect            *pROI = 0);

    DP_STATUS_ENUM setDstConfig(int32_t           width,
                                int32_t           height,
                                int32_t           yPitch,
                                int32_t           uvPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpRect            *pROI   = 0,
                                DpSecure          secure  = DP_SECURE_NONE,
                                bool              doFlush = true);

    DP_STATUS_ENUM setConfigEnd(int32_t &fd);

    DP_STATUS_ENUM setRotate(int32_t rotation)
    {
        GET_CURRENT_JOB(m_pCurJob, m_pJobMutex);

        if (m_prevConfig.rotation != rotation)
        {
            m_pCurJob->config.frameChange = true;
        }

        m_pCurJob->config.rotation = rotation;

        m_pCurJob->state = JOB_CONFIGING;

        return DP_STATUS_RETURN_SUCCESS;
    }

    //Compatible to 89
    DP_STATUS_ENUM setFlip(int flip)
    {
        GET_CURRENT_JOB(m_pCurJob, m_pJobMutex);

        if (m_prevConfig.flipStatus != flip)
        {
            m_pCurJob->config.frameChange = true;
        }

        m_pCurJob->config.flipStatus = flip ? true : false;

        m_pCurJob->state = JOB_CONFIGING;

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setOrientation(uint32_t transform);

    DP_STATUS_ENUM setDither(bool enDither)
    {
        GET_CURRENT_JOB(m_pCurJob, m_pJobMutex);

        if (m_prevConfig.ditherStatus != enDither)
        {
            m_pCurJob->config.frameChange = true;
        }

        m_pCurJob->config.ditherStatus = enDither;

        m_pCurJob->state = JOB_CONFIGING;

        return DP_STATUS_RETURN_SUCCESS;
    }

    uint32_t getPqID();

    DP_STATUS_ENUM setPQParameter(const DpPqParam &pqParam);

    DP_STATUS_ENUM invalidate();

private:

    // not thread safe
    AsyncBlitJob* requireNewJob();

    DP_STATUS_ENUM waitSubmit();
    static void* waitComplete(void* para);

    DpStream          *m_pStream;
    DpChannel         *m_pChannel;
    int32_t           m_channelID;
    DpBasicBufferPool *m_pSrcPool;
    DpBasicBufferPool *m_pDstPool;
    #if 0
    int32_t           m_srcBuffer;
    int32_t           m_srcWidth;
    int32_t           m_srcHeight;
    int32_t           m_srcYPitch;
    int32_t           m_srcUVPitch;
    DpColorFormat     m_srcFormat;
    DP_PROFILE_ENUM   m_srcProfile;
    DpSecure          m_srcSecure;
    bool              m_srcFlush;
    int32_t           m_dstBuffer;
    int32_t           m_dstWidth;
    int32_t           m_dstHeight;
    int32_t           m_dstYPitch;
    int32_t           m_dstUVPitch;
    DpColorFormat     m_dstFormat;
    DP_PROFILE_ENUM   m_dstProfile;
    DpSecure          m_dstSecure;
    bool              m_dstFlush;
    int32_t           m_cropXStart;
    int32_t           m_cropYStart;
    int32_t           m_cropWidth;
    int32_t           m_cropHeight;
    int32_t           m_targetXStart;
    int32_t           m_targetYStart;
    int32_t           m_rotation;
    bool              m_frameChange;
    bool              m_flipStatus;
    bool              m_ditherStatus;
    //Compatible to 89
    int               mTdshp;
    #endif

    uint32_t          m_PqID;
    int32_t           m_pqSupport;
    uint32_t          m_jobNum;
    DpMutex           *m_pJobMutex;
    JobList           m_jobList;
    JobList           m_waitJobList;
    DpCondition       m_jobCond;
    //bool              m_jobConfigDone;
    AsyncBlitJob      *m_pCurJob;
    BlitConfig        m_prevConfig;

    bool              m_instanceExist;
    pthread_t         m_threadHdl;

    DpSync*           m_syncObj;
};

#endif  // __DP_BLIT_STREAM_H__
