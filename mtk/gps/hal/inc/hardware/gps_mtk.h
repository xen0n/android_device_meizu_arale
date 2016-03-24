/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef ANDROID_INCLUDE_HARDWARE_GPS_MTK_H
#define ANDROID_INCLUDE_HARDWARE_GPS_MTK_H

#include <hardware/gps.h>

__BEGIN_DECLS

// xen0n: the required bits are already included, don't re-define them.

/** GPS callback structure with Mediatek extension. */
typedef struct _GpsCallbacks_mtk {
    GpsCallbacks base;

//    gnss_sv_status_callback gnss_sv_status_cb;
} GpsCallbacks_mtk;

__END_DECLS

#endif /* ANDROID_INCLUDE_HARDWARE_GPS_MTK_H */

