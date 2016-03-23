/* //device/system/rild/rild.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <telephony/ril.h>
#define LOG_TAG "C2K_RILD"
#include <utils/Log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <sys/capability.h>
#include <linux/prctl.h>

#include <private/android_filesystem_config.h>
#include "hardware/qemu_pipe.h"
#include "hardware/ccci_intf.h"

#define LIB_PATH_PROPERTY   "rild.libpath"
#define LIB_ARGS_PROPERTY   "rild.libargs"
#define MAX_LIB_ARGS        16

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s -l <ril impl library> [-- <args for impl library>]\n", argv0);
    exit(-1);
}

extern char rild[MAX_SOCKET_NAME_LENGTH];

extern void RIL_register (const RIL_RadioFunctions *callbacks);

extern void RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response,
        size_t responselen);

#if defined(ANDROID_MULTI_SIM)
extern void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id);
#else
extern void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen);
#endif

extern void RIL_requestTimedCallback(RIL_TimedCallback callback, void *param,
        const struct timeval *relativeTime);

// For multi channel support
extern void RIL_requestProxyTimedCallback (RIL_TimedCallback callback,
                               void *param, const struct timeval *relativeTime, int proxyId);
extern RILChannelId RIL_queryMyChannelId(RIL_Token t);
extern int RIL_queryMyProxyIdByThread();

/*
* to notifiy the md state chaned
*/
extern void RIL_MDStateChange(RIL_MDState state);

static struct RIL_Env s_rilEnv = {
    RIL_onRequestComplete,
    RIL_onUnsolicitedResponse,
    RIL_requestTimedCallback
    // For multi channel support
    ,RIL_requestProxyTimedCallback
    ,RIL_queryMyChannelId
    ,RIL_queryMyProxyIdByThread
};

extern void RIL_startEventLoop();

static int make_argv(char * args, char ** argv)
{
    // Note: reserve argv[0]
    int count = 1;
    char * tok;
    char * s = args;

    while ((tok = strtok(s, " \0"))) {
        argv[count] = tok;
        s = NULL;
        count++;
    }
    return count;
}

/*
 * switchUser - Switches UID to radio, preserving CAP_NET_ADMIN capabilities.
 * Our group, cache, was set by init.
 */
void switchUser() {
    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    setuid(AID_RADIO);

    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap;
    header.version = _LINUX_CAPABILITY_VERSION;
    header.pid = 0;
    cap.effective = cap.permitted = (1 << CAP_NET_ADMIN) | (1 << CAP_NET_RAW) | (1 << CAP_SYS_ADMIN);
    cap.inheritable = 0;
    capset(&header, &cap);
}

