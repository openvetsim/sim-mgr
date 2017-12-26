#!/bin/bash

sudo apt-get install apache2
sudo apt-get install mysql-server
sudo apt-get install libapache2-mod-php7.0
sudo apt-get apache2 restart

php -r 'echo "\nPHP is running!! \n\n";'
