/*
 * eeprom.c
 *
 * Created: 22. 6. 2015 17:50:28
 *  Author: Martin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "program.h"
#include "eeprom.h"



#define EEPROM_BUFFER_SIZE             33

static unsigned char _eepromBuffer[EEPROM_BUFFER_SIZE];
static uint16_t _eepromAddressBuffer[EEPROM_BUFFER_SIZE];
static volatile uint8_t _eepromFreeIndex = 0;
static volatile uint8_t _eepromLastIndex = 0;
static volatile uint8_t _eeprom_is_writting = 0;
static volatile uint8_t _eeprom_intelligent_write = 0;


#define _eeprom_buffer_full() \
	(_eepromFreeIndex + 1 == _eepromLastIndex || (_eepromLastIndex == 0 && _eepromFreeIndex == EEPROM_BUFFER_SIZE -1))

#define _eeprom_buffer_empty() \
	(_eepromFreeIndex == _eepromLastIndex)


#define _eeprom_read_byte_raw(aAddress, aResult)	\
	{												\
		EEARL = (aAddress & 0xff);					\
		EEARH = (aAddress >> 8);					\
		EECR |= (1 << EERE);						\
		aResult = EEDR;								\
	}												\

#define _eeprom_write_byte_raw(aAddress, aValue)	\
		{											\
			EEARL = (aAddress & 0xff);				\
			EEARH = (aAddress >> 8);				\
			EEDR = aValue;					\
			EECR |= (1 << EEMWE);					\
			EECR |= (1 << EEWE);					\
		}											\


ISR(EE_READY_vect)
{
	cli();
	if (!_eeprom_buffer_empty()) {
		uint16_t addr = _eepromAddressBuffer[_eepromLastIndex];
		uint8_t valueToWrite = _eepromBuffer[_eepromLastIndex];
		
		++_eepromLastIndex;
		if (_eepromLastIndex == EEPROM_BUFFER_SIZE)
			_eepromLastIndex = 0;
		
		if (_eeprom_intelligent_write) {
			uint8_t storedValue;
			_eeprom_read_byte_raw(addr, storedValue);
			if (storedValue != valueToWrite)
				_eeprom_write_byte_raw(addr, valueToWrite);
		} else _eeprom_write_byte_raw(addr, valueToWrite);
	} else {
		EECR &= ~(1 << EERIE);
		_eeprom_is_writting = 0;
	}
	
	sei();
	
	return;
}



uint8_t myeeprom_read_byte(const uint16_t Address)
{
	uint8_t ret;
	
	myeeprom_write_wait();
	cli();
	_eeprom_read_byte_raw(Address, ret);
	sei();
	
	return ret;
}

uint16_t myeeprom_read_word(const uint16_t Address)
{
	uint16_t l = myeeprom_read_byte(Address);
	uint16_t h = myeeprom_read_byte(Address + 1);
	
	return (l + (h << 8));	
}

void myeeprom_read_buffer(uint16_t Address, void *Buffer, uint16_t Count)
{
	uint8_t *b = (uint8_t *)Buffer;
	
	while (Count > 0) {
		*b = myeeprom_read_byte(Address);
		++b;
		++Address;
		--Count;
	}
	
	return;
}

void myeeprom_write_buffer(uint16_t Address, void *Buffer, uint16_t Size)
{
	if (Size > 0) {
		uint8_t *b = (uint8_t *)Buffer;
		cli();
		while (Size > 0) {
			while (_eeprom_buffer_full()) {
				sleep_enable();
				sei();
				sleep_cpu();
				sleep_disable();
				cli();
			}
			
			_eepromBuffer[_eepromFreeIndex] = *b;
			_eepromAddressBuffer[_eepromFreeIndex] = Address;
			++_eepromFreeIndex;
			if (_eepromFreeIndex == EEPROM_BUFFER_SIZE)
				_eepromFreeIndex = 0;
				
			--Size;
			++b;
			++Address;
			if (_eepromFreeIndex == _eepromLastIndex)
				break;
		}
		
		_eeprom_is_writting = 1;
		EECR |= (1 << EERIE);
		sei();
	}
	
	return;
}

void myeeprom_write_byte(uint16_t Address, uint8_t Value)
{
	myeeprom_write_buffer(Address, &Value, sizeof(Value));
	
	return;
}

void myeeprom_write_word(uint16_t Address, uint16_t Value)
{
	myeeprom_write_buffer(Address, &Value, sizeof(Value));
	
	return;
}

uint8_t myeeprom_is_writting(void)
{
	return _eeprom_is_writting;
}

void myeeprom_write_wait(void)
{
	cli();
	while (_eeprom_is_writting) {
		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
		cli();
	}
	
	return;
}

void myeeprom_set_inteligent_write(const uint8_t Value)
{
	_eeprom_intelligent_write = Value;
	
	return;
}
