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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_IHALPOSTPROCPIPE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_IHALPOSTPROCPIPE_H_
//
#include <utils/Vector.h>
//#include <vector>
//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include "../Port.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {

//temp for v1 82 driver
enum EScenarioFormat
{
    eScenarioFormat_RAW = 0,
    eScenarioFormat_YUV,
    eScenarioFormat_RGB,
    eScenarioFormat_JPG,
    eScenarioFormat_UNKNOWN    = 0xFFFFFFFF,
};


/******************************************************************************
 * @struct MCropRect
 *
 * @brief Cropped Rectangle.
 *
 * @param[in] p_fractional: fractional part of left-top corner in pixels.
 *
 * @param[in] p_integral: integral part of left-top corner in pixels.
 *
 * @param[in] s: size (i.e. width and height) in pixels.
 *
 ******************************************************************************/
struct MCropRect
{
    typedef int                 value_type;
    MPoint                      p_fractional;       //  left-top corner
    MPoint                      p_integral;         //  left-top corner
    MSize                       s;                  //  size: width, height

public:     ////                Instantiation.

    // we don't provide copy-ctor and copy assignment on purpose
    // because we want the compiler generated versions

    inline                      MCropRect(int _w = 0, int _h = 0)
                                    : p_fractional(0, 0), p_integral(0, 0), s(_w, _h)
                                {
                                }

    inline                      MCropRect(MPoint const& topLeft, MPoint const& bottomRight)
                                    : p_fractional(0, 0), p_integral(topLeft), s(topLeft, bottomRight)
                                {
                                }

    inline                      MCropRect(MPoint const& _p, MSize const& _s)
                                    : p_fractional(0, 0), p_integral(_p), s(_s)
                                {
                                }

    inline                      MCropRect(MRect const& _rect)
                                    : p_fractional(0, 0), p_integral(_rect.leftTop()), s(_rect.size())
                                {
                                }

};

/******************************************************************************
 * @struct MCrpRsInfo
 *
 * @brief Cropped Rectangle and Resize Information for whole pipe.
 *
 * @param[in] mCropRect: cropped rectangle.
 *
 * @param[in] mResizeDst: resized size of current dst buffer.
 *
 ******************************************************************************/
struct MCrpRsInfo
{
    MINT32    mGroupID;
    MCropRect mCropRect;
    MSize     mResizeDst;
    MCrpRsInfo(): mGroupID(0),
                  mResizeDst(mCropRect.s){}
};

/*******************************************************************************
* @struct MCropPathInfo
*
* @brief Crop path information.
*
* @param[in] mGroupNum: number of crop group.
*
* @param[in] mGroupID: crop group id.
*
* @param[in] mvPorts: dma port in each crop group.
*
********************************************************************************/
struct MCropPathInfo
{
    MUINT32                     mGroupIdx;
    android::Vector<MUINT32>    mvPorts;
public:
    MCropPathInfo()
    {
       mGroupIdx = 0;
    }
};

///////////////////////////////////////////////////
//test struct
struct ExtraParams
{
    unsigned int imgFmt;
    int imgw;
    int imgh;
    unsigned int memVA;
    unsigned int memPA;
    int memID;
    unsigned int memSize;
    int p2pxlID;
    ExtraParams(
        MUINT32     _imgFmt=0x0,
        MINT32      _imgw=0,
        MINT32      _imgh=0,
        MUINT32     _size = 0,
        MINT32      _memID = -1,
        MUINT32     _virtAddr = 0,
        MUINT32     _phyAddr = 0,
        MINT32      _p2pxlID=0)
        : imgFmt(_imgFmt)
        , imgw(_imgw)
        , imgh(_imgh)
        , memVA(_virtAddr)
        , memPA(_phyAddr)
        , memID(_memID)
        , memSize(_size)
        , p2pxlID(_p2pxlID)
        {
        }
};
///////////////////////////////////////////////////


