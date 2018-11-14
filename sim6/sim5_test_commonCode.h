#ifndef TEST_COMMONCODE_H
#define TEST_COMMONCODE_H


#include "sim5.h"



/* these functions test the various stages of the processor. */

int Test_ID(WORD pcPlus4, WORD instruction,
            WORD *regs,
            ID_EX *out,
            ID_EX *old_idex);
void Test_EX (ID_EX *in, EX_MEM *out, EX_MEM *old_exMem, MEM_WB *old_memWb);
void Test_MEM(EX_MEM *in, MEM_WB *old_memWb, WORD *mem, int memSizeWords, MEM_WB *out);
void Test_WB (MEM_WB *in, WORD *regs);

/* this function tests the full processor in pipelined mode, but it uses a
 * hard-coded sequence of instructions (it ignores branching).  The caller
 * provides pre-filled IF/ID registers (including PC values).
 */
void Test_Pipelined_FixedSequence(WORD *instructions, WORD *pcs, int instCount,
                                  WORD *regs,
                                  WORD *dataMemory, int dataMemSizeWords);

/* this function tests a realistic processor: the caller gives a series of
 * INSTRUCTIONS (in memory), and branches are honored.
 *
 * This terminates on an 'error' (from execute_ID()) or on syscall 10.
 *
 * NOTE: It includes a 'codeOffset' field; this is the effective address of
 * the instruction at element [0].
 */
void Test_FullProcessor(WORD *instMemory, int instMemSizeWords,
                        WORD *regs,
                        WORD *dataMemory, int dataMemSizeWords,
                        WORD  codeOffset);


/* this version simply executes the instructions, in a pipelined fashion,
 * without any debugging printfs.  It's a stripped-down version of
 * Test_FullProcessor()
 */
void ExecProcessor(WORD *instMemory, int instMemSizeWords,
                   WORD *regs,
                   WORD *dataMemory, int dataMemSizeWords,
                   WORD  codeOffset);



/* a helper function, used by some of the simulator functions above. */
int execSyscall(WORD *regs, WORD *dataMemory);


/* these macros are useful for encoding instructions.
 *
 * The first few are macros that allow us to generate some register
 * numbers.
 */
#define S_REG(X) ((X)+16)
#define T_REG(X) ((X) < 8 ? (X)+8 : (X)-8+24)    // t0 is 8; t8 is 24
#define A_REG(X) ((X)+4)
#define V_REG(X) ((X)+2)
#define RA_REG    31
#define SP_REG    29
#define FP_REG    30
#define REG_ZERO   0


/* The next four are macros that represent the three instruction formats.
 *
 * Note that all of the R-format instructions use R_FORMAT(); R_FORMAT_x()
 * is used only by MUL, which has a different opcode.
 */

#define R_FORMAT(funct, rs,rt,rd, shamt) R_FORMAT_x(0x00, funct, rs,rt,rd, shamt)

#define R_FORMAT_x(opcode, funct, rs,rt,rd, shamt)                         \
                                          (((opcode)            << 26)  |  \
                                          (((rs    ) &    0x1f) << 21)  |  \
                                          (((rt    ) &    0x1f) << 16)  |  \
                                          (((rd    ) &    0x1f) << 11)  |  \
                                          (((shamt ) &    0x1f) <<  6)  |  \
                                          (((funct ) &    0x3f) <<  0))

#define I_FORMAT(opcode, rs,rt, imm16)   ((((opcode) &    0x3f) << 26)  |  \
                                          (((rs    ) &    0x1f) << 21)  |  \
                                          (((rt    ) &    0x1f) << 16)  |  \
                                          (((imm16 ) & 0x0ffff) <<  0))

#define J_FORMAT(opcode, addr26)         ((((opcode) &      0x3f) << 26)  |    \
                                          (((addr26) & 0x3ffffff) <<  0))



/* these macros encode various instructions.  Each uses the format macros
 * above to actually do the encoding.
 */
