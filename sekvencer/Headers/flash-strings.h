/*
 * flash_strings.h
 *
 * Created: 23. 6. 2015 21:28:51
 *  Author: Martin
 */ 


#ifndef FLASH_STRINGS_H_
#define FLASH_STRINGS_H_



#define FS_COPYRIGHT                   0
#define FS_HELP_START                  1
#define FS_HELP_END                    31
#define FS_PROGRAM_CREATED             31
#define FS_ACTIVE_PROGRAM              32
#define FS_SELECTED_PROGRAM            33
#define FS_PROGRAM_COUNT               34
#define FS_PROGRAM_MAX                 35
#define FS_PROGRAM_LISTING             36
#define FS_PROGRAM_INST_COUNT          37
#define FS_NONE                        38
#define FS_PROGRAM_INST_LISTING        39
#define FS_PROGRAM_SLOT_LISTING        40
#define FS_GS_USART0_BAUDRATE          41
#define FS_GS_USART0_STOP_BITS         42
#define FS_GS_USART0_PARITY            43
#define FS_GS_TWI_SLAVE_ADDRESS        44
#define FS_GS_CHARACTER_ECHO           45
#define FS_NEWLINE_AFTER               46
#define FS_NEWLINE_BEFORE              47
#define FS_OK                          48
#define FS_ERR                         49
#define FS_TRUE                        50
#define FS_FALSE                       51
#define FS_REPORT_ERROR_MESSAGES       52
#define FS_DISABLE_EEPROM_RESET        53
#define FS_DISABLE_EEPROM_INIT         54
#define FS_DISABLE_EEPROM_DUMP         55

void fs_send(const uint16_t Index, const uint8_t NewLine);
void fs_send_error_message(const uint8_t ErrorCode, const uint8_t NewLine);


#endif /* FLASH-STRINGS_H_ */