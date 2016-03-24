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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_IFEATURESTREAM_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_IFEATURESTREAM_H_
//
#include "IHalPostProcPipe.h"
#include "imem_drv.h"
#include <mtkcam/iopipe/Port.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {

enum EFeatureStreamTag
{
    EFeatureStreamTag_Stream        =0,
    EFeatureStreamTag_IPRaw,   //raw in/yuv out, but pass ob, bpc, lsc
    EFeatureStreamTag_total
};


/******************************************************************************
 *
 * @struct EFeatureModule
 * @brief module enum for feature
 * @details
 *
 ******************************************************************************/
enum EFeatureModule
{
    EFeatureModule_NONE      = 0x0000,
    EFeatureModule_NR3D      = 0x0001,  //nr3d module
    EFeatureModule_STA_FEO   = 0x0010,  //statistic dma data(linke feo)
    EFeatureModule_SRZ1      = 0x0100,
    EFeatureModule_STA_LCEI  = 0x1000,  //when use statistic dma data in lcei (only for work around)
};

/******************************************************************************
 *
 * @struct NR3D
 * @brief parameter for set nr3d
 * @details
 *
 ******************************************************************************/
struct NR3D
{
public:
    MUINT32 fbcntOff_xoff;
    MUINT32 fbcntOff_yoff;
    MUINT32 fbcntSiz_xsiz;
    MUINT32 fbcntSiz_ysiz;
    MUINT32 fbCount_cnt;
    MUINT32 ctrl_lmtOutCntTh;
    MUINT32 ctrl_onEn;
    MUINT32 onOff_onOfStX;
    MUINT32 onOff_onOfStY;
    MUINT32 onSiz_onWd;
    MUINT32 onSiz_onHt;
    MUINT32 gain_weighting;
    MUINT32 vipi_offst;         //in byte
    MUINT32 vipi_readW;     //in pixel
    MUINT32 vipi_readH;     //in pixel

   NR3D()
       : fbcntOff_xoff(0x0)
       , fbcntOff_yoff(0x0)
       , fbcntSiz_xsiz(0x0)
       , fbcntSiz_ysiz(0x0)
       , fbCount_cnt(0x0)
       , ctrl_lmtOutCntTh(0x0)
       , ctrl_onEn(0x0)
       , onOff_onOfStX(0x0)
       , onOff_onOfStY(0x0)
       , onSiz_onWd(0x0)
       , onSiz_onHt(0x0)
       , gain_weighting(0x100)
       , vipi_offst(0x0)
       , vipi_readW(0x0)
       , vipi_readH(0x0)
   {
   }
};


/******************************************************************************
 *
 * @struct StaData
 * @brief additioanl structure only for statistic data. [only support one additional OUTPUT statistic data in one frame]
 * @details
 *
 ******************************************************************************/
struct StaData
{
public:
    IMEM_BUF_INFO   bufInfo;
    MINT32          w;
    MINT32          h;
    MUINT32         stride;
    MUINT32         port_idx;
    MUINT32         port_type;
    MUINT32         port_inout;

    StaData(
        MINT32     _w = 0,
        MINT32     _h = 0,
        MUINT32    _stride = 0)
    : bufInfo(IMEM_BUF_INFO())
    , w(_w)
    , h(_h)
    , stride(_stride)
    , port_idx(0)
    , port_type(0)
    , port_inout(0)
    {}
};

/******************************************************************************
 *
 * @struct SrzSize
 * @brief srz in/out size setting
 * @details
 *
 ******************************************************************************/
struct SrzSize
{
 public:
     MUINT32 in_w;
     MUINT32 in_h;
     MUINT32 out_w;
     MUINT32 out_h;

    SrzSize()
        : in_w(0x0)
        , in_h(0x0)
        , out_w(0x0)
        , out_h(0x0)
    {}
};


//FIXME, remove later
/******************************************************************************
 *
 * @struct ModuleParamInfo
 * @brief parameter for specific hw module which need be by frame set
 * @details
 *
 ******************************************************************************/
struct ModuleParamInfo
{
    EFeatureModule   eFeaturemodule;
    MVOID*           moduleStruct;
public:     //// constructors.

    ModuleParamInfo()
        : eFeaturemodule(EFeatureModule_NONE)
        , moduleStruct(NULL)
    {
    }
    //

};
//FIXME, remove later
/******************************************************************************
 *
 * @struct QStaData
 * @brief additioanl structure only for statistic data. [only support one additional OUTPUT statistic data in one frame]
 * @details
 *
 ******************************************************************************/
