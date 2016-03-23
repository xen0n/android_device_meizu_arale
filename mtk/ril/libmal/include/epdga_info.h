#ifndef __EPDGA_INFO_H__
#define __EPDGA_INFO_H__

#include "mdfx.h"

#define EPDGA_MAX_NUM_PCSCFS    10
#define EPDGA_IPV6_ADDR_BIN_LEN 16

typedef struct epdga_pcscfs_t {
  unsigned short pcscf_num;
  unsigned short pcscf_v6_mask;// bit mask => 1 : IPV6, 0 : IPV4
  unsigned char  pcscf[EPDGA_MAX_NUM_PCSCFS][EPDGA_IPV6_ADDR_BIN_LEN];
} epdga_pcscfs_t;


int mal_epdga_query_pcscfs(void *conn_ptr, epdga_pcscfs_t *pcscfs);
int mal_epdga_query_ap_mac(void *conn_ptr, char *mac_addr);

#endif