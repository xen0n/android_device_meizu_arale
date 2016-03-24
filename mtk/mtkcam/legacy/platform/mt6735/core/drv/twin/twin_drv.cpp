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
#define LOG_TAG "TwinDrv"
//
#include <utils/Errors.h>
//#include <cutils/pmem.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>  // For property_get().
#include <cutils/atomic.h>
#include <sys/ioctl.h>

#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*() CameraProfile APIS.
using namespace CPTool;

#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/drv/twin_drv.h>
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/BuiltinTypes.h>    // For type definitions.
#include "MyUtils.h"
//#include "drv_types.h"
#include "twin_drv_imp.h"

//TODO:remove later
enum EPortIndex
{
    EPortIndex_TG1I,        // 0
    EPortIndex_TG2I,
    EPortIndex_CAMSV_TG1I,
    EPortIndex_CAMSV_TG2I,
    //
};

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{TwinDrv} "
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(twin_drv);
//EXTERN_DBG_LOG_VARIABLE(twin_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (twin_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (twin_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (twin_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (twin_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (twin_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (twin_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define TWIN_ALIGNMENT_UP( _number_, _power_of_2_ ) (((((size_t)_number_) + (( 0x1 << (_power_of_2_) )-1)) >> (_power_of_2_) ) << (_power_of_2_))
#define TWIN_DRV_INIT_MAX        5



class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
//        if  (0==mIdx)
//        {
//            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
//        }
//        else
//        {
            LOG_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};


#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 1   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog(G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr(G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable(EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr(G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/


#define ISP_SCENARIO_TWIN_IP         3
#define ISP_SCENARIO_TWIN_CC         6
//
#define ISP_TWIN_SUB_MODE_RAW        0
#define ISP_TWIN_SUB_MODE_YUV        1
#define ISP_TWIN_SUB_MODE_RGB        2
#define ISP_TWIN_SUB_MODE_JPG        3
#define ISP_TWIN_SUB_MODE_MFB        4
#define ISP_TWIN_SUB_MODE_VEC        0
#define ISP_TWIN_SUB_MODE_RGB_LOAD   3
#define ISP_TWIN_SUB_MODE_MAX        5


/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
//-----------------------------------------------------------------------------
TwinDrvImp::TwinDrvImp():m_pImemDrv(NULL)
{
    LOG_DBG("TwinDrvImp()");
    //
    GLOBAL_PROFILING_LOG_START(Event_TwinDrv);    // Profiling Start.
    mInitCount = 0;
}


//-----------------------------------------------------------------------------
TwinDrvImp::~TwinDrvImp()
{
    LOG_DBG("~TwinDrvImp()");
    //
    GLOBAL_PROFILING_LOG_END();     // Profiling End.
}


//-----------------------------------------------------------------------------
TwinDrv*
TwinDrv::createInstance(void)
{
    DBG_LOG_CONFIG(drv, twin_drv);

    return TwinDrvImp::getInstance();
}

//-----------------------------------------------------------------------------
TwinDrv*
TwinDrvImp::getInstance(void)
{
    static TwinDrvImp Singleton;
    //
    LOG_DBG("&Singleton(0x%x)\n",&Singleton);
    //
    return &Singleton;
}

//-----------------------------------------------------------------------------
void
TwinDrvImp::destroyInstance(void)
{
    LOG_DBG("");
}

//-----------------------------------------------------------------------------
MBOOL
TwinDrvImp::init(void)
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("init TwinDrv");

    //
    Mutex::Autolock lock(mLock); // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.
    //
    LOG_INF("mInitCount(%d)",mInitCount);
    //
    if(mInitCount == 0) {
        int i;
        //
        if ( NULL == m_pImemDrv ) {
            m_pImemDrv = IMemDrv::createInstance();
            m_pImemDrv->init();
        } else {
            LOG_ERR("[Error] m_pImemDrv(0x%x) not equal to null\n",m_pImemDrv);
        }

#if 0
        //
        // twin main working buffer
        if (m_pImemDrv->allocVirtBuf(&m_twinDataInfo)) {
            LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf(m_twinDataInfo) Fail, size=0x%x!!!", m_twinDataInfo.size);
        }
        //
        // for twin debug
        for(i=0; i<TWIN_DBG_BUFFER_NUM; i++){
        }

        //twinlib working buffer
        int twinWorkingSize;
        //twinWorkingSize = twin_main_query_platform_working_buffer_size(TWIN_DRV_MAX_TWIN_NUM);
        twinWorkingSize = TWIN_ALIGNMENT_UP(twinWorkingSize, 2); // 4 alignment
        if ( NULL != m_pImemDrv ) {
            //
            m_WBInfo.size = twinWorkingSize;
            if (m_pImemDrv->allocVirtBuf(&m_WBInfo)) {
                LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf(m_WBInfo) Fail, sz=0x%x!!!", twinWorkingSize);
            }
        } else {
            LOG_ERR("[Error] m_pImemDrv(0x%x) not equal to null\n",m_pImemDrv);
        }
#endif

    }

    //
    if(mInitCount >= TWIN_DRV_INIT_MAX) {
        LOG_ERR("over max mInitCount(%d)",mInitCount);
        Result = MFALSE;
        goto EXIT;
    }
    //
    android_atomic_inc(&mInitCount);

    //
    EXIT:
    LOG_INF("X",mInitCount);
    //
    return Result;
}



