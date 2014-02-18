/*
 * sdram.c
 *
 * Copyright(c) 2010 Texas Instruments.   All rights reserved.
 *
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 * Santosh Shilimkar <santosh.shilimkar@ti.com>
 * Aneesh V	<aneesh@ti.com>
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

#include <aboot/aboot.h>
#include <aboot/io.h>
#include <omap3/hw.h>

#define RESETDONE		(0x1 << 0)

struct sdrc_cs {
	u32 mcfg;		/* 0x80 || 0xB0 */
	u32 mr;			/* 0x84 || 0xB4 */
	u8 res1[0x4];
	u32 emr2;		/* 0x8C || 0xBC */
	u8 res2[0x14];
	u32 rfr_ctrl;		/* 0x84 || 0xD4 */
	u32 manual;		/* 0xA8 || 0xD8 */
	u8 res3[0x4];
};

struct sdrc_actim {
	u32 ctrla;		/* 0x9C || 0xC4 */
	u32 ctrlb;		/* 0xA0 || 0xC8 */
};

struct sdrc {
	u8 res1[0x10];
	u32 sysconfig;		/* 0x10 */
	u32 status;		/* 0x14 */
	u8 res2[0x28];
	u32 cs_cfg;		/* 0x40 */
	u32 sharing;		/* 0x44 */
	u8 res3[0x18];
	u32 dlla_ctrl;		/* 0x60 */
	u32 dlla_status;	/* 0x64 */
	u32 dllb_ctrl;		/* 0x68 */
	u32 dllb_status;	/* 0x6C */
	u32 power;		/* 0x70 */
	u8 res4[0xC];
	struct sdrc_cs cs[2];	/* 0x80 || 0xB0 */
};

#define DLLPHASE_90		(0x1 << 1)
#define LOADDLL			(0x1 << 2)
#define ENADLL			(0x1 << 3)
#define DLL_DELAY_MASK		0xFF00
#define DLL_NO_FILTER_MASK	((0x1 << 9) | (0x1 << 8))

#define PAGEPOLICY_HIGH		(0x1 << 0)
#define SRFRONRESET		(0x1 << 7)
#define PWDNEN			(0x1 << 2)
#define WAKEUPPROC		(0x1 << 26)

#define DDR_SDRAM		(0x1 << 0)
#define DEEPPD			(0x1 << 3)
#define B32NOT16		(0x1 << 4)
#define BANKALLOCATION		(0x2 << 6)
#define RAMSIZE_128		(0x40 << 8) /* RAM size in 2MB chunks */
/* Begin by Rockefeller - Add size definitions for 256/512/1024MB */
#define RAMSIZE_256		(0x80 << 8) /* RAM size in 2MB chunks */
#define RAMSIZE_512		(0x100 << 8) /* RAM size in 2MB chunks */
#define RAMSIZE_1024	(0x200 << 8) /* RAM size in 2MB chunks */
/* End by Rockefeller - Add size definitions for 256/512/1024MB */
#define ADDRMUXLEGACY		(0x1 << 19)
#define CASWIDTH_10BITS		(0x5 << 20)
#define RASWIDTH_13BITS		(0x2 << 24)
#define BURSTLENGTH4		(0x2 << 0)
#define CASL3			(0x3 << 4)
#define SDRC_ACTIM_CTRL0_BASE	(OMAP3XXX_SDRC_BASE + 0x9C)
#define SDRC_ACTIM_CTRL1_BASE	(OMAP3XXX_SDRC_BASE + 0xC4)
#define ARE_ARCV_1		(0x1 << 0)
#define ARCV			(0x4e2 << 8) /* Autorefresh count */
#define OMAP34XX_SDRC_CS0	0x80000000
#define OMAP34XX_SDRC_CS1	0xA0000000
#define CMD_NOP			0x0
#define CMD_PRECHARGE		0x1
#define CMD_AUTOREFRESH		0x2
#define CMD_ENTR_PWRDOWN	0x3
#define CMD_EXIT_PWRDOWN	0x4
#define CMD_ENTR_SRFRSH		0x5
#define CMD_CKE_HIGH		0x6
#define CMD_CKE_LOW		0x7
#define SOFTRESET		(0x1 << 1)
#define SMART_IDLE		(0x2 << 3)
#define REF_ON_IDLE		(0x1 << 6)

