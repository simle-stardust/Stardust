#include "sdcard.h"

#define SD_CARD_SS 4

//#define SDCARD_DEBUG

bool MySD::init(String dir, String name)
{
	SPI.begin();
	pinMode(SS, OUTPUT);
	if (!SD.begin(SD_CARD_SS))
	{
#ifdef SDCARD_DEBUG
		Serial.println("initialization failed!");
#endif
		return 0;
	}
#ifdef SDCARD_DEBUG
	Serial.println("initialization done.");
#endif

	filename = dir + "/" + name + ".txt";
#ifdef SDCARD_DEBUG
	Serial.print("Logging to ");
	Serial.println(filename.c_str());
#endif
	SD.mkdir(dir);
	myFile = SD.open(filename.c_str(), FILE_WRITE);
	if (!myFile)
	{
#ifdef SDCARD_DEBUG
		Serial.println("error opening file");
#endif
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

#ifdef SDCARD_DEBUG
		Serial.print("Saved to SD Card: ");
		Serial.println(line);
#endif
		return 0;
	}
	else
	{
		// if the file didn't open, print an error:
#ifdef SDCARD_DEBUG
		Serial.println("error opening file");
#endif
		return 1;
	}
};