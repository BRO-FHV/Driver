/*
 * Driver: uart.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 13, 2014
 * Description: 
 * Implementation of UART
 */

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <soc_AM335x.h>
#include <hw_beaglebone.h>
#include <hw_uart.h>
#include <hw_types.h>
#include <basic.h>
#include <list/linkedlist.h>
#include "../interrupt/dr_interrupt.h"
#include "dr_uart.h"

// FIFO size
#define NUM_TX_BYTES_PER_TRANS    (56)

// used to signify the application to transmit data to UART TX FIFO
uint32_t txEmptyFlag = TRUE;

// init function forward declaration
extern void UartModuleReset(uint32_t baseAdd);
static void UartFIFODefaultConfigure(void);
static uint32_t UartFIFOConfigure(uint32_t baseAdd, uint32_t fifoConfig);
static uint32_t UartEnhanFuncEnable(uint32_t baseAdd);
static uint32_t UartRegConfigModeEnable(uint32_t baseAdd, uint32_t modeFlag);
static uint32_t UartSubConfigTCRTLRModeEn(uint32_t baseAdd);
static void UartFIFORegisterWrite(uint32_t baseAdd, uint32_t fcrValue);
static uint32_t UartDivisorLatchWrite(uint32_t baseAddr, uint32_t divisorValue);
static void UartEnhanFuncBitValRestore(uint32_t baseAddr,
		uint32_t enhanFnBitVal);
static uint32_t UartOperatingModeSelect(uint32_t baseAddr, uint32_t modeFlag);
static void UartTCRTLRBitValRestore(uint32_t baseAddr, uint32_t tcrTlrBitVal);

// configure function forward declaration
static void UartBaudRateSet(uint32_t baseAddr, uint32_t baudRate);
static uint32_t UartDivisorValCompute(uint32_t moduleClk, uint32_t baudRate,
		uint32_t modeFlag, uint32_t mirOverSampRate);
static void UartLineCharacConfig(uint32_t baseAddr, uint32_t wLenStbFlag,
		uint32_t parityFlag);
static void UartDivisorLatchDisable(uint32_t baseAdd);
static void UartBreakCtl(uint32_t baseAdd, uint32_t breakState);

// write helper function
static uint32_t UartWriteChunk(uint32_t baseAddr);

// interrupt
uint32_t UartIntIdentityGet(uint32_t baseAdd);
void UartInterrupt(void);

typedef struct WriteChunk {
	uint32_t size;
	char* message;
} wChunk_t;

ll_t* chunkList;

/**
 * \brief Enable UART module identified by base address
 */
void UartEnable(uint32_t baseAddr) {
	// Enable Module
	Uart0ModuleClkConfig();

	// Select Uart0
	UartPinMuxSetup(0);

	// Performing a module reset
	UartModuleReset(SOC_UART_0_REGS);

	// crate chunklist
	chunkList = LinkedListCreate();
}

/**
 * \brief Configures UART module identified by base address, with baud rate
 */
void UartConfigure(uint32_t baseAddr, uint32_t baudRate) {
	// Performing FIFO configurations
	UartFIFODefaultConfigure();

	// set baud rate
	UartBaudRateSet(baseAddr, baudRate);

	// Switching to Configuration Mode B
	UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Programming the Line Characteristics
	UartLineCharacConfig(baseAddr,
			(UART_FRAME_WORD_LENGTH_8 | UART_FRAME_NUM_STB_1), UART_PARITY_NONE)
			;

	// Disabling write access to Divisor Latches
	UartDivisorLatchDisable(baseAddr);

	// Disabling Break Control
	UartBreakCtl(baseAddr, UART_BREAK_COND_DISABLE);

	// Switching to UART16x operating mode
	UartOperatingModeSelect(baseAddr, UART16x_OPER_MODE);
}

/**
 * \brief This function enables UART interrupt
 */
void UartSystemIntEnable(void) {
	// set uart interrrupt priority
	IntPrioritySet(SYS_INT_UART0INT, 0, AINTC_HOSTINT_ROUTE_IRQ);

	// register interrupt handler
	IntRegister(SYS_INT_UART0INT, UartInterrupt);

	// enable interrupt
	IntHandlerEnable(SYS_INT_UART0INT);
}

/**
 * \brief   This function enables the specified interrupts in the UART mode of
 *          operation
 *
 * \see uart_irda_cir.c::UARTIntEnable
 */
