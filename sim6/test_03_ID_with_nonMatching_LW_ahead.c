#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "sim5.h"
#include "sim5_test_commonCode.h"


int main()
{
	int i;

#define COUNT 15
	WORD  pcs[COUNT], instructions[COUNT];
	ID_EX out[COUNT];

	instructions[ 0] =  ADD(S_REG(0),  S_REG(1), S_REG(2));
	instructions[ 1] =  SUB(S_REG(3),  S_REG(4), S_REG(5));
	instructions[ 2] =  ADDI(T_REG(2), T_REG(3), -1);
	instructions[ 3] =  ADDI(T_REG(4), T_REG(5), 16);
	instructions[ 4] =  ADDU( 1,  0, 2);
	instructions[ 5] =  SUBU( 3,  4, 5);
	instructions[ 6] = ADDIU( 6,  7,0x1234);
	instructions[ 7] =   AND( 9, 10,11);
	instructions[ 8] =    OR(12, 13,14);
	instructions[ 9] =  ANDI(15, 16,0x5678);
	instructions[10] =   ORI(18, 19,0x9abc);
	instructions[11] =   SLT(21, 22,23);
	instructions[12] =  SLTI(24, 25,-1);
	instructions[13] =    LW(26,  0,104);
	instructions[14] =    SW(29,  0,108);

	/* assign random program counters.  Or rather, pseudorandom, using
	 * a fixed seed
	 */
	srand(0x11110415);
	for (i=0; i<COUNT; i++)
		pcs[i] = rand() & 0xfffffffc;

	/* we need some dummy register values.  None are actually modified
	 * by this test.
	 */
	WORD regs[34];
	for (i=0; i<34; i++)
		regs[i] = 0x01010101 * i;


	/* test the ID phase for all instructions */
	for (i=0; i<COUNT; i++)
	{
		// create a ID_EX ahead which is a LW, but which doesn't
		// match either the rs or rt instructions
		ID_EX lwAhead;
		  memset(&lwAhead, 0, sizeof(lwAhead));

		// rs should be ignored, but I intentionally make it 17 ($s1)
		// to see if the student code checks it.
		lwAhead.rs = 17;

		// I checked, and 19 ($s3) is not read (just written to) by
		// any of the instructions above.  But sometimes, we'll get
		// a false positive - so we'll increment if necessary.
		lwAhead.rt = 19;

		if (lwAhead.rt == ((instructions[i] >> 21) & 0x1f))  // rs
			lwAhead.rt++;
		if (lwAhead.rt == ((instructions[i] >> 16) & 0x1f))  // rt
			lwAhead.rt++;

		// set the rest of the bits to something appropriate for a LW:
		lwAhead.ALUsrc   = 1;
		lwAhead.ALU.op   = 2;
		lwAhead.memRead  = 1;
		lwAhead.regDst   = 0;    // rt, not rd
		lwAhead.regWrite = 1;


		// do the test!  No stall should happen.
		Test_ID(pcs[i], instructions[i], regs, &out[i], NULL);
	}

	return 0;
}


