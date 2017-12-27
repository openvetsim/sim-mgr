#!/bin/bash

sudo apt-get install -y phpmyadmin apache2-utils
sudo apt-get install -y php-mbstring php7.0 php-gettext

sudo phpenmod mcrypt
sudo phpenmod mbstring
sudo ln -s /usr/share/phpmyadmin /var/www/html
sudo service apache2 restart
