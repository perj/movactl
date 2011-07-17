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
EV(source, dtv, 0x100)
EV(source, analogue, 0x110)
EV(source, av1, 0x120)
EV(source, av2, 0x121)
EV(source, av3, 0x122)
EV(source, av4, 0x123)
EV(source, component, 0x140)
EV(source, rgb, 0x160)
EV(source, hdmi1, 0x190)
EV(source, hdmi2, 0x191)
EV(source, hdmi3, 0x192)
EV(source, hdmi4, 0x193)
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

ESTART(aspect_ratio)
EV(aspect_ratio, normal, 0x01)
EV(aspect_ratio, wide, 0x02)
EV(aspect_ratio, zoom, 0x04)
EV(aspect_ratio, original, 0x06)
EV(aspect_ratio, fourteen_nine, 0x07)
EV(aspect_ratio, just_scan, 0x09)
EV(aspect_ratio, full_wide, 0x0B)
EV(aspect_ratio, cinema_zoom_1, 0x10)
EV(aspect_ratio, cinema_zoom_2, 0x11)
EV(aspect_ratio, cinema_zoom_3, 0x12)
EV(aspect_ratio, cinema_zoom_4, 0x13)
EV(aspect_ratio, cinema_zoom_5, 0x14)
EV(aspect_ratio, cinema_zoom_6, 0x15)
EV(aspect_ratio, cinema_zoom_7, 0x16)
EV(aspect_ratio, cinema_zoom_8, 0x17)
EV(aspect_ratio, cinema_zoom_9, 0x18)
EV(aspect_ratio, cinema_zoom_10, 0x19)
EV(aspect_ratio, cinema_zoom_11, 0x1a)
EV(aspect_ratio, cinema_zoom_12, 0x1b)
EV(aspect_ratio, cinema_zoom_13, 0x1c)
EV(aspect_ratio, cinema_zoom_14, 0x1d)
EV(aspect_ratio, cinema_zoom_15, 0x1e)
EV(aspect_ratio, cinema_zoom_16, 0x1f)
EEND(aspect_ratio)

ESTART(video_mute)
EV(video_mute, off, 0)
EV(video_mute, on, 1)
EV(video_mute, picture, 0x10)
EEND(video_mute)

ESTART(colour_temperature)
EV(colour_temperature, medium, 0)
EV(colour_temperature, cool, 1)
EV(colour_temperature, warm, 2)
EEND(colour_temperature)

ESTART(energy_saving)
EV(energy_saving, off, 0)
EV(energy_saving, minimum, 1)
EV(energy_saving, medium, 2)
EV(energy_saving, maximum, 3)
EV(energy_saving, auto, 4)
EV(energy_saving, screen_off, 5)
EEND(energy_saving)

ESTART(tv_band)
EV(tv_band, analogue, 0)
EV(tv_band, dtv, 1)
EV(tv_band, radio, 2)
EEND(tv_band)