void UartIntEnable(uint32_t baseAddr, uint32_t intFlag) {
	uint32_t enhanFnBitVal = 0;
	uint32_t lcrRegValue = 0;

	// Switching to Register Configuration Mode B
	lcrRegValue = UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of EFR[4] and later setting it
	enhanFnBitVal = reg32r(baseAddr, UART_EFR) & UART_EFR_ENHANCED_EN;
	reg32m(baseAddr, UART_EFR, UART_EFR_ENHANCED_EN);

	// Switching to Register Operational Mode of operation
	UartRegConfigModeEnable(baseAddr, UART_REG_OPERATIONAL_MODE);

	/*
	 ** It is suggested that the System Interrupts for UART in the
	 ** Interrupt Controller are enabled after enabling the peripheral
	 ** interrupts of the UART using this API. If done otherwise, there
	 ** is a risk of LCR value not getting restored and illicit characters
	 ** transmitted or received from/to the UART. The situation is explained
	 ** below.
	 ** The scene is that the system interrupt for UART is already enabled and
	 ** the current API is invoked. On enabling the interrupts corresponding
	 ** to IER[7:4] bits below, if any of those interrupt conditions
	 ** already existed, there is a possibility that the control goes to
	 ** Interrupt Service Routine (ISR) without executing the remaining
	 ** statements in this API. Executing the remaining statements is
	 ** critical in that the LCR value is restored in them.
	 ** However, there seems to be no risk in this API for enabling interrupts
	 ** corresponding to IER[3:0] because it is done at the end and no
	 ** statements follow that.
	 */

	// Programming the bits IER[7:4]
	reg32m(baseAddr, UART_IER, (intFlag & 0xF0));

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Restoring the value of EFR[4] to its original value
	reg32a(baseAddr, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAddr, UART_EFR, enhanFnBitVal);

	// Restoring the value of LCR
	reg32w(baseAddr, UART_LCR, lcrRegValue);

	// Programming the bits IER[3:0]
	reg32m(baseAddr, UART_IER, (intFlag & 0x0F));
}

/**
 * \brief   This function disables the specified interrupts in the UART mode of
 *          operation
 *
 * \see uart_irda_cir.c::UARTIntDisable
 */
void UartIntDisable(uint32_t baseAddr, uint32_t intFlag) {
	uint32_t enhanFnBitVal = 0;
	uint32_t lcrRegValue = 0;

	// Switching to Register Configuration Mode B
	lcrRegValue = UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of EFR[4] and later setting it
	enhanFnBitVal = reg32r(baseAddr, UART_EFR) & UART_EFR_ENHANCED_EN;
	reg32m(baseAddr, UART_EFR, UART_EFR_ENHANCED_EN);

	// Switching to Register Operational Mode of operation
	UartRegConfigModeEnable(baseAddr, UART_REG_OPERATIONAL_MODE);
	reg32a(baseAddr, UART_IER, ~(intFlag & 0xFF));

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Restoring the value of EFR[4] to its original value
	reg32a(baseAddr, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAddr, UART_EFR, enhanFnBitVal);

	/* Restoring the value of LCR. */
	reg32w(baseAddr, UART_LCR, lcrRegValue);
}

/**
 * \brief sends message over uart module identified by base address
 *
 * \see uart_irda_cir.c::UARTFIFOWrite
 */
uint32_t UartWrite(uint32_t baseAddr, char *pBuffer, uint32_t numTxBytes) {
	uint32_t numByteChunks = numTxBytes / NUM_TX_BYTES_PER_TRANS;
	uint32_t remainBytes = numTxBytes % NUM_TX_BYTES_PER_TRANS;
	uint32_t bIndex = numByteChunks;
	uint32_t currNumTxBytes = 0;

	wChunk_t* chunk;

	while (bIndex > 0) {
		// create chunk
		chunk = (wChunk_t*) malloc(sizeof(wChunk_t));
		chunk->size = NUM_TX_BYTES_PER_TRANS;
		chunk->message = (char*) malloc(sizeof(NUM_TX_BYTES_PER_TRANS));
		strncat(chunk->message, pBuffer + currNumTxBytes,
				NUM_TX_BYTES_PER_TRANS);
		currNumTxBytes += NUM_TX_BYTES_PER_TRANS;

		// save chunk
		LinkedListAppendFront(chunkList, chunk);
		--bIndex;
	}

	// create chunk
	chunk = (wChunk_t*) malloc(sizeof(wChunk_t));
	chunk->size = remainBytes;
	chunk->message = (char*) malloc(remainBytes);
	strncat(chunk->message, pBuffer + currNumTxBytes, remainBytes);
	currNumTxBytes += remainBytes;

	// save chunk
	LinkedListAppendFront(chunkList, chunk);

	UartWriteChunk(baseAddr);

	return numTxBytes;
}

