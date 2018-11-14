#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "sim5.h"
#include "sim5_test_commonCode.h"


int main()
{
	int i;

#define COUNT 6
	WORD pcs[COUNT], instructions[COUNT];
	ID_EX out[COUNT];

	instructions[0] = BEQ( 8, 9, 0x8104);
	instructions[1] = BEQ( 8,10, 0xc010);
	instructions[2] = BNE(10,11, 0xf000);
	instructions[3] = BNE(10,12, 0x9000);
	instructions[4] =   J(0x01234567);
	instructions[5] =   J(0x03ffffff);

	/* assign random program counters.  Or rather, pseudorandom, using
	 * a fixed seed
	 */
	srand(0x04151633);
	for (i=0; i<COUNT; i++)
		pcs[i] = rand() & 0xfffffffc;


	/* In this testcase, we need to have some equal (and not equal)
	 * registers for testing.
	 */
	WORD regs[34];
	for (i=0; i<34; i++)
		regs[i] = 0x01010101 * i;
	regs[ 8] = regs[ 9] = 1234;
	regs[10] = regs[11] = 5678;

	/* registers 29 and 16 are used for JR instructions */
	regs[29] = 0xf4000204;
	regs[16] = 0xc23f3c18;


	/* test the ID phase for all instructions */
	for (i=0; i<COUNT; i++)
		Test_ID(pcs[i], instructions[i], regs, &out[i], NULL);

	return 0;
}
