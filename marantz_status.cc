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

static status_bool_t
parse_ma_bool (const std::string &arg) {
	if (arg[0] == '2')
		return bool_on;
	return bool_off;
}

static void
parse_ma_string (std::string &dest, const std::string &arg) {
	size_t len = arg.length();

	while (len > 0 && arg[len - 1] == ' ')
		len--;

	dest = arg.substr(0, len);
}

#define UPDATE_FUNC_BOOL(field, code) \
	void \
	ma_status::update_ ## field (const struct ma_info *info, const std::string &arg) { \
		field = parse_ma_bool (arg); \
		notify(code, field); \
	}

#define UPDATE_FUNC_INT(field, code) \
	void \
	ma_status::update_ ## field (const struct ma_info *info, const std::string &arg) { \
		field = std::stoi(arg); \
		notify(code, field); \
	}

#define UPDATE_FUNC_DIRECT(field, code) \
	void \
	ma_status::update_ ## field (const struct ma_info *info, const std::string &arg) { \
		field = static_cast<decltype(field)>(arg[0]); \
		notify(code, field); \
	}

#define UPDATE_FUNC_STRING(field, code) \
	void \
	ma_status::update_ ## field (const struct ma_info *info, const std::string &arg) { \
		parse_ma_string(field, arg); \
		notify(code, field); \
	}


UPDATE_FUNC_BOOL(power, "PWR ");
UPDATE_FUNC_BOOL(audio_att, "ATT ");
UPDATE_FUNC_BOOL(audio_mute, "AMT ");

void
ma_status::update_video_mute(const struct ma_info *info, const std::string &arg) {
	video_mute = parse_ma_bool(arg) == bool_on ? video_mute_on : video_mute_off;
	notify("VMT ", video_mute);
}

void
ma_status::update_volume (const struct ma_info *info, const std::string &arg) {
	if (arg == "-FF")
		volume = MAVOL_MIN;
	else
		volume = std::stoi(arg);
	notify("VOL ", volume);
}

UPDATE_FUNC_INT(tone_bass, "TOB ");
UPDATE_FUNC_INT(tone_treble, "TOT ");

void
ma_status::update_source_select (const struct ma_info *info, const std::string &arg) {
	video_source = (status_source)arg[0];
	notify("SRCV", video_source);
	audio_source = (status_source)arg[1];
	notify("SRCA", audio_source);
}

UPDATE_FUNC_BOOL(multi_channel_input, "71C ");
UPDATE_FUNC_BOOL(hdmi_audio_through, "HAM ");

UPDATE_FUNC_DIRECT(source_input_state, "IST ");

UPDATE_FUNC_INT(sleep, "SLP ");

UPDATE_FUNC_BOOL(menu, "MNU ");

void
ma_status::update_dc_trigger (const struct ma_info *info, const std::string &arg) {
	if (arg[0] == '1') {
		dc_trigger_1 = parse_ma_bool(arg.substr(1));
		notify("DCT1", dc_trigger_1);
	}
}

UPDATE_FUNC_BOOL(front_key_lock, "FKL ");
UPDATE_FUNC_BOOL(simple_setup, "SSU ");

UPDATE_FUNC_DIRECT(surround_mode, "SUR ");
UPDATE_FUNC_DIRECT(dolby_headphone_mode, "DHM ");

void
ma_status::update_test_tone (const struct ma_info *info, const std::string &arg) {
	test_tone_enabled = parse_ma_bool(arg);
	notify("TTOO", test_tone_enabled);
	if (test_tone_enabled == bool_on) {
		test_tone_auto = (arg[1] == '0' ? bool_on : bool_off);
		test_tone_channel = arg[2];
	} else {
		test_tone_auto = bool_off;
		test_tone_channel = 0;
	}
	notify("TTOA", test_tone_auto);
	notify("TTOC", test_tone_channel);
}

UPDATE_FUNC_BOOL(night_mode, "NGT ");

UPDATE_FUNC_DIRECT(digital_signal_format, "SIG ");
UPDATE_FUNC_DIRECT(sampling_frequency, "SFQ ");

