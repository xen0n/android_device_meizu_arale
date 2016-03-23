/*
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <rpc_ril.h>
#include <cw.h>
#include <dlfcn.h>
#include <telephony/ril.h>
#include <telephony/via-ril.h>
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "private/android_filesystem_config.h"
//#include "cpbypass.h"

#ifdef ANDROID_JB
#include <c2k_log.h>
#endif



#define  RPC_RIL_PATH  "/system/lib/librpcril.so"
typedef int (*rpcril_callback)(int, RIL_RPC_ParaTypes, int, void*, RIL_RPC_ParaTypes, int*, void***);

int ReadIMSI(char *** szIMSI)
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;
   int Rtn = 0;
   int len = 1;
   char ** str = NULL;
   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
   /*get rpc for ets bypass*/
   char *p1=NULL;
   char *p2="asdf";
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return Rtn;
   }

   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return Rtn;
   }


   /*Send AT+VUSBETS*/
   if(0 != rpcril_cb(RIL_REQUEST_READ_IMSI, RIL_RPC_PARA_INTS, 0, NULL, RIL_RPC_PARA_STRING, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       return Rtn;
   }
   LOGW("rpcril_cb\n");


   //str = (char **)malloc(sizeof(char *));
  // if(str== NULL)

                //{

                 //   LOGE("malloc memory for strings failure");

                //}

   str = (char**)out_value;
   LOGW("rpcril_cb1 str0 is  %s,*out_value is %s \n",str[0],out_value[0]);
//   p1=(char *)malloc(strlen(p2));
//   strcpy(p1,p2);
   p1=p2;
   LOGW("p1 is %s,p2 is %s \n",p1,p2);
   //**szIMSI=(char *)malloc(strlen(out_value[0]));   
   //strcpy(**szIMSI, out_value[0]);
   //strcpy(szIMSI, (const char *)str[0]);
  **szIMSI=out_value[0]; 
  LOGW("rpcril_cb2 not use strcpy malloc \n");
  LOGW("**szIMSI = %s", **szIMSI);
  LOGW("out_len = %d", out_len);

   str = NULL;
   out_value[0] = NULL;
   free(out_value);
   out_value = NULL;
   
   LOGW("**szIMSI = %s", **szIMSI);
   //sleep(1);
   if(rpcril_handle){
   dlclose(rpcril_handle);
   }
   LOGW("Rtn = %d", Rtn);
   Rtn = 1;
   LOGW("Rtn = %d", Rtn);
   return Rtn;
}

unsigned int EnumAuthAlgs()
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;
   int Rtn = 0;
   char ** str = NULL;

   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return Rtn;
   }

   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return Rtn;
   }


   /*Send AT+VUSBETS*/
   if(0 != rpcril_cb(RIL_REQUEST_GET_UIMAUTH, RIL_RPC_PARA_INTS, 0, NULL, RIL_RPC_PARA_STRINGS, &out_len, &out_value))
    {
       LOGW("rpcril_cb error\n");
       return Rtn;
   }
    
   str = (char**)out_value;
   LOGW("str[0] = %s", str[0]);
   LOGW("out_len = %d", out_len);
   if(!(strcmp("CAVE", str[0]) || strcmp("MD5", str[1])))
         Rtn = CT_AUTH_CAVE|CT_AUTH_MD5;
   else if(!strcmp("CAVE", str[0]))
         Rtn = CT_AUTH_CAVE;
   else if(!strcmp("MD5", str[0]))
         Rtn = CT_AUTH_MD5;
   LOGW("Rtn = %d", Rtn);

   /*
   if(1 == out_len)
   {
      if(strcmp("cave", str[0]))
         Rtn = CT_AUTH_CAVE;
     else if(strcmp("md5", str[0]))
         Rtn = CT_AUTH_MD5;
     else if (strcmp("AKACAVE", str[0]))
         Rtn = CT_AUTH_AKACAVE;
   }
   */

   
   free(out_value[0]);
   free(out_value);
   out_value = NULL;
   //sleep(1);
   if(rpcril_handle){
   dlclose(rpcril_handle);
   }

   return Rtn;
}

