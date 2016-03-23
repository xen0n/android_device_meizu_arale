#include <telephony/ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>

#include "hardware/ccci_intf.h"
#include "netutils/ifc.h"
#include "atchannel.h"
#include "oem-support.h"
#include "ril_callbacks.h"
#include "c2k_ril_data.h"

#ifdef DATA_CHANNEL_CTX
#undef DATA_CHANNEL_CTX
#endif
#define DATA_CHANNEL_CTX getRILChannelCtxFromToken(t)

void initialPdnInfoList(RILChannelCtx *pChannel);
void clearAllPdnInfo();
void clearPdnInfo(PdnInfo* info);
void getL2pValue(RILChannelCtx *pChannel);
int getPdnCapability(RILChannelCtx *pChannel);
int getAvailableCid();

int definePdnContext(const char *requestedApn, int protocol, int availableCid, int authType,
		const char *username, const char* password, RILChannelCtx *pChannel);
int activatePdn(int availableCid, int *pActivatedPdnNum,
		int *pConcatenatedBearerNum, int *activatedCidList, int isEmergency,
		RILChannelCtx *pChannel);
int updatePdnAddressByCid(int cid, RILChannelCtx *pChannel);

int updateDnsAndMtu(RILChannelCtx *pChannel);
int updateActiveStatus(RILChannelCtx *pChannel);
int bindPdnToInterface(int interfaceId, int cid, RILChannelCtx *pChannel);
void bindNetworkInterfaceWithModem(int interfaceId, int modemId);
void updateNetworkInterface(int interfaceId, int mask);
void configureNetworkInterface(int interfaceId, int isUp);

void onMePdnActive(void* urc);
int onPdnModified(void* urc);
int onPdnDeactResult(void* urc);

void initialDataCallResponse(RIL_Data_Call_Response_v11* responses, int length);
void fillDataCallResponse(int interfaceId, RIL_Data_Call_Response_v11* response);
void dumpDataResponse(RIL_Data_Call_Response_v11* response, const char* reason);

void openSocketsForNetworkInterfaceConfig(int interfaceId);
void closeSocketsAfterNetworkInterfaceConfig(int interfaceId);
void setStateOfNwInterface(int s, struct ifreq *ifr, int state, int clr);
void setIpv4AddrToNwInterface(int s, struct ifreq *ifr, const char *addr);
void setIpv6AddrToNwInterface(int s, struct ifreq *ifr, const char *addr);

int convertIpv6Address(char* pdp_addr, char* ipv6_addr, int isLinkLocal);
int getPdnAddressType(char* addr);
int getProtocolType(const char* protocol);
int getAuthType(const char* authTypeStr);
void updatePdnAddressWithAddr(int responseCid, char *addr);

char* getNetworkInterfaceName(int interfaceId);
int isInitialAttachApn(const char *requestedApn, const char * protocol, int authType,
		const char *username, const char* password, RILChannelCtx *pChannel);
void storeDataSystemProperties(int interfaceId);

int queryMatchedPdnWithSameApn(const char* apn);
int apnMatcher(const char* existApn, const char* apn);
int sock_fd_v4[MAX_CCMNI_NUMBER] = { 0 };
int sock_fd_v6[MAX_CCMNI_NUMBER] = { 0 };

// Store the last data call fail cause.
int nLastDataCallFailCause;
char pcL2pValue[MAX_L2P_VALUE_LENGTH] = { 0 };

PdnInfo* pPdnInfo = NULL;
int nMaxPdnSupport = 0;
int nPdnInfoSize = 0;
static const struct timeval TIMEVAL_0 = { 0, 0 };
static const struct timeval TIMEVAL_10 = { 10, 0 };

// [C2K] [IRAT] start
SyncPdnInfo* pSyncPdnInfo = NULL;
int nDeactPdnCount = 0;
int nReactPdnCount = 0;
int nReactSuccCount = 0;

PDN_IRAT_ACTION nIratAction = IRAT_ACTION_UNKNOWN;
PDN_IRAT_TYPE nIratType = IRAT_TYPE_UNKNOWN;
static int nIratDeactCount = 0;

void onIratEvent(const char* urc);
int onPdnSyncFromSourceRat(const char* urc);
void onIratSourceStarted(int sourceRat, int targetRat, int type);
void onIratSourceFinished(int sourceRat, int targetRat, int type);
void onIratTargetStarted(int sourceRat, int targetRat, int type);
void onIratTargetFinished(int sourceRat, int targetRat, int type);
void onIratStateChanged(int sourceRat, int targetRat, int state,
		int isSourceRil);

void clearAllSyncPdnInfo();
void clearSyncPdnInfo(SyncPdnInfo* info);
void cleanupFailedPdns();
void cleanupPdnByCid(int cid);
void cleanupPdnsForFallback();

int getIratType(int sourceRat, int targetRat);
void onIratPdnReactSucc(int cid);
void handleDeactedOrFailedPdns();
void deactivatePdnByCid(int cid);
void retryForFailedPdnByCid(int cid);

int rebindPdnToInterface(int interfaceId, int cid, RILChannelCtx * pChannel);
void reconfigureNetworkInterface(int interfaceId);
void setNetworkTransmitState(int interfaceId, int state);
void resumeAllDataTransmit();
void suspendAllDataTransmit();
void onUpdateIratStatus();
void onResetIratStatus();

extern int s_md3_off;

inline int isEmulatorRunning()
{
	return 0;
}

void testPdnDeactUrc()
{
	// Test PDN deact.
	char *pcTestUrc = calloc(1, 255);
	strcpy(pcTestUrc, "+CGEV: ME PDN DEACT 1");
	onPdnDeactResult(pcTestUrc);
}

void testPdnModifyUrc()
{
	// Test PDN deact.
	char *pcTestUrc = calloc(1, 255);
	strcpy(pcTestUrc, "+CGEV: NW MODIFY 1,99");
	onPdnModified(pcTestUrc);
}

void testIratForTarget()
{
	char *pcTestUrc = calloc(1, 255);
	strcpy(pcTestUrc, "+EI3GPPIRAT:2,4,3");
	onIratEvent(pcTestUrc);

	memset(pcTestUrc, 0, 255);
	strcpy(pcTestUrc,
			"+EGCONTRDP:1,\"ctnet\",1,0,\"140.113.1.1\", \"32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251\"");
	onPdnSyncFromSourceRat(pcTestUrc);

	memset(pcTestUrc, 0, 255);
	strcpy(pcTestUrc, "+EGCONTRDP:2,\"ctwap\",2,0,\"142.113.1.1\"");
	onPdnSyncFromSourceRat(pcTestUrc);

	memset(pcTestUrc, 0, 255);
	strcpy(pcTestUrc, "+EI3GPPIRAT:4,4,2");
	onIratEvent(pcTestUrc);
}

void testIratForSource()
{
	char *pcTestUrc = calloc(1, 255);
	strcpy(pcTestUrc, "+EI3GPPIRAT:1,2,4");
	onIratEvent(pcTestUrc);

	memset(pcTestUrc, 0, 255);
	strcpy(pcTestUrc, "+EI3GPPIRAT:3,2,4");
	onIratEvent(pcTestUrc);
}

void rilDataInitialization(RILChannelCtx *pChannel)
{
	initialPdnInfoList(pChannel);
	getL2pValue(pChannel);
}

// Need to be called when system init.
void initialPdnInfoList(RILChannelCtx *pChannel)
{
	if (pPdnInfo == NULL) {
		nPdnInfoSize = getPdnCapability(pChannel) + 1; // +1 is for cid0 which is used in Irat.
		pPdnInfo = calloc(1, nPdnInfoSize * sizeof(PdnInfo));
		clearAllPdnInfo();
		LOGD("[RILData_Init] Pdn information is initialized, size = %d.", nPdnInfoSize);
	}
}

void clearPdnInfo(PdnInfo* info)
{
	memset(info, 0, sizeof(PdnInfo));
	info->active = DATA_STATE_INACTIVE;
	info->interfaceId = INVALID_CID;
	info->primaryCid = INVALID_CID;
	info->cid = INVALID_CID;
	info->bearerId = INVALID_CID;
	info->ddcId = INVALID_CID;
	info->mtu = -1;
}

void clearAllPdnInfo()
{
	if (pPdnInfo != NULL) {
		int i = 0;
		for (i = 0; i < nPdnInfoSize; i++) {
			clearPdnInfo(&pPdnInfo[i]);
		}
	}
}

int getPdnCapability(RILChannelCtx *pChannel)
{
	if (isEmulatorRunning()) {
		LOGI("Emulator run, getPdnCapability return 8");
		return 8;
	}

	ATResponse *p_response = NULL;
	int err = 0;

	char *line = NULL;
	ATLine *p_cur = NULL;
	int max_pdn_support_23G = 0;
	int max_pdn_support_4G = 0;
	int max_pdn_support_c2k = 0;

	err = at_send_command_singleline("AT+EGPCAP=?", "+EGPCAP:", &p_response,
			pChannel);
	// Resoponse: +EGPCAP:<Max PDP supported by 2/3G>, <Max PDN supported by 4G>, <Max PDN supported by c2k>
	// Resoponse: +EGPCAP:0,0,8
	if (isATCmdRspErr(err, p_response)) {
		LOGI("Not supoprt PDN capability command.");
	} else {
		line = p_response->p_intermediates->line;

		err = at_tok_start(&line);
		if (err < 0) {
			goto error;
		}

		err = at_tok_nextint(&line, &max_pdn_support_23G);
		if (err < 0) {
			goto error;
		}

		err = at_tok_nextint(&line, &max_pdn_support_4G);
		if (err < 0) {
			goto error;
		}
		err = at_tok_nextint(&line, &max_pdn_support_c2k);
		if (err < 0) {
			goto error;
		}
	}
	AT_RSP_FREE(p_response);
	LOGD("[RILData_Init] getPdnCapability: PDN capability is [ %d, %d, %d ]", max_pdn_support_23G,
			max_pdn_support_4G, max_pdn_support_c2k);
	return max_pdn_support_c2k;

error:
	AT_RSP_FREE(p_response);

	if (isEmulatorRunning()) {
		max_pdn_support_23G = 0;
		max_pdn_support_4G = 0;
		max_pdn_support_c2k = 1;
	}
	LOGE("[RILData_Init] getPdnCapability failed, current PDP capability [%d, %d %d]",
			max_pdn_support_23G, max_pdn_support_4G, max_pdn_support_c2k);
	return max_pdn_support_c2k;
}

void getL2pValue(RILChannelCtx *pChannel)
{
	if (isEmulatorRunning()) {
		LOGI("Emulator run, getL2pValue return CCMNI.");
		sprintf(&pcL2pValue, "%s", L2P_VALUE_CCMNI);
		return 8;
	}
	ATResponse *p_response = NULL;
	char* line = NULL;
	char* out;
	int err = 0;
	int bSupportCcmni = 0;
	err = at_send_command_singleline("AT+CGDATA=?", "+CGDATA:", &p_response,
			pChannel);
	// Expect response: +CGDATA:"C-CCMNI"
	if (err < 0 || p_response->success == 0) {
		LOGE("AT+CGDATA=? response error %d.", err);
		goto error;
	}
	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);
	if (err < 0) {
		goto error;
	}

	while (at_tok_hasmore(&line)) {
		err = at_tok_nextstr(&line, &out);
		if (err < 0) {
			goto error;
		}

		if (strcmp(L2P_VALUE_CCMNI, out) == 0) {
			bSupportCcmni = 1;
			break;
		}
	}
	AT_RSP_FREE(p_response);

	if (bSupportCcmni == 1) {
		sprintf(&pcL2pValue, "%s", L2P_VALUE_CCMNI);
	} else {
		// This should never happen because C2K need to use CCMNI as default network interface.
		sprintf(&pcL2pValue, "%s", L2P_VALUE_CCMNI);
	}
	LOGD("[RILData_Init] L2P Value: %s", pcL2pValue);
	return;

error:
	AT_RSP_FREE(p_response);
	sprintf(&pcL2pValue, "%s", L2P_VALUE_CCMNI);
	LOGD("[RILData_Init] Get L2P value fail, set L2P value as %s", pcL2pValue);
	return;
}

// Get network interface name, return cc3mni for pure C2K project, return ccmni with Irat feature support.
char* getNetworkInterfaceName(int interfaceId) {
    char* ret = "";
    if (isCdmaIratSupport()) {
        int curSim = getCdmaSocketSlotId();
        int majorSim = get4GCapabilitySlot();
        LOGD("getNetworkInterfaceName: curSim = %d, majorSim = %d.", curSim,
                majorSim);
        if (curSim == majorSim) {
            ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS1);
        } else {
            ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS3);
        }
    } else {
        ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS3);
    }
    LOGD("getNetworkInterfaceName: interface %d's name = %s", interfaceId, ret);
    return ret;
}

int getAvailableCid()
{
	int i = 1;
	//start from index 1 since we should not select CID0
	//CID0 is for network attach given PDN connection
	for (i = 1; i < nPdnInfoSize; i++) {
		LOGD("getAvailableCid i=%d, nPdnInfoSize = %d, pPdnInfo[i].interfaceId = %d, pPdnInfo[i].active = %d.",
				i, nPdnInfoSize, pPdnInfo[i].interfaceId, pPdnInfo[i].active);
		if (pPdnInfo[i].interfaceId == INVALID_CID
				&& pPdnInfo[i].active == DATA_STATE_INACTIVE) {
			LOGD("The available CID is %d.", i);
			return i;
		}
	}

	return INVALID_CID;
}

