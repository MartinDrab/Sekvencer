/*
 * twi_to_usart.c
 *
 * Created: 19. 7. 2015 21:20:10
 *  Author: Martin
 */ 


#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart0.h"
#include "i2c_master.h"

// Addres of the slave device
#define TWI_SLAVE_ADDRESS			(21)


int main(void)
{
	// Initialize the USART0 interface. 9600 baudrate, no parity, 1 stop bit
	if (usart0_init(9600, 1, 0) == 0) {
		// Enable USART0 transmitter and receiver
		usart0_start_send();
		usart0_start_receive();
		// Turn on pull-up resistors for SDA and SCL pins (needed to run the TWI interface)
		DDRD &= ~3;
		PORTD &= 3;
		PORTD |= 3;
		// And initialize the TWI
		i2c_init();
		while (1) {
			cli();
			// Wait for an input on USART0
			while (!usart0_byte_received()) {
				sleep_enable();
				sei();
				sleep_cpu();
				cli();
				sleep_disable();
			}
			
			sei();
			if (usart0_byte_received()) {
				uint8_t b = 0;
				
				usart0_get_byte(&b);
				// SLA+W for the slave device
				if (i2c_start((TWI_SLAVE_ADDRESS << 1) + I2C_WRITE) == 0) {
					// Write the byte received over USART0 to the device.
					if (i2c_write(b) == 0) {
						i2c_stop();
						// Wait until the device reappears with some output
						while (i2c_start((TWI_SLAVE_ADDRESS << 1) + I2C_READ) != 0)
							i2c_stop();
						
						// Read the output
						while (1) {
							b = i2c_read_ack();					
							if (b == 0xff) {
								// This is the end of the output.
								// Read one byte to empty the transmit buffer of the device
								// and exit
								i2c_read_nack();	
								break;
							}
							
							if (b == 0xfe)
								// The device currently has no data for us – but it will
								// have later.
								continue;
						
							// Send the received byte to the entity listening on the USART0
							usart0_send_buffer(&b, sizeof(b));
						}
					}
					
					i2c_stop();
				}
			}
		}
	}
	
	return 0;
}
