#include <stdio.h>
#include <stdlib.h>
#include <RME_memory.h>
#include <RME_helper.h>

EMU_ERR create_memory(	RISC_memory* mem, uint32_t start_addr, uint32_t end_addr,
							uint8_t data_bytes_per_word, uint8_t slot, MEM_ACCESS access,
							const char* memfile, bool memfile_is_bigendian)
{
	EMU_ERR err;
	uint32_t idx = 0;
	bool found = false;

	if (slot >= MAX_MEM_SLOTS)
	{
		RME_fprintf(stderr, "Error: memory slot %d is out of range 0...%d\n", slot, MAX_MEM_SLOTS);
		return ERR_MEMSLOT_RANGE;
	}
	for (idx = 0; idx < MAX_MEMS; idx++)
	{
		if (mem->data[idx] == NULL) // empty spot
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		RME_fprintf(stderr, "Error: can't create memory, memory object contained full\n");
		return ERR_MEMOBJ_FULL;
	}
	if (!(mem->data[idx] = malloc((end_addr - start_addr + 1) * sizeof(*mem->data[idx]))))
	{
		RME_fprintf(stderr, "Error: code/data memory allocation failed\n");
		return ERR_ALLOC_MEM;
	}
	mem->start_addr[idx] = start_addr;
	mem->end_addr[idx] = end_addr;
	mem->data_bytes_per_word[idx] = data_bytes_per_word;
	mem->access[idx] = access;

	// If memfile was specified, load it
	if (memfile != NULL)
	{
		if((err = load_memory(mem, idx, memfile, memfile_is_bigendian)))
			return err;
	}

	return ERR_NONE;
}


EMU_ERR destroy_memory(RISC_memory* mem)
{
	int idx;

	for (idx = 0; idx < MAX_MEMS; idx++)
	{
		if (mem->data[idx]) // not empty spot
		{
			free(mem->data[idx]);
			mem->data[idx] = NULL;
		}
	}
	return ERR_NONE;
}


EMU_ERR read_memory(RISC_memory* mem, uint32_t addr, uint32_t* data)
{
	// TODO: memory-mapped I/O

	uint8_t idx;
	bool found = false;
	const uint32_t mask[4] = {0x000000FF, 0x0000FFFF, 0x00FFFFFF, 0xFFFFFFFF};
	uint8_t* buffer;
	uint32_t buffer_byte_offset;

	for (idx = 0; idx < MAX_MEMS; idx++)
	{
		if (addr_in_mem_range(mem, idx, addr))
		{
			// found memory where addr is valid in range
			// check if it is in the currently selected slot
			if (mem->slot[idx] == mem->current_slot)
			{
				// check if it is readable
				if (mem->access[idx] != WO)
				{
					found = true;
					break;
				}
			}
		}
	}

	if (!found)
	{
		RME_fprintf(stdout, "Warning: no memory connected to read address 0x%08x", addr);
		*data = INVALID_DATA & mask[mem->data_bytes_per_word[idx]-1];
	}
	else
	{
		buffer_byte_offset = (addr - mem->start_addr[idx]) * mem->data_bytes_per_word[idx];
		buffer = (uint8_t*)mem->data[idx];
		*data = (*(uint32_t*)&buffer[buffer_byte_offset]);
		*data &=  mask[mem->data_bytes_per_word[idx]-1];
	}

	return ERR_NONE;
}

EMU_ERR write_memory(RISC_memory* mem, uint32_t addr, uint32_t data)
{
	// TODO: memory-mapped I/O

	uint8_t idx;
	bool found = false;
	uint8_t* buffer;
	uint32_t buffer_byte_offset;
	uint8_t byte_idx;
	uint8_t* ptr;

	for (idx = 0; idx < MAX_MEMS; idx++)
	{
		if (addr_in_mem_range(mem, idx, addr))
		{
			// found memory where addr is valid in range
			// check if it is in the currently selected slot
			if (mem->slot[idx] == mem->current_slot)
			{
				// check if it is writable
				if (mem->access[idx] != RO)
				{
					found = true;
					break;
				}
			}
		}
	}

	if (!found)
	{
		RME_fprintf(stdout, "Warning: no memory connected to write address 0x%08x", addr);
	}
	else
	{
		buffer_byte_offset = (addr - mem->start_addr[idx]) * mem->data_bytes_per_word[idx];
		buffer = (uint8_t*)mem->data[idx];
		ptr = &buffer[buffer_byte_offset];
		for (byte_idx = 0; byte_idx < mem->data_bytes_per_word[idx]; byte_idx++)
		{
			*ptr = data & 0xFF;
			data >>= 8;
			ptr++;
		}
	}
	return ERR_NONE;
}

