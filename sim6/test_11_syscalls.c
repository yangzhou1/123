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

	
	// addi	$v0, $zero, 1		# print_int(1234) - twice
	// addi $a0, $zero, 1234
	// nop				# we add NOPs because my syscall
	// nop				# code is kind of stupid (no data
	// syscall			# forwarding)
	// syscall
	//
	// addi	$v0, $zero, 1		# print_int(5678)
	// addi $a0, $zero, 5678
	// nop
	// nop
	// syscall
	//
	// addi	$v0, $zero, 11		# print_char('\n')
	// addi $a0, $zero, 0xa
	// nop
	// nop
	// syscall
	//
	// addi	$v0, $zero, 4
	// addi $a0, $zero, 0x1000	# print_str("TEST STRING\n")
	// nop
	// nop
	// syscall
	//
	// addi	$v0, $zero, 10		# sys_exit
	// nop
	// nop
	// syscall

	instMemory[ 0] = ADDI(V_REG(0), REG_ZERO,1);
	instMemory[ 1] = ADDI(A_REG(0), REG_ZERO,1234);
	instMemory[ 2] = NOP();
	instMemory[ 3] = NOP();
	instMemory[ 4] = SYSCALL();
	instMemory[ 5] = SYSCALL();

	instMemory[ 6] = ADDI(V_REG(0), REG_ZERO,1);
	instMemory[ 7] = ADDI(A_REG(0), REG_ZERO,5678);
	instMemory[ 8] = NOP();
	instMemory[ 9] = NOP();
	instMemory[10] = SYSCALL();

	instMemory[11] = ADDI(V_REG(0), REG_ZERO,11);
	instMemory[12] = ADDI(A_REG(0), REG_ZERO,0xa);
	instMemory[13] = NOP();
	instMemory[14] = NOP();
	instMemory[15] = SYSCALL();

	instMemory[16] = ADDI(V_REG(0), REG_ZERO,4);
	instMemory[17] = ADDI(A_REG(0), REG_ZERO,0x1000);
	instMemory[18] = NOP();
	instMemory[19] = NOP();
	instMemory[20] = SYSCALL();

	instMemory[21] = ADDI(V_REG(0), REG_ZERO,10);
	instMemory[22] = NOP();
	instMemory[23] = NOP();
	instMemory[24] = SYSCALL();

	/* set up the string in the data memory */
#define CHARS_TO_WORD(a,b,c,d) (((d) << 24) |   \
                                ((c) << 16) |   \
                                ((b) <<  8) |   \
                                 (a)       )
	dataMemory[0x400] = CHARS_TO_WORD('T','E','S','T');
	dataMemory[0x401] = CHARS_TO_WORD(' ','S','T','R');
	dataMemory[0x402] = CHARS_TO_WORD('I','N','G','\n');
	dataMemory[0x403] = CHARS_TO_WORD('\0', 0,0,0);


	WORD codeOffset = 0x447a5d34;
	ExecProcessor(instMemory, CODE_SIZE,
	              regs,
	              dataMemory, DATA_SIZE,
	              codeOffset);

	return 0;
}