ECaveResult RequestCave(const char *pstrrandu, char*** pstrathoru)
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;
   ECaveResult Rtn = CAVE_RUNFAIL;

   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return Rtn;
   }

   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return Rtn;
   }

   LOGW("LIBCW before rpcril_cb pstrrandu = %s\n",pstrrandu);

   /*Send AT+VUSBETS*/
   if(0 != rpcril_cb(RIL_REQUEST_MAKE_CAVE, RIL_RPC_PARA_STRING, 1, pstrrandu, RIL_RPC_PARA_STRING, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       return Rtn;
   }
   LOGW("out_len = %d", out_len);
   if(1 == out_len)
   {
      //strcpy(pstrathoru, out_value[0]);
      **pstrathoru=out_value[0];
      Rtn = CAVE_SUCESS;
  }
   
   LOGW("LIBCW **pstrathoru = %s,Rtn=%d\n",**pstrathoru,Rtn);
   LOGW("out_value = %s", *out_value);

   out_value[0] = NULL;
   free(out_value);
   out_value = NULL;
   
   //sleep(1);
   if(rpcril_handle){	
   dlclose(rpcril_handle);
   }

   return Rtn;

}

int MakeMD5(const char *pcStrChapid, const char *pcStrChapchallenge, char ***pszResponse)
{
   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;
   int Rtn = -1;
   char **str= (char **)malloc(strlen(pcStrChapid) +strlen(pcStrChapchallenge));
   if(NULL == str)
   {
        LOGE("malloc failed");
        return Rtn;
   }
   //char **str=NULL;
   str[0] = pcStrChapid;
   str[1] = pcStrChapchallenge;


   LOGW("LIBCW before dlopen str[0] = %s, str[1]=%s \n",str[0],str[1]);
   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       free(str);
       str = NULL;
       return Rtn;
   }

   LOGW("LIBCW before dlsym send RpcRequest str[0] = %s, str[1]=%s \n",str[0],str[1]);
   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       free(str);
       str = NULL;
       return Rtn;
   }
   
   LOGW("LIBCW before callback str[0] = %s, str[1]=%s \n",str[0],str[1]);
   /*Send AT+VUSBETS*/
   if(0 != rpcril_cb(RIL_REQUEST_MAKE_MD5, RIL_RPC_PARA_STRINGS, 2, str, RIL_RPC_PARA_STRING, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       free(str);
       str = NULL;
       return Rtn;
   }
   LOGW("out_len = %d", out_len);
   //LOGW("out_value = %s", *out_value);

   if(NULL == out_value)
   {
       LOGW("Rtn = %d", Rtn);
       free(str);
       str = NULL;
       return Rtn;       
    }
   **pszResponse=out_value[0];
   //strcpy(pszResponse, *out_value);
   LOGW("LIBCW after callback pszResponse = %s, out_value[0] =%s \n", **pszResponse,out_value[0]);

   *out_value = NULL;
   out_value = NULL;
   free(out_value);
   LOGW("pszResponse = %s", pszResponse);
   //sleep(1);
   if(rpcril_handle){	
      dlclose(rpcril_handle);
   }
   
   free(str);
   str = NULL;
   Rtn = 1;
   return Rtn;
}

int ListCardType()
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;
   int Rtn = -1;
   int ** p = NULL;
   int *p1 = NULL;

  
   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return Rtn;
   }

   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return Rtn;
   }


   /*Send AT+VUSBETS*/

   if( 0 != rpcril_cb(RIL_REQUEST_GET_PREFMODE, RIL_RPC_PARA_INTS, 0, NULL, RIL_RPC_PARA_INTS, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       return Rtn;
   }
   p = (int**)out_value;
   p1 = *p;

   LOGW("Rtn = %d", p[0]);
   LOGW("Rtn = %d", *p);
   LOGW("Rtn = %d", p1);

   Rtn = *p;
   free(out_value);
   out_value = NULL;
   
   //sleep(1);
   if(rpcril_handle){
   dlclose(rpcril_handle);
   }
   
   return Rtn;
}

