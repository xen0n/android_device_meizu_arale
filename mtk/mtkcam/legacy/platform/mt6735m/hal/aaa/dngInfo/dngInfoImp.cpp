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
#define LOG_TAG "dngInfo"

#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/v3/hal/dngInfo.h>
#include <mtkcam/v3/hal/aaa_utils.h>
#include <mtkcam/hal/IHalSensor.h>
#include <camera_custom_nvram.h>
#include <nvbuf_util.h>
#include <isp_tuning_cam_info.h>

namespace NS3Av3
{
using namespace NSCam;

static MBOOL gen_CM_FM(double in_CCM[3][3], double out_CM[3][3], double out_FM[3][3]);
inline static UINT8 _cvtRefIllum(UINT8 eIdx_CCM);

class DngInfoImp : public DngInfo
{
public:
    static DngInfoImp* getInstance(MUINT32 u4SensorDev);

    virtual IMetadata getStaticMetadata() const;
    virtual IMetadata getDynamicNoiseProfile(MUINT32 iso) const;

protected:
    DngInfoImp(MUINT32 u4SensorDev);
    ~DngInfoImp();

private:
    MUINT8 convertColorTransform(const NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspParam);

    // DNG related
    double fColorMatrix1[3][3];
    double fForwardMatrix1[3][3];
    double fColorMatrix2[3][3];
    double fForwardMatrix2[3][3];
    MUINT8 u1RefIlluminant1;
    MUINT8 u1RefIlluminant2;
    MBOOL fgSupport2ColorMatrices;
    MBOOL fgOK;

    DngNoiseProile_T rNoiseProfile[4];
};

DngInfo*
DngInfo::
getInstance(MUINT32 u4SensorDev)
{
    return DngInfoImp::getInstance(u4SensorDev);
}

DngInfoImp*
DngInfoImp::
getInstance(MUINT32 u4SensorDev)
{
    DngInfoImp* pDngInfo;
    switch (u4SensorDev)
    {
    case SENSOR_DEV_MAIN:
        {
            static DngInfoImp _rInfo(SENSOR_DEV_MAIN);
            pDngInfo = &_rInfo;
        }
        break;
    case SENSOR_DEV_SUB:
        {
            static DngInfoImp _rInfo(SENSOR_DEV_SUB);
            pDngInfo = &_rInfo;
        }
        break;
    case SENSOR_DEV_MAIN_2:
        {
            static DngInfoImp _rInfo(SENSOR_DEV_MAIN_2);
            pDngInfo = &_rInfo;
        }
        break;
    default:
        pDngInfo = NULL;
        break;
    }

    return pDngInfo;
}

DngInfoImp::
DngInfoImp(MUINT32 u4SensorDev)
    : DngInfo(u4SensorDev)
    , fgOK(MFALSE)
{
    NVRAM_CAMERA_ISP_PARAM_STRUCT* pNvram_Isp = MNULL;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, u4SensorDev, (void*&)pNvram_Isp);
    if (err!=0)
    {
        MY_ERR("NvBufUtil getBufAndRead fail");
    }
    else
    {
        ::memcpy(rNoiseProfile, pNvram_Isp->DngMetadata.rNoiseProfile, sizeof(DngNoiseProile_T)*4);
        MY_LOG("[%s] S0(%f,%f) O0(%f,%f) S1(%f,%f) O1(%f,%f) S2(%f,%f) O2(%f,%f) S3(%f,%f) O3(%f,%f)", __FUNCTION__,
            rNoiseProfile[0].S.a, rNoiseProfile[0].S.b, rNoiseProfile[0].O.a, rNoiseProfile[0].O.b,
            rNoiseProfile[1].S.a, rNoiseProfile[1].S.b, rNoiseProfile[1].O.a, rNoiseProfile[1].O.b,
            rNoiseProfile[2].S.a, rNoiseProfile[2].S.b, rNoiseProfile[2].O.a, rNoiseProfile[2].O.b,
            rNoiseProfile[3].S.a, rNoiseProfile[3].S.b, rNoiseProfile[3].O.a, rNoiseProfile[3].O.b);
        if (0 == convertColorTransform(*pNvram_Isp))
        {
            fgOK = MTRUE;
            MY_LOG("[%s] Correctly converted color transform", __FUNCTION__);
        }
    }
}

