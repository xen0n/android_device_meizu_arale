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
#include <common.h>
#include <utils/common.h>
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/******************************************************************************
 *  Type Info Utility
 ******************************************************************************/
enum MTK_TYPE_ENUM {
    TYPE_UNKNOWN = -1,
    TYPE_MUINT8, //= TYPE_BYTE,  // Unsigned 8-bit integer (uint8_t)
    TYPE_MINT32, //= TYPE_INT32,  // Signed 32-bit integer (int32_t)
    TYPE_MFLOAT,// = TYPE_FLOAT,  // 32-bit float (float)
    TYPE_MINT64,// = TYPE_INT64,  // Signed 64-bit integer (int64_t)
    TYPE_MDOUBLE,// = TYPE_DOUBLE, // 64-bit float (double)
    TYPE_MRational, //= TYPE_RATIONAL,// A 64-bit fraction (camera_metadata_rational_t)
    // -- MTK -- //
    TYPE_MPoint,
    TYPE_MSize,
    TYPE_MRect,
    TYPE_IMetadata,
    TYPE_Memory,
    NUM_MTYPES    // Number of type fields
};


/******************************************************************************
 *
 ******************************************************************************/
template <typename _T> struct Type2TypeEnum{};
template <> struct Type2TypeEnum<MUINT8   >{ enum { typeEnum = TYPE_MUINT8 };   };
template <> struct Type2TypeEnum<MINT32   >{ enum { typeEnum = TYPE_MINT32 };   };
template <> struct Type2TypeEnum<MFLOAT   >{ enum { typeEnum = TYPE_MFLOAT };   };
template <> struct Type2TypeEnum<MINT64   >{ enum { typeEnum = TYPE_MINT64 };   };
template <> struct Type2TypeEnum<MDOUBLE  >{ enum { typeEnum = TYPE_MDOUBLE };  };
template <> struct Type2TypeEnum<MRational>{ enum { typeEnum = TYPE_MRational };};
template <> struct Type2TypeEnum<MPoint   >{ enum { typeEnum = TYPE_MPoint };   };
template <> struct Type2TypeEnum<MSize    >{ enum { typeEnum = TYPE_MSize };    };
template <> struct Type2TypeEnum<MRect    >{ enum { typeEnum = TYPE_MRect };    };
template <> struct Type2TypeEnum<IMetadata>{ enum { typeEnum = TYPE_IMetadata };};
template <> struct Type2TypeEnum<IMetadata::Memory>{ enum { typeEnum = TYPE_Memory };};

};

