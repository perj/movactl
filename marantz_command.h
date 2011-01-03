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

SIMPLE_COMMAND (power_toggle, "PWR", "0")
SIMPLE_COMMAND (power_off, "PWR", "1")
SIMPLE_COMMAND (power_on, "PWR", "2")
SIMPLE_COMMAND (power_global_off, "PWR", "3")

SIMPLE_COMMAND (audio_att_toggle, "ATT", "0")
SIMPLE_COMMAND (audio_att_off, "ATT", "1")
SIMPLE_COMMAND (audio_att_on, "ATT", "2")

SIMPLE_COMMAND (audio_mute_toggle, "AMT", "0")
SIMPLE_COMMAND (audio_mute_off, "AMT", "1")
SIMPLE_COMMAND (audio_mute_on, "AMT", "2")

SIMPLE_COMMAND (video_mute_toggle, "VMT", "0")
SIMPLE_COMMAND (video_mute_off, "VMT", "1")
SIMPLE_COMMAND (video_mute_on, "VMT", "2")

SIGNINT_COMMAND (volume_value, "VOL", "0")
SIMPLE_COMMAND (volume_up, "VOL", "1")
SIMPLE_COMMAND (volume_down, "VOL", "2")
SIMPLE_COMMAND (volume_up_fast, "VOL", "3")
SIMPLE_COMMAND (volume_down_fast, "VOL", "4")

SIGNINT_COMMAND (bass_value, "TOB", "0")
SIMPLE_COMMAND (bass_up, "TOB", "1")
SIMPLE_COMMAND (bass_down, "TOB", "2")

SIGNINT_COMMAND (treble_value, "TOT", "0")
SIMPLE_COMMAND (treble_up, "TOT", "1")
SIMPLE_COMMAND (treble_down, "TOT", "2")

SIMPLE_COMMAND (source_select_tv, "SRC", "1")
SIMPLE_COMMAND (source_select_dvd, "SRC", "2")
SIMPLE_COMMAND (source_select_vcr1, "SRC", "3")
SIMPLE_COMMAND (source_select_dss, "SRC", "5")
SIMPLE_COMMAND (source_select_aux1, "SRC", "9")
SIMPLE_COMMAND (source_select_aux2, "SRC", "A")
SIMPLE_COMMAND (source_select_cd, "SRC", "C")
SIMPLE_COMMAND (source_select_cdr, "SRC", "D")
SIMPLE_COMMAND (source_select_tape, "SRC", "E")
SIMPLE_COMMAND (source_select_tuner, "SRC", "F")
SIMPLE_COMMAND (source_select_fm, "SRC", "G")
SIMPLE_COMMAND (source_select_am, "SRC", "H")
SIMPLE_COMMAND (source_select_xm, "SRC", "J")

SIMPLE_COMMAND (multi_channel_toggle, "71C", "0")
SIMPLE_COMMAND (multi_channel_off, "71C", "1")
SIMPLE_COMMAND (multi_channel_on, "71C", "2")

SIMPLE_COMMAND (hdmi_audio_mode_enable, "HAM", "1")
SIMPLE_COMMAND (hdmi_audio_mode_through, "HAM", "2")

UINT_COMMAND (sleep_value, "SLP", "0", 3)
SIMPLE_COMMAND (sleep_off, "SLP", "1")

SIMPLE_COMMAND (menu_toggle, "MNU", "0")
SIMPLE_COMMAND (menu_off, "MNU", "1")
SIMPLE_COMMAND (menu_on, "MNU", "2")
SIMPLE_COMMAND (menu_enter, "MNU", "3")

SIMPLE_COMMAND (cursor_up, "CUR", "1")
SIMPLE_COMMAND (cursor_down, "CUR", "2")
SIMPLE_COMMAND (cursor_left, "CUR", "3")
SIMPLE_COMMAND (cursor_right, "CUR", "4")

SIMPLE_COMMAND (dc_trigger_1_off, "DCT", "11")
SIMPLE_COMMAND (dc_trigger_1_on, "DCT", "12")

SIMPLE_COMMAND (front_lock_key_off, "FKL", "1")
SIMPLE_COMMAND (front_lock_key_on, "FKL", "2")

SIMPLE_COMMAND (simple_setup_toggle, "SSU", "0")
SIMPLE_COMMAND (simple_setup_off, "SSU", "1")
SIMPLE_COMMAND (simple_setup_on, "SSU", "2")
SIMPLE_COMMAND (simple_setup_enter, "SSU", "3")