int main(int argc, char **argv)
{
    const char * rilLibPath = NULL;
    char **rilArgv;
    void *dlHandle;
    const RIL_RadioFunctions *(*rilInit)(const struct RIL_Env *, int, char **);
    const RIL_RadioFunctions *funcs;
    char libPath[PROPERTY_VALUE_MAX];
    unsigned char hasLibArgs = 0;

    int i;

    LOGD("viarild suffix Version: %s", VIA_SUFFIX_VERSION);

    const char *clientId = NULL;
    RLOGD("**RIL Daemon Started**");
    RLOGD("**RILd param count=%d**", argc);

    umask(S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
    for (i = 1; i < argc ;) {
        if (0 == strcmp(argv[i], "-l") && (argc - i > 1)) {
            rilLibPath = argv[i + 1];
            i += 2;
        } else if (0 == strcmp(argv[i], "--")) {
            i++;
            hasLibArgs = 1;
            break;
        } else if (0 == strcmp(argv[i], "-c") &&  (argc - i > 1)) {
                    clientId = argv[i+1];
                    i += 2;
        } else {
            usage(argv[0]);
        }
    }

    if (clientId == NULL) {
        clientId = "0";
    } else if (atoi(clientId) >= MAX_RILDS) {
        RLOGE("Max Number of rild's supported is: %d", MAX_RILDS);
        exit(0);
    }
    if (strncmp(clientId, "0", MAX_CLIENT_ID_LENGTH)) {
        RIL_setRilSocketName(strncat(rild, clientId, MAX_SOCKET_NAME_LENGTH));
    }

    LOGD("Android rild start");
    add_sig_handler();

    if (rilLibPath == NULL) {
        if ( 0 == property_get(LIB_PATH_PROPERTY, libPath, NULL)) {
            // No lib sepcified on the command line, and nothing set in props.
            // Assume "no-ril" case.
            LOGD("No vendor so");
            goto done;
        } else {
            rilLibPath = libPath;
        }
    }

    /* special override when in the emulator */
#if 1
    {
        static char*  arg_overrides[3];
        static char   arg_device[32];
        int           done = 0;

#define  REFERENCE_RIL_PATH  "/system/lib/libreference-ril.so"

        /* first, read /proc/cmdline into memory */
        char          buffer[1024], *p, *q;
        int           len;
        int           fd = open("/proc/cmdline",O_RDONLY);

        if (fd < 0) {
            LOGD("could not open /proc/cmdline:%s", strerror(errno));
            goto OpenLib;
        }

        do {
            len = read(fd,buffer,sizeof(buffer)); }
        while (len == -1 && errno == EINTR);

        if (len < 0) {
            LOGD("could not read /proc/cmdline:%s", strerror(errno));
            close(fd);
            goto OpenLib;
        }
        close(fd);

        if (strstr(buffer, "android.qemud=") != NULL)
        {
            /* the qemud daemon is launched after rild, so
            * give it some time to create its GSM socket
            */
            int  tries = 5;
#define  QEMUD_SOCKET_NAME    "qemud"

            while (1) {
                int  fd;

                sleep(1);

                fd = qemu_pipe_open("qemud:gsm");
                if (fd < 0) {
                    fd = socket_local_client(
                                QEMUD_SOCKET_NAME,
                                ANDROID_SOCKET_NAMESPACE_RESERVED,
                                SOCK_STREAM );
                }
                if (fd >= 0) {
                    close(fd);
                    snprintf( arg_device, sizeof(arg_device), "%s/%s",
                                ANDROID_SOCKET_DIR, QEMUD_SOCKET_NAME );

                    arg_overrides[1] = "-s";
                    arg_overrides[2] = arg_device;
                    done = 1;
                    break;
                }
                LOGD("could not connect to %s socket: %s",
                    QEMUD_SOCKET_NAME, strerror(errno));
                if (--tries == 0)
                    break;
            }
            if (!done) {
                LOGE("could not connect to %s socket (giving up): %s",
                    QEMUD_SOCKET_NAME, strerror(errno));
                while(1)
                    sleep(0x00ffffff);
            }
        }

        /* otherwise, try to see if we passed a device name from the kernel */
        if (!done) do {
#define  KERNEL_OPTION  "android.ril="
#define  DEV_PREFIX     "/dev/"

            p = strstr( buffer, KERNEL_OPTION );
            if (p == NULL)
                break;

            p += sizeof(KERNEL_OPTION)-1;
            q  = strpbrk( p, " \t\n\r" );
            if (q != NULL)
                *q = 0;

            snprintf( arg_device, sizeof(arg_device), DEV_PREFIX "%s", p );
            arg_device[sizeof(arg_device)-1] = 0;
            arg_overrides[1] = "-d";
            arg_overrides[2] = arg_device;
            done = 1;

        } while (0);

        if (done) {
            argv = arg_overrides;
            argc = 3;
            i    = 1;
            hasLibArgs = 1;
            rilLibPath = REFERENCE_RIL_PATH;

            LOGD("overriding with %s %s", arg_overrides[1], arg_overrides[2]);
        }
    }
OpenLib:
#endif
    switchUser();

    dlHandle = dlopen(rilLibPath, RTLD_NOW);

    if (dlHandle == NULL) {
        LOGE(stderr, "dlopen failed: %s\n", dlerror());
        exit(-1);
    }

    RIL_startEventLoop();

    rilInit = (const RIL_RadioFunctions *(*)(const struct RIL_Env *, int, char **))dlsym(dlHandle, "RIL_Init");

    if (rilInit == NULL) {
        LOGE(stderr, "RIL_Init not defined or exported in %s\n", rilLibPath);
        exit(-1);
    }

    if (hasLibArgs) {
        rilArgv = argv + i - 1;
        argc = argc -i + 1;
    } else {
        static char * newArgv[MAX_LIB_ARGS];
        static char args[PROPERTY_VALUE_MAX];
        rilArgv = newArgv;
        property_get(LIB_ARGS_PROPERTY, args, "");
        argc = make_argv(args, rilArgv);
    }

    rilArgv[argc++] = "-c";
    rilArgv[argc++] = clientId;
    RLOGD("RIL_Init argc = %d clientId = %s", argc, rilArgv[argc-1]);

    // Make sure there's a reasonable argv[0]
    rilArgv[0] = argv[0];

    funcs = rilInit(&s_rilEnv, argc, rilArgv);

    RIL_register(funcs);

done:

    while(1) {
        // sleep(UINT32_MAX) seems to return immediately on bionic
        sleep(0x00ffffff);
    }
}

int isMtkInternalSupport() {
    int internalProject;
#ifndef __PRODUCTION_RELEASE__
    internalProject = 1;
#else
    internalProject = 0;
#endif
    RLOGI("isMtkInternalSupport: %d", internalProject);
    return internalProject;
}

int isUserLoad() {
    int isUserLoad = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.build.type", property_value, "");
    isUserLoad = (strcmp("user", property_value) == 0);
    RLOGI("isUserLoad: %d", isUserLoad);
    return isUserLoad;
}

int isEngLoad() {
    int isEngLoad = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.build.type", property_value, "");
    isEngLoad = (strcmp("eng", property_value) == 0);
    RLOGI("isEngLoad: %d", isEngLoad);
    return isEngLoad;
}

void signal_handler(int param)
{
    RLOGD("signal_handler, signal_no=%d", param);
    switch (param)
    {
    RLOGD("signal_no=%d", param);
    case SIGSEGV:
    case SIGABRT:
        LOGD("C2KReset");
        property_set("ril.cdma.report.case", "1");
        property_set("ril.mux.report.case", "2");

#ifdef MTK_ECCCI_C2K
        char ret[32];
        int fd = -1;
        snprintf(ret, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS3));
        fd = open(ret, O_RDWR);
        if(fd >= 0) {
            ioctl(fd, CCCI_IOC_MD_RESET);
            close(fd);
        } else {
            LOGE("open ccci_get_node_name USR_RILD_IOCTL failed");
        }
#else /* MTK_ECCCI_C2K */
        C2KReset();
#endif
        break;
    default:
        exit(0);
        break;
    }
}

void signal_treatment(int param)
{
    RLOGD("signal_treatment, signal_no=%d", param);
    switch (param)
    {
    case SIGSEGV:
    case SIGABRT:	
        signal(param, SIG_DFL);
        raise(param);
        break;
    case SIGUSR2:
        RIL_MDStateChange(MD_STATE_ASSERT);
        break;
    default:
        exit(0);
        break;
    }
}
/// M: Fix rild restart failed after NE in non-eng version.
/// When RILD NE happened, Statusd can not get its exit state using waitpid, so rild restart failed.
/// we need register signal handler function and trigger send signal again as SIG_DFL handler in it.
/// if other signal occoured in NE,please add it here.
void add_sig_handler() {
    RLOGD("add signal handler");
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR2, signal_treatment);
    if (!isMtkInternalSupport() && isUserLoad()) {
        signal(SIGSEGV, signal_handler);
        signal(SIGABRT, signal_handler);
    } else if (!isEngLoad()){
        struct sigaction actions = {0};
        sigemptyset(&actions.sa_mask);
        actions.sa_handler = signal_treatment;
        actions.sa_flags = 0;
        sigaction(SIGSEGV, &actions, NULL);
        sigaction(SIGABRT, &actions, NULL);
    }
}
