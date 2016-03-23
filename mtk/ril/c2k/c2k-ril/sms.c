/* //device/system/reference-ril/misc.h
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/
#define LOG_TAG "C2K_RIL"
#include <utils/Log.h>
#include "sms.h"

void RILCdmaSmsPdu (RIL_CDMA_SMS_Message *SmsMsg, unsigned char *PduP, unsigned char *nlength)
{
    if ( SmsMsg == NULL || PduP == NULL || nlength == NULL )
    {

        return;
    }

    PduP[*nlength] = 0; (*nlength)++; //SMS_MSG_TYPE_POINT_TO_POINT

    *nlength += MakeTeleSrvId(PduP+*nlength, SmsMsg->uTeleserviceID);

    *nlength += MakeAddress( PduP+*nlength, &(SmsMsg->sAddress), 1);

    return;
}

unsigned char MakeTeleSrvId(unsigned char *data, int teleserviceId)
{
    putuint8(data, 0, 8, 0);
    putuint8(data, 8, 8, 2);
    putuint16(data, 16, 16, teleserviceId);
    return( 4 );
}

unsigned char MakeAddress(unsigned char *data, RIL_CDMA_SMS_Address *addr, int MoSmsMsg)
{
    unsigned char i, size, byte, totalLength, result;
    unsigned short getBit=0, startBit=0;

    //IASSERT( addr->numFields <= SMS_MAX_ADDRESS_CHARS );
    if(MoSmsMsg == 1) //hyang: differential the MO address and MT address,
                 //in PSW there is only one address field for MO and MT message
        putuint8(data, startBit, 8, 4); 
    else 
        putuint8(data, startBit, 8, 2);
    startBit += 16;      /* skip over parameter length for now */
    putuint8(data, startBit, 1, addr->digit_mode); startBit++;
    putuint8(data, startBit, 1, addr->number_mode); startBit++;

    if(addr->digit_mode)
    {
        /* 8 bit unspecified address characters */
	    size = 8;
	    /* numberType is present */
	    putuint8(data, startBit, 3, addr->number_type); startBit += 3;

	    if( addr->number_mode == 0 )
	    {
	        /* numberPlan is here too */
	        putuint8(data, startBit, 4, addr->number_plan); startBit += 4;
	    }
    }
    else 
    {
        /* DTMF codes for address */
	    size = 4;
    }

    /* Put NUM_FIELDs in message then increment count */
    putuint8(data, startBit, 8, addr->number_of_digits); startBit += 8;
    for(i=0; i<addr->number_of_digits;i++)
    {
      byte = getuint8(addr->digits, getBit, size);
	    putuint8(data, startBit, size, byte); 
	    startBit += size;
	    getBit += size;
    }
    totalLength = NUM_BYTES(startBit);

    /*   pad the data with 0s  */
    result = (totalLength*8) - startBit;
    if(result)
    {
        putuint8(data, startBit, result , 0);
    }
    /* set the parameter length field */
    putuint8(data, 8, 8, totalLength - 2);

    /* Return the number of octets used */
    return(totalLength);
}

void putuint8( unsigned char *data, unsigned short startBit, unsigned char numBits, unsigned char value )
{
    unsigned char startByteIndex;
    unsigned char startBitIndex;   /* Index within the start byte */
    int  bitsIn2ndByte;
    int  lowBitsSave1stByte;
    unsigned char temp;
    unsigned char mask;

    startByteIndex = (unsigned char)(startBit >> 3);
    startBitIndex  = (unsigned char)(startBit & 0x7);

    /* Two cases - all the bits are in a single byte, or the value to be
     * written spans two bytes.
     */
    bitsIn2ndByte = startBitIndex + numBits - 8;

    if( bitsIn2ndByte > 0 )
      {
        /* value spans 2 bytes */
        /* First, zero out the top bits of the first byte */
        mask = _saveTopMask[ startBitIndex ];
        data[ startByteIndex ] &= mask;

        /* Or in the most significant bits of the value */
        data[ startByteIndex ] |= (~mask & (value >> bitsIn2ndByte));

        /* Now zero out the high bits of the low byte */
        data[ startByteIndex + 1 ] &= ~_saveTopMask[ (unsigned char) bitsIn2ndByte ];
        
        /* Or in the least significant bits of the value */
        data[ startByteIndex + 1 ] |= (value << (8 - bitsIn2ndByte));
      }
    else
      {
        /* set the mask bits */

        lowBitsSave1stByte = -bitsIn2ndByte;

        mask = _valueMask[numBits] << lowBitsSave1stByte;

        /* Zero out the bits we want to replace, i.e. put the bits we
         * want to keep in Temp */

        temp  = data[ startByteIndex ] & ~mask;

        /* Now or in the new bits with the ones we're keeping */
        data[ startByteIndex ] = temp | ( mask & (value << lowBitsSave1stByte));
      }

}
  
