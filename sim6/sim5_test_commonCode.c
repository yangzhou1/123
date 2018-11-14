#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#include "sim5.h"
#include "sim5_test_commonCode.h"



int Test_ID(WORD pcPlus4, WORD instruction,
            WORD *regs,
            ID_EX *out,
            ID_EX *old_idex)
{
	// should this be a parameter?
	EX_MEM *old_exmem = NULL;

	WORD regs_save[34];
	  memcpy(regs_save, regs, sizeof(regs_save));

	InstructionFields fields;
	  memset(&fields, -1, sizeof(fields));

	memset(out, -1, sizeof(*out));

	// if the caller didn't provide an old ID/EX version, then we'll
	// provide a NOP.
	ID_EX  nop_idex;
	EX_MEM nop_exmem;
	  memset(&nop_idex,  0, sizeof(nop_idex));
	  memset(&nop_exmem, 0, sizeof(nop_exmem));
	if (old_idex == NULL)
		old_idex = &nop_idex;
	if (old_exmem == NULL)
		old_exmem = &nop_exmem;

	ID_EX old_idex_save;
	  memcpy(&old_idex_save,  old_idex,  sizeof(old_idex_save));
	ID_EX old_exmem_save;
	  memcpy(&old_exmem_save, old_exmem, sizeof(old_exmem_save));


	printf("Before extract_instructionFields(): pc+4=0x%04x_%04x instruction=0x%04x_%04x\n",
	       (pcPlus4     >> 16) & 0xffff, (pcPlus4          ) & 0xffff,
	       (instruction >> 16) & 0xffff, (instruction      ) & 0xffff);

	extract_instructionFields(instruction, &fields);

	/* save the fields struct for later comparison */
	InstructionFields fields_save;
	  memcpy(&fields_save, &fields, sizeof(fields));

	printf("  fields.opcode = %d\n", fields.opcode);
	printf("  fields.rs     = %d\n", fields.rs);
	printf("  fields.rt     = %d\n", fields.rt);
	printf("  fields.rd     = %d\n", fields.rd);
	printf("  fields.shamt  = %d\n", fields.shamt);
	printf("  fields.funct  = %d\n", fields.funct);
	printf("  fields.imm16  = 0x%04x_%04x\n",
	       (fields.imm16 >> 16) & 0xffff,
	       (fields.imm16      ) & 0xffff);
	printf("  fields.imm32  = 0x%04x_%04x\n",
	       (fields.imm32 >> 16) & 0xffff,
	       (fields.imm32      ) & 0xffff);
	printf("  fields.address  = 0x%04x_%04x\n",
	       (fields.address >> 16) & 0xffff,
	       (fields.address      ) & 0xffff);

	printf("  ---\n");

	int stall = IDtoIF_get_stall(&fields, old_idex, old_exmem);
	printf("  IDtoIF_get_stall = %d\n", stall);

	WORD rsVal = regs[fields.rs];
	WORD rtVal = regs[fields.rt];
	int branchControl = IDtoIF_get_branchControl(&fields, rsVal,rtVal);
	printf("  IDtoIF_get_branchControl = %d\n", branchControl);

	WORD   jumpAddr =   calc_jumpAddr(pcPlus4, &fields);
	WORD branchAddr = calc_branchAddr(pcPlus4, &fields);
	printf("  jumpAddr   = 0x%04x_%04x\n",
	       (jumpAddr   >> 16) & 0xffff, (jumpAddr        ) & 0xffff);
	printf("  branchAddr = 0x%04x_%04x\n",
	       (branchAddr >> 16) & 0xffff, (branchAddr      ) & 0xffff);

	printf("  ---\n");

	int rc = execute_ID(stall, &fields, pcPlus4, rsVal,rtVal, out);

	if (memcmp(&fields, &fields_save, sizeof(fields_save)) != 0)
	{
		printf("ERROR: execute_ID() modified the Fields struct outside of extract_instructionFields().\n");
	}

	if (memcmp(old_idex, &old_idex_save, sizeof(old_idex_save)) != 0)
		printf("ERROR: execute_ID() modified the (old) ID_EX struct.\n");

	if (rc == 0)
	{
		// this is not always an error.  Maybe the instruction was
		// actually invalid?  So we'll just print out a message.
		printf("  execute_ID() returned %d\n", rc);
		return rc;
	}

	printf("  ID_EX.rs = %d\n", out->rs);
	printf("  ID_EX.rt = %d\n", out->rt);
	printf("  ID_EX.rd = %d\n", out->rd);
	printf("  ---\n");
	printf("  ID_EX.rsVal = 0x%04x_%04x\n",
	       (out->rsVal >> 16) & 0xffff, out->rsVal & 0xffff);
	printf("  ID_EX.rtVal = 0x%04x_%04x\n",
	       (out->rtVal >> 16) & 0xffff, out->rtVal & 0xffff);
	printf("  ---\n");
	printf("  ID_EX.ALUsrc      = %d\n", out->ALUsrc);
	printf("  ID_EX.ALU.bNegate = %d\n", out->ALU.bNegate);
	printf("  ID_EX.ALU.op      = %d\n", out->ALU.op);
	printf("  ---\n");
	printf("  ID_EX.memRead  = %d\n", out->memRead);
	printf("  ID_EX.memWrite = %d\n", out->memWrite);
	printf("  ID_EX.memToReg = %d\n", out->memToReg);
	printf("  ---\n");
	printf("  ID_EX.regDst   = %d\n", out->regDst);
	printf("  ID_EX.regWrite = %d\n", out->regWrite);

	printf("\n");

	return rc;
}



