#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "sim5.h"
#include "sim5_test_commonCode.h"


int main()
{
	int i;

#define COUNT 21
	WORD  pcs[COUNT], instructions[COUNT];
	ID_EX  idex [COUNT];
	EX_MEM exmem[COUNT];

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
//	instructions[21] =   JAL(0x07654321);
//	instructions[22] =   JAL(0x0000000f);
//	instructions[23] =    JR(29);
//	instructions[24] =    JR(16);

	/* assign random program counters.  Or rather, pseudorandom, using
	 * a fixed seed
	 */
	srand(0x56782141);
	for (i=0; i<COUNT; i++)
		pcs[i] = rand() & 0xfffffffc;

	/* we need some dummy register values.  None are actually modified
	 * by this test, since no instruction gets to the WB phase.
	 */
	WORD regs[34];
	for (i=0; i<34; i++)
		regs[i] = 0x01010101 * i;


	/* test the ID and EX phases for all instructions.  In all cases, we
	 * just run isolated instructions (as if they were a single-cycle
	 * processor).
	 */
	for (i=0; i<COUNT; i++)
	{
		Test_ID(pcs[i], instructions[i], regs, &idex[i], NULL);

		printf("NON-FORWARDING TEST:\n");
		printf("\n");
		Test_EX(&idex[i], &exmem[i], NULL, NULL);

		printf("TESTING AN ARTIFICIAL FORWARDING: EX/MEM->rs (value 0x12345678)\n");
		printf("\n");
		EX_MEM old_exMem;
		  memset(&old_exMem, 0, sizeof(old_exMem));
		  old_exMem.aluResult = 0x12345678;
		  old_exMem.regWrite  = 1;
		  old_exMem.writeReg  = idex[i].rs;

		Test_EX(&idex[i], &exmem[i], &old_exMem, NULL);

		printf("TESTING AN ARTIFICIAL FORWARDING: EX/MEM->rt (value 0x12345678)\n");
		printf("\n");
		  old_exMem.writeReg = idex[i].rt;

		Test_EX(&idex[i], &exmem[i], &old_exMem, NULL);

		printf("TESTING AN ARTIFICIAL FORWARDING: MEM/WB->rs (value 0xffffffff)\n");
		printf("\n");
		MEM_WB old_memWb;
		  memset(&old_memWb, 0, sizeof(old_memWb));
		  old_memWb.aluResult = 0xffffffff;
		  old_memWb.regWrite  = 1;
		  old_memWb.writeReg  = idex[i].rs;

		Test_EX(&idex[i], &exmem[i], NULL, &old_memWb);

		printf("TESTING AN ARTIFICIAL FORWARDING: MEM/WB->rt (value 0xffffffff)\n");
		printf("\n");
		  old_memWb.writeReg = idex[i].rt;

		Test_EX(&idex[i], &exmem[i], NULL, &old_memWb);

		printf("TESTING AN ARTIFICIAL FORWARDING: BOTH->rs (values 0x12345678 0xffffffff)\n");
		printf("\n");
		  old_exMem.writeReg = old_memWb.writeReg = idex[i].rs;

		Test_EX(&idex[i], &exmem[i], &old_exMem, &old_memWb);

		printf("TESTING AN ARTIFICIAL FORWARDING: BOTH->rt (values 0x12345678 0xffffffff)\n");
		printf("\n");
		  old_exMem.writeReg = old_memWb.writeReg = idex[i].rt;

		Test_EX(&idex[i], &exmem[i], &old_exMem, &old_memWb);

		printf("END ARTIFICIAL FORWARDING TESTS.  MOVING TO THE NEXT INSTRUCTION...\n");
		printf("\n");
	}

	return 0;
}

