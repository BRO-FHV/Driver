/*
 * Driver: uart.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 13, 2014
 * Description: 
 * Definition for UART
 */

#ifndef UART_H_
#define UART_H_

#include <inttypes.h>
#include <uart/hw_uart.h>
#include <stdio.h>

// Values to configure the Operating modes of UART.
#define UART16x_OPER_MODE                   (UART_MDR1_MODE_SELECT_UART16X)
#define UART_SIR_OPER_MODE                  (UART_MDR1_MODE_SELECT_SIR)
#define UART16x_AUTO_BAUD_OPER_MODE         (UART_MDR1_MODE_SELECT_UART16XAUTO)
#define UART13x_OPER_MODE                   (UART_MDR1_MODE_SELECT_UART13X)
#define UART_MIR_OPER_MODE                  (UART_MDR1_MODE_SELECT_MIR)
#define UART_FIR_OPER_MODE                  (UART_MDR1_MODE_SELECT_FIR)
#define UART_CIR_OPER_MODE                  (UART_MDR1_MODE_SELECT_CIR)
#define UART_DISABLED_MODE                  (UART_MDR1_MODE_SELECT_DISABLED)

// Over-sampling rate for MIR mode used to obtain the Divisor Values
#define UART_MIR_OVERSAMPLING_RATE_41          (41)
#define UART_MIR_OVERSAMPLING_RATE_42          (42)

#define UART_MODULE_INPUT_CLK					(48000000u)

// Word Length per frame
#define UART_FRAME_WORD_LENGTH_5            (UART_LCR_CHAR_LENGTH_5BIT)
#define UART_FRAME_WORD_LENGTH_6            (UART_LCR_CHAR_LENGTH_6BIT)
#define UART_FRAME_WORD_LENGTH_7            (UART_LCR_CHAR_LENGTH_7BIT)
#define UART_FRAME_WORD_LENGTH_8            (UART_LCR_CHAR_LENGTH_8BIT)

// Number of Stop Bits per frame
#define UART_FRAME_NUM_STB_1                (UART_LCR_NB_STOP_1BIT << \
                                             UART_LCR_NB_STOP_SHIFT)
#define UART_FRAME_NUM_STB_1_5_2            (UART_LCR_NB_STOP_2BIT << \
                                             UART_LCR_NB_STOP_SHIFT)

// Values to control parity feature
#define UART_PARITY_REPR_1                  (UART_LCR_PARITY_TYPE2 | \
                                             (UART_LCR_PARITY_TYPE1_ODD << \
                                              UART_LCR_PARITY_TYPE1_SHIFT) | \
                                              UART_LCR_PARITY_EN)

#define UART_PARITY_REPR_0                  (UART_LCR_PARITY_TYPE2 | \
                                             (UART_LCR_PARITY_TYPE1_EVEN << \
                                              UART_LCR_PARITY_TYPE1_SHIFT) | \
                                              UART_LCR_PARITY_EN)

#define UART_ODD_PARITY                     ((UART_LCR_PARITY_TYPE1_ODD << \
                                              UART_LCR_PARITY_TYPE1_SHIFT) | \
                                              UART_LCR_PARITY_EN)

#define UART_EVEN_PARITY                    ((UART_LCR_PARITY_TYPE1_EVEN << \
                                              UART_LCR_PARITY_TYPE1_SHIFT) | \
                                              UART_LCR_PARITY_EN)

#define UART_PARITY_NONE                    (UART_LCR_PARITY_EN_DISABLE << \
                                             UART_LCR_PARITY_EN_SHIFT)

// Break condition generation controls
#define UART_BREAK_COND_DISABLE             (UART_LCR_BREAK_EN_NORMAL << \
                                             UART_LCR_BREAK_EN_SHIFT)
#define UART_BREAK_COND_ENABLE              (UART_LCR_BREAK_EN_FORCE << \
                                             UART_LCR_BREAK_EN_SHIFT)

// level for the Transmitter FIFO
#define UART_FCR_TX_TRIG_LVL_8              (UART_FCR_TX_FIFO_TRIG_8SPACES << \
                                             UART_FCR_TX_FIFO_TRIG_SHIFT)
#define UART_FCR_TX_TRIG_LVL_16             (UART_FCR_TX_FIFO_TRIG_16SPACES << \
                                             UART_FCR_TX_FIFO_TRIG_SHIFT)
#define UART_FCR_TX_TRIG_LVL_32             (UART_FCR_TX_FIFO_TRIG_32SPACES << \
                                             UART_FCR_TX_FIFO_TRIG_SHIFT)
#define UART_FCR_TX_TRIG_LVL_56             (UART_FCR_TX_FIFO_TRIG_56SPACES << \
                                             UART_FCR_TX_FIFO_TRIG_SHIFT)

