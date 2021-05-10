#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <RME_types.h>
#include <RME_config.h>
#include <RME_memory.h>
#include <RME_CPU.h>
#include <RME_callbacks.h>
#include "RISC-M_Emulator.h"

int main(int argc, char *argv[])
{
	EMU_ERR err = ERR_NONE;
	RISC_obj* RISC;
	int i, R;

	if ((err = Emu_Init(&RISC)))
		goto Cleanup;

	// Create code memories for bootloader and main program (main consists of 2 memories PROM and PRAM)
	// BootROM (MemSlot 0)
	if ((err = create_memory(&RISC->code_memory, 0x0000000, MAIN_SW_START_ADDR-1, 2, 0, RO, "d:\\meine_Software\\RISC-M\\MagRISC_test\\RISC-M_EMU_Test.bin", BIG_ENDIAN)))
		goto Cleanup;
	// PROM (MemSlot 0)
	if ((err = create_memory(&RISC->code_memory, MAIN_SW_START_ADDR, MAIN_SW_START_ADDR+MAIN_SW_MEM_SIZE-1, 2, 0, RO, "d:\\meine_Software\\RISC-M\\MagRISC_test\\MagRISC_test_PROM.bin", BIG_ENDIAN)))
		goto Cleanup;
	// PRAM (MemSlot 1)
	if ((err = create_memory(&RISC->code_memory, MAIN_SW_START_ADDR, MAIN_SW_START_ADDR+MAIN_SW_MEM_SIZE-1, 2, 1, RO, "d:\\meine_Software\\RISC-M\\MagRISC_test\\MagRISC_test_PRAM.bin", BIG_ENDIAN)))
		goto Cleanup;

	// Connect code memories to the CPU
	if ((err = SetupCallback_read_code_memory(RISC, &RISC->code_memory, &read_code_memory)))
		goto Cleanup;


	// Create data memory
	if ((err = create_memory(&RISC->data_memory, 0x0000000, DATA_MEM_SIZE-1, 4, 0, RW, NULL, 0)))
		goto Cleanup;
	// Link PRAM as data memory for writing from bootloader
	//if ((err = link_memory(&RISC->data_memory, 0x0000000, DATA_MEM_SIZE-1, 2, 0, WO, "d:\\meine_Software\\RISC-M\\MagRISC_test\\MagRISC_test.bin", BIG_ENDIAN)))
	//	goto Cleanup;
	// Connect data memories to the CPU
	if ((err = SetupCallback_read_data_memory(RISC, &RISC->data_memory, &read_data_memory)))
		goto Cleanup;
	if ((err = SetupCallback_write_data_memory(RISC, &RISC->data_memory, &write_data_memory)))
			goto Cleanup;


	if ((err = RISC_Reset(RISC)))
		goto Cleanup;

	do
	{
		select_memory_slot(RISC, 0);
		err = RISC_Execute(RISC);
		for (i=0; i<8; i++)
			R = read_reg(RISC, i);
	}
	while(!err);

Cleanup:
	if (RISC)
	{
		destroy_memory(&RISC->code_memory);
		destroy_memory(&RISC->data_memory);
		free(RISC);
	}

	exit(err);
}


EMU_ERR Emu_Init(RISC_obj** pRISC)
{
	RISC_obj* RISC = NULL;

	if (!(RISC = calloc(1, sizeof(*RISC))))
	{
		fprintf(stderr, "Error: memory allocation of RISC object failed\n");
		*pRISC = NULL;
		return ERR_ALLOC_RISC;
	}

	*pRISC = RISC;

	return ERR_NONE;
}





