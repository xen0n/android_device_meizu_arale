/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#ifndef _STDEXIF_H_
#define _STDEXIF_H_
//

#include <mtkcam/common.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>

using namespace android;
//
class IBaseExif;
struct exifAPP1Info_s;
struct DEBUG_CMN_INFO_S;
//
/*******************************************************************************
* Camera EXIF Command
********************************************************************************/
enum eDebugExifId {
    //
    ID_ERROR        = 0x00000001,
    //
    ID_EIS          = 0x00000002,
    ID_AAA          = 0x00000004,
    ID_ISP          = 0x00000008,
    ID_CMN          = 0x00000010,
    ID_MF           = 0x00000020,
    ID_N3D          = 0x00000040,
    ID_SENSOR       = 0x00000080,
    ID_RESERVE1     = 0x00000100,
    ID_RESERVE2     = 0x00000200,
    ID_RESERVE3     = 0x00000400,
    ID_SHAD_TABLE   = 0x00001000
    //
};

struct ExifIdMap
{
    typedef MUINT32     VAL_T;
    typedef String8     STR_T;
    //
                        ExifIdMap()
                        : m_vStr2Val()
                        , m_vVal2Str()
                    {
                        m_vStr2Val.clear();
                        m_vVal2Str.clear();
#define _ADD_STRING_VALUE_MAP_(_str_, _val_) \
                        do {\
                            m_vStr2Val.add(String8(_str_), _val_); \
                            m_vVal2Str.add(_val_, String8(_str_)); \
                        } while (0)
                        _ADD_STRING_VALUE_MAP_("ERROR",         ID_ERROR);
                        _ADD_STRING_VALUE_MAP_("AAA",           ID_AAA);
                        _ADD_STRING_VALUE_MAP_("ISP",           ID_ISP);
                        _ADD_STRING_VALUE_MAP_("COMMON",        ID_CMN);
                        _ADD_STRING_VALUE_MAP_("MF",            ID_MF);
                        _ADD_STRING_VALUE_MAP_("N3D",           ID_N3D);
                        _ADD_STRING_VALUE_MAP_("SENSOR",        ID_SENSOR);
                        _ADD_STRING_VALUE_MAP_("EIS",           ID_EIS);
                        _ADD_STRING_VALUE_MAP_("SHAD/RESERVE1", ID_RESERVE1);
                        _ADD_STRING_VALUE_MAP_("RESERVE2",      ID_RESERVE2);
                        _ADD_STRING_VALUE_MAP_("RESERVE3",      ID_RESERVE3);
                        _ADD_STRING_VALUE_MAP_("SHAD_TABLE",    ID_SHAD_TABLE);
#undef _ADD_STRING_VALUE_MAP_
                    }
    virtual             ~ExifIdMap() {}
    //
    virtual VAL_T   valueFor(STR_T const& str) const    { return m_vStr2Val.valueFor(str); }
    virtual STR_T   stringFor(VAL_T const& val) const   { return m_vVal2Str.valueFor(val); }
    //
private:
    DefaultKeyedVector<STR_T, VAL_T>    m_vStr2Val;
    DefaultKeyedVector<VAL_T, STR_T>    m_vVal2Str;
};


/*******************************************************************************
*
********************************************************************************/
struct DbgInfo
{
    mutable MUINT8*     puDbgBuf;
    MUINT32             u4BufSize;

    DbgInfo(MUINT8* _puDbgBuf = NULL, MUINT32 _u4BufSize = 0) :
        puDbgBuf(_puDbgBuf), u4BufSize(_u4BufSize)
{
    }
};

/*******************************************************************************
 *
********************************************************************************/
class StdExif : public IBaseCamExif
{
public:     ////    Constructor/Destructor
    StdExif();
    ~StdExif();

public:
    MBOOL                   init(
                                ExifParams const&    rExifParams,
                                MBOOL const enableDbgExif = false
                            );
    MBOOL                   uninit();
    MBOOL                   reset(
                                ExifParams const&    rExifParams,
                                MBOOL const enableDbgExif = false
                            );

    /* V1 API */
    virtual MBOOL           set3AEXIFInfo(EXIF_INFO_T* p3AEXIFInfo) { return true; }
    // -----

    virtual MBOOL           sendCommand(
                                MINT32      cmd,
                                MINT32      arg1 = 0,
                                MUINTPTR    arg2 = 0,
                                MINT32      arg3 = 0
                            );

    //get the size of standard exif (with out thumbnail)
    size_t                  getStdExifSize() const        { return mApp1Size; }

    //get the size of debug exif
    size_t                  getDbgExifSize() const          { return mDbgAppnSize; }

    // get the size of jpeg header (including standard exif, thumbnail, debug exif size)
    size_t                  getHeaderSize() const;

    // set maximum thumbnail size
    void                    setMaxThumbnail(size_t const thumbnailSize);

    // make standard and debug exif
    status_t                make(
                                MUINTPTR const  outputExifBuf,
                                size_t& rOutputExifSize
                            );

private:
    MUINTPTR                getBufAddr() const          { return mpOutputExifBuf; };

    size_t                  getThumbnailSize() const    { return mMaxThumbSize; }

    MBOOL                   isEnableDbgExif() const     { return mbEnableDbgExif; }

    void                    updateStdExif(exifAPP1Info_s* exifApp1Info);

    void                    updateDbgExif();

    void                    getCommonDebugInfo(DEBUG_CMN_INFO_S &a_rDebugCommonInfo);

    MBOOL                   getCamDebugInfo(MUINT8* const pDbgInfo, MUINT32 const rDbgSize, MINT32 const dbgModuleID);

    MBOOL                   appendDebugInfo(
                                MINT32 const dbgModuleID,       //  [I] debug module ID
                                MINT32 const dbgAppn,           //  [I] APPn
                                MUINT8** const ppuAppnBuf       //  [O] Pointer to APPn Buffer
                            );

    MBOOL                   appendCamDebugInfo(
                                MUINT32 const dbgAppn,          //  [I] APPn for CAM module
                                MUINT8** const puAppnBuf        //  [O] Pointer to APPn Buffer
                            );

    MINT32                  determineExifOrientation(
                                MUINT32 const   u4DeviceOrientation,
                                MBOOL const     bIsFacing,
                                MBOOL const     bIsFacingFlip = MFALSE
                            );

private:
    ////
    ExifParams                      mExifParam;
    IBaseExif*                      mpBaseExif;
    //
    MBOOL                           mbEnableDbgExif;
    size_t                          mApp1Size;
    size_t                          mDbgAppnSize;
    size_t                          mMaxThumbSize;
    MUINTPTR                        mpOutputExifBuf;
    //
    Vector<DbgInfo>                 mDbgInfo;
    KeyedVector<MUINT32, MUINT32>   mMapModuleID;
    MINT32                          mi4DbgModuleType;
    ExifIdMap*                      mpDebugIdMap;

};


#endif // _STDEXIF_H_

