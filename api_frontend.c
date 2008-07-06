
#include "api.h"

#include <search.h>
#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#include "serialize.h"

struct ma_notify_info
{
	int fd;
	const char *code;
	int field;

	void *cb;
	void *cbarg;
	struct ma_notify_info *next;
};

struct ma_code_info
{
	const char *code;
	struct ma_notify_info *notify_chain;
};

struct ma_fd_info
{
	int fd;
	struct ma_code_info *codes;
	size_t num_codes;
	size_t alloced_codes;

	char read_buffer[256];
	size_t read_buffer_used;
};

struct ma_fd_info *fd_infos;
size_t num_fd_infos;

struct ma_status_state
{
	int done;
	union {
		enum ma_bool *boolp;
		int *intp;
		enum ma_source *sourcep;
		enum ma_source_state *source_statep;
		enum ma_digital_signal_format *digital_signal_formatp;
		enum ma_sampling_frequency *sampling_frequencyp;
		enum ma_surround_mode *surround_modep;
		enum ma_dolby_headphone_mode *dolby_headphone_modep;
		enum ma_tuner_band *tuner_bandp;
		enum ma_tuner_mode *tuner_modep;
		char *stringp;
	} val;
};

static int
fd_info_cmp (const void *a, const void *b) {
	const struct ma_fd_info *fd_info_a = a;
	const struct ma_fd_info *fd_info_b = b;

	return fd_info_a->fd - fd_info_b->fd;
}

static int
code_info_cmp (const void *a, const void *b) {
	const struct ma_code_info *code_info_a = a;
	const struct ma_code_info *code_info_b = b;

	return strcmp(code_info_a->code, code_info_b->code);
}

static int
send_emmediate_request (int fd, const char *code) {
	struct iovec vecs[3];

	vecs[0].iov_base = (char*)"query ";
	vecs[0].iov_len = sizeof ("query ") - 1;
	vecs[1].iov_base = (char*)code;
	vecs[1].iov_len = strlen (code);
	vecs[2].iov_base = (char*)"\n";
	vecs[2].iov_len = 1;

	return writev(fd, vecs, 3);
}

static int
send_notify_start (int fd, const char *code) {
	struct iovec vecs[3];

	vecs[0].iov_base = (char*)"start ";
	vecs[0].iov_len = sizeof ("start ") - 1;
	vecs[1].iov_base = (char*)code;
	vecs[1].iov_len = strlen (code);
	vecs[2].iov_base = (char*)"\n";
	vecs[2].iov_len = 1;

	return writev(fd, vecs, 3);
}

static int
send_notify_stop (int fd, const char *code) {
	struct iovec vecs[3];

	vecs[0].iov_base = (char*)"stop ";
	vecs[0].iov_len = sizeof ("stop ") - 1;
	vecs[1].iov_base = (char*)code;
	vecs[1].iov_len = strlen (code);
	vecs[2].iov_base = (char*)"\n";
	vecs[2].iov_len = 1;

	return writev(fd, vecs, 3);
}

void
ma_stop_notify (ma_notify_token_t token) {
	struct ma_notify_info *notify_info = token;
	struct ma_fd_info fd_search = {notify_info->fd};
	struct ma_fd_info *fd_info = lfind (&fd_search, fd_infos, &num_fd_infos, sizeof (*fd_infos), fd_info_cmp);
	struct ma_code_info code_search = {notify_info->code};
	struct ma_code_info *code_info;
	struct ma_notify_info *pinfo;

	if (!fd_info)
		return;

	code_info = lfind (&code_search, fd_info->codes, &fd_info->num_codes, sizeof (*fd_info->codes), code_info_cmp);
	if (!code_info)
		return;

	if (code_info->notify_chain == notify_info)
		code_info->notify_chain = notify_info->next;
	else {
		for (pinfo = code_info->notify_chain; pinfo && pinfo->next != notify_info; pinfo = pinfo->next)
			;
		if (pinfo)
			pinfo->next = notify_info->next;
	}
	free (notify_info);
	if (!code_info->notify_chain) {
		size_t cindx = code_info - fd_info->codes;

		send_notify_stop(notify_info->fd, code_info->code);

		if (cindx < --fd_info->num_codes)
			memmove(fd_info->codes + cindx, fd_info->codes + cindx + 1, fd_info->num_codes - cindx);
	}
}

