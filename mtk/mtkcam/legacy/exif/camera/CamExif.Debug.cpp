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
#define LOG_TAG "CamExifDebug"
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/properties.h>
//
#include <Log.h>
#include <common.h>
//
#include "exif/IBaseCamExif.h"
#include "exif/CamExif.h"
//
#include <IBaseExif.h>
//
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/aaa/dbg_aaa_param.h>
#include <debug_exif/aaa/dbg_isp_param.h>
#include <debug_exif/cam/dbg_cam_param.h>
#include <debug_exif/eis/dbg_eis_param.h>
//

/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)


/*******************************************************************************
*
********************************************************************************/
inline void setDebugTag(DEBUG_CMN_INFO_S &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_CMN_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
*
********************************************************************************/
void
CamExif::
getCommonDebugInfo(DEBUG_CMN_INFO_S &a_rDbgCommonInfo)
{
    MBOOL  ret = MTRUE;
    MUINT32 u4ID = 0;

    // Cam Debug Info
    memset(&a_rDbgCommonInfo, 0, sizeof(DEBUG_CMN_INFO_T));

    // Cam Debug Version
    setDebugTag(a_rDbgCommonInfo, CMN_TAG_VERSION, (MUINT32)CMN_DEBUG_TAG_VERSION);

    // Shot mode
    setDebugTag(a_rDbgCommonInfo, CMN_TAG_SHOT_MODE, mCamDbgParam.u4ShotMode);

    // Camera mode: Normal, Engineer
    setDebugTag(a_rDbgCommonInfo, CMN_TAG_CAM_MODE, mCamDbgParam.u4CamMode);

    ret = sendCommand(CMD_REGISTER, DEBUG_CAM_CMN_MID, reinterpret_cast<MUINTPTR>(&u4ID));
    if (!ret) {
        MY_LOGE("ERROR: CMD_REGISTER");
    }

    ret = sendCommand(CMD_SET_DBG_EXIF, u4ID, (MUINTPTR) &a_rDbgCommonInfo, sizeof(DEBUG_CMN_INFO_S));
    if (!ret) {
        MY_LOGE("ERROR: ID_CMN");
    }

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
getCamDebugInfo(
    MUINT8* const pDbgInfo,
    MUINT32 const rDbgSize,
    MINT32 const dbgModuleID
)
{
    MUINT8* pDbgModuleInfo = NULL;
    MUINT32 dbgModuleSize = 0;
    if (mi4DbgModuleType & dbgModuleID) {
        MUINT32 moduleIndex = mMapModuleID.valueFor(dbgModuleID);
        //
        MY_LOGI("Get: ID_%s(0x%04x), Size(%d), Addr(%p)",
            mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID,
            mDbgInfo[moduleIndex].u4BufSize, (MUINTPTR)&mDbgInfo[moduleIndex].puDbgBuf);
        //
        pDbgModuleInfo = mDbgInfo[moduleIndex].puDbgBuf;
        dbgModuleSize = mDbgInfo[moduleIndex].u4BufSize;
        //
    }
    else {
        MY_LOGW("ID_%s(0x%04x) did not exist.", mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID);
        return MFALSE;
    }
    //
    if ( dbgModuleSize > rDbgSize) {
        MY_LOGE("ID_%s(0x%04x) ERROR size! (%d)>(%d)",
            mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID, dbgModuleSize, rDbgSize);
        return MFALSE;
    }
    else {
        memcpy(pDbgInfo, pDbgModuleInfo, rDbgSize);
    }
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
appendDebugInfo(
    MINT32 const dbgModuleID,       //  [I] debug module ID
    MINT32 const dbgAppn,           //  [I] APPn
    MUINT8** const ppuAppnBuf,      //  [O] Pointer to APPn Buffer
    MUINT32* const pu4AppnSize      //  [O] Pointer to APPn Size
)
{
    MBOOL  ret = MTRUE;

    if (mi4DbgModuleType & dbgModuleID)
    {
        MUINT32 appnReturnSize = 0;
        MUINT8* pDbgModuleInfo = NULL;
        MUINT32 dbgModuleSize = 0;
        MUINT32 dbgModuleIndex = mMapModuleID.valueFor(dbgModuleID);
        //
        MY_LOGI("Get: ID_%s(0x%04x), Index(%d), Size(%d), Addr(%p), APP%d",
            mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID,
            dbgModuleIndex, mDbgInfo[dbgModuleIndex].u4BufSize, (MUINTPTR)mDbgInfo[dbgModuleIndex].puDbgBuf, dbgAppn);
        //
        pDbgModuleInfo = mDbgInfo[dbgModuleIndex].puDbgBuf;
        dbgModuleSize = mDbgInfo[dbgModuleIndex].u4BufSize + 2; // Data(n bytes) + Data size(2 bytes)
        //
        if ( (dbgModuleSize > 2) && (dbgModuleSize < 64*1024) )
        {
            mpBaseExif->exifAppnMake(dbgAppn, *ppuAppnBuf, pDbgModuleInfo, dbgModuleSize, &appnReturnSize);
            //
            *ppuAppnBuf += appnReturnSize;
            *pu4AppnSize += appnReturnSize;
        }
        else
        {
            MY_LOGW("dbgModuleSize(%d)", dbgModuleSize);
            ret = MFALSE;
        }
    }
    else
    {
        MY_LOGW("ID_%s(0x%04x) did not exist", mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID);
        ret = MFALSE;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
appendCamDebugInfo(
    MUINT32 const dbgAppn,          // [I] APPn for CAM module
    MUINT8** const ppuAppnBuf,      //  [O] Pointer to APPn Buffer
    MUINT32* const pu4AppnSize      //  [O] Pointer to APPn Size
)
{
    MBOOL  ret = MFALSE;

    //// CAM debug info: Common, MF, N3D, Sensor, Shading
    {
        // COMMON
        DEBUG_CMN_INFO_T rDbgCommonInfo;
        memset(&rDbgCommonInfo, 0, sizeof(DEBUG_CMN_INFO_T));
        getCommonDebugInfo(rDbgCommonInfo);

        //// CAM
        MUINT32 app5ReturnSize = 0;
        MUINT32 dbgCamHeaderSize = 0;
        MUINT8* pDbgCamInfo = NULL;
        MUINT32 dbgCamSize = 0;
        DEBUG_CAM_INFO_T rDbgCamInfo;
        memset(&rDbgCamInfo, 0, sizeof(DEBUG_CAM_INFO_T));
        dbgCamHeaderSize = sizeof(rDbgCamInfo.hdr);

        rDbgCamInfo.hdr.u4KeyID = DEBUG_CAM_KEYID;
        rDbgCamInfo.hdr.u4ModuleCount = MODULE_NUM(DEBUF_CAM_TOT_MODULE_NUM, DEBUF_CAM_TAG_MODULE_NUM);
        rDbgCamInfo.hdr.u4DbgCMNInfoOffset      = dbgCamHeaderSize;
        rDbgCamInfo.hdr.u4DbgMFInfoOffset       = rDbgCamInfo.hdr.u4DbgCMNInfoOffset + sizeof(DEBUG_CMN_INFO_T);
        rDbgCamInfo.hdr.u4DbgN3DInfoOffset      = rDbgCamInfo.hdr.u4DbgMFInfoOffset + sizeof(DEBUG_MF_INFO_T);
        rDbgCamInfo.hdr.u4DbgSENSORInfoOffset   = rDbgCamInfo.hdr.u4DbgN3DInfoOffset + sizeof(DEBUG_N3D_INFO_T);
        rDbgCamInfo.hdr.u4DbgRESERVEAInfoOffset = rDbgCamInfo.hdr.u4DbgSENSORInfoOffset + sizeof(DEBUG_SENSOR_INFO_T);
        rDbgCamInfo.hdr.u4DbgRESERVEBInfoOffset = rDbgCamInfo.hdr.u4DbgRESERVEAInfoOffset + sizeof(DEBUG_RESERVEA_INFO_T);
        rDbgCamInfo.hdr.u4DbgRESERVECInfoOffset = rDbgCamInfo.hdr.u4DbgRESERVEBInfoOffset + sizeof(DEBUG_RESERVEB_INFO_T);
        //
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgCMNInfo,      sizeof(DEBUG_CMN_INFO_T),       ID_CMN);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgMFInfo,       sizeof(DEBUG_MF_INFO_T),        ID_MF);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgN3DInfo,      sizeof(DEBUG_N3D_INFO_T),       ID_N3D);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgSENSORInfo,   sizeof(DEBUG_SENSOR_INFO_T),    ID_SENSOR);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgRESERVEAInfo, sizeof(DEBUG_RESERVEA_INFO_T),  ID_RESERVE1);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgRESERVEBInfo, sizeof(DEBUG_RESERVEB_INFO_T),  ID_RESERVE2);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgRESERVECInfo, sizeof(DEBUG_RESERVEC_INFO_T),  ID_RESERVE3);
        //
        pDbgCamInfo = (MUINT8*) &rDbgCamInfo;
        dbgCamSize  = sizeof(DEBUG_CAM_INFO_T) + 2; // Data(n bytes) + Data size(2 bytes)

        if ( pDbgCamInfo && dbgCamSize > 2)
        {
            mpBaseExif->exifAppnMake(dbgAppn, *ppuAppnBuf, pDbgCamInfo, dbgCamSize, &app5ReturnSize);
            //
            *ppuAppnBuf += app5ReturnSize;
            *pu4AppnSize += app5ReturnSize;
        }
        else
        {
            MY_LOGE("dbgCamSize(%d) < 0", dbgCamSize);
            goto lbExit;
        }
    }

    ret = MTRUE;

