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

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

#include "api.h"

int caught;

struct notify_cb
{
	const char *name;
	int once;
};

void
notify_bool_cb (int fd, ma_notify_token_t token, enum ma_bool value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;

	caught++;
	printf ("%s %s\n", cbdata->name, value == mabool_on ? "on" : "off");
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_bool (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*bool_func)(int, ma_notify_bool_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!bool_func(fd, notify_bool_cb, cbdata))
		err (1, "cli_notify");
}

void
handle_status_bool (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*bool_func)(int, ma_notify_bool_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = 1;

	if (!bool_func(fd, notify_bool_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_int_cb (int fd, ma_notify_token_t token, int value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;

	caught++;
	printf ("%s %d\n", cbdata->name, value);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_int (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*int_func)(int, ma_notify_int_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!int_func(fd, notify_int_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_source_cb (int fd, ma_notify_token_t token, enum ma_source value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case masource_off:
		strval = "off";
		break;
	case masource_tv:
		strval = "tv";
		break;
	case masource_dvd:
		strval = "dvd";
		break;
	case masource_vcr1:
		strval = "vcr1";
		break;
	case masource_dss:
		strval = "dss";
		break;
	case masource_aux1:
		strval = "aux1";
		break;
	case masource_aux2:
		strval = "aux2";
		break;
	case masource_cd:
		strval = "cd";
		break;
	case masource_cdr:
		strval = "cdr";
		break;
	case masource_tape:
		strval = "tape";
		break;
	case masource_tuner:
		strval = "tuner";
		break;
	case masource_fm:
		strval = "fm";
		break;
	case masource_am:
		strval = "am";
		break;
	case masource_xm:
		strval = "xm";
		break;
	case masource_71C:
		strval = "7.1C";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_source (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*source_func)(int, ma_notify_source_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!source_func(fd, notify_source_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_source_state_cb (int fd, ma_notify_token_t token, enum ma_source_state value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case mastate_unknown:
		strval = "unknown";
		break;
	case mastate_off:
		strval = "off";
		break;
	case mastate_on:
		strval = "on";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_source_state (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*source_state_func)(int, ma_notify_source_state_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!source_state_func(fd, notify_source_state_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_digital_signal_format_cb (int fd, ma_notify_token_t token, enum ma_digital_signal_format value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case maformat_none:
		strval = "none";
		break;
	case maformat_dolby_digital_ac3:
		strval = "dolby digital ac3";
		break;
	case maformat_dolby_digital_surround:
		strval = "dolby digital surround";
		break;
	case maformat_dolby_digital_surround_ex:
		strval = "dolby digital surround ex";
		break;
	case maformat_dts:
		strval = "dts";
		break;
	case maformat_dts_es_discreate:
		strval = "dts es discreate";
		break;
	case maformat_dts_es_matrix:
		strval = "dts es matrix";
		break;
	case maformat_aac:
		strval = "aac";
		break;
	case maformat_mpeg:
		strval = "mpeg";
		break;
	case maformat_mlp:
		strval = "mlp";
		break;
	case maformat_pcm:
		strval = "pcm";
		break;
	case maformat_hdcd:
		strval = "hdcd";
		break;
	case maformat_dsd:
		strval = "dsd";
		break;
	case maformat_reserved1:
		strval = "reserved1";
		break;
	case maformat_reserved2:
		strval = "reserved2";
		break;
	case maformat_other:
		strval = "other";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_digital_signal_format (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*digital_signal_format_func)(int, ma_notify_digital_signal_format_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!digital_signal_format_func(fd, notify_digital_signal_format_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_sampling_frequency_cb (int fd, ma_notify_token_t token, enum ma_sampling_frequency value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case masample_out_of_range:
		strval = "out of range";
		break;
	case masample_32khz:
		strval = "32 kHz";
		break;
	case masample_44k1hz:
		strval = "44.1 kHz";
		break;
	case masample_48khz:
		strval = "48 kHz";
		break;
	case masample_88k2hz:
		strval = "88.2 kHz";
		break;
	case masample_96khz:
		strval = "96 kHz";
		break;
	case masample_176k4hz:
		strval = "176.4 kHz";
		break;
	case masample_192khz:
		strval = "192 kHz";
		break;
	case masample_not_digital:
		strval = "not digital";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_sampling_frequency (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*sampling_frequency_func)(int, ma_notify_sampling_frequency_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!sampling_frequency_func(fd, notify_sampling_frequency_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_surround_mode_cb (int fd, ma_notify_token_t token, enum ma_surround_mode value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case masurr_auto:
		strval = "auto";
		break;
	case masurr_stereo:
		strval = "stereo";
		break;
	case masurr_dolby:
		strval = "dolby";
		break;
	case masurr_dolby_pl2x_movie:
		strval = "dolby pl2x movie";
		break;
	case masurr_dolby_pl2_movie:
		strval = "dolby pl2 movie";
		break;
	case masurr_dolby_pl2x_music:
		strval = "dolby pl2x music";
		break;
	case masurr_dolby_pl2_music:
		strval = "dolby pl2 music";
		break;
	case masurr_dolby_pl2x_game:
		strval = "dolby pl2x game";
		break;
	case masurr_dolby_pl2_game:
		strval = "dolby pl2 game";
		break;
	case masurr_dolby_pl:
		strval = "dolby pl";
		break;
	case masurr_ex_es:
		strval = "ex/es";
		break;
	case masurr_virtual_61:
		strval = "virtual 6.1";
		break;
	case masurr_dts_es:
		strval = "dts es";
		break;
	case masurr_neo6_cinema:
		strval = "neo6 cinema";
		break;
	case masurr_neo6_music:
		strval = "neo6 music";
		break;
	case masurr_multi_channel_stereo:
		strval = "multi channel stereo";
		break;
	case masurr_cs2_cinema:
		strval = "cs2 cinema";
		break;
	case masurr_cs2_music:
		strval = "cs2 music";
		break;
	case masurr_cs2_mono:
		strval = "cs2 mono";
		break;
	case masurr_virtual:
		strval = "virtual";
		break;
	case masurr_dts:
		strval = "dts";
		break;
	case masurr_ddplus_pl2x_movie:
		strval = "dd+ pl2x movie";
		break;
	case masurr_ddplus_pl2x_music:
		strval = "dd+ pl2x music";
		break;
	case masurr_source_direct:
		strval = "source direct";
		break;
	case masurr_pure_direct:
		strval = "pure direct";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_surround_mode (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*surround_mode_func)(int, ma_notify_surround_mode_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!surround_mode_func(fd, notify_surround_mode_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_dolby_headphone_mode_cb (int fd, ma_notify_token_t token, enum ma_dolby_headphone_mode value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case maheadphone_bypass:
		strval = "bypass";
		break;
	case maheadphone_dh1:
		strval = "dh1";
		break;
	case maheadphone_dh1_pl2_movie:
		strval = "dh1 + pl2 movie";
		break;
	case maheadphone_dh1_pl2_music:
		strval = "dh1 + pl2 music";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_dolby_headphone_mode (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*dolby_headphone_mode_func)(int, ma_notify_dolby_headphone_mode_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!dolby_headphone_mode_func(fd, notify_dolby_headphone_mode_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_tuner_band_cb (int fd, ma_notify_token_t token, enum ma_tuner_band value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case matuner_xm:
		strval = "xm";
		break;
	case matuner_am:
		strval = "am";
		break;
	case matuner_fm:
		strval = "fm";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_tuner_band (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*tuner_band_func)(int, ma_notify_tuner_band_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!tuner_band_func(fd, notify_tuner_band_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_tuner_mode_cb (int fd, ma_notify_token_t token, enum ma_tuner_mode value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;
	const char *strval = NULL;

	caught++;
	switch (value) {
	case matuner_none:
		strval = "none";
		break;
	case matuner_mono:
		strval = "mono";
		break;
	case matuner_auto:
		strval = "auto";
		break;
	}
	printf ("%s %s\n", cbdata->name, strval);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_tuner_mode (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*tuner_mode_func)(int, ma_notify_tuner_mode_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!tuner_mode_func(fd, notify_tuner_mode_cb, cbdata))
		err (1, "cli_notify");
}

void
notify_string_cb (int fd, ma_notify_token_t token, const char *value, void *cbarg) {
	struct notify_cb *cbdata = cbarg;

	caught++;
	printf ("%s %s\n", cbdata->name, value);
	fflush (stdout);
	if (cbdata->once) {
		ma_stop_notify (token);
		free (cbdata);
	}
}

void
handle_notify_string (int fd, void *func, const char *name, int once) {
	ma_notify_token_t (*string_func)(int, ma_notify_string_cb_t, void *) = func;
	struct notify_cb *cbdata = malloc (sizeof (*cbdata));

	if (!cbdata)
		err (1, "cli_notify");

	cbdata->name = name;
	cbdata->once = once;

	if (!string_func(fd, notify_string_cb, cbdata))
		err (1, "cli_notify");
}

#include "notify_command.h"

int
cli_notify (int fd, int argc, char *argv[], int once) {
	int i;
	int num = 0;

	for (i = 0; i < argc; i++) {
		const struct notify_command *cmd = notify_command (argv[i], strlen (argv[i]));
		if (cmd) {
			cmd->handler (fd, cmd->func, cmd->name, once);
			num++;
		} else
			warn ("No matching notification: %s", argv[i]);
	}
	if (!num)
		err (1, "Could not find any matching notifications");

	while (1) {
		int res = ma_read (fd);

		if (res < 0)
			err (1, "ma_read");
		if (res == 0)
			errx (1, "EOF");

		if (once && caught >= num)
			break;
	}
	return 0;
}
