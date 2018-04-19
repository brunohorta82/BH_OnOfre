
#include <Timing.h> //https://github.com/scargill/Timing
//MQTT
#include <PubSubClient.h>
//ESP
#include <ESP8266WiFi.h>
//Wi-Fi Manger library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager
//OTA 
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Bounce2.h> //https://github.com/thomasfredericks/Bounce-Arduino-Wiring
#define AP_TIMEOUT 60
#define SERIAL_BAUDRATE 115200

//CONFIGURAR O SERVIDOR MQTT
#define MQTT_BROKER_IP "192.168.187.203"
#define MQTT_BROKER_PORT 1883
#define MQTT_AUTH true
#define MQTT_USERNAME "homeassistant"
#define MQTT_PASSWORD "moscasMoscas82"
#define PAYLOAD_OPEN "OPEN"
#define PAYLOAD_CLOSE "CLOSE"
#define PAYLOAD_STOP "STOP"

#define BLIND_OPEN_RELAY 04
#define BLIND_CLOSE_RELAY 05

 
#define BLIND_TOUCH 13



//CONSTANTS
const String HOSTNAME  = "Blinds-a";
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set";

const String MQTT_CONTROL_TOPIC = "home/kitchen/window/set";
const String MQTT_STATE_TOPIC = "home/kitchen/window/state";

WiFiClient wclient;
PubSubClient client(MQTT_BROKER_IP,MQTT_BROKER_PORT,wclient);
Bounce debounceTouch = Bounce();

//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;

String lastState = PAYLOAD_CLOSE;
String nextState = PAYLOAD_OPEN;
int timePressed = 0;

Timing notifTimer;
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
  
  pinMode(BLIND_OPEN_RELAY,OUTPUT);
  pinMode(BLIND_CLOSE_RELAY,OUTPUT);
  
  pinMode(BLIND_TOUCH,INPUT_PULLUP);

  debounceTouch.attach(BLIND_TOUCH);
  debounceTouch.interval(500);//DELAY
}
void openBlinds(){
  digitalWrite(BLIND_CLOSE_RELAY,LOW);
  digitalWrite(BLIND_OPEN_RELAY,HIGH);
  nextState = PAYLOAD_CLOSE;
  lastState = PAYLOAD_OPEN;
  Serial.println(PAYLOAD_OPEN);
}
void closeBlinds(){
  digitalWrite(BLIND_OPEN_RELAY,LOW);
  digitalWrite(BLIND_CLOSE_RELAY,HIGH);
  nextState = PAYLOAD_OPEN;
  lastState = PAYLOAD_CLOSE;
  Serial.println(PAYLOAD_CLOSE);
}

void stopBlinds(){
  digitalWrite(BLIND_OPEN_RELAY,LOW);
  digitalWrite(BLIND_CLOSE_RELAY,LOW);
  nextState = lastState;
  lastState = PAYLOAD_STOP;
  Serial.println(PAYLOAD_STOP);
}

//Chamada de recepção de mensagem 
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  for (int i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
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
    openBlinds();
  }else if (payloadStr.equals(PAYLOAD_CLOSE)){
    closeBlinds();
  }else if (payloadStr.equals(PAYLOAD_STOP)){
    stopBlinds();
  }
  }
} 
  
bool checkMqttConnection(){
  if (!client.connected()) {
    if (MQTT_AUTH ? client.connect(HOSTNAME.c_str(),MQTT_USERNAME, MQTT_PASSWORD) : client.connect(HOSTNAME.c_str())) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED ON MQTT");
      client.subscribe(MQTT_SYSTEM_CONTROL_TOPIC.c_str());
      client.subscribe(MQTT_CONTROL_TOPIC.c_str());
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
    }
  }
  return client.connected();
}


void loop() {
if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()){
      client.loop();
      debounceTouch.update();
if(debounceTouch.read()){
   while(digitalRead(BLIND_TOUCH)){
    timePressed++;
    delay(100);
    if(timePressed == 5){
     stopBlinds();
    return;
      }
    }
  timePressed = 0; 
  if(nextState.equals(PAYLOAD_CLOSE)){
    closeBlinds();
  }else if (nextState.equals(PAYLOAD_OPEN)){
    openBlinds();
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

