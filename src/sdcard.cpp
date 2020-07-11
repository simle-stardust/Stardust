#include "sdcard.h"

MySD::MySD(int _chipSelect) {
    chipSelect = _chipSelect;
}

bool MySD::init(String name,  HardwareSerial *serial) {
  mySerial = serial;
  SPI.begin();
	pinMode(SS, OUTPUT);
  if (!SD.begin(chipSelect))
	{
		mySerial->println("initialization failed!");
		return 0;
	}
	mySerial->println("initialization done.");
  
	filename = name + ".txt";
	mySerial->print("Logging to ");
	mySerial->println(filename.c_str());
	myFile = SD.open(filename.c_str(), FILE_WRITE);
	myFile.close();

  return status();
}

bool MySD::status() {

	mySerial->print("\nCard type: ");
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      mySerial->println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      mySerial->println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      mySerial->println("SDHC");
      break;
    default:
      mySerial->println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    mySerial->println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return 0;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  mySerial->print("\nVolume type is FAT");
  mySerial->println(volume.fatType(), DEC);
  mySerial->println();
  
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  mySerial->print("Volume size (bytes): ");
  mySerial->println(volumesize);
  mySerial->print("Volume size (Kbytes): ");
  volumesize /= 1024;
  mySerial->println(volumesize);
  mySerial->print("Volume size (Mbytes): ");
  volumesize /= 1024;
  mySerial->println(volumesize);

  
  mySerial->println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);
  
  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
  return 1;
}

bool MySD::writeLine(String line)
{
	myFile = SD.open(filename.c_str(), FILE_WRITE);

	if (myFile)
	{
    // if the file opened okay, write to it:
		myFile.println(line);
		// close the file:
		myFile.close();
		return 0;
	}
	else
	{
		// if the file didn't open, print an error:
		mySerial->println("error opening file");
		return 1;
	}
};