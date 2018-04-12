#!/bin/bash
if [[ ${XDG_VTNR} != 7 ]]; then
	echo This shell is intended to run on the GUI window only.
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
 
/usr/bin/firefox -private "http://localhost/sim-ii/vitals.php" &

xdotool search --name --sync "Vet School Simulator" windowfocus %1 key F11

# command to disable screen shutdown and dimming
gsettings set org.gnome.desktop.session idle-delay 0
gsettings set org.gnome.settings-daemon.plugins.power idle-dim false