void putuint16( unsigned char *data, unsigned short startBit, unsigned char numBits, unsigned short value )
{
    /* Make assertions here */

    putuint8(data, startBit,  8, (unsigned char)(value >> (numBits -= 8)) );
    putuint8(data, startBit+8, numBits, (unsigned char)(value & _valueMask[numBits]) );
}

unsigned char getuint8( unsigned char *data, unsigned short startBit, unsigned char numBits )
  {
    unsigned char startByteIndex;
    unsigned char startBitIndex;   /* Index within the start byte */
    unsigned char value;
    int  bitsIn2ndByte;


    startByteIndex = (unsigned char)(startBit >> 3);
    startBitIndex  = (unsigned char)(startBit & 0x07);

    /* First get the bits out of the startByteIndex byte */
    value = data[ startByteIndex ] & _bitValueMask[ startBitIndex ];

    bitsIn2ndByte = numBits + startBitIndex - 8;

    if( bitsIn2ndByte > 0 )
      {
        /* We need some bits out of the next byte */

        value = (value << bitsIn2ndByte) |
                (data[startByteIndex+1] >> (8 - bitsIn2ndByte));
      }
    else
      {
        /*  We need to shift down the bits we already have into the least
         *  significant bit positions.  If we have negative bits in the 2nd
         *  byte, this is the same as having that many TOO MANY in the 1st
         *  byte, so we shift those bits out
         */
        value = value >> -bitsIn2ndByte;
      }

    return( value );
  }

char ConvertChar(char value)
{
	if ( value <= 9 )
		 value += 48;
	else if ( value >= 0xa && value <= 0xf )
		value += 87;
    
       return value;
}

void ConvertAscii(unsigned char* pdu, int length, char strPdu[])
{
	if ( pdu == NULL || strPdu == NULL || length <= 0 )
		return;	

	int i = 0;
	char value = 0;
	for ( i = 0; i < length; i++ )
	{
		value = 0;
		value = ((0xf0 & pdu[i]) >> 4);
		strPdu[i*2] = ConvertChar((char)value);

		value = 0;
		value = 0x0f & pdu[i];
		strPdu[i*2+1] = ConvertChar((char)value);
	}	
}

signed char RILNumCharToDTMF(signed char Digit)
{
  signed char DtmfDigit = 10;

  switch (Digit)
  {
    case '0':
      DtmfDigit = 10;
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      DtmfDigit = (signed char) (Digit - (signed char)'0');
      break;

    case '*':
      DtmfDigit = 11;
      break;

    case '#':
      DtmfDigit = 12;
      break;

    case 'P':
      DtmfDigit = 13;
      break;
      
    case 'T':
      DtmfDigit = 14;
      break;
      
    default:
      // Map invalid digit to a DTMF 0.
      DtmfDigit = 10;
  }

  return (DtmfDigit);
}

/*
 * Convert ascii format to byte format
 */
void ConvertByte(unsigned char* pdu, unsigned char* bytePdu, unsigned short len)
{
  int pdulen = strlen(pdu);
  if ( pdulen / 2 != len )
  {
	    return;
  }

  int i;
  for ( i = 0; i < len; i++ )
  {
    bytePdu[i] = ConvertNum(pdu);
    pdu++;
    bytePdu[i] = bytePdu[i] << 4 | ConvertNum(pdu);
    pdu++;
  }

}

