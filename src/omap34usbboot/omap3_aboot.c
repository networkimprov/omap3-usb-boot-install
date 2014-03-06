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
#include <omap3/mux.h>
#include <omap3/hw.h>
#include <omap3/rom_usb.h>

static unsigned MSG = 0xaabbccdd;

struct usb usb;

/* download u-boot.bin and optional other components */
#define DOWNLOAD_ADDR 0x80008000
#define OMAP34XX_ID_L4_IO_BASE	0x4830A200
#define OMAP34XX_ID_FIRST	(OMAP34XX_ID_L4_IO_BASE + 0x18)
#define OMAP34XX_ID_LAST	(OMAP34XX_ID_FIRST + 0xc)

void init_dieid(char *buf)
{
	unsigned long id = OMAP34XX_ID_LAST;

	while (id >= OMAP34XX_ID_FIRST) {
		snprintf(buf, 8 + 1, "%08x", readl(id));
		buf += 8;
		id -= 4;
	}
}

void aboot(void)
{
	unsigned n;
	unsigned len;
	char dieid[64];

	mux_config();
	sdelay(100);

#if 0
	prcm_init();
#else
	enable_uart_clocks();
#endif
  	ddr_init();
	gpmc_init();

	serial_init();
	printf("\n[ aboot second-stage loader ]\n\n");

	if (usb_open(&usb))
		goto fail;

	usb_write(&usb, &MSG, sizeof(MSG));

	if (usb_read(&usb, &len, sizeof(len)))
		goto fail;

	if (usb_read(&usb, (void*) DOWNLOAD_ADDR, len))
		goto fail;

	init_dieid(dieid);
	printf("dieid: %s\n", dieid);
	if (usb_write(&usb, dieid, sizeof(dieid)))
		goto fail;

	usb_close(&usb);
	printf("booting....\n");

	{
		void (*entry)(unsigned, unsigned, unsigned) = (void*) DOWNLOAD_ADDR;
		entry(0, 2791, 0x80000100);
		for (;;);
	}

fail:
	printf("io error\n");
	for (;;) ;
}
