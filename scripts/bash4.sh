#!/bin/bash

sudo cp ~/sim-mgr/000-default.conf /etc/apache2/sites-available/
sudo a2enmod cgi
sudo service apache2 restart

cd ~/sim-mgr
make
make install
sudo service simmgr start
