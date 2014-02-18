/*
 * cpu.h
 *
 * Copyright(c) 2010 Texas Instruments.   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Texas Instruments nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _OMAP3XXX_CPU_H
#define  _OMAP3XXX_CPU_H


/*
 * 3XXX specific Section
 */

/* Stuff on L3 Interconnect */

/* L3 Firewall */
#if 0
#define A_REQINFOPERM0			(SMX_APE_BASE + 0x05048)
#define A_READPERM0			(SMX_APE_BASE + 0x05050)
#define A_WRITEPERM0			(SMX_APE_BASE + 0x05058)
#endif

#define OMAP3XXX_SDRC_BASE              0x6D000000
#define OMAP3XXX_SMS_BASE               0x6C000000
#define SMX_APE_BASE			0x68000000
#define OMAP3XXX_GPMC_BASE		0x6E000000		/* differ from OMAP4 */
#if 0
#define OMAP3XXX_DMM_BASE		0x4E000000
#endif
#define OMAP3XXX_L4_IO_BASE		0x48000000		/* differ from OMAP4 */
#define OMAP3XXX_WAKEUP_L4_IO_BASE	0x48300000	/* differ from OMAP4 */
#define OMAP3XXX_L4_PER			0x49000000		/* differ from OMAP4 */

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */

/* CONTROL */
#define OMAP3XXX_CTRL_BASE		(OMAP3XXX_L4_IO_BASE+0x2000)

/* UART */
#define OMAP3XXX_UART1			(OMAP3XXX_L4_IO_BASE+0x6a000)
#define OMAP3XXX_UART2			(OMAP3XXX_L4_IO_BASE+0x6c000)
#define OMAP3XXX_UART3			(OMAP3XXX_L4_PER+0x20000)

/* General Purpose Timers */
#define OMAP3XXX_GPT1			0x48318000
#define OMAP3XXX_GPT2			0x49032000
#define OMAP3XXX_GPT3			0x49034000
#define OMAP3XXX_GPT4			0x49036000
#define OMAP3XXX_GPT5			0x49038000
#define OMAP3XXX_GPT6			0x4903A000
#define OMAP3XXX_GPT7			0x4903C000
#define OMAP3XXX_GPT8			0x4903E000
#define OMAP3XXX_GPT9			0x49040000
#define OMAP3XXX_GPT10			0x48086000
#define OMAP3XXX_GPT11			0x48088000
#define OMAP3XXX_GPT12			0x48304000

/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE			0x4830C000
#define WD2_BASE			0x48314000
#define WD3_BASE			0x49030000

/* 32KTIMER */
#define SYNC_32KTIMER_BASE		(0x48320000)
#define S32K_CR				(SYNC_32KTIMER_BASE+0x10)

/* OMAP3 GPIO registers */
#define OMAP3XXX_GPIO1_BASE		0x48310000
#define OMAP3XXX_GPIO2_BASE		0x49050000
#define OMAP3XXX_GPIO3_BASE		0x49052000
#define OMAP3XXX_GPIO4_BASE		0x49054000
#define OMAP3XXX_GPIO5_BASE		0x49056000
#define OMAP3XXX_GPIO6_BASE		0x49058000

/* device type */
#define DEVICE_MASK		(0x03 << 8)
#define TST_DEVICE		0x0
#define EMU_DEVICE		0x1
#define HS_DEVICE		0x2
#define GP_DEVICE		0x3

/* GPMC CS3/cs4/cs6 not avaliable */
#define GPMC_BASE		(OMAP3XXX_GPMC_BASE)
#define GPMC_SYSCONFIG		(OMAP3XXX_GPMC_BASE+0x10)
#define GPMC_IRQSTATUS		(OMAP3XXX_GPMC_BASE+0x18)
#define GPMC_IRQENABLE		(OMAP3XXX_GPMC_BASE+0x1C)
#define GPMC_TIMEOUT_CONTROL	(OMAP3XXX_GPMC_BASE+0x40)
#define GPMC_CONFIG		(OMAP3XXX_GPMC_BASE+0x50)
#define GPMC_STATUS		(OMAP3XXX_GPMC_BASE+0x54)

#define GPMC_CONFIG_CS0		(OMAP3XXX_GPMC_BASE+0x60)
#define GPMC_CONFIG_WIDTH	(0x30)

