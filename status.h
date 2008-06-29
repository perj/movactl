#ifndef STATUS_H
#define STATUS_H

#include <limits.h>
#include <stdint.h>

enum ma_bool
{
	mabool_off, mabool_on
};

#define MAVOL_MIN INT_MIN

enum ma_source
{
	masource_off = '0',
	masource_tv,
	masource_dvd,
	masource_vcr1,

	masource_dss = '5',

	masource_aux1 = '9',
	masource_aux2 = 'A',

	masource_cd = 'C',
	masource_cdr,
	masource_tape,
	masource_tuner,
	masource_fm,
	masource_am,
	masource_xm = 'J',

	masource_71C = 'N'
};

#define MASLEEP_OFF 0

enum ma_surround_mode
{
	masurr_auto = '0',
	masurr_stereo,
	masurr_dolby,
	masurr_dolby_pl2x_movie,
	masurr_dolby_pl2_movie,
	masurr_dolby_pl2x_music,
	masurr_dolby_pl2_music,
	masurr_dolby_pl2x_game,
	masurr_dolby_pl2_game,
	masurr_dolby_pl,
	masurr_ex_es = 'A',
	masurr_virtual_61,

	masurr_dts_es = 'E',
	masurr_neo6_cinema,
	masurr_neo6_music,
	masurr_multi_channel_stereo,
	masurr_cs2_cinema,
	masurr_cs2_music,
	masurr_cs2_mono,
	masurr_virtual,
	masurr_dts,

	masurr_ddplus_pl2x_movie,
	masurr_ddplus_pl2x_music,

	masurr_source_direct = 'T',
	masurr_pure_direct = 'U'
};

enum ma_dolby_headphone_mode
{
	maheadphone_bypass = '0',
	maheadphone_dh1,
	maheadphone_dh1_pl2_movie,
	maheadphone_dh1_pl2_music
};

enum ma_tuner_band
{
	matuner_xm, matuner_am, matuner_fm
};

enum ma_source_state
{
	mastate_unKNOW = '0',
	mastate_off,
	mastate_on
};

enum ma_digital_signal_format
{
	maformat_none = '0',
	maformat_dolby_digital_ac3,
	maformat_dolby_digital_surround,
	maformat_dolby_digital_surround_ex,
	maformat_dts,
	maformat_dts_es_discreate,
	maformat_dts_es_matrix,
	maformat_aac,
	maformat_mpeg,
	maformat_mlp,
	maformat_pcm = 'A',
	maformat_hdcd,
	maformat_dsd,
	maformat_reserved1,
	maformat_reserved2,
	maformat_other
};

enum ma_sampling_frequency
{
	masample_out_of_range = '0',
	masample_32khz,
	masample_44k1hz,
	masample_48khz,
	masample_88k2hz,
	masample_96khz,
	masample_176k4hz,
	masample_192khz,

	masample_not_digital = 'F'
};

enum ma_tuner_mode
{
	matuner_none = '0',
	matuner_mono,
	matuner_auto
};

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
};

#endif /*STATUS_H*/
