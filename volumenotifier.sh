#!/bin/sh

while true; do
	first=1
	/usr/local/bin/morantz listen volume | while read vol val; do
		if [ "$first" = 1 ]; then
			first=0
		else
			(echo "Volume" ; echo "$val dB") | /usr/local/bin/growlnotify -n Morantz -d morantz.volume --image ~/Sound-icon.png
		fi
	done
	sleep 2
done
