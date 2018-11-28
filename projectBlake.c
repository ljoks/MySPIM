#include "spimcore.h"
#define DEBUG_MODE 0


int isValidOP(unsigned num);

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
	if(DEBUG_MODE){printf("ENTERING ALU FUNCTION\n");
	printf("A: %d B: %d\n", A, B);}
	
	// The switch statement doesn't take a char, so I convert ALUControl to an int here for the switch statement.
	int controlInt = (int)ALUControl;
	switch(controlInt)
	{
		case 0:
			*ALUresult = A + B;
			break;
		case 1:
			*ALUresult = A - B;
			break;
		case 2:
			*ALUresult = ((signed)A < (signed)B);
			break;
		case 3:
			*ALUresult = ((unsigned)A < (unsigned)B);
			break;
		case 4:
			*ALUresult = A & B;
			break;
		case 5:
			*ALUresult = A | B;
			break;
		case 6:
			*ALUresult =  B << 16;
			break;
		case 7:
			*ALUresult = ~A; // Binary ones compliment operator. Flips all the bits.
			break;
	}
	
	if(*ALUresult == 0)
	{
		*Zero = 1;
	}
	else
	{
		*Zero = 0;
	}
	if(DEBUG_MODE){printf("EXITING ALU FUNCTION\n");}
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
	if(DEBUG_MODE){printf("\n\nENTERING INSTRUCTION_FETCH\n");}
	// First, check to see if any of the addressing halt conditions can be met.
	if(PC % 4 != 0)
	{
		return 1;
	}
	if(PC >> 2 > 65536 >> 2)
	{
		return 1;
	}
	
	// Grab the instruction from Mem at PC/4.
	*instruction = Mem[PC>>2];
	
	// Use function isValidOP to determine if the op code is valid for this instruction.
	if(!isValidOP(*instruction))
	{
		return 1;
	}
	
	if(DEBUG_MODE){printf("EXITING INSTRUCTION_FETCH\n");}
	return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	if(DEBUG_MODE){printf("ENTERING INSTRUCTION_PARTITION\n");
	printf("Instruction: %x\n", instruction);}
	
	// The formula for finding the subset of bits within the 32 bit number is:
	// (instruction >> [LeastSignificantBitPos]) % 2^[MostSignificantBitPos - LeastSignificantBitPos + 1]
	// Note that for R-Type functions the OP code is 0 and is truly held in the funct variable.
	*op = instruction >> 26;
	if(DEBUG_MODE){printf("op: %d\n", *op);}
	
	// r1 is the first register used in immediate functions where the data is stored.
	*r1 = (instruction >> 21) % 32;
	if(DEBUG_MODE){printf("r1: %d\n", *r1);}
	
	// r2 is the second register used in immediate functions.
	*r2 = (instruction >> 16) % 32;
	if(DEBUG_MODE){printf("r2: %d\n", *r2);}
	
	// r3 is the third register in R-Type functions where the data is stored. r3 = r1+r2.
	*r3 = (instruction >> 11) % 32;
	if(DEBUG_MODE){printf("r3: %d\n", *r3);}
	
	// funct holds the true OP code for R-Type functions.
	*funct = instruction % 64;
	if(DEBUG_MODE){printf("funct: %d\n", *funct);}
	
	// Use offset for constants in immediate functions.
	*offset = instruction % (256*256);
	if(DEBUG_MODE){printf("offset: %d\n", *offset);}
	*jsec = instruction % (256*256*256*4);
	if(DEBUG_MODE){printf("jsec: %d\n", *jsec);}
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
	if(DEBUG_MODE){printf("ENTERING INSTRUCTION_DECODE\n");}
	// This switch statement evaluates the OP code and executes its corresponding block of code.
	// Information on control signals can be found in Study Set 6 on slide 12 and on.
	switch(op)
	{
		case 0:
			if(DEBUG_MODE){printf("R-Type\n");}
			controls->RegDst = 1;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 7;
			controls->MemWrite = 0;
			controls->ALUSrc = 0;
			controls->RegWrite = 1;
			break;
		case 8:
			if(DEBUG_MODE){printf("addi function. I-Type\n");}
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 0;
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			break;
		case 35:
			if(DEBUG_MODE){printf("load word function. I-Type\n");}
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 1;
			controls->MemtoReg = 1;
			controls->ALUOp = 0; // "Don't care" value, not add.
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			break;
		case 43:
			if(DEBUG_MODE){printf("store word function. I-Type\n");}
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 2;
			controls->ALUOp = 0; // "Don't care" value, not add.
			controls->MemWrite = 1;
			controls->ALUSrc = 1;
			controls->RegWrite = 0;
			break;
		case 15:
			if(DEBUG_MODE){printf("load upper immediate function. I-Type\n");}
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 6;
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			break;
		case 4:
			if(DEBUG_MODE){printf("branch on equal function. I-Type\n");}
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 1;
			controls->MemRead = 0;
			controls->MemtoReg = 2;
			controls->ALUOp = 1;
			controls->MemWrite = 0;
			controls->ALUSrc = 0;
			controls->RegWrite = 0;
			break;
		case 10:
			if(DEBUG_MODE){printf("set less than immediate function. I-Type\n");}
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 2;
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			break;
		case 11:
			if(DEBUG_MODE){printf("set less than immediate unsigned function. I-Type\n");}
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 3;
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			break;
		case 2:
			if(DEBUG_MODE){printf("jump function. J-Type\n");}
			controls->RegDst = 2;
			controls->Jump = 1;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 2;
			controls->ALUOp = 0; // "Don't care" value, not add.
			controls->MemWrite = 0;
			controls->ALUSrc = 2;
			controls->RegWrite = 2;
			break;
		default:
			if(DEBUG_MODE){printf("Instruction decode defaulted (Invalid OP code).");}
			return 1;
	}
	//if(DEBUG_MODE){printf("RegDst: %x\nJump: %x\nBranch: %x\nMemRead: %x\nMemtoReg: %x\nALUOp: %x\nMemWrite: %x\nALUSrc: %x\nRegWrite: %x\n", controls->RegDst, controls->Jump, controls->Branch, controls->MemRead,controls->MemtoReg, controls->ALUOp, controls->MemWrite, controls->ALUSrc, controls->RegWrite);}
	if(DEBUG_MODE){printf("EXITING INSTRUCTION_DECODE\n");}
	return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
	if(DEBUG_MODE){printf("ENTERING READ_REGISTER\nr1: %d r2: %d\n", r1, r2);}
	*data1 = Reg[r1];
	*data2 = Reg[r2];
	if(DEBUG_MODE){printf("data1: %d\ndata2: %d\nEXITING READ_REGISTER\n", *data1, *data2);}
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
	// Information from slide 27 of Processor powerpoint.
	// To extend the sign extend the 16 bit immediate, fill the left with 0's if it's a positive number and 1's if it's negative.
	if(DEBUG_MODE){printf("ENTERING SIGN_EXTEND\nOffset: %x\n", offset);}
	
	// If the leftmost bit of the 16 bit immediate is 0, the number must be padded with 0's.
	if(offset >> 15 == 0)
	{
		*extended_value = offset & 0x0000FFFF;
	}
	else // If the leftmost bit isn't 0, the left is padded with 1's.
	{
		*extended_value = offset | -65536;
	}
	if(DEBUG_MODE){printf("extended_value: %x\nEXITING SIGN_EXTEND\n", *extended_value);}
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	if(DEBUG_MODE){printf("ENTERING ALU_operations FUNCTION\n");}
	// The operation performed is determined by the instruction's "funct" field for R-Types and "ALUOp" for others.
	
	if(ALUSrc == 1)
	{
		data2 = extended_value;
	}
	
	if(ALUOp == 7) // Our 6 R-Type functions.
	{
		// Add
		if(funct == 32)
		{
			ALUOp = 0;
		}
		// Sub
		else if(funct == 34)
		{
			ALUOp = 1;
		}
		// SLT Signed
		else if(funct == 42)
		{
			ALUOp = 2;
		}
		// SLT Unsigned
		else if(funct == 43)
		{
			ALUOp = 3;
		}
		// And
		else if(funct == 36)
		{
			ALUOp = 4;
		}
		// Or
		else if(funct == 37)
		{
			ALUOp = 5;
		}
		else
		{
			return 1;
		}
	}
	
	ALU(data1, data2, ALUOp, ALUresult, Zero);
	
	if(DEBUG_MODE){printf("data1: %d, data2: %d, ALUresult: %d\n", data1, data2, *ALUresult);
	printf("EXITING ALU_operations FUNCTION\n");}
	
	return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
	if(DEBUG_MODE){printf("ENTERING rw_memory FUNCTION\n");}
	// If the load or store address is not divisible by 4, a halt condition has been reached.
	if((MemRead || MemWrite) && ALUresult % 4 != 0)
	{
		if(DEBUG_MODE){printf("ALUresult was %d\n", ALUresult);}
		return 1;
	}
	
	// If MemRead is asserted, read from memory to memdata.
	if(MemRead)
	{
		*memdata = Mem[ALUresult>>2];
	}
	
	// If MemWrite is asserted, write data2 to memory.
	if(MemWrite)
	{
		Mem[ALUresult>>2] = data2;
	}
	
	if(DEBUG_MODE){printf("EXITING rw_memory FUNCTION\n");}
	return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
	if(DEBUG_MODE){printf("ENTERING write_register FUNCTION\n");}
	
	if(!RegWrite)
	{
		return;
	}
	// If RegDst is asserted, r3 is the register. Otherwise, r2 is. Copying the address of the register handles this.
	if(RegDst)
	{
		r2 = r3;
	}
	
	if(MemtoReg)
	{
		Reg[r2] = memdata;
	}
	else
	{
		Reg[r2] = ALUresult;
	}
	
	
	if(DEBUG_MODE){printf("EXITING write_register FUNCTION\n");}
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
	if(DEBUG_MODE){printf("ENTERING PC_update FUNCTION\n");}
	// Always incrememnt PC to PC + 4.
	*PC = *PC + 4;
	if(DEBUG_MODE){printf("PC: %x\n", *PC);}
	
	if(Branch == 1 && Zero == 1)
	{
		*PC = *PC + (extended_value*4);
	}
	if(Jump)
	{
		*PC = (jsec * 4) & 0xFFFFFFFF;
		if(DEBUG_MODE){printf("Jump PC: %x\n", *PC);}
	}
	if(DEBUG_MODE){printf("EXITING PC_update FUNCTION\n");}
}

// This function takes a 32 bit number and checks to ensure it has a valid op code. Also works with 6 bit OP codes.
int isValidOP(unsigned num)
{
	if(DEBUG_MODE){printf("ENTERING ISVALID FUNCTION\n");}
	
	// This if statement handles the case of R-Type functions having OP codes in the back.
	// Also, this if statement allows OP codes that have already been reduced to 6 bits to be read.
	if((num >> 26) == 0)
	{
		num = (num % 64);
	}
	else
	{
		num = num >> 26;
	}
	if(num == 32 || num == 34 ||num == 8 || num == 36 || num == 37 || num == 35 || num == 43 || num == 15 || num == 4 || num == 42 || num == 10 || num == 43 || num == 11 || num == 2)
	{
		return 1;
	}
	
	if(DEBUG_MODE){printf("EXITING ISVALID FUNCTION. DECIMAL VALUE WAS: %d\n", num);}
	return 0;
}