struct QStaData
{
    IMEM_BUF_INFO   bufInfo;
    MINT32          w;
    MINT32          h;
    MUINT32         stride;
    PortID          portID;

    QStaData(
        MINT32     _w = 0,
        MINT32     _h = 0,
        MUINT32    _stride = 0)
    : bufInfo(IMEM_BUF_INFO())
    , w(_w)
    , h(_h)
    , stride(_stride)
    , portID(PortID())
    {}
};

//FIXME, remove later
static QStaData gtempPara=QStaData();
struct QFeatureData
{
    QStaData feoData;
    android::Vector<ModuleParamInfo> mvModuleSetting;

    public:     //// constructors.

    QFeatureData()
        : feoData(gtempPara)
        , mvModuleSetting()
    {
    }
};
static QFeatureData gPara=QFeatureData();

/******************************************************************************
 *
 * @class INormalStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details
 * The data path will be Mem --> ISP--XDP --> Mem.
 *
 ******************************************************************************/
class IFeatureStream : public virtual IHalPostProcPipe
{
public:     ////                    Instantiation.

    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static  IFeatureStream*          createInstance(char const* szCallerName,EFeatureStreamTag streamTag,MUINT32 openedSensorIndex, EScenarioFormat scenFmt);

public:     ////                    Attributes.

    /**
     * @brief Query pipe capability.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    static  MBOOL                   queryCapability(IMetadata& rCapability);

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    static  char const*             pipeName() { return "PostProc::VfbStream1"; }

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const { return pipeName(); }
    /**
     * @brief En-queue a request into the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a request of QParams structure.
     * @param[in] rStaData: Reference to a request of QStaDat&a structure.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    virtual MBOOL                   enque(
                                        QParams const& rParams,
                                        QFeatureData& rFeatureData = gPara
                                    )=0;
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
                                        MINT64 i8TimeoutNs = -1,
                                        QFeatureData& rFeatureData = gPara
                                    )=0;

   /**
     * @brief Query dma position that before/behind isp resizer and other dma ports in the same crop group.
     *
     * @details
     *
     * @note
     *
     * @param[in] portIdx: port index.
     * @param[in] beforeCRZ: return the dma is before crz or not.
     * @param[in] vGroupPortIdx: vector of index of the ports that belongs to the same group with portIdx.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
   virtual MBOOL                   queryGroupMember(
                                        MINT32 portIdx,
                                        MBOOL& beforeCRZ,
                                        android::Vector<MINT32>& vGroupPortIdx
                                    )=0;


   /**
        * @brief send isp extra command
        *
        * @details
        *
        * @note
        *
        * @param[in] cmd: command
        * @param[in] arg1: arg1
        * @param[in] arg2: arg2
        *
        * @return
        * - MTRUE indicates success;
        * - MFALSE indicates failure, and an error code can be retrived by sendCommand().
        */

   virtual MBOOL                    sendCommand(
                                        MINT32 cmd,
                                        MINT32 arg1,
                                        MINT32 arg2) = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc
};  //namespace NSIoPipe
};  //namepace NSCam

////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc_FrmB {

enum EFeatureStreamTag
{
    EFeatureStreamTag_Stream        =0,
    EFeatureStreamTag_IPRaw,   //raw in/yuv out, but pass ob, bpc, lsc
    EFeatureStreamTag_total
};


/******************************************************************************
 *
 * @struct EFeatureModule
 * @brief module enum for feature
 * @details
 *
 ******************************************************************************/
enum EFeatureModule
{
    EFeatureModule_NONE      = 0x0000,
    EFeatureModule_NR3D      = 0x0001,  //nr3d module
    EFeatureModule_STA_FEO   = 0x0010,  //statistic dma data(linke feo)
    EFeatureModule_SRZ1      = 0x0100,
    EFeatureModule_STA_LCEI  = 0x1000,  //when use statistic dma data in lcei (only for work around)
};

/******************************************************************************
 *
 * @struct NR3D
 * @brief parameter for set nr3d
 * @details
 *
 ******************************************************************************/
struct NR3D
{
public:
    MUINT32 fbcntOff_xoff;
    MUINT32 fbcntOff_yoff;
    MUINT32 fbcntSiz_xsiz;
    MUINT32 fbcntSiz_ysiz;
    MUINT32 fbCount_cnt;
    MUINT32 ctrl_lmtOutCntTh;
    MUINT32 ctrl_onEn;
    MUINT32 onOff_onOfStX;
    MUINT32 onOff_onOfStY;
    MUINT32 onSiz_onWd;
    MUINT32 onSiz_onHt;
    MUINT32 gain_weighting;
    MUINT32 vipi_offst;         //in byte
    MUINT32 vipi_readW;     //in pixel
    MUINT32 vipi_readH;     //in pixel