void Test_EX(ID_EX *in, EX_MEM *out, EX_MEM *old_exMem, MEM_WB *old_memWb)
{
	ID_EX in_save;
	  memcpy(&in_save, in, sizeof(in_save));

	memset(out, -1, sizeof(*out));

	// if the caller didn't provide an old EX/MEM or MEM/WB, then we'll
	// provide a NOP.
	EX_MEM nop_exMem;
	MEM_WB nop_memWb;
	  memset(&nop_exMem, 0, sizeof(nop_exMem));
	  memset(&nop_memWb, 0, sizeof(nop_memWb));

	if (old_exMem == NULL)
		old_exMem = &nop_exMem;
	if (old_memWb == NULL)
		old_memWb = &nop_memWb;

	EX_MEM old_exMem_save;
	MEM_WB old_memWb_save;
	  memcpy(&old_exMem_save, old_exMem, sizeof(old_exMem_save));
	  memcpy(&old_memWb_save, old_memWb, sizeof(old_memWb_save));


	// there's no need for a preliminary printf(), because we probably
	// just finished Test_ID().


	WORD input1 = EX_getALUinput1(in, old_exMem, old_memWb);
	WORD input2 = EX_getALUinput2(in, old_exMem, old_memWb);
	printf("  ALU input1 = 0x%04x_%04x\n",
	       (input1 >> 16) & 0xffff, input1 & 0xffff);
	printf("  ALU input2 = 0x%04x_%04x\n",
	       (input2 >> 16) & 0xffff, input2 & 0xffff);

	execute_EX(in, input1,input2, out);


	if (memcmp(&in_save, in, sizeof(*in)) != 0)
		printf("ERROR: execute_EX() modified the (old) ID_EX struct.\n");
	if (memcmp(&old_exMem_save, old_exMem, sizeof(*old_exMem)) != 0)
		printf("ERROR: execute_EX() modified the (old) EX_MEM struct.\n");
	if (memcmp(&old_memWb_save, old_memWb, sizeof(*old_memWb)) != 0)
		printf("ERROR: execute_EX() modified the (old) MEM_WB struct.\n");


	printf("  ---\n");
	printf("  EX_MEM.rt = %d\n", out->rt);
	printf("  EX_MEM.rtVal = 0x%04x_%04x\n",
	       (out->rtVal >> 16) & 0xffff,
	       (out->rtVal      ) & 0xffff);
	printf("  ---\n");
	printf("  EX_MEM.memRead  = %d\n", out->memRead);
	printf("  EX_MEM.memWrite = %d\n", out->memWrite);
	printf("  EX_MEM.memToReg = %d\n", out->memToReg);
	printf("  ---\n");
	printf("  EX_MEM.writeReg = %d\n", out->writeReg);
	printf("  EX_MEM.regWrite = %d\n", out->regWrite);
	printf("  ---\n");
	printf("  EX_MEM.aluResult = 0x%04x_%04x\n",
	       (out->aluResult >> 16) & 0xffff, out->aluResult & 0xffff);
	printf("\n");

	return;
}



