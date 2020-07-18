#!/bin/python3

import serial
import datetime
import time

hour = 0
minute = 0
second = 0
DS18B20_0 = 0
DS18B20_1 = 0
DS18B20_2 = 0
humidity = 0
pressure = 0
lattitude = 0
longtitude = 0
altitude = 0
flag_main = 0
hdop = 10

ser = serial.Serial(port="/dev/ttyUSB0", baudrate=115200)  # open serial port

while True:
    time.sleep(5)
    altitude += 25
    flag_main = 64
    if altitude > 500:
        flag_main = 64
    if altitude > 1000:
        flag_main = 64 + 16384
    now = datetime.datetime.now()
    hour = now.hour
    minute = now.minute
    second = now.second
    StringToWrite = "@MarcinSetValues:" + str(hour) + "," + str(minute) + "," + str(second) + "," + str(DS18B20_0) + ","
    StringToWrite += str(DS18B20_1) + "," + str(DS18B20_2) + "," + str(humidity) + "," + str(pressure) + "," + str(lattitude) + ","
    StringToWrite += str(longtitude) + "," + str(altitude) + "," + str(flag_main) + "," + str(hdop) + "\r\n"
    ser.write(str.encode(StringToWrite))
    print(StringToWrite)
    print(str.encode(StringToWrite))
