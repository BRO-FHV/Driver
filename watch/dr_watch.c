/*
 * Driver: watch.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 16, 2014
 * Description: 
 * Definition for System Watch
 */

#include <inttypes.h>
#include "dr_watch.h"

/**
 * \brief This function returns a Timespamp in Miliseconds
 */
uint64_t WatchCurrentTimeStamp(void) {
	return 1000999;
}

/**
 * \brief This function returns a formated TimeStamp
 */
char* WatchCurrentTimeStampString(void) {
	return "1000.999";
}
