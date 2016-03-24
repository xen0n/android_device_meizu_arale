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
#define MTK_LOG_ENABLE 1
#define LOG_TAG "TuningMgr"

#include <cutils/log.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <semaphore.h>

#include "tuning_mgr_imp.h"
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*() CameraProfile APIS.
using namespace CPTool;

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
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

#define TUNING_MGR_ISP_CLEAN_TOP_NUM    6
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
isp_reg_t*  TuningMgr::mpIspReferenceRegMap;
pthread_mutex_t mQueueTopAccessLock;


//
MUINT32 tuningMgrCleanSetting[TUNING_MGR_ISP_CLEAN_TOP_NUM] = {0x4004, 0x4008, 0x4010, 0x4014, 0x4018, 0x401C};


/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
//-----------------------------------------------------------------------------
TuningMgrImp::TuningMgrImp()
                : pIspDrv(NULL)
                , mLock()
                , mQueueInfoLock()
                , mInitCount(0)
{
    int i;
    LOG_DBG("");
    GLOBAL_PROFILING_LOG_START(Event_TdriMgr);    // Profiling Start.
    mInitCount = 0;
    //
}

//-----------------------------------------------------------------------------
TuningMgrImp::~TuningMgrImp()
{
    LOG_DBG("");
    GLOBAL_PROFILING_LOG_END();     // Profiling End.
}

