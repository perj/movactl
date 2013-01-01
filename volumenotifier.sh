#!/bin/sh
# Copyright (c) 2008 Pelle Johansson
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

MOVACMD="/usr/local/bin/movactl -s /var/run/movactl.stereo.sock listen volume power audio_source"
PSNCMD="nc 192.168.2.2 7910"

TVCTL="/usr/local/bin/movactl :tv"
STEREOCTL="/usr/local/bin/movactl :stereo"

is_sleeping() {
	idle=$(ioreg -r -k HIDIdleTime | awk '/HIDIdleTime/ { if (os == 0 || $4 < os) os = $4; } END { printf "%d", os / 1000000000; }')
	return $(expr $idle '<=' 600)
}

power_on() {
	if [ $(expr $(date +%H%M) '>' 730) = 1 ]; then
		$STEREOCTL power on
		$TVCTL power on
	fi
}

while true; do
	( $MOVACMD & $PSNCMD ; kill %1 ) | while read stat val; do
		eval ov=\$old_$stat
		ts=$(date +%s)
		if [ "$ov" != "$val" ] && [ "$val" != negotiating ]; then
			case $stat in
				volume)
					#(echo "Volume" ; echo "$val dB") | /usr/local/bin/growlnotify -n Movactl -d org.morth.per.movactl.volume --image /usr/local/share/Sound-icon.png
					$TVCTL v v $(expr $val + 72)
				;;
				power)
					echo $TVCTL power "$val" >&2
					$TVCTL power "$val"
				;;
				audio_source)
					if [ "$val" = tv ]; then
						$STEREOCTL volume value -32
					elif [ "$val" = dvd ] || [ "$val" = "vcr" ]; then
						$STEREOCTL volume value -37
					fi
				;;
				playstation)
					if [ "$val" = up ]; then
						if [ $(expr $ts '>=' "${ts_playstation:-0}" + 6) = 1 ]; then
							power_on
							$STEREOCTL s s vcr
							$TVCTL s s hdmi1
						else
							# Don't change saved val
							val=down
						fi
					elif [ "$old_audio_source" = "vcr1" ]; then
						if [ "$old_tv" = on ]; then
							$STEREOCTL s s tv
							$TVCTL s s hdmi1
						elif [ "$old_wii" = on ]; then
							$STEREOCTL s s dss
							$TVCTL s s component
						elif is_sleeping; then
							$STEREOCTL power off
						else
							$STEREOCTL s s dvd
							$TVCTL s s hdmi1
						fi
					fi
				;;
				tv)
					if [ "$val" = on ]; then
						power_on
						$STEREOCTL s s tv
						$TVCTL s s hdmi1
					elif [ "$old_audio_source" = "tv" ]; then
						if [ "$old_playstation" = on ]; then
							$STEREOCTL s s vcr
							$TVCTL s s hdmi1
						elif [ "$old_wii" = on ]; then
							$STEREOCTL s s dss
							$TVCTL s s component
						elif is_sleeping; then
							$STEREOCTL power off
						else
							$STEREOCTL s s dvd
							$TVCTL s s hdmi1
						fi
					fi
				;;
				wii)
					if [ "$val" = on ]; then
						power_on
						$STEREOCTL s s dss
						$TVCTL s s component
					elif [ "$old_audio_source" = "dss" ]; then
						if [ "$old_playstation" = on ]; then
							$STEREOCTL s s vcr
							$TVCTL s s hdmi1
						elif [ "$old_tv" = on ]; then
							$STEREOCTL s s tv
							$TVCTL s s hdmi1
						elif is_sleeping; then
							$STEREOCTL power off
						else
							$STEREOCTL s s dvd
							$TVCTL s s hdmi1
						fi
					fi
				;;
			esac
			eval old_$stat=$val
			eval ts_$stat=$ts
		fi
	done
	sleep 2
done
