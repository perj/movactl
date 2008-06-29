
#include "status.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
	int len = strlen (arg);

	while (len > 0 && arg[len - 1] == ' ')
		len--;

	if (len >= destlen)
		len = destlen - 1;
	memcpy(dest, arg, len);
	dest[len] = '\0';
}

#define UPDATE_FUNC_BOOL(field) \
	static void \
	update_ ## field (struct ma_status *status, const struct ma_info *info, const char *arg) { \
		status->field = parse_ma_bool (arg); \
	}

#define UPDATE_FUNC_INT(field) \
	static void \
	update_ ## field (struct ma_status *status, const struct ma_info *info, const char *arg) { \
		status->field = atoi (arg); \
	}

#define UPDATE_FUNC_DIRECT(field) \
	static void \
	update_ ## field (struct ma_status *status, const struct ma_info *info, const char *arg) { \
		status->field = *arg; \
	}

#define UPDATE_FUNC_STRING(field) \
	static void \
	update_ ## field (struct ma_status *status, const struct ma_info *info, const char *arg) { \
		parse_ma_string(status->field, sizeof(status->field), arg); \
	}


UPDATE_FUNC_BOOL(power);
UPDATE_FUNC_BOOL(audio_att);
UPDATE_FUNC_BOOL(audio_mute);
UPDATE_FUNC_BOOL(video_mute);

static void
update_volume (struct ma_status *status, const struct ma_info *info, const char *arg) {
	if (strcmp (arg, "-FF") == 0)
		status->volume = MAVOL_MIN;
	else
		status->volume = atoi (arg);
}

UPDATE_FUNC_INT(bass);
UPDATE_FUNC_INT(treble);

static void
update_source_select (struct ma_status *status, const struct ma_info *info, const char *arg) {
	status->video_source = arg[0];
	status->audio_source = arg[1];
}

UPDATE_FUNC_BOOL(multi_channel_input);
UPDATE_FUNC_BOOL(hdmi_audio_through);

UPDATE_FUNC_DIRECT(source_state);

UPDATE_FUNC_INT(sleep);

UPDATE_FUNC_BOOL(menu);
UPDATE_FUNC_BOOL(dc_trigger);
UPDATE_FUNC_BOOL(front_key_lock);
UPDATE_FUNC_BOOL(simple_setup);

UPDATE_FUNC_DIRECT(surround_mode);
UPDATE_FUNC_DIRECT(dolby_headphone_mode);

static void
update_test_tone (struct ma_status *status, const struct ma_info *info, const char *arg) {
	status->test_tone_enabled = parse_ma_bool(arg);
	if (status->test_tone_enabled == mabool_on) {
		status->test_tone_auto = (arg[1] == '0' ? mabool_on : mabool_off);
		status->test_tone_channel = arg[2];
	}
}

UPDATE_FUNC_BOOL(night_mode);

UPDATE_FUNC_DIRECT(digital_signal_format);
UPDATE_FUNC_DIRECT(sampling_frequency);

static void
update_channel_status (struct ma_status *status, const struct ma_info *info, const char *arg) {
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

	status->channel_status_lfe = (x & 0x4 ? mabool_on : mabool_off);
	status->channel_status_surr_l = (x & 0x2 ? mabool_on : mabool_off);
	status->channel_status_surr_r = (x & 0x1 ? mabool_on : mabool_off);
	status->channel_status_subwoofer = (y & 0x8 ? mabool_on : mabool_off);
	status->channel_status_front_l = (y & 0x4 ? mabool_on : mabool_off);
	status->channel_status_front_r = (y & 0x2 ? mabool_on : mabool_off);
	status->channel_status_center = (y & 0x1 ? mabool_on : mabool_off);
}

UPDATE_FUNC_INT(lip_sync);

static void
update_tuner_frequency (struct ma_status *status, const struct ma_info *info, const char *arg) {
	status->tuner_frequency = atoi(arg);
	if (status->tuner_frequency < 256)
		status->tuner_band = matuner_xm;
	else if (status->tuner_frequency < 2000)
		status->tuner_band = matuner_am;
	else
		status->tuner_band = matuner_fm;
}

