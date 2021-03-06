
#include <aboot/aboot.h>
#include <aboot/io.h>
#include <omap3/mux.h>
#include <omap3/hw.h>

/* syslib.c */

void sr32(u32 addr, u32 start_bit, u32 num_bits, u32 value)
{
	u32 tmp, msk = 0;
	msk = 1 << num_bits;
	--msk;
	tmp = readl(addr) & ~(msk << start_bit);
	tmp |=  value << start_bit;
	writel(tmp, addr);
}

u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound)
{
	u32 i = 0, val;
	do {
		++i;
		val = readl(read_addr) & read_bit_mask;
		if (val == match_value)
			return (1);
		if (i == bound)
			return (0);
	} while (1);
}

void sdelay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

#define MUX_DEFAULT_OMAP3() \
	MV(CP(UART3_CTS_RCTX) , (PTU | IEN | M0))  /* uart3_tx */ \
	MV(CP(UART3_RTS_SD) , (M0))  /* uart3_rts_sd */ \
	MV(CP(UART3_RX_IRRX) , (IEN | M0))  /* uart3_rx */ \
	MV(CP(UART3_TX_IRTX) , (M0))  /* uart3_tx */

void mux_config(void)
{
	MUX_DEFAULT_OMAP3();
}

