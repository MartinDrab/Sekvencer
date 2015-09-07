/*
 * comm.c
 *
 * Created: 24. 6. 2015 17:04:00
 *  Author: Martin
 */ 

#include <avr/io.h>
#include <string.h>
#include "usart0.h"
#include "twi-slave.h"
#include "comm.h"


static volatile ECommunicationMethod _method = cmNone;


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

/************************************************************************/
/*                           PUBLIC FUNCTIONS                           */
/************************************************************************/

void comm_send_buffer(uint8_t *Buffer, const uint16_t Size)
{
	switch (_method) {
		case cmNone: break;
		case cmUSART0: usart0_send_buffer(Buffer,Size); break;
		case cmTWI: twi_slave_send_buffer(Buffer, Size); break;
	}
	
	return;
}

void comm_send_byte(uint8_t Value)
{
	comm_send_buffer(&Value, sizeof(Value));
	
	return;	
}

void comm_send_word(uint16_t Value)
{
	comm_send_buffer((uint8_t *)&Value, sizeof(Value));

	return;	
}

void comm_send_dword(uint32_t Value)
{
	comm_send_buffer((uint8_t *)&Value, sizeof(Value));

	return;	
}

void comm_send_number8(uint8_t Value)
{
	uint8_t buf[4];
	
	buf[0] = '0';
	buf[1] = 'x';
	for (uint8_t i = 0; i < 2; ++i) {
		buf[sizeof(buf) / sizeof(char) - i - 1] = _digit_to_char(16, Value & 0xf);
		Value >>= 4;
	}

	comm_send_buffer(buf, sizeof(buf));
	
	return;
}

void comm_send_number16(uint16_t Value)
{
	uint8_t buf[6];
	
	buf[0] = '0';
	buf[1] = 'x';
	for (uint8_t i = 0; i < 4; ++i) {
		buf[sizeof(buf) / sizeof(char) - i - 1] = _digit_to_char(16, Value & 0xf);
		Value >>= 4;
	}

	comm_send_buffer(buf, sizeof(buf));
	
	return;
}

void comm_send_number32(uint32_t Value)
{
	uint8_t buf[10];
	
	buf[0] = '0';
	buf[1] = 'x';
	for (uint8_t i = 0; i < 8; ++i) {
		buf[sizeof(buf) / sizeof(char) - i - 1] = _digit_to_char(16, Value & 0xf);
		Value >>= 4;
	}

	comm_send_buffer(buf, sizeof(buf));
	
	return;
}

void comm_send_n_bytes(uint8_t N, uint8_t Byte)
{
	for (uint8_t i = 0; i < N; ++i)
		comm_send_byte(Byte);
	
	return;
}

void comm_send_dec_number8(uint8_t Digits, uint8_t Value)
{
	uint8_t i = 0;
	uint8_t buf[3];

	memset(buf, '0', sizeof(buf));
	if (Value > 0) {
		while (Value > 0) {
			buf[sizeof(buf) - i - 1] = _digit_to_char(10, Value % 10);
			++i;
			Value /= 10;
		}
	} else ++i;

	if (Digits > 0)
		comm_send_n_bytes(Digits - i, '0');

	comm_send_buffer(buf + (sizeof(buf) - i), i);
	
	return;
}

void comm_send_dec_number16(uint8_t Digits, uint16_t Value)
{
	uint8_t i = 0;
	uint8_t buf[5];
	
	memset(buf, '0', sizeof(buf));
	if (Value > 0) {
		while (Value > 0) {
			buf[sizeof(buf) - i - 1] = _digit_to_char(10, Value % 10);
			++i;
			Value /= 10;
		}
	} else ++i;

	if (Digits > 0)
		comm_send_n_bytes(Digits - i, '0');
	
	comm_send_buffer(buf + (sizeof(buf) - i), i);
	
	return;
}

void comm_send_dec_number32(uint8_t Digits, uint32_t Value)
{
	uint8_t i = 0;
	uint8_t buf[10];
	
	memset(buf, '0', sizeof(buf));	
	if (Value > 0) {
		while (Value > 0) {
			buf[sizeof(buf) - i - 1] = _digit_to_char(10, Value % 10);
			++i;
			Value /= 10;
		}
	} else ++i;

	if (Digits > 0)
		comm_send_n_bytes(Digits - i, '0');

	comm_send_buffer(buf + (sizeof(buf) - i), i);
	
	return;
}

void comm_send_string(char *String)
{
	uint8_t len = 0;
	
	while (String[len] != '\0')
		++len;
	
	comm_send_buffer((uint8_t *)String, len);
	
	return;
}

uint8_t comm_byte_received(void)
{
	uint8_t ret = 0;
	
	switch (_method) {
		case cmNone:
			ret = usart0_byte_received();
			if (ret)
				_method = cmUSART0;
			else {
				ret = twi_slave_byte_received();
				if (ret)
					_method = cmTWI;
			} 
			break;
		case cmUSART0: ret = usart0_byte_received(); break;
		case cmTWI: ret = twi_slave_byte_received(); break;
	}
	
	return ret;
}


uint8_t comm_get_byte(uint8_t *Byte)
{
	uint8_t ret = 0;
	
	switch (_method) {
		case cmNone: break;
		case cmUSART0: ret = usart0_get_byte(Byte); break;
		case cmTWI: ret = twi_slave_get_byte(Byte); break;
	}
	
	return ret;
}

void comm_send_end(void)
{
	if (_method == cmTWI) {
		uint16_t w = TWI_SLAVE_DATA_END;
		
		twi_slave_send_buffer((uint8_t *)&w, sizeof(w));
	}
	
	return;
}

void comm_set_communication_method(ECommunicationMethod Method)
{
	_method = Method;
	
	return;
}

void comm_send_time(uint32_t Time)
{
	const uint16_t ms = Time % 1000;
	Time /= 1000;
	const uint8_t s = Time % 60;
	Time /= 60;
	const uint8_t min = Time % 60;
	Time /= 60;
	const uint8_t h = Time;
	
	comm_send_dec_number8(2, h);
	comm_send_byte(':');
	comm_send_dec_number8(2, min);
	comm_send_byte(':');
	comm_send_dec_number8(2, s);
	comm_send_byte(':');
	comm_send_dec_number16(3, ms);
	
	
	return;
}
