/*
 * Driver: debug.h
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
void DebugEnable(uint32_t baseAddr);

/**
 * \brief Sends a log message
 *
 * \param sender		sender of message
 * \param message		message to send
 *
 * \return none
 */
void DebugLog(char sender[], char * message);

#endif /* DEBUG_H_ */
