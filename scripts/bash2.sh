#!/bin/bash
RELEASE=`lsb_release -rs`

sudo apt-get install -y phpmyadmin apache2-utils

if [[ ${RELEASE} == "18.04" ]]; 
then
	## echo Running 18.04.
	sudo apt install php-dev libmcrypt-dev php-pear
	sudo pecl channel-update pecl.php.net
	sudo pecl install mcrypt-1.0.1
else
	## echo Not Running 18.04.
	sudo apt-get install -y php-mbstring php7.0 php-gettext
	sudo apt-get install -y php7.0-zip
	sudo phpenmod mcrypt
fi

sudo phpenmod mbstring
sudo ln -s /usr/share/phpmyadmin /var/www/html
sudo service apache2 restart

if [[ ${RELEASE} == "18.04" ]]; 
then
	echo "You must edit /etc/php/7.2/cli/php.ini and /etc/php/7.2/apache2/php.ini"
	echo "to add 'extension=mcrypt.so'"
fi