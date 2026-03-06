def crc8(data: bytes, poly=0x07):
    crc = 0x00
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFF
    return crc

s = "123:PUMP_RUN:ms=500"
print(hex(crc8(s.encode())))
