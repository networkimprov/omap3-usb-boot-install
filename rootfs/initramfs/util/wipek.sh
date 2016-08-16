#!/bin/sh

#
# Try not to touch the external MMC card
#
if ls /dev/mmcblk1 > /dev/null 2>&1; then
	rootfs=/dev/mmcblk1p2
else
	rootfs=/dev/mmcblk0p2
fi

echo "Disabling usb gadget"
udc=$(cat /sys/kernel/config/usb_gadget/g1/UDC)
echo > /sys/kernel/config/usb_gadget/g1/UDC
sleep 1 # in case above not synchronous

if mount $rootfs /new_root; then
	echo "Removing kernel & dtb on $rootfs"
	rm -f /new_root/boot/{vmlinuz-linux-anvl,dtbs/anvl.dtb}

	kver=$(uname -r)
	if [ ! -d /new_root/lib/modules/$kver ]; then
		if [ -d /new_root/lib/modules/factory/$kver ]; then
			ln -s factory/$kver /new_root/lib/modules/$kver
		else
			echo "Modules for $kver not found"
		fi
	fi

	umount /new_root
else
	echo "Could not mount $rootfs"
fi

echo "Enabling usb gadget"
echo "$udc" > /sys/kernel/config/usb_gadget/g1