static ma_notify_token_t
ma_notify (int fd, const char *code, int field, int emmediate, void *cb, void *cbarg) {
	struct ma_fd_info fd_search = {fd};
	struct ma_fd_info *fd_info = lfind (&fd_search, fd_infos, &num_fd_infos, sizeof (*fd_infos), fd_info_cmp);
	struct ma_code_info code_search = {code};
	struct ma_code_info *code_info;
	struct ma_notify_info *notify_info;
	int res;

	notify_info = malloc (sizeof (*notify_info));
	if (!notify_info)
		return NULL;

	notify_info->fd = fd;
	notify_info->code = code;
	notify_info->field = field;
	notify_info->cb = cb;
	notify_info->cbarg = cbarg;

	if (!fd_info) {
		struct ma_fd_info *new_fd_infos = realloc (fd_infos, ++num_fd_infos * sizeof (*fd_infos));

		if (!new_fd_infos) {
			free (notify_info);
			return NULL;
		}
		fd_infos = new_fd_infos;
		fd_info = fd_infos + num_fd_infos - 1;
		*fd_info = fd_search;
	}

	code_info = lfind (&code_search, fd_info->codes, &fd_info->num_codes, sizeof (*fd_info->codes), code_info_cmp);
	if (code_info) {
		if (emmediate) {
			res = send_emmediate_request(fd, code);
			if (res < 0) {
				free (notify_info);
				return NULL;
			}
		}

		notify_info->next = code_info->notify_chain;
		code_info->notify_chain = notify_info;

		return notify_info;
	}

	if (emmediate)
		res = send_emmediate_request (fd, code);
	else
		res = send_notify_start (fd, code);
	if (res < 0) {
		free (notify_info);
		return NULL;
	}

	if (fd_info->alloced_codes == fd_info->num_codes) {
		if (!fd_info->alloced_codes) {
			fd_info->codes = malloc (8 * sizeof (*fd_info->codes));
			if (!fd_info->codes) {
				free (notify_info);
				if (!emmediate)
					send_notify_stop (fd, code);
				return NULL;
			}
			fd_info->alloced_codes = 8;
		} else {
			struct ma_code_info *new_codes = realloc (fd_info->codes,
					2 * fd_info->alloced_codes * sizeof (*fd_info->codes));
			if (!new_codes) {
				free (notify_info);
				if (!emmediate)
					send_notify_stop (fd, code);
				return NULL;
			}
			fd_info->codes = new_codes;
			fd_info->alloced_codes *= 2;
		}
	}
	code_info = fd_info->codes + fd_info->num_codes++;
	code_info->code = code;
	notify_info->next = NULL;
	code_info->notify_chain = notify_info;
	return notify_info;
}

int
ma_open_local (const char *path) {
	struct sockaddr_un unaddr = {0};
	int fd = socket (PF_LOCAL, SOCK_STREAM, 0);

	if (fd < 0)
		return -1;

	unaddr.sun_family = AF_LOCAL;
	strncpy (unaddr.sun_path, path, sizeof (unaddr.sun_path) - 1);
	unaddr.sun_path[sizeof (unaddr.sun_path) - 1] = '\0';

	if (connect (fd, (struct sockaddr*)&unaddr, sizeof (unaddr))) {
		close (fd);
		return -1;
	}

	return fd;
}

void
ma_close (int fd) {
	struct ma_fd_info fd_search = {fd};
	struct ma_fd_info *fd_info = lfind (&fd_search, fd_infos, &num_fd_infos, sizeof (*fd_infos), fd_info_cmp);
	struct ma_code_info *code_info;
	struct ma_notify_info *notify_info;

	if (fd_info) {
		size_t findx = fd_info - fd_infos;

		for (code_info = fd_info->codes; code_info < fd_info->codes + fd_info->num_codes; code_info++) {
			while (code_info->notify_chain) {
				notify_info = code_info->notify_chain;
				code_info->notify_chain = notify_info->next;
				free (notify_info);
			}
		}
		free (fd_info->codes);

		if (findx < --num_fd_infos)
			memmove(fd_infos + findx, fd_infos + findx + 1, num_fd_infos - findx);
	}
	close (fd);
}

