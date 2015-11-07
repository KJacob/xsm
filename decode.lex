%{
	#include "instr.h"
	#include "data.h"
	#include "interrupt.h"
	#define BLANK_THRESHOLD 10
	#define YY_INPUT(buf,result,max_size)	\
	{					\
		int len;			\
		bzero(buf,max_size);		\
		strcpy(buf, instruction);	\
		len = strlen(buf);		\
		result = len;			\
	}
	char tempbuf[16];
	int tempnum;
	int blank_count;
	void decode_indexed_addr(char location[],char index[]);
%}

%option noyywrap

%%

START	{ blank_count=0; return(START);}
MOV		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(MOV); }
ADD		{ yylval.flag = ADD; yylval.flag2 = ILLREG; blank_count=0; return(ARITH); }
SUB		{ yylval.flag = SUB; yylval.flag2 = ILLREG; blank_count=0; return(ARITH); }
MUL		{ yylval.flag = MUL; yylval.flag2 = ILLREG; blank_count=0; return(ARITH); }
DIV		{ yylval.flag = DIV; yylval.flag2 = ILLREG; blank_count=0; return(ARITH); }
MOD		{ yylval.flag = MOD; yylval.flag2 = ILLREG; blank_count=0; return(ARITH); }
INR		{ yylval.flag = INR; yylval.flag2 = ILLREG; blank_count=0; return(ARITH); }
DCR		{ yylval.flag = DCR; yylval.flag2 = ILLREG; blank_count=0; return(ARITH); }
LT		{ yylval.flag = LT; yylval.flag2 = ILLREG; blank_count=0; return(LOGIC); }
GT		{ yylval.flag = GT; yylval.flag2 = ILLREG; blank_count=0; return(LOGIC); }
EQ		{ yylval.flag = EQ; yylval.flag2 = ILLREG; blank_count=0; return(LOGIC); }
NE		{ yylval.flag = NE; yylval.flag2 = ILLREG; blank_count=0; return(LOGIC); }
GE		{ yylval.flag = GE; yylval.flag2 = ILLREG; blank_count=0; return(LOGIC); }
LE		{ yylval.flag = LE; yylval.flag2 = ILLREG; blank_count=0; return(LOGIC); }
JZ 		{ yylval.flag = JZ; yylval.flag2 = ILLREG; blank_count=0; return(BRANCH); }
JNZ 		{ yylval.flag = JNZ; yylval.flag2 = ILLREG; blank_count=0; return(BRANCH); }
JMP 		{ yylval.flag = JMP; yylval.flag2 = ILLREG; blank_count=0; return(BRANCH); }
PUSH		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(PUSH); }
POP		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(POP); }
CALL		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(CALL); }
RET		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(RET); }
IN		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(IN); }
OUT		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(OUT); }
LOAD		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(LOAD); }
STORE		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(STORE); }
HALT		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(HALT); }
INT		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(INT); }
END		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(END); }
BRKP		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(BRKP); }
IRET 		{ yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(IRET);}
PORT 		{yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(PORT);}
INA 		{yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(INA);}
READA		{yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(READA);}
STOREA 		{yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(STOREA);}
ENCRYPT 		{yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(ENCRYPT);}
RESTORE 		{yylval.flag = 0; yylval.flag2 = ILLREG; blank_count=0; return(RESTORE);}
SP 		{ yylval.flag = SP; yylval.flag2 = ILLREG; blank_count=0; return(SP_REG); }
BP		{ yylval.flag = BP; yylval.flag2 = ILLREG; blank_count=0; return(BP_REG); }
IP		{ yylval.flag = IP; yylval.flag2 = ILLREG; blank_count=0; return(IP_REG); }
PTBR		{ yylval.flag = PTBR; yylval.flag2 = ILLREG; blank_count=0; return(PTBR_REG); }
PTLR		{ yylval.flag = PTLR; yylval.flag2 = ILLREG; blank_count=0; return(PTLR_REG); }
EIP		{yylval.flag = EIP; yylval.flag2 = ILLREG; blank_count = 0; return(EIP);}
EC		{yylval.flag = EC; yylval.flag2 = ILLREG; blank_count = 0; return(EC);}
EPN		{yylval.flag = EPN; yylval.flag2 = ILLREG; blank_count = 0; return(EPN);}
EMA		{yylval.flag = EMA; yylval.flag2 = ILLREG; blank_count = 0; return(EMA);}

R[0-9]+ { 
			yylval.flag = REG;
			yytext++;
			tempnum = atoi(yytext);
			blank_count=0;
			if(tempnum > 7)
				return ILLREG;
			yylval.flag2 = tempnum + R0;	
			return(tempnum + R0);
		}

P[0-3] {
			yylval.flag = PORT;
			yytext++;
			tempnum = atoi(yytext);
			yylval.flag2 = tempnum + P0;
			return yylval.flag2;
		}

