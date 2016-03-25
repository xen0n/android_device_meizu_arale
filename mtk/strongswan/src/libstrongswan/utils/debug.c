/*
 * Copyright (C) 2006 Martin Willi
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
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define KB 1000
#define KEY_DIR "/data/ipsec/"
#define MAX_LOG_FILE_NUMBER 2
#define MAX_LOG_FILE_SIZE 100 * KB
#define TIME_SUFFIX_STR_SIZE 30

#include <stdarg.h>

#include "debug.h"

ENUM(debug_names, DBG_DMN, DBG_MAX,
      "DMN",
      "MGR",
      "IKE",
      "CHD",
      "JOB",
      "CFG",
      "KNL",
      "NET",
      "ASN",
      "ENC",
      "TNC",
      "IMC",
      "IMV",
      "PTS",
      "TLS",
      "APP",
      "ESP",
      "LIB",
      "KEY",
);

ENUM(debug_lower_names, DBG_DMN, DBG_MAX,
      "dmn",
      "mgr",
      "ike",
      "chd",
      "job",
      "cfg",
      "knl",
      "net",
      "asn",
      "enc",
      "tnc",
      "imc",
      "imv",
      "pts",
      "tls",
      "app",
      "esp",
      "lib",
      "key",
);

/* declaration of some help functions */
inline void empty_char_array(char* array);
void get_str_with_current_time_suffix(char* str, const char* prefix);
void join_path(char* joined_path, const char* prefix, const char* sufix);

/**
 * level logged by the default logger
 */
static level_t default_level = 1;

/**
 * stream logged to by the default logger
 */
static FILE *default_stream = NULL;

/**
 * default dbg function which printf all to stderr
 */
void dbg_default(debug_t group, level_t level, char *fmt, ...)
{
   if (!default_stream)
   {
      default_stream = stderr;
   }
   if (level <= default_level)
   {
      va_list args;

      va_start(args, fmt);
      vfprintf(default_stream, fmt, args);
      fprintf(default_stream, "\n");
      va_end(args);
   }
}

/**
 * set the level logged by the default stderr logger
 */
void dbg_default_set_level(level_t level)
{
   default_level = level;
}

/**
 * set the stream logged by dbg_default() to
 */
void dbg_default_set_stream(FILE *stream)
{
   default_stream = stream;
}

/**
 * The registered debug hook.
 */
void (*dbg) (debug_t group, level_t level, char *fmt, ...) = dbg_default;

/**
 *  Get other file point for log with prefix of file name
 */

bool get_log_file(FILE** fp, const char* prefix)
{

   DIR *dir;
   struct dirent *ent;
   *fp = NULL;

   const int FILE_NAME_SIZE = strlen(KEY_DIR) + strlen(prefix) + TIME_SUFFIX_STR_SIZE;

   if ( (dir = opendir(KEY_DIR)) ) {

      char least_recently_used_file_name[FILE_NAME_SIZE];
      char most_recently_used_file_name[FILE_NAME_SIZE];
      char full_path_file_name[FILE_NAME_SIZE];
      int  log_file_count = 0;

      empty_char_array(most_recently_used_file_name);
      sprintf( least_recently_used_file_name, "%s_xxxx_xx_xx-xx_xx", prefix);

      /* get the most recently_used & least recently_used file name */
      while( (ent = readdir (dir)) ) {

         if( strpfx(ent->d_name, prefix)  ) {
            ++log_file_count;
            if( strcmp(ent->d_name, least_recently_used_file_name) < 0 ) {
               strcpy(least_recently_used_file_name, ent->d_name);
            }
            if( strcmp(ent->d_name, most_recently_used_file_name) > 0 ) {
               strcpy(most_recently_used_file_name, ent->d_name);
            }
         }

      }
      closedir(dir);

      /* if log file exist, open log file */
      if( !streq(most_recently_used_file_name, "") ) {
         join_path( full_path_file_name, KEY_DIR, most_recently_used_file_name);
         *fp = fopen( full_path_file_name, "a");

         /* if log file has exceeded the limit size, cloase file */
         if( *fp && ftell(*fp) > (MAX_LOG_FILE_SIZE) ) {
            fclose(*fp);
            *fp = NULL;
         }
      }

      /* create new log file when: (1) no any log file exist,
       *                           (2) log file has exceeded the limit size  */
      if( !*fp ) {

         get_str_with_current_time_suffix( most_recently_used_file_name, prefix);

         join_path( full_path_file_name, KEY_DIR, most_recently_used_file_name);
         DBG1(DBG_IKE, "IKEv2 decryption table cannot be opened: %s", full_path_file_name);
         *fp = fopen( full_path_file_name, "w");

         ++log_file_count;
      }

      /* remove file if the number of log file great than MAX_LOG_FILE_SIZE */
      if( log_file_count > MAX_LOG_FILE_NUMBER ) {
         join_path(full_path_file_name, KEY_DIR, least_recently_used_file_name);
         remove( full_path_file_name );
      }

   }

   return (*fp != NULL);
}

/* some help functions */
void join_path(char* joined_path, const char* prefix, const char* suffix)
{
   strcpy( joined_path, path_dirname(prefix));
   sprintf(joined_path, "%s/%s", prefix,suffix);

}

void get_str_with_current_time_suffix(char* str, const char* prefix)
{
   time_t now = time(0);
   struct tm* timeinfo = localtime(&now);
   char suffix[TIME_SUFFIX_STR_SIZE] = "";

   strftime (suffix, TIME_SUFFIX_STR_SIZE,"_%Y_%m_%d-%H_%M", timeinfo);
   sprintf(str, "%s%s", prefix, suffix);
}


inline void empty_char_array(char* array) {
   memset( array, 0, sizeof(array)/sizeof(char));
}

