
#include <Timing.h> //https://github.com/scargill/Timing
//MQTT
#include <PubSubClient.h> //https://www.youtube.com/watch?v=GMMH6qT8_f4
//ESP
#include <ESP8266WiFi.h> //https://www.youtube.com/watch?v=4d8joORYTIA
//Wi-Fi Manger library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager , https://www.youtube.com/watch?v=wWO9n5DnuLA
//OTA 
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Bounce2.h> //https://github.com/thomasfredericks/Bounce-Arduino-Wiring
#define AP_TIMEOUT 60
#define SERIAL_BAUDRATE 115200

//CONFIGURAR O SERVIDOR MQTT
#define MQTT_BROKER_IP "0.0.0.0"
#define MQTT_BROKER_PORT 1883
#define MQTT_AUTH false
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

#define PAYLOAD_OPEN "OPEN"
#define PAYLOAD_CLOSE "CLOSE"
#define PAYLOAD_STOP "STOP"

//OUTPUTS
#define COVER_OPEN_RELAY 04
#define COVER_CLOSE_RELAY 05

//INPUTS
#define SWITCH_OPEN 12
#define SWITCH_CLOSE 13
//CONSTANTS
const String HOSTNAME  = "CoverController";

const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set";


const char *MQTT_CONTROL_TOPIC = "home/kitchen/window/set";
const char *MQTT_STATE_TOPIC = "home/kitchen/window/state";

//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
String lastState = "-";
     
String lastStateNotified = "NONE";
bool stoped = false;  

WiFiClient wclient;
PubSubClient client(MQTT_BROKER_IP,MQTT_BROKER_PORT,wclient);

Bounce debouncerOpen = Bounce();
Bounce debouncerClose = Bounce();

void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  /*define o tempo limite até o portal de configuração ficar novamente inátivo,
   útil para quando alteramos a password do AP*/
  wifiManager.setTimeout(AP_TIMEOUT);
  wifiManager.autoConnect(HOSTNAME.c_str());
  client.setCallback(callback);
  
  pinMode(COVER_OPEN_RELAY,OUTPUT);
  pinMode(COVER_CLOSE_RELAY,OUTPUT);
  
  pinMode( SWITCH_OPEN,INPUT_PULLUP);
  pinMode( SWITCH_CLOSE,INPUT_PULLUP);
  
  debouncerOpen.attach(SWITCH_OPEN);
  debouncerOpen.interval(5);//DELAY
    
  debouncerClose.attach(SWITCH_CLOSE);
  debouncerClose.interval(5);//DELAY
}


void openCover(){
  digitalWrite(COVER_CLOSE_RELAY,LOW);
  digitalWrite(COVER_OPEN_RELAY,HIGH);
  if(!lastStateNotified.equals(PAYLOAD_OPEN)){
    lastStateNotified = PAYLOAD_OPEN;
    client.publish(MQTT_STATE_TOPIC,PAYLOAD_OPEN,true);
    Serial.println("OPEN");
  }
}
void closeCover(){
  digitalWrite(COVER_OPEN_RELAY,LOW);
  digitalWrite(COVER_CLOSE_RELAY,HIGH);
  if(!lastStateNotified.equals(PAYLOAD_CLOSE)){
    lastStateNotified = PAYLOAD_CLOSE;
    client.publish(MQTT_STATE_TOPIC,PAYLOAD_CLOSE,true);
    Serial.println("CLOSE");
   }
  
}

void stopCover(){
  digitalWrite(COVER_OPEN_RELAY,LOW);
  digitalWrite(COVER_CLOSE_RELAY,LOW);
  if(!lastStateNotified.equals(PAYLOAD_STOP)){
    lastStateNotified = PAYLOAD_STOP;
    client.publish(MQTT_STATE_TOPIC,PAYLOAD_STOP,true);
    Serial.println("STOP");
  }
}

//Chamada de recepção de mensagem 
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  for (int i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
  Serial.print("MQTT RECEIVED: ");
  Serial.println(payloadStr);
  String topicStr = String(topic);
  if(topicStr.equals(MQTT_SYSTEM_CONTROL_TOPIC)){
    if(payloadStr.equals("OTA_ON_"+String(HOSTNAME))){
      OTA = true;
      OTABegin = true;
    }else if (payloadStr.equals("OTA_OFF_"+String(HOSTNAME))){
      OTA = false;
      OTABegin = false;
    }else if (payloadStr.equals("REBOOT_"+String(HOSTNAME))){
      ESP.restart();
    }
  }else if ( topicStr.equals(MQTT_CONTROL_TOPIC)){
  if(payloadStr.equals(PAYLOAD_OPEN)){
    openCover();
  }else if (payloadStr.equals(PAYLOAD_CLOSE)){
    closeCover();
  }else if (payloadStr.equals(PAYLOAD_STOP)){
    stopCover();
  }
  }
} 
  
bool checkMqttConnection(){
  if (!client.connected()) {
    if (MQTT_AUTH ? client.connect(HOSTNAME.c_str(),MQTT_USERNAME, MQTT_PASSWORD) : client.connect(HOSTNAME.c_str())) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED ON MQTT");
      client.subscribe(MQTT_SYSTEM_CONTROL_TOPIC.c_str());
      client.subscribe(MQTT_CONTROL_TOPIC);
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
    }
  }
  return client.connected();
}


void loop() {
debouncerOpen.update();
debouncerClose.update();

if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()){
      client.loop();
      
      int realStateOpen = debouncerOpen.read();
      int realStateClose = debouncerClose.read();
     
      //OPEN
      if(!realStateOpen){
          stoped=false;
          openCover();
      }
      //CLOSE
      if(!realStateClose){
          stoped=false;
          closeCover();
      }
      //STOP
      if(realStateOpen && realStateClose){
        if(!stoped){
          stoped=true;
          stopCover();
        }
       }
       
      if(OTA){
        if(OTABegin){
          setupOTA();
          OTABegin= false;
        }
        ArduinoOTA.handle();
      }
    }
  }
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

