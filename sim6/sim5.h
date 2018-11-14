#ifndef __SIM5_H__INCLUDED__
#define __SIM5_H__INCLUDED__



typedef int WORD;



// TODO: add forwarding into the MEM phase (for sw)



/* duplicated from Hardware Project 4 */
typedef struct InstructionFields
{
	// THESE ARE NOT ACTUALLY CONTROL BITS
	//
	// However, you have to fill them out.  Fill out *all* of these fields
	// for *all* instructions, even if it doesn't use that particular
	// field.  For instance, opcode 0 means that it is an R format
	// instruction - but you *still* must set the proper values for imm16,
	// imm32, and address.
	//
	// NOTE: imm16 is the field from the instruction,  imm32 is the
	//       sign-extended version.
	int opcode;
	int rs;
	int rt;
	int rd;
	int shamt;
	int funct;
	int imm16, imm32;
	int address;		// this is the 26 bit field from the J format
} InstructionFields;



typedef struct ID_EX
{
	// these are the register *NUMBERS* (5 bits each), which
	// were mapped directly from the instruction fields.
	int rs,rt,rd;

	// these are the two register *VALUES*, which were read from the
	// register file.
	WORD rsVal, rtVal;


	// save the immediate fields for use in the EX phase
	int imm16, imm32;


	// these are control bits, which were generated based on the
	// opcode/funct.  They are the same as in Simulation 4 -
	// except that the branch/jump fields have been removed.
	//
	// Note that we still have the 3 'extra' fields, for you to use
	// however you wish.

	int ALUsrc;
	struct {
		int bNegate;
		int op;
	} ALU;

	int memRead;
	int memWrite;
	int memToReg;

	int regDst;
	int regWrite;

	WORD extra1, extra2, extra3;
} ID_EX;



typedef struct EX_MEM
{
	// these fields are (roughly) the same ones from the previous phase,
	// minus the ones that we've used, plus the ALU result.  Note that
	// we've used the 'regDst' (1 bit) control to choose the destination
	// register, which is now the (5 bit) 'destReg' field.

	int  rt;         // necessary for forwarding *into* SW instructions
	WORD rtVal;      // carries the data-to-write for SW

	int memRead, memWrite, memToReg;      // 1 bit each

	int writeReg;   // 5 bits
	int regWrite;   // 1 bit

	WORD aluResult;

	// If you need extra fields, I have them available for you...
	WORD extra1, extra2, extra3;
} EX_MEM;



typedef struct MEM_WB
{
	// in the official MIPS design, we carry the ALUresult and MEMresult
	// into the WB phase - and then we use memToReg in that phase.  Do
	// the same in your code.
	int  memToReg;
	WORD aluResult;
	WORD memResult;

	int writeReg;    // 5 bits
	int regWrite;    // 1 bit

	WORD extra1, extra2, extra3;
} MEM_WB;




/* ------------------ EXECUTE METHODS -----------------------
 *
 * Each of these methods takes a pointer to the "input" pipeline register,
 * and also (except for WB) to the "output" pipeline register.  Read
 * values from the input, and set the proper bits in the output.
 *
 * Several of the phases also take pointers to *OTHER* phases.  This allows
 * you to read the current state of other pipeline registers, so that you
 * can do (for instance) data forwarding and LW stalling.
 *
 * A couple of phases (ID, WB) also take a pointer to the array of registers.
 * This is a 34 (not 32!) element array: the last two are 'lo' and 'hi'.  If
 * you choose to implement the 'mult' or 'div' instructions, use these two
 * registers.  Otherwise, they should never change.
 *
 * MEM, of course, gets a pointer to the array of data words.
 *
 * The testcases will implement the IF phase for you.
 */



/* in the ID phase, you have a return value: return 0 to indicate "OK" and
 * 1 to indicate "ERROR".  (This is the equivalent of the 'err' field from
 * Project 9.)
 *
 * The rest of the phases have no ability to report an error; they simply
 * must do their work.
 */

void extract_instructionFields(WORD instruction, InstructionFields *fieldsOut);

int IDtoIF_get_stall(InstructionFields *fields,
                     ID_EX  *old_idex, EX_MEM *old_exmem);
int IDtoIF_get_branchControl(InstructionFields *fields, WORD rsVal, WORD rtVal);

WORD calc_branchAddr(WORD pcPlus4, InstructionFields *fields);
WORD calc_jumpAddr  (WORD pcPlus4, InstructionFields *fields);

int execute_ID(int IDstall,
               InstructionFields *fieldsIn,
               WORD pcPlus4,
               WORD rsVal, WORD rtVal,
               ID_EX *new_idex);

WORD EX_getALUinput1(ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb);
WORD EX_getALUinput2(ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb);

void execute_EX(ID_EX *in, WORD input1, WORD input2,
                EX_MEM *new_exMem);

void execute_MEM(EX_MEM *in, MEM_WB *old_memWb,
                 WORD *mem, MEM_WB *new_memwb);

void execute_WB (MEM_WB *in, WORD *regs);



/* HELPER FUNCTIONS THAT YOU CAN CALL */

static inline WORD signExtend16to32(int val16)
{
	if (val16 & 0x8000)
		return val16 | 0xffff0000;
	else
		return val16;
}


#endif

