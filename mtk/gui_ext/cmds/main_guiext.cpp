#define LOG_TAG "GuiExt"

#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include <binder/BinderService.h>
#include <GuiExtService.h>

using namespace android;

int main(int /*argc*/, char** /*argv*/)
{
    ALOGI("GuiExt service start...");

    GuiExtService::publishAndJoinThreadPool(true);
    // When AAL is launched in its own process, limit the number of
    // binder threads to 4.
    ProcessState::self()->setThreadPoolMaxThreadCount(4);

    ALOGD("GuiExt service exit...");
    return 0;
}
