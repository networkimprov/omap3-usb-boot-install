Arch Linux ARM scripts to assemble Anvl rootfs and installer initramfs.


### Building

Build on armv7 hardware running Arch Linux.
Populates rootfs/<tree> and generates
rootfs/<tree>/anvl-ramfs.img, and
rootfs/initramfs/lsinitcpio.txt.
Running tar cf & xz is left to the user.

    # su
    # cd rootfs
    # ./create-rootfs.sh [-h] [-H hostname] [-p root_password] tree
    # exit


Build images by booting vexpress running Arch in Qemu.
Populates images/, and leaves images/boot/ directory for generated files
and rootfs/fs/ for tree.

    ./makeimg.sh

### Todo

Build u-boot scripts & images. Move src/omap34usbboot to separate repo.


