#include "debug.h"
#include "data.h"
#include "memory_constants.h"
#include "interrupt.h"

/* 
Function to invoke Command Line interface 
*/
void debug_interface()	
{
	char command[100], c;
	int i,j;
	printf("Last Instruction Executed : %s\n", instruction);
	printf("Mode : %s \t Current IP Value: %s\n", (mode == USER_MODE)?"USER":"KERNEL" ,reg[IP_REG]);
	while(1)
	{
		i=0;
		printf("\n# ");
		scanf("%c",&c);
		while(c!='\n')
		{  	
			command[i++] = c;
			scanf("%c",&c);
		}
		command[i] = '\0';
		if(command[0]!='\0')
			if(runCommand(command) == 1)
				return;
	}
}

/* 
Function to process commands 
*/
int runCommand(char command[])
{
	char *name = strtok(command, " ");
	char *arg1, *arg2, *arg3;
	int arg1value, arg2value;	
	if(strcmp(name,"help")==0 || strcmp(name,"h")==0)		//"help" to display all commands
	{
		printf("\n step / s\n\t Single step the exection\n\n");	
		printf(" continue / c\n\t Continue to next breakpoint \n\n");
		printf(" reg / r \n\t Prints the value of all registers \n\n");
		printf(" reg / r <register_name>  \n\t Prints the value of a particular register \n\n");
		printf(" reg / r <register_name1> <register_name2>  \n\t Prints the value of all registers from <register_name1> to <register_name2> \n\n");
		printf(" mem / m <page_num>  \n\t Displays contents of a memory page \n\n");
		printf(" mem / m <page_num1> <page_num2>  \n\t Displays contents of memory pages from <page_num1> to <page_num2>\n\n");
		printf(" pcb / p \n \t Displays the PCB with state as running \n\n");
		printf(" pcb / p <pid> \n\t Displays the <pid> th PCB \n\n");
		printf(" pagetable / pt \n \t Displays the page table at location pointed by PTBR \n\n");
		printf(" pagetable / pt <pid> \n\t Displays the <pid> th page table \n\n");
		printf(" filetable / ft \n \t Displays the System Wide Open File Table\n\n");
		printf(" exit / e \n\t Exit the interface and Halt the machine\n");
		printf(" help / h\n");
	}	
	else if (strcmp(name,"step") == 0 || strcmp(name,"s") == 0)	//Single Stepping
	{
		step_flag = ENABLE;
		return 1;		
	}
	else if (strcmp(name,"continue") == 0 || strcmp(name,"c") == 0)	//Continue till next breakpoint
	{
		step_flag = DISABLE;
		return 1;		
	}	
	else if (strcmp(name,"reg")==0 || strcmp(name,"r")==0) 	//Prints the registers.
	{
		arg1 = strtok(NULL, " ");
		arg2 = strtok(NULL, " ");	
		if(arg1 == NULL)
			printRegisters(R0, NUM_REGS-1);
		else if(arg2 == NULL)
		{
			arg1value = getRegArg(arg1);
			if(arg1value == -1)
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
			else
				printRegisters(arg1value,arg1value);
		}
		else
		{
			arg1value = getRegArg(arg1);
			arg2value = getRegArg(arg2);
			if(arg1value == -1 || arg2value == -1)
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
			else
			{
				if(arg1value > arg2value) 	//swap them
				{
					arg1value = arg1value + arg2value;
					arg2value = arg1value - arg2value;
					arg1value = arg1value - arg2value;
				}
				printRegisters(arg1value,arg2value);
			}
		}
	}	
	else if (strcmp(name,"mem")==0 || strcmp(name,"m")==0)	//displays pages in memory
	{
		arg1 = strtok(NULL, " ");
		arg2 = strtok(NULL, " ");
		if(arg1 == NULL)
			printf("Insufficient argument for \"%s\". See \"help\" for more information",name);
		else if(arg2 == NULL)
		{
			arg1value = atoi(arg1);
			if(arg1value >0 && arg1value < NUM_PAGES)
				printMemory(arg1value);
			else
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
		}
		else
		{
			arg1value = atoi(arg1);
			arg2value = atoi(arg2);
			if(arg1value > arg2value) 	//swap them
			{
				arg1value = arg1value + arg2value;
				arg2value = arg1value - arg2value;
				arg1value = arg1value - arg2value;
			}
			if(arg1value >0 && arg2value < NUM_PAGES)
			{
				while(arg1value <= arg2value)
				{
					printMemory(arg1value);
					arg1value++;
				}
			}
			else
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
		}	
	}						
	else if (strcmp(name,"pcb")==0 || strcmp(name,"p")==0)	//displays PCB of a process
	{
		arg1 = strtok(NULL, " ");
		if(arg1 == NULL)  //finds the PCB with state as running
		{
			int page_no, word_no;
			arg1value = 0;
			while(arg1value < 32)
			{
				page_no = (1536 + arg1value * 32 + 1) / PAGE_SIZE;
				word_no = (1536 + arg1value * 32 + 1) % PAGE_SIZE;
				if(getInteger(page[page_no].word[word_no]) == 2)
					break;
				arg1value++;
			}
			if(arg1value == 32)
			{
				printf("No PCB found with state as running");
				return 0;
			}
		}
		else
		{
			arg1value = atoi(arg1);
			if(arg1value<0 || arg1value >=32)
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return 0;
			}
		}
		printPCB(arg1value);
	}
	else if (strcmp(name,"pagetable")==0 || strcmp(name,"pt")==0)	//displays Page Table of a process
	{
		arg1 = strtok(NULL, " ");
		if(arg1 == NULL)  //finds the page table using PTBR
		{
			int page_no, word_no;
			arg1value = getInteger(reg[PTBR_REG]);
			if(arg1value < 1024 || arg1value > 1272)
			{
				printf("Illegal PTBR value");
				return 0;
			}
		}
		else
		{
			arg1value = 1024 + atoi(arg1) * 8;
			if(arg1value < 1024 || arg1value > 1272)
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return 0;
			}
		}
		printPageTable(arg1value);
	}
	else if (strcmp(name,"filetable")==0 || strcmp(name,"ft")==0)	//displays System Wide Open File Table
		printFileTable();
	else if (strcmp(name,"memfreelist")==0 || strcmp(name,"mf")==0)	//displays System Wide Open File Table
		printMemFreeList();
	else if (strcmp(name,"exit")==0 || strcmp(name,"e")==0)		//Exits the interface
		exit(0);
	else
		printf("Unknown command \"%s\". See \"help\" for more information",name);
	return 0;
}

