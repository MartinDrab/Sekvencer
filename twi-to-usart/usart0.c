/*
 * usart0.c
 *
 * Created: 20. 6. 2015 23:24:40
 *  Author: Martin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "usart0.h"



#define USART_BUFFER_SIZE               33

static volatile uint8_t _sendBuffer[USART_BUFFER_SIZE];
static volatile uint8_t _sendFreeIndex = 0;
static volatile uint8_t _sendLastIndex = 0;

static volatile uint8_t _recvBuffer[USART_BUFFER_SIZE];
static volatile uint8_t _recvBufferStart = 0;
static volatile uint8_t _recvBufferEnd = 0;



#define _usart0_send_buffer_empty() \
	(_sendFreeIndex == _sendLastIndex)

#define _usart0_send_buffer_full() \
	(_sendFreeIndex + 1 == _sendLastIndex || (_sendLastIndex == 0 && _sendFreeIndex == USART_BUFFER_SIZE - 1))

#define _usart0_recv_buffer_empty() \
	(_recvBufferStart == _recvBufferEnd)

#define _usart0_recv_buffer_full() \
	(_recvBufferEnd + 1 == _recvBufferStart || (_recvBufferStart == 0 && _recvBufferEnd == USART_BUFFER_SIZE - 1))


ISR(USART0_RX_vect)
{
	cli();
	if (!_usart0_recv_buffer_full()) {
		_recvBuffer[_recvBufferEnd] = UDR0;
		++_recvBufferEnd;
		if (_recvBufferEnd == USART_BUFFER_SIZE)
			_recvBufferEnd = 0;
	}
	sei();
	
	return;
}

ISR(USART0_TX_vect)
{
	return;
}

ISR(USART0_UDRE_vect)
{
	cli();
	if (!_usart0_send_buffer_empty()) {
		uint8_t byteToSend = _sendBuffer[_sendLastIndex];
		++_sendLastIndex;
		if (_sendLastIndex == USART_BUFFER_SIZE)
			_sendLastIndex = 0;
	
		UDR0 = byteToSend;
	} else {
		UCSR0B &= ~(1 << UDRIE0);
		UCSR0A &= ~(1 << UDRE0);
	}
		
	sei();
			
	return;
}


uint8_t usart0_init(uint32_t BaudRate, uint8_t Stop, uint8_t Parity)
{
	uint8_t ret = USART_ERROR_SUCCESS;
	uint16_t b = 0;
	
	switch (BaudRate) {
		case 4800:
			b = 207;
			break;
		case 9600:
			b = 103;
			break;
		case 19200:
			b = 51;
			break;
		case 38400:
			b = 25;
			break;
		default:
			ret = USART_ERROR_INVALID_BAUD;
			break;
	}
	
	if (ret == USART_ERROR_SUCCESS) {
		if (Parity < 4) {
			if (Stop < 3) {
				uint8_t stop = ((Stop - 1) << USBS0);
				uint8_t par = (Parity << UPM00);
				uint8_t data = (3 << UCSZ00);
				
				UBRR0L = (b & 0xff);
				UBRR0H = ((b >> 8) & 0xf);
				UCSR0C = (par | stop | data);				
			} else ret = USART_ERROR_INVALID_STOP;
		} else ret = USART_ERROR_INVALID_PARITY;
	}
	
	return ret;
}

void usart0_start_receive(void)
{
	UCSR0B |= ((1 << RXCIE0) | (1 << RXEN0));
	
	return;	
}

void usart0_start_send(void)
{
	UCSR0B |= ((1 << TXCIE0) | (1 << TXEN0));
	
	return;
}

void usart0_send_buffer(uint8_t *Buffer, uint8_t Length)
{
	if (Length > 0) {
		cli();
		uint8_t longerThanInternalSize = (Length >= USART_BUFFER_SIZE);
		
		do {
			while (_usart0_send_buffer_full()) {
				sleep_enable();
				if (longerThanInternalSize) {
					UCSR0B |= (1 << UDRIE0);		
					longerThanInternalSize = 0;
				}
				
				sei();
				sleep_cpu();
				sleep_disable();
				cli();
			}
			
			_sendBuffer[_sendFreeIndex] = *Buffer;
			++Buffer;
			--Length;
			++_sendFreeIndex;
			if (_sendFreeIndex == USART_BUFFER_SIZE)
				_sendFreeIndex = 0;
			
			if (_sendFreeIndex == _sendLastIndex)
				break;
		} while (Length > 0);
		
		UCSR0B |= (1 << UDRIE0);
		sei();
	}
		
	return;
}

void usart0_send_wait(void)
{
	cli();
	while (!_usart0_send_buffer_empty()) {
		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
		cli();
	}
	
	sei();
	
	return;
}

void usart0_stop(void)
{	
	cli();
	UCSR0B &= ~((1 << UDRIE0) | (1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0));
	while (UCSR0A & (1 << RXC0))
		UDR0;

	UCSR0A = 0;
	_recvBufferStart = 0;
	_recvBufferEnd = 0;
	_sendFreeIndex = 0;
	_sendLastIndex = 0;
	sei();
	
	return;
}

uint8_t usart0_get_byte(uint8_t *Byte)
{
	uint8_t ret;
	
	cli();
	ret = !_usart0_recv_buffer_empty();
	if (ret) {
		*Byte = _recvBuffer[_recvBufferStart];
		++_recvBufferStart;
		if (_recvBufferStart == USART_BUFFER_SIZE)
			_recvBufferStart = 0;	
	}
	
	sei();
	
	return ret;
}

uint8_t usart0_byte_received(void)
{
	return !_usart0_recv_buffer_empty();
}