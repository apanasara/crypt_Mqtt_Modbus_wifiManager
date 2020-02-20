//https://www.walletfox.com/course/fp_functioncomposition.php
#ifndef TTModbus_h
#define TTModbus_h
#include <ModbusMasterFP.h>       //https://github.com/4-20ma/ModbusMaster... buffer size modified to 128 from 64 ... 
#include <ArduinoJson.h> //version 5.x
class TTModbus
{
  public:
    TTModbus();
    void config(int,int,Stream&);
    void DefaultAddress(char*);
    void deviceSetup(uint8_t);
    void getReading(uint16_t, uint16_t, JsonArray&);
    boolean resetRegister(uint16_t);
    void preTransmission(bool);
    void postTransmission(bool);
  private:
    int _RE,_DE;
    ModbusMasterFP _node;
    Stream& _modbusport;
};

#endif
