#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <process.h>
#include <windows.h>

#include <RME_types.h>
#include <RME_config.h>
#include <RME_memory.h>
#include <RME_CPU.h>
#include <RME_callbacks.h>
#include <RME_helper.h>
#include "RISC-M_Emu.h"

RISC_obj* RISC = NULL;
HANDLE h_mutex;

__declspec (dllexport) int __stdcall emu_create(void)
{
	EMU_ERR err;

	RISC = NULL;

	if ((err = emu_init(&RISC)))
		goto Error;

	// Create code memories for bootloader and main program (main consists of 2 memories PROM and PRAM)
	// BootROM (MemSlot 0)
	if ((err = create_memory(&RISC->code_memory, 0x0000000, MAIN_SW_START_ADDR-1, 2, 0, RO, NULL, 0, BIG_ENDIAN)))
		goto Error;
	// PROM (MemSlot 0)
	if ((err = create_memory(&RISC->code_memory, MAIN_SW_START_ADDR, MAIN_SW_START_ADDR+MAIN_SW_MEM_SIZE-1, 2, 0, RO, NULL, 0, BIG_ENDIAN)))
		goto Error;
	// PRAM (MemSlot 1)
	if ((err = create_memory(&RISC->code_memory, MAIN_SW_START_ADDR, MAIN_SW_START_ADDR+MAIN_SW_MEM_SIZE-1, 2, 1, RO, NULL, 0, BIG_ENDIAN)))
		goto Error;

	// Connect code memories to the CPU
	if ((err = SetupCallback_read_code_memory(RISC, &RISC->code_memory, &read_code_memory)))
		goto Error;


	// Create data memory
	if ((err = create_memory(&RISC->data_memory, 0x0000000, DATA_MEM_SIZE-1, 4, 0, RW, NULL, 0, BIG_ENDIAN)))
		goto Error;
	// Link PRAM as data memory for writing from bootloader
	//if ((err = link_memory(&RISC->data_memory, 0x0000000, DATA_MEM_SIZE-1, 2, 0, WO, "d:\\meine_Software\\RISC-M\\MagRISC_test\\MagRISC_test.bin", 0x0000000, BIG_ENDIAN)))
	//	goto Cleanup;
	// Connect data memories to the CPU
	if ((err = SetupCallback_read_data_memory(RISC, &RISC->data_memory, &read_data_memory)))
		goto Error;
	if ((err = SetupCallback_write_data_memory(RISC, &RISC->data_memory, &write_data_memory)))
			goto Error;


	if ((err = RISC_Reset(RISC)))
		goto Error;

	return ERR_NONE;

Error:
	if (RISC)
	{
		destroy_memory(&RISC->code_memory);
		destroy_memory(&RISC->data_memory);
		free(RISC);
	}
	return err;
}

__declspec (dllexport) void __stdcall emu_destroy(void)
{
	if (RISC)
	{
		destroy_memory(&RISC->code_memory);
		destroy_memory(&RISC->data_memory);
		free(RISC);
	}
}

__declspec (dllexport) int __stdcall emu_load_memory(uint8_t code_or_data, uint8_t slot, uint32_t startaddr, char* filename, int endianness)
{
	EMU_ERR err = ERR_NONE;
	RISC_memory *mem;
	uint8_t idx;
	bool found = false;

	if (code_or_data)
		mem = &RISC->data_memory;
	else
		mem = &RISC->code_memory;

	for (idx = 0; idx < MAX_MEMS; idx++)
	{
		if (addr_in_mem_range(mem, idx, startaddr))
		{
			// found memory where startaddr is in range
			// check if it is in the currently selected slot
			if (mem->slot[idx] == slot)
			{
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		RME_fprintf(stdout, "Warning: no memory connected which includes the address 0x%08x", startaddr);
		err = ERR_ADDR_NOMEM;
	}
	else
	{
		err = load_memory(mem, idx, filename, startaddr, endianness);
	}

	return err;
}

__declspec (dllexport) int __stdcall emu_step(void)
{
	EMU_ERR err = ERR_NONE;

	select_memory_slot(RISC, 0);
	err = RISC_Execute(RISC);
	return err;
}

__declspec (dllexport) int __stdcall emu_run(void)
{
	int thread_id;

	RISC->debug.mode = DBG_RUN;
	if ((thread_id = _beginthread(emu_run_continuous, 0, NULL))== (unsigned long)-1)
		return ERR_CREATE_THREAD;
	return ERR_NONE;
}

__declspec (dllexport) void __stdcall emu_stop(void)
{
	WaitForSingleObject(h_mutex, INFINITE);
	RISC->debug.mode = DBG_STEP;
	ReleaseMutex(h_mutex);
}

__declspec (dllexport) double __stdcall emu_get_cpu_register(uint8_t cpu_register)
{
	WaitForSingleObject(h_mutex, INFINITE);
	if (cpu_register <= R7)
		return (double)RISC->core.R[cpu_register];
	else
		switch(cpu_register)
		{
			case IRF:
				return (double)RISC->core.IRF;
			case IE:
				return (double)RISC->core.IE;
			case SP:
				return (double)RISC->core.SP;
			case PC:
				return (double)RISC->core.PC;
			default:
				return 0;

		}
	ReleaseMutex(h_mutex);
}

__declspec (dllexport) void __stdcall emu_set_breakpoint(uint32_t addr)
{
	// TODO: implement breakpoints
}

__declspec (dllexport) void __stdcall emu_reset(void)
{
	WaitForSingleObject(h_mutex, INFINITE);
	RISC_Reset(RISC);
	ReleaseMutex(h_mutex);
}

__declspec (dllexport) unsigned long __stdcall emu_get_clk_cycles(void)
{
	// TODO: implement DLL interface for clk cycles

	return 0;
}

void emu_run_continuous(void* arg)
{
	while(1)
	{
		WaitForSingleObject(h_mutex, INFINITE);
		if (RISC->debug.mode != DBG_RUN)
		{
			break;
		}
		select_memory_slot(RISC, 0);
		RISC_Execute(RISC);
		ReleaseMutex(h_mutex);
		usleep(20000);
	}
}

EMU_ERR emu_init(RISC_obj** pRISC)
{
	RISC_obj* RISC = NULL;

	if (!(RISC = calloc(1, sizeof(*RISC))))
	{
		RME_fprintf(stderr, "Error: memory allocation of RISC object failed\n");
		*pRISC = NULL;
		return ERR_ALLOC_RISC;
	}

	*pRISC = RISC;

	return ERR_NONE;
}
