/*
 * Name: Yang Zhou
 * Assignment: Sim4
 * todo: to build a pipelined processor by adapting sim4.c
 */
#include"sim5.h"
#include"sim5_test_commonCode.h"
#include<stdio.h>


/*
 * input:WORD instruction, InstructionFields *fieldsOut
 * todo: Exact some controls from the instruction, such as opcode,rs,rt,rd ...
 */
void extract_instructionFields(WORD instruction, InstructionFields *fieldsOut){
    fieldsOut->opcode=(instruction>>26) & 0x3f;   //the opcode is first 6 bits, from 0 to 5

    fieldsOut->rs=(instruction>>21) & 0x1f;       //the rs is 5 bits, and it is from 6 to 10
    fieldsOut->rt=(instruction>>16) & 0x1f;       //the rt is 5 bits, and it is from 11 to 16
    fieldsOut->rd=(instruction>>11) & 0x1f;       //the rs is 5 bits, and it is from 16 to 20
 
    fieldsOut->shamt=(instruction>>6) & 0x1f;     //the shamt is 5 bits, and it is from 21 to 25

    fieldsOut->funct=instruction & 0x3f;          //the funct is last 6 bits, and it is from 26 to 31
 
    fieldsOut->address=instruction & 0x3ffffff;   //the insctruction of j.

    fieldsOut->imm16=instruction & 0xffff;        //imm16 is 16 bits, from 16 to 31

    fieldsOut->imm32=signExtend16to32(instruction & 0xffff);  //imm(32 bits.)
}

/*
 * inputs: InstructionFields *fields, ID_EX  *old_idex, EX_MEM *old_exmem
 * todo: Determin if there is a data hazards and if we need a stall
 */
int IDtoIF_get_stall(InstructionFields *fields, ID_EX  *old_idex, EX_MEM *old_exmem){   
    if(old_exmem->regWrite && fields->opcode == 0x2b) //0x2b=sw
        return 1;
    
    if(old_idex->memRead ==0)
        return 0;
        
    //If R instruction,
    if(fields->opcode==0){
        //add
        if(fields->funct==32){    
          return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }
        //addu
        else if(fields->funct==33){
          return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }
        //sub
        else if(fields->funct==34){
          return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }
        //subu
        else if(fields->funct==35){
          return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }
        //and
        else if(fields->funct==36){
          return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }
        //or
        else if(fields->funct==37){
          return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }
        //nor
        else if(fields->funct==39){
            return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }     
        //slt
        else if(fields->funct==42){
          return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);
        }

        // invalid.
        else   
          return 0;
        
    }


    // I format instruction, the input is only rs
    if(fields->opcode==43){
      return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);

    } else if(fields->opcode==4){        //beq
        return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);

    } else if(fields->opcode==5){        //bne
        return (old_idex->rt==fields->rs||old_idex->rt==fields->rt);

    } else if(fields->opcode==8){        // addi
        return (old_idex->rt==fields->rs);

    } else if(fields->opcode==9){        //addiu
        return (old_idex->rt==fields->rs);

    } else if(fields->opcode==10){       //slti
        return (old_idex->rt==fields->rs);

    } else if(fields->opcode==12){       //andi
        return (old_idex->rt==fields->rs);

    } else if(fields->opcode==13){       //ori
        return (old_idex->rt==fields->rs);

    } else if(fields->opcode==35){       //lw
        return(old_idex->rt==fields->rs);
    }
    return 0;  //other situation.
}

/*
 * inputs: InstructionFields *fields, WORD rsVal, WORD rtVal
 * todo:Check if the current instruction need to do a branch or jump.
 */
int IDtoIF_get_branchControl(InstructionFields *fields, WORD rsVal, WORD rtVal){
  //R format instruction
  if(fields->opcode != 0){  
    //beq:if the instruction is beq,
    //when the two values are equal, it will return 1 and jump to its relative address. otherwise return 0; 
     if(fields->opcode==4){   
      return rsVal==rtVal;
    }

    //bne: do the same thing with beq, but just reverse thing, when not equal return 1, equal return 0.
    else if(fields->opcode==5){
      return rsVal!=rtVal ;
    }

    //j, it needs the absolute address.
    else if(fields->opcode==2){
      return 2;
    }   
  }
   return 0;
}
/*
 * inputs: WORD pcPlus4, InstructionFields *fields
 * todo:Which is the address we want to jump to if it is a branch jump.
 */
WORD calc_branchAddr(WORD pcPlus4, InstructionFields *fields){
    return pcPlus4+(fields->imm32<<2);
}

//If it is a j jump, which address we want to jump to.
WORD calc_jumpAddr  (WORD pcPlus4, InstructionFields *fields){
    return (((pcPlus4-4)>>28)<<28)|((fields->address)<<2);
}

