#include "sdcard.h"

#define SD_CARD_SS 4

bool MySD::init(String dir, String name)
{
	SPI.begin();
	pinMode(SS, OUTPUT);
	if (!SD.begin(SD_CARD_SS))
	{
		Serial.println("initialization failed!");
		return 0;
	}
	Serial.println("initialization done.");

	filename = dir + "/" + name + ".txt";
	Serial.print("Logging to ");
	Serial.println(filename.c_str());
	SD.mkdir(dir);
	myFile = SD.open(filename.c_str(), FILE_WRITE);
	if (!myFile)
	{
		Serial.println("error opening file");
	}
	myFile.close();

	return 1;
}

bool MySD::writeLine(String line)
{
	myFile = SD.open(filename.c_str(), FILE_WRITE);
	if (myFile)
	{
		// if the file opened okay, write to it:
		myFile.print(line.c_str());
		// save the file even if not at the block bound
		myFile.flush();
		// close the file:
		myFile.close();

		Serial.print("Saved to SD Card: ");
		Serial.println(line);
		return 0;
	}
	else
	{
		// if the file didn't open, print an error:
		Serial.println("error opening file");
		return 1;
	}
};