#define GPMC_CONFIG1		(0x00)
#define GPMC_CONFIG2		(0x04)
#define GPMC_CONFIG3		(0x08)
#define GPMC_CONFIG4		(0x0C)
#define GPMC_CONFIG5		(0x10)
#define GPMC_CONFIG6		(0x14)
#define GPMC_CONFIG7		(0x18)
#define GPMC_NAND_CMD		(0x1C)
#define GPMC_NAND_ADR		(0x20)
#define GPMC_NAND_DAT		(0x24)

#define GPMC_ECC_CONFIG		(0x1F4)
#define GPMC_ECC_CONTROL	(0x1F8)
#define GPMC_ECC_SIZE_CONFIG	(0x1FC)
#define GPMC_ECC1_RESULT	(0x200)
#define GPMC_ECC2_RESULT	(0x204)
#define GPMC_ECC3_RESULT	(0x208)
#define GPMC_ECC4_RESULT	(0x20C)
#define GPMC_ECC5_RESULT	(0x210)
#define GPMC_ECC6_RESULT	(0x214)
#define GPMC_ECC7_RESULT	(0x218)
#define GPMC_ECC8_RESULT	(0x21C)
#define GPMC_ECC9_RESULT	(0x220)

#define GPMC_PREFETCH_CONFIG1	(0x1e0)
#define GPMC_PREFETCH_CONFIG2	(0x1e4)
#define GPMC_PREFETCH_CONTROL	(0x1ec)
#define GPMC_PREFETCH_STATUS	(0x1f0)

/* GPMC Mapping */
# define FLASH_BASE		0x10000000  /* NOR flash (aligned to 256 Meg) */
# define FLASH_BASE_SDPV1	0x04000000  /* NOR flash (aligned to 64 Meg) */
# define FLASH_BASE_SDPV2	0x10000000  /* NOR flash (aligned to 256 Meg) */
# define DEBUG_BASE		0x08000000  /* debug board */
# define NAND_BASE		0x30000000  /* NAND addr (actual size small port)*/
# define PISMO2_BASE		0x18000000  /* PISMO2 CS1/2 */
# define ONENAND_MAP		0x20000000  /* OneNand addr (actual size small port */

/* SMS */
#define SMS_SYSCONFIG           (OMAP3XXX_SMS_BASE+0x10)
#define SMS_RG_ATT0             (OMAP3XXX_SMS_BASE+0x48)
#define SMS_CLASS_ARB0          (OMAP3XXX_SMS_BASE+0xD0)
#define BURSTCOMPLETE_GROUP7    (1<<31)

#define SDRC_CS_CFG		(OMAP3XXX_SDRC_BASE+0x40)
#define OMAP3XXX_SDRC_CS0	0x80000000
#define SDRC_POWER		(OMAP3XXX_SDRC_BASE+0x70)
#define SDRC_MCFG_0		(OMAP3XXX_SDRC_BASE+0x80)
#define SDRC_MR_0		(OMAP3XXX_SDRC_BASE+0x84)

/* timer regs offsets (32 bit regs) */
#define TIDR			0x0      /* r */
#define TIOCP_CFG		0x10     /* rw */
#define TISTAT			0x14     /* r */
#define TISR			0x18     /* rw */
#define TIER			0x1C     /* rw */
#define TWER			0x20     /* rw */
#define TCLR			0x24     /* rw */
#define TCRR			0x28     /* rw */
#define TLDR			0x2C     /* rw */
#define TTGR			0x30     /* rw */
#define TWPS			0x34     /* r */
#define TMAR			0x38     /* rw */
#define TCAR1			0x3c     /* r */
#define TSICR			0x40     /* rw */
#define TCAR2			0x44     /* r */
#define GPT_EN			0x03 /* enable sys_clk NO-prescale /1 */

/* Watchdog */
#define WWPS			0x34     /* r */
#define WSPR			0x48     /* rw */
#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

/* I2C base */
#define I2C_BASE1		(OMAP3XXX_L4_IO_BASE + 0x70000)
#define I2C_BASE2		(OMAP3XXX_L4_IO_BASE + 0x72000)
#define I2C_BASE3		(OMAP3XXX_L4_IO_BASE + 0x60000)

/* Peripheral base */
#define CM_FCLKEN1_CORE	(OMAP3XXX_L4_IO_BASE + 0x4A00)
#define CM_ICLKEN1_CORE	(OMAP3XXX_L4_IO_BASE + 0x4A10)

#define CM_FCLKEN_PER	(OMAP3XXX_L4_IO_BASE + 0x5000)
#define CM_ICLKEN_PER	(OMAP3XXX_L4_IO_BASE + 0x5010)
#endif
