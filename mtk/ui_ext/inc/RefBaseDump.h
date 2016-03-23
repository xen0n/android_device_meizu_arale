#ifndef ANDROID_REFBASE_DUMP_H__
#define ANDROID_REFBASE_DUMP_H__

#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <utils/Singleton.h>
#include <IDumpTunnel.h>

namespace android
{
class RefBaseMonitor;

//-------------------------------------------------------------------------
// RefBaseDump
//-------------------------------------------------------------------------
class RefBaseDump : public BnDumpTunnel {

public:
    RefBaseDump(RefBaseMonitor*);
    virtual ~RefBaseDump();

    // IDumpTunnel interface
    virtual status_t kickDump(String8& /*result*/, const char* /*prefix*/);

private:
    RefBaseMonitor *mMonitor;
};

//-------------------------------------------------------------------------
// RefBaseMonitor
//-------------------------------------------------------------------------
class RefBaseMonitor : public Singleton<RefBaseMonitor> {
public:
    RefBaseMonitor();
    virtual ~RefBaseMonitor();

    // add refbase to the monitored list
    status_t monitor(RefBase* );

    // remove refbase from the monitored list
    status_t unmonitor(RefBase* );

    // dump all elements in the monitored list and call printRefs if mIsTracking equals 1
    status_t dump(String8& result);

private:
    status_t getProcessName();

    // if trackMe needed
    bool mIsTracking;

    String8 mProcessName;

    sp<RefBaseDump> mDump;

    // the list where monitored refbase objects are saved
    KeyedVector<RefBase*,int> RbList;

    mutable Mutex mMutex;;
};

};
#endif
