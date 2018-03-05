
#include <Timing.h>
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
#define MQTT_AUTH true
#define MQTT_USERNAME "homeassistant"
#define MQTT_PASSWORD "moscasMoscas82"

#define BLIND_OPEN_RELAY 04
#define BLIND_CLOSE_RELAY 05

 
#define BLIND_TOUCH 13



//CONSTANTS
const String HOSTNAME  = "Blinds";
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set";


const char *MQTT_CONTROL_TOPIC = "room/window/set";
const char *MQTT_STATE_TOPIC = "room/windows/state";

//MQTT BROKERS GRATUITOS PARA TESTES https://github.com/mqtt/mqtt.github.io/wiki/public_brokers
const char* MQTT_SERVER = "192.168.187.203";
long debounceDelay = 50;
WiFiClient wclient;
PubSubClient client(MQTT_SERVER,1883,wclient);
Bounce debouncer1 = Bounce();

String notifiedState = "";

//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
String lastState = "-";
String nextState = "OPEN";
int buttonState;             
int lastButtonState = LOW;   
long lastDebounceTime = 0;

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
  wifiManager.autoConnect(HOSTNAME.c_str(),"xptoxpto");
  client.setCallback(callback);
  
  pinMode(BLIND_OPEN_RELAY,OUTPUT);
  pinMode(BLIND_CLOSE_RELAY,OUTPUT);
  
  pinMode(BLIND_TOUCH,INPUT);

  debouncer1.attach(BLIND_TOUCH);
  debouncer1.interval(5);//DELAY
}
void openBlinds(){
  digitalWrite(BLIND_CLOSE_RELAY,LOW);
  digitalWrite(BLIND_OPEN_RELAY,HIGH);
  nextState = "CLOSE";
  lastState = "OPEN";
  Serial.println("OPEN");
}
void closeBlinds(){
  digitalWrite(BLIND_OPEN_RELAY,LOW);
  digitalWrite(BLIND_CLOSE_RELAY,HIGH);
  nextState = "OPEN";
  lastState = "CLOSE";
  Serial.println("CLOSE");
}

void stopBlinds(){
  digitalWrite(BLIND_OPEN_RELAY,LOW);
  digitalWrite(BLIND_CLOSE_RELAY,LOW);
  Serial.println("STOP");
  nextState = lastState;
  lastState = "STOP";
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
  if(payloadStr.equals("OPEN")){
    openBlinds();
  }else if (payloadStr.equals("CLOSE")){
    closeBlinds();
  }else if (payloadStr.equals("STOP")){
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
      client.subscribe(MQTT_CONTROL_TOPIC);
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
    }
  }
  return client.connected();
}


void loop() {
debouncer1.update();
if(debouncer1.read()){
   while(digitalRead(BLIND_TOUCH)){
    timePressed++;
    delay(100);
    if(timePressed == 5){
     stopBlinds();
    return;
      }
    }
  timePressed = 0; 
  if(nextState.equals("CLOSE")){
    closeBlinds();
  }else if (nextState.equals("OPEN")){
    openBlinds();
  }
 }



if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()){
      client.loop();
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

