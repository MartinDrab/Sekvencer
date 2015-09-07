/*
 * twi_slave.c
 *
 * Created: 24. 6. 2015 16:16:31
 *  Author: Martin
 */ 


#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include <avr/sleep.h>
#include "twi-slave.h"



#define TWI_BUFFER_SIZE					33

static volatile uint8_t _twiSendBuffer[TWI_BUFFER_SIZE];
static volatile uint8_t _twiFreeIndex = 0;
static volatile uint8_t _twiLastIndex = 0;

static volatile uint8_t _twiReceiveBuffer[TWI_BUFFER_SIZE];
static volatile uint8_t _twiReceivedIntervalStart = 0;
static volatile uint8_t _twiReceivedIntervalEnd = 0;

static volatile ETWISlaveMode _twiMode = twismNotAddressed;



#define twi_send_buffer_empty() \
	(_twiFreeIndex == _twiLastIndex)

#define twi_send_buffer_full()	\
	(_twiFreeIndex + 1 == _twiLastIndex || (_twiLastIndex == 0 && _twiFreeIndex + 1 == TWI_BUFFER_SIZE))

#define twi_recv_buffer_full()	\
	(_twiReceivedIntervalEnd + 1 == _twiReceivedIntervalStart || (_twiReceivedIntervalStart == 0 && _twiReceivedIntervalEnd + 1 == TWI_BUFFER_SIZE))

#define twi_recv_buffer_empty() \
	(_twiReceivedIntervalEnd == _twiReceivedIntervalStart)




ISR(TWI_vect)
{
	cli();
	uint8_t twiStatus = TWSR & 0xF8;
	uint8_t controlNext = (1 << TWINT) | (1 << TWIE) | (1 << TWEN);
	
	switch (twiStatus) {
		case 0x60:	// SLA+W seen + ACK
			controlNext |= (1 << TWEA);
			_twiMode = twismReceiver;
			// Empty the send buffer. Someone wishes to send a command to us which
			// means she knows everything about the result of the last command she needs
			// to.
			_twiFreeIndex = 0;
			_twiLastIndex = 0;
			break;
		case 0x80: // data byte received, ACKed
		case 0x88: // data byte received, NACKed
			// If the receive buffer is full, just drop the byte. Size of the receive
			// buffer is documented so it is master's fault not to cope with it.
			if (!twi_recv_buffer_full()) {
				_twiReceiveBuffer[_twiReceivedIntervalEnd] = TWDR;
				++_twiReceivedIntervalEnd;
				if (_twiReceivedIntervalEnd == TWI_BUFFER_SIZE)
					_twiReceivedIntervalEnd = 0;
			}

			controlNext |= (1 << TWEA);
			break;
		case 0xA0: // STOP or a repeated START condition
			// If there are data to transmit, connect the device to the bus.
			if (_twiMode == twismReceiver && !twi_send_buffer_empty())
				controlNext |= (1 << TWEA);
				
			_twiMode = twismNotAddressed;
			break;
		case 0xA8: // SLA+R received, ACK sent
			_twiMode = twismTransmitter;
		case 0xB8: // Data transmitted, ACK sent
			if (!twi_send_buffer_empty()) {
				TWDR = _twiSendBuffer[_twiLastIndex];
				++_twiLastIndex;
				if (_twiLastIndex == TWI_BUFFER_SIZE)
					_twiLastIndex = 0;					
			} else TWDR = TWI_SLAVE_NO_DATA;
					
			controlNext |= (1 << TWEA);
			break;
		case 0xC0:
		case 0xC8: // last byte sent, done
			controlNext |= (1 << TWEA);	
			_twiMode = twismNotAddressed;
			break;
		default:
			controlNext |= (1 << TWEA);
			break;
	}
	
	TWCR = controlNext;
	sei();
		
	return;
}




void twi_slave_init(const uint8_t Address)
{
	cli();
	_twiReceivedIntervalStart = 0;
	_twiReceivedIntervalEnd = 0;
	TWAR = (Address << 1);
	TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
	sei();
	
	
	return;
}

void twi_slave_send_buffer(uint8_t *Buffer, uint16_t Size)
{
	if (Size > 0) {
		cli();
		uint8_t longerThanBuffer = (Size > TWI_BUFFER_SIZE - 1);
		do {
			while (twi_send_buffer_full()) {
				sleep_enable();
				if (longerThanBuffer) {
					TWCR |= (1 << TWEA);					
					longerThanBuffer = 0;
				}
				sei();
				sleep_cpu();
				sleep_disable();
				cli();
			}
			
			_twiSendBuffer[_twiFreeIndex] = *Buffer;
			_twiFreeIndex++;
			if (_twiFreeIndex == TWI_BUFFER_SIZE)
				_twiFreeIndex = 0;
			
			Buffer++;
			Size--;
		} while (Size > 0);
		TWCR |= (1 << TWEA);
		sei();
	}

	return;
}

uint8_t twi_slave_get_byte(uint8_t *Byte)
{
	uint8_t ret;
	
	cli();
	ret = !twi_recv_buffer_empty();
	if (ret) {
		*Byte = _twiReceiveBuffer[_twiReceivedIntervalStart];
		++_twiReceivedIntervalStart;
		if (_twiReceivedIntervalStart == TWI_BUFFER_SIZE)
			_twiReceivedIntervalStart = 0;
	}
	
	sei();
	
	return ret;
}

uint8_t twi_slave_byte_received(void)
{
	uint8_t ret;
	
	cli();
	ret = !twi_recv_buffer_empty();
	sei();
	
	return ret;
}

void twi_slave_start_listening(void)
{
	cli();
	TWCR |= (1 << TWEA);
	sei();
	
	return;
}

void twi_slave_finit(void)
{
	cli();
	TWCR = 0;
	_twiReceivedIntervalStart = 0;
	_twiReceivedIntervalEnd = 0;
	sei();
	
	return;
}
