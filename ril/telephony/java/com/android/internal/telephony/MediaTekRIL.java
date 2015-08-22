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
import android.telephony.Rlog;
import android.telephony.SignalStrength;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

import java.io.IOException;
import java.io.InputStream;

import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;

public class MediaTekRIL extends RIL implements CommandsInterface {

    // MediaTek Custom States
    static final int RIL_REQUEST_MTK_BASE = 2000;
    static final int RIL_REQUEST_HANGUP_ALL = (RIL_REQUEST_MTK_BASE + 0);
    static final int RIL_REQUEST_GET_COLP = (RIL_REQUEST_MTK_BASE + 1);
    static final int RIL_REQUEST_SET_COLP = (RIL_REQUEST_MTK_BASE + 2);
    static final int RIL_REQUEST_GET_COLR = (RIL_REQUEST_MTK_BASE + 3);
    static final int RIL_REQUEST_GET_CCM = (RIL_REQUEST_MTK_BASE + 4);
    static final int RIL_REQUEST_GET_ACM = (RIL_REQUEST_MTK_BASE + 5);
    static final int RIL_REQUEST_GET_ACMMAX = (RIL_REQUEST_MTK_BASE + 6);
    static final int RIL_REQUEST_GET_PPU_AND_CURRENCY = (RIL_REQUEST_MTK_BASE + 7);
    static final int RIL_REQUEST_SET_ACMMAX = (RIL_REQUEST_MTK_BASE + 8);
    static final int RIL_REQUEST_RESET_ACM = (RIL_REQUEST_MTK_BASE + 9);
    static final int RIL_REQUEST_SET_PPU_AND_CURRENCY = (RIL_REQUEST_MTK_BASE + 10);
    static final int RIL_REQUEST_RADIO_POWEROFF = (RIL_REQUEST_MTK_BASE + 11);       
    static final int RIL_REQUEST_DUAL_SIM_MODE_SWITCH = (RIL_REQUEST_MTK_BASE + 12); 
    static final int RIL_REQUEST_QUERY_PHB_STORAGE_INFO = (RIL_REQUEST_MTK_BASE + 13);       
    static final int RIL_REQUEST_WRITE_PHB_ENTRY = (RIL_REQUEST_MTK_BASE + 14);      
    static final int RIL_REQUEST_READ_PHB_ENTRY = (RIL_REQUEST_MTK_BASE + 15);       
    static final int RIL_REQUEST_SET_GPRS_CONNECT_TYPE = (RIL_REQUEST_MTK_BASE + 16);
    static final int RIL_REQUEST_SET_GPRS_TRANSFER_TYPE = (RIL_REQUEST_MTK_BASE + 17);
    static final int RIL_REQUEST_MOBILEREVISION_AND_IMEI = (RIL_REQUEST_MTK_BASE + 18);//Add by mtk80372 for Barcode Number
    static final int RIL_REQUEST_QUERY_SIM_NETWORK_LOCK = (RIL_REQUEST_MTK_BASE + 19);
    static final int RIL_REQUEST_SET_SIM_NETWORK_LOCK = (RIL_REQUEST_MTK_BASE + 20);
    static final int RIL_REQUEST_SET_SCRI = (RIL_REQUEST_MTK_BASE + 21);   
    /* cage_vt start */
    static final int RIL_REQUEST_VT_DIAL = (RIL_REQUEST_MTK_BASE + 22);
    static final int RIL_REQUEST_VOICE_ACCEPT = (RIL_REQUEST_MTK_BASE + 32);
    /* cage_vt end */
    static final int RIL_REQUEST_BTSIM_CONNECT = (RIL_REQUEST_MTK_BASE + 23);
    static final int RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF = (RIL_REQUEST_MTK_BASE + 24);
    static final int RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM = (RIL_REQUEST_MTK_BASE + 25);
    static final int RIL_REQUEST_BTSIM_TRANSFERAPDU = (RIL_REQUEST_MTK_BASE + 26);
    static final int RIL_REQUEST_EMERGENCY_DIAL = (RIL_REQUEST_MTK_BASE + 27);
    static final int RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT = (RIL_REQUEST_MTK_BASE + 28);
    static final int RIL_REQUEST_QUERY_ICCID = (RIL_REQUEST_MTK_BASE + 29);
    static final int RIL_REQUEST_SIM_AUTHENTICATION = (RIL_REQUEST_MTK_BASE + 30);   
    static final int RIL_REQUEST_USIM_AUTHENTICATION = (RIL_REQUEST_MTK_BASE + 31); 
    static final int RIL_REQUEST_RADIO_POWERON = (RIL_REQUEST_MTK_BASE + 33);
    static final int RIL_REQUEST_GET_SMS_SIM_MEM_STATUS = (RIL_REQUEST_MTK_BASE + 34);
    static final int RIL_REQUEST_FORCE_RELEASE_CALL = (RIL_REQUEST_MTK_BASE + 35);
    static final int RIL_REQUEST_SET_CALL_INDICATION = (RIL_REQUEST_MTK_BASE + 36);
    static final int RIL_REQUEST_REPLACE_VT_CALL = (RIL_REQUEST_MTK_BASE + 37);
    /* 3G switch start */
    static final int RIL_REQUEST_GET_3G_CAPABILITY = (RIL_REQUEST_MTK_BASE + 38);
    static final int RIL_REQUEST_SET_3G_CAPABILITY = (RIL_REQUEST_MTK_BASE + 39);
    /* 3G switch end */
    /* User controlled PLMN selector with Access Technology  begin */
    static final int RIL_REQUEST_GET_POL_CAPABILITY = (RIL_REQUEST_MTK_BASE + 40);
    static final int RIL_REQUEST_GET_POL_LIST = (RIL_REQUEST_MTK_BASE + 41);
    static final int RIL_REQUEST_SET_POL_ENTRY = (RIL_REQUEST_MTK_BASE + 42);
    /* User controlled PLMN selector with Access Technology  end */
    /* UPB start */
    static final int RIL_REQUEST_QUERY_UPB_CAPABILITY = (RIL_REQUEST_MTK_BASE + 43);
    static final int RIL_REQUEST_EDIT_UPB_ENTRY = (RIL_REQUEST_MTK_BASE + 44);
    static final int RIL_REQUEST_DELETE_UPB_ENTRY = (RIL_REQUEST_MTK_BASE + 45);
    static final int RIL_REQUEST_READ_UPB_GAS_LIST = (RIL_REQUEST_MTK_BASE + 46);
    static final int RIL_REQUEST_READ_UPB_GRP = (RIL_REQUEST_MTK_BASE + 47);
    static final int RIL_REQUEST_WRITE_UPB_GRP = (RIL_REQUEST_MTK_BASE + 48);
    /* UPB end */
    static final int RIL_REQUEST_DISABLE_VT_CAPABILITY = (RIL_REQUEST_MTK_BASE + 49);
    static final int RIL_REQUEST_HANGUP_ALL_EX = (RIL_REQUEST_MTK_BASE + 50);
    static final int RIL_REQUEST_SET_SIM_RECOVERY_ON = (RIL_REQUEST_MTK_BASE + 51);
    static final int RIL_REQUEST_GET_SIM_RECOVERY_ON = (RIL_REQUEST_MTK_BASE + 52);
    static final int RIL_REQUEST_SET_TRM = (RIL_REQUEST_MTK_BASE + 53);
    static final int RIL_REQUEST_DETECT_SIM_MISSING = (RIL_REQUEST_MTK_BASE + 54);
    static final int RIL_REQUEST_GET_CALIBRATION_DATA = (RIL_REQUEST_MTK_BASE + 55);

