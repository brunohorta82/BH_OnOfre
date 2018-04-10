/**
 * Desenvolvido por Bruno Horta
 * 
 * Todo o código é livre e pode ser utilizado ou alterado
 * 
 * Exemplo: https://www.youtube.com/watch?v=OyY4ymv6db0
 * */
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
 
#include <Bounce2.h>  // https://github.com/thomasfredericks/Bounce-Arduino-Wiring        
#define AP_TIMEOUT 180
#define SERIAL_BAUDRATE 115200

//CONFIGURAR O SERVIDOR MQTT
#define MQTT_BROKER_IP ""
#define MQTT_BROKER_PORT 1883
#define MQTT_AUTH false
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""


#define SWITCH_ONE 12

//CONSTANTS
const String HOSTNAME  = "SeconFloorLightSwitch03";
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set/"+HOSTNAME;
const String MQTT_SWITCH_ONE_COMMAND_TOPIC = "bhhome/second_floor/light/set";


WiFiClient wclient;
PubSubClient client(MQTT_BROKER_IP,MQTT_BROKER_PORT,wclient);
Bounce debouncer1 = Bounce(); 
//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
bool lastButtonState = false;

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
  pinMode(SWITCH_ONE,INPUT_PULLUP);
  debouncer1.attach(SWITCH_ONE);
  debouncer1.interval(5); // interval in ms
}

void toogle(){
  client.publish(MQTT_SWITCH_ONE_COMMAND_TOPIC.c_str(),"toogle");
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
  if(payloadStr.equals("OTA_ON")){
    OTA = true;
    OTABegin = true;
  }else if (payloadStr.equals("OTA_OFF")){
    OTA = true;
    OTABegin = true;
  }else if (payloadStr.equals("REBOOT")){
    ESP.restart();
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
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
    }
  }
  return client.connected();
}

void loop() {
   debouncer1.update();
  if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()){
      int realChange = debouncer1.read();
      if(realChange != lastButtonState){
       toogle();
        lastButtonState = realChange;
        }
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




