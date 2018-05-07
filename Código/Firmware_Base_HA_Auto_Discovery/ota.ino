
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
//OTA 
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

bool OTA = false;
bool OTABegin = false;
void prepareWebserverUpdate(){
  MDNS.begin(HOSTNAME.c_str());
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", HOSTNAME.c_str(), update_path, update_username, update_password);
}

void setupOTA(){
  if (WiFi.status() == WL_CONNECTED && checkMqttConnection()) {
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS SETUP").c_str());
    ArduinoOTA.setHostname(HOSTNAME.c_str());
    ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
    ArduinoOTA.begin();
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS READY").c_str());
  }  
}

void otaLoop(){
  httpServer.handleClient();
      if(OTA){
        if(OTABegin){
          setupOTA();
          OTABegin= false;
        }
        ArduinoOTA.handle();
  }
}

void turnOnArduinoOta(){
  OTA = true;
  OTABegin = true;
}

void turnOffArduinoOta(){
  OTA = false;
  OTABegin = false;
}
