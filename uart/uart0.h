/*
 * Driver: uart0.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * TODO
 */

#ifndef UART0_H_
#define UART0_H_

/**
 * \brief Enables UART0 Instance
 */
void Uart0Enable();

/**
 * \brief Configures UART0 Instance
 *
 * \param baudRate		communication speed
 */
void Uart0Configure(int baudRate);

/**
 * \brief Writes a message
 *
 * \param message
 */
void Uart0Write(char message[]);

#endif /* UART0_H_ */