DngInfoImp::
~DngInfoImp()
{}

template<typename T>
static void _M2R(const T* ptrM, MRational* ptrR, int den)
{
    int i;
    for (i = 0; i < 9; i++)
        *ptrR++ = MRational((*ptrM++)*den, den);
}

IMetadata
DngInfoImp::
getStaticMetadata() const
{
    IMetadata rMeta;
    MRational rMat[9];

    const float I[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    if (fgOK)
    {
        UPDATE_ENTRY_SINGLE(rMeta, MTK_SENSOR_REFERENCE_ILLUMINANT1, u1RefIlluminant1);
        _M2R(&fColorMatrix1[0][0], rMat, 65536);
        UPDATE_ENTRY_ARRAY(rMeta, MTK_SENSOR_COLOR_TRANSFORM1, rMat, 9);
        _M2R(&fForwardMatrix1[0][0], rMat, 65536);
        UPDATE_ENTRY_ARRAY(rMeta, MTK_SENSOR_FORWARD_MATRIX1, rMat, 9);
        _M2R(I, rMat, 65536);
        UPDATE_ENTRY_ARRAY(rMeta, MTK_SENSOR_CALIBRATION_TRANSFORM1, rMat, 9);
        if (fgSupport2ColorMatrices)
        {
            UPDATE_ENTRY_SINGLE(rMeta, MTK_SENSOR_REFERENCE_ILLUMINANT2, u1RefIlluminant2);
            _M2R(&fColorMatrix2[0][0], rMat, 65536);
            UPDATE_ENTRY_ARRAY(rMeta, MTK_SENSOR_COLOR_TRANSFORM2, rMat, 9);
            _M2R(&fForwardMatrix2[0][0], rMat, 65536);
            UPDATE_ENTRY_ARRAY(rMeta, MTK_SENSOR_FORWARD_MATRIX2, rMat, 9);
            _M2R(I, rMat, 65536);
            UPDATE_ENTRY_ARRAY(rMeta, MTK_SENSOR_CALIBRATION_TRANSFORM2, rMat, 9);
        }
    }
    return rMeta;
}

IMetadata
DngInfoImp::
getDynamicNoiseProfile(MUINT32 iso) const
{
    IMetadata rMeta;
    double noise_profile[8] = {0.0};
    int i;
    for (i = 0; i < 4; i++)
    {
        double s, o;
        s = rNoiseProfile[i].S.a * iso + rNoiseProfile[i].S.b;
        o = rNoiseProfile[i].O.a * iso + rNoiseProfile[i].O.b;
        noise_profile[i*2+0] = s < 0.0 ? 0.0 : s;
        noise_profile[i*2+1] = o < 0.0 ? 0.0 : o;
    }
    UPDATE_ENTRY_ARRAY(rMeta, MTK_SENSOR_NOISE_PROFILE, noise_profile, 8);
    return rMeta;
}

MUINT8
DngInfoImp::
convertColorTransform(const NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspParam)
{
// 2'comp, using ASL and ASR to do sign extension
#define TO_INT(a, bit) ((MINT32)((MINT32) (a)) << (32-(bit)) >> (32-(bit)))
#undef MIN
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#undef MAX
#define MAX(a, b) ((a) >= (b)) ? (a) : (b)

    MUINT8 u1Ret = 0;
    double in_CCM[3][3];

    MINT32 i4RefIllum1 = rIspParam.DngMetadata.i4RefereceIlluminant1;
    MINT32 i4RefIllum2 = rIspParam.DngMetadata.i4RefereceIlluminant2;

    i4RefIllum1 = MIN(MAX(i4RefIllum1, 0), NVRAM_CCM_TBL_NUM-1);
    i4RefIllum2 = MIN(MAX(i4RefIllum2, 0), NVRAM_CCM_TBL_NUM-1);

    if (i4RefIllum1 == i4RefIllum2)
    {
        fgSupport2ColorMatrices = MFALSE;
        MY_LOG("[%s] Support Only one color trasform for DNG, NVRAM CCM index(%d)", __FUNCTION__, i4RefIllum1);
    }
    else
    {
        fgSupport2ColorMatrices = MTRUE;
        MY_LOG("[%s] Support two color trasform for DNG, NVRAM CCM idx1(%d), idx2(%d)", __FUNCTION__, i4RefIllum1, i4RefIllum2);
    }

    // convert to android metadata reference illuminant
    
    u1RefIlluminant1 = _cvtRefIllum(i4RefIllum1);
    u1RefIlluminant2 = _cvtRefIllum(i4RefIllum2);

    // convert to android metadata color matrix and forward matrix
    const ISP_NVRAM_CCM_T& ccm1 = rIspParam.ISPRegs.CCM[i4RefIllum1];    // format 4.9, -7.99~8.0
    in_CCM[0][0] = (double) TO_INT(ccm1.conv0a.bits.G2G_CNV_00, 11) / 128;
    in_CCM[0][1] = (double) TO_INT(ccm1.conv0a.bits.G2G_CNV_01, 11) / 128;
    in_CCM[0][2] = (double) TO_INT(ccm1.conv0b.bits.G2G_CNV_02, 11) / 128;
    in_CCM[1][0] = (double) TO_INT(ccm1.conv1a.bits.G2G_CNV_10, 11) / 128;
    in_CCM[1][1] = (double) TO_INT(ccm1.conv1a.bits.G2G_CNV_11, 11) / 128;
    in_CCM[1][2] = (double) TO_INT(ccm1.conv1b.bits.G2G_CNV_12, 11) / 128;
    in_CCM[2][0] = (double) TO_INT(ccm1.conv2a.bits.G2G_CNV_20, 11) / 128;
    in_CCM[2][1] = (double) TO_INT(ccm1.conv2a.bits.G2G_CNV_21, 11) / 128;
    in_CCM[2][2] = (double) TO_INT(ccm1.conv2b.bits.G2G_CNV_22, 11) / 128;
    
    if (gen_CM_FM(in_CCM, fColorMatrix1, fForwardMatrix1))
    {
        MY_LOG("CCM1: [%f, %f, %f][%f, %f, %f][%f, %f, %f]",
            in_CCM[0][0], in_CCM[0][1], in_CCM[0][2],
            in_CCM[1][0], in_CCM[1][1], in_CCM[1][2],
            in_CCM[2][0], in_CCM[2][1], in_CCM[2][2]);
        MY_LOG("CM1: [%f, %f, %f][%f, %f, %f][%f, %f, %f]",
            fColorMatrix1[0][0], fColorMatrix1[0][1], fColorMatrix1[0][2],
            fColorMatrix1[1][0], fColorMatrix1[1][1], fColorMatrix1[1][2],
            fColorMatrix1[2][0], fColorMatrix1[2][1], fColorMatrix1[2][2]);
        MY_LOG("FM1: [%f, %f, %f][%f, %f, %f][%f, %f, %f]",
            fForwardMatrix1[0][0], fForwardMatrix1[0][1], fForwardMatrix1[0][2],
            fForwardMatrix1[1][0], fForwardMatrix1[1][1], fForwardMatrix1[1][2],
            fForwardMatrix1[2][0], fForwardMatrix1[2][1], fForwardMatrix1[2][2]);
    }
    else
    {
        u1Ret = 1;
        fColorMatrix1[0][0] = 1.0f; fColorMatrix1[0][1] = 0.0f; fColorMatrix1[0][2] = 0.0f;
        fColorMatrix1[1][0] = 0.0f; fColorMatrix1[1][1] = 1.0f; fColorMatrix1[1][2] = 0.0f;
        fColorMatrix1[2][0] = 0.0f; fColorMatrix1[2][1] = 0.0f; fColorMatrix1[2][2] = 1.0f;
        ::memcpy(&fForwardMatrix1[0][0], &fColorMatrix1[0][0], sizeof(double)*9);
        MY_ERR("Fail to generate CM1 & FM1 due to invertible in_CCM1!, use identity matrix.");
    }

    if (fgSupport2ColorMatrices)
    {
        const ISP_NVRAM_CCM_T& ccm2 = rIspParam.ISPRegs.CCM[i4RefIllum2];    // format 4.9, -7.99~8.0
        in_CCM[0][0] = (double) TO_INT(ccm2.conv0a.bits.G2G_CNV_00, 11) / 128;
        in_CCM[0][1] = (double) TO_INT(ccm2.conv0a.bits.G2G_CNV_01, 11) / 128;
        in_CCM[0][2] = (double) TO_INT(ccm2.conv0b.bits.G2G_CNV_02, 11) / 128;
        in_CCM[1][0] = (double) TO_INT(ccm2.conv1a.bits.G2G_CNV_10, 11) / 128;
        in_CCM[1][1] = (double) TO_INT(ccm2.conv1a.bits.G2G_CNV_11, 11) / 128;
        in_CCM[1][2] = (double) TO_INT(ccm2.conv1b.bits.G2G_CNV_12, 11) / 128;
        in_CCM[2][0] = (double) TO_INT(ccm2.conv2a.bits.G2G_CNV_20, 11) / 128;
        in_CCM[2][1] = (double) TO_INT(ccm2.conv2a.bits.G2G_CNV_21, 11) / 128;
        in_CCM[2][2] = (double) TO_INT(ccm2.conv2b.bits.G2G_CNV_22, 11) / 128;
        
        if (gen_CM_FM(in_CCM, fColorMatrix2, fForwardMatrix2))
        {
            MY_LOG("CCM2: [%f, %f, %f][%f, %f, %f][%f, %f, %f]",
                in_CCM[0][0], in_CCM[0][1], in_CCM[0][2],
                in_CCM[1][0], in_CCM[1][1], in_CCM[1][2],
                in_CCM[2][0], in_CCM[2][1], in_CCM[2][2]);
            MY_LOG("CM2: [%f, %f, %f][%f, %f, %f][%f, %f, %f]",
                fColorMatrix2[0][0], fColorMatrix2[0][1], fColorMatrix2[0][2],
                fColorMatrix2[1][0], fColorMatrix2[1][1], fColorMatrix2[1][2],
                fColorMatrix2[2][0], fColorMatrix2[2][1], fColorMatrix2[2][2]);
            MY_LOG("FM2: [%f, %f, %f][%f, %f, %f][%f, %f, %f]",
                fForwardMatrix2[0][0], fForwardMatrix2[0][1], fForwardMatrix2[0][2],
                fForwardMatrix2[1][0], fForwardMatrix2[1][1], fForwardMatrix2[1][2],
                fForwardMatrix2[2][0], fForwardMatrix2[2][1], fForwardMatrix2[2][2]);
        }
        else
        {
            u1Ret |= 2;
            fColorMatrix2[0][0] = 1.0f; fColorMatrix2[0][1] = 0.0f; fColorMatrix2[0][2] = 0.0f;
            fColorMatrix2[1][0] = 0.0f; fColorMatrix2[1][1] = 1.0f; fColorMatrix2[1][2] = 0.0f;
            fColorMatrix2[2][0] = 0.0f; fColorMatrix2[2][1] = 0.0f; fColorMatrix2[2][2] = 1.0f;
            ::memcpy(&fForwardMatrix2[0][0], &fColorMatrix2[0][0], sizeof(double)*9);
            MY_ERR("Fail to generate CM2 & FM2 due to invertible in_CCM2!, use identity matrix.");
        }
    }

    return u1Ret;
}

inline static UINT8 _cvtRefIllum(UINT8 eIdx_CCM)
{
    UINT8 u1RefIllum = MTK_SENSOR_REFERENCE_ILLUMINANT1_D65;
    switch (eIdx_CCM)
    {
    case NSIspTuning::eIDX_CCM_D65:
        u1RefIllum = MTK_SENSOR_REFERENCE_ILLUMINANT1_D65;
        break;
    case NSIspTuning::eIDX_CCM_TL84:
        u1RefIllum = MTK_SENSOR_REFERENCE_ILLUMINANT1_COOL_WHITE_FLUORESCENT;
        break;
    case NSIspTuning::eIDX_CCM_CWF:
        u1RefIllum = MTK_SENSOR_REFERENCE_ILLUMINANT1_COOL_WHITE_FLUORESCENT;
        break;
    case NSIspTuning::eIDX_CCM_A:
        u1RefIllum = MTK_SENSOR_REFERENCE_ILLUMINANT1_STANDARD_A;
        break;
    }
    return u1RefIllum;
}

static
MBOOL
gen_CM_FM(double in_CCM[3][3], double out_CM[3][3], double out_FM[3][3])
{
    int i,j;
    double CA[3][3] =     {{1.0478112,  0.0228866, -0.0501270},{ 0.0295424, 0.9904844, -0.0170491},{-0.0092345,  0.0150436, 0.7521316}};
    double inv_CA[3][3] = {{0.9555766, -0.0230393,  0.0631636},{-0.0282895, 1.0099416,  0.0210077},{ 0.0122982, -0.0204830, 1.3299098}};
    double M[3][3] =      {{3.2404542, -1.5371385, -0.4985314},{-0.9692660, 1.8760108,  0.0415560},{ 0.0556434, -0.2040259, 1.0572252}};
    double inv_M[3][3] =  {{0.4124564,  0.3575761,  0.1804375},{ 0.2126729, 0.7151522,  0.0721750},{ 0.0193339,  0.1191920, 0.9503041}};

    double det = in_CCM[0][0] * (in_CCM[1][1] * in_CCM[2][2] - in_CCM[2][1] * in_CCM[1][2]) -
                 in_CCM[0][1] * (in_CCM[1][0] * in_CCM[2][2] - in_CCM[1][2] * in_CCM[2][0]) +
                 in_CCM[0][2] * (in_CCM[1][0] * in_CCM[2][1] - in_CCM[1][1] * in_CCM[2][0]);

    if (det == 0.0)
    {
        MY_ERR("Fail to inverse the matrix");
        return MFALSE;
    }

    double invdet = 1.0 / det;

    double inv_CCM[3][3];
    inv_CCM[0][0] = (in_CCM[1][1] * in_CCM[2][2] - in_CCM[2][1] * in_CCM[1][2]) * invdet;
    inv_CCM[0][1] = (in_CCM[0][2] * in_CCM[2][1] - in_CCM[0][1] * in_CCM[2][2]) * invdet;
    inv_CCM[0][2] = (in_CCM[0][1] * in_CCM[1][2] - in_CCM[0][2] * in_CCM[1][1]) * invdet;
    inv_CCM[1][0] = (in_CCM[1][2] * in_CCM[2][0] - in_CCM[1][0] * in_CCM[2][2]) * invdet;
    inv_CCM[1][1] = (in_CCM[0][0] * in_CCM[2][2] - in_CCM[0][2] * in_CCM[2][0]) * invdet;
    inv_CCM[1][2] = (in_CCM[1][0] * in_CCM[0][2] - in_CCM[0][0] * in_CCM[1][2]) * invdet;
    inv_CCM[2][0] = (in_CCM[1][0] * in_CCM[2][1] - in_CCM[2][0] * in_CCM[1][1]) * invdet;
    inv_CCM[2][1] = (in_CCM[2][0] * in_CCM[0][1] - in_CCM[0][0] * in_CCM[2][1]) * invdet;
    inv_CCM[2][2] = (in_CCM[0][0] * in_CCM[1][1] - in_CCM[1][0] * in_CCM[0][1]) * invdet;

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            out_CM[i][j] = inv_CCM[i][0]*M[0][j] + inv_CCM[i][1]*M[1][j] + inv_CCM[i][2]*M[2][j];
        }
    }

    double CA_inv_M[3][3];

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            CA_inv_M[i][j] = CA[i][0]*inv_M[0][j] + CA[i][1]*inv_M[1][j] + CA[i][2]*inv_M[2][j];
        }
    }

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            out_FM[i][j] = CA_inv_M[i][0]*in_CCM[0][j] + CA_inv_M[i][1]*in_CCM[1][j] + CA_inv_M[i][2]*in_CCM[2][j];
        }
    }

    return MTRUE;
}

};
