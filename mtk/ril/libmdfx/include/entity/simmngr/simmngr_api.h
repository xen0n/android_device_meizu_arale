#ifndef SIMMNGR_API_H
#define SIMMNGR_API_H
#include "mdfx.h"

extern int sm_aka_verify(int rand_len, void *rand, int autn_len, void *autn, void *response, int *response_len);
extern int sm_query_mnc_mcc(void *mnc, int *mnc_len, void *mcc, int *mcc_len);
extern int sm_query_imsi(void *imsi, int *imsi_len);
extern int sm_query_imei(void *imei, int *imei_len);

#endif