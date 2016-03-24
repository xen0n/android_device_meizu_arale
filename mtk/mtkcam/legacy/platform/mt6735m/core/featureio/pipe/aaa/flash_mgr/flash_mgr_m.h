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

#ifndef __FLASH_MGR_M_H__
#define __FLASH_MGR_M_H__


#include <flash_feature.h>
#include <flash_param.h>
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
//#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <utils/threads.h>
using namespace android;
//using namespace NSIoPipe;
//using namespace NSCamIOPipe;





#include "flash_mgr_type.h"


/** \brief the enum of error for flash public API
*
*/
typedef enum
{
    FL_ERR_FlashModeNotSupport=-100,
    FL_ERR_AFLampModeNotSupport=-100,

    FL_ERR_SetLevelFail=-101,


    FL_ERR_CCT_INPUT_SIZE_WRONG = -10001,
    FL_ERR_CCT_OUTPUT_SIZE_WRONG = -10002,
    FL_ERR_CCT_FILE_NOT_EXIST = -10003,



}FlashMgrEnum;



typedef struct
{
    float digRatio;
    int aeScene;
    int flash1Mode;
    int isFireFlashChecked;
    int vBat; //mv
    int flickerMode;
    float evComp;
    int isBurst;




}FlashCapInfo;


typedef struct
{
    int capIsFlash;
    int capIso;
    int capAfeGain;
    int capIspGain;
    int capExp;
    int capDuty;
    int capDutyLT;
    int dutyNum;
    int dutyNumLT;

    int capStrobeEndTime;
    int capStrobeStartTime;
    int capStartTime;
    int capEndTime;
    int capSetTimeOutTime;
    int capSetTimeOutTimeLT;
    int capIsTimeOut;




    int pfStartTime;
    int pfRunTime;
    int pfEndTime;
    int pfIteration;
    int pfRunFrms;
    int pfRunStartFrm;

    int isBatLow;

    int errCnt;
    int err1;
    int err2;
    int err3;
    int errTime1;
    int errTime2;
    int errTime3;



}FlashCapRep;





struct FlashIndConvPara
{
    int ind;
    int duty;
    int dutyLt;
    int dutyNum;
    int dutyNumLt;
    int isDual;
};

/** \brief FlashMgr: the class is mainly for handling flash/strobe related tasks.
*
*/
class FlashMgrM
{
public:


    enum
    {
        e_NonePreview,
        e_VideoPreview,
        e_VideoRecording,
        e_CapturePreview,
        e_Capture,
    };
    enum
    {
        e_Flicker50,
        e_Flicker60,
        e_FlickerUnknown,
    };



    enum
    {
        e_StrobeOff,
        e_StrobeTorch,
        e_StrobeAF,
        e_StrobeMf,
        e_StrobePf,
    };

    struct FlasExpParaEx
{
    int exp;
    int afeGain;
    int ispGain;
};

    static int getFlashSpMode();
    int setMultiCapture(int bMulti);
    //======================
    //cct related function
    //======================

    /** \brief call for testing hardware compentnent. when calling the function, the flash will be turn on and turn off for a while.
    *
    */
    int cctFlashLightTest(void* duty_duration);

    /** \brief get the flash current status (on/off).
    *
    */
    int cctGetFlashInfo(int* isOn);

    int cctSetSpModeCalibration();
    int cctSetSpModeNormal();
    int cctSetSpModeQuickCalibration();
    int cctSetSpModeQuickCalibration2();
    int cctGetQuickCalibrationResult();
    int setQuick2CalibrationExp(int exp, int afe, int isp);



    /** \brief call for set flash enable/disable
    *
    */
    int cctFlashEnable(int en);


    int cctCaliFixAwb2();






    //ACDK_CCT_OP_STROBE_WRITE_NVRAM,    //6
    /** \brief write buf to nvram
    *
    */
    int cctWriteNvram();








  //20
  /** \brief for cct tool used only. Read nvram and change to ACDK_STROBE_STRUCT format.
    *
    */
  int cctReadNvramToPcMeta(void* out, MUINT32* realOutSize);
  //21
  /** \brief for cct tool used only. change from ACDK_STROBE_STRUCT format and save to buf.
    *
    */
  int cctSetNvdataMeta(void* in, int inSize);











    /** \brief not used.
    *
    */
//    void cctGetCheckParaString(int* packetNO, int* isEnd, int* bufLength, unsigned char* buf); //buf: max 1024
    //---------------------------

    /** \brief do ratio calibration during precapture
    *
    */
    int cctCalibration(FlashExePara* para, FlashExeRep* rep);
    int cctCalibrationQuick(FlashExePara* para, FlashExeRep* rep);
    int cctCalibrationQuick2(FlashExePara* para, FlashExeRep* rep);


