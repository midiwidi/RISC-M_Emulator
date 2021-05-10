#ifndef RME_CONFIG_H_
#define RME_CONFIG_H_

// CPU Settings
#define STACK_DEPTH			32

#define OPCODE_MASK			0b11111
#define OPCODE_LSB			11
#define OPCODE_WIDTH		5
#define SRC_MASK			0b11111
#define SRC_LSB				6
#define SRC_WIDTH			5
#define DST_MASK			0b111111
#define DST_LSB				0
#define DST_WIDTH			6
#define SRCDST_MASK			0b11111111111
#define SRCDST_LSB			0
#define SRCDST_WIDTH		11
#define INST_MASK			0xFFFF
#define INST_WIDTH			16

#define MAX_MEMS			16		// max. number of memories for code space and for data space 0...255
#define MAX_MEM_SLOTS		4		// max. number of memories with overlapping addresses (muxed memories)

#define MAIN_SW_START_ADDR	0x200	// program address at which the main software starts
#define MAIN_SW_MEM_SIZE	8192	// depth of memory holding the main software
#define DATA_MEM_SIZE		256		// depth of data memory

#define INVALID_DATA		0xDEADDEAD

#define MAX_BREAKPOINTS		8

#endif /* RME_CONFIG_H_ */