int  GetUimid(char ***pszMeid, char ***pszUimid)
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;
   char ** str = NULL;
   LOGW("%s: suffix Version: %s", __FUNCTION__, VIA_SUFFIX_VERSION);
   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return -1;
   }

   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return -1;
   }

   LOGW("LIBCW before rpcril_cb \n");
   /*Send AT+VUSBETS*/
   if( 0 != rpcril_cb(RIL_REQUEST_GET_UIMID, RIL_RPC_PARA_INTS, 0, NULL, RIL_RPC_PARA_STRINGS, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       return -1;
   }
   
   //str = (char **)out_value;
   //LOGW("out_value = %s", *out_value);
   //strcpy(pszMeid, str[0]);
  // strcpy(pszUimid, str[1]);
   **pszMeid=out_value[0];
   **pszUimid=out_value[1];
   LOGW("LIBCW pszMeid = %s, pszUimid =%s ,out_value[0]=%s, out_value[1]=%s \n", **pszMeid,**pszUimid,out_value[0],out_value[1]);
  /* LOGW("pszMeid = %s", pszMeid);
   LOGW("pszUimid = %s", pszUimid);
   LOGW("out_value = %s", *out_value);
   LOGW("out_value = %s", *out_value);
   LOGW("out_value = %s", out_value[0]);
   LOGW("out_value = %s", out_value[1]);
*/
   out_value[0] = NULL;
   out_value[1] = NULL;
   free(out_value);
   out_value = NULL;
   
   //sleep(1);
   if(rpcril_handle){
   dlclose(rpcril_handle);
   }

   return 0;
}

int  GetGeneratekey(char ***pszSmekey, char ***pszVpm)
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;

   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return -1;
   }

   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return -1;
   }


   /*Send AT+VUSBETS*/
   if(0 != rpcril_cb(RIL_REQUEST_GENERATE_KEY, RIL_RPC_PARA_INTS, 0, NULL, RIL_RPC_PARA_STRINGS, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       return -1;
   }

   //LOGW("out_value = %s", *out_value);
   //strcpy(pszSmekey, out_value[0]);
   //strcpy(pszVpm, out_value[1]);
   **pszSmekey=out_value[0];
   **pszVpm=out_value[1];
   LOGW("LIBCW pszSmekey = %s, pszVpm = %s ,out_value0=%s ,out_value1=%s\n", **pszSmekey,**pszVpm,out_value[0],out_value[1]);

   out_value[0] = NULL;
   out_value[1] = NULL;
   free(out_value);
   out_value = NULL;

   //LOGD("Cpbpass send RIL_REQUEST_GET_UIMAUTH = %d\n", !!enable);
   //sleep(1);
   if(rpcril_handle){
   dlclose(rpcril_handle);
   }

   return 1;
}

int  SSDConfirm(const char *pcStrAuthbs)
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;
   
   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return -1;
   }
   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return -1;
   }

   /*Send AT+VUSBETS*/
   if( 0 != rpcril_cb(RIL_REQUEST_UPDCON_SSD, RIL_RPC_PARA_STRING, 1, pcStrAuthbs, RIL_RPC_PARA_NULL, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       return -1;
   }
   
   //sleep(1);
   if(rpcril_handle){
   dlclose(rpcril_handle);
   }

   return 0;
}

int  SSDUpdate(const char * pcStrRandssd, char ***pszRandbs)
{
   void *rpcril_handle = NULL; /*the handle of loaded librpcril.so*/
   rpcril_callback rpcril_cb = NULL;
   void **out_value = NULL;
   int out_len = 0;

   /*get rpc for ets bypass*/
   rpcril_handle = dlopen(RPC_RIL_PATH, RTLD_NOW);
   if(!rpcril_handle){
       LOGE("load %s failed:%s\n",RPC_RIL_PATH, dlerror());
       return -1;
   }

   rpcril_cb = (rpcril_callback)dlsym(rpcril_handle,"sendRpcRequestComm");
   if(!rpcril_cb){
       dlclose(rpcril_handle);
       LOGW("find symbol sendRpcRequest failed:%s\n",dlerror());
       return -1;
   }

   /*Send AT+VUSBETS*/
   if(0 != rpcril_cb(RIL_REQUEST_UPDATE_SSD, RIL_RPC_PARA_STRING, 1, pcStrRandssd, RIL_RPC_PARA_STRING, &out_len, &out_value))
   {
       LOGW("rpcril_cb error\n");
       return -1;
   }
   
   **pszRandbs=out_value[0];
   //strcpy(pszRandbs, out_value[0]);
   LOGW("LIBCW SSDUpdate pszRandbs = %s,out_value[0]= %s \n", **pszRandbs,out_value[0]);

   out_value[0] = NULL;
   free(out_value);
   out_value = NULL;

   //sleep(1);
   if(rpcril_handle){
   dlclose(rpcril_handle);
   }

   return 0;
}