char ConvertNum(unsigned char* s)
{
	switch( *s )
	{
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	  	return *s - 48;
	  case 'a':
	  case 'b':
	  case 'c':
	  case 'd':
	  case 'e':
	  case 'f':
	 	return *s - 87;
	  case 'A':
	  case 'B':
	  case 'C':
	  case 'D':
	  case 'E':
	  case 'F':
		return *s - 55;
	  default:
		return -1;
	}
}

void ProcessCdmaIncomingSms(unsigned char* bytePdu, unsigned short length, RIL_CDMA_SMS_Message* p_message)
{
	if ( bytePdu == NULL || p_message == NULL )
	  return;

    unsigned char paramId, paramLen, messageCount;
    unsigned short index = 0;

	unsigned char MsgType = bytePdu[index];
	index++;

	while (index < length)
    {
      /* get subparameter id */
        
      paramId = bytePdu[index];
      index ++;
      paramLen = bytePdu[index];
      index ++;
      switch (paramId)
      {
	      case RIL_CDMA_SMS_TELESERVICE_ID:
	      if( paramLen != 2 )
          {
            //should not run into here
            //this must be serious error
          }
          else
          {
             p_message->uTeleserviceID = ((unsigned short)(bytePdu[index])) << 8;
             p_message->uTeleserviceID |= (unsigned short)(bytePdu[index + 1]);
          }
	      break;
        case RIL_CDMA_SMS_SERVICE_CAT:
          // do nothing right now for RIL_CDMA_SMS_Message struct DO NOT have Service Category field          
	        break;
	      case RIL_CDMA_SMS_DEST_ADDR:
        case RIL_CDMA_SMS_ORIG_ADDR:
          messageCount = RILCdmaSmsExtractAddress(bytePdu+index, &(p_message->sAddress));
		  if ( messageCount != paramLen )
	      {
            //should not run into here
            //this must be serious error
	      }
	      break;
        case RIL_CDMA_SMS_ORIG_SUBADDR:
        case RIL_CDMA_SMS_DEST_SUBADDR:
          messageCount = RILCdmaSmsExtractSubaddress(bytePdu + index, &(p_message->sSubAddress));
	        if (messageCount != paramLen)
	        {
              //should not run into here
              //this must be serious error
	        }
	        break;
        case RIL_CDMA_SMS_BEARER_RPLY_OPT:
          // do nothing right now for RIL_CDMA_SMS_Message struct DO NOT have Bearer reply opt field
          break;
        case RIL_CDMA_SMS_CAUSE_CODE:
          // // do nothing right now for RIL_CDMA_SMS_Message struct DO NOT have cause code field
        case RIL_CDMA_SMS_BEARER_DATA:
          p_message->uBearerDataLen = paramLen;
          memcpy(p_message->aBearerData, bytePdu + index, p_message->uBearerDataLen);
	        break;
        default:
			break;
	        
	    }
	    index += paramLen;
    }
}

