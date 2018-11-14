#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "sim5.h"
#include "sim5_test_commonCode.h"


int main()
{
	int i;

#define MEM_SIZE (16*1024)
	WORD dataMemory[MEM_SIZE];


#define COUNT (21+3)
	WORD  pcs[COUNT], instructions[COUNT];

	/* this contains all of the instructions from tests 02 and 03 */
	instructions[ 0] =   ADD(S_REG(0),  S_REG(1), S_REG(2));
	instructions[ 1] =   SUB(S_REG(3),  S_REG(4), S_REG(5));
	instructions[ 2] =  ADDI(T_REG(2), T_REG(3), 0xcccc);
	instructions[ 3] =  ADDI(T_REG(4), T_REG(5), 16);
	instructions[ 4] =  ADDU( 1,  0, 2);
	instructions[ 5] =  SUBU( 3,  4, 5);
	instructions[ 6] = ADDIU( 6,  7,0x1234);
	instructions[ 7] =   AND( 9, 10,11);
	instructions[ 8] =    OR(12, 13,14);
	instructions[ 9] =  ANDI(15, 16,0x9678);
	instructions[10] =   ORI(18, 19,0x9abc);
	instructions[11] =   SLT(21, 22,23);
	instructions[12] =  SLTI(24, 25,0x7777);
	instructions[13] =    LW(26,  0,104);
	instructions[14] =    SW(29,  0,108);
	instructions[15] =   BEQ( 8, 9, 0x8104);
	instructions[16] =   BEQ( 8, 8, 0xc010);
	instructions[17] =   BNE(10,11, 0xf000);
	instructions[18] =   BNE(10,10, 0x9000);
	instructions[19] =     J(0x01234567);
	instructions[20] =     J(0x03ffffff);
	// we add 3 NOPs at the end of the normal instructions, to flush
	// the pipeline.
	instructions[25] = NOP();
	instructions[26] = NOP();
	instructions[27] = NOP();


	/* assign random program counters.  Or rather, pseudorandom, using
	 * a fixed seed
	 */
	srand(0x55552046);
	for (i=0; i<COUNT; i++)
		pcs[i] = rand() & 0xfffffffc;

	/* we need some dummy register values.  None are actually modified
	 * by this test, since no instruction gets to the WB phase.
	 */
	WORD regs[34];
	for (i=0; i<34; i++)
		regs[i] = 0x01010101 * i;

	/* fill in the data memory, since we now are also running the MEM
	 * phase.
	 */
	for (i=0; i<sizeof(dataMemory); i+=4)
		dataMemory[i/4] = 0xffff0000 + i;


	Test_Pipelined_FixedSequence(instructions, pcs, COUNT,
	                             regs,
	                             dataMemory, MEM_SIZE);

	return 0;
}

