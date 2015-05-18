#include "error_helper.h"
#include <stdio.h>

Exception newException(int code, char message[], int fault) {
	Exception e;
	e.code = code;
	strcpy(e.message, message);
	e.fault = fault;
	return e;
}