static uint32_t UartWriteChunk(uint32_t baseAddr) {
	uint32_t lIndex = 0;

	if (txEmptyFlag == TRUE) {
		// TODO get back!
		wChunk_t* chunk = (wChunk_t*) LinkedListGetFront(chunkList);
		if (chunk != NULL ) {
			char* pBuffer = chunk->message;

			for (lIndex = 0; lIndex < chunk->size; lIndex++) {
				// Writing data to the TX FIFO
				reg32w(baseAddr, UART_THR, *pBuffer++);
			}

			free(chunk->message);
			free(chunk);

			txEmptyFlag = FALSE;
			UartIntEnable(baseAddr, UART_INT_THR);
		}
	}

	return lIndex;
}

/**
 * \brief handles uart interrupt
 */
void UartInterrupt(void) {
	uint32_t intId = UartIntIdentityGet(SOC_UART_0_REGS);

	switch (intId) {
	case UART_INTID_TX_THRES_REACH:
		printf("UART_INTID_TX_THRES_REACH\n");
		// enable chunk
		txEmptyFlag = TRUE;

		// Disable the THR interrupt. This has to be done even if the
		UartIntDisable(SOC_UART_0_REGS, UART_INT_THR);

		// write a chunk
		uint32_t l = UartWriteChunk(SOC_UART_0_REGS);
		break;

	case UART_INTID_RX_THRES_REACH:
		printf("UART_INTID_RX_THRES_REACH\n");
		break;

	case UART_INTID_RX_LINE_STAT_ERROR:
		printf("UART_INTID_RX_LINE_STAT_ERROR\n");
		break;

	case UART_INTID_CHAR_TIMEOUT:
		printf("UART_INTID_CHAR_TIMEOUT\n");
		break;

	default:
		printf("DEFAULT\n");
		break;
	}
}

/**
 * \brief disables write access to Divisor Latch registers DLL and DLH
 *
 * \see uart_irda_cir.c::UARTDivisorLatchDisable
 */
static void UartDivisorLatchDisable(uint32_t baseAdd) {
	// Disabling access to Divisor Latch registers by clearing LCR[7] bit
	reg32a(baseAdd, UART_LCR, ~(UART_LCR_DIV_EN));
}

/**
 * \brief introduce or remove a Break condition
 *
 * \see uart_irda_cir.c::UARTBreakCtl
 */
static void UartBreakCtl(uint32_t baseAdd, uint32_t breakState) {
	// Clearing the BREAK_EN bit in LCR
	reg32a(baseAdd, UART_LCR, ~(UART_LCR_BREAK_EN));

	// Programming the BREAK_EN bit in LCR
	reg32m(baseAdd, UART_LCR, (breakState & UART_LCR_BREAK_EN));
}

/**
 * \brief configures the Line Characteristics for the UART instance. The Line Characteristics include:
 *           - Word length per frame
 *           - Number of Stop Bits per frame
 *           - Parity feature configuration
 *
 * \see uart_irda_cir.c::UARTLineCharacConfig
 */
static void UartLineCharacConfig(uint32_t baseAddr, uint32_t wLenStbFlag,
		uint32_t parityFlag) {
	// Clearing the CHAR_LENGTH and NB_STOP fields in LCR
	reg32a(baseAddr, UART_LCR, ~(UART_LCR_NB_STOP | UART_LCR_CHAR_LENGTH));

	// Programming the CHAR_LENGTH and NB_STOP fields in LCR
	reg32m(baseAddr, UART_LCR,
			(wLenStbFlag & (UART_LCR_NB_STOP | UART_LCR_CHAR_LENGTH)));

	// Clearing the PARITY_EN, PARITY_TYPE1 and PARITY_TYPE2 fields in LCR
	reg32a(baseAddr, UART_LCR,
			~(UART_LCR_PARITY_TYPE2 | UART_LCR_PARITY_TYPE1 | UART_LCR_PARITY_EN));

	// Programming the PARITY_EN, PARITY_TYPE1 and PARITY_TYPE2 fields in LCR
	reg32m(baseAddr, UART_LCR,
			(parityFlag
					& (UART_LCR_PARITY_TYPE2 | UART_LCR_PARITY_TYPE1
							| UART_LCR_PARITY_EN)));

}

/**
 * \brief set baudrate to uart module
 *
 * \see uart_irda_cir.c::UARTLineCharacConfig
 */