void requestSetupDataCall(void * data, size_t datalen, RIL_Token t)
{
	/* In CDMA version: data[0] is radioType(GSM/UMTS or CDMA), data[1] is profile,
	 * data[2] is apn, data[3] is username, data[4] is passwd, data[5] is authType,
	 * data[6] is cid, data[7] is intercaceId.
	 */
	const char* profile = ((const char **) data)[1];
	const char* apn = ((const char **) data)[2];
	const char* username = ((const char **) data)[3];
	const char* password = ((const char **) data)[4];
	const int authType = getAuthType(((const char **) data)[5]);
	const int protocol = getProtocolType(((const char **) data)[6]);
	const int interfaceId = atoi(((const char **) data)[7]) - 1;
	LOGD("[RILData_Setup] requestSetupData with interfaceId=%d and apn=%s......",
			interfaceId, apn);

    //Test code
    //testIratForTarget();
    //testIratForSource();

	char *cmd = NULL;
	int err = 0;
	ATResponse *p_response = NULL;
	char *line = NULL;
	char *out = NULL;
	ATLine *p_cur = NULL;
	int isEmergency = 0;
	int i = 0;

    RIL_Data_Call_Response_v11* response = (RIL_Data_Call_Response_v11*) calloc(1,
            sizeof(RIL_Data_Call_Response_v11));
	initialDataCallResponse(response, 1);

	// Activate PDN connection.
	int activatedPdnNum = 0; // Activated PDN number, should be 1 for C2K currently since there is no IPv4v6 fallback case.
	int concatenatedBearerNum = 0; // Activated concatenated bearer number, may be 0 for C2K.
    // Acivated cids of PDN connections.
    int *activatedCidList = (int *) calloc(1, nPdnInfoSize * sizeof(int));

    // Reuse the previous activated PDP in case phone process crash but RILD
    // and MD still maintain a PDP context, If we want to reactivate a new PDP,
    // it will fails on HRPD network because C2K only support single PDP on HRPD.
    int matchedCid = queryMatchedPdnWithSameApn(apn);
    if (matchedCid != INVALID_CID) {
        activatedPdnNum = 1;
        activatedCidList[0] = matchedCid;
        LOGD("[RILData_Setup] requestSetupData reuse cid %d.", matchedCid);
    } else {
        // Get available CID, define PDN context and configure protocal settings.
        int availableCid = getAvailableCid();
        LOGD("[RILData_Setup] requestSetupData with availableCid=%d.",
                availableCid);

        if (0 == definePdnContext(apn, protocol, availableCid, authType,
                        username, password, DATA_CHANNEL_CTX)) {
            goto error;
        }

        for (i = 0; i < nPdnInfoSize; i++) {
            activatedCidList[i] = INVALID_CID;
        }
        if (0 == activatePdn(availableCid, &activatedPdnNum,
                        &concatenatedBearerNum, activatedCidList, isEmergency,
                        DATA_CHANNEL_CTX)) {
            goto error;
        }
    }
	for (i = 0; i < activatedPdnNum; i++) {
		int cid = activatedCidList[i];
		if (pPdnInfo[cid].isDedicateBearer) {
			LOGD("[RILData_Setup] Skip update PDP address of CID%d since it is a dedicate bearer [active=%d]",
					cid, pPdnInfo[cid].active);
		} else {
			if (updatePdnAddressByCid(cid, DATA_CHANNEL_CTX) < 0) {
				goto error;
			}
			if (0 == bindPdnToInterface(interfaceId, cid, DATA_CHANNEL_CTX)) {
				goto error;
			}
		}
	}

	if (updateDnsAndMtu(DATA_CHANNEL_CTX) < 0) {
		goto error;
	}

    bindNetworkInterfaceWithModem(interfaceId, MD_SYS3);

	configureNetworkInterface(interfaceId, ENABLE_CCMNI);
	storeDataSystemProperties(interfaceId);
    // Store APN name when PDN activated successfully.
    for (i = 0; i < activatedPdnNum; i++) {
        sprintf(pPdnInfo[activatedCidList[i]].apn, "%s", apn);
    }

	fillDataCallResponse(interfaceId, response);
	dumpDataResponse(response, "requestSetupDataCall dump response");

	RIL_onRequestComplete(t, RIL_E_SUCCESS, response,
            sizeof(RIL_Data_Call_Response_v11));
	goto finish;

error:
	LOGE("[RILData_Setup] requestSetupDataCall failed: activatedPdnNum = %d.", activatedPdnNum);
	if (activatedCidList != NULL) {
		for (i = 0; i < activatedPdnNum; i++) {
			clearPdnInfo(&pPdnInfo[activatedCidList[i]]);
		}
	}

	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

finish:
	if (response != NULL) {
		freeDataResponse(response);
		free(response);
	}

	if (activatedCidList != NULL) {
		free(activatedCidList);
	}
	LOGD("[RILData_Setup] requestSetupDataCall finished.");
	return;
}

void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t)
{
	char *cmd;
	int err;
	ATResponse *p_response = NULL;
	int i = 0;
	int interfaceId = atoi(((const char **) data)[0]);
	LOGD("[RILData_Deact] requestDeactiveDataCall interfaceId=%d", interfaceId);

	// AT+CGACT=<state>,<cid>;  <state>:0-deactivate;1-activate
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].interfaceId == interfaceId) {
			asprintf(&cmd, "AT+CGACT=0,%d", i);
			err = at_send_command_notimeout(cmd, &p_response, DATA_CHANNEL_CTX);
			free(cmd);
			if (isATCmdRspErr(err, p_response)) {
				int errCode = at_get_cme_error(p_response);
				if (errCode == PS_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED) {
					pPdnInfo[i].active = DATA_STATE_NEED_DEACT;
				}
				goto error;
			}
			AT_RSP_FREE(p_response);
		}
	}

	configureNetworkInterface(interfaceId, DISABLE_CCMNI);
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].interfaceId == interfaceId) {
			clearPdnInfo(&pPdnInfo[i]);
		}
	}
	storeDataSystemProperties(interfaceId);

	LOGD("[RILData_Deact] requestDeactiveDataCall succeed----->.");

	//response deactivation result first then do re-attach
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	return;

error:
	LOGD("[RILData_Deact] requestDeactiveDataCall failed - XXXXXX.");
	configureNetworkInterface(interfaceId, DISABLE_CCMNI);
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	AT_RSP_FREE(p_response);
}

void requestAllowData(void* data, size_t datalen, RIL_Token t)
{
	int isDataAttach = ((int *) data)[0];
	LOGD("[RILData_C2K_Util] requestAllowData isDataAttach= %x", isDataAttach);

	if (isDataAttach == REQUEST_DATA_ATTACH
			|| isDataAttach == REQUEST_DATA_ATTACH_NOT_RESPONSE) {
		requestDataAttach(data, datalen, t);
	} else {
		requestDataIdle(data, datalen, t);
	}

	// For C2K RIL, response detach or attach directly.
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	LOGD("[RILData_C2K_Util] requestAllowData finish.");
}

void requestDataAttach(void* data, size_t datalen, RIL_Token t)
{
    LOGD("[RILData_C2K_Util] requestDataAttach");
    doDataAttach(DATA_CHANNEL_CTX);
}

void requestDataIdle(void* data, size_t datalen, RIL_Token t)
{
	char *cmd;
	int err;
	ATResponse *p_response = NULL;
	int isDataAttach = ((int *) data)[0];
	LOGD("[RILData_C2K_Util] requestDataIdle isDataAttach= %x", isDataAttach);

	asprintf(&cmd, "AT+CGATT=0");
	err = at_send_command(cmd, &p_response, DATA_CHANNEL_CTX);

	if (isATCmdRspErr(err, p_response)) {
		LOGD("[RILData_C2K_Util] requestDataIdle fail with CGATT, err = "
						+ err);
	}
	AT_RSP_FREE(p_response);
	free(cmd);
}

int get4GCapabilitySlot()    {
    char tmp[PROPERTY_VALUE_MAX] = { 0 };
    int simId = 0;

    property_get(PROPERTY_4G_SIM, tmp, "1");
    simId = atoi(tmp);
    return simId;
}

void doDataAttach(RILChannelCtx *p_channel) {
    char *cmd;
    int err;
    ATResponse *p_response = NULL;

    asprintf(&cmd, "AT+CGATT=1");
    err = at_send_command(cmd, &p_response, p_channel);
    LOGD("[RILData_C2K_Util] sendDataAttach AT+CGATT=1.");
    if (isATCmdRspErr(err, p_response)) {
        LOGD("[RILData_C2K_Util] sendDataAttach fail with CGATT, err = " + err);
    }
    AT_RSP_FREE(p_response);
    free(cmd);
}

void combineDataAttach(RILChannelCtx *p_channel) {
    if (isCdmaLteDcSupport()) {
        int curSim = getCdmaSocketSlotId();
        int defaultSim = getDefaultDataSim();
        int capSim = get4GCapabilitySlot();
        LOGD("[RILData_C2K] SVLTE allowDataRequest: capSim = %d, curSim = %d, defaultSim = %d.",
            capSim, curSim, defaultSim);
        // OP09/Solution1 LC+G mode, data on sim 2, this case must ignore.
        if (isC2kSimInsert() && defaultSim == SIM_ID_2 && capSim == SIM_ID_1) {
            LOGD("[RILData_C2K] allowDataRequest ignored.");
            return;
        }
    }
    LOGD("[RILData_C2K] allowDataRequest.");
    doDataAttach(p_channel);
    if (isC2kSimInsert()) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_ALLOWED, NULL, 0);
    }
}

int isC2kSimInsert() {
    int isInsert = 0;
    if (isCdmaLteDcSupport()) {
        char svlte_iccid_c2k[PROPERTY_VALUE_MAX] = { 0 };
        property_get(PROPERTY_SVLTE_SIM1_ICCID_C2K, svlte_iccid_c2k, "");
        LOGD("[isC2kSimInsert] SVLTE C2K iccid = %s.", svlte_iccid_c2k);
        if (strlen(svlte_iccid_c2k) == 0 || strcmp("N/A", svlte_iccid_c2k) == 0) {
            LOGD("[isC2kSimInsert] SVLTE SIM not inserted");
            isInsert = 0;
        } else {
            isInsert = 1;
        }
    } else {
        LOGE("[isC2kSimInsert] not support non svlte");
    }
    LOGD("[isC2kSimInsert] Svlte SIM inserted status = %d", isInsert);
    return isInsert;
}

int getDataEnable()
{
    int ret = 0;
    char dataEnable[PROPERTY_VALUE_MAX] = {0};

    property_get(PROPERTY_MOBILE_DATA_ENABLE, dataEnable, "");
    if(strlen(dataEnable) == 0) {
        LOGD("[RILData_C2K] Data enable not set");
        return 0;
    }

    ret = atoi(dataEnable);
    LOGD("[RILData_C2K] getDataEnable %s, return %d", dataEnable, ret);

    return ret;
}

int getDefaultDataSim()
{
    int i = 0;
    char iccid[PROPERTY_VALUE_MAX] = {0};
    char defaultData[PROPERTY_VALUE_MAX] = {0};

    property_get(PROPERTY_RIL_DATA_ICCID, defaultData, "");
    if (strlen(defaultData) == 0 || strcmp("N/A", defaultData) == 0) {
        LOGD("[RILData_C2K] Default Data not set");
        return 0;
    }

    LOGD("[RILData_C2K] ICCID of Default Data is %s, SIM_COUNT = %d.",
            defaultData, getSimCount());

    for (i = 0; i < getSimCount(); i++) {
        property_get(PROPERTY_ICCID_SIM[i], iccid, "");
        if (strlen(iccid) == 0 || strcmp("N/A", iccid) == 0) {
            LOGD("[RILData_C2K] SIM%d not inserted", i);
            continue;
        }
        if (strStartsWith(iccid, defaultData)) {
            LOGD("[RILData_C2K] Default data is SIM%d, ICCID = %s", (i + 1), iccid);
            return (i + 1);
        }
    }
    LOGD("[RILData_C2K] Not found");
    return 0;
}

//TODO: implement for specific fail causes.
void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t)
{
	RIL_onRequestComplete(t, RIL_E_SUCCESS, &nLastDataCallFailCause,
			sizeof(nLastDataCallFailCause));
}

void requestDataCallList(void * data, size_t datalen, RIL_Token t)
{
	requestOrSendDataCallList(t);
}

