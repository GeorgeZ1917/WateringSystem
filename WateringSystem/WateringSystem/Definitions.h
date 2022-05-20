// Definitions.h


#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define UBRRVAL 103
#define F_CPU 16000000UL

#include <stdint.h>

void usart_init ( void );
void sendbyte ( unsigned char );
void sendstr  ( unsigned char * );
unsigned char receiveByte ( void );

void setup ();
void setWaterPump ();
void updateLamp ( uint8_t hour );
void receiveBluetooth ();
void sendBluetoothData ( unsigned char *, unsigned char *, unsigned char * );

void requestDHT11 ();
void responseDHT11 ( bool & );
uint8_t receiveDataDHT11 ();

extern bool bluetooth;           // This flag is set when the pump is activated through bluetooth.
extern uint8_t DHT11Data [ 5 ];  // Variables used for the DHT11 sensor.

void twi_INIT ();
void DS3231_setTime ( uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year );


#endif /* DEFINITIONS_H_ */