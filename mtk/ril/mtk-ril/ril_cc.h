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

#ifndef RIL_CC_H
#define RIL_CC_H 1

typedef enum {
    GSM,
    IMS,
} dispatch_flg;

typedef enum {
    none,
    mo,
    mt,
    multi,
    conf,
    pending_mo,
} volte_call_state;

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilCcUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

extern void requestGetCurrentCalls(void * data, size_t datalen, RIL_Token t);
extern void requestDial(void * data, size_t datalen, RIL_Token t, int isEmergency);
extern void requestHangup(void * data, size_t datalen, RIL_Token t);
extern void requestHangupWaitingOrBackground(void * data, size_t datalen, RIL_Token t);
extern void requestHangupForegroundResumeBackground(void * data, size_t datalen, RIL_Token t);
extern void requestSwitchWaitingOrHoldingAndActive(void * data, size_t datalen, RIL_Token t);
extern void requestAnswer(void * data, size_t datalen, RIL_Token t);
extern void requestConference(void * data, size_t datalen, RIL_Token t);
extern void requestUdub(void * data, size_t datalen, RIL_Token t);
extern void requestSeparateConnection(void * data, size_t datalen, RIL_Token t);
extern void requestExplicitCallTransfer(void * data, size_t datalen, RIL_Token t);
extern void requestLastCallFailCause(void * data, size_t datalen, RIL_Token t);
extern void requestDtmf(void * data, size_t datalen, RIL_Token t);

/// M: Using MTK proprietary AT command
extern void requestDtmfStart(void * data, size_t datalen, RIL_Token t);
extern void requestDtmfStop(void * data, size_t datalen, RIL_Token t);
extern void requestSetTTYMode(void * data, size_t datalen, RIL_Token t);

extern void requestHangupAll(void * data, size_t datalen, RIL_Token t);
extern void requestForceReleaseCall(void * data, size_t datalen, RIL_Token t);
extern void onCallProgressInfoCallStateChange(char *urc, RIL_SOCKET_ID rid);
extern void onSpeechInfo(char *urc, RIL_SOCKET_ID rid);
extern void onIncomingCallIndication(char *urc, RIL_SOCKET_ID rid);
extern void requestSetCallIndication(void * data, size_t datalen, RIL_Token t);
extern void onSpeechCodecInfo(char *urc, RIL_SOCKET_ID rid);
extern void requestSetSpeechCodecInfo(void *data, size_t datalen, RIL_Token t);
extern void requestGetCcm(void * data, size_t datalen, RIL_Token t);
extern void requestGetAcm(void * data, size_t datalen, RIL_Token t);
extern void requestGetAcmMax(void * data, size_t datalen, RIL_Token t);
extern void requestGetPpuAndCurrency(void * data, size_t datalen, RIL_Token t);
extern void requestSetAcmMax(void * data, size_t datalen, RIL_Token t);
extern void requestResetAcm(void * data, size_t datalen, RIL_Token t);
extern void requestSetPpuAndCurrency(void * data, size_t datalen, RIL_Token t);
extern void requestSetDataCentric(void * data, size_t datalen, RIL_Token t);
extern void onCipherIndication(char *urc, RIL_SOCKET_ID rid);
extern void onSsacBarringInfo(char *urc, const RIL_SOCKET_ID rid);
extern void onSipCallProgressIndicator(char *urc, const RIL_SOCKET_ID rid);
extern void onAdsReport(char *urc, const RIL_SOCKET_ID rid);

/// M: Set service category before making Emergency call. @{
extern void onEmergencyCallServiceCategoryIndication(char *s, RIL_SOCKET_ID rid);
extern void requestSetEccServiceCategory(void *data, size_t datalen, RIL_Token t);
/// @}

/// M: Set ecc list before MO call. @{
extern void requestSetEccList(void *data, size_t datalen, RIL_Token t);
/// @}

/// M: IMS feature. @{
extern void requestAddImsConferenceCallMember(void *data, size_t datalen, RIL_Token t);
extern void requestRemoveImsConferenceCallMember(void *data, size_t datalen, RIL_Token t);
extern void requestDialWithSipUri(void *data, size_t datalen, RIL_Token t);
extern void requestHoldCall(void *data, size_t datalen, RIL_Token t);
extern void requestResumeCall(void *data, size_t datalen, RIL_Token t);
extern void onLteSuppSvcNotification(char *s, RIL_SOCKET_ID rid);
extern void onVoLteEconfSrvccIndication(char *urc, RIL_SOCKET_ID rid);
extern void onImsVopsSupport(char *urc, RIL_SOCKET_ID rid);
extern void onEmbSupport(char *urc, RIL_SOCKET_ID rid);
extern void onVoLteEconfResultIndication(char *urc, RIL_SOCKET_ID rid);
extern void onEpcNetworkFeatureInfoSupport(char *urc, RIL_SOCKET_ID rid);
extern void onSrvccStateNotify(char *urc, RIL_SOCKET_ID rid);
/// @}

/// M: ViLTE feature. @{
extern void onCallModeChangeIndicator(char *urc, const RIL_SOCKET_ID rid);
extern void onVideoCapabilityIndicator(char *urc, const RIL_SOCKET_ID rid);
/// @}

/* Notify modem about IMS call status. */
extern void requestSetImsCallStatus(void *data, size_t datalen, RIL_Token t);

/// M: For 3G VT only @{
extern void requestVtDial(void * data, size_t datalen, RIL_Token t);
extern void requestVoiceAccept(void * data, size_t datalen, RIL_Token t);
extern void requestReplaceVtCall(void * data, size_t datalen, RIL_Token t);
extern void onVtStatusInfo(char *urc, RIL_SOCKET_ID rid);
/// @}

/// M: IMS VoLTE conference dial feature. @{
extern void requestConferenceDial(void * data, size_t datalen, RIL_Token t);
/// @}

/// M: CC071: Add Customer proprietary-IMS RIL interface. @{
/* To notify emergency bearer support indication */
extern void onEmergencyBearerSupportInfo(char *s, RIL_SOCKET_ID rid);
/// @}

/// M: CC072: Add Customer proprietary-IMS RIL interface. @{
/* To transfer IMS call context to modem */
extern void requestSetCallContextTransfer(void *data, size_t datalen, RIL_Token t);
/* To update IMS registration status to modem */
extern void requestUpdateImsRegistrationStatus(void *data, size_t datalen, RIL_Token t);
/// @}

extern void requestSwitchAntenna(void * data, size_t datalen, RIL_Token t);

/// M: IMS ViLTE feature. @{
extern void requestVideoAccept(void * data, size_t datalen, RIL_Token t);
/// @}

#endif /* RIL_CC_H */
