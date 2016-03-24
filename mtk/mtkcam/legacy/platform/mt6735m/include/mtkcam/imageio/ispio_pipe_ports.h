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

/**
* @file ispio_pipe_ports.h
*
* ispio_pipe_ports Header File
*/

#ifndef _ISPIO_PIPE_PORTS_H_
#define _ISPIO_PIPE_PORTS_H_

#include <mtkcam/imageio/ispio_stddef.h>

#include <vector>
using namespace std;

namespace NSImageio
{
    namespace NSIspio
    {
        /**
             * @brief Pipe Port Type
             */
        enum EPortType
        {
            EPortType_Memory,
            EPortType_Sensor,
            EPortType_GDMA,
            EPortType_VRZ_RDMA,
            EPortType_VID_RDMA,
            EPortType_DISP_RDMA,
        };

        /**
             * @brief Pipe Port index
             */
        enum EPortIndex
        {
            EPortIndex_TG1I,    // 0
            EPortIndex_TG2I,
            EPortIndex_IMGI,
            EPortIndex_IMGCI,
            EPortIndex_NR3I,
            EPortIndex_FLKI,    // 5
            EPortIndex_LSCI,
            EPortIndex_LCEI,
            EPortIndex_VIPI,
            EPortIndex_VIP2I,
            EPortIndex_CQI,     // 10
            EPortIndex_TDRI,
            EPortIndex_IMGO,
            EPortIndex_IMG2O,
            EPortIndex_LCSO,
            EPortIndex_AAO,     // 15
            EPortIndex_NR3O,
            EPortIndex_ESFKO,
            EPortIndex_AFO,
            EPortIndex_EISO,
            EPortIndex_DISPO,   // 20
            EPortIndex_VIDO,
            EPortIndex_FDO
        };

        /**
             * @brief Pipe Sensor index
             */
        enum ESensorIndex
        {
            ESensorIndex_NONE   = 0x00,
            ESensorIndex_MAIN   = 0x01,
            ESensorIndex_SUB    = 0x02,
            ESensorIndex_ATV    = 0x04,
            ESensorIndex_MAIN_2 = 0x08,
            ESensorIndex_MAIN_3D= 0x09,
        };

        /**
             * @brief Pipe pass index
             */
        enum EPipePass
        {
            EPipePass_PASS2 = 0,
            EPipePass_PASS2B,
            EPipePass_PASS2C,
            EPipePass_PASS1_TG1,
            EPipePass_PASS1_TG2,
            EPipePass_PASS2_Phy
        };

        /**
             * @brief Pipe Port direction
             */
        enum EPortDirection
        {
            EPortDirection_In,
            EPortDirection_Out,
        };

        /**
             * @brief Pipe Port ID (Descriptor)
             */
        struct PortID
        {
        public:
            MUINT32     type    :   8;      ///<  EPortType
            MUINT32     index   :   8;      ///<  port index
            MUINT32     inout   :   1;      ///<  0:in/1:out
            MUINT32     pipePass;

        public:

            /**
                   * @brief Constructor
                   */
            PortID(
                EPortType const _eType     = EPortType_Memory,
                MUINT32 const _index       = 0,
                MUINT32 const _inout       = 0,
                MUINT32 const _pipePass= EPipePass_PASS2)
            {
                type        = _eType;
                index       = _index;
                inout       = _inout;
                pipePass    = _pipePass;
            }

        public:

            /**
                   * @brief Operations
                   */
            MUINT32 operator()() const
            {
                return  *reinterpret_cast<MUINT32 const*>(this);
            }
        };

        /**
             * @brief Pipe Port Info
             */
        struct PortInfo : public ImgInfo, public PortID, public BufInfo, public RingInfo, public SegmentInfo
        {
        public:

