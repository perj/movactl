/*
 * Copyright (c) 2008, 2011 Pelle Johansson
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

#if 0
#define NOTIFY(name, code, type) \
	ma_notify_token_t ma_notify_ ## name (int fd, ma_notify_ ## type ## _cb_t cb, void *cbarg); \
	int ma_status_ ## name (int fd, status_ ## type ## _t *value);

#define STATUS(name, code, type) \
	int ma_status_ ## name (int fd, status_ ## type ## _t *value);
#endif

NOTIFY (power, "PWR ", bool)
NOTIFY (audio_att, "ATT ", bool)
NOTIFY (audio_mute, "AMT ", bool)
NOTIFY (video_mute, "VMT ", video_mute)
NOTIFY (volume, "VOL ", int)
NOTIFY (tone_bass, "TOB ", int)
NOTIFY (tone_treble, "TOT ", int)
NOTIFY (video_source, "SRCV", source)
NOTIFY (audio_source, "SRCA", source)
NOTIFY (multi_channel_input, "71C ", bool)
NOTIFY (hdmi_audio_through, "HAM ", bool)
NOTIFY (source_input_state, "SIS ", source_state)
NOTIFY (sleep, "SLP ", int)
NOTIFY (menu, "MNU ", bool)
STATUS (dc_trigger_1, "DCT1", bool)
NOTIFY (front_key_lock, "FKL ", bool)
NOTIFY (simple_setup, "SSU ", bool)
NOTIFY (digital_signal_format, "SIG ", digital_signal_format)
NOTIFY (sampling_frequency, "SFQ ", sampling_frequency)
NOTIFY (channel_status_lfe, "CHSB", bool)
NOTIFY (channel_status_surr_l, "CHSl", bool)
NOTIFY (channel_status_surr_r, "CHSr", bool)
NOTIFY (channel_status_subwoofer, "CHSS", bool)
NOTIFY (channel_status_front_l, "CHSL", bool)
NOTIFY (channel_status_front_r, "CHSR", bool)
NOTIFY (channel_status_center, "CHSC", bool)
NOTIFY (surround_mode, "SUR ", surround_mode)
NOTIFY (test_tone_enabled, "TTOO", bool)
NOTIFY (test_tone_auto, "TTOA", bool)
NOTIFY (test_tone_channel, "TTOC", int)
NOTIFY (night_mode, "NGT ", bool)
NOTIFY (dolby_headphone_mode, "DHM ", dolby_headphone_mode)
NOTIFY (lip_sync, "LIP ", int)
NOTIFY (tuner_band, "TFQB", tuner_band)
NOTIFY (tuner_frequency, "TFQF", int)
NOTIFY (tuner_preset, "TPR ", int)
NOTIFY (tuner_preset_info, "TPI ", bool)
NOTIFY (tuner_mode, "TMD ", tuner_mode)
NOTIFY (xm_in_search, "CATS", bool)
NOTIFY (xm_category, "CATN", int)
NOTIFY (xm_channel_name, "CHN ", string)
NOTIFY (xm_artist_name, "ARN ", string)
NOTIFY (xm_song_title, "SON ", string)
NOTIFY (xm_category_name, "CTN ", string)
NOTIFY (multiroom_power, "MPW ", bool)
NOTIFY (multiroom_audio_mute, "MAM ", bool)
NOTIFY (multiroom_volume, "MVL ", int)
NOTIFY (multiroom_volume_fixed, "MVS ", bool)
NOTIFY (multiroom_video_source, "MSCV", source)
NOTIFY (multiroom_audio_source, "MSCA", source)
NOTIFY (multiroom_sleep, "MSL ", int)
NOTIFY (multiroom_speaker, "MSP ", bool)
NOTIFY (multiroom_speaker_volume, "MSV ", int)
NOTIFY (multiroom_speaker_volume_fixed, "MSS ", bool)
NOTIFY (multiroom_speaker_audio_mute, "MSM ", bool)
NOTIFY (multiroom_tuner_band, "MTFB", tuner_band)
NOTIFY (multiroom_tuner_frequency, "MTFF", int)
NOTIFY (multiroom_tuner_preset, "MTP ", int)
NOTIFY (multiroom_tuner_mode, "MTM ", tuner_mode)
