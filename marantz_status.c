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

#include "status.h"
#include "marantz_status.h"
#include "backend.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <event.h>

#include "line.h"

struct ma_info;

static enum ma_bool
parse_ma_bool (const char *arg) {
	if (*arg == '2')
		return mabool_on;
	return mabool_off;
}

static void
parse_ma_string (char *dest, size_t destlen, const char *arg) {
	size_t len = strlen (arg);

	while (len > 0 && arg[len - 1] == ' ')
		len--;

	if (len >= destlen)
		len = destlen - 1;
	memcpy(dest, arg, len);
	dest[len] = '\0';
}

#define MA(s) ((struct ma_status*)(s)->device_specific)

#define UPDATE_FUNC_BOOL(field) \
	static void \
	update_ ## field (struct status *status, const struct ma_info *info, const char *arg) { \
		MA(status)->field = parse_ma_bool (arg); \
	}

#define UPDATE_FUNC_INT(field) \
	static void \
	update_ ## field (struct status *status, const struct ma_info *info, const char *arg) { \
		MA(status)->field = atoi (arg); \
	}

#define UPDATE_FUNC_DIRECT(field) \
	static void \
	update_ ## field (struct status *status, const struct ma_info *info, const char *arg) { \
		MA(status)->field = *arg; \
	}

#define UPDATE_FUNC_STRING(field) \
	static void \
	update_ ## field (struct status *status, const struct ma_info *info, const char *arg) { \
		parse_ma_string(MA(status)->field, sizeof(MA(status)->field), arg); \
	}


UPDATE_FUNC_BOOL(power);
UPDATE_FUNC_BOOL(audio_att);
UPDATE_FUNC_BOOL(audio_mute);
UPDATE_FUNC_BOOL(video_mute);

static void
update_volume (struct status *status, const struct ma_info *info, const char *arg) {
	if (strcmp (arg, "-FF") == 0)
		MA(status)->volume = MAVOL_MIN;
	else
		MA(status)->volume = atoi (arg);
}

UPDATE_FUNC_INT(tone_bass);
UPDATE_FUNC_INT(tone_treble);

static void
update_source_select (struct status *status, const struct ma_info *info, const char *arg) {
	MA(status)->video_source = arg[0];
	MA(status)->audio_source = arg[1];
}

UPDATE_FUNC_BOOL(multi_channel_input);
UPDATE_FUNC_BOOL(hdmi_audio_through);

UPDATE_FUNC_DIRECT(source_input_state);

UPDATE_FUNC_INT(sleep);

UPDATE_FUNC_BOOL(menu);

static void
update_dc_trigger (struct status *status, const struct ma_info *info, const char *arg) {
	if (arg[0] == '1')
		MA(status)->dc_trigger_1 = parse_ma_bool(arg + 1);
}

UPDATE_FUNC_BOOL(front_key_lock);
UPDATE_FUNC_BOOL(simple_setup);

UPDATE_FUNC_DIRECT(surround_mode);
UPDATE_FUNC_DIRECT(dolby_headphone_mode);

static void
update_test_tone (struct status *status, const struct ma_info *info, const char *arg) {
	MA(status)->test_tone_enabled = parse_ma_bool(arg);
	if (MA(status)->test_tone_enabled == mabool_on) {
		MA(status)->test_tone_auto = (arg[1] == '0' ? mabool_on : mabool_off);
		MA(status)->test_tone_channel = arg[2];
	}
}

UPDATE_FUNC_BOOL(night_mode);

UPDATE_FUNC_DIRECT(digital_signal_format);
UPDATE_FUNC_DIRECT(sampling_frequency);

static void
update_channel_status (struct status *status, const struct ma_info *info, const char *arg) {
	int x = arg[0];
	int y = arg[1];

	if (x >= 'A')
		x -= 'A';
	else
		x -= '0';
	if (y >= 'A')
		y -= 'A';
	else
		y -= '0';

	MA(status)->channel_status_lfe = (x & 0x4 ? mabool_on : mabool_off);
	MA(status)->channel_status_surr_l = (x & 0x2 ? mabool_on : mabool_off);
	MA(status)->channel_status_surr_r = (x & 0x1 ? mabool_on : mabool_off);
	MA(status)->channel_status_subwoofer = (y & 0x8 ? mabool_on : mabool_off);
	MA(status)->channel_status_front_l = (y & 0x4 ? mabool_on : mabool_off);
	MA(status)->channel_status_front_r = (y & 0x2 ? mabool_on : mabool_off);
	MA(status)->channel_status_center = (y & 0x1 ? mabool_on : mabool_off);
}

