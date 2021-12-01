#!/bin/bash
#
# This file is part of the sim-mgr distribution (https://github.com/OpenVetSimDevelopers/sim-mgr).
# 
# Copyright (c) 2019-2020 VetSim, Cornell University College of Veterinary Medicine Ithaca, NY
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

if [[ ${WINDOWPATH} == "2" ]];
    then 
    echo "Detected Main Screen on 20.04";
elif [[ ${XDG_VTNR} == 1 ]]; 
    then
    echo "Detected Main Screen on 18.04";
else
    echo This shell is intended to run on the GUI window only.
	echo If you get this message in error, you may need to edit the ~vitals/vitals.sh and ~vitals/.profile files.
	echo env Variables:
    echo WINDOWPATH $WINDOWPATH
    echo XDG_VTNR $XDG_VTNR
    exit;
fi

## Loop until the local HTTP server is responding
URL='http://localhost/'
URL_FOUND=0
until [ $URL_FOUND -ge 1 ]; do
	URL_FOUND=`wget -O /dev/null $URL 2>&1|egrep "200 OK"|wc -l`
	echo Found is $URL_FOUND
	sleep 1;
done

# Run this script in display 0 - the monitor
export DISPLAY=:0
 
# Hide the mouse from the display
unclutter &
sleep 10  ## This allows .profile to complete execution before firefox starts. Otherwise, we can't get to full screen

/usr/bin/firefox -private "http://localhost/sim-ii/vitals.php" &

xdotool search --name --sync "Patient Vitals Monitor" windowfocus %1 key F11

# command to disable screen shutdown and dimming
gsettings set org.gnome.desktop.session idle-delay 0
gsettings set org.gnome.settings-daemon.plugins.power idle-dim false