#define CS0		0x0
#define CS1		0x1 /* mirror CS1 regs appear offset 0x30 from CS0 */

/* DDR - I use Micron DDR */
#define CONFIG_OMAP3_MICRON_DDR		1

enum {
	STACKED = 0,
	IP_DDR = 1,
	COMBO_DDR = 2,
	IP_SDR = 3,
};

#define EARLY_INIT	1

/* Slower full frequency range default timings for x32 operation*/
#define SDRC_SHARING	0x00000100
#define SDRC_MR_0_SDR	0x00000031

#define DLL_OFFSET		0
#define DLL_WRITEDDRCLKX2DIS	1
#define DLL_ENADLL		1
#define DLL_LOCKDLL		0
#define DLL_DLLPHASE_72		0
#define DLL_DLLPHASE_90		1

/* rkw - need to find of 90/72 degree recommendation for speed like before */
#define SDP_SDRC_DLLAB_CTRL	((DLL_ENADLL << 3) | \
				(DLL_LOCKDLL << 2) | (DLL_DLLPHASE_90 << 1))

/* Infineon part of 3430SDP (165MHz optimized) 6.06ns
 *   ACTIMA
 *	TDAL = Twr/Tck + Trp/tck = 15/6 + 18/6 = 2.5 + 3 = 5.5 -> 6
 *	TDPL (Twr) = 15/6	= 2.5 -> 3
 *	TRRD = 12/6	= 2
 *	TRCD = 18/6	= 3
 *	TRP = 18/6	= 3
 *	TRAS = 42/6	= 7
 *	TRC = 60/6	= 10
 *	TRFC = 72/6	= 12
 *   ACTIMB
 *	TCKE = 2
 *	XSR = 120/6 = 20
 */
#define INFINEON_TDAL_165	6
#define INFINEON_TDPL_165	3
#define INFINEON_TRRD_165	2
#define INFINEON_TRCD_165	3
#define INFINEON_TRP_165	3
#define INFINEON_TRAS_165	7
#define INFINEON_TRC_165	10
#define INFINEON_TRFC_165	12
#define INFINEON_V_ACTIMA_165	((INFINEON_TRFC_165 << 27) |		\
		(INFINEON_TRC_165 << 22) | (INFINEON_TRAS_165 << 18) |	\
		(INFINEON_TRP_165 << 15) | (INFINEON_TRCD_165 << 12) |	\
		(INFINEON_TRRD_165 << 9) | (INFINEON_TDPL_165 << 6) |	\
		(INFINEON_TDAL_165))

#define INFINEON_TWTR_165	1
#define INFINEON_TCKE_165	2
#define INFINEON_TXP_165	2
#define INFINEON_XSR_165	20
#define INFINEON_V_ACTIMB_165	((INFINEON_TCKE_165 << 12) |		\
		(INFINEON_XSR_165 << 0) | (INFINEON_TXP_165 << 8) |	\
		(INFINEON_TWTR_165 << 16))

/* Micron part of 3430 EVM (165MHz optimized) 6.06ns
 * ACTIMA
 *	TDAL = Twr/Tck + Trp/tck= 15/6 + 18 /6 = 2.5 + 3 = 5.5 -> 6
 *	TDPL (Twr)	= 15/6	= 2.5 -> 3
 *	TRRD		= 12/6	= 2
 *	TRCD		= 18/6	= 3
 *	TRP		= 18/6	= 3
 *	TRAS		= 42/6	= 7
 *	TRC		= 60/6	= 10
 *	TRFC		= 125/6	= 21
 * ACTIMB
 *	TWTR		= 1
 *	TCKE		= 1
 *	TXSR		= 138/6	= 23
 *	TXP		= 25/6	= 4.1 ~5
 */
