#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

if [[ ${UID} -ne 0 ]]; then
    echo "${0} must be run as root"
    exit 1
fi

mkdir rootfs

# TODO add archlinux-anvl repository to pacman.conf

pacstrap -di rootfs base wpa_supplicant openssh omap-idle

pushd rootfs

# enable logging in through serial
ln -s /lib/systemd/system/getty@service etc/systemd/system/getty.target.wants/getty@ttyGS0.service
echo "ttyGS0" >> etc/securetty
echo "ttyO2" >> etc/securetty

# enable ssh
ln -s /usr/lib/systemd/system/sshd.service etc/systemd/system/multi-user.target.wants/sshd.service

# enable omap-idle
ln -s /etc/systemd/system/omap-idle.service etc/systemd/system/multi-user.target.wants/omap-idle.service

tar cf ../rootfs.tar *

popd
