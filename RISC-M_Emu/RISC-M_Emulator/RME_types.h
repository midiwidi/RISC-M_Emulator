#ifndef RME_TYPES_H_
#define RME_TYPES_H_

#include <RME_config.h>
#include <stdint.h>
#include <sys/time.h>

typedef enum { false, true } bool;

typedef enum
{
	ERR_NONE,				/* Success */
	ERR_ALLOC_RISC,			/* RISC object memory allocation failed */
	ERR_MEMSLOT_RANGE,		/* memory slot is out of range 0...MAX_MEM_SLOT-1 */
	ERR_ALLOC_MEM,			/* code or data memory allocation failed */
	ERR_MEMOBJ_FULL,		/* All available memories (MAX_CODE_MEMS / MAX_MEMS) used*/
	ERR_FILE_OPEN,			/* File open failed */
	ERR_FILE_SIZE,			/* Size of memfile is bigger than the memory it is loaded into */
	ERR_FILE_READ,			/* Reading file reported different numbers of bytes read than requested */
	ERR_CREATE_THREAD,		/* Thread creation failed */
	ERR_STACK_OVF,			/* Stack overflow */
	ERR_STACK_UDF			/* Stack underflow */
} EMU_ERR;

typedef enum
{
	CODE,
	DATA
} MEM_TYPE;

typedef enum
{
	LITTLE_ENDIAN,
	BIG_ENDIAN
} MEM_ENDIANNESS;

typedef enum
{
	RO,	// read only
	WO,	// write only
	RW	// read and write
} MEM_ACCESS;

typedef enum
{
	DBG_STEP,	// debug is in single step mode
	DBG_RUN		// debug is in continuous running mode
} DEBUG_MODE;

typedef enum
{
	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	IRF,
	IE,
	SP,
	PC
} CPU_REGISTER;

typedef struct
{
	uint32_t		R[8];
	uint32_t		IRF;
	uint32_t		IE;
	uint8_t			SP;
	uint32_t		PC;
	uint32_t		stack[STACK_DEPTH];
	bool			debug;
	bool			programming;
	bool			hold;
	struct timeval	exec_end_time;
} RISC_core;

typedef struct
{
	uint8_t current_slot;					// if multiple memories share common addresses, this specifies which memory (slot) is selected
	uint8_t slot[MAX_MEMS];					// memory slot of the memories
	uint8_t data_bytes_per_word[MAX_MEMS];	// data width in bytes (1...4); should be 2 for code memory and usually 4 for data memory
	uint32_t* data[MAX_MEMS];				// memory data (pointer to allocated buffer)
	uint32_t start_addr[MAX_MEMS];			// virtual start address of the different memories
	uint32_t end_addr[MAX_MEMS];			// size of the different memories
	MEM_ACCESS access[MAX_MEMS];			// access restriction (read, write, read/write)
} RISC_memory;

typedef struct
{
	uint32_t breakpoint[MAX_BREAKPOINTS];
	DEBUG_MODE mode;
} RISC_debug;

typedef struct
{
	RISC_core core;
	RISC_memory code_memory;
	RISC_memory data_memory;
	RISC_debug debug;
	EMU_ERR	(*read_code_memory)(void*, uint32_t, uint32_t*);	// callback function to read from code memory
																// EMU_ERR read_code_memory(void* obj_container, uint32_t addr, uint32_t* data)
	void* code_memory_obj_container;							// This will be passed to read_code_memory callback as the 1st parameter.
																// Could be as simple as a pointer to the allocated code memory or a pointer to a
																// structure of a more complex memory model with more features like multiple and/or
																// multiplexed memories (e.g. used for PROM/PRAM...) like the one specified in memory.c
	EMU_ERR	(*read_data_memory)(void*, uint32_t, uint32_t*);	// callback function to read from data memory
	EMU_ERR	(*write_data_memory)(void*, uint32_t, uint32_t);	// callback function to write to data memory
	void* data_memory_obj_container;							// see explanation above for read_code_memory
} RISC_obj;

#endif /* RME_TYPES_H_ */