// level for the Receiver FIFO
#define UART_FCR_RX_TRIG_LVL_8              (UART_FCR_RX_FIFO_TRIG_8CHAR <<   \
                                             UART_FCR_RX_FIFO_TRIG_SHIFT)
#define UART_FCR_RX_TRIG_LVL_16             (UART_FCR_RX_FIFO_TRIG_16CHAR <<  \
                                             UART_FCR_RX_FIFO_TRIG_SHIFT)
#define UART_FCR_RX_TRIG_LVL_56             (UART_FCR_RX_FIFO_TRIG_56CHAR <<  \
                                             UART_FCR_RX_FIFO_TRIG_SHIFT)
#define UART_FCR_RX_TRIG_LVL_60             (UART_FCR_RX_FIFO_TRIG_60CHAR <<  \
                                             UART_FCR_RX_FIFO_TRIG_SHIFT)
// trigger level granularity
#define UART_TRIG_LVL_GRANULARITY_4         (0x0000)
#define UART_TRIG_LVL_GRANULARITY_1         (0x0001)

// register configuration modes
#define UART_REG_CONFIG_MODE_A              (0x0080)
#define UART_REG_CONFIG_MODE_B              (0x00BF)
#define UART_REG_OPERATIONAL_MODE           (0x007F)

// DMA mode
// DMA mode could be configured either through FCR or SCR
#define UART_DMA_EN_PATH_FCR                (UART_SCR_DMA_MODE_CTL_FCR)
#define UART_DMA_EN_PATH_SCR                (UART_SCR_DMA_MODE_CTL_SCR)

// DMA mode selection
#define UART_DMA_MODE_0_ENABLE              (UART_SCR_DMA_MODE_2_MODE0)
#define UART_DMA_MODE_1_ENABLE              (UART_SCR_DMA_MODE_2_MODE1)
#define UART_DMA_MODE_2_ENABLE              (UART_SCR_DMA_MODE_2_MODE2)
#define UART_DMA_MODE_3_ENABLE              (UART_SCR_DMA_MODE_2_MODE3)

// enabling/disabling the interrupts of UART
#define UART_INT_CTS                        (UART_IER_CTS_IT)
#define UART_INT_RTS                        (UART_IER_RTS_IT)
#define UART_INT_XOFF                       (UART_IER_XOFF_IT)
#define UART_INT_SLEEPMODE                  (UART_IER_SLEEP_MODE_IT)
#define UART_INT_MODEM_STAT                 (UART_IER_MODEM_STS_IT)
#define UART_INT_LINE_STAT                  (UART_IER_LINE_STS_IT)
#define UART_INT_THR                        (UART_IER_THR_IT)
#define UART_INT_RHR_CTI                    (UART_IER_RHR_IT)


// Values pertaining to status of UART Interrupt sources
#define UART_INTID_MODEM_STAT               (UART_IIR_IT_TYPE_MODEMINT << \
                                             UART_IIR_IT_TYPE_SHIFT)
#define UART_INTID_TX_THRES_REACH           (UART_IIR_IT_TYPE_THRINT << \
                                             UART_IIR_IT_TYPE_SHIFT)
#define UART_INTID_RX_THRES_REACH           (UART_IIR_IT_TYPE_RHRINT << \
                                             UART_IIR_IT_TYPE_SHIFT)
#define UART_INTID_RX_LINE_STAT_ERROR       (UART_IIR_IT_TYPE_RXSTATUSERROR << \
                                             UART_IIR_IT_TYPE_SHIFT)
#define UART_INTID_CHAR_TIMEOUT             (UART_IIR_IT_TYPE_RXTIMEOUT << \
                                             UART_IIR_IT_TYPE_SHIFT)
#define UART_INTID_XOFF_SPEC_CHAR_DETECT    (UART_IIR_IT_TYPE_XOFF << \
                                             UART_IIR_IT_TYPE_SHIFT)
#define UART_INTID_MODEM_SIG_STATE_CHANGE   (UART_IIR_IT_TYPE_STATECHANGE << \
                                             UART_IIR_IT_TYPE_SHIFT)

// Parameterized macro to configure the FIFO settings
#define UART_FIFO_CONFIG(txGra, rxGra, txTrig, rxTrig, txClr, rxClr, dmaEnPath, dmaMode) \
                        ((uint32_t) \
                         (((txGra & 0xF) << 26) | \
                          ((rxGra & 0xF) << 22) | \
                          ((txTrig & 0xFF) << 14) | \
                          ((rxTrig & 0xFF) << 6) | \
                          ((txClr & 0x1) << 5) | \
                          ((rxClr & 0x1) << 4) | \
                          ((dmaEnPath & 0x1) << 3) | \
                          (dmaMode & 0x7)))

