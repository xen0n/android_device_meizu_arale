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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V1_CUSTOM_INTERNAL_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V1_CUSTOM_INTERNAL_H_
/******************************************************************************
 *
 ******************************************************************************/
namespace android {
    namespace camera {
        namespace v1 {
            typedef enum
            {
                SIZE_RATIO_START,
                SIZE_RATIO_4_3,
                SIZE_RATIO_3_2,
                SIZE_RATIO_5_3,
                SIZE_RATIO_16_9,

                SIZE_RATIO_UNKNOWN,
                SIZE_RATIO_MAX,
            }SIZE_RATIO;

            //
            int isAligned(int w, int h, int aligned);
            int isAligned_16(int w, int h);
            int isSize4_3(int width, int height);
            int isSize3_2(int width, int height);

            int isSize5_3(int width, int height);
            int isSize16_9(int width, int height);

            int getNearestRatio(int w, int h);
            int getSizeRatio(int w, int h);
            int add4_3_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h);
            int add3_2_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h);
            int add5_3_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h);
            int add16_9_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h);
            int generateSizeValues(int max_w, int max_h, char ***size_arr, int force_ratio);

            char *getDefaultPicSize();
            int parseStrToSize(const char *s, int *w, int *h);

            const char **getPicSizes(char const *maxPicSize, int * pCnt, int facing);
            void addExtraPictureSizes( char const ** defaultSizes, int defaultSize, char const ** extraSizes, int extraSize);

            const char **getPrvSizes(char const *maxPrvSize, int * pCnt, int facing);
            void addExtraPreviewSizes( char const ** defaultSizes, int defaultSize, char const ** extraSizes, int extraSize);
        }
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V1_COMMON_H_

