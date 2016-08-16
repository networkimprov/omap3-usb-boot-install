#!/usr/bin/bash

set -e

LATEST="ArchLinuxARM-armv7-latest.tar.gz"
DIR="$(dirname "$0")/images/boot"
ROOTFS="${DIR}/alarm.img"
MOUNT="${DIR}/mnt"
SCR="$(dirname "$DIR")"

case $1 in
rootfs)    OP=create-rootfs.sh ;;
initramfs) OP=initramfs/create-initramfs.sh ;;
scripts)   ;;
*)  echo "Options: scripts, initramfs (requires an existing rootfs), rootfs." >&2 ; exit 1 ;;
esac

for file in ${SCR}/../src/scripts/*.script; do
  name=$(basename ${file%.script})
  if [ "$name" = '*' ]; then break; fi
  mkimage -A arm -T script -C none -n "$name script" -d $file ${SCR}/${name}.scr > /dev/null
  scripts="${scripts}${name}.scr "
done
mkimage -A arm -T firmware -O u-boot -C none -a 0x80800000 -e 0x00000000 \
  -n 'U-Boot 2014.10-00004-g8fcd259 fo' -d ${SCR}/u-boot.bin ${SCR}/u-boot.img > /dev/null
scripts="${scripts}u-boot.img"

if [ "$1" = scripts ]; then
  echo "Wrote $scripts"
  exit 0
fi

if [ ! -f ${ROOTFS} ]; then

  test -d ${DIR} || mkdir ${DIR}
  test -d ${MOUNT} || mkdir ${MOUNT}
  mountpoint ${MOUNT} > /dev/null && sudo umount ${MOUNT}

  echo "Creating virtual disk..."

  qemu-img create ${ROOTFS}.tmp 4G
  mkfs.ext4 -q ${ROOTFS}.tmp
  sudo mount ${ROOTFS}.tmp ${MOUNT}

  # download rootfs from installation instructions
  # http://archlinuxarm.org/platforms/armv7/ti/beagleboard
  echo "Downloading alarm image..."
  curl -sL "http://archlinuxarm.org/os/${LATEST}" -o "${DIR}/${LATEST}"
  echo "Extracting alarm image..."
  sudo bsdtar -xpf "${DIR}/${LATEST}" -C ${MOUNT}
  rm "${DIR}/${LATEST}"

  cp ${MOUNT}/boot/{zImage,dtbs/vexpress-v2p-ca15-tc1.dtb} ${DIR}/

  sudo chmod a+rx ${MOUNT}/root

  sudo umount ${MOUNT}
  mv ${ROOTFS}.tmp ${ROOTFS}
fi

cat > ${DIR}/anvl-makeimg.service <<EOF
[Unit]
Wants=network-online.target
After=network-online.target
After=multi-user.target
[Service]
Type=oneshot
StandardOutput=tty
WorkingDirectory=/root/rootfs
ExecStart=/usr/bin/sleep 5
ExecStart=/usr/bin/pacman -Syu --noconfirm
ExecStart=/root/rootfs/${OP} ./fs
ExecStart=/usr/bin/echo ${OP} DONE
ExecStart=/usr/bin/rm -f /etc/systemd/system/multi-user.target.wants/anvl-makeimg.service
ExecStart=/bin/bash -c 'systemctl poweroff'
EOF

sudo mount ${ROOTFS} ${MOUNT}
if [ $OP = create-rootfs.sh -o -d ${MOUNT}/root/rootfs/fs ]; then
  sudo cp -a ${DIR}/../../rootfs/ ${MOUNT}/root/
  sudo cp ${DIR}/anvl-makeimg.service ${MOUNT}/etc/systemd/system/multi-user.target.wants/
  sudo umount ${MOUNT}
else
  sudo umount ${MOUNT}
  echo "Must create rootfs before initramfs" >&2
  exit 1
fi

# run qemu
QEMU_AUDIO_DRV=none \
qemu-system-arm \
  -machine vexpress-a15 \
  -m 512M \
  -nographic \
  -drive "if=sd,format=raw,file=${ROOTFS}" \
  -kernel "${DIR}/zImage" \
  -append 'console=ttyAMA0 rw root=/dev/mmcblk0 rootwait physmap.enabled=0' \
  -dtb "${DIR}/vexpress-v2p-ca15-tc1.dtb" \
  -no-reboot

echo

sudo mount ${ROOTFS} ${MOUNT}

echo "Wrote $scripts"

if [ -f ${MOUNT}/root/rootfs/fs/anvl-ramfs.img ]; then
  cp ${MOUNT}/root/rootfs/fs/boot/vmlinuz-linux-anvl ${DIR}/../zimage
  cp ${MOUNT}/root/rootfs/fs/boot/dtbs/anvl.dtb ${DIR}/../dtb
  sudo mv ${MOUNT}/root/rootfs/fs/anvl-ramfs.img ${DIR}/../initramfs
  sudo chown ${USER}: ${DIR}/../initramfs
  echo "Wrote initramfs & zImage & dtb"
fi

if [ -f ${MOUNT}/root/rootfs/fs/anvl-rootfs.flag ]; then
  echo "Compiling tarball..."
  sudo rm ${MOUNT}/root/rootfs/fs/anvl-rootfs.flag
  TAR="$(realpath ${DIR}/../anvl-rootfs-$(date +%Y%m%d:%H%M).tar.gz)"
  (cd ${MOUNT}/root/rootfs/fs && sudo tar czf ${TAR} ./*)
  sudo chown ${USER}: ${TAR}
  echo "Wrote ${TAR}"
fi

cp ${MOUNT}/boot/zImage ${DIR}/

sudo umount ${MOUNT}