lbExit:
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
appendDebugExif(
    MUINT8* const puAppnBuf,        //  [O] Pointer to APPn Buffer
    MUINT32* const pu4AppnSize,     //  [O] Pointer to APPn Size
    MUINT32 const u4SensorIdx       // [I] Sensor index. default value is 0. (0 means 1st cam, 1 means 2nd cam)
)
{
    MBOOL  ret = MTRUE;
    MUINT8* pDst = puAppnBuf;
    MUINT32 u4AppnSize = 0;
    MUINT32 u4AppnCAM       = ( 0 == u4SensorIdx ) ? 5 : 10;
    MUINT32 u4AppnAAA       = ( 0 == u4SensorIdx ) ? 6 : 11;
    MUINT32 u4AppnISP       = ( 0 == u4SensorIdx ) ? 7 : 12;
    MUINT32 u4AppnSHAD_TBL  = ( 0 == u4SensorIdx ) ? 8 : 13;

    if ( 0 == u4SensorIdx ) {
        //// EIS debug info
        // [NOTE!!] only 1st cam has EIS debug info.
        appendDebugInfo(ID_EIS, 4, &pDst, &u4AppnSize);
    }

    //// CAM debug info
    appendCamDebugInfo(u4AppnCAM, &pDst, &u4AppnSize);

    //// AAA debug info
    appendDebugInfo(ID_AAA, u4AppnAAA, &pDst, &u4AppnSize);

    //// ISP debug info
    appendDebugInfo(ID_ISP, u4AppnISP, &pDst, &u4AppnSize);

    //// SHAD_ARRAY debug info
    appendDebugInfo(ID_SHAD_TABLE, u4AppnSHAD_TBL, &pDst, &u4AppnSize);

    if  (pu4AppnSize)
    {
        *pu4AppnSize = u4AppnSize;
    }

lbExit:
    return  ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
sendCommand(MINT32 cmd, MINT32 parg1, MUINTPTR parg2, MINT32 parg3)
{
    MBOOL ret = MTRUE;

    //1.Special command
    //MY_LOGI("[sendCommand] cmd: 0x%x \n", cmd);

    switch (cmd)
    {
        case CMD_REGISTER:
        {
            MUINT32 registerName = (MUINT32) parg1;
            //
            switch (registerName) {
                case AAA_DEBUG_KEYID:
                    *(MINT32 *) parg2 = ID_AAA;
                    break;
                case ISP_DEBUG_KEYID:
                    *(MINT32 *) parg2 = ID_ISP;
                    break;
                case DEBUG_EIS_MID:
                    *(MINT32 *) parg2 = ID_EIS;
                    break;
                case DEBUG_CAM_CMN_MID:
                    *(MINT32 *) parg2 = ID_CMN;
                    break;
                case DEBUG_CAM_MF_MID:
                    *(MINT32 *) parg2 = ID_MF;
                    break;
                case DEBUG_CAM_N3D_MID:
                    *(MINT32 *) parg2 = ID_N3D;
                    break;
                case DEBUG_CAM_SENSOR_MID:
                    *(MINT32 *) parg2 = ID_SENSOR;
                    break;
                case DEBUG_CAM_RESERVE1_MID:
                    *(MINT32 *) parg2 = ID_RESERVE1;
                    break;
                case DEBUG_CAM_RESERVE2_MID:
                    *(MINT32 *) parg2 = ID_RESERVE2;
                    break;
                case DEBUG_CAM_RESERVE3_MID:
                    *(MINT32 *) parg2 = ID_RESERVE3;
                    break;
                case DEBUG_SHAD_TABLE_KEYID:
                    *(MINT32 *) parg2 = ID_SHAD_TABLE;
                    break;
                default:
                    *(MINT32 *) parg2 = ID_ERROR;
                    MY_LOGE("registerID: 0x%x", registerName);
                    ret = MFALSE;
                    break;
            }
        }
            break;
        case CMD_SET_DBG_EXIF:
        {
            MUINT32 u4ID    = (MUINT32) parg1;
            MUINTPTR u4Addr  = (MUINTPTR) parg2;
            MUINT32 u4Size  = (MUINT32) parg3;
            //
            if (mi4DbgModuleType & u4ID)
            {
                MY_LOGW("ID_%s(0x%04x) already exists", mpDebugIdMap->stringFor(u4ID).string(), u4ID);
                ret = MFALSE;
            }
            else
            {
                if (u4ID != ID_ERROR) {
                    //
                    DbgInfo tmpDbgInfo;
                    tmpDbgInfo.u4BufSize    = u4Size;
                    tmpDbgInfo.puDbgBuf     = new MUINT8[ u4Size ];
                    memset(tmpDbgInfo.puDbgBuf, 0, u4Size);
                    memcpy(tmpDbgInfo.puDbgBuf, (void*)u4Addr, u4Size);
                    //
                    mMapModuleID.add(u4ID, mDbgInfo.size());
                    mDbgInfo.push_back(tmpDbgInfo);
                    mi4DbgModuleType |= u4ID;
                    //
                    MY_LOGI("Set: ID_%s(0x%04x), Size(%d), Addr(%p)",
                        mpDebugIdMap->stringFor(u4ID).string(), u4ID, tmpDbgInfo.u4BufSize, (MUINTPTR)&tmpDbgInfo.puDbgBuf);
                }
                else {
                    MY_LOGE("ID_ERROR");
                    ret = MFALSE;
                }
            }
        }
            break;
        default:
            MY_LOGE("unsupport cmd: 0x%x", cmd);
            ret = MFALSE;
            break;
    }
    //
    return ret;

}
