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

nonroot=self

netctl enable ethernet-usb

systemctl enable sshd

systemctl enable chrony

# enable samba
systemctl enable smbd
systemctl enable nmbd

systemctl enable watchdog

systemctl enable anvl-getty
systemctl enable anvl-usb
systemctl enable omap-idle

systemctl enable fstrim.timer

# change root password
echo "root:${config_password}" | chpasswd

# change the hostname
echo "${config_hostname}" > /etc/hostname
sed -i "s/^127.0.0.1.*/& ${config_hostname}/" /etc/hosts

useradd -m -G wheel -s /bin/bash ${nonroot}
echo "${nonroot}:${config_password}" | chpasswd

# allow users in the wheel group to run sudo
sed 's/# %wheel ALL=(ALL) ALL/%wheel ALL=(ALL) ALL/' -i /etc/sudoers

sed -e 's/#HandlePowerKey=poweroff/HandlePowerKey=ignore/' -i /etc/systemd/logind.conf

##### User 'self' chaanges

# setup go workspace
mkdir /home/${nonroot}/gopath

cat >> /home/${nonroot}/.bashrc << 'EOF'
add_path() {
  if [[ ! "$PATH" =~ (^|:)"$1"(:|$) ]]; then
    export PATH="$PATH:$1"
  fi
}

export GOPATH="$HOME/gopath"
add_path "$HOME/gopath/bin"
EOF

GOPATH=/home/${nonroot}/gopath go get github.com/networkimprov/info-anvl

mkdir /home/${nonroot}/share

# fix permissions by setting the owner and group owner as self:self
chown -R self:self /home/${nonroot}/gopath /home/${nonroot}/.bashrc /home/${nonroot}/share
