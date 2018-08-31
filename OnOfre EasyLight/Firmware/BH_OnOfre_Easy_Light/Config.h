#include <JustWifi.h> //https://github.com/xoseperez/justwifi
#include <ESP8266mDNS.h>
#include <Timing.h> //https://github.com/scargill/Timing
#include <AsyncMqttClient.h> //https://github.com/marvinroger/async-mqtt-client
#include <ArduinoJson.h> ////Install from Arduino IDE Library Manager
#include <FS.h> 
#include <Ticker.h>
#include <ESPAsyncTCP.h> //https://github.com/me-no-dev/ESPAsyncTCP
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer
#include "devices_manager.h"
#define HARDWARE "bhonofre" 
#define NODE_ID "Easy-Light"
#define FIRMWARE_VERSION 1.0
#define HOSTNAME String(HARDWARE)+"-"+String(NODE_ID)
#define CONFIG_FILENAME  "/config_"+String(HARDWARE)+".json"
#define CONFIG_BUFFER_SIZE 1024
#define WIFI_SSID ""
#define WIFI_SECRET ""
//     ___ ___ ___ ___  _    
//    / __| _ \_ _/ _ \( )___
//   | (_ |  _/| | (_) |/(_-<
//    \___|_| |___\___/  /__/
//     
#define DIRECTION_PIN 14
#define RELAY_ONE 4
#define RELAY_TWO 5 
#define SWITCH_ONE 12
#define SWITCH_TWO 13
#define SWITCH_IO12_NAME "Interrutor 1"
#define SWITCH_IO13_NAME "Interrutor 2"



//    ___ ___ ___ _____ _   _ ___ ___ ___ 
//   | __| __/ __|_   _| | | | _ \ __/ __|
//   | _|| _| (__  | | | |_| |   / _|\__ \
//   |_| |___\___| |_|  \___/|_|_\___|___/
// 
#define PRINT_TO_SERIAL_MONITOR false

//    ___  ___  ___ _____ _    
//   | _ \/ _ \| _ \_   _( )___
//   |  _/ (_) |   / | | |/(_-<
//   |_|  \___/|_|_\ |_|   /__/
//   
#define HTTPS_PORT  443
#define HTTP_PORT  80

//    __  __  ___ _____ _____ 
//   |  \/  |/ _ \_   _|_   _|
//   | |\/| | (_) || |   | |  
//   |_|  |_|\__\_\|_|   |_|  
//  
#define MQTT_BROKER_IP ""
#define MQTT_BROKER_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define PAYLOAD_ON "ON"
#define PAYLOAD_OFF "OFF"
#define PAYLOAD_PULSE_OFF_ON "PULSE_OFF"
#define PAYLOAD_PULSE_ON_OFF "PULSE_ON"

//NODE
String hostname = HOSTNAME;
String nodeId = NODE_ID;

//MQTT
String mqttIpDns = MQTT_BROKER_IP;
String mqttUsername = MQTT_USERNAME;
String mqttPassword = MQTT_PASSWORD;
String baseTopic = String(HARDWARE)+"/"+nodeId;
String availableTopic = String(HARDWARE)+"_"+nodeId+"/status";
const int NUMBER_OF_MQTT_RELAYS = 2;
int MQTT_RELAY_MAP[NUMBER_OF_MQTT_RELAYS] = {RELAY_ONE,RELAY_TWO};

String MQTT_RELAY_TOPIC(int index, bool command){
 return baseTopic+"/relay_"+String(index)+"/"+(command ? "set" : "status");
}
//WI-FI
String wifiSSID = WIFI_SSID;
String wifiSecret = WIFI_SECRET;

//CONTROL FLAGS
bool configNeedsUpdate = false;
bool restartMqtt = false;
bool shouldReboot = false;

//HOME ASSISTANT
bool homeAssistantAutoDiscovery = true;
String homeAssistantAutoDiscoveryPrefix = "homeassistant";

//SWITCH
String switchIO12Name = SWITCH_IO12_NAME;
String switchIO13Name = SWITCH_IO13_NAME;
