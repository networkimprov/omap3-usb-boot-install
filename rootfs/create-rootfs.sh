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
usage: ${0##*/} [option] rootfs-directory

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

if [ ! "${!OPTIND}" ]; then
    echo "Indicate rootfs directory"
    exit 1
fi

pacman -S --needed --noconfirm arch-install-scripts > /dev/null 2>&1

# get the absolute path to the script
DIR="$(dirname "$(realpath "${0}")")"
ROOTFS="${!OPTIND}"

if [ -d "${ROOTFS}" ]; then
    echo "Removing existing ${ROOTFS}"
    rm -rf "${ROOTFS}"
fi

mkdir -p "${ROOTFS}/var/cache/"{man,pacman/pkg}

echo "Copying cache"
ln /var/cache/pacman/pkg/* "${ROOTFS}/var/cache/pacman/pkg/"
ln /var/cache/man/{CACHEDIR.TAG,index.db} "${ROOTFS}/var/cache/man/"

pacstrap -C "${DIR}/pacman.conf" -d "${ROOTFS}" $(sed s/#.*$// "${DIR}/packages.txt")

# override standard pacman with pacman.conf modified for our repository
cp "${DIR}/pacman.conf" "${ROOTFS}/etc/"

# install alsa sound state
cp "${DIR}/asound.state" "${ROOTFS}/var/lib/alsa/"

cp "${DIR}/smb.conf" "${ROOTFS}/etc/samba/"

# overwrite default watchdog config
cp "${DIR}/watchdog.conf" "${ROOTFS}/etc/"

cp "${DIR}/locale.conf" "${ROOTFS}/etc/"

cp "${DIR}/configure-rootfs.sh" "${ROOTFS}/"
arch-chroot "${ROOTFS}" /configure-rootfs.sh -p "${config_password}" -H "${config_hostname}" 
rm "${ROOTFS}/configure-rootfs.sh"

echo "Clearing cache..."
rm /var/cache/pacman/pkg/*
mv "${ROOTFS}/var/cache/pacman/pkg/"* /var/cache/pacman/pkg/

#echo "Tar'ing ${ROOTFS}"
#( cd "${ROOTFS}" && tar cf "${DIR}/anvl-rootfs.tar" ./* )

"${DIR}/initramfs/create-initramfs.sh" "${ROOTFS}"

