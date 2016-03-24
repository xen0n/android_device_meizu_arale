#ifndef __DP_VENC_STREAM_H__
#define __DP_VENC_STREAM_H__

#include "DpDataType.h"
#include <sys/time.h>
#include <vector>

using namespace std;

#define ISP_MAX_OUTPUT_PORT_NUM     4
#define YUV_MAX_BUFFER_NUM          10

class DpStream;
class DpChannel;
class DpBasicBufferPool;
class DpMemory;

class DpMutex;

struct ISP_TPIPE_CONFIG_STRUCT;

typedef std::vector<int32_t> BufIDList;
typedef std::vector<DpJobID> JobIDList;
typedef std::vector<struct timeval> BufTimeList;


class DpVEncStream
{
public:
    enum ISPStreamType
    {
        ISP_IC_STREAM,
        ISP_VR_STREAM,
        ISP_ZSD_STREAM,
        ISP_IP_STREAM,
        ISP_VSS_STREAM,
        ISP_ZSD_SLOW_STREAM,
        ISP_CC_STREAM = ISP_VR_STREAM
    };

    enum ISPStreamHint
    {
        HINT_ISP_NEW_FRAME = 0x0,       // need to do tpipe calculation
        HINT_ISP_TPIPE_NO_CHANGE = 0x1, // can skip tpipe calculation
        HINT_ISP_FRAME_MODE = 0x2,      // ISP-only frame mode
    };

    DpVEncStream(ISPStreamType type);

    ~DpVEncStream();

    DP_STATUS_ENUM startVideoRecord(uint32_t width, uint32_t height, uint32_t framerate = 120);

    DP_STATUS_ENUM stopVideoRecord();

    /* Venc Modify + */

#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
    DP_STATUS_ENUM startVideoRecord_HEVC(uint32_t width, uint32_t height, uint32_t framerate);
    DP_STATUS_ENUM stopVideoRecord_HEVC();
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
    DP_STATUS_ENUM startVideoRecord_H264(uint32_t width, uint32_t height, uint32_t framerate);
    DP_STATUS_ENUM stopVideoRecord_H264();
#endif
    /* Venc Modify - */

    DP_STATUS_ENUM queueSrcBuffer(void     *pVA,
                                  uint32_t size);

    DP_STATUS_ENUM queueSrcBuffer(void     *pVA,
                                  uint32_t MVA,
                                  uint32_t size);

