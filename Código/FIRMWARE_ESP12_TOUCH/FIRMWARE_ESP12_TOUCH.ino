/**
 * Desenvolvido por Bruno Horta
 * 
 * Todo o código é livre e pode ser utilizado ou alterado
 * 
 * Exemplo: https://www.youtube.com/watch?v=OyY4ymv6db0
 * */
// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>
//MQTT
#include <PubSubClient.h>
//ESP
#include <ESP8266WiFi.h>'
//Wi-Fi Manger library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager
//OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>            
#define AP_TIMEOUT 180
#define SERIAL_BAUDRATE 115200
#define MQTT_AUTH false
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define RELAY_ONE 5
#define RELAY_TWO 4
#define PAYLOAD_ON "ON"
#define PAYLOAD_OFF "OFF"
#define TOUCH_ONE 12
#define TOUCH_TWO 13
//CONSTANTS
const String HOSTNAME  = "OnOfreDual";
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set/"+HOSTNAME;
const String MQTT_LIGHT_ONE_TOPIC = "relay/one/set";
const String MQTT_LIGHT_TWO_TOPIC = "relay/two/set";
const String MQTT_LIGHT_ONE_STATE_TOPIC = "relay/one";
const String MQTT_LIGHT_TWO_STATE_TOPIC = "relay/two";
//MQTT BROKERS GRATUITOS PARA TESTES https://github.com/mqtt/mqtt.github.io/wiki/public_brokers
const char* MQTT_SERVER = "192.168.187.23";

WiFiClient wclient;
PubSubClient client(MQTT_SERVER,1883,wclient);

//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
bool lastButtonOneState = false;
bool lastButtonTwoState = false;
bool lastButtonThreeState = false;


Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce(); 

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  /*define o tempo limite até o portal de configuração ficar novamente inátivo,
   útil para quando alteramos a password do AP*/
  wifiManager.setTimeout(AP_TIMEOUT);
  if(!wifiManager.autoConnect(HOSTNAME.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  } 
  client.setCallback(callback);
  pinMode(RELAY_ONE,OUTPUT);
  pinMode(RELAY_TWO,OUTPUT);
  // Setup the first button with an internal pull-up :
  pinMode(TOUCH_ONE,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer1.attach(TOUCH_ONE);
  debouncer1.interval(5); // interval in ms
  
   // Setup the second button with an internal pull-up :
  pinMode(TOUCH_TWO,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer2.attach(TOUCH_TWO);
  debouncer2.interval(5); // interval in ms
}

//LIGA O RELAY ONE e envia o estado ON por MQTT
void turnOnOne(){
  digitalWrite(RELAY_ONE,HIGH);
  if (checkMqttConnection()){
    client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_ON,true);
  }
}

//DESLIGA O RELAY ONE e envia o estado OFF por MQTT
void turnOffOne(){
  digitalWrite(RELAY_ONE,LOW);
  if (checkMqttConnection()){  
    client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_OFF,true);
  }
}

//LIGA O RELAY TWO e envia o estado ON por MQTT
void turnOnTwo(){
  digitalWrite(RELAY_TWO,HIGH);
  if (checkMqttConnection()){
    client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_ON,true);
  }
}

//DESLIGA O RELAY TWO e envia o estado OFF por MQTT
void turnOffTwo(){
  digitalWrite(RELAY_TWO,LOW);
  if (checkMqttConnection()){  
    client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_OFF,true);
  }
}

//Inverte o Estado do RELAY ON (ex: se ele estiver ligado então desliga e vice versa)
void toggleSwitchOne() {
  if(digitalRead(RELAY_ONE)){
   turnOffOne();
  }else{
   turnOnOne();
  }  
}

//Inverte o Estado do RELAY TWO (ex: se ele estiver ligado então desliga e vice versa)
void toggleSwitchTwo() {
  if(digitalRead(RELAY_TWO)){
   turnOffTwo();
  }else{
   turnOnTwo();
  }  
}

//Chamada de recepção de mensagens MQTT
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
      OTA = true;
      OTABegin = true;
    }else if (payloadStr.equals("REBOOT_"+String(HOSTNAME))){
      ESP.restart();
    }
  } else if(topicStr.equals(MQTT_LIGHT_ONE_TOPIC)){
    if(payloadStr.equals(PAYLOAD_ON)){
      turnOnOne();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffOne();
    }
  }else if(topicStr.equals(MQTT_LIGHT_TWO_TOPIC)){
    if(payloadStr.equals(PAYLOAD_ON)){
      turnOnTwo();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffTwo();
    }
  }
}

//Verifica se a ligação está ativa, caso não este liga-se e subscreve aos tópicos de interesse
bool checkMqttConnection(){
  if (!client.connected()) {
    if (MQTT_AUTH ? client.connect(HOSTNAME.c_str(),MQTT_USERNAME, MQTT_PASSWORD) : client.connect(HOSTNAME.c_str())) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED ON MQTT");
      client.subscribe(MQTT_SYSTEM_CONTROL_TOPIC.c_str());
      client.subscribe(MQTT_LIGHT_ONE_TOPIC.c_str());
      client.subscribe(MQTT_LIGHT_TWO_TOPIC.c_str());
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
    }
  }
  return client.connected();
}

void loop() {
  debouncer1.update();
  debouncer2.update();
  bool realOneState = debouncer1.read();
  if(lastButtonOneState != realOneState ){
    lastButtonOneState = realOneState;
    if(realOneState){
    toggleSwitchOne();
    }
  }
  bool realTwoState = debouncer2.read();
  if(lastButtonTwoState != realTwoState  ){
    lastButtonTwoState = realTwoState;
     if(realTwoState){
    toggleSwitchTwo();
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


//Setup do OTA para permitir updates de Firmware via Wi-Fi
void setupOTA(){
  if (WiFi.status() == WL_CONNECTED && checkMqttConnection()) {
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS SETUP").c_str());
    ArduinoOTA.setHostname(HOSTNAME.c_str());
    ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
    ArduinoOTA.begin();
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS READY").c_str());
  }  
}


