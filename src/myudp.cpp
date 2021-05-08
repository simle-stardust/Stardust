#include "myudp.h"

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
	pingTime = 0;
	last_rx_uplink = UPLINK_NONE;

	last_rx_val1 = LAST_RX_VAL_NONE;
	last_rx_val2 = LAST_RX_VAL_NONE;

	// start the Ethernet and UDP:
  	Ethernet.init(10);  // Most Arduino shields
	Ethernet.begin(mac, ip);
	Udp.begin(port);
	Serial.println("UDP: ok");
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

String MyUDP::tick(void)
{  
	String ret = "";

	pingTime++;

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
			pingTime = 0;
			ret = "Rx: ping"; 
		}
		else if (strcmp(RxBuffer, "setState_") == 0)
		{
			if (packetSize >= 10)
			{

				if ((RxBuffer[9] >= '0') && (RxBuffer[9] <= '4'))
				{
					last_rx_uplink = UPLINK_SET_STATE;
					last_rx_val1 = RxBuffer[9] - '0';
					pingTime = 0;
				}
				else 
				{
					last_rx_val1 = LAST_RX_VAL_NONE;
				}

				ret = "RX: setState: " + String(last_rx_val1);
			}
			else 
			{
				ret = "Error! setState len: " + String(packetSize);
			}
		}
		else if (strcmp(RxBuffer, "setValve_") == 0)
		{			
			if (packetSize >= 12)
			{
				if ((RxBuffer[9] >= '1') && (RxBuffer[9] <= '7') &&
					((RxBuffer[11] == '0') || (RxBuffer[11] == '1')))
				{
					last_rx_uplink = UPLINK_SET_VALVE;
					last_rx_val1 = RxBuffer[9] - '0';
					last_rx_val2 = RxBuffer[11] - '0';
					pingTime = 0;
				}
				else 
				{
					last_rx_val1 = LAST_RX_VAL_NONE;
					last_rx_val2 = LAST_RX_VAL_NONE;
				}

				ret = "RX: setValve: " + String(last_rx_val1) + String(last_rx_val2);
			}
			else 
			{
				ret = "Error! setValve len: " + String(packetSize);
			}
		}
		else if (strcmp(RxBuffer, "setPump_") == 0)
		{
			if (packetSize >= 11)
			{
				int val1, val2;

				if (sscanf(RxBuffer, "setPump_%i_%i", &val1, &val2) == 2)
				{
					if (((val1 == 1) || (val1 == 2)) && ((val2 > 0) && (val2 < 256)))
					{
						last_rx_val1 = val1;
						last_rx_val2 = val2;
						pingTime = 0;
					}
					else 
					{
						last_rx_val1 = LAST_RX_VAL_NONE;
						last_rx_val2 = LAST_RX_VAL_NONE;
					}
				}
				else 
				{
					last_rx_val1 = LAST_RX_VAL_NONE;
					last_rx_val2 = LAST_RX_VAL_NONE;
				}

				ret = "RX: SetPump: " + String(last_rx_val1) + String(last_rx_val2);
			}
			else 
			{
				ret = "Error! setPump len: " + String(packetSize);
			}
		}
		else if (strcmp(RxBuffer, "setHeating_") == 0)
		{
			if (packetSize >= 12)
			{
				int val1;

				if (sscanf(RxBuffer, "setHeating_%i", &val1) == 1)
				{
					if ((val1 > 0) && (val1 < 256))
					{
						last_rx_val1 = val1;
						pingTime = 0;
					}
					else 
					{
						last_rx_val1 = LAST_RX_VAL_NONE;
					}
				}
				else 
				{
					last_rx_val1 = LAST_RX_VAL_NONE;
				}

				ret = "RX: setHeating: " + String(last_rx_val1) + String(last_rx_val2);
			}
			else 
			{
				ret = "Error! setHeating len: " + String(packetSize);
			}
		}
		else
		{
			ret = "Unknown packet rx!";
		}
	}
	return ret;
}

uint32_t MyUDP::timeSinceLastPing()
{
	return pingTime;
}

uplink_t MyUDP::getLastUplink(uint8_t* val1, uint8_t* val2)
{
	if (val1 != NULL)
	{
		*val1 = last_rx_val1;
	}
	if (val2 != NULL)
	{
		*val2 = last_rx_val2;
	}

	last_rx_val1 = last_rx_val2 = LAST_RX_VAL_NONE;
	return last_rx_uplink;
}