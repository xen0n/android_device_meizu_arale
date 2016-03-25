/* C code produced by gperf version 3.0.3 */
/* Command-line: /usr/bin/gperf -m 10 -C -G -D -t  */
/* Computed positions: -k'1-2,6,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif


/*
 * Copyright (C) 2005 Andreas Steffen
 * Hochschule fuer Technik Rapperswil, Switzerland
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

#include <string.h>

#include "keywords.h"

struct kw_entry {
    char *name;
    kw_token_t token;
};

#define TOTAL_KEYWORDS 148
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 17
#define MIN_HASH_VALUE 17
#define MAX_HASH_VALUE 277
/* maximum key range = 261, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned short asso_values[] =
    {
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278,  43,
      112, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278,   6, 278,  33, 278,  63,
       48,   4,  49,  64,  66,   4, 278, 134,   4,  74,
       25,  63,  35, 278,   7,  14,   5, 148,   6, 278,
        9,  14,   5, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
      278, 278, 278, 278, 278, 278
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
      case 4:
      case 3:
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const struct kw_entry wordlist[] =
  {
    {"left",              KW_LEFT},
    {"right",             KW_RIGHT},
    {"lifetime",          KW_KEYLIFE},
    {"leftcert",          KW_LEFTCERT},
    {"type",              KW_TYPE},
    {"leftfirewall",      KW_LEFTFIREWALL},
    {"leftsendcert",      KW_LEFTSENDCERT},
    {"rekey",             KW_REKEY},
    {"rightikeport",      KW_RIGHTIKEPORT},
    {"leftprotoport",     KW_LEFTPROTOPORT},
    {"rightintsubnet",    KW_RIGHTINTSUBNET},
    {"leftsigkey",        KW_LEFTSIGKEY},
    {"leftallowany",      KW_LEFTALLOWANY},
    {"leftgroups",        KW_LEFTGROUPS},
    {"leftcertpolicy",    KW_LEFTCERTPOLICY},
    {"rightsubnet",       KW_RIGHTSUBNET},
    {"rightsendcert",     KW_RIGHTSENDCERT},
    {"lifebytes",         KW_LIFEBYTES},
    {"rightrsasigkey",    KW_RIGHTSIGKEY},
    {"leftrsasigkey",     KW_LEFTSIGKEY},
    {"rightsigkey",       KW_RIGHTSIGKEY},
    {"leftintsubnet",     KW_LEFTINTSUBNET},
    {"rightidr_apn",      KW_RIGHTIDR_APN},
    {"strictcrlpolicy",   KW_STRICTCRLPOLICY},
    {"leftdns",           KW_LEFTDNS},
    {"esp",               KW_ESP},
    {"inactivity",        KW_INACTIVITY},
    {"leftnexthop",       KW_LEFT_DEPRECATED},
    {"installpolicy",     KW_INSTALLPOLICY},
    {"virtual_private",   KW_SETUP_DEPRECATED},
    {"reqid",             KW_REQID},
    {"rightprotoport",    KW_RIGHTPROTOPORT},
    {"lifepackets",       KW_LIFEPACKETS},
    {"rightsubnetwithin", KW_RIGHTSUBNET},
    {"plutostart",        KW_SETUP_DEPRECATED},
    {"rightid",           KW_RIGHTID},
    {"rightallowany",     KW_RIGHTALLOWANY},
    {"rightsourceip",     KW_RIGHTSOURCEIP},
    {"rekeyfuzz",         KW_REKEYFUZZ},
    {"eap",               KW_CONN_DEPRECATED},
    {"xauth_identity",    KW_XAUTH_IDENTITY},
    {"rightfirewall",     KW_RIGHTFIREWALL},
    {"leftupdown",        KW_LEFTUPDOWN},
    {"leftca",            KW_LEFTCA},
    {"rightdns",          KW_RIGHTDNS},
    {"nat_traversal",     KW_SETUP_DEPRECATED},
    {"rightnexthop",      KW_RIGHT_DEPRECATED},
    {"crluri",            KW_CRLURI},
    {"rightsourceif",     KW_RIGHTSOURCEIF},
    {"rightcert",         KW_RIGHTCERT},
    {"certuribase",       KW_CERTURIBASE},
    {"packetdefault",     KW_SETUP_DEPRECATED},
    {"leftidr_apn",       KW_LEFTIDR_APN},
    {"crlcheckinterval",  KW_SETUP_DEPRECATED},
    {"ldapbase",          KW_CA_DEPRECATED},
    {"fragmentation",     KW_FRAGMENTATION},
    {"lefthostaccess",    KW_LEFTHOSTACCESS},
    {"rightgroups",       KW_RIGHTGROUPS},
    {"pfs",               KW_PFS_DEPRECATED},
    {"interfaces",        KW_SETUP_DEPRECATED},
    {"rightcertpolicy",   KW_RIGHTCERTPOLICY},
    {"also",              KW_ALSO},
    {"rightpcscf",        KW_RIGHTPCSCF},
    {"righthostaccess",   KW_RIGHTHOSTACCESS},
    {"mediated_by",       KW_MEDIATED_BY},
    {"dpddelay",          KW_DPDDELAY},
    {"leftid",            KW_LEFTID},
    {"eap_identity",      KW_EAP_IDENTITY},
    {"cacert",            KW_CACERT},
    {"xauth",             KW_XAUTH},
    {"rightca",           KW_RIGHTCA},
    {"mediation",         KW_MEDIATION},
    {"leftsourceip",      KW_LEFTSOURCEIP},
    {"rightauth",         KW_RIGHTAUTH},
    {"tfc",               KW_TFC},
    {"rekeymargin",       KW_REKEYMARGIN},
    {"dpdaction",         KW_DPDACTION},
    {"crluri1",           KW_CRLURI},
    {"aggressive",        KW_AGGRESSIVE},
    {"ldaphost",          KW_CA_DEPRECATED},
    {"leftpcscf",         KW_LEFTPCSCF},
    {"plutostderrlog",    KW_SETUP_DEPRECATED},
    {"leftsourceif",      KW_LEFTSOURCEIF},
    {"leftcert2",         KW_LEFTCERT2},
    {"rightid2",          KW_RIGHTID2},
    {"closeaction",       KW_CLOSEACTION},
    {"force_keepalive",   KW_SETUP_DEPRECATED},
    {"leftgroups2",       KW_LEFTGROUPS2},
    {"me_peerid",         KW_ME_PEERID},
    {"aaa_identity",      KW_AAA_IDENTITY},
    {"forceencaps",       KW_FORCEENCAPS},
    {"mark_in",           KW_MARK_IN},
    {"ocspuri",           KW_OCSPURI},
    {"ike",               KW_IKE},
    {"margintime",        KW_REKEYMARGIN},
    {"nocrsend",          KW_SETUP_DEPRECATED},
    {"reauth",            KW_REAUTH},
    {"mobike",	           KW_MOBIKE},
    {"compress",          KW_COMPRESS},
    {"hidetos",           KW_SETUP_DEPRECATED},
    {"marginbytes",       KW_MARGINBYTES},
    {"leftikeport",       KW_LEFTIKEPORT},
    {"marginpackets",     KW_MARGINPACKETS},
    {"leftca2",           KW_LEFTCA2},
    {"plutodebug",        KW_SETUP_DEPRECATED},
    {"fragicmp",          KW_SETUP_DEPRECATED},
    {"rightintnetmask",   KW_RIGHTINTNETMASK},
    {"rightauth2",        KW_RIGHTAUTH2},
    {"ah",                KW_AH},
    {"charonstart",       KW_SETUP_DEPRECATED},
    {"leftsubnet",        KW_LEFTSUBNET},
    {"dpdtimeout",        KW_DPDTIMEOUT},
    {"postpluto",         KW_SETUP_DEPRECATED},
    {"leftid2",           KW_LEFTID2},
    {"leftintnetmask",	   KW_LEFTINTNETMASK},
    {"cachecrls",         KW_CACHECRLS},
    {"mark_out",          KW_MARK_OUT},
    {"ocspuri1",          KW_OCSPURI},
    {"keep_alive",        KW_SETUP_DEPRECATED},
    {"pfsgroup",          KW_PFS_DEPRECATED},
    {"crluri2",           KW_CRLURI2},
    {"rightca2",          KW_RIGHTCA2},
    {"rightupdown",       KW_RIGHTUPDOWN},
    {"rightcert2",        KW_RIGHTCERT2},
    {"leftsubnetwithin",  KW_LEFTSUBNET},
    {"keylife",           KW_KEYLIFE},
    {"rightgroups2",      KW_RIGHTGROUPS2},
    {"uniqueids",         KW_UNIQUEIDS},
    {"ikelifetime",       KW_IKELIFETIME},
    {"dumpdir",           KW_SETUP_DEPRECATED},
    {"authby",            KW_AUTHBY},
    {"keyexchange",       KW_KEYEXCHANGE},
    {"keyingtries",       KW_KEYINGTRIES},
    {"pkcs11module",      KW_PKCS11_DEPRECATED},
    {"charondebug",       KW_CHARONDEBUG},
    {"leftauth",          KW_LEFTAUTH},
    {"pkcs11keepstate",   KW_PKCS11_DEPRECATED},
    {"overridemtu",       KW_SETUP_DEPRECATED},
    {"pkcs11proxy",       KW_PKCS11_DEPRECATED},
    {"pkcs11initargs",    KW_PKCS11_DEPRECATED},
    {"ikedscp",           KW_IKEDSCP,},
    {"mark",              KW_MARK},
    {"auto",              KW_AUTO},
    {"ocspuri2",          KW_OCSPURI2},
    {"klipsdebug",        KW_SETUP_DEPRECATED},
    {"prepluto",          KW_SETUP_DEPRECATED},
    {"modeconfig",        KW_MODECONFIG},
    {"leftauth2",         KW_LEFTAUTH2}
  };

static const short lookup[] =
  {
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,
     -1,   1,  -1,  -1,   2,   3,  -1,   4,   5,   6,
      7,  -1,   8,   9,  10,  -1,  11,  -1,  12,  13,
     14,  15,  -1,  16,  -1,  17,  18,  -1,  -1,  19,
     20,  21,  22,  23,  24,  -1,  25,  26,  27,  -1,
     28,  -1,  29,  -1,  30,  31,  32,  33,  34,  -1,
     35,  36,  -1,  37,  38,  39,  40,  41,  42,  -1,
     43,  44,  45,  46,  47,  -1,  -1,  48,  49,  50,
     -1,  51,  52,  -1,  53,  -1,  -1,  54,  55,  56,
     57,  58,  59,  60,  61,  62,  63,  -1,  64,  65,
     66,  67,  68,  69,  70,  -1,  -1,  71,  72,  73,
     74,  75,  76,  -1,  77,  78,  -1,  -1,  79,  80,
     -1,  81,  82,  83,  -1,  84,  85,  86,  87,  88,
     89,  90,  -1,  91,  92,  93,  94,  -1,  95,  96,
     -1,  97,  98,  -1,  99,  -1,  -1, 100, 101, 102,
    103, 104, 105,  -1, 106,  -1, 107, 108,  -1,  -1,
    109, 110, 111,  -1, 112, 113,  -1,  -1,  -1,  -1,
     -1, 114, 115, 116, 117, 118,  -1,  -1,  -1,  -1,
    119,  -1,  -1, 120, 121, 122, 123, 124, 125, 126,
    127,  -1, 128,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1, 129, 130, 131,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1, 132, 133, 134,
    135, 136, 137,  -1,  -1,  -1,  -1, 138,  -1,  -1,
    139,  -1,  -1, 140,  -1, 141,  -1,  -1, 142,  -1,
     -1,  -1,  -1, 143,  -1,  -1,  -1,  -1,  -1,  -1,
    144, 145,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1, 146,  -1,  -1, 147
  };

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct kw_entry *
in_word_set (str, len)
     register const char *str;
     register unsigned int len;
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return 0;
}