static void UartBaudRateSet(uint32_t baseAddr, uint32_t baudRate) {
	uint32_t divisorValue = 0;

	// Computing the Divisor Value
	divisorValue = UartDivisorValCompute(UART_MODULE_INPUT_CLK, baudRate,
			UART16x_OPER_MODE, UART_MIR_OVERSAMPLING_RATE_42);

	//Programming the Divisor Latches
	UartDivisorLatchWrite(baseAddr, divisorValue);
}

/**
 * \brief computes the divisor value for the specified operating mode. Not part of this API, the
 * 		  divisor value returned is written to the Divisor Latches to configure the Baud Rate
 *
 * \see uart_irda_cir.c::UARTDivisorValCompute
 */
static uint32_t UartDivisorValCompute(uint32_t moduleClk, uint32_t baudRate,
		uint32_t modeFlag, uint32_t mirOverSampRate) {
	uint32_t divisorValue = 0;

	modeFlag &= UART_MDR1_MODE_SELECT;

	switch (modeFlag) {
	case UART16x_OPER_MODE:
	case UART_SIR_OPER_MODE:
		divisorValue = (moduleClk) / (16 * baudRate);
		break;

	case UART13x_OPER_MODE:
		divisorValue = (moduleClk) / (13 * baudRate);
		break;

	case UART_MIR_OPER_MODE:
		divisorValue = (moduleClk) / (mirOverSampRate * baudRate);
		break;

	case UART_FIR_OPER_MODE:
		divisorValue = 0;
		break;

	default:
		break;
	}

	return divisorValue;
}

/**
 * \brief performs a module reset of the UART module. It also waits until the reset process is
 * 	      complete
 *
 * \see uart_irda_cir.c::UARTModuleReset
 */
void UartModuleReset(uint32_t baseAdd) {
	// Performing Software Reset of the module
	reg32m(baseAdd, UART_SYSC, UART_SYSC_SOFTRESET);

	// Wait until the process of Module Reset is complete
	wait(!(reg32r(baseAdd, UART_SYSS) & UART_SYSS_RESETDONE));
}

/**
 * \brief configures fifo with default values
 */
static void UartFIFODefaultConfigure(void) {
	uint32_t fifoConfig = 0;

	/*
	 ** - Transmit Trigger Level Granularity is 4
	 ** - Receiver Trigger Level Granularity is 1
	 ** - Transmit FIFO Space Setting is 56. Hence TX Trigger level
	 **   is 8 (64 - 56). The TX FIFO size is 64 bytes.
	 ** - The Receiver Trigger Level is 1.
	 ** - Clear the Transmit FIFO.
	 ** - Clear the Receiver FIFO.
	 ** - DMA Mode enabling shall happen through SCR register.
	 ** - DMA Mode 0 is enabled. DMA Mode 0 corresponds to No
	 **   DMA Mode. Effectively DMA Mode is disabled.
	 */
	fifoConfig = UART_FIFO_CONFIG(UART_TRIG_LVL_GRANULARITY_4,
			UART_TRIG_LVL_GRANULARITY_1,
			UART_FCR_TX_TRIG_LVL_56,
			1,
			1,
			1,
			UART_DMA_EN_PATH_SCR,
			UART_DMA_MODE_0_ENABLE);

	// Configuring the FIFO settings
	UartFIFOConfigure(SOC_UART_0_REGS, fifoConfig);
}

/**
 * configures the FIFO settings for the UART instance. Specifically, this does the following
 * configurations:
 *   1> Configures the Transmitter and Receiver FIFO Trigger Level granularity
 *   2> Configures the Transmitter and Receiver FIFO Trigger Level
 *   3> Configures the bits which clear/not clear the TX and RX FIFOs
 *   4> Configures the DMA mode of operation
 *
 * \see uart_irda_cir.c::UARTFIFOConfig
 */
