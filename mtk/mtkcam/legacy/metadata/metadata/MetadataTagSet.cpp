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
#include <Log.h>
#include <utils/String8.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/KeyedVector.h>
using namespace android;
//
#include <common.h>
#include <utils/common.h>
using namespace NSCam;
//
#include <metadata/IMetadataTagSet.h>
#include <metadata/mtk_metadata_types.h>
#include <metadata/client/TagMap.h>
/******************************************************************************
 *
 ******************************************************************************/
class DefaultMetadataTagSetImp : public IDefaultMetadataTagSet
{
public:
             DefaultMetadataTagSetImp();
    virtual ~DefaultMetadataTagSetImp() {}

public:
    virtual IMetadataTagSet const& getTagSet() const;

protected:
    IMetadataTagSet mData;
};


/******************************************************************************
 *
 ******************************************************************************/
DefaultMetadataTagSetImp::
DefaultMetadataTagSetImp()
{

#define _IMP_SECTION_INFO_(...)
#undef  _IMP_TAG_INFO_

#define _IMP_TAG_INFO_(_tag_, _type_, _name_) \
    mData.addTag(_tag_, _name_, Type2TypeEnum<_type_>::typeEnum);

#include <metadata/client/mtk_metadata_tag_info.inl>

#undef  _IMP_TAG_INFO_

#undef _IMP_TAGCONVERT_
#define _IMP_TAGCONVERT_(_android_tag_, _mtk_tag_) \
    mData.addTagMap(_android_tag_, _mtk_tag_);
    #if (PLATFORM_SDK_VERSION >= 21)
    ADD_ALL_MEMBERS;
    #endif

#undef _IMP_TAGCONVERT_
}


IDefaultMetadataTagSet*
IDefaultMetadataTagSet::
singleton()
{
    static DefaultMetadataTagSetImp _singleton;
    return &_singleton;
}


IMetadataTagSet const&
DefaultMetadataTagSetImp::
getTagSet() const
{
   return mData;
}


/******************************************************************************
 *
 ******************************************************************************/
class TagInfo : public virtual RefBase
{
public:
    MUINT32                 mTag;
    String8                 mName;
    MINT32                  mTypeEnum;

    TagInfo(MUINT32 const tag,
            char const* name,
            MINT32 const typeEnum)
        : mTag(tag)
        , mName(name)
        , mTypeEnum(typeEnum)
    {
    }

    MUINT32                 tag() const         { return mTag; }
    char const*             name() const        { return mName.string(); }
    MINT32                  typeEnum() const    { return mTypeEnum; }
};


/******************************************************************************
 *
 ******************************************************************************/
class IMetadataTagSet::Implementor
{
public:     ////                        Instantiation.
    virtual                 ~Implementor(){}
                             Implementor()
                                : mTagInfoMap()
                                , android_to_mtk(-1)
                                , mtk_to_android(-1)
                                {}

    Implementor&             operator=(Implementor const& other);
                             Implementor(Implementor const& other);

    virtual MINT32           getType(MUINT32 tag) const
                             {
                                sp<TagInfo> p = mTagInfoMap.valueFor(tag);
                                if  ( p != 0 )
                                {
                                    return  p->typeEnum();
                                }
                                //
                                return TYPE_UNKNOWN;
                             }


    virtual char const*      getName(unsigned int tag) const
                             {
                                  sp<TagInfo> p = mTagInfoMap.valueFor(tag);
                                  if  ( p != 0 )
                                  {
                                      return  p->name();
                                  }
                                  return  NULL;

                             }

    virtual MVOID            addTag(MUINT32 tag, char const* name, MINT32 typeEnum)
                             {
                                  mTagInfoMap.add(tag, new TagInfo(tag, name, typeEnum));
                             }

    virtual MVOID            addTagMap(MUINT32 androidTag, MUINT32 MtkTag) {
                                 android_to_mtk.add(androidTag, MtkTag);
                                 mtk_to_android.add(MtkTag, androidTag);
                             }


    virtual MUINT32          getMtkTag(MUINT32 tag)
                             {
                                 if(tag >= (MUINT32)VENDOR_SECTION_START) {
                                    return tag;
                                 }
                                 return android_to_mtk.valueFor((MUINT32)tag);
                             }

    virtual MUINT32          getAndroidTag(MUINT32 tag)
                             {
                                 if(tag >= (MUINT32)VENDOR_SECTION_START) {
                                    return tag;
                                 }
                                 return mtk_to_android.valueFor((MUINT32)tag);
                             }

protected:
    DefaultKeyedVector<MUINT32, sp<TagInfo> > mTagInfoMap;
    DefaultKeyedVector<MUINT32, MUINT32> android_to_mtk;
    DefaultKeyedVector<MUINT32, MUINT32> mtk_to_android;

};


/******************************************************************************
 *
 ******************************************************************************/
IMetadataTagSet::Implementor::
Implementor(IMetadataTagSet::Implementor const& other)
    : mTagInfoMap(other.mTagInfoMap)
    , android_to_mtk(other.android_to_mtk)
    , mtk_to_android(other.mtk_to_android)
{
}


