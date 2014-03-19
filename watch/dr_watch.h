/*
 * Driver: watch.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 16, 2014
 * Description: 
 * Implementation for System Watch
 */

#ifndef WATCH_H_
#define WATCH_H_

#include <inttypes.h>

/**
 * \brief This function returns a Timespamp in Miliseconds
 *
 * \return Timestamp in Miliseconds
 */
uint64_t WatchCurrentTimeStamp(void);

/**
 * \brief This function returns a formated TimeStamp
 *
 * \return Formated Timestamp
 */
char* WatchCurrentTimeStampString(void);

#endif /* WATCH_H_ */
