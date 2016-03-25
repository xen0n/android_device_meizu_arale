/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "eap_simaka_reauth_card.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <network/wod_channel.h>
#include <unistd.h>

#ifdef ANDROID
#include <private/android_filesystem_config.h> /* for AID_VPN */
#include <sys/capability.h>
#include <linux/prctl.h>
#include <cutils/properties.h>
#endif
typedef struct private_eap_simaka_reauth_card_t private_eap_simaka_reauth_card_t;

/**
 * Private data of an eap_simaka_reauth_card_t object.
 */
struct private_eap_simaka_reauth_card_t {

	/**
	 * Public eap_simaka_reauth_card_t interface.
	 */
	eap_simaka_reauth_card_t public;

	/**
	 * Permanent -> reauth_data_t mappings
	 */
	hashtable_t *reauth;
};

/**
 * Data associated to a reauthentication identity
 */
typedef struct {
	/** currently used reauthentication identity */
	identification_t *id;
	/** associated permanent identity */
	identification_t *permanent;
	/** counter value */
	u_int16_t counter;
	/** master key */
	char mk[HASH_SIZE_SHA1];
} reauth_data_t;

#ifdef ANDROID			 
char mtk_rand[AKA_RAND_LEN];
char mtk_auts[AKA_AUTS_LEN];
#endif

/**
 * hashtable hash function
 */
static u_int hash(identification_t *key)
{
	return chunk_hash(key->get_encoding(key));
}

/**
 * hashtable equals function
 */
static bool equals(identification_t *key1, identification_t *key2)
{
	return key1->equals(key1, key2);
}

METHOD(simaka_card_t, get_reauth, identification_t*,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char mk[HASH_SIZE_SHA1], u_int16_t *counter)
{
	reauth_data_t *data;
	identification_t *reauth;

	/* look up reauthentication data */
	data = this->reauth->remove(this->reauth, id);
	if (!data)
	{
		return NULL;
	}
	*counter = ++data->counter;
	memcpy(mk, data->mk, HASH_SIZE_SHA1);
	reauth = data->id;
	data->permanent->destroy(data->permanent);
	free(data);
	return reauth;
}

METHOD(simaka_card_t, set_reauth, void,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	identification_t* next, char mk[HASH_SIZE_SHA1], u_int16_t counter)
{
	reauth_data_t *data;

	data = this->reauth->get(this->reauth, id);
	if (data)
	{
		data->id->destroy(data->id);
	}
	else
	{
		data = malloc_thing(reauth_data_t);
		data->permanent = id->clone(id);
		this->reauth->put(this->reauth, data->permanent, data);
	}
	data->counter = counter;
	data->id = next->clone(next);
	memcpy(data->mk, mk, HASH_SIZE_SHA1);
}

static char hexdig[] = "0123456789ABCDEF";


void buf2hexstring(char *in_buf, int len, char *out_buf)
{
	int i;

	for (i = 0; i < len; i++)
	{
		out_buf[i*2]   = hexdig[(in_buf[i] >> 4) & 0xF];
		out_buf[i*2+1] = hexdig[(in_buf[i]     ) & 0xF];		
	}
}

void showbufhex(char *msg, char *buf, int len)
{
	char hexstring[1000] = {0};
	
	if (msg != NULL) {
		printf("[%s] length: %d\n", msg, len);
	}
	buf2hexstring(buf, len, hexstring);
	hexstring[2*len] = 0;	
	printf("%s\n", hexstring);
}

