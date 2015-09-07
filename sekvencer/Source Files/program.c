/*
 * program.c
 *
 * Created: 21. 6. 2015 17:04:45
 *  Author: Martin
 */ 

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "eeprom.h"
#include "comm.h"
#include "timer0.h"
#include "flash-strings.h"
#include "global_settings.h"
#include "program.h"


#define NO_PROGRAM_SELECTED_ID         0xFF
#define NO_PROGRAM_ACTIVE_ID           0xFF
#define EMPTY_INSTRUCTION_PLACE        0xFFFF

static unsigned char _activeProgramBuffer[sizeof(LOADED_PROGRAM) + 512*sizeof(PROGRAM_INSTRUCTION)];
static uint8_t _selectedProgramID = NO_PROGRAM_SELECTED_ID;
static LOADED_PROGRAM *_activeProgram = (LOADED_PROGRAM *)&_activeProgramBuffer;



/** Executes a given program instruction.
 *
 *  @param Slots An array of slots for the active program. The slots are
 *  used as a source or destination points for certain commands.
 *  @param Type Type of instruction to execute.
 *  @param Argument Instruction argument.
 */
static void execute(uint8_t *Slots, const EParserInstructionType Type, const uint8_t Arg)
{
	switch (Type) {
		case pitNoInstruction: break;
		case  pitWriteByteToA: PORTA = Arg; break;
		case pitWriteByteToB: PORTB = Arg; break;
		case pitWriteByteToC: PORTC = Arg; break;
		case pitWriteByteToD: PORTD = Arg; break;
		case pitWriteByteToE: PORTE = Arg; break;
		case pitWriteByteToF: PORTF = Arg; break;
		case pitWriteByteToG: PORTG = Arg; break;
		case pitSetBitInA: PORTA |= (1 << Arg); break;
		case pitSetBitInB: PORTB |= (1 << Arg); break;
		case pitSetBitInC: PORTC |= (1 << Arg); break;
		case pitSetBitInD: PORTD |= (1 << Arg); break;
		case pitSetBitInE: PORTE |= (1 << Arg); break;
		case pitSetBitInF: PORTF |= (1 << Arg); break;
		case pitSetBitInG: PORTG |= (1 << Arg); break;
		case pitClearBitInA: PORTA &= ~(1 << Arg); break;
		case pitClearBitInB: PORTB &= ~(1 << Arg); break;
		case pitClearBitInC: PORTC &= ~(1 << Arg); break;
		case pitClearBitInD: PORTD &= ~(1 << Arg); break;
		case pitClearBitInE: PORTE &= ~(1 << Arg); break;
		case pitClearBitInF: PORTF &= ~(1 << Arg); break;
		case pitClearBitInG: PORTG &= ~(1 << Arg); break;
		case pitSetInputA: DDRA &= ~(1 << Arg); break;
		case pitSetInputB: DDRB &= ~(1 << Arg); break;
		case pitSetInputC: DDRC &= ~(1 << Arg); break;
		case pitSetInputD: DDRD &= ~(1 << Arg); break;
		case pitSetInputE: DDRE &= ~(1 << Arg); break;
		case pitSetInputF: DDRF &= ~(1 << Arg); break;
		case pitSetInputG: DDRG &= ~(1 << Arg); break;
		case pitSetOutputA: DDRA |= (1 << Arg); break;
		case pitSetOutputB: DDRB |= (1 << Arg); break;
		case pitSetOutputC: DDRC |= (1 << Arg); break;
		case pitSetOutputD: DDRD |= (1 << Arg); break;
		case pitSetOutputE: DDRE |= (1 << Arg); break;
		case pitSetOutputF: DDRF |= (1 << Arg); break;
		case pitSetOutputG: DDRG |= (1 << Arg); break;
		case pitSetDDRA: DDRA = Arg; break;
		case pitSetDDRB: DDRB = Arg; break;
		case pitSetDDRC: DDRC = Arg; break;
		case pitSetDDRD: DDRD = Arg; break;
		case pitSetDDRE: DDRE = Arg; break;
		case pitSetDDRF: DDRF = Arg; break;
		case pitSetDDRG: DDRG = Arg; break;
		case pitPinAToSlot: Slots[Arg] = PINA; break;
		case pitPinBToSlot: Slots[Arg] = PINB; break;
		case pitPinCToSlot: Slots[Arg] = PINC; break;
		case pitPinDToSlot: Slots[Arg] = PIND; break;
		case pitPinEToSlot: Slots[Arg] = PINE; break;
		case pitPinFToSlot: Slots[Arg] = PINF; break;
		case pitPinGToSlot: Slots[Arg] = PING; break;
		case pitSlotToPortA: PORTA = Slots[Arg]; break;
		case pitSlotToPortB: PORTB = Slots[Arg]; break;
		case pitSlotToPortC: PORTC = Slots[Arg]; break;
		case pitSlotToPortD: PORTD = Slots[Arg]; break;
		case pitSlotToPortE: PORTE = Slots[Arg]; break;
		case pitSlotToPortF: PORTF = Slots[Arg]; break;
		case pitSlotToPortG: PORTG = Slots[Arg]; break;
		default: break;		
	}
	
	return;
}