/******************************************************************************
 *
 * @struct ModuleInfo
 * @brief parameter for specific hw module or dma statistic data which need be by frame set
 * @details
 *
 ******************************************************************************/
struct ModuleInfo
{
    MUINT32   moduleTag;
    MVOID*   moduleStruct;
public:     //// constructors.

    ModuleInfo()
        : moduleTag(0x0)
        , moduleStruct(NULL)
    {
    }
    //

};


/******************************************************************************
 * @struct Input
 *
 * @brief Pipe input parameters.
 *
 * @param[in] mPortID: The input port ID of the pipe.
 *
 * @param[in] mBuffer: A pointer to an image buffer.
 *            Callee must lock, unlock, and signal release-fence.
 *
 * @param[in] mCropRect: Input CROP is applied BEFORE transforming and resizing.
 *
 * @param[in] mTransform: ROTATION CLOCKWISE is applied AFTER FLIP_{H|V}.
 *
 ******************************************************************************/
struct  Input
{
public:     ////                    Fields (Info)
    PortID                          mPortID;
    IImageBuffer*                   mBuffer;

public:     ////                    Fields (Operations)
    MCropRect                       mCropRect;
    MINT32                          mTransform;
//test
    ExtraParams                     mExtraParams;
//
public:     ////                    Constructors.
                                    Input(
                                        PortID const&   rPortID     = PortID(),
                                        IImageBuffer*   buffer      = 0,
                                        MRect const&    cropRect    = MRect(),
                                        MINT32 const    transform   = 0,
                                        ExtraParams     rExtraParams= ExtraParams()
                                    )
                                        : mPortID(rPortID)
                                        , mBuffer(buffer)
                                        , mCropRect(cropRect)
                                        , mTransform(transform)
                                        , mExtraParams(rExtraParams)
                                    {
                                    }
};


/******************************************************************************
 * @struct Output
 *
 * @brief Pipe output parameters.
 *
 * @param[in] mPortID: The output port ID of the pipe.
 *
 * @param[in] mBuffer: A pointer to an image buffer.
 *            Output CROP is applied AFTER the transform.
 *            Callee must lock, unlock, and signal release-fence.
 *
 * @param[in/out] mTransform: ROTATION CLOCKWISE is applied AFTER FLIP_{H|V}.
 *            The result of transform must be set by the pipe if the request of
 *            transform is not supported by the pipe.
 *
 ******************************************************************************/
struct  Output
{
public:     ////                    Fields (Info)
    PortID                          mPortID;
    IImageBuffer*                   mBuffer;

public:     ////                    Fields (Operations)
    MINT32                          mTransform;
//test
    ExtraParams                     mExtraParams;
//
public:     ////                    Constructors.
                                    Output(
                                        PortID const&   rPortID     = PortID(),
                                        IImageBuffer*   buffer      = 0,
                                        MINT32 const    transform   = 0,
                                        ExtraParams     rExtraParams= ExtraParams()
                                    )
                                        : mPortID(rPortID)
                                        , mBuffer(buffer)
                                        , mTransform(transform)
                                        , mExtraParams(rExtraParams)
                                    {
                                    }
};


/******************************************************************************
 *
 * @struct QParams
 *
 * @brief Queuing parameters for the pipe.
 *      input cropping -> resizing ->
 *      output flip_{H|V} -> output rotation -> output cropping
 *
 * @param[in] mpfnCallback: a pointer to a callback function.
 *      If it is NULL, the pipe must put the result into its result queue, and
 *      then a user will get the result by deque() from the pipe later.
 *      If it is not NULL, the pipe does not put the result to its result queue.
 *      The pipe must invoke a callback with the result.
 *
 * @param[in] mpCookie: callback cookie; it shouldn't be modified by the pipe.
 *
 * @param[in] mvIn: a vector of input parameters.
 *
 * @param[in] mvOut: a vector of output parameters.
 *
 * @param[in] mCropRsInfo: a array of pipe crop/resize information.
 *
 * @param[in] mFrameNo: frame number, starting from 0.
 *
 ******************************************************************************/
