#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

if [[ ${UID} -ne 0 ]]; then
    echo "${0} must be run as root"
    exit 1
fi

mkdir rootfs

pacstrap -C pacman.conf -d rootfs base wpa_supplicant openssh omap-idle

# override standard pacman with pacman.conf modified for our repository
cp pacman.conf rootfs/etc/pacman.conf

cp configure-rootfs.sh rootfs

arch-chroot rootfs /configure-rootfs.sh

pushd rootfs

tar cf ../rootfs.tar *

popd
