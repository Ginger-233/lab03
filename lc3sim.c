#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3 machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* 
  MEMORY[A] stores the word address A
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3 State info.                                           */
/***************************************************************/
#define LC_3_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3 ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3 for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3 until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = start ; address <= stop ; address++)
    printf("  0x%.4x (%d) : 0x%.2x\n", address , address , MEMORY[address]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = start ; address <= stop ; address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x\n", address , address , MEMORY[address]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word ;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii] = word;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = program_base;

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3 Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



#define SEXT(x, n) (((x) << (32 - n)) >> (32 - n))

int fetch() {
    int instr;
    instr = MEMORY[CURRENT_LATCHES.PC];
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 1;
    return instr;
}

void SetCC(int dr) {
    int val = NEXT_LATCHES.REGS[dr];
    if ((val & 0x8000) >> 15) {
        NEXT_LATCHES.N = 1;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 0;
    }
    else {
        if (val == 0) {
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
        } else {
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.P = 1;
        }
    }
}

void ADD(int instr) {
    int dr = (instr >> 9) & 7;
    int sr1 = (instr >> 6) & 7;
    int immFlag = (instr >> 5) & 1;
    int result;

    if (immFlag) {
        int imm5 = instr & 0x1F;
        imm5 = SEXT(imm5, 5);
        result = CURRENT_LATCHES.REGS[sr1] + imm5;
    } else {
        int sr2 = instr & 7;
        result = CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2];
    }

    NEXT_LATCHES.REGS[dr] = result;
    SetCC(dr);
}

void AND(int instr) {
    int dr = (instr >> 9) & 7;
    int sr1 = (instr >> 6) & 7;
    int immFlag = (instr >> 5) & 1;

    if (immFlag) {
        int imm5 = instr & 0x1F;
        imm5 = SEXT(imm5, 5);
        NEXT_LATCHES.REGS[dr] = CURRENT_LATCHES.REGS[sr1] & imm5;
    } else {
        int sr2 = instr & 7;
        NEXT_LATCHES.REGS[dr] = CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2];
    }

    SetCC(dr);
}

void BR(int instr) {
    int n = (instr >> 11) & 1;
    int z = (instr >> 10) & 1;
    int p = (instr >> 9) & 1;
    int offset = SEXT(instr & 0x1FF, 9);

    if ((n && CURRENT_LATCHES.N) || (z && CURRENT_LATCHES.Z) || (p && CURRENT_LATCHES.P)) {
        NEXT_LATCHES.PC += offset;
    }
}

void JMP(int instr) {
    int base = (instr >> 6) & 7;
    NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[base];
}

void JSR(int instr) {
    int longFlag = (instr >> 11) & 1;
    if (longFlag) {
        int offset = SEXT(instr & 0x7FF, 11);
        NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
        NEXT_LATCHES.PC += offset;
    } else {
        int base = (instr >> 6) & 7;
        NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
        NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[base];
    }
}

void LD(int instr) {
    int dr = (instr >> 9) & 7;
    int offset = SEXT(instr & 0x1FF, 9);
    int addr = NEXT_LATCHES.PC + offset;
    NEXT_LATCHES.REGS[dr] = MEMORY[addr];
    SetCC(dr);
}

void LDI(int instr) {
    int dr = (instr >> 9) & 7;
    int offset = SEXT(instr & 0x1FF, 9);
    int ptr = MEMORY[NEXT_LATCHES.PC + offset];
    NEXT_LATCHES.REGS[dr] = MEMORY[ptr];
    SetCC(dr);
}

void LDR(int instr) {
    int dr = (instr >> 9) & 7;
    int base = (instr >> 6) & 7;
    int offset = SEXT(instr & 0x3F, 6);
    int addr = CURRENT_LATCHES.REGS[base] + offset;
    NEXT_LATCHES.REGS[dr] = MEMORY[addr];
    SetCC(dr);
}

void LEA(int instr) {
    int dr = (instr >> 9) & 7;
    int offset = SEXT(instr & 0x1FF, 9);
    NEXT_LATCHES.REGS[dr] = NEXT_LATCHES.PC + offset;
}

void NOT(int instr) {
    int dr = (instr >> 9) & 7;
    int sr = (instr >> 6) & 7;
    NEXT_LATCHES.REGS[dr] = ~CURRENT_LATCHES.REGS[sr];
    SetCC(dr);
}

void ST(int instr) {
    int sr = (instr >> 9) & 7;
    int offset = SEXT(instr & 0x1FF, 9);
    int addr = NEXT_LATCHES.PC + offset;
    MEMORY[addr] = CURRENT_LATCHES.REGS[sr] & 0xFFFF;
}

void STI(int instr) {
    int sr = (instr >> 9) & 7;
    int offset = SEXT(instr & 0x1FF, 9);
    int addr = MEMORY[NEXT_LATCHES.PC + offset];
    MEMORY[addr] = CURRENT_LATCHES.REGS[sr] & 0xFFFF;
}

void STR(int instr) {
    int sr = (instr >> 9) & 7;
    int base = (instr >> 6) & 7;
    int offset = SEXT(instr & 0x3F, 6);
    int addr = CURRENT_LATCHES.REGS[base] + offset;
    MEMORY[addr] = CURRENT_LATCHES.REGS[sr] & 0xFFFF;
}

void TRAP(int instr) {
    int trapvect = instr & 0xFF;
    NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
    NEXT_LATCHES.PC = MEMORY[trapvect];
}

void decode(int instr) {
    int opcode = (instr >> 12) & 0xF;

    if (opcode == 1) {
        ADD(instr);
    } else if (opcode == 5) {
        AND(instr);
    } else if (opcode == 0) {
        BR(instr);
    } else if (opcode == 12) {
        JMP(instr);
    } else if (opcode == 4) {
        JSR(instr);
    } else if (opcode == 2) {
        LD(instr);
    } else if (opcode == 10) {
        LDI(instr);
    } else if (opcode == 6) {
        LDR(instr);
    } else if (opcode == 14) {
        LEA(instr);
    } else if (opcode == 9) {
        NOT(instr);
    } else if (opcode == 3) {
        ST(instr);
    } else if (opcode == 11) {
        STI(instr);
    } else if (opcode == 7) {
        STR(instr);
    } else if (opcode == 15) {
        TRAP(instr);
    }
}

void process_instruction() {
    int instr = fetch();
    decode(instr);
    CURRENT_LATCHES = NEXT_LATCHES;
}

