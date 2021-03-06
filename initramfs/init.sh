#!/bin/busybox ash

#
# Minimal busybox init script for an initramfs
#
# Loosely based on the script at:
# http://jootamam.net/howto-initramfs-image.htm
#

#
# Set up initial mounts
#
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev/
mkdir /dev/pts
mount -t devpts none /dev/pts

#
# Install busybox symlinks
#
/bin/busybox --install -s > /dev/null 2>&1
ln -s /bin/busybox /bin/tail
ln -s /bin/busybox /bin/killall
ln -s /bin/busybox /bin/find

echo "Starting init script with cmdline options: $@"

#
# Move modules to the right place
#
if [ -d /lib/modules/kernel ]; then
	mkdir -p /lib/modules/$(uname -r)/kernel
	mv /lib/modules/kernel/modules.* /lib/modules/$(uname -r)/
	mv /lib/modules/kernel/* /lib/modules/$(uname -r)/kernel/
	rmdir /lib/modules/kernel
	depmod -a
fi

#
# Function for initializing USB composite gadget
# Based on the sample configuration at:
# https://wiki.tizen.org/wiki/USB/Linux_USB_Layers/Configfs_Composite_Gadget/Usage_eq._to_g_multi.ko
#
start_usb() {
	vendor=0x1d6b
	product=0x0106
	file0=$1
	omap_dieid=$2

	echo "Starting USB gadgets..."

	modprobe bq24190_charger
	modprobe bq27x00_battery
	modprobe led-class
	modprobe ledtrig-default-on
	modprobe ledtrig-gpio
	modprobe ledtrig-heartbeat
	modprobe leds-pca963x
	modprobe phy-twl4030-usb
	modprobe usbcore
	modprobe musb_hdrc
	modprobe omap2430
	modprobe libcomposite
	modprobe u_serial
	modprobe usb_f_serial
	modprobe usb_f_mass_storage
	modprobe u_ether
	modprobe usb_f_acm
	modprobe usb_f_ecm
	modprobe usb_f_rndis

	mount -t configfs none /sys/kernel/config
	mkdir /sys/kernel/config/usb_gadget/g1
	old_pwd=$(pwd)
	cd /sys/kernel/config/usb_gadget/g1

	echo $product > idProduct
	echo $vendor > idVendor
	mkdir strings/0x409
	echo $omap_dieid > strings/0x409/serialnumber
	echo "Anvl" > strings/0x409/manufacturer
	echo "Multi Gadget" > strings/0x409/product

	mkdir configs/c.1
	echo 120 > configs/c.1/MaxPower
	mkdir configs/c.1/strings/0x409
	echo "Conf 1" > configs/c.1/strings/0x409/configuration

	mkdir configs/c.2
	echo 500 > configs/c.2/MaxPower
	mkdir configs/c.2/strings/0x409
	echo "Conf 2" > configs/c.2/strings/0x409/configuration

	mkdir functions/mass_storage.0
	echo $file0 > functions/mass_storage.0/lun.0/file

	mkdir functions/acm.0
	mkdir functions/ecm.0

	ln -s functions/acm.0 configs/c.1
	ln -s functions/mass_storage.0 configs/c.1

	ln -s functions/ecm.0 configs/c.2
	ln -s functions/acm.0 configs/c.2
	ln -s functions/mass_storage.0 configs/c.2

	echo musb-hdrc.0.auto > /sys/kernel/config/usb_gadget/g1/UDC
	cd $old_pwd
}

#
# Try not to touch the external MMC card
#
if ! ls /dev/mmcblk1 > /dev/null 2>&1; then
	echo "Older kernel? Assuming emmc is mmcblk0.."
	emmc=/dev/mmcblk0
	rootfs=/dev/mmcblk0p2
else
	emmc=/dev/mmcblk1
	rootfs=/dev/mmcblk1p2
fi

#
# Signal we're in install mode with the LEDs
#
blink_leds() {
	if [ ! -f /LED ]; then
		sleep 10
		return 1
	fi

	option=$(cat /LED)
	red="/sys/class/leds/pca963x:red/brightness"
	green="/sys/class/leds/pca963x:green/brightness"
	blue="/sys/class/leds/pca963x:blue/brightness"

	# In microseconds
	blink_rate=500000

	echo 0 > $red
	echo 0 > $green
	echo 0 > $blue

	case $option in
	red)
		echo 30 > $red
		usleep $blink_rate
		echo 0 > $red
		usleep $blink_rate
		;;
	green)
		echo 30 > $green
		usleep $blink_rate
		echo 0 > $green
		usleep $blink_rate
		;;
	blue)
		echo 30 > $blue
		usleep $blink_rate
		echo 0 > $blue
		usleep $blink_rate
		;;
	yellow)
		echo 30 > $red
		echo 30 > $green
		usleep $blink_rate
		echo 0 > $red
		echo 0 > $green
		usleep $blink_rate
		;;
	orange)
		echo 40 > $red
		echo 10 > $green
		usleep $blink_rate
		echo 0 > $red
		echo 0 > $green
		usleep $blink_rate
		;;
	error)
		echo 30 > /sys/class/leds/pca963x\:red/brightness
		usleep 100000
		echo 0 > /sys/class/leds/pca963x\:red/brightness
		usleep 100000
		;;
	*)
		sleep 10
		;;
	esac
}

#
# Run some sanity checks on hardware and loop blinking red LED
# in case of error
#
check_block_device() {
	if [ -b $1 ]; then
		return 0
	fi

	echo "error" > /LED
	while [ 1 ]; do
		blink_leds
	done
}

#
# Check the unique die ID so install knows which device to use
#
check_die_id() {
	for arg in $(cat /proc/cmdline); do
		if echo $arg | grep omap_dieid= > /dev/null; then
			omap_dieid=$(echo $arg | sed -e s/omap_dieid=//)
		fi
		shift
	done
}

if echo $@ | grep really_install > /dev/null 2>&1; then

	check_die_id
	start_usb $emmc $omap_dieid

	echo "Waiting in install mode, console at ttyACM..."
	/sbin/getty -n -l /bin/sh /dev/ttyGS0 115200 &
	/sbin/getty -n -l /bin/sh /dev/ttyO2 115200 &

	check_block_device $emmc

	echo "yellow" > /LED
	while [ 1 ]; do
		blink_leds
	done
elif echo $@ | grep recovery_install > /dev/null 2>&1; then
	echo "Mounting $rootfs for recovery..."
	if mount $rootfs /mnt; then

		echo "Removing potentially broken anvl kernel on rootfs..."
		rm -f /mnt/boot/vmlinuz-linux-anvl

		echo "Removing potentially broken anvl dtb on rootfs..."
		rm -f /mnt/boot/dtbs/anvl.dtb

		echo "Installing initramfs kernel modules to rootfs..."
		tar c /lib/modules | tar x -C /mnt/

		echo "Running depmod for kernel modules on rootfs..."
		busybox chroot /mnt depmod -a

		echo "Installing initramfs firmware to rootfs..."
		tar c /lib/firmware | tar x -C /mnt/

		echo "Unmounting rootfs $rootfs..."
		umount /mnt
	else
		echo "Could not mount rootfs"
	fi

	check_die_id
	start_usb $emmc $omap_dieid

	echo "Waiting in recovery install mode, console at ttyACM..."
	/sbin/getty -n -l /bin/sh /dev/ttyGS0 115200 &
	sleep 2
	echo "Recovery shell, power cycle system when installer is done"
	/bin/sh
else
	echo "Mounting $rootfs as new root..."
	if ! mount $rootfs /mnt; then
		echo "Could not mount rootfs, starting shell.."
		/bin/sh
	fi

	echo "Unmounting temporary file systems..."
	umount /dev/pts
	umount /proc
	umount /sys/kernel/config
	umount /sys
	umount /dev

	echo "Starting /sbin/init on $rootfs..."
	exec switch_root /mnt /sbin/init $@
fi
