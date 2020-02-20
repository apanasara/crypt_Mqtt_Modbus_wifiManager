#include "WifiManager.h"
#include <IPAddress.h>

WifiManager::WifiManager(Config* config) {
  this->config = config;
  this->lastConnectionAttempt = 0;
}

wifi_result WifiManager::connect() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);//https://github.com/esp8266/Arduino/issues/1997
  WiFi.disconnect(true);
  
  WiFi.begin(config->get_ssid(), config->get_passkey());
  int WiFiCounter = 0;

 return waitForConnection();
}

bool WifiManager::connected() {
  return WiFi.status() == WL_CONNECTED;
}

wifi_result WifiManager::loop() {
  if(!connected()) {
    return connect();
  }
  return E_WIFI_OK;
}

wifi_result WifiManager::waitForConnection() {
  uint8_t status;

  long now = millis();
  lastConnectionAttempt = now;
  while(true) {
    now = millis();
    
    if(now - lastConnectionAttempt > WIFI_TIMEOUT) {
      return WIFI_TIMEOUT;
    }
    
    status = WiFi.status(); 
    
    switch(status) {
      case WL_CONNECTED:
          {IPAddress ip = WiFi.localIP();
          Serial.printf("\nConnected with IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
          Serial.printf("signal strength (RSSI): %ld dBm\n", WiFi.RSSI());
          return E_WIFI_OK;}
          
      case WL_CONNECT_FAILED:
        return E_WIFI_CONNECT_FAILED;    
    }
    
    delay(100);
  }
}
