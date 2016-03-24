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
#define LOG_TAG "ExifDbgInf"

//
#include <stdlib.h>
//
#include <Log.h>
#include <common.h>
//
#include <exif/IBaseCamExif.h>
#include <exif/IDbgInfoContainer.h>
//
#include <utils/Vector.h>

using namespace android;
//
/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
class DbgInfoContainer : public IDbgInfoContainer
{
    public:

        DbgInfoContainer();
        ~DbgInfoContainer();

    public: //operations

        virtual MVOID*  queryMemory(MUINT32 key, size_t size, MBOOL clean);

        virtual MBOOL   setToExif(IBaseCamExif* pExif) const;

        virtual MBOOL   copyTo(IDbgInfoContainer* pDbg) const;

    private:

        struct debugInfoBlock
        {
            MUINT32 mKey;
            MVOID*  mPtr;
            size_t  mSize;
        };

        Vector<debugInfoBlock>       mlBlocks;
};


/******************************************************************************
*
*******************************************************************************/
IDbgInfoContainer*
IDbgInfoContainer::
createInstance()
{
    return new DbgInfoContainer();
}


/******************************************************************************
*
*******************************************************************************/
MVOID
IDbgInfoContainer::
destroyInstance()
{
    delete this;
}


/******************************************************************************
*
*******************************************************************************/
IDbgInfoContainer::
IDbgInfoContainer()
{
}


/******************************************************************************
*
*******************************************************************************/
IDbgInfoContainer::
~IDbgInfoContainer()
{
}


/******************************************************************************
*
*******************************************************************************/
DbgInfoContainer::
DbgInfoContainer()
{
}


/******************************************************************************
*
*******************************************************************************/
DbgInfoContainer::
~DbgInfoContainer()
{
    Vector<debugInfoBlock>::iterator iter = mlBlocks.begin();
    while( iter != mlBlocks.end() )
    {
        free(iter->mPtr);
        iter++;
    }
    mlBlocks.clear();
}


/******************************************************************************
*
*******************************************************************************/
MVOID*
DbgInfoContainer::
queryMemory(MUINT32 key, size_t size, MBOOL clean)
{
    MVOID* ptr = (MVOID*)malloc(size);
    if( !ptr )
    {
        MY_LOGE("allocation failed key(0x%x), size(%d)", key, size);
        return 0;
    }

    if( clean )
        memset( (MVOID*)ptr, 0x00, size );

    debugInfoBlock newblock = { key, ptr, size };
    mlBlocks.push_back(newblock);

    //MY_LOGD("key %d, size %d, prt %p", key, size, ptr);
    return ptr;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
DbgInfoContainer::
setToExif(IBaseCamExif* pExif) const
{
    Vector<debugInfoBlock>::const_iterator iter = mlBlocks.begin();
    while( iter != mlBlocks.end() )
    {
        //MY_LOGD("key %d, size %d, prt %p", iter->mKey, iter->mSize, iter->mPtr);
        MINT32 ID;
        pExif->sendCommand(CMD_REGISTER, iter->mKey, reinterpret_cast<MUINTPTR>(&ID));
        pExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MUINTPTR>(iter->mPtr), iter->mSize);
        iter++;
    }
    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
DbgInfoContainer::
copyTo(IDbgInfoContainer* pDbg) const
{
    MBOOL ret = MTRUE;
    Vector<debugInfoBlock>::const_iterator iter = mlBlocks.begin();
    while( iter != mlBlocks.end() )
    {
        MVOID* ptr = pDbg->queryMemory(iter->mKey, iter->mSize, MFALSE);
        if( ptr != NULL ) {
            memcpy(ptr, iter->mPtr, iter->mSize);
        } else {
            MY_LOGE("query memory failed");
            ret = MFALSE;
        }
        iter++;
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