#define UART_FIFO_CONFIG_TXGRA     (0xF << 26)
#define UART_FIFO_CONFIG_RXGRA     (0xF << 22)
#define UART_FIFO_CONFIG_TXTRIG    (0xFF << 14)
#define UART_FIFO_CONFIG_RXTRIG    (0xFF << 6)
#define UART_FIFO_CONFIG_TXCLR     (0x1 << 5)
#define UART_FIFO_CONFIG_RXCLR     (0x1 << 4)
#define UART_FIFO_CONFIG_DMAENPATH (0x1 << 3)
#define UART_FIFO_CONFIG_DMAMODE   (0x7 << 0)

/**
 * \brief This function enables UART module identified with base address
 *
 * \param baseAddr basic address of module
 *
 * \return none
 */
void UartEnable(uint32_t baseAddr);

/**
 * \brief This function configures UART module with default values and given baudrate
 *
 * \param baseAddr basic address of module
 * \param baudRate baud rate to configure
 *
 * \return none
 */
void UartConfigure(uint32_t baseAddr, uint32_t baudRate);

/**
 * \brief This function enables UART interrupt
 *
 * \return none
 */
void UartSystemIntEnable(void);

/**
 * \brief   This function enables the specified interrupts in the UART mode of
 *          operation
 *
 * \param   baseAddr   Memory address of the UART instance being used.
 * \param   intFlag   Bit mask value of the bits corresponding to Interrupt
 *                    Enable Register(IER). This specifies the UART interrupts
 *                    to be enabled.
 *
 *  'intFlag' can take one or a combination of the following macros:
 *  - UART_INT_CTS - to enable Clear-To-Send interrupt,
 *  - UART_INT_RTS - to enable Request-To-Send interrupt,
 *  - UART_INT_XOFF - to enable XOFF interrupt,
 *  - UART_INT_SLEEPMODE - to enable Sleep Mode,
 *  - UART_INT_MODEM_STAT - to enable Modem Status interrupt,
 *  - UART_INT_LINE_STAT - to enable Line Status interrupt,
 *  - UART_INT_THR - to enable Transmitter Holding Register Empty interrupt,
 *  - UART_INT_RHR_CTI - to enable Receiver Data available interrupt and
 *                       Character timeout indication interrupt.
 *
 * \return  None.
 *
 * \note    This API modifies the contents of UART Interrupt Enable Register
 *          (IER). Modifying the bits IER[7:4] requires that EFR[4] be set.
 *          This API does the needful before it accesses IER.
 *          Moreover, this API should be called when UART is operating in
 *          UART 16x Mode, UART 13x Mode or UART 16x Auto-baud mode.\n
 *
 * \see uart_irda_cir.c::UARTIntEnable
 */
void UartIntEnable(uint32_t baseAddr, uint32_t intFlag);

/**
 * \brief   This function disables the specified interrupts in the UART mode of
 *          operation.
 *
 * \param   baseAddr  Memory address of the UART instance being used.
 * \param   intFlag   Bit mask value of the bits corresponding to Interrupt
 *                    Enable Register(IER). This specifies the UART interrupts
 *                    to be disabled.
 *
 *  'intFlag' can take one or a combination of the following macros:
 *  - UART_INT_CTS - to disable Clear-To-Send interrupt,
 *  - UART_INT_RTS - to disable Request-To-Send interrupt,
 *  - UART_INT_XOFF - to disable XOFF interrupt,
 *  - UART_INT_SLEEPMODE - to disable Sleep Mode,
 *  - UART_INT_MODEM_STAT - to disable Modem Status interrupt,
 *  - UART_INT_LINE_STAT - to disable Line Status interrupt,
 *  - UART_INT_THR - to disable Transmitter Holding Register Empty interrupt,
 *  - UART_INT_RHR_CTI - to disable Receiver Data available interrupt and
 *                       Character timeout indication interrupt.
 *
 * \return  None
 *
 * \note  The note section of UARTIntEnable() also applies to this API.
 *
 * \see uart_irda_cir.c::UARTIntDisable
 */
void UartIntDisable(uint32_t baseAddr, uint32_t intFlag);

/**
 * \brief This function sends a message over UART identified with base address
 *
 * \param baseAddr 		basic address of module
 * \param pBuffer 		pointer to message
 * \param numTxBytes 	length of message
 *
 * \return number of data bytes that were written to the TX FIFO
 *
 * \note   This function does not check for the emptiness of the TX FIFO or for
 *         its space availability before writing to it. The application
 *         calling this function has the responsibility of checking the TX
 *         FIFO status before using this API
 */
uint32_t UartWrite(uint32_t baseAddr, const char *pBuffer, uint32_t numTxBytes);
void UartWritef(uint32_t baseAddr,const char* string, va_list vaArg);

#endif /* UART_H_ */
