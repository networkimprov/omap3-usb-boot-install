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

systemctl enable sshd

systemctl enable chrony

# enable samba
systemctl enable smbd
systemctl enable nmbd

systemctl enable watchdog

systemctl enable netctl-auto@mlan0

systemctl enable mwifiex-p2p_wpa

systemctl enable anvl-gadget
#systemctl enable anvl-getty
systemctl enable omap-idle

echo "Disabling default timers:"
ls /usr/lib/systemd/system/*.target.wants/*.timer
rm /usr/lib/systemd/system/*.target.wants/*.timer

# change root password
echo "root:${config_password}" | chpasswd

# change the hostname
echo "${config_hostname}" > /etc/hostname
sed "s/^127.0.0.1.*/& ${config_hostname}/" -i /etc/hosts

# allow users in the wheel group to run sudo
sed 's/# %wheel ALL=(ALL) ALL/%wheel ALL=(ALL) ALL/' -i /etc/sudoers

sed 's/#HandlePowerKey=poweroff/HandlePowerKey=suspend/' -i /etc/systemd/logind.conf

# enable US locale
sed 's/^#en_US.UTF-8/en_US.UTF-8/' -i /etc/locale.gen
locale-gen

##### User 'self' chaanges

useradd -m -G wheel -s /bin/bash ${nonroot}
echo "${nonroot}:${config_password}" | chpasswd

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
chown -R "${nonroot}:${nonroot}" "/home/${nonroot}"
