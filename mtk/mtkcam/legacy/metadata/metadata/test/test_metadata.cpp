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

#define LOG_TAG "TestMetadata"
#include <metadata/IMetadata.h>

#include <Log.h>
#include <common.h>
#include <utils/common.h>
#include <metadata/client/mtk_metadata_tag.h>
#include <metadata/mtk_metadata_types.h>
#include <utils/Vector.h>
using namespace NSCam;
using namespace android;

/*******************************************************************************
*
********************************************************************************/
void testEntry();
void testMetadata();
void testMetadata2();
void testMemory();
void testMetadataMemory();

/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int, char**)
{
    CAM_LOGD("test start");

    testEntry();
    testMetadata();
    testMetadata2();
    testMemory();
    testMetadataMemory();

    CAM_LOGD("test exit");
    return 0;
}

#define ASSERT_IF_NOT_EQAL(_In1, _In2) \
    CAM_LOGA_IF(_In1 != _In2, "should be %d, but is %d", _In2, _In1)

#define ASSERT_IF_EQAL(_In1, _In2) \
    CAM_LOGA_IF(_In1 == _In2, "shouldn't be %d, but is %d", _In2, _In1)

void testMetadata()
{
    MRect region1(MPoint(2,3),MSize(4,5));
    MRect region2(MPoint(20,30),MSize(40,50));

    //test 1: construction
    IMetadata::IEntry entry1(MTK_SCALER_CROP_REGION);
    entry1.push_back(region1, Type2Type< MRect >());

    IMetadata metadata1;
    metadata1.update(MTK_SCALER_CROP_REGION, entry1);

    // add more tags..
    IMetadata::IEntry sensorFps(MTK_SENSOR_INFO_FRAME_RATE);
    sensorFps.push_back(33, Type2Type< MINT32 >());
    metadata1.update(MTK_SENSOR_INFO_FRAME_RATE, sensorFps);


    //test 2: copy construction
    IMetadata metadata2(metadata1);
    ASSERT_IF_NOT_EQAL(metadata2.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()), region1);

    //test 3: assignment operator
    IMetadata metadata3 = metadata1;
    ASSERT_IF_NOT_EQAL(metadata3.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()), region1);


    //test 4: query (read-only)
    const IMetadata::IEntry entry4 = metadata1.entryFor(MTK_SCALER_CROP_REGION);
    ASSERT_IF_NOT_EQAL(entry4.itemAt(0, Type2Type< MRect >()), region1);


    //test 5-1: query and revise through editEntryFor, and update by metadata
    {
        IMetadata::IEntry entry5 = metadata1.editEntryFor(MTK_SCALER_CROP_REGION);
        ASSERT_IF_NOT_EQAL(entry5.itemAt(0, Type2Type< MRect >()), region1);
        entry5.editItemAt(0, Type2Type< MRect >()) = region2;
        metadata1.update(MTK_SCALER_CROP_REGION, entry5);

        const IMetadata::IEntry entry_test = metadata1.editEntryFor(MTK_SCALER_CROP_REGION);
        ASSERT_IF_NOT_EQAL(entry_test.itemAt(0, Type2Type< MRect >()), region2);
    }


    //test 5-2: query and revise through editEntryFor directly
    {
        IMetadata::IEntry &entry5 = metadata2.editEntryFor(MTK_SCALER_CROP_REGION);
        ASSERT_IF_NOT_EQAL(entry5.itemAt(0, Type2Type< MRect >()), region1);
        entry5.editItemAt(0, Type2Type< MRect >()) = region2;

        const IMetadata::IEntry entry_test = metadata2.entryFor(MTK_SCALER_CROP_REGION);
        ASSERT_IF_NOT_EQAL(entry_test.itemAt(0, Type2Type< MRect >()), region2);
    }


    //test 6: set and get nested metadata
    {
        //create
        IMetadata::IEntry sensorFps(MTK_SENSOR_INFO_FRAME_RATE);
        sensorFps.push_back(33, Type2Type< MINT32 >());

        IMetadata::IEntry sensorRegion(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
        MRect rect;
        rect.p.x = 10;
        rect.p.y = 20;
        rect.s.h = 600;
        rect.s.w = 800;
        sensorRegion.push_back(rect, Type2Type< MRect >());

        IMetadata metadataUnion;
        metadataUnion.update(MTK_SENSOR_INFO_FRAME_RATE, sensorFps);
        metadataUnion.update(sensorRegion.tag(), sensorRegion);

        IMetadata::IEntry entryUnion(MTK_SENSOR_INFO_PACKAGE);
        entryUnion.push_back(metadataUnion, Type2Type< IMetadata >());

        IMetadata metadataOverall;
        metadataOverall.update(MTK_SENSOR_INFO_PACKAGE, entryUnion);

        //read
        const IMetadata::IEntry &entry = metadataOverall.entryFor(MTK_SENSOR_INFO_PACKAGE);
        const IMetadata &metadataUnion_getback = entry.itemAt(0, Type2Type< IMetadata >());

        const IMetadata::IEntry &entryB = metadataUnion_getback.entryFor(MTK_SENSOR_INFO_FRAME_RATE);
        const IMetadata::IEntry &entryC = metadataUnion_getback.entryFor(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);

        ASSERT_IF_NOT_EQAL(entryB.itemAt(0, Type2Type< MINT32 >()), 33);
        ASSERT_IF_NOT_EQAL(entryC.itemAt(0, Type2Type< MRect >()).p.x, 10);
        ASSERT_IF_NOT_EQAL(entryC.itemAt(0, Type2Type< MRect >()).p.y, 20);
        ASSERT_IF_NOT_EQAL(entryC.itemAt(0, Type2Type< MRect >()).s.w, 800);
        ASSERT_IF_NOT_EQAL(entryC.itemAt(0, Type2Type< MRect >()).s.h, 600);
    }


    // test 7: remove
    {
        IMetadata metadata7(metadata1);
        const IMetadata::IEntry entry = metadata7.entryFor(MTK_SCALER_CROP_REGION);
        MRect region = entry.itemAt(0, Type2Type< MRect >());

        metadata1.remove(MTK_SCALER_CROP_REGION);

        ASSERT_IF_NOT_EQAL(metadata7.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()), region);
        ASSERT_IF_NOT_EQAL(metadata1.entryFor(MTK_SCALER_CROP_REGION).tag(), IMetadata::IEntry::BAD_TAG);
    }

    // test 8: operator+
    {
        // const values
        MINT32 const sensorFps = 33;
        MRect const activeArray(MPoint(10, 20), MSize(600, 600));
        MRect const cropRegion(MPoint(5, 7), MSize(99, 123));
        //
        IMetadata metadata8_0;
        {
            //create metadata with 2 entry
            IMetadata::IEntry entryFps(MTK_SENSOR_INFO_FRAME_RATE);
            entryFps.push_back(sensorFps, Type2Type< MINT32 >());

            IMetadata::IEntry sensorRegion(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
            sensorRegion.push_back(activeArray, Type2Type< MRect >());

            metadata8_0.update(MTK_SENSOR_INFO_FRAME_RATE, entryFps);
            metadata8_0.update(sensorRegion.tag(), sensorRegion);
        }
        IMetadata metadata8_1;
        {
            IMetadata::IEntry entry1(MTK_SCALER_CROP_REGION);
            entry1.push_back(cropRegion, Type2Type< MRect >());
            metadata8_1.update(MTK_SCALER_CROP_REGION, entry1);
        }

        IMetadata metadataresult = metadata8_0 + metadata8_1;
        {
            // check
            IMetadata::IEntry entry = metadataresult.entryFor(MTK_SENSOR_INFO_FRAME_RATE);
            ASSERT_IF_EQAL( entry.isEmpty(), MTRUE );
            if( ! entry.isEmpty() ) {
                ASSERT_IF_NOT_EQAL(
                        entry.itemAt(0, Type2Type< MINT32 >()),
                        sensorFps);
            }
            entry = metadataresult.entryFor(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
            ASSERT_IF_EQAL( entry.isEmpty(), MTRUE );
            if( ! entry.isEmpty() ) {
                ASSERT_IF_NOT_EQAL(
                        entry.itemAt(0, Type2Type< MRect >()),
                        activeArray);
            }
            entry = metadataresult.entryFor(MTK_SCALER_CROP_REGION);
            ASSERT_IF_EQAL( entry.isEmpty(), MTRUE );
            if( ! entry.isEmpty() ) {
                ASSERT_IF_NOT_EQAL(
                        entry.itemAt(0, Type2Type< MRect >()),
                        cropRegion);
            }
        }
    }
}


void testEntry()
{
    //construction
    IMetadata::IEntry entryA(MTK_SCALER_CROP_REGION);

    //copy construction
    IMetadata::IEntry entryB(entryA);

    //assignmenet operator
    IMetadata::IEntry entryC = entryA;

    //add value
    MRect regionA(MPoint(2,3), MSize(4,5));
    MRect regionB(MPoint(6,7), MSize(8,9));
    MRect regionC(MPoint(10,11), MSize(12,13));

    entryA.push_back(regionA, Type2Type< MRect >());
    entryB.push_back(regionB, Type2Type< MRect >());
    entryC.push_back(regionC, Type2Type< MRect >());

    //copy constructor
    IMetadata::IEntry entryD(entryA);

    //read value
    ASSERT_IF_NOT_EQAL(entryA.itemAt(0, Type2Type< MRect >()), regionA);
    ASSERT_IF_NOT_EQAL(entryD.itemAt(0, Type2Type< MRect >()), entryA.itemAt(0, Type2Type< MRect >()));
    ASSERT_IF_NOT_EQAL(entryB.itemAt(0, Type2Type< MRect >()), regionB);
    ASSERT_IF_NOT_EQAL(entryC.itemAt(0, Type2Type< MRect >()), regionC);

    //edit value
    MRect regionD(MPoint(20,21), MSize(22,23));
    entryA.editItemAt(0, Type2Type< MRect >()) = regionD;
    ASSERT_IF_NOT_EQAL(entryA.itemAt(0, Type2Type< MRect >()), regionD);

    //clear
    entryA.clear();
    ASSERT_IF_NOT_EQAL(entryA.count(), 0);


    //type int32 and int64
    IMetadata::IEntry sensorExp(MTK_SENSOR_EXPOSURE_TIME);
    sensorExp.push_back(1, Type2Type< MINT64>()); //int_64
    ASSERT_IF_EQAL(sensorExp.count(), 0);
    ASSERT_IF_NOT_EQAL(sensorExp.itemAt(0, Type2Type< MINT64 >()), 1);
}


IMetadata
getMetadata2A()
{
    IMetadata::IEntry entryA(MTK_SCALER_CROP_REGION);
    MRect region(MPoint(2,3), MSize(4,5));
    entryA.push_back(region, Type2Type< MRect >());

    IMetadata metadata;
    metadata.update(MTK_SCALER_CROP_REGION, entryA);
    return metadata;
}


IMetadata
getMetadata2B()
{
    IMetadata::IEntry sensorRegion(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
    MRect rect;
    rect.p.x = 10;
    rect.p.y = 20;
    rect.s.h = 600;
    rect.s.w = 800;
    sensorRegion.push_back(rect, Type2Type< MRect >());

    IMetadata metadata;
    metadata.update(sensorRegion.tag(), sensorRegion);
    return metadata;
}


void testMetadata2()
{
    // either add itemAt function with an index as input parameter,
    // or search through all enums.
    IMetadata unionMetadata;
    IMetadata::Tag_t mTag = IMetadata::IEntry::BAD_TAG;

    //
    IMetadata metadataA = getMetadata2A();
    for (size_t i = 0; i < metadataA.count(); i++)
    {
        mTag = metadataA.entryAt(i).tag();
        unionMetadata.update(metadataA.entryAt(i).tag(), metadataA.entryAt(i));
    }

    //
    IMetadata metadataB = getMetadata2B();
    for (size_t i = 0; i < metadataB.count(); i++)
    {
        unionMetadata.update(metadataB.entryAt(i).tag(), metadataB.entryAt(i));
    }

    ASSERT_IF_NOT_EQAL(unionMetadata.count(), metadataA.count() + metadataB.count());
    ASSERT_IF_EQAL(mTag, IMetadata::IEntry::BAD_TAG);

    if (mTag != IMetadata::IEntry::BAD_TAG) {
        IMetadata::IEntry EntryA = unionMetadata.entryFor(mTag);
        IMetadata::IEntry EntryB = metadataA.entryFor(mTag);
        ASSERT_IF_NOT_EQAL(EntryA.editItemAt(0, Type2Type< MRect >()), EntryB.editItemAt(0, Type2Type< MRect >()));
    }
}

void testMemory()
{
    MUINT8 val = 2;
    size_t size = 10;

    IMetadata::Memory mMemory;
    mMemory.resize(size);

    memset(mMemory.editArray(), val, sizeof(MUINT8)*size);

    IMetadata::Memory mMemory2;
    mMemory2 = mMemory;

    mMemory.clear();

    ASSERT_IF_NOT_EQAL(mMemory2.size(), size);
    ASSERT_IF_NOT_EQAL(mMemory2.itemAt(0), val);
}


void testMetadataMemory()
{
    MUINT8 val = 2;
    size_t size = 10;

    IMetadata::Memory mMemory;
    mMemory.resize(size);
    memset(mMemory.editArray(), val, sizeof(MUINT8)*size);

    const int MEMORY_TAG = 1;
    IMetadata::IEntry entry(MEMORY_TAG);
    entry.push_back(mMemory, Type2Type<IMetadata::Memory>());

    IMetadata meta;
    meta.update(MEMORY_TAG, entry);

    //read value
    IMetadata::Memory readOut = meta.entryFor(MEMORY_TAG).itemAt(0, Type2Type<IMetadata::Memory>());
    for(size_t i = 0; i < readOut.size(); i++) {
        ASSERT_IF_NOT_EQAL(readOut.itemAt(i), val);
    }
}