void
ma_status::update_channel_status (const struct ma_info *info, const std::string &arg) {
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

	channel_status_lfe = (x & 0x4 ? bool_on : bool_off);
	channel_status_surr_l = (x & 0x2 ? bool_on : bool_off);
	channel_status_surr_r = (x & 0x1 ? bool_on : bool_off);
	channel_status_subwoofer = (y & 0x8 ? bool_on : bool_off);
	channel_status_front_l = (y & 0x4 ? bool_on : bool_off);
	channel_status_front_r = (y & 0x2 ? bool_on : bool_off);
	channel_status_center = (y & 0x1 ? bool_on : bool_off);
}

UPDATE_FUNC_INT(lip_sync, "LIP ");

void
ma_status::update_tuner_frequency (const struct ma_info *info, const std::string &arg) {
	tuner_frequency = std::stoi(arg);
	notify("TFQF", tuner_frequency);
	if (tuner_frequency < 256)
		tuner_band = tuner_band_xm;
	else if (tuner_frequency < 2000)
		tuner_band = tuner_band_am;
	else
		tuner_band = tuner_band_fm;
	notify("TFQB", tuner_band);
}

UPDATE_FUNC_INT(tuner_preset, "TPR ");
UPDATE_FUNC_BOOL(tuner_preset_info, "TPI ");
UPDATE_FUNC_DIRECT(tuner_mode, "TMD ");

void
ma_status::update_xm_category_search (const struct ma_info *info, const std::string &arg) {
	xm_in_search = parse_ma_bool(arg);
	notify("CATS", xm_in_search);
	xm_category = std::stoi(arg.substr(1));
	notify("CATN", xm_category);
}

UPDATE_FUNC_STRING(xm_category_name, "CTN ");
UPDATE_FUNC_STRING(xm_channel_name, "CHN ");
UPDATE_FUNC_STRING(xm_artist_name, "ARN ");
UPDATE_FUNC_STRING(xm_song_title, "SON ");

UPDATE_FUNC_BOOL(multiroom_power, "MPW ");
UPDATE_FUNC_BOOL(multiroom_audio_mute, "MAM ");

void
ma_status::update_multiroom_volume (const struct ma_info *info, const std::string &arg) {
	if (arg == "-FF")
		multiroom_volume = MAVOL_MIN;
	else
		multiroom_volume = std::stoi(arg);
	notify("MVL ", multiroom_volume);
}

UPDATE_FUNC_BOOL(multiroom_volume_fixed, "MVS ");

void
ma_status::update_multiroom_source_select (const struct ma_info *info, const std::string &arg) {
	multiroom_video_source = (status_source)arg[0];
	notify("MSCV", multiroom_video_source);
	multiroom_audio_source = (status_source)arg[1];
	notify("MSCA", multiroom_audio_source);
}

UPDATE_FUNC_INT(multiroom_sleep, "MSL ");
UPDATE_FUNC_BOOL(multiroom_speaker, "MSP ");

void
ma_status::update_multiroom_speaker_volume (const struct ma_info *info, const std::string &arg) {
	if (arg == "-FF")
		multiroom_speaker_volume = MAVOL_MIN;
	else
		multiroom_speaker_volume = std::stoi(arg);
	notify("MSV ", multiroom_speaker_volume);
}

UPDATE_FUNC_BOOL(multiroom_speaker_volume_fixed, "MSS ");
UPDATE_FUNC_BOOL(multiroom_speaker_audio_mute, "MSM ");

void
ma_status::update_multiroom_tuner_frequency (const struct ma_info *info, const std::string &arg) {
	multiroom_tuner_frequency = std::stoi(arg);
	notify("MTFF", multiroom_tuner_frequency);
	if (multiroom_tuner_frequency < 256)
		multiroom_tuner_band = tuner_band_xm;
	else if (multiroom_tuner_frequency < 2000)
		multiroom_tuner_band = tuner_band_am;
	else
		multiroom_tuner_band = tuner_band_fm;
	notify("MTFB", multiroom_tuner_band);
}

UPDATE_FUNC_INT(multiroom_tuner_preset, "MTP ");
UPDATE_FUNC_DIRECT(multiroom_tuner_mode, "MTM ");

void
ma_status::update_auto_status_feedback (const struct ma_info *info, const std::string &arg) {
	int x = arg[0];

	if (x >= 'A')
		x -= 'A';
	else
		x -= '0';

	auto_status_feedback_layer[3] = (x & 0x8 ? bool_on : bool_off);
	auto_status_feedback_layer[2] = (x & 0x4 ? bool_on : bool_off);
	auto_status_feedback_layer[1] = (x & 0x2 ? bool_on : bool_off);
	auto_status_feedback_layer[0] = (x & 0x1 ? bool_on : bool_off);
}