UPDATE_FUNC_INT(tuner_preset);
UPDATE_FUNC_BOOL(tuner_preset_info);
UPDATE_FUNC_DIRECT(tuner_mode);

static void
update_xm_category_search (struct ma_status *status, const struct ma_info *info, const char *arg) {
	status->xm_in_search = parse_ma_bool(arg);
	status->xm_category = atoi(arg + 1);
}

UPDATE_FUNC_STRING(xm_category_name);
UPDATE_FUNC_STRING(xm_channel_name);
UPDATE_FUNC_STRING(xm_artist_name);
UPDATE_FUNC_STRING(xm_song_title);

UPDATE_FUNC_BOOL(multiroom_power);
UPDATE_FUNC_BOOL(multiroom_audio_mute);

static void
update_multiroom_volume (struct ma_status *status, const struct ma_info *info, const char *arg) {
	if (strcmp (arg, "-FF") == 0)
		status->multiroom_volume = MAVOL_MIN;
	else
		status->multiroom_volume = atoi (arg);
}

UPDATE_FUNC_BOOL(multiroom_volume_fixed);

static void
update_multiroom_source_select (struct ma_status *status, const struct ma_info *info, const char *arg) {
	status->multiroom_video_source = arg[0];
	status->multiroom_audio_source = arg[1];
}

UPDATE_FUNC_INT(multiroom_sleep);
UPDATE_FUNC_BOOL(multiroom_speaker);

static void
update_multiroom_speaker_volume (struct ma_status *status, const struct ma_info *info, const char *arg) {
	if (strcmp (arg, "-FF") == 0)
		status->multiroom_speaker_volume = MAVOL_MIN;
	else
		status->multiroom_speaker_volume = atoi (arg);
}

UPDATE_FUNC_BOOL(multiroom_speaker_volume_fixed);
UPDATE_FUNC_BOOL(multiroom_speaker_audio_mute);

static void
update_multiroom_tuner_frequency (struct ma_status *status, const struct ma_info *info, const char *arg) {
	status->multiroom_tuner_frequency = atoi(arg);
	if (status->multiroom_tuner_frequency < 256)
		status->multiroom_tuner_band = matuner_xm;
	else if (status->multiroom_tuner_frequency < 2000)
		status->multiroom_tuner_band = matuner_am;
	else
		status->multiroom_tuner_band = matuner_fm;
}

UPDATE_FUNC_INT(multiroom_tuner_preset);
UPDATE_FUNC_DIRECT(multiroom_tuner_mode);

static void
update_auto_status_feedback (struct ma_status *status, const struct ma_info *info, const char *arg) {
	int x = *arg;

	if (x >= 'A')
		x -= 'A';
	else
		x -= '0';

	status->auto_status_feedback_layer[3] = (x & 0x8 ? mabool_on : mabool_off);
	status->auto_status_feedback_layer[2] = (x & 0x4 ? mabool_on : mabool_off);
	status->auto_status_feedback_layer[1] = (x & 0x2 ? mabool_on : mabool_off);
	status->auto_status_feedback_layer[0] = (x & 0x1 ? mabool_on : mabool_off);
}

struct ma_info
{
	const char *name;
	const char *code;
	int layer;
	int flags;
	uint64_t know_mask;
	void (*update_func)(struct ma_status *st, const struct ma_info *info, const char *arg);
};

#define ST_CMD_ONLY 1
#define ST_ACK_ONLY 2
#define ST_NO_AUTO 4

