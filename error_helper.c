#include "error_helper.h"
#include <stdio.h>

Exception newException(int code, char message[], int fault_page) {
	Exception e;
	e.code = code;
	strcpy(e.message, message);
	e.fault_page = fault_page;
	return e;
}