UPDATE_FUNC_INT(lip_sync);

static void
update_tuner_frequency (struct status *status, const struct ma_info *info, const char *arg) {
	MA(status)->tuner_frequency = atoi(arg);
	if (MA(status)->tuner_frequency < 256)
		MA(status)->tuner_band = matuner_xm;
	else if (MA(status)->tuner_frequency < 2000)
		MA(status)->tuner_band = matuner_am;
	else
		MA(status)->tuner_band = matuner_fm;
}

UPDATE_FUNC_INT(tuner_preset);
UPDATE_FUNC_BOOL(tuner_preset_info);
UPDATE_FUNC_DIRECT(tuner_mode);

static void
update_xm_category_search (struct status *status, const struct ma_info *info, const char *arg) {
	MA(status)->xm_in_search = parse_ma_bool(arg);
	MA(status)->xm_category = atoi(arg + 1);
}

UPDATE_FUNC_STRING(xm_category_name);
UPDATE_FUNC_STRING(xm_channel_name);
UPDATE_FUNC_STRING(xm_artist_name);
UPDATE_FUNC_STRING(xm_song_title);

UPDATE_FUNC_BOOL(multiroom_power);
UPDATE_FUNC_BOOL(multiroom_audio_mute);

static void
update_multiroom_volume (struct status *status, const struct ma_info *info, const char *arg) {
	if (strcmp (arg, "-FF") == 0)
		MA(status)->multiroom_volume = MAVOL_MIN;
	else
		MA(status)->multiroom_volume = atoi (arg);
}

UPDATE_FUNC_BOOL(multiroom_volume_fixed);

static void
update_multiroom_source_select (struct status *status, const struct ma_info *info, const char *arg) {
	MA(status)->multiroom_video_source = arg[0];
	MA(status)->multiroom_audio_source = arg[1];
}

UPDATE_FUNC_INT(multiroom_sleep);
UPDATE_FUNC_BOOL(multiroom_speaker);

static void
update_multiroom_speaker_volume (struct status *status, const struct ma_info *info, const char *arg) {
	if (strcmp (arg, "-FF") == 0)
		MA(status)->multiroom_speaker_volume = MAVOL_MIN;
	else
		MA(status)->multiroom_speaker_volume = atoi (arg);
}

UPDATE_FUNC_BOOL(multiroom_speaker_volume_fixed);
UPDATE_FUNC_BOOL(multiroom_speaker_audio_mute);

static void
update_multiroom_tuner_frequency (struct status *status, const struct ma_info *info, const char *arg) {
	MA(status)->multiroom_tuner_frequency = atoi(arg);
	if (MA(status)->multiroom_tuner_frequency < 256)
		MA(status)->multiroom_tuner_band = matuner_xm;
	else if (MA(status)->multiroom_tuner_frequency < 2000)
		MA(status)->multiroom_tuner_band = matuner_am;
	else
		MA(status)->multiroom_tuner_band = matuner_fm;
}

UPDATE_FUNC_INT(multiroom_tuner_preset);
UPDATE_FUNC_DIRECT(multiroom_tuner_mode);

static void
update_auto_status_feedback (struct status *status, const struct ma_info *info, const char *arg) {
	int x = *arg;

	if (x >= 'A')
		x -= 'A';
	else
		x -= '0';

	MA(status)->auto_status_feedback_layer[3] = (x & 0x8 ? mabool_on : mabool_off);
	MA(status)->auto_status_feedback_layer[2] = (x & 0x4 ? mabool_on : mabool_off);
	MA(status)->auto_status_feedback_layer[1] = (x & 0x2 ? mabool_on : mabool_off);
	MA(status)->auto_status_feedback_layer[0] = (x & 0x1 ? mabool_on : mabool_off);
}

struct ma_info
{
	const char *name;
	const char *code;
	int layer;
	int flags;
	uint64_t know_mask;
	void (*update_func)(struct status *st, const struct ma_info *info, const char *arg);
};

#define ST_CMD_ONLY 1
#define ST_ACK_ONLY 2
#define ST_NO_AUTO 4

