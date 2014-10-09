#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

die() {
  printf "%s" "${@}"
  echo
  exit 1
}

usage() {
  cat <<EOF
usage: ${0##*/} [option]

  Options:
    -p password    Set the password.
                   Default is pocketanvl.
    -H hostname    Set the hostname.
                   Default is anvl.
    -h             Display this message.
EOF
}

config_password='pocketanvl'
config_hostname='anvl'

while getopts ':p:H:h' flag; do
  case ${flag} in
    p)
      config_password="${OPTARG}"
      ;;
    H)
      config_hostname="${OPTARG}"
      ;;
    h)
      usage
      exit 0
      ;;
    :)
      die '%s: option requires an argument: '\''%s'\' "${0##*/}" "${OPTARG}"
      ;;
    ?)
      die '%s: invalid option: '\''%s'\' "${0##*/}" "${OPTARG}"
      ;;
  esac
done

if [[ ${UID} -ne 0 ]]; then
    echo "${0} must be run as root"
    exit 1
fi

# get the absolute path to the script
DIR="$(dirname "$(realpath "${0}")")"
ROOTFS="/var/tmp/rootfs"

rm -rf "${ROOTFS}"
mkdir "${ROOTFS}"

mkdir -p "${ROOTFS}/var/cache/pacman/pkg"
cp /var/cache/pacman/pkg/* "${ROOTFS}/var/cache/pacman/pkg"

pacstrap -C "${DIR}/pacman.conf" -d "${ROOTFS}" base wpa_supplicant openssh \
  sqlite samba graphicsmagick xdelta3 xapian-core chrony base-devel \
  traceroute dialog sudo \
  omap-idle pacmatic alsa-utils udhcp \
  go git mercurial \
  sd8787_uapsta-fw linux-anvl watchdog anvl-usb

# override standard pacman with pacman.conf modified for our repository
cp "${DIR}/pacman.conf" "${ROOTFS}/etc/"

# install default configuration for netctl
# ethernet over usb
cp "${DIR}/ethernet-usb" "${ROOTFS}/etc/netctl/"

# install alsa sound state
cp "${DIR}/asound.state" "${ROOTFS}/var/lib/alsa/"

# install config for udhcp
cp "${DIR}/udhcpd.conf" "${ROOTFS}/etc/"

cp "${DIR}/wpa_supplicant.conf" "${ROOTFS}/etc/netctl/"

cp "${DIR}/enable_debug.sh" "${ROOTFS}/usr/bin/"

cp "${DIR}/wifi-p2p_init.sh" "${ROOTFS}/usr/bin/"

# overwrite default watchdog config
cp "${DIR}/watchdog.conf" "${ROOTFS}/etc/"

cp "${DIR}/configure-rootfs.sh" "${ROOTFS}/"
arch-chroot "${ROOTFS}" /configure-rootfs.sh -p "${config_password}" -H "${config_hostname}"
rm "${ROOTFS}/configure-rootfs.sh"

# clean up package cache
cp "${ROOTFS}/var/cache/pacman/pkg/"* /var/cache/pacman/pkg/
rm "${ROOTFS}/var/cache/pacman/pkg/"*

pushd "${ROOTFS}"
tar cf "${DIR}/rootfs-$(date +%Y-%m-%d-%H-%M).tar" ./*
popd

rm -rf "${ROOTFS}"