#define MICRON_TDAL_165		6
#define MICRON_TDPL_165		3
#define MICRON_TRRD_165		2
#define MICRON_TRCD_165		3
#define MICRON_TRP_165		3
#define MICRON_TRAS_165		7
#define MICRON_TRC_165		10
#define MICRON_TRFC_165		21
#define MICRON_V_ACTIMA_165 ((MICRON_TRFC_165 << 27) |			\
		(MICRON_TRC_165 << 22) | (MICRON_TRAS_165 << 18) |	\
		(MICRON_TRP_165 << 15) | (MICRON_TRCD_165 << 12) |	\
		(MICRON_TRRD_165 << 9) | (MICRON_TDPL_165 << 6) |	\
		(MICRON_TDAL_165))

#define MICRON_TWTR_165		1
#define MICRON_TCKE_165		1
#define MICRON_XSR_165		23
#define MICRON_TXP_165		5
#define MICRON_V_ACTIMB_165 ((MICRON_TCKE_165 << 12) |			\
		(MICRON_XSR_165 << 0) | (MICRON_TXP_165 << 8) |	\
		(MICRON_TWTR_165 << 16))

/*
 * NUMONYX part of IGEP v2 (165MHz optimized) 6.06ns
 *   ACTIMA
 *      TDAL = Twr/Tck + Trp/tck = 15/6 + 18/6 = 2.5 + 3 = 5.5 -> 6
 *      TDPL (Twr) = 15/6 = 2.5 -> 3
 *      TRRD = 12/6 = 2
 *      TRCD = 22.5/6 = 3.75 -> 4
 *      TRP  = 18/6 = 3
 *      TRAS = 42/6 = 7
 *      TRC  = 60/6 = 10
 *      TRFC = 140/6 = 23.3 -> 24
 *   ACTIMB
 *	TWTR = 2
 *	TCKE = 2
 *	TXSR = 200/6 =  33.3 -> 34
 *	TXP  = 1.0 + 1.1 = 2.1 -> 3
 */
#define NUMONYX_TDAL_165   6
#define NUMONYX_TDPL_165   3
#define NUMONYX_TRRD_165   2
#define NUMONYX_TRCD_165   4
#define NUMONYX_TRP_165    3
#define NUMONYX_TRAS_165   7
#define NUMONYX_TRC_165   10
#define NUMONYX_TRFC_165  24
#define NUMONYX_V_ACTIMA_165 ((NUMONYX_TRFC_165 << 27) | \
		(NUMONYX_TRC_165 << 22) | (NUMONYX_TRAS_165 << 18) | \
		(NUMONYX_TRP_165 << 15) | (NUMONYX_TRCD_165 << 12) | \
		(NUMONYX_TRRD_165 << 9) | (NUMONYX_TDPL_165 << 6) | \
		(NUMONYX_TDAL_165))

#define NUMONYX_TWTR_165   2
#define NUMONYX_TCKE_165   2
#define NUMONYX_TXP_165    3
#define NUMONYX_XSR_165    34
#define NUMONYX_V_ACTIMB_165 ((NUMONYX_TCKE_165 << 12) | \
		(NUMONYX_XSR_165 << 0) | (NUMONYX_TXP_165 << 8) | \
		(NUMONYX_TWTR_165 << 16))

#ifdef CONFIG_OMAP3_INFINEON_DDR
#define V_ACTIMA_165 INFINEON_V_ACTIMA_165
#define V_ACTIMB_165 INFINEON_V_ACTIMB_165
#endif
#ifdef CONFIG_OMAP3_MICRON_DDR
#define V_ACTIMA_165 MICRON_V_ACTIMA_165
#define V_ACTIMB_165 MICRON_V_ACTIMB_165
#endif
#ifdef CONFIG_OMAP3_NUMONYX_DDR
#define V_ACTIMA_165 NUMONYX_V_ACTIMA_165
#define V_ACTIMB_165 NUMONYX_V_ACTIMB_165
#endif

#if !defined(V_ACTIMA_165) || !defined(V_ACTIMB_165)
#error "Please choose the right DDR type in config header"
#endif