    DP_STATUS_ENUM queueSrcBuffer(void     **pVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueSrcBuffer(uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueSrcBuffer(void     **pVAList,
                                  uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    // for ION file descriptor
    DP_STATUS_ENUM queueSrcBuffer(int32_t  fileDesc,
                                  uint32_t *sizeList,
                                  uint32_t planeNum);

    DP_STATUS_ENUM dequeueSrcBuffer();

    /**
     * Description:
     *     Set source buffer configuration information
     *
     * Parameter:
     *     srcFormat: Source buffer format
     *     srcWidth: Source buffer width
     *     srcHeight: Source buffer height
     *     srcPitch: Source buffer pitch
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setSrcConfig(DpColorFormat srcFormat,
                                int32_t       srcWidth,
                                int32_t       srcHeight,
                                int32_t       srcPitch,
                                bool          doFlush = true);


    DP_STATUS_ENUM setSrcConfig(int32_t           width,
                                int32_t           height,
                                int32_t           YPitch,
                                int32_t           UVPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpRect            *pROI   = 0,
                                bool              doFlush = true);

    /**
     * Description:
     *     Set source buffer crop window information
     *
     * Parameter:
     *     XStart: Source crop X start coordinate
     *     XSubpixel: Source crop X subpixel coordinate
     *     YStart: Source crop Y start coordinate
     *     YSubpixel: Source crop Y subpixel coordinate
     *     cropWidth: Source crop window width
     *     cropHeight: Source crop window height
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setSrcCrop(int32_t XStart,
                              int32_t XSubpixel,
                              int32_t YStart,
                              int32_t YSubpixel,
                              int32_t cropWidth,
                              int32_t cropHeight);

    DP_STATUS_ENUM setSrcCrop(int32_t portIndex,
                              int32_t XStart,
                              int32_t XSubpixel,
                              int32_t YStart,
                              int32_t YSubpixel,
                              int32_t cropWidth,
                              int32_t cropHeight);

    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  void     **pVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  void     **pVAList,
                                  uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    // for ION file descriptor
    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  int32_t  fileDesc,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    /**
     * Description:
     *     Acquire a destination buffer for HW processing
     *
     * Parameter:
     *     port: Output port index
     *     base: buffer virtual base address
     *     waitBuf: true for the buffer is ready;
     *              else return immediately
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM dequeueDstBuffer(int32_t portIndex,
                                    void    **pVABase,
                                    bool    waitBuf = true);

    /**
     * Description:
     *     Set destination buffer configuration information
     *
     * Parameter:
     *     format: Destination buffer format
     *     width: Destination buffer width
     *     height: Destination buffer height
     *     pitch: Destination buffer pitch
     *     port: Destination port number
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setDstConfig(int32_t       portIndex,
                                DpColorFormat dstFormat,
                                int32_t       dstWidth,
                                int32_t       dstHeight,
                                int32_t       dstPitch,
                                bool          doFlush = true);


    DP_STATUS_ENUM setDstConfig(int32_t           portIndex,
                                int32_t           width,
                                int32_t           height,
                                int32_t           YPitch,
                                int32_t           UVPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpRect            *pROI   = 0,
                                bool              doFlush = true);


    /**
     * Description:
     *     Set port desired rotation angles
     *
     * Parameter:
     *     portIndex: Port index number
     *     rotation: Desired rotation angle
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setRotation(int32_t portIndex,
                               int32_t rotation);

    /**
     * Description:
     *     Set port desired flip status
     *
     * Parameter:
     *    portIndex: Port index number
     *    flipStatus: Desired flip status
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setFlipStatus(int32_t portIndex,
                                 bool    flipStatus);


    /**
     * Description:
     *     Set extra parameter for ISP
     *
     * Parameter:
     *     extraPara: ISP extra parameters
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setParameter(ISP_TPIPE_CONFIG_STRUCT &extraPara, uint32_t hint = 0);

    DP_STATUS_ENUM setPortType(int32_t        portIndex,
                               uint32_t       portType = 0,
                               DpVEnc_Config* VEnc_cfg = NULL);

    DP_STATUS_ENUM setSharpness(int32_t portIndex,
                                int32_t gain)
    {
        if (m_sharpness[portIndex] != gain)
        {
            m_sharpness[portIndex] = gain;
            m_frameChange = true;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setDither(int32_t portIndex,
                             bool    enDither)
    {
        if (m_ditherStatus[portIndex] != enDither)
        {
            m_ditherStatus[portIndex] = enDither;
            m_frameChange = true;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setPortMatrix(int32_t portIndex, DpColorMatrix *matrix);

    uint32_t getPqID();

    DP_STATUS_ENUM setPQParameter(int32_t portIndex, const DpPqParam &pParam);

    /**
     * Description:
     *     Start ISP stream processing (non-blocking)
     *
     * Parameter:
     *     None
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM startStream();

    /**
     * Description:
     *     Stop ISP stream processing
     *
     * Parameter:
     *     None
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM stopStream();

    DP_STATUS_ENUM dequeueFrameEnd();

    DP_STATUS_ENUM getJPEGFilesize(uint32_t* filesize);

private:
    ISPStreamType           m_streamType;
    int32_t                 m_currentMode;
    bool                    m_frameChange;
    DpStream                *m_pStream;
    DpChannel               *m_pChannel;
    int32_t                 m_channelID;

    DpMutex                 *m_pListMutex;
    DpMutex                 *m_pBufferMutex;
    // Source information
    DpBasicBufferPool       *m_pSrcPool;
    int32_t                 m_srcBuffer;
    BufIDList               m_srcBufferList;
    BufTimeList             m_srcBufferStartTimeList;

    JobIDList               m_jobIDList;
    JobIDList               m_frameDoneList;

    DpColorFormat           m_srcFormat;
    int32_t                 m_srcWidth;
    int32_t                 m_srcHeight;
    int32_t                 m_srcYPitch;
    int32_t                 m_srcUVPitch;
    bool                    m_cropChange;
    bool                    m_srcFlush;
    DpSecure                m_srcSecure;
    DP_PROFILE_ENUM         m_srcProfile;

    // Destination information
    DpBasicBufferPool       *m_pDstPool[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_dstBuffer[ISP_MAX_OUTPUT_PORT_NUM];
    BufIDList               m_dstBufferList[ISP_MAX_OUTPUT_PORT_NUM];
    DpColorFormat           m_dstFormat[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_dstPlane[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_dstWidth[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_dstHeight[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_dstYPitch[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_dstUVPitch[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_rotation[ISP_MAX_OUTPUT_PORT_NUM];
    bool                    m_flipStatus[ISP_MAX_OUTPUT_PORT_NUM];
    bool                    m_dstEnable[ISP_MAX_OUTPUT_PORT_NUM];
    bool                    m_dstWasEnabled[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_sharpness[ISP_MAX_OUTPUT_PORT_NUM];
    bool                    m_ditherStatus[ISP_MAX_OUTPUT_PORT_NUM];
    bool                    m_dstFlush[ISP_MAX_OUTPUT_PORT_NUM];
    uint32_t                m_dstPortType[ISP_MAX_OUTPUT_PORT_NUM];
    DP_PROFILE_ENUM         m_dstProfile[ISP_MAX_OUTPUT_PORT_NUM];
    DpVEnc_Config           m_VEnc_cfg;

    // Crop information
    int32_t                 m_srcXStart;
    int32_t                 m_srcXSubpixel;
    int32_t                 m_srcYStart;
    int32_t                 m_srcYSubpixel;
    int32_t                 m_cropWidth;
    int32_t                 m_cropHeight;

    int32_t                 m_multiSrcXStart[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_multiSrcXSubpixel[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_multiSrcYStart[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_multiSrcYSubpixel[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_multiCropWidth[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_multiCropHeight[ISP_MAX_OUTPUT_PORT_NUM];

    uint32_t                m_jpegEnc_filesize;

    ISP_TPIPE_CONFIG_STRUCT *m_pParameter;

    DpPqConfig              m_PqConfig[ISP_MAX_OUTPUT_PORT_NUM];
    uint32_t                m_PqID[ISP_MAX_OUTPUT_PORT_NUM];
    DpColorMatrix           *m_pDstMatrix[ISP_MAX_OUTPUT_PORT_NUM];

    DpMemory                *m_yuvBuffer[YUV_MAX_BUFFER_NUM];
    uint64_t                m_configFrameCount;
    uint64_t                m_dequeueFrameCount;

    uint32_t                m_encodeFrameWidth;
    uint32_t                m_encodeFrameHeight;

    uint32_t                m_PABuffer[MAX_NUM_READBACK_REGS];
    uint32_t                m_ValueBuffer[MAX_NUM_READBACK_REGS];
    uint32_t                m_numPABuffer;
    int32_t                 m_pqSupport;

    DP_STATUS_ENUM waitSubmit();
    DP_STATUS_ENUM waitComplete();
};

#endif  // __DP_VENC_STREAM_H__
