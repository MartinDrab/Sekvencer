/*
 * sekvencer.c
 *
 * Created: 18. 6. 2015 21:52:46
 *  Author: Martin
 */ 

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "eeprom.h"
#include "timer0.h"
#include "usart0.h"
#include "twi-slave.h"
#include "comm.h"
#include "flash-strings.h"
#include "program.h"
#include "server.h"
#include "global_settings.h"


void init(void)
{
	DDRA = 0xff;
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;
	DDRE = 0xff;
	DDRF = 0xff;
	DDRG = 0x1f;
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE  = 0;
	PORTF = 0;
	PORTG = 0;
	set_sleep_mode(0);
	
	return;
}

void eeprom_erase(void)
{
	for (uint16_t i = 0; i < 4096; i+=2)
		myeeprom_write_word(i, 0xFFFF);
	
	return;
}

void eeprom_dump(void)
{
	uint8_t buf[16];
	
	for (uint16_t i = 0; i < 4096; i += sizeof(buf)) {	
		myeeprom_read_buffer(i, buf, sizeof(buf));
		comm_send_buffer(buf, sizeof(buf));
	}
	
	return;
}

void report_error(const uint8_t ErrorCode)
{
	fs_send(FS_ERR, 0);
	comm_send_dec_number8(0, ErrorCode);
	if (GlobalSettings.PrintErrorMessages) {
		comm_send_byte(' ');
		fs_send_error_message(ErrorCode, 0);
	}
	
	return;
}

int main(void)
{	
	init();
	myeeprom_set_inteligent_write(1);
	global_settings_init();
	global_settings_load();
	twi_slave_init(GlobalSettings.TWISlaveAddress);
	if (usart0_init(GlobalSettings.USART0BaudRate, GlobalSettings.USART0StopBits, GlobalSettings.USART0Parity) == USART_ERROR_SUCCESS) {		 
		uint8_t err = PROGRAM_ERROR_SUCCESS;

		usart0_start_receive();
		usart0_start_send();
		server_reset();
		programs_module_init();
		while (1) {
			uint8_t b;
			
			cli();
			while (!comm_byte_received()) {
				sleep_enable();
				sei();
				sleep_cpu();
				sleep_disable();
				cli();
			}
			
			sei();
			if (comm_get_byte(&b)) {
				if (b == 13) {
					SERVER_STATE serverState;
					server_state(&serverState);
					if (GlobalSettings.NewLineBefore) {
						if (!serverState.done || serverState.State != psDumpEEPROM)
							comm_send_newline();
					}
					
					if (serverState.done) {
						err = PROGRAM_ERROR_SUCCESS;
						switch (serverState.State) {
							case psHelp:
								server_help();
								break;
							case psDumpEEPROM:
								if (!GlobalSettings.DisableEEPROMDump)
									eeprom_dump();
								else err = PROGRAM_ERROR_COMMAND_DISABLED;
								break;
							case psInitEEPROM:
								if (!GlobalSettings.DisableEEPROMInit) {
									programs_init_eeprom();
									global_settings_init();
									global_settings_save();
								} else err = PROGRAM_ERROR_COMMAND_DISABLED;
								break;	
							case psEraseEEPROM:
								if (!GlobalSettings.DisableEEPROMReset)
									eeprom_erase();
								else err = PROGRAM_ERROR_COMMAND_DISABLED;
								break;
								
							case psDeactivateProgram:
								err = program_deactivate();
								break;
							case psActivateProgram:
								err = program_activate();
								break;
							case psLoadProgram:
								err = program_load();
								break;
							case psSelectProgram:
								err = program_select(serverState.Command.Argument);
								break;
							case psUnselectProgram:
								err = program_unselect();
								break;
							case psDeleteProgram:
								err = program_delete();
								break;
							case psCreateProgram:
								err = program_create();
								if (err != PROGRAM_ERROR_NO_PROGRAM_ID_AVAILABLE) {
									fs_send(FS_PROGRAM_CREATED, 0);
									comm_send_number8(err);
									comm_send_newline();
									err = SERVER_ERROR_SUCCESS;
								}
								break;
							case psEnumeratePrograms:
								programs_enumerate();
								break;
							case psListProgramInstructions:
								err = program_list();
								break;
								
							case psAbsoluteTimeInstructions: 
							case psRelativeTimeInstruction: {
								PROGRAM_INSTRUCTION inst;
								
								inst.Type = serverState.Instruction.Type;
								inst.Argument = serverState.Instruction.Argument;
								inst.Milisecond = serverState.Instruction.Hour;
								inst.Milisecond *= 60;
								inst.Milisecond += serverState.Instruction.Minute;
								inst.Milisecond *= 60;
								inst.Milisecond += serverState.Instruction.Second;
								inst.Milisecond *= 1000;
								inst.Milisecond += serverState.Instruction.Milisecond;
								if (serverState.State == psAbsoluteTimeInstructions)
									err = program_set_abs_instruction(&inst);
								else err = program_set_rel_instruction(&inst);
								} break;
							case psDeleteInstruction:
								err = program_delete_instruction(serverState.Command.Argument16);
								break;
								
							case psPrintConfig:
								global_settings_print();
								comm_send_newline();
								break;
							case psWriteConfig:
								err = global_settings_change(serverState.Command.Argument, serverState.Command.Argument16);
								if (err == CONFIG_ERROR_SUCCESS)
									global_settings_save();
								break;
								
							default:
								break;
						}
					} else {
						if (err == SERVER_ERROR_SUCCESS)
							err = SERVER_ERROR_COMMAND_NOT_FINISHED;
					}
					
					if (!serverState.done || serverState.State != psDumpEEPROM) {
						if (err == PROGRAM_ERROR_SUCCESS)
							fs_send(FS_OK, 0);
						else report_error(err);
					
						if (GlobalSettings.NewLineAfter)
							comm_send_newline();				
					}
					
					server_reset();					
				} else {
					err = server_put_char(b);
					if (GlobalSettings.CharacterEcho) {
						if (err == SERVER_ERROR_SUCCESS)
							comm_send_buffer(&b, sizeof(b));
					}
				}
				
				comm_send_end();				
				comm_set_communication_method(cmNone);
			}
		}
		
		usart0_stop();
	}
	
	twi_slave_finit();
	
	return 0;
}
