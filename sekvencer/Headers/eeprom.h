/*
 * eeprom.h
 *
 * Created: 22. 6. 2015 17:50:13
 *  Author: Martin
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_


uint8_t myeeprom_read_byte(const uint16_t Address);
uint16_t myeeprom_read_word(const uint16_t Address);
void myeeprom_read_buffer(uint16_t Address, void *Buffer, uint16_t Count);
void myeeprom_write_buffer(uint16_t Address, void *Buffer, uint16_t Size);
void myeeprom_write_byte(uint16_t Address, uint8_t Value);
void myeeprom_write_word(uint16_t Address, uint16_t Value);
uint8_t myeeprom_is_writting(void);
void myeeprom_write_wait(void);
void myeeprom_set_inteligent_write(const uint8_t Value);



#endif /* EEPROM_H_ */