#define  NOP()               0    // a 32-bit 0 constant is NOP in MIPS!

#define  ADD(rd, rs,rt)      R_FORMAT(32, rs,rt,rd, 0)
#define  ADDU(rd, rs,rt)     R_FORMAT(33, rs,rt,rd, 0)
#define  SUB(rd, rs,rt)      R_FORMAT(34, rs,rt,rd, 0)
#define  SUBU(rd, rs,rt)     R_FORMAT(35, rs,rt,rd, 0)
#define ADDI(rt, rs,imm16)   I_FORMAT( 8, rs,rt,imm16)
#define ADDIU(rt, rs,imm16)  I_FORMAT( 9, rs,rt,imm16)

#define  MUL(rd, rs,rt)      R_FORMAT_x(0x1c,  2, rs,rt,rd, 0)
#define  MULT(rs,rt)         R_FORMAT  (      24, rs,rt, 0,0)
#define  MULTU(rs,rt)        R_FORMAT  (      25, rs,rt, 0,0)
#define  DIV(rs,rt)          R_FORMAT  (      26, rs,rt, 0,0)
#define  DIVU(rs,rt)         R_FORMAT  (      27, rs,rt, 0,0)

#define  MFHI(rd)            R_FORMAT(16, 0,0, rd, 0)
#define  MFLO(rd)            R_FORMAT(18, 0,0, rd, 0)

#define  AND(rd, rs,rt)      R_FORMAT(36, rs,rt,rd, 0)
#define   OR(rd, rs,rt)      R_FORMAT(37, rs,rt,rd, 0)
#define  NOR(rd, rs,rt)      R_FORMAT(39, rs,rt,rd, 0)
#define ANDI(rt, rs,imm16)   I_FORMAT(12, rs,rt,imm16)
#define  ORI(rt, rs,imm16)   I_FORMAT(13, rs,rt,imm16)

#define  LUI(rt, imm16)      I_FORMAT(15,  0,rt,imm16)

#define   SLT(rd, rs,rt)     R_FORMAT(42, rs,rt,rd, 0)
#define  SLTU(rd, rs,rt)     R_FORMAT(43, rs,rt,rd, 0)
#define  SLTI(rt, rs,imm16)  I_FORMAT(10, rs,rt,imm16)
#define SLTIU(rt, rs,imm16)  I_FORMAT(11, rs,rt,imm16)

#define  SLL(rd, rt,shamt)   R_FORMAT( 0,  0,rt,rd, shamt)
#define  SRL(rd, rt,shamt)   R_FORMAT( 2,  0,rt,rd, shamt)
#define  SRA(rd, rt,shamt)   R_FORMAT( 3,  0,rt,rd, shamt)
#define SLLV(rd, rt,rs)      R_FORMAT( 4, rs,rt,rd, 0)
#define SRLV(rd, rt,rs)      R_FORMAT( 6, rs,rt,rd, 0)
#define SRAV(rd, rt,rs)      R_FORMAT( 7, rs,rt,rd, 0)

#define LW(rt, rs,imm16)     I_FORMAT(35, rs,rt,imm16)
#define SW(rt, rs,imm16)     I_FORMAT(43, rs,rt,imm16)

#define LB(rt, rs,imm16)     I_FORMAT(32, rs,rt,imm16)
#define SB(rt, rs,imm16)     I_FORMAT(40, rs,rt,imm16)

#define BEQ(rs,rt, imm16)    I_FORMAT(4,  rs,rt,imm16)
#define BNE(rs,rt, imm16)    I_FORMAT(5,  rs,rt,imm16)
#define   J(address)         J_FORMAT(2,  address)

#define JAL(address)         J_FORMAT(3,  address)
#define  JR(rs)              R_FORMAT(8,  rs, 0,0,0)

#define SYSCALL()            R_FORMAT(12, 0,0,0, 0)


#endif

