#ifndef __DP_BLIT_STREAM_H__
#define __DP_BLIT_STREAM_H__

#include "DpDataType.h"

enum
{
    DP_BLIT_GENERAL_USER = 0,
    DP_BLIT_HWC0 = 0,
    DP_BLIT_GPU = 1,
    DP_BLIT_HWC1 = 1,
    DP_BLIT_HWC2 = 2,
    DP_BLIT_HWC3 = 3,
    DP_BLIT_HWC_120FPS = 4
};


class DpBlitStream
{
public:
    static bool queryHWSupport(uint32_t srcWidth,
                               uint32_t srcHeight,
                               uint32_t dstWidth,
                               uint32_t dstHeight,
                               int32_t  Orientation = 0);

    DpBlitStream();

    ~DpBlitStream();

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

    DP_STATUS_ENUM setRotate(int32_t rotation)
    {
        if (m_rotation != rotation)
        {
            m_rotation = rotation;
            m_frameChange = true;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    //Compatible to 89
    DP_STATUS_ENUM setFlip(int flip)
    {
        if (m_flipStatus != flip)
        {
            m_flipStatus = flip ? true : false;
            m_frameChange = true;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setOrientation(uint32_t transform);

    DP_STATUS_ENUM setTdshp(int gain)
    {
        if (mTdshp != gain)
        {
            mTdshp        = gain;
            m_frameChange = true;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    uint32_t getPqID();

    DP_STATUS_ENUM setPQParameter(const DpPqParam &pParam);

    DP_STATUS_ENUM setDither(bool enDither)
    {
        if (m_ditherStatus != enDither)
        {
            m_ditherStatus = enDither;
            m_frameChange  = true;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setUser(uint32_t eID = 0);

    DP_STATUS_ENUM invalidate();

    DP_STATUS_ENUM pq_process();

private:
    DpStream          *m_pStream;
    DpChannel         *m_pChannel;
    int32_t           m_channelID;
    DpBasicBufferPool *m_pSrcPool;
    DpBasicBufferPool *m_pDstPool;
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
    DpStream          *m_pPqStream;
    DpChannel         *m_pPqChannel;
    DpAutoBufferPool  *m_pPqPool;
    int32_t           m_pqBuffer;
    int32_t           m_cropXStart;
    int32_t           m_cropYStart;
    int32_t           m_cropWidth;
    int32_t           m_cropHeight;
    int32_t           m_cropSubPixelX;
    int32_t           m_cropSubPixelY;
    int32_t           m_targetXStart;
    int32_t           m_targetYStart;
    int32_t           m_rotation;
    bool              m_frameChange;
    bool              m_flipStatus;
    bool              m_ditherStatus;
    uint32_t          m_userID;
    DpPqConfig        m_PqConfig;
    uint32_t          m_PqID;
    int32_t           m_pqSupport;
    uint32_t          m_engFlag;
    //Compatible to 89
    int               mTdshp;
};

#endif  // __DP_BLIT_STREAM_H__
