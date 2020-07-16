#pragma once
#include <Arduino.h>
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
public:
	MySD(int chipSelect);
	bool init(String dir, String name);
	bool info();
	bool writeLine(String line);
};