static char _digit_to_char(const uint8_t Base, const uint8_t Digit)
{
	char ret;

	switch (Base) {
		case 2:
		case 10:
		ret = ('0' + Digit);
		break;
		case 16:
		if (Digit < 10)
		ret = ('0' + Digit);
		else ret = ('A' + Digit - 10);
		break;
		default:
		ret = 0xCC;
		break;
	}
	
	return ret;
}

#define _set_instruction_str_prefix(aBuffer, aCh1, aCh2, aCh3)	\
	{															\
		aBuffer[0] = aCh1;										\
		aBuffer[1] = aCh2;										\
		aBuffer[2] = aCh3;										\
	}															\

static void _instruction_to_string(const EParserInstructionType Type, const uint8_t Arg, char Str[8])
{
	if (Type >= pitWriteByteToA && Type <= pitWriteByteToG) {
		_set_instruction_str_prefix(Str, 'w', 't', 'p');
		Str[3] = ('A' + Type - pitWriteByteToA);
		Str[4] = '-';
		Str[5] = _digit_to_char(16, (Arg >> 4) & 0xf);
		Str[6] = _digit_to_char(16, Arg & 0xf);
		Str[7] = '\0';
	} else if (Type >= pitSetBitInA && Type <= pitSetBitInG) {
		_set_instruction_str_prefix(Str, 's', 'p', 'b');
		Str[3] = ('A' + Type - pitSetBitInA);		
		Str[4] = '0' + Arg;
		Str[5] = '\0';
	}  else if (Type >= pitClearBitInA && Type <= pitClearBitInG) {
		_set_instruction_str_prefix(Str, 'c', 'p', 'b');
		Str[3] = ('A' + Type - pitClearBitInA);
		Str[4] = '0' + Arg;
		Str[5] = '\0';
	} else if (Type >= pitSetInputA && Type <= pitSetInputG) {
		_set_instruction_str_prefix(Str, 'd', 'i', 'b');
		Str[3] = ('A' + Type - pitSetInputA);
		Str[4] = '0' + Arg;
		Str[5] = '\0';		
	} else if (Type >= pitSetOutputA && Type <= pitSetOutputG) {
		_set_instruction_str_prefix(Str, 'd', 'o', 'b');
		Str[3] = ('A' + Type - pitSetOutputA);
		Str[4] = '0' + Arg;
		Str[5] = '\0';		
	} else if (Type >= pitSetDDRA && Type <= pitSetDDRG) {
		_set_instruction_str_prefix(Str, 'd', 'd', 'r');
		Str[3] = ('A' + Type - pitSetDDRA);
		Str[4] = '-';
		Str[5] = _digit_to_char(16, (Arg >> 4) & 0xf);
		Str[6] = _digit_to_char(16, Arg & 0xf);
		Str[7] = '\0';
	} else if (Type >= pitPinAToSlot && Type <= pitPinGToSlot) {
		_set_instruction_str_prefix(Str, 'r', 'p', 's');
		Str[3] = ('A' + Type - pitPinAToSlot);
		Str[4] = '-';
		Str[5] = _digit_to_char(16, (Arg >> 4) & 0xf);
		Str[6] = _digit_to_char(16, Arg & 0xf);
		Str[7] = '\0';
	} else if (Type >= pitSlotToPortA && Type <= pitSlotToPortG) {
		_set_instruction_str_prefix(Str, 'w', 'p', 's');
		Str[3] = ('A' + Type - pitSlotToPortA);
		Str[4] = '-';
		Str[5] = _digit_to_char(16, (Arg >> 4) & 0xf);
		Str[6] = _digit_to_char(16, Arg & 0xf);
		Str[7] = '\0';
	} else {
		Str[0] = '<';
		Str[1] = 'U';
		Str[2] = 'N';
		Str[3] = 'K';
		Str[4] = 'N';
		Str[5] = 'N';		
		Str[6] = '>';
		Str[7] = '\0';
	}

	return;
}