    int setManualFlash(int duty, int dutyLt);
    int clearManualFlash();



public:
    //======================
    //engineer mode related function
    //======================


    //int egnSetPfIndex(int duty, int step);
    //int egnSetMfIndex(int duty, int step);

    int egSetMfDuty(int duty);
    int egGetDutyRange(int* st, int* ed);









public:
    //======================
    //normal used function
    //======================
    int hasFlashHw(int& hasHw);

    /** \brief constructor
    *
    */
    //FlashMgrM();

    FlashMgrM(int sensorDev);

    /** \brief deconstructor
    *
    */
    ~FlashMgrM();

    /** \brief get the handle of FlashMgr (singleton)
    *
    */
    static FlashMgrM* getInstance(int sensorDev);

    /** \brief Before use the object, the object should be init.
    *   @return FlashMgr handle
    */

    int init(int sensorId);
    int initTemp();
    int initSub(int sensorId, int isTemp);


    /** \brief uninit, when exit camera. or in the case, before the FlashMgr init again (with different init parameters).
    *   @return error code : (0: success)
    */
    int uninit();

    int create();
    int destroy();

    /** \brief test if this capture should fire flash or not. call this function before preflash.
    *   @return 1: should fire flash. 0: should NOT fire flash.
    */
    int isNeedFiringFlash();

    /** \brief test if this capture is with or without flash.
    *   @return 1: flash firing during capture. 0: flash not firing during capture.
    */
    int isFlashOnCapture();


    /** \brief get flash mode
    *   @return flash mode AE_STROBE_T in alps\mediatek\custom\mt6582\hal\inc\aaa\flash_feature.h
    *   LIB3A_FLASH_MODE_T alps\mediatek\custom\mt6582\hal\inc\aaa\flash_feature.h
    */
    int getFlashMode();

    /** \brief set flash mode
    *   @return error code : (0: success)
    */
    int setFlashMode(int mode);
    int setAeFlashMode(int aeMode, int flashMode);


    /** \brief set cam mode
    *   @return error code : (0: success)
    */
    int setCamMode(int mode);








    /** \brief set digital zoom
    *   @param in digx100: digital zoom ratio*100
    *   @return error code : (0: success)
    */
    int setDigZoom(int digx100);

    /** \brief set ev compensation value
    *   @param in ind: index of ev comp
    *   @param in ev_step: one step of space of index of ev comp. ev comp value = ind * ev_step
    *   @return error code : (0: success)
    */
    int setEvComp(int ind, float ev_step);

    /** \brief befere capture, the fuction should call to start firing flash if needed.
    *   @return error code : (0: success)
    */
    int capCheckAndFireFlash_Start();

    /** \brief after capture, the fuction should call to start firing flash if needed.
    *   @return error code : (0: success)
    */
    int capCheckAndFireFlash_End();

    /** \brief at any time, call this function to turn of flash led.
    *   @return error code : (0: success)
    */
    int turnOffFlashDevice();

    /** \brief get debug information.
    *   @return error code : (0: success)
    */
    int getDebugInfo(FLASH_DEBUG_INFO_T* p);

    //void setFlashOnOff(int en);

    /** \brief turn on/off af lamp.
    *   @param in en : 1 on. 0 off
    */
    int setAFLampOnOff(int en);

//    int setAFLampOffSch(int ms);

    /** \brief check if AF lamp is on/off.
    *   @return error code : (0: success)
    */
    int isAFLampOn();

    /** \brief process one frame of preflash stage
    *   @return error code : (0: success)
    */
    int doPfOneFrame(FlashExePara* para, FlashExeRep* rep);

    int doPfOneFrameNormal(FlashExePara* para, FlashExeRep* rep);

    int getFlashStyle();



    /** \brief the function should be called after preflash is end.
    *   @return error code : (0: success)
    */
    int endPrecapture();



    /** \brief the function should be called for capture preview end.
    *   @return error code : (0: success)
    */
    int cameraPreviewEnd();

    /** \brief the function should be called for capture preview start.
    *   @return error code : (0: success)
    */
    int cameraPreviewStart();

    /** \brief the function should be called for video preview start.
    *   @return error code : (0: success)
    */
    int videoPreviewStart();

    /** \brief the function should be called for video preview end.
    *   @return error code : (0: success)
    */
    int videoPreviewEnd();

    /** \brief the function should be called for video recording start.
    *   @return error code : (0: success)
    */
    int videoRecordingStart();