/*
 *inputs:int IDstall, InstructionFields *fieldsIn, WORD pcPlus4, WORD rsVal, WORD rtVal, ID_EX *new_idex
 *todo:Implement the main function of ID phase, input is two pipleline register and the output is also stored in the pipeline register.
 */
int execute_ID(int IDstall, InstructionFields *fieldsIn, WORD pcPlus4, WORD rsVal, WORD rtVal, ID_EX *new_idex){

  if(IDstall){
    new_idex->rs=0;
    new_idex->rt=0;
    new_idex->rd=0;
    new_idex->rsVal=0;
    new_idex->rtVal=0;
    new_idex->imm16=0;
    new_idex->imm32=0;
    new_idex->ALUsrc=0;
    new_idex->ALU.bNegate=0;
    new_idex->ALU.op=0;
    new_idex->memRead=0;
    new_idex->memWrite=0;
    new_idex->memToReg=0;
    new_idex->regDst=0;
    new_idex->regWrite=0;
    new_idex->extra1=0;
    new_idex->extra2=0;
    new_idex->extra3=0;
    return 1;
  }
   
    new_idex->rs=fieldsIn->rs;
    new_idex->rt=fieldsIn->rt;
    new_idex->rd=fieldsIn->rd;
    new_idex->rsVal=rsVal;
    new_idex->rtVal=rtVal;
    new_idex->imm16=fieldsIn->imm16;
    new_idex->imm32=fieldsIn->imm32;

    new_idex->regDst=1;
    new_idex->regWrite=1;
    new_idex->extra1=0;
    new_idex->extra2=0;
    new_idex->extra3=0; 

    //R format instruction.
    if(fieldsIn->opcode==0){  
     
       if(fieldsIn->funct==0){      
        new_idex->ALU.op=4;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;

      }else if(fieldsIn->funct==32){     //add
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;

      }  else if(fieldsIn->funct==33){  //addu
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        
      }else if(fieldsIn->funct==34){  //sub
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=1;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        
      }else if(fieldsIn->funct==35){ //subu
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=1;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        
      } else if(fieldsIn->funct==36){ //and
        new_idex->ALU.op=0;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        
      } else if(fieldsIn->funct==37){ //or
        new_idex->ALU.op=1;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        
      } else if(fieldsIn->funct==42){ //slt
        new_idex->ALU.op=3;
        new_idex->ALU.bNegate=1;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
      } else if(fieldsIn->funct==39){ //nor
        new_idex->ALU.op=1;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->extra1=1; 
        
      } else{
        return 0;//Invalid.
      }

      // I format instruction
    } else{ 
    
      if(fieldsIn->opcode==4 || fieldsIn->opcode==5 || fieldsIn->opcode==2){
        new_idex->rs=0;
        new_idex->rt=0;
        new_idex->rd=0;
        new_idex->rsVal=0;
        new_idex->rtVal=0;
        new_idex->ALU.op=0;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=0;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=0;
        new_idex->extra1=0;
        new_idex->extra2=0;
        new_idex->extra3=0;
      }else if(fieldsIn->opcode==8){ //addi
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=1;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=1;
      }
      else if(fieldsIn->opcode==9){ //addiu
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=1;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=1;
      } else if(fieldsIn->opcode==10){    //slti
        new_idex->ALU.op=3;
        new_idex->ALU.bNegate=1;
        new_idex->ALUsrc=1;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=1;
      } else if(fieldsIn->opcode==12){  //andi
        new_idex->ALU.op=0;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=2;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=1;
        new_idex->extra2=1;
      }  else if(fieldsIn->opcode==13){  //ori
        new_idex->ALU.op=1;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=2;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=1;  
      }

      //lui
      else if(fieldsIn->opcode==15){
        new_idex->ALU.op=1;
        new_idex->ALU.bNegate=0;

        //when the lui is executed, ALUsrc will be 3.
        new_idex->ALUsrc=2;
        new_idex->memWrite=0;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=1;
        new_idex->extra3=1;// lui.
      }

      //lw
      else if(fieldsIn->opcode==35){ 
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=1;
        new_idex->memWrite=0;
        new_idex->memRead=1;
        new_idex->memToReg=1;
        new_idex->regDst=0;
        new_idex->regWrite=1;
      }

      //sw
      else if(fieldsIn->opcode==43){ 
        new_idex->ALU.op=2;
        new_idex->ALU.bNegate=0;
        new_idex->ALUsrc=1;
        new_idex->memWrite=1;
        new_idex->memRead=0;
        new_idex->memToReg=0;
        new_idex->regDst=0;
        new_idex->regWrite=0;
      } 
      else
        return 0;//Invalid;
    }
    return 1;
}