\[SP\]		{ yylval.flag = MEM_SP; yylval.flag2 = ILLREG; blank_count=0; return(SP_REG); }
\[BP\]		{ yylval.flag = MEM_BP; yylval.flag2 = ILLREG; blank_count=0; return(BP_REG); }
\[IP\]		{ yylval.flag = MEM_IP; yylval.flag2 = ILLREG; blank_count=0; return(IP_REG); }		//error: Is this needed.
\[PTBR\]	{ yylval.flag = MEM_PTBR; yylval.flag2 = ILLREG; blank_count=0; return(PTBR_REG); }
\[PTLR\]	{ yylval.flag = MEM_PTLR; yylval.flag2 = ILLREG; blank_count=0; return(PTLR_REG); }
\[EFR\]		{ yylval.flag = MEM_EFR; yylval.flag2 = ILLREG; blank_count=0; return(EFR_REG); }
\[R[0-9]+\] 	{	
			yylval.flag = MEM_REG; yylval.flag2 = ILLREG; 
			yytext[yyleng-1]='\0';
			yytext=yytext+2;
			tempnum = atoi(yytext);
			blank_count=0;
			if(tempnum > 7)
				return ILLREG;
			return(tempnum + R0); 
		}

-?[0-9]+		{ yylval.flag = NUM; yylval.flag2 = ILLREG; blank_count=0; return(atoi(yytext)); }
\[[0-9]+\]		{
				yylval.flag = MEM_DIR; yylval.flag2 = ILLREG;
				yytext[yyleng-1]='\0';
				yytext++;
				blank_count=0; return(atoi(yytext));
			}
\[-?[0-9]+\]R[0-9]+	{
				yylval.flag = MEM_DIR_REG;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				tempnum = atoi(tempbuf);
				if(tempnum > 7)
					yylval.flag2 = ILLREG;
				else
					yylval.flag2 =  tempnum + R0;
				blank_count=0; return(atoi(yytext));					
			}
			
\[-?[0-9]+\]SP		{
				yylval.flag = MEM_DIR_SP;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				yylval.flag2 = SP_REG;
				blank_count=0; return(atoi(yytext));					
			}
\[-?[0-9]+\]BP		{
				yylval.flag = MEM_DIR_BP;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				yylval.flag2 = BP_REG;
				blank_count=0; return(atoi(yytext));					
			}
\[-?[0-9]+\]IP		{
				yylval.flag = MEM_DIR_IP;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				yylval.flag2 = IP_REG;
				blank_count=0; return(atoi(yytext));					
			}
\[-?[0-9]+\]PTBR	{
				yylval.flag = MEM_DIR_PTBR;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				yylval.flag2 = PTBR_REG;
				blank_count=0; return(atoi(yytext));					
			}
\[-?[0-9]+\]PTLR	{
				yylval.flag = MEM_DIR_PTLR;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				yylval.flag2 = PTLR_REG;
				blank_count=0; return(atoi(yytext));					
			}
\[-?[0-9]+\]EFR		{
				yylval.flag = MEM_DIR_EFR;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				yylval.flag2 = EFR_REG;
				blank_count=0; return(atoi(yytext));					
			}					
\[-?[0-9]+\]-?[0-9]+	{
				yylval.flag = MEM_DIR_IN;
				yytext++;
				decode_indexed_addr(yytext,tempbuf);	//Not at all tested. Vulnerable ***
				yylval.flag2 = atoi(tempbuf);
				blank_count=0; return(atoi(yytext));					
			}
					
\"[^"]*			{
				if(yytext[yyleng-1] == '\\')
					yymore();
				else
				{
					yytext[yyleng]='\0';
					yytext++;
					strcpy(yylval.data,yytext);
					yylval.flag = STRING; yylval.flag2 = ILLREG;
					blank_count=0; return(0);
				}
			}

[\n\t ]			{blank_count++;
				if(blank_count == BLANK_THRESHOLD)
					return(ILLTOKEN);
				}
\/\/.*			{blank_count=0;}
[,]				;
.				{blank_count=0; return(ILLTOKEN);}
%%

/*
 Splits the token of the form "[location]index" to location and index
 */
void decode_indexed_addr(char location[],char index[]) 			//Not at all tested. Vulnerable ***
{
	int i,flag,j;
	for(i=0,j=0,flag=0;location[i]!='\0';i++,j++)
	{
		if(location[i] == ']')
		{
			flag = 1;
			j=0;
			location[i]='\0';
			if(location[i+1] == 'R' || location[i+1] == 'S' || location[i+1] == 'T')
			{
				i++;
				location[i]='\0';
			}			
		}
		if(flag == 1)
			index[j] = location[i];
	}
	index[j]='\0';	
}
