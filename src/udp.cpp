#include "udp.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[6] = {0xA8, 0x61, 0x0A, 0xAE, 0x64, 0x31};

#define HACKERSPACE_TESTING

#ifdef HACKERSPACE_TESTING
	IPAddress ip(192, 168, 88, 237);
	IPAddress ground_ip(192, 168, 88, 236);
	unsigned long port = 56161;
#else
	// TO DO: change to actual addresses used in BEXUS
	IPAddress ip(192, 168, 1, 3);
	IPAddress ground_ip(192, 168, 1, 1);
	unsigned long port = 2137;
#endif

// Simple wrapper function that sends response to the 
// host that we have received message from recently
void MyUDP::sendResponse(const char * resp)
{
	Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
	Udp.write(resp);
	Udp.endPacket();
}

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

uplink_t MyUDP::tick(void)
{  
	uplink_t ret = UPLINK_NONE;

	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if (packetSize)
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
		// null terminate the receied string
		RxBuffer[packetSize] = '\0';
		// if something has been received over UDP
		Serial.println("Contents:");
		Serial.println(RxBuffer);
		if (strcmp(RxBuffer, "ping") == 0)
		{
			// ping command received, respond with "pong"
			TxBuffer[0] = 'p';
			TxBuffer[1] = 'o';
			TxBuffer[2] = 'n';
			TxBuffer[3] = 'g';
			Udp.beginPacket(ground_ip, port);
			Udp.write(TxBuffer, 4);
			Udp.endPacket();
			ret = UPLINK_PING; 
		}
		else if (strcmp(RxBuffer, "status") == 0)
		{
			ret = UPLINK_STATUS;
		}
		else if (strcmp(RxBuffer, "getState") == 0)
		{
			ret = UPLINK_GET_STATE;
		}
		else if (strcmp(RxBuffer, "setState0") == 0)
		{
			sendResponse("OK0");
			ret = UPLINK_SET_STATE_0;
		}
		else if (strcmp(RxBuffer, "setState1") == 0)
		{
			sendResponse("OK1");
			ret = UPLINK_SET_STATE_1;
		}
		else if (strcmp(RxBuffer, "setState2") == 0)
		{
			sendResponse("OK2");
			ret = UPLINK_SET_STATE_2;
		}
		else if (strcmp(RxBuffer, "setState3") == 0)
		{
			sendResponse("OK3");
			ret = UPLINK_SET_STATE_3;
		}
		else if (strcmp(RxBuffer, "setState4") == 0)
		{
			sendResponse("OK4");
			ret = UPLINK_SET_STATE_4;
		}
		else if (strcmp(RxBuffer, "getAltitude") == 0)
		{
			ret = UPLINK_GET_ALTITUDE;
		}
		else if (strcmp(RxBuffer, "getPosition") == 0)
		{
			ret = UPLINK_GET_POSITION;
		}
		else if (strcmp(RxBuffer, "getServos") == 0)
		{
			ret = UPLINK_GET_SERVOS;
		}
		else if (strcmp(RxBuffer, "getPump") == 0)
		{
			ret = UPLINK_GET_PUMP;
		}
		else if (strcmp(RxBuffer, "getSensors") == 0)
		{
			ret = UPLINK_GET_SENSORS;
		}
		else
		{
			ret = UPLINK_NONE;
		}
	}
	else
	{
		ret = UPLINK_NONE;
	}
	return ret;
}