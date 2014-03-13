/*
 * Driver: uart.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 13, 2014
 * Description: 
 * TODO
 */

#include <hw_types.h>
#include <hw_uart.h>
#include "uart.h"

void UARTModuleReset(unsigned int baseAdd) {
	/* Performing Software Reset of the module. */
	HWREG(baseAdd + UART_SYSC) |= (UART_SYSC_SOFTRESET);

	/* Wait until the process of Module Reset is complete. */
	while (!(HWREG(baseAdd + UART_SYSS) & UART_SYSS_RESETDONE)) {
		;
	}
}

unsigned int UARTFIFOConfig(unsigned int baseAdd, unsigned int fifoConfig) {
	unsigned int txGra = (fifoConfig & UART_FIFO_CONFIG_TXGRA) >> 26;
	unsigned int rxGra = (fifoConfig & UART_FIFO_CONFIG_RXGRA) >> 22;

	unsigned int txTrig = (fifoConfig & UART_FIFO_CONFIG_TXTRIG) >> 14;
	unsigned int rxTrig = (fifoConfig & UART_FIFO_CONFIG_RXTRIG) >> 6;

	unsigned int txClr = (fifoConfig & UART_FIFO_CONFIG_TXCLR) >> 5;
	unsigned int rxClr = (fifoConfig & UART_FIFO_CONFIG_RXCLR) >> 4;

	unsigned int dmaEnPath = (fifoConfig & UART_FIFO_CONFIG_DMAENPATH) >> 3;
	unsigned int dmaMode = (fifoConfig & UART_FIFO_CONFIG_DMAMODE);

	unsigned int enhanFnBitVal = 0;
	unsigned int tcrTlrBitVal = 0;
	unsigned int tlrValue = 0;
	unsigned int fcrValue = 0;

	/* Setting the EFR[4] bit to 1. */
	enhanFnBitVal = UARTEnhanFuncEnable(baseAdd);

	tcrTlrBitVal = UARTSubConfigTCRTLRModeEn(baseAdd);

	/* Enabling FIFO mode of operation. */
	fcrValue |= UART_FCR_FIFO_EN;

	/* Setting the Receiver FIFO trigger level. */
	if (UART_TRIG_LVL_GRANULARITY_1 != rxGra) {
		/* Clearing the RXTRIGGRANU1 bit in SCR. */
		HWREG(baseAdd + UART_SCR) &= ~(UART_SCR_RX_TRIG_GRANU1);

		/* Clearing the RX_FIFO_TRIG_DMA field of TLR register. */HWREG(baseAdd + UART_TLR) &=
				~(UART_TLR_RX_FIFO_TRIG_DMA);

		fcrValue &= ~(UART_FCR_RX_FIFO_TRIG);

		/*
		 ** Checking if 'rxTrig' matches with the RX Trigger level values
		 ** in FCR.
		 */
		if ((UART_FCR_RX_TRIG_LVL_8 == rxTrig)
				|| (UART_FCR_RX_TRIG_LVL_16 == rxTrig)
				|| (UART_FCR_RX_TRIG_LVL_56 == rxTrig)
				|| (UART_FCR_RX_TRIG_LVL_60 == rxTrig)) {
			fcrValue |= (rxTrig & UART_FCR_RX_FIFO_TRIG);
		} else {
			/* RX Trigger level will be a multiple of 4. */
			/* Programming the RX_FIFO_TRIG_DMA field of TLR register. */
			HWREG(baseAdd + UART_TLR) |= ((rxTrig
					<< UART_TLR_RX_FIFO_TRIG_DMA_SHIFT)
					& UART_TLR_RX_FIFO_TRIG_DMA);
		}
	} else {
		/* 'rxTrig' now has the 6-bit RX Trigger level value. */

		rxTrig &= 0x003F;

		/* Collecting the bits rxTrig[5:2]. */
		tlrValue = (rxTrig & 0x003C) >> 2;

		/* Collecting the bits rxTrig[1:0] and writing to 'fcrValue'. */
		fcrValue |= (rxTrig & 0x0003) << UART_FCR_RX_FIFO_TRIG_SHIFT;

		/* Setting the RXTRIGGRANU1 bit of SCR register. */HWREG(baseAdd + UART_SCR) |=
				UART_SCR_RX_TRIG_GRANU1;

		/* Programming the RX_FIFO_TRIG_DMA field of TLR register. */
		HWREG(baseAdd + UART_TLR) |= (tlrValue
				<< UART_TLR_RX_FIFO_TRIG_DMA_SHIFT);

	}

	/* Setting the Transmitter FIFO trigger level. */
	if (UART_TRIG_LVL_GRANULARITY_1 != txGra) {
		/* Clearing the TXTRIGGRANU1 bit in SCR. */
		HWREG(baseAdd + UART_SCR) &= ~(UART_SCR_TX_TRIG_GRANU1);

		/* Clearing the TX_FIFO_TRIG_DMA field of TLR register. */HWREG(baseAdd + UART_TLR) &=
				~(UART_TLR_TX_FIFO_TRIG_DMA);

		fcrValue &= ~(UART_FCR_TX_FIFO_TRIG);

		/*
		 ** Checking if 'txTrig' matches with the TX Trigger level values
		 ** in FCR.
		 */
		if ((UART_FCR_TX_TRIG_LVL_8 == (txTrig))
				|| (UART_FCR_TX_TRIG_LVL_16 == (txTrig))
				|| (UART_FCR_TX_TRIG_LVL_32 == (txTrig))
				|| (UART_FCR_TX_TRIG_LVL_56 == (txTrig))) {
			fcrValue |= (txTrig & UART_FCR_TX_FIFO_TRIG);
		} else {
			/* TX Trigger level will be a multiple of 4. */
			/* Programming the TX_FIFO_TRIG_DMA field of TLR register. */
			HWREG(baseAdd + UART_TLR) |= ((txTrig
					<< UART_TLR_TX_FIFO_TRIG_DMA_SHIFT)
					& UART_TLR_TX_FIFO_TRIG_DMA);
		}
	} else {
		/* 'txTrig' now has the 6-bit TX Trigger level value. */

		txTrig &= 0x003F;

		/* Collecting the bits txTrig[5:2]. */
		tlrValue = (txTrig & 0x003C) >> 2;

		/* Collecting the bits txTrig[1:0] and writing to 'fcrValue'. */
		fcrValue |= (txTrig & 0x0003) << UART_FCR_TX_FIFO_TRIG_SHIFT;

		/* Setting the TXTRIGGRANU1 bit of SCR register. */HWREG(baseAdd + UART_SCR) |=
				UART_SCR_TX_TRIG_GRANU1;

		/* Programming the TX_FIFO_TRIG_DMA field of TLR register. */
		HWREG(baseAdd + UART_TLR) |= (tlrValue
				<< UART_TLR_TX_FIFO_TRIG_DMA_SHIFT);
	}

	if (UART_DMA_EN_PATH_FCR == dmaEnPath) {
		/* Configuring the UART DMA Mode through FCR register. */
		HWREG(baseAdd + UART_SCR) &= ~(UART_SCR_DMA_MODE_CTL);

		dmaMode &= 0x1;

		/* Clearing the bit corresponding to the DMA_MODE in 'fcrValue'. */
		fcrValue &= ~(UART_FCR_DMA_MODE);

		/* Setting the DMA Mode of operation. */
		fcrValue |= (dmaMode << UART_FCR_DMA_MODE_SHIFT);
	} else {
		dmaMode &= 0x3;

		/* Configuring the UART DMA Mode through SCR register. */HWREG(baseAdd + UART_SCR) |=
				UART_SCR_DMA_MODE_CTL;

		/* Clearing the DMAMODE2 field in SCR. */HWREG(baseAdd + UART_SCR) &=
				~(UART_SCR_DMA_MODE_2);

		/* Programming the DMAMODE2 field in SCR. */
		HWREG(baseAdd + UART_SCR) |= (dmaMode << UART_SCR_DMA_MODE_2_SHIFT);
	}

	/* Programming the bits which clear the RX and TX FIFOs. */
	fcrValue |= (rxClr << UART_FCR_RX_FIFO_CLEAR_SHIFT);
	fcrValue |= (txClr << UART_FCR_TX_FIFO_CLEAR_SHIFT);

	/* Writing 'fcrValue' to the FIFO Control Register(FCR). */
	UARTFIFORegisterWrite(baseAdd, fcrValue);

	/* Restoring the value of TCRTLR bit in MCR. */
	UARTTCRTLRBitValRestore(baseAdd, tcrTlrBitVal);

	/* Restoring the value of EFR[4] to the original value. */
	UARTEnhanFuncBitValRestore(baseAdd, enhanFnBitVal);

	return fcrValue;
}
