/*
 * context.h
 *
 *  Created on: 20.05.2013
 *      Author: Daniel
 */

#ifndef CONTEXT_OMAP3530_H_
#define CONTEXT_OMAP3530_H_

//
// TODO: not very portable, those values are matching the CMD file for client apps

#define PROCESS_CODE_START 0x00020000

#define PROCESS_STACK_START 0x10000000
#define PROCESS_STACK_SIZE 128000

#define PROCESS_SYSMEM_START 0x10002000
#define PROCESS_SYSMEM_SIZE 0x00004000

#define PROCESS_CONTEXT_SIZE 17
#define DECLARE_PROCESS_CONTEXT(name) volatile void* name[PROCESS_CONTEXT_SIZE];

#endif /* CONTEXT_OMAP3530_H_ */
