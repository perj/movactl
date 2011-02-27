/*
 * Copyright (c) 2011 Pelle Johansson
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

NOTIFY (power, "PWR ", "ka", bool)
NOTIFY (aspect_ratio, "ART ", "kc", aspect_ratio)
NOTIFY (video_mute, "VMT ", "kd", video_mute)
NOTIFY (audio_mute, "AMT ", "ke", bool)
NOTIFY (volume, "VOL ", "kf", int)

NOTIFY (contrast, "CTR ", "kg", int)
NOTIFY (brightness, "BRT ", "kh", int)
NOTIFY (colour, "CLR ", "ki", int)
NOTIFY (tint, "TNT ", "kj", int)
NOTIFY (sharpness, "SHP ", "kk", int)

NOTIFY (osd, "OSD ", "kl", bool)
NOTIFY (remote_control_lock, "RMT ", "km", bool)

NOTIFY (treble, "TOT ", "kr", int)
NOTIFY (bass, "TOB ", "ks", int)
NOTIFY (balance, "BAL ", "kt", int)

NOTIFY (colour_temperature, "CLT ", "ku", colour_temperature)

NOTIFY (energy_saving, "ESV ", "jq", energy_saving)

NOTIFY (tv_band, "TAPb", "ma", tv_band)
NOTIFY (tv_channel, "TAPc", "ma", int)

NOTIFY (programme_add, "PSK ", "mb", bool)

NOTIFY (back_light, "BLT ", "mg", int)

NOTIFY (source, "SRC ", "xb", source)