//-----------------------------------------------------------------------------
TuningMgr& TuningMgr::getInstance(void)
{
    static  TuningMgrImp Singleton;
    return  Singleton;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::init()
{
    MBOOL Result = MTRUE;
    //
    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("init TuningMgr");

    //
    Mutex::Autolock lock(mLock); // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

    LOG_INF("mInitCount(%d)",mInitCount);
    //
    if(mInitCount == 0) {
        int i;
        LOCAL_PROFILING_LOG_PRINT("run tuning mgr init");

        // create isp driver instance
        pIspDrv = NSIspDrv_FrmB::IspDrv::createInstance();

        if (!pIspDrv) {
            LOG_ERR("IspDrv::createInstance() fail \n");
            Result = MFALSE;
            goto EXIT;
        }
        //
        if( pIspDrv->init("tuning_mgr") == 0) {
            LOG_ERR("IspDrv::init() fail \n");
            Result = MFALSE;
            goto EXIT;
        }
        //
        for(i=ISP_DRV_P2_CQ1;i<ISP_DRV_P2_CQ_NUM;i++){
            mTuningQueUpdateInf[i].eTuningMgrUpdateFunc = eTuningMgrFunc_Null;
            mTuningQueUpdateInf[i].pCurWriteTuningQue = NULL;
        }
        //
        mpIspReferenceRegMap = (isp_reg_t*)malloc(sizeof(isp_reg_t));    //always allocate this(be used to get register offset in macro)
    }
    //
    android_atomic_inc(&mInitCount);
    //
    EXIT:
    LOG_INF("X:\n");
    //
    return Result;
}



//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::uninit()
{
    MBOOL Result = MTRUE;
    int i;
    //
    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("Uninit TuningMgr");
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
    if(mpIspReferenceRegMap != NULL)
    {
        free((MUINT32*)mpIspReferenceRegMap);
        mpIspReferenceRegMap = NULL;
    }

    //isp drv
    pIspDrv->uninit("tuning_mgr");
    pIspDrv->destroyInstance();
    pIspDrv = NULL;


    LOG_INF("Release\n");

EXIT:
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    updateEngine(
        ESoftwareScenario scenario,
        ETuningMgrFunc engin
)
{
    Mutex::Autolock lock(mQueueInfoLock);
    //
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MBOOL ret = MTRUE;
    LOG_DBG("scenario(%d),engin(0x%x)",scenario,engin);
    //
    if(pIspDrv->getP2cqInfoFromScenario(scenario, p2Cq) != MTRUE){
        LOG_ERR("[Error]getP2cqInfoFromScenario fail");
        ret = MFALSE;
    }

    mTuningQueUpdateInf[p2Cq].eTuningMgrUpdateFunc = (ETuningMgrFunc)((MUINT32)engin|(MUINT32)mTuningQueUpdateInf[p2Cq].eTuningMgrUpdateFunc);

    LOG_DBG("p2Cq(%d),eTuningMgrUpdateFunc(0x%x)",p2Cq,mTuningQueUpdateInf[p2Cq].eTuningMgrUpdateFunc);

    return ret;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    dequeBuffer(
        ESoftwareScenario scenario,
        MINT32 MagicNum
)
{
    Mutex::Autolock lock(mQueueInfoLock);
    //
    MBOOL ret = MTRUE;
    ISP_DRV_CQ_ENUM cq;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    LOG_INF("scenario(%d),MagicNum(0x%x)",scenario,MagicNum);
    //
    ret = pIspDrv->deTuningQue(eTuningQueAccessPath_featureio, scenario, MagicNum);
    if(ret == MFALSE){
        LOG_ERR("[Error]deTuningQue fail");
        ret = MFALSE;
    }
    //
    if(pIspDrv->getCqInfoFromScenario(scenario, cq) != MTRUE){
        LOG_ERR("[Error]getCqInfoFromScenario fail");
        ret = MFALSE;
    }
    pIspDrv->mapCqToP2Cq(cq,p2Cq);
    mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue = pIspDrv->getTuningBuf(eTuningQueAccessPath_featureio, cq);
    LOG_DBG("p2Cq(%d),pCurWriteTuningQue(0x%08x)",p2Cq,mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue);
    //
    // clean top setting
    for(int i=0;i<TUNING_MGR_ISP_CLEAN_TOP_NUM;i++){
        mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue[tuningMgrCleanSetting[i]>>2] = 0x00;
    }
    //
    return ret;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    enqueBuffer(
        ESoftwareScenario scenario,
        MINT32 MagicNum
)
{
    Mutex::Autolock lock(mQueueInfoLock);

    MBOOL ret = MTRUE;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    LOG_DBG("scenario(%d),MagicNum(0x%x)",scenario,MagicNum);
    //
    if(pIspDrv->getP2cqInfoFromScenario(scenario, p2Cq) != MTRUE){
        LOG_ERR("[Error]getP2cqInfoFromScenario fail");
    }
    ret = pIspDrv->enTuningQue(eTuningQueAccessPath_featureio, scenario, MagicNum, (EIspTuningMgrFunc)mTuningQueUpdateInf[p2Cq].eTuningMgrUpdateFunc);
    if(ret == MFALSE){
        LOG_ERR("[Error]enTuningQue fail");
        ret = MFALSE;
    }
    mTuningQueUpdateInf[p2Cq].eTuningMgrUpdateFunc = eTuningMgrFunc_Null;
    mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue = NULL;
    //
    return ret;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    flushSetting(
        ESoftwareScenario scenario)
{
    Mutex::Autolock lock(mQueueInfoLock);

    ISP_DRV_CQ_ENUM cq;
    MBOOL ret = MTRUE;
    LOG_INF("scenario(%d)",scenario);
    //
    if(pIspDrv->getCqInfoFromScenario(scenario, cq) != MTRUE){
        LOG_ERR("[Error]getInfoFromScenario fail");
        ret = MFALSE;
    }
    pIspDrv->setP2TuningStatus(cq, MTRUE);  // set tuning path to ture

    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    byPassSetting(
        ESoftwareScenario scenario,
        MINT32 MagicNum)
{
    Mutex::Autolock lock(mQueueInfoLock);
    //
    MBOOL ret = MTRUE;
    LOG_INF("+,scenario(%d),MagicNum(%d)",scenario,MagicNum);
    //
    ret = pIspDrv->bypassTuningQue(scenario, MagicNum);
    if(ret == MFALSE){
        LOG_ERR("[Error]bypassTuningQue fail");
        ret = MFALSE;
    }
    //
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteRegs(
        ESoftwareScenario scenario,
        TUNING_MGR_REG_IO_STRUCT*  pRegIo ,
        MINT32 cnt)
{
    Mutex::Autolock lock(mQueueInfoLock);
    //
    ISP_DRV_P2_CQ_ENUM p2Cq;
    MBOOL ret = MFALSE;
    MUINT32 i;
    //
    LOG_DBG("startAddr(0x%04x),Cnt(%d)",pRegIo[0].Addr,cnt);
    //
    if(pIspDrv->getP2cqInfoFromScenario(scenario, p2Cq) != MTRUE){
        LOG_ERR("[Error]getP2cqInfoFromScenario fail");
    }
    for(i=0;i<cnt;i++){
        mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue[pRegIo[i].Addr>>2] = pRegIo[i].Data;
        LOG_VRB("i(%d),(0x%04x,0x%08x)",i,pRegIo[i].Addr,pRegIo[i].Data);
    }

    return ret;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteReg(
        ESoftwareScenario scenario,
        MUINT32 addr,
        MUINT32 data)
{
    Mutex::Autolock lock(mQueueInfoLock);
    //
    MBOOL ret = MFALSE;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    if(pIspDrv->getP2cqInfoFromScenario(scenario, p2Cq) != MTRUE){
        LOG_ERR("[Error]getP2cqInfoFromScenario fail,scenario(%d),p2Cq(%d)",scenario,p2Cq);
        return -1;
    }
    if(mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue == NULL){
        LOG_ERR("[Error]pCurWriteTuningQue is null point,scenario(%d),p2Cq(%d)",scenario,p2Cq);
        return -1;
    }

    mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue[addr>>2] = data;

    LOG_VRB("p2Cq(%d),addr(0x%08x),data(0x%08x),pCurWriteTuningQue(0x%08x)",p2Cq,addr,data,mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue);

    return ret;
}

//-----------------------------------------------------------------------------
MUINT32 TuningMgrImp::
    tuningMgrReadReg(
        ESoftwareScenario scenario,
        MUINT32 addr)
{
    Mutex::Autolock lock(mQueueInfoLock);

    MUINT32 value=0x0;
    ISP_DRV_P2_CQ_ENUM p2Cq;
    //
    if(pIspDrv->getP2cqInfoFromScenario(scenario, p2Cq) != MTRUE){
        LOG_ERR("[Error]getP2cqInfoFromScenario fail,scenario(%d),p2Cq(%d)",scenario,p2Cq);
        return -1;
    }
    if(mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue == NULL){
        LOG_ERR("[Error]pCurWriteTuningQue is null point,scenario(%d),p2Cq(%d)",scenario,p2Cq);
        return -1;
    }
    value = mTuningQueUpdateInf[p2Cq].pCurWriteTuningQue[addr>>2];
    LOG_VRB("p2Cq(%d),addr(0x%08x),data(0x%08x)",p2Cq,addr,value);

    return value;
}


//-----------------------------------------------------------------------------