void Test_MEM(EX_MEM *in, MEM_WB *old_memWb,
              WORD *mem, int memSizeWords, MEM_WB *out)
{
	// if the caller didn't provide an old MEM/WB, then we'll
	// provide a NOP.
	MEM_WB nop_memWb;
	  memset(&nop_memWb, 0, sizeof(nop_memWb));

	if (old_memWb == NULL)
		old_memWb = &nop_memWb;

	EX_MEM in_save;
	  memcpy(&in_save, in, sizeof(in_save));
	MEM_WB old_memWb_save;
	  memcpy(&old_memWb_save, old_memWb, sizeof(old_memWb_save));

	WORD *mem_save = malloc(sizeof(WORD)*memSizeWords);
	  memcpy(mem_save, mem, sizeof(WORD)*memSizeWords);

	memset(out, -1, sizeof(*out));


	execute_MEM(in, old_memWb, mem, out);


	if (memcmp(&in_save, in, sizeof(in_save)) != 0)
		printf("ERROR: execute_MEM() modified the (old) EX_MEM struct.\n");
	if (memcmp(&old_memWb_save, old_memWb, sizeof(old_memWb_save)) != 0)
		printf("ERROR: execute_MEM() modified the (old) MEM_WB struct.\n");


	// memory modifications are normal - at least, for SW instructions
	int i;
	for (i=0; i<memSizeWords; i++)
	if (mem[i] != mem_save[i])
	{
		int addr = i*4;

		printf("  MEM CHANGED: Address=0x%04x_%04x   Was: 0x%04x_%04x Now: 0x%04x_%04x\n",
		       (addr        >> 16) & 0xffff,
		       (addr             ) & 0xffff,
		       (mem_save[i] >> 16) & 0xffff,
		       (mem_save[i]      ) & 0xffff,
		       (mem     [i] >> 16) & 0xffff,
		       (mem     [i]      ) & 0xffff);
		printf("\n");
	}


	printf("  MEM_WB.memToReg  = %d\n", out->memToReg);
	printf("  MEM_WB.aluResult = 0x%04x_%04x\n",
	       (out->aluResult >> 16) & 0xffff,
	       (out->aluResult      ) & 0xffff);
	printf("  MEM_WB.memResult = 0x%04x_%04x\n",
	       (out->memResult >> 16) & 0xffff,
	       (out->memResult      ) & 0xffff);
	printf("  ---\n");
	printf("  MEM_WB.writeReg = %d\n", out->writeReg);
	printf("  MEM_WB.regWrite = %d\n", out->regWrite);
	printf("\n");

	free(mem_save);
	return;
}



