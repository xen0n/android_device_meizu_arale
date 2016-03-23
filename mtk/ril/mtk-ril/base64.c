/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "base64.h"
#include <string.h>
#include <utils/Log.h>


static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
unsigned char * base64_encode(const unsigned char *src, size_t len,
                  size_t *out_len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;
    size_t olen;
    int line_len;

    olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
    olen += olen / 72; /* line feeds */
    olen++; /* nul termination */
    if (olen < len)
        return NULL; /* integer overflow */
    out = malloc(olen*sizeof(char));
    memset(out, 0, olen*sizeof(char));
    if (out == NULL)
        return NULL;

    end = src + len;
    in = src;
    pos = out;
    line_len = 0;
    while (end - in >= 3) {
        *pos++ = base64_table[(in[0] >> 2) & 0x3f];
        *pos++ = base64_table[(((in[0] & 0x03) << 4) |
                       (in[1] >> 4)) & 0x3f];
        *pos++ = base64_table[(((in[1] & 0x0f) << 2) |
                       (in[2] >> 6)) & 0x3f];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
        line_len += 4;
        if (line_len >= 72) {
            *pos++ = '\n';
            line_len = 0;
        }
    }

    if (end - in) {
        *pos++ = base64_table[(in[0] >> 2) & 0x3f];
        if (end - in == 1) {
            *pos++ = base64_table[((in[0] & 0x03) << 4) & 0x3f];
            *pos++ = '=';
        } else {
            *pos++ = base64_table[(((in[0] & 0x03) << 4) |
                           (in[1] >> 4)) & 0x3f];
            *pos++ = base64_table[((in[1] & 0x0f) << 2) & 0x3f];
        }
        *pos++ = '=';
        line_len += 4;
    }

    if (line_len)
        *pos++ = '\n';

    *pos = '\0';
    if (out_len)
        *out_len = pos - out;
    RLOGD("[base64] encode olen %d, out %s, %d", olen, out, (*out_len));
    return out;
}


/**
 * base64_decode - Base64 decode
 * @src: Data to be decoded
 * @len: Length of the data to be decoded
 * @out_len: Pointer to output length variable
 * Returns: Allocated buffer of out_len bytes of decoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
unsigned char * base64_decode(const unsigned char *src, size_t len,
                  size_t *out_len)
{
    unsigned char dtable[256], *out, *pos, block[4], tmp;
    size_t i, count, olen;
    int pad = 0;

    memset(dtable, 0x80, 256);
    for (i = 0; i < sizeof(base64_table) - 1; i++)
        dtable[base64_table[i]] = (unsigned char) i;
    dtable['='] = 0;

    count = 0;
    for (i = 0; i < len; i++) {
        if (dtable[src[i]] != 0x80)
            count++;
    }

    if (count == 0 || count % 4)
        return NULL;

    olen = count / 4 * 3;
    olen++; // nul termination
    pos = out = malloc(olen*sizeof(char));
    if (out == NULL)
        return NULL;
    memset(out, 0, olen*sizeof(char));
    count = 0;
    for (i = 0; i < len; i++) {
        tmp = dtable[src[i]];
        if (tmp == 0x80)
            continue;

        if (src[i] == '=')
            pad++;
        block[count] = tmp;
        count++;
        if (count == 4) {
            *pos++ = (block[0] << 2) | (block[1] >> 4);
            *pos++ = (block[1] << 4) | (block[2] >> 2);
            *pos++ = (block[2] << 6) | block[3];
            count = 0;
            if (pad) {
                if (pad == 1)
                    pos--;
                else if (pad == 2)
                    pos -= 2;
                else {
                    /* Invalid padding */
                    free(out);
                    return NULL;
                }
                break;
            }
        }
    }

    *out_len = pos - out;
    RLOGD("[base64] decode olen %d, out %s, %d", olen, out, (*out_len));
    return out;
}

const char HEX_DIGITS_B64[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };


unsigned char* byteArrayToHexStringB64(unsigned char* array,int length)
{
    unsigned char* buf = calloc(1, length*2+1);
    int bufIndex = 0;
    int i = 0;
    for (i = 0 ; i < length; i++)
    {
        unsigned char b = array[i];
        buf[bufIndex++] = HEX_DIGITS_B64[(b >> 4) & 0x0F];
        buf[bufIndex++] = HEX_DIGITS_B64[b & 0x0F];
    }
    buf[bufIndex] = '\0';
    return buf;
}


unsigned char * base64_decode_to_str(const unsigned char *src, size_t len)
{
    const unsigned char *out = NULL;
    size_t out_len = 0;
    unsigned char *out_str = NULL;

    RLOGD("[base64] decode src %s, %d", src, len);
    out = base64_decode(src, len, &out_len);
    if (!out) {
        return NULL;
    }

    out_str = byteArrayToHexStringB64(out, out_len);
    free(out);
    return out_str;
}

static int toByte(char c)
{
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    RLOGE("toByte Error: %c",c);
    return 0;
}

int hexStringToByteArrayB64(unsigned char* hexString, unsigned char ** byte)
{
    int length = strlen((char*)hexString);
    unsigned char* buffer = calloc(1, ((length / 2) + 1));
    int i = 0;

    for (i = 0 ; i < length ; i += 2)
    {
        buffer[i / 2] = (unsigned char)((toByte(hexString[i]) << 4) | toByte(hexString[i+1]));
    }

    *byte = buffer;

    return (length/2);
}

unsigned char * base64_encode_to_str(const unsigned char *src, size_t len)
{
    const unsigned char *out = NULL;
    size_t out_len = 0, encode_len = 0;
    unsigned char *out_str = NULL;

    RLOGD("[base64] encode, src %s, %d", src, len);

    out_len = hexStringToByteArrayB64(src, &out);

    RLOGD("[base64] byte array %s, %d", out, out_len);

    out_str = base64_encode(out, out_len, &encode_len);
    if (!out_str)
        return NULL;

    free(out);
    RLOGD("[base64] encode done %s, %d", out_str, encode_len);
    return out_str;
}