static uint32_t UartFIFOConfigure(uint32_t baseAdd, uint32_t fifoConfig) {
	uint32_t txGra = (fifoConfig & UART_FIFO_CONFIG_TXGRA) >> 26;
	uint32_t rxGra = (fifoConfig & UART_FIFO_CONFIG_RXGRA) >> 22;

	uint32_t txTrig = (fifoConfig & UART_FIFO_CONFIG_TXTRIG) >> 14;
	uint32_t rxTrig = (fifoConfig & UART_FIFO_CONFIG_RXTRIG) >> 6;

	uint32_t txClr = (fifoConfig & UART_FIFO_CONFIG_TXCLR) >> 5;
	uint32_t rxClr = (fifoConfig & UART_FIFO_CONFIG_RXCLR) >> 4;

	uint32_t dmaEnPath = (fifoConfig & UART_FIFO_CONFIG_DMAENPATH) >> 3;
	uint32_t dmaMode = (fifoConfig & UART_FIFO_CONFIG_DMAMODE);

	uint32_t enhanFnBitVal = 0;
	uint32_t tcrTlrBitVal = 0;
	uint32_t tlrValue = 0;
	uint32_t fcrValue = 0;

	// Setting the EFR[4] bit to 1
	enhanFnBitVal = UartEnhanFuncEnable(baseAdd);

	tcrTlrBitVal = UartSubConfigTCRTLRModeEn(baseAdd);

	// Enabling FIFO mode of operation
	fcrValue |= UART_FCR_FIFO_EN;

	// Setting the Receiver FIFO trigger level
	if (UART_TRIG_LVL_GRANULARITY_1 != rxGra) {
		// Clearing the RXTRIGGRANU1 bit in SCR
		reg32a(baseAdd, UART_SCR, ~(UART_SCR_RX_TRIG_GRANU1));

		// Clearing the RX_FIFO_TRIG_DMA field of TLR register
		reg32a(baseAdd, UART_TLR, ~(UART_TLR_RX_FIFO_TRIG_DMA));

		fcrValue &= ~(UART_FCR_RX_FIFO_TRIG);

		// Checking if 'rxTrig' matches with the RX Trigger level values in FCR.
		if ((UART_FCR_RX_TRIG_LVL_8 == rxTrig)
				|| (UART_FCR_RX_TRIG_LVL_16 == rxTrig)
				|| (UART_FCR_RX_TRIG_LVL_56 == rxTrig)
				|| (UART_FCR_RX_TRIG_LVL_60 == rxTrig)) {
			fcrValue |= (rxTrig & UART_FCR_RX_FIFO_TRIG);
		} else {
			// RX Trigger level will be a multiple of 4
			// Programming the RX_FIFO_TRIG_DMA field of TLR register
			reg32m(baseAdd, UART_TLR,
					((rxTrig << UART_TLR_RX_FIFO_TRIG_DMA_SHIFT)
							& UART_TLR_RX_FIFO_TRIG_DMA));
		}
	} else {
		// 'rxTrig' now has the 6-bit RX Trigger level value
		rxTrig &= 0x003F;

		// Collecting the bits rxTrig[5:2]
		tlrValue = (rxTrig & 0x003C) >> 2;

		// Collecting the bits rxTrig[1:0] and writing to 'fcrValue'
		fcrValue |= (rxTrig & 0x0003) << UART_FCR_RX_FIFO_TRIG_SHIFT;

		// Setting the RXTRIGGRANU1 bit of SCR register
		reg32m(baseAdd, UART_SCR, UART_SCR_RX_TRIG_GRANU1);

		// Programming the RX_FIFO_TRIG_DMA field of TLR register
		reg32m(baseAdd, UART_TLR,
				(tlrValue << UART_TLR_RX_FIFO_TRIG_DMA_SHIFT));

	}

	// Setting the Transmitter FIFO trigger level
	if (UART_TRIG_LVL_GRANULARITY_1 != txGra) {
		// Clearing the TXTRIGGRANU1 bit in SCR
		reg32a(baseAdd, UART_SCR, ~(UART_SCR_TX_TRIG_GRANU1));

		// Clearing the TX_FIFO_TRIG_DMA field of TLR register
		reg32a(baseAdd, UART_TLR, ~(UART_TLR_TX_FIFO_TRIG_DMA));

		fcrValue &= ~(UART_FCR_TX_FIFO_TRIG);

		// Checking if 'txTrig' matches with the TX Trigger level values in FCR.
		if ((UART_FCR_TX_TRIG_LVL_8 == (txTrig))
				|| (UART_FCR_TX_TRIG_LVL_16 == (txTrig))
				|| (UART_FCR_TX_TRIG_LVL_32 == (txTrig))
				|| (UART_FCR_TX_TRIG_LVL_56 == (txTrig))) {
			fcrValue |= (txTrig & UART_FCR_TX_FIFO_TRIG);
		} else {
			// TX Trigger level will be a multiple of 4
			// Programming the TX_FIFO_TRIG_DMA field of TLR register
			reg32m(baseAdd, UART_TLR,
					((txTrig << UART_TLR_TX_FIFO_TRIG_DMA_SHIFT)
							& UART_TLR_TX_FIFO_TRIG_DMA));
		}
	} else {
		// 'txTrig' now has the 6-bit TX Trigger level value
		txTrig &= 0x003F;

		// Collecting the bits txTrig[5:2]
		tlrValue = (txTrig & 0x003C) >> 2;

		// Collecting the bits txTrig[1:0] and writing to 'fcrValue'
		fcrValue |= (txTrig & 0x0003) << UART_FCR_TX_FIFO_TRIG_SHIFT;

		// Setting the TXTRIGGRANU1 bit of SCR register
		reg32m(baseAdd, UART_SCR, UART_SCR_TX_TRIG_GRANU1);

		// Programming the TX_FIFO_TRIG_DMA field of TLR register
		reg32m(baseAdd, UART_TLR,
				(tlrValue << UART_TLR_TX_FIFO_TRIG_DMA_SHIFT));
	}

	if (UART_DMA_EN_PATH_FCR == dmaEnPath) {
		// Configuring the UART DMA Mode through FCR register
		reg32a(baseAdd, UART_SCR, ~(UART_SCR_DMA_MODE_CTL));

		dmaMode &= 0x1;

		// Clearing the bit corresponding to the DMA_MODE in 'fcrValue'
		fcrValue &= ~(UART_FCR_DMA_MODE);

		// Setting the DMA Mode of operation
		fcrValue |= (dmaMode << UART_FCR_DMA_MODE_SHIFT);
	} else {
		dmaMode &= 0x3;

		// Configuring the UART DMA Mode through SCR register
		reg32m(baseAdd, UART_SCR, UART_SCR_DMA_MODE_CTL);

		// Clearing the DMAMODE2 field in SCR
		reg32a(baseAdd, UART_SCR, ~(UART_SCR_DMA_MODE_2));

		// Programming the DMAMODE2 field in SCR
		reg32m(baseAdd, UART_SCR, (dmaMode << UART_SCR_DMA_MODE_2_SHIFT));
	}

	// Programming the bits which clear the RX and TX FIFOs
	fcrValue |= (rxClr << UART_FCR_RX_FIFO_CLEAR_SHIFT);
	fcrValue |= (txClr << UART_FCR_TX_FIFO_CLEAR_SHIFT);

	// Writing 'fcrValue' to the FIFO Control Register(FCR)
	UartFIFORegisterWrite(baseAdd, fcrValue);

	// Restoring the value of TCRTLR bit in MCR
	UartTCRTLRBitValRestore(baseAdd, tcrTlrBitVal);

	// Restoring the value of EFR[4] to the original value
	UartEnhanFuncBitValRestore(baseAdd, enhanFnBitVal);

	return fcrValue;
}