void requestOrSendDataCallList(RIL_Token t)
{
	ATResponse *p_response = NULL;
	ATLine *p_cur = NULL;
	int err = 0;
	char *out = NULL;
	int i = 0;
    RIL_Data_Call_Response_v11* responsesOfActive = NULL;
	int activeCount = 0;
    RIL_Data_Call_Response_v11* responses = (RIL_Data_Call_Response_v11*) calloc(1,
            MAX_CCMNI_NUMBER * sizeof(RIL_Data_Call_Response_v11));
	initialDataCallResponse(responses, MAX_CCMNI_NUMBER);

	RILChannelCtx *pChannel = NULL;
	if (t == NULL) {
		pChannel = getChannelCtxbyId(DATA_CHANNEL);
	} else {
		pChannel = getRILChannelCtxFromToken(t);
	}

	updateActiveStatus(pChannel);

	for (i = 0; i < nPdnInfoSize; i++) {
		updatePdnAddressByCid(i, pChannel);
		// Not to check error since we expect to query all PDN connections even if error occurred
	}

	if (updateDnsAndMtu(pChannel) < 0) {
		LOGE("[RILData_Query] requestOrSendDataCallList: updateDnsAndMtu failed - XXXXXX");
		goto error;
	}

	for (i = 0; i < MAX_CCMNI_NUMBER; i++) {
		fillDataCallResponse(i, &responses[i]);
		if (responses[i].active == DATA_STATE_ACTIVE) {
			++activeCount;
		}
	}

	LOGD("[RILData_Query] requestOrSendDataCallList: activeCount = %d", activeCount);
	if (activeCount > 0) {
        responsesOfActive = (RIL_Data_Call_Response_v11*) calloc(1,
                activeCount * sizeof(RIL_Data_Call_Response_v11));

		int index = 0;
		for (i = 0; i < MAX_CCMNI_NUMBER; i++) {
			if (responses[i].active == DATA_STATE_ACTIVE) {
				memcpy(&responsesOfActive[index], &responses[i],
                        sizeof(RIL_Data_Call_Response_v11));
				++index;
				dumpDataResponse(&responses[i], "requestOrSendDataCallList dump interface status");
			}
		}
	} else {
		LOGD("[RILData_Query] requestOrSendDataCallList no active response");
	}

	if (t) {
		RIL_onRequestComplete(t, RIL_E_SUCCESS,
				activeCount > 0 ? responsesOfActive : NULL,
                activeCount * sizeof(RIL_Data_Call_Response_v11));
	} else {
		RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
				activeCount > 0 ? responsesOfActive : NULL,
                activeCount * sizeof(RIL_Data_Call_Response_v11));
	}
	goto finish;

error:
	LOGE("[RILData_Query] requestOrSendDataCallList failed - XXXXXX.");
	if (t != NULL) {
		RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	} else {
		RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
	}

	AT_RSP_FREE(p_response);

finish:
	if (responses != NULL) {
		for (i = 0; i < MAX_CCMNI_NUMBER; i++) {
			freeDataResponse(&responses[i]);
		}
		free(responses);
	}

	if (responsesOfActive != NULL) {
		LOGE("requestOrSendDataCallList free responsesOfActive");
		free(responsesOfActive);
	}
	return;
}

void onDataCallListChanged()
{
	requestOrSendDataCallList(NULL);
}

void onMePdnActive(void* urc)
{
	// Initial attached APN activated, keep although no initail attached APN in C2K.
	char *p = strstr(urc, ME_PDN_URC);
	if (p == NULL) {
		LOGD("[RILData_C2K_IRAT] onMePdnActive invalid URC.");
		return;
	}
	p = p + strlen(ME_PDN_URC);
	int activatedCid = atoi(p);
	LOGD("[RILData_C2K_IRAT] onMePdnActive: CID = %d, state = %d, IRAT state = %d.",
			activatedCid, pPdnInfo[activatedCid].active, nIratAction);

	if (nIratAction == IRAT_ACTION_TARGET_STARTED) {
		onIratPdnReactSucc(activatedCid);
	} else {
		if (pPdnInfo[activatedCid].active == DATA_STATE_INACTIVE) {
			pPdnInfo[activatedCid].active = DATA_STATE_LINKDOWN; // Update with link down state.
			pPdnInfo[activatedCid].cid = activatedCid;
			pPdnInfo[activatedCid].primaryCid = activatedCid;
		}
	}
	free(urc);
}

int onPdnModified(void* urc)
{
	int err = 0;
	int cid = INVALID_CID;
	int cause = -1;
	char *line = urc;

	//+CGEV: NW MODIFY <cid>, <cause>, 0, ex:+CGEV: NW MODIFY 1,99,0
	LOGD("[RILData_URC] onPdnModified receive %s", line);
	err = at_tok_start(&line);
	if (err < 0) {
		LOGD("[RILData_URC] onPdnModified return with error %d.", err);
		goto error;
	}

	if ((strlen(line) > 0)
			&& (strncmp(NW_MODIFY_URC, line, strlen(NW_MODIFY_URC)) == 0)) {
		line += strlen(NW_MODIFY_URC);
		err = at_tok_nextint(&line, &cid);
		if (err < 0) {
			LOGE("ERROR occurs when parsing cid.");
			goto error;
		}

		if (at_tok_hasmore(&line)) {
			err = at_tok_nextint(&line, &cause);
			if (err < 0) {
				LOGE("ERROR occurs when parsing cause.");
				goto error;
			}
			if (cause == MODIFY_CAUSE_PPP_RENEGOTIATION) {
				RILChannelCtx *pChannel = getChannelCtxbyId(DATA_CHANNEL);
				if (updatePdnAddressByCid(cid, pChannel) < 0) {
					LOGE("ERROR when updating PDN address caused by NW modify.");
					goto error;
				}
				if (updateDnsAndMtu(pChannel) < 0) {
					LOGE("ERROR when updating DNS caused by NW modify.");
					goto error;
				}

				updateNetworkInterface(pPdnInfo[cid].interfaceId, RESET_ALL_ADDRESSES);

                if (s_md3_off) {
					RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
				} else {
				    RIL_requestProxyTimedCallback(onDataCallListChanged, NULL,
				            &TIMEVAL_0, DATA_CHANNEL);
				}
			}
		} else {
			LOGE("ERROR because no modify cause.");
			goto error;
		}
	} else {
		LOGD("[RILData_URC] onPdnModified return because invalid URC.");
		goto error;
	}

error:
	free(urc);
	return 0;
}

int onPdnDeactResult(void* urc)
{
	int err = 0;
	int primaryCid = INVALID_CID;
	int cid = INVALID_CID;
	char *line = urc;

	//C2K: +CGEV: NW PDN DEACT <cid>
	//C2K: +CGEV: ME PDN DEACT <cid>

	LOGD("[RILData_URC] onPdnDeactResult receive %s", line);
	err = at_tok_start(&line);
	if (err < 0) {
		LOGD("onPdnDeactResult return with error %d.", err);
		goto error;
	}
	int isDedicateBearer = 0;

	if (strlen(line) > 0) {
		LOGD("onPdnDeactResult token start [%s]", line);

		if (strncmp(ME_PDN_DEACT_URC, line, strlen(ME_PDN_DEACT_URC)) == 0) {
			line += strlen(ME_PDN_DEACT_URC);
		} else if (strncmp(NW_PDN_DEACT_URC, line, strlen(NW_PDN_DEACT_URC))
				== 0) {
			line += strlen(NW_PDN_DEACT_URC);
		}
		LOGD("onPdnDeactResult skip prefix [%s]", line);

		err = at_tok_nextint(&line, &primaryCid); //cid or primary cid
		if (err < 0) {
			LOGE("ERROR occurs when parsing first parameter");
			goto error;
		}

		if (at_tok_hasmore(&line)) {
			err = at_tok_nextint(&line, &cid); //cid
			if (err < 0) {
				LOGE("ERROR occurs when parsing cid");
				goto error;
			}
		} else {
			cid = primaryCid;
		}
		LOGD("[RILData_URC] onPdnDeactResult: primaryCid=%d, cid=%d", primaryCid, cid);

		isDedicateBearer = pPdnInfo[cid].isDedicateBearer;
		int i = 0;
		int hasSameInterfaceCid = 0;
		int interfaceId = pPdnInfo[cid].interfaceId;
		if (interfaceId == INVALID_CID) {
			LOGD("[RILData_URC] onPdnDeactResult goto error because of INVALID interfaceId=%d ", interfaceId);
			goto error;
		}

		// If there is another default PDN is with the same interface Id
		// with the deactivated PDN, disable the network interface here.
		// The is mainly for IPv4v6 fallback cause two different PDN with same interface case.
		for (i = 0; i < nPdnInfoSize; i++) {
			if (!pPdnInfo[i].isDedicateBearer
					&& pPdnInfo[i].interfaceId != INVALID_CID
					&& pPdnInfo[i].interfaceId == interfaceId
					&& pPdnInfo[i].primaryCid != cid) {
				LOGD("CID%d has the same interfaceId with CID%d", i, cid);
				hasSameInterfaceCid = 1;
				break;
			}
		}
		if (!hasSameInterfaceCid) {
			LOGD("Deactviate PDN CID/interfaceId is [%d, %d]", cid,
					interfaceId);
			configureNetworkInterface(interfaceId, DISABLE_CCMNI);
		}

		// Clear dedicate PDN which attached to the deactivated PDN.
		for (i = 0; i < nPdnInfoSize; i++) {
			if (pPdnInfo[i].isDedicateBearer && pPdnInfo[i].primaryCid == cid) {
				LOGD("Clean dedicate pdn CID%d info due to default pdn CID%d deactivated",
						pPdnInfo[i].cid, cid);
				clearPdnInfo(&pPdnInfo[i]);
			}
		}
		clearPdnInfo(&pPdnInfo[cid]);
        if (s_md3_off) {
			RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
		} else {
		    RIL_requestProxyTimedCallback(onDataCallListChanged, NULL, &TIMEVAL_0, DATA_CHANNEL);
		}
	} else {
		LOGD("[RILData_URC] onPdnDeactResult return because invalid URC.");
		goto error;
	}
	free(urc);
	return isDedicateBearer;

error:
	free(urc);
	return -1;
}

void initialDataCallResponse(RIL_Data_Call_Response_v11* responses, int length)
{
	int i = 0;
	for (i = 0; i < length; i++) {
        memset(&responses[i], 0, sizeof(RIL_Data_Call_Response_v11));
		responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
		responses[i].cid = INVALID_CID;
	}
}

void fillDataCallResponse(int interfaceId, RIL_Data_Call_Response_v11* response)
{
	int activatedPdnNum = 0;
	int i = 0, nDnsesLen = 0;
	char addressV4[MAX_IPV4_ADDRESS_LENGTH] = { 0 };
	char addressV6[MAX_IPV6_ADDRESS_LENGTH] = { 0 };
	char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH] = { { 0 } };
	char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = { { 0 } };

	int v4DnsLength = 0;
	int v6DnsLength = 0;

	response->active = DATA_STATE_INACTIVE;
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].interfaceId == interfaceId) {
			//There is cid bind to the interface of the response, so set to active
			response->active = pPdnInfo[i].active;
			asprintf(&response->ifname, "%s", getNetworkInterfaceName(interfaceId));

			if (strlen(pPdnInfo[i].addressV4) > 0)
				strcpy(addressV4, pPdnInfo[i].addressV4);
			if (strlen(pPdnInfo[i].addressV6) > 0)
				strcpy(addressV6, pPdnInfo[i].addressV6);

			int j = 0;
			for (j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
				if (strlen(pPdnInfo[i].dnsV4[j]) > 0) {
					strcpy(dnsV4[j], pPdnInfo[i].dnsV4[j]);
					v4DnsLength += strlen(pPdnInfo[i].dnsV4[j]);
					if (j != 0)
						++v4DnsLength; //add one space
				}
				if (strlen(pPdnInfo[i].dnsV6[j]) > 0) {
					strcpy(dnsV6[j], pPdnInfo[i].dnsV6[j]);
					v6DnsLength += strlen(pPdnInfo[i].dnsV6[j]);
					if (j != 0 || v4DnsLength != 0)
						++v6DnsLength; //add one space
				}
			}

            response->mtu = pPdnInfo[i].mtu;
		}
	}

	response->status = PDP_FAIL_NONE;
	response->cid = interfaceId;

	int addressV4Length = strlen(addressV4);
	int addressV6Length = strlen(addressV6);

	if (addressV4Length > 0 && addressV6Length > 0) {
		asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV4V6);
		asprintf(&response->addresses, "%s %s", addressV4, addressV6);
	} else if (addressV6Length > 0) {
		asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV6);
		asprintf(&response->addresses, "%s", addressV6);
	} else {
		asprintf(&response->type, SETUP_DATA_PROTOCOL_IP);
		asprintf(&response->addresses, "%s", addressV4);
	}
	asprintf(&response->gateways, "%s", response->addresses);

	nDnsesLen = v4DnsLength + v6DnsLength + 1;
	response->dnses = calloc(1, nDnsesLen * sizeof(char));
	memset(response->dnses, 0, nDnsesLen * sizeof(char));

	for (i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER; i++) {
		if (v4DnsLength > 0 && strlen(dnsV4[i]) > 0) {
			int currentLength = strlen(response->dnses);
			if (currentLength > 0)
				strcat(response->dnses + currentLength, " ");

			strcat(response->dnses + strlen(response->dnses), dnsV4[i]);
		}

		if (v6DnsLength > 0 && strlen(dnsV6[i]) > 0) {
			int currentLength = strlen(response->dnses);
			if (currentLength > 0)
				strcat(response->dnses + currentLength, " ");

			strcat(response->dnses + strlen(response->dnses), dnsV6[i]);
		}
	}
}