/*
 * Function to get register number from argument
 */
int getRegArg(char *arg)
{
	int argvalue;
	if(strcmp(arg,"BP") == 0 || strcmp(arg,"bp") == 0)
		return(BP_REG);
	else if(strcmp(arg,"SP") == 0 || strcmp(arg,"sp") == 0)
		return(SP_REG);
	else if(strcmp(arg,"IP") == 0 || strcmp(arg,"ip") == 0)
		return(IP_REG);
	else if(strcmp(arg,"PTBR") == 0 || strcmp(arg,"ptbr") == 0)
		return(PTBR_REG);
	else if(strcmp(arg,"PTLR") == 0 || strcmp(arg,"ptlr") == 0)
		return(PTLR_REG);
	else if(strcmp(arg,"EFR") == 0 || strcmp(arg,"efr") == 0)
		return(EFR_REG);
	else
		argvalue = atoi(arg + 1);
	switch(arg[0])
	{
		case 'R':
		case 'r':
			if(argvalue >= 0 && argvalue < NO_USER_REG);
				return(R0 + argvalue);
			break;
		case 'S':
		case 's':
			if(argvalue >= 0 && argvalue < NO_SYS_REG);
				return(S0 + argvalue);
			break;
		case 'T':
		case 't':
			if(argvalue >= 0 && argvalue < NO_TEMP_REG);
				return(T0 + argvalue);
			break;
	}
	return -1;
}

/* Prints all the registers if arg is -1, 
 * otherwise prints the register passed as argument
 */