     //For LGE APIs start
    static final int RIL_REQUEST_GET_PHB_STRING_LENGTH = (RIL_REQUEST_MTK_BASE + 56);
    static final int RIL_REQUEST_GET_PHB_MEM_STORAGE = (RIL_REQUEST_MTK_BASE + 57);
    static final int RIL_REQUEST_SET_PHB_MEM_STORAGE = (RIL_REQUEST_MTK_BASE + 58);
    static final int RIL_REQUEST_READ_PHB_ENTRY_EXT = (RIL_REQUEST_MTK_BASE + 59);
    static final int RIL_REQUEST_WRITE_PHB_ENTRY_EXT = (RIL_REQUEST_MTK_BASE + 60);
    
    // requests for read/write EFsmsp
    static final int RIL_REQUEST_GET_SMS_PARAMS = (RIL_REQUEST_MTK_BASE + 61);
    static final int RIL_REQUEST_SET_SMS_PARAMS = (RIL_REQUEST_MTK_BASE + 62);

    // NFC SEEK start
    static final int RIL_REQUEST_SIM_TRANSMIT_BASIC = (RIL_REQUEST_MTK_BASE + 63);
    static final int RIL_REQUEST_SIM_OPEN_CHANNEL = (RIL_REQUEST_MTK_BASE + 64);
    static final int RIL_REQUEST_SIM_CLOSE_CHANNEL = (RIL_REQUEST_MTK_BASE + 65);
    static final int RIL_REQUEST_SIM_TRANSMIT_CHANNEL = (RIL_REQUEST_MTK_BASE + 66);
    static final int RIL_REQUEST_SIM_GET_ATR = (RIL_REQUEST_MTK_BASE + 67);
    // NFC SEEK end

