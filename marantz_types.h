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

#ifndef MARANTZ_TYPES_H
#define MARANTZ_TYPES_H

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

#endif /*MORANTZ_TYPES_H*/
