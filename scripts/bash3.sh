#!/bin/bash
RELEASE=`lsb_release -rs`

sudo apt-get -y install g++
# wget ftp://ftp.gnu.org/gnu/cgicc/cgicc-3.2.19.tar.gz
# tar xvf cgicc-3.2.19.tar.gz
# sudo mv cgicc /usr/include
if [[ ${RELEASE} == "18.04" ]]; 
then
	sudo apt-get install -y libcgicc-dev
else
	sudo apt-get install -y build-essential libcgicc5v5 libcgicc5-dev libcgicc-doc
fi

sudo apt-get install -y libxml2 libxml2-dev

## Should add check here and skip if /var/lib/cgi-bin already exists
sudo mkdir /var/lib/cgi-bin
sudo chown www-data:www-data /var/lib/cgi-bin
sudo chmod 2777 /var/lib/cgi-bin

## Should add check here and skip if user simmgr already exists
sudo adduser simmgr
sudo cp simmgr_sudo /etc/sudoers.d
sudo chown root:root /etc/sudoers.d/simmgr_sudo
sudo chmod 0440 /etc/sudoers.d/simmgr_sudo
