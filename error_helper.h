#ifndef ERROR_HELPER_H
#define ERROR_HELPER_H
#include <string.h>
struct Exception {
	int code;
	char message[150];
	int fault;
};

typedef struct Exception Exception;

Exception newException(int code, char message[], int fault_page);
#endif