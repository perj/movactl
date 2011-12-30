#!/bin/sh

PATH=/usr/local/bin:$PATH
export PATH

if [ -z "$MOVACTL" ]; then
	MOVACTL=movactl
fi

LOCKFILE=$HOME/.movactl.poweroff.lock

[ -e "$LOCKFILE" ] && exit
touch "$LOCKFILE"

if [ "`$MOVACTL :t status power`" = "power off" ]; then
	[ "`$MOVACTL :s status power`" = "power off" ] && exit
	[ "`$MOVACTL :s status audio_source`" = "audio_source dvd" ] || exit

	if [ "`$MOVACTL :s status digital_signal_format`" = "digital_signal_format none" ]; then
		$MOVACTL :s power off
	fi
elif [ "`$MOVACTL :t status source`" = "source dtv" ]; then
	[ "`$MOVACTL :s status audio_source`" = "audio_source tv" ] && exit

	$MOVACTL :s power on
	$MOVACTL :s source select tv
	$MOVACTL :s volume value -37
fi

rm "$LOCKFILE"
