#define START		0
#define MOV			1
#define ARITH		2
#define LOGIC		3
#define BRANCH		4
#define PUSH		5
#define POP			6
#define CALL		7
#define RET			8
#define IN			9
#define OUT			10
#define HALT		11
#define REG			12
#define MEM_REG		13
#define SP			14
#define BP			15
#define NUM			16
#define ADD			17
#define SUB			18
#define MUL			19
#define DIV			20
#define MOD			21
#define LT			22
#define GT			23
#define EQ			24
#define NE			25
#define GE			26
#define LE			27
#define JZ			28
#define JNZ			29
#define JMP			30
#define LABEL		31
#define MEM_SP		32
#define MEM_BP		33
#define MEM_IP		34
#define MEM_DIR		35

#define IP			37
#define INR			38
#define DCR			39

//Added
#define STRING		40
#define ILLTOKEN	41

#define LOAD		44
#define STORE		45
#define INT			46
#define IRET		47
#define END 		48
#define BRKP		49

#define PTBR		50
#define PTLR		51
#define EIP			52
#define EC			53
#define EPN			54
#define EMA			55
#define MEM_PTBR	56
#define MEM_PTLR	57
#define MEM_EIP		58
#define MEM_EC		59
#define MEM_EPN		60
#define MEM_EMA		61

#define MEM_DIR_REG		62
#define MEM_DIR_SP		63
#define MEM_DIR_BP		64
#define MEM_DIR_IP		65
#define MEM_DIR_PTBR	66
#define MEM_DIR_PTLR	67
#define MEM_DIR_EIP		68
#define MEM_DIR_EC		69
#define MEM_DIR_EPN		70
#define MEM_DIR_EMA		71
#define MEM_DIR_IN		72

#define ILLREG			67

/* New instructions */
#define PORT 			68
#define INA 			69
#define READA 			70
#define STOREA 			71
#define ENCRYPT 		72
#define RESTORE 		73
#define BACKUP 			74
#define LOADA 			75
