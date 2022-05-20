// Main.cpp

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "Definitions.h"
#include "DS3231.h"
#include "simple_twi.h"

unsigned char dht11Error [] = "Error retrieving the DHT11 data.\n\n";


int main ()
{
	setup ();
	char DHT11DataString [ 4 ] [ 5 ];
	char DS3231DataString [ 7 ] [ 2 ];                                    // The data is as follows: second, minute, hour, dayOfWeek, dayOfMonth, month and year.
	uint8_t DS3231Data [ 7 ];             
	while ( true )
	{
		bool retry = true;
		while ( retry )
		{
			requestDHT11 ();
			responseDHT11 ( retry );
		}
		
		for ( int i = 0; i < 4; i ++ )
		{
			DHT11Data [ i ] = receiveDataDHT11 ();                        // Receive the humidity and temperature from the DHT11.
			itoa ( DHT11Data [ i ], & DHT11DataString [ i ] [ 0 ] , 10 ); // Convert the value retrieved from the sensor.
		}
		
		char TempHumString [ 31 ];
		strcpy ( TempHumString, "The humidity is: " );
		strcat ( TempHumString, & DHT11DataString [ 0 ] [ 0 ] );
		strcat ( TempHumString, "." );
		strcat ( TempHumString, & DHT11DataString [ 1 ] [ 0 ] );
		strcat ( TempHumString, "%.\n" );
		
		char TempTempString [ 36 ];
		strcpy ( TempTempString, "The temperature is: " );
		strcat ( TempTempString, & DHT11DataString [ 2 ] [ 0 ] );
		strcat ( TempTempString, "." );
		strcat ( TempTempString, & DHT11DataString [ 3 ] [ 0 ] );
		strcat ( TempTempString, " °C.\n" );
		
		unsigned char * humString = reinterpret_cast < unsigned char * > ( TempHumString );
		unsigned char * tempString = reinterpret_cast < unsigned char * > ( TempTempString );
		
		DS3231_getTime ( DS3231Data );
		for ( int i = 0; i < 7; i ++ )
		{
			itoa ( DS3231Data [ i ], & DS3231DataString [ i ] [ 0 ], 10 );
		}	
		
		char TempDateString  [ 50 ];
		strcpy ( TempDateString, "The date is: " );
		strncat ( TempDateString, & DS3231DataString [ 4 ] [ 0 ], 2 );
		strncat ( TempDateString, "/", 1 );
		strncat ( TempDateString, & DS3231DataString [ 5 ] [ 0 ], 2 );
		strncat ( TempDateString, "/", 1 );
		strncat ( TempDateString, & DS3231DataString [ 6 ] [ 0 ], 2 );
		strncat ( TempDateString, ".  ", 3 );
		strncat ( TempDateString, & DS3231DataString [ 2 ] [ 0 ], 2 );
		strncat ( TempDateString, ":", 1 );
		strncat ( TempDateString, & DS3231DataString [ 1 ] [ 0 ], 2 );
		strncat ( TempDateString, ":", 1 );
		strncat ( TempDateString, & DS3231DataString [ 0 ] [ 0 ], 2 );
		strncat ( TempDateString, ".", 1 );
		
		unsigned char * dateString = reinterpret_cast < unsigned char * > ( TempDateString );
		
		receiveBluetooth ();	
				
		if ( ( PINC & 0x02 ) || bluetooth ) // If the hygrometer's output is high or if we send an "On", set the water pump.
		{
			setWaterPump ();
		}
		updateLamp ( DS3231Data [ 2 ] );
		sendBluetoothData ( humString, tempString, dateString );
		_delay_ms ( 3000 );
	}
	return 0;
}
