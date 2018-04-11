/**
 * Desenvolvido por Bruno Horta
 * 
 * Todo o código é livre e pode ser utilizado ou alterado
 * 
 * Exemplo: https://www.youtube.com/watch?v=OyY4ymv6db0
 * */
//MQTT
#include <PubSubClient.h>//https://www.youtube.com/watch?v=GMMH6qT8_f4  
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

//CONFIGURAR O SERVIDOR MQTT
#define MQTT_BROKER_IP "192.168.187.203"
#define MQTT_BROKER_PORT 1883
#define MQTT_AUTH true
#define MQTT_USERNAME "homeassistant"
#define MQTT_PASSWORD "moscasMoscas82"


#define RELAY_ONE 5
#define RELAY_TWO 4

#define PAYLOAD_ON "ON"
#define PAYLOAD_OFF "OFF"

//CONSTANTS
const String HOSTNAME  = "OnOfreDual-1";
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set/"+HOSTNAME;
const String MQTT_LIGHT_ONE_TOPIC = "relay/one/set";
const String MQTT_LIGHT_TWO_TOPIC = "relay/two/set";
const String MQTT_LIGHT_ONE_STATE_TOPIC = "relay/one";
const String MQTT_LIGHT_TWO_STATE_TOPIC = "relay/two";


WiFiClient wclient;
PubSubClient client(MQTT_BROKER_IP,MQTT_BROKER_PORT,wclient);

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
  pinMode(RELAY_ONE,OUTPUT);
  pinMode(RELAY_TWO,OUTPUT);
}

void turnOnOut1(){
  digitalWrite(RELAY_ONE,HIGH);
  client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_ON);

}

void turnOffOut1(){
   digitalWrite(RELAY_ONE,LOW);  
   client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_OFF);
}

void turnOnOut2(){
  digitalWrite(RELAY_TWO,HIGH);
  client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_ON);
}



void turnOffOut2(){
   digitalWrite(RELAY_TWO,LOW);  
   client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_OFF);

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
 } else if(topicStr.equals(MQTT_LIGHT_ONE_TOPIC)){
  Serial.println(payloadStr);
  if(payloadStr.equals(PAYLOAD_ON)){
      turnOnOut1();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffOut1();
    }
  }else if(topicStr.equals(MQTT_LIGHT_TWO_TOPIC)){
  Serial.println(payloadStr);
  if(payloadStr.equals(PAYLOAD_ON)){
      turnOnOut2();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffOut2();
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




