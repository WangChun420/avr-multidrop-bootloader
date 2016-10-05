/*****************************************************************************
*
* Configuration settings for the bootloader
*
****************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG

////////////////////////////////////////////
/// EEPROM Locations
////////////////////////////////////////////

// RUN_APP setting
#define EEPROM_RUN_APP (uint8_t*)0

// Major & Minor version number locations
#define EEPROM_VER_MAJ (uint8_t*)1
#define EEPROM_VER_MIN (uint8_t*)2

////////////////////////////////////////////
/// Pins
////////////////////////////////////////////

// Signal Line Connection
#define SIGNAL_PIN PD7
#define SIGNAL_DDR_REG DDRD
#define SIGNAL_PORT_REG PIND

////////////////////////////////////////////
/// Serial Communication
////////////////////////////////////////////

#define SERIAL_BAUD 115200

// Convert baud into the register value
#define UART_BAUD_SELECT(baudRate)  (((F_CPU) + 8UL * (baudRate)) / (16UL * (baudRate)) -1UL)

// Enable the serial lines
inline void serialSetup() {
  // PORTD |= (1 << PD0); // Enable pull-up on RX pin

  UCSR0B = (1<<RXEN0); // Enable RX
  UCSR0C = 1<<UCSZ01 | 1<<UCSZ00; // Frame format (8-bit, 1 stop bit)
  UBRR0 =  (unsigned char)UART_BAUD_SELECT(SERIAL_BAUD); // Set baud

  // DE pin LOW for receive
  DDRD |= (1 << PD2);
  PORTD &= ~(1 << PD2);

#ifdef DEBUG
  UCSR0B |= (1<<TXEN0); // Enable TX
#endif
}

// Receive the next byte of data
inline uint8_t serialReceive() {
  while (!(UCSR0A & (1<<RXC0))); // wait for data
  return UDR0;
}

#ifdef DEBUG
inline void serialWrite(uint8_t byte) {
	while (!(UCSR0A & (1<<UDRE0)));	// Wait for empty transmit buffer
	UDR0 = byte;	// send byte
}
#endif

#endif