SIMPLE_COMMAND (surr_mode_auto, "SUR", "00")
SIMPLE_COMMAND (surr_mode_stereo, "SUR", "01")
SIMPLE_COMMAND (surr_mode_dolby, "SUR", "02")
SIMPLE_COMMAND (surr_mode_pl2xmovie, "SUR", "03")
SIMPLE_COMMAND (surr_mode_pl2movie, "SUR", "04")
SIMPLE_COMMAND (surr_mode_pl2xmusic, "SUR", "05")
SIMPLE_COMMAND (surr_mode_pl2music, "SUR", "06")
SIMPLE_COMMAND (surr_mode_pl2xgame, "SUR", "07")
SIMPLE_COMMAND (surr_mode_pl2game, "SUR", "08")
SIMPLE_COMMAND (surr_mode_dolby_prologic, "SUR", "09")
SIMPLE_COMMAND (surr_mode_ex_es, "SUR", "0A")
SIMPLE_COMMAND (surr_mode_virtual_61, "SUR", "0B")
SIMPLE_COMMAND (surr_mode_dts_es, "SUR", "0E")
SIMPLE_COMMAND (surr_mode_neo6_cinema, "SUR", "0F")
SIMPLE_COMMAND (surr_mode_neo6_music, "SUR", "0G")
SIMPLE_COMMAND (surr_mode_multi_channel_stereo, "SUR", "0H")
SIMPLE_COMMAND (surr_mode_cs2_cinema, "SUR", "0I")
SIMPLE_COMMAND (surr_mode_cs2_music, "SUR", "0J")
SIMPLE_COMMAND (surr_mode_cs2_mono, "SUR", "0K")
SIMPLE_COMMAND (surr_mode_virtual, "SUR", "0L")
SIMPLE_COMMAND (surr_mode_dts, "SUR", "0M")
SIMPLE_COMMAND (surr_mode_ddplus_pl2x_movie, "SUR", "0O")
SIMPLE_COMMAND (surr_mode_ddplus_pl2x_music, "SUR", "0P")
SIMPLE_COMMAND (surr_mode_source_direct, "SUR", "0T")
SIMPLE_COMMAND (surr_mode_pure_direct, "SUR", "0U")
SIMPLE_COMMAND (surr_mode_next, "SUR", "1")
SIMPLE_COMMAND (surr_mode_prev, "SUR", "2")

SIMPLE_COMMAND (test_tone_toggle, "TTO", "0")
SIMPLE_COMMAND (test_tone_off, "TTO", "1")
SIMPLE_COMMAND (test_tone_on, "TTO", "2")
SIMPLE_COMMAND (test_tone_next, "TTO", "3")
SIMPLE_COMMAND (test_tone_prev, "TTO", "4")

SIMPLE_COMMAND (night_mode_toggle, "NGT", "0")
SIMPLE_COMMAND (night_mode_off, "NGT", "1")
SIMPLE_COMMAND (night_mode_on, "NGT", "2")

SIMPLE_COMMAND (dolby_headphone_mode_bypass, "DHM", "0")
SIMPLE_COMMAND (dolby_headphone_mode_dh1, "DHM", "1")
SIMPLE_COMMAND (dolby_headphone_mode_dh1_pl2_movie, "DHM", "2")
SIMPLE_COMMAND (dolby_headphone_mode_dh1_pl2_music, "DHM", "3")

UINT_COMMAND (lip_sync_value, "LIP", "0", 3)
SIMPLE_COMMAND (lip_sync_up, "LIP", "1")
SIMPLE_COMMAND (lip_sync_down, "LIP", "2")

UINT_COMMAND (tuner_frequency_value, "TFQ", "0", 5)
SIMPLE_COMMAND (tuner_frequency_up, "TFQ", "1")
SIMPLE_COMMAND (tuner_frequency_down, "TFQ", "2")
SIMPLE_COMMAND (tuner_frequency_auto_up, "TFQ", "3")
SIMPLE_COMMAND (tuner_frequency_auto_down, "TFQ", "4")

UINT_COMMAND (tuner_preset_value, "TPR", "0", 2)
SIMPLE_COMMAND (tuner_preset_up, "TPR", "1")
SIMPLE_COMMAND (tuner_preset_down, "TPR", "2")
SIMPLE_COMMAND (tuner_preset_p_scan_start, "TPR", "3")
SIMPLE_COMMAND (tuner_preset_p_scan_stop, "TPR", "4")

SIMPLE_COMMAND (tuner_preset_info_toggle, "TPI", "0")
SIMPLE_COMMAND (tuner_preset_info_off, "TPI", "1")
SIMPLE_COMMAND (tuner_preset_info_on, "TPI", "2")

SIMPLE_COMMAND (tuner_mode_toggle, "TMD", "0")
SIMPLE_COMMAND (tuner_mode_mono, "TMD", "1")
SIMPLE_COMMAND (tuner_mode_auto, "TMD", "2")

SIMPLE_COMMAND (tuner_memo, "MEM", "0")
SIMPLE_COMMAND (clear, "CLR", "0")

