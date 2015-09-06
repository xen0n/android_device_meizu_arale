/*
 * Copyright (C) 2014 The OmniROM Project <http://www.omnirom.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.internal.telephony;

import static com.android.internal.telephony.RILConstants.*;

import android.content.Context;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncResult;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.telephony.PhoneNumberUtils;
import android.telephony.PhoneRatFamily;
import android.telephony.Rlog;
import android.telephony.SignalStrength;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Vector;

import java.io.IOException;
import java.io.InputStream;

import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;

public class MediaTekRIL extends RIL implements CommandsInterface {

    // TODO: Support multiSIM
    // Sim IDs are 0 / 1
    int mSimId = 0;

    /* ALPS00799783: for restore previous preferred network type when set type fail */
    private int mPreviousPreferredType = -1;

    /* M: call control part start */
    /* DTMF request will be ignored when duplicated sending */
    private class dtmfQueueHandler {

        public dtmfQueueHandler() {
            mDtmfStatus = DTMF_STATUS_STOP;
        }

        public void start() {
            mDtmfStatus = DTMF_STATUS_START;
        }

        public void stop() {
            mDtmfStatus = DTMF_STATUS_STOP;
        }

        public boolean isStart() {
            return (mDtmfStatus == DTMF_STATUS_START);
        }

        public void add(RILRequest o) {
            mDtmfQueue.addElement(o);
        }

        public void remove(RILRequest o) {
            mDtmfQueue.remove(o);
        }

        public void remove(int idx) {
            mDtmfQueue.removeElementAt(idx);
        }

        public RILRequest get() {
            return (RILRequest) mDtmfQueue.get(0);
        }

        public int size() {
            return mDtmfQueue.size();
        }

        public void setPendingRequest(RILRequest r) {
            mPendingCHLDRequest = r;
        }

        public RILRequest getPendingRequest() {
            return mPendingCHLDRequest;
        }

        public void setSendChldRequest() {
            mIsSendChldRequest = true;
        }

        public void resetSendChldRequest() {
            mIsSendChldRequest = false;
        }

        public boolean hasSendChldRequest() {
            riljLog("mIsSendChldRequest = " + mIsSendChldRequest);
            return mIsSendChldRequest;
        }

        public final int MAXIMUM_DTMF_REQUEST = 32;
        private final boolean DTMF_STATUS_START = true;
        private final boolean DTMF_STATUS_STOP = false;

        private boolean mDtmfStatus = DTMF_STATUS_STOP;
        private Vector mDtmfQueue = new Vector(MAXIMUM_DTMF_REQUEST);

        private RILRequest mPendingCHLDRequest = null;
        private boolean mIsSendChldRequest = false;
    }

    private dtmfQueueHandler mDtmfReqQueue = new dtmfQueueHandler();
    /* M: call control part end */

    public MediaTekRIL(Context context, int networkMode, int cdmaSubscription) {
	    super(context, networkMode, cdmaSubscription, null);
    }

    public MediaTekRIL(Context context, int networkMode, int cdmaSubscription, Integer instanceId) {
	    super(context, networkMode, cdmaSubscription, instanceId);
    }

    public static byte[] hexStringToBytes(String s) {
        byte[] ret;

        if (s == null) return null;

        int len = s.length();
        ret = new byte[len/2];

        for (int i=0 ; i <len ; i+=2) {
            ret[i/2] = (byte) ((hexCharToInt(s.charAt(i)) << 4)
                                | hexCharToInt(s.charAt(i+1)));
        }

        return ret;
    }

    static int hexCharToInt(char c) {
         if (c >= '0' && c <= '9') return (c - '0');
         if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
         if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

         throw new RuntimeException ("invalid hex char '" + c + "'");
    }

    protected Object
    responseOperatorInfos(Parcel p) {
        if (mInstanceId == null || mInstanceId == 0) {
            mSimId = 0;
        } else {
            mSimId = mInstanceId;
        }

        String strings[] = (String [])responseStrings(p);
        ArrayList<OperatorInfo> ret;

        if (strings.length % 4 != 0) {
            throw new RuntimeException(
                "RIL_REQUEST_QUERY_AVAILABLE_NETWORKS: invalid response. Got "
                + strings.length + " strings, expected multiples of 4");
        }

        String lacStr = SystemProperties.get("gsm.cops.lac");
        boolean lacValid = false;
        int lacIndex=0;

        Rlog.d(RILJ_LOG_TAG, "lacStr = " + lacStr+" lacStr.length="+lacStr.length()+" strings.length="+strings.length);
        if((lacStr.length() > 0) && (lacStr.length()%4 == 0) && ((lacStr.length()/4) == (strings.length/5 ))){
            Rlog.d(RILJ_LOG_TAG, "lacValid set to true");
            lacValid = true;
        }

        SystemProperties.set("gsm.cops.lac","");

        ret = new ArrayList<OperatorInfo>(strings.length / 4);

        for (int i = 0 ; i < strings.length ; i += 4) {
            if((strings[i+0] != null) && (strings[i+0].startsWith("uCs2") == true)) {
                riljLog("responseOperatorInfos handling UCS2 format name");

                try {
                    strings[i+0] = new String(hexStringToBytes(strings[i+0].substring(4)), "UTF-16");
                } catch(UnsupportedEncodingException ex) {
                    riljLog("responseOperatorInfos UnsupportedEncodingException");
                }
            }

            if ((lacValid == true) && (strings[i] != null)) {
                UiccController uiccController = UiccController.getInstance();
                IccRecords iccRecords = uiccController.getIccRecords(mSimId, UiccController.APP_FAM_3GPP);
                int lacValue = -1;
                String sEons = null;
                String lac = lacStr.substring(lacIndex,lacIndex+4);
                Rlog.d(RILJ_LOG_TAG, "lacIndex="+lacIndex+" lacValue="+lacValue+" lac="+lac+" plmn numeric="+strings[i+2]+" plmn name"+strings[i+0]);

                if(lac != "") {
                    lacValue = Integer.parseInt(lac, 16);
                    lacIndex += 4;
                    if(lacValue != 0xfffe) {
                        /*sEons = iccRecords.getEonsIfExist(strings[i+2],lacValue,true);
                        if(sEons != null) {
                            strings[i] = sEons;
                            Rlog.d(RILJ_LOG_TAG, "plmn name update to Eons: "+strings[i]);
                        }*/
                    } else {
                        Rlog.d(RILJ_LOG_TAG, "invalid lac ignored");
                    }
                }
            }

            if (strings[i] != null && (strings[i].equals("") || strings[i].equals(strings[i+2]))) {
                Operators init = new Operators ();
                String temp = init.unOptimizedOperatorReplace(strings[i+2]);
                riljLog("lookup RIL responseOperatorInfos() " + strings[i+2] + " gave " + temp);
                strings[i] = temp;
                strings[i+1] = temp;
            }

            // NOTE: We don't have the 5th element in MTK, and I don't know about
            // the cases that make this processing necessary. Disable for now.
            /*
            // 1, 2 = 2G
            // > 2 = 3G
            String property_name = "gsm.baseband.capability";
            if(mSimId > 0) {
                property_name = property_name + (mSimId+1);
            }

            int basebandCapability = SystemProperties.getInt(property_name, 3);
            Rlog.d(RILJ_LOG_TAG, "property_name="+property_name+", basebandCapability=" + basebandCapability);
            if (3 < basebandCapability) {
                strings[i+0] = strings[i+0].concat(" " + strings[i+4]);
                strings[i+1] = strings[i+1].concat(" " + strings[i+4]);
            }
            */

            ret.add(
                new OperatorInfo(
                    strings[i+0],
                    strings[i+1],
                    strings[i+2],
                    strings[i+3]));
        }

        return ret;
    }

    private Object
    responseCrssNotification(Parcel p) {
        /*SuppCrssNotification notification = new SuppCrssNotification();

        notification.code = p.readInt();
        notification.type = p.readInt();
        notification.number = p.readString();
        notification.alphaid = p.readString();
        notification.cli_validity = p.readInt();

        return notification;*/

        Rlog.e(RILJ_LOG_TAG, "NOT PROCESSING CRSS NOTIFICATION");
        return null;
    }

    private Object responseEtwsNotification(Parcel p) {
        /*EtwsNotification response = new EtwsNotification();

        response.warningType = p.readInt();
        response.messageId = p.readInt();
        response.serialNumber = p.readInt();
        response.plmnId = p.readString();
        response.securityInfo = p.readString();

        return response;*/
        Rlog.e(RILJ_LOG_TAG, "NOT PROCESSING ETWS NOTIFICATION");

        return null;
    }

    private Object responseSetupDedicateDataCall(Parcel p) {
        int number = p.readInt();
        if (RILJ_LOGD) riljLog("responseSetupDedicateDataCall number=" + number);
        /*
        DedicateDataCallState[] dedicateDataCalls = new DedicateDataCallState[number];
        for (int i=0; i<number; i++) {
            DedicateDataCallState dedicateDataCall = new DedicateDataCallState();
            dedicateDataCall.readFrom(p);
            dedicateDataCalls[i] = dedicateDataCall;

            riljLog("[" + dedicateDataCall.interfaceId + ", " + dedicateDataCall.defaultCid + ", " + dedicateDataCall.cid + ", " + dedicateDataCall.active +
                ", " + dedicateDataCall.signalingFlag + ", " + dedicateDataCall.failCause + ", Qos" + dedicateDataCall.qosStatus +
                ", Tft" + dedicateDataCall.tftStatus + ", PCSCF" + dedicateDataCall.pcscfInfo);
        }

        if (number > 1)
            return dedicateDataCalls;
        else if (number > 0)
            return dedicateDataCalls[0];
        else
            return null;
        */
        Rlog.e(RILJ_LOG_TAG, "responseSetupDedicateDataCall: stub!");

        return null;
    }

    private Object
    responseSetPhoneRatChanged(Parcel p) {
        int num = p.readInt();
        int rat = p.readInt();
        PhoneRatFamily response = new PhoneRatFamily(mInstanceId, rat);

        return response;
    }

    private Object
    responseNetworkInfoWithActs(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseNetworkInfoWithActs: stub!");

        return null;
    }

    private Object
    responseOperatorInfosWithAct(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseOperatorInfosWithAct: stub!");

        return null;
    }

    private Object
    responseFemtoCellInfos(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseFemtoCellInfos: stub!");

        return null;
    }

    private Object
    responseSmsParams(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseSmsParams: stub!");

        return null;
    }

    private Object
    responseSimSmsMemoryStatus(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseSimSmsMemoryStatus: stub!");

        return null;
    }

    private Object
    responseCbConfig(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseCbConfig: stub!");

        return null;
    }

    private Object
    responseModifyDataCall(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseModifyDataCall: stub!");

        return null;
    }

    private Object
    responsePcscfDiscovery(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responsePcscfDiscovery: stub!");

        return null;
    }

    private Object
    responseGetNitzTime(Parcel p) {
        Rlog.e(RILJ_LOG_TAG, "responseGetNitzTime: stub!");

        return null;
    }

    @Override
    public void setLocalCallHold(int lchStatus) {
        byte[] payload = new byte[]{(byte)(lchStatus & 0x7F)};
        Rlog.d(RILJ_LOG_TAG, "setLocalCallHold: lchStatus is " + lchStatus);

        // sendOemRilRequestRaw(OEMHOOK_EVT_HOOK_SET_LOCAL_CALL_HOLD, 1, payload, null);
        Rlog.e(RILJ_LOG_TAG, "setLocalCallHold: stub!");
    }

    @Override
    public void getModemCapability(Message response) {
        Rlog.d(RILJ_LOG_TAG, "GetModemCapability");
        // sendOemRilRequestRaw(OEMHOOK_EVT_HOOK_GET_MODEM_CAPABILITY, 0, null, response);
        Rlog.w(RILJ_LOG_TAG, "GetModemCapability: not really implemented!");
        AsyncResult.forMessage(response, null, CommandException.fromRilErrno(REQUEST_NOT_SUPPORTED));
        response.sendToTarget();
    }

    @Override
    public void updateStackBinding(int stack, int enable, Message response) {
        byte[] payload = new byte[]{(byte)stack,(byte)enable};
        Rlog.d(RILJ_LOG_TAG, "UpdateStackBinding: on Stack: " + stack +
                ", enable/disable: " + enable);

        // sendOemRilRequestRaw(OEMHOOK_EVT_HOOK_UPDATE_SUB_BINDING, 2, payload, response);
        Rlog.e(RILJ_LOG_TAG, "UpdateStackBinding: stub!");
    }

    @Override
    public void
    startDtmf(char c, Message result) {
        /* M: call control part start */
        /* DTMF request will be ignored when the count of requests reaches 32 */
        synchronized (mDtmfReqQueue) {
            if (!mDtmfReqQueue.hasSendChldRequest() && mDtmfReqQueue.size() < mDtmfReqQueue.MAXIMUM_DTMF_REQUEST) {
                if (!mDtmfReqQueue.isStart()) {
                    RILRequest rr = RILRequest.obtain(RIL_REQUEST_DTMF_START, result);

                    rr.mParcel.writeString(Character.toString(c));
                    mDtmfReqQueue.start();
                    mDtmfReqQueue.add(rr);
                    if (mDtmfReqQueue.size() == 1) {
                        riljLog("send start dtmf");
                        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                        send(rr);
                    }
                } else {
                    riljLog("DTMF status conflict, want to start DTMF when status is " + mDtmfReqQueue.isStart());
                }
            }
        }
        /* M: call control part end */
    }
    @Override
    public void
    stopDtmf(Message result) {
        /* M: call control part start */
        /* DTMF request will be ignored when the count of requests reaches 32 */
        synchronized (mDtmfReqQueue) {
            if (!mDtmfReqQueue.hasSendChldRequest() && mDtmfReqQueue.size() < mDtmfReqQueue.MAXIMUM_DTMF_REQUEST) {
                if (mDtmfReqQueue.isStart()) {
                    RILRequest rr = RILRequest.obtain(RIL_REQUEST_DTMF_STOP, result);

                    mDtmfReqQueue.stop();
                    mDtmfReqQueue.add(rr);
                    if (mDtmfReqQueue.size() == 1) {
                        riljLog("send stop dtmf");
                        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                        send(rr);
                    }
                } else {
                    riljLog("DTMF status conflict, want to start DTMF when status is " + mDtmfReqQueue.isStart());
                }
            }
        }
        /* M: call control part end */
    }

    @Override
    public void
    switchWaitingOrHoldingAndActive (Message result) {
        RILRequest rr
                = RILRequest.obtain(
                        RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE,
                                        result);
        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));

        /* M: call control part start */
        handleChldRelatedRequest(rr);
        /* M: call control part end */
    }

    @Override
    public void
    conference (Message result) {

        /* M: call control part start */
        /*
        if (MobileManagerUtils.isSupported()) {
            if (!checkMoMSSubPermission(SubPermissions.MAKE_CONFERENCE_CALL)) {
                return;
            }
        }
        */
        /* M: call control part end */

        RILRequest rr
                = RILRequest.obtain(RIL_REQUEST_CONFERENCE, result);

        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));

        /* M: call control part start */
        handleChldRelatedRequest(rr);
        /* M: call control part end */
    }

    @Override
    public void
    separateConnection (int gsmIndex, Message result) {
        RILRequest rr
                = RILRequest.obtain(RIL_REQUEST_SEPARATE_CONNECTION, result);

        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                            + " " + gsmIndex);

        rr.mParcel.writeInt(1);
        rr.mParcel.writeInt(gsmIndex);

        /* M: call control part start */
        handleChldRelatedRequest(rr);
        /* M: call control part end */
    }

    @Override
    public void
    explicitCallTransfer (Message result) {
        RILRequest rr
                = RILRequest.obtain(RIL_REQUEST_EXPLICIT_CALL_TRANSFER, result);

        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));

        /* M: call control part start */
        handleChldRelatedRequest(rr);
        /* M: call control part end */
    }

    // all that C&P just for responseOperator overriding?
    @Override
    protected RILRequest
    processSolicited (Parcel p) {
        int serial, error;
        boolean found = false;

        serial = p.readInt();
        error = p.readInt();

        RILRequest rr;

        rr = findAndRemoveRequestFromList(serial);

        if (rr == null) {
            Rlog.w(RILJ_LOG_TAG, "Unexpected solicited response! sn: "
                            + serial + " error: " + error);
            return null;
        }

        /* M: call control part start */
        /* DTMF request will be ignored when the count of requests reaches 32 */
        if ((rr.mRequest == RIL_REQUEST_DTMF_START) ||
            (rr.mRequest == RIL_REQUEST_DTMF_STOP)) {
            synchronized (mDtmfReqQueue) {
                mDtmfReqQueue.remove(rr);
                riljLog("remove first item in dtmf queue done, size = " + mDtmfReqQueue.size());
                if (mDtmfReqQueue.size() > 0) {
                    RILRequest rr2 = mDtmfReqQueue.get();
                    if (RILJ_LOGD) riljLog(rr2.serialString() + "> " + requestToString(rr2.mRequest));
                    send(rr2);
                } else {
                    if (mDtmfReqQueue.getPendingRequest() != null) {
                        riljLog("send pending switch request");
                        send(mDtmfReqQueue.getPendingRequest());
                        mDtmfReqQueue.setSendChldRequest();
                        mDtmfReqQueue.setPendingRequest(null);
                    }
                }
            }
        }
        /* M: call control part end */
        Object ret = null;

        if ((rr.mRequest == RIL_REQUEST_QUERY_AVAILABLE_NETWORKS) ||
            (rr.mRequest == RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT)) {
            // mGetAvailableNetworkDoneRegistrant.notifyRegistrants();
            Rlog.e(RILJ_LOG_TAG, "mGetAvailableNetworkDoneRegistrant.notifyRegistrants(); -- not implemented!");
        }

        /* ALPS00799783 START */
        if (rr.mRequest == RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE) {
            if ((error != 0) && (mPreviousPreferredType != -1)) {
                riljLog("restore mPreferredNetworkType from " + mPreferredNetworkType + " to " + mPreviousPreferredType);
                mPreferredNetworkType = mPreviousPreferredType;
            }
            mPreviousPreferredType = -1; //reset
        }
        /* ALPS00799783 END */

        /* M: call control part start */
        if (rr.mRequest == RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE ||
            rr.mRequest == RIL_REQUEST_CONFERENCE ||
            rr.mRequest == RIL_REQUEST_SEPARATE_CONNECTION ||
            rr.mRequest == RIL_REQUEST_EXPLICIT_CALL_TRANSFER) {
            riljLog("clear mIsSendChldRequest");
            mDtmfReqQueue.resetSendChldRequest();
        }
        /* M: call control part end */

        if (error == 0 || p.dataAvail() > 0) {

            /* Convert RIL_REQUEST_GET_MODEM_VERSION back */
            if (SystemProperties.get("ro.cm.device").indexOf("e73") == 0 &&
                  rr.mRequest == 220) {
                rr.mRequest = RIL_REQUEST_BASEBAND_VERSION;
            }

            // either command succeeds or command fails but with data payload
            try {switch (rr.mRequest) {
            /*
 cat libs/telephony/ril_commands.h \
 | egrep "^ *{RIL_" \
 | sed -re 's/\{([^,]+),[^,]+,([^}]+).+/case \1: ret = \2(p); break;/'
             */
            case RIL_REQUEST_GET_SIM_STATUS: ret =  responseIccCardStatus(p); break;
            case RIL_REQUEST_ENTER_SIM_PIN: ret =  responseInts(p); break;
            case RIL_REQUEST_ENTER_SIM_PUK: ret =  responseInts(p); break;
            case RIL_REQUEST_ENTER_SIM_PIN2: ret =  responseInts(p); break;
            case RIL_REQUEST_ENTER_SIM_PUK2: ret =  responseInts(p); break;
            case RIL_REQUEST_CHANGE_SIM_PIN: ret =  responseInts(p); break;
            case RIL_REQUEST_CHANGE_SIM_PIN2: ret =  responseInts(p); break;
            case RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE: ret =  responseInts(p); break;
            case RIL_REQUEST_GET_CURRENT_CALLS: ret =  responseCallList(p); break;
            case RIL_REQUEST_DIAL: ret =  responseVoid(p); break;
            case RIL_REQUEST_GET_IMSI: ret =  responseString(p); break;
            case RIL_REQUEST_HANGUP: ret =  responseVoid(p); break;
            case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND: ret =  responseVoid(p); break;
            case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND: ret = responseVoid(p); break;
            case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE: ret =  responseVoid(p); break;
            case RIL_REQUEST_CONFERENCE: ret =  responseVoid(p); break;
            case RIL_REQUEST_UDUB: ret =  responseVoid(p); break;
            case RIL_REQUEST_LAST_CALL_FAIL_CAUSE: ret =  responseInts(p); break;
            case RIL_REQUEST_SIGNAL_STRENGTH: ret =  responseSignalStrength(p); break;
            case RIL_REQUEST_VOICE_REGISTRATION_STATE: ret =  responseStrings(p); break;
            case RIL_REQUEST_DATA_REGISTRATION_STATE: ret =  responseStrings(p); break;
            case RIL_REQUEST_OPERATOR: ret =  responseOperator(p); break;
            case RIL_REQUEST_RADIO_POWER: ret =  responseVoid(p); break;
            case RIL_REQUEST_DTMF: ret =  responseVoid(p); break;
            case RIL_REQUEST_SEND_SMS: ret =  responseSMS(p); break;
            case RIL_REQUEST_SEND_SMS_EXPECT_MORE: ret =  responseSMS(p); break;
            case RIL_REQUEST_SETUP_DATA_CALL: ret =  responseSetupDataCall(p); break;
            case RIL_REQUEST_SIM_IO: ret =  responseICC_IO(p); break;
            case RIL_REQUEST_SEND_USSD: ret =  responseVoid(p); break;
            case RIL_REQUEST_CANCEL_USSD: ret =  responseVoid(p); break;
            case RIL_REQUEST_GET_CLIR: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_CLIR: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS: ret =  responseCallForward(p); break;
            case RIL_REQUEST_SET_CALL_FORWARD: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_CALL_WAITING: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_CALL_WAITING: ret =  responseVoid(p); break;
            case RIL_REQUEST_SMS_ACKNOWLEDGE: ret =  responseVoid(p); break;
            case RIL_REQUEST_GET_IMEI: ret =  responseString(p); break;
            case RIL_REQUEST_GET_IMEISV: ret =  responseString(p); break;
            case RIL_REQUEST_ANSWER: ret =  responseVoid(p); break;
            case RIL_REQUEST_DEACTIVATE_DATA_CALL: ret = responseDeactivateDataCall(p); break; //VoLTE
            case RIL_REQUEST_QUERY_FACILITY_LOCK: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_FACILITY_LOCK: ret =  responseInts(p); break;
            case RIL_REQUEST_CHANGE_BARRING_PASSWORD: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS : ret =  responseOperatorInfos(p); break;
            case RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS: ret =  responseVoid(p); break;
            case RIL_REQUEST_DTMF_START: ret =  responseVoid(p); break;
            case RIL_REQUEST_DTMF_STOP: ret =  responseVoid(p); break;
            case RIL_REQUEST_BASEBAND_VERSION: ret =  responseString(p); break;
            case RIL_REQUEST_SEPARATE_CONNECTION: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_MUTE: ret =  responseVoid(p); break;
            case RIL_REQUEST_GET_MUTE: ret =  responseInts(p); break;
            case RIL_REQUEST_QUERY_CLIP: ret =  responseInts(p); break;
            case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE: ret =  responseInts(p); break;
            case RIL_REQUEST_DATA_CALL_LIST: ret =  responseDataCallList(p); break;
            case RIL_REQUEST_RESET_RADIO: ret =  responseVoid(p); break;
            case RIL_REQUEST_OEM_HOOK_RAW: ret =  responseRaw(p); break;
            case RIL_REQUEST_OEM_HOOK_STRINGS: ret =  responseStrings(p); break;
            case RIL_REQUEST_SCREEN_STATE: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION: ret =  responseVoid(p); break;
            case RIL_REQUEST_WRITE_SMS_TO_SIM: ret =  responseInts(p); break;
            case RIL_REQUEST_DELETE_SMS_ON_SIM: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_BAND_MODE: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE: ret =  responseInts(p); break;
            case RIL_REQUEST_STK_GET_PROFILE: ret =  responseString(p); break;
            case RIL_REQUEST_STK_SET_PROFILE: ret =  responseVoid(p); break;
            case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND: ret =  responseString(p); break;
            case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE: ret =  responseVoid(p); break;
            case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM: ret =  responseInts(p); break;
            case RIL_REQUEST_EXPLICIT_CALL_TRANSFER: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE: ret =  responseVoid(p); break;
            case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE: ret =  responseGetPreferredNetworkType(p); break;
            case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS: ret = responseCellList(p); break;
            case RIL_REQUEST_SET_LOCATION_UPDATES: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_TTY_MODE: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_TTY_MODE: ret =  responseInts(p); break;
            case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE: ret =  responseInts(p); break;
            case RIL_REQUEST_CDMA_FLASH: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_BURST_DTMF: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_SEND_SMS: ret =  responseSMS(p); break;
            case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE: ret =  responseVoid(p); break;
            case RIL_REQUEST_GSM_GET_BROADCAST_CONFIG: ret =  responseGmsBroadcastConfig(p); break;
            case RIL_REQUEST_GSM_SET_BROADCAST_CONFIG: ret =  responseVoid(p); break;
            case RIL_REQUEST_GSM_BROADCAST_ACTIVATION: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_GET_BROADCAST_CONFIG: ret =  responseCdmaBroadcastConfig(p); break;
            case RIL_REQUEST_CDMA_SET_BROADCAST_CONFIG: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_BROADCAST_ACTIVATION: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY: ret =  responseVoid(p); break;
            case RIL_REQUEST_CDMA_SUBSCRIPTION: ret =  responseStrings(p); break;
            case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM: ret =  responseInts(p); break;
            case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM: ret =  responseVoid(p); break;
            case RIL_REQUEST_DEVICE_IDENTITY: ret =  responseStrings(p); break;
            case RIL_REQUEST_GET_SMSC_ADDRESS: ret = responseString(p); break;
            case RIL_REQUEST_SET_SMSC_ADDRESS: ret = responseVoid(p); break;
            case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE: ret = responseVoid(p); break;
            case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS: ret = responseVoid(p); break;
            case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING: ret = responseVoid(p); break;
            case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE: ret =  responseInts(p); break;
            case RIL_REQUEST_ISIM_AUTHENTICATION: ret =  responseString(p); break;
            case RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU: ret = responseVoid(p); break;
            case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS: ret = responseICC_IO(p); break;
            case RIL_REQUEST_VOICE_RADIO_TECH: ret = responseInts(p); break;
            case RIL_REQUEST_GET_CELL_INFO_LIST: ret = responseCellInfoList(p); break;
            case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_INITIAL_ATTACH_APN: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_DATA_PROFILE: ret = responseVoid(p); break;
            case RIL_REQUEST_IMS_REGISTRATION_STATE: ret = responseInts(p); break;
            case RIL_REQUEST_IMS_SEND_SMS: ret =  responseSMS(p); break;
            case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC: ret =  responseICC_IO(p); break;
            case RIL_REQUEST_SIM_OPEN_CHANNEL: ret  = responseInts(p); break;
            case RIL_REQUEST_SIM_CLOSE_CHANNEL: ret  = responseVoid(p); break;
            case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL: ret = responseICC_IO(p); break;
            case RIL_REQUEST_NV_READ_ITEM: ret = responseString(p); break;
            case RIL_REQUEST_NV_WRITE_ITEM: ret = responseVoid(p); break;
            case RIL_REQUEST_NV_WRITE_CDMA_PRL: ret = responseVoid(p); break;
            case RIL_REQUEST_NV_RESET_CONFIG: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_UICC_SUBSCRIPTION: ret = responseVoid(p); break;
            case RIL_REQUEST_ALLOW_DATA: ret = responseVoid(p); break;
            case RIL_REQUEST_GET_HARDWARE_CONFIG: ret = responseHardwareConfig(p); break;
            case RIL_REQUEST_SIM_AUTHENTICATION: ret =  responseICC_IOBase64(p); break;
            case RIL_REQUEST_SHUTDOWN: ret = responseVoid(p); break;
            case RIL_REQUEST_GET_PHONE_RAT_FAMILY: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_PHONE_RAT_FAMILY: ret =  responsePhoneId(); break;

            /* M: call control part start */
            case RIL_REQUEST_HANGUP_ALL: ret =  responseVoid(p); break;
            case RIL_REQUEST_FORCE_RELEASE_CALL: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_CALL_INDICATION: ret = responseVoid(p); break;
            case RIL_REQUEST_EMERGENCY_DIAL: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_ECC_SERVICE_CATEGORY: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_ECC_LIST: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_SPEECH_CODEC_INFO: ret = responseVoid(p); break;
            /* M: call control part end */

            /// M: IMS feature. @{
            case RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER: responseString(p); break;
            case RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER: responseString(p); break;
            case RIL_REQUEST_DIAL_WITH_SIP_URI: ret = responseVoid(p); break;
            case RIL_REQUEST_RETRIEVE_HELD_CALL: ret = responseVoid(p); break;
            /// @}

            //MTK-START SIM ME lock
            case RIL_REQUEST_QUERY_SIM_NETWORK_LOCK: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_SIM_NETWORK_LOCK: ret =  responseInts(p); break;
            //MTK-END SIM ME lock
            //MTK-START multiple application support
            case RIL_REQUEST_GENERAL_SIM_AUTH: ret =  responseICC_IO(p); break;
            case RIL_REQUEST_OPEN_ICC_APPLICATION: ret = responseInts(p); break;
            case RIL_REQUEST_GET_ICC_APPLICATION_STATUS: ret = responseIccCardStatus(p); break;
            //MTK-END multiple application support
            case RIL_REQUEST_SIM_IO_EX: ret =  responseICC_IO(p); break;
            // PHB Start
            case RIL_REQUEST_QUERY_PHB_STORAGE_INFO: ret = responseInts(p); break;
            case RIL_REQUEST_WRITE_PHB_ENTRY: ret = responseVoid(p); break;
            case RIL_REQUEST_READ_PHB_ENTRY: {
                // ret = responsePhbEntries(p); break;
                Rlog.e(RILJ_LOG_TAG, "RIL_REQUEST_READ_PHB_ENTRY: stub!");
                ret = responseVoid(p); break;
            }
            case RIL_REQUEST_QUERY_UPB_CAPABILITY: ret = responseInts(p); break;
            case RIL_REQUEST_READ_UPB_GRP: ret = responseInts(p); break;
            case RIL_REQUEST_WRITE_UPB_GRP: ret = responseVoid(p); break;
            case RIL_REQUEST_EDIT_UPB_ENTRY: ret = responseVoid(p); break;
            case RIL_REQUEST_DELETE_UPB_ENTRY: ret = responseVoid(p); break;
            case RIL_REQUEST_READ_UPB_GAS_LIST: ret = responseStrings(p); break;
            case RIL_REQUEST_GET_PHB_STRING_LENGTH: ret = responseInts(p); break;
            case RIL_REQUEST_GET_PHB_MEM_STORAGE: {
                // ret = responseGetPhbMemStorage(p); break;
                Rlog.e(RILJ_LOG_TAG, "RIL_REQUEST_GET_PHB_MEM_STORAGE: stub!");
                ret = responseVoid(p); break;
            }
            case RIL_REQUEST_SET_PHB_MEM_STORAGE: ret = responseVoid(p); break;
            case RIL_REQUEST_READ_PHB_ENTRY_EXT: {
                // ret = responseReadPhbEntryExt(p); break;
                Rlog.e(RILJ_LOG_TAG, "RIL_REQUEST_READ_PHB_ENTRY_EXT: stub!");
                ret = responseVoid(p); break;
            }
            case RIL_REQUEST_WRITE_PHB_ENTRY_EXT: ret = responseVoid(p); break;
            // PHB End


            /* M: network part start */
            case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT: ret =  responseVoid(p); break;
            case RIL_REQUEST_GET_POL_CAPABILITY: ret = responseInts(p); break;
            case RIL_REQUEST_GET_POL_LIST: ret = responseNetworkInfoWithActs(p); break;
            case RIL_REQUEST_SET_POL_ENTRY: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_TRM: ret = responseInts(p); break;
            case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT : ret =  responseOperatorInfosWithAct(p); break;
            case RIL_REQUEST_STK_EVDL_CALL_BY_AP: ret = responseVoid(p); break;

            //Femtocell (CSG) feature START
            case RIL_REQUEST_GET_FEMTOCELL_LIST: ret = responseFemtoCellInfos(p); break;
            case RIL_REQUEST_ABORT_FEMTOCELL_LIST: ret = responseVoid(p); break;
            case RIL_REQUEST_SELECT_FEMTOCELL: ret = responseVoid(p); break;
            //Femtocell (CSG) feature END
            /* M: network part end */

            case RIL_REQUEST_QUERY_MODEM_TYPE: ret = responseInts(p); break;
            case RIL_REQUEST_STORE_MODEM_TYPE: ret = responseVoid(p); break;

            // IMS
            case RIL_REQUEST_SET_IMS_ENABLE: ret = responseVoid(p); break;
            case RIL_REQUEST_SIM_GET_ATR: ret = responseString(p); break;
            // M: Fast Dormancy
            case RIL_REQUEST_SET_SCRI: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_FD_MODE: ret = responseInts(p); break;

            // MTK-START, SMS part
            case RIL_REQUEST_GET_SMS_PARAMS: ret = responseSmsParams(p); break;
            case RIL_REQUEST_SET_SMS_PARAMS: ret = responseVoid(p); break;
            case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS: ret = responseSimSmsMemoryStatus(p); break;
            case RIL_REQUEST_SET_ETWS: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO: ret = responseVoid(p); break;
            case RIL_REQUEST_GET_CB_CONFIG_INFO: ret = responseCbConfig(p); break;
            case RIL_REQUEST_REMOVE_CB_MESSAGE: ret = responseVoid(p); break;
            // MTK-END, SMS part
            case RIL_REQUEST_SET_DATA_CENTRIC: ret = responseVoid(p); break;

            //VoLTE
            case RIL_REQUEST_SETUP_DEDICATE_DATA_CALL: ret = responseSetupDedicateDataCall(p); break;
            case RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL: ret = responseVoid(p); break;
            case RIL_REQUEST_MODIFY_DATA_CALL: ret = responseModifyDataCall(p); break;
            case RIL_REQUEST_ABORT_SETUP_DATA_CALL: ret = responseVoid(p); break;
            case RIL_REQUEST_PCSCF_DISCOVERY_PCO: ret=responsePcscfDiscovery(p); break;
            case RIL_REQUEST_CLEAR_DATA_BEARER: ret=responseVoid(p); break;
            // M: CC33 LTE.
            case RIL_REQUEST_SET_DATA_ON_TO_MD: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE: ret = responseVoid(p); break;

            case RIL_REQUEST_BTSIM_CONNECT: ret = responseString(p); break;
            case RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF: ret = responseVoid(p); break;
            case RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM: ret = responseString(p); break;
            case RIL_REQUEST_BTSIM_TRANSFERAPDU: ret = responseString(p); break;

            /* M: call control part start */
            case RIL_REQUEST_SET_IMS_CALL_STATUS: ret = responseVoid(p); break;
            /* M: call control part end */

            /* M: C2K part start */
            case RIL_REQUEST_GET_NITZ_TIME: ret = responseGetNitzTime(p); break;
            case RIL_REQUEST_QUERY_UIM_INSERTED: ret = responseInts(p); break;
            case RIL_REQUEST_SWITCH_HPF: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_AVOID_SYS: ret = responseVoid(p); break;
            case RIL_REQUEST_QUERY_AVOID_SYS: ret = responseInts(p); break;
            case RIL_REQUEST_QUERY_CDMA_NETWORK_INFO: ret = responseStrings(p); break;
            case RIL_REQUEST_GET_LOCAL_INFO: ret =  responseInts(p); break;
            case RIL_REQUEST_UTK_REFRESH: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS: ret = responseInts(p); break;
            case RIL_REQUEST_QUERY_NETWORK_REGISTRATION: ret = responseInts(p); break;
            case RIL_REQUEST_AGPS_TCP_CONNIND: ret = responseVoid(p); break;
            case RIL_REQUEST_AGPS_SET_MPC_IPPORT: ret = responseVoid(p); break;
            case RIL_REQUEST_AGPS_GET_MPC_IPPORT: ret = responseStrings(p); break;
            case RIL_REQUEST_SET_MEID: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_REG_RESUME: ret =  responseVoid(p); break;
            case RIL_REQUEST_ENABLE_REG_PAUSE: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_ETS_DEV: ret =  responseVoid(p); break;
            case RIL_REQUEST_WRITE_MDN: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_VIA_TRM: ret = responseVoid(p); break;
            case RIL_REQUEST_SET_ARSI_THRESHOLD: ret =  responseVoid(p); break;
            /* M: C2K part end */
            default:
                throw new RuntimeException("Unrecognized solicited response: " + rr.mRequest);
            //break;
            }} catch (Throwable tr) {
                // Exceptions here usually mean invalid RIL responses

                Rlog.w(RILJ_LOG_TAG, rr.serialString() + "< "
                        + requestToString(rr.mRequest)
                        + " exception, possible invalid RIL response", tr);

                if (rr.mResult != null) {
                    AsyncResult.forMessage(rr.mResult, null, tr);
                    rr.mResult.sendToTarget();
                }
                return rr;
            }
        }

        if (rr.mRequest == RIL_REQUEST_SHUTDOWN) {
            // Set RADIO_STATE to RADIO_UNAVAILABLE to continue shutdown process
            // regardless of error code to continue shutdown procedure.
            riljLog("Response to RIL_REQUEST_SHUTDOWN received. Error is " +
                    error + " Setting Radio State to Unavailable regardless of error.");
            setRadioState(RadioState.RADIO_UNAVAILABLE);
        }

        // Here and below fake RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, see b/7255789.
        // This is needed otherwise we don't automatically transition to the main lock
        // screen when the pin or puk is entered incorrectly.
        switch (rr.mRequest) {
            case RIL_REQUEST_ENTER_SIM_PUK:
            case RIL_REQUEST_ENTER_SIM_PUK2:
                if (mIccStatusChangedRegistrants != null) {
                    if (RILJ_LOGD) {
                        riljLog("ON enter sim puk fakeSimStatusChanged: reg count="
                                + mIccStatusChangedRegistrants.size());
                    }
                    mIccStatusChangedRegistrants.notifyRegistrants();
                }
                break;
        }

        if (error != 0) {
            switch (rr.mRequest) {
                case RIL_REQUEST_ENTER_SIM_PIN:
                case RIL_REQUEST_ENTER_SIM_PIN2:
                case RIL_REQUEST_CHANGE_SIM_PIN:
                case RIL_REQUEST_CHANGE_SIM_PIN2:
                case RIL_REQUEST_SET_FACILITY_LOCK:
                    if (mIccStatusChangedRegistrants != null) {
                        if (RILJ_LOGD) {
                            riljLog("ON some errors fakeSimStatusChanged: reg count="
                                    + mIccStatusChangedRegistrants.size());
                        }
                        mIccStatusChangedRegistrants.notifyRegistrants();
                    }
                    break;

                case RIL_REQUEST_SET_PHONE_RAT_FAMILY:
                    if (RILJ_LOGD) {
                        riljLog("Set phone RAT failed, send response with phone ID");
                    }
                    ret = responsePhoneId();
                    break;
            }

            rr.onError(error, ret);
            return rr;
        }

        if (RILJ_LOGD) riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
            + " " + retToString(rr.mRequest, ret));

        if (rr.mResult != null) {
            AsyncResult.forMessage(rr.mResult, ret, null);
            rr.mResult.sendToTarget();
        }

        return rr;
    }

    @Override
    protected void
    processUnsolicited (Parcel p) {
        Object ret;
        int dataPosition = p.dataPosition(); // save off position within the Parcel
        int response = p.readInt();

        switch(response) {
            case RIL_UNSOL_NEIGHBORING_CELL_INFO: ret = responseStrings(p); break;
            case RIL_UNSOL_NETWORK_INFO: ret = responseStrings(p); break;
            case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED: ret =  responseVoid(p); break;
            case RIL_UNSOL_INVALID_SIM:  ret = responseStrings(p); break;
            case RIL_UNSOL_RESPONSE_ACMT: ret = responseInts(p); break;
            case RIL_UNSOL_IMEI_LOCK: ret = responseVoid(p); break;
            case RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED: ret = responseInts(p); break;
            case RIL_UNSOL_STK_EVDL_CALL: ret = responseInts(p); break;
            case RIL_UNSOL_STK_CALL_CTRL: ret = responseStrings(p); break;

            case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED: ret =  responseInts(p); break;
            case RIL_UNSOL_SRVCC_STATE_NOTIFY: ret = responseInts(p); break;
            case RIL_UNSOL_HARDWARE_CONFIG_CHANGED: ret = responseHardwareConfig(p); break;
            // fxxk this conflicts with RIL_UNSOL_RADIO_CAPABILITY...
            case RIL_UNSOL_SET_PHONE_RAT_FAMILY_COMPLETE:
                     ret = responseSetPhoneRatChanged(p); break;
            /* M: call control part start */
            case RIL_UNSOL_CALL_FORWARDING: ret = responseInts(p); break;
            case RIL_UNSOL_CRSS_NOTIFICATION: ret = responseCrssNotification(p); break;
            case RIL_UNSOL_INCOMING_CALL_INDICATION: ret = responseStrings(p); break;
            case RIL_UNSOL_CIPHER_INDICATION: ret = responseStrings(p); break;
            case RIL_UNSOL_CNAP: ret = responseStrings(p); break;
            case RIL_UNSOL_SPEECH_CODEC_INFO: ret =  responseInts(p); break;
            /* M: call control part end */
            //MTK-START multiple application support
            case RIL_UNSOL_APPLICATION_SESSION_ID_CHANGED: ret = responseInts(p); break;
            //MTK-END multiple application support
            case RIL_UNSOL_SIM_MISSING: ret = responseInts(p); break;
            case RIL_UNSOL_SIM_RECOVERY: ret = responseInts(p); break;
            case RIL_UNSOL_VIRTUAL_SIM_ON: ret = responseInts(p); break;
            case RIL_UNSOL_VIRTUAL_SIM_OFF: ret = responseInts(p); break;
            case RIL_UNSOL_SIM_PLUG_OUT: ret = responseVoid(p); break;
            case RIL_UNSOL_SIM_PLUG_IN: ret = responseVoid(p); break;
            case RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED: ret = responseVoid(p); break;
            case RIL_UNSOL_DATA_ALLOWED: ret = responseVoid(p); break;
            case RIL_UNSOL_PHB_READY_NOTIFICATION: ret = responseInts(p); break;
            case RIL_UNSOL_STK_SETUP_MENU_RESET: ret = responseVoid(p); break;
            // IMS
            case RIL_UNSOL_IMS_ENABLE_DONE: ret = responseVoid(p); break;
            case RIL_UNSOL_IMS_DISABLE_DONE: ret = responseVoid(p); break;
            case RIL_UNSOL_IMS_REGISTRATION_INFO: ret = responseInts(p); break;
            //VoLTE
            case RIL_UNSOL_DEDICATE_BEARER_ACTIVATED: ret = responseSetupDedicateDataCall(p);break;
            case RIL_UNSOL_DEDICATE_BEARER_MODIFIED: ret = responseSetupDedicateDataCall(p);break;
            case RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED: ret = responseInts(p);break;
            // M: Fast Dormancy
            case RIL_UNSOL_SCRI_RESULT: ret = responseInts(p); break;

            case RIL_UNSOL_RESPONSE_PLMN_CHANGED: ret = responseStrings(p); break;
            case RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED: ret = responseInts(p); break;
            //Remote SIM ME lock related APIs [Start]
            case RIL_UNSOL_MELOCK_NOTIFICATION: ret = responseInts(p); break;
            //Remote SIM ME lock related APIs [End]
            case RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_SUPPORT: ret = responseInts(p); break;

            /// M: IMS feature. @{
            //For updating call ids for conference call after SRVCC is done.
            case RIL_UNSOL_ECONF_SRVCC_INDICATION: ret = responseInts(p); break;
            //For updating conference call merged/added result.
            case RIL_UNSOL_ECONF_RESULT_INDICATION: ret = responseStrings(p); break;
            //For updating call mode and pau information.
            case RIL_UNSOL_CALL_INFO_INDICATION : ret = responseStrings(p); break;
            /// @}

            case RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_INFO: ret = responseInts(p); break;
            case RIL_UNSOL_SRVCC_HANDOVER_INFO_INDICATION:ret = responseInts(p); break;
            // M: CC33 LTE.
            case RIL_UNSOL_RAC_UPDATE: ret = responseVoid(p); break;
            case RIL_UNSOL_REMOVE_RESTRICT_EUTRAN: ret = responseVoid(p); break;

            //MTK-START for MD state change
            case RIL_UNSOL_MD_STATE_CHANGE: ret = responseInts(p); break;
            //MTK-END for MD state change

            case RIL_UNSOL_MO_DATA_BARRING_INFO: ret = responseInts(p); break;
            case RIL_UNSOL_SSAC_BARRING_INFO: ret = responseInts(p); break;
//MTK_TC1_FEATURE for LGE CSMCC_MO_CALL_MODIFIED {
            case RIL_UNSOL_RESPONSE_MO_CALL_STATE_CHANGED: ret =  responseStrings(p); break;
//}
            /* M: C2K part start*/
            case RIL_UNSOL_CDMA_CALL_ACCEPTED: ret = responseVoid(p); break;
            case RIL_UNSOL_UTK_SESSION_END: ret = responseVoid(p); break;
            case RIL_UNSOL_UTK_PROACTIVE_COMMAND: ret = responseString(p); break;
            case RIL_UNSOL_UTK_EVENT_NOTIFY: ret = responseString(p); break;
            case RIL_UNSOL_VIA_GPS_EVENT: ret = responseInts(p); break;
            case RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE: ret = responseInts(p); break;
            case RIL_UNSOL_VIA_PLMN_CHANGE_REG_PAUSE: ret = responseStrings(p); break;
            case RIL_UNSOL_VIA_INVALID_SIM_DETECTED: ret = responseVoid(p); break;
            /* M: C2K part end*/
            default:
                // Rewind the Parcel
                p.setDataPosition(dataPosition);

                // Forward responses that we are not overriding to the super class
                super.processUnsolicited(p);
                return;
        }

        // To avoid duplicating code from RIL.java, we rewrite some response codes to fit
        // AOSP's one (when they do the same effect)
        boolean rewindAndReplace = false;
        int newResponseCode = 0;

        switch (response) {
            case RIL_UNSOL_SET_PHONE_RAT_FAMILY_COMPLETE:
                if (RILJ_LOGD) unsljLogRet(response, ret);
                riljLog("mPhoneRatFamilyreg size :" + mPhoneRatFamilyChangedRegistrants.size());
                if (mPhoneRatFamilyChangedRegistrants != null) {
                    mPhoneRatFamilyChangedRegistrants.notifyRegistrants(
                            new AsyncResult(null, ret, null));
                }
                break;

            case RIL_UNSOL_PHB_READY_NOTIFICATION:
                if (RILJ_LOGD) unsljLogRet(response, ret);
                Rlog.e(RILJ_LOG_TAG, "RIL_UNSOL_PHB_READY_NOTIFICATION: stub!");
                /*
                if (mPhbReadyRegistrants != null) {
                    mPhbReadyRegistrants.notifyRegistrants(
                                        new AsyncResult(null, ret, null));
                }
                */
                break;

            case RIL_UNSOL_RESPONSE_PLMN_CHANGED:
                if (RILJ_LOGD) unsljLogvRet(response, ret);
                if (mPlmnChangeNotificationRegistrant != null) {
                    mPlmnChangeNotificationRegistrant.notifyRegistrants(new AsyncResult(null, ret, null));
                }
                break;

            case RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED:
                if (RILJ_LOGD) unsljLogvRet(response, ret);
                if (mRegistrationSuspendedRegistrant != null) {
                    mRegistrationSuspendedRegistrant.notifyRegistrant(new AsyncResult(null, ret, null));
                }
                break;

            /*
            case RIL_UNSOL_CALL_PROGRESS_INFO:
                rewindAndReplace = true;
                newResponseCode = RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED;
                break;
                */

            case RIL_UNSOL_INCOMING_CALL_INDICATION:
                setCallIndication((String[])ret);
                rewindAndReplace = true;
                newResponseCode = RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED;
                break;

            case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED:
                rewindAndReplace = true;
                newResponseCode = RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED;
                break;

            case RIL_UNSOL_SIM_INSERTED_STATUS:
            case RIL_UNSOL_SIM_MISSING:
            case RIL_UNSOL_SIM_PLUG_OUT:
            case RIL_UNSOL_SIM_PLUG_IN:
                rewindAndReplace = true;
                newResponseCode = RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED;
                break;

            case RIL_UNSOL_SMS_READY_NOTIFICATION:
                Rlog.e(RILJ_LOG_TAG, "RIL_UNSOL_SMS_READY_NOTIFICATION: stub!");
                /*if (mGsmSmsRegistrant != null) {
                    mGsmSmsRegistrant
                        .notifyRegistrant();
                }*/
                break;

            default:
                Rlog.i(RILJ_LOG_TAG, "Unprocessed unsolicited known MTK response: " + response);
        }

        if (rewindAndReplace) {
            Rlog.w(RILJ_LOG_TAG, "Rewriting MTK unsolicited response " + response + " to " + newResponseCode);

            // Rewrite
            p.setDataPosition(dataPosition);
            p.writeInt(newResponseCode);

            // And rewind again in front
            p.setDataPosition(dataPosition);

            super.processUnsolicited(p);
        }
    }

	static String
    requestToString(int request) {
/*
 cat libs/telephony/ril_commands.h \
 | egrep "^ *{RIL_" \
 | sed -re 's/\{RIL_([^,]+),[^,]+,([^}]+).+/case RIL_\1: return "\1";/'
*/
        switch(request) {
            case RIL_REQUEST_GET_SIM_STATUS: return "GET_SIM_STATUS";
            case RIL_REQUEST_ENTER_SIM_PIN: return "ENTER_SIM_PIN";
            case RIL_REQUEST_ENTER_SIM_PUK: return "ENTER_SIM_PUK";
            case RIL_REQUEST_ENTER_SIM_PIN2: return "ENTER_SIM_PIN2";
            case RIL_REQUEST_ENTER_SIM_PUK2: return "ENTER_SIM_PUK2";
            case RIL_REQUEST_CHANGE_SIM_PIN: return "CHANGE_SIM_PIN";
            case RIL_REQUEST_CHANGE_SIM_PIN2: return "CHANGE_SIM_PIN2";
            case RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE: return "ENTER_DEPERSONALIZATION_CODE";
            case RIL_REQUEST_GET_CURRENT_CALLS: return "GET_CURRENT_CALLS";
            case RIL_REQUEST_DIAL: return "DIAL";
            case RIL_REQUEST_GET_IMSI: return "GET_IMSI";
            case RIL_REQUEST_HANGUP: return "HANGUP";
            case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND: return "HANGUP_WAITING_OR_BACKGROUND";
            case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND: return "HANGUP_FOREGROUND_RESUME_BACKGROUND";
            case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE: return "REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
            case RIL_REQUEST_CONFERENCE: return "CONFERENCE";
            case RIL_REQUEST_UDUB: return "UDUB";
            case RIL_REQUEST_LAST_CALL_FAIL_CAUSE: return "LAST_CALL_FAIL_CAUSE";
            case RIL_REQUEST_SIGNAL_STRENGTH: return "SIGNAL_STRENGTH";
            case RIL_REQUEST_VOICE_REGISTRATION_STATE: return "VOICE_REGISTRATION_STATE";
            case RIL_REQUEST_DATA_REGISTRATION_STATE: return "DATA_REGISTRATION_STATE";
            case RIL_REQUEST_OPERATOR: return "OPERATOR";
            case RIL_REQUEST_RADIO_POWER: return "RADIO_POWER";
            case RIL_REQUEST_DTMF: return "DTMF";
            case RIL_REQUEST_SEND_SMS: return "SEND_SMS";
            case RIL_REQUEST_SEND_SMS_EXPECT_MORE: return "SEND_SMS_EXPECT_MORE";
            case RIL_REQUEST_SETUP_DATA_CALL: return "SETUP_DATA_CALL";
            case RIL_REQUEST_SIM_IO: return "SIM_IO";
            case RIL_REQUEST_SEND_USSD: return "SEND_USSD";
            /* M: SS part */
            ///M: For query CNAP
            case RIL_REQUEST_SEND_CNAP: return "SEND_CNAP";
            /* M: SS part end */
            case RIL_REQUEST_CANCEL_USSD: return "CANCEL_USSD";
            case RIL_REQUEST_GET_CLIR: return "GET_CLIR";
            case RIL_REQUEST_SET_CLIR: return "SET_CLIR";
            case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS: return "QUERY_CALL_FORWARD_STATUS";
            case RIL_REQUEST_SET_CALL_FORWARD: return "SET_CALL_FORWARD";
            case RIL_REQUEST_QUERY_CALL_WAITING: return "QUERY_CALL_WAITING";
            case RIL_REQUEST_SET_CALL_WAITING: return "SET_CALL_WAITING";
            case RIL_REQUEST_SMS_ACKNOWLEDGE: return "SMS_ACKNOWLEDGE";
            case RIL_REQUEST_GET_IMEI: return "GET_IMEI";
            case RIL_REQUEST_GET_IMEISV: return "GET_IMEISV";
            case RIL_REQUEST_ANSWER: return "ANSWER";
            case RIL_REQUEST_DEACTIVATE_DATA_CALL: return "DEACTIVATE_DATA_CALL";
            case RIL_REQUEST_QUERY_FACILITY_LOCK: return "QUERY_FACILITY_LOCK";
            case RIL_REQUEST_SET_FACILITY_LOCK: return "SET_FACILITY_LOCK";
            case RIL_REQUEST_CHANGE_BARRING_PASSWORD: return "CHANGE_BARRING_PASSWORD";
            case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE: return "QUERY_NETWORK_SELECTION_MODE";
            case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC: return "SET_NETWORK_SELECTION_AUTOMATIC";
            case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL: return "SET_NETWORK_SELECTION_MANUAL";
            case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS : return "QUERY_AVAILABLE_NETWORKS ";
            case RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS : return "ABORT_QUERY_AVAILABLE_NETWORKS";
            case RIL_REQUEST_DTMF_START: return "DTMF_START";
            case RIL_REQUEST_DTMF_STOP: return "DTMF_STOP";
            case RIL_REQUEST_BASEBAND_VERSION: return "BASEBAND_VERSION";
            case RIL_REQUEST_SEPARATE_CONNECTION: return "SEPARATE_CONNECTION";
            case RIL_REQUEST_SET_MUTE: return "SET_MUTE";
            case RIL_REQUEST_GET_MUTE: return "GET_MUTE";
            case RIL_REQUEST_QUERY_CLIP: return "QUERY_CLIP";
            case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE: return "LAST_DATA_CALL_FAIL_CAUSE";
            case RIL_REQUEST_DATA_CALL_LIST: return "DATA_CALL_LIST";
            case RIL_REQUEST_RESET_RADIO: return "RESET_RADIO";
            case RIL_REQUEST_OEM_HOOK_RAW: return "OEM_HOOK_RAW";
            case RIL_REQUEST_OEM_HOOK_STRINGS: return "OEM_HOOK_STRINGS";
            case RIL_REQUEST_SCREEN_STATE: return "SCREEN_STATE";
            case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION: return "SET_SUPP_SVC_NOTIFICATION";
            case RIL_REQUEST_WRITE_SMS_TO_SIM: return "WRITE_SMS_TO_SIM";
            case RIL_REQUEST_DELETE_SMS_ON_SIM: return "DELETE_SMS_ON_SIM";
            case RIL_REQUEST_SET_BAND_MODE: return "SET_BAND_MODE";
            case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE: return "QUERY_AVAILABLE_BAND_MODE";
            case RIL_REQUEST_STK_GET_PROFILE: return "REQUEST_STK_GET_PROFILE";
            case RIL_REQUEST_STK_SET_PROFILE: return "REQUEST_STK_SET_PROFILE";
            case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND: return "REQUEST_STK_SEND_ENVELOPE_COMMAND";
            case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE: return "REQUEST_STK_SEND_TERMINAL_RESPONSE";
            case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM: return "REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM";
            case RIL_REQUEST_EXPLICIT_CALL_TRANSFER: return "REQUEST_EXPLICIT_CALL_TRANSFER";
            case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE: return "REQUEST_SET_PREFERRED_NETWORK_TYPE";
            case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE: return "REQUEST_GET_PREFERRED_NETWORK_TYPE";
            case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS: return "REQUEST_GET_NEIGHBORING_CELL_IDS";
            case RIL_REQUEST_SET_LOCATION_UPDATES: return "REQUEST_SET_LOCATION_UPDATES";
            case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE: return "RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE";
            case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE: return "RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE";
            case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE: return "RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE";
            case RIL_REQUEST_SET_TTY_MODE: return "RIL_REQUEST_SET_TTY_MODE";
            case RIL_REQUEST_QUERY_TTY_MODE: return "RIL_REQUEST_QUERY_TTY_MODE";
            case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE: return "RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE";
            case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE: return "RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE";
            case RIL_REQUEST_CDMA_FLASH: return "RIL_REQUEST_CDMA_FLASH";
            case RIL_REQUEST_CDMA_BURST_DTMF: return "RIL_REQUEST_CDMA_BURST_DTMF";
            case RIL_REQUEST_CDMA_SEND_SMS: return "RIL_REQUEST_CDMA_SEND_SMS";
            case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE: return "RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE";
            case RIL_REQUEST_GSM_GET_BROADCAST_CONFIG: return "RIL_REQUEST_GSM_GET_BROADCAST_CONFIG";
            case RIL_REQUEST_GSM_SET_BROADCAST_CONFIG: return "RIL_REQUEST_GSM_SET_BROADCAST_CONFIG";
            case RIL_REQUEST_CDMA_GET_BROADCAST_CONFIG: return "RIL_REQUEST_CDMA_GET_BROADCAST_CONFIG";
            case RIL_REQUEST_CDMA_SET_BROADCAST_CONFIG: return "RIL_REQUEST_CDMA_SET_BROADCAST_CONFIG";
            case RIL_REQUEST_GSM_BROADCAST_ACTIVATION: return "RIL_REQUEST_GSM_BROADCAST_ACTIVATION";
            case RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY: return "RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY";
            case RIL_REQUEST_CDMA_BROADCAST_ACTIVATION: return "RIL_REQUEST_CDMA_BROADCAST_ACTIVATION";
            case RIL_REQUEST_CDMA_SUBSCRIPTION: return "RIL_REQUEST_CDMA_SUBSCRIPTION";
            case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM: return "RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM";
            case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM: return "RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM";
            case RIL_REQUEST_DEVICE_IDENTITY: return "RIL_REQUEST_DEVICE_IDENTITY";
            case RIL_REQUEST_GET_SMSC_ADDRESS: return "RIL_REQUEST_GET_SMSC_ADDRESS";
            case RIL_REQUEST_SET_SMSC_ADDRESS: return "RIL_REQUEST_SET_SMSC_ADDRESS";
            case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE: return "REQUEST_EXIT_EMERGENCY_CALLBACK_MODE";
            case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS: return "RIL_REQUEST_REPORT_SMS_MEMORY_STATUS";
            case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING: return "RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING";
            case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE: return "RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE";
            case RIL_REQUEST_ISIM_AUTHENTICATION: return "RIL_REQUEST_ISIM_AUTHENTICATION";
            case RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU: return "RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU";
            case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS: return "RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS";
            case RIL_REQUEST_VOICE_RADIO_TECH: return "RIL_REQUEST_VOICE_RADIO_TECH";
            case RIL_REQUEST_GET_CELL_INFO_LIST: return "RIL_REQUEST_GET_CELL_INFO_LIST";
            case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE: return "RIL_REQUEST_SET_CELL_INFO_LIST_RATE";
            case RIL_REQUEST_SET_INITIAL_ATTACH_APN: return "RIL_REQUEST_SET_INITIAL_ATTACH_APN";
            case RIL_REQUEST_SET_DATA_PROFILE: return "RIL_REQUEST_SET_DATA_PROFILE";
            case RIL_REQUEST_IMS_REGISTRATION_STATE: return "RIL_REQUEST_IMS_REGISTRATION_STATE";
            case RIL_REQUEST_IMS_SEND_SMS: return "RIL_REQUEST_IMS_SEND_SMS";
            case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC: return "RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC";
            case RIL_REQUEST_SIM_OPEN_CHANNEL: return "RIL_REQUEST_SIM_OPEN_CHANNEL";
            case RIL_REQUEST_SIM_CLOSE_CHANNEL: return "RIL_REQUEST_SIM_CLOSE_CHANNEL";
            case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL: return "RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL";
            case RIL_REQUEST_NV_READ_ITEM: return "RIL_REQUEST_NV_READ_ITEM";
            case RIL_REQUEST_NV_WRITE_ITEM: return "RIL_REQUEST_NV_WRITE_ITEM";
            case RIL_REQUEST_NV_WRITE_CDMA_PRL: return "RIL_REQUEST_NV_WRITE_CDMA_PRL";
            case RIL_REQUEST_NV_RESET_CONFIG: return "RIL_REQUEST_NV_RESET_CONFIG";
            case RIL_REQUEST_SET_UICC_SUBSCRIPTION: return "RIL_REQUEST_SET_UICC_SUBSCRIPTION";
            case RIL_REQUEST_ALLOW_DATA: return "RIL_REQUEST_ALLOW_DATA";
            case RIL_REQUEST_GET_HARDWARE_CONFIG: return "GET_HARDWARE_CONFIG";
            case RIL_REQUEST_SIM_AUTHENTICATION: return "RIL_REQUEST_SIM_AUTHENTICATION";
            case RIL_REQUEST_SHUTDOWN: return "RIL_REQUEST_SHUTDOWN";
            case RIL_REQUEST_SET_PHONE_RAT_FAMILY: return "RIL_REQUEST_SET_PHONE_RAT_FAMILY";
            case RIL_REQUEST_GET_PHONE_RAT_FAMILY: return "RIL_REQUEST_GET_PHONE_RAT_FAMILY";

            /* M: call control part start */
            case RIL_REQUEST_HANGUP_ALL: return "HANGUP_ALL";
            case RIL_REQUEST_FORCE_RELEASE_CALL: return "FORCE_RELEASE_CALL";
            case RIL_REQUEST_SET_CALL_INDICATION: return "SET_CALL_INDICATION";
            case RIL_REQUEST_EMERGENCY_DIAL: return "EMERGENCY_DIAL";
            case RIL_REQUEST_SET_ECC_SERVICE_CATEGORY: return "SET_ECC_SERVICE_CATEGORY";
            case RIL_REQUEST_SET_ECC_LIST: return "SET_ECC_LIST";
            case RIL_REQUEST_SET_SPEECH_CODEC_INFO: return "SET_SPEECH_CODEC_INFO";
            /* M: call control part end */

            /// M: IMS feature. @{
            case RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER: return "RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER";
            case RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER: return "RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER";
            case RIL_REQUEST_DIAL_WITH_SIP_URI: return "RIL_REQUEST_DIAL_WITH_SIP_URI";
            case RIL_REQUEST_RETRIEVE_HELD_CALL: return "RIL_REQUEST_RETRIEVE_HELD_CALL";
            /// @}

            //MTK-START SIM ME lock
            case RIL_REQUEST_QUERY_SIM_NETWORK_LOCK: return "QUERY_SIM_NETWORK_LOCK";
            case RIL_REQUEST_SET_SIM_NETWORK_LOCK: return "SET_SIM_NETWORK_LOCK";
            //MTK-END SIM ME lock
            //ISIM
            case RIL_REQUEST_GENERAL_SIM_AUTH: return "RIL_REQUEST_GENERAL_SIM_AUTH";
            case RIL_REQUEST_OPEN_ICC_APPLICATION: return "RIL_REQUEST_OPEN_ICC_APPLICATION";
            case RIL_REQUEST_GET_ICC_APPLICATION_STATUS: return "RIL_REQUEST_GET_ICC_APPLICATION_STATUS";
            case RIL_REQUEST_SIM_IO_EX: return "SIM_IO_EX";

            // PHB Start
            case RIL_REQUEST_QUERY_PHB_STORAGE_INFO: return "RIL_REQUEST_QUERY_PHB_STORAGE_INFO";
            case RIL_REQUEST_WRITE_PHB_ENTRY: return "RIL_REQUEST_WRITE_PHB_ENTRY";
            case RIL_REQUEST_READ_PHB_ENTRY: return "RIL_REQUEST_READ_PHB_ENTRY";
            case RIL_REQUEST_QUERY_UPB_CAPABILITY: return "RIL_REQUEST_QUERY_UPB_CAPABILITY";
            case RIL_REQUEST_EDIT_UPB_ENTRY: return "RIL_REQUEST_EDIT_UPB_ENTRY";
            case RIL_REQUEST_DELETE_UPB_ENTRY: return "RIL_REQUEST_DELETE_UPB_ENTRY";
            case RIL_REQUEST_READ_UPB_GAS_LIST: return "RIL_REQUEST_READ_UPB_GAS_LIST";
            case RIL_REQUEST_READ_UPB_GRP: return "RIL_REQUEST_READ_UPB_GRP";
            case RIL_REQUEST_WRITE_UPB_GRP: return "RIL_REQUEST_WRITE_UPB_GRP";
            case RIL_REQUEST_GET_PHB_STRING_LENGTH: return "RIL_REQUEST_GET_PHB_STRING_LENGTH";
            case RIL_REQUEST_GET_PHB_MEM_STORAGE: return "RIL_REQUEST_GET_PHB_MEM_STORAGE";
            case RIL_REQUEST_SET_PHB_MEM_STORAGE: return "RIL_REQUEST_SET_PHB_MEM_STORAGE";
            case RIL_REQUEST_READ_PHB_ENTRY_EXT: return "RIL_REQUEST_READ_PHB_ENTRY_EXT";
            case RIL_REQUEST_WRITE_PHB_ENTRY_EXT: return "RIL_REQUEST_WRITE_PHB_ENTRY_EXT";
            // PHB End

            /* M: network part start */
            case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT: return "SET_NETWORK_SELECTION_MANUAL_WITH_ACT";
            case RIL_REQUEST_GET_POL_CAPABILITY: return "RIL_REQUEST_GET_POL_CAPABILITY";
            case RIL_REQUEST_GET_POL_LIST: return "RIL_REQUEST_GET_POL_LIST";
            case RIL_REQUEST_SET_POL_ENTRY: return "RIL_REQUEST_SET_POL_ENTRY";
            case RIL_REQUEST_SET_TRM: return "RIL_REQUEST_SET_TRM";
            case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT : return "QUERY_AVAILABLE_NETWORKS_WITH_ACT";
            //Femtocell (CSG) feature START
            case RIL_REQUEST_GET_FEMTOCELL_LIST: return "RIL_REQUEST_GET_FEMTOCELL_LIST";
            case RIL_REQUEST_ABORT_FEMTOCELL_LIST: return "RIL_REQUEST_ABORT_FEMTOCELL_LIST";
            case RIL_REQUEST_SELECT_FEMTOCELL: return "RIL_REQUEST_SELECT_FEMTOCELL";
            //Femtocell (CSG) feature END
            /* M: network part end */
            case RIL_REQUEST_STK_EVDL_CALL_BY_AP: return "RIL_REQUEST_STK_EVDL_CALL_BY_AP";
            case RIL_REQUEST_QUERY_MODEM_TYPE: return "RIL_REQUEST_QUERY_MODEM_TYPE";
            case RIL_REQUEST_STORE_MODEM_TYPE: return "RIL_REQUEST_STORE_MODEM_TYPE";
            case RIL_REQUEST_SIM_GET_ATR: return "SIM_GET_ATR";
            case RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW: return "SIM_OPEN_CHANNEL_WITH_SW";
            //VoLTE
            case RIL_REQUEST_SETUP_DEDICATE_DATA_CALL: return "RIL_REQUEST_SETUP_DEDICATE_DATA_CALL";
            case RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL: return "RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL";
            case RIL_REQUEST_MODIFY_DATA_CALL: return "RIL_REQUEST_MODIFY_DATA_CALL";
            case RIL_REQUEST_ABORT_SETUP_DATA_CALL: return "RIL_REQUEST_ABORT_SETUP_DATA_CALL";
            case RIL_REQUEST_PCSCF_DISCOVERY_PCO: return "RIL_REQUEST_PCSCF_DISCOVERY_PCO";
            case RIL_REQUEST_CLEAR_DATA_BEARER: return "RIL_REQUEST_CLEAR_DATA_BEARER";

            // IMS
            case RIL_REQUEST_SET_IMS_ENABLE: return "RIL_REQUEST_SET_IMS_ENABLE";

            // M: Fast Dormancy
            case RIL_REQUEST_SET_SCRI: return "RIL_REQUEST_SET_SCRI";
            case RIL_REQUEST_SET_FD_MODE: return "RIL_REQUEST_SET_FD_MODE";
            // MTK-START, SMS part
            case RIL_REQUEST_GET_SMS_PARAMS: return "RIL_REQUEST_GET_SMS_PARAMS";
            case RIL_REQUEST_SET_SMS_PARAMS: return "RIL_REQUEST_SET_SMS_PARAMS";
            case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS: return "RIL_REQUEST_GET_SMS_SIM_MEM_STATUS";
            case RIL_REQUEST_SET_ETWS: return "RIL_REQUEST_SET_ETWS";
            case RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO:
                return "RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO";
            case RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO:
                return "RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO";
            case RIL_REQUEST_GET_CB_CONFIG_INFO: return "RIL_REQUEST_GET_CB_CONFIG_INFO";
            case RIL_REQUEST_REMOVE_CB_MESSAGE: return "RIL_REQUEST_REMOVE_CB_MESSAGE";
            // MTK-END, SMS part
            case RIL_REQUEST_SET_DATA_CENTRIC: return "RIL_REQUEST_SET_DATA_CENTRIC";

            case RIL_REQUEST_MODEM_POWEROFF: return "MODEM_POWEROFF";
            case RIL_REQUEST_MODEM_POWERON: return "MODEM_POWERON";
            // M: CC33 LTE.
            case RIL_REQUEST_SET_DATA_ON_TO_MD: return "RIL_REQUEST_SET_DATA_ON_TO_MD";
            case RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE: return "RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE";
            case RIL_REQUEST_BTSIM_CONNECT: return "RIL_REQUEST_BTSIM_CONNECT";
            case RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF: return "RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF";
            case RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM: return "RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM";
            case RIL_REQUEST_BTSIM_TRANSFERAPDU: return "RIL_REQUEST_SEND_BTSIM_TRANSFERAPDU";

            /* M: call control part start */
            case RIL_REQUEST_SET_IMS_CALL_STATUS: return "RIL_REQUEST_SET_IMS_CALL_STATUS";
            /* M: call control part end */

            /* M: C2K part start */
            case RIL_REQUEST_GET_NITZ_TIME: return "RIL_REQUEST_GET_NITZ_TIME";
            case RIL_REQUEST_QUERY_UIM_INSERTED: return "RIL_REQUEST_QUERY_UIM_INSERTED";
            case RIL_REQUEST_SWITCH_HPF: return "RIL_REQUEST_SWITCH_HPF";
            case RIL_REQUEST_SET_AVOID_SYS: return "RIL_REQUEST_SET_AVOID_SYS";
            case RIL_REQUEST_QUERY_AVOID_SYS: return "RIL_REQUEST_QUERY_AVOID_SYS";
            case RIL_REQUEST_QUERY_CDMA_NETWORK_INFO: return "RIL_REQUEST_QUERY_CDMA_NETWORK_INFO";
            case RIL_REQUEST_GET_LOCAL_INFO: return "RIL_REQUEST_GET_LOCAL_INFO";
            case RIL_REQUEST_UTK_REFRESH: return "RIL_REQUEST_UTK_REFRESH";
            case RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS:
                return "RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS";
            case RIL_REQUEST_QUERY_NETWORK_REGISTRATION:
                return "RIL_REQUEST_QUERY_NETWORK_REGISTRATION";
            case RIL_REQUEST_AGPS_TCP_CONNIND: return "RIL_REQUEST_AGPS_TCP_CONNIND";
            case RIL_REQUEST_AGPS_SET_MPC_IPPORT: return "RIL_REQUEST_AGPS_SET_MPC_IPPORT";
            case RIL_REQUEST_AGPS_GET_MPC_IPPORT: return "RIL_REQUEST_AGPS_GET_MPC_IPPORT";
            case RIL_REQUEST_SET_MEID: return "RIL_REQUEST_SET_MEID";
            case RIL_REQUEST_SET_REG_RESUME: return "RIL_REQUEST_SET_REG_RESUME";
            case RIL_REQUEST_ENABLE_REG_PAUSE: return "RIL_REQUEST_ENABLE_REG_PAUSE";
            case RIL_REQUEST_SET_ETS_DEV: return "RIL_REQUEST_SET_ETS_DEV";
            case RIL_REQUEST_WRITE_MDN: return "RIL_REQUEST_WRITE_MDN";
            case RIL_REQUEST_SET_VIA_TRM: return "RIL_REQUEST_SET_VIA_TRM";
            case RIL_REQUEST_SET_ARSI_THRESHOLD: return "RIL_REQUEST_SET_ARSI_THRESHOLD";
            /* M: C2K part end */
            default: return "<unknown request> (" + request + ")";
        }
    }

    // VoLTE
    private Object
    responseDeactivateDataCall(Parcel p) {
        int [] cidArray = null;
        if (p.dataSize() > 0 ) {    //checking Parcel data value
            cidArray = (int []) responseInts(p);
        }

        return cidArray;
    }

    private Object
    responseOperator(Parcel p) {
        int num;
        String response[] = null;

        response = p.readStringArray();

        if (false) {
            num = p.readInt();

            response = new String[num];
            for (int i = 0; i < num; i++) {
                response[i] = p.readString();
            }
        }

        for (int i = 0; i < response.length; i++) {
            if((response[i] != null) && (response[i].startsWith("uCs2") == true))
            {
                riljLog("responseOperator handling UCS2 format name: response[" + i + "]");
                try{
                    response[i] = new String(hexStringToBytes(response[i].substring(4)),"UTF-16");
                }catch(UnsupportedEncodingException ex){
                    riljLog("responseOperatorInfos UnsupportedEncodingException");
                }
            }
        }

        // NOTE: the original code seemingly has some nontrivial SpnOverride
        // modifications, so I'm not going to port that.
        if (response.length > 2 && response[2] != null) {
            if (response[0] != null && (response[0].equals("") || response[0].equals(response[2]))) {
	        Operators init = new Operators ();
	        String temp = init.unOptimizedOperatorReplace(response[2]);
	        riljLog("lookup RIL responseOperator() " + response[2] + " gave " + temp + " was " + response[0] + "/" + response[1] + " before.");
	        response[0] = temp;
	        response[1] = temp;
            }
        }

        return response;
    }

    private Object
    responsePhoneId() {
        return mInstanceId;
    }

    static String
    responseToString(int request)
    {
/*
 cat libs/telephony/ril_unsol_commands.h \
 | egrep "^ *{RIL_" \
 | sed -re 's/\{RIL_([^,]+),[^,]+,([^}]+).+/case RIL_\1: return "\1";/'
*/
        switch(request) {
            case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED: return "UNSOL_RESPONSE_RADIO_STATE_CHANGED";
            case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED: return "UNSOL_RESPONSE_CALL_STATE_CHANGED";
            case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED";
            case RIL_UNSOL_RESPONSE_NEW_SMS: return "UNSOL_RESPONSE_NEW_SMS";
            case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT: return "UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT";
            case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM: return "UNSOL_RESPONSE_NEW_SMS_ON_SIM";
            case RIL_UNSOL_ON_USSD: return "UNSOL_ON_USSD";
            case RIL_UNSOL_ON_USSD_REQUEST: return "UNSOL_ON_USSD_REQUEST";
            case RIL_UNSOL_NITZ_TIME_RECEIVED: return "UNSOL_NITZ_TIME_RECEIVED";
            case RIL_UNSOL_SIGNAL_STRENGTH: return "UNSOL_SIGNAL_STRENGTH";
            case RIL_UNSOL_DATA_CALL_LIST_CHANGED: return "UNSOL_DATA_CALL_LIST_CHANGED";
            case RIL_UNSOL_SUPP_SVC_NOTIFICATION: return "UNSOL_SUPP_SVC_NOTIFICATION";
            case RIL_UNSOL_STK_SESSION_END: return "UNSOL_STK_SESSION_END";
            case RIL_UNSOL_STK_PROACTIVE_COMMAND: return "UNSOL_STK_PROACTIVE_COMMAND";
            case RIL_UNSOL_STK_EVENT_NOTIFY: return "UNSOL_STK_EVENT_NOTIFY";
            case RIL_UNSOL_STK_CALL_SETUP: return "UNSOL_STK_CALL_SETUP";
            case RIL_UNSOL_SIM_SMS_STORAGE_FULL: return "UNSOL_SIM_SMS_STORAGE_FULL";
            case RIL_UNSOL_SIM_REFRESH: return "UNSOL_SIM_REFRESH";
            case RIL_UNSOL_CALL_RING: return "UNSOL_CALL_RING";
            case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED: return "UNSOL_RESPONSE_SIM_STATUS_CHANGED";
            case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS: return "UNSOL_RESPONSE_CDMA_NEW_SMS";
            case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS: return "UNSOL_RESPONSE_NEW_BROADCAST_SMS";
            case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL: return "UNSOL_CDMA_RUIM_SMS_STORAGE_FULL";
            case RIL_UNSOL_RESTRICTED_STATE_CHANGED: return "UNSOL_RESTRICTED_STATE_CHANGED";
            case RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE: return "UNSOL_ENTER_EMERGENCY_CALLBACK_MODE";
            case RIL_UNSOL_CDMA_CALL_WAITING: return "UNSOL_CDMA_CALL_WAITING";
            case RIL_UNSOL_CDMA_OTA_PROVISION_STATUS: return "UNSOL_CDMA_OTA_PROVISION_STATUS";
            case RIL_UNSOL_CDMA_INFO_REC: return "UNSOL_CDMA_INFO_REC";
            case RIL_UNSOL_OEM_HOOK_RAW: return "UNSOL_OEM_HOOK_RAW";
            case RIL_UNSOL_RINGBACK_TONE: return "UNSOL_RINGBACK_TONE";
            case RIL_UNSOL_RESEND_INCALL_MUTE: return "UNSOL_RESEND_INCALL_MUTE";
            case RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED: return "CDMA_SUBSCRIPTION_SOURCE_CHANGED";
            case RIL_UNSOl_CDMA_PRL_CHANGED: return "UNSOL_CDMA_PRL_CHANGED";
            case RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE: return "UNSOL_EXIT_EMERGENCY_CALLBACK_MODE";
            case RIL_UNSOL_RIL_CONNECTED: return "UNSOL_RIL_CONNECTED";
            case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED: return "UNSOL_VOICE_RADIO_TECH_CHANGED";
            case RIL_UNSOL_CELL_INFO_LIST: return "UNSOL_CELL_INFO_LIST";
            case RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED:
                return "UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED";
            case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED:
                    return "RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED";
            case RIL_UNSOL_SRVCC_STATE_NOTIFY:
                    return "UNSOL_SRVCC_STATE_NOTIFY";
            case RIL_UNSOL_HARDWARE_CONFIG_CHANGED: return "RIL_UNSOL_HARDWARE_CONFIG_CHANGED";
            case RIL_UNSOL_ON_SS: return "UNSOL_ON_SS";
            case RIL_UNSOL_STK_CC_ALPHA_NOTIFY: return "UNSOL_STK_CC_ALPHA_NOTIFY";
            case RIL_UNSOL_STK_SEND_SMS_RESULT: return "RIL_UNSOL_STK_SEND_SMS_RESULT";
            case RIL_UNSOL_SET_PHONE_RAT_FAMILY_COMPLETE:
                    return "RIL_UNSOL_SET_PHONE_RAT_FAMILY_COMPLETE";
            /* M: call control part start */
            case RIL_UNSOL_CALL_FORWARDING: return "UNSOL_CALL_FORWARDING";
            case RIL_UNSOL_CRSS_NOTIFICATION: return "UNSOL_CRSS_NOTIFICATION";
            case RIL_UNSOL_INCOMING_CALL_INDICATION: return "UNSOL_INCOMING_CALL_INDICATION";
            case RIL_UNSOL_CIPHER_INDICATION: return "UNSOL_CIPHER_INDICATION";
            case RIL_UNSOL_CNAP: return "UNSOL_CNAP";
            case RIL_UNSOL_SPEECH_CODEC_INFO: return "UNSOL_SPEECH_CODEC_INFO";
            /* M: call control part end */
            //MTK-START multiple application support
            case RIL_UNSOL_APPLICATION_SESSION_ID_CHANGED: return "RIL_UNSOL_APPLICATION_SESSION_ID_CHANGED";
            //MTK-END multiple application support
            case RIL_UNSOL_SIM_MISSING: return "UNSOL_SIM_MISSING";
            case RIL_UNSOL_VIRTUAL_SIM_ON: return "UNSOL_VIRTUAL_SIM_ON";
            case RIL_UNSOL_VIRTUAL_SIM_OFF: return "UNSOL_VIRTUAL_SIM_ON_OFF";
            case RIL_UNSOL_SIM_RECOVERY: return "UNSOL_SIM_RECOVERY";
            case RIL_UNSOL_SIM_PLUG_OUT: return "UNSOL_SIM_PLUG_OUT";
            case RIL_UNSOL_SIM_PLUG_IN: return "UNSOL_SIM_PLUG_IN";
            case RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED: return "RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED";
            case RIL_UNSOL_DATA_ALLOWED: return "RIL_UNSOL_DATA_ALLOWED";
            case RIL_UNSOL_PHB_READY_NOTIFICATION: return "UNSOL_PHB_READY_NOTIFICATION";
            case RIL_UNSOL_IMEI_LOCK: return "UNSOL_IMEI_LOCK";
            case RIL_UNSOL_RESPONSE_ACMT: return "UNSOL_ACMT_INFO";
            case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED";
            case RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED: return "UNSOL_RESPONSE_MMRR_STATUS_CHANGED";
            case RIL_UNSOL_NEIGHBORING_CELL_INFO: return "UNSOL_NEIGHBORING_CELL_INFO";
            case RIL_UNSOL_NETWORK_INFO: return "UNSOL_NETWORK_INFO";
            case RIL_UNSOL_INVALID_SIM: return "RIL_UNSOL_INVALID_SIM";
            case RIL_UNSOL_IMS_ENABLE_DONE: return "RIL_UNSOL_IMS_ENABLE_DONE";
            case RIL_UNSOL_IMS_DISABLE_DONE: return "RIL_UNSOL_IMS_DISABLE_DONE";
            case RIL_UNSOL_IMS_REGISTRATION_INFO: return "RIL_UNSOL_IMS_REGISTRATION_INFO";
            case RIL_UNSOL_STK_SETUP_MENU_RESET: return "RIL_UNSOL_STK_SETUP_MENU_RESET";
            case RIL_UNSOL_RESPONSE_PLMN_CHANGED: return "RIL_UNSOL_RESPONSE_PLMN_CHANGED";
            case RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED: return "RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED";
            //VoLTE
            case RIL_UNSOL_DEDICATE_BEARER_ACTIVATED: return "RIL_UNSOL_DEDICATE_BEARER_ACTIVATED";
            case RIL_UNSOL_DEDICATE_BEARER_MODIFIED: return "RIL_UNSOL_DEDICATE_BEARER_MODIFIED";
            //Remote SIM ME lock related APIs [Start]
            case RIL_UNSOL_MELOCK_NOTIFICATION: return "RIL_UNSOL_MELOCK_NOTIFICATION";
            //Remote SIM ME lock related APIs [End]
            // M: Fast Dormancy
            case RIL_UNSOL_SCRI_RESULT: return "RIL_UNSOL_SCRI_RESULT";
            case RIL_UNSOL_STK_EVDL_CALL: return "RIL_UNSOL_STK_EVDL_CALL";
            case RIL_UNSOL_STK_CALL_CTRL: return "RIL_UNSOL_STK_CALL_CTRL";

            /// M: IMS feature. @{
            case RIL_UNSOL_ECONF_SRVCC_INDICATION: return "RIL_UNSOL_ECONF_SRVCC_INDICATION";
            //For updating conference call merged/added result.
            case RIL_UNSOL_ECONF_RESULT_INDICATION: return "RIL_UNSOL_ECONF_RESULT_INDICATION";
            //For updating call mode and pau information.
            case RIL_UNSOL_CALL_INFO_INDICATION : return "RIL_UNSOL_CALL_INFO_INDICATION";
            /// @}

            case RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_INFO: return "RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_INFO";
            case RIL_UNSOL_SRVCC_HANDOVER_INFO_INDICATION: return "RIL_UNSOL_SRVCC_HANDOVER_INFO_INDICATION";
            // M: CC33 LTE.
            case RIL_UNSOL_RAC_UPDATE: return "RIL_UNSOL_RAC_UPDATE";
            case RIL_UNSOL_REMOVE_RESTRICT_EUTRAN: return "RIL_UNSOL_REMOVE_RESTRICT_EUTRAN";

            //MTK-START for MD state change
            case RIL_UNSOL_MD_STATE_CHANGE: return "RIL_UNSOL_MD_STATE_CHANGE";
            //MTK-END for MD state change

            case RIL_UNSOL_MO_DATA_BARRING_INFO: return "RIL_UNSOL_MO_DATA_BARRING_INFO";
            case RIL_UNSOL_SSAC_BARRING_INFO: return "RIL_UNSOL_SSAC_BARRING_INFO";
//MTK_TC1_FEATURE for LGE CSMCC_MO_CALL_MODIFIED {
			case RIL_UNSOL_RESPONSE_MO_CALL_STATE_CHANGED: return "UNSOL_RESPONSE_MO_CALL_STATE_CHANGED";
//}
            /* M: C2K part start */
            case RIL_UNSOL_CDMA_CALL_ACCEPTED: return "RIL_UNSOL_CDMA_CALL_ACCEPTED";
            case RIL_UNSOL_UTK_SESSION_END: return "RIL_UNSOL_UTK_SESSION_END";
            case RIL_UNSOL_UTK_PROACTIVE_COMMAND: return "RIL_UNSOL_UTK_PROACTIVE_COMMAND";
            case RIL_UNSOL_UTK_EVENT_NOTIFY: return "RIL_UNSOL_UTK_EVENT_NOTIFY";
            case RIL_UNSOL_VIA_GPS_EVENT: return "RIL_UNSOL_VIA_GPS_EVENT";
            case RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE: return "RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE";
            case RIL_UNSOL_VIA_PLMN_CHANGE_REG_PAUSE: return "RIL_UNSOL_VIA_PLMN_CHANGE_REG_PAUSE";
            case RIL_UNSOL_VIA_INVALID_SIM_DETECTED: return "RIL_UNSOL_VIA_INVALID_SIM_DETECTED";
            /* M: C2K part end */
            default: return "<unknown response> (" + request + ")";
        }
    }

    /*
     * to protect modem status we need to avoid two case :
     * 1. DTMF start -> CHLD request -> DTMF stop
     * 2. CHLD request -> DTMF request
     */
    private void handleChldRelatedRequest(RILRequest rr) {
        synchronized (mDtmfReqQueue) {
            int queueSize = mDtmfReqQueue.size();
            int i, j;
            if (queueSize > 0) {
                RILRequest rr2 = mDtmfReqQueue.get();
                if (rr2.mRequest == RIL_REQUEST_DTMF_START) {
                    // need to send the STOP command
                    if (RILJ_LOGD) riljLog("DTMF queue isn't 0, first request is START, send stop dtmf and pending switch");
                    if (queueSize > 1) {
                        j = 2;
                    } else {
                        // need to create a new STOP command
                        j = 1;
                    }
                    if (RILJ_LOGD) riljLog("queue size  " + mDtmfReqQueue.size());

                    for (i = queueSize - 1; i >= j; i--) {
                        mDtmfReqQueue.remove(i);
                    }
                    if (RILJ_LOGD) riljLog("queue size  after " + mDtmfReqQueue.size());
                    if (mDtmfReqQueue.size() == 1) { // only start command, we need to add stop command
                        RILRequest rr3 = RILRequest.obtain(RIL_REQUEST_DTMF_STOP, null);
                        if (RILJ_LOGD) riljLog("add dummy stop dtmf request");
                        mDtmfReqQueue.stop();
                        mDtmfReqQueue.add(rr3);
                    }
                }
                else {
                    // first request is STOP, just remove it and send switch
                    if (RILJ_LOGD) riljLog("DTMF queue isn't 0, first request is STOP, penging switch");
                    j = 1;
                    for (i = queueSize - 1; i >= j; i--) {
                        mDtmfReqQueue.remove(i);
                    }
                }
                mDtmfReqQueue.setPendingRequest(rr);
            } else {
                if (RILJ_LOGD) riljLog("DTMF queue is 0, send switch Immediately");
                mDtmfReqQueue.setSendChldRequest();
                send(rr);
            }
        }
    }

    private
    void setCallIndication(String[] incomingCallInfo) {
	RILRequest rr
            = RILRequest.obtain(RIL_REQUEST_SET_CALL_INDICATION, null);

	int callId = Integer.parseInt(incomingCallInfo[0]);
        int callMode = Integer.parseInt(incomingCallInfo[3]);
        int seqNumber = Integer.parseInt(incomingCallInfo[4]);

	// some guess work is needed here, for now, just 0
	callMode = 0;

        rr.mParcel.writeInt(3);

        rr.mParcel.writeInt(callMode);
        rr.mParcel.writeInt(callId);
        rr.mParcel.writeInt(seqNumber);

        if (RILJ_LOGD) riljLog(rr.serialString() + "> "
            + requestToString(rr.mRequest) + " " + callMode + " " + callId + " " + seqNumber);

        send(rr);
    }

    // Override setupDataCall as the MTK RIL needs 8th param CID (hardwired to 1?)
    @Override
    public void
    setupDataCall(String radioTechnology, String profile, String apn,
            String user, String password, String authType, String protocol,
            Message result) {
        RILRequest rr
                = RILRequest.obtain(RIL_REQUEST_SETUP_DATA_CALL, result);

        rr.mParcel.writeInt(8);

        rr.mParcel.writeString(radioTechnology);
        rr.mParcel.writeString(profile);
        rr.mParcel.writeString(apn);
        rr.mParcel.writeString(user);
        rr.mParcel.writeString(password);
        rr.mParcel.writeString(authType);
        rr.mParcel.writeString(protocol);
        rr.mParcel.writeString("1");

        if (RILJ_LOGD) riljLog(rr.serialString() + "> "
                + requestToString(rr.mRequest) + " " + radioTechnology + " "
                + profile + " " + apn + " " + user + " "
                + password + " " + authType + " " + protocol + "1");

        send(rr);
    }

    protected Object
    responseSignalStrength(Parcel p) {
        SignalStrength s = SignalStrength.makeSignalStrengthFromRilParcel(p);
	return new SignalStrength(s.getGsmSignalStrength(),
				  s.getGsmBitErrorRate(),
				  s.getCdmaDbm(),
				  s.getCdmaEcio(),
				  s.getEvdoDbm(),
				  s.getEvdoEcio(),
				  s.getEvdoSnr(),
				  true);
    }

    private void setRadioStateFromRILInt (int stateCode) {
        switch (stateCode) {
	case 0: case 1: break; // radio off
	default:
	    {
	        RILRequest rr = RILRequest.obtain(RIL_REQUEST_SET_GPRS_TRANSFER_TYPE, null);

		if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));

		rr.mParcel.writeInt(1);
		rr.mParcel.writeInt(1);

		send(rr);
	    }
	    {
	        RILRequest rr = RILRequest.obtain(RIL_REQUEST_SET_GPRS_CONNECT_TYPE, null);

		if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));

		rr.mParcel.writeInt(1);
		rr.mParcel.writeInt(1);

		send(rr);
	    }
	}
    }

    /* Oh well... this broke non-dual-SIM phones including MX4 :-/
    @Override
    public void
    setRadioPower(boolean on, Message result) {
	if ((mInstanceId != null && mInstanceId == 1)) {
		riljLog("SetRadioPower: on/off ignored on SIM2");
		return;
	}

        RILRequest rr = RILRequest.obtain(RIL_REQUEST_DUAL_SIM_MODE_SWITCH, result);

        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));

        rr.mParcel.writeInt(1);
        rr.mParcel.writeInt(on ? 3 : -1); // SIM1 | SIM2 ?
        send(rr);
    }
    */

    @Override
    public void setUiccSubscription(int slotId, int appIndex, int subId,
				    int subStatus, Message result) {
	    if (RILJ_LOGD) riljLog("setUiccSubscription" + slotId + " " + appIndex + " " + subId + " " + subStatus);

	    // Fake response (note: should be sent before mSubscriptionStatusRegistrants or
	    // SubscriptionManager might not set the readiness correctly)
	    AsyncResult.forMessage(result, 0, null);
	    result.sendToTarget();

	    // TODO: Actually turn off/on the radio (and don't fight with the ServiceStateTracker)
	    if (subStatus == 1 /* ACTIVATE */) {
		    // Subscription changed: enabled
		    if (mSubscriptionStatusRegistrants != null) {
			    mSubscriptionStatusRegistrants.notifyRegistrants(
									     new AsyncResult (null, new int[] {1}, null));
		    }
	    } else if (subStatus == 0 /* DEACTIVATE */) {
		    // Subscription changed: disabled
		    if (mSubscriptionStatusRegistrants != null) {
			    mSubscriptionStatusRegistrants.notifyRegistrants(
									     new AsyncResult (null, new int[] {0}, null));
		    }
	    }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setPreferredNetworkType(int networkType , Message response) {
        RILRequest rr = RILRequest.obtain(
                RILConstants.RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, response);

        rr.mParcel.writeInt(1);
        rr.mParcel.writeInt(networkType);

        mPreviousPreferredType = mPreferredNetworkType; //ALPS00799783
        mPreferredNetworkType = networkType;

        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                + " : " + networkType);

        send(rr);
    }

    private static int readRilMessage(InputStream is, byte[] buffer)
            throws IOException {
        int countRead;
        int offset;
        int remaining;
        int messageLength;

        // First, read in the length of the message
        offset = 0;
        remaining = 4;
        do {
            countRead = is.read(buffer, offset, remaining);

            if (countRead < 0 ) {
                Rlog.e(RILJ_LOG_TAG, "Hit EOS reading message length");
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        messageLength = ((buffer[0] & 0xff) << 24)
                | ((buffer[1] & 0xff) << 16)
                | ((buffer[2] & 0xff) << 8)
                | (buffer[3] & 0xff);

        // Then, re-use the buffer and read in the message itself
        offset = 0;
        remaining = messageLength;
        do {
            countRead = is.read(buffer, offset, remaining);

            if (countRead < 0 ) {
                Rlog.e(RILJ_LOG_TAG, "Hit EOS reading message.  messageLength=" + messageLength
                        + " remaining=" + remaining);
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        return messageLength;
    }

	protected RILReceiver createRILReceiver() {
        return new MTKRILReceiver();
    }

	protected class MTKRILReceiver extends RILReceiver {
        byte[] buffer;

        protected MTKRILReceiver() {
            buffer = new byte[RIL_MAX_COMMAND_BYTES];
        }

        @Override
        public void
        run() {
            int retryCount = 0;
            String rilSocket = "rild";

            try {for (;;) {
                LocalSocket s = null;
                LocalSocketAddress l;

                if (mInstanceId == null || mInstanceId == 0 ) {
                    rilSocket = SOCKET_NAME_RIL[0];
                } else {
                    rilSocket = SOCKET_NAME_RIL[mInstanceId];
                }

                int currentSim;
                if (mInstanceId == null || mInstanceId ==0) {
                    currentSim = 0;
                } else {
                    currentSim = mInstanceId;
                }

                int m3GsimId = 0;
                m3GsimId =  SystemProperties.getInt("gsm.3gswitch", 0);
                if((m3GsimId > 0) && (m3GsimId <= 2)) {
                    --m3GsimId;
                } else {
                    m3GsimId = 0;
                }

                if (m3GsimId >= 1) {
                    if (currentSim == 0) {
                       rilSocket = SOCKET_NAME_RIL[m3GsimId];
                    }
                    else if(currentSim == m3GsimId) {
                       rilSocket = SOCKET_NAME_RIL[0];
                    }
                    if (RILJ_LOGD) riljLog("Capability switched, swap sockets [" + currentSim + ", " + rilSocket + "]");
                }

                try {
                    s = new LocalSocket();
                    l = new LocalSocketAddress(rilSocket,
                            LocalSocketAddress.Namespace.RESERVED);
                    s.connect(l);
                } catch (IOException ex){
                    try {
                        if (s != null) {
                            s.close();
                        }
                    } catch (IOException ex2) {
                        //ignore failure to close after failure to connect
                    }

                    // don't print an error message after the the first time
                    // or after the 8th time

                    if (retryCount == 8) {
                        Rlog.e (RILJ_LOG_TAG,
                            "Couldn't find '" + rilSocket
                            + "' socket after " + retryCount
                            + " times, continuing to retry silently");
                    } else if (retryCount >= 0 && retryCount < 8) {
                        Rlog.i (RILJ_LOG_TAG,
                            "Couldn't find '" + rilSocket
                            + "' socket; retrying after timeout");
                    }

                    try {
                        Thread.sleep(SOCKET_OPEN_RETRY_MILLIS);
                    } catch (InterruptedException er) {
                    }

                    retryCount++;
                    continue;
                }

                retryCount = 0;

                mSocket = s;
                Rlog.i(RILJ_LOG_TAG, "(" + mInstanceId + ") Connected to '"
                        + rilSocket + "' socket");

                /* Compatibility with qcom's DSDS (Dual SIM) stack */
                if (needsOldRilFeature("qcomdsds")) {
                    String str = "SUB1";
                    byte[] data = str.getBytes();
                    try {
                        mSocket.getOutputStream().write(data);
                        Rlog.i(RILJ_LOG_TAG, "Data sent!!");
                    } catch (IOException ex) {
                            Rlog.e(RILJ_LOG_TAG, "IOException", ex);
                    } catch (RuntimeException exc) {
                        Rlog.e(RILJ_LOG_TAG, "Uncaught exception ", exc);
                    }
                }

                int length = 0;
                try {
                    InputStream is = mSocket.getInputStream();

                    for (;;) {
                        Parcel p;

                        length = readRilMessage(is, buffer);

                        if (length < 0) {
                            // End-of-stream reached
                            break;
                        }

                        p = Parcel.obtain();
                        p.unmarshall(buffer, 0, length);
                        p.setDataPosition(0);

                        //Rlog.v(RILJ_LOG_TAG, "Read packet: " + length + " bytes");

                        processResponse(p);
                        p.recycle();
                    }
                } catch (java.io.IOException ex) {
                    Rlog.i(RILJ_LOG_TAG, "'" + rilSocket + "' socket closed",
                          ex);
                } catch (Throwable tr) {
                    Rlog.e(RILJ_LOG_TAG, "Uncaught exception read length=" + length +
                        "Exception:" + tr.toString());
                }

                Rlog.i(RILJ_LOG_TAG, "(" + mInstanceId + ") Disconnected from '" + rilSocket
                      + "' socket");

                setRadioState (RadioState.RADIO_UNAVAILABLE);

                try {
                    mSocket.close();
                } catch (IOException ex) {
                }

                mSocket = null;
                RILRequest.resetSerial();

                // Clear request list on close
                clearRequestList(RADIO_NOT_AVAILABLE, false);
            }} catch (Throwable tr) {
                Rlog.e(RILJ_LOG_TAG,"Uncaught exception", tr);
            }

            /* We're disconnected so we don't know the ril version */
            notifyRegistrantsRilConnectionChanged(-1);
        }
    }

    /* broken by new version of MTK RIL, disable for now */
    /*
    public void handle3GSwitch() {
        int simId = mInstanceId == null ? 0 : mInstanceId;
        int newsim = SystemProperties.getInt("gsm.3gswitch", 0);
        newsim = newsim - 1;
        if(!(simId==newsim)) {
	    int prop = SystemProperties.getInt("gsm.3gswitch", 0);
            if (RILJ_LOGD) riljLog("Setting data subscription on SIM" + (simId + 1) + " mInstanceid=" + mInstanceId + " gsm.3gswitch=" + prop);
            RILRequest rr = RILRequest.obtain(RIL_REQUEST_SET_3G_CAPABILITY, null);
            rr.mParcel.writeInt(1);
            int realsim = simId + 1;
            rr.mParcel.writeInt(realsim);
            if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            send(rr);
	    try {
                Thread.sleep(1000);
            } catch (InterruptedException er) {
            }
	    resetRadio(null);
	    try {
                Thread.sleep(4*1000);
            } catch (InterruptedException er) {
            }
	}
	else {
            if (RILJ_LOGD) riljLog("Not setting data subscription on same SIM");
	}
    }

    public void setDataAllowed(boolean allowed, Message result) {
        handle3GSwitch();

        RILRequest rr = RILRequest.obtain(RIL_REQUEST_ALLOW_DATA, result);
        if (RILJ_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                + " " + allowed);

        rr.mParcel.writeInt(1);
        rr.mParcel.writeInt(allowed ? 1 : 0);
        send(rr);
    }
    */
}
