/*
 * Copyright (C) 2015 The CyanogenMod Project
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

#define LOG_TAG "BlisrcDebug"

#include <dlfcn.h>

#include <cutils/log.h>

typedef int (*P_BLISRC_PROCESS)(void *, char *, void *, unsigned int *, void *, unsigned int *);


extern "C" int Blisrc_Process(
		void *p_handle,
		char *p_temp_buf,
		void *p_in_buf,
		unsigned int *p_in_byte_cnt,
		void *p_ou_buf,
		unsigned int *p_ou_byte_cnt
		) {
	ALOGD("Blisrc_Process(%p, %p, %p, %p [=%u], %p, %p [=%u])", p_handle, p_temp_buf, p_in_buf, p_in_byte_cnt, *p_in_byte_cnt, p_ou_buf, p_ou_byte_cnt, *p_ou_byte_cnt);

	void *real_blisrc32_handle = dlopen("libblisrc32.so", RTLD_LAZY | RTLD_LOCAL);
	P_BLISRC_PROCESS real_fn = (P_BLISRC_PROCESS) dlsym(real_blisrc32_handle, "Blisrc_Process");
	int ret = real_fn(p_handle, p_temp_buf, p_in_buf, p_in_byte_cnt, p_ou_buf, p_ou_byte_cnt);
	dlclose(real_blisrc32_handle);

	ALOGD("Blisrc_Process ret=%d (consumed %u, produced %u)", ret, *p_in_byte_cnt, *p_ou_byte_cnt);

	return ret;
}
