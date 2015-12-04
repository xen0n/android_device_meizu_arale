/*
 * Copyright (C) 2014 The CyanogenMod Project
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

package org.cyanogenmod.hardware;

import android.os.SystemProperties;

import org.cyanogenmod.hardware.util.FileUtils;

/**
 * Tap (usually double-tap) to wake. This *should always* be supported by
 * the hardware directly. A lot of recent touch controllers have a firmware
 * option for this
 */
public class TapToWake {

    private static String CONTROL_PATH = "/sys/devices/mx_tsp/gesture_hex";

    // XXX: use a backup-able store instead of this!
    private static String STORE_PROP = "persist.cmhw.mz_taptowake";

    /**
     * Whether device supports it
     *
     * @return boolean Supported devices must return always true
     */
    public static boolean isSupported() {
        return true;
    }

    /**
     * This method return the current activation state
     *
     * @return boolean Must be false when feature is not supported or
     * disabled.
     */
    public static boolean isEnabled() {
        final String tmp = SystemProperties.get(STORE_PROP).trim();
        try {
            return Integer.parseInt(tmp) != 0;
        } catch (final NumberFormatException ignored) {
            return false;
        }
    }

    /**
     * This method allows to set activation state
     *
     * @param state The new state
     * @return boolean for on/off, exception if unsupported
     */
    public static boolean setEnabled(final boolean state) {
        SystemProperties.set(STORE_PROP, state ? "1" : "0");

        return FileUtils.writeLine(CONTROL_PATH, state ? "00020001" : "00010000");
    }

}
