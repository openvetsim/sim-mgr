#!/bin/bash
echo obs_open
whoami
echo Calling export
export DISPLAY=:0.0
echo calling obs
obs &
echo callong xdotool

xdotool search --name --sync --onlyvisible "obs" windowfocus
xdotool getactivewindow windowmove 10000 10000
