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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

#include "usb.h"

typedef struct tocentry {
	unsigned offset;
	unsigned length;
	unsigned flags;
	unsigned align;
	unsigned spare;
	char name[12];
} tocentry;

/* Begin by Rockefeller - Parse ASIC ID */
typedef struct id_block {
	uint8_t		subblock_id;
	uint8_t		subblock_size;
	uint8_t		fixed_value;
	uint8_t		device[2];
	uint8_t		ch_enabled;
	uint8_t		rom_version;
} id_block;

typedef struct omap3_asic_id {
	uint8_t		items;
	id_block	id_subblock;
	uint8_t		reserve_subblock_1[4];
	uint8_t		reserve_subblock_2[23];
	uint8_t		reserve_subblock_3[23];
	uint8_t		checksum_subblock[11];
} omap3_asic_id;

typedef struct omap4_asic_id {
	uint8_t		num_subblocks;
	id_block	id_subblock;
	uint8_t		reserve_subblock_1[4];
	uint8_t		reserve_subblock_2[23];
	uint8_t		reserve_subblock_3[35];
	uint8_t		checksum_subblock[11];
} omap4_asic_id;

typedef union omap_asic_id {
	omap3_asic_id	omap3;
	omap4_asic_id	omap4;
} omap_asic_id;
/* End by Rockefeller - Parse ASIC ID */

#define USE_TOC 0

/* Begin by Rockefeller - Parse ASIC ID */
#define VENDOR_ID_TI                0x0451
#define DEVICE_ID_34XX              0xD009
#define DEVICE_ID_36XX              0xD00E
#define DEVICE_ID_44XX              0xD00F
/* End by Rockefeller - Parse ASIC ID */