   NR3D()
       : fbcntOff_xoff(0x0)
       , fbcntOff_yoff(0x0)
       , fbcntSiz_xsiz(0x0)
       , fbcntSiz_ysiz(0x0)
       , fbCount_cnt(0x0)
       , ctrl_lmtOutCntTh(0x0)
       , ctrl_onEn(0x0)
       , onOff_onOfStX(0x0)
       , onOff_onOfStY(0x0)
       , onSiz_onWd(0x0)
       , onSiz_onHt(0x0)
       , gain_weighting(0x100)
       , vipi_offst(0x0)
       , vipi_readW(0x0)
       , vipi_readH(0x0)
   {
   }
};


/******************************************************************************
 *
 * @struct StaData
 * @brief additioanl structure only for statistic data. [only support one additional OUTPUT statistic data in one frame]
 * @details
 *
 ******************************************************************************/
struct StaData
{
public:
    IMEM_BUF_INFO   bufInfo;
    MINT32          w;
    MINT32          h;
    MUINT32         stride;
    MUINT32         port_idx;
    MUINT32         port_type;
    MUINT32         port_inout;

    StaData(
        MINT32     _w = 0,
        MINT32     _h = 0,
        MUINT32    _stride = 0)
    : bufInfo(IMEM_BUF_INFO())
    , w(_w)
    , h(_h)
    , stride(_stride)
    , port_idx(0)
    , port_type(0)
    , port_inout(0)
    {}
};

/******************************************************************************
 *
 * @struct SrzSize
 * @brief srz in/out size setting
 * @details
 *
 ******************************************************************************/
struct SrzSize
{
 public:
     MUINT32 in_w;
     MUINT32 in_h;
     MUINT32 out_w;
     MUINT32 out_h;

    SrzSize()
        : in_w(0x0)
        , in_h(0x0)
        , out_w(0x0)
        , out_h(0x0)
    {}
};


//FIXME, remove later
/******************************************************************************
 *
 * @struct ModuleParamInfo
 * @brief parameter for specific hw module which need be by frame set
 * @details
 *
 ******************************************************************************/
struct ModuleParamInfo
{
    EFeatureModule   eFeaturemodule;
    MVOID*           moduleStruct;
public:     //// constructors.

    ModuleParamInfo()
        : eFeaturemodule(EFeatureModule_NONE)
        , moduleStruct(NULL)
    {
    }
    //

};

/******************************************************************************
 *
 * @class INormalStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details
 * The data path will be Mem --> ISP--XDP --> Mem.
 *
 ******************************************************************************/
class IFeatureStream : public virtual IHalPostProcPipe
{
public:     ////                    Instantiation.

    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static  IFeatureStream*          createInstance(char const* szCallerName,EFeatureStreamTag streamTag,MUINT32 openedSensorIndex, MBOOL isV3=false);

public:     ////                    Attributes.

    /**
     * @brief Query pipe capability.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    static  MBOOL                   queryCapability(IMetadata& rCapability);

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    static  char const*             pipeName() { return "PostProc::VfbStream1"; }

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const { return pipeName(); }
   /**
     * @brief Query dma position that before/behind isp resizer and other dma ports in the same crop group.
     *
     * @details
     *
     * @note
     *
     * @param[in] portIdx: port index.
     * @param[in] beforeCRZ: return the dma is before crz or not.
     * @param[in] vGroupPortIdx: vector of index of the ports that belongs to the same group with portIdx.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
   virtual MBOOL                   queryGroupMember(
                                        MINT32 portIdx,
                                        MBOOL& beforeCRZ,
                                        android::Vector<MINT32>& vGroupPortIdx
                                    )=0;


   /**
        * @brief send isp extra command
        *
        * @details
        *
        * @note
        *
        * @param[in] cmd: command
        * @param[in] arg1: arg1
        * @param[in] arg2: arg2
        *
        * @return
        * - MTRUE indicates success;
        * - MFALSE indicates failure, and an error code can be retrived by sendCommand().
        */

   virtual MBOOL                    sendCommand(
                                        MINT32 cmd,
                                        MINT32 arg1,
                                        MINT32 arg2) = 0;

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
};  //namespace NSIoPipe_FrmB
};  //namepace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_IFEATURESTREAM_H_

