#ifndef RISC_M_EMU_H_
#define RISC_M_EMU_H_


__declspec (dllexport) int __stdcall emu_create(void);
__declspec (dllexport) void __stdcall emu_destroy(void);

__declspec (dllexport) int __stdcall emu_load_memory(uint8_t code_or_data, uint8_t slot, uint32_t startaddr, char* filename, int endianness);

__declspec (dllexport) int __stdcall emu_step(void);
__declspec (dllexport) int __stdcall emu_run(void);
__declspec (dllexport) void __stdcall emu_stop(void);

__declspec (dllexport) double __stdcall emu_get_cpu_register(uint8_t cpu_register);
__declspec (dllexport) void __stdcall emu_set_breakpoint(uint32_t addr);
__declspec (dllexport) void __stdcall emu_reset(void);
__declspec (dllexport) unsigned long __stdcall emu_get_clk_cycles(void);

void emu_run_continuous(void* arg);
EMU_ERR emu_init(RISC_obj** pRISC);


#endif /* RISC_M_EMU_H_ */
