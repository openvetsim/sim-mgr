#!/bin/bash

sudo apt-get install g++
# wget ftp://ftp.gnu.org/gnu/cgicc/cgicc-3.2.19.tar.gz
# tar xvf cgicc-3.2.19.tar.gz
# sudo mv cgicc /usr/include
sudo apt-get update
sudo apt-get install -y build-essential libcgicc5v5 libcgicc5-dev libcgicc-doc
sudo apt-get update

sudo apt-get install -y libxml2 libxml2-dev
sudo mkdir /var/lib/cgi-bin
sudo chown www-data:www-data /var/lib/cgi-bin
sudo chmod 2777 /var/lib/cgi-bin

sudo adduser simmgr
sudo cp simmgr_sudo /etc/sudoers.d
sudo chown root:root /etc/sudoers.d/simmgr_sudo
sudo chmod 0440 /etc/sudoers.d/simmgr_sudo
