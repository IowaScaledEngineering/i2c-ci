/*************************************************************************
Title:    I2C-CI Firmware
Authors:  Michael Petersen <railfan@drgw.net>
File:     i2c-ci.ino
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2020 Nathan Holmes & Michael Petersen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

PROTOCOL:
    s   = Send start on I2C bus
    WNN = Send hex data NN on I2C bus
    R   = Read data from I2C bus, with NACK, return 2 hex chars over serial
    Q   = Read data from I2C bus, with ACK, return 2 hex chars over serial
    p   = Send stop on I2C bus, return '.' over serial

    \n  = End command recipe and begin I2C transaction, return '.' when done
	
	Return of '!' indicates buffer overflow, recipe ignored
*************************************************************************/

#define   SDA   PD1
#define   SCL   PD0

// Block Write - Block Read Process Call with 255 data bytes:
// S Addr Wr [A] Comm [A] Count [A] Data [A] ... S Addr Rd [A] [Count] A [Data] ... A P
// s Wnn         Wnn      Wnn       Wnn x 255    s Wnn         Wnn       Q x 254  R   p
// <----------- 10 ------------->  <-- 765 -->  <-------- 7 -------->  <--- 255 --> <- 1 -> = 1038 bytes
uint8_t rxStr[2048];

static inline void sda_low() { DDRD |= _BV(SDA); _delay_us(1); }
static inline void sda_high() { DDRD &= ~_BV(SDA); _delay_us(1); }
static inline void scl_low() { DDRD |= _BV(SCL); _delay_us(1); }
static inline void scl_high() { DDRD &= ~_BV(SCL); _delay_us(1); }

void i2cStart(void)
{
	scl_high();
	sda_low();
	scl_low();
	sda_high();
}

void i2cStop(void)
{
	scl_low();
	sda_low();
	scl_high();
	sda_high();
}

uint8_t i2cWriteByte(uint8_t byte)
{
	uint8_t i = 0x80, ack;

	do
	{
		if(byte & i)
		{
			sda_high();
		}
		else
		{
			sda_low();
		}
		
		scl_high();
		scl_low();
		
		i >>= 1;
	} while(i);

	sda_high();  // Release SDA
	
	scl_high();
	if(PIND & _BV(SDA))
		ack = 0;
	else
		ack = 1;
	scl_low();

	return ack;
}

uint8_t i2cReadByte(uint8_t ack)
{
	uint8_t i, data = 0;

	for(i=0; i<8; i++)
	{
		data = data << 1;
		scl_high();
		if(PIND & _BV(SDA))
			data |= 0x01;
		scl_low();
	}
	
	if(ack)
		sda_low();
	scl_high();
	scl_low();
	sda_high();

	return data;
}


/*
uint8_t writeByte(uint8_t addr, uint8_t cmd, uint16_t writeVal)
{
	uint8_t ack;
	
	i2cStart();
	
	i2cWriteByte(addr << 1);
	i2cWriteByte(cmd);
	ack = i2cWriteByte(writeVal);

	i2cStop();

	return ack;
}

uint8_t readByte(uint8_t addr, uint8_t cmd, uint8_t* data)
{
	uint8_t ack = 1;
	*data = 0xFF;
	
	i2cStart();
	
	ack &= i2cWriteByte(addr << 1);
	ack &= i2cWriteByte(cmd);

	i2cStart();

	ack &= i2cWriteByte((addr << 1) | 0x01);
	*data = i2cReadByte(0);

	i2cStop();

	return ack;
}
*/


void setup()
{
	sda_high();
	scl_high();
	DDRD |= _BV(SCL);  // Set SCL as Output

	Serial.begin(115200); //This pipes to the serial monitor
}

void loop()
{
	uint16_t rxPtr = 0;
	uint8_t rxChr = 0;
	uint8_t i;

	uint8_t overflow = 0;
	
	uint8_t data;
	char str[3];
	
	do
	{
		if(Serial.available())
		{
			rxChr = Serial.read();
			if(sizeof(rxStr) == rxPtr)
			{
				// Overflowed, back up the ptr so we don't overrun the buffer
				overflow = 1;
				rxPtr--;
			}
			rxStr[rxPtr++] = rxChr;
		}
	} while('\n' != rxChr);

	if(overflow)
	{
		Serial.print('!');
	}
	else
	{
		for(i=0; i<rxPtr; i++)
		{
			switch(rxStr[i])
			{
				case 's':
					// Start
					i2cStart();
					break;
				case 'p':
					// Stop
					i2cStop();
					break;
				case 'W':
					// Write a byte
					str[0] = rxStr[++i];
					str[1] = rxStr[++i];
					str[2] = 0;
					data = strtol(str, NULL, 16);
					if(!i2cWriteByte(data))
					{
						Serial.print('N');
					}
					break;
				case 'R':
				case 'Q':
					// Read a byte
					data = i2cReadByte(rxStr[i] == 'Q');
					snprintf(str, sizeof(str), "%02X", data);
					Serial.print(str);
					break;
			}
		}
		Serial.print('.');
	}

//	writeByte(0x5C, 0x88, 0x12);
//	snprintf(str, sizeof(str), "%02X", data);
//	Serial.println(str);
}

