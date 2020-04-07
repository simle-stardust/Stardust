#include "sdcard.h"

MySD::MySD(int chipSelect)
{
	pinMode(SS, OUTPUT);

	if (!SD.begin(chipSelect))
	{
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");
}

bool MySD::init(String name) {
	filename = name + ".txt";
	Serial.print("Logging to ");
	Serial.println(filename.c_str());
	myFile = SD.open(filename.c_str(), FILE_WRITE);
	myFile.close();
}

bool MySD::writeLine(String line)
{
	myFile = SD.open(filename.c_str(), FILE_WRITE);

	// if the file opened okay, write to it:
	if (myFile)
	{
		myFile.println(line);
		// close the file:
		myFile.close();
		return 0;
	}
	else
	{
		// if the file didn't open, print an error:
		Serial.println("error opening file");
		return 1;
	}
};