#include "TTModbus.h"
/*--------------Modbus function--------------------*/
TTModbus::TTModbus():_modbusport(Serial){}
void TTModbus::config(int RE,int DE, Stream& modbusport=Serial){
  _RE=RE;
  _DE=DE;
  _modbusport=modbusport;
  
  pinMode(_RE, OUTPUT);
  pinMode(_DE, OUTPUT);
  postTransmission(1);//reading mode by default
}
void TTModbus::preTransmission(bool d){
  digitalWrite(_RE, 1);
  digitalWrite(_DE, 1);
}

void TTModbus::postTransmission(bool d){
  delay(1); //6microsecond delay required for 9600 baudrate
  digitalWrite(_RE, 0);
  digitalWrite(_DE, 0);
}

void TTModbus::deviceSetup(uint8_t sid) {
  _node.begin(sid, _modbusport);
  // Callbacks allow us to configure the RS485 transceiver correctly
  _node.preTransmission.attach(this,&TTModbus::preTransmission);
  _node.postTransmission.attach(this,&TTModbus::postTransmission);
}

void TTModbus::getReading(uint16_t from, uint16_t nxt, JsonArray& values){      
  union{
    uint32_t i;
    float f;
  }u;
  
  _node.clearResponseBuffer();
  delay(100);//https://github.com/4-20ma/ModbusMaster/issues/88
  ESP.wdtDisable(); //https://github.com/4-20ma/ModbusMaster/issues/86
  uint8_t result = _node.readInputRegisters(from, 2*nxt);
  ESP.wdtEnable(1);
  
  if (result == _node.ku8MBSuccess){       
    for (int i=0; i<nxt;i++){
      u.i=(((unsigned long)_node.getResponseBuffer((2*i)+1)<<16) | (_node.getResponseBuffer(2*i)));
      values.add(u.f);
    }
  }
  else{
    for (int i=0; i<nxt;i++) 
      values.add((char*)0); /* (char*)0 = RawJson("null");*/
  }
}

boolean TTModbus::resetRegister(uint16_t address){
  boolean done = false;

  delay(100);
  int result = _node.writeSingleRegister(address,1);
  if (result == _node.ku8MBSuccess)
    done=true;

  return done;
}