void freeDataResponse(RIL_Data_Call_Response_v11* response)
{
	LOGD("freeDataResponse: E type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s",
			response->type, response->ifname, response->addresses,
			response->dnses, response->gateways);

	FREEIF(response->type);
	FREEIF(response->ifname);
	FREEIF(response->addresses);
	FREEIF(response->gateways);
	FREEIF(response->dnses);

	LOGD("freeDataResponse: X ");
}

void dumpDataResponse(RIL_Data_Call_Response_v11* response, const char* reason)
{
	LOGD("%s data call response: status=%d, suggestedRetryTime=%d, interfaceId=%d, active=%d, \
            type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s",
			reason == NULL ? "dumpDataResponse" : reason, response->status,
			response->suggestedRetryTime, response->cid, response->active,
			response->type, response->ifname, response->addresses,
			response->dnses, response->gateways);
}

void openSocketsForNetworkInterfaceConfig(int interfaceId)
{
	sock_fd_v4[interfaceId] = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd_v4[interfaceId] < 0) {
		sock_fd_v4[interfaceId] = -errno; /* save errno for later */
		LOGD("Couldn't create IP socket: errno=%d", errno);
	} else {
		LOGD("Allocate sock_fd=%d, for cid=%d", sock_fd_v4[interfaceId],
				interfaceId + 1);
	}

	sock_fd_v6[interfaceId] = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock_fd_v6[interfaceId] < 0) {
		sock_fd_v6[interfaceId] = -errno; /* save errno for later */
		LOGD("Couldn't create IPv6 socket: errno=%d", errno);
	} else {
		LOGD("Allocate sock6_fd=%d, for cid=%d", sock_fd_v6[interfaceId],
				interfaceId + 1);
	}
}

void closeSocketsAfterNetworkInterfaceConfig(int interfaceId)
{
	close(sock_fd_v4[interfaceId]);
	close(sock_fd_v6[interfaceId]);
	sock_fd_v4[interfaceId] = 0;
	sock_fd_v6[interfaceId] = 0;
}

void setStateOfNwInterface(int socket, struct ifreq *ifr, int state, int clr)
{
	LOGD("[RILData_Util] Configure network state: %d.", state);
	if (ioctl(socket, SIOCGIFFLAGS, ifr) < 0) {
		LOGD("setStateOfNwInterface failed 0 - %x.", ifr->ifr_flags);
		goto error;
	}
	ifr->ifr_flags = (ifr->ifr_flags & (~clr)) | state;
	if (ioctl(socket, SIOCSIFFLAGS, ifr) < 0) {
		LOGD("setStateOfNwInterface failed 1 - %x.", ifr->ifr_flags);
		goto error;
	}
	return;

error:
	LOGD("setStateOfNwInterface failed - XXXXXX: errno = %x.", errno);
	return;
}

void setIpv4AddrToNwInterface(int socket, struct ifreq *ifr, const char *addr)
{
	LOGD("[RILData_Util] Configure IPv4 adress: %s", addr);
	struct sockaddr_in *sin = (struct sockaddr_in *) &ifr->ifr_addr;
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = inet_addr(addr);
	if (ioctl(socket, SIOCSIFADDR, ifr) < 0) {
		LOGD("setIpv4AddrToNwInterface failed - XXXXXX.");
	}
}

void setIpv6AddrToNwInterface(int s, struct ifreq *ifr, const char *addr)
{
	LOGD("[RILData_Util] Configure IPv6 adress: %s", addr);
	struct in6_ifreq ifreq6;
	int ret = 0;

	ret = ioctl(s, SIOCGIFINDEX, ifr);
	if (ret < 0) {
		goto error;
	}

	inet_pton(AF_INET6, addr, &ifreq6.ifr6_addr);
	ifreq6.ifr6_prefixlen = 64;
	ifreq6.ifr6_ifindex = ifr->ifr_ifindex;
	ret = ioctl(s, SIOCSIFADDR, &ifreq6);
	if (ret < 0) {
		goto error;
	}
	return;

error:
	LOGE("setIpv6AddrToNwInterface failed: %d - %d:%s", ret, errno,
			strerror(errno));
	return;

}

int definePdnContext(const char *requestedApn, int protocol, int availableCid, int authType,
		const char *username, const char* password, RILChannelCtx *pChannel)
{
	int nRet = 1; //success
	int err = 0;
	ATResponse *p_response = NULL;
	char *cmd = NULL;
	int emergency_ind = 0; //0:normal, 1:emergency
	int pcscf_discovery_flag = 0;
	int signalingFlag = 0;
	const char *pszIPStr;

	//+CGDCONT=[<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>[,<IPv4AddrAlloc>[,<emergency_indication>
	//[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>]]]]]]]]]]
	if (protocol == IPV4V6) {
		pszIPStr = SETUP_DATA_PROTOCOL_IPV4V6;
	} else if (protocol == IPV6) {
		pszIPStr = SETUP_DATA_PROTOCOL_IPV6;
	} else {
		pszIPStr = SETUP_DATA_PROTOCOL_IP;
	}

	asprintf(&cmd, "AT+CGDCONT=%d,\"%s\",\"%s\",,0,0,0,%d,%d,%d", availableCid,
			pszIPStr, requestedApn, emergency_ind, pcscf_discovery_flag,
			signalingFlag);

	err = at_send_command(cmd, &p_response, pChannel);
	free(cmd);
	if (isATCmdRspErr(err, p_response)) {
		if (p_response != NULL && p_response->success == 0) {
			nLastDataCallFailCause = at_get_cme_error(p_response);
			LOGD("[RILData_Setup] AT+CGDCONT err code = %0x", nLastDataCallFailCause);
		}
		nRet = 0; //failed
	}
	AT_RSP_FREE(p_response);

	int isIaApn = isInitialAttachApn(requestedApn, pszIPStr, authType, username, password, pChannel);
	// set protocol configuration in PDP
	/* Use AT+CGPRCO=<cid>,<user_name>,<passwd>,<DNS1>,<DNS2>,<authType>,<req_v6dns>,<req_v4dns>,<isIA> */
	if (AUTHTYPE_NOT_SET == authType) {
		asprintf(&cmd, "AT+CGPRCO=%d,\"\",\"\",\"\",\"\",0,1,0,%d", availableCid, isIaApn);
	} else {
		asprintf(&cmd, "AT+CGPRCO=%d,\"%s\",\"%s\",\"\",\"\",%d,1,0,%d",
				availableCid, username, password, authType, isIaApn);
	}
	err = at_send_command(cmd, NULL, pChannel);
	free(cmd);
	LOGD("[RILData_Setup] definePdnContext finish: isIaApn = %d, nRet = %d.", isIaApn, nRet);
	return nRet;
}

int activatePdn(int availableCid, int *pActivatedPdnNum,
		int *pConcatenatedBearerNum, int *activatedCidList, int isEmergency,
		RILChannelCtx *pChannel)
{
	int nRet = 0; //0:failed, 1: success
	char *cmd = NULL;
	int err = 0;
	ATResponse *p_response = NULL;
	char *line = NULL, *out = NULL;
	ATLine *p_cur = NULL;
	int activatedPdnNum = 0;
	int concatenatedBearerNum = 0;

	*pActivatedPdnNum = activatedPdnNum;
	*pConcatenatedBearerNum = concatenatedBearerNum;

	LOGD("[RILData_Setup] activatePdn start: availableCid = %d, isEmergency = %d.",
			availableCid, isEmergency);

	/* Use AT+CGACT=1,cid to activate PDP context indicated via this cid */
	asprintf(&cmd, "AT+CGACT=1,%d", availableCid);
	if (isEmulatorRunning()) {
		//Emulator do not support +CGEV: ME PDN ACT X intermediates
		err = at_send_command(cmd, &p_response, pChannel);
		free(cmd);
		if (isATCmdRspErr(err, p_response)) {
			goto error;
		}

		activatedCidList[activatedPdnNum] = availableCid;
		pPdnInfo[availableCid].primaryCid = availableCid;
		pPdnInfo[availableCid].cid = availableCid;
		pPdnInfo[availableCid].active = DATA_STATE_LINKDOWN;
		pPdnInfo[availableCid].isEmergency = isEmergency;
		activatedPdnNum++;
	} else {
		//default bearer (primary): +CGEV: ME PDN ACT <cid>
		//dedicate bearer (secondary): +CGEV: ME PDN ACT <pri_cid> <cid>
		err = at_send_command_multiline_notimeout(cmd, "+CGEV: ME PDN ACT ", &p_response,
				pChannel);
        free(cmd);
        if (isATCmdRspErr(err, p_response)) {
            goto error;
        }

		for (p_cur = p_response->p_intermediates; p_cur != NULL;
				p_cur = p_cur->p_next) {
			line = p_cur->line;
			err = at_tok_start(&line);
			/* line => +CGEV: ME PDN ACT X */
			if (err < 0) {
				goto error;
			}

			LOGD("[RILData_Setup] activatePdn line = %s, len = %d", line, strlen(ME_PDN_URC));
			if (strStartsWith(line, ME_PDN_URC)) {
				char *firstParam = line + strlen(ME_PDN_URC);
				char *secondParam = strstr(firstParam, " ");
				LOGD("[RILData_Setup] activatePdn firstParam = %s, secondParam = %s",
						firstParam, secondParam);
				if (firstParam == NULL) {
					goto error;
				}

				int activatedCid = INVALID_CID;
				int primaryCid = INVALID_CID;
				if (secondParam == NULL) {
					activatedCid = atoi(firstParam);
					primaryCid = activatedCid;
					pPdnInfo[activatedCid].isDedicateBearer = 0;
					//PDP is activated and does not bind to a network interface, set to link down
					pPdnInfo[activatedCid].active = DATA_STATE_LINKDOWN;
					LOGD("[RILData_Setup] [%s] default bearer activated [CID=%d]", __FUNCTION__,
							activatedCid);
				} else {
					primaryCid = atoi(firstParam);
					activatedCid = atoi(secondParam + 1);
					pPdnInfo[activatedCid].isDedicateBearer = 1;
					//The active status of a dedicate bearer should be the same as default bearer
					pPdnInfo[activatedCid].active = pPdnInfo[primaryCid].active;
					++concatenatedBearerNum; //add concatenate counting
					LOGD("[RILData_Setup] [%s] concatenated dedicate bearer activated [CID=%d, primaryCID=%d]",
							__FUNCTION__, activatedCid, primaryCid);
				}

				activatedCidList[activatedPdnNum] = activatedCid;
				pPdnInfo[activatedCid].primaryCid = primaryCid;
				pPdnInfo[activatedCid].cid = activatedCid;
				pPdnInfo[availableCid].isEmergency = isEmergency;
				++activatedPdnNum;
			}
		}
	}

	LOGD("[RILData_Setup] [%s] %d PDN activated (%d concatenated)", __FUNCTION__,
			activatedPdnNum, concatenatedBearerNum);

	if (isATCmdRspErr(err, p_response) || !activatedPdnNum) {
		if (p_response->success == 0) {
			nLastDataCallFailCause = at_get_cme_error(p_response);
			LOGD("[RILData_Setup] AT+CGACT: err code = %0x", nLastDataCallFailCause);
		}
		goto error;
	}

	nRet = 1;
	*pActivatedPdnNum = activatedPdnNum;
	*pConcatenatedBearerNum = concatenatedBearerNum;
	AT_RSP_FREE(p_response);

	LOGD("[RILData_Setup] [%s] succeed----->.", __FUNCTION__);
	return nRet;

error:
	AT_RSP_FREE(p_response);
	LOGE("[RILData_Setup] [%s] failed - XXXXXX.", __FUNCTION__);
	return nRet;
}

int updateActiveStatus(RILChannelCtx *pChannel)
{
	ATResponse *p_response = NULL;
	ATLine *p_cur;
	int maxCidSupported = 0;

	int err = at_send_command_multiline("AT+CGACT?", "+CGACT:", &p_response,
			pChannel);
	if (isATCmdRspErr(err, p_response)) {
		LOGE("[RILData_Query] updateActiveStatus AT+CGACT? failed - XXXXXX.");
		AT_RSP_FREE(p_response);
		return 0;
	}

	for (p_cur = p_response->p_intermediates; p_cur != NULL;
			p_cur = p_cur->p_next) {
		maxCidSupported++;
	}

	LOGD("[RILData_Query] updateActiveStatus PDN capability [%d, %d]", maxCidSupported,
			nPdnInfoSize);

	for (p_cur = p_response->p_intermediates; p_cur != NULL;
			p_cur = p_cur->p_next) {
		char *line = p_cur->line;

		err = at_tok_start(&line);
		if (err < 0) {
			LOGD("[RILData_Query] updateActiveStatus ERROR 0: errno = %d.", err);
			break;
		}

		int responseCid = 0;
		err = at_tok_nextint(&line, &responseCid);
		if (err < 0) {
			LOGD("[RILData_Query] updateActiveStatus ERROR 1: errno = %d.", err);
			break;
		}

		err = at_tok_nextint(&line, &pPdnInfo[responseCid].active);
		if (err < 0) {
			LOGD("[RILData_Query] updateActiveStatus ERROR 2: errno =  %d.", err);
			break;
		}

		if (pPdnInfo[responseCid].active > 0) {
			// 0=inactive, 1=active/physical link down, 2=active/physical link up
			if (pPdnInfo[responseCid].interfaceId == INVALID_CID) {
				pPdnInfo[responseCid].active = DATA_STATE_LINKDOWN;
				LOGD("[RILData_Query] updateActiveStatus CID%d is linkdown", responseCid);
			} else {
				pPdnInfo[responseCid].active = DATA_STATE_ACTIVE;
				LOGD("[RILData_Query] updateActiveStatus CID%d is active", responseCid);
			}
		} else {
			LOGD("[RILData_Query] updateActiveStatus CID%d is inactive", responseCid);
		}
	}
	AT_RSP_FREE(p_response);

	// This may not be real max supported CID, should use getPdnCapability to know PDN capability
	return maxCidSupported;
}

