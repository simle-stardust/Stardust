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

	Serial.print("\nCard type: ");
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();
  
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);

  
  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);
  
  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
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