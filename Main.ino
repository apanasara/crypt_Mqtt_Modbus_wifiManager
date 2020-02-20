
#include "Config.h"
#include "WifiManager.h"
#include "TTMQTT.h"
#include <Thread.h>             // https://github.com/ivanseidel/ArduinoThread
#include <StaticThreadController.h>
#include <ThreadController.h>


#define MAX485_DE      13
#define MAX485_RE_NEG  15
#define SlaveGroup1 "[[1],[[0,n,12],[48,n,2],[52,r,18],[54,r,20],[56,r,19]]]"/* n=next, r=reset */
#define SlaveGroup2 "[[2,3,4],[[8,n,7],[48,n,1],[56,19]]]"
#define slave_address "[" SlaveGroup1 "," SlaveGroup2 "]"

Config config;

WifiManager wifiManager(&config);
TTMQTT mqtt(&config);

/*------------------Wifi functions---------------------*/
void wifiSetup() {
  while (wifiManager.loop() != E_WIFI_OK) {
    Serial.println("Could not connect to WiFi. Will try again in 5 seconds");
    delay(5000);
  }
  Serial.println("Connected to WiFi");
}


/*-----------------Set up-------------------------*/
void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("writing...");
  config.write();
  Serial.println("reading...");
  config.read();
  Serial.println(config.get_mqttCAcert());
  wifiSetup();
  mqtt.modbus.config(MAX485_RE_NEG, MAX485_DE, Serial);
  mqtt.connect();
}

void loop() {
  mqtt.yield(45000L);
}
