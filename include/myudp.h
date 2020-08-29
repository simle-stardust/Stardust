#pragma once
#include "Arduino.h"
#include <SPI.h>          // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008

class MyUDP
{

private:
    // buffers for receiving and sending data
    char  RxBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
    char  TxBuffer[UDP_TX_PACKET_MAX_SIZE];  // a string to send back
    // An EthernetUDP instance to let us send and receive packets over UDP
    EthernetUDP Udp;

public:
	void init(void);
	void writeLine(String line);
    void tick(void);
};