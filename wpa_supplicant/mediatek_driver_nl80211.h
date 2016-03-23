/*
 * Driver interaction with Linux nl80211/cfg80211
 * Copyright (c) 2002-2010, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2003-2004, Instant802 Networks, Inc.
 * Copyright (c) 2005-2006, Devicescape Software, Inc.
 * Copyright (c) 2007, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2009-2010, Atheros Communications
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef _MTK_DRIVER_NL80211_H_
#define _MTK_DRIVER_NL80211_H_


#ifndef BITS
/* Eddie */
/* bits range: for example BITS(16,23) = 0xFF0000
 *   ==>  (BIT(m)-1)   = 0x0000FFFF     ~(BIT(m)-1)   => 0xFFFF0000
 *   ==>  (BIT(n+1)-1) = 0x00FFFFFF
 */
#define BITS(m,n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */


enum nl80211_testmode_sta_link_statistics_attr{
    __NL80211_TESTMODE_STA_STATISTICS_INVALID = 0,
    NL80211_TESTMODE_STA_STATISTICS_VERSION,
    NL80211_TESTMODE_STA_STATISTICS_MAC,
    NL80211_TESTMODE_STA_STATISTICS_LINK_SCORE,
    NL80211_TESTMODE_STA_STATISTICS_FLAG,

    NL80211_TESTMODE_STA_STATISTICS_PER,
    NL80211_TESTMODE_STA_STATISTICS_RSSI,
    NL80211_TESTMODE_STA_STATISTICS_PHY_MODE,
    NL80211_TESTMODE_STA_STATISTICS_TX_RATE,

    NL80211_TESTMODE_STA_STATISTICS_TOTAL_CNT,
    NL80211_TESTMODE_STA_STATISTICS_THRESHOLD_CNT,
    NL80211_TESTMODE_STA_STATISTICS_AVG_PROCESS_TIME,

    NL80211_TESTMODE_STA_STATISTICS_FAIL_CNT,
    NL80211_TESTMODE_STA_STATISTICS_TIMEOUT_CNT,
    NL80211_TESTMODE_STA_STATISTICS_AVG_AIR_TIME,

    NL80211_TESTMODE_STA_STATISTICS_TC_EMPTY_CNT_ARRAY,
    NL80211_TESTMODE_STA_STATISTICS_TC_QUE_LEN_ARRAY,

    NL80211_TESTMODE_STA_STATISTICS_TC_AVG_QUE_LEN_ARRAY,
    NL80211_TESTMODE_STA_STATISTICS_TC_CUR_QUE_LEN_ARRAY,

    NL80211_TESTMODE_STA_STATISTICS_RESERVED_ARRAY,

    NL80211_TESTMODE_STA_STATISTICS_NUM,
    NL80211_TESTMODE_STA_STATISTICS_MAX = NL80211_TESTMODE_STA_STATISTICS_NUM - 1
};

enum nl80211_testmode_link_detect_attr{
    NL80211_TESTMODE_LINK_INVALID = 0,
    NL80211_TESTMODE_LINK_TX_FAIL_CNT,
    NL80211_TESTMODE_LINK_TX_RETRY_CNT,
    NL80211_TESTMODE_LINK_TX_MULTI_RETRY_CNT,
    NL80211_TESTMODE_LINK_ACK_FAIL_CNT,
    NL80211_TESTMODE_LINK_FCS_ERR_CNT,
    NL80211_TESTMODE_LINK_TX_CNT,
    NL80211_TESTMODE_LINK_TX_OK_CNT,
    NL80211_TESTMODE_LINK_TX_OS_CNT,

    NL80211_TESTMODE_LINK_DETECT_NUM,
    NL80211_TESTMODE_LINK_DETECT_MAX = NL80211_TESTMODE_LINK_DETECT_NUM - 1
};

typedef enum _ENUM_TRAFFIC_CLASS_INDEX_T {
    TC0_INDEX = 0,
    TC1_INDEX,
    TC2_INDEX,
    TC3_INDEX,
    TC_DATA_NUM,
    TC4_INDEX = TC_DATA_NUM,
    TC5_INDEX,
    TC_NUM
} ENUM_TRAFFIC_CLASS_INDEX_T;

struct wpa_driver_sta_statistics_s {
    u8      version;
    u8      addr[ETH_ALEN];
    u32     flag;

    u8      link_score;
    u8      per;
    int     rssi;
    u32     phy_mode;
    double  tx_rate;

    u32     tx_total_cnt;
    u32     tx_exc_threshold_cnt;
    u32     tx_avg_process_time;
    u32     tx_fail_cnt;
    u32     tx_timeout_cnt;
    u32     tx_avg_air_time;

