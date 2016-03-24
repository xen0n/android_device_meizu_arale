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
#ifndef _IBASEEXIF_H_
#define _IBASEEXIF_H_

#include "exif_sdflags.h"
#include "BuiltinTypes.h"

/*******************************************************************************
*
********************************************************************************/
typedef struct exifImageInfo_s {
    MUINTPTR bufAddr;
    unsigned int mainWidth;
    unsigned int mainHeight;
    unsigned int thumbSize;
} exifImageInfo_t;

typedef struct exifAPP1Info_s {
    unsigned int imgIndex;      // mtk definition: the index of continuous shot image.
    unsigned int groupID;       // mtk definition: group ID for continuous shot.
    unsigned int bestFocusH;    // mtk definition: focus value (H) for best shot.
    unsigned int bestFocusL;    // mtk definition: focus value (L) for best shot.
    unsigned int refocusPos;            // mtk definition: for image refocus. JPEG(main sensor) in left or right position.
    unsigned char strJpsFileName[32];   // mtk definition: for image refocus. JPS file name for calculating depth map.
    unsigned int exposureTime[2];
    unsigned int fnumber[2];
    int exposureBiasValue[2];
    unsigned int focalLength[2];
    unsigned short orientation;
    unsigned short exposureProgram;
    unsigned short isoSpeedRatings;
    unsigned short meteringMode;
    unsigned short flash;
    unsigned short whiteBalanceMode;
    unsigned short reserved;
    unsigned char strImageDescription[32];
    unsigned char strMake[32];
    unsigned char strModel[32];
    unsigned char strSoftware[32];
    unsigned char strDateTime[20];
    unsigned char strSubSecTime[4];
    unsigned char gpsLatitudeRef[2];
    unsigned char gpsLongitudeRef[2];
    unsigned char reserved02;
    unsigned int digitalZoomRatio[2];
    unsigned short sceneCaptureType;
    unsigned short lightSource;
    unsigned char strFlashPixVer[8];
    unsigned short exposureMode;
    unsigned short reserved03;
    int gpsIsOn;
    int gpsAltitude[2];
    int gpsLatitude[8];
    int gpsLongitude[8];
    int gpsTimeStamp[8];
    unsigned char gpsDateStamp[12];
    unsigned char gpsProcessingMethod[64];
} exifAPP1Info_t;

typedef struct stereoDescriptor_s{
    unsigned char type[1];
    unsigned char layout[1];
    unsigned char flags[1];
    unsigned char separation[1];

}stereoDescriptor_t;

typedef struct exifAPP3Info_s {
    unsigned char identifier[8];
    unsigned char length[2];
    stereoDescriptor_t  stereoDesc[1];
    struct Comments
    {
        unsigned char size[2];
        unsigned char comment[16];
    } cmt;

} exifAPP3Info_t;

/******************************************************************************
 *
 ******************************************************************************/
//namespace NSCam {


/******************************************************************************
 *  BaseExif Interface
 ******************************************************************************/
class IBaseExif
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Instantiation.
    virtual                             ~IBaseExif(){};
                                        IBaseExif(){};

public:     ////                        Operations.
    virtual unsigned int        exifApp1Make(
                                    exifImageInfo_t *pexifImgInfo,
                                    exifAPP1Info_t *pexifAPP1Info,
                                    unsigned int *pretSize
                                ) = 0;

    virtual unsigned int        exifAppnMake(
                                    unsigned int appn,
                                    unsigned char *paddr,
                                    unsigned char *pdata,
                                    unsigned int dataSize,
                                    unsigned int *pretSize,
                                    unsigned int defaultSize = 0
                                ) = 0;

    virtual bool                init(unsigned int const gpsEnFlag) = 0;

    virtual bool                uninit() = 0;

    virtual size_t              exifApp1SizeGet() const = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
//};  //namespace NSCam

#endif // _IBASEEXIF_H_

