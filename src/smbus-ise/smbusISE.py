# *************************************************************************
# Title:	SMBus python class for Iowa Scaled Engineering's i2c-ci
# Authors:  Michael D. Petersen <railfan@drgw.net>
#		   Nathan D. Holmes <maverick@drgw.net>
# File:	 smbus-ise.py
# License:  GNU General Public License v3
#
# LICENSE:
#   Copyright (C) 2020 Nathan Holmes & Michael Petersen
#	
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
# DESCRIPTION:
#   Python library to talk with the Iowa Scaled Engineering i2c-ci
#   I2C-to-USB computer interface.  Mimics the SMBus class from
#   the smbus-cffi and smbus2 libraries.
#
# TODO:
#   Add PEC support
#*************************************************************************

import serial

class SMBus(object):
	"""
	Return an SMBus object connected to the specified serial port
	"""
	_pecEnabled = 0
	_pecMismatch = False
	_nack = False
	lastReturnBytes = bytearray()

	def __init__(self, port=""):
		if port != "":
			self.open(port)

	def close(self):
		"""close()

		Disconnects the object from the bus.
		"""
		self.port.close()

	def dealloc(self):
		self.close()

	def open(self, port):
		self.port = serial.Serial(port, 115200)
		_pecEnabled = 0
		_pecMismatch = False
		_nack = False

	def get_response(self):
		self.lastReturnBytes.clear()
		self._nack = False
		while True:     # FIXME: could block indefinitely, assign timeout when opening serial port?
			incomingByte = self.port.read()
			self.lastReturnBytes.append(incomingByte[0])
			if b'N' == incomingByte:
				self._nack = True
			elif b'.' == incomingByte:
				break

	def write_quick(self, i2c_addr):
		"""
		SMBus Quick transaction
		"""
		cmdString = 'sW{:02X}p\n'.format(i2c_addr << 1)
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()

	def read_byte(self, i2c_addr):
		# FIXME
		"""
		SMBus Read Byte
		"""
		return -1

	def write_byte(self, i2c_addr, data):
		"""
		SMBus Write Byte
		"""
		cmdString = 'sW{:02X}W{:02X}p\n'.format(i2c_addr << 1, data)
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()

	def read_byte_data(self, i2c_addr, command):
		"""
		SMBus Read Byte from command/register
		"""
		cmdString = 'sW{:02X}W{:02X}sW{:02X}Rp\n'.format(i2c_addr << 1, command, (i2c_addr << 1)|0x01)
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()
		if(self._nack):
			return -1
		data = int(self.lastReturnBytes[0:2].decode('utf-8'), 16)
		return data

	def write_byte_data(self, i2c_addr, command, data):
		"""
		SMBus Write Byte to command/register
		"""
		cmdString = 'sW{:02X}W{:02X}W{:02X}p\n'.format(i2c_addr << 1, command, data)
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()

	def read_word_data(self, i2c_addr, command):
		"""
		SMBus Read Word from command/register
		"""
		cmdString = 'sW{:02X}W{:02X}sW{:02X}QRp\n'.format(i2c_addr << 1, command, (i2c_addr << 1)|0x01)
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()
		if(self._nack):
			return -1
		data = int(self.lastReturnBytes[0:2].decode('utf-8'), 16) + (int(self.lastReturnBytes[2:4].decode('utf-8'), 16) * 256)
		return data

	def write_word_data(self, i2c_addr, command, data):
		"""
		SMBus Write Word to command/register
		"""
		cmdString = 'sW{:02X}W{:02X}W{:02X}W{:02X}p\n'.format(i2c_addr << 1, command, data & 0xFF, (data >> 8) & 0xFF)
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()

	def process_call(self, i2c_addr, command, data):
		# FIXME
		"""
		SMBus Process Call
		"""

	def read_block_data(self, i2c_addr, command):
		"""
		SMBus Read Block data from command/register
		"""
		# Todo: add optional length parameter.  If provided, don't interactively determine the length from the response
		cmdString = 'sW{:02X}W{:02X}sW{:02X}Q\n'.format(i2c_addr << 1, command, (i2c_addr << 1)|0x01)
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()   # get length
		if(self._nack):
			return -1
		length = int(self.lastReturnBytes[0:2].decode('utf-8'), 16)
		cmdString = ''
		for i in range(length-1):   # one less than length since the last byte will be grabbed with the 'R' below
			cmdString = cmdString + 'Q'
		cmdString = cmdString + 'Rp\n'
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()
		if(self._nack):
			return -1
		data = []
		for i in range(length):
			data.append(int(self.lastReturnBytes[i*2:(i*2)+2].decode('utf-8'), 16))
		return data

	def write_block_data(self, i2c_addr, command, data):
		"""
		SMBus Write Block data to command/register
		"""
		if(len(data) > 255):
			raise OverflowError("write_block_data length greater than 255")
		cmdString = 'sW{:02X}W{:02X}'.format(i2c_addr << 1, command)
		cmdString = cmdString + 'W{:02X}'.format(len(data))
		for value in data:
			cmdString = cmdString + 'W{:02X}'.format(value)
		cmdString = cmdString + 'p\n'
		self.port.write(cmdString.encode('utf-8'))
		self.get_response()

	def block_process_call(self, i2c_addr, command, data):
		# FIXME
		"""
		SMBus Block Process Call
		"""

	def read_i2c_block_data(self, i2c_addr, command, length=32):
		# FIXME
		"""
		I2C Block Read
		"""

	def write_i2c_block_data(self, i2c_addr, command, data):
		# FIXME
		"""
		I2C Block Write
		"""

	def nack(self):
		return self._nack

	def pec(self):
		return self._pecEnabled

	def pec(self, value):
			self._pecEnabled = bool(value)

	def pecMismatch(self):
		return self._pecMismatch

