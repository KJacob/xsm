#include "simulator.h"

int parseArgument(char * arg) {
	if (strcmp(arg, "--debug") == 0 || strcmp(arg, "-d") == 0) {
		enableDebugMode();
		return 0;
	} else {
		char * flag_name = strtok(arg, "=");
		char * flag_value = strtok(NULL, "=");
		int flag_intValue;
		if (strcmp(flag_name, "--timer") == 0 || strcmp(flag_name, "-t") == 0) {
			if (flag_value != NULL) flag_intValue = getInteger(flag_value);
			if (flag_intValue >= 1 && flag_intValue <= 1024) {
				enableTimer();
				setTimeSlice(flag_intValue);
				return 0;
			} else if (flag_intValue != 0) {
				printf("Invalid arguement %d to timer flag. Timer value should be between 0 and 1024\n", flag_intValue);
				return -1;
			} else {
				disableTimer();
				return 0;
			}
		} else {
			printf("Invalid arguement %s", arg);
			return -1;
		}
	}
}

int main(int argc, char **argv) {
	initializeDebug();
	int i = 1;
	while (i < argc) {
		if (parseArgument(argv[i++]) == -1) exit(0);
	}
	initializeRegisters();
	run(isTimerEnabled());
}



/*
 *	This function does the following:
 *		1. Loads OS Startup Code.
 *		2. Copies the instruction to be parsed as per the address specified by the IP register.
 *		3. Checks whether interrupt is disabled. If not th clock ticks.
 *		4. Begins the lexical analysis by getting the first token and passing it as arguement to executeOneInstruction.
 *		5. If step flag is enabled enters debug mode
 *		6. Finally checks if time slice allocated is over or not. If yes and mode is user mode ,and if interrupt is enabled then
 *			INT 0 code is run.
 */
void run(int is_timer_enabled) {
	if (is_timer_enabled) resetTimer();
	loadStartupCode();
	int instr;
	while (1) {
		YY_FLUSH_BUFFER;
		if (getInstruction(instruction) == -1) continue;	//gets the next instruction in variable instruction		
		instr = yylex();
		if (mode == USER_MODE && is_timer_enabled) timerTick();
		executeOneInstruction(instr);
		if ((watch_count > 0 && checkWatch() == 1) || step_flag == 1) debugInterface();
		if (isTimerCountZero() && is_timer_enabled && mode == USER_MODE) {
			resetTimer();
			invokeHardwareInterrupt(0);
			if (step_flag == 1) printf("TIMER Interrupt\n");
		}
	}
}



/*
 *	This code is used to execute each instruction.
 */
void executeOneInstruction(int instr) {
	int X, Y, flagX1, flagX2, flagY1, flagY2, operation;
	char string[WORD_SIZE];
	bzero(string, 16);
	switch (instr) {
		case START:
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);	//increment IP
			break;
		case MOV:						//1st phase:get the value		2nd phase:store the value
			X = yylex();
			flagX1 = yylval.flag;
			flagX2 = yylval.flag2;
			Y = yylex();
			flagY1 = yylval.flag;
			flagY2 = yylval.flag2;
			strcpy(string, yylval.data);
			if (!performMOV(X, flagX1, flagX2, Y, flagY1, flagY2, string)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
		break;
		case ARITH:
			operation = yylval.flag;
			X = yylex();
			flagX1 = yylval.flag;
			if (operation != INR && operation != DCR) {
				Y = yylex();
				flagY1 = yylval.flag;
			}
			if (!performArithmetic(X, flagX1, Y, flagY1, operation)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
		break;
		case LOGIC:
			operation = yylval.flag;
			X = yylex();
			flagX1 = yylval.flag;
			Y = yylex();
			flagY1 = yylval.flag;
			if (!performLogic(X, flagX1, Y, flagY1, operation)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
		break;
		case BRANCH:
			operation = yylval.flag;
			X = yylex();
			flagX1 = yylval.flag;
			if (operation != JMP) {
				Y = yylex();
				flagY1 = yylval.flag;				
			}
			if (!performBranching(X, flagX1, Y, flagY1, operation)) return;
			break;
								
		case PUSH:
			X = yylex();
			flagX1 = yylval.flag;
			if (!performPush(X, flagX1)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
			break;
			
		case POP:
			X = yylex();
			flagX1 = yylval.flag;
			if (!performPop(X, flagX1)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
			break;

		case CALL:
			X = yylex();
			flagX1 = yylval.flag;
			if (!performCall(X, flagX1)) return;
			break;

		case RET:
			if (!performRet()) return;
			break;

		case INT:
			X = yylex();
			flagX1 = yylval.flag;
			if (!performINT(X, flagX1)) return;
			break;

		case IRET:
			if (!performIRET()) return;
			break;

		case IN:
			X = yylex();
			flagX1 = yylval.flag;
			if (!performIN(X, flagX1)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
			break;
		
		case OUT:
			X = yylex();
			flagX1 = yylval.flag;
			if (!performOUT(X, flagX1)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
			break;

		case LOAD:
		case STORE:
			X = yylex();
			flagX1 = yylval.flag;
			Y = yylex();
			flagY1 = yylval.flag;
			if (!performLoadStore(X, flagX1, Y, flagY1, instr)) return;
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
			break;
				
		case HALT:
			if (mode == USER_MODE) {
				raiseException(newException(EX_ILLINSTR, "Call to Privileged Instruction HALT in USER mode", 0));
				return;
			}
			printf("Machine is halting\n");
			exit(0);
			break;
		case END:
			break;
		
		case BRKP:
			if (isDebugModeOn()) {
				step_flag = 1;
				printf("\nXSM Debug Environment\nType \"help\" for getting a list of commands\n");
			}
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + WORDS_PER_INSTR);
			break;
		default:
			raiseException(newException(EX_ILLINSTR, "Illegal Instruction", 0));
			return;
	}
}