    // CB extension
    static final int RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO = (RIL_REQUEST_MTK_BASE + 68);
    static final int RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO = (RIL_REQUEST_MTK_BASE + 69);
    static final int RIL_REQUEST_GET_CB_CONFIG_INFO = (RIL_REQUEST_MTK_BASE + 70);
    static final int RIL_REQUEST_SET_ALL_CB_LANGUAGE_ON = (RIL_REQUEST_MTK_BASE + 71);
    // CB extension
    
    static final int RIL_REQUEST_SET_ETWS = (RIL_REQUEST_MTK_BASE + 72);

    // [New R8 modem FD]
    static final int RIL_REQUEST_SET_FD_MODE = (RIL_REQUEST_MTK_BASE + 73);

    static final int RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW = (RIL_REQUEST_MTK_BASE + 74); // NFC SEEK

    static final int RIL_REQUEST_SET_CLIP = (RIL_REQUEST_MTK_BASE + 75);

    //MTK-START [mtk80776] WiFi Calling
    static final int RIL_REQUEST_UICC_SELECT_APPLICATION = (RIL_REQUEST_MTK_BASE + 76);
    static final int RIL_REQUEST_UICC_DEACTIVATE_APPLICATION = (RIL_REQUEST_MTK_BASE + 77);
    static final int RIL_REQUEST_UICC_APPLICATION_IO = (RIL_REQUEST_MTK_BASE + 78);
    static final int RIL_REQUEST_UICC_AKA_AUTHENTICATE = (RIL_REQUEST_MTK_BASE + 79);
    static final int RIL_REQUEST_UICC_GBA_AUTHENTICATE_BOOTSTRAP = (RIL_REQUEST_MTK_BASE + 80);
    static final int RIL_REQUEST_UICC_GBA_AUTHENTICATE_NAF = (RIL_REQUEST_MTK_BASE + 81);
    //MTK-END [mtk80776] WiFi Calling
    static final int RIL_REQUEST_STK_EVDL_CALL_BY_AP = (RIL_REQUEST_MTK_BASE + 82);

