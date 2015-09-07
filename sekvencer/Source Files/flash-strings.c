/*
 * flash_strings.c
 *
 * Created: 23. 6. 2015 21:28:38
 *  Author: Martin
 */ 


#include <avr/pgmspace.h>
#include "error.h"
#include "comm.h"
#include "flash-strings.h"


static const char _helpString1  [] PROGMEM = {"SEKVENCER v1.0, Copyright (C) NPRG037, 2014/2015"};
static const char _helpString2  [] PROGMEM = {"List of available and init commands:"};
static const char _helpString3  [] PROGMEM = {"Test (debugging) commands:"};
static const char _helpString4  [] PROGMEM = {"  U - dumps all the EEPROM (sends its content over USART0)"};
static const char _helpString5  [] PROGMEM = {"  r - Erase the whole EEPROM (all cells are filled with ones)"};
static const char _helpString6  [] PROGMEM = {"  I - Initializes the EEPROM and writes a test program to it"};
static const char _helpString7  [] PROGMEM = {"  h - Shows this help"};
static const char _helpString8  [] PROGMEM = {"Commands dealing with programs:"};
static const char _helpString9  [] PROGMEM = {"  l - Loads selected program to SRAM"};
static const char _helpString10 [] PROGMEM = {"  a - activates (runs) program currently stored in SRAM"};
static const char _helpString11 [] PROGMEM = {"  d - deactivates the currently running program"};
static const char _helpString12 [] PROGMEM = {"  S-N - select a program with ID N (a decimal number)"};
static const char _helpString13 [] PROGMEM = {"  u - unselect currently selected program"};
static const char _helpString14 [] PROGMEM = {"  L - prints detailed information about selected program"};
static const char _helpString15 [] PROGMEM = {"  E - prints information about all programs currently stored in EEPROM"};
static const char _helpString16 [] PROGMEM = {"  D - deletes the selected program"};
static const char _helpString17 [] PROGMEM = {"  T-hh:mm:ss:MMM-<instruction> - inserts an instruction into a selected program with its time of action set to hh:mm:ss:MMM (decilal numbers)"};
static const char _helpString18 [] PROGMEM = {"  R-hh:mm:ss:MMM-<instruction> - inserts an instruction into a selected program with its time of action set to hh:mm:ss:MMM after program's last instruction"};
static const char _helpString19 [] PROGMEM = {"  e-N - deletes NNNNth instruction of a selected program (N is a decimal number between 0 and 65535)"};
static const char _helpString20 [] PROGMEM = {"Available instructions:"};
static const char _helpString21 [] PROGMEM = {"  wtp[A-G]-NN - writes a byte NN to port X (NN is a strictly two-digit hexadecimal number)"};
static const char _helpString22 [] PROGMEM = {"  spb[A-G-][0-7] - sets a given bit on port X"};
static const char _helpString23 [] PROGMEM = {"  cpb[A-G]-[0-7] - clears a bit on port X"};
static const char _helpString24 [] PROGMEM = {"  dob[A-G]-[0-7] - sets a given pin as an output pin"};
static const char _helpString25 [] PROGMEM = {"  dib[A-G]-[0-7] - sets a given pin as an input pin"};
static const char _helpString26 [] PROGMEM = {"  ddr[A-G]-NN - writes a byte to DDRX register"};
static const char _helpString27 [] PROGMEM = {"  rps[A-G]-NN - reads a byte from pin X and saves it to slot NN (NN is a hexadecimal strictly two-digit number)"};
static const char _helpString28 [] PROGMEM = {"  wps[A-G]-NN - writes a byte stored in slot NN to port X (NN is a hexadecimal strictly two-digit number)"};
static const char _helpString29 [] PROGMEM = {"Commands dealing with global configuration:"};
static const char _helpString30 [] PROGMEM = {"  G - read global configuration"};
static const char _helpString31 [] PROGMEM = {"  gH-N - change Hth field of the global configuration to value N"};
static const char _programCreated [] PROGMEM = {"A new program has been created. ID = "};
static const char _activeProgram [] PROGMEM = {"Loaded program:   "};
static const char _selectedProgram [] PROGMEM = {"Selected program: "};
static const char _numberOfPrograms [] PROGMEM = {"Number of programs: "};
static const char _maxPrograms [] PROGMEM = {"Max. programs: "};
static const char _programListing [] PROGMEM = {"Program listing: "};
static const char _programInstCount [] PROGMEM = {" instructions"};
static const char _none [] PROGMEM = {"<none>"};
static const char _programInstructionListing [] PROGMEM = {"Instruction listing:"};
static const char _programSlotListing [] PROGMEM = {"Program slot listing:"};
static const char _gsUSART0Baudrate [] PROGMEM = {"USART0 baud rate selector: "};
static const char _gsUSART0StopBits [] PROGMEM = {"USART0 baud stop bits selector: "};
static const char _gsUSART0Parity   [] PROGMEM = {"USART0 baud parity selector: "};
static const char _gsTWISlaveAddress [] PROGMEM = {"TWI slave address: "};
static const char _gsCharacterEcho  [] PROGMEM = {"Echo received input: "};
static const char _gsNewLineAfter [] PROGMEM = {"Send newline after a command is executed or an an error occurs: "};
static const char _gsNewLineBefore [] PROGMEM = {"Send newline before attempting to execude a command: "};
static const char _gsOK [] PROGMEM = {"OK"};
static const char _gsError [] PROGMEM = {"ERR #"};
static const char _gsTrue [] PROGMEM = {"true"};
static const char _gsFalse [] PROGMEM = {"false"};
static const char _gsReportErrorMessages [] PROGMEM = {"Report error messages: "};
static const char _gsDisableEEPROMReset [] PROGMEM = {"Disable EEPROM reset command (r): "};
static const char _gsDisableEEPROMInit [] PROGMEM = {"Disable EEPROM init command (I): "};
static const char _gsDisableEEPROMDump [] PROGMEM = {"Disable EEPROM dump command (U): "};

