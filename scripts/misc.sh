#!/bin/bash

wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | sudo apt-key add -
sudo sh -c 'echo "deb http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google-chrome.list'
sudo apt-get update
sudo apt-get install -y google-chrome-stable

sudo apt-get install -y vsftpd
sudo apt-get install -y openssh-server
sudo apt-get install -y xdotool
sudo apt-get install -y unclutter
sudo apt-get install -y tinc

sudo add-apt-repository -y ppa:obsproject/obs-studio
sudo apt-get -y update
sudo apt-get install -y obs-studio

./updateVitals.sh

# clone sim-player
cd /var/www/html/
git clone https://github.com/tkelleher/sim-player.git

# create video directory for OBS
mkdir /var/www/html/simlogs/video

# update permissions
chown www-data:www-data /var/www
chmod 0755 /var/www/html
chown -R www-data:www-data /var/www/html
chmod g+s /var/www/html/simlogs
chmod g+s /var/www/html/simlogs/video