    static final int RIL_UNSOL_MTK_BASE = 3000; 
    static final int RIL_UNSOL_NEIGHBORING_CELL_INFO = (RIL_UNSOL_MTK_BASE + 0);
    static final int RIL_UNSOL_NETWORK_INFO = (RIL_UNSOL_MTK_BASE + 1);
    static final int RIL_UNSOL_CALL_FORWARDING = (RIL_UNSOL_MTK_BASE + 2);
    static final int RIL_UNSOL_CRSS_NOTIFICATION = (RIL_UNSOL_MTK_BASE + 3);
    static final int RIL_UNSOL_CALL_PROGRESS_INFO = (RIL_UNSOL_MTK_BASE + 4);
    static final int RIL_UNSOL_PHB_READY_NOTIFICATION = (RIL_UNSOL_MTK_BASE + 5);
    static final int RIL_UNSOL_SPEECH_INFO = (RIL_UNSOL_MTK_BASE + 6);
    static final int RIL_UNSOL_SIM_INSERTED_STATUS = (RIL_UNSOL_MTK_BASE + 7);
    static final int RIL_UNSOL_RADIO_TEMPORARILY_UNAVAILABLE = (RIL_UNSOL_MTK_BASE + 8);
    static final int RIL_UNSOL_ME_SMS_STORAGE_FULL = (RIL_UNSOL_MTK_BASE + 9);
    static final int RIL_UNSOL_SMS_READY_NOTIFICATION = (RIL_UNSOL_MTK_BASE + 10);
    static final int RIL_UNSOL_SCRI_RESULT = (RIL_UNSOL_MTK_BASE + 11);
    /* cage_vt start */
    static final int RIL_UNSOL_VT_STATUS_INFO = (RIL_UNSOL_MTK_BASE + 12);
    static final int RIL_UNSOL_VT_RING_INFO = (RIL_UNSOL_MTK_BASE + 13);
    /* cage_vt end */
    static final int RIL_UNSOL_INCOMING_CALL_INDICATION = (RIL_UNSOL_MTK_BASE + 14);
    static final int RIL_UNSOL_SIM_MISSING = (RIL_UNSOL_MTK_BASE + 15);
    static final int RIL_UNSOL_GPRS_DETACH = (RIL_UNSOL_MTK_BASE + 16);
    //MTK-START [mtk04070][120208][ALPS00233196] ATCI for unsolicited response
    static final int RIL_UNSOL_ATCI_RESPONSE = (RIL_UNSOL_MTK_BASE + 17);
    //MTK-END [mtk04070][120208][ALPS00233196] ATCI for unsolicited response
    static final int RIL_UNSOL_SIM_RECOVERY= (RIL_UNSOL_MTK_BASE + 18);
    static final int RIL_UNSOL_VIRTUAL_SIM_ON = (RIL_UNSOL_MTK_BASE + 19);
    static final int RIL_UNSOL_VIRTUAL_SIM_OFF = (RIL_UNSOL_MTK_BASE + 20);
    static final int RIL_UNSOL_INVALID_SIM = (RIL_UNSOL_MTK_BASE + 21); 
    static final int RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED = (RIL_UNSOL_MTK_BASE + 22);
    static final int RIL_UNSOL_RESPONSE_ACMT = (RIL_UNSOL_MTK_BASE + 23);
    static final int RIL_UNSOL_EF_CSP_PLMN_MODE_BIT = (RIL_UNSOL_MTK_BASE + 24);
    static final int RIL_UNSOL_IMEI_LOCK = (RIL_UNSOL_MTK_BASE + 25);
    static final int RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED = (RIL_UNSOL_MTK_BASE + 26);
    static final int RIL_UNSOL_SIM_PLUG_OUT = (RIL_UNSOL_MTK_BASE + 27);
    static final int RIL_UNSOL_SIM_PLUG_IN = (RIL_UNSOL_MTK_BASE + 28);
    static final int RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION = (RIL_UNSOL_MTK_BASE + 29);
    static final int RIL_UNSOL_CNAP = (RIL_UNSOL_MTK_BASE + 30);
    static final int RIL_UNSOL_STK_EVDL_CALL = (RIL_UNSOL_MTK_BASE + 31);

    // TODO: Support multiSIM
    // Sim IDs are 0 / 1
    int mSimId = 0;


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