struct
ma_read_value {
	enum ma_type type;
	union {
		enum ma_bool boolv;
		int intv;
		enum ma_source sourcev;
		enum ma_source_state source_statev;
		enum ma_digital_signal_format digital_signal_formatv;
		enum ma_sampling_frequency sampling_frequencyv;
		enum ma_surround_mode surround_modev;
		enum ma_dolby_headphone_mode dolby_headphone_modev;
		enum ma_tuner_band tuner_bandv;
		enum ma_tuner_mode tuner_modev;
		const char *stringv;
	} val;
};

int
ma_read (int fd) {
	struct ma_fd_info fd_search = {fd};
	struct ma_fd_info *fd_info = lfind (&fd_search, fd_infos, &num_fd_infos, sizeof (*fd_infos), fd_info_cmp);
	int res;

	if (!fd_info) {
		char buf[256];

		/* Just read some data and discard it. */
		return read (fd, buf, sizeof (buf));
	}

	res = read (fd, fd_info->read_buffer + fd_info->read_buffer_used, sizeof (fd_info->read_buffer) - fd_info->read_buffer_used);
	if (res < 0)
		return -1;

	fd_info->read_buffer_used += res;

	while (fd_info->read_buffer_used > 0) {
		char *bp;
		char *ep = fd_info->read_buffer + fd_info->read_buffer_used;

		if ((bp = memchr(fd_info->read_buffer, '\0', fd_info->read_buffer_used))) {
			struct ma_read_value values[10];
			int num_vals = 0;
			int ures;
			struct ma_code_info code_search = {fd_info->read_buffer};
			struct ma_code_info *code_info;

			if (++bp >= ep)
				return res;
			while (1) {
				if (unserialize_type(&bp, ep - bp, &values[num_vals].type))
					return res;
				if (values[num_vals].type == matype_EOD)
					break;

				switch (values[num_vals].type) {
				case matype_bool:
					ures = unserialize_bool(&bp, ep - bp, &values[num_vals].val.boolv);
					break;
				case matype_int:
					ures = unserialize_int(&bp, ep - bp, &values[num_vals].val.intv);
					break;
				case matype_source:
					ures = unserialize_source(&bp, ep - bp, &values[num_vals].val.sourcev);
					break;
				case matype_source_state:
					ures = unserialize_source_state(&bp, ep - bp, &values[num_vals].val.source_statev);
					break;
				case matype_digital_signal_format:
					ures = unserialize_digital_signal_format(&bp, ep - bp, &values[num_vals].val.digital_signal_formatv);
					break;
				case matype_sampling_frequency:
					ures = unserialize_sampling_frequency(&bp, ep - bp, &values[num_vals].val.sampling_frequencyv);
					break;
				case matype_surround_mode:
					ures = unserialize_surround_mode(&bp, ep - bp, &values[num_vals].val.surround_modev);
					break;
				case matype_dolby_headphone_mode:
					ures = unserialize_dolby_headphone_mode(&bp, ep - bp, &values[num_vals].val.dolby_headphone_modev);
					break;
				case matype_tuner_band:
					ures = unserialize_tuner_band(&bp, ep - bp, &values[num_vals].val.tuner_bandv);
					break;
				case matype_tuner_mode:
					ures = unserialize_tuner_mode(&bp, ep - bp, &values[num_vals].val.tuner_modev);
					break;
				case matype_string:
					ures = unserialize_string(&bp, ep - bp, &values[num_vals].val.stringv);
					break;
				default:
					ures = 1;
					break;
				}
				if (ures)
					return res;
				num_vals++;
			}
			code_info = lfind (&code_search, fd_info->codes, &fd_info->num_codes, sizeof (*fd_info->codes), code_info_cmp);
			if (code_info) {
				struct ma_notify_info *notify_info;

				for (notify_info = code_info->notify_chain; notify_info; notify_info = notify_info->next) {
					if (notify_info->field < num_vals) {
						switch (values[notify_info->field].type) {
						case matype_bool:
							((ma_notify_bool_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.boolv, notify_info->cbarg);
							break;
						case matype_int:
							((ma_notify_int_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.intv, notify_info->cbarg);
							break;
						case matype_source:
							((ma_notify_source_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.sourcev, notify_info->cbarg);
							break;
						case matype_source_state:
							((ma_notify_source_state_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.source_statev, notify_info->cbarg);
							break;
						case matype_digital_signal_format:
							((ma_notify_digital_signal_format_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.digital_signal_formatv, notify_info->cbarg);
							break;
						case matype_sampling_frequency:
							((ma_notify_sampling_frequency_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.sampling_frequencyv, notify_info->cbarg);
							break;
						case matype_surround_mode:
							((ma_notify_surround_mode_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.surround_modev, notify_info->cbarg);
							break;
						case matype_dolby_headphone_mode:
							((ma_notify_dolby_headphone_mode_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.dolby_headphone_modev, notify_info->cbarg);
							break;
						case matype_tuner_band:
							((ma_notify_tuner_band_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.tuner_bandv, notify_info->cbarg);
							break;
						case matype_tuner_mode:
							((ma_notify_tuner_mode_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.tuner_modev, notify_info->cbarg);
							break;
						case matype_string:
							((ma_notify_string_cb_t)notify_info->cb)(fd, notify_info, values[notify_info->field].val.stringv, notify_info->cbarg);
							break;
						default:
							break;
						}
					}
				}
			}

			if (bp < ep)
				memmove (fd_info->read_buffer, bp, ep - bp);
			fd_info->read_buffer_used = ep - bp;
		}
	}

	return res;
}


#define STATUS_CB(cbtype, valtype) \
	void \
	ma_status_ ## cbtype ## _cb (int fd, ma_notify_token_t token, valtype value, void *cbarg) { \
		struct ma_status_state *state = cbarg; \
 \
		state->done = 1; \
		*state->val.cbtype ## p = value; \
		ma_stop_notify (token); \
	}

STATUS_CB (bool, enum ma_bool);
STATUS_CB (int, int);
STATUS_CB (source, enum ma_source);
STATUS_CB (source_state, enum ma_source_state);
STATUS_CB (digital_signal_format, enum ma_digital_signal_format);
STATUS_CB (sampling_frequency, enum ma_sampling_frequency);
STATUS_CB (surround_mode, enum ma_surround_mode);
STATUS_CB (dolby_headphone_mode, enum ma_dolby_headphone_mode);
STATUS_CB (tuner_band, enum ma_tuner_band);
STATUS_CB (tuner_mode, enum ma_tuner_mode);

void
ma_status_string_cb (int fd, ma_notify_token_t token, const char *value, void *cbarg) {
	struct ma_status_state *state = cbarg;

	state->done = 1;
	strcpy (state->val.stringp, value); /* XXX bounds check */
	ma_stop_notify (token);
}

#define NOTIFY(name, code, field, cbtype, valtype) \
	ma_notify_token_t \
	ma_notify_ ## name (int fd, ma_notify_ ## cbtype ## _cb_t cb, void *cbarg) { \
		return ma_notify (fd, code, field, 0, cb, cbarg); \
	} \
 \
	int \
	ma_status_ ## name (int fd, valtype *value) { \
		struct ma_status_state state = {0}; \
 \
		state.val.cbtype ## p = value; \
		if (!ma_notify (fd, code, field, 1, ma_status_ ## cbtype ## _cb, &state)) \
			return -1; \
 \
		while (!state.done) { \
			if (ma_read (fd) < 0) \
				return -1; \
		} \
		return 0; \
	}

#define STATUS(name, code, field, cbtype, valtype) \
	int \
	ma_status_ ## name (int fd, valtype *value) { \
		struct ma_status_state state = {0}; \
 \
		state.val.cbtype ## p = value; \
		if (!ma_notify (fd, code, field, 1, ma_status_ ## cbtype ## _cb, &state)) \
			return -1; \
 \
		while (!state.done) { \
			if (ma_read (fd) < 0) \
				return -1; \
		} \
		return 0; \
	}

NOTIFY (power, "PWR", 0, bool, enum ma_bool);
NOTIFY (audio_att, "ATT", 0, bool, enum ma_bool);
NOTIFY (audio_mute, "AMT", 0, bool, enum ma_bool);
NOTIFY (video_mute, "VMT", 0, bool, enum ma_bool);
NOTIFY (volume, "VOL", 0, int, int);
NOTIFY (tone_bass, "TOB", 0, int, int);
NOTIFY (tone_treble, "TOT", 0, int, int);
NOTIFY (video_source, "SRC", 0, source, enum ma_source);
NOTIFY (audio_source, "SRC", 1, source, enum ma_source);
NOTIFY (multi_channel_input, "71C", 0, bool, enum ma_bool);
NOTIFY (hdmi_audio_through, "HAM", 0, bool, enum ma_bool);
NOTIFY (source_input_state, "IST", 0, source_state, enum ma_source_state);
NOTIFY (sleep, "SLP", 0, int, int);
NOTIFY (menu, "MNU", 0, bool, enum ma_bool);
STATUS (dc_trigger, "DCT", 0, bool, enum ma_bool);
NOTIFY (front_key_lock, "FKL", 0, bool, enum ma_bool);
NOTIFY (simple_setup, "SSU", 0, bool, enum ma_bool);
NOTIFY (digital_signal_format, "SIG", 0, digital_signal_format, enum ma_digital_signal_format);
NOTIFY (sampling_frequency, "SFQ", 0, sampling_frequency, enum ma_sampling_frequency);
NOTIFY (channel_status_lfe, "CHS", 0, bool, enum ma_bool);
NOTIFY (channel_status_surr_l, "CHS", 1, bool, enum ma_bool);
NOTIFY (channel_status_surr_r, "CHS", 2, bool, enum ma_bool);
NOTIFY (channel_status_subwoofer, "CHS", 3, bool, enum ma_bool);
NOTIFY (channel_status_front_l, "CHS", 4, bool, enum ma_bool);
NOTIFY (channel_status_front_r, "CHS", 5, bool, enum ma_bool);
NOTIFY (channel_status_center, "CHS", 6, bool, enum ma_bool);
NOTIFY (surround_mode, "SUR", 0, surround_mode, enum ma_surround_mode);
NOTIFY (test_tone_enabled, "TTO", 0, bool, enum ma_bool);
NOTIFY (test_tone_auto, "TTO", 1, bool, enum ma_bool);
NOTIFY (test_tone_channel, "TTO", 2, int, int);
NOTIFY (night_mode, "NGT", 0, bool, enum ma_bool);
NOTIFY (dolby_headphone_mode, "DHM", 0, dolby_headphone_mode, enum ma_dolby_headphone_mode);
NOTIFY (lip_sync, "LIP", 0, int, int);
NOTIFY (tuner_band, "TFQ", 0, tuner_band, enum ma_tuner_band);
NOTIFY (tuner_frequency, "TFQ", 1, int, int);
NOTIFY (tuner_preset, "TPR", 0, int, int);
NOTIFY (tuner_preset_info, "TPI", 0, bool, enum ma_bool);
NOTIFY (tuner_mode, "TMD", 0, tuner_mode, enum ma_tuner_mode);
NOTIFY (xm_in_search, "CAT", 0, bool, enum ma_bool);
NOTIFY (xm_category, "CAT", 1, int, int);
NOTIFY (xm_channel_name, "CHN", 0, string, char);
NOTIFY (xm_artist_name, "ARN", 0, string, char);
NOTIFY (xm_song_title, "SON", 0, string, char);
NOTIFY (xm_category_name, "CTN", 0, string, char);
NOTIFY (multiroom_power, "MPW", 0, bool, enum ma_bool);
NOTIFY (multiroom_audio_mute, "MAM", 0, bool, enum ma_bool);
NOTIFY (multiroom_volume, "MVL", 0, int, int);
NOTIFY (multiroom_volume_fixed, "MVS", 0, bool, enum ma_bool);
NOTIFY (multiroom_video_source, "MSC", 0, source, enum ma_source);
NOTIFY (multiroom_audio_source, "MSC", 1, source, enum ma_source);
NOTIFY (multiroom_sleep, "MSL", 0, int, int);
NOTIFY (multiroom_speaker, "MSP", 0, bool, enum ma_bool);
NOTIFY (multiroom_speaker_volume, "MSV", 0, int, int);
NOTIFY (multiroom_speaker_volume_fixed, "MSS", 0, bool, enum ma_bool);
NOTIFY (multiroom_speaker_audio_mute, "MSM", 0, bool, enum ma_bool);
NOTIFY (multiroom_tuner_band, "MTF", 0, tuner_band, enum ma_tuner_band);
NOTIFY (multiroom_tuner_frequency, "MTF", 1, int, int);
NOTIFY (multiroom_tuner_preset, "MTP", 0, int, int);
NOTIFY (multiroom_tuner_mode, "MTM", 0, tuner_mode, enum ma_tuner_mode);