            /**
                   * @brief Constructor
                   */
            PortInfo()
                : ImgInfo()
                , PortID()
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo)
                : ImgInfo(_ImgInfo)
                , PortID()
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(PortID const& _PortID)
                : ImgInfo()
                , PortID(_PortID)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID)
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo)
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
                ,BufInfo(_BufInfo)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo, RingInfo const& _RingInfo )
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
                , BufInfo(_BufInfo)
                , RingInfo(_RingInfo)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo, RingInfo const& _RingInfo, SegmentInfo const& _SegmentInfo )
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
                , BufInfo(_BufInfo)
                , RingInfo(_RingInfo)
                , SegmentInfo(_SegmentInfo)
            {
            }
        };
    };  //namespace NSIspio
};  //namespace NSImageio

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio_FrmB {
namespace NSIspio_FrmB   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Pipe Port Type.
********************************************************************************/
enum EPortType
{
    EPortType_Memory,
    EPortType_Sensor,
    EPortType_GDMA,
    EPortType_VRZ_RDMA,
    EPortType_VID_RDMA,
    EPortType_DISP_RDMA,
};

/*******************************************************************************
* Pipe Port Capbility.
********************************************************************************/
enum EPortCapbility
{
    EPortCapbility_Non  =0x00,
    EPortCapbility_Cap  =0x01,
    EPortCapbility_Rcrd =0x02,
    EPortCapbility_Disp =0x03,
};

/*******************************************************************************
* Pipe Port index.
********************************************************************************/
enum EPortIndex
{
    EPortIndex_TG1I,    // 0
    EPortIndex_TG2I,
    EPortIndex_IMGI,
    EPortIndex_LSCI,
    EPortIndex_CQI,     //
    EPortIndex_TDRI,    //5
    EPortIndex_IMGO,
    EPortIndex_IMG2O,
    EPortIndex_AAO,     //
    EPortIndex_ESFKO,
    EPortIndex_WROTO,   // 10
    EPortIndex_WDMAO,
    EPortIndex_UNKNOW,
    /*temp add following to avoid build err, need remove*/
    EPortIndex_CAMSV_TG1I ,
    EPortIndex_CAMSV_TG2I ,
    EPortIndex_LCEI,
    EPortIndex_VIPI,
    EPortIndex_VIP2I,
    EPortIndex_VIP3I,
    EPortIndex_UFDI,
    EPortIndex_UFEO,
    EPortIndex_RRZO,
    EPortIndex_IMGO_D,
    EPortIndex_RRZO_D,
    EPortIndex_CAMSV_IMGO,
    EPortIndex_CAMSV_IMGO_D,
    EPortIndex_CAMSV2_IMGO,
    EPortIndex_IMG3O,
    EPortIndex_IMG3BO,
    EPortIndex_IMG3CO,
    EPortIndex_MFBO,
    EPortIndex_LCSO,        //
    EPortIndex_AFO_D,
    EPortIndex_FEO,
    EPortIndex_JPEGO,
    EPortIndex_VENC_STREAMO,
};


/*******************************************************************************
* Pipe Sensor index.
********************************************************************************/
enum ESensorIndex
{
    ESensorIndex_NONE   = 0x00,
    ESensorIndex_MAIN   = 0x01,
    ESensorIndex_SUB    = 0x02,
    ESensorIndex_ATV    = 0x04,
    ESensorIndex_MAIN_2 = 0x08,
    ESensorIndex_MAIN_3D= 0x09,
};

enum EPipePass
{
    EPipePass_PASS2 = 0,
    EPipePass_PASS2B,
    EPipePass_PASS2C,
    EPipePass_PASS1_TG1,
    EPipePass_PASS1_TG1_D,
    EPipePass_PASS2_Phy,
};
/*******************************************************************************
* Pipe Port direction.
********************************************************************************/
enum EPortDirection
{
    EPortDirection_In,
    EPortDirection_Out,
};




/*******************************************************************************
* Pipe Crop Group index.
********************************************************************************/
enum ECropGroupShiftIdx
{
    ECropGroupShiftIdx_1 = 1,
    ECropGroupShiftIdx_2,
    ECropGroupShiftIdx_3,
    ECropGroupShiftIdx_4,
    ECropGroupShiftIdx_5,
    ECropGroupShiftIdx_Num
};

enum ECropGroupIndex
{
    ECropGroupIndex_NONE = 0x01,
    ECropGroupIndex_1    = (1<<ECropGroupShiftIdx_1),
    ECropGroupIndex_2    = (1<<ECropGroupShiftIdx_2),
    ECropGroupIndex_3    = (1<<ECropGroupShiftIdx_3),
    ECropGroupIndex_4    = (1<<ECropGroupShiftIdx_4),
    ECropGroupIndex_5    = (1<<ECropGroupShiftIdx_5),
};

/*******************************************************************************
* Pipe Port Capability (Descriptor).
********************************************************************************/
struct CropPathInfo
{
public:
    MUINT32 u4NumGroup;
    MUINT32 u4CropGroup;
    vector<MINT32> PortIdxVec;

    CropPathInfo()
    {
       u4NumGroup = 0;
       u4CropGroup = 0;
       PortIdxVec.resize(0);
    }
};


/*******************************************************************************
* Pipe Port Capability (Descriptor).
********************************************************************************/
struct PortCap
{
public:
    MUINT32     u4Rotation;     //  support rotation
    MUINT32     u4Flip;         //  Flip
    MUINT32     u4MaxWidth;     //  Support Maximum Image Width
    MUINT32     u4MaxHeight;    //  Support Maximum Image Height
    MUINT32     u4MinWidth;     //  Support Minmum Image Width
    MUINT32     u4MinHeight;    //  Support Minmum Image Height
    MUINT32     u4CropGroup;     //The DMA port is belong to which crop function
    PortCap()
    {
        u4Rotation = 0;
        u4Flip = 0;
        u4MaxWidth = 0;
        u4MaxHeight = 0;
        u4MinWidth = 0;
        u4MinHeight = 0;
        u4CropGroup = 0;
    }
    PortCap(MUINT32 rot, MUINT32 flip,
              MUINT32 maxwidth, MUINT32 maxheight,
              MUINT32 minwidth, MUINT32 minheight,
              MUINT32 cropgroup)
    {
        u4Rotation = rot;
        u4Flip = flip;
        u4MaxWidth = maxwidth;
        u4MaxHeight = maxheight;
        u4MinWidth = minwidth;
        u4MinHeight = minheight;
        u4CropGroup = cropgroup;
    }

};

/*******************************************************************************
* Pipe Port ID (Descriptor).
********************************************************************************/
struct PortID
{
public:     //// fields.
    MUINT32     type     :   8;      //  EPortType
    MUINT32     index    :   8;      //  port index
    MUINT32     inout    :   1;      //  0:in/1:out
    MUINT32     capbility:   2;      // dma port capbility
    MUINT32     pipePass;
    ESoftwareScenario swscenario;
    MUINT32     tgFps;              // fps x 10
    //
public:     //// constructors.
    PortID(
        EPortType const _eType     = EPortType_Memory,
        MUINT32 const _index       = 0,
        MUINT32 const _inout       = 0,
        EPortCapbility const _capbility = EPortCapbility_Non,
        MUINT32 const _pipePass= EPipePass_PASS2,
        ESoftwareScenario const _swscenario=eSoftwareScenario_Main_Normal_Stream
    )
    {
        type        = _eType;
        index       = _index;
        inout       = _inout;
        capbility   = _capbility;
        pipePass    = _pipePass;
        swscenario  = _swscenario;
        tgFps       = 30;
    }
    //
public:     //// operations.
    MUINT32 operator()() const
    {
        return  *reinterpret_cast<MUINT32 const*>(this);
    }
};


/*******************************************************************************
* Pipe Port Info.
********************************************************************************/
struct PortInfo : public ImgInfo, public PortID, public BufInfo
{
public:     //// constructors.
    PortInfo()
        : ImgInfo()
        , PortID()
    {
    }
    //
    PortInfo(ImgInfo const& _ImgInfo)
        : ImgInfo(_ImgInfo)
        , PortID()
    {
    }
    //
    PortInfo(PortID const& _PortID)
        : ImgInfo()
        , PortID(_PortID)
    {
    }
    //
    PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID)
        : ImgInfo(_ImgInfo)
        , PortID(_PortID)
    {
    }
    //
    PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo)
        : ImgInfo(_ImgInfo)
        , PortID(_PortID)
        ,BufInfo(_BufInfo)
    {
    }
    //

};