        if (strings.length % 5 != 0) {
            throw new RuntimeException(
                "RIL_REQUEST_QUERY_AVAILABLE_NETWORKS: invalid response. Got "
                + strings.length + " strings, expected multible of 5");
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

        ret = new ArrayList<OperatorInfo>(strings.length / 5);

        for (int i = 0 ; i < strings.length ; i += 5) {
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

        Object ret = null;

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
            case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND: ret =  responseVoid(p); break;
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
            case RIL_REQUEST_DEACTIVATE_DATA_CALL: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_FACILITY_LOCK: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_FACILITY_LOCK: ret =  responseInts(p); break;
            case RIL_REQUEST_CHANGE_BARRING_PASSWORD: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE: ret =  responseInts(p); break;
            case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC: ret =  responseVoid(p); break;
            case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL: ret =  responseVoid(p); break;
            case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS : ret =  responseOperatorInfos(p); break;
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
            case 104: ret = responseInts(p); break; // RIL_REQUEST_VOICE_RADIO_TECH
            case 105: ret = responseInts(p); break; // RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
            case 106: ret = responseStrings(p); break; // RIL_REQUEST_CDMA_PRL_VERSION
            case 107: ret = responseInts(p);  break; // RIL_REQUEST_IMS_REGISTRATION_STATE
            case RIL_REQUEST_VOICE_RADIO_TECH: ret = responseInts(p); break;
	    case RIL_REQUEST_SET_3G_CAPABILITY: ret =  responseInts(p); break;

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

        if (error != 0) {
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
            case RIL_UNSOL_CALL_FORWARDING: ret = responseInts(p); break;
            case RIL_UNSOL_CRSS_NOTIFICATION: ret = responseCrssNotification(p); break;
            case RIL_UNSOL_CALL_PROGRESS_INFO: ret = responseStrings(p); break;         
            case RIL_UNSOL_PHB_READY_NOTIFICATION: ret = responseVoid(p); break;
            case RIL_UNSOL_SIM_INSERTED_STATUS: ret = responseInts(p); break;            
            case RIL_UNSOL_SIM_MISSING: ret = responseInts(p); break;   
            case RIL_UNSOL_SIM_RECOVERY: ret = responseInts(p); break;         
            case RIL_UNSOL_VIRTUAL_SIM_ON: ret = responseInts(p); break; 
            case RIL_UNSOL_VIRTUAL_SIM_OFF: ret = responseInts(p); break; 
            case RIL_UNSOL_SPEECH_INFO: ret = responseInts(p); break;           
            case RIL_UNSOL_RADIO_TEMPORARILY_UNAVAILABLE: ret = responseInts(p); break; 
            case RIL_UNSOL_ME_SMS_STORAGE_FULL: ret =  responseVoid(p); break;
            case RIL_UNSOL_SMS_READY_NOTIFICATION: ret = responseVoid(p); break;
            case RIL_UNSOL_VT_STATUS_INFO: ret = responseInts(p); break;
            case RIL_UNSOL_VT_RING_INFO: ret = responseVoid(p); break;
            case RIL_UNSOL_SCRI_RESULT: ret = responseInts(p); break;
            case RIL_UNSOL_GPRS_DETACH: ret = responseVoid(p); break;
            case RIL_UNSOL_INCOMING_CALL_INDICATION: ret = responseStrings(p); break;
            case RIL_UNSOL_EF_CSP_PLMN_MODE_BIT: ret = responseInts(p); break;
            case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED: ret =  responseVoid(p); break;
            case RIL_UNSOL_INVALID_SIM:  ret = responseStrings(p); break;
            case RIL_UNSOL_RESPONSE_ACMT: ret = responseInts(p); break;
            case RIL_UNSOL_IMEI_LOCK: ret = responseVoid(p); break;
            case RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED: ret = responseInts(p); break;
            case RIL_UNSOL_SIM_PLUG_OUT: ret = responseInts(p); break;
            case RIL_UNSOL_SIM_PLUG_IN: ret = responseInts(p); break;
            case RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION: ret = responseEtwsNotification(p); break;
            case RIL_UNSOL_CNAP: ret = responseStrings(p); break;
            case RIL_UNSOL_STK_EVDL_CALL: ret = responseInts(p); break;            
            case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED: ret =  responseVoid(p); break;
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
            case RIL_UNSOL_CALL_PROGRESS_INFO:
		rewindAndReplace = true;
		newResponseCode = RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED;
		break;

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
                /*if (mGsmSmsRegistrant != null) {
                    mGsmSmsRegistrant
                        .notifyRegistrant();
                }*/
                break;
            case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
		// intercept and send GPRS_TRANSFER_TYPE and GPRS_CONNECT_TYPE to RIL
	        setRadioStateFromRILInt(p.readInt());
		rewindAndReplace = true;
		newResponseCode = RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED;
		break;
            default:
                Rlog.i(RILJ_LOG_TAG, "Unprocessed unsolicited known MTK response: " + response);
        }

        if (rewindAndReplace) {
            Rlog.w(RILJ_LOG_TAG, "Rewriting MTK unsolicited response to " + newResponseCode);

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
            case RIL_REQUEST_GET_DATA_CALL_PROFILE: return "RIL_REQUEST_GET_DATA_CALL_PROFILE";
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
	    case RIL_REQUEST_SET_3G_CAPABILITY: return "RIL_REQUEST_SET_3G_CAPABILITY";
            default: return "<unknown request>";
        }
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

        if((response[0] != null) && (response[0].startsWith("uCs2") == true))
        {        
            riljLog("responseOperator handling UCS2 format name");			        
            try{	
                response[0] = new String(hexStringToBytes(response[0].substring(4)),"UTF-16");
            }catch(UnsupportedEncodingException ex){
                riljLog("responseOperatorInfos UnsupportedEncodingException");
            }			
        }
		
        if (response[0] != null && (response[0].equals("") || response[0].equals(response[2]))) {
	    Operators init = new Operators ();
	    String temp = init.unOptimizedOperatorReplace(response[2]);
	    riljLog("lookup RIL responseOperator() " + response[2] + " gave " + temp + " was " + response[0] + "/" + response[1] + " before.");
	    response[0] = temp;
	    response[1] = temp;
        }

        return response;
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
}
