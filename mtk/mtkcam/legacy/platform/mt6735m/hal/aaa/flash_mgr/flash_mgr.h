#ifndef __FLASH_MGR_H__
#define __FLASH_MGR_H__

#include "flash_mgr_type.h"
#include <Local.h>

#include <utils/threads.h>
using namespace android;

struct FlashStaData
{
    int aeRow;
    int aeCol;
    void* aeData;
    int awbRow;
    int awbCol;
    void* awbData;

};


    enum
    {
    e_SpModeNormal,
    e_SpModeUserExp,
    e_SpModeCalibration,
    };

class FlashMgr
    {
public:
    static FlashMgr& getInstance();
    static int getFlashSpMode();

    //flow
    int init(int senorDev, int sensorId);
    int uninit(int senorDev);
    int cameraPreviewStart(int senorDev);
    int cameraPreviewEnd(int senorDev);
    int videoPreviewStart(int senorDev);
    int videoPreviewEnd(int senorDev);
    int videoRecordingStart(int senorDev);
    int videoRecordingEnd(int senorDev);
    int capCheckAndFireFlash_Start(int senorDev);
    int capCheckAndFireFlash_End(int senorDev);
    int notifyAfEnter(int senorDev);
    int notifyAfExit(int senorDev);
    int doPfOneFrame(int senorDev, FlashExePara* para, FlashExeRep* rep);
    int doPreviewOneFrame(int senorDev);
    int endPrecapture(int senorDev);
    int getDebugInfo(int senorDev, FLASH_DEBUG_INFO_T* p);

    //control
    int setCamMode(int senorDev, int mode);
    int setDigZoom(int senorDev, int digx100);
    int setEvComp(int senorDev, int ind, float ev_step);
    int setAeFlashMode(int senorDev, int aeMode, int flashMOde);
    int getFlashMode(int senorDev);
    int getAfLampMode(int senorDev);
    int setMultiCapture(int senorDev, int bMulti);

    //rep
    int isFlashOnCapture(int senorDev);
    int isNeedFiringFlash(int senorDev);
    int egGetDutyRange(int senorDev, int* st, int* ed);
    int egGetStepRange(int senorDev, int* st, int* ed);
    int setCapPara(int senorDev);
    int setPfParaToAe(int senorDev);

    //strobeHw
    int turnOffFlashDevice(int senorDev);
    int setAFLampOnOff(int senorDev, int en);
    int isAFLampOn(int senorDev);
    int setTorchOnOff(int senorDev, int en);
    int setStrobeCallbackFunc(int senorDev, void (* pFunc)(MINT32 en));
    int hasFlashHw(int senorDev, int& hasHw);

    //misc
    int egSetMfDutyStep(int senorDev,int duty, int step);


    int cctCaliFixAwb2(int senorDev);


    int cctWriteNvram(int senorDev);
    int cctSetNvdataMeta(int senorDev, void* in, int inSize);
    int cctReadNvramToPcMeta(int senorDev, void* out, MUINT32* realOutSize);
    int cctFlashLightTest(int senorDev, void* in);

    int cctFlashEnable(int senorDev, int en);
    int cctGetFlashInfo(int senorDev, int* isOn);

    int cctSetSpModeCalibration(int senorDev);
    int cctSetSpModeNormal(int senorDev);

    int setTorchLevel(int level);
    int getTorchLevel(int& level);

    int setChildMain(void* child);
    int setChildSub(void* child);
    int setChildMain2(void* child);


    int setManualFlash(int senorDev, int duty, int dutyLt);
    int clearManualFlash(int senorDev);

    int setCaptureFlashOnOff(int senorDev, int en);
    int updateOneFrame(int senorDev);
    int getFlashState(int senorDev);






private:
    FlashMgr();
    int mTorchLevel;
    mutable Mutex mLock;
    void* mChildMain;
    void* mChildSub;
    void* mChildMain2;



};
#endif //#ifndef __FLASH_MGR_H__
