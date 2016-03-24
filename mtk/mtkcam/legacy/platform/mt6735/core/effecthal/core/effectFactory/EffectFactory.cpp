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

#define LOG_TAG "mmsdk/EffectFactory"

//#include <cutils/xlog.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <IEffectHal.h>
//#include "HdrEffectHal.h"
#include "FaceBeautyEffectHal.h"
#include "JPGEffectHal.h"

#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
//using namespace android;


/******************************************************************************
 *
 ******************************************************************************/
#if 0
IEffectFactory *gEffectFactory = NULL;
#endif

extern "C"
IEffectFactory*
createEffectFactory()
{
#if 0
    if(!gEffectFactory) {
        gEffectFactory = new EffectFactory();
    }
    return gEffectFactory;
#else
     ALOGD("[%s] - start", __FUNCTION__);
    return new EffectFactory();
#endif
}


EffectFactory::
EffectFactory()
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
};


EffectFactory::
~EffectFactory()
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
}

// status_t
// EffectFactory::
// createEffectHal(EffectHalVersion nameVersion, sp<IEffectHal> &effectHal)
// {
//     //@todo implement this - check error
//     FUNCTION_LOG_START;
//     // effectHal = new HdrEffectHal();
//     FUNCTION_LOG_END;
//     return OK;
// }


status_t
EffectFactory::
createEffectHalClient(EffectHalVersion nameVersion, sp<IEffectHalClient> &effectHalClient)
{
    FUNCTION_LOG_START;

    ALOGD("[%s] create name=%s", __FUNCTION__, nameVersion.effectName.string());

    if (nameVersion.effectName == String8("Jpg"))
    {
        ALOGD("[%s] create JpgEffectHal", __FUNCTION__);
        IEffectHal *effectHal = new JPGEffectHal();
        effectHalClient = new EffectHalClient(effectHal, nameVersion.effectName);
        // IEffectHal *effectHal = new HdrEffectHal();
        // effectHalClient = new EffectHalClient(effectHal);
    }
    else
    {
        ALOGD("[%s] create FaceBeautyEffectHal", __FUNCTION__);
        IEffectHal *effectHal = new FaceBeautyEffectHal();
        effectHalClient = new EffectHalClient(effectHal, nameVersion.effectName);
    }

    MINT32 maxJpegsize = 0, jpegsize=0, maxJpegWidth=0, maxJpegHeight=0;
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(0);
    sp <BasicParameters> staticMetadata = new BasicParameters();
    if( ! pMetadataProvider.get() )
    {
        ALOGE("[%s] ! pMetadataProvider.get() , set staticMetadata DeviceVersion = 1", __FUNCTION__);
        staticMetadata->set("isHal3", 0);
    }
    else
    {
        staticMetadata->set("isHal3", (pMetadataProvider->getDeviceVersion() == MTK_HAL_VERSION_3_2));
        ALOGD("[%s] Device Version is1.0= %d,is 3.2=%d",__FUNCTION__ ,
                (pMetadataProvider->getDeviceVersion()==MTK_HAL_VERSION_1_0),
                 (pMetadataProvider->getDeviceVersion()==MTK_HAL_VERSION_3_2));

        IMetadata::IEntry const& entryScaler =
            pMetadataProvider->geMtktStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);

        if ( entryScaler.isEmpty() )
        {
            ALOGE("no static MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS");
        }
        else
        {
            const int STREAM_CONFIGURATION_SIZE = 4;
            const int STREAM_FORMAT_OFFSET = 0;
            const int STREAM_WIDTH_OFFSET = 1;
            const int STREAM_HEIGHT_OFFSET = 2;
            const int STREAM_IS_INPUT_OFFSET = 3;
            MINT32 format = 0;
            // android.scaler.availableStreamConfigurations
            // int32 x n x 4
            for(size_t i = 0; i < entryScaler.count(); i += STREAM_CONFIGURATION_SIZE)
            {
                //sava support format
                format = entryScaler.itemAt(i+STREAM_FORMAT_OFFSET, Type2Type<MINT32>());
                char str[16];
                sprintf(str, "%d", format);

                if (staticMetadata->getInt(str) == -1)
                {
                    staticMetadata->set(str, 1);
                    ALOGD("[%s] format:%d support", __FUNCTION__, format);
                }
                if (entryScaler.itemAt(i+STREAM_FORMAT_OFFSET, Type2Type<MINT32>())!= HAL_PIXEL_FORMAT_BLOB)
                {
                    continue;
                }
                //avaiblable blob size list should order in descedning.
                int32_t width = entryScaler.itemAt(i + STREAM_WIDTH_OFFSET, Type2Type<MINT32>());
                int32_t height = entryScaler.itemAt(i + STREAM_HEIGHT_OFFSET, Type2Type<MINT32>());
                int32_t isInput = entryScaler.itemAt(i + STREAM_IS_INPUT_OFFSET, Type2Type<MINT32>());

                //ALOGD("[%s] w=%d, h=%d, isInput=%d", __FUNCTION__, width, height, isInput);
                jpegsize = (width*height)*1.2; //*2*0.6

                if (isInput  == ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT && jpegsize > maxJpegsize)
                {
                    maxJpegWidth = width;
                    maxJpegHeight = height;
                }

                if (jpegsize > maxJpegsize)
                {
                    maxJpegsize = jpegsize;
                }

                if (isInput == ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT
                         && (width * height > maxJpegWidth * maxJpegHeight))
                {
                    ALOGD("[%s] maxJpegsize=%d, maxJpegWidth=%d, maxJpegHeight=%d",
                             __FUNCTION__, maxJpegsize, maxJpegWidth, maxJpegHeight);
                }
            }
        }
         ALOGD("[%s] maxJpegsize=%d, maxJpegWidth=%d, maxJpegHeight=%d",
            __FUNCTION__, maxJpegsize, maxJpegWidth, maxJpegHeight);
    }

    maxJpegsize = 15474585;
    staticMetadata->set("maxJpegsize", maxJpegsize);
    staticMetadata->set("maxJpegWidth", maxJpegWidth);
    staticMetadata->set("maxJpegHeight", maxJpegHeight);
    effectHalClient->setStaticMetadata(staticMetadata);





    FUNCTION_LOG_END;
    return OK;
}


status_t
EffectFactory::
getSupportedVersion(String8 effectName, Vector<EffectHalVersion> &versions)
{
    FUNCTION_LOG_START;
    versions.clear();

    //@todo implement this
    #if 1   //EXAMPLE
    EffectHalVersion version;
    if (effectName == "Jpg")
    {
        version.effectName = "Jpg";
        version.major = 1;
        version.minor = 0;
    }
    else
    {
        version.effectName = "FaceBeauty";
        version.major = 1;
        version.minor = 0;
    }
    versions.add(version);
    #endif

    FUNCTION_LOG_END;
    return OK;
}


#if 0   //@todo implement this - wait chengtian
status_t
EffectFactory::
getSupportedVersion(Vector<String8> effectNames, Vector<Vector<EffectHalVersion> >  &versions)
{
    FUNCTION_LOG_START;
    versions.clear();

    //@todo implement this
    #if 1   //EXAMPLE
    Vector<EffectHalVersion> v;
    EffectHalVersion version;
    version.effectName = "Hdr";
    version.major = 1;
    version.minor = 0;
    v.add(version);
    versions.add(v);
    #endif

    FUNCTION_LOG_END;
    return OK;
}
#endif