EMU_ERR load_memory(RISC_memory* mem, int idx, const char* memfile, bool memfile_is_bigendian)
{
	FILE* fid;
	long int filesize;
	long int size_in_mem;
	uint32_t addr;
	uint32_t ui32_tmp;
	uint8_t* p_ui8_tmp;

	// Open the file
	if (!(fid = fopen(memfile, "r")))
	{
		RME_fprintf(stderr, "Error: file %s open failed\n", memfile);
		return ERR_FILE_OPEN;
	}

	// Determine the file size
	fseek(fid, 0, SEEK_END);
	filesize = ftell(fid);
	fseek(fid, 0, SEEK_SET);

	// Error if file is bigger than selected memory
	if (filesize > (mem->end_addr[idx] - mem->start_addr[idx] + 1) * mem->data_bytes_per_word[idx])
	{
		RME_fprintf(stderr, "Error: size (%ld) of the memory file %s is bigger than the memory it is loaded into (%ld)\n", filesize, memfile, (long)(mem->end_addr - mem->start_addr + 1) * mem->data_bytes_per_word[idx]);
		return ERR_FILE_SIZE;
	}

	// Warn if file size is not a multiple of the memory word size
	size_in_mem = filesize / mem->data_bytes_per_word[idx];
	if (size_in_mem * mem->data_bytes_per_word[idx] != filesize)
	{
		RME_fprintf(stdout, "Warning: size of memory file %s is not a multiple of the memory width (%d)\n", memfile, mem->data_bytes_per_word[idx]);
	}

	// Read file into selected memory
	if (fread(mem->data[idx], 1, filesize, fid) != filesize)
	{
		RME_fprintf(stderr, "Error: file %s read failed\n", memfile);
		return ERR_FILE_READ;
	}

	// Swap bytes if endianness of file doesn't match machine endianness
	if (mem->data_bytes_per_word[idx] > 1) //endianness doesn't matter for single bytes
	{
		ui32_tmp = 1;
		if (memfile_is_bigendian == *(uint8_t*)&ui32_tmp) // *(uint8_t*)&ui32_tmp is equal to 1 if we are on a little endian machine
		{
			p_ui8_tmp = (uint8_t*)mem->data[idx];
			for (addr = 0; addr < size_in_mem; addr++)
			{
				switch(mem->data_bytes_per_word[idx])
				{
					case 2:
						ui32_tmp = *((uint16_t*)p_ui8_tmp);
						*((uint16_t*)p_ui8_tmp) = 	((ui32_tmp>>8)&0x000000ff) |	// move byte 1 to byte 0
													((ui32_tmp<<8)&0x0000ff00);		// move byte 0 to byte 1
						p_ui8_tmp += 2;
						break;
					case 3: // not tested
						ui32_tmp = *((uint8_t*)mem->data[idx] + addr);
						*((uint32_t*)mem->data[idx] + addr) = 	((ui32_tmp >> 16) & 0x000000ff) | 	// move byte 2 to byte 0
																((ui32_tmp      ) & 0x0000ff00) | 	// move byte 1 to byte 1
																((ui32_tmp << 16) & 0x00ff0000);	// move byte 0 to byte 2
						p_ui8_tmp += 3;
						break;
					case 4:
						ui32_tmp = *((uint32_t*)p_ui8_tmp);
						*((uint32_t*)p_ui8_tmp) = 	((ui32_tmp >> 24) & 0x000000ff) | 	// move byte 3 to byte 0
													((ui32_tmp >>  8) & 0x0000ff00) | 	// move byte 2 to byte 1
													((ui32_tmp <<  8) & 0x00ff0000) | 	// move byte 1 to byte 2
													((ui32_tmp << 24) & 0xff000000);	// move byte 0 to byte 3
						p_ui8_tmp += 4;
						break;
				}
			}
		}
	}
	return ERR_NONE;
}

void select_memory_slot(RISC_obj* RISC, uint8_t boot_operation)
{

}

bool addr_in_mem_range(RISC_memory* mem, uint8_t mem_idx, uint32_t addr)
{
	return (addr >= mem->start_addr[mem_idx] && addr <= mem->end_addr[mem_idx]);
}