//Take two pipeline register as input, return the correct ALU input1. 
//Including the case when data forwarding happen.There are several cases.1:data forwading from ALU output2: dataforwarding from MEM phase
WORD EX_getALUinput1(ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb){

    if (old_exMem->regWrite && old_exMem->writeReg == in->rs) {
        return old_exMem->aluResult;

    }  else if (old_memWb->regWrite && old_memWb->writeReg == in->rs) {
        return old_memWb->aluResult;

    }
    return in->rsVal; //return the current rsval.
}

/*
 * input: ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb
 * This function is to fill the ALU input2, which is rt or imm field. This function also need to take data forwarding into consideration.
 */
WORD EX_getALUinput2(ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb){
  //this condiction is to check is input2 read from registers or not. 
  if(in->ALUsrc==0){

  //data harvard will happen on rt register.
    if(old_exMem->regWrite && old_exMem->writeReg==in->rt){
      return old_exMem->aluResult;
    }    
    if(old_memWb->regWrite && old_memWb->writeReg==in->rt){
      //from ALU
      if(old_memWb->memToReg==0)
        return old_memWb->aluResult;
      else
        return old_memWb->memResult;
    }

    return in->rtVal;
  } else if(in->ALUsrc==1){
    return in->imm32;
  }else if(in->ALUsrc==2){ 
   
    if(in->extra3 == 1) //lui
        return in->imm16 << 16;
    
    return in->imm16;
  }
  
}

/*
 * input: ID_EX *in, WORD input1, WORD input2, EX_MEM *new_exMem
 * Implement the function of EX phase, take two pipeline register as input and choose the right pipeline register to write.
 */
void execute_EX(ID_EX *in, WORD input1, WORD input2, EX_MEM *new_exMem){
            
 // to do the same thing with ID_EX pipeline register,
    new_exMem->rt = in->rt;
    new_exMem->rtVal=in->rtVal;
    new_exMem->memRead=in->memRead;
    new_exMem->memWrite=in->memWrite;
    new_exMem->memToReg=in->memToReg;
    new_exMem->regWrite=in->regWrite;
    new_exMem->extra1=in->extra1;
    new_exMem->extra2=in->extra2;
    new_exMem->extra2=in->extra2;

  // R format instruction.
    if(in->regDst)
        new_exMem->writeReg=in->rd;

    else //otherwise I format instruction.
        new_exMem->writeReg=in->rt;
  
  //and (alu.op is 0)
    if(in->ALU.op==0)
        new_exMem->aluResult=input1&input2;
   
  //or (alu.op is 1)
    if(in->ALU.op==1){
        if(in->extra1==1)
            new_exMem->aluResult=~(input1|input2);
        else
            new_exMem->aluResult=input1|input2;
    }

    // and (alu.op is 2)
    if(in->ALU.op==2){ 
        if(in->ALU.bNegate==0)
            new_exMem->aluResult=input1+input2;

        // sub (alu.bNegate is 0)
        else
            new_exMem->aluResult=input1-input2;
    }

    //less, (alu.op is 3)
    if(in->ALU.op==3){
        if(input1-input2<0)
            new_exMem->aluResult=1;
        else
            new_exMem->aluResult=0;
    }

    // 0 if the alu.op is 4.
   if(in->ALU.op==4) 
        new_exMem->aluResult=0;
     
}

/*
 * input: EX_MEM *in, MEM_WB *old_memWb, WORD *mem, MEM_WB *new_memwb
 * This function executes MEM phase, take two pipeline register as input and write the correct register in the end
 */
void execute_MEM(EX_MEM *in, MEM_WB *old_memWb, WORD *mem, MEM_WB *new_memwb)
{
  //copy pipeline register from EX_MEM to MEM_WB 
    new_memwb->memToReg=in->memToReg;
    new_memwb->aluResult=in->aluResult;
    new_memwb->writeReg=in->writeReg;
    new_memwb->regWrite=in->regWrite;
    new_memwb->extra1=in->extra1;
    new_memwb->extra2=in->extra2;
    new_memwb->extra3=in->extra3;

    int mem_index = (in->aluResult)>>2;
    if(in->memRead){  //to read the memory.
        new_memwb->memResult = mem[mem_index];

    }else if(in->memWrite){ //to write the memory.
        mem[mem_index] = in->rtVal;
        new_memwb->memResult=0;

    }else
        new_memwb->memResult=0;
  
}

/*
 * input: MEM_WB *in, WORD *regs
 * Execute the WB phase,take two pipeline register as input and write the correct register in the end.
 */
void execute_WB (MEM_WB *in, WORD *regs){
    //check whether write to the register.
    if(in->regWrite){
        //write the result to the register from mem.
        if(in->memToReg == 1){
            regs[in->writeReg] = in->memResult;
            return;
        } else {    //otherwise write from the alu.
            regs[in->writeReg] = in->aluResult;
        }
    }
 }
