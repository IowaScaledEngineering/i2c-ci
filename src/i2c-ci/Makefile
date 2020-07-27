#*************************************************************************
#Title:    
#Authors:  Michael Petersen
#          
#File:     
#
#*************************************************************************

BASE_NAME = i2c-ci

BOARD  = SparkFun:avr:promicro:cpu=16MHzatmega32U4
PORT   = /dev/ttyACM0

ARDUINO = ~/arduino

help:
	@echo "make verify..... Build $(BASE_NAME)"
	@echo "make upload..... Flash the firmware"
	@echo "make terminal... Open uart terminal"

verify:
	$(ARDUINO) --verify --board $(BOARD) --port $(PORT) $(BASE_NAME).ino

upload:
	$(ARDUINO) --upload --board $(BOARD) --port $(PORT) $(BASE_NAME).ino

terminal:
	miniterm.py /dev/ttyACM0 115200
