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

#define LOG_TAG "MtkCam/camtest"
//
//
#include <stdio.h>
#include <stdlib.h>
//
#include "CamLog.h"
//
//
/******************************************************************************
 *
 ******************************************************************************/
void    Test_ImageBufferAllocator();
void    Test_GrallocImageBufferHeap();
void    Test_IonImageBufferHeap();
void    Test_ImageBuffer();
void    Test_JpegImageBufferFromBlobHeap();

enum eCaseID {
    //
    ID_Allocator    = 0x0000,
    ID_ImageBuffer  = 0x0001,
    ID_JpegBuf      = 0x0002,
    ID_IonHeap      = 0x0004,
    ID_GrallocHeap  = 0x0008,
    //
};


static void usage()
{
    CAM_LOGD("Please enter UT case:");
    CAM_LOGD("   0x%x - Test_ImageBufferAllocator ",          ID_Allocator);
    CAM_LOGD("   0x%x - Test_ImageBuffer ",                   ID_ImageBuffer);
    CAM_LOGD("   0x%x - Test_JpegImageBufferFromBlobHeap ",   ID_JpegBuf);
    CAM_LOGD("   0x%x - Test_IonImageBufferHeap ",            ID_IonHeap);
    CAM_LOGD("   0x%x - Test_GrallocImageBufferHeap ",        ID_GrallocHeap);
}

/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char** argv)
{
    CAM_LOGD("+++++++++++START+++++++++++");
    //
    usage();
    //
    if( argc < 2 )
    {
        return -1;
    }
    //
    int testcase = atoi(argv[1]);
    //
    if ( testcase & ID_Allocator ) {
        Test_ImageBufferAllocator();
    }
    if ( testcase & ID_ImageBuffer ) {
        Test_ImageBuffer();
    }
    if ( testcase & ID_JpegBuf ) {
        Test_JpegImageBufferFromBlobHeap();
    }
    if ( testcase & ID_IonHeap ) {
        Test_IonImageBufferHeap();
    }
    if ( testcase & ID_GrallocHeap ) {
        Test_GrallocImageBufferHeap();
    }
    //
    //
    CAM_LOGD("-----------E N D-----------");
    return 0;
}

