#!/bin/bash

export DISPLAY=:0.0
obs &
xdotool search --name --sync --onlyvisible "obs" windowfocus
xdotool getactivewindow windowmove 10000 10000
