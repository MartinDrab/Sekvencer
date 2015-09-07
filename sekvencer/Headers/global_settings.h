/*
 * global_settings.h
 *
 * Created: 24. 6. 2015 23:43:42
 *  Author: Martin
 */ 


#ifndef GLOBAL_SETTINGS_H_
#define GLOBAL_SETTINGS_H_

typedef struct {
	uint16_t USART0BaudRate;
	uint8_t USART0StopBits;
	uint8_t USART0Parity;
	uint8_t TWISlaveAddress;
	uint8_t CharacterEcho;
	uint8_t NewLineAfter;
	uint8_t NewLineBefore;
	uint8_t PrintErrorMessages;
	uint8_t DisableEEPROMReset;
	uint8_t DisableEEPROMInit;
	uint8_t DisableEEPROMDump;
} GLOBAL_SETTINGS_SRAM;

typedef struct {
	uint8_t Signature[4];
	GLOBAL_SETTINGS_SRAM Settings;
} STORED_GLOBAL_SETTINGS;




extern GLOBAL_SETTINGS_SRAM GlobalSettings;


uint8_t global_settings_load(void);
void global_settings_save(void);
void global_settings_init(void);
void global_settings_print(void);
void global_settings_print(void);
uint8_t global_settings_change(uint8_t Field, uint16_t Value);


#endif /* GLOBAL_SETTINGS_H_ */
