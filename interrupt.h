#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "disk.h"
#include "data.h"
#include "utility.h"
#include <stdlib.h>

#define NUM_INT				8
#define PAGE_PER_INT		2

#define INT0 				9	//TIMER
#define INT1 				11
#define INT2 				13
#define INT3 				15
#define INT4 				17
#define INT5 				19
#define INT6 				21
#define INT7 				23

#define INT_START_PAGE					9
#define SOFTWARE_INT_START_PAGE			11

#define EXCEPTION_HANDLER				7
#define TIMER_INTERRUPT_HANDLER			9

int isSafeToInvokeInterrupt(int);
void invokeHardwareInterrupt(int);
void invokeSoftwareInterrupt(int);

#endif
