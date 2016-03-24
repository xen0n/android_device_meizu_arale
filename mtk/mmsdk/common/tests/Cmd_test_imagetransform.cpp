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

#define LOG_TAG "MMSdkHALTest"

//
#include <utils/Log.h>
#include <utils/Vector.h>
#include <binder/Parcel.h>
#include <cutils/memory.h>
#include <hardware/hardware.h>
#include <mmsdk_common.h>

#include <Format.h>
#include <common.h>
#include <imageTransform_device.h>

//
#include "inc/log.h"
#include "inc/utils.h"
#include "inc/command.h"
#include "res/YV12_640x480.h"


#if defined(HAVE_COMMAND_imageTransform)

#define DEVICE_MAJOR_VERSION(a)   ((a >> 8)& 0xff)
#define DEVICE_MINOR_VERSION(a)   ((a & 0xff))

// 
using namespace android;
using namespace NSCam; 

//
//
//
/******************************************************************************
 *  Command
 *      service <client> <-h>
 *
 *      -h:                     help
 *      <client>   client connect test 
 *
 ******************************************************************************/
namespace NSCmd_imageTransform {
class CmdImp : public CmdBase
{
//
private:
    mmsdk_module_t              *mModule; 
    imageTransform_device_t     *mDevice; 

    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                    , mModule(0)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

protected:  ////                Implementation.
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd);

    virtual bool                onOpen(); 
protected:  ////                Implementation.
    virtual mmsdk_module_t*     getHALModule() {return mModule;};

    virtual bool                checkHALModuleVersion(mmsdk_module_t const *module); 
    virtual bool                closeDevice(imageTransform_device_t **device); 
    virtual bool                openDevice(hw_module_t *module, imageTransform_device_t **device);
    virtual bool                loadHALModule(mmsdk_module_t **module); 
    virtual bool                allocBuf(IImageBuffer **pBuf, MSize const & imgSize, int const format); 
    virtual bool                freeBuf(IImageBuffer **pBuf); 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Data Members (Parameters)


};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_imageTransform, new CmdImp(HAVE_COMMAND_imageTransform));
};  // NSCmd_test_preview
using namespace NSCmd_imageTransform;


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    return  onParseActionCommand(rvCmd);
}




/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseActionCommand(Vector<String8>& rvCmd)
{
    //  (1) Start to parse ACTION commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        //
        if  ( s8Cmd == "-h" ) {
            String8 text;
            text += "\n";
            text += "\n   service <client> <-h>";
            text += "\n   <-h>:                   help";
            text += "\n   <client>  client connect test";
            text += "\n";
            MY_LOGD("%s", text.string());
            return  true;
        }
        //
        if  ( s8Cmd == "open" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onOpen(); 
        }
        //
    }
    return  false;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onOpen()
{
    // 
    MY_LOGD("onOpen"); 
    //
    bool ret = false; 
    ret = loadHALModule(&mModule) 
          && checkHALModuleVersion(mModule); 
    if (!ret)
    {
        MY_LOGE("Fail to load HAL module or version check fail"); 
        return false; 
    }
    //
    ret = openDevice(&mModule->common, &mDevice); 
    
    //
    IImageBuffer *pSrcBuffer = NULL; 
    //
    if (!allocBuf(&pSrcBuffer, MSize(640, 480), eImgFmt_YV12))
    {
        MY_LOGE("Fail to alloc source image buffer");  
        return false; 
    }
    //
    memcpy((void*)pSrcBuffer->getBufVA(0), (void*)TestImage_YV12_640x480_1, pSrcBuffer->getBufSizeInBytes(0)); 
    memcpy((void*)pSrcBuffer->getBufVA(1), (void*)(TestImage_YV12_640x480_1+pSrcBuffer->getBufSizeInBytes(0)),  
                                           pSrcBuffer->getBufSizeInBytes(1)); 
    memcpy((void*)pSrcBuffer->getBufVA(2), (void*)(TestImage_YV12_640x480_1+pSrcBuffer->getBufSizeInBytes(0)+ 
                                                                    pSrcBuffer->getBufSizeInBytes(1)),
                                           pSrcBuffer->getBufSizeInBytes(2)); 
    //test
    pSrcBuffer->saveToFile("/sdcard/test.yuv"); 
  
    //
    IImageBuffer *pDestBuffer = NULL; 
    if (!allocBuf(&pDestBuffer, MSize(640, 480), eImgFmt_YUY2))
    {
        MY_LOGE("Fail to alloc destination image buffer");  
        return false; 
    }
    transformOptions_t transformOptions; 
    transformOptions.rect = MRect(640, 480);

    //
    mDevice->ops->applyTransform(mDevice, pSrcBuffer, pDestBuffer, &transformOptions); 

    pDestBuffer->saveToFile("/sdcard/result.yuv"); 

    freeBuf(&pSrcBuffer); 
    freeBuf(&pDestBuffer); 

    
    //
    ret = closeDevice(&mDevice); 
    return 0; 
}


