#include "myudp.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[6] = {0xA8, 0x61, 0x0A, 0xAE, 0x82, 0xA6};

#define HACKERSPACE_TESTING

#ifdef HACKERSPACE_TESTING
	// actually it can be assigned to us automatically
	// by DHCP, at least in my home //Szymon
	IPAddress our_ip(192, 168, 1, 105);
	// IP address we will be sending data to
	IPAddress ground_ip(192, 168, 1, 100);
	// port on which we will be listening for data
	unsigned long our_port = 2137;
	// port we are sending data to
	unsigned long ground_port = 62398;
#else
	// TO DO: change to actual addresses used in BEXUS
	IPAddress ip(192, 168, 1, 3);
	IPAddress ground_ip(192, 168, 1, 1);
	unsigned long port = 2137;
#endif

void MyUDP::init(void)
{
	pingTime = 0;
	last_rx_uplink = UPLINK_NONE;

	last_rx_val1 = LAST_RX_VAL_NONE;
	last_rx_val2 = LAST_RX_VAL_NONE;

	// start the Ethernet and UDP:
  	Ethernet.init(10);  // Most Arduino shields

	//int ret = Ethernet.begin(mac, 10000, 10000);

	Ethernet.begin(mac, our_ip);

	// TODO: Change init function if using static config
	//if (ret != 1) // timeouts for DHCP
	//{
	//	status = -1;
	//	Serial.println("UDP: error:  " + String(ret));
	//	return;
	//}
	if (Udp.begin(our_port) != 1)
	{
		status = -1;
		Serial.println("UDP: error2");
		return;
	}
	status = 0;
	Serial.println("UDP: ok");

	IPAddress assigned_ip = Ethernet.localIP();
	String toPrint = "UDP: our Address = ";
	toPrint += assigned_ip[0];
	toPrint += ".";
	toPrint += assigned_ip[1];
	toPrint += ".";
	toPrint += assigned_ip[2];
	toPrint += ".";
	toPrint += assigned_ip[3];

	Serial.println(toPrint);
}

void MyUDP::writeLine(String line)
{ 
	size_t len = line.length();
    size_t i = 0;

	// do not attempt to send because it will block for a long time
	if (status != 0) return;

	Serial.print("Writing to UDP: ");
	Serial.println(line);

    // the actual loop that enumerates the buffer
    for (i=0; i < len/DOWNLINK_SINGLE_PKT_SIZE; ++i)
    {
        memcpy(TxBuffer, line.c_str() + (i * DOWNLINK_SINGLE_PKT_SIZE), DOWNLINK_SINGLE_PKT_SIZE);
    	Udp.beginPacket(ground_ip, ground_port);
		Udp.write(TxBuffer, DOWNLINK_SINGLE_PKT_SIZE);
		Udp.endPacket();
    }

    // if there is anything left over
    if (len % DOWNLINK_SINGLE_PKT_SIZE)
	{
        memcpy(TxBuffer, line.c_str() + (len - len % DOWNLINK_SINGLE_PKT_SIZE), len % DOWNLINK_SINGLE_PKT_SIZE);
    	Udp.beginPacket(ground_ip, ground_port);
		Udp.write(TxBuffer, len % DOWNLINK_SINGLE_PKT_SIZE);
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
		else if (strncmp(RxBuffer, "setState_", 9) == 0)
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
		else if (strncmp(RxBuffer, "setValve_", 9) == 0)
		{			
			if (packetSize >= 12)
			{
				int val1, val2;

				if (sscanf(RxBuffer, "setValve_%i_%i", &val1, &val2) == 2)
				{
					if ((val1 >= 1) && (val1 <= 14) &&
						((val2 == 0) || (val2 == 1)))
					{
						last_rx_uplink = UPLINK_SET_VALVE;
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

				ret = "RX: setValve: " + String(last_rx_val1) + " " + String(last_rx_val2);
			}
			else 
			{
				ret = "Error! setValve len: " + String(packetSize);
			}
		}
		else if (strncmp(RxBuffer, "setPump_", 8) == 0)
		{
			if (packetSize >= 11)
			{
				int val1, val2;

				if (sscanf(RxBuffer, "setPump_%i_%i", &val1, &val2) == 2)
				{
					if (((val1 == 1) || (val1 == 2)) && ((val2 >= 0) && (val2 < 256)))
					{
						last_rx_uplink = UPLINK_SET_PUMP;
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
		else if (strncmp(RxBuffer, "setHeating_", 11) == 0)
		{
			if (packetSize >= 12)
			{
				int val1;

				if (sscanf(RxBuffer, "setHeating_%i", &val1) == 1)
				{
					if ((val1 >= 0) && (val1 < 256))
					{
						last_rx_uplink = UPLINK_SET_HEATING;
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