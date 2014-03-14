/*
 * Driver: uart.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 13, 2014
 * Description: 
 * TODO
 */

#ifndef UART_H_
#define UART_H_

#define UART_FIFO_CONFIG_TXGRA     (0xF << 26)
#define UART_FIFO_CONFIG_RXGRA     (0xF << 22)

/**
 * Enables UART0
 */
void UART0Enable();

/**
 * Select UART0
 */
void UART0PinMuxSetup();

/**
 * This API configures the FIFO settings for the UART0 instance.
 * Specifically, this does the following configurations:
 * 	 1> Configures the Transmitter and Receiver FIFO Trigger Level granularity
 *   2> Configures the Transmitter and Receiver FIFO Trigger Level
 *   3> Configures the bits which clear/not clear the TX and RX FIFOs
 *   4> Configures the DMA mode of operation
 */
unsigned int UART0FIFOConfig(unsigned int fifoConfig);

/**
 * This API is used to write the specified divisor value to Divisor
 * Latch registers DLL and DLH for UART0.
 */
unsigned int UART0DivisorLatchWrite(unsigned int divisorValue);

/**
 * Reset UART0
 */
void UART0ModuleReset();

/**
 * This API configures the specified Register Configuration mode for
 * the UART.
 */
unsigned int UART0RegConfigModeEnable(unsigned int modeFlag);

/**
 * This API disables write access to Divisor Latch registers DLL and
 * DLH.
 */
void UART0DivisorLatchDisable();

/**
 * This API configures the Line Characteristics for the
 * UART instance. The Line Characteristics include:
 *   - Word length per frame
 *   - Number of Stop Bits per frame
 *   - Parity feature configuration
 */
extern void UARTLineCharacConfig(unsigned int wLenStbFlag,
		unsigned int parityFlag);

/**
 * Parameterized macro to configure the FIFO settings.
 */
#define UART_FIFO_CONFIG(txGra, rxGra, txTrig, rxTrig, txClr, rxClr, dmaEnPath, dmaMode) \
                        ((unsigned int) \
                         (((txGra & 0xF) << 26) | \
                          ((rxGra & 0xF) << 22) | \
                          ((txTrig & 0xFF) << 14) | \
                          ((rxTrig & 0xFF) << 6) | \
                          ((txClr & 0x1) << 5) | \
                          ((rxClr & 0x1) << 4) | \
                          ((dmaEnPath & 0x1) << 3) | \
                          (dmaMode & 0x7)))

/**
 * This API performs a module reset of the UART module. It also
 * waits until the reset process is complete.
 */
void UARTModuleReset(unsigned int baseAdd);

/**
 * This API configures the FIFO settings for the UART instance.
 * Specifically, this does the following configurations:
 * 	 1> Configures the Transmitter and Receiver FIFO Trigger Level granularity
 *   2> Configures the Transmitter and Receiver FIFO Trigger Level
 *   3> Configures the bits which clear/not clear the TX and RX FIFOs
 *   4> Configures the DMA mode of operation
 */
unsigned int UARTFIFOConfig(unsigned int baseAdd, unsigned int fifoConfig);

/**
 * This API is used to write the specified divisor value to Divisor
 * Latch registers DLL and DLH.
 */
unsigned int UARTDivisorLatchWrite(unsigned int baseAdd,
		unsigned int divisorValue);

/**
 * This API computes the divisor value for the specified operating
 * mode. Not part of this API, the divisor value returned is written
 * to the Divisor Latches to configure the Baud Rate.
 */
unsigned int UARTDivisorValCompute(unsigned int moduleClk,
		unsigned int baudRate, unsigned int modeFlag,
		unsigned int mirOverSampRate);

/**
 * This API configures the specified Register Configuration mode for
 * the UART.
 */
unsigned int UARTRegConfigModeEnable(unsigned int baseAdd,
		unsigned int modeFlag);

/**
 * This API configures the Line Characteristics for the
 * UART instance. The Line Characteristics include:
 *   - Word length per frame
 *   - Number of Stop Bits per frame
 *   - Parity feature configuration
 */
extern void UARTLineCharacConfig(unsigned int baseAdd, unsigned int wLenStbFlag,
		unsigned int parityFlag);

/**
 * This API disables write access to Divisor Latch registers DLL and
 * DLH.
 */
void UARTDivisorLatchDisable(unsigned int baseAdd);

/**
 * This API performs a module reset of the UART module. It also
 * waits until the reset process is complete.
 */
void UARTModuleReset(unsigned int baseAdd);

#endif /* UART_H_ */
