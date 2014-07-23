#!/usr/bin/bash

# exit immediately when a command returns a nonzero
set -e

die() {
  printf "${@}"
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

DIR="$(dirname ${0})"

mkdir "${DIR}/rootfs"

pacstrap -C pacman.conf -d "${DIR}/rootfs" base wpa_supplicant openssh \
  sqlite samba graphicsmagick xdelta3 xapian-core chrony base-devel \
  traceroute dialog \
  omap-idle nodejs nodejs-inotify pacmatic

# override standard pacman with pacman.conf modified for our repository
cp pacman.conf "${DIR}/rootfs/etc/"

cp configure-rootfs.sh "${DIR}/rootfs/"
arch-chroot "${DIR}/rootfs" /configure-rootfs.sh -p ${config_password} -H ${config_hostname}
rm "${DIR}/rootfs/configure-rootfs.sh"

pushd "${DIR}/rootfs"
tar cf "${DIR}/../rootfs.tar" *
popd
