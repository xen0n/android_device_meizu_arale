
#ifndef __EXPORT_h
#define __EXPORT_h

#ifdef __cplusplus
fd
extern "C" {
#endif

#pragma once

#ifdef _WIN32

#ifdef CDMAAPI_EXPORT
#define CDMA_API extern "C" _declspec(dllexport)
#else
#define CDMA_API extern "C" _declspec(dllimport)
#endif

#else

#ifdef CDMAAPI_EXPORT
ff
#define CDMA_API extern "C" 
#else
#define CDMA_API extern
#endif 

#endif

typedef enum
{
    CAVE_SUCESS = 0, //成功
    CAVE_NOUIM =1,	 //没有UIM
    CAVE_RUNFAIL=2, //运行失败
    CAVE_UNKNOW = 3  //未知错误
}ECaveResult;

//UIM卡鉴权算法定义
#define CT_AUTH_CAVE		1	//cave算法
#define CT_AUTH_MD5			1<<1//md5算法
#define CT_AUTH_AKACAVE		1<<2

//------------------------------------------------------------------------
//描述：获取UIM卡的IMSI号
//参数：szIMSI:返回的IMSI号
//返回：成功返回true,失败返回false
//------------------------------------------------------------------------
CDMA_API int ReadIMSI(char *** szIMSI);//ReadIMSI(BYTE szIMSI[32]);


//------------------------------------------------------------------------
//描述：UIM卡鉴权算法查询
//参数：无
//返回：0表示什么算法都不支持或出错了
//		1表示支持cave算法，
//		其他请参数UIM卡鉴权算法定义
//------------------------------------------------------------------------
CDMA_API unsigned int EnumAuthAlgs();


//------------------------------------------------------------------------
//描述：执行CAVE鉴权
//参数：pstrrandu 请参考统一认证AT指令定义说明
//		pstrathoru
//返回：请参数ECaveResult定义
//------------------------------------------------------------------------
CDMA_API ECaveResult RequestCave(const char *pstrrandu, char*** pstrathoru);

//------------------------------------------------------------------------
//描述：执行CAVE鉴权
//参数：pcStrChapid: 请参考统一认证AT指令定义说明
//		pcStrChapchallenge
//		pszResponse
//返回：1成功，其他值失败
//------------------------------------------------------------------------
CDMA_API int MakeMD5(const char *pcStrChapid, const char *pcStrChapchallenge, char ***pszResponse);


//------------------------------------------------------------------------
//描述：数据卡所支持的模式查询
//参数：无
//返回：-1获取失败
//		0:cdma模式
//		1:hdr模式
//		2:混合模式
//------------------------------------------------------------------------
CDMA_API int ListCardType();

//------------------------------------------------------------------------
//描述：读取数据卡/模块的ESN和UIMID
//参数：pszMeid: 请参考统一认证AT指令定义说明
//		pszUimid
//返回：0成功，其他值失败
//------------------------------------------------------------------------
CDMA_API int GetUimid(char ***pszMeid, char ***pszUimid);


//------------------------------------------------------------------------
//描述：Generate key/VPM查询
//参数：pcStrRandu: 请参考统一认证AT指令定义说明
//		pcStrAuthu
//		pszSmekey
//		pszVpm
//返回：1成功，其他值失败
//------------------------------------------------------------------------
CDMA_API int GetGeneratekey(char ***pszSmekey, char ***pszVpm);


//------------------------------------------------------------------------
//描述：SSD更新确认
//返回：0成功，其他值失败
//------------------------------------------------------------------------
CDMA_API int SSDConfirm(const char *pcStrAuthbs);


//------------------------------------------------------------------------
//描述： SSD更新
//返回：0成功，其他值失败
//------------------------------------------------------------------------
CDMA_API int SSDUpdate(const char * pcStrRandssd, char ***pszRandbs);

#ifdef __cplusplus
}
#endif

#endif /* _EXPORT_H */