/**
 * \brief sets a certain bit in Enhanced Feature Register(EFR) which
 * 		  shall avail the UART to use some Enhanced Features
 *
 * \see uart_irda_cir.c::UARTEnhanFuncEnable
 */
uint32_t UartEnhanFuncEnable(uint32_t baseAdd) {
	uint32_t enhanFnBitVal = 0;
	uint32_t lcrRegValue = 0;

	/* Enabling Configuration Mode B of operation. */
	lcrRegValue = UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	/* Collecting the current value of ENHANCEDEN bit of EFR. */
	enhanFnBitVal = (reg32r(baseAdd, UART_EFR) & UART_EFR_ENHANCED_EN);

	/* Setting the ENHANCEDEN bit in EFR register. */
	reg32m(baseAdd, UART_EFR, UART_EFR_ENHANCED_EN);

	/* Programming LCR with the collected value. */
	reg32w(baseAdd, UART_LCR, lcrRegValue);

	return enhanFnBitVal;
}

/**
 * \brief configures the specified Register Configuration mode for
 *        the UART
 *
 * \see uart_irda_cir.c::UARTRegConfigModeEnable
 */
static uint32_t UartRegConfigModeEnable(uint32_t baseAdd, uint32_t modeFlag) {
	uint32_t lcrRegValue = 0;

	/* Preserving the current value of LCR. */
	lcrRegValue = reg32r(baseAdd, UART_LCR);

	switch (modeFlag) {
	case UART_REG_CONFIG_MODE_A:
	case UART_REG_CONFIG_MODE_B:
		reg32w(baseAdd, UART_LCR, modeFlag & 0xFF);
		break;

	case UART_REG_OPERATIONAL_MODE:
		reg32a(baseAdd, UART_LCR, 0x7F);
		break;

	default:
		break;
	}

	return lcrRegValue;
}

/**
 * \brief enables the TCR_TLR Sub_Configuration Mode of operation
 *
 * \see uart_irda_cir.c::UARTSubConfigTCRTLRModeEn
 */
