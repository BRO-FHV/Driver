/*
 * Driver: debug.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * TODO
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <inttypes.h>

/**
 * \brief Enables Debug Console
 */
void DebugEnable();

/**
 * \brief Sends a log message
 *
 * \param sender		sender of message
 * \param message
 */
void DebugLog(uint32_t sender, char message[]);

#endif /* DEBUG_H_ */
