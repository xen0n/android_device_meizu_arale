/*
 * Driver interaction with extended Linux CFG8021
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 */
#include "hardware_legacy/driver_nl80211.h"
#include "mediatek_driver_nl80211.h"
#include "wpa_supplicant_i.h"
#include "config.h"
#include "driver_i.h"

#ifdef ANDROID
#include "android_drv.h"
#endif

typedef struct android_wifi_priv_cmd {
    char *buf;
    int used_len;
    int total_len;
} android_wifi_priv_cmd;

static int drv_errors = 0;

static void wpa_driver_send_hang_msg(struct wpa_driver_nl80211_data *drv)
{
    drv_errors++;
    if (drv_errors > DRV_NUMBER_SEQUENTIAL_ERRORS) {
        drv_errors = 0;
    }
}

static int testmode_sta_statistics_handler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *sinfo[NL80211_TESTMODE_STA_STATISTICS_NUM];
    struct wpa_driver_sta_statistics_s *sta_statistics = (struct wpa_driver_sta_statistics_s *)arg;
    unsigned char i = 0;
    static struct nla_policy policy[NL80211_TESTMODE_STA_STATISTICS_NUM] = {
        [NL80211_TESTMODE_STA_STATISTICS_VERSION]               = { .type = NLA_U8 },
        [NL80211_TESTMODE_STA_STATISTICS_MAC]                   = { .type = NLA_UNSPEC },
        [NL80211_TESTMODE_STA_STATISTICS_LINK_SCORE]            = { .type = NLA_U8 },
        [NL80211_TESTMODE_STA_STATISTICS_FLAG]                  = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_PER]                   = { .type = NLA_U8 },
        [NL80211_TESTMODE_STA_STATISTICS_RSSI]                  = { .type = NLA_U8 },
        [NL80211_TESTMODE_STA_STATISTICS_PHY_MODE]              = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_TX_RATE]               = { .type = NLA_U16 },
        [NL80211_TESTMODE_STA_STATISTICS_FAIL_CNT]              = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_TIMEOUT_CNT]           = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_AVG_AIR_TIME]          = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_TOTAL_CNT]             = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_THRESHOLD_CNT]         = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_AVG_PROCESS_TIME]      = { .type = NLA_U32 },
        [NL80211_TESTMODE_STA_STATISTICS_TC_EMPTY_CNT_ARRAY]    = { .type = NLA_UNSPEC },
        [NL80211_TESTMODE_STA_STATISTICS_TC_QUE_LEN_ARRAY]      = { .type = NLA_UNSPEC },
        [NL80211_TESTMODE_STA_STATISTICS_TC_AVG_QUE_LEN_ARRAY]  = { .type = NLA_UNSPEC },
        [NL80211_TESTMODE_STA_STATISTICS_TC_CUR_QUE_LEN_ARRAY]  = { .type = NLA_UNSPEC },
        [NL80211_TESTMODE_STA_STATISTICS_RESERVED_ARRAY]        = { .type = NLA_UNSPEC }
    };

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
          genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[NL80211_ATTR_TESTDATA] ||
            nla_parse_nested(sinfo, NL80211_TESTMODE_STA_STATISTICS_MAX, tb[NL80211_ATTR_TESTDATA], policy))
        return NL_SKIP;

    for(i=1; i < NL80211_TESTMODE_STA_STATISTICS_NUM; i++) {
        if(sinfo[i]) {
            switch(i) {
                case NL80211_TESTMODE_STA_STATISTICS_VERSION:
                    sta_statistics->version = nla_get_u8(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_MAC:
                    nla_memcpy(sta_statistics->addr, sinfo[i], ETH_ALEN);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_LINK_SCORE:
                    sta_statistics->link_score = nla_get_u8(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_FLAG:
                    sta_statistics->flag = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_PER:
                    sta_statistics->per = nla_get_u8(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_RSSI:
                    sta_statistics->rssi = (((int)nla_get_u8(sinfo[i]) - 220) / 2);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_PHY_MODE:
                    sta_statistics->phy_mode = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_TX_RATE:
                    sta_statistics->tx_rate = (((double)nla_get_u16(sinfo[i])) / 2);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_FAIL_CNT:
                    sta_statistics->tx_fail_cnt = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_TIMEOUT_CNT:
                    sta_statistics->tx_timeout_cnt = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_AVG_AIR_TIME:
                    sta_statistics->tx_avg_air_time = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_TOTAL_CNT:
                    sta_statistics->tx_total_cnt = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_THRESHOLD_CNT:
                    sta_statistics->tx_exc_threshold_cnt = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_AVG_PROCESS_TIME:
                    sta_statistics->tx_avg_process_time = nla_get_u32(sinfo[i]);
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_TC_EMPTY_CNT_ARRAY:
                    nla_memcpy(sta_statistics->tc_buf_full_cnt, sinfo[i], sizeof(sta_statistics->tc_buf_full_cnt));
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_TC_QUE_LEN_ARRAY:
                    nla_memcpy(sta_statistics->tc_que_len, sinfo[i], sizeof(sta_statistics->tc_que_len));
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_TC_AVG_QUE_LEN_ARRAY:
                    nla_memcpy(sta_statistics->tc_avg_que_len, sinfo[i], sizeof(sta_statistics->tc_avg_que_len));
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_TC_CUR_QUE_LEN_ARRAY:
                    nla_memcpy(sta_statistics->tc_cur_que_len, sinfo[i], sizeof(sta_statistics->tc_cur_que_len));
                    break;
                case NL80211_TESTMODE_STA_STATISTICS_RESERVED_ARRAY:
                    nla_memcpy(sta_statistics->reserved, sinfo[i], sizeof(sta_statistics->reserved));
                    break;
                default:
                    break;
            }
        }
    }

    return NL_SKIP;
}

static void * nl80211_cmd(struct wpa_driver_nl80211_data *drv,
              struct nl_msg *msg, int flags, uint8_t cmd)
{
    return genlmsg_put(msg, 0, 0, drv->global->nl80211_id,
               0, flags, cmd, 0);
}

int wpa_driver_nl80211_testmode(void *priv, const u8 *data,
                            size_t data_len)
{
    return 0;
}

static int wpa_driver_nl80211_driver_sw_cmd(void *priv,
    int set, unsigned long *adr, unsigned long *dat)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;
    struct wpa_driver_sw_cmd_params params;
    struct nl_msg *msg, *cqm = NULL;
    int ret = 0;

    os_memset(&params, 0, sizeof(params));

    params.hdr.index = 1;
    params.hdr.index = params.hdr.index | (0x01 << 24);
    params.hdr.buflen = sizeof(struct wpa_driver_sw_cmd_params);

    params.adr = *adr;
    params.data = *dat;

    if(set)
        params.set = 1;
    else
        params.set = 0;

    wpa_driver_nl80211_testmode(priv, (u8 *)&params, sizeof(struct wpa_driver_sw_cmd_params));
    return 0;
}

/*************************************************************
OVERLAPPED functins, previous defination is in driver_nl80211.c,
it will be modified
**************************************************************/

/**************************************************************************/
extern int wpa_config_write(const char *name, struct wpa_config *config);

static int wpa_driver_mediatek_set_country(void *priv, const char *alpha2_arg)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;
    int ioctl_sock=-1;
    struct iwreq iwr;
    int ret=-1;
    char buf[11];

    wpa_printf(MSG_DEBUG, "wpa_driver_nl80211_set_country");
    ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (ioctl_sock < 0) {
        wpa_printf(MSG_ERROR, "%s: socket(PF_INET,SOCK_DGRAM)", __func__);
        return -1;
    }
    os_memset(&iwr, 0, sizeof(iwr));
    os_strlcpy(iwr.ifr_name, drv->first_bss->ifname, IFNAMSIZ);
    sprintf(buf,"COUNTRY %s",alpha2_arg);
    iwr.u.data.pointer = buf;
    iwr.u.data.length = strlen(buf);
    if ((ret = ioctl(ioctl_sock, 0x8B0C, &iwr)) < 0) {//SIOCSIWPRIV
        wpa_printf(MSG_DEBUG, "ioctl[SIOCSIWPRIV]: %s",buf);
        close(ioctl_sock);
        return ret;
    }
    else {
        close(ioctl_sock);
        return 0;
    }

}

