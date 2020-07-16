#pragma once
#include <Arduino.h>
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
	bool init(String dir, String name);
	bool writeLine(String line);
};