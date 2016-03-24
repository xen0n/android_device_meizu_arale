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


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

// utilities for crop

inline MINT32 div_round(MINT32 const numerator, MINT32 const denominator) {
    return (( numerator < 0 ) ^ (denominator < 0 )) ?
        (( numerator - denominator/2)/denominator) : (( numerator + denominator/2)/denominator);
}

struct vector_f //vector with floating point
{
    MPoint  p;
    MPoint  pf;

                                vector_f(
                                        MPoint const& rP = MPoint(),
                                        MPoint const& rPf = MPoint()
                                        )
                                    : p(rP)
                                    , pf(rPf)
                                {}
};

struct simpleTransform
{
    // just support translation than scale, not a general formulation
    // translation
    MPoint    tarOrigin;
    // scale
    MSize     oldScale;
    MSize     newScale;

                                simpleTransform(
                                        MPoint rOrigin = MPoint(),
                                        MSize  rOldScale = MSize(),
                                        MSize  rNewScale = MSize()
                                        )
                                    : tarOrigin(rOrigin)
                                    , oldScale(rOldScale)
                                    , newScale(rNewScale)
                                {}
};

// transform MPoint
inline MPoint transform(simpleTransform const& trans, MPoint const& p) {
    return MPoint(
            div_round( (p.x - trans.tarOrigin.x) * trans.newScale.w, trans.oldScale.w),
            div_round( (p.y - trans.tarOrigin.y) * trans.newScale.h, trans.oldScale.h)
            );
};

inline MPoint inv_transform(simpleTransform const& trans, MPoint const& p) {
    return MPoint(
            div_round( p.x * trans.oldScale.w, trans.newScale.w) + trans.tarOrigin.x,
            div_round( p.y * trans.oldScale.h, trans.newScale.h) + trans.tarOrigin.y
            );
};

inline int int_floor(float x) {
    int i = (int)x;
    return i - (i > x);
}

// transform vector_f
inline vector_f transform(simpleTransform const& trans, vector_f const& p) {
    MFLOAT const x = (p.p.x + (p.pf.x/(MFLOAT)(1u<<31))) * trans.newScale.w / trans.oldScale.w;
    MFLOAT const y = (p.p.y + (p.pf.y/(MFLOAT)(1u<<31))) * trans.newScale.h / trans.oldScale.h;
    int const x_int = int_floor(x);
    int const y_int = int_floor(y);
    return vector_f(
            MPoint(x_int, y_int),
            MPoint((x - x_int) * (1u<<31), (y - y_int) * (1u<<31))
            );
};

inline vector_f inv_transform(simpleTransform const& trans, vector_f const& p) {
    MFLOAT const x = (p.p.x + (p.pf.x/(MFLOAT)(1u<<31))) * trans.oldScale.w / trans.newScale.w;
    MFLOAT const y = (p.p.y + (p.pf.y/(MFLOAT)(1u<<31))) * trans.oldScale.h / trans.newScale.h;
    int const x_int = int_floor(x);
    int const y_int = int_floor(y);
    return vector_f(
            MPoint(x_int, y_int),
            MPoint((x - x_int) * (1u<<31), (y - y_int) * (1u<<31))
            );
};

// transform MSize
inline MSize transform(simpleTransform const& trans, MSize const& s) {
    return MSize(
            div_round( s.w * trans.newScale.w, trans.oldScale.w),
            div_round( s.h * trans.newScale.h, trans.oldScale.h)
            );
};

inline MSize inv_transform(simpleTransform const& trans, MSize const& s) {
    return MSize(
            div_round( s.w * trans.oldScale.w, trans.newScale.w),
            div_round( s.h * trans.oldScale.h, trans.newScale.h)
            );
};

// transform MRect
inline MRect transform(simpleTransform const& trans, MRect const& r) {
    return MRect(transform(trans, r.p), transform(trans, r.s));
};

inline MRect inv_transform(simpleTransform const& trans, MRect const& r) {
    return MRect(inv_transform(trans, r.p), inv_transform(trans, r.s));
};


};
};

