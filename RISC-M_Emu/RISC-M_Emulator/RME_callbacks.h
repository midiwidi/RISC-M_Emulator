#ifndef RME_CALLBACKS_H_
#define RME_CALLBACKS_H_

#include <RME_memory.h>

EMU_ERR SetupCallback_read_code_memory(RISC_obj* RISC, void* obj_container, EMU_ERR	(*callback)(void*, uint32_t, uint32_t*));
EMU_ERR SetupCallback_read_data_memory(RISC_obj* RISC, void* obj_container, EMU_ERR	(*callback)(void*, uint32_t, uint32_t*));
EMU_ERR SetupCallback_write_data_memory(RISC_obj* RISC, void* obj_container, EMU_ERR (*callback)(void*, uint32_t, uint32_t));

EMU_ERR read_code_memory(void* obj_container, uint32_t addr, uint32_t* data);
EMU_ERR read_data_memory(void* obj_container, uint32_t addr, uint32_t* data);
EMU_ERR write_data_memory(void* obj_container, uint32_t addr, uint32_t data);

#endif /* RME_CALLBACKS_H_ */
