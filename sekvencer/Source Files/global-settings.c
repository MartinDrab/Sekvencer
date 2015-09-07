/*
 * global_settings.c
 *
 * Created: 24. 6. 2015 23:43:23
 *  Author: Martin
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "error.h"
#include "eeprom.h"
#include "flash-strings.h"
#include "comm.h"
#include "global_settings.h"


static const STORED_GLOBAL_SETTINGS _defaultSettings PROGMEM = {
	{'S', 'Q', 'C', 'R'},   // Signature
	{
		9600,                      // Baud rate selector
		1,                      // Stop bits
		0,                      // Parity
		21,                   // TWI Slave Address
		1,                      // Echo characters
		1,						// Newline after
		1,						// Newline before
		1,						// Error messages
		0,						// Disable EEPROM reset
		0,						// Disable EEPROM init
		0,						// Disable EEPROM dump
	}
};


GLOBAL_SETTINGS_SRAM GlobalSettings;



#define signature_valid(aSignature) \
	(aSignature[0] == 'S' && aSignature[1] == 'Q' && aSignature[2] == 'C' && aSignature[3] == 'R')


void global_settings_init(void)
{
	STORED_GLOBAL_SETTINGS gs;
	uint8_t *dst = (uint8_t *)&gs;
	
	for (uint8_t i = 0; i < sizeof(STORED_GLOBAL_SETTINGS); ++i) {
		*dst = pgm_read_byte_far((uint8_t *)&_defaultSettings + i);
		++dst;
	}	
	
	if (signature_valid(gs.Signature)) {
		GlobalSettings = gs.Settings;
	} else {
		GlobalSettings.CharacterEcho = 1;
		GlobalSettings.TWISlaveAddress = 21;
		GlobalSettings.USART0BaudRate = 9600;
		GlobalSettings.USART0Parity = 0;
		GlobalSettings.USART0StopBits = 1;
		GlobalSettings.NewLineAfter = 1;
		GlobalSettings.NewLineBefore = 1;
		GlobalSettings.PrintErrorMessages = 1;
		GlobalSettings.DisableEEPROMReset = 0;
		GlobalSettings.DisableEEPROMInit = 0;
		GlobalSettings.DisableEEPROMDump = 0;
	}
	
	return;
}

void global_settings_save(void)
{
	STORED_GLOBAL_SETTINGS sgs;
	
	sgs.Signature[0] = 'S';
	sgs.Signature[1] = 'Q';
	sgs.Signature[2] = 'C';
	sgs.Signature[3] = 'R';
	sgs.Settings = GlobalSettings;
	myeeprom_write_buffer(0, (uint8_t *)&sgs, sizeof(sgs));
	
	return;
}

uint8_t global_settings_load(void)
{
	uint8_t ret = 0;
	STORED_GLOBAL_SETTINGS sgs;

	myeeprom_read_buffer(0, sgs.Signature, sizeof(sgs.Signature));	
	ret = signature_valid(sgs.Signature);	
	if (ret) {
		myeeprom_read_buffer(sizeof(sgs.Signature), (uint8_t *)&sgs + sizeof(sgs.Signature), sizeof(sgs) - sizeof(sgs.Signature));
		GlobalSettings = sgs.Settings;
	}
	
	return ret;
}

void global_settings_print(void)
{
	fs_send(FS_GS_USART0_BAUDRATE, 0);
	comm_send_dec_number16(0, GlobalSettings.USART0BaudRate);
	comm_send_newline();
	fs_send(FS_GS_USART0_STOP_BITS, 0);
	comm_send_dec_number8(0, GlobalSettings.USART0StopBits);
	comm_send_newline();
	fs_send(FS_GS_USART0_PARITY, 0);
	comm_send_dec_number8(0, GlobalSettings.USART0Parity);
	comm_send_newline();
	fs_send(FS_GS_TWI_SLAVE_ADDRESS, 0);
	comm_send_number8(GlobalSettings.TWISlaveAddress);
	comm_send_newline();
	fs_send(FS_GS_CHARACTER_ECHO, 0);
	comm_send_boolean(GlobalSettings.CharacterEcho);
	comm_send_newline();
	fs_send(FS_NEWLINE_AFTER, 0);
	comm_send_boolean(GlobalSettings.NewLineAfter);
	comm_send_newline();
	fs_send(FS_NEWLINE_BEFORE, 0);
	comm_send_boolean(GlobalSettings.NewLineBefore);	
	comm_send_newline();
	fs_send(FS_REPORT_ERROR_MESSAGES, 0);
	comm_send_boolean(GlobalSettings.PrintErrorMessages);
	comm_send_newline();
	fs_send(FS_DISABLE_EEPROM_RESET, 0);
	comm_send_boolean(GlobalSettings.DisableEEPROMReset);
	comm_send_newline();
	fs_send(FS_DISABLE_EEPROM_INIT, 0);
	comm_send_boolean(GlobalSettings.DisableEEPROMInit);
	comm_send_newline();
	fs_send(FS_DISABLE_EEPROM_DUMP, 0);
	comm_send_boolean(GlobalSettings.DisableEEPROMDump);
	comm_send_newline();
	
	return;
}

uint8_t global_settings_change(uint8_t Field, uint16_t Value)
{
	uint8_t ret = CONFIG_ERROR_SUCCESS;
	
	switch (Field) {
		case 0: {
			if (Value == 4800 || Value == 9600 || Value == 19200 || Value == 38400)
				GlobalSettings.USART0BaudRate = Value;
			else ret = CONFIG_ERROR_INVALID_VALUE;
		} break;
		case 1: {
			if (Value > 0 && Value <= 2)
				GlobalSettings.USART0StopBits = Value;
			else ret = CONFIG_ERROR_INVALID_VALUE;
		} break;
		case 2: {
			if (Value >= 0 && Value <= 3 && Value != 1)
				GlobalSettings.USART0Parity = Value;
			else ret = CONFIG_ERROR_INVALID_VALUE;
		} break;
		case 3: {
			if (Value > 0 && Value < 128)
				GlobalSettings.TWISlaveAddress = Value;
			else ret = CONFIG_ERROR_INVALID_VALUE;
		} break;
		case 4: {
			GlobalSettings.CharacterEcho = Value;
		} break;
		case 5: {
			GlobalSettings.NewLineAfter = Value;
		} break;
		case 6: {
			GlobalSettings.NewLineBefore = Value;
		} break;
		case 7: {
			GlobalSettings.PrintErrorMessages = Value;
		} break;	
		case 8: {
			GlobalSettings.DisableEEPROMReset = Value;
		} break;
		case 9: {
			GlobalSettings.DisableEEPROMInit = Value;
		} break;
		case 10: {
			GlobalSettings.DisableEEPROMDump = Value;
		} break;
		default: {
			ret = CONFIG_ERROR_INVALID_FIELD;
		} break;		
	}
	
	return ret;
}
