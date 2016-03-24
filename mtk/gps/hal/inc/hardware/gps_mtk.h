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
#if 0
/** Maximum number of GNSS SVs for gps_sv_status_callback(). */
#define GNSS_MAX_SVS 256

/** Represents GNSS SV information. */
typedef struct {
    /** set to sizeof(GnssSvInfo) */
    size_t          size;
    /** Pseudo-random number for the SV. */
    int     prn;
    /** Signal to noise ratio. */
    float   snr;
    /** Elevation of SV in degrees. */
    float   elevation;
    /** Azimuth of SV in degrees. */
    float   azimuth;
    /**
     * SVs have ephemeris data.
     */
    bool has_ephemeris;
    /**
     * SVs have almanac data.
     */
    bool has_almanac;
    /**
     * SVs were used for computing the most recent position fix.
     */
    bool used_in_fix;
} GnssSvInfo;

/** Represents GNSS SV status. */
typedef struct {
    /** set to sizeof(GnssSvStatus) */
    size_t          size;

    /** Number of SVs currently visible. */
    int         num_svs;

    /** Contains an array of GNSS SV information. */
    GnssSvInfo   sv_list[GNSS_MAX_SVS];
} GnssSvStatus;
#endif

/**
 * Callback with GNSS SV status information.
 * Can only be called from a thread created by create_thread_cb.
 */
typedef void (* gnss_sv_status_callback)(GnssSvStatus* sv_info);

/** GPS callback structure with Mediatek extension. */
typedef struct _GpsCallbacks_mtk {
    GpsCallbacks base;

    gnss_sv_status_callback gnss_sv_status_cb;
} GpsCallbacks_mtk;

__END_DECLS

#endif /* ANDROID_INCLUDE_HARDWARE_GPS_MTK_H */

