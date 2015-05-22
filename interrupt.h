#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "disk.h"
#include "data.h"
#include "utility.h"
#include <stdlib.h>

#define NUM_INT				19
#define PAGE_PER_INT		2

#define INT0 				2	//EXHandler

#define INT_START_PAGE					2
#define SOFTWARE_INT_START_PAGE			10

#define EXCEPTION_HANDLER				2
#define TIMER_INTERRUPT_HANDLER			4

int isSafeToInvokeInterrupt(int);
void invokeHardwareInterrupt(int);
void invokeSoftwareInterrupt(int);

#endif
