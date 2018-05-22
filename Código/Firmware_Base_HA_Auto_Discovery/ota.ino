
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

void prepareWebserverUpdate(){
  MDNS.begin(HOSTNAME.c_str());
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", HOSTNAME.c_str(), update_path, update_username, update_password);
}

void otaLoop(){
  httpServer.handleClient();
    
}
