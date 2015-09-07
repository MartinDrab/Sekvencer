/*
 * program.h
 *
 * Created: 21. 6. 2015 17:11:55
 *  Author: Martin
 */ 


#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "error.h"
#include "server.h"


#define MAXIMUM_PROGRAM_COUNT          32
#define PROGRAM_SLOT_COUNT             32

typedef struct {
	EParserInstructionType Type;
	uint8_t Argument;	
	uint32_t Milisecond;
} PROGRAM_INSTRUCTION;

typedef struct {
	uint16_t FirstInstructionAddress;
} EEPROM_PROGRAM;

typedef struct {
	uint8_t Slots[PROGRAM_SLOT_COUNT];
	uint16_t NumberOfInstructions;
	uint8_t ID;
	uint8_t Active;
	uint16_t CurrentInstruction;
	PROGRAM_INSTRUCTION Instructions[1];
} LOADED_PROGRAM;




uint8_t program_select(uint8_t ID);
uint8_t program_unselect(void);
uint8_t program_load(void);
uint8_t program_activate(void);
uint8_t program_deactivate(void);
uint8_t program_set_abs_instruction(PROGRAM_INSTRUCTION *Instruction);
uint8_t program_set_rel_instruction(PROGRAM_INSTRUCTION *Instruction);
uint8_t program_delete_instruction(uint16_t InstructionIndex);
void program_execute_instruction(void);
uint8_t program_create(void);
uint8_t program_delete(void);
uint8_t program_list(void);

void programs_enumerate(void);
void programs_init_eeprom(void);

void programs_module_init(void);



#endif /* PROGRAM_H_ */