#define _get_first_possible_instruction() \
	(uint16_t)(sizeof(EEPROM_PROGRAM)*MAXIMUM_PROGRAM_COUNT+sizeof(STORED_GLOBAL_SETTINGS))

#define _get_program_start_address(aProgramID) \
	((uint16_t)(aProgramID)*sizeof(EEPROM_PROGRAM) + sizeof(STORED_GLOBAL_SETTINGS))

#define _get_next_program_instruction_address(aCurrent) \
	myeeprom_read_word(aCurrent)

#define _get_program_first_instruction_address(aProgramID) \
	myeeprom_read_word(_get_program_start_address(aProgramID))

#define _program_exists(aProgramID) \
	(_get_program_first_instruction_address(aProgramID) != EMPTY_INSTRUCTION_PLACE)



static uint8_t _get_program_nth_instruction(const uint8_t ProgramID, uint16_t Index, uint16_t *Address, uint16_t *Prev, uint16_t *Next)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	uint16_t p = _get_program_start_address(ProgramID);
	uint16_t c = _get_program_first_instruction_address(ProgramID);
	uint16_t n = 0;
	
	if (c != 0) {
		n = _get_next_program_instruction_address(c);
		while (c != 0 && Index > 0) {
			--Index;
			p = c;
			c = n;
			n = _get_next_program_instruction_address(n);
		}
		
		if (c == 0)
			ret = PROGRAM_ERROR_NO_SUCH_INSTRUCTION_INDEX;
	} else ret = PROGRAM_ERROR_NO_SUCH_INSTRUCTION_INDEX;
	
	if (ret == PROGRAM_ERROR_SUCCESS) {
		*Address = c;
		*Prev = p;
		*Next = n;
	}
	
	return ret;
}

static uint16_t _get_program_last_instruction(uint8_t ProgramID)
{
	uint16_t next = 0;
	uint16_t ret = _get_program_start_address(ProgramID);
	
	do {
		next = _get_next_program_instruction_address(ret);
		if (next != 0)
			ret = next;
	} while (next != 0);
	
	if (ret == _get_program_start_address(ProgramID))
		ret = 0;
	
	return ret;
}

static uint16_t _read_program_instruction(uint16_t Address, PROGRAM_INSTRUCTION *Instruction)
{
	uint16_t ret;
	
	ret = myeeprom_read_word(Address);
	myeeprom_read_buffer(Address + sizeof(uint16_t), Instruction, sizeof(PROGRAM_INSTRUCTION));

	return ret;
}

static void _write_program_instruction(uint16_t PrevInstruction, uint16_t NextInstruction, uint16_t Address, PROGRAM_INSTRUCTION *Instruction)
{
	if (PrevInstruction != 0)
		myeeprom_write_word(PrevInstruction, Address);
		
	myeeprom_write_word(Address, NextInstruction);
	myeeprom_write_buffer(Address + sizeof(uint16_t), Instruction, sizeof(PROGRAM_INSTRUCTION));
	
	return;
}

