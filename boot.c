#include "boot.h"


/*
 *	Load the OS Startup code from the disk to proper location in memory.
 *
 *	Steps :	1.	Clear OS Startup Page
 *			2.	Read OS Startup Code from Disk to Page
 *			3.	Set IP to OS Startup Code
 */

void loadStartupCode() {
	mode = KERNEL_MODE;
	emptyPage(OS_STARTUP_CODE_PAGENO);
	readFromDisk(OS_STARTUP_CODE_PAGENO, BOOT_BLOCK);
	storeInteger(reg[IP_REG], OS_STARTUP_CODE_PAGENO * PAGE_SIZE);
}


/*
 *	Initialize all the registers to zero
 */
void initializeRegisters() {
	int i;
	for (i = 0; i < NUM_REGS; i++) storeInteger(reg[i], 0);
}
