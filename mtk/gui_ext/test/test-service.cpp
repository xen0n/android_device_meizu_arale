#define LOG_TAG "GuiExt-Test"

#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#ifdef USE_PTHREAD
#include <pthread.h> 
#else
#include <utils/AndroidThreads.h> 
#endif
#include <binder/BinderService.h>
#include <AALService.h>


void printids(const char *s)
{ 
#ifdef USE_PTHREAD    
    pid_t pid; 
    pthread_t tid; 

    pid = getpid(); 
    tid = pthread_self(); 
    printf("%s pid %u tid %u (0x%x)\n",s,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid); 
#else    
    printf("%s pid %u tid %u \n",s,(unsigned int)getpid(),(unsigned int)android::getThreadId()); 
#endif
} 

int aal_service_fun(void *arg)
{ 
    printids("new thread:"); 
    android::AALService::publishAndJoinThreadPool();

    return 0; 
} 

int main(int argc, char** argv)
{
    ALOGD("AAL service start...");
    
#ifdef USE_PTHREAD
    pthread_t aal_tid; 
    int err;     
    err = pthread_create(&aal_tid, NULL, aal_service_fun, NULL); 
    if(err != 0){ 
        printf("can't create thread: %s\n", strerror(err)); 
        return -1; 
    } 
#else
    if (!android::createThreadEtc(aal_service_fun, NULL, "AALService")) {
        printf("can't create thread\n"); 
        return -1; 
    }         
#endif        
    printids("main thread:"); 

    sleep(10000);


    ALOGD("AAL service finish...");
    return 0;    
}