int atcmd_txrx(char* txbuf, int txlen, char *rxbuf, int *rxlen)
{
	int sockfd, ret, wod_rxlen;
	struct sockaddr_in servaddr,cliaddr;
	char inrxbuf[1000] = {0};

	*rxlen = 0;

	ret = wod_tcp_txrx(txbuf, txlen, inrxbuf, &wod_rxlen);
	if (ret < 0) {
		DBG1(DBG_IKE, "RX failed ... !!!");
		return -1;
	}

	if ((inrxbuf[0] == 0x0D) && (inrxbuf[1] == 0x0A)) {
		*rxlen = wod_rxlen - 12;
		while (*rxlen < (wod_rxlen - 4)) {
			if ((inrxbuf[*rxlen+2] == 0x0D) && (inrxbuf[*rxlen+3] == 0x0A)) {
				break;
			}
			(*rxlen)++;
		}
		memcpy(rxbuf, inrxbuf+2, *rxlen);
		rxbuf[*rxlen] = 0;
	}
	DBG1(DBG_IKE, "rx... got len: %d: %02X, %02X", *rxlen, (unsigned char)inrxbuf[0], (unsigned char)inrxbuf[1]);
	close(sockfd);

	return 0;
}

static unsigned char hex2bin(char hex)
{
	switch (hex)
	{
		case '0' ... '9':
			return hex - '0';
		case 'A' ... 'F':
			return hex - 'A' + 10;
		case 'a' ... 'f':
			return hex - 'a' + 10;
		default:
			return 0;
	}
}

void hexstr2buf(char *hex_buf, int hex_len, char *bin_buf)
{
	int i;

	for (i = 0; i < hex_len; i+=2)
	{
		bin_buf[i/2] = (hex2bin(hex_buf[i]) << 4) | hex2bin(hex_buf[i+1]);
	}		
}