static uint32_t UartSubConfigTCRTLRModeEn(uint32_t baseAdd) {
	uint32_t enhanFnBitVal = 0;
	uint32_t tcrTlrValue = 0;
	uint32_t lcrRegValue = 0;

	/* Switching to Register Configuration Mode B. */
	lcrRegValue = UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	/* Collecting the current value of EFR[4] and later setting it. */
	enhanFnBitVal = reg32r(baseAdd, UART_EFR) & UART_EFR_ENHANCED_EN;
	reg32m(baseAdd, UART_EFR, UART_EFR_ENHANCED_EN);

	/* Switching to Register Configuration Mode A. */
	UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_A);

	/* Collecting the bit value of MCR[6]. */
	tcrTlrValue = (reg32r(baseAdd, UART_MCR) & UART_MCR_TCR_TLR);

	/* Setting the TCRTLR bit in Modem Control Register(MCR). */
	reg32m(baseAdd, UART_MCR, UART_MCR_TCR_TLR);

	/* Switching to Register Configuration Mode B. */
	UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	/* Restoring the value of EFR[4] to its original value. */
	reg32a(baseAdd, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAdd, UART_EFR, enhanFnBitVal);

	/* Restoring the value of LCR. */
	reg32w(baseAdd, UART_LCR, lcrRegValue);

	return tcrTlrValue;
}

/**
 * \brief write a specified value to the FIFO Control Register(FCR)
 *
 * \see uart_irda_cir.c::UARTFIFORegisterWrite
 */
static void UartFIFORegisterWrite(uint32_t baseAdd, uint32_t fcrValue) {
	uint32_t divLatchRegVal = 0;
	uint32_t enhanFnBitVal = 0;
	uint32_t lcrRegValue = 0;

	// Switching to Register Configuration Mode A of operation
	lcrRegValue = UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_A);

	// Clearing the contents of Divisor Latch Registers
	divLatchRegVal = UartDivisorLatchWrite(baseAdd, 0x0000);

	// Set the EFR[4] bit to 1
	enhanFnBitVal = UartEnhanFuncEnable(baseAdd);

	// Writing the 'fcrValue' to the FCR register
	reg32w(baseAdd, UART_FCR, fcrValue);

	// Restoring the value of EFR[4] to its original value
	UartEnhanFuncBitValRestore(baseAdd, enhanFnBitVal);

	// Programming the Divisor Latch Registers with the collected value
	UartDivisorLatchWrite(baseAdd, divLatchRegVal);

	// Reinstating LCR with its original value
	reg32w(baseAdd, UART_LCR, lcrRegValue);
}

/**
 * \brief  write the specified divisor value to Divisor Latch registers DLL and DLH
 *
 * \see uart_irda_cir.c::UARTDivisorLatchWrite
 */
static uint32_t UartDivisorLatchWrite(uint32_t baseAddr, uint32_t divisorValue) {
	volatile uint32_t enhanFnBitVal = 0;
	volatile uint32_t sleepMdBitVal = 0;
	volatile uint32_t lcrRegValue = 0;
	volatile uint32_t operMode = 0;
	uint32_t divRegVal = 0;

	// Switching to Register Configuration Mode B
	lcrRegValue = UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of EFR[4] and later setting it
	enhanFnBitVal = reg32r(baseAddr, UART_EFR) & UART_EFR_ENHANCED_EN;
	reg32m(baseAddr, UART_EFR, UART_EFR_ENHANCED_EN);

	// Switching to Register Operational Mode
	UartRegConfigModeEnable(baseAddr, UART_REG_OPERATIONAL_MODE);

	//Collecting the current value of IER[4](SLEEPMODE bit) and later clearing it
	sleepMdBitVal = reg32r(baseAddr, UART_IER) & UART_IER_SLEEP_MODE_IT;
	reg32a(baseAddr, UART_IER, ~(UART_IER_SLEEP_MODE_IT));

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of Divisor Latch Registers
	divRegVal = reg32r(baseAddr, UART_DLL) & 0xFF;
	divRegVal |= ((reg32r(baseAddr, UART_DLH) & 0x3F) << 8);

	// Switch the UART instance to Disabled state
	operMode = UartOperatingModeSelect(baseAddr,
			UART_MDR1_MODE_SELECT_DISABLED);

	// Writing to Divisor Latch Low(DLL) register
	reg32w(baseAddr, UART_DLL, divisorValue & 0x00FF);

	// Writing to Divisor Latch High(DLH) register
	reg32w(baseAddr, UART_DLH, ((divisorValue & 0x3F00) >> 8));

	// Restoring the Operating Mode of UART
	UartOperatingModeSelect(baseAddr, operMode);

	// Switching to Register Operational Mode
	UartRegConfigModeEnable(baseAddr, UART_REG_OPERATIONAL_MODE);

	// Restoring the value of IER[4] to its original value
	reg32m(baseAddr, UART_IER, sleepMdBitVal);

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Restoring the value of EFR[4] to its original value
	reg32a(baseAddr, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAddr, UART_EFR, enhanFnBitVal);

	/* Restoring the value of LCR Register. */
	reg32w(baseAddr, UART_LCR, lcrRegValue);

	return divRegVal;
}