const struct ma_info infos[] = {
	{"POWER", "PWR", 1, 0, ST_KNOW_power, update_power},
	{"AUDIO ATT", "ATT", 3, 0, ST_KNOW_audio_att, update_audio_att},
	{"AUDIO MUTE", "AMT", 1, 0, ST_KNOW_audio_mute, update_audio_mute},
	{"VIDEO MUTE", "VMT", 1, 0, ST_KNOW_video_mute, update_video_mute},
	{"VOLUME", "VOL", 1, 0, ST_KNOW_volume, update_volume},
	{"TONE BASS", "TOB", 1, 0, ST_KNOW_tone_bass, update_tone_bass},
	{"TONE TREBLE", "TOT", 1, 0, ST_KNOW_tone_treble, update_tone_treble},
	{"SOURCE Select", "SRC", 1, 0, ST_KNOW_video_source | ST_KNOW_audio_source, update_source_select},
	{"Multi Channel (7.1 Channel Input)", "71C", 1, 0, ST_KNOW_multi_channel_input, update_multi_channel_input},
	{"MDMI AUDIO MODE", "HAM", 1, 0, ST_KNOW_hdmi_audio_through, update_hdmi_audio_through},
	{"Source Input State", "IST", 1, 0, ST_KNOW_source_input_state, update_source_input_state},
	{"SLEEP", "SLP", 2, 0, ST_KNOW_sleep, update_sleep},
	{"MENU", "MNU", 4, 0, ST_KNOW_menu, update_menu},
	{"CURSOR", "CUR", 0, ST_CMD_ONLY | ST_ACK_ONLY, 0, NULL},
	{"DC TRG.", "DCT", 0, ST_NO_AUTO, 0, update_dc_trigger},
	{"FRONT KEY LOCK", "FKL", 1, 0, ST_KNOW_front_key_lock, update_front_key_lock},
	{"Simple Setup", "SSU", 4, 0, ST_KNOW_simple_setup, update_simple_setup},
	{"Surr. Mode", "SUR", 2, 0, ST_KNOW_surround_mode, update_surround_mode},
	{"Dolby Headphone Mode", "DHM", 3, 0, ST_KNOW_dolby_headphone_mode, update_dolby_headphone_mode},
	{"Test Tone", "TTO", 1, 0, ST_KNOW_test_tone_enabled | ST_KNOW_test_tone_auto | ST_KNOW_test_tone_channel, update_test_tone},
	{"Night Mode", "NGT", 3, 0, ST_KNOW_night_mode, update_night_mode},
	{"Signal Format", "SIG", 4, 0, ST_KNOW_digital_signal_format, update_digital_signal_format},
	{"Sampling Frequency", "SFQ", 0, ST_NO_AUTO, ST_KNOW_sampling_frequency, update_sampling_frequency},
	{"Channel Status", "CHS", 0, ST_NO_AUTO, ST_KNOW_channel_status_lfe | ST_KNOW_channel_status_surr_l | ST_KNOW_channel_status_surr_r | ST_KNOW_channel_status_subwoofer | ST_KNOW_channel_status_front_l | ST_KNOW_channel_status_front_r | ST_KNOW_channel_status_center, update_channel_status},
	{"Lip Sync.", "LIP", 4, 0, ST_KNOW_lip_sync, update_lip_sync},
	{"Tuner Frequency", "TFQ", 3, 0, ST_KNOW_tuner_band | ST_KNOW_tuner_frequency, update_tuner_frequency},
	{"Tuner Preset", "TPR", 2, 0, ST_KNOW_tuner_preset, update_tuner_preset},
	{"Tuner Preset Info.", "TPI", 2, 0, ST_KNOW_tuner_preset_info, update_tuner_preset_info},
	{"Tuner Mode", "TMD", 2, 0, ST_KNOW_tuner_mode, update_tuner_mode},
	{"Tuner MEMO", "MEM", 0, ST_CMD_ONLY | ST_ACK_ONLY, 0, NULL},
	{"CLEAR", "CLR", 0 , ST_CMD_ONLY | ST_ACK_ONLY, 0, NULL},
	{"XM Display mode", "XDP", 1, 0, 0, NULL}, /* XXX no info on this one */
	{"XM Category Search", "CAT", 1, 0, ST_KNOW_xm_in_search | ST_KNOW_xm_category, update_xm_category_search},
	{"XM Category Name", "CTN", 1, 0, ST_KNOW_xm_category_name, update_xm_category_name},
	{"XM Channel Name", "CHN", 4, 0, ST_KNOW_xm_channel_name, update_xm_channel_name},
	{"XM Artist Name", "ARN", 4, 0, ST_KNOW_xm_artist_name, update_xm_artist_name},
	{"XM Song Title", "SON", 4, 0, ST_KNOW_xm_song_title, update_xm_song_title},
	{"XM Signal Status", "SST", 1, 0, 0, NULL}, /* XXX no info on this one */
	{"Multi Room POWER", "MPW", 1, 0, ST_KNOW_multiroom_power, update_multiroom_power},
	{"Multi Room AUDIO MUTE", "MAM", 1, 0, ST_KNOW_multiroom_audio_mute, update_multiroom_audio_mute},
	{"Multi Room VOLUME", "MVL", 1, 0, ST_KNOW_multiroom_volume, update_multiroom_volume},
	{"Multi Room Volume Set", "MVS", 2, 0, ST_KNOW_multiroom_volume_fixed, update_multiroom_volume_fixed},
	{"Multi Room SOURCE Select", "MSC", 1, 0, ST_KNOW_multiroom_video_source | ST_KNOW_multiroom_audio_source, update_multiroom_source_select},
	{"Multi Room SLEEP", "MSL", 2, 0, ST_KNOW_multiroom_sleep, update_multiroom_sleep},
	{"Multi Room SPEAKER", "MSP", 2, 0, ST_KNOW_multiroom_speaker, update_multiroom_speaker},
	{"Multi Room Speaker VOLUME", "MSV", 1, 0, ST_KNOW_multiroom_speaker_volume, update_multiroom_speaker_volume},
	{"Multi Room Speaker Volume Set", "MSS", 2, 0, ST_KNOW_multiroom_speaker_volume_fixed, update_multiroom_speaker_volume_fixed},
	{"Multi Room Speaker A-MUTE", "MSM", 1, 0, ST_KNOW_multiroom_speaker_audio_mute, update_multiroom_speaker_audio_mute},
	{"Multi Room Tuner Frequency", "MTF", 3, 0, ST_KNOW_multiroom_tuner_frequency, update_multiroom_tuner_frequency},
	{"Multi Room Tuner Preset", "MTP", 2, 0, ST_KNOW_multiroom_tuner_preset, update_multiroom_tuner_preset},
	{"Multi Room Tuner Mode", "MTM", 2, 0, ST_KNOW_multiroom_tuner_mode, update_multiroom_tuner_mode},
	{"Auto status feedback", "AST", 0, ST_CMD_ONLY, 0, update_auto_status_feedback}
};
const int num_infos = sizeof (infos) / sizeof (*infos);