METHOD(simaka_card_t, resync, bool,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{

	if (memcmp(rand, mtk_rand, AKA_RAND_LEN) != 0) {
		return FALSE;
	}

	memcpy(auts, mtk_auts, AKA_AUTS_LEN);
	
	return TRUE;
}

METHOD(simaka_card_t, get_triplet, bool,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	char txbuf[1000];
	char rxbuf[1000];
	char rand_hexstr[SIM_RAND_LEN*2+1];
	int rxlen;

	char at_cmd[128];
	char at_rsp[512];
	unsigned int sw1, sw2;
	char *ptr, *ptrE;
	int off, len;

	DBG1(DBG_IKE, "enter request SIM...");
	showbufhex("rand", rand, AKA_RAND_LEN);
	
	//	AT+EAUTH="F5427D46CC36495E4451E8950F641FF8"
	buf2hexstring(rand, SIM_RAND_LEN, rand_hexstr);
	rand_hexstr[SIM_RAND_LEN*2] = 0;
	sprintf(txbuf, "AT+EAUTH=\"%s\"\r", rand_hexstr);

	DBG1(DBG_IKE, "Send AT cmd: %s", txbuf);
	atcmd_txrx(txbuf, strlen(txbuf), rxbuf, &rxlen);
	rxbuf[rxlen] = 0;
	
	DBG1(DBG_IKE, "Got response: %s", rxbuf);
	if (strncmp(rxbuf, "+EAUTH:", 7) == 0) {
		//	+EAUTH: 144, 0, "04884F7F0308963011BF1F04C0A9"
		//	got SW1
		ptr = ptrE = rxbuf+7;
		while (*ptrE != ',')
			ptrE++;
		*ptrE = 0;		
		sw1 = atol(ptr);
		
		//	got SW2
		ptr = ptrE + 1;
		while (*ptrE != ',')
			ptrE++;
		*ptrE = 0;
		sw2 = atol(ptr);
		
		//	got Response
		ptr = ptrE + 1;
		while (*ptrE != '\"')
			ptrE++;
		ptr = ptrE + 1;
		ptrE = ptr+1;
		while (*ptrE != '\"')
			ptrE++;
		*ptrE = 0;
		strcpy(at_rsp, ptr);		
		DBG1(DBG_IKE, " SW[%02X%02X]: %s (%d)", sw1, sw2, at_rsp, strlen(at_rsp));

		rxlen = strlen(at_rsp);
		
		hexstr2buf(at_rsp, rxlen, rxbuf);
		rxlen /= 2;
		showbufhex("at_rsp", rxbuf, rxlen);
	
		//	parse response
		//	offset 0	: Len of sRES, Ls
		//	offset 1	: sRES
		//	offset 1+Ls	: Len of Kc, Lk
		//	offset 2+Ls : Kc

		off = 0;
		//	sRES
		len = rxbuf[off++];
		if ((off + len) > rxlen) {
			return FALSE;
		}
		memcpy(sres, rxbuf+off, len);
		off += len;
		showbufhex("sRES", sres, len);
			
		//	Kc
		len = rxbuf[off++];
		if ((off + len) > rxlen) {
			return FALSE;
		}
		memcpy(kc, rxbuf+off, len);
		off += len;
		showbufhex("Kc", kc, len);

		return TRUE;
	}
	return FALSE;
}

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	char txbuf[1000];
	char rxbuf[1000];
	char rand_hexstr[AKA_RAND_LEN*2+1];
	char autn_hexstr[AKA_AUTN_LEN*2+1];
	int rxlen;

	char at_cmd[128];
	char at_rsp[512];
	unsigned int sw1, sw2;
	char *ptr, *ptrE;
	int off, len;
	
	DBG1(DBG_IKE, "enter...");
	showbufhex("rand", rand, AKA_RAND_LEN);
	showbufhex("autn", autn, AKA_AUTN_LEN);

	//	AT+EAUTH="F5427D46CC36495E4451E8950F641FF8","B11B447B41230000F42709D4CBD0FE99"
	buf2hexstring(rand, AKA_RAND_LEN, rand_hexstr);
	rand_hexstr[AKA_RAND_LEN*2] = 0;
	buf2hexstring(autn, AKA_AUTN_LEN, autn_hexstr);
	autn_hexstr[AKA_AUTN_LEN*2] = 0;
	sprintf(txbuf, "AT+EAUTH=\"%s\",\"%s\"\r", rand_hexstr, autn_hexstr);

	DBG1(DBG_IKE, "Send AT cmd: %s", txbuf);
	atcmd_txrx(txbuf, strlen(txbuf), rxbuf, &rxlen);
	rxbuf[rxlen] = 0;
	
	DBG1(DBG_IKE, "Got response: %s", rxbuf);
	if (strncmp(rxbuf, "+EAUTH:", 7) == 0) {
		//	+EAUTH: 144, 0, "DB08E767CDCB6F28B2C810338F1E9225349166298D69D35C53A3EC1081AB62AF8D0632660D990451EB65C04508963011BF1F04C0A9"
		//	got SW1
		ptr = ptrE = rxbuf+7;
		while (*ptrE != ',')
			ptrE++;
		*ptrE = 0;		
		sw1 = atol(ptr);
		
		//	got SW2
		ptr = ptrE + 1;
		while (*ptrE != ',' && ptrE <= (rxbuf+rxlen))
			ptrE++;
		if (*ptrE == ',')
		{
			*ptrE = 0;
			sw2 = atol(ptr);			
			//	got Response
			ptr = ptrE + 1;
			while (*ptrE != '\"')
				ptrE++;
			ptr = ptrE + 1;
			ptrE = ptr+1;
			while (*ptrE != '\"')
				ptrE++;
			*ptrE = 0;
			strcpy(at_rsp, ptr);		
			DBG1(DBG_IKE, " SW[%02X%02X]: %s (%d)", sw1, sw2, at_rsp, strlen(at_rsp));

			rxlen = strlen(at_rsp);
			
			hexstr2buf(at_rsp, rxlen, rxbuf);
			rxlen /= 2;

			showbufhex("at_rsp", rxbuf, rxlen);		
			//	parse response
		} else {
			*ptrE = 0;
			sw2 = atol(ptr);			
			DBG1(DBG_IKE, " SW[%02X%02X]", sw1, sw2);
			rxbuf[0] = 0;
		}
		
		off = 0;
		
		if ((unsigned char)rxbuf[off] == 0xDB) {
			DBG1(DBG_IKE, "Go DB");			
			// 	Successful 3G authentication
			//	offset 0	: 0xDB
			//	offset 1	: Len of RES, Lr
			//	offset 2	: RES
			//	offset 2+Lr	: Len of CK, Lc
			//	offset 3+Lr: CK
			//	offset 3+Lr+Lc: Len of IK, Li
			//	offset 4+Lr+Lc: IK
			off++;
			//	RES
			len = rxbuf[off++];
			memcpy(res, rxbuf+off, len);
			off += len;
			*res_len = len;
			showbufhex("RES", res, len);
				
			//	CK
			len = rxbuf[off++];
			memcpy(ck, rxbuf+off, len);
			off += len;
			showbufhex("CK", ck, len);

			//	IK
			len = rxbuf[off++];
			memcpy(ik, rxbuf+off, len);
			off += len;
			showbufhex("IK", ik, len);

			return SUCCESS;
		} else if ((unsigned char)rxbuf[off] == 0xDC) {
			/*
			   When verifying AUTN, the identity module may detect that the sequence
			   number the network uses is not within the correct range.  In this
			   case, the identity module calculates a sequence number
			   synchronization parameter AUTS and sends it to the network.  AKA
			   authentication may then be retried with a new authentication vector
			   generated using the synchronized sequence number.
			*/
			//	Synchronisation failure
			//	offset 0	: 0xDC
			//	offset 1	: Len of AUTS
			//	offset 2	: RES
			DBG1(DBG_IKE, "Go DC");			

			off++;
			//	AUTS
			len = rxbuf[off++];
			memcpy(mtk_auts, rxbuf+off, len);
			off += len;
			showbufhex("AUTS", mtk_auts, len);
			memcpy(mtk_rand, rand, AKA_RAND_LEN);
			
			return INVALID_STATE;
		} else {
			DBG1(DBG_IKE, "Go Other");			
		}		
	}
	
	return FAILED;
}

