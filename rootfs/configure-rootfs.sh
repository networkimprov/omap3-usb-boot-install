#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

PASSWORD='pocketanvl'
HOSTNAME='anvl'

# enable logging in through serial
systemctl enable getty@ttyGS0
echo "ttyGS0" >> etc/securetty
echo "ttyO2" >> etc/securetty

systemctl enable sshd

systemctl enable omap-idle

systemctl enable chrony

# enable samba
systemctl enable samba
systemctl enable nmbd
systemctl enable smbd.socket

# change root password
echo "root:${PASSWORD}" | chpasswd

# change the hostname
echo ${HOSTNAME} > /etc/hostname
sed -i "s/^127.0.0.1.*/& ${HOSTNAME}/" /etc/hosts