struct  QParams
{
    typedef MVOID                   (*PFN_CALLBACK_T)(QParams& rParams);
    PFN_CALLBACK_T                  mpfnCallback;
    MVOID*                          mpCookie;
    MVOID*                          mpPrivaData;
    android::Vector<MVOID*>         mvPrivaData;
    android::Vector<MVOID*>         mpTuningData;         // for p2 tuning data


#if 0
    std::vector<Input>              mvIn;
    std::vector<Output>             mvOut;
#else
    android::Vector<Input>          mvIn;
    android::Vector<Output>         mvOut;
#endif
    android::Vector<MCrpRsInfo>     mvCropRsInfo;
    MUINT32                         mFrameNo;
    android::Vector<MUINT32>        mvMagicNo;
    MBOOL                           mDequeSuccess;
    android::Vector<ModuleInfo>     mvModuleData;
                                    //
                                    QParams()
                                        : mpfnCallback(NULL)
                                        , mpCookie(NULL)
                                        , mpPrivaData(NULL)
                                        , mvPrivaData()
                                        , mvIn()
                                        , mvOut()
                                        , mFrameNo(0)
                                        , mvMagicNo()
                                        , mDequeSuccess(MFALSE)
                                        , mvModuleData()
                                    {
                                    }
};


/******************************************************************************
 *
 * @class IHalPostProcPipe
 * @brief Post-Processing Pipe Interface.
 * @details
 * The data path will be Mem --> ISP--XDP --> Mem.
 *
 ******************************************************************************/
class IHalPostProcPipe
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor.

    /**
     * @brief Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalPostProcPipe() {};

public:     ////                    Instantiation.

    /**
     * @brief destroy the pipe instance
     *
     * @details
     *
     * @note
     */
    virtual MVOID                   destroyInstance(char const* szCallerName)   = 0;

    /**
     * @brief init the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   init()                                      = 0;

    /**
     * @brief uninit the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   uninit()                                    = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * - A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const                         = 0;

    /**
     * @brief get the last error code
     *
     * @details
     *
     * @note
     *
     * @return
     * - The last error code
     *
     */
    virtual MERROR                  getLastErrorCode() const                    = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * @brief En-queue a request into the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a request of QParams structure.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    virtual MBOOL                   enque(
                                        QParams const& rParams
                                    )                                           = 0;

    /**
     * @brief De-queue a result from the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a result of QParams structure.
     *
     * @param[in] i8TimeoutNs: timeout in nanoseconds \n
     *      If i8TimeoutNs > 0, a timeout is specified, and this call will \n
     *      be blocked until a result is ready. \n
     *      If i8TimeoutNs = 0, this call must return immediately no matter \n
     *      whether any buffer is ready or not. \n
     *      If i8TimeoutNs = -1, an infinite timeout is specified, and this call
     *      will block forever.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   deque(
                                        QParams& rParams,
                                        MINT64 i8TimeoutNs = -1
                                    )                                           = 0;
   /**
     * @brief notify start video record for slow motion support.
     *
     * @param[in] wd: width
     * @param[in] ht: height
     *
     * @details
     *
     * @note
     *
     * @return
     *      - [true]
     */
    virtual MBOOL                    startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps=120)=0;
    /**
     * @brief notify stop video record for slow motion support.
     *
     * @details
     *
     * @note
     *
     * @return
     *      - [true]
     */
    virtual MBOOL                    stopVideoRecord()=0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc
};  //namespace NSIoPipe
};  //namespace NSCam

//////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc_FrmB {

/******************************************************************************
 * @struct MCropRect
 *
 * @brief Cropped Rectangle.
 *
 * @param[in] p_fractional: fractional part of left-top corner in pixels.
 *
 * @param[in] p_integral: integral part of left-top corner in pixels.
 *
 * @param[in] s: size (i.e. width and height) in pixels.
 *
 ******************************************************************************/
