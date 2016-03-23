#define LOG_TAG "RefBaseDump"

#if 0
#define RBD_LOGV(x, ...) ALOGV(x, ##__VA_ARGS__)
#define RBD_LOGD(x, ...) ALOGD(x, ##__VA_ARGS__)
#define RBD_LOGI(x, ...) ALOGI(x, ##__VA_ARGS__)
#define RBD_LOGW(x, ...) ALOGW(x, ##__VA_ARGS__)
#define RBD_LOGE(x, ...) ALOGE(x, ##__VA_ARGS__)
#else
#define RBD_LOGV(x, ...)
#define RBD_LOGD(x, ...)
#define RBD_LOGI(x, ...)
#define RBD_LOGW(x, ...)
#define RBD_LOGE(x, ...)
#endif

#include <cutils/log.h>
#include <cutils/process_name.h>
#include <cutils/properties.h>
#include <RefBaseDump.h>

namespace android {

//-------------------------------------------------------------------------
// RefBaseDump
//-------------------------------------------------------------------------

RefBaseDump::RefBaseDump(RefBaseMonitor* pMonitor)
    : mMonitor(pMonitor) {
}

RefBaseDump::~RefBaseDump() {
}

status_t RefBaseDump::kickDump(String8& result, const char* prefix) {
    return mMonitor->dump(result);
}

//-------------------------------------------------------------------------
// RefBaseMonitor
//-------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(RefBaseMonitor)

RefBaseMonitor::RefBaseMonitor() {
    char value[PROPERTY_VALUE_MAX];
    RBD_LOGI("RefBaseMonitor ctor - %p", this);
    getProcessName();
    property_get("debug.rb.dump", value, "Mary had a little lamb");
    mIsTracking = (-1 != mProcessName.find(value));
    mDump = new RefBaseDump(this);
    DumpTunnelHelper::getInstance().regDump(mDump, String8::format("RB-%p", this));
}

RefBaseMonitor::~RefBaseMonitor() {
    RBD_LOGI("RefBaseMonitor dtor - %p", this);
    DumpTunnelHelper::getInstance().unregDump(String8::format("RB-%p", this));
}

status_t RefBaseMonitor::monitor(RefBase *pRb) {
    Mutex::Autolock _l(mMutex);
    if (mIsTracking) {
        pRb->trackMe(true, false);
    }
    RbList.add(pRb, 0);
    return NO_ERROR;
}

status_t RefBaseMonitor::unmonitor(RefBase *pRb) {
    Mutex::Autolock _l(mMutex);
    RbList.removeItem(pRb);
    return NO_ERROR;
}

status_t RefBaseMonitor::dump(String8& result) {
    int listSz;
    RefBase *pRb;
    Mutex::Autolock _l(mMutex);
    RBD_LOGI("RefBaseMonitor Dump - %p", this);
    listSz = RbList.size();
    result.appendFormat("\t  [%8p]    RefCnt   %s", this, mProcessName.string());
    result.append(mIsTracking ? " <- tracking\n" : "\n");
    result.append("\t  -----------------------\n");

    for (int i = 0; i < listSz; i++) {
        pRb = RbList.keyAt(i);
        if (mIsTracking) {
            pRb->printRefs();
        }
        result.appendFormat("\t   %2d) %8p %4d\n", i, pRb, pRb->getStrongCount());
    }
    result.append("\t*****************************************************\n");
    return NO_ERROR;
}

status_t RefBaseMonitor::getProcessName() {
    int pid = getpid();
    FILE *fp = fopen(String8::format("/proc/%d/cmdline", pid), "r");
    if (NULL != fp) {
        const size_t size = 64;
        char proc_name[size];
        fgets(proc_name, size, fp);
        fclose(fp);
        mProcessName = proc_name;
    } else {
        mProcessName = "unknownProcess";
    }
    return NO_ERROR;
}

}
