#ifndef __EPDGA_H__
#define __EPDGA_H__

#include <stdint.h>

#define IFNAMSIZ         16
#define APN_NAME_LEN     20
#define V4_ADDR_STR_LEN  16
#define V6_ADDR_STR_LEN  40
#define IF_NAME_LEN      20
#define IMSI_STR_LEN     25
#define MNC_STR_LEN      4
#define MCC_STR_LEN      4
#define MAC_ADDR_STR_LEN 18
#define FQDN_STR_LEN     200

#define EPDGA_MAX_NUM_PCSCFS 10
#define EPDGA_IPV6_ADDR_BIN_LEN 16

#define AKA_AUTH_LEN     64
#define AKA_RAND_LEN     64
#define AKA_RESPONSE_LEN 500


typedef char ea_apn_name_t[APN_NAME_LEN];


typedef enum ea_rsp_type_t {
  NONE_RSP,
  ATTACH_CNF,
  DETACH_CNF,
  HOL2W_CNF,
  DISCONNECT
} ea_rsp_type_t;

typedef struct ea_sim_info_t {
  char imsi[IMSI_STR_LEN];
  char mcc[MNC_STR_LEN];
  char mnc[MNC_STR_LEN];
} ea_sim_info_t;

typedef struct ea_if_info_t {
  char if_name[IFNAMSIZ];
  char addr_v6[V6_ADDR_STR_LEN];
  char addr_v4[V4_ADDR_STR_LEN];
} ea_if_info_t;

typedef enum ea_ip_type_t {
  IPv4   = 0,
  IPv6   = 1,
  IPv4v6 = 2,
} ea_ip_type_t;

typedef struct ea_attach_info_t {
  ea_apn_name_t  apn;
  ea_if_info_t   wifi;
  ea_sim_info_t  sim;
  ea_ip_type_t   pdn_type;
  ea_if_info_t   interface;
  uint32_t       timeout_sec;
  char           fqdn[FQDN_STR_LEN];
  char ap_mac[MAC_ADDR_STR_LEN];
} ea_attach_info_t;

typedef struct ea_detach_info_t {
  ea_apn_name_t  apn;
} ea_detach_info_t;

typedef ea_attach_info_t ea_hol2w_info_t;

typedef struct ea_rsp_t {
  ea_rsp_type_t type;
  ea_apn_name_t apn;
  uint32_t status; //uint32_t
  uint32_t sub_errno;
  char if_name[IFNAMSIZ];
  uint32_t mtu;
  char addr_v6[V6_ADDR_STR_LEN];
  char pcscf_v6[V6_ADDR_STR_LEN];
  char dns_v6[V6_ADDR_STR_LEN];
  char addr_v4[V4_ADDR_STR_LEN];
  char pcscf_v4[V4_ADDR_STR_LEN];
  char dns_v4[V4_ADDR_STR_LEN];
} ea_rsp_t;

typedef struct ea_pcscfs_t {
  unsigned short pcscf_num;
  unsigned short pcscf_v6_mask;// bit mask => 1 : IPV6, 0 : IPV4
  unsigned char  pcscf[EPDGA_MAX_NUM_PCSCFS][EPDGA_IPV6_ADDR_BIN_LEN];
} ea_pcscfs_t;

typedef struct ea_sim_req_t {
  ea_apn_name_t  apn;
  char rand[AKA_AUTH_LEN];
  char auth[AKA_RAND_LEN];
} ea_sim_req_t;

typedef struct ea_sim_rsp_t {
  int sw1;
  int sw2;
  char response[AKA_RESPONSE_LEN];
} ea_sim_rsp_t;

typedef struct ea_pdn_info {
  ea_apn_name_t  apn;
  char ap_mac[MAC_ADDR_STR_LEN];
  ea_pcscfs_t pcscfs;
} ea_pdn_info;

#endif