#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include "cutils/log.h"//"cutils/log.h"
#include <cutils/sockets.h>
#include <dlfcn.h>
#define SHARE_RIL_PATH "/system/lib/libshareril.so"

//#ifdef ANDROID_JB
#include <c2k_log.h>
//#endif

typedef int (*cw_callback2)(char**);
typedef int (*cw_callback13)(const char*, char**);
typedef int (*cw_callback33)(char**, char**);

typedef int (*cw_callback4)(const char*, char**);

int main(int argc, char *argv[]) {
	char* szIMSI = NULL;
	const char *pstrrandu = "510882";
	char* pstrathoru = NULL;
        void *cw_handle2 = NULL; /*the handle of loaded libshareril.so*/
        cw_callback2 cw_cb2 = NULL;
        void *cw_handle13 = NULL; /*the handle of loaded librpcril.so*/
        cw_callback13 cw_cb13 = NULL;
        int iRtn = 0;

        char* uimauth = NULL;
        char* key = NULL;
        char* plcm = NULL;
        void* cw_handle33 = NULL;
        cw_callback33 cw_cb33 = NULL;
        
        char* chapID = "12";
        char* chapChallenge = "510882";
        char* chapStr ="12510882";
        char* chapResponse = NULL;
        void* cw_handle4 = NULL;
        cw_callback4 cw_cb4 = NULL;
        
        LOGD("Testcw: suffix Version: %s", VIA_SUFFIX_VERSION);
///////////////////IMSI//////////////////////////////
        LOGD("start aka_get_imsi");
        cw_handle2 = dlopen(SHARE_RIL_PATH, RTLD_NOW);
    	if(!cw_handle2){
        LOGE("load %s failed:%s\n",SHARE_RIL_PATH, dlerror());
        return -1;
        }
    	cw_cb2  = (cw_callback2)dlsym(cw_handle2,"aka_get_imsi");
    	if(!cw_cb2 ){
        dlclose(cw_handle2);
        LOGE("IMSI find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    	}
   	iRtn=cw_cb2(&szIMSI);
        LOGD("*******testcw aka_get_imsi  is %s,iRtn is %d\n",szIMSI,iRtn);
        if(cw_handle2){
	dlclose(cw_handle2);
    	}
/////////////////cave////////////////////////////
    
        LOGD("start cave_authentication");
    	cw_handle13 = dlopen(SHARE_RIL_PATH, RTLD_NOW);
    	if(!cw_handle13){
        LOGE("load %s failed:%s\n",SHARE_RIL_PATH, dlerror());
        return -1;
    	}
    	cw_cb13 = (cw_callback13)dlsym(cw_handle13,"cave_authentication");
    	if(!cw_cb13 ){
        dlclose(cw_handle13);
        LOGE("CAVE find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    	}
    	iRtn = cw_cb13(pstrrandu, &pstrathoru);
    	LOGD("********testcw cave_authentication  pstrrandu=%s, pstrathoru = %s ,iRtn=%d\n",pstrrandu,pstrathoru,iRtn);
    	if(cw_handle13){
	dlclose(cw_handle13);
        }

  ///////////////////UIMAUTH//////////////////////////////
        LOGD("start aka get uimauth");
        cw_handle2 = dlopen(SHARE_RIL_PATH, RTLD_NOW);
    	if(!cw_handle2){
        LOGE("load %s failed:%s\n",SHARE_RIL_PATH, dlerror());
        return -1;
        }
    	cw_cb2  = (cw_callback2)dlsym(cw_handle2,"aka_get_uimauth");
    	if(!cw_cb2 ){
        dlclose(cw_handle2);
        LOGE("UIMAUTH find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    	}
   	iRtn=cw_cb2(&uimauth);
        LOGD("*******testcw aka_get_uimauth  is %s,iRtn is %d\n",uimauth,iRtn);
        if(cw_handle2){
	dlclose(cw_handle2);
    	}

        /////////////////KEY PLCM////////////////////////////
    
        LOGD("start gen key plcm");
    	cw_handle33 = dlopen(SHARE_RIL_PATH, RTLD_NOW);
    	if(!cw_handle33){
        LOGE("load %s failed:%s\n",SHARE_RIL_PATH, dlerror());
        return -1;
    	}
    	cw_cb33 = (cw_callback33)dlsym(cw_handle33,"fmc_aka_gen_key_plcm");
    	if(!cw_cb33 ){
        dlclose(cw_handle33);
        LOGE("KEY PLCM find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    	}
    	iRtn = cw_cb33(&key, &plcm);
    	LOGD("********testcw fmc_aka_gen_key_plcm key=%s, plcm = %s ,iRtn=%d\n",key,plcm,iRtn);
    	if(cw_handle13){
	dlclose(cw_handle13);
        }

        /////////////////MD5////////////////////////////
    
        LOGD("start MD5");
    	cw_handle4 = dlopen(SHARE_RIL_PATH, RTLD_NOW);
    	if(!cw_handle4){
        LOGE("load %s failed:%s\n",SHARE_RIL_PATH, dlerror());
        return -1;
    	}
    	cw_cb4 = (cw_callback4)dlsym(cw_handle4,"md5_authentication");
    	if(!cw_cb4 ){
        dlclose(cw_handle4);
        LOGE("MD5 find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    	}
    	//iRtn = cw_cb4(chapID,chapChallenge,&chapResponse);
    	iRtn = cw_cb4(chapStr, &chapResponse);
    	LOGD("********testcw md5_authentication chapID = %s, chapChanllenge = %s ,chapResponse = %s ,iRtn=%d\n",chapID,chapChallenge,chapResponse,iRtn);
    	iRtn = cw_cb4("01510882", &chapResponse);
    	LOGD("********testcw md5_authentication chapID = 01, chapChanllenge = %s ,chapResponse = %s ,iRtn=%d\n",chapID,chapChallenge,chapResponse,iRtn);
    	if(cw_handle4){
	dlclose(cw_handle4);
        }
  	return 0;
}