const struct ma_info infos[] = {
	{"POWER", "PWR", 1, 0, ST_KNOW_POWER, update_power},
	{"AUDIO ATT", "ATT", 3, 0, ST_KNOW_AUDIO_ATT, update_audio_att},
	{"AUDIO MUTE", "AMT", 1, 0, ST_KNOW_AUDIO_MUTE, update_audio_mute},
	{"VIDEO MUTE", "VMT", 1, 0, ST_KNOW_VIDEO_MUTE, update_video_mute},
	{"VOLUME", "VOL", 1, 0, ST_KNOW_VOLUME, update_volume},
	{"TONE BASS", "TOB", 1, 0, ST_KNOW_BASS, update_bass},
	{"TONE TREBLE", "TOT", 1, 0, ST_KNOW_TREBLE, update_treble},
	{"SOURCE Select", "SRC", 1, 0, ST_KNOW_VIDEO_SOURCE | ST_KNOW_AUDIO_SOURCE, update_source_select},
	{"Multi Channel (7.1 Channel Input)", "71C", 1, 0, ST_KNOW_MULTI_CHANNEL_INPUT, update_multi_channel_input},
	{"MDMI AUDIO MODE", "HAM", 1, 0, ST_KNOW_HDMI_AUDIO_THROUGH, update_hdmi_audio_through},
	{"Source Input State", "IST", 1, 0, ST_KNOW_SOURCE_STATE, update_source_state},
	{"SLEEP", "SLP", 2, 0, ST_KNOW_SLEEP, update_sleep},
	{"MENU", "MNU", 4, 0, ST_KNOW_MENU, update_menu},
	{"CURSOR", "CUR", 0, ST_CMD_ONLY | ST_ACK_ONLY, 0, NULL},
	{"DC TRG.", "DCT", 0, ST_NO_AUTO, ST_KNOW_DC_TRIGGER, update_dc_trigger},
	{"FRONT KEY LOCK", "FKL", 1, 0, ST_KNOW_FRONT_KEY_LOCK, update_front_key_lock},
	{"Simple Setup", "SSU", 4, 0, ST_KNOW_SIMPLE_SETUP, update_simple_setup},
	{"Surr. Mode", "SUR", 2, 0, ST_KNOW_SURROUND_MODE, update_surround_mode},
	{"Dolby Headphone Mode", "DHM", 3, 0, ST_KNOW_DOLBY_HEADPHONE_MODE, update_dolby_headphone_mode},
	{"Test Tone", "TTO", 1, 0, ST_KNOW_TEST_TONE, update_test_tone},
	{"Night Mode", "NGT", 3, 0, ST_KNOW_NIGHT_MODE, update_night_mode},
	{"Signal Format", "SIG", 4, 0, ST_KNOW_DIGITAL_SIGNAL_FORMAT, update_digital_signal_format},
	{"Sampling Frequency", "SFQ", 0, ST_NO_AUTO, ST_KNOW_SAMPLING_FREQUENCY, update_sampling_frequency},
	{"Channel Status", "CHS", 0, ST_NO_AUTO, ST_KNOW_CHANNEL_STATUS, update_channel_status},
	{"Lip Sync.", "LIP", 4, 0, ST_KNOW_LIP_SYNC, update_lip_sync},
	{"Tuner Frequency", "TFQ", 3, 0, ST_KNOW_TUNER_FREQUENCY, update_tuner_frequency},
	{"Tuner Preset", "TPR", 2, 0, ST_KNOW_TUNER_PRESET, update_tuner_preset},
	{"Tuner Preset Info.", "TPI", 2, 0, ST_KNOW_TUNER_PRESET_INFO, update_tuner_preset_info},
	{"Tuner Mode", "TMD", 2, 0, ST_KNOW_TUNER_MODE, update_tuner_mode},
	{"Tuner MEMO", "MEM", 0, ST_CMD_ONLY | ST_ACK_ONLY, 0, NULL},
	{"CLEAR", "CLR", 0 , ST_CMD_ONLY | ST_ACK_ONLY, 0, NULL},
	{"XM Display mode", "XDP", 1, 0, 0, NULL}, /* XXX no info on this one */
	{"XM Category Search", "CAT", 1, 0, ST_KNOW_XM_IN_SEARCH | ST_KNOW_XM_CATEGORY, update_xm_category_search},
	{"XM Category Name", "CTN", 1, 0, ST_KNOW_XM_CATEGORY_NAME, update_xm_category_name},
	{"XM Channel Name", "CHN", 4, 0, ST_KNOW_XM_CHANNEL_NAME, update_xm_channel_name},
	{"XM Artist Name", "ARN", 4, 0, ST_KNOW_XM_ARTIST_NAME, update_xm_artist_name},
	{"XM Song Title", "SON", 4, 0, ST_KNOW_XM_SONG_TITLE, update_xm_song_title},
	{"XM Signal Status", "SST", 1, 0, 0, NULL}, /* XXX no info on this one */
	{"Multi Room POWER", "MPW", 1, 0, ST_KNOW_MULTIROOM_POWER, update_multiroom_power},
	{"Multi Room AUDIO MUTE", "MAM", 1, 0, ST_KNOW_MULTIROOM_AUDIO_MUTE, update_multiroom_audio_mute},
	{"Multi Room VOLUME", "MVL", 1, 0, ST_KNOW_MULTIROOM_VOLUME, update_multiroom_volume},
	{"Multi Room Volume Set", "MVS", 2, 0, ST_KNOW_MULTIROOM_VOLUME_FIXED, update_multiroom_volume_fixed},
	{"Multi Room SOURCE Select", "MSC", 1, 0, ST_KNOW_MULTIROOM_VIDEO_SOURCE | ST_KNOW_MULTIROOM_AUDIO_SOURCE, update_multiroom_source_select},
	{"Multi Room SLEEP", "MSL", 2, 0, ST_KNOW_MULTIROOM_SLEEP, update_multiroom_sleep},
	{"Multi Room SPEAKER", "MSP", 2, 0, ST_KNOW_MULTIROOM_SPEAKER, update_multiroom_speaker},
	{"Multi Room Speaker VOLUME", "MSV", 1, 0, ST_KNOW_MULTIROOM_SPEAKER_VOLUME, update_multiroom_speaker_volume},
	{"Multi Room Speaker Volume Set", "MSS", 2, 0, ST_KNOW_MULTIROOM_SPEAKER_VOLUME_FIXED, update_multiroom_speaker_volume_fixed},
	{"Multi Room Speaker A-MUTE", "MSM", 1, 0, ST_KNOW_MULTIROOM_SPEAKER_AUDIO_MUTE, update_multiroom_speaker_audio_mute},
	{"Multi Room Tuner Frequency", "MTF", 3, 0, ST_KNOW_MULTIROOM_TUNER_FREQUENCY, update_multiroom_tuner_frequency},
	{"Multi Room Tuner Preset", "MTP", 2, 0, ST_KNOW_MULTIROOM_TUNER_PRESET, update_multiroom_tuner_preset},
	{"Multi Room Tuner Mode", "MTM", 2, 0, ST_KNOW_MULTIROOM_TUNER_MODE, update_multiroom_tuner_mode},
	{"Auto status feedback", "AST", 0, ST_CMD_ONLY, 0, update_auto_status_feedback}
};
const int num_infos = sizeof (infos) / sizeof (*infos);

void
enable_auto_status_layer(int fd, struct ma_status *status, int layer) {
	int flags = 0;
	char arg[2];
	int i;

	for (i = 0 ; i < 4; i++) {
		if (status->auto_status_feedback_layer[i] || layer == i + 1)
			flags |= 1 << i;
	}
	snprintf (arg, sizeof (arg), "%X", flags);
	send_command(fd, "AST", arg);
}

void
update_status (int fd, struct ma_status *status, const char *line) {
	const char *cp = strchr(line, ':');
	int i;

	if (!cp)
		return;

	if (line[0] == '@')
		line++;

	for (i = 0; i < num_infos; i++) {
		if (strncmp(line, infos[i].code, cp - line) == 0) {
			infos[i].update_func (status, &infos[i], cp + 1);
			if (infos[i].layer > 0 && status->auto_status_feedback_layer[infos[i].layer - 1] == mabool_off) {
				enable_auto_status_layer(fd, status, infos[i].layer);
			}
			if (infos[i].layer > 0)
				status->known_fields |= infos[i].know_mask;
			return;
		}
	}
}