unsigned char RILCdmaSmsExtractAddress(unsigned char* BytePdu, RIL_CDMA_SMS_Address* Address)
{
    unsigned char i, size, byte, totalLength;
    unsigned short startBit=0, putBit=0;

    Address->digit_mode = getuint8(BytePdu, startBit, 1); startBit++;
    Address->number_mode = getuint8(BytePdu, startBit, 1); startBit++;

    /*
     *  Calculate bit/byte offsets for the beginning of the
     *  CHARi field.
     */
    if( Address->digit_mode )
    {
        /* numberType is present */
        if(Address->number_mode)
        {
            /*
             * TODO: should handle data number type case
             */
            // See 3GPP2 C.S0015-B 3.4.3.3 (Address Parameters)
            LOGD("digit mode and number mode are both 1, have number type filed, to get it");
            Address->number_type = (RIL_CDMA_SMS_NumberType)getuint8(BytePdu, startBit, 3); startBit += 3;
        }
        else
        {
            Address->number_type = (RIL_CDMA_SMS_NumberType)getuint8(BytePdu, startBit, 3); startBit += 3;
            /* numberPlan is here too */
            Address->number_plan = (RIL_CDMA_SMS_NumberType)getuint8(BytePdu, startBit, 4); startBit += 4;
        }

        size = 8;  /* all address characters will be 8 bits */
        
    }
    else
    {
        size = 4;  /* all address characters will be 4 bits */
    }
    /* Get numFields from message then increment count */
    Address->number_of_digits = getuint8(BytePdu, startBit, 8); 
    startBit += 8;
    for( i = 0; i < Address->number_of_digits; i++ ) 
    {
        byte = getuint8(BytePdu, startBit, size);
        putuint8(Address->digits, putBit, size, byte);
        startBit += size;
        putBit += size;
    }

	if ( Address->digit_mode == 0 )  //transform 4bit number encoding into 8bit number encoding
	{
		/*transform DTMF format to ascii format*/
		char* AsciiNum = (char*)malloc(Address->number_of_digits + 1);
		memset(AsciiNum, 0, Address->number_of_digits + 1);
	
		for (i = 0; (i < (Address->number_of_digits + 1) / 2) ; i++)
		{
			signed char Dtmf = Address->digits[i] >> 4;
   			//mStrDigital += (char)AslSmsDTMFToChar(Dtmf);
			AsciiNum[i*2] = (char)RILNumDTMFToChar(Dtmf);
   		
			Dtmf = Address->digits[i] & 0x0f;
   			if (i*2 + 1 < Address->number_of_digits)
    		{
     			AsciiNum[i*2 + 1]	= (char)RILNumDTMFToChar(Dtmf);
    		}
		}

		memcpy(Address->digits, AsciiNum, Address->number_of_digits);
		Address->digit_mode = 1;//set digit mode to 8bit format

		if ( AsciiNum != NULL )
	  		free(AsciiNum);
	}

    totalLength = NUM_BYTES(startBit);
    return( totalLength );
}

unsigned char RILCdmaSmsExtractSubaddress(unsigned char *BytePdu, RIL_CDMA_SMS_Subaddress* addr )
{
    unsigned char i, totalLength;
    unsigned short startBit = 0;

    addr->subaddressType = getuint8(BytePdu, startBit, 3); startBit += 3;
    addr->odd = getuint8(BytePdu, startBit, 1); startBit++;
    addr->number_of_digits = getuint8(BytePdu, startBit, 8); startBit += 8;

    for( i = 0; i < addr->number_of_digits; i++)
    {
        addr->digits[i] = getuint8(BytePdu, startBit, 8); 
        startBit += 8;
    }
    totalLength = NUM_BYTES(startBit);
    return(totalLength);
}

/*
 * SmsAtStr Format : +CMT: [Dev], [Index], [PduLen], [Pdu]
 *
 * SmsPdu : take SmsPdu String, should be freed by callee
 *
 * length : take length of SmsPdu value
 *
 */
char ParseSmsAtString(char* SmsAtStr, char** SmsPdu, unsigned short* length)
{
  if ( SmsAtStr == NULL || length == NULL )
    return -1; 

  //strtok use non const string to seperate string
  char* NonConstAtStr = (char*)malloc(strlen(SmsAtStr) + 1);
  memset(NonConstAtStr, 0, strlen(SmsAtStr) + 1);
  memcpy(NonConstAtStr, SmsAtStr, strlen(SmsAtStr));

  char Seps[]   = ":,";
  char* Token;
  int Index = 0;

  Token = strtok(NonConstAtStr, Seps);
  while( Token != NULL )
  {
    Token = strtok(NULL, Seps);

    if ( Index == 2 )   //Index == 2 indicate this token take pdu length value
    {
      *length = atoi(Token) * 2;
    }

    if ( Index == 3 )   //Index == 3 indicate this token take pud strings
    {
      unsigned short len = strlen(Token);
      *SmsPdu = (char*)malloc(len + 1);
      memset(*SmsPdu, 0, len + 1);
      memcpy(*SmsPdu, Token, len);
    }

    Index++;
  }

  if ( NonConstAtStr != NULL )
    free(NonConstAtStr);

  return 0;
}


/*
 * Encode RIL_CDMA_SMS_Message into Cdma format pdu
 */