void
enable_auto_status_layer(struct backend_device *bdev, struct ma_status *status, int layer) {
	int flags = 0;
	int i;

	for (i = 0 ; i < 4; i++) {
		if (status->auto_status_feedback_layer[i] || layer == i + 1)
			flags |= 1 << i;
	}
	backend_send(bdev, "@AST:%X\r", flags);
}

struct status_notify_info
{
	struct status *status;
	char *code;
	void (*cb)(struct status *status, status_notify_token_t token, const char *code, void *cbarg, void *data, size_t len);
	void *cbarg;

	struct status_notify_info *next;
};

int marantz_status_serialize (struct status *status, const char *code, void *buf, size_t *buflen);

void
marantz_update_status (struct backend_device *bdev, struct status *status, const char *line) {
	const char *cp = strchr(line, ':');
	int i;
	struct status_notify_info *notify;
	char buf[256];
	size_t len;

	if (!cp)
		return;

	if (line[0] == '@')
		line++;

	for (i = 0; i < num_infos; i++) {
		if (strncmp(line, infos[i].code, cp - line) == 0) {
			infos[i].update_func (status, &infos[i], cp + 1);
			if (infos[i].layer > 0 && MA(status)->auto_status_feedback_layer[infos[i].layer - 1] == mabool_off) {
				enable_auto_status_layer(bdev, MA(status), infos[i].layer);
			}
			if (infos[i].layer > 0)
				MA(status)->known_fields |= infos[i].know_mask;
			len = 0 ;
			for (notify = status->notify_chain; notify; notify = notify->next) {
				if (strncmp (line, notify->code, cp - line) == 0) {
					if (!len) {
						int res;

						len = sizeof (buf);
						if ((res = marantz_status_serialize (status, notify->code, buf, &len)) != SERIALIZE_OK) {
							warnx("status_serialize failed: %d", res);
							return;
						}
					}
					notify->cb (status, notify, notify->code, notify->cbarg, buf, len);
				}
			}
			return;
		}
	}
}