static const char* const _stringTable[] PROGMEM = {
	_helpString1,
	_helpString2,
	_helpString3,
	_helpString4,
	_helpString5,
	_helpString6,
	_helpString7,
	_helpString8,
	_helpString9,	
	_helpString10,
	_helpString11,
	_helpString12,
	_helpString13,
	_helpString14,
	_helpString15,
	_helpString16,
	_helpString17,
	_helpString18,
	_helpString19,
	_helpString20,
	_helpString21,
	_helpString22,
	_helpString23,
	_helpString24,
	_helpString25,
	_helpString26,
	_helpString27,	
	_helpString28,
	_helpString29,
	_helpString30,
	_helpString31,	
	_programCreated,
	_activeProgram,
	_selectedProgram,
	_numberOfPrograms,
	_maxPrograms,
	_programListing,
	_programInstCount,
	_none,
	_programInstructionListing,
	_programSlotListing,
	_gsUSART0Baudrate,
	_gsUSART0StopBits,
	_gsUSART0Parity,
	_gsTWISlaveAddress,
	_gsCharacterEcho,
	_gsNewLineAfter,
	_gsNewLineBefore,
	_gsOK,
	_gsError,
	_gsTrue,
	_gsFalse,
	_gsReportErrorMessages,
	_gsDisableEEPROMReset,
	_gsDisableEEPROMInit,
	_gsDisableEEPROMDump,
};

static const char _serverError1 [] PROGMEM = {"Unknown command"};
static const char _serverError2 [] PROGMEM = {"Invalid input"};
static const char _serverError3 [] PROGMEM = {"The character is not a hex digit"};
static const char _serverError4 [] PROGMEM = {"The character is not a decimal digit"};
static const char _serverError5 [] PROGMEM = {"The character is not a binary digit"};
static const char _serverError6 [] PROGMEM = {"The number base is invalid"};
static const char _serverError7 [] PROGMEM = {"Invalid program ID"};
static const char _serverError8 [] PROGMEM = {"Invalid value for minutes"};
static const char _serverError9 [] PROGMEM = {"Invalid value for seconds"};
static const char _serverError10 [] PROGMEM = {"Invalid instruction index"};
static const char _serverError11 [] PROGMEM = {"Invalid instruction prefix"};
static const char _serverError12 [] PROGMEM = {"Invalid port letter"};
static const char _serverError13 [] PROGMEM = {"Invalid bit number"};
static const char _serverError14 [] PROGMEM = {"Command not finished"};
static const char _serverError15 [] PROGMEM = {"Invalid config field"};