METHOD(eap_simaka_reauth_card_t, destroy, void,
	private_eap_simaka_reauth_card_t *this)
{
	enumerator_t *enumerator;
	reauth_data_t *data;
	void *key;

	enumerator = this->reauth->create_enumerator(this->reauth);
	while (enumerator->enumerate(enumerator, &key, &data))
	{
		data->id->destroy(data->id);
		data->permanent->destroy(data->permanent);
		free(data);
	}
	enumerator->destroy(enumerator);

	this->reauth->destroy(this->reauth);
	free(this);
}

/**
 * See header
 */
eap_simaka_reauth_card_t *eap_simaka_reauth_card_create()
{
	private_eap_simaka_reauth_card_t *this;
#ifndef ANDROID			 
	FILE *pFile;
#endif

	if (!lib->caps->check(lib->caps, CAP_NET_BIND_SERVICE))
	{	/* Bind a socket to Internet domain privileged ports (port numbers less than 1024). */
		DBG1(DBG_NET, "eap_simaka_reauth plugin requires CAP_NET_BIND_SERVICE capability");
		return NULL;
	}
	else if (!lib->caps->keep(lib->caps, CAP_NET_RAW))
	{	/* use RAW and PACKET sockets */
		/* bind to any address for transparent proxying */
		DBG1(DBG_NET, "eap_simaka_reauth plugin requires CAP_NET_RAW capability");
		return NULL;
	}

	INIT(this,
		.public = {
			.card = {
				.get_triplet = _get_triplet,
				.get_quintuplet = _get_quintuplet,
				.resync = _resync,
				.get_pseudonym = (void*)return_null,
				.set_pseudonym = (void*)nop,
				.get_reauth = _get_reauth,
				.set_reauth = _set_reauth,
			},
			.destroy = _destroy,
		},
		.reauth = hashtable_create((void*)hash, (void*)equals, 0),
	);

	return &this->public;
}