static void _delete_program_instruction(uint16_t Prev, uint16_t Next, uint16_t Address)
{
	if (Prev != 0)
		myeeprom_write_word(Prev, Next);
	
	myeeprom_write_word(Address, EMPTY_INSTRUCTION_PLACE);
	
	return;
}

static uint16_t _find_empty_instruction(void)
{
	uint8_t found = 0;
	uint16_t ret = _get_first_possible_instruction();
	
	for (uint16_t i = ret; i < 4096; i += (sizeof(PROGRAM_INSTRUCTION) + sizeof(uint16_t))) {
		uint16_t n = myeeprom_read_word(i);	
		if (n == EMPTY_INSTRUCTION_PLACE) {
			ret = i;
			found = 1;
			break;
		}		
	}
	
	if (!found)
		ret = 0;
	
	return ret;
}

static uint16_t _find_previous_instruction(uint8_t ProgramID, uint32_t Miliseconds, uint16_t *Address)
{
	uint16_t ret = 0;
	uint16_t prev = _get_program_start_address(ProgramID);
	uint16_t cur = _get_program_first_instruction_address(ProgramID);
	
	if (cur != 0) {
		do {
			PROGRAM_INSTRUCTION inst;
				
			uint16_t next = _read_program_instruction(cur, &inst);
			if (inst.Milisecond > Miliseconds)
				break;
					
			prev = cur;
			cur = next;
		} while (cur != 0);
			
		*Address = prev;
		ret = cur;
	} else *Address = 0;
	
	return ret;
}

static void test_program_fill(void)
{
	_activeProgram->ID = 0;
	for (uint8_t i = 0; i < sizeof(_activeProgram->Slots) / sizeof(uint8_t); ++i)
		_activeProgram->Slots[i] = 0;
	
	_activeProgram->NumberOfInstructions = 16;
	for (uint32_t i = 0; i < 16; ++i) {
		PROGRAM_INSTRUCTION *ins = &_activeProgram->Instructions[i];
		
		ins->Milisecond = 100*((i >> 1) + 1);
		ins->Argument = ((i >> 1) - (i & 1)) & 0x7;
		if (i & 1)
			ins->Type = pitClearBitInA;
		else ins->Type = pitSetBitInA;
	}

	_activeProgram->CurrentInstruction = 0;
	_activeProgram->Active = 0;

	return;	
}

/************************************************************************/
/*                       PUBLIC FUNCTIONS                               */
/************************************************************************/

uint8_t program_select(uint8_t ID)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	
	if (ID < MAXIMUM_PROGRAM_COUNT) {
		uint16_t first = _get_program_first_instruction_address(ID);
		if (first != EMPTY_INSTRUCTION_PLACE) {
			if (_selectedProgramID != NO_PROGRAM_SELECTED_ID)
				program_unselect();
			
			_selectedProgramID = ID;
		} else ret = PROGRAM_ERROR_PROGRAM_DOES_NOT_EXIST;
	} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	
	return ret;
}

