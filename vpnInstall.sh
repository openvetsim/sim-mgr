#!/bin/bash
#
# Install TINC and connect to vpn.newforce.us server
#

FILE="/tmp/out.$$"
GREP="/bin/grep"
#....
# Make sure only root can run our script
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

MACID=`cat /sys/class/net/eno1/address`
MACID2="${MACID//:}"
MACID3=`echo $MACID2 | tr /a-z/ /A-Z/`
HOSTNAME="SimMgr$MACID3"

# echo $HOSTNAME


echo "Installing NFVPN"
mkdir /etc/tinc/nfvpn
mkdir /etc/tinc/nfvpn/hosts
./vpnconf -c $HOSTNAME
yes "" | tincd -n nfvpn -K4096
./vpnconf -g $HOSTNAME

if [ $(grep -c '^nfvpn$' /etc/tinc/nets.boot ) -eq 0 ]
then
	cat <<EOT >> /etc/tinc/nets.boot
nfvpn
EOT
fi