struct MCropRect
{
    typedef int                 value_type;
    MPoint                      p_fractional;       //  left-top corner
    MPoint                      p_integral;         //  left-top corner
    MSize                       s;                  //  size: width, height

public:     ////                Instantiation.

    // we don't provide copy-ctor and copy assignment on purpose
    // because we want the compiler generated versions

    inline                      MCropRect(int _w = 0, int _h = 0)
                                    : p_fractional(0, 0), p_integral(0, 0), s(_w, _h)
                                {
                                }

    inline                      MCropRect(MPoint const& topLeft, MPoint const& bottomRight)
                                    : p_fractional(0, 0), p_integral(topLeft), s(topLeft, bottomRight)
                                {
                                }

    inline                      MCropRect(MPoint const& _p, MSize const& _s)
                                    : p_fractional(0, 0), p_integral(_p), s(_s)
                                {
                                }

    inline                      MCropRect(MRect const& _rect)
                                    : p_fractional(0, 0), p_integral(_rect.leftTop()), s(_rect.size())
                                {
                                }

};

/******************************************************************************
 * @struct MCrpRsInfo
 *
 * @brief Cropped Rectangle and Resize Information for whole pipe.
 *
 * @param[in] mCropRect: cropped rectangle.
 *
 * @param[in] mResizeDst: resized size of current dst buffer.
 *
 ******************************************************************************/
struct MCrpRsInfo
{
    MINT32    mGroupID;
    MCropRect mCropRect;
    MSize     mResizeDst;
    MCrpRsInfo(): mGroupID(0),
                  mResizeDst(mCropRect.s){}
};

/*******************************************************************************
* @struct MCropPathInfo
*
* @brief Crop path information.
*
* @param[in] mGroupNum: number of crop group.
*
* @param[in] mGroupID: crop group id.
*
* @param[in] mvPorts: dma port in each crop group.
*
********************************************************************************/
struct MCropPathInfo
{
    MUINT32                     mGroupIdx;
    android::Vector<MUINT32>    mvPorts;
public:
    MCropPathInfo()
    {
       mGroupIdx = 0;
    }
};

///////////////////////////////////////////////////
//test struct
struct ExtraParams
{
    unsigned int imgFmt;
    int imgw;
    int imgh;
    unsigned int memVA;
    unsigned int memPA;
    int memID;
    unsigned int memSize;
    int p2pxlID;
    ExtraParams(
        MUINT32     _imgFmt=0x0,
        MINT32      _imgw=0,
        MINT32      _imgh=0,
        MUINT32     _size = 0,
        MINT32      _memID = -1,
        MUINT32     _virtAddr = 0,
        MUINT32     _phyAddr = 0,
        MINT32      _p2pxlID=0)
        : imgFmt(_imgFmt)
        , imgw(_imgw)
        , imgh(_imgh)
        , memVA(_virtAddr)
        , memPA(_phyAddr)
        , memID(_memID)
        , memSize(_size)
        , p2pxlID(_p2pxlID)
        {
        }
};
///////////////////////////////////////////////////


/******************************************************************************
 *
 * @struct ModuleInfo
 * @brief parameter for specific hw module or dma statistic data which need be by frame set
 * @details
 *
 ******************************************************************************/
struct ModuleInfo
{
    MUINT32   moduleTag;
    MVOID*   moduleStruct;
public:     //// constructors.

    ModuleInfo()
        : moduleTag(0x0)
        , moduleStruct(NULL)
    {
    }
    //

};


/******************************************************************************
 * @struct Input
 *
 * @brief Pipe input parameters.
 *
 * @param[in] mPortID: The input port ID of the pipe.
 *
 * @param[in] mBuffer: A pointer to an image buffer.
 *            Callee must lock, unlock, and signal release-fence.
 *
 * @param[in] mCropRect: Input CROP is applied BEFORE transforming and resizing.
 *
 * @param[in] mTransform: ROTATION CLOCKWISE is applied AFTER FLIP_{H|V}.
 *
 ******************************************************************************/