int wpas_get_sta_statistics(struct wpa_supplicant *wpa_s, char *sta_addr, u8 *buf)
{
    struct wpa_driver_get_sta_statistics_params params;

    os_memset(&params, 0, sizeof(params));

    if(sta_addr)
        os_memcpy(params.addr, sta_addr, ETH_ALEN);

    wpa_printf(MSG_DEBUG, "get_sta_statistics ["MACSTR"]", MAC2STR(params.addr));

    params.hdr.index = 0x10;
    params.hdr.index = params.hdr.index | (0x01 << 24);
    params.hdr.buflen = sizeof(struct wpa_driver_get_sta_statistics_params);

    /* buffer for return structure */
    params.buf = buf;

    return wpa_driver_nl80211_testmode(wpa_s->drv_priv, (u8 *)&params,
        sizeof(struct wpa_driver_get_sta_statistics_params));
}


static int print_sta_statistics(struct wpa_supplicant *wpa_s, struct wpa_driver_sta_statistics_s *sta_stats,
              unsigned long mask, char *buf, size_t buflen)
{
    size_t i;
    int ret;
    char *pos, *end;

    pos = buf;
    end = buf + buflen;

    ret = os_snprintf(pos, end - pos, "sta_addr="MACSTR"\n", MAC2STR(sta_stats->addr));
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "link_score=%d\n", sta_stats->link_score);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "per=%d\n", sta_stats->per);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "rssi=%d\n", sta_stats->rssi);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "phy=0x%08X\n", sta_stats->phy_mode);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "rate=%.1f\n", sta_stats->tx_rate);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "total_cnt=%d\n", sta_stats->tx_total_cnt);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "threshold_cnt=%d\n", sta_stats->tx_exc_threshold_cnt);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "fail_cnt=%d\n", sta_stats->tx_fail_cnt);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "timeout_cnt=%d\n", sta_stats->tx_timeout_cnt);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "apt=%d\n", sta_stats->tx_avg_process_time);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "aat=%d\n", sta_stats->tx_avg_air_time);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "TC_buf_full_cnt=%d:%d:%d:%d\n",
                                    sta_stats->tc_buf_full_cnt[TC0_INDEX],
                                    sta_stats->tc_buf_full_cnt[TC1_INDEX],
                                    sta_stats->tc_buf_full_cnt[TC2_INDEX],
                                    sta_stats->tc_buf_full_cnt[TC3_INDEX]);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "TC_sta_que_len=%d:%d:%d:%d\n",
                                    sta_stats->tc_que_len[TC0_INDEX],
                                    sta_stats->tc_que_len[TC1_INDEX],
                                    sta_stats->tc_que_len[TC2_INDEX],
                                    sta_stats->tc_que_len[TC3_INDEX]);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "TC_avg_que_len=%d:%d:%d:%d\n",
                                    sta_stats->tc_avg_que_len[TC0_INDEX],
                                    sta_stats->tc_avg_que_len[TC1_INDEX],
                                    sta_stats->tc_avg_que_len[TC2_INDEX],
                                    sta_stats->tc_avg_que_len[TC3_INDEX]);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "TC_cur_que_len=%d:%d:%d:%d\n",
                                    sta_stats->tc_cur_que_len[TC0_INDEX],
                                    sta_stats->tc_cur_que_len[TC1_INDEX],
                                    sta_stats->tc_cur_que_len[TC2_INDEX],
                                    sta_stats->tc_cur_que_len[TC3_INDEX]);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "flag=0x%08X\n", sta_stats->flag);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "reserved0=");
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;
    for(i = 0; i < 16; i++) {
        ret = os_snprintf(pos, end - pos, "%02X", sta_stats->reserved[i]);
        if (ret < 0 || ret >= end - pos)
            return 0;
        pos += ret;

        if(((i + 1) % 4) == 0) {
            ret = os_snprintf(pos, end - pos, " ", sta_stats->reserved[i]);
            if (ret < 0 || ret >= end - pos)
                return 0;
            pos += ret;
        }
    }
    ret = os_snprintf(pos, end - pos, "\n", sta_stats->reserved[i]);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "reserved1=");
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;
    for(i = 16; i < 32; i++) {
        ret = os_snprintf(pos, end - pos, "%02X", sta_stats->reserved[i]);
        if (ret < 0 || ret >= end - pos)
            return 0;
        pos += ret;

        if(((i + 1) % 4) == 0) {
            ret = os_snprintf(pos, end - pos, " ", sta_stats->reserved[i]);
            if (ret < 0 || ret >= end - pos)
                return 0;
            pos += ret;
        }
    }
    ret = os_snprintf(pos, end - pos, "\n", sta_stats->reserved[i]);
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    ret = os_snprintf(pos, end - pos, "====\n");
    if (ret < 0 || ret >= end - pos)
        return 0;
    pos += ret;

    return pos - buf;
}