void Test_WB(MEM_WB *in, WORD *regs)
{
	MEM_WB in_save;
	  memcpy(&in_save, in, sizeof(in_save));

	WORD regs_save[34];
	  memcpy(regs_save, regs, sizeof(regs_save));


	execute_WB(in, regs);


	if (memcmp(&in_save, in, sizeof(in_save)) != 0)
		printf("ERROR: execute_WB() modified the (old) MEM_WB struct.\n");


	// reigster modifications are normal, for many instructions
	int count = 0;
	int i;
	for (i=0; i<34; i++)
	if (regs[i] != regs_save[i])
	{
		count++;

		printf("  REGISTER %d CHANGED.  Was: 0x%04x_%04x Now: 0x%04x_%04x\n",
		       i,
		       (regs_save[i] >> 16) & 0xffff,
		       (regs_save[i]      ) & 0xffff,
		       (regs     [i] >> 16) & 0xffff,
		       (regs     [i]      ) & 0xffff);
	}

	printf("  A total of %d registers were changed.\n", count);
	printf("\n");

	return;
}



void Test_Pipelined_FixedSequence(WORD *instructions, WORD *pcs, int instCount,
                                  WORD *regs,
                                  WORD *dataMemory, int dataMemorySizeWords)
{
	ID_EX  idex [2];
	EX_MEM exmem[2];
	MEM_WB memwb[2];

	printf("+-----------------------------------------------+\n");
	printf("| Initializing the EX, MEM, WB phases to NOP... |\n");
	printf("+-----------------------------------------------+\n");
	printf("\n");

	memset(idex , 0, sizeof(idex));
	memset(exmem, 0, sizeof(exmem));
	memset(memwb, 0, sizeof(memwb));

	int i;
	for (i=0; i<instCount; i++)
	{
		// for every iteration of the pipeline, we run the WB phase
		// *FIRST* (to update registers).
		//
		// For the rest, the order is kind of arbitrary, since we
		// always read from [0] and write to [1].  However, we'll
		// choose to do the conventional ID, EX, MEM, just because it
		// looks nice.

		printf("-------------- Clock %d ---------------\n", i);
		printf("WB phase:\n");
		Test_WB(&memwb[0], regs);

		printf("ID phase:\n");
		Test_ID(pcs[i], instructions[i], regs, &idex[1], &idex[0]);

		printf("EX phase:\n");
		Test_EX (&idex[0], &exmem[1], &exmem[0], &memwb[0]);

		printf("MEM phase:\n");
		Test_MEM(&exmem[0], &memwb[0], dataMemory, dataMemorySizeWords, &memwb[1]);

		// copy each [1] back into [0] to be the input for the next
		// clock cycle.
		memcpy(&idex [0], &idex [1], sizeof(idex [0]));
		memcpy(&exmem[0], &exmem[1], sizeof(exmem[0]));
		memcpy(&memwb[0], &memwb[1], sizeof(memwb[0]));
	}

	return;
}



