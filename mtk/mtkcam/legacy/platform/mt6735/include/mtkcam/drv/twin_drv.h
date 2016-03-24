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
#ifndef _TWIN_DRV_H_
#define _TWIN_DRV_H_
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#if 0
enum EPortIndex
{
    EPortIndex_TG1I,        // 0
    EPortIndex_TG2I,
    EPortIndex_CAMSV_TG1I,
    EPortIndex_CAMSV_TG2I,
    //
};
#endif
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
#if 0
class TwinMemBuffer
{
public:
    unsigned int size;
    unsigned int cnt;
    unsigned int base_vAddr;
    unsigned int base_pAddr;
    unsigned int ofst_addr;
    unsigned int alignment;
public:
    TwinMemBuffer():
        size(0),cnt(0),base_vAddr(0),base_pAddr(0),ofst_addr(0), alignment(16)
        {};
};
#endif

class TwinSize
{
public:
    unsigned long w;
    unsigned long h;
    unsigned long stride;

public:
    TwinSize():
        w(0),h(0), stride(0)
        {};


    TwinSize(unsigned long _w, unsigned long _h )
        {
            w = _w; h = _h;
        };

};


class TwinRect
{
public:
    MUINT32         x;
    MUINT32         y;
    MUINT32         floatX; /* x float precise - 32 bit */
    MUINT32         floatY; /* y float precise - 32 bit */
    unsigned long   w;
    unsigned long   h;


public:
    TwinRect():
        x(0),y(0),floatX(0),floatY(0),w(0),h(0)
        {};

   TwinRect(long _x, long _y, long _floatX, long _floatY, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; floatX = _floatX; floatY = _floatY; w = _w; h = _h;
        };

};

class stRRZCfg
{
public:
    unsigned long isTwinMode;
    unsigned long pass1_path;
    unsigned long lsc_en;
    unsigned long sd_lwidth;
    unsigned long sd_xnum;
    unsigned long m_num;
    TwinSize rrz_in_size;
    TwinRect rrz_in_roi;
    TwinSize rrz_out_size;
public:
    stRRZCfg(){};
};

class TwinDrvCfg
{
public:
    stRRZCfg rrz_cfg;

};

/**************************************************************************
 *                 E X T E R N A L    I N T E R F A C E                             *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/


/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class TwinDrv
{
    public:


    protected:
        TwinDrv() {};
        virtual ~TwinDrv() {};
    //
    public:
        static TwinDrv*  createInstance(void);
        virtual void     destroyInstance(void) = 0;

        virtual int     init(void) = 0;
        virtual int     uninit(void) = 0;
        virtual int     configTwinPara(TwinDrvCfg* pTwinInfo) = 0;
        virtual MBOOL   runDbgTwinMain(void) = 0;
    public://driver object operation
        virtual void*    getInCfg(void) = 0;
        virtual void*    getOutCfg(void) = 0;
        virtual MUINT32  calCoefStep(MUINT32 in, MUINT32 crop, MUINT32 crop_ofst, MUINT32 crop_sub_ofst, MUINT32 out) = 0;
        virtual MUINT32  getCoefTbl(MUINT32 resize_in, MUINT32 resize_out) = 0;
};

//-----------------------------------------------------------------------------
#endif  // _TWIN_DRV_H_

