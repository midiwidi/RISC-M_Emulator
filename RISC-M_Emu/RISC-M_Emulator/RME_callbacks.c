#include <RME_callbacks.h>

EMU_ERR SetupCallback_read_code_memory(RISC_obj* RISC, void* obj_container, EMU_ERR	(*callback)(void*, uint32_t, uint32_t*))
{
	RISC->read_code_memory = callback;
	RISC->code_memory_obj_container = obj_container;
	return ERR_NONE;
}

EMU_ERR SetupCallback_read_data_memory(RISC_obj* RISC, void* obj_container, EMU_ERR	(*callback)(void*, uint32_t, uint32_t*))
{
	RISC->read_data_memory = callback;
	RISC->data_memory_obj_container = obj_container;
	return ERR_NONE;
}

EMU_ERR SetupCallback_write_data_memory(RISC_obj* RISC, void* obj_container, EMU_ERR (*callback)(void*, uint32_t, uint32_t))
{
	RISC->write_data_memory = callback;
	RISC->data_memory_obj_container = obj_container;
	return ERR_NONE;
}

EMU_ERR read_code_memory(void* obj_container, uint32_t addr, uint32_t* data)
{
	return read_memory(obj_container, addr, data);
}

EMU_ERR read_data_memory(void* obj_container, uint32_t addr, uint32_t* data)
{
	return read_memory(obj_container, addr, data);
}

EMU_ERR write_data_memory(void* obj_container, uint32_t addr, uint32_t data)
{
	return write_memory(obj_container, addr, data);
}
