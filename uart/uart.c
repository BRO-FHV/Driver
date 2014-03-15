/*
 * Driver: uart.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 13, 2014
 * Description: 
 * TODO description
 *
 * TODO documentation
 */

#include <inttypes.h>
#include <soc_AM335x.h>
#include <hw_beaglebone.h>
#include <hw_uart.h>
#include <basic.h>
#include "uart.h"

// init function forward declaration
extern void UartModuleReset(uint32_t baseAdd);
static void UartFIFOConfigure(void);
static uint32_t UartFIFOConfig(uint32_t baseAdd, uint32_t fifoConfig);
static uint32_t UartEnhanFuncEnable(uint32_t baseAdd);
static uint32_t UartRegConfigModeEnable(uint32_t baseAdd, uint32_t modeFlag);
static uint32_t UartSubConfigTCRTLRModeEn(uint32_t baseAdd);
static void UartFIFORegisterWrite(uint32_t baseAdd, uint32_t fcrValue);
static uint32_t UartDivisorLatchWrite(uint32_t baseAdd, uint32_t divisorValue);
static void UartEnhanFuncBitValRestore(uint32_t baseAdd, uint32_t enhanFnBitVal);
static uint32_t UartOperatingModeSelect(uint32_t baseAdd, uint32_t modeFlag);
static void UartTCRTLRBitValRestore(uint32_t baseAdd, uint32_t tcrTlrBitVal);

// configure function forward declaration
static void UartBaudRateSet(uint32_t baseAddr, uint32_t baudRate);
static uint32_t UartDivisorValCompute(uint32_t moduleClk,
		uint32_t baudRate, uint32_t modeFlag,
		uint32_t mirOverSampRate);

void UartEnable(uint32_t baseAddr) {
	// Enable Module
	Uart0ModuleClkConfig();

	// Select Uart0
	UartPinMuxSetup(0);

	// Performing a module reset
	UartModuleReset(SOC_UART_0_REGS);

	UartFIFOConfigure();
}

void UartConfigure(uint32_t baseAddr, uint32_t baudRate) {
	// set baud rate
	UartBaudRateSet(baseAddr, baudRate);
}

static void UartBaudRateSet(uint32_t baseAddr, uint32_t baudRate) {
	uint32_t divisorValue = 0;

	// Computing the Divisor Value
	divisorValue = UartDivisorValCompute(UART_MODULE_INPUT_CLK,
			baudRate, UART16x_OPER_MODE, UART_MIR_OVERSAMPLING_RATE_42);

	//Programming the Divisor Latches
	UartDivisorLatchWrite(SOC_UART_0_REGS, divisorValue);
}