int RILEncodeCdmaSmsPdu(RIL_CDMA_SMS_Message* p_message, char** s_pdu, char** s_number)
{

    if ( p_message == NULL )
        return -1;

    *s_number = (char*)malloc(p_message->sAddress.number_of_digits + 1);
    memset(*s_number, 0, p_message->sAddress.number_of_digits + 1);
    memcpy(*s_number, p_message->sAddress.digits, p_message->sAddress.number_of_digits);
    int i = 0;


    unsigned char pdu[255] = {0};
    unsigned char length = 0;
    char strPdu[255*2 + 1] = {0};  

    //transform RIL_CDMA_SMS_Message number encoding from 8bit encoding to 4bit encoding
    unsigned char adjust = p_message->sAddress.number_of_digits % 2 == 0 ? 0 : 1;
    unsigned char dtmf_len = p_message->sAddress.number_of_digits / 2 + adjust;
    signed char* DTMF = (signed char*)malloc(dtmf_len);

    /*
    * TODO:
    * number of digits should not exceed max length
    */


    for ( i = 0; i < p_message->sAddress.number_of_digits; i++ )
    {
      DTMF[i / 2] = p_message->sAddress.digits[i]; 
      DTMF[i / 2] = DTMF[i / 2] << 4 | p_message->sAddress.digits[i+1];
      i++;
    } 

    memset(p_message->sAddress.digits, 0, p_message->sAddress.number_of_digits);
    memcpy(p_message->sAddress.digits, DTMF, dtmf_len);
    p_message->sAddress.digit_mode = 0; //change to 4bit encoding


    //encoding TeleserviceId & number fileds into byte format pdu
    RILCdmaSmsPdu(p_message, pdu, &length);
    
    //add RIL_CDMA_SMS_BEARER_RPLY_OPT filed
    pdu[length++] = 0x06;
    pdu[length++] = 0x01;
    pdu[length++] = 0x00;

    //add beaer data field which already encoded in framework into byte format pdu
    pdu[length] = 0x08;
    pdu[length + 1] = p_message->uBearerDataLen;
    memcpy(pdu + 2 + length, p_message->aBearerData, p_message->uBearerDataLen);

    //change byte format pdu into ascii format pdu
    ConvertAscii(pdu, length + 2 + p_message->uBearerDataLen, strPdu);

    //free memory malloced before
    if ( DTMF != NULL )
        free(DTMF);

    //already done this job
    *s_pdu = (char*)malloc((length + 2 + p_message->uBearerDataLen)*2 + 1);
    memset(*s_pdu, 0, (length + 2 + p_message->uBearerDataLen)*2 + 1);
    strcpy(*s_pdu, strPdu);  

    return 0;
}

signed char RILNumDTMFToChar(signed char DtmfDigit)
{
  signed char CharDigit = (char)'0';

  switch (DtmfDigit)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      CharDigit = (signed char) (DtmfDigit + (signed char)'0');
      break;

    case 0:
    case 10:
      CharDigit = (signed char)'0';
      break;

    case 11:
      CharDigit = (signed char)'*';
      break;

    case 12:
      CharDigit = (signed char)'#';
      break;
      
    case 13:
      CharDigit = (signed char)'P';
      break;

    case 14:
      CharDigit = (signed char)'T';
      break;

    default:
      // Invalid DTMF digit.
      CharDigit = -1;
  }
  return (CharDigit);
}

int RetrieveIndex(char* s)
{
	int Result = -1;
	if ( s == NULL )
		return Result;

	while ( *s != ',' ) s++;

	Result = atoi(++s);

	return Result;

}

unsigned int RetrievePDUStr(char* s, char begin, char end, char** pdu)
{
	if ( s == NULL )
		return 0;

	int len = RetrieveStr(s, begin, end, pdu);

	return len;
}

unsigned int RetrieveStr(char* s, char begin, char end, char** str)
{
	unsigned int len = 0;
	char* head = NULL;

	if ( s == NULL )
		return len;
  
	while ( *s != begin ) 
		s++; //find head

	head = ++s;
	while ( *s != end && *s != NULL ) 
	{
		s++; 
		len++;
	} //find tail

	if ( *s == NULL || len == 0 ) //if tail is NULL terminated char means no tail tag
		return len = 0;

	*str = (char*)malloc(len + 1); //copy string into str
	memset((*str), 0, len + 1);
	memcpy(*str, head, len);

	return len;
}