int updatePdnAddressByCid(int cid, RILChannelCtx *pChannel)
{
	int err = -1;
	char *line = NULL, *out = NULL;
	char *cmd = NULL;
	ATResponse *p_response = NULL;
	int responseCid = -1;

	/* Use AT+CGPADDR=cid to query the ip address assigned to this PDP context indicated via this cid */
	asprintf(&cmd, "AT+CGPADDR=%d", cid);
	err = at_send_command_singleline(cmd, "+CGPADDR:", &p_response, pChannel);
	free(cmd);

	/* Parse the response to get the ip address */
	if (isATCmdRspErr(err, p_response)) {
		goto error;
	}
	line = p_response->p_intermediates->line;

//For test
//#if(TC==2)
//	strcpy(line, "+CGPADDR:1,\"140.113.1.1\", \"32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251\"");
//#elif(TC==1)
//	strcpy(line, "+CGPADDR:1,\"32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251\"");
//#elif(TC==3)
//	if(cid == 1) {
//		//sprintf(line, "+CGPADDR:%d,\"32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251\"", cid);
//		sprintf(line, "+CGPADDR:%d,\"140.113.1.1\"", cid);
//	} else {
//		//sprintf(line, "+CGPADDR:%d,\"140.113.1.1\"", cid);
//		sprintf(line, "+CGPADDR:%d,\"32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251\"", cid);
//	}
//#elif(TC==4)
//	strcpy(line, "+CGPADDR:1,\"140.113.1.4\"");
//#elif(TC==5)
//	strcpy(line, "+CGPADDR:1,\"32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251\"");
//#endif

	err = at_tok_start(&line);
	/* line => +CGPADDR: <cid>,<PDP_addr_1>,<PDP_addr_2> */

	if (err < 0) {
		LOGD("[RILData_Query] updatePdnAddressByCid ERROR 0: errno = %d.", err);
		goto error;
	}

	/* Get 1st parameter: CID */
	err = at_tok_nextint(&line, &responseCid);
	if (err < 0) {
		LOGD("[RILData_Query] updatePdnAddressByCid ERROR 1: errno = %d.", err);
		goto error;
	}

	/* Get 2nd parameter: IPAddr1 */
	err = at_tok_nextstr(&line, &out);
	if (err < 0) {
		LOGD("[RILData_Query] updatePdnAddressByCid ERROR 2: errno = %d.", err);
		goto error;
	}
	updatePdnAddressWithAddr(responseCid, out);

	/* Only for IPv4V6 + dual stack PDP context */
	/* Get 3rd paramter: IPaddr 2*/
	if (at_tok_hasmore(&line)) {
		err = at_tok_nextstr(&line, &out);
		if (err < 0) {
			LOGD("[RILData_Query] updatePdnAddressByCid ERROR 3: errno = %d.", err);
			goto error;
		}
		updatePdnAddressWithAddr(responseCid, out);
	}
	AT_RSP_FREE(p_response);

	LOGD("[RILData_Query] getPdnAdrrByCid cid= %d, ipv4 = %s, ipv6 = %s.", cid,
			pPdnInfo[responseCid].addressV4, pPdnInfo[responseCid].addressV6);
	return 0;

error:
	/* Free the p_response for +CGPADDR: */
	LOGD("[RILData_Query] Update PDP address of CID%d failed - XXXXXX.", cid);
	AT_RSP_FREE(p_response);
	return -1;
}

void updatePdnAddressWithAddr(int responseCid, char *addr)
{
	if (getPdnAddressType(addr) == IPV4) {
		sprintf(pPdnInfo[responseCid].addressV4, "%s", addr);
	} else {
		convertIpv6Address(addr, pPdnInfo[responseCid].addressV6, 1);
	}
}

int updateDnsAndMtu(RILChannelCtx *pChannel)
{
	char *line = NULL;
	char *out = NULL;
	char *cmd = NULL;
	ATLine *p_cur = NULL;
	ATResponse *p_response = NULL;

	//Use AT+CGPRCO? to query each cid's dns server address and MTU
	//+CGPRCO:<cid>,"<dnsV4-1>","<dnsV4-2>","<dnsV6-1>","<dnsV6-2>", <MTU>
	int err = at_send_command_multiline("AT+CGPRCO?", "+CGPRCO:", &p_response,
			pChannel);
	if (isATCmdRspErr(err, p_response)) {
		LOGD("[RILData_Query] updateDnsAndMtu ERROR 0.");
		goto error;
	}

	int i = 0;
	for (i = 0; i < nPdnInfoSize; i++) {
		//clear DNS information in CID table since we would query it again
		memset(pPdnInfo[i].dnsV4, 0, sizeof(pPdnInfo[i].dnsV4));
		memset(pPdnInfo[i].dnsV6, 0, sizeof(pPdnInfo[i].dnsV6));
	}

	for (p_cur = p_response->p_intermediates; p_cur != NULL;
			p_cur = p_cur->p_next) {
		line = p_cur->line;
		err = at_tok_start(&line);

		if (err < 0) {
			LOGD("[RILData_Query] updateDnsAndMtu ERROR 1: errno = %d.", err);
			goto error;
		}

		/* Get 1st parameter: CID */
		int responseCid = -1;
		err = at_tok_nextint(&line, &responseCid);
		if (err < 0) {
			LOGD("[RILData_Query] updateDnsAndMtu ERROR 2: errno = %d.", err);
			goto error;
		}

		int v4Count = 0;
		int v6Count = 0;
		for (i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER * 2; i++) {
			if (!at_tok_hasmore(&line)) {
				break;
			}

			err = at_tok_nextstr(&line, &out);
			if (err < 0) {
				goto error;
			}

			if (out != NULL && strlen(out) > 0
					&& strcmp(NULL_IPV4_ADDRESS, out) != 0) {
				if (getPdnAddressType(out) == IPV6) {
					convertIpv6Address(out,
							pPdnInfo[responseCid].dnsV6[v6Count], 0);
					LOGD("[RILData_Query] updateDnsAndMtu DNS%d V6 for cid%d %s", v6Count + 1,
							responseCid, pPdnInfo[responseCid].dnsV6[v6Count]);
					++v6Count;
				} else {
					strcpy(pPdnInfo[responseCid].dnsV4[v4Count], out);
					LOGD("[RILData_Query] updateDnsAndMtu DNS%d V4 for cid%d %s", v4Count + 1,
							responseCid, pPdnInfo[responseCid].dnsV4[v4Count]);
					++v4Count;
				}
			}
		}

		// C2K network may send MTU configure from operator network, like Verizon.
		err = at_tok_nextint(&line, &pPdnInfo[responseCid].mtu);
		LOGD("[RILData_Query] updateDnsAndMtu mtu = %d.", pPdnInfo[responseCid].mtu);
		if (err < 0) {
			LOGD("[RILData_Query] Error happens when updateing MTU for cid %d.", responseCid);
			goto error;
		}
	}

	AT_RSP_FREE(p_response);
	return 0;

error:
	LOGE("[RILData_Query] updateDnsAndMtu failed - XXXXXX.");
	AT_RSP_FREE(p_response);
	return -1;
}

int bindPdnToInterface(int interfaceId, int cid, RILChannelCtx *pChannel)
{
	int nRet = 1; //0:failed, 1: success
	char *cmd = NULL;
	int err = 0;

	LOGD("[RILData_Setup] bindPdnToInterface start E: interfaceId = %d, cid = %d.",interfaceId, cid);
	if (pPdnInfo[cid].interfaceId == INVALID_CID
			|| pPdnInfo[cid].interfaceId != interfaceId) {
		/* AT+CGDATA=<L2P>,<cid>,<channel ID> */
		asprintf(&cmd, "AT+CGDATA=\"%s\",%d,%d", pcL2pValue, cid,
				interfaceId + 1); //The channel id is from 1~n
		err = at_send_command(cmd, NULL, pChannel);
		free(cmd);
		if (err < 0) {
			LOGD("[RILData_Setup] CID%d fail to bind interface%d", cid,
					interfaceId);
			nRet = 0;
			goto error;
		} else {
			//The PDP is bind to a network interface, set to active
			LOGD("[RILData_Setup] CID%d is bind to interface%d", cid,
					interfaceId);
			pPdnInfo[cid].interfaceId = interfaceId;
			pPdnInfo[cid].active = DATA_STATE_ACTIVE;
		}
	} else {
		LOGD("[RILData_Setup] [%s] not to bind interface again, CID%d is already bind to interface%d",
				__FUNCTION__, cid, interfaceId);
	}

	LOGD("[RILData_Setup] bindPdnToInterface succeed----->.");
	return nRet;

error:
	LOGD("[RILData_Setup] bindPdnToInterface failed - XXXXXX, nRet = %d.", nRet);
	return nRet;
}

// Bind CCMNI interface with specified modem.
void bindNetworkInterfaceWithModem(int interfaceId, int modemId)
{
	LOGD("bindNetworkInterfaceWithModem interface %d to modem %d", interfaceId,
			modemId);
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));

	ifc_ccmni_md_cfg(ifr.ifr_name, modemId);
}

// Update network interface because PPP Re-Negotioation / IRAT, the IP and DNS may changed,
// bind the new PDN address and reset data connections.
void updateNetworkInterface(int interfaceId, int mask)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
	LOGD("[RILData_Util] updateNetworkInterface: %s", ifr.ifr_name);
	ifc_reset_connections(ifr.ifr_name, mask);

	openSocketsForNetworkInterfaceConfig(interfaceId);
	int i = 0;
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].interfaceId == interfaceId) {
			if (strlen(pPdnInfo[i].addressV4) > 0 && (mask & 0x01)) {
				setStateOfNwInterface(sock_fd_v4[interfaceId], &ifr, IFF_UP, 0);
				setIpv4AddrToNwInterface(sock_fd_v4[interfaceId], &ifr,
						pPdnInfo[i].addressV4);
			}
			if (strlen(pPdnInfo[i].addressV6) > 0 && (mask & 0x02)) {
				setStateOfNwInterface(sock_fd_v6[interfaceId], &ifr, IFF_UP, 0);
				setIpv6AddrToNwInterface(sock_fd_v6[interfaceId], &ifr,
						pPdnInfo[i].addressV6);
			}
		}
	}
	closeSocketsAfterNetworkInterfaceConfig(interfaceId);
}

void configureNetworkInterface(int interfaceId, int isUp)
{
    if (interfaceId == INVALID_CID) {
        LOGD("configureNetworkInterface return directly because of invaild isUp=%d", isUp);
        return;
    }

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
	openSocketsForNetworkInterfaceConfig(interfaceId);

	int i = 0;
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].interfaceId == interfaceId) {
			if (isUp) {
				LOGD("[RILData_Util] configureNetworkInterface %s with %s(v4) and %s(v6)",
						ifr.ifr_name, pPdnInfo[i].addressV4, pPdnInfo[i].addressV6);
				if (strlen(pPdnInfo[i].addressV4) > 0) {
					setStateOfNwInterface(sock_fd_v4[interfaceId], &ifr, IFF_UP, 0);
					setIpv4AddrToNwInterface(sock_fd_v4[interfaceId], &ifr,
							pPdnInfo[i].addressV4);
				}

				if (strlen(pPdnInfo[i].addressV6) > 0) {
					setStateOfNwInterface(sock_fd_v6[interfaceId], &ifr, IFF_UP, 0);
					setIpv6AddrToNwInterface(sock_fd_v6[interfaceId], &ifr,
							pPdnInfo[i].addressV6);
				}
			} else {
				LOGD("[RILData_Util] configureNetworkInterface reset %s to down.", ifr.ifr_name);
				ifc_reset_connections(ifr.ifr_name, RESET_ALL_ADDRESSES);
				ifc_remove_default_route(ifr.ifr_name);
				ifc_disable(ifr.ifr_name);
			}
		}
	}

	closeSocketsAfterNetworkInterfaceConfig(interfaceId);
}

int getAuthType(const char* authTypeStr)
{
	int authType = atoi(authTypeStr);
	//Application 0->none, 1->PAP, 2->CHAP, 3->PAP/CHAP;
	//Modem 0->PAP, 1->CHAP, 2->NONE, 3->PAP/CHAP;
	switch (authType) {
	case 0:
		return AUTHTYPE_NONE;
	case 1:
		return AUTHTYPE_PAP;
	case 2:
		return AUTHTYPE_CHAP;
	case 3:
		return AUTHTYPE_PAP_CHAP;
	default:
		return AUTHTYPE_NOT_SET;
	}
}

