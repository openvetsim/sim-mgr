#!/bin/bash
URL='http://localhost/'

while [[ $(wget -O /dev/null $URL 2>&1|egrep "400 OK"|wc -l) == "1" ]]; do
	echo "Waiting...";
    sleep 10;
done
echo "Starting in 10 seconds...";
sleep 10;

# Run this script in display 0 - the monitor
export DISPLAY=:0
 
# Hide the mouse from the display
unclutter &
 
# If Chromium crashes (usually due to rebooting), clear the crash flag so we don't have the annoying warning bar
if [ -f  /home/vitals/.config/chromium/Default/Preferences ]; then
	sed -i 's/"exited_cleanly":false/"exited_cleanly":true/' /home/vitals/.config/chromium/Default/Preferences
	sed -i 's/"exit_type":"Crashed"/"exit_type":"Normal"/' /home/vitals/.config/chromium/Default/Preferences
fi

# get screen dimensions
read RES_X RES_Y <<<$(xdpyinfo | awk -F'[ x]+' '/dimensions:/{print $3, $4}')
 
# Run Chromium and open tabs
#/usr/bin/chromium-browser --kiosk --window-position=0,0 --window-size="${RES_X},${RES_Y}" http://localhost/sim-ii/vitals.php &
/usr/bin/google-chrome --kiosk --window-size="${RES_X},${RES_Y}" --incognito http://localhost/sim-ii/vitals.php &
chromePid=$!

# Start the kiosk loop. This keystroke changes the Chromium tab
# To have just anti-idle, use this line instead:
# xdotool keydown ctrl; xdotool keyup ctrl;
# Otherwise, the ctrl+Tab is designed to switch tabs in Chrome
# 
# The "kill -0" checks that chrome is still running. The loop breaks if chrome is terminated
##
while kill -0 $chromePid 2> /dev/null; do
	xdotool keydown ctrl+Tab; xdotool keyup ctrl+Tab;
	sleep 15
done
