#include <stdio.h>
#include <memory.h>

#include "sim5.h"
#include "sim5_test_commonCode.h"



int main()
{
#define CODE_SIZE (16*1024)
#define DATA_SIZE (16*1024)
	WORD regs[34];
	WORD instMemory[CODE_SIZE];
	WORD dataMemory[DATA_SIZE];

	// fill in the registers and data memory with some good default values
	int i;
	for (i=0; i<34; i++)
		regs[i] = 0x01010101 * i;
	for (i=0; i<sizeof(dataMemory); i+=4)
		dataMemory[i/4] = 0xffff0000 + i;


	// num   = 10;
	// count = 5;
	// step  = 3;
	// for (int i=0; i<count; i++)
	// {
	//	num += step;
	//	printf("%d\n", num);
	// }
	//
	// ---------------------
	//
	//	addi	$t0, $zero, 0		# i     = 0
	//	addi	$t1, $zero, 10		# num   = 10
	//	addi	$t2, $zero, 5		# count = 5
	//	addi	$t4, $zero, 3		# step  = 3
	//	
	// RANGE_LOOP:
	//	slt	$t3, $t0, $t2
	//      nop                             # added for simplicity
	//      nop
	//	beq	$t3, $zero, RANGE_LOOP_END
	//
	//	add	$t1, $t1, $t4		# num += step
	//
	//	addi	$v0, $zero, 1		# print_int(num)
	//	addi	$a0, $t1, $zero
	//      nop                             # added for simplicity
	//      nop
	//	syscall
	//
	//	addi	$v0, $zero, 11		# print_char('\n')
	//	addi	$a0, $zero, 0xa
	//      nop                             # added for simplicity
	//      nop
	//	syscall
	//
	//	addi	$t0, $t0, 1		# i++
	//	j	RANGE_LOOP
	//
	// RANGE_LOOP_END:
	//	addi	$v0, $zero, 10		# sys_exit
	//      nop                             # added for simplicity
	//      nop
	//	syscall

	// NEW FOR PROJECT 11:
	//
	// We will insert 2 NOPs *BEFORE* the BEQ isntruction, so that we
	// don't have to worry about data-forwarding issues.

	
	WORD codeOffset = 0x12340000;

	// init
	instMemory[ 0] = ADDI(T_REG(0), REG_ZERO,0);
	instMemory[ 1] = ADDI(T_REG(1), REG_ZERO,10);
	instMemory[ 2] = ADDI(T_REG(2), REG_ZERO,5);
	instMemory[ 3] = ADDI(T_REG(4), REG_ZERO,3);

	// RANGE_LOOP:
	instMemory[ 4] = SLT(T_REG(3), T_REG(0),T_REG(2));
	instMemory[ 5] = NOP();
	instMemory[ 6] = NOP();
	instMemory[ 7] = BEQ(T_REG(3),REG_ZERO, 13);

	// num += step
	instMemory[ 8] = ADD(T_REG(1), T_REG(1),T_REG(4));

	// print_int(num)
	instMemory[ 9] = ADDI(V_REG(0), REG_ZERO,1);
	instMemory[10] = ADD (A_REG(0), T_REG(1), REG_ZERO);
	instMemory[11] = NOP();
	instMemory[12] = NOP();
	instMemory[13] = SYSCALL();

	// print_char('\n')
	instMemory[14] = ADDI(V_REG(0), REG_ZERO,11);
	instMemory[15] = ADDI(A_REG(0), REG_ZERO,0xa);
	instMemory[16] = NOP();
	instMemory[17] = NOP();
	instMemory[18] = SYSCALL();

	// i++
	instMemory[19] = ADDI(T_REG(0), T_REG(0),1);

	// j RANGE_LOOP
	instMemory[20] = J(0x08d0004);

	// RANGE_LOOP_END
	instMemory[21] = ADDI(V_REG(0), REG_ZERO,10);
	instMemory[22] = NOP();
	instMemory[23] = NOP();
	instMemory[24] = SYSCALL();


	Test_FullProcessor(instMemory, CODE_SIZE,
	                   regs,
	                   dataMemory, DATA_SIZE,
	                   codeOffset);

	return 0;
}