int wpa_driver_get_sta_statistics(struct wpa_supplicant *wpa_s, char *addr,
                                char *buf, size_t buflen)
{
    char *str = NULL;
    int len = 0;
    u8 sta_addr[ETH_ALEN];

    struct wpa_driver_sta_statistics_s sta_statistics;


    if (hwaddr_aton(addr, sta_addr)) {
        wpa_printf(MSG_DEBUG, "CTRL_IFACE GET_STA_STATISTICS: invalid "
               "address '%s'", addr);
        return -1;
    }

    if (wpas_get_sta_statistics(wpa_s, sta_addr, (u8 *)&sta_statistics) < 0) {
        wpa_printf(MSG_DEBUG, "CTRL_IFACE GET_STA_STATISTICS: command failed");
        return -1;
    }
    len = print_sta_statistics(wpa_s, &sta_statistics, 0x00, buf, buflen);

    return len;

}

int wpa_driver_nl80211_driver_cmd(void *priv, char *cmd, char *buf,
                  size_t buf_len )
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;
    struct ifreq ifr;
    android_wifi_priv_cmd priv_cmd;
    struct wpa_supplicant *wpa_s;
    struct hostapd_data *hapd;
    int ret = -1;
    int handled = 0;
    union wpa_event_data event;

    if (drv == NULL) {
        wpa_printf(MSG_ERROR, "%s: drv is NULL. Exiting", __func__);
        return -1;
    }
    if (drv->ctx == NULL) {
        wpa_printf(MSG_ERROR, "%s: drv->ctx is NULL. Exiting", __func__);
        return -1;
    }

    if (os_strcmp(bss->ifname, "ap0") == 0) {
        hapd = (struct hostapd_data *)(drv->ctx);
    }
    else {
        wpa_s = (struct wpa_supplicant *)(drv->ctx);
        if (wpa_s->conf == NULL) {
            wpa_printf(MSG_ERROR, "%s: wpa_s->conf is NULL. Exiting", __func__);
            return -1;
        }
    }

    wpa_printf(MSG_DEBUG, "iface %s recv cmd %s", bss->ifname, cmd);
    handled = 1;

    if (os_strncasecmp(cmd, "POWERMODE ", 10) == 0) {
        int state;
        state = atoi(cmd + 10);
        wpa_printf(MSG_DEBUG, "POWERMODE=%d", state);
    }  else if (os_strncasecmp(cmd, "GET_STA_STATISTICS ", 19) == 0) {
        ret = wpa_driver_get_sta_statistics(wpa_s, cmd + 19, buf, buf_len);
    }  else if (os_strncmp(cmd, "MACADDR", os_strlen("MACADDR"))==0) {
        u8 macaddr[ETH_ALEN] = {};
        os_memcpy(&macaddr, wpa_s->own_addr, ETH_ALEN);
        ret = snprintf(buf, buf_len, "Macaddr = " MACSTR "\n", MAC2STR(macaddr));
        wpa_printf(MSG_DEBUG, "%s", buf);
    } else if(os_strncasecmp(cmd, "COUNTRY", os_strlen("COUNTRY"))==0) {
        wpa_printf(MSG_INFO, "set country: %s", cmd+8);
        //ret = wpa_drv_set_country(wpa_s, cmd+8);
        ret = wpa_driver_mediatek_set_country(priv, cmd+8);
    } else if (os_strcasecmp(cmd, "start") == 0) {
        if (ret = linux_set_iface_flags(drv->global->ioctl_sock,
                                       drv->first_bss->ifname, 1)) {
            wpa_printf(MSG_INFO, "nl80211: Could not set interface UP, ret=%d \n", ret);
        } else {
            wpa_msg(drv->ctx, MSG_INFO, "CTRL-EVENT-DRIVER-STATE STARTED");
        }
    } else if (os_strcasecmp(cmd, "stop") == 0) {
        if (drv->associated) {
            ret = wpa_drv_deauthenticate(wpa_s, drv->bssid, WLAN_REASON_DEAUTH_LEAVING);
            if (ret != 0)
                wpa_printf(MSG_DEBUG,"DRIVER-STOP error, ret=%d", ret);
        } else {
            wpa_printf(MSG_INFO, "nl80211: not associated, no need to deauthenticate \n");
        }

        if (ret = linux_set_iface_flags(drv->global->ioctl_sock,
                                       drv->first_bss->ifname, 0)) {
            wpa_printf(MSG_INFO, "nl80211: Could not set interface Down, ret=%d \n", ret);
        } else {
            wpa_msg(drv->ctx, MSG_INFO, "CTRL-EVENT-DRIVER-STATE STOPPED");
        }
    } else if (os_strncasecmp(cmd, "getpower", 8) == 0) {
        u32 mode;
        if (ret == 0) {
            ret = snprintf(buf, buf_len, "powermode = %u\n", mode);
            wpa_printf(MSG_DEBUG, "%s", buf);
            if (ret < (int)buf_len) {
                return( ret );
            }
        }
    } else if (os_strncasecmp(cmd, "get-rts-threshold", 17) == 0) {
        u32 thd;
        if (ret == 0) {
            ret = snprintf(buf, buf_len, "rts-threshold = %u\n", thd);
            wpa_printf(MSG_DEBUG, "%s", buf);
            if (ret < (int)buf_len)
                return ret;
        }
    } else if (os_strncasecmp(cmd, "set-rts-threshold", 17) == 0) {
        u32 thd = 0;
        char *cp = cmd + 17;
        char *endp;
        if (*cp != '\0') {
            thd = (u32)strtol(cp, &endp, 0);
        }
    } else if (os_strncasecmp(cmd, "rxfilter-add", 12) == 0 ) {
        unsigned long sw_cmd = 0x9F000000;
        unsigned long idx = 0;
        char *cp = cmd + 12;
        char *endp;

        if (*cp != '\0') {
            idx = (u32)strtol(cp, &endp, 0);
            if (endp != cp) {
                idx += 0x00900200;
                wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
                ret = 0;
            }
        }
    } else if (os_strncasecmp(cmd, "rxfilter-remove", 15) == 0 ) {
        unsigned long sw_cmd = 0x9F000000;
        unsigned long idx = 0;
        char *cp = cmd + 15;
        char *endp;

        if (*cp != '\0') {
            idx = (u32)strtol(cp, &endp, 0);
            if (endp != cp) {
                idx += 0x00900300;
                wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
                ret = 0;
            }
        }
    } else if (os_strncasecmp(cmd, "rxfilter-stop", 13) == 0 ) {
        unsigned long sw_cmd = 0x9F000000;
        unsigned long idx = 0x00900000;
        wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
        ret = 0;
    } else if (os_strncasecmp(cmd, "rxfilter-start", 14) == 0 ) {
        unsigned long sw_cmd = 0x9F000000;
        unsigned long idx = 0x00900100;
        wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
        ret = 0;
    } else if (os_strcasecmp(cmd, "btcoexscan-start") == 0) {
        ret = 0; /* mt5921 linux driver not implement yet */
    } else if (os_strcasecmp(cmd, "btcoexscan-stop") == 0) {
        ret = 0; /* mt5921 linux driver not implement yet */
    } else if( os_strncasecmp(cmd, "btcoexmode", 10) == 0 ) {
        ret = 0; /* mt5921 linux driver not implement yet */
    } else if (os_strncasecmp(cmd, "smt-rate", 8) == 0 ) {
        unsigned long sw_cmd = 0xFFFF0123;
        unsigned long idx = 0;
        char *cp = cmd + 8;
        char *endp;

        if (*cp != '\0') {
            idx = (u32)strtol(cp, &endp, 0);
            if (endp != cp) {
                wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
                ret = 0;
            }
        }
    } else if (os_strncasecmp(cmd, "smt-test-on", 11) == 0 ) {
        unsigned long sw_cmd = 0xFFFF1234;
        unsigned long idx = 0;
        wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
        ret = 0;
    } else if (os_strncasecmp(cmd, "smt-test-off", 12) == 0 ) {
        unsigned long sw_cmd = 0xFFFF1235;
        unsigned long idx = 0;
        wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
        ret = 0;
    }
    else {
        handled = 0;
        wpa_printf(MSG_INFO,"Unsupported command");
    }

    if(handled == 0) {

        os_memcpy(buf, cmd, strlen(cmd) + 1);
        memset(&ifr, 0, sizeof(ifr));
        memset(&priv_cmd, 0, sizeof(priv_cmd));
        os_strlcpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);

        priv_cmd.buf = buf;
        priv_cmd.used_len = buf_len;
        priv_cmd.total_len = buf_len;
        ifr.ifr_data = &priv_cmd;

        if ((ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr)) < 0) {
            wpa_printf(MSG_ERROR, "%s: failed to issue private commands\n", __func__);
            wpa_driver_send_hang_msg(drv);
        } else {

            wpa_printf(MSG_DEBUG, "%s: ret = %d used = %u total = %u",
                            __func__, ret , priv_cmd.used_len, priv_cmd.total_len);

            drv_errors = 0;
            ret = 0;
                    ret = priv_cmd.used_len;
                if ((os_strncasecmp(cmd, "WLS_BATCHING", 12) == 0))
                        ret = strlen(buf);
            if (os_strncasecmp(cmd, "SETBAND", 7) == 0) {
                wpa_printf(MSG_INFO, "%s: Unsupported command SETBAND\n",__func__);
                    }
            wpa_printf(MSG_INFO, "%s: buf = %s ret = %d used = %u total = %u",
                            __func__, buf, ret , priv_cmd.used_len, priv_cmd.total_len);
        }
    }

    return ret;
}

int wpa_driver_set_p2p_noa(void *priv, u8 count, int start, int duration)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;

    wpa_printf(MSG_DEBUG, "iface %s P2P_SET_NOA %d %d %d, ignored", bss->ifname, count, start, duration);
    return -1;
}

int wpa_driver_get_p2p_noa(void *priv, u8 *buf, size_t len)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;

    wpa_printf(MSG_DEBUG, "iface %s P2P_GET_NOA, ignored", bss->ifname);
    return -1;
}

int wpa_driver_set_p2p_ps(void *priv, int legacy_ps, int opp_ps, int ctwindow)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;

    wpa_printf(MSG_DEBUG, "iface %s P2P_SET_PS, ignored", bss->ifname);
    return -1;
}

int wpa_driver_set_ap_wps_p2p_ie(void *priv, const struct wpabuf *beacon,
                 const struct wpabuf *proberesp,
                 const struct wpabuf *assocresp)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;

    wpa_printf(MSG_DEBUG, "iface %s set_ap_wps_p2p_ie, ignored", bss->ifname);
    return 0;
}