/*
 * GPMC settings -
 * Definitions is as per the following format
 * #define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. STNOR - Intel Strata Flash
 * x is GPMC config registers from 1 to 6 (there will be 6 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of
 * the same. if values are independent of the board, this definition will be
 * present in this file if values are dependent on the board, then this should
 * go into corresponding mem-boardName.h file
 *
 * Currently valid part Names are (PART):
 * STNOR - Intel Strata Flash
 * SMNAND - Samsung NAND
 * MPDB - H4 MPDB board
 * SBNOR - Sibley NOR
 * MNAND - Micron Large page x16 NAND
 * ONNAND - Samsung One NAND
 *
 * include/configs/file.h contains the defn - for all CS we are interested
 * #define OMAP34XX_GPMC_CSx PART
 * #define OMAP34XX_GPMC_CSx_SIZE Size
 * #define OMAP34XX_GPMC_CSx_MAP Map
 * Where:
 * x - CS number
 * PART - Part Name as defined above
 * SIZE - how big is the mapping to be
 *   GPMC_SIZE_128M - 0x8
 *   GPMC_SIZE_64M  - 0xC
 *   GPMC_SIZE_32M  - 0xE
 *   GPMC_SIZE_16M  - 0xF
 * MAP  - Map this CS to which address(GPMC address space)- Absolute address
 *   >>24 before being used.
 */
#define GPMC_SIZE_128M	0x8
#define GPMC_SIZE_64M	0xC
#define GPMC_SIZE_32M	0xE
#define GPMC_SIZE_16M	0xF

#define SMNAND_GPMC_CONFIG1	0x00000800
#define SMNAND_GPMC_CONFIG2	0x00141400
#define SMNAND_GPMC_CONFIG3	0x00141400
#define SMNAND_GPMC_CONFIG4	0x0F010F01
#define SMNAND_GPMC_CONFIG5	0x010C1414
#define SMNAND_GPMC_CONFIG6	0x1F0F0A80
#define SMNAND_GPMC_CONFIG7	0x00000C44

#define M_NAND_GPMC_CONFIG1	0x00001800
#define M_NAND_GPMC_CONFIG2	0x00141400
#define M_NAND_GPMC_CONFIG3	0x00141400
#define M_NAND_GPMC_CONFIG4	0x0F010F01
#define M_NAND_GPMC_CONFIG5	0x010C1414
#define M_NAND_GPMC_CONFIG6	0x1f0f0A80
#define M_NAND_GPMC_CONFIG7	0x00000C44

#define STNOR_GPMC_CONFIG1	0x3
#define STNOR_GPMC_CONFIG2	0x00151501
#define STNOR_GPMC_CONFIG3	0x00060602
#define STNOR_GPMC_CONFIG4	0x11091109
#define STNOR_GPMC_CONFIG5	0x01141F1F
#define STNOR_GPMC_CONFIG6	0x000004c4

#define SIBNOR_GPMC_CONFIG1	0x1200
#define SIBNOR_GPMC_CONFIG2	0x001f1f00
#define SIBNOR_GPMC_CONFIG3	0x00080802
#define SIBNOR_GPMC_CONFIG4	0x1C091C09
#define SIBNOR_GPMC_CONFIG5	0x01131F1F
#define SIBNOR_GPMC_CONFIG6	0x1F0F03C2

#define SDPV2_MPDB_GPMC_CONFIG1	0x00611200
#define SDPV2_MPDB_GPMC_CONFIG2	0x001F1F01
#define SDPV2_MPDB_GPMC_CONFIG3	0x00080803
#define SDPV2_MPDB_GPMC_CONFIG4	0x1D091D09
#define SDPV2_MPDB_GPMC_CONFIG5	0x041D1F1F
#define SDPV2_MPDB_GPMC_CONFIG6	0x1D0904C4

#define MPDB_GPMC_CONFIG1	0x00011000
#define MPDB_GPMC_CONFIG2	0x001f1f01
#define MPDB_GPMC_CONFIG3	0x00080803
#define MPDB_GPMC_CONFIG4	0x1c0b1c0a
#define MPDB_GPMC_CONFIG5	0x041f1F1F
#define MPDB_GPMC_CONFIG6	0x1F0F04C4

#define P2_GPMC_CONFIG1	0x0
#define P2_GPMC_CONFIG2	0x0
#define P2_GPMC_CONFIG3	0x0
#define P2_GPMC_CONFIG4	0x0
#define P2_GPMC_CONFIG5	0x0
#define P2_GPMC_CONFIG6	0x0

