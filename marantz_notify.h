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

#ifndef _MORANTZ_NOTIFY_H
#define _MORANTZ_NOTIFY_H

#ifndef NOTIFY
#define NOTIFY(name, cbtype, valtype) \
	ma_notify_token_t ma_notify_ ## name (int fd, ma_notify_ ## cbtype ## _cb_t cb, void *cbarg); \
	int ma_status_ ## name (int fd, valtype *value);

#define STATUS(name, valtype) \
	int ma_status_ ## name (int fd, valtype *value);
#endif

NOTIFY (power, bool, enum ma_bool);
NOTIFY (audio_att, bool, enum ma_bool);
NOTIFY (audio_mute, bool, enum ma_bool);
NOTIFY (video_mute, bool, enum ma_bool);
NOTIFY (volume, int, int);
NOTIFY (tone_bass, int, int);
NOTIFY (tone_treble, int, int);
NOTIFY (video_source, source, enum ma_source);
NOTIFY (audio_source, source, enum ma_source);
NOTIFY (multi_channel_input, bool, enum ma_bool);
NOTIFY (hdmi_audio_through, bool, enum ma_bool);
NOTIFY (source_input_state, source_state, enum ma_source_state);
NOTIFY (sleep, int, int);
NOTIFY (menu, bool, enum ma_bool);
STATUS (dc_trigger, enum ma_bool);
NOTIFY (front_key_lock, bool, enum ma_bool);
NOTIFY (simple_setup, bool, enum ma_bool);
NOTIFY (digital_signal_format, digital_signal_format, enum ma_digital_signal_format);
NOTIFY (sampling_frequency, sampling_frequency, enum ma_sampling_frequency);
NOTIFY (channel_status_lfe, bool, enum ma_bool);
NOTIFY (channel_status_surr_l, bool, enum ma_bool);
NOTIFY (channel_status_surr_r, bool, enum ma_bool);
NOTIFY (channel_status_subwoofer, bool, enum ma_bool);
NOTIFY (channel_status_front_l, bool, enum ma_bool);
NOTIFY (channel_status_front_r, bool, enum ma_bool);
NOTIFY (channel_status_center, bool, enum ma_bool);
NOTIFY (surround_mode, surround_mode, enum ma_surround_mode);
NOTIFY (test_tone_enabled, bool, enum ma_bool);
NOTIFY (test_tone_auto, bool, enum ma_bool);
NOTIFY (test_tone_channel, int, int);
NOTIFY (night_mode, bool, enum ma_bool);
NOTIFY (dolby_headphone_mode, dolby_headphone_mode, enum ma_dolby_headphone_mode);
NOTIFY (lip_sync, int, int);
NOTIFY (tuner_band, tuner_band, enum ma_tuner_band);
NOTIFY (tuner_frequency, int, int);
NOTIFY (tuner_preset, int, int);
NOTIFY (tuner_preset_info, bool, enum ma_bool);
NOTIFY (tuner_mode, tuner_mode, enum ma_tuner_mode);
NOTIFY (xm_in_search, bool, enum ma_bool);
NOTIFY (xm_category, int, int);
NOTIFY (xm_channel_name, string, char);
NOTIFY (xm_artist_name, string, char);
NOTIFY (xm_song_title, string, char);
NOTIFY (xm_category_name, string, char);
NOTIFY (multiroom_power, bool, enum ma_bool);
NOTIFY (multiroom_audio_mute, bool, enum ma_bool);
NOTIFY (multiroom_volume, int, int);
NOTIFY (multiroom_volume_fixed, bool, enum ma_bool);
NOTIFY (multiroom_video_source, source, enum ma_source);
NOTIFY (multiroom_audio_source, source, enum ma_source);
NOTIFY (multiroom_sleep, int, int);
NOTIFY (multiroom_speaker, bool, enum ma_bool);
NOTIFY (multiroom_speaker_volume, int, int);
NOTIFY (multiroom_speaker_volume_fixed, bool, enum ma_bool);
NOTIFY (multiroom_speaker_audio_mute, bool, enum ma_bool);
NOTIFY (multiroom_tuner_band, tuner_band, enum ma_tuner_band);
NOTIFY (multiroom_tuner_frequency, int, int);
NOTIFY (multiroom_tuner_preset, int, int);
NOTIFY (multiroom_tuner_mode, tuner_mode, enum ma_tuner_mode);

#undef NOTIFY
#undef STATUS

#endif /*_MORANTZ_NOTIFY_H*/