    /** \brief the function should be called for video recording end.
    *   @return error code : (0: success)
    */
    int videoRecordingEnd();

    /** \brief get project parameters of flash (at auto mode)
    *   @return flash project parameters
    */
    FLASH_PROJECT_PARA& getAutoProjectPara();

    /** \brief get project parameters of flash (by ae mode)
    *   @param aeMode: ae mode
    *   @return flash project parameters
    */
    FLASH_PROJECT_PARA& getFlashProjectPara(int aeMode, int isisForceFlash);





    int setStrobeCallbackFunc(void (* pFunc)(MINT32 en));




    int setTorchOnOff(int en);


    int notifyAfEnter();
    int notifyAfExit();





    int setCapPara();
    int setPfParaToAe();

    //void hwSetFlashOn(int sid, int reTrig=1);
    void setFlashOn(int duty, int timeOut, int duty2, int timeOut2, int reTrig=1);

    void hwSetFlashOff();

    void genCapInfo(FlashExePara* in);
    //void genCapInfo();

    void setTorchLevel(int level);

private:

    void (* g_onOffCallBackFunc)(MINT32 en);


    int isNeedWaitCooling(int curMs, int* ms);
    int start(FlashExePara* in);
    //int start();
    int run(FlashExePara* para, FlashExeRep* rep);
    int end();

    void hw_convert3ASta(FlashAlgStaData* staData, void* staBuf, int winX, int winY);


    void hw_setFlashProfile(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara, NVRAM_CAMERA_STROBE_STRUCT* pNvram);
    void hw_setPreference(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara);
    void hw_setCapPline(FLASH_PROJECT_PARA* pPrjPara, FlashAlg* pStrobeAlg);
    void hw_setPfPline(FlashAlg* pStrobeAlg);
    void hw_getAEExpPara(FlashAlgExpPara* aePara);
    void hw_speedUpExpPara(FlashAlgExpPara* expPara, int maxAfe);
    void hw_gainToIso(int afe, int isp, int* iso);


    int nvGetBuf(NVRAM_CAMERA_STROBE_STRUCT*& buf, int isForceRead=0);
    int nvWrite();
    int nv3AWrite();
    int nvReadDefault();


    int getFlashModeStyle(int sensorType, int flashMode);


    int cctSetAaoInitPara();


    void turnOnAf();
    void turnOnTorch();
    void turnOnPf();

    void hw_turnOffFlash();
    void hw_setExpPara(FlashAlgExpPara* expPara, int sensorType, FLASH_PROJECT_PARA* pPrjPara);

    void hw_isoToGain(int iso, int* afe, int* isp);
    void hw_capIsoToGain(int iso, int* afe, int* isp);
    void hw_setCapExpPara(FlashAlgExpPara* expPara);
    void logProjectPara(FLASH_PROJECT_PARA* pp);
    void debugCnt();

    int ind2duty(FlashIndConvPara& para);
    int duty2ind(FlashIndConvPara& para);



private:

    inline void setDebugTag(FLASH_DEBUG_INFO_T &a_rFlashInfo, MINT32 a_i4ID, MINT32 a_i4Value)
    {
        a_rFlashInfo.Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_FLASH_MODULE_ID, a_i4ID, 0);
        a_rFlashInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
    }


    void addErr(int err);





private:

    //mutex
    mutable Mutex mLock;
    //ui & control
    int m_sensorDev;
    int mSensorId;
    int m_pfFrameCount;
    float m_evComp;
    int mFlash1Mode;
    int mCamMode;
    int g_previewMode;
    float m_digRatio;
    int m_isAfState;
    int mIsNeedFiredFlashChecked;
    int mIsMultiCapture;
    int mTorchLevel;
    //data
    NVRAM_CAMERA_STROBE_STRUCT* mpStrobeNvbuf;
    FlashCapInfo mCapInfo;
    FlashCapRep mCapRep;
    FlashAlg* mpFlashAlg;
    //flash
    StrobeDrv* g_pStrobe;
    StrobeDrv* g_pStrobe2;
    //report
    int m_flashOnPrecapture;
    int m_bRunPreFlash;
    int m_iteration;
    int g_flashQuickCalibrationResult;
    //debug & misc
    int mDebugSceneCnt;
    FlashAlgExpPara g_expPara;
    FlasExpParaEx g_pfExpPara;

    //state data
    int mCoolingStartTime;
    int mCoolingWaitTime;
    int mFlashState;
    int mFlashPreSt;
    int mPreRunFrm;
    int mFlashHwState;
    int mRunCycleFrame;
    //flashState

    int mStrobeState;
    int mIsInited;






};



#endif  //#define __FLASH_MGR_M_H__
