#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch((int) ALUControl){
        case 0: // add
            *ALUresult = A + B;
            break;

        case 1: //subtract
            *ALUresult = A - B;
            break;

        case 2: // A < B (signed)
            if((signed)A < (signed)B){
                *ALUresult = 1;
            }
            else{
                *ALUresult = 0;
            }
            break;

        case 3: //A < B unsigned
            if(A < B){
                *ALUresult = 1;
            }else{
                *ALUresult = 0;
            }
            break;
        case 4: //A AND B
            *ALUresult = A & B;
            break;

        case 5: //A OR B
            *ALUresult = A | B;
            break;

        case 6: // left shift
            B << 16;
            break;

        case 7: //Not A
            *ALUresult = ~A;
            break;

    }

    if(*ALUresult == 0){ //check for zero
        *Zero = 1;
    }
    else{
        *Zero = 0;
    }
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    unsigned MemIndex = PC >> 2;

    if(PC % 4 == 0) { //check for proper format
        *instruction = Mem[MemIndex];

        return 0;
    }else{
        return 1;
    }
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    unsigned rPartition = 0x1f;
    unsigned functopPartition = 0x0000003f;
    unsigned offsetPartition = 0x0000ffff;
    unsigned jsecPartition = 0x03ffffff;

    *op = (instruction >> 26) & functopPartition; //31-26
    *r1 = (instruction >> 21) & rPartition; //25-21
    *r2 = (instruction >> 16) & rPartition; //20-16
    *r3 = (instruction >> 11) & rPartition; //15-11
    *funct = instruction & functopPartition; //5-0
    *offset = instruction & offsetPartition; //15-0
    *jsec = instruction & jsecPartition; //25-0
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    switch(op) {
       	case 0:
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

        default: //return 1 for halt
            return 1;

    }
    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
	*data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
	if (offset >> 15 == 1)
	{
		*extended_value = offset | 0xffff0000;
	}

	else if (offset >> 15 == 0)
	{
		*extended_value = offset & 0x0000ffff;
	}
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    if(ALUSrc == 1){
        data2 = extended_value;
    }

    if(ALUOp == 7){
        switch(funct)
        {
            case 32: //add
                ALUOp = 0;
                break;

            case 34: //Subtract
                ALUOp = 1;
                break;

            case 42: //set less than (signed)
                ALUOp = 2;
                break;

            case 43: //set less than (unsigned)
                ALUOp = 3;
                break;

            case 36: //and
                ALUOp = 4;
                break;

            case 37: //or
                ALUOp = 5;
                break;

            case 6: //shift left extended value 16
                ALUOp = 6;
                break;

            case 39: //NOR
                ALUOp = 7;
                break;

            default: //halt
                return 1;
        }




    }

    ALU(data1, data2, ALUOp, ALUresult, Zero);

    return 0;

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    if(MemRead == 1) //if reading
    {
        if((ALUresult % 4) == 0)
        {
            *memdata = Mem[ALUresult >> 2];
        }else //halt
        {
            return 1;
        }
    }

    if(MemWrite == 1) //if writing
    {
        if((ALUresult % 4) == 0)
        {
            Mem[ALUresult >> 2] = data2;
        }
        else //halt
        {
            return 1;
        }

    }

    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if(RegWrite == 1) //if writing
    {
        if(MemtoReg == 1 && RegDst == 0)
            Reg[r2] = memdata;

        else if(MemtoReg == 1 && RegDst == 1)
            Reg[r3] = memdata;

        else if(MemtoReg == 0 && RegDst == 0)
            Reg[r2] = ALUresult;

        else if(MemtoReg == 0 && RegDst == 1)
            Reg[r3] = ALUresult;
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC += 4; //always increment by 4

    if(Zero == 1 && Branch == 1) //if we got zero and branching then add extended value
        *PC += extended_value << 2;

    if(Jump == 1)
        *PC = (jsec << 2) | (*PC & 0xf0000000);
}
