#include <Ticker.h>
#define RELAY_DEVICE "relay"
#define RELAY_STATES_FILE "relay_states.json"
Ticker pulseTicker;

String relayState(int gpio){
  return digitalRead(gpio) ? "on" : "off"; 
}
void turnOn(String topic, int gpio, bool inverted) {
  pinMode(gpio,OUTPUT);
 if(inverted){
      digitalWrite( gpio,LOW);
    }else{
      digitalWrite( gpio,HIGH);
    }
    
    mqttSend(topic,true,PAYLOAD_ON);
    switchNotify(gpio, digitalRead(gpio));
    logger("[RELAY GPIO: "+String(gpio)+"] ON");
    saveLastState(gpio,true);
}

void turnOff(String topic,int gpio, bool inverted) {
    pinMode(gpio,OUTPUT);
  if(inverted){
      digitalWrite( gpio,HIGH);
    }else{
      digitalWrite( gpio,LOW);
    }
    mqttSend(topic,true,PAYLOAD_OFF);  
    switchNotify(gpio, digitalRead(gpio));
    logger("[RELAY GPIO: "+String(gpio)+"] OFF");
    saveLastState(gpio,false);
}

void turnOnInverted(String topic, int gpio) {
  turnOn(topic,gpio,true);
  
}

void turnOffInverted(String topic, int gpio) {
  turnOff(topic, gpio,true);
}
void initNormal(bool state,int gpio){
  int index = gpio  == SWITCH_ONE ? 0 : 1;
  String topic = MQTT_TOPIC_BUILDER("relay",index,false);
  if(state){
    turnOnNormal(topic,gpio);
   }else{
    turnOffNormal(topic,gpio);
   }
   
 }
 
bool toogleNormal(int gpio){
  int index = gpio  == SWITCH_ONE ? 0 : 1;
  String topic = MQTT_TOPIC_BUILDER("relay",index,false);
  if(digitalRead(gpio)){
    turnOffNormal(topic,gpio);
   }else{
    turnOnNormal(topic,gpio);
   }
   return digitalRead(gpio);
 }
void turnOnNormal(String topic, int gpio) {
  turnOn(topic, gpio,false);
}

void turnOffNormal(String topic,int gpio) {
  turnOff(topic, gpio,false);
}


void mqttqttRelayControl(String topic, String payload){
 if(payload.equals(PAYLOAD_ON)){
  for(int i = 0 ; i < NUMBER_OF_MQTT_RELAYS; i++){
    if(topic.equals(MQTT_TOPIC_BUILDER(RELAY_DEVICE,i,true))){
      turnOnNormal(MQTT_TOPIC_BUILDER(RELAY_DEVICE,i,false),MQTT_RELAY_MAP[i]);
      logger("[RELAY "+String(i+1)+"] ON");
      }
    }
  }else if (payload.equals(PAYLOAD_OFF)){
    for(int i = 0 ; i < NUMBER_OF_MQTT_RELAYS; i++){
    if(topic.equals(MQTT_TOPIC_BUILDER(RELAY_DEVICE,i,true))){
      turnOffNormal(MQTT_TOPIC_BUILDER(RELAY_DEVICE,i,false),MQTT_RELAY_MAP[i]);
   
      
      }
    }
  }
}

void saveLastState(int _gpio, bool _state){
    DynamicJsonBuffer jsonBuffer(50);
      JsonObject& statesJson = jsonBuffer.createObject();
      statesJson[String(_gpio)] = _state;
      
   if(SPIFFS.begin()){
      File rFile = SPIFFS.open(RELAY_STATES_FILE,"w+");
      if(!rFile){
        logger("[CONFIG] Open config file Error!");
      } else {
       
      statesJson.printTo(rFile);
      }
      rFile.close();
   }else{
     logger("[CONFIG] Open file system Error!");
  }
  SPIFFS.end();
  
}