void printRegisters(int arg1, int arg2)
{
	int i=1;
	while(arg1 <= arg2)
	{
		switch(arg1) 
		{
			case BP_REG: 
				printf("BP: %s\t",reg[BP_REG]);
				break;
			case SP_REG: 
				printf("SP: %s\t",reg[SP_REG]);
				break;
			case IP_REG: 
				printf("IP: %s\t",reg[IP_REG]);
				break;
			case PTBR_REG: 
				printf("PTBR: %s\t",reg[PTBR_REG]);
				break;
			case PTLR_REG: 
				printf("PTLR: %s\t",reg[PTLR_REG]);
				break;
			case EFR_REG: 
				printf("EFR: %s\t",reg[EFR_REG]);
				break;		
			default: 
				if(arg1<S0)
					printf("R%d: %s\t",arg1,reg[arg1]);
				else if(arg1<T0)
					printf("S%d: %s\t",arg1-S0,reg[arg1]);
				else
					printf("T%d: %s\t",arg1-T0,reg[arg1]);
				break;
		}
		if(i % 4 == 0)
			printf("\n");
		arg1++;
		i++;
	}
	printf("\n");
}

/*
 * This fuction prints the memory page passed as argument.
 */
void printMemory(int page_no)
{
	int word_no;
	printf("Page No : %d",page_no);
	for(word_no = 0; word_no < PAGE_SIZE; word_no++)
	{
		if(word_no % 4 == 0)
			printf("\n");
		printf("%d: %s \t\t", word_no, page[page_no].word[word_no]);
	}
	printf("\n\n");
}

/*
 * This fuction prints the PCB of process with given process ID.
 */
void printPCB(int pid)
{
	int page_no, word_no, counter;
	page_no = (1536 + pid * 32) / PAGE_SIZE;
	word_no = (1536 + pid * 32) % PAGE_SIZE;
	printf("PID\t: %s\nSTATE\t: %s\n", page[page_no].word[word_no], page[page_no].word[word_no+1]);
	printf("BP\t: %s\n", page[page_no].word[word_no+2]);
	printf("SP\t: %s\n", page[page_no].word[word_no+3]);
	printf("IP\t: %s\n", page[page_no].word[word_no+4]);
	printf("PTBR\t: %s\n", page[page_no].word[word_no+5]);
	printf("PTLR\t: %s\n", page[page_no].word[word_no+6]);
	counter=0;
	while(counter < 8)
	{
		printf("R%d\t: %s\n", counter, page[page_no].word[word_no+7+counter]);
		counter++;
	}
	printf("Per-Process Open File Table\n");
	counter = 0;
	while(counter < 8)
	{
		printf("%d: %s\t%s\n", counter, page[page_no].word[word_no+15+ counter*2], page[page_no].word[word_no+16+ counter*2]);
		counter++;
	}
}

/*
 * This fuction prints the page table of process with given process ID.
 */
void printPageTable(int ptbr)
{
	int page_no, word_no, counter;
	page_no = ptbr / PAGE_SIZE;
	word_no = ptbr % PAGE_SIZE;
	printf("Page Table\n");
	counter = 0;
	while(counter < 4)
	{
		printf("%d: %s\t%s\n", counter, page[page_no].word[word_no+ counter*2], page[page_no].word[word_no + counter*2 +1]);
		counter++;
	}
}

/* 
 * This function prints the system wide open file table
 */
 void printFileTable()
 {
	int page_no, word_no, counter;
	page_no = 1344 / PAGE_SIZE;
	word_no = 1344 % PAGE_SIZE;
	printf("System Wide Open File Table\n");
	counter = 0;
	while(counter < 64)
	{
		printf("%d: %s\t%s\n", counter, page[page_no].word[word_no+ counter*2], page[page_no].word[word_no + counter*2 +1]);
		counter++;
	}
 }
 
 /* 
 * This function prints the memory free list
 */
 void printMemFreeList()
 {
	int page_no, word_no, counter;
	page_no = 1280 / PAGE_SIZE;
	word_no = 1280 % PAGE_SIZE;
	printf("Memory Free List");
	counter = 0;
	while(counter < 64)
	{
		if(counter % 4 == 0)
			printf("\n");
		printf("%d: %s \t\t", counter, page[page_no].word[word_no + counter]);
	}
	printf("\n\n");
 }