IMetadataTagSet::Implementor&
IMetadataTagSet::Implementor::
operator=(IMetadataTagSet::Implementor const& other)
{
    if (this != &other)
    {
        //release mMap'storage
        //assign other.mMap's storage pointer to mMap
        //add 1 to storage's sharebuffer
        mTagInfoMap = other.mTagInfoMap;
        android_to_mtk = other.android_to_mtk;
        mtk_to_android = other.mtk_to_android;
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadataTagSet::
IMetadataTagSet()
    : mpImp(new Implementor())
{
}


IMetadataTagSet::
~IMetadataTagSet()
{
    if(mpImp) delete mpImp;
}


IMetadataTagSet::IMetadataTagSet(IMetadataTagSet const& other)
    : mpImp(new Implementor(*(other.mpImp)))
{
}


IMetadataTagSet&
IMetadataTagSet::operator=(IMetadataTagSet const& other)
{
    if (this != &other) {
        delete mpImp;
        mpImp = new Implementor(*(other.mpImp));
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


MINT32
IMetadataTagSet::
getType(MUINT32 tag) const
{
    if(mpImp) return mpImp->getType(tag);
    return 0;
}


char const*
IMetadataTagSet::
getName(unsigned int tag) const
{
    if(mpImp) return mpImp->getName(tag);
    return NULL;
}


MVOID
IMetadataTagSet::
addTag(MUINT32 tag, char const* name, MINT32 typeEnum)
{
    if(mpImp) mpImp->addTag(tag, name, typeEnum);
}

MVOID
IMetadataTagSet::
addTagMap(MUINT32 androidTag, MUINT32 MtkTag)
{
    if(mpImp) mpImp->addTagMap(androidTag, MtkTag);
}


MUINT32
IMetadataTagSet::
getMtkTag(MUINT32 tag) const
{
    #if (PLATFORM_SDK_VERSION >= 21)
    if(mpImp) return mpImp->getMtkTag(tag);
    #endif
    return -1;
}


MUINT32
IMetadataTagSet::
getAndroidTag(MUINT32 tag) const
{
    #if (PLATFORM_SDK_VERSION >= 21)
    if(mpImp) return mpImp->getAndroidTag(tag);
    #endif
    return -1;
}

/******************************************************************************
 *
 ******************************************************************************/
/* //workaround for build fail
namespace NSCam {


void
dumpMetadataEntry(IMetadata::IEntry const& entry, android::String8& rs8Log)
{

    int const typeEnum = get_mtk_metadata_tag_type(entry.tag());

    rs8Log += String8::format("TAG <%s> ", get_mtk_metadata_tag_name(entry.tag()));

    for (MUINT i = 0; i < entry.count(); i++)
    {
        switch  ( typeEnum )
        {
        case TYPE_MUINT8:{
            MUINT8 const& v = entry.itemAt(i, Type2Type<MUINT8>());
            rs8Log += String8::format("%d ", v);
            }break;
        case TYPE_MINT32:{
            MINT32 const& v = entry.itemAt(i, Type2Type<MINT32>());
            rs8Log += String8::format("%d ", v);
            }break;
        case TYPE_MFLOAT:{
            MFLOAT const& v = entry.itemAt(i, Type2Type<MFLOAT>());
            rs8Log += String8::format("%f ", v);
            }break;
        case TYPE_MINT64:{
            MINT64 const& v = entry.itemAt(i, Type2Type<MINT64>());
            rs8Log += String8::format("%lld ", v);
            }break;
        case TYPE_MDOUBLE:{
            MDOUBLE const& v = entry.itemAt(i, Type2Type<MDOUBLE>());
            rs8Log += String8::format("%lf ", v);
            }break;
        case TYPE_MRational:{
            MRational const& v = entry.itemAt(i, Type2Type<MRational>());
            rs8Log += String8::format("%d/%d ", v.numerator, v.denominator);
            }break;
        case TYPE_MPoint:{
            MPoint const& v = entry.itemAt(i, Type2Type<MPoint>());
            rs8Log += String8::format("(%d %d)", v.x, v.y);
            }break;
        case TYPE_MSize:{
            MSize const& v = entry.itemAt(i, Type2Type<MSize>());
            rs8Log += String8::format("(%d %d)", v.w, v.h);
            }break;
        case TYPE_MRect:{
            MRect const& v = entry.itemAt(i, Type2Type<MRect>());
            rs8Log += String8::format("(%d %d %d %d)", v.p.x, v.p.y, v.s.w, v.s.h);
            }break;
        case TYPE_IMetadata:{
            rs8Log += String8("\n");
            IMetadata const& v = entry.itemAt(i, Type2Type<IMetadata>());
            dumpMetadata(v, rs8Log);
            }break;
        default:
            break;
        }
    }
}

}
*/


/******************************************************************************
 *
 ******************************************************************************/
/*
namespace NSCam {
void
dumpMetadata(IMetadata const& metadata, android::String8& rs8Log)
{
    rs8Log += String8::format("<IMetadata> count:%d \n", metadata.count());
    for (MUINT i = 0; i < metadata.count(); i++)
    {
        IMetadata::IEntry const& entry = metadata.entryAt(i);
        dumpMetadataEntry(entry, rs8Log);
        rs8Log += String8("\n");
    }
}
}
*/
