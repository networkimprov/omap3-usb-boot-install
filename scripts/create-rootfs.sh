#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

if [[ ${UID} -ne 0 ]]; then
    echo "${0} must be run as root"
    exit 1
fi

mkdir rootfs

pacstrap -di rootfs base wpa_supplicant openssh

pushd rootfs

# enable logging in through serial
ln -s /lib/systemd/system/getty@service etc/systemd/system/getty.target.wants/getty@ttyGS0.service
echo "ttyGS0" >> etc/securetty
echo "ttyGS0" >> etc/securetty

# enable ssh
ln -s /usr/lib/systemd/system/sshd.service etc/systemd/system/multi-user.target.wants/sshd.service

tar cf ../rootfs.tar *

popd