    u32     tc_buf_full_cnt[TC_DATA_NUM];
    u32     tc_que_len[TC_DATA_NUM];

    u32     tc_avg_que_len[TC_DATA_NUM];
    u32     tc_cur_que_len[TC_DATA_NUM];

    u8      reserved[32];
};

struct wpa_driver_sta_link_detect_s {
    u64     tx_fail_cnt;
    u64     tx_retry_cnt;
    u64     tx_multi_retry_cnt;
    u64     ack_fail_cnt;
    u64     fcs_err_cnt;
    u64     tx_cnt;
    u64     tx_ok_cnt;
    u64     tx_os_cnt;
};

/* SIOCSIWENCODEEXT definitions */
#define IW_ENCODE_ALG_SMS4  0x20

/* P2P Sigma*/
struct wpa_driver_test_mode_info {
    u32  index;
    u32  buflen;
};

struct wpa_driver_testmode_params {
    struct wpa_driver_test_mode_info hdr;
    u8  *buf;
};

struct wpa_driver_get_sta_statistics_params {
    struct wpa_driver_test_mode_info hdr;
    u32 version;
    u32 flag;
    u8  addr[ETH_ALEN];
    u8  *buf;
};

struct wpa_driver_p2p_sigma_params {
    struct wpa_driver_test_mode_info hdr;
    u32 idx;
    u32 value;
};

struct wpa_driver_get_sta_link_detect_params {
    struct wpa_driver_test_mode_info hdr;
    u8  *buf;
};

/* Hotspot Client Management */
struct wpa_driver_hotspot_params {
    struct wpa_driver_test_mode_info hdr;
    u8  blocked;
    u8  bssid[ETH_ALEN];
};

/* SW CMD */
struct wpa_driver_sw_cmd_params {
    struct wpa_driver_test_mode_info hdr;
    u8  set;
    u32 adr;
    u32 data;
};

struct wpa_driver_suspendmode_params {
    struct wpa_driver_test_mode_info hdr;
    u8  suspend;
};

/* WAPI */
struct iw_encode_exts {
    u32 ext_flags; /* IW_ENCODE_EXT_* */
    u8  tx_seq[IW_ENCODE_SEQ_MAX_SIZE]; /* LSB first */
    u8  rx_seq[IW_ENCODE_SEQ_MAX_SIZE]; /* LSB first */
    u8  addr[ETH_ALEN]; /* ff:ff:ff:ff:ff:ff for broadcast/multicast
			       * (group) keys or unicast address for
			       * individual keys */
    u16 alg; /* IW_ENCODE_ALG_* */
    u16 key_len;
    u8  key[32];
};

struct wpa_driver_wapi_key_params {
    struct wpa_driver_test_mode_info hdr;
    u8  key_index;
    u8  key_len;
    struct iw_encode_exts extparams;
};

/* CONFIG_MTK_P2P */
struct wpa_driver_wfd_data_s {
    struct wpa_driver_test_mode_info hdr;
    u32 WfdCmdType;
    u8  WfdEnable;
    u8  WfdCoupleSinkStatus;
    u8  WfdSessionAvailable;
    u8  WfdSigmaMode;
    u16 WfdDevInfo;
    u16 WfdControlPort;
    u16 WfdMaximumTp;
    u16 WfdExtendCap;
    u8  WfdCoupleSinkAddress[ETH_ALEN];
    u8  WfdAssociatedBssid[ETH_ALEN];
    u8  WfdVideoIp[4];
    u8  WfdAudioIp[4];
    u16 WfdVideoPort;
    u16 WfdAudioPort;
    u32 WfdFlag;
    u32 WfdPolicy;
    u32 WfdState;
    u8  WfdSessionInformationIE[24*8];  /* Include Subelement ID, length */
    u16 WfdSessionInformationIELen;
    u8  Reverved1[2];
    u8  WfdPrimarySinkMac[ETH_ALEN];
    u8  WfdSecondarySinkMac[ETH_ALEN];
    u32 WfdAdvancedFlag;
    /* Group 1 64 bytes */
    u8  WfdLocalIp[4];
    u16 WfdLifetimeAc2; /* Unit is 2 TU */
    u16 WfdLifetimeAc3; /* Unit is 2 TU */
    u16 WfdCounterThreshold;  /* Unit is ms */
    u8  Reverved2[54];
    /* Group 2 64 bytes */
    u8  Reverved3[64];
    /* Group 3 64 bytes */
    u8  Reverved4[64];
} wfd_data;

struct wpa_driver_set_beamplus_params {
    struct wpa_driver_test_mode_info hdr;
    u32 value;
};