#define SERIALIZE_FIELD(mask, type, field) \
	do { \
		if ((!mask || (info->know_mask & mask)) && (res = serialize_ ## type (&buf, &len, *buflen, MA(status)->field))) {\
			warnx ("serialize failed: len = %d, *buflen = %d", (int)len, (int)*buflen); \
			return res; \
		} \
	} while (0)
int
marantz_status_serialize (struct status *status, const char *code, void *buf, size_t *buflen) {
	const struct ma_info *info;

	for (info = infos; info < infos + num_infos; info++) {
		if (strcmp(code, info->code) == 0) {
			size_t len;
			int res;

			if ((MA(status)->known_fields & info->know_mask) != info->know_mask)
				return SERIALIZE_UNKNOWN;

			len = strlen (code) + 1;
			if (len > *buflen)
				return SERIALIZE_BUFFER_FULL;
			memcpy(buf, code, len);
			buf = (char*)buf + len;

			SERIALIZE_FIELD (ST_KNOW_power, bool, power);
			SERIALIZE_FIELD (ST_KNOW_audio_att, bool, audio_att);
			SERIALIZE_FIELD (ST_KNOW_audio_mute, bool, audio_mute);
			SERIALIZE_FIELD (ST_KNOW_video_mute, bool, video_mute);
			SERIALIZE_FIELD (ST_KNOW_volume, int, volume);
			SERIALIZE_FIELD (ST_KNOW_tone_bass, int, tone_bass);
			SERIALIZE_FIELD (ST_KNOW_tone_treble, int, tone_treble);
			SERIALIZE_FIELD (ST_KNOW_video_source, source, video_source);
			SERIALIZE_FIELD (ST_KNOW_audio_source, source, audio_source);
			SERIALIZE_FIELD (ST_KNOW_multi_channel_input, bool, multi_channel_input);
			SERIALIZE_FIELD (ST_KNOW_hdmi_audio_through, bool, hdmi_audio_through);
			SERIALIZE_FIELD (ST_KNOW_source_input_state, source_state, source_input_state);
			SERIALIZE_FIELD (ST_KNOW_sleep, int, sleep);
			SERIALIZE_FIELD (ST_KNOW_menu, bool, menu);
			SERIALIZE_FIELD (0, bool, dc_trigger_1);
			SERIALIZE_FIELD (ST_KNOW_front_key_lock, bool, front_key_lock);
			SERIALIZE_FIELD (ST_KNOW_simple_setup, bool, simple_setup);
			SERIALIZE_FIELD (ST_KNOW_digital_signal_format, digital_signal_format, digital_signal_format);
			SERIALIZE_FIELD (ST_KNOW_sampling_frequency, sampling_frequency, sampling_frequency);
			SERIALIZE_FIELD (ST_KNOW_channel_status_lfe, bool, channel_status_lfe);
			SERIALIZE_FIELD (ST_KNOW_channel_status_surr_l, bool, channel_status_surr_l);
			SERIALIZE_FIELD (ST_KNOW_channel_status_surr_r, bool, channel_status_surr_r);
			SERIALIZE_FIELD (ST_KNOW_channel_status_subwoofer, bool, channel_status_subwoofer);
			SERIALIZE_FIELD (ST_KNOW_channel_status_front_l, bool, channel_status_front_l);
			SERIALIZE_FIELD (ST_KNOW_channel_status_front_r, bool, channel_status_front_r);
			SERIALIZE_FIELD (ST_KNOW_channel_status_center, bool, channel_status_center);
			SERIALIZE_FIELD (ST_KNOW_surround_mode, surround_mode, surround_mode);
			SERIALIZE_FIELD (ST_KNOW_test_tone_enabled, bool, test_tone_enabled);
			SERIALIZE_FIELD (ST_KNOW_test_tone_auto, bool, test_tone_auto);
			SERIALIZE_FIELD (ST_KNOW_test_tone_channel, int, test_tone_channel);
			SERIALIZE_FIELD (ST_KNOW_night_mode, bool, night_mode);
			SERIALIZE_FIELD (ST_KNOW_dolby_headphone_mode, dolby_headphone_mode, dolby_headphone_mode);
			SERIALIZE_FIELD (ST_KNOW_lip_sync, int, lip_sync);
			SERIALIZE_FIELD (ST_KNOW_tuner_band, tuner_band, tuner_band);
			SERIALIZE_FIELD (ST_KNOW_tuner_frequency, int, tuner_frequency);
			SERIALIZE_FIELD (ST_KNOW_tuner_preset, int, tuner_preset);
			SERIALIZE_FIELD (ST_KNOW_tuner_preset_info, bool, tuner_preset_info);
			SERIALIZE_FIELD (ST_KNOW_tuner_mode, tuner_mode, tuner_mode);
			SERIALIZE_FIELD (ST_KNOW_xm_in_search, bool, xm_in_search);
			SERIALIZE_FIELD (ST_KNOW_xm_category, int, xm_category);
			SERIALIZE_FIELD (ST_KNOW_xm_channel_name, string, xm_channel_name);
			SERIALIZE_FIELD (ST_KNOW_xm_artist_name, string, xm_artist_name);
			SERIALIZE_FIELD (ST_KNOW_xm_song_title, string, xm_song_title);
			SERIALIZE_FIELD (ST_KNOW_xm_category_name, string, xm_category_name);
			SERIALIZE_FIELD (ST_KNOW_multiroom_power, bool, multiroom_power);
			SERIALIZE_FIELD (ST_KNOW_multiroom_audio_mute, bool, multiroom_audio_mute);
			SERIALIZE_FIELD (ST_KNOW_multiroom_volume, int, multiroom_volume);
			SERIALIZE_FIELD (ST_KNOW_multiroom_volume_fixed, bool, multiroom_volume_fixed);
			SERIALIZE_FIELD (ST_KNOW_multiroom_video_source, source, multiroom_video_source);
			SERIALIZE_FIELD (ST_KNOW_multiroom_audio_source, source, multiroom_audio_source);
			SERIALIZE_FIELD (ST_KNOW_multiroom_sleep, int, multiroom_sleep);
			SERIALIZE_FIELD (ST_KNOW_multiroom_speaker, bool, multiroom_speaker);
			SERIALIZE_FIELD (ST_KNOW_multiroom_speaker_volume, int, multiroom_speaker_volume);
			SERIALIZE_FIELD (ST_KNOW_multiroom_speaker_volume_fixed, bool, multiroom_speaker_volume_fixed);
			SERIALIZE_FIELD (ST_KNOW_multiroom_speaker_audio_mute, bool, multiroom_speaker_audio_mute);
			SERIALIZE_FIELD (ST_KNOW_multiroom_tuner_frequency, tuner_band, multiroom_tuner_band);
			SERIALIZE_FIELD (ST_KNOW_multiroom_tuner_frequency, int, multiroom_tuner_frequency);
			SERIALIZE_FIELD (ST_KNOW_multiroom_tuner_preset, int, multiroom_tuner_preset);
			SERIALIZE_FIELD (ST_KNOW_multiroom_tuner_mode, tuner_mode, multiroom_tuner_mode);
			if (len + 1 > *buflen)
				return SERIALIZE_BUFFER_FULL;
			*(uint8_t*)buf = matype_EOD;
			*buflen = len + 1;

			return SERIALIZE_OK;
		}
	}
	return SERIALIZE_INVALID;
}

status_notify_token_t
marantz_status_notify (struct status *status, const char *code,
		void (*status_notify_cb)(struct status *status, status_notify_token_t token, const char *code, void *cbarg,
				void *data, size_t len),
		void *cbarg) {
	struct status_notify_info *info = malloc (sizeof (*info));

	if (!info)
		return NULL;
	info->code = strdup (code);
	if (!info->code) {
		free (info);
		return NULL;
	}
	info->status = status;
	info->cb = status_notify_cb;
	info->cbarg = cbarg;
	info->next = status->notify_chain;
	status->notify_chain = info;

	return info;
}

void
marantz_setup_status (struct backend_device *bdev, struct status *status) {
	status->device_specific = malloc (sizeof (struct ma_status));
	if (!status->device_specific)
		err (1, "malloc");

	enable_auto_status_layer (bdev, MA(status), 1);
	MA(status)->known_fields = 0;
}

struct status_dispatch marantz_dispatch = {
	marantz_setup_status,
	"\r\n",
	marantz_update_status,
	NULL,
	marantz_status_serialize,
	marantz_send_command,
};
