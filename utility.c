#include "utility.h"

/*
 * Gets the instruction pointed by IP, to the argument
 * Return 0 on success
 * Returns -1 on error after setting IP to exception handler
 */
int getInstruction(char *instruction) {
	struct address translatedAddr;
	int len;
	bzero(instruction, WORD_SIZE * WORDS_PER_INSTR);
	if (getType(reg[IP_REG]) == TYPE_STR) {	
		raiseException(newException(EX_ILLMEM, "Illegal IP value. Not an address.\n", 0));
		return -1;
	}
	if (mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR) {	
		raiseException(newException(EX_ILLMEM, "Illegal PTLR value.\n", 0));
		return -1;
	}
	if (getInteger(reg[IP_REG]) < 0 || getInteger(reg[IP_REG]) + 1 >= SIZE_OF_MEM) {
		raiseException(newException(EX_ILLMEM, "IP Register value out of bounds.\n", 0));
		return -1;
	}
	if (mode == USER_MODE) {
		if (getInteger(reg[IP_REG]) < 0 || getInteger(reg[IP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {
			printf("%d", getInteger(reg[IP_REG]));
			raiseException(newException(EX_ILLOPERAND, "Illegal IP Access.\n", 0));
			return -1;
		}
	}
	translatedAddr = translate(getInteger(reg[IP_REG]));
	if (translatedAddr.page_no == -1 && translatedAddr.word_no == -1) return -1;
	strcpy(instruction, page[translatedAddr.page_no].word[translatedAddr.word_no]);
	translatedAddr = translate(getInteger(reg[IP_REG]) + 1);
	if (translatedAddr.page_no == -1 && translatedAddr.word_no == -1) return -1;
	len = strlen(instruction);
	instruction[len]=' ';
	instruction[len + 1]='\0';
	strcat(instruction, page[translatedAddr.page_no].word[translatedAddr.word_no]);
	return 0;
}

void emptyPage(int page_no) {
	int i;
	for(i = 0 ; i < PAGE_SIZE ; i++) {
		strcpy(page[page_no].word[i], "");
	}
}

struct address translate(int virtual_addr) {
	if (mode == USER_MODE) {
		struct address resultant_addr;
		int page_entry;
		resultant_addr.page_no = -1;
		resultant_addr.word_no = -1;
		if (getType(reg[PTBR_REG]) == TYPE_STR) {
			raiseException(newException(EX_ILLMEM, "Illegal Register Value.\n", 0));
			return resultant_addr;
		}
		page_entry = getInteger(reg[PTBR_REG]) + (virtual_addr / PAGE_SIZE) * 2;
		if (page[(page_entry+ 1 ) / PAGE_SIZE].word[(page_entry + 1) % PAGE_SIZE][1] == VALID ) { 
			resultant_addr.page_no = getInteger(page[page_entry / PAGE_SIZE].word[page_entry % PAGE_SIZE] );
			resultant_addr.word_no = virtual_addr % PAGE_SIZE;
			page[(page_entry + 1) / PAGE_SIZE].word[(page_entry + 1) % PAGE_SIZE][0] = REFERENCED;
		}
		else raiseException(newException(EX_PAGEFAULT, "Page Fault.\n", virtual_addr / PAGE_SIZE));
		return resultant_addr;
	} else {
		struct address resultant_addr;
		resultant_addr.page_no = virtual_addr / PAGE_SIZE;
		resultant_addr.word_no = virtual_addr % PAGE_SIZE;
		return resultant_addr;
	}
}

Address translateAddress(int address) {
	Address resultant_address = { -1, -1 };
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return resultant_address;
	}
	if (mode == USER_MODE) {
		int page_entry;
		page_entry = getInteger(reg[PTBR_REG]) + (address / PAGE_SIZE) * 2;
		if (page[(page_entry + 1) / PAGE_SIZE].word[(page_entry + 1) % PAGE_SIZE][1] == VALID) { 
			resultant_address.page = getInteger(page[page_entry / PAGE_SIZE].word[page_entry % PAGE_SIZE]);
			resultant_address.word = address % PAGE_SIZE;
			page[(page_entry + 1) / PAGE_SIZE].word[(page_entry + 1) % PAGE_SIZE][0] = REFERENCED;
		} else raiseException(newException(EX_PAGEFAULT, "Page Fault.\n", address / PAGE_SIZE));
		return resultant_address;
	} else {
		resultant_address.page = address / PAGE_SIZE;
		resultant_address.word = address % PAGE_SIZE;
		return resultant_address;
	}
}

int getInteger(char * str) {
	return atoi(str);
}

void storeInteger(char * str, int num) {
	sprintf(str, "%d", num);
}

int getType(char * str) {
	int i = 0;
	if (str[i] == '+' || str[i] == '-') i++;
	for (; str[i] != '\0'; i++)
		if (!(str[i] >= '0' && str[i] <= '9')) return TYPE_STR;
	return TYPE_INT;
}

void raiseException(Exception e) {
	if (mode == KERNEL_MODE) {
		printf("<ERROR:%d:%s> %s\n",getInteger(reg[IP_REG]), instruction, e.message);
		if (isDebugModeOn()) debugInterface();
		exit(0);
	} else {
		int ex_flag;
		ex_flag = getInteger(reg[IP_REG]) * 1000;
		ex_flag += e.fault_page * 10;
		ex_flag += e.code;
		mode = KERNEL_MODE;
		storeInteger(reg[EMA_REG], e.fault_page);//TODO Change e.fault_page to memory address
		storeInteger(reg[EIP_REG], getInteger(reg[IP_REG]));
		storeInteger(reg[EC_REG], e.code);
		storeInteger(reg[EPN_REG], e.fault_page);
		storeInteger(reg[IP_REG], EXCEPTION_HANDLER * PAGE_SIZE);
	}
}

int isSafeState() {
	if (getType(reg[SP_REG]) == TYPE_STR || getType(reg[BP_REG]) == TYPE_STR || getType(reg[PTLR_REG]) == TYPE_STR) {
		raiseException(newException(EX_ILLMEM, "Illegal Register Value.\n", 0));
		return 0;
	}
	if (getInteger(reg[PTLR_REG]) < 0 || getInteger(reg[PTLR_REG]) >= SIZE_OF_MEM) {
		raiseException(newException(EX_ILLMEM, "Illegal address access.\n PTLR value is out of bounds.\n", 0));
		return 0;
	}
	if (getInteger(reg[SP_REG]) + 1 < 0) {
		raiseException(newException(EX_ILLMEM, "Stack underflow.\n", 0));
		return 0;
	}
	if (getInteger(reg[SP_REG]) >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)) {
		raiseException(newException(EX_ILLMEM, "Stack overflow.\n", 0));
		return 0;
	}
	if (getInteger(reg[BP_REG]) < 0) {
		raiseException(newException(EX_ILLMEM, "Negative value for BP.\n", 0));
		return 0;
	}
	if (getInteger(reg[BP_REG]) >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[BP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)) {
		raiseException(newException(EX_ILLMEM, "BP Register Value out of bounds.\n", 0));
		return 0;
	}
	return 1;
}


Exception isRegisterInaccessible(int r) {
	if (r < 0 || r > (NO_USER_REG + NO_PORTS + NO_SPECIAL_REG)) {
		return newException(EX_ILLOPERAND, "Invalid Register Provided", 0);
	}
	//	User Register
	if (r >= R0 && r < R0 + NO_USER_REG) return newException(EX_NONE, "", 0);
	//	System Register
	if (r >= P0 && r < P0 + NO_PORTS) {
		if (mode == KERNEL_MODE) return newException(EX_NONE, "", 0);;
		return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode", 0);
	}
	//	Special Register
	if (mode == KERNEL_MODE) return newException(EX_NONE, "", 0); 
	if (r == SP_REG || r == BP_REG) return newException(EX_NONE, "", 0);
	if (r == PTBR_REG) return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode -> {PTBR}", 0);
	if (r == PTLR_REG) return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode -> {PTLR}", 0);
	if (r == IP_REG) return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode -> {IP}", 0);
	if (r == EIP_REG) return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode -> {EIP}", 0);
	if (r == EC_REG) return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode -> {EC}", 0);
	if (r == EPN_REG) return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode -> {EPN}", 0);
	if (r == EMA_REG) return newException(EX_ILLOPERAND, "Illegal Register Access in User Mode -> {EMA}", 0);
}

Exception isSafeState2() {
	if (getType(reg[SP_REG]) == TYPE_STR || getType(reg[BP_REG]) == TYPE_STR || getType(reg[PTLR_REG]) == TYPE_STR
		|| getType(reg[PTBR_REG]) == TYPE_STR) {
		return newException(EX_ILLMEM, "Illegal Register value", 0);
	}
	if (getInteger(reg[PTBR_REG]) < 0 || getInteger(reg[PTBR_REG]) >= SIZE_OF_MEM) {
		return newException(EX_ILLMEM, "Illegal address access.\n PTBR value is out of bounds.", 0);
	}
	if (getInteger(reg[PTLR_REG]) < 0 || getInteger(reg[PTLR_REG]) >= SIZE_OF_MEM) {
		return newException(EX_ILLMEM, "Illegal address access.\n PTLR value is out of bounds.", 0);
	}
	if (getInteger(reg[SP_REG]) + 1 < 0) {
		return newException(EX_ILLMEM, "Stack underflow\n", 0);
	}
	if (getInteger(reg[SP_REG]) >= SIZE_OF_MEM || (mode == USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)) {
		return newException(EX_ILLMEM, "Stack overflow\n", 0);
	}
	if (getInteger(reg[BP_REG]) < 0) {
		return newException(EX_ILLMEM, "Negative Value for BP Register\n", 0);
	}
	if (getInteger(reg[BP_REG]) >= SIZE_OF_MEM || (mode == USER_MODE && getInteger(reg[BP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)) {
		return newException(EX_ILLMEM, "BP Register Value out of bounds\n", 0);
	}
	return newException(EX_NONE, "", 0);	
}

Exception isRestrictedMemoryLocation(int x) {
	if (x < 0 || x >= SIZE_OF_MEM || (mode == USER_MODE && x >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)) {
		return newException(EX_ILLMEM, "Illegal Memory Access", 0);
	}
	return newException(EX_NONE, "", 0);
}

char * getWordFromAddress(Address x) {
	if (x.page == -1 && x.word == -1) return NULL;
	return page[x.page].word[x.word];	
}

char * getWordFromMemoryLocation(int r) {
	return getWordFromAddress(translateAddress(r));
}

int storeWordToAddress(Address x, char * word) {
	if (x.page == -1 && x.word == -1) return 0;
	strcpy(page[x.page].word[x.word], word);
	return 1;
}

int storeWordToMemoryLocation(int r, char * word) {
	return storeWordToAddress(translateAddress(r), word);
}


char * resolveOperand(int r, int flag1, int flag2, char * string) {
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return NULL;
	}
	if (flag1 != NUM && flag1 != STRING && flag1 != MEM_DIR) {
		if (flag1 != MEM_DIR_IN) {
			int reg = (flag2 != ILLREG) ? flag2 : r;
			e = isRegisterInaccessible(reg);
			if (e.code != EX_NONE) {
				raiseException(e);
				return NULL;
			}
		}
	}
	char * result;
	Address translatedAddress;
	switch (flag1) {
		case REG:
		case SP:
		case BP:
		case IP:
		case PTBR:
		case PTLR:
		case EFR:
			return reg[r];
			break;
		case NUM:
			result = malloc(WORD_SIZE);
			sprintf(result, "%d", r);
			return result;
			break;
		case STRING:
			return string;
			break;
		case MEM_REG:
			if (getType(reg[r]) == TYPE_STR) {
				raiseException(newException(EX_ILLOPERAND, "String value supplied in lieu of Memory Address", 0));
				return NULL;
			}
			e = isRestrictedMemoryLocation(getInteger(reg[r]));
			if (e.code != EX_NONE) {
				raiseException(e);
				return NULL;
			}
			return getWordFromMemoryLocation(getInteger(reg[r]));
			break;
		case MEM_SP:
		case MEM_BP:
			return getWordFromMemoryLocation(getInteger(reg[r]));
			break;
		case MEM_PTBR:
		case MEM_PTLR:
			return getWordFromMemoryLocation(getInteger(reg[r]));
			break;
		case MEM_IP:
		case MEM_DIR_IP:		
			raiseException(newException(EX_ILLOPERAND, "Cannot use memory reference with IP in any mode", 0));
			return NULL;
			break;
		case MEM_EFR:
		case MEM_DIR_EFR:
			raiseException(newException(EX_ILLOPERAND, "Cannot use memory reference with EFR in any mode", 0));
			return NULL;
			break;
		case MEM_DIR:
			return getWordFromMemoryLocation(r);
			break;
		case MEM_DIR_REG:
			if (getType(reg[flag2]) == TYPE_STR) {
				raiseException(newException(EX_ILLOPERAND, "Illegal register value", 0));
				return NULL;
			}
			r += getInteger(reg[flag2]);
			e = isRestrictedMemoryLocation(r);
			if (e.code != EX_NONE) {
				raiseException(e);
				return NULL;
			}
			return getWordFromMemoryLocation(r);
			break;
		case MEM_DIR_SP:
		case MEM_DIR_BP:
		case MEM_DIR_PTBR:
		case MEM_DIR_PTLR:
			r += getInteger(reg[flag2]);
			e = isRestrictedMemoryLocation(r);
			if (e.code != EX_NONE) {
				raiseException(e);
				return NULL;
			}
			return getWordFromMemoryLocation(r);
			break;
		case MEM_DIR_IN:
			r += flag2;
			e = isRestrictedMemoryLocation(r);
			if (e.code != EX_NONE) {
				raiseException(e);
				return NULL;
			}
			return getWordFromMemoryLocation(r);
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal Source Operand", 0));
			return NULL;
			break;
	}
}

int storeValue(int r, int flag1, int flag2, char * value) {
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	if (flag1 != NUM && flag1 != STRING && flag1 != MEM_DIR) {
		if (flag1 != MEM_DIR_IN) {
			int reg = (flag2 != ILLREG) ? flag2 : r;
			e = isRegisterInaccessible(reg);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
		}
	}
	switch (flag1) {
		case REG:
		case SP:
		case BP: 
		case PTBR:
		case PTLR:
			strcpy(reg[r], value);
			return 1;
			break;
		case IP:
			raiseException(newException(EX_ILLOPERAND, "Cannot alter read-only register IP", 0));
			return 0;
			break;
		case EFR:
			raiseException(newException(EX_ILLOPERAND, "Cannot alter read-only register EFR", 0));
			return 0;					
			break;
		case MEM_REG:
			if (getType(reg[r]) == TYPE_STR) {
				raiseException(newException(EX_ILLOPERAND, "String value supplied in lieu of Memory Address", 0));
				return 0;
			}
			e = isRestrictedMemoryLocation(getInteger(reg[r]));
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			return storeWordToMemoryLocation(getInteger(reg[r]), value);
			break;
		case MEM_SP:
		case MEM_BP:
		case MEM_PTBR:
		case MEM_PTLR:
			return storeWordToMemoryLocation(getInteger(reg[r]), value);
			break;
		case MEM_IP:
		case MEM_DIR_IP:
			raiseException(newException(EX_ILLOPERAND, "Cannot use memory reference with IP in any mode", 0));
			return 0;
			break;
		case MEM_EFR:
		case MEM_DIR_EFR:						
			raiseException(newException(EX_ILLOPERAND, "Cannot use memory reference with EFR in any mode", 0));
			return 0;
			break;
		case MEM_DIR:
			e = isRestrictedMemoryLocation(r);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			return storeWordToMemoryLocation(r, value);
			break;
		case MEM_DIR_REG:
			if (getType(reg[flag2]) == TYPE_STR) {
				raiseException(newException(EX_ILLOPERAND, "Illegal register value", 0));
				return 0;
			}
			r += getInteger(reg[flag2]);
			e = isRestrictedMemoryLocation(r);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			return storeWordToMemoryLocation(r, value);
			break;
		case MEM_DIR_SP:
		case MEM_DIR_BP:
		case MEM_DIR_PTBR:
		case MEM_DIR_PTLR:
			r += getInteger(reg[flag2]);
			e = isRestrictedMemoryLocation(r);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			return storeWordToMemoryLocation(r, value);
			break;
		case MEM_DIR_IN:
			r += flag2;
			e = isRestrictedMemoryLocation(r);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			return storeWordToMemoryLocation(r, value);
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal Target Operand", 0));
			return 0;
			break;
	}
}

int performArithmetic(int X, int flagX, int Y, int flagY, int operation) {
	int valueX, valueY;
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	switch (flagX) {
		case REG:
		case SP:
		case BP:
		case PTBR:
		case PTLR:
			e = isRegisterInaccessible(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			if (getType(reg[X]) == TYPE_STR) {
				raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
				return 0;
			}
			valueX = X;
			break;
		case IP:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand IP. Cannot alter readonly register", 0));
			return 0;
			break;
		case EFR:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand EFR. Cannot alter readonly register", 0));
			return 0;
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
			return 0;
			break;
	}

	if (operation != INR && operation != DCR) {
		switch (flagY) {
			case REG:
			case SP:
			case BP:
			case PTBR:
			case PTLR:
				e = isRegisterInaccessible(Y);
				if (e.code != EX_NONE) {
					raiseException(e);
					return 0;
				}
				if (getType(reg[Y]) == TYPE_STR) {
					raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
					return 0;
				}
				valueY = getInteger(reg[Y]);
				break;
			case NUM:
				valueY = Y;
				break;
			case IP:
				raiseException(newException(EX_ILLOPERAND, "Illegal operand IP. Cannot alter readonly register", 0));
				return 0;
				break;
			case EFR:
				raiseException(newException(EX_ILLOPERAND, "Illegal operand EFR. Cannot alter readonly register", 0));
				return 0;
				break;
			default:
				raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
				return 0;
				break;
		}
	}

	switch (operation) {
		case ADD:
			storeInteger(reg[valueX], getInteger(reg[valueX]) + valueY);
			break;			
		case SUB:
			storeInteger(reg[valueX], getInteger(reg[valueX]) - valueY);
			break;
		case MUL:
			storeInteger(reg[valueX], getInteger(reg[valueX]) * valueY);
			break;
		case DIV:
			if (valueY == 0) {
				raiseException(newException(EX_ILLOPERAND, "Divide by ZERO", 0));
				return 0;
			}
			storeInteger(reg[valueX], getInteger(reg[valueX]) / valueY);
			break;
		case MOD:
			if (valueY == 0) {
				raiseException(newException(EX_ILLOPERAND, "Divide by ZERO", 0));
				return 0;
			}
			storeInteger(reg[valueX], getInteger(reg[valueX]) % valueY);
			break;
		case INR:
			storeInteger(reg[valueX], getInteger(reg[valueX]) + 1);
			break;
		case DCR:
			storeInteger(reg[valueX], getInteger(reg[valueX]) - 1);
			break;
		default:
			raiseException(newException(EX_ILLINSTR, "Illegal Instruction", 0));
			return 0;
			break;
	}
	return 1;
}

int performMOV(int X, int flagX1, int flagX2, int Y, int flagY1, int flagY2, char * value) {
	char * resolvedValue = resolveOperand(Y, flagY1, flagY2, value);
	if (resolvedValue == NULL) return 0;
	if ((flagY1 == MEM_DIR_REG || flagY1 == MEM_DIR_SP || flagY1 == MEM_DIR_BP || flagY1 == MEM_DIR_PTBR || flagY1 == MEM_DIR_PTLR
		|| flagY1 == MEM_DIR_IN) && (flagX1 == MEM_DIR_REG || flagX1 == MEM_DIR_SP || flagX1 == MEM_DIR_BP || flagX1 == MEM_DIR_PTBR
		|| flagX1 == MEM_DIR_PTLR || flagX1 == MEM_DIR_IN)) {
		raiseException(newException(EX_ILLOPERAND, "Illegal Operands", 0));
		return 0;
	}
	return (storeValue(X, flagX1, flagX2, resolvedValue));
}

int performLogic(int X, int flagX, int Y, int flagY, int operation) {
	int valueX, valueY;
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}

	switch (flagX) {
		case REG:
		case SP:
		case BP:
		case PTBR:
		case PTLR:
			e = isRegisterInaccessible(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			break;
		case IP:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand IP. Cannot alter readonly register", 0));
			return 0;
			break;
		case EFR:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand EFR. Cannot alter readonly register", 0));
			return 0;
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
			return 0;
			break;
	}

	switch (flagY) {
		case REG:
		case SP:
		case BP:
		case PTBR:
		case PTLR:
			e = isRegisterInaccessible(Y);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			break;
		case IP:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand IP. Cannot alter readonly register", 0));
			return 0;
			break;
		case EFR:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand EFR. Cannot alter readonly register", 0));
			return 0;
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
			return 0;
			break;
	}
	if (getType(reg[X]) != getType(reg[Y])) {
		raiseException(newException(EX_ILLOPERAND, "Operand Type Mismatch to logical expression", 0));
		return 0;
	}
	switch (operation) {
		case LT:
			valueX = (getType(reg[X]) == TYPE_INT) ? (getInteger(reg[X]) < getInteger(reg[Y])) : (strcmp(reg[X], reg[Y]) < 0);
			break;
		case GT:
			valueX = (getType(reg[X]) == TYPE_INT) ? (getInteger(reg[X]) > getInteger(reg[Y])) : (strcmp(reg[X], reg[Y]) > 0);
			break;
		case EQ:
			valueX = (getType(reg[X]) == TYPE_INT) ? (getInteger(reg[X]) == getInteger(reg[Y])) : (!strcmp(reg[X], reg[Y]));
			break;
		case NE:
			valueX = (getType(reg[X]) == TYPE_INT) ? (getInteger(reg[X]) != getInteger(reg[Y])) : (!!strcmp(reg[X], reg[Y]));
			break;
		case LE:
			valueX = (getType(reg[X]) == TYPE_INT) ? (getInteger(reg[X]) <= getInteger(reg[Y])) : (strcmp(reg[X], reg[Y]) >= 0);
			break;
		case GE:
			valueX = (getType(reg[X]) == TYPE_INT) ? (getInteger(reg[X]) >= getInteger(reg[Y])) : (strcmp(reg[X], reg[Y]) >= 0);
			break;
		default:
			raiseException(newException(EX_ILLINSTR, "Illegal Instruction", 0));
			return;
			break;
	}
	storeInteger(reg[X], valueX);
	return 1;
}

int performBranching(int X, int flagX, int Y, int flagY, int operation) {
	int valueX, valueY;
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	switch (operation) {
		case JZ:
		case JNZ:
			switch (flagX) {
				case REG:
				case SP:
				case BP:
				case IP:
				case PTBR:
				case PTLR:
				case EFR:
					e = isRegisterInaccessible(X);
					if (e.code != EX_NONE) {
						raiseException(e);
						return 0;
					}
					break;					
				default:
					raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
					return 0;
					break;
			}
			if (flagY != NUM) {
				raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
				return 0;
			}
			e = isRestrictedMemoryLocation(Y);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			if ((getType(reg[X]) == TYPE_INT && operation == JZ && getInteger(reg[X]) == 0) 
				|| ((getType(reg[X]) == TYPE_STR) || (operation == JNZ && getInteger(reg[X])))) storeInteger(reg[IP_REG], Y);
			else storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
			break;
		case JMP:
			if (flagX != NUM) {
				raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
				return 0;
			}		
			e = isRestrictedMemoryLocation(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			storeInteger(reg[IP_REG], X);
			break;
		default:
			raiseException(newException(EX_ILLINSTR, "Illegal Instruction", 0));
			return 0;
			break;		
	}
	return 1;
}

int performPush(int X, int flagX) {
	Address T = translateAddress(getInteger(reg[SP_REG]) + 1);
	if (T.page == -1 && T.word == -1) return 0;
	Exception e;

	switch (flagX) {
		case REG:
		case SP:
		case BP:
		case IP:
		case PTBR:
		case PTLR:
		case EFR:
			e = isRegisterInaccessible(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			if (storeWordToAddress(T, reg[X])) {
				storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
				return 1;
			} else return 0;
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal Operand", 0));
			return 0;
			break;
	}
}

int performPop(int X, int flagX) {
	char * value;
	Exception e;
	Address T = translateAddress(getInteger(reg[SP_REG]));
	if (T.page == -1 && T.word == -1) return 0;
	switch (flagX) {
		case REG:
			e = isRegisterInaccessible(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}				
			value = getWordFromAddress(T);
			strcpy(reg[X], value);
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);
			return 1;
			break;
		case IP:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand IP. Cannot alter readonly register", 0));
			return 0;
			break;
		case EFR:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand EFR. Cannot alter readonly register", 0));
			return 0;
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal Operand", 0));
			return 0;
			break;
	}
}

int performCall(int X, int flagX) {
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	if (flagX != NUM) {
		raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
		return 0;
	}		
	e = isRestrictedMemoryLocation(X);
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	Address	T = translateAddress(getInteger(reg[SP_REG]) + 1);
	if (T.page == -1 && T.word == -1) return;
	storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
	storeInteger(page[T.page].word[T.word], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
	storeInteger(reg[IP_REG], X);
	return 1;
}

int performRet() {
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	Address T = translateAddress(getInteger(reg[SP_REG]));
	if (T.page == -1 && T.word == -1) return;
	char * value = getWordFromAddress(T);
	if (value == NULL || getType(value) == TYPE_STR) {
		raiseException(newException(EX_ILLMEM, "Illegal return address", 0));
		return 0;
	}
	int result = getInteger(value);
	e = isRestrictedMemoryLocation(result);
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	storeInteger(reg[IP_REG], result);
	storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);
	return 1;
}

int performINT(int X, int flagX) {
	if (mode == KERNEL_MODE) {
		raiseException(newException(EX_ILLINSTR, "Cannot call INT in KERNEL Mode", 0));
		return 0;
	}
	if (flagX != NUM) {
		raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
		return;
	}
	invokeInterrupt(X);
	return 1;
}

int performIRET() {
	if (mode == USER_MODE) {
		raiseException(newException(EX_ILLINSTR, "Call to Privileged Instruction IRET in USER mode", 0));
		return 0;
	}
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	mode = USER_MODE;
	Address T = translateAddress(getInteger(reg[SP_REG]));
	if (T.page == -1 && T.word == -1) {
		mode = KERNEL_MODE;
		return 0;
	}
	char * value = getWordFromAddress(T);
	if (getType(value) == TYPE_STR) {
		mode = KERNEL_MODE;
		raiseException(newException(EX_ILLMEM, "Illegal return address", 0));
		return 0;
	}
	int result = getInteger(value);
	if (result < 0 || result >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {
		mode = KERNEL_MODE;
		raiseException(newException(EX_ILLMEM, "Illegal return address", 0));
		return 0;
	}
	storeInteger(reg[IP_REG], result);
	storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);
	return 1;
}

int performIN(int X, int flagX) {
	int value;
	Exception e;
	switch (flagX) {
		case REG:
		case SP:
		case BP:
		case PTBR:
		case PTLR:
			e = isRegisterInaccessible(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			break;
		case IP:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand IP. Cannot alter readonly register", 0));
			return 0;
		case EFR:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand EFR. Cannot alter readonly register", 0));
			return 0;
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand.", 0));
			return 0;
			break;
	}
	char input[WORD_SIZE];
	scanf("%s", input);
	FLUSH_STDIN(input);
	input[WORD_SIZE - 1] = '\0';
	strcpy(reg[X], input);
	return 1;
}

int performOUT(int X, int flagX) {
	Exception e;
	switch (flagX) {
		case REG:
		case SP:
		case BP:
		case IP:
		case PTBR:
		case PTLR:
		case EFR:
			e = isRegisterInaccessible(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand.", 0));
			return 0;
	}
	printf("%s\n", reg[X]);
	fflush(stdout);
	return 1;
}

int performLoadStore(int X, int flagX, int Y, int flagY, int instruction) {
	if (mode == USER_MODE) {
		raiseException(newException(EX_ILLINSTR, "Call to Privileged Instruction in USER mode", 0));		
		return 0;
	}
	Exception e = isSafeState2();
	if (e.code != EX_NONE) {
		raiseException(e);
		return 0;
	}
	switch (flagX) {
		case REG:
		case SP:
		case BP:
		case IP:
		case PTBR:
		case PTLR:
		case EFR:
			e = isRegisterInaccessible(X);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			if (getType(reg[X]) == TYPE_STR) {
				raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
				return 0;
			} else X = getInteger(reg[X]);					
			break;
		case NUM:
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
			return 0;
			break;
	}
	switch (flagY) {
		case REG:
		case SP:
		case BP:
		case IP:
		case PTBR:
		case PTLR:
		case EFR:
			e = isRegisterInaccessible(Y);
			if (e.code != EX_NONE) {
				raiseException(e);
				return 0;
			}
			if (getType(reg[Y]) == TYPE_STR) {
				raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
				return 0;
			} else Y = getInteger(reg[Y]);					
			break;
		case NUM:
			break;
		default:
			raiseException(newException(EX_ILLOPERAND, "Illegal operand", 0));
			return 0;
			break;
	}
	if (instruction == LOAD) {			
		emptyPage(X);
		readFromDisk(X, Y);
	} else if (instruction == STORE) writeToDisk(Y, X);
	return 1;
}