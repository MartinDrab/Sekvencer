/*
 * usart.h
 *
 * Created: 20. 6. 2015 23:25:20
 *  Author: Martin
 */ 


#ifndef USART_H_
#define USART_H_


#define USART_ERROR_SUCCESS	           0
#define USART_ERROR_INVALID_PARITY     1
#define USART_ERROR_INVALID_STOP       2
#define USART_ERROR_INVALID_BAUD       3


uint8_t usart0_init(uint32_t BaudRate, uint8_t Stop, uint8_t Parity);

void usart0_start_receive(void);
uint8_t usart0_get_byte(uint8_t *Byte);
uint8_t usart0_byte_received(void);
void usart0_start_send(void);
void usart0_send_buffer(uint8_t *Buffer, uint8_t Length);
void usart0_send_wait(void);
void usart0_stop(void);


#endif /* USART_H_ */