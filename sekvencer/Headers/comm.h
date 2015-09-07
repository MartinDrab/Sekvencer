/*
 * comm.h
 *
 * Created: 24. 6. 2015 17:04:21
 *  Author: Martin
 */ 


#ifndef COMM_H_
#define COMM_H_


typedef enum {
	cmNone,
	cmUSART0,
	cmTWI,
} ECommunicationMethod;


void comm_send_buffer(uint8_t *Buffer, const uint16_t Size);
void comm_send_byte(uint8_t Value);
void comm_send_word(uint16_t Value);
void comm_send_dword(uint32_t Value);
void comm_send_number8(uint8_t Value);
void comm_send_number16(uint16_t Value);
void comm_send_number32(uint32_t Value);
void comm_send_dec_number8(uint8_t Digits, uint8_t Value);
void comm_send_dec_number16(uint8_t Digits, uint16_t Value);
void comm_send_dec_number32(uint8_t Digits, uint32_t Value);
void comm_send_n_bytes(uint8_t N, uint8_t Byte);
void comm_send_time(uint32_t Time);
void comm_send_end(void);

void comm_send_string(char *String);
uint8_t comm_byte_received(void);
#define comm_send_newline()		\
	{							\
		comm_send_byte('\r');	\
		comm_send_byte('\n');	\
	}							\

#define comm_send_boolean(aValue)	\
	{                               \
		if ((aValue))				\
			fs_send(FS_TRUE, 0);	\
		else fs_send(FS_FALSE, 0);	\
	}								\

uint8_t comm_get_byte(uint8_t *Byte);

void comm_set_communication_method(ECommunicationMethod Method);



#endif /* COMM_H_ */
