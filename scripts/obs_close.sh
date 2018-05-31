#!/bin/bash

export DISPLAY=:0.0
xdotool search --name --sync --onlyvisible "obs" windowclose
