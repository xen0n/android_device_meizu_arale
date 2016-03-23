#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include "cutils/log.h"
#include <cutils/sockets.h>
//#include <cw.h>
#include <dlfcn.h>
//#define LOG_TAG                 "CWCLIENT-AT"
#define CW_PATH "/system/lib/libcw.so"

#include <c2k_log.h>


#define SUCCESS 1
#define FAILED  0
typedef int (*cw_callback1)();
typedef int (*cw_callback2)(char***);
typedef int (*cw_callback13)(char*, char***);
typedef int (*cw_callback33)(char***, char***);
typedef int (*cw_callback4)(char*, char*, char***);
typedef int (*cw_callback5)(char*, char*, char***, char***);
//int ReadIMSI(char ** szIMSI)
//int aka_get_imsi(char ** szIMSI)
int aka_get_imsi(char ** szIMSI)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback2 cw_cb = NULL;
    int iRtn = FAILED;

   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return FAILED;
    }
    cw_cb  = (cw_callback2)dlsym(cw_handle,"ReadIMSI");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return FAILED;
    }

    /*Send AT+VUSBETS*/
    iRtn=cw_cb(&szIMSI);
        LOGW("cwclient szIMSI &&& is %s, iRtn is %d \n",*szIMSI,iRtn);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
        dlclose(cw_handle);
    }
    return iRtn;
}

int aka_get_uimauth(char ** uimauth)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback1 cw_cb = NULL;
    int iRtn = FAILED;

   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return FAILED;
    }
    cw_cb  = (cw_callback1)dlsym(cw_handle,"EnumAuthAlgs");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return FAILED;
    }

    /*Send AT+VUSBETS*/
    iRtn = cw_cb();
    if(iRtn==1)
    {
        *uimauth="\"CAVE\"";
    }
    else if(iRtn==2)
    {
        *uimauth="\"MD5\"";
        iRtn=1;
    }
    else if(iRtn==3)
    {
        *uimauth= "\"CAVE\",\"MD5\"";//"CAVE & MD5 all support";
        iRtn=1;
    }
    else if(iRtn==0)
    {
        *uimauth="not support";
    }
    LOGD("CWCLIENT EnumAuthAlgsC iRtn = %d ,uimauth=%s \n", iRtn,*uimauth);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
        dlclose(cw_handle);
    }

    return SUCCESS;
}


int ListCardType()
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback1 cw_cb = NULL;
    int iRtn = 0;
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return -1;
    }
    cw_cb  = (cw_callback1)dlsym(cw_handle,"ListCardType");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    }

    /*Send AT+VUSBETS*/
    iRtn = cw_cb();
    LOGD("CWCLIENT ListCardType iRtn = %d \n", iRtn);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
        dlclose(cw_handle);
    }

    return iRtn;
}

//int RequestCave(const char *pstrrandu, char** pstrathoru)
int cave_authentication(const char *input, char** pstrathoru)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback1 cw_cb = NULL;
    int iRtn = FAILED;
    int i = 0;

    /* fill input to 3 bytes */
    char pstrrandu[24];
    int len = 6 - strlen(input);
    if (len < 0) {
        len = 0;
    }
    strcpy(pstrrandu + len, input);
    for (i = 0; i < len; i++) {
        pstrrandu[i] = '0';
    }

   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return FAILED;
    }
    cw_cb  = (cw_callback13)dlsym(cw_handle,"RequestCave");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return FAILED;
    }

    //LOGD("CWCLIENT before callback pstrrandu=%s, *pstrathoru = %s ,iRtn=%d\n",pstrrandu,*pstrathoru,iRtn);
    /*Send AT+VUSBETS*/
    iRtn = cw_cb(pstrrandu, &pstrathoru);
    //LOGD("CWCLIENT pstrrandu=%s, *pstrathoru = %s ,iRtn=%d\n",pstrrandu,*pstrathoru,iRtn);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
        dlclose(cw_handle);
    }
    if(iRtn==0) {
        return SUCCESS;
    }
    else{
        return FAILED;
    }
}


int GetUimid(char **pszMeid, char **pszUimid)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback1 cw_cb = NULL;
    int iRtn = 0;
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return -1;
    }
    cw_cb  = (cw_callback33)dlsym(cw_handle,"GetUimid");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    }

    /*Send AT+VUSBETS*/
    iRtn = cw_cb(&pszMeid, &pszUimid);
    LOGD("CWCLIENT GetUimid  pszMeid = %s, pszUimid=%s\n", *pszMeid,*pszUimid);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
	dlclose(cw_handle);
    }

    return iRtn;
}



int SSDConfirm(const char *pcStrAuthbs)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback1 cw_cb = NULL;
    int iRtn = 0;
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return -1;
    }
    cw_cb  = (cw_callback2)dlsym(cw_handle,"SSDConfirm");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    }

    /*Send AT+VUSBETS*/
    iRtn = cw_cb(pcStrAuthbs);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
	dlclose(cw_handle);
    }

    return iRtn;
}