int getProtocolType(const char* protocol)
{
	int type = IPV4;
	if (protocol == NULL) {
		return type;
	}

	if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IP)) {
		type = IPV4;
	} else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV6)) {
		type = IPV6;
	} else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV4V6)) {
		type = IPV4V6;
	}

	LOGD("The protocol type is %d", type);
	return type;
}

int getPdnAddressType(char* addr)
{
	int type = IPV4;
	int length = strlen(addr);
	if (length >= MAX_IPV6_ADDRESS_LENGTH) {
		type = IPV4V6;
	} else if (length >= MAX_IPV4_ADDRESS_LENGTH) {
		type = IPV6;
	}
	return type;
}

int convertIpv6Address(char* pdp_addr, char* ipv6_addr, int isLinkLocal)
{
	char *p = NULL;
	int value = 0;
	int len = 0;
	int count = 8;

	if (isLinkLocal == 1) {
		strcpy(ipv6_addr, IPV6_PREFIX);
	} else {
		strcpy(ipv6_addr, NULL_ADDR);
	}

	p = strtok(pdp_addr, ".");

	//32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251
	//2001:0DB8:0000:0003:3D30:61B6:32FE:71FB

	// Skip the 64 bit of this PDP address if this pdp adress is a local link address
	if (isLinkLocal == 1) {
		while (count) {
			p = strtok(NULL, ".");
			count--;
		}
	}

	while (p) {
		value = atoi(p);
		sprintf(ipv6_addr, "%s%02X", ipv6_addr, value);
		p = strtok(NULL, ".");
		if (p == NULL) {
			LOGE("The format of IP address is illegal");
			return -1;
		}
		value = atoi(p);
		sprintf(ipv6_addr, "%s%02x:", ipv6_addr, value);
		p = strtok(NULL, ".");
	}

	LOGD("before convertIpv6Address:%s", ipv6_addr);
	len = strlen(ipv6_addr);
	ipv6_addr[len - 1] = '\0';
	LOGD("convertIpv6Address:%s", ipv6_addr);

	return 0;
}

void getIaCache(char* cache)
{
    //property1: [ICCID],[Protocol],[Auth Type],[Username]
    //property2: [APN]
    char iaProperty[PROPERTY_VALUE_MAX] = {0};
    char iaProperty2[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_IA, iaProperty, "");
    property_get(PROPERTY_IA_APN, iaProperty2, "");

    if (strlen(iaProperty) > 0) {
        sprintf(cache, "%s,%s", iaProperty, iaProperty2);
    } else {
        sprintf(cache, "%s", ""); //no IA cache
    }
    LOGD("[RILData_Util] getIaCache [%s], [%s][%s]", cache, iaProperty, iaProperty2);
}

int isInitialAttachApn(const char *requestedApn, const char * protocol,
		int authType, const char *username, const char* password, RILChannelCtx *pChannel)
{
	char iaProperty[PROPERTY_VALUE_MAX * 2] = { 0 };
	getIaCache(iaProperty);
    LOGD("[RILData_Util]isInitialAttachApn IaCache=%s", iaProperty);
	if (strlen(iaProperty) == 0) {
		// No initial attach APN, return false.
		return 0;
	}

	// Check if current IA property is different with what we want to set
	char apnParameter[PROPERTY_VALUE_MAX * 2] = { 0 };
	int canHandleIms = 0;
	if (strlen(password) == 0) {
		char iccid[PROPERTY_VALUE_MAX] = { 0 };
        property_get(PROPERTY_ICCID_SIM[getCdmaSocketSlotId() - 1], iccid, "");
		sprintf(apnParameter, "%s,%s,%d,%s,%d,%s", iccid, protocol, authType,
				username, canHandleIms, requestedApn);
	} else {
		//when password is set, iccid is not recorded, so we do not need to compare iccid
		sprintf(apnParameter, "%s,%s,%d,%s,%d,%s", "", protocol, authType,
				username, canHandleIms, requestedApn);
	}

    LOGD("[RILData_Util]isInitialAttachApn IaCache=%s, apnParameter=%s.", iaProperty,
			apnParameter);
    if (strcmp(apnParameter, iaProperty) == 0 || strcmp(requestedApn, APN_CTNET) == 0) {
		return 1;
	}

	return 0;
}

int queryMatchedPdnWithSameApn(const char* apn) {
    int i = 1;
    // Start from index 1 since we should not select CID0
    // CID0 is for network attach given PDN connection
    for (i = 1; i < nPdnInfoSize; i++) {
        LOGD("[RILData_Util]Query APN i=%d, apn = %s, pPdnInfo[i].interfaceId = %d, pPdnInfo[i].active = %d.",
                i, pPdnInfo[i].apn, pPdnInfo[i].interfaceId,
                pPdnInfo[i].active);
        if (pPdnInfo[i].interfaceId != INVALID_CID
                && pPdnInfo[i].active == DATA_STATE_ACTIVE
                && apnMatcher(pPdnInfo[i].apn, apn) == 0) {
            LOGD("[RILData_Util]Found reuseable CID%d for APN %s.", pPdnInfo[i].cid, apn);
            return pPdnInfo[i].cid;
        }
    }
    LOGD("[RILData_Util]Not found any cid for APN %s.", apn);
    return INVALID_CID;
}

int apnMatcher(const char* existApn, const char* apn) {
    // The APN is composed of two parts as follows: The APN Network Identifier & The APN Operator Identifier
    // The APN Operator Identifier is in the format of "mnc<MNC>.mcc<MCC>.gprs"
    // The valid APN format: <apn>[.mnc<MNC>.mcc<MCC>[.gprs]]
    int apnLength = strlen(apn);
    char *tmpExistApn = NULL;
    char *tmpApn = NULL;
    int j = 0;
    int result = -1;

    LOGD("[RILData_Util]apnMatcher: existApn = %s, apn = %s, apnLength = %d, exApnLen = %d.",
            existApn, apn, apnLength, strlen(existApn));
    if ((int) strlen(existApn) < apnLength) {
        return result;
    }

    tmpExistApn = calloc(1, (apnLength + 1) * sizeof(char));
    tmpApn = calloc(1, (apnLength + 1) * sizeof(char));

    // change to lower case to compare
    for (j = 0; j < apnLength; j++) {
        tmpExistApn[j] = tolower(existApn[j]);
        tmpApn[j] = tolower(apn[j]);
    }

    result = strcmp(tmpExistApn, tmpApn);
    if (result == 0) {
        if (!((strlen(apn) == strlen(existApn))
                || strStartsWith((existApn + apnLength), ".mnc")
                || strStartsWith((existApn + apnLength), ".MNC")
                || strStartsWith((existApn + apnLength), ".mcc")
                || strStartsWith((existApn + apnLength), ".MCC"))) {
            LOGD("[RILData_Util]apnMatcher: apns are equal but format unexpected");
            result = -1;
        }
    }

    free(tmpExistApn);
    free(tmpApn);

    return result;
}

void storeDataSystemProperties(int interfaceId)
{
	char propertyName[PROPERTY_KEY_MAX] = { 0 };
	char propertyValue[PROPERTY_VALUE_MAX] = { 0 };
	int i = 0;
	int j = 0;

	memset(propertyName, 0, sizeof(propertyName));
	sprintf(propertyName, "net.%s.local-ip", getNetworkInterfaceName(interfaceId));
	property_set(propertyName, "");
	LOGD("storeDataSystemProperties clear property [%s]", propertyName);

	memset(propertyName, 0, sizeof(propertyName));
	sprintf(propertyName, "net.%s.gw", getNetworkInterfaceName(interfaceId));
	property_set(propertyName, "");
	LOGD("storeDataSystemProperties clear property [%s]", propertyName);

	memset(propertyName, 0, sizeof(propertyName));
	sprintf(propertyName, "net.%s.local-ipv6", getNetworkInterfaceName(interfaceId));
	property_set(propertyName, "");
	LOGD("storeDataSystemProperties clear property [%s]", propertyName);

	memset(propertyName, 0, sizeof(propertyName));
	sprintf(propertyName, "net.%s.ipv6.gw", getNetworkInterfaceName(interfaceId));
	property_set(propertyName, "");
	LOGD("storeDataSystemProperties clear property [%s]", propertyName);

	// Clear DNS properties for both IPv4 and IPv6.
	for (j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
		memset(propertyName, 0, sizeof(propertyName));
		sprintf(propertyName, "net.%s.dns%d", getNetworkInterfaceName(interfaceId),
				j + 1);
		property_set(propertyName, "");
		LOGD("storeDataSystemProperties clear property [%s]", propertyName);

		memset(propertyName, 0, sizeof(propertyName));
		sprintf(propertyName, "net.%s.ipv6.dns%d", getNetworkInterfaceName(interfaceId),
				j + 1);
		property_set(propertyName, "");
		LOGD("storeDataSystemProperties clear property [%s]", propertyName);
	}

	// Store IP address, gateway and DNS for both IPv4 and IPv6.
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].interfaceId == interfaceId) {
			if (strlen(pPdnInfo[i].addressV4) > 0) {
				memset(propertyName, 0, sizeof(propertyName));
				sprintf(propertyName, "net.%s.local-ip", getNetworkInterfaceName(interfaceId));
				property_set(propertyName, pPdnInfo[i].addressV4);
				LOGD("storeDataSystemProperties set property [%s, %s]",
						propertyName, pPdnInfo[i].addressV4);

				memset(propertyName, 0, sizeof(propertyName));
				sprintf(propertyName, "net.%s.gw", getNetworkInterfaceName(interfaceId));
				property_set(propertyName, pPdnInfo[i].addressV4);
				LOGD("storeDataSystemProperties set property [%s, %s]",
						propertyName, pPdnInfo[i].addressV4);

				for (j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
					memset(propertyName, 0, sizeof(propertyName));
					sprintf(propertyName, "net.%s.dns%d", getNetworkInterfaceName(interfaceId), j + 1);
					property_set(propertyName, pPdnInfo[i].dnsV4[j]);
					LOGD("storeDataSystemProperties set property [%s, %s]",
							propertyName, pPdnInfo[i].dnsV4[j]);
				}
			}

			if (strlen(pPdnInfo[i].addressV6) > 0) {
				memset(propertyName, 0, sizeof(propertyName));
				sprintf(propertyName, "net.%s.local-ipv6", getNetworkInterfaceName(interfaceId));
				property_set(propertyName, pPdnInfo[i].addressV6);
				LOGD("storeDataSystemProperties set property [%s, %s]",
						propertyName, pPdnInfo[i].addressV6);

				memset(propertyName, 0, sizeof(propertyName));
				sprintf(propertyName, "net.%s.ipv6.gw", getNetworkInterfaceName(interfaceId));
				property_set(propertyName, pPdnInfo[i].addressV6);
				LOGD("storeDataSystemProperties set property [%s, %s]",
						propertyName, pPdnInfo[i].addressV6);

				for (j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
					memset(propertyName, 0, sizeof(propertyName));
					sprintf(propertyName, "net.%s.ipv6.dns%d", getNetworkInterfaceName(interfaceId), j + 1);
					property_set(propertyName, pPdnInfo[i].dnsV6[j]);
					LOGD("storeDataSystemProperties set property [%s, %s]",
							propertyName, pPdnInfo[i].dnsV6[j]);
				}
			}

			// Store MTU to net.ccmnix.mtu
			memset(propertyName, 0, sizeof(propertyName));
			sprintf(propertyName, "net.%s.mtu",
					getNetworkInterfaceName(interfaceId));
			memset(propertyValue, 0, sizeof(propertyValue));
			sprintf(propertyValue, "%d", pPdnInfo[i].mtu);
			property_set(propertyName, propertyValue);
			LOGD("storeDataSystemProperties set property [%s, %d]",
					propertyName, pPdnInfo[i].mtu);
		}
	}
}

// The AT channel for data requests is RIL_CHANNEL_2(ttySDIO6), detail see ril_commands.h.
int rilDataMain(int request, void *data, size_t datalen, RIL_Token t)
{
	LOGD("rilDataMain request = %d, datalen = %d.", request, datalen);
	switch (request) {
	case RIL_REQUEST_SETUP_DATA_CALL:
		requestSetupDataCall(data, datalen, t);
		break;
	case RIL_REQUEST_DATA_CALL_LIST:
		requestDataCallList(data, datalen, t);
		break;
	case RIL_REQUEST_DEACTIVATE_DATA_CALL:
		requestDeactiveDataCall(data, datalen, t);
		break;
	case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
		requestLastDataCallFailCause(data, datalen, t);
		break;
	case RIL_REQUEST_ALLOW_DATA:
		requestAllowData(data, datalen, t);
		break;
	case RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE:
		confirmIratChange(data, datalen, t);
	default:
		return 0; /* no matched request */
	}

	return 1; /* request found and handled */
}