struct  Input
{
public:     ////                    Fields (Info)
    PortID                          mPortID;
    IImageBuffer*                   mBuffer;

public:     ////                    Fields (Operations)
    MCropRect                       mCropRect;
    MINT32                          mTransform;
//test
    ExtraParams                     mExtraParams;
//
public:     ////                    Constructors.
                                    Input(
                                        PortID const&   rPortID     = PortID(),
                                        IImageBuffer*   buffer      = 0,
                                        MRect const&    cropRect    = MRect(),
                                        MINT32 const    transform   = 0,
                                        ExtraParams     rExtraParams= ExtraParams()
                                    )
                                        : mPortID(rPortID)
                                        , mBuffer(buffer)
                                        , mCropRect(cropRect)
                                        , mTransform(transform)
                                        , mExtraParams(rExtraParams)
                                    {
                                    }
};


/******************************************************************************
 * @struct Output
 *
 * @brief Pipe output parameters.
 *
 * @param[in] mPortID: The output port ID of the pipe.
 *
 * @param[in] mBuffer: A pointer to an image buffer.
 *            Output CROP is applied AFTER the transform.
 *            Callee must lock, unlock, and signal release-fence.
 *
 * @param[in/out] mTransform: ROTATION CLOCKWISE is applied AFTER FLIP_{H|V}.
 *            The result of transform must be set by the pipe if the request of
 *            transform is not supported by the pipe.
 *
 ******************************************************************************/
struct  Output
{
public:     ////                    Fields (Info)
    PortID                          mPortID;
    IImageBuffer*                   mBuffer;

public:     ////                    Fields (Operations)
    MINT32                          mTransform;
//test
    ExtraParams                     mExtraParams;
//
public:     ////                    Constructors.
                                    Output(
                                        PortID const&   rPortID     = PortID(),
                                        IImageBuffer*   buffer      = 0,
                                        MINT32 const    transform   = 0,
                                        ExtraParams     rExtraParams= ExtraParams()
                                    )
                                        : mPortID(rPortID)
                                        , mBuffer(buffer)
                                        , mTransform(transform)
                                        , mExtraParams(rExtraParams)
                                    {
                                    }
};


/******************************************************************************
 *
 * @struct QParams
 *
 * @brief Queuing parameters for the pipe.
 *      input cropping -> resizing ->
 *      output flip_{H|V} -> output rotation -> output cropping
 *
 * @param[in] mpfnCallback: a pointer to a callback function.
 *      If it is NULL, the pipe must put the result into its result queue, and
 *      then a user will get the result by deque() from the pipe later.
 *      If it is not NULL, the pipe does not put the result to its result queue.
 *      The pipe must invoke a callback with the result.
 *
 * @param[in] mpCookie: callback cookie; it shouldn't be modified by the pipe.
 *
 * @param[in] mvIn: a vector of input parameters.
 *
 * @param[in] mvOut: a vector of output parameters.
 *
 * @param[in] mCropRsInfo: a array of pipe crop/resize information.
 *
 * @param[in] mFrameNo: frame number, starting from 0.
 *
 ******************************************************************************/
