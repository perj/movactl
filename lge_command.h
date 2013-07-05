/*
 * Copyright (c) 2008 Pelle Johansson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
THROTTLED_COMMAND (power_off, "PWR1", "ka", "00", 2, 0)
THROTTLED_COMMAND (power_on, "PWR2", "ka", "01", 12, 0)

SIMPLE_COMMAND (aspect_ratio_normal, "ARTN", "kc", "01")
SIMPLE_COMMAND (aspect_ratio_wide, "ARTW", "kc", "02")
SIMPLE_COMMAND (aspect_ratio_zoom, "ARTZ", "kc", "04")
SIMPLE_COMMAND (aspect_ratio_original, "ARTO", "kc", "06")
SIMPLE_COMMAND (aspect_ratio_fourteen_nine, "ARFN", "kc", "07")
SIMPLE_COMMAND (aspect_ratio_just_scan, "ARTS", "kc", "09")
SIMPLE_COMMAND (aspect_ratio_full_wide, "ARTF", "kc", "0B")
SIMPLE_COMMAND (aspect_ratio_cinema1, "ARC1", "kc", "10")
SIMPLE_COMMAND (aspect_ratio_cinema2, "ARC2", "kc", "11")
SIMPLE_COMMAND (aspect_ratio_cinema3, "ARC3", "kc", "12")
SIMPLE_COMMAND (aspect_ratio_cinema4, "ARC4", "kc", "13")
SIMPLE_COMMAND (aspect_ratio_cinema5, "ARC5", "kc", "14")
SIMPLE_COMMAND (aspect_ratio_cinema6, "ARC6", "kc", "15")
SIMPLE_COMMAND (aspect_ratio_cinema7, "ARC7", "kc", "16")
SIMPLE_COMMAND (aspect_ratio_cinema8, "ARC8", "kc", "17")
SIMPLE_COMMAND (aspect_ratio_cinema9, "ARC9", "kc", "18")
SIMPLE_COMMAND (aspect_ratio_cinema10, "ARCA", "kc", "19")
SIMPLE_COMMAND (aspect_ratio_cinema11, "ARCB", "kc", "1A")
SIMPLE_COMMAND (aspect_ratio_cinema12, "ARCC", "kc", "1B")
SIMPLE_COMMAND (aspect_ratio_cinema13, "ARCD", "kc", "1C")
SIMPLE_COMMAND (aspect_ratio_cinema14, "ARCE", "kc", "1D")
SIMPLE_COMMAND (aspect_ratio_cinema15, "ARCF", "kc", "1E")
SIMPLE_COMMAND (aspect_ratio_cinema16, "ARCG", "kc", "1F")

SIMPLE_COMMAND (video_mute_off, "VMT1", "kd", "00")
SIMPLE_COMMAND (video_mute_on, "VMT2", "kd", "01")
SIMPLE_COMMAND (video_mute_picture, "VMT3", "kd", "10")

SIMPLE_COMMAND (audio_mute_off, "AMT1", "ke", "00")
SIMPLE_COMMAND (audio_mute_on, "AMT2", "ke", "01")

UINT_COMMAND (volume_value, "VOL0", "kf")

UINT_COMMAND (contrast_value, "CTR0", "kg")

UINT_COMMAND (brightness_value, "BRT0", "kh") 

UINT_COMMAND (colour_value, "CLR0", "ki")

UINT_COMMAND (tint_value, "TNT0", "kj")

UINT_COMMAND (sharpness_value, "SHP0", "kk")

SIMPLE_COMMAND (osd_off, "OSD1", "kl", "00")
SIMPLE_COMMAND (osd_on, "OSD2", "kl", "01") 

SIMPLE_COMMAND (remote_control_lock_off, "RMT1", "km", "00")
SIMPLE_COMMAND (remote_control_lock_on, "RMT2", "km", "01")

UINT_COMMAND (treble_value, "TOT0", "kr")
UINT_COMMAND (bass_value, "TOB0", "ks")

UINT_COMMAND (balance_value, "BAL0", "kt")

SIMPLE_COMMAND (colour_temperature_medium, "CLT1", "ku", "00")
SIMPLE_COMMAND (colour_temperature_cool, "CLT2", "ku", "01")
SIMPLE_COMMAND (colour_temperature_warm, "CLT3", "ku", "02")

SIMPLE_COMMAND (energy_saving_off, "ESV1", "jq", "00")
SIMPLE_COMMAND (energy_saving_minimum, "ESV2", "jq", "01")
SIMPLE_COMMAND (energy_saving_medium, "ESV3", "jq", "02")
SIMPLE_COMMAND (energy_saving_maximum, "ESV4", "jq", "03")
SIMPLE_COMMAND (energy_saving_auto, "ESV5", "jq", "04")
SIMPLE_COMMAND (energy_saving_screen_off, "ESV6", "jq", "05")

SIMPLE_COMMAND (auto_configure, "ACFG", "ju", "01")

UINT2_SUFF_COMMAND (tune_analogue, "TAP0", "ma", "00")
UINT2_SUFF_COMMAND (tune_dtv, "TAP1", "ma", "10")
UINT2_SUFF_COMMAND (tune_radio, "TAP2", "ma", "20")

SIMPLE_COMMAND (programme_skip, "PSK1", "mb", "00")
SIMPLE_COMMAND (programme_add, "PSK2", "mb", "01")

#define KEYCODE(x, y) SIMPLE_COMMAND (key ## _ ## x, "RK" y, "mc", y)
#include "lge_keys.h"
#undef KEYCODE

UINT_COMMAND (back_light, "BLT0", "mg")

SIMPLE_COMMAND (source_select_dtv, "SRDT", "xb", "00")
SIMPLE_COMMAND (source_select_analogue, "SRAT", "xb", "10")
SIMPLE_COMMAND (source_select_av1, "SRA1", "xb", "20")
SIMPLE_COMMAND (source_select_av2, "SRA2", "xb", "21")
SIMPLE_COMMAND (source_select_av3, "SRA3", "xb", "22")
SIMPLE_COMMAND (source_select_av4, "SRA4", "xb", "23")
SIMPLE_COMMAND (source_select_component, "SRMC", "xb", "40")
SIMPLE_COMMAND (source_select_rgb, "SRGB", "xb", "60")
SIMPLE_COMMAND (source_select_hdmi1, "SRH1", "xb", "A0")
SIMPLE_COMMAND (source_select_hdmi2, "SRH2", "xb", "A1")
SIMPLE_COMMAND (source_select_hdmi3, "SRH3", "xb", "A2")
SIMPLE_COMMAND (source_select_hdmi4, "SRH4", "xb", "A3")