//-----------------------------------------------------------------------------
MBOOL
TwinDrvImp::uninit(void)
{
    MBOOL Result = MTRUE;
    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("Uninit TpieDrv");
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_INF("mInitCount(%d)",mInitCount);
    //
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0) {
        goto EXIT;
    }
    //
    if(mInitCount == 0) {
        int i;

        //release tdri data buffer
        //m_twinDataInfo is filled by alloc
#if 0
        if ( m_pImemDrv->freeVirtBuf(&m_twinDataInfo) ) {
            LOG_ERR("ERROR:m_pImemDrv->freeVirtBuf");
            Result = MFALSE;
            goto EXIT;
        }
        m_twinDataInfo.size = 0;
        m_twinDataInfo.memID = -1;
        m_twinDataInfo.virtAddr = NULL;
#endif

        m_pImemDrv->uninit();
        m_pImemDrv->destroyInstance();
        m_pImemDrv = NULL;
    }
    //
    EXIT:
    LOG_INF("X",mInitCount);
    return Result;
}

//
MBOOL TwinDrvImp::
runDbgTwinMain(void)
{
    int i;

    for(i=0; i<TWIN_DBG_BUFFER_NUM; i++){
        LOG_INF("X:runDbgTwinMain(%d)",i);
#if 0 //_mt6593fpga_dvt_use_
        twin_print_platform_config((const ISP_TWIN_CONFIG_STRUCT *)pDbgTwinStruct[i]);
#endif
        LOG_INF("E(%d)",i);
    }
    //
    return MTRUE;
}
//
MBOOL TwinDrvImp::
runTwinMain(
    TwinDrvCfg* pTdriInfo
)
{
MBOOL ret = MTRUE;

    LOCAL_PROFILING_LOG_AUTO_START(Event_TwinDrv);
    LOG_DBG("Start to Config Twin Mode");
    LOCAL_PROFILING_LOG_PRINT("run twin main");

    if ( 0 != dual_cal_platform(&m_dual_isp_in_config, &m_dual_isp_out_config,NULL, NULL) ) {
        LOG_ERR("twin cal error!");
        ret = MFALSE;
    }

    LOCAL_PROFILING_LOG_PRINT("end twin main");

    return ret;

}


MBOOL TwinDrvImp::
configTwinPara(
    TwinDrvCfg* pTdriInfo
)
{
MBOOL ret = MTRUE;
TwinDrvCfg twinInfo;

    Mutex::Autolock lock(mLock); // acquires a lock on m_mutex

    ret = runTwinMain((TwinDrvCfg*)&twinInfo);


EXIT:
    return ret;

}


MUINT32
TwinDrvImp::
calCoefStep(
    MUINT32 in,
    MUINT32 crop,
    MUINT32 crop_ofst,
    MUINT32 crop_sub_ofst,
    MUINT32 out
)
{
/*
        MUINT32 rrz_in_w        = rrz_cfg.rrz_in_size.w;
        MUINT32 rrz_in_crop_w   = rrz_cfg.rrz_crop.w;

        MUINT32 left_ofst = rrz_cfg.rrz_crop.x;
        MUINT32 left_sub_ofst  = rrz_cfg.rrz_crop.floatX;
        MUINT32 right_ofst      = rrz_in_w- left_ofst - rrz_in_crop_w - (left_sub_ofst?1:0);

        MUINT32 rrz_out_w = rrz_cfg.rrz_crop.w;

        //TODO:define in dual_cal.h. remove later
#define DUAL_RRZ_PREC (1<<15)

        MUINT32 rrz_h_step = ( (rrz_in_w - 1 - left_ofst - right_ofst)*DUAL_RRZ_PREC ) / (rrz_out_w - 1);
*/


    MUINT32 rrz_h_step = ( ( in - 1 - crop_ofst - ( in- crop_ofst - crop - (crop_sub_ofst?1:0) ) )*DUAL_RRZ_PREC ) / (out - 1);

    LOG_DBG("+ in/crop/ofst/sub/out(%d/%d/%d/%d/%d),step(0x%x)",in,crop,crop_ofst,crop_sub_ofst,out,rrz_h_step);

    return rrz_h_step;
}

MUINT32
TwinDrvImp::
getCoefTbl(
    MUINT32 resize_in,
    MUINT32 resize_out
)
{
    /*
            resizing ratio  suggested table
               1.0~0.9           0~3
               0.9~0.8           4~7
               0.8~0.7           8~12
               0.7~0.6          13~18
               0.6~0.5          19~24
               0.5~0.4          25~30
        */

struct {
    MUINT32 left;
    MUINT32 right;
}map_tbl[11] = {{1,1},{1,1},{1,1},{1,1},{25,30},{19,24},{13,18},{8,12},{4,7},{0,3},{0,3}};

MUINT32 scale_ratio_int = (MUINT32)( (resize_out*10) / resize_in );
MUINT32 scale_ratio_int_100 = (MUINT32)( (resize_out*100) / resize_in );
MUINT32 table = 0;

    LOG_DBG("+in/out(%d/%d),ratio(%d)",resize_in,resize_out,scale_ratio_int);

    if ( 0 == resize_in || 0 == resize_out ) {
        LOG_ERR("rrz param error:0 size(%d/%d)",resize_in,resize_out);
        return 0;
    }
    if ( resize_out > resize_in ) {
        LOG_ERR("rrz param error:not support size up");
        return 0;
    }
    if ( 4 > scale_ratio_int ) {
        LOG_ERR("rrz param error:max 0.4(%d/%d)",resize_in,resize_out);
        return 0;
    }

    if ( resize_out == resize_in ) {
        table = 0x1F; //table index 31
    }
    else {
        table = map_tbl[scale_ratio_int].left + \
                  ( ( (scale_ratio_int+1)*10 - scale_ratio_int_100 )* \
                  ( map_tbl[scale_ratio_int].right-map_tbl[scale_ratio_int].left ) )/10;
        table &= 0x1F;
    }

    LOG_DBG("table(%d)",table);

    return table;
}





//-----------------------------------------------------------------------------

