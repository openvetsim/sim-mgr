#!/bin/bash
#
# This file is part of the sim-mgr distribution (https://github.com/OpenVetSimDevelopers/sim-mgr).
# 
# Copyright (c) 2019 VetSim, Cornell University College of Veterinary Medicine Ithaca, NY
# 
# This program is free software: you can redistribute it and/or modify  
# it under the terms of the GNU General Public License as published by  
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License 
# along with this program. If not, see <http://www.gnu.org/licenses/>.

export DISPLAY=:0.0
read WHICH  <<< $(which obs)
len=${#WHICH}
if [ $len -eq 0 ]; then
	exit
fi

read OBSPID  <<< $(pidof -s obs)
OBSWIN="$(xdotool search --all --onlyvisible --pid $OBSPID --name OBS )"

len=${#OBSWIN}
if [ $len -ne 0 ]; then
	export DISPLAY=:0.0
	xdotool windowactivate $OBSWIN
	xdotool key --window $OBSWIN ctrl+A
	xdotool windowminimize  $OBSWIN
else
	echo "Failed to find OBS."
	exit -1
fi