void Test_FullProcessor(WORD *instMemory, int instMemSizeWords,
                        WORD *regs,
                        WORD *dataMemory, int dataMemSizeWords,
                        WORD  codeOffset)
{
	/* this is largely similar to Test_PIpelined_FixedSequence(), except
	 * that instead of having an array of IF/ID registers already filled
	 * in, we dynamically fill one, basd on the current PC.
	 *
	 * Also, this supports syscalls.
	 */

	WORD   instructions[2], pcs[2];
	ID_EX  idex [2];
	EX_MEM exmem[2];
	MEM_WB memwb[2];

	/* initialize all of the pipeline registers (except for ID/EX) to
	 * zero (NOP)
	 */
	instructions[0] = instructions[1] = pcs[0] = pcs[1] = 0;
	memset(idex , 0, sizeof(idex));
	memset(exmem, 0, sizeof(exmem));
	memset(memwb, 0, sizeof(memwb));

	/* except, that the first IF/ID must be initialized with the 
	 * starting PC.
	 */
	pcs[0]          = codeOffset;
	instructions[0] = instMemory[0];

	int clock = 0;
	while (1)
	{
		// for every iteration of the pipeline, we run the WB phase
		// *FIRST* (to update registers).
		//
		// For the rest, the order is kind of arbitrary, since we
		// always read from [0] and write to [1].  However, we'll
		// choose to do the conventional ID, EX, MEM, just because it
		// looks nice.

		printf("-------------- Clock %d ---------------\n", clock);
		printf("WB phase:\n");
		Test_WB(&memwb[0], regs);

		printf("ID phase:\n");

		int stall, branchControl;
		WORD rsVal, rtVal;
		WORD branchAddr, jumpAddr;

		if (instructions[0] == SYSCALL())
		{
			// handle syscalls locally
			if (execSyscall(regs, dataMemory) != 0)
				return;

			// turn it into a NOP
			stall = 0;
			branchControl = 0;
			memset(&idex[1], 0, sizeof(idex[1]));
		}
		else
		{
			InstructionFields fields;
			extract_instructionFields(instructions[0], &fields);

			// see above.  [0] is the *OLD* value for the
			// pipeline register, and [1] is the *NEW*
			stall = IDtoIF_get_stall(&fields, &idex[0], &exmem[0]);

			rsVal = regs[fields.rs];
			rtVal = regs[fields.rt];

			branchControl = IDtoIF_get_branchControl(&fields, rsVal,rtVal);

			branchAddr = calc_branchAddr(pcs[0]+4, &fields);
			jumpAddr   = calc_jumpAddr  (pcs[0]+4, &fields);

			// orinary instructions
			int rc = Test_ID( pcs[0]+4, instructions[0],
			                  regs,
			                 &idex[1],
			                 &idex[0]);
			if (rc == 0)
			{
				printf("Test_FullProcessor(): Ending program because execute_ID() returned %d\n", rc);
				return;
			}
		}

		// figure out the proper PC for the new IF/ID; also, fill in
		// the proper instruction.
		if (stall)
		{
			/* in a stall, the IF/ID register doesn't change;
			 * nor do the program counter or instruction
			 */
			instructions[1] = instructions[0];
			pcs         [1] = pcs         [0];
		}
		else
		{
			if (branchControl == 1)
				pcs[1] = branchAddr;
			else if (branchControl == 2)
				pcs[1] = jumpAddr;
			else if (branchControl == 3)
				pcs[1] = rsVal;
			else
				pcs[1] = pcs[0]+4;

			int instIndx = (pcs[1] - codeOffset)/4;

			if (instIndx <  0                ||
			    instIndx >= instMemSizeWords ||
			    pcs[1] % 4 != 0)
			{
				printf("ERROR: Invalid Program Counter 0x%08x\n", pcs[0]);
				return;
			}

			instructions[1] = instMemory[instIndx];
		}

		printf("EX phase:\n");
		Test_EX (&idex[0], &exmem[1], &exmem[0], &memwb[0]);

		printf("MEM phase:\n");
		Test_MEM(&exmem[0], &memwb[0], dataMemory, dataMemSizeWords, &memwb[1]);


		// copy each [1] back into [0] to be the input for the next
		// clock cycle.
		instructions[0] = instructions[1];
		pcs         [0] = pcs         [1];
		memcpy(&idex [0], &idex [1], sizeof(idex [0]));
		memcpy(&exmem[0], &exmem[1], sizeof(exmem[0]));
		memcpy(&memwb[0], &memwb[1], sizeof(memwb[0]));

		clock++;
	}
}



