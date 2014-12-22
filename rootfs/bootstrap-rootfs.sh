#!/usr/bin/bash

set -e

BOOTSTRAP_IMAGE="hd.img"
MOUNTPOINT="mnt"

# download rootfs from installation instructions
# http://archlinuxarm.org/platforms/armv7/ti/beagleboard
if [ ! -f 'ArchLinuxARM-armv7-latest.tar.gz' ]; then
  curl -O -L 'http://archlinuxarm.org/os/ArchLinuxARM-armv7-latest.tar.gz'
fi

# cleanup existing image
rm -f ${BOOTSTRAP_IMAGE}

# make the mountpoint if it doesn't exist
[ ! -d ${MOUNTPOINT} ] && mkdir ${MOUNTPOINT}

# unmount the mountpoint
if mountpoint ${MOUNTPOINT}; then
  sudo umount ${MOUNTPOINT}
fi

# allocate
fallocate -l 4G ${BOOTSTRAP_IMAGE}

# format
mkfs.ext4 ${BOOTSTRAP_IMAGE}

# mount
sudo mount ${BOOTSTRAP_IMAGE} ${MOUNTPOINT}

# extract rootfs
sudo bsdtar -x -C ${MOUNTPOINT} -f ArchLinuxARM-armv7-latest.tar.gz

# copy kernel
cp ${MOUNTPOINT}/boot/zImage .

# unmount
sudo umount ${MOUNTPOINT}

# run qemu
qemu-system-arm \
  -machine vexpress-a9 \
  -m 1024M \
  -curses \
  -net nic \
  -net user \
  -sd ${BOOTSTRAP_IMAGE} \
  -kernel zImage \
  -m 512M \
  -no-reboot \
  -append 'console=ttyAMA0,115200n8 rw root=/dev/mmcblk0p2 rootwait physmap.enabled=0 debug earlyprintk' \
  -serial stdio
