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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/IState.h>
#include "State.h"
using namespace NSMtkEngCamAdapter;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

namespace android {
namespace NSMtkEngCamAdapter {
/*******************************************************************************
*   StateManager
*******************************************************************************/
class StateManager : public IStateManager
{
public:     ////            Instantiation.
    virtual void            destroyInstance();

public:
    virtual bool            init();
    virtual bool            uninit();

public:     ////            Attributes.
    virtual IState*         getCurrentState() const { return mpCurrState; }

    virtual bool            isState(ENState const eState);

public:     ////            Operations.
#if 0
    //
    //  eState:
    //      [in] the state to wait.
    //
    //  timeout:
    //      [in] the timeout to wait in nanoseconds. -1 indicates no timeout.
    //
    virtual status_t        waitState(ENState const eState, nsecs_t const timeout = -1);
#endif
    virtual status_t        transitState(ENState const eNewState);

    virtual bool            registerOneShotObserver(IObserver* pObserver);
    virtual void            unregisterObserver(IObserver* pObserver);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////            Instantiation.
    IState*                 getStateInst(ENState const eState);
    friend class            StateObserver;

public:     ////            Instantiation.
                            StateManager();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////            Data Members.
    Mutex                   mStateLock;
    Condition               mStateCond;
    IState* volatile        mpCurrState;        //  Pointer to the current state.
    //
    typedef List<IObserver*>ObserverList_t;
    Mutex                   mObserverLock;
    ObserverList_t          mObserverList;
    //
    IState*                 mpStateIdle;
    IState*                 mpStatePreview;
    IState*                 mpStatePreCapture;
    IState*                 mpStateCapture;
    IState*                 mpStatePreviewCapture;
    IState*                 mpStateRecording;
    IState*                 mpStateVideoSnapshot;
};

}; // namespace NSMtkEngCamAdapter
}; // namespace android

/*******************************************************************************
 *
 ******************************************************************************/
IState*
StateManager::
getStateInst(ENState const eState)
{
    switch  (eState)
    {
        case IState::eState_Idle:
            return mpStateIdle;
        case IState::eState_Preview:
            return mpStatePreview;
        case IState::eState_PreCapture:
            return mpStatePreCapture;
        case IState::eState_Capture:
            return mpStateCapture;
        case IState::eState_PreviewCapture:
            return mpStatePreviewCapture;
        case IState::eState_Recording:
            return mpStateRecording;
        case IState::eState_VideoSnapshot:
            return mpStateVideoSnapshot;
        default:
            MY_LOGW("bad eState(%d)", eState);
            break;
    };
    return  NULL;
}


/*******************************************************************************
 *
 ******************************************************************************/
IStateManager*
IStateManager::
createInstance()
{
    return new StateManager;
}


/*******************************************************************************
 *
 ******************************************************************************/
void
StateManager::
destroyInstance()
{
    MY_LOGD("(%p)", this);
    delete this;
}


/*******************************************************************************
 *
 ******************************************************************************/
StateManager::
StateManager()
    : IStateManager()
    , mStateLock()
    , mStateCond()
    , mpCurrState(NULL)
    //
    , mObserverLock()
    , mObserverList()
    //
    , mpStateIdle(NULL)
    , mpStatePreview(NULL)
    , mpStatePreCapture(NULL)
    , mpStateCapture(NULL)
    , mpStatePreviewCapture(NULL)
    , mpStateRecording(NULL)
    , mpStateVideoSnapshot(NULL)

