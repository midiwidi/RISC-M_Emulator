#ifndef RME_CPU_H_
#define RME_CPU_H_

#include <RME_types.h>

typedef enum
{
	JMP,	/* der Sprungbefehl                                              */
	SUBT,	/* der Subtraktionsbefehl                                        */
	ADD,	/* der Additionsbefehl                                           */
	MOV,	/* der Verschiebebefehlbefehl                                    */
	MOVTM,	/* Daten in den externen Speicher schreiben                      */
	MOVFM,	/* Daten vom externen Speicher lesen                             */
	MOVTIO,	/* Daten zu IOs schreiben                                        */
	MOVFIO,	/* Daten von IOs lesen                                           */
	MOVC,	/* Befehl zum Laden einer Konstanten                             */
	JNEZ,	/* Springe wenn nicht Null                                       */
	JGEZ,	/* Springe wenn größer oder gleich Null                          */
	SHIFTR,	/* um 1.Stelle nach rechts schieben                              */
	SHIFTL,	/* um 1.Stelle nach links schieben                               */
	MULT,	/* Multipliziere                                                 */
	HOLD,	/* Processor anhalten                                            */
	CALL,	/* Sprung zu einem Unterprogramm                                 */
	UND,	/* Bitweises UND                                                 */
	ODER,	/* Bitweises ODER                                                */
	RET,	/* Rücksprung aus Unterprogramm                                  */
	LJMP,	/* der Sprungbefehl zu Adressen größer 2047                      */
	LJNEZ,	/* Springe wenn nicht Null (zu Adressen größer 2047)             */
	LJGEZ,	/* Springe wenn größer oder gleich Null (zu Adressen größer 2047)*/
	LCALL	/* Sprung zu einem Unterprogramm (zu Adressen größer 2047)       */
} opcodes;

EMU_ERR RISC_Reset(RISC_obj* RISC);
EMU_ERR RISC_Execute(RISC_obj* RISC);
void RISC_Start(RISC_obj* RISC);

void write_reg(RISC_obj* RISC, uint32_t data, uint8_t dst);
uint32_t read_reg(RISC_obj* RISC, uint8_t src);
EMU_ERR stack_push(RISC_obj* RISC);
EMU_ERR stack_pop(RISC_obj* RISC);

#endif /* RME_CPU_H_ */
