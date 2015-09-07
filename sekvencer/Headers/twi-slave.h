/*
 * twi_slave.h
 *
 * Created: 24. 6. 2015 16:16:10
 *  Author: Martin
 */ 


#ifndef TWI_SLAVE_H_
#define TWI_SLAVE_H_

/** This byte is sent over TWI if there are no data to be sent. */
#define TWI_SLAVE_NO_DATA					0xfe

/** This sequence means that the data produced by the last command ends,
    so it is pointless to make further reads. A better option is to write data
	to the slave and issue a new command. */
#define TWI_SLAVE_DATA_END					0xffff

typedef enum _ETWISlaveMode {
	twismNotAddressed = 0,
	twismTransmitter = 1,
	twismReceiver = 2,
} ETWISlaveMode;



void twi_slave_init(const uint8_t Address);
void twi_slave_send_buffer(uint8_t *Buffer, uint16_t Size);
uint8_t twi_slave_get_byte(uint8_t *Byte);
uint8_t twi_slave_byte_received(void);
void twi_slave_finit(void);
void twi_slave_start_listening(void);



#endif /* TWI-SLAVE_H_ */