/*******************************************************************************
* Pipe ModuleParam Info (Descriptor).
********************************************************************************/
struct ModuleParaInfo
{
    EP2Module   eP2module;
    MVOID*      moduleStruct;
public:     //// constructors.

    ModuleParaInfo()
        : eP2module(EP2Module_NONE)
        , moduleStruct(NULL)
    {
    }
    //

};

/*******************************************************************************
* IspP1PrivateData
********************************************************************************/
struct IspP1PrivateData{
    MUINT32 ResizeSrcW;
    MUINT32 ResizeSrcH;
    MUINT32 ResizeDstW;
    MUINT32 ResizeDstH;
    MUINT32 ResizeCropX;
    MUINT32 ResizeCropY;
};


/*******************************************************************************
* Pipe Package Info
********************************************************************************/
struct PipePackageInfo
{
    MINT32 burstQIdx;
    MINT32 dupCqIdx;
    MVOID* pCookie;
    vector<ModuleParaInfo> vModuleParams;
    MVOID* pTuningQue;
    IspP1PrivateData mP1PrivateData;
    MBOOL  isV3;
    MUINT32 pixIdP2;

public:     //// constructors.
    PipePackageInfo()
        : burstQIdx(0)
        , dupCqIdx(0)
        , pCookie(NULL)
        , vModuleParams()
        , pTuningQue(NULL)
        , mP1PrivateData()
        , isV3(false)
        , pixIdP2(0)
    {
        vModuleParams.resize(0);
        //
        mP1PrivateData.ResizeSrcW = 0;
        mP1PrivateData.ResizeSrcH = 0;
        mP1PrivateData.ResizeDstW = 0;
        mP1PrivateData.ResizeDstH = 0;
        mP1PrivateData.ResizeCropX = 0;
        mP1PrivateData.ResizeCropY = 0;
    }
};



////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio_FrmB
};  //namespace NSImageio_FrmB

#endif  //  _ISPIO_PIPE_PORTS_H_

