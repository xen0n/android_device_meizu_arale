#pragma once


#ifdef WIN32
#include "win_test.h"
#include "win_test_cpp.h"

#else
#include <utils/threads.h>
using namespace android;

#endif

class StrobeGlobalDriver
{
public:
    static StrobeGlobalDriver* getInstance();
    int init(int sensorDev, int strobeId);
    int uninit(int sensorDev, int strobeId);

    int openkd();
    int closekd();

    int sendCommand(int cmd, int sensorDev, int strobeId, int arg);
    int sendCommand_nolock(int cmd, int sensorDev, int strobeId, int arg);

    int sendCommandRet(int cmd, int sensorDev, int strobeId, int* arg);
    int sendCommandRet_nolock(int cmd, int sensorDev, int strobeId, int* arg);
    int openkd_nolock();
    int closekd_nolock();



private:
    int mStrobeHandle;
    int mUsers;
    mutable Mutex mLock;
    StrobeGlobalDriver();


};
