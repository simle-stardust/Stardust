#include "myudp.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[6] = {0xA8, 0x61, 0x0A, 0xAE, 0x64, 0x31};

#define HACKERSPACE_TESTING

#ifdef HACKERSPACE_TESTING
IPAddress ip(10, 14, 10, 40);
IPAddress ground_ip(10, 14, 10, 39);
unsigned long port = 56556;
#else
// TO DO: change to actual addresses used in BEXUS
IPAddress ip(192, 168, 1, 3);
IPAddress ground_ip(192, 168, 1, 1);
unsigned long port = 2137;
#endif

void MyUDP::init(void)
{
	// start the Ethernet and UDP:
  	Ethernet.init(10);  // Most Arduino shields
	Ethernet.begin(mac, ip);
	Udp.begin(port);
	Serial.println("udp ok");
}

void MyUDP::writeLine(String line)
{ 
	size_t len = line.length();
    size_t i = 0;

    // the actual loop that enumerates the buffer
    for (i=0; i < len/UDP_TX_PACKET_MAX_SIZE; ++i)
    {
        memcpy(TxBuffer, line.c_str() + (i * UDP_TX_PACKET_MAX_SIZE), UDP_TX_PACKET_MAX_SIZE);
    	Udp.beginPacket(ground_ip, port);
		Udp.write(TxBuffer, UDP_TX_PACKET_MAX_SIZE);
		Udp.endPacket();
    }

    // if there is anything left over
    if (len % UDP_TX_PACKET_MAX_SIZE)
	{
        memcpy(TxBuffer, line.c_str() + (len - len % UDP_TX_PACKET_MAX_SIZE), len % UDP_TX_PACKET_MAX_SIZE);
    	Udp.beginPacket(ground_ip, port);
		Udp.write(TxBuffer, len % UDP_TX_PACKET_MAX_SIZE);
		Udp.endPacket();
	}

}

void MyUDP::tick(void)
{  
	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if(packetSize)
	{
		Serial.print("Received packet of size ");
		Serial.println(packetSize);
		Serial.print("From ");
		IPAddress remote = Udp.remoteIP();
		for (int i =0; i < 4; i++)
		{
			Serial.print(remote[i], DEC);
			if (i < 3)
			{
				Serial.print(".");
			}
		}
		Serial.print(", port ");
		Serial.println(Udp.remotePort());

		// read the packet into packetBufffer
		Udp.read(RxBuffer, UDP_TX_PACKET_MAX_SIZE);
		Serial.println("Contents:");
		Serial.println(RxBuffer);

	}
}