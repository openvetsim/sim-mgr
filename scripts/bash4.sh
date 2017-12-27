#!/bin/bash

sudo mv ~/sim-mgr/000-default.conf /etc/apache2/sites-available/
sudo a2enmod cgi
sudo service apache2 restart

cd ~/sim-mgr
sudo make
sudo make install
sudo service simmgr start

cd /var/www/html
sudo git clone https://github.com/tkelleher/sim-ii.git

cd /var/www/html
sudo git clone https://github.com/tkelleher/scenarios.git

sudo mkdir simlogs
sudo chown www-data:www-data simlogs
sudo chown 2777 simlogs

