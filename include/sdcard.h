#pragma once
#include <SPI.h>
#include <SD.h>

class MySD
{
private:
	Sd2Card card;
	SdVolume volume;
	SdFile root;
	File myFile;
	String filename;

public:
	MySD(int chipSelect);
	bool init(String name);
	bool writeLine(String line);
};