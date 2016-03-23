#ifndef _CUTILS_RPC_RIL_H
#define _CUTILS_RPC_RIL_H

typedef enum {
    RIL_RPC_PARA_INTS = 0,
    RIL_RPC_PARA_STRING = 1,
    RIL_RPC_PARA_STRINGS  = 2,
    RIL_RPC_PARA_RAW = 3,
    RIL_RPC_PARA_NULL = 4,
    RIL_RPC_PARA_RESERVE,
} RIL_RPC_ParaTypes;
#ifdef __cplusplus
namespace android{
extern "C" int sendRpcRequest(int request_num, int value);
extern "C" int sendRpcRequestComm(int request_num, RIL_RPC_ParaTypes in_types, int in_len, void *in_value, RIL_RPC_ParaTypes out_types, int *out_len, void ***out_value);
};
#else
int sendRpcRequest(int request_num, int value);
int sendRpcRequestComm(int request_num, RIL_RPC_ParaTypes in_types, int in_len, void *in_value, RIL_RPC_ParaTypes out_types, int *out_len, void ***out_value);
#endif

#endif /*_CUTILS_RPC_RIL_H*/

