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
#include <utils/Errors.h>
#include <utils/Log.h>
#include <fcntl.h>
#include "../inc/nvram_drv.h"
#include "nvram_drv_imp.h"
#include "libnvram.h"
#include "CFG_file_lid.h"
#include "camera_custom_AEPlinetable.h"
#include <aaa_types.h>
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include "CFG_file_lid.h"//AP_CFG_RESERVED_1 for AudEnh
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#define SUPPORT_BINDER_NVRAM 0
//#include "nvram_agent_client.h"
#include <cutils/properties.h>

#ifdef NVRAM_SUPPORT
#include "camera_custom_msdk.h"
#endif



/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/
#undef LOG_TAG
#define LOG_TAG "NvramDrv"

#define NVRAM_DRV_LOG(fmt, arg...)    ALOGD(LOG_TAG " " fmt, ##arg)
#define NVRAM_DRV_ERR(fmt, arg...)    ALOGE(LOG_TAG "MError: %5d: " fmt, __LINE__, ##arg)

#define logI(fmt, arg...)    NVRAM_DRV_LOG(fmt, ##arg)
#define logE(fmt, arg...)    NVRAM_DRV_ERR(fmt, ##arg)

#define INVALID_HANDLE_VALUE (-1)


using namespace android;
/*******************************************************************************
*
********************************************************************************/
static unsigned long const g_u4NvramDataSize[CAMERA_DATA_TYPE_NUM] =
{
    sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
    sizeof(NVRAM_CAMERA_3A_STRUCT),
    sizeof(NVRAM_CAMERA_SHADING_STRUCT),
    sizeof(NVRAM_LENS_PARA_STRUCT),
    sizeof(AE_PLINETABLE_T),
    sizeof(NVRAM_CAMERA_STROBE_STRUCT),
    sizeof(CAMERA_TSF_TBL_STRUCT),
    sizeof(NVRAM_CAMERA_GEOMETRY_STRUCT),
    sizeof(NVRAM_CAMERA_FEATURE_STRUCT),
    sizeof(NVRAM_CAMERA_VERSION_STRUCT),

};
    static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
/*******************************************************************************
*
********************************************************************************/

int binderWriteNvram(int file_lid, void *pBuf, int sh);
int binderReadNvram(int file_lid, void *pBuf, int sh);

NvramDrvBase*
NvramDrvBase::createInstance()
{
    return NvramDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrv::getInstance()
{
    static NvramDrv singleton;
    return &singleton;
}


void NvramDrv::setAndroidMode(int isAndroid)
{
    Mutex::Autolock lock(mLock);
    //mAndroidMode=isAndroid;
    mAndroidMode=0;
}

/*******************************************************************************
*
********************************************************************************/
void
NvramDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::NvramDrv()
    : NvramDrvBase()
{
    mAndroidMode=0;
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::~NvramDrv()
{
}

/*******************************************************************************
*
********************************************************************************/

int checkDataVersionNew(
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    int version
)
{
    int err = NVRAM_NO_ERROR;
    NVRAM_DRV_LOG("checkDataVersionNew");
    //err = NVRAM_DATA_VERSION_ERROR;
    int targetVersion=0;
    if (a_eNvramDataType == CAMERA_NVRAM_DATA_ISP)   // ISP
        targetVersion = NVRAM_CAMERA_PARA_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_3A)   // 3A
        targetVersion = NVRAM_CAMERA_3A_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_SHADING)   // Shading
        targetVersion = NVRAM_CAMERA_SHADING_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_LENS)   // Lens
        targetVersion = NVRAM_CAMERA_LENS_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_STROBE)   // strobe
        targetVersion = NVRAM_CAMERA_STROBE_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_GEOMETRY)   // geometry
        targetVersion = NVRAM_CAMERA_GEOMETRY_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_FEATURE)   // strobe
        targetVersion = NVRAM_CAMERA_FEATURE_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_DATA_AE_PLINETABLE)   // strobe
        targetVersion = NVRAM_CAMERA_PLINE_FILE_VERSION;
    else
        NVRAM_DRV_ERR("checkDataVersion(): incorrect data type\n");

    NVRAM_DRV_LOG("checkDataVersionNew v=%d vTar=%d", version, targetVersion);

    if(version!=targetVersion)
        return NVRAM_DATA_VERSION_ERROR;
    else
        return NVRAM_NO_ERROR;
}


int NvramDrv::readNoDefault(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
                          void *a_pNvramData,
                          unsigned long a_u4NvramDataSize)
{
    int err = NVRAM_NO_ERROR;
    NVRAM_DRV_LOG("[readNoDefault] id=%d dev=%d",a_eSensorType,a_eNvramDataType);
    err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);

    if (err != NVRAM_NO_ERROR)
        NVRAM_DRV_ERR("readNvramData() error: ==> readDefaultData()\n");
    return err;
}

