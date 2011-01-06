/*
 * Copyright (c) 2011 Pelle Johansson
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

ESTART(bool)
EV(bool, off, 0)
EV(bool, on, 1)
EEND(bool)

ESTART(source)
EV(source, off, '0')
EV(source, tv, '1')
EV(source, dvd, '2')
EV(source, vcr1, '3')
EV(source, dss, '5')
EV(source, aux1, '9')
EV(source, aux2, 'A')
EV(source, cd, 'C')
EV(source, cdr, 'D')
EV(source, tape, 'E')
EV(source, tuner, 'F')
EV(source, fm, 'G')
EV(source, am, 'H')
EV(source, xm, 'J')
EV(source, 71c, 'N')
EEND(source)

ESTART(surround_mode)
EV(surround_mode, auto, '0')
EV(surround_mode, stereo, '1')
EV(surround_mode, dolby, '2')
EV(surround_mode, dolby_pl2x_movie, '3')
EV(surround_mode, dolby_pl2_movie, '4')
EV(surround_mode, dolby_pl2x_music, '5')
EV(surround_mode, dolby_pl2_music, '6')
EV(surround_mode, dolby_pl2x_game, '7')
EV(surround_mode, dolby_pl2_game, '8')
EV(surround_mode, dolby_pl, '9')
EV(surround_mode, ex_es, 'A')
EV(surround_mode, virtual_61, 'B')
EV(surround_mode, dts_es, 'E')
EV(surround_mode, neo6_cinema, 'F')
EV(surround_mode, neo6_music, 'G')
EV(surround_mode, multi_channel_stereo, 'H')
EV(surround_mode, cs2_cinema, 'I')
EV(surround_mode, cs2_music, 'J')
EV(surround_mode, cs2_mono, 'K')
EV(surround_mode, virtual, 'L')
EV(surround_mode, dts, 'M')
EV(surround_mode, masurr_ddplus_pl2x_movie, 'N')
EV(surround_mode, masurr_ddplus_pl2x_music, 'O')
EV(surround_mode, masurr_source_direct, 'T')
EV(surround_mode, masurr_pure_direct, 'U')
EEND(surround_mode)

ESTART(dolby_headphone_mode)
EV(dolby_headphone_mode, bypass, '0')
EV(dolby_headphone_mode, dh1, '1')
EV(dolby_headphone_mode, dh1_pl2_movie, '2')
EV(dolby_headphone_mode, dh1_pl2_music, '3')
EEND(dolby_headphone_mode)

ESTART(tuner_band)
EV(tuner_band, xm, 0)
EV(tuner_band, am, 1)
EV(tuner_band, fm, 2)
EEND(tuner_band)

ESTART(source_state)
EV(source_state, unknown, '0')
EV(source_state, off, '1')
EV(source_state, on, '2')
EEND(source_state)

ESTART(digital_signal_format)
EV(digital_signal_format, none, '0')
EV(digital_signal_format, dolby_digital_ac3, '1')
EV(digital_signal_format, dolby_digital_surround, '2')
EV(digital_signal_format, dolby_digital_surround_ex, '3')
EV(digital_signal_format, dts, '4')
EV(digital_signal_format, dts_es_discreate, '5')
EV(digital_signal_format, dts_es_matrix, '6')
EV(digital_signal_format, aac, '7')
EV(digital_signal_format, mpeg, '8')
EV(digital_signal_format, mlp, '9')
EV(digital_signal_format, pcm, 'A')
EV(digital_signal_format, hdcd, 'B')
EV(digital_signal_format, dsd, 'C')
EV(digital_signal_format, reserved1, 'D')
EV(digital_signal_format, reserved2, 'E')
EV(digital_signal_format, other, 'F')
EEND(digital_signal_format)

ESTART(sampling_frequency)
EV(sampling_frequency, out_of_range, '0')
EV(sampling_frequency, 32khz, '1')
EV(sampling_frequency, 44k1hz, '2')
EV(sampling_frequency, 48khz, '3')
EV(sampling_frequency, 88k2hz, '4')
EV(sampling_frequency, 96khz, '5')
EV(sampling_frequency, 176k4hz, '6')
EV(sampling_frequency, 192khz, '7')
EV(sampling_frequency, not_digital, 'F')
EEND(sampling_frequency)

ESTART(tuner_mode)
EV(tuner_mode, none, '0')
EV(tuner_mode, mono, '1')
EV(tuner_mode, auto, '2')
EEND(tuner_mode)