static uint32_t UartDivisorValCompute(uint32_t moduleClk,
		uint32_t baudRate, uint32_t modeFlag,
		uint32_t mirOverSampRate) {
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

void UartModuleReset(uint32_t baseAdd) {
	// Performing Software Reset of the module
	reg32m(baseAdd, UART_SYSC, UART_SYSC_SOFTRESET);

	// Wait until the process of Module Reset is complete
	wait(!(reg32r(baseAdd, UART_SYSS) & UART_SYSS_RESETDONE));
}

static void UartFIFOConfigure(void) {
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

	/* Configuring the FIFO settings. */
	UartFIFOConfig(SOC_UART_0_REGS, fifoConfig);
}

static uint32_t UartFIFOConfig(uint32_t baseAdd, uint32_t fifoConfig) {
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

static uint32_t UartDivisorLatchWrite(uint32_t baseAdd, uint32_t divisorValue) {
	volatile uint32_t enhanFnBitVal = 0;
	volatile uint32_t sleepMdBitVal = 0;
	volatile uint32_t lcrRegValue = 0;
	volatile uint32_t operMode = 0;
	uint32_t divRegVal = 0;

	// Switching to Register Configuration Mode B
	lcrRegValue = UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of EFR[4] and later setting it
	enhanFnBitVal = reg32r(baseAdd, UART_EFR) & UART_EFR_ENHANCED_EN;
	reg32m(baseAdd, UART_EFR, UART_EFR_ENHANCED_EN);

	// Switching to Register Operational Mode
	UartRegConfigModeEnable(baseAdd, UART_REG_OPERATIONAL_MODE);

	//Collecting the current value of IER[4](SLEEPMODE bit) and later clearing it
	sleepMdBitVal = reg32r(baseAdd, UART_IER) & UART_IER_SLEEP_MODE_IT;
	reg32a(baseAdd, UART_IER, ~(UART_IER_SLEEP_MODE_IT));

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of Divisor Latch Registers
	divRegVal = reg32r(baseAdd, UART_DLL) & 0xFF;
	divRegVal |= ((reg32r(baseAdd, UART_DLH) & 0x3F) << 8);

	// Switch the UART instance to Disabled state
	operMode = UartOperatingModeSelect(baseAdd, UART_MDR1_MODE_SELECT_DISABLED);

	// Writing to Divisor Latch Low(DLL) register
	reg32w(baseAdd, UART_DLL, divisorValue & 0x00FF);

	// Writing to Divisor Latch High(DLH) register
	reg32w(baseAdd, UART_DLH, ((divisorValue & 0x3F00) >> 8));

	// Restoring the Operating Mode of UART
	UartOperatingModeSelect(baseAdd, operMode);

	// Switching to Register Operational Mode
	UartRegConfigModeEnable(baseAdd, UART_REG_OPERATIONAL_MODE);

	// Restoring the value of IER[4] to its original value
	reg32m(baseAdd, UART_IER, sleepMdBitVal);

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	// Restoring the value of EFR[4] to its original value
	reg32a(baseAdd, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAdd, UART_EFR, enhanFnBitVal);

	/* Restoring the value of LCR Register. */
	reg32w(baseAdd, UART_LCR, lcrRegValue);

	return divRegVal;
}

static void UartEnhanFuncBitValRestore(uint32_t baseAdd, uint32_t enhanFnBitVal) {
	uint32_t lcrRegValue = 0;

	// Enabling Configuration Mode B of operation
	lcrRegValue = UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	// Restoring the value of EFR[4]
	reg32a(baseAdd, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAdd, UART_EFR, (enhanFnBitVal & UART_EFR_ENHANCED_EN));

	// Programming LCR with the collected value
	reg32w(baseAdd, UART_LCR, lcrRegValue);
}

static uint32_t UartOperatingModeSelect(uint32_t baseAdd, uint32_t modeFlag) {
	uint32_t operMode = 0;

	operMode = (reg32r(baseAdd, UART_MDR1) & UART_MDR1_MODE_SELECT);

	/* Clearing the MODESELECT field in MDR1. */
	reg32a(baseAdd, UART_MDR1, ~(UART_MDR1_MODE_SELECT));
	/* Programming the MODESELECT field in MDR1. */
	reg32m(baseAdd, UART_MDR1, (modeFlag & UART_MDR1_MODE_SELECT));

	return operMode;
}

static void UartTCRTLRBitValRestore(uint32_t baseAdd, uint32_t tcrTlrBitVal) {
	uint32_t enhanFnBitVal = 0;

	// Switching to Register Configuration Mode B
	lcrRegValue = UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	// Collecting the current value of EFR[4] and later setting it
	enhanFnBitVal = reg32r(baseAdd, UART_EFR) & UART_EFR_ENHANCED_EN;
	reg32m(baseAdd, UART_EFR, UART_EFR_ENHANCED_EN);

	// Switching to Configuration Mode A of operation
	UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_A);

	// Programming MCR[6] with the corresponding bit value in 'tcrTlrBitVal'
	reg32a(baseAdd, UART_MCR, ~(UART_MCR_TCR_TLR));
	reg32m(baseAdd, UART_MCR, (tcrTlrBitVal & UART_MCR_TCR_TLR));

	// Switching to Register Configuration Mode B
	UartRegConfigModeEnable(baseAdd, UART_REG_CONFIG_MODE_B);

	/* Restoring the value of EFR[4] to its original value. */
	reg32a(baseAdd, UART_EFR, ~(UART_EFR_ENHANCED_EN));
	reg32m(baseAdd, UART_EFR, enhanFnBitVal);

	/* Restoring the value of LCR. */
	reg32w(baseAdd, UART_LCR, lcrRegValue);
}
