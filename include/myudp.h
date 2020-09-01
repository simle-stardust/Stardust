#pragma once
#include "Arduino.h"
#include <SPI.h>          // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008

typedef enum uplink_t_def
{
    UPLINK_NONE,
    UPLINK_PING,
    UPLINK_STATUS,
    UPLINK_GET_STATE,
    UPLINK_SET_STATE_0,
    UPLINK_SET_STATE_1,
    UPLINK_SET_STATE_2,
    UPLINK_SET_STATE_3,
    UPLINK_SET_STATE_4,
    UPLINK_GET_ALTITUDE,
    UPLINK_GET_POSITION,
    UPLINK_GET_SERVOS,
    UPLINK_GET_PUMP,
    UPLINK_GET_SENSORS,
} uplink_t;

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
    uplink_t tick(void);
};