struct ma_info
{
	const char *code;
	int layer;
	int flags;
	uint64_t know_mask;
	void (ma_status::*update_func)(const struct ma_info *info, const std::string &arg);
};

#define ST_CMD_ONLY 1
#define ST_ACK_ONLY 2
#define ST_NO_AUTO 4

const struct ma_info infos[] = {
#define INFO(name, code, level, id) {code, level, 0, ST_KNOW_##id, &ma_status::update_##id},
#define INFO_KNOW(name, code, level, know, id) {code, level, 0, know, &ma_status::update_##id},
#define INFO_ACK_ONLY(name, code) {code, 0, ST_CMD_ONLY | ST_ACK_ONLY, 0, NULL},
#define INFO_NO_AUTO(name, code, id) {code, 0, ST_NO_AUTO, ST_KNOW_##id, &ma_status::update_##id},
#define INFO_KNOW_NO_AUTO(name, code, know, id) {code, 0, ST_NO_AUTO, know, &ma_status::update_##id},
#define NO_INFO(name, code, level) {code, level, 0, 0, NULL},
#define INFO_CMD_ONLY(name, code, id) {code, 0, ST_CMD_ONLY, 0, &ma_status::update_##id},
#include "marantz_info.h"
#undef INFO
#undef INFO_KNOW
#undef INFO_ACK_ONLY
#undef INFO_NO_AUTO
#undef INFO_KNOW_NO_AUTO
#undef NO_INFO
#undef INFO_CMD_ONLY
};
const int num_infos = sizeof (infos) / sizeof (*infos);

struct ma_code
{
	const char *name;
	const char *code;
};

static const struct ma_code ma_codes[] = {
#define NOTIFY(name, code, type) { #name, code },
#define STATUS(name, code, type) { #name, code },
#include "marantz_notify.h"
#undef NOTIFY
#undef STATUS
	{ NULL, NULL }
};

void
ma_status::enable_auto_status_layer(int layer)
{
	int flags = 0;
	int i;

	for (i = 0 ; i < 4; i++) {
		if (auto_status_feedback_layer[i] || layer == i + 1)
			flags |= 1 << i;
	}
	bdev.send("@AST:%X\r", flags);
}

void
ma_status::update_status(const std::string &line, const struct backend_output *inptr)
{
	size_t cpos = line.find(':');
	int i;

	/* Don't need this info */
	while (inptr)
		bdev.remove_output(&inptr);

	if (cpos == std::string::npos)
		return;

	std::string code = line.substr(0, cpos);
	std::string arg = line.substr(cpos + 1);
	if (code[0] == '@')
		code.erase(0);

	for (i = 0; i < num_infos; i++) {
		if (code == infos[i].code) {
			(this->*infos[i].update_func)(&infos[i], arg);
			if (infos[i].layer > 0 && auto_status_feedback_layer[infos[i].layer - 1] == bool_off) {
				enable_auto_status_layer(infos[i].layer);
			}
			if (infos[i].layer > 0)
				known_fields |= infos[i].know_mask;
			return;
		}
	}
}

ma_status::ma_status(backend_ptr &bdev)
	: status(bdev)
{
}

const char *
ma_status::packet_separators()
const
{
	return "\r";
}

int
ma_status::query_status (const std::string &code)
const
{
	const struct ma_code *macode;

	for (macode = ma_codes ; macode->code ; macode++) {
		if (code == macode->code)
			return 0;
	}
	return -1;
}

int
ma_status::query(const std::string &code, std::string &out_buf)
{
	/* TODO */
	return STATUS_UNKNOWN;
}

void
ma_status::status_setup()
{
	memset(auto_status_feedback_layer, 0, sizeof(auto_status_feedback_layer));
	enable_auto_status_layer(1);
	known_fields = 0;
}

int
ma_status::send_status_request(const std::string &code)
{
	bdev.send("@%.3s:?\r", code.c_str());
	return 0;
}

struct status *marantz_creator(backend_ptr &bdev)
{
	return new ma_status(bdev);
}

