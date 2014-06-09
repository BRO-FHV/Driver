/*
 * Driver: dr_console.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * Definition for debug console
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <inttypes.h>

/**
 * \brief Enables Debug Console
 *
 * \param baseAddr		address of UART module
 *
 * \return none
 */
void ConsoleEnable(uint32_t baseAddr);

/**
 * \brief Sends a log message
 *
 * \param sender		sender of message
 * \param message		message to send
 *
 * \return none
 */
void ConsoleLog(char sender[], char * message);

/**
 * \brief Sends a log message with variable amount of arguments
 *
 * \param sender	sender of message
 * \param message	message to send
 * \param ...		argument list
 */
void ConsoleLogf(char sender[], const char *message, ...);

#endif /* DEBUG_H_ */
