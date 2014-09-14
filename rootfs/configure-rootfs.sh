#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

while getopts ':p:H:' flag; do
  case ${flag} in
    p)
      config_password="${OPTARG}"
      ;;
    H)
      config_hostname="${OPTARG}"
      ;;
    :)
      die '%s: option requires an argument: '\''%s'\' "${0##*/}" "${OPTARG}"
      ;;
    ?)
      die '%s: invalid option: '\''%s'\' "${0##*/}" "${OPTARG}"
      ;;
  esac
done

# enable logging in through serial
systemctl enable getty@ttyGS0
echo "ttyGS0" >> etc/securetty
echo "ttyO2" >> etc/securetty

netctl enable ethernet-usb

systemctl enable sshd

systemctl enable omap-idle

systemctl enable chrony

# enable samba
systemctl enable samba
systemctl enable nmbd
systemctl enable smbd.socket

systemctl enable watchdog

# change root password
echo "root:${config_password}" | chpasswd

# change the hostname
echo ${config_hostname} > /etc/hostname
sed -i "s/^127.0.0.1.*/& ${config_hostname}/" /etc/hosts
