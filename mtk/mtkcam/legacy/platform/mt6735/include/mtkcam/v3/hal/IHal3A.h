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

/**
* @file IHal3A.h
* @brief Declarations of Abstraction of 3A Hal Class and Top Data Structures
*/

#ifndef __IHAL_3A_V3_H__
#define __IHAL_3A_V3_H__

#include <mtkcam/common.h>
#include <mtkcam/metadata/IMetadata.h>
#include <utils/List.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
#include <mtkcam/v3/hal/aaa_utils.h>
//#include <mtkcam/exif/IDbgInfoContainer.h>

//class IBaseCamExif;

namespace NS3Av3
{
using namespace NSCam;
using namespace android;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct MetaSet_T
{
    IMetadata appMeta;
    IMetadata halMeta;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IHal3ACb
{
public:
    virtual             ~IHal3ACb() {}

public:
    virtual void        doNotifyCb (
                            MINT32  _msgType,
                            MINTPTR _ext1,
                            MINTPTR _ext2,
                            MINTPTR _ext3
                        ) = 0;

    enum ECb_T
    {
        eID_NOTIFY_3APROC_FINISH    = 0,
        /*
            _ext1: magic number of current request
            _ext2: bit[0] OK/ERROR; bit[1] Init Ready
            _ext3: magic number of current used statistic magic number
        */

        eID_NOTIFY_READY2CAP        = 1,

        eID_NOTIFY_CURR_RESULT      = 2,
        /*
            _ext1: magic number of current result
            _ext2: metadata tag (key)
            _ext3: value
        */
        eID_MSGTYPE_NUM
    };
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief Interface of 3A Hal Class
 */
class IHal3A {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
                        IHal3A(){}
    virtual             ~IHal3A();

private: // disable copy constructor and copy assignment operator
                        IHal3A(const IHal3A&);
    IHal3A&             operator=(const IHal3A&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum E_VER
    {
        E_Camera_1 = 0,
        E_Camera_3 = 1
    };
    //
    /**
     * @brief Create instance of IHal3A
     * @param [in] eVersion, E_Camera_1, E_Camera_3
     * @param [in] i4SensorIdx
     */
    static IHal3A*      createInstance(E_VER eVersion, MINT32 const i4SensorIdx, const char* strUser);
    /**
     * @brief destroy instance of IHal3A
     */
    virtual MVOID       destroyInstance(const char* strUser) {}
    /**
     * @brief start 3A
     */
    virtual MINT32      start(MINT32 i4StartNum=0) = 0;
    /**
     * @brief stop 3A
     */
    virtual MINT32      stop() = 0;

    // interfaces for metadata processing
    /**
     * @brief Set list of controls in terms of metadata via IHal3A
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      set(const List<MetaSet_T>& controls) = 0;

    /**
     * @brief Set pass2 tuning in terms of metadata via IHal3A
     * @param [in] flowType 0 for processed raw, 1 for pure raw
     * @param [in] control MetaSet_T
     * @param [out] pRegBuf buffer address for register setting
     * @param [out] result IMetadata
     */
    virtual MINT32      setIsp(MINT32 flowType, const MetaSet_T& control, void* pRegBuf, MetaSet_T* pResult) = 0;

    /**
     * @brief Get dynamic result with specified frame ID via IHal3A
     * @param [in] frmId specified frame ID (magic number)
     * @param [out] result in terms of metadata
     */
    //virtual MINT32      get(MUINT32 frmId, IMetadata&  result) = 0;
    virtual MINT32      get(MUINT32 frmId, MetaSet_T& result) = 0;
    virtual MINT32      getCur(MUINT32 frmId, MetaSet_T& result) = 0;

    /**
     * @brief Attach callback for notifying
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     */
    virtual MINT32      attachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb) = 0;

    /**
     * @brief Dettach callback
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     */
    virtual MINT32      detachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb) = 0;


    /**
     * @brief Get delay frames via IHal3A
     * @param [out] delay_info in terms of metadata with MTK defined tags.
     */
    virtual MINT32      getDelay(IMetadata& delay_info) const= 0;

    /**
     * @brief Get delay frames via IHal3A
     * @param [in] tag belongs to control+dynamic
     * @return
     * - MINT32 delay frame.
     */
    virtual MINT32      getDelay(MUINT32 tag) const = 0;

    /**
     * @brief Get capacity of metadata list via IHal3A
     * @return
     * - MINT32 value of capacity.
     */
    virtual MINT32      getCapacity() const = 0;

    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2) = 0;

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode
     */
    virtual MVOID       setSensorMode(MINT32 i4SensorMode) = 0;

    virtual MVOID       notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg = 0) = 0;

    virtual MVOID       setFDEnable(MBOOL fgEnable) = 0;

    virtual MBOOL       setFDInfo(MVOID* prFaces) = 0;
};

}; // namespace NS3Av3

#endif
