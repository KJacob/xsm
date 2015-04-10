#include "interrupt.h"
#include <stdio.h>

/*
 *	This function runs the scheduler which is the INT 0 code.
*/

int isSafeToInvokeInterrupt(int n) {
	if (n < 0 || n >= NUM_INT) {
		raiseException(newException(EX_ILLINSTR, "Illegal Interrupt Call.\n", 0));
		return 0;
	}
	if (!isSafeState()) return 0;
	if (mode == KERNEL_MODE) {
		if (n == 0) return 1;
		raiseException(newException(EX_ILLINSTR, "Illegal Interrupt Call from Kernel Mode.\n", 0));
		return 0;
	}
	return 1;
}

void invokeInterrupt(int n) {
	if (!isSafeToInvokeInterrupt(n)) return;
	if (n > 0 && n < NUM_INT) invokeSoftwareInterrupt(n);
	else invokeHardwareInterrupt(n);
}

void invokeHardwareInterrupt(int n) {
	struct address translated_addr;
	translated_addr = translate(getInteger(reg[SP_REG])+1);
	if (translated_addr.page_no == -1 && translated_addr.word_no == -1) return;
	storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
	storeInteger(page[translated_addr.page_no].word[translated_addr.word_no], getInteger(reg[IP_REG]));
	mode = KERNEL_MODE;
	storeInteger(reg[IP_REG], (n * PAGE_PER_INT + INT_START_PAGE) * PAGE_SIZE);
}

void invokeSoftwareInterrupt(int n) {
	struct address translated_addr;
	translated_addr = translate(getInteger(reg[SP_REG]) + 1);
	if (translated_addr.page_no == -1 && translated_addr.word_no == -1) return;
	storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
	storeInteger(page[translated_addr.page_no].word[translated_addr.word_no], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
	mode = KERNEL_MODE;
	storeInteger(reg[IP_REG], (n * PAGE_PER_INT + INT_START_PAGE) * PAGE_SIZE);
}