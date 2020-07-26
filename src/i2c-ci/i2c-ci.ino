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

*************************************************************************/

int RXLED = 17;  // The RX LED has a defined Arduino pin

#define   SDA   PD1
#define   SCL   PD0

static inline void sda_low() { DDRD |= _BV(SDA); _delay_us(10); }
static inline void sda_high() { DDRD &= ~_BV(SDA); _delay_us(10); }
static inline void scl_low() { DDRD |= _BV(SCL); _delay_us(10); }
static inline void scl_high() { DDRD &= ~_BV(SCL); _delay_us(10); }

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



void setup()
{
	sda_high();
	scl_high();
	DDRD |= _BV(SCL);  // Set SCL as Output

	Serial.begin(115200); //This pipes to the serial monitor
}

uint8_t i;

void loop()
{
	uint8_t data;
	char str[3];
	
	Serial.print("Read data: ");

	writeByte(0x5C, 0x88, i);
	readByte(0x5C, 0x88, &data);

	snprintf(str, sizeof(str), "%02X", data);
	
	Serial.println(str);

	digitalWrite(RXLED, LOW);   // set the RX LED ON
	TXLED0; //TX LED is not tied to a normally controlled pin so a macro is needed, turn LED OFF
	delay(200);              // wait for a second

	digitalWrite(RXLED, HIGH);    // set the RX LED OFF
	TXLED1; //TX LED macro to turn LED ON
	delay(200);              // wait for a second
	
	i++;
}

