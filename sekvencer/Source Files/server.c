/*
 * server.c
 *
 * Created: 21. 6. 2015 13:20:10
 *  Author: Martin
 */ 

#include <avr/io.h>
#include <string.h>
#include "flash-strings.h"
#include "program.h"
#include "server.h"



static SERVER_STATE _state;





uint8_t _to_digit(const char Character, const uint8_t Base, uint8_t *digit)
{
	uint8_t ret = SERVER_ERROR_SUCCESS;
	
	switch (Base) {
		case 16:			
		case 10:
			if (Character >= '0' && Character <= '9')
				*digit = (Character - '0');
			else if (Base == 16) {
				if (Character >= 'a' && Character <= 'f')
					*digit = (Character - 'a' + 10);
				else if (Character >= 'A' && Character <= 'F')
					*digit = (Character - 'A' + 10);
				else ret = SERVER_ERROR_NOT_A_HEX_DIGIT;
			} else ret = SERVER_ERROR_NOT_A_DEC_DIGIT;
			break;
		case 2:
			if (Character == '0' || Character == '1')
				*digit = (Character - '0');
			else ret = SERVER_ERROR_NOT_A_BINARY_DIGIT;
			break;
		default:
			ret = SERVER_ERROR_INVALID_NUMBER_BASE;
			break;
	}
	
	return ret;
}








void server_reset(void)
{
	for (uint8_t i = 0; i < sizeof(_state.InputStorage) / sizeof(char); ++i)
		_state.InputStorage[i] = '\0';
	
	_state.done = 0;
	_state.State = psStart;
	_state.SubState = pssUnknown;
	_state.Index = 0;
	_state.Instruction.Argument = 0;
	_state.Instruction.Hour = 0;
	_state.Instruction.Milisecond = 0;
	_state.Instruction.Minute = 0;
	_state.Instruction.Second = 0;
	_state.Instruction.Type = pitNoInstruction;
	
	return;
}

#define compare_instruction_prefix(aBuffer, aChar1, aChar2, aChar3) \
	(aBuffer[0] == aChar1 && aBuffer[1] == aChar2 && aBuffer[2] == aChar3)

#define init_instruction_info(aInstructionType, aNewSubState)	\
	{																\
		_state.Instruction.Type = aInstructionType;					\
		_state.Instruction.Argument = 0;							\
		_state.SubState = aNewSubState;								\
		_state.Index = 0;											\
	}																\

#define is_port_byte_instruction(aInstructionType)										\
	((aInstructionType >= pitWriteByteToA && aInstructionType <= pitWriteByteToG) ||	\
	 (aInstructionType >= pitSetDDRA && aInstructionType <= pitSetDDRG) ||				\
	 (aInstructionType >= pitSlotToPortA && aInstructionType <= pitSlotToPortG) ||		\
	 (aInstructionType >= pitPinAToSlot && aInstructionType <= pitPinGToSlot))			\
	

