/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef RIL_OEM_H
#define RIL_OEM_H 1

// TODO: requestSetMute
#define MTK_REQUEST_SET_MUTE(data,datalen,token) \
        requestSetMute(data,datalen,token)
// TODO: requestGetMute
#define MTK_REQUEST_GET_MUTE(data,datalen,token) \
        requestGetMute(data,datalen,token)
// TODO: requestResetRadio
#define MTK_REQUEST_RESET_RADIO(data,datalen,token) \
        requestResetRadio(data,datalen,token)
// TODO: requestOemHookRaw
#define MTK_REQUEST_OEM_HOOK_RAW(data,datalen,token) \
        requestOemHookRaw(data,datalen,token)
#define MTK_REQUEST_OEM_HOOK_STRINGS(data,datalen,token) \
        requestOemHookStrings(data,datalen,token)
// TODO: requestScreenState
#define MTK_REQUEST_SCREEN_STATE(data,datalen,token) \
        requestScreenState(data,datalen,token)

// TODO: requestSetMute
extern void requestSetMute(void * data, size_t datalen, RIL_Token t);
// TODO: requestGetMute
extern void requestGetMute(void * data, size_t datalen, RIL_Token t);
// TODO: requestResetRadio
extern RIL_Errno resetRadio();
extern void requestResetRadio(void * data, size_t datalen, RIL_Token t);
// TODO: requestOemHookRaw
extern void requestOemHookRaw(void * data, size_t datalen, RIL_Token t);
extern void requestOemHookStrings(void * data, size_t datalen, RIL_Token t);
// TODO: requestScreenState
extern void requestScreenState(void * data, size_t datalen, RIL_Token t);

extern int queryMainProtocol();
extern ApplyRadioCapabilityResult applyRadioCapability(RIL_RadioCapability* rc, RIL_Token t);
extern void requestGetRadioCapability(void * data, size_t datalen, RIL_Token t);
extern void requestSetRadioCapability(void * data, size_t datalen, RIL_Token t);

//TBD, remove
extern int get3GCapabilitySim(RIL_Token t);
extern void set3GCapability(RIL_Token t, int setting);
extern void requestConfigSimSwitch(void * data, size_t datalen, RIL_Token t);
extern void requestGetImei(void * data, size_t datalen, RIL_Token t);
extern void requestGetImeisv(void * data, size_t datalen, RIL_Token t);
extern void requestBasebandVersion(void * data, size_t datalen, RIL_Token t);
extern int getSimSwitchMode(int ccci_sys_fd);
extern void setCardSlot(int cardSlot);
extern int getCardSlot();
extern void setExternalModemSlot(int externalModemSlot);
extern int isSimSwitchSwapped();
extern void flightModeBoot();
extern void bootupGetIccid(RIL_SOCKET_ID rid);
extern void bootupSetRadio(RIL_SOCKET_ID rid);
extern void bootupGetImei(RIL_SOCKET_ID rid);
extern void bootupGetImeisv(RIL_SOCKET_ID rid);
extern void bootupGetBasebandVersion(RIL_SOCKET_ID rid);
extern void bootupGetCalData(RIL_SOCKET_ID rid);
extern void requestMobileRevisionAndIMEI(void * data, size_t datalen, RIL_Token t);
extern void requestSetTrm(void * data, size_t datalen, RIL_Token t);
extern void requestSN(RIL_SOCKET_ID rid);
extern void requestGetGcfMode(RIL_SOCKET_ID rid);
extern void requestGetCalibrationData(void * data, size_t datalen, RIL_Token t);

extern int triggerCCCIIoctlEx(int request, int *param);
extern int triggerCCCIIoctl(int request);
extern int getMappingSIMByCurrentMode(RIL_SOCKET_ID rid);
extern void upadteSystemPropertyByCurrentMode(int rid, char* key1, char* key2, char* value);
extern void requestStoreModem(void *data, size_t datalen, RIL_Token t);
extern void requestQueryModem(void *data, size_t datalen, RIL_Token t);
extern void requestReloadModem(void *data, size_t datalen, RIL_Token t);
extern void setActiveModemType(int mdType);
extern int getActiveModemType();
extern void requestQueryThermal(void *data, size_t datalen, RIL_Token t);
extern void requestSetImsEnabled(void * data, size_t datalen, RIL_Token t);
extern void onImsEnabling(int enable, RIL_SOCKET_ID rid);
extern void onImsEnabled(int enable, RIL_SOCKET_ID rid);
extern void onAbnormalEvent(char *urc, const RIL_SOCKET_ID rid);
extern void onSwitchModemCauseInformation(char *urc, const RIL_SOCKET_ID rid);

extern int rilOemMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilOemUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);
extern int handleOemUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);
extern void requestSetModemThermal(void * data, size_t datalen, RIL_Token t);

// M: Epdg feature, long format for screen on, short format for scree off
extern int mal_set_ps_format(int long_format);

void updateCFUQueryType(const char *cfuType);
void initCFUQueryType();

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

//SIM mode for internal md
#define SIM_SWITCH_MODE_SINGLE_TALK_MDSYS       (1)
#define SIM_SWITCH_MODE_SINGLE_TALK_MDSYS_LITE  (2)
#define SIM_SWITCH_MODE_DUAL_TALK               (3)
#define SIM_SWITCH_MODE_DUAL_TALK_SWAP          (4)

//SIM Switch type
#define SIM_SWITCH_TYPE_INTERNAL (0)
#define SIM_SWITCH_TYPE_EXTERNAL (1)

//SIM Switch mode property key
#define PROPERTY_SIM_SWITCH_MODE "ril.simswitch.mode"
#define PROPERTY_SWITCH_MODEM_CAUSE_TYPE "ril.switch.modem.cause.type"
#define PROPERTY_SWITCH_MODEM_CAUSE_TYPE_DEFAULT_VALUE "255"

#define PROPERTY_SWITCH_MODEM_DELAY_INFO "ril.switch.modem.delay.info"
#define PROPERTY_SWITCH_MODEM_DELAY_INFO_DEFAULT_VALUE "\"FFFFFF\",255,0"


#define GEMINI_SIM_1 (0)
#define GEMINI_SIM_2 (1)
#define GEMINI_SIM_3 (2)
#define GEMINI_SIM_4 (3)

#define ENV_MAGIC     'e'
#define ENV_READ        _IOW(ENV_MAGIC, 1, int)
#define ENV_WRITE         _IOW(ENV_MAGIC, 2, int)
#define BUF_MAX_LEN 40
#define SETTING_QUERY_CFU_TYPE "persist.radio.cfu.querytype"

struct env_ioctl
{
    char *name;
    int name_len;
    char *value;
    int value_len;
};

typedef enum {
    md_type_invalid = 0,
    modem_wg = 3,
    modem_tg,
    modem_lwg,
    modem_ltg,
    modem_ltng
} e_modem_type_t;

#endif /* RIL_OEM_H */
