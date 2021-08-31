#pragma once
#include "Arduino.h"
#include <SPI.h>          // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008

#define DOWNLINK_SINGLE_PKT_SIZE 24 //increase UDP size

#define LAST_RX_VAL_NONE  (0xFF)

typedef enum uplink_t_def
{
    UPLINK_NONE,
    UPLINK_PING,
    UPLINK_SET_STATE,
    UPLINK_SET_VALVE,
    UPLINK_SET_PUMP,
    UPLINK_SET_HEATING,
} uplink_t;

class MyUDP
{

private:
    // buffers for receiving and sending data
    char  RxBuffer[DOWNLINK_SINGLE_PKT_SIZE];  //buffer to hold incoming packet,
    char  TxBuffer[DOWNLINK_SINGLE_PKT_SIZE];  // a string to send back
    // An EthernetUDP instance to let us send and receive packets over UDP
    EthernetUDP Udp;
    uint32_t pingTime;
    uplink_t last_rx_uplink;

    uint8_t last_rx_val1;
    uint8_t last_rx_val2;

    int8_t status;
    uint8_t fail_cnt;
    int initConnection(void);

public:
	void init(void);
    int8_t getStatus(void);
	void writeLine(uint8_t *buf, uint16_t len);
    String tick(void);
    uint32_t timeSinceLastPing(void);
    uplink_t getLastUplink(uint8_t* val1, uint8_t* val2);
};