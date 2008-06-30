#ifndef _MARANTZ_API_H
#define _MARANTZ_API_H

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
	mastate_unknown = '0',
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

typedef struct ma_notify_info *ma_notify_token_t;
typedef void (*ma_notify_bool_cb_t)(int fd, ma_notify_token_t token, enum ma_bool value, void *cbarg);
typedef void (*ma_notify_int_cb_t)(int fd, ma_notify_token_t token, int value, void *cbarg);
typedef void (*ma_notify_source_cb_t)(int fd, ma_notify_token_t token, enum ma_source value, void *cbarg);
typedef void (*ma_notify_source_state_cb_t)(int fd, ma_notify_token_t token, enum ma_source_state value, void *cbarg);
typedef void (*ma_notify_digital_signal_format_cb_t)(int fd, ma_notify_token_t token, enum ma_digital_signal_format value, void *cbarg);
typedef void (*ma_notify_sampling_frequency_cb_t)(int fd, ma_notify_token_t token, enum ma_sampling_frequency value, void *cbarg);
typedef void (*ma_notify_surround_mode_cb_t)(int fd, ma_notify_token_t token, enum ma_surround_mode value, void *cbarg);
typedef void (*ma_notify_dolby_headphone_mode_cb_t)(int fd, ma_notify_token_t token, enum ma_dolby_headphone_mode value, void *cbarg);
typedef void (*ma_notify_tuner_band_cb_t)(int fd, ma_notify_token_t token, enum ma_tuner_band value, void *cbarg);
typedef void (*ma_notify_tuner_mode_cb_t)(int fd, ma_notify_token_t token, enum ma_tuner_mode value, void *cbarg);
typedef void (*ma_notify_string_cb_t)(int fd, ma_notify_token_t token, const char *value, void *cbarg);

int ma_open_local (const char *path);
void ma_close (int fd);
int ma_read (int fd);

void ma_stop_notify (ma_notify_token_t token);

#define NOTIFY(name, cbtype, valtype) \
	ma_notify_token_t ma_notify_ ## name (int fd, ma_notify_ ## cbtype ## _cb_t cb, void *cbarg); \
	int ma_status_ ## name (int fd, valtype *value);

#define STATUS(name, valtype) \
	int ma_status_ ## name (int fd, valtype *value);

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

#endif /*_MARANTZ_API_H*/
