#!/bin/bash
# 

# If Vitals user does not exist, create it.
getent passwd vitals  > /dev/null
if [ $? -eq 0 ]; then
	echo "Updating Vitals user"
else
	echo "Creating Vitals user."
	sudo adduser vitals
	sudo usermod -a -G www-data vitals
	
	## Add call to vitals.sh in the .profile for vitals.
	echo -e "if [[ ${XDG_VTNR} == 7 ]];\n    then\n    $HOME/vitals.sh &\n    obsmon &\nfi\n" >> ~vitals/.profile
fi

sudo cp vitals.sh /home/vitals/
chown vitals:vitals /home/vitals/vitals.sh
sudo cp grabScreen.sh /home/vitals/
chown vitals:vitals /home/vitals/grabScreen.sh