struct  QParams
{
    typedef MVOID                   (*PFN_CALLBACK_T)(QParams& rParams);
    PFN_CALLBACK_T                  mpfnCallback;
    MVOID*                          mpCookie;
    MVOID*                          mpPrivaData;                    //CHRISTOPHER: remove later
    android::Vector<MVOID*>         mvPrivaData;
    android::Vector<MVOID*>         mpTuningData;         // for p2 tuning data
    //
    // passed from pass1 (HRZ information)
    android::Vector<MRect>          mvP1SrcCrop;
    android::Vector<MSize>          mvP1Dst;
    android::Vector<MRect>          mvP1DstCrop;
    //
#if 0
    std::vector<Input>              mvIn;
    std::vector<Output>             mvOut;
#else
    android::Vector<Input>          mvIn;
    android::Vector<Output>         mvOut;
#endif
    android::Vector<MCrpRsInfo>     mvCropRsInfo;
    MUINT32                         mFrameNo;                       //CHRISTOPHER: remove later
    android::Vector<MUINT32>        mvMagicNo;
    MBOOL                           mDequeSuccess;
    android::Vector<ModuleInfo>     mvModuleData;
                                    //
                                    QParams()
                                        : mpfnCallback(NULL)
                                        , mpCookie(NULL)
                                        , mpPrivaData(NULL)
                                        , mvPrivaData()
                                        , mpTuningData()
                                        , mvP1SrcCrop()
                                        , mvP1Dst()
                                        , mvP1DstCrop()
                                        , mvIn()
                                        , mvOut()
                                        , mFrameNo(0)
                                        , mvMagicNo()
                                        , mDequeSuccess(MFALSE)
                                        , mvModuleData()
                                    {
                                    }
};


/******************************************************************************
 *
 * @class IHalPostProcPipe
 * @brief Post-Processing Pipe Interface.
 * @details
 * The data path will be Mem --> ISP--XDP --> Mem.
 *
 ******************************************************************************/
class IHalPostProcPipe
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor.

    /**
     * @brief Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalPostProcPipe() {};

public:     ////                    Instantiation.

    /**
     * @brief destroy the pipe instance
     *
     * @details
     *
     * @note
     */
    virtual MVOID                   destroyInstance(char const* szCallerName)   = 0;

    /**
     * @brief init the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   init()                                      = 0;

    /**
     * @brief uninit the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   uninit()                                    = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * - A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const                         = 0;

    /**
     * @brief get the last error code
     *
     * @details
     *
     * @note
     *
     * @return
     * - The last error code
     *
     */
    virtual MERROR                  getLastErrorCode() const                    = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * @brief En-queue a request into the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a request of QParams structure.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    virtual MBOOL                   enque(
                                        QParams const& rParams
                                    )                                           = 0;

    /**
     * @brief De-queue a result from the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a result of QParams structure.
     *
     * @param[in] i8TimeoutNs: timeout in nanoseconds \n
     *      If i8TimeoutNs > 0, a timeout is specified, and this call will \n
     *      be blocked until a result is ready. \n
     *      If i8TimeoutNs = 0, this call must return immediately no matter \n
     *      whether any buffer is ready or not. \n
     *      If i8TimeoutNs = -1, an infinite timeout is specified, and this call
     *      will block forever.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   deque(
                                        QParams& rParams,
                                        MINT64 i8TimeoutNs = -1
                                    )                                           = 0;
   /**
     * @brief notify start video record for slow motion support.
     *
     * @param[in] wd: width
     * @param[in] ht: height
     *
     * @details
     *
     * @note
     *
     * @return
     *      - [true]
     */
    virtual MBOOL                    startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps=120)=0;
    /**
     * @brief notify stop video record for slow motion support.
     *
     * @details
     *
     * @note
     *
     * @return
     *      - [true]
     */
    virtual MBOOL                    stopVideoRecord()=0;
    /**
     * @brief get a buffer for tuning data provider
     *
     * @details
     *
     * @note
     *
     * @param[out] size: get the tuning buffer size
     * @param[out] pTuningQueBuf : get the tuning buffer point, user need to map to isp_reg_t(isp_reg.h) before use it.
     *
     * @return
     * - MTRUE  indicates success;
     * - MFALSE indicates failure;
     */
    virtual MBOOL                   deTuningQue(unsigned int& size, void* &pTuningQueBuf) = 0;
    /**
     * @brief return the buffer which be got before
     *
     * @details
     *
     * @note
     *
     * @param[in] pTuningQueBuf : return the pTuningQueBuf
     *
     * @return
     * - MTRUE  indicates success;
     * - MFALSE indicates failure;
     */
    virtual MBOOL                   enTuningQue(void* pTuningQueBuf) = 0;



};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc_FrmB
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_IHALPOSTPROCPIPE_H_