void ExecProcessor(WORD *instMemory, int instMemSizeWords,
                   WORD *regs,
                   WORD *dataMemory, int dataMemSizeWords,
                   WORD  codeOffset)
{
	/* this is basically the same function as Test_FullProcessor(),
	 * except that it calls the user functions directly, instead of
	 * using the Test*() functions (which do too much printing).
	 */

	WORD   instructions[2], pcs[2];
	ID_EX  idex [2];
	EX_MEM exmem[2];
	MEM_WB memwb[2];

	instructions[0] = instructions[1] = pcs[0] = pcs[1] = 0;
	memset(idex , 0, sizeof(idex));
	memset(exmem, 0, sizeof(exmem));
	memset(memwb, 0, sizeof(memwb));

	pcs[0]          = codeOffset;
	instructions[0] = instMemory[0];

	while (1)
	{
		execute_WB(&memwb[0], regs);

		int stall, branchControl;
		WORD rsVal, rtVal;
		WORD branchAddr, jumpAddr;

		if (instructions[0] == SYSCALL())
		{
			if (execSyscall(regs, dataMemory) != 0)
				return;

			/* pretend that "something" happened - and that
			 * NOP is the correct operation to pass forward
			 * through EX.
			 */
			stall = 0;
			branchControl = 0;
			memset(&idex[1], 0, sizeof(idex[1]));
		}
		else
		{
			InstructionFields fields;
			extract_instructionFields(instructions[0], &fields);

			// see above.  [0] is the *OLD* value for the
			// pipeline register, and [1] is the *NEW*
			stall = IDtoIF_get_stall(&fields, &idex[0], &exmem[0]);

			rsVal = regs[fields.rs];
			rtVal = regs[fields.rt];

			branchControl = IDtoIF_get_branchControl(&fields, rsVal,rtVal);

			branchAddr = calc_branchAddr(pcs[0]+4, &fields);
			jumpAddr   = calc_jumpAddr  (pcs[0]+4, &fields);

			int rc = execute_ID( stall,
			                    &fields,
			                     pcs[0]+4, rsVal,rtVal,
			                    &idex[1]);
			if (rc == 0)
			{
				printf("ExecProcessor(): Ending program because execute_ID() returned %d\n", rc);
				return;
			}
		}

		if (stall)
		{
			/* in a stall, the IF/ID register doesn't change;
			 * nor do the program counter or instruction
			 */
			instructions[1] = instructions[0];
			pcs         [1] = pcs         [0];
		}
		else
		{
			if (branchControl == 1)
				pcs[1] = branchAddr;
			else if (branchControl == 2)
				pcs[1] = jumpAddr;
			else if (branchControl == 3)
				pcs[1] = rsVal;
			else
				pcs[1] = pcs[0]+4;

			int instIndx = (pcs[1] - codeOffset)/4;

			if (instIndx <  0                ||
			    instIndx >= instMemSizeWords ||
			    pcs[1] % 4 != 0)
			{
				printf("ERROR: Invalid Program Counter 0x%08x\n", pcs[0]);
				return;
			}

			instructions[1] = instMemory[instIndx];
		}

		WORD aluInput1 = EX_getALUinput1(&idex[0], &exmem[0], &memwb[0]);
		WORD aluInput2 = EX_getALUinput2(&idex[0], &exmem[0], &memwb[0]);

		execute_EX (&idex [0], aluInput1,aluInput2, &exmem[1]);
		execute_MEM(&exmem[0], &memwb[0], dataMemory, &memwb[1]);


		// copy each [1] back into [0] to be the input for the next
		// clock cycle.
		memcpy(&instructions[0], &instructions[1],
		                      sizeof(instructions[0]));
		memcpy(&pcs  [0], &pcs  [1], sizeof(pcs  [0]));
		memcpy(&idex [0], &idex [1], sizeof(idex [0]));
		memcpy(&exmem[0], &exmem[1], sizeof(exmem[0]));
		memcpy(&memwb[0], &memwb[1], sizeof(memwb[0]));
	}
}



int execSyscall(WORD *regs, WORD *dataMemory)
{
	WORD v0 = regs[2];
	WORD a0 = regs[4];

	// syscall 10: exit
	if (v0 == 10)
	{
		printf("--- syscall 10 executed: Normal termination of the assembly language program.\n");
		return 1;
	}


	// syscall 1: print_int
	if (v0 == 1)
		printf("%d", a0);

	// syscall 11: print_char
	else if (v0 == 11)
		printf("%c", a0);


	// syscall 4: print_str
	else if (v0 == 4)
		printf("%s", ((char*)dataMemory)+a0);


	// unrecognized syscall
	else
		printf("--- ERROR: Unrecognized syscall $v0=%d\n", v0);

	return 0;
}


