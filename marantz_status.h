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

#ifndef MARANTZ_STATUS_H
#define MARANTZ_STATUS_H

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

#include "serialize.h"

struct backend_device;

struct ma_status
{
#define ST_KNOW_POWER (1LL << 0)
	enum ma_bool power;
#define ST_KNOW_AUDIO_ATT (1LL << 1)
	enum ma_bool audio_att;
#define ST_KNOW_AUDIO_MUTE (1LL << 2)
	enum ma_bool audio_mute;
#define ST_KNOW_VIDEO_MUTE (1LL << 3)
	enum ma_bool video_mute;

#define ST_KNOW_VOLUME (1LL << 4)
	int volume;

#define ST_KNOW_BASS (1LL << 5)
	int bass;
#define ST_KNOW_TREBLE (1LL << 6)
	int treble;

#define ST_KNOW_VIDEO_SOURCE (1LL << 7)
	enum ma_source video_source;
#define ST_KNOW_AUDIO_SOURCE (1LL << 8)
	enum ma_source audio_source;

#define ST_KNOW_MULTI_CHANNEL_INPUT (1LL << 9)
	enum ma_bool multi_channel_input;
#define ST_KNOW_HDMI_AUDIO_THROUGH (1LL << 10)
	enum ma_bool hdmi_audio_through;

#define ST_KNOW_SOURCE_STATE (1LL << 11)
	enum ma_source_state source_state;

#define ST_KNOW_SLEEP (1LL << 12)
	int sleep;
#define ST_KNOW_MENU (1LL << 13)
	enum ma_bool menu;

#define ST_KNOW_DC_TRIGGER (1LL << 14)
	enum ma_bool dc_trigger;
#define ST_KNOW_FRONT_KEY_LOCK (1LL << 15)
	enum ma_bool front_key_lock;

#define ST_KNOW_SIMPLE_SETUP (1LL << 16)
	enum ma_bool simple_setup;

#define ST_KNOW_DIGITAL_SIGNAL_FORMAT (1LL << 17)
	enum ma_digital_signal_format digital_signal_format;
#define ST_KNOW_SAMPLING_FREQUENCY (1LL << 18)
	enum ma_sampling_frequency sampling_frequency;
	
#define ST_KNOW_CHANNEL_STATUS (1LL << 19)
	enum ma_bool channel_status_lfe;
	enum ma_bool channel_status_surr_l;
	enum ma_bool channel_status_surr_r;
	enum ma_bool channel_status_subwoofer;
	enum ma_bool channel_status_front_l;
	enum ma_bool channel_status_front_r;
	enum ma_bool channel_status_center;

#define ST_KNOW_SURROUND_MODE (1LL << 20)
	enum ma_surround_mode surround_mode;

#define ST_KNOW_TEST_TONE (1LL << 21)
	enum ma_bool test_tone_enabled;
	enum ma_bool test_tone_auto;
	int test_tone_channel;

#define ST_KNOW_NIGHT_MODE (1LL << 22)
	enum ma_bool night_mode;

#define ST_KNOW_DOLBY_HEADPHONE_MODE (1LL << 23)
	enum ma_dolby_headphone_mode dolby_headphone_mode;

#define ST_KNOW_LIP_SYNC (1LL << 24)
	int lip_sync;

#define ST_KNOW_TUNER_FREQUENCY (1LL << 25)
	enum ma_tuner_band tuner_band;
	int tuner_frequency;

#define ST_KNOW_TUNER_PRESET (1LL << 26)
	int tuner_preset;
#define ST_KNOW_TUNER_PRESET_INFO (1LL << 27)
	enum ma_bool tuner_preset_info;
#define ST_KNOW_TUNER_MODE (1LL << 28)
	enum ma_tuner_mode tuner_mode;

#define ST_KNOW_XM_IN_SEARCH (1LL << 29)
	enum ma_bool xm_in_search;
#define ST_KNOW_XM_CATEGORY (1LL << 30)
	int xm_category;

#define ST_KNOW_XM_CHANNEL_NAME (1LL << 31)
	char xm_channel_name[11];
#define ST_KNOW_XM_ARTIST_NAME (1LL << 32)
	char xm_artist_name[17];
#define ST_KNOW_XM_SONG_TITLE (1LL << 33)
	char xm_song_title[17];
#define ST_KNOW_XM_CATEGORY_NAME (1LL << 34)
	char xm_category_name[9];

#define ST_KNOW_MULTIROOM_POWER (1LL << 35)
	enum ma_bool multiroom_power;
#define ST_KNOW_MULTIROOM_AUDIO_MUTE (1LL << 36)
	enum ma_bool multiroom_audio_mute;
#define ST_KNOW_MULTIROOM_VOLUME (1LL << 37)
	int multiroom_volume;
#define ST_KNOW_MULTIROOM_VOLUME_FIXED (1LL << 38)
	enum ma_bool multiroom_volume_fixed;

#define ST_KNOW_MULTIROOM_VIDEO_SOURCE (1LL << 39)
	enum ma_source multiroom_video_source;
#define ST_KNOW_MULTIROOM_AUDIO_SOURCE (1LL << 40)
	enum ma_source multiroom_audio_source;

#define ST_KNOW_MULTIROOM_SLEEP (1LL << 41)
	int multiroom_sleep;
#define ST_KNOW_MULTIROOM_SPEAKER (1LL << 42)
	enum ma_bool multiroom_speaker;
#define ST_KNOW_MULTIROOM_SPEAKER_VOLUME (1LL << 43)
	int multiroom_speaker_volume;
#define ST_KNOW_MULTIROOM_SPEAKER_VOLUME_FIXED (1LL << 44)
	enum ma_bool multiroom_speaker_volume_fixed;
#define ST_KNOW_MULTIROOM_SPEAKER_AUDIO_MUTE (1LL << 45)
	enum ma_bool multiroom_speaker_audio_mute;

#define ST_KNOW_MULTIROOM_TUNER_FREQUENCY (1LL << 46)
	enum ma_tuner_band multiroom_tuner_band;
	int multiroom_tuner_frequency;

#define ST_KNOW_MULTIROOM_TUNER_PRESET (1LL << 47)
	int multiroom_tuner_preset;
#define ST_KNOW_MULTIROOM_TUNER_MODE (1LL << 48)
	enum ma_tuner_mode multiroom_tuner_mode;

	enum ma_bool auto_status_feedback_layer[4];

	uint64_t known_fields;

	struct status_notify_info *notify_chain;
};

extern struct status_dispatch marantz_dispatch;

void marantz_send_command(struct backend_device *bdev, const char *cmd, int narg, int32_t *args);

#endif /*MARANTZ_STATUS_H*/
