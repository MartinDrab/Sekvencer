/*
 * server.h
 *
 * Created: 21. 6. 2015 13:19:50
 *  Author: Martin
 */ 


#ifndef SERVER_H_
#define SERVER_H_

#include "error.h"

// E = Enumerate programs
// C = Create program (returns an ID of the newly created program)
// S-NN = Select program
// u - unselect program
// D = Delete selected program
// a = Activate selected program
// d - deactivate selected program
// L - List program instructions
// l - Load program to SRAM
// T-HH:MM:SS:mmm- ABSOLUTE instruction
// R-HH:MM:SS:mm-  RELATIVE instruction
//     wtp[A-G]-NN
//     spb[A-G-][0-7]
//     cpb[A-G][0-7]
//     dob[A-G][0-7]
//     dib[A-G][0-7]
//     ddr[A-G]-NN
//     rps[A-G]-NN
//     wps[A-G]-NN
//
// U - dump EEPROM
// I - init EEPROM
// r - Erase EEPROM
// h - help
// G - print global config
// g-N-XX - set global config N to value X

typedef enum {
	psStart,
	psEnumeratePrograms,
	psCreateProgram,
	psSelectProgram,
	psUnselectProgram,
	psDeleteProgram,
	psLoadProgram,
	psActivateProgram,
	psDeactivateProgram,
	psListProgramInstructions,
	psAbsoluteTimeInstructions,
	psRelativeTimeInstruction,
	psDeleteInstruction,
	psInitEEPROM,
	psDumpEEPROM,
	psEraseEEPROM,
	psHelp,
	psPrintConfig,
	psWriteConfig,
} EParserState;

typedef enum {
	pssUnknown,
	pssParseHexByte,
	pssParseBitNumber,
	pssParseGBitNumber,
	pssParseSlotNumber,
	pssParseProgramNumber,
	ppsParseInstructionNumber,
	pssParseHours,
	pssParseMinutes,
	pssParseSeconds,
	pssParseMiliSeconds,
	pssParseInstructionCommand,
	pssParsePortLetter,
	pssParseConfigField,
	pssParseConfigValue,
} EParserSubState;

typedef enum {
	pitNoInstruction,
	pitWriteByteToA,
	pitWriteByteToB,
	pitWriteByteToC,
	pitWriteByteToD,
	pitWriteByteToE,
	pitWriteByteToF,
	pitWriteByteToG,
	pitSetBitInA,
	pitSetBitInB,
	pitSetBitInC,
	pitSetBitInD,
	pitSetBitInE,
	pitSetBitInF,
	pitSetBitInG,
	pitClearBitInA,
	pitClearBitInB,
	pitClearBitInC,
	pitClearBitInD,
	pitClearBitInE,
	pitClearBitInF,
	pitClearBitInG,		
	pitSetInputA,
	pitSetInputB,
	pitSetInputC,
	pitSetInputD,
	pitSetInputE,
	pitSetInputF,
	pitSetInputG,	
	pitSetOutputA,
	pitSetOutputB,
	pitSetOutputC,
	pitSetOutputD,
	pitSetOutputE,
	pitSetOutputF,
	pitSetOutputG,
	pitSetDDRA,
	pitSetDDRB,
	pitSetDDRC,
	pitSetDDRD,
	pitSetDDRE,
	pitSetDDRF,
	pitSetDDRG,	
	pitPinAToSlot,
	pitPinBToSlot,
	pitPinCToSlot,
	pitPinDToSlot,
	pitPinEToSlot,
	pitPinFToSlot,
	pitPinGToSlot,
	pitSlotToPortA,
	pitSlotToPortB,
	pitSlotToPortC,
	pitSlotToPortD,
	pitSlotToPortE,
	pitSlotToPortF,
	pitSlotToPortG,		
} EParserInstructionType;

typedef struct {
	EParserState State;
	EParserSubState SubState;
	uint8_t Index;
	uint8_t done;
	union {
		struct {
			uint8_t Hour;
			uint8_t Minute;
			uint8_t Second;
			uint16_t Milisecond;
			EParserInstructionType Type;
			uint8_t Argument;
		} Instruction;
		struct {
			uint8_t Argument;
			uint16_t Argument16;
		} Command;	
	};
	char InputStorage[3];
} SERVER_STATE;





void server_reset(void);
uint8_t server_put_char(char Character);
void server_state(SERVER_STATE *State);
void server_help(void);


#endif /* SERVER_H_ */