{
    MY_LOGD("(%p)", this);
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
init()
{
    //
    mpStateIdle          = new StateIdle(IState::eState_Idle, this);
    mpStatePreview       = new StatePreview(IState::eState_Preview, this);
    mpStatePreCapture    = new StatePreCapture(IState::eState_PreCapture, this);
    mpStateCapture       = new StateCapture(IState::eState_Capture, this);
    mpStatePreviewCapture   = new StatePreviewCapture(  IState::eState_PreviewCapture,  this);
    mpStateRecording     = new StateRecording(IState::eState_Recording, this);
    mpStateVideoSnapshot = new StateVideoSnapshot(IState::eState_VideoSnapshot, this);
    //
    mpCurrState          = getStateInst(IState::eState_Idle);
    //
    return true;
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
uninit()
{
    //
    delete mpStateIdle;
    delete mpStatePreview;
    delete mpStatePreCapture;
    delete mpStateCapture;
    delete mpStatePreviewCapture;
    delete mpStateRecording;
    delete mpStateVideoSnapshot;
    //
    return true;
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
isState(ENState const eState)
{
    IState*const pWaitedState = getStateInst(eState);
    //
    Mutex::Autolock _lock(mStateLock);
//    MY_LOGD_IF(1, "(%d)[%s] current/waited=%s/%s", ::gettid(), __FUNCTION__, mpCurrState->getName(), pWaitedState->getName());
    if  ( pWaitedState != mpCurrState )
    {
        MY_LOGD("current/waited=%s/%s", mpCurrState->getName(), pWaitedState->getName());
        return  false;
    }
    //
    return  true;
}


/*******************************************************************************
 *
 ******************************************************************************/
#if 0
status_t
StateManager::
waitState(ENState const eState, nsecs_t const timeout /*= -1*/)
{
    status_t status = OK;
    //
    IState*const pWaitedState = getStateInst(eState);
    //
    Mutex::Autolock _lock(mStateLock);
    if  ( pWaitedState != mpCurrState )
    {
        MY_LOGD_IF(1, "current/waited=%s/%s, timeout(%lld)", mpCurrState->getName(), pWaitedState->getName(), timeout);
        switch  (timeout)
        {
        case 0:     //  not wait.
            status = TIMED_OUT;
            break;
        case -1:    //  wait without timeout.
            status = mStateCond.wait(mStateLock);
            break;
        default:    //  wait with a given timeout.
            status = mStateCond.waitRelative(mStateLock, timeout);
            break;
        }
        //
        if  ( pWaitedState != mpCurrState || OK != status ) {
            MY_LOGW("Timeout: current/waited=%s/%s, status[%s(%d)]", mpCurrState->getName(), pWaitedState->getName(), ::strerror(-status), -status);
        }
    }
    //
    return  status;
}
#endif

/*******************************************************************************
 *
 ******************************************************************************/
status_t
StateManager::
transitState(ENState const eNewState)
{
    IState*const pNewState = getStateInst(eNewState);
    if  ( ! pNewState )
    {
        MY_LOGW("pNewState==NULL (eNewState:%d)", eNewState);
        return  INVALID_OPERATION;
    }
    //
    {
        Mutex::Autolock _lock(mStateLock);
        MY_LOGI("%s --> %s", mpCurrState->getName(), pNewState->getName());
        mpCurrState = pNewState;
        mStateCond.broadcast();
    }
    //
    {
        Mutex::Autolock _lock(mObserverLock);
        for (ObserverList_t::iterator it = mObserverList.begin(); it != mObserverList.end(); it++)
        {
            (*it)->notify(eNewState);
        }
        mObserverList.clear();
    }
    return  OK;
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
registerOneShotObserver(IObserver* pObserver)
{
    if  ( pObserver == 0 ) {
        return  false;
    }
    //
    Mutex::Autolock _lock(mObserverLock);
    pObserver->notify(getCurrentState()->getEnum());
    mObserverList.push_back(pObserver);
    return  true;
}


/*******************************************************************************
 *
 ******************************************************************************/
void
StateManager::
unregisterObserver(IObserver* pObserver)
{
    Mutex::Autolock _lock(mObserverLock);
    //
    for (ObserverList_t::iterator it = mObserverList.begin(); it != mObserverList.end(); it++)
    {
        if  ( pObserver == (*it) )
        {
            MY_LOGD("(%p)", (*it));
            mObserverList.erase(it);
            break;
        }
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateObserver
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*******************************************************************************
 *
 ******************************************************************************/
IStateManager::
StateObserver::
StateObserver(IStateManager* pStateManager)
    : mpStateManager(pStateManager)
    , mLock()
    , mCond()
    , meCurrState(pStateManager->getCurrentState()->getEnum())
{
}


/*******************************************************************************
 *
 ******************************************************************************/
IStateManager::
StateObserver::
~StateObserver()
{
    mpStateManager->unregisterObserver(this);
}


/*******************************************************************************
 *
 ******************************************************************************/
void
IStateManager::
StateObserver::
notify(ENState eNewState)
{
    Mutex::Autolock _lock(mLock);
    meCurrState = eNewState;
    mCond.broadcast();
}


/*******************************************************************************
 *
 ******************************************************************************/
status_t
IStateManager::
StateObserver::
waitState(ENState eState, nsecs_t const timeout)
{
    status_t status = OK;
    //
    Mutex::Autolock _lock(mLock);
    //
    ENState eInitState = meCurrState;
    //
    if  ( eState != meCurrState )
    {
        MY_LOGW("<StateObserver> + current/waited=%d/%d, timeout(%lld)",
            meCurrState,
            eState,
            (long long)timeout
            );
/*        MY_LOGD_IF(
            1,
            "<StateObserver> + now/current/waited=%s/%s/%s, timeout(%lld)",
            mpStateManager->getCurrentState()->getName(),
            StateManager::getStateInst(meCurrState)->getName(),
            StateManager::getStateInst(eState)->getName(),
            timeout
        );*/
        switch  (timeout)
        {
        case 0:     //  not wait.
            status = TIMED_OUT;
            break;
        case -1:    //  wait without timeout.
            status = mCond.wait(mLock);
            break;
        default:    //  wait with a given timeout.
            status = mCond.waitRelative(mLock, timeout);
            break;
        }
        //
        if  ( eState != meCurrState )
        {
            status = FAILED_TRANSACTION;
        }
        //
        if  ( OK != status )
        {
            MY_LOGW(
                "<StateObserver> Timeout: now/current/waited/init=%d/%d/%d/%d, status[%s(%d)]",
                mpStateManager->getCurrentState()->getEnum(),
                meCurrState,
                eState,
                eInitState,
                ::strerror(-status), -status
            );
/*            MY_LOGW(
                "<StateObserver> Timeout: now/current/waited/init=%s/%s/%s/%s, status[%s(%d)]",
                mpStateManager->getCurrentState()->getName(),
                StateManager::getStateInst(meCurrState)->getName(),
                StateManager::getStateInst(eState)->getName(),
                StateManager::getStateInst(eInitState)->getName(),
                ::strerror(-status), -status
            );*/
        }
    }
    //
    return  status;
}