static const char _programError64 [] PROGMEM = {"Invalid program index"};
static const char _programError65 [] PROGMEM = {"No program selected"};
static const char _programError66 [] PROGMEM = {"No program active"};
static const char _programError67 [] PROGMEM = {"Program does not exist"};
static const char _programError68 [] PROGMEM = {"No instructions in the program"};
static const char _programError69 [] PROGMEM = {"Invalid instruction index"};
static const char _programError70 [] PROGMEM = {"No program loaded in SRAM"};
static const char _programError71 [] PROGMEM = {"Program is already running"};
static const char _programError72 [] PROGMEM = {"The command is disabled by the global settings "};
static const char _programErrorFF [] PROGMEM = {"No program ID available"};
	
static const char _configError128 [] PROGMEM = {"Invalid config field"};
static const char _configError129 [] PROGMEM = {"Invalid config value"};

static const char *const _serverErrorTable [] PROGMEM = {
	_serverError1,
	_serverError2,
	_serverError3,
	_serverError4,
	_serverError5,
	_serverError6,
	_serverError7,
	_serverError8,
	_serverError9,
	_serverError10,
	_serverError11,
	_serverError12,
	_serverError13,
	_serverError14,
	_serverError15,
};

static const char *const _programErrorTable [] PROGMEM = {
	_programError64,
	_programError65,
	_programError66,
	_programError67,
	_programError68,
	_programError69,
	_programError70,
	_programError71,
	_programError72,	
	_programErrorFF	
};

static const char *const _configErrorTable [] PROGMEM = {
	_configError128,
	_configError129,
};


void fs_send_error_message(const uint8_t ErrorCode, const uint8_t NewLine)
{
	char *pgmStr = 0;
	
	if (ErrorCode >= SERVER_ERROR_MIN && ErrorCode <= SERVER_ERROR_MAX)
		pgmStr = pgm_read_ptr_far(&_serverErrorTable[ErrorCode - SERVER_ERROR_MIN]);
	else if (ErrorCode >= PROGRAM_ERROR_MIN && ErrorCode <= PROGRAM_ERROR_MAX)
		pgmStr = pgm_read_ptr_far(&_programErrorTable[ErrorCode - PROGRAM_ERROR_MIN]);
	else if (ErrorCode >= CONFIG_ERROR_MIN && ErrorCode <= CONFIG_ERROR_MAX)
		pgmStr = pgm_read_ptr_far(&_configErrorTable[ErrorCode - CONFIG_ERROR_MIN]);
	else if (ErrorCode == PROGRAM_ERROR_NO_PROGRAM_ID_AVAILABLE)
		pgmStr = pgm_read_ptr_far(&_programErrorFF);
	
	uint8_t Char = pgm_read_byte_far(pgmStr);
	while (Char != '\0') {
		comm_send_buffer(&Char, sizeof(Char));
		pgmStr++;
		Char = pgm_read_byte_far(pgmStr);
	}
	
	if (NewLine)
	comm_send_newline();
		
	return;
}

void fs_send(const uint16_t Index, const uint8_t NewLine)
{
	char *pgmStr = pgm_read_ptr_far(&_stringTable[Index]);

	uint8_t Char = pgm_read_byte_far(pgmStr);
	while (Char != '\0') {
		comm_send_buffer(&Char, sizeof(Char));
		pgmStr++;
		Char = pgm_read_byte_far(pgmStr);
	}	
	
	if (NewLine)
		comm_send_newline();
	
	return;
}
