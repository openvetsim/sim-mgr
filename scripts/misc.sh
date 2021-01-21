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

wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | sudo apt-key add -
sudo sh -c 'echo "deb http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google-chrome.list'
sudo apt update
sudo apt install -y google-chrome-stable

# sudo apt install -y vsftpd
sudo apt install -y openssh-server
sudo apt install -y xdotool
sudo apt install -y unclutter
sudo apt install -y tinc
sudo apt install -y net-tools

sudo add-apt-repository -y ppa:obsproject/obs-studio
sudo apt -y update
sudo apt install -y obs-studio

./updateVitals.sh

# add user vet to www-data group
sudo usermod -a -G www-data vet
sudo usermod -a -G www-data simmgr

# update permissions
sudo chown www-data:www-data /var/www
sudo chmod 0755 /var/www/html
sudo chown -R www-data:www-data /var/www/html

sudo -u www-data mkdir /var/www/html/simlogs
sudo -u www-data chown www-data:www-data /var/www/html/simlogs
sudo -u www-data chmod 2777 /var/www/html/simlogs
sudo -u www-data chmod g+s /var/www/html/simlogs

# create video directory for OBS
sudo -u www-data mkdir /var/www/html/simlogs/video
sudo -u www-data chmod g+sw /var/www/html/simlogs/video

# clone the sim-ii
cd /var/www/html
sudo -u www-data git clone https://github.com/openvetsim/sim-ii.git

cd /var/www/html
sudo -u www-data git clone https://github.com/openvetsim/scenarios.git

# clone sim-player
cd /var/www/html/
sudo -u www-data git clone https://github.com/openvetsim/sim-player.git

# update ownership for all files....again
#sudo chown -R www-data:www-data /var/www/html
sudo chmod -R g+w /var/www/html

# update permissions for scenarios
cd /var/www/html
sudo chmod -R 777 ./scenarios

