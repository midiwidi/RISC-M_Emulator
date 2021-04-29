#ifndef RME_MEMORY_H_
#define RME_MEMORY_H_

#include <RME_types.h>

EMU_ERR create_memory(	RISC_memory* mem, uint32_t start_addr, uint32_t end_addr,
						uint8_t data_bytes_per_word, uint8_t slot, MEM_ACCESS access,
						const char* memfile, bool memfile_is_bigendian);
EMU_ERR destroy_memory(RISC_memory* mem);
EMU_ERR read_memory(RISC_memory* mem, uint32_t addr, uint32_t* data);
EMU_ERR write_memory(RISC_memory* mem, uint32_t addr, uint32_t data);
EMU_ERR load_memory(RISC_memory* mem, int idx, const char* memfile, bool memfile_is_bigendian);
void select_memory_slot(RISC_obj* RISC, uint8_t boot_operation);

bool addr_in_mem_range(RISC_memory* mem, uint8_t mem_idx, uint32_t addr);

#endif /* RME_MEMORY_H_ */
