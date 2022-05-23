// Definitions.cpp
/* In this file the definitions for the used functions are included. The functions usart_init, sendbyte, sendstr, receiveByte and receiveBluetooth
were taken from EE-Diary (https://www.ee-diary.ga/2021/06/hc-05-bluetooth-module-and-atmega328p.html), with some minor modifications. 
The functions requestDHT11, responseDHT11, and receiveDataDHT11 were taken from ElectronicWings 
(https://www.electronicwings.com/avr-atmega/dht11-sensor-interfacing-with-atmega16-32), with some minor modifications, especially regarding 
the pulse duration and the timeout mechanisms. */


#include "Definitions.h"
#include <avr/io.h>
#include <util/delay.h>

#define USART_BAUDRATE 9600

unsigned char onStatus [] = "on.\n";
unsigned char offStatus [] = "off.\n";
unsigned char startMsg [] = "\n\nBluetooth hygrometer control. Send 'y' to turn on the water pump.\n";
unsigned char tempMsg [] = "The temperature is: ";
unsigned char dotMsg [] = ".";
unsigned char degMsg [] = "°C.\n";
unsigned char statusPump [] = "The water pump is ";
unsigned char statusLamp [] = "The UV lamp is ";

unsigned char rxdata;

bool bluetooth = false;                               // This flag is set when the pump is activated through bluetooth.

uint8_t DHT11Data [ 5 ];                              // Variables used for the DHT11 sensor. They are intHum, decHum, intTemp, decTemp and CheckSum.

char buffer [ 10 ];

void usart_init ( void )
{
	UBRR0H = ( unsigned char ) ( UBRRVAL >> 8 ) ;     // High byte.
	UBRR0L = ( unsigned char ) UBRRVAL;     		  // Low byte.
	UCSR0B |=  ( 1 << TXEN0 )  |  ( 1 << RXEN0 ) ;	  // Enable Transmitter and Receiver.
	UCSR0C |=  ( 1 << UCSZ01 ) | ( 1 << UCSZ00 ) ; 	  // Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size.
	UCSR0B |= ( 1 << RXCIE0 );                        // Enable the USART Receive interrupt.
} 

void sendbyte ( unsigned char MSG )
{
	while ( ( UCSR0A & ( 1 << UDRE0 ) ) == 0 ) ;   // Wait if a byte is being transmitted
	UDR0 = MSG;
}

void sendstr ( unsigned char * s )
{
	unsigned char i = 0;
	while ( s [ i ]  != '\0' )
	{
		sendbyte ( s [ i ] );
		i ++;
	}
	return;
}

unsigned char receiveByte ( void )
{
	uint16_t i = 0;
	while ( ( ! ( UCSR0A & ( 1 << RXC0 ) ) ) && i < 1000 )
	{
		i ++;
		if ( i == 1000 )
		{
			return '~';
		}
	}
	return UDR0;
}

void receiveBluetooth ()
{
	rxdata = receiveByte ();
	if ( rxdata == 'y' )
	{
		bluetooth = true;              // The bluetooth flag is true, so the water pump will be set.
	}
	else if ( rxdata == 'n' )
	{
		bluetooth = false;             // The bluetooth flag is false, so the water pump will not be set.
	}
	else
	{
		;
	}
	return;
}

void sendBluetoothData ( unsigned char * HumString, unsigned char * TempString, unsigned char * DateString )
{
	sendstr ( startMsg );
	sendstr ( statusLamp );
	if ( false )
	{
		sendstr ( onStatus );
	}
	else
	{
		sendstr ( offStatus );
	}
	sendstr ( HumString );
	sendstr ( TempString );
	sendstr ( DateString );
	return;
}

void setup () 
{
	DDRC = 0x00;                       // Port C is declared as input.
	DDRB = 0xFF;                       // Port B is declared as output.
	PORTB = 0x03;                      // Port B is set to output 0x03.
	usart_init();                      // Initialize the USART.

	twi_INIT ();                       // Initialize the TWI communication protocol.
	//DS3231_setTime ( 20, 8, 10, 4, 12, 5, 22 ); // Set the date and time.
}

void setWaterPump ()
{
	PORTB &= ~ ( 1 << PORTB0 );          // Set PB0 high, i.e. the water pump.
	_delay_ms ( 5000 );                // The water pump will be on for 5 seconds.
	PORTB |= ( 1 << PORTB0 );        // Turn off the water pump.
	bluetooth = false;
	return;
}

void updateLamp ( uint8_t hour )
{
	if ( ( hour >= 18 ) || hour < 8 )
	{
		PORTB &= ~ ( 1 << PORTB1 );    // Set PB1 low, i.e. set the lamp.
	}
	else
	{
		PORTB |= ( 1 << PORTB1 );      // Set PB1 high, i.e. deactivate the lamp.
	}
	return;
}

void requestDHT11 ()				   // Send start pulse/request to the microcontroller.
{
	DDRC |= ( 1 << PC0 );
	PORTC &= ~ ( 1 << PC0 );	       // Set pin PC0 low.
	_delay_ms ( 25 );			       // Wait for 20 ms.
	PORTC |= ( 1 << PC0 );	           // Set pin PC0 high.
	_delay_us ( 30 );			       // Wait for 20 ms.
	DDRC &= ~ ( 1 << PC0 );
	return;
}

void responseDHT11 ( bool & flag )	   // Receive response from DHT11
{
	uint16_t j = 0;
	while ( PINC & ( 1 << PC0 ) )
	{
		j ++;
		if ( j > 1000 )
		{
			flag = true;
			return;
		}
	}
	while ( ( PINC & ( 1 << PC0 ) ) == 0 )
	{
		;
	}
	while ( PINC & ( 1 << PC0 ) )
	{
		;
	}
	flag = false;
	return;
}

uint8_t receiveDataDHT11 ()			                  // Receive data from the DHT11 sensor.
{
	DHT11Data [ 4 ] = 0;
	for ( uint8_t q = 0; q < 8; q ++ )
	{
		while ( ( PINC & ( 1 << PC0 ) ) == 0 )        // Check if received bit 0 or 1.
		{
			;
		}
		_delay_us ( 30 );
		if ( PINC & ( 1 << PC0 ) )                    // If high pulse is greater than 30 us.
		{
			DHT11Data [ 4 ] = ( DHT11Data [ 4 ] << 1 ) | 0x01;  	              // Then it is HIGH.
		}
		else			                              // Otherwise it is LOW.
		{
			DHT11Data [ 4 ] = ( DHT11Data [ 4 ] << 1 );
		}
		while ( PINC & ( 1 << PC0 ) )
		{
			;
		}
	}
	return DHT11Data [ 4 ];
}
