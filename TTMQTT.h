/*
   Limits are defined in Char[]size]
*/

#ifndef TTMQTT_h
#define TTMQTT_h

#include <SPI.h>
#include <WifiIPStack.h> //modified as per IPstack.h
#include <Countdown.h> //modified with type casting
#include <MQTTClientFP.h>
#include "Config.h"
#include "TTModbus.h"

class TTMQTT
{
  public:
    TTMQTT(Config* config);
    void connect();
    bool isRequested = false;
    void credentials(const char*, const char*, const char*);
    TTModbus modbus;
    int yield(unsigned long);
  private:
    Config* config;
    char _topic[10];
    char _sentTopic[10];
    bool _pubDone = false;
    WifiIPStack _ipstack;
    MQTT::Client<WifiIPStack, Countdown, 100, 2> _client;
    void _publisher(const char*, const char*, int);
    void _pubConfirm(MQTT::MessageData&);
    void _RequestScanner(MQTT::MessageData&);
    int rc;
    int _Response(const char*,int);
};
#endif
