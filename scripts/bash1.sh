#!/bin/bash

sudo apt-get install -y apache2
sudo apt-get install mysql-server
#sudo apt-get install -y libapache2-mod-php7.0
sudo apt-get install -y php
sudo service apache2 restart

php -r 'echo "\n PHP is RUNNING!!!\n\n";'

