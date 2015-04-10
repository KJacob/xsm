#ifndef BOOT_H

#define BOOT_H
#include "memory_constants.h"
#include "disk.h"
#include "data.h"

/*
 *	Load the OS Startup code from the disk to proper location in memory.
 */
void loadStartupCode();

/*
 *	Initialize all the registers to zero
 */
void initializeRegisters();

#endif