/**
 * \brief restores the ENHANCEDEN bit value of EFR register(EFR[4]) to the corresponding bit
 * 		  value in 'enhanFnBitVal' passed as a parameter to this API.
 *
 * \see uart_irda_cir.c::UARTEnhanFuncBitValRestore
 */
static void UartEnhanFuncBitValRestore(uint32_t baseAddr,
		uint32_t enhanFnBitVal) {
	uint32_t lcrRegValue = 0;

	// Enabling Configuration Mode B of operation
	lcrRegValue = UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Restoring the value of EFR[4]
	reg32a(baseAddr, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAddr, UART_EFR, (enhanFnBitVal & UART_EFR_ENHANCED_EN));

	// Programming LCR with the collected value
	reg32w(baseAddr, UART_LCR, lcrRegValue);
}

/**
 * \brief configures the operating mode for the UART instance.
 *        The different operating modes are:
 *           - UART(16x, 13x, 16x Auto-Baud)
 *           - IrDA(SIR, MIR, FIR)
 *           - CIR
 *           - Disabled state(default state)
 *
 * \see uart_irda_cir.c::UARTOperatingModeSelect
 */
static uint32_t UartOperatingModeSelect(uint32_t baseAddr, uint32_t modeFlag) {
	uint32_t operMode = 0;

	operMode = (reg32r(baseAddr, UART_MDR1) & UART_MDR1_MODE_SELECT);

	/* Clearing the MODESELECT field in MDR1. */
	reg32a(baseAddr, UART_MDR1, ~(UART_MDR1_MODE_SELECT));
	/* Programming the MODESELECT field in MDR1. */
	reg32m(baseAddr, UART_MDR1, (modeFlag & UART_MDR1_MODE_SELECT));

	return operMode;
}

/**
 * \brief restores the TCRTLR bit(MCR[6]) value in Modem Control Register(MCR) to the
 *  	  corresponding bit value in 'tcrTlrBitVal' passed as a parameter to this API
 *
 * \see uart_irda_cir.c::UARTTCRTLRBitValRestore
 */
static void UartTCRTLRBitValRestore(uint32_t baseAddr, uint32_t tcrTlrBitVal) {
	uint32_t enhanFnBitVal = 0;
	uint32_t lcrRegValue = 0;

	// Switching to Register Configuration Mode B
	lcrRegValue = UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of EFR[4] and later setting it
	enhanFnBitVal = reg32r(baseAddr, UART_EFR) & UART_EFR_ENHANCED_EN;
	reg32m(baseAddr, UART_EFR, UART_EFR_ENHANCED_EN);

	// Switching to Configuration Mode A of operation
	UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_A);

	// Programming MCR[6] with the corresponding bit value in 'tcrTlrBitVal'
	reg32a(baseAddr, UART_MCR, ~(UART_MCR_TCR_TLR));
	reg32m(baseAddr, UART_MCR, (tcrTlrBitVal & UART_MCR_TCR_TLR));

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAddr, UART_REG_CONFIG_MODE_B);

	/* Restoring the value of EFR[4] to its original value. */
	reg32a(baseAddr, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAddr, UART_EFR, enhanFnBitVal);

	/* Restoring the value of LCR. */
	reg32w(baseAddr, UART_LCR, lcrRegValue);
}

/**
 * \brief returns type of uart interrupt
 *
 * \see uart_irda_cir.c::UARTIntIdentityGet
 */
uint32_t UartIntIdentityGet(uint32_t baseAdd) {
	uint32_t lcrRegValue = 0;
	uint32_t retVal = 0;

	// Switching to Register Operational Mode of operation
	lcrRegValue = UartRegConfigModeEnable(baseAdd, UART_REG_OPERATIONAL_MODE);

	retVal = (reg32r(baseAdd, UART_IIR) & UART_IIR_IT_TYPE);

	/* Restoring the value of LCR. */
	reg32w(baseAdd, UART_LCR, lcrRegValue);

	return retVal;
}
