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

###  script to add the OBS Websocket

sudo apt update
sudo apt install qt5-image-formats-plugins
sudo wget https://github.com/Palakis/obs-websocket/releases/download/4.9.0/obs-websocket_4.9.0-1_amd64.deb
sudo dpkg -i obs-websocket_4.9.0-1_amd64.deb
sudo apt -f install