int NvramDrv::readNvrameEx(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          unsigned long u4SensorID,
                          CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
                          void *a_pNvramData,
                          unsigned long a_u4NvramDataSize,
                          int version)
{
    int err = NVRAM_NO_ERROR;
    NVRAM_DRV_LOG("[readNvrameEx] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);
    if(a_eSensorType == DUAL_CAMERA_MAIN_SENSOR || a_eSensorType == DUAL_CAMERA_SUB_SENSOR || a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR)
       ;
    else
    {
        NVRAM_DRV_LOG("[readNvrameEx] sensorId error, line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_pNvramData==0)
    {
        NVRAM_DRV_LOG("[readNvrameEx] buf adr error (=0), line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_eNvramDataType < CAMERA_DATA_TYPE_START || a_eNvramDataType>=CAMERA_DATA_TYPE_NUM)
    {
        NVRAM_DRV_LOG("[readNvrameEx] date type id error, line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType])
    {
        NVRAM_DRV_LOG("[readNvrameEx] buf size is error, line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }


    if( a_eNvramDataType==CAMERA_DATA_TSF_TABLE)
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
                NVRAM_DRV_ERR("readDefaultData() error:\n");
          return err;
    }

    Mutex::Autolock lock(mLock);
    if(checkDataVersionNew(a_eNvramDataType, version)==NVRAM_NO_ERROR)
    {
        switch(a_eNvramDataType)
        {
        case CAMERA_NVRAM_DATA_ISP:
        case CAMERA_NVRAM_DATA_3A:
        case CAMERA_NVRAM_DATA_SHADING:
        case CAMERA_NVRAM_DATA_LENS:
        case CAMERA_NVRAM_DATA_STROBE:
        case CAMERA_NVRAM_DATA_GEOMETRY:
        case CAMERA_NVRAM_DATA_FEATURE:
        case CAMERA_DATA_AE_PLINETABLE:
            err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR)
            {
                NVRAM_DRV_ERR("readDefaultData() error:\n");
                err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
                if (err != NVRAM_NO_ERROR)
                    NVRAM_DRV_ERR("readDefaultData() error:\n");
            }
        default:
        break;
        }

    }
    else
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
            NVRAM_DRV_ERR("readDefaultData() error:\n");
        return err;
    }
    return err;
}
int
NvramDrv::readNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData,
    unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[readNvram] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

    if ((a_eSensorType > DUAL_CAMERA_MAIN_SECOND_SENSOR) ||
        (a_eSensorType < DUAL_CAMERA_MAIN_SENSOR) ||
        //(a_eNvramDataType > CAMERA_DATA_AE_PLINETABLE) ||
        (a_eNvramDataType >= CAMERA_DATA_TYPE_NUM) ||
        (a_eNvramDataType < CAMERA_NVRAM_DATA_ISP) ||
        (a_pNvramData == NULL) ||
        (a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType]))
    {
        NVRAM_DRV_LOG("[readNvram] error: line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }



    Mutex::Autolock lock(mLock);

    switch(a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
    case CAMERA_NVRAM_DATA_3A:
    case CAMERA_NVRAM_DATA_SHADING:
    case CAMERA_NVRAM_DATA_LENS:
    case CAMERA_NVRAM_DATA_STROBE:
    case CAMERA_DATA_AE_PLINETABLE:
        err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR) {
            NVRAM_DRV_ERR("readNvramData() error: ==> readDefaultData()\n");
            err = readDefaultData(a_eSensorType, a_u4SensorID, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR) {
                NVRAM_DRV_ERR("readDefaultData() error:\n");
            }
            break;
        }

        if (checkDataVersion(a_eNvramDataType, a_pNvramData) != NVRAM_NO_ERROR) {
            err = readDefaultData(a_eSensorType,a_u4SensorID, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR) {
                NVRAM_DRV_ERR("readDefaultData() error:\n");
            }
        }
        break;
    case CAMERA_DATA_TSF_TABLE:
        err = readDefaultData(a_eSensorType, a_u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR) {
            NVRAM_DRV_ERR("readDefaultData() TSF table error:\n");
        }
        break;
    default:
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData,
    unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[writeNvram] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);
    if(a_eSensorType == DUAL_CAMERA_MAIN_SENSOR || a_eSensorType == DUAL_CAMERA_SUB_SENSOR || a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR)
       ;
    else
    {
        NVRAM_DRV_LOG("[readNvrameEx] sensorId error, line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_pNvramData==0)
    {
        NVRAM_DRV_LOG("[readNvrameEx] buf adr error (=0), line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_eNvramDataType < CAMERA_DATA_TYPE_START || a_eNvramDataType>=CAMERA_DATA_TYPE_NUM)
    {
        NVRAM_DRV_LOG("[readNvrameEx] date type id error, line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType])
    {
        NVRAM_DRV_LOG("[readNvrameEx] buf size is error, line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }

    Mutex::Autolock lock(mLock);

    err = writeNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::checkDataVersion(
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[checkDataVersion]\n");

    if (a_eNvramDataType == CAMERA_NVRAM_DATA_ISP) { // ISP
        PNVRAM_CAMERA_ISP_PARAM_STRUCT pCameraNvramData = (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[ISP] NVRAM data version = %d; F/W data version = %d\n", pCameraNvramData->Version, NVRAM_CAMERA_PARA_FILE_VERSION);

        if (pCameraNvramData->Version != NVRAM_CAMERA_PARA_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_3A) { // 3A
        PNVRAM_CAMERA_3A_STRUCT p3ANvramData = (PNVRAM_CAMERA_3A_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[3A] NVRAM data version = %d; F/W data version = %d\n", p3ANvramData->u4Version, NVRAM_CAMERA_3A_FILE_VERSION);

        if (p3ANvramData->u4Version != NVRAM_CAMERA_3A_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_SHADING) { // Shading
        PNVRAM_CAMERA_SHADING_STRUCT pShadingNvramData = (PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[Shading] NVRAM data version = %d; F/W data version = %d\n", pShadingNvramData->Shading.Version, NVRAM_CAMERA_SHADING_FILE_VERSION);

        if (pShadingNvramData->Shading.Version != NVRAM_CAMERA_SHADING_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_LENS) { // Lens
        PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[Lens] NVRAM data version = %d; F/W data version = %d\n", pLensNvramData->Version, NVRAM_CAMERA_LENS_FILE_VERSION);

        if (pLensNvramData->Version != NVRAM_CAMERA_LENS_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_STROBE) { // strobe
        PNVRAM_CAMERA_STROBE_STRUCT pStrobeNvramData = (PNVRAM_CAMERA_STROBE_STRUCT)a_pNvramData;
        NVRAM_DRV_LOG("[Strobe] NVRAM data version = %d; F/W data version = %d\n", pStrobeNvramData->u4Version, NVRAM_CAMERA_STROBE_FILE_VERSION);
        if (pStrobeNvramData->u4Version != NVRAM_CAMERA_STROBE_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else {
        NVRAM_DRV_ERR("checkDataVersion(): incorrect data type\n");
}

    return err;
}


static int getMs()
{
    int t;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (ts.tv_sec*1000+ts.tv_nsec/1000000);
    return t;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readNvramData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{

    NVRAM_DRV_LOG("[readNvramData]+ a_eSensorType=%d a_eNvramDataType=%d", a_eSensorType, a_eNvramDataType);
    F_ID rNvramFileID;
    int i4FileInfo;
    int i4RecSize;
    int i4RecNum;
//seanlin 121221 avoid camera has not inited>
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    if (!bCustomInit) {
        cameraCustomInit();
        if((a_eSensorType==DUAL_CAMERA_MAIN_2_SENSOR) || (a_eSensorType==DUAL_CAMERA_MAIN_SECOND_SENSOR))
            LensCustomInit(8);
        else
            LensCustomInit((unsigned int)a_eSensorType);
        bCustomInit = 1;
    }
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
//seanlin 121221 avoid camera has not inited<
    if(a_eNvramDataType==CAMERA_NVRAM_DATA_SHADING)
    {
        //int NvramDrv::readMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
        int idList[]=
        {
            AP_CFG_RDCL_CAMERA_SHADING_LID,
            AP_CFG_RDCL_CAMERA_SHADING2_LID,
            AP_CFG_RDCL_CAMERA_SHADING3_LID,
            AP_CFG_RDCL_CAMERA_SHADING4_LID,
            AP_CFG_RDCL_CAMERA_SHADING5_LID,
            AP_CFG_RDCL_CAMERA_SHADING6_LID,
            AP_CFG_RDCL_CAMERA_SHADING7_LID,
            AP_CFG_RDCL_CAMERA_SHADING8_LID,
            AP_CFG_RDCL_CAMERA_SHADING9_LID,
            AP_CFG_RDCL_CAMERA_SHADING10_LID,
            AP_CFG_RDCL_CAMERA_SHADING11_LID,
            AP_CFG_RDCL_CAMERA_SHADING12_LID,
        };
        int err;
        err = readMultiNvram(a_pNvramData, sizeof(ISP_SHADING_STRUCT), idList, 12, a_eSensorType);
        return err;
    }
    else if(a_eNvramDataType==CAMERA_DATA_AE_PLINETABLE)
    {
        //int NvramDrv::readMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
        int idList[]=
        {
            AP_CFG_RDCL_CAMERA_PLINE_LID,
            AP_CFG_RDCL_CAMERA_PLINE2_LID,
            AP_CFG_RDCL_CAMERA_PLINE3_LID,
            AP_CFG_RDCL_CAMERA_PLINE4_LID,
            AP_CFG_RDCL_CAMERA_PLINE5_LID,
            AP_CFG_RDCL_CAMERA_PLINE6_LID,
            AP_CFG_RDCL_CAMERA_PLINE7_LID,
            AP_CFG_RDCL_CAMERA_PLINE8_LID,
            AP_CFG_RDCL_CAMERA_PLINE9_LID,
            AP_CFG_RDCL_CAMERA_PLINE10_LID,
            AP_CFG_RDCL_CAMERA_PLINE11_LID,
            AP_CFG_RDCL_CAMERA_PLINE12_LID,
        };
        int err;
        err = readMultiNvram(a_pNvramData, sizeof(AE_PLINETABLE_T), idList, 12, a_eSensorType);
        return err;
    }


    switch (a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
        i4FileInfo = AP_CFG_RDCL_CAMERA_PARA_LID;
        break;
    case CAMERA_NVRAM_DATA_3A:
        i4FileInfo = AP_CFG_RDCL_CAMERA_3A_LID;
        break;
    case CAMERA_NVRAM_DATA_SHADING:
        i4FileInfo = AP_CFG_RDCL_CAMERA_SHADING_LID;
        break;
    case CAMERA_NVRAM_DATA_LENS:
        i4FileInfo = AP_CFG_RDCL_CAMERA_LENS_LID;
        break;
    case CAMERA_NVRAM_DATA_STROBE:
        i4FileInfo = AP_CFG_RDCL_CAMERA_DEFECT_LID;
        break;
    case CAMERA_NVRAM_DATA_FEATURE:
        i4FileInfo = AP_CFG_RDCL_CAMERA_FEATURE_LID;
        break;
    case CAMERA_NVRAM_DATA_GEOMETRY:
        i4FileInfo = AP_CFG_RDCL_CAMERA_GEOMETRY_LID;
        break;
    case CAMERA_DATA_AE_PLINETABLE:
        i4FileInfo =     AP_CFG_RDCL_CAMERA_PLINE_LID;
        break;

    case CAMERA_NVRAM_VERSION:
        a_eSensorType = DUAL_CAMERA_MAIN_SENSOR;
        i4FileInfo = AP_CFG_RDCL_CAMERA_VERSION_LID;
        break;
    default :
        NVRAM_DRV_ERR("readNvramData(): incorrect data type\n");
        return NVRAM_READ_PARAMETER_ERROR;
        break;
    }

#ifdef NVRAM_SUPPORT


    int ms1;
    int ms2;
    ms1 = getMs();
    while(1)
    {
//        logI("check nvram demon");
        NVRAM_DRV_LOG("check nvram demon");

        char nvram_init_val[PROPERTY_VALUE_MAX];
        property_get("service.nvram_init", nvram_init_val, NULL);
        if (strcmp(nvram_init_val, "Ready") == 0 || strcmp(nvram_init_val, "Pre_Ready") == 0)
        {
            break;
        }
        ms2 = getMs();
        if((ms2-ms1)>10000)
        {
            logE("error demomn %d ms, timeout", ms2-ms1);
            break;
        }
        usleep(100*1000);
    }

    if(mAndroidMode==0)
    {
    rNvramFileID = NVM_GetFileDesc(i4FileInfo, &i4RecSize, &i4RecNum, ISREAD);
    if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
        NVRAM_DRV_ERR("readNvramData(): create NVRAM file fail\n");
        return NVRAM_CAMERA_FILE_ERROR;
    }

    if (a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
    }

    if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize*2, SEEK_SET);
    }

    read(rNvramFileID.iFileDesc, a_pNvramData, i4RecSize);

    NVM_CloseFileDesc(rNvramFileID);
}
    else
    {
#if SUPPORT_BINDER_NVRAM
    int sh=0;
    if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR)
        sh=2;
    else if (a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR)
        sh=1;
    int ret;
    ret = binderReadNvram(i4FileInfo, a_pNvramData, sh);
#endif

    }

#endif
    unsigned char* dd;
    dd = (unsigned char*)a_pNvramData;
    NVRAM_DRV_LOG("[readNvramData]- %d %d %d %d %d %d %d %d %d %d",
        (int)dd[0], (int)dd[1], (int)dd[2], (int)dd[3], (int)dd[4],
        (int)dd[5], (int)dd[6], (int)dd[7], (int)dd[8], (int)dd[9]);

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvramData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    F_ID rNvramFileID;
    int i4FileInfo;
    int i4RecSize;
    int i4RecNum;

    NVRAM_DRV_LOG("[writeNvramData]+ a_eSensorType=%d a_eNvramDataType=%d", a_eSensorType, a_eNvramDataType);
    unsigned char* dd;
    dd = (unsigned char*)a_pNvramData;
    NVRAM_DRV_LOG("[writeNvramData] %d %d %d %d %d %d %d %d %d %d",
        (int)dd[0], (int)dd[1], (int)dd[2], (int)dd[3], (int)dd[4],
        (int)dd[5], (int)dd[6], (int)dd[7], (int)dd[8], (int)dd[9]);


    if(a_eNvramDataType==CAMERA_NVRAM_DATA_SHADING)
    {
        //int NvramDrv::readMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
        int idList[]=
        {
            AP_CFG_RDCL_CAMERA_SHADING_LID,
            AP_CFG_RDCL_CAMERA_SHADING2_LID,
            AP_CFG_RDCL_CAMERA_SHADING3_LID,
            AP_CFG_RDCL_CAMERA_SHADING4_LID,
            AP_CFG_RDCL_CAMERA_SHADING5_LID,
            AP_CFG_RDCL_CAMERA_SHADING6_LID,
            AP_CFG_RDCL_CAMERA_SHADING7_LID,
            AP_CFG_RDCL_CAMERA_SHADING8_LID,
            AP_CFG_RDCL_CAMERA_SHADING9_LID,
            AP_CFG_RDCL_CAMERA_SHADING10_LID,
            AP_CFG_RDCL_CAMERA_SHADING11_LID,
            AP_CFG_RDCL_CAMERA_SHADING12_LID,
        };
        int err;
        err = writeMultiNvram(a_pNvramData, sizeof(ISP_SHADING_STRUCT), idList, 12, a_eSensorType);
        return err;
    }
    else if(a_eNvramDataType==CAMERA_DATA_AE_PLINETABLE)
    {
        //int NvramDrv::readMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
        int idList[]=
        {
            AP_CFG_RDCL_CAMERA_PLINE_LID,
            AP_CFG_RDCL_CAMERA_PLINE2_LID,
            AP_CFG_RDCL_CAMERA_PLINE3_LID,
            AP_CFG_RDCL_CAMERA_PLINE4_LID,
            AP_CFG_RDCL_CAMERA_PLINE5_LID,
            AP_CFG_RDCL_CAMERA_PLINE6_LID,
            AP_CFG_RDCL_CAMERA_PLINE7_LID,
            AP_CFG_RDCL_CAMERA_PLINE8_LID,
            AP_CFG_RDCL_CAMERA_PLINE9_LID,
            AP_CFG_RDCL_CAMERA_PLINE10_LID,
            AP_CFG_RDCL_CAMERA_PLINE11_LID,
            AP_CFG_RDCL_CAMERA_PLINE12_LID,
        };
        int err;
        err = writeMultiNvram(a_pNvramData, sizeof(AE_PLINETABLE_T), idList, 12, a_eSensorType);
        return err;
    }

    switch (a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
        i4FileInfo = AP_CFG_RDCL_CAMERA_PARA_LID;
        break;
    case CAMERA_NVRAM_DATA_3A:
        i4FileInfo = AP_CFG_RDCL_CAMERA_3A_LID;
        break;
    case CAMERA_NVRAM_DATA_SHADING:
        i4FileInfo = AP_CFG_RDCL_CAMERA_SHADING_LID;
        break;
    case CAMERA_NVRAM_DATA_LENS:
        i4FileInfo = AP_CFG_RDCL_CAMERA_LENS_LID;
        break;
    case CAMERA_NVRAM_DATA_STROBE:
        i4FileInfo = AP_CFG_RDCL_CAMERA_DEFECT_LID;
        break;
    case CAMERA_NVRAM_DATA_FEATURE:
        i4FileInfo = AP_CFG_RDCL_CAMERA_FEATURE_LID;
        break;
    case CAMERA_NVRAM_DATA_GEOMETRY:
        i4FileInfo = AP_CFG_RDCL_CAMERA_GEOMETRY_LID;
        break;
    case CAMERA_DATA_AE_PLINETABLE:
        i4FileInfo =     AP_CFG_RDCL_CAMERA_PLINE_LID;
        break;
    case CAMERA_NVRAM_VERSION:
        a_eSensorType = DUAL_CAMERA_MAIN_SENSOR;
        i4FileInfo = AP_CFG_RDCL_CAMERA_VERSION_LID;
        break;
    default:
        NVRAM_DRV_ERR("writeNvramData(): incorrect data type\n");
        return NVRAM_WRITE_PARAMETER_ERROR;
        break;
    }

#ifdef NVRAM_SUPPORT

int ms1;
int ms2;
ms1 = getMs();
while(1)
{

    char nvram_init_val[PROPERTY_VALUE_MAX];
    property_get("service.nvram_init", nvram_init_val, NULL);
    if (strcmp(nvram_init_val, "Ready") == 0 || strcmp(nvram_init_val, "Pre_Ready") == 0)
    {
        break;
    }
    ms2 = getMs();
    if((ms2-ms1)>10000)
    {
        logE("error demomn %d ms, timeout", ms2-ms1);
        break;
    }
    usleep(100*1000);
}


if(mAndroidMode==0)
    {
    rNvramFileID = NVM_GetFileDesc(i4FileInfo, &i4RecSize, &i4RecNum, ISWRITE);
    if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
        NVRAM_DRV_ERR("writeNvramData(): create NVRAM file fail\n");
        return NVRAM_CAMERA_FILE_ERROR;
    }

    if (a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
    }

    if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize*2, SEEK_SET);
    }

    write(rNvramFileID.iFileDesc, a_pNvramData, i4RecSize);

    NVM_CloseFileDesc(rNvramFileID);
}
else
{
#if SUPPORT_BINDER_NVRAM
    int sh=0;
    if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR)
        sh=2;
    else if (a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR)
        sh=1;
    int ret;
    ret = binderWriteNvram(i4FileInfo, a_pNvramData, sh);
#endif

}
#endif

    NVRAM_DRV_LOG("[ writeNvramData]-");

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readDefaultData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{

//    static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    NVRAM_DRV_LOG("[readDefaultData] sensor ID = %ld; NVRAM data type = %d\n", a_u4SensorID, a_eNvramDataType);

#ifdef NVRAM_SUPPORT

    if (!bCustomInit) {
        cameraCustomInit();
        if((a_eSensorType==DUAL_CAMERA_MAIN_2_SENSOR) || (a_eSensorType==DUAL_CAMERA_MAIN_SECOND_SENSOR))
            LensCustomInit(8);
        else
            LensCustomInit((unsigned int)a_eSensorType);
        bCustomInit = 1;
    }

    switch (a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
        GetCameraDefaultPara(a_u4SensorID, (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData,NULL,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_3A:
        GetCameraDefaultPara(a_u4SensorID, NULL,(PNVRAM_CAMERA_3A_STRUCT)a_pNvramData,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_SHADING:
        GetCameraDefaultPara(a_u4SensorID, NULL,NULL,(PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData,NULL);
        break;
    case CAMERA_NVRAM_DATA_LENS:
        GetLensDefaultPara((PNVRAM_LENS_PARA_STRUCT)a_pNvramData);
        {
            PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;
            pLensNvramData->Version = NVRAM_CAMERA_LENS_FILE_VERSION;
        }
        break;
    case CAMERA_DATA_AE_PLINETABLE:
        GetCameraDefaultPara(a_u4SensorID, NULL,NULL,NULL,(PAE_PLINETABLE_STRUCT)a_pNvramData);
        break;

    case CAMERA_NVRAM_DATA_STROBE:
        int ret;
        ret = cust_fillDefaultStrobeNVRam(a_eSensorType, a_pNvramData);
        break;

    case CAMERA_DATA_TSF_TABLE:
        if (0 != GetCameraTsfDefaultTbl(a_u4SensorID, (PCAMERA_TSF_TBL_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
    case CAMERA_NVRAM_DATA_FEATURE:
        //NVRAM_DRV_LOG("[readDefaultData] feature line=%d",__LINE__);
        GetCameraFeatureDefault(a_u4SensorID, (NVRAM_CAMERA_FEATURE_STRUCT*)a_pNvramData);
        break;

    default:
        break;
    }

#endif

    return NVRAM_NO_ERROR;
}


int nvGetFlickerPara(MUINT32 SensorId, int SensorMode, void* buf)
{
    NVRAM_DRV_LOG("nvGetFlickerPara id=%d mode=%d", SensorId, SensorMode);
    int err;
    err = msdkGetFlickerPara(SensorId, SensorMode, buf);
    if(err!=0)
        NVRAM_DRV_LOG("nvGetFlickerPara error:=%d", err);
    return err;
}
#if SUPPORT_BINDER_NVRAM
int binderGetNvramDisc(int file_lid, int& rec_size, int& rec_num)
{
    //ALOGD("%s(), file_lid = %d, pBuf = %p, gUseBinderToAccessNVRam = %d", __FUNCTION__, file_lid, pBuf, gUseBinderToAccessNVRam);



    //int rec_size = 0;
    //int rec_num = 0;
    NvRAMAgentClient *NvRAMClient = NvRAMAgentClient::create();
    if (NvRAMClient == NULL)
    {
        ALOGE("%s(), NvRAMClient == NULL", __FUNCTION__);
        return -1;
    }
    else
    {
        int result;
        result = NvRAMClient->getFileDesSize(file_lid, rec_size, rec_num);
        delete NvRAMClient;
        if(result==0)
            return -1;
        else
            return 0;
    }

}


int binderReadNvram(int file_lid, void *pBuf, int sh)
{
    //reference code:
    //alps\mediatek\external\audiocustparam\AudioCustParam.cpp
    //audioReadNVRamFile ()
    //audioWriteNVRamFile()
    logI("%s(), file_lid = %d, pBuf = %p sh=%d", __FUNCTION__, file_lid, pBuf, sh);
    int ret;
    int rec_size;
    int rec_num;
    ret = binderGetNvramDisc(file_lid, rec_size, rec_num);
    if(ret!=0)
    {
        logE("binderGetNvramDisc");
        return -1;
    }
    logI("file_lid = %d, rec_size=%d rec_num=%d", file_lid, rec_size, rec_num);

    if (pBuf == NULL)
    {
        logE("%s(), pBuf == NULL, return 0", __FUNCTION__);
        return -1;
    }

    int result = 0;
    int read_size = 0;

    NvRAMAgentClient *NvRAMClient = NvRAMAgentClient::create();
    if (NvRAMClient == NULL)
    {
        logE("%s(), NvRAMClient == NULL", __FUNCTION__);
        result = -1;
    }
    else
    {

        char *data = NvRAMClient->readFile(file_lid, read_size);
        if (data == NULL)
        {
            logE("%s(), data == NULL", __FUNCTION__);
            result = -1;
        }
        else
        {
            if(rec_size*rec_num!=read_size)
            {
                logE("size is not same read_size=%d", read_size);
                result = -1;
            }
            else
            {
                logI("%s(), data = %p, read_size = %d", __FUNCTION__, data, read_size);
                memcpy(pBuf, data+rec_size*sh, rec_size);
                free(data);
                result = 0;
            }
        }
        delete NvRAMClient;
    }
    return result;
}


int binderWriteNvram(int file_lid, void *pBuf, int sh)
{
    logI("%s() file_lid = %d, pBuf = %p, sh=%d", __FUNCTION__, file_lid, pBuf, sh);
    logI("%s() buf[0]=%d", __FUNCTION__, (((int*)pBuf)[0]));
    int ret;
    int rec_size;
    int rec_num;
    ret = binderGetNvramDisc(file_lid, rec_size, rec_num);
    if(ret!=0)
    {
        logE("binderGetNvramDisc");
        return -1;
    }
    logI("file_lid = %d, rec_size=%d rec_num=%d", file_lid, rec_size, rec_num);


    if (pBuf == NULL)
    {
        logE("%s(), pBuf == NULL, return 0", __FUNCTION__);
        return -1;
    }

    int result = 0;
    int write_size = 0;


    NvRAMAgentClient *NvRAMClient = NvRAMAgentClient::create();
    if (NvRAMClient == NULL)
    {
        logE("%s(), NvRAMClient == NULL", __FUNCTION__);
        return -1;
    }
    else
    {
        write_size = NvRAMClient->writeFileEx(file_lid, sh, rec_size, (char *)pBuf);
        delete NvRAMClient;
    }

    logI("%s() writeSize=%d", __FUNCTION__, write_size);
    return 0;
}

#endif

int NvramDrv::writeNvramReal(void* buf, int id, int dev)
{
    logI("writeNvramReal %p %d %d", buf, id, dev);
    F_ID rNvramFileID;
    int i4RecSize;
    int i4RecNum;

#ifdef NVRAM_SUPPORT

    if(mAndroidMode==0)
    {
    rNvramFileID = NVM_GetFileDesc(id, &i4RecSize, &i4RecNum, ISWRITE);
    if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
        NVRAM_DRV_ERR("writeNvramData(): create NVRAM file fail\n");
        return NVRAM_CAMERA_FILE_ERROR;
    }

    if (dev == DUAL_CAMERA_MAIN_SECOND_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
    }

    if (dev == DUAL_CAMERA_SUB_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize*2, SEEK_SET);
    }

    write(rNvramFileID.iFileDesc, buf, i4RecSize);

    NVM_CloseFileDesc(rNvramFileID);
}
else
{
#if SUPPORT_BINDER_NVRAM
    int sh=0;
    if (dev == DUAL_CAMERA_SUB_SENSOR)
        sh=2;
    else if (dev == DUAL_CAMERA_MAIN_SECOND_SENSOR)
        sh=1;
    int ret;
    ret = binderWriteNvram(id, buf, sh);
#endif

}
#endif

    logI("writeNvramReal-");

    return 0;
}



int NvramDrv::readNvramReal(void* buf, int id, int dev)
{
    logI("readNvramReal %p %d %d", buf, id, dev);
    F_ID rNvramFileID;
    //int i4FileInfo;
    int i4RecSize;
    int i4RecNum;
//seanlin 121221 avoid camera has not inited>
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    if (!bCustomInit) {
        cameraCustomInit();
        if((dev==DUAL_CAMERA_MAIN_2_SENSOR) || (dev==DUAL_CAMERA_MAIN_SECOND_SENSOR))
            LensCustomInit(8);
        else
            LensCustomInit((unsigned int)dev);
        bCustomInit = 1;
    }
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
//seanlin 121221 avoid camera has not inited<




#ifdef NVRAM_SUPPORT

    if(mAndroidMode==0)
    {
    rNvramFileID = NVM_GetFileDesc(id, &i4RecSize, &i4RecNum, ISREAD);
    if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
        NVRAM_DRV_ERR("readNvramData(): create NVRAM file fail\n");
        return NVRAM_CAMERA_FILE_ERROR;
    }

    if (dev == DUAL_CAMERA_MAIN_SECOND_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
    }

    if (dev == DUAL_CAMERA_SUB_SENSOR) {
        lseek(rNvramFileID.iFileDesc, i4RecSize*2, SEEK_SET);
    }

    read(rNvramFileID.iFileDesc, buf, i4RecSize);

    NVM_CloseFileDesc(rNvramFileID);
}
    else
    {

#if SUPPORT_BINDER_NVRAM
    int sh=0;
    if (dev == DUAL_CAMERA_SUB_SENSOR)
        sh=2;
    else if (dev == DUAL_CAMERA_MAIN_SECOND_SENSOR)
        sh=1;
    int ret;
    ret = binderReadNvram(id, buf, sh);
#endif

}

#endif

    logI("readNvramReal-");



    return 0;
}

//i4FileInfo
//a_pNvramData



//#define NV_REC_SZ 80000

#define NV_REC_SZ1 MAXIMUM_NVRAM_CAMERA_PLINE_FILE_SIZE
#define NV_REC_SZ2 MAXIMUM_NVRAM_CAMERA_SHADING_FILE_SIZE

int NvramDrv::readMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
{
    int NV_REC_SZ=NV_REC_SZ1;
    if(NV_REC_SZ1!=NV_REC_SZ2)
    {
        logE("pline and shading file size is not same!");
        NV_REC_SZ=NV_REC_SZ1>NV_REC_SZ2?NV_REC_SZ1:NV_REC_SZ2;
    }


    if(num*NV_REC_SZ<bufSz)
        logE("bufSz=%d readSize=%d (%dx%d)",bufSz, num*NV_REC_SZ, num, NV_REC_SZ);
    int restSz=bufSz;
    char* pCpBuf;
    int cpSz;
    int i;
    char* b;
    b = new char [NV_REC_SZ];
    pCpBuf = (char*)buf;
    for(i=0;i<num;i++)
    {
        if(restSz<=0)
            break;
        readNvramReal(b, idList[i], dev);
        if(restSz>NV_REC_SZ)
            cpSz = NV_REC_SZ;
        else
            cpSz = restSz;
        memcpy(pCpBuf, b, cpSz);
        pCpBuf += cpSz;
        restSz-=cpSz;
    }
    delete []b;
    return 0;
}


int NvramDrv::writeMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
{
    int NV_REC_SZ=NV_REC_SZ1;
    if(NV_REC_SZ1!=NV_REC_SZ2)
    {
        logE("pline and shading file size is not same!");
        NV_REC_SZ=NV_REC_SZ1>NV_REC_SZ2?NV_REC_SZ1:NV_REC_SZ2;
    }
    if(num*NV_REC_SZ<bufSz)
        logE("bufSz=%d readSize=%d (%dx%d)",bufSz, num*NV_REC_SZ, num, NV_REC_SZ);
    int restSz=bufSz;
    char* pCpBuf;
    int cpSz;
    int i;
    char* b;
    b = new char [NV_REC_SZ];
    pCpBuf = (char*)buf;
    for(i=0;i<num;i++)
    {
        if(restSz<=0)
            break;

        if(restSz>NV_REC_SZ)
            cpSz = NV_REC_SZ;
        else
            cpSz = restSz;
        memcpy(b, pCpBuf, cpSz);
        writeNvramReal(b, idList[i], dev);
        pCpBuf += cpSz;
        restSz-=cpSz;
    }
    delete []b;
    return 0;
}
