#include <stdio.h>
#include <string.h>
#include <RME_CPU.h>
#include <RME_helper.h>

EMU_ERR RISC_Reset(RISC_obj* RISC)
{
	bool debug;

	// Save members which should not be zeroed by a reset
	debug = RISC->core.debug;

	// Reset CPU state
	memset(&RISC->core, 0, sizeof(RISC->core));

	// Restore saved members
	RISC->core.debug = debug;

	return ERR_NONE;
}

EMU_ERR RISC_Execute(RISC_obj* RISC)
{
	uint32_t instr;		// 32bit instead of 16bit because of the common memory read function (code and data memory) upper 16 bits ignored
	opcodes opcode;
	uint32_t src, dst, srcdst;
	EMU_ERR err;

	uint32_t ui32_tmp;

	// If RISC is in HOLD then don't do anything
	if (RISC->core.hold)
		return ERR_NONE;

	if ((err = RISC->read_code_memory(RISC->code_memory_obj_container, RISC->core.PC, &instr)))
		goto Error;
	RISC->core.PC++;

	opcode = (instr >> OPCODE_LSB) & OPCODE_MASK;
	src = (instr >> SRC_LSB) & SRC_MASK;
	dst = (instr >> DST_LSB) & DST_MASK;
	srcdst = (instr >> SRCDST_LSB) & SRCDST_MASK;

	switch(opcode)
	{
		case JMP:
			RISC->core.PC = srcdst;
			break;
		case SUBT:
			RISC->core.R[0] = (int32_t)RISC->core.R[0] - (int32_t)RISC->core.R[1];
			break;
		case ADD:
			RISC->core.R[0] = (int32_t)RISC->core.R[0] + (int32_t)RISC->core.R[1];
			break;
		case MOV:
			write_reg(RISC, read_reg(RISC, src), dst);
			break;
		case MOVTM:
			if ((err = RISC->write_data_memory(RISC->data_memory_obj_container, RISC->core.R[0], read_reg(RISC, src))))
				goto Error;
			break;
		case MOVFM:
			if ((err = RISC->read_data_memory(RISC->data_memory_obj_container, RISC->core.R[0], &ui32_tmp)))
					goto Error;
			write_reg(RISC, ui32_tmp, dst);
			break;
		case MOVTIO:
			break;
		case MOVFIO:
			break;
		case MOVC:
			if ((err = RISC->read_code_memory(RISC->code_memory_obj_container, RISC->core.PC, &ui32_tmp)))
				goto Error;
			RISC->core.PC++;
			write_reg(RISC, (src << INST_WIDTH) | (ui32_tmp & INST_MASK), dst);
			break;
		case JNEZ:
			if (RISC->core.R[0] != 0)
				RISC->core.PC = srcdst;
			break;
		case JGEZ:
			if ((int32_t)RISC->core.R[0] >= 0)
				RISC->core.PC = srcdst;
			break;
		case SHIFTR:
			// Handle R2 first
			RISC->core.R[2] >>= 1;
			// Shift LSB of R0 into the MSB of R2
			if (RISC->core.R[0] & 0x00000001)
				RISC->core.R[2] |= 0x80000000;

			// Then handle R0
			ui32_tmp = RISC->core.R[0] & 0x80000000;
			RISC->core.R[0] >>= 1;
			// Extend sign bit
			RISC->core.R[0] |= ui32_tmp;
			break;
		case SHIFTL:
			RISC->core.R[2] <<= 1;
			RISC->core.R[0] <<= 1;
			break;
		case MULT:
			break;
		case HOLD:
			RISC->core.hold = true;
			break;
		case CALL:
			if ((err = stack_push(RISC)))
				goto Error;
			RISC->core.PC = srcdst;
			break;
		case UND:
			RISC->core.R[0] = RISC->core.R[0] & RISC->core.R[1];
			break;
		case ODER:
			RISC->core.R[0] = RISC->core.R[0] | RISC->core.R[1];
			break;
		case RET:
			if ((err = stack_pop(RISC)))
				goto Error;
			break;
		case LJMP:
			if ((err = RISC->read_code_memory(RISC->code_memory_obj_container, RISC->core.PC, &ui32_tmp)))
				goto Error;
			RISC->core.PC = (srcdst << INST_WIDTH) | (ui32_tmp & INST_MASK);
			break;
		case LJNEZ:
			if (RISC->core.R[0] != 0)
			{
				if ((err = RISC->read_code_memory(RISC->code_memory_obj_container, RISC->core.PC, &ui32_tmp)))
					goto Error;
				RISC->core.PC = (srcdst << INST_WIDTH) | (ui32_tmp & INST_MASK);
			}
			else
				RISC->core.PC++;
			break;
		case LJGEZ:
			if ((int32_t)RISC->core.R[0] >= 0)
			{
				if ((err = RISC->read_code_memory(RISC->code_memory_obj_container, RISC->core.PC, &ui32_tmp)))
					goto Error;
				RISC->core.PC = (srcdst << INST_WIDTH) | (ui32_tmp & INST_MASK);
			}
			else
				RISC->core.PC++;
			break;
		case LCALL:
			if ((err = RISC->read_code_memory(RISC->code_memory_obj_container, RISC->core.PC, &ui32_tmp)))
				goto Error;
			RISC->core.PC++;
			if ((err = stack_push(RISC)))
				goto Error;
			RISC->core.PC = (srcdst << INST_WIDTH) | (ui32_tmp & INST_MASK);
			break;
		default:
			break;
	}

Error:
	return err;
}

void RISC_Start(RISC_obj* RISC)
{
	// Release RISC HOLD
	RISC->core.hold = false;
}

void write_reg(RISC_obj* RISC, uint32_t data, uint8_t dst)
{
	if (dst < 8) // General Purpose Register 0...7
		RISC->core.R[dst] = data;
	else
	{
		switch(dst)
		{
			case 8: // Interrupt Enable Register
				RISC->core.IE = data;
				break;
			case 9: // Interrupt Clear
				RISC->core.IRF &= ~data;
				break;
			default:
				RME_fprintf(stdout, "Warning: invalid register destination address (%d)\n", dst);
		}
	}
}

uint32_t read_reg(RISC_obj* RISC, uint8_t src)
{
	if (src < 8) // General Purpose Register 0...7
		return RISC->core.R[src];
	else
	{
		switch(src)
		{
			// 8 = Arithmetic and Logic Unit (handled directly in RISC_Execute)
			// 9 = Constant (handled directly in RISC_Execute)
			case 10: // Interrupt Request Flags
				return RISC->core.IRF;
			default:
				RME_fprintf(stdout, "Warning: invalid register source address (%d)\n", src);
				return INVALID_DATA;
		}
	}
}

EMU_ERR stack_push(RISC_obj* RISC)
{
	if (RISC->core.SP >= STACK_DEPTH)
	{
		RME_fprintf(stderr, "Error: stack overflow");
		return ERR_STACK_OVF;
	}
	RISC->core.stack[RISC->core.SP] = RISC->core.PC;
	RISC->core.SP++;
	return ERR_NONE;
}

EMU_ERR stack_pop(RISC_obj* RISC)
{
	if (RISC->core.SP == 0)
	{
		RME_fprintf(stderr, "Error: stack underflow");
		return ERR_STACK_UDF;
	}
	RISC->core.SP--;
	RISC->core.PC = RISC->core.stack[RISC->core.SP];
	return ERR_NONE;
}
