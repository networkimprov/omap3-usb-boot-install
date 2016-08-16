#!/usr/bin/bash

set -e

if [ ! "$1" ]; then
  echo "Indicate rootfs directory"
  exit 1
fi

DIR=$(dirname $0)
FS=$1

arch-chroot ${FS}/ pacman -Sy linux-anvl --needed --noconfirm --noprogressbar > /dev/null
OS=$(arch-chroot ${FS}/ pacman -Q linux-anvl | sed 's/linux-anvl \(.*\)/\1-ARCH/')

pushd ${FS}/usr/lib/modules > /dev/null
fid=(factory/*)
if [ "${fid[0]}" != "factory/${OS}" ]; then
  echo "Updating /usr/lib/modules/factory/"
  rm -rf ./factory
  fid=(./*)
  mkdir ./factory && cp -al ${fid[@]} ./factory/
  touch ../../../anvl-rootfs.flag
fi
popd > /dev/null

cp -p ${DIR}/init.sh ${FS}/init
cp -p ${DIR}/mkinitcpio.conf ${FS}/
cp -p ${DIR}/util/* ${FS}/

arch-chroot ${FS}/ mkinitcpio -c /mkinitcpio.conf -k $OS -g /var/tmp/anvl-ramfs.img

rm $(cd ${DIR}/util && for file in *; do echo ${FS}/$file; done)
rm ${FS}/mkinitcpio.conf
rm ${FS}/init

mv ${FS}/var/tmp/anvl-ramfs.img ${FS}/
lsinitcpio -v ${FS}/anvl-ramfs.img > ${DIR}/lsinitcpio.txt