int rilDataUnsolicited(const char *s, const char *sms_pdu)
{
    if (strStartsWith(s, "+CGEV: NW PDN DEACT")
            || strStartsWith(s, "+CGEV: ME PDN DEACT")) {
        // Ignore PDN deact message report from source RAT during Irat.
        if (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
            LOGD("[RILData_C2K_URC] Ignore source PDN deact during IRAT: status = %d.", nIratAction);
            RIL_requestProxyTimedCallback(onUpdateIratStatus, NULL,
                    &TIMEVAL_0, DATA_CHANNEL);
        } else {
            LOGD("[RILData_C2K_URC] PDN deact by NW/ME.");
            // Make sure to allocate new buffer for URC string.
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            RIL_requestProxyTimedCallback(onPdnDeactResult, (void *) urc,
                    &TIMEVAL_0, DATA_CHANNEL);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV: ME")) {
        if (strStartsWith(s, "+CGEV: ME PDN ACT")) {
            LOGD("[RILData_C2K_URC] PDN activated.");
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            RIL_requestProxyTimedCallback(onMePdnActive, (void *) urc,
                    &TIMEVAL_0, DATA_CHANNEL);
        } else {
            //+CGEV: ME related cases should be handled in setup data call request handler
            LOGD("[RILData_C2K_URC] ignore +CGEV: ME cases (%s)", s);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV: NW")) {
        if (strStartsWith(s, "+CGEV: NW MODIFY")) {
            LOGD("[RILData_C2K_URC] NW PDN MODIFY...");
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            RIL_requestProxyTimedCallback(onPdnModified, (void *) urc, &TIMEVAL_0, DATA_CHANNEL);
        } else {
            LOGD("[RILData_C2K_URC] Ignore +CGEV: NW cases (%s)", s);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV:")) {
        LOGD("[RILData_C2K_URC] Unhandled CGEV: urc = %s.", s);
        /* Really, we can ignore NW CLASS and ME CLASS events here,
         * but right now we don't since extranous
         * RIL_UNSOL_DATA_CALL_LIST_CHANGED calls are tolerated
         */
        if (s_md3_off) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
        } else {
            RIL_requestProxyTimedCallback(onDataCallListChanged, NULL, &TIMEVAL_0, DATA_CHANNEL);
        }

        return 1;
    } else if (strStartsWith(s, "+EI3GPPIRAT:")) {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        LOGD("[RILData_C2K_URC] urc = %s.", urc);
        RIL_requestProxyTimedCallback(onIratEvent, (void *) urc, &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    } else if (strStartsWith(s, "+EGCONTRDP:")) {
        LOGD("[RILData_C2K_URC] EGCONTRDP nIratAction = %d.", nIratAction);
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        RIL_requestProxyTimedCallback(onPdnSyncFromSourceRat, (void *) urc,
                &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    }

    return 0;
}

inline int isATCmdRspErr(int err, const ATResponse *p_response)
{
	//assert(p_response); //checking null here ???
	return (err < 0 || 0 == p_response->success) ? 1 : 0;
}

//TODO: remove this function.
int IsDatacallDialed(RIL_CALL_STATUS* callStatus)
{
	return 0;
}

// Irat code start
void onIratSourceStarted(int sourceRat, int targetRat, int type)
{
	LOGD("[RILData_C2K_IRAT] onIratSourceStarted: sourceRat = %d, targetRat = %d",
			sourceRat, targetRat);
	suspendAllDataTransmit();
	onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_SOURCE_STARTED, type);
}

void onIratSourceFinished(int sourceRat, int targetRat, int type)
{
	LOGD("[RILData_C2K_IRAT] onIratSourceFinished: sourceRat = %d, targetRat = %d",
			sourceRat, targetRat);

	switch (type) {
		case IRAT_TYPE_FAILED: {
			// IRAT fail: 1. resume all interfaces.
			// 2. deactviate previous deactivate failed PDNs.
			// 3. report IRAT status.
			resumeAllDataTransmit();
			int i = 0;
			for (i = 0; i < nPdnInfoSize; i++) {
				if (pPdnInfo[i].cid != INVALID_CID
						&& pPdnInfo[i].active == DATA_STATE_NEED_DEACT) {
					deactivatePdnByCid(pPdnInfo[i].cid);
				}
			}
			nIratAction = IRAT_ACTION_UNKNOWN;
			break;
		}

		case IRAT_TYPE_EHRPD_LTE: {
			clearAllPdnInfo();
			break;
		}

		case IRAT_TYPE_HRPD_LTE: {
			resumeAllDataTransmit();
			// Down all CCMNI interfaces and clean PDN info.
			cleanupPdnsForFallback();
			break;
		}

		default: {
			LOGW("[RILData_C2K_IRAT] Never should run into this case: type = %d.", type);
			break;
		}
	}
	onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_SOURCE_FINISHED, type);
}

void onIratTargetStarted(int sourceRat, int targetRat, int type)
{
	LOGD("[RILData_C2K_IRAT] onIratTargetStarted: sourceRat = %d, targetRat = %d",
			sourceRat, targetRat);
	nReactPdnCount = 0;
	nDeactPdnCount = 0;
	nReactSuccCount = 0;

	if (pSyncPdnInfo == NULL) {
		pSyncPdnInfo = calloc(1, nPdnInfoSize * sizeof(SyncPdnInfo));
		// Initialize sync PDN info list.
		clearAllSyncPdnInfo();
		LOGD("[RILData_C2K_IRAT] onIratTargetStarted init sync PDN info, size = %d.",
				nPdnInfoSize);
	}
	onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_TARGET_STARTED, type);
}

void onIratTargetFinished(int sourceRat, int targetRat, int type)
{
	LOGD("[RILData_C2K_IRAT] onIratTargetFinished: sourceRat = %d, targetRat = %d, "
			"nReactPdnCount = %d, nReactSuccCount = %d", sourceRat,
			targetRat, nReactPdnCount, nReactSuccCount);

	switch (type) {
		case IRAT_TYPE_FAILED: {
			clearAllPdnInfo();
			break;
		}

		case IRAT_TYPE_LTE_EHRPD: {
			// IRAT succeed.
			if (nDeactPdnCount != 0 || nReactPdnCount != nReactSuccCount) {
				handleDeactedOrFailedPdns(IRAT_NEED_RETRY);
				cleanupFailedPdns();
			}
			break;
		}

		case IRAT_TYPE_LTE_HRPD: {
			if (nDeactPdnCount != 0) {
				handleDeactedOrFailedPdns(IRAT_NO_RETRY);
			}
			cleanupFailedPdns();
			LOGD("[RILData_C2K_IRAT] fallback case finished.");
			break;
		}

		default: {
			LOGW("[RILData_C2K_IRAT] Never should run into this case: type = %d.", type);
			break;
		}
	}
	clearAllSyncPdnInfo();
	onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_TARGET_FINISHED, type);
	nIratAction = IRAT_ACTION_UNKNOWN;
}

void onIratStateChanged(int sourceRat, int targetRat, int action, int type)
{
	RIL_Pdn_IratInfo *pPdnIratInfo = (RIL_Pdn_IratInfo *) alloca(sizeof(RIL_Pdn_IratInfo));
	memset(pPdnIratInfo, 0, sizeof(RIL_Pdn_IratInfo));
	pPdnIratInfo->sourceRat = sourceRat;
	pPdnIratInfo->targetRat = targetRat;
	pPdnIratInfo->action = action;
    pPdnIratInfo->type = type;

	LOGD("[RILData_C2K_IRAT] onIratStateChanged: sourceRat = %d, targetRat = %d, "
				" nReactSuccCount = %d", sourceRat, targetRat, nReactSuccCount);
	RIL_onUnsolicitedResponse(RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE, pPdnIratInfo,
			sizeof(RIL_Pdn_IratInfo));
}

void onIratPdnReactSucc(int cid)
{
	LOGD("[RILData_C2K_IRAT] onIratPdnReactSucc: cid = %d, pdnStatus = %d.", cid,
			pSyncPdnInfo[cid].pdnStatus);
	pSyncPdnInfo[cid].pdnStatus = IRAT_PDN_STATUS_REACTED;
	pPdnInfo[cid].active = DATA_STATE_ACTIVE;

	RILChannelCtx* rilchnlctx = getChannelCtxbyId(DATA_CHANNEL);
	if (updatePdnAddressByCid(cid, rilchnlctx) < 0) {
		goto error;
	}

	if (0 == rebindPdnToInterface(pSyncPdnInfo[cid].interfaceId, cid, rilchnlctx)) {
		goto error;
	}

	if (updateDnsAndMtu(rilchnlctx) < 0) {
		goto error;
	}
	nReactSuccCount++;
	reconfigureNetworkInterface(pSyncPdnInfo[cid].interfaceId);
	return;

error:
    LOGD("[RILData_C2K_IRAT] onIratPdnReactSucc error happens, cid = %d.", cid);

    // Resume data transfer for the interface in case of MD3 is powered off.
    // Ex: IPO power on, MD3 powered off and the AT command will sent fail
    // Here we need to resume the data transmit
    setNetworkTransmitState(pSyncPdnInfo[cid].interfaceId, RESUME_DATA_TRANSFER);

    return;
}

void confirmIratChange(void *data, size_t datalen, RIL_Token t)
{
	int err = -1;
	int apDecision = 0;
	char *cmd = NULL;
	ATResponse *p_response = NULL;

	apDecision = ((int *) data)[0];
	LOGD("[RILData_C2K_IRAT] confirmIratChange: apDecision=%d.", apDecision);

	// Confirm IRAT change.
	asprintf(&cmd, "AT+EI3GPPIRAT=%d", apDecision);
	err = at_send_command(cmd, &p_response, DATA_CHANNEL_CTX);
	free(cmd);
	if (isATCmdRspErr(err, p_response)) {
		LOGE("confirmIratChange failed !");
		goto error;
	}

	LOGD("[RILData_C2K_IRAT] confirmIratChange succeed -");
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	return;

error:
	LOGD("[RILData_GSM_IRAT] confirmIRatChange failed - XXXXXX");
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
}

void onIratEvent(const char* urc)
{
	int err = 0;
	int action = -1;
	int sourceRat = 0;
	int targetRat = 0;
	char* line = urc;
	// +EI3GPPIRAT: <action>,<source RAT>,<target RAT>
	// 1: source rat start  2: target rat start
	// 3: source rat finish 4: target rat finish
	// RAT: 0: not specified 1:1XRTT 2:HRPD 3:EHRPD 4:LTE
	LOGD("[RILData_C2K_IRAT] onIratEvent: action = %d, urc = %s.", nIratAction, urc);

	err = at_tok_start(&line);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onIratEvent error 0.");
		goto error;
	}

	/* Get 1st parameter: start */
	err = at_tok_nextint(&line, &action);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onIratEvent error 1.");
		goto error;
	}

	/* Get 2nd parameter: source rat */
	err = at_tok_nextint(&line, &sourceRat);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onIratEvent error 2.");
		goto error;
	}

	/* Get 3rd parameter: target rat */
	err = at_tok_nextint(&line, &targetRat);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onIratEvent error 3.");
		goto error;
	}

	nIratAction = action;
	// Get IRAT type for later process.
	nIratType = getIratType(sourceRat, targetRat);

	LOGD("[RILData_C2K_IRAT] onIratEvent [%d, %d, %d, %d]", action, sourceRat,
			targetRat, nIratType);
	if (action == IRAT_ACTION_SOURCE_STARTED) {
		onIratSourceStarted(sourceRat, targetRat, nIratType);
	} else if (action == IRAT_ACTION_TARGET_STARTED) {
		onIratTargetStarted(sourceRat, targetRat, nIratType);
	} else if (action == IRAT_ACTION_SOURCE_FINISHED) {
		onIratSourceFinished(sourceRat, targetRat, nIratType);
	} else if (action == IRAT_ACTION_TARGET_FINISHED) {
		onIratTargetFinished(sourceRat, targetRat, nIratType);
	}

	free(urc);
	return;
error:
	LOGD("[RILData_C2K_IRAT] onIratEvent error - XXXXXX.");
	free(urc);
	return;
}

