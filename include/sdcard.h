#pragma once
#include <SPI.h>
#include <SD.h>

class MySD
{
private:
	unsigned int chipSelect = 0;
	Sd2Card card;
	SdVolume volume;
	SdFile root;
	File myFile;
	String filename;
	HardwareSerial *mySerial;
public:
	MySD(int chipSelect);
	bool init(String name, HardwareSerial *serial);
	bool status();
	bool writeLine(String line);
};