int usb_boot(usb_handle *usb,
	     void *data, unsigned sz, 
	     void *data2, unsigned sz2)
{
	uint32_t msg_boot = 0xF0030002;
/* Begin by Rockefeller - Parse ASIC ID */
	uint32_t msg_get_asic_id = 0xF0030003;
/* End by Rockefeller - Parse ASIC ID */
	uint32_t msg_size = sz;
/* Begin by Rockefeller - Parse ASIC ID */
	omap_asic_id	asic_id;
	int	count = 0;
/* End by Rockefeller - Parse ASIC ID */
	int ret = 0;

/* Begin by Rockefeller - Parse ASIC ID */
	memset(&asic_id, 0, sizeof(asic_id));

	switch (usb->pid)
	{
	case DEVICE_ID_44XX:	/* 0xD00F */
		fprintf(stderr,"sending get asic id message to target...\n");
		usb_write(usb, &msg_get_asic_id, sizeof(msg_get_asic_id));
		count = usb_read(usb, &(asic_id.omap4), sizeof(asic_id.omap4));
		usb->device = (asic_id.omap4.id_subblock.device[0] << 8) | asic_id.omap4.id_subblock.device[1];
		break;
	case DEVICE_ID_34XX:	/* 0xD009 */
	case DEVICE_ID_36XX:	/* 0xD00E */
		count = usb_read(usb, &(asic_id.omap3), sizeof(asic_id.omap3));
		usb->device = (asic_id.omap3.id_subblock.device[0] << 8) | asic_id.omap3.id_subblock.device[1];
		sleep(1);
		break;
	default:
		break;
	}

	/* dump asic id */
	{
		int i;
		char *p = (char *)&asic_id;

		for (i = 0; i < count; i++)
		{
			fprintf(stderr, "%02X", p[i]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "Device OMAP%04X found\n", usb->device);
/* End by Rockefeller - Parse ASIC ID */

	fprintf(stderr,"sending 2ndstage to target...\n");
	count = usb_write(usb, &msg_boot, sizeof(msg_boot));
	if (count != sizeof(msg_boot))
		return -1;

	count = usb_write(usb, &msg_size, sizeof(msg_size));
	if (count != sizeof(msg_size))
		return -1;

	count = usb_write(usb, data, sz);
	if (count != sz)
		return -1;

	if (data2) {
		char dieid[64];

		fprintf(stderr,"waiting for 2ndstage response...\n");
		usb_read(usb, &msg_size, sizeof(msg_size));
		if (msg_size != 0xaabbccdd) {
			fprintf(stderr,"unexpected 2ndstage response\n");
			return -1;
		}
		msg_size = sz2;
		fprintf(stderr,"sending image to target...\n");
		count = usb_write(usb, &msg_size, sizeof(msg_size));
		if (count != sizeof(msg_size))
			return -1;

		count = usb_write(usb, data2, sz2);
		if (count != sz2)
			return -1;

		count = usb_read(usb, dieid, sizeof(dieid));
		if (count != sizeof(dieid)) {
			fprintf(stderr, "could not get die ID\n");
			return -1;
		}

		fprintf(stderr, "dieid: %s\n", dieid);
	}
	
	return 0;
}

int match_omap4_bootloader(usb_ifc_info *ifc)
{
/* Begin by Rockefeller - Parse ASIC ID */
	if (ifc->dev_vendor != VENDOR_ID_TI)
		return -1;

	switch (ifc->dev_product)
	{
	case DEVICE_ID_34XX:	/* 0xD009 */
	case DEVICE_ID_36XX:	/* 0xD00E */
	case DEVICE_ID_44XX:	/* 0xD00F */
		//fprintf(stderr,"PID[0x%04X] found\n", ifc->dev_product);
		break;
	default:
		//fprintf(stderr,"PID[0x%04X] is not supported yet\n", ifc->dev_product);
		return -1;
	}
/* End by Rockefeller - Parse ASIC ID */

	return 0;
}

void *load_file(const char *file, unsigned *sz)
{
	void *data;
	struct stat s;
	int fd;
	
	fd = open(file, O_RDONLY);
	if (fd < 0)
		return 0;
	
	if (fstat(fd, &s))
		goto fail;
	
	data = malloc(s.st_size);
	if (!data)
		goto fail;
	
	if (read(fd, data, s.st_size) != s.st_size) {
		free(data);
		goto fail;
	}
	
	close(fd);
	*sz = s.st_size;
	return data;
	
fail:
	close(fd);
	return 0;
}

extern void _binary_out_aboot_bin_start;
extern void _binary_out_aboot_bin_end;
extern void _binary_out_omap3_aboot_bin_start;
extern void _binary_out_omap3_aboot_bin_end;

int main(int argc, char **argv)
{
	void *data, *data2;
	unsigned sz, sz2;
	usb_handle *usb;
	int once = 1;

	fprintf(stderr,"?\n");
	if (argc < 2) {
		fprintf(stderr,"usage: usbboot [ <2ndstage> ] <image>\n");
		return 0;
	}

	if (argc < 3) {
		fprintf(stderr,"using built-in 2ndstage.bin\n");
		/* Read 2ndstage.bin after pid is determined */
#if 0
		data = &_binary_out_aboot_bin_start;
		sz = &_binary_out_aboot_bin_end - &_binary_out_aboot_bin_start;
#endif
	} else {
		data = load_file(argv[1], &sz);
		if (data == 0) {
			fprintf(stderr,"cannot load '%s'\n", argv[1]);
			return -1;
		}
		argc--;
		argv++;
	}
	
	data2 = load_file(argv[1], &sz2);
	if (data2 == 0) {
		fprintf(stderr,"cannot load '%s'\n", argv[1]);
		return -1;
	}

	for (;;) {
		int res;

		usb = usb_open(match_omap4_bootloader);
		if (usb)
		{
			if (argc < 3)
			{
				switch (usb->pid)
				{
				case DEVICE_ID_44XX:	/* 0xD00F */
					data = &_binary_out_aboot_bin_start;
					sz = &_binary_out_aboot_bin_end - &_binary_out_aboot_bin_start;
					break;
				case DEVICE_ID_34XX:	/* 0xD009 */
				case DEVICE_ID_36XX:	/* 0xD00E */
					data = &_binary_out_omap3_aboot_bin_start;
					sz = &_binary_out_omap3_aboot_bin_end - &_binary_out_omap3_aboot_bin_start;
					break;
				default:
					fprintf(stderr, "Un-supported PID=0x%04X\n", usb->pid);
					return -1;
				}
			}

			res = usb_boot(usb, data, sz, data2, sz2);
			if (res) {
				fprintf(stderr,
					"error %i, please replug the device without a battery\n",
					res);
			} else {
				return res;
			}
		}
		if (once) {
			once = 0;
			fprintf(stderr,"waiting for OMAP3/OMAP4 device...\n");
		}
		usleep(250);
	}
	
	return -1;    
}
