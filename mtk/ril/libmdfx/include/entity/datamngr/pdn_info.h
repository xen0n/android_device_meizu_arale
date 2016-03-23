#ifndef PDN_INFO_H
#define PDN_INFO_H
#include "mdfx.h"
#include "apn_table.h"
#include "datamngr_debug.h"
#include "datamngr_event.h"

///////////////////////////////////////////////
#define EPDG_CID_OFFSET         (100)
#define MAX_MID_NUM             (11)

#define MID_INVALID             (-1)
#define MID_IMS                 (0)
#define MID_EIMS                (1)
#define MID_FIXED               (2)

#define ERAN_TYPE_MOBILE        (1)
#define ERAN_TYPE_WIFI          (2)

int add_cid2mid(int cid, int mid);
int get_cid2mid(int cid);
int get_mid2cid(int mid);
///////////////////////////////////////////////
#define DM_MAX_PDN_NUM (16)


typedef struct {
    int interfaceId;
    int primaryCid;
    int cid;
    int ddcId; //for dedicate bearer
    int isDedicateBearer;
    int isEmergency; //0: normal, 1: emergency PDN
    int active; //0: inactive, 1: link down, 2: active
    int signalingFlag;
    char ifname[STRING_SIZE];
    char apn[STRING_SIZE];
    int protocol;
    unsigned char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    unsigned char addressV6[MAX_IPV6_ADDRESS_LENGTH];
    unsigned dnsV4Num;
    unsigned dnsV6Num;
    unsigned char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH];
    unsigned char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    
    unsigned prefixLength;
    unsigned preferLifetime;
    unsigned validLifetime;
    
    void *priv;

    MAL_Dedicate_Data_Call_Struct bearer;

} dm_pdn_info_t;



//dm_pdn_info_t * alloc_pdn_info();
dm_pdn_info_t * find_pdn_info_by_apn(char *apn);
dm_pdn_info_t * find_pdn_info_by_index(unsigned index);
dm_pdn_info_t * find_pdn_info_by_interface(int interface_id);
int clear_pdn_info_by_cid(int cid);
int clear_pdn_info_by_if(int interfaceId, int eran_type);
dm_pdn_info_t * find_pdn_info_by_cid(int cid);
int add_pre_pdn_info(unsigned interface_id, char *apn);
int retrieve_pre_pdn_info(unsigned interface_id, char *apn);
int remove_pre_pdn_info(unsigned interface_id);
int get_interface_by_cid(int cid);
int reset_pdn_info();
void dump_pdn_info();


#endif

