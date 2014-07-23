#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

# enable logging in through serial
ln -s /lib/systemd/system/getty@service etc/systemd/system/getty.target.wants/getty@ttyGS0.service
echo "ttyGS0" >> etc/securetty
echo "ttyO2" >> etc/securetty

systemctl enable sshd

systemctl enable omap-idle
