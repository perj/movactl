
#include "serialize.h"

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define EAT_LEN \
	if (*buflen + needlen > maxlen) \
		return SERIALIZE_BUFFER_FULL; \
	*buflen += needlen

#define STORE(type, val) \
	*(type*)*buf = val; \
	*buf = (char*)*buf + sizeof (type);

#define READ(type, valp) \
	*valp = *(type*)*buf; \
	*buf += sizeof (type);

int
serialize_bool (void **buf, size_t *buflen, size_t maxlen, enum ma_bool value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_bool);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_int (void **buf, size_t *buflen, size_t maxlen, int value) {
	size_t needlen = sizeof (uint8_t) + sizeof (int32_t);

	EAT_LEN;
	STORE (uint8_t, matype_int);
	STORE (int32_t, htonl(value));
	return SERIALIZE_OK;
}

int
serialize_source (void **buf, size_t *buflen, size_t maxlen, enum ma_source value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_source);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_source_state (void **buf, size_t *buflen, size_t maxlen, enum ma_source_state value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_source_state);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_digital_signal_format (void **buf, size_t *buflen, size_t maxlen, enum ma_digital_signal_format value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_digital_signal_format);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_sampling_frequency (void **buf, size_t *buflen, size_t maxlen, enum ma_sampling_frequency value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_sampling_frequency);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_surround_mode (void **buf, size_t *buflen, size_t maxlen, enum ma_surround_mode value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_surround_mode);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_dolby_headphone_mode (void **buf, size_t *buflen, size_t maxlen, enum ma_dolby_headphone_mode value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_dolby_headphone_mode);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_tuner_band (void **buf, size_t *buflen, size_t maxlen, enum ma_tuner_band value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_tuner_band);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_tuner_mode (void **buf, size_t *buflen, size_t maxlen, enum ma_tuner_mode value) {
	size_t needlen = sizeof (uint8_t) + sizeof (uint8_t);

	EAT_LEN;
	STORE (uint8_t, matype_tuner_mode);
	STORE (uint8_t, value);
	return SERIALIZE_OK;
}

int
serialize_string (void **buf, size_t *buflen, size_t maxlen, const char *value) {
	size_t sl = strlen (value) + 1;
	size_t needlen = sizeof (uint8_t) + sl;

	EAT_LEN;
	STORE (uint8_t, matype_string);
	memcpy (*buf, value, sl);
	*buf = (char*)*buf + sl;
	return SERIALIZE_OK;
}

int
unserialize_type (char **buf, size_t buflen, enum ma_type *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int
unserialize_bool (char **buf, size_t buflen, enum ma_bool *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_int (char **buf, size_t buflen, int *value) {
	if (buflen < sizeof (int32_t))
		return 1;
	READ (int32_t, value);
	*value = ntohl(*value);
	return 0;
}

int unserialize_source (char **buf, size_t buflen, enum ma_source *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_source_state (char **buf, size_t buflen, enum ma_source_state *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_digital_signal_format (char **buf, size_t buflen, enum ma_digital_signal_format *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_sampling_frequency (char **buf, size_t buflen, enum ma_sampling_frequency *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_surround_mode (char **buf, size_t buflen, enum ma_surround_mode *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_dolby_headphone_mode (char **buf, size_t buflen, enum ma_dolby_headphone_mode *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_tuner_band (char **buf, size_t buflen, enum ma_tuner_band *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_tuner_mode (char **buf, size_t buflen, enum ma_tuner_mode *value) {
	if (buflen < sizeof (uint8_t))
		return 1;
	READ (uint8_t, value);
	return 0;
}

int unserialize_string (char **buf, size_t buflen, const char **value) {
	char *ep = memchr (*buf, '\0', buflen);

	if (!ep)
		return 1;
	*value = *buf;
	*buf = ep + 1;
	return 0;
}