/******************************************************************************
 *
 ******************************************************************************/
bool 
CmdImp::
loadHALModule(mmsdk_module_t **module)
{
    //
    if (::hw_get_module(MMSDK_HARDWARE_MODULE_ID, 
           (const hw_module_t **)module) < 0)
    {
        MY_LOGE("Could not load mmsdk HAL module");
        return false; 
    }
    // 
    MY_LOGD("Load \"%s\" mmsdk moudle", (*module)->common.name); 
    //
    return true; 
  
}

/******************************************************************************
 *
 ******************************************************************************/
bool 
CmdImp::
checkHALModuleVersion(mmsdk_module_t const *module)
{
    if (module->common.module_api_version == MMSDK_MODULE_API_VERSION_1_0)
    {
        MY_LOGD("MMSDK API is 1.0"); 
    }
    else 
    {
        MY_LOGD("MMSDK API is not 1.0"); 
        return false; 
    }
    return true; 
}

/******************************************************************************
 *
 ******************************************************************************/
bool 
CmdImp::
openDevice(hw_module_t *module, imageTransform_device_t **device) 
{
    //    
    if (0 == module)
    {
        MY_LOGE("Null module object"); 
        return false; 
    }
    // 
    bool ret = module->methods->open(module, MMSDK_HARDWARE_IMAGE_TRANSFORM, 
                                   (hw_device_t**)device); 
    
    return ret; 
}


/******************************************************************************
 *
 ******************************************************************************/
bool 
CmdImp::
closeDevice(imageTransform_device_t  **device)
{
    //
    if (0 == *device)
    {
        MY_LOGE("Null device object"); 
        return false; 
    }
    //
    int rc = (*device)->common.close(&((*device)->common)); 
    if (rc != OK)
    {
        MY_LOGE("Could not close image transform device:%d", rc); 
        return false; 
    }
    
    return true;    
}


/******************************************************************************
* 
*******************************************************************************/
bool 
CmdImp:: 
allocBuf(IImageBuffer **pBuf, MSize const & imgSize, int const format) 
{
    MUINT32 bufStridesInBytes[3] = {0};
using namespace NSCam::Utils::Format; 

    MUINT32 plane = queryPlaneCount(format);
    for (MUINT32 i = 0; i < plane; i++)
    { 
        bufStridesInBytes[i] = ( queryPlaneWidthInPixels(format,i, imgSize.w) * queryPlaneBitsPerPixel(format,i) ) >> 3;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator::ImgParam imgParam(
            format, imgSize, bufStridesInBytes, bufBoundaryInBytes, plane
            );


    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    *pBuf = allocator->alloc_ion(LOG_TAG, imgParam);
    if  ( (*pBuf) == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return  MFALSE;
    }

    if ( !(*pBuf)->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
    {
        MY_LOGE("lock Buffer failed\n");
        return false; 
    }

    //
    MY_LOGD("ImgBitsPerPixel:%d BufSizeInBytes:%d %d", (*pBuf)->getImgBitsPerPixel(), (*pBuf)->getBufSizeInBytes(0), imgSize.size());
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
bool 
CmdImp::
freeBuf(IImageBuffer **pBuf)
{
    if( !(*pBuf)->unlockBuf( LOG_TAG ) )
    {
        MY_LOGE("unlock Buffer failed\n");
        return false; 
    }
    *pBuf = NULL;     
    return true;  
}

/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

