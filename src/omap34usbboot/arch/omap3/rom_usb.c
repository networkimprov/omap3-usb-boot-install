/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <aboot/aboot.h>
#include <aboot/io.h>
#include <omap3/rom_usb.h>

#define DEVICE_TYPE_UART	0x10
#define DEVICE_TYPE_HS_USB	0x11
#define DEVICE_TYPE_SSI		0x12
#define DEVICE_TYPE_SSI_HANDSHAKE	0x13
#define DEVICE_TYPE_NOMOREDEVICE	0x14

#define STATUS_OKAY		0
#define STATUS_FAILED		1
#define STATUS_TIMEOUT		2
#define STATUS_BAD_PARAM	3
#define STATUS_WAITING		4
#define STATUS_NO_MEMORY	5
#define STATUS_INVALID_PTR	6

struct rom_driver {
	int (*init)(struct rom_handle *rh);
	int (*read)(struct rom_handle *rh);
	int (*write)(struct rom_handle *rh);
	int (*close)(void);
};

#define API(n) ( (void*) (*((u32 *) (n))) )

struct rom_driver	usb_rom_driver;
struct rom_handle boot;
u16 peripheral_init_options;

int usb_open(struct usb *usb)
{
	int n;

	memset(usb, 0, sizeof(*usb));
	memset(&boot, 0, sizeof(boot));

	boot.device_type = DEVICE_TYPE_HS_USB;
	peripheral_init_options = 0x0A;
	boot.options = &peripheral_init_options;

	usb_rom_driver.init = API(0x14324);
	usb_rom_driver.write = API(0x14328);
	usb_rom_driver.read = API(0x1432C);
	usb_rom_driver.close = API(0x14330);
	usb->io = &usb_rom_driver;

	memcpy(&(usb->dread), &boot, sizeof(boot));
	memcpy(&(usb->dwrite), &boot, sizeof(boot));
#if 0
	boot.status = -1;
	n = usb->io->init(&boot);
#endif

	return n;
}

void usb_queue_read(struct usb *usb, void *data, unsigned len)
{
	int n;
	usb->dread.data = data;
	usb->dread.length = len;
	usb->dread.status = -1;
	n = usb->io->read(&usb->dread);
	if (n)
		usb->dread.status = n;
}

int usb_wait_read(struct usb *usb)
{
	for (;;) {
		if (usb->dread.status == -1)
			continue;
		if (usb->dread.status == STATUS_WAITING)
			continue;
		return usb->dread.status;
	}
}

void usb_queue_write(struct usb *usb, void *data, unsigned len)
{
	int n;
	usb->dwrite.data = data;
	usb->dwrite.length = len;
	usb->dwrite.status = -1;
	n = usb->io->write(&usb->dwrite);
	if (n)
		usb->dwrite.status = n;
}

int usb_wait_write(struct usb *usb)
{
	for (;;) {
		if (usb->dwrite.status == -1)
			continue;
		if (usb->dwrite.status == STATUS_WAITING)
			continue;
		return usb->dwrite.status;
	}
}

#define USB_MAX_IO 65536
int usb_read(struct usb *usb, void *data, unsigned len)
{
	unsigned xfer;
	unsigned char *x = data;
	int n;
	while (len > 0) {
		xfer = (len > USB_MAX_IO) ? USB_MAX_IO : len;
		usb_queue_read(usb, x, xfer);
		n = usb_wait_read(usb);
		if (n)
			return n;
		x += xfer;
		len -= xfer;
	}
	return 0;
}

int usb_write(struct usb *usb, void *data, unsigned len)
{
	usb_queue_write(usb, data, len);
	return usb_wait_write(usb);
}

void usb_close(struct usb *usb)
{
	usb->io->close();
}
