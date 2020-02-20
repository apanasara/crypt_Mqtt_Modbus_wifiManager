#include "TTMQTT.h"
#include <ArduinoJson.h> //version 6.x

/*----Static Variable declaration----*/
//WifiIPStack _ipstack= WifiIPStack("/ca.crt.der","/ca.key.der");

TTMQTT::TTMQTT(Config* config):_client(_ipstack){
  this->config = config;
  isRequested = false;
}

/*--Modbus Handling functions--
void TTMQTT::Address(char* Address){
  strcpy(TTMQTT::_Address,Address);
}*/
void TTMQTT::connect(){
  Serial.println("Entered into MQTT setup");
    /*----Message Handler functions-----*/

    _client.messageHandlers[0].topicFilter="deca/request";
    _client.messageHandlers[0].onReceive.attach(this,&TTMQTT::_RequestScanner);
  
  do{
    rc=_ipstack.disconnect();//to flush & stop
    delay(2000);
    rc=_ipstack.secureConnect(config->get_mqttCAcert(), \
                              config->get_mqttFingerprint(), \
                              config->get_mqttServerName(), \
                              config->get_mqttPort());
    if(rc==1) break;
    Serial.println("will be reconnected in 2 second");
  }while (rc!=1);

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.username.cstring = config->get_mqttUsername();
  data.password.cstring = config->get_mqttPassword();
  data.clientID.cstring = config->get_deviceName();
  Serial.println(data.clientID.cstring);
  Serial.println(data.username.cstring);
  Serial.println(data.password.cstring);
    
  do{
    rc = _client.connect(data);
    if(rc==0) break;
    Serial.printf("MQTT connection status code : %d\n",rc);
    if(rc==-3)
      connect();
    delay(5000);
  }while(rc != 0);
  
  isRequested = false;
  Serial.println("MQTT connected....");
}
/*--------Message pubisher-----*/
void TTMQTT::_publisher(const char* topic_p,const char* msg_payload, int qos) {    
  MQTT::Message message;
  message.qos = (MQTT::QoS)qos;
  message.retained = false;
  message.dup = false;
  message.payload = (void*)msg_payload;
  message.payloadlen = strlen(msg_payload)+1;
  strcpy(_sentTopic,topic_p);
  if (!_client.isConnected())
    connect();
  rc = _client.publish(topic_p, message);
  if (rc==-3)
    connect();
  /*--receiving sent message--*/
  if(qos>0)
  {
      while (_pubDone)
      {
        Serial.println("Waiting for QoS 2 message");
        yield(1000);  
      }
      _pubDone=false;
  }
  _sentTopic[0]='\0';
}

/*----message delivery confirmer------*/
void TTMQTT::_pubConfirm(MQTT::MessageData& md){
    MQTT::Message &message = md.message;
    if(strcmp(_sentTopic,md.topicName.lenstring.data)==0)
      _pubDone=true;
}
/*---------Live Message Handler-----*/
void TTMQTT::_RequestScanner(MQTT::MessageData& md){
  isRequested = true;
  MQTT::Message &message = md.message;
  //strncmp(str1, str2, 4); //https://www.geeksforgeeks.org/difference-strncmp-strcmp-c-cpp/
  
  //Serial.println(message.payload);
  const char* msg_buf=(char*)message.payload;//memcpy(_Laddress,message.payload,message.payloadlen+1);
  Serial.printf("verification successful %s",msg_buf);

  DynamicJsonDocument msgBuffer(1024);

  // compute the required size
  const size_t CAPACITY = JSON_ARRAY_SIZE(3);

  // allocate the memory for the document
  StaticJsonDocument<CAPACITY> doc;
  
  deserializeJson(msgBuffer,msg_buf);
  // extract the values
  JsonArray Msg = msgBuffer.as<JsonArray>();
  auto address=Msg[0].as<const char*>();
  int qos = Msg[1].as<int>();
  
  //JsonArray& Msg = msgBuffer.parseArray(msg_buf);
  //char address[]=Msg.get<const char*>(0);
  //int qos = Msg.get<int>(1);
  int fails = _Response(address,qos);
  Serial.printf("No of fails : %d\n",fails);
  msgBuffer.clear();
}

/*--sending readings--*/
int TTMQTT::_Response(const char* address, int qos=0){
  bool rst;
  int no_of_fail = 0;
   
  DynamicJsonDocument groupBuffer(1024), msgBuffer(1024);
  deserializeJson(groupBuffer,address);
  // extract the values
  JsonArray Groups = groupBuffer.as<JsonArray>();

  
  //JsonArray& Groups = groupBuffer.parseArray(address);
  
  for (auto g : Groups)
  {
    JsonArray SlaveID = g[0].as<JsonArray>();
    JsonArray Addresses = g[1].as<JsonArray>();
        
    for (uint8_t sid : SlaveID) 
    {
      modbus.deviceSetup(sid);
      char sidStr[3];//upto 2 digits
      sidStr[0]='\0';
      sprintf(sidStr,"%d",sid);
      Serial.println(sidStr);
      JsonObject msg = msgBuffer.as<JsonObject>();//.createObject();
      JsonArray values = msg.createNestedArray(sidStr); 

      for(auto a : Addresses){
        uint16_t from, r_address, nxt;
        union{
          uint32_t i;
          float f;
        }u;

        from = a[0].as<unsigned int>();
        char todo = a[1].as<char>();
        if(todo=='n'){
            nxt=a[2].as<unsigned int>();
            rst = false;
        }
        else{
            nxt = 1;
            r_address=a[2].as<unsigned int>();
            rst = true;
        }
        modbus.getReading(from,nxt,values);
        if(rst)
          boolean isdone = modbus.resetRegister(r_address);
      }/*Address loop*/
      
      /*following block is for transforming message-variables to string*/
      {
        String message_buff="";
        serializeJson(msg,message_buff);
        //msg.printTo(message_buff);//https://github.com/bblanchon/ArduinoJson/issues/485
        Serial.println(" ");
        Serial.println(message_buff);
        msgBuffer.clear();

        char conTopic[20];
        conTopic[0]='\0';
        strcat(conTopic,_topic);
        strcat(conTopic,"/");
        strcat(conTopic,sidStr);
        
        Serial.println(conTopic);
        _publisher(conTopic,message_buff.c_str(),qos);
        message_buff="";
      }/*message transformation block*/
      Serial.println("Published....");
    }/*Slave loop*/
  }/*Group loop*/

  groupBuffer.clear();
  return no_of_fail;
}
int TTMQTT::yield(unsigned long timeout_ms){
  return _client.yield(timeout_ms);
}