#define ONENAND_GPMC_CONFIG1	0x00001200
#define ONENAND_GPMC_CONFIG2	0x000F0F01
#define ONENAND_GPMC_CONFIG3	0x00030301
#define ONENAND_GPMC_CONFIG4	0x0F040F04
#define ONENAND_GPMC_CONFIG5	0x010F1010
#define ONENAND_GPMC_CONFIG6	0x1F060000

#define NET_GPMC_CONFIG1	0x00001000
#define NET_GPMC_CONFIG2	0x001e1e01
#define NET_GPMC_CONFIG3	0x00080300
#define NET_GPMC_CONFIG4	0x1c091c09
#define NET_GPMC_CONFIG5	0x04181f1f
#define NET_GPMC_CONFIG6	0x00000FCF
#define NET_GPMC_CONFIG7	0x00000f6c

/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS	8
/* max number of GPMC regs */
#define GPMC_MAX_REG	7

#define PISMO1_NOR	1
#define PISMO1_NAND	2
#define PISMO2_CS0	3
#define PISMO2_CS1	4
#define PISMO1_ONENAND	5
#define DBG_MPDB	6
#define PISMO2_NAND_CS0 7
#define PISMO2_NAND_CS1 8

/* make it readable for the gpmc_init */
#define PISMO1_NOR_BASE		FLASH_BASE
#define PISMO1_NAND_BASE	NAND_BASE
#define PISMO2_CS0_BASE		PISMO2_MAP1
#define PISMO1_ONEN_BASE	ONENAND_MAP
#define DBG_MPDB_BASE		DEBUG_BASE



void ddr_init(void)
{
	struct sdrc *sdrc_base = (struct sdrc *)OMAP3XXX_SDRC_BASE;
	struct sdrc_actim *sdrc_actim_base;
	int cs = 0;

	/* reset sdrc controller */
	writel(SOFTRESET, &sdrc_base->sysconfig);
	wait_on_value(RESETDONE, RESETDONE, &sdrc_base->status,
			12000000);
	writel(0, &sdrc_base->sysconfig);

	/* setup sdrc to ball mux */
	writel(SDRC_SHARING, &sdrc_base->sharing);

	/* Disable Power Down of CKE cuz of 1 CKE on combo part */
	writel(WAKEUPPROC | SRFRONRESET | PAGEPOLICY_HIGH,
			&sdrc_base->power);

	writel(ENADLL | DLLPHASE_90, &sdrc_base->dlla_ctrl);
	sdelay(0x20000);

	sdrc_actim_base = (struct sdrc_actim *)SDRC_ACTIM_CTRL0_BASE;
#if 1
	/* DDR size = 128MB */
	writel(RASWIDTH_13BITS | CASWIDTH_10BITS | ADDRMUXLEGACY |
			RAMSIZE_128 | BANKALLOCATION | B32NOT16 | B32NOT16 |
			DEEPPD | DDR_SDRAM, &sdrc_base->cs[cs].mcfg);
#else
	/* DDR size = 256MB */
	writel(RASWIDTH_13BITS | CASWIDTH_10BITS | ADDRMUXLEGACY |
			RAMSIZE_256 | BANKALLOCATION | B32NOT16 | B32NOT16 |
			DEEPPD | DDR_SDRAM, &sdrc_base->cs[cs].mcfg);
#endif

	writel(ARCV | ARE_ARCV_1, &sdrc_base->cs[cs].rfr_ctrl);
	writel(V_ACTIMA_165, &sdrc_actim_base->ctrla);
	writel(V_ACTIMB_165, &sdrc_actim_base->ctrlb);

	writel(CMD_NOP, &sdrc_base->cs[cs].manual);
	writel(CMD_PRECHARGE, &sdrc_base->cs[cs].manual);
	writel(CMD_AUTOREFRESH, &sdrc_base->cs[cs].manual);
	writel(CMD_AUTOREFRESH, &sdrc_base->cs[cs].manual);

	/*
	* CAS latency 3, Write Burst = Read Burst, Serial Mode,
	* Burst length = 4
	*/
	writel(CASL3 | BURSTLENGTH4, &sdrc_base->cs[cs].mr);
}