uint8_t server_put_char(char Character)
{
	uint8_t ret = SERVER_ERROR_SUCCESS;
	
		switch (_state.State) {
			case psStart: {
				switch (Character) {
					case 'E':
						_state.State = psEnumeratePrograms;
						_state.done = 1;
						break;
					case 'C':
						_state.State = psCreateProgram;
						_state.done = 1;
						break;
					case 'S':
						_state.State = psSelectProgram;
						break;
					case 'D':
						_state.State = psDeleteProgram;
						_state.done = 1;
						break;
					case 'a':
						_state.State = psActivateProgram;
						_state.done = 1;
						break;
					case 'd':
						_state.State = psDeactivateProgram;
						_state.done = 1;
						break;
					case 'L':
						_state.State = psListProgramInstructions;
						_state.done = 1;
						break;
					case 'l':
						_state.State = psLoadProgram;
						_state.done = 1;
						break;
					case 'R':
						_state.State = psRelativeTimeInstruction;
						break;
					case 'T':
						_state.State = psAbsoluteTimeInstructions;
						break;
					case 'e':
						_state.State = psDeleteInstruction;
						_state.Command.Argument16 = 0;
						break;
					case 'u':
						_state.State = psUnselectProgram;
						_state.done = 1;
						break;
						
					case 'I':
						_state.State = psInitEEPROM;
						_state.done = 1;
						break;
					case 'U':
						_state.State = psDumpEEPROM;
						_state.done = 1;
						break;
					case 'r':
						_state.State = psEraseEEPROM;
						_state.done = 1;
						break;
					case 'h':
						_state.State = psHelp;
						_state.done = 1;
						break;
					
					case 'G':
						_state.State = psPrintConfig;
						_state.done = 1;
						break;
					case 'g':
						_state.State = psWriteConfig;
						_state.SubState = pssParseConfigField;
						_state.Index = 0;
						_state.Command.Argument = 0;
						break;
					default:
						ret = SERVER_ERROR_UNKNOWN_COMMAND;
						server_reset();
						break;
				}
			} break;
			case psWriteConfig: {
				if (Character == '-') {
					if (_state.SubState == pssParseConfigField && _state.Index > 0) {
						_state.SubState = pssParseConfigValue;
						_state.Index = 0;
						_state.Command.Argument16 = 0;
					} else ret = SERVER_ERROR_INVALID_INPUT;
				} else {
					switch (_state.SubState) {
						case pssParseConfigField: {
							if (_state.Index < 2) {
								uint8_t digit;
							
								ret = _to_digit(Character, 16, &digit);
								if (ret == SERVER_ERROR_SUCCESS) {
									_state.Command.Argument = _state.Command.Argument*16 + digit;
									++_state.Index;
								}
							} else ret = SERVER_ERROR_INVALID_CONFIG_FIELD;
						} break;
						case pssParseConfigValue: {
							if (_state.Index < 5) {
								uint8_t digit;
								
								ret = _to_digit(Character, 10, &digit);
								if (ret == SERVER_ERROR_SUCCESS) {
									_state.Command.Argument16 = _state.Command.Argument16*10 + digit;
									++_state.Index;
									_state.done = 1;
								}
							} else ret = SERVER_ERROR_INVALID_CONFIG_FIELD;
						} break;
						default:
							ret = SERVER_ERROR_INVALID_INPUT;
							break;
					}
				}
			} break;
			case psDeleteInstruction: {
				if (Character == '-') {
					_state.SubState = ppsParseInstructionNumber;
					_state.Index = 0;
				} else {
					if (_state.SubState == ppsParseInstructionNumber) {
						if (_state.Index < 5) {
							uint8_t digit;
						
							ret = _to_digit(Character, 10, &digit);
							if (ret == SERVER_ERROR_SUCCESS) {
								_state.Index++;
								_state.Command.Argument16 = _state.Command.Argument16*10 + digit;
								_state.done = 1;
							}
						} else ret = SERVER_ERROR_INVALID_INSTRUCTION_NUMBER;
					} else ret = SERVER_ERROR_INVALID_INPUT;
				}
			} break;
			case psSelectProgram: {
				if (Character == '-') {
					_state.SubState = pssParseProgramNumber;
					_state.Command.Argument = 0;
					_state.Index = 0;
				} else {
					if (_state.SubState == pssParseProgramNumber) {
						if (_state.Index < 2) {
							uint8_t d = 0;
						
							ret = _to_digit(Character, 10, &d);
							if (ret == SERVER_ERROR_SUCCESS) {
								_state.Index++;
								_state.Command.Argument = _state.Command.Argument*10 + d;
								if (_state.Command.Argument < MAXIMUM_PROGRAM_COUNT)
									_state.done = 1;
							}
						} else ret = SERVER_ERROR_INVALID_INPUT;
					} else ret = SERVER_ERROR_INVALID_INPUT;
				}
			} break;
			case psRelativeTimeInstruction:
			case psAbsoluteTimeInstructions: {
				if (Character == '-') {
					switch (_state.SubState) {
						case pssUnknown:
							_state.SubState = pssParseHours;
							_state.Instruction.Hour = 0;
							_state.Index = 0;
							break;
						case pssParseMiliSeconds:
							_state.SubState = pssParseInstructionCommand;
							_state.Instruction.Type = pitNoInstruction;
							_state.Instruction.Argument = 0;
							_state.Index = 0;
							break;
						case pssParsePortLetter:
							if (is_port_byte_instruction(_state.Instruction.Type)) {
								_state.SubState = pssParseHexByte;
								_state.Instruction.Argument = 0;
								_state.Index = 0;
							} else ret = SERVER_ERROR_INVALID_INPUT;
							break;
						default:
							ret = SERVER_ERROR_INVALID_INPUT;
							break;
					}
				} else if (Character == ':') {
					switch (_state.SubState) {
						case pssParseHours:
							_state.SubState = pssParseMinutes;
							_state.Instruction.Minute = 0;
							_state.Index = 0;							
							break;
						case pssParseMinutes:
							_state.SubState = pssParseSeconds;
							_state.Instruction.Second = 0;
							_state.Index = 0;							
							break;
						case pssParseSeconds:
							_state.SubState = pssParseMiliSeconds;
							_state.Instruction.Milisecond = 0;
							_state.Index = 0;							
							break;
						default:
							ret = SERVER_ERROR_INVALID_INPUT;
							break;
					}
				} else {
					switch (_state.SubState) {
						case pssParseHexByte: {
							if (_state.Index < 2) {
								uint8_t digit;
								
								ret = _to_digit(Character, 16, &digit);
								if (ret == SERVER_ERROR_SUCCESS) {
									_state.Index++;
									_state.Instruction.Argument = _state.Instruction.Argument*16 + digit;
									_state.done = 1;
								}
							} else ret = SERVER_ERROR_INVALID_INPUT;
						} break;
						case pssParseHours:
						case pssParseMinutes:
						case pssParseSeconds: {							
							if (_state.Index < 2) {
								uint8_t digit = 0;
								
								ret = _to_digit(Character, 10, &digit);
								if (ret == SERVER_ERROR_SUCCESS) {
									_state.Index++;
									switch (_state.SubState) {
										case pssParseHours:
											_state.Instruction.Hour = _state.Instruction.Hour*10 + digit;
											break;
										case pssParseMinutes:
											_state.Instruction.Minute = _state.Instruction.Minute*10 + digit;
											if (_state.Instruction.Minute >= 60)
												ret = SERVER_ERROR_INVALID_MINUTE_VALUE;
											break;
										case pssParseSeconds:
											_state.Instruction.Second = _state.Instruction.Second*10 + digit;
											if (_state.Instruction.Second >= 60)
												ret = SERVER_ERROR_INVALID_SECOND_VALUE;
											break;
										default:
											ret = SERVER_ERROR_INVALID_INPUT;
											break;
									}
								}							
							} else ret = SERVER_ERROR_INVALID_INPUT;
						} break;
						case pssParseMiliSeconds: {
							if (_state.Index < 3) {
								uint8_t digit;
								
								ret = _to_digit(Character, 10, &digit);
								if (ret == SERVER_ERROR_SUCCESS) {
									_state.Index++;
									_state.Instruction.Milisecond = _state.Instruction.Milisecond*10 + digit;
								}
							} else ret = SERVER_ERROR_INVALID_INPUT;
						} break;
						case pssParseInstructionCommand: {
							if (_state.Index < 3) {
								_state.InputStorage[_state.Index] = Character;
								_state.Index++;
								 if (_state.Index == 3) {
									 if (compare_instruction_prefix(_state.InputStorage, 'w', 't', 'p')) {
										 init_instruction_info(pitWriteByteToA, pssParsePortLetter);
									 } else if (compare_instruction_prefix(_state.InputStorage, 'c', 'p', 'b')) {
										 init_instruction_info(pitClearBitInA, pssParsePortLetter);
									 } else if (compare_instruction_prefix(_state.InputStorage, 's', 'p', 'b')) {
										 init_instruction_info(pitSetBitInA, pssParsePortLetter);
									 } else if (compare_instruction_prefix(_state.InputStorage, 'd', 'o', 'b')) {
										 init_instruction_info(pitSetOutputA, pssParsePortLetter);
									 } else if (compare_instruction_prefix(_state.InputStorage, 'd', 'i', 'b')) {
										 init_instruction_info(pitSetInputA, pssParsePortLetter);
									 } else if (compare_instruction_prefix(_state.InputStorage, 'd', 'd', 'r')) {
										 init_instruction_info(pitSetDDRA, pssParsePortLetter);
									 } else if (compare_instruction_prefix(_state.InputStorage, 'r', 'p', 's')) {
										 init_instruction_info(pitPinAToSlot, pssParsePortLetter);
									 } else if (compare_instruction_prefix(_state.InputStorage, 'w', 'p', 's')) {
										 init_instruction_info(pitSlotToPortA, pssParsePortLetter);
									 } else ret = SERVER_ERROR_INVALID_INSTRUCTION_PREFIX;
								}
							} else ret = SERVER_ERROR_INVALID_INSTRUCTION_PREFIX;
						} break;
						case pssParsePortLetter: {
							EParserInstructionType pit = _state.Instruction.Type;
							
							if (Character >= 'A' && Character <= 'G')
								_state.Instruction.Type += (Character - 'A');
							else if (Character >= 'a' && Character <= 'g')
								_state.Instruction.Type += (Character - 'a');
							else ret = SERVER_ERROR_INVALID_PORT_LETTER;
						
							if (ret == SERVER_ERROR_SUCCESS) {
								if (pit == pitSetBitInA || pit == pitClearBitInA || pit == pitSetInputA || pit == pitSetOutputA) {
									_state.SubState = pssParseBitNumber;
									if (_state.Instruction.Type - pit == ('G' - 'A'))
										_state.SubState = pssParseGBitNumber;
								}
							}
						} break;
						case pssParseBitNumber:
						case pssParseGBitNumber: {
							if (Character >= '0' && (Character <= '4' || (_state.SubState == pssParseBitNumber && Character <= '7'))) {
								_state.Instruction.Argument = (Character - '0');
								_state.done = 1;
							} else ret = SERVER_ERROR_INVALID_BIT_NUMBER;
						} break;
						default:
							ret = SERVER_ERROR_INVALID_INPUT;
							break;
					}
				}
			} break;
			default:
				ret = SERVER_ERROR_INVALID_INPUT;
				break;
		}
	
	return ret;
}

void server_state(SERVER_STATE *State)
{
	memcpy(State, &_state, sizeof(_state));

	return;
}

void server_help(void)
{
	fs_send(FS_COPYRIGHT, 1);
	for (uint16_t i = FS_HELP_START; i < FS_HELP_END; ++i)
		fs_send(i, 1);
	
	return;
}