SIMPLE_COMMAND (xm_category_toggle, "CAT", "0")
SIMPLE_COMMAND (xm_category_channel_up, "CAT", "1")
SIMPLE_COMMAND (xm_category_channel_down, "CAT", "2")
SIMPLE_COMMAND (xm_category_next, "CAT", "3")
SIMPLE_COMMAND (xm_category_prev, "CAT", "4")

SIMPLE_COMMAND (multiroom_power_toggle, "MPW", "0")
SIMPLE_COMMAND (multiroom_power_off, "MPW", "1")
SIMPLE_COMMAND (multiroom_power_on, "MPW", "2")

SIMPLE_COMMAND (multiroom_audio_mute_toggle, "MAM", "0")
SIMPLE_COMMAND (multiroom_audio_mute_off, "MAM", "1")
SIMPLE_COMMAND (multiroom_audio_mute_on, "MAM", "2")

SIGNINT_COMMAND (multiroom_volume_value, "MVL", "0")
SIMPLE_COMMAND (multiroom_volume_up, "MVL", "1")
SIMPLE_COMMAND (multiroom_volume_down, "MVL", "2")

SIMPLE_COMMAND (multiroom_volume_set_variable, "MVS", "1")
SIMPLE_COMMAND (multiroom_volume_set_fixed, "MVS", "2")

SIMPLE_COMMAND (multiroom_source_select_tv, "MSC", "1")
SIMPLE_COMMAND (multiroom_source_select_dvd, "MSC", "2")
SIMPLE_COMMAND (multiroom_source_select_vcr1, "MSC", "3")
SIMPLE_COMMAND (multiroom_source_select_dss, "MSC", "5")
SIMPLE_COMMAND (multiroom_source_select_aux1, "MSC", "9")
SIMPLE_COMMAND (multiroom_source_select_aux2, "MSC", "A")
SIMPLE_COMMAND (multiroom_source_select_cd, "MSC", "C")
SIMPLE_COMMAND (multiroom_source_select_cdr, "MSC", "D")
SIMPLE_COMMAND (multiroom_source_select_tape, "MSC", "E")
SIMPLE_COMMAND (multiroom_source_select_tuner, "MSC", "F")
SIMPLE_COMMAND (multiroom_source_select_fm, "MSC", "G")
SIMPLE_COMMAND (multiroom_source_select_am, "MSC", "H")
SIMPLE_COMMAND (multiroom_source_select_xm, "MSC", "J")

UINT_COMMAND (multiroom_sleep_value, "MSL", "0", 3)
SIMPLE_COMMAND (multiroom_sleep_off, "MSL", "1")

SIMPLE_COMMAND (multiroom_speaker_toggle, "MSP", "0")
SIMPLE_COMMAND (multiroom_speaker_off, "MSP", "1")
SIMPLE_COMMAND (multiroom_speaker_on, "MSP", "2")

SIGNINT_COMMAND (multiroom_speaker_volume_value, "MSV", "0")
SIMPLE_COMMAND (multiroom_speaker_volume_up, "MSV", "1")
SIMPLE_COMMAND (multiroom_speaker_volume_down, "MSV", "2")

SIMPLE_COMMAND (multiroom_speaker_volume_set_variable, "MSS", "1")
SIMPLE_COMMAND (multiroom_speaker_volume_set_fixed, "MSS", "2")

SIMPLE_COMMAND (multiroom_speaker_audio_mute_toggle, "MSM", "0")
SIMPLE_COMMAND (multiroom_speaker_audio_mute_off, "MSM", "1")
SIMPLE_COMMAND (multiroom_speaker_audio_mute_on, "MSM", "2")

UINT_COMMAND (multiroom_tuner_frequency_value, "MTF", "0", 5)
SIMPLE_COMMAND (multiroom_tuner_frequency_up, "MTF", "1")
SIMPLE_COMMAND (multiroom_tuner_frequency_down, "MTF", "2")
SIMPLE_COMMAND (multiroom_tuner_frequency_auto_up, "MTF", "3")
SIMPLE_COMMAND (multiroom_tuner_frequency_auto_down, "MTF", "4")

UINT_COMMAND (multiroom_tuner_preset_value, "MTP", "0", 2)
SIMPLE_COMMAND (multiroom_tuner_preset_up, "MTP", "1")
SIMPLE_COMMAND (multiroom_tuner_preset_down, "MTP", "2")
SIMPLE_COMMAND (multiroom_tuner_preset_scan_start, "MTP", "3")
SIMPLE_COMMAND (multiroom_tuner_preset_scan_stop, "MTP", "4")

SIMPLE_COMMAND (multiroom_tuner_mode_toggle, "MTM", "0")
SIMPLE_COMMAND (multiroom_tuner_mode_mono, "MTM", "1")
SIMPLE_COMMAND (multiroom_tuner_mode_auto, "MTM", "2")