int SSDUpdate(const char * pcStrRandssd, char **pszRandbs)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback1 cw_cb = NULL;
    int iRtn = 0;
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return -1;
    }
    cw_cb  = (cw_callback13)dlsym(cw_handle,"SSDUpdate");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return -1;
    }

    /*Send AT+VUSBETS*/
    iRtn = cw_cb(pcStrRandssd, &pszRandbs);
    LOGD("CWCLIENT  SSDUpdate pcStrRandssd = %s, pszRandbs=%s\n", pcStrRandssd,*pszRandbs);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
	dlclose(cw_handle);
    }

    return iRtn;
}

int fmc_aka_gen_key_plcm(char **pszSmekey, char **pszVpm)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback1 cw_cb = NULL;
    int iRtn = FAILED;
    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return FAILED;
    }
    cw_cb  = (cw_callback33)dlsym(cw_handle,"GetGeneratekey");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return FAILED;
    }

    /*Send AT+VUSBETS*/
    iRtn = cw_cb(&pszSmekey, &pszVpm);
    LOGD("CWCLIENT Generatekey pszSmekey=%s, pszVpm=%s\n",*pszSmekey,*pszVpm);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
	dlclose(cw_handle);
    }

    return iRtn;
}


#define CHAP_ID_LEN 2
int md5_authentication(const char *inputStr, char **pszResponse)
{
    void *cw_handle = NULL; /*the handle of loaded librpcril.so*/
    cw_callback4 cw_cb = NULL;
    int iRtn = FAILED;
    char pcStrChapid[CHAP_ID_LEN * 2 + 1];
    int iChapid = 0;
    char *pcStrChapchallenge;
    int inputLen = strlen(inputStr);
    int i;
    char cInput;
   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
    if (inputLen < CHAP_ID_LEN) {
        LOGE("inputStr %s tooShort %d",inputStr, inputLen);
        return FAILED;
    }

    /*get rpc for ets bypass*/
    cw_handle = dlopen(CW_PATH, RTLD_NOW);
    if(!cw_handle){
        LOGE("load %s failed:%s\n",CW_PATH, dlerror());
        return FAILED;
    }
    cw_cb  = (cw_callback4)dlsym(cw_handle,"MakeMD5");
    if(!cw_cb ){
        dlclose(cw_handle);
        LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
        return FAILED;
    }

    for (i =0 ; i < CHAP_ID_LEN; i++) {
        iChapid <<= 4;
        cInput = inputStr[i];
        if (cInput >= 'a' && cInput <= 'f') {
            iChapid += cInput - 'a' + 10;
        } else if (cInput >= 'A' && cInput <= 'F') {
            iChapid += cInput - 'A' + 10;
        } else if (cInput >= '0' && cInput <= '9') {
            iChapid += cInput - '0';
        }
    }
    sprintf(pcStrChapid, "%d", iChapid);
    pcStrChapchallenge = (char*)malloc(inputLen);
    strcpy(pcStrChapchallenge, inputStr + CHAP_ID_LEN);

    LOGD("CWCLIENT before callback pcStrChapid = %s, pcStrChapchallenge=%s, pszResponse=%s\n",pcStrChapid,pcStrChapchallenge,*pszResponse);
    /*Send AT+VUSBETS*/
    iRtn = cw_cb(pcStrChapid, pcStrChapchallenge, &pszResponse);
    LOGD("CWCLIENT after callback pcStrChapid = %s, pcStrChapchallenge=%s, pszResponse=%s\n",pcStrChapid,pcStrChapchallenge,*pszResponse);
    //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
    //sleep(1);

    if(cw_handle){
        dlclose(cw_handle);
    }

    free(pcStrChapchallenge);
    return iRtn;
}

char * copyResult(char *source, int isFreeSource) {
    char *resultP;
    if( source == NULL)
        return NULL;
    resultP = malloc(strlen(source) + 1);
    strcpy(resultP, source);
    if(isFreeSource)
    {
        free(source);
        source = NULL;
    }
    return resultP;
}

char * dsatvend_exec_cave_cmd(char *input) {
    char *result = NULL;
    int ret = cave_authentication(input, &result);
    if(ret == FAILED)
        return NULL;
    return copyResult(result, 1);
}

char * dsatvend_exec_md5_authr(char *input) {
    char *result = NULL;
    int ret = md5_authentication(input, &result);
    if(ret == FAILED)
        return NULL;
    return copyResult(result, 1);
}

char * fn_uim_auth(char *auth) {
    char *result = NULL;
    int ret = aka_get_uimauth(&result);
    if(ret == FAILED)
        return 1;
    strcpy(auth, result);
    return 0;
}

char * dsatvend_get_imsi(char *auth) {
    char *imsi = NULL;
    int ret = aka_get_imsi(&imsi);
    if(ret == FAILED)
        return NULL;
    return copyResult(imsi, 1);
}


