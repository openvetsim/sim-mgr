#!/bin/bash
#echo obs_open
#whoami
#echo Calling export
export DISPLAY=:0.0
#echo calling obs
obs &

#echo calling xdotool

xdotool search --name --sync --onlyvisible "obs" windowfocus
STATUS=$?
if [ $STATUS -eq 0 ]; then
	#xdotool getactivewindow windowmove 10000 10000
	xdotool getactivewindow windowminimize
	STATUS=$?
	if [ $STATUS -eq 0 ]; then
		echo "Failed window move"
	fi
else
	echo "Failed to get window focus"
	exit $STATUS
fi
xdotool search --name --sync "Vet School Simulator" windowfocus