enum nl80211_testmode_params {
    /* Old test mode command id, compatible with exist testmode command */
    NL80211_TESTMODE_SW_CMD = 1,
    NL80211_TESTMODE_WAPI = 2,
    NL80211_TESTMODE_HS20 = 3,
    NL80211_TESTMODE_POORLINK = 4,
    NL80211_TESTMODE_STATISTICS = 0x10,
    NL80211_TESTMODE_LINK_DETECT = 0x20,

    /* New test mode command id, should greater than TESTMODE_CMD_ID_NEW_BEGIN */
    NL80211_TESTMODE_NEW_BEGIN = 100,
    NL80211_TESTMODE_SUSPEND = 101,
};

#ifdef CONFIG_MTK_LTE_COEX
enum nl80211_testmode_available_chan_attr{
    NL80211_TESTMODE_AVAILABLE_CHAN_INVALID = 0,
    NL80211_TESTMODE_AVAILABLE_CHAN_2G_BASE_1,
    NL80211_TESTMODE_AVAILABLE_CHAN_5G_BASE_34,
    NL80211_TESTMODE_AVAILABLE_CHAN_5G_BASE_100,
    NL80211_TESTMODE_AVAILABLE_CHAN_5G_BASE_149,
    NL80211_TESTMODE_AVAILABLE_CHAN_5G_BASE_184,

    NL80211_TESTMODE_AVAILABLE_CHAN_NUM,
    NL80211_TESTMODE_AVAILABLE_CHAN_MAX = NL80211_TESTMODE_AVAILABLE_CHAN_NUM - 1
};

struct wpa_driver_get_available_channel_params {
    struct wpa_driver_test_mode_info hdr;
    u8  *buf;
};

struct wpa_driver_available_chan_s {
    u32 ch_2g_base1;
    u32 ch_5g_base34;
    u32 ch_5g_base100;
    u32 ch_5g_base149;
    u32 ch_5g_base184;
};
#endif

#ifdef CONFIG_MTK_HS20R1
enum TestModeCmdType {
    TESTMODE_CMD_ID_SW_CMD = 1,
    TESTMODE_CMD_ID_WAPI = 2,
    TESTMODE_CMD_ID_HS20 = 3,
    NUM_OF_TESTMODE_CMD_ID
};

enum Hs20CmdType {
    HS20_CMD_ID_SET_BSSID_POOL = 0,
    NUM_OF_HS20_CMD_ID
};

struct param_hs20_set_bssid_pool {
    u8 fgBssidPoolIsEnable;
    u8 ucNumBssidPool;
    u8 arBssidPool[8][ETH_ALEN];
};

struct wpa_driver_hs20_data_s {
    struct wpa_driver_test_mode_info hdr;
    enum Hs20CmdType CmdType;
    struct param_hs20_set_bssid_pool hs20_set_bssid_pool;
};


#define WFA_IF_NAME_LEN   16
#define WFA_HS20_TYPE_LEN 16
#define WFA_HS20_LEN_256  256
#define WFA_HS20_LEN_32   32


#ifdef CONFIG_MTK_HS20R1_SIGMA

#define HS20_MAX_NUM_CREDENTIAL 8

struct hs20_credential {
    struct hs20_credential *prNext;

    char intf[WFA_IF_NAME_LEN];
    char type[WFA_HS20_TYPE_LEN];
    char username[WFA_HS20_LEN_32];
    char password[WFA_HS20_LEN_256];
    char imsi[WFA_HS20_LEN_32];
    char plmn_mnc[WFA_HS20_LEN_32];
    char plmn_mcc[WFA_HS20_LEN_32];
    char root_ca[WFA_HS20_LEN_32];
    char realm[WFA_HS20_LEN_32];
    u8   prefer;
    char fqdn[WFA_HS20_LEN_32];
    char clientCA[WFA_HS20_LEN_32];
};

struct sigma_cmd_sta_cred_list {
    struct hs20_credential *prNext;
    size_t cred_count;
};

struct hs20_sigma {
    u8 **bssid_pool;
    size_t bssid_pool_count;
    // struct sigma_cmd_sta_bssid_pool *prBssidPool;
    // struct sigma_cmd_sta_add_credential arCred[HS20_MAX_NUM_CREDENTIAL];
    // size_t cred_count;
    struct sigma_cmd_sta_cred_list *prCredLst;
    u8 is_running;
    u8 is_connected;
    u8 assoc_backoff_time;
    u8 is_force_reassoc;
    // u8 *ns_ssid;
    // int ns_ssid_len;
    // u8 ns_bssid[ETH_ALEN];
};

#endif /* CONFIG_MTK_HS20R1_SIGMA */

#endif /* CONFIG_MTK_HS20R1 */


#endif
