/*
 * Driver: dr_sd.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 11.06.2014
 * Description:
 * TODO
 */

#include <inttypes.h>
#include "thirdParty/fatfs/src/integer.h"
#ifndef DR_SD_H_
#define DR_SD_H_

int startFileSystem(void);
void  getElfFile(uint8_t * dataBuf,DWORD size ,const char * path);

#endif /* DR_SD_H_ */



