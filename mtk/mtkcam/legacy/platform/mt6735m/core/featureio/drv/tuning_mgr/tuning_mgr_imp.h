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
#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#ifndef _TUNING_MGR_IMP_H_
#define _TUNING_MGR_IMP_H_

#include <cutils/log.h>    // for ALOG?().

#include <mtkcam/common.h>
using namespace NSCam;
#include <mtkcam/drv_common/isp_reg.h>
//#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/featureio/tuning_mgr.h>
//#include <mtkcam/drv/tpipe_drv.h>
#include <mtkcam/drv_common/imem_drv.h>

//-----------------------------------------------------------------------------
using namespace android;
using namespace NSIspDrv_FrmB;
//-----------------------------------------------------------------------------

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""
#undef   DBG_LOG_LEVEL                      // Decide Debug Log level for current file. Can only choose from 2~8.
#define  DBG_LOG_LEVEL      4               // 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).


#define DBG_LOG_LEVEL_SILENT    8
#define DBG_LOG_LEVEL_ASSERT    7
#define DBG_LOG_LEVEL_ERROR        6
#define DBG_LOG_LEVEL_WARN        5
#define DBG_LOG_LEVEL_INFO        4
#define DBG_LOG_LEVEL_DEBUG        3
#define DBG_LOG_LEVEL_VERBOSE    2

#undef    __func__
#define    __func__    __FUNCTION__


#if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_SILENT)        // 8
    #undef LOG_AST
    #undef LOG_ERR
    #undef LOG_WRN
    #undef LOG_INF
    #undef LOG_DBG
    #undef LOG_VRB
    #define LOG_AST(cond, fmt, arg...)
    #define LOG_ERR(fmt, arg...)
    #define LOG_WRN(fmt, arg...)
    #define LOG_INF(fmt, arg...)
    #define LOG_DBG(fmt, arg...)
    #define LOG_VRB(fmt, arg...)
#endif    // (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_SILENT)


#ifndef USING_MTK_LDVT   // Not using LDVT.
//    #include <cutils/log.h>
    #define NEW_LINE_CHAR           ""      // ALOG?() already includes a new line char at the end of line, so don't have to add one.

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ASSERT)        // 7
        #undef LOG_AST
        #define LOG_AST(cond, fmt, arg...)      \
            do {        \
                if (!(cond))        \
                    ALOGE("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
            } while (0)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ERROR)        // 6
        #undef LOG_ERR
        #define LOG_ERR(fmt, arg...)        ALOGE(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg)    // When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_WARN)        // 5
        #undef LOG_WRN
        #define LOG_WRN(fmt, arg...)        ALOGW(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_INFO)        // 4
        #undef LOG_INF
        #define LOG_INF(fmt, arg...)        ALOGI(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Info>: Show general system information. Like OS version, start/end of Service...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_DEBUG)        // 3
        #undef LOG_DBG
        #define LOG_DBG(fmt, arg...)        ALOGD(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_VERBOSE)    // 2
        #undef LOG_VRB
        #define LOG_VRB(fmt, arg...)        ALOGD(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #endif
#else   // Using LDVT.
    #include "uvvf.h"
    #define NEW_LINE_CHAR           "\n"

#if 1    // Enable LOG_*().
    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ASSERT)        // 7
        #undef LOG_AST
        #define LOG_AST(expr, fmt, arg...)                                                                                                       \
            do {                                                                                                                                    \
                if (!(expr))                                                                                                                        \
                    VV_ERRMSG("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);     \
            } while (0)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ERROR)        // 6
        #undef LOG_ERR
        #define LOG_ERR(fmt, arg...)        VV_ERRMSG(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg)    // When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_WARN)        // 5
        #undef LOG_WRN
        #define LOG_WRN(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_INFO)        // 4
        #undef LOG_INF
        #define LOG_INF(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Info>: Show general system information. Like OS version, start/end of Service...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_DEBUG)        // 3
        #undef LOG_DBG
        #define LOG_DBG(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_VERBOSE)    // 2
        #undef LOG_VRB
        #define LOG_VRB(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #endif
#else    // Disable LOG_*().
    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ASSERT)        // 7
        #undef LOG_AST
        #define LOG_AST(expr, fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ERROR)        // 6
        #undef LOG_ERR
        #define LOG_ERR(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_WARN)        // 5
        #undef LOG_WRN
        #define LOG_WRN(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_INFO)        // 4
        #undef LOG_INF
        #define LOG_INF(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_DEBUG)        // 3
        #undef LOG_DBG
        #define LOG_DBG(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_VERBOSE)    // 2
        #undef LOG_VRB
        #define LOG_VRB(fmt, arg...)
    #endif

#endif    // Enable/Disable LOG_*().

#endif  // USING_MTK_LDVT


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef struct{
    ETuningMgrFunc eTuningMgrUpdateFunc;
    MUINT32* pCurWriteTuningQue; // the queue of featureio path be used currently
}TUNING_MGR_UPDATE_INFO;


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class TuningMgrImp : public TuningMgr
{
    friend  TuningMgr& TuningMgr::getInstance();
    public:

    protected:
        TuningMgrImp();
        ~TuningMgrImp();
    //
    public:
        virtual MBOOL   init();
        virtual MBOOL   uninit();
        //
        //
        virtual MBOOL updateEngine(ESoftwareScenario scenario, ETuningMgrFunc engin);
        virtual MBOOL dequeBuffer(ESoftwareScenario scenario, MINT32 MagicNum);
        virtual MBOOL enqueBuffer(ESoftwareScenario scenario, MINT32 MagicNum);
        virtual MBOOL flushSetting(ESoftwareScenario scenario);
        virtual MBOOL byPassSetting(ESoftwareScenario scenario, MINT32 MagicNum);

    public:
        virtual MBOOL tuningMgrWriteRegs(ESoftwareScenario scenario, TUNING_MGR_REG_IO_STRUCT*  pRegIo , MINT32 cnt);
        virtual MBOOL tuningMgrWriteReg(ESoftwareScenario scenario, MUINT32 addr, MUINT32 data);
        virtual MUINT32 tuningMgrReadReg(ESoftwareScenario scenario,MUINT32 addr);
        //
    private:
        //
        NSIspDrv_FrmB::IspDrv      *pIspDrv;
        //
        mutable Mutex       mLock;
        mutable Mutex       mQueueInfoLock;
        volatile MINT32     mInitCount;
        //
        TUNING_MGR_UPDATE_INFO mTuningQueUpdateInf[ISP_DRV_P2_CQ_NUM];
        //
};

//-----------------------------------------------------------------------------
#endif // _TUNING_MGR_IMP_H_

