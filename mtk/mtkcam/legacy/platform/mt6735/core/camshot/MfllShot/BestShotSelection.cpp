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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
//
#define LOG_TAG "MtkCam/BSS"
#ifndef WIN32
#include <mtkcam/Log.h>
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __func__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
#endif

#include "../inc/BestShotSelection.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define BSS_ALG best_shot_stats

using namespace android;
namespace NSCamShot {



//-----------------------------------------------------------------------------
//    copy from simMfll.cpp
//-----------------------------------------------------------------------------
struct data{
    long long val;
    MUINT8 index;
    MINT32 gmvx, gmvy;
    MUINT8* pimg;
};

static
int compare_data(const void* a, const void* b)
{
    data *s1, *s2;

    s1 = (data *)a;
    s2 = (data *)b;

    return s2->val - s1->val;
}

static
MUINT8 best_shot_selection(long long *score,
                         MUINT8** img0, MUINT8** img1, MUINT8** img2, MUINT8** img3,
                         /*MUINT8** base_img, MUINT32 img_buffer_size,*/
                         Gmv *pgmv,
                         MUINT32 *sortedIndex)
{
    FUNC_START;

    int i;

    data items[4];

    for(i = 0; i < 4; i++)
    {
        items[i].index = i;
        items[i].val   = score[i];
        items[i].gmvx  = pgmv[i].x;
        items[i].gmvy  = pgmv[i].y;
    }

    items[0].pimg  = *img0;
    items[1].pimg  = *img1;
    items[2].pimg  = *img2;
    items[3].pimg  = *img3;

    qsort(items, 4, sizeof(data), compare_data);

//    *base_img = *base_img + items[0].index * img_buffer_size;

    *img0 = items[0].pimg;
    *img1 = items[1].pimg;
    *img2 = items[2].pimg;
    *img3 = items[3].pimg;

    for(i = 0; i < 4; i++)
    {
        //gmv
        pgmv[i].x = items[i].gmvx - items[0].gmvx;
        pgmv[i].y = items[i].gmvy - items[0].gmvy;
        //order
        sortedIndex[i] = items[i].index;
    }

    FUNC_END;
    return items[0].index;
}

static
long long best_shot_stats(MUINT8* img, MUINT32 inWidth, MUINT32 inHeight,
                          MUINT32 iBSS_ROI_width, MUINT32 iBSS_ROI_height,
                          MUINT32 iBSS_x0, MUINT32 iBSS_y0,
                          MUINT8 iScale_Factor, MUINT8 iClip_TH0, MUINT8 iClip_TH1, MUINT32 THS)
{
    FUNC_START;

    long long score = 0;

    MUINT8 *ptROW0, *ptROW1;
    MUINT8 *p0, *p1;
    MINT32 y, x;
    MUINT8 gradx, grady;
    MUINT32 iSkip_Cnt;
//    MUINT32 grad;
    MUINT32 grad_hist[256];
    MUINT32 *pCnt;
    MUINT32 high_cnt;

    // prevent BSS under access unmapped address.
    if(iBSS_x0 < 0){
        iBSS_x0 = 0;
    }
    if(iBSS_y0 < 0){
        iBSS_y0 = 0;
    }
    // prevent BSS over access unmapped address.
    if(iBSS_x0 + iBSS_ROI_width > inWidth){
        iBSS_x0 = inWidth - iBSS_ROI_width;
    }
    if(iBSS_y0 + iBSS_ROI_height > inHeight){
        iBSS_y0 = inHeight - iBSS_ROI_height;
    }

    MY_LOGD("[best_shot_stats] w/h(%d,%d), roi w/h/x/y(%d,%d,%d,%d) scale/th0/th1/ths(%d,%d,%d)"
            , inWidth, inHeight
            , iBSS_ROI_width, iBSS_ROI_height
            , iBSS_x0, iBSS_y0
            , iScale_Factor
            , iClip_TH0, iClip_TH1, THS
            );

    memset(grad_hist, 0, 256 * sizeof(MUINT32));

    iSkip_Cnt = iScale_Factor * inWidth;

    ptROW0 = img + iBSS_y0 * inWidth + iBSS_x0;
    ptROW1 = ptROW0 + inWidth;
    //ptROW1 = ptROW0 + 2*inWidth;

    for(y = iBSS_ROI_height; y > 0; y-=iScale_Factor)
    {
        p0 = ptROW0;
        p1 = ptROW1;

        for(x = iBSS_ROI_width; x > 0; x--)
        {
            //gradx = abs(*(p0 + 2) - *p0);
            gradx = abs(*(p0 + 1) - *p0);
            grady = abs(*p1 - *p0);

            //score += (gradx*gradx + grady*grady);
            grad_hist[gradx]++;

            grad_hist[grady]++;

            p0++;
            p1++;
        }

        ptROW0 += iSkip_Cnt;

        ptROW1 += iSkip_Cnt;
    }

    pCnt = grad_hist + iClip_TH0;

    for(y = iClip_TH0; y < iClip_TH1; y++)
    {
        score += (*pCnt * y * y);

        pCnt++;
    }

    high_cnt = 0;

    for(y = iClip_TH1; y < 256; y++)
    {
        high_cnt += *pCnt;

        pCnt++;
    }

    score += (high_cnt * THS);
    MY_LOGD("[best_shot_stats] score(%lld)", score);

    FUNC_END;
    return score;
}


static
int bss(MF_BSS_PARAM_STRUCT &MF_BSS_PARAM, BSS_INPUT_DATA *in, BSS_OUTPUT_DATA *out)
{
    FUNC_START;

    MY_LOGD("MF_BSS_ON = %d\n", MF_BSS_PARAM.MF_BSS_ON);
    MY_LOGD("MF_BSS_ROI_WIDTH = %d\n", MF_BSS_PARAM.MF_BSS_ROI_WIDTH);
    MY_LOGD("MF_BSS_ROI_HEIGHT = %d\n", MF_BSS_PARAM.MF_BSS_ROI_HEIGHT);
    MY_LOGD("MF_BSS_ROI_X0 = %d\n", MF_BSS_PARAM.MF_BSS_ROI_X0);
    MY_LOGD("MF_BSS_ROI_Y0 = %d\n", MF_BSS_PARAM.MF_BSS_ROI_Y0);
    MY_LOGD("MF_BSS_SCALE_FACTOR = %d\n", MF_BSS_PARAM.MF_BSS_SCALE_FACTOR);
    MY_LOGD("MF_BSS_CLIP_TH0 = %d\n", MF_BSS_PARAM.MF_BSS_CLIP_TH0);
    MY_LOGD("MF_BSS_CLIP_TH1 = %d\n", MF_BSS_PARAM.MF_BSS_CLIP_TH1);
    MY_LOGD("MF_BSS_ZERO = %d\n", MF_BSS_PARAM.MF_BSS_ZERO);

    MUINT8 *Proc1QBImg       = in->Proc1QBImg;
    MUINT8 *Proc1QR1Img       = in->Proc1QR1Img;
    MUINT8 *Proc1QR2Img       = in->Proc1QR2Img;
    MUINT8 *Proc1QR3Img       = in->Proc1QR3Img;

    MUINT32 inMEWidth        = in->inMEWidth;
    MUINT32 inMEHeight        = in->inMEHeight;

    Gmv    *gmv = in->gmv;
    MUINT32 iClip_TH1_Squared;
    long long sharp_score[4];
    MUINT8  base_idx = 0;

    iClip_TH1_Squared = MF_BSS_PARAM.MF_BSS_CLIP_TH1 * MF_BSS_PARAM.MF_BSS_CLIP_TH1;

    if( MF_BSS_PARAM.MF_BSS_ON )
    {
        sharp_score[0] = BSS_ALG(Proc1QBImg, inMEWidth, inMEHeight,
                             MF_BSS_PARAM.MF_BSS_ROI_WIDTH, MF_BSS_PARAM.MF_BSS_ROI_HEIGHT,
                             MF_BSS_PARAM.MF_BSS_ROI_X0, MF_BSS_PARAM.MF_BSS_ROI_Y0,
                             MF_BSS_PARAM.MF_BSS_SCALE_FACTOR, MF_BSS_PARAM.MF_BSS_CLIP_TH0,
                             MF_BSS_PARAM.MF_BSS_CLIP_TH1, iClip_TH1_Squared);

        if( (MUINT32)abs(gmv[1].x) <= MF_BSS_PARAM.MF_BSS_ZERO &&
            (MUINT32)abs(gmv[1].y) <= MF_BSS_PARAM.MF_BSS_ZERO &&
            (MUINT32)abs(gmv[2].x) <= MF_BSS_PARAM.MF_BSS_ZERO &&
            (MUINT32)abs(gmv[2].y) <= MF_BSS_PARAM.MF_BSS_ZERO &&
            (MUINT32)abs(gmv[3].x) <= MF_BSS_PARAM.MF_BSS_ZERO &&
            (MUINT32)abs(gmv[3].y) <= MF_BSS_PARAM.MF_BSS_ZERO )
        {
            sharp_score[1] = BSS_ALG(Proc1QR1Img, inMEWidth, inMEHeight,
                                 MF_BSS_PARAM.MF_BSS_ROI_WIDTH, MF_BSS_PARAM.MF_BSS_ROI_HEIGHT,
                                 MF_BSS_PARAM.MF_BSS_ROI_X0, MF_BSS_PARAM.MF_BSS_ROI_Y0,
                                 MF_BSS_PARAM.MF_BSS_SCALE_FACTOR, MF_BSS_PARAM.MF_BSS_CLIP_TH0, MF_BSS_PARAM.MF_BSS_CLIP_TH1, iClip_TH1_Squared);

            sharp_score[2] = BSS_ALG(Proc1QR2Img, inMEWidth, inMEHeight,
                                 MF_BSS_PARAM.MF_BSS_ROI_WIDTH, MF_BSS_PARAM.MF_BSS_ROI_HEIGHT,
                                 MF_BSS_PARAM.MF_BSS_ROI_X0, MF_BSS_PARAM.MF_BSS_ROI_Y0,
                                 MF_BSS_PARAM.MF_BSS_SCALE_FACTOR, MF_BSS_PARAM.MF_BSS_CLIP_TH0, MF_BSS_PARAM.MF_BSS_CLIP_TH1, iClip_TH1_Squared);

            sharp_score[3] = BSS_ALG(Proc1QR3Img, inMEWidth, inMEHeight,
                                 MF_BSS_PARAM.MF_BSS_ROI_WIDTH, MF_BSS_PARAM.MF_BSS_ROI_HEIGHT,
                                 MF_BSS_PARAM.MF_BSS_ROI_X0, MF_BSS_PARAM.MF_BSS_ROI_Y0,
                                 MF_BSS_PARAM.MF_BSS_SCALE_FACTOR, MF_BSS_PARAM.MF_BSS_CLIP_TH0, MF_BSS_PARAM.MF_BSS_CLIP_TH1, iClip_TH1_Squared);
        }
        else
        {
            sharp_score[1] = BSS_ALG(Proc1QR1Img, inMEWidth, inMEHeight,
                                 MF_BSS_PARAM.MF_BSS_ROI_WIDTH, MF_BSS_PARAM.MF_BSS_ROI_HEIGHT,
                                 MF_BSS_PARAM.MF_BSS_ROI_X0 + gmv[1].x, MF_BSS_PARAM.MF_BSS_ROI_Y0 + gmv[1].y,
                                 MF_BSS_PARAM.MF_BSS_SCALE_FACTOR, MF_BSS_PARAM.MF_BSS_CLIP_TH0, MF_BSS_PARAM.MF_BSS_CLIP_TH1, iClip_TH1_Squared);

            sharp_score[2] = BSS_ALG(Proc1QR2Img, inMEWidth, inMEHeight,
                                 MF_BSS_PARAM.MF_BSS_ROI_WIDTH, MF_BSS_PARAM.MF_BSS_ROI_HEIGHT,
                                 MF_BSS_PARAM.MF_BSS_ROI_X0 + gmv[2].x, MF_BSS_PARAM.MF_BSS_ROI_Y0 + gmv[2].y,
                                 MF_BSS_PARAM.MF_BSS_SCALE_FACTOR, MF_BSS_PARAM.MF_BSS_CLIP_TH0, MF_BSS_PARAM.MF_BSS_CLIP_TH1, iClip_TH1_Squared);

            sharp_score[3] = BSS_ALG(Proc1QR3Img, inMEWidth, inMEHeight,
                                 MF_BSS_PARAM.MF_BSS_ROI_WIDTH, MF_BSS_PARAM.MF_BSS_ROI_HEIGHT,
                                 MF_BSS_PARAM.MF_BSS_ROI_X0 + gmv[3].x, MF_BSS_PARAM.MF_BSS_ROI_Y0 + gmv[3].y,
                                 MF_BSS_PARAM.MF_BSS_SCALE_FACTOR, MF_BSS_PARAM.MF_BSS_CLIP_TH0, MF_BSS_PARAM.MF_BSS_CLIP_TH1, iClip_TH1_Squared);
        }

        //Proc2BImg = Proc2BImg0;

        MUINT32 sortedIndex[4] = {0};
        Gmv pgmv[4];
        for(int i=0; i<4; i++) {
            pgmv[i].x = in->gmv[i].x;
            pgmv[i].y = in->gmv[i].y;
        }

        base_idx = best_shot_selection(sharp_score,
                                       &Proc1QBImg, &Proc1QR1Img, &Proc1QR2Img, &Proc1QR3Img,
                                       /*&Proc2BImg, Proc2ImgBufsize<<1,*/
                                       pgmv,
                                       sortedIndex
                                       );

        MY_LOGD("sharp_score[0] = %lld\n", sharp_score[0]);
        MY_LOGD("sharp_score[1] = %lld\n", sharp_score[1]);
        MY_LOGD("sharp_score[2] = %lld\n", sharp_score[2]);
        MY_LOGD("sharp_score[3] = %lld\n", sharp_score[3]);
        MY_LOGD("base_idx = %d\n", base_idx);

        for(int i=0; i<4; i++) {
            out->originalOrder[i] = sortedIndex[i];
            out->gmv[i].x = pgmv[i].x;
            out->gmv[i].y = pgmv[i].y;
            MY_LOGD("bss output[%d] = index(%d), gmv(%d,%d)\n"
                    , i
                    , out->originalOrder[i]
                    , out->gmv[i].x
                    , out->gmv[i].y
                    );
        }

    }

    FUNC_END;
    return base_idx;
}


//-----------------------------------------------------------------------------
// BestShotSelection
//-----------------------------------------------------------------------------
class BestShotSelectionImp : public BestShotSelection {
    public:
        //
        //virtual BestShotSelection*  createInstance();
        //virtual MVOID               destroyInstance();
        virtual MBOOL               init(const MF_BSS_PARAM_STRUCT& param);
        virtual MBOOL               sort(BSS_INPUT_DATA *in, BSS_OUTPUT_DATA *output);
    private:
        MF_BSS_PARAM_STRUCT         mParam;
};

BestShotSelection*
BestShotSelection::
createInstance()
{
    return new BestShotSelectionImp;
}


MVOID
BestShotSelection::
destroyInstance()
{
    delete this;
}


BestShotSelection::
BestShotSelection()
{
}


BestShotSelection::
~BestShotSelection()
{
}


//-----------------------------------------------------------------------------
// BestShotSelectionImp
//-----------------------------------------------------------------------------
MBOOL
BestShotSelectionImp::
init(const MF_BSS_PARAM_STRUCT &param)
{
    FUNC_START;
    mParam = param;
    FUNC_END;
    return MTRUE;
}


MBOOL
BestShotSelectionImp::
sort(BSS_INPUT_DATA *in, BSS_OUTPUT_DATA *out)
{
    FUNC_START;
    bss(mParam, in, out);
    FUNC_END;
    return MTRUE;
}


};  //namespace NSCamShot


