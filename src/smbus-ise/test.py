from smbusISE import SMBus

smb = SMBus("/dev/ttyACM0")

smb.write_quick(0x5C)
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

smb.write_byte(0x5C, 0x88)
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

smb.write_byte_data(0x5C, 0x88, 0xFE)
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

data = smb.read_byte_data(0x5C, 0x88)
print("Return Data: {:d} 0x{:02X}".format(data, data))
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

smb.write_word_data(0x5C, 0x88, 0x1234)
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

data = smb.read_word_data(0x5C, 0x88)
print("Return Data: {:d} 0x{:04X}".format(data, data))
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

smb.write_block_data(0x5C, 0x88, [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77])
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

data = smb.read_block_data(0x5C, 0x88)
print("Return Data: ", end='')
if(-1 != data):
    for value in data:
        print(" 0x{:02X}".format(value), end='')
    print()
else:
    print(data)
print(smb.lastReturnBytes.decode())
print(smb.nack())
print()

smb.close()