uint8_t program_unselect(void)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;

	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID) {
		if (_selectedProgramID < MAXIMUM_PROGRAM_COUNT) {
			_selectedProgramID = NO_PROGRAM_SELECTED_ID;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NO_PROGRAM_SELECTED;
	
	return ret;
}

uint8_t program_load(void)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	
	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID) {
		if (_selectedProgramID < MAXIMUM_PROGRAM_COUNT) {
			if (_activeProgram->ID != NO_PROGRAM_ACTIVE_ID && _activeProgram->Active)
				program_deactivate();

			_activeProgram->ID = _selectedProgramID;
			for (uint8_t i = 0; i < PROGRAM_SLOT_COUNT; ++i)
				_activeProgram->Slots[i] = 0;
			
			_activeProgram->NumberOfInstructions = 0;
			uint16_t instAddress = _get_program_first_instruction_address(_selectedProgramID);
			if (instAddress != 0) {
				uint16_t instCount = 0;
				
				do {
					instAddress = _read_program_instruction(instAddress, &_activeProgram->Instructions[instCount]);
					++instCount;
				} while (instAddress != 0);
				
				_activeProgram->NumberOfInstructions = instCount;
				_activeProgram->CurrentInstruction = 0;
			} else ret = PROGRAM_ERROR_NO_INSTRUCTIONS_AVAILABLE;
			
			if (ret != PROGRAM_ERROR_SUCCESS)
				_activeProgram->ID = NO_PROGRAM_ACTIVE_ID;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NO_PROGRAM_SELECTED;
		
	return ret;
}

uint8_t program_activate(void)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;

	if (_activeProgram->ID != NO_PROGRAM_SELECTED_ID) {
		if (_activeProgram->ID < MAXIMUM_PROGRAM_COUNT) {
			if (!_activeProgram->Active) {
				_activeProgram->Active = 1;
				_activeProgram->CurrentInstruction = 0;
				if (_activeProgram->Instructions[0].Milisecond > 0)
					timer0_start(_activeProgram->Instructions[0].Milisecond);
				else {
					// The purpose of this timer0_start call is just to start the timer.
					// The provided time interval never expires since it is changed inside
					// program_execute_instruction function.
					timer0_start(1000);
					program_execute_instruction();
				}
			} else ret = PROGRAM_ERROR_ALREADY_ACTIVE;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NOT_LOADED;
	
	return ret;
}

uint8_t program_deactivate(void)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	
	if (_activeProgram->ID != NO_PROGRAM_ACTIVE_ID) {
		if (_activeProgram->ID < MAXIMUM_PROGRAM_COUNT) {
			if (_activeProgram->Active) {
				timer0_stop();
				_activeProgram->Active = 0;
			} else ret = PROGRAM_ERROR_NO_PROGRAM_ACTIVE;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NOT_LOADED;
	
	return ret;
}

uint8_t program_set_abs_instruction(PROGRAM_INSTRUCTION *Instruction)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	
	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID) {
		if (_selectedProgramID < MAXIMUM_PROGRAM_COUNT) {
			uint16_t prev;
			uint16_t next;
			uint16_t new;
			
			new = _find_empty_instruction();
			if (new != 0) {
				next = _find_previous_instruction(_selectedProgramID, Instruction->Milisecond, &prev);
				if (prev == 0) {
					prev = _get_program_start_address(_selectedProgramID);
					next = _get_program_first_instruction_address(_selectedProgramID);
				}
				
				_write_program_instruction(prev, next, new, Instruction);
			} else ret = PROGRAM_ERROR_NO_INSTRUCTIONS_AVAILABLE;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NO_PROGRAM_SELECTED;
	
	return ret;	
}

uint8_t program_set_rel_instruction(PROGRAM_INSTRUCTION *Instruction)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	
	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID) {
		if (_selectedProgramID < MAXIMUM_PROGRAM_COUNT) {
			uint16_t new = _find_empty_instruction();
			if (new != 0) {
				uint16_t last = _get_program_last_instruction(_selectedProgramID);
			
				if (last != 0) {
					PROGRAM_INSTRUCTION tmpInst;
					
					ret = _read_program_instruction(last, &tmpInst);
					if (ret == PROGRAM_ERROR_SUCCESS)
						Instruction->Milisecond += tmpInst.Milisecond;
				} else last = _get_program_start_address(_selectedProgramID);
				
				if (ret == PROGRAM_ERROR_SUCCESS)
				_write_program_instruction(last, 0, new, Instruction);
			} else ret = PROGRAM_ERROR_NO_INSTRUCTIONS_AVAILABLE;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NO_PROGRAM_SELECTED;
	
	return ret;	
}
uint8_t program_delete_instruction(uint16_t InstructionIndex)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	
	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID) {
		if (_selectedProgramID < MAXIMUM_PROGRAM_COUNT) {
			uint16_t p;
			uint16_t n;
			uint16_t c;
			
			ret = _get_program_nth_instruction(_selectedProgramID, InstructionIndex, &c, &p, &n);
			if (ret == PROGRAM_ERROR_SUCCESS)
				_delete_program_instruction(p, n, c);
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NO_PROGRAM_SELECTED;
	
	return ret;
}

void program_execute_instruction(void)
{
	uint32_t nextTime;
	
	if (_activeProgram->ID != NO_PROGRAM_ACTIVE_ID && _activeProgram->Active) {
		do {
			PROGRAM_INSTRUCTION *inst = &_activeProgram->Instructions[_activeProgram->CurrentInstruction];

			EParserInstructionType type = inst->Type;
			uint8_t arg = inst->Argument;
	
			PROGRAM_INSTRUCTION *nextInst;	
			_activeProgram->CurrentInstruction++;
			if (_activeProgram->CurrentInstruction < _activeProgram->NumberOfInstructions) {
				nextInst = &_activeProgram->Instructions[_activeProgram->CurrentInstruction];
				nextTime = nextInst->Milisecond - inst->Milisecond;
				if (nextTime > 0)
					timer0_plan_next(nextTime);
			} else {
				_activeProgram->CurrentInstruction = 0;
				nextInst = &_activeProgram->Instructions[0];
				nextTime = nextInst->Milisecond;
				if (nextTime > 0)
					timer0_plan_next(nextTime);
			}
		
			execute(_activeProgram->Slots, type, arg);		
		} while (nextTime == 0);
	}
	
	return;
}

uint8_t program_create(void)
{
	uint8_t ret = PROGRAM_ERROR_NO_PROGRAM_ID_AVAILABLE;
	
	for (uint8_t i = 0; i < MAXIMUM_PROGRAM_COUNT; ++i) {
		if (!_program_exists(i)) {
			ret = i;
			_selectedProgramID = ret;
			myeeprom_write_word(_get_program_start_address(ret), 0);
			break;
		}		
	}
	
	return ret;
}

uint8_t program_delete(void)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;
	
	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID) {
		if (_selectedProgramID < MAXIMUM_PROGRAM_COUNT) {
			uint16_t start = _get_program_start_address(_selectedProgramID);
			uint16_t instAddress = _get_program_first_instruction_address(_selectedProgramID);
			myeeprom_write_word(start, EMPTY_INSTRUCTION_PLACE);
			while (instAddress != 0) {
				start = _get_next_program_instruction_address(instAddress);
				myeeprom_write_word(instAddress, EMPTY_INSTRUCTION_PLACE);
				instAddress = start;				
			}			
			
			_selectedProgramID = NO_PROGRAM_SELECTED_ID;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NO_PROGRAM_SELECTED;
	
	return ret;	
}

uint8_t program_list(void)
{
	uint8_t ret = PROGRAM_ERROR_SUCCESS;

	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID) {
		if (_selectedProgramID <= MAXIMUM_PROGRAM_COUNT) {
			uint16_t inst = _get_program_first_instruction_address(_selectedProgramID);
			if (inst != EMPTY_INSTRUCTION_PLACE) {
				uint16_t instCount = 0;

				fs_send(FS_SELECTED_PROGRAM, 0);
				comm_send_dec_number8(0, _selectedProgramID);
				comm_send_newline();
				if (_activeProgram->ID == _selectedProgramID && _activeProgram->Active) {
					fs_send(FS_PROGRAM_SLOT_LISTING, 1);				
					for (uint8_t i = 0; i < PROGRAM_SLOT_COUNT; ++i) {
						comm_send_byte(' ');
						comm_send_byte(' ');
						comm_send_dec_number8(2, i);
						comm_send_byte(' ');
						comm_send_byte('=');
						comm_send_byte(' ');
						comm_send_dec_number8(0, _activeProgram->Slots[i]);
						comm_send_newline();										
					}
				}

				fs_send(FS_PROGRAM_INST_LISTING, 1);
				while (inst != 0) {
					char instStr[8];
					PROGRAM_INSTRUCTION d;

					inst = _read_program_instruction(inst, &d);
					_instruction_to_string(d.Type, d.Argument, instStr);
					comm_send_byte(' ');
					comm_send_byte(' ');
					comm_send_dec_number16(0, instCount);
					comm_send_byte(':');
					comm_send_byte(' ');
					comm_send_time(d.Milisecond);
					comm_send_byte(' ');
					comm_send_string(instStr);
					comm_send_newline();
					++instCount;
				}

				comm_send_dec_number16(0, instCount);
				fs_send(FS_PROGRAM_INST_COUNT, 1);
				comm_send_newline();
			} else ret = PROGRAM_ERROR_PROGRAM_DOES_NOT_EXIST;
		} else ret = PROGRAM_ERROR_INVALID_PROGRAM_NUMBER;
	} else ret = PROGRAM_ERROR_NO_PROGRAM_SELECTED;

	return ret;	
}

void programs_enumerate(void)
{
	uint8_t programCount = 0;
	
	fs_send(FS_PROGRAM_LISTING, 1);
	for (uint8_t i = 0; i < MAXIMUM_PROGRAM_COUNT; ++i) {
		uint16_t inst = _get_program_first_instruction_address(i);
		
		if (inst != EMPTY_INSTRUCTION_PLACE) {
			uint16_t instCount = 0;
			
			++programCount;
			while (inst != 0) {
				inst = _get_next_program_instruction_address(inst);
				++instCount;
			}
			
			comm_send_byte(' ');
			comm_send_byte(' ');
			comm_send_byte('I');
			comm_send_byte('D');			
			comm_send_byte('=');
			comm_send_dec_number8(2, i);
			comm_send_byte(':');
			comm_send_byte(' ');
			comm_send_dec_number16(0, instCount);
			fs_send(FS_PROGRAM_INST_COUNT, 1);
		}
	}
	
	fs_send(FS_PROGRAM_COUNT, 0);
	comm_send_dec_number8(0, programCount);
	comm_send_newline();
	fs_send(FS_PROGRAM_MAX, 0);
	comm_send_dec_number8(0, MAXIMUM_PROGRAM_COUNT);
	comm_send_newline();	
	fs_send(FS_SELECTED_PROGRAM, 0);
	if (_selectedProgramID != NO_PROGRAM_SELECTED_ID)
		comm_send_dec_number8(0, _selectedProgramID);
	else fs_send(FS_NONE, 0);
	
	comm_send_newline();
	fs_send(FS_ACTIVE_PROGRAM, 0);
	if (_activeProgram->ID != NO_PROGRAM_ACTIVE_ID)
		comm_send_dec_number8(0, _activeProgram->ID);
	else fs_send(FS_NONE, 0);
	
	comm_send_newline();
	
	return;
}

void programs_init_eeprom(void)
{	
	for (uint8_t i = 0; i < MAXIMUM_PROGRAM_COUNT; ++i) {
		uint16_t addr = _get_program_start_address(i);
		myeeprom_write_word(addr, EMPTY_INSTRUCTION_PLACE);
	}

	test_program_fill();
	uint16_t prev = _get_program_start_address(_activeProgram->ID);
	for (uint16_t i = 0; i < _activeProgram->NumberOfInstructions; ++i) {
		uint16_t new = _find_empty_instruction();
		_write_program_instruction(prev, 0, new, &_activeProgram->Instructions[i]);
		prev = new;
	}

	_selectedProgramID = NO_PROGRAM_SELECTED_ID;
	_activeProgram->ID = NO_PROGRAM_ACTIVE_ID;

	return;
}

void programs_module_init(void)
{
	_activeProgram->ID = NO_PROGRAM_ACTIVE_ID;
	_activeProgram->Active = 0;
	
	return;
}