int onPdnSyncFromSourceRat(const char* urc)
{
	int err = 0;
	int cid = INVALID_CID;
	int interfaceId = INVALID_CID;
	int pdnStatus = IRAT_PDN_STATUS_UNKNOWN;
	char *out = NULL;
	char* line = urc;

	//+EGCONTRDP: <cid>,<APN_name>,<interface id>,<need_deact>, <PDP_addr_1>, [<PDP_addr2>]
	LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceRat: action = %d, urc = %s",
			nIratAction, urc);

	if (nIratAction != IRAT_ACTION_TARGET_STARTED) {
		LOGD("[RILData_C2K_IRAT] Ignore onPdnSyncFromSourceRat.");
		return -1;
	}
	err = at_tok_start(&line);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 0.");
		goto error;
	}

	/* Get 1st parameter: CID */
	err = at_tok_nextint(&line, &cid);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 1.");
		goto error;
	}
	pPdnInfo[cid].cid = cid;
	pSyncPdnInfo[cid].cid = cid;

	/* Get 2nd parameter: apn name */
	err = at_tok_nextstr(&line, &out);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 2.");
		goto error;
	}
	sprintf(pPdnInfo[cid].apn, "%s", out);
	sprintf(pSyncPdnInfo[cid].apn, "%s", out);

	/* Get 3rd parameter: interface id */
	err = at_tok_nextint(&line, &interfaceId);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 3.");
		goto error;
	}
	// -1 because we +1 when bind data using CGDATA
	pPdnInfo[cid].interfaceId = interfaceId - 1;
	pSyncPdnInfo[cid].interfaceId = interfaceId - 1;

	/* Get 4th parameter: pdn status */
	err = at_tok_nextint(&line, &pdnStatus);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceModem error 4.");
		goto error;
	}
	pSyncPdnInfo[cid].pdnStatus = pdnStatus;

	/* Get 5th parameter: IPAddr1 */
	err = at_tok_nextstr(&line, &out);
	if (err < 0) {
		LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceModem error 5.");
		goto error;
	}
	if (getPdnAddressType(out) == IPV4) {
		sprintf(pSyncPdnInfo[cid].addressV4, "%s", out);
	} else {
		convertIpv6Address(out, pSyncPdnInfo[cid].addressV6, 1);
	}

	/* Only for IPv4V6 + dual stack PDP context */
	/* Get 6th paramter: IPaddr 2*/
	if (at_tok_hasmore(&line)) {
		err = at_tok_nextstr(&line, &out);
		if (err < 0) {
			LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceModem error 6.");
			goto error;
		}

		if (getPdnAddressType(out) == IPV4) {
			sprintf(pSyncPdnInfo[cid].addressV4, "%s", out);
		} else {
			convertIpv6Address(out, pSyncPdnInfo[cid].addressV6, 1);
		}
	}

	pPdnInfo[cid].isDedicateBearer = 0;
	pPdnInfo[cid].isEmergency = 0;
	pPdnInfo[cid].active = DATA_STATE_INACTIVE;
	if (pSyncPdnInfo[cid].pdnStatus == IRAT_PDN_STATUS_DEACTED) {
		nDeactPdnCount++;
	}
	nReactPdnCount++;

	LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceRat: cid = %d, interfaceId = %d, "
			"pdnCount = %d, deactPdnCount = %d, apn = %s, ipv4 = %s, ipv6 = %s.",
			pSyncPdnInfo[cid].cid, pSyncPdnInfo[cid].interfaceId,
			nReactPdnCount, nDeactPdnCount, pPdnInfo[cid].apn,
			pSyncPdnInfo[cid].addressV4, pSyncPdnInfo[cid].addressV6);
	free(urc);
	return 0;

error:
	LOGD("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error - XXXXXX.");
	free(urc);
	return -1;
}

int rebindPdnToInterface(int interfaceId, int cid, RILChannelCtx * pChannel)
{
	int nRet = 1; //0:failed, 1: success
	char *cmd = NULL;
	int err = 0;

	LOGD("[RILData_C2K_IRAT] rebindPdnToInterface: cid = %d, interfaceId = %d E",
			cid, interfaceId);
	/* AT+CGDATA=<L2P>,<cid>,<channel ID> */
	asprintf(&cmd, "AT+CGDATA=\"%s\",%d,%d", pcL2pValue, cid, interfaceId + 1); //The channel id is from 1~n
	err = at_send_command(cmd, NULL, pChannel);
	free(cmd);
	if (err < 0) {
		LOGD("[RILData_Setup] CID%d fail to bind interface%d", cid,
				interfaceId);
		nRet = 0;
		goto error;
	} else {
		//The PDP is bind to a network interface, set to active
		LOGD("[RILData_Setup] CID%d is bind to interface%d", cid, interfaceId);
		pPdnInfo[cid].interfaceId = interfaceId;
		pPdnInfo[cid].active = DATA_STATE_ACTIVE;
	}

	LOGD("[RILData_C2K_IRAT] rebindPdnToInterface: finished X");
	return nRet;

error:
	LOGD("[RILData_C2K_IRAT] rebindPdnToInterface: error X");
	return nRet;
}

// Reconfigure CCMNI interface, the sequence need to be in order.
// 1. Bind CCMNI interface with current MD id.
// 2. Reset IP stack if IP changed.
// 3. Resume data transfer for the interface.
void reconfigureNetworkInterface(int interfaceId)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
	// Bind CCMNI interface with current MD id.
	ifc_ccmni_md_cfg(ifr.ifr_name, MD_SYS3);
	LOGD("[RILData_C2K_IRAT] reconfigureNetworkInterface: id = %d, name = %s.",
			interfaceId, ifr.ifr_name);

	int resetMask = 0;
	int i = 0;
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].interfaceId == interfaceId) {
			if (strlen(pSyncPdnInfo[i].addressV4) > 0) {
				if (strcmp(pSyncPdnInfo[i].addressV4, pPdnInfo[i].addressV4) != 0) {
					resetMask |= 0x01;
					LOGD("IPv4 address lost during IRAT %d, old = %s, new=%s.", interfaceId, pSyncPdnInfo[i].addressV4, pPdnInfo[i].addressV4);
				}
			}
			if (strlen(pSyncPdnInfo[i].addressV6) > 0) {
				if (strcmp(pSyncPdnInfo[i].addressV6, pPdnInfo[i].addressV6) != 0) {
					resetMask |= 0x02;
					LOGD("IPv6 address lost during IRAT %d, old = %s, new=%s.", interfaceId,pSyncPdnInfo[i].addressV6, pPdnInfo[i].addressV6);
				}
			}
		}
	}
	
	LOGD("[RILData_C2K_IRAT] reconfigureNetworkInterface resetMask = %2x.", resetMask);
	// Reset IP stack if IP changed.
	if (resetMask != 0) {
		updateNetworkInterface(interfaceId, resetMask);
	}

	// Resume data transfer for the interface.
	setNetworkTransmitState(interfaceId, RESUME_DATA_TRANSFER);
}

void resumeAllDataTransmit()
{
	LOGD("[RILData_C2K_IRAT] resumeAllDataTransmit...");
	int i = 0;
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].active == DATA_STATE_ACTIVE) {
			setNetworkTransmitState(pPdnInfo[i].interfaceId, RESUME_DATA_TRANSFER);
		}
	}
}

void suspendAllDataTransmit()
{
	LOGD("[RILData_C2K_IRAT] suspendAllDataTransmit...");
	int i = 0;
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].active == DATA_STATE_ACTIVE) {
			setNetworkTransmitState(pPdnInfo[i].interfaceId, SUSPEND_DATA_TRANSFER);
		}
	}
}

void setNetworkTransmitState(int interfaceId, int state)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
	LOGD("[RILData_C2K_IRAT] setNetworkTransmitState: id = %d, name= %s, state = %d.",
			interfaceId, ifr.ifr_name, state);
	ifc_set_txq_state(ifr.ifr_name, state);
}

void handleDeactedOrFailedPdns(int retryFailedPdn)
{
	if (pSyncPdnInfo != NULL) {
		int i = 0;
		for (i = 0; i < nPdnInfoSize; i++) {
			LOGD("[RILData_C2K_IRAT] handleDeactedOrFailedPdns: i = %d, cid = %d, status = %d.",
					i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
			// Deactivate PDN which is already deactivate in source RAT.
			if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_DEACTED) {
				deactivatePdnByCid(pSyncPdnInfo[i].cid);
			}
		}

		if (retryFailedPdn) {
			for (i = 0; i < nPdnInfoSize; i++) {
				LOGD("[RILData_C2K_IRAT] handleDeactedOrFailedPdns: i = %d, cid = %d, status = %d.",
						i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
				if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
					retryForFailedPdnByCid(pSyncPdnInfo[i].cid);
				}
			}
		}
	}
}

// Retry to deactivate PDN in target RAT which is already deactivated by source RAT,
// we don't check the result even if the deactivate failed.
void deactivatePdnByCid(int cid)
{
	LOGD("[RILData_C2K_IRAT] deactivatePdnByCid: cid = %d.", cid);
	char *cmd;
	ATResponse *p_response = NULL;
	RILChannelCtx *pChannel = getChannelCtxbyId(DATA_CHANNEL);

	asprintf(&cmd, "AT+CGACT=0,%d", cid);
	int err = at_send_command_notimeout(cmd, &p_response, pChannel);
	free(cmd);
	if (isATCmdRspErr(err, p_response)) {
		// Go on even if deactivated fail, clean up PDN info in RILD.
		int errCode = at_get_cme_error(p_response);
		if (errCode == PS_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED) {
			pPdnInfo[cid].active = DATA_STATE_NEED_DEACT;
		}
		goto error;
	}

	configureNetworkInterface(pPdnInfo[cid].interfaceId, DISABLE_CCMNI);
	clearPdnInfo(&pPdnInfo[cid]);
	storeDataSystemProperties(pPdnInfo[cid].interfaceId);
	AT_RSP_FREE(p_response);
	LOGD("[RILData_C2K_IRAT] deactivatePdnByCid finished----->.");
	return;

error:
	LOGD("[RILData_C2K_IRAT] deactivatePdnByCid[%d] failed with error[%d] - XXXXXX.",
					cid, err);
	AT_RSP_FREE(p_response);
}

// C2K need no retry for failed PDN currently(From modem's comment).
void retryForFailedPdnByCid(int cid)
{
	LOGD("[RILData_C2K_IRAT] retryForFailedPdnByCid: cid = %d.", cid);
}

void cleanupFailedPdns()
{
	if (pSyncPdnInfo != NULL) {
		int i = 0;
		for (i = 0; i < nPdnInfoSize; i++) {
			LOGD("[RILData_C2K_IRAT] cleanupFailedPdns: i = %d, cid = %d, status = %d.",
					i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
			if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
				cleanupPdnByCid(pSyncPdnInfo[i].cid);
			}
		}
	}
}

void cleanupPdnByCid(int cid)
{
	int i = 0;
	int interfaceId = pPdnInfo[cid].interfaceId;
	LOGD("[RILData_C2K_IRAT] cleanupPdnByCid: cid = %d, interfaceId = %d.",
			cid, interfaceId);

	// Resume data transfer and disable the CCMNI interface.
	setNetworkTransmitState(interfaceId, RESUME_DATA_TRANSFER);
	configureNetworkInterface(interfaceId, DISABLE_CCMNI);

	// Clear dedicate PDN which attached to the deactivated PDN.
	for (i = 0; i < nPdnInfoSize; i++) {
		if (pPdnInfo[i].isDedicateBearer && pPdnInfo[i].primaryCid == cid) {
			LOGD("[RILData_C2K_IRAT] Clean dedicate pdn CID%d info due to default pdn CID%d deactivated",
					pPdnInfo[i].cid, cid);
			clearPdnInfo(&pPdnInfo[i]);
		}
	}
	clearPdnInfo(&pPdnInfo[cid]);
}

void cleanupPdnsForFallback()
{
	if (pPdnInfo != NULL) {
		int i = 0;
		for (i = 0; i < nPdnInfoSize; i++) {
			LOGD("[RILData_C2K_IRAT] cleanupPdnsForFallback: i = %d, cid = %d, active = %d.",
					i, pPdnInfo[i].cid, pPdnInfo[i].active);
			if (pPdnInfo[i].cid != INVALID_CID) {
				cleanupPdnByCid(pPdnInfo[i].cid);
			}
		}
	}
}

void clearAllSyncPdnInfo()
{
	if (pSyncPdnInfo != NULL) {
		int i = 0;
		for (i = 0; i < nPdnInfoSize; i++) {
			clearSyncPdnInfo(&pSyncPdnInfo[i]);
		}
	}
}

void clearSyncPdnInfo(SyncPdnInfo* info)
{
	memset(info, 0, sizeof(SyncPdnInfo));
	info->interfaceId = INVALID_CID;
	info->cid = INVALID_CID;
	info->pdnStatus = IRAT_PDN_STATUS_UNKNOWN;
}

int getIratType(int sourceRat, int targetRat)
{
	int iratType = IRAT_TYPE_UNKNOWN;
	if (sourceRat == RAT_LTE && targetRat == RAT_EHRPD) {
		iratType = IRAT_TYPE_LTE_EHRPD;
	} else if (sourceRat == RAT_LTE && (targetRat == RAT_HRPD || targetRat == RAT_1XRTT)) {
		iratType = IRAT_TYPE_LTE_HRPD;
	} else if (sourceRat == RAT_EHRPD && targetRat == RAT_LTE) {
		iratType = IRAT_TYPE_EHRPD_LTE;
	} else if ((sourceRat == RAT_HRPD || sourceRat == RAT_1XRTT) && targetRat == RAT_LTE) {
		iratType = IRAT_TYPE_HRPD_LTE;
	} else if (sourceRat == targetRat) {
		iratType = IRAT_TYPE_FAILED;
	}
	LOGD("[RILData_C2K_IRAT] getIratType: iratType = %d.", iratType);
	return iratType;
}

void onUpdateIratStatus()
{
    nIratDeactCount++;
    LOGD("[onUpdateIratStatus] rid = %d", DATA_CHANNEL);
    RIL_requestProxyTimedCallback(onResetIratStatus, NULL,
            &TIMEVAL_10, DATA_CHANNEL);
}

void onResetIratStatus()
{
    LOGD("[onResetIratStatus] nIratDeactCount = %d, nIratAction = %d", nIratDeactCount, nIratAction);
    nIratDeactCount--;
    // during IRAT multiple PDN DEACT may be received, reset by the last one.
    // we suppose the time snap between two DEACT is less than 10s, this should be enough.
    // Add nIratAction = source start/finish check to avoid the two IRAT events happen within 10s
    // Ex: 3G->4G ---(<10s)--- 4G -> 3G, the nIratAction should not be reset in this case
    if ((nIratDeactCount == 0) &&
        (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED)) {
        nIratAction = IRAT_ACTION_UNKNOWN;
    }
}
